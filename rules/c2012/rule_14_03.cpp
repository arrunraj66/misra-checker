/*
 * MISRA C:2012 Rule 14.3 - Clang-backed detector
 *
 * Topic: Control statement expressions
 * Engineering intent: Independent checker contract for Rule 14.3 in the Control statement expressions family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Use control-flow and data-flow analysis to validate loop counters, invariance, and essentially Boolean controlling expressions.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: A controlling expression with a compile-time value is reported; while (1) and for (;;) are accepted.
 * See docs/known-limits.md for interpretations.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"
#include "misra/c2012/rule_support.hpp"

#include <memory>

namespace misra::c2012 {
namespace {

class Rule_14_03 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "14.3",
        RuleCategory::Required,
        Decidability::Undecidable,
        AnalysisScope::System,
        {true, true},
        "Control statement expressions",
        "Independent checker contract for Rule 14.3 in the Control statement expressions family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Use control-flow and data-flow analysis to validate loop counters, invariance, and essentially Boolean controlling expressions.",
        ImplementationStatus::Implemented,
    };
    return descriptor;
  }

  [[nodiscard]] RuleEvaluation evaluate(
      const AnalysisContext& context) const override {
    return support::syntax_findings(context, "misra-c2012-14.3-invariant-condition");
  }
};

}  // namespace

RulePtr make_rule_14_03() {
  return std::make_unique<Rule_14_03>();
}

}  // namespace misra::c2012
