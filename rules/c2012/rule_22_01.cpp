/*
 * MISRA C:2012 Rule 22.1 - engineering scaffold
 *
 * Topic: Resource management
 * Engineering intent: Independent checker contract for Rule 22.1 in the Resource management family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Track acquisition, ownership, state transitions, release, files, streams, and exceptional control-flow paths.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: Subset: report a local object holding an allocation or open stream that is neither released nor handed on in the function.
 * Evidence: AST, preprocessor, diagnostic and control-flow observations recorded by the Clang adapter.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"

#include <memory>

#include "misra/observation_rule.hpp"

namespace misra::c2012 {
namespace {

class Rule_22_01 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "22.1",
        RuleCategory::Required,
        Decidability::Undecidable,
        AnalysisScope::System,
        {true, true},
        "Resource management",
        "Independent checker contract for Rule 22.1 in the Resource management family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Track acquisition, ownership, state transitions, release, files, streams, and exceptional control-flow paths.",
        ImplementationStatus::Implemented,
    };
    return descriptor;
  }

  [[nodiscard]] RuleEvaluation evaluate(
      const AnalysisContext& context) const override {
    return findings_from_observations(context, {"resource-not-released"},
                                      "misra-c2012-22.1-resource-not-released", {}, {"resource-not-released"});
  }
};

}  // namespace

RulePtr make_rule_22_01() {
  return std::make_unique<Rule_22_01>();
}

}  // namespace misra::c2012
