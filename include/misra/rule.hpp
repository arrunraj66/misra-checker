#pragma once

#include <memory>
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

struct SourceLocation final {
  std::string file;
  unsigned int line;
  unsigned int column;
};

struct GotoStatementFact final {
  SourceLocation location;
  SourceLocation target_location;
  bool target_declared_later;
  bool originates_from_macro;
  bool target_originates_from_macro;
  // True when the label is declared in the goto's block or an enclosing one.
  bool target_in_enclosing_block;
};

struct LoopFact final {
  SourceLocation location;
  // break statements bound to this loop plus gotos that leave it.
  unsigned int terminating_jumps;
};

struct ControlFlowFacts final {
  std::vector<GotoStatementFact> goto_statements;
  std::vector<LoopFact> loops;
};

// A candidate construct recorded by the Clang adapter. `kind` names the
// construct class; `detail` carries a name where relevant (e.g. a callee).
struct Observation final {
  std::string kind;
  SourceLocation location;
  std::string detail;
};

struct AnalysisContext final {
  ControlFlowFacts control_flow;
  std::vector<Observation> observations;
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
