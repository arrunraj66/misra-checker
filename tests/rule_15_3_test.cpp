#include <cstdlib>
#include <iostream>

#include "misra/rule_registry.hpp"

namespace {

void require(const bool condition, const char* message) {
  if (!condition) {
    std::cerr << "rule_15_3_test: " << message << '\n';
    std::exit(EXIT_FAILURE);
  }
}

misra::GotoStatementFact make_goto(const bool enclosing) {
  return {{"example.c", 8U, 5U}, {"example.c", 12U, 1U}, true, false, false,
          enclosing};
}

}  // namespace

int main() {
  const misra::RuleRegistry registry;
  const misra::Rule* const rule = registry.find("15.3");
  require(rule != nullptr, "Rule 15.3 must be registered");

  misra::AnalysisContext compliant;
  compliant.control_flow.goto_statements.push_back(make_goto(true));
  const misra::RuleEvaluation ok = rule->evaluate(compliant);
  require(ok.status == misra::EvaluationStatus::Complete, "must complete");
  require(ok.findings.empty(), "enclosing-block label must not be reported");

  misra::AnalysisContext violating;
  violating.control_flow.goto_statements.push_back(make_goto(false));
  const misra::RuleEvaluation bad = rule->evaluate(violating);
  require(bad.findings.size() == 1U, "non-enclosing label must be reported");
  require(bad.findings.front().location.line == 8U, "must locate the goto");
  require(bad.findings.front().certainty == misra::FindingCertainty::Definite,
          "must be definite");
  return EXIT_SUCCESS;
}
