/*
 * MISRA C:2012 Rule 15.4 - engineering scaffold
 *
 * Topic: Control flow
 * Engineering intent: Independent checker contract for Rule 15.4 in the Control flow family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Build the control-flow graph and inspect jumps, loop exits, compound statements, and function exit structure.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: For every for, while and do loop written in the primary
 * source file, count the break statements bound to that loop (not to a nested
 * loop or switch) plus the goto statements inside it whose label lies outside
 * it. Report loops with more than one such terminating jump.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"

#include <memory>

namespace misra::c2012 {
namespace {

class Rule_15_04 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "15.4",
        RuleCategory::Advisory,
        Decidability::Decidable,
        AnalysisScope::SingleTranslationUnit,
        {true, true},
        "Control flow",
        "Independent checker contract for Rule 15.4 in the Control flow family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Build the control-flow graph and inspect jumps, loop exits, compound statements, and function exit structure.",
        ImplementationStatus::Implemented,
    };
    return descriptor;
  }

  [[nodiscard]] RuleEvaluation evaluate(
      const AnalysisContext& context) const override {
    RuleEvaluation evaluation{EvaluationStatus::Complete, {}};
    for (const LoopFact& loop : context.control_flow.loops) {
      if (loop.terminating_jumps > 1U) {
        evaluation.findings.push_back(
            {"misra-c2012-15.4-multiple-loop-exits", loop.location,
             FindingCertainty::Definite});
      }
    }
    return evaluation;
  }
};

}  // namespace

RulePtr make_rule_15_04() {
  return std::make_unique<Rule_15_04>();
}

}  // namespace misra::c2012
