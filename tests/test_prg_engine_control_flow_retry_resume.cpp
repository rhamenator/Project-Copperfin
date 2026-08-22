// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_prg_engine_control_flow_support.h"

namespace cf_test_prg_engine_control_flow {
void test_retry_reexecutes_faulting_statement() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_retry";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "retry_test.prg";
    write_text(
        main_path,
        "attempt_count = 0\n"
        "ON ERROR DO handleerr\n"
        "DO missing_routine\n"
        "after_retry = 1\n"
        "RETURN\n"
        "PROCEDURE handleerr\n"
        "attempt_count = attempt_count + 1\n"
        "IF attempt_count < 2\n"
        "  RETRY\n"
        "ENDIF\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "RETRY test script should complete");

    const auto attempt = state.globals.find("attempt_count");
    const auto after = state.globals.find("after_retry");
    expect(attempt != state.globals.end(), "attempt_count should be set");
    expect(after != state.globals.end(), "execution should continue after RETRY handler finally returns");
    if (attempt != state.globals.end()) {
        expect(copperfin::runtime::format_value(attempt->second) == "2",
               "handler should have been called twice (once retry, once return)");
    }
    if (after != state.globals.end()) {
        expect(copperfin::runtime::format_value(after->second) == "1",
               "post-fault statement should run after handler's normal RETURN");
    }

    fs::remove_all(temp_root, ignored);
}

void test_resume_next_continues_after_fault() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_resume_next";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "resume_test.prg";
    write_text(
        main_path,
        "ON ERROR DO handleerr\n"
        "DO missing_routine\n"
        "after_error = 1\n"
        "RETURN\n"
        "PROCEDURE handleerr\n"
        "PUBLIC handled\n"
        "handled = 1\n"
        "RESUME\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "RESUME test script should complete");

    const auto handled = state.globals.find("handled");
    const auto after_error = state.globals.find("after_error");
    expect(handled != state.globals.end(), "error handler should have run and set handled flag");
    expect(after_error != state.globals.end(), "RESUME should skip the faulting statement and continue to next");
    if (handled != state.globals.end()) {
        expect(copperfin::runtime::format_value(handled->second) == "1", "handled should be 1");
    }
    if (after_error != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_error->second) == "1",
               "statement after faulting one should execute after RESUME");
    }

    fs::remove_all(temp_root, ignored);
}

void test_retry_with_no_fault_checkpoint_is_noop() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_retry_noop";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "retry_noop.prg";
    write_text(
        main_path,
        "noop_count = 1\n"
        "RETRY\n"
        "noop_count = 2\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "RETRY outside error handler should not crash");

    const auto noop_count = state.globals.find("noop_count");
    expect(noop_count != state.globals.end(), "noop_count should be set");
    if (noop_count != state.globals.end()) {
        expect(copperfin::runtime::format_value(noop_count->second) == "2",
               "RETRY with no fault checkpoint is a no-op; execution should continue to next statement");
    }

    fs::remove_all(temp_root, ignored);
}

}  // namespace cf_test_prg_engine_control_flow
