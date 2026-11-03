// Data-driven integration test: <database> then groups of
// <rule-id> <fixture[,fixture...]> <expected-finding-count>; the fixtures of
// one group are analysed together as one program.
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "misra/clang_frontend.hpp"
#include "misra/rule_registry.hpp"

int main(const int argc, char* argv[]) {
  if (argc < 5 || ((argc - 2) % 3) != 0) {
    std::cerr << "usage: db (rule fixture count)...\n";
    return EXIT_FAILURE;
  }
  const misra::ClangFrontend frontend;
  const misra::RuleRegistry registry;
  int failures = 0;
  for (int i = 2; i < argc; i += 3) {
    const std::string id = argv[i];
    const std::string fixture = argv[i + 1];
    std::vector<std::string> files;
    std::stringstream stream(fixture);
    for (std::string file; std::getline(stream, file, ',');) {
      files.push_back(file);
    }
    const std::size_t expected = std::stoul(argv[i + 2]);
    const misra::Rule* const rule = registry.find(id);
    if (rule == nullptr) {
      std::cerr << "unknown rule " << id << '\n';
      return EXIT_FAILURE;
    }
    const misra::FrontendResult result = frontend.analyze(argv[1], files);
    if (!result.success) {
      std::cerr << fixture << ": " << result.error_message << '\n';
      ++failures;
      continue;
    }
    const misra::RuleEvaluation evaluation = rule->evaluate(result.context);
    if (evaluation.status != misra::EvaluationStatus::Complete ||
        evaluation.findings.size() != expected) {
      std::cerr << "Rule " << id << " on " << fixture << ": expected "
                << expected << " finding(s), got " << evaluation.findings.size()
                << '\n';
      ++failures;
    }
  }
  return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
