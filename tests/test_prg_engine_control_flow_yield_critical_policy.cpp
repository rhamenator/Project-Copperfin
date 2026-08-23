// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_prg_engine_control_flow_support.h"

namespace cf_test_prg_engine_control_flow {
void test_yield_is_explicit_policy_exception_in_enter_critical() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_enter_critical_yield_policy_exception";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "yield_enter_critical_policy_exception_test.prg";
    write_text(
        main_path,
        "lEnteredCritical = .F.\n"
        "lYieldedInCritical = .F.\n"
        "ENTER CRITICAL\n"
        "lEnteredCritical = .T.\n"
        "YIELD\n"
        "lYieldedInCritical = .T.\n"
        "EXIT CRITICAL\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "YIELD should be allowed inside ENTER CRITICAL");

    const auto entered = state.globals.find("lenteredcritical");
    const auto yielded = state.globals.find("lyieldedincritical");
    expect(entered != state.globals.end(), "ENTER CRITICAL body should execute");
    expect(yielded != state.globals.end(), "YIELD should execute inside ENTER CRITICAL");

    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto &event) {
        return event.category == "runtime.critical.enter";
    }), "ENTER CRITICAL should emit critical-enter event");
    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto &event) {
        return event.category == "runtime.yield" && event.detail.find("operation=YIELD") != std::string::npos;
    }), "ENTER CRITICAL body should emit operation-tagged runtime.yield");
    expect(std::none_of(state.events.begin(), state.events.end(), [](const auto &event) {
        return event.category == "runtime.critical.blocking_violation" &&
               event.detail.find("operation=YIELD") != std::string::npos;
    }), "YIELD should remain policy exception while in CRITICAL section");

    const auto yield_event_it = std::find_if(state.events.begin(), state.events.end(), [](const auto &event) {
        return event.category == "runtime.yield" && event.detail.find("operation=YIELD") != std::string::npos;
    });
    const auto enter_event_it = std::find_if(state.events.begin(), state.events.end(), [](const auto &event) {
        return event.category == "runtime.critical.enter";
    });
    const auto exit_event_it = std::find_if(state.events.begin(), state.events.end(), [](const auto &event) {
        return event.category == "runtime.critical.exit";
    });
    if (yield_event_it != state.events.end() && enter_event_it != state.events.end() && exit_event_it != state.events.end()) {
        expect(std::distance(state.events.begin(), enter_event_it) <
               std::distance(state.events.begin(), yield_event_it),
               "runtime.yield should occur after ENTER CRITICAL");
        expect(std::distance(state.events.begin(), yield_event_it) <
               std::distance(state.events.begin(), exit_event_it),
               "runtime.yield should occur before EXIT CRITICAL");
    }

    fs::remove_all(temp_root, ignored);
}
} // namespace cf_test_prg_engine_control_flow
