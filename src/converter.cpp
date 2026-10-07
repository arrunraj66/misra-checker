#include "misra/converter.hpp"

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <map>
#include <sstream>

#include "misra/clang_frontend.hpp"
#include "misra/license.hpp"
#include "misra/rule_registry.hpp"

namespace misra {
namespace {

namespace fs = std::filesystem;

struct Analysis final {
  bool ok;
  std::string error;
  std::vector<std::string> keys;  // one entry per finding: key@line
};

bool read_file(const fs::path& path, std::string& out) {
  std::ifstream file(path, std::ios::binary);
  if (!file) {
    return false;
  }
  std::ostringstream contents;
  contents << file.rdbuf();
  out = contents.str();
  return true;
}

bool write_file(const fs::path& path, const std::string& data) {
  std::ofstream file(path, std::ios::binary | std::ios::trunc);
  file << data;
  return static_cast<bool>(file);
}

Analysis analyze(const std::string& database, const std::string& source) {
  const ClangFrontend frontend;
  const FrontendResult result = frontend.analyze(database, {source});
  if (!result.success) {
    return {false, result.error_message, {}};
  }
  Analysis analysis{true, {}, {}};
  const RuleRegistry registry;
  for (const auto& rule : registry.rules()) {
    const RuleEvaluation evaluation = rule->evaluate(result.context);
    if (evaluation.status != EvaluationStatus::Complete) {
      continue;
    }
    for (const Finding& finding : evaluation.findings) {
      analysis.keys.push_back(std::string{finding.message_key} + "@" +
                              std::to_string(finding.location.line) + ":" +
                              std::to_string(finding.location.column));
    }
  }
  return analysis;
}

std::string key_of(const std::string& entry) {
  return entry.substr(0, entry.find('@'));
}

std::map<std::string, int> count_by_key(const std::vector<std::string>& entries) {
  std::map<std::string, int> counts;
  for (const std::string& entry : entries) {
    ++counts[key_of(entry)];
  }
  return counts;
}

const FixClass* select_fix_class(const std::vector<std::string>& entries,
                                 const std::string& requested) {
  for (const FixClass& fix : fix_classes()) {
    if (!requested.empty() && (requested != fix.id)) {
      continue;
    }
    for (const std::string& entry : entries) {
      if (fix.matches(key_of(entry))) {
        return &fix;
      }
    }
  }
  return nullptr;
}

std::string json_escape(const std::string& text) {
  std::string out;
  for (const char c : text) {
    switch (c) {
      case '"': out += "\\\""; break;
      case '\\': out += "\\\\"; break;
      case '\n': out += "\\n"; break;
      case '\r': out += "\\r"; break;
      case '\t': out += "\\t"; break;
      default: out.push_back(c);
    }
  }
  return out;
}

void append_audit(const ConvertOptions& options, const ConvertReport& report,
                  const std::string& before, const std::string& after,
                  const char* fix_class) {
  std::ofstream log(options.output_dir / "audit.jsonl", std::ios::app);
  log << "{\"time\":\"" << today_iso_date() << "\",\"file\":\""
      << json_escape(options.source_file) << "\",\"fix_class\":\""
      << (fix_class != nullptr ? fix_class : "") << "\",\"outcome\":\""
      << outcome_name(report.outcome) << "\",\"detail\":\""
      << json_escape(report.detail) << "\",\"findings_before\":"
      << report.findings_before.size() << ",\"findings_after\":"
      << report.findings_after.size() << ",\"sha256_before\":\""
      << sha256_hex(before) << "\",\"sha256_after\":\"" << sha256_hex(after)
      << "\",\"apply_requested\":" << (options.apply ? "true" : "false")
      << "}\n";
}

std::string build_prompt(const FixClass& fix, const std::string& source,
                         const std::vector<std::string>& findings) {
  std::ostringstream prompt;
  prompt << "You are remediating static-analysis findings in C source code.\n"
         << "Return ONLY the complete corrected file, with no commentary.\n"
         << "Constraints:\n"
         << "- Preserve observable behavior exactly; do not change public "
            "signatures, types, or unrelated code and formatting.\n"
         << "- Make the smallest change that removes the findings.\n"
         << "- Do not introduce new constructs that violate MISRA C:2012.\n"
         << "Fix class: " << fix.id << "\nStrategy: " << fix.strategy << "\n"
         << "Findings (key@line):\n";
  for (const std::string& finding : findings) {
    prompt << "- " << finding << '\n';
  }
  prompt << "----- BEGIN FILE -----\n" << source << "----- END FILE -----\n";
  return prompt.str();
}

// Restores the original source unless released.
class RestoreGuard final {
 public:
  RestoreGuard(fs::path path, std::string original)
      : path_(std::move(path)), original_(std::move(original)) {}
  RestoreGuard(const RestoreGuard&) = delete;
  RestoreGuard& operator=(const RestoreGuard&) = delete;
  ~RestoreGuard() {
    if (active_) {
      write_file(path_, original_);
    }
  }
  void release() { active_ = false; }

 private:
  fs::path path_;
  std::string original_;
  bool active_ = true;
};

ConvertReport finish(const ConvertOptions& options, ConvertReport report,
                     const std::string& before, const std::string& after,
                     const FixClass* fix) {
  append_audit(options, report, before, after, fix != nullptr ? fix->id : nullptr);
  return report;
}

}  // namespace

const std::vector<FixClass>& fix_classes() {
  // Order matters: without --fix-class the first class that matches a finding
  // is used. Deterministic-friendly classes come first.
  static const std::vector<FixClass> classes{
      {"literal-hygiene",
       {"misra-c2012-7.1-", "misra-c2012-7.2-", "misra-c2012-7.3-"},
       "Rewrite only the flagged integer literals: replace octal constants by "
       "the same decimal value, add a U suffix to unsigned constants, and use "
       "an uppercase L suffix. Do not change values, types or other code.",
       false},
      {"goto-elimination",
       {"misra-c2012-15.1-", "misra-c2012-15.2-", "misra-c2012-15.3-"},
       "Replace goto-based control flow with structured constructs "
       "(if/else, loops, early return, or a single exit variable) with "
       "identical behavior.",
       false},
      {"structured-control-flow",
       {"misra-c2012-15.5-", "misra-c2012-15.6-", "misra-c2012-15.7-",
        "misra-c2012-16.3-", "misra-c2012-16.4-", "misra-c2012-16.5-",
        "misra-c2012-16.6-", "misra-c2012-14.2-"},
       "Add braces around single-statement bodies, end else-if chains with a "
       "final else, give every switch a default clause and a break per "
       "clause, and restructure functions to a single return at the end, "
       "without changing behavior.",
       false},
      {"boolean-and-side-effects",
       {"misra-c2012-14.4-", "misra-c2012-13.4-", "misra-c2012-13.5-",
        "misra-c2012-13.6-", "misra-c2012-12.3-"},
       "Make controlling expressions explicit comparisons (for example "
       "x != 0), move assignments out of conditions into separate "
       "statements, hoist side effects out of the right operand of && and "
       "||, and remove comma operators, preserving evaluation order and "
       "behavior.",
       false},
      {"call-and-parameter-hygiene",
       {"misra-c2012-17.7-", "misra-c2012-17.8-", "misra-c2012-2.7-"},
       "Cast discarded call results to void only when ignoring them is "
       "intended, copy modified parameters into a local variable, and mark "
       "intentionally unused parameters; keep signatures and behavior "
       "unchanged.",
       false},
      {"declaration-hygiene",
       {"misra-c2012-8.2-", "misra-c2012-8.8-", "misra-c2012-8.10-",
        "misra-c2012-8.11-", "misra-c2012-8.14-"},
       "Give functions full prototypes with named parameters, repeat static "
       "on every declaration of an internal-linkage entity, make inline "
       "functions static, give extern arrays an explicit size and remove "
       "restrict qualifiers, without changing behavior.",
       false},
      {"pointer-cast-hygiene",
       {"misra-c2012-11.8-", "misra-c2012-11.5-", "misra-c2012-11.9-",
        "misra-c2012-7.4-"},
       "Preserve const qualification in casts, use const char * for string "
       "literals, spell null pointer constants as NULL, and avoid void "
       "pointer to object pointer conversions where a typed pointer is "
       "available, without changing behavior.",
       false},
  };
  return classes;
}

const char* outcome_name(const ConvertOutcome outcome) {
  switch (outcome) {
    case ConvertOutcome::NothingToConvert: return "nothing-to-convert";
    case ConvertOutcome::Rejected: return "rejected";
    case ConvertOutcome::Proposed: return "proposed";
    case ConvertOutcome::Applied: return "applied";
    case ConvertOutcome::Error: return "error";
  }
  return "unknown";
}

std::string extract_proposed_source(const std::string& reply) {
  const std::size_t open = reply.find("```");
  if (open == std::string::npos) {
    return reply;
  }
  const std::size_t body = reply.find('\n', open);
  const std::size_t close = reply.find("```", body == std::string::npos ? open : body);
  if ((body == std::string::npos) || (close == std::string::npos)) {
    return reply;
  }
  return reply.substr(body + 1U, close - body - 1U);
}

ConvertReport convert_file(const ConvertOptions& options) {
  std::error_code ec;
  fs::create_directories(options.output_dir, ec);

  std::string original;
  if (!read_file(options.source_file, original)) {
    return {ConvertOutcome::Error, "cannot read " + options.source_file, {}, {}, {}, {}};
  }
  if (options.provider_command.empty()) {
    return {ConvertOutcome::Error, "no AI provider command configured", {}, {}, {}, {}};
  }

  const Analysis before = analyze(options.compilation_database, options.source_file);
  if (!before.ok) {
    return {ConvertOutcome::Error, "baseline analysis failed: " + before.error, {}, {}, {}, {}};
  }
  ConvertReport report{ConvertOutcome::NothingToConvert, "no findings in a supported fix class",
                       before.keys, {}, {}, {}};
  const FixClass* fix = select_fix_class(before.keys, options.fix_class);
  if (fix == nullptr) {
    return finish(options, report, original, original, nullptr);
  }
  if (options.apply && !fix->auto_apply_approved) {
    report.outcome = ConvertOutcome::Error;
    report.detail = std::string{"fix class '"} + fix->id +
                    "' is not approved for automatic application; use suggestion mode";
    return finish(options, report, original, original, fix);
  }

  // Ask the provider.
  const fs::path stem = options.output_dir / fs::path(options.source_file).filename();
  const fs::path prompt_path = fs::path(stem).concat(".prompt.txt");
  const fs::path reply_path = fs::path(stem).concat(".reply.txt");
  write_file(prompt_path, build_prompt(*fix, original, before.keys));
  const std::string command = options.provider_command + " < '" + prompt_path.string() +
                              "' > '" + reply_path.string() + "'";
  if (std::system(command.c_str()) != 0) {
    report.outcome = ConvertOutcome::Error;
    report.detail = "AI provider command failed";
    return finish(options, report, original, original, fix);
  }
  std::string reply;
  read_file(reply_path, reply);
  const std::string proposed = extract_proposed_source(reply);
  if (proposed.empty() || (proposed == original)) {
    report.outcome = ConvertOutcome::Rejected;
    report.detail = "provider returned no change";
    return finish(options, report, original, original, fix);
  }

  // Trial the proposal in place; the guard restores the source on any exit
  // path other than an approved apply.
  RestoreGuard guard(options.source_file, original);
  write_file(options.source_file, proposed);

  const Analysis after = analyze(options.compilation_database, options.source_file);
  if (!after.ok) {
    report.outcome = ConvertOutcome::Rejected;
    report.detail = "gate failed: proposal does not compile: " + after.error;
    return finish(options, report, original, proposed, fix);
  }
  report.findings_after = after.keys;

  const std::map<std::string, int> counts_before = count_by_key(before.keys);
  const std::map<std::string, int> counts_after = count_by_key(after.keys);
  for (const auto& [key, count] : counts_after) {
    const auto previous = counts_before.find(key);
    const int allowed = (previous == counts_before.end()) ? 0 : previous->second;
    if (count > allowed) {
      report.outcome = ConvertOutcome::Rejected;
      report.detail = "gate failed: new or increased finding " + key;
      return finish(options, report, original, proposed, fix);
    }
  }
  for (const auto& [key, count] : counts_before) {
    (void)count;
    if (fix->matches(key) && (counts_after.count(key) != 0U)) {
      report.outcome = ConvertOutcome::Rejected;
      report.detail = "gate failed: targeted finding remains: " + key;
      return finish(options, report, original, proposed, fix);
    }
  }
  if (!options.verify_command.empty() && (std::system(options.verify_command.c_str()) != 0)) {
    report.outcome = ConvertOutcome::Rejected;
    report.detail = "gate failed: verify command returned non-zero";
    return finish(options, report, original, proposed, fix);
  }

  report.proposal_path = fs::path(stem).concat(".proposed");
  report.patch_path = fs::path(stem).concat(".patch");
  write_file(report.proposal_path, proposed);
  const std::string diff = "diff -u '" + fs::path(stem).concat(".orig").string() + "' '" +
                           report.proposal_path.string() + "' > '" +
                           report.patch_path.string() + "'";
  const fs::path original_copy = fs::path(stem).concat(".orig");
  write_file(original_copy, original);
  (void)std::system(diff.c_str());  // exit 1 simply means "files differ"
  fs::remove(original_copy, ec);

  if (options.apply) {
    guard.release();
    report.outcome = ConvertOutcome::Applied;
    report.detail = "all gates passed; change applied";
  } else {
    report.outcome = ConvertOutcome::Proposed;
    report.detail = "all gates passed; proposal written, source unchanged";
  }
  return finish(options, report, original, proposed, fix);
}

}  // namespace misra
