// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// VR-5611-NUMERIC-CONSTANT-DIVISION-NONWHOLE-001: independent native roots.
#include "../src/runtime/prg_numeric_constant_division_intrinsic.h"
#include "../src/runtime/prg_numeric_expression_context.h"
#include "../src/runtime/prg_numeric_unary_metadata.h"
#include "fixtures/vfp9-numeric-division-context/nonwhole-direct-expected.h"
#include "fixtures/vfp9-numeric-division-context/constant-intrinsic-expected.h"
#include <array>
#include <bit>
#include <iostream>
#include <limits>

int main() {
    using namespace copperfin::runtime;
    using namespace copperfin::test;
    const auto I = PrgNumericStorage::integer, R = PrgNumericStorage::real;
    unsigned failures = 0;
    const auto expect = [&](bool ok, const char* label) {
        if (!ok) { ++failures; std::cerr << "FAIL: " << label << '\n'; }
    };
    const auto value = [](double n, PrgNumericMetadata metadata) {
        PrgValue v; v.kind = PrgValueKind::number; v.number_value = n;
        v.numeric_metadata = metadata; return v;
    };
    const auto negate = [&](PrgValue& v) {
        const auto m = negated_numeric_metadata(v.numeric_metadata);
        expect(m.metadata.has_value() && !m.overflow, "existing unary attributes bounded");
        if (!m.metadata) return;
        v.numeric_metadata = m.metadata;
        v.number_value = m.metadata->storage == I ? (v.number_value == 0.0 ? 0.0 : -v.number_value)
            : negated_numeric_expression_payload(v.number_value, false, true);
    };
    // Syntax-only postorder tree: no separately evaluated child result is an
    // expected descriptor. Only captured literals and complete roots are oracles.
    std::array<PrgValue, std::size(native_nonwhole_nodes)> nodes{};
    for (std::size_t n = 0; n < nodes.size(); ++n) {
        const auto& source = native_nonwhole_nodes[n];
        if (source.op == NativeNonwholeOp::literal) {
            nodes[n] = value(std::bit_cast<double>(source.bits), source.metadata);
            continue;
        }
        expect(source.left < n, "postorder left child");
        if (source.left >= n) continue;
        if (source.op == NativeNonwholeOp::negate) {
            nodes[n] = nodes[source.left]; negate(nodes[n]); continue;
        }
        expect(source.right < n, "postorder right child");
        if (source.right >= n) continue;
        const auto& l = nodes[source.left]; const auto& r = nodes[source.right];
        const auto lb = std::bit_cast<std::uint64_t>(l.number_value);
        const auto rb = std::bit_cast<std::uint64_t>(r.number_value);
        const auto lm = l.numeric_metadata, rm = r.numeric_metadata;
        const auto q = l.number_value / r.number_value;
        const auto result = constant_nonwhole_division_intrinsic(l, r, q, true);
        expect(result.has_value(), "native nonwhole tree admitted");
        if (result) nodes[n] = value(result->payload, result->metadata);
        expect(!constant_nonwhole_division_intrinsic(l, r, q, false), "unproved provenance rejected");
        expect(lb == std::bit_cast<std::uint64_t>(l.number_value) &&
               rb == std::bit_cast<std::uint64_t>(r.number_value) &&
               lm == l.numeric_metadata && rm == r.numeric_metadata, "tree operands unchanged");
    }
    unsigned roots = 0;
    for (const auto& row : native_nonwhole_roots) {
        expect(row.node < nodes.size(), "root index bounded");
        if (row.node >= nodes.size()) continue;
        const auto& actual = nodes[row.node];
        const bool matched = actual.kind == PrgValueKind::number && !actual.is_null &&
            actual.numeric_metadata == row.metadata &&
            std::bit_cast<std::uint64_t>(actual.number_value) == row.bits;
        if (!matched) std::cerr << "Native nonwhole core=" << row.core << '\n';
        expect(matched, "complete native root metadata and payload"); ++roots;
    }
    expect(roots == 68U, "all frozen native nonwhole roots");
    unsigned preserved = 0;
    for (const auto& row : native_constant_division_rows) {
        const auto l = value(std::bit_cast<double>(row.left_bits), {row.left_width,row.left_scale,row.left_integer ? I : R});
        const auto r = value(std::bit_cast<double>(row.right_bits), {row.right_width,row.right_scale,row.right_integer ? I : R});
        const auto q = l.number_value / r.number_value;
        const auto result = constant_nonwhole_division_intrinsic(l,r,q,true);
        const auto old = constant_division_intrinsic(l,r,q,true);
        expect(result && old && result->metadata == old->metadata &&
               std::bit_cast<std::uint64_t>(result->payload) == std::bit_cast<std::uint64_t>(old->payload),
               "old helper behavior unchanged");
        if (!result) continue;
        auto actual = value(result->payload,result->metadata);
        if (row.grouped_minus) negate(actual);
        expect(actual.numeric_metadata == PrgNumericMetadata{row.result_width,row.result_scale,row.result_integer ? I : R} &&
               std::bit_cast<std::uint64_t>(actual.number_value) == row.result_bits,
               "all older native intrinsic rows preserved"); ++preserved;
    }
    expect(preserved == 640U, "complete older native inventory");
    const auto l = value(5.5,{4,2,R}), r = value(2,{1,0,I});
    expect(!constant_division_intrinsic(l,r,2.75,true), "old helper nonwhole frontier unchanged");
    expect(constant_nonwhole_division_intrinsic(l,r,2.75,true).has_value(), "new real frontier admitted");
    expect(!constant_nonwhole_division_intrinsic(value(5.5,{4,0,I}),r,2.75,true), "fractional Integer tag rejected");
    for (const double n : {std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::infinity(),
                           -std::numeric_limits<double>::infinity(), 1e10, -1e10, 1e300}) {
        expect(!constant_nonwhole_division_intrinsic(l,r,n,true), "unsupported quotient rejected");
        expect(!constant_nonwhole_division_intrinsic(value(n,{4,2,R}),r,2.75,true), "unsupported left rejected");
        expect(!constant_nonwhole_division_intrinsic(l,value(n,{4,2,R}),2.75,true), "unsupported right rejected");
    }
    for (const auto m : {PrgNumericMetadata{0,0,R}, PrgNumericMetadata{32,0,R},
                         PrgNumericMetadata{4,19,R}, PrgNumericMetadata{3,2,R},
                         PrgNumericMetadata{13,0,I}, PrgNumericMetadata{14,0,R},
                         PrgNumericMetadata{std::numeric_limits<std::uint32_t>::max(),0,R},
                         PrgNumericMetadata{1,0,static_cast<PrgNumericStorage>(255)}}) {
        expect(!constant_nonwhole_division_intrinsic(value(5,m),r,2.5,true), "invalid left metadata rejected");
        expect(!constant_nonwhole_division_intrinsic(l,value(2,m),2.75,true), "invalid right metadata rejected");
    }
    for (const double n : {0.0,-0.0,0.5,-0.5})
        expect(!constant_nonwhole_division_intrinsic(l,value(n,{4,2,R}),2.75,true), "small or zero divisor rejected");
    expect(!constant_nonwhole_division_intrinsic(value(2147483648.0,{10,0,I}),r,1073741824.0,true), "Integer bound checked");
    for (const bool on_left : {true,false}) {
        const auto reject = [&](const PrgValue& bad) {
            expect(!constant_nonwhole_division_intrinsic(on_left ? bad : l,on_left ? r : bad,2.75,true),
                   "unknown NULL or non-Number operand rejected");
        };
        auto bad = l; bad.numeric_metadata.reset(); reject(bad);
        bad = l; bad.is_null = true; reject(bad);
        for (const auto kind : {PrgValueKind::currency,PrgValueKind::int64,PrgValueKind::uint64,PrgValueKind::boolean}) {
            bad = l; bad.kind = kind; reject(bad);
        }
    }
    if (failures) return 1;
    std::cout << "68 native nonwhole roots/314 syntax nodes, 640 older native rows and bounded-domain controls passed\n";
}
