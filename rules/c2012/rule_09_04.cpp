/*
 * MISRA C:2012 Rule 9.4 - engineering scaffold
 *
 * Topic: Initialization
 * Engineering intent: Independent checker contract for Rule 9.4 in the Initialization family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Model object initialization state, aggregate coverage, initializer shape, designators, and storage duration.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: Report an element designated more than once in one initializer list.
 * Evidence: AST/preprocessor observations recorded by the Clang adapter.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"

#include <memory>

#include "misra/observation_rule.hpp"

namespace misra::c2012 {
namespace {

class Rule_09_04 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "9.4",
        RuleCategory::Required,
        Decidability::Decidable,
        AnalysisScope::SingleTranslationUnit,
        {false, true},
        "Initialization",
        "Independent checker contract for Rule 9.4 in the Initialization family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Model object initialization state, aggregate coverage, initializer shape, designators, and storage duration.",
        ImplementationStatus::Implemented,
    };
    return descriptor;
  }

  [[nodiscard]] RuleEvaluation evaluate(
      const AnalysisContext& context) const override {
    return findings_from_observations(context, {"duplicate-initialization"},
                                      "misra-c2012-9.4-duplicate-initialization");
  }
};

}  // namespace

RulePtr make_rule_09_04() {
  return std::make_unique<Rule_09_04>();
}

}  // namespace misra::c2012
