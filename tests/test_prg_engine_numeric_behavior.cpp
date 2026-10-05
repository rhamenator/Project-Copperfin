// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/runtime/prg_engine.h"
#include "copperfin/vfp/dbf_table.h"
#include "../src/runtime/prg_engine_helpers.h"
#include "../src/runtime/prg_engine_date_time_functions.h"
#include "../src/runtime/prg_engine_numeric_functions.h"
#include "../src/runtime/prg_engine_runtime_surface_functions.h"
#include "../src/runtime/prg_compatibility_error.h"
#include "prg_engine_test_support.h"

#include <cmath>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <limits>
#include <string>
#include <system_error>
#include <utility>
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
    rows.push_back({"SET NUMERICBEHAVIOR TO COPPERFIN", "SUBSTR('abcdef',4294967296)", "C:"});
    rows.push_back({"SET NUMERICBEHAVIOR TO COPPERFIN", "SUBSTR('abcdef',EXP(1000))", "C:"});
    rows.push_back({"SET NUMERICBEHAVIOR TO COPPERFIN", "SUBSTRC('abcdef',1E20)", "C:"});
    rows.push_back({"SET NUMERICBEHAVIOR TO COPPERFIN", "SUBSTRC('abcdef',EXP(1000))", "C:"});
    // VFP9's start conversion is function-specific: a positive value above INT_MAX is reduced to a signed 32-bit
    // value. When that result is at or before the end, SUBSTR/SUBSTRC clamp to the final character; a wrapped value
    // beyond the end remains empty. This is a legacy quirk, so it is available only in VFP9 mode.
    for (const char *value : {"1E20", "1E300", "2147483648", "4294967295", "4294967296",
                              "4294967297", "4294967302", "9007199254740992", "EXP(1000)"}) {
        rows.push_back({"SET NUMERICBEHAVIOR TO VFP9", std::string("SUBSTR('abcdef',") + value + ")", "C:f"});
        rows.push_back({"SET NUMERICBEHAVIOR TO VFP9", std::string("SUBSTRC('abcdef',") + value + ")", "C:f"});
    }
    rows.push_back({"SET NUMERICBEHAVIOR TO VFP9", "SUBSTR('abcdef',4294967296,1)", "C:f"});
    rows.push_back({"SET NUMERICBEHAVIOR TO VFP9", "SUBSTRC('abcdef',4294967296,1)", "C:f"});
    rows.push_back({"SET NUMERICBEHAVIOR TO VFP9", "SUBSTRC('café猫',4294967296)", "C:猫"});
    rows.push_back({"SET NUMERICBEHAVIOR TO VFP9", "SUBSTR('abcdef',2147483647)", "C:"});
    rows.push_back({"SET NUMERICBEHAVIOR TO VFP9", "SUBSTRC('abcdef',2147483647)", "C:"});
    rows.push_back({"SET NUMERICBEHAVIOR TO VFP9", "SUBSTR('abcdef',1E10)", "C:"});
    rows.push_back({"SET NUMERICBEHAVIOR TO VFP9", "SUBSTRC('abcdef',1E10)", "C:"});
    rows.push_back({"SET NUMERICBEHAVIOR TO VFP9", "SUBSTR('abcdef',4294967303)", "C:"});
    rows.push_back({"SET NUMERICBEHAVIOR TO VFP9", "SUBSTRC('abcdef',4294967303)", "C:"});
    // Occurrence controls are validated before conversion (#5611/#6776). Installed VFP9 raises error 11 for
    // sub-unit, nonpositive, non-finite, and oversized AT/STRTRAN occurrence arguments. Copperfin keeps that
    // stable fail-closed contract in both modes. The one observed VFP9 conversion quirk is STRTRAN's 2^32-1 ->
    // -1 sentinel; it is available only in VFP9 mode, while COPPERFIN rejects the out-of-range value.
    for (const char *mode : {"COPPERFIN", "VFP9"}) {
        const std::string set = std::string("SET NUMERICBEHAVIOR TO ") + mode;
        for (const char *value : {"0.5", "0", "-1.5", "-1.9", "-2.9", "16777185", "2147483647",
                                  "2147483648", "1E10", "1E20", "1E300", "EXP(1000)"}) {
            rows.push_back({set, std::string("AT('b','abcabc',") + value + ")", "ERR11"});
            rows.push_back({set, std::string("STRTRAN('abcabc','b','x',") + value + ")", "ERR11"});
            rows.push_back({set, std::string("STRTRAN('abcabc','b','x',1,") + value + ")", "ERR11"});
        }
        for (const char *function : {"ATC", "ATCC", "RAT", "RATC"}) {
            rows.push_back({set, std::string(function) + "('b','abcabc',0.5)", "ERR11"});
            rows.push_back({set, std::string(function) + "('b','abcabc',16777185)", "ERR11"});
            rows.push_back({set, std::string(function) + "('b','abcabc',2147483647)", "ERR11"});
        }
        rows.push_back({set, "AT('b','abcabc',2.9)", "N:5"});
        rows.push_back({set, "AT('b','abcabc',16777184)", "N:0"});
        rows.push_back({set, "STRTRAN('abcabc','b','x',2.9)", "C:abcaxc"});
        rows.push_back({set, "STRTRAN('abcabc','b','x',1,2.9)", "C:axcaxc"});
        rows.push_back({set, "STRTRAN('abcabc','b','x',16777184)", "C:abcabc"});
        rows.push_back({set, "STRTRAN('abcabc','b','x',1,16777184)", "C:axcaxc"});
        rows.push_back({set, "STRTRAN('abcabc','b','x',-1)", "C:axcaxc"});
        rows.push_back({set, "STRTRAN('abcabc','b','x',1,-1)", "C:axcaxc"});
        for (const char *value : {"0.5", "-2.9", "2147483647", "2147483648", "4294967295", "4294967296",
                                  "1E10", "1E20", "1E300", "EXP(1000)"}) {
            rows.push_back({set, std::string("GETWORDNUM('a b c',") + value + ")", "C:"});
        }
    }
    rows.push_back({"SET NUMERICBEHAVIOR TO COPPERFIN", "STRTRAN('abcabc','b','x',4294967295)", "ERR11"});
    rows.push_back({"SET NUMERICBEHAVIOR TO COPPERFIN", "STRTRAN('abcabc','b','x',1,4294967295)", "ERR11"});
    rows.push_back({"SET NUMERICBEHAVIOR TO VFP9", "STRTRAN('abcabc','b','x',4294967295)", "C:axcaxc"});
    rows.push_back({"SET NUMERICBEHAVIOR TO VFP9", "STRTRAN('abcabc','b','x',1,4294967295)", "C:axcaxc"});
    // RQ-CF-PRG-STR-ARGUMENT-BOUNDS-001 (#5611/#6776): installed VFP9 accepts widths 0..237 and
    // decimals 0..18 after truncation. Reject invalid values before formatting or padding so no argument can become an
    // unbounded precision or allocation. Explicit VFP9 mode retains the recovered negative low-32-bit quirk.
    for (const char *mode : {"COPPERFIN", "VFP9"}) {
        const std::string set = std::string("SET NUMERICBEHAVIOR TO ") + mode;
        rows.push_back({set, "STR(1.5,25,2)", "C:                     1.50"});
        rows.push_back({set, "LEN(STR(1.5,237,2))", "N:237"});
        rows.push_back({set, "STR(1.5,238,2)", "ERR1908"});
        rows.push_back({set, "STR(1.5,-1,2)", "ERR1908"});
        rows.push_back({set, "STR(1.5,0,2)", "C:"});
        rows.push_back({set, "STR('not-a-number',0,2)", "ERR1"});
        rows.push_back({set, "STR(1.5,25,18)", "C:     1.500000000000000000"});
        rows.push_back({set, "STR(1.5,25,19)", "ERR1908"});
        rows.push_back({set, "STR(1.5,10,-1)", "ERR1908"});
        rows.push_back({set, "STR(1.5,10,-0.9)", "C:         2"});
        rows.push_back({set, "STR(1.5,10,100)", "ERR1908"});
        rows.push_back({set, "STR(1.5,10,2147483647)", "ERR1908"});
        rows.push_back({set, "STR(1.5,10,2147483648)", "ERR1908"});
        rows.push_back({set, "STR(1.5,10,1E20)", "ERR1908"});
        rows.push_back({set, "STR(1.5,10,1E300)", "ERR1908"});
        rows.push_back({set, "STR(1.5,10,EXP(1000))", "ERR1908"});
        rows.push_back({set, "STR(1.5,EXP(1000),2)", "ERR1908"});
        rows.push_back({set, "STR(1.5,10,2.9)", "C:      1.50"});
    }
    rows.push_back({"SET NUMERICBEHAVIOR TO COPPERFIN", "STR(1.5,10,-4294967295)", "ERR1908"});
    rows.push_back({"SET NUMERICBEHAVIOR TO COPPERFIN", "STR(1,-4294967295)", "ERR1908"});
    rows.push_back({"SET NUMERICBEHAVIOR TO COPPERFIN", "STR(1.5,10,-1E20)", "ERR1908"});
    rows.push_back({"SET NUMERICBEHAVIOR TO COPPERFIN", "STR(1,-1E20)", "ERR1908"});
    rows.push_back({"SET NUMERICBEHAVIOR TO VFP9", "STR(1.5,10,-4294967295)", "C:       1.5"});
    rows.push_back({"SET NUMERICBEHAVIOR TO VFP9", "STR(1,-4294967295)", "C:1"});
    rows.push_back({"SET NUMERICBEHAVIOR TO VFP9", "STR(1.5,10,-4294967296)", "C:         2"});
    rows.push_back({"SET NUMERICBEHAVIOR TO VFP9", "STR(1,-4294967296)", "C:"});
    rows.push_back({"SET NUMERICBEHAVIOR TO VFP9", "STR(1.5,10,-1E20)", "C:         2"});
    rows.push_back({"SET NUMERICBEHAVIOR TO VFP9", "STR(1,-1E20)", "C:"});
    rows.push_back({"SET NUMERICBEHAVIOR TO COPPERFIN", "STR(1.5,10,-EXP(1000))", "ERR1908"});
    rows.push_back({"SET NUMERICBEHAVIOR TO COPPERFIN", "STR(1,-EXP(1000))", "ERR1908"});
    rows.push_back({"SET NUMERICBEHAVIOR TO VFP9", "STR(1.5,10,-EXP(1000))", "C:         2"});
    rows.push_back({"SET NUMERICBEHAVIOR TO VFP9", "STR(1,-EXP(1000))", "C:"});
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

    // Governing requirement: RQ-CF-PRG-CHR-BOUNDARIES-001.
    // CHR truncates ordinary fractions and accepts only a resulting byte. COPPERFIN rejects every raw value outside
    // that range. VFP9 preserves its asymmetric 32-bit conversion quirk for negative values: wrapped byte values and
    // the integer-indefinite zero are accepted, while positive values above 255 are rejected before conversion.
    for (const char *mode : {"COPPERFIN", "VFP9"}) {
        const std::string set = std::string("SET NUMERICBEHAVIOR TO ") + mode;
        rows.push_back({set, "ASC(CHR(-0.9))", "N:0"});
        rows.push_back({set, "ASC(CHR(0.9))", "N:0"});
        rows.push_back({set, "ASC(CHR(65.9))", "N:65"});
        rows.push_back({set, "ASC(CHR(255.9))", "N:255"});
        rows.push_back({set, "CHR(-1)", "ERR11"});
        rows.push_back({set, "CHR(256)", "ERR11"});
        rows.push_back({set, "CHR(2147483648)", "ERR11"});
        rows.push_back({set, "CHR(4294967296)", "ERR11"});
        rows.push_back({set, "CHR(EXP(1000))", "ERR11"});
    }
    for (const char *code : {"-EXP(1000)", "-1E300", "-1E20", "-9223372036854775808",
                             "-8589934592", "-4294967296"}) {
        rows.push_back({"SET NUMERICBEHAVIOR TO COPPERFIN", std::string("CHR(") + code + ")", "ERR11"});
        rows.push_back({"SET NUMERICBEHAVIOR TO VFP9", std::string("ASC(CHR(") + code + "))", "N:0"});
    }
    rows.push_back({"SET NUMERICBEHAVIOR TO COPPERFIN", "CHR(-4294967295)", "ERR11"});
    rows.push_back({"SET NUMERICBEHAVIOR TO VFP9", "ASC(CHR(-4294967295))", "N:1"});
    rows.push_back({"SET NUMERICBEHAVIOR TO COPPERFIN", "CHR(-4294967294)", "ERR11"});
    rows.push_back({"SET NUMERICBEHAVIOR TO VFP9", "ASC(CHR(-4294967294))", "N:2"});
    rows.push_back({"SET NUMERICBEHAVIOR TO COPPERFIN", "CHR(-4294967041)", "ERR11"});
    rows.push_back({"SET NUMERICBEHAVIOR TO VFP9", "ASC(CHR(-4294967041))", "N:255"});
    rows.push_back({"SET NUMERICBEHAVIOR TO VFP9", "CHR(-4294967040)", "ERR11"});
    rows.push_back({"SET NUMERICBEHAVIOR TO COPPERFIN", "CHR(-17179869184)", "ERR11"});
    rows.push_back({"SET NUMERICBEHAVIOR TO VFP9", "ASC(CHR(-17179869184))", "N:0"});
    for (const char *code : {"-4294967297", "-2147483649", "-2147483648", "-2147483647", "-256"}) {
        rows.push_back({"SET NUMERICBEHAVIOR TO VFP9", std::string("CHR(") + code + ")", "ERR11"});
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

    // Governing requirement: RQ-CF-PRG-DATE-CONSTRUCTOR-BOUNDS-001.
    // Installed VFP9 truncates DATE()/DATETIME() components, accepts years 100..9999, and raises error 11 for
    // invalid calendar/time components. Positive out-of-range values are rejected before conversion. Its negative
    // signed-32-bit conversion quirk remains available only in VFP9 mode, so a wrapped year or time component may
    // become valid while COPPERFIN rejects the same raw value.
    for (const char *mode : {"COPPERFIN", "VFP9"}) {
        const std::string set = std::string("SET NUMERICBEHAVIOR TO ") + mode;
        rows.push_back({set, "DTOS(DATE(2026.9,1.9,1.9))", "C:20260101"});
        rows.push_back({set, "TTOC(DATETIME(2026,1,1,0,0,1.9),1)", "C:20260101000001"});
        rows.push_back({set, "DTOS(DATE(100,1,1))", "C:01000101"});
        rows.push_back({set, "DTOS(DATE(9999,12,31))", "C:99991231"});
        rows.push_back({set, "DATE(99,1,1)", "ERR11"});
        rows.push_back({set, "DATE(10000,1,1)", "ERR11"});
        rows.push_back({set, "DATE(2026,2,29)", "ERR11"});
        rows.push_back({set, "DATE(2026,0,1)", "ERR11"});
        rows.push_back({set, "DATE(2026,1,0)", "ERR11"});
        rows.push_back({set, "DATETIME(2026,1,1,24,0,0)", "ERR11"});
        rows.push_back({set, "DATETIME(2026,1,1,0,60,0)", "ERR11"});
        rows.push_back({set, "DATETIME(2026,1,1,0,0,60)", "ERR11"});
        rows.push_back({set, "DATE(1E20,1,1)", "ERR11"});
        rows.push_back({set, "DATE(EXP(1000),1,1)", "ERR11"});
        rows.push_back({set, "DATETIME(2026,1,1,1E20,0,0)", "ERR11"});
        rows.push_back({set, "DATETIME(2026,1,1,EXP(1000),0,0)", "ERR11"});
    }
    for (const char *expression : {
             "DATE(-4294967196,1,1)",
             "DATETIME(2026,1,1,-4294967295,0,0)",
             "DATETIME(2026,1,1,-EXP(1000),0,0)"}) {
        rows.push_back({"SET NUMERICBEHAVIOR TO COPPERFIN", expression, "ERR11"});
    }
    rows.push_back({"SET NUMERICBEHAVIOR TO VFP9", "DTOS(DATE(-4294967196,1,1))", "C:01000101"});
    rows.push_back({"SET NUMERICBEHAVIOR TO VFP9",
                    "TTOC(DATETIME(2026,1,1,-4294967295,0,0),1)", "C:20260101010000"});
    rows.push_back({"SET NUMERICBEHAVIOR TO VFP9",
                    "TTOC(DATETIME(2026,1,1,-EXP(1000),0,0),1)", "C:20260101000000"});

    // Governing requirement: RQ-CF-PRG-DOW-FIRST-DAY-BOUNDS-001.
    for (const char *mode : {"COPPERFIN", "VFP9"}) {
        const std::string set = std::string("SET FDOW TO 4\nSET NUMERICBEHAVIOR TO ") + mode;
        rows.push_back({set, "DOW(DATE(2026,1,7),0)", "N:1"});
        rows.push_back({set, "DOW(DATE(2026,1,7),1.9)", "N:4"});
        rows.push_back({set, "DOW(DATE(2026,1,7),-0.9)", "N:1"});
        rows.push_back({set, "DOW(DATE(2026,1,7),8)", "ERR11"});
        rows.push_back({set, "DOW(DATE(2026,1,7),-1)", "ERR11"});
    }
    for (const char *value : {"1E20", "-1E20", "4294967296", "-4294967295", "EXP(1000)", "-EXP(1000)"}) {
        rows.push_back({"SET FDOW TO 4\nSET NUMERICBEHAVIOR TO COPPERFIN",
                        std::string("DOW(DATE(2026,1,7),") + value + ")", "ERR11"});
    }
    for (const char *value : {"1E20", "-1E20", "4294967296", "EXP(1000)", "-EXP(1000)"}) {
        rows.push_back({"SET FDOW TO 4\nSET NUMERICBEHAVIOR TO VFP9",
                        std::string("DOW(DATE(2026,1,7),") + value + ")", "N:1"});
    }
    rows.push_back({"SET FDOW TO 4\nSET NUMERICBEHAVIOR TO VFP9",
                    "DOW(DATE(2026,1,7),-4294967295)", "N:4"});
    rows.push_back({"SET FDOW TO 4\nSET NUMERICBEHAVIOR TO VFP9",
                    "DOW(DATE(2026,1,7),4294967295)", "ERR11"});

    // Governing requirement: RQ-CF-PRG-WEEK-OPTIONS-001.
    for (const char *mode : {"COPPERFIN", "VFP9"}) {
        const std::string set = std::string("SET FWEEK TO 3\nSET FDOW TO 4\nSET NUMERICBEHAVIOR TO ") + mode;
        rows.push_back({set, "WEEK(DATE(2021,1,1),0)", "N:52"});
        rows.push_back({set, "WEEK(DATE(2021,1,1),1,0)", "N:1"});
        rows.push_back({set, "WEEK(DATE(2021,1,1),1,7)", "N:1"});
        rows.push_back({set, "WEEK(DATE(2021,1,1),1,7.9)", "N:1"});
        rows.push_back({set, "WEEK(DATE(2021,1,1),2.9)", "N:53"});
        rows.push_back({set, "WEEK(DATE(2021,1,1),4)", "ERR11"});
        rows.push_back({set, "WEEK(DATE(2021,1,1),-1)", "ERR11"});
        rows.push_back({set, "WEEK(DATE(2021,1,1),1,8)", "ERR11"});
        rows.push_back({set, "WEEK(DATE(2021,1,1),1,-1)", "ERR11"});
    }
    for (const char *value : {"1E20", "-1E20", "4294967296", "EXP(1000)", "-EXP(1000)"}) {
        rows.push_back({"SET FWEEK TO 3\nSET NUMERICBEHAVIOR TO COPPERFIN",
                        std::string("WEEK(DATE(2021,1,1),") + value + ")", "ERR11"});
        rows.push_back({"SET FWEEK TO 3\nSET NUMERICBEHAVIOR TO VFP9",
                        std::string("WEEK(DATE(2021,1,1),") + value + ")", "N:52"});
    }
    rows.push_back({"SET NUMERICBEHAVIOR TO VFP9",
                    "WEEK(DATE(2021,1,1),-4294967295)", "N:1"});
    rows.push_back({"SET NUMERICBEHAVIOR TO VFP9",
                    "WEEK(DATE(2021,1,1),4294967295)", "ERR11"});

    // Governing requirement: RQ-CF-PRG-RGB-COMPONENT-BOUNDS-001.
    for (const char *mode : {"COPPERFIN", "VFP9"}) {
        const std::string set = std::string("SET NUMERICBEHAVIOR TO ") + mode;
        rows.push_back({set, "RGB(0,0,0)", "N:0"});
        rows.push_back({set, "RGB(1.9,2.9,3.9)", "N:197,121"});
        rows.push_back({set, "RGB(-0.9,0,0)", "N:0"});
        rows.push_back({set, "RGB(255,255,255)", "N:16,777,215"});
        rows.push_back({set, "RGB(-1,0,0)", "ERR11"});
        rows.push_back({set, "RGB(256,0,0)", "ERR11"});
        rows.push_back({set, "RGB(0,256,0)", "ERR11"});
        rows.push_back({set, "RGB(0,0,256)", "ERR11"});
        rows.push_back({set, "RGB(EXP(1000),0,0)", "ERR11"});
    }
    rows.push_back({"SET NUMERICBEHAVIOR TO COPPERFIN", "RGB(255.9,0,0)", "N:255"});
    rows.push_back({"SET NUMERICBEHAVIOR TO VFP9", "RGB(255.9,0,0)", "ERR11"});
    for (const char *component : {"-EXP(1000)", "-1E20", "-4294967296"}) {
        rows.push_back({"SET NUMERICBEHAVIOR TO COPPERFIN",
                        std::string("RGB(") + component + ",0,0)", "ERR11"});
        rows.push_back({"SET NUMERICBEHAVIOR TO VFP9",
                        std::string("RGB(") + component + ",0,0)", "N:0"});
    }
    rows.push_back({"SET NUMERICBEHAVIOR TO COPPERFIN", "RGB(-4294967295,0,0)", "ERR11"});
    rows.push_back({"SET NUMERICBEHAVIOR TO VFP9", "RGB(-4294967295,0,0)", "N:1"});
    rows.push_back({"SET NUMERICBEHAVIOR TO VFP9", "RGB(-4294967041,0,0)", "N:255"});
    rows.push_back({"SET NUMERICBEHAVIOR TO VFP9", "RGB(-4294967040,0,0)", "ERR11"});

    // Governing requirement: RQ-CF-PRG-RAND-SEED-BOUNDS-001.
    // Installed VFP9 truncates positive fractions, treats every non-positive
    // seed as the same reset request, and takes the low 32 bits of large
    // positive seeds. COPPERFIN keeps the first two documented behaviors but
    // rejects positive values outside uint32 and non-finite values.
    for (const char *mode : {"COPPERFIN", "VFP9"}) {
        const std::string set = std::string("SET NUMERICBEHAVIOR TO ") + mode + "\n";
        rows.push_back({set +
                            "a=RAND(123)\nb=RAND()\nc=RAND(123.9)\nd=RAND()",
                        "a==c AND b==d", "L:true"});
        rows.push_back({set +
                            "a=RAND(4294967295)\nb=RAND()\nc=RAND(4294967295.9)\nd=RAND()",
                        "a==c AND b==d", "L:true"});
        rows.push_back({set +
                            "a=RAND($4294967295.0000)\nb=RAND()\nc=RAND($4294967295.0001)\nd=RAND()",
                        "a==c AND b==d", "L:true"});
        rows.push_back({set +
                            "a=RAND(0)\nb=RAND()\nc=RAND(-1E20)\nd=RAND()",
                        "a==c AND b==d", "L:true"});
        rows.push_back({std::string("SET NUMERICBEHAVIOR TO ") + mode,
                        "RAND(4294967295)>=0 AND RAND()<1", "L:true"});
    }
    rows.push_back({"SET NUMERICBEHAVIOR TO COPPERFIN", "RAND(4294967296)", "ERR11"});
    rows.push_back({"SET NUMERICBEHAVIOR TO COPPERFIN", "RAND(EXP(1000))", "ERR11"});
    rows.push_back({"SET NUMERICBEHAVIOR TO VFP9\n"
                    "a=RAND(1)\nb=RAND()\nc=RAND(4294967297)\nd=RAND()",
                    "a==c AND b==d", "L:true"});
    rows.push_back({"SET NUMERICBEHAVIOR TO VFP9\n"
                    "a=RAND(4294967296)\nb=RAND()\nc=RAND(EXP(1000))\nd=RAND()",
                    "a==c AND b==d", "L:true"});
    // RQ-CF-PRG-HEX-NUMERIC-BOUNDS-001: Copperfin extension, not a VFP9 builtin.
    for (const char* mode : {"COPPERFIN", "VFP9"}) {
        const std::string set = std::string("SET NUMERICBEHAVIOR TO ") + mode;
        rows.push_back({set, "HEX(255.9)", "C:FF"});
        rows.push_back({set, "HEX(-1E300)", "C:0"});
        rows.push_back({set, "HEX(9223372036854775808.0)", "ERR11"});
        rows.push_back({set, "HEX(1E300)", "ERR11"});
        rows.push_back({set, "HEX(EXP(1000))", "ERR11"});
        rows.push_back({set, "HEX(-EXP(1000))", "ERR11"});
        rows.push_back({set, "HEX($562949953421311.9999)", "C:1FFFFFFFFFFFF"});
    }
    // RQ-CF-PRG-SYS-SELECTOR-NUMERIC-001: selector admission only; SYS return gaps are separate.
    for (const char* mode : {"COPPERFIN", "VFP9"}) {
        const std::string set = std::string("SET NUMERICBEHAVIOR TO ") + mode;
        rows.push_back({set, "SYS(5.9)==SYS(5)", "L:true"});
        rows.push_back({set, "SYS(9223372036854775808.0)",
                        std::string(mode) == "COPPERFIN" ? "ERR11" : "C:0"});
        rows.push_back({set, "SYS(1E300)", std::string(mode) == "COPPERFIN" ? "ERR11" : "C:0"});
        rows.push_back({set, "SYS(EXP(1000))", std::string(mode) == "COPPERFIN" ? "ERR11" : "C:0"});
        rows.push_back({set, "SYS(-EXP(1000))", std::string(mode) == "COPPERFIN" ? "ERR11" : "C:0"});
    }
    rows.push_back({"SET NUMERICBEHAVIOR TO VFP9", "SYS(-4294967291)==SYS(5)", "L:true"});
    // RQ-CF-PRG-CPCURRENT-NUMERIC-001: query-domain/type parity is a separate gap.
    for (const char* mode : {"COPPERFIN", "VFP9"}) {
        const std::string set = std::string("SET NUMERICBEHAVIOR TO ") + mode;
        for (const char* argument : {"1.9", "2.5", "2147483648", "4294967298", "1E300", "-1E300",
                                     "EXP(1000)", "-EXP(1000)"}) {
            rows.push_back({set, std::string("CPCURRENT(") + argument + ")", "ERR11"});
        }
    }
    // RQ-CF-PRG-RELATION-INDEX-NUMERIC-001: index conversion, not relation order.
    for (const char* mode : {"COPPERFIN", "VFP9"}) {
        const std::string set = std::string("SET NUMERICBEHAVIOR TO ") + mode;
        for (const char* function : {"RELATION", "TARGET"}) {
            for (const char* argument : {"0", "0.9", "9999.49", "10000", "2147483647",
                                         "4294967297", "1E300", "-1E300", "EXP(1000)", "-EXP(1000)"}) {
                rows.push_back({set, std::string(function) + "(" + argument + ")", "ERR11"});
            }
        }
    }
    // RQ-CF-PRG-SET-TEXTMERGE-NUMERIC-001: admission, not query-result parity.
    for (const char* mode : {"COPPERFIN", "VFP9"}) {
        const std::string set = std::string("SET NUMERICBEHAVIOR TO ") + mode;
        for (const char* argument : {"0", "-1", "0.9", "5", "2147483647", "1E300", "-1E300",
                                     "EXP(1000)", "-EXP(1000)"}) {
            rows.push_back({set, std::string("SET('TEXTMERGE', ") + argument + ")", "ERR11"});
        }
        rows.push_back({set, "SET('TEXTMERGE', 3.9)", "C:SHOW"});
    }
    // RQ-CF-PRG-ALINES-FLAGS-NUMERIC-001: retained native conversion observations;
    // first element distinguishes trimming, not full flag/splitting semantics.
    struct AlinesCase { const char* argument; const char* copperfin; const char* vfp9; };
    const std::vector<AlinesCase> alines_cases{
        {"0", "C: a ", "C: a "},
        {"1", "C:a", "C:a"},
        {"2", "C: a ", "C: a "},
        {"3", "C:a", "C:a"},
        {"4", "C: a ", "C: a "},
        {"31", "C:a", "C:a"},
        {"32", "ERR11", "ERR11"},
        {"-1", "ERR11", "ERR11"},
        {"0.49", "C: a ", "C: a "},
        {"0.5", "C: a ", "C: a "},
        {"0.9", "C: a ", "C: a "},
        {"1.49", "C:a", "C:a"},
        {"1.5", "C:a", "C:a"},
        {"1.9", "C:a", "C:a"},
        {"2.5", "C: a ", "C: a "},
        {"31.49", "C:a", "C:a"},
        {"31.9", "C:a", "C:a"},
        {"32.5", "ERR11", "ERR11"},
        {"2147483647", "ERR11", "ERR11"},
        {"2147483648", "ERR11", "ERR11"},
        {"-2147483648", "ERR11", "ERR11"},
        {"-2147483649", "ERR11", "ERR11"},
        {"4294967297", "ERR11", "C:a"},
        {"-4294967295", "ERR11", "C:a"},
        {"4294967327", "ERR11", "C:a"},
        {"-4294967265", "ERR11", "C:a"},
        {"1E300", "ERR11", "C: a "},
        {"-1E300", "ERR11", "C: a "},
        {"EXP(1000)", "ERR11", "C: a "},
        {"-EXP(1000)", "ERR11", "C: a "},
        {"-0.49", "C: a ", "C: a "},
        {"-0.5", "C: a ", "C: a "},
        {"-0.9", "C: a ", "C: a "},
        {"-1.1", "ERR11", "ERR11"},
        {"4294967328", "ERR11", "ERR11"},
        {"-4294967264", "ERR11", "ERR11"}};
    for (const char* mode : {"COPPERFIN", "VFP9"}) {
        const std::string set = std::string("SET NUMERICBEHAVIOR TO ") + mode;
        const std::string array = set + "\nLOCAL ARRAY aFlag[1]\naFlag[1] = 'sentinel'";
        for (const auto& row : alines_cases) {
            rows.push_back({array + "\nx = ALINES(aFlag, ' a ' + CHR(13) + ' b ', " + row.argument + ")",
                            "aFlag[1]", std::string(mode) == "COPPERFIN" ? row.copperfin : row.vfp9});
        }
        // Rejection is catchable and leaves shape/content unchanged in both modes.
        for (const char* argument : {"32", "-1", "4294967328", "-4294967264"}) {
            rows.push_back({array + "\nLOCAL nError\nnError = 0\nTRY\n"
                            "x = ALINES(aFlag, ' a ' + CHR(13) + ' b ', " + argument + ")\n"
                            "CATCH TO oEx\nnError = oEx.ErrorNo\nENDTRY",
                            "ALLTRIM(STR(nError)) + ':' + ALLTRIM(STR(ALEN(aFlag))) + ':' + aFlag[1]",
                            "C:11:1:sentinel"});
        }
        // Preserve omitted flags, custom delimiters and existing non-Numeric coercions.
        rows.push_back({array + "\nx = ALINES(aFlag, ' a ' + CHR(13) + ' b ')", "aFlag[1]", "C: a "});
        rows.push_back({array + "\nx = ALINES(aFlag, ' a | b ', 1, '|')", "aFlag[1]", "C:a"});
        for (const char* argument : {".T.", "'1.5'", "$1.5000"}) {
            rows.push_back({array + "\nx = ALINES(aFlag, ' a ' + CHR(13) + ' b ', " + argument + ")",
                            "aFlag[1]", std::string(argument) == ".T." ? "C:a" : "C: a "});
        }
        rows.push_back({array + "\nx = ALINES(aFlag, ' a ' + CHR(13) + ' b ', .NULL.)",
                        "aFlag[1]", "C: a "});
    }
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
    using copperfin::runtime::checked_declared_int32_argument;
    using copperfin::runtime::checked_declared_int64_argument;
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

    // Governing requirement: RQ-CF-PRG-DECLARE-INT32-001 (#6050/#6776).
    expect(checked_declared_int32_argument(
               copperfin::runtime::make_number_value(2147483647.0), NumericBehavior::copperfin) == 2147483647,
           "DECLARE INTEGER keeps INT32_MAX");
    expect(checked_declared_int32_argument(
               copperfin::runtime::make_number_value(2147483648.0), NumericBehavior::copperfin) ==
               std::numeric_limits<std::int32_t>::min(),
           "DECLARE INTEGER interprets 2^31 through its low 32 bits");
    expect(checked_declared_int32_argument(
               copperfin::runtime::make_number_value(4294967295.0), NumericBehavior::copperfin) == -1,
           "DECLARE INTEGER preserves the UINT32_MAX bit pattern");
    expect(checked_declared_int32_argument(
               copperfin::runtime::make_number_value(4294967296.0), NumericBehavior::copperfin) == 0,
           "DECLARE INTEGER discards bits above the low 32");
    expect(checked_declared_int32_argument(
               copperfin::runtime::make_number_value(-2147483649.0), NumericBehavior::copperfin) == 2147483647,
           "DECLARE INTEGER wraps negative values through the low 32 bits");
    expect(checked_declared_int32_argument(
               copperfin::runtime::make_number_value(-2.9), NumericBehavior::copperfin) == -2,
           "DECLARE INTEGER truncates fractions toward zero before taking low bits");
    expect(checked_declared_int32_argument(
               copperfin::runtime::make_int64_value(kMin), NumericBehavior::copperfin) == 0,
           "DECLARE INTEGER accepts exact INT64_MIN without a floating round trip");
    expect(checked_declared_int32_argument(copperfin::runtime::make_uint64_value(
               std::numeric_limits<std::uint64_t>::max()), NumericBehavior::copperfin) == -1,
           "DECLARE INTEGER accepts exact UINT64_MAX low bits");
    expect(!checked_declared_int32_argument(
                copperfin::runtime::make_number_value(1E300), NumericBehavior::copperfin).has_value(),
           "COPPERFIN DECLARE INTEGER rejects huge finite values outside the supported conversion range");
    expect(checked_declared_int32_argument(
               copperfin::runtime::make_number_value(1E300), NumericBehavior::vfp9) == 0,
           "VFP9 DECLARE INTEGER maps a huge finite value to the integer-indefinite low bits");
    expect(checked_declared_int32_argument(
               copperfin::runtime::make_number_value(-1E300), NumericBehavior::vfp9) == 0,
           "VFP9 DECLARE INTEGER maps a huge negative finite value to the integer-indefinite low bits");
    for (const auto behavior : {NumericBehavior::copperfin, NumericBehavior::vfp9}) {
        expect(!checked_declared_int32_argument(
                    copperfin::runtime::make_number_value(inf), behavior).has_value(),
               "DECLARE INTEGER rejects infinity in both numeric modes");
        expect(!checked_declared_int32_argument(
                    copperfin::runtime::make_number_value(nan), behavior).has_value(),
               "DECLARE INTEGER rejects NaN in both numeric modes");
    }

    expect(checked_declared_int64_argument(copperfin::runtime::make_int64_value(kMin)) == kMin,
           "DECLARE INTEGER64 accepts exact INT64_MIN");
    expect(checked_declared_int64_argument(copperfin::runtime::make_int64_value(kMax)) == kMax,
           "DECLARE INTEGER64 accepts exact INT64_MAX");
    expect(checked_declared_int64_argument(copperfin::runtime::make_uint64_value(
               static_cast<std::uint64_t>(kMax))) == kMax,
           "DECLARE INTEGER64 accepts an exact unsigned value through INT64_MAX");
    expect(!checked_declared_int64_argument(copperfin::runtime::make_uint64_value(
                static_cast<std::uint64_t>(kMax) + 1U)).has_value(),
           "DECLARE INTEGER64 rejects an exact unsigned value above INT64_MAX");
    expect(!checked_declared_int64_argument(copperfin::runtime::make_uint64_value(
                std::numeric_limits<std::uint64_t>::max())).has_value(),
           "DECLARE INTEGER64 rejects exact UINT64_MAX");
    expect(checked_declared_int64_argument(copperfin::runtime::make_number_value(-9223372036854775808.0)) == kMin,
           "DECLARE INTEGER64 accepts numeric INT64_MIN");
    expect(!checked_declared_int64_argument(copperfin::runtime::make_number_value(9223372036854775808.0)).has_value(),
           "DECLARE INTEGER64 rejects numeric 2^63");
    expect(!checked_declared_int64_argument(copperfin::runtime::make_number_value(-inf)).has_value(),
           "DECLARE INTEGER64 rejects negative infinity");

    // Governing requirement: RQ-CF-PRG-ROUND-BOUNDARIES-001.
    // Extreme negative decimal places collapse finite magnitudes to signed zero, but must not hide a non-finite
    // first argument. Exercise NaN directly because no stable PRG expression produces it without raising first.
    for (const char* mode : {"COPPERFIN", "VFP9"}) {
        const auto set_callback = [mode](const std::string& setting) {
            return setting == "NUMERICBEHAVIOR" ? std::string(mode) : std::string{};
        };
        const auto positive_infinity = copperfin::runtime::evaluate_numeric_function(
            "round", {copperfin::runtime::make_number_value(inf), copperfin::runtime::make_number_value(-309.0)},
            set_callback);
        const auto negative_infinity = copperfin::runtime::evaluate_numeric_function(
            "round", {copperfin::runtime::make_number_value(-inf), copperfin::runtime::make_number_value(-309.0)},
            set_callback);
        const auto nan_result = copperfin::runtime::evaluate_numeric_function(
            "round", {copperfin::runtime::make_number_value(nan), copperfin::runtime::make_number_value(-309.0)},
            set_callback);
        expect(positive_infinity.has_value() && std::isinf(positive_infinity->number_value) &&
                   !std::signbit(positive_infinity->number_value),
               std::string(mode) + ": ROUND must preserve positive infinity at extreme negative places");
        expect(negative_infinity.has_value() && std::isinf(negative_infinity->number_value) &&
                   std::signbit(negative_infinity->number_value),
               std::string(mode) + ": ROUND must preserve negative infinity at extreme negative places");
        expect(nan_result.has_value() && std::isnan(nan_result->number_value),
               std::string(mode) + ": ROUND must preserve NaN at extreme negative places");
    }
}

void test_date_time_constructor_direct_numeric_boundaries() {
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const auto callback = [](const char* mode) {
        return [mode](const std::string& setting) {
            return setting == "NUMERICBEHAVIOR" ? std::string(mode) : std::string{};
        };
    };
    const auto args_with_hour = [](const copperfin::runtime::PrgValue& hour) {
        return std::vector<copperfin::runtime::PrgValue>{
            copperfin::runtime::make_number_value(2026.0),
            copperfin::runtime::make_number_value(1.0),
            copperfin::runtime::make_number_value(1.0),
            hour,
            copperfin::runtime::make_number_value(0.0),
            copperfin::runtime::make_number_value(0.0)};
    };

    const auto exact_low_bit_one = copperfin::runtime::make_int64_value(
        std::numeric_limits<std::int64_t>::min() + 1);
    const auto wrapped = copperfin::runtime::evaluate_date_time_function(
        "datetime", args_with_hour(exact_low_bit_one), callback("VFP9"));
    expect(wrapped.has_value() && wrapped->string_value.find("01:00:00") != std::string::npos,
           "VFP9 DATETIME should preserve an exact int64 component's low 32 bits");

    const auto nan_vfp9 = copperfin::runtime::evaluate_date_time_function(
        "datetime", args_with_hour(copperfin::runtime::make_number_value(nan)), callback("VFP9"));
    expect(nan_vfp9.has_value() && nan_vfp9->string_value.find("00:00:00") != std::string::npos,
           "VFP9 DATETIME should map an integer-indefinite time component to zero");

    for (const auto& value : {exact_low_bit_one, copperfin::runtime::make_number_value(nan)}) {
        bool rejected = false;
        try {
            (void)copperfin::runtime::evaluate_date_time_function(
                "datetime", args_with_hour(value), callback("COPPERFIN"));
        } catch (const copperfin::runtime::PrgCompatibilityError& error) {
            rejected = error.error_code() == 11;
        }
        expect(rejected, "COPPERFIN DATETIME should reject an out-of-range or non-finite component with error 11");
    }
}

void test_dow_direct_numeric_boundaries() {
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const auto callback = [](const char* mode) {
        return [mode](const std::string& setting) {
            if (setting == "NUMERICBEHAVIOR") {
                return std::string(mode);
            }
            return setting == "FDOW" ? std::string("4") : std::string{};
        };
    };
    const auto args_with_first_day = [](const copperfin::runtime::PrgValue& first_day) {
        return std::vector<copperfin::runtime::PrgValue>{
            copperfin::runtime::make_date_value("01/07/2026", 2026, 1, 7),
            first_day};
    };

    const auto exact_low_bit_one = copperfin::runtime::make_int64_value(
        std::numeric_limits<std::int64_t>::min() + 1);
    const auto exact_low_bit_zero = copperfin::runtime::make_uint64_value(UINT64_C(4294967296));
    for (const auto& expectation : std::vector<std::pair<copperfin::runtime::PrgValue, double>>{
             {exact_low_bit_one, 4.0},
             {exact_low_bit_zero, 1.0},
             {copperfin::runtime::make_number_value(nan), 1.0}}) {
        const auto result = copperfin::runtime::evaluate_date_time_function(
            "dow", args_with_first_day(expectation.first), callback("VFP9"));
        expect(result.has_value() && result->number_value == expectation.second,
               "VFP9 DOW should preserve exact low-32-bit conversion and integer-indefinite zero");

        bool rejected = false;
        try {
            (void)copperfin::runtime::evaluate_date_time_function(
                "dow", args_with_first_day(expectation.first), callback("COPPERFIN"));
        } catch (const copperfin::runtime::PrgCompatibilityError& error) {
            rejected = error.error_code() == 11;
        }
        expect(rejected, "COPPERFIN DOW should reject an out-of-range or non-finite first-day option with error 11");
    }
}

void test_week_direct_numeric_boundaries() {
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const auto callback = [](const char* mode) {
        return [mode](const std::string& setting) {
            if (setting == "NUMERICBEHAVIOR") {
                return std::string(mode);
            }
            if (setting == "FWEEK") {
                return std::string("3");
            }
            return setting == "FDOW" ? std::string("4") : std::string{};
        };
    };
    const auto args_with_first_week = [](const copperfin::runtime::PrgValue& first_week) {
        return std::vector<copperfin::runtime::PrgValue>{
            copperfin::runtime::make_date_value("01/01/2021", 2021, 1, 1),
            first_week};
    };

    const auto exact_low_bit_one = copperfin::runtime::make_int64_value(
        std::numeric_limits<std::int64_t>::min() + 1);
    const auto exact_low_bit_zero = copperfin::runtime::make_uint64_value(UINT64_C(4294967296));
    for (const auto& expectation : std::vector<std::pair<copperfin::runtime::PrgValue, double>>{
             {exact_low_bit_one, 1.0},
             {exact_low_bit_zero, 52.0},
             {copperfin::runtime::make_number_value(nan), 52.0}}) {
        const auto result = copperfin::runtime::evaluate_date_time_function(
            "week", args_with_first_week(expectation.first), callback("VFP9"));
        expect(result.has_value() && result->number_value == expectation.second,
               "VFP9 WEEK should preserve exact low-32-bit conversion and integer-indefinite zero");

        bool rejected = false;
        try {
            (void)copperfin::runtime::evaluate_date_time_function(
                "week", args_with_first_week(expectation.first), callback("COPPERFIN"));
        } catch (const copperfin::runtime::PrgCompatibilityError& error) {
            rejected = error.error_code() == 11;
        }
        expect(rejected, "COPPERFIN WEEK should reject an out-of-range or non-finite first-week option with error 11");
    }
}

void test_rgb_direct_numeric_boundaries() {
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const auto callback = [](const char* mode) {
        return [mode](const std::string& setting) {
            return setting == "NUMERICBEHAVIOR" ? std::string(mode) : std::string{};
        };
    };
    const auto args_with_red = [](const copperfin::runtime::PrgValue& red) {
        return std::vector<copperfin::runtime::PrgValue>{
            red,
            copperfin::runtime::make_number_value(0.0),
            copperfin::runtime::make_number_value(0.0)};
    };

    const auto exact_low_bit_one = copperfin::runtime::make_int64_value(
        std::numeric_limits<std::int64_t>::min() + 1);
    for (const auto& value : {exact_low_bit_one, copperfin::runtime::make_number_value(nan)}) {
        const auto result = copperfin::runtime::evaluate_numeric_function(
            "rgb", args_with_red(value), callback("VFP9"));
        const double expected = value.kind == copperfin::runtime::PrgValueKind::int64 ? 1.0 : 0.0;
        expect(result.has_value() && result->number_value == expected,
               "VFP9 RGB should preserve exact low-32-bit conversion and integer-indefinite zero");

        bool rejected = false;
        try {
            (void)copperfin::runtime::evaluate_numeric_function(
                "rgb", args_with_red(value), callback("COPPERFIN"));
        } catch (const copperfin::runtime::PrgCompatibilityError& error) {
            rejected = error.error_code() == 11;
        }
        expect(rejected, "COPPERFIN RGB should reject out-of-range or non-finite exact components");
    }

    bool rejected = false;
    try {
        (void)copperfin::runtime::evaluate_numeric_function(
            "rgb",
            args_with_red(copperfin::runtime::make_uint64_value(UINT64_C(4294967296))),
            callback("VFP9"));
    } catch (const copperfin::runtime::PrgCompatibilityError& error) {
        rejected = error.error_code() == 11;
    }
    expect(rejected, "VFP9 RGB should reject an exact positive component above 255 before wrapping");
}

void test_rand_direct_numeric_boundaries() {
    const double inf = std::numeric_limits<double>::infinity();
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const auto callback = [](const char* mode) {
        return [mode](const std::string& setting) {
            return setting == "NUMERICBEHAVIOR" ? std::string(mode) : std::string{};
        };
    };
    const auto evaluate = [&](const copperfin::runtime::PrgValue& seed, const char* mode) {
        return copperfin::runtime::evaluate_numeric_function("rand", {seed}, callback(mode));
    };
    const auto in_unit_range = [](const std::optional<copperfin::runtime::PrgValue>& value) {
        return value.has_value() && value->number_value >= 0.0 && value->number_value < 1.0;
    };

    const auto exact_currency_maximum = copperfin::runtime::make_currency_value(
        INT64_C(4294967295) * INT64_C(10000));
    for (const char* mode : {"COPPERFIN", "VFP9"}) {
        expect(in_unit_range(evaluate(exact_currency_maximum, mode)),
               std::string(mode) + " RAND should accept the exact uint32 Currency seed boundary");
        expect(in_unit_range(evaluate(
                   copperfin::runtime::make_int64_value(std::numeric_limits<std::int64_t>::min()), mode)),
               std::string(mode) + " RAND should reset safely for an exact INT64_MIN seed");
    }

    for (const auto& seed : {
             copperfin::runtime::make_uint64_value(std::numeric_limits<std::uint64_t>::max()),
             copperfin::runtime::make_currency_value(INT64_C(4294967296) * INT64_C(10000)),
             copperfin::runtime::make_number_value(inf),
             copperfin::runtime::make_number_value(-inf)}) {
        expect(in_unit_range(evaluate(seed, "VFP9")),
               "VFP9 RAND should define exact low-32-bit and infinite seed conversion");

        bool rejected = false;
        try {
            (void)evaluate(seed, "COPPERFIN");
        } catch (const copperfin::runtime::PrgCompatibilityError& error) {
            rejected = error.error_code() == 11;
        }
        expect(rejected, "COPPERFIN RAND should reject out-of-range or non-finite seeds with error 11");
    }

    for (const char* mode : {"COPPERFIN", "VFP9"}) {
        bool rejected = false;
        try {
            (void)evaluate(copperfin::runtime::make_number_value(nan), mode);
        } catch (const copperfin::runtime::PrgCompatibilityError& error) {
            rejected = error.error_code() == 11;
        }
        expect(rejected, std::string(mode) + " RAND should reject NaN with error 11");
    }
}

// RQ-CF-PRG-HEX-NUMERIC-BOUNDS-001, derived from #5611/#6776 safety policy.
// Both settings use the extension contract; #5880 retains name/precedence parity.
void test_hex_direct_numeric_boundaries() {
    using namespace copperfin::runtime;
    const double inf = std::numeric_limits<double>::infinity();
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double upper = 9223372036854775808.0;
    const auto evaluate = [](const PrgValue& value, const char* mode) {
        return evaluate_numeric_function("hex", {value}, [mode](const std::string& setting) {
            return setting == "NUMERICBEHAVIOR" ? std::string(mode) : std::string{};
        });
    };
    const std::vector<std::pair<PrgValue, std::string>> accepted{
        {make_number_value(0.0), "0"},
        {make_number_value(-0.0), "0"},
        {make_number_value(0.9), "0"},
        {make_number_value(-0.9), "0"},
        {make_number_value(255.9), "FF"},
        {make_number_value(65536.0), "10000"},
        {make_number_value(std::nextafter(upper, 0.0)), "7FFFFFFFFFFFFC00"},
        {make_number_value(-upper), "0"},
        {make_number_value(std::nextafter(-upper, -inf)), "0"},
        {make_number_value(-1E300), "0"},
        {make_int64_value(std::numeric_limits<std::int64_t>::min()), "0"},
        {make_int64_value(std::numeric_limits<std::int64_t>::max()), "7FFFFFFFFFFFFFFF"},
        {make_int64_value(INT64_C(9007199254740993)), "20000000000001"},
        {make_uint64_value(UINT64_C(9007199254740993)), "20000000000001"},
        {make_uint64_value(static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())),
         "7FFFFFFFFFFFFFFF"},
        {make_currency_value(INT64_C(5629499534213120000) - 1), "1FFFFFFFFFFFF"},
        {make_currency_value(std::numeric_limits<std::int64_t>::max()), "346DC5D638865"},
        {make_currency_value(std::numeric_limits<std::int64_t>::min()), "0"},
        {make_currency_value(9999), "0"},
        // Coercion remains outside this numeric-conversion slice.
        {make_string_value("255"), "FF"},
        {make_boolean_value(true), "1"},
        {make_null_value(), "0"},
        {make_empty_value(), "0"}};
    const std::vector<PrgValue> rejected{
        make_number_value(upper),
        make_number_value(std::nextafter(upper, inf)),
        make_number_value(1E300),
        make_number_value(inf),
        make_number_value(-inf),
        make_number_value(nan),
        make_uint64_value(UINT64_C(9223372036854775808)),
        make_uint64_value(std::numeric_limits<std::uint64_t>::max())};
    for (const char* mode : {"COPPERFIN", "VFP9"}) {
        for (std::size_t i = 0; i < accepted.size(); ++i) {
            const auto result = evaluate(accepted[i].first, mode);
            expect(result.has_value() && result->kind == PrgValueKind::string &&
                       result->string_value == accepted[i].second,
                   std::string(mode) + " HEX accepted boundary " + std::to_string(i));
        }
        for (std::size_t i = 0; i < rejected.size(); ++i) {
            bool caught = false;
            try {
                (void)evaluate(rejected[i], mode);
            } catch (const PrgCompatibilityError& error) {
                caught = error.error_code() == 11;
            }
            expect(caught, std::string(mode) + " HEX rejected boundary " + std::to_string(i));
        }
    }
}

// RQ-CF-PRG-SYS-SELECTOR-NUMERIC-001 (#5611/#6776). The retained installed probe
// determines Numeric conversion, not unimplemented SYS return semantics.
// Assert semantic results/errors/callbacks, not floating-point status flags:
// non-strict FP builds may speculate a guarded cast (notably optimized ARM64).
// ASan/UBSan/float-cast-overflow separately check source-level conversion safety.
void test_sys_selector_direct_numeric_boundaries() {
    using namespace copperfin::runtime;
    const double inf = std::numeric_limits<double>::infinity();
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double upper = 9223372036854775808.0;
    struct Case { PrgValue value; const char* copperfin; const char* vfp9; };
    const std::vector<Case> cases{
        {make_number_value(5), "/sys-sentinel/", "/sys-sentinel/"},
        {make_number_value(5.49), "/sys-sentinel/", "/sys-sentinel/"},
        {make_number_value(5.5), "/sys-sentinel/", "/sys-sentinel/"},
        {make_number_value(5.9), "/sys-sentinel/", "/sys-sentinel/"},
        {make_number_value(-0.9), "0", "0"},
        {make_number_value(-1), "0", "0"},
        {make_number_value(2147483647), "0", "0"},
        {make_number_value(2147483648), "0", "0"},
        {make_number_value(-2147483648.0), "0", "0"},
        {make_number_value(-2147483649.0), "0", "0"},
        {make_number_value(4294967301.0), "0", "0"},
        {make_number_value(4294969326.0), "0", "0"}, // low bits are state-changing SYS(2030)
        {make_number_value(-4294967291.0), "0", "/sys-sentinel/"},
        {make_number_value(std::nextafter(upper, 0.0)), "0", "0"},
        {make_number_value(upper), "ERR11", "0"},
        {make_number_value(std::nextafter(upper, inf)), "ERR11", "0"},
        {make_number_value(-upper), "0", "0"},
        {make_number_value(std::nextafter(-upper, -inf)), "ERR11", "0"},
        {make_number_value(1E300), "ERR11", "0"},
        {make_number_value(-1E300), "ERR11", "0"},
        {make_number_value(inf), "ERR11", "0"},
        {make_number_value(-inf), "ERR11", "0"},
        {make_number_value(nan), "ERR11", "ERR11"},
        {make_int64_value(std::numeric_limits<std::int64_t>::max()), "0", "0"},
        {make_int64_value(std::numeric_limits<std::int64_t>::min() + 5), "0", "/sys-sentinel/"},
        {make_uint64_value(UINT64_C(9223372036854775807)), "0", "0"},
        {make_uint64_value(UINT64_C(9223372036854775808)), "ERR11", "0"},
        {make_uint64_value(std::numeric_limits<std::uint64_t>::max()), "ERR11", "0"},
        // Nonnumeric/Currency coercion remains unchanged, but rounded values are checked before casting.
        {make_boolean_value(true), "0", "0"},
        {make_null_value(), "0", "0"},
        {make_string_value("5.9"), "0", "0"},
        {make_string_value("1E300"), "ERR11", "ERR11"},
        {make_currency_value(59000), "0", "0"}};
    for (const char* mode : {"COPPERFIN", "VFP9"}) {
        for (std::size_t i = 0; i < cases.size(); ++i) {
            int operation_callbacks = 0;
            const auto setting = [&](const std::string& name) {
                if (name == "NUMERICBEHAVIOR") { return std::string(mode); }
                ++operation_callbacks;
                return std::string("1");
            };
            std::string actual;
            try {
                const auto result = evaluate_runtime_surface_function(
                    "sys", {cases[i].value, make_number_value(1)}, {}, "/sys-sentinel/",
                    {}, {}, 0, {}, 0, {}, 0, {}, {}, {}, {}, {}, {}, {}, {}, setting,
                    {}, {}, false, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {});
                actual = result.has_value() && result->kind == PrgValueKind::string
                    ? result->string_value : "wrong-result-type";
            } catch (const PrgCompatibilityError& error) {
                actual = "ERR" + std::to_string(error.error_code());
            }
            const char* expected = std::string(mode) == "COPPERFIN" ? cases[i].copperfin : cases[i].vfp9;
            const std::string label = std::string(mode) + " SYS selector boundary " + std::to_string(i);
            expect(actual == expected, label + " should preserve checked selector admission");
            expect(operation_callbacks == 0, label + " must not alias a state-changing SYS operation");
        }
        // Positive control: the callback guard above is connected to actual dispatch.
        int operation_callbacks = 0;
        const auto setting = [&](const std::string& name) {
            if (name == "NUMERICBEHAVIOR") { return std::string(mode); }
            expect(name == std::string("__sys2030__\x1f") + "1",
                   "SYS(2030,1) should request the existing session-local switch");
            ++operation_callbacks;
            return std::string("1");
        };
        const auto result = evaluate_runtime_surface_function(
            "sys", {make_number_value(2030), make_number_value(1)}, {}, "/sys-sentinel/",
            {}, {}, 0, {}, 0, {}, 0, {}, {}, {}, {}, {}, {}, {}, {}, setting,
            {}, {}, false, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {});
        expect(result.has_value() && result->kind == PrgValueKind::number && result->number_value == 1,
               std::string(mode) + " valid SYS selector should retain its operation result");
        expect(operation_callbacks == 1,
               std::string(mode) + " valid SYS selector should reach the operation callback");
    }
}

// RQ-CF-PRG-CPCURRENT-NUMERIC-001 (#5611/#6776): bounded integer admission
// precedes configured/default lookup. Native query-domain/type parity is separate.
void test_cpcurrent_direct_numeric_boundaries() {
    using namespace copperfin::runtime;
    const double inf = std::numeric_limits<double>::infinity();
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double upper = 2147483648.0;
    enum class Result { configured, host, oem, error };
    struct Case { PrgValue value; Result expected; };
    const std::vector<Case> cases{
        {make_number_value(0), Result::configured},
        {make_number_value(-0.0), Result::configured},
        {make_number_value(1), Result::host},
        {make_number_value(2), Result::oem},
        {make_number_value(-1), Result::host}, // existing query-domain gaps remain separate
        {make_number_value(99), Result::host},
        {make_number_value(0.49), Result::error},
        {make_number_value(0.5), Result::error},
        {make_number_value(0.9), Result::error},
        {make_number_value(1.49), Result::error},
        {make_number_value(1.5), Result::error},
        {make_number_value(1.9), Result::error},
        {make_number_value(2.49), Result::error},
        {make_number_value(2.5), Result::error},
        {make_number_value(2.9), Result::error},
        {make_number_value(-0.9), Result::error},
        {make_number_value(upper - 1.0), Result::host},
        {make_number_value(std::nextafter(upper, 0.0)), Result::error},
        {make_number_value(upper), Result::error},
        {make_number_value(-upper), Result::host},
        {make_number_value(std::nextafter(-upper, -inf)), Result::error},
        {make_number_value(4294967298.0), Result::error},
        {make_number_value(-4294967294.0), Result::error},
        {make_number_value(1E300), Result::error},
        {make_number_value(-1E300), Result::error},
        {make_number_value(inf), Result::error},
        {make_number_value(-inf), Result::error},
        {make_number_value(nan), Result::error},
        {make_int64_value(2), Result::oem},
        {make_int64_value(std::numeric_limits<std::int32_t>::max()), Result::host},
        {make_int64_value(std::numeric_limits<std::int64_t>::max()), Result::error},
        {make_int64_value(std::numeric_limits<std::int64_t>::min() + 2), Result::error},
        {make_uint64_value(2), Result::oem},
        {make_uint64_value(UINT64_C(2147483648)), Result::error},
        {make_uint64_value(std::numeric_limits<std::uint64_t>::max()), Result::error},
        // Preserve other coercions behind checked rounding; native type parity remains separate.
        {make_currency_value(19000), Result::oem},
        {make_boolean_value(true), Result::host},
        {make_null_value(), Result::configured},
        {make_string_value("2"), Result::oem},
        {make_string_value("1E300"), Result::error}};
    for (const char* mode : {"COPPERFIN", "VFP9"}) {
        int configured_reads = 0;
        const auto setting = [&](const std::string& name) {
            if (name == "NUMERICBEHAVIOR") { return std::string(mode); }
            expect(name == "CODEPAGE", "CPCURRENT should not mutate or query unrelated session state");
            ++configured_reads;
            return std::string("777");
        };
        const auto call = [&](const std::vector<PrgValue>& arguments) {
            return evaluate_runtime_surface_function(
                "cpcurrent", arguments, {}, {}, {}, {}, 0, {}, 0, {}, 0, {}, {}, {}, {}, {}, {}, {}, {}, setting,
                {}, {}, false, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {});
        };
        const auto omitted = call({});
        expect(omitted.has_value() && omitted->kind == PrgValueKind::number && omitted->number_value == 777,
               std::string(mode) + " omitted CPCURRENT selector should preserve configuration lookup");
        expect(configured_reads == 1, "omitted CPCURRENT should read configuration exactly once");
        const auto host = call({make_number_value(1)});
        const auto oem = call({make_number_value(2)});
        expect(host.has_value() && host->kind == PrgValueKind::number &&
                   oem.has_value() && oem->kind == PrgValueKind::number,
               "CPCURRENT host/OEM controls should return code-page numbers");
        if (!host.has_value() || !oem.has_value()) { continue; }
        for (std::size_t i = 0; i < cases.size(); ++i) {
            configured_reads = 0;
            bool threw = false;
            std::optional<PrgValue> actual;
            try {
                actual = call({cases[i].value});
            } catch (const PrgCompatibilityError& error) {
                threw = true;
                expect(error.error_code() == 11, "invalid CPCURRENT selector should raise catchable error 11");
            }
            const std::string label = std::string(mode) + " CPCURRENT boundary " + std::to_string(i);
            const auto expected = cases[i].expected;
            if (expected == Result::error) {
                expect(threw, label + " should reject before selecting a code-page query");
            } else {
                const double expected_value = expected == Result::configured ? 777
                    : expected == Result::host ? host->number_value : oem->number_value;
                expect(!threw && actual.has_value() && actual->kind == PrgValueKind::number &&
                           actual->number_value == expected_value,
                       label + " should retain its configured/host/OEM query result");
            }
            expect(configured_reads == (expected == Result::configured ? 1 : 0),
                   label + " should preserve configured-state reads on rejection and other queries");
        }
    }
}

// RQ-CF-PRG-RELATION-INDEX-NUMERIC-001: native Numeric admission and derived
// exact-integer/NaN safety; callback captures isolate conversion from #6971 order.
void test_relation_index_direct_numeric_boundaries() {
    using namespace copperfin::runtime;
    const double inf = std::numeric_limits<double>::infinity();
    struct Case { PrgValue value; std::int64_t copperfin; std::int64_t vfp9; };
    // -1 means error 11; 0 means the preserved nonnumeric empty-result path.
    const std::vector<Case> cases{
        {make_number_value(1), 1, 1}, {make_number_value(2), 2, 2},
        {make_number_value(3), 3, 3}, {make_number_value(0), -1, -1},
        {make_number_value(-1), -1, -1}, {make_number_value(0.9), -1, -1},
        {make_number_value(std::nextafter(1.0, 0.0)), -1, -1},
        {make_number_value(1.49), 1, 1}, {make_number_value(1.5), 1, 1},
        {make_number_value(1.9), 1, 1}, {make_number_value(2.5), 2, 2},
        {make_number_value(9998.9), 9998, 9998}, {make_number_value(9999), 9999, 9999},
        {make_number_value(std::nextafter(9999.0, inf)), -1, -1},
        {make_number_value(9999.49), -1, -1}, {make_number_value(10000), -1, -1},
        {make_number_value(2147483647.0), -1, -1},
        {make_number_value(4294967297.0), -1, -1},
        {make_number_value(-4294967295.0), -1, 1},
        {make_number_value(-4294967294.0), -1, 2},
        {make_number_value(-4294957297.0), -1, 9999},
        {make_number_value(-4294957296.0), -1, -1},
        {make_number_value(1E300), -1, -1}, {make_number_value(-1E300), -1, -1},
        {make_number_value(inf), -1, -1}, {make_number_value(-inf), -1, -1},
        {make_number_value(std::numeric_limits<double>::quiet_NaN()), -1, -1},
        {make_int64_value(2), 2, 2}, {make_uint64_value(9999), 9999, 9999},
        {make_int64_value(-4294967295LL), -1, 1},
        {make_int64_value(std::numeric_limits<std::int64_t>::max()), -1, -1},
        {make_uint64_value(std::numeric_limits<std::uint64_t>::max()), -1, -1},
        // Other coercions remain range-checked preservation, not native parity.
        {make_currency_value(19000), 2, 2}, {make_boolean_value(true), 1, 1},
        {make_null_value(), 0, 0}, {make_string_value("1.9"), 2, 2},
        {make_string_value("1E300"), -1, -1}};
    for (const char* mode : {"COPPERFIN", "VFP9"}) {
        for (const char* function : {"relation", "target"}) {
            for (std::size_t i = 0; i < cases.size(); ++i) {
                const auto expected = std::string(mode) == "COPPERFIN" ? cases[i].copperfin : cases[i].vfp9;
                int callbacks = 0;
                const auto setting = [&](const std::string& name) {
                    if (name == "NUMERICBEHAVIOR") { return std::string(mode); }
                    ++callbacks;
                    const std::string prefix = std::string("__relation_introspection__\x1f") + function + "\x1f";
                    expect(name == prefix + std::to_string(expected) + "\x1fParentSentinel",
                           "RELATION/TARGET should preserve the exact index and work-area designator");
                    return std::string("query-sentinel");
                };
                bool threw = false;
                std::optional<PrgValue> actual;
                try {
                    actual = evaluate_runtime_surface_function(
                        function, {cases[i].value, make_string_value("ParentSentinel")}, {}, {}, {}, {}, 0, {}, 0,
                        {}, 0, {}, {}, {}, {}, {}, {}, {}, {}, setting,
                        {}, {}, false, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {});
                } catch (const PrgCompatibilityError& error) {
                    threw = true;
                    expect(error.error_code() == 11, "invalid relation index should raise catchable error 11");
                }
                const std::string label = std::string(mode) + " " + function + " boundary " + std::to_string(i);
                if (expected == -1) {
                    expect(threw, label + " should reject before introspection");
                } else {
                    expect(!threw && actual.has_value() && actual->kind == PrgValueKind::string &&
                               actual->string_value == (expected == 0 ? "" : "query-sentinel"),
                           label + " should preserve the converted query result");
                }
                expect(callbacks == (expected > 0 ? 1 : 0),
                       label + " should not introspect rejected or empty indexes");
            }
        }
    }
}

// RQ-CF-PRG-SET-TEXTMERGE-NUMERIC-001: only Numeric admission is native;
// callback controls preserve independent query-result and other SET contracts.
void test_set_textmerge_direct_numeric_boundaries() {
    using namespace copperfin::runtime;
    const double inf = std::numeric_limits<double>::infinity();
    struct Case { PrgValue value; int copperfin; int vfp9; };
    // -1 is error 11; positive values identify the admitted Numeric variant.
    const std::vector<Case> cases{
        {make_number_value(1), 1, 1}, {make_number_value(2), 2, 2},
        {make_number_value(3), 3, 3}, {make_number_value(4), 4, 4},
        {make_number_value(0), -1, -1}, {make_number_value(-1), -1, -1},
        {make_number_value(0.9), -1, -1}, {make_number_value(std::nextafter(1.0, 0.0)), -1, -1},
        {make_number_value(1.49), 1, 1}, {make_number_value(1.5), 1, 1},
        {make_number_value(1.9), 1, 1}, {make_number_value(2.5), 2, 2},
        {make_number_value(3.9), 3, 3}, {make_number_value(4.9), 4, 4},
        {make_number_value(std::nextafter(5.0, 0.0)), 4, 4}, {make_number_value(5), -1, -1},
        {make_number_value(2147483647), -1, -1},
        {make_number_value(4294967297.0), -1, 1}, {make_number_value(4294967299.0), -1, 3},
        {make_number_value(4294967300.0), -1, 4}, {make_number_value(4294967296.0), -1, -1},
        {make_number_value(-4294967295.0), -1, 1}, {make_number_value(-4294967293.0), -1, 3},
        {make_number_value(-4294967292.0), -1, 4},
        {make_number_value(1E300), -1, -1}, {make_number_value(-1E300), -1, -1},
        {make_number_value(inf), -1, -1}, {make_number_value(-inf), -1, -1},
        {make_number_value(std::numeric_limits<double>::quiet_NaN()), -1, -1},
        {make_int64_value(3), 3, 3}, {make_uint64_value(4), 4, 4},
        {make_int64_value(std::numeric_limits<std::int64_t>::min() + 1), -1, 1},
        {make_uint64_value(UINT64_C(9223372036854775809)), -1, 1}};
    for (const char* mode : {"COPPERFIN", "VFP9"}) {
        const auto evaluate = [&](const std::string& option, const std::optional<PrgValue>& variant,
                                  const std::string& expected_key, const bool reject) {
            int callbacks = 0;
            const auto setting = [&](const std::string& name) {
                if (name == "NUMERICBEHAVIOR") { return std::string(mode); }
                ++callbacks;
                expect(name == expected_key, "SET query should preserve its admitted variant key");
                return std::string("query-sentinel");
            };
            std::vector<PrgValue> arguments{make_string_value(option)};
            if (variant.has_value()) { arguments.push_back(*variant); }
            bool threw = false;
            std::optional<PrgValue> actual;
            try {
                actual = evaluate_runtime_surface_function(
                    "set", arguments, {}, {}, {}, {}, 0, {}, 0, {}, 0,
                    {}, {}, {}, {}, {}, {}, {}, {}, setting,
                    {}, {}, false, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {});
            } catch (const PrgCompatibilityError& error) {
                threw = true;
                expect(error.error_code() == 11, "invalid TEXTMERGE variant should raise error 11");
            }
            expect(reject ? threw : !threw && actual.has_value() && actual->kind == PrgValueKind::string &&
                                     actual->string_value == "query-sentinel",
                   std::string(mode) + " SET variant should preserve result or catchable rejection");
            expect(callbacks == (reject ? 0 : 1), "rejected variant should not query setting state");
        };
        for (const auto& row : cases) {
            const int index = std::string(mode) == "COPPERFIN" ? row.copperfin : row.vfp9;
            const std::string key = index == 1 || index == 3
                ? "TeXtMeRgE," + std::to_string(index) : "TeXtMeRgE";
            evaluate("TeXtMeRgE", row.value, key, index == -1);
        }
        // Preserve other coercions and omitted/other-SET variants, not native type parity.
        evaluate("TEXTMERGE", make_currency_value(19000), "TEXTMERGE", false);
        evaluate("TEXTMERGE", make_string_value(" 3 "), "TEXTMERGE,3", false);
        evaluate("TEXTMERGE", make_boolean_value(true), "TEXTMERGE", false);
        evaluate("TEXTMERGE", make_null_value(), "TEXTMERGE", false);
        evaluate("TEXTMERGE", std::nullopt, "TEXTMERGE", false);
        evaluate("EXACT", make_number_value(inf), "EXACT", false);
        evaluate("TEXTMERGE", make_string_value("1E300"), "TEXTMERGE", false);
    }
}

// RQ-CF-PRG-ALINES-FLAGS-NUMERIC-001: exact integers/NaN are derived safety
// boundaries; other coercions are preservation controls, not native type parity.
void test_alines_flags_direct_numeric_boundaries() {
    using namespace copperfin::runtime;
    const double inf = std::numeric_limits<double>::infinity();
    struct Case { PrgValue value; int copperfin; int vfp9; };
    // -1 denotes rejected admission; otherwise the exact converted flags.
    const std::vector<Case> cases{
        {make_number_value(0), 0, 0},
        {make_number_value(1), 1, 1},
        {make_number_value(2), 2, 2},
        {make_number_value(3), 3, 3},
        {make_number_value(4), 4, 4},
        {make_number_value(31), 31, 31},
        {make_number_value(32), -1, -1},
        {make_number_value(-1), -1, -1},
        {make_number_value(0.49), 0, 0},
        {make_number_value(0.5), 0, 0},
        {make_number_value(0.9), 0, 0},
        {make_number_value(1.49), 1, 1},
        {make_number_value(1.5), 1, 1},
        {make_number_value(1.9), 1, 1},
        {make_number_value(2.5), 2, 2},
        {make_number_value(31.49), 31, 31},
        {make_number_value(31.9), 31, 31},
        {make_number_value(32.5), -1, -1},
        {make_number_value(2147483647), -1, -1},
        {make_number_value(2147483648), -1, -1},
        {make_number_value(-2147483648), -1, -1},
        {make_number_value(-2147483649), -1, -1},
        {make_number_value(4294967297.0), -1, 1},
        {make_number_value(-4294967295.0), -1, 1},
        {make_number_value(4294967327.0), -1, 31},
        {make_number_value(-4294967265.0), -1, 31},
        {make_number_value(1E300), -1, 0},
        {make_number_value(-1E300), -1, 0},
        {make_number_value(inf), -1, 0},
        {make_number_value(-inf), -1, 0},
        {make_number_value(-0.49), 0, 0},
        {make_number_value(-0.5), 0, 0},
        {make_number_value(-0.9), 0, 0},
        {make_number_value(-1.1), -1, -1},
        {make_number_value(4294967328.0), -1, -1},
        {make_number_value(-4294967264.0), -1, -1},
        {make_number_value(std::nextafter(-1.0, 0.0)), 0, 0},
        {make_number_value(std::nextafter(-1.0, -inf)), -1, -1},
        {make_number_value(std::nextafter(1.0, 0.0)), 0, 0},
        {make_number_value(std::nextafter(32.0, 0.0)), 31, 31},
        {make_number_value(std::nextafter(32.0, inf)), -1, -1},
        {make_number_value(std::numeric_limits<double>::quiet_NaN()), -1, 0},
        {make_int64_value(0), 0, 0}, {make_int64_value(31), 31, 31},
        {make_int64_value(-1), -1, -1}, {make_int64_value(32), -1, -1},
        {make_int64_value(std::numeric_limits<std::int64_t>::min()), -1, 0},
        {make_int64_value(std::numeric_limits<std::int64_t>::min() + 1), -1, 1},
        {make_int64_value(std::numeric_limits<std::int64_t>::max()), -1, -1},
        {make_int64_value(INT64_C(9007199254740993)), -1, 1},
        {make_uint64_value(0), 0, 0}, {make_uint64_value(31), 31, 31},
        {make_uint64_value(32), -1, -1},
        {make_uint64_value(UINT64_C(9223372036854775839)), -1, 31},
        {make_uint64_value(UINT64_C(9223372036854775840)), -1, -1},
        {make_uint64_value(std::numeric_limits<std::uint64_t>::max()), -1, -1},
        {make_string_value("0.5"), 1, 1}, {make_string_value("1.5"), 2, 2},
        {make_string_value("32"), 32, 32},
        {make_string_value("2147483647"), 2147483647, 2147483647},
        {make_string_value("2147483647.5"), -1, -1},
        {make_string_value("2147483648"), -1, -1},
        {make_string_value("-2147483648.5"), -1, -1},
        {make_string_value("1E300"), -1, -1},
        {make_currency_value(15000), 2, 2},
        {make_currency_value(std::numeric_limits<std::int64_t>::max()), -1, -1},
        {make_boolean_value(true), 1, 1}, {make_boolean_value(false), 0, 0},
        {make_null_value(), 0, 0}};
    for (const auto behavior : {NumericBehavior::copperfin, NumericBehavior::vfp9}) {
        for (std::size_t index = 0; index < cases.size(); ++index) {
            const auto& row = cases[index];
            const int expected = behavior == NumericBehavior::copperfin ? row.copperfin : row.vfp9;
            const auto actual = checked_alines_flags_argument(row.value, behavior);
            expect(expected == -1 ? !actual.has_value() : actual.has_value() && *actual == expected,
                   "ALINES exact flags/rejection case " + std::to_string(index) +
                   (behavior == NumericBehavior::copperfin ? " COPPERFIN" : " VFP9"));
        }
    }
}

}  // namespace

int main() {
    test_conversion_helpers();
    test_date_time_constructor_direct_numeric_boundaries();
    test_dow_direct_numeric_boundaries();
    test_week_direct_numeric_boundaries();
    test_rgb_direct_numeric_boundaries();
    test_rand_direct_numeric_boundaries();
    test_hex_direct_numeric_boundaries();
    test_sys_selector_direct_numeric_boundaries();
    test_cpcurrent_direct_numeric_boundaries();
    test_relation_index_direct_numeric_boundaries();
    test_set_textmerge_direct_numeric_boundaries();
    test_alines_flags_direct_numeric_boundaries();
    test_numeric_behavior_script_rows();
    test_gomonth_out_of_range_dbf_round_trip();
    if (const int failures = test_failures(); failures != 0) {
        std::cerr << failures << " test(s) failed\n";
        return 1;
    }
    return 0;
}
