/*
 * MISRA C:2012 Rule 15.6 - engineering scaffold
 *
 * Topic: Control flow
 * Engineering intent: Independent checker contract for Rule 15.6 in the Control flow family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Build the control-flow graph and inspect jumps, loop exits, compound statements, and function exit structure.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Implementation status: Scaffold only; no compliance decision is made.
 */

#include "misra/c2012/rule_factories.hpp"

#include <memory>

namespace misra::c2012 {
namespace {

class Rule_15_06 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "15.6",
        RuleCategory::Required,
        Decidability::Decidable,
        AnalysisScope::SingleTranslationUnit,
        {true, true},
        "Control flow",
        "Independent checker contract for Rule 15.6 in the Control flow family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Build the control-flow graph and inspect jumps, loop exits, compound statements, and function exit structure.",
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

RulePtr make_rule_15_06() {
  return std::make_unique<Rule_15_06>();
}

}  // namespace misra::c2012
