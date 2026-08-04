// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_prg_engine_control_flow_support.h"

namespace cf_test_prg_engine_control_flow {

void test_bare_null_expression_preserves_null_error_sentinels() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_bare_null";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "bare_null.prg";
    write_text(
        main_path,
        "LOCAL m.oError, m.oConfig\n"
        "m.oError = NULL\n"
        "STORE NULL TO m.oConfig, m.oError\n"
        "lBareNull = ISNULL(m.oError)\n"
        "cBareType = VARTYPE(m.oError)\n"
        "lDottedNull = ISNULL(.NULL.)\n"
        "TRY\n"
        "    THROW 9001\n"
        "CATCH TO m.oError\n"
        "    lCaughtObject = VARTYPE(m.oError) = 'O'\n"
        "ENDTRY\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "bare NULL expression script should complete: " + state.message);

    const auto bare_null = state.globals.find("lbarenull");
    const auto bare_type = state.globals.find("cbaretype");
    const auto dotted_null = state.globals.find("ldottednull");
    const auto caught_object = state.globals.find("lcaughtobject");
    expect(bare_null != state.globals.end() && copperfin::runtime::format_value(bare_null->second) == "true",
           "bare NULL should be recognized by ISNULL");
    expect(bare_type != state.globals.end() && copperfin::runtime::format_value(bare_type->second) == "X",
           "bare NULL should preserve VARTYPE X");
    expect(dotted_null != state.globals.end() && copperfin::runtime::format_value(dotted_null->second) == "true",
           "dotted NULL should remain recognized");
    expect(caught_object != state.globals.end() &&
               copperfin::runtime::format_value(caught_object->second) == "true",
           "CATCH TO m.oError should bind an Exception object");

    fs::remove_all(temp_root, ignored);
}

}  // namespace cf_test_prg_engine_control_flow
