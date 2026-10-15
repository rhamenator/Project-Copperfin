// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
#pragma once
#include "copperfin/runtime/prg_engine.h"
#include <cstdint>
#include <optional>
#include <string_view>

namespace copperfin::runtime {

// RQ-CF-PRG-NUMERIC-INTEGER-DESCRIPTOR-001: classify attributes AFTER
// successful payload parsing, not syntax or value parsing. Exact staged
// native-backed contract: fixtures/vfp9-numeric-division-context/integer-contract.md.
inline std::optional<PrgNumericMetadata> plain_integer_literal_metadata(
    std::string_view literal) noexcept {
    if (literal.empty() || literal.size() > 10U) return std::nullopt;
    constexpr std::uint32_t maximum = 2147483647U;
    std::uint32_t value = 0U;
    for (const char character : literal) {
        if (character < '0' || character > '9') return std::nullopt;
        const auto digit = static_cast<std::uint32_t>(character - '0');
        if (value > (maximum - digit) / 10U) return std::nullopt;
        value = value * 10U + digit;
    }
    // Integer's unused native decimals are NOT read: zero is a normalized
    // placeholder ignored by consumers for integer storage.
    return PrgNumericMetadata{static_cast<std::uint32_t>(literal.size()), 0U,
                              PrgNumericStorage::integer};
}

// Known native Integer has no signed zero; ordinary nonzero negation stays
// unchanged. No floating-to-integral cast, rounding, allocation or epsilon.
inline double negated_integer_numeric_payload(double value) noexcept {
    return value == 0.0 ? 0.0 : -value;
}

} // namespace copperfin::runtime
