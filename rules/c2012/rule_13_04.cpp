/*
 * MISRA C:2012 Rule 13.4 - Clang-backed detector
 *
 * Topic: Side effects and evaluation order
 * Engineering intent: Independent checker contract for Rule 13.4 in the Side effects and evaluation order family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Track reads, writes, volatile access, sequencing, persistent side effects, and possible evaluation orders.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: An assignment operator whose result value is consumed by its context is reported.
 * Findings produced by macro expansion are tagged; see docs/macro-policy.md.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"
#include "misra/c2012/rule_support.hpp"

#include <memory>

namespace misra::c2012 {
namespace {

class Rule_13_04 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "13.4",
        RuleCategory::Advisory,
        Decidability::Decidable,
        AnalysisScope::SingleTranslationUnit,
        {true, true},
        "Side effects and evaluation order",
        "Independent checker contract for Rule 13.4 in the Side effects and evaluation order family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Track reads, writes, volatile access, sequencing, persistent side effects, and possible evaluation orders.",
        ImplementationStatus::Implemented,
    };
    return descriptor;
  }

  [[nodiscard]] RuleEvaluation evaluate(
      const AnalysisContext& context) const override {
    return support::syntax_findings(context, "misra-c2012-13.4-assignment-value-used");
  }
};

}  // namespace

RulePtr make_rule_13_04() {
  return std::make_unique<Rule_13_04>();
}

}  // namespace misra::c2012
