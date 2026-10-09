/*
 * MISRA C:2012 Rule 5.5 - engineering scaffold
 *
 * Topic: Identifiers
 * Engineering intent: Independent checker contract for Rule 5.5 in the Identifiers family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Build scoped symbol, namespace, linkage, macro, and significant-character indexes across the configured analysis scope.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: Report a declared identifier that has the same name as a macro defined in the main file.
 * Evidence: AST/preprocessor observations recorded by the Clang adapter.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"

#include <memory>

#include "misra/observation_rule.hpp"

namespace misra::c2012 {
namespace {

class Rule_05_05 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "5.5",
        RuleCategory::Required,
        Decidability::Decidable,
        AnalysisScope::SingleTranslationUnit,
        {true, true},
        "Identifiers",
        "Independent checker contract for Rule 5.5 in the Identifiers family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Build scoped symbol, namespace, linkage, macro, and significant-character indexes across the configured analysis scope.",
        ImplementationStatus::Implemented,
    };
    return descriptor;
  }

  [[nodiscard]] RuleEvaluation evaluate(
      const AnalysisContext& context) const override {
    return findings_from_observations(context, {"identifier-matches-macro-name"},
                                      "misra-c2012-5.5-identifier-matches-macro-name");
  }
};

}  // namespace

RulePtr make_rule_05_05() {
  return std::make_unique<Rule_05_05>();
}

}  // namespace misra::c2012
