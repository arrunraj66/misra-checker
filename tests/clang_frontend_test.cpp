#include <cstdlib>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include "misra/clang_frontend.hpp"

namespace {

void require(const bool condition, const std::string_view message) {
  if (!condition) {
    std::cerr << "clang_frontend_test: " << message << '\n';
    std::exit(EXIT_FAILURE);
  }
}

misra::FrontendResult expect_gotos(const misra::ClangFrontend& frontend,
                                   const std::string& database,
                                   const std::string& source,
                                   const std::size_t expected) {
  misra::FrontendResult result = frontend.analyze(database, {source});
  require(result.success, result.error_message);
  require(result.analyzed_files.size() == 1U,
          "exactly one translation unit must be analyzed");
  require(result.context.control_flow.goto_statements.size() == expected,
          "unexpected goto fact count");
  return result;
}

}  // namespace

int main(const int argc, char* argv[]) {
  require(argc == 6, "expected database and four fixture paths");

  const misra::ClangFrontend frontend;
  const std::string database = argv[1];
  expect_gotos(frontend, database, argv[2], 0U);
  expect_gotos(frontend, database, argv[3], 1U);
  const misra::FrontendResult macro_result =
      expect_gotos(frontend, database, argv[4], 1U);
  require(macro_result.context.control_flow.goto_statements.front()
              .originates_from_macro,
          "macro provenance must be retained");
  expect_gotos(frontend, database, argv[5], 0U);
  return EXIT_SUCCESS;
}
