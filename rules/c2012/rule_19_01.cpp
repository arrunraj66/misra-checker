/*
 * MISRA C:2012 Rule 19.1 - engineering scaffold
 *
 * Topic: Overlapping storage
 * Engineering intent: Independent checker contract for Rule 19.1 in the Overlapping storage family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Model unions, aliases, object representations, and potentially overlapping storage regions.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: Subset: report self-assignment and copy calls whose source and destination are the same expression.
 * Evidence: AST, preprocessor, diagnostic and control-flow observations recorded by the Clang adapter.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"

#include <memory>

#include "misra/observation_rule.hpp"

namespace misra::c2012 {
namespace {

class Rule_19_01 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "19.1",
        RuleCategory::Mandatory,
        Decidability::Undecidable,
        AnalysisScope::System,
        {true, true},
        "Overlapping storage",
        "Independent checker contract for Rule 19.1 in the Overlapping storage family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Model unions, aliases, object representations, and potentially overlapping storage regions.",
        ImplementationStatus::Implemented,
    };
    return descriptor;
  }

  [[nodiscard]] RuleEvaluation evaluate(
      const AnalysisContext& context) const override {
    return findings_from_observations(context, {"overlapping-copy"},
                                      "misra-c2012-19.1-overlapping-copy", {}, {});
  }
};

}  // namespace

RulePtr make_rule_19_01() {
  return std::make_unique<Rule_19_01>();
}

}  // namespace misra::c2012
