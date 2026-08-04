// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <filesystem>
#include <limits>
#include <string>
#include <string_view>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace copperfin::platform {

inline std::string path_to_utf8_string(const std::filesystem::path& value) {
#if defined(_WIN32)
    const std::wstring native = value.wstring();
    if (native.empty() || native.size() > static_cast<std::size_t>((std::numeric_limits<int>::max)())) {
        return {};
    }
    const int byte_count = ::WideCharToMultiByte(
        CP_UTF8,
        WC_ERR_INVALID_CHARS,
        native.data(),
        static_cast<int>(native.size()),
        nullptr,
        0,
        nullptr,
        nullptr);
    if (byte_count <= 0) {
        return {};
    }
    std::string decoded(static_cast<std::size_t>(byte_count), '\0');
    if (::WideCharToMultiByte(
            CP_UTF8,
            WC_ERR_INVALID_CHARS,
            native.data(),
            static_cast<int>(native.size()),
            decoded.data(),
            byte_count,
            nullptr,
            nullptr) != byte_count) {
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

inline std::filesystem::path path_from_utf8_string(std::string_view value) {
#if defined(_WIN32)
    if (value.empty() || value.size() > static_cast<std::size_t>((std::numeric_limits<int>::max)())) {
        return {};
    }
    const int character_count = ::MultiByteToWideChar(
        CP_UTF8,
        MB_ERR_INVALID_CHARS,
        value.data(),
        static_cast<int>(value.size()),
        nullptr,
        0);
    if (character_count <= 0) {
        return {};
    }
    std::wstring native(static_cast<std::size_t>(character_count), L'\0');
    if (::MultiByteToWideChar(
            CP_UTF8,
            MB_ERR_INVALID_CHARS,
            value.data(),
            static_cast<int>(value.size()),
            native.data(),
            character_count) != character_count) {
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

inline bool path_component_equal_for_platform(
    const std::filesystem::path& left,
    const std::filesystem::path& right) {
#if defined(_WIN32)
    const std::wstring left_value = left.native();
    const std::wstring right_value = right.native();
    if (::CompareStringOrdinal(
            left_value.c_str(),
            static_cast<int>(left_value.size()),
            right_value.c_str(),
            static_cast<int>(right_value.size()),
            TRUE) == CSTR_EQUAL) {
        return true;
    }

    // Retain the older invariant-locale entry point for Windows environments
    // where the ordinal and extended comparison APIs do not share the same
    // Unicode case table.
    if (::CompareStringW(
            LOCALE_INVARIANT,
            NORM_IGNORECASE,
            left_value.c_str(),
            static_cast<int>(left_value.size()),
            right_value.c_str(),
            static_cast<int>(right_value.size())) == CSTR_EQUAL) {
        return true;
    }

    // Some Windows builds do not apply the complete Unicode simple-case
    // table through CompareStringOrdinal. The invariant locale comparison
    // supplies the Windows filesystem's case-insensitive behavior for those
    // code points without consulting the user's active locale.
    if (::CompareStringEx(
            LOCALE_NAME_INVARIANT,
            NORM_IGNORECASE,
            left_value.c_str(),
            static_cast<int>(left_value.size()),
            right_value.c_str(),
            static_cast<int>(right_value.size()),
            nullptr,
            nullptr,
            0) == CSTR_EQUAL) {
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
            LOCALE_NAME_INVARIANT,
            LCMAP_LOWERCASE,
            value.c_str(),
            static_cast<int>(value.size()),
            nullptr,
            0,
            nullptr,
            nullptr,
            0);
        if (required > 0) {
            std::wstring mapped(static_cast<std::size_t>(required), L'\0');
            if (::LCMapStringEx(
                    LOCALE_NAME_INVARIANT,
                    LCMAP_LOWERCASE,
                    value.c_str(),
                    static_cast<int>(value.size()),
                    mapped.data(),
                    required,
                    nullptr,
                    nullptr,
                    0) > 0) {
                return mapped;
            }
        }

        std::wstring fallback = value;
        // CharLowerBuffW returns zero when the buffer needs no changes as
        // well as when the call cannot process the buffer. The unchanged
        // value is still a valid case-folding result.
        (void)::CharLowerBuffW(fallback.data(), static_cast<DWORD>(fallback.size()));
        return fallback;
    };
    return invariant_lowercase(left_value) == invariant_lowercase(right_value);
#else
    return left == right;
#endif
}

}  // namespace copperfin::platform
