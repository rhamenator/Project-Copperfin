// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_prg_engine_control_flow_support.h"

namespace cf_test_prg_engine_control_flow {
void test_throw_exception_object_chains_outer_uservalue_reference() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_throw_exception_chain";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "throw_exception_chain.prg";
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
        "    THROW oInner\n"
        "  ENDTRY\n"
        "CATCH TO oOuter\n"
        "  lOuterIsInner = COMPOBJ(oOuter, oInner)\n"
        "  cOuterMsg = oOuter.Message\n"
        "  nOuterCode = oOuter.ErrorNo\n"
        "  nOuterLine = oOuter.LineNo\n"
        "  cOuterStmt = oOuter.LineContents\n"
        "  cOuterUserValueType = VARTYPE(oOuter.UserValue)\n"
        "  lOuterUserValueIsInner = COMPOBJ(oOuter.UserValue, oInner)\n"
        "  oChained = oOuter.UserValue\n"
        "  cChainedInnerMsg = oChained.Message\n"
        "  nChainedInnerLine = oChained.LineNo\n"
        "  cChainedInnerStmt = oChained.LineContents\n"
        "  cChainedInnerUserValue = oChained.UserValue\n"
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
    expect(state.completed, "THROW oInner chaining script should complete: " + state.message);

    const auto check = [&](const std::string& name, const std::string& expected) {
        const auto it = state.globals.find(name);
        if (it == state.globals.end()) {
            expect(false, name + " should be captured");
            return;
        }
        expect(copperfin::runtime::format_value(it->second) == expected,
               name + " expected '" + expected + "' got '" + copperfin::runtime::format_value(it->second) + "'");
    };

    check("louterisinner", "false");
    check("coutermsg", "User Thrown Error.");
    check("noutercode", "2071");
    check("couterstmt", "THROW oInner");
    check("couteruservaluetype", "O");
    check("louteruservalueisinner", "true");
    check("cchainedinneruservalue", "patched");
    check("nerrrows", "1");
    check("nerrcode", "2071");
    check("cerrmsg", "User Thrown Error.");
    check("cerrstmt", "THROW oInner");
    check("cfnmsg", "User Thrown Error.");

    const auto inner_message = state.globals.find("cinnermsg");
    const auto inner_line = state.globals.find("ninnerline");
    const auto inner_stmt = state.globals.find("cinnerstmt");
    const auto outer_line = state.globals.find("nouterline");
    const auto chained_inner_msg = state.globals.find("cchainedinnermsg");
    const auto chained_inner_line = state.globals.find("nchainedinnerline");
    const auto chained_inner_stmt = state.globals.find("cchainedinnerstmt");
    const auto err_line = state.globals.find("nerrline");
    const auto fn_code = state.globals.find("nfncode");
    const auto fn_line = state.globals.find("nfnline");

    expect(inner_message != state.globals.end(), "inner exception chain script should expose inner Message");
    expect(inner_line != state.globals.end(), "inner exception chain script should expose inner LineNo");
    expect(inner_stmt != state.globals.end(), "inner exception chain script should expose inner LineContents");
    expect(outer_line != state.globals.end(), "outer chained exception should expose LineNo");
    expect(chained_inner_msg != state.globals.end(), "outer chained UserValue should expose inner Message");
    expect(chained_inner_line != state.globals.end(), "outer chained UserValue should expose inner LineNo");
    expect(chained_inner_stmt != state.globals.end(), "outer chained UserValue should expose inner LineContents");
    expect(err_line != state.globals.end(), "AERROR() should expose outer chained line");
    expect(fn_code != state.globals.end(), "ERROR() should expose outer chained code");
    expect(fn_line != state.globals.end(), "LINENO() should expose outer chained line");

    if (inner_message != state.globals.end() && chained_inner_msg != state.globals.end()) {
        expect(copperfin::runtime::format_value(inner_message->second) ==
                   copperfin::runtime::format_value(chained_inner_msg->second),
               "outer chained UserValue should preserve the original inner Message");
    }
    if (inner_line != state.globals.end() && chained_inner_line != state.globals.end()) {
        expect(copperfin::runtime::format_value(inner_line->second) ==
                   copperfin::runtime::format_value(chained_inner_line->second),
               "outer chained UserValue should preserve the original inner LineNo");
    }
    if (inner_stmt != state.globals.end() && chained_inner_stmt != state.globals.end()) {
        expect(copperfin::runtime::format_value(inner_stmt->second) ==
                   copperfin::runtime::format_value(chained_inner_stmt->second),
               "outer chained UserValue should preserve the original inner LineContents");
    }
    if (outer_line != state.globals.end()) {
        expect(copperfin::runtime::format_value(outer_line->second) == "9",
               "outer chained exception should report the THROW oInner line");
    }
    if (err_line != state.globals.end()) {
        expect(copperfin::runtime::format_value(err_line->second) == "9",
               "AERROR()[1,5] should report the THROW oInner line");
    }
    if (fn_line != state.globals.end()) {
        expect(copperfin::runtime::format_value(fn_line->second) == "9",
               "LINENO() should report the THROW oInner line");
    }
    if (fn_code != state.globals.end()) {
        expect(copperfin::runtime::format_value(fn_code->second) == "2071",
               "ERROR() should report user-thrown code 2071 for THROW oInner");
    }

    expect(state.ole_objects.size() == 2U,
           "THROW oInner chaining should materialize distinct inner and outer Exception objects");

    fs::remove_all(temp_root, ignored);
}
} // namespace cf_test_prg_engine_control_flow
