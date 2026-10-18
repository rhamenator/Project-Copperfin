// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// VR-5611-NUMERIC-CONSTANT-DIVISION-INTRINSIC-001 and MATERIALIZATION-001.
#include "../src/runtime/prg_numeric_constant_division_intrinsic.h"
#include "../src/runtime/prg_numeric_constant_division_materialization.h"
#include "../src/runtime/prg_numeric_expression_context.h"
#include "../src/runtime/prg_numeric_unary_metadata.h"
#include "fixtures/vfp9-numeric-division-context/constant-intrinsic-expected.h"
#include <bit>
#include <iostream>
#include <limits>

int main() {
    using namespace copperfin::runtime;
    const auto I = PrgNumericStorage::integer, R = PrgNumericStorage::real;
    int failures = 0;
    const auto expect = [&](bool ok, const char* label) {
        if (!ok) { ++failures; std::cerr << "FAIL: " << label << '\n'; }
    };
    const auto value = [](double n, PrgNumericMetadata metadata) {
        PrgValue v; v.kind = PrgValueKind::number; v.number_value = n; v.numeric_metadata = metadata; return v;
    };
    unsigned checked = 0;
    for (const auto& row : native_constant_division_rows) {
        const auto l = value(std::bit_cast<double>(row.left_bits), {row.left_width, row.left_scale, row.left_integer ? I : R});
        const auto r = value(std::bit_cast<double>(row.right_bits), {row.right_width, row.right_scale, row.right_integer ? I : R});
        const auto result = constant_division_intrinsic(l, r, l.number_value / r.number_value, true);
        expect(result.has_value(), "native eligible pair");
        if (!result) continue;
        auto metadata = result->metadata;
        double payload = result->payload;
        if (row.grouped_minus) {
            const auto negated = negated_numeric_metadata(metadata);
            expect(negated.metadata.has_value() && !negated.overflow, "bounded existing unary composition");
            if (!negated.metadata) continue;
            metadata = *negated.metadata;
            payload = metadata.storage == I ? (payload == 0.0 ? 0.0 : -payload)
                : negated_numeric_expression_payload(payload, false, true);
        }
        const bool matched = metadata == PrgNumericMetadata{row.result_width, row.result_scale, row.result_integer ? I : R} &&
            std::bit_cast<std::uint64_t>(payload) == row.result_bits;
        if (!matched) std::cerr << "Native batch=" << row.batch << " row=" << row.index << '\n';
        expect(matched, "frozen native intrinsic metadata/payload");
        expect(l.numeric_metadata == PrgNumericMetadata{row.left_width, row.left_scale, row.left_integer ? I : R} &&
            r.numeric_metadata == PrgNumericMetadata{row.right_width, row.right_scale, row.right_integer ? I : R}, "input metadata unchanged");
        expect(std::bit_cast<std::uint64_t>(l.number_value) == row.left_bits && std::bit_cast<std::uint64_t>(r.number_value) == row.right_bits, "input bits unchanged");
        expect(!constant_division_intrinsic(l, r, l.number_value / r.number_value, false), "unproved constant node unknown");
        ++checked;
    }
    expect(checked == 640U, "complete frozen native inventory");
    const auto l = value(5, {1, 0, I}), r = value(2, {1, 0, I});
    for (const double n : {std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::infinity(),
                           -std::numeric_limits<double>::infinity(), 1e10, -1e10, 1e300}) {
        expect(!constant_division_intrinsic(l, r, n, true), "unsupported quotient unknown");
        expect(!constant_division_intrinsic(value(n, {4, 2, R}), r, 2.5, true), "unsupported operand unknown");
    }
    for (const auto m : {PrgNumericMetadata{0, 0, R}, PrgNumericMetadata{32, 0, R},
                         PrgNumericMetadata{4, 19, R}, PrgNumericMetadata{3, 2, R},
                         PrgNumericMetadata{13, 0, I}, PrgNumericMetadata{14, 0, R},
                         PrgNumericMetadata{std::numeric_limits<std::uint32_t>::max(), 0, R},
                         PrgNumericMetadata{1, 0, static_cast<PrgNumericStorage>(255)}}) {
        expect(!constant_division_intrinsic(value(5, m), r, 2.5, true), "unsupported/malformed metadata unknown");
    }
    expect(!constant_division_intrinsic(value(5.5, {4, 2, R}), r, 2.75, true), "nonwhole operand deferred");
    expect(!constant_division_intrinsic(l, value(0, {1, 0, I}), 2.5, true), "zero divisor unknown");
    expect(!constant_division_intrinsic(l, value(0.5, {3, 1, R}), 10.0, true), "small divisor deferred");
    expect(!constant_division_intrinsic(value(2147483648.0, {10, 0, I}), r, 1073741824.0, true), "native Integer bound checked");
    auto unknown = l; unknown.numeric_metadata.reset();
    expect(!constant_division_intrinsic(unknown, r, 2.5, true), "unknown metadata not inferred");
    auto null = l; null.is_null = true;
    expect(!constant_division_intrinsic(null, r, 2.5, true), "NULL not admitted");
    for (const auto kind : {PrgValueKind::currency, PrgValueKind::int64, PrgValueKind::uint64, PrgValueKind::boolean}) {
        auto other = l; other.kind = kind;
        expect(!constant_division_intrinsic(other, r, 2.5, true), "other kinds not admitted");
    }
    const auto unused = constant_division_intrinsic(value(5, {1, std::numeric_limits<std::uint32_t>::max(), I}), r, 2.5, true);
    expect(unused && unused->metadata == PrgNumericMetadata{1, 0, R}, "native Integer scale slot ignored");
    // Explicit native 5/2 root descriptors from the pre-hook header/captures.
    expect(materialized_constant_division_metadata({1, 0, R}, "0") ==
        PrgNumericMetadata{1, 0, R}, "native direct D0");
    expect(materialized_constant_division_metadata({1, 0, R}, "14") ==
        PrgNumericMetadata{16, 14, R}, "native direct D14");
    expect(materialized_constant_division_metadata({6, 4, R}, "2") ==
        PrgNumericMetadata{6, 4, R}, "native explicit scale exceeds setting");
    expect(materialized_constant_division_metadata({2, 0, I}, "18") ==
        PrgNumericMetadata{2, 0, I}, "native integral constant has no floor");
    for (const auto setting : {"", "-1", "+2", " 2", "2 ", "02", "19", "2x", "4294967296"})
        expect(!materialized_constant_division_metadata({1, 0, R}, setting),
               "invalid DECIMALS text fails closed");
    for (const auto metadata : {PrgNumericMetadata{0, 0, R}, PrgNumericMetadata{32, 0, R},
                               PrgNumericMetadata{3, 2, R}, PrgNumericMetadata{4, 19, R},
                               PrgNumericMetadata{13, 0, R}, PrgNumericMetadata{13, 0, I},
                               PrgNumericMetadata{1, 0, static_cast<PrgNumericStorage>(255)}})
        expect(!materialized_constant_division_metadata(metadata, "14"),
               "unsupported materialization metadata fails closed");
    if (failures) return 1;
    std::cout << "640 frozen native intrinsic rows, materialization and unsupported-domain controls passed\n";
}
