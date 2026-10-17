/*
 * MISRA C:2012 Rule 8.7 - engineering scaffold
 *
 * Topic: Declarations and definitions
 * Engineering intent: Independent checker contract for Rule 8.7 in the Declarations and definitions family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Reconcile declarations, definitions, linkage, prototypes, qualifiers, and cross-translation-unit symbol records.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: Whole-program: report an externally visible function or object that is defined but referenced only from its own translation unit (main excluded; Inconclusive with a single translation unit).
 * Evidence: file-scope symbol facts merged across all translation units of the run.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"

#include <memory>

#include "misra/symbol_rule.hpp"

namespace misra::c2012 {
namespace {

class Rule_08_07 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "8.7",
        RuleCategory::Advisory,
        Decidability::Decidable,
        AnalysisScope::System,
        {true, true},
        "Declarations and definitions",
        "Independent checker contract for Rule 8.7 in the Declarations and definitions family. The exact normative predicate remains linked to the controlled licensed rule specification.",
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
      const SymbolFact* definition = nullptr;
      for (const SymbolFact* fact : facts) {
        if (fact->role == SymbolRole::Definition) {
          definition = fact;
        }
      }
      if (definition == nullptr) {
        continue;
      }
      bool used_elsewhere = false;
      for (const SymbolFact* fact : facts) {
        used_elsewhere = used_elsewhere ||
                         ((fact->role == SymbolRole::Reference) &&
                          (fact->translation_unit != definition->translation_unit));
      }
      if (!used_elsewhere) {
        evaluation.findings.push_back({"misra-c2012-8.7-single-unit-external",
                                       definition->location, FindingCertainty::Definite});
      }
    }
    return evaluation;
  }
};

}  // namespace

RulePtr make_rule_08_07() {
  return std::make_unique<Rule_08_07>();
}

}  // namespace misra::c2012
