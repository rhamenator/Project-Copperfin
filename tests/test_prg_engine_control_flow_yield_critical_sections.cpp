// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_prg_engine_control_flow_support.h"

namespace cf_test_prg_engine_control_flow {
void test_yield_allowed_in_enter_critical_regression_minimal() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_yield_enter_critical_minimal_regression";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "yield_enter_critical_minimal_regression_test.prg";
    write_text(
        main_path,
        "entered = .F.\n"
        "yielded = .F.\n"
        "ENTER CRITICAL\n"
        "entered = .T.\n"
        "YIELD\n"
        "yielded = .T.\n"
        "EXIT CRITICAL\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "ENTER CRITICAL/YIELD minimal regression should complete");
    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.critical.enter";
    }), "ENTER CRITICAL should emit runtime.critical.enter");
    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.yield" && event.detail.find("operation=YIELD") != std::string::npos;
    }), "ENTER CRITICAL + YIELD should emit operation-tagged runtime.yield");
    expect(std::none_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.critical.blocking_violation" &&
               event.detail.find("operation=YIELD") != std::string::npos;
    }), "YIELD in ENTER CRITICAL remains the intentional policy exception");

    fs::remove_all(temp_root, ignored);
}
void test_yield_in_enter_critical_has_no_blocking_violation() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_yield_enter_critical_locking_regression";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "yield_enter_critical_locking_regression_test.prg";
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
    expect(state.completed, "ENTER CRITICAL should allow YIELD without blocking-policy error");

    const auto entered = state.globals.find("lenteredcritical");
    const auto yielded = state.globals.find("lyieldedincritical");
    expect(entered != state.globals.end(), "ENTER CRITICAL body should execute");
    expect(yielded != state.globals.end(), "YIELD inside ENTER CRITICAL should execute");
    if (entered != state.globals.end())
    {
        expect(entered->second.boolean_value, "critical section body should run before YIELD");
    }
    if (yielded != state.globals.end())
    {
        expect(yielded->second.boolean_value, "YIELD should complete inside ENTER CRITICAL");
    }

    const auto yield_event = std::find_if(state.events.begin(), state.events.end(), [](const auto &event) {
        return event.category == "runtime.yield" &&
               event.detail.find("operation=YIELD") != std::string::npos;
    });
    expect(yield_event != state.events.end(), "YIELD should emit operation-tagged runtime.yield event");
    expect(std::none_of(state.events.begin(), state.events.end(), [](const auto &event) {
        return event.category == "runtime.critical.blocking_violation" &&
               event.detail.find("operation=YIELD") != std::string::npos;
    }), "YIELD should be exempt from critical-section blocking rule");

    const auto critical_enter = std::find_if(state.events.begin(), state.events.end(), [](const auto &event) {
        return event.category == "runtime.critical.enter";
    });
    const auto critical_exit = std::find_if(state.events.begin(), state.events.end(), [](const auto &event) {
        return event.category == "runtime.critical.exit";
    });
    expect(critical_enter != state.events.end(), "ENTER CRITICAL should emit runtime.critical.enter");
    expect(critical_exit != state.events.end(), "EXIT CRITICAL should emit runtime.critical.exit");
    if (critical_enter != state.events.end() && critical_exit != state.events.end() && yield_event != state.events.end())
    {
        expect(std::distance(state.events.begin(), critical_enter) <
               std::distance(state.events.begin(), yield_event),
               "runtime.yield should happen after ENTER CRITICAL");
        expect(std::distance(state.events.begin(), yield_event) <
               std::distance(state.events.begin(), critical_exit),
               "runtime.yield should happen before EXIT CRITICAL");
    }

    fs::remove_all(temp_root, ignored);
}
void test_yield_in_enter_critical_is_explicit_policy_exception_regression() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_yield_enter_critical_policy_exception_regression";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "yield_enter_critical_policy_exception_regression_test.prg";
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
    expect(state.completed, "ENTER CRITICAL/YIELD policy-exception regression should complete");

    const auto entered = state.globals.find("lenteredcritical");
    const auto yielded = state.globals.find("lyieldedincritical");
    expect(entered != state.globals.end(), "policy exception should execute CRITICAL body");
    expect(yielded != state.globals.end(), "YIELD should execute inside ENTER CRITICAL");
    if (entered != state.globals.end()) {
        expect(entered->second.boolean_value, "policy exception requires CRITICAL body before yield");
    }
    if (yielded != state.globals.end()) {
        expect(yielded->second.boolean_value, "yield should continue inside policy exception path");
    }

    const auto yield_event = std::find_if(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.yield" && event.detail.find("operation=YIELD") != std::string::npos;
    });
    expect(yield_event != state.events.end(), "policy exception should emit operation-tagged runtime.yield");

    const auto critical_enter = std::find_if(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.critical.enter";
    });
    const auto critical_exit = std::find_if(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.critical.exit";
    });
    expect(critical_enter != state.events.end(), "ENTER CRITICAL should emit runtime.critical.enter");
    expect(critical_exit != state.events.end(), "EXIT CRITICAL should emit runtime.critical.exit");
    if (critical_enter != state.events.end() && critical_exit != state.events.end() && yield_event != state.events.end()) {
        expect(std::distance(state.events.begin(), critical_enter) <
               std::distance(state.events.begin(), yield_event),
               "runtime.yield should occur after ENTER CRITICAL");
        expect(std::distance(state.events.begin(), yield_event) <
               std::distance(state.events.begin(), critical_exit),
               "runtime.yield should occur before EXIT CRITICAL");
    }

    expect(std::none_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.critical.blocking_violation" &&
               event.detail.find("operation=YIELD") != std::string::npos;
    }), "policy exception should not emit blocking violation for YIELD");

    fs::remove_all(temp_root, ignored);
}
void test_enter_critical_allows_yield_without_blocking_violation_event() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_enter_critical_yield_policy_contract";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "yield_enter_critical_policy_contract_test.prg";
    write_text(
        main_path,
        "lEnteredCritical = .F.\n"
        "lYielded = .F.\n"
        "ENTER CRITICAL\n"
        "lEnteredCritical = .T.\n"
        "YIELD\n"
        "lYielded = .T.\n"
        "EXIT CRITICAL\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "ENTER CRITICAL should allow YIELD as a policy exception");

    const auto entered = state.globals.find("lenteredcritical");
    const auto yielded = state.globals.find("lyielded");
    expect(entered != state.globals.end(), "policy test should execute CRITICAL body");
    expect(yielded != state.globals.end(), "policy test should execute YIELD in CRITICAL");
    if (entered != state.globals.end()) {
        expect(entered->second.boolean_value, "CRITICAL body should run before YIELD");
    }
    if (yielded != state.globals.end()) {
        expect(yielded->second.boolean_value, "YIELD should continue after yielding");
    }

    const auto enter_event = std::find_if(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.critical.enter";
    });
    const auto yield_event = std::find_if(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.yield" &&
               event.detail.find("operation=YIELD") != std::string::npos;
    });
    const auto exit_event = std::find_if(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.critical.exit";
    });
    expect(enter_event != state.events.end(), "ENTER CRITICAL should emit runtime.critical.enter");
    expect(yield_event != state.events.end(), "YIELD in CRITICAL should emit operation-tagged runtime.yield");
    expect(exit_event != state.events.end(), "EXIT CRITICAL should emit runtime.critical.exit");

    if (enter_event != state.events.end() && yield_event != state.events.end() && exit_event != state.events.end()) {
        expect(std::distance(state.events.begin(), enter_event) < std::distance(state.events.begin(), yield_event),
               "YIELD should execute after ENTER CRITICAL");
        expect(std::distance(state.events.begin(), yield_event) < std::distance(state.events.begin(), exit_event),
               "YIELD should execute before EXIT CRITICAL");
    }

    expect(std::none_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.critical.blocking_violation" &&
               event.detail.find("operation=YIELD") != std::string::npos;
    }), "YIELD inside ENTER CRITICAL should not emit a blocking violation");

    fs::remove_all(temp_root, ignored);
}
void test_yield_is_allowed_while_holding_critical_section() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_yield_critical_regression";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "yield_critical_regression.prg";
    write_text(
        main_path,
        "lInCritical = .F.\n"
        "lYielded = .F.\n"
        "ENTER CRITICAL shared\n"
        "lInCritical = .T.\n"
        "YIELD\n"
        "lYielded = .T.\n"
        "EXIT CRITICAL shared\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "YIELD inside CRITICAL should complete");

    const auto entered_it = state.globals.find("lincritical");
    const auto yielded_it = state.globals.find("lyielded");
    expect(entered_it != state.globals.end(), "CRITICAL body should execute");
    expect(yielded_it != state.globals.end(), "YIELD should execute inside CRITICAL");
    if (entered_it != state.globals.end()) {
        expect(entered_it->second.boolean_value, "CRITICAL section body should run before YIELD");
    }
    if (yielded_it != state.globals.end()) {
        expect(yielded_it->second.boolean_value, "YIELD should run and continue inside CRITICAL");
    }

    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.yield";
    }), "YIELD inside CRITICAL should emit runtime.yield");
    expect(std::none_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.critical.blocking_violation" &&
               event.detail.find("operation=YIELD") != std::string::npos;
    }), "YIELD inside CRITICAL should not trigger blocking policy");

    fs::remove_all(temp_root, ignored);
}
void test_yield_in_critical_section_keeps_section_semantics() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_yield_critical_section_contract";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "yield_in_critical_section_contract_test.prg";
    write_text(
        main_path,
        "lInside = .F.\n"
        "lYielded = .F.\n"
        "ENTER CRITICAL shared\n"
        "lInside = .T.\n"
        "YIELD\n"
        "lYielded = .T.\n"
        "EXIT CRITICAL shared\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "YIELD in CRITICAL should complete");

    const auto entered = state.globals.find("linside");
    const auto yielded = state.globals.find("lyielded");
    expect(entered != state.globals.end(), "CRITICAL body should execute");
    expect(yielded != state.globals.end(), "YIELD should continue inside CRITICAL");
    if (entered != state.globals.end()) {
        expect(entered->second.boolean_value, "CRITICAL section should be entered before YIELD");
    }
    if (yielded != state.globals.end()) {
        expect(yielded->second.boolean_value, "YIELD should complete inside CRITICAL");
    }
    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.yield";
    }), "CRITICAL-held YIELD should emit runtime.yield");
    expect(std::none_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.critical.blocking_violation" &&
               event.detail.find("operation=YIELD") != std::string::npos;
    }), "YIELD in CRITICAL should be a policy exception");

    fs::remove_all(temp_root, ignored);
}
void test_yield_is_allowed_in_default_critical_section_is_policy_exception() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_yield_default_critical_policy_exception";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "yield_default_critical_policy_exception_test.prg";
    write_text(
        main_path,
        "lEntered = .F.\n"
        "lYieldedInDefault = .F.\n"
        "ENTER CRITICAL\n"
        "lEntered = .T.\n"
        "YIELD\n"
        "lYieldedInDefault = .T.\n"
        "EXIT CRITICAL\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "YIELD in default CRITICAL should complete");

    const auto entered = state.globals.find("lentered");
    const auto yielded = state.globals.find("lyieldedindefault");
    expect(entered != state.globals.end(), "default CRITICAL body should execute");
    expect(yielded != state.globals.end(), "YIELD should execute inside default CRITICAL");
    if (entered != state.globals.end()) {
        expect(entered->second.boolean_value, "default CRITICAL should be entered before YIELD");
    }
    if (yielded != state.globals.end()) {
        expect(yielded->second.boolean_value, "YIELD should continue execution inside default CRITICAL");
    }

    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.yield";
    }), "YIELD in default CRITICAL should emit runtime.yield");
    expect(std::none_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.critical.blocking_violation" &&
               event.detail.find("operation=YIELD") != std::string::npos;
    }), "YIELD in default CRITICAL should remain an allowed policy exception");

    fs::remove_all(temp_root, ignored);
}
void test_yield_is_allowed_in_critical_section_is_policy_exception() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_yield_critical_policy_exception";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "yield_critical_policy_exception_test.prg";
    write_text(
        main_path,
        "lEnteredCritical = .F.\n"
        "lYieldedInCritical = .F.\n"
        "ENTER CRITICAL shared\n"
        "lEnteredCritical = .T.\n"
        "YIELD\n"
        "lYieldedInCritical = .T.\n"
        "EXIT CRITICAL shared\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "YIELD policy exception test should complete");

    const auto entered = state.globals.find("lenteredcritical");
    const auto yielded = state.globals.find("lyieldedincritical");
    expect(entered != state.globals.end(), "CRITICAL body should execute");
    expect(yielded != state.globals.end(), "YIELD should execute inside CRITICAL");
    if (entered != state.globals.end()) {
        expect(entered->second.boolean_value, "CRITICAL section should be entered before YIELD");
    }
    if (yielded != state.globals.end()) {
        expect(yielded->second.boolean_value, "YIELD should continue execution inside CRITICAL");
    }

    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.yield";
    }), "YIELD in CRITICAL should emit runtime.yield");

    expect(std::none_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.critical.blocking_violation" &&
               event.detail.find("operation=YIELD") != std::string::npos;
    }), "YIELD in CRITICAL should remain an allowed policy exception");

    fs::remove_all(temp_root, ignored);
}
void test_yield_inside_critical_section_is_allowed() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_yield_inside_critical_cmd";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "yield_inside_critical_test.prg";
    write_text(
        main_path,
        "PROCEDURE worker\n"
        "    SLEEP 1\n"
        "    RETURN\n"
        "ENDPROC\n"
        "lYieldedInCritical = .F.\n"
        "SPAWN worker TO nTask\n"
        "ENTER CRITICAL shared\n"
        "YIELD\n"
        "lYieldedInCritical = .T.\n"
        "EXIT CRITICAL shared\n"
        "AWAIT nTask TO lDone\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "YIELD inside critical-section test should complete");

    const auto yield_in_critical = state.globals.find("lyieldedincritical");
    const auto await_done = state.globals.find("ldone");
    expect(yield_in_critical != state.globals.end(), "YIELD inside CRITICAL should execute and set lYieldedInCritical");
    expect(await_done != state.globals.end(), "post-YIELD await should still complete");
    if (yield_in_critical != state.globals.end()) {
        expect(yield_in_critical->second.boolean_value, "YIELD inside CRITICAL should complete without entering CATCH path");
    }
    if (await_done != state.globals.end()) {
        expect(await_done->second.boolean_value, "AWAIT after CRITICAL/YIELD should report completed task");
    }

    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.yield";
    }), "YIELD inside CRITICAL should emit a runtime.yield event");
    expect(std::none_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.critical.blocking_violation" &&
               event.detail.find("operation=YIELD") != std::string::npos;
    }), "YIELD inside CRITICAL should remain allowed and not trigger blocking policy");

    fs::remove_all(temp_root, ignored);
}
void test_yield_allowed_in_enter_critical_is_small_regression() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_yield_enter_critical_regression";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "yield_enter_critical_regression_test.prg";
    write_text(
        main_path,
        "lYielded = .F.\n"
        "lEnteredCritical = .F.\n"
        "ENTER CRITICAL\n"
        "lEnteredCritical = .T.\n"
        "YIELD\n"
        "lYielded = .T.\n"
        "EXIT CRITICAL\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "ENTER CRITICAL/YIELD regression should complete");

    const auto yielded = state.globals.find("lyielded");
    const auto entered = state.globals.find("lenteredcritical");
    expect(yielded != state.globals.end(), "YIELD should continue after ENTER CRITICAL");
    if (yielded != state.globals.end()) {
        expect(yielded->second.boolean_value, "YIELD should set lYielded after resuming from section");
    }
    expect(entered != state.globals.end(), "ENTER CRITICAL body should execute before YIELD");
    if (entered != state.globals.end()) {
        expect(entered->second.boolean_value, "CRITICAL body should set entry flag");
    }

    const auto yield_event_pos = std::find_if(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.yield";
    });
    expect(yield_event_pos != state.events.end(), "ENTER CRITICAL + YIELD should emit runtime.yield");
    const auto first_critical_enter = std::find_if(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.critical.enter";
    });
    const auto first_critical_exit = std::find_if(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.critical.exit";
    });
    expect(first_critical_enter != state.events.end(), "ENTER CRITICAL should emit runtime.critical.enter");
    expect(first_critical_exit != state.events.end(), "EXIT CRITICAL should emit runtime.critical.exit");
    if (first_critical_enter != state.events.end() && first_critical_exit != state.events.end() &&
        yield_event_pos != state.events.end()) {
        expect(std::distance(state.events.begin(), first_critical_enter) <
               std::distance(state.events.begin(), yield_event_pos),
               "runtime.yield should occur after entering CRITICAL");
        expect(std::distance(state.events.begin(), yield_event_pos) <
               std::distance(state.events.begin(), first_critical_exit),
               "runtime.yield should occur before CRITICAL exit");
    }
    expect(std::none_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.critical.blocking_violation" &&
               event.detail.find("operation=YIELD") != std::string::npos;
    }), "ENTER CRITICAL + YIELD should remain blocked-policy exception");
    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.yield" &&
               event.detail.find("operation=YIELD") != std::string::npos;
    }), "YIELD event detail should identify the operation");

    fs::remove_all(temp_root, ignored);
}
void test_yield_allowed_in_reentrant_enter_critical_section() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_yield_reentrant_enter_critical_regression";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "yield_reentrant_enter_critical_regression_test.prg";
    write_text(
        main_path,
        "lOuterEntered = .F.\n"
        "lInnerEntered = .F.\n"
        "lYielded = .F.\n"
        "ENTER CRITICAL shared\n"
        "lOuterEntered = .T.\n"
        "ENTER CRITICAL shared\n"
        "lInnerEntered = .T.\n"
        "YIELD\n"
        "lYielded = .T.\n"
        "EXIT CRITICAL shared\n"
        "EXIT CRITICAL shared\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "YIELD inside reentrant ENTER CRITICAL should complete");

    const auto outer_entered = state.globals.find("louterentered");
    const auto inner_entered = state.globals.find("linnerentered");
    const auto yielded = state.globals.find("lyielded");
    expect(outer_entered != state.globals.end(), "outer ENTER CRITICAL body should execute");
    expect(inner_entered != state.globals.end(), "inner ENTER CRITICAL body should execute");
    expect(yielded != state.globals.end(), "YIELD should execute inside reentrant ENTER CRITICAL");
    if (outer_entered != state.globals.end()) {
        expect(outer_entered->second.boolean_value, "outer CRITICAL entry flag should be true");
    }
    if (inner_entered != state.globals.end()) {
        expect(inner_entered->second.boolean_value, "inner CRITICAL entry flag should be true");
    }
    if (yielded != state.globals.end()) {
        expect(yielded->second.boolean_value, "YIELD should execute and set lYielded");
    }

    const auto critical_enter_count =
        static_cast<std::size_t>(std::count_if(state.events.begin(), state.events.end(), [](const auto& event) {
            return event.category == "runtime.critical.enter";
        }));
    const auto critical_exit_count =
        static_cast<std::size_t>(std::count_if(state.events.begin(), state.events.end(), [](const auto& event) {
            return event.category == "runtime.critical.exit";
        }));
    expect(critical_enter_count == 2U, "reentrant ENTER CRITICAL should emit two enter events");
    expect(critical_exit_count == 2U, "reentrant ENTER CRITICAL should emit two exit events");

    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.yield" && event.detail.find("operation=YIELD") != std::string::npos;
    }), "reentrant ENTER CRITICAL should emit runtime.yield");
    expect(std::none_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.critical.blocking_violation" &&
               event.detail.find("operation=YIELD") != std::string::npos;
    }), "YIELD inside reentrant ENTER CRITICAL should remain allowed");

    fs::remove_all(temp_root, ignored);
}
void test_yield_in_enter_critical_is_explicit_policy_exception_small() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_yield_enter_critical_policy_exception_small";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "yield_enter_critical_policy_exception_small.prg";
    write_text(
        main_path,
        "entered = .F.\n"
        "yielded = .F.\n"
        "ENTER CRITICAL\n"
        "entered = .T.\n"
        "YIELD\n"
        "yielded = .T.\n"
        "EXIT CRITICAL\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "ENTER CRITICAL should allow YIELD as a policy exception");

    const auto entered = state.globals.find("entered");
    const auto yielded = state.globals.find("yielded");
    expect(entered != state.globals.end(), "ENTER CRITICAL body should execute");
    expect(yielded != state.globals.end(), "YIELD should execute inside ENTER CRITICAL");
    if (entered != state.globals.end()) {
        expect(entered->second.boolean_value, "CRITICAL body should run before YIELD");
    }
    if (yielded != state.globals.end()) {
        expect(yielded->second.boolean_value, "YIELD should complete inside CRITICAL");
    }

    const auto yield_event = std::find_if(state.events.begin(), state.events.end(), [](const auto &event) {
        return event.category == "runtime.yield" &&
               event.detail.find("operation=YIELD") != std::string::npos;
    });
    const auto enter_event = std::find_if(state.events.begin(), state.events.end(), [](const auto &event) {
        return event.category == "runtime.critical.enter";
    });
    const auto exit_event = std::find_if(state.events.begin(), state.events.end(), [](const auto &event) {
        return event.category == "runtime.critical.exit";
    });
    expect(yield_event != state.events.end(), "YIELD in ENTER CRITICAL should emit runtime.yield");
    expect(enter_event != state.events.end(), "ENTER CRITICAL should emit runtime.critical.enter");
    expect(exit_event != state.events.end(), "EXIT CRITICAL should emit runtime.critical.exit");
    expect(std::none_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.critical.blocking_violation" &&
               event.detail.find("operation=YIELD") != std::string::npos;
    }), "YIELD should remain an explicit policy exception while critical section is held");

    if (yield_event != state.events.end() && enter_event != state.events.end() &&
        exit_event != state.events.end()) {
        expect(std::distance(state.events.begin(), enter_event) <
               std::distance(state.events.begin(), yield_event),
               "YIELD should occur after ENTER CRITICAL");
        expect(std::distance(state.events.begin(), yield_event) <
               std::distance(state.events.begin(), exit_event),
               "YIELD should occur before EXIT CRITICAL");
    }

    fs::remove_all(temp_root, ignored);
}
}  // namespace cf_test_prg_engine_control_flow
