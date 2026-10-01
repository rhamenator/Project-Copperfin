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

// Governing requirement: RQ-CF-PRG-DATE-LITERALS-001 (#5920).
//
// Every expectation is installed VFP9 (09.00.0000.7423, Windows VM COM probe 2026-10-01, retained at
// /home/rich/temp/vfp9-probes/datetime-types-c24/result-literals.txt): a strict {^yyyy-mm-dd[,][h:m[:s] [a|p]]}
// literal is a Date or a DateTime, {} and its blank forms are empty Date or DateTime values, an out-of-range
// value is error 2034 and a ^ literal that does not match the grammar is error 2032. The expected text is
// "<VARTYPE>:<DTOS or TTOC(x,1)>:<EMPTY>" (the text is blank for an empty value) or "ERR<number>". The text is compared in its sortable form so the test does not depend on the display format (SET HOURS, and the empty-date display of #5998). The non-strict form {06/15/2026} (error 2032 under
// VFP9's default STRICTDATE 1) stays a Character value here and is not covered.

struct Row {
    const char *expression;
    const char *expected;
};

const std::vector<Row> kRows = {
    {"{^2026-06-15}", "D:20260615:F"},
    {"{^2026/06/15}", "D:20260615:F"},
    {"{^2026.06.15}", "D:20260615:F"},
    {"{^2026-6-5}", "D:20260605:F"},
    {"{^2026-06-15 13:45:30}", "T:20260615134530:F"},
    {"{^2026-06-15 13:45}", "T:20260615134500:F"},
    {"{^2026-06-15 1:02:03 PM}", "T:20260615130203:F"},
    {"{^2026-06-15 1:02 AM}", "T:20260615010200:F"},
    {"{^2026-06-15 12:00:00 AM}", "T:20260615000000:F"},
    {"{^2026-06-15 12:00:00 PM}", "T:20260615120000:F"},
    {"{^2026-06-15 13}", "ERR2032"},
    {"{}", "D::T"},
    {"{  /  /  }", "D::T"},
    {"{//}", "D::T"},
    {"{^ :}", "ERR2032"},
    {"{ : }", "T::T"},
    {"{/ /: }", "T::T"},
    {"{^2026-02-30}", "ERR2034"},
    {"{^2026-13-01}", "ERR2034"},
    {"{^2026-00-10}", "ERR2034"},
    {"{^2026-06-15 25:00:00}", "ERR2034"},
    {"{^2026-06-15 13:61:00}", "ERR2034"},
    {"{^2026-06-15 13:45:61}", "ERR2034"},
    {"{^0001-01-01}", "D:00010101:F"},
    {"{^1753-01-01}", "D:17530101:F"},
    {"{^1752-12-31}", "D:17521231:F"},
    {"{^9999-12-31}", "D:99991231:F"},
    {"{^2026-06-15 13:45:30.500}", "T:20260615134530:F"},
    {"{^2026-02-29}", "ERR2034"},
    {"{^2024-02-29}", "D:20240229:F"},
    {"{^26-06-15}", "ERR2032"},
    {"{^2026-06-15 13:45:30 PM}", "ERR2034"},
    {"{^2026-06-15}+1", "D:20260616:F"},
    {"{^2026-06-15 13:45:30}+1", "T:20260615134531:F"}
};

std::string run_rows(const fs::path &dir) {
    std::string body = "LOCAL cOut, oEx, x\ncOut = ''\n";
    for (const Row &row : kRows) {
        body += "TRY\n";
        body += "x = " + std::string(row.expression) + "\n";
        body += "cOut = cOut + VARTYPE(x) + ':' + IIF(EMPTY(x), '', IIF(VARTYPE(x) = 'D', DTOS(x), TTOC(x, 1))) + ':' + IIF(EMPTY(x), 'T', 'F') + CHR(10)\n";
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

void test_braced_date_literals_match_vfp9() {
    const fs::path dir = fs::temp_directory_path() / "copperfin_date_literals";
    std::error_code ignored;
    fs::remove_all(dir, ignored);
    const std::string output = run_rows(dir);
    expect(output.rfind("<incomplete", 0U) != 0U, "date literals: the script should complete: " + output);
    std::vector<std::string> lines;
    for (std::size_t start = 0U; start < output.size();) {
        const std::size_t end = output.find('\n', start);
        lines.push_back(output.substr(start, end == std::string::npos ? std::string::npos : end - start));
        if (end == std::string::npos) {
            break;
        }
        start = end + 1U;
    }
    expect(lines.size() >= kRows.size(), "date literals: one result per row, got " + std::to_string(lines.size()));
    for (std::size_t index = 0U; index < kRows.size() && index < lines.size(); ++index) {
        expect(lines[index] == kRows[index].expected,
            std::string("date literals: ") + kRows[index].expression + " expected [" + kRows[index].expected +
                "], got [" + lines[index] + "]");
    }
    fs::remove_all(dir, ignored);
}

// #5920: the component accessors read a literal's Date and DateTime parts (they returned 0 for a Character
// literal). WEEK({^2021-01-01}) is 1 and DOW({^2021-01-02}) is 7 in VFP9.
void test_component_accessors_read_literals() {
    const fs::path dir = fs::temp_directory_path() / "copperfin_date_literal_parts";
    std::error_code ignored;
    fs::remove_all(dir, ignored);
    fs::create_directories(dir);
    write_text(dir / "parts.prg",
        "LOCAL c\n"
        "c = ALLTRIM(STR(YEAR({^2021-01-02}))) + '|' + ALLTRIM(STR(MONTH({^2021-01-02}))) + '|' + "
        "ALLTRIM(STR(DAY({^2021-01-02}))) + '|' + ALLTRIM(STR(DOW({^2021-01-02}))) + '|' + "
        "ALLTRIM(STR(HOUR({^2021-01-02 12:34:56}))) + '|' + ALLTRIM(STR(MINUTE({^2021-01-02 12:34:56}))) + '|' + "
        "ALLTRIM(STR(SEC({^2021-01-02 12:34:56}))) + '|' + ALLTRIM(STR(WEEK({^2021-01-01})))\n"
        "STRTOFILE(c, 'parts.txt')\nRETURN\n");
    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options((dir / "parts.prg").string(), dir.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "date literal parts: the script should complete: " + state.message);
    expect(read_text(dir / "parts.txt") == "2021|1|2|7|12|34|56|1",
        "date literal parts: expected 2021|1|2|7|12|34|56|1, got [" + read_text(dir / "parts.txt") + "]");
    fs::remove_all(dir, ignored);
}

}  // namespace

int main() {
    test_braced_date_literals_match_vfp9();
    test_component_accessors_read_literals();
    if (const int failures = test_failures(); failures != 0) {
        std::cerr << failures << " test(s) failed\n";
        return 1;
    }
    return 0;
}
