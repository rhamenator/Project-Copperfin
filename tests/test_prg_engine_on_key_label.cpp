// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "prg_engine_test_support.h"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <system_error>

namespace {

void test_on_key_label_assignments_dispatch_and_stack_restore()
{
    namespace fs = std::filesystem;
    using copperfin::runtime::DebugPauseReason;
    using copperfin::runtime::DebugResumeAction;
    using copperfin::runtime::PrgRuntimeSession;
    using copperfin::runtime::format_value;
    using copperfin::test_support::expect;
    using copperfin::test_support::make_runtime_session_options;
    using copperfin::test_support::write_text;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_on_key_label";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "on_key_label.prg";
    write_text(
        main_path,
        "PUBLIC cTrace\n"
        "cTrace = ''\n"
        "ON KEY F5 cTrace = cTrace + 'A'\n"
        "ON KEY LABEL F6 cTrace = cTrace + 'X'\n"
        "ON KEY LABEL F6\n"
        "PUSH KEY CLEAR\n"
        "ON KEY LABEL F5 cTrace = cTrace + 'B'\n"
        "POP KEY\n"
        "ON KEY LABEL F9 ON KEY\n"
        "READ EVENTS\n"
        "RETURN\n");

    PrgRuntimeSession session = PrgRuntimeSession::create(
        make_runtime_session_options(main_path, temp_root));
    auto state = session.run(DebugResumeAction::continue_run);
    expect(state.reason == DebugPauseReason::event_loop,
        "ON KEY LABEL fixture should enter READ EVENTS");
    expect(!session.dispatch_key_label("F6"),
        "ON KEY LABEL <key> without a command should restore that key to normal behavior");
    expect(session.dispatch_key_label("f5"),
        "case-insensitive host key labels should dispatch the restored assignment");
    state = session.run(DebugResumeAction::continue_run);
    const auto after_five = state.globals.find("ctrace");
    expect(state.reason == DebugPauseReason::event_loop &&
               after_five != state.globals.end() && format_value(after_five->second) == "A",
        "POP KEY should restore the assignment saved before PUSH KEY CLEAR");
    expect(session.dispatch_key_label("F9"),
        "ON KEY LABEL should dispatch a static ON KEY command at the next wait state");
    state = session.run(DebugResumeAction::continue_run);
    expect(state.reason == DebugPauseReason::event_loop,
        "ON KEY handler should return to the active event loop");
    expect(!session.dispatch_key_label("F5"),
        "ON KEY should clear all active ON KEY LABEL assignments");
    expect(
        std::any_of(state.events.begin(), state.events.end(), [](const auto &event) {
            return event.category == "runtime.push_key" && event.detail.find("target=CLEAR") != std::string::npos;
        }),
        "PUSH KEY CLEAR should retain stack telemetry while snapshotting assignments");
    expect(
        std::any_of(state.events.begin(), state.events.end(), [](const auto &event) {
            return event.category == "runtime.key_label" && event.detail == "key=F5";
        }),
        "ON KEY LABEL dispatch should emit invariant key-label telemetry");

    const fs::path clear_all_path = temp_root / "pop_key_all.prg";
    write_text(
        clear_all_path,
        "ON KEY LABEL F5 WAIT WINDOW 'saved' NOWAIT\n"
        "PUSH KEY\n"
        "ON KEY LABEL F6 WAIT WINDOW 'temporary' NOWAIT\n"
        "POP KEY ALL\n"
        "READ EVENTS\n"
        "RETURN\n");
    PrgRuntimeSession clear_all_session = PrgRuntimeSession::create(
        make_runtime_session_options(clear_all_path, temp_root));
    state = clear_all_session.run(DebugResumeAction::continue_run);
    expect(state.reason == DebugPauseReason::event_loop,
        "POP KEY ALL fixture should enter READ EVENTS");
    expect(!clear_all_session.dispatch_key_label("F5") &&
               !clear_all_session.dispatch_key_label("F6"),
        "POP KEY ALL should clear both current assignments and saved key-assignment snapshots");

    fs::remove_all(temp_root, ignored);
}

}  // namespace

int main()
{
    test_on_key_label_assignments_dispatch_and_stack_restore();
    if (copperfin::test_support::test_failures() != 0)
    {
        std::cerr << copperfin::test_support::test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }
    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
