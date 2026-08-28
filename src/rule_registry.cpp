#include "misra/rule_registry.hpp"

#include <algorithm>

#include "misra/c2012/rule_factories.hpp"

namespace misra {

RuleRegistry::RuleRegistry() {
  const auto factories = c2012::rule_factories();
  rules_.reserve(factories.size());

  for (const RuleFactory factory : factories) {
    rules_.push_back(factory());
  }
}

std::size_t RuleRegistry::size() const noexcept { return rules_.size(); }

const std::vector<RulePtr>& RuleRegistry::rules() const noexcept {
  return rules_;
}

const Rule* RuleRegistry::find(const std::string_view id) const noexcept {
  const auto match = std::find_if(
      rules_.begin(), rules_.end(), [id](const RulePtr& rule) {
        return rule->descriptor().id == id;
      });

  return match == rules_.end() ? nullptr : match->get();
}

}  // namespace misra
