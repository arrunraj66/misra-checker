/*
 * MISRA C:2012 Rule 10.3 - Clang-backed detector
 *
 * Topic: Essential type model
 * Engineering intent: Independent checker contract for Rule 10.3 in the Essential type model family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Propagate essential types through expressions and verify ranks, conversions, operands, and composite-expression results.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: Assignment, initialization, argument passing or return of a value to a narrower type or a different essential category is reported; non-negative signed constants may initialize unsigned objects when they fit.
 * Uses the essential type model in src/essential_type.hpp; see docs/known-limits.md.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"
#include "misra/c2012/rule_support.hpp"

#include <memory>

namespace misra::c2012 {
namespace {

class Rule_10_03 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "10.3",
        RuleCategory::Required,
        Decidability::Decidable,
        AnalysisScope::SingleTranslationUnit,
        {true, true},
        "Essential type model",
        "Independent checker contract for Rule 10.3 in the Essential type model family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Propagate essential types through expressions and verify ranks, conversions, operands, and composite-expression results.",
        ImplementationStatus::Implemented,
    };
    return descriptor;
  }

  [[nodiscard]] RuleEvaluation evaluate(
      const AnalysisContext& context) const override {
    return support::syntax_findings(context, "misra-c2012-10.3-assignment-category-or-width");
  }
};

}  // namespace

RulePtr make_rule_10_03() {
  return std::make_unique<Rule_10_03>();
}

}  // namespace misra::c2012
