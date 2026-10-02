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

// Governing requirement: RQ-CF-PRG-TRANSFORM-CURRENCY-PICTURE-001 (#6168).
//
// A currency-only numeric picture ('$999') shapes the result: installed VFP9 (09.00.0000.7423, Windows VM COM probe
// 2026-10-01, retained at /home/rich/temp/vfp9-probes/currency-c25/result12.txt and result13.txt) truncates the value
// toward zero, right-aligns it in the picture's width, and lets the dollar sign take the leftmost padding space;
// a value that fills the whole width has no padding so the sign is dropped (999 is '$999', 1000 is '1000', -12 in
// '$99' is '-12') and a wider one is asterisks. The expected text is the result between brackets so padding shows.

struct Row {
    const char *expression;
    const char *expected;
};

const std::vector<Row> kRows = {
    {"TRANSFORM(5,'$999')", "[$  5]"},
    {"TRANSFORM(5,'$99')", "[$ 5]"},
    {"TRANSFORM(5,'$9')", "[$5]"},
    {"TRANSFORM(12345,'$999')", "[****]"},
    {"TRANSFORM(12,'$999')", "[$ 12]"},
    {"TRANSFORM(-5,'$999')", "[$ -5]"},
    {"TRANSFORM(5.5,'$999')", "[$  5]"},
    {"TRANSFORM(5.6,'$999')", "[$  5]"},
    {"TRANSFORM(2.5,'$999')", "[$  2]"},
    {"TRANSFORM(-5.6,'$999')", "[$ -5]"},
    {"TRANSFORM(-12,'$99')", "[-12]"},
    {"TRANSFORM(-123,'$99')", "[***]"},
    {"TRANSFORM(999,'$999')", "[$999]"},
    {"TRANSFORM(1000,'$999')", "[1000]"},
    {"TRANSFORM($12.5,'$999')", "[$ 12]"},
    {"TRANSFORM(0.4,'$999')", "[$  0]"},
    {"TRANSFORM(5,'$#99')", "[$  5]"},
    {"TRANSFORM(0,'$999')", "[$  0]"},
    {"TRANSFORM(5,'@B $999')", "[$  5]"},
    // Non-finite and very wide values (retained probe result17.txt).
    {"TRANSFORM(EXP(1000),'$999')", "[****]"},
    {"TRANSFORM(-EXP(1000),'$999')", "[****]"},
    {"TRANSFORM(9.1E18,'$99999999999999999999')", "[$ 9100000000000000000]"},
    // A Currency truncates exactly from its scaled integer, where a double would round up (VFP9 shows ...474).
    {"TRANSFORM($900719925474.9999,'$99999999999999')", "[$  900719925474]"},
    {"TRANSFORM($-900719925474.9999,'$99999999999999')", "[$ -900719925474]"}
};

// Rows run after SET CURRENCY, SET POINT and SET SEPARATOR: installed VFP9 leaves the picture's literal dollar sign and
// the whole-unit digits alone (result17.txt), so these match the plain rows.
const std::vector<Row> kSettingRows = {
    {"TRANSFORM(5,'$999')", "[$  5]"},
    {"TRANSFORM(12,'$999')", "[$ 12]"},
    {"TRANSFORM(999,'$999')", "[$999]"},
    {"TRANSFORM(5,'$99')", "[$ 5]"},
};

std::string run_rows(const fs::path &dir, const std::vector<Row> &rows, const std::string &preamble) {
    std::string body = "LOCAL cOut, oEx, x\ncOut = ''\n" + preamble;
    for (const Row &row : rows) {
        body += "TRY\n";
        body += "x = " + std::string(row.expression) + "\n";
        body += "cOut = cOut + '[' + x + ']' + CHR(10)\n";
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

void test_transform_currency_picture_matches_vfp9() {
    const fs::path dir = fs::temp_directory_path() / "copperfin_transform_currency_picture";
    std::error_code ignored;
    fs::remove_all(dir, ignored);
    const std::string output = run_rows(dir, kRows, "");
    expect(output.rfind("<incomplete", 0U) != 0U, "TRANSFORM currency picture: the script should complete: " + output);
    std::vector<std::string> lines;
    for (std::size_t start = 0U; start < output.size();) {
        const std::size_t end = output.find('\n', start);
        lines.push_back(output.substr(start, end == std::string::npos ? std::string::npos : end - start));
        if (end == std::string::npos) {
            break;
        }
        start = end + 1U;
    }
    expect(lines.size() >= kRows.size(),
        "TRANSFORM currency picture: one result per row, got " + std::to_string(lines.size()));
    for (std::size_t index = 0U; index < kRows.size() && index < lines.size(); ++index) {
        expect(lines[index] == kRows[index].expected,
            std::string("TRANSFORM currency picture: ") + kRows[index].expression + " expected [" +
                kRows[index].expected + "], got [" + lines[index] + "]");
    }
    fs::remove_all(dir, ignored);
}

void test_transform_currency_picture_ignores_currency_settings() {
    const fs::path dir = fs::temp_directory_path() / "copperfin_transform_currency_picture_settings";
    std::error_code ignored;
    fs::remove_all(dir, ignored);
    const std::string output = run_rows(dir, kSettingRows, "SET CURRENCY TO 'EUR'\nSET POINT TO ','\nSET SEPARATOR TO '.'\n");
    expect(output.rfind("<incomplete", 0U) != 0U, "TRANSFORM currency picture settings: the script should complete: " + output);
    std::size_t start = 0U;
    for (const Row &row : kSettingRows) {
        const std::size_t end = output.find('\n', start);
        const std::string line = output.substr(start, end == std::string::npos ? std::string::npos : end - start);
        expect(line == row.expected,
            std::string("TRANSFORM currency picture settings: ") + row.expression + " expected [" + row.expected +
                "], got [" + line + "]");
        if (end == std::string::npos) {
            break;
        }
        start = end + 1U;
    }
    fs::remove_all(dir, ignored);
}

}  // namespace

int main() {
    test_transform_currency_picture_matches_vfp9();
    test_transform_currency_picture_ignores_currency_settings();
    if (const int failures = test_failures(); failures != 0) {
        std::cerr << failures << " test(s) failed\n";
        return 1;
    }
    return 0;
}
