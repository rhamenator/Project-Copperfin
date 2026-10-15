// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// RQ-CF-PRG-NUMERIC-UNARY-ATTRIBUTES-001 / VR-5611-NUMERIC-UNARY-ATTRIBUTES-001.
// Attribute-only native vectors plus synthetic host-representation safety.
#include "../src/runtime/prg_numeric_unary_metadata.h"

#include <array>
#include <iostream>
#include <limits>

int main() {
    using namespace copperfin::runtime;
    int failures=0;
    const auto expect=[&](bool condition,const char* label) {
        if (!condition) { ++failures; std::cerr << "FAIL: " << label << '\n'; }
    };
    const auto unknown=negated_numeric_metadata(std::nullopt);
    expect(!unknown.metadata && !unknown.overflow,"absent metadata stays unknown");
    struct Row { PrgNumericMetadata source; PrgNumericMetadata expected; };
    constexpr auto maximum=std::numeric_limits<std::uint32_t>::max();
    const std::array<Row,10> rows={{{{4,2},{5,2}},{{5,2},{6,2}},{{6,2},{7,2}},
        {{7,4},{8,4}},{{13,2},{14,2}},{{14,2},{15,2}},{{40,18},{41,18}},
        {{41,18},{42,18}},{{0,0},{1,0}},{{maximum-1U,maximum},{maximum,maximum}}}};
    for (const auto& row:rows) {
        const std::optional<PrgNumericMetadata> input=row.source;
        const auto result=negated_numeric_metadata(input);
        expect(!result.overflow && result.metadata==row.expected,"width increment and unchanged scale");
        expect(input==row.source,"input descriptor not mutated");
    }
    for (const auto scale:std::array<std::uint32_t,3>{0,18,maximum}) {
        const auto overflow=negated_numeric_metadata(PrgNumericMetadata{maximum,scale});
        expect(overflow.overflow && !overflow.metadata,"synthetic host width cannot wrap");
    }
    if (failures) return 1;
    std::cout << "Unary attributes: native vectors and synthetic checked-width boundaries passed\n";
    return 0;
}
