/*
 * MISRA C:2012 Rule 15.7 - Clang-backed detector
 *
 * Topic: Control flow
 * Engineering intent: Independent checker contract for Rule 15.7 in the Control flow family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Build the control-flow graph and inspect jumps, loop exits, compound statements, and function exit structure.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: Every if ... else if chain must end with a final else clause. A lone if without else-if is out of scope.
 * Findings in system headers and other files are ignored.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"

#include <memory>

namespace misra::c2012 {
namespace {

class Rule_15_07 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "15.7",
        RuleCategory::Required,
        Decidability::Decidable,
        AnalysisScope::SingleTranslationUnit,
        {true, true},
        "Control flow",
        "Independent checker contract for Rule 15.7 in the Control flow family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Build the control-flow graph and inspect jumps, loop exits, compound statements, and function exit structure.",
        ImplementationStatus::Implemented,
    };
    return descriptor;
  }

  [[nodiscard]] RuleEvaluation evaluate(
      const AnalysisContext& context) const override {
    RuleEvaluation evaluation{EvaluationStatus::Complete, {}};
    for (const IfElseChainFact& chain : context.control_flow.if_else_chains) {
      if (!chain.has_final_else) {
        evaluation.findings.push_back(
            {"misra-c2012-15.7-missing-final-else", chain.location,
             FindingCertainty::Definite});
      }
    }
    return evaluation;
  }
};

}  // namespace

RulePtr make_rule_15_07() {
  return std::make_unique<Rule_15_07>();
}

}  // namespace misra::c2012
