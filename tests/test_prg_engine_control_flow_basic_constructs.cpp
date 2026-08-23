// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_prg_engine_control_flow_support.h"

namespace cf_test_prg_engine_control_flow {
void test_command_keyword_scanner_ignores_nested_text() {
    using copperfin::runtime::extract_command_clause;
    using copperfin::runtime::find_keyword_top_level;
    using copperfin::runtime::split_csv_like;

    const std::string quoted_to = "VALUE \"not the TO keyword\" TO cTarget";
    const std::size_t quoted_to_position = find_keyword_top_level(quoted_to, "TO");
    expect(quoted_to_position == quoted_to.rfind("TO"),
           "top-level keyword scanner should ignore double-quoted keyword text");

    const std::string bracketed_in = "FOR aValues[ASCAN(aWords, 'IN')] IN People";
    const std::size_t bracketed_in_position = find_keyword_top_level(bracketed_in, "IN");
    expect(bracketed_in_position == bracketed_in.rfind("IN"),
           "top-level keyword scanner should ignore bracketed keyword text");

    const std::string braced_to = "VALUE {|x| x = TO} TO cBlock";
    const std::size_t braced_to_position = find_keyword_top_level(braced_to, "TO");
    expect(braced_to_position == braced_to.rfind("TO"),
           "top-level keyword scanner should ignore braced keyword text");

    const std::string clause_text = "TO \"literal IN value\" IN WorkArea";
    expect(extract_command_clause(clause_text, "TO", {"IN"}) == "\"literal IN value\"",
           "clause extraction should ignore stop keywords inside double-quoted strings");

    const std::vector<std::string> parts = split_csv_like("first, {|x| x, y}, second[1,2]");
    expect(parts.size() == 3U,
           "CSV-like splitter should keep commas inside braced blocks and bracketed expressions together");
}

void test_do_while_and_loop_control_flow() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_do_while";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 21}, {"BRAVO", 28}, {"CHARLIE", 33}, {"DELTA", 44}});

    const fs::path main_path = temp_root / "control.prg";
    write_text(
        main_path,
        "nWhile = 0\n"
        "i = 0\n"
        "DO WHILE i < 5\n"
        "    i = i + 1\n"
        "    IF i = 2\n"
        "        LOOP\n"
        "    ENDIF\n"
        "    nWhile = nWhile + i\n"
        "    IF i = 4\n"
        "        EXIT\n"
        "    ENDIF\n"
        "ENDDO\n"
        "nNested = 0\n"
        "outer = 0\n"
        "DO WHILE outer < 2\n"
        "    outer = outer + 1\n"
        "    inner = 0\n"
        "    DO WHILE inner < 3\n"
        "        inner = inner + 1\n"
        "        IF inner = 2\n"
        "            LOOP\n"
        "        ENDIF\n"
        "        nNested = nNested + 1\n"
        "    ENDDO\n"
        "ENDDO\n"
        "nFor = 0\n"
        "FOR j = 1 TO 5\n"
        "    IF j = 2\n"
        "        LOOP\n"
        "    ENDIF\n"
        "    nFor = nFor + j\n"
        "    IF j = 4\n"
        "        EXIT\n"
        "    ENDIF\n"
        "ENDFOR\n"
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SELECT People\n"
        "nScan = 0\n"
        "SCAN\n"
        "    IF NAME = 'BRAVO'\n"
        "        LOOP\n"
        "    ENDIF\n"
        "    nScan = nScan + 1\n"
        "    IF NAME = 'CHARLIE'\n"
        "        EXIT\n"
        "    ENDIF\n"
        "ENDSCAN\n"
        "cAfterScan = NAME\n"
        "nAfterWhileIndex = i\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "DO WHILE/loop control script should complete");

    const auto while_total = state.globals.find("nwhile");
    const auto nested_total = state.globals.find("nnested");
    const auto for_total = state.globals.find("nfor");
    const auto scan_total = state.globals.find("nscan");
    const auto after_scan = state.globals.find("cafterscan");
    const auto after_while_index = state.globals.find("nafterwhileindex");

    expect(while_total != state.globals.end(), "DO WHILE should leave its accumulator in globals");
    expect(nested_total != state.globals.end(), "nested DO WHILE loops should leave their accumulator in globals");
    expect(for_total != state.globals.end(), "FOR with LOOP/EXIT should leave its accumulator in globals");
    expect(scan_total != state.globals.end(), "SCAN with LOOP/EXIT should leave its accumulator in globals");
    expect(after_scan != state.globals.end(), "SCAN EXIT should leave the current record available");
    expect(after_while_index != state.globals.end(), "DO WHILE EXIT should preserve the exiting iteration state");

    if (while_total != state.globals.end()) {
        expect(copperfin::runtime::format_value(while_total->second) == "8", "DO WHILE should honor LOOP and EXIT");
    }
    if (nested_total != state.globals.end()) {
        expect(copperfin::runtime::format_value(nested_total->second) == "4", "nested DO WHILE loops should reevaluate each loop independently");
    }
    if (for_total != state.globals.end()) {
        expect(copperfin::runtime::format_value(for_total->second) == "8", "LOOP and EXIT should apply to FOR loops");
    }
    if (scan_total != state.globals.end()) {
        expect(copperfin::runtime::format_value(scan_total->second) == "2", "LOOP and EXIT should apply to SCAN loops");
    }
    if (after_scan != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_scan->second) == "CHARLIE", "EXIT inside SCAN should leave the cursor on the exiting record");
    }
    if (after_while_index != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_while_index->second) == "4", "EXIT inside DO WHILE should leave the current iteration state intact");
    }

    fs::remove_all(temp_root, ignored);
}

void test_logical_operators_drive_control_flow() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_logical_control_flow";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "logical_flow.prg";
    write_text(
        main_path,
        "cIfBranch = ''\n"
        "cNotBranch = ''\n"
        "nLoopCount = 0\n"
        "nOrCount = 0\n"
        "IF (1 = 1) AND (2 = 3)\n"
        "    cIfBranch = 'wrong-true'\n"
        "ELSE\n"
        "    cIfBranch = 'correct-false'\n"
        "ENDIF\n"
        "IF NOT (1 = 2)\n"
        "    cNotBranch = 'keyword-not'\n"
        "ENDIF\n"
        "IF .NOT. (2 = 2)\n"
        "    cNotBranch = 'wrong-dotted-not'\n"
        "ENDIF\n"
        "IF 'ERR' $ 'FATAL ERR LINE'\n"
        "    cNotBranch = cNotBranch + '|contains'\n"
        "ENDIF\n"
        "IF 1 # 2\n"
        "    cNotBranch = cNotBranch + '|hash-ne'\n"
        "ENDIF\n"
        "IF 1 != 2\n"
        "    cNotBranch = cNotBranch + '|bang-ne'\n"
        "ENDIF\n"
        "IF 2 ^ 3 = 8\n"
        "    cNotBranch = cNotBranch + '|pow-caret'\n"
        "ENDIF\n"
        "IF 2 ** 3 = 8\n"
        "    cNotBranch = cNotBranch + '|pow-starstar'\n"
        "ENDIF\n"
        "DO WHILE nLoopCount < 3 AND .F.\n"
        "    nLoopCount = nLoopCount + 1\n"
        "ENDDO\n"
        "DO WHILE nOrCount < 2 OR .F.\n"
        "    nOrCount = nOrCount + 1\n"
        "ENDDO\n"
        "lAndGuard = .F. AND (1 / 0)\n"
        "lOrGuard = .T. OR (1 / 0)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "logical operator control-flow script should complete");

    const auto if_branch = state.globals.find("cifbranch");
    const auto not_branch = state.globals.find("cnotbranch");
    const auto loop_count = state.globals.find("nloopcount");
    const auto or_count = state.globals.find("norcount");
    const auto and_guard = state.globals.find("landguard");
    const auto or_guard = state.globals.find("lorguard");

    expect(if_branch != state.globals.end(), "compound IF should assign a branch marker");
    expect(not_branch != state.globals.end(), "keyword NOT should assign a branch marker");
    expect(loop_count != state.globals.end(), "compound AND in DO WHILE should leave its loop counter");
    expect(or_count != state.globals.end(), "compound OR in DO WHILE should leave its loop counter");
    expect(and_guard != state.globals.end(), "short-circuited AND assignment should complete");
    expect(or_guard != state.globals.end(), "short-circuited OR assignment should complete");

    if (if_branch != state.globals.end()) {
        expect(copperfin::runtime::format_value(if_branch->second) == "correct-false",
               "compound IF should evaluate the full AND expression");
    }
    if (not_branch != state.globals.end()) {
        expect(copperfin::runtime::format_value(not_branch->second) == "keyword-not|contains|hash-ne|bang-ne|pow-caret|pow-starstar",
               "NOT/.NOT., $, #, !=, ^, and ** should behave correctly in IF predicates");
    }
    if (loop_count != state.globals.end()) {
        expect(copperfin::runtime::format_value(loop_count->second) == "0",
               "DO WHILE with false AND tail should not execute its body");
    }
    if (or_count != state.globals.end()) {
        expect(copperfin::runtime::format_value(or_count->second) == "2",
               "DO WHILE with OR tail should reevaluate both clauses until the left side becomes false");
    }
    if (and_guard != state.globals.end()) {
        expect(copperfin::runtime::format_value(and_guard->second) == "false",
               "AND should short-circuit a false left operand");
    }
    if (or_guard != state.globals.end()) {
        expect(copperfin::runtime::format_value(or_guard->second) == "true",
               "OR should short-circuit a true left operand");
    }

    fs::remove_all(temp_root, ignored);
}

void test_do_case_control_flow() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_do_case";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}, {"DELTA", 40}});

    const fs::path main_path = temp_root / "do_case.prg";
    write_text(
        main_path,
        "nValue = 2\n"
        "cBranch = ''\n"
        "DO CASE\n"
        "    CASE nValue = 1\n"
        "        cBranch = 'ONE'\n"
        "    CASE nValue = 2\n"
        "        cBranch = 'TWO'\n"
        "    OTHERWISE\n"
        "        cBranch = 'OTHER'\n"
        "ENDCASE\n"
        "nNoMatch = 0\n"
        "DO CASE\n"
        "    CASE .F.\n"
        "        nNoMatch = 1\n"
        "ENDCASE\n"
        "cNested = ''\n"
        "DO CASE\n"
        "    CASE .T.\n"
        "        DO CASE\n"
        "            CASE 1 = 2\n"
        "                cNested = 'BAD'\n"
        "            CASE 2 = 2\n"
        "                cNested = 'INNER'\n"
        "            OTHERWISE\n"
        "                cNested = 'MISS'\n"
        "        ENDCASE\n"
        "    OTHERWISE\n"
        "        cNested = 'OUTER'\n"
        "ENDCASE\n"
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SELECT People\n"
        "nTagged = 0\n"
        "SCAN\n"
        "    DO CASE\n"
        "        CASE AGE < 20\n"
        "            cTag = 'YOUNG'\n"
        "        CASE AGE < 35\n"
        "            cTag = 'MID'\n"
        "        OTHERWISE\n"
        "            cTag = 'SENIOR'\n"
        "    ENDCASE\n"
        "    IF cTag = 'MID'\n"
        "        nTagged = nTagged + 1\n"
        "    ENDIF\n"
        "ENDSCAN\n"
        "cAfterScan = cTag\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "DO CASE script should complete");

    const auto branch = state.globals.find("cbranch");
    const auto no_match = state.globals.find("nnomatch");
    const auto nested = state.globals.find("cnested");
    const auto tagged = state.globals.find("ntagged");
    const auto after_scan = state.globals.find("cafterscan");

    expect(branch != state.globals.end(), "DO CASE should expose the selected branch result");
    expect(no_match != state.globals.end(), "DO CASE without OTHERWISE should complete without mutating unmatched state");
    expect(nested != state.globals.end(), "nested DO CASE blocks should execute correctly");
    expect(tagged != state.globals.end(), "DO CASE inside SCAN should participate in cursor-backed logic");
    expect(after_scan != state.globals.end(), "DO CASE inside SCAN should leave the last computed branch value");

    if (branch != state.globals.end()) {
        expect(copperfin::runtime::format_value(branch->second) == "TWO", "DO CASE should execute the first matching CASE branch only");
    }
    if (no_match != state.globals.end()) {
        expect(copperfin::runtime::format_value(no_match->second) == "0", "DO CASE with no match and no OTHERWISE should fall through cleanly");
    }
    if (nested != state.globals.end()) {
        expect(copperfin::runtime::format_value(nested->second) == "INNER", "nested DO CASE blocks should honor inner matching semantics");
    }
    if (tagged != state.globals.end()) {
        expect(copperfin::runtime::format_value(tagged->second) == "2", "DO CASE inside SCAN should classify matching rows without fall-through");
    }
    if (after_scan != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_scan->second) == "SENIOR", "DO CASE should preserve the last branch result inside loop-driven execution");
    }

    fs::remove_all(temp_root, ignored);
}

}
