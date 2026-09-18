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

void test_await_failed_output_assignment_does_not_replay_events_or_leak_task() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_await_failed_assignment";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "await_failed_assignment.prg";
    write_text(
        main_path,
        "PUBLIC error_seen\n"
        "error_seen = .F.\n"
        "ON ERROR DO caught\n"
        "SPAWN worker TO nTask\n"
        "AWAIT nTask TO missing.property\n"
        "lStillRegisteredAfterFailure = (CFTASKSTATUS(nTask) == 'completed')\n"
        "AWAIT nTask TO lSecond\n"
        "cStatusAfterSecond = CFTASKSTATUS(nTask)\n"
        "RETURN\n"
        "PROCEDURE worker\n"
        "? 'unique_child_output_marker'\n"
        "RETURN .T.\n"
        "ENDPROC\n"
        "PROCEDURE caught\n"
        "error_seen = .T.\n"
        "RETURN\n"
        "ENDPROC\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "#6456: failed-AWAIT-assignment test should complete: " + state.message);

    const auto error_seen_it = state.globals.find("error_seen");
    expect(error_seen_it != state.globals.end() && error_seen_it->second.boolean_value,
           "#6456: AWAIT nTask TO missing.property should raise a catchable error the ON ERROR handler observes");

    const auto still_registered_it = state.globals.find("lstillregisteredafterfailure");
    expect(still_registered_it != state.globals.end() && still_registered_it->second.boolean_value,
           "#6456: the task must remain registered (not erased) after a failed output assignment, so a "
           "later retry AWAIT can still reach it");

    const auto second_it = state.globals.find("lsecond");
    expect(second_it != state.globals.end() && second_it->second.boolean_value,
           "#6456: a retry AWAIT with a valid target should successfully consume the still-registered task");

    const auto status_after_second_it = state.globals.find("cstatusaftersecond");
    expect(status_after_second_it != state.globals.end() &&
               status_after_second_it->second.string_value == "unknown",
           "#6456: the successful retry AWAIT should finally erase the task, exactly once");

    const auto marker_count = std::count_if(
        state.events.begin(), state.events.end(),
        [](const auto &event) {
            return event.category == "runtime.print" && event.detail == "unique_child_output_marker";
        });
    expect(marker_count == 1,
           "#6456: the child's event stream must be merged into the parent exactly once, not replayed on the "
           "retry AWAIT -- got " + std::to_string(marker_count) + " occurrence(s)");

    fs::remove_all(temp_root, ignored);
}

void test_spawn_read_events_is_rejected_catchably() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_spawn_read_events_rejected";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "spawn_read_events_rejected.prg";
    write_text(
        main_path,
        "SPAWN event_worker TO nTask\n"
        "AWAIT nTask TO lJoined\n"
        "cStatusAfter = CFTASKSTATUS(nTask)\n"
        "RETURN\n"
        "PROCEDURE event_worker\n"
        "? 'before_read_events'\n"
        "READ EVENTS\n"
        "? 'after_read_events'\n"
        "RETURN .T.\n"
        "ENDPROC\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "#6457: spawned READ EVENTS rejection test should complete: " + state.message);

    const auto joined_it = state.globals.find("ljoined");
    expect(joined_it != state.globals.end() && !joined_it->second.boolean_value,
           "#6457: a spawned task that hits READ EVENTS must not report completed");

    const auto status_after_it = state.globals.find("cstatusafter");
    expect(status_after_it != state.globals.end() && status_after_it->second.string_value == "unknown",
           "#6457: AWAIT should still consume (erase) the rejected task exactly like any other faulted task");

    const auto before_marker = std::find_if(
        state.events.begin(), state.events.end(),
        [](const auto &event) { return event.category == "runtime.print" && event.detail == "before_read_events"; });
    expect(before_marker != state.events.end(),
           "#6457: the worker should run up to the READ EVENTS statement");

    const auto after_marker = std::find_if(
        state.events.begin(), state.events.end(),
        [](const auto &event) { return event.category == "runtime.print" && event.detail == "after_read_events"; });
    expect(after_marker == state.events.end(),
           "#6457: READ EVENTS in a spawned task must fault immediately rather than silently pausing or "
           "letting execution continue past it");

    const auto await_event = std::find_if(
        state.events.begin(), state.events.end(),
        [](const auto &event) { return event.category == "runtime.task.await"; });
    expect(await_event != state.events.end() && await_event->detail.find("state=event_loop") == std::string::npos,
           "#6457: the task must not be classified with the nonterminal 'event_loop' pause reason -- READ "
           "EVENTS in a spawned task must genuinely fault, not silently pause forever, got '" +
               (await_event != state.events.end() ? await_event->detail : "<missing>") + "'");
    expect(await_event != state.events.end() &&
               await_event->detail.find("not supported in a spawned task") != std::string::npos,
           "#6457: the AWAIT event should report the specific READ EVENTS rejection reason, got '" +
               (await_event != state.events.end() ? await_event->detail : "<missing>") + "'");

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
        "first = 2\n"
        "counter = 2\n"
        "second = 3\n"
        "calls = 0\n"
        "order = ''\n"
        "cTarget = resolve_target()\n"
        "SPAWN &cTarget WITH record_call(first), counter, record_call(second) TO nTask\n"
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

void test_spawn_rejects_by_reference_argument() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_spawn_byref_rejected";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "spawn_byref_rejected.prg";
    write_text(
        main_path,
        "counter = 1\n"
        "lRejected = .F.\n"
        "TRY\n"
        "    SPAWN worker WITH @counter TO task_handle\n"
        "CATCH TO oErr\n"
        "    lRejected = .T.\n"
        "    cErrorMessage = oErr.Message\n"
        "ENDTRY\n"
        "counterAfter = counter\n"
        "RETURN\n"
        "PROCEDURE worker\n"
        "LPARAMETERS value\n"
        "value = 99\n"
        "RETURN\n"
        "ENDPROC\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "#6447: SPAWN by-reference rejection test should complete: " + state.message);

    const auto rejected_it = state.globals.find("lrejected");
    expect(rejected_it != state.globals.end() && rejected_it->second.boolean_value,
           "#6447: SPAWN WITH @counter must raise a catchable error instead of silently accepting a "
           "by-reference argument it cannot honor");

    const auto error_message_it = state.globals.find("cerrormessage");
    expect(error_message_it != state.globals.end() &&
               error_message_it->second.string_value.find("counter") != std::string::npos,
           "#6447: the rejection error should name the offending variable, got '" +
               (error_message_it != state.globals.end() ? error_message_it->second.string_value : "<missing>") +
               "'");

    const auto counter_after_it = state.globals.find("counterafter");
    expect(counter_after_it != state.globals.end() && counter_after_it->second.number_value == 1.0,
           "#6447: counter must be untouched -- the task must never have started at all, not started and then "
           "silently lost its writeback");

    const auto spawn_event = std::find_if(
        state.events.begin(), state.events.end(),
        [](const auto &event) { return event.category == "runtime.task.spawn"; });
    expect(spawn_event == state.events.end(),
           "#6447: rejecting the by-reference argument must happen before the task is started -- no "
           "runtime.task.spawn event should be emitted");

    const auto task_handle_it = state.globals.find("task_handle");
    expect(task_handle_it == state.globals.end(),
           "#6447: the TO target should never be assigned a task handle when SPAWN itself is rejected");

    fs::remove_all(temp_root, ignored);
}

void test_spawn_failed_handle_assignment_does_not_leave_orphan_task() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_spawn_failed_handle_assignment";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path marker_path = temp_root / "worker_ran.txt";
    const fs::path main_path = temp_root / "spawn_failed_handle_assignment.prg";
    write_text(
        main_path,
        "PUBLIC error_seen, after_spawn\n"
        "error_seen = .F.\n"
        "after_spawn = .F.\n"
        "ON ERROR DO handleerr\n"
        "SPAWN worker TO missing.prop\n"
        "after_spawn = .T.\n"
        "SLEEP 200\n"
        "RETURN\n"
        "PROCEDURE handleerr\n"
        "error_seen = .T.\n"
        "RETURN\n"
        "ENDPROC\n"
        "PROCEDURE worker\n"
        "STRTOFILE('ran', '" + marker_path.string() + "')\n"
        "RETURN\n"
        "ENDPROC\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "#6450: failed-handle-assignment test should complete: " + state.message);

    const auto error_seen_it = state.globals.find("error_seen");
    expect(error_seen_it != state.globals.end() && error_seen_it->second.boolean_value,
           "#6450: SPAWN worker TO missing.prop should raise a catchable error the ON ERROR handler observes");

    const auto after_spawn_it = state.globals.find("after_spawn");
    expect(after_spawn_it != state.globals.end() && after_spawn_it->second.boolean_value,
           "#6450: execution should resume normally after the handled SPAWN target-assignment failure");

    const auto spawn_event = std::find_if(
        state.events.begin(), state.events.end(),
        [](const auto &event) { return event.category == "runtime.task.spawn"; });
    expect(spawn_event == state.events.end(),
           "#6450: a failed target assignment must mean the task was never started at all -- no "
           "runtime.task.spawn event should be emitted");

    expect(!std::filesystem::exists(marker_path, ignored),
           "#6450: the worker must never have started running -- a task whose handle assignment failed must "
           "be rolled back before the child is created, not started and then left running with no reachable "
           "handle (the original bug: the task ran to completion despite the assignment failure)");

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

void test_spawn_quit_releases_only_its_own_locks_not_the_parents() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_spawn_quit_lock_isolation";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "spawn_quit_lock_isolation.prg";
    write_text(
        main_path,
        "CREATE TABLE race (name C(12))\n"
        "SELECT race\n"
        "APPEND BLANK\n"
        "REPLACE name WITH 'ORIGINAL1'\n"
        "lParentLock = FLOCK()\n"
        "SPAWN quitter TO nQuitter\n"
        "AWAIT nQuitter TO lQuitterDone\n"
        "lParentStillLocked = ISFLOCKED()\n"
        "SPAWN intruder TO nIntruder\n"
        "DO WHILE CFTASKSTATUS(nIntruder) == 'running'\n"
        "ENDDO\n"
        "lIntruderAcquiredLock = CFTASKRESULT(nIntruder)\n"
        "AWAIT nIntruder TO lIntruderTaskCompleted\n"
        "RETURN\n"
        "PROCEDURE quitter\n"
        "QUIT\n"
        "ENDPROC\n"
        "PROCEDURE intruder\n"
        "SELECT race\n"
        "RETURN FLOCK()\n"
        "ENDPROC\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "#6453: lock-isolation test should complete: " + state.message);

    const auto parent_lock_it = state.globals.find("lparentlock");
    expect(parent_lock_it != state.globals.end() && parent_lock_it->second.boolean_value,
           "#6453: the parent should successfully acquire FLOCK() before spawning");

    const auto quitter_done_it = state.globals.find("lquitterdone");
    expect(quitter_done_it != state.globals.end() && quitter_done_it->second.boolean_value,
           "#6453: the spawned QUIT task should be observed as completed");

    const auto still_locked_it = state.globals.find("lparentstilllocked");
    expect(still_locked_it != state.globals.end() && still_locked_it->second.boolean_value,
           "#6453: the parent's FLOCK() must survive a sibling spawned task's QUIT shutdown -- a spawned "
           "child's shutdown must release only its own locks, not clear the whole shared lock-owner map");

    const auto intruder_it = state.globals.find("lintruderacquiredlock");
    expect(intruder_it != state.globals.end() && !intruder_it->second.boolean_value,
           "#6453: a second spawned task must not be able to acquire FLOCK() on a table the parent still "
           "holds locked (a sibling task's QUIT must not release the parent's lock), got '" +
               (intruder_it != state.globals.end() ? copperfin::runtime::format_value(intruder_it->second) : "<missing>") +
               "'");

    const auto intruder_timeout_event = std::find_if(
        state.events.begin(), state.events.end(),
        [](const auto &event) { return event.category == "runtime.lock_timeout"; });
    expect(intruder_timeout_event != state.events.end(),
           "#6453: the second spawned task's FLOCK() attempt should exhaust its reprocess retry budget and "
           "time out (proving it genuinely contended with the still-held parent lock, not merely returning "
           "false for an unrelated reason)");

    fs::remove_all(temp_root, ignored);
}

void test_spawn_natural_completion_releases_its_own_locks() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_spawn_natural_completion_lock_release";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "spawn_natural_completion_lock_release.prg";
    write_text(
        main_path,
        "CREATE TABLE race (name C(12))\n"
        "SELECT race\n"
        "APPEND BLANK\n"
        "REPLACE name WITH 'ORIGINAL1'\n"
        "SPAWN worker TO nWorker\n"
        "DO WHILE CFTASKSTATUS(nWorker) == 'running'\n"
        "ENDDO\n"
        "lWorkerAcquiredLock = CFTASKRESULT(nWorker)\n"
        "AWAIT nWorker TO lWorkerDone\n"
        "lParentAcquiredLockAfter = FLOCK()\n"
        "RETURN\n"
        "PROCEDURE worker\n"
        "SELECT race\n"
        "RETURN FLOCK()\n"
        "ENDPROC\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "#6453: natural-completion lock-release test should complete: " + state.message);

    const auto worker_done_it = state.globals.find("lworkerdone");
    expect(worker_done_it != state.globals.end() && worker_done_it->second.boolean_value,
           "#6453: the spawned worker task should be observed as completed");

    const auto worker_lock_it = state.globals.find("lworkeracquiredlock");
    expect(worker_lock_it != state.globals.end() && worker_lock_it->second.boolean_value,
           "#6453: the spawned worker should successfully FLOCK() the table before returning naturally");

    const auto parent_lock_after_it = state.globals.find("lparentacquiredlockafter");
    expect(parent_lock_after_it != state.globals.end() && parent_lock_after_it->second.boolean_value,
           "#6453: a spawned task that returns naturally (no QUIT) must release its own locks on completion "
           "just as an explicit QUIT does -- otherwise the parent can never re-acquire the lock the worker "
           "held, because the worker's owner-scoped lock entries would never be cleared");

    const auto lock_timeout_event = std::find_if(
        state.events.begin(), state.events.end(),
        [](const auto &event) { return event.category == "runtime.lock_timeout"; });
    expect(lock_timeout_event == state.events.end(),
           "#6453: the parent's post-completion FLOCK() should succeed immediately, not contend and time out "
           "against a lock the worker never released");

    fs::remove_all(temp_root, ignored);
}

void test_spawn_transaction_rollback_does_not_erase_committed_sibling_write() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_spawn_transaction_rollback_conflict";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "race.dbf";

    const fs::path main_path = temp_root / "spawn_transaction_rollback_conflict.prg";
    write_text(
        main_path,
        "CREATE TABLE race (name C(12))\n"
        "SELECT race\n"
        "APPEND BLANK\n"
        "REPLACE name WITH 'ORIGINAL1'\n"
        "APPEND BLANK\n"
        "REPLACE name WITH 'ORIGINAL2'\n"
        "BEGIN TRANSACTION\n"
        "GO 1\n"
        "REPLACE name WITH 'PARENT'\n"
        "SPAWN worker TO nTask\n"
        "AWAIT nTask TO lWorkerCompleted\n"
        "ROLLBACK\n"
        "nTxnLevelAfterRollback = TXNLEVEL()\n"
        "GO 1\n"
        "cRecord1After = ALLTRIM(name)\n"
        "GO 2\n"
        "cRecord2After = ALLTRIM(name)\n"
        "RETURN\n"
        "PROCEDURE worker\n"
        "SELECT race\n"
        "GO 2\n"
        "REPLACE name WITH 'CHILD'\n"
        "RETURN\n"
        "ENDPROC\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "#6451: rollback-conflict test should complete: " + state.message);

    // The spawned worker has no transaction of its own, but REPLACE always
    // takes an implicit record lock for its physical write regardless of
    // transaction state -- so it must contend with (and lose to) the
    // parent's transaction-held exclusive lock on the same table, exactly
    // like any other genuine FLOCK()/RLOCK() conflict.
    const auto worker_completed_it = state.globals.find("lworkercompleted");
    expect(worker_completed_it != state.globals.end() && !worker_completed_it->second.boolean_value,
           "#6451: the spawned worker's REPLACE should contend with the parent's open transaction and fail, "
           "not silently succeed against a table the parent transaction has claimed");

    const auto lock_timeout_event = std::find_if(
        state.events.begin(), state.events.end(),
        [](const auto &event) { return event.category == "runtime.lock_timeout"; });
    expect(lock_timeout_event != state.events.end(),
           "#6451: the worker's REPLACE should genuinely contend with the parent's transaction-held lock and "
           "time out via the reprocess retry budget, not merely fail for an unrelated reason");

    const auto txnlevel_it = state.globals.find("ntxnlevelafterrollback");
    expect(txnlevel_it != state.globals.end() && txnlevel_it->second.number_value == 0.0,
           "#6451: with the sibling write correctly serialized out, ROLLBACK should succeed normally and "
           "reset TXNLEVEL() to 0");

    const auto record1_it = state.globals.find("crecord1after");
    expect(record1_it != state.globals.end() && record1_it->second.string_value == "ORIGINAL1",
           "#6451: record 1 should be rolled back from 'PARENT' to 'ORIGINAL1'");

    const auto record2_it = state.globals.find("crecord2after");
    expect(record2_it != state.globals.end() && record2_it->second.string_value == "ORIGINAL2",
           "#6451: record 2 should remain 'ORIGINAL2' -- the worker's 'CHILD' write must never have landed, "
           "got '" + (record2_it != state.globals.end() ? record2_it->second.string_value : "<missing>") + "'");

    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 2U);
    expect(parse_result.ok, "#6451: race.dbf should remain readable after the rollback");
    if (parse_result.ok && parse_result.table.records.size() == 2U)
    {
        expect(parse_result.table.records[0].values[0].display_value == "ORIGINAL1",
               "#6451: on-disk record 1 should be rolled back to 'ORIGINAL1', got '" +
                   parse_result.table.records[0].values[0].display_value + "'");
        expect(parse_result.table.records[1].values[0].display_value == "ORIGINAL2",
               "#6451: on-disk record 2 should remain 'ORIGINAL2' -- the worker's write was correctly "
               "serialized out by the parent's open transaction, not silently allowed through and then "
               "erased, got '" + parse_result.table.records[1].values[0].display_value + "'");
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


}  // namespace cf_test_prg_engine_control_flow

