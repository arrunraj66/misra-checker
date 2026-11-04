/*
 * MISRA C:2012 Rule 4.2 - Clang-backed detector
 *
 * Topic: Character sets and lexical elements
 * Engineering intent: Independent checker contract for Rule 4.2 in the Character sets and lexical elements family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Inspect preprocessing tokens, escape sequences, character encodings, and implementation-defined lexical behavior.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: A trigraph sequence anywhere in the text of a project file is reported.
 * See docs/known-limits.md for interpretations.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"
#include "misra/c2012/rule_support.hpp"

#include <memory>

namespace misra::c2012 {
namespace {

class Rule_04_02 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "4.2",
        RuleCategory::Advisory,
        Decidability::Decidable,
        AnalysisScope::SingleTranslationUnit,
        {true, true},
        "Character sets and lexical elements",
        "Independent checker contract for Rule 4.2 in the Character sets and lexical elements family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Inspect preprocessing tokens, escape sequences, character encodings, and implementation-defined lexical behavior.",
        ImplementationStatus::Implemented,
    };
    return descriptor;
  }

  [[nodiscard]] RuleEvaluation evaluate(
      const AnalysisContext& context) const override {
    return support::syntax_findings(context, "misra-c2012-4.2-trigraph");
  }
};

}  // namespace

RulePtr make_rule_04_02() {
  return std::make_unique<Rule_04_02>();
}

}  // namespace misra::c2012
