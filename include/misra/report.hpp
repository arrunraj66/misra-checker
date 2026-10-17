#pragma once

// Result reporting: deviation records (MISRA Compliance:2020 style) and SARIF.

#include <map>
#include <string>
#include <vector>

namespace misra {

struct ReportRow final {
  std::string file;
  unsigned int line = 0U;
  unsigned int column = 0U;
  std::string rule;      // "8.4"
  std::string category;  // "mandatory" | "required" | "advisory"
  std::string key;       // "misra-c2012-8.4-no-prior-declaration"
  std::string topic;     // paraphrased topic from the rule descriptor
  std::string deviation_id;      // set when a deviation record covers the row
  std::string justification;
};

struct Deviation final {
  std::string id;
  std::vector<std::string> rules;
  std::string file;  // path suffix or '*' pattern; empty = any file
  unsigned int line_from = 0U;  // 0 = any line
  unsigned int line_to = 0U;
  std::string justification;
  std::string approved_by;
  std::string expires;  // YYYY-MM-DD; empty = never
};

struct DeviationSet final {
  std::vector<Deviation> deviations;
  std::vector<std::string> errors;  // non-empty => the file must not be used
};

// `categories` maps rule id -> category so that mandatory rules, which MISRA
// does not allow to be deviated, are rejected.
DeviationSet load_deviations(const std::string& path,
                             const std::map<std::string, std::string>& categories);

struct DeviationOutcome final {
  std::vector<ReportRow> active;
  std::vector<ReportRow> deviated;
  std::vector<std::string> unused;   // ids that matched nothing
  std::vector<std::string> expired;  // ids ignored because they expired
};

// `today` is YYYY-MM-DD.
DeviationOutcome apply_deviations(std::vector<ReportRow> rows,
                                  const std::vector<Deviation>& deviations,
                                  const std::string& today);

struct SarifOptions final {
  std::string tool_version;
  std::string source_root;  // when set, paths below it become relative URIs
  bool compile_errors = false;
  std::vector<std::string> notes;  // extra informational notifications
};

std::string to_sarif(const DeviationOutcome& outcome, const SarifOptions& options);

std::string today_iso();

}  // namespace misra
