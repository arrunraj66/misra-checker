#include <iostream>

#include "misra/converter.hpp"

int main() {
  int failures = 0;
  if (misra::extract_proposed_source("```c\nint x;\n```\n") != "int x;\n") {
    std::cerr << "fence stripping failed\n";
    ++failures;
  }
  if (misra::extract_proposed_source("int y;\n") != "int y;\n") {
    std::cerr << "plain passthrough failed\n";
    ++failures;
  }
  for (const misra::FixClass& fix : misra::fix_classes()) {
    if (fix.auto_apply_approved) {
      std::cerr << "no fix class may be auto-apply approved before validation\n";
      ++failures;
    }
  }
  return failures == 0 ? 0 : 1;
}
