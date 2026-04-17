#include "copperfin/runtime/prg_engine.h"
#include "copperfin/vfp/dbf_table.h"
#include "prg_engine_test_support.h"
#include <algorithm>
#include <cerrno>
#include <cctype>
#include <cstdlib>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <process.h>
#include <sstream>
#include <system_error>
#include <vector>

namespace {

using namespace copperfin::test_support;

void test_breakpoints_and_stepping() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "main.prg";
    write_text(
        main_path,
        "x = 1\n"
        "DO localproc\n"
        "x = x + 1\n"
        "RETURN\n"
        "PROCEDURE localproc\n"
        "x = x + 2\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });
    session.add_breakpoint({.file_path = main_path.string(), .line = 2});

    auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::breakpoint, "continue should stop on the configured breakpoint");
    expect(state.location.line == 2U, "breakpoint should land on line 2");

    state = session.run(copperfin::runtime::DebugResumeAction::step_into);
    expect(state.reason == copperfin::runtime::DebugPauseReason::step, "step into should pause after entering the procedure");
    expect(state.location.line == 6U, "step into should land on the first line of the local procedure");

    state = session.run(copperfin::runtime::DebugResumeAction::step_out);
    expect(state.reason == copperfin::runtime::DebugPauseReason::step, "step out should pause after returning to the caller");
    expect(state.location.line == 3U, "step out should land on the caller line after the DO");

    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "continue should complete the script");
    const auto x = state.globals.find("x");
    expect(x != state.globals.end(), "script should leave x in globals");
    if (x != state.globals.end()) {
        expect(copperfin::runtime::format_value(x->second) == "4", "x should equal 4 after the procedure and caller increment");
    }

    fs::remove_all(temp_root, ignored);
}

void test_read_events_pause() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_events";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "events.prg";
    write_text(
        main_path,
        "x = 1\n"
        "READ EVENTS\n"
        "x = x + 1\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop, "READ EVENTS should pause the runtime in event-loop mode");
    expect(state.waiting_for_events, "event-loop pause should report waiting_for_events");

    fs::remove_all(temp_root, ignored);
}

void test_activate_popup_pause() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_popup";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "popup.prg";
    write_text(
        main_path,
        "ACTIVATE POPUP Shortcut\n"
        "x = 2\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop, "ACTIVATE POPUP should pause the runtime in event-loop mode");
    expect(state.waiting_for_events, "popup activation should report waiting_for_events");

    fs::remove_all(temp_root, ignored);
}

void test_dispatch_event_handler() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_dispatch";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "dispatch.prg";
    write_text(
        main_path,
        "PUBLIC x\n"
        "ACTIVATE POPUP Shortcut\n"
        "RETURN\n"
        "PROCEDURE item1\n"
        "x = 7\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop, "startup popup activation should pause in the event loop");
    expect(session.dispatch_event_handler("item1"), "dispatch_event_handler should find the target routine while waiting for events");

    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop, "dispatching an event handler should return to the event loop");
    const auto x = state.globals.find("x");
    expect(x != state.globals.end(), "dispatched routine should be able to set global variables");
    if (x != state.globals.end()) {
        expect(copperfin::runtime::format_value(x->second) == "7", "dispatched routine should update x before returning to the event loop");
    }

    fs::remove_all(temp_root, ignored);
}

void test_local_variables_in_stack_frame() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_locals";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "locals.prg";
    write_text(
        main_path,
        "DO localproc\n"
        "RETURN\n"
        "PROCEDURE localproc\n"
        "LOCAL itemCount\n"
        "itemCount = 9\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    session.add_breakpoint({.file_path = main_path.string(), .line = 5});
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::breakpoint, "local-variable test should stop on the itemCount assignment");
    expect(!state.call_stack.empty(), "local-variable test should include a stack frame");
    if (!state.call_stack.empty()) {
        const auto local = state.call_stack.front().locals.find("itemcount");
        expect(local != state.call_stack.front().locals.end(), "stack frame should expose declared LOCAL variables");
        if (local != state.call_stack.front().locals.end()) {
            expect(copperfin::runtime::format_value(local->second) == "", "LOCAL variables should exist before assignment");
        }
    }

    fs::remove_all(temp_root, ignored);
}

void test_report_form_pause() {
    namespace fs = std::filesystem;
    const fs::path report_path = R"(C:\Program Files (x86)\Microsoft Visual FoxPro 9\Samples\Solution\Reports\invoice.frx)";
    if (!fs::exists(report_path)) {
        return;
    }

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_report";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "report.prg";
    write_text(
        main_path,
        "REPORT FORM '" + report_path.string() + "' PREVIEW\n"
        "x = 2\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop, "REPORT FORM PREVIEW should pause in the event loop");
    expect(state.waiting_for_events, "report preview should report waiting_for_events");

    fs::remove_all(temp_root, ignored);
}

void test_label_form_pause() {
    namespace fs = std::filesystem;
    const fs::path label_path = R"(C:\Program Files (x86)\Microsoft Visual FoxPro 9\Samples\Solution\Reports\cust.lbx)";
    if (!fs::exists(label_path)) {
        return;
    }

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_label";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "label.prg";
    write_text(
        main_path,
        "LABEL FORM '" + label_path.string() + "' PREVIEW\n"
        "x = 2\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop, "LABEL FORM PREVIEW should pause in the event loop");
    expect(state.waiting_for_events, "label preview should report waiting_for_events");

    fs::remove_all(temp_root, ignored);
}

void test_do_form_pause() {
    namespace fs = std::filesystem;
    const fs::path form_path = R"(C:\Program Files (x86)\Microsoft Visual FoxPro 9\Wizards\Template\Books\Forms\books.scx)";
    if (!fs::exists(form_path)) {
        return;
    }

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_doform";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "doform.prg";
    write_text(
        main_path,
        "DO FORM '" + form_path.string() + "'\n"
        "x = 2\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop, "DO FORM should now enter the event loop for runnable forms");
    expect(state.waiting_for_events, "form launch should report waiting_for_events");

    fs::remove_all(temp_root, ignored);
}

void test_export_vfp_compatibility_corpus_script() {
    namespace fs = std::filesystem;
    const fs::path script_path = R"(E:\Project-Copperfin\scripts\export-vfp-compatibility-corpus.ps1)";
    const fs::path fixture_root = R"(E:\Project-Copperfin\build\compatibility_corpus_fixture)";
    const fs::path output_root = R"(E:\Project-Copperfin\build\compatibility_corpus_output)";
    const fs::path installed_root = fixture_root / "installed";
    const fs::path vfp_source_root = fixture_root / "vfpsource";
    const fs::path legacy_root = fixture_root / "legacy";
    const fs::path regression_root = fixture_root / "regression";

    std::error_code ignored;
    fs::remove_all(fixture_root, ignored);
    fs::remove_all(output_root, ignored);

    fs::create_directories(installed_root / "Samples" / "Solution" / "Reports");
    fs::create_directories(installed_root / "Wizards" / "Template" / "Books" / "Forms");
    fs::create_directories(vfp_source_root / "ReportBuilder");
    fs::create_directories(legacy_root / "Legacy");
    fs::create_directories(regression_root / "runtime");

    write_text(installed_root / "Samples" / "Solution" / "Reports" / "invoice.frx", "report fixture");
    write_text(installed_root / "Wizards" / "Template" / "Books" / "Forms" / "books.scx", "form fixture");
    write_text(vfp_source_root / "ReportBuilder" / "builder.prg", "PROCEDURE builder\nRETURN\n");
    write_text(legacy_root / "Legacy" / "sample.pjx", "project fixture");
    write_text(regression_root / "runtime" / "macro.spr", "SCREEN fixture");
    write_text(vfp_source_root / "ReportBuilder" / "ignore.txt", "not a FoxPro asset");

    std::vector<std::string> script_args = {
        "powershell",
        "-NoProfile",
        "-ExecutionPolicy",
        "Bypass",
        "-File",
        script_path.string(),
        "-OutputDirectory",
        output_root.string(),
        "-InstalledVfpRoots",
        installed_root.string(),
        "-VfpSourceRoots",
        vfp_source_root.string(),
        "-LegacyProjectRoots",
        legacy_root.string(),
        "-RegressionSampleRoots",
        regression_root.string()
    };

    std::vector<const char*> argv;
    argv.reserve(script_args.size() + 1U);
    for (const auto& arg : script_args) {
        argv.push_back(arg.c_str());
    }
    argv.push_back(nullptr);

    const intptr_t exit_code = _spawnvp(_P_WAIT, "powershell", const_cast<char* const*>(argv.data()));
    expect(exit_code != -1, "compatibility corpus exporter should launch powershell successfully");
    if (exit_code == -1) {
        std::cerr << "FAIL: powershell launch error: "
                  << std::error_code(errno, std::generic_category()).message() << "\n";
        fs::remove_all(fixture_root, ignored);
        fs::remove_all(output_root, ignored);
        return;
    }
    expect(exit_code == 0, "compatibility corpus exporter should succeed for synthetic fixture roots");

    const fs::path manifest_path = output_root / "vfp-compatibility-corpus.json";
    const fs::path summary_path = output_root / "vfp-compatibility-corpus-summary.json";
    expect(fs::exists(manifest_path), "compatibility corpus exporter should write the manifest JSON");
    expect(fs::exists(summary_path), "compatibility corpus exporter should write the summary JSON");
    if (!fs::exists(manifest_path) || !fs::exists(summary_path)) {
        fs::remove_all(fixture_root, ignored);
        fs::remove_all(output_root, ignored);
        return;
    }

    const std::string manifest = read_text(manifest_path);
    const std::string summary = read_text(summary_path);

    expect(manifest.find(R"(Samples\\Solution\\Reports\\invoice.frx)") != std::string::npos,
           "manifest should include installed VFP sample report assets");
    expect(manifest.find(R"(Wizards\\Template\\Books\\Forms\\books.scx)") != std::string::npos,
           "manifest should include installed VFP wizard form assets");
    expect(manifest.find(R"(ReportBuilder\\builder.prg)") != std::string::npos,
           "manifest should include local VFP source PRGs");
    expect(manifest.find(R"(Legacy\\sample.pjx)") != std::string::npos,
           "manifest should include legacy project assets");
    expect(manifest.find(R"(runtime\\macro.spr)") != std::string::npos,
           "manifest should include regression sample assets");
    expect(manifest.find("\"assetCategory\":  \"designer\"") != std::string::npos,
           "manifest should classify designer assets");
    expect(manifest.find("\"assetCategory\":  \"code\"") != std::string::npos,
           "manifest should classify code assets");
    expect(manifest.find("\"assetCategory\":  \"application\"") != std::string::npos,
           "manifest should classify project and app assets");
    expect(manifest.find("ignore.txt") == std::string::npos,
           "manifest should ignore unsupported file extensions");

    expect(summary.find("\"totalEntries\":  5") != std::string::npos,
           "summary should report the exported entry count");
    expect(summary.find("\"installed-vfp\":  2") != std::string::npos,
           "summary should count installed VFP assets");
    expect(summary.find("\"local-vfp-source\":  1") != std::string::npos,
           "summary should count VFP source assets");
    expect(summary.find("\"legacy-project\":  1") != std::string::npos,
           "summary should count legacy project assets");
    expect(summary.find("\"regression-sample\":  1") != std::string::npos,
           "summary should count regression sample assets");

    fs::remove_all(fixture_root, ignored);
    fs::remove_all(output_root, ignored);
}

void test_work_area_and_data_session_compatibility() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_workareas";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "workarea.prg";
    write_text(
        main_path,
        "SELECT 0\n"
        "nArea = SELECT()\n"
        "SET DATASESSION TO 3\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "work area script should complete");
    expect(state.work_area.selected == 1, "SELECT 0 should allocate and select work area 1 in a fresh session");
    expect(state.work_area.data_session == 3, "SET DATASESSION TO should update compatibility state");
    const auto area = state.globals.find("narea");
    expect(area != state.globals.end(), "SELECT() result should be available to PRG code");
    if (area != state.globals.end()) {
        expect(copperfin::runtime::format_value(area->second) == "1", "SELECT() should return the current work area");
    }

    fs::remove_all(temp_root, ignored);
}

void test_eval_macro_and_runtime_state_semantics() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_eval_macro_state";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    const fs::path new_default = temp_root / "workspace";
    fs::create_directories(new_default);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}});

    const fs::path main_path = temp_root / "eval_macro_state.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "cAliasExpr = 'People'\n"
        "cFieldExpr = 'NAME'\n"
        "cEvalExpr = 'AGE + 5'\n"
        "cNearBefore = SET('NEAR')\n"
        "SET NEAR ON\n"
        "cNearAfter = SET('NEAR')\n"
        "cDefaultBefore = SET('DEFAULT')\n"
        "SET DEFAULT TO '" + new_default.string() + "'\n"
        "cDefaultAfter = SET('DEFAULT')\n"
        "cAliasFromEval = EVAL('ALIAS()')\n"
        "cNameFromMacro = &cFieldExpr\n"
        "nEvalAge = EVAL(cEvalExpr)\n"
        "USE IN &cAliasExpr\n"
        "lUsedAfterClose = USED('People')\n"
        "nAreaAfterClose = SELECT('People')\n"
        "SET DATASESSION TO 2\n"
        "cNearSession2 = SET('NEAR')\n"
        "cDefaultSession2 = SET('DEFAULT')\n"
        "lFileSession2 = FILE('people.dbf')\n"
        "SET DATASESSION TO 1\n"
        "cNearRestored = SET('NEAR')\n"
        "cDefaultRestored = SET('DEFAULT')\n"
        "lFileRestored = FILE('people.dbf')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "eval/macro/state script should complete");

    const auto near_before = state.globals.find("cnearbefore");
    const auto near_after = state.globals.find("cnearafter");
    const auto default_before = state.globals.find("cdefaultbefore");
    const auto default_after = state.globals.find("cdefaultafter");
    const auto alias_from_eval = state.globals.find("caliasfromeval");
    const auto name_from_macro = state.globals.find("cnamefrommacro");
    const auto eval_age = state.globals.find("nevalage");
    const auto used_after_close = state.globals.find("lusedafterclose");
    const auto area_after_close = state.globals.find("nareaafterclose");
    const auto near_session2 = state.globals.find("cnearsession2");
    const auto default_session2 = state.globals.find("cdefaultsession2");
    const auto file_session2 = state.globals.find("lfilesession2");
    const auto near_restored = state.globals.find("cnearrestored");
    const auto default_restored = state.globals.find("cdefaultrestored");
    const auto file_restored = state.globals.find("lfilerestored");

    expect(near_before != state.globals.end(), "SET('NEAR') before enabling it should be captured");
    expect(near_after != state.globals.end(), "SET('NEAR') after enabling it should be captured");
    expect(default_before != state.globals.end(), "SET('DEFAULT') before change should be captured");
    expect(default_after != state.globals.end(), "SET('DEFAULT') after change should be captured");
    expect(alias_from_eval != state.globals.end(), "EVAL() should be able to evaluate runtime-state expressions");
    expect(name_from_macro != state.globals.end(), "&macro field resolution should be captured");
    expect(eval_age != state.globals.end(), "EVAL() of a stored expression should be captured");
    expect(used_after_close != state.globals.end(), "USE IN <expr> close semantics should be captured");
    expect(area_after_close != state.globals.end(), "SELECT('alias') after USE IN <expr> should be captured");
    expect(near_session2 != state.globals.end(), "SET('NEAR') in a fresh second session should be captured");
    expect(default_session2 != state.globals.end(), "SET('DEFAULT') in a fresh second session should be captured");
    expect(file_session2 != state.globals.end(), "FILE() in a fresh second session should be captured");
    expect(near_restored != state.globals.end(), "SET('NEAR') after restoring the original session should be captured");
    expect(default_restored != state.globals.end(), "SET('DEFAULT') after restoring the original session should be captured");
    expect(file_restored != state.globals.end(), "FILE() after restoring the original session should be captured");

    if (near_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(near_before->second) == "OFF", "SET('NEAR') should report OFF before it is enabled");
    }
    if (near_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(near_after->second) == "ON", "SET('NEAR') should report ON after SET NEAR ON");
    }
    if (default_before != state.globals.end()) {
        expect(
            lowercase_copy(copperfin::runtime::format_value(default_before->second)) == lowercase_copy(temp_root.string()),
            "SET('DEFAULT') should expose the startup working directory before changes");
    }
    if (default_after != state.globals.end()) {
        expect(
            lowercase_copy(copperfin::runtime::format_value(default_after->second)) == lowercase_copy(new_default.string()),
            "SET('DEFAULT') should expose the updated default directory");
    }
    if (alias_from_eval != state.globals.end()) {
        expect(copperfin::runtime::format_value(alias_from_eval->second) == "People", "EVAL('ALIAS()') should evaluate in the current runtime context");
    }
    if (name_from_macro != state.globals.end()) {
        expect(copperfin::runtime::format_value(name_from_macro->second) == "ALPHA", "&macro should substitute a stored field name inside expressions");
    }
    if (eval_age != state.globals.end()) {
        expect(copperfin::runtime::format_value(eval_age->second) == "15", "EVAL() should evaluate stored arithmetic expressions against the current record");
    }
    if (used_after_close != state.globals.end()) {
        expect(copperfin::runtime::format_value(used_after_close->second) == "false", "USE IN <expr> should close the targeted alias");
    }
    if (area_after_close != state.globals.end()) {
        expect(copperfin::runtime::format_value(area_after_close->second) == "0", "closing an alias through USE IN <expr> should clear SELECT('alias') lookup");
    }
    if (near_session2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(near_session2->second) == "OFF", "SET() state should stay isolated in a fresh data session");
    }
    if (default_session2 != state.globals.end()) {
        expect(
            lowercase_copy(copperfin::runtime::format_value(default_session2->second)) == lowercase_copy(temp_root.string()),
            "a fresh data session should start with the startup working directory as SET('DEFAULT')");
    }
    if (file_session2 != state.globals.end()) {
        expect(
            copperfin::runtime::format_value(file_session2->second) == "true",
            "relative FILE() checks in a fresh data session should resolve against that session's default directory");
    }
    if (near_restored != state.globals.end()) {
        expect(copperfin::runtime::format_value(near_restored->second) == "ON", "restoring the original data session should restore its SET() state");
    }
    if (default_restored != state.globals.end()) {
        expect(
            lowercase_copy(copperfin::runtime::format_value(default_restored->second)) == lowercase_copy(new_default.string()),
            "restoring the original data session should restore its changed SET('DEFAULT') value");
    }
    if (file_restored != state.globals.end()) {
        expect(
            copperfin::runtime::format_value(file_restored->second) == "false",
            "relative FILE() checks after restoring the original session should use that session's default directory");
    }

    fs::remove_all(temp_root, ignored);
}

void test_sql_and_ole_compatibility_functions() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sql_ole";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "interop.prg";
    write_text(
        main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "nExec = SQLEXEC(nConn, 'select * from customers')\n"
        "nDisc = SQLDISCONNECT(nConn)\n"
        "oExcel = CREATEOBJECT('Excel.Application')\n"
        "oRunning = GETOBJECT('Word.Application')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "interop script should complete");
    const auto conn = state.globals.find("nconn");
    expect(conn != state.globals.end(), "SQLCONNECT should return a handle");
    if (conn != state.globals.end()) {
        expect(copperfin::runtime::format_value(conn->second) == "1", "first SQLCONNECT handle should be 1");
    }
    const auto exec = state.globals.find("nexec");
    expect(exec != state.globals.end(), "SQLEXEC should return a result code");
    if (exec != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec->second) == "1", "SQLEXEC should report success for a known handle");
    }
    const auto disc = state.globals.find("ndisc");
    expect(disc != state.globals.end(), "SQLDISCONNECT should return a result code");
    if (disc != state.globals.end()) {
        expect(copperfin::runtime::format_value(disc->second) == "1", "SQLDISCONNECT should report success for a known handle");
    }
    expect(state.sql_connections.empty(), "SQLDISCONNECT should clear tracked SQL connections");
    expect(state.ole_objects.size() == 2U, "CREATEOBJECT and GETOBJECT should register OLE compatibility objects");
    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "sql.connect"; }),
        "SQLCONNECT should emit a sql.connect event");
    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "ole.createobject"; }),
        "CREATEOBJECT should emit an ole.createobject event");

    fs::remove_all(temp_root, ignored);
}
void test_sql_pass_through_rows_affected_and_provider_hint() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sql_rows";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "sql_rows.prg";
    write_text(
        main_path,
        "nConn = SQLCONNECT('Driver=ODBC Driver 18 for SQL Server;Server=Northwind')\n"
        "nInsert = SQLEXEC(nConn, 'insert into customers values (1)')\n"
        "nUpdate = SQLEXEC(nConn, 'update customers set id = 2 where id = 1')\n"
        "nDelete = SQLEXEC(nConn, 'delete from customers where id = 2')\n"
        "nRows = SQLROWCOUNT(nConn)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SQL pass-through DML script should complete");

    const auto rows = state.globals.find("nrows");
    expect(rows != state.globals.end(), "SQLROWCOUNT should expose rows affected for the latest SQLEXEC DML command");
    if (rows != state.globals.end()) {
        expect(copperfin::runtime::format_value(rows->second) == "1", "SQLROWCOUNT should return the last DML rows-affected value");
    }

    expect(!state.sql_connections.empty(), "SQL script should keep connection metadata while connected");
    if (!state.sql_connections.empty()) {
        expect(state.sql_connections.front().provider == "odbc", "SQLCONNECT target hints should classify ODBC provider metadata");
        expect(state.sql_connections.front().last_result_count == 1U, "connection state should retain the latest SQLEXEC rows-affected count");
    }

    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "sql.rows";
    }), "SQL DML execution should emit sql.rows runtime events");

    fs::remove_all(temp_root, ignored);
}

void test_sql_prepare_and_connection_properties() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sql_prepare";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "sql_prepare.prg";
    write_text(
        main_path,
        "nConn = SQLCONNECT('Provider=SQLOLEDB;Data Source=Northwind')\n"
        "nSetTimeout = SQLSETPROP(nConn, 'QueryTimeout', 45)\n"
        "nPrepare = SQLPREPARE(nConn, 'select * from customers')\n"
        "nExecPrepared = SQLEXEC(nConn)\n"
        "nRowsPrepared = SQLROWCOUNT(nConn)\n"
        "cProvider = SQLGETPROP(nConn, 'Provider')\n"
        "nTimeout = SQLGETPROP(nConn, 'QueryTimeout')\n"
        "cPrepared = SQLGETPROP(nConn, 'PreparedCommand')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SQL prepare/property script should complete");

    const auto prepare = state.globals.find("nprepare");
    const auto exec_prepared = state.globals.find("nexecprepared");
    const auto rows_prepared = state.globals.find("nrowsprepared");
    const auto provider = state.globals.find("cprovider");
    const auto timeout = state.globals.find("ntimeout");
    const auto prepared_text = state.globals.find("cprepared");

    expect(prepare != state.globals.end(), "SQLPREPARE should return a status code");
    expect(exec_prepared != state.globals.end(), "SQLEXEC(handle) should execute prepared SQL");
    expect(rows_prepared != state.globals.end(), "SQLROWCOUNT should report prepared SELECT row count");
    expect(provider != state.globals.end(), "SQLGETPROP should return provider metadata");
    expect(timeout != state.globals.end(), "SQLSETPROP/SQLGETPROP should round-trip numeric timeout metadata");
    expect(prepared_text != state.globals.end(), "SQLGETPROP should return prepared command text");

    if (prepare != state.globals.end()) {
        expect(copperfin::runtime::format_value(prepare->second) == "1", "SQLPREPARE should report success for known handles");
    }
    if (exec_prepared != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec_prepared->second) == "1", "SQLEXEC(handle) should execute prepared statements successfully");
    }
    if (rows_prepared != state.globals.end()) {
        expect(copperfin::runtime::format_value(rows_prepared->second) == "3", "SQLROWCOUNT should expose prepared SELECT result cardinality");
    }
    if (provider != state.globals.end()) {
        expect(copperfin::runtime::format_value(provider->second) == "oledb", "provider hinting should classify Provider= connect strings as OLE DB");
    }
    if (timeout != state.globals.end()) {
        expect(copperfin::runtime::format_value(timeout->second) == "45", "SQLSETPROP should persist query timeout metadata");
    }
    if (prepared_text != state.globals.end()) {
        expect(copperfin::runtime::format_value(prepared_text->second) == "select * from customers", "prepared SQL text should be retrievable through SQLGETPROP");
    }

    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "sql.prepare";
    }), "SQLPREPARE should emit sql.prepare events");

    fs::remove_all(temp_root, ignored);
}

void test_use_and_data_session_isolation() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_tables";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_simple_dbf(table_path, {"ALPHA", "BRAVO"});

    const fs::path main_path = temp_root / "tables.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "cAlias1 = ALIAS()\n"
        "nCount1 = RECCOUNT()\n"
        "nRec1 = RECNO()\n"
        "lEof1 = EOF()\n"
        "lBof1 = BOF()\n"
        "SELECT People\n"
        "SET DATASESSION TO 2\n"
        "cAlias2 = ALIAS()\n"
        "nCount2 = RECCOUNT()\n"
        "USE '" + table_path.string() + "' ALIAS SessionTwo IN 0\n"
        "cAlias3 = ALIAS()\n"
        "SET DATASESSION TO 1\n"
        "cAlias4 = ALIAS()\n"
        "USE IN People\n"
        "cAlias5 = ALIAS()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "table/data-session script should complete");
    expect(state.work_area.data_session == 1, "runtime should finish back in data session 1");
    expect(state.work_area.aliases.empty(), "USE IN People should close the session-1 cursor");

    const auto alias1 = state.globals.find("calias1");
    const auto count1 = state.globals.find("ncount1");
    const auto rec1 = state.globals.find("nrec1");
    const auto eof1 = state.globals.find("leof1");
    const auto bof1 = state.globals.find("lbof1");
    const auto alias2 = state.globals.find("calias2");
    const auto count2 = state.globals.find("ncount2");
    const auto alias3 = state.globals.find("calias3");
    const auto alias4 = state.globals.find("calias4");
    const auto alias5 = state.globals.find("calias5");

    expect(alias1 != state.globals.end(), "ALIAS() after USE should be captured");
    expect(count1 != state.globals.end(), "RECCOUNT() after USE should be captured");
    expect(rec1 != state.globals.end(), "RECNO() after USE should be captured");
    expect(eof1 != state.globals.end(), "EOF() after USE should be captured");
    expect(bof1 != state.globals.end(), "BOF() after USE should be captured");
    expect(alias2 != state.globals.end(), "ALIAS() in a fresh second data session should be captured");
    expect(count2 != state.globals.end(), "RECCOUNT() in a fresh second data session should be captured");
    expect(alias3 != state.globals.end(), "ALIAS() after opening a second-session cursor should be captured");
    expect(alias4 != state.globals.end(), "ALIAS() after restoring session 1 should be captured");
    expect(alias5 != state.globals.end(), "ALIAS() after USE IN should be captured");

    if (alias1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(alias1->second) == "People", "USE ... ALIAS should establish the alias in the selected work area");
    }
    if (count1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(count1->second) == "2", "RECCOUNT() should reflect opened DBF rows");
    }
    if (rec1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(rec1->second) == "1", "RECNO() should start at the first record");
    }
    if (eof1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(eof1->second) == "false", "EOF() should be false immediately after opening a non-empty table");
    }
    if (bof1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(bof1->second) == "false", "BOF() should be false immediately after opening a non-empty table");
    }
    if (alias2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(alias2->second).empty(), "switching to a fresh data session should isolate work-area aliases");
    }
    if (count2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(count2->second) == "0", "RECCOUNT() in a fresh data session should be zero");
    }
    if (alias3 != state.globals.end()) {
        expect(copperfin::runtime::format_value(alias3->second) == "SessionTwo", "session 2 should maintain its own alias set");
    }
    if (alias4 != state.globals.end()) {
        expect(copperfin::runtime::format_value(alias4->second) == "People", "restoring data session 1 should restore its selected alias");
    }
    if (alias5 != state.globals.end()) {
        expect(copperfin::runtime::format_value(alias5->second).empty(), "USE IN alias should close the targeted work area");
    }

    fs::remove_all(temp_root, ignored);
}
void test_report_form_to_file_renders_without_event_loop_pause() {
    namespace fs = std::filesystem;
    const fs::path report_path = R"(C:\Program Files (x86)\Microsoft Visual FoxPro 9\Samples\Solution\Reports\invoice.frx)";
    if (!fs::exists(report_path)) {
        return;
    }

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_report_render";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path output_path = temp_root / "invoice_render.txt";
    const fs::path main_path = temp_root / "report_render.prg";
    write_text(
        main_path,
        "REPORT FORM '" + report_path.string() + "' TO FILE '" + output_path.string() + "'\n"
        "x = 2\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "REPORT FORM TO FILE should complete without entering a preview event loop");
    expect(!state.waiting_for_events, "REPORT FORM TO FILE should not leave the runtime waiting_for_events");
    expect(fs::exists(output_path), "REPORT FORM TO FILE should materialize an output artifact");

    const auto x_value = state.globals.find("x");
    expect(x_value != state.globals.end(), "statements after REPORT FORM TO FILE should continue executing");
    if (x_value != state.globals.end()) {
        expect(copperfin::runtime::format_value(x_value->second) == "2", "REPORT FORM TO FILE should not block follow-on statements");
    }

    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "report.render";
    }), "REPORT FORM TO FILE should emit a report.render event");

    fs::remove_all(temp_root, ignored);
}

void test_label_form_to_file_renders_without_event_loop_pause() {
    namespace fs = std::filesystem;
    const fs::path label_path = R"(C:\Program Files (x86)\Microsoft Visual FoxPro 9\Samples\Solution\Reports\cust.lbx)";
    if (!fs::exists(label_path)) {
        return;
    }

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_label_render";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path output_path = temp_root / "cust_render.txt";
    const fs::path main_path = temp_root / "label_render.prg";
    write_text(
        main_path,
        "LABEL FORM '" + label_path.string() + "' TO FILE '" + output_path.string() + "'\n"
        "x = 2\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "LABEL FORM TO FILE should complete without entering a preview event loop");
    expect(!state.waiting_for_events, "LABEL FORM TO FILE should not leave the runtime waiting_for_events");
    expect(fs::exists(output_path), "LABEL FORM TO FILE should materialize an output artifact");

    const auto x_value = state.globals.find("x");
    expect(x_value != state.globals.end(), "statements after LABEL FORM TO FILE should continue executing");
    if (x_value != state.globals.end()) {
        expect(copperfin::runtime::format_value(x_value->second) == "2", "LABEL FORM TO FILE should not block follow-on statements");
    }

    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "label.render";
    }), "LABEL FORM TO FILE should emit a label.render event");

    fs::remove_all(temp_root, ignored);
}

void test_cross_session_alias_and_work_area_isolation() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_cross_session_isolation";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_simple_dbf(table_path, {"ALPHA", "BRAVO"});

    const fs::path main_path = temp_root / "cross_session_isolation.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "nS1PeopleArea = SELECT('People')\n"
        "lS1PeopleUsedBefore = USED('People')\n"
        "SET DATASESSION TO 2\n"
        "nS2PeopleLookupBefore = SELECT('People')\n"
        "USE '" + table_path.string() + "' ALIAS SessionTwoPeople IN 0\n"
        "nS2PeopleArea = SELECT('SessionTwoPeople')\n"
        "lS2PeopleUsedBefore = USED('SessionTwoPeople')\n"
        "SET DATASESSION TO 1\n"
        "USE IN 1\n"
        "lS1PeopleUsedAfterClose = USED('People')\n"
        "nS1PeopleLookupAfterClose = SELECT('People')\n"
        "SET DATASESSION TO 2\n"
        "lS2PeopleStillUsed = USED('SessionTwoPeople')\n"
        "nS2PeopleAreaAfterReturn = SELECT('SessionTwoPeople')\n"
        "cS2AliasAfterReturn = ALIAS()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "cross-session alias/work-area isolation script should complete");
    expect(state.work_area.data_session == 2, "script should finish in data session 2");

    const auto s1_area = state.globals.find("ns1peoplearea");
    const auto s1_used_before = state.globals.find("ls1peopleusedbefore");
    const auto s2_lookup_before = state.globals.find("ns2peoplelookupbefore");
    const auto s2_area = state.globals.find("ns2peoplearea");
    const auto s2_used_before = state.globals.find("ls2peopleusedbefore");
    const auto s1_used_after_close = state.globals.find("ls1peopleusedafterclose");
    const auto s1_lookup_after_close = state.globals.find("ns1peoplelookupafterclose");
    const auto s2_still_used = state.globals.find("ls2peoplestillused");
    const auto s2_area_after_return = state.globals.find("ns2peopleareaafterreturn");
    const auto s2_alias_after_return = state.globals.find("cs2aliasafterreturn");

    expect(s1_area != state.globals.end(), "session-1 alias area should be captured");
    expect(s1_used_before != state.globals.end(), "session-1 USED() before close should be captured");
    expect(s2_lookup_before != state.globals.end(), "session-2 lookup for session-1 alias should be captured");
    expect(s2_area != state.globals.end(), "session-2 alias area should be captured");
    expect(s2_used_before != state.globals.end(), "session-2 USED() before returning should be captured");
    expect(s1_used_after_close != state.globals.end(), "session-1 USED() after close should be captured");
    expect(s1_lookup_after_close != state.globals.end(), "session-1 alias lookup after close should be captured");
    expect(s2_still_used != state.globals.end(), "session-2 USED() after returning should be captured");
    expect(s2_area_after_return != state.globals.end(), "session-2 alias area after return should be captured");
    expect(s2_alias_after_return != state.globals.end(), "session-2 selected alias after return should be captured");

    if (s1_area != state.globals.end()) {
        expect(copperfin::runtime::format_value(s1_area->second) == "1", "session 1 should open People in work area 1");
    }
    if (s1_used_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(s1_used_before->second) == "true", "session 1 should report People as open");
    }
    if (s2_lookup_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(s2_lookup_before->second) == "0", "session 2 should not resolve aliases from session 1");
    }
    if (s2_area != state.globals.end()) {
        expect(copperfin::runtime::format_value(s2_area->second) == "1", "session 2 should reuse its own work area 1 independently");
    }
    if (s2_used_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(s2_used_before->second) == "true", "session 2 should report its own alias as open");
    }
    if (s1_used_after_close != state.globals.end()) {
        expect(copperfin::runtime::format_value(s1_used_after_close->second) == "false", "USE IN 1 in session 1 should close only session-1 area 1");
    }
    if (s1_lookup_after_close != state.globals.end()) {
        expect(copperfin::runtime::format_value(s1_lookup_after_close->second) == "0", "session-1 alias lookup should clear after close");
    }
    if (s2_still_used != state.globals.end()) {
        expect(copperfin::runtime::format_value(s2_still_used->second) == "true", "closing session-1 area 1 should not affect session-2 area 1");
    }
    if (s2_area_after_return != state.globals.end()) {
        expect(copperfin::runtime::format_value(s2_area_after_return->second) == "1", "session-2 alias lookup should still resolve to session-2 area 1");
    }
    if (s2_alias_after_return != state.globals.end()) {
        expect(copperfin::runtime::format_value(s2_alias_after_return->second) == "SessionTwoPeople", "session-2 selected alias should still be SessionTwoPeople");
    }

    fs::remove_all(temp_root, ignored);
}

void test_use_in_existing_alias_reuses_target_work_area() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_use_in_existing_alias";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path people_path = temp_root / "people.dbf";
    const fs::path cities_path = temp_root / "cities.dbf";
    write_simple_dbf(people_path, {"ALPHA", "BRAVO"});
    write_simple_dbf(cities_path, {"OSLO", "TOKYO"});

    const fs::path main_path = temp_root / "use_in_existing_alias.prg";
    write_text(
        main_path,
        "USE '" + people_path.string() + "' ALIAS People IN 0\n"
        "nAreaBefore = SELECT()\n"
        "cAliasBefore = ALIAS()\n"
        "USE '" + cities_path.string() + "' ALIAS Cities IN People\n"
        "nAreaAfter = SELECT()\n"
        "cAliasAfter = ALIAS()\n"
        "cTopName = NAME\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "USE ... IN existing alias script should complete");
    expect(state.work_area.selected == 1, "USE ... IN alias should keep the targeted work area selected");
    expect(state.work_area.aliases.size() == 1U, "USE ... IN alias should replace the target work area instead of allocating a new one");

    const auto area_before = state.globals.find("nareabefore");
    const auto alias_before = state.globals.find("caliasbefore");
    const auto area_after = state.globals.find("nareaafter");
    const auto alias_after = state.globals.find("caliasafter");
    const auto top_name = state.globals.find("ctopname");

    expect(area_before != state.globals.end(), "initial SELECT() should be captured");
    expect(alias_before != state.globals.end(), "initial ALIAS() should be captured");
    expect(area_after != state.globals.end(), "SELECT() after USE ... IN alias should be captured");
    expect(alias_after != state.globals.end(), "ALIAS() after USE ... IN alias should be captured");
    expect(top_name != state.globals.end(), "replacement cursor fields should be available after USE ... IN alias");

    if (area_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(area_before->second) == "1", "initial USE IN 0 should allocate work area 1");
    }
    if (alias_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(alias_before->second) == "People", "initial alias should be visible before replacement");
    }
    if (area_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(area_after->second) == "1", "USE ... IN alias should reuse the alias target's work area");
    }
    if (alias_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(alias_after->second) == "Cities", "USE ... IN alias should replace the target work area's alias");
    }
    if (top_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(top_name->second) == "OSLO", "USE ... IN alias should expose the replacement table's current record");
    }

    fs::remove_all(temp_root, ignored);
}

void test_use_in_nonselected_alias_preserves_selected_work_area() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_use_in_nonselected_alias";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path people_path = temp_root / "people.dbf";
    const fs::path cities_path = temp_root / "cities.dbf";
    const fs::path orders_path = temp_root / "orders.dbf";
    write_simple_dbf(people_path, {"ALPHA", "BRAVO"});
    write_simple_dbf(cities_path, {"OSLO", "TOKYO"});
    write_simple_dbf(orders_path, {"ORD100", "ORD200"});

    const fs::path main_path = temp_root / "use_in_nonselected_alias.prg";
    write_text(
        main_path,
        "USE '" + people_path.string() + "' ALIAS People IN 0\n"
        "USE '" + cities_path.string() + "' ALIAS Cities IN 0\n"
        "SELECT People\n"
        "nSelectedBefore = SELECT()\n"
        "cAliasBefore = ALIAS()\n"
        "USE '" + orders_path.string() + "' ALIAS Orders IN Cities\n"
        "nSelectedAfterReplace = SELECT()\n"
        "cAliasAfterReplace = ALIAS()\n"
        "nOrdersArea = SELECT('Orders')\n"
        "cOrdersDbf = DBF('Orders')\n"
        "USE IN Orders\n"
        "nSelectedAfterClose = SELECT()\n"
        "cAliasAfterClose = ALIAS()\n"
        "lOrdersOpenAfterClose = USED('Orders')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "USE ... IN non-selected alias script should complete");
    expect(state.work_area.selected == 1, "replacing and closing a non-selected alias should preserve the selected work area");
    expect(state.work_area.aliases.size() == 1U, "closing the non-selected replacement alias should leave only the selected alias open");

    const auto selected_before = state.globals.find("nselectedbefore");
    const auto alias_before = state.globals.find("caliasbefore");
    const auto selected_after_replace = state.globals.find("nselectedafterreplace");
    const auto alias_after_replace = state.globals.find("caliasafterreplace");
    const auto orders_area = state.globals.find("nordersarea");
    const auto orders_dbf = state.globals.find("cordersdbf");
    const auto selected_after_close = state.globals.find("nselectedafterclose");
    const auto alias_after_close = state.globals.find("caliasafterclose");
    const auto orders_open_after_close = state.globals.find("lordersopenafterclose");

    expect(selected_before != state.globals.end(), "initial selected work area should be captured");
    expect(alias_before != state.globals.end(), "initial selected alias should be captured");
    expect(selected_after_replace != state.globals.end(), "selected work area after non-selected replacement should be captured");
    expect(alias_after_replace != state.globals.end(), "selected alias after non-selected replacement should be captured");
    expect(orders_area != state.globals.end(), "replacement alias work area should be discoverable");
    expect(orders_dbf != state.globals.end(), "replacement alias DBF identity should be captured");
    expect(selected_after_close != state.globals.end(), "selected work area after closing the non-selected alias should be captured");
    expect(alias_after_close != state.globals.end(), "selected alias after closing the non-selected alias should be captured");
    expect(orders_open_after_close != state.globals.end(), "USED('Orders') after close should be captured");

    if (selected_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(selected_before->second) == "1", "People should remain selected before the non-selected replacement");
    }
    if (alias_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(alias_before->second) == "People", "People should be the selected alias before replacement");
    }
    if (selected_after_replace != state.globals.end()) {
        expect(copperfin::runtime::format_value(selected_after_replace->second) == "1", "USE ... IN a non-selected alias should preserve the selected work area");
    }
    if (alias_after_replace != state.globals.end()) {
        expect(copperfin::runtime::format_value(alias_after_replace->second) == "People", "USE ... IN a non-selected alias should preserve the selected alias");
    }
    if (orders_area != state.globals.end()) {
        expect(copperfin::runtime::format_value(orders_area->second) == "2", "the replacement alias should still reuse the targeted non-selected work area");
    }
    if (orders_dbf != state.globals.end()) {
        expect(copperfin::runtime::format_value(orders_dbf->second) == orders_path.lexically_normal().string(), "the replacement alias should point at the new table");
    }
    if (selected_after_close != state.globals.end()) {
        expect(copperfin::runtime::format_value(selected_after_close->second) == "1", "closing a non-selected alias should preserve the selected work area");
    }
    if (alias_after_close != state.globals.end()) {
        expect(copperfin::runtime::format_value(alias_after_close->second) == "People", "closing a non-selected alias should preserve the selected alias");
    }
    if (orders_open_after_close != state.globals.end()) {
        expect(copperfin::runtime::format_value(orders_open_after_close->second) == "false", "closing the replacement alias should clear its USED() state");
    }

    fs::remove_all(temp_root, ignored);
}

void test_plain_use_reuses_current_selected_work_area() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_plain_use_current_area";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path people_path = temp_root / "people.dbf";
    const fs::path cities_path = temp_root / "cities.dbf";
    write_simple_dbf(people_path, {"ALPHA", "BRAVO"});
    write_simple_dbf(cities_path, {"OSLO", "TOKYO"});

    const fs::path main_path = temp_root / "plain_use_current_area.prg";
    write_text(
        main_path,
        "SELECT 0\n"
        "nAreaEmpty = SELECT()\n"
        "USE '" + people_path.string() + "' ALIAS People\n"
        "nAreaAfterFirstUse = SELECT()\n"
        "cAliasAfterFirstUse = ALIAS()\n"
        "USE '" + cities_path.string() + "' ALIAS Cities\n"
        "nAreaAfterReplace = SELECT()\n"
        "cAliasAfterReplace = ALIAS()\n"
        "cDbfAfterReplace = DBF()\n"
        "nPeopleAreaAfterReplace = SELECT('People')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "plain USE current-area script should complete");
    expect(state.work_area.selected == 1, "plain USE should keep using the current selected work area");
    expect(state.work_area.aliases.size() == 1U, "plain USE replacement should reuse the selected work area instead of allocating another one");

    const auto area_empty = state.globals.find("nareaempty");
    const auto area_after_first_use = state.globals.find("nareaafterfirstuse");
    const auto alias_after_first_use = state.globals.find("caliasafterfirstuse");
    const auto area_after_replace = state.globals.find("nareaafterreplace");
    const auto alias_after_replace = state.globals.find("caliasafterreplace");
    const auto dbf_after_replace = state.globals.find("cdbfafterreplace");
    const auto people_area_after_replace = state.globals.find("npeopleareaafterreplace");

    expect(area_empty != state.globals.end(), "selected empty area should be captured");
    expect(area_after_first_use != state.globals.end(), "plain USE on an empty selected area should be captured");
    expect(alias_after_first_use != state.globals.end(), "alias after the first plain USE should be captured");
    expect(area_after_replace != state.globals.end(), "plain USE replacement area should be captured");
    expect(alias_after_replace != state.globals.end(), "alias after plain USE replacement should be captured");
    expect(dbf_after_replace != state.globals.end(), "DBF() after plain USE replacement should be captured");
    expect(people_area_after_replace != state.globals.end(), "SELECT('People') after plain USE replacement should be captured");

    if (area_empty != state.globals.end()) {
        expect(copperfin::runtime::format_value(area_empty->second) == "1", "SELECT 0 should choose work area 1 in a fresh session");
    }
    if (area_after_first_use != state.globals.end()) {
        expect(copperfin::runtime::format_value(area_after_first_use->second) == "1", "plain USE should fill the currently selected empty work area");
    }
    if (alias_after_first_use != state.globals.end()) {
        expect(copperfin::runtime::format_value(alias_after_first_use->second) == "People", "plain USE should establish its alias in the current work area");
    }
    if (area_after_replace != state.globals.end()) {
        expect(copperfin::runtime::format_value(area_after_replace->second) == "1", "plain USE replacement should stay in the selected work area");
    }
    if (alias_after_replace != state.globals.end()) {
        expect(copperfin::runtime::format_value(alias_after_replace->second) == "Cities", "plain USE replacement should swap the alias in place");
    }
    if (dbf_after_replace != state.globals.end()) {
        expect(copperfin::runtime::format_value(dbf_after_replace->second) == cities_path.lexically_normal().string(), "plain USE replacement should point at the replacement table");
    }
    if (people_area_after_replace != state.globals.end()) {
        expect(copperfin::runtime::format_value(people_area_after_replace->second) == "0", "plain USE replacement should remove the replaced alias from SELECT('alias') lookup");
    }

    fs::remove_all(temp_root, ignored);
}

void test_select_and_use_in_designator_expressions() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_designator_expressions";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path people_path = temp_root / "people.dbf";
    const fs::path cities_path = temp_root / "cities.dbf";
    const fs::path orders_path = temp_root / "orders.dbf";
    write_simple_dbf(people_path, {"ALPHA", "BRAVO"});
    write_simple_dbf(cities_path, {"OSLO", "ROME"});
    write_simple_dbf(orders_path, {"ORDER1", "ORDER2"});

    const fs::path main_path = temp_root / "designator_expressions.prg";
    write_text(
        main_path,
        "USE '" + people_path.string() + "' ALIAS People IN 0\n"
        "USE '" + cities_path.string() + "' ALIAS Cities IN 0\n"
        "nTargetArea = 1\n"
        "SELECT nTargetArea\n"
        "nSelectedNumeric = SELECT()\n"
        "cSelectedNumericAlias = ALIAS()\n"
        "cTargetAlias = 'Cities'\n"
        "SELECT cTargetAlias\n"
        "nSelectedAlias = SELECT()\n"
        "cSelectedAlias = ALIAS()\n"
        "USE '" + orders_path.string() + "' ALIAS Orders IN cTargetAlias\n"
        "nOrdersArea = SELECT('Orders')\n"
        "nCitiesArea = SELECT('Cities')\n"
        "nSelectedAfterUse = SELECT()\n"
        "cSelectedAfterUse = ALIAS()\n"
        "GO TOP\n"
        "cTopAfterUse = NAME\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "expression-based SELECT and USE ... IN script should complete");

    const auto selected_numeric = state.globals.find("nselectednumeric");
    const auto selected_numeric_alias = state.globals.find("cselectednumericalias");
    const auto selected_alias = state.globals.find("nselectedalias");
    const auto selected_alias_name = state.globals.find("cselectedalias");
    const auto orders_area = state.globals.find("nordersarea");
    const auto cities_area = state.globals.find("ncitiesarea");
    const auto selected_after_use = state.globals.find("nselectedafteruse");
    const auto selected_after_use_name = state.globals.find("cselectedafteruse");
    const auto top_after_use = state.globals.find("ctopafteruse");

    expect(selected_numeric != state.globals.end(), "SELECT nTargetArea should expose the selected area");
    expect(selected_numeric_alias != state.globals.end(), "SELECT nTargetArea should expose the selected alias");
    expect(selected_alias != state.globals.end(), "SELECT cTargetAlias should expose the selected area");
    expect(selected_alias_name != state.globals.end(), "SELECT cTargetAlias should expose the selected alias");
    expect(orders_area != state.globals.end(), "USE ... IN cTargetAlias should expose the replacement alias area");
    expect(cities_area != state.globals.end(), "USE ... IN cTargetAlias should clear the replaced alias lookup");
    expect(selected_after_use != state.globals.end(), "USE ... IN cTargetAlias should preserve the selected work area");
    expect(selected_after_use_name != state.globals.end(), "USE ... IN cTargetAlias should select the replacement alias");
    expect(top_after_use != state.globals.end(), "USE ... IN cTargetAlias should expose the replacement table's current record");

    if (selected_numeric != state.globals.end()) {
        expect(copperfin::runtime::format_value(selected_numeric->second) == "1", "SELECT nTargetArea should resolve numeric expression targets");
    }
    if (selected_numeric_alias != state.globals.end()) {
        expect(copperfin::runtime::format_value(selected_numeric_alias->second) == "People", "SELECT nTargetArea should select the numeric target area");
    }
    if (selected_alias != state.globals.end()) {
        expect(copperfin::runtime::format_value(selected_alias->second) == "2", "SELECT cTargetAlias should resolve alias expression targets");
    }
    if (selected_alias_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(selected_alias_name->second) == "Cities", "SELECT cTargetAlias should select the targeted alias");
    }
    if (orders_area != state.globals.end()) {
        expect(copperfin::runtime::format_value(orders_area->second) == "2", "USE ... IN cTargetAlias should reuse the targeted alias work area");
    }
    if (cities_area != state.globals.end()) {
        expect(copperfin::runtime::format_value(cities_area->second) == "0", "USE ... IN cTargetAlias should remove the replaced alias");
    }
    if (selected_after_use != state.globals.end()) {
        expect(copperfin::runtime::format_value(selected_after_use->second) == "2", "USE ... IN cTargetAlias should keep the targeted work area selected");
    }
    if (selected_after_use_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(selected_after_use_name->second) == "Orders", "USE ... IN cTargetAlias should select the replacement alias");
    }
    if (top_after_use != state.globals.end()) {
        expect(copperfin::runtime::format_value(top_after_use->second) == "ORDER1", "USE ... IN cTargetAlias should expose the replacement table rows");
    }

    fs::remove_all(temp_root, ignored);
}

void test_expression_driven_in_targeting_across_local_data_commands() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_in_target_commands";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path people_path = temp_root / "people.dbf";
    const fs::path cities_path = temp_root / "cities.dbf";
    const fs::path orders_path = temp_root / "orders.dbf";
    const fs::path people_cdx_path = temp_root / "people.cdx";
    write_people_dbf(people_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}});
    write_people_dbf(cities_path, {{"OSLO", 1}, {"ROME", 2}});
    write_people_dbf(orders_path, {{"ONE", 10}, {"TWO", 20}, {"THREE", 30}});
    write_synthetic_cdx(people_cdx_path, "NAME", "UPPER(NAME)");

    const fs::path main_path = temp_root / "in_target_commands.prg";
    write_text(
        main_path,
        "USE '" + people_path.string() + "' ALIAS People IN 0\n"
        "USE '" + cities_path.string() + "' ALIAS Cities IN 0\n"
        "USE '" + orders_path.string() + "' ALIAS Orders IN 0\n"
        "SELECT Cities\n"
        "cOrderTarget = 'People'\n"
        "cDataTarget = 'Orders'\n"
        "cResolvedTargetAlias = ALIAS(cOrderTarget)\n"
        "cPeopleTagBefore = TAG(1, 'People')\n"
        "SET ORDER TO TAG NAME IN cOrderTarget\n"
        "cPeopleOrder = ORDER('People')\n"
        "cCitiesOrder = ORDER('Cities')\n"
        "SEEK 'BRAVO' IN cOrderTarget\n"
        "nPeopleRecAfterSeek = RECNO('People')\n"
        "GO TOP IN cDataTarget\n"
        "SKIP 1 IN cDataTarget\n"
        "nOrdersRecAfterSkip = RECNO('Orders')\n"
        "LOCATE FOR AGE >= 20 WHILE AGE < 30 IN cDataTarget\n"
        "nOrdersRecAfterLocate = RECNO('Orders')\n"
        "GO TOP IN cDataTarget\n"
        "nScanHits = 0\n"
        "SCAN FOR AGE >= 20 WHILE AGE < 30 IN cDataTarget\n"
        "  nScanHits = nScanHits + 1\n"
        "ENDSCAN\n"
        "GO TOP IN cDataTarget\n"
        "REPLACE AGE WITH 77 FOR AGE >= 10 WHILE AGE < 30 IN cDataTarget\n"
        "DELETE FOR AGE = 77 WHILE AGE < 30 IN cDataTarget\n"
        "RECALL FOR AGE = 77 WHILE AGE < 30 IN cDataTarget\n"
        "nSelectedAfterCommands = SELECT()\n"
        "cSelectedAfterCommands = ALIAS()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "expression-driven IN targeting script should complete");

    const auto people_order = state.globals.find("cpeopleorder");
    const auto cities_order = state.globals.find("ccitiesorder");
    const auto people_rec_after_seek = state.globals.find("npeoplerecafterseek");
    const auto orders_rec_after_skip = state.globals.find("nordersrecafterskip");
    const auto orders_rec_after_locate = state.globals.find("nordersrecafterlocate");
    const auto scan_hits = state.globals.find("nscanhits");
    const auto selected_after_commands = state.globals.find("nselectedaftercommands");
    const auto selected_alias_after_commands = state.globals.find("cselectedaftercommands");

    expect(people_order != state.globals.end(), "SET ORDER TO ... IN cTarget should expose the targeted cursor order");
    expect(cities_order != state.globals.end(), "SET ORDER TO ... IN cTarget should leave the selected cursor order unchanged");
    expect(people_rec_after_seek != state.globals.end(), "SEEK ... IN cTarget should expose the targeted cursor position");
    expect(orders_rec_after_skip != state.globals.end(), "GO/SKIP IN cTarget should expose the targeted cursor position");
    expect(orders_rec_after_locate != state.globals.end(), "LOCATE ... IN cTarget should expose the targeted cursor position");
    expect(scan_hits != state.globals.end(), "SCAN ... IN cTarget should expose the targeted cursor iteration count");
    expect(selected_after_commands != state.globals.end(), "IN-targeted commands should preserve the current selected work area");
    expect(selected_alias_after_commands != state.globals.end(), "IN-targeted commands should preserve the current selected alias");

    if (people_order != state.globals.end()) {
        expect(copperfin::runtime::format_value(people_order->second) == "NAME", "SET ORDER TO ... IN cTarget should affect the targeted non-selected cursor");
    }
    if (cities_order != state.globals.end()) {
        expect(copperfin::runtime::format_value(cities_order->second).empty(), "SET ORDER TO ... IN cTarget should not alter the selected cursor");
    }
    if (people_rec_after_seek != state.globals.end()) {
        expect(copperfin::runtime::format_value(people_rec_after_seek->second) == "2", "SEEK ... IN cTarget should position the targeted cursor");
    }
    if (orders_rec_after_skip != state.globals.end()) {
        expect(copperfin::runtime::format_value(orders_rec_after_skip->second) == "2", "GO/SKIP IN cTarget should navigate the targeted cursor");
    }
    if (orders_rec_after_locate != state.globals.end()) {
        expect(copperfin::runtime::format_value(orders_rec_after_locate->second) == "2", "LOCATE ... WHILE ... IN cTarget should stop matching at the WHILE boundary on the targeted cursor");
    }
    if (scan_hits != state.globals.end()) {
        expect(copperfin::runtime::format_value(scan_hits->second) == "1", "SCAN ... WHILE ... IN cTarget should only iterate rows before the WHILE boundary on the targeted cursor");
    }
    if (selected_after_commands != state.globals.end()) {
        expect(copperfin::runtime::format_value(selected_after_commands->second) == "2", "IN-targeted commands should preserve the selected work area");
    }
    if (selected_alias_after_commands != state.globals.end()) {
        expect(copperfin::runtime::format_value(selected_alias_after_commands->second) == "Cities", "IN-targeted commands should preserve the selected alias");
    }

    const auto orders_result = copperfin::vfp::parse_dbf_table_from_file(orders_path.string(), 3U);
    expect(orders_result.ok, "Orders DBF should remain readable after IN-targeted mutation commands");
    if (orders_result.ok) {
        expect(orders_result.table.records.size() == 3U, "Orders DBF should still contain three records");
        if (orders_result.table.records.size() >= 3U) {
            expect(orders_result.table.records[0].values[1].display_value == "77", "REPLACE ... FOR ... WHILE ... IN cTarget should update matching record 1");
            expect(orders_result.table.records[1].values[1].display_value == "77", "REPLACE ... FOR ... WHILE ... IN cTarget should update matching record 2");
            expect(orders_result.table.records[2].values[1].display_value == "30", "REPLACE ... FOR ... WHILE ... IN cTarget should stop at the WHILE boundary");
            expect(!orders_result.table.records[0].deleted, "DELETE/RECALL ... WHILE ... IN cTarget should restore matching record 1");
            expect(!orders_result.table.records[1].deleted, "DELETE/RECALL ... WHILE ... IN cTarget should restore matching record 2");
            expect(!orders_result.table.records[2].deleted, "DELETE/RECALL ... WHILE ... IN cTarget should not affect records outside the WHILE boundary");
        }
    }

    fs::remove_all(temp_root, ignored);
}

void test_select_zero_and_use_in_zero_reuse_closed_work_area() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_select_zero_reuse";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path people_path = temp_root / "people.dbf";
    const fs::path cities_path = temp_root / "cities.dbf";
    write_simple_dbf(people_path, {"ALPHA", "BRAVO"});
    write_simple_dbf(cities_path, {"OSLO", "TOKYO"});

    const fs::path main_path = temp_root / "select_zero_reuse.prg";
    write_text(
        main_path,
        "USE '" + people_path.string() + "' ALIAS People IN 0\n"
        "USE IN People\n"
        "nSelectedAfterClose = SELECT()\n"
        "nNextFreeBeforeReuse = SELECT(0)\n"
        "USE '" + cities_path.string() + "' ALIAS Cities IN 0\n"
        "nCitiesArea = SELECT('Cities')\n"
        "nSelectedAfterReuse = SELECT()\n"
        "nNextFreeAfterReuse = SELECT(0)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SELECT(0) reuse script should complete");
    expect(state.work_area.selected == 1, "reopening through IN 0 should keep the reused work area selected");
    expect(state.work_area.aliases.size() == 1U, "reopening through IN 0 should reuse the freed work area instead of allocating a new one");

    const auto selected_after_close = state.globals.find("nselectedafterclose");
    const auto next_free_before_reuse = state.globals.find("nnextfreebeforereuse");
    const auto cities_area = state.globals.find("ncitiesarea");
    const auto selected_after_reuse = state.globals.find("nselectedafterreuse");
    const auto next_free_after_reuse = state.globals.find("nnextfreeafterreuse");

    expect(selected_after_close != state.globals.end(), "selected work area after closing the cursor should be captured");
    expect(next_free_before_reuse != state.globals.end(), "SELECT(0) before reopening should be captured");
    expect(cities_area != state.globals.end(), "SELECT('Cities') after reopening should be captured");
    expect(selected_after_reuse != state.globals.end(), "selected work area after reopening should be captured");
    expect(next_free_after_reuse != state.globals.end(), "SELECT(0) after reopening should be captured");

    if (selected_after_close != state.globals.end()) {
        expect(copperfin::runtime::format_value(selected_after_close->second) == "1", "closing the selected alias should leave the same work area selected");
    }
    if (next_free_before_reuse != state.globals.end()) {
        expect(copperfin::runtime::format_value(next_free_before_reuse->second) == "1", "SELECT(0) should report the freed work area without consuming it");
    }
    if (cities_area != state.globals.end()) {
        expect(copperfin::runtime::format_value(cities_area->second) == "1", "USE ... IN 0 should reuse the freed work area");
    }
    if (selected_after_reuse != state.globals.end()) {
        expect(copperfin::runtime::format_value(selected_after_reuse->second) == "1", "reopening through IN 0 should keep the reused work area selected");
    }
    if (next_free_after_reuse != state.globals.end()) {
        expect(copperfin::runtime::format_value(next_free_after_reuse->second) == "2", "SELECT(0) should advance only after the freed work area is reused");
    }

    fs::remove_all(temp_root, ignored);
}

void test_go_and_skip_cursor_navigation() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_navigation";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_simple_dbf(table_path, {"ALPHA", "BRAVO", "CHARLIE"});

    const fs::path main_path = temp_root / "navigate.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "GO BOTTOM\n"
        "nBottom = RECNO()\n"
        "SKIP\n"
        "lEof1 = EOF()\n"
        "nAfterEof = RECNO()\n"
        "SKIP -1\n"
        "nBackOne = RECNO()\n"
        "GO TOP\n"
        "nTop = RECNO()\n"
        "SKIP -1\n"
        "lBof1 = BOF()\n"
        "nAfterBof = RECNO()\n"
        "GO 2\n"
        "nMiddle = RECNO()\n"
        "GO 99\n"
        "lEof2 = EOF()\n"
        "nAfterGoPastEnd = RECNO()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "navigation script should complete");

    const auto bottom = state.globals.find("nbottom");
    const auto eof1 = state.globals.find("leof1");
    const auto after_eof = state.globals.find("naftereof");
    const auto back_one = state.globals.find("nbackone");
    const auto top = state.globals.find("ntop");
    const auto bof1 = state.globals.find("lbof1");
    const auto after_bof = state.globals.find("nafterbof");
    const auto middle = state.globals.find("nmiddle");
    const auto eof2 = state.globals.find("leof2");
    const auto after_go_past_end = state.globals.find("naftergopastend");

    expect(bottom != state.globals.end(), "GO BOTTOM should affect RECNO()");
    expect(eof1 != state.globals.end(), "SKIP past the end should affect EOF()");
    expect(after_eof != state.globals.end(), "RECNO() after EOF should be captured");
    expect(back_one != state.globals.end(), "SKIP -1 should recover from EOF");
    expect(top != state.globals.end(), "GO TOP should affect RECNO()");
    expect(bof1 != state.globals.end(), "SKIP -1 from the top should affect BOF()");
    expect(after_bof != state.globals.end(), "RECNO() after BOF should be captured");
    expect(middle != state.globals.end(), "GO 2 should move to the requested record");
    expect(eof2 != state.globals.end(), "GO past the end should affect EOF()");
    expect(after_go_past_end != state.globals.end(), "RECNO() after GO past the end should be captured");

    if (bottom != state.globals.end()) {
        expect(copperfin::runtime::format_value(bottom->second) == "3", "GO BOTTOM should move to the last record");
    }
    if (eof1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(eof1->second) == "true", "SKIP from the last record should move to EOF");
    }
    if (after_eof != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_eof->second) == "4", "RECNO() at EOF should be record_count + 1");
    }
    if (back_one != state.globals.end()) {
        expect(copperfin::runtime::format_value(back_one->second) == "3", "SKIP -1 from EOF should move back to the last record");
    }
    if (top != state.globals.end()) {
        expect(copperfin::runtime::format_value(top->second) == "1", "GO TOP should move to the first record");
    }
    if (bof1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(bof1->second) == "true", "SKIP -1 from the first record should move to BOF");
    }
    if (after_bof != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_bof->second) == "0", "RECNO() at BOF should be zero");
    }
    if (middle != state.globals.end()) {
        expect(copperfin::runtime::format_value(middle->second) == "2", "GO 2 should move to the requested record number");
    }
    if (eof2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(eof2->second) == "true", "GO past the end should move to EOF");
    }
    if (after_go_past_end != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_go_past_end->second) == "4", "GO past the end should place RECNO() at record_count + 1");
    }

    expect(
        has_runtime_event(state.events, "runtime.go", "BOTTOM") &&
        has_runtime_event(state.events, "runtime.go", "TOP") &&
        has_runtime_event(state.events, "runtime.skip", "1") &&
        has_runtime_event(state.events, "runtime.skip", "-1"),
        "navigation commands should emit runtime.go and runtime.skip events");

    fs::remove_all(temp_root, ignored);
}

void test_cursor_identity_functions_for_local_tables() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_cursor_identity_local";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_simple_dbf(table_path, {"ALPHA", "BRAVO"});

    const fs::path main_path = temp_root / "identity_local.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "lUsed1 = USED()\n"
        "lUsedPeople = USED('People')\n"
        "cDbf1 = DBF()\n"
        "cDbfPeople = DBF('People')\n"
        "nFields1 = FCOUNT()\n"
        "nFieldsPeople = FCOUNT('People')\n"
        "SET DATASESSION TO 2\n"
        "lUsed2 = USED('People')\n"
        "SET DATASESSION TO 1\n"
        "USE IN People\n"
        "lUsedAfterClose = USED('People')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "local cursor identity script should complete");

    const auto used1 = state.globals.find("lused1");
    const auto used_people = state.globals.find("lusedpeople");
    const auto dbf1 = state.globals.find("cdbf1");
    const auto dbf_people = state.globals.find("cdbfpeople");
    const auto fields1 = state.globals.find("nfields1");
    const auto fields_people = state.globals.find("nfieldspeople");
    const auto used2 = state.globals.find("lused2");
    const auto used_after_close = state.globals.find("lusedafterclose");

    expect(used1 != state.globals.end(), "USED() should be captured for the current local cursor");
    expect(used_people != state.globals.end(), "USED('People') should be captured for the named local cursor");
    expect(dbf1 != state.globals.end(), "DBF() should be captured for the current local cursor");
    expect(dbf_people != state.globals.end(), "DBF('People') should be captured for the named local cursor");
    expect(fields1 != state.globals.end(), "FCOUNT() should be captured for the current local cursor");
    expect(fields_people != state.globals.end(), "FCOUNT('People') should be captured for the named local cursor");
    expect(used2 != state.globals.end(), "USED('People') in a different data session should be captured");
    expect(used_after_close != state.globals.end(), "USED('People') after USE IN should be captured");

    if (used1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(used1->second) == "true", "USED() should report true for an open local cursor");
    }
    if (used_people != state.globals.end()) {
        expect(copperfin::runtime::format_value(used_people->second) == "true", "USED('People') should report true for an open alias");
    }
    if (dbf1 != state.globals.end()) {
        expect(
            lowercase_copy(copperfin::runtime::format_value(dbf1->second)).find("people.dbf") != std::string::npos,
            "DBF() should expose the opened local table path");
    }
    if (dbf_people != state.globals.end()) {
        expect(
            lowercase_copy(copperfin::runtime::format_value(dbf_people->second)).find("people.dbf") != std::string::npos,
            "DBF('People') should expose the opened local table path");
    }
    if (fields1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(fields1->second) == "1", "FCOUNT() should reflect the local DBF schema");
    }
    if (fields_people != state.globals.end()) {
        expect(copperfin::runtime::format_value(fields_people->second) == "1", "FCOUNT('People') should reflect the local DBF schema");
    }
    if (used2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(used2->second) == "false", "data sessions should isolate USED('alias') state");
    }
    if (used_after_close != state.globals.end()) {
        expect(copperfin::runtime::format_value(used_after_close->second) == "false", "USE IN should clear USED('alias') state");
    }

    fs::remove_all(temp_root, ignored);
}

void test_local_use_auto_allocation_tracks_session_selection_flow() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_local_selection_flow";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path people_path = temp_root / "people.dbf";
    const fs::path orders_path = temp_root / "orders.dbf";
    write_simple_dbf(people_path, {"ALPHA", "BRAVO"});
    write_simple_dbf(orders_path, {"ORDER1", "ORDER2"});

    const fs::path main_path = temp_root / "local_selection_flow.prg";
    write_text(
        main_path,
        "SELECT 0\n"
        "nSession1SelectedBefore = SELECT()\n"
        "USE '" + people_path.string() + "' ALIAS People\n"
        "nSession1Area = SELECT('People')\n"
        "nSession1SelectedAfter = SELECT()\n"
        "SET DATASESSION TO 2\n"
        "SELECT 0\n"
        "SELECT 0\n"
        "nSession2SelectedBefore = SELECT()\n"
        "USE '" + orders_path.string() + "' ALIAS Orders\n"
        "nSession2Area = SELECT('Orders')\n"
        "nSession2SelectedAfter = SELECT()\n"
        "SET DATASESSION TO 1\n"
        "nSession1AreaBack = SELECT('People')\n"
        "nSession1SelectedBack = SELECT()\n"
        "cSession1AliasBack = ALIAS()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "local selection-flow script should complete");
    expect(state.work_area.data_session == 1, "local selection-flow script should restore data session 1");

    const auto session1_selected_before = state.globals.find("nsession1selectedbefore");
    const auto session1_area = state.globals.find("nsession1area");
    const auto session1_selected_after = state.globals.find("nsession1selectedafter");
    const auto session2_selected_before = state.globals.find("nsession2selectedbefore");
    const auto session2_area = state.globals.find("nsession2area");
    const auto session2_selected_after = state.globals.find("nsession2selectedafter");
    const auto session1_area_back = state.globals.find("nsession1areaback");
    const auto session1_selected_back = state.globals.find("nsession1selectedback");
    const auto session1_alias_back = state.globals.find("csession1aliasback");

    expect(session1_selected_before != state.globals.end(), "session-1 selected area before USE should be captured");
    expect(session1_area != state.globals.end(), "session-1 local cursor area should be captured");
    expect(session1_selected_after != state.globals.end(), "session-1 selected area after USE should be captured");
    expect(session2_selected_before != state.globals.end(), "session-2 selected area before USE should be captured");
    expect(session2_area != state.globals.end(), "session-2 local cursor area should be captured");
    expect(session2_selected_after != state.globals.end(), "session-2 selected area after USE should be captured");
    expect(session1_area_back != state.globals.end(), "session-1 local cursor area after restoring the session should be captured");
    expect(session1_selected_back != state.globals.end(), "session-1 selected area after restoring the session should be captured");
    expect(session1_alias_back != state.globals.end(), "session-1 alias after restoring the session should be captured");

    if (session1_selected_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(session1_selected_before->second) == "1", "session 1 should keep work area 1 selected before its first USE");
    }
    if (session1_area != state.globals.end()) {
        expect(copperfin::runtime::format_value(session1_area->second) == "1", "plain USE should reuse session 1's selected empty work area");
    }
    if (session1_selected_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(session1_selected_after->second) == "1", "plain USE should keep the local cursor on session 1's selected work area");
    }
    if (session2_selected_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(session2_selected_before->second) == "2", "session 2 should preserve its own SELECT 0 flow before USE");
    }
    if (session2_area != state.globals.end()) {
        expect(copperfin::runtime::format_value(session2_area->second) == "2", "plain USE should reuse session 2's selected empty work area");
    }
    if (session2_selected_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(session2_selected_after->second) == "2", "plain USE should keep the local cursor on session 2's selected work area");
    }
    if (session1_area_back != state.globals.end()) {
        expect(copperfin::runtime::format_value(session1_area_back->second) == "1", "restoring session 1 should preserve that session's local cursor work area");
    }
    if (session1_selected_back != state.globals.end()) {
        expect(copperfin::runtime::format_value(session1_selected_back->second) == "1", "restoring session 1 should restore its selected work area");
    }
    if (session1_alias_back != state.globals.end()) {
        expect(copperfin::runtime::format_value(session1_alias_back->second) == "People", "restoring session 1 should restore its selected alias");
    }

    fs::remove_all(temp_root, ignored);
}

void test_local_selected_empty_area_reuses_after_datasession_round_trip() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_local_selection_reuse_round_trip";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path people_path = temp_root / "people.dbf";
    const fs::path orders_path = temp_root / "orders.dbf";
    const fs::path cities_path = temp_root / "cities.dbf";
    write_simple_dbf(people_path, {"ALPHA", "BRAVO"});
    write_simple_dbf(orders_path, {"ORDER1", "ORDER2"});
    write_simple_dbf(cities_path, {"OSLO", "TOKYO"});

    const fs::path main_path = temp_root / "local_selection_reuse_round_trip.prg";
    write_text(
        main_path,
        "USE '" + people_path.string() + "' ALIAS People IN 0\n"
        "USE IN People\n"
        "nSession1SelectedAfterClose = SELECT()\n"
        "nSession1NextFreeAfterClose = SELECT(0)\n"
        "SET DATASESSION TO 2\n"
        "USE '" + orders_path.string() + "' ALIAS Orders IN 0\n"
        "SET DATASESSION TO 1\n"
        "USE '" + cities_path.string() + "' ALIAS Cities\n"
        "nCitiesArea = SELECT('Cities')\n"
        "nSession1SelectedAfterReuse = SELECT()\n"
        "cSession1AliasAfterReuse = ALIAS()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "local selection reuse round-trip script should complete");
    expect(state.work_area.data_session == 1, "local selection reuse round-trip script should restore data session 1");

    const auto session1_selected_after_close = state.globals.find("nsession1selectedafterclose");
    const auto session1_next_free_after_close = state.globals.find("nsession1nextfreeafterclose");
    const auto cities_area = state.globals.find("ncitiesarea");
    const auto session1_selected_after_reuse = state.globals.find("nsession1selectedafterreuse");
    const auto session1_alias_after_reuse = state.globals.find("csession1aliasafterreuse");

    expect(session1_selected_after_close != state.globals.end(), "session-1 selected area after closing its alias should be captured");
    expect(session1_next_free_after_close != state.globals.end(), "session-1 SELECT(0) after closing its alias should be captured");
    expect(cities_area != state.globals.end(), "session-1 replacement local cursor area should be captured after restoring the session");
    expect(session1_selected_after_reuse != state.globals.end(), "session-1 selected area after reuse should be captured");
    expect(session1_alias_after_reuse != state.globals.end(), "session-1 alias after reuse should be captured");

    if (session1_selected_after_close != state.globals.end()) {
        expect(copperfin::runtime::format_value(session1_selected_after_close->second) == "1", "closing the selected alias should leave session 1 on that empty work area");
    }
    if (session1_next_free_after_close != state.globals.end()) {
        expect(copperfin::runtime::format_value(session1_next_free_after_close->second) == "1", "SELECT(0) should report the freed selected work area before the round trip");
    }
    if (cities_area != state.globals.end()) {
        expect(copperfin::runtime::format_value(cities_area->second) == "1", "restoring session 1 should let plain USE reuse that emptied selected work area");
    }
    if (session1_selected_after_reuse != state.globals.end()) {
        expect(copperfin::runtime::format_value(session1_selected_after_reuse->second) == "1", "restoring session 1 should keep the reused local cursor on the original selected work area");
    }
    if (session1_alias_after_reuse != state.globals.end()) {
        expect(copperfin::runtime::format_value(session1_alias_after_reuse->second) == "Cities", "restoring session 1 should expose the replacement alias on the reused work area");
    }

    fs::remove_all(temp_root, ignored);
}

void test_set_order_and_seek_for_local_tables() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_seek";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    write_simple_dbf(table_path, {"ALPHA", "BRAVO", "CHARLIE"});
    write_synthetic_cdx(cdx_path, "NAME", "UPPER(NAME)");

    const fs::path main_path = temp_root / "seek.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO TAG NAME\n"
        "SEEK 'BRAVO'\n"
        "lFound1 = FOUND()\n"
        "nRec1 = RECNO()\n"
        "SEEK 'ZZZZ'\n"
        "lFound2 = FOUND()\n"
        "lEof2 = EOF()\n"
        "nRec2 = RECNO()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "seek script should complete");

    const auto found1 = state.globals.find("lfound1");
    const auto rec1 = state.globals.find("nrec1");
    const auto found2 = state.globals.find("lfound2");
    const auto eof2 = state.globals.find("leof2");
    const auto rec2 = state.globals.find("nrec2");

    expect(found1 != state.globals.end(), "FOUND() after a successful SEEK should be captured");
    expect(rec1 != state.globals.end(), "RECNO() after a successful SEEK should be captured");
    expect(found2 != state.globals.end(), "FOUND() after a failed SEEK should be captured");
    expect(eof2 != state.globals.end(), "EOF() after a failed SEEK should be captured");
    expect(rec2 != state.globals.end(), "RECNO() after a failed SEEK should be captured");

    if (found1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(found1->second) == "true", "SEEK should set FOUND() when it locates a matching record");
    }
    if (rec1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(rec1->second) == "2", "SEEK should move the record pointer to the matched row");
    }
    if (found2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(found2->second) == "false", "failed SEEK should clear FOUND()");
    }
    if (eof2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(eof2->second) == "true", "failed SEEK should move the cursor to EOF");
    }
    if (rec2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(rec2->second) == "4", "failed SEEK should place RECNO() at record_count + 1");
    }

    expect(
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
            return event.category == "runtime.order" &&
                event.detail.find("NAME [norm=upper, coll=case-folded]") != std::string::npos;
        }) &&
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
            return event.category == "runtime.seek" &&
                event.detail.find("NAME [norm=upper, coll=case-folded]: BRAVO -> found") != std::string::npos;
        }),
        "SET ORDER and SEEK should emit runtime.order and runtime.seek events");

    fs::remove_all(temp_root, ignored);
}

void test_seek_uses_grounded_order_normalization_hints() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_seek_normalization";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    write_simple_dbf(table_path, {"ALPHA", "BRAVO", "CHARLIE"});
    write_synthetic_cdx(cdx_path, "NAME", "UPPER(NAME)");

    const fs::path main_path = temp_root / "seek_normalization.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO TAG NAME\n"
        "lSeekCmd = SEEK('bravo')\n"
        "nSeekCmdRec = RECNO()\n"
        "GO TOP\n"
        "lSeekFn = SEEK('charlie', 'People', 'NAME')\n"
        "nSeekFnRec = RECNO()\n"
        "SET ORDER TO TAG NAME DESCENDING\n"
        "lSeekDesc = SEEK('alpha')\n"
        "nSeekDescRec = RECNO()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "normalization-aware seek script should complete");

    const auto seek_cmd = state.globals.find("lseekcmd");
    const auto seek_cmd_rec = state.globals.find("nseekcmdrec");
    const auto seek_fn = state.globals.find("lseekfn");
    const auto seek_fn_rec = state.globals.find("nseekfnrec");
    const auto seek_desc = state.globals.find("lseekdesc");
    const auto seek_desc_rec = state.globals.find("nseekdescrec");

    expect(seek_cmd != state.globals.end(), "command SEEK result should be captured");
    expect(seek_cmd_rec != state.globals.end(), "command SEEK RECNO() should be captured");
    expect(seek_fn != state.globals.end(), "SEEK() result should be captured");
    expect(seek_fn_rec != state.globals.end(), "SEEK() RECNO() should be captured");
    expect(seek_desc != state.globals.end(), "descending normalized SEEK result should be captured");
    expect(seek_desc_rec != state.globals.end(), "descending normalized SEEK RECNO() should be captured");

    if (seek_cmd != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_cmd->second) == "true", "command SEEK should honor grounded upper normalization hints");
    }
    if (seek_cmd_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_cmd_rec->second) == "2", "command SEEK should land on the case-folded match");
    }
    if (seek_fn != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_fn->second) == "true", "SEEK() should honor grounded upper normalization hints");
    }
    if (seek_fn_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_fn_rec->second) == "3", "SEEK() should move to the normalized match");
    }
    if (seek_desc != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_desc->second) == "true", "descending SEEK should also honor grounded normalization hints");
    }
    if (seek_desc_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_desc_rec->second) == "1", "descending SEEK should land on the normalized exact match");
    }

    fs::remove_all(temp_root, ignored);
}

void test_seek_supports_composite_tag_expressions() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_seek_composite";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "LAST", .type = 'C', .length = 10U},
        {.name = "FIRST", .type = 'C', .length = 10U},
    };
    const std::vector<std::vector<std::string>> records{
        {"DOE", "JOHN"},
        {"SMITH", "JANE"},
        {"TAYLOR", "ALEX"},
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "composite seek DBF fixture should be created");
    write_synthetic_cdx(cdx_path, "FULLNAME", "UPPER(LAST+FIRST)");

    const fs::path main_path = temp_root / "seek_composite.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO TAG FULLNAME\n"
        "lSeekCmd = SEEK('smithjane')\n"
        "nSeekCmdRec = RECNO()\n"
        "GO TOP\n"
        "lSeekFn = SEEK('doejohn', 'People', 'FULLNAME')\n"
        "nSeekFnRec = RECNO()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "composite-expression seek script should complete");

    const auto seek_cmd = state.globals.find("lseekcmd");
    const auto seek_cmd_rec = state.globals.find("nseekcmdrec");
    const auto seek_fn = state.globals.find("lseekfn");
    const auto seek_fn_rec = state.globals.find("nseekfnrec");

    expect(seek_cmd != state.globals.end(), "command SEEK on a composite tag should be captured");
    expect(seek_cmd_rec != state.globals.end(), "command composite SEEK RECNO() should be captured");
    expect(seek_fn != state.globals.end(), "SEEK() on a composite tag should be captured");
    expect(seek_fn_rec != state.globals.end(), "SEEK() composite RECNO() should be captured");

    if (seek_cmd != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_cmd->second) == "true", "command SEEK should match concatenated composite-tag keys");
    }
    if (seek_cmd_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_cmd_rec->second) == "2", "command SEEK should land on the concatenated composite-tag match");
    }
    if (seek_fn != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_fn->second) == "true", "SEEK() should match concatenated composite-tag keys");
    }
    if (seek_fn_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_fn_rec->second) == "1", "SEEK() should land on the requested composite-tag match");
    }

    fs::remove_all(temp_root, ignored);
}

void test_seek_supports_left_function_tag_expressions() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_seek_left";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    write_simple_dbf(table_path, {"ALPHA", "BRAVO", "CHARLIE"});
    write_synthetic_cdx(cdx_path, "NAME3", "UPPER(LEFT(NAME, 3))");

    const fs::path main_path = temp_root / "seek_left.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO TAG NAME3\n"
        "lSeekCmd = SEEK('cha')\n"
        "nSeekCmdRec = RECNO()\n"
        "GO TOP\n"
        "lSeekFn = SEEK('bra', 'People', 'NAME3')\n"
        "nSeekFnRec = RECNO()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "LEFT()-expression seek script should complete");

    const auto seek_cmd = state.globals.find("lseekcmd");
    const auto seek_cmd_rec = state.globals.find("nseekcmdrec");
    const auto seek_fn = state.globals.find("lseekfn");
    const auto seek_fn_rec = state.globals.find("nseekfnrec");

    expect(seek_cmd != state.globals.end(), "command SEEK on a LEFT() tag should be captured");
    expect(seek_cmd_rec != state.globals.end(), "command LEFT() SEEK RECNO() should be captured");
    expect(seek_fn != state.globals.end(), "SEEK() on a LEFT() tag should be captured");
    expect(seek_fn_rec != state.globals.end(), "SEEK() LEFT() RECNO() should be captured");

    if (seek_cmd != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_cmd->second) == "true", "command SEEK should match LEFT()-derived tag keys");
    }
    if (seek_cmd_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_cmd_rec->second) == "3", "command SEEK should land on the LEFT()-derived exact match");
    }
    if (seek_fn != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_fn->second) == "true", "SEEK() should match LEFT()-derived tag keys");
    }
    if (seek_fn_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_fn_rec->second) == "2", "SEEK() should land on the requested LEFT()-derived match");
    }

    fs::remove_all(temp_root, ignored);
}

void test_seek_supports_right_function_tag_expressions() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_seek_right";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    write_simple_dbf(table_path, {"ALPHA", "BRAVO", "CHARLIE"});
    write_synthetic_cdx(cdx_path, "NAMEEND", "UPPER(RIGHT(NAME, 3))");

    const fs::path main_path = temp_root / "seek_right.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO TAG NAMEEND\n"
        "lSeekCmd = SEEK('lie')\n"
        "nSeekCmdRec = RECNO()\n"
        "GO TOP\n"
        "lSeekFn = SEEK('avo', 'People', 'NAMEEND')\n"
        "nSeekFnRec = RECNO()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "RIGHT()-expression seek script should complete");

    const auto seek_cmd = state.globals.find("lseekcmd");
    const auto seek_cmd_rec = state.globals.find("nseekcmdrec");
    const auto seek_fn = state.globals.find("lseekfn");
    const auto seek_fn_rec = state.globals.find("nseekfnrec");

    expect(seek_cmd != state.globals.end(), "command SEEK on a RIGHT() tag should be captured");
    expect(seek_cmd_rec != state.globals.end(), "command RIGHT() SEEK RECNO() should be captured");
    expect(seek_fn != state.globals.end(), "SEEK() on a RIGHT() tag should be captured");
    expect(seek_fn_rec != state.globals.end(), "SEEK() RIGHT() RECNO() should be captured");

    if (seek_cmd != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_cmd->second) == "true", "command SEEK should match RIGHT()-derived tag keys");
    }
    if (seek_cmd_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_cmd_rec->second) == "3", "command SEEK should land on the RIGHT()-derived exact match");
    }
    if (seek_fn != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_fn->second) == "true", "SEEK() should match RIGHT()-derived tag keys");
    }
    if (seek_fn_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_fn_rec->second) == "2", "SEEK() should land on the requested RIGHT()-derived match");
    }

    fs::remove_all(temp_root, ignored);
}

void test_seek_supports_substr_function_tag_expressions() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_seek_substr";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    write_simple_dbf(table_path, {"ALPHA", "BRAVO", "CHARLIE"});
    write_synthetic_cdx(cdx_path, "NAMEMID", "UPPER(SUBSTR(NAME, 2, 3))");

    const fs::path main_path = temp_root / "seek_substr.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO TAG NAMEMID\n"
        "lSeekCmd = SEEK('har')\n"
        "nSeekCmdRec = RECNO()\n"
        "GO TOP\n"
        "lSeekFn = SEEK('rav', 'People', 'NAMEMID')\n"
        "nSeekFnRec = RECNO()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SUBSTR()-expression seek script should complete");

    const auto seek_cmd = state.globals.find("lseekcmd");
    const auto seek_cmd_rec = state.globals.find("nseekcmdrec");
    const auto seek_fn = state.globals.find("lseekfn");
    const auto seek_fn_rec = state.globals.find("nseekfnrec");

    expect(seek_cmd != state.globals.end(), "command SEEK on a SUBSTR() tag should be captured");
    expect(seek_cmd_rec != state.globals.end(), "command SUBSTR() SEEK RECNO() should be captured");
    expect(seek_fn != state.globals.end(), "SEEK() on a SUBSTR() tag should be captured");
    expect(seek_fn_rec != state.globals.end(), "SEEK() SUBSTR() RECNO() should be captured");

    if (seek_cmd != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_cmd->second) == "true", "command SEEK should match SUBSTR()-derived tag keys");
    }
    if (seek_cmd_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_cmd_rec->second) == "3", "command SEEK should land on the SUBSTR()-derived exact match");
    }
    if (seek_fn != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_fn->second) == "true", "SEEK() should match SUBSTR()-derived tag keys");
    }
    if (seek_fn_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_fn_rec->second) == "2", "SEEK() should land on the requested SUBSTR()-derived match");
    }

    fs::remove_all(temp_root, ignored);
}

void test_seek_supports_padl_function_tag_expressions() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_seek_padl";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    write_simple_dbf(table_path, {"ALPHA", "BRAVO", "CHARLIE"});
    write_synthetic_cdx(cdx_path, "LPAD", "UPPER(PADL(NAME, 8, '0'))");

    const fs::path main_path = temp_root / "seek_padl.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO TAG LPAD\n"
        "lSeekCmd = SEEK('000ALPHA')\n"
        "nSeekCmdRec = RECNO()\n"
        "GO TOP\n"
        "lSeekFn = SEEK('000BRAVO', 'People', 'LPAD')\n"
        "nSeekFnRec = RECNO()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "PADL()-expression seek script should complete");

    const auto seek_cmd = state.globals.find("lseekcmd");
    const auto seek_cmd_rec = state.globals.find("nseekcmdrec");
    const auto seek_fn = state.globals.find("lseekfn");
    const auto seek_fn_rec = state.globals.find("nseekfnrec");

    expect(seek_cmd != state.globals.end(), "command SEEK on a PADL() tag should be captured");
    expect(seek_cmd_rec != state.globals.end(), "command PADL() SEEK RECNO() should be captured");
    expect(seek_fn != state.globals.end(), "SEEK() on a PADL() tag should be captured");
    expect(seek_fn_rec != state.globals.end(), "SEEK() PADL() RECNO() should be captured");

    if (seek_cmd != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_cmd->second) == "true", "command SEEK should match PADL()-derived tag keys");
    }
    if (seek_cmd_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_cmd_rec->second) == "1", "command SEEK should land on the PADL()-derived exact match");
    }
    if (seek_fn != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_fn->second) == "true", "SEEK() should match PADL()-derived tag keys");
    }
    if (seek_fn_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_fn_rec->second) == "2", "SEEK() should land on the requested PADL()-derived match");
    }

    fs::remove_all(temp_root, ignored);
}

void test_seek_supports_padr_function_tag_expressions() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_seek_padr";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    write_simple_dbf(table_path, {"ALPHA", "BRAVO", "CHARLIE"});
    write_synthetic_cdx(cdx_path, "RPAD", "UPPER(PADR(NAME, 8, '0'))");

    const fs::path main_path = temp_root / "seek_padr.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO TAG RPAD\n"
        "lSeekCmd = SEEK('ALPHA000')\n"
        "nSeekCmdRec = RECNO()\n"
        "GO TOP\n"
        "lSeekFn = SEEK('BRAVO000', 'People', 'RPAD')\n"
        "nSeekFnRec = RECNO()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "PADR()-expression seek script should complete");

    const auto seek_cmd = state.globals.find("lseekcmd");
    const auto seek_cmd_rec = state.globals.find("nseekcmdrec");
    const auto seek_fn = state.globals.find("lseekfn");
    const auto seek_fn_rec = state.globals.find("nseekfnrec");

    expect(seek_cmd != state.globals.end(), "command SEEK on a PADR() tag should be captured");
    expect(seek_cmd_rec != state.globals.end(), "command PADR() SEEK RECNO() should be captured");
    expect(seek_fn != state.globals.end(), "SEEK() on a PADR() tag should be captured");
    expect(seek_fn_rec != state.globals.end(), "SEEK() PADR() RECNO() should be captured");

    if (seek_cmd != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_cmd->second) == "true", "command SEEK should match PADR()-derived tag keys");
    }
    if (seek_cmd_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_cmd_rec->second) == "1", "command SEEK should land on the PADR()-derived exact match");
    }
    if (seek_fn != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_fn->second) == "true", "SEEK() should match PADR()-derived tag keys");
    }
    if (seek_fn_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_fn_rec->second) == "2", "SEEK() should land on the requested PADR()-derived match");
    }

    fs::remove_all(temp_root, ignored);
}

void test_seek_supports_padl_default_padding_tag_expressions() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_seek_padl_default";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    write_simple_dbf(table_path, {"ALPHA", "BRAVO", "CHARLIE"});
    write_synthetic_cdx(cdx_path, "LPADSP", "UPPER(PADL(NAME, 7))");

    const fs::path main_path = temp_root / "seek_padl_default.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO TAG LPADSP\n"
        "lSeekCmd = SEEK('  BRAVO')\n"
        "nSeekCmdRec = RECNO()\n"
        "GO TOP\n"
        "lSeekFn = SEEK('  ALPHA', 'People', 'LPADSP')\n"
        "nSeekFnRec = RECNO()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "PADL() default-padding seek script should complete");

    const auto seek_cmd = state.globals.find("lseekcmd");
    const auto seek_cmd_rec = state.globals.find("nseekcmdrec");
    const auto seek_fn = state.globals.find("lseekfn");
    const auto seek_fn_rec = state.globals.find("nseekfnrec");

    expect(seek_cmd != state.globals.end(), "command SEEK on a default PADL() tag should be captured");
    expect(seek_cmd_rec != state.globals.end(), "command default PADL() SEEK RECNO() should be captured");
    expect(seek_fn != state.globals.end(), "SEEK() on a default PADL() tag should be captured");
    expect(seek_fn_rec != state.globals.end(), "SEEK() default PADL() RECNO() should be captured");

    if (seek_cmd != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_cmd->second) == "true", "command SEEK should match default PADL()-derived tag keys");
    }
    if (seek_cmd_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_cmd_rec->second) == "2", "command SEEK should land on the default PADL()-derived exact match");
    }
    if (seek_fn != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_fn->second) == "true", "SEEK() should match default PADL()-derived tag keys");
    }
    if (seek_fn_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_fn_rec->second) == "1", "SEEK() should land on the requested default PADL()-derived match");
    }

    fs::remove_all(temp_root, ignored);
}

void test_seek_supports_padr_default_padding_tag_expressions() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_seek_padr_default";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    write_simple_dbf(table_path, {"ALPHA", "BRAVO", "CHARLIE"});
    write_synthetic_cdx(cdx_path, "RPADSP", "UPPER(PADR(NAME, 7))");

    const fs::path main_path = temp_root / "seek_padr_default.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO TAG RPADSP\n"
        "lSeekCmd = SEEK('ALPHA  ')\n"
        "nSeekCmdRec = RECNO()\n"
        "GO TOP\n"
        "lSeekFn = SEEK('CHARLIE', 'People', 'RPADSP')\n"
        "nSeekFnRec = RECNO()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "PADR() default-padding seek script should complete");

    const auto seek_cmd = state.globals.find("lseekcmd");
    const auto seek_cmd_rec = state.globals.find("nseekcmdrec");
    const auto seek_fn = state.globals.find("lseekfn");
    const auto seek_fn_rec = state.globals.find("nseekfnrec");

    expect(seek_cmd != state.globals.end(), "command SEEK on a default PADR() tag should be captured");
    expect(seek_cmd_rec != state.globals.end(), "command default PADR() SEEK RECNO() should be captured");
    expect(seek_fn != state.globals.end(), "SEEK() on a default PADR() tag should be captured");
    expect(seek_fn_rec != state.globals.end(), "SEEK() default PADR() RECNO() should be captured");

    if (seek_cmd != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_cmd->second) == "true", "command SEEK should match default PADR()-derived tag keys");
    }
    if (seek_cmd_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_cmd_rec->second) == "1", "command SEEK should land on the default PADR()-derived exact match");
    }
    if (seek_fn != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_fn->second) == "true", "SEEK() should match default PADR()-derived tag keys");
    }
    if (seek_fn_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_fn_rec->second) == "3", "SEEK() should land on the requested default PADR()-derived match");
    }

    fs::remove_all(temp_root, ignored);
}

void test_seek_supports_str_function_tag_expressions() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_seek_str";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}});
    write_synthetic_cdx(cdx_path, "AGESTR", "UPPER(STR(AGE, 3))");

    const fs::path main_path = temp_root / "seek_str.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO TAG AGESTR\n"
        "lSeekCmd = SEEK(' 30')\n"
        "nSeekCmdRec = RECNO()\n"
        "GO TOP\n"
        "lSeekFn = SEEK(' 10', 'People', 'AGESTR')\n"
        "nSeekFnRec = RECNO()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "STR()-expression seek script should complete");

    const auto seek_cmd = state.globals.find("lseekcmd");
    const auto seek_cmd_rec = state.globals.find("nseekcmdrec");
    const auto seek_fn = state.globals.find("lseekfn");
    const auto seek_fn_rec = state.globals.find("nseekfnrec");

    expect(seek_cmd != state.globals.end(), "command SEEK on a STR() tag should be captured");
    expect(seek_cmd_rec != state.globals.end(), "command STR() SEEK RECNO() should be captured");
    expect(seek_fn != state.globals.end(), "SEEK() on a STR() tag should be captured");
    expect(seek_fn_rec != state.globals.end(), "SEEK() STR() RECNO() should be captured");

    if (seek_cmd != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_cmd->second) == "true", "command SEEK should match STR()-derived tag keys");
    }
    if (seek_cmd_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_cmd_rec->second) == "3", "command SEEK should land on the STR()-derived exact match");
    }
    if (seek_fn != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_fn->second) == "true", "SEEK() should match STR()-derived tag keys");
    }
    if (seek_fn_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_fn_rec->second) == "1", "SEEK() should land on the requested STR()-derived match");
    }

    fs::remove_all(temp_root, ignored);
}

void test_seek_supports_str_default_width_tag_expressions() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_seek_str_default";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}});
    write_synthetic_cdx(cdx_path, "AGESTRD", "UPPER(STR(AGE))");

    const fs::path main_path = temp_root / "seek_str_default.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO TAG AGESTRD\n"
        "lSeekCmd = SEEK('        30')\n"
        "nSeekCmdRec = RECNO()\n"
        "GO TOP\n"
        "lSeekFn = SEEK('        10', 'People', 'AGESTRD')\n"
        "nSeekFnRec = RECNO()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "STR() default-width seek script should complete");

    const auto seek_cmd = state.globals.find("lseekcmd");
    const auto seek_cmd_rec = state.globals.find("nseekcmdrec");
    const auto seek_fn = state.globals.find("lseekfn");
    const auto seek_fn_rec = state.globals.find("nseekfnrec");

    expect(seek_cmd != state.globals.end(), "command SEEK on a STR() default-width tag should be captured");
    expect(seek_cmd_rec != state.globals.end(), "command STR() default-width SEEK RECNO() should be captured");
    expect(seek_fn != state.globals.end(), "SEEK() on a STR() default-width tag should be captured");
    expect(seek_fn_rec != state.globals.end(), "SEEK() STR() default-width RECNO() should be captured");

    if (seek_cmd != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_cmd->second) == "true", "command SEEK should match STR() default-width derived keys");
    }
    if (seek_cmd_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_cmd_rec->second) == "3", "command SEEK should land on STR() default-width exact match");
    }
    if (seek_fn != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_fn->second) == "true", "SEEK() should match STR() default-width derived keys");
    }
    if (seek_fn_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_fn_rec->second) == "1", "SEEK() should land on requested STR() default-width match");
    }

    fs::remove_all(temp_root, ignored);
}

void test_seek_supports_str_decimal_tag_expressions() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_seek_str_decimal";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}});
    write_synthetic_cdx(cdx_path, "AGESTRX", "UPPER(STR(AGE, 5, 1))");

    const fs::path main_path = temp_root / "seek_str_decimal.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO TAG AGESTRX\n"
        "lSeekCmd = SEEK(' 30.0')\n"
        "nSeekCmdRec = RECNO()\n"
        "GO TOP\n"
        "lSeekFn = SEEK(' 10.0', 'People', 'AGESTRX')\n"
        "nSeekFnRec = RECNO()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "STR() decimal seek script should complete");

    const auto seek_cmd = state.globals.find("lseekcmd");
    const auto seek_cmd_rec = state.globals.find("nseekcmdrec");
    const auto seek_fn = state.globals.find("lseekfn");
    const auto seek_fn_rec = state.globals.find("nseekfnrec");

    expect(seek_cmd != state.globals.end(), "command SEEK on a STR() decimal tag should be captured");
    expect(seek_cmd_rec != state.globals.end(), "command STR() decimal SEEK RECNO() should be captured");
    expect(seek_fn != state.globals.end(), "SEEK() on a STR() decimal tag should be captured");
    expect(seek_fn_rec != state.globals.end(), "SEEK() STR() decimal RECNO() should be captured");

    if (seek_cmd != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_cmd->second) == "true", "command SEEK should match STR() decimal derived keys");
    }
    if (seek_cmd_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_cmd_rec->second) == "3", "command SEEK should land on STR() decimal exact match");
    }
    if (seek_fn != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_fn->second) == "true", "SEEK() should match STR() decimal derived keys");
    }
    if (seek_fn_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_fn_rec->second) == "1", "SEEK() should land on requested STR() decimal match");
    }

    fs::remove_all(temp_root, ignored);
}

void test_set_near_changes_seek_failure_position() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_set_near";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    write_simple_dbf(table_path, {"ALPHA", "CHARLIE", "ECHO"});
    write_synthetic_cdx(cdx_path, "NAME", "UPPER(NAME)");

    const fs::path main_path = temp_root / "set_near.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO TAG NAME\n"
        "SET NEAR ON\n"
        "SEEK 'BRAVO'\n"
        "lNearFound = FOUND()\n"
        "lNearEof = EOF()\n"
        "nNearRec = RECNO()\n"
        "SET NEAR OFF\n"
        "SEEK 'BRAVO'\n"
        "lFarFound = FOUND()\n"
        "lFarEof = EOF()\n"
        "nFarRec = RECNO()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SET NEAR seek script should complete");

    const auto near_found = state.globals.find("lnearfound");
    const auto near_eof = state.globals.find("lneareof");
    const auto near_rec = state.globals.find("nnearrec");
    const auto far_found = state.globals.find("lfarfound");
    const auto far_eof = state.globals.find("lfareof");
    const auto far_rec = state.globals.find("nfarrec");

    expect(near_found != state.globals.end(), "FOUND() after SET NEAR ON should be captured");
    expect(near_eof != state.globals.end(), "EOF() after SET NEAR ON should be captured");
    expect(near_rec != state.globals.end(), "RECNO() after SET NEAR ON should be captured");
    expect(far_found != state.globals.end(), "FOUND() after SET NEAR OFF should be captured");
    expect(far_eof != state.globals.end(), "EOF() after SET NEAR OFF should be captured");
    expect(far_rec != state.globals.end(), "RECNO() after SET NEAR OFF should be captured");

    if (near_found != state.globals.end()) {
        expect(copperfin::runtime::format_value(near_found->second) == "false", "SET NEAR ON should keep FOUND() false when SEEK misses");
    }
    if (near_eof != state.globals.end()) {
        expect(copperfin::runtime::format_value(near_eof->second) == "false", "SET NEAR ON should position to the nearest record instead of EOF");
    }
    if (near_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(near_rec->second) == "2", "SET NEAR ON should position to the next ordered record");
    }
    if (far_found != state.globals.end()) {
        expect(copperfin::runtime::format_value(far_found->second) == "false", "SET NEAR OFF should still report a failed seek");
    }
    if (far_eof != state.globals.end()) {
        expect(copperfin::runtime::format_value(far_eof->second) == "true", "SET NEAR OFF should leave the cursor at EOF after a missed seek");
    }
    if (far_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(far_rec->second) == "4", "SET NEAR OFF should place RECNO() at record_count + 1 after a missed seek");
    }

    expect(
        has_runtime_event(state.events, "runtime.set", "NEAR ON") &&
        has_runtime_event(state.events, "runtime.set", "NEAR OFF"),
        "SET NEAR changes should emit runtime.set events");

    fs::remove_all(temp_root, ignored);
}

void test_set_order_descending_changes_seek_ordering() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_set_order_descending";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    write_simple_dbf(table_path, {"ALPHA", "CHARLIE", "ECHO"});
    write_synthetic_cdx(cdx_path, "NAME", "UPPER(NAME)");

    const fs::path main_path = temp_root / "set_order_descending.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO TAG NAME DESCENDING\n"
        "cOrder = ORDER()\n"
        "SEEK 'CHARLIE'\n"
        "lExactFound = FOUND()\n"
        "nExactRec = RECNO()\n"
        "SET NEAR ON\n"
        "SEEK 'BRAVO'\n"
        "lNearFound = FOUND()\n"
        "lNearEof = EOF()\n"
        "nNearRec = RECNO()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "descending SET ORDER script should complete");

    const auto order = state.globals.find("corder");
    const auto exact_found = state.globals.find("lexactfound");
    const auto exact_rec = state.globals.find("nexactrec");
    const auto near_found = state.globals.find("lnearfound");
    const auto near_eof = state.globals.find("lneareof");
    const auto near_rec = state.globals.find("nnearrec");

    expect(order != state.globals.end(), "ORDER() after descending SET ORDER should be captured");
    expect(exact_found != state.globals.end(), "FOUND() after exact descending SEEK should be captured");
    expect(exact_rec != state.globals.end(), "RECNO() after exact descending SEEK should be captured");
    expect(near_found != state.globals.end(), "FOUND() after descending SET NEAR seek should be captured");
    expect(near_eof != state.globals.end(), "EOF() after descending SET NEAR seek should be captured");
    expect(near_rec != state.globals.end(), "RECNO() after descending SET NEAR seek should be captured");

    if (order != state.globals.end()) {
        expect(copperfin::runtime::format_value(order->second) == "NAME", "descending SET ORDER should still expose the controlling tag name");
    }
    if (exact_found != state.globals.end()) {
        expect(copperfin::runtime::format_value(exact_found->second) == "true", "descending exact SEEK should still report FOUND()");
    }
    if (exact_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(exact_rec->second) == "2", "descending exact SEEK should still land on the matching row");
    }
    if (near_found != state.globals.end()) {
        expect(copperfin::runtime::format_value(near_found->second) == "false", "descending SET NEAR SEEK should still report a miss");
    }
    if (near_eof != state.globals.end()) {
        expect(copperfin::runtime::format_value(near_eof->second) == "false", "descending SET NEAR SEEK should stay off EOF when a nearby key exists");
    }
    if (near_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(near_rec->second) == "1", "descending SET NEAR SEEK should move to the next row in descending order");
    }

    expect(
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
            return event.category == "runtime.order" &&
                event.detail.find("NAME [norm=upper, coll=case-folded, dir=descending]") != std::string::npos;
        }) &&
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
            return event.category == "runtime.seek" &&
                event.detail.find("NAME [norm=upper, coll=case-folded, dir=descending]: BRAVO -> not found") != std::string::npos;
        }),
        "descending SET ORDER and SEEK should emit order-direction metadata");

    fs::remove_all(temp_root, ignored);
}

void test_seek_command_accepts_tag_override_without_set_order() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_seek_tag_override";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    write_simple_dbf(table_path, {"ALPHA", "BRAVO", "CHARLIE"});
    write_synthetic_cdx(cdx_path, "NAME", "UPPER(NAME)");

    const fs::path main_path = temp_root / "seek_tag_override.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SEEK 'BRAVO' TAG NAME\n"
        "lFound = FOUND()\n"
        "nRec = RECNO()\n"
        "cOrderAfter = ORDER()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SEEK ... TAG script should complete");

    const auto found = state.globals.find("lfound");
    const auto rec = state.globals.find("nrec");
    const auto order_after = state.globals.find("corderafter");

    expect(found != state.globals.end(), "SEEK ... TAG should expose FOUND()");
    expect(rec != state.globals.end(), "SEEK ... TAG should expose RECNO()");
    expect(order_after != state.globals.end(), "SEEK ... TAG should leave ORDER() observable");

    if (found != state.globals.end()) {
        expect(copperfin::runtime::format_value(found->second) == "true", "SEEK ... TAG should find matches using the named tag");
    }
    if (rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(rec->second) == "2", "SEEK ... TAG should position the cursor on the matching row");
    }
    if (order_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(order_after->second).empty(), "SEEK ... TAG should not permanently change the controlling order");
    }

    expect(
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
            return event.category == "runtime.seek" &&
                event.detail.find("NAME [norm=upper, coll=case-folded]: BRAVO -> found") != std::string::npos;
        }),
        "SEEK ... TAG should expose the temporary order metadata in runtime.seek events");

    fs::remove_all(temp_root, ignored);
}

void test_seek_command_accepts_descending_tag_override_without_set_order() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_seek_descending_tag_override";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    write_simple_dbf(table_path, {"ALPHA", "CHARLIE", "ECHO"});
    write_synthetic_cdx(cdx_path, "NAME", "UPPER(NAME)");

    const fs::path main_path = temp_root / "seek_descending_tag_override.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET NEAR ON\n"
        "SEEK 'BRAVO' TAG NAME DESCENDING\n"
        "lFound = FOUND()\n"
        "lEof = EOF()\n"
        "nRec = RECNO()\n"
        "cOrderAfter = ORDER()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SEEK ... TAG DESCENDING script should complete");

    const auto found = state.globals.find("lfound");
    const auto eof = state.globals.find("leof");
    const auto rec = state.globals.find("nrec");
    const auto order_after = state.globals.find("corderafter");

    expect(found != state.globals.end(), "SEEK ... TAG DESCENDING should expose FOUND()");
    expect(eof != state.globals.end(), "SEEK ... TAG DESCENDING should expose EOF()");
    expect(rec != state.globals.end(), "SEEK ... TAG DESCENDING should expose RECNO()");
    expect(order_after != state.globals.end(), "SEEK ... TAG DESCENDING should leave ORDER() observable");

    if (found != state.globals.end()) {
        expect(copperfin::runtime::format_value(found->second) == "false", "descending tag override should still report a miss for an in-between key");
    }
    if (eof != state.globals.end()) {
        expect(copperfin::runtime::format_value(eof->second) == "false", "descending tag override should honor SET NEAR and stay off EOF");
    }
    if (rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(rec->second) == "1", "descending tag override should position to the next row in descending order");
    }
    if (order_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(order_after->second).empty(), "descending tag override should not permanently change the controlling order");
    }

    expect(
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
            return event.category == "runtime.seek" &&
                event.detail.find("NAME [norm=upper, coll=case-folded, dir=descending]: BRAVO -> not found") != std::string::npos;
        }),
        "SEEK ... TAG DESCENDING should expose the temporary descending metadata in runtime.seek events");

    fs::remove_all(temp_root, ignored);
}

void test_seek_related_index_functions() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_seek_functions";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    write_simple_dbf(table_path, {"ALPHA", "BRAVO", "CHARLIE"});
    write_synthetic_cdx(cdx_path, "NAME", "UPPER(NAME)");

    const fs::path main_path = temp_root / "seek_functions.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO TAG NAME\n"
        "cOrder1 = ORDER()\n"
        "cOrder2 = ORDER('People', 1)\n"
        "cTag1 = TAG(1, 'People')\n"
        "lSeekFn = SEEK('BRAVO', 'People', 'NAME')\n"
        "nSeekRec = RECNO()\n"
        "GO TOP\n"
        "lIndexNoMove = INDEXSEEK('CHARLIE', .F., 'People', 'NAME')\n"
        "nAfterNoMove = RECNO()\n"
        "lIndexMove = INDEXSEEK('CHARLIE', .T., 'People', 'NAME')\n"
        "nAfterMove = RECNO()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "seek/index helper script should complete");

    const auto order1 = state.globals.find("corder1");
    const auto order2 = state.globals.find("corder2");
    const auto tag1 = state.globals.find("ctag1");
    const auto seek_fn = state.globals.find("lseekfn");
    const auto seek_rec = state.globals.find("nseekrec");
    const auto index_no_move = state.globals.find("lindexnomove");
    const auto after_no_move = state.globals.find("nafternomove");
    const auto index_move = state.globals.find("lindexmove");
    const auto after_move = state.globals.find("naftermove");

    expect(order1 != state.globals.end(), "ORDER() should be captured");
    expect(order2 != state.globals.end(), "ORDER(alias, pathFlag) should be captured");
    expect(tag1 != state.globals.end(), "TAG() should be captured");
    expect(seek_fn != state.globals.end(), "SEEK() should be captured");
    expect(seek_rec != state.globals.end(), "RECNO() after SEEK() should be captured");
    expect(index_no_move != state.globals.end(), "INDEXSEEK(.F.) should be captured");
    expect(after_no_move != state.globals.end(), "RECNO() after INDEXSEEK(.F.) should be captured");
    expect(index_move != state.globals.end(), "INDEXSEEK(.T.) should be captured");
    expect(after_move != state.globals.end(), "RECNO() after INDEXSEEK(.T.) should be captured");

    if (order1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(order1->second) == "NAME", "ORDER() should expose the controlling tag");
    }
    if (order2 != state.globals.end()) {
        expect(
            copperfin::runtime::format_value(order2->second).find("PEOPLE.CDX") != std::string::npos,
            "ORDER(alias, pathFlag) should expose the controlling index path");
    }
    if (tag1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(tag1->second) == "NAME", "TAG() should expose the first open tag");
    }
    if (seek_fn != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_fn->second) == "true", "SEEK() should return true for a match");
    }
    if (seek_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_rec->second) == "2", "SEEK() should move the record pointer to the matching row");
    }
    if (index_no_move != state.globals.end()) {
        expect(copperfin::runtime::format_value(index_no_move->second) == "true", "INDEXSEEK(.F.) should report matches");
    }
    if (after_no_move != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_no_move->second) == "1", "INDEXSEEK(.F.) should not move the record pointer");
    }
    if (index_move != state.globals.end()) {
        expect(copperfin::runtime::format_value(index_move->second) == "true", "INDEXSEEK(.T.) should report matches");
    }
    if (after_move != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_move->second) == "3", "INDEXSEEK(.T.) should move the record pointer to the match");
    }

    fs::remove_all(temp_root, ignored);
}

void test_seek_function_accepts_direction_suffix_in_order_designator() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_seek_direction_suffix";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    write_simple_dbf(table_path, {"ALPHA", "CHARLIE", "ECHO"});
    write_synthetic_cdx(cdx_path, "NAME", "UPPER(NAME)");

    const fs::path main_path = temp_root / "seek_direction_suffix.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET NEAR ON\n"
        "GO TOP\n"
        "lAscFound = SEEK('BRAVO', 'People', 'NAME')\n"
        "nAscRec = RECNO()\n"
        "GO TOP\n"
        "lDescFound = SEEK('BRAVO', 'People', 'NAME DESCENDING')\n"
        "nDescRec = RECNO()\n"
        "cOrderAfter = ORDER()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SEEK() direction-suffix script should complete");

    const auto asc_found = state.globals.find("lascfound");
    const auto asc_rec = state.globals.find("nascrec");
    const auto desc_found = state.globals.find("ldescfound");
    const auto desc_rec = state.globals.find("ndescrec");
    const auto order_after = state.globals.find("corderafter");

    expect(asc_found != state.globals.end(), "ascending SEEK() result should be captured");
    expect(asc_rec != state.globals.end(), "ascending SEEK() RECNO() should be captured");
    expect(desc_found != state.globals.end(), "descending SEEK() result should be captured");
    expect(desc_rec != state.globals.end(), "descending SEEK() RECNO() should be captured");
    expect(order_after != state.globals.end(), "ORDER() after SEEK() probes should be captured");

    if (asc_found != state.globals.end()) {
        expect(copperfin::runtime::format_value(asc_found->second) == "false", "ascending SEEK() should report a miss for an in-between key");
    }
    if (asc_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(asc_rec->second) == "2", "ascending SEEK() should move to the next row in ascending order");
    }
    if (desc_found != state.globals.end()) {
        expect(copperfin::runtime::format_value(desc_found->second) == "false", "descending SEEK() should report a miss for an in-between key");
    }
    if (desc_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(desc_rec->second) == "1", "descending SEEK() should move to the next row in descending order");
    }
    if (order_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(order_after->second).empty(), "SEEK() order-designator override should not permanently change ORDER()");
    }

    fs::remove_all(temp_root, ignored);
}

void test_order_and_tag_preserve_index_file_identity() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_idx_identity";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path idx_path = temp_root / "people.idx";
    write_simple_dbf(table_path, {"ALPHA", "BRAVO", "CHARLIE"});
    write_synthetic_idx(idx_path, "UPPER(NAME)");

    const fs::path main_path = temp_root / "idx_identity.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO 1\n"
        "cOrderPath = ORDER('People', 1)\n"
        "cTagFromIdx = TAG('" + idx_path.string() + "', 1, 'People')\n"
        "lSeek = SEEK('CHARLIE', 'People')\n"
        "nSeekRec = RECNO()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "idx identity script should complete");

    const auto order_path = state.globals.find("corderpath");
    const auto tag_from_idx = state.globals.find("ctagfromidx");
    const auto seek_value = state.globals.find("lseek");
    const auto seek_rec = state.globals.find("nseekrec");

    expect(order_path != state.globals.end(), "ORDER(alias, pathFlag) should be captured for IDX-backed orders");
    expect(tag_from_idx != state.globals.end(), "TAG(indexFile, tagNumber, alias) should be captured for IDX-backed orders");
    expect(seek_value != state.globals.end(), "SEEK() should be captured for IDX-backed orders");
    expect(seek_rec != state.globals.end(), "RECNO() after IDX-backed SEEK() should be captured");

    if (order_path != state.globals.end()) {
        expect(
            copperfin::runtime::format_value(order_path->second).find("PEOPLE.IDX") != std::string::npos,
            "ORDER(alias, pathFlag) should preserve the actual IDX file identity");
    }
    if (tag_from_idx != state.globals.end()) {
        expect(
            copperfin::runtime::format_value(tag_from_idx->second) == "PEOPLE",
            "TAG(indexFile, tagNumber, alias) should resolve the order from the actual IDX file");
    }
    if (seek_value != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_value->second) == "true", "SEEK() should work with the loaded IDX order");
    }
    if (seek_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_rec->second) == "3", "SEEK() should move to the matching record when using the IDX order");
    }

    fs::remove_all(temp_root, ignored);
}

void test_seek_respects_grounded_order_for_expression_hints() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_order_for_expression";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path idx_path = temp_root / "people.idx";
    write_simple_dbf(table_path, {"ALPHA", "BRAVO", "CHARLIE"});
    mark_simple_dbf_record_deleted(table_path, 2U);
    write_synthetic_idx_with_for(idx_path, "UPPER(NAME)", "DELETED() = .F.");

    const fs::path main_path = temp_root / "order_for_expression.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO 1\n"
        "SEEK 'BRAVO'\n"
        "lDeletedFound = FOUND()\n"
        "lDeletedEof = EOF()\n"
        "nDeletedRec = RECNO()\n"
        "SET NEAR ON\n"
        "GO TOP\n"
        "SEEK 'BRAVO'\n"
        "lNearFound = FOUND()\n"
        "nNearRec = RECNO()\n"
        "SEEK 'CHARLIE'\n"
        "lVisibleFound = FOUND()\n"
        "nVisibleRec = RECNO()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "FOR-filtered order SEEK script should complete");

    const auto deleted_found = state.globals.find("ldeletedfound");
    const auto deleted_eof = state.globals.find("ldeletedeof");
    const auto deleted_rec = state.globals.find("ndeletedrec");
    const auto near_found = state.globals.find("lnearfound");
    const auto near_rec = state.globals.find("nnearrec");
    const auto visible_found = state.globals.find("lvisiblefound");
    const auto visible_rec = state.globals.find("nvisiblerec");

    expect(deleted_found != state.globals.end(), "SEEK on a filtered-out key should expose FOUND()");
    expect(deleted_eof != state.globals.end(), "SEEK on a filtered-out key should expose EOF()");
    expect(deleted_rec != state.globals.end(), "SEEK on a filtered-out key should expose RECNO()");
    expect(near_found != state.globals.end(), "SET NEAR SEEK on a filtered-out key should expose FOUND()");
    expect(near_rec != state.globals.end(), "SET NEAR SEEK on a filtered-out key should expose RECNO()");
    expect(visible_found != state.globals.end(), "SEEK on a visible key should expose FOUND()");
    expect(visible_rec != state.globals.end(), "SEEK on a visible key should expose RECNO()");

    if (deleted_found != state.globals.end()) {
        expect(copperfin::runtime::format_value(deleted_found->second) == "false", "SEEK should ignore keys filtered out by the grounded order FOR expression");
    }
    if (deleted_eof != state.globals.end()) {
        expect(copperfin::runtime::format_value(deleted_eof->second) == "true", "SEEK without SET NEAR should move to EOF when only a filtered-out key matches");
    }
    if (deleted_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(deleted_rec->second) == "4", "SEEK without SET NEAR should position after the visible rows when the filtered-out key is skipped");
    }
    if (near_found != state.globals.end()) {
        expect(copperfin::runtime::format_value(near_found->second) == "false", "SET NEAR should still report a miss for a filtered-out key");
    }
    if (near_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(near_rec->second) == "3", "SET NEAR should move to the next visible indexed key after a filtered-out match");
    }
    if (visible_found != state.globals.end()) {
        expect(copperfin::runtime::format_value(visible_found->second) == "true", "SEEK should still find keys allowed by the grounded order FOR expression");
    }
    if (visible_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(visible_rec->second) == "3", "SEEK should position on the visible row that survives the order FOR expression");
    }

    fs::remove_all(temp_root, ignored);
}


void test_ndx_numeric_domain_guides_seek_near_ordering() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_ndx_numeric_domain";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path ndx_path = temp_root / "people.ndx";
    write_people_dbf(table_path, {{"ALPHA", 2}, {"BRAVO", 10}, {"CHARLIE", 20}});
    write_synthetic_ndx(ndx_path, "AGE", true);

    const fs::path main_path = temp_root / "ndx_numeric_domain.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO 1\n"
        "SET NEAR ON\n"
        "SEEK '9'\n"
        "lFound = FOUND()\n"
        "lEof = EOF()\n"
        "nRec = RECNO()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "NDX numeric-domain seek script should complete");

    const auto found = state.globals.find("lfound");
    const auto eof = state.globals.find("leof");
    const auto rec = state.globals.find("nrec");

    expect(found != state.globals.end(), "NDX numeric-domain seek should expose FOUND()");
    expect(eof != state.globals.end(), "NDX numeric-domain seek should expose EOF()");
    expect(rec != state.globals.end(), "NDX numeric-domain seek should expose RECNO()");

    if (found != state.globals.end()) {
        expect(copperfin::runtime::format_value(found->second) == "false", "NDX numeric-domain seek should still report a miss for a non-existent key");
    }
    if (eof != state.globals.end()) {
        expect(copperfin::runtime::format_value(eof->second) == "false", "NDX numeric-domain SET NEAR should position to the next numeric key instead of EOF");
    }
    if (rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(rec->second) == "2", "NDX numeric-domain seek should treat AGE keys numerically when choosing the nearest record");
    }

    fs::remove_all(temp_root, ignored);
}

void test_set_near_is_scoped_by_data_session() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_set_near_datasession";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    write_simple_dbf(table_path, {"ALPHA", "CHARLIE", "ECHO"});
    write_synthetic_cdx(cdx_path, "NAME", "UPPER(NAME)");

    const fs::path main_path = temp_root / "set_near_datasession.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO TAG NAME\n"
        "SET NEAR ON\n"
        "SEEK 'BRAVO'\n"
        "lNear1Found = FOUND()\n"
        "lNear1Eof = EOF()\n"
        "nNear1Rec = RECNO()\n"
        "SET DATASESSION TO 2\n"
        "USE '" + table_path.string() + "' ALIAS PeopleTwo IN 0\n"
        "SET ORDER TO TAG NAME\n"
        "SEEK 'BRAVO'\n"
        "lNear2Found = FOUND()\n"
        "lNear2Eof = EOF()\n"
        "nNear2Rec = RECNO()\n"
        "SET DATASESSION TO 1\n"
        "GO TOP\n"
        "SEEK 'BRAVO'\n"
        "lNear1BackFound = FOUND()\n"
        "lNear1BackEof = EOF()\n"
        "nNear1BackRec = RECNO()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SET NEAR data-session script should complete");

    const auto near1_found = state.globals.find("lnear1found");
    const auto near1_eof = state.globals.find("lnear1eof");
    const auto near1_rec = state.globals.find("nnear1rec");
    const auto near2_found = state.globals.find("lnear2found");
    const auto near2_eof = state.globals.find("lnear2eof");
    const auto near2_rec = state.globals.find("nnear2rec");
    const auto near1_back_found = state.globals.find("lnear1backfound");
    const auto near1_back_eof = state.globals.find("lnear1backeof");
    const auto near1_back_rec = state.globals.find("nnear1backrec");

    expect(near1_found != state.globals.end(), "session-1 SET NEAR FOUND() should be captured");
    expect(near1_eof != state.globals.end(), "session-1 SET NEAR EOF() should be captured");
    expect(near1_rec != state.globals.end(), "session-1 SET NEAR RECNO() should be captured");
    expect(near2_found != state.globals.end(), "session-2 SEEK FOUND() should be captured");
    expect(near2_eof != state.globals.end(), "session-2 SEEK EOF() should be captured");
    expect(near2_rec != state.globals.end(), "session-2 SEEK RECNO() should be captured");
    expect(near1_back_found != state.globals.end(), "restored session-1 SEEK FOUND() should be captured");
    expect(near1_back_eof != state.globals.end(), "restored session-1 SEEK EOF() should be captured");
    expect(near1_back_rec != state.globals.end(), "restored session-1 SEEK RECNO() should be captured");

    if (near1_found != state.globals.end()) {
        expect(copperfin::runtime::format_value(near1_found->second) == "false", "SET NEAR ON should still leave FOUND() false on a missed seek");
    }
    if (near1_eof != state.globals.end()) {
        expect(copperfin::runtime::format_value(near1_eof->second) == "false", "SET NEAR ON in session 1 should keep the cursor off EOF");
    }
    if (near1_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(near1_rec->second) == "2", "SET NEAR ON in session 1 should move to the nearest ordered row");
    }
    if (near2_found != state.globals.end()) {
        expect(copperfin::runtime::format_value(near2_found->second) == "false", "a fresh second data session should still report a missed seek");
    }
    if (near2_eof != state.globals.end()) {
        expect(copperfin::runtime::format_value(near2_eof->second) == "true", "SET NEAR should not bleed into a fresh second data session");
    }
    if (near2_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(near2_rec->second) == "4", "a fresh second data session should keep the default SET NEAR OFF seek position");
    }
    if (near1_back_found != state.globals.end()) {
        expect(copperfin::runtime::format_value(near1_back_found->second) == "false", "restoring session 1 should preserve missed-seek FOUND() behavior");
    }
    if (near1_back_eof != state.globals.end()) {
        expect(copperfin::runtime::format_value(near1_back_eof->second) == "false", "restoring session 1 should restore its SET NEAR ON behavior");
    }
    if (near1_back_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(near1_back_rec->second) == "2", "restoring session 1 should restore its nearest-record seek position");
    }

    fs::remove_all(temp_root, ignored);
}

void test_foxtools_registration_and_call_bridge() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_foxtools";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "foxtools.prg";
    write_text(
        main_path,
        "SET LIBRARY TO 'Foxtools'\n"
        "cFoxTools = FoxToolVer()\n"
        "nMain = MainHwnd()\n"
        "hPid = RegFn32('GetCurrentProcessId', '', 'I', 'kernel32.dll')\n"
        "nPid = CallFn(hPid)\n"
        "hLen = RegFn32('lstrlenA', 'C', 'I', 'kernel32.dll')\n"
        "nLen = CallFn(hLen, 'Copperfin')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "Foxtools bridge script should complete");

    const auto foxtools = state.globals.find("cfoxtools");
    const auto main = state.globals.find("nmain");
    const auto hpid = state.globals.find("hpid");
    const auto pid = state.globals.find("npid");
    const auto hlen = state.globals.find("hlen");
    const auto length = state.globals.find("nlen");

    expect(foxtools != state.globals.end(), "FoxToolVer() should be captured");
    expect(main != state.globals.end(), "MainHwnd() should be captured");
    expect(hpid != state.globals.end(), "RegFn32 handle should be captured");
    expect(pid != state.globals.end(), "CallFn(handle) should be captured");
    expect(hlen != state.globals.end(), "second RegFn32 handle should be captured");
    expect(length != state.globals.end(), "CallFn(string) should be captured");

    if (foxtools != state.globals.end()) {
        expect(!copperfin::runtime::format_value(foxtools->second).empty(), "FoxToolVer() should return a non-empty version string");
    }
    if (main != state.globals.end()) {
        expect(copperfin::runtime::format_value(main->second) == "1001", "MainHwnd() should expose the placeholder host window handle");
    }
    if (hpid != state.globals.end()) {
        expect(copperfin::runtime::format_value(hpid->second) == "1", "first RegFn32 call should allocate handle 1");
    }
    if (pid != state.globals.end()) {
        expect(copperfin::runtime::format_value(pid->second) != "0", "CallFn(GetCurrentProcessId) should return a non-zero process id");
    }
    if (hlen != state.globals.end()) {
        expect(copperfin::runtime::format_value(hlen->second) == "2", "second RegFn32 call should allocate handle 2");
    }
    if (length != state.globals.end()) {
        expect(copperfin::runtime::format_value(length->second) == "9", "CallFn(lstrlenA, 'Copperfin') should return the string length");
    }

    expect(
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.library"; }) &&
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "interop.regfn"; }) &&
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "interop.callfn"; }),
        "Foxtools bridge should emit library, registration, and call events");

    fs::remove_all(temp_root, ignored);
}

void test_foxtools_registration_is_scoped_by_data_session() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_foxtools_datasession";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "foxtools_datasession.prg";
    write_text(
        main_path,
        "SET LIBRARY TO 'Foxtools'\n"
        "hPid1 = RegFn32('GetCurrentProcessId', '', 'I', 'kernel32.dll')\n"
        "SET DATASESSION TO 2\n"
        "SET LIBRARY TO 'Foxtools'\n"
        "nCrossCall = CallFn(hPid1)\n"
        "hLen2 = RegFn32('lstrlenA', 'C', 'I', 'kernel32.dll')\n"
        "nLen2 = CallFn(hLen2, 'AB')\n"
        "SET DATASESSION TO 1\n"
        "nPid1Back = CallFn(hPid1)\n"
        "hLen1Back = RegFn32('lstrlenA', 'C', 'I', 'kernel32.dll')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "Foxtools data-session script should complete");

    const auto hpid1 = state.globals.find("hpid1");
    const auto cross_call = state.globals.find("ncrosscall");
    const auto hlen2 = state.globals.find("hlen2");
    const auto len2 = state.globals.find("nlen2");
    const auto pid1_back = state.globals.find("npid1back");
    const auto hlen1_back = state.globals.find("hlen1back");

    expect(hpid1 != state.globals.end(), "session-1 RegFn32 handle should be captured");
    expect(cross_call != state.globals.end(), "cross-session CallFn result should be captured");
    expect(hlen2 != state.globals.end(), "session-2 RegFn32 handle should be captured");
    expect(len2 != state.globals.end(), "session-2 CallFn result should be captured");
    expect(pid1_back != state.globals.end(), "restored session-1 CallFn result should be captured");
    expect(hlen1_back != state.globals.end(), "restored session-1 RegFn32 handle should be captured");

    if (hpid1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(hpid1->second) == "1", "the first RegFn32 handle in session 1 should be 1");
    }
    if (cross_call != state.globals.end()) {
        expect(copperfin::runtime::format_value(cross_call->second) == "-1", "CallFn should reject a RegFn32 handle from another data session");
    }
    if (hlen2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(hlen2->second) == "1", "the first RegFn32 handle in a fresh second data session should restart at 1");
    }
    if (len2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(len2->second) == "2", "session-2 CallFn should use its own registered handle");
    }
    if (pid1_back != state.globals.end()) {
        expect(copperfin::runtime::format_value(pid1_back->second) != "0", "restoring session 1 should restore its RegFn32 handle lookup");
    }
    if (hlen1_back != state.globals.end()) {
        expect(copperfin::runtime::format_value(hlen1_back->second) == "2", "restoring session 1 should restore its next RegFn32 handle allocation");
    }

    fs::remove_all(temp_root, ignored);
}

void test_set_exact_affects_comparisons_and_seek() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_set_exact";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    write_simple_dbf(table_path, {"ALPHA", "BRAVO", "CHARLIE"});
    write_synthetic_cdx(cdx_path, "NAME", "UPPER(NAME)");

    const fs::path main_path = temp_root / "set_exact.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO TAG NAME\n"
        "lEqOff = 'CHARLIE' = 'CHAR'\n"
        "lSeekOff = SEEK('BR')\n"
        "nRecOff = RECNO()\n"
        "SET EXACT ON\n"
        "lEqOn = 'CHARLIE' = 'CHAR'\n"
        "lSeekOn = SEEK('BR')\n"
        "lEofOn = EOF()\n"
        "SET DATASESSION TO 2\n"
        "lEqSession2 = 'CHARLIE' = 'CHAR'\n"
        "SET DATASESSION TO 1\n"
        "lEqBack = 'CHARLIE' = 'CHAR'\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SET EXACT script should complete");

    const auto eq_off = state.globals.find("leqoff");
    const auto seek_off = state.globals.find("lseekoff");
    const auto rec_off = state.globals.find("nrecoff");
    const auto eq_on = state.globals.find("leqon");
    const auto seek_on = state.globals.find("lseekon");
    const auto eof_on = state.globals.find("leofon");
    const auto eq_session2 = state.globals.find("leqsession2");
    const auto eq_back = state.globals.find("leqback");

    expect(eq_off != state.globals.end(), "SET EXACT OFF comparison result should be captured");
    expect(seek_off != state.globals.end(), "SET EXACT OFF seek result should be captured");
    expect(rec_off != state.globals.end(), "SET EXACT OFF RECNO() should be captured");
    expect(eq_on != state.globals.end(), "SET EXACT ON comparison result should be captured");
    expect(seek_on != state.globals.end(), "SET EXACT ON seek result should be captured");
    expect(eof_on != state.globals.end(), "SET EXACT ON EOF() should be captured");
    expect(eq_session2 != state.globals.end(), "session 2 comparison result should be captured");
    expect(eq_back != state.globals.end(), "session 1 restored comparison result should be captured");

    if (eq_off != state.globals.end()) {
        expect(copperfin::runtime::format_value(eq_off->second) == "true", "SET EXACT OFF should allow right-side prefix string comparison");
    }
    if (seek_off != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_off->second) == "true", "SET EXACT OFF should allow prefix seeks");
    }
    if (rec_off != state.globals.end()) {
        expect(copperfin::runtime::format_value(rec_off->second) == "2", "SET EXACT OFF seek should land on the matching prefix row");
    }
    if (eq_on != state.globals.end()) {
        expect(copperfin::runtime::format_value(eq_on->second) == "false", "SET EXACT ON should require full string equality");
    }
    if (seek_on != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_on->second) == "false", "SET EXACT ON should reject prefix seeks");
    }
    if (eof_on != state.globals.end()) {
        expect(copperfin::runtime::format_value(eof_on->second) == "true", "SET EXACT ON failed seek should leave the cursor at EOF");
    }
    if (eq_session2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(eq_session2->second) == "true", "SET EXACT should be scoped to the current data session");
    }
    if (eq_back != state.globals.end()) {
        expect(copperfin::runtime::format_value(eq_back->second) == "false", "restoring the original data session should restore its SET EXACT state");
    }

    fs::remove_all(temp_root, ignored);
}

void test_use_again_and_alias_collision_semantics() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_use_again";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_simple_dbf(table_path, {"ALPHA", "BRAVO"});

    const fs::path main_path = temp_root / "use_again.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "USE '" + table_path.string() + "' ALIAS PeopleAgain AGAIN IN 0\n"
        "nAreaAgain = SELECT()\n"
        "cAliasAgain = ALIAS()\n"
        "USE '" + table_path.string() + "' ALIAS PeopleThird IN 0\n"
        "xAfterError = 7\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error, "duplicate table opens without AGAIN should pause with an error");
    expect(state.location.line == 5U, "duplicate table opens without AGAIN should highlight the failing USE statement");
    expect(
        state.message.find("USE AGAIN is required") != std::string::npos,
        "duplicate table opens without AGAIN should report a USE AGAIN message");

    const auto area_again = state.globals.find("nareaagain");
    const auto alias_again = state.globals.find("caliasagain");
    expect(area_again != state.globals.end(), "USE AGAIN should let execution reach the second-area checks");
    expect(alias_again != state.globals.end(), "USE AGAIN should let execution expose the second alias");
    if (area_again != state.globals.end()) {
        expect(copperfin::runtime::format_value(area_again->second) == "2", "USE AGAIN IN 0 should allocate a second work area");
    }
    if (alias_again != state.globals.end()) {
        expect(copperfin::runtime::format_value(alias_again->second) == "PeopleAgain", "USE AGAIN should keep the requested second alias");
    }

    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "continuing after a USE AGAIN error should keep the host alive");
    const auto after_error = state.globals.find("xaftererror");
    expect(after_error != state.globals.end(), "post-error statements should still run after continuing");
    if (after_error != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_error->second) == "7", "post-error statements should still update globals");
    }

    fs::remove_all(temp_root, ignored);
}

void test_use_in_selected_alias_replacement_clears_old_alias_and_order_state() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_selected_alias_replacement";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path people_path = temp_root / "people.dbf";
    const fs::path cities_path = temp_root / "cities.dbf";
    const fs::path people_idx_path = temp_root / "people.idx";
    write_simple_dbf(people_path, {"ALPHA", "BRAVO"});
    write_simple_dbf(cities_path, {"OSLO", "ROME"});
    write_synthetic_idx(people_idx_path, "UPPER(NAME)");

    const fs::path main_path = temp_root / "selected_alias_replacement.prg";
    write_text(
        main_path,
        "USE '" + people_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO 1\n"
        "GO BOTTOM\n"
        "USE '" + cities_path.string() + "' ALIAS Cities IN People\n"
        "nOldAliasArea = SELECT('People')\n"
        "nNewAliasArea = SELECT('Cities')\n"
        "cAliasAfter = ALIAS()\n"
        "cOrderAfter = ORDER()\n"
        "nRecAfter = RECNO()\n"
        "cTopAfter = NAME\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "selected alias replacement script should complete");

    const auto old_alias_area = state.globals.find("noldaliasarea");
    const auto new_alias_area = state.globals.find("nnewaliasarea");
    const auto alias_after = state.globals.find("caliasafter");
    const auto order_after = state.globals.find("corderafter");
    const auto rec_after = state.globals.find("nrecafter");
    const auto top_after = state.globals.find("ctopafter");

    expect(old_alias_area != state.globals.end(), "SELECT('People') after selected alias replacement should be captured");
    expect(new_alias_area != state.globals.end(), "SELECT('Cities') after selected alias replacement should be captured");
    expect(alias_after != state.globals.end(), "ALIAS() after selected alias replacement should be captured");
    expect(order_after != state.globals.end(), "ORDER() after selected alias replacement should be captured");
    expect(rec_after != state.globals.end(), "RECNO() after selected alias replacement should be captured");
    expect(top_after != state.globals.end(), "field access after selected alias replacement should be captured");

    if (old_alias_area != state.globals.end()) {
        expect(copperfin::runtime::format_value(old_alias_area->second) == "0", "selected alias replacement should clear the old alias lookup");
    }
    if (new_alias_area != state.globals.end()) {
        expect(copperfin::runtime::format_value(new_alias_area->second) == "1", "selected alias replacement should reuse the selected work area in place");
    }
    if (alias_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(alias_after->second) == "Cities", "selected alias replacement should expose the new alias immediately");
    }
    if (order_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(order_after->second).empty(), "selected alias replacement should clear the old active order state when the replacement has no orders");
    }
    if (rec_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(rec_after->second) == "1", "selected alias replacement should reset the cursor position for the new table");
    }
    if (top_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(top_after->second) == "OSLO", "selected alias replacement should expose the new table's first record");
    }

    fs::remove_all(temp_root, ignored);
}


void test_select_missing_alias_is_an_error() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_select_missing_alias";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "select_missing.prg";
    write_text(
        main_path,
        "SELECT MissingAlias\n"
        "xAfterError = 9\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error, "SELECT MissingAlias should pause with an error");
    expect(state.location.line == 1U, "SELECT MissingAlias should highlight the failing line");
    expect(
        state.message.find("SELECT target work area not found") != std::string::npos,
        "SELECT MissingAlias should report a missing-target message");

    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "continuing after a SELECT error should keep the host alive");
    const auto after_error = state.globals.find("xaftererror");
    expect(after_error != state.globals.end(), "post-error statements should still run after SELECT errors");
    if (after_error != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_error->second) == "9", "post-error statements should still update globals after SELECT errors");
    }

    fs::remove_all(temp_root, ignored);
}

void test_use_in_missing_alias_is_an_error() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_use_in_missing_alias";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_simple_dbf(table_path, {"ALPHA", "BRAVO"});

    const fs::path main_path = temp_root / "use_in_missing_alias.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN MissingAlias\n"
        "xAfterError = 11\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error, "USE ... IN MissingAlias should pause with an error");
    expect(state.location.line == 1U, "USE ... IN MissingAlias should highlight the failing line");
    expect(
        state.message.find("USE target work area not found") != std::string::npos,
        "USE ... IN MissingAlias should report a missing-target message");

    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "continuing after a USE ... IN alias error should keep the host alive");
    const auto after_error = state.globals.find("xaftererror");
    expect(after_error != state.globals.end(), "post-error statements should still run after USE ... IN alias errors");
    if (after_error != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_error->second) == "11", "post-error statements should still update globals after USE ... IN alias errors");
    }
    expect(state.work_area.aliases.empty(), "USE ... IN MissingAlias should not open a fallback work area");

    fs::remove_all(temp_root, ignored);
}

void test_runtime_fault_containment() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_faults";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "faults.prg";
    write_text(
        main_path,
        "x = 'abc' - 1\n"
        "y = 7\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error, "broken code should pause with an error instead of killing the host");
    expect(state.location.line == 1U, "runtime faults should highlight the faulting line");
    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.error"; }),
        "runtime faults should emit a runtime.error event");

    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "continuing after a trapped runtime error should keep the session alive");
    const auto y = state.globals.find("y");
    expect(y != state.globals.end(), "post-fault statements should still be able to run");
    if (y != state.globals.end()) {
        expect(copperfin::runtime::format_value(y->second) == "7", "post-fault statements should update globals");
    }

    fs::remove_all(temp_root, ignored);
}

void test_set_filter_scopes_local_cursor_visibility() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_set_filter";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}, {"DELTA", 40}});

    const fs::path main_path = temp_root / "set_filter.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "USE '" + table_path.string() + "' ALIAS Other AGAIN IN 0\n"
        "SELECT People\n"
        "SET FILTER TO AGE >= 30\n"
        "GO TOP\n"
        "cTopFiltered = NAME\n"
        "SKIP\n"
        "cNextFiltered = NAME\n"
        "SKIP\n"
        "lFilteredEof = EOF()\n"
        "LOCATE FOR NAME = 'BRAVO'\n"
        "lFilteredFound = FOUND()\n"
        "lFilteredLocateEof = EOF()\n"
        "SELECT Other\n"
        "GO TOP\n"
        "cOtherTop = NAME\n"
        "SELECT People\n"
        "SET FILTER OFF\n"
        "GO TOP\n"
        "cTopUnfiltered = NAME\n"
        "LOCATE FOR NAME = 'BRAVO'\n"
        "cLocateUnfiltered = NAME\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SET FILTER script should complete");

    const auto top_filtered = state.globals.find("ctopfiltered");
    const auto next_filtered = state.globals.find("cnextfiltered");
    const auto filtered_eof = state.globals.find("lfilteredeof");
    const auto filtered_found = state.globals.find("lfilteredfound");
    const auto filtered_locate_eof = state.globals.find("lfilteredlocateeof");
    const auto other_top = state.globals.find("cothertop");
    const auto top_unfiltered = state.globals.find("ctopunfiltered");
    const auto locate_unfiltered = state.globals.find("clocateunfiltered");

    expect(top_filtered != state.globals.end(), "filtered GO TOP should expose the first visible record");
    expect(next_filtered != state.globals.end(), "filtered SKIP should expose the next visible record");
    expect(filtered_eof != state.globals.end(), "filtered SKIP past the last visible row should update EOF()");
    expect(filtered_found != state.globals.end(), "filtered LOCATE should expose FOUND()");
    expect(filtered_locate_eof != state.globals.end(), "filtered LOCATE miss should expose EOF()");
    expect(other_top != state.globals.end(), "filters should not bleed into a second alias/work area");
    expect(top_unfiltered != state.globals.end(), "SET FILTER OFF should restore unfiltered navigation");
    expect(locate_unfiltered != state.globals.end(), "SET FILTER OFF should restore unfiltered LOCATE behavior");

    if (top_filtered != state.globals.end()) {
        expect(copperfin::runtime::format_value(top_filtered->second) == "CHARLIE", "GO TOP should land on the first filtered-visible row");
    }
    if (next_filtered != state.globals.end()) {
        expect(copperfin::runtime::format_value(next_filtered->second) == "DELTA", "SKIP should move among filtered-visible rows");
    }
    if (filtered_eof != state.globals.end()) {
        expect(copperfin::runtime::format_value(filtered_eof->second) == "true", "SKIP past the filtered-visible tail should reach EOF");
    }
    if (filtered_found != state.globals.end()) {
        expect(copperfin::runtime::format_value(filtered_found->second) == "false", "LOCATE should not find rows excluded by the active filter");
    }
    if (filtered_locate_eof != state.globals.end()) {
        expect(copperfin::runtime::format_value(filtered_locate_eof->second) == "true", "LOCATE misses within a filtered set should leave the cursor at EOF");
    }
    if (other_top != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_top->second) == "ALPHA", "SET FILTER should remain scoped to the targeted cursor/work area");
    }
    if (top_unfiltered != state.globals.end()) {
        expect(copperfin::runtime::format_value(top_unfiltered->second) == "ALPHA", "SET FILTER OFF should restore full-table GO TOP semantics");
    }
    if (locate_unfiltered != state.globals.end()) {
        expect(copperfin::runtime::format_value(locate_unfiltered->second) == "BRAVO", "SET FILTER OFF should restore full-table LOCATE behavior");
    }

    expect(
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.filter" && event.detail.find("AGE >= 30") != std::string::npos; }) &&
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.filter" && event.detail == "OFF"; }),
        "SET FILTER changes should emit runtime.filter events");

    fs::remove_all(temp_root, ignored);
}

void test_set_filter_in_targets_nonselected_alias() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_set_filter_in";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}, {"DELTA", 40}});

    const fs::path main_path = temp_root / "set_filter_in.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "USE '" + table_path.string() + "' ALIAS Other AGAIN IN 0\n"
        "SELECT People\n"
        "cTargetAlias = 'Other'\n"
        "SET FILTER TO AGE >= 30 IN cTargetAlias\n"
        "GO TOP\n"
        "cPeopleTop = NAME\n"
        "SELECT Other\n"
        "GO TOP\n"
        "cOtherTop = NAME\n"
        "SET FILTER OFF IN cTargetAlias\n"
        "GO TOP\n"
        "cOtherTopUnfiltered = NAME\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SET FILTER ... IN script should complete");

    const auto people_top = state.globals.find("cpeopletop");
    const auto other_top = state.globals.find("cothertop");
    const auto other_top_unfiltered = state.globals.find("cothertopunfiltered");

    expect(people_top != state.globals.end(), "selected alias top row should be captured");
    expect(other_top != state.globals.end(), "targeted alias filtered row should be captured");
    expect(other_top_unfiltered != state.globals.end(), "targeted alias unfiltered row should be captured");

    if (people_top != state.globals.end()) {
        expect(copperfin::runtime::format_value(people_top->second) == "ALPHA", "SET FILTER ... IN cTargetAlias should not affect People");
    }
    if (other_top != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_top->second) == "CHARLIE", "SET FILTER ... IN cTargetAlias should affect the targeted alias");
    }
    if (other_top_unfiltered != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_top_unfiltered->second) == "ALPHA", "SET FILTER OFF IN cTargetAlias should restore unfiltered navigation for the targeted alias");
    }

    expect(
        std::count_if(state.events.begin(), state.events.end(), [](const auto& event) {
            return event.category == "runtime.filter";
        }) >= 2,
        "SET FILTER ... IN and SET FILTER OFF IN should emit runtime.filter events");

    fs::remove_all(temp_root, ignored);
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

    expect(
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.calculate"; }),
        "CALCULATE should emit runtime.calculate events");

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
    const auto sum_rest = state.globals.find("nsumrest");
    const auto sum_next_three = state.globals.find("nsumnextthree");
    const auto sum_double_next_three = state.globals.find("nsumdoublenextthree");
    const auto avg_while = state.globals.find("navgwhile");
    const auto people_rec = state.globals.find("npeoplerec");

    expect(count_rest != state.globals.end(), "COUNT REST should assign into a variable");
    expect(count_next_two != state.globals.end(), "COUNT NEXT should assign into a variable");
    expect(count_record_four != state.globals.end(), "COUNT RECORD should assign into a variable");
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
    expect(
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.total"; }),
        "TOTAL should emit runtime.total events");

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
    expect(
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.total"; }),
        "TOTAL for SQL cursors should emit runtime.total events");

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

    const fs::path main_path = temp_root / "aerror.prg";
    write_text(
        main_path,
        "ON ERROR DO handleerr\n"
        "DO missing_target\n"
        "after_error = 'continued'\n"
        "RETURN\n"
        "PROCEDURE handleerr\n"
        "nErrorRows = AERROR(aErr)\n"
        "nErrorCols = ALEN(aErr, 2)\n"
        "nErrorCode = aErr[1,1]\n"
        "cErrorMessage = aErr[1,2]\n"
        "cErrorParam = aErr[1,3]\n"
        "nErrorWorkArea = aErr[1,4]\n"
        "cTriggerInfo = VARTYPE(aErr[1,5])\n"
        "cReservedOne = VARTYPE(aErr[1,6])\n"
        "cReservedTwo = VARTYPE(aErr[1,7])\n"
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
    const auto trigger_info = state.globals.find("ctriggerinfo");
    const auto reserved_one = state.globals.find("creservedone");
    const auto reserved_two = state.globals.find("creservedtwo");
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
    expect(trigger_info != state.globals.end(), "AERROR() should expose an empty trigger-info column for normal runtime errors");
    expect(reserved_one != state.globals.end(), "AERROR() should expose an empty reserved column");
    expect(reserved_two != state.globals.end(), "AERROR() should expose a second empty reserved column");
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
            "AERROR() work-area column should capture the selected work area");
    }
    if (trigger_info != state.globals.end()) {
        expect(copperfin::runtime::format_value(trigger_info->second) == "U",
            "AERROR() trigger-info column should be empty for non-trigger runtime errors");
    }
    if (reserved_one != state.globals.end()) {
        expect(copperfin::runtime::format_value(reserved_one->second) == "U",
            "AERROR() first reserved column should be empty for normal runtime errors");
    }
    if (reserved_two != state.globals.end()) {
        expect(copperfin::runtime::format_value(reserved_two->second) == "U",
            "AERROR() second reserved column should be empty for normal runtime errors");
    }
    if (sys2018 != state.globals.end()) {
        expect(copperfin::runtime::format_value(sys2018->second) == "MISSING_TARGET",
            "SYS(2018) should expose the uppercase runtime error parameter");
    }
    if (line != state.globals.end()) {
        expect(copperfin::runtime::format_value(line->second) == "2",
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
        "nSqlResult = SQLEXEC(999, 'select * from missing')\n"
        "nSqlRows = AERROR(aSqlErr)\n"
        "nSqlCode = aSqlErr[1,1]\n"
        "cSqlMessage = aSqlErr[1,2]\n"
        "cSqlDetail = aSqlErr[1,3]\n"
        "cSqlState = aSqlErr[1,4]\n"
        "nSqlNative = aSqlErr[1,5]\n"
        "ON ERROR DO oleerr\n"
        "missingOle.SomeProperty = 42\n"
        "RETURN\n"
        "PROCEDURE oleerr\n"
        "nOleRows = AERROR(aOleErr)\n"
        "nOleCode = aOleErr[1,1]\n"
        "cOleMessage = aOleErr[1,2]\n"
        "cOleDetail = aOleErr[1,3]\n"
        "cOleApp = aOleErr[1,4]\n"
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
    const auto ole_rows = state.globals.find("nolerows");
    const auto ole_code = state.globals.find("nolecode");
    const auto ole_message = state.globals.find("colemessage");
    const auto ole_detail = state.globals.find("coledetail");
    const auto ole_app = state.globals.find("coleapp");
    const auto ole_native = state.globals.find("nolenative");

    expect(sql_rows != state.globals.end(), "SQL AERROR should return a row count");
    expect(sql_code != state.globals.end(), "SQL AERROR should populate code");
    expect(sql_message != state.globals.end(), "SQL AERROR should populate message");
    expect(sql_detail != state.globals.end(), "SQL AERROR should populate detail");
    expect(sql_state != state.globals.end(), "SQL AERROR should populate SQL state");
    expect(sql_native != state.globals.end(), "SQL AERROR should populate native code");
    expect(ole_rows != state.globals.end(), "OLE AERROR should return a row count");
    expect(ole_code != state.globals.end(), "OLE AERROR should populate code");
    expect(ole_message != state.globals.end(), "OLE AERROR should populate message");
    expect(ole_detail != state.globals.end(), "OLE AERROR should populate detail");
    expect(ole_app != state.globals.end(), "OLE AERROR should populate app name");
    expect(ole_native != state.globals.end(), "OLE AERROR should populate native code");

    if (sql_rows != state.globals.end()) {
        expect(copperfin::runtime::format_value(sql_rows->second) == "1", "SQL AERROR should expose one row");
    }
    if (sql_code != state.globals.end()) {
        expect(copperfin::runtime::format_value(sql_code->second) == "1526", "SQL failures should use the VFP ODBC error code");
    }
    if (sql_message != state.globals.end()) {
        expect(copperfin::runtime::format_value(sql_message->second).find("SQL handle not found") != std::string::npos,
            "SQL AERROR message should preserve SQL failure text");
    }
    if (sql_detail != state.globals.end()) {
        expect(copperfin::runtime::format_value(sql_detail->second) == "999",
            "SQL AERROR detail should preserve the failing handle parameter");
    }
    if (sql_state != state.globals.end()) {
        expect(copperfin::runtime::format_value(sql_state->second) == "HY000", "SQL AERROR should expose a generic SQL state");
    }
    if (sql_native != state.globals.end()) {
        expect(copperfin::runtime::format_value(sql_native->second) == "-1", "SQL AERROR should expose a native failure code");
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
    if (ole_native != state.globals.end()) {
        expect(copperfin::runtime::format_value(ole_native->second) == "1429",
            "OLE AERROR native column should preserve the automation error code");
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
            copperfin::runtime::format_value(prop_value->second) == "ole:Sample.Object.caption",
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

void test_string_and_math_expression_functions() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_str_math";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "str_math.prg";
    write_text(
        main_path,
        "l = LEN('hello')\n"
        "lft = LEFT('hello', 3)\n"
        "rgt = RIGHT('hello', 2)\n"
        "up = UPPER('hello')\n"
        "lo = LOWER('WORLD')\n"
        "sp = LEN(SPACE(5))\n"
        "repl = REPLICATE('ab', 3)\n"
        "trimmed = LTRIM('  hi  ')\n"
        "rtrimmed = RTRIM('  hi  ')\n"
        "a = ABS(-7)\n"
        "b = INT(3.9)\n"
        "c = MOD(10, 3)\n"
        "d = ROUND(3.567, 2)\n"
        "e = SIGN(-5)\n"
        "f = IIF(.T., 'yes', 'no')\n"
        "g = BETWEEN(5, 1, 10)\n"
        "h = OCCURS('l', 'hello world')\n"
        "v = VAL('42')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "string/math function script should complete");

    const auto check = [&](const std::string& name, const std::string& expected) {
        const auto it = state.globals.find(name);
        if (it == state.globals.end()) {
            expect(false, name + " variable not found");
            return;
        }
        expect(copperfin::runtime::format_value(it->second) == expected,
            name + " expected '" + expected + "' got '" + copperfin::runtime::format_value(it->second) + "'");
    };

    check("l", "5");
    check("lft", "hel");
    check("rgt", "lo");
    check("up", "HELLO");
    check("lo", "world");
    check("sp", "5");
    check("repl", "ababab");
    check("trimmed", "hi  ");
    check("rtrimmed", "  hi");
    check("a", "7");
    check("b", "3");
    check("c", "1");
    check("d", "3.57");
    check("e", "-1");
    check("f", "yes");
    check("g", "true");
    check("h", "3");
    check("v", "42");

    fs::remove_all(temp_root, ignored);
}

void test_type_and_null_expression_functions() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_type_null";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "type_null.prg";
    write_text(
        main_path,
        "x = 5\n"
        "t_num = VARTYPE(x)\n"
        "t_str = VARTYPE('hello')\n"
        "t_bool = VARTYPE(.T.)\n"
        "em = EMPTY('')\n"
        "em2 = EMPTY(0)\n"
        "not_em = EMPTY('hi')\n"
        "nvl_result = NVL('', 'fallback')\n"
        "nvl_ok = NVL('value', 'fallback')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "type/null function script should complete");

    const auto check = [&](const std::string& name, const std::string& expected) {
        const auto it = state.globals.find(name);
        if (it == state.globals.end()) {
            expect(false, name + " variable not found");
            return;
        }
        expect(copperfin::runtime::format_value(it->second) == expected,
            name + " expected '" + expected + "' got '" + copperfin::runtime::format_value(it->second) + "'");
    };

    check("t_num", "N");
    check("t_str", "C");
    check("t_bool", "L");
    check("em", "true");
    check("em2", "true");
    check("not_em", "false");
    check("nvl_result", "");  // NVL('','fallback') returns '' since '' is string, not .NULL.
    check("nvl_ok", "value");

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

void test_scatter_memvar_from_current_record() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_scatter";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_simple_dbf(temp_root / "people.dbf", {"Alice", "Bob"});

    const fs::path main_path = temp_root / "scatter_test.prg";
    write_text(
        main_path,
        "USE '" + (temp_root / "people.dbf").string() + "'\n"
        "GO 1\n"
        "SCATTER MEMVAR\n"
        "grabbed = m.NAME\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SCATTER MEMVAR script should complete");

    const auto grabbed = state.globals.find("grabbed");
    expect(grabbed != state.globals.end(), "grabbed variable should exist after SCATTER MEMVAR");
    if (grabbed != state.globals.end()) {
        const std::string val = copperfin::runtime::format_value(grabbed->second);
        expect(val.find("Alice") != std::string::npos || val == "Alice      ",
            "SCATTER MEMVAR should copy NAME field into m.NAME (got: '" + val + "')");
    }

    fs::remove_all(temp_root, ignored);
}

void test_scatter_gather_memvar_fields_blank_and_for_semantics() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_scatter_gather_memvar_fields";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .length = 10U},
        {.name = "AGE", .type = 'N', .length = 3U},
        {.name = "ACTIVE", .type = 'L', .length = 1U},
    };
    const std::vector<std::vector<std::string>> records{
        {"Alice", "42", "true"},
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "scatter/gather typed DBF fixture should be created");

    const fs::path main_path = temp_root / "scatter_gather_memvar_fields.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "'\n"
        "GO 1\n"
        "SCATTER FIELDS NAME, AGE MEMVAR\n"
        "cName = m.NAME\n"
        "nAgePlus = m.AGE + 1\n"
        "cActiveType = VARTYPE(m.ACTIVE)\n"
        "SCATTER FIELDS AGE, ACTIVE MEMVAR BLANK\n"
        "nBlankAge = m.AGE\n"
        "lBlankActive = m.ACTIVE\n"
        "m.NAME = 'Skipped'\n"
        "GATHER MEMVAR FIELDS NAME FOR .F.\n"
        "m.NAME = 'Updated'\n"
        "GATHER MEMVAR FIELDS NAME FOR .T.\n"
        "cAfterGather = NAME\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SCATTER/GATHER MEMVAR field-filter script should complete");

    const auto name = state.globals.find("cname");
    const auto age_plus = state.globals.find("nageplus");
    const auto active_type = state.globals.find("cactivetype");
    const auto blank_age = state.globals.find("nblankage");
    const auto blank_active = state.globals.find("lblankactive");
    const auto after_gather = state.globals.find("caftergather");
    const bool has_skipped_event = std::any_of(state.events.begin(), state.events.end(),
        [](const copperfin::runtime::RuntimeEvent& ev) {
            return ev.category == "runtime.gather" && ev.detail == "memvar skipped";
        });

    expect(name != state.globals.end(), "SCATTER FIELDS should populate selected NAME memvar");
    expect(age_plus != state.globals.end(), "SCATTER should preserve numeric AGE as a numeric memvar");
    expect(active_type != state.globals.end(), "SCATTER FIELDS should leave unselected ACTIVE undefined");
    expect(blank_age != state.globals.end(), "SCATTER BLANK should create a numeric blank AGE value");
    expect(blank_active != state.globals.end(), "SCATTER BLANK should create a logical blank ACTIVE value");
    expect(has_skipped_event, "GATHER FOR .F. should skip field replacement");
    expect(after_gather != state.globals.end(), "GATHER FOR .T. should update the selected field");

    if (name != state.globals.end()) {
        expect(copperfin::runtime::format_value(name->second) == "Alice",
            "SCATTER FIELDS should copy NAME into m.NAME");
    }
    if (age_plus != state.globals.end()) {
        expect(copperfin::runtime::format_value(age_plus->second) == "43",
            "SCATTER should expose numeric AGE as arithmetic-capable");
    }
    if (active_type != state.globals.end()) {
        expect(copperfin::runtime::format_value(active_type->second) == "U",
            "SCATTER FIELDS should not populate omitted ACTIVE memvar");
    }
    if (blank_age != state.globals.end()) {
        expect(copperfin::runtime::format_value(blank_age->second) == "0",
            "SCATTER BLANK should use numeric zero for numeric fields");
    }
    if (blank_active != state.globals.end()) {
        expect(copperfin::runtime::format_value(blank_active->second) == "false",
            "SCATTER BLANK should use false for logical fields");
    }
    if (after_gather != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_gather->second) == "Updated",
            "GATHER FOR .T. should apply field replacement");
    }
    const auto persisted = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 10U);
    expect(persisted.ok, "GATHER MEMVAR destination table should remain readable");
    if (persisted.ok && !persisted.table.records.empty() && !persisted.table.records[0U].values.empty()) {
        expect(persisted.table.records[0U].values[0U].display_value == "Updated",
            "only the FOR .T. GATHER should persist the NAME update");
    }

    fs::remove_all(temp_root, ignored);
}

void test_scatter_to_array_and_gather_from_array_round_trip() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_scatter_gather_array";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .length = 10U},
        {.name = "AGE", .type = 'N', .length = 3U},
    };
    const std::vector<std::vector<std::string>> records{
        {"Alice", "42"},
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "scatter/gather array DBF fixture should be created");

    const fs::path main_path = temp_root / "scatter_gather_array.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "'\n"
        "GO 1\n"
        "SCATTER FIELDS NAME, AGE TO aRow\n"
        "nArrayLen = ALEN(aRow)\n"
        "nRows = ALEN(aRow, 1)\n"
        "nCols = ALEN(aRow, 2)\n"
        "cFirst = aRow[1]\n"
        "nSecondPlus = aRow(2) + 1\n"
        "REPLACE NAME WITH 'Changed', AGE WITH 7\n"
        "GATHER FROM aRow FIELDS NAME, AGE\n"
        "cAfterName = NAME\n"
        "nAfterAge = AGE\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SCATTER TO array / GATHER FROM array script should complete");

    const auto array_len = state.globals.find("narraylen");
    const auto rows = state.globals.find("nrows");
    const auto cols = state.globals.find("ncols");
    const auto first = state.globals.find("cfirst");
    const auto second_plus = state.globals.find("nsecondplus");
    const auto after_name = state.globals.find("caftername");
    const auto after_age = state.globals.find("nafterage");

    expect(array_len != state.globals.end(), "ALEN(aRow) should expose array element count");
    expect(rows != state.globals.end(), "ALEN(aRow, 1) should expose first dimension");
    expect(cols != state.globals.end(), "ALEN(aRow, 2) should expose second dimension");
    expect(first != state.globals.end(), "aRow[1] should read the first scattered value");
    expect(second_plus != state.globals.end(), "aRow(2) should read the second scattered value");
    expect(after_name != state.globals.end(), "GATHER FROM array should restore NAME");
    expect(after_age != state.globals.end(), "GATHER FROM array should restore AGE");

    if (array_len != state.globals.end()) {
        expect(copperfin::runtime::format_value(array_len->second) == "2",
            "SCATTER TO array should create two array elements");
    }
    if (rows != state.globals.end()) {
        expect(copperfin::runtime::format_value(rows->second) == "2",
            "SCATTER TO array should create a one-dimensional row count matching field count");
    }
    if (cols != state.globals.end()) {
        expect(copperfin::runtime::format_value(cols->second) == "1",
            "SCATTER TO array should expose one column for first-pass one-dimensional arrays");
    }
    if (first != state.globals.end()) {
        expect(copperfin::runtime::format_value(first->second) == "Alice",
            "array bracket access should read the first scattered value");
    }
    if (second_plus != state.globals.end()) {
        expect(copperfin::runtime::format_value(second_plus->second) == "43",
            "array paren access should preserve numeric scattered values");
    }
    if (after_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_name->second) == "Alice",
            "GATHER FROM array should write NAME from the array");
    }
    if (after_age != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_age->second) == "42",
            "GATHER FROM array should write AGE from the array");
    }

    fs::remove_all(temp_root, ignored);
}

void test_runtime_array_mutator_functions() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_array_mutators";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "tools.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "FIRST", .type = 'C', .length = 10U},
        {.name = "SECOND", .type = 'C', .length = 10U},
    };
    const std::vector<std::vector<std::string>> records{
        {"Zulu", "Alpha"},
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "array mutator DBF fixture should be created");

    const fs::path main_path = temp_root / "array_mutators.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "'\n"
        "SCATTER FIELDS FIRST, SECOND TO aTools\n"
        "nScanAlpha = ASCAN(aTools, 'Alpha')\n"
        "nSortOk = ASORT(aTools)\n"
        "cSortedFirst = aTools[1]\n"
        "cSortedSecond = aTools[2]\n"
        "nDeleteOk = ADEL(aTools, 1)\n"
        "cAfterDeleteFirst = aTools[1]\n"
        "nInsertOk = AINS(aTools, 1)\n"
        "cAfterInsertFirstType = VARTYPE(aTools[1])\n"
        "cAfterInsertSecond = aTools[2]\n"
        "nResize = ASIZE(aTools, 4)\n"
        "nLenAfterResize = ALEN(aTools)\n"
        "cPreservedAfterResize = aTools[2]\n"
        "DIMENSION aSource[2,3], aTarget[2,3], aFlat[1]\n"
        "aSource[1,1] = 'A'\n"
        "aSource[1,2] = 'B'\n"
        "aSource[1,3] = 'C'\n"
        "aSource[2,1] = 'D'\n"
        "aSource[2,2] = 'E'\n"
        "aSource[2,3] = 'F'\n"
        "nElement = AELEMENT(aSource, 2, 2)\n"
        "nElementRow = ASUBSCRIPT(aSource, nElement, 1)\n"
        "nElementColumn = ASUBSCRIPT(aSource, nElement, 2)\n"
        "nCopyWindow = ACOPY(aSource, aTarget, 2, 3, 3)\n"
        "cTargetOne = aTarget[1,3]\n"
        "cTargetTwo = aTarget[2,1]\n"
        "cTargetThree = aTarget[2,2]\n"
        "nCopyAll = ACOPY(aSource, aFlat)\n"
        "nFlatLen = ALEN(aFlat)\n"
        "cFlatSix = aFlat[6]\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "runtime array mutator script should complete");

    const auto scan_alpha = state.globals.find("nscanalpha");
    const auto sort_ok = state.globals.find("nsortok");
    const auto sorted_first = state.globals.find("csortedfirst");
    const auto sorted_second = state.globals.find("csortedsecond");
    const auto delete_ok = state.globals.find("ndeleteok");
    const auto after_delete_first = state.globals.find("cafterdeletefirst");
    const auto insert_ok = state.globals.find("ninsertok");
    const auto after_insert_first_type = state.globals.find("cafterinsertfirsttype");
    const auto after_insert_second = state.globals.find("cafterinsertsecond");
    const auto resize = state.globals.find("nresize");
    const auto len_after_resize = state.globals.find("nlenafterresize");
    const auto preserved_after_resize = state.globals.find("cpreservedafterresize");
    const auto element = state.globals.find("nelement");
    const auto element_row = state.globals.find("nelementrow");
    const auto element_column = state.globals.find("nelementcolumn");
    const auto copy_window = state.globals.find("ncopywindow");
    const auto target_one = state.globals.find("ctargetone");
    const auto target_two = state.globals.find("ctargettwo");
    const auto target_three = state.globals.find("ctargetthree");
    const auto copy_all = state.globals.find("ncopyall");
    const auto flat_len = state.globals.find("nflatlen");
    const auto flat_six = state.globals.find("cflatsix");

    expect(scan_alpha != state.globals.end(), "ASCAN should return a captured position");
    expect(sort_ok != state.globals.end(), "ASORT should return a status");
    expect(sorted_first != state.globals.end(), "ASORT should leave a readable first element");
    expect(sorted_second != state.globals.end(), "ASORT should leave a readable second element");
    expect(delete_ok != state.globals.end(), "ADEL should return a status");
    expect(after_delete_first != state.globals.end(), "ADEL should shift following elements left");
    expect(insert_ok != state.globals.end(), "AINS should return a status");
    expect(after_insert_first_type != state.globals.end(), "AINS should insert an empty slot");
    expect(after_insert_second != state.globals.end(), "AINS should shift existing elements right");
    expect(resize != state.globals.end(), "ASIZE should return the new element count");
    expect(len_after_resize != state.globals.end(), "ALEN should reflect ASIZE result");
    expect(preserved_after_resize != state.globals.end(), "ASIZE should preserve existing values");
    expect(element != state.globals.end(), "AELEMENT should return a linear element number");
    expect(element_row != state.globals.end(), "ASUBSCRIPT should resolve the element row");
    expect(element_column != state.globals.end(), "ASUBSCRIPT should resolve the element column");
    expect(copy_window != state.globals.end(), "ACOPY should return a copied window count");
    expect(target_one != state.globals.end(), "ACOPY should copy into the requested target element");
    expect(target_two != state.globals.end(), "ACOPY should continue across target rows");
    expect(target_three != state.globals.end(), "ACOPY should copy the full requested window");
    expect(copy_all != state.globals.end(), "ACOPY should copy all remaining source elements by default");
    expect(flat_len != state.globals.end(), "ACOPY should grow a one-dimensional target when needed");
    expect(flat_six != state.globals.end(), "ACOPY should preserve the final copied source element");

    if (scan_alpha != state.globals.end()) {
        expect(copperfin::runtime::format_value(scan_alpha->second) == "2", "ASCAN should find Alpha in the original second slot");
    }
    if (sort_ok != state.globals.end()) {
        expect(copperfin::runtime::format_value(sort_ok->second) == "1", "ASORT should report success");
    }
    if (sorted_first != state.globals.end()) {
        expect(copperfin::runtime::format_value(sorted_first->second) == "Alpha", "ASORT should sort Alpha first");
    }
    if (sorted_second != state.globals.end()) {
        expect(copperfin::runtime::format_value(sorted_second->second) == "Zulu", "ASORT should sort Zulu second");
    }
    if (delete_ok != state.globals.end()) {
        expect(copperfin::runtime::format_value(delete_ok->second) == "1", "ADEL should report success");
    }
    if (after_delete_first != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_delete_first->second) == "Zulu", "ADEL should shift Zulu into the first slot");
    }
    if (insert_ok != state.globals.end()) {
        expect(copperfin::runtime::format_value(insert_ok->second) == "1", "AINS should report success");
    }
    if (after_insert_first_type != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_insert_first_type->second) == "U", "AINS should insert an empty slot");
    }
    if (after_insert_second != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_insert_second->second) == "Zulu", "AINS should shift Zulu into the second slot");
    }
    if (resize != state.globals.end()) {
        expect(copperfin::runtime::format_value(resize->second) == "4", "ASIZE should report the new element count");
    }
    if (len_after_resize != state.globals.end()) {
        expect(copperfin::runtime::format_value(len_after_resize->second) == "4", "ALEN should reflect ASIZE growth");
    }
    if (preserved_after_resize != state.globals.end()) {
        expect(copperfin::runtime::format_value(preserved_after_resize->second) == "Zulu", "ASIZE should preserve shifted values");
    }
    if (element != state.globals.end()) {
        expect(copperfin::runtime::format_value(element->second) == "5", "AELEMENT(aSource, 2, 2) should return row-major element 5");
    }
    if (element_row != state.globals.end()) {
        expect(copperfin::runtime::format_value(element_row->second) == "2", "ASUBSCRIPT(..., 1) should return row 2");
    }
    if (element_column != state.globals.end()) {
        expect(copperfin::runtime::format_value(element_column->second) == "2", "ASUBSCRIPT(..., 2) should return column 2");
    }
    if (copy_window != state.globals.end()) {
        expect(copperfin::runtime::format_value(copy_window->second) == "3", "ACOPY should report three copied elements");
    }
    if (target_one != state.globals.end()) {
        expect(copperfin::runtime::format_value(target_one->second) == "B", "ACOPY should place source element 2 in target element 3");
    }
    if (target_two != state.globals.end()) {
        expect(copperfin::runtime::format_value(target_two->second) == "C", "ACOPY should place source element 3 in target element 4");
    }
    if (target_three != state.globals.end()) {
        expect(copperfin::runtime::format_value(target_three->second) == "D", "ACOPY should place source element 4 in target element 5");
    }
    if (copy_all != state.globals.end()) {
        expect(copperfin::runtime::format_value(copy_all->second) == "6", "ACOPY without count should copy all source elements");
    }
    if (flat_len != state.globals.end()) {
        expect(copperfin::runtime::format_value(flat_len->second) == "6", "ACOPY should grow a flat target to six elements");
    }
    if (flat_six != state.globals.end()) {
        expect(copperfin::runtime::format_value(flat_six->second) == "F", "ACOPY should copy the final element into the grown flat target");
    }

    fs::remove_all(temp_root, ignored);
}

void test_array_dimension_and_element_assignment() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_array_assignment";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "array_assignment.prg";
    write_text(
        main_path,
        "DIMENSION aGrid[2,3], aList(2)\n"
        "DECLARE aDeclared[1,2]\n"
        "aGrid[1,1] = 'A'\n"
        "aGrid[1,2] = 'B'\n"
        "aGrid(2,3) = 'F'\n"
        "aDeclared[1,2] = 'D'\n"
        "aList[1] = 10\n"
        "aList(2) = 15\n"
        "nGridRows = ALEN(aGrid, 1)\n"
        "nGridCols = ALEN(aGrid, 2)\n"
        "nDeclaredCols = ALEN(aDeclared, 2)\n"
        "cGridA = aGrid[1,1]\n"
        "cGridB = aGrid(1,2)\n"
        "cGridF = aGrid[2,3]\n"
        "cDeclared = aDeclared[1,2]\n"
        "nListSum = aList[1] + aList(2)\n"
        "nResize = ASIZE(aGrid, 3, 4)\n"
        "cGridFAfterResize = aGrid[2,3]\n"
        "aGrid[3,4] = 'L'\n"
        "cGridL = aGrid(3,4)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "array declaration and element assignment script should complete");

    const auto rows = state.globals.find("ngridrows");
    const auto cols = state.globals.find("ngridcols");
    const auto declared_cols = state.globals.find("ndeclaredcols");
    const auto grid_a = state.globals.find("cgrida");
    const auto grid_b = state.globals.find("cgridb");
    const auto grid_f = state.globals.find("cgridf");
    const auto declared = state.globals.find("cdeclared");
    const auto list_sum = state.globals.find("nlistsum");
    const auto resize = state.globals.find("nresize");
    const auto grid_f_after_resize = state.globals.find("cgridfafterresize");
    const auto grid_l = state.globals.find("cgridl");

    expect(rows != state.globals.end(), "DIMENSION should expose row count through ALEN");
    expect(cols != state.globals.end(), "DIMENSION should expose column count through ALEN");
    expect(declared_cols != state.globals.end(), "DECLARE should expose column count through ALEN");
    expect(grid_a != state.globals.end(), "bracket array assignment should be readable");
    expect(grid_b != state.globals.end(), "paren array read should be readable");
    expect(grid_f != state.globals.end(), "paren array assignment should be readable through bracket syntax");
    expect(declared != state.globals.end(), "DECLARE array assignment should be readable");
    expect(list_sum != state.globals.end(), "one-dimensional array assignment should support arithmetic reads");
    expect(resize != state.globals.end(), "ASIZE should resize declared 2D arrays");
    expect(grid_f_after_resize != state.globals.end(), "ASIZE should preserve existing 2D values");
    expect(grid_l != state.globals.end(), "array assignment should write newly grown 2D cells");

    if (rows != state.globals.end()) {
        expect(copperfin::runtime::format_value(rows->second) == "2", "DIMENSION aGrid[2,3] should create two rows");
    }
    if (cols != state.globals.end()) {
        expect(copperfin::runtime::format_value(cols->second) == "3", "DIMENSION aGrid[2,3] should create three columns");
    }
    if (declared_cols != state.globals.end()) {
        expect(copperfin::runtime::format_value(declared_cols->second) == "2", "DECLARE aDeclared[1,2] should create two columns");
    }
    if (grid_a != state.globals.end()) {
        expect(copperfin::runtime::format_value(grid_a->second) == "A", "aGrid[1,1] should contain A");
    }
    if (grid_b != state.globals.end()) {
        expect(copperfin::runtime::format_value(grid_b->second) == "B", "aGrid(1,2) should contain B");
    }
    if (grid_f != state.globals.end()) {
        expect(copperfin::runtime::format_value(grid_f->second) == "F", "aGrid[2,3] should contain F");
    }
    if (declared != state.globals.end()) {
        expect(copperfin::runtime::format_value(declared->second) == "D", "aDeclared[1,2] should contain D");
    }
    if (list_sum != state.globals.end()) {
        expect(copperfin::runtime::format_value(list_sum->second) == "25", "array element reads should participate in arithmetic");
    }
    if (resize != state.globals.end()) {
        expect(copperfin::runtime::format_value(resize->second) == "12", "ASIZE(aGrid, 3, 4) should report twelve elements");
    }
    if (grid_f_after_resize != state.globals.end()) {
        expect(copperfin::runtime::format_value(grid_f_after_resize->second) == "F", "ASIZE should preserve existing 2D cells");
    }
    if (grid_l != state.globals.end()) {
        expect(copperfin::runtime::format_value(grid_l->second) == "L", "aGrid[3,4] should contain L after growth");
    }

    fs::remove_all(temp_root, ignored);
}

void test_array_metadata_and_text_functions() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_array_metadata";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_people_dbf(temp_root / "people.dbf", {{"Alice", 41}});
    write_text(temp_root / "alpha.txt", "abc");
    write_text(temp_root / "beta.bin", "not matched");

    const fs::path main_path = temp_root / "array_metadata.prg";
    write_text(
        main_path,
        "USE '" + (temp_root / "people.dbf").string() + "' ALIAS People\n"
        "nLineCount = ALINES(aLines, ' red ' + CHR(13) + CHR(10) + 'blue', 1)\n"
        "cLineOne = aLines[1]\n"
        "cLineTwo = aLines[2]\n"
        "nFileCount = ADIR(aFiles, '" + (temp_root / "*.txt").string() + "')\n"
        "cFileName = aFiles[1,1]\n"
        "nFileSize = aFiles[1,2]\n"
        "cFileAttr = aFiles[1,5]\n"
        "nFieldCount = AFIELDS(aFields)\n"
        "nFieldCols = ALEN(aFields, 2)\n"
        "cFieldOneName = aFields[1,1]\n"
        "cFieldOneType = aFields[1,2]\n"
        "nFieldOneWidth = aFields[1,3]\n"
        "cFieldTwoName = aFields[2,1]\n"
        "cFieldTwoType = aFields[2,2]\n"
        "nFieldTwoWidth = aFields[2,3]\n"
        "nFieldTwoDecimals = aFields[2,4]\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "array metadata and text function script should complete");

    const auto line_count = state.globals.find("nlinecount");
    const auto line_one = state.globals.find("clineone");
    const auto line_two = state.globals.find("clinetwo");
    const auto file_count = state.globals.find("nfilecount");
    const auto file_name = state.globals.find("cfilename");
    const auto file_size = state.globals.find("nfilesize");
    const auto file_attr = state.globals.find("cfileattr");
    const auto field_count = state.globals.find("nfieldcount");
    const auto field_cols = state.globals.find("nfieldcols");
    const auto field_one_name = state.globals.find("cfieldonename");
    const auto field_one_type = state.globals.find("cfieldonetype");
    const auto field_one_width = state.globals.find("nfieldonewidth");
    const auto field_two_name = state.globals.find("cfieldtwoname");
    const auto field_two_type = state.globals.find("cfieldtwotype");
    const auto field_two_width = state.globals.find("nfieldtwowidth");
    const auto field_two_decimals = state.globals.find("nfieldtwodecimals");

    expect(line_count != state.globals.end(), "ALINES should return a count");
    expect(line_one != state.globals.end(), "ALINES should populate first line");
    expect(line_two != state.globals.end(), "ALINES should populate second line");
    expect(file_count != state.globals.end(), "ADIR should return a count");
    expect(file_name != state.globals.end(), "ADIR should populate file name");
    expect(file_size != state.globals.end(), "ADIR should populate file size");
    expect(file_attr != state.globals.end(), "ADIR should populate attribute column");
    expect(field_count != state.globals.end(), "AFIELDS should return a field count");
    expect(field_cols != state.globals.end(), "AFIELDS should expose its metadata column count");
    expect(field_one_name != state.globals.end(), "AFIELDS should populate first field name");
    expect(field_one_type != state.globals.end(), "AFIELDS should populate first field type");
    expect(field_one_width != state.globals.end(), "AFIELDS should populate first field width");
    expect(field_two_name != state.globals.end(), "AFIELDS should populate second field name");
    expect(field_two_type != state.globals.end(), "AFIELDS should populate second field type");
    expect(field_two_width != state.globals.end(), "AFIELDS should populate second field width");
    expect(field_two_decimals != state.globals.end(), "AFIELDS should populate second field decimals");

    if (line_count != state.globals.end()) {
        expect(copperfin::runtime::format_value(line_count->second) == "2", "ALINES should split two lines");
    }
    if (line_one != state.globals.end()) {
        expect(copperfin::runtime::format_value(line_one->second) == "red", "ALINES flag 1 should trim the first line");
    }
    if (line_two != state.globals.end()) {
        expect(copperfin::runtime::format_value(line_two->second) == "blue", "ALINES should preserve second line text");
    }
    if (file_count != state.globals.end()) {
        expect(copperfin::runtime::format_value(file_count->second) == "1", "ADIR should match only the txt file");
    }
    if (file_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(file_name->second) == "alpha.txt", "ADIR should return the matched file name");
    }
    if (file_size != state.globals.end()) {
        expect(copperfin::runtime::format_value(file_size->second) == "3", "ADIR should return the file size");
    }
    if (file_attr != state.globals.end()) {
        expect(copperfin::runtime::format_value(file_attr->second).find("D") == std::string::npos,
            "ADIR should not mark normal files as directories");
    }
    if (field_count != state.globals.end()) {
        expect(copperfin::runtime::format_value(field_count->second) == "2", "AFIELDS should report two fields");
    }
    if (field_cols != state.globals.end()) {
        expect(copperfin::runtime::format_value(field_cols->second) == "16", "AFIELDS should expose sixteen metadata columns");
    }
    if (field_one_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(field_one_name->second) == "NAME", "AFIELDS first field name should be NAME");
    }
    if (field_one_type != state.globals.end()) {
        expect(copperfin::runtime::format_value(field_one_type->second) == "C", "AFIELDS first field type should be C");
    }
    if (field_one_width != state.globals.end()) {
        expect(copperfin::runtime::format_value(field_one_width->second) == "10", "AFIELDS first field width should be 10");
    }
    if (field_two_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(field_two_name->second) == "AGE", "AFIELDS second field name should be AGE");
    }
    if (field_two_type != state.globals.end()) {
        expect(copperfin::runtime::format_value(field_two_type->second) == "N", "AFIELDS second field type should be N");
    }
    if (field_two_width != state.globals.end()) {
        expect(copperfin::runtime::format_value(field_two_width->second) == "3", "AFIELDS second field width should be 3");
    }
    if (field_two_decimals != state.globals.end()) {
        expect(copperfin::runtime::format_value(field_two_decimals->second) == "0", "AFIELDS second field decimals should be 0");
    }

    fs::remove_all(temp_root, ignored);
}

void test_copy_to_emits_event() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_copy_to";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_simple_dbf(temp_root / "source.dbf", {"row1", "row2"});

    const fs::path main_path = temp_root / "copy_to_test.prg";
    write_text(
        main_path,
        "USE '" + (temp_root / "source.dbf").string() + "'\n"
        "COPY TO 'dest'\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "COPY TO script should complete");

    const bool has_event = std::any_of(state.events.begin(), state.events.end(),
        [](const copperfin::runtime::RuntimeEvent& ev) {
            return ev.category == "runtime.copy_to";
        });
    expect(has_event, "COPY TO should emit a runtime.copy_to event");

    fs::remove_all(temp_root, ignored);
}

void test_copy_to_creates_destination_dbf() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_copy_to_full";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_simple_dbf(temp_root / "source.dbf", {"Alice", "Bob", "Carol"});

    const fs::path main_path = temp_root / "copy_to_full.prg";
    const std::string dest_path = (temp_root / "dest.dbf").string();
    write_text(
        main_path,
        "USE '" + (temp_root / "source.dbf").string() + "'\n"
        "COPY TO '" + dest_path + "'\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "COPY TO full script should complete");
    expect(fs::exists(dest_path), "COPY TO should create destination DBF file");

    if (fs::exists(dest_path)) {
        const auto result = copperfin::vfp::parse_dbf_table_from_file(dest_path, 100U);
        expect(result.ok, "COPY TO destination DBF should be readable");
        expect(result.table.records.size() == 3U,
            "COPY TO should copy all 3 records (got " + std::to_string(result.table.records.size()) + ")");
        if (result.table.records.size() >= 1U) {
            const auto& first = result.table.records[0U].values;
            const bool has_alice = !first.empty() && first[0U].display_value.find("Alice") != std::string::npos;
            expect(has_alice, "COPY TO first record should contain Alice");
        }
    }

    fs::remove_all(temp_root, ignored);
}

void test_copy_structure_to_creates_empty_schema() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_copy_struct";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_simple_dbf(temp_root / "source.dbf", {"Alice", "Bob"});

    const fs::path main_path = temp_root / "copy_struct.prg";
    const std::string dest_path = (temp_root / "schema.dbf").string();
    write_text(
        main_path,
        "USE '" + (temp_root / "source.dbf").string() + "'\n"
        "COPY STRUCTURE TO '" + dest_path + "'\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "COPY STRUCTURE TO script should complete");
    expect(fs::exists(dest_path), "COPY STRUCTURE TO should create destination DBF");

    if (fs::exists(dest_path)) {
        const auto result = copperfin::vfp::parse_dbf_table_from_file(dest_path, 100U);
        expect(result.ok, "COPY STRUCTURE TO destination DBF should be readable");
        expect(result.table.records.empty(), "COPY STRUCTURE TO should produce zero rows");
        expect(!result.table.fields.empty(), "COPY STRUCTURE TO should preserve field descriptors");
    }

    fs::remove_all(temp_root, ignored);
}

void test_append_from_copies_records_into_current_table() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_append_from";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    // Create destination with one row; source with two rows
    write_simple_dbf(temp_root / "dest.dbf", {"Alice"});
    write_simple_dbf(temp_root / "source.dbf", {"Bob", "Carol"});

    const fs::path main_path = temp_root / "append_from.prg";
    write_text(
        main_path,
        "USE '" + (temp_root / "dest.dbf").string() + "'\n"
        "APPEND FROM '" + (temp_root / "source.dbf").string() + "'\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "APPEND FROM script should complete");

    const bool has_event = std::any_of(state.events.begin(), state.events.end(),
        [](const copperfin::runtime::RuntimeEvent& ev) {
            return ev.category == "runtime.append_from";
        });
    expect(has_event, "APPEND FROM should emit a runtime.append_from event");

    // Verify destination now has 3 records (1 original + 2 from source)
    const auto result = copperfin::vfp::parse_dbf_table_from_file(
        (temp_root / "dest.dbf").string(), 100U);
    expect(result.ok, "APPEND FROM destination DBF should be readable after append");
    expect(result.table.records.size() == 3U,
        "APPEND FROM should produce 3 records total (got " + std::to_string(result.table.records.size()) + ")");
    if (result.table.records.size() >= 3U) {
        const bool has_bob = result.table.records[1U].values[0U].display_value.find("Bob") != std::string::npos;
        expect(has_bob, "APPEND FROM second record should be Bob");
        const bool has_carol = result.table.records[2U].values[0U].display_value.find("Carol") != std::string::npos;
        expect(has_carol, "APPEND FROM third record should be Carol");
    }

    fs::remove_all(temp_root, ignored);
}

void test_copy_to_type_sdf_writes_fixed_width_text_rows() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_copy_to_sdf";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_simple_dbf(temp_root / "source.dbf", {"Alice", "Bob"});

    const fs::path main_path = temp_root / "copy_to_sdf.prg";
    const std::string dest_path = (temp_root / "people.sdf").string();
    write_text(
        main_path,
        "USE '" + (temp_root / "source.dbf").string() + "'\n"
        "COPY TO '" + dest_path + "' TYPE SDF\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "COPY TO TYPE SDF script should complete");
    expect(fs::exists(dest_path), "COPY TO TYPE SDF should create the destination text file");
    if (fs::exists(dest_path)) {
        const std::string contents = read_text(dest_path);
        expect(contents == "Alice     \r\nBob       \r\n",
            "COPY TO TYPE SDF should write fixed-width rows using DBF field lengths");
    }

    fs::remove_all(temp_root, ignored);
}

void test_append_from_type_sdf_imports_fixed_width_text_rows() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_append_from_sdf";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_simple_dbf(temp_root / "dest.dbf", {});
    write_text(temp_root / "people.sdf", "Dora      \r\nEvan      \r\n");

    const fs::path main_path = temp_root / "append_from_sdf.prg";
    write_text(
        main_path,
        "USE '" + (temp_root / "dest.dbf").string() + "'\n"
        "APPEND FROM '" + (temp_root / "people.sdf").string() + "' TYPE SDF\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "APPEND FROM TYPE SDF script should complete");

    const auto result = copperfin::vfp::parse_dbf_table_from_file(
        (temp_root / "dest.dbf").string(), 100U);
    expect(result.ok, "APPEND FROM TYPE SDF destination DBF should be readable");
    expect(result.table.records.size() == 2U,
        "APPEND FROM TYPE SDF should append two rows");
    if (result.table.records.size() >= 2U) {
        expect(result.table.records[0U].values[0U].display_value == "Dora",
            "first SDF row should import into the first DBF record");
        expect(result.table.records[1U].values[0U].display_value == "Evan",
            "second SDF row should import into the second DBF record");
    }

    fs::remove_all(temp_root, ignored);
}

void test_copy_to_type_csv_and_delimited_text_rows() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_copy_to_csv";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_people_dbf(temp_root / "people.dbf", {{"Ann,Lee", 7}, {"Bob", 42}});

    const fs::path main_path = temp_root / "copy_to_csv.prg";
    const std::string csv_path = (temp_root / "people.csv").string();
    const std::string pipe_path = (temp_root / "people_pipe.txt").string();
    const std::string custom_path = (temp_root / "people_custom.txt").string();
    write_text(
        main_path,
        "USE '" + (temp_root / "people.dbf").string() + "'\n"
        "COPY TO '" + csv_path + "' TYPE CSV FIELDS NAME, AGE\n"
        "COPY TO '" + pipe_path + "' DELIMITED WITH CHARACTER '|' FIELDS NAME, AGE FOR AGE > 10\n"
        "COPY TO '" + custom_path + "' DELIMITED WITH '_' WITH CHARACTER ';' FIELDS NAME, AGE FOR AGE > 10\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "COPY TO TYPE CSV/DELIMITED script should complete");
    expect(fs::exists(csv_path), "COPY TO TYPE CSV should create destination text file");
    expect(fs::exists(pipe_path), "COPY TO DELIMITED should create destination text file");
    expect(fs::exists(custom_path), "COPY TO DELIMITED custom enclosure should create destination text file");

    if (fs::exists(csv_path)) {
        const std::string contents = read_text(csv_path);
        expect(contents == "NAME,AGE\r\n\"Ann,Lee\",7\r\n\"Bob\",42\r\n",
            "COPY TO TYPE CSV should write a field-name header and quote character fields");
    }
    if (fs::exists(pipe_path)) {
        const std::string contents = read_text(pipe_path);
        expect(contents == "\"Bob\"|42\r\n",
            "COPY TO DELIMITED WITH CHARACTER should honor the delimiter and FOR clause");
    }
    if (fs::exists(custom_path)) {
        const std::string contents = read_text(custom_path);
        expect(contents == "_Bob_;42\r\n",
            "COPY TO DELIMITED WITH enclosure plus WITH CHARACTER should honor both VFP options");
    }

    fs::remove_all(temp_root, ignored);
}

void test_append_from_type_csv_imports_delimited_rows() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_append_from_csv";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_people_dbf(temp_root / "dest.dbf", {});
    write_text(temp_root / "people.csv", "NAME,AGE\r\n\"Ivy, Jr\",9\r\n\"Max\",44\r\n");
    write_text(temp_root / "people_pipe.txt", "\"Nia\"|12\r\n");
    write_text(temp_root / "people_custom.txt", "_Ora_;15\r\n");

    const fs::path main_path = temp_root / "append_from_csv.prg";
    write_text(
        main_path,
        "USE '" + (temp_root / "dest.dbf").string() + "'\n"
        "APPEND FROM '" + (temp_root / "people.csv").string() + "' TYPE CSV FIELDS NAME, AGE\n"
        "APPEND FROM '" + (temp_root / "people_pipe.txt").string() + "' DELIMITED WITH CHARACTER '|' FIELDS NAME, AGE\n"
        "APPEND FROM '" + (temp_root / "people_custom.txt").string() + "' DELIMITED WITH '_' WITH CHARACTER ';' FIELDS NAME, AGE\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "APPEND FROM TYPE CSV/DELIMITED script should complete");

    const auto result = copperfin::vfp::parse_dbf_table_from_file(
        (temp_root / "dest.dbf").string(), 100U);
    expect(result.ok, "APPEND FROM TYPE CSV destination DBF should be readable");
    expect(result.table.records.size() == 4U,
        "APPEND FROM TYPE CSV/DELIMITED should append four rows");
    if (result.table.records.size() >= 4U) {
        expect(result.table.records[0U].values[0U].display_value == "Ivy, Jr",
            "first CSV row should preserve comma inside quoted character field");
        expect(result.table.records[0U].values[1U].display_value == "9",
            "first CSV numeric field should import into AGE");
        expect(result.table.records[1U].values[0U].display_value == "Max",
            "second CSV row should import NAME");
        expect(result.table.records[1U].values[1U].display_value == "44",
            "second CSV row should import AGE");
        expect(result.table.records[2U].values[0U].display_value == "Nia",
            "DELIMITED WITH CHARACTER row should import NAME");
        expect(result.table.records[2U].values[1U].display_value == "12",
            "DELIMITED WITH CHARACTER row should import AGE");
        expect(result.table.records[3U].values[0U].display_value == "Ora",
            "DELIMITED custom enclosure row should import NAME");
        expect(result.table.records[3U].values[1U].display_value == "15",
            "DELIMITED custom enclosure row should import AGE");
    }

    fs::remove_all(temp_root, ignored);
}

void test_copy_to_array_fills_2d_runtime_array() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_copy_to_array";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_people_dbf(temp_root / "people.dbf", {{"Alice", 30}, {"Bob", 25}});

    const fs::path main_path = temp_root / "copy_to_array.prg";
    write_text(
        main_path,
        "USE '" + (temp_root / "people.dbf").string() + "'\n"
        "COPY TO ARRAY myarr\n"
        "row1_name = myarr[1, 1]\n"
        "row1_age = myarr[1, 2]\n"
        "row2_name = myarr[2, 1]\n"
        "row2_age = myarr[2, 2]\n"
        "arr_rows = ALEN(myarr, 1)\n"
        "arr_cols = ALEN(myarr, 2)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "COPY TO ARRAY script should complete");

    const auto chk = [&](const std::string& var, const std::string& expected, const std::string& msg) {
        const auto it = state.globals.find(var);
        expect(it != state.globals.end(), var + " should exist in globals");
        if (it != state.globals.end()) {
            const std::string val = copperfin::runtime::format_value(it->second);
            expect(val == expected, msg + " (got '" + val + "')");
        }
    };
    chk("arr_rows",  "2",     "COPY TO ARRAY 2 records should give 2 rows");
    chk("arr_cols",  "2",     "COPY TO ARRAY 2 fields should give 2 columns");
    chk("row1_name", "Alice", "COPY TO ARRAY row 1 col 1 should be NAME");
    chk("row1_age",  "30",    "COPY TO ARRAY row 1 col 2 should be AGE");
    chk("row2_name", "Bob",   "COPY TO ARRAY row 2 col 1 should be NAME");
    chk("row2_age",  "25",    "COPY TO ARRAY row 2 col 2 should be AGE");

    fs::remove_all(temp_root, ignored);
}

void test_append_from_array_writes_records_from_2d_array() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_append_from_array";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    // Source table has two records; dest table starts empty.
    write_people_dbf(temp_root / "source.dbf", {{"Carol", 55}, {"Dave", 19}});
    write_people_dbf(temp_root / "dest.dbf", {});

    const fs::path main_path = temp_root / "append_from_array.prg";
    write_text(
        main_path,
        "USE '" + (temp_root / "source.dbf").string() + "'\n"
        "COPY TO ARRAY tmparr\n"
        "USE '" + (temp_root / "dest.dbf").string() + "'\n"
        "APPEND FROM ARRAY tmparr\n"
        "GO 1\n"
        "dest_name1 = NAME\n"
        "dest_age1 = AGE\n"
        "GO 2\n"
        "dest_name2 = NAME\n"
        "dest_age2 = AGE\n"
        "dest_rc = RECCOUNT()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "APPEND FROM ARRAY script should complete");

    const auto chk2 = [&](const std::string& var, const std::string& expected, const std::string& msg) {
        const auto it = state.globals.find(var);
        expect(it != state.globals.end(), var + " should exist in globals");
        if (it != state.globals.end()) {
            const std::string val = copperfin::runtime::format_value(it->second);
            expect(val == expected, msg + " (got '" + val + "')");
        }
    };
    chk2("dest_rc",    "2",     "APPEND FROM ARRAY should append 2 records");
    chk2("dest_name1", "Carol", "APPEND FROM ARRAY record 1 NAME should match");
    chk2("dest_age1",  "55",    "APPEND FROM ARRAY record 1 AGE should match");
    chk2("dest_name2", "Dave",  "APPEND FROM ARRAY record 2 NAME should match");
    chk2("dest_age2",  "19",    "APPEND FROM ARRAY record 2 AGE should match");

    fs::remove_all(temp_root, ignored);
}

void test_gather_memvar_round_trips_field_values() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_gather_rt";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_simple_dbf(temp_root / "table.dbf", {"Alice", "Bob"});

    const fs::path main_path = temp_root / "gather_rt.prg";
    write_text(
        main_path,
        "USE '" + (temp_root / "table.dbf").string() + "'\n"
        "GO 1\n"
        "SCATTER MEMVAR\n"
        "m.NAME = 'Updated'\n"
        "GATHER MEMVAR\n"
        "updated_name = NAME\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "GATHER MEMVAR round-trip script should complete");

    const auto it = state.globals.find("updated_name");
    expect(it != state.globals.end(), "updated_name variable should exist after GATHER MEMVAR");
    if (it != state.globals.end()) {
        const std::string val = copperfin::runtime::format_value(it->second);
        expect(val.find("Updated") != std::string::npos,
            "GATHER MEMVAR should write m.NAME back to record (got '" + val + "')");
    }

    fs::remove_all(temp_root, ignored);
}

void test_m_dot_namespace_shares_bare_memory_variable_binding() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_m_dot_namespace";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "m_dot.prg";
    write_text(
        main_path,
        "m.customer = 'Alice'\n"
        "from_bare = customer\n"
        "M.customer = 'Bob'\n"
        "from_prefixed = m.customer\n"
        "customer = 'Carol'\n"
        "from_m_after_bare = m.customer\n"
        "PUBLIC m.public_name\n"
        "m.public_name = 'Public'\n"
        "from_public_bare = public_name\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "m. namespace script should complete");

    const auto customer = state.globals.find("customer");
    const auto prefixed_customer = state.globals.find("m.customer");
    const auto from_bare = state.globals.find("from_bare");
    const auto from_prefixed = state.globals.find("from_prefixed");
    const auto from_m_after_bare = state.globals.find("from_m_after_bare");
    const auto from_public_bare = state.globals.find("from_public_bare");

    expect(customer != state.globals.end(), "m.customer should be stored under the bare customer binding");
    expect(prefixed_customer == state.globals.end(), "m.customer should not create a separate prefixed global binding");
    expect(from_bare != state.globals.end(), "bare reads should see m.customer assignments");
    expect(from_prefixed != state.globals.end(), "m. reads should see bare memory-variable storage");
    expect(from_m_after_bare != state.globals.end(), "m. reads should see later bare assignments");
    expect(from_public_bare != state.globals.end(), "PUBLIC m.name should declare the bare memory-variable binding");

    if (customer != state.globals.end()) {
        expect(copperfin::runtime::format_value(customer->second) == "Carol",
            "bare assignment should update the shared m.customer binding");
    }
    if (from_bare != state.globals.end()) {
        expect(copperfin::runtime::format_value(from_bare->second) == "Alice",
            "bare reads should resolve the initial m.customer value");
    }
    if (from_prefixed != state.globals.end()) {
        expect(copperfin::runtime::format_value(from_prefixed->second) == "Bob",
            "m.customer reads should resolve the updated shared value");
    }
    if (from_m_after_bare != state.globals.end()) {
        expect(copperfin::runtime::format_value(from_m_after_bare->second) == "Carol",
            "m.customer should resolve a value assigned through the bare name");
    }
    if (from_public_bare != state.globals.end()) {
        expect(copperfin::runtime::format_value(from_public_bare->second) == "Public",
            "PUBLIC m.public_name should be readable through public_name");
    }

    fs::remove_all(temp_root, ignored);
}

}  // namespace

int main() {
    test_breakpoints_and_stepping();
    test_read_events_pause();
    test_activate_popup_pause();
    test_dispatch_event_handler();
    test_local_variables_in_stack_frame();
    test_report_form_pause();
    test_label_form_pause();
    test_report_form_to_file_renders_without_event_loop_pause();
    test_label_form_to_file_renders_without_event_loop_pause();
    test_do_form_pause();
    test_export_vfp_compatibility_corpus_script();
    test_work_area_and_data_session_compatibility();
    test_eval_macro_and_runtime_state_semantics();
    test_sql_and_ole_compatibility_functions();
    test_sql_pass_through_rows_affected_and_provider_hint();
    test_sql_prepare_and_connection_properties();
    test_use_and_data_session_isolation();
    test_cross_session_alias_and_work_area_isolation();
    test_use_in_existing_alias_reuses_target_work_area();
    test_use_in_nonselected_alias_preserves_selected_work_area();
    test_plain_use_reuses_current_selected_work_area();
    test_select_and_use_in_designator_expressions();
    test_expression_driven_in_targeting_across_local_data_commands();
    test_select_zero_and_use_in_zero_reuse_closed_work_area();
    test_go_and_skip_cursor_navigation();
    test_cursor_identity_functions_for_local_tables();
    test_local_use_auto_allocation_tracks_session_selection_flow();
    test_local_selected_empty_area_reuses_after_datasession_round_trip();
    test_set_order_and_seek_for_local_tables();
    test_seek_uses_grounded_order_normalization_hints();
    test_seek_supports_composite_tag_expressions();
    test_seek_supports_left_function_tag_expressions();
    test_seek_supports_right_function_tag_expressions();
    test_seek_supports_substr_function_tag_expressions();
    test_seek_supports_padl_function_tag_expressions();
    test_seek_supports_padr_function_tag_expressions();
    test_seek_supports_padl_default_padding_tag_expressions();
    test_seek_supports_padr_default_padding_tag_expressions();
    test_seek_supports_str_function_tag_expressions();
    test_seek_supports_str_default_width_tag_expressions();
    test_seek_supports_str_decimal_tag_expressions();
    test_set_near_changes_seek_failure_position();
    test_set_order_descending_changes_seek_ordering();
    test_seek_command_accepts_tag_override_without_set_order();
    test_seek_command_accepts_descending_tag_override_without_set_order();
    test_set_near_is_scoped_by_data_session();
    test_seek_related_index_functions();
    test_seek_function_accepts_direction_suffix_in_order_designator();
    test_order_and_tag_preserve_index_file_identity();
    test_seek_respects_grounded_order_for_expression_hints();
    test_ndx_numeric_domain_guides_seek_near_ordering();
    test_foxtools_registration_and_call_bridge();
    test_foxtools_registration_is_scoped_by_data_session();
    test_set_exact_affects_comparisons_and_seek();
    test_use_again_and_alias_collision_semantics();
    test_use_in_selected_alias_replacement_clears_old_alias_and_order_state();
    test_select_missing_alias_is_an_error();
    test_use_in_missing_alias_is_an_error();
    test_set_filter_scopes_local_cursor_visibility();
    test_set_filter_in_targets_nonselected_alias();
    test_do_while_and_loop_control_flow();
    test_do_case_control_flow();
    test_text_endtext_literal_blocks();
    test_aggregate_functions_respect_visibility();
    test_calculate_command_aggregates();
    test_command_level_aggregate_commands();
    test_command_level_aggregate_scope_and_while_semantics();
    test_total_command_for_local_tables();
    test_total_command_supports_currency_and_integer_fields();
    test_total_command_for_sql_result_cursors();
    test_runtime_fault_containment();
    test_private_declaration_masks_caller_variable();
    test_private_variable_visible_to_called_routines();
    test_store_command_assigns_multiple_variables();
    test_runtime_guardrail_limits_call_depth_without_crashing_host();
    test_runtime_guardrail_limits_statement_budget_without_crashing_host();
    test_static_diagnostic_flags_likely_infinite_do_while_loop();
    test_config_fpw_overrides_runtime_limits();
    test_config_fpw_overrides_temp_directory_default();
    test_elseif_control_flow_executes_matching_branch();
    test_do_with_parameters_binds_arguments_in_called_routine();
    test_do_with_by_reference_updates_caller_variable();
    test_on_error_do_handler_dispatches_routine();
    test_on_error_do_with_handler_receives_error_metadata();
    test_aerror_populates_structured_runtime_error_array();
    test_aerror_exposes_sql_and_ole_specific_rows();
    test_with_endwith_resolves_leading_dot_member_access();
    test_try_catch_finally_handles_runtime_errors();
    test_try_finally_runs_without_catch_on_success();
    test_string_and_math_expression_functions();
    test_type_and_null_expression_functions();
    test_print_command_emits_event();
    test_close_command_closes_all_work_areas();
    test_erase_copy_rename_file_commands();
    test_scatter_memvar_from_current_record();
    test_scatter_gather_memvar_fields_blank_and_for_semantics();
    test_scatter_to_array_and_gather_from_array_round_trip();
    test_runtime_array_mutator_functions();
    test_array_dimension_and_element_assignment();
    test_array_metadata_and_text_functions();
    test_copy_to_emits_event();
    test_copy_to_creates_destination_dbf();
    test_copy_structure_to_creates_empty_schema();
    test_append_from_copies_records_into_current_table();
    test_copy_to_type_sdf_writes_fixed_width_text_rows();
    test_append_from_type_sdf_imports_fixed_width_text_rows();
    test_copy_to_type_csv_and_delimited_text_rows();
    test_append_from_type_csv_imports_delimited_rows();
    test_copy_to_array_fills_2d_runtime_array();
    test_append_from_array_writes_records_from_2d_array();
    test_gather_memvar_round_trips_field_values();
    test_m_dot_namespace_shares_bare_memory_variable_binding();

    if (test_failures() != 0) {
        std::cerr << test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
