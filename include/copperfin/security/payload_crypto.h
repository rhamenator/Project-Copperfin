// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include <cstddef>
#include <string>
#include <string_view>

namespace copperfin::security {

inline constexpr std::size_t kPayloadCryptoMaxBytes = 1024U * 1024U;
inline constexpr std::size_t kPayloadCryptoMaxBase64Bytes =
    ((kPayloadCryptoMaxBytes + 2U) / 3U) * 4U;

enum class PayloadCryptoError {
    none,
    input_too_large,
    invalid_base64,
    invalid_digest,
    hash_failed,
};

struct PayloadCryptoResult {
    PayloadCryptoError error = PayloadCryptoError::none;
    std::string text;

    [[nodiscard]] bool ok() const noexcept {
        return error == PayloadCryptoError::none;
    }
};

struct PayloadCryptoVerificationResult {
    PayloadCryptoError error = PayloadCryptoError::none;
    bool matches = false;

    [[nodiscard]] bool ok() const noexcept {
        return error == PayloadCryptoError::none;
    }
};

[[nodiscard]] PayloadCryptoResult payload_sha256_hex(std::string_view input);
[[nodiscard]] PayloadCryptoResult payload_hmac_sha256_hex(
    std::string_view key,
    std::string_view input);
[[nodiscard]] PayloadCryptoVerificationResult payload_hmac_sha256_verify(
    std::string_view key,
    std::string_view input,
    std::string_view expected_hex_digest);
[[nodiscard]] PayloadCryptoResult payload_base64_encode(std::string_view input);
[[nodiscard]] PayloadCryptoResult payload_base64_decode(std::string_view input);

}  // namespace copperfin::security
