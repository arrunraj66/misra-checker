/*
 * MISRA C:2012 Rule 5.6 - engineering scaffold
 *
 * Topic: Identifiers
 * Engineering intent: Independent checker contract for Rule 5.6 in the Identifiers family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Build scoped symbol, namespace, linkage, macro, and significant-character indexes across the configured analysis scope.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: Report a typedef whose name is reused by another declaration (a tag naming the same type is accepted).
 * Evidence: AST/preprocessor observations recorded by the Clang adapter.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"

#include <memory>

#include "misra/observation_rule.hpp"

namespace misra::c2012 {
namespace {

class Rule_05_06 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "5.6",
        RuleCategory::Required,
        Decidability::Decidable,
        AnalysisScope::System,
        {true, true},
        "Identifiers",
        "Independent checker contract for Rule 5.6 in the Identifiers family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Build scoped symbol, namespace, linkage, macro, and significant-character indexes across the configured analysis scope.",
        ImplementationStatus::Implemented,
    };
    return descriptor;
  }

  [[nodiscard]] RuleEvaluation evaluate(
      const AnalysisContext& context) const override {
    return findings_from_observations(context, {"typedef-name-not-unique"},
                                      "misra-c2012-5.6-typedef-name-not-unique");
  }
};

}  // namespace

RulePtr make_rule_05_06() {
  return std::make_unique<Rule_05_06>();
}

}  // namespace misra::c2012
