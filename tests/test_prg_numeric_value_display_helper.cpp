// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// VR-5611-NUMERIC-VALUE-DISPLAY-001: frozen native strings, never support-generated expectations.
#include "../src/runtime/prg_numeric_value_display.h"
#include "fixtures/vfp9-numeric-division-context/small-divisor-public-expected.h"

#include <bit>
#include <iostream>
#include <limits>

int main() {
    using namespace copperfin::runtime;
    using namespace copperfin::test;
    unsigned failures = 0U;
    const auto expect = [&](const bool ok, const char* label) {
        if (!ok) { ++failures; std::cerr << "FAIL: " << label << '\n'; }
    };
    constexpr unsigned settings[]{0U, 2U, 6U, 14U, 18U};
    unsigned rows = 0U;
    for (const auto& row : native_small_divisor_rows) {
        if (!row.admitted) continue;
        const double value = std::bit_cast<double>(row.bits);
        for (std::size_t index = 0U; index < 5U; ++index) {
            for (unsigned fixed_index = 0U; fixed_index < 2U; ++fixed_index) {
                const auto actual = value_associated_numeric_display(
                    value, row.metadata[index], settings[index], fixed_index != 0U);
                const auto* expected = row.display[index * 2U + fixed_index];
                if (!actual || *actual != expected)
                    std::cerr << row.label << " setting=" << settings[index]
                              << " fixed=" << fixed_index << '\n';
                expect(actual && *actual == expected, "complete frozen display frontier");
                ++rows;
            }
        }
    }
    expect(rows == 2450U, "all admitted display contexts");

    const PrgNumericMetadata valid{4U, 2U, PrgNumericStorage::real};
    for (const double unsupported : {std::numeric_limits<double>::quiet_NaN(),
                                     std::numeric_limits<double>::infinity(),
                                     -std::numeric_limits<double>::infinity(),
                                     1e10, -1e10, 1e300})
        expect(!value_associated_numeric_display(unsupported, valid, 2U, false),
               "unsupported payload rejected");
    for (const auto malformed : {
             PrgNumericMetadata{0U, 0U, PrgNumericStorage::real},
             PrgNumericMetadata{32U, 0U, PrgNumericStorage::real},
             PrgNumericMetadata{4U, 19U, PrgNumericStorage::real},
             PrgNumericMetadata{3U, 2U, PrgNumericStorage::real},
             PrgNumericMetadata{14U, 0U, PrgNumericStorage::real},
             PrgNumericMetadata{4U, 0U, static_cast<PrgNumericStorage>(255)}})
        expect(!value_associated_numeric_display(2.5, malformed, 2U, false),
               "malformed or unscoped metadata rejected");
    expect(!value_associated_numeric_display(2.5, valid, 19U, false),
           "unsupported setting rejected");
    expect(!value_associated_numeric_display(
               2.5, {1U, 0U, PrgNumericStorage::integer}, 2U, false),
           "fractional Integer payload rejected");
    expect(!value_associated_numeric_display(
               1000000.0, {10U, 0U, PrgNumericStorage::integer}, 18U, true),
           "scaled quotient wider than uint64 rejected");

    if (failures) return 1;
    std::cout << rows << " frozen value-associated Numeric displays and fail-closed controls passed\n";
}
