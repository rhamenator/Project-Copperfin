// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#pragma once

#include <filesystem>
#include <limits>
#include <string>
#include <string_view>

#if defined(_WIN32)
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

}  // namespace copperfin::platform
