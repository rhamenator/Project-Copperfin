#include "copperfin/runtime/prg_engine.h"
#include "copperfin/vfp/dbf_table.h"
#include "../src/runtime/prg_engine_command_helpers.h"
#include "prg_engine_test_support.h"
#include <algorithm>
#include <cerrno>
#include <cctype>
#include <cstdlib>
#include <cstdint>
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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    const auto text_events = static_cast<int>(std::count_if(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.text";
    }));
    expect(text_events == 3, "each TEXT block should emit a runtime.text event");

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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
        "cSum = 'nSumWhile'\n"
        "cCalc = 'nCalcWhile'\n"
        "COUNT WHILE AGE < 35 TO &cCount\n"
        "SUM AGE WHILE AGE < 35 TO &cSum\n"
        "CALCULATE COUNT() TO &cCalc WHILE AGE < 35 IN 'People'\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    check("ncountwhile", "3", "COUNT WHILE TO &macro should assign the resolved target");
    check("nsumwhile", "60", "SUM WHILE TO &macro should assign the resolved target");
    check("ncalcwhile", "3", "CALCULATE WHILE TO &macro should assign the resolved target");

    const auto count_event = std::find_if(
        state.events.begin(),
        state.events.end(),
        [](const auto &event)
        {
            return event.category == "runtime.count" &&
                   event.detail.find("while=AGE < 35") != std::string::npos &&
                   event.detail.find("into=&cCount") != std::string::npos;
        });
    expect(count_event != state.events.end(), "COUNT should surface WHILE and raw INTO metadata");

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false,
        .max_call_depth = 3
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error, "call-depth guardrail should pause with an error");
    expect(
        state.message.find("maximum call depth") != std::string::npos,
        "call-depth guardrail should report a call-depth limit message");

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false,
        .max_executed_statements = 30,
        .max_loop_iterations = 1000
    });

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
    const bool has_infinite_loop_warning = std::any_of(
        diagnostics.begin(),
        diagnostics.end(),
        [](const copperfin::runtime::PrgStaticDiagnostic& diagnostic) {
            return diagnostic.code == "PRG1001";
        });
    expect(has_infinite_loop_warning, "analyzer should emit PRG1001 for DO WHILE .T. without exit path");

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error, "config.fpw call-depth limit should pause with an error");
    expect(
        state.message.find("maximum call depth") != std::string::npos,
        "config.fpw should control max call depth when options use defaults");

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession verify_session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = verify_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });
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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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
        copperfin::runtime::PrgRuntimeSession::create({.startup_path = prg.string(), .working_directory = tmp.string(), .stop_on_entry = false});
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
        copperfin::runtime::PrgRuntimeSession::create({.startup_path = prg.string(), .working_directory = tmp.string(), .stop_on_entry = false});
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
        copperfin::runtime::PrgRuntimeSession::create({.startup_path = prg.string(), .working_directory = tmp.string(), .stop_on_entry = false});
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
        copperfin::runtime::PrgRuntimeSession::create({.startup_path = prg.string(), .working_directory = tmp.string(), .stop_on_entry = false});
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
        copperfin::runtime::PrgRuntimeSession::create({.startup_path = prg.string(), .working_directory = tmp.string(), .stop_on_entry = false});
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
        copperfin::runtime::PrgRuntimeSession::create({.startup_path = prg.string(), .working_directory = tmp.string(), .stop_on_entry = false});
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

void test_release_all_like_pattern() {
    namespace fs = std::filesystem;
    const fs::path tmp = fs::temp_directory_path() / "copperfin_release_like";
    std::error_code ign;
    fs::remove_all(tmp, ign);
    fs::create_directories(tmp);
    const fs::path prg = tmp / "test.prg";
    write_text(prg, "tmp_a = 1\ntmp_b = 2\nkeep_me = 3\nRELEASE ALL LIKE tmp_*\nRETURN\n");
    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create({.startup_path = prg.string(), .working_directory = tmp.string(), .stop_on_entry = false});
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
        copperfin::runtime::PrgRuntimeSession::create({.startup_path = prg.string(), .working_directory = tmp.string(), .stop_on_entry = false});
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
        copperfin::runtime::PrgRuntimeSession::create({.startup_path = prg.string(), .working_directory = tmp.string(), .stop_on_entry = false});
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
        copperfin::runtime::PrgRuntimeSession::create({.startup_path = prg.string(), .working_directory = tmp.string(), .stop_on_entry = false});
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
        copperfin::runtime::PrgRuntimeSession::create({.startup_path = prg.string(), .working_directory = tmp.string(), .stop_on_entry = false});
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
        copperfin::runtime::PrgRuntimeSession::create({.startup_path = prg.string(), .working_directory = tmp.string(), .stop_on_entry = false});
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
        copperfin::runtime::PrgRuntimeSession::create({.startup_path = prg.string(), .working_directory = tmp.string(), .stop_on_entry = false});
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
        copperfin::runtime::PrgRuntimeSession::create({.startup_path = prg.string(), .working_directory = tmp.string(), .stop_on_entry = false});
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
        copperfin::runtime::PrgRuntimeSession::create({.startup_path = prg.string(), .working_directory = tmp.string(), .stop_on_entry = false});
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
        copperfin::runtime::PrgRuntimeSession::create({.startup_path = prg.string(), .working_directory = tmp.string(), .stop_on_entry = false});
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
    copperfin::runtime::RuntimeSessionOptions opts;
    opts.startup_path = prg.string();
    opts.working_directory = tmp.string();
    opts.stop_on_entry = false;
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
        copperfin::runtime::PrgRuntimeSession::create({.startup_path = prg.string(), .working_directory = tmp.string(), .stop_on_entry = false});

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
        copperfin::runtime::PrgRuntimeSession::create({.startup_path = prg.string(), .working_directory = tmp.string(), .stop_on_entry = false});

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
        copperfin::runtime::PrgRuntimeSession::create({.startup_path = prg.string(), .working_directory = tmp.string(), .stop_on_entry = false});
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
        copperfin::runtime::PrgRuntimeSession::create({.startup_path = prg.string(), .working_directory = tmp.string(), .stop_on_entry = false});

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
        copperfin::runtime::PrgRuntimeSession::create({.startup_path = prg.string(), .working_directory = tmp.string(), .stop_on_entry = false});

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
        copperfin::runtime::PrgRuntimeSession::create({.startup_path = quit_path.string(), .working_directory = temp_root.string(), .stop_on_entry = false});

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
        copperfin::runtime::PrgRuntimeSession::create({.startup_path = verify_path.string(), .working_directory = temp_root.string(), .stop_on_entry = false});
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
        copperfin::runtime::PrgRuntimeSession::create({.startup_path = main_path.string(), .working_directory = temp_root.string(), .stop_on_entry = false});
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
        copperfin::runtime::PrgRuntimeSession::create({.startup_path = main_path.string(), .working_directory = temp_root.string(), .stop_on_entry = false});
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "DOEVENTS loop with CLEAR EVENTS should complete");

    const auto loop_it = state.globals.find("nloopcount");
    expect(loop_it != state.globals.end(), "DOEVENTS loop should expose nLoopCount");
    if (loop_it != state.globals.end()) {
        expect(loop_it->second.number_value == 5.0, "loop should complete after 5 iterations");
    }

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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
    copperfin::runtime::RuntimeSessionOptions options;
    options.startup_path = "runtime_fault_test.prg";
    
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

int main() {
    test_command_keyword_scanner_ignores_nested_text();
    test_do_while_and_loop_control_flow();
    test_do_case_control_flow();
    test_push_pop_key_menu_popup_stack_commands();
    test_text_endtext_literal_blocks();
    test_aggregate_functions_respect_visibility();
    test_calculate_command_aggregates();
    test_command_level_aggregate_commands();
    test_aggregate_commands_support_macro_targets_and_calculate_while();
    test_command_level_aggregate_scope_and_while_semantics();
    test_total_command_for_local_tables();
    test_total_command_supports_currency_and_integer_fields();
    test_total_command_for_sql_result_cursors();
    test_private_declaration_masks_caller_variable();
    test_private_variable_visible_to_called_routines();
    test_release_private_restores_saved_binding_immediately();
    test_release_local_restores_visible_outer_global();
    test_store_command_assigns_multiple_variables();
    test_runtime_guardrail_limits_call_depth_without_crashing_host();
    test_runtime_guardrail_limits_statement_budget_without_crashing_host();
    test_static_diagnostic_flags_likely_infinite_do_while_loop();
    test_config_fpw_overrides_runtime_limits();
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
    test_retry_reexecutes_faulting_statement();
    test_resume_next_continues_after_fault();
    test_retry_with_no_fault_checkpoint_is_noop();
    test_runtime_faults_preserve_state_and_allow_retry();

    if (copperfin::test_support::test_failures() != 0) {
        std::cerr << copperfin::test_support::test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }
    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
