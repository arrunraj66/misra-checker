/*
 * MISRA C:2012 Rule 8.13 - engineering scaffold
 *
 * Topic: Declarations and definitions
 * Engineering intent: Independent checker contract for Rule 8.13 in the Declarations and definitions family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Reconcile declarations, definitions, linkage, prototypes, qualifiers, and cross-translation-unit symbol records.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: Subset: report a pointer parameter to non-const whose every use in the function is provably read-only.
 * Evidence: AST, preprocessor, diagnostic and control-flow observations recorded by the Clang adapter.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"

#include <memory>

#include "misra/observation_rule.hpp"

namespace misra::c2012 {
namespace {

class Rule_08_13 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "8.13",
        RuleCategory::Advisory,
        Decidability::Undecidable,
        AnalysisScope::System,
        {true, true},
        "Declarations and definitions",
        "Independent checker contract for Rule 8.13 in the Declarations and definitions family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Reconcile declarations, definitions, linkage, prototypes, qualifiers, and cross-translation-unit symbol records.",
        ImplementationStatus::Implemented,
    };
    return descriptor;
  }

  [[nodiscard]] RuleEvaluation evaluate(
      const AnalysisContext& context) const override {
    return findings_from_observations(context, {"pointer-could-be-const"},
                                      "misra-c2012-8.13-pointer-could-be-const", {}, {"pointer-could-be-const"});
  }
};

}  // namespace

RulePtr make_rule_08_13() {
  return std::make_unique<Rule_08_13>();
}

}  // namespace misra::c2012
