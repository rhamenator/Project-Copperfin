// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
#pragma once
#include "copperfin/runtime/prg_engine.h"
#include <algorithm>
#include <charconv>
#include <optional>
#include <string_view>

namespace copperfin::runtime {
// RQ-CF-PRG-NUMERIC-CONSTANT-DIVISION-MATERIALIZATION-001: prepare once
// at an eligible constant division node, not after a later operand callback.
// Pure bounded metadata operation: no payload change, allocation or narrowing.
inline std::optional<PrgNumericMetadata> materialized_constant_division_metadata(
    PrgNumericMetadata intrinsic, std::string_view decimals_text) noexcept {
    if (intrinsic.display_width < 1U || intrinsic.display_width > 31U) return std::nullopt;
    if (intrinsic.storage == PrgNumericStorage::integer) {
        if (intrinsic.display_width > 12U) return std::nullopt;
        intrinsic.decimal_places = 0U;
        return intrinsic;
    }
    if (intrinsic.storage != PrgNumericStorage::real || intrinsic.decimal_places > 18U)
        return std::nullopt;
    const auto suffix = intrinsic.decimal_places + (intrinsic.decimal_places > 0U ? 1U : 0U);
    if (intrinsic.display_width <= suffix || intrinsic.display_width - suffix > 12U ||
        decimals_text.empty() || (decimals_text.size() > 1U && decimals_text.front() == '0'))
        return std::nullopt;
    unsigned setting = 0;
    const auto parsed = std::from_chars(decimals_text.data(), decimals_text.data() + decimals_text.size(), setting);
    if (parsed.ec != std::errc{} || parsed.ptr != decimals_text.data() + decimals_text.size() || setting > 18U)
        return std::nullopt;
    const auto scale = std::max(intrinsic.decimal_places, setting);
    const auto width = intrinsic.display_width + scale - intrinsic.decimal_places +
        (intrinsic.decimal_places == 0U && scale > 0U ? 1U : 0U);
    return PrgNumericMetadata{width, scale, PrgNumericStorage::real};
}
} // namespace copperfin::runtime
