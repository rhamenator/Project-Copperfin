// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_prg_engine_control_flow_support.h"

namespace cf_test_prg_engine_control_flow {
void test_spawn_critical_section_serializes_workers() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_spawn_critical_cmd";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "spawn_critical_test.prg";
    write_text(
        main_path,
        "PROCEDURE worker\n"
        "    ENTER CRITICAL shared\n"
        "    FOR nSpin = 1 TO 50\n"
        "        YIELD\n"
        "    ENDFOR\n"
        "    EXIT CRITICAL shared\n"
        "    RETURN\n"
        "ENDPROC\n"
        "SPAWN worker TO nFirst\n"
        "SPAWN worker TO nSecond\n"
        "AWAIT nFirst TO lFirstDone\n"
        "AWAIT nSecond TO lSecondDone\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "critical-section spawn test should complete");

    const auto enter_count = std::count_if(
        state.events.begin(), state.events.end(),
        [](const auto& event) { return event.category == "runtime.critical.enter"; });
    const auto exit_count = std::count_if(
        state.events.begin(), state.events.end(),
        [](const auto& event) { return event.category == "runtime.critical.exit"; });
    expect(enter_count == 2, "both workers should enter the critical section");
    expect(exit_count == 2, "both workers should exit the critical section");

    const auto first_done = state.globals.find("lfirstdone");
    const auto second_done = state.globals.find("lseconddone");
    expect(first_done != state.globals.end(), "first worker completion flag should be captured");
    expect(second_done != state.globals.end(), "second worker completion flag should be captured");
    if (first_done != state.globals.end()) {
        expect(first_done->second.boolean_value, "first worker should complete successfully");
    }
    if (second_done != state.globals.end()) {
        expect(second_done->second.boolean_value, "second worker should complete successfully");
    }

    fs::remove_all(temp_root, ignored);
}

void test_critical_section_order_policy_rejects_descending_nested_acquire() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_critical_order_policy";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "critical_order_test.prg";
    write_text(
        main_path,
        "PROCEDURE workergood\n"
        "    ENTER CRITICAL alpha\n"
        "    ENTER CRITICAL beta\n"
        "    EXIT CRITICAL beta\n"
        "    EXIT CRITICAL alpha\n"
        "    RETURN\n"
        "ENDPROC\n"
        "PROCEDURE workerbad\n"
        "    ENTER CRITICAL beta\n"
        "    ENTER CRITICAL alpha\n"
        "    EXIT CRITICAL beta\n"
        "    RETURN\n"
        "ENDPROC\n"
        "SPAWN workergood TO nGood\n"
        "SPAWN workerbad TO nBad\n"
        "AWAIT nGood TO lGoodDone\n"
        "AWAIT nBad TO lBadDone\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "critical-section order-policy script should complete");

    const auto good_done = state.globals.find("lgooddone");
    const auto bad_done = state.globals.find("lbaddone");
    expect(good_done != state.globals.end(), "good worker completion flag should be captured");
    expect(bad_done != state.globals.end(), "bad worker completion flag should be captured");
    if (good_done != state.globals.end()) {
        expect(good_done->second.boolean_value, "ascending nested critical-section order should succeed");
    }
    if (bad_done != state.globals.end()) {
        expect(!bad_done->second.boolean_value, "descending nested critical-section order should fail deterministically");
    }

    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.critical.order_violation" &&
               event.detail.find("held=beta requested=alpha") != std::string::npos;
    }), "descending nested critical-section order should emit a runtime.critical.order_violation event");

    fs::remove_all(temp_root, ignored);
}

void test_critical_section_exit_order_is_enforced() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_critical_exit_order";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "critical_exit_order_test.prg";
    write_text(
        main_path,
        "PROCEDURE workerbad\n"
        "    ENTER CRITICAL alpha\n"
        "    ENTER CRITICAL beta\n"
        "    EXIT CRITICAL alpha\n"
        "    RETURN\n"
        "ENDPROC\n"
        "PROCEDURE workergood\n"
        "    ENTER CRITICAL alpha\n"
        "    ENTER CRITICAL beta\n"
        "    EXIT CRITICAL beta\n"
        "    EXIT CRITICAL alpha\n"
        "    lGoodDone = .T.\n"
        "    RETURN\n"
        "ENDPROC\n"
        "SPAWN workerbad TO nBad\n"
        "SPAWN workergood TO nGood\n"
        "AWAIT nBad TO lBadAwaitDone\n"
        "AWAIT nGood TO lGoodAwaitDone\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "critical-section exit-order policy script should complete");

    const auto bad_await_done = state.globals.find("lbadawaitdone");
    const auto good_await_done = state.globals.find("lgoodawaitdone");
    expect(bad_await_done != state.globals.end(), "bad worker should report await completion");
    expect(good_await_done != state.globals.end(), "good worker should report await completion");
    if (bad_await_done != state.globals.end()) {
        expect(!bad_await_done->second.boolean_value, "out-of-order EXIT CRITICAL should fault the bad worker");
    }
    if (good_await_done != state.globals.end()) {
        expect(good_await_done->second.boolean_value, "good worker should terminate after valid critical usage");
    }

    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.critical.order_violation" &&
               event.detail.find("held=beta requested=alpha") != std::string::npos;
    }), "out-of-order EXIT CRITICAL should emit runtime.critical.order_violation");

    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.task.await" &&
               event.detail.find("state=error") != std::string::npos &&
               event.detail.find("handle=") != std::string::npos;
    }), "bad worker AWAIT should report task error state");

    fs::remove_all(temp_root, ignored);
}

void test_critical_section_reentrant_enter_same_section_is_allowed() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_critical_reentrant_policy";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "critical_reentrant_test.prg";
    write_text(
        main_path,
        "lOuterEntered = .F.\n"
        "lInnerEntered = .F.\n"
        "lInnerExited = .F.\n"
        "lOuterExited = .F.\n"
        "ENTER CRITICAL  shared\n"
        "lOuterEntered = .T.\n"
        "ENTER CRITICAL    sHaReD\n"
        "lInnerEntered = .T.\n"
        "EXIT CRITICAL shared\n"
        "lInnerExited = .T.\n"
        "EXIT CRITICAL shared\n"
        "lOuterExited = .T.\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "critical-section reentrant-enter script should complete");

    const auto outer_entered = state.globals.find("louterentered");
    const auto inner_entered = state.globals.find("linnerentered");
    const auto inner_exited = state.globals.find("linnerexited");
    const auto outer_exited = state.globals.find("louterexited");
    expect(outer_entered != state.globals.end(), "outer critical section should be entered");
    expect(inner_entered != state.globals.end(), "inner critical section should be entered with same normalized section");
    expect(inner_exited != state.globals.end(), "inner critical section should be exited");
    expect(outer_exited != state.globals.end(), "outer critical section should be exited");
    if (outer_entered != state.globals.end()) {
        expect(outer_entered->second.boolean_value, "outer critical section should execute");
    }
    if (inner_entered != state.globals.end()) {
        expect(inner_entered->second.boolean_value, "inner critical section should execute");
    }
    if (inner_exited != state.globals.end()) {
        expect(inner_exited->second.boolean_value, "inner critical exit path should execute");
    }
    if (outer_exited != state.globals.end()) {
        expect(outer_exited->second.boolean_value, "outer critical exit path should execute");
    }

    const auto enter_count = std::count_if(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.critical.enter";
    });
    const auto exit_count = std::count_if(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.critical.exit";
    });
    expect(enter_count == 2U, "same-section re-entry should emit two enter events");
    expect(exit_count == 2U, "same-section re-entry should emit two exit events");

    expect(std::none_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.critical.order_violation";
    }), "same-section re-entry should not emit critical order violations");

    fs::remove_all(temp_root, ignored);
}

void test_critical_section_blocking_policy_rejects_await_inside_section() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_critical_await_policy";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "critical_await_test.prg";
    write_text(
        main_path,
        "PROCEDURE worker\n"
        "    SLEEP 1\n"
        "    RETURN\n"
        "ENDPROC\n"
        "SPAWN worker TO nTask\n"
        "TRY\n"
        "    ENTER CRITICAL shared\n"
        "    AWAIT nTask TO lDone\n"
        "    lAwaitBlocked = .F.\n"
        "CATCH TO err_text\n"
        "    lAwaitBlocked = .T.\n"
        "    cAwaitError = err_text.Message\n"
        "ENDTRY\n"
        "EXIT CRITICAL shared\n"
        "AWAIT nTask TO lDoneAfterExit\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "critical-section AWAIT policy script should complete");

    const auto await_blocked = state.globals.find("lawaitblocked");
    const auto await_error = state.globals.find("cawaiterror");
    const auto done_after_exit = state.globals.find("ldoneafterexit");
    expect(await_blocked != state.globals.end(), "AWAIT policy script should capture the blocking-policy result");
    expect(await_error != state.globals.end(), "AWAIT policy script should capture the blocking-policy message");
    expect(done_after_exit != state.globals.end(), "AWAIT policy script should still await successfully after leaving the critical section");
    if (await_blocked != state.globals.end()) {
        expect(await_blocked->second.boolean_value, "AWAIT inside a critical section should be rejected");
    }
    if (await_error != state.globals.end()) {
        expect(copperfin::runtime::format_value(await_error->second) ==
                   "Blocking operation AWAIT is not allowed while holding CRITICAL section shared",
               "AWAIT policy error should route through the default locale catalog");
    }
    if (done_after_exit != state.globals.end()) {
        expect(done_after_exit->second.boolean_value, "AWAIT should succeed once the critical section is exited");
    }

    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.critical.blocking_violation" &&
               event.detail.find("operation=AWAIT") != std::string::npos;
    }), "AWAIT inside a critical section should emit a runtime.critical.blocking_violation event");

    fs::remove_all(temp_root, ignored);
}

void test_critical_section_blocking_policy_rejects_sleep_inside_section() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_critical_sleep_policy";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "critical_sleep_test.prg";
    write_text(
        main_path,
        "TRY\n"
        "    ENTER CRITICAL shared\n"
        "    SLEEP 5\n"
        "    lSleepBlocked = .F.\n"
        "CATCH TO err_text\n"
        "    lSleepBlocked = .T.\n"
        "    cSleepError = err_text.Message\n"
        "ENDTRY\n"
        "EXIT CRITICAL shared\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "critical-section SLEEP policy script should complete");

    const auto sleep_blocked = state.globals.find("lsleepblocked");
    const auto sleep_error = state.globals.find("csleeperror");
    expect(sleep_blocked != state.globals.end(), "SLEEP policy script should capture the blocking-policy result");
    expect(sleep_error != state.globals.end(), "SLEEP policy script should capture the blocking-policy message");
    if (sleep_blocked != state.globals.end()) {
        expect(sleep_blocked->second.boolean_value, "SLEEP inside a critical section should be rejected");
    }
    if (sleep_error != state.globals.end()) {
        expect(copperfin::runtime::format_value(sleep_error->second) ==
                   "Blocking operation SLEEP is not allowed while holding CRITICAL section shared",
               "SLEEP policy error should route through the default locale catalog");
    }

    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.critical.blocking_violation" &&
               event.detail.find("operation=SLEEP") != std::string::npos;
    }), "SLEEP inside a critical section should emit a runtime.critical.blocking_violation event");

    fs::remove_all(temp_root, ignored);
}

void test_critical_sections_release_on_task_fault_without_deadlock() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_critical_fault_release";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "critical_fault_release_test.prg";
    write_text(
        main_path,
        "PROCEDURE bad_worker\n"
        "    ENTER CRITICAL shared\n"
        "    1 / 0\n"
        "ENDPROC\n"
        "PROCEDURE good_worker\n"
        "    ENTER CRITICAL shared\n"
        "    EXIT CRITICAL shared\n"
        "    RETURN\n"
        "ENDPROC\n"
        "SPAWN bad_worker TO nBad\n"
        "SPAWN good_worker TO nGood\n"
        "AWAIT nBad TO lBadDone\n"
        "AWAIT nGood TO lGoodDone\n"
        "RETURN\n");

    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string(), false));

    std::promise<copperfin::runtime::RuntimePauseState> run_promise;
    auto run_future = run_promise.get_future();
    std::thread run_thread([session = std::move(session), run_promise = std::move(run_promise)]() mutable {
        try
        {
            run_promise.set_value(session.run(copperfin::runtime::DebugResumeAction::continue_run));
        }
        catch (...)
        {
            run_promise.set_exception(std::current_exception());
        }
    });

    const bool finished = run_future.wait_for(std::chrono::seconds(3)) == std::future_status::ready;
    if (!finished)
    {
        run_thread.detach();
        expect(false, "faulted worker in shared CRITICAL should not deadlock later tasks");
        return;
    }

    run_thread.join();
    const auto state = run_future.get();
    expect(state.completed,
           "critical-section fault-recovery script should complete after faulted worker exits");

    const auto bad_done = state.globals.find("lbaddone");
    const auto good_done = state.globals.find("lgooddone");
    expect(bad_done != state.globals.end(), "script should report bad-worker await result");
    expect(good_done != state.globals.end(), "script should report good-worker await result");
    if (bad_done != state.globals.end())
    {
        expect(!bad_done->second.boolean_value,
               "bad worker should report failed completion because of fault");
    }
    if (good_done != state.globals.end())
    {
        expect(good_done->second.boolean_value, "good worker should complete after bad worker fault");
    }

    const auto critical_enter_events = std::count_if(
        state.events.begin(),
        state.events.end(),
        [](const auto& event) {
            return event.category == "runtime.critical.enter";
        });
    expect(critical_enter_events >= 2U,
           "good worker should still enter CRITICAL after bad worker fault");

    expect(std::none_of(state.events.begin(), state.events.end(), [](const auto &event) {
               return event.category == "runtime.critical.blocking_violation" &&
                      event.detail.find("operation=AWAIT") != std::string::npos;
           }),
           "fault-recovery scenario should not emit unrelated AWAIT blocking violation");

    fs::remove_all(temp_root, ignored);
}

void test_yield_preserves_fault_metadata_when_followed_by_error() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_yield_fault_cmd";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "yield_fault_test.prg";
    write_text(
        main_path,
        "YIELD\n"
        "nFail = 1 / 0\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!state.completed, "YIELD fault test should stop on error");
    expect(state.reason == copperfin::runtime::DebugPauseReason::error, "YIELD fault test should pause on error");
    expect(state.location.line == 2U, "fault metadata should point at the post-YIELD faulting line");
    expect(state.statement_text.find("1 / 0") != std::string::npos || state.statement_text.find("1/0") != std::string::npos,
        "fault metadata should preserve the offending statement text");

    const auto yield_event = std::find_if(
        state.events.begin(), state.events.end(),
        [](const auto& event) { return event.category == "runtime.yield"; });
    const auto error_event = std::find_if(
        state.events.begin(), state.events.end(),
        [](const auto& event) { return event.category == "runtime.error"; });
    expect(yield_event != state.events.end(), "YIELD should still emit its runtime.yield event before the fault");
    expect(error_event != state.events.end(), "faulting line should still emit a runtime.error event");

    fs::remove_all(temp_root, ignored);
}

}  // namespace cf_test_prg_engine_control_flow
