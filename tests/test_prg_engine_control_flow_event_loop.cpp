// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_prg_engine_control_flow_support.h"

namespace cf_test_prg_engine_control_flow {
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
}
