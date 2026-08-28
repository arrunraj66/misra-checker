#pragma once

#include <string>
#include <vector>

#include "misra/rule.hpp"

namespace misra {

struct FrontendResult final {
  bool success;
  AnalysisContext context;
  std::vector<std::string> analyzed_files;
  std::string error_message;
};

class ClangFrontend final {
 public:
  [[nodiscard]] FrontendResult analyze(
      const std::string& compilation_database,
      const std::vector<std::string>& requested_files = {}) const;
};

}  // namespace misra
