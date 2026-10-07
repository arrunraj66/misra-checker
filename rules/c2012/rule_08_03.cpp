/*
 * MISRA C:2012 Rule 8.3 - engineering scaffold
 *
 * Topic: Declarations and definitions
 * Engineering intent: Independent checker contract for Rule 8.3 in the Declarations and definitions family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Reconcile declarations, definitions, linkage, prototypes, qualifiers, and cross-translation-unit symbol records.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: Report declarations of one external name whose type (qualifiers included) or parameter names differ from the first declaration seen.
 * Evidence: file-scope symbol facts merged across all translation units of the run.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"

#include <memory>

#include "misra/symbol_rule.hpp"

namespace misra::c2012 {
namespace {

class Rule_08_03 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "8.3",
        RuleCategory::Required,
        Decidability::Decidable,
        AnalysisScope::System,
        {true, true},
        "Declarations and definitions",
        "Independent checker contract for Rule 8.3 in the Declarations and definitions family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Reconcile declarations, definitions, linkage, prototypes, qualifiers, and cross-translation-unit symbol records.",
        ImplementationStatus::Implemented,
    };
    return descriptor;
  }

  [[nodiscard]] RuleEvaluation evaluate(
      const AnalysisContext& context) const override {
    RuleEvaluation evaluation{EvaluationStatus::Complete, {}};
    for (const auto& [name, facts] : group_symbols(context, true)) {
      const SymbolFact* first = nullptr;
      for (const SymbolFact* fact : facts) {
        if (fact->role == SymbolRole::Reference) {
          continue;
        }
        if (first == nullptr) {
          first = fact;
          continue;
        }
        const bool names_differ = !fact->param_names.empty() &&
                                  !first->param_names.empty() &&
                                  (fact->param_names != first->param_names);
        if ((fact->type_text != first->type_text) || names_differ) {
          evaluation.findings.push_back({"misra-c2012-8.3-inconsistent-declaration",
                                         fact->location, FindingCertainty::Definite});
        }
      }
    }
    return evaluation;
  }
};

}  // namespace

RulePtr make_rule_08_03() {
  return std::make_unique<Rule_08_03>();
}

}  // namespace misra::c2012
