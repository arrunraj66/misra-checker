/*
 * MISRA C:2012 Rule 5.7 - Clang-backed detector
 *
 * Topic: Identifiers
 * Engineering intent: Independent checker contract for Rule 5.7 in the Identifiers family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Build scoped symbol, namespace, linkage, macro, and significant-character indexes across the configured analysis scope.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: Tag names defined at more than one distinct location are reported. Whole-program rule: results are valid only when the compilation
 * database covers the complete program.
 * Findings produced by macro expansion are tagged; see docs/macro-policy.md.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"
#include "misra/c2012/rule_support.hpp"

#include <memory>

namespace misra::c2012 {
namespace {

class Rule_05_07 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "5.7",
        RuleCategory::Required,
        Decidability::Decidable,
        AnalysisScope::System,
        {true, true},
        "Identifiers",
        "Independent checker contract for Rule 5.7 in the Identifiers family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Build scoped symbol, namespace, linkage, macro, and significant-character indexes across the configured analysis scope.",
        ImplementationStatus::Implemented,
    };
    return descriptor;
  }

  [[nodiscard]] RuleEvaluation evaluate(
      const AnalysisContext& context) const override {
    return support::duplicate_tag_names(context);
  }
};

}  // namespace

RulePtr make_rule_05_07() {
  return std::make_unique<Rule_05_07>();
}

}  // namespace misra::c2012
