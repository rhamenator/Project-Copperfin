// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/runtime/prg_engine.h"
#include "../src/runtime/prg_engine_helpers.h"
#include "../src/runtime/prg_engine_numeric_functions.h"
#include "../src/runtime/prg_compatibility_error.h"
#include "prg_engine_test_support.h"

#include <filesystem>
#include <iterator>
#include <fstream>
#include <iostream>
#include <limits>
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

// Rows at the boundary of the Currency range, compared exactly with TRANSFORM at SET DECIMALS 4 (STR() goes through a
// double and loses the last digits).
const std::vector<Row> kBoundaryRows = {
    // Results that leave the Currency range are error 1988 (result8.txt: the rows where the exact result is out of range).
    {"ROUND($922337203685477.5807,0)", "ERR1988"},
    {"ROUND($922337203685477.5807,1)", "ERR1988"},
    {"ROUND($922337203685477.5807,3)", "ERR1988"},
    {"ROUND($-922337203685477.5807,0)", "ERR1988"},
    {"ROUND($-922337203685477.5807,3)", "ERR1988"},
    {"ROUND($922337203685477.5001,0)", "ERR1988"},
    {"ROUND($922337203685476.5001,0)", "Y:$922,337,203,685,477.0000"},
    {"MOD($922337203685477.5807,$-0.0001)", "Y:$0.0000"},
    {"MOD($922337203685477.5807,$0.0001)", "Y:$0.0000"},
    {"MOD($-922337203685477.5807,$-0.0001)", "Y:$0.0000"},
    // Deliberate improvements over VFP9, which reports 1988 whenever a ROUND is near the top of the range even
    // when the exact result fits (ROUND of the maximum to 4 places changes nothing): Copperfin raises it only when
    // the exact result is out of range. CEILING at the very top overflows (VFP9 wraps to a wrong value).
    {"ROUND($922337203685477.5807,4)", "Y:$922,337,203,685,477.5807"},
    {"ROUND($922337203685477.5807,2)", "Y:$922,337,203,685,477.5800"},
    {"ROUND($922337203685477.5804,3)", "Y:$922,337,203,685,477.5800"},
    {"CEILING($922337203685477.5807)", "ERR1988"},
    {"CEILING($-922337203685477.5807)", "ERR1988"},
};

// Fresh installed-VFP9 probe evidence for Numeric divisors is retained under
// tests/fixtures/vfp9-currency-mod-numeric-divisor-observation/ (#5611/#6776).
const std::vector<Row> kCurrencyModNumericRows = {
    {"MOD($0.0004,0.00014)", "Y:$0.0001"},
    {"MOD($0.0004,-0.00014)", "Y:$0.0000"},
    {"MOD($0.0001,0.00015)", "Y:$0.0001"},
    {"MOD($12.3456,2.00004)", "Y:$0.3454"},
    {"MOD($12.3456,2.00005)", "Y:$0.3453"},
    {"MOD($12.3456,2.00006)", "Y:$0.3452"},
    {"MOD($10,1E20)", "Y:$10.0000"},
    {"MOD($10,-1E20)", "ERR1988"},
    {"MOD($10,EXP(1000))", "ERR11"},
    {"MOD($10,-EXP(1000))", "ERR11"},
};

const std::vector<Row> kCurrencyModVfp9Rows = {
    {"MOD($10,1E20)", "Y:$10.0000"},
    {"MOD($10,-1E20)", "Y:$0.0000"},
    {"MOD($10,EXP(1000))", "Y:$0.0000"},
    {"MOD($10,-EXP(1000))", "Y:$0.0000"},
};

std::string run_rows(
    const fs::path &dir,
    const std::vector<Row> &rows,
    const bool exact,
    const std::string &numeric_behavior = {}) {
    std::string body = exact ? "LOCAL cOut, oEx, x\ncOut = ''\nSET DECIMALS TO 4\n" : "LOCAL cOut, oEx, x\ncOut = ''\n";
    if (!numeric_behavior.empty()) {
        body += "SET NUMERICBEHAVIOR TO " + numeric_behavior + "\n";
    }
    for (const Row &row : rows) {
        body += "TRY\n";
        body += "x = " + std::string(row.expression) + "\n";
        body += exact ? "cOut = cOut + VARTYPE(x) + ':' + TRANSFORM(x) + CHR(10)\n"
                      : "cOut = cOut + VARTYPE(x) + ':' + LTRIM(STR(x, 22, 4)) + CHR(10)\n";
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

void check_rows(
    const std::vector<Row> &rows,
    const bool exact,
    const std::string &label,
    const std::string &numeric_behavior = {}) {
    const fs::path dir = fs::temp_directory_path() / ("copperfin_currency_functions_" + label);
    std::error_code ignored;
    fs::remove_all(dir, ignored);
    const std::string output = run_rows(dir, rows, exact, numeric_behavior);
    expect(output.rfind("<incomplete", 0U) != 0U, "currency functions " + label + ": the script should complete: " + output);
    std::vector<std::string> lines;
    for (std::size_t start = 0U; start < output.size();) {
        const std::size_t end = output.find('\n', start);
        lines.push_back(output.substr(start, end == std::string::npos ? std::string::npos : end - start));
        if (end == std::string::npos) {
            break;
        }
        start = end + 1U;
    }
    expect(lines.size() >= rows.size(), "currency functions " + label + ": one result per row, got " + std::to_string(lines.size()));
    for (std::size_t index = 0U; index < rows.size() && index < lines.size(); ++index) {
        expect(lines[index] == rows[index].expected,
            "currency functions " + label + ": " + rows[index].expression + " expected [" + rows[index].expected +
                "], got [" + lines[index] + "]");
    }
    fs::remove_all(dir, ignored);
}

void test_currency_functions_match_vfp9() {
    check_rows(kRows, false, "values");
    check_rows(kBoundaryRows, true, "boundary");
    check_rows(kCurrencyModNumericRows, true, "mod_numeric_copperfin", "COPPERFIN");
    check_rows(kCurrencyModVfp9Rows, true, "mod_numeric_vfp9", "VFP9");
}

void test_currency_mod_direct_numeric_boundaries() {
    const auto callback = [](const char* mode) {
        return [mode](const std::string& setting) {
            return setting == "NUMERICBEHAVIOR" ? std::string(mode) : std::string{};
        };
    };
    const auto args_with_divisor = [](const copperfin::runtime::PrgValue& divisor) {
        return std::vector<copperfin::runtime::PrgValue>{
            copperfin::runtime::make_currency_value(100000), divisor};
    };

    const auto exact_positive = copperfin::runtime::evaluate_numeric_function(
        "mod",
        args_with_divisor(copperfin::runtime::make_uint64_value(std::numeric_limits<std::uint64_t>::max())),
        callback("COPPERFIN"));
    expect(exact_positive.has_value() && exact_positive->currency_value == 100000,
        "Currency MOD should preserve an exact positive dividend below a huge exact positive divisor");

    bool rejected = false;
    try {
        (void)copperfin::runtime::evaluate_numeric_function(
            "mod",
            args_with_divisor(copperfin::runtime::make_int64_value(std::numeric_limits<std::int64_t>::min())),
            callback("COPPERFIN"));
    } catch (const copperfin::runtime::PrgCompatibilityError& error) {
        rejected = error.error_code() == 1988;
    }
    expect(rejected, "Copperfin Currency MOD should reject an exact out-of-range negative result");

    const auto vfp_exact_negative = copperfin::runtime::evaluate_numeric_function(
        "mod",
        args_with_divisor(copperfin::runtime::make_int64_value(std::numeric_limits<std::int64_t>::min())),
        callback("VFP9"));
    expect(vfp_exact_negative.has_value() && vfp_exact_negative->currency_value == 0,
        "VFP9 Currency MOD should map an exact out-of-range negative result to integer-indefinite zero");

    const auto vfp_nan = copperfin::runtime::evaluate_numeric_function(
        "mod",
        args_with_divisor(copperfin::runtime::make_number_value(std::numeric_limits<double>::quiet_NaN())),
        callback("VFP9"));
    expect(vfp_nan.has_value() && vfp_nan->currency_value == 0,
        "VFP9 Currency MOD should map a NaN divisor to integer-indefinite zero");
}

// MOD of the stored minimum by -0.0001 is INT64_MIN % -1, signed-division overflow (a SIGFPE on common targets). The
// minimum cannot come from INSERT, so a Y table is written and its record is patched to INT64_MIN (#6039 review).
void test_mod_of_stored_minimum_does_not_overflow() {
    const fs::path dir = fs::temp_directory_path() / "copperfin_currency_mod_minimum";
    std::error_code ignored;
    fs::remove_all(dir, ignored);
    fs::create_directories(dir);
    write_text(dir / "make.prg",
        "CREATE TABLE cy (amount Y)\nINSERT INTO cy (amount) VALUES ($1)\nUSE\nRETURN\n");
    {
        auto session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options((dir / "make.prg").string(), dir.string(), false));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "currency MOD minimum: the table should be created: " + state.message);
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
    write_text(dir / "mod.prg",
        "LOCAL c\nSET DECIMALS TO 4\nUSE cy\n"
        "c = TRANSFORM(amount) + ';' + TRANSFORM(MOD(amount, $-0.0001)) + ';' + TRANSFORM(MOD(amount, -0.0001)) + ';' + "
        "TRANSFORM(MOD(amount, $0.0001))\n"
        "STRTOFILE(c, 'mod.txt')\nRETURN\n");
    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options((dir / "mod.prg").string(), dir.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "currency MOD minimum: the script should complete: " + state.message);
    expect(read_text(dir / "mod.txt") == "$-922,337,203,685,477.5808;$0.0000;$0.0000;$0.0000",
        "currency MOD minimum: got [" + read_text(dir / "mod.txt") + "]");
    fs::remove_all(dir, ignored);
}

}  // namespace

int main() {
    test_currency_functions_match_vfp9();
    test_currency_mod_direct_numeric_boundaries();
    test_mod_of_stored_minimum_does_not_overflow();
    if (const int failures = test_failures(); failures != 0) {
        std::cerr << failures << " test(s) failed\n";
        return 1;
    }
    return 0;
}
