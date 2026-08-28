#include <cstdlib>
#include <iostream>
#include <set>
#include <string_view>

#include "misra/rule_registry.hpp"

namespace {

void require(const bool condition, const std::string_view message) {
  if (!condition) {
    std::cerr << "rule_registry_test: " << message << '\n';
    std::exit(EXIT_FAILURE);
  }
}

}  // namespace

int main() {
  const misra::RuleRegistry registry;
  require(registry.size() == 143U, "expected exactly 143 rule structures");

  std::set<std::string_view> ids;
  std::size_t mandatory = 0U;
  std::size_t required = 0U;
  std::size_t advisory = 0U;
  std::size_t decidable = 0U;
  std::size_t undecidable = 0U;
  std::size_t single_unit = 0U;
  std::size_t system = 0U;

  for (const auto& rule : registry.rules()) {
    const auto& descriptor = rule->descriptor();
    require(ids.insert(descriptor.id).second, "rule identifiers must be unique");
    require(!descriptor.topic.empty(), "topic must be documented");
    require(!descriptor.intent_summary.empty(), "intent summary must be documented");
    require(!descriptor.analysis_expansion.empty(),
            "analysis expansion must be documented");
    require(descriptor.status == misra::ImplementationStatus::Scaffold,
            "new rule structures must not claim implementation");

    switch (descriptor.category) {
      case misra::RuleCategory::Mandatory:
        ++mandatory;
        break;
      case misra::RuleCategory::Required:
        ++required;
        break;
      case misra::RuleCategory::Advisory:
        ++advisory;
        break;
    }

    if (descriptor.decidability == misra::Decidability::Decidable) {
      ++decidable;
    } else {
      ++undecidable;
    }

    if (descriptor.scope == misra::AnalysisScope::SingleTranslationUnit) {
      ++single_unit;
    } else {
      ++system;
    }
  }

  require(mandatory == 10U, "expected 10 mandatory rules");
  require(required == 101U, "expected 101 required rules");
  require(advisory == 32U, "expected 32 advisory rules");
  require(decidable == 116U, "expected 116 decidable rules");
  require(undecidable == 27U, "expected 27 undecidable rules");
  require(single_unit == 104U, "expected 104 single-unit rules");
  require(system == 39U, "expected 39 system rules");
  require(registry.find("1.1") != nullptr, "Rule 1.1 must be registered");
  require(registry.find("22.6") != nullptr, "Rule 22.6 must be registered");
  require(registry.find("99.99") == nullptr, "unknown rule must not resolve");

  return EXIT_SUCCESS;
}
