/*
 * MISRA C:2012 Rule 3.2 - engineering scaffold
 *
 * Topic: Comments
 * Engineering intent: Independent checker contract for Rule 3.2 in the Comments family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Inspect raw source spelling, comment tokens, line splicing, and comment boundaries before semantic analysis.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: Report a line comment that is continued onto the next line by a trailing backslash.
 * Evidence: AST/preprocessor observations recorded by the Clang adapter.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"

#include <memory>

#include "misra/observation_rule.hpp"

namespace misra::c2012 {
namespace {

class Rule_03_02 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "3.2",
        RuleCategory::Required,
        Decidability::Decidable,
        AnalysisScope::SingleTranslationUnit,
        {false, true},
        "Comments",
        "Independent checker contract for Rule 3.2 in the Comments family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Inspect raw source spelling, comment tokens, line splicing, and comment boundaries before semantic analysis.",
        ImplementationStatus::Implemented,
    };
    return descriptor;
  }

  [[nodiscard]] RuleEvaluation evaluate(
      const AnalysisContext& context) const override {
    return findings_from_observations(context, {"comment-line-splice"},
                                      "misra-c2012-3.2-comment-line-splice");
  }
};

}  // namespace

RulePtr make_rule_03_02() {
  return std::make_unique<Rule_03_02>();
}

}  // namespace misra::c2012
