// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "base64.h"

namespace copperfin::licensing {

namespace {

int decode_char(char ch) {
    if (ch >= 'A' && ch <= 'Z') {
        return ch - 'A';
    }
    if (ch >= 'a' && ch <= 'z') {
        return (ch - 'a') + 26;
    }
    if (ch >= '0' && ch <= '9') {
        return (ch - '0') + 52;
    }
    if (ch == '+') {
        return 62;
    }
    if (ch == '/') {
        return 63;
    }
    return -1;
}

}  // namespace

std::optional<std::vector<std::uint8_t>> base64_decode(const std::string& input) {
    std::string filtered;
    filtered.reserve(input.size());
    for (const char ch : input) {
        if (ch == '\n' || ch == '\r' || ch == ' ' || ch == '\t') {
            continue;
        }
        filtered.push_back(ch);
    }

    const std::size_t length = filtered.size();
    if (length == 0U || (length % 4U) != 0U) {
        return std::nullopt;
    }

    std::vector<std::uint8_t> out;
    out.reserve((length / 4U) * 3U);

    for (std::size_t index = 0U; index < length; index += 4U) {
        const bool pad2 = filtered[index + 2U] == '=';
        const bool pad3 = filtered[index + 3U] == '=';

        const int c0 = decode_char(filtered[index]);
        const int c1 = decode_char(filtered[index + 1U]);
        const int c2 = pad2 ? 0 : decode_char(filtered[index + 2U]);
        const int c3 = pad3 ? 0 : decode_char(filtered[index + 3U]);

        if (c0 < 0 || c1 < 0 || c2 < 0 || c3 < 0) {
            return std::nullopt;
        }
        if (pad2 && !pad3) {
            // "==X" with a real character after a pad byte is invalid.
            return std::nullopt;
        }

        const auto triple = (static_cast<std::uint32_t>(c0) << 18U) |
            (static_cast<std::uint32_t>(c1) << 12U) |
            (static_cast<std::uint32_t>(c2) << 6U) |
            static_cast<std::uint32_t>(c3);

        out.push_back(static_cast<std::uint8_t>((triple >> 16U) & 0xFFU));
        if (!pad2) {
            out.push_back(static_cast<std::uint8_t>((triple >> 8U) & 0xFFU));
        }
        if (!pad3) {
            out.push_back(static_cast<std::uint8_t>(triple & 0xFFU));
        }
    }

    return out;
}

}  // namespace copperfin::licensing
