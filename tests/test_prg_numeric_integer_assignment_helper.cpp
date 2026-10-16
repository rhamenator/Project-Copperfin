// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// VR-5611-NUMERIC-INTEGER-ASSIGNMENT-001: independent native expected widths.
#include "../src/runtime/prg_numeric_integer_assignment.h"
#include <array>
#include <iostream>
#include <limits>

int main() {
    using namespace copperfin::runtime;
    int failures=0;
    const auto expect=[&](bool ok,const char* text){if(!ok){++failures;std::cerr<<"FAIL: "<<text<<'\n';}};
    struct Row{double payload;std::uint32_t width;};
    const std::array<Row,12> rows={{{0.0,10},{-0.0,10},{1.0,10},{-1.0,10},
        {5.0,10},{-5.0,10},{999999999.0,10},{-999999999.0,10},
        {1000000000.0,10},{-1000000000.0,11},{2147483647.0,10},{-2147483647.0,11}}};
    for(const auto row:rows)for(const auto sourceWidth:{1U,10U,11U,20U,std::numeric_limits<std::uint32_t>::max()}){
        expect(assigned_integer_numeric_metadata(PrgNumericMetadata{sourceWidth,123U,PrgNumericStorage::integer},row.payload)==
            PrgNumericMetadata{row.width,0U,PrgNumericStorage::integer},"native Integer assigned width/ignored decimal slot");
    }
    const PrgNumericMetadata integer{7U,99U,PrgNumericStorage::integer},real{7U,2U};
    for(const auto payload:{0.0,-0.0,5.0,1.5,std::numeric_limits<double>::infinity(),
                           -std::numeric_limits<double>::infinity(),std::numeric_limits<double>::quiet_NaN()}){
        expect(!assigned_integer_numeric_metadata(std::nullopt,payload),"unknown not inferred");
        expect(assigned_integer_numeric_metadata(real,payload)==real,"real descriptor untouched");
    }
    for(const auto payload:{-2147483648.0,2147483648.0,1.5,-1.5,1e300,
                           std::numeric_limits<double>::infinity(),-std::numeric_limits<double>::infinity(),
                           std::numeric_limits<double>::quiet_NaN()}){
        expect(assigned_integer_numeric_metadata(integer,payload)==integer,"unsupported Integer payload unchanged");
    }
    if(failures)return 1;
    std::cout<<"Integer assignment helper: independent native widths and unsupported controls passed\n";
}
