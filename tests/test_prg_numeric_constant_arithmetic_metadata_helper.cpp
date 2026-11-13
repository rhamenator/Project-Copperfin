// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
#include "../src/runtime/prg_numeric_constant_arithmetic_metadata.h"

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
    const auto integer = [&](double n, unsigned width) { return value(n, {width, 0, I}); };
    for (const unsigned scale : {2U, 6U, 14U}) {
        const auto setting = scale == 2U ? "2" : scale == 6U ? "6" : "14";
        const auto power = constant_numeric_arithmetic_metadata(
            ConstantNumericArithmeticOperation::power, integer(2, 1), integer(-53, 3),
            std::pow(2.0, -53.0), setting, true);
        expect(power == ConstantNumericArithmeticMetadata{{scale + 15U, scale, R},
            {scale + 11U, scale, R}}, "native power attributes");
        if (!power) continue;
        const auto powered = value(std::pow(2.0, -53.0), power->intrinsic);
        const auto product = constant_numeric_arithmetic_metadata(
            ConstantNumericArithmeticOperation::multiplication, integer(90, 2), powered,
            90.0 * powered.number_value, setting, true);
        expect(product == power, "native multiplication attributes");
        if (!product) continue;
        const auto multiplied = value(90.0 * powered.number_value, product->intrinsic);
        const auto difference = constant_numeric_arithmetic_metadata(
            ConstantNumericArithmeticOperation::subtraction, integer(1, 1), multiplied,
            1.0 - multiplied.number_value, setting, true);
        expect(difference == power, "native computed additive attributes");
    }
    const auto scientific = constant_numeric_arithmetic_metadata(
        ConstantNumericArithmeticOperation::subtraction, integer(1, 1), value(1e-14, {15, 14, R}),
        1.0 - 1e-14, "2", true);
    expect(scientific == ConstantNumericArithmeticMetadata{{17, 14, R}, {25, 14, R}},
        "native scientific additive attributes independent of DECIMALS");
    const auto one = integer(1, 1);
    for (const auto setting : {"", "02", "-1", "+2", "19", "2x"})
        expect(!constant_numeric_arithmetic_metadata(ConstantNumericArithmeticOperation::power,
            integer(2, 1), integer(-53, 3), 1.0, setting, true), "invalid setting fails closed");
    expect(!constant_numeric_arithmetic_metadata(ConstantNumericArithmeticOperation::power,
        integer(2, 1), integer(-53, 3), 1.0, "2", false), "unproved node fails closed");
    expect(!constant_numeric_arithmetic_metadata(ConstantNumericArithmeticOperation::power,
        integer(2, 1), integer(53, 2), 1.0, "2", true), "positive exponent deferred");
    expect(!constant_numeric_arithmetic_metadata(ConstantNumericArithmeticOperation::multiplication,
        one, value(0.5, {3, 1, R}), 0.5, "2", true), "ordinary multiplication deferred");
    auto unknown = one; unknown.numeric_metadata.reset();
    expect(!constant_numeric_arithmetic_metadata(ConstantNumericArithmeticOperation::subtraction,
        unknown, value(0.1, {3, 1, R}), 0.9, "2", true), "unknown metadata deferred");
    expect(!constant_numeric_arithmetic_metadata(ConstantNumericArithmeticOperation::subtraction,
        one, value(std::numeric_limits<double>::infinity(), {17, 2, R}), 0.0, "2", true),
        "nonfinite operand deferred");
    expect(!constant_numeric_arithmetic_metadata(ConstantNumericArithmeticOperation::subtraction,
        one, value(0.1, {32, 1, R}), 0.9, "2", true), "invalid width deferred");
    if (failures) return 1;
    std::cout << "native constant arithmetic metadata frontier and fail-closed controls passed\n";
}
