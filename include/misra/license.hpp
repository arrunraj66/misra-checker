#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace misra {

struct LicenseInfo final {
  std::string customer;
  std::string expires;  // ISO date, YYYY-MM-DD
  std::vector<std::string> features;
};

struct LicenseResult final {
  bool valid;
  LicenseInfo info;
  std::string error_message;
};

[[nodiscard]] std::string sha256_hex(std::string_view data);
[[nodiscard]] std::string hmac_sha256_hex(std::string_view key,
                                          std::string_view data);

// License file format (text, one field per line):
//   customer=<text>
//   expires=<YYYY-MM-DD>
//   features=<comma separated, e.g. convert>
//   signature=<hex HMAC-SHA256 of the canonical text of the first three lines>
// `today` is an ISO date supplied by the caller so checks are testable.
[[nodiscard]] LicenseResult verify_license_text(std::string_view text,
                                                std::string_view key,
                                                std::string_view today);
[[nodiscard]] LicenseResult load_license_file(const std::string& path,
                                              std::string_view today);
[[nodiscard]] bool has_feature(const LicenseInfo& info,
                               std::string_view feature);
[[nodiscard]] std::string today_iso_date();

}  // namespace misra
