// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/runtime/prg_engine.h"
#include "prg_engine_test_support.h"

#include <filesystem>
#include <iostream>
#include <locale>
#include <string>
#include <system_error>
#include <vector>

namespace {

using namespace copperfin::test_support;

namespace fs = std::filesystem;

// Governing requirement: RQ-CF-PRG-MDY-DMY-001 (#5924).
//
// Every expectation is installed VFP9 (09.00.0000.7423, Windows VM COM probe 2026-10-01, retained at
// /home/rich/temp/vfp9-probes/datetime-types-c24/result-mdy.txt and ~/temp/vfp9-probes/mdy-dmy-52.out):
// MDY(d) is "April 18, 2026" and DMY(d) is "18 April 2026" for a Date or DateTime (the day is always two digits and
// the year has two digits when SET CENTURY is OFF), an empty value is "*bad date*", NULL is NULL, any other type
// is error 11, no argument is error 1229 and more than one is error 1230 (so the three-number form of the old
// constructor is error 1230). SET DATE and SET MARK have no effect. Each row sets its own SET statements first.
// The expected text is "<VARTYPE>:<value>" or "ERR<number>".

struct Row {
    const char *setup;
    const char *expression;
    const char *expected;
};

const std::vector<Row> kRows = {
    {"SET CENTURY ON", "MDY(DATE(2026,4,18))", "C:April 18, 2026"},
    {"SET CENTURY ON", "MDY(DATETIME(2026,4,18,13,45,30))", "C:April 18, 2026"},
    {"SET CENTURY ON", "MDY({})", "C:*bad date*"},
    {"SET CENTURY ON", "MDY({ : })", "C:*bad date*"},
    {"SET CENTURY ON", "MDY(DATE(2026,1,1))", "C:January 01, 2026"},
    {"SET CENTURY ON", "MDY(DATE(2026,12,31))", "C:December 31, 2026"},
    {"SET CENTURY ON", "MDY(DATE(1999,2,9))", "C:February 09, 1999"},
    {"SET CENTURY ON", "MDY(DATE(2000,3,1))", "C:March 01, 2000"},
    {"SET CENTURY ON", "MDY(DATE(1753,1,1))", "C:January 01, 1753"},
    {"SET CENTURY ON", "MDY(DATE(9999,12,31))", "C:December 31, 9999"},
    {"SET CENTURY ON", "MDY('04/18/2026')", "ERR11"},
    {"SET CENTURY ON", "MDY(5)", "ERR11"},
    {"SET CENTURY ON", "MDY(.T.)", "ERR11"},
    {"SET CENTURY ON", "MDY(oX)", "ERR11"},
    {"SET CENTURY ON", "MDY(.NULL.)", "X:.NULL."},
    {"SET CENTURY ON", "DMY(DATE(2026,4,18))", "C:18 April 2026"},
    {"SET CENTURY ON", "DMY(DATETIME(2026,4,18,13,45,30))", "C:18 April 2026"},
    {"SET CENTURY ON", "DMY({})", "C:*bad date*"},
    {"SET CENTURY ON", "DMY({ : })", "C:*bad date*"},
    {"SET CENTURY ON", "DMY(DATE(2026,1,1))", "C:01 January 2026"},
    {"SET CENTURY ON", "DMY(DATE(2026,12,31))", "C:31 December 2026"},
    {"SET CENTURY ON", "DMY(DATE(1999,2,9))", "C:09 February 1999"},
    {"SET CENTURY ON", "DMY(DATE(2000,3,1))", "C:01 March 2000"},
    {"SET CENTURY ON", "DMY(DATE(1753,1,1))", "C:01 January 1753"},
    {"SET CENTURY ON", "DMY(DATE(9999,12,31))", "C:31 December 9999"},
    {"SET CENTURY ON", "DMY('04/18/2026')", "ERR11"},
    {"SET CENTURY ON", "DMY(5)", "ERR11"},
    {"SET CENTURY ON", "DMY(.T.)", "ERR11"},
    {"SET CENTURY ON", "DMY(oX)", "ERR11"},
    {"SET CENTURY ON", "DMY(.NULL.)", "X:.NULL."},
    {"SET CENTURY OFF", "MDY(DATE(2026,4,18))", "C:April 18, 26"},
    {"SET CENTURY OFF", "MDY(DATETIME(2026,4,18,13,45,30))", "C:April 18, 26"},
    {"SET CENTURY OFF", "MDY({})", "C:*bad date*"},
    {"SET CENTURY OFF", "MDY({ : })", "C:*bad date*"},
    {"SET CENTURY OFF", "MDY(DATE(2026,1,1))", "C:January 01, 26"},
    {"SET CENTURY OFF", "MDY(DATE(2026,12,31))", "C:December 31, 26"},
    {"SET CENTURY OFF", "MDY(DATE(1999,2,9))", "C:February 09, 99"},
    {"SET CENTURY OFF", "MDY(DATE(2000,3,1))", "C:March 01, 00"},
    {"SET CENTURY OFF", "MDY(DATE(1753,1,1))", "C:January 01, 53"},
    {"SET CENTURY OFF", "MDY(DATE(9999,12,31))", "C:December 31, 99"},
    {"SET CENTURY OFF", "MDY('04/18/2026')", "ERR11"},
    {"SET CENTURY OFF", "MDY(5)", "ERR11"},
    {"SET CENTURY OFF", "MDY(.T.)", "ERR11"},
    {"SET CENTURY OFF", "MDY(oX)", "ERR11"},
    {"SET CENTURY OFF", "MDY(.NULL.)", "X:.NULL."},
    {"SET CENTURY OFF", "DMY(DATE(2026,4,18))", "C:18 April 26"},
    {"SET CENTURY OFF", "DMY(DATETIME(2026,4,18,13,45,30))", "C:18 April 26"},
    {"SET CENTURY OFF", "DMY({})", "C:*bad date*"},
    {"SET CENTURY OFF", "DMY({ : })", "C:*bad date*"},
    {"SET CENTURY OFF", "DMY(DATE(2026,1,1))", "C:01 January 26"},
    {"SET CENTURY OFF", "DMY(DATE(2026,12,31))", "C:31 December 26"},
    {"SET CENTURY OFF", "DMY(DATE(1999,2,9))", "C:09 February 99"},
    {"SET CENTURY OFF", "DMY(DATE(2000,3,1))", "C:01 March 00"},
    {"SET CENTURY OFF", "DMY(DATE(1753,1,1))", "C:01 January 53"},
    {"SET CENTURY OFF", "DMY(DATE(9999,12,31))", "C:31 December 99"},
    {"SET CENTURY OFF", "DMY('04/18/2026')", "ERR11"},
    {"SET CENTURY OFF", "DMY(5)", "ERR11"},
    {"SET CENTURY OFF", "DMY(.T.)", "ERR11"},
    {"SET CENTURY OFF", "DMY(oX)", "ERR11"},
    {"SET CENTURY OFF", "DMY(.NULL.)", "X:.NULL."},
    {"SET CENTURY ON", "MDY()", "ERR1229"},
    {"SET CENTURY ON", "MDY(DATE(2026,4,18),1)", "ERR1230"},
    {"SET CENTURY ON", "MDY(4,18,2026)", "ERR1230"},
    {"SET CENTURY ON", "DMY(18,4,2026)", "ERR1230"},
    {"SET CENTURY ON", "DMY()", "ERR1229"},
    {"SET CENTURY ON", "DMY(DATE(2026,4,18),1)", "ERR1230"},
    {"SET CENTURY ON", "MDY(DATE(2026,4,18),DATE(2026,4,18))", "ERR1230"},
    {"SET CENTURY OFF", "MDY(DATE(2026,1,5))", "C:January 05, 26"},
    {"SET CENTURY OFF", "MDY(DATE(2026,2,5))", "C:February 05, 26"},
    {"SET CENTURY OFF", "MDY(DATE(2026,3,5))", "C:March 05, 26"},
    {"SET CENTURY OFF", "MDY(DATE(2026,4,5))", "C:April 05, 26"},
    {"SET CENTURY OFF", "MDY(DATE(2026,5,5))", "C:May 05, 26"},
    {"SET CENTURY OFF", "MDY(DATE(2026,6,5))", "C:June 05, 26"},
    {"SET CENTURY OFF", "MDY(DATE(2026,7,5))", "C:July 05, 26"},
    {"SET CENTURY OFF", "MDY(DATE(2026,8,5))", "C:August 05, 26"},
    {"SET CENTURY OFF", "MDY(DATE(2026,9,5))", "C:September 05, 26"},
    {"SET CENTURY OFF", "MDY(DATE(2026,10,5))", "C:October 05, 26"},
    {"SET CENTURY OFF", "MDY(DATE(2026,11,5))", "C:November 05, 26"},
    {"SET CENTURY OFF", "MDY(DATE(2026,12,5))", "C:December 05, 26"},
    {"SET CENTURY OFF", "DMY(DATE(2026,1,5))", "C:05 January 26"},
    {"SET CENTURY OFF", "DMY(DATE(2026,2,5))", "C:05 February 26"},
    {"SET CENTURY OFF", "DMY(DATE(2026,3,5))", "C:05 March 26"},
    {"SET CENTURY OFF", "DMY(DATE(2026,4,5))", "C:05 April 26"},
    {"SET CENTURY OFF", "DMY(DATE(2026,5,5))", "C:05 May 26"},
    {"SET CENTURY OFF", "DMY(DATE(2026,6,5))", "C:05 June 26"},
    {"SET CENTURY OFF", "DMY(DATE(2026,7,5))", "C:05 July 26"},
    {"SET CENTURY OFF", "DMY(DATE(2026,8,5))", "C:05 August 26"},
    {"SET CENTURY OFF", "DMY(DATE(2026,9,5))", "C:05 September 26"},
    {"SET CENTURY OFF", "DMY(DATE(2026,10,5))", "C:05 October 26"},
    {"SET CENTURY OFF", "DMY(DATE(2026,11,5))", "C:05 November 26"},
    {"SET CENTURY OFF", "DMY(DATE(2026,12,5))", "C:05 December 26"},
    {"SET CENTURY ON\nSET DATE TO DMY", "MDY(DATE(2026,4,18))", "C:April 18, 2026"},
    {"SET CENTURY ON\nSET DATE TO DMY", "DMY(DATE(2026,4,18))", "C:18 April 2026"},
    {"SET CENTURY ON\nSET DATE TO YMD", "MDY(DATE(2026,4,18))", "C:April 18, 2026"},
    {"SET CENTURY ON\nSET DATE TO YMD", "DMY(DATE(2026,4,18))", "C:18 April 2026"},
    {"SET CENTURY ON\nSET MARK TO '-'", "MDY(DATE(2026,4,18))", "C:April 18, 2026"},
    {"SET CENTURY ON\nSET MARK TO '-'", "DMY(DATE(2026,4,18))", "C:18 April 2026"}
};

std::string run_rows(const fs::path &dir) {
    std::string body = "LOCAL cOut, oEx, x\ncOut = ''\nPUBLIC oX\noX = CREATEOBJECT('Custom')\n";
    for (const Row &row : kRows) {
        body += "SET DATE TO MDY\nSET MARK TO '/'\n";
        body += std::string(row.setup) + "\n";
        body += "TRY\n";
        body += "x = " + std::string(row.expression) + "\n";
        body += "cOut = cOut + VARTYPE(x) + ':' + IIF(ISNULL(x), '.NULL.', x) + CHR(10)\n";
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

// A global locale that groups digits (2,026): MDY and DMY must still print the year as 2026.
struct GroupingNumpunct : std::numpunct<char> {
    char do_thousands_sep() const override { return ','; }
    std::string do_grouping() const override { return "\3"; }
};

void test_mdy_dmy_match_vfp9() {
    const fs::path dir = fs::temp_directory_path() / "copperfin_mdy_dmy";
    std::error_code ignored;
    fs::remove_all(dir, ignored);
    const std::locale previous_locale = std::locale::global(std::locale(std::locale::classic(), new GroupingNumpunct));
    const std::string output = run_rows(dir);
    std::locale::global(previous_locale);
    expect(output.rfind("<incomplete", 0U) != 0U, "MDY/DMY: the script should complete: " + output);
    std::vector<std::string> lines;
    for (std::size_t start = 0U; start < output.size();) {
        const std::size_t end = output.find('\n', start);
        lines.push_back(output.substr(start, end == std::string::npos ? std::string::npos : end - start));
        if (end == std::string::npos) {
            break;
        }
        start = end + 1U;
    }
    expect(lines.size() >= kRows.size(), "MDY/DMY: one result per row, got " + std::to_string(lines.size()));
    for (std::size_t index = 0U; index < kRows.size() && index < lines.size(); ++index) {
        expect(lines[index] == kRows[index].expected,
            std::string("MDY/DMY: [") + kRows[index].setup + "] " + kRows[index].expression + " expected [" +
                kRows[index].expected + "], got [" + lines[index] + "]");
    }
    fs::remove_all(dir, ignored);
}

}  // namespace

int main() {
    test_mdy_dmy_match_vfp9();
    if (const int failures = test_failures(); failures != 0) {
        std::cerr << failures << " test(s) failed\n";
        return 1;
    }
    return 0;
}
