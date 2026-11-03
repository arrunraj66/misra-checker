/*
 * MISRA C:2012 Rule 14.4 - Clang-backed detector
 *
 * Topic: Control statement expressions
 * Engineering intent: Independent checker contract for Rule 14.4 in the Control statement expressions family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Use control-flow and data-flow analysis to validate loop counters, invariance, and essentially Boolean controlling expressions.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: The controlling expression of if/for/while/do must be essentially Boolean: a comparison, logical operation, _Bool value or <stdbool.h> constant.
 * Findings produced by macro expansion are tagged; see docs/macro-policy.md.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"
#include "misra/c2012/rule_support.hpp"

#include <memory>

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
    return support::non_boolean_condition_findings(context, "misra-c2012-14.4-non-boolean-condition");
  }
};

}  // namespace

RulePtr make_rule_14_04() {
  return std::make_unique<Rule_14_04>();
}

}  // namespace misra::c2012
