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
    hash_failed,
};

struct PayloadCryptoResult {
    PayloadCryptoError error = PayloadCryptoError::none;
    std::string text;

    [[nodiscard]] bool ok() const noexcept {
        return error == PayloadCryptoError::none;
    }
};

[[nodiscard]] PayloadCryptoResult payload_sha256_hex(std::string_view input);
[[nodiscard]] PayloadCryptoResult payload_base64_encode(std::string_view input);
[[nodiscard]] PayloadCryptoResult payload_base64_decode(std::string_view input);

}  // namespace copperfin::security
