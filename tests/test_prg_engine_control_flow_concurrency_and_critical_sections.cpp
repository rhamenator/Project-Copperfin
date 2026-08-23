// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_prg_engine_control_flow_support.h"

namespace cf_test_prg_engine_control_flow {
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

void test_sleep_duration_uses_heap_backed_frame_continuations() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_sleep_continuation";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path depth_path = temp_root / "sleep_depth_limit.prg";
    write_text(
        depth_path,
        "SLEEP recurse(1)\n"
        "RETURN\n"
        "FUNCTION recurse\n"
        "LPARAMETERS depth\n"
        "SLEEP recurse(depth + 1)\n");

    auto depth_options = make_runtime_session_options(depth_path.string(), temp_root.string(), false);
    depth_options.max_call_depth = 2048U;
    copperfin::runtime::PrgRuntimeSession depth_session =
        copperfin::runtime::PrgRuntimeSession::create(depth_options);
    const auto depth_state = depth_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(
        depth_state.reason == copperfin::runtime::DebugPauseReason::error,
        "deep SLEEP duration recursion should stop at the runtime guardrail");
    expect(
        depth_state.message.find("maximum call depth") != std::string::npos,
        "deep SLEEP duration recursion should report the configured call-depth diagnostic");

    const fs::path semantics_path = temp_root / "sleep_expression_semantics.prg";
    write_text(
        semantics_path,
        "calls = 0\n"
        "SLEEP delay()\n"
        "after = calls\n"
        "RETURN\n"
        "FUNCTION delay\n"
        "calls = calls + 1\n"
        "RETURN 0\n");

    copperfin::runtime::PrgRuntimeSession semantics_session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(semantics_path.string(), temp_root.string(), false));
    const auto semantics_state = semantics_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(semantics_state.completed, "SLEEP expression semantics script should complete: " + semantics_state.message);

    const auto after = semantics_state.globals.find("after");
    expect(after != semantics_state.globals.end(), "SLEEP duration UDF should leave the post-call count visible");
    if (after != semantics_state.globals.end()) {
        expect(
            copperfin::runtime::format_value(after->second) == "1",
            "SLEEP should evaluate its duration UDF exactly once");
    }

    const std::size_t sleep_event_count = static_cast<std::size_t>(std::count_if(
        semantics_state.events.begin(),
        semantics_state.events.end(),
        [](const copperfin::runtime::RuntimeEvent &event) {
            return event.category == "runtime.sleep" &&
                   event.detail.find("duration=0ms") != std::string::npos &&
                   event.detail.find("expression=delay()") != std::string::npos;
        }));
    expect(sleep_event_count == 1U, "resumed SLEEP should emit exactly one runtime.sleep event");

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
        "nAwaitResolveCalls = 0\n"
        "PROCEDURE worker\n"
        "    SLEEP 1\n"
        "    RETURN\n"
        "ENDPROC\n"
        "SPAWN worker TO nTask\n"
        "AWAIT resolve_task_handle(nTask) TO lDone\n"
        "RETURN\n"
        "FUNCTION resolve_task_handle\n"
        "LPARAMETERS value\n"
        "nAwaitResolveCalls = nAwaitResolveCalls + 1\n"
        "RETURN value\n"
        "ENDFUNC\n");

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
    const auto resolve_calls_it = state.globals.find("nawaitresolvecalls");
    expect(resolve_calls_it != state.globals.end(), "AWAIT should preserve the handle resolver call counter");
    if (resolve_calls_it != state.globals.end()) {
        expect(copperfin::runtime::format_value(resolve_calls_it->second) == "1",
               "AWAIT should evaluate the UDF-produced handle exactly once");
    }

    fs::remove_all(temp_root, ignored);
}

void test_spawn_task_supervision_observes_status_result_and_output_without_consuming_task() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_task_supervision";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "task_supervision_test.prg";
    write_text(
        main_path,
        "PROCEDURE worker\n"
        "    ? 'alpha'\n"
        "    SLEEP 25\n"
        "    ? 'beta'\n"
        "    RETURN 42\n"
        "ENDPROC\n"
        "SPAWN worker TO nTask\n"
        "cInitialStatus = CFTASKSTATUS(nTask)\n"
        "uEarlyResult = CFTASKRESULT(nTask)\n"
        "DO WHILE CFTASKSTATUS(nTask) == 'running'\n"
        "    YIELD\n"
        "ENDDO\n"
        "cFinalStatus = CFTASKSTATUS(nTask)\n"
        "nTaskResult = CFTASKRESULT(nTask)\n"
        "cTaskOutput = CFTASKOUTPUT(nTask)\n"
        "lCancelFinished = CFTASKCANCEL(nTask)\n"
        "AWAIT nTask TO lAwaitDone\n"
        "cAfterAwaitStatus = CFTASKSTATUS(nTask)\n"
        "SPAWN worker TO nNextTask\n"
        "lHandleAdvanced = nNextTask > nTask\n"
        "AWAIT nNextTask TO lNextDone\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "nonblocking task-supervision script should complete: " + state.message);

    const auto initial_status = state.globals.find("cinitialstatus");
    const auto early_result = state.globals.find("uearlyresult");
    const auto final_status = state.globals.find("cfinalstatus");
    const auto task_result = state.globals.find("ntaskresult");
    const auto task_output = state.globals.find("ctaskoutput");
    const auto cancel_finished = state.globals.find("lcancelfinished");
    const auto await_done = state.globals.find("lawaitdone");
    const auto after_await_status = state.globals.find("cafterawaitstatus");
    const auto handle_advanced = state.globals.find("lhandleadvanced");
    const auto next_done = state.globals.find("lnextdone");
    expect(initial_status != state.globals.end() && initial_status->second.string_value == "running",
           "CFTASKSTATUS should observe a sleeping task without blocking");
    expect(early_result != state.globals.end() &&
               early_result->second.kind == copperfin::runtime::PrgValueKind::empty,
           "CFTASKRESULT should return EMPTY while a task is still running");
    expect(final_status != state.globals.end() && final_status->second.string_value == "completed",
           "CFTASKSTATUS should retain the terminal completion state");
    expect(task_result != state.globals.end() &&
               copperfin::runtime::format_value(task_result->second) == "42",
           "CFTASKRESULT should expose the retained PRG return value");
    expect(task_output != state.globals.end() && task_output->second.string_value == "alpha\nbeta",
           "CFTASKOUTPUT should expose retained print output in emission order");
    expect(cancel_finished != state.globals.end() && !cancel_finished->second.boolean_value,
           "CFTASKCANCEL should reject a terminal task");
    expect(await_done != state.globals.end() && await_done->second.boolean_value,
           "supervision reads must not consume the task before legacy AWAIT");
    expect(after_await_status != state.globals.end() && after_await_status->second.string_value == "unknown",
           "legacy AWAIT should retain its existing consume-and-erase behavior");
    expect(handle_advanced != state.globals.end() && handle_advanced->second.boolean_value,
           "SPAWN should not recycle a consumed task handle");
    expect(next_done != state.globals.end() && next_done->second.boolean_value,
           "a later monotonically allocated task should remain awaitable");

    fs::remove_all(temp_root, ignored);
}

void test_spawn_task_supervision_serializes_same_handle_completion_publication() {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_prg_task_supervision_same_handle";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "task_supervision_same_handle_test.prg";
    write_text(
        main_path,
        "PROCEDURE worker\n"
        "    SLEEP 25\n"
        "    ? 'published once'\n"
        "    RETURN 42\n"
        "ENDPROC\n"
        "PROCEDURE watcher\n"
        "    DO WHILE CFTASKSTATUS(nTarget) == 'running'\n"
        "        YIELD\n"
        "    ENDDO\n"
        "    RETURN CFTASKRESULT(nTarget)\n"
        "ENDPROC\n"
        "SPAWN worker TO nTarget\n"
        "SPAWN watcher TO nWatcherOne\n"
        "SPAWN watcher TO nWatcherTwo\n"
        "AWAIT nWatcherOne TO lWatcherOneDone\n"
        "AWAIT nWatcherTwo TO lWatcherTwoDone\n"
        "nRetainedResult = CFTASKRESULT(nTarget)\n"
        "cRetainedOutput = CFTASKOUTPUT(nTarget)\n"
        "AWAIT nTarget TO lTargetDone\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed,
           "same-handle task-supervision script should complete: " + state.message);

    const auto watcher_one_done = state.globals.find("lwatcheronedone");
    const auto watcher_two_done = state.globals.find("lwatchertwodone");
    const auto retained_result = state.globals.find("nretainedresult");
    const auto retained_output = state.globals.find("cretainedoutput");
    const auto target_done = state.globals.find("ltargetdone");
    expect(watcher_one_done != state.globals.end() && watcher_one_done->second.boolean_value,
           "the first sibling watcher should complete");
    expect(watcher_two_done != state.globals.end() && watcher_two_done->second.boolean_value,
           "the second sibling watcher should complete");
    expect(retained_result != state.globals.end() &&
               copperfin::runtime::format_value(retained_result->second) == "42",
           "concurrent watchers should retain the target result without corruption");
    expect(retained_output != state.globals.end() &&
               retained_output->second.string_value == "published once",
           "concurrent watchers should retain the target output without corruption");
    expect(target_done != state.globals.end() && target_done->second.boolean_value,
           "the target should remain awaitable after concurrent supervision");

    fs::remove_all(temp_root, ignored);
}

void test_spawn_task_supervision_requests_cooperative_cancellation() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_task_supervision_cancel";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "task_supervision_cancel_test.prg";
    write_text(
        main_path,
        "PROCEDURE worker\n"
        "    SLEEP 5000\n"
        "    RETURN 99\n"
        "ENDPROC\n"
        "SPAWN worker TO nTask\n"
        "ENTER CRITICAL supervision\n"
        "lCancelAccepted = CFTASKCANCEL(nTask)\n"
        "cCancelStatus = CFTASKSTATUS(nTask)\n"
        "EXIT CRITICAL supervision\n"
        "AWAIT nTask TO lAwaitDone\n"
        "lCancelUnknown = CFTASKCANCEL(nTask)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "cooperative task-cancellation script should complete: " + state.message);

    const auto cancel_accepted = state.globals.find("lcancelaccepted");
    const auto cancel_status = state.globals.find("ccancelstatus");
    const auto await_done = state.globals.find("lawaitdone");
    const auto cancel_unknown = state.globals.find("lcancelunknown");
    expect(cancel_accepted != state.globals.end() && cancel_accepted->second.boolean_value,
           "CFTASKCANCEL should accept a request for a running task");
    expect(cancel_status != state.globals.end() &&
               (cancel_status->second.string_value == "cancel-requested" ||
                cancel_status->second.string_value == "error"),
           "CFTASKSTATUS should deterministically expose cancellation progress or completion");
    expect(await_done != state.globals.end() && !await_done->second.boolean_value,
           "AWAIT should report a cooperatively cancelled task as incomplete");
    expect(cancel_unknown != state.globals.end() && !cancel_unknown->second.boolean_value,
           "CFTASKCANCEL should reject an erased task handle");
    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto &event) {
        return event.category == "runtime.task.cancel_requested" &&
               event.detail.find("handle=") != std::string::npos;
    }), "accepted CFTASKCANCEL should emit a traceable cancellation-request event");
    expect(std::none_of(state.events.begin(), state.events.end(), [](const auto &event) {
        return event.category == "runtime.critical.blocking_violation";
    }), "nonblocking CFTASK supervision should remain allowed inside a critical section");

    fs::remove_all(temp_root, ignored);
}

void test_spawn_arguments_use_heap_backed_frame_continuations() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_spawn_argument_continuation";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path depth_path = temp_root / "spawn_argument_depth_limit.prg";
    write_text(
        depth_path,
        "SPAWN worker WITH recurse_argument(1) TO nTask\n"
        "RETURN\n"
        "FUNCTION recurse_argument\n"
        "LPARAMETERS nDepth\n"
        "RETURN recurse_argument(nDepth + 1)\n"
        "PROCEDURE worker\n"
        "LPARAMETERS value\n"
        "RETURN\n");

    auto depth_options = make_runtime_session_options(depth_path.string(), temp_root.string(), false);
    depth_options.max_call_depth = 2048U;
    copperfin::runtime::PrgRuntimeSession depth_session =
        copperfin::runtime::PrgRuntimeSession::create(depth_options);
    const auto depth_state = depth_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(
        depth_state.reason == copperfin::runtime::DebugPauseReason::error,
        "deep SPAWN argument recursion should stop at the runtime guardrail");
    expect(
        depth_state.message.find("maximum call depth") != std::string::npos,
        "deep SPAWN argument recursion should report the configured call-depth diagnostic");

    const fs::path semantics_path = temp_root / "spawn_argument_semantics.prg";
    write_text(
        semantics_path,
        "SET UDFPARMS TO REFERENCE\n"
        "first = 2\n"
        "counter = 2\n"
        "second = 3\n"
        "calls = 0\n"
        "order = ''\n"
        "cTarget = resolve_target()\n"
        "SPAWN &cTarget WITH record_call(first), @counter, record_call(second) TO nTask\n"
        "AWAIT nTask TO lDone\n"
        "afterCalls = calls\n"
        "afterOrder = order\n"
        "RETURN\n"
        "FUNCTION record_call\n"
        "LPARAMETERS value\n"
        "calls = calls + 1\n"
        "IF value = 2\n"
        "order = order + 'A'\n"
        "ELSE\n"
        "order = order + 'B'\n"
        "ENDIF\n"
        "RETURN value + 10\n"
        "FUNCTION resolve_target\n"
        "RETURN 'worker'\n"
        "ENDFUNC\n"
        "PROCEDURE worker\n"
        "LPARAMETERS firstValue, forwarded, secondValue\n"
        "? firstValue + forwarded + secondValue\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession semantics_session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(semantics_path.string(), temp_root.string(), false));
    const auto semantics_state = semantics_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(semantics_state.completed, "SPAWN argument continuation semantics script should complete: " + semantics_state.message);

    const auto done = semantics_state.globals.find("ldone");
    const auto calls = semantics_state.globals.find("aftercalls");
    const auto order = semantics_state.globals.find("afterorder");
    expect(done != semantics_state.globals.end(), "SPAWN/AWAIT should expose the completion flag");
    expect(calls != semantics_state.globals.end(), "SPAWN argument UDF call count should be captured");
    expect(order != semantics_state.globals.end(), "SPAWN argument evaluation order should be captured");
    if (done != semantics_state.globals.end())
    {
        expect(done->second.boolean_value, "SPAWN/AWAIT should complete after resumed argument evaluation");
    }
    if (calls != semantics_state.globals.end())
    {
        expect(copperfin::runtime::format_value(calls->second) == "2", "SPAWN UDF arguments should each execute exactly once");
    }
    if (order != semantics_state.globals.end())
    {
        expect(copperfin::runtime::format_value(order->second) == "AB", "SPAWN UDF arguments should preserve left-to-right order");
    }
    expect(
        std::any_of(
            semantics_state.events.begin(),
            semantics_state.events.end(),
            [](const auto &event) {
                return event.category == "runtime.print" && event.detail == "27";
            }),
        "SPAWN should forward resumed argument values into the child routine");

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

void test_request_cancel_rolls_back_active_transaction_and_resets_txnlevel() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_request_cancel_txn";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}});

    const fs::path main_path = temp_root / "request_cancel_txn.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "BEGIN TRANSACTION\n"
        "APPEND BLANK\n"
        "REPLACE NAME WITH 'TEMP'\n"
        "REPLACE AGE WITH 99\n"
        "SLEEP 5000\n"
        "cShouldNotRun = 'yes'\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
    session.add_breakpoint({.file_path = main_path.string(), .line = 6U});
    const auto sleep_breakpoint = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(sleep_breakpoint.reason == copperfin::runtime::DebugPauseReason::breakpoint,
           "request_cancel transaction test should stop deterministically at SLEEP");
    expect(sleep_breakpoint.statement_text == "SLEEP 5000",
           "request_cancel transaction test should identify the pending SLEEP statement");

    session.request_cancel();
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!state.completed, "request_cancel during an active transaction should stop the run with an error pause");
    expect(state.reason == copperfin::runtime::DebugPauseReason::error,
           "request_cancel during an active transaction should return an error pause");
    expect(state.message == "SLEEP cancelled.",
           "request_cancel during SLEEP should preserve the existing localized sleep-cancel message");
    expect(state.location.file_path == main_path.string() && state.location.line == 6U,
           "request_cancel during SLEEP should preserve the exact SLEEP source location");
    expect(state.statement_text == "SLEEP 5000",
           "request_cancel during SLEEP should preserve the exact faulting statement text");

    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.task.cancelled";
    }), "request_cancel should emit runtime.task.cancelled");
    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.transaction.rollback" && event.detail == "0";
    }), "request_cancel inside an active transaction should emit runtime.transaction.rollback detail=0");

    const auto blocked_marker = state.globals.find("cshouldnotrun");
    expect(blocked_marker == state.globals.end(),
           "request_cancel should prevent statements after the cancelled SLEEP from executing");

    const auto txnlevel_watch = session.evaluate_watch_expression("TXNLEVEL()");
    expect(txnlevel_watch.ok, "request_cancel error pause should still allow TXNLEVEL() watch evaluation");
    if (txnlevel_watch.ok)
    {
        expect(copperfin::runtime::format_value(txnlevel_watch.value) == "0",
               "request_cancel should reset TXNLEVEL() to 0 after rolling back the active transaction");
    }

    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 2U);
    expect(parse_result.ok, "request_cancel should leave the DBF readable after rollback");
    expect(parse_result.table.records.size() == 2U,
           "request_cancel should remove the appended transactional row from disk");
    if (parse_result.ok && parse_result.table.records.size() == 2U)
    {
        expect(parse_result.table.records[0].values[0].display_value == "ALPHA",
               "request_cancel should preserve original text values after rollback");
        expect(parse_result.table.records[0].values[1].display_value == "10",
               "request_cancel should preserve original numeric values after rollback");
    }

    const fs::path transaction_root = temp_root / "runtime-temp" / "transactions";
    if (std::filesystem::exists(transaction_root, ignored))
    {
        bool found_pending_journal = false;
        for (const auto &entry : std::filesystem::directory_iterator(transaction_root, ignored))
        {
            if (!entry.is_directory(ignored))
            {
                continue;
            }
            const auto name = entry.path().filename().string();
            if (name.rfind("txn_", 0U) == 0U)
            {
                found_pending_journal = true;
                break;
            }
        }
        expect(!found_pending_journal,
               "request_cancel should clean up staged transaction journals after rollback");
    }

    const fs::path generic_path = temp_root / "request_cancel_generic.prg";
    write_text(generic_path, "nShouldNotRun = 1\nRETURN\n");
    copperfin::runtime::PrgRuntimeSession generic_session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(generic_path.string(), temp_root.string(), false));
    generic_session.request_cancel();
    const auto generic_state = generic_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(generic_state.reason == copperfin::runtime::DebugPauseReason::error,
           "request_cancel before a non-SLEEP statement should retain the generic error pause");
    expect(generic_state.message == "Async task cancelled.",
           "request_cancel before a non-SLEEP statement should retain the generic localized message");
    expect(generic_state.globals.find("nshouldnotrun") == generic_state.globals.end(),
           "generic request_cancel should still prevent the pending non-SLEEP statement from executing");

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
