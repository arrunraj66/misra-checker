/*
 * MISRA C:2012 Rule 2.2 - engineering scaffold
 *
 * Topic: Unused and unreachable code
 * Engineering intent: Independent checker contract for Rule 2.2 in the Unused and unreachable code family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Build control-flow and reference graphs, then combine reachability, side-effect, and whole-program usage evidence.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Implementation status: Scaffold only; no compliance decision is made.
 */

#include "misra/c2012/rule_factories.hpp"

#include <memory>

namespace misra::c2012 {
namespace {

class Rule_02_02 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "2.2",
        RuleCategory::Required,
        Decidability::Undecidable,
        AnalysisScope::System,
        {true, true},
        "Unused and unreachable code",
        "Independent checker contract for Rule 2.2 in the Unused and unreachable code family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Build control-flow and reference graphs, then combine reachability, side-effect, and whole-program usage evidence.",
        ImplementationStatus::Scaffold,
    };
    return descriptor;
  }

  [[nodiscard]] RuleEvaluation evaluate(
      const AnalysisContext& context) const override {
    (void)context;
    return {EvaluationStatus::NotImplemented};
  }
};

}  // namespace

RulePtr make_rule_02_02() {
  return std::make_unique<Rule_02_02>();
}

}  // namespace misra::c2012
