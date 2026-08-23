// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_prg_engine_control_flow_support.h"

namespace cf_test_prg_engine_control_flow {
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
