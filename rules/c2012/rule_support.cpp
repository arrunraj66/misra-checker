#include "misra/c2012/rule_support.hpp"

#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <tuple>
#include <vector>

namespace misra::c2012::support {
namespace {

RuleEvaluation complete() { return {EvaluationStatus::Complete, {}}; }

std::string where(const SourceLocation& l) {
  return l.file + ':' + std::to_string(l.line) + ':' + std::to_string(l.column);
}

bool before(const SourceLocation& a, const SourceLocation& b) {
  if (a.file != b.file) return a.file < b.file;
  if (a.line != b.line) return a.line < b.line;
  return a.column < b.column;
}

bool is_entity(const DeclarationFact& d) {
  return d.kind == DeclKind::Function || d.kind == DeclKind::Object;
}

// One entry per distinct (kind, name, location): headers are seen once per
// translation unit that includes them.
std::vector<const DeclarationFact*> unique_declarations(
    const AnalysisContext& context, bool (*keep)(const DeclarationFact&)) {
  std::set<std::string> seen;
  std::vector<const DeclarationFact*> result;
  for (const DeclarationFact& d : context.program.declarations) {
    if (keep(d) && seen.insert(declaration_key(d.location, d.name)).second) {
      result.push_back(&d);
    }
  }
  std::sort(result.begin(), result.end(),
            [](const DeclarationFact* a, const DeclarationFact* b) {
              return before(a->location, b->location);
            });
  return result;
}

// Two declarations of one entity describe compatible types when the element
// types agree and every array bound is equal or absent from at least one.
bool types_compatible(const DeclarationFact& a, const DeclarationFact& b) {
  if (a.type != b.type || a.array_dimensions.size() != b.array_dimensions.size()) {
    return false;
  }
  for (std::size_t i = 0U; i < a.array_dimensions.size(); ++i) {
    const long long x = a.array_dimensions[i];
    const long long y = b.array_dimensions[i];
    if (x >= 0 && y >= 0 && x != y) {
      return false;
    }
  }
  return true;
}

constexpr std::size_t kExternalSignificant = 31U;
constexpr std::size_t kMacroSignificant = 63U;

}  // namespace

RuleEvaluation syntax_findings(const AnalysisContext& context,
                               const std::string_view key) {
  RuleEvaluation e = complete();
  for (const SyntaxFact& f : context.syntax) {
    if (f.kind == key) {
      e.findings.push_back({key, f.location, FindingCertainty::Definite});
    }
  }
  return e;
}

RuleEvaluation call_findings(
    const AnalysisContext& context,
    const std::initializer_list<std::string_view> callees,
    const std::string_view key) {
  RuleEvaluation e = complete();
  for (const CallFact& call : context.calls) {
    if (call.callee_in_system_header &&
        std::find(callees.begin(), callees.end(), call.callee) !=
            callees.end()) {
      e.findings.push_back({key, call.location, FindingCertainty::Definite});
    }
  }
  return e;
}

RuleEvaluation include_findings(
    const AnalysisContext& context,
    const std::initializer_list<std::string_view> headers,
    const std::string_view key) {
  RuleEvaluation e = complete();
  for (const IncludeFact& include : context.includes) {
    const std::size_t slash = include.header.find_last_of('/');
    const std::string_view base =
        std::string_view(include.header)
            .substr(slash == std::string::npos ? 0U : slash + 1U);
    if (std::find(headers.begin(), headers.end(), base) != headers.end()) {
      e.findings.push_back({key, include.location, FindingCertainty::Definite});
    }
  }
  return e;
}

RuleEvaluation non_boolean_condition_findings(const AnalysisContext& context,
                                              const std::string_view key) {
  RuleEvaluation e = complete();
  for (const ControllingExpressionFact& f : context.controlling_expressions) {
    if (!f.essentially_boolean) {
      e.findings.push_back({key, f.location, FindingCertainty::Definite});
    }
  }
  return e;
}

RuleEvaluation multiple_loop_exit_findings(const AnalysisContext& context,
                                           const std::string_view key) {
  RuleEvaluation e = complete();
  for (const LoopExitFact& f : context.loop_exits) {
    if (f.terminating_jumps > 1U) {
      e.findings.push_back({key, f.location, FindingCertainty::Definite});
    }
  }
  return e;
}

RuleEvaluation function_exit_findings(const AnalysisContext& context,
                                      const std::string_view key) {
  RuleEvaluation e = complete();
  for (const FunctionExitFact& f : context.function_exits) {
    if (f.return_count > 1U || (f.return_count == 1U && !f.ends_with_return)) {
      e.findings.push_back({key, f.location, FindingCertainty::Definite});
    }
  }
  return e;
}

// ------------------------------------------------------------------- scopes

RuleEvaluation identifiers_not_distinct_in_scope(const AnalysisContext& context) {
  RuleEvaluation e = complete();
  using Key = std::tuple<std::size_t, std::size_t, int, std::string>;
  std::map<Key, std::map<std::string, std::vector<const IdentifierFact*>>> groups;
  for (const IdentifierFact& f : context.identifiers) {
    groups[{f.translation_unit, f.scope, static_cast<int>(f.space),
            f.name.substr(0U, kExternalSignificant)}][f.name]
        .push_back(&f);
  }
  std::set<std::string> reported;
  for (const auto& group : groups) {
    bool first = true;
    for (const auto& names : group.second) {
      if (!first) {
        for (const IdentifierFact* f : names.second) {
          if (reported.insert(declaration_key(f->location, f->name)).second) {
            e.findings.push_back({"misra-c2012-5.2-identifiers-not-distinct",
                                  f->location, FindingCertainty::Definite});
          }
        }
      }
      first = false;
    }
  }
  return e;
}

RuleEvaluation identifier_hides_outer(const AnalysisContext& context) {
  RuleEvaluation e = complete();
  using Key = std::tuple<std::size_t, int, std::string>;  // scope, space, name
  std::map<Key, std::vector<const IdentifierFact*>> index;
  for (const IdentifierFact& f : context.identifiers) {
    if (f.space != IdentifierSpace::Member) {
      index[{f.scope, static_cast<int>(f.space),
             f.name.substr(0U, kExternalSignificant)}]
          .push_back(&f);
    }
  }
  std::set<std::string> reported;
  for (const IdentifierFact& inner : context.identifiers) {
    if (inner.space == IdentifierSpace::Member) continue;
    const auto parent_of = context.scope_parents.find(inner.scope);
    std::size_t outer = parent_of == context.scope_parents.end() ? 0U : parent_of->second;
    bool hides = false;
    while (outer != 0U && !hides) {
      const auto found = index.find({outer, static_cast<int>(inner.space),
                                     inner.name.substr(0U, kExternalSignificant)});
      if (found != index.end()) {
        for (const IdentifierFact* candidate : found->second) {
          // The outer declaration must precede the inner one. Locations in
          // different files cannot be ordered here; a header is assumed to
          // come first.
          if (candidate->translation_unit == inner.translation_unit &&
              (candidate->location.file != inner.location.file ||
               before(candidate->location, inner.location))) {
            hides = true;
            break;
          }
        }
      }
      const auto next = context.scope_parents.find(outer);
      outer = next == context.scope_parents.end() ? 0U : next->second;
    }
    if (hides &&
        reported.insert(declaration_key(inner.location, inner.name)).second) {
      e.findings.push_back({"misra-c2012-5.3-identifier-hides-outer",
                            inner.location, FindingCertainty::Definite});
    }
  }
  return e;
}

// ------------------------------------------------------------- whole program

RuleEvaluation external_identifier_clash(const AnalysisContext& context) {
  RuleEvaluation e = complete();
  const auto decls = unique_declarations(context, [](const DeclarationFact& d) {
    return is_entity(d) && d.linkage == LinkageKind::External &&
           d.name.size() >= kExternalSignificant;
  });
  std::map<std::string, std::map<std::string, const DeclarationFact*>> groups;
  for (const DeclarationFact* d : decls) {
    groups[d->name.substr(0U, kExternalSignificant)].emplace(d->name, d);
  }
  for (const auto& group : groups) {
    bool first = true;
    for (const auto& member : group.second) {
      if (!first) {
        e.findings.push_back({"misra-c2012-5.1-external-identifier-clash",
                              member.second->location,
                              FindingCertainty::Definite});
      }
      first = false;
    }
  }
  return e;
}

RuleEvaluation macro_identifier_clash(const AnalysisContext& context) {
  RuleEvaluation e = complete();
  std::map<std::size_t, std::map<std::string, std::map<std::string, SourceLocation>>> tus;
  for (const MacroFact& m : context.program.macros) {
    if (m.name.size() >= kMacroSignificant) {
      tus[m.translation_unit][m.name.substr(0U, kMacroSignificant)].emplace(
          m.name, m.location);
    }
  }
  std::set<std::string> reported;
  for (const auto& tu : tus) {
    for (const auto& group : tu.second) {
      bool first = true;
      for (const auto& member : group.second) {
        if (!first && reported.insert(where(member.second)).second) {
          e.findings.push_back({"misra-c2012-5.4-macro-identifier-clash",
                                member.second, FindingCertainty::Definite});
        }
        first = false;
      }
    }
  }
  return e;
}

RuleEvaluation identifier_macro_clash(const AnalysisContext& context) {
  RuleEvaluation e = complete();
  std::set<std::string> reported;
  for (const MacroFact& m : context.program.macros) {
    for (const DeclarationFact& d : context.program.declarations) {
      if (d.translation_unit != m.translation_unit) continue;
      if (m.name.substr(0U, kExternalSignificant) ==
              d.name.substr(0U, kExternalSignificant) &&
          reported.insert(declaration_key(d.location, d.name)).second) {
        e.findings.push_back({"misra-c2012-5.5-identifier-macro-clash",
                              d.location, FindingCertainty::Definite});
      }
    }
  }
  return e;
}

namespace {

RuleEvaluation duplicate_names(const AnalysisContext& context, DeclKind kind,
                               const std::string_view key) {
  RuleEvaluation e = complete();
  std::map<std::string, std::vector<const DeclarationFact*>> by_name;
  const auto all = unique_declarations(context, [](const DeclarationFact&) {
    return true;
  });
  for (const DeclarationFact* d : all) {
    if (d->kind == kind) by_name[d->name].push_back(d);
  }
  for (const auto& entry : by_name) {
    for (std::size_t i = 1U; i < entry.second.size(); ++i) {
      e.findings.push_back(
          {key, entry.second[i]->location, FindingCertainty::Definite});
    }
  }
  return e;
}

}  // namespace

RuleEvaluation duplicate_typedef_names(const AnalysisContext& context) {
  return duplicate_names(context, DeclKind::Typedef,
                         "misra-c2012-5.6-duplicate-typedef-name");
}

RuleEvaluation duplicate_tag_names(const AnalysisContext& context) {
  return duplicate_names(context, DeclKind::Tag,
                         "misra-c2012-5.7-duplicate-tag-name");
}

RuleEvaluation external_internal_name_clash(const AnalysisContext& context) {
  RuleEvaluation e = complete();
  std::set<std::string> external_definitions;
  for (const DeclarationFact& d : context.program.declarations) {
    if (is_entity(d) && d.linkage == LinkageKind::External && d.is_definition) {
      external_definitions.insert(d.name);
    }
  }
  for (const DeclarationFact* d : unique_declarations(
           context, [](const DeclarationFact& x) {
             return is_entity(x) && x.linkage == LinkageKind::Internal;
           })) {
    if (external_definitions.count(d->name) != 0U) {
      e.findings.push_back({"misra-c2012-5.8-external-name-reused",
                            d->location, FindingCertainty::Definite});
    }
  }
  return e;
}

RuleEvaluation internal_name_not_unique(const AnalysisContext& context) {
  RuleEvaluation e = complete();
  std::map<std::string, std::vector<const DeclarationFact*>> by_name;
  for (const DeclarationFact* d : unique_declarations(
           context, [](const DeclarationFact& x) {
             return is_entity(x) && x.linkage == LinkageKind::Internal;
           })) {
    by_name[d->name].push_back(d);
  }
  for (const auto& entry : by_name) {
    const std::string& first_file = entry.second.front()->location.file;
    for (const DeclarationFact* d : entry.second) {
      if (d->location.file != first_file) {
        e.findings.push_back({"misra-c2012-5.9-internal-name-not-unique",
                              d->location, FindingCertainty::Definite});
      }
    }
  }
  return e;
}

namespace {

RuleEvaluation unused_declarations(const AnalysisContext& context,
                                   DeclKind kind, const std::string_view key) {
  RuleEvaluation e = complete();
  for (const DeclarationFact* d : unique_declarations(
           context, [](const DeclarationFact&) { return true; })) {
    if (d->kind == kind &&
        context.program.referenced_declarations.count(
            declaration_key(d->location, d->name)) == 0U) {
      e.findings.push_back({key, d->location, FindingCertainty::Definite});
    }
  }
  return e;
}

}  // namespace

RuleEvaluation unused_typedefs(const AnalysisContext& context) {
  return unused_declarations(context, DeclKind::Typedef,
                             "misra-c2012-2.3-unused-typedef");
}

RuleEvaluation unused_tags(const AnalysisContext& context) {
  return unused_declarations(context, DeclKind::Tag,
                             "misra-c2012-2.4-unused-tag");
}

RuleEvaluation unused_macros(const AnalysisContext& context) {
  RuleEvaluation e = complete();
  std::set<std::string> reported;
  for (const MacroFact& m : context.program.macros) {
    if (context.program.used_macros.count(m.name) == 0U &&
        reported.insert(declaration_key(m.location, m.name)).second) {
      e.findings.push_back(
          {"misra-c2012-2.5-unused-macro", m.location, FindingCertainty::Definite});
    }
  }
  return e;
}

RuleEvaluation inconsistent_declarations(const AnalysisContext& context) {
  RuleEvaluation e = complete();
  std::map<std::string, std::vector<const DeclarationFact*>> by_name;
  for (const DeclarationFact* d : unique_declarations(
           context, [](const DeclarationFact& x) {
             return is_entity(x) && x.linkage == LinkageKind::External;
           })) {
    by_name[d->name].push_back(d);
  }
  for (const auto& entry : by_name) {
    for (std::size_t i = 1U; i < entry.second.size(); ++i) {
      const DeclarationFact& other = *entry.second[i];
      // Compatibility is not transitive (a[] ~ a[4], a[] ~ a[5], a[4] !~ a[5]),
      // so compare against every earlier declaration.
      bool mismatch = false;
      for (std::size_t j = 0U; j < i && !mismatch; ++j) {
        mismatch = !types_compatible(*entry.second[j], other);
      }
      if (mismatch) {
        e.findings.push_back({"misra-c2012-8.3-type-mismatch", other.location,
                              FindingCertainty::Definite});
        continue;
      }
      const DeclarationFact& base = *entry.second.front();
      const std::size_t n =
          std::min(base.parameter_names.size(), other.parameter_names.size());
      for (std::size_t p = 0U; p < n; ++p) {
        if (!base.parameter_names[p].empty() &&
            !other.parameter_names[p].empty() &&
            base.parameter_names[p] != other.parameter_names[p]) {
          e.findings.push_back({"misra-c2012-8.3-parameter-name-mismatch",
                                other.location, FindingCertainty::Definite});
          break;
        }
      }
    }
  }
  return e;
}

RuleEvaluation declared_in_multiple_files(const AnalysisContext& context) {
  RuleEvaluation e = complete();
  std::map<std::string, std::vector<const DeclarationFact*>> by_name;
  for (const DeclarationFact* d : unique_declarations(
           context, [](const DeclarationFact& x) {
             return is_entity(x) && x.linkage == LinkageKind::External &&
                    !x.is_definition;
           })) {
    by_name[d->name].push_back(d);
  }
  for (const auto& entry : by_name) {
    const std::string& first_file = entry.second.front()->location.file;
    for (const DeclarationFact* d : entry.second) {
      if (d->location.file != first_file) {
        e.findings.push_back({"misra-c2012-8.5-declared-in-multiple-files",
                              d->location, FindingCertainty::Definite});
      }
    }
  }
  return e;
}

RuleEvaluation definition_count(const AnalysisContext& context) {
  RuleEvaluation e = complete();
  std::map<std::string, std::map<std::size_t, const DeclarationFact*>> defs;
  std::map<std::string, const DeclarationFact*> first_declaration;
  for (const DeclarationFact& d : context.program.declarations) {
    if (!is_entity(d) || d.linkage != LinkageKind::External) continue;
    first_declaration.emplace(d.name, &d);
    if (d.is_definition) defs[d.name].emplace(d.translation_unit, &d);
  }
  std::set<std::string> reported;
  for (const auto& entry : defs) {
    bool first = true;
    for (const auto& unit : entry.second) {
      if (!first && reported.insert(where(unit.second->location)).second) {
        e.findings.push_back({"misra-c2012-8.6-multiple-definitions",
                              unit.second->location,
                              FindingCertainty::Definite});
      }
      first = false;
    }
  }
  std::set<std::string> referenced;
  for (const ReferenceFact& r : context.program.references) {
    referenced.insert(r.name);
  }
  for (const auto& entry : first_declaration) {
    if (defs.count(entry.first) == 0U && referenced.count(entry.first) != 0U) {
      // The definition may legitimately live in a library outside the
      // analysed program.
      e.findings.push_back({"misra-c2012-8.6-no-definition",
                            entry.second->location, FindingCertainty::Possible});
    }
  }
  return e;
}

RuleEvaluation external_used_in_one_unit(const AnalysisContext& context) {
  RuleEvaluation e = complete();
  std::map<std::string, std::set<std::size_t>> users;
  for (const ReferenceFact& r : context.program.references) {
    users[r.name].insert(r.translation_unit);
  }
  std::set<std::string> reported;
  for (const DeclarationFact& d : context.program.declarations) {
    if (!is_entity(d) || d.linkage != LinkageKind::External ||
        !d.is_definition || d.name == "main") {
      continue;
    }
    const std::set<std::size_t>& referencing = users[d.name];
    const bool used_elsewhere = std::any_of(
        referencing.begin(), referencing.end(),
        [&d](std::size_t tu) { return tu != d.translation_unit; });
    if (!used_elsewhere &&
        reported.insert(declaration_key(d.location, d.name)).second) {
      e.findings.push_back({"misra-c2012-8.7-could-be-static", d.location,
                            FindingCertainty::Possible});
    }
  }
  return e;
}

}  // namespace misra::c2012::support
