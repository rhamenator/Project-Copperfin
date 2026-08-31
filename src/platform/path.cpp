// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/path.h"

#include <algorithm>
#include <cctype>
#include <limits>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace copperfin::platform {

std::string path_to_utf8_string(const std::filesystem::path& value) {
#if defined(_WIN32)
    const std::wstring native = value.wstring();
    if (native.empty() || native.size() > static_cast<std::size_t>((std::numeric_limits<int>::max)())) {
        return {};
    }
    const int byte_count = ::WideCharToMultiByte(
        CP_UTF8, WC_ERR_INVALID_CHARS, native.data(), static_cast<int>(native.size()),
        nullptr, 0, nullptr, nullptr);
    if (byte_count <= 0) {
        return {};
    }
    std::string decoded(static_cast<std::size_t>(byte_count), '\0');
    if (::WideCharToMultiByte(
            CP_UTF8, WC_ERR_INVALID_CHARS, native.data(), static_cast<int>(native.size()),
            decoded.data(), byte_count, nullptr, nullptr) != byte_count) {
        return {};
    }
    return decoded;
#else
    const auto encoded = value.u8string();
    std::string decoded;
    decoded.reserve(encoded.size());
    for (const char8_t ch : encoded) {
        decoded.push_back(static_cast<char>(ch));
    }
    return decoded;
#endif
}

std::filesystem::path path_from_utf8_string(std::string_view value) {
#if defined(_WIN32)
    if (value.empty() || value.size() > static_cast<std::size_t>((std::numeric_limits<int>::max)())) {
        return {};
    }
    const int character_count = ::MultiByteToWideChar(
        CP_UTF8, MB_ERR_INVALID_CHARS, value.data(), static_cast<int>(value.size()), nullptr, 0);
    if (character_count <= 0) {
        return {};
    }
    std::wstring native(static_cast<std::size_t>(character_count), L'\0');
    if (::MultiByteToWideChar(
            CP_UTF8, MB_ERR_INVALID_CHARS, value.data(), static_cast<int>(value.size()),
            native.data(), character_count) != character_count) {
        return {};
    }
    return std::filesystem::path(native);
#else
    std::u8string encoded;
    encoded.reserve(value.size());
    for (const unsigned char ch : value) {
        encoded.push_back(static_cast<char8_t>(ch));
    }
    return std::filesystem::path(encoded);
#endif
}

bool path_component_equal_for_platform(
    const std::filesystem::path& left,
    const std::filesystem::path& right) {
#if defined(_WIN32)
    const std::wstring left_value = left.native();
    const std::wstring right_value = right.native();
    const auto maximum_api_length =
        static_cast<std::size_t>((std::numeric_limits<int>::max)());
    if (left_value.size() > maximum_api_length || right_value.size() > maximum_api_length) {
        return false;
    }
    if (::CompareStringOrdinal(
            left_value.c_str(), static_cast<int>(left_value.size()),
            right_value.c_str(), static_cast<int>(right_value.size()), TRUE) == CSTR_EQUAL) {
        return true;
    }

    // Retain the older invariant-locale entry point for Windows environments
    // where the ordinal and extended comparison APIs do not share the same
    // Unicode case table.
    if (::CompareStringW(
            LOCALE_INVARIANT, NORM_IGNORECASE,
            left_value.c_str(), static_cast<int>(left_value.size()),
            right_value.c_str(), static_cast<int>(right_value.size())) == CSTR_EQUAL) {
        return true;
    }

    // Some Windows builds do not apply the complete Unicode simple-case
    // table through CompareStringOrdinal. The invariant locale comparison
    // supplies the Windows filesystem's case-insensitive behavior for those
    // code points without consulting the user's active locale.
    if (::CompareStringEx(
            LOCALE_NAME_INVARIANT, NORM_IGNORECASE,
            left_value.c_str(), static_cast<int>(left_value.size()),
            right_value.c_str(), static_cast<int>(right_value.size()),
            nullptr, nullptr, 0) == CSTR_EQUAL) {
        return true;
    }

    // Some supported Windows environments do not provide complete Unicode
    // simple-case behavior through CompareStringOrdinal alone. Prefer the
    // invariant mapping API, with a native fallback when it is unavailable.
    const auto invariant_lowercase = [](const std::wstring& value) {
        if (value.empty()) {
            return std::wstring{};
        }
        const int required = ::LCMapStringEx(
            LOCALE_NAME_INVARIANT, LCMAP_LOWERCASE,
            value.c_str(), static_cast<int>(value.size()),
            nullptr, 0, nullptr, nullptr, 0);
        if (required > 0) {
            std::wstring mapped(static_cast<std::size_t>(required), L'\0');
            if (::LCMapStringEx(
                    LOCALE_NAME_INVARIANT, LCMAP_LOWERCASE,
                    value.c_str(), static_cast<int>(value.size()), mapped.data(), required,
                    nullptr, nullptr, 0) > 0) {
                return mapped;
            }
        }

        std::wstring fallback = value;
        // CharLowerBuffW reports zero on failure. Returning the unchanged
        // input makes a mapping failure conservative rather than treating
        // unconverted text as a successful fold.
        if (::CharLowerBuffW(fallback.data(), static_cast<DWORD>(fallback.size())) == 0) {
            return value;
        }
        return fallback;
    };
    return invariant_lowercase(left_value) == invariant_lowercase(right_value);
#else
    return left == right;
#endif
}

bool path_is_hidden(const std::filesystem::path& value) {
    const std::filesystem::path normalized = value.lexically_normal();
#if defined(_WIN32)
    const DWORD attributes = ::GetFileAttributesW(normalized.c_str());
    if (attributes == INVALID_FILE_ATTRIBUTES) {
        return false;
    }
    return (attributes & FILE_ATTRIBUTE_HIDDEN) != 0;
#else
    // path::filename() is empty when the path has a trailing directory
    // separator (e.g. "foo/bar/"), so fall back to the parent's filename to
    // find the actual final path component instead of silently treating a
    // trailing-separator variant of a hidden name as visible.
    std::filesystem::path component = normalized.filename();
    if (component.empty()) {
        component = normalized.parent_path().filename();
    }
    const std::string name = path_to_utf8_string(component);
    return !name.empty() && name.front() == '.';
#endif
}

bool path_is_system(const std::filesystem::path& value) {
#if defined(_WIN32)
    const std::filesystem::path normalized = value.lexically_normal();
    const DWORD attributes = ::GetFileAttributesW(normalized.c_str());
    return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_SYSTEM) != 0;
#else
    static_cast<void>(value);
    return false;
#endif
}

bool path_is_hidden_or_system(const std::filesystem::path& value) {
    return path_is_hidden(value) || path_is_system(value);
}

std::string path_dos_8dot3_filename(const std::filesystem::path& value) {
#if defined(_WIN32)
    const std::filesystem::path normalized = value.lexically_normal();
    std::wstring buffer(260U, L'\0');
    for (;;) {
        const DWORD length = ::GetShortPathNameW(
            normalized.c_str(), buffer.data(), static_cast<DWORD>(buffer.size()));
        if (length == 0U) {
            return path_to_utf8_string(normalized.filename());
        }
        if (length < buffer.size()) {
            buffer.resize(length);
            return path_to_utf8_string(std::filesystem::path(buffer).filename());
        }
        buffer.assign(static_cast<std::size_t>(length) + 1U, L'\0');
    }
#else
    return path_to_utf8_string(value.filename());
#endif
}

std::optional<std::string> path_volume_label(const std::filesystem::path& value) {
#if defined(_WIN32)
    const std::filesystem::path normalized = value.lexically_normal();
    std::wstring volume_root(32768U, L'\0');
    if (::GetVolumePathNameW(
            normalized.c_str(), volume_root.data(), static_cast<DWORD>(volume_root.size())) == 0) {
        return std::nullopt;
    }
    volume_root.resize(std::wcslen(volume_root.c_str()));

    std::wstring label(MAX_PATH + 1U, L'\0');
    if (::GetVolumeInformationW(
            volume_root.c_str(), label.data(), static_cast<DWORD>(label.size()), nullptr, nullptr,
            nullptr, nullptr, 0U) == 0) {
        return std::nullopt;
    }
    label.resize(std::wcslen(label.c_str()));
    return path_to_utf8_string(std::filesystem::path(label));
#else
    static_cast<void>(value);
    return std::nullopt;
#endif
}

bool path_equal_case_insensitive(
    const std::filesystem::path& left,
    const std::filesystem::path& right) {
    const std::filesystem::path normalized_left = left.lexically_normal();
    const std::filesystem::path normalized_right = right.lexically_normal();
#if defined(_WIN32)
    // Whole-path identity needs the same invariant Unicode fallback layers
    // and fail-closed length guard as Windows component comparison.
    return path_component_equal_for_platform(normalized_left, normalized_right);
#else
    const auto lowercase_path = [](const std::filesystem::path& value) {
        std::string lowered = path_to_utf8_string(value);
        std::transform(lowered.begin(), lowered.end(), lowered.begin(), [](unsigned char ch) {
            return static_cast<char>(std::tolower(ch));
        });
        return lowered;
    };
    return lowercase_path(normalized_left) == lowercase_path(normalized_right);
#endif
}

bool path_has_embedded_nul(const std::filesystem::path& path) {
    const auto& native = path.native();
    return native.find(typename std::filesystem::path::value_type{}) !=
        std::filesystem::path::string_type::npos;
}

bool path_has_dot_component(const std::filesystem::path& path) {
    for (const auto& component : path) {
        if (component == "." || component == "..") {
            return true;
        }
    }
    return false;
}

#if defined(_WIN32)
bool path_has_windows_alias_prone_component(const std::filesystem::path& path) {
    for (const auto& component : path) {
        const auto& native = component.native();
        if (!native.empty() &&
            (native.back() == L'.' || native.back() == L' ')) {
            return true;
        }
    }
    return false;
}

namespace {

bool path_component_is_reserved_windows_device_name(
    const std::filesystem::path::string_type& component) {
    const auto stop = component.find_first_of(L".:");
    const std::filesystem::path::string_type stem =
        (stop == std::filesystem::path::string_type::npos)
            ? component
            : component.substr(0U, stop);
    if (stem.empty() || stem.size() > 4U) {
        return false;
    }
    std::filesystem::path::string_type upper;
    upper.reserve(stem.size());
    for (const wchar_t ch : stem) {
        upper.push_back(
            (ch >= L'a' && ch <= L'z') ? static_cast<wchar_t>(ch - (L'a' - L'A')) : ch);
    }
    static constexpr std::wstring_view reserved_names[] = {
        L"CON", L"PRN", L"AUX", L"NUL",
        L"COM1", L"COM2", L"COM3", L"COM4", L"COM5", L"COM6", L"COM7", L"COM8", L"COM9",
        L"LPT1", L"LPT2", L"LPT3", L"LPT4", L"LPT5", L"LPT6", L"LPT7", L"LPT8", L"LPT9"
    };
    for (const std::wstring_view name : reserved_names) {
        if (upper == name) {
            return true;
        }
    }
    return false;
}

}  // namespace

bool path_has_reserved_windows_device_name_component(
    const std::filesystem::path& path) {
    for (const auto& component : path) {
        if (path_component_is_reserved_windows_device_name(component.native())) {
            return true;
        }
    }
    return false;
}
#else
bool path_has_windows_alias_prone_component(const std::filesystem::path&) {
    return false;
}

bool path_has_reserved_windows_device_name_component(const std::filesystem::path&) {
    return false;
}
#endif

}  // namespace copperfin::platform
