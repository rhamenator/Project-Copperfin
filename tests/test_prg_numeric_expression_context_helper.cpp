// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// RQ-CF-PRG-NUMERIC-UNARY-ZERO-CONTEXT-001: pure scope/payload contract.
#include "../src/runtime/prg_numeric_expression_context.h"
#include <array>
#include <bit>
#include <cstdint>
#include <iostream>
#include <stdexcept>

int main() {
    using namespace copperfin::runtime;
    int failures=0;
    const auto expect=[&](bool condition,const char* message){if(!condition){++failures;std::cerr<<"FAIL: "<<message<<'\n';}};
    bool runtime=true;
    {
        ScopedNumericExpressionContext outer(runtime);
        expect(!runtime,"operand scope independent of earlier runtime primary");
        {
            ScopedNumericExpressionContext inner(runtime);
            runtime=true;
        }
        expect(runtime,"nested runtime primary propagated");
        {ScopedNumericExpressionContext constant(runtime);expect(!runtime,"constant suboperand isolated");}
        expect(runtime,"enclosing runtime context restored");
    }
    expect(runtime,"previous outer context retained");
    runtime=false;
    try {ScopedNumericExpressionContext scope(runtime);runtime=true;throw std::runtime_error("control");}
    catch(const std::runtime_error&){}
    expect(runtime,"exception retains observed context");
    runtime=false;
    {ScopedNumericExpressionContext scope(runtime);}
    expect(!runtime,"constant-only scope remains constant");
    struct Row{std::uint64_t input;bool runtime;bool known;std::uint64_t output;};
    constexpr std::uint64_t minusZero=0x8000000000000000ULL;
    const std::array<Row,10> rows={{{0,false,true,minusZero},{minusZero,false,true,minusZero},
        {0,true,true,0},{minusZero,true,true,minusZero},{0,true,false,0},{minusZero,true,false,minusZero},
        {0,false,false,minusZero},{minusZero,false,false,0},
        {0x3FF0000000000000ULL,true,true,0xBFF0000000000000ULL},
        {0x7FF8000000000042ULL,false,true,0xFFF8000000000042ULL}}};
    for(const auto& row:rows)expect(std::bit_cast<std::uint64_t>(negated_numeric_expression_payload(
        std::bit_cast<double>(row.input),row.runtime,row.known))==row.output,"native zero or unchanged nonzero/unknown control bits");
    if(failures)return 1;
    std::cout<<"Scoped expression context and bounded zero payload passed\n";
}
