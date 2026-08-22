// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_prg_engine_control_flow_support.h"

namespace cf_test_prg_engine_control_flow {
void test_bare_throw_rethrows_active_exception_object() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_bare_throw_rethrow";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "bare_throw_rethrow.prg";
    write_text(
        main_path,
        "TRY\n"
        "  TRY\n"
        "    broken = LOG(-1)\n"
        "  CATCH TO oInner\n"
        "    cInnerMsg = oInner.Message\n"
        "    nInnerLine = oInner.LineNo\n"
        "    cInnerStmt = oInner.LineContents\n"
        "    oInner.UserValue = 'patched'\n"
        "    THROW\n"
        "  ENDTRY\n"
        "CATCH TO oOuter\n"
        "  lSameRef = COMPOBJ(oInner, oOuter)\n"
        "  cOuterMsg = oOuter.Message\n"
        "  nOuterLine = oOuter.LineNo\n"
        "  cOuterStmt = oOuter.LineContents\n"
        "  cOuterUserValue = oOuter.UserValue\n"
        "  cOuterUserValueType = VARTYPE(oOuter.UserValue)\n"
        "  nErrRows = AERROR(aErr)\n"
        "  nErrCode = aErr[1,1]\n"
        "  cErrMsg = aErr[1,2]\n"
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
    expect(state.completed, "bare THROW rethrow script should complete: " + state.message);

    const auto check = [&](const std::string& name, const std::string& expected) {
        const auto it = state.globals.find(name);
        if (it == state.globals.end()) {
            expect(false, name + " should be captured");
            return;
        }
        expect(copperfin::runtime::format_value(it->second) == expected,
               name + " expected '" + expected + "' got '" + copperfin::runtime::format_value(it->second) + "'");
    };

    check("lsameref", "true");
    check("couteruservalue", "patched");
    check("couteruservaluetype", "C");
    check("nerrrows", "1");

    const auto inner_message = state.globals.find("cinnermsg");
    const auto outer_message = state.globals.find("coutermsg");
    const auto err_message = state.globals.find("cerrmsg");
    const auto fn_message = state.globals.find("cfnmsg");
    const auto inner_line = state.globals.find("ninnerline");
    const auto outer_line = state.globals.find("nouterline");
    const auto err_line = state.globals.find("nerrline");
    const auto fn_line = state.globals.find("nfnline");
    const auto inner_stmt = state.globals.find("cinnerstmt");
    const auto outer_stmt = state.globals.find("couterstmt");
    const auto err_stmt = state.globals.find("cerrstmt");
    const auto err_code = state.globals.find("nerrcode");
    const auto fn_code = state.globals.find("nfncode");

    expect(inner_message != state.globals.end(), "inner bare THROW catch should expose Message");
    expect(outer_message != state.globals.end(), "outer bare THROW catch should expose Message");
    expect(err_message != state.globals.end(), "AERROR() should expose bare THROW message");
    expect(fn_message != state.globals.end(), "MESSAGE() should expose bare THROW message");
    expect(inner_line != state.globals.end(), "inner bare THROW catch should expose LineNo");
    expect(outer_line != state.globals.end(), "outer bare THROW catch should expose LineNo");
    expect(err_line != state.globals.end(), "AERROR() should expose bare THROW line");
    expect(fn_line != state.globals.end(), "LINENO() should expose bare THROW line");
    expect(inner_stmt != state.globals.end(), "inner bare THROW catch should expose LineContents");
    expect(outer_stmt != state.globals.end(), "outer bare THROW catch should expose LineContents");
    expect(err_stmt != state.globals.end(), "AERROR() should expose bare THROW statement");
    expect(err_code != state.globals.end(), "AERROR() should expose bare THROW code");
    expect(fn_code != state.globals.end(), "ERROR() should expose bare THROW code");

    if (inner_message != state.globals.end() && outer_message != state.globals.end()) {
        expect(copperfin::runtime::format_value(inner_message->second) ==
                   copperfin::runtime::format_value(outer_message->second),
               "bare THROW should preserve the original Exception Message across outer CATCH");
    }
    if (outer_message != state.globals.end() && err_message != state.globals.end() && fn_message != state.globals.end()) {
        expect(copperfin::runtime::format_value(outer_message->second) ==
                   copperfin::runtime::format_value(err_message->second),
               "bare THROW should keep AERROR()[1,2] aligned with the rethrown Exception Message");
        expect(copperfin::runtime::format_value(outer_message->second) ==
                   copperfin::runtime::format_value(fn_message->second),
               "bare THROW should keep MESSAGE() aligned with the rethrown Exception Message");
    }
    if (inner_line != state.globals.end() && outer_line != state.globals.end()) {
        expect(copperfin::runtime::format_value(inner_line->second) ==
                   copperfin::runtime::format_value(outer_line->second),
               "bare THROW should preserve the original Exception LineNo across outer CATCH");
    }
    if (outer_line != state.globals.end() && err_line != state.globals.end() && fn_line != state.globals.end()) {
        expect(copperfin::runtime::format_value(outer_line->second) ==
                   copperfin::runtime::format_value(err_line->second),
               "bare THROW should keep AERROR()[1,5] aligned with the rethrown Exception LineNo");
        expect(copperfin::runtime::format_value(outer_line->second) ==
                   copperfin::runtime::format_value(fn_line->second),
               "bare THROW should keep LINENO() aligned with the rethrown Exception LineNo");
    }
    if (inner_stmt != state.globals.end() && outer_stmt != state.globals.end()) {
        expect(copperfin::runtime::format_value(inner_stmt->second) ==
                   copperfin::runtime::format_value(outer_stmt->second),
               "bare THROW should preserve the original Exception LineContents across outer CATCH");
    }
    if (outer_stmt != state.globals.end() && err_stmt != state.globals.end()) {
        expect(copperfin::runtime::format_value(outer_stmt->second) ==
                   copperfin::runtime::format_value(err_stmt->second),
               "bare THROW should keep AERROR()[1,7] aligned with the rethrown Exception LineContents");
    }
    if (err_code != state.globals.end() && fn_code != state.globals.end()) {
        expect(copperfin::runtime::format_value(err_code->second) ==
                   copperfin::runtime::format_value(fn_code->second),
               "bare THROW should keep AERROR()[1,1] aligned with ERROR()");
    }

    fs::remove_all(temp_root, ignored);
}
} // namespace cf_test_prg_engine_control_flow
