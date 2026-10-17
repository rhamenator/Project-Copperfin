// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// VR-5611-NUMERIC-RUNTIME-DIVISION-001: native constants, not helper-derived.
#include "../src/runtime/prg_numeric_runtime_division.h"
#include <bit>
#include <iostream>
#include <limits>
int main(){
    using namespace copperfin::runtime;
    int failures=0;const auto expect=[&](bool ok,const char* label){if(!ok){++failures;std::cerr<<"FAIL: "<<label<<'\n';}};
    const auto value=[](double n,PrgNumericMetadata m){PrgValue v;v.kind=PrgValueKind::number;v.number_value=n;v.numeric_metadata=m;return v;};
    const auto I=PrgNumericStorage::integer,R=PrgNumericStorage::real;
    struct Row{PrgNumericMetadata l,r;const char* setting;PrgNumericMetadata expected;};
    const Row rows[]={
        {{1,0,I},{1,0,I},"0",{4,2,R}},{{1,0,I},{1,0,I},"2",{6,4,R}},
        {{1,0,I},{1,0,I},"6",{10,8,R}},{{1,0,I},{1,0,I},"14",{18,16,R}},{{1,0,I},{1,0,I},"18",{20,18,R}},
        {{10,0,I},{10,0,I},"2",{24,4,R}},{{10,0,I},{10,0,I},"14",{36,16,R}},{{10,0,I},{10,0,I},"18",{38,18,R}},
        {{13,2,R},{13,2,R},"0",{23,2,R}},{{13,2,R},{13,2,R},"2",{25,4,R}},{{13,2,R},{13,2,R},"14",{37,16,R}},{{13,2,R},{13,2,R},"18",{39,18,R}},
        {{25,0,R},{1,0,I},"0",{28,2,R}},{{25,0,R},{1,0,I},"2",{30,4,R}},
        {{25,14,R},{1,0,I},"2",{25,14,R}},{{25,14,R},{1,0,I},"14",{27,16,R}},{{25,14,R},{1,0,I},"18",{29,18,R}},
        {{40,18,R},{40,18,R},"2",{40,18,R}},{{25,0,I},{25,0,I},"2",{40,4,R}},
        {{13,2,R},{10,0,I},"2",{24,4,R}},{{25,14,R},{4,2,R},"2",{26,14,R}}};
    for(const auto& row:rows){const auto l=value(5,row.l),r=value(2,row.r);expect(runtime_numeric_division_metadata(l,r,2.5,row.setting,true)==row.expected,"native metadata");expect(!runtime_numeric_division_metadata(l,r,2.5,row.setting,false),"constant context unknown");expect(l.numeric_metadata==row.l&&r.numeric_metadata==row.r,"input descriptors untouched");}
    const auto l=value(5,{10,0,I}),r=value(2,{10,0,I});
    for(const auto setting:{"","-1","19","999999999999999999999999999999","2.0","2x"," 2","+2","02"})expect(!runtime_numeric_division_metadata(l,r,2.5,setting,true),"invalid setting unknown");
    for(const auto n:{std::numeric_limits<double>::infinity(),-std::numeric_limits<double>::infinity(),std::numeric_limits<double>::quiet_NaN(),1e10,-1e10,1e300}){
        expect(!runtime_numeric_division_metadata(l,r,n,"2",true),"unsupported result unknown");
        expect(!runtime_numeric_division_metadata(value(n,{13,2,R}),r,2.5,"2",true),"unsupported operand unknown");}
    for(const auto m:{PrgNumericMetadata{0,0,R},PrgNumericMetadata{41,0,R},PrgNumericMetadata{4,19,R},PrgNumericMetadata{2,2,R},PrgNumericMetadata{std::numeric_limits<std::uint32_t>::max(),0,R}})expect(!runtime_numeric_division_metadata(value(5,m),r,2.5,"2",true),"malformed bounds unknown");
    expect(!runtime_numeric_division_metadata(value(5.5,{10,0,I}),r,2.75,"2",true),"invalid Integer payload unknown");expect(!runtime_numeric_division_metadata(value(2147483648.0,{10,0,I}),r,1073741824.0,"2",true),"Integer range checked");
    auto missing=l;missing.numeric_metadata.reset();expect(!runtime_numeric_division_metadata(missing,r,2.5,"2",true),"unknown metadata not inferred");
    auto null=l;null.is_null=true;expect(!runtime_numeric_division_metadata(null,r,2.5,"2",true),"NULL not admitted");auto exact=l;exact.kind=PrgValueKind::int64;expect(!runtime_numeric_division_metadata(exact,r,2.5,"2",true),"exact64 not admitted");auto currency=l;currency.kind=PrgValueKind::currency;expect(!runtime_numeric_division_metadata(currency,r,2.5,"2",true),"Currency not admitted");
    expect(!runtime_numeric_division_metadata(l,value(0,{10,0,I}),2.5,"2",true),"zero divisor unknown");
    expect(runtime_numeric_division_metadata(value(5,{10,std::numeric_limits<std::uint32_t>::max(),I}),r,2.5,"2",true)==PrgNumericMetadata{24,4,R},"Integer unused scale ignored");
    const auto negative=value(-0.0,{12,1,R});const auto bits=std::bit_cast<std::uint64_t>(negative.number_value);expect(runtime_numeric_division_metadata(negative,r,-0.0,"2",true)==PrgNumericMetadata{24,4,R},"zero metadata");expect(std::bit_cast<std::uint64_t>(negative.number_value)==bits,"negative zero input untouched");
    if(failures) {
        return 1;
    }
    std::cout<<"Runtime division helper native metadata and bounds passed\n";
}
