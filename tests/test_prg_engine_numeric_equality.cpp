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

// Governing requirement: RQ-CF-PRG-NUMERIC-EQUALITY-001 (#6034).
//
// Every expectation is installed VFP9 (09.00.0000.7423, Windows VM COM probes 2026-10-01, retained at
// /home/rich/temp/vfp9-probes/numeric-eq-c24/result*.txt) at its default settings: numbers that are the same
// double or differ by about one unit in the last place are equal (a computed 0.1+0.2 equals 0.3), numbers that
// differ in the 15th or 16th significant digit, or by the former absolute 0.000001, are distinct
// (1.0000005 <> 1), and the ordering operators agree with equality. Known divergences from VFP9, which tracks
// decimal places on its numeric values and rounds computed results to them, are listed in the traceability row
// and are not in this table: 1E14+0.01=1E14 and 1E10+1E-6=1E10 are .F. in VFP9, 1E-300=0 is .T., and the
// result depends on SET DECIMALS (at 18 decimals 0.1+0.2=0.3 is .F.).

struct Row {
    const char *expression;
    const char *expected;
};

const std::vector<Row> kRows = {
    {"0.1+0.2=0.3", ".T."},
    {"0.1*3=0.3", ".T."},
    {"1/3*3=1", ".T."},
    {"SQRT(2)*SQRT(2)=2", ".T."},
    {"100*1.1=110", ".T."},
    {"1.1+2.2=3.3", ".T."},
    {"0.7+0.1=0.8", ".T."},
    {"4.35*100=435", ".T."},
    {"1.15*100=115", ".T."},
    {"0.1+0.2<>0.3", ".F."},
    {"0.1+0.2#0.3", ".F."},
    {"0.1+0.2!=0.3", ".F."},
    {"0.1+0.2==0.3", ".T."},
    {"1.0000005=1", ".F."},
    {"1.0000005=1.0000000", ".F."},
    {"1.0000005<>1", ".T."},
    {"1.0000005#1", ".T."},
    {"-1.0000005=-1", ".F."},
    {"1.0000005=1.0000005", ".T."},
    {"1.00000000001=1", ".F."},
    {"1.0000000000001=1", ".F."},
    {"1.00000000000001=1", ".F."},
    {"1.000000000000001=1", ".F."},
    {"1.0000000000000001=1", ".T."},
    {"0.0000005=0", ".F."},
    {"0.0000000000001=0", ".F."},
    {"1000000.0000005=1000000", ".F."},
    {"123456789.123456789=123456789.123456788", ".T."},
    {"1E15+1=1E15", ".F."},
    {"1E15+0.5=1E15", ".F."},
    {"1E15+0.25=1E15", ".F."},
    {"1E20+1=1E20", ".T."},
    {"1E20+1E5=1E20", ".F."},
    {"1E20+1E6=1E20", ".F."},
    {"1E20*1.0000001=1E20", ".F."},
    {"1E10+1E-5=1E10", ".F."},
    {"1E10+1E-7=1E10", ".T."},
    {"1E14+0.1=1E14", ".F."},
    {"1+1E-15=1", ".F."},
    {"1+1E-16=1", ".T."},
    {"1+5*1E-15=1", ".F."},
    {"1+100*1E-15=1", ".F."},
    {"INLIST(1,1.0000005)", ".F."},
    {"INLIST(0.3,0.1+0.2)", ".T."},
    {"INLIST(0.1+0.2,0.3)", ".T."},
    {"INLIST(1,2,1.0000005,1)", ".T."},
    {"INLIST(1,2,1.0000005,3)", ".F."},
    {"1.0000005>1", ".T."},
    {"1.0000005<1", ".F."},
    {"1.0000005>=1", ".T."},
    {"1.0000005<=1", ".F."},
    {"0.1+0.2>0.3", ".F."},
    {"0.1+0.2<0.3", ".F."},
    {"0.1+0.2>=0.3", ".T."},
    {"0.1+0.2<=0.3", ".T."}
};

std::string run_rows(const fs::path &dir) {
    std::string body = "LOCAL cOut, oEx, x\ncOut = ''\n";
    for (const Row &row : kRows) {
        body += "TRY\n";
        body += "x = " + std::string(row.expression) + "\n";
        body += "cOut = cOut + IIF(x, '.T.', '.F.') + CHR(10)\n";
        body += "CATCH TO oEx\n";
        body += "cOut = cOut + 'ERR' + ALLTRIM(STR(oEx.ErrorNo)) + CHR(10)\n";
        body += "ENDTRY\n";
    }
    // ASCAN and a DBF field with six or more decimal places (the cases the issue names).
    body += "DIMENSION aNums[3]\naNums[1] = 1.0000005\naNums[2] = 0.1 + 0.2\naNums[3] = 5\n";
    body += "cOut = cOut + ALLTRIM(STR(ASCAN(aNums, 1.0000000))) + CHR(10)\n";
    body += "cOut = cOut + ALLTRIM(STR(ASCAN(aNums, 0.3))) + CHR(10)\n";
    body += "cOut = cOut + ALLTRIM(STR(ASCAN(aNums, 1.0000005))) + CHR(10)\n";
    body += "CREATE CURSOR tn (n1 N(20,10), n2 N(20,10))\nAPPEND BLANK\n";
    body += "REPLACE n1 WITH 1.0000005, n2 WITH 1.0000000\n";
    body += "cOut = cOut + IIF(n1 = n2, '.T.', '.F.') + IIF(n1 <> n2, '.T.', '.F.') + IIF(n1 = 1.0000005, '.T.', '.F.') + CHR(10)\n";
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

void test_numeric_equality_matches_vfp9() {
    const fs::path dir = fs::temp_directory_path() / "copperfin_numeric_equality";
    std::error_code ignored;
    fs::remove_all(dir, ignored);
    const std::string output = run_rows(dir);
    expect(output.rfind("<incomplete", 0U) != 0U, "numeric equality: the script should complete: " + output);
    std::vector<std::string> lines;
    for (std::size_t start = 0U; start < output.size();) {
        const std::size_t end = output.find('\n', start);
        lines.push_back(output.substr(start, end == std::string::npos ? std::string::npos : end - start));
        if (end == std::string::npos) {
            break;
        }
        start = end + 1U;
    }
    expect(lines.size() >= kRows.size() + 4U, "numeric equality: one result per row, got " + std::to_string(lines.size()));
    for (std::size_t index = 0U; index < kRows.size() && index < lines.size(); ++index) {
        expect(lines[index] == kRows[index].expected,
            std::string("numeric equality: ") + kRows[index].expression + " expected [" + kRows[index].expected +
                "], got [" + lines[index] + "]");
    }
    // ASCAN finds 0.3 at the computed 0.1+0.2, never matches 1.0 against 1.0000005, and finds 1.0000005 exactly;
    // the six-decimal DBF fields are distinct (equal .F., different .T., the literal match .T.).
    const std::size_t base = kRows.size();
    if (lines.size() >= base + 4U) {
        expect(lines[base] == "0", "numeric equality: ASCAN(aNums, 1.0000000) must not match 1.0000005, got " + lines[base]);
        expect(lines[base + 1U] == "2", "numeric equality: ASCAN(aNums, 0.3) should find the computed 0.1+0.2, got " + lines[base + 1U]);
        expect(lines[base + 2U] == "1", "numeric equality: ASCAN(aNums, 1.0000005) should find element 1, got " + lines[base + 2U]);
        expect(lines[base + 3U] == ".F..T..T.", "numeric equality: DBF six-decimal fields, got " + lines[base + 3U]);
    }
    fs::remove_all(dir, ignored);
}

// REPLACE stores the number, not its six-digit display text: value_as_string() kept 123.457 for 123.456789 and 1
// for 1.0000005. Every expectation is installed VFP9 (probe retained at
// /home/rich/temp/vfp9-probes/numeric-eq-c24/result6.txt): the value is rounded half away from zero to the field's
// scale. Columns: value, N(20,10), N(12,6), N(10,2), N(18,0), F(20,8).
struct StoreRow {
    const char *value;
    const char *n1;
    const char *n2;
    const char *n3;
    const char *n4;
    const char *f1;
};

const std::vector<StoreRow> kStoreRows = {
    {"123.456789", "123.4567890000", "123.456789", "123.46", "123", "123.45678900"},
    {"1.234567891", "1.2345678910", "1.234568", "1.23", "1", "1.23456789"},
    {"0.1234567", "0.1234567000", "0.123457", "0.12", "0", "0.12345670"},
    {"1.0000005", "1.0000005000", "1.000001", "1.00", "1", "1.00000050"},
    {"1.000005", "1.0000050000", "1.000005", "1.00", "1", "1.00000500"},
    {"12345.6789012", "12345.6789012000", "12345.678901", "12345.68", "12346", "12345.67890120"},
    {"-2.5000005", "-2.5000005000", "-2.500001", "-2.50", "-3", "-2.50000050"},
    {"0.000001234", "0.0000012340", "0.000001", "0.00", "0", "0.00000123"},
    {"1E-7", "0.0000001000", "0.000000", "0.00", "0", "0.00000010"},
    {"0.1+0.2", "0.3000000000", "0.300000", "0.30", "0", "0.30000000"},
    {"1/3", "0.3333333333", "0.333333", "0.33", "0", "0.33333333"},
    {"2/3", "0.6666666667", "0.666667", "0.67", "1", "0.66666667"},
    {"100/7", "14.2857142857", "14.285714", "14.29", "14", "14.28571429"}
};

void test_replace_keeps_numeric_digits() {
    const fs::path dir = fs::temp_directory_path() / "copperfin_numeric_store";
    std::error_code ignored;
    fs::remove_all(dir, ignored);
    fs::create_directories(dir);
    std::string body = "LOCAL cOut\ncOut = ''\n"
                       "CREATE TABLE tn (n1 N(20,10), n2 N(12,6), n3 N(10,2), n4 N(18,0), f1 F(20,8))\nAPPEND BLANK\n";
    for (const StoreRow &row : kStoreRows) {
        body += std::string("REPLACE n1 WITH ") + row.value + ", n2 WITH " + row.value + ", n3 WITH " + row.value +
                ", n4 WITH " + row.value + ", f1 WITH " + row.value + "\n";
        body += "cOut = cOut + ALLTRIM(STR(n1,20,10)) + '|' + ALLTRIM(STR(n2,20,6)) + '|' + ALLTRIM(STR(n3,20,2)) + '|' + "
                "ALLTRIM(STR(n4,20,0)) + '|' + ALLTRIM(STR(f1,20,8)) + CHR(10)\n";
    }
    body += "STRTOFILE(cOut, 'store.txt')\nRETURN\n";
    write_text(dir / "store.prg", body);
    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options((dir / "store.prg").string(), dir.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "numeric store: the script should complete: " + state.message);
    const std::string output = read_text(dir / "store.txt");
    std::vector<std::string> lines;
    for (std::size_t start = 0U; start < output.size();) {
        const std::size_t end = output.find('\n', start);
        lines.push_back(output.substr(start, end == std::string::npos ? std::string::npos : end - start));
        if (end == std::string::npos) {
            break;
        }
        start = end + 1U;
    }
    expect(lines.size() >= kStoreRows.size(), "numeric store: one result per value, got " + std::to_string(lines.size()));
    for (std::size_t index = 0U; index < kStoreRows.size() && index < lines.size(); ++index) {
        const StoreRow &row = kStoreRows[index];
        const std::string expected = std::string(row.n1) + "|" + row.n2 + "|" + row.n3 + "|" + row.n4 + "|" + row.f1;
        expect(lines[index] == expected,
            std::string("numeric store: REPLACE WITH ") + row.value + " expected [" + expected + "], got [" + lines[index] + "]");
    }
    fs::remove_all(dir, ignored);
}

}  // namespace

int main() {
    test_numeric_equality_matches_vfp9();
    test_replace_keeps_numeric_digits();
    if (const int failures = test_failures(); failures != 0) {
        std::cerr << failures << " test(s) failed\n";
        return 1;
    }
    return 0;
}
