#pragma once

#include <memory>
#include <string_view>

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

struct AnalysisContext final {
  // Populated by the parser and analysis platform in later phases.
};

enum class EvaluationStatus {
  NotImplemented,
  Complete,
  Inconclusive,
};

struct RuleEvaluation final {
  EvaluationStatus status;
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
