// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_prg_engine_control_flow_support.h"

namespace cf_test_prg_engine_control_flow {
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
}  // namespace cf_test_prg_engine_control_flow
