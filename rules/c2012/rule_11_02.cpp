/*
 * MISRA C:2012 Rule 11.2 - engineering scaffold
 *
 * Topic: Pointer conversions
 * Engineering intent: Independent checker contract for Rule 11.2 in the Pointer conversions family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Classify source and destination pointer categories, qualifiers, object/function types, and null pointer constants.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: Report a conversion between a pointer to an incomplete type and any other pointer or integer type.
 * Evidence: AST/preprocessor observations recorded by the Clang adapter.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"

#include <memory>

#include "misra/observation_rule.hpp"

namespace misra::c2012 {
namespace {

class Rule_11_02 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "11.2",
        RuleCategory::Required,
        Decidability::Decidable,
        AnalysisScope::SingleTranslationUnit,
        {true, true},
        "Pointer conversions",
        "Independent checker contract for Rule 11.2 in the Pointer conversions family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Classify source and destination pointer categories, qualifiers, object/function types, and null pointer constants.",
        ImplementationStatus::Implemented,
    };
    return descriptor;
  }

  [[nodiscard]] RuleEvaluation evaluate(
      const AnalysisContext& context) const override {
    return findings_from_observations(context, {"incomplete-pointer-conversion"},
                                      "misra-c2012-11.2-incomplete-pointer-conversion");
  }
};

}  // namespace

RulePtr make_rule_11_02() {
  return std::make_unique<Rule_11_02>();
}

}  // namespace misra::c2012
