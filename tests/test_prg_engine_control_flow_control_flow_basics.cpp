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

void test_push_pop_key_menu_popup_stack_commands() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_push_pop_stack_commands";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "push_pop_stack_commands.prg";
    write_text(
        main_path,
        "PUSH KEY CTRL+F\n"
        "PUSH KEY ALT+G\n"
        "POP KEY\n"
        "POP KEY\n"
        "POP KEY\n"
        "PUSH MENU _MSYSMENU\n"
        "PUSH MENU _MFILE PAD 1\n"
        "POP MENU\n"
        "POP MENU\n"
        "POP MENU\n"
        "PUSH POPUP ShortcutMenu\n"
        "PUSH POPUP ContextMenu\n"
        "POP POPUP\n"
        "POP POPUP\n"
        "POP POPUP\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "PUSH/POP stack command script should complete");

    const auto push_key_count = static_cast<int>(std::count_if(state.events.begin(), state.events.end(), [](const auto &event) {
        return event.category == "runtime.push_key";
    }));
    const auto pop_key_count = static_cast<int>(std::count_if(state.events.begin(), state.events.end(), [](const auto &event) {
        return event.category == "runtime.pop_key";
    }));
    const auto push_menu_count = static_cast<int>(std::count_if(state.events.begin(), state.events.end(), [](const auto &event) {
        return event.category == "runtime.push_menu";
    }));
    const auto pop_menu_count = static_cast<int>(std::count_if(state.events.begin(), state.events.end(), [](const auto &event) {
        return event.category == "runtime.pop_menu";
    }));
    const auto push_popup_count = static_cast<int>(std::count_if(state.events.begin(), state.events.end(), [](const auto &event) {
        return event.category == "runtime.push_popup";
    }));
    const auto pop_popup_count = static_cast<int>(std::count_if(state.events.begin(), state.events.end(), [](const auto &event) {
        return event.category == "runtime.pop_popup";
    }));

    expect(push_key_count == 2, "PUSH KEY should emit runtime.push_key per command");
    expect(pop_key_count == 3, "POP KEY should emit runtime.pop_key per command");
    expect(push_menu_count == 2, "PUSH MENU should emit runtime.push_menu per command");
    expect(pop_menu_count == 3, "POP MENU should emit runtime.pop_menu per command");
    expect(push_popup_count == 2, "PUSH POPUP should emit runtime.push_popup per command");
    expect(pop_popup_count == 3, "POP POPUP should emit runtime.pop_popup per command");

    expect(
        std::any_of(state.events.begin(), state.events.end(), [](const auto &event) {
            return event.category == "runtime.push_key" && event.detail.find("depth=2") != std::string::npos &&
                   event.detail.find("target=ALT+G") != std::string::npos;
        }),
        "PUSH KEY should increment stack depth and include the pushed key marker");
    expect(
        std::any_of(state.events.begin(), state.events.end(), [](const auto &event) {
            return event.category == "runtime.pop_key" && event.detail.find("depth=0") != std::string::npos &&
                   event.detail.find("empty=true") != std::string::npos;
        }),
        "POP KEY on an empty stack should be a safe no-op and report empty stack detail");

    expect(
        std::any_of(state.events.begin(), state.events.end(), [](const auto &event) {
            return event.category == "runtime.push_menu" && event.detail.find("depth=2") != std::string::npos &&
                   event.detail.find("target=_MFILE PAD 1") != std::string::npos;
        }),
        "PUSH MENU should increment stack depth and include the pushed menu marker");
    expect(
        std::any_of(state.events.begin(), state.events.end(), [](const auto &event) {
            return event.category == "runtime.pop_menu" && event.detail.find("depth=0") != std::string::npos &&
                   event.detail.find("empty=true") != std::string::npos;
        }),
        "POP MENU on an empty stack should be a safe no-op and report empty stack detail");

    expect(
        std::any_of(state.events.begin(), state.events.end(), [](const auto &event) {
            return event.category == "runtime.push_popup" && event.detail.find("depth=2") != std::string::npos &&
                   event.detail.find("target=ContextMenu") != std::string::npos;
        }),
        "PUSH POPUP should increment stack depth and include the pushed popup marker");
    expect(
        std::any_of(state.events.begin(), state.events.end(), [](const auto &event) {
            return event.category == "runtime.pop_popup" && event.detail.find("depth=0") != std::string::npos &&
                   event.detail.find("empty=true") != std::string::npos;
        }),
        "POP POPUP on an empty stack should be a safe no-op and report empty stack detail");

    fs::remove_all(temp_root, ignored);
}

void test_text_endtext_literal_blocks() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_text_blocks";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "text_blocks.prg";
    write_text(
        main_path,
        "nTextMergeCalls = 0\n"
        "cName = 'Copperfin'\n"
        "nCount = 3\n"
        "TEXT TO cBody NOSHOW\n"
        "Alpha\n"
        "\n"
        "* literal star line\n"
        "&& literal ampersand line\n"
        "ENDTEXT\n"
        "TEXT TO cBody ADDITIVE NOSHOW\n"
        "Bravo\n"
        "ENDTEXT\n"
        "TEXT TO cMerged TEXTMERGE NOSHOW\n"
        "Name=<<merge_probe(cName)>>; Count=<<merge_probe(nCount)>>\n"
        "ENDTEXT\n"
        "cNameExpr = 'LEFT(cName, 9)'\n"
        "cNameExprHolder = 'cNameExpr'\n"
        "cNameExprDeepHolder = 'cNameExprHolder'\n"
        "cFieldExpr = 'cName'\n"
        "cFieldExprHolder = 'cFieldExpr'\n"
        "cFieldExprDeepHolder = 'cFieldExprHolder'\n"
        "cRecursiveExpr = '<<EVAL(&cNameExprDeepHolder)>>'\n"
        "cRecursiveExprHolder = 'cRecursiveExpr'\n"
        "cRecursiveExprDeepHolder = 'cRecursiveExprHolder'\n"
        "TEXT TO cMergedNested TEXTMERGE NOSHOW\n"
        "Eval=<<EVAL(cNameExpr)>>; Macro=<<&cFieldExpr>>\n"
        "ENDTEXT\n"
        "TEXT TO cMergedSecondHop TEXTMERGE NOSHOW\n"
        "Eval=<<EVAL(&cNameExprDeepHolder)>>; Macro=<<&cFieldExprDeepHolder>>\n"
        "ENDTEXT\n"
        "TEXT TO cMergedRecursiveSecondHop TEXTMERGE NOSHOW\n"
        "Recursive=<<&cRecursiveExprDeepHolder>>\n"
        "ENDTEXT\n"
        "RETURN\n"
        "FUNCTION merge_probe\n"
        "LPARAMETERS value\n"
        "nTextMergeCalls = nTextMergeCalls + 1\n"
        "RETURN value\n"
        "ENDFUNC\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "TEXT/ENDTEXT script should complete");

    const auto body = state.globals.find("cbody");
    expect(body != state.globals.end(), "TEXT TO should assign the captured block into the target variable");
    if (body != state.globals.end()) {
        expect(
            copperfin::runtime::format_value(body->second) == "Alpha\n\n* literal star line\n&& literal ampersand line\nBravo\n",
            "TEXT/ENDTEXT should preserve literal lines and ADDITIVE should append the next block");
    }

    const auto merged = state.globals.find("cmerged");
    expect(merged != state.globals.end(), "TEXT TEXTMERGE should assign merged block content");
    if (merged != state.globals.end()) {
        expect(
            copperfin::runtime::format_value(merged->second) == "Name=Copperfin; Count=3\n",
            "TEXT TEXTMERGE should interpolate <<expression>> segments using runtime expression evaluation");
    }

    const auto text_merge_calls = state.globals.find("ntextmergecalls");
    expect(text_merge_calls != state.globals.end(),
           "TEXT TEXTMERGE should preserve the UDF interpolation call counter");
    if (text_merge_calls != state.globals.end()) {
        expect(copperfin::runtime::format_value(text_merge_calls->second) == "2",
               "TEXT TEXTMERGE should evaluate each UDF interpolation exactly once");
    }

    const auto merged_nested = state.globals.find("cmergednested");
    expect(merged_nested != state.globals.end(), "TEXT TEXTMERGE should assign nested eval/macro merged block content");
    if (merged_nested != state.globals.end()) {
        expect(
            copperfin::runtime::format_value(merged_nested->second) == "Eval=Copperfin; Macro=Copperfin\n",
            "TEXT TEXTMERGE should preserve nested EVAL() and &macro interpolation inside merged expressions");
    }

    const auto merged_second_hop = state.globals.find("cmergedsecondhop");
    expect(merged_second_hop != state.globals.end(), "TEXT TEXTMERGE should assign second-hop nested eval/macro merged block content");
    if (merged_second_hop != state.globals.end()) {
        expect(
            copperfin::runtime::format_value(merged_second_hop->second) == "Eval=Copperfin; Macro=Copperfin\n",
            "TEXT TEXTMERGE should preserve second-hop nested EVAL() and &macro interpolation inside merged expressions");
    }

    const auto merged_recursive_second_hop = state.globals.find("cmergedrecursivesecondhop");
    expect(merged_recursive_second_hop != state.globals.end(), "TEXT TEXTMERGE should assign second-hop merged block content");
    if (merged_recursive_second_hop != state.globals.end()) {
        expect(
            copperfin::runtime::format_value(merged_recursive_second_hop->second) == "Recursive=<<EVAL(&cNameExprDeepHolder)>>\n",
            "TEXT TEXTMERGE should preserve a second-pass expression as literal interpolated data");
    }

    const auto text_events = static_cast<int>(std::count_if(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.text";
    }));
    expect(text_events == 6, "each TEXT block should emit a runtime.text event");

    fs::remove_all(temp_root, ignored);
}

void test_text_endtext_honors_set_textmerge_state_and_delimiters() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_textmerge_set_state";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "textmerge_state.prg";
    write_text(
        main_path,
        "cName = 'Copperfin'\n"
        "SET TEXTMERGE ON\n"
        "SET TEXTMERGE DELIMITERS TO '{|', '|}'\n"
        "TEXT TO cMerged NOSHOW\n"
        "Value={|cName|}\n"
        "ENDTEXT\n"
        "SET TEXTMERGE OFF\n"
        "TEXT TO cLiteral NOSHOW\n"
        "Value={|cName|}\n"
        "ENDTEXT\n"
        "SET TEXTMERGE DELIMITERS TO '<@'\n"
        "TEXT TO cMergedShared TEXTMERGE NOSHOW\n"
        "Shared=<@cName<@\n"
        "ENDTEXT\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "TEXT/ENDTEXT script honoring SET TEXTMERGE state should complete");

    const auto merged = state.globals.find("cmerged");
    const auto literal = state.globals.find("cliteral");
    const auto merged_shared = state.globals.find("cmergedshared");

    expect(merged != state.globals.end(), "plain TEXT/ENDTEXT should assign merged output when SET TEXTMERGE is ON");
    expect(literal != state.globals.end(), "plain TEXT/ENDTEXT should assign literal output when SET TEXTMERGE is OFF");
    expect(merged_shared != state.globals.end(), "TEXT ... TEXTMERGE should honor shared delimiters from SET TEXTMERGE DELIMITERS");

    if (merged != state.globals.end()) {
        expect(
            copperfin::runtime::format_value(merged->second) == "Value=Copperfin\n",
            "plain TEXT/ENDTEXT should merge current session delimiters when SET TEXTMERGE is ON");
    }
    if (literal != state.globals.end()) {
        expect(
            copperfin::runtime::format_value(literal->second) == "Value={|cName|}\n",
            "plain TEXT/ENDTEXT should stay literal when SET TEXTMERGE is OFF");
    }
    if (merged_shared != state.globals.end()) {
        expect(
            copperfin::runtime::format_value(merged_shared->second) == "Shared=Copperfin\n",
            "TEXT ... TEXTMERGE should honor single-argument shared delimiters from SET TEXTMERGE DELIMITERS");
    }

    fs::remove_all(temp_root, ignored);
}

void test_text_endtext_textmerge_keeps_interpolated_delimiters_literal() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_textmerge_single_pass";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "textmerge_single_pass.prg";
    write_text(
        main_path,
        "cDefaultData = 'a<<1+1>>b'\n"
        "TEXT TO cDefaultMerged TEXTMERGE NOSHOW\n"
        "<<cDefaultData>>\n"
        "ENDTEXT\n"
        "SET TEXTMERGE DELIMITERS TO '{|', '|}'\n"
        "cCustomData = 'x{|1+1|}y'\n"
        "TEXT TO cCustomMerged TEXTMERGE NOSHOW\n"
        "{|cCustomData|}\n"
        "ENDTEXT\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "single-pass TEXT TEXTMERGE script should complete");

    const auto default_merged = state.globals.find("cdefaultmerged");
    const auto custom_merged = state.globals.find("ccustommerged");
    expect(default_merged != state.globals.end(), "default-delimiter TEXT TEXTMERGE output should be captured");
    expect(custom_merged != state.globals.end(), "custom-delimiter TEXT TEXTMERGE output should be captured");
    if (default_merged != state.globals.end())
    {
        expect(copperfin::runtime::format_value(default_merged->second) == "a<<1+1>>b\n",
               "TEXT TEXTMERGE must not rescan default-delimiter text produced by interpolation");
    }
    if (custom_merged != state.globals.end())
    {
        expect(copperfin::runtime::format_value(custom_merged->second) == "x{|1+1|}y\n",
               "TEXT TEXTMERGE must not rescan custom-delimiter text produced by interpolation");
    }

    fs::remove_all(temp_root, ignored);
}

void test_scan_on_empty_table_does_not_execute_body() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_scan_empty_table";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "empty_people.dbf";
    write_people_dbf(table_path, {});

    const fs::path main_path = temp_root / "scan_empty.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS EmptyPeople IN 0\n"
        "nScanHits = 0\n"
        "SCAN\n"
        "    nScanHits = nScanHits + 1\n"
        "ENDSCAN\n"
        "lAfterScanEof = EOF()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SCAN over an empty table should complete");

    const auto scan_hits = state.globals.find("nscanhits");
    const auto after_scan_eof = state.globals.find("lafterscaneof");
    expect(scan_hits != state.globals.end(), "empty-table SCAN should still expose the scan counter");
    expect(after_scan_eof != state.globals.end(), "empty-table SCAN should expose EOF() state after scan");

    if (scan_hits != state.globals.end()) {
        expect(copperfin::runtime::format_value(scan_hits->second) == "0",
               "SCAN body should not execute for an empty table");
    }
    if (after_scan_eof != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_scan_eof->second) == "true",
               "EOF() should remain true after scanning an empty table");
    }

    fs::remove_all(temp_root, ignored);
}

void test_locate_on_empty_table_sets_eof() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_locate_empty_table";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "empty_people.dbf";
    write_people_dbf(table_path, {});

    const fs::path main_path = temp_root / "locate_empty.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS EmptyPeople IN 0\n"
        "LOCATE FOR .T.\n"
        "lFoundAfterLocate = FOUND()\n"
        "lEofAfterLocate = EOF()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "LOCATE on an empty table should complete");

    const auto found_after_locate = state.globals.find("lfoundafterlocate");
    const auto eof_after_locate = state.globals.find("leofafterlocate");
    expect(found_after_locate != state.globals.end(), "LOCATE on an empty table should expose FOUND()");
    expect(eof_after_locate != state.globals.end(), "LOCATE on an empty table should expose EOF()");

    if (found_after_locate != state.globals.end()) {
        expect(copperfin::runtime::format_value(found_after_locate->second) == "false",
               "LOCATE on an empty table should leave FOUND() false");
    }
    if (eof_after_locate != state.globals.end()) {
        expect(copperfin::runtime::format_value(eof_after_locate->second) == "true",
               "LOCATE on an empty table should leave EOF() true");
    }

    fs::remove_all(temp_root, ignored);
}

void test_locate_continue_advances_to_later_matches() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_locate_continue";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}, {"DELTA", 40}});

    const fs::path main_path = temp_root / "locate_continue.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "LOCATE FOR AGE >= 20 WHILE AGE < 40\n"
        "nHits = 0\n"
        "cFirst = ''\n"
        "cSecond = ''\n"
        "DO WHILE FOUND()\n"
        "    nHits = nHits + 1\n"
        "    IF nHits = 1\n"
        "        cFirst = NAME\n"
        "    ELSE\n"
        "        cSecond = NAME\n"
        "    ENDIF\n"
        "    CONTINUE\n"
        "ENDDO\n"
        "lFoundAfter = FOUND()\n"
        "lEofAfter = EOF()\n"
        "nRecAfter = RECNO()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "LOCATE/CONTINUE script should complete: " + state.message);

    const auto hits = state.globals.find("nhits");
    const auto first = state.globals.find("cfirst");
    const auto second = state.globals.find("csecond");
    const auto found_after = state.globals.find("lfoundafter");
    const auto eof_after = state.globals.find("leofafter");
    const auto rec_after = state.globals.find("nrecafter");

    expect(hits != state.globals.end(), "LOCATE/CONTINUE should expose the number of matching rows");
    expect(first != state.globals.end(), "LOCATE/CONTINUE should expose the first matching name");
    expect(second != state.globals.end(), "LOCATE/CONTINUE should expose the second matching name");
    expect(found_after != state.globals.end(), "LOCATE/CONTINUE should expose FOUND() after the final CONTINUE");
    expect(eof_after != state.globals.end(), "LOCATE/CONTINUE should expose EOF() after the final CONTINUE");
    expect(rec_after != state.globals.end(), "LOCATE/CONTINUE should expose RECNO() after the final CONTINUE");

    if (hits != state.globals.end()) {
        expect(copperfin::runtime::format_value(hits->second) == "2", "LOCATE/CONTINUE should iterate both matches before the WHILE boundary");
    }
    if (first != state.globals.end()) {
        expect(copperfin::runtime::format_value(first->second) == "BRAVO", "LOCATE/CONTINUE should start from the first matching record");
    }
    if (second != state.globals.end()) {
        expect(copperfin::runtime::format_value(second->second) == "CHARLIE", "CONTINUE should advance to the next matching record");
    }
    if (found_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(found_after->second) == "false", "final CONTINUE should clear FOUND() after the last match");
    }
    if (eof_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(eof_after->second) == "true", "final CONTINUE should move to EOF after the last match");
    }
    if (rec_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(rec_after->second) == "5", "final CONTINUE should place RECNO() at record_count + 1");
    }

    fs::remove_all(temp_root, ignored);
}

void test_go_top_bottom_on_empty_table_does_not_crash() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_go_topbottom_empty";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "empty_tbl.dbf";
    write_people_dbf(table_path, {});

    const fs::path main_path = temp_root / "go_topbottom_empty.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS EmptyTbl IN 0\n"
        "GO TOP\n"
        "lBofAfterTop = BOF()\n"
        "lEofAfterTop = EOF()\n"
        "nRecAfterTop = RECNO()\n"
        "GO BOTTOM\n"
        "lBofAfterBottom = BOF()\n"
        "lEofAfterBottom = EOF()\n"
        "nRecAfterBottom = RECNO()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "GO TOP/BOTTOM on empty table should not crash");

    const auto bof_after_top    = state.globals.find("lbofaftertop");
    const auto eof_after_top    = state.globals.find("leofaftertop");
    const auto bof_after_bottom = state.globals.find("lbofafterbottom");
    const auto eof_after_bottom = state.globals.find("leofafterbottom");
    const auto rec_after_top = state.globals.find("nrecaftertop");
    const auto rec_after_bottom = state.globals.find("nrecafterbottom");

    expect(bof_after_top    != state.globals.end(), "GO TOP on empty table should expose BOF()");
    expect(eof_after_top    != state.globals.end(), "GO TOP on empty table should expose EOF()");
    expect(bof_after_bottom != state.globals.end(), "GO BOTTOM on empty table should expose BOF()");
    expect(eof_after_bottom != state.globals.end(), "GO BOTTOM on empty table should expose EOF()");
    expect(rec_after_top != state.globals.end(), "GO TOP on empty table should expose RECNO()");
    expect(rec_after_bottom != state.globals.end(), "GO BOTTOM on empty table should expose RECNO()");

    if (bof_after_top != state.globals.end()) {
        expect(copperfin::runtime::format_value(bof_after_top->second) == "true",
               "GO TOP on empty table should leave BOF() true");
    }
    if (eof_after_top != state.globals.end()) {
        expect(copperfin::runtime::format_value(eof_after_top->second) == "true",
               "GO TOP on empty table should leave EOF() true");
    }
    if (bof_after_bottom != state.globals.end()) {
        expect(copperfin::runtime::format_value(bof_after_bottom->second) == "true",
               "GO BOTTOM on empty table should leave BOF() true");
    }
    if (eof_after_bottom != state.globals.end()) {
        expect(copperfin::runtime::format_value(eof_after_bottom->second) == "true",
               "GO BOTTOM on empty table should leave EOF() true");
    }
    if (rec_after_top != state.globals.end()) {
        expect(copperfin::runtime::format_value(rec_after_top->second) == "1",
               "GO TOP on empty table should report RECNO() as the first record number");
    }
    if (rec_after_bottom != state.globals.end()) {
        expect(copperfin::runtime::format_value(rec_after_bottom->second) == "1",
               "GO BOTTOM on empty table should report RECNO() as the first record number");
    }

    fs::remove_all(temp_root, ignored);
}

void test_go_top_bottom_with_no_visible_records_sets_bof_and_eof() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_go_topbottom_filtered_empty";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}});

    const fs::path main_path = temp_root / "go_topbottom_filtered_empty.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET FILTER TO .F.\n"
        "GO TOP\n"
        "lBofAfterTop = BOF()\n"
        "lEofAfterTop = EOF()\n"
        "GO BOTTOM\n"
        "lBofAfterBottom = BOF()\n"
        "lEofAfterBottom = EOF()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "GO TOP/BOTTOM with no visible records should complete");

    const auto bof_after_top = state.globals.find("lbofaftertop");
    const auto eof_after_top = state.globals.find("leofaftertop");
    const auto bof_after_bottom = state.globals.find("lbofafterbottom");
    const auto eof_after_bottom = state.globals.find("leofafterbottom");

    expect(bof_after_top != state.globals.end(), "filtered GO TOP should expose BOF()");
    expect(eof_after_top != state.globals.end(), "filtered GO TOP should expose EOF()");
    expect(bof_after_bottom != state.globals.end(), "filtered GO BOTTOM should expose BOF()");
    expect(eof_after_bottom != state.globals.end(), "filtered GO BOTTOM should expose EOF()");

    if (bof_after_top != state.globals.end()) {
        expect(copperfin::runtime::format_value(bof_after_top->second) == "false",
               "GO TOP with no visible records in a nonempty table should leave BOF() false");
    }
    if (eof_after_top != state.globals.end()) {
        expect(copperfin::runtime::format_value(eof_after_top->second) == "true",
               "GO TOP with no visible records should leave EOF() true");
    }
    if (bof_after_bottom != state.globals.end()) {
        expect(copperfin::runtime::format_value(bof_after_bottom->second) == "true",
               "GO BOTTOM with no visible records should leave BOF() true");
    }
    if (eof_after_bottom != state.globals.end()) {
        expect(copperfin::runtime::format_value(eof_after_bottom->second) == "true",
               "GO BOTTOM with no visible records should leave EOF() true");
    }

    fs::remove_all(temp_root, ignored);
}

void test_static_diagnostic_flags_likely_infinite_do_while_loop() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_static_diag";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english_catalog = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish_catalog = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese_catalog = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo_catalog = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const std::vector<std::string_view> keys{
        "Runtime.PrgStaticAnalysis.Diagnostic.PRG0001.Message",
        "Runtime.PrgStaticAnalysis.Diagnostic.PRG1001.Message",
        "Runtime.PrgStaticAnalysis.Diagnostic.PRG1002.Message"};

    expect(
        english_catalog.translate("Runtime.PrgStaticAnalysis.Diagnostic.PRG1001.Message") ==
            "Likely infinite loop: DO WHILE condition is always true and no EXIT/RETURN path was found.",
        "#2605: PRG1001 should preserve the en-US catalog text");
    expect(
        spanish_catalog.translate("Runtime.PrgStaticAnalysis.Diagnostic.PRG1002.Message") ==
            "Al bloque DO WHILE le falta ENDDO.",
        "#2605: PRG1002 should resolve through the es-419 catalog");
    expect(
        portuguese_catalog.translate("Runtime.PrgStaticAnalysis.Diagnostic.PRG0001.Message") ==
            "Falha ao analisar o codigo-fonte PRG: nao foi possivel abrir o arquivo.",
        "#2605: PRG0001 should resolve through the pt-BR catalog");
    expect(
        pseudo_catalog.translate("Runtime.PrgStaticAnalysis.Diagnostic.PRG1002.Message") ==
            copperfin::localization::pseudo_localize("DO WHILE block is missing ENDDO."),
        "#2605: qps-ploc static-analysis diagnostics should route through the pseudo-localization transform");
    expect(
        count_missing_locale_keys(spanish_catalog, "es-419", keys) == 0U,
        "#2605: es-419 should define every remaining Runtime.PrgStaticAnalysis localization key");
    expect(
        count_missing_locale_keys(portuguese_catalog, "pt-BR", keys) == 0U,
        "#2605: pt-BR should define every remaining Runtime.PrgStaticAnalysis localization key");
    expect(
        count_missing_locale_keys(pseudo_catalog, "qps-ploc", keys) == 0U,
        "#2605: qps-ploc should define every remaining Runtime.PrgStaticAnalysis localization key");

    ScopedEnvironmentValue scoped_locale("COPPERFIN_LOCALE");
    set_env_value("COPPERFIN_LOCALE", "en-US", true);

    const fs::path flagged_path = temp_root / "flagged.prg";
    write_text(
        flagged_path,
        "DO WHILE .T.\n"
        "x = 1\n"
        "ENDDO\n");

    const auto diagnostics = copperfin::runtime::analyze_prg_file(flagged_path.string());
    expect(!diagnostics.empty(), "analyzer should emit diagnostics for likely infinite loops");
    const auto infinite_loop_warning = std::find_if(
        diagnostics.begin(),
        diagnostics.end(),
        [](const copperfin::runtime::PrgStaticDiagnostic& diagnostic) {
            return diagnostic.code == "PRG1001";
        });
    expect(infinite_loop_warning != diagnostics.end(), "analyzer should emit PRG1001 for DO WHILE .T. without exit path");
    if (infinite_loop_warning != diagnostics.end()) {
        expect(infinite_loop_warning->severity == copperfin::runtime::DiagnosticSeverity::warning,
            "PRG1001 severity should remain warning");
        expect(
            infinite_loop_warning->message ==
                "Likely infinite loop: DO WHILE condition is always true and no EXIT/RETURN path was found.",
            "PRG1001 message should route through the default locale catalog");
    }

    set_env_value("COPPERFIN_LOCALE", "es-419", true);
    const auto spanish_diagnostics = copperfin::runtime::analyze_prg_file(flagged_path.string());
    const auto spanish_infinite_loop_warning = std::find_if(
        spanish_diagnostics.begin(),
        spanish_diagnostics.end(),
        [](const copperfin::runtime::PrgStaticDiagnostic& diagnostic) {
            return diagnostic.code == "PRG1001";
        });
    expect(
        spanish_infinite_loop_warning != spanish_diagnostics.end() &&
            spanish_infinite_loop_warning->message ==
                "Bucle probablemente infinito: la condicion DO WHILE siempre es verdadera y no se encontro ninguna ruta EXIT/RETURN.",
        "#2605: PRG1001 should refresh to es-419 when the runtime locale changes in-process");
    set_env_value("COPPERFIN_LOCALE", "en-US", true);

    const fs::path missing_enddo_path = temp_root / "missing_enddo.prg";
    write_text(
        missing_enddo_path,
        "DO WHILE .T.\n"
        "x = 1\n");
    const auto missing_enddo_diagnostics = copperfin::runtime::analyze_prg_file(missing_enddo_path.string());
    const auto missing_enddo_error = std::find_if(
        missing_enddo_diagnostics.begin(),
        missing_enddo_diagnostics.end(),
        [](const copperfin::runtime::PrgStaticDiagnostic& diagnostic) {
            return diagnostic.code == "PRG1002";
        });
    expect(missing_enddo_error != missing_enddo_diagnostics.end(), "analyzer should emit PRG1002 for unterminated DO WHILE");
    if (missing_enddo_error != missing_enddo_diagnostics.end()) {
        expect(missing_enddo_error->severity == copperfin::runtime::DiagnosticSeverity::error,
            "PRG1002 severity should remain error");
        expect(missing_enddo_error->message == "DO WHILE block is missing ENDDO.",
            "PRG1002 message should route through the default locale catalog");
    }

    set_env_value("COPPERFIN_LOCALE", "pt-BR", true);
    const auto portuguese_missing_enddo_diagnostics =
        copperfin::runtime::analyze_prg_file(missing_enddo_path.string());
    const auto portuguese_missing_enddo_error = std::find_if(
        portuguese_missing_enddo_diagnostics.begin(),
        portuguese_missing_enddo_diagnostics.end(),
        [](const copperfin::runtime::PrgStaticDiagnostic& diagnostic) {
            return diagnostic.code == "PRG1002";
        });
    expect(
        portuguese_missing_enddo_error != portuguese_missing_enddo_diagnostics.end() &&
            portuguese_missing_enddo_error->message == "O bloco DO WHILE esta sem ENDDO.",
        "#2605: PRG1002 should refresh to pt-BR when the runtime locale changes in-process");
    set_env_value("COPPERFIN_LOCALE", "en-US", true);

    const fs::path missing_path = temp_root / "missing.prg";
    const auto missing_file_diagnostics = copperfin::runtime::analyze_prg_file(missing_path.string());
    expect(missing_file_diagnostics.size() == 1U, "missing PRG source should emit exactly one diagnostic");
    if (!missing_file_diagnostics.empty()) {
        expect(missing_file_diagnostics.front().code == "PRG0001",
            "missing PRG source diagnostic code should remain stable");
        expect(missing_file_diagnostics.front().severity == copperfin::runtime::DiagnosticSeverity::error,
            "PRG0001 severity should remain error");
        expect(
            missing_file_diagnostics.front().message == "Failed to analyze PRG source: file could not be opened.",
            "PRG0001 message should route through the default locale catalog");
    }

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    const auto pseudo_missing_file_diagnostics = copperfin::runtime::analyze_prg_file(missing_path.string());
    expect(
        pseudo_missing_file_diagnostics.size() == 1U &&
            pseudo_missing_file_diagnostics.front().message ==
                copperfin::localization::pseudo_localize(
                    "Failed to analyze PRG source: file could not be opened."),
        "#2605: PRG0001 should refresh to qps-ploc when the runtime locale changes in-process");

    set_env_value("COPPERFIN_LOCALE", "en-US", true);

    const fs::path safe_path = temp_root / "safe.prg";
    write_text(
        safe_path,
        "DO WHILE .T.\n"
        "EXIT\n"
        "ENDDO\n");
    const auto safe_diagnostics = copperfin::runtime::analyze_prg_file(safe_path.string());
    const bool safe_has_warning = std::any_of(
        safe_diagnostics.begin(),
        safe_diagnostics.end(),
        [](const copperfin::runtime::PrgStaticDiagnostic& diagnostic) {
            return diagnostic.code == "PRG1001";
        });
    expect(!safe_has_warning, "analyzer should not emit PRG1001 when an explicit EXIT path exists");

    fs::remove_all(temp_root, ignored);
}

void test_elseif_control_flow_executes_matching_branch() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_elseif";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "elseif_test.prg";
    write_text(
        main_path,
        "x = 2\n"
        "IF x = 1\n"
        "  outcome = 'if'\n"
        "ELSEIF x = 2\n"
        "  outcome = 'elseif'\n"
        "ELSE\n"
        "  outcome = 'else'\n"
        "ENDIF\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "ELSEIF script should complete");
    const auto outcome = state.globals.find("outcome");
    expect(outcome != state.globals.end(), "ELSEIF script should assign outcome");
    if (outcome != state.globals.end()) {
        expect(copperfin::runtime::format_value(outcome->second) == "elseif", "ELSEIF branch should be selected");
    }

    fs::remove_all(temp_root, ignored);
}

void test_block_terminators_ignore_trailing_annotations() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_terminator_annotations";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "terminator_annotations.prg";
    write_text(
        main_path,
        "LOCAL nValue, nLoop\n"
        "nValue = 0\n"
        "IF .T.\n"
        "    nValue = nValue + 1\n"
        "ENDIF nValue = 1\n"
        "FOR nLoop = 1 TO 2\n"
        "    nValue = nValue + 1\n"
        "ENDFOR nLoop = 1 TO 2\n"
        "RETURN nValue\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "annotated block terminators should complete");
    expect(state.last_return_value.has_value(), "annotated block terminators should preserve RETURN");
    if (state.last_return_value.has_value()) {
        expect(copperfin::runtime::format_value(*state.last_return_value) == "3",
               "annotated block terminators should not execute their suffixes");
    }

    fs::remove_all(temp_root, ignored);
}

void test_with_endwith_resolves_leading_dot_member_access() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_with";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "with_block.prg";
    write_text(
        main_path,
        "obj = CREATEOBJECT('Sample.Object')\n"
        "WITH obj\n"
        "  .Caption = 'Hello'\n"
        "  prop_value = .Caption\n"
        "  call_value = .Add('World')\n"
        "ENDWITH\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "WITH/ENDWITH script should complete");

    const auto prop_value = state.globals.find("prop_value");
    const auto call_value = state.globals.find("call_value");
    expect(prop_value != state.globals.end(), "WITH should resolve leading-dot property reads");
    expect(call_value != state.globals.end(), "WITH should resolve leading-dot method calls");
    if (prop_value != state.globals.end()) {
        expect(
            copperfin::runtime::format_value(prop_value->second) == "Hello",
            "WITH property access should bind to the target object");
    }
    if (call_value != state.globals.end()) {
        expect(
            copperfin::runtime::format_value(call_value->second).find("object:Sample.Object.add#") == 0U,
            "WITH method calls should bind to the target object");
    }

    const bool has_with_event = std::any_of(
        state.events.begin(),
        state.events.end(),
        [](const copperfin::runtime::RuntimeEvent& event) {
            return event.category == "runtime.with";
        });
    expect(has_with_event, "runtime should emit a WITH event");

    fs::remove_all(temp_root, ignored);
}

void test_with_endwith_preserves_reserved_dotted_logical_tokens() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_with_dotted_tokens";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "with_dotted_tokens.prg";
    write_text(
        main_path,
        "obj = CREATEOBJECT('Sample.Object')\n"
        "lFound = .T.\n"
        "WITH obj\n"
        "  start_true = .T.\n"
        "  start_false = .F.\n"
        "  start_not = .NOT. lFound\n"
        "  grouped_and = (.T.) .AND. (.T.)\n"
        "  grouped_or = (.F.) .OR. (.T.)\n"
        "  comma_tokens = IIF(.F., .F., .T.)\n"
        "  null_token = ISNULL(.NULL.)\n"
        "  mixed_case = .nOt. .f.\n"
        "  .Caption = 'Updated'\n"
        "  .TrueValue = 7\n"
        "  .NotValue = 8\n"
        "  member_caption = .Caption\n"
        "  member_true_value = .TrueValue\n"
        "  member_not_value = .NotValue\n"
        "ENDWITH\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed,
           "#3928: WITH should preserve reserved dotted logical and NULL tokens");

    const auto expect_value = [&](const std::string& name, const std::string& expected) {
        const auto value = state.globals.find(name);
        expect(value != state.globals.end(), "#3928: WITH dotted-token script should assign " + name);
        if (value != state.globals.end()) {
            expect(copperfin::runtime::format_value(value->second) == expected,
                   "#3928: WITH dotted-token result mismatch for " + name);
        }
    };

    expect_value("start_true", "true");
    expect_value("start_false", "false");
    expect_value("start_not", "false");
    expect_value("grouped_and", "true");
    expect_value("grouped_or", "true");
    expect_value("comma_tokens", "true");
    expect_value("null_token", "true");
    expect_value("mixed_case", "true");
    expect_value("member_caption", "Updated");
    expect_value("member_true_value", "7");
    expect_value("member_not_value", "8");

    fs::remove_all(temp_root, ignored);
}

void test_with_target_uses_heap_backed_expression_continuations() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_with_target_continuation";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "with_target_continuation.prg";
    write_text(
        main_path,
        "FUNCTION make_with_target\n"
        "    LOCAL target\n"
        "    target = CREATEOBJECT('Sample.Object')\n"
        "    target.Caption = 'from udf'\n"
        "    RETURN target\n"
        "ENDFUNC\n"
        "WITH make_with_target()\n"
        "    outer_caption = .Caption\n"
        "    WITH make_with_target()\n"
        "        inner_caption = .Caption\n"
        "    ENDWITH\n"
        "    outer_after_inner = .Caption\n"
        "ENDWITH\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "WITH target UDF script should complete through the expression trampoline");

    const auto expect_caption = [&](const char *name) {
        const auto value = state.globals.find(name);
        expect(value != state.globals.end(), std::string("WITH target UDF should assign ") + name);
        if (value != state.globals.end()) {
            expect(
                copperfin::runtime::format_value(value->second) == "from udf",
                std::string("WITH target UDF should preserve ") + name + " after resumption");
        }
    };
    expect_caption("outer_caption");
    expect_caption("inner_caption");
    expect_caption("outer_after_inner");

    const auto with_event_count = std::count_if(
        state.events.begin(),
        state.events.end(),
        [](const copperfin::runtime::RuntimeEvent &event) {
            return event.category == "runtime.with";
        });
    expect(with_event_count == 2, "WITH target UDF should bind each target exactly once");

    fs::remove_all(temp_root, ignored);
}

void test_loop_and_exit_unwind_with_bindings_before_jump() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_with_loop_unwind";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 21}, {"BRAVO", 28}, {"CHARLIE", 33}});

    const fs::path main_path = temp_root / "with_loop_unwind.prg";
    write_text(
        main_path,
        "outer_obj = CREATEOBJECT('Sample.Object')\n"
        "inner_obj = CREATEOBJECT('Sample.Object')\n"
        "outer_obj.Caption = 'Outer'\n"
        "inner_obj.Caption = 'Inner'\n"
        "WITH outer_obj\n"
        "    i = 0\n"
        "    DO WHILE i < 3\n"
        "        i = i + 1\n"
        "        WITH inner_obj\n"
        "            IF i < 3\n"
        "                LOOP\n"
        "            ENDIF\n"
        "        ENDWITH\n"
        "    ENDDO\n"
        "    while_loop_caption = .Caption\n"
        "    DO WHILE .T.\n"
        "        WITH inner_obj\n"
        "            EXIT\n"
        "        ENDWITH\n"
        "    ENDDO\n"
        "    while_exit_caption = .Caption\n"
        "    FOR j = 1 TO 3\n"
        "        WITH inner_obj\n"
        "            IF j < 3\n"
        "                LOOP\n"
        "            ENDIF\n"
        "        ENDWITH\n"
        "    ENDFOR\n"
        "    for_loop_caption = .Caption\n"
        "    FOR k = 1 TO 3\n"
        "        WITH inner_obj\n"
        "            EXIT\n"
        "        ENDWITH\n"
        "    ENDFOR\n"
        "    for_exit_caption = .Caption\n"
        "    USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "    SELECT People\n"
        "    SCAN\n"
        "        WITH inner_obj\n"
        "            IF NAME # 'CHARLIE'\n"
        "                LOOP\n"
        "            ENDIF\n"
        "        ENDWITH\n"
        "    ENDSCAN\n"
        "    scan_loop_caption = .Caption\n"
        "    GO TOP\n"
        "    SCAN\n"
        "        WITH inner_obj\n"
        "            EXIT\n"
        "        ENDWITH\n"
        "    ENDSCAN\n"
        "    scan_exit_caption = .Caption\n"
        "ENDWITH\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "#3842: LOOP/EXIT inside WITH should not leak stale bindings across loop jumps");

    const auto expect_outer_caption = [&](const char *name, const char *description) {
        const auto value = state.globals.find(name);
        expect(value != state.globals.end(), std::string("#3842: ") + description + " should assign a result");
        if (value != state.globals.end()) {
            expect(
                copperfin::runtime::format_value(value->second) == "Outer",
                std::string("#3842: ") + description + " should keep the outer WITH binding active");
        }
    };

    expect_outer_caption("while_loop_caption", "DO WHILE LOOP");
    expect_outer_caption("while_exit_caption", "DO WHILE EXIT");
    expect_outer_caption("for_loop_caption", "FOR LOOP");
    expect_outer_caption("for_exit_caption", "FOR EXIT");
    expect_outer_caption("scan_loop_caption", "SCAN LOOP");
    expect_outer_caption("scan_exit_caption", "SCAN EXIT");

    fs::remove_all(temp_root, ignored);
}

void test_loop_and_exit_unwind_case_contexts_before_jump() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_case_loop_unwind";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 21}, {"BRAVO", 28}, {"CHARLIE", 33}});

    const fs::path main_path = temp_root / "case_loop_unwind.prg";
    write_text(
        main_path,
        "lOuterCasePreserved = .T.\n"
        "DO CASE\n"
        "    CASE .T.\n"
        "        nWhileLoop = 0\n"
        "        DO WHILE nWhileLoop < 3\n"
        "            nWhileLoop = nWhileLoop + 1\n"
        "            DO CASE\n"
        "                CASE .T.\n"
        "                    LOOP\n"
        "            ENDCASE\n"
        "        ENDDO\n"
        "        nWhileExit = 0\n"
        "        DO WHILE .T.\n"
        "            DO CASE\n"
        "                CASE .T.\n"
        "                    nWhileExit = nWhileExit + 1\n"
        "                    EXIT\n"
        "            ENDCASE\n"
        "        ENDDO\n"
        "        nForLoop = 0\n"
        "        FOR i = 1 TO 3\n"
        "            DO CASE\n"
        "                CASE .T.\n"
        "                    nForLoop = nForLoop + 1\n"
        "                    LOOP\n"
        "            ENDCASE\n"
        "        ENDFOR\n"
        "        nForExit = 0\n"
        "        FOR j = 1 TO 3\n"
        "            DO CASE\n"
        "                CASE .T.\n"
        "                    nForExit = nForExit + 1\n"
        "                    EXIT\n"
        "            ENDCASE\n"
        "        ENDFOR\n"
        "        DIMENSION aItems(3)\n"
        "        aItems(1) = 'A'\n"
        "        aItems(2) = 'B'\n"
        "        aItems(3) = 'C'\n"
        "        nEachLoop = 0\n"
        "        FOR EACH cItem IN aItems\n"
        "            DO CASE\n"
        "                CASE .T.\n"
        "                    nEachLoop = nEachLoop + 1\n"
        "                    LOOP\n"
        "            ENDCASE\n"
        "        ENDFOR\n"
        "        nEachExit = 0\n"
        "        FOR EACH cItem IN aItems\n"
        "            DO CASE\n"
        "                CASE .T.\n"
        "                    nEachExit = nEachExit + 1\n"
        "                    EXIT\n"
        "            ENDCASE\n"
        "        ENDFOR\n"
        "        USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "        SELECT People\n"
        "        nScanLoop = 0\n"
        "        SCAN\n"
        "            DO CASE\n"
        "                CASE .T.\n"
        "                    nScanLoop = nScanLoop + 1\n"
        "                    LOOP\n"
        "            ENDCASE\n"
        "        ENDSCAN\n"
        "        GO TOP\n"
        "        nScanExit = 0\n"
        "        SCAN\n"
        "            DO CASE\n"
        "                CASE .T.\n"
        "                    nScanExit = nScanExit + 1\n"
        "                    EXIT\n"
        "            ENDCASE\n"
        "        ENDSCAN\n"
        "        FOR nStress = 1 TO 10000\n"
        "            DO CASE\n"
        "                CASE .T.\n"
        "                    LOOP\n"
        "            ENDCASE\n"
        "        ENDFOR\n"
        "    CASE .T.\n"
        "        lOuterCasePreserved = .F.\n"
        "ENDCASE\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "#4019: LOOP/EXIT inside DO CASE should complete without retaining case contexts");

    const auto expect_value = [&](const char *name, const char *expected) {
        const auto value = state.globals.find(name);
        expect(value != state.globals.end(), std::string("#4019: expected result ") + name);
        if (value != state.globals.end()) {
            expect(copperfin::runtime::format_value(value->second) == expected,
                   std::string("#4019: unexpected result for ") + name);
        }
    };

    expect_value("loutercasepreserved", "true");
    expect_value("nwhileloop", "3");
    expect_value("nwhileexit", "1");
    expect_value("nforloop", "3");
    expect_value("nforexit", "1");
    expect_value("neachloop", "3");
    expect_value("neachexit", "1");
    expect_value("nscanloop", "3");
    expect_value("nscanexit", "1");

    fs::remove_all(temp_root, ignored);
}

void test_print_command_emits_event() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_print_cmd";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "print_test.prg";
    write_text(
        main_path,
        "? 'hello world'\n"
        "? 1 + 2\n"
        "SET DECIMALS TO 4\n"
        "SET POINT TO ','\n"
        "SET SEPARATOR TO '.'\n"
        "? 1 / 3\n"
        "? 12345.6789\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "? print command script should complete");

    expect(has_runtime_event(state.events, "runtime.print", "hello world"),
        "? 'hello world' should emit a runtime.print event with detail 'hello world'");

    const bool has_three = std::any_of(state.events.begin(), state.events.end(), [](const copperfin::runtime::RuntimeEvent& ev) {
        return ev.category == "runtime.print" && ev.detail == "3";
    });
    expect(has_three, "? 1 + 2 should emit a runtime.print event with detail '3'");

    const bool has_fraction = std::any_of(state.events.begin(), state.events.end(), [](const copperfin::runtime::RuntimeEvent& ev) {
        return ev.category == "runtime.print" && ev.detail == "0,3333";
    });
    expect(has_fraction, "SET DECIMALS and SET POINT should format fractional print output");

    const bool has_grouped = std::any_of(state.events.begin(), state.events.end(), [](const copperfin::runtime::RuntimeEvent& ev) {
        return ev.category == "runtime.print" && ev.detail == "12.345,6789";
    });
    expect(has_grouped, "SET SEPARATOR should format grouped print output");

    fs::remove_all(temp_root, ignored);
}

void test_erase_copy_rename_file_commands() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_file_ops";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    // Write a file to erase
    write_text(temp_root / "to_erase.txt", "data");
    // Write a file to copy/rename
    write_text(temp_root / "original.txt", "content");

    const fs::path main_path = temp_root / "file_ops.prg";
    write_text(
        main_path,
        "nEraseCalls = 0\n"
        "nCopySourceCalls = 0\n"
        "nCopyDestinationCalls = 0\n"
        "nRenameSourceCalls = 0\n"
        "nRenameDestinationCalls = 0\n"
        "ERASE erase_target('to_erase.txt')\n"
        "COPY FILE copy_source('original.txt') TO copy_destination('copied.txt')\n"
        "RENAME rename_source('copied.txt') TO rename_destination('renamed.txt')\n"
        "RETURN\n"
        "FUNCTION erase_target\n"
        "LPARAMETERS value\n"
        "nEraseCalls = nEraseCalls + 1\n"
        "RETURN value\n"
        "ENDFUNC\n"
        "FUNCTION copy_source\n"
        "LPARAMETERS value\n"
        "nCopySourceCalls = nCopySourceCalls + 1\n"
        "RETURN value\n"
        "ENDFUNC\n"
        "FUNCTION copy_destination\n"
        "LPARAMETERS value\n"
        "nCopyDestinationCalls = nCopyDestinationCalls + 1\n"
        "RETURN value\n"
        "ENDFUNC\n"
        "FUNCTION rename_source\n"
        "LPARAMETERS value\n"
        "nRenameSourceCalls = nRenameSourceCalls + 1\n"
        "RETURN value\n"
        "ENDFUNC\n"
        "FUNCTION rename_destination\n"
        "LPARAMETERS value\n"
        "nRenameDestinationCalls = nRenameDestinationCalls + 1\n"
        "RETURN value\n"
        "ENDFUNC\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "file ops script should complete");

    expect(!fs::exists(temp_root / "to_erase.txt"), "ERASE should have deleted to_erase.txt");
    const auto erase_calls = state.globals.find("nerasecalls");
    expect(erase_calls != state.globals.end(), "ERASE should preserve the path resolver call counter");
    if (erase_calls != state.globals.end()) {
        expect(copperfin::runtime::format_value(erase_calls->second) == "1",
               "ERASE should evaluate the UDF-produced path exactly once");
    }
    const auto copy_source_calls = state.globals.find("ncopysourcecalls");
    expect(copy_source_calls != state.globals.end(), "COPY FILE should preserve the source resolver call counter");
    if (copy_source_calls != state.globals.end()) {
        expect(copperfin::runtime::format_value(copy_source_calls->second) == "1",
               "COPY FILE should evaluate the source UDF exactly once");
    }
    const auto copy_destination_calls = state.globals.find("ncopydestinationcalls");
    expect(copy_destination_calls != state.globals.end(), "COPY FILE should preserve the destination resolver call counter");
    if (copy_destination_calls != state.globals.end()) {
        expect(copperfin::runtime::format_value(copy_destination_calls->second) == "1",
               "COPY FILE should evaluate the destination UDF exactly once");
    }
    const auto rename_source_calls = state.globals.find("nrenamesourcecalls");
    expect(rename_source_calls != state.globals.end(), "RENAME should preserve the source resolver call counter");
    if (rename_source_calls != state.globals.end()) {
        expect(copperfin::runtime::format_value(rename_source_calls->second) == "1",
               "RENAME should evaluate the source UDF exactly once");
    }
    const auto rename_destination_calls = state.globals.find("nrenamedestinationcalls");
    expect(rename_destination_calls != state.globals.end(), "RENAME should preserve the destination resolver call counter");
    if (rename_destination_calls != state.globals.end()) {
        expect(copperfin::runtime::format_value(rename_destination_calls->second) == "1",
               "RENAME should evaluate the destination UDF exactly once");
    }
    expect(fs::exists(temp_root / "original.txt"), "COPY FILE should leave original.txt intact");
    expect(fs::exists(temp_root / "renamed.txt"), "RENAME should create renamed.txt");
    expect(!fs::exists(temp_root / "copied.txt"), "RENAME should remove the old file name copied.txt");

    fs::remove_all(temp_root, ignored);
}

void test_erase_copy_file_strict_verified_bytes() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_copy_file_verified_bytes";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path source = temp_root / "source.txt";
    const fs::path destination = temp_root / "destination.txt";
    const std::string admitted_bytes = "admitted source bytes\n";
    write_text(source, "tampered source bytes\n");
    const fs::path copy_path = temp_root / "strict_copy.prg";
    write_text(
        copy_path,
        "COPY FILE '" + source.generic_string() + "' TO '" + destination.generic_string() + "'\n"
        "RETURN\n");

    auto options = make_runtime_session_options(copy_path.string(), temp_root.string(), false);
    options.verified_file_byte_overrides.emplace(source.string(), admitted_bytes);
    options.require_verified_file_byte_overrides = true;
    const auto state = copperfin::runtime::PrgRuntimeSession::create(options)
                           .run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "strict COPY FILE admitted-byte script should complete: " + state.message);
    expect(fs::exists(destination), "strict COPY FILE should create its destination");
    if (fs::exists(destination)) {
        expect(read_text(destination) == admitted_bytes,
               "strict COPY FILE should copy admitted bytes instead of tampered disk bytes");
    }

    const fs::path missing = temp_root / "missing.txt";
    const fs::path missing_copy_path = temp_root / "strict_missing_copy.prg";
    write_text(
        missing_copy_path,
        "COPY FILE '" + missing.generic_string() + "' TO '" + destination.generic_string() + "'\n"
        "RETURN\n");
    auto missing_options = make_runtime_session_options(missing_copy_path.string(), temp_root.string(), false);
    missing_options.require_verified_file_byte_overrides = true;
    const auto missing_state = copperfin::runtime::PrgRuntimeSession::create(missing_options)
                                   .run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!missing_state.completed, "strict COPY FILE should reject an unadmitted source");
    expect(missing_state.message.find("COPY FILE source is not an admitted verified file") != std::string::npos,
           "strict COPY FILE rejection should use the localized verified-byte diagnostic");

    fs::remove_all(temp_root, ignored);
}

void test_rename_file_command_rejects_existing_destination() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_rename_existing_target";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_text(temp_root / "source.txt", "source-content");
    write_text(temp_root / "existing.txt", "existing-content");

    const fs::path main_path = temp_root / "rename_existing_target.prg";
    write_text(
        main_path,
        "RENAME 'source.txt' TO 'existing.txt'\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!state.completed, "#3703: RENAME should fail when the destination file already exists");
    expect(
        state.message.find("destination already exists") != std::string::npos,
        "#3703: existing-destination RENAME should explain the target-exists failure");
    expect(
        fs::exists(temp_root / "source.txt"),
        "#3703: failing RENAME should leave the original source file in place");
    expect(
        fs::exists(temp_root / "existing.txt"),
        "#3703: failing RENAME should preserve the pre-existing destination file");
    expect(
        read_text(temp_root / "source.txt") == "source-content",
        "#3703: failing RENAME should preserve the source file contents");
    expect(
        read_text(temp_root / "existing.txt") == "existing-content",
        "#3703: failing RENAME should not overwrite the existing destination contents");

    fs::remove_all(temp_root, ignored);
}

void test_for_each_iterates_array_elements() {
    namespace fs = std::filesystem;
    const fs::path tmp = fs::temp_directory_path() / "copperfin_for_each_array";
    std::error_code ign;
    fs::remove_all(tmp, ign);
    fs::create_directories(tmp);
    const fs::path prg = tmp / "test.prg";
    write_text(prg,
        "DIMENSION fruits(3)\n"
        "fruits(1) = 'apple'\n"
        "fruits(2) = 'banana'\n"
        "fruits(3) = 'cherry'\n"
        "result = ''\n"
        "FOR EACH elem IN fruits\n"
        "    result = result + elem + ','\n"
        "ENDFOR\n"
        "RETURN\n");
    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(prg.string(), tmp.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "FOR EACH over array should complete");
    const auto it = state.globals.find("result");
    expect(it != state.globals.end(), "result should be set after FOR EACH");
    expect(it->second.string_value == "apple,banana,cherry,", "FOR EACH should iterate all array elements");
    fs::remove_all(tmp, ign);
}

void test_for_each_single_element_expression() {
    namespace fs = std::filesystem;
    const fs::path tmp = fs::temp_directory_path() / "copperfin_for_each_scalar";
    std::error_code ign;
    fs::remove_all(tmp, ign);
    fs::create_directories(tmp);
    const fs::path prg = tmp / "test.prg";
    write_text(prg,
        "result = ''\n"
        "FOR EACH item IN 'hello'\n"
        "    result = item\n"
        "ENDFOR\n"
        "RETURN\n");
    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(prg.string(), tmp.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "FOR EACH over scalar should complete");
    const auto it = state.globals.find("result");
    expect(it != state.globals.end(), "result should be set");
    expect(it->second.string_value == "hello", "FOR EACH scalar treats expression as single element");
    fs::remove_all(tmp, ign);
}

void test_for_each_iterates_native_collection_direct_and_member_path() {
    namespace fs = std::filesystem;
    const fs::path tmp = fs::temp_directory_path() / "copperfin_for_each_native_collection";
    std::error_code ign;
    fs::remove_all(tmp, ign);
    fs::create_directories(tmp);
    const fs::path prg = tmp / "test.prg";
    write_text(prg,
        "oHost = CREATEOBJECT('HostBox')\n"
        "oItems = oHost.oItems\n"
        "cDirect = ''\n"
        "cMember = ''\n"
        "cEmpty = 'start'\n"
        "FOR EACH oItem IN oItems\n"
        "    cDirect = cDirect + oItem + ','\n"
        "ENDFOR\n"
        "FOR EACH oItem IN oHost.oItems\n"
        "    cMember = cMember + oItem + ','\n"
        "ENDFOR\n"
        "FOR EACH oItem IN oHost.oEmpty\n"
        "    cEmpty = cEmpty + '!'\n"
        "ENDFOR\n"
        "RETURN\n"
        "DEFINE CLASS WorkerCollection AS Collection\n"
        "ENDDEFINE\n"
        "DEFINE CLASS HostBox AS Custom\n"
        "    oItems = .NULL.\n"
        "    oEmpty = .NULL.\n"
        "    PROCEDURE Init\n"
        "        THIS.oItems = CREATEOBJECT('WorkerCollection')\n"
        "        THIS.oItems.Add('alpha')\n"
        "        THIS.oItems.Add('beta', 'second')\n"
        "        THIS.oEmpty = CREATEOBJECT('WorkerCollection')\n"
        "        RETURN\n"
        "    ENDPROC\n"
        "ENDDEFINE\n");
    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(prg.string(), tmp.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "FOR EACH over native Collection should complete");

    const auto check = [&](const std::string& name, const std::string& expected)
    {
        const auto it = state.globals.find(name);
        expect(it != state.globals.end(), name + " should be set");
        if (it != state.globals.end())
        {
            expect(it->second.string_value == expected,
                   name + " expected '" + expected + "' got '" + it->second.string_value + "'");
        }
    };

    check("cdirect", "alpha,beta,");
    check("cmember", "alpha,beta,");
    check("cempty", "start");
    fs::remove_all(tmp, ign);
}

void test_for_each_foxobject_qualifier_tolerates_direct_and_member_path_collections() {
    namespace fs = std::filesystem;
    const fs::path tmp = fs::temp_directory_path() / "copperfin_for_each_foxobject_collection";
    std::error_code ign;
    fs::remove_all(tmp, ign);
    fs::create_directories(tmp);
    const fs::path prg = tmp / "test.prg";
    write_text(prg,
        "oHost = CREATEOBJECT('HostBox')\n"
        "oItems = oHost.oItems\n"
        "cDirect = ''\n"
        "cMember = ''\n"
        "FOR EACH oItem IN oItems FOXOBJECT\n"
        "    cDirect = cDirect + oItem.cTag + ','\n"
        "ENDFOR\n"
        "FOR EACH oItem IN oHost.oItems FOXOBJECT\n"
        "    cMember = cMember + oItem.cTag + ','\n"
        "ENDFOR\n"
        "RETURN\n"
        "DEFINE CLASS WorkerCollection AS Collection\n"
        "ENDDEFINE\n"
        "DEFINE CLASS TagChild AS Custom\n"
        "    cTag = ''\n"
        "ENDDEFINE\n"
        "DEFINE CLASS HostBox AS Custom\n"
        "    oItems = .NULL.\n"
        "    PROCEDURE Init\n"
        "        LOCAL oFirst, oSecond\n"
        "        THIS.oItems = CREATEOBJECT('WorkerCollection')\n"
        "        oFirst = CREATEOBJECT('TagChild')\n"
        "        oFirst.cTag = 'alpha'\n"
        "        oSecond = CREATEOBJECT('TagChild')\n"
        "        oSecond.cTag = 'beta'\n"
        "        THIS.oItems.Add(oFirst)\n"
        "        THIS.oItems.Add(oSecond)\n"
        "        RETURN\n"
        "    ENDPROC\n"
        "ENDDEFINE\n");
    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(prg.string(), tmp.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "FOR EACH ... FOXOBJECT over native Collection should complete");

    const auto check = [&](const std::string& name, const std::string& expected)
    {
        const auto it = state.globals.find(name);
        expect(it != state.globals.end(), name + " should be set");
        if (it != state.globals.end())
        {
            expect(it->second.string_value == expected,
                   name + " expected '" + expected + "' got '" + it->second.string_value + "'");
        }
    };

    check("cdirect", "alpha,beta,");
    check("cmember", "alpha,beta,");
    fs::remove_all(tmp, ign);
}

}  // namespace cf_test_prg_engine_control_flow
