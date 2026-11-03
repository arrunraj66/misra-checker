/*
 * MISRA C:2012 Rule 21.3 - Clang-backed detector
 *
 * Topic: Standard library usage
 * Engineering intent: Independent checker contract for Rule 21.3 in the Standard library usage family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Match headers, reserved identifiers, declarations, and calls against the configured controlled-library policy.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: Calls to the <stdlib.h> dynamic memory functions malloc, calloc, realloc and free are reported.
 * Findings in system headers and other files are ignored.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"

#include <memory>
#include <set>
#include <string_view>

namespace misra::c2012 {
namespace {

class Rule_21_03 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "21.3",
        RuleCategory::Required,
        Decidability::Decidable,
        AnalysisScope::SingleTranslationUnit,
        {true, true},
        "Standard library usage",
        "Independent checker contract for Rule 21.3 in the Standard library usage family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Match headers, reserved identifiers, declarations, and calls against the configured controlled-library policy.",
        ImplementationStatus::Implemented,
    };
    return descriptor;
  }

  [[nodiscard]] RuleEvaluation evaluate(
      const AnalysisContext& context) const override {
    static const std::set<std::string_view> kBanned{"malloc", "calloc", "realloc", "free"};
    RuleEvaluation evaluation{EvaluationStatus::Complete, {}};
    for (const CallFact& call : context.calls) {
      if (call.callee_in_system_header && kBanned.count(call.callee) != 0U) {
        evaluation.findings.push_back(
            {"misra-c2012-21.3-dynamic-memory", call.location, FindingCertainty::Definite});
      }
    }
    return evaluation;
  }
};

}  // namespace

RulePtr make_rule_21_03() {
  return std::make_unique<Rule_21_03>();
}

}  // namespace misra::c2012
