#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace misra {

// Remediation classes the converter may work on. A class is advisory
// (suggestion-only) until independent validation sets auto_apply_approved.
struct FixClass final {
  const char* id;
  std::vector<std::string> message_key_prefixes;
  const char* strategy;  // paraphrased guidance sent to the AI provider
  bool auto_apply_approved;

  [[nodiscard]] bool matches(const std::string& message_key) const {
    for (const std::string& prefix : message_key_prefixes) {
      if (message_key.rfind(prefix, 0) == 0) {
        return true;
      }
    }
    return false;
  }
};

[[nodiscard]] const std::vector<FixClass>& fix_classes();

struct ConvertOptions final {
  std::string compilation_database;
  std::string source_file;
  std::filesystem::path output_dir;
  // Shell command: reads the prompt on stdin, writes the full proposed file
  // on stdout. Must be provided by the operator (cloud or on-prem model).
  std::string provider_command;
  // Optional shell command run after the built-in gates; non-zero exit fails.
  std::string verify_command;
  // Restrict the run to one fix class id; empty selects the first match.
  std::string fix_class;
  bool apply = false;
};

enum class ConvertOutcome {
  NothingToConvert,
  Rejected,   // a gate failed; source restored
  Proposed,   // all gates passed; proposal written, source untouched
  Applied,    // all gates passed and change kept (approved classes only)
  Error,
};

struct ConvertReport final {
  ConvertOutcome outcome;
  std::string detail;
  std::vector<std::string> findings_before;
  std::vector<std::string> findings_after;
  std::filesystem::path proposal_path;
  std::filesystem::path patch_path;
};

[[nodiscard]] ConvertReport convert_file(const ConvertOptions& options);
[[nodiscard]] const char* outcome_name(ConvertOutcome outcome);

// Extracts the full-file replacement from a provider reply (strips Markdown
// code fences when present).
[[nodiscard]] std::string extract_proposed_source(const std::string& reply);

}  // namespace misra
