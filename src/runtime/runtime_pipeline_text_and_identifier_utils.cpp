// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "runtime_pipeline_support.h"

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace copperfin::runtime {

namespace runtime_pipeline_detail {

std::string sanitize_file_name(const std::string& value) {
    std::string sanitized;
    sanitized.reserve(value.size());
    for (const char ch : value) {
        if ((ch >= 'A' && ch <= 'Z') ||
            (ch >= 'a' && ch <= 'z') ||
            (ch >= '0' && ch <= '9') ||
            ch == '-' || ch == '_') {
            sanitized.push_back(ch);
        } else {
            sanitized.push_back('_');
        }
    }
    return sanitized.empty() ? "copperfin_app" : sanitized;
}

std::string trim_copy(std::string value) {
    const auto is_space = [](unsigned char ch) {
        return std::isspace(ch) != 0;
    };
    value.erase(value.begin(), std::find_if(value.begin(), value.end(), [&](unsigned char ch) {
        return !is_space(ch);
    }));
    while (!value.empty() && is_space(static_cast<unsigned char>(value.back()))) {
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

std::string canonical_casefolded_path_identity(const std::filesystem::path& path) {
    std::filesystem::path identity_path = path.lexically_normal();
#if defined(_WIN32)
    std::error_code identity_path_error;
    const std::filesystem::path absolute_identity_path =
        std::filesystem::absolute(identity_path, identity_path_error).lexically_normal();
    if (!identity_path_error) {
        const std::filesystem::path weak_identity_path =
            std::filesystem::weakly_canonical(absolute_identity_path, identity_path_error);
        identity_path = identity_path_error ? absolute_identity_path : weak_identity_path;
    }
    std::wstring case_folded_path = identity_path.native();
    if (!case_folded_path.empty()) {
        (void)::CharLowerBuffW(
            case_folded_path.data(),
            static_cast<DWORD>(case_folded_path.size()));
    }
    return copperfin::platform::path_to_utf8_string(std::filesystem::path(case_folded_path));
#else
    return copperfin::platform::path_to_utf8_string(identity_path);
#endif
}

std::string quote_manifest_value(const std::string& value) {
    std::string escaped;
    escaped.reserve(value.size());
    for (const char ch : value) {
        if (ch == '\\') {
            escaped += "\\\\";
        } else if (ch == '\n') {
            escaped += "\\n";
        } else if (ch == '\r') {
            escaped += "\\r";
        } else if (ch == '|') {
            escaped += "\\|";
        } else {
            escaped.push_back(ch);
        }
    }
    return escaped;
}

std::vector<std::string> unique_non_empty_paths_preserve_order(std::initializer_list<std::string> values) {
    std::vector<std::string> normalized_values;
    normalized_values.reserve(values.size());
    std::unordered_set<std::string> seen;
    for (const std::string& value : values) {
        if (value.empty()) {
            continue;
        }
        const std::string normalized = copperfin::platform::path_to_utf8_string(
            copperfin::platform::path_from_utf8_string(value).lexically_normal());
        if (normalized.empty() || !seen.insert(normalized).second) {
            continue;
        }
        normalized_values.push_back(normalized);
    }
    return normalized_values;
}

std::string normalize_export_symbol(std::string value) {
    value = trim_copy(std::move(value));
    const std::size_t whitespace = value.find_first_of(" \t(");
    if (whitespace != std::string::npos) {
        value = trim_copy(value.substr(0U, whitespace));
    }
    return value;
}

std::string json_escape(std::string_view value) {
    return copperfin::platform::json_escape_string(value);
}

std::string extract_declared_parameter_name(const std::string& raw_name) {
    std::string parameter_name = trim_copy(raw_name);
    if (!parameter_name.empty() && parameter_name.front() == '@') {
        parameter_name.erase(parameter_name.begin());
    }
    const std::size_t equals = parameter_name.find('=');
    if (equals != std::string::npos) {
        parameter_name = trim_copy(parameter_name.substr(0U, equals));
    }
    return parameter_name;
}

std::string sanitize_cpp_identifier(const std::string& value, const std::size_t fallback_index) {
    std::string sanitized;
    sanitized.reserve(value.size() + 8U);
    for (const char ch : value) {
        if ((ch >= 'A' && ch <= 'Z') ||
            (ch >= 'a' && ch <= 'z') ||
            (ch >= '0' && ch <= '9') ||
            ch == '_') {
            sanitized.push_back(ch);
        } else {
            sanitized.push_back('_');
        }
    }

    if (sanitized.empty()) {
        sanitized = "arg" + std::to_string(fallback_index + 1U);
    }
    if ((sanitized.front() >= '0' && sanitized.front() <= '9')) {
        sanitized.insert(sanitized.begin(), '_');
    }
    return sanitized;
}

std::string sanitize_csharp_identifier(std::string value, const std::string& fallback) {
    value = trim_copy(std::move(value));
    if (value.empty()) {
        return fallback;
    }

    std::string sanitized;
    sanitized.reserve(value.size());
    for (std::size_t index = 0; index < value.size(); ++index) {
        const unsigned char ch = static_cast<unsigned char>(value[index]);
        const bool valid =
            std::isalnum(ch) != 0 ||
            ch == '_';
        if (!valid) {
            sanitized.push_back('_');
            continue;
        }
        if (sanitized.empty() && std::isdigit(ch) != 0) {
            sanitized.push_back('_');
        }
        sanitized.push_back(static_cast<char>(ch));
    }
    return sanitized.empty() ? fallback : sanitized;
}

std::string sanitize_csharp_routine_identifier(std::string value, const std::string& fallback) {
    std::string sanitized = sanitize_csharp_identifier(std::move(value), fallback);
    for (char& ch : sanitized) {
        if (std::isalpha(static_cast<unsigned char>(ch)) != 0) {
            ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
            break;
        }
    }
    return sanitized;
}

std::string unquote_literal(std::string value) {
    value = trim_copy(std::move(value));
    if (value.size() >= 2U &&
        ((value.front() == '\'' && value.back() == '\'') ||
         (value.front() == '"' && value.back() == '"'))) {
        value = value.substr(1U, value.size() - 2U);
    }
    return value;
}

std::string sanitize_csharp_compound_identifier(std::string value, const std::string& fallback) {
    value = trim_copy(std::move(value));
    if (value.empty()) {
        return fallback;
    }

    std::string sanitized;
    sanitized.reserve(value.size());
    bool capitalize_next = true;
    for (const char raw_ch : value) {
        const unsigned char ch = static_cast<unsigned char>(raw_ch);
        if (std::isalnum(ch) != 0) {
            char output = static_cast<char>(ch);
            if (capitalize_next && std::isalpha(ch) != 0) {
                output = static_cast<char>(std::toupper(ch));
            }
            if (sanitized.empty() && std::isdigit(ch) != 0) {
                sanitized.push_back('_');
            }
            sanitized.push_back(output);
            capitalize_next = false;
            continue;
        }

        if (!sanitized.empty() && sanitized.back() != '_') {
            sanitized.push_back('_');
        }
        capitalize_next = true;
    }

    while (!sanitized.empty() && sanitized.back() == '_') {
        sanitized.pop_back();
    }
    return sanitized.empty() ? fallback : sanitized;
}

std::string join_strings(const std::vector<std::string>& values) {
    std::ostringstream stream;
    for (std::size_t index = 0; index < values.size(); ++index) {
        stream << values[index];
        if ((index + 1U) != values.size()) {
            stream << ";";
        }
    }
    return stream.str();
}

}  // namespace runtime_pipeline_detail

}  // namespace copperfin::runtime
