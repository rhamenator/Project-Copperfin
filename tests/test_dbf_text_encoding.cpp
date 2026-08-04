// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "copperfin/vfp/dbf_text_encoding.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <string_view>

namespace {

int failures = 0;

void expect(bool condition, std::string_view message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

void test_cp1252_decode_and_encode() {
    constexpr std::string_view legacy_bytes = "caf\xE9 \x80";
    constexpr std::string_view utf8_text = "caf\xC3\xA9 \xE2\x82\xAC";

    const auto decoded = copperfin::vfp::decode_dbf_text(0x03U, legacy_bytes);
    expect(decoded.ok, "CP1252 decoding should succeed");
    expect(decoded.error == copperfin::vfp::DbfTextEncodingError::none,
           "CP1252 decoding should not report an error");
    expect(decoded.text == utf8_text, "CP1252 decoding should produce exact UTF-8 bytes");

    const auto encoded = copperfin::vfp::encode_dbf_text(0x03U, utf8_text);
    expect(encoded.ok, "CP1252 encoding should succeed");
    expect(encoded.error == copperfin::vfp::DbfTextEncodingError::none,
           "CP1252 encoding should not report an error");
    expect(encoded.text == legacy_bytes, "CP1252 encoding should produce exact legacy bytes");
}

void test_cp1251_decode_and_encode() {
    constexpr std::string_view legacy_bytes = "\xCF\xF0\xE8\xE2\xE5\xF2";
    constexpr std::string_view utf8_text = "\xD0\x9F\xD1\x80\xD0\xB8\xD0\xB2\xD0\xB5\xD1\x82";

    const auto decoded = copperfin::vfp::decode_dbf_text(0xC9U, legacy_bytes);
    expect(decoded.ok, "CP1251 decoding should succeed");
    expect(decoded.error == copperfin::vfp::DbfTextEncodingError::none,
           "CP1251 decoding should not report an error");
    expect(decoded.text == utf8_text, "CP1251 decoding should produce exact UTF-8 bytes");

    const auto encoded = copperfin::vfp::encode_dbf_text(0xC9U, utf8_text);
    expect(encoded.ok, "CP1251 encoding should succeed");
    expect(encoded.error == copperfin::vfp::DbfTextEncodingError::none,
           "CP1251 encoding should not report an error");
    expect(encoded.text == legacy_bytes, "CP1251 encoding should produce exact legacy bytes");
}

void test_cp1252_rejects_unrepresentable_characters() {
    constexpr std::string_view emoji = "\xF0\x9F\x98\x80";

    const auto encoded = copperfin::vfp::encode_dbf_text(0x03U, emoji);
    expect(!encoded.ok, "CP1252 encoding should reject an emoji");
    expect(encoded.error == copperfin::vfp::DbfTextEncodingError::unrepresentable_character,
           "CP1252 emoji failure should report an unrepresentable character");
    expect(encoded.text.empty(), "failed CP1252 encoding should not produce bytes");
}

void test_invalid_utf8_writes_fail() {
    constexpr std::string_view invalid_utf8 = "\xC3\x28";

    const auto encoded = copperfin::vfp::encode_dbf_text(0x03U, invalid_utf8);
    expect(!encoded.ok, "encoding invalid UTF-8 should fail");
    expect(encoded.text.empty(), "invalid UTF-8 encoding should not produce bytes");
}

void test_unmarked_utf8_compatibility_round_trip() {
    constexpr std::string_view utf8_text =
        "Caf\xC3\xA9 \xD0\x9F\xD1\x80\xD0\xB8\xD0\xB2\xD0\xB5\xD1\x82 \xF0\x9F\x98\x80";

    const auto encoded = copperfin::vfp::encode_dbf_text(0U, utf8_text);
    expect(encoded.ok, "unmarked UTF-8 encoding should succeed");
    expect(encoded.error == copperfin::vfp::DbfTextEncodingError::none,
           "unmarked UTF-8 encoding should not report an error");
    expect(encoded.text == utf8_text, "unmarked UTF-8 encoding should preserve exact bytes");

    const auto decoded = copperfin::vfp::decode_dbf_text(0U, encoded.text);
    expect(decoded.ok, "unmarked UTF-8 decoding should succeed");
    expect(decoded.error == copperfin::vfp::DbfTextEncodingError::none,
           "unmarked UTF-8 decoding should not report an error");
    expect(decoded.text == utf8_text, "unmarked UTF-8 decoding should preserve exact bytes");
}

}  // namespace

int main() {
    test_cp1252_decode_and_encode();
    test_cp1251_decode_and_encode();
    test_cp1252_rejects_unrepresentable_characters();
    test_invalid_utf8_writes_fail();
    test_unmarked_utf8_compatibility_round_trip();

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
