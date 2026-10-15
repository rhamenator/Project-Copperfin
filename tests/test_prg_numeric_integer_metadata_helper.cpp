// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// RQ-CF-PRG-NUMERIC-INTEGER-DESCRIPTOR-001: pure typed carrier/classifier/unary contract.
#include "../src/runtime/prg_numeric_integer_metadata.h"
#include "../src/runtime/prg_numeric_unary_metadata.h"
#include <bit>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

int main() {
    using namespace copperfin::runtime;
    int failures=0;
    const auto expect=[&](bool condition,const char* message){if(!condition){++failures;std::cerr<<"FAIL: "<<message<<'\n';}};
    expect(PrgNumericMetadata{4,2}.storage==PrgNumericStorage::real,"old aggregate defaults real");
    for(const auto& [source,width]:std::vector<std::pair<std::string,std::uint32_t>>{
        {"0",1},{"5",1},{"0005",4},{"0000000005",10},{"2147483646",10},{"2147483647",10},{"0000000000",10}}) {
        const auto m=plain_integer_literal_metadata(source);
        expect(m==PrgNumericMetadata{width,0,PrgNumericStorage::integer},"native staged Integer descriptor");
        const auto minus=negated_numeric_metadata(m);
        expect(!minus.overflow&&minus.metadata==PrgNumericMetadata{width+1U,0,PrgNumericStorage::integer},"Integer width/storage propagation");
    }
    for(const std::string source:{"","-1","+1","1.0",".0","1e0","1I64","1U64"," 1","1 ","2147483648","4294967295","00000000005","00000000000","9999999999"})
        expect(!plain_integer_literal_metadata(source),"unsupported metadata stays unknown, not syntax rejection");
    const auto overflow=negated_numeric_metadata(PrgNumericMetadata{std::numeric_limits<std::uint32_t>::max(),0,PrgNumericStorage::integer});
    expect(overflow.overflow&&!overflow.metadata,"checked Integer descriptor width overflow");
    expect(negated_numeric_metadata(PrgNumericMetadata{4,2}).metadata==PrgNumericMetadata{5,2},"known real attributes unchanged");
    expect(!negated_numeric_metadata(std::nullopt).metadata,"unknown stays unknown");
    PrgValue original;original.kind=PrgValueKind::number;original.number_value=5.0;
    original.numeric_metadata=PrgNumericMetadata{4,0,PrgNumericStorage::integer};
    auto copy=original;original.numeric_metadata->display_width=99;
    expect(copy.numeric_metadata==PrgNumericMetadata{4,0,PrgNumericStorage::integer},"typed value copy independent");
    expect(copy.number_value==5.0&&copy.kind==PrgValueKind::number,"native Integer not exact64 extension or changed payload");
    for(const std::uint64_t input:{0ULL,0x8000000000000000ULL})
        expect(std::bit_cast<std::uint64_t>(negated_integer_numeric_payload(std::bit_cast<double>(input)))==0,"known Integer zero positive");
    expect(negated_integer_numeric_payload(2147483647.0)==-2147483647.0,"nonzero boundary payload unchanged");
    if(failures)return 1;
    std::cout<<"Typed carrier and bounded Integer helper passed\n";
}
