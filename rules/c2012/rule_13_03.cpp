/*
 * MISRA C:2012 Rule 13.3 - engineering scaffold
 *
 * Topic: Side effects and evaluation order
 * Engineering intent: Independent checker contract for Rule 13.3 in the Side effects and evaluation order family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Track reads, writes, volatile access, sequencing, persistent side effects, and possible evaluation orders.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: Report an increment or decrement that is not a whole expression statement and shares its full expression with another side effect.
 * Evidence: AST, preprocessor, diagnostic and control-flow observations recorded by the Clang adapter.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"

#include <memory>

#include "misra/observation_rule.hpp"

namespace misra::c2012 {
namespace {

class Rule_13_03 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "13.3",
        RuleCategory::Advisory,
        Decidability::Decidable,
        AnalysisScope::SingleTranslationUnit,
        {true, true},
        "Side effects and evaluation order",
        "Independent checker contract for Rule 13.3 in the Side effects and evaluation order family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Track reads, writes, volatile access, sequencing, persistent side effects, and possible evaluation orders.",
        ImplementationStatus::Implemented,
    };
    return descriptor;
  }

  [[nodiscard]] RuleEvaluation evaluate(
      const AnalysisContext& context) const override {
    return findings_from_observations(context, {"increment-with-side-effect"},
                                      "misra-c2012-13.3-increment-with-side-effect", {}, {});
  }
};

}  // namespace

RulePtr make_rule_13_03() {
  return std::make_unique<Rule_13_03>();
}

}  // namespace misra::c2012
