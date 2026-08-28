/*
 * MISRA C:2012 Rule 19.2 - engineering scaffold
 *
 * Topic: Overlapping storage
 * Engineering intent: Independent checker contract for Rule 19.2 in the Overlapping storage family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Model unions, aliases, object representations, and potentially overlapping storage regions.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Implementation status: Scaffold only; no compliance decision is made.
 */

#include "misra/c2012/rule_factories.hpp"

#include <memory>

namespace misra::c2012 {
namespace {

class Rule_19_02 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "19.2",
        RuleCategory::Advisory,
        Decidability::Decidable,
        AnalysisScope::SingleTranslationUnit,
        {true, true},
        "Overlapping storage",
        "Independent checker contract for Rule 19.2 in the Overlapping storage family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Model unions, aliases, object representations, and potentially overlapping storage regions.",
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

RulePtr make_rule_19_02() {
  return std::make_unique<Rule_19_02>();
}

}  // namespace misra::c2012
