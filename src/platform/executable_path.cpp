// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/executable_path.h"

#include "copperfin/platform/environment.h"

#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#if defined(_WIN32)
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#include <unistd.h>
#elif defined(__linux__)
#include <unistd.h>
#endif

namespace copperfin::platform {

namespace {

template <typename Character>
std::vector<std::basic_string<Character>> split_search_path_list(
    std::basic_string_view<Character> raw_paths,
    Character separator) {
    std::vector<std::basic_string<Character>> values;
    std::size_t start = 0U;
    while (start <= raw_paths.size()) {
        const std::size_t end = raw_paths.find(separator, start);
        if (end == std::basic_string_view<Character>::npos) {
            values.emplace_back(raw_paths.substr(start));
            break;
        }
        values.emplace_back(raw_paths.substr(start, end - start));
        start = end + 1U;
    }
    return values;
}

#if defined(_WIN32)
void append_unique(
    std::vector<std::filesystem::path>& values,
    const std::filesystem::path& value) {
    if (!value.empty() && std::find(values.begin(), values.end(), value) == values.end()) {
        values.push_back(value);
    }
}
#endif

#if !defined(_WIN32)
std::optional<std::string> default_posix_search_path_value() {
#if defined(_CS_PATH)
    const std::size_t required_size = confstr(_CS_PATH, nullptr, 0U);
    if (required_size <= 1U) {
        return std::nullopt;
    }
    std::string value(required_size, '\0');
    if (confstr(_CS_PATH, value.data(), value.size()) == 0U) {
        return std::nullopt;
    }
    const std::size_t terminator = value.find('\0');
    if (terminator == std::string::npos) {
        return std::nullopt;
    }
    value.resize(terminator);
    return value;
#else
    return std::string("/bin:/usr/bin");
#endif
}
#endif

std::vector<std::filesystem::path> executable_search_suffixes(
    const std::filesystem::path& invocation_path) {
    if (invocation_path.has_extension()) {
        return {std::filesystem::path()};
    }

#if defined(_WIN32)
    std::vector<std::filesystem::path> suffixes;
    const auto path_extensions = read_environment_path("PATHEXT");
    const std::wstring native_extensions =
        path_extensions.has_value() ? path_extensions->native() : std::wstring();
    for (std::wstring suffix : split_search_path_list<wchar_t>(native_extensions, L';')) {
        if (suffix.empty()) {
            continue;
        }
        if (suffix.front() != L'.') {
            suffix.insert(suffix.begin(), L'.');
        }
        append_unique(suffixes, std::filesystem::path(suffix));
    }
    if (suffixes.empty()) {
        suffixes = {L".exe", L".com", L".bat", L".cmd"};
    }
    return suffixes;
#else
    return {std::filesystem::path()};
#endif
}

std::filesystem::path normalize_executable_path(const std::filesystem::path& path) {
    namespace fs = std::filesystem;
    if (path.empty()) {
        return {};
    }

    std::error_code canonical_error;
    const fs::path canonical = fs::weakly_canonical(path, canonical_error);
    if (!canonical_error) {
        return canonical;
    }

    std::error_code absolute_error;
    const fs::path absolute = fs::absolute(path, absolute_error);
    return absolute_error ? path.lexically_normal() : absolute.lexically_normal();
}

bool is_launchable_candidate(const std::filesystem::path& path) {
    std::error_code type_error;
    if (!std::filesystem::is_regular_file(path, type_error)) {
        return false;
    }
#if defined(_WIN32)
    return true;
#else
    return access(path.c_str(), X_OK) == 0;
#endif
}

std::filesystem::path query_running_executable_path() {
#if defined(_WIN32)
    std::vector<wchar_t> buffer(260U, L'\0');
    while (buffer.size() <= 65536U) {
        const DWORD length = GetModuleFileNameW(
            nullptr,
            buffer.data(),
            static_cast<DWORD>(buffer.size()));
        if (length == 0U) {
            return {};
        }
        if (length < buffer.size()) {
            return normalize_executable_path(
                std::filesystem::path(std::wstring(buffer.data(), length)));
        }
        buffer.resize(buffer.size() * 2U, L'\0');
    }
#elif defined(__APPLE__)
    std::uint32_t required_size = 1024U;
    std::vector<char> buffer(required_size, '\0');
    if (_NSGetExecutablePath(buffer.data(), &required_size) != 0) {
        buffer.assign(required_size, '\0');
        if (_NSGetExecutablePath(buffer.data(), &required_size) != 0) {
            return {};
        }
    }
    return normalize_executable_path(std::filesystem::path(buffer.data()));
#elif defined(__linux__)
    std::vector<char> buffer(1024U, '\0');
    while (buffer.size() <= 1024U * 1024U) {
        const ssize_t length = readlink("/proc/self/exe", buffer.data(), buffer.size());
        if (length < 0) {
            return {};
        }
        if (static_cast<std::size_t>(length) < buffer.size()) {
            return normalize_executable_path(
                std::filesystem::path(std::string(buffer.data(), static_cast<std::size_t>(length))));
        }
        buffer.resize(buffer.size() * 2U, '\0');
    }
#endif
    return {};
}

}  // namespace

#if !defined(_WIN32)
std::optional<std::string> default_posix_search_path() {
    return default_posix_search_path_value();
}
#endif

std::filesystem::path resolve_executable_invocation_path(
    const std::filesystem::path& invocation_path) {
    namespace fs = std::filesystem;
    if (invocation_path.empty()) {
        return {};
    }

    if (invocation_path.is_absolute() || invocation_path.has_parent_path()) {
        return normalize_executable_path(invocation_path);
    }

#if defined(_WIN32)
    const auto path_value = read_environment_path("PATH");
    const std::wstring native_path =
        path_value.has_value() ? path_value->native() : std::wstring();
    const auto search_roots = split_search_path_list<wchar_t>(native_path, L';');
#else
    const auto path_value = read_environment_variable("PATH");
    std::vector<std::string> search_roots;
    if (path_value.has_value()) {
        search_roots = split_search_path_list<char>(*path_value, ':');
    } else if (const auto default_path = default_posix_search_path_value(); default_path.has_value()) {
        search_roots = split_search_path_list<char>(*default_path, ':');
    }
#endif
    const std::vector<fs::path> suffixes = executable_search_suffixes(invocation_path);
    for (const auto& raw_root : search_roots) {
        const fs::path search_root =
            raw_root.empty() ? fs::current_path() : fs::path(raw_root);
        for (const fs::path& suffix : suffixes) {
            fs::path candidate = search_root / invocation_path;
            candidate += suffix.native();
            if (!is_launchable_candidate(candidate)) {
                continue;
            }
            return normalize_executable_path(candidate);
        }
    }

#if defined(_WIN32)
    return normalize_executable_path(invocation_path);
#else
    return path_value.has_value()
        ? normalize_executable_path(invocation_path)
        : invocation_path;
#endif
}

std::filesystem::path resolve_running_executable_path(
    const std::filesystem::path& fallback_invocation_path) {
    const std::filesystem::path running_path = query_running_executable_path();
    return running_path.empty()
        ? resolve_executable_invocation_path(fallback_invocation_path)
        : running_path;
}

}  // namespace copperfin::platform
