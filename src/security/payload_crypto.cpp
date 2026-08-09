// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/security/payload_crypto.h"

#include "copperfin/security/sha256.h"

#include <cstdint>
#include <utility>

namespace copperfin::security {

namespace {

constexpr char kBase64Alphabet[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int decode_base64_char(const char value) noexcept {
    if (value >= 'A' && value <= 'Z') {
        return value - 'A';
    }
    if (value >= 'a' && value <= 'z') {
        return (value - 'a') + 26;
    }
    if (value >= '0' && value <= '9') {
        return (value - '0') + 52;
    }
    if (value == '+') {
        return 62;
    }
    if (value == '/') {
        return 63;
    }
    return -1;
}

PayloadCryptoResult failure(const PayloadCryptoError error) {
    return {.error = error, .text = {}};
}

}  // namespace

PayloadCryptoResult payload_sha256_hex(const std::string_view input) {
    if (input.size() > kPayloadCryptoMaxBytes) {
        return failure(PayloadCryptoError::input_too_large);
    }
    const auto hashed = sha256_hex_for_text(std::string(input));
    return hashed.ok
        ? PayloadCryptoResult{.error = PayloadCryptoError::none, .text = hashed.hex_digest}
        : failure(PayloadCryptoError::hash_failed);
}

PayloadCryptoResult payload_base64_encode(const std::string_view input) {
    if (input.size() > kPayloadCryptoMaxBytes) {
        return failure(PayloadCryptoError::input_too_large);
    }

    std::string encoded;
    encoded.reserve(((input.size() + 2U) / 3U) * 4U);
    for (std::size_t offset = 0U; offset < input.size(); offset += 3U) {
        const std::size_t remaining = input.size() - offset;
        const auto first = static_cast<std::uint8_t>(input[offset]);
        const auto second = remaining >= 2U
            ? static_cast<std::uint8_t>(input[offset + 1U])
            : std::uint8_t{0U};
        const auto third = remaining >= 3U
            ? static_cast<std::uint8_t>(input[offset + 2U])
            : std::uint8_t{0U};
        const std::uint32_t triple =
            (static_cast<std::uint32_t>(first) << 16U) |
            (static_cast<std::uint32_t>(second) << 8U) |
            static_cast<std::uint32_t>(third);

        encoded.push_back(kBase64Alphabet[(triple >> 18U) & 0x3FU]);
        encoded.push_back(kBase64Alphabet[(triple >> 12U) & 0x3FU]);
        encoded.push_back(remaining >= 2U
            ? kBase64Alphabet[(triple >> 6U) & 0x3FU]
            : '=');
        encoded.push_back(remaining >= 3U
            ? kBase64Alphabet[triple & 0x3FU]
            : '=');
    }
    return {.error = PayloadCryptoError::none, .text = std::move(encoded)};
}

PayloadCryptoResult payload_base64_decode(const std::string_view input) {
    if (input.size() > kPayloadCryptoMaxBase64Bytes) {
        return failure(PayloadCryptoError::input_too_large);
    }
    if (input.empty()) {
        return {.error = PayloadCryptoError::none, .text = {}};
    }
    if ((input.size() % 4U) != 0U) {
        return failure(PayloadCryptoError::invalid_base64);
    }

    std::string decoded;
    decoded.reserve((input.size() / 4U) * 3U);
    for (std::size_t offset = 0U; offset < input.size(); offset += 4U) {
        const bool final_quartet = offset + 4U == input.size();
        const bool pad_third = input[offset + 2U] == '=';
        const bool pad_fourth = input[offset + 3U] == '=';
        if ((!final_quartet && (pad_third || pad_fourth)) ||
            (pad_third && !pad_fourth)) {
            return failure(PayloadCryptoError::invalid_base64);
        }

        const int first = decode_base64_char(input[offset]);
        const int second = decode_base64_char(input[offset + 1U]);
        const int third = pad_third ? 0 : decode_base64_char(input[offset + 2U]);
        const int fourth = pad_fourth ? 0 : decode_base64_char(input[offset + 3U]);
        if (first < 0 || second < 0 || third < 0 || fourth < 0) {
            return failure(PayloadCryptoError::invalid_base64);
        }
        if ((pad_third && (second & 0x0F) != 0) ||
            (pad_fourth && !pad_third && (third & 0x03) != 0)) {
            return failure(PayloadCryptoError::invalid_base64);
        }

        const std::uint32_t triple =
            (static_cast<std::uint32_t>(first) << 18U) |
            (static_cast<std::uint32_t>(second) << 12U) |
            (static_cast<std::uint32_t>(third) << 6U) |
            static_cast<std::uint32_t>(fourth);
        decoded.push_back(static_cast<char>((triple >> 16U) & 0xFFU));
        if (!pad_third) {
            decoded.push_back(static_cast<char>((triple >> 8U) & 0xFFU));
        }
        if (!pad_fourth) {
            decoded.push_back(static_cast<char>(triple & 0xFFU));
        }
    }
    if (decoded.size() > kPayloadCryptoMaxBytes) {
        return failure(PayloadCryptoError::input_too_large);
    }
    return {.error = PayloadCryptoError::none, .text = std::move(decoded)};
}

}  // namespace copperfin::security
