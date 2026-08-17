// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "prg_engine_test_support.h"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <system_error>

namespace {

void test_on_escape_assignment_priority_and_set_escape_gate()
{
    namespace fs = std::filesystem;
    using copperfin::runtime::DebugPauseReason;
    using copperfin::runtime::DebugResumeAction;
    using copperfin::runtime::PrgRuntimeSession;
    using copperfin::runtime::format_value;
    using copperfin::test_support::expect;
    using copperfin::test_support::make_runtime_session_options;
    using copperfin::test_support::read_text;
    using copperfin::test_support::write_text;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_on_escape";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path enabled_path = temp_root / "on_escape_enabled.prg";
    write_text(enabled_path,
        "PUBLIC cTrace, cAssigned\n"
        "cTrace = ''\n"
        "ON ESCAPE cTrace = cTrace + 'E'\n"
        "ON KEY LABEL ESC cTrace = cTrace + 'K'\n"
        "cAssigned = ON('ESCAPE')\n"
        "READ EVENTS\n"
        "RETURN\n");

    auto enabled_options = make_runtime_session_options(enabled_path, temp_root);
    enabled_options.startup_source_text = read_text(enabled_path);
    PrgRuntimeSession enabled = PrgRuntimeSession::create(enabled_options);
    auto state = enabled.run(DebugResumeAction::continue_run);
    expect(state.reason == DebugPauseReason::event_loop, "ON ESCAPE fixture should enter READ EVENTS");
    const auto assigned = state.globals.find("cassigned");
    expect(assigned != state.globals.end() &&
               format_value(assigned->second) == "cTrace = cTrace + 'E'",
           "ON('ESCAPE') should return the assigned command exactly");
    expect(enabled.dispatch_key_label("ESC"),
           "ON ESCAPE should take priority over ON KEY LABEL ESC when SET ESCAPE is ON");
    state = enabled.run(DebugResumeAction::continue_run);
    const auto trace_after_escape = state.globals.find("ctrace");
    expect(state.reason == DebugPauseReason::event_loop && trace_after_escape != state.globals.end() &&
               format_value(trace_after_escape->second) == "E",
           "ON ESCAPE should return to READ EVENTS after executing its static command");
    enabled.request_escape();
    state = enabled.run(DebugResumeAction::continue_run);
    const auto trace_after_host_request = state.globals.find("ctrace");
    expect(state.reason == DebugPauseReason::event_loop && trace_after_host_request != state.globals.end() &&
               format_value(trace_after_host_request->second) == "EE",
           "a host Escape request should be consumed at the next safe event-loop boundary");

    const fs::path disabled_root = fs::temp_directory_path() / "copperfin_prg_engine_on_escape_disabled";
    fs::remove_all(disabled_root, ignored);
    fs::create_directories(disabled_root);
    const fs::path disabled_path = disabled_root / "on_escape_disabled.prg";
    const std::string disabled_source =
        "PUBLIC cTrace\n"
        "cTrace = ''\n"
        "ON ESCAPE cTrace = cTrace + 'E'\n"
        "ON KEY LABEL ESC cTrace = cTrace + 'K'\n"
        "SET ESCAPE OFF\n"
        "READ EVENTS\n"
        "RETURN\n";
    write_text(disabled_path, disabled_source);
    auto disabled_options = make_runtime_session_options(disabled_path, disabled_root);
    disabled_options.startup_source_text = disabled_source;
    PrgRuntimeSession disabled = PrgRuntimeSession::create(disabled_options);
    state = disabled.run(DebugResumeAction::continue_run);
    expect(state.reason == DebugPauseReason::event_loop,
           "SET ESCAPE OFF fixture should enter READ EVENTS: " + state.message);
    expect(!disabled.dispatch_escape(), "SET ESCAPE OFF should suppress direct ON ESCAPE dispatch");
    expect(disabled.dispatch_key_label("ESC"),
           "SET ESCAPE OFF should leave an ON KEY LABEL ESC assignment eligible to run");
    state = disabled.run(DebugResumeAction::continue_run);
    const auto trace_after_key = state.globals.find("ctrace");
    expect(state.reason == DebugPauseReason::event_loop && trace_after_key != state.globals.end() &&
               format_value(trace_after_key->second) == "K",
           "ON KEY LABEL ESC should run when ON ESCAPE is gated off");

    fs::remove_all(temp_root, ignored);
    fs::remove_all(disabled_root, ignored);
}

}  // namespace

int main()
{
    test_on_escape_assignment_priority_and_set_escape_gate();
    if (copperfin::test_support::test_failures() != 0)
    {
        std::cerr << copperfin::test_support::test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }
    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
