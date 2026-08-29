/*
 * MISRA C:2012 Rule 15.2 - engineering scaffold
 *
 * Topic: Control flow
 * Engineering intent: Independent checker contract for Rule 15.2 in the Control flow family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Build the control-flow graph and inspect jumps, loop exits, compound statements, and function exit structure.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: For every resolved goto in a successfully parsed
 * translation unit, compare the source order of the jump and target label.
 * Report jumps whose target label is not declared later in the function.
 * Macro-origin information and both source locations remain available in the
 * analysis fact for diagnostic and validation evidence.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"

#include <memory>

namespace misra::c2012 {
namespace {

class Rule_15_02 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "15.2",
        RuleCategory::Required,
        Decidability::Decidable,
        AnalysisScope::SingleTranslationUnit,
        {true, true},
        "Control flow",
        "Independent checker contract for Rule 15.2 in the Control flow family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Build the control-flow graph and inspect jumps, loop exits, compound statements, and function exit structure.",
        ImplementationStatus::Implemented,
    };
    return descriptor;
  }

  [[nodiscard]] RuleEvaluation evaluate(
      const AnalysisContext& context) const override {
    RuleEvaluation evaluation{EvaluationStatus::Complete, {}};
    for (const GotoStatementFact& fact :
         context.control_flow.goto_statements) {
      if (!fact.target_declared_later) {
        evaluation.findings.push_back(
            {"misra-c2012-15.2-backward-goto", fact.location,
             FindingCertainty::Definite});
      }
    }
    return evaluation;
  }
};

}  // namespace

RulePtr make_rule_15_02() {
  return std::make_unique<Rule_15_02>();
}

}  // namespace misra::c2012
