#include <cstdlib>
#include <iostream>
#include <string>
#include <string_view>

#include "misra/clang_frontend.hpp"
#include "misra/rule_registry.hpp"

namespace {

void require(const bool condition, const std::string_view message) {
  if (!condition) {
    std::cerr << "rule_15_2_integration_test: " << message << '\n';
    std::exit(EXIT_FAILURE);
  }
}

void expect_findings(const misra::ClangFrontend& frontend,
                     const misra::Rule& rule, const std::string& database,
                     const std::string& source,
                     const bool expected_target_declared_later,
                     const std::size_t expected_findings) {
  const misra::FrontendResult frontend_result =
      frontend.analyze(database, {source});
  require(frontend_result.success, frontend_result.error_message);
  require(frontend_result.context.control_flow.goto_statements.size() == 1U,
          "fixture must contain exactly one goto statement");
  require(frontend_result.context.control_flow.goto_statements.front()
              .target_declared_later == expected_target_declared_later,
          "frontend computed unexpected jump direction");

  const misra::RuleEvaluation evaluation =
      rule.evaluate(frontend_result.context);
  require(evaluation.status == misra::EvaluationStatus::Complete,
          "Rule 15.2 evaluation must complete");
  require(evaluation.findings.size() == expected_findings,
          "unexpected Rule 15.2 finding count");
}

}  // namespace

int main(const int argc, char* argv[]) {
  require(argc == 6, "expected database and four fixture paths");

  const misra::ClangFrontend frontend;
  const misra::RuleRegistry registry;
  const misra::Rule* const rule = registry.find("15.2");
  require(rule != nullptr, "Rule 15.2 must be registered");

  const std::string database = argv[1];
  expect_findings(frontend, *rule, database, argv[2], true, 0U);
  expect_findings(frontend, *rule, database, argv[3], false, 1U);
  expect_findings(frontend, *rule, database, argv[4], true, 0U);
  expect_findings(frontend, *rule, database, argv[5], false, 1U);
  return EXIT_SUCCESS;
}
