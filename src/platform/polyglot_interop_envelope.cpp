// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/polyglot_interop_envelope.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <set>
#include <string>
#include <string_view>
#include <utility>

namespace copperfin::platform {

namespace {

constexpr std::size_t hard_max_document_bytes =
    std::size_t{16U} * 1024U * 1024U;
constexpr std::uint32_t hard_max_nesting_depth = 64U;

bool is_lowercase_ascii_letter(const char value) noexcept {
    return value >= 'a' && value <= 'z';
}

bool is_machine_identifier(const std::string_view value) noexcept {
    if (value.empty() || !is_lowercase_ascii_letter(value.front())) {
        return false;
    }
    return std::all_of(value.begin() + 1, value.end(), [](const char character) {
        return is_lowercase_ascii_letter(character) ||
            (character >= '0' && character <= '9') || character == '.' ||
            character == '_' || character == '-';
    });
}

bool is_semantic_version(const std::string_view value) noexcept {
    unsigned int components = 0U;
    bool digit_seen = false;
    for (const char character : value) {
        if (character >= '0' && character <= '9') {
            digit_seen = true;
            continue;
        }
        if (character != '.' || !digit_seen || components >= 2U) {
            return false;
        }
        ++components;
        digit_seen = false;
    }
    return components == 2U && digit_seen;
}

PolyglotInteropEnvelopeResult invalid_result(
    const PolyglotInteropEnvelopeError error,
    const char* error_code) {
    PolyglotInteropEnvelopeResult result;
    result.error = error;
    result.error_code = error_code;
    return result;
}

bool append_utf8(std::string& output, const std::uint32_t codepoint) {
    if (codepoint <= 0x7FU) {
        output.push_back(static_cast<char>(codepoint));
        return true;
    }
    if (codepoint <= 0x7FFU) {
        output.push_back(static_cast<char>(0xC0U | (codepoint >> 6U)));
        output.push_back(static_cast<char>(0x80U | (codepoint & 0x3FU)));
        return true;
    }
    if (codepoint >= 0xD800U && codepoint <= 0xDFFFU) {
        return false;
    }
    if (codepoint <= 0xFFFFU) {
        output.push_back(static_cast<char>(0xE0U | (codepoint >> 12U)));
        output.push_back(static_cast<char>(0x80U | ((codepoint >> 6U) & 0x3FU)));
        output.push_back(static_cast<char>(0x80U | (codepoint & 0x3FU)));
        return true;
    }
    if (codepoint <= 0x10FFFFU) {
        output.push_back(static_cast<char>(0xF0U | (codepoint >> 18U)));
        output.push_back(static_cast<char>(0x80U | ((codepoint >> 12U) & 0x3FU)));
        output.push_back(static_cast<char>(0x80U | ((codepoint >> 6U) & 0x3FU)));
        output.push_back(static_cast<char>(0x80U | (codepoint & 0x3FU)));
        return true;
    }
    return false;
}

class JsonCursor final {
public:
    JsonCursor(const std::string_view document, const std::uint32_t max_depth)
        : document_(document), max_depth_(max_depth) {}

    [[nodiscard]] bool consume(const char expected) noexcept {
        skip_whitespace();
        if (position_ >= document_.size() || document_[position_] != expected) {
            return false;
        }
        ++position_;
        return true;
    }

    [[nodiscard]] bool next_is(const char expected) const noexcept {
        std::size_t position = position_;
        skip_whitespace_at(position);
        return position < document_.size() && document_[position] == expected;
    }

    [[nodiscard]] bool parse_string(std::string& value) {
        skip_whitespace();
        if (position_ >= document_.size() || document_[position_] != '"') {
            return false;
        }
        ++position_;
        value.clear();
        while (position_ < document_.size()) {
            const unsigned char character =
                static_cast<unsigned char>(document_[position_++]);
            if (character == static_cast<unsigned char>('"')) {
                return true;
            }
            if (character < 0x20U) {
                return false;
            }
            if (character == static_cast<unsigned char>('\\')) {
                if (!parse_escape(value)) {
                    return false;
                }
                continue;
            }
            if (character < 0x80U) {
                value.push_back(static_cast<char>(character));
                continue;
            }
            if (!append_raw_utf8(value, character)) {
                return false;
            }
        }
        return false;
    }

    [[nodiscard]] bool parse_boolean(bool& value) noexcept {
        skip_whitespace();
        if (match_literal("true")) {
            value = true;
            return true;
        }
        if (match_literal("false")) {
            value = false;
            return true;
        }
        return false;
    }

    [[nodiscard]] bool capture_object(std::string& value) {
        skip_whitespace();
        if (position_ >= document_.size() || document_[position_] != '{') {
            return false;
        }
        const std::size_t start = position_;
        if (!skip_value(1U)) {
            return false;
        }
        value.assign(document_.substr(start, position_ - start));
        return true;
    }

    [[nodiscard]] bool at_end() const noexcept {
        std::size_t position = position_;
        skip_whitespace_at(position);
        return position == document_.size();
    }

private:
    static int hex_value(const char value) noexcept {
        if (value >= '0' && value <= '9') {
            return value - '0';
        }
        if (value >= 'a' && value <= 'f') {
            return value - 'a' + 10;
        }
        if (value >= 'A' && value <= 'F') {
            return value - 'A' + 10;
        }
        return -1;
    }

    [[nodiscard]] bool parse_hex_quad(std::uint32_t& value) noexcept {
        if (document_.size() - position_ < 4U) {
            return false;
        }
        value = 0U;
        for (unsigned int index = 0U; index < 4U; ++index) {
            const int digit = hex_value(document_[position_++]);
            if (digit < 0) {
                return false;
            }
            value = (value << 4U) | static_cast<std::uint32_t>(digit);
        }
        return true;
    }

    [[nodiscard]] bool parse_escape(std::string& value) {
        if (position_ >= document_.size()) {
            return false;
        }
        const char escaped = document_[position_++];
        switch (escaped) {
        case '"':
        case '\\':
        case '/':
            value.push_back(escaped);
            return true;
        case 'b':
            value.push_back('\b');
            return true;
        case 'f':
            value.push_back('\f');
            return true;
        case 'n':
            value.push_back('\n');
            return true;
        case 'r':
            value.push_back('\r');
            return true;
        case 't':
            value.push_back('\t');
            return true;
        case 'u':
            break;
        default:
            return false;
        }

        std::uint32_t codepoint = 0U;
        if (!parse_hex_quad(codepoint)) {
            return false;
        }
        if (codepoint >= 0xD800U && codepoint <= 0xDBFFU) {
            if (document_.size() - position_ < 6U ||
                document_[position_] != '\\' || document_[position_ + 1U] != 'u') {
                return false;
            }
            position_ += 2U;
            std::uint32_t low_surrogate = 0U;
            if (!parse_hex_quad(low_surrogate) ||
                low_surrogate < 0xDC00U || low_surrogate > 0xDFFFU) {
                return false;
            }
            codepoint = 0x10000U + ((codepoint - 0xD800U) << 10U) +
                (low_surrogate - 0xDC00U);
        } else if (codepoint >= 0xDC00U && codepoint <= 0xDFFFU) {
            return false;
        }
        return append_utf8(value, codepoint);
    }

    [[nodiscard]] bool append_raw_utf8(
        std::string& value,
        const unsigned char first) {
        std::size_t continuation_count = 0U;
        std::uint32_t codepoint = 0U;
        std::uint32_t minimum = 0U;
        if (first >= 0xC2U && first <= 0xDFU) {
            continuation_count = 1U;
            codepoint = first & 0x1FU;
            minimum = 0x80U;
        } else if (first >= 0xE0U && first <= 0xEFU) {
            continuation_count = 2U;
            codepoint = first & 0x0FU;
            minimum = 0x800U;
        } else if (first >= 0xF0U && first <= 0xF4U) {
            continuation_count = 3U;
            codepoint = first & 0x07U;
            minimum = 0x10000U;
        } else {
            return false;
        }
        if (document_.size() - position_ < continuation_count) {
            return false;
        }
        const std::size_t sequence_start = position_ - 1U;
        for (std::size_t index = 0U; index < continuation_count; ++index) {
            const unsigned char continuation =
                static_cast<unsigned char>(document_[position_++]);
            if ((continuation & 0xC0U) != 0x80U) {
                return false;
            }
            codepoint = (codepoint << 6U) | (continuation & 0x3FU);
        }
        if (codepoint < minimum || codepoint > 0x10FFFFU ||
            (codepoint >= 0xD800U && codepoint <= 0xDFFFU)) {
            return false;
        }
        value.append(document_.substr(sequence_start, continuation_count + 1U));
        return true;
    }

    [[nodiscard]] bool match_literal(const std::string_view literal) noexcept {
        if (document_.substr(position_, literal.size()) != literal) {
            return false;
        }
        position_ += literal.size();
        return true;
    }

    [[nodiscard]] bool skip_value(const std::uint32_t depth) {
        skip_whitespace();
        if (position_ >= document_.size()) {
            return false;
        }
        const char character = document_[position_];
        if (character == '{') {
            return skip_object(depth);
        }
        if (character == '[') {
            return skip_array(depth);
        }
        if (character == '"') {
            std::string ignored;
            return parse_string(ignored);
        }
        if (character == 't') {
            return match_literal("true");
        }
        if (character == 'f') {
            return match_literal("false");
        }
        if (character == 'n') {
            return match_literal("null");
        }
        return skip_number();
    }

    [[nodiscard]] bool skip_object(const std::uint32_t depth) {
        if (depth > max_depth_ || !consume('{')) {
            return false;
        }
        std::set<std::string> keys;
        if (consume('}')) {
            return true;
        }
        while (true) {
            std::string key;
            if (!parse_string(key) || keys.find(key) != keys.end() ||
                !consume(':') || !skip_value(depth + 1U)) {
                return false;
            }
            keys.insert(std::move(key));
            if (consume('}')) {
                return true;
            }
            if (!consume(',') || next_is('}')) {
                return false;
            }
        }
    }

    [[nodiscard]] bool skip_array(const std::uint32_t depth) {
        if (depth > max_depth_ || !consume('[')) {
            return false;
        }
        if (consume(']')) {
            return true;
        }
        while (true) {
            if (!skip_value(depth + 1U)) {
                return false;
            }
            if (consume(']')) {
                return true;
            }
            if (!consume(',') || next_is(']')) {
                return false;
            }
        }
    }

    [[nodiscard]] bool skip_number() noexcept {
        if (position_ >= document_.size()) {
            return false;
        }
        if (document_[position_] == '-') {
            ++position_;
        }
        if (position_ >= document_.size()) {
            return false;
        }
        if (document_[position_] == '0') {
            ++position_;
            if (position_ < document_.size() &&
                document_[position_] >= '0' && document_[position_] <= '9') {
                return false;
            }
        } else if (document_[position_] >= '1' && document_[position_] <= '9') {
            while (position_ < document_.size() &&
                   document_[position_] >= '0' && document_[position_] <= '9') {
                ++position_;
            }
        } else {
            return false;
        }
        if (position_ < document_.size() && document_[position_] == '.') {
            ++position_;
            const std::size_t fractional_start = position_;
            while (position_ < document_.size() &&
                   document_[position_] >= '0' && document_[position_] <= '9') {
                ++position_;
            }
            if (position_ == fractional_start) {
                return false;
            }
        }
        if (position_ < document_.size() &&
            (document_[position_] == 'e' || document_[position_] == 'E')) {
            ++position_;
            if (position_ < document_.size() &&
                (document_[position_] == '+' || document_[position_] == '-')) {
                ++position_;
            }
            const std::size_t exponent_start = position_;
            while (position_ < document_.size() &&
                   document_[position_] >= '0' && document_[position_] <= '9') {
                ++position_;
            }
            if (position_ == exponent_start) {
                return false;
            }
        }
        return true;
    }

    void skip_whitespace() noexcept {
        skip_whitespace_at(position_);
    }

    void skip_whitespace_at(std::size_t& position) const noexcept {
        while (position < document_.size()) {
            const char character = document_[position];
            if (character != ' ' && character != '\t' &&
                character != '\r' && character != '\n') {
                break;
            }
            ++position;
        }
    }

    std::string_view document_;
    std::size_t position_ = 0U;
    std::uint32_t max_depth_ = 0U;
};

struct ParsedError final {
    std::string code;
    std::string message;
    bool retryable = false;
    bool has_code = false;
    bool has_message = false;
    bool has_retryable = false;
};

bool parse_error_object(JsonCursor& cursor, ParsedError& error) {
    if (!cursor.consume('{')) {
        return false;
    }
    std::set<std::string> keys;
    if (cursor.consume('}')) {
        return true;
    }
    while (true) {
        std::string key;
        if (!cursor.parse_string(key) || keys.find(key) != keys.end() ||
            !cursor.consume(':')) {
            return false;
        }
        keys.insert(key);
        if (key == "code") {
            if (!cursor.parse_string(error.code)) {
                return false;
            }
            error.has_code = true;
        } else if (key == "message") {
            if (!cursor.parse_string(error.message)) {
                return false;
            }
            error.has_message = true;
        } else if (key == "retryable") {
            if (!cursor.parse_boolean(error.retryable)) {
                return false;
            }
            error.has_retryable = true;
        } else {
            return false;
        }
        if (cursor.consume('}')) {
            return true;
        }
        if (!cursor.consume(',') || cursor.next_is('}')) {
            return false;
        }
    }
}

}  // namespace

PolyglotInteropEnvelopeResult parse_polyglot_interop_envelope(
    const std::string_view document,
    const PolyglotInteropEnvelopeExpectation& expectation) {
    if (document.empty()) {
        return invalid_result(
            PolyglotInteropEnvelopeError::document_required,
            "polyglot.envelope.document_required");
    }
    if (expectation.max_document_bytes == 0U ||
        expectation.max_document_bytes > hard_max_document_bytes ||
        expectation.max_nesting_depth == 0U ||
        expectation.max_nesting_depth > hard_max_nesting_depth) {
        return invalid_result(
            PolyglotInteropEnvelopeError::invalid_limits,
            "polyglot.envelope.invalid_limits");
    }
    if (document.size() > expectation.max_document_bytes) {
        return invalid_result(
            PolyglotInteropEnvelopeError::document_too_large,
            "polyglot.envelope.document_too_large");
    }
    if (!is_machine_identifier(expectation.capability_id)) {
        return invalid_result(
            PolyglotInteropEnvelopeError::invalid_capability_id,
            "polyglot.envelope.invalid_capability_id");
    }
    if (expectation.correlation_id.empty()) {
        return invalid_result(
            PolyglotInteropEnvelopeError::correlation_id_required,
            "polyglot.envelope.correlation_id_required");
    }
    if (!is_semantic_version(expectation.protocol_version)) {
        return invalid_result(
            PolyglotInteropEnvelopeError::invalid_protocol_version,
            "polyglot.envelope.invalid_protocol_version");
    }

    JsonCursor cursor(document, expectation.max_nesting_depth);
    if (!cursor.consume('{')) {
        return invalid_result(
            PolyglotInteropEnvelopeError::invalid_json,
            "polyglot.envelope.invalid_json");
    }

    std::set<std::string> keys;
    std::string envelope_version;
    std::string kind;
    ParsedError candidate_error;
    PolyglotInteropEnvelopeResult result;
    bool has_envelope_version = false;
    bool has_kind = false;
    bool has_capability_id = false;
    bool has_correlation_id = false;
    bool has_protocol_version = false;
    bool has_payload = false;
    bool has_error = false;

    if (!cursor.consume('}')) {
        while (true) {
            std::string key;
            if (!cursor.parse_string(key) || keys.find(key) != keys.end() ||
                !cursor.consume(':')) {
                return invalid_result(
                    PolyglotInteropEnvelopeError::invalid_document,
                    "polyglot.envelope.invalid_document");
            }
            keys.insert(key);
            if (key == "envelope_version") {
                has_envelope_version = cursor.parse_string(envelope_version);
                if (!has_envelope_version) {
                    return invalid_result(
                        PolyglotInteropEnvelopeError::invalid_json,
                        "polyglot.envelope.invalid_json");
                }
            } else if (key == "kind") {
                has_kind = cursor.parse_string(kind);
                if (!has_kind) {
                    return invalid_result(
                        PolyglotInteropEnvelopeError::invalid_json,
                        "polyglot.envelope.invalid_json");
                }
            } else if (key == "capability_id") {
                has_capability_id = cursor.parse_string(result.envelope.capability_id);
                if (!has_capability_id) {
                    return invalid_result(
                        PolyglotInteropEnvelopeError::invalid_json,
                        "polyglot.envelope.invalid_json");
                }
            } else if (key == "correlation_id") {
                has_correlation_id = cursor.parse_string(result.envelope.correlation_id);
                if (!has_correlation_id) {
                    return invalid_result(
                        PolyglotInteropEnvelopeError::invalid_json,
                        "polyglot.envelope.invalid_json");
                }
            } else if (key == "protocol_version") {
                has_protocol_version = cursor.parse_string(result.envelope.protocol_version);
                if (!has_protocol_version) {
                    return invalid_result(
                        PolyglotInteropEnvelopeError::invalid_json,
                        "polyglot.envelope.invalid_json");
                }
            } else if (key == "payload") {
                has_payload = cursor.capture_object(result.envelope.payload_json);
                if (!has_payload) {
                    return invalid_result(
                        PolyglotInteropEnvelopeError::invalid_json,
                        "polyglot.envelope.invalid_json");
                }
            } else if (key == "error") {
                has_error = parse_error_object(cursor, candidate_error);
                if (!has_error) {
                    return invalid_result(
                        PolyglotInteropEnvelopeError::invalid_document,
                        "polyglot.envelope.invalid_document");
                }
            } else {
                return invalid_result(
                    PolyglotInteropEnvelopeError::invalid_document,
                    "polyglot.envelope.invalid_document");
            }

            if (cursor.consume('}')) {
                break;
            }
            if (!cursor.consume(',') || cursor.next_is('}')) {
                return invalid_result(
                    PolyglotInteropEnvelopeError::invalid_json,
                    "polyglot.envelope.invalid_json");
            }
        }
    }
    if (!cursor.at_end()) {
        return invalid_result(
            PolyglotInteropEnvelopeError::invalid_json,
            "polyglot.envelope.invalid_json");
    }
    if (!has_envelope_version || !has_kind || !has_capability_id ||
        !has_correlation_id || !has_protocol_version) {
        return invalid_result(
            PolyglotInteropEnvelopeError::invalid_document,
            "polyglot.envelope.invalid_document");
    }
    if (envelope_version != "1.0") {
        return invalid_result(
            PolyglotInteropEnvelopeError::invalid_envelope_version,
            "polyglot.envelope.invalid_envelope_version");
    }
    if (kind == "success") {
        result.envelope.kind = PolyglotInteropEnvelopeKind::success;
        if (!has_payload || has_error) {
            return invalid_result(
                PolyglotInteropEnvelopeError::payload_required,
                "polyglot.envelope.payload_required");
        }
    } else if (kind == "error") {
        result.envelope.kind = PolyglotInteropEnvelopeKind::error;
        if (!has_error || has_payload || !candidate_error.has_code ||
            !candidate_error.has_message || !candidate_error.has_retryable) {
            return invalid_result(
                PolyglotInteropEnvelopeError::error_required,
                "polyglot.envelope.error_required");
        }
        if (!is_machine_identifier(candidate_error.code)) {
            return invalid_result(
                PolyglotInteropEnvelopeError::invalid_error_code,
                "polyglot.envelope.invalid_error_code");
        }
        result.envelope.candidate_error_code = std::move(candidate_error.code);
        result.envelope.candidate_error_message = std::move(candidate_error.message);
        result.envelope.candidate_error_retryable = candidate_error.retryable;
    } else {
        return invalid_result(
            PolyglotInteropEnvelopeError::invalid_kind,
            "polyglot.envelope.invalid_kind");
    }
    if (!is_machine_identifier(result.envelope.capability_id)) {
        return invalid_result(
            PolyglotInteropEnvelopeError::invalid_capability_id,
            "polyglot.envelope.invalid_capability_id");
    }
    if (result.envelope.capability_id != expectation.capability_id) {
        return invalid_result(
            PolyglotInteropEnvelopeError::capability_id_mismatch,
            "polyglot.envelope.capability_id_mismatch");
    }
    if (result.envelope.correlation_id.empty()) {
        return invalid_result(
            PolyglotInteropEnvelopeError::correlation_id_required,
            "polyglot.envelope.correlation_id_required");
    }
    if (result.envelope.correlation_id != expectation.correlation_id) {
        return invalid_result(
            PolyglotInteropEnvelopeError::correlation_id_mismatch,
            "polyglot.envelope.correlation_id_mismatch");
    }
    if (!is_semantic_version(result.envelope.protocol_version)) {
        return invalid_result(
            PolyglotInteropEnvelopeError::invalid_protocol_version,
            "polyglot.envelope.invalid_protocol_version");
    }
    if (result.envelope.protocol_version != expectation.protocol_version) {
        return invalid_result(
            PolyglotInteropEnvelopeError::protocol_version_mismatch,
            "polyglot.envelope.protocol_version_mismatch");
    }
    return result;
}

}  // namespace copperfin::platform
