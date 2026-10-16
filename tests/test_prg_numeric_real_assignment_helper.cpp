// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// VR-5611-NUMERIC-REAL-ASSIGNMENT-001: native constants, not helper-generated.
#include "../src/runtime/prg_numeric_real_assignment.h"
#include <bit>
#include <iostream>
#include <limits>

int main() {
    using namespace copperfin::runtime;
    int failures=0;
    const auto expect=[&](bool ok,const char* text){if(!ok){++failures;std::cerr<<"FAIL: "<<text<<'\n';}};
    struct Row{double payload;std::uint32_t decimals,width;};
    const Row rows[]={{1,1,12},{-1,1,13},{1,2,13},{-1,2,14},{1,18,29},{-1,18,30},
        {0.0,1,12},{-0.0,1,12},{0.0,18,29},{-0.0,18,29},
        {1e-18,18,29},{-1e-18,18,30},{9999999999.0,1,12},{-9999999999.0,1,13},
        {5.0,2,13},{-5.0,2,14}};
    for(const auto row:rows)for(const auto oldWidth:{1U,40U,std::numeric_limits<std::uint32_t>::max()}) {
        const auto bits=std::bit_cast<std::uint64_t>(row.payload);
        expect(assigned_real_numeric_metadata(PrgNumericMetadata{oldWidth,row.decimals},row.payload)==
            PrgNumericMetadata{row.width,row.decimals},"native width/scale/storage");
        expect(std::bit_cast<std::uint64_t>(row.payload)==bits,"input bits unchanged");
    }
    for(const auto decimals:{0U,19U,std::numeric_limits<std::uint32_t>::max()}) {
        const PrgNumericMetadata unsupported{7U,decimals};
        expect(assigned_real_numeric_metadata(unsupported,1.0)==unsupported,"unsupported scale unchanged");
    }
    const PrgNumericMetadata real{7U,9U},integer{7U,0U,PrgNumericStorage::integer};
    for(const auto payload:{1.0,-0.0,1e10,-1e10,1e300,std::numeric_limits<double>::infinity(),
            -std::numeric_limits<double>::infinity(),std::numeric_limits<double>::quiet_NaN()}) {
        expect(!assigned_real_numeric_metadata(std::nullopt,payload),"unknown not inferred");
        expect(assigned_real_numeric_metadata(integer,payload)==integer,"Integer untouched");
    }
    for(const auto payload:{1e10,-1e10,1e300,std::numeric_limits<double>::infinity(),
            -std::numeric_limits<double>::infinity(),std::numeric_limits<double>::quiet_NaN()})
        expect(assigned_real_numeric_metadata(real,payload)==real,"unsupported magnitude/nonfinite unchanged");
    if(failures)return 1;
    std::cout<<"Real Numeric assignment helper: native descriptors and bounded controls passed\n";
}
