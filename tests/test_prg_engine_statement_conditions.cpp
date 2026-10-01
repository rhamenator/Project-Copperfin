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

// Governing requirement: RQ-CF-PRG-STATEMENT-CONDITION-TYPES-001 (#5939).
//
// Installed VFP9 (09.00.0000.7423, Windows VM COM probe 2026-10-01, retained at
// /home/rich/temp/vfp9-probes/null-semantics-c24/vfp9-result-flow-msgs.txt) accepts only a Logical value or
// NULL (false) as the condition of a statement or clause and raises a different error per statement: IF 9,
// DO CASE 10, a FOR or WHILE clause (DO WHILE, LOCATE, COUNT, SCAN, REPLACE FOR, DELETE FOR, ...) 1127,
// SET FILTER 37 and SQL WHERE 1833. ELSEIF is the one place VFP9 accepts any value.

struct Row {
    const char *name;
    const char *statement;   // a statement block that ends by assigning x = 'ran'
    const char *expected;    // "ok" or "ERR<number>"
};

const std::vector<Row> kRows = {
    {"IF 1", "IF 1\nx = 'ran'\nENDIF", "ERR9"},
    {"IF string", "IF 'a'\nx = 'ran'\nENDIF", "ERR9"},
    {"IF 0", "IF 0\nx = 'ran'\nENDIF", "ERR9"},
    {"IF date", "IF DATE(2026,1,1)\nx = 'ran'\nENDIF", "ERR9"},
    {"IF .NULL.", "IF .NULL.\nx = 'ran'\nENDIF", "ok"},
    {"IF .T.", "IF .T.\nx = 'ran'\nENDIF", "ok"},
    {"ELSEIF 1", "IF .F.\nx = 1\nELSEIF 1\nx = 'ran'\nENDIF", "ok"},
    {"DO CASE 1", "DO CASE\nCASE 1\nx = 'ran'\nENDCASE", "ERR10"},
    {"DO CASE string", "DO CASE\nCASE 'a'\nx = 'ran'\nENDCASE", "ERR10"},
    {"DO CASE .T.", "DO CASE\nCASE .T.\nx = 'ran'\nENDCASE", "ok"},
    {"DO WHILE 1", "DO WHILE 1\nEXIT\nENDDO", "ERR1127"},
    {"DO WHILE .NULL.", "DO WHILE .NULL.\nEXIT\nENDDO", "ok"},
    {"LOCATE 1", "LOCATE FOR 1", "ERR1127"},
    {"LOCATE string", "LOCATE FOR 'a'", "ERR1127"},
    {"COUNT 1", "COUNT FOR 1 TO n", "ERR1127"},
    {"COUNT field", "COUNT FOR ID TO n", "ERR1127"},
    {"SCAN 1", "SCAN FOR 1\nENDSCAN", "ERR1127"},
    {"REPLACE FOR 1", "REPLACE ID WITH 2 FOR 1", "ERR1127"},
    {"DELETE FOR 1", "DELETE FOR 1", "ERR1127"},
    {"LOCATE .T.", "LOCATE FOR .T.", "ok"},
    {"SQL WHERE 1", "SELECT * FROM t WHERE 1 INTO ARRAY aq", "ERR1833"},
    {"SQL WHERE field", "SELECT * FROM t WHERE ID INTO ARRAY aq", "ERR1833"},
    {"SET FILTER 1", "SET FILTER TO 1\nGO TOP", "ERR37"},
    {"SET FILTER .T.", "SET FILTER TO .T.\nGO TOP", "ok"},
};

std::string run_rows(const fs::path &dir) {
    std::string body = "LOCAL cOut, oEx, x, n\ncOut = ''\nCREATE CURSOR t (ID N(3))\nAPPEND BLANK\nREPLACE ID WITH 1\nAPPEND BLANK\nREPLACE ID WITH 2\n";
    for (const Row &row : kRows) {
        body += "TRY\n";
        body += std::string(row.statement) + "\n";
        body += "cOut = cOut + 'ok' + CHR(10)\n";
        body += "CATCH TO oEx\n";
        body += "cOut = cOut + 'ERR' + ALLTRIM(STR(oEx.ErrorNo)) + CHR(10)\n";
        body += "ENDTRY\n";
        body += "SET FILTER TO\nSELECT t\n";
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

void test_statement_conditions_match_vfp9() {
    const fs::path dir = fs::temp_directory_path() / "copperfin_statement_condition_types";
    std::error_code ignored;
    fs::remove_all(dir, ignored);
    const std::string output = run_rows(dir);
    expect(output.rfind("<incomplete", 0U) != 0U, "statement conditions: the script should complete: " + output);
    std::vector<std::string> lines;
    for (std::size_t start = 0U; start < output.size();) {
        const std::size_t end = output.find('\n', start);
        lines.push_back(output.substr(start, end == std::string::npos ? std::string::npos : end - start));
        if (end == std::string::npos) {
            break;
        }
        start = end + 1U;
    }
    expect(lines.size() >= kRows.size(), "statement conditions: one result per row, got " + std::to_string(lines.size()));
    for (std::size_t index = 0U; index < kRows.size() && index < lines.size(); ++index) {
        expect(lines[index] == kRows[index].expected,
            std::string("statement conditions: ") + kRows[index].name + " expected [" + kRows[index].expected +
                "], got [" + lines[index] + "]");
    }
    fs::remove_all(dir, ignored);
}

}  // namespace

int main() {
    test_statement_conditions_match_vfp9();
    if (const int failures = test_failures(); failures != 0) {
        std::cerr << failures << " test(s) failed\n";
        return 1;
    }
    return 0;
}
