/*
 * MISRA C:2012 Rule 16.4 - Clang-backed detector
 *
 * Topic: Switch statements
 * Engineering intent: Independent checker contract for Rule 16.4 in the Switch statements family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Model switch, case, default, fallthrough, and termination edges in the control-flow graph.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: Every switch statement must contain a default label.
 * Findings in system headers and other files are ignored.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"

#include <memory>

namespace misra::c2012 {
namespace {

class Rule_16_04 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "16.4",
        RuleCategory::Required,
        Decidability::Decidable,
        AnalysisScope::SingleTranslationUnit,
        {true, true},
        "Switch statements",
        "Independent checker contract for Rule 16.4 in the Switch statements family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Model switch, case, default, fallthrough, and termination edges in the control-flow graph.",
        ImplementationStatus::Implemented,
    };
    return descriptor;
  }

  [[nodiscard]] RuleEvaluation evaluate(
      const AnalysisContext& context) const override {
    RuleEvaluation evaluation{EvaluationStatus::Complete, {}};
    for (const SwitchStatementFact& sw : context.control_flow.switch_statements) {
      if (!sw.has_default) {
        evaluation.findings.push_back(
            {"misra-c2012-16.4-missing-default", sw.location,
             FindingCertainty::Definite});
      }
    }
    return evaluation;
  }
};

}  // namespace

RulePtr make_rule_16_04() {
  return std::make_unique<Rule_16_04>();
}

}  // namespace misra::c2012
