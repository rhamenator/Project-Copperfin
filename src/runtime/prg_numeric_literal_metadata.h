// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
#pragma once

#include "copperfin/runtime/prg_engine.h"

#include <algorithm>
#include <cstddef>
#include <optional>
#include <string_view>

namespace copperfin::runtime {

// RQ-CF-PRG-NUMERIC-LITERAL-METADATA-001; native evidence and exact staged
// domain: tests/fixtures/vfp9-numeric-literal-metadata/recovered-requirements.md.
// This classifies metadata only AFTER successful numeric parsing. It is not a
// syntax validator, payload parser, allocation size or general precision rule.
inline std::optional<PrgNumericMetadata> plain_decimal_literal_metadata(
    std::string_view literal) noexcept {
    if (literal.size() < 3U || literal.size() > 256U) {
        return std::nullopt;
    }
    std::size_t point = std::string_view::npos;
    for (std::size_t index = 0; index < literal.size(); ++index) {
        const char character = literal[index];
        if (character == '.' && point == std::string_view::npos) {
            point = index;
        } else if (character < '0' || character > '9') {
            return std::nullopt;
        }
    }
    if (point == std::string_view::npos || point == 0U || point + 1U == literal.size()) {
        return std::nullopt;
    }
    // Both casts are bounded independently of the caller's source-size limits.
    return PrgNumericMetadata{
        static_cast<std::uint32_t>(std::min<std::size_t>(40U, literal.size())),
        static_cast<std::uint32_t>(std::min<std::size_t>(18U, literal.size() - point - 1U))};
}

}  // namespace copperfin::runtime
