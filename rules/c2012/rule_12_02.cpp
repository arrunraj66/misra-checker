/*
 * MISRA C:2012 Rule 12.2 - Clang-backed detector
 *
 * Topic: Expressions
 * Engineering intent: Independent checker contract for Rule 12.2 in the Expressions family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Analyze expression trees for precedence, operand ranges, operator constraints, array use, and target-dependent behavior.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: A constant shift count that is negative or not smaller than the width of the (essential) left operand is reported; counts that are not constants are not judged.
 * See docs/known-limits.md for interpretations.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"
#include "misra/c2012/rule_support.hpp"

#include <memory>

namespace misra::c2012 {
namespace {

class Rule_12_02 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "12.2",
        RuleCategory::Required,
        Decidability::Undecidable,
        AnalysisScope::System,
        {true, true},
        "Expressions",
        "Independent checker contract for Rule 12.2 in the Expressions family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Analyze expression trees for precedence, operand ranges, operator constraints, array use, and target-dependent behavior.",
        ImplementationStatus::Implemented,
    };
    return descriptor;
  }

  [[nodiscard]] RuleEvaluation evaluate(
      const AnalysisContext& context) const override {
    return support::syntax_findings(context, "misra-c2012-12.2-shift-count-range");
  }
};

}  // namespace

RulePtr make_rule_12_02() {
  return std::make_unique<Rule_12_02>();
}

}  // namespace misra::c2012
