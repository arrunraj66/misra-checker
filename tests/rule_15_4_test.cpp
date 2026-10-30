#include <cstdlib>
#include <iostream>

#include "misra/rule_registry.hpp"

namespace {

void require(const bool condition, const char* message) {
  if (!condition) {
    std::cerr << "rule_15_4_test: " << message << '\n';
    std::exit(EXIT_FAILURE);
  }
}

}  // namespace

int main() {
  const misra::RuleRegistry registry;
  const misra::Rule* const rule = registry.find("15.4");
  require(rule != nullptr, "Rule 15.4 must be registered");

  misra::AnalysisContext context;
  context.control_flow.loops.push_back({{"example.c", 3U, 5U}, 0U});
  context.control_flow.loops.push_back({{"example.c", 9U, 5U}, 1U});
  context.control_flow.loops.push_back({{"example.c", 20U, 5U}, 2U});
  const misra::RuleEvaluation evaluation = rule->evaluate(context);
  require(evaluation.status == misra::EvaluationStatus::Complete,
          "must complete");
  require(evaluation.findings.size() == 1U,
          "only the loop with two terminating jumps is reported");
  require(evaluation.findings.front().location.line == 20U,
          "must locate the loop");
  return EXIT_SUCCESS;
}
