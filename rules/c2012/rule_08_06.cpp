/*
 * MISRA C:2012 Rule 8.6 - engineering scaffold
 *
 * Topic: Declarations and definitions
 * Engineering intent: Independent checker contract for Rule 8.6 in the Declarations and definitions family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Reconcile declarations, definitions, linkage, prototypes, qualifiers, and cross-translation-unit symbol records.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: Whole-program: report an external name defined in more than one translation unit, or referenced but never defined (requires every translation unit of the program in one run; Inconclusive otherwise).
 * Evidence: file-scope symbol facts merged across all translation units of the run.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"

#include <memory>

#include "misra/symbol_rule.hpp"

namespace misra::c2012 {
namespace {

class Rule_08_06 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "8.6",
        RuleCategory::Required,
        Decidability::Decidable,
        AnalysisScope::System,
        {true, true},
        "Declarations and definitions",
        "Independent checker contract for Rule 8.6 in the Declarations and definitions family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Reconcile declarations, definitions, linkage, prototypes, qualifiers, and cross-translation-unit symbol records.",
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
    for (const auto& [name, facts] : group_symbols(context, true)) {
      std::set<std::string> defining_units;
      std::vector<const SymbolFact*> definitions;
      const SymbolFact* first_reference = nullptr;
      for (const SymbolFact* fact : facts) {
        if (fact->role == SymbolRole::Definition) {
          defining_units.insert(fact->translation_unit);
          definitions.push_back(fact);
        } else if ((fact->role == SymbolRole::Reference) && (first_reference == nullptr)) {
          first_reference = fact;
        }
      }
      if (defining_units.size() > 1U) {
        for (const SymbolFact* definition : definitions) {
          evaluation.findings.push_back({"misra-c2012-8.6-multiple-definitions",
                                         definition->location, FindingCertainty::Definite});
        }
      } else if (definitions.empty() && (first_reference != nullptr)) {
        evaluation.findings.push_back({"misra-c2012-8.6-no-definition",
                                       first_reference->location, FindingCertainty::Possible});
      }
    }
    return evaluation;
  }
};

}  // namespace

RulePtr make_rule_08_06() {
  return std::make_unique<Rule_08_06>();
}

}  // namespace misra::c2012
