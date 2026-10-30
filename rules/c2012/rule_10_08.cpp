/*
 * MISRA C:2012 Rule 10.8 - engineering scaffold
 *
 * Topic: Essential type model
 * Engineering intent: Independent checker contract for Rule 10.8 in the Essential type model family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Propagate essential types through expressions and verify ranks, conversions, operands, and composite-expression results.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: Report a cast of a composite expression to a wider or different essential type.
 * Evidence: AST, preprocessor, diagnostic and control-flow observations recorded by the Clang adapter.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"

#include <memory>

#include "misra/observation_rule.hpp"

namespace misra::c2012 {
namespace {

class Rule_10_08 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "10.8",
        RuleCategory::Required,
        Decidability::Decidable,
        AnalysisScope::SingleTranslationUnit,
        {true, true},
        "Essential type model",
        "Independent checker contract for Rule 10.8 in the Essential type model family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Propagate essential types through expressions and verify ranks, conversions, operands, and composite-expression results.",
        ImplementationStatus::Implemented,
    };
    return descriptor;
  }

  [[nodiscard]] RuleEvaluation evaluate(
      const AnalysisContext& context) const override {
    return findings_from_observations(context, {"composite-cast-widened"},
                                      "misra-c2012-10.8-composite-cast-widened", {}, {});
  }
};

}  // namespace

RulePtr make_rule_10_08() {
  return std::make_unique<Rule_10_08>();
}

}  // namespace misra::c2012
