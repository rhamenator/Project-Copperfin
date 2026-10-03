// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/runtime/prg_engine.h"
#include "copperfin/vfp/dbf_table.h"
#include "../src/runtime/prg_engine_helpers.h"
#include "prg_engine_test_support.h"

#include <cstdint>
#include <filesystem>
#include <iostream>
#include <limits>
#include <string>
#include <system_error>
#include <vector>

namespace {

using namespace copperfin::test_support;

namespace fs = std::filesystem;

// Governing requirement: RQ-CF-PRG-NUMERIC-BEHAVIOR-001 (#6776, covering #5611, #6029).
//
// `SET NUMERICBEHAVIOR TO COPPERFIN | VFP9` selects how a Numeric argument that is out of range for the integer a
// function needs is handled. COPPERFIN (the default) truncates toward zero and saturates, so a huge count means "all"
// and a huge negative one "none". VFP9 reproduces installed VFP9 SP2, which converts such counts through a 32-bit
// integer: truncate to int64 (NaN, infinite or outside int64 gives the CPU's integer indefinite, low 32 bits 0), then
// take the low 32 bits as a signed int. The VFP9 expectations below are the probe results retained in
// ~/temp/vfp9-probes/numconv-6776/probe1.txt (a Windows 11 VM, VFP9 SP2 hotfix 3, 2026-10-02), not derived from the
// model, so the model is itself under test.

struct Value {
    const char *text;
    const char *copperfin_all_or_none;  // COPPERFIN result for a 6-character string: "all", "none", or the exact text
    const char *vfp9_left;              // VFP9 result of LEFT/RIGHT/LEFTC/RIGHTC('abcdef', value)
};

const std::vector<Value> kValues = {
    {"1E20", "all", ""},
    {"-1E20", "none", ""},
    {"1E300", "all", ""},
    {"2147483647", "all", "all"},
    {"2147483648", "all", ""},
    {"-2147483648", "none", ""},
    {"-2147483649", "none", "all"},
    {"4294967295", "all", ""},
    {"4294967296", "all", ""},
    {"9007199254740992", "all", ""},
    {"1E10", "all", "all"},
    {"0.5", "none", ""},
    {"2.9", "ab", "ab"},
    {"-2.9", "none", ""},
};

std::string expected_for(const char *function, const std::string &marker) {
    const bool right = std::string(function).rfind("RIGHT", 0U) == 0U;
    if (marker == "all") {
        return "abcdef";
    }
    if (marker == "none" || marker.empty()) {
        return "";
    }
    return right ? "ef" : "ab";  // the only other marker used is "ab", the count 2.9
}

struct Row {
    std::string setup;       // statement run before the expression, e.g. SET NUMERICBEHAVIOR TO VFP9
    std::string expression;
    std::string expected;    // "C:<text>" or "ERR<number>"
};

std::vector<Row> build_rows() {
    std::vector<Row> rows;
    // The default (never set) is COPPERFIN. This must be the first row: every later row sets the mode.
    rows.push_back({"", "SET('NUMERICBEHAVIOR')", "C:COPPERFIN"});
    for (const char *function : {"LEFT", "RIGHT", "LEFTC", "RIGHTC"}) {
        for (const Value &value : kValues) {
            const std::string expression = std::string(function) + "('abcdef'," + value.text + ")";
            rows.push_back({"SET NUMERICBEHAVIOR TO COPPERFIN", expression,
                            "C:" + expected_for(function, value.copperfin_all_or_none)});
            rows.push_back({"SET NUMERICBEHAVIOR TO VFP9", expression,
                            "C:" + expected_for(function, value.vfp9_left)});
        }
    }
    // The setting is reported and validated.
    rows.push_back({"SET NUMERICBEHAVIOR TO VFP9", "SET('NUMERICBEHAVIOR')", "C:VFP9"});
    rows.push_back({"SET NUMERICBEHAVIOR TO 'copperfin'", "SET('NUMERICBEHAVIOR')", "C:COPPERFIN"});
    rows.push_back({"SET NUMERICBEHAVIOR TO vfp9", "SET('NUMERICBEHAVIOR')", "C:VFP9"});
    rows.push_back({"SET NUMERICBEHAVIOR TO BOGUS", "SET('NUMERICBEHAVIOR')", "ERR11"});
    // Quoting forms are accepted; an omitted, empty or unknown value is error 11 and leaves the setting unchanged.
    rows.push_back({"SET NUMERICBEHAVIOR TO \"VFP9\"", "SET('NUMERICBEHAVIOR')", "C:VFP9"});
    rows.push_back({"SET NUMERICBEHAVIOR TO \"copperfin\"", "SET('NUMERICBEHAVIOR')", "C:COPPERFIN"});
    rows.push_back({"SET NUMERICBEHAVIOR TO VFP9", "SET('NUMERICBEHAVIOR')", "C:VFP9"});
    rows.push_back({"SET NUMERICBEHAVIOR", "SET('NUMERICBEHAVIOR')", "ERR11"});
    rows.push_back({"SET NUMERICBEHAVIOR TO", "SET('NUMERICBEHAVIOR')", "ERR11"});
    rows.push_back({"SET NUMERICBEHAVIOR TO ''", "SET('NUMERICBEHAVIOR')", "ERR11"});
    rows.push_back({"SET NUMERICBEHAVIOR TO \"\"", "SET('NUMERICBEHAVIOR')", "ERR11"});
    rows.push_back({"SET NUMERICBEHAVIOR TO 'vfp9 '", "SET('NUMERICBEHAVIOR')", "C:VFP9"});
    rows.push_back({"SET NUMERICBEHAVIOR TO 'vfp9x'", "SET('NUMERICBEHAVIOR')", "ERR11"});
    // After the rejected forms the setting still reads VFP9 (it is never reset).
    rows.push_back({"", "SET('NUMERICBEHAVIOR')", "C:VFP9"});
    // Slice 2 (#6029): SUBSTR and STUFF. Values from probe1.txt; STUFF and the SUBSTR length saturate in VFP9 too, so
    // those rows are identical in both modes. SUBSTR's start is probed below (VFP9 wraps; COPPERFIN saturates).
    for (const char *mode : {"COPPERFIN", "VFP9"}) {
        const std::string set = std::string("SET NUMERICBEHAVIOR TO ") + mode;
        rows.push_back({set, "STUFF('abcdef',1E20,1,'X')", "C:abcdefX"});
        rows.push_back({set, "STUFF('abcdef',-1E20,1,'X')", "C:Xbcdef"});
        rows.push_back({set, "STUFF('abcdef',2,1E20,'X')", "C:aX"});
        rows.push_back({set, "STUFF('abcdef',2,-1E20,'X')", "C:aXbcdef"});
        rows.push_back({set, "STUFF('abcdef',2147483648,1,'X')", "C:abcdefX"});
        rows.push_back({set, "STUFF('abcdef',4294967296,1,'X')", "C:abcdefX"});
        rows.push_back({set, "STUFF('abcdef',2,4294967296,'X')", "C:aX"});
        rows.push_back({set, "SUBSTR('abcdef',2,1E20)", "C:bcdef"});
        rows.push_back({set, "SUBSTR('abcdef',2,1E300)", "C:bcdef"});
        rows.push_back({set, "SUBSTR('abcdef',2,4294967296)", "C:bcdef"});
        rows.push_back({set, "SUBSTR('abcdef',2,-1E20)", "C:"});
        rows.push_back({set, "SUBSTR('abcdef',2,2.9)", "C:bc"});
    }
    // In-range start boundaries (#3704): consistent in VFP9 (probe2.txt, stuff-boundaries-6145c, substr-boundary-*,
    // substr-start-fractions-*), so they are the default and do not depend on the switch.
    for (const char *mode : {"COPPERFIN", "VFP9"}) {
        const std::string set = std::string("SET NUMERICBEHAVIOR TO ") + mode;
        for (const Row &row : std::vector<Row>{
                 {"", "STUFF('abcdef',0,1,'X')", "C:Xbcdef"},
                 {"", "STUFF('abcdef',-5,2,'X')", "C:Xcdef"},
                 {"", "STUFF('abcdef',1,-1,'X')", "C:Xabcdef"},
                 {"", "STUFF('abcdef',1,0,'X')", "C:Xabcdef"},
                 {"", "STUFF('abcdef',0,0,'X')", "C:Xabcdef"},
                 {"", "STUFF('abcdef',-1,100,'X')", "C:X"},
                 {"", "STUFF('abcdef',6,2,'X')", "C:abcdeX"},
                 {"", "STUFF('abcdef',7,2,'X')", "C:abcdefX"},
                 {"", "STUFF('abcdef',8,2,'X')", "C:abcdefX"},
                 {"", "STUFF('abcdef',99,0,'X')", "C:abcdefX"},
                 {"", "STUFF('abcdef',1.9,2.9,'X')", "C:Xcdef"},
                 {"", "STUFFC('abcdef',0,2,'X')", "C:Xcdef"},
                 {"", "STUFFC('abcdef',-1,2,'X')", "C:Xcdef"},
                 {"", "STUFFC('abcdef',1,-1,'X')", "C:Xabcdef"},
                 {"", "STUFFC('abcdef',6,2,'X')", "C:abcdeX"},
                 {"", "STUFFC('abcdef',7,2,'X')", "C:abcdefX"},
                 {"", "STUFFC('abcdef',8,2,'X')", "C:abcdefX"},
                 {"", "STUFFC('abcdef',99,0,'X')", "C:abcdefX"},
                 {"", "STUFFC('abcdef',1.9,2.9,'X')", "C:Xcdef"},
                 {"", "STUFFC('abcdef',1E20,1,'X')", "C:abcdefX"},
                 {"", "STUFFC('abcdef',-1E20,1,'X')", "C:Xbcdef"},
                 {"", "STUFFC('abcdef',2,1E20,'X')", "C:aX"},
                 {"", "STUFFC('abcdef',2,-1E20,'X')", "C:aXbcdef"},
                 {"", "GETWORDNUM('a b c',0.5)", "C:"},
                 {"", "GETWORDNUM('a b c',1.9)", "C:a"},
                 {"", "GETWORDNUM('a b c',1E20)", "C:"},
                 {"", "GETWORDNUM('a b c',1E300)", "C:"},
                 {"", "GETWORDNUM('a b c',0)", "C:"},
                 {"", "GETWORDNUM('a b c',-1)", "C:"},
                 {"", "GETWORDNUM('a b c',-1E20)", "C:"},
                 {"", "GETWORDNUM('a b c',EXP(1000))", "C:"},
                 {"", "MLINE('a'+CHR(13)+'b',0.5)", "C:"},
                 {"", "MLINE('a'+CHR(13)+'b',1.9)", "C:a"},
                 {"", "MLINE('a'+CHR(13)+'b',2)", "C:b"},
                 {"", "MLINE('a'+CHR(13)+'b',1E20)", "C:"},
                 {"", "MLINE('a'+CHR(13)+'b',EXP(1000))", "C:"},
                 {"", "MLINE('a'+CHR(13)+'b',0)", "C:"},
                 {"", "SUBSTR('abcdef',0)", "C:"},
                 {"", "SUBSTR('abcdef',-1)", "C:"},
                 {"", "SUBSTR('abcdef',0.5)", "C:"},
                 {"", "SUBSTR('abcdef',0.9)", "C:"},
                 {"", "SUBSTR('abcdef',1.5)", "C:abcdef"},
                 {"", "SUBSTR('abcdef',1.9)", "C:abcdef"},
                 {"", "SUBSTR('abcdef',2.9)", "C:bcdef"},
                 {"", "SUBSTR('abcdef',0,100)", "C:"},
                 {"", "SUBSTR('abcdef',1,0)", "C:"},
                 {"", "SUBSTR('abcdef',1,-1)", "C:"},
                 {"", "SUBSTR('abcdef',1,2)", "C:ab"},
                 {"", "SUBSTR('abcdef',6,100)", "C:f"},
                 {"", "SUBSTR('abcdef',7,100)", "C:"},
                 {"", "SUBSTRC('abcdef',0)", "C:"},
                 {"", "SUBSTRC('abcdef',-1)", "C:"},
                 {"", "SUBSTRC('abcdef',0.5)", "C:"},
                 {"", "SUBSTRC('abcdef',1.9)", "C:abcdef"},
                 {"", "SUBSTRC('abcdef',2.9)", "C:bcdef"},
             }) {
            rows.push_back({set, row.expression, row.expected});
        }
    }
    rows.push_back({"SET NUMERICBEHAVIOR TO COPPERFIN", "SUBSTR('abcdef',1E20)", "C:"});
    rows.push_back({"SET NUMERICBEHAVIOR TO COPPERFIN", "SUBSTR('abcdef',2147483648)", "C:"});
    rows.push_back({"SET NUMERICBEHAVIOR TO COPPERFIN", "SUBSTR('abcdef',2147483647)", "C:"});
    // STR decimals (review of slice 2): VFP9 accepts 0 to 18 and raises error 1908 above that (probe4.txt); a huge value
    // must be rejected before setprecision(), not saturated to INT_MAX and formatted.
    for (const char *mode : {"COPPERFIN", "VFP9"}) {
        const std::string set = std::string("SET NUMERICBEHAVIOR TO ") + mode;
        rows.push_back({set, "STR(1.5,25,2)", "C:                     1.50"});
        rows.push_back({set, "STR(1.5,25,18)", "C:     1.500000000000000000"});
        rows.push_back({set, "STR(1.5,25,19)", "ERR1908"});
        rows.push_back({set, "STR(1.5,10,100)", "ERR1908"});
        rows.push_back({set, "STR(1.5,10,2147483647)", "ERR1908"});
        rows.push_back({set, "STR(1.5,10,2147483648)", "ERR1908"});
        rows.push_back({set, "STR(1.5,10,1E20)", "ERR1908"});
        rows.push_back({set, "STR(1.5,10,1E300)", "ERR1908"});
        rows.push_back({set, "STR(1.5,10,EXP(1000))", "ERR1908"});
        rows.push_back({set, "STR(1.5,10,2.9)", "C:      1.50"});
    }
    // In-range counts are identical in both modes.
    for (const char *mode : {"COPPERFIN", "VFP9"}) {
        const std::string set = std::string("SET NUMERICBEHAVIOR TO ") + mode;
        rows.push_back({set, "LEFT('abcdef',3)", "C:abc"});
        rows.push_back({set, "RIGHT('abcdef',3)", "C:def"});
        rows.push_back({set, "LEFT('abcdef',0)", "C:"});
        rows.push_back({set, "RIGHT('abcdef',100)", "C:abcdef"});
    }

    // Array conversions (#6030 under #5611/#6776). The primary position and dimension errors are stable VFP9
    // contracts, so they apply in both modes. Only the observed 32-bit conversion quirks are mode-dependent.
    const std::string array_setup =
        "DIMENSION a[3]\n"
        "a[1] = 'a'\n"
        "a[2] = 'b'\n"
        "a[3] = 'c'\n";
    const std::string copy_setup =
        array_setup +
        "DIMENSION b[3]\n"
        "b[1] = 'x'\n"
        "b[2] = 'y'\n"
        "b[3] = 'z'\n";
    for (const char *mode : {"COPPERFIN", "VFP9"}) {
        const std::string set = std::string("SET NUMERICBEHAVIOR TO ") + mode + "\n";
        rows.push_back({set + array_setup, "AELEMENT(a,1E20)", "ERR1234"});
        rows.push_back({set + array_setup, "AELEMENT(a,1,1E20)", "ERR230"});
        rows.push_back({set + array_setup, "ASUBSCRIPT(a,1E20,1)", "ERR1234"});
        rows.push_back({set + array_setup, "ASUBSCRIPT(a,1,1E20)", "ERR1234"});
        rows.push_back({set + array_setup, "ASCAN(a,'b',1,1E20)", "ERR1234"});
        rows.push_back({set + array_setup, "ASCAN(a,'b',1,-1,-1,1E20)", "ERR11"});
        rows.push_back({set + copy_setup, "ACOPY(a,b,1E20)", "ERR1234"});
        rows.push_back({set + copy_setup, "ACOPY(a,b,1,-1,1E20)", "ERR1234"});
        rows.push_back({set + array_setup, "ADEL(a,1E20)", "ERR1234"});
        rows.push_back({set + array_setup, "AINS(a,1E20)", "ERR1234"});
        rows.push_back({set + array_setup, "ASORT(a,4294967296)", "ERR1234"});
    }
    rows.push_back({"SET NUMERICBEHAVIOR TO COPPERFIN\n" + array_setup,
                    "ASCAN(a,'b',2147483648)", "N:0"});
    rows.push_back({"SET NUMERICBEHAVIOR TO VFP9\n" + array_setup,
                    "ASCAN(a,'b',2147483648)", "N:2"});
    rows.push_back({"SET NUMERICBEHAVIOR TO COPPERFIN\n" + array_setup,
                    "ASORT(a,2147483648)", "ERR1234"});
    rows.push_back({"SET NUMERICBEHAVIOR TO VFP9\n" + array_setup,
                    "ASORT(a,2147483648)", "N:1"});
    for (const char *value : {"1E20", "-1E20", "1E300", "4294967296", "9007199254740992", "0.5"}) {
        rows.push_back({"SET NUMERICBEHAVIOR TO VFP9\n" + array_setup,
                        std::string("ASCAN(a,'b',") + value + ")", "ERR1234"});
    }
    for (const char *value : {"2147483647", "-2147483649", "1E10"}) {
        rows.push_back({"SET NUMERICBEHAVIOR TO VFP9\n" + array_setup,
                        std::string("ASCAN(a,'b',") + value + ")", "N:0"});
    }
    for (const char *value : {"2147483648", "-2147483648", "4294967295", "2.9", "-2.9"}) {
        rows.push_back({"SET NUMERICBEHAVIOR TO VFP9\n" + array_setup,
                        std::string("ASCAN(a,'b',") + value + ")", "N:2"});
    }
    for (const char *value : {"1E20", "-1E20", "1E300", "2147483647", "-2147483649",
                              "4294967296", "9007199254740992", "1E10", "0.5"}) {
        rows.push_back({"SET NUMERICBEHAVIOR TO VFP9\n" + array_setup,
                        std::string("ASORT(a,") + value + ")", "ERR1234"});
    }
    for (const char *value : {"2147483648", "-2147483648", "4294967295", "2.9", "-2.9"}) {
        rows.push_back({"SET NUMERICBEHAVIOR TO VFP9\n" + array_setup,
                        std::string("ASORT(a,") + value + ")", "N:1"});
    }
    rows.push_back({"SET NUMERICBEHAVIOR TO COPPERFIN\n" + array_setup,
                    "ASORT(a,1,-1,1E20)", "ERR11"});
    rows.push_back({"SET NUMERICBEHAVIOR TO VFP9\n" + array_setup,
                    "ASORT(a,1,-1,1E20)", "N:1"});
    rows.push_back({"SET NUMERICBEHAVIOR TO COPPERFIN\n" + copy_setup,
                    "ACOPY(a,b,1,1E20)", "N:3"});
    rows.push_back({"SET NUMERICBEHAVIOR TO VFP9\n" + copy_setup,
                    "ACOPY(a,b,1,1E20)", "N:0"});
    rows.push_back({"SET NUMERICBEHAVIOR TO COPPERFIN\n" + array_setup,
                    "ACOPY(a,bCopperfinNew,1,2,2)+ALEN(bCopperfinNew)+"
                    "IIF(bCopperfinNew[2]=='a',1,0)+IIF(bCopperfinNew[3]=='b',1,0)", "N:7"});
    rows.push_back({"SET NUMERICBEHAVIOR TO VFP9\n" + array_setup,
                    "ACOPY(a,bVfp9New,1,2,2)+ALEN(bVfp9New)+"
                    "IIF(bVfp9New[2]=='a',1,0)+IIF(bVfp9New[3]=='b',1,0)", "N:7"});
    const std::string matrix_setup =
        "DIMENSION a[3,2]\n"
        "a[1,1] = 'a'\n"
        "a[1,2] = 'b'\n"
        "a[2,1] = 'c'\n"
        "a[2,2] = 'd'\n"
        "a[3,1] = 'e'\n"
        "a[3,2] = 'f'\n";
    for (const char *mode : {"COPPERFIN", "VFP9"}) {
        const std::string set = std::string("SET NUMERICBEHAVIOR TO ") + mode + "\n";
        rows.push_back({set + matrix_setup, "ASCAN(a,'d',2,2,2)", "N:4"});
        rows.push_back({set + matrix_setup, "ASCAN(a,'d',2,3,2)", "ERR1234"});
        rows.push_back({set + matrix_setup, "ASCAN(a,'d',1,-1,3)", "ERR1234"});
    }
    rows.push_back({"SET NUMERICBEHAVIOR TO VFP9\n" + matrix_setup,
                    "ASCAN(a,'d',1,-1,4294967301)", "ERR1234"});
    rows.push_back({"SET NUMERICBEHAVIOR TO VFP9\n" + matrix_setup,
                    "ASCAN(a,'d',1,-1,4294967297)", "N:0"});
    for (const char *mode : {"COPPERFIN", "VFP9"}) {
        const std::string set = std::string("SET NUMERICBEHAVIOR TO ") + mode + "\n";
        rows.push_back({set + "DIMENSION oneColumn[3,1]\n", "ASUBSCRIPT(oneColumn,2,2)", "N:1"});
    }
    for (const char *count : {"EXP(1000)", "-EXP(1000)"}) {
        rows.push_back({"SET NUMERICBEHAVIOR TO COPPERFIN\n" + copy_setup,
                        std::string("ACOPY(a,b,1,") + count + ")", "ERR1234"});
        rows.push_back({"", "b[1]+b[2]+b[3]", "C:xyz"});
        rows.push_back({"SET NUMERICBEHAVIOR TO COPPERFIN\n" + array_setup,
                        std::string("ASORT(a,1,") + count + ")", "ERR1234"});
        rows.push_back({"", "a[1]+a[2]+a[3]", "C:abc"});
        rows.push_back({"SET NUMERICBEHAVIOR TO VFP9\n" + copy_setup,
                        std::string("ACOPY(a,b,1,") + count + ")", "N:0"});
        rows.push_back({"", "b[1]+b[2]+b[3]", "C:xyz"});
        rows.push_back({"SET NUMERICBEHAVIOR TO VFP9\nDIMENSION a[3]\na[1]='c'\na[2]='a'\na[3]='b'\n",
                        std::string("ASORT(a,1,") + count + ")", "N:1"});
        rows.push_back({"", "a[1]+a[2]+a[3]", "C:abc"});
    }
    for (const char *mode : {"COPPERFIN", "VFP9"}) {
        const std::string set = std::string("SET NUMERICBEHAVIOR TO ") + mode + "\n";
        rows.push_back({set + array_setup, "ADEL(a,1E20)", "ERR1234"});
        rows.push_back({"", "a[1]+a[2]+a[3]", "C:abc"});
        rows.push_back({set + array_setup, "AINS(a,1E20)", "ERR1234"});
        rows.push_back({"", "a[1]+a[2]+a[3]", "C:abc"});
        rows.push_back({set + array_setup, "ASORT(a,1E20)", "ERR1234"});
        rows.push_back({"", "a[1]+a[2]+a[3]", "C:abc"});
        rows.push_back({set + copy_setup, "ACOPY(a,b,1E20)", "ERR1234"});
        rows.push_back({"", "b[1]+b[2]+b[3]", "C:xyz"});
    }

    // BITLSHIFT/BITRSHIFT (#5765 under #5611/#6776). Both functions operate on 32-bit bit patterns and truncate
    // ordinary fractional arguments. VFP9's out-of-range double conversion is selectable; COPPERFIN rejects it.
    for (const char *mode : {"COPPERFIN", "VFP9"}) {
        const std::string set = std::string("SET NUMERICBEHAVIOR TO ") + mode;
        rows.push_back({set, "BITLSHIFT(1,-1)", "ERR11"});
        rows.push_back({set, "BITRSHIFT(256,-1)", "ERR11"});
        rows.push_back({set, "BITLSHIFT(1,-0.5)", "I:1"});
        rows.push_back({set, "BITRSHIFT(256,0.5)", "I:256"});
        rows.push_back({set, "BITLSHIFT(1,2.9)", "I:4"});
        rows.push_back({set, "BITRSHIFT(256,2.9)", "I:64"});
        rows.push_back({set, "BITLSHIFT(1,31)", "I:-2147483648"});
        rows.push_back({set, "BITRSHIFT(256,31)", "I:0"});
        rows.push_back({set, "BITLSHIFT(1,32)", "ERR11"});
        rows.push_back({set, "BITRSHIFT(256,64)", "ERR11"});
        rows.push_back({set, "BITLSHIFT(2147483647,1)", "I:-2"});
        rows.push_back({set, "BITLSHIFT(-1,1)", "I:-2"});
        rows.push_back({set, "BITLSHIFT(-0.5,1)", "I:0"});
        rows.push_back({set, "BITLSHIFT(1.9,1)", "I:2"});
        rows.push_back({set, "BITRSHIFT(-1,1)", "I:2147483647"});
        rows.push_back({set, "BITRSHIFT(-2147483648,31)", "I:1"});
        rows.push_back({set, "BITLSHIFT(4294967295,0)", "I:-1"});
        rows.push_back({set, "BITRSHIFT(10000000000,1)", "I:705032704"});
    }
    for (const char *count : {"EXP(1000)", "-EXP(1000)", "1E20", "-1E20", "1E300",
                              "4294967296", "9007199254740992"}) {
        rows.push_back({"SET NUMERICBEHAVIOR TO COPPERFIN", std::string("BITLSHIFT(1,") + count + ")", "ERR11"});
        rows.push_back({"SET NUMERICBEHAVIOR TO COPPERFIN", std::string("BITRSHIFT(256,") + count + ")", "ERR11"});
        rows.push_back({"SET NUMERICBEHAVIOR TO VFP9", std::string("BITLSHIFT(1,") + count + ")", "I:1"});
        rows.push_back({"SET NUMERICBEHAVIOR TO VFP9", std::string("BITRSHIFT(256,") + count + ")", "I:256"});
    }
    for (const char *count : {"2147483647", "2147483648", "-2147483648", "-2147483649",
                              "4294967295", "10000000000"}) {
        rows.push_back({"SET NUMERICBEHAVIOR TO VFP9", std::string("BITLSHIFT(1,") + count + ")", "ERR11"});
        rows.push_back({"SET NUMERICBEHAVIOR TO VFP9", std::string("BITRSHIFT(256,") + count + ")", "ERR11"});
    }
    for (const char *operand : {"EXP(1000)", "-EXP(1000)", "1E20", "-1E20", "1E300"}) {
        rows.push_back({"SET NUMERICBEHAVIOR TO COPPERFIN", std::string("BITLSHIFT(") + operand + ",1)", "ERR11"});
        rows.push_back({"SET NUMERICBEHAVIOR TO COPPERFIN", std::string("BITRSHIFT(") + operand + ",1)", "ERR11"});
        rows.push_back({"SET NUMERICBEHAVIOR TO VFP9", std::string("BITLSHIFT(") + operand + ",1)", "I:0"});
        rows.push_back({"SET NUMERICBEHAVIOR TO VFP9", std::string("BITRSHIFT(") + operand + ",1)", "I:0"});
    }

    // Governing requirement: RQ-CF-PRG-ROUND-BOUNDARIES-001.
    // ROUND truncates ordinary fractional decimal-place arguments. COPPERFIN saturates huge values consistently;
    // VFP9 reproduces its recovered 32-bit conversion, including normalizing INT32_MIN to zero decimal places.
    for (const char *mode : {"COPPERFIN", "VFP9"}) {
        const std::string set = std::string("SET NUMERICBEHAVIOR TO ") + mode;
        rows.push_back({set, "ROUND(1.5,2.9)", "N:1.5"});
        rows.push_back({set, "ROUND(1.5,-1.9)", "N:0"});
        rows.push_back({set, "ROUND($1.5,2.9)", "Y:$1.50"});
        rows.push_back({set, "ROUND($1.5,-1.9)", "Y:$0.00"});
    }
    for (const char *places : {"EXP(1000)", "1E20", "1E300", "2147483648", "4294967296",
                               "9007199254740992"}) {
        rows.push_back({"SET NUMERICBEHAVIOR TO COPPERFIN",
                        std::string("ROUND(1.5,") + places + ")", "N:1.5"});
        rows.push_back({"SET NUMERICBEHAVIOR TO COPPERFIN",
                        std::string("ROUND($1.5,") + places + ")", "Y:$1.50"});
    }
    for (const char *places : {"-EXP(1000)", "-1E20", "-9223372036854775808"}) {
        rows.push_back({"SET NUMERICBEHAVIOR TO COPPERFIN",
                        std::string("ROUND(1.5,") + places + ")", "N:0"});
        rows.push_back({"SET NUMERICBEHAVIOR TO COPPERFIN",
                        std::string("ROUND($1.5,") + places + ")", "Y:$0.00"});
    }
    for (const char *places : {"EXP(1000)", "-EXP(1000)", "1E20", "-1E20", "1E300",
                               "-9223372036854775808", "-4294967296", "-2147483648", "2147483648",
                               "4294967296", "9007199254740992"}) {
        rows.push_back({"SET NUMERICBEHAVIOR TO VFP9",
                        std::string("ROUND(1.5,") + places + ")", "N:2"});
        rows.push_back({"SET NUMERICBEHAVIOR TO VFP9",
                        std::string("ROUND($1.5,") + places + ")", "Y:$2.00"});
    }
    for (const char *places : {"-4294967297", "4294967295"}) {
        rows.push_back({"SET NUMERICBEHAVIOR TO VFP9",
                        std::string("ROUND(1.5,") + places + ")", "N:0"});
        rows.push_back({"SET NUMERICBEHAVIOR TO VFP9",
                        std::string("ROUND($1.5,") + places + ")", "Y:$0.00"});
    }
    for (const char *places : {"-4294967295", "-2147483649", "2147483647", "1E10"}) {
        rows.push_back({"SET NUMERICBEHAVIOR TO VFP9",
                        std::string("ROUND(1.5,") + places + ")", "N:1.5"});
        rows.push_back({"SET NUMERICBEHAVIOR TO VFP9",
                        std::string("ROUND($1.5,") + places + ")", "Y:$1.50"});
    }

    // Governing requirement: RQ-CF-PRG-GOMONTH-BOUNDARIES-001.
    // GOMONTH/EOMONTH (#5608 under #5611/#6776). Both modes truncate ordinary fractional offsets and enforce
    // VFP9's 1753..9999 result-year range. VFP9 mode additionally preserves its 32-bit conversion quirks.
    for (const char *mode : {"COPPERFIN", "VFP9"}) {
        const std::string set = std::string("SET NUMERICBEHAVIOR TO ") + mode;
        rows.push_back({set, "DTOS(GOMONTH(DATE(2026,1,31),1.9))", "C:20260228"});
        rows.push_back({set, "DTOS(GOMONTH(DATE(2026,1,31),-1.9))", "C:20251231"});
        rows.push_back({set, "DTOS(GOMONTH(DATE(1752,12,31),1))", "C:17530131"});
        rows.push_back({set, "DTOS(GOMONTH(DATE(1753,1,1),-1))", "C:"});
        rows.push_back({set, "DTOS(GOMONTH(DATE(1753,1,1),98963))", "C:99991201"});
        rows.push_back({set, "DTOS(GOMONTH(DATE(1753,1,1),98964))", "C:"});
        rows.push_back({set, "DTOS(GOMONTH(DATE(9999,12,31),1))", "C:"});
        rows.push_back({set, "DTOS(EOMONTH(DATE(2026,1,15),1.9))", "C:20260228"});
        rows.push_back({set, "DTOS(EOMONTH(DATE(2026,1,15),-1.9))", "C:20251231"});
        rows.push_back({set, "DTOS(EOMONTH(DATE(1752,12,31)))", "C:"});
        rows.push_back({set, "DTOS(EOMONTH(DATE(1753,1,1),-1))", "C:"});
        rows.push_back({set, "DTOS(EOMONTH(DATE(9999,12,1),0))", "C:99991231"});
        rows.push_back({set, "DTOS(EOMONTH(DATE(9999,12,1),1))", "C:"});
        rows.push_back({set, "DTOS(GOMONTH(DATE(2026,1,31),10000000000))", "C:"});
        rows.push_back({set, "DTOS(EOMONTH(DATE(2026,1,15),10000000000))", "C:"});
    }
    for (const char *offset : {"EXP(1000)", "-EXP(1000)", "1E20", "-1E20", "1E300"}) {
        rows.push_back({"SET NUMERICBEHAVIOR TO COPPERFIN",
                        std::string("DTOS(GOMONTH(DATE(2026,1,31),") + offset + "))", "ERR11"});
        rows.push_back({"SET NUMERICBEHAVIOR TO COPPERFIN",
                        std::string("DTOS(EOMONTH(DATE(2026,1,15),") + offset + "))", "ERR11"});
        rows.push_back({"SET NUMERICBEHAVIOR TO VFP9",
                        std::string("DTOS(GOMONTH(DATE(2026,1,31),") + offset + "))", "C:20260131"});
        rows.push_back({"SET NUMERICBEHAVIOR TO VFP9",
                        std::string("DTOS(EOMONTH(DATE(2026,1,15),") + offset + "))", "C:20260131"});
    }
    for (const char *offset : {"-4294967297", "4294967295"}) {
        rows.push_back({"SET NUMERICBEHAVIOR TO COPPERFIN",
                        std::string("DTOS(GOMONTH(DATE(2026,1,31),") + offset + "))", "C:"});
        rows.push_back({"SET NUMERICBEHAVIOR TO VFP9",
                        std::string("DTOS(GOMONTH(DATE(2026,1,31),") + offset + "))", "C:20251231"});
    }
    rows.push_back({"SET NUMERICBEHAVIOR TO COPPERFIN",
                    "DTOS(GOMONTH(DATE(2026,1,31),4294967296))", "C:"});
    rows.push_back({"SET NUMERICBEHAVIOR TO VFP9",
                    "DTOS(GOMONTH(DATE(2026,1,31),4294967296))", "C:20260131"});
    return rows;
}

std::string run_rows(const fs::path &dir, const std::vector<Row> &rows) {
    std::string body = "LOCAL cOut, oEx, x\ncOut = ''\n";
    for (const Row &row : rows) {
        body += "TRY\n";
        if (!row.setup.empty()) {
            body += row.setup + "\n";
        }
        body += "x = " + row.expression + "\n";
        body += "cOut = cOut + VARTYPE(x) + ':' + TRANSFORM(x) + CHR(10)\n";
        body += "CATCH TO oEx\n";
        body += "cOut = cOut + 'ERR' + ALLTRIM(STR(oEx.ErrorNo)) + CHR(10)\n";
        body += "ENDTRY\n";
    }
    body += "STRTOFILE(cOut, 'results.txt')\nRETURN\n";
    fs::create_directories(dir / "script");
    write_text(dir / "script" / "rows.prg", body);
    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options((dir / "script" / "rows.prg").string(), dir.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    if (!state.completed) {
        return "<incomplete: " + state.message + ">";
    }
    return read_text(dir / "results.txt");
}

void test_numeric_behavior_script_rows() {
    const fs::path dir = fs::temp_directory_path() / "copperfin_numeric_behavior";
    std::error_code ignored;
    fs::remove_all(dir, ignored);
    const std::vector<Row> rows = build_rows();
    const std::string output = run_rows(dir, rows);
    expect(output.rfind("<incomplete", 0U) != 0U, "numeric behavior: the script should complete: " + output);
    std::vector<std::string> lines;
    for (std::size_t start = 0U; start < output.size();) {
        const std::size_t end = output.find('\n', start);
        if (end == std::string::npos) {
            break;
        }
        lines.push_back(output.substr(start, end - start));
        start = end + 1U;
    }
    expect(lines.size() == rows.size(),
           "numeric behavior: expected " + std::to_string(rows.size()) + " result lines, got " +
               std::to_string(lines.size()) + " from [" + output + "]");
    for (std::size_t index = 0U; index < rows.size() && index < lines.size(); ++index) {
        expect(lines[index] == rows[index].expected,
               "numeric behavior: [" + rows[index].setup + "] " + rows[index].expression + " expected [" +
                   rows[index].expected + "], got [" + lines[index] + "]");
    }
    fs::remove_all(dir, ignored);
}

// Governing requirement: RQ-CF-PRG-GOMONTH-BOUNDARIES-001.
// Empty out-of-calendar results must remain empty when persisted, including when a Date result is assigned to a
// DateTime field. This checks the physical DBF representation after the runtime closes the table.
void test_gomonth_out_of_range_dbf_round_trip() {
    const fs::path dir = fs::temp_directory_path() / "copperfin_gomonth_out_of_range_dbf";
    std::error_code ignored;
    fs::remove_all(dir, ignored);
    fs::create_directories(dir);
    const fs::path table_path = dir / "dates.dbf";
    const fs::path script_path = dir / "round_trip.prg";
    write_text(
        script_path,
        "CREATE TABLE '" + table_path.string() + "' (gd D, ed D, gt T, et T)\n"
        "APPEND BLANK\n"
        "REPLACE gd WITH GOMONTH(DATE(1753,1,1),-1), ;\n"
        "        ed WITH EOMONTH(DATE(9999,12,1),1), ;\n"
        "        gt WITH GOMONTH(DATE(1753,1,1),-1), ;\n"
        "        et WITH EOMONTH(DATE(9999,12,1),1)\n"
        "USE\n"
        "RETURN\n");

    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(script_path.string(), dir.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "GOMONTH DBF round trip should complete: " + state.message);

    const auto parsed = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 1U);
    expect(parsed.ok && parsed.table.records.size() == 1U,
           "GOMONTH DBF round trip should produce one readable record");
    if (parsed.ok && parsed.table.records.size() == 1U) {
        const auto& values = parsed.table.records.front().values;
        expect(values.size() == 4U, "GOMONTH DBF round trip should retain four fields");
        if (values.size() == 4U) {
            expect(uppercase_ascii(values[0].field_name) == "GD" && values[0].display_value.empty(),
                   "out-of-range GOMONTH must persist as an empty DBF Date");
            expect(uppercase_ascii(values[1].field_name) == "ED" && values[1].display_value.empty(),
                   "out-of-range EOMONTH must persist as an empty DBF Date");
            expect(uppercase_ascii(values[2].field_name) == "GT" &&
                       values[2].display_value == "julian:0 millis:0",
                   "out-of-range GOMONTH must persist as an empty DBF DateTime");
            expect(uppercase_ascii(values[3].field_name) == "ET" &&
                       values[3].display_value == "julian:0 millis:0",
                   "out-of-range EOMONTH must persist as an empty DBF DateTime");
        }
    }
    fs::remove_all(dir, ignored);
}

// The conversion helpers directly, including the values a script cannot easily produce (NaN, infinity).
void test_conversion_helpers() {
    using copperfin::runtime::NumericBehavior;
    using copperfin::runtime::numeric_count_argument;
    using copperfin::runtime::checked_truncated_numeric_to_int64;
    constexpr std::int64_t kMax = std::numeric_limits<std::int64_t>::max();
    constexpr std::int64_t kMin = std::numeric_limits<std::int64_t>::min();
    const double inf = std::numeric_limits<double>::infinity();
    const double nan = std::numeric_limits<double>::quiet_NaN();

    expect(numeric_count_argument(1E20, NumericBehavior::copperfin) == kMax, "COPPERFIN: 1E20 saturates high");
    expect(numeric_count_argument(-1E20, NumericBehavior::copperfin) == kMin, "COPPERFIN: -1E20 saturates low");
    expect(numeric_count_argument(inf, NumericBehavior::copperfin) == kMax, "COPPERFIN: +infinity saturates high");
    expect(numeric_count_argument(-inf, NumericBehavior::copperfin) == kMin, "COPPERFIN: -infinity saturates low");
    expect(numeric_count_argument(nan, NumericBehavior::copperfin) == 0, "COPPERFIN: NaN is 0");
    expect(numeric_count_argument(9223372036854775808.0, NumericBehavior::copperfin) == kMax, "COPPERFIN: 2^63 saturates");
    expect(numeric_count_argument(-9223372036854775808.0, NumericBehavior::copperfin) == kMin, "COPPERFIN: -2^63 is INT64_MIN");
    expect(numeric_count_argument(2.9, NumericBehavior::copperfin) == 2, "COPPERFIN: truncates toward zero");
    expect(numeric_count_argument(-2.9, NumericBehavior::copperfin) == -2, "COPPERFIN: truncates toward zero (negative)");

    expect(numeric_count_argument(2147483647.0, NumericBehavior::vfp9) == 2147483647, "VFP9: INT32_MAX is kept");
    expect(numeric_count_argument(2147483648.0, NumericBehavior::vfp9) == -2147483648LL, "VFP9: 2^31 wraps to INT32_MIN");
    expect(numeric_count_argument(4294967295.0, NumericBehavior::vfp9) == -1, "VFP9: 2^32-1 is -1");
    expect(numeric_count_argument(4294967296.0, NumericBehavior::vfp9) == 0, "VFP9: 2^32 is 0");
    expect(numeric_count_argument(-2147483649.0, NumericBehavior::vfp9) == 2147483647, "VFP9: -2^31-1 wraps to INT32_MAX");
    expect(numeric_count_argument(1E10, NumericBehavior::vfp9) == 1410065408, "VFP9: 1E10 keeps its low 32 bits");
    expect(numeric_count_argument(1E20, NumericBehavior::vfp9) == 0, "VFP9: outside int64 is the integer indefinite (0)");
    expect(numeric_count_argument(-1E20, NumericBehavior::vfp9) == 0, "VFP9: -1E20 is the integer indefinite (0)");
    expect(numeric_count_argument(inf, NumericBehavior::vfp9) == 0, "VFP9: infinity is the integer indefinite (0)");
    expect(numeric_count_argument(nan, NumericBehavior::vfp9) == 0, "VFP9: NaN is the integer indefinite (0)");
    expect(numeric_count_argument(9007199254740992.0, NumericBehavior::vfp9) == 0, "VFP9: 2^53 has low 32 bits 0");
    expect(numeric_count_argument(-2.9, NumericBehavior::vfp9) == -2, "VFP9: truncates toward zero");

    expect(checked_truncated_numeric_to_int64(2.9) == 2, "checked integer conversion truncates toward zero");
    expect(checked_truncated_numeric_to_int64(-2.9) == -2, "checked negative conversion truncates toward zero");
    expect(checked_truncated_numeric_to_int64(-9223372036854775808.0) == kMin,
           "checked conversion accepts INT64_MIN");
    expect(!checked_truncated_numeric_to_int64(9223372036854775808.0).has_value(),
           "checked conversion rejects 2^63");
    expect(!checked_truncated_numeric_to_int64(inf).has_value(), "checked conversion rejects infinity");
    expect(!checked_truncated_numeric_to_int64(-inf).has_value(), "checked conversion rejects negative infinity");
    expect(!checked_truncated_numeric_to_int64(nan).has_value(), "checked conversion rejects NaN");
}

}  // namespace

int main() {
    test_conversion_helpers();
    test_numeric_behavior_script_rows();
    test_gomonth_out_of_range_dbf_round_trip();
    if (const int failures = test_failures(); failures != 0) {
        std::cerr << failures << " test(s) failed\n";
        return 1;
    }
    return 0;
}
