/*
 * MISRA C:2012 Rule 20.6 - engineering scaffold
 *
 * Topic: Preprocessing directives
 * Engineering intent: Independent checker contract for Rule 20.6 in the Preprocessing directives family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Inspect directives, macro definitions, arguments, expansions, token formation, and conditional-compilation structure.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: Report a preprocessing directive embedded in macro arguments (reported by the compiler as an extension warning).
 * Evidence: AST, preprocessor, diagnostic and control-flow observations recorded by the Clang adapter.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"

#include <memory>

#include "misra/observation_rule.hpp"

namespace misra::c2012 {
namespace {

class Rule_20_06 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "20.6",
        RuleCategory::Required,
        Decidability::Decidable,
        AnalysisScope::SingleTranslationUnit,
        {true, true},
        "Preprocessing directives",
        "Independent checker contract for Rule 20.6 in the Preprocessing directives family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Inspect directives, macro definitions, arguments, expansions, token formation, and conditional-compilation structure.",
        ImplementationStatus::Implemented,
    };
    return descriptor;
  }

  [[nodiscard]] RuleEvaluation evaluate(
      const AnalysisContext& context) const override {
    return findings_from_observations(context, {"embedded-directive"},
                                      "misra-c2012-20.6-embedded-directive", {}, {});
  }
};

}  // namespace

RulePtr make_rule_20_06() {
  return std::make_unique<Rule_20_06>();
}

}  // namespace misra::c2012
