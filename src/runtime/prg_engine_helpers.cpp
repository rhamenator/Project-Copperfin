// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "prg_engine_helpers.h"

#include "localized_text.h"
#include "prg_compatibility_error.h"

#include "copperfin/platform/invariant_numeric.h"
#include "copperfin/platform/path.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <ctime>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <functional>
#include <iomanip>
#include <limits>
#include <locale>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace copperfin::runtime {

std::string trim_copy(std::string value) {
    value.erase(value.begin(), std::find_if(value.begin(), value.end(), [](unsigned char ch) {
        return std::isspace(ch) == 0;
    }));
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back())) != 0) {
        value.pop_back();
    }
    return value;
}

std::string ltrim_space_copy(std::string value) {
    const std::size_t start = value.find_first_not_of(' ');
    return start == std::string::npos ? std::string{} : value.substr(start);
}

std::string rtrim_space_copy(std::string value) {
    const std::size_t end = value.find_last_not_of(' ');
    return end == std::string::npos ? std::string{} : value.substr(0U, end + 1U);
}

std::string trim_space_copy(std::string value) {
    const std::size_t start = value.find_first_not_of(' ');
    if (start == std::string::npos) {
        return {};
    }
    const std::size_t end = value.find_last_not_of(' ');
    return value.substr(start, end - start + 1U);
}

std::string lowercase_copy(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

bool starts_with_insensitive(const std::string& value, const std::string& prefix) {
    if (value.size() < prefix.size()) {
        return false;
    }
    for (std::size_t index = 0; index < prefix.size(); ++index) {
        if (std::tolower(static_cast<unsigned char>(value[index])) !=
            std::tolower(static_cast<unsigned char>(prefix[index]))) {
            return false;
        }
    }
    return true;
}

bool paths_equal_insensitive(const std::string& left, const std::string& right) {
    return copperfin::platform::path_equal_case_insensitive(
        copperfin::platform::path_from_utf8_string(left),
        copperfin::platform::path_from_utf8_string(right));
}

std::string normalize_identifier(std::string value) {
    return lowercase_copy(trim_copy(std::move(value)));
}

bool declared_dll_type_uses_64_bit_integer(std::string type_name) {
    const std::string normalized = normalize_identifier(std::move(type_name));
    return normalized == "longlong" ||
           normalized == "integer64" ||
           normalized == "i64";
}

bool declared_dll_type_is_single(std::string type_name) {
    return normalize_identifier(std::move(type_name)) == "single";
}

bool declared_dll_type_is_short(std::string type_name) {
    return normalize_identifier(std::move(type_name)) == "short";
}

bool declared_dll_type_is_numeric_parameter(std::string type_name) {
    const std::string normalized = normalize_identifier(std::move(type_name));
    return normalized == "integer" || normalized == "i" ||
           normalized == "long" || normalized == "l" ||
           normalized == "double" || normalized == "d" || normalized == "f" ||
           declared_dll_type_is_single(normalized) ||
           declared_dll_type_uses_64_bit_integer(normalized);
}

bool declared_dll_parameter_list_contains_type(
    const std::string& parameter_types,
    const std::string& requested_type) {
    const std::string normalized_requested_type = normalize_identifier(requested_type);
    std::istringstream parameters(parameter_types);
    std::string raw_parameter;
    while (std::getline(parameters, raw_parameter, ',')) {
        const std::string parameter = trim_copy(std::move(raw_parameter));
        std::size_t type_end = 0U;
        while (type_end < parameter.size() &&
               std::isspace(static_cast<unsigned char>(parameter[type_end])) == 0 &&
               parameter[type_end] != '@' &&
               parameter[type_end] != '(') {
            ++type_end;
        }
        if (normalize_identifier(parameter.substr(0U, type_end)) == normalized_requested_type) {
            return true;
        }
    }
    return false;
}

std::size_t declared_dll_x86_stdcall_stack_bytes(const std::string& parameter_types) {
    std::size_t stack_bytes = 0U;
    std::istringstream parameters(parameter_types);
    std::string raw_parameter;
    while (std::getline(parameters, raw_parameter, ',')) {
        const std::string parameter = trim_copy(std::move(raw_parameter));
        if (parameter.empty()) {
            continue;
        }

        std::size_t type_end = 0U;
        while (type_end < parameter.size() &&
               std::isspace(static_cast<unsigned char>(parameter[type_end])) == 0 &&
               parameter[type_end] != '@' &&
               parameter[type_end] != '(') {
            ++type_end;
        }
        const std::string type_name = normalize_identifier(parameter.substr(0U, type_end));
        const bool by_reference = parameter.find('@') != std::string::npos;
        const bool uses_eight_bytes = !by_reference &&
                                      (type_name == "double" ||
                                       type_name == "d" ||
                                       type_name == "f" ||
                                       declared_dll_type_uses_64_bit_integer(type_name));
        stack_bytes += uses_eight_bytes ? 8U : 4U;
    }
    return stack_bytes;
}

std::string normalize_memory_variable_identifier(std::string value) {
    std::string normalized = normalize_identifier(std::move(value));
    if (starts_with_insensitive(normalized, "m.")) {
        normalized = normalized.substr(2U);
    }
    return normalized;
}

std::string normalize_path(const std::string& value) {
    if (value.empty()) {
        return {};
    }
    return copperfin::platform::path_to_utf8_string(
        copperfin::platform::path_from_utf8_string(value).lexically_normal());
}

bool paths_equal_for_platform(const std::string& left, const std::string& right) {
#if defined(_WIN32)
    return paths_equal_insensitive(left, right);
#else
    return normalize_path(left) == normalize_path(right);
#endif
}

bool is_index_file_path(const std::string& value) {
    const std::string extension = lowercase_copy(copperfin::platform::path_to_utf8_string(
        copperfin::platform::path_from_utf8_string(trim_copy(value)).extension()));
    return extension == ".cdx" || extension == ".dcx" || extension == ".idx" ||
           extension == ".ndx" || extension == ".mdx";
}

std::string unquote_string(std::string value) {
    value = trim_copy(std::move(value));
    if (value.size() >= 2U && value.front() == '\'' && value.back() == '\'') {
        return value.substr(1U, value.size() - 2U);
    }
    return value;
}

std::string take_first_token(std::string value) {
    value = trim_copy(std::move(value));
    if (value.empty()) {
        return value;
    }
    if (value.front() == '\'') {
        const auto closing = value.find('\'', 1U);
        if (closing != std::string::npos) {
            return value.substr(0U, closing + 1U);
        }
        return value;
    }

    const auto separator = value.find(' ');
    return separator == std::string::npos ? value : value.substr(0U, separator);
}

std::string take_first_asset_path_token(std::string value) {
    value = trim_copy(std::move(value));
    if (!value.empty() && (value.front() == '"' || value.front() == '[')) {
        const char closing = value.front() == '[' ? ']' : '"';
        const auto end = value.find(closing, 1U);
        return end == std::string::npos ? value : value.substr(0U, end + 1U);
    }
    return take_first_token(std::move(value));
}

// #6557 review: like take_first_token(), but keeps a bracket literal
// ([...]) or a parenthesized expression ((...), balanced across nested
// parens and any '...'/"..." literals inside it) intact even when it
// contains spaces, instead of truncating at the first one. Used for
// command targets (USE) that accept a quoted/bracketed literal or a
// parenthesized expression alongside a bare, unquoted filename.
std::string take_first_command_target_token(std::string value) {
    value = trim_copy(std::move(value));
    if (value.empty()) {
        return value;
    }
    if (value.front() == '[') {
        const auto closing = value.find(']', 1U);
        return closing == std::string::npos ? value : value.substr(0U, closing + 1U);
    }
    if (value.front() == '(') {
        std::size_t depth = 0U;
        for (std::size_t index = 0U; index < value.size(); ++index) {
            const char ch = value[index];
            if (ch == '\'' || ch == '"') {
                const auto closing = value.find(ch, index + 1U);
                if (closing == std::string::npos) {
                    return value;
                }
                index = closing;
                continue;
            }
            if (ch == '(') {
                ++depth;
                continue;
            }
            if (ch == ')') {
                --depth;
                if (depth == 0U) {
                    return value.substr(0U, index + 1U);
                }
            }
        }
        return value;
    }
    return take_first_token(std::move(value));
}

std::string unquote_asset_path_token(std::string value) {
    value = trim_copy(std::move(value));
    if (value.size() >= 2U &&
        ((value.front() == '"' && value.back() == '"') ||
         (value.front() == '[' && value.back() == ']'))) {
        return value.substr(1U, value.size() - 2U);
    }
    return unquote_string(std::move(value));
}

std::pair<std::string, std::string> split_first_word(std::string value) {
    value = trim_copy(std::move(value));
    if (value.empty()) {
        return {};
    }

    const auto separator = std::find_if(
        value.begin(),
        value.end(),
        [](const unsigned char character) {
            return std::isspace(character) != 0;
        });
    if (separator == value.end()) {
        return {value, {}};
    }

    return {
        value.substr(0U, static_cast<std::size_t>(separator - value.begin())),
        trim_copy(value.substr(static_cast<std::size_t>(separator - value.begin()) + 1U))
    };
}

std::string take_keyword_value(const std::string& text, const std::string& keyword) {
    const std::string upper = uppercase_copy(text);
    const std::string pattern = " " + uppercase_copy(keyword) + " ";
    const auto position = upper.find(pattern);
    if (position == std::string::npos) {
        return {};
    }
    return take_first_token(text.substr(position + pattern.size()));
}

std::string runtime_error_parameter(const std::string& message) {
    const std::size_t quoted_start = message.find('\'');
    if (quoted_start != std::string::npos) {
        const std::size_t quoted_end = message.find('\'', quoted_start + 1U);
        if (quoted_end != std::string::npos && quoted_end > quoted_start + 1U) {
            return message.substr(quoted_start + 1U, quoted_end - quoted_start - 1U);
        }
    }
    const std::size_t colon = message.rfind(':');
    if (colon != std::string::npos && colon + 1U < message.size()) {
        return trim_copy(message.substr(colon + 1U));
    }
    return message;
}

std::string uppercase_copy(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::toupper(ch));
    });
    return value;
}

bool is_bare_identifier_text(const std::string& value) {
    return !value.empty() && std::all_of(value.begin(), value.end(), [](unsigned char ch) {
        return std::isalnum(ch) != 0 || ch == '_';
    });
}

bool is_memory_variable_reference_text(const std::string& value) {
    const std::string identifier =
        value.size() > 2U && starts_with_insensitive(value, "m.")
            ? value.substr(2U)
            : value;
    return is_bare_identifier_text(identifier) &&
           (std::isalpha(static_cast<unsigned char>(identifier.front())) != 0 ||
            identifier.front() == '_');
}

std::string collapse_identifier(const std::string& value) {
    std::string normalized;
    normalized.reserve(value.size());

    for (char ch : value) {
        const auto raw = static_cast<unsigned char>(ch);
        if (raw >= static_cast<unsigned char>('a') && raw <= static_cast<unsigned char>('z')) {
            normalized.push_back(static_cast<char>(raw - ('a' - 'A')));
        } else if ((raw >= static_cast<unsigned char>('A') && raw <= static_cast<unsigned char>('Z')) ||
                   (raw >= static_cast<unsigned char>('0') && raw <= static_cast<unsigned char>('9')) ||
                   raw >= 0x80U) {
            normalized.push_back(ch);
        }
    }

    return normalized;
}

std::string unquote_identifier(std::string value) {
    value = trim_copy(std::move(value));
    if (value.size() >= 2U) {
        if ((value.front() == '\'' && value.back() == '\'') ||
            (value.front() == '"' && value.back() == '"')) {
            return value.substr(1U, value.size() - 2U);
        }
    }
    return value;
}

std::string normalize_index_value(std::string value) {
    value = trim_copy(std::move(value));
    if (value == ".T.") {
        return "true";
    }
    if (value == ".F.") {
        return "false";
    }
    return value;
}

std::optional<double> try_parse_invariant_double(std::string_view value, const bool allow_nonfinite) {
    return copperfin::platform::try_parse_invariant_double(value, allow_nonfinite);
}

std::optional<std::int64_t> try_parse_invariant_currency(std::string_view value) {
    const std::string text = trim_copy(std::string(value));
    if (text.empty()) {
        return std::nullopt;
    }

    std::size_t position = 0U;
    bool negative = false;
    if (text[position] == '+' || text[position] == '-') {
        negative = text[position] == '-';
        ++position;
    }
    if (position == text.size()) {
        return std::nullopt;
    }

    std::string digits;
    digits.reserve(text.size());
    std::size_t fractional_digits = 0U;
    bool saw_digit = false;
    while (position < text.size() && std::isdigit(static_cast<unsigned char>(text[position])) != 0) {
        digits.push_back(text[position++]);
        saw_digit = true;
    }
    if (position < text.size() && text[position] == '.') {
        ++position;
        while (position < text.size() && std::isdigit(static_cast<unsigned char>(text[position])) != 0) {
            digits.push_back(text[position++]);
            ++fractional_digits;
            saw_digit = true;
        }
    }
    if (!saw_digit) {
        return std::nullopt;
    }

    std::int64_t exponent = 0;
    if (position < text.size() && (text[position] == 'e' || text[position] == 'E')) {
        ++position;
        bool exponent_negative = false;
        if (position < text.size() && (text[position] == '+' || text[position] == '-')) {
            exponent_negative = text[position] == '-';
            ++position;
        }
        if (position == text.size() || std::isdigit(static_cast<unsigned char>(text[position])) == 0) {
            return std::nullopt;
        }

        constexpr std::int64_t exponent_limit = 1000000;
        std::int64_t exponent_value = 0;
        while (position < text.size() && std::isdigit(static_cast<unsigned char>(text[position])) != 0) {
            const std::int64_t digit = static_cast<std::int64_t>(text[position++] - '0');
            if (exponent_value < exponent_limit) {
                exponent_value = std::min(exponent_limit, exponent_value * 10 + digit);
            }
        }
        exponent = exponent_negative ? -exponent_value : exponent_value;
    }
    if (position != text.size()) {
        return std::nullopt;
    }

    const std::size_t first_nonzero = digits.find_first_not_of('0');
    if (first_nonzero == std::string::npos) {
        return std::int64_t{0};
    }
    digits.erase(0U, first_nonzero);

    const std::int64_t scale = 4 + exponent - static_cast<std::int64_t>(fractional_digits);
    const std::uint64_t magnitude_limit = negative
                                               ? static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max()) + 1U
                                               : static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max());
    std::string integer_digits;
    bool round_up = false;
    if (scale >= 0) {
        if (digits.size() > 19U || scale > static_cast<std::int64_t>(19U - digits.size())) {
            return std::nullopt;
        }
        integer_digits = digits;
        integer_digits.append(static_cast<std::size_t>(scale), '0');
    } else {
        const std::int64_t discarded_count = -scale;
        const std::size_t drop = discarded_count >= static_cast<std::int64_t>(digits.size())
                                     ? digits.size()
                                     : static_cast<std::size_t>(discarded_count);
        const std::size_t kept_count = digits.size() - drop;
        integer_digits = digits.substr(0U, kept_count);
        if (drop > 0U && discarded_count <= static_cast<std::int64_t>(digits.size())) {
            round_up = digits[kept_count] >= '5';
        }
    }

    if (integer_digits.size() > 19U) {
        return std::nullopt;
    }
    std::uint64_t magnitude = 0U;
    for (const char digit : integer_digits) {
        magnitude = magnitude * 10U + static_cast<std::uint64_t>(digit - '0');
    }
    if (round_up) {
        if (magnitude == std::numeric_limits<std::uint64_t>::max()) {
            return std::nullopt;
        }
        ++magnitude;
    }
    if (magnitude > magnitude_limit) {
        return std::nullopt;
    }
    if (negative) {
        if (magnitude == magnitude_limit) {
            return std::numeric_limits<std::int64_t>::min();
        }
        return -static_cast<std::int64_t>(magnitude);
    }
    return static_cast<std::int64_t>(magnitude);
}

std::optional<double> try_parse_numeric_index_value(const std::string& value) {
    const std::string trimmed = trim_copy(value);
    if (trimmed.empty()) {
        return std::nullopt;
    }
    return try_parse_invariant_double(trimmed);
}

int compare_index_keys(
    const std::string& left,
    const std::string& right,
    const std::string& key_domain_hint) {
    if (key_domain_hint == "numeric_or_date") {
        const auto left_numeric = try_parse_numeric_index_value(left);
        const auto right_numeric = try_parse_numeric_index_value(right);
        if (left_numeric.has_value() && right_numeric.has_value()) {
            if (*left_numeric < *right_numeric) {
                return -1;
            }
            if (*left_numeric > *right_numeric) {
                return 1;
            }
            return 0;
        }
    }

    if (left < right) {
        return -1;
    }
    if (left > right) {
        return 1;
    }
    return 0;
}

std::optional<std::string> record_field_value(const vfp::DbfRecord& record, const std::string& field_name) {
    const std::string normalized_field = collapse_identifier(field_name);
    for (const auto& value : record.values) {
        if (collapse_identifier(value.field_name) == normalized_field) {
            return value.display_value;
        }
    }
    return std::nullopt;
}

std::string evaluate_index_expression(const std::string& expression, const vfp::DbfRecord& record) {
    const std::string trimmed = trim_copy(expression);
    const std::string upper = uppercase_copy(trimmed);

    const auto split_top_level = [](const std::string& value, char delimiter) {
        std::vector<std::string> parts;
        std::string current;
        current.reserve(value.size());
        int depth = 0;
        char quote = '\0';
        for (char ch : value) {
            if (quote != '\0') {
                current.push_back(ch);
                if (ch == quote) {
                    quote = '\0';
                }
                continue;
            }
            if (ch == '\'' || ch == '"') {
                quote = ch;
                current.push_back(ch);
                continue;
            }
            if (ch == '(') {
                ++depth;
                current.push_back(ch);
                continue;
            }
            if (ch == ')' && depth > 0) {
                --depth;
                current.push_back(ch);
                continue;
            }
            if (ch == delimiter && depth == 0) {
                parts.push_back(trim_copy(current));
                current.clear();
                continue;
            }
            current.push_back(ch);
        }
        parts.push_back(trim_copy(current));
        return parts;
    };
    const auto split_top_level_plus = [&](const std::string& value) {
        return split_top_level(value, '+');
    };
    const auto parse_function_arguments = [&](const std::string& value, const std::string& prefix) -> std::optional<std::vector<std::string>> {
        const std::string candidate = trim_copy(value);
        if (!starts_with_insensitive(candidate, prefix + "(") || candidate.back() != ')') {
            return std::nullopt;
        }

        std::vector<std::string> arguments = split_top_level(
            trim_copy(candidate.substr(prefix.size() + 1U, candidate.size() - prefix.size() - 2U)),
            ',');
        for (std::string& argument : arguments) {
            argument = trim_copy(std::move(argument));
        }
        return arguments;
    };

    std::function<bool(const std::string&)> is_concat_safe = [&](const std::string& candidate) {
        const std::string part = trim_copy(candidate);
        if (part.empty()) {
            return false;
        }

        const std::vector<std::string> nested_parts = split_top_level_plus(part);
        if (nested_parts.size() > 1U) {
            return std::all_of(nested_parts.begin(), nested_parts.end(), is_concat_safe);
        }

        if (part.size() >= 2U &&
            ((part.front() == '\'' && part.back() == '\'') ||
             (part.front() == '"' && part.back() == '"'))) {
            return true;
        }
        if (record_field_value(record, part).has_value()) {
            return true;
        }
        if (try_parse_numeric_index_value(part).has_value()) {
            return false;
        }

        const auto is_supported_unary = [&](const std::string& prefix) {
            if (!starts_with_insensitive(part, prefix + "(") || part.back() != ')') {
                return false;
            }
            const std::string inner = trim_copy(part.substr(prefix.size() + 1U, part.size() - prefix.size() - 2U));
            return is_concat_safe(inner);
        };
        const auto is_supported_binary_with_count = [&](const std::string& prefix) {
            const auto arguments = parse_function_arguments(part, prefix);
            if (!arguments.has_value() || arguments->size() != 2U) {
                return false;
            }
            if (!is_concat_safe((*arguments)[0])) {
                return false;
            }
            return try_parse_numeric_index_value(evaluate_index_expression((*arguments)[1], record)).has_value();
        };
        const auto is_supported_substr = [&]() {
            const auto arguments = parse_function_arguments(part, "SUBSTR");
            if (!arguments.has_value() || (arguments->size() != 2U && arguments->size() != 3U)) {
                return false;
            }
            if (!is_concat_safe((*arguments)[0])) {
                return false;
            }
            if (!try_parse_numeric_index_value(evaluate_index_expression((*arguments)[1], record)).has_value()) {
                return false;
            }
            return arguments->size() == 2U ||
                try_parse_numeric_index_value(evaluate_index_expression((*arguments)[2], record)).has_value();
        };
        const auto is_supported_pad = [&](const std::string& prefix) {
            const auto arguments = parse_function_arguments(part, prefix);
            if (!arguments.has_value() || (arguments->size() != 2U && arguments->size() != 3U)) {
                return false;
            }
            if (!is_concat_safe((*arguments)[0])) {
                return false;
            }
            if (!try_parse_numeric_index_value(evaluate_index_expression((*arguments)[1], record)).has_value()) {
                return false;
            }
            return arguments->size() == 2U || is_concat_safe((*arguments)[2]);
        };
        const auto is_supported_str = [&]() {
            const auto arguments = parse_function_arguments(part, "STR");
            if (!arguments.has_value() || arguments->empty() || arguments->size() > 3U) {
                return false;
            }

            if (!try_parse_numeric_index_value(evaluate_index_expression((*arguments)[0], record)).has_value()) {
                return false;
            }
            if (arguments->size() >= 2U &&
                !try_parse_numeric_index_value(evaluate_index_expression((*arguments)[1], record)).has_value()) {
                return false;
            }
            return arguments->size() <= 2U ||
                try_parse_numeric_index_value(evaluate_index_expression((*arguments)[2], record)).has_value();
        };

        return is_supported_unary("UPPER") ||
            is_supported_unary("LOWER") ||
            is_supported_unary("ALLTRIM") ||
            is_supported_unary("LTRIM") ||
            is_supported_unary("RTRIM") ||
            is_supported_binary_with_count("LEFT") ||
            is_supported_binary_with_count("RIGHT") ||
            is_supported_substr() ||
            is_supported_pad("PADL") ||
            is_supported_pad("PADR") ||
            is_supported_str();
    };

    const auto apply_unary = [&](const std::string& prefix, auto&& transform) -> std::optional<std::string> {
        if (!starts_with_insensitive(trimmed, prefix + "(") || trimmed.back() != ')') {
            return std::nullopt;
        }

        const std::string inner = trim_copy(trimmed.substr(prefix.size() + 1U, trimmed.size() - prefix.size() - 2U));
        std::string value = evaluate_index_expression(inner, record);
        transform(value);
        return value;
    };

    if (const auto upper_value = apply_unary("UPPER", [](std::string& value) {
            value = uppercase_copy(value);
        })) {
        return *upper_value;
    }
    if (const auto lower_value = apply_unary("LOWER", [](std::string& value) {
            std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
                return static_cast<char>(std::tolower(ch));
            });
        })) {
        return *lower_value;
    }
    if (const auto trim_value = apply_unary("ALLTRIM", [](std::string& value) {
            value = trim_space_copy(std::move(value));
        })) {
        return *trim_value;
    }
    if (const auto ltrim_value = apply_unary("LTRIM", [](std::string& value) {
            value = ltrim_space_copy(std::move(value));
        })) {
        return *ltrim_value;
    }
    if (const auto rtrim_value = apply_unary("RTRIM", [](std::string& value) {
            value = rtrim_space_copy(std::move(value));
        })) {
        return *rtrim_value;
    }
    if (const auto left_args = parse_function_arguments(trimmed, "LEFT")) {
        if (left_args->size() == 2U) {
            std::string value = evaluate_index_expression((*left_args)[0], record);
            const auto count = try_parse_numeric_index_value(evaluate_index_expression((*left_args)[1], record));
            if (count.has_value()) {
                const long long requested = static_cast<long long>(std::llround(*count));
                if (requested <= 0LL) {
                    return {};
                }
                const std::size_t length = static_cast<std::size_t>(requested);
                if (value.size() > length) {
                    value.resize(length);
                }
                return value;
            }
        }
    }
    if (const auto right_args = parse_function_arguments(trimmed, "RIGHT")) {
        if (right_args->size() == 2U) {
            std::string value = evaluate_index_expression((*right_args)[0], record);
            const auto count = try_parse_numeric_index_value(evaluate_index_expression((*right_args)[1], record));
            if (count.has_value()) {
                const long long requested = static_cast<long long>(std::llround(*count));
                if (requested <= 0LL) {
                    return {};
                }
                const std::size_t length = std::min<std::size_t>(value.size(), static_cast<std::size_t>(requested));
                if (length < value.size()) {
                    value = value.substr(value.size() - length);
                }
                return value;
            }
        }
    }
    if (const auto substr_args = parse_function_arguments(trimmed, "SUBSTR")) {
        if (substr_args->size() == 2U || substr_args->size() == 3U) {
            std::string value = evaluate_index_expression((*substr_args)[0], record);
            const auto start = try_parse_numeric_index_value(evaluate_index_expression((*substr_args)[1], record));
            if (start.has_value()) {
                const long long requested_start = static_cast<long long>(std::llround(*start));
                if (requested_start <= 0LL) {
                    return {};
                }
                const std::size_t start_index = static_cast<std::size_t>(requested_start - 1LL);
                if (start_index >= value.size()) {
                    return {};
                }
                if (substr_args->size() == 2U) {
                    return value.substr(start_index);
                }
                const auto length = try_parse_numeric_index_value(evaluate_index_expression((*substr_args)[2], record));
                if (length.has_value()) {
                    const long long requested_length = static_cast<long long>(std::llround(*length));
                    if (requested_length <= 0LL) {
                        return {};
                    }
                    return value.substr(start_index, static_cast<std::size_t>(requested_length));
                }
            }
        }
    }
    const auto apply_pad = [&](const std::string& prefix, bool left_pad) -> std::optional<std::string> {
        const auto pad_args = parse_function_arguments(trimmed, prefix);
        if (!pad_args.has_value() || (pad_args->size() != 2U && pad_args->size() != 3U)) {
            return std::nullopt;
        }

        std::string value = evaluate_index_expression((*pad_args)[0], record);
        const auto length = try_parse_numeric_index_value(evaluate_index_expression((*pad_args)[1], record));
        if (!length.has_value()) {
            return std::nullopt;
        }

        const long long requested = static_cast<long long>(std::llround(*length));
        if (requested <= 0LL) {
            return std::string{};
        }

        const std::size_t target_length = static_cast<std::size_t>(requested);
        if (value.size() > target_length) {
            // #5948: real VFP9 truncates an over-length source to its
            // leftmost target_length characters for both PADL() and
            // PADR(), regardless of which side each function pads on when
            // the source is too short (confirmed against actual VFP9
            // output: PADL('abcdef',3) is "abc", not "def" -- see the
            // same fix and evidence in evaluate_string_function()'s own
            // PADL/PADR/PADC handling in prg_engine_string_functions.cpp).
            // The prior left_pad branch here kept the *rightmost*
            // target_length characters, a plausible-seeming but wrong
            // mirror of PADL's own padding side.
            value.resize(target_length);
            return value;
        }

        char pad_character = ' ';
        if (pad_args->size() == 3U) {
            const std::string pad_text = evaluate_index_expression((*pad_args)[2], record);
            if (!pad_text.empty()) {
                pad_character = pad_text.front();
            }
        }

        if (value.size() < target_length) {
            const std::size_t padding = target_length - value.size();
            if (left_pad) {
                value.insert(value.begin(), padding, pad_character);
            } else {
                value.append(padding, pad_character);
            }
        }

        return value;
    };
    if (const auto padl_value = apply_pad("PADL", true)) {
        return *padl_value;
    }
    if (const auto padr_value = apply_pad("PADR", false)) {
        return *padr_value;
    }
    if (const auto str_args = parse_function_arguments(trimmed, "STR")) {
        if (!str_args->empty() && str_args->size() <= 3U) {
            const auto numeric = try_parse_numeric_index_value(evaluate_index_expression((*str_args)[0], record));
            if (numeric.has_value()) {
                long long requested_width = 10LL;
                if (str_args->size() >= 2U) {
                    const auto width = try_parse_numeric_index_value(evaluate_index_expression((*str_args)[1], record));
                    if (!width.has_value()) {
                        return {};
                    }
                    requested_width = static_cast<long long>(std::llround(*width));
                }
                if (requested_width <= 0LL) {
                    return {};
                }

                int decimals = 0;
                if (str_args->size() == 3U) {
                    const auto requested_decimals =
                        try_parse_numeric_index_value(evaluate_index_expression((*str_args)[2], record));
                    if (!requested_decimals.has_value()) {
                        return {};
                    }
                    decimals = static_cast<int>(std::max(0LL, static_cast<long long>(std::llround(*requested_decimals))));
                }

                std::ostringstream formatted;
                formatted.imbue(std::locale::classic());
                if (decimals > 0) {
                    formatted << std::fixed << std::setprecision(decimals) << *numeric;
                } else {
                    formatted << std::llround(*numeric);
                }

                std::string value = formatted.str();
                const std::size_t target_width = static_cast<std::size_t>(requested_width);
                if (value.size() > target_width) {
                    return std::string(target_width, '*');
                }
                if (value.size() < target_width) {
                    value.insert(value.begin(), target_width - value.size(), ' ');
                }
                return value;
            }
        }
    }

    const std::vector<std::string> concat_parts = split_top_level_plus(trimmed);
    if (concat_parts.size() > 1U &&
        std::all_of(concat_parts.begin(), concat_parts.end(), is_concat_safe)) {
        std::string combined;
        for (const auto& part : concat_parts) {
            combined += evaluate_index_expression(part, record);
        }
        return combined;
    }

    if (trimmed.size() >= 2U &&
        ((trimmed.front() == '\'' && trimmed.back() == '\'') ||
         (trimmed.front() == '"' && trimmed.back() == '"'))) {
        return unquote_identifier(trimmed);
    }

    if (const auto field_value = record_field_value(record, trimmed)) {
        return normalize_index_value(*field_value);
    }

    return normalize_index_value(trimmed);
}

bool has_keyword(const std::string& text, const std::string& keyword) {
    const std::string upper = " " + uppercase_copy(text) + " ";
    const std::string pattern = " " + uppercase_copy(keyword) + " ";
    return upper.find(pattern) != std::string::npos;
}

bool parse_object_handle_reference(const PrgValue& value, int& handle, std::string& prog_id) {
    if (value.kind != PrgValueKind::string || !value.is_object_reference) {
        return false;
    }

    const std::string prefix = "object:";
    if (value.string_value.rfind(prefix, 0) != 0) {
        return false;
    }

    const auto separator = value.string_value.rfind('#');
    if (separator == std::string::npos || separator <= prefix.size()) {
        return false;
    }

    prog_id = value.string_value.substr(prefix.size(), separator - prefix.size());
    const auto parsed_handle = copperfin::platform::try_parse_invariant_integer<int>(
        std::string_view(value.string_value).substr(separator + 1U));
    if (!parsed_handle.has_value() || *parsed_handle <= 0) {
        return false;
    }
    handle = *parsed_handle;
    return true;
}

PrgValue make_empty_value() {
    return {};
}

PrgOperandClass classify_operand(const PrgValue& value) {
    switch (value.kind) {
        case PrgValueKind::string:
            if (value.is_object_reference) {
                // A runtime object is held as a string token; it is an Object to every operator.
                return PrgOperandClass::object;
            }
            return value.string_flavor == PrgStringFlavor::date
                       ? PrgOperandClass::date
                       : (value.string_flavor == PrgStringFlavor::datetime ? PrgOperandClass::datetime
                                                                           : PrgOperandClass::character);
        case PrgValueKind::number:
        case PrgValueKind::int64:
        case PrgValueKind::uint64:
            return PrgOperandClass::numeric;
        case PrgValueKind::currency:
            return PrgOperandClass::currency;
        case PrgValueKind::boolean:
            return PrgOperandClass::logical;
        default:
            return PrgOperandClass::empty;
    }
}

std::string format_round_trip_decimal(const double value) {
    if (!std::isfinite(value)) {
        return std::isnan(value) ? "nan" : (value < 0.0 ? "-inf" : "inf");
    }
    if (value == 0.0) {
        return "0";
    }
    // Locale-independent in both directions: the classic locale always writes '.', and the invariant parser
    // reads it back (snprintf/strtod follow the process LC_NUMERIC and would write a comma under de-DE).
    std::string scientific;
    for (int precision = 15; precision <= 17; ++precision) {
        std::ostringstream stream;
        stream.imbue(std::locale::classic());
        stream << std::scientific << std::setprecision(precision - 1) << value;
        scientific = stream.str();
        const auto parsed = try_parse_invariant_double(scientific);
        if (parsed.has_value() && *parsed == value) {
            break;
        }
    }
    // scientific is [-]d.ddddde[+-]xx: rebuild it as plain decimal.
    const bool negative = scientific.front() == '-';
    const std::size_t exponent_at = scientific.find('e');
    std::string digits;
    for (std::size_t index = negative ? 1U : 0U; index < exponent_at; ++index) {
        if (scientific[index] != '.') {
            digits.push_back(scientific[index]);
        }
    }
    while (digits.size() > 1U && digits.back() == '0') {
        digits.pop_back();
    }
    const int exponent = std::atoi(scientific.c_str() + exponent_at + 1U);
    std::string text;
    if (exponent >= 0) {
        const std::size_t integer_digits = static_cast<std::size_t>(exponent) + 1U;
        if (digits.size() <= integer_digits) {
            text = digits + std::string(integer_digits - digits.size(), '0');
        } else {
            text = digits.substr(0U, integer_digits) + "." + digits.substr(integer_digits);
        }
    } else {
        text = "0." + std::string(static_cast<std::size_t>(-exponent - 1), '0') + digits;
    }
    return negative ? "-" + text : text;
}

bool numeric_values_equal(const double left, const double right) {
    if (left == right) {
        return true;
    }
    if (!std::isfinite(left) || !std::isfinite(right)) {
        return false;
    }
    const double magnitude = std::max(std::abs(left), std::abs(right));
    return std::abs(left - right) <= std::numeric_limits<double>::epsilon() * magnitude;
}

namespace {
// #6035: sign and magnitude of an exact 64-bit integer; zero is never negative.
struct ExactInteger {
    bool negative = false;
    std::uint64_t magnitude = 0U;
};

// Exactly representable double bounds of the exact-integer range [-2^63, 2^64), and the magnitude of INT64_MIN.
constexpr double kNegTwoPow63 = -9223372036854775808.0;
constexpr double kTwoPow64 = 18446744073709551616.0;
constexpr std::uint64_t kInt64MinMagnitude = 9223372036854775808ULL;

bool is_wide_integer_kind(const PrgValue& value) {
    return value.kind == PrgValueKind::int64 || value.kind == PrgValueKind::uint64;
}

std::optional<ExactInteger> exact_integer_operand(const PrgValue& value) {
    if (value.kind == PrgValueKind::int64) {
        const std::int64_t signed_value = value.int64_value;
        if (signed_value < 0) {
            return ExactInteger{true, std::uint64_t{0} - static_cast<std::uint64_t>(signed_value)};
        }
        return ExactInteger{false, static_cast<std::uint64_t>(signed_value)};
    }
    if (value.kind == PrgValueKind::uint64) {
        return ExactInteger{false, value.uint64_value};
    }
    if (value.kind != PrgValueKind::number) {
        return std::nullopt;
    }
    const double number = value.number_value;
    if (!std::isfinite(number) || std::trunc(number) != number) {
        return std::nullopt;
    }
    if (number >= kTwoPow64 || number < kNegTwoPow63) {
        return std::nullopt;
    }
    if (number < 0.0) {
        return ExactInteger{true, static_cast<std::uint64_t>(-number)};
    }
    return ExactInteger{false, static_cast<std::uint64_t>(number)};
}

bool exact_integer_operands(const PrgValue& left, const PrgValue& right, ExactInteger& exact_left, ExactInteger& exact_right) {
    if (!is_wide_integer_kind(left) && !is_wide_integer_kind(right)) {
        return false;
    }
    const auto first = exact_integer_operand(left);
    const auto second = exact_integer_operand(right);
    if (!first.has_value() || !second.has_value()) {
        return false;
    }
    exact_left = *first;
    exact_right = *second;
    return true;
}

[[noreturn]] void throw_exact_integer_overflow() {
    throw PrgCompatibilityError(runtime_text("Runtime.Prg.String.Error.NumericOverflow"), 39);
}

ExactInteger exact_integer_add(const ExactInteger& left, const ExactInteger& right) {
    if (left.negative == right.negative) {
        const std::uint64_t sum = left.magnitude + right.magnitude;
        if (sum < left.magnitude) {
            throw_exact_integer_overflow();
        }
        return ExactInteger{left.negative && sum != 0U, sum};
    }
    if (left.magnitude == right.magnitude) {
        return ExactInteger{};
    }
    return left.magnitude > right.magnitude
               ? ExactInteger{left.negative, left.magnitude - right.magnitude}
               : ExactInteger{right.negative, right.magnitude - left.magnitude};
}

ExactInteger exact_integer_multiply(const ExactInteger& left, const ExactInteger& right) {
    if (left.magnitude == 0U || right.magnitude == 0U) {
        return ExactInteger{};
    }
    if (left.magnitude > std::numeric_limits<std::uint64_t>::max() / right.magnitude) {
        throw_exact_integer_overflow();
    }
    return ExactInteger{left.negative != right.negative, left.magnitude * right.magnitude};
}

PrgValue exact_integer_result(const ExactInteger& exact, const bool allow_unsigned) {
    if (exact.negative) {
        if (exact.magnitude > kInt64MinMagnitude) {
            throw_exact_integer_overflow();
        }
        if (exact.magnitude == kInt64MinMagnitude) {
            // 2^63 has no positive int64 to negate.
            return make_int64_value(std::numeric_limits<std::int64_t>::min());
        }
        return make_int64_value(-static_cast<std::int64_t>(exact.magnitude));
    }
    if (exact.magnitude <= static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())) {
        return make_int64_value(static_cast<std::int64_t>(exact.magnitude));
    }
    if (allow_unsigned) {
        return make_uint64_value(exact.magnitude);
    }
    throw_exact_integer_overflow();
}

int compare_exact_integers(const ExactInteger& left, const ExactInteger& right) {
    if (left.negative != right.negative) {
        return left.negative ? -1 : 1;
    }
    if (left.magnitude == right.magnitude) {
        return 0;
    }
    const bool magnitude_less = left.magnitude < right.magnitude;
    return (magnitude_less != left.negative) ? -1 : 1;
}
}  // namespace

std::optional<PrgValue> try_exact_integer_arithmetic(const char operation, const PrgValue& left, const PrgValue& right) {
    ExactInteger exact_left;
    ExactInteger exact_right;
    if (!exact_integer_operands(left, right, exact_left, exact_right)) {
        return std::nullopt;
    }
    const bool allow_unsigned = left.kind == PrgValueKind::uint64 || right.kind == PrgValueKind::uint64;
    switch (operation) {
    case '+':
        return exact_integer_result(exact_integer_add(exact_left, exact_right), allow_unsigned);
    case '-': {
        ExactInteger negated = exact_right;
        negated.negative = !exact_right.negative && exact_right.magnitude != 0U;
        return exact_integer_result(exact_integer_add(exact_left, negated), allow_unsigned);
    }
    case '*':
        return exact_integer_result(exact_integer_multiply(exact_left, exact_right), allow_unsigned);
    case '/': {
        if (!is_wide_integer_kind(left) || !is_wide_integer_kind(right)) {
            return std::nullopt;
        }
        if (exact_right.magnitude == 0U) {
            throw std::runtime_error(runtime_text("Runtime.Prg.Expression.Error.IntegerDivisionByZero"));
        }
        const std::uint64_t quotient = exact_left.magnitude / exact_right.magnitude;
        return exact_integer_result(
            ExactInteger{quotient != 0U && exact_left.negative != exact_right.negative, quotient}, allow_unsigned);
    }
    default:
        return std::nullopt;
    }
}

std::optional<int> try_exact_integer_compare(const PrgValue& left, const PrgValue& right) {
    ExactInteger exact_left;
    ExactInteger exact_right;
    if (!exact_integer_operands(left, right, exact_left, exact_right)) {
        return std::nullopt;
    }
    return compare_exact_integers(exact_left, exact_right);
}

PrgValue negate_exact_integer(const PrgValue& value) {
    const auto exact = exact_integer_operand(value);
    if (!exact.has_value() || !is_wide_integer_kind(value)) {
        throw std::logic_error("negate_exact_integer requires an int64 or uint64 value");
    }
    ExactInteger negated = *exact;
    negated.negative = !exact->negative && exact->magnitude != 0U;
    // The negation of a uint64 may not become uint64: only int64 results are representable.
    return exact_integer_result(negated, false);
}

NumericBehavior numeric_behavior(const std::function<std::string(const std::string&)>& set_callback) {
    if (!set_callback) {
        return NumericBehavior::copperfin;
    }
    return normalize_identifier(set_callback("NUMERICBEHAVIOR")) == "vfp9" ? NumericBehavior::vfp9
                                                                           : NumericBehavior::copperfin;
}

std::int64_t saturating_numeric_to_int64(const double value) {
    if (std::isnan(value)) {
        return 0;
    }
    // 2^63 is exactly representable; every double at or above it is out of int64 range.
    if (value >= 9223372036854775808.0) {
        return std::numeric_limits<std::int64_t>::max();
    }
    if (value <= -9223372036854775808.0) {
        return std::numeric_limits<std::int64_t>::min();
    }
    return static_cast<std::int64_t>(value);
}

namespace {

std::int32_t signed_int32_from_low_bits(const std::uint32_t low_bits) {
    constexpr auto kSignedMax = static_cast<std::uint32_t>(std::numeric_limits<std::int32_t>::max());
    if (low_bits <= kSignedMax) {
        return static_cast<std::int32_t>(low_bits);
    }
    // UINT32_MAX - low_bits is at most INT32_MAX, so this cast and the
    // subtraction are defined on every target with exact 32-bit integers.
    return -1 - static_cast<std::int32_t>(std::numeric_limits<std::uint32_t>::max() - low_bits);
}

std::int32_t declared_int32_from_int64(const std::int64_t value) {
    const auto low_bits = static_cast<std::uint32_t>(static_cast<std::uint64_t>(value));
    return signed_int32_from_low_bits(low_bits);
}

} // namespace

std::int64_t vfp9_numeric_to_int32(const double value) {
    if (!std::isfinite(value) || value >= 9223372036854775808.0 || value < -9223372036854775808.0) {
        return 0;  // integer indefinite is 0x8000000000000000; its low 32 bits are 0
    }
    return declared_int32_from_int64(static_cast<std::int64_t>(value));
}

std::int64_t vfp9_numeric_to_int32(const PrgValue& value) {
    if (value.kind == PrgValueKind::int64) {
        return declared_int32_from_int64(value.int64_value);
    }
    if (value.kind == PrgValueKind::uint64) {
        return signed_int32_from_low_bits(static_cast<std::uint32_t>(value.uint64_value));
    }
    return vfp9_numeric_to_int32(value_as_number(value));
}

std::int64_t numeric_count_argument(const double value, const NumericBehavior behavior) {
    return behavior == NumericBehavior::vfp9 ? vfp9_numeric_to_int32(value) : saturating_numeric_to_int64(value);
}

std::optional<std::int64_t> checked_truncated_numeric_to_int64(const double value) {
    if (!std::isfinite(value) || value >= 9223372036854775808.0 || value < -9223372036854775808.0) {
        return std::nullopt;
    }
    return static_cast<std::int64_t>(value);
}

// RQ-CF-PRG-DEFINE-BAR-NUMERIC-001 (#5611/#6776). Native RELATIVE controls
// distinguish positive saturation from negative low32 aliases. Admission is
// bounded before any cast; nonfinite rejection in both modes is owner-derived.
std::optional<std::int32_t> checked_define_bar_number_argument(
    const double value, const NumericBehavior behavior) {
    if (!std::isfinite(value)) {
        return std::nullopt;
    }
    const auto converted = behavior == NumericBehavior::vfp9
        ? std::optional<std::int64_t>(value >= 4294967296.0
              ? std::numeric_limits<std::int32_t>::max() : vfp9_numeric_to_int32(value))
        : checked_truncated_numeric_to_int64(value);
    if (!converted || *converted < 1 || *converted > std::numeric_limits<std::int32_t>::max()) {
        return std::nullopt;
    }
    return static_cast<std::int32_t>(*converted);
}

// RQ-CF-PRG-ON-BAR-NUMERIC-001: independent singleton native controls recover
// the same positive conversion as DEFINE BAR, plus -1/-2 lookup sentinels.
// Do not bind a sentinel as a user bar or infer system-menu support from it.
std::optional<std::int32_t> checked_on_bar_number_argument(
    const double value, const NumericBehavior behavior) {
    if (behavior == NumericBehavior::vfp9 && std::isfinite(value) && value < 4294967296.0) {
        const auto converted = vfp9_numeric_to_int32(value);
        if (converted == -1 || converted == -2) {
            return static_cast<std::int32_t>(converted);
        }
    }
    return checked_define_bar_number_argument(value, behavior);
}

// RQ-CF-PRG-ON-SELECTION-BAR-NUMERIC-001: independent native DO/action
// singleton controls establish the same conversion, not the same error code.
std::optional<std::int32_t> checked_on_selection_bar_number_argument(
    const double value, const NumericBehavior behavior) {
    return checked_on_bar_number_argument(value, behavior);
}

// RQ-CF-PRG-SET-SKIP-BAR-NUMERIC-001: independent singleton native state
// controls recover the conversion, but -1/-2 succeeds here without user mutation.
std::optional<std::int32_t> checked_set_skip_bar_number_argument(
    const double value, const NumericBehavior behavior) {
    return checked_on_bar_number_argument(value, behavior);
}

// RQ-CF-PRG-SET-MARK-BAR-NUMERIC-001: independent paired native mark controls
// recover this conversion; command admission differs from SET SKIP.
std::optional<std::int32_t> checked_set_mark_bar_number_argument(
    const double value, const NumericBehavior behavior) {
    return checked_on_bar_number_argument(value, behavior);
}

// RQ-CF-PRG-MRKBAR-NUMERIC-001 (#5611/#6776): independent native singleton
// mark queries distinguish both-sign aliases from setter saturation.
std::optional<std::int64_t> checked_mrkbar_number_argument(
    const PrgValue& value, const NumericBehavior behavior) {
    const bool numeric = value.kind == PrgValueKind::number ||
                         value.kind == PrgValueKind::int64 || value.kind == PrgValueKind::uint64;
    if (value.kind == PrgValueKind::number && !std::isfinite(value.number_value)) {
        return std::nullopt;
    }
    std::optional<std::int64_t> bar;
    if (numeric && behavior == NumericBehavior::vfp9) {
        bar = vfp9_numeric_to_int32(value);
    } else if (value.kind == PrgValueKind::int64) {
        bar = value.int64_value;
    } else if (value.kind == PrgValueKind::uint64) {
        if (value.uint64_value <= static_cast<std::uint64_t>(INT32_MAX)) {
            bar = static_cast<std::int64_t>(value.uint64_value);
        }
    } else {
        const double raw = value_as_number(value);
        bar = checked_truncated_numeric_to_int64(numeric ? raw : std::round(raw));
    }
    if (!bar || *bar < 1 || (numeric && *bar > INT32_MAX)) {
        return std::nullopt;
    }
    return bar;
}

// RQ-CF-PRG-SKPBAR-NUMERIC-001 (#5611/#6776): independent enabled/disabled
// singleton controls match MRKBAR conversion; general native1612 gap#7098
// is separate. Parent-derived exact-integer/nonfinite/coercion bounds share
// this checked helper without migrating the first-pass lookup policy.
std::optional<std::int64_t> checked_skpbar_number_argument(
    const PrgValue& value, const NumericBehavior behavior) {
    return checked_mrkbar_number_argument(value, behavior);
}

// RQ-CF-PRG-PRMBAR-NUMERIC-001 (#5611/#6776/#7096): independently observed
// singleton prompt identities match the query conversion, not setter saturation.
// Parent-derived exact-integer/nonfinite/coercion safety shares this helper;
// general native1612 errors and GETBAR position conversion remain separate.
std::optional<std::int64_t> checked_prmbar_number_argument(
    const PrgValue& value, const NumericBehavior behavior) {
    return checked_mrkbar_number_argument(value, behavior);
}

// RQ-CF-PRG-SKIP-COUNT-NUMERIC-001 (#5611/#6776): count conversion only,
// not shared navigation or native type admission. Widened int32 counts make
// the existing long-long abs operation defined, including INT32_MIN.
std::optional<std::int32_t> checked_skip_count_argument(
    const PrgValue& value, const NumericBehavior behavior) {
    if (value.kind == PrgValueKind::number && std::isnan(value.number_value)) {
        return std::nullopt;
    }
    if (behavior == NumericBehavior::vfp9 &&
        (value.kind == PrgValueKind::number || value.kind == PrgValueKind::int64 ||
         value.kind == PrgValueKind::uint64)) {
        return static_cast<std::int32_t>(vfp9_numeric_to_int32(value));
    }
    std::optional<std::int64_t> converted;
    if (value.kind == PrgValueKind::int64) {
        converted = value.int64_value;
    } else if (value.kind == PrgValueKind::uint64) {
        if (value.uint64_value <= static_cast<std::uint64_t>(INT32_MAX)) {
            converted = static_cast<std::int64_t>(value.uint64_value);
        }
    } else if (value.kind == PrgValueKind::number) {
        converted = checked_truncated_numeric_to_int64(value.number_value);
    } else {
        converted = checked_truncated_numeric_to_int64(std::round(value_as_number(value)));
    }
    if (!converted || *converted < INT32_MIN || *converted > INT32_MAX) {
        return std::nullopt;
    }
    return static_cast<std::int32_t>(*converted);
}

// RQ-CF-PRG-SLEEP-DURATION-NUMERIC-001: derived extension, no native alias.
std::optional<std::size_t> checked_sleep_duration_argument(const PrgValue& value) {
    std::uint64_t duration = 0;
    if (value.kind == PrgValueKind::uint64) {
        duration = value.uint64_value;
    } else if (value.kind == PrgValueKind::int64) {
        if (value.int64_value < 0) return std::nullopt;
        duration = static_cast<std::uint64_t>(value.int64_value);
    } else {
        const double raw = value_as_number(value);
        if (!std::isfinite(raw) || raw < 0) return std::nullopt;
        const auto rounded = checked_truncated_numeric_to_int64(std::round(raw));
        if (!rounded || *rounded < 0) return std::nullopt;
        duration = static_cast<std::uint64_t>(*rounded);
    }
    if (duration > static_cast<std::uint64_t>(INT64_MAX) ||
        duration > std::numeric_limits<std::size_t>::max()) return std::nullopt;
    return static_cast<std::size_t>(duration);
}

// RQ-CF-PRG-UNLOCK-RECORD-NUMERIC-001 (#5611/#6776): installed native
// shared-table observations. Keep record-existence/type/lock policy separate.
std::optional<std::uint64_t> checked_unlock_record_argument(
    const PrgValue& value, const NumericBehavior behavior) {
    const bool numeric = value.kind == PrgValueKind::number ||
        value.kind == PrgValueKind::int64 || value.kind == PrgValueKind::uint64;
    if (value.kind == PrgValueKind::number && std::isnan(value.number_value)) {
        return std::nullopt;
    }
    std::optional<std::int64_t> converted;
    if (value.kind == PrgValueKind::int64) {
        converted = value.int64_value;
    } else if (value.kind == PrgValueKind::uint64) {
        if (value.uint64_value <= static_cast<std::uint64_t>(INT32_MAX)) {
            converted = static_cast<std::int64_t>(value.uint64_value);
        }
    } else {
        converted = checked_truncated_numeric_to_int64(
            numeric ? value.number_value : std::round(value_as_number(value)));
    }
    if (converted && *converted >= 0 && *converted <= INT32_MAX) {
        return static_cast<std::uint64_t>(*converted);
    }
    if (!numeric || behavior != NumericBehavior::vfp9) {
        return std::nullopt;
    }
    const bool negative = (value.kind == PrgValueKind::int64 && value.int64_value < 0) ||
        (value.kind == PrgValueKind::number && value.number_value < 0);
    if (negative) {
        const auto alias = vfp9_numeric_to_int32(value);
        return alias > 0 ? std::optional<std::uint64_t>(static_cast<std::uint64_t>(alias))
                         : std::nullopt;
    }
    // Native positive 2^31..2^32-1 rejects, but positive >=2^32 never
    // releases a wrapped small record. Retain the wide target when possible;
    // beyond the shared int64 model use zero (a non-record, never a lock ID).
    const bool below_uint32_wrap = value.kind == PrgValueKind::uint64
        ? value.uint64_value < UINT64_C(4294967296)
        : converted && *converted < INT64_C(4294967296);
    if (below_uint32_wrap) {
        return std::nullopt;
    }
    if (value.kind == PrgValueKind::uint64) {
        return value.uint64_value;
    }
    return converted ? static_cast<std::uint64_t>(*converted) : UINT64_C(0);
}

// RQ-CF-PRG-GO-RECORD-NUMERIC-001 (#5611/#6776): conversion only,
// not record existence/type admission (#7081). Preserve pending negatives.
std::optional<std::int32_t> checked_go_record_argument(
    const PrgValue& value, const NumericBehavior behavior) {
    std::optional<std::int64_t> converted;
    const bool negative_numeric =
        (value.kind == PrgValueKind::number && value.number_value < 0) ||
        (value.kind == PrgValueKind::int64 && value.int64_value < 0);
    if (value.kind == PrgValueKind::int64) {
        converted = value.int64_value;
    } else if (value.kind == PrgValueKind::uint64) {
        if (value.uint64_value <= static_cast<std::uint64_t>(INT32_MAX)) {
            converted = static_cast<std::int64_t>(value.uint64_value);
        }
    } else if (value.kind == PrgValueKind::number) {
        converted = checked_truncated_numeric_to_int64(value.number_value);
    } else {
        // Preserve checked existing half-away coercions, not native type10.
        converted = checked_truncated_numeric_to_int64(std::round(value_as_number(value)));
    }
    if (converted && *converted >= INT32_MIN && *converted <= INT32_MAX) {
        return static_cast<std::int32_t>(*converted);
    }
    if (negative_numeric && behavior == NumericBehavior::vfp9) {
        const auto alias = vfp9_numeric_to_int32(value);
        // Native buffered probes reject huge negative operands whose low32
        // would synthesize a negative pending identity. Only positive aliases.
        if (alias > 0) {
            return static_cast<std::int32_t>(alias);
        }
    }
    return std::nullopt;
}

// RQ-CF-PRG-ERROR-COMMAND-NUMERIC-001 (#5611/#6776): retained native
// evaluation-order observations in vfp9-error-command-numeric-observation.
std::optional<std::int32_t> checked_error_number_argument(
    const PrgValue& value, const NumericBehavior behavior) {
    std::optional<std::int64_t> converted;
    const bool negative_numeric =
        (value.kind == PrgValueKind::number && value.number_value < 0) ||
        (value.kind == PrgValueKind::int64 && value.int64_value < 0);
    if (negative_numeric && behavior == NumericBehavior::vfp9) {
        converted = vfp9_numeric_to_int32(value);
    } else if (value.kind == PrgValueKind::int64) {
        converted = value.int64_value;
    } else if (value.kind == PrgValueKind::uint64) {
        if (value.uint64_value <= static_cast<std::uint64_t>(INT32_MAX)) {
            converted = static_cast<std::int64_t>(value.uint64_value);
        }
    } else if (value.kind == PrgValueKind::number) {
        converted = checked_truncated_numeric_to_int64(value.number_value);
    } else if (value.kind == PrgValueKind::currency) {
        // Preserve existing Currency rounding, not native ERROR type parity.
        converted = checked_truncated_numeric_to_int64(std::round(value_as_number(value)));
    }
    if (!converted || *converted < 0 || *converted > INT32_MAX) {
        return std::nullopt;
    }
    return static_cast<std::int32_t>(*converted);
}

// RQ-CF-PRG-CURSORSETPROP-BUFFERING-NUMERIC-001 (#5611/#6776);
// native Numeric oracle and exact-extension policy retained in its fixture.
std::optional<std::int32_t> checked_cursor_buffering_mode_argument(
    const PrgValue& value, const NumericBehavior behavior) {
    const bool numeric = value.kind == PrgValueKind::number ||
        value.kind == PrgValueKind::int64 || value.kind == PrgValueKind::uint64;
    std::optional<std::int64_t> mode;
    if (numeric && behavior == NumericBehavior::vfp9) {
        mode = vfp9_numeric_to_int32(value);
    } else if (value.kind == PrgValueKind::int64) {
        mode = value.int64_value;
    } else if (value.kind == PrgValueKind::uint64) {
        if (value.uint64_value >= 1U && value.uint64_value <= 5U) {
            mode = static_cast<std::int64_t>(value.uint64_value);
        }
    } else {
        mode = checked_truncated_numeric_to_int64(numeric ? value.number_value :
            std::round(value_as_number(value)));
    }
    if (!mode || *mode < 1 || *mode > 5) {
        return std::nullopt;
    }
    return static_cast<std::int32_t>(*mode);
}

// RQ-CF-PRG-BINDEVENT-FLAGS-NUMERIC-001 (#5611/#6776); native method
// observations and derived routine-extension equivalence in the fixture.
std::optional<std::int32_t> checked_bindevent_flags_argument(
    const PrgValue& value, const NumericBehavior behavior) {
    const bool numeric = value.kind == PrgValueKind::number ||
        value.kind == PrgValueKind::int64 || value.kind == PrgValueKind::uint64;
    std::optional<std::int64_t> flags;
    if (numeric && behavior == NumericBehavior::vfp9) {
        flags = vfp9_numeric_to_int32(value);
    } else if (value.kind == PrgValueKind::int64) {
        flags = value.int64_value;
    } else if (value.kind == PrgValueKind::uint64) {
        if (value.uint64_value <= static_cast<std::uint64_t>(INT32_MAX)) {
            flags = static_cast<std::int64_t>(value.uint64_value);
        }
    } else {
        flags = checked_truncated_numeric_to_int64(numeric ? value.number_value :
            std::round(value_as_number(value)));
    }
    if (!flags || *flags > INT32_MAX || *flags < (numeric ? 0 : INT32_MIN)) {
        return std::nullopt;
    }
    // Signed32 admission above makes narrowing defined. Only supported bits
    // are exposed; no sign-dependent integral conversion is needed.
    return static_cast<std::int32_t>(*flags) & 3;
}

// RQ-CF-PRG-ALINES-FLAGS-NUMERIC-001 (#5611/#6776); native evidence retained in
// tests/fixtures/vfp9-alines-flags-numeric-observation/. Splitting is unchanged.
std::optional<std::int32_t> checked_alines_flags_argument(
    const PrgValue& value,
    const NumericBehavior behavior) {
    if (value.kind == PrgValueKind::number || value.kind == PrgValueKind::int64 ||
        value.kind == PrgValueKind::uint64) {
        std::optional<std::int64_t> flags;
        if (behavior == NumericBehavior::vfp9) {
            flags = vfp9_numeric_to_int32(value);
        } else if (value.kind == PrgValueKind::int64) {
            flags = value.int64_value;
        } else if (value.kind == PrgValueKind::uint64) {
            if (value.uint64_value <= 31U) {
                flags = static_cast<std::int64_t>(value.uint64_value);
            }
        } else {
            flags = checked_truncated_numeric_to_int64(value.number_value);
        }
        if (!flags.has_value() || *flags < 0 || *flags > 31) {
            return std::nullopt;
        }
        return static_cast<std::int32_t>(*flags);
    }
    // Preserve other existing coercions without inferring a native type contract.
    // std::round has llround's in-range rounding, without its integer-domain call.
    const auto flags = checked_truncated_numeric_to_int64(std::round(value_as_number(value)));
    if (!flags.has_value() || *flags < std::numeric_limits<std::int32_t>::min() ||
        *flags > std::numeric_limits<std::int32_t>::max()) {
        return std::nullopt;
    }
    return static_cast<std::int32_t>(*flags);
}

// RQ-CF-PRG-ADIR-DISPLAY-NUMERIC-001 (#5611/#6776); installed native evidence
// in tests/fixtures/vfp9-adir-display-numeric-observation/. Rendering is unchanged.
std::optional<std::int32_t> checked_adir_display_argument(
    const PrgValue& value,
    const NumericBehavior behavior) {
    if (value.kind == PrgValueKind::number || value.kind == PrgValueKind::int64 ||
        value.kind == PrgValueKind::uint64) {
        std::optional<std::int64_t> flag;
        if (behavior == NumericBehavior::vfp9) {
            flag = vfp9_numeric_to_int32(value);
        } else if (value.kind == PrgValueKind::int64) {
            flag = value.int64_value;
        } else if (value.kind == PrgValueKind::uint64) {
            if (value.uint64_value <= 3U) {
                flag = static_cast<std::int64_t>(value.uint64_value);
            }
        } else {
            flag = checked_truncated_numeric_to_int64(value.number_value);
        }
        if (!flag.has_value() || *flag < 0 || *flag > 3) {
            return std::nullopt;
        }
        return static_cast<std::int32_t>(*flag);
    }
    // Preserve existing coercions, not a newly recovered native type contract.
    const auto flag = checked_truncated_numeric_to_int64(std::round(value_as_number(value)));
    if (!flag.has_value() || *flag < std::numeric_limits<std::int32_t>::min() ||
        *flag > std::numeric_limits<std::int32_t>::max()) {
        return std::nullopt;
    }
    return static_cast<std::int32_t>(*flag);
}

// RQ-CF-PRG-AFONT-SIZE-NUMERIC-001 (#5611/#6776); retained native AFONT
// observations recover truncation and -1 aliases, not every converted value.
// Exact extended integers and indefinite zero follow the shared numeric model.
std::optional<std::int32_t> checked_afont_size_argument(
    const PrgValue& value,
    const NumericBehavior behavior) {
    const bool numeric = value.kind == PrgValueKind::number || value.kind == PrgValueKind::int64 ||
                         value.kind == PrgValueKind::uint64;
    if (numeric && behavior == NumericBehavior::vfp9) {
        return static_cast<std::int32_t>(vfp9_numeric_to_int32(value));
    }
    std::optional<std::int64_t> size;
    if (value.kind == PrgValueKind::int64) {
        size = value.int64_value;
    } else if (value.kind == PrgValueKind::uint64) {
        if (value.uint64_value <= static_cast<std::uint64_t>(std::numeric_limits<std::int32_t>::max())) {
            size = static_cast<std::int64_t>(value.uint64_value);
        }
    } else {
        // Keep other existing coercions' half-away rounding, with no unsafe llround.
        size = checked_truncated_numeric_to_int64(
            numeric ? value.number_value : std::round(value_as_number(value)));
    }
    if (!size.has_value() || *size < std::numeric_limits<std::int32_t>::min() ||
        *size > std::numeric_limits<std::int32_t>::max()) {
        return std::nullopt;
    }
    return static_cast<std::int32_t>(*size);
}

// RQ-CF-PRG-FIELD-INDEX-NUMERIC-001 (#5611/#6776). The retained native
// FIELD fixture distinguishes negative wrapping from positive empty results.
std::optional<std::size_t> checked_field_index_argument(
    const PrgValue& value,
    const NumericBehavior behavior) {
    const bool numeric = value.kind == PrgValueKind::number || value.kind == PrgValueKind::int64 ||
                         value.kind == PrgValueKind::uint64;
    if (numeric && behavior == NumericBehavior::vfp9) {
        if (value.kind == PrgValueKind::number && std::isnan(value.number_value)) {
            return std::nullopt;
        }
        constexpr auto ceiling = std::numeric_limits<std::int32_t>::max();
        const bool oversized_positive = value.kind == PrgValueKind::int64
            ? value.int64_value > ceiling
            : value.kind == PrgValueKind::uint64
                ? value.uint64_value > static_cast<std::uint64_t>(ceiling)
                : value.number_value > static_cast<double>(ceiling);
        // Zero is an empty-lookup sentinel, not a recovered exact native index.
        if (oversized_positive) {
            return 0U;
        }
        return static_cast<std::size_t>(std::max<std::int64_t>(0, vfp9_numeric_to_int32(value)));
    }
    const auto index = numeric ? checked_declared_int64_argument(value)
        : checked_truncated_numeric_to_int64(std::round(value_as_number(value)));
    if (!index.has_value()) {
        return std::nullopt;
    }
    if (*index <= 0) {
        return 0U;
    }
    if (static_cast<std::uint64_t>(*index) > std::numeric_limits<std::size_t>::max()) {
        return std::nullopt;
    }
    return static_cast<std::size_t>(*index);
}

// RQ-CF-PRG-FSIZE-INDEX-NUMERIC-001 (#5611/#6776): FSIZE's installed
// Numeric type rejection is consistent, not FIELD's switchable index quirk.
std::optional<std::size_t> checked_fsize_index_argument(const PrgValue& value) {
    if (value.kind == PrgValueKind::number || value.kind == PrgValueKind::int64 ||
        value.kind == PrgValueKind::uint64) {
        return std::nullopt;
    }
    return checked_field_index_argument(value, NumericBehavior::copperfin);
}

// RQ-CF-PRG-SELECT-SELECTOR-NUMERIC-001 (#5611/#6776); query routing
// remains separate (#6013). No unchecked integral conversion reaches it.
std::optional<std::int32_t> checked_select_selector_argument(
    const PrgValue& value,
    const NumericBehavior behavior) {
    const bool numeric = value.kind == PrgValueKind::number || value.kind == PrgValueKind::int64 ||
                         value.kind == PrgValueKind::uint64;
    if (value.kind == PrgValueKind::number && std::isnan(value.number_value)) {
        return std::nullopt; // derived safety boundary, not a native NaN claim
    }
    const auto selector = checked_afont_size_argument(value, behavior);
    if (!selector.has_value() || (numeric && (*selector < 0 || *selector > 32767))) {
        return std::nullopt;
    }
    return selector;
}

// RQ-CF-PRG-SQLGETPROP-HANDLE-NUMERIC-001 (#5611/#6776): installed
// connection-free queries distinguish negative-only zero aliases.
std::optional<std::int32_t> checked_sqlgetprop_handle_argument(
    const PrgValue& value,
    const NumericBehavior behavior) {
    const bool negative_numeric =
        (value.kind == PrgValueKind::number && value.number_value < 0.0) ||
        (value.kind == PrgValueKind::int64 && value.int64_value < 0);
    if (negative_numeric && behavior == NumericBehavior::vfp9) {
        return vfp9_numeric_to_int32(value);
    }
    // Reuse the checked signed-int32 conversion only, not AFONT semantics.
    // Non-Numeric operands retain existing checked half-away rounding.
    return checked_afont_size_argument(value, NumericBehavior::copperfin);
}

// RQ-CF-PRG-SQLSETPROP-HANDLE-NUMERIC-001: the retained connection-free
// setter probe independently establishes the same negative-only aliases.
std::optional<std::int32_t> checked_sqlsetprop_handle_argument(
    const PrgValue& value,
    const NumericBehavior behavior) {
    return checked_sqlgetprop_handle_argument(value, behavior);
}

// RQ-CF-PRG-SQLDISCONNECT-HANDLE-NUMERIC-001: the independent connection-free
// disconnect probe establishes the same negative-only zero aliases.
std::optional<std::int32_t> checked_sqldisconnect_handle_argument(
    const PrgValue& value,
    const NumericBehavior behavior) {
    return checked_sqlgetprop_handle_argument(value, behavior);
}

// RQ-CF-PRG-SQLCANCEL-HANDLE-NUMERIC-001: derived #5611/#6776 admission
// while native connected-index conversion remains a recorded recovery gap.
// Neither mode speculates about SQLGETPROP's negative-only aliases.
std::optional<std::int32_t> checked_sqlcancel_handle_argument(
    const PrgValue& value,
    const NumericBehavior) {
    return checked_sqlgetprop_handle_argument(value, NumericBehavior::copperfin);
}

// RQ-CF-PRG-SQLCOMMIT-HANDLE-NUMERIC-001: derived #5611/#6776 safety;
// the native connection-free fixture does not establish wrapping aliases.
std::optional<std::int32_t> checked_sqlcommit_handle_argument(
    const PrgValue& value,
    const NumericBehavior) {
    return checked_sqlgetprop_handle_argument(value, NumericBehavior::copperfin);
}

// RQ-CF-PRG-SQLROLLBACK-HANDLE-NUMERIC-001: derived #5611/#6776 safety;
// the native connection-free fixture does not establish wrapping aliases.
std::optional<std::int32_t> checked_sqlrollback_handle_argument(
    const PrgValue& value,
    const NumericBehavior) {
    return checked_sqlgetprop_handle_argument(value, NumericBehavior::copperfin);
}

// RQ-CF-PRG-SQLTABLES-HANDLE-NUMERIC-001: derived #5611/#6776 safety;
// the native connection-free fixture cannot establish converted indices.
std::optional<std::int32_t> checked_sqltables_handle_argument(
    const PrgValue& value,
    const NumericBehavior) {
    return checked_sqlgetprop_handle_argument(value, NumericBehavior::copperfin);
}

// RQ-CF-PRG-SQLDATABASES-HANDLE-NUMERIC-001: owner-directed extension
// safety under #5611/#6776. Native VFP9 lacks this function; both modes
// use the same checked policy rather than speculating about legacy aliases.
std::optional<std::int32_t> checked_sqldatabases_handle_argument(
    const PrgValue& value,
    const NumericBehavior) {
    return checked_sqlgetprop_handle_argument(value, NumericBehavior::copperfin);
}

// RQ-CF-PRG-SQLPRIMARYKEYS-HANDLE-NUMERIC-001: owner-directed extension
// safety under #5611/#6776; native presence evidence is not an index oracle.
// Both modes use finite checked admission, without invented legacy aliases.
std::optional<std::int32_t> checked_sqlprimarykeys_handle_argument(
    const PrgValue& value,
    const NumericBehavior) {
    return checked_sqlgetprop_handle_argument(value, NumericBehavior::copperfin);
}

// RQ-CF-PRG-SQLFOREIGNKEYS-HANDLE-NUMERIC-001: owner-directed extension
// safety under #5611/#6776; native presence evidence is not an index oracle.
// Both modes use finite checked admission, without invented legacy aliases.
std::optional<std::int32_t> checked_sqlforeignkeys_handle_argument(
    const PrgValue& value,
    const NumericBehavior) {
    return checked_sqlgetprop_handle_argument(value, NumericBehavior::copperfin);
}

// RQ-CF-PRG-SQLCOLUMNS-HANDLE-NUMERIC-001: derived #5611/#6776 safety;
// the native connection-free fixture cannot establish converted indices.
// Both modes reject unsafe handles without speculating about legacy aliases.
std::optional<std::int32_t> checked_sqlcolumns_handle_argument(
    const PrgValue& value,
    const NumericBehavior) {
    return checked_sqlgetprop_handle_argument(value, NumericBehavior::copperfin);
}

// RQ-CF-PRG-SQLROWCOUNT-HANDLE-NUMERIC-001: owner-directed extension
// safety under #5611/#6776. Installed VFP9 lacks this function, so neither
// mode infers native aliases; finite checked admission prevents wrong reads.
std::optional<std::int32_t> checked_sqlrowcount_handle_argument(
    const PrgValue& value,
    const NumericBehavior) {
    return checked_sqlgetprop_handle_argument(value, NumericBehavior::copperfin);
}

// RQ-CF-PRG-SQLPREPARE-HANDLE-NUMERIC-001: derived #5611/#6776 safety;
// native absent errors cannot distinguish connected indices or aliases.
// Preserve existing other coercions; their native admission gap is #7045.
std::optional<std::int32_t> checked_sqlprepare_handle_argument(
    const PrgValue& value,
    const NumericBehavior) {
    return checked_sqlgetprop_handle_argument(value, NumericBehavior::copperfin);
}

// RQ-CF-PRG-SQLEXEC-HANDLE-NUMERIC-001: #5611/#6776 checked safety;
// identical native absent errors establish no connected indices or aliases.
// Preserve checked existing coercions; native admission gap is #7047.
std::optional<std::int32_t> checked_sqlexec_handle_argument(
    const PrgValue& value,
    const NumericBehavior) {
    return checked_sqlgetprop_handle_argument(value, NumericBehavior::copperfin);
}

// RQ-CF-PRG-CALLFN-HANDLE-NUMERIC-001 (#5611/#6776): installed
// FoxTools live handle 1 distinguishes truncation and both-sign low-16 aliases.
// Exact extended integers follow derived exact modular policy, not VFP types.
std::optional<std::int32_t> checked_callfn_handle_argument(
    const PrgValue& value, const NumericBehavior behavior) {
    const bool numeric = value.kind == PrgValueKind::number ||
                         value.kind == PrgValueKind::int64 || value.kind == PrgValueKind::uint64;
    if (numeric && behavior == NumericBehavior::vfp9) {
        std::uint64_t bits;
        if (value.kind == PrgValueKind::int64) {
            bits = static_cast<std::uint64_t>(value.int64_value);
        } else if (value.kind == PrgValueKind::uint64) {
            bits = value.uint64_value;
        } else {
            // Unsafe native absent errors do not establish a converted index.
            const auto integer = checked_truncated_numeric_to_int64(value.number_value);
            if (!integer) return std::nullopt;
            bits = static_cast<std::uint64_t>(*integer);
        }
        return static_cast<std::int32_t>(bits & UINT64_C(65535));
    }
    // Conversion only; do not inherit native SQL aliases or callback semantics.
    return checked_sqlgetprop_handle_argument(value, NumericBehavior::copperfin);
}

// RQ-CF-PRG-ALEN-DIMENSION-NUMERIC-001 (#5611/#6776). Native arrays
// independently distinguish truncation, low-32 aliases and indefinite zero.
// NaN rejection and exact extended integer modular policy are derived safety.
std::optional<std::int32_t> checked_alen_dimension_argument(
    const PrgValue& value, const NumericBehavior behavior) {
    const bool numeric = value.kind == PrgValueKind::number ||
                         value.kind == PrgValueKind::int64 || value.kind == PrgValueKind::uint64;
    if (value.kind == PrgValueKind::number && std::isnan(value.number_value)) {
        return std::nullopt;
    }
    std::optional<std::int64_t> dimension;
    if (numeric && behavior == NumericBehavior::vfp9) {
        dimension = vfp9_numeric_to_int32(value);
    } else if (value.kind == PrgValueKind::int64) {
        dimension = value.int64_value;
    } else if (value.kind == PrgValueKind::uint64) {
        if (value.uint64_value <= 2U) dimension = static_cast<std::int64_t>(value.uint64_value);
    } else {
        // No half-away rounding: preserve this site's ordinary coercions.
        dimension = checked_truncated_numeric_to_int64(value_as_number(value));
    }
    if (!dimension || (numeric && (*dimension < 0 || *dimension > 2)) ||
        *dimension < std::numeric_limits<std::int32_t>::min() ||
        *dimension > std::numeric_limits<std::int32_t>::max()) {
        return std::nullopt;
    }
    return static_cast<std::int32_t>(*dimension);
}

// RQ-CF-PRG-TAG-ORDINAL-NUMERIC-001 (#5611/#6776). Installed TAG
// independently distinguishes ordinals, raw-positive ceiling and negative
// low-32 aliases. NaN/exact extension boundaries derive from safety policy.
std::optional<std::size_t> checked_tag_ordinal_argument(
    const PrgValue& value, const NumericBehavior behavior) {
    const bool numeric = value.kind == PrgValueKind::number ||
                         value.kind == PrgValueKind::int64 || value.kind == PrgValueKind::uint64;
    if (!numeric) {
        // Preserve this site's existing minimum-one/truncation coercions,
        // without claiming native other-type admission or routing parity.
        const double ordinal = std::max(1.0, value_as_number(value));
        // The exclusive power-of-two bound is exact even when SIZE_MAX is
        // not exactly representable as double. Do not narrow through int64.
        if (!std::isfinite(ordinal) ||
            ordinal >= std::ldexp(1.0, std::numeric_limits<std::size_t>::digits)) {
            return std::nullopt;
        }
        return static_cast<std::size_t>(ordinal);
    }
    constexpr std::int64_t ceiling = 32767;
    if ((value.kind == PrgValueKind::number &&
         (std::isnan(value.number_value) || value.number_value > ceiling)) ||
        (value.kind == PrgValueKind::int64 && value.int64_value > ceiling) ||
        (value.kind == PrgValueKind::uint64 && value.uint64_value > ceiling)) {
        return std::nullopt;
    }
    const auto ordinal = behavior == NumericBehavior::vfp9
        ? std::optional<std::int64_t>{vfp9_numeric_to_int32(value)}
        : checked_declared_int64_argument(value);
    if (!ordinal || *ordinal < 1 || *ordinal > ceiling) {
        return std::nullopt;
    }
    return static_cast<std::size_t>(*ordinal);
}

// RQ-CF-PRG-KEY-ORDINAL-NUMERIC-001 (#5611/#6776). Installed KEY
// independently distinguishes ordinals, raw-positive ceiling and negative
// low-32 aliases. NaN/exact extension boundaries derive from safety policy.
std::optional<std::size_t> checked_key_ordinal_argument(
    const PrgValue& value, const NumericBehavior behavior) {
    const bool numeric = value.kind == PrgValueKind::number ||
                         value.kind == PrgValueKind::int64 || value.kind == PrgValueKind::uint64;
    if (!numeric) {
        // Preserve this site's existing less-than-one empty/truncation coercions,
        // without claiming native other-type admission or routing parity.
        const double ordinal = value_as_number(value);
        // Engaged zero means the preserved empty selection, not an error.
        if (!(ordinal >= 1.0)) return std::size_t{0};
        // The exclusive power-of-two bound is exact even when SIZE_MAX is
        // not exactly representable as double. Do not narrow through int64.
        if (!std::isfinite(ordinal) ||
            ordinal >= std::ldexp(1.0, std::numeric_limits<std::size_t>::digits)) {
            return std::nullopt;
        }
        return static_cast<std::size_t>(ordinal);
    }
    constexpr std::int64_t ceiling = 32767;
    if ((value.kind == PrgValueKind::number &&
         (std::isnan(value.number_value) || value.number_value > ceiling)) ||
        (value.kind == PrgValueKind::int64 && value.int64_value > ceiling) ||
        (value.kind == PrgValueKind::uint64 && value.uint64_value > ceiling)) {
        return std::nullopt;
    }
    const auto ordinal = behavior == NumericBehavior::vfp9
        ? std::optional<std::int64_t>{vfp9_numeric_to_int32(value)}
        : checked_declared_int64_argument(value);
    if (!ordinal || *ordinal < 1 || *ordinal > ceiling) {
        return std::nullopt;
    }
    return static_cast<std::size_t>(*ordinal);
}


// RQ-CF-PRG-DESCENDING-ORDINAL-NUMERIC-001 (#5611/#6776). Installed DESCENDING
// independently distinguishes ordinals, raw-positive ceiling and negative
// low-32 aliases. NaN/exact extension boundaries derive from safety policy.
std::optional<std::size_t> checked_descending_ordinal_argument(
    const PrgValue& value, const NumericBehavior behavior) {
    const bool numeric = value.kind == PrgValueKind::number ||
                         value.kind == PrgValueKind::int64 || value.kind == PrgValueKind::uint64;
    if (!numeric) {
        // Preserve this site's existing less-than-one empty/truncation coercions,
        // without claiming native other-type admission or routing parity.
        const double ordinal = value_as_number(value);
        // Engaged zero means the preserved empty selection, not an error.
        if (!(ordinal >= 1.0)) return std::size_t{0};
        // The exclusive power-of-two bound is exact even when SIZE_MAX is
        // not exactly representable as double. Do not narrow through int64.
        if (!std::isfinite(ordinal) ||
            ordinal >= std::ldexp(1.0, std::numeric_limits<std::size_t>::digits)) {
            return std::nullopt;
        }
        return static_cast<std::size_t>(ordinal);
    }
    constexpr std::int64_t ceiling = 32767;
    if ((value.kind == PrgValueKind::number &&
         (std::isnan(value.number_value) || value.number_value > ceiling)) ||
        (value.kind == PrgValueKind::int64 && value.int64_value > ceiling) ||
        (value.kind == PrgValueKind::uint64 && value.uint64_value > ceiling)) {
        return std::nullopt;
    }
    const auto ordinal = behavior == NumericBehavior::vfp9
        ? std::optional<std::int64_t>{vfp9_numeric_to_int32(value)}
        : checked_declared_int64_argument(value);
    if (!ordinal || *ordinal < 1 || *ordinal > ceiling) {
        return std::nullopt;
    }
    return static_cast<std::size_t>(*ordinal);
}

// RQ-CF-PRG-SET-DATASESSION-NUMERIC-001 (#5611/#6776). Installed VFP9
// sessions 1/2 recover fractional truncation and both-sign low-32 aliases.
// Exact integers/NaN and other positive IDs follow derived safety policy.
std::optional<std::int32_t> checked_datasession_selector_argument(
    const PrgValue& value,
    const NumericBehavior behavior) {
    const bool numeric = value.kind == PrgValueKind::number || value.kind == PrgValueKind::int64 ||
                         value.kind == PrgValueKind::uint64;
    if (value.kind == PrgValueKind::number && std::isnan(value.number_value)) {
        return std::nullopt;
    }
    const auto selector = checked_afont_size_argument(
        value, numeric ? behavior : NumericBehavior::copperfin);
    if (!selector.has_value() || (numeric && *selector < 1)) {
        return std::nullopt;
    }
    // Preserve existing non-Numeric coercions without claiming native type parity.
    return std::max<std::int32_t>(1, *selector);
}

// RQ-CF-PRG-SET-DECIMALS-NUMERIC-001 (#5611/#6776). Retained installed
// VFP9 observations recover truncation, 0..18 and both-sign aliases/indefinite.
std::optional<std::int32_t> checked_set_decimals_argument(
    const PrgValue& value,
    const NumericBehavior behavior) {
    const bool numeric = value.kind == PrgValueKind::number || value.kind == PrgValueKind::int64 ||
                         value.kind == PrgValueKind::uint64;
    if (!numeric) {
        // Preserve existing non-Numeric coercions, not native type parity.
        try {
            const auto rounded = checked_truncated_numeric_to_int64(std::round(value_as_number(value)));
            return rounded.has_value() ? static_cast<std::int32_t>(std::clamp<std::int64_t>(*rounded, 0, 18)) : 2;
        } catch (...) {
            return 2; // Same coercion fallback as the previous SET integer path.
        }
    }
    const auto decimals = checked_afont_size_argument(value, behavior);
    if (!decimals.has_value() || *decimals < 0 || *decimals > 18) {
        return std::nullopt;
    }
    return *decimals;
}

// RQ-CF-PRG-SET-FDOW-NUMERIC-001 (#5611/#6776). Installed VFP9
// recovers 1..7 truncation and negative-only aliases, not positive wrapping.
std::optional<std::int32_t> checked_set_fdow_argument(
    const PrgValue& value,
    const NumericBehavior behavior) {
    const bool numeric = value.kind == PrgValueKind::number || value.kind == PrgValueKind::int64 ||
                         value.kind == PrgValueKind::uint64;
    if (!numeric) {
        // Preserve ordinary previous coercions, not native type parity.
        try {
            const auto rounded = checked_truncated_numeric_to_int64(std::round(value_as_number(value)));
            return rounded.has_value() ? static_cast<std::int32_t>(std::clamp<std::int64_t>(*rounded, 1, 7)) : 1;
        } catch (...) {
            return 1;
        }
    }
    // Reuse only the independently verified negative-only conversion model,
    // not SQL property/session semantics. Zero/indefinite also reject here.
    const auto day = checked_sqlgetprop_handle_argument(value, behavior);
    if (!day.has_value() || *day < 1 || *day > 7) {
        return std::nullopt;
    }
    return *day;
}

// RQ-CF-PRG-SET-FWEEK-NUMERIC-001 (#5611/#6776). Installed VFP9
// recovers 1..3 truncation and negative-only aliases, not positive wrapping.
std::optional<std::int32_t> checked_set_fweek_argument(
    const PrgValue& value,
    const NumericBehavior behavior) {
    const bool numeric = value.kind == PrgValueKind::number || value.kind == PrgValueKind::int64 ||
                         value.kind == PrgValueKind::uint64;
    if (!numeric) {
        // Preserve ordinary previous coercions, not native type parity.
        try {
            const auto rounded = checked_truncated_numeric_to_int64(std::round(value_as_number(value)));
            return rounded.has_value() ? static_cast<std::int32_t>(std::clamp<std::int64_t>(*rounded, 1, 3)) : 1;
        } catch (...) {
            return 1;
        }
    }
    // Reuse only the independently verified negative-only conversion model,
    // not SQL property/session semantics. Zero/indefinite also reject here.
    const auto week = checked_sqlgetprop_handle_argument(value, behavior);
    if (!week.has_value() || *week < 1 || *week > 3) {
        return std::nullopt;
    }
    return *week;
}

// RQ-CF-PRG-SET-EPOCH-NUMERIC-001: derived extension contract under the
// owner's 2026-10-06 retention/judgment policy, not native EPOCH semantics.
std::optional<std::int32_t> checked_set_epoch_argument(const PrgValue& value) {
    std::optional<std::int64_t> year;
    if (value.kind == PrgValueKind::int64) {
        year = value.int64_value;
    } else if (value.kind == PrgValueKind::uint64) {
        if (value.uint64_value > 9999) {
            return std::nullopt;
        }
        year = static_cast<std::int64_t>(value.uint64_value);
    } else if (value.kind == PrgValueKind::number) {
        year = checked_truncated_numeric_to_int64(value.number_value);
    } else {
        // Preserve ordinary pre-existing coercions without type-parity claims.
        try {
            const auto rounded = checked_truncated_numeric_to_int64(std::round(value_as_number(value)));
            return rounded.has_value() ? static_cast<std::int32_t>(std::clamp<std::int64_t>(*rounded, 1, 9999)) : 1950;
        } catch (...) {
            return 1950;
        }
    }
    if (!year.has_value() || *year < 1 || *year > 9999) {
        return std::nullopt;
    }
    return static_cast<std::int32_t>(*year);
}

// RQ-CF-PRG-SET-CENTURY-NUMERIC-001: bounded, exact conversion before
// mutation; native observations in vfp9-set-century-rollover-observation.
std::optional<std::int32_t> checked_set_century_argument(
    const PrgValue& value, const NumericBehavior behavior, const int minimum, const int maximum) {
    std::optional<std::int64_t> converted;
    if (value.is_null || (value.kind != PrgValueKind::number && value.kind != PrgValueKind::int64 &&
        value.kind != PrgValueKind::uint64 && value.kind != PrgValueKind::currency)) {
        return std::nullopt;
    }
    if (behavior == NumericBehavior::vfp9) {
        converted = value.kind == PrgValueKind::currency
            ? vfp9_numeric_to_int32(make_int64_value(value.currency_value / 10000))
            : vfp9_numeric_to_int32(value);
    } else if (value.kind == PrgValueKind::int64) {
        converted = value.int64_value;
    } else if (value.kind == PrgValueKind::uint64) {
        if (value.uint64_value > static_cast<std::uint64_t>(maximum)) return std::nullopt;
        converted = static_cast<std::int64_t>(value.uint64_value);
    } else if (value.kind == PrgValueKind::currency) {
        converted = value.currency_value / 10000;
    } else {
        converted = checked_truncated_numeric_to_int64(value.number_value);
    }
    if (!converted.has_value() || *converted < minimum || *converted > maximum) return std::nullopt;
    return static_cast<std::int32_t>(*converted);
}

int default_set_century_epoch_for_year(const int year) {
    // Derived clock-boundary policy keeps the shared representable year domain.
    return static_cast<int>(std::clamp<std::int64_t>(static_cast<std::int64_t>(year) - 50, 1, 9999));
}

int default_set_century_epoch() {
    const auto local = local_time_from_time_t(std::time(nullptr));
    return default_set_century_epoch_for_year(local.tm_year + 1900);
}

std::optional<std::int32_t> checked_declared_int32_argument(
    const PrgValue& value,
    const NumericBehavior behavior) {
    if (value.kind == PrgValueKind::int64) {
        return static_cast<std::int32_t>(vfp9_numeric_to_int32(value));
    }
    if (value.kind == PrgValueKind::uint64) {
        return static_cast<std::int32_t>(vfp9_numeric_to_int32(value));
    }
    const double numeric_value = value_as_number(value);
    if (!std::isfinite(numeric_value)) {
        return std::nullopt;
    }
    const auto converted = checked_truncated_numeric_to_int64(numeric_value);
    if (!converted.has_value()) {
        if (behavior == NumericBehavior::vfp9) {
            return static_cast<std::int32_t>(vfp9_numeric_to_int32(numeric_value));
        }
        return std::nullopt;
    }
    return declared_int32_from_int64(*converted);
}

std::optional<std::int64_t> checked_declared_int64_argument(const PrgValue& value) {
    if (value.kind == PrgValueKind::int64) {
        return value.int64_value;
    }
    if (value.kind == PrgValueKind::uint64) {
        if (value.uint64_value > static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())) {
            return std::nullopt;
        }
        return static_cast<std::int64_t>(value.uint64_value);
    }
    return checked_truncated_numeric_to_int64(value_as_number(value));
}

std::size_t saturating_size_argument(const double value, const std::size_t minimum) {
    const std::int64_t truncated = saturating_numeric_to_int64(value);
    if (truncated <= 0 || static_cast<std::uint64_t>(truncated) < minimum) {
        return minimum;
    }
    return static_cast<std::size_t>(std::min<std::uint64_t>(
        static_cast<std::uint64_t>(truncated), std::numeric_limits<std::size_t>::max()));
}

std::int64_t rounded_numeric_to_int64(const double value) {
    if (std::isnan(value)) {
        return 0;
    }
    if (value >= 9223372036854775808.0 || value <= -9223372036854775808.0) {
        return saturating_numeric_to_int64(value);
    }
    return static_cast<std::int64_t>(std::llround(value));
}

int saturating_int_argument(const double value, const int minimum, const int maximum) {
    const std::int64_t truncated = saturating_numeric_to_int64(value);
    return static_cast<int>(std::clamp<std::int64_t>(truncated, minimum, maximum));
}

std::optional<std::size_t> checked_character_string_length(const double requested) {
    if (!std::isfinite(requested)) {
        return std::nullopt;
    }
    const double truncated = std::trunc(std::max(0.0, requested));
    if (truncated > kVfpMaxCharacterStringLength) {
        return std::nullopt;
    }
    return static_cast<std::size_t>(truncated);
}

bool numeric_prg_values_equal(const PrgValue& left, const PrgValue& right) {
    if (const auto comparison = try_exact_integer_compare(left, right); comparison.has_value()) {
        return *comparison == 0;
    }
    if (left.kind == PrgValueKind::currency && right.kind == PrgValueKind::currency) {
        return left.currency_value == right.currency_value;
    }
    return numeric_values_equal(value_as_number(left), value_as_number(right));
}

namespace {

bool is_aggregate_numeric(const PrgValue& value) {
    return value.kind == PrgValueKind::number || value.kind == PrgValueKind::int64 ||
           value.kind == PrgValueKind::uint64 || value.kind == PrgValueKind::currency;
}

// Orders two values of one domain for MIN and MAX. A Date or DateTime compares by its timeline value (an empty one
// is the lowest), a Currency pair by its scaled integers, a Logical false before true, other numbers by value and
// Character values ignoring trailing blanks.
int aggregate_compare_domain(const PrgValue& value) {
    if (is_aggregate_numeric(value)) {
        return 1;
    }
    if (value.kind == PrgValueKind::boolean) {
        return 2;
    }
    if (value.kind == PrgValueKind::string && value.string_flavor != PrgStringFlavor::none) {
        return 3;
    }
    return 4;   // Character (and anything else that compares as text)
}

int compare_aggregate_values(const PrgValue& left, const PrgValue& right) {
    const bool left_temporal = left.kind == PrgValueKind::string && left.string_flavor != PrgStringFlavor::none;
    const bool right_temporal = right.kind == PrgValueKind::string && right.string_flavor != PrgStringFlavor::none;
    if (left_temporal && right_temporal) {
        const std::int64_t a = left.string_value.empty() ? std::numeric_limits<std::int64_t>::min() : left.int64_value;
        const std::int64_t b = right.string_value.empty() ? std::numeric_limits<std::int64_t>::min() : right.int64_value;
        return a < b ? -1 : (a > b ? 1 : 0);
    }
    if (left.kind == PrgValueKind::boolean && right.kind == PrgValueKind::boolean) {
        return static_cast<int>(left.boolean_value) - static_cast<int>(right.boolean_value);
    }
    if (left.kind == PrgValueKind::currency && right.kind == PrgValueKind::currency) {
        return left.currency_value < right.currency_value ? -1 : (left.currency_value > right.currency_value ? 1 : 0);
    }
    if (is_aggregate_numeric(left) && is_aggregate_numeric(right)) {
        const double a = value_as_number(left);
        const double b = value_as_number(right);
        return a < b ? -1 : (a > b ? 1 : 0);
    }
    const std::string a = rtrim_space_copy(value_as_string(left));
    const std::string b = rtrim_space_copy(value_as_string(right));
    return a < b ? -1 : (a > b ? 1 : 0);
}

}  // namespace

AggregateAccumulator::AggregateAccumulator(const bool sql_semantics) : sql_semantics_(sql_semantics) {}

void AggregateAccumulator::add(const std::string& function, const PrgValue& value) {
    if (value.is_null || value.kind == PrgValueKind::empty) {
        return;
    }
    if (value.unparsed_numeric_field) {
        // A numeric DBF field that overflowed its width holds a run of asterisks; every aggregate skips it as they
        // always have (test_aggregate_helpers_tolerate_non_numeric_field_text). Provenance, not the text: a Character
        // value "***" is still a Character.
        return;
    }
    if (function == "min" || function == "max") {
        if (count_ == 0U) {
            minimum_ = value;
            maximum_ = value;
            compare_domain_ = aggregate_compare_domain(value);
        } else {
            if (aggregate_compare_domain(value) != compare_domain_) {
                throw PrgCompatibilityError(runtime_text("Runtime.Prg.Expression.Error.OperatorOperandTypeMismatch"), 107);
            }
            if (compare_aggregate_values(value, minimum_) < 0) {
                minimum_ = value;
            }
            if (compare_aggregate_values(value, maximum_) > 0) {
                maximum_ = value;
            }
        }
        ++count_;
        return;
    }
    if (!is_aggregate_numeric(value)) {
        if (sql_semantics_) {
            throw PrgCompatibilityError(runtime_text("Runtime.Prg.Aggregate.Error.SqlNonNumeric"), 1811);
        }
        throw PrgCompatibilityError(runtime_text("Runtime.Prg.Aggregate.Error.NotNumeric"), 27);
    }
    double_sum_ += value_as_number(value);
    if (value.kind != PrgValueKind::currency) {
        currency_only_ = false;
    } else if (currency_only_) {
        const std::int64_t addend = value.currency_value;
        if ((addend > 0 && currency_sum_ > std::numeric_limits<std::int64_t>::max() - addend) ||
            (addend < 0 && currency_sum_ < std::numeric_limits<std::int64_t>::min() - addend)) {
            throw PrgCompatibilityError(runtime_text("Runtime.Prg.Expression.Error.CurrencyOutOfRange"), 1988);
        }
        currency_sum_ += addend;
    }
    ++count_;
}

PrgValue AggregateAccumulator::result(const std::string& function, const PrgValue& empty_result) const {
    if (count_ == 0U) {
        return empty_result;
    }
    if (function == "min") {
        return minimum_;
    }
    if (function == "max") {
        return maximum_;
    }
    if (function == "sum") {
        return currency_only_ ? make_currency_value(currency_sum_) : make_number_value(double_sum_);
    }
    // avg / average
    if (!currency_only_) {
        return make_number_value(double_sum_ / static_cast<double>(count_));
    }
    const std::int64_t divisor = static_cast<std::int64_t>(count_);
    std::int64_t quotient = currency_sum_ / divisor;
    const std::int64_t remainder = currency_sum_ % divisor;
    // CALCULATE AVG and the AVERAGE command truncate toward zero (installed VFP9: the average of $0.0001, $0.0002 and
    // $0.0002 is $0.0001), while SQL AVG rounds half away from zero ($0.0002).
    const std::int64_t magnitude = remainder < 0 ? -remainder : remainder;
    if (sql_semantics_ && magnitude >= divisor - magnitude) {
        quotient += currency_sum_ < 0 ? -1 : 1;
    }
    return make_currency_value(quotient);
}

std::string aggregate_distinct_key(const PrgValue& value) {
    switch (value.kind) {
        case PrgValueKind::number: {
            std::ostringstream stream;
            stream.imbue(std::locale::classic());
            stream << std::setprecision(17) << value.number_value;
            return "n:" + stream.str();
        }
        case PrgValueKind::int64:
            return "n:" + std::to_string(value.int64_value);
        case PrgValueKind::uint64:
            return "n:" + std::to_string(value.uint64_value);
        case PrgValueKind::currency:
            return "y:" + std::to_string(value.currency_value);
        case PrgValueKind::boolean:
            return value.boolean_value ? "l:t" : "l:f";
        case PrgValueKind::string:
            return "s:" + rtrim_space_copy(value.string_value);
        default:
            return "e:";
    }
}

bool statement_condition_value(const PrgValue& value, const StatementConditionKind kind) {
    if (value.is_null) {
        return false;
    }
    const PrgOperandClass operand_class = classify_operand(value);
    if (operand_class == PrgOperandClass::logical) {
        return value_as_bool(value);
    }
    if (operand_class == PrgOperandClass::empty) {
        // A condition that produced no value at all: a predicate whose fault ON ERROR already handled, or an
        // unresolved identifier (error 12 in VFP9, which an operator reports but a bare condition cannot be
        // told apart from a handled fault). It stays false, as before.
        return false;
    }
    switch (kind) {
        case StatementConditionKind::if_statement:
            throw PrgCompatibilityError(runtime_text("Runtime.Prg.Expression.Error.DataTypeMismatch"), 9);
        case StatementConditionKind::do_case:
            throw PrgCompatibilityError(runtime_text("Runtime.Prg.Statement.Error.SyntaxError"), 10);
        case StatementConditionKind::filter:
            throw PrgCompatibilityError(runtime_text("Runtime.Prg.Statement.Error.FilterLogicalRequired"), 37);
        case StatementConditionKind::sql_where:
            throw PrgCompatibilityError(runtime_text("Runtime.Prg.Statement.Error.SqlWhereInvalid"), 1833);
        case StatementConditionKind::for_while:
            break;
    }
    throw PrgCompatibilityError(runtime_text("Runtime.Prg.Statement.Error.ForWhileLogicalRequired"), 1127);
}

PrgValue make_null_value() {
    PrgValue result;
    result.is_null = true;
    return result;
}

PrgValue make_boolean_value(bool value) {
    PrgValue result;
    result.kind = PrgValueKind::boolean;
    result.boolean_value = value;
    return result;
}

PrgValue make_number_value(double value) {
    PrgValue result;
    result.kind = PrgValueKind::number;
    result.number_value = value;
    return result;
}

PrgValue make_string_value(std::string value) {
    PrgValue result;
    result.kind = PrgValueKind::string;
    result.string_value = std::move(value);
    return result;
}

PrgValue make_object_reference_value(std::string value) {
    PrgValue result = make_string_value(std::move(value));
    result.is_object_reference = true;
    return result;
}

PrgValue make_date_value(std::string value) {
    PrgValue result = make_string_value(std::move(value));
    result.string_flavor = PrgStringFlavor::date;
    // Flavored strings retain a timeline scalar in their otherwise-unused integer payload.
    int year = 0;
    int month = 0;
    int day = 0;
    if (parse_runtime_date_string(result.string_value, year, month, day)) {
        constexpr std::int64_t seconds_per_day = 24LL * 60LL * 60LL;
        result.int64_value = static_cast<std::int64_t>(date_to_julian(year, month, day)) * seconds_per_day;
    }
    return result;
}

PrgValue make_date_value(std::string value, int year, int month, int day) {
    PrgValue result = make_string_value(std::move(value));
    result.string_flavor = PrgStringFlavor::date;
    constexpr std::int64_t seconds_per_day = 24LL * 60LL * 60LL;
    result.int64_value = static_cast<std::int64_t>(date_to_julian(year, month, day)) * seconds_per_day;
    return result;
}

PrgValue make_datetime_value(std::string value) {
    PrgValue result = make_string_value(std::move(value));
    result.string_flavor = PrgStringFlavor::datetime;
    int year = 0;
    int month = 0;
    int day = 0;
    int hour = 0;
    int minute = 0;
    int second = 0;
    if (parse_runtime_datetime_string(result.string_value, year, month, day, hour, minute, second)) {
        constexpr std::int64_t seconds_per_day = 24LL * 60LL * 60LL;
        result.int64_value = (static_cast<std::int64_t>(date_to_julian(year, month, day)) * seconds_per_day) +
                             (hour * 3600LL) + (minute * 60LL) + second;
    }
    return result;
}

PrgValue make_datetime_value(
    std::string value,
    int year,
    int month,
    int day,
    int hour,
    int minute,
    int second) {
    PrgValue result = make_string_value(std::move(value));
    result.string_flavor = PrgStringFlavor::datetime;
    constexpr std::int64_t seconds_per_day = 24LL * 60LL * 60LL;
    result.int64_value = (static_cast<std::int64_t>(date_to_julian(year, month, day)) * seconds_per_day) +
                         (hour * 3600LL) + (minute * 60LL) + second;
    return result;
}

namespace {

// Unsigned 128-bit integer for the exact Currency multiply and divide; a plain pair of 64-bit words so it behaves the
// same on every compiler (MSVC has no __int128).
struct U128 {
    std::uint64_t hi = 0U;
    std::uint64_t lo = 0U;
};

U128 u128_multiply(const std::uint64_t a, const std::uint64_t b) {
    const std::uint64_t a_lo = a & 0xFFFFFFFFULL;
    const std::uint64_t a_hi = a >> 32U;
    const std::uint64_t b_lo = b & 0xFFFFFFFFULL;
    const std::uint64_t b_hi = b >> 32U;
    const std::uint64_t ll = a_lo * b_lo;
    const std::uint64_t lh = a_lo * b_hi;
    const std::uint64_t hl = a_hi * b_lo;
    const std::uint64_t hh = a_hi * b_hi;
    const std::uint64_t carry = ((ll >> 32U) + (lh & 0xFFFFFFFFULL) + (hl & 0xFFFFFFFFULL)) >> 32U;
    U128 result;
    result.lo = a * b;
    result.hi = hh + (lh >> 32U) + (hl >> 32U) + carry;
    return result;
}

// a * b, or false if it does not fit in 128 bits.
bool u128_multiply_by_u64(const U128& a, const std::uint64_t b, U128& out) {
    const U128 low = u128_multiply(a.lo, b);
    const U128 high = u128_multiply(a.hi, b);
    if (high.hi != 0U) {
        return false;
    }
    const std::uint64_t hi = low.hi + high.lo;
    if (hi < low.hi) {
        return false;
    }
    out.hi = hi;
    out.lo = low.lo;
    return true;
}

bool u128_less(const U128& a, const U128& b) {
    return a.hi != b.hi ? a.hi < b.hi : a.lo < b.lo;
}

U128 u128_subtract(const U128& a, const U128& b) {
    U128 result;
    result.lo = a.lo - b.lo;
    result.hi = a.hi - b.hi - (a.lo < b.lo ? 1U : 0U);
    return result;
}

// Schoolbook shift-and-subtract division: 128 iterations, fine for a monetary operator.
void u128_divide(const U128& numerator, const U128& denominator, U128& quotient, U128& remainder) {
    quotient = U128{};
    remainder = U128{};
    for (int bit = 127; bit >= 0; --bit) {
        remainder.hi = (remainder.hi << 1U) | (remainder.lo >> 63U);
        remainder.lo <<= 1U;
        const std::uint64_t word = bit >= 64 ? numerator.hi : numerator.lo;
        remainder.lo |= (word >> static_cast<unsigned>(bit % 64)) & 1U;
        if (!u128_less(remainder, denominator)) {
            remainder = u128_subtract(remainder, denominator);
            if (bit >= 64) {
                quotient.hi |= 1ULL << static_cast<unsigned>(bit - 64);
            } else {
                quotient.lo |= 1ULL << static_cast<unsigned>(bit);
            }
        }
    }
}

// numerator / denominator rounded half away from zero, as an unsigned magnitude; false if it needs more than 64 bits.
bool u128_divide_round(const U128& numerator, const U128& denominator, std::uint64_t& magnitude) {
    U128 quotient;
    U128 remainder;
    u128_divide(numerator, denominator, quotient, remainder);
    if (quotient.hi != 0U) {
        return false;
    }
    magnitude = quotient.lo;
    // Round half away from zero: the remainder is at least half of the divisor.
    if (!u128_less(remainder, u128_subtract(denominator, remainder))) {
        if (magnitude == std::numeric_limits<std::uint64_t>::max()) {
            return false;
        }
        ++magnitude;
    }
    return true;
}

bool u128_power_of_ten(const int exponent, U128& out) {
    out = U128{0U, 1U};
    for (int index = 0; index < exponent; ++index) {
        if (!u128_multiply_by_u64(out, 10U, out)) {
            return false;
        }
    }
    return true;
}

// An operand as sign, magnitude and a decimal exponent: magnitude * 10^-scale.
struct ExactOperand {
    bool negative = false;
    std::uint64_t magnitude = 0U;
    int scale = 0;
};

bool exact_operand(const PrgValue& value, ExactOperand& out) {
    switch (value.kind) {
        case PrgValueKind::currency:
            out.negative = value.currency_value < 0;
            out.magnitude = out.negative ? 0ULL - static_cast<std::uint64_t>(value.currency_value)
                                         : static_cast<std::uint64_t>(value.currency_value);
            out.scale = 4;
            return true;
        case PrgValueKind::int64:
            out.negative = value.int64_value < 0;
            out.magnitude = out.negative ? 0ULL - static_cast<std::uint64_t>(value.int64_value)
                                         : static_cast<std::uint64_t>(value.int64_value);
            out.scale = 0;
            return true;
        case PrgValueKind::uint64:
            out.negative = false;
            out.magnitude = value.uint64_value;
            out.scale = 0;
            return true;
        case PrgValueKind::number:
            break;
        default:
            return false;
    }
    if (!std::isfinite(value.number_value)) {
        return false;
    }
    const std::string text = format_round_trip_decimal(value.number_value);
    std::size_t index = 0U;
    out.negative = !text.empty() && text[0] == '-';
    if (out.negative) {
        index = 1U;
    }
    std::string digits;
    int scale = 0;
    bool after_point = false;
    for (; index < text.size(); ++index) {
        const char ch = text[index];
        if (ch == '.') {
            after_point = true;
        } else if (ch >= '0' && ch <= '9') {
            digits.push_back(ch);
            if (after_point) {
                ++scale;
            }
        } else {
            return false;   // an exponent or anything else: not a plain decimal
        }
    }
    const std::size_t first = digits.find_first_not_of('0');
    digits = first == std::string::npos ? std::string("0") : digits.substr(first);
    if (digits.size() > 19U || scale > 18) {
        return false;
    }
    std::uint64_t magnitude = 0U;
    for (const char ch : digits) {
        const std::uint64_t digit = static_cast<std::uint64_t>(ch - '0');
        if (magnitude > (std::numeric_limits<std::uint64_t>::max() - digit) / 10U) {
            return false;
        }
        magnitude = magnitude * 10U + digit;
    }
    out.magnitude = magnitude;
    out.scale = scale;
    return true;
}

}  // namespace

CurrencyArithmeticResult currency_multiply_divide_exact(const PrgValue& left, const PrgValue& right, const char operation) {
    CurrencyArithmeticResult result;
    ExactOperand a;
    ExactOperand b;
    if (!exact_operand(left, a) || !exact_operand(right, b)) {
        return result;   // unsupported
    }
    std::uint64_t magnitude = 0U;
    bool fits = false;
    if (operation == '*') {
        // (a / 10^sa) * (b / 10^sb) as a scaled Currency is a * b / 10^(sa + sb - 4).
        const int shift = a.scale + b.scale - 4;
        if (shift < 0) {
            return result;
        }
        U128 divisor;
        if (!u128_power_of_ten(shift, divisor)) {
            return result;
        }
        fits = u128_divide_round(u128_multiply(a.magnitude, b.magnitude), divisor, magnitude);
    } else {
        // (a / 10^sa) / (b / 10^sb) as a scaled Currency is a * 10^(4 + sb - sa) / b.
        const int shift = 4 + b.scale - a.scale;
        U128 numerator{0U, a.magnitude};
        U128 denominator{0U, b.magnitude};
        U128 factor;
        if (denominator.hi == 0U && denominator.lo == 0U) {
            return result;
        }
        if (!u128_power_of_ten(shift >= 0 ? shift : -shift, factor)) {
            return result;
        }
        if (shift >= 0) {
            if (!u128_multiply_by_u64(factor, a.magnitude, numerator)) {
                return result;
            }
        } else if (!u128_multiply_by_u64(factor, b.magnitude, denominator)) {
            return result;
        }
        fits = u128_divide_round(numerator, denominator, magnitude);
    }
    const bool negative = (a.negative != b.negative) && magnitude != 0U;
    constexpr std::uint64_t kMaxPositive = static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max());
    if (!fits || (!negative && magnitude > kMaxPositive) || (negative && magnitude > kMaxPositive + 1U)) {
        result.status = CurrencyArithmeticStatus::out_of_range;
        return result;
    }
    result.status = CurrencyArithmeticStatus::ok;
    result.scaled = negative ? static_cast<std::int64_t>(0ULL - magnitude) : static_cast<std::int64_t>(magnitude);
    return result;
}

CurrencyDecimal parse_currency_decimal(const std::string& text, const bool allow_storage_minimum) {
    CurrencyDecimal result;
    std::size_t index = 0U;
    const std::size_t end = text.find_last_not_of(" \t") == std::string::npos ? 0U : text.find_last_not_of(" \t") + 1U;
    while (index < end && (text[index] == ' ' || text[index] == '\t')) {
        ++index;
    }
    bool negative = false;
    if (index < end && text[index] == '-') {
        negative = true;
        ++index;
    }
    std::string integer_digits;
    while (index < end && std::isdigit(static_cast<unsigned char>(text[index])) != 0) {
        integer_digits.push_back(text[index++]);
    }
    std::string fraction_digits;
    if (index < end && text[index] == '.') {
        ++index;
        while (index < end && std::isdigit(static_cast<unsigned char>(text[index])) != 0) {
            fraction_digits.push_back(text[index++]);
        }
    }
    if (index != end || (integer_digits.empty() && fraction_digits.empty())) {
        return result;   // malformed
    }
    const std::size_t first_significant = integer_digits.find_first_not_of('0');
    integer_digits = first_significant == std::string::npos ? std::string{} : integer_digits.substr(first_significant);
    result.status = CurrencyDecimalStatus::out_of_range;
    if (integer_digits.size() > 15U) {
        return result;
    }
    std::uint64_t whole = 0U;
    for (const char digit : integer_digits) {
        whole = (whole * 10U) + static_cast<std::uint64_t>(digit - '0');
    }
    std::uint64_t fraction = 0U;
    for (std::size_t position = 0U; position < 4U; ++position) {
        fraction = (fraction * 10U) + (position < fraction_digits.size()
                                           ? static_cast<std::uint64_t>(fraction_digits[position] - '0')
                                           : 0U);
    }
    const bool round_up = fraction_digits.size() > 4U && fraction_digits[4] >= '5';
    const std::uint64_t magnitude = (whole * 10000U) + fraction + (round_up ? 1U : 0U);
    const std::uint64_t storage_limit = static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max());
    if (allow_storage_minimum && negative && magnitude == storage_limit + 1U) {
        result.status = CurrencyDecimalStatus::ok;
        result.scaled = std::numeric_limits<std::int64_t>::min();
        return result;
    }
    if (magnitude > storage_limit) {
        return result;
    }
    result.status = CurrencyDecimalStatus::ok;
    result.scaled = negative ? -static_cast<std::int64_t>(magnitude) : static_cast<std::int64_t>(magnitude);
    return result;
}

std::string format_currency_decimal_text(const std::int64_t scaled, const int decimals) {
    const bool negative = scaled < 0;
    const std::uint64_t magnitude = negative ? static_cast<std::uint64_t>(-(scaled + 1)) + 1U
                                             : static_cast<std::uint64_t>(scaled);
    const int places = std::clamp(decimals, 0, 4);
    std::uint64_t divisor = 1U;
    for (int step = 0; step < 4 - places; ++step) {
        divisor *= 10U;
    }
    std::uint64_t unit = 1U;
    for (int step = 0; step < places; ++step) {
        unit *= 10U;
    }
    const std::uint64_t rounded = (magnitude + (divisor / 2U)) / divisor;
    std::string text = std::string(negative ? "-" : "") + std::to_string(rounded / unit);
    if (places > 0) {
        std::string fraction = std::to_string(rounded % unit);
        fraction.insert(0U, static_cast<std::size_t>(places) - fraction.size(), '0');
        text += "." + fraction;
    }
    if (decimals > 4) {
        text += std::string(static_cast<std::size_t>(decimals - 4), '0');
    }
    return text;
}

PrgValue make_int64_value(std::int64_t value) {
    PrgValue result;
    result.kind = PrgValueKind::int64;
    result.int64_value = value;
    return result;
}

PrgValue make_uint64_value(std::uint64_t value) {
    PrgValue result;
    result.kind = PrgValueKind::uint64;
    result.uint64_value = value;
    return result;
}

PrgValue make_currency_value(std::int64_t scaled_value) {
    PrgValue result;
    result.kind = PrgValueKind::currency;
    result.currency_value = scaled_value;
    return result;
}

namespace {

enum class BinaryEncoding {
    integer_1,
    integer_2,
    integer_4,
    sortable_double,
    native_float,
    native_double,
    currency
};

struct BinarySelector {
    BinaryEncoding encoding = BinaryEncoding::integer_4;
    bool reverse = false;
    bool suppress_sign_transform = false;
};

[[noreturn]] void throw_invalid_binary_conversion() {
    throw PrgCompatibilityError(runtime_text("Runtime.Prg.Expression.Error.InvalidArgument"), 11);
}

bool is_numeric_value(const PrgValue& value) {
    return value.kind == PrgValueKind::number || value.kind == PrgValueKind::int64 ||
           value.kind == PrgValueKind::uint64 || value.kind == PrgValueKind::currency;
}

std::optional<int> exact_numeric_selector(const PrgValue& selector) {
    if (!is_numeric_value(selector)) {
        return std::nullopt;
    }
    if (selector.kind == PrgValueKind::int64) {
        if (selector.int64_value < std::numeric_limits<int>::min() ||
            selector.int64_value > std::numeric_limits<int>::max()) {
            return std::nullopt;
        }
        return static_cast<int>(selector.int64_value);
    }
    if (selector.kind == PrgValueKind::uint64) {
        if (selector.uint64_value > static_cast<std::uint64_t>(std::numeric_limits<int>::max())) {
            return std::nullopt;
        }
        return static_cast<int>(selector.uint64_value);
    }
    if (selector.kind == PrgValueKind::currency) {
        if (selector.currency_value % 10000 != 0) {
            return std::nullopt;
        }
        const std::int64_t integral = selector.currency_value / 10000;
        if (integral < std::numeric_limits<int>::min() || integral > std::numeric_limits<int>::max()) {
            return std::nullopt;
        }
        return static_cast<int>(integral);
    }
    if (!std::isfinite(selector.number_value) || std::trunc(selector.number_value) != selector.number_value ||
        selector.number_value < static_cast<double>(std::numeric_limits<int>::min()) ||
        selector.number_value > static_cast<double>(std::numeric_limits<int>::max())) {
        return std::nullopt;
    }
    return static_cast<int>(selector.number_value);
}

BinaryEncoding integer_encoding_for_width(const int width) {
    switch (width) {
        case 1: return BinaryEncoding::integer_1;
        case 2: return BinaryEncoding::integer_2;
        case 4: return BinaryEncoding::integer_4;
        default: throw_invalid_binary_conversion();
    }
}

std::size_t encoding_width(const BinaryEncoding encoding) {
    switch (encoding) {
        case BinaryEncoding::integer_1: return 1U;
        case BinaryEncoding::integer_2: return 2U;
        case BinaryEncoding::integer_4:
        case BinaryEncoding::native_float: return 4U;
        case BinaryEncoding::sortable_double:
        case BinaryEncoding::native_double:
        case BinaryEncoding::currency: return 8U;
    }
    throw_invalid_binary_conversion();
}

BinarySelector parse_bintoc_selector(const PrgValue* selector) {
    if (selector == nullptr) {
        return {};
    }
    if (selector->kind != PrgValueKind::string) {
        const auto width = exact_numeric_selector(*selector);
        if (!width.has_value()) {
            throw_invalid_binary_conversion();
        }
        if (*width == 8) {
            return {BinaryEncoding::sortable_double, false, false};
        }
        return {integer_encoding_for_width(*width), false, false};
    }

    const std::string flags = uppercase_copy(selector->string_value);
    if (flags.empty()) {
        throw_invalid_binary_conversion();
    }
    BinarySelector result;
    bool has_encoding = false;
    bool saw_reverse = false;
    bool saw_suppress = false;
    for (const char flag : flags) {
        if (flag == 'R') {
            if (saw_reverse) throw_invalid_binary_conversion();
            saw_reverse = result.reverse = true;
            continue;
        }
        if (flag == 'S') {
            if (saw_suppress) throw_invalid_binary_conversion();
            saw_suppress = result.suppress_sign_transform = true;
            continue;
        }
        if (has_encoding) {
            throw_invalid_binary_conversion();
        }
        has_encoding = true;
        switch (flag) {
            case '1': result.encoding = BinaryEncoding::integer_1; break;
            case '2': result.encoding = BinaryEncoding::integer_2; break;
            case '4': result.encoding = BinaryEncoding::integer_4; break;
            case '8': result.encoding = BinaryEncoding::sortable_double; break;
            case 'F': result.encoding = BinaryEncoding::native_float; break;
            case 'B': result.encoding = BinaryEncoding::native_double; break;
            default: throw_invalid_binary_conversion();
        }
    }
    return result;
}

BinarySelector parse_ctobin_selector(const PrgValue* selector, const std::size_t source_size) {
    if (selector != nullptr && selector->kind != PrgValueKind::string) {
        throw_invalid_binary_conversion();
    }
    BinarySelector result;
    bool has_encoding = false;
    bool saw_reverse = false;
    bool saw_suppress = false;
    const std::string flags = selector == nullptr ? std::string{} : uppercase_copy(selector->string_value);
    if (selector != nullptr && flags.empty()) {
        throw_invalid_binary_conversion();
    }
    for (const char flag : flags) {
        if (flag == 'R') {
            if (saw_reverse) throw_invalid_binary_conversion();
            saw_reverse = result.reverse = true;
            continue;
        }
        if (flag == 'S') {
            if (saw_suppress) throw_invalid_binary_conversion();
            saw_suppress = result.suppress_sign_transform = true;
            continue;
        }
        if (has_encoding) {
            throw_invalid_binary_conversion();
        }
        has_encoding = true;
        switch (flag) {
            case '1': result.encoding = BinaryEncoding::integer_1; break;
            case '2': result.encoding = BinaryEncoding::integer_2; break;
            case '4': result.encoding = BinaryEncoding::integer_4; break;
            case '8': result.encoding = BinaryEncoding::native_double; break;
            case 'N': result.encoding = source_size == 4U ? BinaryEncoding::native_float
                                                          : BinaryEncoding::native_double; break;
            case 'B': result.encoding = BinaryEncoding::sortable_double; break;
            case 'Y': result.encoding = BinaryEncoding::currency; break;
            default: throw_invalid_binary_conversion();
        }
    }
    if (!has_encoding) {
        if (source_size == 8U) {
            result.encoding = BinaryEncoding::sortable_double;
        } else if (source_size == 1U || source_size == 2U || source_size == 4U) {
            result.encoding = integer_encoding_for_width(static_cast<int>(source_size));
        } else {
            throw_invalid_binary_conversion();
        }
    }
    if ((result.encoding == BinaryEncoding::native_float && source_size != 4U) ||
        (result.encoding == BinaryEncoding::native_double && source_size != 8U) ||
        encoding_width(result.encoding) != source_size) {
        throw_invalid_binary_conversion();
    }
    return result;
}

std::optional<std::int64_t> signed_integer_argument(const PrgValue& value, const int bits) {
    const std::int64_t minimum = bits == 32 ? std::numeric_limits<std::int32_t>::min()
                                           : -(std::int64_t{1} << (bits - 1));
    const std::int64_t maximum = bits == 32 ? std::numeric_limits<std::int32_t>::max()
                                           : (std::int64_t{1} << (bits - 1)) - 1;
    if (value.kind == PrgValueKind::int64) {
        return value.int64_value >= minimum && value.int64_value <= maximum
                   ? std::optional<std::int64_t>(value.int64_value)
                   : std::nullopt;
    }
    if (value.kind == PrgValueKind::uint64) {
        return value.uint64_value <= static_cast<std::uint64_t>(maximum)
                   ? std::optional<std::int64_t>(static_cast<std::int64_t>(value.uint64_value))
                   : std::nullopt;
    }
    if (value.kind == PrgValueKind::currency) {
        const std::int64_t integral = value.currency_value / 10000;
        return integral >= minimum && integral <= maximum ? std::optional<std::int64_t>(integral) : std::nullopt;
    }
    if (value.kind != PrgValueKind::number || !std::isfinite(value.number_value)) {
        return std::nullopt;
    }
    const double truncated = std::trunc(value.number_value);
    return truncated >= static_cast<double>(minimum) && truncated <= static_cast<double>(maximum)
               ? std::optional<std::int64_t>(static_cast<std::int64_t>(truncated))
               : std::nullopt;
}

std::string big_endian_bytes(std::uint64_t value, const std::size_t width) {
    std::string result(width, '\0');
    for (std::size_t index = 0U; index < width; ++index) {
        result[width - index - 1U] = static_cast<char>(value & 0xFFU);
        value >>= 8U;
    }
    return result;
}

std::string little_endian_bytes(std::uint64_t value, const std::size_t width) {
    std::string result(width, '\0');
    for (std::size_t index = 0U; index < width; ++index) {
        result[index] = static_cast<char>(value & 0xFFU);
        value >>= 8U;
    }
    return result;
}

std::uint64_t unsigned_from_big_endian(const std::string& bytes) {
    std::uint64_t result = 0U;
    for (const unsigned char byte : bytes) {
        result = (result << 8U) | byte;
    }
    return result;
}

std::uint64_t unsigned_from_little_endian(const std::string& bytes) {
    std::uint64_t result = 0U;
    for (std::size_t index = bytes.size(); index-- > 0U;) {
        result = (result << 8U) | static_cast<unsigned char>(bytes[index]);
    }
    return result;
}

std::int64_t signed_from_twos_complement(const std::uint64_t value, const int bits) {
    if (bits == 64) {
        if (value <= static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())) {
            return static_cast<std::int64_t>(value);
        }
        return -1 - static_cast<std::int64_t>(std::numeric_limits<std::uint64_t>::max() - value);
    }
    const std::uint64_t sign_bit = std::uint64_t{1} << (bits - 1);
    if ((value & sign_bit) == 0U) {
        return static_cast<std::int64_t>(value);
    }
    const std::uint64_t mask = (std::uint64_t{1} << bits) - 1U;
    return -1 - static_cast<std::int64_t>(mask - value);
}

template <typename Floating, typename Unsigned>
Unsigned floating_bits(const Floating value) {
    static_assert(sizeof(Floating) == sizeof(Unsigned));
    Unsigned bits = 0;
    std::memcpy(&bits, &value, sizeof(bits));
    return bits;
}

template <typename Floating, typename Unsigned>
Floating floating_from_bits(const Unsigned bits) {
    static_assert(sizeof(Floating) == sizeof(Unsigned));
    Floating value = 0;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}

}  // namespace

PrgValue bintoc_value(const PrgValue& value, const PrgValue* selector) {
    if (value.is_null || (selector != nullptr && selector->is_null)) {
        return make_null_value();
    }
    if (!is_numeric_value(value)) {
        throw_invalid_binary_conversion();
    }
    BinarySelector parsed = parse_bintoc_selector(selector);
    std::string result;
    if (parsed.encoding == BinaryEncoding::integer_1 || parsed.encoding == BinaryEncoding::integer_2 ||
        parsed.encoding == BinaryEncoding::integer_4) {
        const std::size_t width = encoding_width(parsed.encoding);
        const auto converted = signed_integer_argument(value, static_cast<int>(width * 8U));
        if (!converted.has_value()) {
            throw_invalid_binary_conversion();
        }
        result = big_endian_bytes(static_cast<std::uint64_t>(*converted), width);
        if (!parsed.suppress_sign_transform) {
            result[0] = static_cast<char>(static_cast<unsigned char>(result[0]) ^ 0x80U);
        }
    } else if (parsed.encoding == BinaryEncoding::native_float) {
        const double numeric = value_as_number(value);
        if (!std::isfinite(numeric) ||
            numeric < static_cast<double>(std::numeric_limits<float>::lowest()) ||
            numeric > static_cast<double>(std::numeric_limits<float>::max())) {
            throw_invalid_binary_conversion();
        }
        const float converted = static_cast<float>(numeric);
        result = little_endian_bytes(floating_bits<float, std::uint32_t>(converted), 4U);
    } else if (parsed.encoding == BinaryEncoding::native_double) {
        const double converted = value_as_number(value);
        if (!std::isfinite(converted)) {
            throw_invalid_binary_conversion();
        }
        result = little_endian_bytes(floating_bits<double, std::uint64_t>(converted), 8U);
    } else if (value.kind == PrgValueKind::currency) {
        result = big_endian_bytes(static_cast<std::uint64_t>(value.currency_value), 8U);
        result[0] = static_cast<char>(static_cast<unsigned char>(result[0]) ^ 0x80U);
    } else {
        const double converted = value_as_number(value);
        if (!std::isfinite(converted)) {
            throw_invalid_binary_conversion();
        }
        std::uint64_t bits = floating_bits<double, std::uint64_t>(converted);
        if ((bits & (std::uint64_t{1} << 63U)) != 0U) {
            bits = ~bits;
        } else {
            bits ^= std::uint64_t{1} << 63U;
        }
        result = big_endian_bytes(bits, 8U);
    }
    if (parsed.reverse) {
        std::reverse(result.begin(), result.end());
    }
    return make_string_value(std::move(result));
}

PrgValue ctobin_value(const PrgValue& value, const PrgValue* selector) {
    if (value.is_null || (selector != nullptr && selector->is_null)) {
        return make_null_value();
    }
    if (value.kind != PrgValueKind::string || value.is_object_reference) {
        throw_invalid_binary_conversion();
    }
    std::string bytes = value.string_value;
    const BinarySelector parsed = parse_ctobin_selector(selector, bytes.size());
    if (parsed.reverse) {
        std::reverse(bytes.begin(), bytes.end());
    }
    if (parsed.encoding == BinaryEncoding::integer_1 || parsed.encoding == BinaryEncoding::integer_2 ||
        parsed.encoding == BinaryEncoding::integer_4) {
        if (!parsed.suppress_sign_transform) {
            bytes[0] = static_cast<char>(static_cast<unsigned char>(bytes[0]) ^ 0x80U);
        }
        return make_number_value(static_cast<double>(signed_from_twos_complement(
            unsigned_from_big_endian(bytes), static_cast<int>(bytes.size() * 8U))));
    }
    if (parsed.encoding == BinaryEncoding::native_float) {
        const float converted = floating_from_bits<float, std::uint32_t>(
            static_cast<std::uint32_t>(unsigned_from_little_endian(bytes)));
        if (!std::isfinite(converted)) {
            throw_invalid_binary_conversion();
        }
        return make_number_value(static_cast<double>(converted));
    }
    if (parsed.encoding == BinaryEncoding::native_double) {
        const double converted = floating_from_bits<double, std::uint64_t>(unsigned_from_little_endian(bytes));
        if (!std::isfinite(converted)) {
            throw_invalid_binary_conversion();
        }
        return make_number_value(converted);
    }
    if (parsed.encoding == BinaryEncoding::currency) {
        if (!parsed.suppress_sign_transform) {
            bytes[0] = static_cast<char>(static_cast<unsigned char>(bytes[0]) ^ 0x80U);
        }
        return make_currency_value(signed_from_twos_complement(unsigned_from_big_endian(bytes), 64));
    }
    std::uint64_t bits = unsigned_from_big_endian(bytes);
    if (!parsed.suppress_sign_transform) {
        if ((bits & (std::uint64_t{1} << 63U)) != 0U) {
            bits ^= std::uint64_t{1} << 63U;
        } else {
            bits = ~bits;
        }
    }
    const double converted = floating_from_bits<double, std::uint64_t>(bits);
    if (!std::isfinite(converted)) {
        throw_invalid_binary_conversion();
    }
    return make_number_value(converted);
}

bool value_as_bool(const PrgValue& value) {
    switch (value.kind) {
        case PrgValueKind::boolean:
            return value.boolean_value;
        case PrgValueKind::number:
            return std::abs(value.number_value) > 0.000001;
        case PrgValueKind::string:
            return !value.string_value.empty();
        case PrgValueKind::int64:
            return value.int64_value != 0;
        case PrgValueKind::uint64:
            return value.uint64_value != 0U;
        case PrgValueKind::currency:
            return value.currency_value != 0;
        case PrgValueKind::empty:
            return false;
    }
    return false;
}

double value_as_number(const PrgValue& value) {
    switch (value.kind) {
        case PrgValueKind::boolean:
            return value.boolean_value ? 1.0 : 0.0;
        case PrgValueKind::number:
            return value.number_value;
        case PrgValueKind::string: {
            const std::string trimmed = trim_copy(value.string_value);
            if (trimmed.empty()) {
                return 0.0;
            }
            if (const auto parsed = try_parse_invariant_double(trimmed); parsed.has_value()) {
                return *parsed;
            }
            throw std::invalid_argument("invalid invariant numeric value");
        }
        case PrgValueKind::int64:
            return static_cast<double>(value.int64_value);
        case PrgValueKind::uint64:
            return static_cast<double>(value.uint64_value);
        case PrgValueKind::currency:
            return static_cast<double>(value.currency_value) / 10000.0;
        case PrgValueKind::empty:
            return 0.0;
    }
    return 0.0;
}

std::string value_as_string(const PrgValue& value) {
    switch (value.kind) {
        case PrgValueKind::boolean:
            return value.boolean_value ? "true" : "false";
        case PrgValueKind::number: {
            std::ostringstream stream;
            stream.imbue(std::locale::classic());
            if (std::abs(value.number_value - std::round(value.number_value)) < 0.000001) {
                stream << std::llround(value.number_value);
            } else {
                stream << value.number_value;
            }
            return stream.str();
        }
        case PrgValueKind::string:
            return value.string_value;
        case PrgValueKind::int64:
            return std::to_string(value.int64_value);
        case PrgValueKind::uint64:
            return std::to_string(value.uint64_value);
        case PrgValueKind::currency: {
            const bool negative = value.currency_value < 0;
            const std::uint64_t magnitude = negative
                                                ? static_cast<std::uint64_t>(-(value.currency_value + 1)) + 1U
                                                : static_cast<std::uint64_t>(value.currency_value);
            const std::uint64_t whole = magnitude / 10000U;
            const std::uint64_t fraction = magnitude % 10000U;
            std::ostringstream stream;
            stream.imbue(std::locale::classic());
            if (negative) {
                stream << '-';
            }
            stream << whole << '.' << std::setw(4) << std::setfill('0') << fraction;
            return stream.str();
        }
        case PrgValueKind::empty:
            return {};
    }
    return {};
}

int date_to_julian(int year, int month, int day) {
    // Astronomical Julian Day Number (Fliegel-Van Flandern), the value a VFP DateTime field stores in the first four
    // bytes of its eight (installed VFP9 stores {^2019-12-31} as 2458849, #6757). An earlier version subtracted 702
    // "to match tests" and made DateTime columns 702 days off from VFP9 in both directions.
    return ((1461 * (year + 4800 + (month - 14) / 12)) / 4
         + (367 * (month - 2 - 12 * ((month - 14) / 12))) / 12
         - (3 * ((year + 4900 + (month - 14) / 12) / 100)) / 4
         + day - 32075);
}

int stored_millis_to_seconds_of_day(int& julian_day, const long long millis) {
    // VFP9 stores the time of day with a stray millisecond (03:04:05 is 11044999) and shows it rounded to the nearest
    // second, half up, carrying into the next day (86399999 is the next midnight; probe result15.txt, #6757).
    const long long total_seconds = (millis + 500LL) / 1000LL;
    const long long carry = total_seconds / 86400LL;
    // A corrupt table can hold the largest day number; saturate so the caller's date-range check rejects it instead of
    // the addition overflowing.
    julian_day = julian_day > std::numeric_limits<int>::max() - carry
                     ? std::numeric_limits<int>::max()
                     : julian_day + static_cast<int>(carry);
    return static_cast<int>(total_seconds % 86400LL);
}

void julian_to_date(int julian, int& year, int& month, int& day) {
    int l = julian + 68569;
    int n = (4 * l) / 146097;
    l = l - (146097 * n + 3) / 4;
    int i = (4000 * (l + 1)) / 1461001;
    l = l - (1461 * i) / 4 + 31;
    int j = (80 * l) / 2447;
    day = l - (2447 * j) / 80;
    l = j / 11;
    month = j + 2 - (12 * l);
    year = 100 * (n - 49) + i + l;
}

bool julian_to_runtime_date(int julian, int& year, int& month, int& day) {
    static const int kMinimumJulian = date_to_julian(1, 1, 1);
    static const int kMaximumJulian = date_to_julian(9999, 12, 31);
    if (julian < kMinimumJulian || julian > kMaximumJulian) {
        return false;
    }

    julian_to_date(julian, year, month, day);
    const int max_day = days_in_month(year, month);
    return max_day > 0 && day >= 1 && day <= max_day;
}

std::size_t portable_path_separator_position(const std::string& path) {
    const std::size_t slash = path.find_last_of('/');
    const std::size_t backslash = path.find_last_of('\\');
    if (slash == std::string::npos) {
        return backslash;
    }
    if (backslash == std::string::npos) {
        return slash;
    }
    return std::max(slash, backslash);
}

std::string portable_path_drive(const std::string& path) {
    if (path.size() >= 2U && std::isalpha(static_cast<unsigned char>(path[0])) != 0 && path[1] == ':') {
        return path.substr(0U, 2U);
    }
    const auto is_separator = [](const char ch) {
        return ch == '\\' || ch == '/';
    };
    const bool is_namespace_path =
        path.size() >= 4U &&
        is_separator(path[0]) &&
        path[1] == path[0] &&
        (path[2] == '?' || path[2] == '.') &&
        is_separator(path[3]);
    if (is_namespace_path) {
        const bool is_extended_unc =
            path[2] == '?' &&
            path.size() >= 8U &&
            lowercase_copy(path.substr(4U, 3U)) == "unc" &&
            is_separator(path[7]);
        if (is_extended_unc) {
            const std::size_t server_start = 8U;
            const std::size_t server_end = path.find_first_of("\\/", server_start);
            if (server_end != std::string::npos && server_end > server_start) {
                const std::size_t share_start = server_end + 1U;
                if (share_start < path.size()) {
                    const std::size_t share_end = path.find_first_of("\\/", share_start);
                    if (share_end == std::string::npos) {
                        return path;
                    }
                    if (share_end > share_start) {
                        return path.substr(0U, share_end);
                    }
                }
            }
            return {};
        }
        if (path.size() >= 6U &&
            std::isalpha(static_cast<unsigned char>(path[4])) != 0 &&
            path[5] == ':') {
            return path.substr(0U, 6U);
        }
        return {};
    }
    if (path.size() >= 2U && (path[0] == '\\' || path[0] == '/') && path[1] == path[0]) {
        const std::size_t server_end = path.find_first_of("\\/", 2U);
        if (server_end != std::string::npos) {
            const std::size_t share_end = path.find_first_of("\\/", server_end + 1U);
            if (share_end != std::string::npos) {
                return path.substr(0U, share_end);
            }
            if (server_end > 2U && server_end + 1U < path.size()) {
                return path;
            }
        }
    }
    return {};
}

std::string portable_path_parent(const std::string& path) {
    const std::size_t separator = portable_path_separator_position(path);
    if (separator == std::string::npos) {
        return {};
    }
    if (separator == 0U) {
        return path.substr(0U, 1U);
    }
    if (separator == 2U && path.size() >= 3U && path[1] == ':') {
        return path.substr(0U, 3U);
    }
    return path.substr(0U, separator);
}

std::string portable_path_filename(const std::string& path) {
    const std::size_t separator = portable_path_separator_position(path);
    return separator == std::string::npos ? path : path.substr(separator + 1U);
}

std::string portable_path_extension(const std::string& path) {
    const std::string filename = portable_path_filename(path);
    const std::size_t dot = filename.find_last_of('.');
    return dot == std::string::npos || dot == 0U ? std::string{} : filename.substr(dot + 1U);
}

std::string portable_path_stem(const std::string& path) {
    const std::string filename = portable_path_filename(path);
    const std::size_t dot = filename.find_last_of('.');
    return dot == std::string::npos || dot == 0U ? filename : filename.substr(0U, dot);
}

std::string portable_force_extension(const std::string& path, std::string extension) {
    if (!extension.empty() && extension[0] == '.') {
        extension.erase(extension.begin());
    }
    const std::size_t separator = portable_path_separator_position(path);
    const std::size_t filename_start = separator == std::string::npos ? 0U : separator + 1U;
    const std::size_t dot = path.find_last_of('.');
    const bool has_extension = dot != std::string::npos && dot >= filename_start && dot != filename_start;
    const std::string stem_path = has_extension ? path.substr(0U, dot) : path;
    return extension.empty() ? stem_path : stem_path + "." + extension;
}

std::string portable_force_path(const std::string& path, std::string directory) {
    const std::string filename = portable_path_filename(path);
    if (directory.empty()) {
        return filename;
    }

    const bool drive_path =
        directory.size() >= 2U &&
        std::isalpha(static_cast<unsigned char>(directory[0])) != 0 &&
        directory[1] == ':';
    const bool unc_path =
        directory.size() >= 2U &&
        (directory[0] == '\\' || directory[0] == '/') &&
        directory[1] == directory[0];
    char separator = '/';
    if (drive_path || unc_path) {
        separator = '\\';
    } else if (directory.front() != '/') {
        const std::size_t first_separator = directory.find_first_of("\\/");
        if (first_separator != std::string::npos) {
            separator = directory[first_separator];
        }
    }

    const char alternate_separator = separator == '\\' ? '/' : '\\';
    std::replace(directory.begin(), directory.end(), alternate_separator, separator);
    if (directory.back() != separator) {
        directory += separator;
    }
    return directory + filename;
}

std::tm local_time_from_time_t(std::time_t raw_time) {
    std::tm local{};
#if defined(_WIN32)
    localtime_s(&local, &raw_time);
#else
    localtime_r(&raw_time, &local);
#endif
    return local;
}

bool is_leap_year(int year) {
    if ((year % 400) == 0) {
        return true;
    }
    if ((year % 100) == 0) {
        return false;
    }
    return (year % 4) == 0;
}

int days_in_month(int year, int month) {
    static constexpr std::array<int, 12U> kDays = {
        31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month < 1 || month > 12) {
        return 0;
    }
    if (month == 2 && is_leap_year(year)) {
        return 29;
    }
    return kDays[static_cast<std::size_t>(month - 1)];
}

bool parse_runtime_date_string(const std::string& raw, int& year, int& month, int& day) {
    const std::string value = trim_copy(raw);
    if (value.empty()) {
        return false;
    }

    const auto first_slash = value.find('/');
    if (first_slash != std::string::npos) {
        const auto second_slash = value.find('/', first_slash + 1U);
        if (second_slash == std::string::npos) {
            return false;
        }
        try {
            month = std::stoi(value.substr(0U, first_slash));
            day = std::stoi(value.substr(first_slash + 1U, second_slash - first_slash - 1U));
            std::size_t year_end = second_slash + 1U;
            while (year_end < value.size() && std::isdigit(static_cast<unsigned char>(value[year_end])) != 0) {
                ++year_end;
            }
            if (year_end != value.size()) {
                return false;
            }
            year = std::stoi(value.substr(second_slash + 1U, year_end - second_slash - 1U));
        } catch (...) {
            return false;
        }
    } else {
        if (value.size() != 8U) {
            return false;
        }
        for (std::size_t index = 0U; index < 8U; ++index) {
            if (std::isdigit(static_cast<unsigned char>(value[index])) == 0) {
                return false;
            }
        }
        try {
            year = std::stoi(value.substr(0U, 4U));
            month = std::stoi(value.substr(4U, 2U));
            day = std::stoi(value.substr(6U, 2U));
        } catch (...) {
            return false;
        }
    }

    if (year <= 0 || month < 1 || month > 12) {
        return false;
    }
    const int max_day = days_in_month(year, month);
    if (day < 1 || day > max_day) {
        return false;
    }
    return true;
}

std::string format_runtime_date_string(int year, int month, int day) {
    std::ostringstream stream;
    stream.imbue(std::locale::classic());
    stream << std::setfill('0')
           << std::setw(2) << month << '/'
           << std::setw(2) << day << '/'
           << std::setw(4) << year;
    return stream.str();
}

bool parse_runtime_time_string(const std::string& raw, int& hour, int& minute, int& second) {
    const std::string value = trim_copy(raw);
    if (value.empty()) {
        return false;
    }

    const auto first_colon = value.find(':');
    const auto second_colon = first_colon == std::string::npos ? std::string::npos : value.find(':', first_colon + 1U);
    if (first_colon == std::string::npos || second_colon == std::string::npos) {
        return false;
    }

    if (value.find('/') != std::string::npos || value.find(' ') != std::string::npos || value.find('T') != std::string::npos) {
        return false;
    }

    const auto parse_component = [&](const std::size_t start, const std::size_t end, int& out) -> bool {
        if (end <= start) {
            return false;
        }
        for (std::size_t index = start; index < end; ++index) {
            if (std::isdigit(static_cast<unsigned char>(value[index])) == 0) {
                return false;
            }
        }
        try {
            out = std::stoi(value.substr(start, end - start));
        } catch (...) {
            return false;
        }
        return true;
    };

    if (!parse_component(0U, first_colon, hour) ||
        !parse_component(first_colon + 1U, second_colon, minute) ||
        !parse_component(second_colon + 1U, value.size(), second)) {
        return false;
    }

    return hour >= 0 && hour <= 23 && minute >= 0 && minute <= 59 && second >= 0 && second <= 59;
}

bool parse_runtime_datetime_string(
    const std::string& raw,
    int& year,
    int& month,
    int& day,
    int& hour,
    int& minute,
    int& second) {
    const std::string value = trim_copy(raw);
    if (value.empty()) {
        return false;
    }

    const auto separator = value.find_first_of(" T");
    const std::string date_part = separator == std::string::npos ? value : value.substr(0U, separator);
    const std::string time_part = separator == std::string::npos ? std::string{} : trim_copy(value.substr(separator + 1U));

    if (!parse_runtime_date_string(date_part, year, month, day)) {
        return false;
    }

    hour = 0;
    minute = 0;
    second = 0;
    if (!time_part.empty() && !parse_runtime_time_string(time_part, hour, minute, second)) {
        return false;
    }

    return true;
}

std::string format_runtime_datetime_string(int year, int month, int day, int hour, int minute, int second) {
    std::ostringstream stream;
    stream.imbue(std::locale::classic());
    stream << std::setfill('0')
           << std::setw(2) << month << '/'
           << std::setw(2) << day << '/'
           << std::setw(4) << year << ' '
           << std::setw(2) << hour << ':'
           << std::setw(2) << minute << ':'
           << std::setw(2) << second;
    return stream.str();
}

int weekday_number_sunday_first(int year, int month, int day) {
    std::tm local_tm{};
    local_tm.tm_year = year - 1900;
    local_tm.tm_mon = month - 1;
    local_tm.tm_mday = day;
    local_tm.tm_hour = 12;
    local_tm.tm_min = 0;
    local_tm.tm_sec = 0;
    const std::time_t converted = std::mktime(&local_tm);
    if (converted == static_cast<std::time_t>(-1)) {
        return 0;
    }

    std::tm normalized{};
#if defined(_WIN32)
    localtime_s(&normalized, &converted);
#else
    localtime_r(&converted, &normalized);
#endif
    return normalized.tm_wday + 1;
}

std::vector<std::string> split_text_lines(const std::string& contents)
{
    std::vector<std::string> lines;
    std::string current;
    for (std::size_t index = 0U; index < contents.size(); ++index)
    {
        const char ch = contents[index];
        if (ch == '\r' || ch == '\n')
        {
            lines.push_back(current);
            current.clear();
            if (ch == '\r' && index + 1U < contents.size() && contents[index + 1U] == '\n')
            {
                ++index;
            }
            continue;
        }
        current.push_back(ch);
    }
    if (!current.empty() || (!contents.empty() && contents.back() != '\r' && contents.back() != '\n'))
    {
        lines.push_back(current);
    }
    return lines;
}

}  // namespace copperfin::runtime
