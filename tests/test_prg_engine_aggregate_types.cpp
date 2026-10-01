// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/runtime/prg_engine.h"
#include "prg_engine_test_support.h"

#include <filesystem>
#include <functional>
#include <iterator>
#include <fstream>
#include <iostream>
#include <string>
#include <system_error>
#include <vector>

namespace {

using namespace copperfin::test_support;

namespace fs = std::filesystem;

// Governing requirement: RQ-CF-PRG-AGGREGATE-TYPES-001 (#6040, #6041).
//
// SUM, AVG, MIN and MAX keep the type of their operand. Every expectation is installed VFP9 (09.00.0000.7423, Windows
// VM COM probe 2026-10-01, retained at /home/rich/temp/vfp9-probes/currency-c25/result9.txt, result10.txt and
// result11.txt) over one cursor with a NULL row, except the rows in kExactRows, where VFP9 is off by one unit
// because its own $900719925474.0993 literal passes through a double above 2^53 scaled units (Copperfin keeps
// Currency exact there, a deliberate improvement). SUM and AVG of Currency stay Currency and are summed exactly
// (overflow is error 1988); CALCULATE and the AVERAGE command truncate the Currency average toward zero where SQL AVG
// rounds half away from zero; SUM and AVG of a Date, DateTime or Character value are error 27 "Not a numeric
// expression." in CALCULATE and the commands and error 1811 in SQL; MIN and MAX compare Date, DateTime and Character
// values and return them with their own type. NULL values are skipped. Not covered here: the typed empty-set results
// (VFP9 returns an empty Date, DateTime or Currency; Copperfin returns Numeric 0) and the SQL AVG of an Integer, which
// VFP9 truncates. The expected text is "<VARTYPE>:<formatted value>" or "ERR<number>".

struct Row {
    const char *statement;   // a PRG fragment that leaves its result in r1 (or a message in r1 on error)
    const char *expected;
};

const std::vector<Row> kRows = {
    {"CALCULATE SUM(n) TO r1\n", "N:35.7500"},
    {"CALCULATE AVG(n) TO r1\n", "N:11.9167"},
    {"CALCULATE MIN(n) TO r1\n", "N:5.0000"},
    {"CALCULATE MAX(n) TO r1\n", "N:20.2500"},
    {"SUM n TO r1\n", "N:35.7500"},
    {"AVERAGE n TO r1\n", "N:11.9167"},
    {"CALCULATE AVG(y) TO r1\n", "Y:$300,239,975,161.5331"},
    {"CALCULATE MIN(y) TO r1\n", "Y:$0.0001"},
    {"AVERAGE y TO r1\n", "Y:$300,239,975,161.5331"},
    {"CALCULATE SUM(i) TO r1\n", "N:35.0000"},
    {"CALCULATE AVG(i) TO r1\n", "N:11.6667"},
    {"CALCULATE MIN(i) TO r1\n", "N:5.0000"},
    {"CALCULATE MAX(i) TO r1\n", "N:20.0000"},
    {"SUM i TO r1\n", "N:35.0000"},
    {"AVERAGE i TO r1\n", "N:11.6667"},
    {"CALCULATE SUM(b) TO r1\n", "N:35.7500"},
    {"CALCULATE AVG(b) TO r1\n", "N:11.9167"},
    {"CALCULATE MIN(b) TO r1\n", "N:5.0000"},
    {"CALCULATE MAX(b) TO r1\n", "N:20.2500"},
    {"SUM b TO r1\n", "N:35.7500"},
    {"AVERAGE b TO r1\n", "N:11.9167"},
    {"CALCULATE SUM(f) TO r1\n", "N:35.7500"},
    {"CALCULATE AVG(f) TO r1\n", "N:11.9167"},
    {"CALCULATE MIN(f) TO r1\n", "N:5.0000"},
    {"CALCULATE MAX(f) TO r1\n", "N:20.2500"},
    {"SUM f TO r1\n", "N:35.7500"},
    {"AVERAGE f TO r1\n", "N:11.9167"},
    {"CALCULATE MIN(d) TO r1\n", "D:20191231"},
    {"CALCULATE MAX(d) TO r1\n", "D:20240506"},
    {"CALCULATE SUM(d) TO r1\n", "ERR27"},
    {"CALCULATE AVG(d) TO r1\n", "ERR27"},
    {"CALCULATE MIN(t) TO r1\n", "T:20191231235959"},
    {"CALCULATE MAX(t) TO r1\n", "T:20240506121314"},
    {"CALCULATE SUM(t) TO r1\n", "ERR27"},
    {"CALCULATE AVG(t) TO r1\n", "ERR27"},
    {"CALCULATE MIN(c) TO r1\n", "C:apple"},
    {"CALCULATE MAX(c) TO r1\n", "C:cherry"},
    {"CALCULATE SUM(c) TO r1\n", "ERR27"},
    {"CALCULATE AVG(c) TO r1\n", "ERR27"},
    {"SELECT SUM(n) FROM ag INTO ARRAY aQ\nr1 = aQ(1)\n", "N:35.7500"},
    {"SELECT AVG(n) FROM ag INTO ARRAY aQ\nr1 = aQ(1)\n", "N:11.9167"},
    {"SELECT MIN(n) FROM ag INTO ARRAY aQ\nr1 = aQ(1)\n", "N:5.0000"},
    {"SELECT MAX(n) FROM ag INTO ARRAY aQ\nr1 = aQ(1)\n", "N:20.2500"},
    {"SELECT MIN(y) FROM ag INTO ARRAY aQ\nr1 = aQ(1)\n", "Y:$0.0001"},
    {"SELECT SUM(i) FROM ag INTO ARRAY aQ\nr1 = aQ(1)\n", "N:35.0000"},
    {"SELECT MIN(i) FROM ag INTO ARRAY aQ\nr1 = aQ(1)\n", "N:5.0000"},
    {"SELECT MAX(i) FROM ag INTO ARRAY aQ\nr1 = aQ(1)\n", "N:20.0000"},
    {"SELECT SUM(d) FROM ag INTO ARRAY aQ\nr1 = aQ(1)\n", "ERR1811"},
    {"SELECT AVG(d) FROM ag INTO ARRAY aQ\nr1 = aQ(1)\n", "ERR1811"},
    {"SELECT MIN(d) FROM ag INTO ARRAY aQ\nr1 = aQ(1)\n", "D:20191231"},
    {"SELECT MAX(d) FROM ag INTO ARRAY aQ\nr1 = aQ(1)\n", "D:20240506"},
    {"SELECT SUM(t) FROM ag INTO ARRAY aQ\nr1 = aQ(1)\n", "ERR1811"},
    {"SELECT AVG(t) FROM ag INTO ARRAY aQ\nr1 = aQ(1)\n", "ERR1811"},
    {"SELECT MIN(t) FROM ag INTO ARRAY aQ\nr1 = aQ(1)\n", "T:20191231235959"},
    {"SELECT MAX(t) FROM ag INTO ARRAY aQ\nr1 = aQ(1)\n", "T:20240506121314"},
    {"SELECT SUM(c) FROM ag INTO ARRAY aQ\nr1 = aQ(1)\n", "ERR1811"},
    {"SELECT AVG(c) FROM ag INTO ARRAY aQ\nr1 = aQ(1)\n", "ERR1811"},
    {"SELECT MIN(c) FROM ag INTO ARRAY aQ\nr1 = aQ(1)\n", "C:apple"},
    {"SELECT MAX(c) FROM ag INTO ARRAY aQ\nr1 = aQ(1)\n", "C:cherry"},
    {"SELECT g, MIN(y) FROM ag GROUP BY g INTO ARRAY aQ\nr1 = fmt(aQ(1,2)) + '|' + fmt(aQ(2,2))\nlRaw = .T.\n", "Y:$10.5000|Y:$0.0001"},
    {"SELECT g, SUM(d) FROM ag GROUP BY g INTO ARRAY aQ\nr1 = aQ(1,2)\n", "ERR1811"},
    {"SELECT g, MIN(d) FROM ag GROUP BY g INTO ARRAY aQ\nr1 = fmt(aQ(1,2)) + '|' + fmt(aQ(2,2))\nlRaw = .T.\n", "D:20200102|D:20191231"},
    {"SELECT g, MAX(d) FROM ag GROUP BY g INTO ARRAY aQ\nr1 = fmt(aQ(1,2)) + '|' + fmt(aQ(2,2))\nlRaw = .T.\n", "D:20240506|D:20191231"},
    {"SELECT g, SUM(c) FROM ag GROUP BY g INTO ARRAY aQ\nr1 = aQ(1,2)\n", "ERR1811"},
    {"SELECT g, MIN(c) FROM ag GROUP BY g INTO ARRAY aQ\nr1 = fmt(aQ(1,2)) + '|' + fmt(aQ(2,2))\nlRaw = .T.\n", "C:apple|C:cherry"},
    {"SELECT g, MAX(c) FROM ag GROUP BY g INTO ARRAY aQ\nr1 = fmt(aQ(1,2)) + '|' + fmt(aQ(2,2))\nlRaw = .T.\n", "C:banana|C:cherry"},
};

const std::vector<Row> kExactRows = {
    {"CALCULATE SUM(y) TO r1\n", "Y:$900,719,925,484.5994"},
    {"CALCULATE MAX(y) TO r1\n", "Y:$900,719,925,474.0993"},
    {"SUM y TO r1\n", "Y:$900,719,925,484.5994"},
    {"SELECT SUM(y) FROM ag INTO ARRAY aQ\nr1 = aQ(1)\n", "Y:$900,719,925,484.5994"},
    {"SELECT AVG(y) FROM ag INTO ARRAY aQ\nr1 = aQ(1)\n", "Y:$300,239,975,161.5331"},
    {"SELECT MAX(y) FROM ag INTO ARRAY aQ\nr1 = aQ(1)\n", "Y:$900,719,925,474.0993"},
    {"SELECT g, SUM(y) FROM ag GROUP BY g INTO ARRAY aQ\nr1 = fmt(aQ(1,2)) + '|' + fmt(aQ(2,2))\nlRaw = .T.\n", "Y:$900,719,925,484.5993|Y:$0.0001"},
    {"SELECT g, MAX(y) FROM ag GROUP BY g INTO ARRAY aQ\nr1 = fmt(aQ(1,2)) + '|' + fmt(aQ(2,2))\nlRaw = .T.\n", "Y:$900,719,925,474.0993|Y:$0.0001"},
};

const std::vector<Row> kOverflowRows = {
    {"CALCULATE SUM(y) TO r1\n", "ERR1988"},
    {"SUM y TO r1\n", "ERR1988"},
    {"CALCULATE AVG(y) TO r1\n", "ERR1988"},
};

struct AverageRow {
    const char *values;
    const char *calculate;
    const char *sql;
};

const std::vector<AverageRow> kAverageRows = {
    {"$0.0001,$0.0002,$0.0002", "$0.0001", "$0.0002"},
    {"$-0.0001,$-0.0002,$-0.0002", "$-0.0001", "$-0.0002"},
    {"$0.0001,$0.0001,$0.0001,$0.0002", "$0.0001", "$0.0001"},
    {"$0.0001,$0.0002", "$0.0001", "$0.0002"},
};

const char *kFormatter =
    "FUNCTION fmt(x)\n"
    "DO CASE\n"
    "CASE VARTYPE(x) = 'D'\nRETURN 'D:' + DTOC(x, 1)\n"
    "CASE VARTYPE(x) = 'T'\nRETURN 'T:' + TTOC(x, 1)\n"
    "CASE VARTYPE(x) = 'C'\nRETURN 'C:' + RTRIM(x)\n"
    "CASE VARTYPE(x) = 'N'\nRETURN 'N:' + LTRIM(STR(x, 22, 4))\n"
    "OTHERWISE\nRETURN VARTYPE(x) + ':' + TRANSFORM(x)\n"
    "ENDCASE\n";

std::string row_script(const std::vector<Row> &rows, const std::string &setup) {
    std::string body = "LOCAL cOut, oEx, r1, lRaw\nDIMENSION aQ(1)\ncOut = ''\nSET DECIMALS TO 4\n" + setup;
    for (const Row &row : rows) {
        body += "SELECT ag\nr1 = ''\nlRaw = .F.\nTRY\n";
        body += row.statement;
        body += "cOut = cOut + IIF(lRaw, r1, fmt(r1)) + CHR(10)\n";
        body += "CATCH TO oEx\n";
        body += "cOut = cOut + 'ERR' + ALLTRIM(STR(oEx.ErrorNo)) + IIF(GETENV('CF_DEBUG') = '1', ' ' + oEx.Message, '') + CHR(10)\n";
        body += "ENDTRY\n";
    }
    body += "STRTOFILE(cOut, 'results.txt')\nRETURN\n";
    return body + kFormatter;
}

const char *kStandardSetup =
    "CREATE CURSOR ag (n N(12,4) NULL, y Y NULL, i I NULL, b B(4) NULL, f F(12,4) NULL, d D NULL, t T NULL, "
    "c C(10) NULL, g C(1))\n"
    "INSERT INTO ag (n,y,i,b,f,d,t,c,g) VALUES (10.5, $10.5, 10, 10.5, 10.5, {^2020-01-02}, "
    "{^2020-01-02 03:04:05}, 'banana', 'a')\n"
    "INSERT INTO ag (n,y,i,b,f,d,t,c,g) VALUES (20.25, $900719925474.0993, 20, 20.25, 20.25, {^2024-05-06}, "
    "{^2024-05-06 12:13:14}, 'apple', 'a')\n"
    "INSERT INTO ag (n,y,i,b,f,d,t,c,g) VALUES (5, $0.0001, 5, 5, 5, {^2019-12-31}, {^2019-12-31 23:59:59}, "
    "'cherry', 'b')\n"
    "INSERT INTO ag (n,y,i,b,f,d,t,c,g) VALUES (.NULL., .NULL., .NULL., .NULL., .NULL., .NULL., .NULL., .NULL., 'b')\n";

std::vector<std::string> run_script(
    const fs::path &dir, const std::string &script, const std::function<void(const fs::path &)> &before = {}) {
    std::error_code ignored;
    fs::remove_all(dir, ignored);
    fs::create_directories(dir / "script");
    if (before) {
        before(dir);
    }
    write_text(dir / "script" / "rows.prg", script);
    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options((dir / "script" / "rows.prg").string(), dir.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    if (!state.completed) {
        return {"<incomplete: " + state.message + ">"};
    }
    const std::string output = read_text(dir / "results.txt");
    std::vector<std::string> lines;
    for (std::size_t start = 0U; start < output.size();) {
        const std::size_t end = output.find('\n', start);
        lines.push_back(output.substr(start, end == std::string::npos ? std::string::npos : end - start));
        if (end == std::string::npos) {
            break;
        }
        start = end + 1U;
    }
    return lines;
}

void check_rows(const std::vector<Row> &rows, const std::string &setup, const std::string &label,
    const std::function<void(const fs::path &)> &before = {}) {
    const fs::path dir = fs::temp_directory_path() / ("copperfin_aggregate_types_" + label);
    const std::vector<std::string> lines = run_script(dir, row_script(rows, setup), before);
    expect(lines.empty() || lines[0].rfind("<incomplete", 0U) != 0U,
        "aggregate types " + label + ": the script should complete: " + (lines.empty() ? "" : lines[0]));
    expect(lines.size() >= rows.size(), "aggregate types " + label + ": one result per row, got " +
        std::to_string(lines.size()));
    for (std::size_t index = 0U; index < rows.size() && index < lines.size(); ++index) {
        expect(lines[index] == rows[index].expected,
            "aggregate types " + label + ": " + rows[index].statement + " expected [" + rows[index].expected +
                "], got [" + lines[index] + "]");
    }
    std::error_code ignored;
    fs::remove_all(dir, ignored);
}

void test_aggregates_keep_operand_types() {
    check_rows(kRows, kStandardSetup, "vfp9");
    check_rows(kExactRows, kStandardSetup, "exact");
}

// A real table written by VFP9 (09.00.0000.7423: CREATE TABLE dt (d D NULL, t T NULL, c C(10) NULL, y Y NULL,
// n N(12,4) NULL) and four INSERTs, the last all NULL; retained at ~/temp/vfp9-probes/currency-c25/dt.dbf), so the
// typed extrema are decoded from bytes VFP9 wrote and not from Copperfin's own writer (#6755 review). The expected
// values are VFP9's own CALCULATE results over the same table (result14.txt).
const unsigned char kVfpTemporalTable[681] = {
    0x30, 0x1a, 0x0a, 0x01, 0x04, 0x00, 0x00, 0x00, 0xe8, 0x01, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00,
    0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x44, 0x01, 0x00, 0x00, 0x00,
    0x08, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x54, 0x09, 0x00, 0x00, 0x00,
    0x08, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x43, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x43, 0x11, 0x00, 0x00, 0x00,
    0x0a, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x59, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x59, 0x1b, 0x00, 0x00, 0x00,
    0x08, 0x04, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x4e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4e, 0x23, 0x00, 0x00, 0x00,
    0x0c, 0x04, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x5f, 0x4e, 0x75, 0x6c, 0x6c, 0x46, 0x6c, 0x61, 0x67, 0x73, 0x00, 0x30, 0x2f, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
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
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x32, 0x30, 0x32, 0x30, 0x30, 0x31, 0x30,
    0x32, 0xe3, 0x84, 0x25, 0x00, 0x87, 0x88, 0xa8, 0x00, 0x62, 0x61, 0x6e, 0x61, 0x6e, 0x61, 0x20,
    0x20, 0x20, 0x20, 0x28, 0x9a, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x31, 0x30, 0x2e, 0x35, 0x30, 0x30, 0x30, 0x00, 0x20, 0x32, 0x30, 0x32, 0x34, 0x30, 0x35, 0x30,
    0x36, 0x15, 0x8b, 0x25, 0x00, 0x8f, 0x4b, 0x9f, 0x02, 0x61, 0x70, 0x70, 0x6c, 0x65, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x32, 0x30, 0x2e, 0x32, 0x35, 0x30, 0x30, 0x00, 0x20, 0x32, 0x30, 0x31, 0x39, 0x31, 0x32, 0x33,
    0x31, 0xe1, 0x84, 0x25, 0x00, 0x18, 0x58, 0x26, 0x05, 0x63, 0x68, 0x65, 0x72, 0x72, 0x79, 0x20,
    0x20, 0x20, 0x20, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x35, 0x2e, 0x30, 0x30, 0x30, 0x30, 0x00, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x1f, 0x1a,
};

void test_vfp_written_table_extrema() {
    const std::vector<Row> rows = {
        {"CALCULATE MIN(d) TO r1\n", "D:20191231"},
        {"CALCULATE MAX(d) TO r1\n", "D:20240506"},
        {"CALCULATE MIN(t) TO r1\n", "T:20191231235959"},
        {"CALCULATE MAX(t) TO r1\n", "T:20240506121314"},
        {"CALCULATE MIN(c) TO r1\n", "C:apple"},
        {"CALCULATE MAX(c) TO r1\n", "C:cherry"},
        {"CALCULATE SUM(n) TO r1\n", "N:35.7500"},
        {"CALCULATE SUM(d) TO r1\n", "ERR27"},
    };
    check_rows(rows, "USE dt ALIAS ag\n", "vfp_table", [](const fs::path &dir) {
        std::ofstream out(dir / "dt.dbf", std::ios::binary);
        out.write(reinterpret_cast<const char *>(kVfpTemporalTable), sizeof(kVfpTemporalTable));
    });
}

// MIN and MAX of an expression that mixes Character with Numeric, Date or Logical values is error 107, while a Numeric
// next to a Currency and a Date next to a DateTime compare (installed VFP9, result14.txt); SUM of mixed values follows
// the SUM rules.
void test_mixed_domain_aggregates() {
    const std::vector<Row> rows = {
        {"CALCULATE MIN(IIF(RECNO()=1,1,'a')) TO r1\n", "ERR107"},
        {"CALCULATE MAX(IIF(RECNO()=1,{^2020-01-01},'a')) TO r1\n", "ERR107"},
        {"CALCULATE MAX(IIF(RECNO()=1,.T.,1)) TO r1\n", "ERR107"},
        {"CALCULATE MIN(IIF(RECNO()=1,1,$2)) TO r1\n", "N:1.0000"},
        {"CALCULATE MIN(IIF(RECNO()=1,{^2020-01-01},{^2020-01-01 01:00:00})) TO r1\n", "D:20200101"},
        {"CALCULATE SUM(IIF(RECNO()=1,1,'a')) TO r1\n", "ERR27"},
        {"CALCULATE SUM(IIF(RECNO()=1,1,$2)) TO r1\n", "N:7.0000"},
    };
    check_rows(rows, kStandardSetup, "mixed");
}

// An aggregate that raises an error must leave the work area on the caller's record, with the source alias unchanged:
// statement atomicity for CALCULATE, the commands and SQL, plain and grouped (#6755 review).
void test_aggregate_errors_leave_the_work_area_alone() {
    const std::vector<Row> rows = {
        {"GO 2\nTRY\nCALCULATE SUM(c) TO r1\nCATCH\nENDTRY\nr1 = RECNO()\n", "N:2.0000"},
        {"GO 3\nTRY\nSUM d TO r1\nCATCH\nENDTRY\nr1 = RECNO()\n", "N:3.0000"},
        {"GO 2\nTRY\nCALCULATE MIN(IIF(RECNO()=1,1,'a')) TO r1\nCATCH\nENDTRY\nr1 = RECNO()\n", "N:2.0000"},
        {"GO 2\nTRY\nSELECT SUM(c) FROM ag INTO ARRAY aQ\nCATCH\nENDTRY\nr1 = RECNO()\n", "N:2.0000"},
        {"GO 3\nTRY\nSELECT g, SUM(c) FROM ag GROUP BY g INTO ARRAY aQ\nCATCH\nENDTRY\nr1 = RECNO()\n", "N:3.0000"},
        {"GO 2\nTRY\nSELECT g, SUM(c) FROM ag GROUP BY g INTO ARRAY aQ\nCATCH\nENDTRY\nr1 = UPPER(ALIAS())\n", "C:AG"},
    };
    check_rows(rows, kStandardSetup, "atomic");
}

// A Character value that merely looks like the asterisk run of an overflowed numeric field is still a Character: MIN
// and MAX return it and SUM and AVG are error 27 (#6755 review). The numeric-field marker itself is skipped, covered by
// test_aggregate_helpers_tolerate_non_numeric_field_text.
void test_all_asterisk_character_values_are_characters() {
    const std::string setup =
        "CREATE CURSOR ag (c C(3))\nINSERT INTO ag VALUES ('***')\nINSERT INTO ag VALUES ('abc')\n";
    const std::vector<Row> rows = {
        {"CALCULATE MIN(c) TO r1\n", "C:***"},
        {"CALCULATE MAX(c) TO r1\n", "C:abc"},
        {"CALCULATE SUM(c) TO r1\n", "ERR27"},
        {"SELECT AVG(c) FROM ag INTO ARRAY aQ\nr1 = aQ(1)\n", "ERR1811"},
    };
    check_rows(rows, setup, "asterisks");
}

void test_currency_aggregate_overflow() {
    const std::string setup =
        "CREATE CURSOR ag (y Y)\nINSERT INTO ag VALUES ($922337203685477.5807)\nINSERT INTO ag VALUES ($1)\n";
    check_rows(kOverflowRows, setup, "overflow");
}

void test_currency_average_rounding() {
    int index = 0;
    for (const AverageRow &entry : kAverageRows) {
        std::string setup = "CREATE CURSOR ag (y Y)\n";
        const std::string values = entry.values;
        for (std::size_t start = 0U; start <= values.size();) {
            const std::size_t end = values.find(',', start);
            setup += "INSERT INTO ag VALUES (" +
                     values.substr(start, end == std::string::npos ? std::string::npos : end - start) + ")\n";
            if (end == std::string::npos) {
                break;
            }
            start = end + 1U;
        }
        // CALCULATE AVG and the AVERAGE command truncate toward zero; SQL AVG rounds half away from zero.
        const std::string calculate = std::string("Y:") + entry.calculate;
        const std::string sql = std::string("Y:") + entry.sql;
        const std::vector<Row> checked = {
            {"CALCULATE AVG(y) TO r1\n", calculate.c_str()},
            {"AVERAGE y TO r1\n", calculate.c_str()},
            {"SELECT AVG(y) FROM ag INTO ARRAY aQ\nr1 = aQ(1)\n", sql.c_str()}};
        check_rows(checked, setup, "average_" + std::to_string(index++));
    }
}

}  // namespace

int main() {
    test_aggregates_keep_operand_types();
    test_vfp_written_table_extrema();
    test_mixed_domain_aggregates();
    test_aggregate_errors_leave_the_work_area_alone();
    test_all_asterisk_character_values_are_characters();
    test_currency_aggregate_overflow();
    test_currency_average_rounding();
    if (const int failures = test_failures(); failures != 0) {
        std::cerr << failures << " test(s) failed\n";
        return 1;
    }
    return 0;
}
