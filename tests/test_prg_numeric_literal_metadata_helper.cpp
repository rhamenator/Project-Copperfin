// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// RQ-CF-PRG-NUMERIC-LITERAL-METADATA-001; independent native-backed attributes.
#include "../src/runtime/prg_numeric_literal_metadata.h"
#include <iostream>
#include <string>
#include <string_view>

int main() {
    using copperfin::runtime::plain_decimal_literal_metadata;
    using copperfin::runtime::PrgNumericMetadata;
    int failures = 0;
    const auto check = [&](std::string_view source, PrgNumericMetadata expected) {
        if (plain_decimal_literal_metadata(source) != expected) {
            ++failures; std::cerr << "plain decimal metadata mismatch: " << source << '\n';
        }
    };
    check("1.0", {3,1}); check("1.00", {4,2}); check("001.2300", {8,4});
    check("0.0000", {6,4}); check("0.0000000000000000001", {21,18});
    check("123456789012345678901.0", {23,1});
    check("1." + std::string(17,'0'), {19,17});
    check("1." + std::string(18,'0'), {20,18});
    check("1." + std::string(38,'0'), {40,18});
    check("1." + std::string(39,'0'), {40,18});
    check("1." + std::string(254,'0'), {40,18});
    check(std::string(254,'0') + ".1", {40,1});
    for (const auto source : {"", "1", "1.", ".1", "-1.00", "+1.00", "1E-14",
            "1.00E2", " 1.00", "1.00 ", "(1.00)", "1..0", "NaN", "inf", "1,00"}) {
        if (plain_decimal_literal_metadata(source)) {
            ++failures; std::cerr << "unsupported metadata inferred: " << source << '\n';
        }
    }
    for (const auto& source : {std::string("1.\0",3), "1." + std::string(255,'0'),
            std::string(1024*1024,'1')}) {
        if (plain_decimal_literal_metadata(source)) {
            ++failures; std::cerr << "unsupported long/NUL metadata inferred\n";
        }
    }
    if (failures) return 1;
    std::cout << "Plain decimal metadata helper: native and unsupported-domain checks passed\n";
    return 0;
}
