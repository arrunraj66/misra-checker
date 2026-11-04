/*
 * MISRA C:2012 Rule 21.2 - Clang-backed detector
 *
 * Topic: Standard library usage
 * Engineering intent: Independent checker contract for Rule 21.2 in the Standard library usage family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Match headers, reserved identifiers, declarations, and calls against the configured controlled-library policy.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: A declaration of a reserved identifier (leading double underscore or underscore and capital anywhere, any leading underscore at file scope) is reported, as is a definition of a function that the standard library declares.
 * See docs/known-limits.md for interpretations.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"
#include "misra/c2012/rule_support.hpp"

#include <memory>

namespace misra::c2012 {
namespace {

class Rule_21_02 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "21.2",
        RuleCategory::Required,
        Decidability::Decidable,
        AnalysisScope::SingleTranslationUnit,
        {true, true},
        "Standard library usage",
        "Independent checker contract for Rule 21.2 in the Standard library usage family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Match headers, reserved identifiers, declarations, and calls against the configured controlled-library policy.",
        ImplementationStatus::Implemented,
    };
    return descriptor;
  }

  [[nodiscard]] RuleEvaluation evaluate(
      const AnalysisContext& context) const override {
    return support::merged(support::reserved_identifiers(context), support::syntax_findings(context, "misra-c2012-21.2-redefines-library"));
  }
};

}  // namespace

RulePtr make_rule_21_02() {
  return std::make_unique<Rule_21_02>();
}

}  // namespace misra::c2012
