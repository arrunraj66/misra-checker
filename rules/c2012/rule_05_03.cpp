/*
 * MISRA C:2012 Rule 5.3 - Clang-backed detector
 *
 * Topic: Identifiers
 * Engineering intent: Independent checker contract for Rule 5.3 in the Identifiers family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Build scoped symbol, namespace, linkage, macro, and significant-character indexes across the configured analysis scope.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: A declaration whose first 31 characters coincide with an earlier declaration in an enclosing scope (same name space) is reported. Identifiers from system headers are not modelled.
 * Block scopes are tracked by the frontend; see docs/known-limits.md.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"
#include "misra/c2012/rule_support.hpp"

#include <memory>

namespace misra::c2012 {
namespace {

class Rule_05_03 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "5.3",
        RuleCategory::Required,
        Decidability::Decidable,
        AnalysisScope::SingleTranslationUnit,
        {true, true},
        "Identifiers",
        "Independent checker contract for Rule 5.3 in the Identifiers family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Build scoped symbol, namespace, linkage, macro, and significant-character indexes across the configured analysis scope.",
        ImplementationStatus::Implemented,
    };
    return descriptor;
  }

  [[nodiscard]] RuleEvaluation evaluate(
      const AnalysisContext& context) const override {
    return support::identifier_hides_outer(context);
  }
};

}  // namespace

RulePtr make_rule_05_03() {
  return std::make_unique<Rule_05_03>();
}

}  // namespace misra::c2012
