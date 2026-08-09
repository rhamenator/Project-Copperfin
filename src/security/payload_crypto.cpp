// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/security/payload_crypto.h"

#include "copperfin/security/sha256.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <utility>

namespace copperfin::security {

namespace {

constexpr char kBase64Alphabet[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
constexpr std::size_t kSha256BlockBytes = 64U;
constexpr std::size_t kSha256DigestBytes = 32U;
constexpr std::size_t kSha256HexBytes = kSha256DigestBytes * 2U;

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

int decode_lower_hex_char(const char value) noexcept {
    if (value >= '0' && value <= '9') {
        return value - '0';
    }
    if (value >= 'a' && value <= 'f') {
        return (value - 'a') + 10;
    }
    return -1;
}

bool decode_sha256_hex(
    const std::string_view hex,
    std::array<std::uint8_t, kSha256DigestBytes>& bytes) noexcept {
    if (hex.size() != kSha256HexBytes) {
        return false;
    }
    for (std::size_t index = 0U; index < bytes.size(); ++index) {
        const int high = decode_lower_hex_char(hex[index * 2U]);
        const int low = decode_lower_hex_char(hex[index * 2U + 1U]);
        if (high < 0 || low < 0) {
            return false;
        }
        bytes[index] = static_cast<std::uint8_t>((high << 4U) | low);
    }
    return true;
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

PayloadCryptoResult payload_hmac_sha256_hex(
    const std::string_view key,
    const std::string_view input) {
    if (key.size() > kPayloadCryptoMaxBytes || input.size() > kPayloadCryptoMaxBytes) {
        return failure(PayloadCryptoError::input_too_large);
    }

    std::array<std::uint8_t, kSha256BlockBytes> key_block{};
    if (key.size() > key_block.size()) {
        const auto key_hash = sha256_hex_for_text(std::string(key));
        std::array<std::uint8_t, kSha256DigestBytes> key_digest{};
        if (!key_hash.ok || !decode_sha256_hex(key_hash.hex_digest, key_digest)) {
            return failure(PayloadCryptoError::hash_failed);
        }
        std::copy(key_digest.begin(), key_digest.end(), key_block.begin());
    } else {
        std::transform(
            key.begin(),
            key.end(),
            key_block.begin(),
            [](const char value) { return static_cast<std::uint8_t>(value); });
    }

    std::string inner_input(kSha256BlockBytes, '\0');
    std::string outer_input(kSha256BlockBytes, '\0');
    for (std::size_t index = 0U; index < key_block.size(); ++index) {
        inner_input[index] = static_cast<char>(key_block[index] ^ 0x36U);
        outer_input[index] = static_cast<char>(key_block[index] ^ 0x5cU);
    }
    inner_input.append(input);
    const auto inner_hash = sha256_hex_for_text(inner_input);
    std::array<std::uint8_t, kSha256DigestBytes> inner_digest{};
    if (!inner_hash.ok || !decode_sha256_hex(inner_hash.hex_digest, inner_digest)) {
        return failure(PayloadCryptoError::hash_failed);
    }
    outer_input.append(
        reinterpret_cast<const char*>(inner_digest.data()),
        inner_digest.size());
    const auto outer_hash = sha256_hex_for_text(outer_input);
    return outer_hash.ok
        ? PayloadCryptoResult{.error = PayloadCryptoError::none, .text = outer_hash.hex_digest}
        : failure(PayloadCryptoError::hash_failed);
}

PayloadCryptoVerificationResult payload_hmac_sha256_verify(
    const std::string_view key,
    const std::string_view input,
    const std::string_view expected_hex_digest) {
    if (key.size() > kPayloadCryptoMaxBytes || input.size() > kPayloadCryptoMaxBytes) {
        return {.error = PayloadCryptoError::input_too_large, .matches = false};
    }
    std::array<std::uint8_t, kSha256DigestBytes> expected_bytes{};
    if (!decode_sha256_hex(expected_hex_digest, expected_bytes)) {
        return {.error = PayloadCryptoError::invalid_digest, .matches = false};
    }
    const auto actual = payload_hmac_sha256_hex(key, input);
    if (!actual.ok()) {
        return {.error = actual.error, .matches = false};
    }

    unsigned int difference = 0U;
    for (std::size_t index = 0U; index < kSha256HexBytes; ++index) {
        difference |= static_cast<unsigned int>(
            static_cast<unsigned char>(actual.text[index]) ^
            static_cast<unsigned char>(expected_hex_digest[index]));
    }
    return {.error = PayloadCryptoError::none, .matches = difference == 0U};
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
