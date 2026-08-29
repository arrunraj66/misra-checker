#include "misra/clang_frontend.hpp"

#include <memory>
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

 private:
  clang::SourceManager& source_manager_;
  AnalysisContext& context_;
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
