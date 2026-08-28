/*
 * MISRA C:2012 Rule 18.4 - engineering scaffold
 *
 * Topic: Pointers and arrays
 * Engineering intent: Independent checker contract for Rule 18.4 in the Pointers and arrays family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Track pointer provenance, array bounds, arithmetic, comparisons, object lifetime, variable-length arrays, and flexible members.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Implementation status: Scaffold only; no compliance decision is made.
 */

#include "misra/c2012/rule_factories.hpp"

#include <memory>

namespace misra::c2012 {
namespace {

class Rule_18_04 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "18.4",
        RuleCategory::Advisory,
        Decidability::Decidable,
        AnalysisScope::SingleTranslationUnit,
        {true, true},
        "Pointers and arrays",
        "Independent checker contract for Rule 18.4 in the Pointers and arrays family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Track pointer provenance, array bounds, arithmetic, comparisons, object lifetime, variable-length arrays, and flexible members.",
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

RulePtr make_rule_18_04() {
  return std::make_unique<Rule_18_04>();
}

}  // namespace misra::c2012
