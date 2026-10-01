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

// Governing requirement: RQ-CF-PRG-NULL-OPERATOR-SEMANTICS-001 (#5934, #5942, #5979, #6140).
//
// Every expectation below is installed VFP9 (09.00.0000.7423, Windows VM COM probe 2026-09-30, retained at
// /home/rich/temp/vfp9-probes/null-semantics-c24/vfp9-result.txt), recorded as VARTYPE():value. NULL
// propagates through comparison and arithmetic operators, AND/OR/NOT are three-valued (a deciding
// operand wins even next to NULL), BETWEEN and INLIST follow three-valued logic, and EMPTY()/ISBLANK()
// report NULL as neither empty nor blank.

struct Row {
    const char *expression;
    const char *expected;   // "<VARTYPE>:<value>", with NULL written as "X:.NULL."
};

const std::vector<Row> kRows = {
        {".NULL. = .NULL.", "X:.NULL."},
        {".NULL. == .NULL.", "X:.NULL."},
        {".NULL. <> 1", "X:.NULL."},
        {".NULL. < 1", "X:.NULL."},
        {".NULL. > 1", "X:.NULL."},
        {"1 = .NULL.", "X:.NULL."},
        {"'a' = .NULL.", "X:.NULL."},
        {"'a' == .NULL.", "X:.NULL."},
        {".NULL. == 'a'", "X:.NULL."},
        {"'a' $ .NULL.", "X:.NULL."},
        {".NULL. $ 'abc'", "X:.NULL."},
        {".NULL. = .T.", "X:.NULL."},
        {".NULL. = {^2026-01-01}", "X:.NULL."},
        {".NULL. <= 1", "X:.NULL."},
        {".NULL. >= 1", "X:.NULL."},
        {".NULL. != 1", "X:.NULL."},
        {".NULL. # 1", "X:.NULL."},
        {".NULL. + 1", "X:.NULL."},
        {"1 + .NULL.", "X:.NULL."},
        {".NULL. - 1", "X:.NULL."},
        {"1 - .NULL.", "X:.NULL."},
        {".NULL. * 2", "X:.NULL."},
        {".NULL. / 2", "X:.NULL."},
        {".NULL. ^ 2", "X:.NULL."},
        {".NULL. % 2", "X:.NULL."},
        {"-.NULL.", "X:.NULL."},
        {"+.NULL.", "X:.NULL."},
        {".NULL. + .NULL.", "X:.NULL."},
        {"'a' + .NULL.", "X:.NULL."},
        {".NULL. + 'a'", "X:.NULL."},
        {"{^2026-01-01} + .NULL.", "X:.NULL."},
        {"{^2026-01-01} - .NULL.", "X:.NULL."},
        {"1 + 2 * .NULL.", "X:.NULL."},
        {".NULL. AND .T.", "X:.NULL."},
        {".NULL. AND .F.", "L:.F."},
        {".T. AND .NULL.", "X:.NULL."},
        {".F. AND .NULL.", "L:.F."},
        {".NULL. AND .NULL.", "X:.NULL."},
        {".NULL. OR .T.", "L:.T."},
        {".NULL. OR .F.", "X:.NULL."},
        {".T. OR .NULL.", "L:.T."},
        {".F. OR .NULL.", "X:.NULL."},
        {".NULL. OR .NULL.", "X:.NULL."},
        {"NOT .NULL.", "X:.NULL."},
        {"!.NULL.", "X:.NULL."},
        {".NOT. .NULL.", "X:.NULL."},
        {"INLIST(.NULL.,.NULL.)", "X:.NULL."},
        {"INLIST(.NULL.,0)", "X:.NULL."},
        {"INLIST(1,.NULL.,1)", "L:.T."},
        {"INLIST(1,.NULL.,2)", "X:.NULL."},
        {"INLIST(1,2,3)", "L:.F."},
        {"INLIST(1,1,.NULL.)", "L:.T."},
        {"INLIST('a','b',.NULL.)", "X:.NULL."},
        {"INLIST(.NULL.,.NULL.,1)", "X:.NULL."},
        {"EMPTY(.NULL.)", "L:.F."},
        {"ISBLANK(.NULL.)", "L:.F."},
        {"ISNULL(.NULL.)", "L:.T."},
        {"ISNULL(0)", "L:.F."},
        {"NVL(.NULL.,5)", "N:5"},
        {"NVL(0,5)", "N:0"},
        {"NVL(.NULL.,.NULL.)", "X:.NULL."},
        {"EVL(.NULL.,5)", "X:.NULL."},
        {"EVL(0,5)", "N:5"},
        {"EVL('',5)", "N:5"},
        {"EVL(.F.,5)", "N:5"},
        {"EVL(.NULL.,.NULL.)", "X:.NULL."},
        {"VARTYPE(.NULL.)", "C:X"},
        {"IIF(.NULL.,1,2)", "N:2"},
        {"BETWEEN(.NULL.,0,1)", "X:.NULL."},
        {"BETWEEN(1,.NULL.,2)", "X:.NULL."},
        {"BETWEEN(1,0,.NULL.)", "X:.NULL."},
        {"BETWEEN(5,.NULL.,2)", "L:.F."},
        {"BETWEEN(0,1,.NULL.)", "L:.F."}
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

void test_operators_and_predicates_match_vfp9_three_valued_semantics() {
    const fs::path dir = fs::temp_directory_path() / "copperfin_null_semantics_rows";
    std::error_code ignored;
    fs::remove_all(dir, ignored);
    const std::string output = evaluate_all(dir);
    expect(output.rfind("<incomplete", 0U) != 0U, "NULL rows: the script should complete: " + output);
    std::vector<std::string> lines;
    for (std::size_t start = 0U; start < output.size();) {
        const std::size_t end = output.find('\n', start);
        lines.push_back(output.substr(start, end == std::string::npos ? std::string::npos : end - start));
        if (end == std::string::npos) {
            break;
        }
        start = end + 1U;
    }
    expect(lines.size() >= kRows.size(), "NULL rows: one result per expression, got " + std::to_string(lines.size()));
    for (std::size_t index = 0U; index < kRows.size() && index < lines.size(); ++index) {
        expect(lines[index] == kRows[index].expected,
            std::string("NULL rows: ") + kRows[index].expression + " expected [" + kRows[index].expected + "], got [" + lines[index] + "]");
    }
    fs::remove_all(dir, ignored);
}

// A NULL read from a nullable DBF field behaves the same way in commands that filter on it.
void test_a_null_dbf_field_follows_the_same_rules() {
    const fs::path dir = fs::temp_directory_path() / "copperfin_null_semantics_dbf";
    std::error_code ignored;
    fs::remove_all(dir, ignored);
    fs::create_directories(dir / "script");
    write_text(dir / "script" / "dbf.prg",
        "CREATE TABLE t (NAME C(10) NULL, AMT N(6,2) NULL)\n"
        "APPEND BLANK\nREPLACE NAME WITH 'x', AMT WITH 5\n"
        "APPEND BLANK\nREPLACE NAME WITH .NULL., AMT WITH .NULL.\n"
        "APPEND BLANK\nREPLACE NAME WITH '', AMT WITH 0\n"
        "COUNT FOR NAME = 'x' TO nEq\n"
        "COUNT FOR NAME <> 'x' TO nNe\n"
        "COUNT FOR ISNULL(NAME) TO nNull\n"
        "COUNT FOR EMPTY(NAME) TO nEmpty\n"
        "COUNT FOR ISBLANK(NAME) TO nBlank\n"
        "COUNT FOR AMT + 1 > 0 TO nSum\n"
        "COUNT FOR NOT NAME = 'x' TO nNot\n"
        "GO 2\n"
        "cEmpty = IIF(EMPTY(NAME), 'T', 'F')\n"
        "cCmp = VARTYPE(NAME = 'x') + VARTYPE(AMT + 1) + VARTYPE(NAME $ 'abc')\n"
        "STRTOFILE(TRANSFORM(nEq) + ',' + TRANSFORM(nNe) + ',' + TRANSFORM(nNull) + ',' + TRANSFORM(nEmpty) + ',' + "
        "TRANSFORM(nBlank) + ',' + TRANSFORM(nSum) + ',' + TRANSFORM(nNot) + ',' + cEmpty + ',' + cCmp, 'out.txt')\n"
        "USE\nRETURN\n");
    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options((dir / "script" / "dbf.prg").string(), dir.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "NULL dbf: the script should complete: " + state.message);
    // Installed VFP9 gives exactly this for the same script (retained at
    // ~/temp/vfp9-probes/null-semantics-c24/vfp9-result-dbf.txt). Rows: 1 'x'/5, 2 NULL/NULL, 3 ''/0.
    // A NULL comparison selects nothing in either direction (NAME = 'x' and NAME <> 'x' each match one
    // row, NOT NAME = 'x' matches only the blank row), EMPTY()/ISBLANK() count only the blank row, and
    // comparison, arithmetic and $ on the NULL row are VARTYPE X.
    expect(read_text(dir / "out.txt") == "1,1,1,1,1,2,1,F,XXX", "NULL dbf: got [" + read_text(dir / "out.txt") + "]");
    fs::remove_all(dir, ignored);
}

}  // namespace

int main() {
    test_operators_and_predicates_match_vfp9_three_valued_semantics();
    test_a_null_dbf_field_follows_the_same_rules();
    if (const int failures = test_failures(); failures != 0) {
        std::cerr << failures << " test(s) failed\n";
        return 1;
    }
    return 0;
}
