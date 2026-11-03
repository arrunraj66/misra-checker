/*
 * MISRA C:2012 Rule 5.2 - Clang-backed detector
 *
 * Topic: Identifiers
 * Engineering intent: Independent checker contract for Rule 5.2 in the Identifiers family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Build scoped symbol, namespace, linkage, macro, and significant-character indexes across the configured analysis scope.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: Distinct identifiers (first 31 characters) declared in the same scope and name space are reported, including members of one aggregate.
 * Block scopes are tracked by the frontend; see docs/known-limits.md.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"
#include "misra/c2012/rule_support.hpp"

#include <memory>

namespace misra::c2012 {
namespace {

class Rule_05_02 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "5.2",
        RuleCategory::Required,
        Decidability::Decidable,
        AnalysisScope::SingleTranslationUnit,
        {true, true},
        "Identifiers",
        "Independent checker contract for Rule 5.2 in the Identifiers family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Build scoped symbol, namespace, linkage, macro, and significant-character indexes across the configured analysis scope.",
        ImplementationStatus::Implemented,
    };
    return descriptor;
  }

  [[nodiscard]] RuleEvaluation evaluate(
      const AnalysisContext& context) const override {
    return support::identifiers_not_distinct_in_scope(context);
  }
};

}  // namespace

RulePtr make_rule_05_02() {
  return std::make_unique<Rule_05_02>();
}

}  // namespace misra::c2012
