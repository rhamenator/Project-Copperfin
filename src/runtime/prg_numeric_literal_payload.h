// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
#pragma once

#include "prg_numeric_literal_metadata.h"

#include <cfenv>
#include <cmath>
#include <limits>
#include <optional>
#include <string_view>

namespace copperfin::runtime {

// RQ-CF-PRG-NUMERIC-DECIMAL-LITERAL-PAYLOAD-001:
// tests/fixtures/vfp9-numeric-literal-payload/contract.md. UNUSED support first.
// Native-backed unsigned plain decimal shape only; not shared invariant-data
// parsing, a syntax/grammar extension, decimal arithmetic or display rounding.
inline std::optional<double> plain_decimal_literal_payload(std::string_view literal) noexcept {
    static_assert(sizeof(double)==8&&std::numeric_limits<double>::is_iec559&&
                  std::numeric_limits<double>::digits==53&&std::numeric_limits<double>::radix==2);
    // The existing complete-shape classifier bounds length3..256 and validates
    // every byte before floating work. Its display caps NEVER cap these digits.
    if(!plain_decimal_literal_metadata(literal)||std::fegetround()!=FE_TONEAREST)
        return std::nullopt;

    const auto point=literal.find('.');
    double whole=0.0,fraction=0.0,divisor=1.0;
    for(std::size_t index=0;index<point;++index) {
        // Volatile temporaries enforce observed binary64 rounding, NOT thread
        // synchronization. Separate stores prohibit multiply/add contraction
        // and excess-precision propagation across significant native steps.
        const volatile double product=whole*10.0;
        const volatile double next=product+static_cast<double>(literal[index]-'0');
        whole=next;
    }
    for(std::size_t index=point+1;index<literal.size();++index) {
        const volatile double product=fraction*10.0;
        const volatile double next=product+static_cast<double>(literal[index]-'0');
        const volatile double scaled=divisor*10.0;
        fraction=next;
        divisor=scaled;
    }
    // At most255 digits and a positive divisor: every intermediate is finite
    // inside the admitted domain. Retain a final fail-closed finite guard.
    const volatile double ratio=fraction/divisor;
    const volatile double combined=whole+ratio;
    const double result=combined;
    return std::isfinite(result)?std::optional<double>{result}:std::nullopt;
}

} // namespace copperfin::runtime
