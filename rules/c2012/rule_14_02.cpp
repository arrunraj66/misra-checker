/*
 * MISRA C:2012 Rule 14.2 - engineering scaffold
 *
 * Topic: Control statement expressions
 * Engineering intent: Independent checker contract for Rule 14.2 in the Control statement expressions family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Use control-flow and data-flow analysis to validate loop counters, invariance, and essentially Boolean controlling expressions.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: Subset: report a for statement that lacks a single loop counter modified only by its third clause, tested by its second clause, and untouched by the body (the no-clause for(;;) form is exempt).
 * Evidence: AST/preprocessor/control-flow observations recorded by the Clang adapter.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"

#include <memory>

#include "misra/observation_rule.hpp"

namespace misra::c2012 {
namespace {

class Rule_14_02 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "14.2",
        RuleCategory::Required,
        Decidability::Undecidable,
        AnalysisScope::System,
        {true, true},
        "Control statement expressions",
        "Independent checker contract for Rule 14.2 in the Control statement expressions family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Use control-flow and data-flow analysis to validate loop counters, invariance, and essentially Boolean controlling expressions.",
        ImplementationStatus::Implemented,
    };
    return descriptor;
  }

  [[nodiscard]] RuleEvaluation evaluate(
      const AnalysisContext& context) const override {
    return findings_from_observations(context, {"for-loop-not-well-formed"},
                                      "misra-c2012-14.2-for-loop-not-well-formed");
  }
};

}  // namespace

RulePtr make_rule_14_02() {
  return std::make_unique<Rule_14_02>();
}

}  // namespace misra::c2012
