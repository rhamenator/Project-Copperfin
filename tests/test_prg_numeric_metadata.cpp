// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// RQ-CF-PRG-NUMERIC-METADATA-CARRIER-001; native provenance and scope:
// fixtures/vfp9-listitem-column-numeric/metadata-carrier-requirements.md
#include "copperfin/runtime/prg_engine.h"

#include <array>
#include <bit>
#include <cstdint>
#include <iostream>
#include <limits>
#include <map>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace {
using copperfin::runtime::PrgNumericMetadata;
using copperfin::runtime::PrgValue;
using copperfin::runtime::PrgValueKind;
int failures=0;
void expect(bool condition,const char* message) {
    if(!condition) { ++failures; std::cerr << message << '\n'; }
}
PrgValue numeric(std::uint64_t bits,PrgNumericMetadata metadata) {
    PrgValue value;
    value.kind=PrgValueKind::number;
    value.number_value=std::bit_cast<double>(bits);
    value.numeric_metadata=metadata;
    return value;
}
void check(const PrgValue& value,std::uint64_t bits,PrgNumericMetadata metadata) {
    expect(value.kind==PrgValueKind::number,"Numeric kind preserved");
    expect(std::bit_cast<std::uint64_t>(value.number_value)==bits,"payload bits preserved");
    expect(value.numeric_metadata==metadata,"value-associated metadata preserved");
}
PrgValue through_parameter_and_return(PrgValue value) { return value; }
void test_presence_and_native_distinctions() {
    static_assert(std::is_aggregate_v<PrgValue>);
    static_assert(std::is_aggregate_v<PrgNumericMetadata>);
    static_assert(sizeof(double)==sizeof(std::uint64_t));
    for(const auto kind:std::array{PrgValueKind::empty,PrgValueKind::boolean,
            PrgValueKind::number,PrgValueKind::string,PrgValueKind::int64,
            PrgValueKind::uint64,PrgValueKind::currency}) {
        PrgValue value;
        value.kind=kind;
        expect(!value.numeric_metadata,"existing kinds start without invented metadata");
        value.is_null=true;
        expect(!value.numeric_metadata,"NULL does not invent Numeric metadata");
    }
    // Actual assigned VFP9 descriptors: identical near-one payloads, width25/
    // decimals14 versus width13/decimals2; the carrier must distinguish them.
    constexpr std::uint64_t near_one=0x3FEFFFFFFFFFFFA6ULL;
    const auto scientific=numeric(near_one,{25,14});
    const auto power=numeric(near_one,{13,2});
    check(scientific,near_one,{25,14});
    check(power,near_one,{13,2});
    expect(scientific.numeric_metadata!=power.numeric_metadata,"same double is not metadata identity");
    // Native literal2.5 versus computed5/2 (assigned), same double but scale1/2.
    constexpr std::uint64_t fraction=0x4004000000000000ULL;
    check(numeric(fraction,{12,1}),fraction,{12,1});
    check(numeric(fraction,{13,2}),fraction,{13,2});
    // Native direct1.00 versus assigned1.00; represent both, do not infer width.
    check(numeric(0x3FF0000000000000ULL,{4,2}),0x3FF0000000000000ULL,{4,2});
    check(numeric(0x3FF0000000000000ULL,{13,2}),0x3FF0000000000000ULL,{13,2});
}
void test_owned_copy_paths() {
    constexpr std::uint64_t bits=0x3FEFFFFFFFFFFFA6ULL;
    auto source=numeric(bits,{25,14});
    const auto copied=source;
    PrgValue assigned;
    assigned=source;
    const auto returned=through_parameter_and_return(source);
    std::vector<PrgValue> array{source};
    std::map<std::string,PrgValue> locals{{"value",source}};
    source.numeric_metadata=PrgNumericMetadata{13,2};
    check(source,bits,{13,2});
    check(copied,bits,{25,14});
    check(assigned,bits,{25,14});
    check(returned,bits,{25,14});
    check(array.at(0),bits,{25,14});
    check(locals.at("value"),bits,{25,14});
    auto moved=std::move(assigned);
    check(moved,bits,{25,14});
    PrgValue move_assigned;
    move_assigned=std::move(moved);
    check(move_assigned,bits,{25,14});
    move_assigned=PrgValue{};
    expect(!move_assigned.numeric_metadata,"replacing a value clears prior metadata");
}
void test_no_implicit_payload_interpretation() {
    // Derived host robustness cases: these do not assert VFP acceptance or
    // authorize interpreting metadata as a buffer size/unchecked cast bound.
    for(const auto bits:std::array<std::uint64_t,8>{0,0x8000000000000000ULL,1,
            0x8000000000000001ULL,0x7FEFFFFFFFFFFFFFULL,0x7FF0000000000000ULL,
            0xFFF0000000000000ULL,0x7FF8000000001234ULL}) {
        auto value=numeric(bits,{25,14});
        check(through_parameter_and_return(value),bits,{25,14});
        value.numeric_metadata=PrgNumericMetadata{0,0};
        check(value,bits,{0,0});
        constexpr auto maximum=std::numeric_limits<std::uint32_t>::max();
        value.numeric_metadata=PrgNumericMetadata{maximum,maximum};
        check(value,bits,{maximum,maximum});
        value.numeric_metadata.reset();
        expect(!value.numeric_metadata,"metadata absence remains distinct from zero");
        expect(std::bit_cast<std::uint64_t>(value.number_value)==bits,"metadata reset does not round payload");
    }
}
} // namespace

int main() {
    test_presence_and_native_distinctions();
    test_owned_copy_paths();
    test_no_implicit_payload_interpretation();
    if(failures) return 1;
    std::cout << "Numeric metadata carrier: native distinctions, ownership and payload checks passed\n";
    return 0;
}
