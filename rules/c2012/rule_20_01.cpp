/*
 * MISRA C:2012 Rule 20.1 - engineering scaffold
 *
 * Topic: Preprocessing directives
 * Engineering intent: Independent checker contract for Rule 20.1 in the Preprocessing directives family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Inspect directives, macro definitions, arguments, expansions, token formation, and conditional-compilation structure.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: Report an include directive preceded by anything other than directives or comments.
 * Evidence: AST/preprocessor observations recorded by the Clang adapter.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"

#include <memory>

#include "misra/observation_rule.hpp"

namespace misra::c2012 {
namespace {

class Rule_20_01 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "20.1",
        RuleCategory::Advisory,
        Decidability::Decidable,
        AnalysisScope::SingleTranslationUnit,
        {true, true},
        "Preprocessing directives",
        "Independent checker contract for Rule 20.1 in the Preprocessing directives family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Inspect directives, macro definitions, arguments, expansions, token formation, and conditional-compilation structure.",
        ImplementationStatus::Implemented,
    };
    return descriptor;
  }

  [[nodiscard]] RuleEvaluation evaluate(
      const AnalysisContext& context) const override {
    return findings_from_observations(context, {"include-after-code"},
                                      "misra-c2012-20.1-include-after-code");
  }
};

}  // namespace

RulePtr make_rule_20_01() {
  return std::make_unique<Rule_20_01>();
}

}  // namespace misra::c2012
