/*
 * MISRA C:2012 Rule 2.5 - Clang-backed detector
 *
 * Topic: Unused and unreachable code
 * Engineering intent: Independent checker contract for Rule 2.5 in the Unused and unreachable code family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Build control-flow and reference graphs, then combine reachability, side-effect, and whole-program usage evidence.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: Macros never expanded or tested anywhere in the analysed program are reported. Whole-program rule: results are valid only when the compilation
 * database covers the complete program.
 * Findings produced by macro expansion are tagged; see docs/macro-policy.md.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"
#include "misra/c2012/rule_support.hpp"

#include <memory>

namespace misra::c2012 {
namespace {

class Rule_02_05 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "2.5",
        RuleCategory::Advisory,
        Decidability::Decidable,
        AnalysisScope::System,
        {true, true},
        "Unused and unreachable code",
        "Independent checker contract for Rule 2.5 in the Unused and unreachable code family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Build control-flow and reference graphs, then combine reachability, side-effect, and whole-program usage evidence.",
        ImplementationStatus::Implemented,
    };
    return descriptor;
  }

  [[nodiscard]] RuleEvaluation evaluate(
      const AnalysisContext& context) const override {
    return support::unused_macros(context);
  }
};

}  // namespace

RulePtr make_rule_02_05() {
  return std::make_unique<Rule_02_05>();
}

}  // namespace misra::c2012
