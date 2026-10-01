// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/runtime/prg_engine.h"
#include "prg_engine_test_support.h"

#include <filesystem>
#include <fstream>
#include <iterator>
#include <iostream>
#include <string>
#include <system_error>
#include <vector>

namespace {

using namespace copperfin::test_support;

namespace fs = std::filesystem;

// Governing requirement: RQ-CF-PRG-CURRENCY-OVERFLOW-001 (#6037).
//
// Currency arithmetic that leaves +-922337203685477.5807 is error 1988 "Currency value is out of range." and stops the
// statement; it must never widen to an inexact Number. Every expectation is installed VFP9 (09.00.0000.7423, Windows
// VM COM probe 2026-10-01, retained at /home/rich/temp/vfp9-probes/currency-c25/result7.txt) at SET DECIMALS 4: +, -, *
// and / overflow (including with a Number operand and a Numeric far outside the range), ROUND at the boundary
// overflows, and dividing a Currency by zero is error 1988, not the Numeric division-by-zero error. Rows whose VFP9
// display of the maximum is rounded through a double are left out. Three rows are not probe rows: the CEILING pair is
// a deliberate improvement over VFP9, which at the very top of the range silently wraps to a wrong value
// (CEILING($922337203685477.5807) is $-922337203685476), and -($-max-$0) checks that negating the maximum is fine. The
// stored minimum (INT64_MIN), which VFP9 cannot display, is covered separately with a patched Y table. The expected
// text is "<VARTYPE>:<TRANSFORM>" or "ERR<number>".

struct Row {
    const char *expression;
    const char *expected;
};

const std::vector<Row> kRows = {
    {"$922337203685477.5807+$0.0001", "ERR1988"},
    {"$922337203685477.5807+0.0001", "ERR1988"},
    {"0.0001+$922337203685477.5807", "ERR1988"},
    {"$-922337203685477.5807-$0.0002", "ERR1988"},
    {"$-922337203685477.5807-0.0002", "ERR1988"},
    {"$922337203685477.5807*2", "ERR1988"},
    {"$922337203685477.5807*1.0001", "ERR1988"},
    {"$922337203685477.5807*$2", "ERR1988"},
    {"$500000000000000*$3", "ERR1988"},
    {"$922337203685477.5807/0.5", "ERR1988"},
    {"$922337203685477.5807/$0.5", "ERR1988"},
    {"1000000000/$0.00001", "ERR1988"},
    {"$1+1E15", "ERR1988"},
    {"$1+1E19", "ERR1988"},
    {"$1*1E15", "ERR1988"},
    {"$1/1E-15", "ERR1988"},
    {"$922337203685477.5807*0", "Y:$0.0000"},
    {"ROUND($922337203685477.5807,0)", "ERR1988"},
    {"INT($922337203685477.5807)", "Y:$922,337,203,685,477.0000"},
    {"FLOOR($922337203685477.5807)", "Y:$922,337,203,685,477.0000"},
    {"ROUND($922337203685477.5807,-3)", "Y:$922,337,203,685,000.0000"},
    {"$1/0", "ERR1988"},
    {"$0/0", "ERR1988"},
    {"$1/$0", "ERR1988"},
    {"$922337203685477.5807+$-922337203685477.5807", "Y:$0.0000"},
    {"$922337203685477.5807-$922337203685477.5807", "Y:$0.0000"},
    {"$922337203685477.5807-$-1", "ERR1988"},
    {"$-1-$922337203685477.5807", "ERR1988"},
    {"$100000000*$100000000", "ERR1988"},
    {"$100000000*$10000", "Y:$1,000,000,000,000.0000"},
    {"$100000000*$1000000", "Y:$100,000,000,000,000.0000"},
    {"$1000000000000*$1000000", "ERR1988"},
    {"CEILING($922337203685477.5807)", "ERR1988"},
    {"CEILING($-922337203685477.5807)", "ERR1988"},
    {"-($-922337203685477.5807-$0)", "Y:$922,337,203,685,477.5807"}
};

std::string run_rows(const fs::path &dir) {
    std::string body = "LOCAL cOut, oEx, x\ncOut = ''\nSET DECIMALS TO 4\n";
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

void test_currency_overflow_matches_vfp9() {
    const fs::path dir = fs::temp_directory_path() / "copperfin_currency_overflow";
    std::error_code ignored;
    fs::remove_all(dir, ignored);
    const std::string output = run_rows(dir);
    expect(output.rfind("<incomplete", 0U) != 0U, "currency overflow: the script should complete: " + output);
    std::vector<std::string> lines;
    for (std::size_t start = 0U; start < output.size();) {
        const std::size_t end = output.find('\n', start);
        lines.push_back(output.substr(start, end == std::string::npos ? std::string::npos : end - start));
        if (end == std::string::npos) {
            break;
        }
        start = end + 1U;
    }
    expect(lines.size() >= kRows.size(), "currency overflow: one result per row, got " + std::to_string(lines.size()));
    for (std::size_t index = 0U; index < kRows.size() && index < lines.size(); ++index) {
        expect(lines[index] == kRows[index].expected,
            std::string("currency overflow: ") + kRows[index].expression + " expected [" + kRows[index].expected +
                "], got [" + lines[index] + "]");
    }
    fs::remove_all(dir, ignored);
}

// The stored Currency minimum (-922337203685477.5808, INT64_MIN) cannot be written by INSERT, so a Y table is created
// and its record patched, as the Currency function tests do. Only a result outside the range is error 1988: negating
// the minimum is, but subtracting it from itself is zero and -0.0001 minus it is the maximum (#6754 review).
void test_stored_currency_minimum_arithmetic() {
    const fs::path dir = fs::temp_directory_path() / "copperfin_currency_overflow_minimum";
    std::error_code ignored;
    fs::remove_all(dir, ignored);
    fs::create_directories(dir);
    write_text(dir / "make.prg", "CREATE TABLE cy (amount Y)\nINSERT INTO cy (amount) VALUES ($1)\nUSE\nRETURN\n");
    {
        auto session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options((dir / "make.prg").string(), dir.string(), false));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "currency minimum: the table should be created: " + state.message);
    }
    std::string bytes;
    {
        std::ifstream in(dir / "cy.dbf", std::ios::binary);
        bytes.assign(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
    }
    // The record is the last 9 bytes before the end-of-file marker: 1 deletion byte + 8 bytes of Currency.
    const std::size_t record = bytes.size() - 1U - 8U;
    for (std::size_t index = 0U; index < 7U; ++index) {
        bytes[record + index] = '\0';
    }
    bytes[record + 7U] = static_cast<char>(0x80);
    {
        std::ofstream out(dir / "cy.dbf", std::ios::binary);
        out.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
    }
    const std::vector<Row> rows = {
        {"amount", "Y:$-922,337,203,685,477.5808"},
        {"-amount", "ERR1988"},
        {"0-amount", "ERR1988"},
        {"$0-amount", "ERR1988"},
        {"amount-amount", "Y:$0.0000"},
        {"amount+$922337203685477.5807", "Y:$-0.0001"},
        {"$-0.0001-amount", "Y:$922,337,203,685,477.5807"},
        {"-0.0001-amount", "Y:$922,337,203,685,477.5807"},
        {"$-1-amount", "Y:$922,337,203,685,476.5808"},
        {"$1-amount", "ERR1988"},
        {"amount-$1", "ERR1988"},
        {"amount-0", "Y:$-922,337,203,685,477.5808"},
        {"amount+$1", "Y:$-922,337,203,685,476.5808"},
    };
    std::string body = "LOCAL cOut, oEx, x\ncOut = ''\nSET DECIMALS TO 4\nUSE cy\n";
    for (const Row &row : rows) {
        body += "TRY\nx = " + std::string(row.expression) + "\n";
        body += "cOut = cOut + VARTYPE(x) + ':' + TRANSFORM(x) + CHR(10)\n";
        body += "CATCH TO oEx\ncOut = cOut + 'ERR' + ALLTRIM(STR(oEx.ErrorNo)) + CHR(10)\nENDTRY\n";
    }
    body += "STRTOFILE(cOut, 'minimum.txt')\nRETURN\n";
    write_text(dir / "run.prg", body);
    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options((dir / "run.prg").string(), dir.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "currency minimum: the script should complete: " + state.message);
    const std::string output = read_text(dir / "minimum.txt");
    std::vector<std::string> lines;
    for (std::size_t start = 0U; start < output.size();) {
        const std::size_t end = output.find('\n', start);
        lines.push_back(output.substr(start, end == std::string::npos ? std::string::npos : end - start));
        if (end == std::string::npos) {
            break;
        }
        start = end + 1U;
    }
    expect(lines.size() >= rows.size(), "currency minimum: one result per row, got " + std::to_string(lines.size()));
    for (std::size_t index = 0U; index < rows.size() && index < lines.size(); ++index) {
        expect(lines[index] == rows[index].expected,
            std::string("currency minimum: ") + rows[index].expression + " expected [" + rows[index].expected +
                "], got [" + lines[index] + "]");
    }
    fs::remove_all(dir, ignored);
}

}  // namespace

int main() {
    test_currency_overflow_matches_vfp9();
    test_stored_currency_minimum_arithmetic();
    if (const int failures = test_failures(); failures != 0) {
        std::cerr << failures << " test(s) failed\n";
        return 1;
    }
    return 0;
}
