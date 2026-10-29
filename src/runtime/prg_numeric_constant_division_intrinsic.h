// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
#pragma once

#include "copperfin/runtime/prg_engine.h"
#include <algorithm>
#include <cmath>
#include <optional>

namespace copperfin::runtime {

struct ConstantDivisionIntrinsicResult {
    PrgNumericMetadata metadata;
    double payload;
};

namespace detail {

inline std::optional<ConstantDivisionIntrinsicResult> constant_division_intrinsic_impl(
    const PrgValue& left, const PrgValue& right, double quotient,
    bool constant_literal_node, bool allow_nonwhole_real,
    bool allow_small_divisor) noexcept {
    if (!constant_literal_node || !std::isfinite(quotient) ||
        std::abs(quotient) >= 10000000000.0 || right.number_value == 0.0 ||
        (!allow_small_divisor && std::abs(right.number_value) < 1.0)) {
        return std::nullopt;
    }
    const auto eligible = [allow_nonwhole_real](const PrgValue& value) {
        if (value.is_null || value.kind != PrgValueKind::number ||
            !value.numeric_metadata || !std::isfinite(value.number_value) ||
            std::abs(value.number_value) >= 10000000000.0) return false;
        const auto& m = *value.numeric_metadata;
        if (m.display_width < 1U || m.display_width > 31U) return false;
        if (m.storage == PrgNumericStorage::integer) {
            return m.display_width <= 12U && std::abs(value.number_value) <= 2147483647.0 &&
                std::trunc(value.number_value) == value.number_value;
        }
        if (m.storage != PrgNumericStorage::real || m.decimal_places > 18U) return false;
        if (!allow_nonwhole_real && std::trunc(value.number_value) != value.number_value) return false;
        // Subtract only after validating the point/scale contribution.
        const auto suffix = m.decimal_places + (m.decimal_places > 0U ? 1U : 0U);
        return m.display_width > suffix && m.display_width - suffix <= 12U;
    };
    if (!eligible(left) || !eligible(right)) return std::nullopt;
    const auto& l = *left.numeric_metadata;
    const auto& r = *right.numeric_metadata;
    const auto dl = l.storage == PrgNumericStorage::integer ? 0U : l.decimal_places;
    const auto dr = r.storage == PrgNumericStorage::integer ? 0U : r.decimal_places;
    const bool integer = l.storage == PrgNumericStorage::integer &&
        r.storage == PrgNumericStorage::integer &&
        std::trunc(quotient) == quotient && std::abs(quotient) <= 2147483647.0;
    const auto scale = integer ? 0U : std::min(18U, dl + dr);
    const auto left_whole = l.display_width - dl - (dl > 0U ? 1U : 0U);
    const auto magnitude = std::abs(quotient);
    unsigned quotient_whole = magnitude >= 1.0 ? 1U : 0U;
    // Exact bounded decimal thresholds; no floating-to-integral conversion.
    for (double threshold = 10.0; magnitude >= threshold; threshold *= 10.0) ++quotient_whole;
    if (quotient < 0.0) ++quotient_whole;
    const auto width = std::max(left_whole, quotient_whole) + scale + (scale > 0U ? 1U : 0U);
    return ConstantDivisionIntrinsicResult{
        PrgNumericMetadata{width, scale, integer ? PrgNumericStorage::integer : PrgNumericStorage::real},
        quotient == 0.0 ? 0.0 : quotient};
}

} // namespace detail

// RQ-CF-PRG-NUMERIC-CONSTANT-DIVISION-INTRINSIC-001:
// fixtures/vfp9-numeric-division-context/constant-intrinsic-contract.md.
// Preserve the original whole-operand frontier and all640 native rows.
inline std::optional<ConstantDivisionIntrinsicResult> constant_division_intrinsic(
    const PrgValue& left, const PrgValue& right, double quotient,
    bool constant_literal_node) noexcept {
    return detail::constant_division_intrinsic_impl(
        left,right,quotient,constant_literal_node,false,false);
}

// RQ-CF-PRG-NUMERIC-CONSTANT-DIVISION-NONWHOLE-001:
// fixtures/vfp9-numeric-division-context/nonwhole-timing-contract.md.
// Caller must prove a constant literal-derived node, never infer origin from
// a double/descriptor. Only real storage gains fractional eligibility; Integer
// must still be integral. No SET DECIMALS, materialization, replay, allocation,
// floating-to-integer conversion, rounding or nonzero payload correction.
inline std::optional<ConstantDivisionIntrinsicResult> constant_nonwhole_division_intrinsic(
    const PrgValue& left, const PrgValue& right, double quotient,
    bool constant_literal_node) noexcept {
    return detail::constant_division_intrinsic_impl(
        left,right,quotient,constant_literal_node,true,false);
}

// RQ-CF-PRG-NUMERIC-CONSTANT-DIVISION-SMALL-DIVISOR-001:
// fixtures/vfp9-numeric-division-context/small-divisor-recovery.md.
// This pure, currently unused support extends only the independently frozen
// finite literal-derived frontier. The caller must prove constant-node origin;
// zero/nonfinite/magnitude/metadata and Integer bounds remain fail-closed.
inline std::optional<ConstantDivisionIntrinsicResult> constant_small_divisor_intrinsic(
    const PrgValue& left, const PrgValue& right, double quotient,
    bool constant_literal_node) noexcept {
    return detail::constant_division_intrinsic_impl(
        left,right,quotient,constant_literal_node,true,true);
}

} // namespace copperfin::runtime
