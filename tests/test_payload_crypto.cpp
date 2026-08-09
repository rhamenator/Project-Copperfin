// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/security/payload_crypto.h"

#include <iostream>
#include <string>
#include <vector>

namespace {

int failures = 0;

void expect(const bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
}

void test_sha256_vectors() {
    using copperfin::security::payload_sha256_hex;

    const auto empty = payload_sha256_hex("");
    expect(empty.ok() &&
               empty.text == "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855",
           "SHA-256 should match the empty-input vector");
    const auto abc = payload_sha256_hex("abc");
    expect(abc.ok() &&
               abc.text == "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad",
           "SHA-256 should match the abc vector");
}

void test_base64_vectors_and_binary_round_trip() {
    using copperfin::security::payload_base64_decode;
    using copperfin::security::payload_base64_encode;

    const std::vector<std::pair<std::string, std::string>> vectors{
        {"", ""},
        {"f", "Zg=="},
        {"fo", "Zm8="},
        {"foo", "Zm9v"},
        {"foob", "Zm9vYg=="},
        {"fooba", "Zm9vYmE="},
        {"foobar", "Zm9vYmFy"},
    };
    for (const auto& [plain, encoded] : vectors) {
        const auto encoded_result = payload_base64_encode(plain);
        expect(encoded_result.ok() && encoded_result.text == encoded,
               "Base64 encoding should match the RFC 4648 vector for " + plain);
        const auto decoded_result = payload_base64_decode(encoded);
        expect(decoded_result.ok() && decoded_result.text == plain,
               "Base64 decoding should match the RFC 4648 vector for " + plain);
    }

    const std::string binary{"\0\xff\x10", 3U};
    const auto encoded = payload_base64_encode(binary);
    expect(encoded.ok() && encoded.text == "AP8Q",
           "Base64 should preserve zero and high bytes");
    const auto decoded = payload_base64_decode(encoded.text);
    expect(decoded.ok() && decoded.text == binary,
           "Base64 binary round trip should preserve exact bytes");
}

void test_strict_decode_and_fixed_bounds() {
    using copperfin::security::PayloadCryptoError;
    using copperfin::security::kPayloadCryptoMaxBase64Bytes;
    using copperfin::security::kPayloadCryptoMaxBytes;
    using copperfin::security::payload_base64_decode;
    using copperfin::security::payload_base64_encode;
    using copperfin::security::payload_sha256_hex;

    for (const std::string malformed : {
             "Zg=", "Zg=A", "Zg==\n", "Z g==", "Zg==Zg==", "====",
             "Zh==", "Zm9=", "Zg--"}) {
        const auto result = payload_base64_decode(malformed);
        expect(result.error == PayloadCryptoError::invalid_base64,
               "strict Base64 should reject malformed/noncanonical text: " + malformed);
    }

    const std::string maximum(kPayloadCryptoMaxBytes, 'x');
    const auto encoded = payload_base64_encode(maximum);
    expect(encoded.ok() && encoded.text.size() == kPayloadCryptoMaxBase64Bytes,
           "exact maximum payload should encode within the fixed text ceiling");
    const auto decoded = payload_base64_decode(encoded.text);
    expect(decoded.ok() && decoded.text == maximum,
           "exact maximum payload should round trip");
    expect(payload_sha256_hex(maximum).ok(),
           "exact maximum payload should hash");

    const std::string oversized(kPayloadCryptoMaxBytes + 1U, 'x');
    expect(payload_sha256_hex(oversized).error == PayloadCryptoError::input_too_large,
           "SHA-256 should reject input above the fixed ceiling");
    expect(payload_base64_encode(oversized).error == PayloadCryptoError::input_too_large,
           "Base64 encoding should reject input above the fixed ceiling");
    expect(payload_base64_decode(std::string(kPayloadCryptoMaxBase64Bytes + 4U, 'A')).error ==
               PayloadCryptoError::input_too_large,
           "Base64 decoding should reject text above the fixed ceiling");
}

}  // namespace

int main() {
    test_sha256_vectors();
    test_base64_vectors_and_binary_round_trip();
    test_strict_decode_and_fixed_bounds();
    if (failures != 0) {
        std::cerr << failures << " test(s) failed\n";
        return 1;
    }
    return 0;
}
