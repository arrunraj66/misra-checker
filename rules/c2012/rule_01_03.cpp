/*
 * MISRA C:2012 Rule 1.3 - engineering scaffold
 *
 * Topic: Standard C conformance
 * Engineering intent: Independent checker contract for Rule 1.3 in the Standard C conformance family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Inspect parser diagnostics, implementation limits, language extensions, and the controlled catalogue of undefined or unspecified behavior.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: Subset: report constant division by zero, constant array indexes outside the array, dereference of a null pointer constant and out-of-range constant shifts.
 * Evidence: AST, preprocessor, diagnostic and control-flow observations recorded by the Clang adapter.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"

#include <memory>

#include "misra/observation_rule.hpp"

namespace misra::c2012 {
namespace {

class Rule_01_03 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "1.3",
        RuleCategory::Required,
        Decidability::Undecidable,
        AnalysisScope::System,
        {true, true},
        "Standard C conformance",
        "Independent checker contract for Rule 1.3 in the Standard C conformance family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Inspect parser diagnostics, implementation limits, language extensions, and the controlled catalogue of undefined or unspecified behavior.",
        ImplementationStatus::Implemented,
    };
    return descriptor;
  }

  [[nodiscard]] RuleEvaluation evaluate(
      const AnalysisContext& context) const override {
    return findings_from_observations(context, {"division-by-zero", "array-index-constant-out-of-range", "null-pointer-dereference", "shift-out-of-range"},
                                      "misra-c2012-1.3-undefined-behavior", {}, {});
  }
};

}  // namespace

RulePtr make_rule_01_03() {
  return std::make_unique<Rule_01_03>();
}

}  // namespace misra::c2012
