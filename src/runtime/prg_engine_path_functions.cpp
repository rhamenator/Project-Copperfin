// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "prg_engine_path_functions.h"

#include "copperfin/platform/path.h"
#include "prg_engine_helpers.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <utility>
#include <vector>

namespace copperfin::runtime {

namespace {

bool is_windows_drive_absolute_path(const std::string& value) {
    return value.size() >= 3U &&
        std::isalpha(static_cast<unsigned char>(value[0])) != 0 &&
        value[1] == ':' &&
        (value[2] == '\\' || value[2] == '/');
}

bool is_unc_path(const std::string& value) {
    return value.size() >= 2U &&
        ((value[0] == '\\' && value[1] == '\\') || (value[0] == '/' && value[1] == '/'));
}

bool is_posix_absolute_path(const std::string& value) {
    return !value.empty() && value.front() == '/';
}

std::string normalize_posix_absolute_path(std::string value) {
    std::replace(value.begin(), value.end(), '\\', '/');

    // POSIX-rooted VFP paths are a portable syntax contract. Do not pass
    // them through the host path parser, which treats a leading slash as a
    // root-relative Windows path when this code runs on Windows.
    std::vector<std::string> components;
    std::size_t start = 0U;
    while (start < value.size()) {
        const std::size_t separator = value.find('/', start);
        const std::size_t end = separator == std::string::npos ? value.size() : separator;
        const std::string component = value.substr(start, end - start);
        if (!component.empty() && component != ".") {
            if (component == "..") {
                if (!components.empty()) {
                    components.pop_back();
                }
            } else {
                components.push_back(component);
            }
        }
        if (separator == std::string::npos) {
            break;
        }
        start = separator + 1U;
    }

    std::string normalized = "/";
    for (std::size_t index = 0U; index < components.size(); ++index) {
        if (index != 0U) {
            normalized.push_back('/');
        }
        normalized += components[index];
    }
    return normalized;
}

std::vector<std::string> split_windows_path_components(const std::string& value, std::size_t start) {
    std::vector<std::string> components;
    while (start < value.size()) {
        const std::size_t separator = value.find('\\', start);
        const std::size_t end = separator == std::string::npos ? value.size() : separator;
        if (end > start) {
            components.push_back(value.substr(start, end - start));
        }
        if (separator == std::string::npos) {
            break;
        }
        start = separator + 1U;
    }
    return components;
}

void normalize_windows_tail(std::vector<std::string>& components, std::size_t root_component_count) {
    std::vector<std::string> normalized;
    normalized.reserve(components.size());
    const std::size_t locked_count = std::min(root_component_count, components.size());
    normalized.insert(normalized.end(), components.begin(), components.begin() + locked_count);
    for (std::size_t index = locked_count; index < components.size(); ++index) {
        const std::string& component = components[index];
        if (component == ".") {
            continue;
        }
        if (component == "..") {
            if (normalized.size() > locked_count) {
                normalized.pop_back();
            }
            continue;
        }
        normalized.push_back(component);
    }
    components = std::move(normalized);
}

std::string normalize_windows_absolute_path(std::string value) {
    const std::string original = value;
    std::replace(value.begin(), value.end(), '/', '\\');
    if (is_windows_drive_absolute_path(value)) {
        std::vector<std::string> components = split_windows_path_components(value, 3U);
        normalize_windows_tail(components, 0U);

        std::string result = value.substr(0U, 3U);
        for (std::size_t index = 0U; index < components.size(); ++index) {
            if (index != 0U) {
                result.push_back('\\');
            }
            result += components[index];
        }
        return result;
    }

    const bool device_namespace = value.size() >= 4U &&
        value[0] == '\\' && value[1] == '\\' &&
        (value[2] == '?' || value[2] == '.') && value[3] == '\\';
    if (device_namespace) {
        const bool extended_unc = value.size() >= 8U && uppercase_copy(value.substr(0U, 8U)) == "\\\\?\\UNC\\";
        const bool extended_drive = value.size() >= 7U && value[2] == '?' &&
            std::isalpha(static_cast<unsigned char>(value[4])) != 0 &&
            value[5] == ':' && value[6] == '\\';
        if (!extended_unc && !extended_drive) {
            return original;
        }
    }

    std::vector<std::string> components = split_windows_path_components(value, 2U);
    std::size_t root_component_count = 2U;
    if (components.size() >= 2U && components[0] == "?" && uppercase_copy(components[1]) == "UNC") {
        root_component_count = 4U;
    }
    normalize_windows_tail(components, root_component_count);

    std::string result = "\\\\";
    for (std::size_t index = 0U; index < components.size(); ++index) {
        if (index != 0U) {
            result.push_back('\\');
        }
        result += components[index];
    }
    return result;
}

std::string normalize_relative_path_separators(std::string value) {
    std::replace(
        value.begin(),
        value.end(),
        '\\',
        static_cast<char>(std::filesystem::path::preferred_separator));
    return value;
}

std::string portable_full_path(const std::string& raw_path, const std::string& default_directory) {
    if (is_windows_drive_absolute_path(raw_path) || is_unc_path(raw_path)) {
        return normalize_windows_absolute_path(raw_path);
    }
    if (is_posix_absolute_path(raw_path)) {
        return normalize_posix_absolute_path(raw_path);
    }

    std::filesystem::path path = copperfin::platform::path_from_utf8_string(
        normalize_relative_path_separators(raw_path));
    if (path.is_relative()) {
        path = copperfin::platform::path_from_utf8_string(default_directory) / path;
    }
    std::error_code absolute_error;
    const std::filesystem::path absolute_path = std::filesystem::absolute(path, absolute_error);
    if (absolute_error) {
        return copperfin::platform::path_to_utf8_string(path.lexically_normal());
    }
    return copperfin::platform::path_to_utf8_string(absolute_path.lexically_normal());
}

}  // namespace

std::optional<PrgValue> evaluate_path_function(
    const std::string& function,
    const std::vector<PrgValue>& arguments,
    const std::string& default_directory) {
    if (function == "fullpath" && !arguments.empty()) {
        return make_string_value(portable_full_path(value_as_string(arguments[0]), default_directory));
    }
    if (function == "curdir") {
        return make_string_value(default_directory);
    }
    if (function == "justfname" && !arguments.empty()) {
        return make_string_value(portable_path_filename(value_as_string(arguments[0])));
    }
    if (function == "justpath" && !arguments.empty()) {
        return make_string_value(portable_path_parent(value_as_string(arguments[0])));
    }
    if (function == "juststem" && !arguments.empty()) {
        return make_string_value(portable_path_stem(value_as_string(arguments[0])));
    }
    if (function == "justext" && !arguments.empty()) {
        return make_string_value(portable_path_extension(value_as_string(arguments[0])));
    }
    if (function == "justdrive" && !arguments.empty()) {
        return make_string_value(portable_path_drive(value_as_string(arguments[0])));
    }
    if (function == "forceext" && arguments.size() >= 2U) {
        return make_string_value(portable_force_extension(value_as_string(arguments[0]), value_as_string(arguments[1])));
    }
    if (function == "forcepath" && arguments.size() >= 2U) {
        return make_string_value(portable_force_path(value_as_string(arguments[0]), value_as_string(arguments[1])));
    }
    if (function == "defaultext" && arguments.size() >= 2U) {
        const std::string path = value_as_string(arguments[0]);
        if (!portable_path_extension(path).empty()) {
            return make_string_value(path);
        }
        return make_string_value(portable_force_extension(path, value_as_string(arguments[1])));
    }
    if (function == "addbs" && !arguments.empty()) {
        std::string path = value_as_string(arguments[0]);
        if (!path.empty() && path.back() != '\\' && path.back() != '/') {
            path += '\\';
        }
        return make_string_value(std::move(path));
    }

    return std::nullopt;
}

}  // namespace copperfin::runtime
