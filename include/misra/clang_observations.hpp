#pragma once

#include <memory>

#include "misra/rule.hpp"

namespace clang {
class ASTContext;
class CommentHandler;
class CompilerInstance;
}  // namespace clang

namespace misra {

// Registers preprocessor-level observers (macros, comments) on the compiler.
// The returned handler must outlive parsing.
[[nodiscard]] std::unique_ptr<clang::CommentHandler>
install_preprocessor_observers(clang::CompilerInstance& compiler,
                               AnalysisContext& context);

// Walks the translation unit and records AST-level observations.
void collect_ast_observations(clang::ASTContext& ast, AnalysisContext& context);

}  // namespace misra
