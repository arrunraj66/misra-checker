/*
 * MISRA C:2012 Rule 18.1 - engineering scaffold
 *
 * Topic: Pointers and arrays
 * Engineering intent: Independent checker contract for Rule 18.1 in the Pointers and arrays family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Track pointer provenance, array bounds, arithmetic, comparisons, object lifetime, variable-length arrays, and flexible members.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: Subset: report constant offsets or constant indexes that fall outside the declared array.
 * Evidence: AST, preprocessor, diagnostic and control-flow observations recorded by the Clang adapter.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"

#include <memory>

#include "misra/observation_rule.hpp"

namespace misra::c2012 {
namespace {

class Rule_18_01 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "18.1",
        RuleCategory::Required,
        Decidability::Undecidable,
        AnalysisScope::System,
        {true, true},
        "Pointers and arrays",
        "Independent checker contract for Rule 18.1 in the Pointers and arrays family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Track pointer provenance, array bounds, arithmetic, comparisons, object lifetime, variable-length arrays, and flexible members.",
        ImplementationStatus::Implemented,
    };
    return descriptor;
  }

  [[nodiscard]] RuleEvaluation evaluate(
      const AnalysisContext& context) const override {
    return findings_from_observations(context, {"pointer-arithmetic-out-of-bounds"},
                                      "misra-c2012-18.1-pointer-arithmetic-out-of-bounds", {}, {});
  }
};

}  // namespace

RulePtr make_rule_18_01() {
  return std::make_unique<Rule_18_01>();
}

}  // namespace misra::c2012
