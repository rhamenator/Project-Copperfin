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

void test_do_command_macro_target() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_do_macro_target";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "do_macro_target.prg";
    write_text(
        main_path,
        "cProc = 'worker'\n"
        "DO &cProc\n"
        "RETURN\n"
        "PROCEDURE worker\n"
        "nWorkerRan = 1\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "DO &macro-target script should complete");

    const auto worker_ran = state.globals.find("nworkerran");
    expect(worker_ran != state.globals.end(), "DO &macro target should resolve and invoke the routine");
    if (worker_ran != state.globals.end()) {
        expect(copperfin::runtime::format_value(worker_ran->second) == "1", "DO &macro target routine should run exactly once");
    }

    fs::remove_all(temp_root, ignored);
}

void test_export_vfp_compatibility_corpus_script() {
    namespace fs = std::filesystem;
    const fs::path repo_root = fs::path(__FILE__).parent_path().parent_path();
    const fs::path script_path = repo_root / "scripts" / "export-vfp-compatibility-corpus.ps1";
    const fs::path fixture_root = repo_root / "build" / "compatibility_corpus_fixture";
    const fs::path output_root = repo_root / "build" / "compatibility_corpus_output";
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

    intptr_t exit_code = -1;
#if defined(_WIN32)
    exit_code = _spawnvp(_P_WAIT, "powershell", const_cast<char* const*>(argv.data()));
#else
    const int has_pwsh = std::system("command -v pwsh >/dev/null 2>&1");
    const int has_powershell = has_pwsh == 0 ? 0 : std::system("command -v powershell >/dev/null 2>&1");
    if (has_pwsh != 0 && has_powershell != 0) {
        fs::remove_all(fixture_root, ignored);
        fs::remove_all(output_root, ignored);
        return;
    }
    script_args[0] = has_pwsh == 0 ? "pwsh" : "powershell";
    argv.clear();
    for (const auto& arg : script_args) {
        argv.push_back(arg.c_str());
    }
    argv.push_back(nullptr);
    const pid_t child = fork();
    if (child == 0) {
        execvp(argv[0], const_cast<char* const*>(argv.data()));
        _exit(127);
    }
    if (child > 0) {
        int status = 0;
        if (waitpid(child, &status, 0) == child && WIFEXITED(status)) {
            exit_code = WEXITSTATUS(status);
        }
    }
#endif
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

    expect(manifest.find("Samples/Solution/Reports/invoice.frx") != std::string::npos ||
               manifest.find(R"(Samples\\Solution\\Reports\\invoice.frx)") != std::string::npos,
           "manifest should include installed VFP sample report assets");
    expect(manifest.find("Wizards/Template/Books/Forms/books.scx") != std::string::npos ||
               manifest.find(R"(Wizards\\Template\\Books\\Forms\\books.scx)") != std::string::npos,
           "manifest should include installed VFP wizard form assets");
    expect(manifest.find("ReportBuilder/builder.prg") != std::string::npos ||
               manifest.find(R"(ReportBuilder\\builder.prg)") != std::string::npos,
           "manifest should include local VFP source PRGs");
    expect(manifest.find("Legacy/sample.pjx") != std::string::npos ||
               manifest.find(R"(Legacy\\sample.pjx)") != std::string::npos,
           "manifest should include legacy project assets");
    expect(manifest.find("runtime/macro.spr") != std::string::npos ||
               manifest.find(R"(runtime\\macro.spr)") != std::string::npos,
           "manifest should include regression sample assets");
    expect(manifest.find("\"assetCategory\":  \"designer\"") != std::string::npos ||
               manifest.find("\"assetCategory\": \"designer\"") != std::string::npos,
           "manifest should classify designer assets");
    expect(manifest.find("\"assetCategory\":  \"code\"") != std::string::npos ||
               manifest.find("\"assetCategory\": \"code\"") != std::string::npos,
           "manifest should classify code assets");
    expect(manifest.find("\"assetCategory\":  \"application\"") != std::string::npos ||
               manifest.find("\"assetCategory\": \"application\"") != std::string::npos,
           "manifest should classify project and app assets");
    expect(manifest.find("ignore.txt") == std::string::npos,
           "manifest should ignore unsupported file extensions");

    expect(summary.find("\"totalEntries\":  5") != std::string::npos ||
               summary.find("\"totalEntries\": 5") != std::string::npos,
           "summary should report the exported entry count");
    expect(summary.find("\"installed-vfp\":  2") != std::string::npos ||
               summary.find("\"installed-vfp\": 2") != std::string::npos,
           "summary should count installed VFP assets");
    expect(summary.find("\"local-vfp-source\":  1") != std::string::npos ||
               summary.find("\"local-vfp-source\": 1") != std::string::npos,
           "summary should count VFP source assets");
    expect(summary.find("\"legacy-project\":  1") != std::string::npos ||
               summary.find("\"legacy-project\": 1") != std::string::npos,
           "summary should count legacy project assets");
    expect(summary.find("\"regression-sample\":  1") != std::string::npos ||
               summary.find("\"regression-sample\": 1") != std::string::npos,
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

    write_text(
        temp_root / "macro_deep.prg",
        "cFieldExpr = 'cValue'\n"
        "cRowExpr = '1'\n"
        "cValue = 'HELLO'\n"
        "DIMENSION aData[2]\n"
        "aData[1] = 'ALPHA'\n"
        "aData[2] = 'BRAVO'\n"
        "cResult1 = aData[&cRowExpr]\n"
        "cResult2 = LEN(&cFieldExpr)\n"
        "cTarget = 'cResult3'\n"
        "&cTarget = 'MACROASSIGN'\n"
        "cParamExpr = 'cParamValue'\n"
        "cParamValue = 'PARAMVAL'\n"
        "DO testparam WITH &cParamExpr\n"
        "RETURN\n"
        "PROCEDURE testparam\n"
        "LPARAMETERS p1\n"
        "cParamResult = p1\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession macro_session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = (temp_root / "macro_deep.prg").string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });
    const auto macro_state = macro_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    const auto cResult1 = macro_state.globals.find("cresult1");
    const auto cResult2 = macro_state.globals.find("cresult2");
    const auto cResult3 = macro_state.globals.find("cresult3");
    const auto cParamResult = macro_state.globals.find("cparamresult");
    expect(cResult1 != macro_state.globals.end(), "macro in array subscript should resolve");
    expect(cResult2 != macro_state.globals.end(), "macro in function argument should resolve");
    expect(cResult3 != macro_state.globals.end(), "macro in assignment target should resolve");
    expect(cParamResult != macro_state.globals.end(), "macro in parameter passing should resolve");
    if (cResult1 != macro_state.globals.end()) {
        expect(copperfin::runtime::format_value(cResult1->second) == "ALPHA", "macro in array subscript should yield correct value");
    }
    if (cResult2 != macro_state.globals.end()) {
        expect(copperfin::runtime::format_value(cResult2->second) == "5", "macro in function argument should yield correct value");
    }
    if (cResult3 != macro_state.globals.end()) {
        expect(copperfin::runtime::format_value(cResult3->second) == "MACROASSIGN", "macro in assignment target should assign correctly");
    }
    if (cParamResult != macro_state.globals.end()) {
        expect(copperfin::runtime::format_value(cParamResult->second) == "PARAMVAL", "macro in parameter passing should yield correct value");
    }

    const fs::path new_default = temp_root / "workspace";
    fs::create_directories(new_default);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}});
    const fs::path path_probe_dir = temp_root / "path_probe";
    fs::create_directories(path_probe_dir);
    const fs::path path_only_file = path_probe_dir / "path_only_session.txt";
    write_text(path_only_file, "session path file");

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
        "SET NEAR TO .F.\n"
        "cNearAfterToFalse = SET('NEAR')\n"
        "cExactBefore = SET('EXACT')\n"
        "SET EXACT ON\n"
        "cExactAfter = SET('EXACT')\n"
        "SET EXACT TO 'OFF'\n"
        "cExactAfterToStringOff = SET('EXACT')\n"
        "cDeletedBefore = SET('DELETED')\n"
        "SET DELETED ON\n"
        "cDeletedAfter = SET('DELETED')\n"
        "SET DELETED TO 0\n"
        "cDeletedAfterToZero = SET('DELETED')\n"
        "cPathBefore = SET('PATH')\n"
        "lPathFileBefore = FILE('path_only_session.txt')\n"
        "SET PATH TO '" + path_probe_dir.string() + "'\n"
        "cPathAfter = SET('PATH')\n"
        "lPathFileAfter = FILE('path_only_session.txt')\n"
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
        "cExactSession2 = SET('EXACT')\n"
        "cDeletedSession2 = SET('DELETED')\n"
        "cPathSession2 = SET('PATH')\n"
        "cDefaultSession2 = SET('DEFAULT')\n"
        "lFileSession2 = FILE('people.dbf')\n"
        "lPathFileSession2 = FILE('path_only_session.txt')\n"
        "SET DATASESSION TO 1\n"
        "cNearRestored = SET('NEAR')\n"
        "cExactRestored = SET('EXACT')\n"
        "cDeletedRestored = SET('DELETED')\n"
        "cPathRestored = SET('PATH')\n"
        "cDefaultRestored = SET('DEFAULT')\n"
        "lFileRestored = FILE('people.dbf')\n"
        "lPathFileRestored = FILE('path_only_session.txt')\n"
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
    const auto near_after_to_false = state.globals.find("cnearaftertofalse");
    const auto exact_before = state.globals.find("cexactbefore");
    const auto exact_after = state.globals.find("cexactafter");
    const auto exact_after_to_string_off = state.globals.find("cexactaftertostringoff");
    const auto deleted_before = state.globals.find("cdeletedbefore");
    const auto deleted_after = state.globals.find("cdeletedafter");
    const auto deleted_after_to_zero = state.globals.find("cdeletedaftertozero");
    const auto path_before = state.globals.find("cpathbefore");
    const auto path_after = state.globals.find("cpathafter");
    const auto path_file_before = state.globals.find("lpathfilebefore");
    const auto path_file_after = state.globals.find("lpathfileafter");
    const auto default_before = state.globals.find("cdefaultbefore");
    const auto default_after = state.globals.find("cdefaultafter");
    const auto alias_from_eval = state.globals.find("caliasfromeval");
    const auto name_from_macro = state.globals.find("cnamefrommacro");
    const auto eval_age = state.globals.find("nevalage");
    const auto used_after_close = state.globals.find("lusedafterclose");
    const auto area_after_close = state.globals.find("nareaafterclose");
    const auto near_session2 = state.globals.find("cnearsession2");
    const auto exact_session2 = state.globals.find("cexactsession2");
    const auto deleted_session2 = state.globals.find("cdeletedsession2");
    const auto path_session2 = state.globals.find("cpathsession2");
    const auto default_session2 = state.globals.find("cdefaultsession2");
    const auto file_session2 = state.globals.find("lfilesession2");
    const auto path_file_session2 = state.globals.find("lpathfilesession2");
    const auto near_restored = state.globals.find("cnearrestored");
    const auto exact_restored = state.globals.find("cexactrestored");
    const auto deleted_restored = state.globals.find("cdeletedrestored");
    const auto path_restored = state.globals.find("cpathrestored");
    const auto default_restored = state.globals.find("cdefaultrestored");
    const auto file_restored = state.globals.find("lfilerestored");
    const auto path_file_restored = state.globals.find("lpathfilerestored");

    expect(near_before != state.globals.end(), "SET('NEAR') before enabling it should be captured");
    expect(near_after != state.globals.end(), "SET('NEAR') after enabling it should be captured");
    expect(near_after_to_false != state.globals.end(), "SET('NEAR') after SET NEAR TO .F. should be captured");
    expect(exact_before != state.globals.end(), "SET('EXACT') before enabling it should be captured");
    expect(exact_after != state.globals.end(), "SET('EXACT') after enabling it should be captured");
    expect(exact_after_to_string_off != state.globals.end(), "SET('EXACT') after SET EXACT TO 'OFF' should be captured");
    expect(deleted_before != state.globals.end(), "SET('DELETED') before enabling it should be captured");
    expect(deleted_after != state.globals.end(), "SET('DELETED') after enabling it should be captured");
    expect(deleted_after_to_zero != state.globals.end(), "SET('DELETED') after SET DELETED TO 0 should be captured");
    expect(path_before != state.globals.end(), "SET('PATH') before change should be captured");
    expect(path_after != state.globals.end(), "SET('PATH') after change should be captured");
    expect(path_file_before != state.globals.end(), "FILE() before SET PATH should be captured");
    expect(path_file_after != state.globals.end(), "FILE() after SET PATH should be captured");
    expect(default_before != state.globals.end(), "SET('DEFAULT') before change should be captured");
    expect(default_after != state.globals.end(), "SET('DEFAULT') after change should be captured");
    expect(alias_from_eval != state.globals.end(), "EVAL() should be able to evaluate runtime-state expressions");
    expect(name_from_macro != state.globals.end(), "&macro field resolution should be captured");
    expect(eval_age != state.globals.end(), "EVAL() of a stored expression should be captured");
    expect(used_after_close != state.globals.end(), "USE IN <expr> close semantics should be captured");
    expect(area_after_close != state.globals.end(), "SELECT('alias') after USE IN <expr> should be captured");
    expect(near_session2 != state.globals.end(), "SET('NEAR') in a fresh second session should be captured");
    expect(exact_session2 != state.globals.end(), "SET('EXACT') in a fresh second session should be captured");
    expect(deleted_session2 != state.globals.end(), "SET('DELETED') in a fresh second session should be captured");
    expect(path_session2 != state.globals.end(), "SET('PATH') in a fresh second session should be captured");
    expect(default_session2 != state.globals.end(), "SET('DEFAULT') in a fresh second session should be captured");
    expect(file_session2 != state.globals.end(), "FILE() in a fresh second session should be captured");
    expect(path_file_session2 != state.globals.end(), "FILE() for a SET PATH-only file in a fresh second session should be captured");
    expect(near_restored != state.globals.end(), "SET('NEAR') after restoring the original session should be captured");
    expect(exact_restored != state.globals.end(), "SET('EXACT') after restoring the original session should be captured");
    expect(deleted_restored != state.globals.end(), "SET('DELETED') after restoring the original session should be captured");
    expect(path_restored != state.globals.end(), "SET('PATH') after restoring the original session should be captured");
    expect(default_restored != state.globals.end(), "SET('DEFAULT') after restoring the original session should be captured");
    expect(file_restored != state.globals.end(), "FILE() after restoring the original session should be captured");
    expect(path_file_restored != state.globals.end(), "FILE() for a SET PATH-only file after restoring the original session should be captured");

    if (near_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(near_before->second) == "OFF", "SET('NEAR') should report OFF before it is enabled");
    }
    if (near_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(near_after->second) == "ON", "SET('NEAR') should report ON after SET NEAR ON");
    }
    if (near_after_to_false != state.globals.end()) {
        expect(copperfin::runtime::format_value(near_after_to_false->second) == "OFF", "SET('NEAR') should report OFF after SET NEAR TO .F.");
    }
    if (exact_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(exact_before->second) == "OFF", "SET('EXACT') should report OFF before it is enabled");
    }
    if (exact_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(exact_after->second) == "ON", "SET('EXACT') should report ON after SET EXACT ON");
    }
    if (exact_after_to_string_off != state.globals.end()) {
        expect(copperfin::runtime::format_value(exact_after_to_string_off->second) == "OFF", "SET('EXACT') should report OFF after SET EXACT TO 'OFF'");
    }
    if (deleted_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(deleted_before->second) == "OFF", "SET('DELETED') should report OFF before it is enabled");
    }
    if (deleted_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(deleted_after->second) == "ON", "SET('DELETED') should report ON after SET DELETED ON");
    }
    if (deleted_after_to_zero != state.globals.end()) {
        expect(copperfin::runtime::format_value(deleted_after_to_zero->second) == "OFF", "SET('DELETED') should report OFF after SET DELETED TO 0");
    }
    if (path_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(path_before->second) == "OFF", "SET('PATH') should report OFF before it is configured");
    }
    if (path_after != state.globals.end()) {
        const std::string normalized_path_after = lowercase_copy(copperfin::runtime::format_value(path_after->second));
        expect(normalized_path_after.find(lowercase_copy(path_probe_dir.string())) != std::string::npos,
               "SET('PATH') should include the configured search directory");
    }
    if (path_file_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(path_file_before->second) == "false",
               "FILE() should miss path-only files before SET PATH is configured");
    }
    if (path_file_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(path_file_after->second) == "true",
               "FILE() should find path-only files after SET PATH is configured");
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
    if (exact_session2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(exact_session2->second) == "OFF", "SET('EXACT') should stay isolated in a fresh data session");
    }
    if (deleted_session2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(deleted_session2->second) == "OFF", "SET('DELETED') should stay isolated in a fresh data session");
    }
    if (path_session2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(path_session2->second) == "OFF",
               "SET('PATH') should stay isolated in a fresh data session");
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
    if (path_file_session2 != state.globals.end()) {
        expect(
            copperfin::runtime::format_value(path_file_session2->second) == "false",
            "path-only files should not resolve in a fresh session without SET PATH configuration");
    }
    if (near_restored != state.globals.end()) {
        expect(copperfin::runtime::format_value(near_restored->second) == "OFF", "restoring the original data session should restore its toggled SET() state");
    }
    if (exact_restored != state.globals.end()) {
        expect(copperfin::runtime::format_value(exact_restored->second) == "OFF", "restoring the original data session should restore its toggled SET('EXACT') value");
    }
    if (deleted_restored != state.globals.end()) {
        expect(copperfin::runtime::format_value(deleted_restored->second) == "OFF", "restoring the original data session should restore its toggled SET('DELETED') value");
    }
    if (path_restored != state.globals.end()) {
        const std::string normalized_path_restored = lowercase_copy(copperfin::runtime::format_value(path_restored->second));
        expect(normalized_path_restored.find(lowercase_copy(path_probe_dir.string())) != std::string::npos,
               "restoring the original data session should restore its SET('PATH') value");
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
    if (path_file_restored != state.globals.end()) {
        expect(
            copperfin::runtime::format_value(path_file_restored->second) == "true",
            "path-only files should resolve again after restoring the session that configured SET PATH");
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


}  // namespace

int main() {
    test_read_events_pause();
    test_activate_popup_pause();
    test_dispatch_event_handler();
    test_local_variables_in_stack_frame();
    test_report_form_pause();
    test_label_form_pause();
    test_do_form_pause();
    test_do_command_macro_target();
    test_export_vfp_compatibility_corpus_script();
    test_work_area_and_data_session_compatibility();
    test_eval_macro_and_runtime_state_semantics();
    test_sql_and_ole_compatibility_functions();
    test_sql_pass_through_rows_affected_and_provider_hint();
    test_sql_prepare_and_connection_properties();

    if (copperfin::test_support::test_failures() != 0) {
        std::cerr << copperfin::test_support::test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }
    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
