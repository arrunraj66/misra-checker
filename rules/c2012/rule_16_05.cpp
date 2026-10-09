/*
 * MISRA C:2012 Rule 16.5 - engineering scaffold
 *
 * Topic: Switch statements
 * Engineering intent: Independent checker contract for Rule 16.5 in the Switch statements family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Model switch, case, default, fallthrough, and termination edges in the control-flow graph.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: Report a switch whose default clause is neither the first nor the last clause.
 * Evidence: AST/preprocessor observations recorded by the Clang adapter.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"

#include <memory>

#include "misra/observation_rule.hpp"

namespace misra::c2012 {
namespace {

class Rule_16_05 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "16.5",
        RuleCategory::Required,
        Decidability::Decidable,
        AnalysisScope::SingleTranslationUnit,
        {true, true},
        "Switch statements",
        "Independent checker contract for Rule 16.5 in the Switch statements family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Model switch, case, default, fallthrough, and termination edges in the control-flow graph.",
        ImplementationStatus::Implemented,
    };
    return descriptor;
  }

  [[nodiscard]] RuleEvaluation evaluate(
      const AnalysisContext& context) const override {
    return findings_from_observations(context, {"default-not-first-or-last"},
                                      "misra-c2012-16.5-default-not-first-or-last");
  }
};

}  // namespace

RulePtr make_rule_16_05() {
  return std::make_unique<Rule_16_05>();
}

}  // namespace misra::c2012
