/*
 * MISRA C:2012 Rule 18.7 - engineering scaffold
 *
 * Topic: Pointers and arrays
 * Engineering intent: Independent checker contract for Rule 18.7 in the Pointers and arrays family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Track pointer provenance, array bounds, arithmetic, comparisons, object lifetime, variable-length arrays, and flexible members.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: Report a structure whose last member is an array of unspecified size.
 * Evidence: AST/preprocessor observations recorded by the Clang adapter.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"

#include <memory>

#include "misra/observation_rule.hpp"

namespace misra::c2012 {
namespace {

class Rule_18_07 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "18.7",
        RuleCategory::Required,
        Decidability::Decidable,
        AnalysisScope::SingleTranslationUnit,
        {false, true},
        "Pointers and arrays",
        "Independent checker contract for Rule 18.7 in the Pointers and arrays family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Track pointer provenance, array bounds, arithmetic, comparisons, object lifetime, variable-length arrays, and flexible members.",
        ImplementationStatus::Implemented,
    };
    return descriptor;
  }

  [[nodiscard]] RuleEvaluation evaluate(
      const AnalysisContext& context) const override {
    return findings_from_observations(context, {"flexible-array-member"},
                                      "misra-c2012-18.7-flexible-array-member");
  }
};

}  // namespace

RulePtr make_rule_18_07() {
  return std::make_unique<Rule_18_07>();
}

}  // namespace misra::c2012
