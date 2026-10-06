/*
 * MISRA C:2012 Rule 2.5 - engineering scaffold
 *
 * Topic: Unused and unreachable code
 * Engineering intent: Independent checker contract for Rule 2.5 in the Unused and unreachable code family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Build control-flow and reference graphs, then combine reachability, side-effect, and whole-program usage evidence.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: Report a macro defined in the main file that is never expanded or tested.
 * Evidence: AST/preprocessor observations recorded by the Clang adapter.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"

#include <memory>

#include "misra/observation_rule.hpp"

namespace misra::c2012 {
namespace {

class Rule_02_05 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "2.5",
        RuleCategory::Advisory,
        Decidability::Decidable,
        AnalysisScope::System,
        {true, true},
        "Unused and unreachable code",
        "Independent checker contract for Rule 2.5 in the Unused and unreachable code family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Build control-flow and reference graphs, then combine reachability, side-effect, and whole-program usage evidence.",
        ImplementationStatus::Implemented,
    };
    return descriptor;
  }

  [[nodiscard]] RuleEvaluation evaluate(
      const AnalysisContext& context) const override {
    return findings_from_observations(context, {"unused-macro"},
                                      "misra-c2012-2.5-unused-macro");
  }
};

}  // namespace

RulePtr make_rule_02_05() {
  return std::make_unique<Rule_02_05>();
}

}  // namespace misra::c2012
