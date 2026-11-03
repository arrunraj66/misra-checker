/*
 * MISRA C:2012 Rule 7.1 - Clang-backed detector
 *
 * Topic: Literals and constants
 * Engineering intent: Independent checker contract for Rule 7.1 in the Literals and constants family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Inspect literal spelling, suffixes, values, inferred types, escape forms, and string-literal use.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: Integer constants spelled with a leading 0 followed by digits are octal and are reported.
 * Findings produced by macro expansion are tagged; see docs/macro-policy.md.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"
#include "misra/c2012/rule_support.hpp"

#include <memory>

namespace misra::c2012 {
namespace {

class Rule_07_01 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "7.1",
        RuleCategory::Required,
        Decidability::Decidable,
        AnalysisScope::SingleTranslationUnit,
        {true, true},
        "Literals and constants",
        "Independent checker contract for Rule 7.1 in the Literals and constants family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Inspect literal spelling, suffixes, values, inferred types, escape forms, and string-literal use.",
        ImplementationStatus::Implemented,
    };
    return descriptor;
  }

  [[nodiscard]] RuleEvaluation evaluate(
      const AnalysisContext& context) const override {
    return support::syntax_findings(context, "misra-c2012-7.1-octal-constant");
  }
};

}  // namespace

RulePtr make_rule_07_01() {
  return std::make_unique<Rule_07_01>();
}

}  // namespace misra::c2012
