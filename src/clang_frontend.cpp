#include "misra/clang_frontend.hpp"

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

  // Resolves block containment once every label has been seen.
  void Finish() {
    for (const PendingGoto& pending : pending_) {
      const auto found = label_block_.find(pending.label);
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
                        {open_blocks_.begin(), open_blocks_.end()}});
    return true;
  }

 private:
  struct PendingGoto final {
    std::size_t index;
    const clang::LabelDecl* label;
    std::set<const clang::CompoundStmt*> enclosing;
  };

  clang::SourceManager& source_manager_;
  AnalysisContext& context_;
  std::vector<const clang::CompoundStmt*> open_blocks_;
  std::map<const clang::LabelDecl*, const clang::CompoundStmt*> label_block_;
  std::vector<PendingGoto> pending_;
};

class FactConsumer final : public clang::ASTConsumer {
 public:
  FactConsumer(clang::SourceManager& source_manager, AnalysisContext& context)
      : visitor_(source_manager, context) {}

  void HandleTranslationUnit(clang::ASTContext& context) override {
    visitor_.TraverseDecl(context.getTranslationUnitDecl());
    visitor_.Finish();
  }

 private:
  FactVisitor visitor_;
};

class FactAction final : public clang::ASTFrontendAction {
 public:
  explicit FactAction(AnalysisContext& context) : context_(context) {}

  std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
      clang::CompilerInstance& compiler, llvm::StringRef) override {
    return std::make_unique<FactConsumer>(compiler.getSourceManager(), context_);
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
