#include <cstdlib>
#include <iostream>
#include <string_view>

#include "misra/rule_registry.hpp"

namespace {

void require(const bool condition, const std::string_view message) {
  if (!condition) {
    std::cerr << "rule_15_2_test: " << message << '\n';
    std::exit(EXIT_FAILURE);
  }
}

misra::GotoStatementFact make_goto(const bool target_declared_later) {
  return {{"example.c", 8U, 5U},
          {"example.c", target_declared_later ? 12U : 3U, 1U},
          target_declared_later,
          false,
          false};
}

}  // namespace

int main() {
  const misra::RuleRegistry registry;
  const misra::Rule* const rule = registry.find("15.2");
  require(rule != nullptr, "Rule 15.2 must be registered");

  misra::AnalysisContext compliant_context;
  compliant_context.control_flow.goto_statements.push_back(make_goto(true));
  const misra::RuleEvaluation compliant = rule->evaluate(compliant_context);
  require(compliant.status == misra::EvaluationStatus::Complete,
          "evaluation must complete");
  require(compliant.findings.empty(),
          "forward goto must not produce a finding");

  misra::AnalysisContext violating_context;
  violating_context.control_flow.goto_statements.push_back(make_goto(false));
  const misra::RuleEvaluation violating = rule->evaluate(violating_context);
  require(violating.status == misra::EvaluationStatus::Complete,
          "violating evaluation must complete");
  require(violating.findings.size() == 1U,
          "backward goto must produce one finding");
  require(violating.findings.front().location.line == 8U,
          "finding must identify the goto statement");
  require(violating.findings.front().certainty ==
              misra::FindingCertainty::Definite,
          "resolved source ordering must be definite");

  return EXIT_SUCCESS;
}
