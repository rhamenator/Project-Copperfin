// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

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

void test_sleep_command_emits_runtime_sleep_event() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_sleep_cmd";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "sleep_test.prg";
    write_text(
        main_path,
        "nDelay = 1\n"
        "SLEEP nDelay\n"
        "nAfter = 42\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SLEEP test should complete");

    const auto sleep_event = std::find_if(
        state.events.begin(), state.events.end(),
        [](const auto& event) { return event.category == "runtime.sleep"; });
    expect(sleep_event != state.events.end(), "SLEEP should emit a runtime.sleep event");
    if (sleep_event != state.events.end()) {
        expect(sleep_event->detail.find("duration=1ms") != std::string::npos,
            "SLEEP event should report the resolved duration");
    }

    const auto after_it = state.globals.find("nafter");
    expect(after_it != state.globals.end(), "SLEEP script should continue after the delay");
    if (after_it != state.globals.end()) {
        expect(after_it->second.number_value == 42.0, "SLEEP should not disturb later statements");
    }

    fs::remove_all(temp_root, ignored);
}

void test_spawn_and_await_command_runs_task_to_completion() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_spawn_await_cmd";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "spawn_await_test.prg";
    write_text(
        main_path,
        "PROCEDURE worker\n"
        "    SLEEP 1\n"
        "    RETURN\n"
        "ENDPROC\n"
        "SPAWN worker TO nTask\n"
        "AWAIT nTask TO lDone\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SPAWN/AWAIT test should complete");

    const auto spawn_event = std::find_if(
        state.events.begin(), state.events.end(),
        [](const auto& event) { return event.category == "runtime.task.spawn"; });
    const auto await_event = std::find_if(
        state.events.begin(), state.events.end(),
        [](const auto& event) { return event.category == "runtime.task.await"; });
    expect(spawn_event != state.events.end(), "SPAWN should emit a task-spawn event");
    expect(await_event != state.events.end(), "AWAIT should emit a task-await event");
    if (spawn_event != state.events.end()) {
        expect(spawn_event->detail.find("handle=") != std::string::npos,
            "SPAWN event should report a task handle");
    }
    if (await_event != state.events.end()) {
        expect(await_event->detail.find("state=completed") != std::string::npos,
            "AWAIT event should report completed task state");
    }

    const auto handle_it = state.globals.find("ntask");
    const auto done_it = state.globals.find("ldone");
    expect(handle_it != state.globals.end(), "SPAWN should assign the task handle");
    expect(done_it != state.globals.end(), "AWAIT should assign the completion flag");
    if (handle_it != state.globals.end()) {
        expect(handle_it->second.number_value > 0.0, "SPAWN should return a positive task handle");
    }
    if (done_it != state.globals.end()) {
        expect(done_it->second.boolean_value, "AWAIT should report a completed task");
    }

    fs::remove_all(temp_root, ignored);
}

void test_spawn_cancellation_propagates_to_sibling_tasks() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_spawn_cancel_cmd";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "spawn_cancel_test.prg";
    write_text(
        main_path,
        "PROCEDURE worker\n"
        "    SLEEP 50\n"
        "    RETURN\n"
        "ENDPROC\n"
        "PROCEDURE canceler\n"
        "    SLEEP 1\n"
        "    CANCEL\n"
        "ENDPROC\n"
        "SPAWN worker TO nWorker\n"
        "SPAWN canceler TO nCancel\n"
        "AWAIT nCancel TO lCancelDone\n"
        "AWAIT nWorker TO lWorkerDone\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "spawn cancellation test should complete");

    const auto cancelled_event = std::find_if(
        state.events.begin(), state.events.end(),
        [](const auto& event) { return event.category == "runtime.task.cancelled"; });
    const auto cancel_await_event = std::find_if(
        state.events.begin(), state.events.end(),
        [](const auto& event) { return event.category == "runtime.task.await" && event.detail.find("handle=") != std::string::npos && event.detail.find("state=error") != std::string::npos; });
    expect(cancelled_event != state.events.end(), "cancellation should emit a runtime.task.cancelled event");
    expect(cancel_await_event != state.events.end(), "awaiting a cancelled task should report an error state");

    const auto cancel_done = state.globals.find("lcanceldone");
    const auto worker_done = state.globals.find("lworkerdone");
    expect(cancel_done != state.globals.end(), "canceler completion flag should be captured");
    expect(worker_done != state.globals.end(), "worker completion flag should be captured");
    if (cancel_done != state.globals.end()) {
        expect(!cancel_done->second.boolean_value, "canceler task should not report completed after CANCEL");
    }
    if (worker_done != state.globals.end()) {
        expect(!worker_done->second.boolean_value, "worker task should be marked incomplete after cancellation");
    }

    fs::remove_all(temp_root, ignored);
}

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
