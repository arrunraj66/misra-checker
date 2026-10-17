/*
 * MISRA C:2012 Rule 6.2 - engineering scaffold
 *
 * Topic: Types and bit-fields
 * Engineering intent: Independent checker contract for Rule 6.2 in the Types and bit-fields family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Model declared types, bit-field properties, signedness, widths, and target implementation characteristics.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: Report a named bit-field of signed integer type with width one.
 * Evidence: AST/preprocessor observations recorded by the Clang adapter.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"

#include <memory>

#include "misra/observation_rule.hpp"

namespace misra::c2012 {
namespace {

class Rule_06_02 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "6.2",
        RuleCategory::Required,
        Decidability::Decidable,
        AnalysisScope::SingleTranslationUnit,
        {true, true},
        "Types and bit-fields",
        "Independent checker contract for Rule 6.2 in the Types and bit-fields family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Model declared types, bit-field properties, signedness, widths, and target implementation characteristics.",
        ImplementationStatus::Implemented,
    };
    return descriptor;
  }

  [[nodiscard]] RuleEvaluation evaluate(
      const AnalysisContext& context) const override {
    return findings_from_observations(context, {"signed-single-bit-field"},
                                      "misra-c2012-6.2-signed-single-bit-field");
  }
};

}  // namespace

RulePtr make_rule_06_02() {
  return std::make_unique<Rule_06_02>();
}

}  // namespace misra::c2012
