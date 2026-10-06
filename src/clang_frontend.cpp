#include "misra/clang_frontend.hpp"

#include "misra/clang_observations.hpp"

#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/Stmt.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Tooling/CompilationDatabase.h"
#include "clang/Tooling/JSONCompilationDatabase.h"
#include "clang/Tooling/Tooling.h"

namespace misra {
namespace {

class FactVisitor final : public clang::RecursiveASTVisitor<FactVisitor> {
 public:
  FactVisitor(clang::SourceManager& source_manager, AnalysisContext& context)
      : source_manager_(source_manager), context_(context) {}

  // The data-recursion queue is deliberately ignored: with a queue the base
  // class defers children, so the open-block stack would already be popped.
  bool TraverseCompoundStmt(clang::CompoundStmt* block,
                            DataRecursionQueue* = nullptr) {
    for (clang::Stmt* child : block->body()) {
      // Labels may be nested directly (a: b: stmt) or under case labels.
      clang::Stmt* current = child;
      while (current != nullptr) {
        if (auto* label = llvm::dyn_cast<clang::LabelStmt>(current)) {
          label_block_[label->getDecl()] = block;
          current = label->getSubStmt();
        } else if (auto* case_stmt = llvm::dyn_cast<clang::SwitchCase>(current)) {
          current = case_stmt->getSubStmt();
        } else {
          current = nullptr;
        }
      }
    }
    open_blocks_.push_back(block);
    const bool result = RecursiveASTVisitor::TraverseCompoundStmt(block, nullptr);
    open_blocks_.pop_back();
    return result;
  }

  bool TraverseForStmt(clang::ForStmt* loop, DataRecursionQueue* = nullptr) {
    begin_breakable(loop->getForLoc(), true);
    const bool result = RecursiveASTVisitor::TraverseForStmt(loop, nullptr);
    end_breakable();
    return result;
  }

  bool TraverseWhileStmt(clang::WhileStmt* loop, DataRecursionQueue* = nullptr) {
    begin_breakable(loop->getWhileLoc(), true);
    const bool result = RecursiveASTVisitor::TraverseWhileStmt(loop, nullptr);
    end_breakable();
    return result;
  }

  bool TraverseDoStmt(clang::DoStmt* loop, DataRecursionQueue* = nullptr) {
    begin_breakable(loop->getDoLoc(), true);
    const bool result = RecursiveASTVisitor::TraverseDoStmt(loop, nullptr);
    end_breakable();
    return result;
  }

  bool TraverseSwitchStmt(clang::SwitchStmt* statement,
                          DataRecursionQueue* = nullptr) {
    begin_breakable(statement->getSwitchLoc(), false);
    const bool result = RecursiveASTVisitor::TraverseSwitchStmt(statement, nullptr);
    end_breakable();
    return result;
  }

  bool VisitBreakStmt(clang::BreakStmt*) {
    if (!breakables_.empty() && breakables_.back().is_loop &&
        breakables_.back().loop_index >= 0) {
      ++context_.control_flow
            .loops[static_cast<std::size_t>(breakables_.back().loop_index)]
            .terminating_jumps;
    }
    return true;
  }

  bool VisitLabelStmt(clang::LabelStmt* label) {
    label_loops_[label->getDecl()] = open_loops();
    return true;
  }

  // Resolves block containment once every label has been seen.
  void Finish() {
    for (const PendingGoto& pending : pending_) {
      const auto found = label_block_.find(pending.label);
      const auto label_loops = label_loops_.find(pending.label);
      for (const int loop : pending.loops) {
        if ((label_loops == label_loops_.end()) ||
            (label_loops->second.count(loop) == 0U)) {
          ++context_.control_flow.loops[static_cast<std::size_t>(loop)]
                .terminating_jumps;
        }
      }
      context_.control_flow.goto_statements[pending.index]
          .target_in_enclosing_block =
          (found != label_block_.end()) &&
          (pending.enclosing.count(found->second) != 0U);
    }
  }

  bool VisitGotoStmt(clang::GotoStmt* statement) {
    const clang::SourceLocation goto_spelling = statement->getGotoLoc();
    const clang::SourceLocation label_spelling =
        statement->getLabel()->getLocation();
    const clang::SourceLocation location =
        source_manager_.getExpansionLoc(goto_spelling);
    const clang::SourceLocation target_location =
        source_manager_.getExpansionLoc(label_spelling);
    if (location.isInvalid() ||
        target_location.isInvalid() ||
        !source_manager_.isWrittenInMainFile(location)) {
      return true;
    }

    const clang::PresumedLoc presumed = source_manager_.getPresumedLoc(location);
    const clang::PresumedLoc target_presumed =
        source_manager_.getPresumedLoc(target_location);
    if (presumed.isInvalid() || target_presumed.isInvalid()) {
      return true;
    }

    context_.control_flow.goto_statements.push_back(
        {{presumed.getFilename(), presumed.getLine(), presumed.getColumn()},
         {target_presumed.getFilename(), target_presumed.getLine(),
          target_presumed.getColumn()},
         source_manager_.isBeforeInTranslationUnit(goto_spelling,
                                                   label_spelling),
         goto_spelling.isMacroID(), label_spelling.isMacroID(), false});
    pending_.push_back({context_.control_flow.goto_statements.size() - 1U,
                        statement->getLabel(),
                        {open_blocks_.begin(), open_blocks_.end()},
                        open_loops()});
    return true;
  }

 private:
  struct PendingGoto final {
    std::size_t index;
    const clang::LabelDecl* label;
    std::set<const clang::CompoundStmt*> enclosing;
    std::set<int> loops;
  };

  struct Breakable final {
    bool is_loop;
    int loop_index;  // -1 when the loop is not written in the main file
  };

  void begin_breakable(const clang::SourceLocation spelling, const bool is_loop) {
    int index = -1;
    if (is_loop) {
      const clang::SourceLocation location =
          source_manager_.getExpansionLoc(spelling);
      const clang::PresumedLoc presumed =
          location.isValid() ? source_manager_.getPresumedLoc(location)
                             : clang::PresumedLoc();
      if (presumed.isValid() && source_manager_.isWrittenInMainFile(location)) {
        context_.control_flow.loops.push_back(
            {{presumed.getFilename(), presumed.getLine(), presumed.getColumn()},
             0U});
        index = static_cast<int>(context_.control_flow.loops.size()) - 1;
      }
    }
    breakables_.push_back({is_loop, index});
  }

  void end_breakable() { breakables_.pop_back(); }

  std::set<int> open_loops() const {
    std::set<int> loops;
    for (const Breakable& breakable : breakables_) {
      if (breakable.is_loop && (breakable.loop_index >= 0)) {
        loops.insert(breakable.loop_index);
      }
    }
    return loops;
  }

  clang::SourceManager& source_manager_;
  AnalysisContext& context_;
  std::vector<const clang::CompoundStmt*> open_blocks_;
  std::map<const clang::LabelDecl*, const clang::CompoundStmt*> label_block_;
  std::vector<PendingGoto> pending_;
  std::vector<Breakable> breakables_;
  std::map<const clang::LabelDecl*, std::set<int>> label_loops_;
};

class FactConsumer final : public clang::ASTConsumer {
 public:
  FactConsumer(clang::CompilerInstance& compiler, AnalysisContext& context)
      : visitor_(compiler.getSourceManager(), context),
        context_(context),
        comment_handler_(install_preprocessor_observers(compiler, context)) {}

  void HandleTranslationUnit(clang::ASTContext& context) override {
    visitor_.TraverseDecl(context.getTranslationUnitDecl());
    visitor_.Finish();
    collect_ast_observations(context, context_);
  }

 private:
  FactVisitor visitor_;
  AnalysisContext& context_;
  std::unique_ptr<clang::CommentHandler> comment_handler_;
};

class FactAction final : public clang::ASTFrontendAction {
 public:
  explicit FactAction(AnalysisContext& context) : context_(context) {}

  std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
      clang::CompilerInstance& compiler, llvm::StringRef) override {
    return std::make_unique<FactConsumer>(compiler, context_);
  }

 private:
  AnalysisContext& context_;
};

class FactActionFactory final : public clang::tooling::FrontendActionFactory {
 public:
  explicit FactActionFactory(AnalysisContext& context) : context_(context) {}

  std::unique_ptr<clang::FrontendAction> create() override {
    return std::make_unique<FactAction>(context_);
  }

 private:
  AnalysisContext& context_;
};

}  // namespace

FrontendResult ClangFrontend::analyze(
    const std::string& compilation_database,
    const std::vector<std::string>& requested_files) const {
  std::string load_error;
  std::unique_ptr<clang::tooling::JSONCompilationDatabase> database =
      clang::tooling::JSONCompilationDatabase::loadFromFile(
          compilation_database, load_error,
          clang::tooling::JSONCommandLineSyntax::AutoDetect);
  if (!database) {
    return {false, {}, {}, "cannot load compilation database: " + load_error};
  }

  std::vector<std::string> files = requested_files;
  if (files.empty()) {
    files = database->getAllFiles();
  }
  if (files.empty()) {
    return {false, {}, {}, "compilation database contains no source files"};
  }

  FrontendResult result{true, {}, files, {}};
  clang::tooling::ClangTool tool(*database, files);
  FactActionFactory factory(result.context);
  if (tool.run(&factory) != 0) {
    result.success = false;
    result.error_message = "Clang could not analyze one or more translation units";
  }
  return result;
}

}  // namespace misra
