/*
 * MISRA C:2012 Rule 21.6 - Clang-backed detector
 *
 * Topic: Standard library usage
 * Engineering intent: Independent checker contract for Rule 21.6 in the Standard library usage family. The exact normative predicate remains linked to the controlled licensed rule specification.
 * Analysis expansion: Match headers, reserved identifiers, declarations, and calls against the configured controlled-library policy.
 * Normative notice: Wording, amplification, exceptions, and examples remain in
 * the licensed MISRA specification and require independent approval.
 * Detection contract: Calls to the <stdio.h>/<wchar.h> input/output functions are reported.
 * Findings produced by macro expansion are tagged; see docs/macro-policy.md.
 * Implementation status: Implemented; independent validation is pending.
 */

#include "misra/c2012/rule_factories.hpp"
#include "misra/c2012/rule_support.hpp"

#include <memory>

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
    return support::call_findings(context, {"clearerr", "fclose", "feof", "ferror", "fflush", "fgetc", "fgetpos", "fgets", "fopen", "fprintf", "fputc", "fputs", "fread", "freopen", "fscanf", "fseek", "fsetpos", "ftell", "fwrite", "getc", "getchar", "gets", "perror", "printf", "putc", "putchar", "puts", "remove", "rename", "rewind", "scanf", "setbuf", "setvbuf", "snprintf", "sprintf", "sscanf", "tmpfile", "tmpnam", "ungetc", "vfprintf", "vfscanf", "vprintf", "vscanf", "vsnprintf", "vsprintf", "vsscanf", "fwprintf", "fwscanf", "wprintf", "wscanf", "vfwprintf", "vwprintf", "getwc", "getwchar", "putwc", "putwchar", "ungetwc", "fgetwc", "fgetws", "fputwc", "fputws", "fwide"}, "misra-c2012-21.6-stdio-function");
  }
};

}  // namespace

RulePtr make_rule_21_06() {
  return std::make_unique<Rule_21_06>();
}

}  // namespace misra::c2012
