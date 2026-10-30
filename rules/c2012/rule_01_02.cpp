/*
 * MISRA C:2012 Rule 1.2 - engineering scaffold
 *
 * Topic: Standard C conformance
 * Engineering intent: Independent checker contract for Rule 1.2 in the Standard C conformance family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Inspect parser diagnostics, implementation limits, language extensions, and the controlled catalogue of undefined or unspecified behavior.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: Report use of compiler extensions: statement expressions, inline assembly, computed goto, label addresses, case ranges, typeof, omitted conditional operand and explicit attributes.
 * Evidence: AST/preprocessor observations recorded by the Clang adapter.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"

#include <memory>

#include "misra/observation_rule.hpp"

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
    return findings_from_observations(context, {"language-extension"},
                                      "misra-c2012-1.2-language-extension");
  }
};

}  // namespace

RulePtr make_rule_01_02() {
  return std::make_unique<Rule_01_02>();
}

}  // namespace misra::c2012
