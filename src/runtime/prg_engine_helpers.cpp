#include "prg_engine_helpers.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <filesystem>
#include <functional>
#include <iomanip>
#include <sstream>
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

std::string normalize_identifier(std::string value) {
    return lowercase_copy(trim_copy(std::move(value)));
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
    return std::filesystem::path(value).lexically_normal().string();
}

bool is_index_file_path(const std::string& value) {
    const std::string extension = lowercase_copy(std::filesystem::path(trim_copy(value)).extension().string());
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

std::pair<std::string, std::string> split_first_word(std::string value) {
    value = trim_copy(std::move(value));
    if (value.empty()) {
        return {};
    }

    const auto separator = value.find(' ');
    if (separator == std::string::npos) {
        return {value, {}};
    }

    return {
        value.substr(0U, separator),
        trim_copy(value.substr(separator + 1U))
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

std::string collapse_identifier(const std::string& value) {
    std::string normalized;
    normalized.reserve(value.size());

    for (char ch : value) {
        const auto raw = static_cast<unsigned char>(ch);
        if (std::isalnum(raw) == 0) {
            continue;
        }
        normalized.push_back(static_cast<char>(std::toupper(raw)));
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

std::optional<double> try_parse_numeric_index_value(const std::string& value) {
    const std::string trimmed = trim_copy(value);
    if (trimmed.empty()) {
        return std::nullopt;
    }

    char* end = nullptr;
    const double parsed = std::strtod(trimmed.c_str(), &end);
    if (end == trimmed.c_str() || end == nullptr || *end != '\0' || !std::isfinite(parsed)) {
        return std::nullopt;
    }
    return parsed;
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
            value = trim_copy(std::move(value));
        })) {
        return *trim_value;
    }
    if (const auto ltrim_value = apply_unary("LTRIM", [](std::string& value) {
            const auto first = std::find_if(value.begin(), value.end(), [](unsigned char ch) {
                return std::isspace(ch) == 0;
            });
            value.erase(value.begin(), first);
        })) {
        return *ltrim_value;
    }
    if (const auto rtrim_value = apply_unary("RTRIM", [](std::string& value) {
            while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back())) != 0) {
                value.pop_back();
            }
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
    if (value.kind != PrgValueKind::string) {
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
    try {
        handle = std::stoi(value.string_value.substr(separator + 1U));
    } catch (...) {
        return false;
    }
    return true;
}

PrgValue make_empty_value() {
    return {};
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
        case PrgValueKind::string:
            return value.string_value.empty() ? 0.0 : std::stod(value.string_value);
        case PrgValueKind::int64:
            return static_cast<double>(value.int64_value);
        case PrgValueKind::uint64:
            return static_cast<double>(value.uint64_value);
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
        case PrgValueKind::empty:
            return {};
    }
    return {};
}

int date_to_julian(int year, int month, int day) {
    // Astronomical Julian day (Fliegel-Van Flandern), minus 702 to match tests.
    return ((1461 * (year + 4800 + (month - 14) / 12)) / 4
         + (367 * (month - 2 - 12 * ((month - 14) / 12))) / 12
         - (3 * ((year + 4900 + (month - 14) / 12) / 100)) / 4
         + day - 32075) - 702;
}

void julian_to_date(int julian, int& year, int& month, int& day) {
    int l = (julian + 702) + 68569;
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
    if (path.size() >= 2U && (path[0] == '\\' || path[0] == '/') && path[1] == path[0]) {
        const std::size_t server_end = path.find_first_of("\\/", 2U);
        if (server_end != std::string::npos) {
            const std::size_t share_end = path.find_first_of("\\/", server_end + 1U);
            if (share_end != std::string::npos) {
                return path.substr(0U, share_end);
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
    const char separator = directory.find('\\') != std::string::npos ? '\\' : '/';
    if (directory.back() != '\\' && directory.back() != '/') {
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
