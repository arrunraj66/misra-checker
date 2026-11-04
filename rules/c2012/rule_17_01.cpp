/*
 * MISRA C:2012 Rule 17.1 - Clang-backed detector
 *
 * Topic: Functions
 * Engineering intent: Independent checker contract for Rule 17.1 in the Functions family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Resolve calls and prototypes, construct a call graph, inspect recursion, parameters, arrays, and return-value use.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: Use of <stdarg.h> (the include, or a va_arg expression) is reported.
 * See docs/known-limits.md for interpretations.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"
#include "misra/c2012/rule_support.hpp"

#include <memory>

namespace misra::c2012 {
namespace {

class Rule_17_01 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "17.1",
        RuleCategory::Required,
        Decidability::Decidable,
        AnalysisScope::SingleTranslationUnit,
        {true, true},
        "Functions",
        "Independent checker contract for Rule 17.1 in the Functions family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Resolve calls and prototypes, construct a call graph, inspect recursion, parameters, arrays, and return-value use.",
        ImplementationStatus::Implemented,
    };
    return descriptor;
  }

  [[nodiscard]] RuleEvaluation evaluate(
      const AnalysisContext& context) const override {
    return support::merged(support::include_findings(context, {"stdarg.h"}, "misra-c2012-17.1-stdarg"), support::syntax_findings(context, "misra-c2012-17.1-stdarg"));
  }
};

}  // namespace

RulePtr make_rule_17_01() {
  return std::make_unique<Rule_17_01>();
}

}  // namespace misra::c2012
