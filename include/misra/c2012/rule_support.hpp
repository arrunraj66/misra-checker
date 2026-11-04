#pragma once

#include <initializer_list>
#include <string_view>

#include "misra/rule.hpp"

// Reusable evaluation helpers shared by the rule classes. Each helper turns
// analysis facts into findings; the rule class keeps its own identity,
// metadata and diagnostic key.
namespace misra::c2012::support {

// --- single translation unit ---------------------------------------------
RuleEvaluation syntax_findings(const AnalysisContext& context,
                               std::string_view key);
RuleEvaluation call_findings(const AnalysisContext& context,
                             std::initializer_list<std::string_view> callees,
                             std::string_view key);
RuleEvaluation include_findings(const AnalysisContext& context,
                                std::initializer_list<std::string_view> headers,
                                std::string_view key);
RuleEvaluation non_boolean_condition_findings(const AnalysisContext& context,
                                              std::string_view key);
RuleEvaluation merged(RuleEvaluation first, RuleEvaluation second);
RuleEvaluation multiple_loop_exit_findings(const AnalysisContext& context,
                                           std::string_view key);
RuleEvaluation function_exit_findings(const AnalysisContext& context,
                                      std::string_view key);

// --- scopes ----------------------------------------------------------------
RuleEvaluation identifiers_not_distinct_in_scope(const AnalysisContext& context);  // 5.2
RuleEvaluation identifier_hides_outer(const AnalysisContext& context);             // 5.3

// --- whole program (valid only for a complete compilation database) ------
RuleEvaluation external_identifier_clash(const AnalysisContext& context);   // 5.1
RuleEvaluation macro_identifier_clash(const AnalysisContext& context);      // 5.4
RuleEvaluation identifier_macro_clash(const AnalysisContext& context);      // 5.5
RuleEvaluation duplicate_typedef_names(const AnalysisContext& context);     // 5.6
RuleEvaluation duplicate_tag_names(const AnalysisContext& context);         // 5.7
RuleEvaluation external_internal_name_clash(const AnalysisContext& context);// 5.8
RuleEvaluation internal_name_not_unique(const AnalysisContext& context);    // 5.9
RuleEvaluation recursive_calls(const AnalysisContext& context);             // 17.2
RuleEvaluation unused_typedefs(const AnalysisContext& context);             // 2.3
RuleEvaluation unused_tags(const AnalysisContext& context);                 // 2.4
RuleEvaluation unused_macros(const AnalysisContext& context);               // 2.5
RuleEvaluation inconsistent_declarations(const AnalysisContext& context);   // 8.3
RuleEvaluation declared_in_multiple_files(const AnalysisContext& context);  // 8.5
RuleEvaluation definition_count(const AnalysisContext& context);            // 8.6
RuleEvaluation external_used_in_one_unit(const AnalysisContext& context);   // 8.7

}  // namespace misra::c2012::support
