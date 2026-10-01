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

// Governing requirement: RQ-CF-PRG-TYPED-NULL-001 (#6506).
//
// Every expectation is installed VFP9 (09.00.0000.7423, Windows VM COM probe 2026-10-01, retained at
// /home/rich/temp/vfp9-probes/typed-null-c24/result.txt): VARTYPE of a NULL field is X, VARTYPE(x, .T.) is the
// field's declared type (Integer, Double and Float report N, a Memo reports C, the others their own letter),
// VARTYPE(x, .F.) is X, TYPE() looks through the NULL (a Memo stays M), a variable copied from a NULL field keeps
// its type, the .NULL. literal is X either way, and a second argument that is not Logical is error 11. Two cases
// are left out because they differ here: the NULL result of an operation (VFP9 reports N for VARTYPE(cN + 1, .T.))
// and a variable assigned the .NULL. literal (VFP9 reports L for VARTYPE(w, .T.)); both stay untyped (X). The expected text is the result, or "ERR<number>".

struct Row {
    const char *expression;
    const char *expected;
};

const std::vector<Row> kRows = {
    // A NULL second argument is not Logical either (result2.txt).
    {"VARTYPE(cN,.NULL.)", "ERR11"},
    {"VARTYPE(10,.NULL.)", "ERR11"},
    {"VARTYPE(cN)", "X"},
    {"VARTYPE(cN,.T.)", "N"},
    {"VARTYPE(cN,.F.)", "X"},
    {"TYPE('cN')", "N"},
    {"VARTYPE(cC)", "X"},
    {"VARTYPE(cC,.T.)", "C"},
    {"VARTYPE(cC,.F.)", "X"},
    {"TYPE('cC')", "C"},
    {"VARTYPE(cD)", "X"},
    {"VARTYPE(cD,.T.)", "D"},
    {"VARTYPE(cD,.F.)", "X"},
    {"TYPE('cD')", "D"},
    {"VARTYPE(cT)", "X"},
    {"VARTYPE(cT,.T.)", "T"},
    {"VARTYPE(cT,.F.)", "X"},
    {"TYPE('cT')", "T"},
    {"VARTYPE(cL)", "X"},
    {"VARTYPE(cL,.T.)", "L"},
    {"VARTYPE(cL,.F.)", "X"},
    {"TYPE('cL')", "L"},
    {"VARTYPE(cI)", "X"},
    {"VARTYPE(cI,.T.)", "N"},
    {"VARTYPE(cI,.F.)", "X"},
    {"TYPE('cI')", "N"},
    {"VARTYPE(cY)", "X"},
    {"VARTYPE(cY,.T.)", "Y"},
    {"VARTYPE(cY,.F.)", "X"},
    {"TYPE('cY')", "Y"},
    {"VARTYPE(cB)", "X"},
    {"VARTYPE(cB,.T.)", "N"},
    {"VARTYPE(cB,.F.)", "X"},
    {"TYPE('cB')", "N"},
    {"VARTYPE(cF)", "X"},
    {"VARTYPE(cF,.T.)", "N"},
    {"VARTYPE(cF,.F.)", "X"},
    {"TYPE('cF')", "N"},
    {"VARTYPE(cM)", "X"},
    {"VARTYPE(cM,.T.)", "C"},
    {"VARTYPE(cM,.F.)", "X"},
    {"TYPE('cM')", "M"},
    {"VARTYPE(.NULL.)", "X"},
    {"VARTYPE(.NULL.,.T.)", "X"},
    {"VARTYPE(.NULL.,.F.)", "X"},
    {"VARTYPE(cN,1)", "ERR11"},
    {"VARTYPE(cN,'x')", "ERR11"},
    {"VARTYPE(cN,0)", "ERR11"},
    {"VARTYPE(x)", "X"},
    {"VARTYPE(x,.T.)", "N"},
    {"VARTYPE(y,.T.)", "C"},
    {"VARTYPE(z,.T.)", "D"},
    {"TYPE('x')", "N"}
};

std::string run_rows(const fs::path &dir) {
    std::string body =
        "LOCAL cOut, oEx\ncOut = ''\n"
        "CREATE CURSOR tn (cN N(10,2) NULL, cC C(10) NULL, cD D NULL, cT T NULL, cL L NULL, cI I NULL, cY Y NULL, "
        "cB B(8) NULL, cF F(10,2) NULL, cM M NULL)\n"
        "APPEND BLANK\n"
        "REPLACE cN WITH .NULL., cC WITH .NULL., cD WITH .NULL., cT WITH .NULL., cL WITH .NULL., cI WITH .NULL., "
        "cY WITH .NULL., cB WITH .NULL., cF WITH .NULL., cM WITH .NULL.\n"
        "PUBLIC x, y, z, w\nx = cN\ny = cC\nz = cD\nw = .NULL.\n";
    for (const Row &row : kRows) {
        body += "TRY\n";
        body += "cOut = cOut + " + std::string(row.expression) + " + CHR(10)\n";
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

void test_typed_null_matches_vfp9() {
    const fs::path dir = fs::temp_directory_path() / "copperfin_typed_null";
    std::error_code ignored;
    fs::remove_all(dir, ignored);
    const std::string output = run_rows(dir);
    expect(output.rfind("<incomplete", 0U) != 0U, "typed null: the script should complete: " + output);
    std::vector<std::string> lines;
    for (std::size_t start = 0U; start < output.size();) {
        const std::size_t end = output.find('\n', start);
        lines.push_back(output.substr(start, end == std::string::npos ? std::string::npos : end - start));
        if (end == std::string::npos) {
            break;
        }
        start = end + 1U;
    }
    expect(lines.size() >= kRows.size(), "typed null: one result per row, got " + std::to_string(lines.size()));
    for (std::size_t index = 0U; index < kRows.size() && index < lines.size(); ++index) {
        expect(lines[index] == kRows[index].expected,
            std::string("typed null: ") + kRows[index].expression + " expected [" + kRows[index].expected +
                "], got [" + lines[index] + "]");
    }
    fs::remove_all(dir, ignored);
}

// Aggregates skip NULLs (installed VFP9, result.txt): over 10, 20, NULL, NULL, CALCULATE SUM is 30, CNT() 4, AVG 15,
// MIN 10 and MAX 20; COUNT TO is 4 (records), SUM and AVERAGE skip the NULLs; and SQL COUNT(*) is 4 while
// COUNT(field) is 2 (the non-NULL values), SUM 30, AVG 15, MIN 10, MAX 20. This corrects the assumption in #6506
// that VFP9 does not skip NULLs.
void test_aggregates_skip_nulls() {
    const fs::path dir = fs::temp_directory_path() / "copperfin_null_aggregates";
    std::error_code ignored;
    fs::remove_all(dir, ignored);
    fs::create_directories(dir);
    write_text(dir / "agg.prg",
        "LOCAL c\n"
        "CREATE CURSOR tn (cN N(10,2) NULL, nn N(10,2))\n"
        "APPEND BLANK\nREPLACE cN WITH .NULL., nn WITH 0\n"
        "APPEND BLANK\nREPLACE cN WITH 10, nn WITH 1\n"
        "APPEND BLANK\nREPLACE cN WITH 20, nn WITH 2\n"
        "APPEND BLANK\nREPLACE cN WITH .NULL., nn WITH 3\n"
        "CALCULATE SUM(cN) TO s1\nCALCULATE CNT() TO c1\nCALCULATE AVG(cN) TO a1\nCALCULATE MIN(cN) TO m1\nCALCULATE MAX(cN) TO x1\n"
        "COUNT TO n1\nSUM cN TO s2\nAVERAGE cN TO a2\n"
        "DIMENSION aQ[1,6]\n"
        "SELECT COUNT(*), COUNT(cN), SUM(cN), AVG(cN), MIN(cN), MAX(cN) FROM tn INTO ARRAY aQ\n"
        "c = ALLTRIM(STR(s1)) + ',' + ALLTRIM(STR(c1)) + ',' + ALLTRIM(STR(a1)) + ',' + ALLTRIM(STR(m1)) + ',' + ALLTRIM(STR(x1)) + '|' + "
        "ALLTRIM(STR(n1)) + ',' + ALLTRIM(STR(s2)) + ',' + ALLTRIM(STR(a2)) + '|' + "
        "ALLTRIM(STR(aQ[1,1])) + ',' + ALLTRIM(STR(aQ[1,2])) + ',' + ALLTRIM(STR(aQ[1,3])) + ',' + ALLTRIM(STR(aQ[1,4])) + ',' + "
        "ALLTRIM(STR(aQ[1,5])) + ',' + ALLTRIM(STR(aQ[1,6]))\n"
        "STRTOFILE(c, 'agg.txt')\nRETURN\n");
    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options((dir / "agg.prg").string(), dir.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "null aggregates: the script should complete: " + state.message);
    expect(read_text(dir / "agg.txt") == "30,4,15,10,20|4,30,15|4,2,30,15,10,20",
        "null aggregates: expected 30,4,15,10,20|4,30,15|4,2,30,15,10,20, got [" + read_text(dir / "agg.txt") + "]");
    fs::remove_all(dir, ignored);
}

// COUNT(expr) counts the non-NULL values, blank strings included, and COUNT(DISTINCT expr) each distinct non-NULL
// value once, in plain and grouped queries (installed VFP9, retained at result3.txt). Over cN = 10, 10, 20, NULL
// and cC = '', 'x', NULL, '' with g = a, a, b, b: COUNT(*) 4, COUNT(cN) 3, COUNT(DISTINCT cN) 2, COUNT(cC) 3,
// COUNT(DISTINCT cC) 2; grouped by g: (2, 2, 2) and (2, 1, 1), and COUNT(DISTINCT cN) 1 and 1.
void test_count_counts_non_null_values() {
    const fs::path dir = fs::temp_directory_path() / "copperfin_count_non_null";
    std::error_code ignored;
    fs::remove_all(dir, ignored);
    fs::create_directories(dir);
    write_text(dir / "count.prg",
        "LOCAL c\n"
        "CREATE CURSOR tn (cN N(10,2) NULL, cC C(10) NULL, g C(1))\n"
        "APPEND BLANK\nREPLACE cN WITH 10, cC WITH '', g WITH 'a'\n"
        "APPEND BLANK\nREPLACE cN WITH 10, cC WITH 'x', g WITH 'a'\n"
        "APPEND BLANK\nREPLACE cN WITH 20, cC WITH .NULL., g WITH 'b'\n"
        "APPEND BLANK\nREPLACE cN WITH .NULL., cC WITH '', g WITH 'b'\n"
        "DIMENSION a1[1,4], a2[1,1], a3[2,4], a4[2,2]\n"
        "SELECT COUNT(*), COUNT(cN), COUNT(DISTINCT cN), COUNT(cC) FROM tn INTO ARRAY a1\n"
        "SELECT COUNT(DISTINCT cC) FROM tn INTO ARRAY a2\n"
        "SELECT g, COUNT(*), COUNT(cN), COUNT(cC) FROM tn GROUP BY g INTO ARRAY a3\n"
        "SELECT g, COUNT(DISTINCT cN) FROM tn GROUP BY g INTO ARRAY a4\n"
        "c = ALLTRIM(STR(a1[1,1])) + ',' + ALLTRIM(STR(a1[1,2])) + ',' + ALLTRIM(STR(a1[1,3])) + ',' + ALLTRIM(STR(a1[1,4])) + '|' + "
        "ALLTRIM(STR(a2[1,1])) + '|' + "
        "a3[1,1] + ALLTRIM(STR(a3[1,2])) + ALLTRIM(STR(a3[1,3])) + ALLTRIM(STR(a3[1,4])) + ',' + "
        "a3[2,1] + ALLTRIM(STR(a3[2,2])) + ALLTRIM(STR(a3[2,3])) + ALLTRIM(STR(a3[2,4])) + '|' + "
        "a4[1,1] + ALLTRIM(STR(a4[1,2])) + ',' + a4[2,1] + ALLTRIM(STR(a4[2,2]))\n"
        "STRTOFILE(c, 'count.txt')\nRETURN\n");
    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options((dir / "count.prg").string(), dir.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "COUNT non-null: the script should complete: " + state.message);
    expect(read_text(dir / "count.txt") == "4,3,2,3|2|a222,b211|a1,b1",
        "COUNT non-null: expected 4,3,2,3|2|a222,b211|a1,b1, got [" + read_text(dir / "count.txt") + "]");
    fs::remove_all(dir, ignored);
}

}  // namespace

int main() {
    test_typed_null_matches_vfp9();
    test_aggregates_skip_nulls();
    test_count_counts_non_null_values();
    if (const int failures = test_failures(); failures != 0) {
        std::cerr << failures << " test(s) failed\n";
        return 1;
    }
    return 0;
}
