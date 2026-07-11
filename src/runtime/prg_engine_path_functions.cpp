// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "prg_engine_path_functions.h"

#include "prg_engine_helpers.h"

#include <algorithm>
#include <cctype>
#include <filesystem>

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
    return std::filesystem::path(value).lexically_normal().generic_string();
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
        return raw_path;
    }
    if (is_posix_absolute_path(raw_path)) {
        return normalize_posix_absolute_path(raw_path);
    }

    std::filesystem::path path(normalize_relative_path_separators(raw_path));
    if (path.is_relative()) {
        path = std::filesystem::path(default_directory) / path;
    }
    return std::filesystem::absolute(path).lexically_normal().string();
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
