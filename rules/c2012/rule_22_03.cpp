/*
 * MISRA C:2012 Rule 22.3 - Clang-backed detector
 *
 * Topic: Resource management
 * Engineering intent: Independent checker contract for Rule 22.3 in the Resource management family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Track acquisition, ownership, state transitions, release, files, streams, and exceptional control-flow paths.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: Provable subset: one function opening the same literal path twice with at least one write-capable mode is reported as possible.
 * See docs/known-limits.md for interpretations.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"
#include "misra/c2012/rule_support.hpp"

#include <memory>

namespace misra::c2012 {
namespace {

class Rule_22_03 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "22.3",
        RuleCategory::Required,
        Decidability::Undecidable,
        AnalysisScope::System,
        {true, true},
        "Resource management",
        "Independent checker contract for Rule 22.3 in the Resource management family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Track acquisition, ownership, state transitions, release, files, streams, and exceptional control-flow paths.",
        ImplementationStatus::Implemented,
    };
    return descriptor;
  }

  [[nodiscard]] RuleEvaluation evaluate(
      const AnalysisContext& context) const override {
    return support::syntax_findings(context, "misra-c2012-22.3-file-open-twice");
  }
};

}  // namespace

RulePtr make_rule_22_03() {
  return std::make_unique<Rule_22_03>();
}

}  // namespace misra::c2012
