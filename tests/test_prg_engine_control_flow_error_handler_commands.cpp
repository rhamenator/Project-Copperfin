// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_prg_engine_control_flow_support.h"

namespace cf_test_prg_engine_control_flow {
void test_bare_on_error_restores_default_error_handling() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_bare_on_error";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "bare_on_error.prg";
    write_text(
        main_path,
        "PUBLIC handlerCount\n"
        "handlerCount = 0\n"
        "ON ERROR DO handleerr\n"
        "DO missing_handled_target\n"
        "ON ERROR\n"
        "DO missing_uncaught_target\n"
        "afterUncaughtFault = .T.\n"
        "RETURN\n"
        "PROCEDURE handleerr\n"
        "handlerCount = handlerCount + 1\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!state.completed, "bare ON ERROR should restore uncaught-error behavior");
    expect(state.message.find("missing_uncaught_target") != std::string::npos,
           "the fault after bare ON ERROR should propagate with its target identity: " + state.message);

    const auto handler_count = state.globals.find("handlercount");
    expect(handler_count != state.globals.end(), "the installed handler should process the first fault");
    if (handler_count != state.globals.end()) {
        expect(copperfin::runtime::format_value(handler_count->second) == "1",
               "bare ON ERROR should prevent the old handler from processing the second fault");
    }
    expect(state.globals.find("afteruncaughtfault") == state.globals.end(),
           "execution should stop at the uncaught fault after bare ON ERROR");

    const auto handler_events = std::count_if(
        state.events.begin(),
        state.events.end(),
        [](const copperfin::runtime::RuntimeEvent &event) {
            return event.category == "runtime.error_handler";
        });
    expect(handler_events == 1, "only the fault before bare ON ERROR should dispatch the installed handler");

    fs::remove_all(temp_root, ignored);
}
}  // namespace cf_test_prg_engine_control_flow
