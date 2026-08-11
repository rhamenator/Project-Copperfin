// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/json.h"

#include <algorithm>
#include <iterator>
#include <limits>
#include <set>
#include <vector>

namespace copperfin::platform {

std::string json_escape_string(std::string_view value) {
    constexpr char hex_digits[] = "0123456789abcdef";
    std::string escaped;
    escaped.reserve(value.size());
    for (const unsigned char ch : value) {
        switch (ch) {
            case '"': escaped += "\\\""; break;
            case '\\': escaped += "\\\\"; break;
            case '\b': escaped += "\\b"; break;
            case '\f': escaped += "\\f"; break;
            case '\n': escaped += "\\n"; break;
            case '\r': escaped += "\\r"; break;
            case '\t': escaped += "\\t"; break;
            default:
                if (ch < 0x20U) {
                    escaped += "\\u00";
                    escaped.push_back(hex_digits[(ch >> 4U) & 0x0FU]);
                    escaped.push_back(hex_digits[ch & 0x0FU]);
                } else {
                    escaped.push_back(static_cast<char>(ch));
                }
                break;
        }
    }
    return escaped;
}

namespace {

constexpr std::size_t hard_max_document_bytes =
    std::size_t{16U} * 1024U * 1024U;
constexpr std::uint32_t hard_max_nesting_depth = 64U;
constexpr std::size_t hard_max_value_count = 65536U;

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

bool is_valid_utf8(const std::string_view value) noexcept {
    std::size_t position = 0U;
    while (position < value.size()) {
        const unsigned char first = static_cast<unsigned char>(value[position++]);
        if (first < 0x80U) {
            continue;
        }
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
        if (value.size() - position < continuation_count) {
            return false;
        }
        for (std::size_t index = 0U; index < continuation_count; ++index) {
            const unsigned char continuation =
                static_cast<unsigned char>(value[position++]);
            if ((continuation & 0xC0U) != 0x80U) {
                return false;
            }
            codepoint = (codepoint << 6U) | (continuation & 0x3FU);
        }
        if (codepoint < minimum || codepoint > 0x10FFFFU ||
            (codepoint >= 0xD800U && codepoint <= 0xDFFFU)) {
            return false;
        }
    }
    return true;
}

struct JsonNode final {
    JsonValueKind kind = JsonValueKind::invalid;
    std::size_t start = 0U;
    std::size_t end = 0U;
    std::string decoded_string;
    std::vector<JsonNode> array_values;
    std::vector<std::string> object_keys;
    std::vector<JsonNode> object_values;
};

class JsonDocumentParser final {
public:
    JsonDocumentParser(
        const std::string_view document,
        const std::uint32_t max_depth,
        const std::size_t max_value_count)
        : document_(document),
          max_depth_(max_depth),
          max_value_count_(max_value_count) {}

    [[nodiscard]] bool parse(JsonNode& root) {
        return parse_value(root, 1U) && at_end();
    }

    [[nodiscard]] bool value_count_exceeded() const noexcept {
        return value_count_exceeded_;
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

    void skip_whitespace() noexcept {
        while (position_ < document_.size()) {
            const char character = document_[position_];
            if (character != ' ' && character != '\t' &&
                character != '\r' && character != '\n') {
                return;
            }
            ++position_;
        }
    }

    [[nodiscard]] bool at_end() noexcept {
        skip_whitespace();
        return position_ == document_.size();
    }

    [[nodiscard]] bool consume(const char expected) noexcept {
        skip_whitespace();
        if (position_ >= document_.size() || document_[position_] != expected) {
            return false;
        }
        ++position_;
        return true;
    }

    [[nodiscard]] bool next_is(const char expected) noexcept {
        skip_whitespace();
        return position_ < document_.size() && document_[position_] == expected;
    }

    [[nodiscard]] bool match_literal(const std::string_view literal) noexcept {
        if (document_.substr(position_, literal.size()) != literal) {
            return false;
        }
        position_ += literal.size();
        return true;
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
        case '/': value.push_back(escaped); return true;
        case 'b': value.push_back('\b'); return true;
        case 'f': value.push_back('\f'); return true;
        case 'n': value.push_back('\n'); return true;
        case 'r': value.push_back('\r'); return true;
        case 't': value.push_back('\t'); return true;
        case 'u': break;
        default: return false;
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
        if (first >= 0xC2U && first <= 0xDFU) {
            continuation_count = 1U;
        } else if (first >= 0xE0U && first <= 0xEFU) {
            continuation_count = 2U;
        } else if (first >= 0xF0U && first <= 0xF4U) {
            continuation_count = 3U;
        } else {
            return false;
        }
        if (document_.size() - position_ < continuation_count) {
            return false;
        }
        const std::size_t start = position_ - 1U;
        for (std::size_t index = 0U; index < continuation_count; ++index) {
            const unsigned char continuation =
                static_cast<unsigned char>(document_[position_++]);
            if ((continuation & 0xC0U) != 0x80U) {
                return false;
            }
        }
        value.append(document_.substr(start, continuation_count + 1U));
        return true;
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
            } else if (character < 0x80U) {
                value.push_back(static_cast<char>(character));
            } else if (!append_raw_utf8(value, character)) {
                return false;
            }
        }
        return false;
    }

    [[nodiscard]] bool parse_number() noexcept {
        if (position_ < document_.size() && document_[position_] == '-') {
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
            const std::size_t start = position_;
            while (position_ < document_.size() &&
                   document_[position_] >= '0' && document_[position_] <= '9') {
                ++position_;
            }
            if (position_ == start) {
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
            const std::size_t start = position_;
            while (position_ < document_.size() &&
                   document_[position_] >= '0' && document_[position_] <= '9') {
                ++position_;
            }
            if (position_ == start) {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] bool parse_value(JsonNode& node, const std::uint32_t depth) {
        skip_whitespace();
        if (position_ >= document_.size()) {
            return false;
        }
        if (value_count_ >= max_value_count_) {
            value_count_exceeded_ = true;
            return false;
        }
        ++value_count_;
        node.start = position_;
        const char character = document_[position_];
        bool parsed = false;
        if (character == '{') {
            node.kind = JsonValueKind::object;
            parsed = parse_object(node, depth);
        } else if (character == '[') {
            node.kind = JsonValueKind::array;
            parsed = parse_array(node, depth);
        } else if (character == '"') {
            node.kind = JsonValueKind::string;
            parsed = parse_string(node.decoded_string);
        } else if (character == 't') {
            node.kind = JsonValueKind::boolean;
            parsed = match_literal("true");
        } else if (character == 'f') {
            node.kind = JsonValueKind::boolean;
            parsed = match_literal("false");
        } else if (character == 'n') {
            node.kind = JsonValueKind::null_value;
            parsed = match_literal("null");
        } else {
            node.kind = JsonValueKind::number;
            parsed = parse_number();
        }
        if (parsed) {
            node.end = position_;
        }
        return parsed;
    }

    [[nodiscard]] bool parse_object(JsonNode& node, const std::uint32_t depth) {
        if (depth > max_depth_ || !consume('{')) {
            return false;
        }
        std::set<std::string> keys;
        if (consume('}')) {
            return true;
        }
        while (true) {
            std::string key;
            if (!parse_string(key) || keys.contains(key) || !consume(':')) {
                return false;
            }
            keys.insert(key);
            JsonNode value;
            if (!parse_value(value, depth + 1U)) {
                return false;
            }
            node.object_keys.push_back(std::move(key));
            node.object_values.push_back(std::move(value));
            if (consume('}')) {
                return true;
            }
            if (!consume(',') || next_is('}')) {
                return false;
            }
        }
    }

    [[nodiscard]] bool parse_array(JsonNode& node, const std::uint32_t depth) {
        if (depth > max_depth_ || !consume('[')) {
            return false;
        }
        if (consume(']')) {
            return true;
        }
        while (true) {
            JsonNode value;
            if (!parse_value(value, depth + 1U)) {
                return false;
            }
            node.array_values.push_back(std::move(value));
            if (consume(']')) {
                return true;
            }
            if (!consume(',') || next_is(']')) {
                return false;
            }
        }
    }

    std::string_view document_;
    std::size_t position_ = 0U;
    std::uint32_t max_depth_ = 0U;
    std::size_t max_value_count_ = 0U;
    std::size_t value_count_ = 0U;
    bool value_count_exceeded_ = false;
};

bool decode_pointer_token(const std::string_view encoded, std::string& decoded) {
    decoded.clear();
    decoded.reserve(encoded.size());
    for (std::size_t index = 0U; index < encoded.size(); ++index) {
        if (encoded[index] != '~') {
            decoded.push_back(encoded[index]);
            continue;
        }
        if (++index >= encoded.size()) {
            return false;
        }
        if (encoded[index] == '0') {
            decoded.push_back('~');
        } else if (encoded[index] == '1') {
            decoded.push_back('/');
        } else {
            return false;
        }
    }
    return true;
}

bool parse_array_index(const std::string_view token, std::size_t& index) noexcept {
    if (token.empty() || (token.size() > 1U && token.front() == '0')) {
        return false;
    }
    index = 0U;
    for (const char character : token) {
        if (character < '0' || character > '9') {
            return false;
        }
        const std::size_t digit = static_cast<std::size_t>(character - '0');
        if (index > (std::numeric_limits<std::size_t>::max() - digit) / 10U) {
            return false;
        }
        index = index * 10U + digit;
    }
    return true;
}

JsonSelectionResult error_result(const JsonSelectionError error) {
    JsonSelectionResult result;
    result.error = error;
    return result;
}

}  // namespace

JsonSelectionResult select_json_value(
    const std::string_view document,
    const std::string_view json_pointer,
    const JsonDocumentLimits& limits) {
    if (document.empty()) {
        return error_result(JsonSelectionError::document_required);
    }
    if (limits.max_document_bytes == 0U ||
        limits.max_document_bytes > hard_max_document_bytes ||
        limits.max_nesting_depth == 0U ||
        limits.max_nesting_depth > hard_max_nesting_depth ||
        limits.max_value_count == 0U ||
        limits.max_value_count > hard_max_value_count) {
        return error_result(JsonSelectionError::invalid_limits);
    }
    if (document.size() > limits.max_document_bytes) {
        return error_result(JsonSelectionError::document_too_large);
    }
    if (!is_valid_utf8(document)) {
        return error_result(JsonSelectionError::invalid_utf8);
    }

    JsonNode root;
    JsonDocumentParser parser(
        document,
        limits.max_nesting_depth,
        limits.max_value_count);
    if (!parser.parse(root)) {
        if (parser.value_count_exceeded()) {
            return error_result(JsonSelectionError::value_count_exceeded);
        }
        return error_result(JsonSelectionError::invalid_json);
    }

    const JsonNode* selected = &root;
    if (!json_pointer.empty()) {
        if (json_pointer.front() != '/') {
            return error_result(JsonSelectionError::invalid_pointer);
        }
        std::size_t token_start = 1U;
        while (true) {
            const std::size_t separator = json_pointer.find('/', token_start);
            const std::string_view encoded = json_pointer.substr(
                token_start,
                separator == std::string_view::npos
                    ? std::string_view::npos
                    : separator - token_start);
            std::string token;
            if (!decode_pointer_token(encoded, token)) {
                return error_result(JsonSelectionError::invalid_pointer);
            }
            if (selected->kind == JsonValueKind::object) {
                const auto match = std::find_if(
                    selected->object_keys.begin(),
                    selected->object_keys.end(),
                    [&](const std::string& key) { return key == token; });
                if (match == selected->object_keys.end()) {
                    return error_result(JsonSelectionError::value_not_found);
                }
                const auto index = static_cast<std::size_t>(
                    std::distance(selected->object_keys.begin(), match));
                selected = &selected->object_values[index];
            } else if (selected->kind == JsonValueKind::array) {
                std::size_t index = 0U;
                if (!parse_array_index(token, index) ||
                    index >= selected->array_values.size()) {
                    return error_result(JsonSelectionError::value_not_found);
                }
                selected = &selected->array_values[index];
            } else {
                return error_result(JsonSelectionError::value_not_found);
            }
            if (separator == std::string_view::npos) {
                break;
            }
            token_start = separator + 1U;
        }
    }

    JsonSelectionResult result;
    result.kind = selected->kind;
    result.raw_json.assign(document.substr(
        selected->start,
        selected->end - selected->start));
    result.decoded_string = selected->decoded_string;
    return result;
}

JsonObjectMembersResult select_json_object_member_names(
    const std::string_view document,
    const std::string_view json_pointer,
    const JsonDocumentLimits& limits) {
    const JsonSelectionResult selection =
        select_json_value(document, json_pointer, limits);
    if (!selection.ok()) {
        return {.error = selection.error, .names = {}};
    }
    if (selection.kind != JsonValueKind::object) {
        return {.error = JsonSelectionError::value_not_found, .names = {}};
    }

    JsonNode root;
    JsonDocumentParser parser(
        selection.raw_json,
        limits.max_nesting_depth,
        limits.max_value_count);
    if (!parser.parse(root) || root.kind != JsonValueKind::object) {
        return {
            .error = parser.value_count_exceeded()
                ? JsonSelectionError::value_count_exceeded
                : JsonSelectionError::invalid_json,
            .names = {}};
    }
    return {.error = JsonSelectionError::none, .names = std::move(root.object_keys)};
}

std::string_view json_value_kind_name(const JsonValueKind kind) noexcept {
    switch (kind) {
    case JsonValueKind::null_value: return "null";
    case JsonValueKind::boolean: return "boolean";
    case JsonValueKind::number: return "number";
    case JsonValueKind::string: return "string";
    case JsonValueKind::array: return "array";
    case JsonValueKind::object: return "object";
    case JsonValueKind::invalid: return "invalid";
    }
    return "invalid";
}

}  // namespace copperfin::platform
