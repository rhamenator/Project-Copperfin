// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#pragma once

#include <filesystem>
#include <string>
#include <string_view>

namespace copperfin::platform {

inline std::string path_to_utf8_string(const std::filesystem::path& value) {
    const auto encoded = value.u8string();
    std::string decoded;
    decoded.reserve(encoded.size());
    for (const char8_t ch : encoded) {
        decoded.push_back(static_cast<char>(ch));
    }
    return decoded;
}

inline std::filesystem::path path_from_utf8_string(std::string_view value) {
    std::u8string encoded;
    encoded.reserve(value.size());
    for (const unsigned char ch : value) {
        encoded.push_back(static_cast<char8_t>(ch));
    }
    return std::filesystem::path(encoded);
}

}  // namespace copperfin::platform
