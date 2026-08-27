#include <iostream>
#include <string_view>

namespace {

constexpr std::string_view kVersion = "0.1.0";

}  // namespace

int main(int argc, char* argv[]) {
  if ((argc == 2) && (std::string_view{argv[1]} == "--version")) {
    std::cout << "misra-checker " << kVersion << '\n';
    return 0;
  }

  std::cout << "MISRA C checker foundation initialized.\n";
  return 0;
}
