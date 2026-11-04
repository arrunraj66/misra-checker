/*
 * MISRA C:2012 Rule 1.2 - Clang-backed detector
 *
 * Topic: Standard C conformance
 * Engineering intent: Independent checker contract for Rule 1.2 in the Standard C conformance family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Inspect parser diagnostics, implementation limits, language extensions, and the controlled catalogue of undefined or unspecified behavior.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: Any language extension diagnosed by the compiler under -pedantic in a project file is reported.
 * See docs/known-limits.md for interpretations.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"
#include "misra/c2012/rule_support.hpp"

#include <memory>

namespace misra::c2012 {
namespace {

class Rule_01_02 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "1.2",
        RuleCategory::Advisory,
        Decidability::Undecidable,
        AnalysisScope::SingleTranslationUnit,
        {true, true},
        "Standard C conformance",
        "Independent checker contract for Rule 1.2 in the Standard C conformance family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Inspect parser diagnostics, implementation limits, language extensions, and the controlled catalogue of undefined or unspecified behavior.",
        ImplementationStatus::Implemented,
    };
    return descriptor;
  }

  [[nodiscard]] RuleEvaluation evaluate(
      const AnalysisContext& context) const override {
    return support::syntax_findings(context, "misra-c2012-1.2-language-extension");
  }
};

}  // namespace

RulePtr make_rule_01_02() {
  return std::make_unique<Rule_01_02>();
}

}  // namespace misra::c2012
