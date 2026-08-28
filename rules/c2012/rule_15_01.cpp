/*
 * MISRA C:2012 Rule 15.1 - engineering scaffold
 *
 * Topic: Control flow
 * Engineering intent: Independent checker contract for Rule 15.1 in the Control flow family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Build the control-flow graph and inspect jumps, loop exits, compound statements, and function exit structure.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: Report every explicit goto statement written in the
 * primary source file. Macro expansion locations are attributed to their use.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"

#include <memory>

namespace misra::c2012 {
namespace {

class Rule_15_01 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "15.1",
        RuleCategory::Advisory,
        Decidability::Decidable,
        AnalysisScope::SingleTranslationUnit,
        {true, true},
        "Control flow",
        "Independent checker contract for Rule 15.1 in the Control flow family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Build the control-flow graph and inspect jumps, loop exits, compound statements, and function exit structure.",
        ImplementationStatus::Implemented,
    };
    return descriptor;
  }

  [[nodiscard]] RuleEvaluation evaluate(
      const AnalysisContext& context) const override {
    RuleEvaluation evaluation{EvaluationStatus::Complete, {}};
    evaluation.findings.reserve(context.control_flow.goto_statements.size());
    for (const GotoStatementFact& fact :
         context.control_flow.goto_statements) {
      evaluation.findings.push_back(
          {"misra-c2012-15.1-goto", fact.location,
           FindingCertainty::Definite});
    }
    return evaluation;
  }
};

}  // namespace

RulePtr make_rule_15_01() {
  return std::make_unique<Rule_15_01>();
}

}  // namespace misra::c2012
