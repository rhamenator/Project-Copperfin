// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// RQ-CF-PRG-NUMERIC-DECIMAL-LITERAL-PAYLOAD-001: native fields; added in UNUSED support phase.
#include "../src/runtime/prg_numeric_literal_payload.h"
#include "fixtures/vfp9-numeric-literal-payload/native-expected.h"

#include <bit>
#include <cfenv>
#include <cstdint>
#include <iostream>
#include <string>
#include <string_view>

namespace {
struct RestoreRounding {
    int original=std::fegetround();
    ~RestoreRounding(){if(original!=-1)std::fesetround(original);}
};
}

int main() {
    using copperfin::runtime::plain_decimal_literal_payload;
    unsigned failures=0,selected=0;
    const auto expect=[&](bool condition,std::string_view label) {
        if(!condition){++failures;std::cerr<<"FAIL: "<<label<<'\n';}
    };
    RestoreRounding rounding;
    expect(rounding.original==FE_TONEAREST,"native-backed nearest-rounding environment");
    for(const auto& row:copperfin::test::native_literal_payload_rows) {
        // Helper accepts unsigned literal tokens, not an expression evaluator.
        // The independent public test still asserts ALL1193 native rows.
        if(!row.plain_literal)continue;
        ++selected;
        const auto value=plain_decimal_literal_payload(row.expression);
        expect(value.has_value(),row.label);
        if(value)expect(std::bit_cast<std::uint64_t>(*value)==row.bits,row.expression);
    }
    expect(selected==458,"all245 profile +213 held-out unsigned literal inputs");
    for(const auto input:{"","0","0.",".1","1e-14","-1.0","+1.0"," 1.0","1.0 ",
                          "1..0","$1.0","NaN","inf","1_0.0","0x1.0"})
        expect(!plain_decimal_literal_payload(input),"unsupported shape stays unspecified");
    expect(!plain_decimal_literal_payload(std::string_view("1.0\0",4)),"embedded NUL rejected");
    expect(!plain_decimal_literal_payload("\xC2\xB2.0"),"non-ASCII byte shape rejected");
    expect(!plain_decimal_literal_payload(std::string(255,'0')+".1"),"257-byte producer boundary");
    expect(!plain_decimal_literal_payload(std::string(1'000'000,'9')),"large input rejected before traversal");
    expect(std::fegetround()==rounding.original,"ordinary helper calls preserve rounding mode");

    // Unknown floating environments are not silently called native-compatible.
    // Guard returns unspecified without changing caller state; restore in all paths.
    expect(std::fesetround(FE_UPWARD)==0,"establish scoped upward rounding control");
    expect(!plain_decimal_literal_payload("1.25"),"non-nearest environment stays unspecified");
    expect(std::fegetround()==FE_UPWARD,"helper does not modify caller rounding mode");
    expect(std::fesetround(rounding.original)==0,"restore original rounding mode");
    expect(std::fegetround()==rounding.original,"scoped environment restored");
    std::cout<<"literal payload helper: selected="<<selected<<" failures="<<failures<<'\n';
    return failures?1:0;
}
