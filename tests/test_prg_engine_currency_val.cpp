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

// Governing requirement: RQ-CF-PRG-031 (#6728).
//
// A dollar-prefixed VAL that leaves the Currency range is error 1988 "Currency value is out of range." in installed
// VFP9 (09.00.0000.7423, Windows VM COM probe 2026-10-01, retained at
// /home/rich/temp/vfp9-probes/currency-c25/result12.txt), not the Numeric overflow error 39 the earlier analogy
// assumed. The value is rounded half away from zero to four decimals first, so $922337203685477.58075 is out of range
// and $922337203685477.58074 is the maximum; trailing text and surrounding blanks are ignored. Deliberate improvement
// over VFP9: it displays the maximum as $922,337,203,685,477.6000 (the VAL result passes through a double), Copperfin
// keeps the exact ...477.5807. VAL ignores an exponent after a dollar prefix ($1E3 is $1). Probe rows with VFP9's own quirks are not copied ($5., $-5, $+5 and $ 5). The
// expected text is "<VARTYPE>:<TRANSFORM>" or "ERR<number>".

struct Row {
    const char *expression;
    const char *expected;
};

const std::vector<Row> kRows = {
    {"VAL('$922337203685477.5807')", "Y:$922,337,203,685,477.5807"},
    {"VAL('$922337203685477.5808')", "ERR1988"},
    {"VAL('$-922337203685477.5808')", "ERR1988"},
    {"VAL('$-922337203685477.5809')", "ERR1988"},
    {"VAL('$922337203685477.58075')", "ERR1988"},
    {"VAL('$922337203685477.58074')", "Y:$922,337,203,685,477.5807"},
    {"VAL('$922337203685478')", "ERR1988"},
    {"VAL('$-922337203685478')", "ERR1988"},
    {"VAL('$9999999999999999999')", "ERR1988"},
    {"VAL('$922337203685477.5807abc')", "Y:$922,337,203,685,477.5807"},
    {"VAL('$922337203685477.5807 ')", "Y:$922,337,203,685,477.5807"},
    {"VAL(' $922337203685477.5807')", "Y:$922,337,203,685,477.5807"},
    {"VAL('$1.23456')", "Y:$1.2346"},
    {"VAL('$1.23455')", "Y:$1.2346"},
    {"VAL('$-1.23455')", "Y:$-1.2346"},
    {"VAL('$0.00005')", "Y:$0.0001"},
    {"VAL('$0.00004')", "Y:$0.0000"},
    {"VAL('$.5')", "Y:$0.5000"},
    {"VAL('$')", "Y:$0.0000"},
    {"VAL('$a')", "Y:$0.0000"},
    {"VAL('$1,5')", "Y:$1.0000"},
    {"VAL('$1E3')", "Y:$1.0000"},
    {"VAL('$1.2E2')", "Y:$1.2000"},
    {"VAL('$-2e+3')", "Y:$-2.0000"}
};

std::string run_rows(const fs::path &dir) {
    std::string body = "LOCAL cOut, oEx, x\ncOut = ''\nSET DECIMALS TO 4\nSET CURRENCY TO '$'\nSET POINT TO '.'\nSET SEPARATOR TO ','\n";
    for (const Row &row : kRows) {
        body += "TRY\n";
        body += "x = " + std::string(row.expression) + "\n";
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

void test_currency_val_matches_vfp9() {
    const fs::path dir = fs::temp_directory_path() / "copperfin_currency_val";
    std::error_code ignored;
    fs::remove_all(dir, ignored);
    const std::string output = run_rows(dir);
    expect(output.rfind("<incomplete", 0U) != 0U, "currency VAL: the script should complete: " + output);
    std::vector<std::string> lines;
    for (std::size_t start = 0U; start < output.size();) {
        const std::size_t end = output.find('\n', start);
        lines.push_back(output.substr(start, end == std::string::npos ? std::string::npos : end - start));
        if (end == std::string::npos) {
            break;
        }
        start = end + 1U;
    }
    expect(lines.size() >= kRows.size(), "currency VAL: one result per row, got " + std::to_string(lines.size()));
    for (std::size_t index = 0U; index < kRows.size() && index < lines.size(); ++index) {
        expect(lines[index] == kRows[index].expected,
            std::string("currency VAL: ") + kRows[index].expression + " expected [" + kRows[index].expected +
                "], got [" + lines[index] + "]");
    }
    fs::remove_all(dir, ignored);
}

}  // namespace

int main() {
    test_currency_val_matches_vfp9();
    if (const int failures = test_failures(); failures != 0) {
        std::cerr << failures << " test(s) failed\n";
        return 1;
    }
    return 0;
}
