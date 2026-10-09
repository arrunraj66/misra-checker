#pragma once

#include <initializer_list>
#include <string_view>

#include "misra/rule.hpp"

namespace misra {

// Maps recorded observations to findings. An observation matches when its
// kind is listed and, if `details` is non-empty, its detail is listed too.
[[nodiscard]] inline RuleEvaluation findings_from_observations(
    const AnalysisContext& context,
    std::initializer_list<std::string_view> kinds,
    const std::string_view message_key,
    std::initializer_list<std::string_view> details = {},
    std::initializer_list<std::string_view> possible_kinds = {}) {
  RuleEvaluation evaluation{EvaluationStatus::Complete, {}};
  for (const Observation& observation : context.observations) {
    bool kind_matches = false;
    for (const std::string_view kind : kinds) {
      kind_matches = kind_matches || (observation.kind == kind);
    }
    if (!kind_matches) {
      continue;
    }
    bool detail_matches = (details.size() == 0U);
    for (const std::string_view detail : details) {
      detail_matches = detail_matches || (observation.detail == detail);
    }
    if (detail_matches) {
      bool possible = false;
      for (const std::string_view kind : possible_kinds) {
        possible = possible || (observation.kind == kind);
      }
      evaluation.findings.push_back(
          {message_key, observation.location,
           possible ? FindingCertainty::Possible : FindingCertainty::Definite});
    }
  }
  return evaluation;
}

}  // namespace misra
