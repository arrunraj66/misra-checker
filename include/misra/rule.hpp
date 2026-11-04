#pragma once

#include <cstddef>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace misra {

enum class RuleCategory {
  Mandatory,
  Required,
  Advisory,
};

enum class Decidability {
  Decidable,
  Undecidable,
};

enum class AnalysisScope {
  SingleTranslationUnit,
  System,
};

enum class ImplementationStatus {
  Scaffold,
  Implemented,
  IndependentlyValidated,
};

struct LanguageApplicability final {
  bool c90;
  bool c99;
};

struct RuleDescriptor final {
  std::string_view id;
  RuleCategory category;
  Decidability decidability;
  AnalysisScope scope;
  LanguageApplicability languages;
  std::string_view topic;
  std::string_view intent_summary;
  std::string_view analysis_expansion;
  ImplementationStatus status;
};

// Findings are reported at the *expansion* location (where the programmer
// wrote the code). from_macro is true when the construct was produced by a
// macro expansion; see docs/macro-policy.md.
struct SourceLocation final {
  std::string file;
  unsigned int line;
  unsigned int column;
  bool from_macro = false;
};

struct GotoStatementFact final {
  SourceLocation location;
  SourceLocation target_location;
  bool target_declared_later;
  bool originates_from_macro;
  bool target_originates_from_macro;
};

// A statement body owned by if/else/for/while/do/switch.
struct StatementBodyFact final {
  SourceLocation location;  // location of the body (or of the construct)
  std::string_view construct;
  bool is_compound;
};

// An if ... else if ... chain. Only chains containing an "else if" appear.
struct IfElseChainFact final {
  SourceLocation location;  // location of the last "if" in the chain
  bool has_final_else;
};

struct SwitchStatementFact final {
  SourceLocation location;
  bool has_default;
};

struct ControlFlowFacts final {
  std::vector<GotoStatementFact> goto_statements;
  std::vector<StatementBodyFact> statement_bodies;
  std::vector<IfElseChainFact> if_else_chains;
  std::vector<SwitchStatementFact> switch_statements;
};

// A call whose callee resolved to a named function declaration.
struct CallFact final {
  SourceLocation location;
  std::string callee;
  bool callee_in_system_header;
  bool originates_from_macro;
};

// Generic single-translation-unit syntax fact. `kind` is the diagnostic key of
// the rule that consumes it and always points at a string literal.
struct SyntaxFact final {
  SourceLocation location;
  std::string_view kind;
};

// Controlling expression of an if/while/do/for statement (Rule 14.4).
struct ControllingExpressionFact final {
  SourceLocation location;
  std::string_view construct;
  bool essentially_boolean;
};

// Jumps that terminate an iteration statement (Rule 15.4).
struct LoopExitFact final {
  SourceLocation location;
  std::size_t terminating_jumps;
};

// Return statements of a function definition (Rule 15.5).
struct FunctionExitFact final {
  SourceLocation location;
  std::size_t return_count;
  bool ends_with_return;
};

// A #include directive written in a non-system file.
struct IncludeFact final {
  SourceLocation location;
  std::string header;
  bool angled;
};

enum class DeclKind {
  Function,
  Object,
  Typedef,
  Tag,
};

enum class LinkageKind {
  External,
  Internal,
  None,
};

struct DeclarationFact final {
  SourceLocation location;
  std::string name;
  DeclKind kind;
  LinkageKind linkage;
  bool is_definition;  // includes C tentative definitions
  std::string type;    // canonical spelling of the type without array bounds
  std::vector<std::string> parameter_names;
  // Array bounds, outermost first; -1 where the bound is absent. Kept apart
  // from `type` because `int a[]` and `int a[4]` are compatible.
  std::vector<long long> array_dimensions;
  std::size_t translation_unit;
};

// A reference to an externally visible function or object.
struct ReferenceFact final {
  std::string name;
  std::size_t translation_unit;
};

struct MacroFact final {
  SourceLocation location;
  std::string name;
  bool function_like;
  std::size_t translation_unit;
};

// A direct call between two project functions, for the call-graph rules.
// Names are qualified for internal-linkage functions so equal static names in
// different files stay distinct.
struct CallEdgeFact final {
  SourceLocation location;
  std::string caller;
  std::string callee;
  // True for an edge that depends on a function pointer: a function address
  // handed to a library function that will call it back.
  bool via_pointer = false;
};

// A project function whose address is used other than as a direct callee.
struct AddressTakenFact final {
  SourceLocation location;
  std::string function;
  std::string signature;  // canonical function type
};

// A call through a function pointer of the given pointee type.
struct IndirectCallFact final {
  SourceLocation location;
  std::string caller;
  std::string signature;
};

// Cross-translation-unit facts. Only meaningful when the compilation database
// covers the whole program.
struct ProgramFacts final {
  std::vector<DeclarationFact> declarations;
  std::vector<ReferenceFact> references;
  std::vector<MacroFact> macros;
  std::vector<CallEdgeFact> call_edges;
  std::vector<AddressTakenFact> address_taken;
  std::vector<IndirectCallFact> indirect_calls;
  std::set<std::string> used_macros;
  // Keys produced by declaration_key() of typedefs/tags that are referenced.
  std::set<std::string> referenced_declarations;
};

enum class IdentifierSpace {
  Ordinary,  // objects, functions, typedefs, enumeration constants
  Tag,       // struct, union and enum tags
  Member,    // members of one struct or union (scope = that aggregate)
};

// An identifier declaration with the scope that contains it. Scope ids are
// unique within an analysis run; id 0 means "no enclosing scope".
struct IdentifierFact final {
  SourceLocation location;
  std::string name;
  std::size_t scope;
  IdentifierSpace space;
  std::size_t translation_unit;
};

[[nodiscard]] inline std::string declaration_key(const SourceLocation& location,
                                                 const std::string& name) {
  return location.file + ':' + std::to_string(location.line) + ':' +
         std::to_string(location.column) + ':' + name;
}

struct AnalysisContext final {
  ControlFlowFacts control_flow;
  std::vector<CallFact> calls;
  std::vector<SyntaxFact> syntax;
  std::vector<ControllingExpressionFact> controlling_expressions;
  std::vector<LoopExitFact> loop_exits;
  std::vector<FunctionExitFact> function_exits;
  std::vector<IncludeFact> includes;
  ProgramFacts program;
  std::vector<IdentifierFact> identifiers;
  std::map<std::size_t, std::size_t> scope_parents;  // scope -> enclosing scope
  std::size_t translation_units = 0U;
};

enum class EvaluationStatus {
  NotImplemented,
  Complete,
  Inconclusive,
};

enum class FindingCertainty {
  Definite,
  Possible,
};

struct Finding final {
  std::string_view message_key;
  SourceLocation location;
  FindingCertainty certainty;
};

struct RuleEvaluation final {
  RuleEvaluation(const EvaluationStatus evaluation_status)
      : status(evaluation_status) {}

  RuleEvaluation(const EvaluationStatus evaluation_status,
                 std::vector<Finding> evaluation_findings)
      : status(evaluation_status),
        findings(std::move(evaluation_findings)) {}

  EvaluationStatus status;
  std::vector<Finding> findings;
};

class Rule {
 public:
  virtual ~Rule() = default;

  [[nodiscard]] virtual const RuleDescriptor& descriptor() const noexcept = 0;
  [[nodiscard]] virtual RuleEvaluation evaluate(
      const AnalysisContext& context) const = 0;
};

using RulePtr = std::unique_ptr<Rule>;
using RuleFactory = RulePtr (*)();

}  // namespace misra
