#pragma once

// Internal helpers shared by the Clang adapter translation units.

#include <memory>
#include <string>
#include <utility>

#include "clang/Basic/LangOptions.h"
#include "clang/Basic/SourceManager.h"
#include "misra/rule.hpp"

namespace clang {
class ASTContext;
class PPCallbacks;
}  // namespace clang

namespace misra {

// Appends observations written in the main file; everything else is ignored.
class Recorder final {
 public:
  Recorder(clang::SourceManager& source_manager, AnalysisContext& context)
      : source_manager_(source_manager), context_(context) {}

  void add(std::string kind, const clang::SourceLocation spelling,
           std::string detail = {}) const {
    if (spelling.isInvalid()) {
      return;
    }
    const clang::SourceLocation location =
        source_manager_.getExpansionLoc(spelling);
    if (!source_manager_.isWrittenInMainFile(location)) {
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

  [[nodiscard]] clang::SourceManager& source_manager() const {
    return source_manager_;
  }

 private:
  clang::SourceManager& source_manager_;
  AnalysisContext& context_;
};

// Extra preprocessor observer (macro definition/use analysis).
[[nodiscard]] std::unique_ptr<clang::PPCallbacks> make_extra_macro_observer(
    const Recorder& recorder, const clang::LangOptions& language);

// Control-flow-graph based observations (Rules 2.1, 2.2, 9.1, 17.4).
void collect_cfg_observations(clang::ASTContext& ast, const Recorder& recorder);

// Extra AST, text and cross-reference observations.
void collect_extra_observations(clang::ASTContext& ast, const Recorder& recorder,
                                AnalysisContext& context);

}  // namespace misra
