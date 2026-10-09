/*
 * MISRA C:2012 Rule 12.1 - engineering scaffold
 *
 * Topic: Expressions
 * Engineering intent: Independent checker contract for Rule 12.1 in the Expressions family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Analyze expression trees for precedence, operand ranges, operator constraints, array use, and target-dependent behavior.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: Strict reading: report a binary operator with an unparenthesized binary operand of a different precedence level.
 * Evidence: AST, preprocessor, diagnostic and control-flow observations recorded by the Clang adapter.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"

#include <memory>

#include "misra/observation_rule.hpp"

namespace misra::c2012 {
namespace {

class Rule_12_01 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "12.1",
        RuleCategory::Advisory,
        Decidability::Decidable,
        AnalysisScope::SingleTranslationUnit,
        {true, true},
        "Expressions",
        "Independent checker contract for Rule 12.1 in the Expressions family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Analyze expression trees for precedence, operand ranges, operator constraints, array use, and target-dependent behavior.",
        ImplementationStatus::Implemented,
    };
    return descriptor;
  }

  [[nodiscard]] RuleEvaluation evaluate(
      const AnalysisContext& context) const override {
    return findings_from_observations(context, {"implicit-precedence"},
                                      "misra-c2012-12.1-implicit-precedence", {}, {});
  }
};

}  // namespace

RulePtr make_rule_12_01() {
  return std::make_unique<Rule_12_01>();
}

}  // namespace misra::c2012
