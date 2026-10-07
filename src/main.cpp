#include <filesystem>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include <cstdlib>

#include "misra/clang_frontend.hpp"
#include "misra/converter.hpp"
#include "misra/license.hpp"
#include "misra/rule_registry.hpp"

namespace {

constexpr std::string_view kVersion = "0.4.0";

constexpr std::string_view category_name(const misra::RuleCategory category) {
  switch (category) {
    case misra::RuleCategory::Mandatory:
      return "mandatory";
    case misra::RuleCategory::Required:
      return "required";
    case misra::RuleCategory::Advisory:
      return "advisory";
  }
  return "unknown";
}

void print_usage() {
  std::cerr
      << "Usage:\n"
      << "  misra-checker --version\n"
      << "  misra-checker --list-rules\n"
      << "  misra-checker analyze --compile-commands <file-or-directory> "
         "[--file <source>]...\n"
      << "  misra-checker convert --compile-commands <file> --file <source>\n"
         "      --provider-cmd <shell-command> [--verify-cmd <shell-command>]\n"
         "      [--output-dir <dir>] [--license <file>] [--fix-class <id>] [--apply]\n";
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
      // Rules driven by compiler diagnostics are still meaningful.
      const misra::RuleRegistry failed_registry;
      std::size_t diagnostic_findings = 0U;
      for (const char* id : {"1.1", "20.14"}) {
        const misra::Rule* rule = failed_registry.find(id);
        if (rule == nullptr) {
          continue;
        }
        for (const misra::Finding& finding : rule->evaluate(frontend_result.context).findings) {
          ++diagnostic_findings;
          std::cout << finding.location.file << ':' << finding.location.line << ':'
                    << finding.location.column << ": "
                    << category_name(rule->descriptor().category)
                    << ": MISRA C:2012 Rule " << id << " [" << finding.message_key << "]\n";
        }
      }
      return diagnostic_findings == 0U ? 2 : 1;
    }

    const misra::RuleRegistry registry;
    std::size_t completed_rules = 0U;
    std::size_t inconclusive_rules = 0U;
    std::size_t finding_count = 0U;
    for (const auto& rule : registry.rules()) {
      const misra::RuleEvaluation evaluation =
          rule->evaluate(frontend_result.context);
      if (evaluation.status == misra::EvaluationStatus::Inconclusive) {
        ++inconclusive_rules;
        continue;
      }
      if (evaluation.status != misra::EvaluationStatus::Complete) {
        continue;
      }

      ++completed_rules;
      for (const misra::Finding& finding : evaluation.findings) {
        ++finding_count;
        std::cout << finding.location.file << ':' << finding.location.line << ':'
                  << finding.location.column << ": "
                  << category_name(rule->descriptor().category)
                  << ": MISRA C:2012 Rule "
                  << rule->descriptor().id << " [" << finding.message_key
                  << "]\n";
      }
    }

    std::cout << "Analyzed " << frontend_result.analyzed_files.size()
              << " translation unit(s); " << completed_rules
              << " implemented rule(s); " << finding_count << " finding(s).\n";
    if (inconclusive_rules != 0U) {
      std::cout << inconclusive_rules
                << " whole-program rule(s) inconclusive: analyze at least two "
                   "translation units together.\n";
    }
    return finding_count == 0U ? 0 : 1;
  }

  if ((argc >= 2) && (std::string_view{argv[1]} == "convert")) {
    misra::ConvertOptions options;
    options.output_dir = "misra-convert-out";
    std::string license_path;
    if (const char* env = std::getenv("MISRA_LICENSE_FILE")) {
      license_path = env;
    }
    for (int index = 2; index < argc; ++index) {
      const std::string_view argument{argv[index]};
      const bool has_value = (index + 1) < argc;
      if ((argument == "--compile-commands") && has_value) {
        options.compilation_database = argv[++index];
      } else if ((argument == "--file") && has_value) {
        options.source_file = argv[++index];
      } else if ((argument == "--provider-cmd") && has_value) {
        options.provider_command = argv[++index];
      } else if ((argument == "--verify-cmd") && has_value) {
        options.verify_command = argv[++index];
      } else if ((argument == "--output-dir") && has_value) {
        options.output_dir = argv[++index];
      } else if ((argument == "--license") && has_value) {
        license_path = argv[++index];
      } else if ((argument == "--fix-class") && has_value) {
        options.fix_class = argv[++index];
      } else if (argument == "--apply") {
        options.apply = true;
      } else {
        print_usage();
        return 64;
      }
    }
    if (options.compilation_database.empty() || options.source_file.empty() ||
        options.provider_command.empty()) {
      print_usage();
      return 64;
    }
    if (std::filesystem::is_directory(options.compilation_database)) {
      options.compilation_database += "/compile_commands.json";
    }

    const misra::LicenseResult license =
        misra::load_license_file(license_path, misra::today_iso_date());
    if (!license.valid || !misra::has_feature(license.info, "convert")) {
      std::cerr << "misra-checker: converter requires a valid license with the "
                   "'convert' feature ("
                << (license.valid ? "feature not granted" : license.error_message)
                << ")\n";
      return 77;
    }

    const misra::ConvertReport report = misra::convert_file(options);
    std::cout << misra::outcome_name(report.outcome) << ": " << report.detail
              << '\n';
    if (!report.patch_path.empty()) {
      std::cout << "patch: " << report.patch_path.string() << '\n';
    }
    return (report.outcome == misra::ConvertOutcome::Proposed ||
            report.outcome == misra::ConvertOutcome::Applied ||
            report.outcome == misra::ConvertOutcome::NothingToConvert)
               ? 0
               : 1;
  }

  const misra::RuleRegistry registry;
  std::cout << "MISRA C checker foundation initialized with "
            << registry.size() << " rule structures.\n";
  return 0;
}
