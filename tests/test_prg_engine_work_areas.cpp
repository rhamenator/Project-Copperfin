// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "copperfin/runtime/prg_engine.h"
#include "copperfin/localization/localization.h"
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
#include <string_view>
#include <system_error>
#include <vector>

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

namespace {

using namespace copperfin::test_support;

std::string active_runtime_text(
    std::string_view key,
    const copperfin::localization::PlaceholderMap& placeholders = {}) {
    return copperfin::localization::load_catalogs(
        copperfin::localization::resolve_catalog_root(),
        copperfin::localization::select_locale()).translate(key, placeholders);
}

std::string active_call_depth_fault() {
    return active_runtime_text(
        "Runtime.Prg.Core.Error.RuntimeFault",
        {{"detail", active_runtime_text(
            "Runtime.Prg.Core.Error.GuardrailCallDepthExceeded",
            {{"limit", "1024"}})}});
}

void write_synthetic_report_surface(const std::filesystem::path& asset_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "HPOS", .type = 'N', .length = 10U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "WIDTH", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U}
    };
    const std::vector<std::vector<std::string>> records{
        {"9", "9", "detail header expression", "", "0", "", "200", "detail-header-guid"},
        {"8", "", "NAME", "100", "20", "700", "100", "name-field-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(asset_path.string(), fields, records);
    expect(create_result.ok, "synthetic report surface fixture should be created");
}

void write_synthetic_report_status_surface(
    const std::filesystem::path& asset_path,
    const std::string& expression = "SYS(2040)") {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "HPOS", .type = 'N', .length = 10U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "WIDTH", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U}
    };
    const std::vector<std::vector<std::string>> records{
        {"9", "9", "detail header expression", "", "0", "", "200", "detail-header-guid"},
        {"8", "", expression, "100", "20", "700", "100", "report-status-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(asset_path.string(), fields, records);
    expect(create_result.ok, "synthetic report status fixture should be created");
}

void write_named_age_dbf(
    const std::filesystem::path& path,
    const std::vector<std::pair<std::string, int>>& records) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "AGE", .type = 'N', .length = 3U}
    };

    std::vector<std::vector<std::string>> serialized_records;
    serialized_records.reserve(records.size());
    for (const auto& [name, age] : records) {
        serialized_records.push_back({name, std::to_string(age)});
    }

    const auto create_result = copperfin::vfp::create_dbf_table_file(path.string(), fields, serialized_records);
    expect(create_result.ok, "named-age DBF fixture should be created");
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
        "cDataSessionBefore = SET('DATASESSION')\n"
        "SELECT People\n"
        "nDataSessionCalls = 0\n"
        "SET DATASESSION TO data_session_id(2)\n"
        "cDataSessionAfter = SET('DATASESSION')\n"
        "cAlias2 = ALIAS()\n"
        "nCount2 = RECCOUNT()\n"
        "USE '" + table_path.string() + "' ALIAS SessionTwo IN 0\n"
        "cAlias3 = ALIAS()\n"
        "SET DATASESSION TO 1\n"
        "cAlias4 = ALIAS()\n"
        "USE IN People\n"
        "cAlias5 = ALIAS()\n"
        "RETURN\n"
        "FUNCTION data_session_id\n"
        "LPARAMETERS value\n"
        "nDataSessionCalls = nDataSessionCalls + 1\n"
        "RETURN value\n"
        "ENDFUNC\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "table/data-session script should complete");
    expect(state.work_area.data_session == 1, "runtime should finish back in data session 1");
    expect(state.work_area.aliases.empty(), "USE IN People should close the session-1 cursor");
    const auto data_session_calls = state.globals.find("ndatasessioncalls");
    expect(data_session_calls != state.globals.end(), "SET DATASESSION should preserve the resolver call counter");
    if (data_session_calls != state.globals.end()) {
        expect(copperfin::runtime::format_value(data_session_calls->second) == "1",
               "SET DATASESSION should evaluate the session UDF exactly once");
    }
    const auto data_session_before = state.globals.find("cdatasessionbefore");
    const auto data_session_after = state.globals.find("cdatasessionafter");
    expect(data_session_before != state.globals.end(), "SET(DATASESSION) before switching should be captured");
    expect(data_session_after != state.globals.end(), "SET(DATASESSION) after switching should be captured");
    if (data_session_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(data_session_before->second) == "1",
               "SET(DATASESSION) should report the initial data session");
    }
    if (data_session_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(data_session_after->second) == "2",
               "SET(DATASESSION) should report the active data session");
    }

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

void test_use_missing_target_uses_localized_error() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_use_missing_target";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path missing_stem = temp_root / "missing_people";
    const fs::path expected_path = missing_stem.string() + ".dbf";
    const fs::path main_path = temp_root / "use_missing_target.prg";
    write_text(
        main_path,
        "USE '" + missing_stem.string() + "' ALIAS Missing IN 0\n"
        "x = 2\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error,
           "missing USE target should pause with an error");
    expect(
        state.message == active_runtime_text(
            "Runtime.Prg.Cursor.Error.UseTargetResolveFailed",
            {{"path", expected_path.string()}}),
        "missing USE target error should route through the active locale catalog");

    const auto x_value = state.globals.find("x");
    expect(x_value == state.globals.end(), "statements after missing USE target should not execute");

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

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

void test_sys2040_report_status_tracks_preview_and_output() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sys2040_report_status";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path preview_asset = temp_root / "status_preview.frx";
    const fs::path preview_program = temp_root / "status_preview.prg";
    write_synthetic_report_surface(preview_asset);
    write_text(
        preview_program,
        "REPORT FORM '" + preview_asset.string() + "' PREVIEW\n"
        "nAfterPreview = SYS(2040)\n"
        "RETURN\n"
        "PROCEDURE closepreview\n"
        "CLEAR EVENTS\n"
        "RETURN\n"
        "ENDPROC\n");

    auto preview_session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(preview_program.string(), temp_root.string()));
    const auto preview_state = preview_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(preview_state.reason == copperfin::runtime::DebugPauseReason::event_loop,
           "SYS(2040) preview fixture should pause in the report event loop");
    const auto preview_status = preview_session.evaluate_watch_expression("SYS(2040)");
    expect(preview_status.ok && copperfin::runtime::format_value(preview_status.value) == "1",
           "SYS(2040) should report preview status while REPORT FORM PREVIEW is active");
    const auto preview_interrupted = preview_session.evaluate_watch_expression("SYS(2024)");
    expect(preview_interrupted.ok && copperfin::runtime::format_value(preview_interrupted.value) == "N",
           "SYS(2024) should report an uninterrupted active preview as character N");
    expect(preview_session.dispatch_event_handler("closepreview"),
           "SYS(2040) preview fixture should dispatch its cleanup handler");
    const auto after_preview_state = preview_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(after_preview_state.completed,
           "SYS(2040) preview fixture should complete after CLEAR EVENTS");
    const auto after_preview = after_preview_state.globals.find("nafterpreview");
    expect(after_preview != after_preview_state.globals.end() &&
               copperfin::runtime::format_value(after_preview->second) == "0",
           "SYS(2040) should reset to idle after preview cleanup");

    const fs::path interrupted_program = temp_root / "status_interrupted.prg";
    write_text(
        interrupted_program,
        "REPORT FORM '" + preview_asset.string() + "' PREVIEW\n"
        "RETURN\n"
        "PROCEDURE interruptpreview\n"
        "CANCEL\n"
        "RETURN\n"
        "ENDPROC\n");
    auto interrupted_session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(interrupted_program.string(), temp_root.string()));
    const auto interrupted_preview = interrupted_session.run(
        copperfin::runtime::DebugResumeAction::continue_run);
    expect(interrupted_preview.reason == copperfin::runtime::DebugPauseReason::event_loop,
           "SYS(2024) interruption fixture should enter the report event loop");
    expect(interrupted_session.dispatch_event_handler("interruptpreview"),
           "SYS(2024) interruption fixture should dispatch its cancellation handler");
    const auto interrupted_step_state = interrupted_session.run(
        copperfin::runtime::DebugResumeAction::step_into);
    expect(interrupted_step_state.reason == copperfin::runtime::DebugPauseReason::step,
           "SYS(2024) interruption should stop after executing the cancellation handler");
    const auto interrupted_public = interrupted_session.evaluate_watch_expression("SYS(2024)");
    expect(interrupted_public.ok &&
               interrupted_public.value.kind == copperfin::runtime::PrgValueKind::string &&
               copperfin::runtime::format_value(interrupted_public.value) == "Y",
           "SYS(2024) should return public character Y after CANCEL interrupts an active preview");
    const auto interrupted_event_state = interrupted_session.run(
        copperfin::runtime::DebugResumeAction::continue_run);
    expect(interrupted_event_state.reason == copperfin::runtime::DebugPauseReason::event_loop,
           "SYS(2024) interruption should restore the existing event-loop contract after handler dispatch");
    expect(std::any_of(
               interrupted_event_state.events.begin(),
               interrupted_event_state.events.end(),
               [](const copperfin::runtime::RuntimeEvent& event) {
                   return event.category == "report.interrupted" && event.detail == "Y";
               }),
           "SYS(2024) should record Y when CANCEL interrupts an active preview");

    const fs::path table_path = temp_root / "status_people.dbf";
    const fs::path output_path = temp_root / "status_output.txt";
    const fs::path output_program = temp_root / "status_output.prg";
    write_named_age_dbf(table_path, {{"Alice", 20}});
    write_synthetic_report_status_surface(temp_root / "status_output.frx", "SYS(2024)");
    write_text(
        output_program,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "REPORT FORM '" + (temp_root / "status_output.frx").string() + "' TO FILE '" + output_path.string() + "'\n"
        "RETURN\n");

    auto output_session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(output_program.string(), temp_root.string()));
    const auto output_state = output_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(output_state.completed, "SYS(2040) output fixture should complete");
    const std::string output_text = read_text(output_path);
    expect(output_text.find("object_exprs=1:N") != std::string::npos,
           "SYS(2024) should report an uninterrupted output as character N");

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

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

void test_report_and_label_to_file_emit_filtered_data_rows() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_report_filtered_rows";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_named_age_dbf(table_path, {{"Alice", 20}, {"Bob", 30}, {"Cara", 32}, {"Dana", 40}});

    const auto run_case = [&](const std::string& command, const fs::path& asset_path, const fs::path& output_path) {
        write_synthetic_report_surface(asset_path);

        const fs::path main_path = temp_root / (asset_path.stem().string() + ".prg");
        write_text(
            main_path,
            "USE '" + table_path.string() + "' ALIAS People IN 0\n"
            "SET FILTER TO AGE >= 20\n"
            + command + " '" + asset_path.string() + "' TO FILE '" + output_path.string() + "' FOR AGE <> 30 WHILE AGE < 35\n"
            "x = 2\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, command + " TO FILE with FOR/WHILE should complete");
        expect(fs::exists(output_path), command + " TO FILE with FOR/WHILE should materialize an output artifact");

        const std::string output_text = read_text(output_path);
        expect(output_text.find("sections=1") != std::string::npos,
               command + " TO FILE should preserve shared layout section metadata");
        expect(output_text.find("cursor=People") != std::string::npos,
               command + " TO FILE should identify the active work area");
        expect(output_text.find("set_filter=AGE >= 20") != std::string::npos,
               command + " TO FILE should surface the active cursor filter");
        expect(output_text.find("for=AGE <> 30") != std::string::npos,
               command + " TO FILE should surface the parsed FOR clause");
        expect(output_text.find("while=AGE < 35") != std::string::npos,
               command + " TO FILE should surface the parsed WHILE clause");
        expect(output_text.find("rows=2") != std::string::npos,
               command + " TO FILE should render only the qualifying rows");
        expect(output_text.find("row[1]=NAME=Alice|AGE=20") != std::string::npos,
               command + " TO FILE should render the first qualifying record");
        expect(output_text.find("row[1]=NAME=Alice|AGE=20|object_exprs=1:Alice") != std::string::npos,
               command + " TO FILE should evaluate the layout object expression for the first qualifying record");
        expect(output_text.find("row[3]=NAME=Cara|AGE=32") != std::string::npos,
               command + " TO FILE should render later qualifying records before the WHILE boundary");
        expect(output_text.find("row[3]=NAME=Cara|AGE=32|object_exprs=1:Cara") != std::string::npos,
               command + " TO FILE should evaluate the layout object expression for later qualifying records");
        expect(output_text.find("Bob") == std::string::npos,
               command + " TO FILE should exclude rows filtered by FOR");
        expect(output_text.find("Dana") == std::string::npos,
               command + " TO FILE should stop before rows outside the WHILE boundary");

        const auto x_value = state.globals.find("x");
        expect(x_value != state.globals.end(), command + " TO FILE should continue executing follow-on statements");
        if (x_value != state.globals.end()) {
            expect(copperfin::runtime::format_value(x_value->second) == "2",
                   command + " TO FILE should not block follow-on statements");
        }
    };

    run_case("REPORT FORM", temp_root / "filtered_report.frx", temp_root / "filtered_report.txt");
    run_case("LABEL FORM", temp_root / "filtered_label.lbx", temp_root / "filtered_label.txt");

    fs::remove_all(temp_root, ignored);
}

void test_report_form_missing_asset_uses_localized_error() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_report_missing_asset";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path report_path = temp_root / "missing_invoice.frx";
    const fs::path main_path = temp_root / "report_missing_asset.prg";
    write_text(
        main_path,
        "REPORT FORM '" + report_path.string() + "' PREVIEW\n"
        "x = 2\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error,
           "missing report asset should pause with an error");
    expect(
        state.message == active_runtime_text(
            "Runtime.Prg.ReportAsset.Error.ResolveFailed",
            {{"path", report_path.string()}}),
        "missing report asset error should route through the active locale catalog");

    const auto x_value = state.globals.find("x");
    expect(x_value == state.globals.end(), "statements after missing report asset should not execute");

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

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

void test_work_area_upper_boundary_wraps_without_overflow() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_work_area_upper_boundary";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path people_path = temp_root / "people.dbf";
    write_simple_dbf(people_path, {"ALPHA", "BRAVO"});
    const fs::path main_path = temp_root / "work_area_upper_boundary.prg";
    write_text(
        main_path,
        "SELECT 32767\n"
        "nBoundaryArea = SELECT()\n"
        "USE '" + people_path.string() + "' ALIAS People IN 0\n"
        "nWrappedArea = SELECT()\n"
        "cWrappedAlias = ALIAS()\n"
        "SELECT 32767\n"
        "nBoundaryAreaAgain = SELECT()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "work-area upper-boundary script should complete");

    const auto check = [&](const std::string &name, const std::string &expected, const std::string &message) {
        const auto found = state.globals.find(name);
        expect(found != state.globals.end(), message + " should be captured");
        if (found != state.globals.end()) {
            expect(copperfin::runtime::format_value(found->second) == expected, message);
        }
    };
    check("nboundaryarea", "32767", "SELECT 32767 should preserve the VFP upper work-area boundary");
    check("nwrappedarea", "1", "SELECT 0 after area 32767 should wrap to the lowest available area");
    check("cwrappedalias", "People", "wrapped automatic allocation should open the requested alias");
    check("nboundaryareaagain", "32767", "reselecting area 32767 after wrapping should remain stable");

    fs::remove_all(temp_root, ignored);
}

void test_work_area_above_vfp_boundary_fails_closed() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_work_area_out_of_range";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "work_area_out_of_range.prg";
    write_text(main_path, "SELECT 32768\nRETURN\n");
    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!state.completed, "out-of-range work-area script should fail closed");
    expect(state.reason == copperfin::runtime::DebugPauseReason::error,
           "out-of-range work-area selection should pause with an error");
    expect(state.message.find("SELECT target work area not found") != std::string::npos,
           "out-of-range work-area selection should use the existing target diagnostic");

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

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

void test_select_zero_skips_occupied_watermark_areas() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_select_zero_occupied_watermark";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path first_path = temp_root / "first.dbf";
    const fs::path second_path = temp_root / "second.dbf";
    const fs::path third_path = temp_root / "third.dbf";
    write_simple_dbf(first_path, {"FIRST"});
    write_simple_dbf(second_path, {"SECOND"});
    write_simple_dbf(third_path, {"THIRD"});

    const fs::path main_path = temp_root / "select_zero_occupied_watermark.prg";
    write_text(
        main_path,
        "USE '" + first_path.string() + "' ALIAS First IN 0\n"
        "SELECT 0\n"
        "USE '" + second_path.string() + "' ALIAS Second\n"
        "SELECT 0\n"
        "USE '" + third_path.string() + "' ALIAS Third\n"
        "USE IN First\n"
        "SELECT 0\n"
        "nFirstSelected = SELECT()\n"
        "SELECT 0\n"
        "nSecondSelected = SELECT()\n"
        "cSecondAlias = ALIAS('Second')\n"
        "cThirdAlias = ALIAS('Third')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SELECT(0) occupied-watermark script should complete");

    const auto first_selected = state.globals.find("nfirstselected");
    const auto second_selected = state.globals.find("nsecondselected");
    const auto second_alias = state.globals.find("csecondalias");
    const auto third_alias = state.globals.find("cthirdalias");
    expect(first_selected != state.globals.end(), "first SELECT 0 result should be captured");
    expect(second_selected != state.globals.end(), "second SELECT 0 result should be captured");
    expect(second_alias != state.globals.end(), "occupied second work-area alias should remain readable");
    expect(third_alias != state.globals.end(), "occupied third work-area alias should remain readable");
    if (first_selected != state.globals.end()) {
        expect(copperfin::runtime::format_value(first_selected->second) == "1",
               "SELECT 0 should first reuse the lowest freed work area");
    }
    if (second_selected != state.globals.end()) {
        expect(copperfin::runtime::format_value(second_selected->second) == "4",
               "SELECT 0 should skip occupied work areas after the allocation watermark");
    }
    if (second_alias != state.globals.end()) {
        expect(copperfin::runtime::format_value(second_alias->second) == "Second",
               "SELECT(0) should not replace the occupied second work area");
    }
    if (third_alias != state.globals.end()) {
        expect(copperfin::runtime::format_value(third_alias->second) == "Third",
               "SELECT(0) should not replace the occupied third work area");
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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

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
        expect(copperfin::runtime::format_value(after_bof->second) == "1", "RECNO() at BOF should remain at the first record number");
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

void test_ordered_go_and_skip_cursor_navigation() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_ordered_navigation";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_simple_dbf(table_path, {"ZEBRA", "APPLE", "MANGO"});
    const fs::path index_path = temp_root / "people.idx";
    write_synthetic_idx(index_path, "NAME");

    const fs::path main_path = temp_root / "ordered_navigate.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO 1\n"
        "GO TOP\n"
        "cTop = NAME\n"
        "SKIP\n"
        "cNext = NAME\n"
        "SKIP -1\n"
        "cPrevious = NAME\n"
        "GO BOTTOM\n"
        "cBottom = NAME\n"
        "SKIP -1\n"
        "cBack = NAME\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "ordered navigation script should complete");

    const auto top = state.globals.find("ctop");
    const auto next = state.globals.find("cnext");
    const auto previous = state.globals.find("cprevious");
    const auto bottom = state.globals.find("cbottom");
    const auto back = state.globals.find("cback");
    expect(top != state.globals.end(), "ordered GO TOP should expose the first key");
    expect(next != state.globals.end(), "ordered SKIP should expose the next key");
    expect(previous != state.globals.end(), "ordered reverse SKIP should expose the previous key");
    expect(bottom != state.globals.end(), "ordered GO BOTTOM should expose the last key");
    expect(back != state.globals.end(), "ordered reverse SKIP from the bottom should expose the previous key");
    if (top != state.globals.end()) {
        expect(copperfin::runtime::format_value(top->second) == "APPLE", "ordered GO TOP should use the lowest active-order key");
    }
    if (next != state.globals.end()) {
        expect(copperfin::runtime::format_value(next->second) == "MANGO", "ordered SKIP should advance in active-order sequence");
    }
    if (previous != state.globals.end()) {
        expect(copperfin::runtime::format_value(previous->second) == "APPLE", "ordered reverse SKIP should move backward in active-order sequence");
    }
    if (bottom != state.globals.end()) {
        expect(copperfin::runtime::format_value(bottom->second) == "ZEBRA", "ordered GO BOTTOM should use the highest active-order key");
    }
    if (back != state.globals.end()) {
        expect(copperfin::runtime::format_value(back->second) == "MANGO", "ordered reverse SKIP from the bottom should use active-order sequence");
    }

    fs::remove_all(temp_root, ignored);
}

void test_skip_delta_uses_heap_backed_frame_continuations() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_skip_continuation";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_simple_dbf(table_path, {"ALPHA", "BRAVO", "CHARLIE"});

    const fs::path deep_path = temp_root / "skip_deep.prg";
    write_text(
        deep_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "GO TOP\n"
        "SKIP skip_delta(2048)\n"
        "RETURN\n"
        "FUNCTION skip_delta\n"
        "LPARAMETERS nDepth\n"
        "IF nDepth <= 0\n"
        "RETURN 1\n"
        "ENDIF\n"
        "RETURN skip_delta(nDepth - 1)\n"
        "ENDFUNC\n");

    copperfin::runtime::PrgRuntimeSession deep_session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(deep_path.string(), temp_root.string()));
    const auto deep_state = deep_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!deep_state.completed, "deep SKIP delta recursion should stop at the configured call-depth guard");
    expect(
        deep_state.message == active_call_depth_fault(),
        "deep SKIP delta recursion should report Copperfin's call-depth diagnostic instead of overflowing the host stack");

    const fs::path side_effect_path = temp_root / "skip_side_effect.prg";
    write_text(
        side_effect_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "GO TOP\n"
        "nCalls = 0\n"
        "SKIP skip_delta()\n"
        "nRec = RECNO()\n"
        "RETURN\n"
        "FUNCTION skip_delta\n"
        "nCalls = nCalls + 1\n"
        "RETURN 1\n"
        "ENDFUNC\n");

    copperfin::runtime::PrgRuntimeSession side_effect_session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(side_effect_path.string(), temp_root.string()));
    const auto side_effect_state = side_effect_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(side_effect_state.completed, "SKIP delta UDF script should complete");

    const auto calls = side_effect_state.globals.find("ncalls");
    const auto record = side_effect_state.globals.find("nrec");
    expect(calls != side_effect_state.globals.end(), "SKIP delta UDF call count should be captured");
    expect(record != side_effect_state.globals.end(), "SKIP should still expose RECNO() after a resumed delta evaluation");
    if (calls != side_effect_state.globals.end())
    {
        expect(copperfin::runtime::format_value(calls->second) == "1", "SKIP delta UDF should run exactly once across suspension and resume");
    }
    if (record != side_effect_state.globals.end())
    {
        expect(copperfin::runtime::format_value(record->second) == "2", "resumed SKIP should move by the UDF-provided delta");
    }

    fs::remove_all(temp_root, ignored);
}

void test_go_record_expression_uses_heap_backed_frame_continuations() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_go_continuation";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_simple_dbf(table_path, {"ALPHA", "BRAVO", "CHARLIE"});

    const fs::path deep_path = temp_root / "go_deep.prg";
    write_text(
        deep_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "GO go_record(2048)\n"
        "RETURN\n"
        "FUNCTION go_record\n"
        "LPARAMETERS nDepth\n"
        "IF nDepth <= 0\n"
        "RETURN 2\n"
        "ENDIF\n"
        "RETURN go_record(nDepth - 1)\n"
        "ENDFUNC\n");

    copperfin::runtime::PrgRuntimeSession deep_session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(deep_path.string(), temp_root.string()));
    const auto deep_state = deep_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!deep_state.completed, "deep GO record recursion should stop at the configured call-depth guard");
    expect(
        deep_state.message == active_call_depth_fault(),
        "deep GO record recursion should report Copperfin's call-depth diagnostic instead of overflowing the host stack");

    const fs::path side_effect_path = temp_root / "go_side_effect.prg";
    write_text(
        side_effect_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "nCalls = 0\n"
        "GO go_record()\n"
        "nRec = RECNO()\n"
        "RETURN\n"
        "FUNCTION go_record\n"
        "nCalls = nCalls + 1\n"
        "RETURN 2\n"
        "ENDFUNC\n");

    copperfin::runtime::PrgRuntimeSession side_effect_session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(side_effect_path.string(), temp_root.string()));
    const auto side_effect_state = side_effect_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(side_effect_state.completed, "GO record UDF script should complete");

    const auto calls = side_effect_state.globals.find("ncalls");
    const auto record = side_effect_state.globals.find("nrec");
    expect(calls != side_effect_state.globals.end(), "GO record UDF call count should be captured");
    expect(record != side_effect_state.globals.end(), "GO should still expose RECNO() after a resumed record evaluation");
    if (calls != side_effect_state.globals.end())
    {
        expect(copperfin::runtime::format_value(calls->second) == "1", "GO record UDF should run exactly once across suspension and resume");
    }
    if (record != side_effect_state.globals.end())
    {
        expect(copperfin::runtime::format_value(record->second) == "2", "resumed GO should move to the UDF-provided record");
    }

    fs::remove_all(temp_root, ignored);
}

void test_unlock_record_expression_uses_heap_backed_frame_continuations() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_unlock_continuation";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_simple_dbf(table_path, {"ALPHA", "BRAVO", "CHARLIE"});

    const fs::path deep_path = temp_root / "unlock_deep.prg";
    write_text(
        deep_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "UNLOCK RECORD unlock_record(2048) IN People\n"
        "RETURN\n"
        "FUNCTION unlock_record\n"
        "LPARAMETERS nDepth\n"
        "IF nDepth <= 0\n"
        "RETURN 1\n"
        "ENDIF\n"
        "RETURN unlock_record(nDepth - 1)\n"
        "ENDFUNC\n");

    copperfin::runtime::PrgRuntimeSession deep_session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(deep_path.string(), temp_root.string()));
    const auto deep_state = deep_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!deep_state.completed, "deep UNLOCK RECORD recursion should stop at the configured call-depth guard");
    expect(
        deep_state.message == active_call_depth_fault(),
        "deep UNLOCK RECORD recursion should report Copperfin's call-depth diagnostic instead of overflowing the host stack");

    const fs::path side_effect_path = temp_root / "unlock_side_effect.prg";
    write_text(
        side_effect_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "nCalls = 0\n"
        "UNLOCK RECORD unlock_record() IN People\n"
        "RETURN\n"
        "FUNCTION unlock_record\n"
        "nCalls = nCalls + 1\n"
        "RETURN 1\n"
        "ENDFUNC\n");

    copperfin::runtime::PrgRuntimeSession side_effect_session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(side_effect_path.string(), temp_root.string()));
    const auto side_effect_state = side_effect_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(side_effect_state.completed, "UNLOCK RECORD UDF script should complete");

    const auto calls = side_effect_state.globals.find("ncalls");
    expect(calls != side_effect_state.globals.end(), "UNLOCK RECORD UDF call count should be captured");
    if (calls != side_effect_state.globals.end())
    {
        expect(copperfin::runtime::format_value(calls->second) == "1", "UNLOCK RECORD UDF should run exactly once across suspension and resume");
    }
    expect(
        has_runtime_event(side_effect_state.events, "runtime.unlock", "People RECORD 1"),
        "resumed UNLOCK RECORD should preserve the existing runtime.unlock event detail");

    fs::remove_all(temp_root, ignored);
}

void test_use_target_expression_uses_heap_backed_frame_continuations() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_use_continuation";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_simple_dbf(table_path, {"ALPHA", "BRAVO", "CHARLIE"});

    const fs::path deep_path = temp_root / "use_deep.prg";
    write_text(
        deep_path,
        "USE use_target(2048) ALIAS People IN 0\n"
        "RETURN\n"
        "FUNCTION use_target\n"
        "LPARAMETERS nDepth\n"
        "IF nDepth <= 0\n"
        "RETURN '" + table_path.string() + "'\n"
        "ENDIF\n"
        "RETURN use_target(nDepth - 1)\n"
        "ENDFUNC\n");

    copperfin::runtime::PrgRuntimeSession deep_session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(deep_path.string(), temp_root.string()));
    const auto deep_state = deep_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!deep_state.completed, "deep USE target recursion should stop at the configured call-depth guard");
    expect(
        deep_state.message == active_call_depth_fault(),
        "deep USE target recursion should report Copperfin's call-depth diagnostic instead of overflowing the host stack");

    const fs::path side_effect_path = temp_root / "use_side_effect.prg";
    write_text(
        side_effect_path,
        "nCalls = 0\n"
        "nAliasCalls = 0\n"
        "USE use_target() ALIAS alias_target('People') IN 0\n"
        "cAlias = ALIAS()\n"
        "RETURN\n"
        "FUNCTION use_target\n"
        "nCalls = nCalls + 1\n"
        "RETURN '" + table_path.string() + "'\n"
        "ENDFUNC\n"
        "FUNCTION alias_target\n"
        "LPARAMETERS value\n"
        "nAliasCalls = nAliasCalls + 1\n"
        "RETURN value\n"
        "ENDFUNC\n");

    copperfin::runtime::PrgRuntimeSession side_effect_session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(side_effect_path.string(), temp_root.string()));
    const auto side_effect_state = side_effect_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(side_effect_state.completed, "USE target UDF script should complete");

    const auto calls = side_effect_state.globals.find("ncalls");
    const auto alias_calls = side_effect_state.globals.find("naliascalls");
    const auto alias = side_effect_state.globals.find("calias");
    expect(calls != side_effect_state.globals.end(), "USE target UDF call count should be captured");
    expect(alias_calls != side_effect_state.globals.end(), "USE alias UDF call count should be captured");
    expect(alias != side_effect_state.globals.end(), "USE should still expose the selected alias after a resumed target evaluation");
    if (calls != side_effect_state.globals.end())
    {
        expect(copperfin::runtime::format_value(calls->second) == "1", "USE target UDF should run exactly once across suspension and resume");
    }
    if (alias_calls != side_effect_state.globals.end())
    {
        expect(copperfin::runtime::format_value(alias_calls->second) == "1", "USE alias UDF should run exactly once across suspension and resume");
    }
    if (alias != side_effect_state.globals.end())
    {
        expect(copperfin::runtime::format_value(alias->second) == "People", "resumed USE should preserve the explicit alias");
    }

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

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

void test_set_fields_limits_field_lookup() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_set_fields";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 11}, {"BRAVO", 22}});

    const fs::path main_path = temp_root / "set_fields.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "cFieldsDefault = SET('FIELDS')\n"
        "cNameBefore = NAME\n"
        "nAgeBefore = AGE\n"
        "SET FIELDS TO NAME\n"
        "cFieldsLimited = SET('FIELDS')\n"
        "cNameLimited = NAME\n"
        "xAgeLimited = AGE\n"
        "xAliasAgeLimited = People.AGE\n"
        "SET FIELDS OFF\n"
        "cFieldsOff = SET('FIELDS')\n"
        "nAgeOff = AGE\n"
        "SET FIELDS ON\n"
        "cFieldsOn = SET('FIELDS')\n"
        "xAgeOn = AGE\n"
        "SET FIELDS TO ALL\n"
        "cFieldsAll = SET('FIELDS')\n"
        "nAgeAll = AGE\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SET FIELDS field-lookup script should complete");

    const auto check = [&](const std::string& name, const std::string& expected) {
        const auto it = state.globals.find(name);
        if (it == state.globals.end()) {
            expect(false, name + " variable not found");
            return;
        }
        expect(copperfin::runtime::format_value(it->second) == expected,
               name + " expected '" + expected + "' got '" + copperfin::runtime::format_value(it->second) + "'");
    };

    check("cfieldsdefault", "OFF");
    check("cnamebefore", "ALPHA");
    check("nagebefore", "11");
    check("cfieldslimited", "NAME");
    check("cnamelimited", "ALPHA");
    check("xagelimited", "");
    check("xaliasagelimited", "");
    check("cfieldsoff", "OFF");
    check("nageoff", "11");
    check("cfieldson", "NAME");
    check("xageon", "");
    check("cfieldsall", "ALL");
    check("nageall", "11");

    fs::remove_all(temp_root, ignored);
}

void test_set_fields_like_and_except_limit_field_lookup() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_set_fields_like_except";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .length = 12U},
        {.name = "NOTE", .type = 'C', .length = 12U},
        {.name = "AGE", .type = 'N', .length = 3U},
    };
    const std::vector<std::vector<std::string>> records{
        {"ALPHA", "Memo", "11"},
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "SET FIELDS LIKE/EXCEPT fixture should be created");

    const fs::path main_path = temp_root / "set_fields_like_except.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET FIELDS TO LIKE N*\n"
        "cFieldsLike = SET('FIELDS')\n"
        "cNameLike = NAME\n"
        "cNoteLike = NOTE\n"
        "xAgeLike = AGE\n"
        "SET FIELDS TO EXCEPT NOTE\n"
        "cFieldsExcept = SET('FIELDS')\n"
        "cNameExcept = NAME\n"
        "xNoteExcept = NOTE\n"
        "nAgeExcept = AGE\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SET FIELDS LIKE/EXCEPT field-lookup script should complete");

    const auto check = [&](const std::string& name, const std::string& expected) {
        const auto it = state.globals.find(name);
        if (it == state.globals.end()) {
            expect(false, name + " variable not found");
            return;
        }
        expect(copperfin::runtime::format_value(it->second) == expected,
               name + " expected '" + expected + "' got '" + copperfin::runtime::format_value(it->second) + "'");
    };

    check("cfieldslike", "LIKE N*");
    check("cnamelike", "ALPHA");
    check("cnotelike", "Memo");
    check("xagelike", "");
    check("cfieldsexcept", "EXCEPT NOTE");
    check("cnameexcept", "ALPHA");
    check("xnoteexcept", "");
    check("nageexcept", "11");

    fs::remove_all(temp_root, ignored);
}

void test_set_fields_is_scoped_by_data_session() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_set_fields_datasession";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 11}, {"BRAVO", 22}});

    const fs::path main_path = temp_root / "set_fields_datasession.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET FIELDS TO NAME\n"
        "cFieldsSession1 = SET('FIELDS')\n"
        "xAgeSession1 = AGE\n"
        "SET DATASESSION TO 2\n"
        "USE '" + table_path.string() + "' ALIAS PeopleTwo IN 0\n"
        "cFieldsSession2 = SET('FIELDS')\n"
        "nAgeSession2 = AGE\n"
        "SET FIELDS TO AGE\n"
        "cFieldsSession2After = SET('FIELDS')\n"
        "xNameSession2After = NAME\n"
        "SET DATASESSION TO 1\n"
        "cFieldsSession1Restored = SET('FIELDS')\n"
        "xAgeSession1Restored = AGE\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SET FIELDS data-session isolation script should complete");

    const auto check = [&](const std::string& name, const std::string& expected) {
        const auto it = state.globals.find(name);
        if (it == state.globals.end()) {
            expect(false, name + " variable not found");
            return;
        }
        expect(copperfin::runtime::format_value(it->second) == expected,
               name + " expected '" + expected + "' got '" + copperfin::runtime::format_value(it->second) + "'");
    };

    check("cfieldssession1", "NAME");
    check("xagesession1", "");
    check("cfieldssession2", "OFF");
    check("nagesession2", "11");
    check("cfieldssession2after", "AGE");
    check("xnamesession2after", "");
    check("cfieldssession1restored", "NAME");
    check("xagesession1restored", "");

    fs::remove_all(temp_root, ignored);
}


}  // namespace

void test_two_work_areas_on_same_table_see_consistent_mutations();  // defined below
void test_getnextmodified_traverses_buffered_records();

int main() {
    test_use_and_data_session_isolation();
    test_use_missing_target_uses_localized_error();
    test_report_form_to_file_renders_without_event_loop_pause();
    test_sys2040_report_status_tracks_preview_and_output();
    test_label_form_to_file_renders_without_event_loop_pause();
    test_report_and_label_to_file_emit_filtered_data_rows();
    test_report_form_missing_asset_uses_localized_error();
    test_cross_session_alias_and_work_area_isolation();
    test_use_in_existing_alias_reuses_target_work_area();
    test_use_in_nonselected_alias_preserves_selected_work_area();
    test_plain_use_reuses_current_selected_work_area();
    test_work_area_upper_boundary_wraps_without_overflow();
    test_work_area_above_vfp_boundary_fails_closed();
    test_select_and_use_in_designator_expressions();
    test_expression_driven_in_targeting_across_local_data_commands();
    test_select_zero_and_use_in_zero_reuse_closed_work_area();
    test_select_zero_skips_occupied_watermark_areas();
    test_go_and_skip_cursor_navigation();
    test_ordered_go_and_skip_cursor_navigation();
    test_skip_delta_uses_heap_backed_frame_continuations();
    test_go_record_expression_uses_heap_backed_frame_continuations();
    test_unlock_record_expression_uses_heap_backed_frame_continuations();
    test_use_target_expression_uses_heap_backed_frame_continuations();
    test_cursor_identity_functions_for_local_tables();
    test_local_use_auto_allocation_tracks_session_selection_flow();
    test_local_selected_empty_area_reuses_after_datasession_round_trip();
    test_set_fields_limits_field_lookup();
    test_set_fields_like_and_except_limit_field_lookup();
    test_set_fields_is_scoped_by_data_session();
    test_getnextmodified_traverses_buffered_records();
    test_two_work_areas_on_same_table_see_consistent_mutations();

    if (copperfin::test_support::test_failures() != 0) {
        std::cerr << copperfin::test_support::test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }
    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}

void test_getnextmodified_traverses_buffered_records() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_getnextmodified";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}});
    const fs::path main_path = temp_root / "getnextmodified.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People\n"
        "PUBLIC nErrorCount, nError1, nError2, nError3, nError4, nError5\n"
        "PUBLIC cError1, cError2, cError3, cError4, cError5\n"
        "nErrorCount = 0\n"
        "nError1 = 0\n"
        "nError2 = 0\n"
        "nError3 = 0\n"
        "nError4 = 0\n"
        "nError5 = 0\n"
        "cError1 = ''\n"
        "cError2 = ''\n"
        "cError3 = ''\n"
        "cError4 = ''\n"
        "cError5 = ''\n"
        "ON ERROR DO HandleGetNextModifiedError\n"
        "nNoArguments = GETNEXTMODIFIED()\n"
        "nRowBufferingDisabled = GETNEXTMODIFIED(0)\n"
        "=CURSORSETPROP('Buffering', 5, 'People')\n"
        "nMissingAlias = GETNEXTMODIFIED(0, 'MissingAlias')\n"
        "ON ERROR\n"
        "=CURSORSETPROP('Buffering', 5, 'People')\n"
        "GO 1\n"
        "REPLACE NAME WITH 'CHANGED'\n"
        "GO 3\n"
        "REPLACE AGE WITH 99\n"
        "nFirst = GETNEXTMODIFIED(0)\n"
        "nSecond = GETNEXTMODIFIED(nFirst)\n"
        "nNone = GETNEXTMODIFIED(nSecond)\n"
        "nAlias = GETNEXTMODIFIED(0, 'People')\n"
        "nWorkArea = GETNEXTMODIFIED(0, 1, .T.)\n"
        "=TABLEREVERT(.T., 'People')\n"
        "nAfterRevert = GETNEXTMODIFIED(0, 'People')\n"
        "ON ERROR DO HandleGetNextModifiedError\n"
        "=CURSORSETPROP('Buffering', 3, 'People')\n"
        "GO 1\n"
        "REPLACE NAME WITH 'ROW-OPTIMISTIC'\n"
        "nRowBufferedOptimistic = GETNEXTMODIFIED(0, 'People')\n"
        "=TABLEREVERT(.T., 'People')\n"
        "=CURSORSETPROP('Buffering', 2, 'People')\n"
        "GO 1\n"
        "REPLACE NAME WITH 'ROW-PESSIMISTIC'\n"
        "nRowBufferedPessimistic = GETNEXTMODIFIED(0, 'People')\n"
        "ON ERROR\n"
        "RETURN\n"
        "PROCEDURE HandleGetNextModifiedError\n"
        "nErrorCount = nErrorCount + 1\n"
        "IF nErrorCount = 1\n"
        "    nError1 = ERROR()\n"
        "    cError1 = MESSAGE()\n"
        "ELSEIF nErrorCount = 2\n"
        "    nError2 = ERROR()\n"
        "    cError2 = MESSAGE()\n"
        "ELSEIF nErrorCount = 3\n"
        "    nError3 = ERROR()\n"
        "    cError3 = MESSAGE()\n"
        "ELSEIF nErrorCount = 4\n"
        "    nError4 = ERROR()\n"
        "    cError4 = MESSAGE()\n"
        "ELSEIF nErrorCount = 5\n"
        "    nError5 = ERROR()\n"
        "    cError5 = MESSAGE()\n"
        "ENDIF\n"
        "RETURN\n"
        "ENDPROC\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "GETNEXTMODIFIED buffered-record script should complete");

    const auto check = [&](const std::string& name, const std::string& expected)
    {
        const auto it = state.globals.find(name);
        expect(it != state.globals.end(), name + " result should be present");
        if (it != state.globals.end()) {
            expect(copperfin::runtime::format_value(it->second) == expected,
                   name + " expected " + expected + " but got " + copperfin::runtime::format_value(it->second));
        }
    };
    check("nfirst", "1");
    check("nsecond", "3");
    check("nnone", "0");
    check("nalias", "1");
    check("nworkarea", "1");
    check("nafterrevert", "0");
    check("nerrorcount", "5");
    check("nerror1", "1229");
    check("nerror2", "1596");
    check("nerror3", "13");
    check("nerror4", "1596");
    check("nerror5", "1596");

    const auto catalog = copperfin::localization::load_catalogs(
        copperfin::localization::resolve_catalog_root(),
        copperfin::localization::select_locale());
    const std::string too_few_arguments = catalog.translate(
        "Runtime.Prg.Records.Error.TooFewArguments");
    const std::string table_buffering_not_enabled = catalog.translate(
        "Runtime.Prg.Records.Error.TableBufferingNotEnabled");
    const std::string alias_not_found = catalog.translate(
        "Runtime.Prg.Records.Error.AliasNotFound");
    check("cerror1", too_few_arguments);
    check("cerror2", table_buffering_not_enabled);
    check("cerror3", alias_not_found);
    check("cerror4", table_buffering_not_enabled);
    check("cerror5", table_buffering_not_enabled);

    fs::remove_all(temp_root, ignored);
}

void test_two_work_areas_on_same_table_see_consistent_mutations() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_two_areas_same_table";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    copperfin::test_support::write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}});

    const fs::path main_path = temp_root / "two_areas.prg";
    copperfin::test_support::write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS Area1 IN 0\n"
            "USE '" + table_path.string() + "' ALIAS Area2 AGAIN IN 0\n"
        "SELECT Area1\n"
        "GO 1\n"
        "REPLACE NAME WITH 'MUTATED'\n"
        "SELECT Area2\n"
        "GO 1\n"
        "cArea2Name = NAME\n"
        "nArea2Age = AGE\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    copperfin::test_support::expect(state.completed, "two-work-areas same table script should complete");

    const auto area2_name = state.globals.find("carea2name");
    const auto area2_age = state.globals.find("narea2age");

    copperfin::test_support::expect(area2_name != state.globals.end(), "Area2 NAME after mutation via Area1 should be captured");
    copperfin::test_support::expect(area2_age != state.globals.end(), "Area2 AGE after mutation via Area1 should be captured");

    if (area2_name != state.globals.end()) {
        copperfin::test_support::expect(
            copperfin::runtime::format_value(area2_name->second) == "MUTATED",
            "mutation via Area1 should be visible when reading the same record via Area2");
    }
    if (area2_age != state.globals.end()) {
        copperfin::test_support::expect(
            copperfin::runtime::format_value(area2_age->second) == "10",
            "unreplaced fields should retain their original values when reading via Area2");
    }

    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 5U);
    copperfin::test_support::expect(parse_result.ok, "DBF should remain readable after two-work-area mutation");
    if (parse_result.table.records.size() >= 1U) {
        copperfin::test_support::expect(
            parse_result.table.records[0].values[0].display_value == "MUTATED",
            "mutation via Area1 should be persisted on disk");
    }

    fs::remove_all(temp_root, ignored);
}
