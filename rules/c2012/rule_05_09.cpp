/*
 * MISRA C:2012 Rule 5.9 - engineering scaffold
 *
 * Topic: Identifiers
 * Engineering intent: Independent checker contract for Rule 5.9 in the Identifiers family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Build scoped symbol, namespace, linkage, macro, and significant-character indexes across the configured analysis scope.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: Whole-program: report internal-linkage functions or objects that share a name but are defined at different locations (Inconclusive with a single translation unit).
 * Evidence: file-scope symbol facts merged across all translation units of the run.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"

#include <memory>

#include "misra/symbol_rule.hpp"

namespace misra::c2012 {
namespace {

class Rule_05_09 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "5.9",
        RuleCategory::Advisory,
        Decidability::Decidable,
        AnalysisScope::System,
        {true, true},
        "Identifiers",
        "Independent checker contract for Rule 5.9 in the Identifiers family. The exact normative predicate remains linked to the controlled licensed rule specification.",
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
    for (const auto& [name, facts] : group_symbols(context, false)) {
      std::set<std::string> places;
      for (const SymbolFact* fact : facts) {
        if (fact->role == SymbolRole::Definition) {
          places.insert(location_key(fact->location));
        }
      }
      if (places.size() > 1U) {
        for (const SymbolFact* fact : facts) {
          if (fact->role == SymbolRole::Definition) {
            evaluation.findings.push_back({"misra-c2012-5.9-internal-name-reused",
                                           fact->location, FindingCertainty::Definite});
          }
        }
      }
    }
    return evaluation;
  }
};

}  // namespace

RulePtr make_rule_05_09() {
  return std::make_unique<Rule_05_09>();
}

}  // namespace misra::c2012
