// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/runtime/prg_engine.h"
#include "../src/runtime/prg_compatibility_error.h"
#include "../src/runtime/prg_engine_helpers.h"
#include "prg_engine_test_support.h"

#include <filesystem>
#include <iostream>
#include <string>
#include <system_error>
#include <vector>

namespace {

using namespace copperfin::test_support;
namespace fs = std::filesystem;

// Governing requirement: RQ-CF-PRG-BINARY-CONVERSION-001 (#5766 under #5611/#6776).
// Golden bytes and errors come from installed VFP9 SP2 and are retained at
// ~/temp/vfp9-probes/bintoc-5766/probe*.out. Unlike the former self-round-trip,
// these rows pin the external byte representation, result type, selector
// grammar and pre-allocation validation independently.
struct Row {
    const char *expression;
    const char *expected;
};

const std::vector<Row> kRows = {
    {"VARTYPE(CTOBIN(CHR(128)))", "C:N"},
    {"IIF(CTOBIN(CHR(128))=0,'T','F')", "C:T"},
    {"IIF(CTOBIN(CHR(128),'S')=-128,'T','F')", "C:T"},
    {"IIF(CTOBIN(CHR(0)+CHR(128))=-32640,'T','F')", "C:T"},
    {"IIF(CTOBIN(CHR(0)+CHR(128),'S')=128,'T','F')", "C:T"},
    {"IIF(CTOBIN(CHR(128)+CHR(0),'R')=-32640,'T','F')", "C:T"},
    {"IIF(CTOBIN(CHR(0)+CHR(0)+CHR(0)+CHR(128),'S')=128,'T','F')", "C:T"},
    {"IIF(ABS(CTOBIN(CHR(0)+CHR(0)+CHR(192)+CHR(63),'N')-1.5)<0.000001,'T','F')", "C:T"},
    {"IIF(ABS(CTOBIN(CHR(24)+CHR(45)+CHR(68)+CHR(84)+CHR(251)+CHR(33)+CHR(9)+CHR(64),'N')-PI())<0.000000000001,'T','F')", "C:T"},
    {"IIF(ABS(CTOBIN(CHR(192)+CHR(40)+CHR(174)+CHR(20)+CHR(122)+CHR(225)+CHR(71)+CHR(174),'B')-12.34)<0.000000000001,'T','F')", "C:T"},
    {"IIF(CTOBIN(CHR(128)+CHR(0)+CHR(0)+CHR(0)+CHR(0)+CHR(1)+CHR(226)+CHR(8),'Y')=$12.34,'T','F')", "C:T"},
    {"VARTYPE(CTOBIN(CHR(128)+CHR(0)+CHR(0)+CHR(0)+CHR(0)+CHR(1)+CHR(226)+CHR(8),'Y'))", "C:Y"},
    {"IIF(ABS(CTOBIN(BINTOC(1.5,'F'),'N')-1.5)<0.000001,'T','F')", "C:T"},
    {"IIF(ABS(CTOBIN(BINTOC(PI(),'B'),'N')-PI())<0.000000000001,'T','F')", "C:T"},
    {"IIF(ABS(CTOBIN(BINTOC(PI(),'BR'),'NRS')-PI())<0.000000000001,'T','F')", "C:T"},
    {"IIF(ABS(CTOBIN(BINTOC(12.34,8),'B')-12.34)<0.000000000001,'T','F')", "C:T"},
    {"IIF(CTOBIN(BINTOC($12.34,8),'Y')=$12.34,'T','F')", "C:T"},
    {"IIF(ISNULL(BINTOC(.NULL.,4)),'T','F')", "C:T"},
    {"IIF(ISNULL(CTOBIN(.NULL.)),'T','F')", "C:T"},
    {"BINTOC(128,1)", "ERR11"},
    {"BINTOC(-129,1)", "ERR11"},
    {"BINTOC(32768,2)", "ERR11"},
    {"BINTOC(2147483648,4)", "ERR11"},
    {"BINTOC(1,3)", "ERR11"},
    {"BINTOC(1,-1)", "ERR11"},
    {"BINTOC(1,1E300)", "ERR11"},
    {"BINTOC(1,EXP(1000))", "ERR11"},
    {"BINTOC('x',4)", "ERR11"},
    {"BINTOC(1,'')", "ERR11"},
    {"BINTOC(1,'X')", "ERR11"},
    {"BINTOC(1,'4RR')", "ERR11"},
    // The documented base selectors are mutually exclusive. VFP9 accepts a
    // few ambiguous repeated combinations; Copperfin rejects them rather than
    // silently choosing one interpretation.
    {"BINTOC(1,'FB')", "ERR11"},
    {"BINTOC(1,'12')", "ERR11"},
    {"BINTOC(1,4,'R')", "ERR11"},
    {"CTOBIN('')", "ERR11"},
    {"CTOBIN(CHR(1)+CHR(2)+CHR(3))", "ERR11"},
    {"CTOBIN(REPLICATE(CHR(1),9))", "ERR11"},
    {"CTOBIN(CHR(1),'2')", "ERR11"},
    {"CTOBIN(CHR(1),'B')", "ERR11"},
    {"CTOBIN(CHR(1),'Y')", "ERR11"},
    {"CTOBIN(CHR(1),'X')", "ERR11"},
    {"CTOBIN(CHR(1),'1SS')", "ERR11"},
    {"CTOBIN(CHR(1),'1','R')", "ERR11"},
    {"CTOBIN(1)", "ERR11"},
};

std::string run_rows(const fs::path &dir) {
    std::string body = "LOCAL cOut, oEx, x\ncOut = ''\n";
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

void test_binary_conversion_matches_vfp9_and_is_bounded() {
    const fs::path dir = fs::temp_directory_path() / "copperfin_binary_conversion";
    std::error_code ignored;
    fs::remove_all(dir, ignored);
    const std::string output = run_rows(dir);
    expect(output.rfind("<incomplete", 0U) != 0U, "binary conversion script should complete: " + output);
    std::vector<std::string> lines;
    for (std::size_t start = 0U; start < output.size();) {
        const std::size_t end = output.find('\n', start);
        if (end == std::string::npos) break;
        lines.push_back(output.substr(start, end - start));
        start = end + 1U;
    }
    expect(lines.size() == kRows.size(), "binary conversion should emit one result per row");
    const std::size_t count = std::min(lines.size(), kRows.size());
    for (std::size_t index = 0U; index < count; ++index) {
        expect(lines[index] == kRows[index].expected,
               std::string("binary conversion: ") + kRows[index].expression + " expected [" +
                   kRows[index].expected + "] got [" + lines[index] + "]");
    }
    fs::remove_all(dir, ignored);
}

std::string hex_bytes(const copperfin::runtime::PrgValue &value) {
    static constexpr char kDigits[] = "0123456789ABCDEF";
    std::string result;
    for (const unsigned char byte : value.string_value) {
        result.push_back(kDigits[byte >> 4U]);
        result.push_back(kDigits[byte & 0x0FU]);
    }
    return result;
}

void expect_bintoc_hex(
    const copperfin::runtime::PrgValue &value,
    const copperfin::runtime::PrgValue *selector,
    const std::string &expected) {
    const auto result = copperfin::runtime::bintoc_value(value, selector);
    expect(result.kind == copperfin::runtime::PrgValueKind::string && hex_bytes(result) == expected,
           "BINTOC golden bytes expected [" + expected + "] got [" + hex_bytes(result) + "]");
}

void test_bintoc_golden_bytes() {
    using namespace copperfin::runtime;
    const PrgValue one = make_number_value(1.0);
    const PrgValue minus_one = make_number_value(-1.0);
    const PrgValue width1 = make_number_value(1.0);
    const PrgValue width2 = make_number_value(2.0);
    const PrgValue width4 = make_number_value(4.0);
    const PrgValue width8 = make_number_value(8.0);
    expect_bintoc_hex(one, nullptr, "80000001");
    expect_bintoc_hex(one, &width1, "81");
    expect_bintoc_hex(one, &width2, "8001");
    expect_bintoc_hex(minus_one, &width1, "7F");
    expect_bintoc_hex(minus_one, &width2, "7FFF");
    expect_bintoc_hex(minus_one, &width4, "7FFFFFFF");
    expect_bintoc_hex(make_number_value(-128), &width1, "00");
    expect_bintoc_hex(make_number_value(127), &width1, "FF");
    expect_bintoc_hex(make_number_value(-32768), &width2, "0000");
    expect_bintoc_hex(make_number_value(32767), &width2, "FFFF");
    expect_bintoc_hex(make_number_value(-2147483648.0), &width4, "00000000");
    expect_bintoc_hex(make_number_value(2147483647.0), &width4, "FFFFFFFF");
    const PrgValue flags4s = make_string_value("4S");
    const PrgValue flags4r = make_string_value("4R");
    const PrgValue flags4rs = make_string_value("4RS");
    expect_bintoc_hex(minus_one, &flags4s, "FFFFFFFF");
    expect_bintoc_hex(minus_one, &flags4r, "FFFFFF7F");
    expect_bintoc_hex(one, &flags4rs, "01000000");
    const PrgValue flags_f = make_string_value("F");
    const PrgValue flags_fr = make_string_value("FR");
    const PrgValue flags_b = make_string_value("B");
    const PrgValue flags_br = make_string_value("BR");
    expect_bintoc_hex(make_number_value(1.5), &flags_f, "0000C03F");
    expect_bintoc_hex(make_number_value(-100), &flags_fr, "C2C80000");
    expect_bintoc_hex(make_number_value(3.14159265358979323846), &flags_b, "182D4454FB210940");
    expect_bintoc_hex(make_number_value(3.14159265358979323846), &flags_br, "400921FB54442D18");
    expect_bintoc_hex(make_number_value(12.34), &width8, "C028AE147AE147AE");
    expect_bintoc_hex(make_number_value(-12.34), &width8, "3FD751EB851EB851");
    expect_bintoc_hex(make_currency_value(123400), &width8, "800000000001E208");
    expect_bintoc_hex(make_currency_value(-123400), &width8, "7FFFFFFFFFFE1DF8");
}

void test_ctobin_rejects_internal_object_references() {
    using namespace copperfin::runtime;
    PrgValue object_reference = make_string_value(std::string(1U, '\x80'));
    object_reference.is_object_reference = true;
    try {
        (void)ctobin_value(object_reference, nullptr);
        expect(false, "CTOBIN should reject an object reference even though its internal payload is string-backed");
    } catch (const PrgCompatibilityError &error) {
        expect(error.error_code() == 11, "CTOBIN object-reference rejection should be catchable error 11");
    }
}

}  // namespace

int main() {
    test_bintoc_golden_bytes();
    test_ctobin_rejects_internal_object_references();
    test_binary_conversion_matches_vfp9_and_is_bounded();
    if (const int failures = copperfin::test_support::test_failures(); failures != 0) {
        std::cerr << failures << " failure(s)\n";
        return 1;
    }
    return 0;
}
