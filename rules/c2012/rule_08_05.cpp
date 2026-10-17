/*
 * MISRA C:2012 Rule 8.5 - engineering scaffold
 *
 * Topic: Declarations and definitions
 * Engineering intent: Independent checker contract for Rule 8.5 in the Declarations and definitions family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Reconcile declarations, definitions, linkage, prototypes, qualifiers, and cross-translation-unit symbol records.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: Report an external function or object that has non-defining declarations in more than one file (declare once, in a header).
 * Evidence: file-scope symbol facts merged across all translation units of the run.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"

#include <memory>

#include "misra/symbol_rule.hpp"

namespace misra::c2012 {
namespace {

class Rule_08_05 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "8.5",
        RuleCategory::Required,
        Decidability::Decidable,
        AnalysisScope::System,
        {true, true},
        "Declarations and definitions",
        "Independent checker contract for Rule 8.5 in the Declarations and definitions family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Reconcile declarations, definitions, linkage, prototypes, qualifiers, and cross-translation-unit symbol records.",
        ImplementationStatus::Implemented,
    };
    return descriptor;
  }

  [[nodiscard]] RuleEvaluation evaluate(
      const AnalysisContext& context) const override {
    RuleEvaluation evaluation{EvaluationStatus::Complete, {}};
    for (const auto& [name, facts] : group_symbols(context, true)) {
      std::set<std::string> files;
      for (const SymbolFact* fact : facts) {
        if (fact->role == SymbolRole::Declaration) {
          files.insert(fact->location.file);
        }
      }
      if (files.size() > 1U) {
        for (const SymbolFact* fact : facts) {
          if (fact->role == SymbolRole::Declaration) {
            evaluation.findings.push_back({"misra-c2012-8.5-declared-in-multiple-files",
                                           fact->location, FindingCertainty::Definite});
          }
        }
      }
    }
    return evaluation;
  }
};

}  // namespace

RulePtr make_rule_08_05() {
  return std::make_unique<Rule_08_05>();
}

}  // namespace misra::c2012
