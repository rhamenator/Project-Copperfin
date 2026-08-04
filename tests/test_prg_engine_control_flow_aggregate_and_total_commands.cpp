// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_prg_engine_control_flow_support.h"

#include <locale>

namespace cf_test_prg_engine_control_flow {

namespace {

class comma_decimal_numpunct final : public std::numpunct<char> {
protected:
    char do_decimal_point() const override { return ','; }
    char do_thousands_sep() const override { return '.'; }
    std::string do_grouping() const override { return "\3"; }
};

class scoped_global_locale {
public:
    explicit scoped_global_locale(const std::locale& replacement)
        : previous_(std::locale::global(replacement)) {}

    ~scoped_global_locale() { std::locale::global(previous_); }

    scoped_global_locale(const scoped_global_locale&) = delete;
    scoped_global_locale& operator=(const scoped_global_locale&) = delete;

private:
    std::locale previous_;
};

}  // namespace

void test_total_numeric_formatting_ignores_global_locale() {
    const std::locale comma_locale(std::locale::classic(), new comma_decimal_numpunct());
    scoped_global_locale locale_guard(comma_locale);

    expect(
        copperfin::runtime::format_total_numeric_value(1234.5, 2U) == "1234.50",
        "TOTAL decimal formatting should remain period-decimal under a comma-decimal global locale");
    expect(
        copperfin::runtime::format_total_numeric_value(1234.5, 0U) == "1235",
        "TOTAL integral rounding should remain unchanged under a comma-decimal global locale");
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

void test_aggregate_command_errors_use_default_locale_messages() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_aggregate_command_errors";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}});

    const auto run_error_script = [&](const std::string& file_stem, const std::string& script) {
        const fs::path main_path = temp_root / (file_stem + ".prg");
        write_text(main_path, script);
        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
        return session.run(copperfin::runtime::DebugResumeAction::continue_run);
    };

    const auto missing_assignments = run_error_script("calculate_missing_assignments", "CALCULATE FOR .T.\n");
    expect(missing_assignments.reason == copperfin::runtime::DebugPauseReason::error,
        "CALCULATE without assignments should pause with an error");
    expect(
        missing_assignments.message == "CALCULATE requires one or more aggregate TO/INTO assignments",
        "CALCULATE missing-assignment error should route through the default locale catalog");

    const auto malformed_expression = run_error_script(
        "calculate_malformed_expression",
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "CALCULATE AGE TO nBad\n");
    expect(malformed_expression.reason == copperfin::runtime::DebugPauseReason::error,
        "CALCULATE with a malformed aggregate expression should pause with an error");
    expect(
        malformed_expression.message == "CALCULATE requires aggregate expressions like COUNT() or SUM(field)",
        "CALCULATE malformed-expression error should route through the default locale catalog");

    const auto count_multi_target = run_error_script(
        "count_multi_target",
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "COUNT TO nOne, nTwo\n");
    expect(count_multi_target.reason == copperfin::runtime::DebugPauseReason::error,
        "COUNT TO with multiple targets should pause with an error");
    expect(count_multi_target.message == "COUNT TO only accepts a single variable target",
        "COUNT TO multi-target error should route through the default locale catalog");

    const auto no_work_area = run_error_script(
        "sum_no_work_area",
        "SUM AGE TO nSum\n");
    expect(no_work_area.reason == copperfin::runtime::DebugPauseReason::error,
        "SUM without a selected work area should pause with an error");
    expect(no_work_area.message == "SUM requires a selected work area",
        "aggregate no-work-area error should route through the default locale catalog");

    const auto missing_target_work_area = run_error_script(
        "sum_missing_target_work_area",
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SUM AGE TO nSum IN 'MissingAlias'\n");
    expect(missing_target_work_area.reason == copperfin::runtime::DebugPauseReason::error,
        "SUM with a missing IN target should pause with an error");
    expect(missing_target_work_area.message == "SUM target work area not found",
        "aggregate target-work-area error should route through the default locale catalog");

    const auto to_array_missing_target = run_error_script(
        "sum_to_array_missing_target",
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SUM AGE TO ARRAY\n");
    expect(to_array_missing_target.reason == copperfin::runtime::DebugPauseReason::error,
        "SUM TO ARRAY without a target array name should pause with an error");
    expect(to_array_missing_target.message == "SUM TO ARRAY requires a target array name",
        "aggregate TO ARRAY missing-target error should route through the default locale catalog");

    const auto target_mismatch = run_error_script(
        "sum_target_mismatch",
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SUM AGE TO nOne, nTwo\n");
    expect(target_mismatch.reason == copperfin::runtime::DebugPauseReason::error,
        "SUM TO with too many targets should pause with an error");
    expect(target_mismatch.message == "SUM TO requires one variable per aggregate expression",
        "aggregate TO target-count mismatch should route through the default locale catalog");

    fs::remove_all(temp_root, ignored);
}

void test_aggregate_command_errors_localize_without_changing_runtime_behavior() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_aggregate_command_error_localization";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}});

    const auto run_error_script = [&](const std::string& file_stem, const std::string& script) {
        const fs::path main_path = temp_root / (file_stem + ".prg");
        write_text(main_path, script);
        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
        return session.run(copperfin::runtime::DebugResumeAction::continue_run);
    };

    set_env_value("COPPERFIN_LOCALE", "es-419", true);
    const auto spanish_missing_assignments = run_error_script("calculate_missing_assignments_es", "CALCULATE FOR .T.\n");
    expect(spanish_missing_assignments.reason == copperfin::runtime::DebugPauseReason::error,
           "#2595: es-419 CALCULATE without assignments should still pause with an error");
    expect(
        spanish_missing_assignments.message == "CALCULATE requiere una o mas asignaciones agregadas TO/INTO",
        "#2595: es-419 CALCULATE missing-assignment error should localize the prose");

    set_env_value("COPPERFIN_LOCALE", "pt-BR", true);
    const auto portuguese_malformed_expression = run_error_script(
        "calculate_malformed_expression_pt",
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "CALCULATE AGE TO nBad\n");
    expect(portuguese_malformed_expression.reason == copperfin::runtime::DebugPauseReason::error,
           "#2595: pt-BR CALCULATE malformed-expression should still pause with an error");
    expect(
        portuguese_malformed_expression.message == "CALCULATE exige expressoes agregadas como COUNT() ou SUM(field)",
        "#2595: pt-BR CALCULATE malformed-expression error should localize the prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    const auto pseudo_count_multi_target = run_error_script(
        "count_multi_target_qps",
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "COUNT TO nOne, nTwo\n");
    expect(pseudo_count_multi_target.reason == copperfin::runtime::DebugPauseReason::error,
           "#2595: qps-ploc COUNT TO with multiple targets should still pause with an error");
    expect(
        pseudo_count_multi_target.message ==
            copperfin::localization::pseudo_localize("COUNT TO only accepts a single variable target"),
        "#2595: qps-ploc COUNT TO multi-target error should resolve through the pseudo-localization transform");

    set_env_value("COPPERFIN_LOCALE", "es-419", true);
    const auto spanish_no_work_area = run_error_script("sum_no_work_area_es", "SUM AGE TO nSum\n");
    expect(spanish_no_work_area.reason == copperfin::runtime::DebugPauseReason::error,
           "#2721: es-419 SUM without a selected work area should still pause with an error");
    expect(
        spanish_no_work_area.message == "SUM requiere un area de trabajo seleccionada",
        "#2721: es-419 aggregate no-work-area error should localize the prose");

    set_env_value("COPPERFIN_LOCALE", "pt-BR", true);
    const auto portuguese_target_mismatch = run_error_script(
        "sum_target_mismatch_pt",
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SUM AGE TO nOne, nTwo\n");
    expect(portuguese_target_mismatch.reason == copperfin::runtime::DebugPauseReason::error,
           "#2721: pt-BR SUM TO target mismatch should still pause with an error");
    expect(
        portuguese_target_mismatch.message == "SUM TO exige uma variavel por cada expressao agregada",
        "#2721: pt-BR aggregate TO target-count mismatch should localize the prose");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    const auto pseudo_to_array_missing_target = run_error_script(
        "sum_to_array_missing_target_qps",
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SUM AGE TO ARRAY\n");
    expect(pseudo_to_array_missing_target.reason == copperfin::runtime::DebugPauseReason::error,
           "#2721: qps-ploc SUM TO ARRAY without a target name should still pause with an error");
    expect(
        pseudo_to_array_missing_target.message.find("[!! ") == 0U &&
            pseudo_to_array_missing_target.message.find("SUM") != std::string::npos &&
            pseudo_to_array_missing_target.message.find("TO ARRAY") != std::string::npos &&
            pseudo_to_array_missing_target.message.find("requires a target array name") == std::string::npos,
        "#2721: qps-ploc aggregate TO ARRAY missing-target error should pseudo-localize prose while preserving invariant command tokens");

    set_env_value("COPPERFIN_LOCALE", "en-US", true);

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

void test_total_command_tolerates_non_numeric_field_text() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_total_command_overflow";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "sales.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "REGION", .type = 'C', .length = 10U},
        {.name = "AMOUNT", .type = 'N', .length = 6U}
    };
    const std::vector<std::vector<std::string>> records{
        {"EAST", "10"},
        {"EAST", "******"},
        {"WEST", "8"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "overflow-marker sales DBF fixture should be created");

    const fs::path output_path = temp_root / "totals.dbf";
    const fs::path main_path = temp_root / "total_overflow.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS Sales IN 0\n"
        "TOTAL TO '" + output_path.string() + "' ON REGION FIELDS AMOUNT\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "TOTAL should tolerate a non-numeric/overflow-marker field value instead of faulting: " + state.message);

    const auto totals_result = copperfin::vfp::parse_dbf_table_from_file(output_path.string(), 10U);
    expect(totals_result.ok, "TOTAL should write a readable output DBF despite the overflow-marker row");
    if (totals_result.ok) {
        const auto east_record = std::find_if(
            totals_result.table.records.begin(),
            totals_result.table.records.end(),
            [](const auto& record) { return record.values[0].display_value == "EAST"; });
        expect(east_record != totals_result.table.records.end(), "TOTAL should still produce an EAST group total");
        if (east_record != totals_result.table.records.end()) {
            expect(east_record->values[1].display_value == "10",
                "TOTAL should sum only the parseable field values, skipping the non-numeric overflow marker");
        }
    }

    fs::remove_all(temp_root, ignored);
}

void test_aggregate_helpers_tolerate_non_numeric_field_text() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_aggregate_overflow";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "sales.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "REGION", .type = 'C', .length = 10U},
        {.name = "AMOUNT", .type = 'N', .length = 6U}
    };
    const std::vector<std::vector<std::string>> records{
        {"EAST", "10"},
        {"EAST", "******"},
        {"WEST", "8"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "aggregate overflow-marker sales DBF fixture should be created");

    const fs::path main_path = temp_root / "aggregate_overflow.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS Sales IN 0\n"
        "nSum = SUM(AMOUNT)\n"
        "nAvg = AVG(AMOUNT)\n"
        "nMin = MIN(AMOUNT)\n"
        "nMax = MAX(AMOUNT)\n"
        "CALCULATE SUM(AMOUNT) TO nCalcSum, AVG(AMOUNT) TO nCalcAvg, MIN(AMOUNT) TO nCalcMin, MAX(AMOUNT) TO nCalcMax\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "aggregate helpers should tolerate non-numeric/overflow-marker field text instead of faulting: " + state.message);

    const auto check = [&](const std::string& name, const std::string& expected) {
        const auto it = state.globals.find(name);
        if (it == state.globals.end()) {
            expect(false, name + " variable not found");
            return;
        }
        expect(copperfin::runtime::format_value(it->second) == expected,
            name + " expected '" + expected + "' got '" + copperfin::runtime::format_value(it->second) + "'");
    };

    check("nsum", "18");
    check("navg", "9");
    check("nmin", "8");
    check("nmax", "10");
    check("ncalcsum", "18");
    check("ncalcavg", "9");
    check("ncalcmin", "8");
    check("ncalcmax", "10");

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

}  // namespace cf_test_prg_engine_control_flow
