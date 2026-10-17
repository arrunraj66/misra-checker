#include <cstdlib>
#include <iostream>

#include "misra/license.hpp"

namespace {
int failures = 0;
void check(const bool condition, const char* what) {
  if (!condition) {
    std::cerr << "FAIL: " << what << '\n';
    ++failures;
  }
}
}  // namespace

int main() {
  // NIST test vectors.
  check(misra::sha256_hex("abc") ==
            "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad",
        "sha256 abc");
  check(misra::hmac_sha256_hex("key", "The quick brown fox jumps over the lazy dog") ==
            "f7bc83f430538424b13298e6aa6fb143ef4d59a14946175997479dbc2d1a3cd8",
        "hmac vector");

  const std::string body = "customer=ACME\nexpires=2030-01-01\nfeatures=convert\n";
  const std::string text = body + "signature=" + misra::hmac_sha256_hex("k", body) + "\n";
  const auto ok = misra::verify_license_text(text, "k", "2026-01-01");
  check(ok.valid && misra::has_feature(ok.info, "convert"), "valid license");
  check(!misra::verify_license_text(text, "other", "2026-01-01").valid, "wrong key");
  check(!misra::verify_license_text(text, "k", "2031-01-01").valid, "expired");
  std::string tampered = text;
  tampered.replace(tampered.find("ACME"), 4U, "EVIL");
  check(!misra::verify_license_text(tampered, "k", "2026-01-01").valid, "tampered");
  check(!misra::verify_license_text("customer=x\n", "k", "2026-01-01").valid, "missing fields");
  return failures == 0 ? 0 : 1;
}
