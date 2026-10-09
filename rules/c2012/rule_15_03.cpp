/*
 * MISRA C:2012 Rule 15.3 - engineering scaffold
 *
 * Topic: Control flow
 * Engineering intent: Independent checker contract for Rule 15.3 in the Control flow family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Build the control-flow graph and inspect jumps, loop exits, compound statements, and function exit structure.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: For every resolved goto in a successfully parsed
 * translation unit, report the jump when its target label is not declared in
 * the same compound statement as the jump or in a compound statement that
 * encloses it. Containment is computed from the Clang AST, not source order.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"

#include <memory>

namespace misra::c2012 {
namespace {

class Rule_15_03 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "15.3",
        RuleCategory::Required,
        Decidability::Decidable,
        AnalysisScope::SingleTranslationUnit,
        {true, true},
        "Control flow",
        "Independent checker contract for Rule 15.3 in the Control flow family. The exact normative predicate remains linked to the controlled licensed rule specification.",
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
      if (!fact.target_in_enclosing_block) {
        evaluation.findings.push_back(
            {"misra-c2012-15.3-label-not-in-enclosing-block", fact.location,
             FindingCertainty::Definite});
      }
    }
    return evaluation;
  }
};

}  // namespace

RulePtr make_rule_15_03() {
  return std::make_unique<Rule_15_03>();
}

}  // namespace misra::c2012
