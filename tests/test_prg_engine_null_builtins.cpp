// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/runtime/prg_engine.h"
#include "prg_engine_test_support.h"

#include <filesystem>
#include <iostream>
#include <string>
#include <system_error>
#include <vector>

namespace {

using namespace copperfin::test_support;

namespace fs = std::filesystem;

// Governing requirement: RQ-CF-PRG-NULL-BUILTIN-PROPAGATION-001 (#5936).
//
// Every expectation below is installed VFP9 (09.00.0000.7423, Windows VM COM probe 2026-09-30, retained at
// /home/rich/temp/vfp9-probes/null-semantics-c24/vfp9-result.txt and vfp9-result-funcs.txt), recorded as
// VARTYPE():value. A built-in that VFP9 defines as null-propagating returns NULL (VARTYPE X) when any
// argument is NULL, whichever argument it is; ISALPHA/ISDIGIT/ISLOWER/ISUPPER return .F. instead.

struct Row {
    const char *expression;
    const char *expected;   // "<VARTYPE>:<value>", with NULL written as "X:.NULL."
};


const std::vector<Row> kRows = {
        {"ABS(.NULL.)", "X:.NULL."},
        {"LEN(.NULL.)", "X:.NULL."},
        {"UPPER(.NULL.)", "X:.NULL."},
        {"LOWER(.NULL.)", "X:.NULL."},
        {"LEFT(.NULL.,1)", "X:.NULL."},
        {"RIGHT(.NULL.,1)", "X:.NULL."},
        {"SUBSTR(.NULL.,1,1)", "X:.NULL."},
        {"LEFT('abc',.NULL.)", "X:.NULL."},
        {"TRIM(.NULL.)", "X:.NULL."},
        {"LTRIM(.NULL.)", "X:.NULL."},
        {"ALLTRIM(.NULL.)", "X:.NULL."},
        {"AT('a',.NULL.)", "X:.NULL."},
        {"AT(.NULL.,'abc')", "X:.NULL."},
        {"STR(.NULL.)", "X:.NULL."},
        {"STR(1,.NULL.)", "X:.NULL."},
        {"VAL(.NULL.)", "X:.NULL."},
        {"INT(.NULL.)", "X:.NULL."},
        {"ROUND(.NULL.,1)", "X:.NULL."},
        {"ROUND(1.5,.NULL.)", "X:.NULL."},
        {"MAX(.NULL.,1)", "X:.NULL."},
        {"MIN(.NULL.,1)", "X:.NULL."},
        {"MOD(.NULL.,2)", "X:.NULL."},
        {"MOD(5,.NULL.)", "X:.NULL."},
        {"SQRT(.NULL.)", "X:.NULL."},
        {"CEILING(.NULL.)", "X:.NULL."},
        {"FLOOR(.NULL.)", "X:.NULL."},
        {"EXP(.NULL.)", "X:.NULL."},
        {"LOG(.NULL.)", "X:.NULL."},
        {"SIGN(.NULL.)", "X:.NULL."},
        {"YEAR(.NULL.)", "X:.NULL."},
        {"MONTH(.NULL.)", "X:.NULL."},
        {"DAY(.NULL.)", "X:.NULL."},
        {"HOUR(.NULL.)", "X:.NULL."},
        {"DOW(.NULL.)", "X:.NULL."},
        {"DTOC(.NULL.)", "X:.NULL."},
        {"DTOS(.NULL.)", "X:.NULL."},
        {"CTOD(.NULL.)", "X:.NULL."},
        {"TTOC(.NULL.)", "X:.NULL."},
        {"PADL(.NULL.,5)", "X:.NULL."},
        {"PADL('a',.NULL.)", "X:.NULL."},
        {"REPLICATE(.NULL.,2)", "X:.NULL."},
        {"REPLICATE('a',.NULL.)", "X:.NULL."},
        {"CHR(.NULL.)", "X:.NULL."},
        {"ASC(.NULL.)", "X:.NULL."},
        {"SPACE(.NULL.)", "X:.NULL."},
        {"STRTRAN(.NULL.,'a','b')", "X:.NULL."},
        {"CHRTRAN(.NULL.,'a','b')", "X:.NULL."},
        {"ISDIGIT(.NULL.)", "L:.F."},
        {"ISALPHA(.NULL.)", "L:.F."},
        {"ISUPPER(.NULL.)", "L:.F."},
        {"LIKE('a*',.NULL.)", "X:.NULL."},
        {"PROPER(.NULL.)", "X:.NULL."},
        {"STUFF(.NULL.,1,1,\"x\")", "X:.NULL."},
        {"OCCURS(\"a\",.NULL.)", "X:.NULL."},
        {"LEN(TRIM(.NULL.))", "X:.NULL."},
        {"EMPTY(ABS(.NULL.))", "L:.F."},
        {"BITAND(.NULL.,1)", "X:.NULL."},
        {"UPPER(LEFT(.NULL.,1))", "X:.NULL."},
        {"ISDIGIT(.NULL.)", "L:.F."},
        {"ISALPHA(.NULL.)", "L:.F."},
        {"ISLOWER(.NULL.)", "L:.F."},
        {"ISUPPER(.NULL.)", "L:.F."},
        // Scalar MIN/MAX propagate; the NULL-aware functions keep their own behavior.
        {"MAX(1,.NULL.,3)", "X:.NULL."},
        {"MIN(.NULL.,1)", "X:.NULL."},
        {"MAX(1,2,3)", "N:3"},
        {"MIN(4,2,9)", "N:2"},
        {"NVL(UPPER(.NULL.),'x')", "C:x"},
        {"ISNULL(LEFT(.NULL.,1))", "L:.T."},
        {"LEN('abc')", "N:3"},
        {"UPPER('abc')", "C:ABC"},
        {"ABS(-3)", "N:3"},
        {"SUBSTR('abcdef',2,3)", "C:bcd"},
        // Copperfin date extensions (not VFP9 functions, so these rows follow their VFP9 siblings and are not
        // VFP-backed): NULL in, NULL out.
        {"QUARTER(.NULL.)", "X:.NULL."},
        {"EOMONTH(.NULL.)", "X:.NULL."},
        {"DTOJ(.NULL.)", "X:.NULL."},
        {"ISLEAPYEAR(.NULL.)", "X:.NULL."}
};

std::string evaluate_all(const fs::path &dir) {
    std::string body = "cOut = ''\n";
    for (std::size_t index = 0U; index < kRows.size(); ++index) {
        body += "x = " + std::string(kRows[index].expression) + "\n";
        body += "cOut = cOut + VARTYPE(x) + ':' + IIF(ISNULL(x), '.NULL.', IIF(VARTYPE(x) = 'L', IIF(x, '.T.', '.F.'), "
                "TRANSFORM(x))) + CHR(10)\n";
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

void test_builtins_return_null_for_a_null_argument_as_vfp9_does() {
    const fs::path dir = fs::temp_directory_path() / "copperfin_null_semantics_rows";
    std::error_code ignored;
    fs::remove_all(dir, ignored);
    const std::string output = evaluate_all(dir);
    expect(output.rfind("<incomplete", 0U) != 0U, "NULL builtins: the script should complete: " + output);
    std::vector<std::string> lines;
    for (std::size_t start = 0U; start < output.size();) {
        const std::size_t end = output.find('\n', start);
        lines.push_back(output.substr(start, end == std::string::npos ? std::string::npos : end - start));
        if (end == std::string::npos) {
            break;
        }
        start = end + 1U;
    }
    expect(lines.size() >= kRows.size(), "NULL builtins: one result per expression, got " + std::to_string(lines.size()));
    for (std::size_t index = 0U; index < kRows.size() && index < lines.size(); ++index) {
        expect(lines[index] == kRows[index].expected,
            std::string("NULL builtins: ") + kRows[index].expression + " expected [" + kRows[index].expected + "], got [" + lines[index] + "]");
    }
    fs::remove_all(dir, ignored);
}

// A NULL read from a nullable DBF field propagates through built-ins the same way.
void test_a_null_dbf_field_propagates_through_builtins() {
    const fs::path dir = fs::temp_directory_path() / "copperfin_null_builtins_dbf";
    std::error_code ignored;
    fs::remove_all(dir, ignored);
    fs::create_directories(dir / "script");
    write_text(dir / "script" / "dbf.prg",
        "CREATE TABLE t (NAME C(10) NULL, AMT N(6,2) NULL)\n"
        "APPEND BLANK\nREPLACE NAME WITH 'abc', AMT WITH 5\n"
        "APPEND BLANK\nREPLACE NAME WITH .NULL., AMT WITH .NULL.\n"
        "GO 1\n"
        "c1 = VARTYPE(UPPER(NAME)) + VARTYPE(ABS(AMT)) + VARTYPE(LEN(NAME))\n"
        "GO 2\n"
        "c2 = VARTYPE(UPPER(NAME)) + VARTYPE(ABS(AMT)) + VARTYPE(LEN(NAME)) + VARTYPE(LEFT(NAME,1)) + VARTYPE(ROUND(AMT,1))\n"
        "COUNT FOR LEN(NAME) > 0 TO nLong\n"
        "STRTOFILE(c1 + ',' + c2 + ',' + TRANSFORM(nLong), 'out.txt')\n"
        "USE\nRETURN\n");
    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options((dir / "script" / "dbf.prg").string(), dir.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "NULL builtins dbf: the script should complete: " + state.message);
    // A non-null row keeps C/N/N; the NULL row is X for every propagating built-in, and LEN(NAME) > 0 is
    // NULL on that row so only the non-null row is counted.
    expect(read_text(dir / "out.txt") == "CNN,XXXXX,1", "NULL builtins dbf: got [" + read_text(dir / "out.txt") + "]");
    fs::remove_all(dir, ignored);
}

// The rule applies only to a call the built-in would actually answer. A call with an unsupported argument
// count is not turned into NULL (installed VFP9 reports an invalid argument count for these; Copperfin's
// normal dispatch is unchanged), and a user routine that merely shares a built-in's name is still reached.
void test_propagation_does_not_intercept_malformed_calls_or_user_routines() {
    const fs::path dir = fs::temp_directory_path() / "copperfin_null_builtins_arity";
    std::error_code ignored;
    fs::remove_all(dir, ignored);
    fs::create_directories(dir / "script");
    write_text(dir / "script" / "arity.prg",
        "LOCAL cOut, oEx, x\n"
        "cOut = ''\n"
        "TRY\nx = ABS(.NULL., 1)\ncOut = cOut + 'abs2=' + VARTYPE(x) + CHR(10)\nCATCH TO oEx\ncOut = cOut + 'abs2=ERR' + CHR(10)\nENDTRY\n"
        "TRY\nx = LEN(.NULL., 2)\ncOut = cOut + 'len2=' + VARTYPE(x) + CHR(10)\nCATCH TO oEx\ncOut = cOut + 'len2=ERR' + CHR(10)\nENDTRY\n"
        "TRY\nx = LEFT(.NULL.)\ncOut = cOut + 'left1=' + VARTYPE(x) + CHR(10)\nCATCH TO oEx\ncOut = cOut + 'left1=ERR' + CHR(10)\nENDTRY\n"
        "STRTOFILE(cOut, 'out.txt')\n"
        "RETURN\n");
    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options((dir / "script" / "arity.prg").string(), dir.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "NULL builtins arity: the script should complete: " + state.message);
    const std::string out = read_text(dir / "out.txt");
    for (const char *name : {"abs2=", "len2=", "left1="}) {
        const std::size_t at = out.find(name);
        expect(at != std::string::npos, std::string("NULL builtins arity: missing ") + name);
        if (at != std::string::npos) {
            const std::string rest = out.substr(at + std::string(name).size(), 1U);
            expect(rest != "X", std::string("NULL builtins arity: a malformed ") + name + " call must not become NULL, got [" + out + "]");
        }
    }

    // A user routine named like a built-in is reached when the call is not a valid built-in call.
    fs::remove_all(dir, ignored);
    fs::create_directories(dir / "script");
    write_text(dir / "script" / "udf.prg",
        "x = LEFT(.NULL.)\n"
        "STRTOFILE(VARTYPE(x) + ':' + IIF(ISNULL(x), 'NULL', x), 'udf.txt')\n"
        "RETURN\n"
        "FUNCTION LEFT\nLPARAMETERS pValue\nRETURN 'udf'\nENDFUNC\n");
    auto udf_session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options((dir / "script" / "udf.prg").string(), dir.string(), false));
    const auto udf_state = udf_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(udf_state.completed, "NULL builtins udf: the script should complete: " + udf_state.message);
    expect(read_text(dir / "udf.txt") == "C:udf",
        "NULL builtins udf: a one-argument user FUNCTION LEFT must be reached, got [" + read_text(dir / "udf.txt") + "]");
    fs::remove_all(dir, ignored);
}

}  // namespace

int main() {
    test_builtins_return_null_for_a_null_argument_as_vfp9_does();
    test_a_null_dbf_field_propagates_through_builtins();
    test_propagation_does_not_intercept_malformed_calls_or_user_routines();
    if (const int failures = test_failures(); failures != 0) {
        std::cerr << failures << " test(s) failed\n";
        return 1;
    }
    return 0;
}
