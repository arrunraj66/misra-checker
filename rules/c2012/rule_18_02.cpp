/*
 * MISRA C:2012 Rule 18.2 - engineering scaffold
 *
 * Topic: Pointers and arrays
 * Engineering intent: Independent checker contract for Rule 18.2 in the Pointers and arrays family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Track pointer provenance, array bounds, arithmetic, comparisons, object lifetime, variable-length arrays, and flexible members.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: Subset: report subtraction of pointers derived from two different declared arrays.
 * Evidence: AST, preprocessor, diagnostic and control-flow observations recorded by the Clang adapter.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"

#include <memory>

#include "misra/observation_rule.hpp"

namespace misra::c2012 {
namespace {

class Rule_18_02 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "18.2",
        RuleCategory::Required,
        Decidability::Undecidable,
        AnalysisScope::System,
        {true, true},
        "Pointers and arrays",
        "Independent checker contract for Rule 18.2 in the Pointers and arrays family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Track pointer provenance, array bounds, arithmetic, comparisons, object lifetime, variable-length arrays, and flexible members.",
        ImplementationStatus::Implemented,
    };
    return descriptor;
  }

  [[nodiscard]] RuleEvaluation evaluate(
      const AnalysisContext& context) const override {
    return findings_from_observations(context, {"pointer-subtraction-different-objects"},
                                      "misra-c2012-18.2-pointer-subtraction-different-objects", {}, {});
  }
};

}  // namespace

RulePtr make_rule_18_02() {
  return std::make_unique<Rule_18_02>();
}

}  // namespace misra::c2012
