/*
 * MISRA C:2012 Rule 8.7 - engineering scaffold
 *
 * Topic: Declarations and definitions
 * Engineering intent: Independent checker contract for Rule 8.7 in the Declarations and definitions family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Reconcile declarations, definitions, linkage, prototypes, qualifiers, and cross-translation-unit symbol records.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Implementation status: Scaffold only; no compliance decision is made.
 */

#include "misra/c2012/rule_factories.hpp"

#include <memory>

namespace misra::c2012 {
namespace {

class Rule_08_07 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "8.7",
        RuleCategory::Advisory,
        Decidability::Decidable,
        AnalysisScope::System,
        {true, true},
        "Declarations and definitions",
        "Independent checker contract for Rule 8.7 in the Declarations and definitions family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Reconcile declarations, definitions, linkage, prototypes, qualifiers, and cross-translation-unit symbol records.",
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

RulePtr make_rule_08_07() {
  return std::make_unique<Rule_08_07>();
}

}  // namespace misra::c2012
