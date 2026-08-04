// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_prg_engine_control_flow_support.h"

namespace cf_test_prg_engine_control_flow {

void test_implicit_routine_assignments_are_frame_private() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_implicit_private_scope";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "implicit_private_scope.prg";
    write_text(
        main_path,
        "global_marker = 'original'\n"
        "DO outer_scope\n"
        "after_first_type = TYPE('gLeak')\n"
        "DO outer_scope\n"
        "after_second_type = TYPE('gLeak')\n"
        "DO update_existing_global\n"
        "after_existing_global = global_marker\n"
        "RETURN\n"
        "PROCEDURE outer_scope\n"
        "PUBLIC outer_seen_type\n"
        "DO inner_scope\n"
        "outer_seen_type = TYPE('gLeak')\n"
        "RETURN\n"
        "PROCEDURE inner_scope\n"
        "gLeak = 'temporary'\n"
        "RETURN\n"
        "PROCEDURE update_existing_global\n"
        "global_marker = 'updated'\n"
        "RETURN\n");

    const auto state = copperfin::runtime::PrgRuntimeSession::create(
                           make_runtime_session_options(main_path.string(), temp_root.string(), false))
                           .run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "implicit private scope script should complete: " + state.message);

    const auto check = [&](const std::string &name, const std::string &expected)
    {
        const auto value = state.globals.find(name);
        expect(value != state.globals.end(), name + " should be captured");
        if (value != state.globals.end())
        {
            expect(copperfin::runtime::format_value(value->second) == expected,
                   name + " should preserve implicit-private scope behavior");
        }
    };

    check("outer_seen_type", "U");
    check("after_first_type", "U");
    check("after_second_type", "U");
    check("after_existing_global", "updated");

    fs::remove_all(temp_root, ignored);
}

} // namespace cf_test_prg_engine_control_flow
