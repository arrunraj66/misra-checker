/*
 * MISRA C:2012 Rule 13.2 - engineering scaffold
 *
 * Topic: Side effects and evaluation order
 * Engineering intent: Independent checker contract for Rule 13.2 in the Side effects and evaluation order family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Track reads, writes, volatile access, sequencing, persistent side effects, and possible evaluation orders.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: Subset: report a scalar variable that is modified and also read or modified without a sequence point in one full expression.
 * Evidence: AST, preprocessor, diagnostic and control-flow observations recorded by the Clang adapter.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"

#include <memory>

#include "misra/observation_rule.hpp"

namespace misra::c2012 {
namespace {

class Rule_13_02 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "13.2",
        RuleCategory::Required,
        Decidability::Undecidable,
        AnalysisScope::System,
        {true, true},
        "Side effects and evaluation order",
        "Independent checker contract for Rule 13.2 in the Side effects and evaluation order family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Track reads, writes, volatile access, sequencing, persistent side effects, and possible evaluation orders.",
        ImplementationStatus::Implemented,
    };
    return descriptor;
  }

  [[nodiscard]] RuleEvaluation evaluate(
      const AnalysisContext& context) const override {
    return findings_from_observations(context, {"unsequenced-access"},
                                      "misra-c2012-13.2-unsequenced-access", {}, {});
  }
};

}  // namespace

RulePtr make_rule_13_02() {
  return std::make_unique<Rule_13_02>();
}

}  // namespace misra::c2012
