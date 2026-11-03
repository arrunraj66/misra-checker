#include "misra/clang_frontend.hpp"

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <set>

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/Decl.h"
#include "clang/AST/Expr.h"
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
         goto_spelling.isMacroID(), label_spelling.isMacroID()});
    return true;
  }

 bool VisitIfStmt(clang::IfStmt* statement) {
    record_body(statement->getThen(), "if", statement->getIfLoc());
    const clang::Stmt* const else_branch = statement->getElse();
    if (else_branch != nullptr && !llvm::isa<clang::IfStmt>(else_branch)) {
      record_body(else_branch, "else", statement->getElseLoc());
    }
    if (else_branch != nullptr) {
      else_if_members_.insert(else_branch);
    }
    // Chain head: an if that is not itself the else branch of another if.
    if (else_if_members_.count(statement) == 0U &&
        llvm::isa_and_nonnull<clang::IfStmt>(else_branch)) {
      const clang::IfStmt* last = statement;
      while (const auto* next =
                 llvm::dyn_cast_or_null<clang::IfStmt>(last->getElse())) {
        last = next;
      }
      if (const auto loc = locate(last->getIfLoc())) {
        context_.control_flow.if_else_chains.push_back(
            {*loc, last->getElse() != nullptr});
      }
    }
    return true;
  }

  bool VisitForStmt(clang::ForStmt* statement) {
    record_body(statement->getBody(), "for", statement->getForLoc());
    return true;
  }

  bool VisitWhileStmt(clang::WhileStmt* statement) {
    record_body(statement->getBody(), "while", statement->getWhileLoc());
    return true;
  }

  bool VisitDoStmt(clang::DoStmt* statement) {
    record_body(statement->getBody(), "do", statement->getDoLoc());
    return true;
  }

  bool VisitSwitchStmt(clang::SwitchStmt* statement) {
    record_body(statement->getBody(), "switch", statement->getSwitchLoc());
    bool has_default = false;
    for (const clang::SwitchCase* c = statement->getSwitchCaseList();
         c != nullptr; c = c->getNextSwitchCase()) {
      if (llvm::isa<clang::DefaultStmt>(c)) {
        has_default = true;
      }
    }
    if (const auto loc = locate(statement->getSwitchLoc())) {
      context_.control_flow.switch_statements.push_back({*loc, has_default});
    }
    return true;
  }

  bool VisitCallExpr(clang::CallExpr* call) {
    const clang::FunctionDecl* const callee = call->getDirectCallee();
    if (callee == nullptr || !callee->getIdentifier()) {
      return true;
    }
    const clang::SourceLocation spelling = call->getBeginLoc();
    if (const auto loc = locate(spelling)) {
      context_.calls.push_back(
          {*loc, callee->getName().str(),
           source_manager_.isInSystemHeader(callee->getLocation()),
           spelling.isMacroID()});
    }
    return true;
  }

 private:
  [[nodiscard]] std::optional<SourceLocation> locate(
      const clang::SourceLocation raw) const {
    const clang::SourceLocation expansion = source_manager_.getExpansionLoc(raw);
    if (expansion.isInvalid() || !source_manager_.isWrittenInMainFile(expansion)) {
      return std::nullopt;
    }
    const clang::PresumedLoc presumed = source_manager_.getPresumedLoc(expansion);
    if (presumed.isInvalid()) {
      return std::nullopt;
    }
    return SourceLocation{presumed.getFilename(), presumed.getLine(),
                          presumed.getColumn()};
  }

  void record_body(const clang::Stmt* body, const std::string_view construct,
                   const clang::SourceLocation construct_loc) {
    if (body == nullptr) {
      return;
    }
    if (const auto loc = locate(construct_loc)) {
      context_.control_flow.statement_bodies.push_back(
          {*loc, construct, llvm::isa<clang::CompoundStmt>(body)});
    }
  }

  clang::SourceManager& source_manager_;
  AnalysisContext& context_;
  std::set<const clang::Stmt*> else_if_members_;
};

class FactConsumer final : public clang::ASTConsumer {
 public:
  FactConsumer(clang::SourceManager& source_manager, AnalysisContext& context)
      : visitor_(source_manager, context) {}

  void HandleTranslationUnit(clang::ASTContext& context) override {
    visitor_.TraverseDecl(context.getTranslationUnitDecl());
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
