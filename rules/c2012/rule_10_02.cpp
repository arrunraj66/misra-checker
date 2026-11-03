/*
 * MISRA C:2012 Rule 10.2 - Clang-backed detector
 *
 * Topic: Essential type model
 * Engineering intent: Independent checker contract for Rule 10.2 in the Essential type model family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Propagate essential types through expressions and verify ranks, conversions, operands, and composite-expression results.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: char + char, integer - char and other inappropriate character operands of addition or subtraction are reported.
 * Uses the essential type model in src/essential_type.hpp; see docs/known-limits.md.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"
#include "misra/c2012/rule_support.hpp"

#include <memory>

namespace misra::c2012 {
namespace {

class Rule_10_02 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "10.2",
        RuleCategory::Required,
        Decidability::Decidable,
        AnalysisScope::SingleTranslationUnit,
        {true, true},
        "Essential type model",
        "Independent checker contract for Rule 10.2 in the Essential type model family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Propagate essential types through expressions and verify ranks, conversions, operands, and composite-expression results.",
        ImplementationStatus::Implemented,
    };
    return descriptor;
  }

  [[nodiscard]] RuleEvaluation evaluate(
      const AnalysisContext& context) const override {
    return support::syntax_findings(context, "misra-c2012-10.2-character-arithmetic");
  }
};

}  // namespace

RulePtr make_rule_10_02() {
  return std::make_unique<Rule_10_02>();
}

}  // namespace misra::c2012
