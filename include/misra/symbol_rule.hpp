#pragma once

#include <map>
#include <set>
#include <string>
#include <string_view>
#include <vector>

#include "misra/rule.hpp"

namespace misra {

[[nodiscard]] inline std::string location_key(const SourceLocation& location) {
  return location.file + ":" + std::to_string(location.line) + ":" +
         std::to_string(location.column);
}

// Whole-program symbol rules need facts from more than one translation unit.
[[nodiscard]] inline bool has_multiple_units(const AnalysisContext& context) {
  std::set<std::string> units(context.translation_units.begin(),
                              context.translation_units.end());
  return units.size() > 1U;
}

// Facts grouped by name, with duplicate sightings of one location (e.g. a
// header seen from several translation units) merged per role.
using SymbolGroups = std::map<std::string, std::vector<const SymbolFact*>>;

[[nodiscard]] inline SymbolGroups group_symbols(const AnalysisContext& context,
                                                const bool external) {
  SymbolGroups groups;
  std::set<std::string> seen;
  for (const SymbolFact& fact : context.symbols) {
    if (fact.external_linkage != external) {
      continue;
    }
    const std::string key = fact.name + "|" + std::to_string(static_cast<int>(fact.role)) +
                            "|" + location_key(fact.location) + "|" +
                            (fact.role == SymbolRole::Reference ? fact.translation_unit : "");
    if (seen.insert(key).second) {
      groups[fact.name].push_back(&fact);
    }
  }
  return groups;
}

}  // namespace misra
