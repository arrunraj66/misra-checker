/*
 * MISRA C:2012 Rule 16.2 - Clang-backed detector
 *
 * Topic: Switch statements
 * Engineering intent: Independent checker contract for Rule 16.2 in the Switch statements family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Model switch, case, default, fallthrough, and termination edges in the control-flow graph.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: A case or default label whose closest enclosing compound statement is not the body of its switch is reported.
 * See docs/known-limits.md for interpretations.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"
#include "misra/c2012/rule_support.hpp"

#include <memory>

namespace misra::c2012 {
namespace {

class Rule_16_02 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "16.2",
        RuleCategory::Required,
        Decidability::Decidable,
        AnalysisScope::SingleTranslationUnit,
        {true, true},
        "Switch statements",
        "Independent checker contract for Rule 16.2 in the Switch statements family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Model switch, case, default, fallthrough, and termination edges in the control-flow graph.",
        ImplementationStatus::Implemented,
    };
    return descriptor;
  }

  [[nodiscard]] RuleEvaluation evaluate(
      const AnalysisContext& context) const override {
    return support::syntax_findings(context, "misra-c2012-16.2-label-not-in-switch-body");
  }
};

}  // namespace

RulePtr make_rule_16_02() {
  return std::make_unique<Rule_16_02>();
}

}  // namespace misra::c2012
