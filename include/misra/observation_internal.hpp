#pragma once

// Internal helpers shared by the Clang adapter translation units.

#include <memory>
#include <string>
#include <utility>

#include "clang/AST/Decl.h"
#include "clang/AST/Expr.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/Stmt.h"
#include "clang/Basic/SourceManager.h"
#include "misra/rule.hpp"

namespace clang {
class ASTContext;
class PPCallbacks;
class Preprocessor;
}  // namespace clang

namespace misra {

// After a compile error Clang leaves recovery expressions and null types in
// the AST. Visitors skip such subtrees so that one error does not hide, or
// crash the analysis of, the rest of the translation unit.
inline bool has_ast_errors(const clang::Stmt* node) {
  const auto* expression = llvm::dyn_cast_or_null<clang::Expr>(node);
  return (expression != nullptr) &&
         (expression->getType().isNull() || expression->containsErrors());
}

inline bool has_decl_errors(const clang::Decl* node) {
  if (node == nullptr) {
    return false;
  }
  if (node->isInvalidDecl()) {
    return true;
  }
  const auto* variable = llvm::dyn_cast<clang::VarDecl>(node);
  return (variable != nullptr) && variable->hasInit() &&
         has_ast_errors(variable->getInit());
}

#define MISRA_SKIP_INVALID_AST                                                \
  bool TraverseStmt(clang::Stmt* node, DataRecursionQueue* queue = nullptr) { \
    return ::misra::has_ast_errors(node) ||                                   \
           RecursiveASTVisitor::TraverseStmt(node, queue);                    \
  }                                                                           \
  bool TraverseDecl(clang::Decl* node) {                                      \
    return ::misra::has_decl_errors(node) ||                                  \
           RecursiveASTVisitor::TraverseDecl(node);                           \
  }

// Appends observations written in the main file; everything else is ignored.
class Recorder final {
 public:
  Recorder(clang::SourceManager& source_manager, AnalysisContext& context)
      : source_manager_(source_manager), context_(context) {}

  // `at_spelling` reports a construct written inside a macro definition at
  // that definition (when it is project code) instead of at each expansion.
  // `unrestricted` also keeps helper facts from project headers.
  void add(std::string kind, const clang::SourceLocation spelling,
           std::string detail = {}, const bool at_spelling = false,
           const bool unrestricted = false) const {
    if (spelling.isInvalid()) {
      return;
    }
    clang::SourceLocation location = source_manager_.getExpansionLoc(spelling);
    if (at_spelling) {
      const clang::SourceLocation written = source_manager_.getSpellingLoc(spelling);
      if (written.isValid() && !source_manager_.isInSystemHeader(written) &&
          (source_manager_.isWrittenInMainFile(written) ||
           allowed_outside_main(written, kind))) {
        location = written;
      }
    }
    if (!source_manager_.isWrittenInMainFile(location) &&
        !(unrestricted && !source_manager_.isInSystemHeader(location)) &&
        !allowed_outside_main(location, kind)) {
      return;
    }
    const clang::PresumedLoc presumed = source_manager_.getPresumedLoc(location);
    if (presumed.isInvalid()) {
      return;
    }
    context_.observations.push_back(
        {std::move(kind),
         {presumed.getFilename(), presumed.getLine(), presumed.getColumn()},
         std::move(detail)});
  }

  // Project headers are reported too, except for findings that only make
  // sense relative to one translation unit (usage-based ones).
  [[nodiscard]] bool allowed_outside_main(const clang::SourceLocation location,
                                          const std::string& kind) const {
    if (source_manager_.isInSystemHeader(location) ||
        source_manager_.isWrittenInMainFile(location)) {
      return false;
    }
    static const char* const kTranslationUnitRelative[] = {
        "unused-typedef", "unused-tag", "unused-macro", "single-function-object",
        "macro-definition", "unused-parameter", "no-prior-declaration",
        "hides-outer-declaration", "missing-return-value", "include-after-code"};
    for (const char* excluded : kTranslationUnitRelative) {
      if (kind == excluded) {
        return false;
      }
    }
    const clang::PresumedLoc presumed = source_manager_.getPresumedLoc(location);
    return presumed.isValid();
  }

  [[nodiscard]] clang::SourceManager& source_manager() const {
    return source_manager_;
  }

 private:
  clang::SourceManager& source_manager_;
  AnalysisContext& context_;
};

// Extra preprocessor observer (macro definition/use analysis).
[[nodiscard]] std::unique_ptr<clang::PPCallbacks> make_extra_macro_observer(
    const Recorder& recorder, clang::Preprocessor& preprocessor);

// Raw-text, per-function and expression-level observations (second wave).
void collect_rest_observations(clang::ASTContext& ast, const Recorder& recorder);

// Control-flow-graph based observations (Rules 2.1, 2.2, 9.1, 17.4).
void collect_cfg_observations(clang::ASTContext& ast, const Recorder& recorder);

// Extra AST, text and cross-reference observations.
void collect_extra_observations(clang::ASTContext& ast, const Recorder& recorder,
                                AnalysisContext& context);

}  // namespace misra
