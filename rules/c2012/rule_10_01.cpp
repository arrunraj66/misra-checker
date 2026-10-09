/*
 * MISRA C:2012 Rule 10.1 - engineering scaffold
 *
 * Topic: Essential type model
 * Engineering intent: Independent checker contract for Rule 10.1 in the Essential type model family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Propagate essential types through expressions and verify ranks, conversions, operands, and composite-expression results.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: Subset: report Boolean or enumerated operands of arithmetic operators, unsigned unary minus, and non-unsigned or non-constant operands of bitwise and shift operators.
 * Evidence: AST/preprocessor observations recorded by the Clang adapter.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"

#include <memory>

#include "misra/observation_rule.hpp"

namespace misra::c2012 {
namespace {

class Rule_10_01 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "10.1",
        RuleCategory::Required,
        Decidability::Decidable,
        AnalysisScope::SingleTranslationUnit,
        {true, true},
        "Essential type model",
        "Independent checker contract for Rule 10.1 in the Essential type model family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Propagate essential types through expressions and verify ranks, conversions, operands, and composite-expression results.",
        ImplementationStatus::Implemented,
    };
    return descriptor;
  }

  [[nodiscard]] RuleEvaluation evaluate(
      const AnalysisContext& context) const override {
    return findings_from_observations(context, {"inappropriate-operand-type"},
                                      "misra-c2012-10.1-inappropriate-operand-type");
  }
};

}  // namespace

RulePtr make_rule_10_01() {
  return std::make_unique<Rule_10_01>();
}

}  // namespace misra::c2012
