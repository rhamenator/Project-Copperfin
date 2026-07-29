// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

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
#include <locale>
#include <sstream>
#include <system_error>
#include <vector>

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

void test_verified_source_errors_are_localized();






namespace {

using namespace copperfin::test_support;

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

void test_insert_select_numeric_serialization_ignores_global_locale() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_insert_select_locale";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path source_path = temp_root / "source.dbf";
    const fs::path destination_path = temp_root / "destination.dbf";
    const fs::path main_path = temp_root / "main.prg";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "VALUE", .type = 'N', .offset = 1U, .length = 10U, .decimal_count = 2U}
    };
    const auto source_create = copperfin::vfp::create_dbf_table_file(
        source_path.string(), fields, {{"1.25"}});
    const auto destination_create = copperfin::vfp::create_dbf_table_file(
        destination_path.string(), fields, {});
    expect(source_create.ok && destination_create.ok,
           "INSERT SELECT locale fixtures should be created");
    write_text(
        main_path,
        "USE '" + source_path.string() + "' ALIAS SourceRows IN 0\n"
        "USE '" + destination_path.string() + "' ALIAS DestinationRows IN 0\n"
        "INSERT INTO DestinationRows (VALUE) SELECT VALUE + 0.25 FROM SourceRows\n"
        "SELECT DestinationRows\n"
        "nInserted = RECCOUNT()\n"
        "RETURN\n");

    const std::locale comma_locale(std::locale::classic(), new comma_decimal_numpunct());
    scoped_global_locale locale_guard(comma_locale);
    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed,
           "INSERT SELECT numeric serialization should remain executable under a comma-decimal global locale: " +
               state.message);
    const auto inserted = state.globals.find("ninserted");
    expect(inserted != state.globals.end() &&
               copperfin::runtime::format_value(inserted->second) == "1",
           "INSERT SELECT should materialize one row when numeric text remains period-decimal");

    fs::remove_all(temp_root, ignored);
}

void test_runtime_session_options_contain_temporary_files() {
    namespace fs = std::filesystem;
    const fs::path fixture_root = fs::absolute(fs::temp_directory_path() / "copperfin_prg_engine_options");
    std::error_code ignored;
    fs::remove_all(fixture_root, ignored);
    fs::create_directories(fixture_root / "process-working-directory");
    const fs::path startup_path = fixture_root / "main.prg";

    const auto implicit_options = make_runtime_session_options(startup_path, fs::path{});
    expect(implicit_options.working_directory.empty(),
           "empty working directory should remain empty in runtime session options");
    expect(fs::path(implicit_options.temp_directory) == fixture_root / "runtime-temp",
           "empty working directory should keep runtime temp files under the absolute startup fixture root");
    expect(fs::path(implicit_options.temp_directory).is_absolute(),
           "empty working directory should never produce a process-relative runtime temp path");

    const auto fallback_options = make_runtime_session_options(fs::path("main.prg"), fs::path{});
    const fs::path expected_fallback = fs::absolute(fs::temp_directory_path()) /
        "copperfin-prg-engine-tests" / "runtime-temp";
    expect(fs::path(fallback_options.temp_directory) == expected_fallback,
           "relative startup path should use the named Copperfin test area under the absolute OS temp directory");

    const fs::path explicit_working_directory = "relative-test-working-directory";
    const auto explicit_options = make_runtime_session_options(startup_path, explicit_working_directory);
    expect(fs::path(explicit_options.working_directory) == explicit_working_directory,
           "explicit working directory should remain unchanged in runtime session options");
    expect(fs::path(explicit_options.temp_directory) == explicit_working_directory / "runtime-temp",
           "explicit working directory should retain its existing runtime temp path behavior");

    const fs::path table_path = fixture_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}});
    write_text(
        startup_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "REPLACE NAME WITH 'CHANGED'\n"
        "UNDO\n"
        "RETURN\n");

    const fs::path original_process_directory = fs::current_path();
    const fs::path simulated_process_directory = fixture_root / "process-working-directory";
    fs::current_path(simulated_process_directory);
    {
        auto session = copperfin::runtime::PrgRuntimeSession::create(implicit_options);
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "empty-working-directory command-undo fixture should complete");
        expect(has_runtime_event(state.events, "runtime.command_undo", "LATEST"),
               "empty-working-directory fixture should exercise command-undo journal creation and replay");
    }
    fs::current_path(original_process_directory);

    expect(!fs::exists(simulated_process_directory / "runtime-temp"),
           "empty working directory should not create runtime-temp under the process working directory");

#if !defined(_WIN32)
    const fs::path temp_status_loop = fixture_root / "temp-status-loop";
    std::error_code temp_status_error;
    fs::create_symlink(temp_status_loop, temp_status_loop, temp_status_error);
    if (!temp_status_error) {
        auto status_options = implicit_options;
        status_options.temp_directory = temp_status_loop.string();
        auto status_session = copperfin::runtime::PrgRuntimeSession::create(status_options);
        const auto status_state = status_session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(status_state.completed,
               "#4401: runtime session should fall back when explicit temp-directory status fails");
        const auto config_event = std::find_if(
            status_state.events.begin(),
            status_state.events.end(),
            [](const copperfin::runtime::RuntimeEvent& event) {
                return event.category == "runtime.config";
            });
        expect(config_event != status_state.events.end() &&
                   config_event->detail.find(temp_status_loop.string()) == std::string::npos,
               "#4401: runtime config should not retain a status-error temp-directory path");
    }
#endif

    fs::remove_all(fixture_root, ignored);
}

void test_verified_startup_source_text_overrides_changed_disk_source() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_verified_source";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "verified.prg";
    const fs::path include_path = temp_root / "VERIFIED.H";
    write_text(
        main_path,
        "#INCLUDE 'verified.h'\n"
        "PUBLIC sourceValue\n"
        "sourceValue = SOURCE_VALUE\n");
    write_text(include_path, "#DEFINE SOURCE_VALUE 'disk'\n");

    auto options = make_runtime_session_options(main_path.string(), temp_root.string());
    options.startup_source_text =
        "#INCLUDE 'verified.h'\n"
        "PUBLIC sourceValue\n"
        "sourceValue = SOURCE_VALUE\n";
    options.source_text_overrides.emplace(
        include_path.lexically_normal().string(),
        "#DEFINE SOURCE_VALUE 'verified snapshot'\n");
    options.require_source_text_overrides = true;
    auto session = copperfin::runtime::PrgRuntimeSession::create(options);
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    const auto source_value = state.globals.find("sourcevalue");
    expect(source_value != state.globals.end() &&
               copperfin::runtime::format_value(source_value->second) == "verified snapshot",
           "runtime startup should case-insensitively parse verified source text without reopening disk source");

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

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
        "PUBLIC publicCount\n"
        "public_default = publicCount\n"
        "DO localproc\n"
        "RETURN\n"
        "PROCEDURE localproc\n"
        "LOCAL itemCount\n"
        "itemCount = 9\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    session.add_breakpoint({.file_path = main_path.string(), .line = 7});
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::breakpoint, "local-variable test should stop on the itemCount assignment");
    expect(state.statement_text == "itemCount = 9",
           "breakpoint pause state should preserve the current statement text");
    expect(!state.call_stack.empty(), "local-variable test should include a stack frame");
    if (!state.call_stack.empty()) {
        const auto local = state.call_stack.front().locals.find("itemcount");
        expect(local != state.call_stack.front().locals.end(), "stack frame should expose declared LOCAL variables");
        if (local != state.call_stack.front().locals.end()) {
            expect(copperfin::runtime::format_value(local->second) == "false",
                   "LOCAL variables should initialize to logical false before assignment");
        }
    }
    const auto public_value = state.globals.find("publiccount");
    const auto public_default = state.globals.find("public_default");
    expect(public_value != state.globals.end(), "PUBLIC variable should be captured before assignment");
    expect(public_default != state.globals.end(), "PUBLIC default value should be captured");
    if (public_value != state.globals.end()) {
        expect(copperfin::runtime::format_value(public_value->second) == "false",
               "PUBLIC variables should initialize to logical false");
    }
    if (public_default != state.globals.end()) {
        expect(copperfin::runtime::format_value(public_default->second) == "false",
               "PUBLIC default reads should return logical false");
    }

    fs::remove_all(temp_root, ignored);
}

void test_breakpoint_on_first_executable_line_hits_after_entry_continue() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_first_line_breakpoint";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "first_line_breakpoint.prg";
    write_text(
        main_path,
        "x = 1\n"
        "x = x + 1\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), true));

    session.add_breakpoint({.file_path = main_path.string(), .line = 1});
    const auto entry_state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(entry_state.reason == copperfin::runtime::DebugPauseReason::entry,
           "first-line breakpoint test should stop on entry first");

    const auto breakpoint_state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(breakpoint_state.reason == copperfin::runtime::DebugPauseReason::breakpoint,
           "continuing from entry should hit a first-line breakpoint");
    expect(breakpoint_state.statement_text == "x = 1",
           "first-line breakpoint should preserve the current statement text");
    expect(!breakpoint_state.call_stack.empty(),
           "first-line breakpoint should expose a stack frame");
    if (!breakpoint_state.call_stack.empty()) {
        expect(breakpoint_state.call_stack.front().line == 1U,
               "first-line breakpoint should report line 1");
    }

    fs::remove_all(temp_root, ignored);
}

void test_step_pause_state_preserves_statement_text_across_step_modes() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_step_statement_text";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "step_statement_text.prg";
    write_text(
        main_path,
        "DO worker\n"
        "x = 2\n"
        "RETURN\n"
        "PROCEDURE worker\n"
        "y = 1\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), true));

    const auto entry_state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(entry_state.reason == copperfin::runtime::DebugPauseReason::entry,
           "step-state test should stop on entry first");

    const auto step_into_state = session.run(copperfin::runtime::DebugResumeAction::step_into);
    expect(step_into_state.reason == copperfin::runtime::DebugPauseReason::step,
           "step-into should pause with step reason");
    expect(step_into_state.statement_text == "y = 1",
           "step-into over DO worker should pause on the first worker statement");
    expect(!step_into_state.call_stack.empty(),
           "step-into state should expose a stack frame");
    if (!step_into_state.call_stack.empty()) {
        expect(step_into_state.call_stack.front().routine_name == "worker",
               "step-into should move execution into the worker routine");
    }

    copperfin::runtime::PrgRuntimeSession step_over_session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), true));
    const auto step_over_entry = step_over_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(step_over_entry.reason == copperfin::runtime::DebugPauseReason::entry,
           "step-over test should stop on entry first");

    const auto step_over_state = step_over_session.run(copperfin::runtime::DebugResumeAction::step_over);
    expect(step_over_state.reason == copperfin::runtime::DebugPauseReason::step,
           "step-over should pause with step reason");
    expect(step_over_state.statement_text == "x = 2",
           "step-over on DO worker should resume at the caller's next statement");
    expect(!step_over_state.call_stack.empty(),
           "step-over state should expose a stack frame");
    if (!step_over_state.call_stack.empty()) {
        expect(step_over_state.call_stack.front().routine_name == "main",
               "step-over should return to the caller frame");
    }

    copperfin::runtime::PrgRuntimeSession step_out_session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), true));
    const auto step_out_entry = step_out_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(step_out_entry.reason == copperfin::runtime::DebugPauseReason::entry,
           "step-out test should stop on entry first");

    const auto step_out_inner = step_out_session.run(copperfin::runtime::DebugResumeAction::step_into);
    expect(step_out_inner.reason == copperfin::runtime::DebugPauseReason::step,
           "step-out setup should first step into the worker routine");

    const auto step_out_state = step_out_session.run(copperfin::runtime::DebugResumeAction::step_out);
    expect(step_out_state.reason == copperfin::runtime::DebugPauseReason::step,
           "step-out should pause with step reason");
    expect(step_out_state.statement_text == "x = 2",
           "step-out from worker should resume at the caller's next statement");
    expect(!step_out_state.call_stack.empty(),
           "step-out state should expose a stack frame");
    if (!step_out_state.call_stack.empty()) {
        expect(step_out_state.call_stack.front().routine_name == "main",
               "step-out should unwind back to the caller frame");
    }

    fs::remove_all(temp_root, ignored);
}

void test_breakpoint_pause_preserves_selected_cursor_inspection_state() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_breakpoint_cursor_state";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}});

    const fs::path main_path = temp_root / "breakpoint_cursor_state.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "GO 2\n"
        "x = AGE\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    session.add_breakpoint({.file_path = main_path.string(), .line = 3});
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::breakpoint,
           "cursor-state test should stop on the third-line breakpoint");
    expect(state.statement_text == "x = AGE",
           "cursor-state breakpoint should preserve the current statement text");
    expect(state.work_area.selected == 1,
           "breakpoint pause should preserve the selected work area");
    expect(state.work_area.aliases.size() == 1U,
           "breakpoint pause should preserve the open alias map");
    if (state.work_area.aliases.size() == 1U) {
        expect(state.work_area.aliases.begin()->second == "People",
               "breakpoint pause should preserve the selected alias name");
    }

    const auto runtime_cursor = std::find_if(state.cursors.begin(), state.cursors.end(), [](const auto& cursor) {
        return cursor.alias == "People";
    });
    expect(runtime_cursor != state.cursors.end(),
           "breakpoint pause should preserve the open cursor in runtime inspection state");
    if (runtime_cursor != state.cursors.end()) {
        expect(runtime_cursor->work_area == 1,
               "breakpoint pause should preserve the cursor work area");
        expect(runtime_cursor->recno == 2U,
               "breakpoint pause should preserve the current record position");
        expect(runtime_cursor->record_count == 2U,
               "breakpoint pause should preserve the open cursor record count");
        expect(!runtime_cursor->source_path.empty(),
               "breakpoint pause should preserve the cursor source path");
    }

    fs::remove_all(temp_root, ignored);
}

void test_step_pause_preserves_selected_cursor_inspection_state() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_step_cursor_state";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}});

    const fs::path main_path = temp_root / "step_cursor_state.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "GO 2\n"
        "DO worker\n"
        "x = AGE\n"
        "RETURN\n"
        "PROCEDURE worker\n"
        "y = 1\n"
        "RETURN\n");

    const auto expect_cursor_state = [](const copperfin::runtime::RuntimePauseState& state,
                                        const std::string& expected_statement,
                                        const std::string& expected_frame) {
        expect(state.reason == copperfin::runtime::DebugPauseReason::step,
               "step cursor-state test should pause with step reason");
        expect(state.statement_text == expected_statement,
               "step cursor-state test should preserve the expected paused statement text");
        expect(state.work_area.selected == 1,
               "step cursor-state test should preserve the selected work area");
        expect(state.work_area.aliases.size() == 1U,
               "step cursor-state test should preserve the alias map");
        if (state.work_area.aliases.size() == 1U) {
            expect(state.work_area.aliases.begin()->second == "People",
                   "step cursor-state test should preserve the selected alias name");
        }
        expect(!state.call_stack.empty(),
               "step cursor-state test should expose a stack frame");
        if (!state.call_stack.empty()) {
            expect(state.call_stack.front().routine_name == expected_frame,
                   "step cursor-state test should preserve the expected top frame");
        }

        const auto runtime_cursor = std::find_if(state.cursors.begin(), state.cursors.end(), [](const auto& cursor) {
            return cursor.alias == "People";
        });
        expect(runtime_cursor != state.cursors.end(),
               "step cursor-state test should preserve the open cursor in runtime inspection state");
        if (runtime_cursor != state.cursors.end()) {
            expect(runtime_cursor->work_area == 1,
                   "step cursor-state test should preserve cursor work area");
            expect(runtime_cursor->recno == 2U,
                   "step cursor-state test should preserve current record position");
            expect(runtime_cursor->record_count == 2U,
                   "step cursor-state test should preserve current record count");
            expect(!runtime_cursor->source_path.empty(),
                   "step cursor-state test should preserve cursor source path");
        }
    };

    copperfin::runtime::PrgRuntimeSession step_into_session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), true));
    const auto step_into_entry = step_into_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(step_into_entry.reason == copperfin::runtime::DebugPauseReason::entry,
           "step-into cursor-state test should stop on entry first");
    const auto step_into_use = step_into_session.run(copperfin::runtime::DebugResumeAction::step_into);
    expect(step_into_use.reason == copperfin::runtime::DebugPauseReason::step,
           "step-into cursor-state setup should pause after USE");
    const auto step_into_go = step_into_session.run(copperfin::runtime::DebugResumeAction::step_into);
    expect(step_into_go.reason == copperfin::runtime::DebugPauseReason::step,
           "step-into cursor-state setup should pause after GO");
    const auto step_into_state = step_into_session.run(copperfin::runtime::DebugResumeAction::step_into); // into worker
    expect_cursor_state(step_into_state, "y = 1", "worker");

    copperfin::runtime::PrgRuntimeSession step_over_session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), true));
    const auto step_over_entry = step_over_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(step_over_entry.reason == copperfin::runtime::DebugPauseReason::entry,
           "step-over cursor-state test should stop on entry first");
    const auto step_over_use = step_over_session.run(copperfin::runtime::DebugResumeAction::step_into);
    expect(step_over_use.reason == copperfin::runtime::DebugPauseReason::step,
           "step-over cursor-state setup should pause after USE");
    const auto step_over_go = step_over_session.run(copperfin::runtime::DebugResumeAction::step_into);
    expect(step_over_go.reason == copperfin::runtime::DebugPauseReason::step,
           "step-over cursor-state setup should pause after GO");
    const auto step_over_state = step_over_session.run(copperfin::runtime::DebugResumeAction::step_over); // over DO worker
    expect_cursor_state(step_over_state, "x = AGE", "main");

    copperfin::runtime::PrgRuntimeSession step_out_session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), true));
    const auto step_out_entry = step_out_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(step_out_entry.reason == copperfin::runtime::DebugPauseReason::entry,
           "step-out cursor-state test should stop on entry first");
    const auto step_out_use = step_out_session.run(copperfin::runtime::DebugResumeAction::step_into);
    expect(step_out_use.reason == copperfin::runtime::DebugPauseReason::step,
           "step-out cursor-state setup should pause after USE");
    const auto step_out_go = step_out_session.run(copperfin::runtime::DebugResumeAction::step_into);
    expect(step_out_go.reason == copperfin::runtime::DebugPauseReason::step,
           "step-out cursor-state setup should pause after GO");
    const auto step_out_inner = step_out_session.run(copperfin::runtime::DebugResumeAction::step_into);
    expect(step_out_inner.reason == copperfin::runtime::DebugPauseReason::step,
           "step-out cursor-state setup should step into worker before stepping out");
    const auto step_out_state = step_out_session.run(copperfin::runtime::DebugResumeAction::step_out);
    expect_cursor_state(step_out_state, "x = AGE", "main");

    fs::remove_all(temp_root, ignored);
}

void test_watch_expression_evaluates_locals_globals_and_cursor_fields() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_watch_eval";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}});

    const fs::path main_path = temp_root / "watch_eval.prg";
    write_text(
        main_path,
        "PUBLIC gValue\n"
        "gValue = 5\n"
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "GO 2\n"
        "DO worker\n"
        "RETURN\n"
        "PROCEDURE worker\n"
        "LOCAL nLocal\n"
        "nLocal = AGE + gValue\n"
        "x = nLocal\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));
    session.add_breakpoint({.file_path = main_path.string(), .line = 10});

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::breakpoint,
           "watch-eval test should stop on the worker assignment breakpoint");
    expect(state.statement_text == "x = nLocal",
           "watch-eval test should pause on the expected worker assignment");

    const auto local_watch = session.evaluate_watch_expression("nLocal");
    expect(local_watch.ok, "watch-eval test should resolve LOCAL variables");
    if (local_watch.ok) {
        expect(copperfin::runtime::format_value(local_watch.value) == "25",
               "watch-eval test should report the computed LOCAL value");
    }

    const auto global_watch = session.evaluate_watch_expression("gValue");
    expect(global_watch.ok, "watch-eval test should resolve PUBLIC/ global variables");
    if (global_watch.ok) {
        expect(copperfin::runtime::format_value(global_watch.value) == "5",
               "watch-eval test should report the global value");
    }

    const auto field_watch = session.evaluate_watch_expression("AGE");
    expect(field_watch.ok, "watch-eval test should resolve selected cursor fields");
    if (field_watch.ok) {
        expect(copperfin::runtime::format_value(field_watch.value) == "20",
               "watch-eval test should report the current record field value");
    }

    const auto alias_watch = session.evaluate_watch_expression("ALIAS()");
    expect(alias_watch.ok, "watch-eval test should evaluate runtime-surface functions");
    if (alias_watch.ok) {
        expect(copperfin::runtime::format_value(alias_watch.value) == "People",
               "watch-eval test should preserve selected work-area context during watch evaluation");
    }

    const auto malformed_watch = session.evaluate_watch_expression("   ");
    expect(!malformed_watch.ok, "watch-eval test should reject empty watch expressions");
    expect(malformed_watch.message == "Watch expression is empty.",
           "watch-eval empty-expression message should route through the default locale catalog");

    const auto preserved_state = session.state();
    expect(preserved_state.reason == copperfin::runtime::DebugPauseReason::breakpoint,
           "watch-eval test should not resume execution while evaluating watches");
    expect(preserved_state.statement_text == "x = nLocal",
           "watch-eval test should preserve the paused statement after watch evaluation");

    const auto completed_state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(completed_state.completed, "watch-eval test should be able to resume to completion");
    const auto unpaused_watch = session.evaluate_watch_expression("gValue");
    expect(!unpaused_watch.ok, "watch-eval test should reject evaluation without a paused frame");
    expect(unpaused_watch.message == "Watch evaluation requires a paused runtime frame.",
           "watch-eval paused-frame message should route through the default locale catalog");

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop, "DO FORM should now enter the event loop for runnable forms");
    expect(state.waiting_for_events, "form launch should report waiting_for_events");

    fs::remove_all(temp_root, ignored);
}

void test_prg_filesystem_status_errors_become_runtime_results() {
#if !defined(_WIN32)
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_filesystem_status";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    const fs::path loop_root = temp_root / "status_loop";
    fs::create_symlink(loop_root, loop_root, ignored);
    if (ignored) {
        fs::remove_all(temp_root, ignored);
        return;
    }

    const auto run_script = [&](const std::string &name, const std::string &source) {
        const fs::path main_path = temp_root / (name + ".prg");
        write_text(main_path, source);
        auto session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string(), false));
        return session.run(copperfin::runtime::DebugResumeAction::continue_run);
    };

    const fs::path table_path = loop_root / "people.dbf";
    const auto use_state = run_script("use_status_error", "USE '" + table_path.string() + "' ALIAS People\nRETURN\n");
    expect(use_state.reason == copperfin::runtime::DebugPauseReason::error,
           "#4399: USE filesystem status errors should become runtime faults");
    expect(use_state.message == "Unable to resolve USE target: " + table_path.lexically_normal().string(),
           "#4399: USE filesystem status errors should preserve the localized target diagnostic");

    const fs::path program_path = loop_root / "worker.prg";
    const auto do_state = run_script("do_status_error", "DO " + program_path.string() + "\nRETURN\n");
    expect(do_state.reason == copperfin::runtime::DebugPauseReason::error,
           "#4399: DO filesystem status errors should become runtime faults");
    expect(do_state.message == "Unable to resolve DO target: " + program_path.string(),
           "#4399: DO filesystem status errors should preserve the localized target diagnostic");

    const auto call_state = run_script("call_status_error", "CALL " + program_path.string() + "\nRETURN\n");
    expect(call_state.reason == copperfin::runtime::DebugPauseReason::error,
           "#4399: CALL filesystem status errors should become runtime faults");
    expect(call_state.message == "Unable to resolve CALL target: " + program_path.string(),
           "#4399: CALL filesystem status errors should preserve the localized target diagnostic");

    const auto spawn_state = run_script(
        "spawn_status_error",
        "SPAWN " + program_path.string() + " TO nTask\nRETURN\n");
    expect(spawn_state.reason == copperfin::runtime::DebugPauseReason::error,
           "#4399: SPAWN filesystem status errors should become runtime faults");
    expect(spawn_state.message == "Unable to resolve SPAWN target: " + program_path.string(),
           "#4399: SPAWN filesystem status errors should preserve the localized target diagnostic");

    const fs::path form_path = loop_root / "form.scx";
    const auto form_state = run_script("form_status_error", "DO FORM '" + form_path.string() + "'\nRETURN\n");
    expect(form_state.completed,
           "#4399: DO FORM filesystem status errors should preserve the existing missing-form completion behavior");

    fs::remove_all(temp_root, ignored);
#endif
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
        "PUBLIC nWorkerRan\n"
        "cProc = 'worker'\n"
        "cProcHolder = 'cProc'\n"
        "cProcDeepHolder = 'cProcHolder'\n"
        "DO &cProcDeepHolder\n"
        "RETURN\n"
        "PROCEDURE worker\n"
        "nWorkerRan = 1\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

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
        "PUBLIC cParamResult\n"
        "cFieldExpr = 'cValue'\n"
        "cRowExpr = '1'\n"
        "cFieldExprHolder = 'cFieldExpr'\n"
        "cFieldExprDeepHolder = 'cFieldExprHolder'\n"
        "cRowExprHolder = 'cRowExpr'\n"
        "cRowExprDeepHolder = 'cRowExprHolder'\n"
        "cValue = 'HELLO'\n"
        "DIMENSION aData[2]\n"
        "aData[1] = 'ALPHA'\n"
        "aData[2] = 'BRAVO'\n"
        "cResult1 = aData[&cRowExprDeepHolder]\n"
        "cResult2 = LEN(&cFieldExprDeepHolder)\n"
        "cTarget = 'cResult3'\n"
        "cTargetHolder = 'cTarget'\n"
        "cTargetDeepHolder = 'cTargetHolder'\n"
        "&cTargetDeepHolder = 'MACROASSIGN'\n"
        "cParamExpr = 'cParamValue'\n"
        "cParamExprHolder = 'cParamExpr'\n"
        "cParamExprDeepHolder = 'cParamExprHolder'\n"
        "cParamValue = 'PARAMVAL'\n"
        "DO testparam WITH &cParamExprDeepHolder\n"
        "RETURN\n"
        "PROCEDURE testparam\n"
        "LPARAMETERS p1\n"
        "cParamResult = p1\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession macro_session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options((temp_root / "macro_deep.prg").string(), temp_root.string()));
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
        "cAliasExprHolder = 'cAliasExpr'\n"
        "cAliasExprDeepHolder = 'cAliasExprHolder'\n"
        "cFieldExpr = 'NAME'\n"
        "cFieldExprHolder = 'cFieldExpr'\n"
        "cFieldExprDeepHolder = 'cFieldExprHolder'\n"
        "cEvalExpr = 'AGE + 5'\n"
        "cEvalAliasExpr = 'ALIAS()'\n"
        "cEvalAliasExprHolder = 'cEvalAliasExpr'\n"
        "cEvalAliasExprDeepHolder = 'cEvalAliasExprHolder'\n"
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
        "nSetDefaultPathCalls = 0\n"
        "SET DEFAULT TO set_default_path('" + new_default.string() + "')\n"
        "cDefaultAfter = SET('DEFAULT')\n"
        "cAliasFromEval = EVAL('ALIAS()')\n"
        "cAliasFromEvalMacro = EVAL(&cEvalAliasExpr)\n"
        "cAliasFromEvalNested = EVAL(&cEvalAliasExprHolder)\n"
        "cAliasFromEvalSecondHop = EVAL(&cEvalAliasExprDeepHolder)\n"
        "cNameFromMacro = &cFieldExpr\n"
        "cNameFromMacroSecondHop = &cFieldExprDeepHolder\n"
        "nEvalAge = EVAL(cEvalExpr)\n"
        "USE IN &cAliasExprDeepHolder\n"
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
        "RETURN\n"
        "FUNCTION set_default_path\n"
        "LPARAMETERS value\n"
        "nSetDefaultPathCalls = nSetDefaultPathCalls + 1\n"
        "RETURN value\n"
        "ENDFUNC\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

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
    const auto set_default_path_calls = state.globals.find("nsetdefaultpathcalls");
    const auto alias_from_eval = state.globals.find("caliasfromeval");
    const auto alias_from_eval_macro = state.globals.find("caliasfromevalmacro");
    const auto alias_from_eval_nested = state.globals.find("caliasfromevalnested");
    const auto alias_from_eval_second_hop = state.globals.find("caliasfromevalsecondhop");
    const auto name_from_macro = state.globals.find("cnamefrommacro");
    const auto name_from_macro_second_hop = state.globals.find("cnamefrommacrosecondhop");
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
    expect(set_default_path_calls != state.globals.end(), "SET DEFAULT should preserve the path resolver call counter");
    if (set_default_path_calls != state.globals.end()) {
        expect(copperfin::runtime::format_value(set_default_path_calls->second) == "1",
               "SET DEFAULT should evaluate the path UDF exactly once");
    }
    expect(alias_from_eval != state.globals.end(), "EVAL() should be able to evaluate runtime-state expressions");
    expect(alias_from_eval_macro != state.globals.end(), "EVAL(&macro) should preserve expression text for runtime-state evaluation");
    expect(alias_from_eval_nested != state.globals.end(), "EVAL(&holder) should preserve nested macro-indirection for runtime-state evaluation");
    expect(alias_from_eval_second_hop != state.globals.end(), "EVAL(&deep-holder) should preserve second-hop nested macro-indirection for runtime-state evaluation");
    expect(name_from_macro != state.globals.end(), "&macro field resolution should be captured");
    expect(name_from_macro_second_hop != state.globals.end(), "&deep-holder field resolution should be captured");
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
    if (alias_from_eval_second_hop != state.globals.end()) {
        expect(copperfin::runtime::format_value(alias_from_eval_second_hop->second) == "People",
               "EVAL(&deep-holder) should preserve runtime-state expression text through a second holder hop");
    }
    if (deleted_after_to_zero != state.globals.end()) {
        expect(copperfin::runtime::format_value(deleted_after_to_zero->second) == "OFF", "SET('DELETED') should report OFF after SET DELETED TO 0");
    }
    if (path_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(path_before->second).empty(), "SET('PATH') should report empty string before it is configured");
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
    if (alias_from_eval_macro != state.globals.end()) {
        expect(copperfin::runtime::format_value(alias_from_eval_macro->second) == "People", "EVAL(&cExpr) should evaluate the macro-expanded expression text in the current runtime context");
    }
    if (alias_from_eval_nested != state.globals.end()) {
        expect(copperfin::runtime::format_value(alias_from_eval_nested->second) == "People", "EVAL(&cHolder) should chase nested macro indirection before evaluating runtime-state expression text");
    }
    if (name_from_macro != state.globals.end()) {
        expect(copperfin::runtime::format_value(name_from_macro->second) == "ALPHA", "&macro should substitute a stored field name inside expressions");
    }
    if (name_from_macro_second_hop != state.globals.end()) {
        expect(copperfin::runtime::format_value(name_from_macro_second_hop->second) == "ALPHA",
               "&deep-holder should substitute a second-hop stored field name inside expressions");
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
         expect(copperfin::runtime::format_value(path_session2->second).empty(),
             "SET('PATH') should be empty in a fresh data session (no path inheritance)");
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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

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

void test_set_state_variables_strictdate_enginebehavior_optimize() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "test_set_vars";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path prg_path = temp_root / "set_vars_test.prg";
    write_text(
        prg_path,
        "* Test SET STRICTDATE, ENGINEBEHAVIOR, OPTIMIZE\n"
        "nTest = 1\n"
        "cStrictDateBefore = SET('STRICTDATE')\n"
        "cEngineBehaviorBefore = SET('ENGINEBEHAVIOR')\n"
        "cOptimizeBefore = SET('OPTIMIZE')\n"
        "\n"
        "SET STRICTDATE ON\n"
        "cStrictDateAfter = SET('STRICTDATE')\n"
        "\n"
        "SET ENGINEBEHAVIOR ON\n"
        "cEngineBehaviorAfter = SET('ENGINEBEHAVIOR')\n"
        "\n"
        "SET OPTIMIZE ON\n"
        "cOptimizeAfter = SET('OPTIMIZE')\n"
        "\n"
        "* Test boolean variants\n"
        "SET STRICTDATE TO .F.\n"
        "cStrictDateAfterOff = SET('STRICTDATE')\n"
        "\n"
        "SET OPTIMIZE TO 'false'\n"
        "cOptimizeAfterFalse = SET('OPTIMIZE')\n"
        "\n"
        "* Test data session isolation\n"
        "SET DATASESSION TO 2\n"
        "cStrictDateSession2 = SET('STRICTDATE')\n"
        "cEngineBehaviorSession2 = SET('ENGINEBEHAVIOR')\n"
        "cOptimizeSession2 = SET('OPTIMIZE')\n"
        "\n"
        "SET DATASESSION TO 1\n"
        "cStrictDateRestored = SET('STRICTDATE')\n"
        "cEngineBehaviorRestored = SET('ENGINEBEHAVIOR')\n"
        "cOptimizeRestored = SET('OPTIMIZE')\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(prg_path, temp_root));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "set_vars_test script should complete");

    const auto strictdate_before = state.globals.find("cstrictdatebefore");
    const auto strictdate_after = state.globals.find("cstrictdateafter");
    const auto strictdate_after_off = state.globals.find("cstrictdateafteroff");
    const auto enginebehavior_before = state.globals.find("cenginebehaviorbefore");
    const auto enginebehavior_after = state.globals.find("cenginebehaviorafter");
    const auto optimize_before = state.globals.find("coptimizebefore");
    const auto optimize_after = state.globals.find("coptimizeafter");
    const auto optimize_after_false = state.globals.find("coptimizeafterfalse");
    const auto strictdate_session2 = state.globals.find("cstrictdatesession2");
    const auto enginebehavior_session2 = state.globals.find("cenginebehaviorsession2");
    const auto optimize_session2 = state.globals.find("coptimizesession2");
    const auto strictdate_restored = state.globals.find("cstrictdaterestored");
    const auto enginebehavior_restored = state.globals.find("cenginebehaviorrestored");
    const auto optimize_restored = state.globals.find("coptimizerestored");

    expect(strictdate_before != state.globals.end(), "SET('STRICTDATE') before change should be captured");
    expect(strictdate_after != state.globals.end(), "SET('STRICTDATE') after SET STRICTDATE ON should be captured");
    expect(strictdate_after_off != state.globals.end(), "SET('STRICTDATE') after SET STRICTDATE TO .F. should be captured");
    expect(enginebehavior_before != state.globals.end(), "SET('ENGINEBEHAVIOR') before change should be captured");
    expect(enginebehavior_after != state.globals.end(), "SET('ENGINEBEHAVIOR') after SET ENGINEBEHAVIOR TO 'VFP' should be captured");
    expect(optimize_before != state.globals.end(), "SET('OPTIMIZE') before change should be captured");
    expect(optimize_after != state.globals.end(), "SET('OPTIMIZE') after SET OPTIMIZE ON should be captured");
    expect(optimize_after_false != state.globals.end(), "SET('OPTIMIZE') after SET OPTIMIZE TO 'false' should be captured");
    expect(strictdate_session2 != state.globals.end(), "SET('STRICTDATE') in session 2 should be captured");
    expect(enginebehavior_session2 != state.globals.end(), "SET('ENGINEBEHAVIOR') in session 2 should be captured");
    expect(optimize_session2 != state.globals.end(), "SET('OPTIMIZE') in session 2 should be captured");
    expect(strictdate_restored != state.globals.end(), "SET('STRICTDATE') after restoring session 1 should be captured");
    expect(enginebehavior_restored != state.globals.end(), "SET('ENGINEBEHAVIOR') after restoring session 1 should be captured");
    expect(optimize_restored != state.globals.end(), "SET('OPTIMIZE') after restoring session 1 should be captured");

    if (strictdate_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(strictdate_before->second) == "OFF", "SET('STRICTDATE') should default to OFF");
    }
    if (strictdate_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(strictdate_after->second) == "ON", "SET('STRICTDATE') should report ON after SET STRICTDATE ON");
    }
    if (strictdate_after_off != state.globals.end()) {
        expect(copperfin::runtime::format_value(strictdate_after_off->second) == "OFF", "SET('STRICTDATE') should report OFF after SET STRICTDATE TO .F.");
    }
    if (enginebehavior_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(enginebehavior_before->second) == "OFF", "SET('ENGINEBEHAVIOR') should default to OFF");
    }
    if (enginebehavior_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(enginebehavior_after->second) == "ON", "SET('ENGINEBEHAVIOR') should report ON after SET ENGINEBEHAVIOR ON");
    }
    if (optimize_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(optimize_before->second) == "OFF", "SET('OPTIMIZE') should default to OFF");
    }
    if (optimize_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(optimize_after->second) == "ON", "SET('OPTIMIZE') should report ON after SET OPTIMIZE ON");
    }
    if (optimize_after_false != state.globals.end()) {
        expect(copperfin::runtime::format_value(optimize_after_false->second) == "OFF", "SET('OPTIMIZE') should report OFF after SET OPTIMIZE TO 'false'");
    }
    if (strictdate_session2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(strictdate_session2->second) == "OFF", "SET('STRICTDATE') in a fresh session should be OFF");
    }
    if (enginebehavior_session2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(enginebehavior_session2->second) == "OFF", "SET('ENGINEBEHAVIOR') in a fresh session should be OFF");
    }
    if (optimize_session2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(optimize_session2->second) == "OFF", "SET('OPTIMIZE') in a fresh session should be OFF");
    }
    if (strictdate_restored != state.globals.end()) {
        expect(copperfin::runtime::format_value(strictdate_restored->second) == "OFF", "restoring session 1 should restore SET('STRICTDATE') to its state after SET STRICTDATE TO .F.");
    }
    if (enginebehavior_restored != state.globals.end()) {
        expect(copperfin::runtime::format_value(enginebehavior_restored->second) == "ON", "restoring session 1 should restore SET('ENGINEBEHAVIOR') to its state after SET ENGINEBEHAVIOR ON");
    }
    if (optimize_restored != state.globals.end()) {
        expect(copperfin::runtime::format_value(optimize_restored->second) == "OFF", "restoring session 1 should restore SET('OPTIMIZE') to its state after SET OPTIMIZE TO 'false'");
    }

    fs::remove_all(temp_root, ignored);
}

void test_set_state_variables_talk_safety_escape() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "test_set_flags";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path prg_path = temp_root / "set_flags_test.prg";
    write_text(
        prg_path,
        "* Test SET TALK, SAFETY, ESCAPE\n"
        "cTalkBefore = SET('TALK')\n"
        "cSafetyBefore = SET('SAFETY')\n"
        "cEscapeBefore = SET('ESCAPE')\n"
        "\n"
        "SET TALK TO 1\n"
        "cTalkAfterOne = SET('TALK')\n"
        "SET TALK TO 'false'\n"
        "cTalkAfterFalse = SET('TALK')\n"
        "\n"
        "SET SAFETY TO .T.\n"
        "cSafetyAfterTrue = SET('SAFETY')\n"
        "SET SAFETY TO 0\n"
        "cSafetyAfterZero = SET('SAFETY')\n"
        "\n"
        "SET ESCAPE ON\n"
        "cEscapeAfterOn = SET('ESCAPE')\n"
        "SET ESCAPE TO 'no'\n"
        "cEscapeAfterNo = SET('ESCAPE')\n"
        "\n"
        "SET DATASESSION TO 2\n"
        "cTalkSession2 = SET('TALK')\n"
        "cSafetySession2 = SET('SAFETY')\n"
        "cEscapeSession2 = SET('ESCAPE')\n"
        "\n"
        "SET DATASESSION TO 1\n"
        "cTalkRestored = SET('TALK')\n"
        "cSafetyRestored = SET('SAFETY')\n"
        "cEscapeRestored = SET('ESCAPE')\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(prg_path, temp_root));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "set_flags_test script should complete");

    const auto talk_before = state.globals.find("ctalkbefore");
    const auto safety_before = state.globals.find("csafetybefore");
    const auto escape_before = state.globals.find("cescapebefore");
    const auto talk_after_one = state.globals.find("ctalkafterone");
    const auto talk_after_false = state.globals.find("ctalkafterfalse");
    const auto safety_after_true = state.globals.find("csafetyaftertrue");
    const auto safety_after_zero = state.globals.find("csafetyafterzero");
    const auto escape_after_on = state.globals.find("cescapeafteron");
    const auto escape_after_no = state.globals.find("cescapeafterno");
    const auto talk_session2 = state.globals.find("ctalksession2");
    const auto safety_session2 = state.globals.find("csafetysession2");
    const auto escape_session2 = state.globals.find("cescapesession2");
    const auto talk_restored = state.globals.find("ctalkrestored");
    const auto safety_restored = state.globals.find("csafetyrestored");
    const auto escape_restored = state.globals.find("cescaperestored");

    expect(talk_before != state.globals.end(), "SET('TALK') before change should be captured");
    expect(safety_before != state.globals.end(), "SET('SAFETY') before change should be captured");
    expect(escape_before != state.globals.end(), "SET('ESCAPE') before change should be captured");
    expect(talk_after_one != state.globals.end(), "SET('TALK') after SET TALK TO 1 should be captured");
    expect(talk_after_false != state.globals.end(), "SET('TALK') after SET TALK TO 'false' should be captured");
    expect(safety_after_true != state.globals.end(), "SET('SAFETY') after SET SAFETY TO .T. should be captured");
    expect(safety_after_zero != state.globals.end(), "SET('SAFETY') after SET SAFETY TO 0 should be captured");
    expect(escape_after_on != state.globals.end(), "SET('ESCAPE') after SET ESCAPE ON should be captured");
    expect(escape_after_no != state.globals.end(), "SET('ESCAPE') after SET ESCAPE TO 'no' should be captured");
    expect(talk_session2 != state.globals.end(), "SET('TALK') in session 2 should be captured");
    expect(safety_session2 != state.globals.end(), "SET('SAFETY') in session 2 should be captured");
    expect(escape_session2 != state.globals.end(), "SET('ESCAPE') in session 2 should be captured");
    expect(talk_restored != state.globals.end(), "SET('TALK') after restoring session 1 should be captured");
    expect(safety_restored != state.globals.end(), "SET('SAFETY') after restoring session 1 should be captured");
    expect(escape_restored != state.globals.end(), "SET('ESCAPE') after restoring session 1 should be captured");

    if (talk_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(talk_before->second) == "OFF", "SET('TALK') should default to OFF");
    }
    if (safety_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(safety_before->second) == "OFF", "SET('SAFETY') should default to OFF");
    }
    if (escape_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(escape_before->second) == "OFF", "SET('ESCAPE') should default to OFF");
    }
    if (talk_after_one != state.globals.end()) {
        expect(copperfin::runtime::format_value(talk_after_one->second) == "ON", "SET('TALK') should report ON after SET TALK TO 1");
    }
    if (talk_after_false != state.globals.end()) {
        expect(copperfin::runtime::format_value(talk_after_false->second) == "OFF", "SET('TALK') should report OFF after SET TALK TO 'false'");
    }
    if (safety_after_true != state.globals.end()) {
        expect(copperfin::runtime::format_value(safety_after_true->second) == "ON", "SET('SAFETY') should report ON after SET SAFETY TO .T.");
    }
    if (safety_after_zero != state.globals.end()) {
        expect(copperfin::runtime::format_value(safety_after_zero->second) == "OFF", "SET('SAFETY') should report OFF after SET SAFETY TO 0");
    }
    if (escape_after_on != state.globals.end()) {
        expect(copperfin::runtime::format_value(escape_after_on->second) == "ON", "SET('ESCAPE') should report ON after SET ESCAPE ON");
    }
    if (escape_after_no != state.globals.end()) {
        expect(copperfin::runtime::format_value(escape_after_no->second) == "OFF", "SET('ESCAPE') should report OFF after SET ESCAPE TO 'no'");
    }
    if (talk_session2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(talk_session2->second) == "OFF", "SET('TALK') in a fresh session should be OFF");
    }
    if (safety_session2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(safety_session2->second) == "OFF", "SET('SAFETY') in a fresh session should be OFF");
    }
    if (escape_session2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(escape_session2->second) == "OFF", "SET('ESCAPE') in a fresh session should be OFF");
    }
    if (talk_restored != state.globals.end()) {
        expect(copperfin::runtime::format_value(talk_restored->second) == "OFF", "restoring session 1 should restore SET('TALK') to its prior OFF state");
    }
    if (safety_restored != state.globals.end()) {
        expect(copperfin::runtime::format_value(safety_restored->second) == "OFF", "restoring session 1 should restore SET('SAFETY') to its prior OFF state");
    }
    if (escape_restored != state.globals.end()) {
        expect(copperfin::runtime::format_value(escape_restored->second) == "OFF", "restoring session 1 should restore SET('ESCAPE') to its prior OFF state");
    }

    fs::remove_all(temp_root, ignored);
}

void test_transaction_processing_txnlevel_and_sessions() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "test_transactions";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path prg_path = temp_root / "transactions_test.prg";
    write_text(
        prg_path,
        "nTxnStart = TXNLEVEL()\n"
        "BEGIN TRANSACTION\n"
        "nTxnAfterBegin1 = TXNLEVEL()\n"
        "BEGIN TRANSACTION\n"
        "nTxnAfterBegin2 = TXNLEVEL()\n"
        "END TRANSACTION\n"
        "nTxnAfterEnd = TXNLEVEL()\n"
        "ROLLBACK\n"
        "nTxnAfterRollback = TXNLEVEL()\n"
        "SET DATASESSION TO 2\n"
        "nTxnSession2 = TXNLEVEL()\n"
        "BEGIN TRANSACTION\n"
        "nTxnSession2AfterBegin = TXNLEVEL()\n"
        "SET DATASESSION TO 1\n"
        "nTxnRestoredSession1 = TXNLEVEL()\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(prg_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "transactions_test script should complete");

    const auto txn_start = state.globals.find("ntxnstart");
    const auto txn_after_begin_1 = state.globals.find("ntxnafterbegin1");
    const auto txn_after_begin_2 = state.globals.find("ntxnafterbegin2");
    const auto txn_after_end = state.globals.find("ntxnafterend");
    const auto txn_after_rollback = state.globals.find("ntxnafterrollback");
    const auto txn_session2 = state.globals.find("ntxnsession2");
    const auto txn_session2_after_begin = state.globals.find("ntxnsession2afterbegin");
    const auto txn_restored_session1 = state.globals.find("ntxnrestoredsession1");

    expect(txn_start != state.globals.end(), "TXNLEVEL() at start should be captured");
    expect(txn_after_begin_1 != state.globals.end(), "TXNLEVEL() after first BEGIN TRANSACTION should be captured");
    expect(txn_after_begin_2 != state.globals.end(), "TXNLEVEL() after second BEGIN TRANSACTION should be captured");
    expect(txn_after_end != state.globals.end(), "TXNLEVEL() after END TRANSACTION should be captured");
    expect(txn_after_rollback != state.globals.end(), "TXNLEVEL() after ROLLBACK should be captured");
    expect(txn_session2 != state.globals.end(), "TXNLEVEL() in session 2 should be captured");
    expect(txn_session2_after_begin != state.globals.end(), "TXNLEVEL() after BEGIN TRANSACTION in session 2 should be captured");
    expect(txn_restored_session1 != state.globals.end(), "TXNLEVEL() after restoring session 1 should be captured");

    if (txn_start != state.globals.end()) {
        expect(copperfin::runtime::format_value(txn_start->second) == "0", "TXNLEVEL() should start at 0");
    }
    if (txn_after_begin_1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(txn_after_begin_1->second) == "1", "first BEGIN TRANSACTION should raise TXNLEVEL() to 1");
    }
    if (txn_after_begin_2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(txn_after_begin_2->second) == "2", "second BEGIN TRANSACTION should raise TXNLEVEL() to 2");
    }
    if (txn_after_end != state.globals.end()) {
        expect(copperfin::runtime::format_value(txn_after_end->second) == "1", "END TRANSACTION should decrement TXNLEVEL() by one");
    }
    if (txn_after_rollback != state.globals.end()) {
        expect(copperfin::runtime::format_value(txn_after_rollback->second) == "0", "ROLLBACK should reset TXNLEVEL() to 0");
    }
    if (txn_session2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(txn_session2->second) == "0", "a fresh data session should start with TXNLEVEL() == 0");
    }
    if (txn_session2_after_begin != state.globals.end()) {
        expect(copperfin::runtime::format_value(txn_session2_after_begin->second) == "1", "BEGIN TRANSACTION in session 2 should affect only session 2");
    }
    if (txn_restored_session1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(txn_restored_session1->second) == "0", "restoring session 1 should restore session 1 TXNLEVEL() state");
    }

    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.transaction.begin";
    }), "BEGIN TRANSACTION should emit runtime.transaction.begin events");

    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.transaction.end";
    }), "END TRANSACTION should emit runtime.transaction.end events");

    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.transaction.rollback";
    }), "ROLLBACK should emit runtime.transaction.rollback events");

    fs::remove_all(temp_root, ignored);
}

void test_transaction_commands_without_active_transaction_fault() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "test_transactions_without_active_transaction";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_fault_case = [&](const std::string &command, const std::string &label) {
        const fs::path prg_path = temp_root / (label + ".prg");
        write_text(
            prg_path,
            std::string("nBefore = TXNLEVEL()\n")
            + command + "\n"
            "nAfter = 1\n");
        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(prg_path.string(), temp_root.string()));
        auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.reason == copperfin::runtime::DebugPauseReason::error,
               label + " without an active transaction should pause with an error");
        expect(state.location.line == 2U,
               label + " without an active transaction should identify the command line");
        expect(state.message == "No active transaction.",
               label + " should use the localized no-active-transaction diagnostic");
        state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, label + " fault should be recoverable");
        expect(state.globals.contains("nafter"), label + " should continue after the trapped fault");
    };

    run_fault_case("ROLLBACK", "rollback_without_transaction");
    run_fault_case("END TRANSACTION", "end_without_transaction");

    fs::remove_all(temp_root, ignored);
}


}  // namespace

int main() {
    test_insert_select_numeric_serialization_ignores_global_locale();
    test_runtime_session_options_contain_temporary_files();
    test_verified_startup_source_text_overrides_changed_disk_source();
    test_verified_source_errors_are_localized();
    test_read_events_pause();
    test_activate_popup_pause();
    test_dispatch_event_handler();
    test_local_variables_in_stack_frame();
    test_breakpoint_on_first_executable_line_hits_after_entry_continue();
    test_step_pause_state_preserves_statement_text_across_step_modes();
    test_breakpoint_pause_preserves_selected_cursor_inspection_state();
    test_step_pause_preserves_selected_cursor_inspection_state();
    test_watch_expression_evaluates_locals_globals_and_cursor_fields();
    test_report_form_pause();
    test_label_form_pause();
    test_do_form_pause();
    test_prg_filesystem_status_errors_become_runtime_results();
    test_do_command_macro_target();
    test_export_vfp_compatibility_corpus_script();
    test_work_area_and_data_session_compatibility();
    test_eval_macro_and_runtime_state_semantics();
    test_sql_and_ole_compatibility_functions();
    test_sql_pass_through_rows_affected_and_provider_hint();
    test_sql_prepare_and_connection_properties();
    test_set_state_variables_strictdate_enginebehavior_optimize();
    test_set_state_variables_talk_safety_escape();
    test_transaction_processing_txnlevel_and_sessions();
    test_transaction_commands_without_active_transaction_fault();

    if (copperfin::test_support::test_failures() != 0) {
        std::cerr << copperfin::test_support::test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }
    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
