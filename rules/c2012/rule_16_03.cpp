/*
 * MISRA C:2012 Rule 16.3 - Clang-backed detector
 *
 * Topic: Switch statements
 * Engineering intent: Independent checker contract for Rule 16.3 in the Switch statements family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Model switch, case, default, fallthrough, and termination edges in the control-flow graph.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: A switch clause (labels plus statements) whose last statement is not an unconditional break, possibly inside a nested block, is reported; return, goto and continue do not count.
 * See docs/known-limits.md for interpretations.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"
#include "misra/c2012/rule_support.hpp"

#include <memory>

namespace misra::c2012 {
namespace {

class Rule_16_03 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "16.3",
        RuleCategory::Required,
        Decidability::Decidable,
        AnalysisScope::SingleTranslationUnit,
        {true, true},
        "Switch statements",
        "Independent checker contract for Rule 16.3 in the Switch statements family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Model switch, case, default, fallthrough, and termination edges in the control-flow graph.",
        ImplementationStatus::Implemented,
    };
    return descriptor;
  }

  [[nodiscard]] RuleEvaluation evaluate(
      const AnalysisContext& context) const override {
    return support::syntax_findings(context, "misra-c2012-16.3-clause-not-terminated-by-break");
  }
};

}  // namespace

RulePtr make_rule_16_03() {
  return std::make_unique<Rule_16_03>();
}

}  // namespace misra::c2012
