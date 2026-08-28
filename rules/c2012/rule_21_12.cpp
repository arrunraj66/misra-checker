/*
 * MISRA C:2012 Rule 21.12 - engineering scaffold
 *
 * Topic: Standard library usage
 * Engineering intent: Independent checker contract for Rule 21.12 in the Standard library usage family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Match headers, reserved identifiers, declarations, and calls against the configured controlled-library policy.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Implementation status: Scaffold only; no compliance decision is made.
 */

#include "misra/c2012/rule_factories.hpp"

#include <memory>

namespace misra::c2012 {
namespace {

class Rule_21_12 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "21.12",
        RuleCategory::Advisory,
        Decidability::Decidable,
        AnalysisScope::SingleTranslationUnit,
        {false, true},
        "Standard library usage",
        "Independent checker contract for Rule 21.12 in the Standard library usage family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Match headers, reserved identifiers, declarations, and calls against the configured controlled-library policy.",
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

RulePtr make_rule_21_12() {
  return std::make_unique<Rule_21_12>();
}

}  // namespace misra::c2012
