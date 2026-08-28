#include <iostream>
#include <string_view>

#include "misra/rule_registry.hpp"

namespace {

constexpr std::string_view kVersion = "0.1.0";

}  // namespace

int main(int argc, char* argv[]) {
  if ((argc == 2) && (std::string_view{argv[1]} == "--version")) {
    std::cout << "misra-checker " << kVersion << '\n';
    return 0;
  }

  if ((argc == 2) && (std::string_view{argv[1]} == "--list-rules")) {
    const misra::RuleRegistry registry;
    for (const auto& rule : registry.rules()) {
      std::cout << rule->descriptor().id << '\n';
    }
    return 0;
  }

  const misra::RuleRegistry registry;
  std::cout << "MISRA C checker foundation initialized with "
            << registry.size() << " rule structures.\n";
  return 0;
}
