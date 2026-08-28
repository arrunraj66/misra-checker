#include <filesystem>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include "misra/clang_frontend.hpp"
#include "misra/rule_registry.hpp"

namespace {

constexpr std::string_view kVersion = "0.2.0";

void print_usage() {
  std::cerr
      << "Usage:\n"
      << "  misra-checker --version\n"
      << "  misra-checker --list-rules\n"
      << "  misra-checker analyze --compile-commands <file-or-directory> "
         "[--file <source>]...\n";
}

}  // namespace

int main(int argc, char* argv[]) {
  if ((argc == 2) && (std::string_view{argv[1]} == "--version")) {
    std::cout << "misra-checker " << kVersion << '\n';
    return 0;
  }

  if ((argc == 2) && (std::string_view{argv[1]} == "--list-rules")) {
    const misra::RuleRegistry registry;
    for (const auto& rule : registry.rules()) {
      std::cout << rule->descriptor().id << '\n';
    }
    return 0;
  }

  if ((argc >= 2) && (std::string_view{argv[1]} == "analyze")) {
    std::filesystem::path compilation_database;
    std::vector<std::string> requested_files;

    for (int index = 2; index < argc; ++index) {
      const std::string_view argument{argv[index]};
      if ((argument == "--compile-commands") && ((index + 1) < argc)) {
        compilation_database = argv[++index];
      } else if ((argument == "--file") && ((index + 1) < argc)) {
        requested_files.emplace_back(argv[++index]);
      } else {
        print_usage();
        return 64;
      }
    }

    if (compilation_database.empty()) {
      print_usage();
      return 64;
    }
    if (std::filesystem::is_directory(compilation_database)) {
      compilation_database /= "compile_commands.json";
    }

    const misra::ClangFrontend frontend;
    const misra::FrontendResult frontend_result = frontend.analyze(
        compilation_database.string(), requested_files);
    if (!frontend_result.success) {
      std::cerr << "misra-checker: " << frontend_result.error_message << '\n';
      return 2;
    }

    const misra::RuleRegistry registry;
    std::size_t completed_rules = 0U;
    std::size_t finding_count = 0U;
    for (const auto& rule : registry.rules()) {
      const misra::RuleEvaluation evaluation =
          rule->evaluate(frontend_result.context);
      if (evaluation.status != misra::EvaluationStatus::Complete) {
        continue;
      }

      ++completed_rules;
      for (const misra::Finding& finding : evaluation.findings) {
        ++finding_count;
        std::cout << finding.location.file << ':' << finding.location.line << ':'
                  << finding.location.column << ": advisory: MISRA C:2012 Rule "
                  << rule->descriptor().id << " [" << finding.message_key
                  << "]\n";
      }
    }

    std::cout << "Analyzed " << frontend_result.analyzed_files.size()
              << " translation unit(s); " << completed_rules
              << " implemented rule(s); " << finding_count << " finding(s).\n";
    return finding_count == 0U ? 0 : 1;
  }

  const misra::RuleRegistry registry;
  std::cout << "MISRA C checker foundation initialized with "
            << registry.size() << " rule structures.\n";
  return 0;
}
