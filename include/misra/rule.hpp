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
};

struct ControlFlowFacts final {
  std::vector<GotoStatementFact> goto_statements;
};

struct AnalysisContext final {
  ControlFlowFacts control_flow;
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
