// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_prg_engine_control_flow_support.h"

namespace cf_test_prg_engine_control_flow {
void test_thrown_expression_fault_aerror_columns_match_error_message_functions() {
    // #153: when a thrown expression fault is caught via ON ERROR, AERROR()
    // columns must agree with ERROR(), MESSAGE(), and LINENO() diagnostic functions
    // so developers see a consistent normalized diagnostic surface.
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_aerror_norm";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "aerror_norm.prg";
    write_text(
        main_path,
        "ON ERROR DO handleerr\n"
        "bad_val = LOG(-1)\n"
        "after_fault = 1\n"
        "RETURN\n"
        "PROCEDURE handleerr\n"
        "PUBLIC nErrRows, nErrCode, cErrMsg, nErrLine, cErrProc, nFnCode, cFnMsg, nFnLine, cFnProg\n"
        "nErrRows = AERROR(aErrNorm)\n"
        "nErrCode = aErrNorm[1,1]\n"
        "cErrMsg = aErrNorm[1,2]\n"
        "nErrLine = aErrNorm[1,5]\n"
        "cErrProc = aErrNorm[1,6]\n"
        "nFnCode = ERROR()\n"
        "cFnMsg = MESSAGE()\n"
        "nFnLine = LINENO()\n"
        "cFnProg = PROGRAM()\n"
        "RETURN\n"
        "ENDPROC\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "#153: AERROR normalization script should complete");

    const auto err_rows = state.globals.find("nerrrows");
    const auto err_code = state.globals.find("nerrcode");
    const auto err_msg = state.globals.find("cerrmsg");
    const auto err_line = state.globals.find("nerrline");
    const auto err_proc = state.globals.find("cerrproc");
    const auto fn_code = state.globals.find("nfncode");
    const auto fn_msg = state.globals.find("cfnmsg");
    const auto fn_line = state.globals.find("nfnline");
    const auto fn_prog = state.globals.find("cfnprog");
    const auto after_fault = state.globals.find("after_fault");

    expect(err_rows != state.globals.end(), "#153: AERROR() should return a row count");
    expect(err_code != state.globals.end(), "#153: AERROR() column 1 (error code) should be set");
    expect(err_msg != state.globals.end(), "#153: AERROR() column 2 (message) should be set");
    expect(err_line != state.globals.end(), "#153: AERROR() column 5 (line) should be set");
    expect(err_proc != state.globals.end(), "#153: AERROR() column 6 (procedure) should be set");
    expect(fn_code != state.globals.end(), "#153: ERROR() function should be available in handler");
    expect(fn_msg != state.globals.end(), "#153: MESSAGE() function should be available in handler");
    expect(fn_line != state.globals.end(), "#153: LINENO() function should be available in handler");
    expect(fn_prog != state.globals.end(), "#153: PROGRAM() function should be available in handler");
    expect(after_fault != state.globals.end(), "#153: execution should continue after ON ERROR handler");

    // AERROR column 1 must equal ERROR()
    if (err_code != state.globals.end() && fn_code != state.globals.end()) {
        expect(copperfin::runtime::format_value(err_code->second) ==
               copperfin::runtime::format_value(fn_code->second),
               "#153: AERROR()[1,1] error code should match ERROR() function value");
    }
    // AERROR column 2 must equal MESSAGE()
    if (err_msg != state.globals.end() && fn_msg != state.globals.end()) {
        expect(copperfin::runtime::format_value(err_msg->second) ==
               copperfin::runtime::format_value(fn_msg->second),
               "#153: AERROR()[1,2] message should match MESSAGE() function value");
    }
    // AERROR column 5 must equal LINENO()
    if (err_line != state.globals.end() && fn_line != state.globals.end()) {
        expect(copperfin::runtime::format_value(err_line->second) ==
               copperfin::runtime::format_value(fn_line->second),
               "#153: AERROR()[1,5] line should match LINENO() function value");
    }
    // AERROR column 6 must equal PROGRAM()
    if (err_proc != state.globals.end() && fn_prog != state.globals.end()) {
        expect(copperfin::runtime::format_value(err_proc->second) ==
               copperfin::runtime::format_value(fn_prog->second),
               "#153: AERROR()[1,6] procedure should match PROGRAM() function value");
    }
    // The fault line should be line 2 (bad_val = LOG(-1))
    if (err_line != state.globals.end()) {
        expect(copperfin::runtime::format_value(err_line->second) == "2",
               "#153: AERROR()[1,5] should report line 2 as the faulting line");
    }
    // The message must contain meaningful diagnostic text for a LOG(-1) fault
    if (err_msg != state.globals.end()) {
        expect(!copperfin::runtime::format_value(err_msg->second).empty(),
               "#153: AERROR()[1,2] diagnostic message should be non-empty for a thrown expression fault");
    }
    if (err_rows != state.globals.end()) {
        expect(copperfin::runtime::format_value(err_rows->second) == "1",
               "#153: AERROR() should return exactly one row for a single fault");
    }

    fs::remove_all(temp_root, ignored);
}

void test_repeated_on_error_faults_refresh_normalized_diagnostics() {
    // #153: AERROR()/ERROR()/MESSAGE()/LINENO()/PROGRAM() should refresh for
    // each new ON ERROR fault, not retain stale values from prior faults.
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_aerror_norm_repeat";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "aerror_norm_repeat.prg";
    write_text(
        main_path,
        "ON ERROR DO handleerr\n"
        "x = LOG(-1)\n"
        "y = ACOS(2)\n"
        "RETURN\n"
        "PROCEDURE handleerr\n"
        "PUBLIC gFaultCount, nErrRows, nErrCode1, cErrMsg1, nErrLine1, cErrProc1, nFnCode1, cFnMsg1, nFnLine1, cFnProg1, nErrCode2, cErrMsg2, nErrLine2, cErrProc2, nFnCode2, cFnMsg2, nFnLine2, cFnProg2\n"
        "IF TYPE('gFaultCount') <> 'N'\n"
        "    gFaultCount = 0\n"
        "ENDIF\n"
        "gFaultCount = gFaultCount + 1\n"
        "nErrRows = AERROR(aErrNorm)\n"
        "IF gFaultCount = 1\n"
        "    nErrCode1 = aErrNorm[1,1]\n"
        "    cErrMsg1 = aErrNorm[1,2]\n"
        "    nErrLine1 = aErrNorm[1,5]\n"
        "    cErrProc1 = aErrNorm[1,6]\n"
        "    nFnCode1 = ERROR()\n"
        "    cFnMsg1 = MESSAGE()\n"
        "    nFnLine1 = LINENO()\n"
        "    cFnProg1 = PROGRAM()\n"
        "ELSE\n"
        "    nErrCode2 = aErrNorm[1,1]\n"
        "    cErrMsg2 = aErrNorm[1,2]\n"
        "    nErrLine2 = aErrNorm[1,5]\n"
        "    cErrProc2 = aErrNorm[1,6]\n"
        "    nFnCode2 = ERROR()\n"
        "    cFnMsg2 = MESSAGE()\n"
        "    nFnLine2 = LINENO()\n"
        "    cFnProg2 = PROGRAM()\n"
        "ENDIF\n"
        "RETURN\n"
        "ENDPROC\n");

    const auto state =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false))
            .run(copperfin::runtime::DebugResumeAction::continue_run);

    expect(state.completed, "#153: repeated ON ERROR normalization script should complete");

    const auto nerrline1 = state.globals.find("nerrline1");
    const auto nerrline2 = state.globals.find("nerrline2");
    const auto cfnmsg1 = state.globals.find("cfnmsg1");
    const auto cfnmsg2 = state.globals.find("cfnmsg2");
    const auto nfnline1 = state.globals.find("nfnline1");
    const auto nfnline2 = state.globals.find("nfnline2");
    const auto cfnprog1 = state.globals.find("cfnprog1");
    const auto cfnprog2 = state.globals.find("cfnprog2");
    const auto nerrcode1 = state.globals.find("nerrcode1");
    const auto nerrcode2 = state.globals.find("nerrcode2");
    const auto nfncode1 = state.globals.find("nfncode1");
    const auto nfncode2 = state.globals.find("nfncode2");

    expect(nerrline1 != state.globals.end(), "#153: first fault AERROR line should be captured");
    expect(nerrline2 != state.globals.end(), "#153: second fault AERROR line should be captured");
    expect(cfnmsg1 != state.globals.end(), "#153: first fault MESSAGE() should be captured");
    expect(cfnmsg2 != state.globals.end(), "#153: second fault MESSAGE() should be captured");
    expect(nfnline1 != state.globals.end(), "#153: first fault LINENO() should be captured");
    expect(nfnline2 != state.globals.end(), "#153: second fault LINENO() should be captured");
    expect(cfnprog1 != state.globals.end(), "#153: first fault PROGRAM() should be captured");
    expect(cfnprog2 != state.globals.end(), "#153: second fault PROGRAM() should be captured");
    expect(nerrcode1 != state.globals.end(), "#153: first fault AERROR code should be captured");
    expect(nerrcode2 != state.globals.end(), "#153: second fault AERROR code should be captured");
    expect(nfncode1 != state.globals.end(), "#153: first fault ERROR() should be captured");
    expect(nfncode2 != state.globals.end(), "#153: second fault ERROR() should be captured");

    if (nerrline1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(nerrline1->second) == "2",
               "#153: first fault AERROR line should report line 2");
    }
    if (nerrline2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(nerrline2->second) == "3",
               "#153: second fault AERROR line should report line 3");
    }
    if (nfnline1 != state.globals.end() && nerrline1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(nfnline1->second) ==
                   copperfin::runtime::format_value(nerrline1->second),
               "#153: first fault LINENO() should match AERROR line");
    }
    if (nfnline2 != state.globals.end() && nerrline2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(nfnline2->second) ==
                   copperfin::runtime::format_value(nerrline2->second),
               "#153: second fault LINENO() should match AERROR line");
    }
    if (cfnmsg1 != state.globals.end() && cfnmsg2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(cfnmsg1->second) !=
                   copperfin::runtime::format_value(cfnmsg2->second),
               "#153: message text should refresh between LOG and ACOS faults");
    }
    if (cfnprog1 != state.globals.end() && cfnprog2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(cfnprog1->second) ==
                   copperfin::runtime::format_value(cfnprog2->second),
               "#153: both faults should report the same procedure context");
    }
    if (nerrcode1 != state.globals.end() && nfncode1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(nerrcode1->second) ==
                   copperfin::runtime::format_value(nfncode1->second),
               "#153: first fault AERROR code should match ERROR()");
    }
    if (nerrcode2 != state.globals.end() && nfncode2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(nerrcode2->second) ==
                   copperfin::runtime::format_value(nfncode2->second),
               "#153: second fault AERROR code should match ERROR()");
    }

    fs::remove_all(temp_root, ignored);
}
} // namespace cf_test_prg_engine_control_flow

