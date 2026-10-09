/*
 * MISRA C:2012 Rule 5.8 - engineering scaffold
 *
 * Topic: Identifiers
 * Engineering intent: Independent checker contract for Rule 5.8 in the Identifiers family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Build scoped symbol, namespace, linkage, macro, and significant-character indexes across the configured analysis scope.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: Whole-program: report an external-linkage name that is also the name of an internal-linkage function or object elsewhere in the program (Inconclusive with a single translation unit).
 * Evidence: file-scope symbol facts merged across all translation units of the run.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"

#include <memory>

#include "misra/symbol_rule.hpp"

namespace misra::c2012 {
namespace {

class Rule_05_08 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "5.8",
        RuleCategory::Required,
        Decidability::Decidable,
        AnalysisScope::System,
        {true, true},
        "Identifiers",
        "Independent checker contract for Rule 5.8 in the Identifiers family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Build scoped symbol, namespace, linkage, macro, and significant-character indexes across the configured analysis scope.",
        ImplementationStatus::Implemented,
    };
    return descriptor;
  }

  [[nodiscard]] RuleEvaluation evaluate(
      const AnalysisContext& context) const override {
    if (!has_multiple_units(context)) {
      return {EvaluationStatus::Inconclusive};
    }
    RuleEvaluation evaluation{EvaluationStatus::Complete, {}};
    const SymbolGroups external = group_symbols(context, true);
    for (const auto& [name, facts] : group_symbols(context, false)) {
      const auto match = external.find(name);
      if (match == external.end()) {
        continue;
      }
      for (const SymbolFact* fact : facts) {
        if (fact->role == SymbolRole::Definition) {
          evaluation.findings.push_back({"misra-c2012-5.8-external-name-reused",
                                         fact->location, FindingCertainty::Definite});
        }
      }
      for (const SymbolFact* fact : match->second) {
        if (fact->role == SymbolRole::Definition) {
          evaluation.findings.push_back({"misra-c2012-5.8-external-name-reused",
                                         fact->location, FindingCertainty::Definite});
        }
      }
    }
    return evaluation;
  }
};

}  // namespace

RulePtr make_rule_05_08() {
  return std::make_unique<Rule_05_08>();
}

}  // namespace misra::c2012
