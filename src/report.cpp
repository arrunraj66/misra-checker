#include "misra/report.hpp"

#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <set>
#include <utility>

#include "llvm/Support/FormatVariadic.h"
#include "llvm/Support/JSON.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/raw_ostream.h"

namespace misra {
namespace {

bool is_rule_id(const std::string& text) {
  const std::size_t dot = text.find('.');
  if ((dot == std::string::npos) || (dot == 0U) || ((dot + 1U) >= text.size())) {
    return false;
  }
  return std::all_of(text.begin(), text.end(),
                     [](const char c) { return ((c >= '0') && (c <= '9')) || (c == '.'); }) &&
         (text.find('.', dot + 1U) == std::string::npos);
}

bool is_date(const std::string& text) {
  if ((text.size() != 10U) || (text[4] != '-') || (text[7] != '-')) {
    return false;
  }
  for (std::size_t i = 0U; i < text.size(); ++i) {
    if ((i != 4U) && (i != 7U) && ((text[i] < '0') || (text[i] > '9'))) {
      return false;
    }
  }
  return true;
}

std::string normalise(std::string path) {
  std::replace(path.begin(), path.end(), '\\', '/');
  return path;
}

bool glob_match(const std::string& pattern, const std::string& text) {
  std::size_t p = 0U;
  std::size_t t = 0U;
  std::size_t star = std::string::npos;
  std::size_t mark = 0U;
  while (t < text.size()) {
    if ((p < pattern.size()) && (pattern[p] == '*')) {
      star = p++;
      mark = t;
    } else if ((p < pattern.size()) && (pattern[p] == text[t])) {
      ++p;
      ++t;
    } else if (star != std::string::npos) {
      p = star + 1U;
      t = ++mark;
    } else {
      return false;
    }
  }
  while ((p < pattern.size()) && (pattern[p] == '*')) {
    ++p;
  }
  return p == pattern.size();
}

// A plain pattern matches a path suffix at a directory boundary; a pattern with
// '*' is matched against the whole path and against every suffix after a '/'.
bool file_matches(const std::string& pattern_in, const std::string& path_in) {
  if (pattern_in.empty()) {
    return true;
  }
  const std::string pattern = normalise(pattern_in);
  const std::string path = normalise(path_in);
  const bool wildcard = pattern.find('*') != std::string::npos;
  if (!wildcard) {
    if (path == pattern) {
      return true;
    }
    return (path.size() > pattern.size()) &&
           (path.compare(path.size() - pattern.size(), pattern.size(), pattern) == 0) &&
           (path[path.size() - pattern.size() - 1U] == '/');
  }
  if (glob_match(pattern, path)) {
    return true;
  }
  for (std::size_t slash = path.find('/'); slash != std::string::npos;
       slash = path.find('/', slash + 1U)) {
    if (glob_match(pattern, path.substr(slash + 1U))) {
      return true;
    }
  }
  return false;
}

std::string level_for(const std::string& category) {
  if (category == "mandatory") {
    return "error";
  }
  if (category == "required") {
    return "warning";
  }
  return "note";
}

std::string message_for(const ReportRow& row) {
  std::string text = row.key;
  const std::string prefix = "misra-c2012-" + row.rule + "-";
  if (text.rfind(prefix, 0U) == 0U) {
    text = text.substr(prefix.size());
  }
  std::replace(text.begin(), text.end(), '-', ' ');
  if (!text.empty()) {
    text[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(text[0])));
  }
  return text + " (MISRA C:2012 Rule " + row.rule + ")";
}

std::string uri_for(const std::string& file, const std::string& root, bool& relative) {
  const std::string path = normalise(file);
  std::string base = normalise(root);
  relative = false;
  if (!base.empty()) {
    if (base.back() != '/') {
      base += '/';
    }
    if (path.rfind(base, 0U) == 0U) {
      relative = true;
      return path.substr(base.size());
    }
  }
  return path;
}

}  // namespace

std::string today_iso() {
  if (const char* forced = std::getenv("MISRA_TODAY")) {  // test aid
    return forced;
  }
  const std::time_t now = std::time(nullptr);
  std::tm parts{};
  localtime_r(&now, &parts);
  char buffer[16];
  std::strftime(buffer, sizeof(buffer), "%Y-%m-%d", &parts);
  return buffer;
}

DeviationSet load_deviations(const std::string& path,
                             const std::map<std::string, std::string>& categories) {
  DeviationSet set;
  auto buffer = llvm::MemoryBuffer::getFile(path);
  if (!buffer) {
    set.errors.push_back("cannot read deviation file '" + path + "'");
    return set;
  }
  auto parsed = llvm::json::parse((*buffer)->getBuffer());
  if (!parsed) {
    set.errors.push_back("deviation file is not valid JSON: " +
                         llvm::toString(parsed.takeError()));
    return set;
  }
  const llvm::json::Object* root = parsed->getAsObject();
  const llvm::json::Array* list = (root != nullptr) ? root->getArray("deviations") : nullptr;
  if (list == nullptr) {
    set.errors.push_back("deviation file must be an object with a \"deviations\" array");
    return set;
  }
  std::set<std::string> ids;
  std::size_t index = 0U;
  for (const llvm::json::Value& entry : *list) {
    ++index;
    const llvm::json::Object* object = entry.getAsObject();
    const std::string where = "deviation #" + std::to_string(index);
    if (object == nullptr) {
      set.errors.push_back(where + ": must be an object");
      continue;
    }
    Deviation deviation;
    const auto text = [&](const char* name) {
      const auto value = object->getString(name);
      return value ? value->str() : std::string{};
    };
    deviation.id = text("id");
    deviation.justification = text("justification");
    deviation.approved_by = text("approved_by");
    deviation.file = text("file");
    deviation.expires = text("expires");
    const std::string label = deviation.id.empty() ? where : "deviation " + deviation.id;
    if (deviation.id.empty()) {
      set.errors.push_back(where + ": \"id\" is required");
    } else if (!ids.insert(deviation.id).second) {
      set.errors.push_back(label + ": duplicate id");
    }
    if (deviation.justification.empty()) {
      set.errors.push_back(label + ": \"justification\" is required");
    }
    if (deviation.approved_by.empty()) {
      set.errors.push_back(label + ": \"approved_by\" is required");
    }
    if (!deviation.expires.empty() && !is_date(deviation.expires)) {
      set.errors.push_back(label + ": \"expires\" must be YYYY-MM-DD");
    }
    const llvm::json::Array* rule_list = object->getArray("rules");
    if ((rule_list == nullptr) || rule_list->empty()) {
      set.errors.push_back(label + ": \"rules\" must list at least one rule");
    } else {
      for (const llvm::json::Value& rule : *rule_list) {
        const auto name = rule.getAsString();
        if (!name || !is_rule_id(name->str())) {
          set.errors.push_back(label + ": invalid rule id in \"rules\"");
          continue;
        }
        const auto found = categories.find(name->str());
        if (found == categories.end()) {
          set.errors.push_back(label + ": unknown rule " + name->str());
        } else if (found->second == "mandatory") {
          set.errors.push_back(label + ": rule " + name->str() +
                               " is mandatory and cannot be deviated");
        } else {
          deviation.rules.push_back(name->str());
        }
      }
    }
    if (const llvm::json::Array* lines = object->getArray("lines")) {
      const auto bound = [&](const std::size_t position) -> int64_t {
        if (lines->size() != 2U) {
          return -1;
        }
        const auto value = (*lines)[position].getAsInteger();
        return value ? static_cast<int64_t>(*value) : -1;
      };
      const int64_t from = bound(0U);
      const int64_t to = bound(1U);
      if ((from < 1) || (to < from)) {
        set.errors.push_back(label + ": \"lines\" must be [from, to] with 1 <= from <= to");
      } else {
        deviation.line_from = static_cast<unsigned int>(from);
        deviation.line_to = static_cast<unsigned int>(to);
      }
    }
    set.deviations.push_back(std::move(deviation));
  }
  return set;
}

DeviationOutcome apply_deviations(std::vector<ReportRow> rows,
                                  const std::vector<Deviation>& deviations,
                                  const std::string& today) {
  DeviationOutcome outcome;
  std::set<std::string> used;
  std::set<std::string> expired;
  for (ReportRow& row : rows) {
    const Deviation* match = nullptr;
    for (const Deviation& deviation : deviations) {
      if (!deviation.expires.empty() && (deviation.expires < today)) {
        expired.insert(deviation.id);
        continue;
      }
      if (std::find(deviation.rules.begin(), deviation.rules.end(), row.rule) ==
          deviation.rules.end()) {
        continue;
      }
      if (!file_matches(deviation.file, row.file)) {
        continue;
      }
      if ((deviation.line_from != 0U) &&
          ((row.line < deviation.line_from) || (row.line > deviation.line_to))) {
        continue;
      }
      match = &deviation;
      break;
    }
    if (match != nullptr) {
      row.deviation_id = match->id;
      row.justification = match->justification;
      used.insert(match->id);
      outcome.deviated.push_back(std::move(row));
    } else {
      outcome.active.push_back(std::move(row));
    }
  }
  for (const Deviation& deviation : deviations) {
    if (expired.count(deviation.id) != 0U) {
      outcome.expired.push_back(deviation.id);
    } else if (used.count(deviation.id) == 0U) {
      outcome.unused.push_back(deviation.id);
    }
  }
  return outcome;
}

std::string to_sarif(const DeviationOutcome& outcome, const SarifOptions& options) {
  using llvm::json::Array;
  using llvm::json::Object;
  using llvm::json::Value;

  std::vector<const ReportRow*> all;
  for (const ReportRow& row : outcome.active) {
    all.push_back(&row);
  }
  for (const ReportRow& row : outcome.deviated) {
    all.push_back(&row);
  }

  std::map<std::string, const ReportRow*> rule_sample;
  for (const ReportRow* row : all) {
    rule_sample.emplace(row->rule, row);
  }
  Array rules;
  std::map<std::string, std::size_t> rule_index;
  for (const auto& [rule, row] : rule_sample) {
    rule_index[rule] = rules.size();
    rules.push_back(Object{
        {"id", "misra-c2012-" + rule},
        {"name", "MISRA-C-2012-" + rule},
        {"shortDescription", Object{{"text", "MISRA C:2012 Rule " + rule + ": " + row->topic}}},
        {"defaultConfiguration", Object{{"level", level_for(row->category)}}},
        {"properties", Object{{"category", row->category},
                              {"tags", Array{"misra", "misra-c2012", row->category}}}}});
  }

  Array results;
  for (const ReportRow* row : all) {
    bool relative = false;
    const std::string uri = uri_for(row->file, options.source_root, relative);
    Object artifact{{"uri", uri}};
    if (relative) {
      artifact["uriBaseId"] = "SRCROOT";
    }
    Object result{
        {"ruleId", "misra-c2012-" + row->rule},
        {"ruleIndex", static_cast<int64_t>(rule_index[row->rule])},
        {"level", level_for(row->category)},
        {"message", Object{{"text", message_for(*row)}}},
        {"locations",
         Array{Object{{"physicalLocation",
                       Object{{"artifactLocation", std::move(artifact)},
                              {"region", Object{{"startLine", static_cast<int64_t>(row->line)},
                                                {"startColumn", static_cast<int64_t>(row->column)}}}}}}}},
        {"properties", Object{{"check", row->key}, {"category", row->category}}}};
    if (!row->deviation_id.empty()) {
      result["suppressions"] = Array{Object{{"kind", "external"},
                                            {"status", "accepted"},
                                            {"justification", row->justification},
                                            {"properties", Object{{"deviationId", row->deviation_id}}}}};
    }
    results.push_back(std::move(result));
  }

  Array notifications;
  const auto notify = [&](const std::string& level, const std::string& text) {
    notifications.push_back(Object{{"level", level}, {"message", Object{{"text", text}}}});
  };
  if (options.compile_errors) {
    notify("warning", "Clang reported compile errors; analysis continued on the recovered AST and results may be incomplete.");
  }
  for (const std::string& id : outcome.expired) {
    notify("warning", "Deviation " + id + " has expired and was not applied.");
  }
  for (const std::string& id : outcome.unused) {
    notify("note", "Deviation " + id + " matched no finding.");
  }
  for (const std::string& note : options.notes) {
    notify("note", note);
  }

  Object run{
      {"tool", Object{{"driver", Object{{"name", "misra-checker"},
                                         {"version", options.tool_version},
                                         {"informationUri", "https://github.com/arrunraj66/misra-checker"},
                                         {"rules", std::move(rules)}}}}},
      {"results", std::move(results)},
      {"invocations", Array{Object{{"executionSuccessful", true},
                                   {"toolExecutionNotifications", std::move(notifications)}}}}};
  if (!options.source_root.empty()) {
    std::string base = normalise(options.source_root);
    if (base.back() != '/') {
      base += '/';
    }
    run["originalUriBaseIds"] = Object{{"SRCROOT", Object{{"uri", "file://" + base}}}};
  }

  Value log = Object{
      {"$schema", "https://json.schemastore.org/sarif-2.1.0.json"},
      {"version", "2.1.0"},
      {"runs", Array{std::move(run)}}};
  std::string text;
  llvm::raw_string_ostream stream(text);
  stream << llvm::formatv("{0:2}", log) << '\n';
  return stream.str();
}

}  // namespace misra
