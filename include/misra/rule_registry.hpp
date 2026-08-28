#pragma once

#include <cstddef>
#include <memory>
#include <string_view>
#include <vector>

#include "misra/rule.hpp"

namespace misra {

class RuleRegistry final {
 public:
  RuleRegistry();

  [[nodiscard]] std::size_t size() const noexcept;
  [[nodiscard]] const std::vector<RulePtr>& rules() const noexcept;
  [[nodiscard]] const Rule* find(std::string_view id) const noexcept;

 private:
  std::vector<RulePtr> rules_;
};

}  // namespace misra
