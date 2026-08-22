// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_prg_engine_control_flow_support.h"

namespace cf_test_prg_engine_control_flow {
void test_catch_when_false_falls_through_to_later_clause() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_catch_when_fallthrough";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "catch_when_fallthrough.prg";
    write_text(
        main_path,
        "TRY\n"
        "  THROW 42\n"
        "CATCH TO oSkip WHEN oSkip.ErrorNo = 1\n"
        "  cHandled = 'wrong'\n"
        "CATCH TO oMatch WHEN oMatch.ErrorNo = 2071 AND VARTYPE(oSkip) = 'U'\n"
        "  cHandled = 'right'\n"
        "  nCaughtValue = oMatch.UserValue\n"
        "  cCaughtType = VARTYPE(oMatch.UserValue)\n"
        "  cSkipType = VARTYPE(oSkip)\n"
        "  nErrRows = AERROR(aErr)\n"
        "  nErrCode = aErr[1,1]\n"
        "ENDTRY\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "CATCH WHEN fallthrough script should complete: " + state.message);

    const auto check = [&](const std::string& name, const std::string& expected) {
        const auto it = state.globals.find(name);
        if (it == state.globals.end()) {
            expect(false, name + " should be captured");
            return;
        }
        expect(copperfin::runtime::format_value(it->second) == expected,
               name + " expected '" + expected + "' got '" + copperfin::runtime::format_value(it->second) + "'");
    };

    check("chandled", "right");
    check("ncaughtvalue", "42");
    check("ccaughttype", "N");
    check("cskiptype", "U");
    check("nerrrows", "1");
    check("nerrcode", "2071");

    fs::remove_all(temp_root, ignored);
}

void test_catch_to_when_false_resets_variable_and_falls_to_outer_handler() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_catch_when_outer";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "catch_when_outer.prg";
    write_text(
        main_path,
        "TRY\n"
        "  TRY\n"
        "    THROW 42\n"
        "  CATCH TO oSkip WHEN oSkip.ErrorNo = 1\n"
        "    cInnerHandled = 'wrong'\n"
        "  ENDTRY\n"
        "CATCH TO oOuter\n"
        "  lOuterHandled = .T.\n"
        "  cSkipType = VARTYPE(oSkip)\n"
        "  nOuterValue = oOuter.UserValue\n"
        "  cOuterType = VARTYPE(oOuter.UserValue)\n"
        "  nErrRows = AERROR(aErr)\n"
        "  nErrCode = aErr[1,1]\n"
        "ENDTRY\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "outer-handler CATCH WHEN script should complete: " + state.message);

    const auto check = [&](const std::string& name, const std::string& expected) {
        const auto it = state.globals.find(name);
        if (it == state.globals.end()) {
            expect(false, name + " should be captured");
            return;
        }
        expect(copperfin::runtime::format_value(it->second) == expected,
               name + " expected '" + expected + "' got '" + copperfin::runtime::format_value(it->second) + "'");
    };

    check("louterhandled", "true");
    check("cskiptype", "U");
    check("noutervalue", "42");
    check("coutertype", "N");
    check("nerrrows", "1");
    check("nerrcode", "2071");

    const auto inner_handled = state.globals.find("cinnerhandled");
    expect(inner_handled == state.globals.end(),
           "non-matching inner CATCH WHEN should not execute its body before outer fallthrough");

    fs::remove_all(temp_root, ignored);
}
} // namespace cf_test_prg_engine_control_flow
