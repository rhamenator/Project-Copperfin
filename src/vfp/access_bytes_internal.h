// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

// Project-internal helpers shared between the Access/Jet physical-format
// readers (access_table_definition.cpp, access_msysobjects.cpp). Not part
// of this codebase's public include/ tree -- these are implementation
// details of the Access container/TDEF/row-decode slices, not a stable
// API surface.

#include <cstdint>
#include <string>
#include <vector>

namespace copperfin::vfp::access_bytes_internal {

inline std::uint16_t read_le_u16(const std::vector<std::uint8_t>& bytes, std::size_t offset) {
    return static_cast<std::uint16_t>(bytes[offset]) |
           (static_cast<std::uint16_t>(bytes[offset + 1U]) << 8U);
}

inline std::uint32_t read_le_u32(const std::vector<std::uint8_t>& bytes, std::size_t offset) {
    return static_cast<std::uint32_t>(bytes[offset]) |
           (static_cast<std::uint32_t>(bytes[offset + 1U]) << 8U) |
           (static_cast<std::uint32_t>(bytes[offset + 2U]) << 16U) |
           (static_cast<std::uint32_t>(bytes[offset + 3U]) << 24U);
}

// Jet3 column/text bytes are stored in the database's legacy single-byte
// code page, not UTF-8 (see access_table_definition.cpp's own documented
// reasoning for why this codebase does not yet transcode from the real
// code page -- it lives behind page 0's RC4-obscured region). This
// guarantees the returned string is always valid UTF-8 (an invariant
// every other string this codebase returns upholds) by replacing any
// byte sequence that is not valid UTF-8 with U+FFFD rather than passing
// legacy-code-page bytes through unchanged. A real ASCII-only Jet3
// string -- the overwhelming common case in practice, confirmed by every
// column/catalog-row name observed during this codebase's real-fixture
// cross-validation -- round-trips unchanged either way.
inline std::string sanitize_as_utf8(const std::vector<std::uint8_t>& raw) {
    std::string text;
    text.reserve(raw.size());
    std::size_t index = 0U;
    while (index < raw.size()) {
        const std::uint8_t lead = raw[index];
        std::size_t sequence_length = 0U;
        std::uint32_t code_point = 0U;
        if (lead < 0x80U) {
            sequence_length = 1U;
            code_point = lead;
        } else if ((lead & 0xE0U) == 0xC0U) {
            sequence_length = 2U;
            code_point = lead & 0x1FU;
        } else if ((lead & 0xF0U) == 0xE0U) {
            sequence_length = 3U;
            code_point = lead & 0x0FU;
        } else if ((lead & 0xF8U) == 0xF0U) {
            sequence_length = 4U;
            code_point = lead & 0x07U;
        } else {
            text += "\xEF\xBF\xBD";
            ++index;
            continue;
        }
        bool valid = (index + sequence_length <= raw.size());
        for (std::size_t offset = 1U; valid && offset < sequence_length; ++offset) {
            const std::uint8_t continuation = raw[index + offset];
            if ((continuation & 0xC0U) != 0x80U) {
                valid = false;
                break;
            }
            code_point = (code_point << 6U) | (continuation & 0x3FU);
        }
        // Reject overlong encodings and surrogate/out-of-range code points.
        if (valid &&
            ((sequence_length == 2U && code_point < 0x80U) ||
             (sequence_length == 3U && code_point < 0x800U) ||
             (sequence_length == 4U && code_point < 0x10000U) ||
             (code_point >= 0xD800U && code_point <= 0xDFFFU) ||
             code_point > 0x10FFFFU)) {
            valid = false;
        }
        if (valid) {
            text.append(reinterpret_cast<const char*>(&raw[index]), sequence_length);
            index += sequence_length;
        } else {
            text += "\xEF\xBF\xBD";
            ++index;
        }
    }
    return text;
}

}  // namespace copperfin::vfp::access_bytes_internal
