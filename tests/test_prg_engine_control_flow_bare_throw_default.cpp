// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_prg_engine_control_flow_support.h"

namespace cf_test_prg_engine_control_flow {
void test_bare_throw_without_active_exception_creates_user_thrown_default() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_bare_throw_default";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "bare_throw_default.prg";
    write_text(
        main_path,
        "TRY\n"
        "  THROW\n"
        "CATCH TO oErr\n"
        "  cCatchMessage = oErr.Message\n"
        "  cCatchUserValueType = VARTYPE(oErr.UserValue)\n"
        "  nErrRows = AERROR(aErr)\n"
        "  nErrCode = aErr[1,1]\n"
        "  cErrMsg = aErr[1,2]\n"
        "  cErrParam = aErr[1,3]\n"
        "  nErrLine = aErr[1,5]\n"
        "  cErrStmt = aErr[1,7]\n"
        "  nFnCode = ERROR()\n"
        "  cFnMsg = MESSAGE()\n"
        "  nFnLine = LINENO()\n"
        "ENDTRY\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "bare THROW default script should complete: " + state.message);

    const auto check = [&](const std::string& name, const std::string& expected) {
        const auto it = state.globals.find(name);
        if (it == state.globals.end()) {
            expect(false, name + " should be captured");
            return;
        }
        expect(copperfin::runtime::format_value(it->second) == expected,
               name + " expected '" + expected + "' got '" + copperfin::runtime::format_value(it->second) + "'");
    };

    check("ccatchmessage", "User Thrown Error.");
    check("ccatchuservaluetype", "U");
    check("nerrrows", "1");
    check("cerrmsg", "User Thrown Error.");
    check("cerrparam", "");
    check("cerrstmt", "THROW");
    check("cfnmsg", "User Thrown Error.");

    const auto err_code = state.globals.find("nerrcode");
    const auto err_line = state.globals.find("nerrline");
    const auto fn_code = state.globals.find("nfncode");
    const auto fn_line = state.globals.find("nfnline");

    expect(err_code != state.globals.end(), "AERROR() should expose bare THROW default code");
    expect(err_line != state.globals.end(), "AERROR() should expose bare THROW default line");
    expect(fn_code != state.globals.end(), "ERROR() should expose bare THROW default code");
    expect(fn_line != state.globals.end(), "LINENO() should expose bare THROW default line");

    if (err_code != state.globals.end() && fn_code != state.globals.end()) {
        expect(copperfin::runtime::format_value(err_code->second) == "2071",
               "bare THROW without an active exception should surface ErrorNo 2071");
        expect(copperfin::runtime::format_value(err_code->second) ==
                   copperfin::runtime::format_value(fn_code->second),
               "bare THROW without an active exception should keep AERROR()[1,1] aligned with ERROR()");
    }
    if (err_line != state.globals.end()) {
        expect(copperfin::runtime::format_value(err_line->second) == "2",
               "bare THROW without an active exception should report the THROW line");
    }
    if (fn_line != state.globals.end()) {
        expect(copperfin::runtime::format_value(fn_line->second) == "2",
               "bare THROW without an active exception should keep LINENO() aligned with the THROW line");
    }

    expect(state.ole_objects.size() == 1U,
           "bare THROW without an active exception should materialize one Exception object");
    if (state.ole_objects.size() == 1U) {
        const auto& caught_object = state.ole_objects[0];
        expect(caught_object.prog_id == "Exception",
               "bare THROW default should preserve the builtin Exception class token");
        const auto error_no = caught_object.properties.find("errorno");
        const auto user_value = caught_object.properties.find("uservalue");
        expect(error_no != caught_object.properties.end() &&
                   copperfin::runtime::format_value(error_no->second) == "2071",
               "bare THROW default should preserve user-thrown ErrorNo 2071");
        expect(user_value != caught_object.properties.end() &&
                   user_value->second.kind == copperfin::runtime::PrgValueKind::empty,
               "bare THROW default should leave UserValue empty");
    }

    fs::remove_all(temp_root, ignored);
}

}  // namespace cf_test_prg_engine_control_flow
