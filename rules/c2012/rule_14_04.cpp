/*
 * MISRA C:2012 Rule 14.4 - engineering scaffold
 *
 * Topic: Control statement expressions
 * Engineering intent: Independent checker contract for Rule 14.4 in the Control statement expressions family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Use control-flow and data-flow analysis to validate loop counters, invariance, and essentially Boolean controlling expressions.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: Report an if, loop or conditional-operator condition that is not essentially Boolean (relational, equality, logical, negation, or _Bool).
 * Evidence: AST/preprocessor observations recorded by the Clang adapter.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"

#include <memory>

#include "misra/observation_rule.hpp"

namespace misra::c2012 {
namespace {

class Rule_14_04 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "14.4",
        RuleCategory::Required,
        Decidability::Decidable,
        AnalysisScope::SingleTranslationUnit,
        {true, true},
        "Control statement expressions",
        "Independent checker contract for Rule 14.4 in the Control statement expressions family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Use control-flow and data-flow analysis to validate loop counters, invariance, and essentially Boolean controlling expressions.",
        ImplementationStatus::Implemented,
    };
    return descriptor;
  }

  [[nodiscard]] RuleEvaluation evaluate(
      const AnalysisContext& context) const override {
    return findings_from_observations(context, {"non-boolean-condition"},
                                      "misra-c2012-14.4-non-boolean-condition");
  }
};

}  // namespace

RulePtr make_rule_14_04() {
  return std::make_unique<Rule_14_04>();
}

}  // namespace misra::c2012
