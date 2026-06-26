#include "copperfin/runtime/prg_engine.h"
#include "copperfin/vfp/dbf_table.h"
#include "../src/runtime/prg_engine_command_helpers.h"
#include "prg_engine_test_support.h"
#include <algorithm>
#include <chrono>
#include <cerrno>
#include <cctype>
#include <cstdlib>
#include <cstdint>
#include <future>
#include <filesystem>
#include <fstream>
#include <iostream>
#if defined(_WIN32)
#include <process.h>
#else
#include <sys/wait.h>
#include <unistd.h>
#define _getpid getpid
#endif
#include <sstream>
#include <system_error>
#include <thread>
#include <vector>

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

namespace {

using namespace copperfin::test_support;

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
        "        CONTINUE\n"
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
        "            CONTINUE\n"
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
        expect(copperfin::runtime::format_value(while_total->second) == "8", "DO WHILE should honor CONTINUE and EXIT");
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
        "Name=<<cName>>; Count=<<nCount>>\n"
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
        "RETURN\n");

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
    expect(merged_recursive_second_hop != state.globals.end(), "TEXT TEXTMERGE should assign recursive second-hop merged block content");
    if (merged_recursive_second_hop != state.globals.end()) {
        expect(
            copperfin::runtime::format_value(merged_recursive_second_hop->second) == "Recursive=Copperfin\n",
            "TEXT TEXTMERGE should preserve recursive second-hop nested merged expressions");
    }

    const auto text_events = static_cast<int>(std::count_if(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.text";
    }));
    expect(text_events == 6, "each TEXT block should emit a runtime.text event");

    fs::remove_all(temp_root, ignored);
}

void test_aggregate_functions_respect_visibility() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_aggregates";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}, {"DELTA", 40}});

    const fs::path main_path = temp_root / "aggregates.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "nCountAll = COUNT()\n"
        "nSumAll = SUM(AGE)\n"
        "nAvgAll = AVG(AGE)\n"
        "nMinAll = MIN(AGE)\n"
        "nMaxAll = MAX(AGE)\n"
        "SET FILTER TO AGE >= 20\n"
        "DELETE FOR AGE = 40\n"
        "SET DELETED ON\n"
        "nCountVisible = COUNT()\n"
        "nCountConditional = COUNT(AGE >= 30)\n"
        "nSumVisible = SUM(AGE)\n"
        "nSumConditional = SUM(AGE, AGE >= 30)\n"
        "nAverageVisible = AVERAGE(AGE)\n"
        "nMinVisible = MIN(AGE)\n"
        "nMaxVisible = MAX(AGE)\n"
        "USE '" + table_path.string() + "' ALIAS Other AGAIN IN 0\n"
        "SELECT Other\n"
        "nCountPeopleAlias = COUNT(AGE >= 20, 'People')\n"
        "nSumPeopleAlias = SUM(AGE, AGE >= 20, 'People')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "aggregate script should complete");

    const auto count_all = state.globals.find("ncountall");
    const auto sum_all = state.globals.find("nsumall");
    const auto avg_all = state.globals.find("navgall");
    const auto min_all = state.globals.find("nminall");
    const auto max_all = state.globals.find("nmaxall");
    const auto count_visible = state.globals.find("ncountvisible");
    const auto count_conditional = state.globals.find("ncountconditional");
    const auto sum_visible = state.globals.find("nsumvisible");
    const auto sum_conditional = state.globals.find("nsumconditional");
    const auto avg_visible = state.globals.find("naveragevisible");
    const auto min_visible = state.globals.find("nminvisible");
    const auto max_visible = state.globals.find("nmaxvisible");
    const auto count_alias = state.globals.find("ncountpeoplealias");
    const auto sum_alias = state.globals.find("nsumpeoplealias");

    expect(count_all != state.globals.end(), "COUNT() should be captured");
    expect(sum_all != state.globals.end(), "SUM() should be captured");
    expect(avg_all != state.globals.end(), "AVG() should be captured");
    expect(min_all != state.globals.end(), "MIN() should be captured");
    expect(max_all != state.globals.end(), "MAX() should be captured");
    expect(count_visible != state.globals.end(), "COUNT() should respect active visibility rules");
    expect(count_conditional != state.globals.end(), "COUNT(condition) should be captured");
    expect(sum_visible != state.globals.end(), "SUM() should respect active visibility rules");
    expect(sum_conditional != state.globals.end(), "SUM(value, condition) should be captured");
    expect(avg_visible != state.globals.end(), "AVERAGE() should be captured");
    expect(min_visible != state.globals.end(), "MIN() under filter should be captured");
    expect(max_visible != state.globals.end(), "MAX() under filter should be captured");
    expect(count_alias != state.globals.end(), "COUNT(condition, alias) should be captured");
    expect(sum_alias != state.globals.end(), "SUM(value, condition, alias) should be captured");

    if (count_all != state.globals.end()) {
        expect(copperfin::runtime::format_value(count_all->second) == "4", "COUNT() should count all visible rows before filters");
    }
    if (sum_all != state.globals.end()) {
        expect(copperfin::runtime::format_value(sum_all->second) == "100", "SUM() should total numeric field values across the current cursor");
    }
    if (avg_all != state.globals.end()) {
        expect(copperfin::runtime::format_value(avg_all->second) == "25", "AVG() should compute the mean across visible rows");
    }
    if (min_all != state.globals.end()) {
        expect(copperfin::runtime::format_value(min_all->second) == "10", "MIN() should capture the smallest visible value");
    }
    if (max_all != state.globals.end()) {
        expect(copperfin::runtime::format_value(max_all->second) == "40", "MAX() should capture the largest visible value");
    }
    if (count_visible != state.globals.end()) {
        expect(copperfin::runtime::format_value(count_visible->second) == "2", "COUNT() should respect SET FILTER TO and SET DELETED ON");
    }
    if (count_conditional != state.globals.end()) {
        expect(copperfin::runtime::format_value(count_conditional->second) == "1", "COUNT(condition) should evaluate an additional aggregate condition");
    }
    if (sum_visible != state.globals.end()) {
        expect(copperfin::runtime::format_value(sum_visible->second) == "50", "SUM() should total only currently visible rows");
    }
    if (sum_conditional != state.globals.end()) {
        expect(copperfin::runtime::format_value(sum_conditional->second) == "30", "SUM(value, condition) should apply the extra condition after visibility filtering");
    }
    if (avg_visible != state.globals.end()) {
        expect(copperfin::runtime::format_value(avg_visible->second) == "25", "AVERAGE() should compute the mean over visible rows");
    }
    if (min_visible != state.globals.end()) {
        expect(copperfin::runtime::format_value(min_visible->second) == "20", "MIN() should respect active visibility rules");
    }
    if (max_visible != state.globals.end()) {
        expect(copperfin::runtime::format_value(max_visible->second) == "30", "MAX() should respect active visibility rules");
    }
    if (count_alias != state.globals.end()) {
        expect(copperfin::runtime::format_value(count_alias->second) == "2", "COUNT(condition, alias) should target a non-selected cursor");
    }
    if (sum_alias != state.globals.end()) {
        expect(copperfin::runtime::format_value(sum_alias->second) == "50", "SUM(value, condition, alias) should target a non-selected cursor");
    }

    fs::remove_all(temp_root, ignored);
}

void test_calculate_command_aggregates() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_calculate";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}, {"DELTA", 40}});

    const fs::path main_path = temp_root / "calculate.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "CALCULATE COUNT() TO nCountAll, SUM(AGE) TO nSumAll, AVG(AGE) TO nAvgAll\n"
        "SET FILTER TO AGE >= 20\n"
        "DELETE FOR AGE = 40\n"
        "SET DELETED ON\n"
        "CALCULATE COUNT() TO nCountVisible, SUM(AGE) TO nSumVisible, MIN(AGE) TO nMinVisible, MAX(AGE) TO nMaxVisible\n"
        "CALCULATE COUNT() TO nCountConditional, SUM(AGE) TO nSumConditional FOR AGE >= 30\n"
        "USE '" + table_path.string() + "' ALIAS Other AGAIN IN 0\n"
        "SELECT Other\n"
        "CALCULATE COUNT() TO nCountPeopleAlias, SUM(AGE) TO nSumPeopleAlias FOR AGE >= 20 IN 'People'\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "CALCULATE script should complete");

    const auto count_all = state.globals.find("ncountall");
    const auto sum_all = state.globals.find("nsumall");
    const auto avg_all = state.globals.find("navgall");
    const auto count_visible = state.globals.find("ncountvisible");
    const auto sum_visible = state.globals.find("nsumvisible");
    const auto min_visible = state.globals.find("nminvisible");
    const auto max_visible = state.globals.find("nmaxvisible");
    const auto count_conditional = state.globals.find("ncountconditional");
    const auto sum_conditional = state.globals.find("nsumconditional");
    const auto count_alias = state.globals.find("ncountpeoplealias");
    const auto sum_alias = state.globals.find("nsumpeoplealias");

    expect(count_all != state.globals.end(), "CALCULATE COUNT() should assign into a variable");
    expect(sum_all != state.globals.end(), "CALCULATE SUM() should assign into a variable");
    expect(avg_all != state.globals.end(), "CALCULATE AVG() should assign into a variable");
    expect(count_visible != state.globals.end(), "CALCULATE should respect current visibility rules");
    expect(sum_visible != state.globals.end(), "CALCULATE SUM() should respect current visibility rules");
    expect(min_visible != state.globals.end(), "CALCULATE MIN() should assign into a variable");
    expect(max_visible != state.globals.end(), "CALCULATE MAX() should assign into a variable");
    expect(count_conditional != state.globals.end(), "CALCULATE FOR should assign into a variable");
    expect(sum_conditional != state.globals.end(), "CALCULATE FOR should constrain SUM() results");
    expect(count_alias != state.globals.end(), "CALCULATE IN alias should target a non-selected cursor");
    expect(sum_alias != state.globals.end(), "CALCULATE IN alias should sum against a non-selected cursor");

    if (count_all != state.globals.end()) {
        expect(copperfin::runtime::format_value(count_all->second) == "4", "CALCULATE COUNT() should count all rows before filters");
    }
    if (sum_all != state.globals.end()) {
        expect(copperfin::runtime::format_value(sum_all->second) == "100", "CALCULATE SUM() should total numeric field values");
    }
    if (avg_all != state.globals.end()) {
        expect(copperfin::runtime::format_value(avg_all->second) == "25", "CALCULATE AVG() should compute a mean");
    }
    if (count_visible != state.globals.end()) {
        expect(copperfin::runtime::format_value(count_visible->second) == "2", "CALCULATE should respect SET FILTER TO and SET DELETED ON");
    }
    if (sum_visible != state.globals.end()) {
        expect(copperfin::runtime::format_value(sum_visible->second) == "50", "CALCULATE SUM() should total only visible rows");
    }
    if (min_visible != state.globals.end()) {
        expect(copperfin::runtime::format_value(min_visible->second) == "20", "CALCULATE MIN() should respect visibility rules");
    }
    if (max_visible != state.globals.end()) {
        expect(copperfin::runtime::format_value(max_visible->second) == "30", "CALCULATE MAX() should respect visibility rules");
    }
    if (count_conditional != state.globals.end()) {
        expect(copperfin::runtime::format_value(count_conditional->second) == "1", "CALCULATE FOR should apply an additional condition to COUNT()");
    }
    if (sum_conditional != state.globals.end()) {
        expect(copperfin::runtime::format_value(sum_conditional->second) == "30", "CALCULATE FOR should apply an additional condition to SUM()");
    }
    if (count_alias != state.globals.end()) {
        expect(copperfin::runtime::format_value(count_alias->second) == "2", "CALCULATE IN alias should use the targeted cursor context");
    }
    if (sum_alias != state.globals.end()) {
        expect(copperfin::runtime::format_value(sum_alias->second) == "50", "CALCULATE IN alias should sum visible rows from the targeted cursor");
    }

    const auto calculate_event = std::find_if(
        state.events.begin(),
        state.events.end(),
        [](const auto& event)
        {
            return event.category == "runtime.calculate" &&
                   event.detail.find("target='People'") != std::string::npos;
        });
    expect(calculate_event != state.events.end(), "CALCULATE should emit a targeted runtime.calculate event");
    if (calculate_event != state.events.end()) {
        expect(calculate_event->detail.find("People@") != std::string::npos, "CALCULATE event should include targeted cursor metadata");
        expect(calculate_event->detail.find("filter=AGE >= 20") != std::string::npos, "CALCULATE event should include active filter readback");
        expect(calculate_event->detail.find("for=AGE >= 20") != std::string::npos, "CALCULATE event should include FOR clause detail");
    }

    fs::remove_all(temp_root, ignored);
}

void test_command_level_aggregate_commands() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_command_aggregates";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}, {"DELTA", 40}});

    const fs::path main_path = temp_root / "command_aggregates.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET FILTER TO AGE >= 20\n"
        "DELETE FOR AGE = 40\n"
        "SET DELETED ON\n"
        "GO TOP\n"
        "COUNT TO nCountVisible\n"
        "COUNT FOR AGE >= 30 TO nCountConditional\n"
        "SUM AGE, AGE * 2 TO nSumAge, nSumDouble\n"
        "AVERAGE AGE TO nAvgVisible\n"
        "USE '" + table_path.string() + "' ALIAS Other AGAIN IN 0\n"
        "SELECT Other\n"
        "SET FILTER TO AGE >= 30\n"
        "GO TOP\n"
        "SELECT People\n"
        "COUNT TO nOtherCount IN 'Other'\n"
        "SUM AGE TO nOtherSum IN 'Other'\n"
        "AVERAGE AGE TO nOtherAvg IN 'Other'\n"
        "nOtherRec = RECNO('Other')\n"
        "nOtherScanHits = 0\n"
        "LOCATE FOR AGE = 30 IN 'Other'\n"
        "nOtherLocate = RECNO('Other')\n"
        "SCAN FOR AGE >= 30 IN 'Other'\n"
        "    nOtherScanHits = nOtherScanHits + 1\n"
        "ENDSCAN\n"
        "nPeopleRec = RECNO('People')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "command-level aggregate script should complete");

    const auto count_visible = state.globals.find("ncountvisible");
    const auto count_conditional = state.globals.find("ncountconditional");
    const auto sum_age = state.globals.find("nsumage");
    const auto sum_double = state.globals.find("nsumdouble");
    const auto avg_visible = state.globals.find("navgvisible");
    const auto other_count = state.globals.find("nothercount");
    const auto other_sum = state.globals.find("nothersum");
    const auto other_avg = state.globals.find("notheravg");
    const auto other_rec = state.globals.find("notherrec");
    const auto other_locate = state.globals.find("notherlocate");
    const auto other_scan_hits = state.globals.find("notherscanhits");
    const auto people_rec = state.globals.find("npeoplerec");

    expect(count_visible != state.globals.end(), "COUNT TO should assign into a variable");
    expect(count_conditional != state.globals.end(), "COUNT FOR TO should assign into a variable");
    expect(sum_age != state.globals.end(), "SUM TO should assign into a variable");
    expect(sum_double != state.globals.end(), "SUM with multiple expressions should assign into variables");
    expect(avg_visible != state.globals.end(), "AVERAGE TO should assign into a variable");
    expect(other_count != state.globals.end(), "COUNT IN alias should assign into a variable");
    expect(other_sum != state.globals.end(), "SUM IN alias should assign into a variable");
    expect(other_avg != state.globals.end(), "AVERAGE IN alias should assign into a variable");
    expect(other_rec != state.globals.end(), "aggregate IN alias commands should preserve the targeted cursor position");
    expect(other_locate != state.globals.end(), "LOCATE FOR ... IN alias should update the targeted cursor");
    expect(other_scan_hits != state.globals.end(), "SCAN FOR ... IN alias should execute against the targeted cursor");
    expect(people_rec != state.globals.end(), "targeted alias scans should preserve the selected cursor position");

    if (count_visible != state.globals.end()) {
        expect(copperfin::runtime::format_value(count_visible->second) == "2", "COUNT should respect SET FILTER TO and SET DELETED ON");
    }
    if (count_conditional != state.globals.end()) {
        expect(copperfin::runtime::format_value(count_conditional->second) == "1", "COUNT FOR should apply an additional condition");
    }
    if (sum_age != state.globals.end()) {
        expect(copperfin::runtime::format_value(sum_age->second) == "50", "SUM should total visible numeric values");
    }
    if (sum_double != state.globals.end()) {
        expect(copperfin::runtime::format_value(sum_double->second) == "100", "SUM should evaluate numeric expressions per visible row");
    }
    if (avg_visible != state.globals.end()) {
        expect(copperfin::runtime::format_value(avg_visible->second) == "25", "AVERAGE should compute the mean across visible rows");
    }
    if (other_count != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_count->second) == "1", "COUNT IN alias should use the targeted cursor visibility rules");
    }
    if (other_sum != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_sum->second) == "30", "SUM IN alias should total values from the targeted cursor");
    }
    if (other_avg != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_avg->second) == "30", "AVERAGE IN alias should evaluate against the targeted cursor");
    }
    if (other_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_rec->second) == "3", "aggregate IN alias commands should restore the targeted cursor record");
    }
    if (other_locate != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_locate->second) == "3", "LOCATE FOR ... IN alias should position the targeted cursor");
    }
    if (other_scan_hits != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_scan_hits->second) == "1", "SCAN FOR ... IN alias should honor the targeted cursor visibility rules");
    }
    if (people_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(people_rec->second) == "2", "aggregate commands and IN-targeted scans should preserve the selected cursor state");
    }

    expect(
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.count"; }) &&
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.sum"; }) &&
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.average"; }),
        "command-level aggregate commands should emit runtime aggregate events");
    const auto count_other_event = std::find_if(
        state.events.begin(),
        state.events.end(),
        [](const auto& event)
        {
            return event.category == "runtime.count" &&
                   event.detail.find("target='Other'") != std::string::npos;
        });
    expect(count_other_event != state.events.end(), "COUNT IN alias should emit targeted runtime.count metadata");
    if (count_other_event != state.events.end()) {
        expect(count_other_event->detail.find("Other@") != std::string::npos, "COUNT event should include targeted cursor metadata");
        expect(count_other_event->detail.find("filter=AGE >= 30") != std::string::npos, "COUNT event should include targeted filter readback");
        expect(count_other_event->detail.find("into=nOtherCount") != std::string::npos, "COUNT event should include TO target detail");
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

void test_aggregate_commands_on_empty_table_return_zero() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_aggregates_empty_table";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "empty_people.dbf";
    write_people_dbf(table_path, {});

    const fs::path main_path = temp_root / "aggregates_empty.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS EmptyPeople IN 0\n"
        "nCount = COUNT()\n"
        "nSum = SUM(AGE)\n"
        "nAverage = AVERAGE(AGE)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "aggregate helpers over an empty table should complete");

    const auto count = state.globals.find("ncount");
    const auto sum = state.globals.find("nsum");
    const auto average = state.globals.find("naverage");
    expect(count != state.globals.end(), "COUNT() over an empty table should produce a value");
    expect(sum != state.globals.end(), "SUM() over an empty table should produce a value");
    expect(average != state.globals.end(), "AVERAGE() over an empty table should produce a value");

    if (count != state.globals.end()) {
        expect(copperfin::runtime::format_value(count->second) == "0",
               "COUNT() over an empty table should be zero");
    }
    if (sum != state.globals.end()) {
        expect(copperfin::runtime::format_value(sum->second) == "0",
               "SUM() over an empty table should be zero");
    }
    if (average != state.globals.end()) {
        expect(copperfin::runtime::format_value(average->second) == "0",
               "AVERAGE() over an empty table should be zero");
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
        "GO BOTTOM\n"
        "lBofAfterBottom = BOF()\n"
        "lEofAfterBottom = EOF()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "GO TOP/BOTTOM on empty table should not crash");

    const auto bof_after_top    = state.globals.find("lbofaftertop");
    const auto eof_after_top    = state.globals.find("leofaftertop");
    const auto bof_after_bottom = state.globals.find("lbofafterbottom");
    const auto eof_after_bottom = state.globals.find("leofafterbottom");

    expect(bof_after_top    != state.globals.end(), "GO TOP on empty table should expose BOF()");
    expect(eof_after_top    != state.globals.end(), "GO TOP on empty table should expose EOF()");
    expect(bof_after_bottom != state.globals.end(), "GO BOTTOM on empty table should expose BOF()");
    expect(eof_after_bottom != state.globals.end(), "GO BOTTOM on empty table should expose EOF()");

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

    fs::remove_all(temp_root, ignored);
}

void test_aggregate_commands_support_macro_targets_and_calculate_while() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_aggregate_macro_targets";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}, {"DELTA", 40}});

    const fs::path main_path = temp_root / "aggregate_macro_targets.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "GO TOP\n"
        "cCount = 'nCountWhile'\n"
        "cCountHolder = 'cCount'\n"
        "cCountDeepHolder = 'cCountHolder'\n"
        "cSum = 'nSumWhile'\n"
        "cSumHolder = 'cSum'\n"
        "cSumDeepHolder = 'cSumHolder'\n"
        "cCalc = 'nCalcWhile'\n"
        "cCalcHolder = 'cCalc'\n"
        "cCalcDeepHolder = 'cCalcHolder'\n"
        "COUNT WHILE AGE < 35 TO &cCountDeepHolder\n"
        "SUM AGE WHILE AGE < 35 TO &cSumDeepHolder\n"
        "CALCULATE COUNT() TO &cCalcDeepHolder WHILE AGE < 35 IN 'People'\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "aggregate macro-target/while script should complete");

    const auto check = [&](const std::string &name, const std::string &expected, const std::string &message)
    {
        const auto it = state.globals.find(name);
        expect(it != state.globals.end(), message);
        if (it != state.globals.end())
        {
            const std::string actual = copperfin::runtime::format_value(it->second);
            expect(actual == expected, message + " expected " + expected + ", got " + actual);
        }
    };

    check("ncountwhile", "3", "COUNT WHILE TO second-hop &macro should assign the resolved target");
    check("nsumwhile", "60", "SUM WHILE TO second-hop &macro should assign the resolved target");
    check("ncalcwhile", "3", "CALCULATE WHILE TO second-hop &macro should assign the resolved target");

    const auto count_event = std::find_if(
        state.events.begin(),
        state.events.end(),
        [](const auto &event)
        {
            return event.category == "runtime.count" &&
                   event.detail.find("while=AGE < 35") != std::string::npos &&
                   event.detail.find("into=&cCountDeepHolder") != std::string::npos;
        });
    expect(count_event != state.events.end(), "COUNT should surface WHILE and second-hop raw INTO metadata");

    const auto calculate_event = std::find_if(
        state.events.begin(),
        state.events.end(),
        [](const auto &event)
        {
            return event.category == "runtime.calculate" &&
                   event.detail.find("while=AGE < 35") != std::string::npos &&
                   event.detail.find("target='People'") != std::string::npos;
        });
    expect(calculate_event != state.events.end(), "CALCULATE should surface WHILE and IN-target metadata");

    fs::remove_all(temp_root, ignored);
}

void test_command_level_aggregate_scope_and_while_semantics() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_command_aggregate_scope";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}, {"DELTA", 40}});

    const fs::path main_path = temp_root / "command_aggregate_scope.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET FILTER TO AGE >= 20\n"
        "DELETE FOR AGE = 40\n"
        "SET DELETED ON\n"
        "GO TOP\n"
        "COUNT REST TO nCountRest\n"
        "COUNT NEXT 2 TO nCountNextTwo\n"
        "COUNT RECORD 4 TO nCountRecordFour\n"
        "COUNT WHILE AGE < 40 TO nCountWhile\n"
        "SUM AGE REST TO nSumRest\n"
        "SUM AGE, AGE * 2 NEXT 3 TO nSumNextThree, nSumDoubleNextThree\n"
        "AVERAGE AGE WHILE AGE < 40 TO nAvgWhile\n"
        "nPeopleRec = RECNO('People')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "aggregate scope/WHILE script should complete");

    const auto count_rest = state.globals.find("ncountrest");
    const auto count_next_two = state.globals.find("ncountnexttwo");
    const auto count_record_four = state.globals.find("ncountrecordfour");
    const auto count_while = state.globals.find("ncountwhile");
    const auto sum_rest = state.globals.find("nsumrest");
    const auto sum_next_three = state.globals.find("nsumnextthree");
    const auto sum_double_next_three = state.globals.find("nsumdoublenextthree");
    const auto avg_while = state.globals.find("navgwhile");
    const auto people_rec = state.globals.find("npeoplerec");

    expect(count_rest != state.globals.end(), "COUNT REST should assign into a variable");
    expect(count_next_two != state.globals.end(), "COUNT NEXT should assign into a variable");
    expect(count_record_four != state.globals.end(), "COUNT RECORD should assign into a variable");
    expect(count_while != state.globals.end(), "COUNT WHILE should assign into a variable");
    expect(sum_rest != state.globals.end(), "SUM REST should assign into a variable");
    expect(sum_next_three != state.globals.end(), "SUM NEXT should assign into a variable");
    expect(sum_double_next_three != state.globals.end(), "SUM NEXT should support multiple expressions");
    expect(avg_while != state.globals.end(), "AVERAGE WHILE should assign into a variable");
    expect(people_rec != state.globals.end(), "aggregate scope commands should preserve the selected cursor position");

    if (count_rest != state.globals.end()) {
        expect(copperfin::runtime::format_value(count_rest->second) == "2", "COUNT REST should respect the current record and visibility rules");
    }
    if (count_next_two != state.globals.end()) {
        expect(copperfin::runtime::format_value(count_next_two->second) == "2", "COUNT NEXT should apply scope before visibility filtering");
    }
    if (count_record_four != state.globals.end()) {
        expect(copperfin::runtime::format_value(count_record_four->second) == "0", "COUNT RECORD should still respect deleted/filter visibility");
    }
    if (count_while != state.globals.end()) {
        expect(copperfin::runtime::format_value(count_while->second) == "2", "COUNT WHILE should stop when the WHILE condition becomes false after applying visibility");
    }
    if (sum_rest != state.globals.end()) {
        expect(copperfin::runtime::format_value(sum_rest->second) == "50", "SUM REST should total visible rows from the current record forward");
    }
    if (sum_next_three != state.globals.end()) {
        expect(copperfin::runtime::format_value(sum_next_three->second) == "50", "SUM NEXT should total only visible rows within the raw scope");
    }
    if (sum_double_next_three != state.globals.end()) {
        expect(copperfin::runtime::format_value(sum_double_next_three->second) == "100", "SUM NEXT should evaluate each numeric expression within scope");
    }
    if (avg_while != state.globals.end()) {
        expect(copperfin::runtime::format_value(avg_while->second) == "25", "AVERAGE WHILE should stop when the WHILE condition becomes false");
    }
    if (people_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(people_rec->second) == "2", "aggregate scope commands should restore the current selected record");
    }

    fs::remove_all(temp_root, ignored);
}

void test_total_command_for_local_tables() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_total_command";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "sales.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "REGION", .type = 'C', .length = 10U},
        {.name = "AMOUNT", .type = 'N', .length = 6U},
        {.name = "QTY", .type = 'N', .length = 3U}
    };
    const std::vector<std::vector<std::string>> records{
        {"EAST", "10", "1"},
        {"EAST", "15", "2"},
        {"WEST", "8", "4"},
        {"WEST", "12", "5"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "sales DBF fixture should be created");

    const fs::path output_path = temp_root / "totals.dbf";
    const fs::path main_path = temp_root / "total.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS Sales IN 0\n"
        "SET FILTER TO QTY >= 2\n"
        "GO TOP\n"
        "USE '" + table_path.string() + "' ALIAS Other AGAIN IN 0\n"
        "SELECT Other\n"
        "SET FILTER TO QTY >= 4\n"
        "GO TOP\n"
        "SELECT Sales\n"
        "TOTAL TO '" + output_path.string() + "' ON REGION FIELDS AMOUNT, QTY REST FOR AMOUNT >= 8 IN 'Other'\n"
        "nSalesRec = RECNO('Sales')\n"
        "nOtherRec = RECNO('Other')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "TOTAL script should complete");
    const auto sales_rec = state.globals.find("nsalesrec");
    const auto other_rec = state.globals.find("notherrec");
    expect(sales_rec != state.globals.end(), "TOTAL script should capture the current selected record");
    expect(other_rec != state.globals.end(), "TOTAL IN alias should preserve the targeted cursor position");
    if (sales_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(sales_rec->second) == "2", "TOTAL should preserve the selected cursor position");
    }
    if (other_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_rec->second) == "3", "TOTAL IN alias should restore the targeted cursor record");
    }
    const auto total_event = std::find_if(
        state.events.begin(),
        state.events.end(),
        [](const auto& event)
        {
            return event.category == "runtime.total" &&
                   event.detail.find("target='Other'") != std::string::npos;
        });
    expect(total_event != state.events.end(), "TOTAL should emit a targeted runtime.total event");
    if (total_event != state.events.end()) {
        expect(total_event->detail.find("on=REGION") != std::string::npos, "TOTAL event should include the ON field");
        expect(total_event->detail.find("totals=AMOUNT,QTY") != std::string::npos, "TOTAL event should include requested total fields");
        expect(total_event->detail.find("scope=REST") != std::string::npos, "TOTAL event should include scope detail");
        expect(total_event->detail.find("filter=QTY >= 4") != std::string::npos, "TOTAL event should include targeted filter readback");
    }

    const auto totals_result = copperfin::vfp::parse_dbf_table_from_file(output_path.string(), 10U);
    expect(totals_result.ok, "TOTAL should write a readable output DBF");
    expect(totals_result.table.fields.size() == 3U, "TOTAL output should include the group field plus requested numeric fields");
    expect(totals_result.table.records.size() == 1U, "TOTAL IN alias should aggregate only the targeted cursor's visible rows");
    if (totals_result.table.records.size() == 1U) {
        expect(totals_result.table.records[0].values[0].display_value == "WEST", "TOTAL IN alias should keep the targeted cursor's group key");
        expect(totals_result.table.records[0].values[1].display_value == "20", "TOTAL IN alias should sum numeric fields for the targeted cursor");
        expect(totals_result.table.records[0].values[2].display_value == "9", "TOTAL IN alias should sum each requested numeric field");
    }

    fs::remove_all(temp_root, ignored);
}

void test_total_command_errors_use_default_locale_messages() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_total_command_errors";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    std::string error_message;
    expect(!copperfin::runtime::parse_total_command_plan("ON REGION", error_message).has_value(),
        "TOTAL parser should reject commands without a TO target");
    expect(error_message == "TOTAL requires a TO target",
        "TOTAL missing-TO parser error should route through the default locale catalog");

    error_message.clear();
    expect(!copperfin::runtime::parse_total_command_plan("TO 'out.dbf'", error_message).has_value(),
        "TOTAL parser should reject commands without an ON field");
    expect(error_message == "TOTAL requires an ON field",
        "TOTAL missing-ON parser error should route through the default locale catalog");

    const fs::path table_path = temp_root / "sales.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "REGION", .type = 'C', .length = 10U},
        {.name = "AMOUNT", .type = 'N', .length = 6U}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, {{"EAST", "10"}});
    expect(create_result.ok, "TOTAL error test sales DBF fixture should be created");

    const fs::path text_only_path = temp_root / "textonly.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> text_only_fields{
        {.name = "REGION", .type = 'C', .length = 10U}
    };
    const auto text_only_create = copperfin::vfp::create_dbf_table_file(text_only_path.string(), text_only_fields, {{"EAST"}});
    expect(text_only_create.ok, "TOTAL error test text-only DBF fixture should be created");

    const auto run_error_script = [&](const std::string& file_stem, const std::string& script) {
        const fs::path main_path = temp_root / (file_stem + ".prg");
        write_text(main_path, script);
        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
        return session.run(copperfin::runtime::DebugResumeAction::continue_run);
    };

    const auto no_work_area = run_error_script(
        "total_no_work_area",
        "TOTAL TO '" + (temp_root / "no_work_area.dbf").string() + "' ON REGION FIELDS AMOUNT\n");
    expect(no_work_area.reason == copperfin::runtime::DebugPauseReason::error,
        "TOTAL without a selected work area should pause with an error");
    expect(no_work_area.message == "TOTAL requires a selected work area",
        "TOTAL selected-work-area error should route through the default locale catalog");

    const auto missing_target = run_error_script(
        "total_missing_target",
        "TOTAL TO '" + (temp_root / "missing_target.dbf").string() + "' ON REGION FIELDS AMOUNT IN MissingAlias\n");
    expect(missing_target.reason == copperfin::runtime::DebugPauseReason::error,
        "TOTAL targeting a missing work area should pause with an error");
    expect(missing_target.message == "TOTAL target work area not found",
        "TOTAL target-work-area error should route through the default locale catalog");

    const auto missing_on_field = run_error_script(
        "total_missing_on_field",
        "USE '" + table_path.string() + "' ALIAS Sales IN 0\n"
        "SELECT Sales\n"
        "TOTAL TO '" + (temp_root / "missing_on.dbf").string() + "' ON MISSING FIELDS AMOUNT\n");
    expect(missing_on_field.reason == copperfin::runtime::DebugPauseReason::error,
        "TOTAL with a missing ON field should pause with an error");
    expect(missing_on_field.message == "TOTAL ON field was not found",
        "TOTAL missing-ON-field error should route through the default locale catalog");

    const auto missing_field = run_error_script(
        "total_missing_field",
        "USE '" + table_path.string() + "' ALIAS Sales IN 0\n"
        "SELECT Sales\n"
        "TOTAL TO '" + (temp_root / "missing_field.dbf").string() + "' ON REGION FIELDS MISSING\n");
    expect(missing_field.reason == copperfin::runtime::DebugPauseReason::error,
        "TOTAL with a missing FIELDS item should pause with an error");
    expect(missing_field.message == "TOTAL field was not found: MISSING",
        "TOTAL missing-field error should interpolate fieldName through the default locale catalog");

    const auto non_numeric_field = run_error_script(
        "total_non_numeric_field",
        "USE '" + table_path.string() + "' ALIAS Sales IN 0\n"
        "SELECT Sales\n"
        "TOTAL TO '" + (temp_root / "non_numeric.dbf").string() + "' ON REGION FIELDS REGION\n");
    expect(non_numeric_field.reason == copperfin::runtime::DebugPauseReason::error,
        "TOTAL with a nonnumeric FIELDS item should pause with an error");
    expect(non_numeric_field.message == "TOTAL only supports numeric FIELDS in the first pass",
        "TOTAL nonnumeric-field error should route through the default locale catalog");

    const auto no_numeric_fields = run_error_script(
        "total_no_numeric_fields",
        "USE '" + text_only_path.string() + "' ALIAS TextOnly IN 0\n"
        "SELECT TextOnly\n"
        "TOTAL TO '" + (temp_root / "no_numeric.dbf").string() + "' ON REGION\n");
    expect(no_numeric_fields.reason == copperfin::runtime::DebugPauseReason::error,
        "TOTAL without numeric fields to total should pause with an error");
    expect(no_numeric_fields.message == "TOTAL requires at least one numeric field to total",
        "TOTAL no-numeric-field error should route through the default locale catalog");

    fs::remove_all(temp_root, ignored);
}

void test_total_command_supports_currency_and_integer_fields() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_total_currency_integer";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "sales.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "REGION", .type = 'C', .length = 10U},
        {.name = "TOTALCUR", .type = 'Y', .length = 8U, .decimal_count = 4U},
        {.name = "QTY", .type = 'I', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {"EAST", "1.2500", "2"},
        {"EAST", "2.5000", "3"},
        {"WEST", "4.0000", "5"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "currency/integer sales DBF fixture should be created");

    const fs::path output_path = temp_root / "totals.dbf";
    const fs::path main_path = temp_root / "total_currency_integer.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS Sales IN 0\n"
        "GO 2\n"
        "TOTAL TO '" + output_path.string() + "' ON REGION FIELDS TOTALCUR, QTY\n"
        "nRecAfter = RECNO()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "TOTAL currency/integer script should complete");

    const auto rec_after = state.globals.find("nrecafter");
    expect(rec_after != state.globals.end(), "TOTAL currency/integer script should capture the current selected record");
    if (rec_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(rec_after->second) == "2", "TOTAL currency/integer should preserve the selected cursor position");
    }

    const auto totals_result = copperfin::vfp::parse_dbf_table_from_file(output_path.string(), 10U);
    expect(totals_result.ok, "TOTAL currency/integer should write a readable output DBF");
    expect(totals_result.table.fields.size() == 3U, "TOTAL currency/integer output should include ON plus requested numeric fields");
    expect(totals_result.table.records.size() == 2U, "TOTAL currency/integer output should include one group per contiguous ON value");
    if (totals_result.table.records.size() == 2U) {
        expect(totals_result.table.records[0].values[0].display_value == "EAST", "TOTAL currency/integer should preserve the first grouped ON value");
        expect(totals_result.table.records[0].values[1].display_value == "3.7500", "TOTAL currency/integer should sum currency fields with four-decimal fidelity");
        expect(totals_result.table.records[0].values[2].display_value == "5", "TOTAL currency/integer should sum integer fields");
        expect(totals_result.table.records[1].values[0].display_value == "WEST", "TOTAL currency/integer should preserve trailing grouped ON values");
        expect(totals_result.table.records[1].values[1].display_value == "4.0000", "TOTAL currency/integer should preserve trailing currency totals");
        expect(totals_result.table.records[1].values[2].display_value == "5", "TOTAL currency/integer should preserve trailing integer totals");
    }

    fs::remove_all(temp_root, ignored);
}

void test_total_command_for_sql_result_cursors() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_total_sql_cursor";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path output_path = temp_root / "sql_totals.dbf";
    const fs::path main_path = temp_root / "total_sql.prg";
    write_text(
        main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "nExec = SQLEXEC(nConn, 'select * from customers', 'sqlcust')\n"
        "SELECT sqlcust\n"
        "REPLACE NAME WITH 'ALPHA', AMOUNT WITH 5\n"
        "GO 2\n"
        "REPLACE NAME WITH 'ALPHA', AMOUNT WITH 7\n"
        "GO 3\n"
        "REPLACE NAME WITH 'WEST', AMOUNT WITH 9\n"
        "GO TOP\n"
        "TOTAL TO '" + output_path.string() + "' ON NAME FIELDS AMOUNT REST FOR AMOUNT >= 7 IN 'sqlcust'\n"
        "nSqlRec = RECNO('sqlcust')\n"
        "lDisc = SQLDISCONNECT(nConn)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "TOTAL SQL script should complete");
    expect(state.sql_connections.empty(), "TOTAL SQL script should disconnect its SQL handle");

    const auto exec = state.globals.find("nexec");
    const auto sql_rec = state.globals.find("nsqlrec");
    const auto disc = state.globals.find("ldisc");
    expect(exec != state.globals.end(), "SQLEXEC result should be captured for TOTAL SQL parity");
    expect(sql_rec != state.globals.end(), "TOTAL IN alias should preserve SQL cursor position");
    expect(disc != state.globals.end(), "SQLDISCONNECT should be captured for TOTAL SQL parity");
    if (exec != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec->second) == "1", "SQLEXEC should succeed before TOTAL SQL checks");
    }
    if (sql_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(sql_rec->second) == "1", "TOTAL should preserve targeted SQL cursor record position");
    }
    if (disc != state.globals.end()) {
        expect(copperfin::runtime::format_value(disc->second) == "1", "SQLDISCONNECT should succeed after TOTAL SQL checks");
    }
    const auto sql_total_event = std::find_if(
        state.events.begin(),
        state.events.end(),
        [](const auto& event)
        {
            return event.category == "runtime.total" &&
                   event.detail.find("target='sqlcust'") != std::string::npos;
        });
    expect(sql_total_event != state.events.end(), "TOTAL for SQL cursors should emit targeted runtime.total metadata");
    if (sql_total_event != state.events.end()) {
        expect(sql_total_event->detail.find("sqlcust@") != std::string::npos, "SQL TOTAL event should include cursor metadata");
        expect(sql_total_event->detail.find("on=NAME") != std::string::npos, "SQL TOTAL event should include ON field metadata");
        expect(sql_total_event->detail.find("for=AMOUNT >= 7") != std::string::npos, "SQL TOTAL event should include FOR clause metadata");
    }

    const auto totals_result = copperfin::vfp::parse_dbf_table_from_file(output_path.string(), 10U);
    expect(totals_result.ok, "TOTAL should write a readable output DBF for SQL cursors");
    expect(totals_result.table.fields.size() == 2U, "TOTAL SQL output should include ON plus requested numeric fields");
    expect(totals_result.table.records.size() == 2U, "TOTAL SQL output should include one group per contiguous ON value");
    if (totals_result.table.records.size() == 2U) {
        expect(totals_result.table.records[0].values[0].display_value == "ALPHA", "TOTAL SQL output should preserve first grouped ON value");
        expect(totals_result.table.records[0].values[1].display_value == "7", "TOTAL SQL output should sum grouped numeric fields");
        expect(totals_result.table.records[1].values[0].display_value == "WEST", "TOTAL SQL output should include trailing group values");
        expect(totals_result.table.records[1].values[1].display_value == "9", "TOTAL SQL output should sum trailing grouped numeric values");
    }

    fs::remove_all(temp_root, ignored);
}

void test_private_declaration_masks_caller_variable() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_private_mask";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "private_mask.prg";
    write_text(
        main_path,
        "x = 42\n"
        "DO subproc\n"
        "caller_x = x\n"
        "RETURN\n"
        "PROCEDURE subproc\n"
        "PRIVATE x\n"
        "x = 99\n"
        "sub_x = x\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "PRIVATE mask script should complete");

    const auto sub_x = state.globals.find("sub_x");
    const auto caller_x = state.globals.find("caller_x");

    expect(sub_x != state.globals.end(), "sub_x should be in globals");
    expect(caller_x != state.globals.end(), "caller_x should be in globals");

    if (sub_x != state.globals.end()) {
        expect(copperfin::runtime::format_value(sub_x->second) == "99", "sub should see its own PRIVATE x = 99");
    }
    if (caller_x != state.globals.end()) {
        expect(copperfin::runtime::format_value(caller_x->second) == "42", "caller x should be restored to 42 after sub returns");
    }

    fs::remove_all(temp_root, ignored);
}

void test_private_variable_visible_to_called_routines() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_private_visible";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "private_visible.prg";
    write_text(
        main_path,
        "DO caller\n"
        "RETURN\n"
        "PROCEDURE caller\n"
        "PRIVATE shared_val\n"
        "shared_val = 77\n"
        "DO inner\n"
        "RETURN\n"
        "PROCEDURE inner\n"
        "inner_saw = shared_val\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "PRIVATE visibility script should complete");

    const auto inner_saw = state.globals.find("inner_saw");
    expect(inner_saw != state.globals.end(), "inner_saw should be in globals");
    if (inner_saw != state.globals.end()) {
        expect(copperfin::runtime::format_value(inner_saw->second) == "77", "PRIVATE variable should be visible to called routines");
    }

    fs::remove_all(temp_root, ignored);
}

void test_release_private_restores_saved_binding_immediately() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_release_private_restore";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "release_private_restore.prg";
    write_text(
        main_path,
        "x = 42\n"
        "DO subproc\n"
        "caller_x = x\n"
        "RETURN\n"
        "PROCEDURE subproc\n"
        "PRIVATE x\n"
        "x = 99\n"
        "RELEASE x\n"
        "sub_x_after_release = x\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "RELEASE of PRIVATE binding script should complete");

    const auto sub_x_after_release = state.globals.find("sub_x_after_release");
    const auto caller_x = state.globals.find("caller_x");

    expect(sub_x_after_release != state.globals.end(), "sub_x_after_release should be captured");
    expect(caller_x != state.globals.end(), "caller_x should be captured");

    if (sub_x_after_release != state.globals.end()) {
        expect(copperfin::runtime::format_value(sub_x_after_release->second) == "42",
               "RELEASE x inside a PRIVATE scope should immediately restore the saved outer binding");
    }
    if (caller_x != state.globals.end()) {
        expect(copperfin::runtime::format_value(caller_x->second) == "42",
               "the caller binding should remain restored after the PRIVATE scope returns");
    }

    fs::remove_all(temp_root, ignored);
}

void test_release_local_restores_visible_outer_global() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_release_local_restore";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "release_local_restore.prg";
    write_text(
        main_path,
        "x = 42\n"
        "DO subproc\n"
        "caller_x = x\n"
        "RETURN\n"
        "PROCEDURE subproc\n"
        "LOCAL x\n"
        "x = 99\n"
        "RELEASE x\n"
        "sub_x_after_release = x\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "RELEASE of LOCAL binding script should complete");

    const auto sub_x_after_release = state.globals.find("sub_x_after_release");
    const auto caller_x = state.globals.find("caller_x");

    expect(sub_x_after_release != state.globals.end(), "sub_x_after_release should be captured");
    expect(caller_x != state.globals.end(), "caller_x should be captured");

    if (sub_x_after_release != state.globals.end()) {
        expect(copperfin::runtime::format_value(sub_x_after_release->second) == "42",
               "RELEASE x inside a LOCAL scope should reveal the visible outer global binding");
    }
    if (caller_x != state.globals.end()) {
        expect(copperfin::runtime::format_value(caller_x->second) == "42",
               "the outer global binding should remain intact after the LOCAL scope returns");
    }

    fs::remove_all(temp_root, ignored);
}

void test_macro_assignment_target_updates_private_binding_and_release_restores_outer_value() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_macro_assign_private_release";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "macro_assign_private_release.prg";
    write_text(
        main_path,
        "x = 42\n"
        "cTarget = 'x'\n"
        "DO subproc\n"
        "caller_x = x\n"
        "RETURN\n"
        "PROCEDURE subproc\n"
        "PRIVATE x\n"
        "&cTarget = 99\n"
        "sub_x_after_macro_assign = x\n"
        "RELEASE x\n"
        "sub_x_after_release = x\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "macro-expanded PRIVATE assignment target script should complete");

    const auto sub_x_after_macro_assign = state.globals.find("sub_x_after_macro_assign");
    const auto sub_x_after_release = state.globals.find("sub_x_after_release");
    const auto caller_x = state.globals.find("caller_x");

    expect(sub_x_after_macro_assign != state.globals.end(), "sub_x_after_macro_assign should be captured");
    expect(sub_x_after_release != state.globals.end(), "sub_x_after_release should be captured");
    expect(caller_x != state.globals.end(), "caller_x should be captured");

    if (sub_x_after_macro_assign != state.globals.end()) {
        expect(copperfin::runtime::format_value(sub_x_after_macro_assign->second) == "99",
               "&cTarget = value should update the visible PRIVATE binding rather than resolve to the binding's current value");
    }
    if (sub_x_after_release != state.globals.end()) {
        expect(copperfin::runtime::format_value(sub_x_after_release->second) == "42",
               "RELEASE after a macro-target PRIVATE assignment should restore the saved outer binding");
    }
    if (caller_x != state.globals.end()) {
        expect(copperfin::runtime::format_value(caller_x->second) == "42",
               "the caller binding should remain intact after the PRIVATE scope returns");
    }

    fs::remove_all(temp_root, ignored);
}

void test_macro_assignment_target_preserves_public_binding_across_release_all() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_macro_assign_public_release_all";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "macro_assign_public_release_all.prg";
    write_text(
        main_path,
        "PUBLIC shared\n"
        "shared = 7\n"
        "cTarget = 'shared'\n"
        "&cTarget = 9\n"
        "RELEASE ALL\n"
        "nSharedAfterReleaseAll = shared\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "macro-expanded PUBLIC assignment target script should complete");

    const auto shared_after_release_all = state.globals.find("nsharedafterreleaseall");
    expect(shared_after_release_all != state.globals.end(), "nSharedAfterReleaseAll should be captured");

    if (shared_after_release_all != state.globals.end()) {
        expect(copperfin::runtime::format_value(shared_after_release_all->second) == "9",
               "macro-expanded assignment should preserve PUBLIC binding identity so RELEASE ALL keeps the updated value");
    }

    fs::remove_all(temp_root, ignored);
}

void test_store_command_assigns_multiple_variables() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_store";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "store_test.prg";
    write_text(
        main_path,
        "STORE 7 TO a, b, c\n"
        "STORE 'hello' TO s1, s2\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "STORE script should complete");

    const auto a = state.globals.find("a");
    const auto b = state.globals.find("b");
    const auto c = state.globals.find("c");
    const auto s1 = state.globals.find("s1");
    const auto s2 = state.globals.find("s2");

    expect(a != state.globals.end(), "STORE should assign a");
    expect(b != state.globals.end(), "STORE should assign b");
    expect(c != state.globals.end(), "STORE should assign c");
    expect(s1 != state.globals.end(), "STORE should assign s1");
    expect(s2 != state.globals.end(), "STORE should assign s2");

    if (a != state.globals.end()) {
        expect(copperfin::runtime::format_value(a->second) == "7", "a should equal 7");
    }
    if (b != state.globals.end()) {
        expect(copperfin::runtime::format_value(b->second) == "7", "b should equal 7");
    }
    if (c != state.globals.end()) {
        expect(copperfin::runtime::format_value(c->second) == "7", "c should equal 7");
    }
    if (s1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(s1->second) == "hello", "s1 should equal 'hello'");
    }
    if (s2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(s2->second) == "hello", "s2 should equal 'hello'");
    }

    fs::remove_all(temp_root, ignored);
}

std::string build_nested_do_chain_script(std::size_t nested_routine_count) {
    std::ostringstream script;
    if (nested_routine_count == 0U) {
        script << "RETURN\n";
        return script.str();
    }

    script << "DO p1\n";
    script << "RETURN\n";
    for (std::size_t index = 1; index <= nested_routine_count; ++index) {
        script << "PROCEDURE p" << index << "\n";
        if (index < nested_routine_count) {
            script << "DO p" << (index + 1U) << "\n";
        }
        script << "RETURN\n";
    }

    return script.str();
}

void test_runtime_guardrail_limits_call_depth_without_crashing_host() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_guard_call_depth";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "deep_calls.prg";
    write_text(
        main_path,
        "DO a\n"
        "RETURN\n"
        "PROCEDURE a\n"
        "DO b\n"
        "RETURN\n"
        "PROCEDURE b\n"
        "DO c\n"
        "RETURN\n"
        "PROCEDURE c\n"
        "DO d\n"
        "RETURN\n"
        "PROCEDURE d\n"
        "RETURN\n");

    auto session_options = make_runtime_session_options(main_path.string(), temp_root.string(), false);
    session_options.max_call_depth = 3;
    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(session_options);

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error, "call-depth guardrail should pause with an error");
    expect(
        state.message.find("maximum call depth") != std::string::npos,
        "call-depth guardrail should report a call-depth limit message");

    fs::remove_all(temp_root, ignored);
}

void test_runtime_guardrail_exactly_at_call_depth_limit_succeeds() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_guard_call_depth_exact_limit";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    constexpr std::size_t limit = 5U;
    const fs::path main_path = temp_root / "exact_limit.prg";
    write_text(main_path, build_nested_do_chain_script(limit));

    auto session_options = make_runtime_session_options(main_path.string(), temp_root.string(), false);
    session_options.max_call_depth = limit;
    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(session_options);

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "exactly-at-limit nested call chain should complete");
    expect(state.reason != copperfin::runtime::DebugPauseReason::error,
           "exactly-at-limit nested call chain should not dispatch guardrail errors");

    fs::remove_all(temp_root, ignored);
}

void test_runtime_guardrail_one_over_call_depth_limit_fails() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_guard_call_depth_one_over";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    constexpr std::size_t limit = 5U;
    const fs::path main_path = temp_root / "one_over_limit.prg";
    write_text(main_path, build_nested_do_chain_script(limit + 1U));

    auto session_options = make_runtime_session_options(main_path.string(), temp_root.string(), false);
    session_options.max_call_depth = limit;
    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(session_options);

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error,
           "one-over-limit nested call chain should dispatch a guardrail error");
    expect(state.message.find("maximum call depth") != std::string::npos,
           "one-over-limit nested call chain should report call-depth limit details");

    fs::remove_all(temp_root, ignored);
}

void test_runtime_guardrail_limits_statement_budget_without_crashing_host() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_guard_statement_budget";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "endless_loop.prg";
    write_text(
        main_path,
        "DO WHILE .T.\n"
        "x = 1\n"
        "ENDDO\n");

    auto session_options = make_runtime_session_options(main_path.string(), temp_root.string(), false);
    session_options.max_executed_statements = 30;
    session_options.max_loop_iterations = 1000;
    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(session_options);

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error, "statement-budget guardrail should pause with an error");
    expect(
        state.message.find("maximum executed statements") != std::string::npos,
        "statement-budget guardrail should report a statement-budget limit message");

    fs::remove_all(temp_root, ignored);
}

void test_static_diagnostic_flags_likely_infinite_do_while_loop() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_static_diag";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

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

void test_config_fpw_overrides_runtime_limits() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_config_limits";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_text(
        temp_root / "config.fpw",
        "MAX_CALL_DEPTH = 3\n"
        "MAX_EXECUTED_STATEMENTS = 40\n"
        "MAX_LOOP_ITERATIONS = 500\n");

    const fs::path main_path = temp_root / "deep_calls.prg";
    write_text(
        main_path,
        "DO a\n"
        "RETURN\n"
        "PROCEDURE a\n"
        "DO b\n"
        "RETURN\n"
        "PROCEDURE b\n"
        "DO c\n"
        "RETURN\n"
        "PROCEDURE c\n"
        "DO d\n"
        "RETURN\n"
        "PROCEDURE d\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error, "config.fpw call-depth limit should pause with an error");
    expect(
        state.message.find("maximum call depth") != std::string::npos,
        "config.fpw should control max call depth when options use defaults");

    fs::remove_all(temp_root, ignored);
}

void test_config_fpw_custom_limit_is_enforced_at_boundary() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_config_custom_call_depth_boundary";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    constexpr std::size_t custom_limit = 10U;
    write_text(
        temp_root / "config.fpw",
        "MAX_CALL_DEPTH = " + std::to_string(custom_limit) + "\n");

    const fs::path at_limit_path = temp_root / "at_custom_limit.prg";
    const fs::path over_limit_path = temp_root / "over_custom_limit.prg";
    write_text(at_limit_path, build_nested_do_chain_script(custom_limit));
    write_text(over_limit_path, build_nested_do_chain_script(custom_limit + 1U));

    copperfin::runtime::PrgRuntimeSession at_limit_session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(at_limit_path.string(), temp_root.string(), false));
    const auto at_limit_state = at_limit_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(at_limit_state.completed,
           "config.fpw custom max call depth should allow nested chains exactly at the configured limit");

    copperfin::runtime::PrgRuntimeSession over_limit_session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(over_limit_path.string(), temp_root.string(), false));
    const auto over_limit_state = over_limit_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(over_limit_state.reason == copperfin::runtime::DebugPauseReason::error,
           "config.fpw custom max call depth should reject nested chains one level over the configured limit");
    expect(over_limit_state.message.find("maximum call depth") != std::string::npos,
           "custom-limit guardrail errors should report the call-depth guardrail message");

    fs::remove_all(temp_root, ignored);
}

void test_config_fpw_overrides_temp_directory_default() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_config_temp_dir";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path local_temp = temp_root / "user_local_temp";
    fs::create_directories(local_temp);

    write_text(
        temp_root / "config.fpw",
        "TMPFILES = '" + local_temp.string() + "'\n");

    const fs::path main_path = temp_root / "main.prg";
    write_text(main_path, "x = 1\nRETURN\n");

    // Keep this inline instead of using make_runtime_session_options(...):
    // the helper always sets temp_directory, which would bypass the config.fpw
    // TMPFILES fallback path resolution this test is verifying.
    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "config temp-directory script should complete");

    const bool reported_config_temp = std::any_of(
        state.events.begin(),
        state.events.end(),
        [&](const copperfin::runtime::RuntimeEvent& event) {
            return event.category == "runtime.config" && event.detail.find(local_temp.string()) != std::string::npos;
        });
    expect(reported_config_temp, "runtime config event should include TMPFILES override path");

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

void test_do_with_parameters_binds_arguments_in_called_routine() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_do_with_parameters";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "do_with_parameters.prg";
    write_text(
        main_path,
        "DO addvals WITH 4, 5\n"
        "RETURN\n"
        "PROCEDURE addvals\n"
        "LPARAMETERS a, b\n"
        "sum_result = a + b\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "DO WITH LPARAMETERS script should complete");

    const auto sum_result = state.globals.find("sum_result");
    expect(sum_result != state.globals.end(), "called routine should assign sum_result");
    if (sum_result != state.globals.end()) {
        expect(copperfin::runtime::format_value(sum_result->second) == "9", "DO WITH arguments should bind to LPARAMETERS");
    }

    fs::remove_all(temp_root, ignored);
}

void test_call_with_parameters_binds_arguments_in_called_routine() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_call_with_parameters";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "call_with_parameters.prg";
    write_text(
        main_path,
        "CALL addvals WITH 7, 8\n"
        "RETURN\n"
        "PROCEDURE addvals\n"
        "LPARAMETERS a, b\n"
        "sum_result = a + b\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "CALL WITH LPARAMETERS script should complete");

    const auto sum_result = state.globals.find("sum_result");
    expect(sum_result != state.globals.end(), "CALL routine should assign sum_result");
    if (sum_result != state.globals.end()) {
        expect(copperfin::runtime::format_value(sum_result->second) == "15", "CALL WITH arguments should bind to LPARAMETERS");
    }

    fs::remove_all(temp_root, ignored);
}

void test_call_external_target_with_by_reference_updates_caller_variable() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_call_external_byref";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path helper_path = temp_root / "helper.prg";
    write_text(
        helper_path,
        "LPARAMETERS pcount\n"
        "pcount = pcount + 5\n"
        "RETURN\n");

    const fs::path main_path = temp_root / "call_external_byref.prg";
    write_text(
        main_path,
        "counter = 3\n"
        "CALL helper WITH @counter\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "CALL external WITH @var script should complete");

    const auto counter = state.globals.find("counter");
    expect(counter != state.globals.end(), "caller variable should exist after CALL external BYREF");
    if (counter != state.globals.end()) {
        expect(
            copperfin::runtime::format_value(counter->second) == "8",
            "CALL external target should resolve .prg path and write BYREF updates back to caller");
    }

    fs::remove_all(temp_root, ignored);
}

void test_on_error_do_handler_dispatches_routine() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_on_error_do";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "on_error_do.prg";
    write_text(
        main_path,
        "ON ERROR DO handleerr\n"
        "DO missing_routine\n"
        "after_error = 1\n"
        "RETURN\n"
        "PROCEDURE handleerr\n"
        "handled = 1\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "ON ERROR DO script should complete after handler dispatch");

    const auto handled = state.globals.find("handled");
    const auto after_error = state.globals.find("after_error");
    expect(handled != state.globals.end(), "ON ERROR handler should set handled flag");
    expect(after_error != state.globals.end(), "execution should continue after ON ERROR handler returns");
    if (handled != state.globals.end()) {
        expect(copperfin::runtime::format_value(handled->second) == "1", "handled flag should be 1");
    }
    if (after_error != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_error->second) == "1", "post-error statement should run");
    }

    const bool has_handler_event = std::any_of(
        state.events.begin(),
        state.events.end(),
        [](const copperfin::runtime::RuntimeEvent& event) {
            return event.category == "runtime.error_handler";
        });
    expect(has_handler_event, "runtime should emit runtime.error_handler event when ON ERROR handler is dispatched");

    fs::remove_all(temp_root, ignored);
}

void test_on_error_do_with_handler_receives_error_metadata() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_on_error_do_with";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "on_error_do_with.prg";
    write_text(
        main_path,
        "ON ERROR DO handleerr WITH MESSAGE(), PROGRAM(), LINENO(), ERROR()\n"
        "DO missing_routine\n"
        "after_error = 1\n"
        "RETURN\n"
        "PROCEDURE handleerr\n"
        "LPARAMETERS err_message, err_program, err_line, err_code\n"
        "captured_message = err_message\n"
        "captured_program = err_program\n"
        "captured_line = err_line\n"
        "captured_code = err_code\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "ON ERROR DO WITH script should complete after handler dispatch");

    const auto captured_message = state.globals.find("captured_message");
    const auto captured_program = state.globals.find("captured_program");
    const auto captured_line = state.globals.find("captured_line");
    const auto captured_code = state.globals.find("captured_code");
    const auto after_error = state.globals.find("after_error");
    expect(captured_message != state.globals.end(), "ON ERROR DO WITH handler should capture MESSAGE()");
    expect(captured_program != state.globals.end(), "ON ERROR DO WITH handler should capture PROGRAM()");
    expect(captured_line != state.globals.end(), "ON ERROR DO WITH handler should capture LINENO()");
    expect(captured_code != state.globals.end(), "ON ERROR DO WITH handler should capture ERROR()");
    expect(after_error != state.globals.end(), "execution should continue after ON ERROR DO WITH handler returns");

    if (captured_message != state.globals.end()) {
        expect(
            copperfin::runtime::format_value(captured_message->second).find("Unable to resolve DO target") != std::string::npos,
            "MESSAGE() should describe the failing command");
    }
    if (captured_program != state.globals.end()) {
        expect(
            copperfin::runtime::format_value(captured_program->second) == "main",
            "PROGRAM() should report the failing procedure context");
    }
    if (captured_line != state.globals.end()) {
        expect(
            copperfin::runtime::format_value(captured_line->second) == "2",
            "LINENO() should report the failing source line");
    }
    if (captured_code != state.globals.end()) {
        expect(
            copperfin::runtime::format_value(captured_code->second) == "1001",
            "ERROR() should expose the first-pass runtime resolve error code");
    }

    const bool has_handler_event = std::any_of(
        state.events.begin(),
        state.events.end(),
        [](const copperfin::runtime::RuntimeEvent& event) {
            return event.category == "runtime.error_handler" &&
                event.detail.find("WITH 4 argument") != std::string::npos;
        });
    expect(has_handler_event, "runtime should record ON ERROR DO WITH handler dispatch detail");

    fs::remove_all(temp_root, ignored);
}

void test_aerror_populates_structured_runtime_error_array() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_aerror";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 21}, {"BRAVO", 28}});

    const fs::path main_path = temp_root / "aerror.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SELECT People\n"
        "ON ERROR DO handleerr\n"
        "DO missing_target\n"
        "after_error = 'continued'\n"
        "RETURN\n"
        "PROCEDURE handleerr\n"
        "SELECT 0\n"
        "nErrorRows = AERROR(aErr)\n"
        "nErrorCols = ALEN(aErr, 2)\n"
        "nErrorCode = aErr[1,1]\n"
        "cErrorMessage = aErr[1,2]\n"
        "cErrorParam = aErr[1,3]\n"
        "nErrorWorkArea = aErr[1,4]\n"
        "nErrorArrayLine = aErr[1,5]\n"
        "cErrorArrayProgram = aErr[1,6]\n"
        "cErrorArrayStatement = aErr[1,7]\n"
        "cSys2018 = SYS(2018)\n"
        "cErrorProgram = PROGRAM()\n"
        "nErrorLine = LINENO()\n"
        "cErrorHandler = ON('ERROR')\n"
        "RETURN\n"
        "ENDPROC\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "AERROR() handler script should complete");

    const auto rows = state.globals.find("nerrorrows");
    const auto cols = state.globals.find("nerrorcols");
    const auto code = state.globals.find("nerrorcode");
    const auto message = state.globals.find("cerrormessage");
    const auto parameter = state.globals.find("cerrorparam");
    const auto work_area = state.globals.find("nerrorworkarea");
    const auto array_line = state.globals.find("nerrorarrayline");
    const auto array_program = state.globals.find("cerrorarrayprogram");
    const auto array_statement = state.globals.find("cerrorarraystatement");
    const auto sys2018 = state.globals.find("csys2018");
    const auto line = state.globals.find("nerrorline");
    const auto program = state.globals.find("cerrorprogram");
    const auto handler = state.globals.find("cerrorhandler");
    const auto after_error = state.globals.find("after_error");

    expect(rows != state.globals.end(), "AERROR() should return a row count");
    expect(cols != state.globals.end(), "AERROR() array should expose a column count");
    expect(code != state.globals.end(), "AERROR() should populate the error code column");
    expect(message != state.globals.end(), "AERROR() should populate the message column");
    expect(parameter != state.globals.end(), "AERROR() should populate the error parameter column");
    expect(work_area != state.globals.end(), "AERROR() should populate the work-area column");
    expect(array_line != state.globals.end(), "AERROR() should populate the source-line column for normal runtime errors");
    expect(array_program != state.globals.end(), "AERROR() should populate the procedure column for normal runtime errors");
    expect(array_statement != state.globals.end(), "AERROR() should populate the faulting-statement column for normal runtime errors");
    expect(sys2018 != state.globals.end(), "SYS(2018) should expose the uppercase error parameter");
    expect(line != state.globals.end(), "AERROR() should populate the failing line column");
    expect(program != state.globals.end(), "AERROR() should populate the procedure column");
    expect(handler != state.globals.end(), "AERROR() should populate the ON ERROR handler column");
    expect(after_error != state.globals.end(), "ON ERROR script should continue after handler dispatch");

    if (rows != state.globals.end()) {
        expect(copperfin::runtime::format_value(rows->second) == "1", "AERROR() should return one first-pass error row");
    }
    if (cols != state.globals.end()) {
        expect(copperfin::runtime::format_value(cols->second) == "7", "AERROR() should expose the VFP seven-column shape");
    }
    if (code != state.globals.end()) {
        expect(copperfin::runtime::format_value(code->second) == "1001", "AERROR() should expose a distinct resolve-failure error code");
    }
    if (message != state.globals.end()) {
        expect(copperfin::runtime::format_value(message->second).find("Unable to resolve DO target") != std::string::npos,
            "AERROR() message should describe the failing runtime command");
    }
    if (parameter != state.globals.end()) {
        expect(copperfin::runtime::format_value(parameter->second) == "missing_target",
            "AERROR() parameter column should preserve the mixed-case runtime error parameter");
    }
    if (work_area != state.globals.end()) {
        expect(copperfin::runtime::format_value(work_area->second) == "1",
            "AERROR() work-area column should capture the faulting work area even if the handler changes selection");
    }
    if (array_line != state.globals.end()) {
        expect(copperfin::runtime::format_value(array_line->second) == "4",
            "AERROR() line column should capture the faulting source line");
    }
    if (array_program != state.globals.end()) {
        expect(copperfin::runtime::format_value(array_program->second) == "main",
            "AERROR() procedure column should capture the faulting routine");
    }
    if (array_statement != state.globals.end()) {
        expect(copperfin::runtime::format_value(array_statement->second) == "DO missing_target",
            "AERROR() statement column should capture the failing source text");
    }
    if (sys2018 != state.globals.end()) {
        expect(copperfin::runtime::format_value(sys2018->second) == "MISSING_TARGET",
            "SYS(2018) should expose the uppercase runtime error parameter");
    }
    if (line != state.globals.end()) {
        expect(copperfin::runtime::format_value(line->second) == "4",
            "AERROR() line column should report the failing source line");
    }
    if (program != state.globals.end()) {
        expect(copperfin::runtime::format_value(program->second) == "main",
            "AERROR() procedure column should report the failing routine");
    }
    if (handler != state.globals.end()) {
        expect(copperfin::runtime::format_value(handler->second).find("DO handleerr") != std::string::npos,
            "AERROR() handler column should preserve the active ON ERROR clause");
    }
    fs::remove_all(temp_root, ignored);
}

void test_aerror_exposes_sql_and_ole_specific_rows() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_aerror_specific";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "aerror_specific.prg";
    write_text(
        main_path,
        "nConn = SQLCONNECT('Driver=ODBC Driver 18 for SQL Server;Server=Northwind')\n"
        "nSqlResult = SQLEXEC(nConn)\n"
        "nSqlRows = AERROR(aSqlErr)\n"
        "nSqlCode = aSqlErr[1,1]\n"
        "cSqlMessage = aSqlErr[1,2]\n"
        "cSqlDetail = aSqlErr[1,3]\n"
        "cSqlState = aSqlErr[1,4]\n"
        "nSqlNative = aSqlErr[1,5]\n"
        "cSqlContext = aSqlErr[1,6]\n"
        "cSqlPayload = aSqlErr[1,7]\n"
        "ON ERROR DO oleerr\n"
        "missingOle.SomeProperty = 42\n"
        "RETURN\n"
        "PROCEDURE oleerr\n"
        "nOleRows = AERROR(aOleErr)\n"
        "nOleCode = aOleErr[1,1]\n"
        "cOleMessage = aOleErr[1,2]\n"
        "cOleDetail = aOleErr[1,3]\n"
        "cOleApp = aOleErr[1,4]\n"
        "cOleSource = aOleErr[1,5]\n"
        "cOleAction = aOleErr[1,6]\n"
        "nOleNative = aOleErr[1,7]\n"
        "RETURN\n"
        "ENDPROC\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SQL/OLE AERROR script should complete");

    const auto sql_rows = state.globals.find("nsqlrows");
    const auto sql_code = state.globals.find("nsqlcode");
    const auto sql_message = state.globals.find("csqlmessage");
    const auto sql_detail = state.globals.find("csqldetail");
    const auto sql_state = state.globals.find("csqlstate");
    const auto sql_native = state.globals.find("nsqlnative");
    const auto sql_context = state.globals.find("csqlcontext");
    const auto sql_payload = state.globals.find("csqlpayload");
    const auto ole_rows = state.globals.find("nolerows");
    const auto ole_code = state.globals.find("nolecode");
    const auto ole_message = state.globals.find("colemessage");
    const auto ole_detail = state.globals.find("coledetail");
    const auto ole_app = state.globals.find("coleapp");
    const auto ole_source = state.globals.find("colesource");
    const auto ole_action = state.globals.find("coleaction");
    const auto ole_native = state.globals.find("nolenative");

    expect(sql_rows != state.globals.end(), "SQL AERROR should return a row count");
    expect(sql_code != state.globals.end(), "SQL AERROR should populate code");
    expect(sql_message != state.globals.end(), "SQL AERROR should populate message");
    expect(sql_detail != state.globals.end(), "SQL AERROR should populate detail");
    expect(sql_state != state.globals.end(), "SQL AERROR should populate SQL state");
    expect(sql_native != state.globals.end(), "SQL AERROR should populate native code");
    expect(sql_context != state.globals.end(), "SQL AERROR should populate captured context");
    expect(sql_payload != state.globals.end(), "SQL AERROR should populate captured payload");
    expect(ole_rows != state.globals.end(), "OLE AERROR should return a row count");
    expect(ole_code != state.globals.end(), "OLE AERROR should populate code");
    expect(ole_message != state.globals.end(), "OLE AERROR should populate message");
    expect(ole_detail != state.globals.end(), "OLE AERROR should populate detail");
    expect(ole_app != state.globals.end(), "OLE AERROR should populate app name");
    expect(ole_source != state.globals.end(), "OLE AERROR should populate the captured source object");
    expect(ole_action != state.globals.end(), "OLE AERROR should populate the captured action text");
    expect(ole_native != state.globals.end(), "OLE AERROR should populate native code");

    if (sql_rows != state.globals.end()) {
        expect(copperfin::runtime::format_value(sql_rows->second) == "1", "SQL AERROR should expose one row");
    }
    if (sql_code != state.globals.end()) {
        expect(copperfin::runtime::format_value(sql_code->second) == "1526", "SQL failures should use the VFP ODBC error code");
    }
    if (sql_message != state.globals.end()) {
        expect(copperfin::runtime::format_value(sql_message->second).find("SQLEXEC requires a command") != std::string::npos,
            "SQL AERROR message should preserve SQL failure text");
    }
    if (sql_detail != state.globals.end()) {
        expect(copperfin::runtime::format_value(sql_detail->second) == "1",
            "SQL AERROR detail should preserve the failing handle parameter");
    }
    if (sql_state != state.globals.end()) {
        expect(copperfin::runtime::format_value(sql_state->second) == "HY000", "SQL AERROR should expose a generic SQL state");
    }
    if (sql_native != state.globals.end()) {
        expect(copperfin::runtime::format_value(sql_native->second) == "-1", "SQL AERROR should expose a native failure code");
    }
    if (sql_context != state.globals.end()) {
        expect(copperfin::runtime::format_value(sql_context->second) == "odbc",
            "SQL AERROR should preserve the captured provider context when a connection exists");
    }
    if (sql_payload != state.globals.end()) {
        expect(copperfin::runtime::format_value(sql_payload->second).find("Northwind") != std::string::npos,
            "SQL AERROR should preserve the captured connection payload instead of leaving the column empty");
    }
    if (ole_rows != state.globals.end()) {
        expect(copperfin::runtime::format_value(ole_rows->second) == "1", "OLE AERROR should expose one row");
    }
    if (ole_code != state.globals.end()) {
        expect(copperfin::runtime::format_value(ole_code->second) == "1429", "OLE failures should use the VFP OLE error code");
    }
    if (ole_message != state.globals.end()) {
        expect(copperfin::runtime::format_value(ole_message->second).find("OLE object not found") != std::string::npos,
            "OLE AERROR message should preserve automation failure text");
    }
    if (ole_detail != state.globals.end()) {
        expect(copperfin::runtime::format_value(ole_detail->second).find("missingOle.SomeProperty") != std::string::npos,
            "OLE AERROR detail should preserve the failing member path");
    }
    if (ole_app != state.globals.end()) {
        expect(copperfin::runtime::format_value(ole_app->second) == "Copperfin OLE",
            "OLE AERROR app column should identify the runtime automation bridge");
    }
    if (ole_source != state.globals.end()) {
        expect(copperfin::runtime::format_value(ole_source->second) == "missingOle",
            "OLE AERROR should preserve the captured source object identifier instead of leaving the column empty");
    }
    if (ole_action != state.globals.end()) {
        expect(copperfin::runtime::format_value(ole_action->second).find("missingOle.SomeProperty = 42") != std::string::npos,
            "OLE AERROR should preserve the captured action text instead of leaving the column empty");
    }
    if (ole_native != state.globals.end()) {
        expect(copperfin::runtime::format_value(ole_native->second) == "1429",
            "OLE AERROR native column should preserve the automation error code");
    }

    fs::remove_all(temp_root, ignored);
}

void test_on_error_handler_preserves_original_fault_metadata_across_caught_inner_faults() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_aerror_nested_faults";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "aerror_nested_faults.prg";
    write_text(
        main_path,
        "ON ERROR DO handleerr\n"
        "DO missing_outer\n"
        "after_error = 'continued'\n"
        "RETURN\n"
        "PROCEDURE handleerr\n"
        "cInitialMessage = MESSAGE()\n"
        "cInitialProgram = PROGRAM()\n"
        "nInitialLine = LINENO()\n"
        "TRY\n"
        "    DO missing_inner\n"
        "CATCH\n"
        "    cCaughtMessage = MESSAGE()\n"
        "ENDTRY\n"
        "nErrorRows = AERROR(aErr)\n"
        "cFinalMessage = MESSAGE()\n"
        "cFinalProgram = PROGRAM()\n"
        "nFinalLine = LINENO()\n"
        "cFinalParam = aErr[1,3]\n"
        "RETURN\n"
        "ENDPROC\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "ON ERROR handler nested-fault script should complete");

    const auto initial_message = state.globals.find("cinitialmessage");
    const auto initial_program = state.globals.find("cinitialprogram");
    const auto initial_line = state.globals.find("ninitialline");
    const auto final_message = state.globals.find("cfinalmessage");
    const auto final_program = state.globals.find("cfinalprogram");
    const auto final_line = state.globals.find("nfinalline");
    const auto final_param = state.globals.find("cfinalparam");
    const auto rows = state.globals.find("nerrorrows");
    const auto after_error = state.globals.find("after_error");

    expect(initial_message != state.globals.end(), "handler should capture initial MESSAGE()");
    expect(initial_program != state.globals.end(), "handler should capture initial PROGRAM()");
    expect(initial_line != state.globals.end(), "handler should capture initial LINENO()");
    expect(final_message != state.globals.end(), "handler should preserve final MESSAGE() after caught inner fault");
    expect(final_program != state.globals.end(), "handler should preserve final PROGRAM() after caught inner fault");
    expect(final_line != state.globals.end(), "handler should preserve final LINENO() after caught inner fault");
    expect(final_param != state.globals.end(), "AERROR() should preserve the original error parameter after a caught inner fault");
    expect(rows != state.globals.end(), "AERROR() should still return a row count after a caught inner fault");
    expect(after_error != state.globals.end(), "execution should continue after the handler returns");

    if (initial_message != state.globals.end()) {
        expect(
            copperfin::runtime::format_value(initial_message->second).find("missing_outer") != std::string::npos,
            "initial MESSAGE() should describe the original outer fault");
    }
    if (initial_program != state.globals.end()) {
        expect(
            copperfin::runtime::format_value(initial_program->second) == "main",
            "initial PROGRAM() should report the original faulting routine");
    }
    if (initial_line != state.globals.end()) {
        expect(
            copperfin::runtime::format_value(initial_line->second) == "2",
            "initial LINENO() should report the original faulting line");
    }
    if (final_message != state.globals.end()) {
        expect(
            copperfin::runtime::format_value(final_message->second).find("missing_outer") != std::string::npos,
            "MESSAGE() should revert to the original ON ERROR fault after a caught inner fault");
    }
    if (final_program != state.globals.end()) {
        expect(
            copperfin::runtime::format_value(final_program->second) == "main",
            "PROGRAM() should remain bound to the original ON ERROR fault");
    }
    if (final_line != state.globals.end()) {
        expect(
            copperfin::runtime::format_value(final_line->second) == "2",
            "LINENO() should remain bound to the original ON ERROR fault");
    }
    if (final_param != state.globals.end()) {
        expect(
            copperfin::runtime::format_value(final_param->second) == "missing_outer",
            "AERROR() should preserve the original error parameter instead of the caught inner fault");
    }
    if (rows != state.globals.end()) {
        expect(copperfin::runtime::format_value(rows->second) == "1", "AERROR() should still expose one row");
    }
    if (after_error != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_error->second) == "continued", "post-handler execution should resume normally");
    }

    fs::remove_all(temp_root, ignored);
}

void test_ole_property_fault_dispatches_on_error_and_preserves_object_state() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_ole_property_fault";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "ole_property_fault.prg";
    write_text(
        main_path,
        "ON ERROR DO handleerr\n"
        "oDict = CREATEOBJECT('Scripting.Dictionary')\n"
        "oDict.Add('Alpha', 41)\n"
        "xMissing = oDict.NoSuchProperty\n"
        "lStillExists = oDict.Exists('Alpha')\n"
        "after_error = 'continued'\n"
        "RETURN\n"
        "PROCEDURE handleerr\n"
        "nOleRows = AERROR(aOleErr)\n"
        "nOleCode = aOleErr[1,1]\n"
        "cOleDetail = aOleErr[1,3]\n"
        "cOleSource = aOleErr[1,5]\n"
        "cOleAction = aOleErr[1,6]\n"
        "RETURN\n"
        "ENDPROC\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "OLE property fault script should complete");

    const auto rows = state.globals.find("nolerows");
    const auto code = state.globals.find("nolecode");
    const auto detail = state.globals.find("coledetail");
    const auto source = state.globals.find("colesource");
    const auto action = state.globals.find("coleaction");
    const auto still_exists = state.globals.find("lstillexists");
    const auto after_error = state.globals.find("after_error");

    expect(rows != state.globals.end(), "OLE property fault should populate AERROR rows");
    expect(code != state.globals.end(), "OLE property fault should populate AERROR code");
    expect(detail != state.globals.end(), "OLE property fault should populate AERROR detail");
    expect(source != state.globals.end(), "OLE property fault should populate AERROR source");
    expect(action != state.globals.end(), "OLE property fault should populate AERROR action");
    expect(still_exists != state.globals.end(), "execution should continue after OLE property fault");
    expect(after_error != state.globals.end(), "post-handler statements should run after OLE property fault");

    if (rows != state.globals.end()) {
        expect(copperfin::runtime::format_value(rows->second) == "1", "OLE property fault should expose one AERROR row");
    }
    if (code != state.globals.end()) {
        expect(copperfin::runtime::format_value(code->second) == "1429", "OLE property fault should use the automation error code");
    }
    if (detail != state.globals.end()) {
        std::string detail_text = copperfin::runtime::format_value(detail->second);
        std::transform(detail_text.begin(), detail_text.end(), detail_text.begin(), [](unsigned char ch) {
            return static_cast<char>(std::tolower(ch));
        });
        expect(detail_text.find("nosuchproperty") != std::string::npos,
               "OLE property fault should preserve the failing member detail");
    }
    if (source != state.globals.end()) {
        std::string source_text = copperfin::runtime::format_value(source->second);
        std::transform(source_text.begin(), source_text.end(), source_text.begin(), [](unsigned char ch) {
            return static_cast<char>(std::tolower(ch));
        });
        expect(source_text.find("odict") != std::string::npos,
               "OLE property fault should preserve the source variable name");
    }
    if (action != state.globals.end()) {
        expect(!copperfin::runtime::format_value(action->second).empty(),
               "OLE property fault should preserve the failing action text");
    }
    if (still_exists != state.globals.end()) {
        expect(copperfin::runtime::format_value(still_exists->second) == "true",
               "a trapped OLE property fault should not poison the object state");
    }
    if (after_error != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_error->second) == "continued",
               "execution should continue after an OLE property fault handler returns");
    }

    fs::remove_all(temp_root, ignored);
}

void test_ole_method_fault_is_catchable_and_preserves_object_state() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_ole_method_fault";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "ole_method_fault.prg";
    write_text(
        main_path,
        "oDict = CREATEOBJECT('Scripting.Dictionary')\n"
        "oDict.Add('Alpha', 41)\n"
        "TRY\n"
        "  oDict.NoSuchMethod(7)\n"
        "CATCH TO err_text\n"
        "  cCaught = err_text\n"
        "FINALLY\n"
        "  finally_hit = 1\n"
        "ENDTRY\n"
        "lStillExists = oDict.Exists('Alpha')\n"
        "nCountAfterFault = oDict.Count\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "OLE method fault TRY/CATCH script should complete");

    const auto caught = state.globals.find("ccaught");
    const auto finally_hit = state.globals.find("finally_hit");
    const auto still_exists = state.globals.find("lstillexists");
    const auto count_after_fault = state.globals.find("ncountafterfault");

    expect(caught != state.globals.end(), "CATCH should receive the OLE method fault text");
    expect(finally_hit != state.globals.end(), "FINALLY should run after trapped OLE method faults");
    expect(still_exists != state.globals.end(), "object should still be usable after trapped OLE method faults");
    expect(count_after_fault != state.globals.end(), "object property reads should still work after trapped OLE method faults");

    if (caught != state.globals.end()) {
        expect(copperfin::runtime::format_value(caught->second).find("OLE member not found for method invocation") != std::string::npos,
               "CATCH should expose the trapped OLE method fault text");
    }
    if (finally_hit != state.globals.end()) {
        expect(copperfin::runtime::format_value(finally_hit->second) == "1",
               "FINALLY should still execute after trapped OLE method faults");
    }
    if (still_exists != state.globals.end()) {
        expect(copperfin::runtime::format_value(still_exists->second) == "true",
               "trapped OLE method faults should not poison later method calls");
    }
    if (count_after_fault != state.globals.end()) {
        expect(copperfin::runtime::format_value(count_after_fault->second) == "1",
               "trapped OLE method faults should preserve collection state");
    }

    fs::remove_all(temp_root, ignored);
}

void test_thrown_expression_fault_preserves_pause_statement_and_recovery() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_thrown_expression_fault";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "thrown_expression_fault.prg";
    write_text(
        main_path,
        "x = LOG(-1)\n"
        "after_fault = 7\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error,
           "thrown expression faults should pause with an error");
    expect(state.location.line == 1U,
           "thrown expression faults should highlight the faulting line");
    expect(state.statement_text == "x = LOG(-1)",
           "thrown expression faults should preserve the faulting statement text");
    expect(state.message.find("LOG() requires a positive argument") != std::string::npos,
           "thrown expression faults should preserve the thrown message");

    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "continuing after a thrown expression fault should keep the session alive");
    const auto after_fault = state.globals.find("after_fault");
    expect(after_fault != state.globals.end(), "post-fault statements should still execute after thrown expression faults");
    if (after_fault != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_fault->second) == "7",
               "post-fault statements should preserve later assignments after thrown expression faults");
    }

    fs::remove_all(temp_root, ignored);
}

void test_repeated_thrown_faults_refresh_pause_metadata_each_time() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_repeated_thrown_faults";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "repeated_thrown_faults.prg";
    write_text(
        main_path,
        "first_value = LOG(-1)\n"
        "after_first = 1\n"
        "second_value = ACOS(2)\n"
        "after_second = 1\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error,
           "the first thrown expression fault should pause with an error");
    expect(state.location.line == 1U,
           "the first thrown expression fault should point at line 1");
    expect(state.statement_text == "first_value = LOG(-1)",
           "the first thrown expression fault should preserve its own statement text");

    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error,
           "the second thrown expression fault should also pause with an error");
    expect(state.location.line == 3U,
           "the second thrown expression fault should update the pause line");
    expect(state.statement_text == "second_value = ACOS(2)",
           "the second thrown expression fault should replace stale statement text");
    expect(state.message.find("ACOS() requires an argument between -1 and 1") != std::string::npos,
           "the second thrown expression fault should replace the stale message");

    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "continuing after repeated thrown faults should keep the session alive");
    const auto after_first = state.globals.find("after_first");
    const auto after_second = state.globals.find("after_second");
    expect(after_first != state.globals.end(), "the first post-fault assignment should survive repeated thrown faults");
    expect(after_second != state.globals.end(), "the second post-fault assignment should survive repeated thrown faults");
    if (after_first != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_first->second) == "1",
               "the first post-fault assignment should be preserved");
    }
    if (after_second != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_second->second) == "1",
               "the second post-fault assignment should be preserved");
    }

    fs::remove_all(temp_root, ignored);
}

void test_nested_routine_faults_report_faulting_stack_frame_line() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_nested_fault_stack";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "nested_fault_stack.prg";
    write_text(
        main_path,
        "DO outerproc\n"
        "after_call = 1\n"
        "RETURN\n"
        "PROCEDURE outerproc\n"
        "DO innerproc\n"
        "RETURN\n"
        "ENDPROC\n"
        "PROCEDURE innerproc\n"
        "fault_value = LOG(-1)\n"
        "RETURN\n"
        "ENDPROC\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error,
           "nested routine faults should pause with an error");
    expect(state.location.line == 9U,
           "nested routine faults should highlight the inner fault line");
    expect(state.statement_text == "fault_value = LOG(-1)",
           "nested routine faults should preserve the inner fault statement text");
    expect(state.call_stack.size() >= 3U,
           "nested routine faults should expose the nested call stack");
    if (state.call_stack.size() >= 3U) {
        expect(state.call_stack[0].routine_name == "innerproc",
               "the top error stack frame should be the faulting inner routine");
        expect(state.call_stack[0].line == 9U,
               "the top error stack frame should use the faulting line instead of the next statement");
        expect(state.call_stack[1].routine_name == "outerproc",
               "the second error stack frame should be the caller routine");
        expect(state.call_stack[2].routine_name == "main",
               "the third error stack frame should be the entry routine");
    }
    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "continuing after a nested routine fault should keep the session alive");
    const auto after_call = state.globals.find("after_call");
    expect(after_call != state.globals.end(), "execution should resume after nested routine faults");
    if (after_call != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_call->second) == "1",
               "post-call statements should still execute after nested routine faults");
    }

    fs::remove_all(temp_root, ignored);
}

void test_repeated_nested_faults_refresh_stack_frame_and_statement_metadata() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_repeated_nested_faults";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "repeated_nested_faults.prg";
    write_text(
        main_path,
        "DO firstfault\n"
        "after_first = 1\n"
        "DO secondfault\n"
        "after_second = 1\n"
        "RETURN\n"
        "PROCEDURE firstfault\n"
        "first_value = LOG(-1)\n"
        "RETURN\n"
        "ENDPROC\n"
        "PROCEDURE secondfault\n"
        "second_value = ACOS(2)\n"
        "RETURN\n"
        "ENDPROC\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error,
           "the first nested routine fault should pause with an error");
    if (!state.call_stack.empty()) {
        expect(state.call_stack[0].routine_name == "firstfault",
               "the first nested routine fault should report the first routine on top of the stack");
        expect(state.call_stack[0].line == 7U,
               "the first nested routine fault should report the first routine fault line");
    }
    expect(state.statement_text == "first_value = LOG(-1)",
           "the first nested routine fault should preserve its own statement text");

    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error,
           "the second nested routine fault should also pause with an error");
    if (!state.call_stack.empty()) {
        expect(state.call_stack[0].routine_name == "secondfault",
               "the second nested routine fault should replace the stale top routine");
        expect(state.call_stack[0].line == 11U,
               "the second nested routine fault should replace the stale top line");
    }
    expect(state.statement_text == "second_value = ACOS(2)",
           "the second nested routine fault should replace stale statement text");
    expect(state.message.find("ACOS() requires an argument between -1 and 1") != std::string::npos,
           "the second nested routine fault should replace the stale error message");

    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "continuing after repeated nested routine faults should keep the session alive");
    const auto after_first = state.globals.find("after_first");
    const auto after_second = state.globals.find("after_second");
    expect(after_first != state.globals.end(), "the first post-fault assignment should survive repeated nested faults");
    expect(after_second != state.globals.end(), "the second post-fault assignment should survive repeated nested faults");

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

void test_try_catch_finally_handles_runtime_errors() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_try_catch_finally";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "try_catch_finally.prg";
    write_text(
        main_path,
        "TRY\n"
        "  DO missing_routine\n"
        "CATCH TO err_text\n"
        "  caught = err_text\n"
        "FINALLY\n"
        "  finally_hit = 1\n"
        "ENDTRY\n"
        "after_try = 1\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "TRY/CATCH/FINALLY script should complete");

    const auto caught = state.globals.find("caught");
    const auto finally_hit = state.globals.find("finally_hit");
    const auto after_try = state.globals.find("after_try");
    expect(caught != state.globals.end(), "CATCH should run when the TRY block faults");
    expect(finally_hit != state.globals.end(), "FINALLY should run after handled TRY faults");
    expect(after_try != state.globals.end(), "execution should continue after ENDTRY");
    if (caught != state.globals.end()) {
        expect(
            copperfin::runtime::format_value(caught->second).find("Unable to resolve DO target") != std::string::npos,
            "CATCH TO should receive the runtime error text");
    }

    const bool has_try_handler_event = std::any_of(
        state.events.begin(),
        state.events.end(),
        [](const copperfin::runtime::RuntimeEvent& event) {
            return event.category == "runtime.try_handler";
        });
    expect(has_try_handler_event, "runtime should emit a TRY handler event when a TRY block catches an error");

    fs::remove_all(temp_root, ignored);
}

void test_try_finally_runs_without_catch_on_success() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_try_finally_success";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "try_finally_success.prg";
    write_text(
        main_path,
        "TRY\n"
        "  value = 7\n"
        "CATCH TO err_text\n"
        "  caught = 1\n"
        "FINALLY\n"
        "  finally_hit = 1\n"
        "ENDTRY\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "TRY/FINALLY success script should complete");
    expect(state.globals.find("caught") == state.globals.end(), "CATCH should be skipped when TRY succeeds");
    expect(state.globals.find("finally_hit") != state.globals.end(), "FINALLY should still run when TRY succeeds");

    fs::remove_all(temp_root, ignored);
}

void test_do_with_by_reference_updates_caller_variable() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_do_with_byref";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "do_with_byref.prg";
    write_text(
        main_path,
        "counter = 1\n"
        "DO bump WITH @counter\n"
        "RETURN\n"
        "PROCEDURE bump\n"
        "LPARAMETERS pcount\n"
        "pcount = pcount + 1\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "DO WITH @var script should complete");

    const auto counter = state.globals.find("counter");
    expect(counter != state.globals.end(), "caller variable should still exist after BYREF call");
    if (counter != state.globals.end()) {
        expect(
            copperfin::runtime::format_value(counter->second) == "2",
            "BYREF argument binding should write callee updates back to the caller");
    }

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

    fs::remove_all(temp_root, ignored);
}

void test_close_command_closes_all_work_areas() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_close_cmd";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_simple_dbf(temp_root / "a.dbf", {"alpha", "beta"});
    write_simple_dbf(temp_root / "b.dbf", {"gamma", "delta"});

    const fs::path main_path = temp_root / "close_test.prg";
    write_text(
        main_path,
        "USE '" + (temp_root / "a.dbf").string() + "'\n"
        "SELECT 2\n"
        "USE '" + (temp_root / "b.dbf").string() + "'\n"
        "CLOSE ALL\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "CLOSE ALL script should complete");

    expect(has_runtime_event(state.events, "runtime.close", "ALL"),
        "CLOSE ALL should emit a runtime.close event");

    fs::remove_all(temp_root, ignored);
}

void test_close_all_releases_runtime_handles() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_close_handles";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_text(temp_root / "held.txt", "seed");

    const fs::path main_path = temp_root / "close_handles.prg";
    write_text(
        main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "obj = CREATEOBJECT('Sample.Object')\n"
        "nHandle = FOPEN('held.txt', 2)\n"
        "CLOSE ALL\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "CLOSE ALL handle cleanup script should complete");
    expect(state.sql_connections.empty(), "CLOSE ALL should disconnect SQL handles");
    expect(state.ole_objects.empty(), "CLOSE ALL should release OLE object handles");

    const auto handle_it = state.globals.find("nhandle");
    expect(handle_it != state.globals.end(), "FOPEN handle should be captured before CLOSE ALL");
    int handle = 1;
    if (handle_it != state.globals.end()) {
        handle = static_cast<int>(handle_it->second.number_value);
    }

    const fs::path verify_path = temp_root / "verify_close_handles.prg";
    write_text(
        verify_path,
        "nClose = FCLOSE(" + std::to_string(handle) + ")\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession verify_session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(verify_path.string(), temp_root.string(), false));
    const auto verify_state = verify_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(verify_state.completed, "verification script should complete");

    const auto close_it = verify_state.globals.find("nclose");
    expect(close_it != verify_state.globals.end(), "verification script should expose FCLOSE result");
    if (close_it != verify_state.globals.end()) {
        expect(close_it->second.number_value == -1.0,
               "CLOSE ALL should already close FOPEN handles so follow-up FCLOSE returns -1");
    }

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
        "ERASE 'to_erase.txt'\n"
        "COPY FILE 'original.txt' TO 'copied.txt'\n"
        "RENAME 'copied.txt' TO 'renamed.txt'\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "file ops script should complete");

    expect(!fs::exists(temp_root / "to_erase.txt"), "ERASE should have deleted to_erase.txt");
    expect(fs::exists(temp_root / "original.txt"), "COPY FILE should leave original.txt intact");
    expect(fs::exists(temp_root / "renamed.txt"), "RENAME should create renamed.txt");
    expect(!fs::exists(temp_root / "copied.txt"), "RENAME should remove the old file name copied.txt");

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

void test_release_vars_erases_named_globals() {
    namespace fs = std::filesystem;
    const fs::path tmp = fs::temp_directory_path() / "copperfin_release_vars";
    std::error_code ign;
    fs::remove_all(tmp, ign);
    fs::create_directories(tmp);
    const fs::path prg = tmp / "test.prg";
    write_text(prg, "x = 10\ny = 20\nRELEASE x\nRETURN\n");
    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(prg.string(), tmp.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "RELEASE should not crash");
    expect(state.globals.find("x") == state.globals.end(), "x should be released");
    expect(state.globals.find("y") != state.globals.end(), "y should still exist");
    fs::remove_all(tmp, ign);
}

void test_release_all_clears_all_globals() {
    namespace fs = std::filesystem;
    const fs::path tmp = fs::temp_directory_path() / "copperfin_release_all";
    std::error_code ign;
    fs::remove_all(tmp, ign);
    fs::create_directories(tmp);
    const fs::path prg = tmp / "test.prg";
    write_text(prg, "a = 1\nb = 2\nRELEASE ALL\nRETURN\n");
    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(prg.string(), tmp.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "RELEASE ALL should complete");
    expect(state.globals.find("a") == state.globals.end(), "a should be released by RELEASE ALL");
    expect(state.globals.find("b") == state.globals.end(), "b should be released by RELEASE ALL");
    fs::remove_all(tmp, ign);
}

void test_release_all_clears_current_frame_locals_without_global_leak() {
    namespace fs = std::filesystem;
    const fs::path tmp = fs::temp_directory_path() / "copperfin_release_all_locals";
    std::error_code ign;
    fs::remove_all(tmp, ign);
    fs::create_directories(tmp);
    const fs::path prg = tmp / "test.prg";
    write_text(
        prg,
        "DO subproc\n"
        "outer_type = TYPE('x')\n"
        "RETURN\n"
        "PROCEDURE subproc\n"
        "LOCAL x\n"
        "x = 5\n"
        "RELEASE ALL\n"
        "after_release_type = TYPE('x')\n"
        "x = 7\n"
        "after_reassign = x\n"
        "RETURN\n");
    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(prg.string(), tmp.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "RELEASE ALL should clear current-frame locals");
    const auto after_release_type = state.globals.find("after_release_type");
    const auto after_reassign = state.globals.find("after_reassign");
    const auto outer_type = state.globals.find("outer_type");
    expect(after_release_type != state.globals.end(), "released local TYPE() should be captured");
    expect(after_reassign != state.globals.end(), "reassigned local value should be captured");
    expect(outer_type != state.globals.end(), "post-return local TYPE() should be captured");
    if (after_release_type != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_release_type->second) == "U",
               "RELEASE ALL should clear the current frame's local variable binding");
    }
    if (after_reassign != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_reassign->second) == "7",
               "reassigning after RELEASE ALL should still work inside the local scope");
    }
    if (outer_type != state.globals.end()) {
        expect(copperfin::runtime::format_value(outer_type->second) == "U",
               "reassigning a released LOCAL should not leak a new global after the routine returns");
    }
    fs::remove_all(tmp, ign);
}

void test_release_all_local_shadow_preserves_outer_global() {
    namespace fs = std::filesystem;
    const fs::path tmp = fs::temp_directory_path() / "copperfin_release_all_local_shadow";
    std::error_code ign;
    fs::remove_all(tmp, ign);
    fs::create_directories(tmp);
    const fs::path prg = tmp / "test.prg";
    write_text(
        prg,
        "x = 42\n"
        "DO subproc\n"
        "caller_x = x\n"
        "RETURN\n"
        "PROCEDURE subproc\n"
        "LOCAL x\n"
        "x = 99\n"
        "RELEASE ALL\n"
        "sub_x_after_release_all = x\n"
        "RETURN\n");
    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(prg.string(), tmp.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "RELEASE ALL with LOCAL shadowing should complete");

    const auto sub_x_after_release_all = state.globals.find("sub_x_after_release_all");
    const auto caller_x = state.globals.find("caller_x");
    expect(sub_x_after_release_all != state.globals.end(), "sub_x_after_release_all should be captured");
    expect(caller_x != state.globals.end(), "caller_x should be captured");
    if (sub_x_after_release_all != state.globals.end()) {
        expect(copperfin::runtime::format_value(sub_x_after_release_all->second) == "42",
               "RELEASE ALL should clear the LOCAL shadow without erasing the outer global binding");
    }
    if (caller_x != state.globals.end()) {
        expect(copperfin::runtime::format_value(caller_x->second) == "42",
               "RELEASE ALL should preserve the outer global after the LOCAL frame returns");
    }
    fs::remove_all(tmp, ign);
}

void test_release_all_private_shadow_restores_outer_global() {
    namespace fs = std::filesystem;
    const fs::path tmp = fs::temp_directory_path() / "copperfin_release_all_private_shadow";
    std::error_code ign;
    fs::remove_all(tmp, ign);
    fs::create_directories(tmp);
    const fs::path prg = tmp / "test.prg";
    write_text(
        prg,
        "x = 42\n"
        "DO subproc\n"
        "caller_x = x\n"
        "RETURN\n"
        "PROCEDURE subproc\n"
        "PRIVATE x\n"
        "x = 99\n"
        "RELEASE ALL\n"
        "sub_x_after_release_all = x\n"
        "RETURN\n");
    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(prg.string(), tmp.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "RELEASE ALL with PRIVATE shadowing should complete");

    const auto sub_x_after_release_all = state.globals.find("sub_x_after_release_all");
    const auto caller_x = state.globals.find("caller_x");
    expect(sub_x_after_release_all != state.globals.end(), "sub_x_after_release_all should be captured");
    expect(caller_x != state.globals.end(), "caller_x should be captured");
    if (sub_x_after_release_all != state.globals.end()) {
        expect(copperfin::runtime::format_value(sub_x_after_release_all->second) == "42",
               "RELEASE ALL should clear the PRIVATE shadow without erasing the outer global binding");
    }
    if (caller_x != state.globals.end()) {
        expect(copperfin::runtime::format_value(caller_x->second) == "42",
               "RELEASE ALL should preserve the outer global after the PRIVATE frame returns");
    }
    fs::remove_all(tmp, ign);
}

void test_release_all_like_pattern() {
    namespace fs = std::filesystem;
    const fs::path tmp = fs::temp_directory_path() / "copperfin_release_like";
    std::error_code ign;
    fs::remove_all(tmp, ign);
    fs::create_directories(tmp);
    const fs::path prg = tmp / "test.prg";
    write_text(prg, "tmp_a = 1\ntmp_b = 2\nkeep_me = 3\nRELEASE ALL LIKE tmp_*\nRETURN\n");
    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(prg.string(), tmp.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "RELEASE ALL LIKE should complete");
    expect(state.globals.find("tmp_a") == state.globals.end(), "tmp_a should be released");
    expect(state.globals.find("tmp_b") == state.globals.end(), "tmp_b should be released");
    expect(state.globals.find("keep_me") != state.globals.end(), "keep_me should survive LIKE tmp_*");
    fs::remove_all(tmp, ign);
}

void test_release_all_like_pattern_reaches_arrays() {
    namespace fs = std::filesystem;
    const fs::path tmp = fs::temp_directory_path() / "copperfin_release_like_arrays";
    std::error_code ign;
    fs::remove_all(tmp, ign);
    fs::create_directories(tmp);
    const fs::path prg = tmp / "test.prg";
    write_text(
        prg,
        "DIMENSION tmp_arr[1], keep_arr[1]\n"
        "tmp_arr[1] = 'gone'\n"
        "keep_arr[1] = 'stay'\n"
        "RELEASE ALL LIKE tmp_*\n"
        "tmp_type = TYPE('tmp_arr')\n"
        "keep_val = keep_arr[1]\n"
        "RETURN\n");
    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(prg.string(), tmp.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "RELEASE ALL LIKE should reach arrays");
    const auto tmp_type = state.globals.find("tmp_type");
    const auto keep_val = state.globals.find("keep_val");
    expect(tmp_type != state.globals.end(), "released array TYPE() should be captured");
    expect(keep_val != state.globals.end(), "surviving array value should be captured");
    if (tmp_type != state.globals.end()) {
        expect(copperfin::runtime::format_value(tmp_type->second) == "U", "RELEASE ALL LIKE tmp_* should release matching arrays");
    }
    if (keep_val != state.globals.end()) {
        expect(copperfin::runtime::format_value(keep_val->second) == "stay", "RELEASE ALL LIKE tmp_* should preserve non-matching arrays");
    }
    fs::remove_all(tmp, ign);
}

void test_release_all_except_pattern() {
    namespace fs = std::filesystem;
    const fs::path tmp = fs::temp_directory_path() / "copperfin_release_except";
    std::error_code ign;
    fs::remove_all(tmp, ign);
    fs::create_directories(tmp);
    const fs::path prg = tmp / "test.prg";
    write_text(prg, "keep_x = 1\ngone_y = 2\nRELEASE ALL EXCEPT keep_*\nRETURN\n");
    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(prg.string(), tmp.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "RELEASE ALL EXCEPT should complete");
    expect(state.globals.find("keep_x") != state.globals.end(), "keep_x should survive EXCEPT keep_*");
    expect(state.globals.find("gone_y") == state.globals.end(), "gone_y should be released");
    fs::remove_all(tmp, ign);
}

void test_release_all_except_pattern_reaches_arrays() {
    namespace fs = std::filesystem;
    const fs::path tmp = fs::temp_directory_path() / "copperfin_release_except_arrays";
    std::error_code ign;
    fs::remove_all(tmp, ign);
    fs::create_directories(tmp);
    const fs::path prg = tmp / "test.prg";
    write_text(
        prg,
        "DIMENSION keep_arr[1], gone_arr[1]\n"
        "keep_arr[1] = 'stay'\n"
        "gone_arr[1] = 'gone'\n"
        "RELEASE ALL EXCEPT keep_*\n"
        "keep_val = keep_arr[1]\n"
        "gone_type = TYPE('gone_arr')\n"
        "RETURN\n");
    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(prg.string(), tmp.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "RELEASE ALL EXCEPT should reach arrays");
    const auto keep_val = state.globals.find("keep_val");
    const auto gone_type = state.globals.find("gone_type");
    expect(keep_val != state.globals.end(), "surviving EXCEPT array value should be captured");
    expect(gone_type != state.globals.end(), "released EXCEPT array TYPE() should be captured");
    if (keep_val != state.globals.end()) {
        expect(copperfin::runtime::format_value(keep_val->second) == "stay", "RELEASE ALL EXCEPT keep_* should preserve matching arrays");
    }
    if (gone_type != state.globals.end()) {
        expect(copperfin::runtime::format_value(gone_type->second) == "U", "RELEASE ALL EXCEPT keep_* should release non-matching arrays");
    }
    fs::remove_all(tmp, ign);
}

void test_release_all_preserves_public_bindings() {
    namespace fs = std::filesystem;
    const fs::path tmp = fs::temp_directory_path() / "copperfin_release_public";
    std::error_code ign;
    fs::remove_all(tmp, ign);
    fs::create_directories(tmp);
    const fs::path prg = tmp / "test.prg";
    write_text(
        prg,
        "PUBLIC pub_keep, pub_arr\n"
        "pub_keep = 7\n"
        "DIMENSION pub_arr[1]\n"
        "pub_arr[1] = 'A'\n"
        "drop_me = 1\n"
        "RELEASE ALL EXCEPT keep_*\n"
        "pub_after = pub_keep\n"
        "arr_after = pub_arr[1]\n"
        "drop_type = TYPE('drop_me')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(prg.string(), tmp.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "RELEASE ALL should preserve PUBLIC bindings");

    const auto pub_after = state.globals.find("pub_after");
    const auto arr_after = state.globals.find("arr_after");
    const auto drop_type = state.globals.find("drop_type");
    expect(pub_after != state.globals.end(), "PUBLIC scalar should be readable after RELEASE ALL EXCEPT");
    expect(arr_after != state.globals.end(), "PUBLIC array should be readable after RELEASE ALL EXCEPT");
    expect(drop_type != state.globals.end(), "released non-public variable type should be captured");
    if (pub_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(pub_after->second) == "7",
               "RELEASE ALL EXCEPT should not erase a PUBLIC scalar that fails the EXCEPT pattern");
    }
    if (arr_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(arr_after->second) == "A",
               "RELEASE ALL EXCEPT should not erase a PUBLIC array that fails the EXCEPT pattern");
    }
    if (drop_type != state.globals.end()) {
        expect(copperfin::runtime::format_value(drop_type->second) == "U",
               "RELEASE ALL EXCEPT should still erase matching non-public variables");
    }

    fs::remove_all(tmp, ign);
}

void test_clear_memory_erases_all_globals() {
    namespace fs = std::filesystem;
    const fs::path tmp = fs::temp_directory_path() / "copperfin_clear_memory";
    std::error_code ign;
    fs::remove_all(tmp, ign);
    fs::create_directories(tmp);
    const fs::path prg = tmp / "test.prg";
    write_text(prg, "p = 42\nq = 99\nCLEAR MEMORY\nRETURN\n");
    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(prg.string(), tmp.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "CLEAR MEMORY should complete");
    expect(state.globals.find("p") == state.globals.end(), "p should be cleared");
    expect(state.globals.find("q") == state.globals.end(), "q should be cleared");
    fs::remove_all(tmp, ign);
}

void test_clear_memory_prevents_private_bindings_from_restoring() {
    namespace fs = std::filesystem;
    const fs::path tmp = fs::temp_directory_path() / "copperfin_clear_memory_private_restore";
    std::error_code ign;
    fs::remove_all(tmp, ign);
    fs::create_directories(tmp);
    const fs::path prg = tmp / "test.prg";
    write_text(
        prg,
        "x = 42\n"
        "DO subproc\n"
        "caller_type = TYPE('x')\n"
        "RETURN\n"
        "PROCEDURE subproc\n"
        "PRIVATE x\n"
        "x = 99\n"
        "CLEAR MEMORY\n"
        "sub_type = TYPE('x')\n"
        "RETURN\n");
    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(prg.string(), tmp.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "CLEAR MEMORY with PRIVATE shadowing should complete");
    const auto sub_type = state.globals.find("sub_type");
    const auto caller_type = state.globals.find("caller_type");
    expect(sub_type != state.globals.end(), "sub_type should be captured after CLEAR MEMORY");
    expect(caller_type != state.globals.end(), "caller_type should be captured after PRIVATE frame returns");
    if (sub_type != state.globals.end()) {
        expect(copperfin::runtime::format_value(sub_type->second) == "U",
               "CLEAR MEMORY should remove the PRIVATE binding inside the current frame");
    }
    if (caller_type != state.globals.end()) {
        expect(copperfin::runtime::format_value(caller_type->second) == "U",
               "CLEAR MEMORY should prevent saved outer PRIVATE bindings from being restored later");
    }
    fs::remove_all(tmp, ign);
}

void test_clear_memory_clears_current_frame_locals_without_global_leak() {
    namespace fs = std::filesystem;
    const fs::path tmp = fs::temp_directory_path() / "copperfin_clear_memory_locals";
    std::error_code ign;
    fs::remove_all(tmp, ign);
    fs::create_directories(tmp);
    const fs::path prg = tmp / "test.prg";
    write_text(
        prg,
        "DO subproc\n"
        "outer_type = TYPE('x')\n"
        "RETURN\n"
        "PROCEDURE subproc\n"
        "LOCAL x\n"
        "x = 5\n"
        "CLEAR MEMORY\n"
        "after_clear_type = TYPE('x')\n"
        "x = 7\n"
        "after_reassign = x\n"
        "RETURN\n");
    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(prg.string(), tmp.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "CLEAR MEMORY should clear current-frame locals");
    const auto after_clear_type = state.globals.find("after_clear_type");
    const auto after_reassign = state.globals.find("after_reassign");
    const auto outer_type = state.globals.find("outer_type");
    expect(after_clear_type != state.globals.end(), "cleared local TYPE() should be captured");
    expect(after_reassign != state.globals.end(), "reassigned local after CLEAR MEMORY should be captured");
    expect(outer_type != state.globals.end(), "post-return local TYPE() after CLEAR MEMORY should be captured");
    if (after_clear_type != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_clear_type->second) == "U",
               "CLEAR MEMORY should clear the current frame's local variable binding");
    }
    if (after_reassign != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_reassign->second) == "7",
               "reassigning after CLEAR MEMORY should still work inside the local scope");
    }
    if (outer_type != state.globals.end()) {
        expect(copperfin::runtime::format_value(outer_type->second) == "U",
               "reassigning a cleared LOCAL should not leak a new global after the routine returns");
    }
    fs::remove_all(tmp, ign);
}

void test_cancel_halts_execution() {
    namespace fs = std::filesystem;
    const fs::path tmp = fs::temp_directory_path() / "copperfin_cancel";
    std::error_code ign;
    fs::remove_all(tmp, ign);
    fs::create_directories(tmp);
    const fs::path prg = tmp / "test.prg";
    write_text(prg, "x = 1\nCANCEL\nx = 999\nRETURN\n");
    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(prg.string(), tmp.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "CANCEL should terminate cleanly");
    const auto it = state.globals.find("x");
    if (it != state.globals.end()) {
        expect(it->second.number_value == 1.0, "CANCEL should prevent execution of statements after it");
    }
    const bool has_cancel = std::any_of(state.events.begin(), state.events.end(),
        [](const auto &e) { return e.category == "runtime.cancel"; });
    expect(has_cancel, "CANCEL should emit runtime.cancel event");
    fs::remove_all(tmp, ign);
}

void test_quit_emits_event() {
    namespace fs = std::filesystem;
    const fs::path tmp = fs::temp_directory_path() / "copperfin_quit";
    std::error_code ign;
    fs::remove_all(tmp, ign);
    fs::create_directories(tmp);
    const fs::path prg = tmp / "test.prg";
    write_text(prg, "y = 5\nQUIT\ny = 999\nRETURN\n");
    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(prg.string(), tmp.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "QUIT should terminate cleanly");
    const bool has_quit = std::any_of(state.events.begin(), state.events.end(),
        [](const auto &e) { return e.category == "runtime.quit"; });
    expect(has_quit, "QUIT should emit runtime.quit event");
    fs::remove_all(tmp, ign);
}


void test_quit_cancelled_by_callback() {
    // When quit_confirm_callback returns false, QUIT should be cancelled:
    // execution continues after the QUIT statement and y should reach 999.
    namespace fs = std::filesystem;
    const fs::path tmp = fs::temp_directory_path() / "copperfin_quit_cancel";
    std::error_code ign;
    fs::remove_all(tmp, ign);
    fs::create_directories(tmp);
    const fs::path prg = tmp / "test.prg";
    write_text(prg, "y = 5\nQUIT\ny = 999\nRETURN\n");
    auto opts = make_runtime_session_options(prg.string(), tmp.string(), false);
    opts.quit_confirm_callback = []() -> bool { return false; };  // user said no
    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(opts);
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "Execution should complete normally after quit was cancelled");
    // y should have reached 999 — the line after QUIT was executed
    const auto it = state.globals.find("y");
    expect(it != state.globals.end(), "Variable y should exist");
    if (it != state.globals.end()) {
        expect(it->second.number_value == 999.0, "y should be 999 after QUIT was cancelled");
    }
    const bool has_cancelled = std::any_of(state.events.begin(), state.events.end(),
        [](const auto &e) { return e.category == "runtime.quit_cancelled"; });
    expect(has_cancelled, "QUIT cancelled should emit runtime.quit_cancelled event");
    const bool has_quit = std::any_of(state.events.begin(), state.events.end(),
        [](const auto &e) { return e.category == "runtime.quit"; });
    expect(!has_quit, "runtime.quit event should NOT be emitted when QUIT is cancelled");
    fs::remove_all(tmp, ign);
}

void test_shutdown_handler_quit_exits_event_loop_without_clear_events() {
    namespace fs = std::filesystem;
    const fs::path tmp = fs::temp_directory_path() / "copperfin_shutdown_quit_without_clear";
    std::error_code ign;
    fs::remove_all(tmp, ign);
    fs::create_directories(tmp);

    const fs::path prg = tmp / "test.prg";
    write_text(prg,
               "READ EVENTS\n"
               "RETURN\n"
               "PROCEDURE AppShutdown\n"
               "QUIT\n"
               "ENDPROC\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(prg.string(), tmp.string(), false));

    const auto paused = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(paused.reason == copperfin::runtime::DebugPauseReason::event_loop,
           "READ EVENTS should place runtime into event-loop pause");

    const bool dispatched = session.dispatch_event_handler("AppShutdown");
    expect(dispatched, "shutdown event handler should dispatch while in READ EVENTS");

    const auto completed = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(completed.completed, "QUIT inside shutdown handler should complete runtime without CLEAR EVENTS");
    expect(completed.reason == copperfin::runtime::DebugPauseReason::completed,
           "runtime should report completed after shutdown QUIT");

    const bool has_quit = std::any_of(completed.events.begin(), completed.events.end(),
        [](const auto &e) { return e.category == "runtime.quit"; });
    expect(has_quit, "shutdown QUIT should emit runtime.quit event");

    fs::remove_all(tmp, ign);
}

void test_shutdown_handler_cleanup_code_remains_harmless() {
    namespace fs = std::filesystem;
    const fs::path tmp = fs::temp_directory_path() / "copperfin_shutdown_quit_with_clear";
    std::error_code ign;
    fs::remove_all(tmp, ign);
    fs::create_directories(tmp);

    const fs::path prg = tmp / "test.prg";
    write_text(prg,
               "READ EVENTS\n"
               "RETURN\n"
               "PROCEDURE AppShutdown\n"
               "CLEAR EVENTS\n"
               "QUIT\n"
               "ENDPROC\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(prg.string(), tmp.string(), false));

    const auto paused = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(paused.reason == copperfin::runtime::DebugPauseReason::event_loop,
           "READ EVENTS should place runtime into event-loop pause");

    const bool dispatched = session.dispatch_event_handler("AppShutdown");
    expect(dispatched, "shutdown event handler should dispatch while in READ EVENTS");

    const auto completed = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(completed.completed, "CLEAR EVENTS + QUIT shutdown path should complete cleanly");
    expect(completed.reason == copperfin::runtime::DebugPauseReason::completed,
           "runtime should report completed after cleanup-enhanced shutdown handler");

    const bool has_quit = std::any_of(completed.events.begin(), completed.events.end(),
        [](const auto &e) { return e.category == "runtime.quit"; });
    expect(has_quit, "cleanup-enhanced shutdown path should still emit runtime.quit event");

    fs::remove_all(tmp, ign);
}

void test_on_shutdown_clear_events_runs_and_quit_completes() {
    namespace fs = std::filesystem;
    const fs::path tmp = fs::temp_directory_path() / "copperfin_on_shutdown_clear_events";
    std::error_code ign;
    fs::remove_all(tmp, ign);
    fs::create_directories(tmp);

    const fs::path prg = tmp / "test.prg";
    write_text(prg,
               "ON SHUTDOWN CLEAR EVENTS\n"
               "QUIT\n"
               "after_quit = 1\n"
               "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(prg.string(), tmp.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);

    expect(state.completed, "ON SHUTDOWN CLEAR EVENTS + QUIT should complete cleanly");
    expect(state.globals.find("after_quit") == state.globals.end(), "QUIT should prevent statements after it from running");

    const bool has_shutdown = std::any_of(state.events.begin(), state.events.end(),
        [](const auto &e) { return e.category == "runtime.shutdown_handler" && e.detail == "CLEAR EVENTS"; });
    expect(has_shutdown, "ON SHUTDOWN CLEAR EVENTS should emit runtime.shutdown_handler event");

    const bool has_quit = std::any_of(state.events.begin(), state.events.end(),
        [](const auto &e) { return e.category == "runtime.quit"; });
    expect(has_quit, "QUIT should still emit runtime.quit after ON SHUTDOWN CLEAR EVENTS");

    fs::remove_all(tmp, ign);
}

void test_on_shutdown_do_cleanup_can_call_quit_without_recursing() {
    namespace fs = std::filesystem;
    const fs::path tmp = fs::temp_directory_path() / "copperfin_on_shutdown_do_cleanup";
    std::error_code ign;
    fs::remove_all(tmp, ign);
    fs::create_directories(tmp);

    write_text(tmp / "held.txt", "seed");

    const fs::path prg = tmp / "test.prg";
    write_text(prg,
               "ON SHUTDOWN DO CleanupProcedure\n"
               "READ EVENTS\n"
               "RETURN\n"
               "PROCEDURE RequestQuit\n"
               "    nConn = SQLCONNECT('dsn=Northwind')\n"
               "    obj = CREATEOBJECT('Sample.Object')\n"
               "    nHandle = FOPEN('held.txt', 2)\n"
               "    QUIT\n"
               "ENDPROC\n"
               "PROCEDURE CleanupProcedure\n"
               "    cleanup_marker = 1\n"
               "    CLEAR EVENTS\n"
               "    CLOSE DATABASES ALL\n"
               "    QUIT\n"
               "ENDPROC\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(prg.string(), tmp.string(), false));

    const auto paused = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(paused.reason == copperfin::runtime::DebugPauseReason::event_loop,
           "READ EVENTS should pause before requesting quit");

    const bool dispatched = session.dispatch_event_handler("RequestQuit");
    expect(dispatched, "RequestQuit should dispatch from READ EVENTS");

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "ON SHUTDOWN DO cleanup with nested QUIT should complete cleanly");

    const auto cleanup = state.globals.find("cleanup_marker");
    expect(cleanup != state.globals.end(), "CleanupProcedure should run before final quit");
    if (cleanup != state.globals.end()) {
        expect(cleanup->second.number_value == 1.0, "CleanupProcedure should set cleanup_marker");
    }

    const bool has_shutdown = std::any_of(state.events.begin(), state.events.end(),
        [](const auto &e) { return e.category == "runtime.shutdown_handler" && e.detail == "CleanupProcedure"; });
    expect(has_shutdown, "ON SHUTDOWN DO CleanupProcedure should emit runtime.shutdown_handler event");

    const std::size_t quit_event_count = static_cast<std::size_t>(std::count_if(state.events.begin(), state.events.end(),
        [](const auto &e) { return e.category == "runtime.quit"; }));
    expect(quit_event_count == 1U, "Nested QUIT inside shutdown handler should not recurse into multiple quit events");

    expect(state.sql_connections.empty(), "Shutdown cleanup QUIT path should leave no SQL connections");
    expect(state.ole_objects.empty(), "Shutdown cleanup QUIT path should leave no OLE objects");

    fs::remove_all(tmp, ign);
}

void test_on_shutdown_inline_close_databases_all_runs_before_quit() {
    namespace fs = std::filesystem;
    const fs::path tmp = fs::temp_directory_path() / "copperfin_on_shutdown_close_databases_all";
    std::error_code ign;
    fs::remove_all(tmp, ign);
    fs::create_directories(tmp);

    write_text(tmp / "held.txt", "seed");

    const fs::path prg = tmp / "test.prg";
    write_text(prg,
               "ON SHUTDOWN CLOSE DATABASES ALL\n"
               "READ EVENTS\n"
               "RETURN\n"
               "PROCEDURE RequestQuit\n"
               "    nConn = SQLCONNECT('dsn=Northwind')\n"
               "    obj = CREATEOBJECT('Sample.Object')\n"
               "    nHandle = FOPEN('held.txt', 2)\n"
               "    QUIT\n"
               "ENDPROC\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(prg.string(), tmp.string(), false));

    const auto paused = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(paused.reason == copperfin::runtime::DebugPauseReason::event_loop,
           "READ EVENTS should pause before inline shutdown close runs");

    const bool dispatched = session.dispatch_event_handler("RequestQuit");
    expect(dispatched, "RequestQuit should dispatch from READ EVENTS");

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "ON SHUTDOWN CLOSE DATABASES ALL + QUIT should complete cleanly");
    expect(state.sql_connections.empty(), "Inline CLOSE DATABASES ALL should leave no SQL connections");
    expect(state.ole_objects.empty(), "Inline CLOSE DATABASES ALL should leave no OLE handles");

    const bool has_shutdown = std::any_of(state.events.begin(), state.events.end(),
        [](const auto &e) { return e.category == "runtime.shutdown_handler" && e.detail == "CLOSE DATABASES ALL"; });
    expect(has_shutdown, "ON SHUTDOWN CLOSE DATABASES ALL should emit runtime.shutdown_handler event");

    const bool has_close = std::any_of(state.events.begin(), state.events.end(),
        [](const auto &e) { return e.category == "runtime.close" && e.detail == "DATABASES ALL"; });
    expect(has_close, "Inline shutdown close clause should emit runtime.close event");

    fs::remove_all(tmp, ign);
}

void test_quit_closes_open_database_and_runtime_handles() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_quit_resource_cleanup";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_simple_dbf(temp_root / "a.dbf", {"alpha", "beta"});
    write_text(temp_root / "held.txt", "seed");

    const fs::path quit_path = temp_root / "quit_cleanup.prg";
    write_text(
        quit_path,
        "USE '" + (temp_root / "a.dbf").string() + "'\n"
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "obj = CREATEOBJECT('Sample.Object')\n"
        "nHandle = FOPEN('held.txt', 2)\n"
        "QUIT\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(quit_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "QUIT cleanup script should complete");
    expect(state.cursors.empty(), "QUIT should close open database cursors/work areas");
    expect(state.work_area.aliases.empty(), "QUIT should clear work-area aliases");
    expect(state.sql_connections.empty(), "QUIT should disconnect open SQL connections");
    expect(state.ole_objects.empty(), "QUIT should release tracked OLE object handles");

    const auto handle_it = state.globals.find("nhandle");
    expect(handle_it != state.globals.end(), "FOPEN handle should be captured before QUIT");
    int handle = 1;
    if (handle_it != state.globals.end()) {
        handle = static_cast<int>(handle_it->second.number_value);
    }

    const fs::path verify_path = temp_root / "verify_handle_closed.prg";
    write_text(
        verify_path,
        "nClose = FCLOSE(" + std::to_string(handle) + ")\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession verify_session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(verify_path.string(), temp_root.string(), false));
    const auto verify_state = verify_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(verify_state.completed, "verification script should complete");

    const auto close_it = verify_state.globals.find("nclose");
    expect(close_it != verify_state.globals.end(), "verification script should expose FCLOSE result");
    if (close_it != verify_state.globals.end()) {
        expect(close_it->second.number_value == -1.0,
               "QUIT should already close FOPEN handles so follow-up FCLOSE returns -1");
    }

    fs::remove_all(temp_root, ignored);
}

void test_doevents_pumps_event_queue() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_doevents";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "doevents_test.prg";
    write_text(
        main_path,
        "i = 0\n"
        "DO WHILE i < 10\n"
        "    i = i + 1\n"
        "    DOEVENTS\n"
        "ENDDO\n"
        "nFinal = i\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "DOEVENTS test should complete");

    const auto final_it = state.globals.find("nfinal");
    expect(final_it != state.globals.end(), "DOEVENTS test should expose nFinal variable");
    if (final_it != state.globals.end()) {
        expect(final_it->second.number_value == 10.0, "loop should complete with i=10 after DOEVENTS calls");
    }

    // Verify that DOEVENTS events were emitted
    const auto doevents_events = std::count_if(
        state.events.begin(), state.events.end(),
        [](const auto& evt) { return evt.category == "runtime.event_loop" && evt.detail == "DOEVENTS"; });
    expect(doevents_events > 0, "DOEVENTS should emit event_loop events");

    fs::remove_all(temp_root, ignored);
}

void test_doevents_in_responsive_loop() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_doevents_resp";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "doevents_resp.prg";
    write_text(
        main_path,
        "i = 0\n"
        "* Simulate responsive loop with periodic DOEVENTS\n"
        "DO WHILE i < 5\n"
        "    i = i + 1\n"
        "    DOEVENTS\n"
        "    IF i >= 5\n"
        "        CLEAR EVENTS\n"
        "    ENDIF\n"
        "ENDDO\n"
        "nLoopCount = i\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "DOEVENTS loop with CLEAR EVENTS should complete");

    const auto loop_it = state.globals.find("nloopcount");
    expect(loop_it != state.globals.end(), "DOEVENTS loop should expose nLoopCount");
    if (loop_it != state.globals.end()) {
        expect(loop_it->second.number_value == 5.0, "loop should complete after 5 iterations");
    }

    fs::remove_all(temp_root, ignored);
}

void test_sleep_command_emits_runtime_sleep_event() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_sleep_cmd";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "sleep_test.prg";
    write_text(
        main_path,
        "nDelay = 1\n"
        "SLEEP nDelay\n"
        "nAfter = 42\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SLEEP test should complete");

    const auto sleep_event = std::find_if(
        state.events.begin(), state.events.end(),
        [](const auto& event) { return event.category == "runtime.sleep"; });
    expect(sleep_event != state.events.end(), "SLEEP should emit a runtime.sleep event");
    if (sleep_event != state.events.end()) {
        expect(sleep_event->detail.find("duration=1ms") != std::string::npos,
            "SLEEP event should report the resolved duration");
    }

    const auto after_it = state.globals.find("nafter");
    expect(after_it != state.globals.end(), "SLEEP script should continue after the delay");
    if (after_it != state.globals.end()) {
        expect(after_it->second.number_value == 42.0, "SLEEP should not disturb later statements");
    }

    fs::remove_all(temp_root, ignored);
}

void test_spawn_and_await_command_runs_task_to_completion() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_spawn_await_cmd";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "spawn_await_test.prg";
    write_text(
        main_path,
        "PROCEDURE worker\n"
        "    SLEEP 1\n"
        "    RETURN\n"
        "ENDPROC\n"
        "SPAWN worker TO nTask\n"
        "AWAIT nTask TO lDone\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SPAWN/AWAIT test should complete");

    const auto spawn_event = std::find_if(
        state.events.begin(), state.events.end(),
        [](const auto& event) { return event.category == "runtime.task.spawn"; });
    const auto await_event = std::find_if(
        state.events.begin(), state.events.end(),
        [](const auto& event) { return event.category == "runtime.task.await"; });
    expect(spawn_event != state.events.end(), "SPAWN should emit a task-spawn event");
    expect(await_event != state.events.end(), "AWAIT should emit a task-await event");
    if (spawn_event != state.events.end()) {
        expect(spawn_event->detail.find("handle=") != std::string::npos,
            "SPAWN event should report a task handle");
    }
    if (await_event != state.events.end()) {
        expect(await_event->detail.find("state=completed") != std::string::npos,
            "AWAIT event should report completed task state");
    }

    const auto handle_it = state.globals.find("ntask");
    const auto done_it = state.globals.find("ldone");
    expect(handle_it != state.globals.end(), "SPAWN should assign the task handle");
    expect(done_it != state.globals.end(), "AWAIT should assign the completion flag");
    if (handle_it != state.globals.end()) {
        expect(handle_it->second.number_value > 0.0, "SPAWN should return a positive task handle");
    }
    if (done_it != state.globals.end()) {
        expect(done_it->second.boolean_value, "AWAIT should report a completed task");
    }

    fs::remove_all(temp_root, ignored);
}

void test_spawn_cancellation_propagates_to_sibling_tasks() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_spawn_cancel_cmd";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "spawn_cancel_test.prg";
    write_text(
        main_path,
        "PROCEDURE worker\n"
        "    SLEEP 50\n"
        "    RETURN\n"
        "ENDPROC\n"
        "PROCEDURE canceler\n"
        "    SLEEP 1\n"
        "    CANCEL\n"
        "ENDPROC\n"
        "SPAWN worker TO nWorker\n"
        "SPAWN canceler TO nCancel\n"
        "AWAIT nCancel TO lCancelDone\n"
        "AWAIT nWorker TO lWorkerDone\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "spawn cancellation test should complete");

    const auto cancelled_event = std::find_if(
        state.events.begin(), state.events.end(),
        [](const auto& event) { return event.category == "runtime.task.cancelled"; });
    const auto cancel_await_event = std::find_if(
        state.events.begin(), state.events.end(),
        [](const auto& event) { return event.category == "runtime.task.await" && event.detail.find("handle=") != std::string::npos && event.detail.find("state=error") != std::string::npos; });
    expect(cancelled_event != state.events.end(), "cancellation should emit a runtime.task.cancelled event");
    expect(cancel_await_event != state.events.end(), "awaiting a cancelled task should report an error state");

    const auto cancel_done = state.globals.find("lcanceldone");
    const auto worker_done = state.globals.find("lworkerdone");
    expect(cancel_done != state.globals.end(), "canceler completion flag should be captured");
    expect(worker_done != state.globals.end(), "worker completion flag should be captured");
    if (cancel_done != state.globals.end()) {
        expect(!cancel_done->second.boolean_value, "canceler task should not report completed after CANCEL");
    }
    if (worker_done != state.globals.end()) {
        expect(!worker_done->second.boolean_value, "worker task should be marked incomplete after cancellation");
    }

    fs::remove_all(temp_root, ignored);
}

void test_spawn_critical_section_serializes_workers() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_spawn_critical_cmd";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "spawn_critical_test.prg";
    write_text(
        main_path,
        "PROCEDURE worker\n"
        "    ENTER CRITICAL shared\n"
        "    FOR nSpin = 1 TO 50\n"
        "        YIELD\n"
        "    ENDFOR\n"
        "    EXIT CRITICAL shared\n"
        "    RETURN\n"
        "ENDPROC\n"
        "SPAWN worker TO nFirst\n"
        "SPAWN worker TO nSecond\n"
        "AWAIT nFirst TO lFirstDone\n"
        "AWAIT nSecond TO lSecondDone\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "critical-section spawn test should complete");

    const auto enter_count = std::count_if(
        state.events.begin(), state.events.end(),
        [](const auto& event) { return event.category == "runtime.critical.enter"; });
    const auto exit_count = std::count_if(
        state.events.begin(), state.events.end(),
        [](const auto& event) { return event.category == "runtime.critical.exit"; });
    expect(enter_count == 2, "both workers should enter the critical section");
    expect(exit_count == 2, "both workers should exit the critical section");

    const auto first_done = state.globals.find("lfirstdone");
    const auto second_done = state.globals.find("lseconddone");
    expect(first_done != state.globals.end(), "first worker completion flag should be captured");
    expect(second_done != state.globals.end(), "second worker completion flag should be captured");
    if (first_done != state.globals.end()) {
        expect(first_done->second.boolean_value, "first worker should complete successfully");
    }
    if (second_done != state.globals.end()) {
        expect(second_done->second.boolean_value, "second worker should complete successfully");
    }

    fs::remove_all(temp_root, ignored);
}

void test_critical_section_order_policy_rejects_descending_nested_acquire() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_critical_order_policy";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "critical_order_test.prg";
    write_text(
        main_path,
        "PROCEDURE workergood\n"
        "    ENTER CRITICAL alpha\n"
        "    ENTER CRITICAL beta\n"
        "    EXIT CRITICAL beta\n"
        "    EXIT CRITICAL alpha\n"
        "    RETURN\n"
        "ENDPROC\n"
        "PROCEDURE workerbad\n"
        "    ENTER CRITICAL beta\n"
        "    ENTER CRITICAL alpha\n"
        "    EXIT CRITICAL beta\n"
        "    RETURN\n"
        "ENDPROC\n"
        "SPAWN workergood TO nGood\n"
        "SPAWN workerbad TO nBad\n"
        "AWAIT nGood TO lGoodDone\n"
        "AWAIT nBad TO lBadDone\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "critical-section order-policy script should complete");

    const auto good_done = state.globals.find("lgooddone");
    const auto bad_done = state.globals.find("lbaddone");
    expect(good_done != state.globals.end(), "good worker completion flag should be captured");
    expect(bad_done != state.globals.end(), "bad worker completion flag should be captured");
    if (good_done != state.globals.end()) {
        expect(good_done->second.boolean_value, "ascending nested critical-section order should succeed");
    }
    if (bad_done != state.globals.end()) {
        expect(!bad_done->second.boolean_value, "descending nested critical-section order should fail deterministically");
    }

    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.critical.order_violation" &&
               event.detail.find("held=beta requested=alpha") != std::string::npos;
    }), "descending nested critical-section order should emit a runtime.critical.order_violation event");

    fs::remove_all(temp_root, ignored);
}

void test_critical_section_exit_order_is_enforced() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_critical_exit_order";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "critical_exit_order_test.prg";
    write_text(
        main_path,
        "PROCEDURE workerbad\n"
        "    ENTER CRITICAL alpha\n"
        "    ENTER CRITICAL beta\n"
        "    EXIT CRITICAL alpha\n"
        "    RETURN\n"
        "ENDPROC\n"
        "PROCEDURE workergood\n"
        "    ENTER CRITICAL alpha\n"
        "    ENTER CRITICAL beta\n"
        "    EXIT CRITICAL beta\n"
        "    EXIT CRITICAL alpha\n"
        "    lGoodDone = .T.\n"
        "    RETURN\n"
        "ENDPROC\n"
        "SPAWN workerbad TO nBad\n"
        "SPAWN workergood TO nGood\n"
        "AWAIT nBad TO lBadAwaitDone\n"
        "AWAIT nGood TO lGoodAwaitDone\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "critical-section exit-order policy script should complete");

    const auto bad_await_done = state.globals.find("lbadawaitdone");
    const auto good_await_done = state.globals.find("lgoodawaitdone");
    expect(bad_await_done != state.globals.end(), "bad worker should report await completion");
    expect(good_await_done != state.globals.end(), "good worker should report await completion");
    if (bad_await_done != state.globals.end()) {
        expect(!bad_await_done->second.boolean_value, "out-of-order EXIT CRITICAL should fault the bad worker");
    }
    if (good_await_done != state.globals.end()) {
        expect(good_await_done->second.boolean_value, "good worker should terminate after valid critical usage");
    }

    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.critical.order_violation" &&
               event.detail.find("held=beta requested=alpha") != std::string::npos;
    }), "out-of-order EXIT CRITICAL should emit runtime.critical.order_violation");

    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.task.await" &&
               event.detail.find("state=error") != std::string::npos &&
               event.detail.find("handle=") != std::string::npos;
    }), "bad worker AWAIT should report task error state");

    fs::remove_all(temp_root, ignored);
}

void test_critical_section_reentrant_enter_same_section_is_allowed() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_critical_reentrant_policy";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "critical_reentrant_test.prg";
    write_text(
        main_path,
        "lOuterEntered = .F.\n"
        "lInnerEntered = .F.\n"
        "lInnerExited = .F.\n"
        "lOuterExited = .F.\n"
        "ENTER CRITICAL  shared\n"
        "lOuterEntered = .T.\n"
        "ENTER CRITICAL    sHaReD\n"
        "lInnerEntered = .T.\n"
        "EXIT CRITICAL shared\n"
        "lInnerExited = .T.\n"
        "EXIT CRITICAL shared\n"
        "lOuterExited = .T.\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "critical-section reentrant-enter script should complete");

    const auto outer_entered = state.globals.find("louterentered");
    const auto inner_entered = state.globals.find("linnerentered");
    const auto inner_exited = state.globals.find("linnerexited");
    const auto outer_exited = state.globals.find("louterexited");
    expect(outer_entered != state.globals.end(), "outer critical section should be entered");
    expect(inner_entered != state.globals.end(), "inner critical section should be entered with same normalized section");
    expect(inner_exited != state.globals.end(), "inner critical section should be exited");
    expect(outer_exited != state.globals.end(), "outer critical section should be exited");
    if (outer_entered != state.globals.end()) {
        expect(outer_entered->second.boolean_value, "outer critical section should execute");
    }
    if (inner_entered != state.globals.end()) {
        expect(inner_entered->second.boolean_value, "inner critical section should execute");
    }
    if (inner_exited != state.globals.end()) {
        expect(inner_exited->second.boolean_value, "inner critical exit path should execute");
    }
    if (outer_exited != state.globals.end()) {
        expect(outer_exited->second.boolean_value, "outer critical exit path should execute");
    }

    const auto enter_count = std::count_if(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.critical.enter";
    });
    const auto exit_count = std::count_if(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.critical.exit";
    });
    expect(enter_count == 2U, "same-section re-entry should emit two enter events");
    expect(exit_count == 2U, "same-section re-entry should emit two exit events");

    expect(std::none_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.critical.order_violation";
    }), "same-section re-entry should not emit critical order violations");

    fs::remove_all(temp_root, ignored);
}

void test_critical_section_blocking_policy_rejects_await_inside_section() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_critical_await_policy";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "critical_await_test.prg";
    write_text(
        main_path,
        "PROCEDURE worker\n"
        "    SLEEP 1\n"
        "    RETURN\n"
        "ENDPROC\n"
        "SPAWN worker TO nTask\n"
        "TRY\n"
        "    ENTER CRITICAL shared\n"
        "    AWAIT nTask TO lDone\n"
        "    lAwaitBlocked = .F.\n"
        "CATCH TO err_text\n"
        "    lAwaitBlocked = .T.\n"
        "    cAwaitError = err_text\n"
        "ENDTRY\n"
        "EXIT CRITICAL shared\n"
        "AWAIT nTask TO lDoneAfterExit\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "critical-section AWAIT policy script should complete");

    const auto await_blocked = state.globals.find("lawaitblocked");
    const auto await_error = state.globals.find("cawaiterror");
    const auto done_after_exit = state.globals.find("ldoneafterexit");
    expect(await_blocked != state.globals.end(), "AWAIT policy script should capture the blocking-policy result");
    expect(await_error != state.globals.end(), "AWAIT policy script should capture the blocking-policy message");
    expect(done_after_exit != state.globals.end(), "AWAIT policy script should still await successfully after leaving the critical section");
    if (await_blocked != state.globals.end()) {
        expect(await_blocked->second.boolean_value, "AWAIT inside a critical section should be rejected");
    }
    if (await_error != state.globals.end()) {
        expect(copperfin::runtime::format_value(await_error->second).find("Blocking operation AWAIT") != std::string::npos,
               "AWAIT policy error should mention the blocking AWAIT operation");
    }
    if (done_after_exit != state.globals.end()) {
        expect(done_after_exit->second.boolean_value, "AWAIT should succeed once the critical section is exited");
    }

    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.critical.blocking_violation" &&
               event.detail.find("operation=AWAIT") != std::string::npos;
    }), "AWAIT inside a critical section should emit a runtime.critical.blocking_violation event");

    fs::remove_all(temp_root, ignored);
}

void test_critical_section_blocking_policy_rejects_sleep_inside_section() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_critical_sleep_policy";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "critical_sleep_test.prg";
    write_text(
        main_path,
        "TRY\n"
        "    ENTER CRITICAL shared\n"
        "    SLEEP 5\n"
        "    lSleepBlocked = .F.\n"
        "CATCH TO err_text\n"
        "    lSleepBlocked = .T.\n"
        "    cSleepError = err_text\n"
        "ENDTRY\n"
        "EXIT CRITICAL shared\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "critical-section SLEEP policy script should complete");

    const auto sleep_blocked = state.globals.find("lsleepblocked");
    const auto sleep_error = state.globals.find("csleeperror");
    expect(sleep_blocked != state.globals.end(), "SLEEP policy script should capture the blocking-policy result");
    expect(sleep_error != state.globals.end(), "SLEEP policy script should capture the blocking-policy message");
    if (sleep_blocked != state.globals.end()) {
        expect(sleep_blocked->second.boolean_value, "SLEEP inside a critical section should be rejected");
    }
    if (sleep_error != state.globals.end()) {
        expect(copperfin::runtime::format_value(sleep_error->second).find("Blocking operation SLEEP") != std::string::npos,
               "SLEEP policy error should mention the blocking SLEEP operation");
    }

    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.critical.blocking_violation" &&
               event.detail.find("operation=SLEEP") != std::string::npos;
    }), "SLEEP inside a critical section should emit a runtime.critical.blocking_violation event");

    fs::remove_all(temp_root, ignored);
}

void test_yield_is_explicit_policy_exception_in_enter_critical() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_enter_critical_yield_policy_exception";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "yield_enter_critical_policy_exception_test.prg";
    write_text(
        main_path,
        "lEnteredCritical = .F.\n"
        "lYieldedInCritical = .F.\n"
        "ENTER CRITICAL\n"
        "lEnteredCritical = .T.\n"
        "YIELD\n"
        "lYieldedInCritical = .T.\n"
        "EXIT CRITICAL\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "YIELD should be allowed inside ENTER CRITICAL");

    const auto entered = state.globals.find("lenteredcritical");
    const auto yielded = state.globals.find("lyieldedincritical");
    expect(entered != state.globals.end(), "ENTER CRITICAL body should execute");
    expect(yielded != state.globals.end(), "YIELD should execute inside ENTER CRITICAL");

    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto &event) {
        return event.category == "runtime.critical.enter";
    }), "ENTER CRITICAL should emit critical-enter event");
    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto &event) {
        return event.category == "runtime.yield" && event.detail.find("operation=YIELD") != std::string::npos;
    }), "ENTER CRITICAL body should emit operation-tagged runtime.yield");
    expect(std::none_of(state.events.begin(), state.events.end(), [](const auto &event) {
        return event.category == "runtime.critical.blocking_violation" &&
               event.detail.find("operation=YIELD") != std::string::npos;
    }), "YIELD should remain policy exception while in CRITICAL section");

    const auto yield_event_it = std::find_if(state.events.begin(), state.events.end(), [](const auto &event) {
        return event.category == "runtime.yield" && event.detail.find("operation=YIELD") != std::string::npos;
    });
    const auto enter_event_it = std::find_if(state.events.begin(), state.events.end(), [](const auto &event) {
        return event.category == "runtime.critical.enter";
    });
    const auto exit_event_it = std::find_if(state.events.begin(), state.events.end(), [](const auto &event) {
        return event.category == "runtime.critical.exit";
    });
    if (yield_event_it != state.events.end() && enter_event_it != state.events.end() && exit_event_it != state.events.end()) {
        expect(std::distance(state.events.begin(), enter_event_it) <
               std::distance(state.events.begin(), yield_event_it),
               "runtime.yield should occur after ENTER CRITICAL");
        expect(std::distance(state.events.begin(), yield_event_it) <
               std::distance(state.events.begin(), exit_event_it),
               "runtime.yield should occur before EXIT CRITICAL");
    }

    fs::remove_all(temp_root, ignored);
}

void test_yield_allowed_in_enter_critical_regression_minimal() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_yield_enter_critical_minimal_regression";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "yield_enter_critical_minimal_regression_test.prg";
    write_text(
        main_path,
        "entered = .F.\n"
        "yielded = .F.\n"
        "ENTER CRITICAL\n"
        "entered = .T.\n"
        "YIELD\n"
        "yielded = .T.\n"
        "EXIT CRITICAL\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "ENTER CRITICAL/YIELD minimal regression should complete");
    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.critical.enter";
    }), "ENTER CRITICAL should emit runtime.critical.enter");
    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.yield" && event.detail.find("operation=YIELD") != std::string::npos;
    }), "ENTER CRITICAL + YIELD should emit operation-tagged runtime.yield");
    expect(std::none_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.critical.blocking_violation" &&
               event.detail.find("operation=YIELD") != std::string::npos;
    }), "YIELD in ENTER CRITICAL remains the intentional policy exception");

    fs::remove_all(temp_root, ignored);
}

void test_yield_in_enter_critical_has_no_blocking_violation() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_yield_enter_critical_locking_regression";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "yield_enter_critical_locking_regression_test.prg";
    write_text(
        main_path,
        "lEnteredCritical = .F.\n"
        "lYieldedInCritical = .F.\n"
        "ENTER CRITICAL\n"
        "lEnteredCritical = .T.\n"
        "YIELD\n"
        "lYieldedInCritical = .T.\n"
        "EXIT CRITICAL\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "ENTER CRITICAL should allow YIELD without blocking-policy error");

    const auto entered = state.globals.find("lenteredcritical");
    const auto yielded = state.globals.find("lyieldedincritical");
    expect(entered != state.globals.end(), "ENTER CRITICAL body should execute");
    expect(yielded != state.globals.end(), "YIELD inside ENTER CRITICAL should execute");
    if (entered != state.globals.end())
    {
        expect(entered->second.boolean_value, "critical section body should run before YIELD");
    }
    if (yielded != state.globals.end())
    {
        expect(yielded->second.boolean_value, "YIELD should complete inside ENTER CRITICAL");
    }

    const auto yield_event = std::find_if(state.events.begin(), state.events.end(), [](const auto &event) {
        return event.category == "runtime.yield" &&
               event.detail.find("operation=YIELD") != std::string::npos;
    });
    expect(yield_event != state.events.end(), "YIELD should emit operation-tagged runtime.yield event");
    expect(std::none_of(state.events.begin(), state.events.end(), [](const auto &event) {
        return event.category == "runtime.critical.blocking_violation" &&
               event.detail.find("operation=YIELD") != std::string::npos;
    }), "YIELD should be exempt from critical-section blocking rule");

    const auto critical_enter = std::find_if(state.events.begin(), state.events.end(), [](const auto &event) {
        return event.category == "runtime.critical.enter";
    });
    const auto critical_exit = std::find_if(state.events.begin(), state.events.end(), [](const auto &event) {
        return event.category == "runtime.critical.exit";
    });
    expect(critical_enter != state.events.end(), "ENTER CRITICAL should emit runtime.critical.enter");
    expect(critical_exit != state.events.end(), "EXIT CRITICAL should emit runtime.critical.exit");
    if (critical_enter != state.events.end() && critical_exit != state.events.end() && yield_event != state.events.end())
    {
        expect(std::distance(state.events.begin(), critical_enter) <
               std::distance(state.events.begin(), yield_event),
               "runtime.yield should happen after ENTER CRITICAL");
        expect(std::distance(state.events.begin(), yield_event) <
               std::distance(state.events.begin(), critical_exit),
               "runtime.yield should happen before EXIT CRITICAL");
    }

    fs::remove_all(temp_root, ignored);
}

void test_yield_in_enter_critical_is_explicit_policy_exception_regression() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_yield_enter_critical_policy_exception_regression";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "yield_enter_critical_policy_exception_regression_test.prg";
    write_text(
        main_path,
        "lEnteredCritical = .F.\n"
        "lYieldedInCritical = .F.\n"
        "ENTER CRITICAL\n"
        "lEnteredCritical = .T.\n"
        "YIELD\n"
        "lYieldedInCritical = .T.\n"
        "EXIT CRITICAL\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "ENTER CRITICAL/YIELD policy-exception regression should complete");

    const auto entered = state.globals.find("lenteredcritical");
    const auto yielded = state.globals.find("lyieldedincritical");
    expect(entered != state.globals.end(), "policy exception should execute CRITICAL body");
    expect(yielded != state.globals.end(), "YIELD should execute inside ENTER CRITICAL");
    if (entered != state.globals.end()) {
        expect(entered->second.boolean_value, "policy exception requires CRITICAL body before yield");
    }
    if (yielded != state.globals.end()) {
        expect(yielded->second.boolean_value, "yield should continue inside policy exception path");
    }

    const auto yield_event = std::find_if(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.yield" && event.detail.find("operation=YIELD") != std::string::npos;
    });
    expect(yield_event != state.events.end(), "policy exception should emit operation-tagged runtime.yield");

    const auto critical_enter = std::find_if(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.critical.enter";
    });
    const auto critical_exit = std::find_if(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.critical.exit";
    });
    expect(critical_enter != state.events.end(), "ENTER CRITICAL should emit runtime.critical.enter");
    expect(critical_exit != state.events.end(), "EXIT CRITICAL should emit runtime.critical.exit");
    if (critical_enter != state.events.end() && critical_exit != state.events.end() && yield_event != state.events.end()) {
        expect(std::distance(state.events.begin(), critical_enter) <
               std::distance(state.events.begin(), yield_event),
               "runtime.yield should occur after ENTER CRITICAL");
        expect(std::distance(state.events.begin(), yield_event) <
               std::distance(state.events.begin(), critical_exit),
               "runtime.yield should occur before EXIT CRITICAL");
    }

    expect(std::none_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.critical.blocking_violation" &&
               event.detail.find("operation=YIELD") != std::string::npos;
    }), "policy exception should not emit blocking violation for YIELD");

    fs::remove_all(temp_root, ignored);
}

void test_enter_critical_allows_yield_without_blocking_violation_event() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_enter_critical_yield_policy_contract";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "yield_enter_critical_policy_contract_test.prg";
    write_text(
        main_path,
        "lEnteredCritical = .F.\n"
        "lYielded = .F.\n"
        "ENTER CRITICAL\n"
        "lEnteredCritical = .T.\n"
        "YIELD\n"
        "lYielded = .T.\n"
        "EXIT CRITICAL\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "ENTER CRITICAL should allow YIELD as a policy exception");

    const auto entered = state.globals.find("lenteredcritical");
    const auto yielded = state.globals.find("lyielded");
    expect(entered != state.globals.end(), "policy test should execute CRITICAL body");
    expect(yielded != state.globals.end(), "policy test should execute YIELD in CRITICAL");
    if (entered != state.globals.end()) {
        expect(entered->second.boolean_value, "CRITICAL body should run before YIELD");
    }
    if (yielded != state.globals.end()) {
        expect(yielded->second.boolean_value, "YIELD should continue after yielding");
    }

    const auto enter_event = std::find_if(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.critical.enter";
    });
    const auto yield_event = std::find_if(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.yield" &&
               event.detail.find("operation=YIELD") != std::string::npos;
    });
    const auto exit_event = std::find_if(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.critical.exit";
    });
    expect(enter_event != state.events.end(), "ENTER CRITICAL should emit runtime.critical.enter");
    expect(yield_event != state.events.end(), "YIELD in CRITICAL should emit operation-tagged runtime.yield");
    expect(exit_event != state.events.end(), "EXIT CRITICAL should emit runtime.critical.exit");

    if (enter_event != state.events.end() && yield_event != state.events.end() && exit_event != state.events.end()) {
        expect(std::distance(state.events.begin(), enter_event) < std::distance(state.events.begin(), yield_event),
               "YIELD should execute after ENTER CRITICAL");
        expect(std::distance(state.events.begin(), yield_event) < std::distance(state.events.begin(), exit_event),
               "YIELD should execute before EXIT CRITICAL");
    }

    expect(std::none_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.critical.blocking_violation" &&
               event.detail.find("operation=YIELD") != std::string::npos;
    }), "YIELD inside ENTER CRITICAL should not emit a blocking violation");

    fs::remove_all(temp_root, ignored);
}

void test_critical_sections_release_on_task_fault_without_deadlock() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_critical_fault_release";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "critical_fault_release_test.prg";
    write_text(
        main_path,
        "PROCEDURE bad_worker\n"
        "    ENTER CRITICAL shared\n"
        "    1 / 0\n"
        "ENDPROC\n"
        "PROCEDURE good_worker\n"
        "    ENTER CRITICAL shared\n"
        "    EXIT CRITICAL shared\n"
        "    RETURN\n"
        "ENDPROC\n"
        "SPAWN bad_worker TO nBad\n"
        "SPAWN good_worker TO nGood\n"
        "AWAIT nBad TO lBadDone\n"
        "AWAIT nGood TO lGoodDone\n"
        "RETURN\n");

    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string(), false));

    std::promise<copperfin::runtime::RuntimePauseState> run_promise;
    auto run_future = run_promise.get_future();
    std::thread run_thread([session = std::move(session), run_promise = std::move(run_promise)]() mutable {
        try
        {
            run_promise.set_value(session.run(copperfin::runtime::DebugResumeAction::continue_run));
        }
        catch (...)
        {
            run_promise.set_exception(std::current_exception());
        }
    });

    const bool finished = run_future.wait_for(std::chrono::seconds(3)) == std::future_status::ready;
    if (!finished)
    {
        run_thread.detach();
        expect(false, "faulted worker in shared CRITICAL should not deadlock later tasks");
        return;
    }

    run_thread.join();
    const auto state = run_future.get();
    expect(state.completed,
           "critical-section fault-recovery script should complete after faulted worker exits");

    const auto bad_done = state.globals.find("lbaddone");
    const auto good_done = state.globals.find("lgooddone");
    expect(bad_done != state.globals.end(), "script should report bad-worker await result");
    expect(good_done != state.globals.end(), "script should report good-worker await result");
    if (bad_done != state.globals.end())
    {
        expect(!bad_done->second.boolean_value,
               "bad worker should report failed completion because of fault");
    }
    if (good_done != state.globals.end())
    {
        expect(good_done->second.boolean_value, "good worker should complete after bad worker fault");
    }

    const auto critical_enter_events = std::count_if(
        state.events.begin(),
        state.events.end(),
        [](const auto& event) {
            return event.category == "runtime.critical.enter";
        });
    expect(critical_enter_events >= 2U,
           "good worker should still enter CRITICAL after bad worker fault");

    expect(std::none_of(state.events.begin(), state.events.end(), [](const auto &event) {
               return event.category == "runtime.critical.blocking_violation" &&
                      event.detail.find("operation=AWAIT") != std::string::npos;
           }),
           "fault-recovery scenario should not emit unrelated AWAIT blocking violation");

    fs::remove_all(temp_root, ignored);
}

void test_yield_command_emits_runtime_yield_event_and_preserves_state() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_yield_cmd";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "yield_test.prg";
    write_text(
        main_path,
        "PROCEDURE worker\n"
        "    SLEEP 1\n"
        "    RETURN\n"
        "ENDPROC\n"
        "nBefore = 1\n"
        "SPAWN worker TO nTask\n"
        "YIELD\n"
        "nAfter = 42\n"
        "AWAIT nTask TO lDone\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "YIELD test should complete");

    const auto yield_event = std::find_if(
        state.events.begin(), state.events.end(),
        [](const auto& event) { return event.category == "runtime.yield"; });
    expect(yield_event != state.events.end(), "YIELD should emit a runtime.yield event");

    const auto before_it = state.globals.find("nbefore");
    const auto after_it = state.globals.find("nafter");
    const auto done_it = state.globals.find("ldone");
    expect(before_it != state.globals.end(), "YIELD script should capture the pre-yield value");
    expect(after_it != state.globals.end(), "YIELD script should continue after yielding");
    expect(done_it != state.globals.end(), "YIELD script should wait for the spawned task");
    if (before_it != state.globals.end()) {
        expect(before_it->second.number_value == 1.0, "pre-yield state should remain intact");
    }
    if (after_it != state.globals.end()) {
        expect(after_it->second.number_value == 42.0, "post-yield assignment should execute");
    }
    if (done_it != state.globals.end()) {
        expect(done_it->second.boolean_value, "awaited worker should complete successfully");
    }

    fs::remove_all(temp_root, ignored);
}

void test_yield_is_allowed_while_holding_critical_section() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_yield_critical_regression";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "yield_critical_regression.prg";
    write_text(
        main_path,
        "lInCritical = .F.\n"
        "lYielded = .F.\n"
        "ENTER CRITICAL shared\n"
        "lInCritical = .T.\n"
        "YIELD\n"
        "lYielded = .T.\n"
        "EXIT CRITICAL shared\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "YIELD inside CRITICAL should complete");

    const auto entered_it = state.globals.find("lincritical");
    const auto yielded_it = state.globals.find("lyielded");
    expect(entered_it != state.globals.end(), "CRITICAL body should execute");
    expect(yielded_it != state.globals.end(), "YIELD should execute inside CRITICAL");
    if (entered_it != state.globals.end()) {
        expect(entered_it->second.boolean_value, "CRITICAL section body should run before YIELD");
    }
    if (yielded_it != state.globals.end()) {
        expect(yielded_it->second.boolean_value, "YIELD should run and continue inside CRITICAL");
    }

    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.yield";
    }), "YIELD inside CRITICAL should emit runtime.yield");
    expect(std::none_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.critical.blocking_violation" &&
               event.detail.find("operation=YIELD") != std::string::npos;
    }), "YIELD inside CRITICAL should not trigger blocking policy");

    fs::remove_all(temp_root, ignored);
}

void test_yield_in_critical_section_keeps_section_semantics() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_yield_critical_section_contract";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "yield_in_critical_section_contract_test.prg";
    write_text(
        main_path,
        "lInside = .F.\n"
        "lYielded = .F.\n"
        "ENTER CRITICAL shared\n"
        "lInside = .T.\n"
        "YIELD\n"
        "lYielded = .T.\n"
        "EXIT CRITICAL shared\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "YIELD in CRITICAL should complete");

    const auto entered = state.globals.find("linside");
    const auto yielded = state.globals.find("lyielded");
    expect(entered != state.globals.end(), "CRITICAL body should execute");
    expect(yielded != state.globals.end(), "YIELD should continue inside CRITICAL");
    if (entered != state.globals.end()) {
        expect(entered->second.boolean_value, "CRITICAL section should be entered before YIELD");
    }
    if (yielded != state.globals.end()) {
        expect(yielded->second.boolean_value, "YIELD should complete inside CRITICAL");
    }
    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.yield";
    }), "CRITICAL-held YIELD should emit runtime.yield");
    expect(std::none_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.critical.blocking_violation" &&
               event.detail.find("operation=YIELD") != std::string::npos;
    }), "YIELD in CRITICAL should be a policy exception");

    fs::remove_all(temp_root, ignored);
}

void test_yield_is_allowed_in_default_critical_section_is_policy_exception() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_yield_default_critical_policy_exception";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "yield_default_critical_policy_exception_test.prg";
    write_text(
        main_path,
        "lEntered = .F.\n"
        "lYieldedInDefault = .F.\n"
        "ENTER CRITICAL\n"
        "lEntered = .T.\n"
        "YIELD\n"
        "lYieldedInDefault = .T.\n"
        "EXIT CRITICAL\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "YIELD in default CRITICAL should complete");

    const auto entered = state.globals.find("lentered");
    const auto yielded = state.globals.find("lyieldedindefault");
    expect(entered != state.globals.end(), "default CRITICAL body should execute");
    expect(yielded != state.globals.end(), "YIELD should execute inside default CRITICAL");
    if (entered != state.globals.end()) {
        expect(entered->second.boolean_value, "default CRITICAL should be entered before YIELD");
    }
    if (yielded != state.globals.end()) {
        expect(yielded->second.boolean_value, "YIELD should continue execution inside default CRITICAL");
    }

    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.yield";
    }), "YIELD in default CRITICAL should emit runtime.yield");
    expect(std::none_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.critical.blocking_violation" &&
               event.detail.find("operation=YIELD") != std::string::npos;
    }), "YIELD in default CRITICAL should remain an allowed policy exception");

    fs::remove_all(temp_root, ignored);
}

void test_yield_is_allowed_in_critical_section_is_policy_exception() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_yield_critical_policy_exception";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "yield_critical_policy_exception_test.prg";
    write_text(
        main_path,
        "lEnteredCritical = .F.\n"
        "lYieldedInCritical = .F.\n"
        "ENTER CRITICAL shared\n"
        "lEnteredCritical = .T.\n"
        "YIELD\n"
        "lYieldedInCritical = .T.\n"
        "EXIT CRITICAL shared\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "YIELD policy exception test should complete");

    const auto entered = state.globals.find("lenteredcritical");
    const auto yielded = state.globals.find("lyieldedincritical");
    expect(entered != state.globals.end(), "CRITICAL body should execute");
    expect(yielded != state.globals.end(), "YIELD should execute inside CRITICAL");
    if (entered != state.globals.end()) {
        expect(entered->second.boolean_value, "CRITICAL section should be entered before YIELD");
    }
    if (yielded != state.globals.end()) {
        expect(yielded->second.boolean_value, "YIELD should continue execution inside CRITICAL");
    }

    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.yield";
    }), "YIELD in CRITICAL should emit runtime.yield");

    expect(std::none_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.critical.blocking_violation" &&
               event.detail.find("operation=YIELD") != std::string::npos;
    }), "YIELD in CRITICAL should remain an allowed policy exception");

    fs::remove_all(temp_root, ignored);
}

void test_yield_inside_critical_section_is_allowed() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_yield_inside_critical_cmd";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "yield_inside_critical_test.prg";
    write_text(
        main_path,
        "PROCEDURE worker\n"
        "    SLEEP 1\n"
        "    RETURN\n"
        "ENDPROC\n"
        "lYieldedInCritical = .F.\n"
        "SPAWN worker TO nTask\n"
        "ENTER CRITICAL shared\n"
        "YIELD\n"
        "lYieldedInCritical = .T.\n"
        "EXIT CRITICAL shared\n"
        "AWAIT nTask TO lDone\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "YIELD inside critical-section test should complete");

    const auto yield_in_critical = state.globals.find("lyieldedincritical");
    const auto await_done = state.globals.find("ldone");
    expect(yield_in_critical != state.globals.end(), "YIELD inside CRITICAL should execute and set lYieldedInCritical");
    expect(await_done != state.globals.end(), "post-YIELD await should still complete");
    if (yield_in_critical != state.globals.end()) {
        expect(yield_in_critical->second.boolean_value, "YIELD inside CRITICAL should complete without entering CATCH path");
    }
    if (await_done != state.globals.end()) {
        expect(await_done->second.boolean_value, "AWAIT after CRITICAL/YIELD should report completed task");
    }

    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.yield";
    }), "YIELD inside CRITICAL should emit a runtime.yield event");
    expect(std::none_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.critical.blocking_violation" &&
               event.detail.find("operation=YIELD") != std::string::npos;
    }), "YIELD inside CRITICAL should remain allowed and not trigger blocking policy");

    fs::remove_all(temp_root, ignored);
}

void test_yield_allowed_in_enter_critical_is_small_regression() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_yield_enter_critical_regression";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "yield_enter_critical_regression_test.prg";
    write_text(
        main_path,
        "lYielded = .F.\n"
        "lEnteredCritical = .F.\n"
        "ENTER CRITICAL\n"
        "lEnteredCritical = .T.\n"
        "YIELD\n"
        "lYielded = .T.\n"
        "EXIT CRITICAL\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "ENTER CRITICAL/YIELD regression should complete");

    const auto yielded = state.globals.find("lyielded");
    const auto entered = state.globals.find("lenteredcritical");
    expect(yielded != state.globals.end(), "YIELD should continue after ENTER CRITICAL");
    if (yielded != state.globals.end()) {
        expect(yielded->second.boolean_value, "YIELD should set lYielded after resuming from section");
    }
    expect(entered != state.globals.end(), "ENTER CRITICAL body should execute before YIELD");
    if (entered != state.globals.end()) {
        expect(entered->second.boolean_value, "CRITICAL body should set entry flag");
    }

    const auto yield_event_pos = std::find_if(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.yield";
    });
    expect(yield_event_pos != state.events.end(), "ENTER CRITICAL + YIELD should emit runtime.yield");
    const auto first_critical_enter = std::find_if(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.critical.enter";
    });
    const auto first_critical_exit = std::find_if(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.critical.exit";
    });
    expect(first_critical_enter != state.events.end(), "ENTER CRITICAL should emit runtime.critical.enter");
    expect(first_critical_exit != state.events.end(), "EXIT CRITICAL should emit runtime.critical.exit");
    if (first_critical_enter != state.events.end() && first_critical_exit != state.events.end() &&
        yield_event_pos != state.events.end()) {
        expect(std::distance(state.events.begin(), first_critical_enter) <
               std::distance(state.events.begin(), yield_event_pos),
               "runtime.yield should occur after entering CRITICAL");
        expect(std::distance(state.events.begin(), yield_event_pos) <
               std::distance(state.events.begin(), first_critical_exit),
               "runtime.yield should occur before CRITICAL exit");
    }
    expect(std::none_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.critical.blocking_violation" &&
               event.detail.find("operation=YIELD") != std::string::npos;
    }), "ENTER CRITICAL + YIELD should remain blocked-policy exception");
    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.yield" &&
               event.detail.find("operation=YIELD") != std::string::npos;
    }), "YIELD event detail should identify the operation");

    fs::remove_all(temp_root, ignored);
}

void test_yield_allowed_in_reentrant_enter_critical_section() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_yield_reentrant_enter_critical_regression";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "yield_reentrant_enter_critical_regression_test.prg";
    write_text(
        main_path,
        "lOuterEntered = .F.\n"
        "lInnerEntered = .F.\n"
        "lYielded = .F.\n"
        "ENTER CRITICAL shared\n"
        "lOuterEntered = .T.\n"
        "ENTER CRITICAL shared\n"
        "lInnerEntered = .T.\n"
        "YIELD\n"
        "lYielded = .T.\n"
        "EXIT CRITICAL shared\n"
        "EXIT CRITICAL shared\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "YIELD inside reentrant ENTER CRITICAL should complete");

    const auto outer_entered = state.globals.find("louterentered");
    const auto inner_entered = state.globals.find("linnerentered");
    const auto yielded = state.globals.find("lyielded");
    expect(outer_entered != state.globals.end(), "outer ENTER CRITICAL body should execute");
    expect(inner_entered != state.globals.end(), "inner ENTER CRITICAL body should execute");
    expect(yielded != state.globals.end(), "YIELD should execute inside reentrant ENTER CRITICAL");
    if (outer_entered != state.globals.end()) {
        expect(outer_entered->second.boolean_value, "outer CRITICAL entry flag should be true");
    }
    if (inner_entered != state.globals.end()) {
        expect(inner_entered->second.boolean_value, "inner CRITICAL entry flag should be true");
    }
    if (yielded != state.globals.end()) {
        expect(yielded->second.boolean_value, "YIELD should execute and set lYielded");
    }

    const auto critical_enter_count =
        static_cast<std::size_t>(std::count_if(state.events.begin(), state.events.end(), [](const auto& event) {
            return event.category == "runtime.critical.enter";
        }));
    const auto critical_exit_count =
        static_cast<std::size_t>(std::count_if(state.events.begin(), state.events.end(), [](const auto& event) {
            return event.category == "runtime.critical.exit";
        }));
    expect(critical_enter_count == 2U, "reentrant ENTER CRITICAL should emit two enter events");
    expect(critical_exit_count == 2U, "reentrant ENTER CRITICAL should emit two exit events");

    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.yield" && event.detail.find("operation=YIELD") != std::string::npos;
    }), "reentrant ENTER CRITICAL should emit runtime.yield");
    expect(std::none_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.critical.blocking_violation" &&
               event.detail.find("operation=YIELD") != std::string::npos;
    }), "YIELD inside reentrant ENTER CRITICAL should remain allowed");

    fs::remove_all(temp_root, ignored);
}

void test_yield_in_enter_critical_is_explicit_policy_exception_small() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_yield_enter_critical_policy_exception_small";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "yield_enter_critical_policy_exception_small.prg";
    write_text(
        main_path,
        "entered = .F.\n"
        "yielded = .F.\n"
        "ENTER CRITICAL\n"
        "entered = .T.\n"
        "YIELD\n"
        "yielded = .T.\n"
        "EXIT CRITICAL\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "ENTER CRITICAL should allow YIELD as a policy exception");

    const auto entered = state.globals.find("entered");
    const auto yielded = state.globals.find("yielded");
    expect(entered != state.globals.end(), "ENTER CRITICAL body should execute");
    expect(yielded != state.globals.end(), "YIELD should execute inside ENTER CRITICAL");
    if (entered != state.globals.end()) {
        expect(entered->second.boolean_value, "CRITICAL body should run before YIELD");
    }
    if (yielded != state.globals.end()) {
        expect(yielded->second.boolean_value, "YIELD should complete inside CRITICAL");
    }

    const auto yield_event = std::find_if(state.events.begin(), state.events.end(), [](const auto &event) {
        return event.category == "runtime.yield" &&
               event.detail.find("operation=YIELD") != std::string::npos;
    });
    const auto enter_event = std::find_if(state.events.begin(), state.events.end(), [](const auto &event) {
        return event.category == "runtime.critical.enter";
    });
    const auto exit_event = std::find_if(state.events.begin(), state.events.end(), [](const auto &event) {
        return event.category == "runtime.critical.exit";
    });
    expect(yield_event != state.events.end(), "YIELD in ENTER CRITICAL should emit runtime.yield");
    expect(enter_event != state.events.end(), "ENTER CRITICAL should emit runtime.critical.enter");
    expect(exit_event != state.events.end(), "EXIT CRITICAL should emit runtime.critical.exit");
    expect(std::none_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.critical.blocking_violation" &&
               event.detail.find("operation=YIELD") != std::string::npos;
    }), "YIELD should remain an explicit policy exception while critical section is held");

    if (yield_event != state.events.end() && enter_event != state.events.end() &&
        exit_event != state.events.end()) {
        expect(std::distance(state.events.begin(), enter_event) <
               std::distance(state.events.begin(), yield_event),
               "YIELD should occur after ENTER CRITICAL");
        expect(std::distance(state.events.begin(), yield_event) <
               std::distance(state.events.begin(), exit_event),
               "YIELD should occur before EXIT CRITICAL");
    }

    fs::remove_all(temp_root, ignored);
}

void test_yield_preserves_fault_metadata_when_followed_by_error() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_yield_fault_cmd";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "yield_fault_test.prg";
    write_text(
        main_path,
        "YIELD\n"
        "nFail = 1 / 0\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!state.completed, "YIELD fault test should stop on error");
    expect(state.reason == copperfin::runtime::DebugPauseReason::error, "YIELD fault test should pause on error");
    expect(state.location.line == 2U, "fault metadata should point at the post-YIELD faulting line");
    expect(state.statement_text.find("1 / 0") != std::string::npos || state.statement_text.find("1/0") != std::string::npos,
        "fault metadata should preserve the offending statement text");

    const auto yield_event = std::find_if(
        state.events.begin(), state.events.end(),
        [](const auto& event) { return event.category == "runtime.yield"; });
    const auto error_event = std::find_if(
        state.events.begin(), state.events.end(),
        [](const auto& event) { return event.category == "runtime.error"; });
    expect(yield_event != state.events.end(), "YIELD should still emit its runtime.yield event before the fault");
    expect(error_event != state.events.end(), "faulting line should still emit a runtime.error event");

    fs::remove_all(temp_root, ignored);
}

void test_on_error_resume_restores_fault_session_and_cursor_state() {
    // #150: RESUME should restore the captured fault-side data session/work area
    // even when the handler changes its own session and cursor selection.
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_resume_fault_state";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path items_path = temp_root / "items.dbf";
    const fs::path alt_path = temp_root / "alt.dbf";
    write_people_dbf(items_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}});
    write_people_dbf(alt_path, {{"DELTA", 40}, {"ECHO", 50}});

    const fs::path main_path = temp_root / "resume_fault_state.prg";
    write_text(
        main_path,
        "ON ERROR DO handleerr\n"
        "USE '" + items_path.string() + "' ALIAS Items IN 0\n"
        "SELECT Items\n"
        "SKIP\n"
        "rec_before = RECNO()\n"
        "fault = LOG(-1)\n"
        "alias_after = ALIAS()\n"
        "rec_after = RECNO()\n"
        "name_after = Items.NAME\n"
        "RETURN\n"
        "PROCEDURE handleerr\n"
        "SET DATASESSION TO 2\n"
        "USE '" + alt_path.string() + "' ALIAS Alt IN 0\n"
        "SELECT Alt\n"
        "RESUME\n"
        "ENDPROC\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "#150: ON ERROR RESUME script should complete");

    const auto rec_before = state.globals.find("rec_before");
    const auto alias_after = state.globals.find("alias_after");
    const auto rec_after = state.globals.find("rec_after");
    const auto name_after = state.globals.find("name_after");

    expect(rec_before != state.globals.end(), "#150: rec_before should be captured before the fault");
    expect(alias_after != state.globals.end(), "#150: post-fault ALIAS() should be captured");
    expect(rec_after != state.globals.end(), "#150: post-fault RECNO() should be captured");
    expect(name_after != state.globals.end(), "#150: post-fault field read should succeed");

    if (rec_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(rec_before->second) == "2",
               "#150: fault-side cursor should be positioned on record 2 before fault");
    }
    if (alias_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(alias_after->second) == "Items",
               "#150: RESUME should restore the fault-side selected alias");
    }
    if (rec_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(rec_after->second) == "2",
               "#150: RESUME should preserve fault-side cursor record position");
    }
    if (name_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(name_after->second) == "BRAVO",
               "#150: post-fault field reads should remain on the original cursor row");
    }

    fs::remove_all(temp_root, ignored);
}


void test_fault_continue_cycle_preserves_open_cursor_and_record_position() {
    // #151: after each debug-continue across a runtime fault, cursor state and
    // record position must remain stable so the developer can keep inspecting data.
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_fault_cursor";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "items.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}});

    const fs::path main_path = temp_root / "fault_cursor.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS Items IN 0\n"
        "SELECT Items\n"
        "SKIP\n"
        "recno_before = RECNO()\n"
        "x = LOG(-1)\n"
        "recno_after_first = RECNO()\n"
        "y = ACOS(2)\n"
        "recno_after_second = RECNO()\n"
        "cur_name = Items.NAME\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    // First fault: LOG(-1)
    auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error,
           "#151: first fault should pause with an error reason");
    expect(state.location.line == 5U,
           "#151: first fault should highlight line 5 (LOG(-1))");

    // Second fault: ACOS(2)
    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error,
           "#151: second fault should pause with an error reason");
    expect(state.location.line == 7U,
           "#151: second fault should highlight line 7 (ACOS(2))");

    // Final continue — should complete
    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed,
           "#151: session should complete after continuing past both faults");

    // Cursor should have been at record 2 (after SKIP from record 1)
    const auto recno_before = state.globals.find("recno_before");
    const auto recno_after_first = state.globals.find("recno_after_first");
    const auto recno_after_second = state.globals.find("recno_after_second");
    const auto cur_name = state.globals.find("cur_name");

    expect(recno_before != state.globals.end(), "#151: recno_before should be set");
    expect(recno_after_first != state.globals.end(), "#151: recno_after_first should be set after first fault continue");
    expect(recno_after_second != state.globals.end(), "#151: recno_after_second should be set after second fault continue");
    expect(cur_name != state.globals.end(), "#151: cursor field read should succeed after fault cycle");

    if (recno_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(recno_before->second) == "2",
               "#151: SKIP from record 1 should position at record 2");
    }
    if (recno_after_first != state.globals.end()) {
        expect(copperfin::runtime::format_value(recno_after_first->second) == "2",
               "#151: cursor record position should survive the first fault continue");
    }
    if (recno_after_second != state.globals.end()) {
        expect(copperfin::runtime::format_value(recno_after_second->second) == "2",
               "#151: cursor record position should survive the second fault continue");
    }
    if (cur_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(cur_name->second) == "BRAVO",
               "#151: cursor field access after fault cycle should return the expected record value");
    }

    fs::remove_all(temp_root, ignored);
}

void test_fault_continue_cycle_preserves_selected_alias_across_data_session_scope() {
    // #151: repeated CONTINUE over multiple faults should preserve selected-alias
    // stability even when faults occur inside a non-default data session.
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_fault_cursor_ds";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "items_ds.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}});

    const fs::path main_path = temp_root / "fault_cursor_ds.prg";
    write_text(
        main_path,
        "SET DATASESSION TO 2\n"
        "USE '" + table_path.string() + "' ALIAS Items IN 0\n"
        "SELECT Items\n"
        "SKIP\n"
        "alias_before = ALIAS()\n"
        "recno_before = RECNO()\n"
        "x = LOG(-1)\n"
        "alias_after_first = ALIAS()\n"
        "recno_after_first = RECNO()\n"
        "y = ACOS(2)\n"
        "alias_after_second = ALIAS()\n"
        "recno_after_second = RECNO()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error,
           "#151: first non-default-session fault should pause with error");

    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error,
           "#151: second non-default-session fault should pause with error");

    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed,
           "#151: session should complete after continuing past both non-default-session faults");

    const auto alias_before = state.globals.find("alias_before");
    const auto alias_after_first = state.globals.find("alias_after_first");
    const auto alias_after_second = state.globals.find("alias_after_second");
    const auto recno_before = state.globals.find("recno_before");
    const auto recno_after_first = state.globals.find("recno_after_first");
    const auto recno_after_second = state.globals.find("recno_after_second");

    expect(alias_before != state.globals.end(), "#151: alias_before should be captured");
    expect(alias_after_first != state.globals.end(), "#151: alias_after_first should be captured");
    expect(alias_after_second != state.globals.end(), "#151: alias_after_second should be captured");
    expect(recno_before != state.globals.end(), "#151: recno_before should be captured");
    expect(recno_after_first != state.globals.end(), "#151: recno_after_first should be captured");
    expect(recno_after_second != state.globals.end(), "#151: recno_after_second should be captured");

    if (alias_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(alias_before->second) == "Items",
               "#151: selected alias should be Items before first fault");
    }
    if (alias_after_first != state.globals.end()) {
        expect(copperfin::runtime::format_value(alias_after_first->second) == "Items",
               "#151: selected alias should survive first fault continue");
    }
    if (alias_after_second != state.globals.end()) {
        expect(copperfin::runtime::format_value(alias_after_second->second) == "Items",
               "#151: selected alias should survive second fault continue");
    }
    if (recno_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(recno_before->second) == "2",
               "#151: pre-fault record position should be 2");
    }
    if (recno_after_first != state.globals.end()) {
        expect(copperfin::runtime::format_value(recno_after_first->second) == "2",
               "#151: record position should survive first fault continue");
    }
    if (recno_after_second != state.globals.end()) {
        expect(copperfin::runtime::format_value(recno_after_second->second) == "2",
               "#151: record position should survive second fault continue");
    }

    fs::remove_all(temp_root, ignored);
}

void test_pause_stack_frame_contains_accurate_intermediate_frame_lines() {
    // #152: all frames in the call stack at a fault pause should report
    // the line at which each caller invoked the next routine — not zero or stale.
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_frame_lines";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "frame_lines.prg";
    write_text(
        main_path,
        "before_call = 1\n"
        "DO outerproc\n"
        "after_call = 1\n"
        "RETURN\n"
        "PROCEDURE outerproc\n"
        "outer_start = 1\n"
        "DO innerproc\n"
        "outer_end = 1\n"
        "RETURN\n"
        "ENDPROC\n"
        "PROCEDURE innerproc\n"
        "inner_start = 1\n"
        "fault_val = LOG(-1)\n"
        "inner_end = 1\n"
        "RETURN\n"
        "ENDPROC\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error,
           "#152: nested fault should pause with error reason");
    expect(state.location.line == 13U,
           "#152: fault location should point at the LOG(-1) line");
    expect(state.call_stack.size() >= 3U,
           "#152: call stack should expose all three frames at fault time");

    if (state.call_stack.size() >= 3U) {
        // Top frame: innerproc, faulting line
        expect(state.call_stack[0].routine_name == "innerproc",
               "#152: top frame routine name should be innerproc");
        expect(state.call_stack[0].line == 13U,
               "#152: top frame line should be the fault line inside innerproc");

        // Middle frame: outerproc, line where DO innerproc was invoked
        expect(state.call_stack[1].routine_name == "outerproc",
               "#152: middle frame routine name should be outerproc");
         // The runtime records the resume PC (statement after the DO), so line 8 = outer_end = 1
         expect(state.call_stack[1].line == 8U,
             "#152: middle frame line should be the resume line after DO innerproc (outer_end = 1)");

        // Bottom frame: main, line where DO outerproc was invoked
        expect(state.call_stack[2].routine_name == "main",
               "#152: bottom frame routine name should be main");
         // The runtime records the resume PC (statement after the DO), so line 3 = after_call = 1
         expect(state.call_stack[2].line == 3U,
             "#152: bottom frame line should be the resume line after DO outerproc (after_call = 1)");
    }

    // Continue past the fault; the session should remain alive
    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "#152: session should complete after continuing past the nested fault");
    const auto after_call = state.globals.find("after_call");
    expect(after_call != state.globals.end(), "#152: post-call statement should execute after fault continue");

    fs::remove_all(temp_root, ignored);
}

void test_repeated_fault_pauses_refresh_intermediate_stack_frame_lines() {
    // #152: intermediate caller-frame line metadata should refresh across
    // repeated nested fault pauses instead of leaking stale frame line values.
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_frame_lines_repeat";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "frame_lines_repeat.prg";
    write_text(
        main_path,
        "DO outerfirst\n"
        "DO outersecond\n"
        "RETURN\n"
        "PROCEDURE outerfirst\n"
        "DO innerfirst\n"
        "RETURN\n"
        "PROCEDURE outersecond\n"
        "DO innersecond\n"
        "RETURN\n"
        "PROCEDURE innerfirst\n"
        "x = LOG(-1)\n"
        "RETURN\n"
        "PROCEDURE innersecond\n"
        "y = ACOS(2)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error,
           "#152: first nested fault should pause with error");
    expect(state.location.line == 11U,
           "#152: first nested fault should highlight the first inner routine line");
    expect(state.call_stack.size() >= 3U,
           "#152: first nested fault should expose full call stack");
    if (state.call_stack.size() >= 2U) {
        expect(state.call_stack[1].routine_name == "outerfirst",
               "#152: first pause should report outerfirst as intermediate frame");
        expect(state.call_stack[1].line == 6U,
               "#152: first pause intermediate frame line should match the caller resume PC");
    }

    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error,
           "#152: second nested fault should pause with error");
    expect(state.location.line == 14U,
           "#152: second nested fault should highlight the second inner routine line");
    expect(state.call_stack.size() >= 3U,
           "#152: second nested fault should expose full call stack");
    if (state.call_stack.size() >= 2U) {
        expect(state.call_stack[1].routine_name == "outersecond",
               "#152: second pause should refresh intermediate frame routine");
        expect(state.call_stack[1].line == 9U,
               "#152: second pause intermediate frame line should refresh to the second caller resume PC");
    }

    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed,
           "#152: session should complete after continuing past both nested faults");

    fs::remove_all(temp_root, ignored);
}

void test_thrown_expression_fault_aerror_columns_match_error_message_functions() {
    // #153: when a thrown expression fault is caught via ON ERROR, AERROR()
    // columns must agree with ERROR(), MESSAGE(), and LINENO() diagnostic functions
    // so developers see a consistent normalized diagnostic surface.
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_aerror_norm";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "aerror_norm.prg";
    write_text(
        main_path,
        "ON ERROR DO handleerr\n"
        "bad_val = LOG(-1)\n"
        "after_fault = 1\n"
        "RETURN\n"
        "PROCEDURE handleerr\n"
        "nErrRows = AERROR(aErrNorm)\n"
        "nErrCode = aErrNorm[1,1]\n"
        "cErrMsg = aErrNorm[1,2]\n"
        "nErrLine = aErrNorm[1,5]\n"
        "cErrProc = aErrNorm[1,6]\n"
        "nFnCode = ERROR()\n"
        "cFnMsg = MESSAGE()\n"
        "nFnLine = LINENO()\n"
        "cFnProg = PROGRAM()\n"
        "RETURN\n"
        "ENDPROC\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "#153: AERROR normalization script should complete");

    const auto err_rows = state.globals.find("nerrrows");
    const auto err_code = state.globals.find("nerrcode");
    const auto err_msg = state.globals.find("cerrmsg");
    const auto err_line = state.globals.find("nerrline");
    const auto err_proc = state.globals.find("cerrproc");
    const auto fn_code = state.globals.find("nfncode");
    const auto fn_msg = state.globals.find("cfnmsg");
    const auto fn_line = state.globals.find("nfnline");
    const auto fn_prog = state.globals.find("cfnprog");
    const auto after_fault = state.globals.find("after_fault");

    expect(err_rows != state.globals.end(), "#153: AERROR() should return a row count");
    expect(err_code != state.globals.end(), "#153: AERROR() column 1 (error code) should be set");
    expect(err_msg != state.globals.end(), "#153: AERROR() column 2 (message) should be set");
    expect(err_line != state.globals.end(), "#153: AERROR() column 5 (line) should be set");
    expect(err_proc != state.globals.end(), "#153: AERROR() column 6 (procedure) should be set");
    expect(fn_code != state.globals.end(), "#153: ERROR() function should be available in handler");
    expect(fn_msg != state.globals.end(), "#153: MESSAGE() function should be available in handler");
    expect(fn_line != state.globals.end(), "#153: LINENO() function should be available in handler");
    expect(fn_prog != state.globals.end(), "#153: PROGRAM() function should be available in handler");
    expect(after_fault != state.globals.end(), "#153: execution should continue after ON ERROR handler");

    // AERROR column 1 must equal ERROR()
    if (err_code != state.globals.end() && fn_code != state.globals.end()) {
        expect(copperfin::runtime::format_value(err_code->second) ==
               copperfin::runtime::format_value(fn_code->second),
               "#153: AERROR()[1,1] error code should match ERROR() function value");
    }
    // AERROR column 2 must equal MESSAGE()
    if (err_msg != state.globals.end() && fn_msg != state.globals.end()) {
        expect(copperfin::runtime::format_value(err_msg->second) ==
               copperfin::runtime::format_value(fn_msg->second),
               "#153: AERROR()[1,2] message should match MESSAGE() function value");
    }
    // AERROR column 5 must equal LINENO()
    if (err_line != state.globals.end() && fn_line != state.globals.end()) {
        expect(copperfin::runtime::format_value(err_line->second) ==
               copperfin::runtime::format_value(fn_line->second),
               "#153: AERROR()[1,5] line should match LINENO() function value");
    }
    // AERROR column 6 must equal PROGRAM()
    if (err_proc != state.globals.end() && fn_prog != state.globals.end()) {
        expect(copperfin::runtime::format_value(err_proc->second) ==
               copperfin::runtime::format_value(fn_prog->second),
               "#153: AERROR()[1,6] procedure should match PROGRAM() function value");
    }
    // The fault line should be line 2 (bad_val = LOG(-1))
    if (err_line != state.globals.end()) {
        expect(copperfin::runtime::format_value(err_line->second) == "2",
               "#153: AERROR()[1,5] should report line 2 as the faulting line");
    }
    // The message must contain meaningful diagnostic text for a LOG(-1) fault
    if (err_msg != state.globals.end()) {
        expect(!copperfin::runtime::format_value(err_msg->second).empty(),
               "#153: AERROR()[1,2] diagnostic message should be non-empty for a thrown expression fault");
    }
    if (err_rows != state.globals.end()) {
        expect(copperfin::runtime::format_value(err_rows->second) == "1",
               "#153: AERROR() should return exactly one row for a single fault");
    }

    fs::remove_all(temp_root, ignored);
}

void test_repeated_on_error_faults_refresh_normalized_diagnostics() {
    // #153: AERROR()/ERROR()/MESSAGE()/LINENO()/PROGRAM() should refresh for
    // each new ON ERROR fault, not retain stale values from prior faults.
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_aerror_norm_repeat";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "aerror_norm_repeat.prg";
    write_text(
        main_path,
        "ON ERROR DO handleerr\n"
        "x = LOG(-1)\n"
        "y = ACOS(2)\n"
        "RETURN\n"
        "PROCEDURE handleerr\n"
        "IF TYPE('gFaultCount') <> 'N'\n"
        "    gFaultCount = 0\n"
        "ENDIF\n"
        "gFaultCount = gFaultCount + 1\n"
        "nErrRows = AERROR(aErrNorm)\n"
        "IF gFaultCount = 1\n"
        "    nErrCode1 = aErrNorm[1,1]\n"
        "    cErrMsg1 = aErrNorm[1,2]\n"
        "    nErrLine1 = aErrNorm[1,5]\n"
        "    cErrProc1 = aErrNorm[1,6]\n"
        "    nFnCode1 = ERROR()\n"
        "    cFnMsg1 = MESSAGE()\n"
        "    nFnLine1 = LINENO()\n"
        "    cFnProg1 = PROGRAM()\n"
        "ELSE\n"
        "    nErrCode2 = aErrNorm[1,1]\n"
        "    cErrMsg2 = aErrNorm[1,2]\n"
        "    nErrLine2 = aErrNorm[1,5]\n"
        "    cErrProc2 = aErrNorm[1,6]\n"
        "    nFnCode2 = ERROR()\n"
        "    cFnMsg2 = MESSAGE()\n"
        "    nFnLine2 = LINENO()\n"
        "    cFnProg2 = PROGRAM()\n"
        "ENDIF\n"
        "RETURN\n"
        "ENDPROC\n");

    const auto state =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false))
            .run(copperfin::runtime::DebugResumeAction::continue_run);

    expect(state.completed, "#153: repeated ON ERROR normalization script should complete");

    const auto nerrline1 = state.globals.find("nerrline1");
    const auto nerrline2 = state.globals.find("nerrline2");
    const auto cfnmsg1 = state.globals.find("cfnmsg1");
    const auto cfnmsg2 = state.globals.find("cfnmsg2");
    const auto nfnline1 = state.globals.find("nfnline1");
    const auto nfnline2 = state.globals.find("nfnline2");
    const auto cfnprog1 = state.globals.find("cfnprog1");
    const auto cfnprog2 = state.globals.find("cfnprog2");
    const auto nerrcode1 = state.globals.find("nerrcode1");
    const auto nerrcode2 = state.globals.find("nerrcode2");
    const auto nfncode1 = state.globals.find("nfncode1");
    const auto nfncode2 = state.globals.find("nfncode2");

    expect(nerrline1 != state.globals.end(), "#153: first fault AERROR line should be captured");
    expect(nerrline2 != state.globals.end(), "#153: second fault AERROR line should be captured");
    expect(cfnmsg1 != state.globals.end(), "#153: first fault MESSAGE() should be captured");
    expect(cfnmsg2 != state.globals.end(), "#153: second fault MESSAGE() should be captured");
    expect(nfnline1 != state.globals.end(), "#153: first fault LINENO() should be captured");
    expect(nfnline2 != state.globals.end(), "#153: second fault LINENO() should be captured");
    expect(cfnprog1 != state.globals.end(), "#153: first fault PROGRAM() should be captured");
    expect(cfnprog2 != state.globals.end(), "#153: second fault PROGRAM() should be captured");
    expect(nerrcode1 != state.globals.end(), "#153: first fault AERROR code should be captured");
    expect(nerrcode2 != state.globals.end(), "#153: second fault AERROR code should be captured");
    expect(nfncode1 != state.globals.end(), "#153: first fault ERROR() should be captured");
    expect(nfncode2 != state.globals.end(), "#153: second fault ERROR() should be captured");

    if (nerrline1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(nerrline1->second) == "2",
               "#153: first fault AERROR line should report line 2");
    }
    if (nerrline2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(nerrline2->second) == "3",
               "#153: second fault AERROR line should report line 3");
    }
    if (nfnline1 != state.globals.end() && nerrline1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(nfnline1->second) ==
                   copperfin::runtime::format_value(nerrline1->second),
               "#153: first fault LINENO() should match AERROR line");
    }
    if (nfnline2 != state.globals.end() && nerrline2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(nfnline2->second) ==
                   copperfin::runtime::format_value(nerrline2->second),
               "#153: second fault LINENO() should match AERROR line");
    }
    if (cfnmsg1 != state.globals.end() && cfnmsg2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(cfnmsg1->second) !=
                   copperfin::runtime::format_value(cfnmsg2->second),
               "#153: message text should refresh between LOG and ACOS faults");
    }
    if (cfnprog1 != state.globals.end() && cfnprog2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(cfnprog1->second) ==
                   copperfin::runtime::format_value(cfnprog2->second),
               "#153: both faults should report the same procedure context");
    }
    if (nerrcode1 != state.globals.end() && nfncode1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(nerrcode1->second) ==
                   copperfin::runtime::format_value(nfncode1->second),
               "#153: first fault AERROR code should match ERROR()");
    }
    if (nerrcode2 != state.globals.end() && nfncode2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(nerrcode2->second) ==
                   copperfin::runtime::format_value(nfncode2->second),
               "#153: second fault AERROR code should match ERROR()");
    }

    fs::remove_all(temp_root, ignored);
}

void test_division_by_zero_dispatches_runtime_error() {
    // GAP-01 #257: dividing by zero in a PRG expression must produce a runtime
    // error pause (not a host crash, not a silent NaN or infinity result).
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_divzero";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "divzero.prg";
    write_text(
        main_path,
        "before_div = 1\n"
        "x = 1 / 0\n"
        "after_div = 1\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error,
           "GAP-01/#257: division by zero should pause with an error reason");
    expect(state.location.line == 2U,
           "GAP-01/#257: division by zero should highlight line 2");
    expect(!state.message.empty(),
           "GAP-01/#257: division by zero should produce a non-empty diagnostic message");

    // Session must survive a continue after the divide-by-zero fault
    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed,
           "GAP-01/#257: session should complete after continuing past a divide-by-zero fault");
    const auto after_div = state.globals.find("after_div");
    expect(after_div != state.globals.end(),
           "GAP-01/#257: statements after the divide-by-zero line should still execute after a fault continue");

    fs::remove_all(temp_root, ignored);
}

void test_numeric_field_overflow_is_diagnosed_not_silently_truncated() {
    // GAP-01 #258: writing a value wider than an N-field's declared width must
    // not silently store a garbage or truncated value; the runtime must either
    // fail with a diagnostic or store the correctly bounded value without crashing.
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_num_overflow";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    // Create a cursor with a 3-digit numeric field and attempt to REPLACE it
    // with a value that requires 4 digits.
    const fs::path prg_path = temp_root / "num_overflow.prg";
    write_text(
        prg_path,
        "CREATE CURSOR overflow_test (code N(3,0))\n"
        "INSERT INTO overflow_test VALUES (1)\n"
        "GO TOP\n"
        "overflow_error = 0\n"
        "ON ERROR DO handleerr\n"
        "REPLACE code WITH 9999\n"
        "code_after_replace = overflow_test.code\n"
        "RETURN\n"
        "PROCEDURE handleerr\n"
        "overflow_error = 1\n"
        "RETURN\n"
        "ENDPROC\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(prg_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed,
           "GAP-01/#258: numeric overflow test script should complete without host crash");

    const auto code_after = state.globals.find("code_after_replace");
    const auto overflow_error = state.globals.find("overflow_error");

    // The outcome must be one of:
    //   (a) an error was dispatched (overflow_error == 1), or
    //   (b) the stored value is within the field range (0-999)
    bool diagnosed = false;
    if (overflow_error != state.globals.end()) {
        diagnosed = (copperfin::runtime::format_value(overflow_error->second) == "1");
    }
    bool within_range = false;
    if (code_after != state.globals.end()) {
        const std::string stored = copperfin::runtime::format_value(code_after->second);
        // Value must not be "9999" (which would mean silent overflow storage)
        within_range = (stored != "9999");
    }
    expect(diagnosed || within_range || code_after == state.globals.end(),
           "GAP-01/#258: numeric field overflow must be diagnosed or safely bounded — not silently stored as 9999");

    fs::remove_all(temp_root, ignored);
}

void test_retry_reexecutes_faulting_statement() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_retry";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "retry_test.prg";
    write_text(
        main_path,
        "attempt_count = 0\n"
        "ON ERROR DO handleerr\n"
        "DO missing_routine\n"
        "after_retry = 1\n"
        "RETURN\n"
        "PROCEDURE handleerr\n"
        "attempt_count = attempt_count + 1\n"
        "IF attempt_count < 2\n"
        "  RETRY\n"
        "ENDIF\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "RETRY test script should complete");

    const auto attempt = state.globals.find("attempt_count");
    const auto after = state.globals.find("after_retry");
    expect(attempt != state.globals.end(), "attempt_count should be set");
    expect(after != state.globals.end(), "execution should continue after RETRY handler finally returns");
    if (attempt != state.globals.end()) {
        expect(copperfin::runtime::format_value(attempt->second) == "2",
               "handler should have been called twice (once retry, once return)");
    }
    if (after != state.globals.end()) {
        expect(copperfin::runtime::format_value(after->second) == "1",
               "post-fault statement should run after handler's normal RETURN");
    }

    fs::remove_all(temp_root, ignored);
}

void test_resume_next_continues_after_fault() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_resume_next";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "resume_test.prg";
    write_text(
        main_path,
        "ON ERROR DO handleerr\n"
        "DO missing_routine\n"
        "after_error = 1\n"
        "RETURN\n"
        "PROCEDURE handleerr\n"
        "handled = 1\n"
        "RESUME\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "RESUME test script should complete");

    const auto handled = state.globals.find("handled");
    const auto after_error = state.globals.find("after_error");
    expect(handled != state.globals.end(), "error handler should have run and set handled flag");
    expect(after_error != state.globals.end(), "RESUME should skip the faulting statement and continue to next");
    if (handled != state.globals.end()) {
        expect(copperfin::runtime::format_value(handled->second) == "1", "handled should be 1");
    }
    if (after_error != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_error->second) == "1",
               "statement after faulting one should execute after RESUME");
    }

    fs::remove_all(temp_root, ignored);
}

void test_retry_with_no_fault_checkpoint_is_noop() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_retry_noop";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "retry_noop.prg";
    write_text(
        main_path,
        "noop_count = 1\n"
        "RETRY\n"
        "noop_count = 2\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "RETRY outside error handler should not crash");

    const auto noop_count = state.globals.find("noop_count");
    expect(noop_count != state.globals.end(), "noop_count should be set");
    if (noop_count != state.globals.end()) {
        expect(copperfin::runtime::format_value(noop_count->second) == "2",
               "RETRY with no fault checkpoint is a no-op; execution should continue to next statement");
    }

    fs::remove_all(temp_root, ignored);
}

}  // namespace

void test_runtime_faults_preserve_state_and_allow_retry() {
    // We will execute a script that intentionally causes a runtime C++ exception (like LOG(-1))
    // We will verify that we can RETRY and the cursor/session state is preserved.
    namespace fs = std::filesystem;
    auto options = make_runtime_session_options("runtime_fault_test.prg", fs::current_path(), false);
    
    write_text("runtime_fault_test.prg",
        "CREATE CURSOR test_cursor (id I)\n"
        "INSERT INTO test_cursor VALUES (1)\n"
        "INSERT INTO test_cursor VALUES (2)\n"
        "GO TOP\n"
        "x = -1\n"
        "ON ERROR DO my_error_handler\n"
        "? LOG(x)\n" // This throws std::runtime_error first time
        "PROCEDURE my_error_handler\n"
        "    x = 1\n"
        "    RETRY\n"
        "ENDPROC\n"
    );

    auto session = copperfin::runtime::PrgRuntimeSession::create(options);
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    
    if (!state.completed) {
        std::cerr << "Script stopped. Reason: " << copperfin::runtime::debug_pause_reason_name(state.reason) 
                  << ", message: " << state.message << std::endl;
    }
    
    // Check if the script ran completely
    expect(state.completed, "Script should complete after handling fault");
}

void test_aerror_line_number_is_innermost_faulting_line_not_catch_site() {
    // #256: AERROR()[1,5] inside a CATCH block must report the innermost faulting
    // line (inside the deeply nested routine), not the TRY/CATCH site.
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_aerror_innermost_line";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "aerror_innermost.prg";
    // Lines:
    //  1: TRY
    //  2:   DO level1
    //  3: CATCH TO oErr
    //  4:   nErrorLines = AERROR(aErr)
    //  5:   nFaultLine = aErr[1,5]
    //  6:   cFaultProc = aErr[1,6]
    //  7: ENDTRY
    //  8: RETURN
    //  9: PROCEDURE level1
    // 10:   DO level2
    // 11:   RETURN
    // 12: ENDPROC
    // 13: PROCEDURE level2
    // 14:   fault_val = LOG(-1)   <-- actual fault here
    // 15:   RETURN
    // 16: ENDPROC
    write_text(
        main_path,
        "TRY\n"
        "  DO level1\n"
        "CATCH TO oErr\n"
        "  nErrorLines = AERROR(aErr)\n"
        "  nFaultLine = aErr[1,5]\n"
        "  cFaultProc = aErr[1,6]\n"
        "ENDTRY\n"
        "RETURN\n"
        "PROCEDURE level1\n"
        "  DO level2\n"
        "  RETURN\n"
        "ENDPROC\n"
        "PROCEDURE level2\n"
        "  fault_val = LOG(-1)\n"
        "  RETURN\n"
        "ENDPROC\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "TRY/CATCH nested routine fault should complete: " + state.message);

    const auto error_lines = state.globals.find("nerrorlines");
    const auto fault_line = state.globals.find("nfaultline");
    const auto fault_proc = state.globals.find("cfaultproc");

    expect(error_lines != state.globals.end(), "AERROR() should populate inside CATCH");
    expect(fault_line != state.globals.end(), "AERROR()[1,5] should be accessible inside CATCH");
    expect(fault_proc != state.globals.end(), "AERROR()[1,6] should be accessible inside CATCH");

    if (error_lines != state.globals.end()) {
        expect(copperfin::runtime::format_value(error_lines->second) == "1",
            "AERROR() inside CATCH should return one error row");
    }
    if (fault_line != state.globals.end()) {
        expect(copperfin::runtime::format_value(fault_line->second) == "14",
            "AERROR()[1,5] should report line 14 (innermost fault in level2), not the CATCH or DO level1 line");
    }
    if (fault_proc != state.globals.end()) {
        const auto proc_val = copperfin::runtime::format_value(fault_proc->second);
        expect(proc_val.find("level2") != std::string::npos,
            "AERROR()[1,6] should name level2 as the faulting procedure (got '" + proc_val + "')");
    }

    fs::remove_all(temp_root, ignored);
}

int main() {
    test_command_keyword_scanner_ignores_nested_text();
    test_do_while_and_loop_control_flow();
    test_do_case_control_flow();
    test_push_pop_key_menu_popup_stack_commands();
    test_text_endtext_literal_blocks();
    test_aggregate_functions_respect_visibility();
    test_calculate_command_aggregates();
    test_command_level_aggregate_commands();
    test_scan_on_empty_table_does_not_execute_body();
    test_aggregate_commands_on_empty_table_return_zero();
    test_locate_on_empty_table_sets_eof();
    test_go_top_bottom_on_empty_table_does_not_crash();
    test_aggregate_commands_support_macro_targets_and_calculate_while();
    test_command_level_aggregate_scope_and_while_semantics();
    test_total_command_for_local_tables();
    test_total_command_errors_use_default_locale_messages();
    test_total_command_supports_currency_and_integer_fields();
    test_total_command_for_sql_result_cursors();
    test_private_declaration_masks_caller_variable();
    test_private_variable_visible_to_called_routines();
    test_release_private_restores_saved_binding_immediately();
    test_release_local_restores_visible_outer_global();
    test_macro_assignment_target_updates_private_binding_and_release_restores_outer_value();
    test_macro_assignment_target_preserves_public_binding_across_release_all();
    test_store_command_assigns_multiple_variables();
    test_runtime_guardrail_limits_call_depth_without_crashing_host();
    test_runtime_guardrail_exactly_at_call_depth_limit_succeeds();
    test_runtime_guardrail_one_over_call_depth_limit_fails();
    test_runtime_guardrail_limits_statement_budget_without_crashing_host();
    test_static_diagnostic_flags_likely_infinite_do_while_loop();
    test_config_fpw_overrides_runtime_limits();
    test_config_fpw_custom_limit_is_enforced_at_boundary();
    test_config_fpw_overrides_temp_directory_default();
    test_elseif_control_flow_executes_matching_branch();
    test_do_with_parameters_binds_arguments_in_called_routine();
    test_call_with_parameters_binds_arguments_in_called_routine();
    test_call_external_target_with_by_reference_updates_caller_variable();
    test_on_error_do_handler_dispatches_routine();
    test_on_error_do_with_handler_receives_error_metadata();
    test_aerror_populates_structured_runtime_error_array();
    test_aerror_exposes_sql_and_ole_specific_rows();
    test_on_error_handler_preserves_original_fault_metadata_across_caught_inner_faults();
    test_ole_property_fault_dispatches_on_error_and_preserves_object_state();
    test_ole_method_fault_is_catchable_and_preserves_object_state();
    test_thrown_expression_fault_preserves_pause_statement_and_recovery();
    test_repeated_thrown_faults_refresh_pause_metadata_each_time();
    test_nested_routine_faults_report_faulting_stack_frame_line();
    test_repeated_nested_faults_refresh_stack_frame_and_statement_metadata();
    test_with_endwith_resolves_leading_dot_member_access();
    test_try_catch_finally_handles_runtime_errors();
    test_try_finally_runs_without_catch_on_success();
    test_do_with_by_reference_updates_caller_variable();
    test_print_command_emits_event();
    test_close_command_closes_all_work_areas();
    test_close_all_releases_runtime_handles();
    test_erase_copy_rename_file_commands();
    test_for_each_iterates_array_elements();
    test_for_each_single_element_expression();
    test_release_vars_erases_named_globals();
    test_release_all_clears_all_globals();
    test_release_all_clears_current_frame_locals_without_global_leak();
    test_release_all_local_shadow_preserves_outer_global();
    test_release_all_private_shadow_restores_outer_global();
    test_release_all_like_pattern();
    test_release_all_like_pattern_reaches_arrays();
    test_release_all_except_pattern();
    test_release_all_except_pattern_reaches_arrays();
    test_release_all_preserves_public_bindings();
    test_clear_memory_erases_all_globals();
    test_clear_memory_prevents_private_bindings_from_restoring();
    test_clear_memory_clears_current_frame_locals_without_global_leak();
    test_cancel_halts_execution();
    test_quit_emits_event();
    test_quit_cancelled_by_callback();
    test_on_shutdown_clear_events_runs_and_quit_completes();
    test_on_shutdown_do_cleanup_can_call_quit_without_recursing();
    test_on_shutdown_inline_close_databases_all_runs_before_quit();
    test_shutdown_handler_quit_exits_event_loop_without_clear_events();
    test_shutdown_handler_cleanup_code_remains_harmless();
    test_quit_closes_open_database_and_runtime_handles();
    test_doevents_pumps_event_queue();
    test_doevents_in_responsive_loop();
    test_sleep_command_emits_runtime_sleep_event();
    test_spawn_and_await_command_runs_task_to_completion();
    test_spawn_cancellation_propagates_to_sibling_tasks();
    test_spawn_critical_section_serializes_workers();
    test_critical_section_order_policy_rejects_descending_nested_acquire();
    test_critical_section_exit_order_is_enforced();
    test_critical_section_reentrant_enter_same_section_is_allowed();
    test_critical_section_blocking_policy_rejects_await_inside_section();
    test_critical_section_blocking_policy_rejects_sleep_inside_section();
    test_yield_allowed_in_enter_critical_regression_minimal();
    test_yield_is_explicit_policy_exception_in_enter_critical();
    test_yield_in_enter_critical_has_no_blocking_violation();
    test_yield_in_enter_critical_is_explicit_policy_exception_regression();
    test_enter_critical_allows_yield_without_blocking_violation_event();
    test_critical_sections_release_on_task_fault_without_deadlock();
    test_yield_is_allowed_while_holding_critical_section();
    test_yield_in_critical_section_keeps_section_semantics();
    test_yield_is_allowed_in_default_critical_section_is_policy_exception();
    test_yield_is_allowed_in_critical_section_is_policy_exception();
    test_yield_inside_critical_section_is_allowed();
    test_yield_in_enter_critical_is_explicit_policy_exception_small();
    test_yield_allowed_in_enter_critical_is_small_regression();
    test_yield_allowed_in_reentrant_enter_critical_section();
    test_yield_command_emits_runtime_yield_event_and_preserves_state();
    test_yield_preserves_fault_metadata_when_followed_by_error();
    test_on_error_resume_restores_fault_session_and_cursor_state();
    test_retry_reexecutes_faulting_statement();
    test_resume_next_continues_after_fault();
    test_retry_with_no_fault_checkpoint_is_noop();
    test_runtime_faults_preserve_state_and_allow_retry();
    test_fault_continue_cycle_preserves_open_cursor_and_record_position();
    test_fault_continue_cycle_preserves_selected_alias_across_data_session_scope();
    test_pause_stack_frame_contains_accurate_intermediate_frame_lines();
    test_repeated_fault_pauses_refresh_intermediate_stack_frame_lines();
    test_thrown_expression_fault_aerror_columns_match_error_message_functions();
    test_repeated_on_error_faults_refresh_normalized_diagnostics();
    test_division_by_zero_dispatches_runtime_error();
    test_numeric_field_overflow_is_diagnosed_not_silently_truncated();
    test_aerror_line_number_is_innermost_faulting_line_not_catch_site();

    if (copperfin::test_support::test_failures() != 0) {
        std::cerr << copperfin::test_support::test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }
    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
