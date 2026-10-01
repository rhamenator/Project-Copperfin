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

// Governing requirement: RQ-CF-PRG-DATE-FUNCTION-ARGUMENT-TYPES-001 (#6142).
//
// Every expectation is installed VFP9 (09.00.0000.7423, Windows VM COM probe 2026-10-01, retained at
// /home/rich/temp/vfp9-probes/datetime-types-c24/result.txt): the date and time functions take a Date or a
// DateTime and raise error 11 for a Character, Numeric, Logical or Object argument (DTOT takes only a Date and
// TTOD only a DateTime; GOMONTH and DOW need a Numeric second argument). A NULL argument returns NULL. A Date or
// DateTime result is compared by type only (the display of a datetime differs, see #5998), and so is TTOC. The
// MDY and DMY are not covered here (their numeric constructor and missing text form are #5924), and numbers are
// formatted with STR because a pictureless TRANSFORM groups digits (#6731). The expected text is "<VARTYPE>:<TRANSFORM>", "<VARTYPE>:" for a type-only result, or "ERR<number>".

struct Row {
    const char *expression;
    const char *expected;
};

const std::vector<Row> kRows = {
    {"DOW(DATE(2026,6,15))", "N:2"},
    {"DOW(DATETIME(2026,6,15,13,45,30))", "N:2"},
    {"DOW('06/15/2026')", "ERR11"},
    {"DOW('13:45:30')", "ERR11"},
    {"DOW(5)", "ERR11"},
    {"DOW(.T.)", "ERR11"},
    {"DOW(oX)", "ERR11"},
    {"DOW(.NULL.)", "X:.NULL."},
    {"CDOW(DATE(2026,6,15))", "C:Monday"},
    {"CDOW(DATETIME(2026,6,15,13,45,30))", "C:Monday"},
    {"CDOW('06/15/2026')", "ERR11"},
    {"CDOW('13:45:30')", "ERR11"},
    {"CDOW(5)", "ERR11"},
    {"CDOW(.T.)", "ERR11"},
    {"CDOW(oX)", "ERR11"},
    {"CDOW(.NULL.)", "X:.NULL."},
    {"CMONTH(DATE(2026,6,15))", "C:June"},
    {"CMONTH(DATETIME(2026,6,15,13,45,30))", "C:June"},
    {"CMONTH('06/15/2026')", "ERR11"},
    {"CMONTH('13:45:30')", "ERR11"},
    {"CMONTH(5)", "ERR11"},
    {"CMONTH(.T.)", "ERR11"},
    {"CMONTH(oX)", "ERR11"},
    {"CMONTH(.NULL.)", "X:.NULL."},
    {"YEAR(DATE(2026,6,15))", "N:2026"},
    {"YEAR(DATETIME(2026,6,15,13,45,30))", "N:2026"},
    {"YEAR('06/15/2026')", "ERR11"},
    {"YEAR('13:45:30')", "ERR11"},
    {"YEAR(5)", "ERR11"},
    {"YEAR(.T.)", "ERR11"},
    {"YEAR(oX)", "ERR11"},
    {"YEAR(.NULL.)", "X:.NULL."},
    {"MONTH(DATE(2026,6,15))", "N:6"},
    {"MONTH(DATETIME(2026,6,15,13,45,30))", "N:6"},
    {"MONTH('06/15/2026')", "ERR11"},
    {"MONTH('13:45:30')", "ERR11"},
    {"MONTH(5)", "ERR11"},
    {"MONTH(.T.)", "ERR11"},
    {"MONTH(oX)", "ERR11"},
    {"MONTH(.NULL.)", "X:.NULL."},
    {"DAY(DATE(2026,6,15))", "N:15"},
    {"DAY(DATETIME(2026,6,15,13,45,30))", "N:15"},
    {"DAY('06/15/2026')", "ERR11"},
    {"DAY('13:45:30')", "ERR11"},
    {"DAY(5)", "ERR11"},
    {"DAY(.T.)", "ERR11"},
    {"DAY(oX)", "ERR11"},
    {"DAY(.NULL.)", "X:.NULL."},
    {"WEEK(DATE(2026,6,15))", "N:25"},
    {"WEEK(DATETIME(2026,6,15,13,45,30))", "N:25"},
    {"WEEK('06/15/2026')", "ERR11"},
    {"WEEK('13:45:30')", "ERR11"},
    {"WEEK(5)", "ERR11"},
    {"WEEK(.T.)", "ERR11"},
    {"WEEK(oX)", "ERR11"},
    {"WEEK(.NULL.)", "X:.NULL."},
    {"DTOS(DATE(2026,6,15))", "C:20260615"},
    {"DTOS(DATETIME(2026,6,15,13,45,30))", "C:20260615"},
    {"DTOS('06/15/2026')", "ERR11"},
    {"DTOS('13:45:30')", "ERR11"},
    {"DTOS(5)", "ERR11"},
    {"DTOS(.T.)", "ERR11"},
    {"DTOS(oX)", "ERR11"},
    {"DTOS(.NULL.)", "X:.NULL."},
    {"DTOC(DATE(2026,6,15))", "C:06/15/2026"},
    {"DTOC(DATETIME(2026,6,15,13,45,30))", "C:06/15/2026"},
    {"DTOC('06/15/2026')", "ERR11"},
    {"DTOC('13:45:30')", "ERR11"},
    {"DTOC(5)", "ERR11"},
    {"DTOC(.T.)", "ERR11"},
    {"DTOC(oX)", "ERR11"},
    {"DTOC(.NULL.)", "X:.NULL."},
    {"DTOT(DATE(2026,6,15))", "T:"},
    {"DTOT(DATETIME(2026,6,15,13,45,30))", "ERR11"},
    {"DTOT('06/15/2026')", "ERR11"},
    {"DTOT('13:45:30')", "ERR11"},
    {"DTOT(5)", "ERR11"},
    {"DTOT(.T.)", "ERR11"},
    {"DTOT(oX)", "ERR11"},
    {"DTOT(.NULL.)", "X:.NULL."},
    {"TTOD(DATE(2026,6,15))", "ERR11"},
    {"TTOD(DATETIME(2026,6,15,13,45,30))", "D:"},
    {"TTOD('06/15/2026')", "ERR11"},
    {"TTOD('13:45:30')", "ERR11"},
    {"TTOD(5)", "ERR11"},
    {"TTOD(.T.)", "ERR11"},
    {"TTOD(oX)", "ERR11"},
    {"TTOD(.NULL.)", "X:.NULL."},
    {"HOUR(DATE(2026,6,15))", "N:0"},
    {"HOUR(DATETIME(2026,6,15,13,45,30))", "N:13"},
    {"HOUR('06/15/2026')", "ERR11"},
    {"HOUR('13:45:30')", "ERR11"},
    {"HOUR(5)", "ERR11"},
    {"HOUR(.T.)", "ERR11"},
    {"HOUR(oX)", "ERR11"},
    {"HOUR(.NULL.)", "X:.NULL."},
    {"MINUTE(DATE(2026,6,15))", "N:0"},
    {"MINUTE(DATETIME(2026,6,15,13,45,30))", "N:45"},
    {"MINUTE('06/15/2026')", "ERR11"},
    {"MINUTE('13:45:30')", "ERR11"},
    {"MINUTE(5)", "ERR11"},
    {"MINUTE(.T.)", "ERR11"},
    {"MINUTE(oX)", "ERR11"},
    {"MINUTE(.NULL.)", "X:.NULL."},
    {"SEC(DATE(2026,6,15))", "N:0"},
    {"SEC(DATETIME(2026,6,15,13,45,30))", "N:30"},
    {"SEC('06/15/2026')", "ERR11"},
    {"SEC('13:45:30')", "ERR11"},
    {"SEC(5)", "ERR11"},
    {"SEC(.T.)", "ERR11"},
    {"SEC(oX)", "ERR11"},
    {"SEC(.NULL.)", "X:.NULL."},
    {"TTOC(DATE(2026,6,15))", "C:"},
    {"TTOC(DATETIME(2026,6,15,13,45,30))", "C:"},
    {"TTOC('06/15/2026')", "ERR11"},
    {"TTOC('13:45:30')", "ERR11"},
    {"TTOC(5)", "ERR11"},
    {"TTOC(.T.)", "ERR11"},
    {"TTOC(oX)", "ERR11"},
    {"TTOC(.NULL.)", "X:.NULL."},
    {"GOMONTH(DATE(2026,6,15),1)", "D:"},
    {"GOMONTH(DATETIME(2026,6,15,13,45,30),1)", "D:"},
    {"GOMONTH('06/15/2026',1)", "ERR11"},
    {"GOMONTH('13:45:30',1)", "ERR11"},
    {"GOMONTH(5,1)", "ERR11"},
    {"GOMONTH(.T.,1)", "ERR11"},
    {"GOMONTH(oX,1)", "ERR11"},
    {"GOMONTH(.NULL.,1)", "X:.NULL."},
    {"GOMONTH(DATE(2026,6,15),DATE(2026,6,15))", "ERR11"},
    {"GOMONTH(DATE(2026,6,15),DATETIME(2026,6,15,13,45,30))", "ERR11"},
    {"GOMONTH(DATE(2026,6,15),'06/15/2026')", "ERR11"},
    {"GOMONTH(DATE(2026,6,15),'13:45:30')", "ERR11"},
    {"GOMONTH(DATE(2026,6,15),5)", "D:"},
    {"GOMONTH(DATE(2026,6,15),.T.)", "ERR11"},
    {"GOMONTH(DATE(2026,6,15),oX)", "ERR11"},
    {"GOMONTH(DATE(2026,6,15),.NULL.)", "X:.NULL."},
    {"DOW(DATE(2026,6,15),DATE(2026,6,15))", "ERR11"},
    {"DOW(DATE(2026,6,15),DATETIME(2026,6,15,13,45,30))", "ERR11"},
    {"DOW(DATE(2026,6,15),'06/15/2026')", "ERR11"},
    {"DOW(DATE(2026,6,15),'13:45:30')", "ERR11"},
    {"DOW(DATE(2026,6,15),5)", "N:5"},
    {"DOW(DATE(2026,6,15),.T.)", "ERR11"},
    {"DOW(DATE(2026,6,15),oX)", "ERR11"}
};

std::string run_rows(const fs::path &dir) {
    std::string body = "LOCAL cOut, oEx, x\ncOut = ''\nPUBLIC oX\noX = CREATEOBJECT('Custom')\n";
    for (const Row &row : kRows) {
        body += "TRY\n";
        body += "x = " + std::string(row.expression) + "\n";
        body += "cOut = cOut + VARTYPE(x) + ':' + IIF(ISNULL(x), '.NULL.', IIF(INLIST(VARTYPE(x), 'D', 'T') OR '" + std::string(row.expression).substr(0, 5) + "' = 'TTOC(', '', IIF(VARTYPE(x) = 'N', ALLTRIM(STR(x)), ALLTRIM(TRANSFORM(x))))) + CHR(10)\n";
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

void test_date_function_argument_types_match_vfp9() {
    const fs::path dir = fs::temp_directory_path() / "copperfin_date_function_argument_types";
    std::error_code ignored;
    fs::remove_all(dir, ignored);
    const std::string output = run_rows(dir);
    expect(output.rfind("<incomplete", 0U) != 0U, "date function types: the script should complete: " + output);
    std::vector<std::string> lines;
    for (std::size_t start = 0U; start < output.size();) {
        const std::size_t end = output.find('\n', start);
        lines.push_back(output.substr(start, end == std::string::npos ? std::string::npos : end - start));
        if (end == std::string::npos) {
            break;
        }
        start = end + 1U;
    }
    expect(lines.size() >= kRows.size(), "date function types: one result per row, got " + std::to_string(lines.size()));
    for (std::size_t index = 0U; index < kRows.size() && index < lines.size(); ++index) {
        expect(lines[index] == kRows[index].expected,
            std::string("date function types: ") + kRows[index].expression + " expected [" + kRows[index].expected +
                "], got [" + lines[index] + "]");
    }
    fs::remove_all(dir, ignored);
}

}  // namespace

int main() {
    test_date_function_argument_types_match_vfp9();
    if (const int failures = test_failures(); failures != 0) {
        std::cerr << failures << " test(s) failed\n";
        return 1;
    }
    return 0;
}
