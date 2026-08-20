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

bool path_is_hidden_or_system(const std::filesystem::path& value) {
    const std::filesystem::path normalized = value.lexically_normal();
#if defined(_WIN32)
    const DWORD attributes = ::GetFileAttributesW(normalized.c_str());
    if (attributes == INVALID_FILE_ATTRIBUTES) {
        return false;
    }
    return (attributes & (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM)) != 0;
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

}  // namespace copperfin::platform
