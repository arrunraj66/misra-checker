/*
 * MISRA C:2012 Rule 2.4 - engineering scaffold
 *
 * Topic: Unused and unreachable code
 * Engineering intent: Independent checker contract for Rule 2.4 in the Unused and unreachable code family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Build control-flow and reference graphs, then combine reachability, side-effect, and whole-program usage evidence.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: Report a named struct, union or enum tag declared in the main file that is never referred to as a type.
 * Evidence: AST/preprocessor observations recorded by the Clang adapter.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"

#include <memory>

#include "misra/observation_rule.hpp"

namespace misra::c2012 {
namespace {

class Rule_02_04 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "2.4",
        RuleCategory::Advisory,
        Decidability::Decidable,
        AnalysisScope::System,
        {true, true},
        "Unused and unreachable code",
        "Independent checker contract for Rule 2.4 in the Unused and unreachable code family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Build control-flow and reference graphs, then combine reachability, side-effect, and whole-program usage evidence.",
        ImplementationStatus::Implemented,
    };
    return descriptor;
  }

  [[nodiscard]] RuleEvaluation evaluate(
      const AnalysisContext& context) const override {
    return findings_from_observations(context, {"unused-tag"},
                                      "misra-c2012-2.4-unused-tag");
  }
};

}  // namespace

RulePtr make_rule_02_04() {
  return std::make_unique<Rule_02_04>();
}

}  // namespace misra::c2012
