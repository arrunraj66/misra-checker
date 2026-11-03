/*
 * MISRA C:2012 Rule 8.12 - Clang-backed detector
 *
 * Topic: Declarations and definitions
 * Engineering intent: Independent checker contract for Rule 8.12 in the Declarations and definitions family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Reconcile declarations, definitions, linkage, prototypes, qualifiers, and cross-translation-unit symbol records.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: Implicitly valued enumerators sharing a value with another enumerator are reported.
 * Findings produced by macro expansion are tagged; see docs/macro-policy.md.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"
#include "misra/c2012/rule_support.hpp"

#include <memory>

namespace misra::c2012 {
namespace {

class Rule_08_12 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "8.12",
        RuleCategory::Required,
        Decidability::Decidable,
        AnalysisScope::SingleTranslationUnit,
        {true, true},
        "Declarations and definitions",
        "Independent checker contract for Rule 8.12 in the Declarations and definitions family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Reconcile declarations, definitions, linkage, prototypes, qualifiers, and cross-translation-unit symbol records.",
        ImplementationStatus::Implemented,
    };
    return descriptor;
  }

  [[nodiscard]] RuleEvaluation evaluate(
      const AnalysisContext& context) const override {
    return support::syntax_findings(context, "misra-c2012-8.12-implicit-enumerator-duplicate");
  }
};

}  // namespace

RulePtr make_rule_08_12() {
  return std::make_unique<Rule_08_12>();
}

}  // namespace misra::c2012
