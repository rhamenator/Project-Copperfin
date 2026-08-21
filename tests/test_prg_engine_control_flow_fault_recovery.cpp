// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_prg_engine_control_flow_support.h"

namespace cf_test_prg_engine_control_flow {

void test_runtime_faults_preserve_state_and_allow_retry() {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::absolute(fs::temp_directory_path()) /
        ("copperfin_prg_engine_runtime_fault_retry_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "runtime_fault_test.prg";
    write_text(main_path,
        "CREATE CURSOR test_cursor (id I)\n"
        "INSERT INTO test_cursor VALUES (1)\n"
        "INSERT INTO test_cursor VALUES (2)\n"
        "GO TOP\n"
        "x = -1\n"
        "ON ERROR DO my_error_handler\n"
        "? LOG(x)\n"
        "PROCEDURE my_error_handler\n"
        "    x = 1\n"
        "    RETRY\n"
        "ENDPROC\n");

    {
        const auto options = make_runtime_session_options(main_path, temp_root, false);
        const fs::path runtime_temp = options.temp_directory;
        expect(runtime_temp.is_absolute(), "#4075: caught-fault runtime temp should be absolute");
        expect(runtime_temp.parent_path().lexically_normal() == temp_root.lexically_normal(),
               "#4075: caught-fault runtime temp should remain beneath its fixture owner");
        auto session = copperfin::runtime::PrgRuntimeSession::create(options);
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);

        if (!state.completed) {
            std::cerr << "Script stopped. Reason: " << copperfin::runtime::debug_pause_reason_name(state.reason)
                      << ", message: " << state.message << std::endl;
        }

        expect(state.completed, "Script should complete after handling fault");
    }

    fs::remove_all(temp_root, ignored);
    expect(!fs::exists(temp_root), "#4075: caught-fault fixture should clean its owned runtime state");
}

}  // namespace cf_test_prg_engine_control_flow
