// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/environment.h"

#include "copperfin/platform/path.h"

#include <cstdlib>
#include <mutex>

namespace copperfin::platform {
namespace {

bool is_valid_environment_name(std::string_view name) {
    return !name.empty() && name.find('\0') == std::string_view::npos &&
        name.find('=') == std::string_view::npos;
}

bool contains_nul(std::string_view value) {
    return value.find('\0') != std::string_view::npos;
}

#if defined(_WIN32)
std::optional<std::wstring> widen_ascii_environment_name(std::string_view name) {
    if (!is_valid_environment_name(name)) {
        return std::nullopt;
    }

    std::wstring wide_name;
    wide_name.reserve(name.size());
    for (const unsigned char ch : name) {
        if (ch > 0x7FU) {
            return std::nullopt;
        }
        wide_name.push_back(static_cast<wchar_t>(ch));
    }
    return wide_name;
}

std::optional<std::wstring> widen_utf8_environment_value(std::string_view value) {
    if (contains_nul(value)) {
        return std::nullopt;
    }
    if (value.empty()) {
        return std::wstring{};
    }

    const std::wstring native = path_from_utf8_string(value).native();
    return native.empty() ? std::nullopt : std::optional<std::wstring>(native);
}

std::optional<std::string> narrow_utf8_environment_value(const wchar_t* value) {
    if (value == nullptr) {
        return std::nullopt;
    }
    if (*value == L'\0') {
        return std::string{};
    }

    const std::string utf8 = path_to_utf8_string(std::filesystem::path(value));
    return utf8.empty() ? std::nullopt : std::optional<std::string>(utf8);
}
#else
// This boundary coordinates only callers that use these helpers. Code that calls
// getenv, setenv, unsetenv, or putenv directly must provide the same external
// process-wide serialization before running concurrently with Copperfin access.
std::mutex& environment_mutex() {
    static std::mutex mutex;
    return mutex;
}
#endif

}  // namespace

std::optional<std::string> read_environment_variable(std::string_view name) {
    if (!is_valid_environment_name(name)) {
        return std::nullopt;
    }

#if defined(_WIN32)
    const auto wide_name = widen_ascii_environment_name(name);
    if (!wide_name.has_value()) {
        return std::nullopt;
    }

    wchar_t* raw = nullptr;
    std::size_t length = 0;
    if (_wdupenv_s(&raw, &length, wide_name->c_str()) != 0 || raw == nullptr) {
        return std::nullopt;
    }
    const auto value = narrow_utf8_environment_value(raw);
    std::free(raw);
    return value;
#else
    const std::string key(name);
    const std::lock_guard<std::mutex> lock(environment_mutex());
    if (const char* raw = std::getenv(key.c_str()); raw != nullptr) {
        return std::string(raw);
    }
    return std::nullopt;
#endif
}

std::string read_environment_variable_or_empty(std::string_view name) {
    const auto value = read_environment_variable(name);
    return value.has_value() ? *value : std::string{};
}

std::optional<std::filesystem::path> read_environment_path(std::string_view name) {
    if (!is_valid_environment_name(name)) {
        return std::nullopt;
    }

#if defined(_WIN32)
    const auto wide_name = widen_ascii_environment_name(name);
    if (!wide_name.has_value()) {
        return std::nullopt;
    }

    wchar_t* raw = nullptr;
    std::size_t length = 0;
    if (_wdupenv_s(&raw, &length, wide_name->c_str()) != 0 || raw == nullptr) {
        return std::nullopt;
    }
    std::filesystem::path value(raw);
    std::free(raw);
    return value;
#else
    const auto value = read_environment_variable(name);
    return value.has_value()
        ? std::optional<std::filesystem::path>(std::filesystem::path(*value))
        : std::nullopt;
#endif
}

bool write_environment_variable(std::string_view name, std::string_view value) {
    if (!is_valid_environment_name(name) || contains_nul(value)) {
        return false;
    }

#if defined(_WIN32)
    const auto wide_name = widen_ascii_environment_name(name);
    const auto wide_value = widen_utf8_environment_value(value);
    return wide_name.has_value() && wide_value.has_value() &&
        _wputenv_s(wide_name->c_str(), wide_value->c_str()) == 0;
#else
    const std::string key(name);
    const std::string assigned_value(value);
    const std::lock_guard<std::mutex> lock(environment_mutex());
    return setenv(key.c_str(), assigned_value.c_str(), 1) == 0;
#endif
}

bool write_environment_path(std::string_view name, const std::filesystem::path& value) {
    if (!is_valid_environment_name(name)) {
        return false;
    }

#if defined(_WIN32)
    const auto wide_name = widen_ascii_environment_name(name);
    const auto native_value = value.native();
    return wide_name.has_value() && native_value.find(L'\0') == std::wstring::npos &&
        _wputenv_s(wide_name->c_str(), native_value.c_str()) == 0;
#else
    return write_environment_variable(name, value.native());
#endif
}

bool clear_environment_variable(std::string_view name) {
    if (!is_valid_environment_name(name)) {
        return false;
    }

#if defined(_WIN32)
    const auto wide_name = widen_ascii_environment_name(name);
    return wide_name.has_value() && _wputenv_s(wide_name->c_str(), L"") == 0;
#else
    const std::string key(name);
    const std::lock_guard<std::mutex> lock(environment_mutex());
    return unsetenv(key.c_str()) == 0;
#endif
}

bool clear_environment_path(std::string_view name) {
    if (!is_valid_environment_name(name)) {
        return false;
    }

#if defined(_WIN32)
    const auto wide_name = widen_ascii_environment_name(name);
    return wide_name.has_value() && _wputenv_s(wide_name->c_str(), L"") == 0;
#else
    return clear_environment_variable(name);
#endif
}

}  // namespace copperfin::platform
