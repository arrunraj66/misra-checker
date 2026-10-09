/*
 * MISRA C:2012 Rule 17.7 - engineering scaffold
 *
 * Topic: Functions
 * Engineering intent: Independent checker contract for Rule 17.7 in the Functions family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Resolve calls and prototypes, construct a call graph, inspect recursion, parameters, arrays, and return-value use.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: Report a call to a non-void function used as an expression statement (a cast to void is accepted).
 * Evidence: AST/preprocessor observations recorded by the Clang adapter.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"

#include <memory>

#include "misra/observation_rule.hpp"

namespace misra::c2012 {
namespace {

class Rule_17_07 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "17.7",
        RuleCategory::Required,
        Decidability::Decidable,
        AnalysisScope::SingleTranslationUnit,
        {true, true},
        "Functions",
        "Independent checker contract for Rule 17.7 in the Functions family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Resolve calls and prototypes, construct a call graph, inspect recursion, parameters, arrays, and return-value use.",
        ImplementationStatus::Implemented,
    };
    return descriptor;
  }

  [[nodiscard]] RuleEvaluation evaluate(
      const AnalysisContext& context) const override {
    return findings_from_observations(context, {"unused-call-result"},
                                      "misra-c2012-17.7-unused-call-result");
  }
};

}  // namespace

RulePtr make_rule_17_07() {
  return std::make_unique<Rule_17_07>();
}

}  // namespace misra::c2012
