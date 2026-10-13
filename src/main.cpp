#include <algorithm>
#include <filesystem>
#include <fstream>
#include <map>
#include <tuple>
#include <set>
#include <sstream>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include <cstdlib>

#include "misra/clang_frontend.hpp"
#include "misra/converter.hpp"
#include "misra/license.hpp"
#include "misra/report.hpp"
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
         "      [--format text|sarif] [--sarif-out <file>] [--source-root <dir>]\n"
         "      [--deviations <file.json>]\n"
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
    std::string format = "text";
    std::string sarif_out;
    std::string deviations_path;
    std::string source_root;

    for (int index = 2; index < argc; ++index) {
      const std::string_view argument{argv[index]};
      const bool has_value = (index + 1) < argc;
      if ((argument == "--compile-commands") && has_value) {
        compilation_database = argv[++index];
      } else if ((argument == "--file") && has_value) {
        requested_files.emplace_back(argv[++index]);
      } else if ((argument == "--format") && has_value) {
        format = argv[++index];
      } else if ((argument == "--sarif-out") && has_value) {
        sarif_out = argv[++index];
      } else if ((argument == "--deviations") && has_value) {
        deviations_path = argv[++index];
      } else if ((argument == "--source-root") && has_value) {
        source_root = argv[++index];
      } else {
        print_usage();
        return 64;
      }
    }

    if (compilation_database.empty() || ((format != "text") && (format != "sarif"))) {
      print_usage();
      return 64;
    }
    if (std::filesystem::is_directory(compilation_database)) {
      compilation_database /= "compile_commands.json";
    }

    const misra::RuleRegistry registry;
    std::map<std::string, std::string> categories;
    std::map<std::string, std::string> topics;
    for (const auto& rule : registry.rules()) {
      const std::string id{rule->descriptor().id};
      categories[id] = std::string{category_name(rule->descriptor().category)};
      topics[id] = std::string{rule->descriptor().topic};
    }

    // Validate the deviation file before spending time on the analysis.
    std::vector<misra::Deviation> deviations;
    if (!deviations_path.empty()) {
      misra::DeviationSet loaded = misra::load_deviations(deviations_path, categories);
      if (!loaded.errors.empty()) {
        for (const std::string& error : loaded.errors) {
          std::cerr << "misra-checker: deviations: " << error << '\n';
        }
        return 65;
      }
      deviations = std::move(loaded.deviations);
    }

    const misra::ClangFrontend frontend;
    const misra::FrontendResult frontend_result = frontend.analyze(
        compilation_database.string(), requested_files);
    // Clang recovers from most errors and still hands over a usable AST, so a
    // compile error must not hide every other finding. Analysis continues on
    // the recovered AST; the user is told the result may be incomplete.
    const bool compile_errors = !frontend_result.success;
    if (compile_errors) {
      if (frontend_result.analyzed_files.empty()) {
        std::cerr << "misra-checker: " << frontend_result.error_message << '\n';
        return 2;
      }
      std::cerr << "misra-checker: " << frontend_result.error_message
                << "; continuing on the recovered AST, results may be incomplete\n";
    }

    std::set<std::string> printed;
    std::vector<misra::ReportRow> ordered;
    std::size_t completed_rules = 0U;
    std::size_t inconclusive_rules = 0U;
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
      const std::string id{rule->descriptor().id};
      for (const misra::Finding& finding : evaluation.findings) {
        std::ostringstream identity;
        identity << finding.location.file << ':' << finding.location.line << ':'
                 << finding.location.column << ':' << id << ':' << finding.message_key;
        // Headers are seen by several translation units; report each once.
        if (printed.insert(identity.str()).second) {
          misra::ReportRow row;
          row.file = finding.location.file;
          row.line = finding.location.line;
          row.column = finding.location.column;
          row.rule = id;
          row.category = categories[id];
          row.key = finding.message_key;
          row.topic = topics[id];
          ordered.push_back(std::move(row));
        }
      }
    }

    // Deterministic output regardless of container iteration order.
    std::sort(ordered.begin(), ordered.end(),
              [](const misra::ReportRow& a, const misra::ReportRow& b) {
                return std::tie(a.file, a.line, a.column, a.rule, a.key) <
                       std::tie(b.file, b.line, b.column, b.rule, b.key);
              });
    const misra::DeviationOutcome outcome =
        misra::apply_deviations(std::move(ordered), deviations, misra::today_iso());

    const auto line_for = [](const misra::ReportRow& row, const std::string_view label) {
      std::ostringstream line;
      line << row.file << ':' << row.line << ':' << row.column << ": " << label
           << ": MISRA C:2012 Rule " << row.rule << " [" << row.key << ']';
      return line.str();
    };

    misra::SarifOptions sarif_options;
    sarif_options.tool_version = std::string{kVersion};
    sarif_options.source_root = source_root;
    sarif_options.compile_errors = compile_errors;
    const bool sarif_needed = (format == "sarif") || !sarif_out.empty();
    const std::string sarif = sarif_needed ? misra::to_sarif(outcome, sarif_options) : std::string{};
    if (!sarif_out.empty()) {
      std::ofstream file(sarif_out);
      file << sarif;
      if (!file) {
        std::cerr << "misra-checker: cannot write '" << sarif_out << "'\n";
        return 2;
      }
    }

    // With --format sarif stdout carries only the SARIF document.
    std::ostream& text = (format == "sarif") ? std::cerr : std::cout;
    if (format == "sarif") {
      std::cout << sarif;
    } else {
      for (const misra::ReportRow& row : outcome.active) {
        text << line_for(row, row.category) << '\n';
      }
    }
    for (const misra::ReportRow& row : outcome.deviated) {
      if (format == "text") {
        text << line_for(row, "deviated") << " (" << row.deviation_id << ")\n";
      }
    }
    for (const std::string& id : outcome.expired) {
      std::cerr << "misra-checker: deviation " << id << " has expired and was not applied\n";
    }
    for (const std::string& id : outcome.unused) {
      std::cerr << "misra-checker: deviation " << id << " matched no finding\n";
    }

    text << "Analyzed " << frontend_result.analyzed_files.size()
         << " translation unit(s); " << completed_rules
         << " implemented rule(s); " << outcome.active.size() << " finding(s).\n";
    if (!deviations.empty()) {
      text << outcome.deviated.size() << " finding(s) deviated by "
           << (deviations.size() - outcome.unused.size() - outcome.expired.size())
           << " deviation record(s) in use.\n";
    }
    if (inconclusive_rules != 0U) {
      text << inconclusive_rules
           << " whole-program rule(s) inconclusive: analyze at least two "
              "translation units together.\n";
    }
    if (!outcome.active.empty()) {
      return 1;
    }
    return compile_errors ? 2 : 0;
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
