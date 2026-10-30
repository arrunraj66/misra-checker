/*
 * MISRA C:2012 Rule 15.5 - engineering scaffold
 *
 * Topic: Control flow
 * Engineering intent: Independent checker contract for Rule 15.5 in the Control flow family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Build the control-flow graph and inspect jumps, loop exits, compound statements, and function exit structure.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: Report a function with more than one return statement, or whose single return is not the last statement of the body.
 * Evidence: AST/preprocessor observations recorded by the Clang adapter.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"

#include <memory>

#include "misra/observation_rule.hpp"

namespace misra::c2012 {
namespace {

class Rule_15_05 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "15.5",
        RuleCategory::Advisory,
        Decidability::Decidable,
        AnalysisScope::SingleTranslationUnit,
        {true, true},
        "Control flow",
        "Independent checker contract for Rule 15.5 in the Control flow family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Build the control-flow graph and inspect jumps, loop exits, compound statements, and function exit structure.",
        ImplementationStatus::Implemented,
    };
    return descriptor;
  }

  [[nodiscard]] RuleEvaluation evaluate(
      const AnalysisContext& context) const override {
    return findings_from_observations(context, {"multiple-exit"},
                                      "misra-c2012-15.5-multiple-exit");
  }
};

}  // namespace

RulePtr make_rule_15_05() {
  return std::make_unique<Rule_15_05>();
}

}  // namespace misra::c2012
