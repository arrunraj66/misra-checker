/*
 * MISRA C:2012 Rule 21.5 - engineering scaffold
 *
 * Topic: Standard library usage
 * Engineering intent: Independent checker contract for Rule 21.5 in the Standard library usage family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Match headers, reserved identifiers, declarations, and calls against the configured controlled-library policy.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: Report a call to one of the listed functions: signal handling function called (matched by callee name).
 * Evidence: AST/preprocessor observations recorded by the Clang adapter.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"

#include <memory>

#include "misra/observation_rule.hpp"

namespace misra::c2012 {
namespace {

class Rule_21_05 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "21.5",
        RuleCategory::Required,
        Decidability::Decidable,
        AnalysisScope::SingleTranslationUnit,
        {true, true},
        "Standard library usage",
        "Independent checker contract for Rule 21.5 in the Standard library usage family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Match headers, reserved identifiers, declarations, and calls against the configured controlled-library policy.",
        ImplementationStatus::Implemented,
    };
    return descriptor;
  }

  [[nodiscard]] RuleEvaluation evaluate(
      const AnalysisContext& context) const override {
    return findings_from_observations(context, {"call"},
                                      "misra-c2012-21.5-banned-function", {"signal", "raise"});
  }
};

}  // namespace

RulePtr make_rule_21_05() {
  return std::make_unique<Rule_21_05>();
}

}  // namespace misra::c2012
