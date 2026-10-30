/*
 * MISRA C:2012 Rule 7.3 - engineering scaffold
 *
 * Topic: Literals and constants
 * Engineering intent: Independent checker contract for Rule 7.3 in the Literals and constants family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Inspect literal spelling, suffixes, values, inferred types, escape forms, and string-literal use.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: Report an integer or floating literal whose suffix uses lowercase l.
 * Evidence: AST/preprocessor observations recorded by the Clang adapter.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"

#include <memory>

#include "misra/observation_rule.hpp"

namespace misra::c2012 {
namespace {

class Rule_07_03 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "7.3",
        RuleCategory::Required,
        Decidability::Decidable,
        AnalysisScope::SingleTranslationUnit,
        {true, true},
        "Literals and constants",
        "Independent checker contract for Rule 7.3 in the Literals and constants family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Inspect literal spelling, suffixes, values, inferred types, escape forms, and string-literal use.",
        ImplementationStatus::Implemented,
    };
    return descriptor;
  }

  [[nodiscard]] RuleEvaluation evaluate(
      const AnalysisContext& context) const override {
    return findings_from_observations(context, {"lowercase-l-suffix"},
                                      "misra-c2012-7.3-lowercase-l-suffix");
  }
};

}  // namespace

RulePtr make_rule_07_03() {
  return std::make_unique<Rule_07_03>();
}

}  // namespace misra::c2012
