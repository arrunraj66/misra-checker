/*
 * MISRA C:2012 Rule 20.10 - engineering scaffold
 *
 * Topic: Preprocessing directives
 * Engineering intent: Independent checker contract for Rule 20.10 in the Preprocessing directives family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Inspect directives, macro definitions, arguments, expansions, token formation, and conditional-compilation structure.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Implementation status: Scaffold only; no compliance decision is made.
 */

#include "misra/c2012/rule_factories.hpp"

#include <memory>

namespace misra::c2012 {
namespace {

class Rule_20_10 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "20.10",
        RuleCategory::Advisory,
        Decidability::Decidable,
        AnalysisScope::SingleTranslationUnit,
        {true, true},
        "Preprocessing directives",
        "Independent checker contract for Rule 20.10 in the Preprocessing directives family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Inspect directives, macro definitions, arguments, expansions, token formation, and conditional-compilation structure.",
        ImplementationStatus::Scaffold,
    };
    return descriptor;
  }

  [[nodiscard]] RuleEvaluation evaluate(
      const AnalysisContext& context) const override {
    (void)context;
    return {EvaluationStatus::NotImplemented};
  }
};

}  // namespace

RulePtr make_rule_20_10() {
  return std::make_unique<Rule_20_10>();
}

}  // namespace misra::c2012
