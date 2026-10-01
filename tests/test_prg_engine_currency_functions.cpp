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

// Governing requirement: RQ-CF-PRG-CURRENCY-FUNCTIONS-001 (#6039).
//
// Every expectation is installed VFP9 (09.00.0000.7423, Windows VM COM probe 2026-10-01, retained at
// /home/rich/temp/vfp9-probes/currency-c25/result3.txt): ABS, INT, CEILING, FLOOR, ROUND and MOD keep a Currency
// argument as Currency in exact four-decimal arithmetic. For Currency CEILING rounds away from zero and FLOOR and INT
// truncate toward zero (CEILING($-12.3456) is $-13, the opposite of Numeric); ROUND is half away from zero at the
// requested place (negative places round to tens and hundreds); MOD takes the sign of the divisor, stays Currency
// only when its first argument is, and is error 1307 for a zero divisor; MAX and MIN are Currency only when every
// argument is. SQRT, EXP, LOG, SIGN and the other functions return a Numeric. The expected text is
// "<VARTYPE>:<LTRIM(STR(x,22,4))>" or "ERR<number>".

struct Row {
    const char *expression;
    const char *expected;
};

const std::vector<Row> kRows = {
    {"ABS($12.3456)", "Y:12.3456"},
    {"ABS($-12.3456)", "Y:12.3456"},
    {"ABS($0.5)", "Y:0.5000"},
    {"ABS($-0.5)", "Y:0.5000"},
    {"ABS($12)", "Y:12.0000"},
    {"ABS($-12)", "Y:12.0000"},
    {"ABS($0)", "Y:0.0000"},
    {"ABS($0.0001)", "Y:0.0001"},
    {"ABS($-0.0001)", "Y:0.0001"},
    {"ABS($1.5)", "Y:1.5000"},
    {"ABS($-1.5)", "Y:1.5000"},
    {"ABS($2.5)", "Y:2.5000"},
    {"ABS($-2.5)", "Y:2.5000"},
    {"ABS($0.9999)", "Y:0.9999"},
    {"ABS($-0.9999)", "Y:0.9999"},
    {"ABS($99999.9999)", "Y:99999.9999"},
    {"ABS($-99999.9999)", "Y:99999.9999"},
    {"INT($12.3456)", "Y:12.0000"},
    {"INT($-12.3456)", "Y:-12.0000"},
    {"INT($0.5)", "Y:0.0000"},
    {"INT($-0.5)", "Y:0.0000"},
    {"INT($12)", "Y:12.0000"},
    {"INT($-12)", "Y:-12.0000"},
    {"INT($0)", "Y:0.0000"},
    {"INT($0.0001)", "Y:0.0000"},
    {"INT($-0.0001)", "Y:0.0000"},
    {"INT($1.5)", "Y:1.0000"},
    {"INT($-1.5)", "Y:-1.0000"},
    {"INT($2.5)", "Y:2.0000"},
    {"INT($-2.5)", "Y:-2.0000"},
    {"INT($0.9999)", "Y:0.0000"},
    {"INT($-0.9999)", "Y:0.0000"},
    {"INT($99999.9999)", "Y:99999.0000"},
    {"INT($-99999.9999)", "Y:-99999.0000"},
    {"CEILING($12.3456)", "Y:13.0000"},
    {"CEILING($-12.3456)", "Y:-13.0000"},
    {"CEILING($0.5)", "Y:1.0000"},
    {"CEILING($-0.5)", "Y:-1.0000"},
    {"CEILING($12)", "Y:12.0000"},
    {"CEILING($-12)", "Y:-12.0000"},
    {"CEILING($0)", "Y:0.0000"},
    {"CEILING($0.0001)", "Y:1.0000"},
    {"CEILING($-0.0001)", "Y:-1.0000"},
    {"CEILING($1.5)", "Y:2.0000"},
    {"CEILING($-1.5)", "Y:-2.0000"},
    {"CEILING($2.5)", "Y:3.0000"},
    {"CEILING($-2.5)", "Y:-3.0000"},
    {"CEILING($0.9999)", "Y:1.0000"},
    {"CEILING($-0.9999)", "Y:-1.0000"},
    {"CEILING($99999.9999)", "Y:100000.0000"},
    {"CEILING($-99999.9999)", "Y:-100000.0000"},
    {"FLOOR($12.3456)", "Y:12.0000"},
    {"FLOOR($-12.3456)", "Y:-12.0000"},
    {"FLOOR($0.5)", "Y:0.0000"},
    {"FLOOR($-0.5)", "Y:0.0000"},
    {"FLOOR($12)", "Y:12.0000"},
    {"FLOOR($-12)", "Y:-12.0000"},
    {"FLOOR($0)", "Y:0.0000"},
    {"FLOOR($0.0001)", "Y:0.0000"},
    {"FLOOR($-0.0001)", "Y:0.0000"},
    {"FLOOR($1.5)", "Y:1.0000"},
    {"FLOOR($-1.5)", "Y:-1.0000"},
    {"FLOOR($2.5)", "Y:2.0000"},
    {"FLOOR($-2.5)", "Y:-2.0000"},
    {"FLOOR($0.9999)", "Y:0.0000"},
    {"FLOOR($-0.9999)", "Y:0.0000"},
    {"FLOOR($99999.9999)", "Y:99999.0000"},
    {"FLOOR($-99999.9999)", "Y:-99999.0000"},
    {"SIGN($12.3456)", "N:1.0000"},
    {"SIGN($-12.3456)", "N:-1.0000"},
    {"SIGN($0.5)", "N:1.0000"},
    {"SIGN($-0.5)", "N:-1.0000"},
    {"SIGN($12)", "N:1.0000"},
    {"SIGN($-12)", "N:-1.0000"},
    {"SIGN($0)", "N:0.0000"},
    {"SIGN($0.0001)", "N:1.0000"},
    {"SIGN($-0.0001)", "N:-1.0000"},
    {"SIGN($1.5)", "N:1.0000"},
    {"SIGN($-1.5)", "N:-1.0000"},
    {"SIGN($2.5)", "N:1.0000"},
    {"SIGN($-2.5)", "N:-1.0000"},
    {"SIGN($0.9999)", "N:1.0000"},
    {"SIGN($-0.9999)", "N:-1.0000"},
    {"SIGN($99999.9999)", "N:1.0000"},
    {"SIGN($-99999.9999)", "N:-1.0000"},
    {"ROUND($12.3456,0)", "Y:12.0000"},
    {"ROUND($-12.3456,0)", "Y:-12.0000"},
    {"ROUND($1.5,0)", "Y:2.0000"},
    {"ROUND($-1.5,0)", "Y:-2.0000"},
    {"ROUND($2.5,0)", "Y:3.0000"},
    {"ROUND($-2.5,0)", "Y:-3.0000"},
    {"ROUND($0.00005,0)", "Y:0.0000"},
    {"ROUND($-0.00005,0)", "Y:0.0000"},
    {"ROUND($12.34565,0)", "Y:12.0000"},
    {"ROUND($1234.5678,0)", "Y:1235.0000"},
    {"ROUND($12.3456,1)", "Y:12.3000"},
    {"ROUND($-12.3456,1)", "Y:-12.3000"},
    {"ROUND($1.5,1)", "Y:1.5000"},
    {"ROUND($-1.5,1)", "Y:-1.5000"},
    {"ROUND($2.5,1)", "Y:2.5000"},
    {"ROUND($-2.5,1)", "Y:-2.5000"},
    {"ROUND($0.00005,1)", "Y:0.0000"},
    {"ROUND($-0.00005,1)", "Y:0.0000"},
    {"ROUND($12.34565,1)", "Y:12.3000"},
    {"ROUND($1234.5678,1)", "Y:1234.6000"},
    {"ROUND($12.3456,2)", "Y:12.3500"},
    {"ROUND($-12.3456,2)", "Y:-12.3500"},
    {"ROUND($1.5,2)", "Y:1.5000"},
    {"ROUND($-1.5,2)", "Y:-1.5000"},
    {"ROUND($2.5,2)", "Y:2.5000"},
    {"ROUND($-2.5,2)", "Y:-2.5000"},
    {"ROUND($0.00005,2)", "Y:0.0000"},
    {"ROUND($-0.00005,2)", "Y:0.0000"},
    {"ROUND($12.34565,2)", "Y:12.3500"},
    {"ROUND($1234.5678,2)", "Y:1234.5700"},
    {"ROUND($12.3456,3)", "Y:12.3460"},
    {"ROUND($-12.3456,3)", "Y:-12.3460"},
    {"ROUND($1.5,3)", "Y:1.5000"},
    {"ROUND($-1.5,3)", "Y:-1.5000"},
    {"ROUND($2.5,3)", "Y:2.5000"},
    {"ROUND($-2.5,3)", "Y:-2.5000"},
    {"ROUND($0.00005,3)", "Y:0.0000"},
    {"ROUND($-0.00005,3)", "Y:0.0000"},
    {"ROUND($12.34565,3)", "Y:12.3460"},
    {"ROUND($1234.5678,3)", "Y:1234.5680"},
    {"ROUND($12.3456,4)", "Y:12.3456"},
    {"ROUND($-12.3456,4)", "Y:-12.3456"},
    {"ROUND($1.5,4)", "Y:1.5000"},
    {"ROUND($-1.5,4)", "Y:-1.5000"},
    {"ROUND($2.5,4)", "Y:2.5000"},
    {"ROUND($-2.5,4)", "Y:-2.5000"},
    {"ROUND($0.00005,4)", "Y:0.0001"},
    {"ROUND($-0.00005,4)", "Y:-0.0001"},
    {"ROUND($12.34565,4)", "Y:12.3457"},
    {"ROUND($1234.5678,4)", "Y:1234.5678"},
    {"ROUND($12.3456,5)", "Y:12.3456"},
    {"ROUND($-12.3456,5)", "Y:-12.3456"},
    {"ROUND($1.5,5)", "Y:1.5000"},
    {"ROUND($-1.5,5)", "Y:-1.5000"},
    {"ROUND($2.5,5)", "Y:2.5000"},
    {"ROUND($-2.5,5)", "Y:-2.5000"},
    {"ROUND($0.00005,5)", "Y:0.0001"},
    {"ROUND($-0.00005,5)", "Y:-0.0001"},
    {"ROUND($12.34565,5)", "Y:12.3457"},
    {"ROUND($1234.5678,5)", "Y:1234.5678"},
    {"ROUND($12.3456,-1)", "Y:10.0000"},
    {"ROUND($-12.3456,-1)", "Y:-10.0000"},
    {"ROUND($1.5,-1)", "Y:0.0000"},
    {"ROUND($-1.5,-1)", "Y:0.0000"},
    {"ROUND($2.5,-1)", "Y:0.0000"},
    {"ROUND($-2.5,-1)", "Y:0.0000"},
    {"ROUND($0.00005,-1)", "Y:0.0000"},
    {"ROUND($-0.00005,-1)", "Y:0.0000"},
    {"ROUND($12.34565,-1)", "Y:10.0000"},
    {"ROUND($1234.5678,-1)", "Y:1230.0000"},
    {"ROUND($12.3456,-2)", "Y:0.0000"},
    {"ROUND($-12.3456,-2)", "Y:0.0000"},
    {"ROUND($1.5,-2)", "Y:0.0000"},
    {"ROUND($-1.5,-2)", "Y:0.0000"},
    {"ROUND($2.5,-2)", "Y:0.0000"},
    {"ROUND($-2.5,-2)", "Y:0.0000"},
    {"ROUND($0.00005,-2)", "Y:0.0000"},
    {"ROUND($-0.00005,-2)", "Y:0.0000"},
    {"ROUND($12.34565,-2)", "Y:0.0000"},
    {"ROUND($1234.5678,-2)", "Y:1200.0000"},
    {"MOD($-12.3456,2)", "Y:1.6544"},
    {"MOD($12.3456,2)", "Y:0.3456"},
    {"MOD($12.3456,-2)", "Y:-1.6544"},
    {"MOD($-12.3456,-2)", "Y:-0.3456"},
    {"MOD($12.3456,$3)", "Y:0.3456"},
    {"MOD($12.3456,$-3)", "Y:-2.6544"},
    {"MOD(12.3456,$3)", "N:0.3456"},
    {"MOD($7,3)", "Y:1.0000"},
    {"MOD($7,0)", "ERR1307"},
    {"MOD($7,$0)", "ERR1307"},
    {"MOD($0.5,$0.2)", "Y:0.1000"},
    {"MOD($-0.5,$0.2)", "Y:0.1000"},
    {"MOD($922337203685477,$7)", "Y:6.0000"},
    {"MOD(-$7,$3)", "Y:2.0000"},
    {"MOD($7,$-3)", "Y:-2.0000"},
    {"MOD($7.5,$2.5)", "Y:0.0000"},
    {"SQRT($4)", "N:2.0000"},
    {"EXP($1)", "N:2.7183"},
    {"LOG($10)", "N:2.3026"},
    {"LOG10($100)", "N:2.0000"},
    {"SIN($1)", "N:0.8415"},
    {"$2^3", "N:8.0000"},
    {"SIGN($-5)", "N:-1.0000"},
    {"MAX($1,$2,$3)", "Y:3.0000"},
    {"MIN($3,$1,$2)", "Y:1.0000"},
    {"MAX($1,$-2)", "Y:1.0000"},
    {"MIN($1,2)", "N:1.0000"},
    {"MAX($1.5,$1.5)", "Y:1.5000"},
    {"MAX(1,$2)", "N:2.0000"},
    {"CEILING(3)", "N:3.0000"},
    {"FLOOR(-3.2)", "N:-4.0000"},
    {"CEILING(-3.2)", "N:-3.0000"},
    {"INT(-3.9)", "N:-3.0000"}
};

std::string run_rows(const fs::path &dir) {
    std::string body = "LOCAL cOut, oEx, x\ncOut = ''\n";
    for (const Row &row : kRows) {
        body += "TRY\n";
        body += "x = " + std::string(row.expression) + "\n";
        body += "cOut = cOut + VARTYPE(x) + ':' + LTRIM(STR(x, 22, 4)) + CHR(10)\n";
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

void test_currency_functions_match_vfp9() {
    const fs::path dir = fs::temp_directory_path() / "copperfin_currency_functions";
    std::error_code ignored;
    fs::remove_all(dir, ignored);
    const std::string output = run_rows(dir);
    expect(output.rfind("<incomplete", 0U) != 0U, "currency functions: the script should complete: " + output);
    std::vector<std::string> lines;
    for (std::size_t start = 0U; start < output.size();) {
        const std::size_t end = output.find('\n', start);
        lines.push_back(output.substr(start, end == std::string::npos ? std::string::npos : end - start));
        if (end == std::string::npos) {
            break;
        }
        start = end + 1U;
    }
    expect(lines.size() >= kRows.size(), "currency functions: one result per row, got " + std::to_string(lines.size()));
    for (std::size_t index = 0U; index < kRows.size() && index < lines.size(); ++index) {
        expect(lines[index] == kRows[index].expected,
            std::string("currency functions: ") + kRows[index].expression + " expected [" + kRows[index].expected +
                "], got [" + lines[index] + "]");
    }
    fs::remove_all(dir, ignored);
}

}  // namespace

int main() {
    test_currency_functions_match_vfp9();
    if (const int failures = test_failures(); failures != 0) {
        std::cerr << failures << " test(s) failed\n";
        return 1;
    }
    return 0;
}
