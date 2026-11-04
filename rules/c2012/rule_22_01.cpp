/*
 * MISRA C:2012 Rule 22.1 - Clang-backed detector
 *
 * Topic: Resource management
 * Engineering intent: Independent checker contract for Rule 22.1 in the Resource management family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Track acquisition, ownership, state transitions, release, files, streams, and exceptional control-flow paths.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: Provable subset: a local pointer receiving an allocation whose every use only reads through it (never released, returned or stored) is reported as possible.
 * See docs/known-limits.md for interpretations.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"
#include "misra/c2012/rule_support.hpp"

#include <memory>

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
    return support::syntax_findings(context, "misra-c2012-22.1-resource-not-released");
  }
};

}  // namespace

RulePtr make_rule_22_01() {
  return std::make_unique<Rule_22_01>();
}

}  // namespace misra::c2012
