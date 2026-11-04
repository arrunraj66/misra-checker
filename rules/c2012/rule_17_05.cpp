/*
 * MISRA C:2012 Rule 17.5 - Clang-backed detector
 *
 * Topic: Functions
 * Engineering intent: Independent checker contract for Rule 17.5 in the Functions family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Resolve calls and prototypes, construct a call graph, inspect recursion, parameters, arrays, and return-value use.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: An array argument smaller than the declared bound of an array parameter is reported; arguments that are not arrays of known size cannot be judged.
 * See docs/known-limits.md for interpretations.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"
#include "misra/c2012/rule_support.hpp"

#include <memory>

namespace misra::c2012 {
namespace {

class Rule_17_05 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "17.5",
        RuleCategory::Advisory,
        Decidability::Undecidable,
        AnalysisScope::System,
        {true, true},
        "Functions",
        "Independent checker contract for Rule 17.5 in the Functions family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Resolve calls and prototypes, construct a call graph, inspect recursion, parameters, arrays, and return-value use.",
        ImplementationStatus::Implemented,
    };
    return descriptor;
  }

  [[nodiscard]] RuleEvaluation evaluate(
      const AnalysisContext& context) const override {
    return support::syntax_findings(context, "misra-c2012-17.5-array-argument-too-small");
  }
};

}  // namespace

RulePtr make_rule_17_05() {
  return std::make_unique<Rule_17_05>();
}

}  // namespace misra::c2012
