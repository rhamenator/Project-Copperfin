#include "copperfin/runtime/prg_engine.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

void write_text(const std::filesystem::path& path, const std::string& contents) {
    std::ofstream output(path, std::ios::binary);
    output << contents;
}

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

}  // namespace

int main() {
    test_breakpoints_and_stepping();
    test_read_events_pause();
    test_activate_popup_pause();
    test_dispatch_event_handler();
    test_local_variables_in_stack_frame();
    test_report_form_pause();
    test_do_form_pause();
    test_work_area_and_data_session_compatibility();
    test_sql_and_ole_compatibility_functions();

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
