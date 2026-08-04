// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "license_payload_parser.h"

#include <cctype>
#include <cstddef>
#include <limits>
#include <utility>

namespace copperfin::licensing {

namespace {

void set_error(
    ParsedLicenseFile& result,
    std::string key,
    std::string raw,
    std::string argument = {}) {
    result.error = std::move(raw);
    result.error_key = std::move(key);
    result.error_argument = std::move(argument);
}

struct Cursor {
    std::string_view text;
    std::size_t pos = 0;
};

void skip_ws(Cursor& cursor) {
    while (cursor.pos < cursor.text.size()) {
        const char ch = cursor.text[cursor.pos];
        if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') {
            ++cursor.pos;
        } else {
            break;
        }
    }
}

bool consume(Cursor& cursor, char expected) {
    skip_ws(cursor);
    if (cursor.pos < cursor.text.size() && cursor.text[cursor.pos] == expected) {
        ++cursor.pos;
        return true;
    }
    return false;
}

bool peek_is(Cursor& cursor, char expected) {
    skip_ws(cursor);
    return cursor.pos < cursor.text.size() && cursor.text[cursor.pos] == expected;
}

bool is_high_surrogate(unsigned int code_unit) {
    return code_unit >= 0xD800U && code_unit <= 0xDBFFU;
}

bool is_low_surrogate(unsigned int code_unit) {
    return code_unit >= 0xDC00U && code_unit <= 0xDFFFU;
}

bool append_utf8(std::string& out, unsigned int code_point) {
    if (code_point > 0x10FFFFU || is_high_surrogate(code_point) || is_low_surrogate(code_point)) {
        return false;
    }

    if (code_point < 0x80U) {
        out.push_back(static_cast<char>(code_point));
    } else if (code_point < 0x800U) {
        out.push_back(static_cast<char>(0xC0U | (code_point >> 6U)));
        out.push_back(static_cast<char>(0x80U | (code_point & 0x3FU)));
    } else if (code_point < 0x10000U) {
        out.push_back(static_cast<char>(0xE0U | (code_point >> 12U)));
        out.push_back(static_cast<char>(0x80U | ((code_point >> 6U) & 0x3FU)));
        out.push_back(static_cast<char>(0x80U | (code_point & 0x3FU)));
    } else {
        out.push_back(static_cast<char>(0xF0U | (code_point >> 18U)));
        out.push_back(static_cast<char>(0x80U | ((code_point >> 12U) & 0x3FU)));
        out.push_back(static_cast<char>(0x80U | ((code_point >> 6U) & 0x3FU)));
        out.push_back(static_cast<char>(0x80U | (code_point & 0x3FU)));
    }
    return true;
}

bool parse_hex4(Cursor& cursor, unsigned int& value) {
    if (cursor.pos > cursor.text.size() || (cursor.text.size() - cursor.pos) < 4U) {
        return false;
    }
    value = 0U;
    for (unsigned int k = 0U; k < 4U; ++k) {
        const char hex_char = cursor.text[cursor.pos + k];
        value <<= 4U;
        if (hex_char >= '0' && hex_char <= '9') {
            value |= static_cast<unsigned int>(hex_char - '0');
        } else if (hex_char >= 'a' && hex_char <= 'f') {
            value |= static_cast<unsigned int>(hex_char - 'a' + 10);
        } else if (hex_char >= 'A' && hex_char <= 'F') {
            value |= static_cast<unsigned int>(hex_char - 'A' + 10);
        } else {
            return false;
        }
    }
    cursor.pos += 4U;
    return true;
}

bool parse_json_string(Cursor& cursor, std::string& out) {
    skip_ws(cursor);
    if (cursor.pos >= cursor.text.size() || cursor.text[cursor.pos] != '"') {
        return false;
    }
    ++cursor.pos;
    out.clear();

    while (true) {
        if (cursor.pos >= cursor.text.size()) {
            return false;
        }
        const char ch = cursor.text[cursor.pos];
        if (ch == '"') {
            ++cursor.pos;
            return true;
        }
        if (ch == '\\') {
            ++cursor.pos;
            if (cursor.pos >= cursor.text.size()) {
                return false;
            }
            const char escaped = cursor.text[cursor.pos];
            switch (escaped) {
                case '"':
                    out.push_back('"');
                    ++cursor.pos;
                    break;
                case '\\':
                    out.push_back('\\');
                    ++cursor.pos;
                    break;
                case '/':
                    out.push_back('/');
                    ++cursor.pos;
                    break;
                case 'b':
                    out.push_back('\b');
                    ++cursor.pos;
                    break;
                case 'f':
                    out.push_back('\f');
                    ++cursor.pos;
                    break;
                case 'n':
                    out.push_back('\n');
                    ++cursor.pos;
                    break;
                case 'r':
                    out.push_back('\r');
                    ++cursor.pos;
                    break;
                case 't':
                    out.push_back('\t');
                    ++cursor.pos;
                    break;
                case 'u': {
                    ++cursor.pos;
                    unsigned int code_point = 0U;
                    if (!parse_hex4(cursor, code_point)) {
                        return false;
                    }
                    if (is_high_surrogate(code_point)) {
                        if (cursor.pos >= cursor.text.size() || cursor.text[cursor.pos] != '\\') {
                            return false;
                        }
                        ++cursor.pos;
                        if (cursor.pos >= cursor.text.size() || cursor.text[cursor.pos] != 'u') {
                            return false;
                        }
                        ++cursor.pos;

                        unsigned int low_surrogate = 0U;
                        if (!parse_hex4(cursor, low_surrogate) || !is_low_surrogate(low_surrogate)) {
                            return false;
                        }
                        code_point = 0x10000U +
                            ((code_point - 0xD800U) << 10U) +
                            (low_surrogate - 0xDC00U);
                    } else if (is_low_surrogate(code_point)) {
                        return false;
                    }
                    if (!append_utf8(out, code_point)) {
                        return false;
                    }
                    break;
                }
                default:
                    return false;
            }
        } else if (static_cast<unsigned char>(ch) < 0x20U) {
            return false;
        } else {
            out.push_back(ch);
            ++cursor.pos;
        }
    }
}

bool parse_json_integer(Cursor& cursor, long long& out) {
    skip_ws(cursor);
    const std::size_t start = cursor.pos;
    bool negative = false;
    if (cursor.pos < cursor.text.size() && cursor.text[cursor.pos] == '-') {
        negative = true;
        ++cursor.pos;
    }
    if (cursor.pos >= cursor.text.size() || (std::isdigit(static_cast<unsigned char>(cursor.text[cursor.pos])) == 0)) {
        cursor.pos = start;
        return false;
    }

    const auto positive_limit = static_cast<unsigned long long>(std::numeric_limits<long long>::max());
    const unsigned long long magnitude_limit = negative ? positive_limit + 1ULL : positive_limit;
    unsigned long long magnitude = 0ULL;
    while (cursor.pos < cursor.text.size() && (std::isdigit(static_cast<unsigned char>(cursor.text[cursor.pos])) != 0)) {
        const auto digit = static_cast<unsigned long long>(cursor.text[cursor.pos] - '0');
        if (magnitude > (magnitude_limit - digit) / 10ULL) {
            cursor.pos = start;
            return false;
        }
        magnitude = (magnitude * 10ULL) + digit;
        ++cursor.pos;
    }

    if (cursor.pos < cursor.text.size() &&
        (cursor.text[cursor.pos] == '.' || cursor.text[cursor.pos] == 'e' || cursor.text[cursor.pos] == 'E')) {
        cursor.pos = start;
        return false;
    }

    if (negative && magnitude == magnitude_limit) {
        out = std::numeric_limits<long long>::min();
    } else {
        const auto signed_magnitude = static_cast<long long>(magnitude);
        out = negative ? -signed_magnitude : signed_magnitude;
    }
    return true;
}

bool parse_flat_scalar_object(Cursor& cursor, PayloadFields& out) {
    if (!consume(cursor, '{')) {
        return false;
    }
    if (peek_is(cursor, '}')) {
        consume(cursor, '}');
        return true;
    }

    while (true) {
        std::string key;
        if (!parse_json_string(cursor, key)) {
            return false;
        }
        if (!consume(cursor, ':')) {
            return false;
        }

        if (peek_is(cursor, '"')) {
            std::string value;
            if (!parse_json_string(cursor, value)) {
                return false;
            }
            out[key] = PayloadValue::make_string(value);
        } else {
            long long value = 0;
            if (!parse_json_integer(cursor, value)) {
                return false;
            }
            out[key] = PayloadValue::make_integer(value);
        }

        if (consume(cursor, ',')) {
            continue;
        }
        if (consume(cursor, '}')) {
            return true;
        }
        return false;
    }
}

}  // namespace

ParsedLicenseFile parse_license_file(std::string_view json_text) {
    ParsedLicenseFile result;
    Cursor cursor{json_text, 0};

    if (!consume(cursor, '{')) {
        set_error(result, "Licensing.Error.ExpectedTopLevelObject", "expected top-level JSON object");
        return result;
    }

    if (peek_is(cursor, '}')) {
        consume(cursor, '}');
        set_error(result, "Licensing.Error.NoFields", "license file has no fields");
        return result;
    }

    bool has_payload = false;
    bool has_algorithm = false;
    bool has_signature = false;

    while (true) {
        std::string key;
        if (!parse_json_string(cursor, key)) {
            set_error(result, "Licensing.Error.ExpectedJsonKey", "expected a JSON key string");
            return result;
        }
        if (!consume(cursor, ':')) {
            set_error(result, "Licensing.Error.ExpectedColonAfterKey", "expected ':' after key");
            return result;
        }

        if (key == "payload") {
            if (!parse_flat_scalar_object(cursor, result.payload_fields)) {
                set_error(result, "Licensing.Error.MalformedPayloadObject", "malformed payload object");
                return result;
            }
            has_payload = true;
        } else if (key == "signature_algorithm") {
            if (!parse_json_string(cursor, result.signature_algorithm)) {
                set_error(result, "Licensing.Error.MalformedSignatureAlgorithmField", "malformed signature_algorithm field");
                return result;
            }
            has_algorithm = true;
        } else if (key == "signature") {
            if (!parse_json_string(cursor, result.signature_base64)) {
                set_error(result, "Licensing.Error.MalformedSignatureField", "malformed signature field");
                return result;
            }
            has_signature = true;
        } else {
            set_error(
                result,
                "Licensing.Error.UnexpectedTopLevelKey",
                "unexpected top-level key: " + key,
                key);
            return result;
        }

        if (consume(cursor, ',')) {
            continue;
        }
        if (consume(cursor, '}')) {
            break;
        }
        set_error(result, "Licensing.Error.ExpectedCommaOrObjectEnd", "expected ',' or '}'");
        return result;
    }

    if (!has_payload || !has_algorithm || !has_signature) {
        set_error(
            result,
            "Licensing.Error.MissingRequiredField",
            "missing required top-level field (payload, signature_algorithm, signature)");
        return result;
    }

    result.ok = true;
    return result;
}

}  // namespace copperfin::licensing
