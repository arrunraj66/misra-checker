/*
 * MISRA C:2012 Rule 14.1 - engineering scaffold
 *
 * Topic: Control statement expressions
 * Engineering intent: Independent checker contract for Rule 14.1 in the Control statement expressions family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Use control-flow and data-flow analysis to validate loop counters, invariance, and essentially Boolean controlling expressions.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: Report a for loop whose increment expression modifies a floating-point variable.
 * Evidence: AST/preprocessor observations recorded by the Clang adapter.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"

#include <memory>

#include "misra/observation_rule.hpp"

namespace misra::c2012 {
namespace {

class Rule_14_01 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "14.1",
        RuleCategory::Required,
        Decidability::Undecidable,
        AnalysisScope::System,
        {true, true},
        "Control statement expressions",
        "Independent checker contract for Rule 14.1 in the Control statement expressions family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Use control-flow and data-flow analysis to validate loop counters, invariance, and essentially Boolean controlling expressions.",
        ImplementationStatus::Implemented,
    };
    return descriptor;
  }

  [[nodiscard]] RuleEvaluation evaluate(
      const AnalysisContext& context) const override {
    return findings_from_observations(context, {"float-loop-counter"},
                                      "misra-c2012-14.1-float-loop-counter");
  }
};

}  // namespace

RulePtr make_rule_14_01() {
  return std::make_unique<Rule_14_01>();
}

}  // namespace misra::c2012
