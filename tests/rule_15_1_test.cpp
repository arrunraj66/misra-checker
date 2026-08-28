#include <cstdlib>
#include <iostream>
#include <string_view>

#include "misra/rule_registry.hpp"

namespace {

void require(const bool condition, const std::string_view message) {
  if (!condition) {
    std::cerr << "rule_15_1_test: " << message << '\n';
    std::exit(EXIT_FAILURE);
  }
}

}  // namespace

int main() {
  const misra::RuleRegistry registry;
  const misra::Rule* const rule = registry.find("15.1");
  require(rule != nullptr, "Rule 15.1 must be registered");

  misra::AnalysisContext compliant_context;
  const misra::RuleEvaluation compliant = rule->evaluate(compliant_context);
  require(compliant.status == misra::EvaluationStatus::Complete,
          "evaluation must complete");
  require(compliant.findings.empty(), "compliant context must have no findings");

  misra::AnalysisContext violating_context;
  violating_context.control_flow.goto_statements.push_back(
      {{"example.c", 7U, 3U}, false});
  const misra::RuleEvaluation violating = rule->evaluate(violating_context);
  require(violating.status == misra::EvaluationStatus::Complete,
          "violating evaluation must complete");
  require(violating.findings.size() == 1U,
          "one goto fact must produce one finding");
  require(violating.findings.front().location.line == 7U,
          "finding must preserve its source line");
  require(violating.findings.front().certainty ==
              misra::FindingCertainty::Definite,
          "syntactic goto finding must be definite");

  return EXIT_SUCCESS;
}
