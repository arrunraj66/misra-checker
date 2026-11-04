/*
 * MISRA C:2012 Rule 9.1 - Clang-backed detector
 *
 * Topic: Initialization
 * Engineering intent: Independent checker contract for Rule 9.1 in the Initialization family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Model object initialization state, aggregate coverage, initializer shape, designators, and storage duration.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: An automatic scalar object without an initializer that is read before any assignment or address-of occurs in source order is reported as possible; branches and loops are not modelled.
 * See docs/known-limits.md for interpretations.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"
#include "misra/c2012/rule_support.hpp"

#include <memory>

namespace misra::c2012 {
namespace {

class Rule_09_01 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "9.1",
        RuleCategory::Mandatory,
        Decidability::Undecidable,
        AnalysisScope::System,
        {true, true},
        "Initialization",
        "Independent checker contract for Rule 9.1 in the Initialization family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Model object initialization state, aggregate coverage, initializer shape, designators, and storage duration.",
        ImplementationStatus::Implemented,
    };
    return descriptor;
  }

  [[nodiscard]] RuleEvaluation evaluate(
      const AnalysisContext& context) const override {
    return support::syntax_findings(context, "misra-c2012-9.1-uninitialized-read");
  }
};

}  // namespace

RulePtr make_rule_09_01() {
  return std::make_unique<Rule_09_01>();
}

}  // namespace misra::c2012
