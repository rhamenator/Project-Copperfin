// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "test_prg_engine_control_flow_support.h"

namespace cf_test_prg_engine_control_flow {
void test_runtime_guardrail_limits_call_depth_without_crashing_host() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_guard_call_depth";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "deep_calls.prg";
    write_text(
        main_path,
        "DO a\n"
        "RETURN\n"
        "PROCEDURE a\n"
        "DO b\n"
        "RETURN\n"
        "PROCEDURE b\n"
        "DO c\n"
        "RETURN\n"
        "PROCEDURE c\n"
        "DO d\n"
        "RETURN\n"
        "PROCEDURE d\n"
        "RETURN\n");

    auto session_options = make_runtime_session_options(main_path.string(), temp_root.string(), false);
    session_options.max_call_depth = 3;
    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(session_options);

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error, "call-depth guardrail should pause with an error");
    expect(
        state.message.find("maximum call depth") != std::string::npos,
        "call-depth guardrail should report a call-depth limit message");

    fs::remove_all(temp_root, ignored);
}

void test_runtime_guardrail_exactly_at_call_depth_limit_succeeds() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_guard_call_depth_exact_limit";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    constexpr std::size_t limit = 5U;
    const fs::path main_path = temp_root / "exact_limit.prg";
    write_text(main_path, build_nested_do_chain_script(limit));

    auto session_options = make_runtime_session_options(main_path.string(), temp_root.string(), false);
    session_options.max_call_depth = limit;
    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(session_options);

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "exactly-at-limit nested call chain should complete");
    expect(state.reason != copperfin::runtime::DebugPauseReason::error,
           "exactly-at-limit nested call chain should not dispatch guardrail errors");

    fs::remove_all(temp_root, ignored);
}

void test_runtime_guardrail_one_over_call_depth_limit_fails() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_guard_call_depth_one_over";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    constexpr std::size_t limit = 5U;
    const fs::path main_path = temp_root / "one_over_limit.prg";
    write_text(main_path, build_nested_do_chain_script(limit + 1U));

    auto session_options = make_runtime_session_options(main_path.string(), temp_root.string(), false);
    session_options.max_call_depth = limit;
    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(session_options);

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error,
           "one-over-limit nested call chain should dispatch a guardrail error");
    expect(state.message.find("maximum call depth") != std::string::npos,
           "one-over-limit nested call chain should report call-depth limit details");

    fs::remove_all(temp_root, ignored);
}

void test_runtime_guardrail_limits_statement_budget_without_crashing_host() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_guard_statement_budget";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "endless_loop.prg";
    write_text(
        main_path,
        "DO WHILE .T.\n"
        "x = 1\n"
        "ENDDO\n");

    auto session_options = make_runtime_session_options(main_path.string(), temp_root.string(), false);
    session_options.max_executed_statements = 30;
    session_options.max_loop_iterations = 1000;
    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(session_options);

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error, "statement-budget guardrail should pause with an error");
    expect(
        state.message.find("maximum executed statements") != std::string::npos,
        "statement-budget guardrail should report a statement-budget limit message");

    fs::remove_all(temp_root, ignored);
}

void test_runtime_guardrail_limits_loop_iterations_without_crashing_host() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_guard_loop_iterations";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "loop_limit.prg";
    write_text(
        main_path,
        "nCounter = 0\n"
        "DO WHILE .T.\n"
        "nCounter = nCounter + 1\n"
        "ENDDO\n");

    auto session_options = make_runtime_session_options(main_path.string(), temp_root.string(), false);
    session_options.max_executed_statements = 1000;
    session_options.max_loop_iterations = 5;
    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(session_options);

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error, "loop-iteration guardrail should pause with an error");
    expect(
        state.message.find("maximum loop iterations") != std::string::npos,
        "loop-iteration guardrail should report a loop-iteration limit message");

    fs::remove_all(temp_root, ignored);
}

void test_runtime_guardrail_errors_localize_without_changing_behavior() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_guardrail_localization";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue scoped_locale("COPPERFIN_LOCALE");
    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);

    const fs::path call_depth_path = temp_root / "call_depth.prg";
    write_text(
        call_depth_path,
        "DO a\n"
        "RETURN\n"
        "PROCEDURE a\n"
        "DO b\n"
        "RETURN\n"
        "PROCEDURE b\n"
        "DO c\n"
        "RETURN\n"
        "PROCEDURE c\n"
        "RETURN\n");

    auto call_depth_options = make_runtime_session_options(call_depth_path.string(), temp_root.string(), false);
    call_depth_options.max_call_depth = 2;
    copperfin::runtime::PrgRuntimeSession call_depth_session =
        copperfin::runtime::PrgRuntimeSession::create(call_depth_options);
    const auto call_depth_state = call_depth_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(call_depth_state.reason == copperfin::runtime::DebugPauseReason::error,
           "#2720: qps-ploc call-depth guardrail should still pause with an error");
    expect(
        call_depth_state.message.find("[!! ") == 0U &&
            call_depth_state.message.find("2") != std::string::npos &&
            call_depth_state.message.find("maximum call depth") == std::string::npos,
        "#2720: qps-ploc call-depth guardrail should pseudo-localize prose while preserving the numeric limit");

    const fs::path statement_budget_path = temp_root / "statement_budget.prg";
    write_text(
        statement_budget_path,
        "DO WHILE .T.\n"
        "x = 1\n"
        "ENDDO\n");

    auto statement_budget_options =
        make_runtime_session_options(statement_budget_path.string(), temp_root.string(), false);
    statement_budget_options.max_executed_statements = 12;
    statement_budget_options.max_loop_iterations = 1000;
    copperfin::runtime::PrgRuntimeSession statement_budget_session =
        copperfin::runtime::PrgRuntimeSession::create(statement_budget_options);
    const auto statement_budget_state = statement_budget_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(statement_budget_state.reason == copperfin::runtime::DebugPauseReason::error,
           "#2720: qps-ploc executed-statements guardrail should still pause with an error");
    expect(
        statement_budget_state.message.find("[!! ") == 0U &&
            statement_budget_state.message.find("12") != std::string::npos &&
            statement_budget_state.message.find("maximum executed statements") == std::string::npos,
        "#2720: qps-ploc executed-statements guardrail should pseudo-localize prose while preserving the numeric limit");

    const fs::path loop_iterations_path = temp_root / "loop_iterations.prg";
    write_text(
        loop_iterations_path,
        "nCounter = 0\n"
        "DO WHILE .T.\n"
        "nCounter = nCounter + 1\n"
        "ENDDO\n");

    auto loop_iterations_options =
        make_runtime_session_options(loop_iterations_path.string(), temp_root.string(), false);
    loop_iterations_options.max_executed_statements = 1000;
    loop_iterations_options.max_loop_iterations = 4;
    copperfin::runtime::PrgRuntimeSession loop_iterations_session =
        copperfin::runtime::PrgRuntimeSession::create(loop_iterations_options);
    const auto loop_iterations_state = loop_iterations_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(loop_iterations_state.reason == copperfin::runtime::DebugPauseReason::error,
           "#2720: qps-ploc loop-iterations guardrail should still pause with an error");
    expect(
        loop_iterations_state.message.find("[!! ") == 0U &&
            loop_iterations_state.message.find("4") != std::string::npos &&
            loop_iterations_state.message.find("maximum loop iterations") == std::string::npos,
        "#2720: qps-ploc loop-iterations guardrail should pseudo-localize prose while preserving the numeric limit");

    fs::remove_all(temp_root, ignored);
}

void test_config_fpw_overrides_runtime_limits() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_config_limits";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_text(
        temp_root / "config.fpw",
        "MAX_CALL_DEPTH = 3\n"
        "MAX_EXECUTED_STATEMENTS = 40\n"
        "MAX_LOOP_ITERATIONS = 500\n");

    const fs::path main_path = temp_root / "deep_calls.prg";
    write_text(
        main_path,
        "DO a\n"
        "RETURN\n"
        "PROCEDURE a\n"
        "DO b\n"
        "RETURN\n"
        "PROCEDURE b\n"
        "DO c\n"
        "RETURN\n"
        "PROCEDURE c\n"
        "DO d\n"
        "RETURN\n"
        "PROCEDURE d\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error, "config.fpw call-depth limit should pause with an error");
    expect(
        state.message.find("maximum call depth") != std::string::npos,
        "config.fpw should control max call depth when options use defaults");

    fs::remove_all(temp_root, ignored);
}

void test_config_fpw_rejects_grouped_integer_tokens() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_config_grouped_integer";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_text(temp_root / "config.fpw", "MAX_CALL_DEPTH = 1.234\n");
    const fs::path main_path = temp_root / "grouped_config.prg";
    write_text(
        main_path,
        "DO a\n"
        "RETURN\n"
        "PROCEDURE a\n"
        "DO b\n"
        "RETURN\n"
        "PROCEDURE b\n"
        "RETURN\n");

    const auto state = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string(), false))
        .run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed,
           "grouped CONFIG.FPW integer tokens should be rejected instead of truncating to a smaller limit");

    fs::remove_all(temp_root, ignored);
}

void test_config_fpw_custom_limit_is_enforced_at_boundary() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_config_custom_call_depth_boundary";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    constexpr std::size_t custom_limit = 10U;
    write_text(
        temp_root / "config.fpw",
        "MAX_CALL_DEPTH = " + std::to_string(custom_limit) + "\n");

    const fs::path at_limit_path = temp_root / "at_custom_limit.prg";
    const fs::path over_limit_path = temp_root / "over_custom_limit.prg";
    write_text(at_limit_path, build_nested_do_chain_script(custom_limit));
    write_text(over_limit_path, build_nested_do_chain_script(custom_limit + 1U));

    copperfin::runtime::PrgRuntimeSession at_limit_session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(at_limit_path.string(), temp_root.string(), false));
    const auto at_limit_state = at_limit_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(at_limit_state.completed,
           "config.fpw custom max call depth should allow nested chains exactly at the configured limit");

    copperfin::runtime::PrgRuntimeSession over_limit_session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(over_limit_path.string(), temp_root.string(), false));
    const auto over_limit_state = over_limit_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(over_limit_state.reason == copperfin::runtime::DebugPauseReason::error,
           "config.fpw custom max call depth should reject nested chains one level over the configured limit");
    expect(over_limit_state.message.find("maximum call depth") != std::string::npos,
           "custom-limit guardrail errors should report the call-depth guardrail message");

    fs::remove_all(temp_root, ignored);
}

void test_config_fpw_overrides_temp_directory_default() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_config_temp_dir";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path local_temp = temp_root / "user_local_temp";
    fs::create_directories(local_temp);

    write_text(
        temp_root / "config.fpw",
        "TMPFILES = '" + local_temp.string() + "'\n");

    const fs::path main_path = temp_root / "main.prg";
    write_text(main_path, "x = 1\nRETURN\n");

    // Keep this inline instead of using make_runtime_session_options(...):
    // the helper always sets temp_directory, which would bypass the config.fpw
    // TMPFILES fallback path resolution this test is verifying.
    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "config temp-directory script should complete");

    const bool reported_config_temp = std::any_of(
        state.events.begin(),
        state.events.end(),
        [&](const copperfin::runtime::RuntimeEvent& event) {
            return event.category == "runtime.config" && event.detail.find(local_temp.string()) != std::string::npos;
        });
    expect(reported_config_temp, "runtime config event should include TMPFILES override path");

    fs::remove_all(temp_root, ignored);
}

void test_close_command_closes_all_work_areas() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_close_cmd";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_simple_dbf(temp_root / "a.dbf", {"alpha", "beta"});
    write_simple_dbf(temp_root / "b.dbf", {"gamma", "delta"});

    const fs::path main_path = temp_root / "close_test.prg";
    write_text(
        main_path,
        "USE '" + (temp_root / "a.dbf").string() + "'\n"
        "SELECT 2\n"
        "USE '" + (temp_root / "b.dbf").string() + "'\n"
        "CLOSE ALL\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "CLOSE ALL script should complete");

    expect(has_runtime_event(state.events, "runtime.close", "ALL"),
        "CLOSE ALL should emit a runtime.close event");

    fs::remove_all(temp_root, ignored);
}

void test_close_all_releases_runtime_handles() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_close_handles";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_text(temp_root / "held.txt", "seed");

    const fs::path main_path = temp_root / "close_handles.prg";
    write_text(
        main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "obj = CREATEOBJECT('Sample.Object')\n"
        "nHandle = FOPEN('held.txt', 2)\n"
        "CLOSE ALL\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "CLOSE ALL handle cleanup script should complete");
    expect(state.sql_connections.empty(), "CLOSE ALL should disconnect SQL handles");
    expect(state.ole_objects.empty(), "CLOSE ALL should release OLE object handles");

    const auto handle_it = state.globals.find("nhandle");
    expect(handle_it != state.globals.end(), "FOPEN handle should be captured before CLOSE ALL");
    int handle = 1;
    if (handle_it != state.globals.end()) {
        handle = static_cast<int>(handle_it->second.number_value);
    }

    const fs::path verify_path = temp_root / "verify_close_handles.prg";
    write_text(
        verify_path,
        "nClose = FCLOSE(" + std::to_string(handle) + ")\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession verify_session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(verify_path.string(), temp_root.string(), false));
    const auto verify_state = verify_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(verify_state.completed, "verification script should complete");

    const auto close_it = verify_state.globals.find("nclose");
    expect(close_it != verify_state.globals.end(), "verification script should expose FCLOSE result");
    if (close_it != verify_state.globals.end()) {
        expect(close_it->second.number_value == -1.0,
               "CLOSE ALL should already close FOPEN handles so follow-up FCLOSE returns -1");
    }

    fs::remove_all(temp_root, ignored);
}

void test_cancel_halts_execution() {
    namespace fs = std::filesystem;
    const fs::path tmp = fs::temp_directory_path() / "copperfin_cancel";
    std::error_code ign;
    fs::remove_all(tmp, ign);
    fs::create_directories(tmp);
    const fs::path prg = tmp / "test.prg";
    write_text(prg, "x = 1\nCANCEL\nx = 999\nRETURN\n");
    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(prg.string(), tmp.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "CANCEL should terminate cleanly");
    const auto it = state.globals.find("x");
    if (it != state.globals.end()) {
        expect(it->second.number_value == 1.0, "CANCEL should prevent execution of statements after it");
    }
    const bool has_cancel = std::any_of(state.events.begin(), state.events.end(),
        [](const auto &e) { return e.category == "runtime.cancel"; });
    expect(has_cancel, "CANCEL should emit runtime.cancel event");
    fs::remove_all(tmp, ign);
}

void test_quit_emits_event() {
    namespace fs = std::filesystem;
    const fs::path tmp = fs::temp_directory_path() / "copperfin_quit";
    std::error_code ign;
    fs::remove_all(tmp, ign);
    fs::create_directories(tmp);
    const fs::path prg = tmp / "test.prg";
    write_text(prg, "y = 5\nQUIT\ny = 999\nRETURN\n");
    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(prg.string(), tmp.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "QUIT should terminate cleanly");
    const bool has_quit = std::any_of(state.events.begin(), state.events.end(),
        [](const auto &e) { return e.category == "runtime.quit"; });
    expect(has_quit, "QUIT should emit runtime.quit event");
    fs::remove_all(tmp, ign);
}

void test_quit_cancelled_by_callback() {
    // When quit_confirm_callback returns false, QUIT should be cancelled:
    // execution continues after the QUIT statement and y should reach 999.
    namespace fs = std::filesystem;
    const fs::path tmp = fs::temp_directory_path() / "copperfin_quit_cancel";
    std::error_code ign;
    fs::remove_all(tmp, ign);
    fs::create_directories(tmp);
    const fs::path prg = tmp / "test.prg";
    write_text(prg, "y = 5\nQUIT\ny = 999\nRETURN\n");
    auto opts = make_runtime_session_options(prg.string(), tmp.string(), false);
    opts.quit_confirm_callback = []() -> bool { return false; };  // user said no
    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(opts);
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "Execution should complete normally after quit was cancelled");
    // y should have reached 999 — the line after QUIT was executed
    const auto it = state.globals.find("y");
    expect(it != state.globals.end(), "Variable y should exist");
    if (it != state.globals.end()) {
        expect(it->second.number_value == 999.0, "y should be 999 after QUIT was cancelled");
    }
    const bool has_cancelled = std::any_of(state.events.begin(), state.events.end(),
        [](const auto &e) { return e.category == "runtime.quit_cancelled"; });
    expect(has_cancelled, "QUIT cancelled should emit runtime.quit_cancelled event");
    const bool has_quit = std::any_of(state.events.begin(), state.events.end(),
        [](const auto &e) { return e.category == "runtime.quit"; });
    expect(!has_quit, "runtime.quit event should NOT be emitted when QUIT is cancelled");
    fs::remove_all(tmp, ign);
}

void test_shutdown_handler_quit_exits_event_loop_without_clear_events() {
    namespace fs = std::filesystem;
    const fs::path tmp = fs::temp_directory_path() / "copperfin_shutdown_quit_without_clear";
    std::error_code ign;
    fs::remove_all(tmp, ign);
    fs::create_directories(tmp);

    const fs::path prg = tmp / "test.prg";
    write_text(prg,
               "READ EVENTS\n"
               "RETURN\n"
               "PROCEDURE AppShutdown\n"
               "QUIT\n"
               "ENDPROC\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(prg.string(), tmp.string(), false));

    const auto paused = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(paused.reason == copperfin::runtime::DebugPauseReason::event_loop,
           "READ EVENTS should place runtime into event-loop pause");

    const bool dispatched = session.dispatch_event_handler("AppShutdown");
    expect(dispatched, "shutdown event handler should dispatch while in READ EVENTS");

    const auto completed = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(completed.completed, "QUIT inside shutdown handler should complete runtime without CLEAR EVENTS");
    expect(completed.reason == copperfin::runtime::DebugPauseReason::completed,
           "runtime should report completed after shutdown QUIT");

    const bool has_quit = std::any_of(completed.events.begin(), completed.events.end(),
        [](const auto &e) { return e.category == "runtime.quit"; });
    expect(has_quit, "shutdown QUIT should emit runtime.quit event");

    fs::remove_all(tmp, ign);
}

void test_shutdown_handler_cleanup_code_remains_harmless() {
    namespace fs = std::filesystem;
    const fs::path tmp = fs::temp_directory_path() / "copperfin_shutdown_quit_with_clear";
    std::error_code ign;
    fs::remove_all(tmp, ign);
    fs::create_directories(tmp);

    const fs::path prg = tmp / "test.prg";
    write_text(prg,
               "READ EVENTS\n"
               "RETURN\n"
               "PROCEDURE AppShutdown\n"
               "CLEAR EVENTS\n"
               "QUIT\n"
               "ENDPROC\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(prg.string(), tmp.string(), false));

    const auto paused = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(paused.reason == copperfin::runtime::DebugPauseReason::event_loop,
           "READ EVENTS should place runtime into event-loop pause");

    const bool dispatched = session.dispatch_event_handler("AppShutdown");
    expect(dispatched, "shutdown event handler should dispatch while in READ EVENTS");

    const auto completed = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(completed.completed, "CLEAR EVENTS + QUIT shutdown path should complete cleanly");
    expect(completed.reason == copperfin::runtime::DebugPauseReason::completed,
           "runtime should report completed after cleanup-enhanced shutdown handler");

    const bool has_quit = std::any_of(completed.events.begin(), completed.events.end(),
        [](const auto &e) { return e.category == "runtime.quit"; });
    expect(has_quit, "cleanup-enhanced shutdown path should still emit runtime.quit event");

    fs::remove_all(tmp, ign);
}

void test_on_shutdown_clear_events_runs_and_quit_completes() {
    namespace fs = std::filesystem;
    const fs::path tmp = fs::temp_directory_path() / "copperfin_on_shutdown_clear_events";
    std::error_code ign;
    fs::remove_all(tmp, ign);
    fs::create_directories(tmp);

    const fs::path prg = tmp / "test.prg";
    write_text(prg,
               "ON SHUTDOWN CLEAR EVENTS\n"
               "QUIT\n"
               "after_quit = 1\n"
               "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(prg.string(), tmp.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);

    expect(state.completed, "ON SHUTDOWN CLEAR EVENTS + QUIT should complete cleanly");
    expect(state.globals.find("after_quit") == state.globals.end(), "QUIT should prevent statements after it from running");

    const bool has_shutdown = std::any_of(state.events.begin(), state.events.end(),
        [](const auto &e) { return e.category == "runtime.shutdown_handler" && e.detail == "CLEAR EVENTS"; });
    expect(has_shutdown, "ON SHUTDOWN CLEAR EVENTS should emit runtime.shutdown_handler event");

    const bool has_quit = std::any_of(state.events.begin(), state.events.end(),
        [](const auto &e) { return e.category == "runtime.quit"; });
    expect(has_quit, "QUIT should still emit runtime.quit after ON SHUTDOWN CLEAR EVENTS");

    fs::remove_all(tmp, ign);
}

void test_on_shutdown_do_cleanup_can_call_quit_without_recursing() {
    namespace fs = std::filesystem;
    const fs::path tmp = fs::temp_directory_path() / "copperfin_on_shutdown_do_cleanup";
    std::error_code ign;
    fs::remove_all(tmp, ign);
    fs::create_directories(tmp);

    write_text(tmp / "held.txt", "seed");

    const fs::path prg = tmp / "test.prg";
    write_text(prg,
               "ON SHUTDOWN DO CleanupProcedure\n"
               "READ EVENTS\n"
               "RETURN\n"
               "PROCEDURE RequestQuit\n"
               "    nConn = SQLCONNECT('dsn=Northwind')\n"
               "    obj = CREATEOBJECT('Sample.Object')\n"
               "    nHandle = FOPEN('held.txt', 2)\n"
               "    QUIT\n"
               "ENDPROC\n"
               "PROCEDURE CleanupProcedure\n"
               "    PUBLIC cleanup_marker\n"
               "    cleanup_marker = 1\n"
               "    CLEAR EVENTS\n"
               "    CLOSE DATABASES ALL\n"
               "    QUIT\n"
               "ENDPROC\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(prg.string(), tmp.string(), false));

    const auto paused = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(paused.reason == copperfin::runtime::DebugPauseReason::event_loop,
           "READ EVENTS should pause before requesting quit");

    const bool dispatched = session.dispatch_event_handler("RequestQuit");
    expect(dispatched, "RequestQuit should dispatch from READ EVENTS");

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "ON SHUTDOWN DO cleanup with nested QUIT should complete cleanly");

    const auto cleanup = state.globals.find("cleanup_marker");
    expect(cleanup != state.globals.end(), "CleanupProcedure should run before final quit");
    if (cleanup != state.globals.end()) {
        expect(cleanup->second.number_value == 1.0, "CleanupProcedure should set cleanup_marker");
    }

    const bool has_shutdown = std::any_of(state.events.begin(), state.events.end(),
        [](const auto &e) { return e.category == "runtime.shutdown_handler" && e.detail == "CleanupProcedure"; });
    expect(has_shutdown, "ON SHUTDOWN DO CleanupProcedure should emit runtime.shutdown_handler event");

    const std::size_t quit_event_count = static_cast<std::size_t>(std::count_if(state.events.begin(), state.events.end(),
        [](const auto &e) { return e.category == "runtime.quit"; }));
    expect(quit_event_count == 1U, "Nested QUIT inside shutdown handler should not recurse into multiple quit events");

    expect(state.sql_connections.empty(), "Shutdown cleanup QUIT path should leave no SQL connections");
    expect(state.ole_objects.empty(), "Shutdown cleanup QUIT path should leave no OLE objects");

    fs::remove_all(tmp, ign);
}

void test_on_shutdown_inline_close_databases_all_runs_before_quit() {
    namespace fs = std::filesystem;
    const fs::path tmp = fs::temp_directory_path() / "copperfin_on_shutdown_close_databases_all";
    std::error_code ign;
    fs::remove_all(tmp, ign);
    fs::create_directories(tmp);

    write_text(tmp / "held.txt", "seed");

    const fs::path prg = tmp / "test.prg";
    write_text(prg,
               "ON SHUTDOWN CLOSE DATABASES ALL\n"
               "READ EVENTS\n"
               "RETURN\n"
               "PROCEDURE RequestQuit\n"
               "    nConn = SQLCONNECT('dsn=Northwind')\n"
               "    obj = CREATEOBJECT('Sample.Object')\n"
               "    nHandle = FOPEN('held.txt', 2)\n"
               "    QUIT\n"
               "ENDPROC\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(prg.string(), tmp.string(), false));

    const auto paused = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(paused.reason == copperfin::runtime::DebugPauseReason::event_loop,
           "READ EVENTS should pause before inline shutdown close runs");

    const bool dispatched = session.dispatch_event_handler("RequestQuit");
    expect(dispatched, "RequestQuit should dispatch from READ EVENTS");

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "ON SHUTDOWN CLOSE DATABASES ALL + QUIT should complete cleanly");
    expect(state.sql_connections.empty(), "Inline CLOSE DATABASES ALL should leave no SQL connections");
    expect(state.ole_objects.empty(), "Inline CLOSE DATABASES ALL should leave no OLE handles");

    const bool has_shutdown = std::any_of(state.events.begin(), state.events.end(),
        [](const auto &e) { return e.category == "runtime.shutdown_handler" && e.detail == "CLOSE DATABASES ALL"; });
    expect(has_shutdown, "ON SHUTDOWN CLOSE DATABASES ALL should emit runtime.shutdown_handler event");

    const bool has_close = std::any_of(state.events.begin(), state.events.end(),
        [](const auto &e) { return e.category == "runtime.close" && e.detail == "DATABASES ALL"; });
    expect(has_close, "Inline shutdown close clause should emit runtime.close event");

    fs::remove_all(tmp, ign);
}

void test_quit_closes_open_database_and_runtime_handles() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_quit_resource_cleanup";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_simple_dbf(temp_root / "a.dbf", {"alpha", "beta"});
    write_text(temp_root / "held.txt", "seed");

    const fs::path quit_path = temp_root / "quit_cleanup.prg";
    write_text(
        quit_path,
        "USE '" + (temp_root / "a.dbf").string() + "'\n"
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "obj = CREATEOBJECT('Sample.Object')\n"
        "nHandle = FOPEN('held.txt', 2)\n"
        "QUIT\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(quit_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "QUIT cleanup script should complete");
    expect(state.cursors.empty(), "QUIT should close open database cursors/work areas");
    expect(state.work_area.aliases.empty(), "QUIT should clear work-area aliases");
    expect(state.sql_connections.empty(), "QUIT should disconnect open SQL connections");
    expect(state.ole_objects.empty(), "QUIT should release tracked OLE object handles");

    const auto handle_it = state.globals.find("nhandle");
    expect(handle_it != state.globals.end(), "FOPEN handle should be captured before QUIT");
    int handle = 1;
    if (handle_it != state.globals.end()) {
        handle = static_cast<int>(handle_it->second.number_value);
    }

    const fs::path verify_path = temp_root / "verify_handle_closed.prg";
    write_text(
        verify_path,
        "nClose = FCLOSE(" + std::to_string(handle) + ")\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession verify_session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(verify_path.string(), temp_root.string(), false));
    const auto verify_state = verify_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(verify_state.completed, "verification script should complete");

    const auto close_it = verify_state.globals.find("nclose");
    expect(close_it != verify_state.globals.end(), "verification script should expose FCLOSE result");
    if (close_it != verify_state.globals.end()) {
        expect(close_it->second.number_value == -1.0,
               "QUIT should already close FOPEN handles so follow-up FCLOSE returns -1");
    }

    fs::remove_all(temp_root, ignored);
}

}  // namespace cf_test_prg_engine_control_flow
