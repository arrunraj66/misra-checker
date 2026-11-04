/*
 * MISRA C:2012 Rule 7.4 - Clang-backed detector
 *
 * Topic: Literals and constants
 * Engineering intent: Independent checker contract for Rule 7.4 in the Literals and constants family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Inspect literal spelling, suffixes, values, inferred types, escape forms, and string-literal use.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: A string literal assigned, initialized or returned to a pointer to non-const character type is reported.
 * See docs/known-limits.md for interpretations.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"
#include "misra/c2012/rule_support.hpp"

#include <memory>

namespace misra::c2012 {
namespace {

class Rule_07_04 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "7.4",
        RuleCategory::Required,
        Decidability::Decidable,
        AnalysisScope::SingleTranslationUnit,
        {true, true},
        "Literals and constants",
        "Independent checker contract for Rule 7.4 in the Literals and constants family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Inspect literal spelling, suffixes, values, inferred types, escape forms, and string-literal use.",
        ImplementationStatus::Implemented,
    };
    return descriptor;
  }

  [[nodiscard]] RuleEvaluation evaluate(
      const AnalysisContext& context) const override {
    return support::syntax_findings(context, "misra-c2012-7.4-string-literal-to-non-const");
  }
};

}  // namespace

RulePtr make_rule_07_04() {
  return std::make_unique<Rule_07_04>();
}

}  // namespace misra::c2012
