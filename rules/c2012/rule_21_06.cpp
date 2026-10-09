/*
 * MISRA C:2012 Rule 21.6 - engineering scaffold
 *
 * Topic: Standard library usage
 * Engineering intent: Independent checker contract for Rule 21.6 in the Standard library usage family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Match headers, reserved identifiers, declarations, and calls against the configured controlled-library policy.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: Report a call to one of the listed functions: standard input/output function called (matched by callee name).
 * Evidence: AST/preprocessor observations recorded by the Clang adapter.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"

#include <memory>

#include "misra/observation_rule.hpp"

namespace misra::c2012 {
namespace {

class Rule_21_06 final : public Rule {
 public:
  [[nodiscard]] const RuleDescriptor& descriptor() const noexcept override {
    static constexpr RuleDescriptor descriptor{
        "21.6",
        RuleCategory::Required,
        Decidability::Decidable,
        AnalysisScope::SingleTranslationUnit,
        {true, true},
        "Standard library usage",
        "Independent checker contract for Rule 21.6 in the Standard library usage family. The exact normative predicate remains linked to the controlled licensed rule specification.",
        "Match headers, reserved identifiers, declarations, and calls against the configured controlled-library policy.",
        ImplementationStatus::Implemented,
    };
    return descriptor;
  }

  [[nodiscard]] RuleEvaluation evaluate(
      const AnalysisContext& context) const override {
    return findings_from_observations(context, {"call"},
                                      "misra-c2012-21.6-banned-function", {"printf", "fprintf", "sprintf", "snprintf", "vprintf", "vfprintf", "vsprintf", "vsnprintf", "scanf", "fscanf", "sscanf", "vscanf", "vfscanf", "vsscanf", "fopen", "freopen", "fclose", "fflush", "fread", "fwrite", "fgetc", "fgets", "fputc", "fputs", "getc", "getchar", "gets", "putc", "putchar", "puts", "ungetc", "fseek", "ftell", "rewind", "fgetpos", "fsetpos", "clearerr", "feof", "ferror", "perror", "remove", "rename", "tmpfile", "tmpnam", "setbuf", "setvbuf"});
  }
};

}  // namespace

RulePtr make_rule_21_06() {
  return std::make_unique<Rule_21_06>();
}

}  // namespace misra::c2012
