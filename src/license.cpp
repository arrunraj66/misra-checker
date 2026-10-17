#include "misra/license.hpp"

#include <array>
#include <cstdint>
#include <ctime>
#include <fstream>
#include <map>
#include <sstream>

#ifndef MISRA_LICENSE_HMAC_KEY
#define MISRA_LICENSE_HMAC_KEY "development-key-do-not-ship"
#endif

namespace misra {
namespace {

constexpr std::array<std::uint32_t, 64> kRound{
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1,
    0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786,
    0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147,
    0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b,
    0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a,
    0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};

std::uint32_t rotr(const std::uint32_t value, const unsigned int count) {
  return (value >> count) | (value << (32U - count));
}

std::array<std::uint8_t, 32> sha256_raw(std::string_view data) {
  std::array<std::uint32_t, 8> state{0x6a09e667, 0xbb67ae85, 0x3c6ef372,
                                     0xa54ff53a, 0x510e527f, 0x9b05688c,
                                     0x1f83d9ab, 0x5be0cd19};
  std::string message{data};
  const std::uint64_t bit_length = static_cast<std::uint64_t>(data.size()) * 8U;
  message.push_back(static_cast<char>(0x80));
  while ((message.size() % 64U) != 56U) {
    message.push_back('\0');
  }
  for (int shift = 56; shift >= 0; shift -= 8) {
    message.push_back(static_cast<char>((bit_length >> shift) & 0xffU));
  }

  for (std::size_t offset = 0; offset < message.size(); offset += 64U) {
    std::array<std::uint32_t, 64> w{};
    for (std::size_t i = 0; i < 16U; ++i) {
      const std::size_t base = offset + (i * 4U);
      w[i] = (static_cast<std::uint32_t>(
                  static_cast<std::uint8_t>(message[base])) << 24U) |
             (static_cast<std::uint32_t>(
                  static_cast<std::uint8_t>(message[base + 1U])) << 16U) |
             (static_cast<std::uint32_t>(
                  static_cast<std::uint8_t>(message[base + 2U])) << 8U) |
             static_cast<std::uint32_t>(
                 static_cast<std::uint8_t>(message[base + 3U]));
    }
    for (std::size_t i = 16U; i < 64U; ++i) {
      const std::uint32_t s0 =
          rotr(w[i - 15U], 7U) ^ rotr(w[i - 15U], 18U) ^ (w[i - 15U] >> 3U);
      const std::uint32_t s1 =
          rotr(w[i - 2U], 17U) ^ rotr(w[i - 2U], 19U) ^ (w[i - 2U] >> 10U);
      w[i] = w[i - 16U] + s0 + w[i - 7U] + s1;
    }
    std::array<std::uint32_t, 8> v = state;
    for (std::size_t i = 0; i < 64U; ++i) {
      const std::uint32_t s1 =
          rotr(v[4], 6U) ^ rotr(v[4], 11U) ^ rotr(v[4], 25U);
      const std::uint32_t ch = (v[4] & v[5]) ^ (~v[4] & v[6]);
      const std::uint32_t t1 = v[7] + s1 + ch + kRound[i] + w[i];
      const std::uint32_t s0 =
          rotr(v[0], 2U) ^ rotr(v[0], 13U) ^ rotr(v[0], 22U);
      const std::uint32_t maj = (v[0] & v[1]) ^ (v[0] & v[2]) ^ (v[1] & v[2]);
      const std::uint32_t t2 = s0 + maj;
      v[7] = v[6];
      v[6] = v[5];
      v[5] = v[4];
      v[4] = v[3] + t1;
      v[3] = v[2];
      v[2] = v[1];
      v[1] = v[0];
      v[0] = t1 + t2;
    }
    for (std::size_t i = 0; i < 8U; ++i) {
      state[i] += v[i];
    }
  }

  std::array<std::uint8_t, 32> digest{};
  for (std::size_t i = 0; i < 8U; ++i) {
    digest[(i * 4U)] = static_cast<std::uint8_t>(state[i] >> 24U);
    digest[(i * 4U) + 1U] = static_cast<std::uint8_t>(state[i] >> 16U);
    digest[(i * 4U) + 2U] = static_cast<std::uint8_t>(state[i] >> 8U);
    digest[(i * 4U) + 3U] = static_cast<std::uint8_t>(state[i]);
  }
  return digest;
}

std::string to_hex(const std::array<std::uint8_t, 32>& digest) {
  static constexpr char kDigits[] = "0123456789abcdef";
  std::string out;
  for (const std::uint8_t byte : digest) {
    out.push_back(kDigits[byte >> 4U]);
    out.push_back(kDigits[byte & 0x0fU]);
  }
  return out;
}

bool constant_time_equal(std::string_view a, std::string_view b) {
  if (a.size() != b.size()) {
    return false;
  }
  unsigned int difference = 0U;
  for (std::size_t i = 0; i < a.size(); ++i) {
    difference |= static_cast<unsigned int>(a[i] ^ b[i]);
  }
  return difference == 0U;
}

std::vector<std::string> split(const std::string& text, const char separator) {
  std::vector<std::string> parts;
  std::string current;
  for (const char c : text) {
    if (c == separator) {
      parts.push_back(current);
      current.clear();
    } else {
      current.push_back(c);
    }
  }
  parts.push_back(current);
  return parts;
}

}  // namespace

std::string sha256_hex(std::string_view data) { return to_hex(sha256_raw(data)); }

std::string hmac_sha256_hex(std::string_view key, std::string_view data) {
  std::string block_key{key};
  if (block_key.size() > 64U) {
    const auto hashed = sha256_raw(block_key);
    block_key.assign(hashed.begin(), hashed.end());
  }
  block_key.resize(64U, '\0');
  std::string inner(64U, '\0');
  std::string outer(64U, '\0');
  for (std::size_t i = 0; i < 64U; ++i) {
    inner[i] = static_cast<char>(block_key[i] ^ 0x36);
    outer[i] = static_cast<char>(block_key[i] ^ 0x5c);
  }
  const auto inner_hash = sha256_raw(inner + std::string{data});
  outer.append(inner_hash.begin(), inner_hash.end());
  return to_hex(sha256_raw(outer));
}

LicenseResult verify_license_text(std::string_view text, std::string_view key,
                                  std::string_view today) {
  std::map<std::string, std::string> fields;
  std::istringstream stream{std::string{text}};
  std::string line;
  while (std::getline(stream, line)) {
    if (!line.empty() && (line.back() == '\r')) {
      line.pop_back();
    }
    const std::size_t equals = line.find('=');
    if (equals == std::string::npos) {
      continue;
    }
    fields[line.substr(0, equals)] = line.substr(equals + 1U);
  }
  for (const char* required : {"customer", "expires", "features", "signature"}) {
    if (fields.find(required) == fields.end()) {
      return {false, {}, std::string{"license field missing: "} + required};
    }
  }

  const std::string canonical = "customer=" + fields["customer"] +
                                "\nexpires=" + fields["expires"] +
                                "\nfeatures=" + fields["features"] + "\n";
  if (!constant_time_equal(hmac_sha256_hex(key, canonical),
                           fields["signature"])) {
    return {false, {}, "license signature is invalid"};
  }
  if (fields["expires"] < std::string{today}) {
    return {false, {}, "license expired on " + fields["expires"]};
  }
  return {true,
          {fields["customer"], fields["expires"], split(fields["features"], ',')},
          {}};
}

LicenseResult load_license_file(const std::string& path,
                                std::string_view today) {
  std::ifstream file(path);
  if (!file) {
    return {false, {}, "cannot read license file: " + path};
  }
  std::ostringstream contents;
  contents << file.rdbuf();
  return verify_license_text(contents.str(), MISRA_LICENSE_HMAC_KEY, today);
}

bool has_feature(const LicenseInfo& info, std::string_view feature) {
  for (const std::string& name : info.features) {
    if (name == feature) {
      return true;
    }
  }
  return false;
}

std::string today_iso_date() {
  const std::time_t now = std::time(nullptr);
  std::tm parts{};
  gmtime_r(&now, &parts);
  char buffer[16];
  std::strftime(buffer, sizeof(buffer), "%Y-%m-%d", &parts);
  return buffer;
}

}  // namespace misra
