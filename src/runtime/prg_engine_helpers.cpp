#include "prg_engine_helpers.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <functional>
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

        return is_supported_unary("UPPER") ||
            is_supported_unary("LOWER") ||
            is_supported_unary("ALLTRIM") ||
            is_supported_unary("LTRIM") ||
            is_supported_unary("RTRIM") ||
            is_supported_binary_with_count("LEFT") ||
            is_supported_binary_with_count("RIGHT") ||
            is_supported_substr();
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

bool value_as_bool(const PrgValue& value) {
    switch (value.kind) {
        case PrgValueKind::boolean:
            return value.boolean_value;
        case PrgValueKind::number:
            return std::abs(value.number_value) > 0.000001;
        case PrgValueKind::string:
            return !value.string_value.empty();
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
        case PrgValueKind::empty:
            return {};
    }
    return {};
}

}  // namespace copperfin::runtime
