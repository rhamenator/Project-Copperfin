// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/runtime/prg_engine.h"
#include "prg_engine_test_support.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <system_error>
#include <vector>

namespace {

using namespace copperfin::test_support;

namespace fs = std::filesystem;

// Governing requirement: RQ-CF-PRG-CURRENCY-TYPING-001 (#6036, #6061).
//
// Every expectation is installed VFP9 (09.00.0000.7423, Windows VM COM probe 2026-10-01, retained at
// /home/rich/temp/vfp9-probes/currency-c25/result.txt): a $-prefixed literal is a Currency (Y) value rounded to
// four decimals, a value outside +-922337203685477.5807 is error 1988, arithmetic with a Currency operand stays
// Currency (^ is Numeric), and a Y field reads as Currency. The expected text is "<VARTYPE>:<TRANSFORM>" or
// "ERR<number>". Not in this table, because they are separate issues or untouched: malformed literals
// ($1,000 is a function-argument error in VFP9), the Currency-preserving functions (ABS, ROUND, INT, CEILING,
// FLOOR, MAX, MIN, MOD, MTON, NTOM; #6039), Currency overflow in arithmetic (error 1988; #6037), the display width
// of a Numeric result such as $1.25^2 (VFP9 shows 1.5625), and the display of a value above 2^53 whole units (a VFP9
// double-precision artifact).

struct Row {
    const char *expression;
    const char *expected;
};

const std::vector<Row> kRows = {
    // A $ in operand position that is not followed by a numeral is VFP9's error 1300 ("Function name is missing )."),
    // not a Logical true (result4.txt).
    {"$1E3", "ERR1300"},
    {"$1e3", "ERR1300"},
    {"$1.2.3", "ERR1300"},
    {"$+3", "ERR1300"},
    {"$", "ERR1300"},
    {"$.", "ERR1300"},
    {"$a", "ERR1300"},
    {"$1a", "ERR1300"},
    {"$1.25", "Y:$1.25"},
    {"$2.0001", "Y:$2.00"},
    {"$1", "Y:$1.00"},
    {"$.5", "Y:$0.50"},
    {"$-1", "Y:$-1.00"},
    {"$-1.25", "Y:$-1.25"},
    {"$1.23456", "Y:$1.23"},
    {"$1.23455", "Y:$1.23"},
    {"$1.23454", "Y:$1.23"},
    {"$0.00005", "Y:$0.00"},
    {"$0.00004", "Y:$0.00"},
    {"$-0.00005", "Y:$-0.00"},
    {"$922337203685477.5808", "ERR1988"},
    {"$-922337203685477.5808", "ERR1988"},
    {"$999999999999999999", "ERR1988"},
    {"$ 5", "Y:$5.00"},
    {"$0", "Y:$0.00"},
    {"$00012.5", "Y:$12.50"},
    {"$1.0000", "Y:$1.00"},
    {"$100000000000000", "Y:$100,000,000,000,000.00"},
    {"$1.25+$2.0001", "Y:$3.25"},
    {"$1.25-$2.0001", "Y:$-0.75"},
    {"$1.25*$2", "Y:$2.50"},
    {"$1.25*2", "Y:$2.50"},
    {"2*$1.25", "Y:$2.50"},
    {"$10/$4", "Y:$2.50"},
    {"$10/4", "Y:$2.50"},
    {"10/$4", "Y:$2.50"},
    {"$10/3", "Y:$3.33"},
    {"$1.25+1", "Y:$2.25"},
    {"1+$1.25", "Y:$2.25"},
    {"$1.25-1", "Y:$0.25"},
    {"-$1.25", "Y:$-1.25"},
    {"+$1.25", "Y:$1.25"},
    {"$3=3", "L:.T."},
    {"$3==3", "L:.T."},
    {"$3>2", "L:.T."},
    {"$1.25=$1.2500", "L:.T."},
    {"$1.23456=$1.2346", "L:.T."},
    {"$1.00001=$1", "L:.T."},
    {"VARTYPE($1.25)", "C:Y"},
    {"VARTYPE($1.25+1)", "C:Y"},
    {"VARTYPE($1.25*$2)", "C:Y"},
    {"VARTYPE($10/4)", "C:Y"},
    {"VARTYPE($1.25^2)", "C:N"},
    {"VARTYPE(-$1)", "C:Y"},
    {"VARTYPE($1.25/$5)", "C:Y"}
};

std::string run_rows(const fs::path &dir) {
    std::string body = "LOCAL cOut, oEx, x\ncOut = ''\n";
    for (const Row &row : kRows) {
        body += "TRY\n";
        body += "x = " + std::string(row.expression) + "\n";
        body += "cOut = cOut + VARTYPE(x) + ':' + IIF(VARTYPE(x) = 'L', IIF(x, '.T.', '.F.'), TRANSFORM(x)) + CHR(10)\n";
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

void test_currency_literals_match_vfp9() {
    const fs::path dir = fs::temp_directory_path() / "copperfin_currency_literals";
    std::error_code ignored;
    fs::remove_all(dir, ignored);
    const std::string output = run_rows(dir);
    expect(output.rfind("<incomplete", 0U) != 0U, "currency literals: the script should complete: " + output);
    std::vector<std::string> lines;
    for (std::size_t start = 0U; start < output.size();) {
        const std::size_t end = output.find('\n', start);
        lines.push_back(output.substr(start, end == std::string::npos ? std::string::npos : end - start));
        if (end == std::string::npos) {
            break;
        }
        start = end + 1U;
    }
    expect(lines.size() >= kRows.size(), "currency literals: one result per row, got " + std::to_string(lines.size()));
    for (std::size_t index = 0U; index < kRows.size() && index < lines.size(); ++index) {
        expect(lines[index] == kRows[index].expected,
            std::string("currency literals: ") + kRows[index].expression + " expected [" + kRows[index].expected +
                "], got [" + lines[index] + "]");
    }
    fs::remove_all(dir, ignored);
}

// A Currency field is a Currency value in the runtime (#6061): VFP9 reports TYPE=Y and VALUE=$12.35 for 12.3456.
void test_currency_field_is_currency() {
    const fs::path dir = fs::temp_directory_path() / "copperfin_currency_field";
    std::error_code ignored;
    fs::remove_all(dir, ignored);
    fs::create_directories(dir);
    write_text(dir / "field.prg",
        "LOCAL c\n"
        "CREATE TABLE money (amount Y)\n"
        "INSERT INTO money (amount) VALUES (12.3456)\n"
        "GO TOP\n"
        "c = VARTYPE(money.amount) + ';' + TRANSFORM(money.amount) + ';' + VARTYPE(money.amount + 1) + ';' + "
        "LTRIM(STR(money.amount, 12, 4)) + ';' + IIF(money.amount = $12.3456, 'eq', 'ne')\n"
        "APPEND BLANK\n"
        "c = c + ';' + VARTYPE(money.amount) + ';' + TRANSFORM(money.amount)\n"
        "STRTOFILE(c, 'field.txt')\nRETURN\n");
    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options((dir / "field.prg").string(), dir.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "currency field: the script should complete: " + state.message);
    expect(read_text(dir / "field.txt") == "Y;$12.35;Y;12.3456;eq;Y;$0.00",
        "currency field: expected Y;$12.35;Y;12.3456;eq;Y;$0.00, got [" + read_text(dir / "field.txt") + "]");
    fs::remove_all(dir, ignored);
}

// TRANSFORM(Y) honors SET POINT, SET SEPARATOR, SET CURRENCY and SET DECIMALS (installed VFP9, retained at
// currency-c25/result2.txt). SET CURRENCY RIGHT placement and a blank-space SET SEPARATOR (the runtime reads a blank
// as unset) are not covered.
void test_currency_transform_honors_settings() {
    const fs::path dir = fs::temp_directory_path() / "copperfin_currency_transform_settings";
    std::error_code ignored;
    fs::remove_all(dir, ignored);
    fs::create_directories(dir);
    write_text(dir / "settings.prg",
        "LOCAL c\n"
        "c = TRANSFORM($1234.5678) + '|' + TRANSFORM(-$1234.5678) + '|' + TRANSFORM($0.5) + '|' + TRANSFORM($1234567.891)\n"
        "SET POINT TO ','\n"
        "c = c + '|' + TRANSFORM($1234.5678) + '|' + TRANSFORM($1234567.891)\n"
        "SET SEPARATOR TO '.'\n"
        "c = c + '|' + TRANSFORM($1234.5678) + '|' + TRANSFORM($1234567.891)\n"
        "SET POINT TO\nSET SEPARATOR TO\nSET CURRENCY TO 'EUR'\n"
        "c = c + '|' + TRANSFORM($1234.5678) + '|' + TRANSFORM(-$5)\n"
        "SET CURRENCY TO '$'\nSET DECIMALS TO 4\n"
        "c = c + '|' + TRANSFORM($1234.5678)\n"
        "SET DECIMALS TO 0\n"
        "c = c + '|' + TRANSFORM($1234.5678)\n"
        "STRTOFILE(c, 'settings.txt')\nRETURN\n");
    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options((dir / "settings.prg").string(), dir.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "currency settings: the script should complete: " + state.message);
    const std::string expected =
        "$1,234.57|$-1,234.57|$0.50|$1,234,567.89|$1,234,57|$1,234,567,89|$1.234,57|$1.234.567,89|"
        "EUR1,234.57|EUR-5.00|$1,234.5678|$1,235";
    expect(read_text(dir / "settings.txt") == expected,
        "currency settings: expected [" + expected + "], got [" + read_text(dir / "settings.txt") + "]");
    fs::remove_all(dir, ignored);
}

// A real table written by VFP9 (09.00.0000.7423: CREATE TABLE cy (amount Y) and six INSERTs of $12.3456, $-12.3456,
// $922337203685477.5807, $0, $0.0001 and $-0.0001; retained at ~/temp/vfp9-probes/currency-c25/cy.dbf). The fourth
// record is patched to the stored minimum (INT64_MIN, -922337203685477.5808), which INSERT cannot produce, so the
// full signed range is read straight from the bytes (#6061).
const unsigned char kVfpCurrencyTable[383] = {
    0x30, 0x1a, 0x0a, 0x01, 0x06, 0x00, 0x00, 0x00, 0x48, 0x01, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00,
    0x41, 0x4d, 0x4f, 0x55, 0x4e, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x59, 0x01, 0x00, 0x00, 0x00,
    0x08, 0x04, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x0d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x40, 0xe2, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x20, 0xc0, 0x1d, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0x20, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0x7f, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x01, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1a,
};

void test_vfp_written_currency_table_is_read_exactly() {
    const fs::path dir = fs::temp_directory_path() / "copperfin_vfp_currency_table";
    std::error_code ignored;
    fs::remove_all(dir, ignored);
    fs::create_directories(dir);
    std::string bytes(reinterpret_cast<const char *>(kVfpCurrencyTable), sizeof(kVfpCurrencyTable));
    constexpr std::size_t kHeaderLength = 328U;   // the header length stored at offset 8
    constexpr std::size_t kRecordLength = 9U;     // 1 deletion byte + 8 bytes of Currency
    // Patch record 4 ($0) to INT64_MIN, little-endian.
    const std::size_t record4 = kHeaderLength + (3U * kRecordLength) + 1U;
    for (std::size_t index = 0U; index < 7U; ++index) {
        bytes[record4 + index] = '\0';
    }
    bytes[record4 + 7U] = static_cast<char>(0x80);
    {
        std::ofstream out(dir / "cy.dbf", std::ios::binary);
        out.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
    }
    write_text(dir / "read.prg",
        "LOCAL c\n"
        "SET DECIMALS TO 4\n"
        "USE cy\n"
        "c = ''\n"
        "SCAN\n"
        "    c = c + VARTYPE(amount) + TRANSFORM(amount) + ';'\n"
        "ENDSCAN\n"
        "STRTOFILE(c, 'read.txt')\nRETURN\n");
    auto options = make_runtime_session_options((dir / "read.prg").string(), dir.string(), false);
    auto session = copperfin::runtime::PrgRuntimeSession::create(options);
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "VFP currency table: the script should complete: " + state.message);
    const std::string expected =
        "Y$12.3456;Y$-12.3456;Y$922,337,203,685,477.5807;Y$-922,337,203,685,477.5808;Y$0.0001;Y$-0.0001;";
    expect(read_text(dir / "read.txt") == expected,
        "VFP currency table: expected [" + expected + "], got [" + read_text(dir / "read.txt") + "]");
    fs::remove_all(dir, ignored);
}

// A deliberate improvement, not a VFP9 row: VFP9 parses a $ literal through a double, so above about 900.7 billion
// currency units (2^53 scaled units; the value carries no currency, so this is realistic for a low-value unit such as
// the rial or dong) it loses the last digits ($900719925474.0003 reads back as ...0004 and $900719925474.0993 + 0 as ...0994, result6.txt).
// Copperfin keeps the exact scaled integer and adds a Currency and a Number as integers.
void test_large_currency_arithmetic_is_exact() {
    const fs::path dir = fs::temp_directory_path() / "copperfin_currency_exact";
    std::error_code ignored;
    fs::remove_all(dir, ignored);
    fs::create_directories(dir);
    write_text(dir / "exact.prg",
        "LOCAL c\n"
        "SET DECIMALS TO 4\n"
        "c = TRANSFORM($900719925474.0003 + 0.0001) + ';' + TRANSFORM($900719925474.0993 + 0) + ';' + "
        "TRANSFORM($900719925474.0003 - 0.0001) + ';' + TRANSFORM(0.0001 - $900719925474.0003) + ';' + "
        "TRANSFORM($900719925474.0003 + $0.0001)\n"
        "STRTOFILE(c, 'exact.txt')\nRETURN\n");
    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options((dir / "exact.prg").string(), dir.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "exact currency: the script should complete: " + state.message);
    const std::string expected =
        "$900,719,925,474.0004;$900,719,925,474.0993;$900,719,925,474.0002;$-900,719,925,474.0002;$900,719,925,474.0004";
    expect(read_text(dir / "exact.txt") == expected,
        "exact currency: expected [" + expected + "], got [" + read_text(dir / "exact.txt") + "]");
    fs::remove_all(dir, ignored);
}

}  // namespace

int main() {
    test_vfp_written_currency_table_is_read_exactly();
    test_large_currency_arithmetic_is_exact();
    test_currency_literals_match_vfp9();
    test_currency_field_is_currency();
    test_currency_transform_honors_settings();
    if (const int failures = test_failures(); failures != 0) {
        std::cerr << failures << " test(s) failed\n";
        return 1;
    }
    return 0;
}
