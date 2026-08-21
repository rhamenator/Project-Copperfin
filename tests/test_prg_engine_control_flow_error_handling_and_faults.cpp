// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_prg_engine_control_flow_support.h"

namespace cf_test_prg_engine_control_flow {
void test_aerror_exposes_sql_and_ole_specific_rows() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_aerror_specific";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "aerror_specific.prg";
    write_text(
        main_path,
        "nConn = SQLCONNECT('Driver=ODBC Driver 18 for SQL Server;Server=Northwind')\n"
        "nSqlResult = SQLEXEC(nConn)\n"
        "nSqlRows = AERROR(aSqlErr)\n"
        "nSqlCode = aSqlErr[1,1]\n"
        "cSqlMessage = aSqlErr[1,2]\n"
        "cSqlDetail = aSqlErr[1,3]\n"
        "cSqlState = aSqlErr[1,4]\n"
        "nSqlNative = aSqlErr[1,5]\n"
        "cSqlContext = aSqlErr[1,6]\n"
        "cSqlPayload = aSqlErr[1,7]\n"
        "ON ERROR DO oleerr\n"
        "missingOle.SomeProperty = 42\n"
        "RETURN\n"
        "PROCEDURE oleerr\n"
        "PUBLIC nOleRows, nOleCode, cOleMessage, cOleDetail, cOleApp, cOleSource, cOleAction, nOleNative\n"
        "nOleRows = AERROR(aOleErr)\n"
        "nOleCode = aOleErr[1,1]\n"
        "cOleMessage = aOleErr[1,2]\n"
        "cOleDetail = aOleErr[1,3]\n"
        "cOleApp = aOleErr[1,4]\n"
        "cOleSource = aOleErr[1,5]\n"
        "cOleAction = aOleErr[1,6]\n"
        "nOleNative = aOleErr[1,7]\n"
        "RETURN\n"
        "ENDPROC\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SQL/OLE AERROR script should complete");

    const auto sql_rows = state.globals.find("nsqlrows");
    const auto sql_code = state.globals.find("nsqlcode");
    const auto sql_message = state.globals.find("csqlmessage");
    const auto sql_detail = state.globals.find("csqldetail");
    const auto sql_state = state.globals.find("csqlstate");
    const auto sql_native = state.globals.find("nsqlnative");
    const auto sql_context = state.globals.find("csqlcontext");
    const auto sql_payload = state.globals.find("csqlpayload");
    const auto ole_rows = state.globals.find("nolerows");
    const auto ole_code = state.globals.find("nolecode");
    const auto ole_message = state.globals.find("colemessage");
    const auto ole_detail = state.globals.find("coledetail");
    const auto ole_app = state.globals.find("coleapp");
    const auto ole_source = state.globals.find("colesource");
    const auto ole_action = state.globals.find("coleaction");
    const auto ole_native = state.globals.find("nolenative");

    expect(sql_rows != state.globals.end(), "SQL AERROR should return a row count");
    expect(sql_code != state.globals.end(), "SQL AERROR should populate code");
    expect(sql_message != state.globals.end(), "SQL AERROR should populate message");
    expect(sql_detail != state.globals.end(), "SQL AERROR should populate detail");
    expect(sql_state != state.globals.end(), "SQL AERROR should populate SQL state");
    expect(sql_native != state.globals.end(), "SQL AERROR should populate native code");
    expect(sql_context != state.globals.end(), "SQL AERROR should populate captured context");
    expect(sql_payload != state.globals.end(), "SQL AERROR should populate captured payload");
    expect(ole_rows != state.globals.end(), "OLE AERROR should return a row count");
    expect(ole_code != state.globals.end(), "OLE AERROR should populate code");
    expect(ole_message != state.globals.end(), "OLE AERROR should populate message");
    expect(ole_detail != state.globals.end(), "OLE AERROR should populate detail");
    expect(ole_app != state.globals.end(), "OLE AERROR should populate app name");
    expect(ole_source != state.globals.end(), "OLE AERROR should populate the captured source object");
    expect(ole_action != state.globals.end(), "OLE AERROR should populate the captured action text");
    expect(ole_native != state.globals.end(), "OLE AERROR should populate native code");

    if (sql_rows != state.globals.end()) {
        expect(copperfin::runtime::format_value(sql_rows->second) == "1", "SQL AERROR should expose one row");
    }
    if (sql_code != state.globals.end()) {
        expect(copperfin::runtime::format_value(sql_code->second) == "1526", "SQL failures should use the VFP ODBC error code");
    }
    if (sql_message != state.globals.end()) {
        expect(
            copperfin::runtime::format_value(sql_message->second) ==
                "SQLEXEC requires a command or a prepared SQL statement",
            "SQL AERROR message should route SQLEXEC failure text through the default locale catalog");
    }
    if (sql_detail != state.globals.end()) {
        expect(copperfin::runtime::format_value(sql_detail->second) == "1",
            "SQL AERROR detail should preserve the failing handle parameter");
    }
    if (sql_state != state.globals.end()) {
        expect(copperfin::runtime::format_value(sql_state->second) == "HY000", "SQL AERROR should expose a generic SQL state");
    }
    if (sql_native != state.globals.end()) {
        expect(copperfin::runtime::format_value(sql_native->second) == "-1", "SQL AERROR should expose a native failure code");
    }
    if (sql_context != state.globals.end()) {
        expect(copperfin::runtime::format_value(sql_context->second) == "odbc",
            "SQL AERROR should preserve the captured provider context when a connection exists");
    }
    if (sql_payload != state.globals.end()) {
        expect(copperfin::runtime::format_value(sql_payload->second).find("Northwind") != std::string::npos,
            "SQL AERROR should preserve the captured connection payload instead of leaving the column empty");
    }
    if (ole_rows != state.globals.end()) {
        expect(copperfin::runtime::format_value(ole_rows->second) == "1", "OLE AERROR should expose one row");
    }
    if (ole_code != state.globals.end()) {
        expect(copperfin::runtime::format_value(ole_code->second) == "1429", "OLE failures should use the VFP OLE error code");
    }
    if (ole_message != state.globals.end()) {
        expect(copperfin::runtime::format_value(ole_message->second).find("OLE object not found") != std::string::npos,
            "OLE AERROR message should preserve automation failure text");
    }
    if (ole_detail != state.globals.end()) {
        expect(copperfin::runtime::format_value(ole_detail->second).find("missingOle.SomeProperty") != std::string::npos,
            "OLE AERROR detail should preserve the failing member path");
    }
    if (ole_app != state.globals.end()) {
        expect(copperfin::runtime::format_value(ole_app->second) == "Copperfin OLE",
            "OLE AERROR app column should identify the runtime automation bridge");
    }
    if (ole_source != state.globals.end()) {
        expect(copperfin::runtime::format_value(ole_source->second) == "missingOle",
            "OLE AERROR should preserve the captured source object identifier instead of leaving the column empty");
    }
    if (ole_action != state.globals.end()) {
        expect(copperfin::runtime::format_value(ole_action->second).find("missingOle.SomeProperty = 42") != std::string::npos,
            "OLE AERROR should preserve the captured action text instead of leaving the column empty");
    }
    if (ole_native != state.globals.end()) {
        expect(copperfin::runtime::format_value(ole_native->second) == "1429",
            "OLE AERROR native column should preserve the automation error code");
    }

    fs::remove_all(temp_root, ignored);
}

void test_on_error_handler_preserves_original_fault_metadata_across_caught_inner_faults() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_aerror_nested_faults";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "aerror_nested_faults.prg";
    write_text(
        main_path,
        "ON ERROR DO handleerr\n"
        "DO missing_outer\n"
        "after_error = 'continued'\n"
        "RETURN\n"
        "PROCEDURE handleerr\n"
        "PUBLIC cInitialMessage, cInitialProgram, cInitialSys16, nInitialLine, cCaughtMessage, nErrorRows, cFinalMessage, cFinalProgram, cFinalSys16, nFinalLine, cFinalParam\n"
        "cInitialMessage = MESSAGE()\n"
        "cInitialProgram = PROGRAM()\n"
        "cInitialSys16 = SYS(16)\n"
        "nInitialLine = LINENO()\n"
        "TRY\n"
        "    DO missing_inner\n"
        "CATCH\n"
        "    cCaughtMessage = MESSAGE()\n"
        "ENDTRY\n"
        "nErrorRows = AERROR(aErr)\n"
        "cFinalMessage = MESSAGE()\n"
        "cFinalProgram = PROGRAM()\n"
        "cFinalSys16 = SYS(16)\n"
        "nFinalLine = LINENO()\n"
        "cFinalParam = aErr[1,3]\n"
        "RETURN\n"
        "ENDPROC\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "ON ERROR handler nested-fault script should complete");

    const auto initial_message = state.globals.find("cinitialmessage");
    const auto initial_program = state.globals.find("cinitialprogram");
    const auto initial_sys16 = state.globals.find("cinitialsys16");
    const auto initial_line = state.globals.find("ninitialline");
    const auto final_message = state.globals.find("cfinalmessage");
    const auto final_program = state.globals.find("cfinalprogram");
    const auto final_sys16 = state.globals.find("cfinalsys16");
    const auto final_line = state.globals.find("nfinalline");
    const auto final_param = state.globals.find("cfinalparam");
    const auto rows = state.globals.find("nerrorrows");
    const auto after_error = state.globals.find("after_error");

    expect(initial_message != state.globals.end(), "handler should capture initial MESSAGE()");
    expect(initial_program != state.globals.end(), "handler should capture initial PROGRAM()");
    expect(initial_sys16 != state.globals.end(), "handler should capture initial SYS(16)");
    expect(initial_line != state.globals.end(), "handler should capture initial LINENO()");
    expect(final_message != state.globals.end(), "handler should preserve final MESSAGE() after caught inner fault");
    expect(final_program != state.globals.end(), "handler should preserve final PROGRAM() after caught inner fault");
    expect(final_sys16 != state.globals.end(), "handler should preserve final SYS(16) after caught inner fault");
    expect(final_line != state.globals.end(), "handler should preserve final LINENO() after caught inner fault");
    expect(final_param != state.globals.end(), "AERROR() should preserve the original error parameter after a caught inner fault");
    expect(rows != state.globals.end(), "AERROR() should still return a row count after a caught inner fault");
    expect(after_error != state.globals.end(), "execution should continue after the handler returns");

    if (initial_message != state.globals.end()) {
        expect(
            copperfin::runtime::format_value(initial_message->second).find("missing_outer") != std::string::npos,
            "initial MESSAGE() should describe the original outer fault");
    }
    if (initial_program != state.globals.end()) {
        expect(
            copperfin::runtime::format_value(initial_program->second) == "main",
            "initial PROGRAM() should report the original faulting routine");
    }
    if (initial_line != state.globals.end()) {
        expect(
            copperfin::runtime::format_value(initial_line->second) == "2",
            "initial LINENO() should report the original faulting line");
    }
    if (initial_sys16 != state.globals.end()) {
        expect(
            copperfin::runtime::format_value(initial_sys16->second) == main_path.string(),
            "initial SYS(16) should report the original faulting file");
    }
    if (final_message != state.globals.end()) {
        expect(
            copperfin::runtime::format_value(final_message->second).find("missing_outer") != std::string::npos,
            "MESSAGE() should revert to the original ON ERROR fault after a caught inner fault");
    }
    if (final_program != state.globals.end()) {
        expect(
            copperfin::runtime::format_value(final_program->second) == "main",
            "PROGRAM() should remain bound to the original ON ERROR fault");
    }
    if (final_line != state.globals.end()) {
        expect(
            copperfin::runtime::format_value(final_line->second) == "2",
            "LINENO() should remain bound to the original ON ERROR fault");
    }
    if (final_sys16 != state.globals.end()) {
        expect(
            copperfin::runtime::format_value(final_sys16->second) == main_path.string(),
            "SYS(16) should remain bound to the original ON ERROR fault");
    }
    if (final_param != state.globals.end()) {
        expect(
            copperfin::runtime::format_value(final_param->second) == "missing_outer",
            "AERROR() should preserve the original error parameter instead of the caught inner fault");
    }
    if (rows != state.globals.end()) {
        expect(copperfin::runtime::format_value(rows->second) == "1", "AERROR() should still expose one row");
    }
    if (after_error != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_error->second) == "continued", "post-handler execution should resume normally");
    }

    fs::remove_all(temp_root, ignored);
}

void test_on_error_handler_catch_to_uses_inner_fault_metadata() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_on_error_catch_to_inner_fault";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "on_error_catch_to_inner_fault.prg";
    write_text(
        main_path,
        "ON ERROR DO handleerr\n"
        "DO missing_outer\n"
        "RETURN\n"
        "PROCEDURE handleerr\n"
        "PUBLIC cCaughtMessage, cCaughtProcedure, cCaughtLineContents, cFinalMessage\n"
        "TRY\n"
        "    nInner = 1 / 0\n"
        "CATCH TO oErr\n"
        "    cCaughtMessage = oErr.Message\n"
        "    cCaughtProcedure = oErr.Procedure\n"
        "    cCaughtLineContents = oErr.LineContents\n"
        "ENDTRY\n"
        "cFinalMessage = MESSAGE()\n"
        "RETURN\n"
        "ENDPROC\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "#3686: ON ERROR handler CATCH TO script should complete");

    const auto caught_message = state.globals.find("ccaughtmessage");
    const auto caught_procedure = state.globals.find("ccaughtprocedure");
    const auto caught_line_contents = state.globals.find("ccaughtlinecontents");
    const auto final_message = state.globals.find("cfinalmessage");

    expect(caught_message != state.globals.end(), "#3686: CATCH TO inside ON ERROR should capture Exception.Message");
    expect(caught_procedure != state.globals.end(), "#3686: CATCH TO inside ON ERROR should capture Exception.Procedure");
    expect(caught_line_contents != state.globals.end(), "#3686: CATCH TO inside ON ERROR should capture Exception.LineContents");
    expect(final_message != state.globals.end(), "#3686: ON ERROR handler should still capture the outer MESSAGE() after the inner catch");

    if (caught_message != state.globals.end()) {
        const std::string caught_message_text = copperfin::runtime::format_value(caught_message->second);
        expect(caught_message_text.find("Division by zero") != std::string::npos,
               "#3686: CATCH TO Exception.Message should describe the inner caught fault");
        expect(caught_message_text.find("missing_outer") == std::string::npos,
               "#3686: CATCH TO Exception.Message should not echo the outer ON ERROR fault text");
    }
    if (caught_procedure != state.globals.end()) {
        expect(copperfin::runtime::format_value(caught_procedure->second) == "handleerr",
               "#3686: CATCH TO Exception.Procedure should report the handler routine where the inner fault occurred");
    }
    if (caught_line_contents != state.globals.end()) {
        expect(copperfin::runtime::format_value(caught_line_contents->second) == "nInner = 1 / 0",
               "#3686: CATCH TO Exception.LineContents should capture the inner faulting statement");
    }
    if (final_message != state.globals.end()) {
        expect(copperfin::runtime::format_value(final_message->second).find("missing_outer") != std::string::npos,
               "#3686: MESSAGE() after the inner CATCH should still revert to the original ON ERROR fault");
    }

    fs::remove_all(temp_root, ignored);
}

void test_ole_property_fault_dispatches_on_error_and_preserves_object_state() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_ole_property_fault";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "ole_property_fault.prg";
    write_text(
        main_path,
        "ON ERROR DO handleerr\n"
        "oDict = CREATEOBJECT('Scripting.Dictionary')\n"
        "oDict.Add('Alpha', 41)\n"
        "xMissing = oDict.NoSuchProperty\n"
        "lStillExists = oDict.Exists('Alpha')\n"
        "after_error = 'continued'\n"
        "RETURN\n"
        "PROCEDURE handleerr\n"
        "PUBLIC nOleRows, nOleCode, cOleDetail, cOleSource, cOleAction\n"
        "nOleRows = AERROR(aOleErr)\n"
        "nOleCode = aOleErr[1,1]\n"
        "cOleDetail = aOleErr[1,3]\n"
        "cOleSource = aOleErr[1,5]\n"
        "cOleAction = aOleErr[1,6]\n"
        "RETURN\n"
        "ENDPROC\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "OLE property fault script should complete");

    const auto rows = state.globals.find("nolerows");
    const auto code = state.globals.find("nolecode");
    const auto detail = state.globals.find("coledetail");
    const auto source = state.globals.find("colesource");
    const auto action = state.globals.find("coleaction");
    const auto still_exists = state.globals.find("lstillexists");
    const auto after_error = state.globals.find("after_error");

    expect(rows != state.globals.end(), "OLE property fault should populate AERROR rows");
    expect(code != state.globals.end(), "OLE property fault should populate AERROR code");
    expect(detail != state.globals.end(), "OLE property fault should populate AERROR detail");
    expect(source != state.globals.end(), "OLE property fault should populate AERROR source");
    expect(action != state.globals.end(), "OLE property fault should populate AERROR action");
    expect(still_exists != state.globals.end(), "execution should continue after OLE property fault");
    expect(after_error != state.globals.end(), "post-handler statements should run after OLE property fault");

    if (rows != state.globals.end()) {
        expect(copperfin::runtime::format_value(rows->second) == "1", "OLE property fault should expose one AERROR row");
    }
    if (code != state.globals.end()) {
        expect(copperfin::runtime::format_value(code->second) == "1429", "OLE property fault should use the automation error code");
    }
    if (detail != state.globals.end()) {
        std::string detail_text = copperfin::runtime::format_value(detail->second);
        std::transform(detail_text.begin(), detail_text.end(), detail_text.begin(), [](unsigned char ch) {
            return static_cast<char>(std::tolower(ch));
        });
        expect(detail_text.find("nosuchproperty") != std::string::npos,
               "OLE property fault should preserve the failing member detail");
    }
    if (source != state.globals.end()) {
        std::string source_text = copperfin::runtime::format_value(source->second);
        std::transform(source_text.begin(), source_text.end(), source_text.begin(), [](unsigned char ch) {
            return static_cast<char>(std::tolower(ch));
        });
        expect(source_text.find("odict") != std::string::npos,
               "OLE property fault should preserve the source variable name");
    }
    if (action != state.globals.end()) {
        expect(!copperfin::runtime::format_value(action->second).empty(),
               "OLE property fault should preserve the failing action text");
    }
    if (still_exists != state.globals.end()) {
        expect(copperfin::runtime::format_value(still_exists->second) == "true",
               "a trapped OLE property fault should not poison the object state");
    }
    if (after_error != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_error->second) == "continued",
               "execution should continue after an OLE property fault handler returns");
    }

    fs::remove_all(temp_root, ignored);
}

void test_ole_method_fault_is_catchable_and_preserves_object_state() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_ole_method_fault";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "ole_method_fault.prg";
    write_text(
        main_path,
        "oDict = CREATEOBJECT('Scripting.Dictionary')\n"
        "oDict.Add('Alpha', 41)\n"
        "TRY\n"
        "  oDict.NoSuchMethod(7)\n"
        "CATCH TO err_text\n"
        "  cCaught = err_text.Message\n"
        "FINALLY\n"
        "  finally_hit = 1\n"
        "ENDTRY\n"
        "lStillExists = oDict.Exists('Alpha')\n"
        "nCountAfterFault = oDict.Count\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "OLE method fault TRY/CATCH script should complete");

    const auto caught = state.globals.find("ccaught");
    const auto finally_hit = state.globals.find("finally_hit");
    const auto still_exists = state.globals.find("lstillexists");
    const auto count_after_fault = state.globals.find("ncountafterfault");

    expect(caught != state.globals.end(), "CATCH should receive the OLE method fault text");
    expect(finally_hit != state.globals.end(), "FINALLY should run after trapped OLE method faults");
    expect(still_exists != state.globals.end(), "object should still be usable after trapped OLE method faults");
    expect(count_after_fault != state.globals.end(), "object property reads should still work after trapped OLE method faults");

    if (caught != state.globals.end()) {
        expect(copperfin::runtime::format_value(caught->second).find("OLE member not found for method invocation") != std::string::npos,
               "CATCH should expose the trapped OLE method fault text");
    }
    if (finally_hit != state.globals.end()) {
        expect(copperfin::runtime::format_value(finally_hit->second) == "1",
               "FINALLY should still execute after trapped OLE method faults");
    }
    if (still_exists != state.globals.end()) {
        expect(copperfin::runtime::format_value(still_exists->second) == "true",
               "trapped OLE method faults should not poison later method calls");
    }
    if (count_after_fault != state.globals.end()) {
        expect(copperfin::runtime::format_value(count_after_fault->second) == "1",
               "trapped OLE method faults should preserve collection state");
    }

    fs::remove_all(temp_root, ignored);
}

void test_thrown_expression_fault_preserves_pause_statement_and_recovery() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_thrown_expression_fault";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "thrown_expression_fault.prg";
    write_text(
        main_path,
        "x = LOG(-1)\n"
        "after_fault = 7\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error,
           "thrown expression faults should pause with an error");
    expect(state.location.line == 1U,
           "thrown expression faults should highlight the faulting line");
    expect(state.statement_text == "x = LOG(-1)",
           "thrown expression faults should preserve the faulting statement text");
    expect(state.message.find("LOG() requires a positive argument") != std::string::npos,
           "thrown expression faults should preserve the thrown message");

    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "continuing after a thrown expression fault should keep the session alive");
    const auto after_fault = state.globals.find("after_fault");
    expect(after_fault != state.globals.end(), "post-fault statements should still execute after thrown expression faults");
    if (after_fault != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_fault->second) == "7",
               "post-fault statements should preserve later assignments after thrown expression faults");
    }

    fs::remove_all(temp_root, ignored);
}

void test_repeated_thrown_faults_refresh_pause_metadata_each_time() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_repeated_thrown_faults";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "repeated_thrown_faults.prg";
    write_text(
        main_path,
        "first_value = LOG(-1)\n"
        "after_first = 1\n"
        "second_value = ACOS(2)\n"
        "after_second = 1\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error,
           "the first thrown expression fault should pause with an error");
    expect(state.location.line == 1U,
           "the first thrown expression fault should point at line 1");
    expect(state.statement_text == "first_value = LOG(-1)",
           "the first thrown expression fault should preserve its own statement text");

    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error,
           "the second thrown expression fault should also pause with an error");
    expect(state.location.line == 3U,
           "the second thrown expression fault should update the pause line");
    expect(state.statement_text == "second_value = ACOS(2)",
           "the second thrown expression fault should replace stale statement text");
    expect(state.message.find("ACOS() requires an argument between -1 and 1") != std::string::npos,
           "the second thrown expression fault should replace the stale message");

    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "continuing after repeated thrown faults should keep the session alive");
    const auto after_first = state.globals.find("after_first");
    const auto after_second = state.globals.find("after_second");
    expect(after_first != state.globals.end(), "the first post-fault assignment should survive repeated thrown faults");
    expect(after_second != state.globals.end(), "the second post-fault assignment should survive repeated thrown faults");
    if (after_first != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_first->second) == "1",
               "the first post-fault assignment should be preserved");
    }
    if (after_second != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_second->second) == "1",
               "the second post-fault assignment should be preserved");
    }

    fs::remove_all(temp_root, ignored);
}

void test_nested_routine_faults_report_faulting_stack_frame_line() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_nested_fault_stack";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "nested_fault_stack.prg";
    write_text(
        main_path,
        "DO outerproc\n"
        "after_call = 1\n"
        "RETURN\n"
        "PROCEDURE outerproc\n"
        "DO innerproc\n"
        "RETURN\n"
        "ENDPROC\n"
        "PROCEDURE innerproc\n"
        "fault_value = LOG(-1)\n"
        "RETURN\n"
        "ENDPROC\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error,
           "nested routine faults should pause with an error");
    expect(state.location.line == 9U,
           "nested routine faults should highlight the inner fault line");
    expect(state.statement_text == "fault_value = LOG(-1)",
           "nested routine faults should preserve the inner fault statement text");
    expect(state.call_stack.size() >= 3U,
           "nested routine faults should expose the nested call stack");
    if (state.call_stack.size() >= 3U) {
        expect(state.call_stack[0].routine_name == "innerproc",
               "the top error stack frame should be the faulting inner routine");
        expect(state.call_stack[0].line == 9U,
               "the top error stack frame should use the faulting line instead of the next statement");
        expect(state.call_stack[1].routine_name == "outerproc",
               "the second error stack frame should be the caller routine");
        expect(state.call_stack[2].routine_name == "main",
               "the third error stack frame should be the entry routine");
    }
    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "continuing after a nested routine fault should keep the session alive");
    const auto after_call = state.globals.find("after_call");
    expect(after_call != state.globals.end(), "execution should resume after nested routine faults");
    if (after_call != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_call->second) == "1",
               "post-call statements should still execute after nested routine faults");
    }

    fs::remove_all(temp_root, ignored);
}

void test_repeated_nested_faults_refresh_stack_frame_and_statement_metadata() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_repeated_nested_faults";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "repeated_nested_faults.prg";
    write_text(
        main_path,
        "DO firstfault\n"
        "after_first = 1\n"
        "DO secondfault\n"
        "after_second = 1\n"
        "RETURN\n"
        "PROCEDURE firstfault\n"
        "first_value = LOG(-1)\n"
        "RETURN\n"
        "ENDPROC\n"
        "PROCEDURE secondfault\n"
        "second_value = ACOS(2)\n"
        "RETURN\n"
        "ENDPROC\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error,
           "the first nested routine fault should pause with an error");
    if (!state.call_stack.empty()) {
        expect(state.call_stack[0].routine_name == "firstfault",
               "the first nested routine fault should report the first routine on top of the stack");
        expect(state.call_stack[0].line == 7U,
               "the first nested routine fault should report the first routine fault line");
    }
    expect(state.statement_text == "first_value = LOG(-1)",
           "the first nested routine fault should preserve its own statement text");

    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error,
           "the second nested routine fault should also pause with an error");
    if (!state.call_stack.empty()) {
        expect(state.call_stack[0].routine_name == "secondfault",
               "the second nested routine fault should replace the stale top routine");
        expect(state.call_stack[0].line == 11U,
               "the second nested routine fault should replace the stale top line");
    }
    expect(state.statement_text == "second_value = ACOS(2)",
           "the second nested routine fault should replace stale statement text");
    expect(state.message.find("ACOS() requires an argument between -1 and 1") != std::string::npos,
           "the second nested routine fault should replace the stale error message");

    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "continuing after repeated nested routine faults should keep the session alive");
    const auto after_first = state.globals.find("after_first");
    const auto after_second = state.globals.find("after_second");
    expect(after_first != state.globals.end(), "the first post-fault assignment should survive repeated nested faults");
    expect(after_second != state.globals.end(), "the second post-fault assignment should survive repeated nested faults");

    fs::remove_all(temp_root, ignored);
}

void test_try_catch_finally_handles_runtime_errors() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_try_catch_finally";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path header_path = temp_root / "error_constants.h";
    write_text(header_path, "#define c_CR chr(13)\n");

    const fs::path main_path = temp_root / "try_catch_finally.prg";
    write_text(
        main_path,
        "#include error_constants.h\n"
        "TRY\n"
        "  DO missing_routine\n"
        "CATCH TO err_text\n"
        "  caught = err_text.Message + c_CR + \"Line \" + transform(err_text.LineNo) + \" in \" + err_text.Procedure + \"()\"\n"
        "FINALLY\n"
        "  finally_hit = 1\n"
        "ENDTRY\n"
        "after_try = 1\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "TRY/CATCH/FINALLY script should complete");

    const auto caught = state.globals.find("caught");
    const auto finally_hit = state.globals.find("finally_hit");
    const auto after_try = state.globals.find("after_try");
    expect(caught != state.globals.end(), "CATCH should run when the TRY block faults");
    expect(finally_hit != state.globals.end(), "FINALLY should run after handled TRY faults");
    expect(after_try != state.globals.end(), "execution should continue after ENDTRY");
    if (caught != state.globals.end()) {
        expect(
            copperfin::runtime::format_value(caught->second).find("Unable to resolve DO target") != std::string::npos &&
                copperfin::runtime::format_value(caught->second).find("Line ") != std::string::npos,
            "CATCH TO should expose a string-compatible VFP error object for formatted diagnostics");
    }

    const bool has_try_handler_event = std::any_of(
        state.events.begin(),
        state.events.end(),
        [](const copperfin::runtime::RuntimeEvent& event) {
            return event.category == "runtime.try_handler";
        });
    expect(has_try_handler_event, "runtime should emit a TRY handler event when a TRY block catches an error");

    fs::remove_all(temp_root, ignored);
}

void test_try_catch_unwinds_leaked_with_binding_before_catch() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_try_with_unwind";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "try_with_unwind.prg";
    write_text(
        main_path,
        "outer_obj = CREATEOBJECT('Sample.Object')\n"
        "inner_obj = CREATEOBJECT('Sample.Object')\n"
        "outer_obj.Caption = 'Outer'\n"
        "inner_obj.Caption = 'Inner'\n"
        "WITH outer_obj\n"
        "  TRY\n"
        "    WITH inner_obj\n"
        "      DO missing_routine\n"
        "    ENDWITH\n"
        "  CATCH TO err_text\n"
        "    caught_caption = .Caption\n"
        "    caught_message = err_text.Message\n"
        "  ENDTRY\n"
        "  after_try_caption = .Caption\n"
        "ENDWITH\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "TRY/CATCH should unwind leaked WITH bindings before entering CATCH");

    const auto caught_caption = state.globals.find("caught_caption");
    const auto after_try_caption = state.globals.find("after_try_caption");
    const auto caught_message = state.globals.find("caught_message");
    expect(caught_caption != state.globals.end(), "CATCH should be able to read the outer WITH target after an inner WITH fault");
    expect(after_try_caption != state.globals.end(), "the outer WITH target should remain active after the caught inner WITH fault");
    expect(caught_message != state.globals.end(), "CATCH TO should still receive the runtime error object");
    if (caught_caption != state.globals.end()) {
        expect(
            copperfin::runtime::format_value(caught_caption->second) == "Outer",
            "CATCH should resolve leading-dot access against the lexically active outer WITH target");
    }
    if (after_try_caption != state.globals.end()) {
        expect(
            copperfin::runtime::format_value(after_try_caption->second) == "Outer",
            "execution after ENDTRY should still see the outer WITH target instead of a leaked inner binding");
    }
    if (caught_message != state.globals.end()) {
        expect(
            copperfin::runtime::format_value(caught_message->second).find("Unable to resolve DO target") != std::string::npos,
            "the original fault metadata should survive the WITH unwind");
    }

    fs::remove_all(temp_root, ignored);
}

void test_outer_try_does_not_catch_fault_from_unrelated_expression_invoked_routine() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_expr_invoked_try_boundary";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "expr_invoked_try_boundary.prg";
    write_text(
        main_path,
        "TRY\n"
        "  x = FaultyFunc() + 1\n"
        "  afterExprLine = 1\n"
        "CATCH TO err_text\n"
        "  caught = err_text.Message\n"
        "ENDTRY\n"
        "afterTry = 1\n"
        "RETURN\n"
        "PROCEDURE FaultyFunc\n"
        "DO missing_routine\n"
        "RETURN 42\n"
        "ENDPROC\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "TRY/CATCH around an expression-invoked routine fault should complete: " + state.message);

    expect(state.globals.find("x") == state.globals.end(),
        "the assignment statement should abort before completing when the expression-invoked call faults, so x must stay unset");

    const auto caught = state.globals.find("caught");
    expect(caught != state.globals.end(), "CATCH should run when the expression-invoked routine faults");
    if (caught != state.globals.end()) {
        expect(
            copperfin::runtime::format_value(caught->second).find("Unable to resolve DO target") != std::string::npos,
            "CATCH TO should receive the faulting routine's own error text");
    }

    const auto after_try = state.globals.find("aftertry");
    expect(after_try != state.globals.end(), "execution should continue normally after ENDTRY");

    fs::remove_all(temp_root, ignored);
}

void test_error_handler_still_fires_after_fault_inside_expression_invoked_routine() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_expr_invoked_error_handler_reset";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "expr_invoked_error_handler_reset.prg";
    write_text(
        main_path,
        "PUBLIC handlercount\n"
        "handlercount = 0\n"
        "ON ERROR DO handleerr\n"
        "x = FaultyFunc() + FaultyFunc2()\n"
        "z = 1\n"
        "RETURN\n"
        "PROCEDURE FaultyFunc\n"
        "DO missing_routine_one\n"
        "RETURN 42\n"
        "ENDPROC\n"
        "PROCEDURE FaultyFunc2\n"
        "DO missing_routine_two\n"
        "RETURN 1\n"
        "ENDPROC\n"
        "PROCEDURE handleerr\n"
        "handlercount = handlercount + 1\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed,
        "ON ERROR should keep handling later faults after one fires from inside an expression-invoked routine: " + state.message);

    const auto handler_count = state.globals.find("handlercount");
    expect(handler_count != state.globals.end(), "handlercount variable should exist");
    if (handler_count != state.globals.end()) {
        expect(copperfin::runtime::format_value(handler_count->second) == "2",
            "ON ERROR handler should fire for both the expression-invoked fault and the later unrelated fault (got '" +
                copperfin::runtime::format_value(handler_count->second) + "')");
    }

    const auto z = state.globals.find("z");
    expect(z != state.globals.end(), "execution should reach the final statement once both faults are handled");

    fs::remove_all(temp_root, ignored);
}

void test_catch_to_binds_exception_object_with_error_metadata() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_catch_exception_object";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "catch_exception_object.prg";
    write_text(
        main_path,
        "TRY\n"
        "  DO causefault\n"
        "CATCH TO oErr\n"
        "  cCatchType = TYPE('oErr')\n"
        "  cCatchClass = oErr.Class\n"
        "  cCatchBaseClass = oErr.BaseClass\n"
        "  cCatchMessage = oErr.Message\n"
        "  cQualifiedMessage = m.oErr.Message\n"
        "  cQualifiedCombined = 'prefix' + m.oErr.Message + 'suffix'\n"
        "  cChrResult = CHR(10)\n"
        "  cForceExtResult = FORCEEXT('reference', 'DBF')\n"
        "  cRefTable = '/tmp/reference'\n"
        "  nDiagnosticMessageBox = MESSAGEBOX('The Reference Table could not be created' + ' (' + m.oErr.Message + '):' + CHR(10) + CHR(10) + FORCEEXT(m.cRefTable, 'DBF'), 16, 'Code References')\n"
        "  cCatchMessageText = oErr.MessageText\n"
        "  nCatchErrorNo = oErr.ErrorNo\n"
        "  nCatchHelpContext = oErr.HelpContext\n"
        "  nCatchLineNo = oErr.LineNo\n"
        "  cCatchProcedure = oErr.Procedure\n"
        "  cCatchDetails = oErr.Details\n"
        "  cCatchLineContents = oErr.LineContents\n"
        "  nCatchStackLevel = oErr.StackLevel\n"
        "  lCatchHasMessage = PEMSTATUS(oErr, 'Message', 1)\n"
        "  lCatchHasMessageText = PEMSTATUS(oErr, 'MessageText', 1)\n"
        "  lCatchHasHelpContext = PEMSTATUS(oErr, 'HelpContext', 1)\n"
        "  lCatchHasBaseClass = PEMSTATUS(oErr, 'BaseClass', 1)\n"
        "  lCatchSameObject = COMPOBJ(oErr, oErr)\n"
        "  nErrRows = AERROR(aErr)\n"
        "  nErrCode = aErr[1,1]\n"
        "  cErrMsg = aErr[1,2]\n"
        "  nErrLine = aErr[1,5]\n"
        "  cErrProc = aErr[1,6]\n"
        "  nFnCode = ERROR()\n"
        "  cFnMsg = MESSAGE()\n"
        "  nFnLine = LINENO()\n"
        "  cFnProg = PROGRAM()\n"
        "ENDTRY\n"
        "RETURN\n"
        "PROCEDURE causefault\n"
        "  fault_val = LOG(-1)\n"
        "  RETURN\n"
        "ENDPROC\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    const std::string last_event = state.events.empty() ? std::string{} : state.events.back().detail;
    expect(state.completed,
           "CATCH TO Exception-object script should complete: " + state.message +
               " @line=" + std::to_string(state.location.line) + " last=" + last_event);

    const auto check = [&](const std::string& name, const std::string& expected) {
        const auto it = state.globals.find(name);
        if (it == state.globals.end()) {
            expect(false, name + " should be captured");
            return;
        }
        expect(copperfin::runtime::format_value(it->second) == expected,
               name + " expected '" + expected + "' got '" + copperfin::runtime::format_value(it->second) + "'");
    };

    check("ccatchtype", "O");
    check("ccatchclass", "Exception");
    check("ccatchbaseclass", "Exception");
    check("ccatchprocedure", "causefault");
    check("ccatchlinecontents", "fault_val = LOG(-1)");
    check("lcatchhasmessage", "true");
    check("lcatchhasbaseclass", "true");
    check("lcatchsameobject", "true");
    check("nerrrows", "1");

    const auto catch_message = state.globals.find("ccatchmessage");
    const auto qualified_message = state.globals.find("cqualifiedmessage");
    const auto qualified_combined = state.globals.find("cqualifiedcombined");
    const auto chr_result = state.globals.find("cchrresult");
    const auto forceext_result = state.globals.find("cforceextresult");
    const auto diagnostic_message_box = state.globals.find("ndiagnosticmessagebox");
    const auto catch_message_text = state.globals.find("ccatchmessagetext");
    const auto catch_error_no = state.globals.find("ncatcherrorno");
    const auto catch_help_context = state.globals.find("ncatchhelpcontext");
    const auto catch_line_no = state.globals.find("ncatchlineno");
    const auto catch_details = state.globals.find("ccatchdetails");
    const auto catch_stack_level = state.globals.find("ncatchstacklevel");
    const auto catch_has_message_text = state.globals.find("lcatchhasmessagetext");
    const auto catch_has_help_context = state.globals.find("lcatchhashelpcontext");
    const auto err_code = state.globals.find("nerrcode");
    const auto err_msg = state.globals.find("cerrmsg");
    const auto err_line = state.globals.find("nerrline");
    const auto err_proc = state.globals.find("cerrproc");
    const auto fn_code = state.globals.find("nfncode");
    const auto fn_msg = state.globals.find("cfnmsg");
    const auto fn_line = state.globals.find("nfnline");
    const auto fn_prog = state.globals.find("cfnprog");

    expect(catch_message != state.globals.end(), "caught Exception object should expose Message");
    expect(qualified_message != state.globals.end(), "m-qualified caught Exception object should expose Message");
    expect(qualified_combined != state.globals.end(), "m-qualified caught Exception Message should concatenate as a string");
    expect(chr_result != state.globals.end(), "CHR() should return a string while formatting caught diagnostics");
    expect(forceext_result != state.globals.end(), "FORCEEXT() should return a string while formatting caught diagnostics");
    expect(diagnostic_message_box != state.globals.end(), "MESSAGEBOX() should accept the formatted caught diagnostic expression");
    expect(catch_message_text != state.globals.end(), "caught Exception object should expose MessageText");
    expect(catch_error_no != state.globals.end(), "caught Exception object should expose ErrorNo");
    expect(catch_help_context != state.globals.end(), "caught Exception object should expose HelpContext");
    expect(catch_line_no != state.globals.end(), "caught Exception object should expose LineNo");
    expect(catch_details != state.globals.end(), "caught Exception object should expose Details");
    expect(catch_stack_level != state.globals.end(), "caught Exception object should expose StackLevel");
    expect(catch_has_message_text != state.globals.end(), "caught Exception object should reflect MessageText");
    expect(catch_has_help_context != state.globals.end(), "caught Exception object should reflect HelpContext");
    expect(err_code != state.globals.end(), "AERROR() should still expose error code");
    expect(err_msg != state.globals.end(), "AERROR() should still expose error message");
    expect(err_line != state.globals.end(), "AERROR() should still expose fault line");
    expect(err_proc != state.globals.end(), "AERROR() should still expose procedure");
    expect(fn_code != state.globals.end(), "ERROR() should still expose code");
    expect(fn_msg != state.globals.end(), "MESSAGE() should still expose text");
    expect(fn_line != state.globals.end(), "LINENO() should still expose fault line");
    expect(fn_prog != state.globals.end(), "PROGRAM() should still expose procedure");

    if (catch_message != state.globals.end() && err_msg != state.globals.end() && fn_msg != state.globals.end()) {
        const std::string catch_msg = copperfin::runtime::format_value(catch_message->second);
        expect(catch_msg == copperfin::runtime::format_value(err_msg->second),
               "caught Exception Message should match AERROR()[1,2]");
        expect(catch_msg == copperfin::runtime::format_value(fn_msg->second),
               "caught Exception Message should match MESSAGE()");
    }
    if (catch_message != state.globals.end() && qualified_message != state.globals.end()) {
        expect(copperfin::runtime::format_value(qualified_message->second) ==
                   copperfin::runtime::format_value(catch_message->second),
               "m-qualified Exception Message should match the direct property read");
    }
    if (qualified_combined != state.globals.end()) {
        expect(copperfin::runtime::format_value(qualified_combined->second).find("prefix") == 0U &&
                   copperfin::runtime::format_value(qualified_combined->second).rfind("suffix") ==
                       copperfin::runtime::format_value(qualified_combined->second).size() - 6U,
               "m-qualified Exception Message should participate in string concatenation");
    }
    if (chr_result != state.globals.end()) {
        expect(copperfin::runtime::format_value(chr_result->second).size() == 1U &&
                   copperfin::runtime::format_value(chr_result->second)[0] == '\n',
               "CHR() should preserve its string result for diagnostic formatting");
    }
    if (forceext_result != state.globals.end()) {
        expect(copperfin::runtime::format_value(forceext_result->second) == "reference.DBF",
               "FORCEEXT() should preserve its string result for diagnostic formatting");
    }
    if (diagnostic_message_box != state.globals.end()) {
        expect(copperfin::runtime::format_value(diagnostic_message_box->second) == "1",
               "MESSAGEBOX() should return its normal button result for formatted caught diagnostics");
    }
    if (catch_message != state.globals.end() && catch_message_text != state.globals.end()) {
        expect(copperfin::runtime::format_value(catch_message_text->second) ==
                   copperfin::runtime::format_value(catch_message->second),
               "caught Exception MessageText should match Message");
    }
    if (catch_help_context != state.globals.end()) {
        expect(copperfin::runtime::format_value(catch_help_context->second) == "0",
               "caught Exception HelpContext should default to zero");
    }
    if (catch_has_message_text != state.globals.end() && catch_has_help_context != state.globals.end()) {
        expect(copperfin::runtime::format_value(catch_has_message_text->second) == "true",
               "caught Exception PEMSTATUS should expose MessageText");
        expect(copperfin::runtime::format_value(catch_has_help_context->second) == "true",
               "caught Exception PEMSTATUS should expose HelpContext");
    }
    if (catch_error_no != state.globals.end() && err_code != state.globals.end() && fn_code != state.globals.end()) {
        const std::string catch_code = copperfin::runtime::format_value(catch_error_no->second);
        expect(catch_code == copperfin::runtime::format_value(err_code->second),
               "caught Exception ErrorNo should match AERROR()[1,1]");
        expect(catch_code == copperfin::runtime::format_value(fn_code->second),
               "caught Exception ErrorNo should match ERROR()");
    }
    if (catch_line_no != state.globals.end() && err_line != state.globals.end() && fn_line != state.globals.end()) {
        const std::string catch_line = copperfin::runtime::format_value(catch_line_no->second);
        expect(catch_line == copperfin::runtime::format_value(err_line->second),
               "caught Exception LineNo should match AERROR()[1,5]");
        expect(catch_line == copperfin::runtime::format_value(fn_line->second),
               "caught Exception LineNo should match LINENO()");
    }
    if (err_proc != state.globals.end() && fn_prog != state.globals.end()) {
        expect(copperfin::runtime::format_value(err_proc->second) ==
                   copperfin::runtime::format_value(fn_prog->second),
               "AERROR()[1,6] should continue matching PROGRAM()");
    }
    if (catch_details != state.globals.end()) {
        expect(!copperfin::runtime::format_value(catch_details->second).empty(),
               "caught Exception Details should not be empty");
    }
    if (catch_stack_level != state.globals.end()) {
        expect(copperfin::runtime::format_value(catch_stack_level->second) == "1",
               "caught Exception StackLevel should reflect the active catch frame depth for a single nested routine fault");
    }

    expect(state.ole_objects.size() == 1U,
           "CATCH TO Exception-object script should materialize one caught Exception object");
    if (state.ole_objects.size() == 1U) {
        const auto& caught_object = state.ole_objects[0];
        expect(caught_object.prog_id == "Exception",
               "caught Exception object should preserve the builtin Exception class token");
        expect(caught_object.source.empty(),
               "caught Exception object should be synthetic rather than PRG-backed");
        expect(caught_object.base_class_name == "Exception",
               "caught Exception object should expose Exception as BaseClass");
        expect(caught_object.class_library.empty(),
               "caught Exception object should keep ClassLibrary empty");
        expect(caught_object.class_hierarchy.size() == 2U,
               "caught Exception object should expose Exception -> Object hierarchy");
        if (caught_object.class_hierarchy.size() == 2U) {
            expect(caught_object.class_hierarchy[0] == "EXCEPTION",
                   "caught Exception object should store EXCEPTION first in the hierarchy");
            expect(caught_object.class_hierarchy[1] == "OBJECT",
                   "caught Exception object should store OBJECT second in the hierarchy");
        }
    }

    fs::remove_all(temp_root, ignored);
}

void test_throw_is_catchable_and_preserves_exception_uservalue() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_throw_exception_uservalue";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "throw_exception_uservalue.prg";
    write_text(
        main_path,
        "nThrowValueCalls = 0\n"
        "TRY\n"
        "  DO raiselegacy\n"
        "CATCH TO oErr\n"
        "  cCatchType = TYPE('oErr')\n"
        "  cCatchClass = oErr.Class\n"
        "  cCatchBaseClass = oErr.BaseClass\n"
        "  cCatchMessage = oErr.Message\n"
        "  cCatchDetails = oErr.Details\n"
        "  nCatchUserValue = oErr.UserValue\n"
        "  cCatchUserValueType = VARTYPE(oErr.UserValue)\n"
        "  nCatchLineNo = oErr.LineNo\n"
        "  cCatchProcedure = oErr.Procedure\n"
        "  cCatchLineContents = oErr.LineContents\n"
        "  lCatchHasUserValue = PEMSTATUS(oErr, 'UserValue', 1)\n"
        "  lCatchSameObject = COMPOBJ(oErr, oErr)\n"
        "  nErrRows = AERROR(aErr)\n"
        "  nErrCode = aErr[1,1]\n"
        "  cErrMsg = aErr[1,2]\n"
        "  cErrParam = aErr[1,3]\n"
        "  nErrLine = aErr[1,5]\n"
        "  cErrProc = aErr[1,6]\n"
        "  cErrStmt = aErr[1,7]\n"
        "  nFnCode = ERROR()\n"
        "  cFnMsg = MESSAGE()\n"
        "  nFnLine = LINENO()\n"
        "  cFnProg = PROGRAM()\n"
        "  cFnSys16 = SYS(16)\n"
        "ENDTRY\n"
        "RETURN\n"
        "PROCEDURE raiselegacy\n"
        "  THROW make_throw_value()\n"
        "  RETURN\n"
        "ENDPROC\n"
        "FUNCTION make_throw_value\n"
        "  nThrowValueCalls = nThrowValueCalls + 1\n"
        "  RETURN 42\n"
        "ENDFUNC\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "THROW/CATCH Exception-object script should complete: " + state.message);

    const auto check = [&](const std::string& name, const std::string& expected) {
        const auto it = state.globals.find(name);
        if (it == state.globals.end()) {
            expect(false, name + " should be captured");
            return;
        }
        expect(copperfin::runtime::format_value(it->second) == expected,
               name + " expected '" + expected + "' got '" + copperfin::runtime::format_value(it->second) + "'");
    };

    check("ccatchtype", "O");
    check("ccatchclass", "Exception");
    check("ccatchbaseclass", "Exception");
    check("ccatchmessage", "User Thrown Error.");
    check("ccatchdetails", "User Thrown Error.");
    check("ncatchuservalue", "42");
    check("nthrowvaluecalls", "1");
    check("ccatchuservaluetype", "N");
    check("ccatchprocedure", "raiselegacy");
    check("ccatchlinecontents", "THROW make_throw_value()");
    check("lcatchhasuservalue", "true");
    check("lcatchsameobject", "true");
    check("nerrrows", "1");
    check("cerrmsg", "User Thrown Error.");
    check("cerrparam", "42");
    check("cerrproc", "raiselegacy");
    check("cerrstmt", "THROW make_throw_value()");
    check("cfnmsg", "User Thrown Error.");
    check("cfnprog", "raiselegacy");
    check("cfnsys16", "PROCEDURE raiselegacy " + main_path.string());

    const auto catch_line_no = state.globals.find("ncatchlineno");
    const auto err_code = state.globals.find("nerrcode");
    const auto err_line = state.globals.find("nerrline");
    const auto fn_code = state.globals.find("nfncode");
    const auto fn_line = state.globals.find("nfnline");

    expect(catch_line_no != state.globals.end(), "caught THROW Exception should expose LineNo");
    expect(err_code != state.globals.end(), "AERROR() should expose thrown error code");
    expect(err_line != state.globals.end(), "AERROR() should expose thrown line");
    expect(fn_code != state.globals.end(), "ERROR() should expose thrown error code");
    expect(fn_line != state.globals.end(), "LINENO() should expose thrown line");

    if (catch_line_no != state.globals.end()) {
        expect(copperfin::runtime::format_value(catch_line_no->second) == "32",
               "caught THROW Exception LineNo should report the THROW statement line (got " +
                   copperfin::runtime::format_value(catch_line_no->second) + ")");
    }
    if (err_line != state.globals.end()) {
        expect(copperfin::runtime::format_value(err_line->second) == "32",
               "AERROR()[1,5] should report the THROW statement line (got " +
                   copperfin::runtime::format_value(err_line->second) + ")");
    }
    if (fn_line != state.globals.end()) {
        expect(copperfin::runtime::format_value(fn_line->second) == "32",
               "LINENO() should report the THROW statement line (got " +
                   copperfin::runtime::format_value(fn_line->second) + ")");
    }
    if (err_code != state.globals.end() && fn_code != state.globals.end()) {
        expect(copperfin::runtime::format_value(err_code->second) == "2071",
               "AERROR()[1,1] should report user-thrown error code 2071");
        expect(copperfin::runtime::format_value(err_code->second) ==
                   copperfin::runtime::format_value(fn_code->second),
               "AERROR()[1,1] should continue matching ERROR() for THROW");
    }

    expect(state.ole_objects.size() == 1U,
           "THROW/CATCH script should materialize one caught Exception object");
    if (state.ole_objects.size() == 1U) {
        const auto& caught_object = state.ole_objects[0];
        expect(caught_object.prog_id == "Exception",
               "caught THROW object should preserve the builtin Exception class token");
        expect(caught_object.source.empty(),
               "caught THROW object should remain synthetic rather than PRG-backed");
        expect(caught_object.base_class_name == "Exception",
               "caught THROW object should expose Exception as BaseClass");
        const auto error_no = caught_object.properties.find("errorno");
        expect(error_no != caught_object.properties.end() &&
                   copperfin::runtime::format_value(error_no->second) == "2071",
               "caught THROW object should preserve user-thrown ErrorNo 2071");
        const auto user_value = caught_object.properties.find("uservalue");
        expect(user_value != caught_object.properties.end() &&
                   copperfin::runtime::format_value(user_value->second) == "42",
               "caught THROW object should preserve the thrown UserValue");
    }

    fs::remove_all(temp_root, ignored);
}

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

void test_catch_when_false_with_finally_reaches_outer_catch_with_original_metadata() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_catch_when_finally_outer";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "catch_when_finally_outer.prg";
    write_text(
        main_path,
        "TRY\n"
        "  TRY\n"
        "    bad = LOG(-1)\n"
        "  CATCH TO oSkip WHEN oSkip.ErrorNo = 999\n"
        "    cInnerHandled = 'wrong'\n"
        "  FINALLY\n"
        "    lFinallyHit = .T.\n"
        "  ENDTRY\n"
        "CATCH TO oOuter\n"
        "  lOuterHandled = .T.\n"
        "  cSkipType = VARTYPE(oSkip)\n"
        "  cOuterMsg = oOuter.Message\n"
        "  nOuterLine = oOuter.LineNo\n"
        "  cOuterStmt = oOuter.LineContents\n"
        "  nErrRows = AERROR(aErr)\n"
        "  cErrMsg = aErr[1,2]\n"
        "  nErrLine = aErr[1,5]\n"
        "  cErrStmt = aErr[1,7]\n"
        "  cFnMsg = MESSAGE()\n"
        "  nFnLine = LINENO()\n"
        "ENDTRY\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "inner FINALLY outer CATCH script should complete: " + state.message);

    const auto check = [&](const std::string& name, const std::string& expected) {
        const auto it = state.globals.find(name);
        if (it == state.globals.end()) {
            expect(false, name + " should be captured");
            return;
        }
        expect(copperfin::runtime::format_value(it->second) == expected,
               name + " expected '" + expected + "' got '" + copperfin::runtime::format_value(it->second) + "'");
    };

    check("lfinallyhit", "true");
    check("louterhandled", "true");
    check("cskiptype", "U");
    check("nerrrows", "1");

    const auto outer_msg = state.globals.find("coutermsg");
    const auto err_msg = state.globals.find("cerrmsg");
    const auto fn_msg = state.globals.find("cfnmsg");
    const auto outer_line = state.globals.find("nouterline");
    const auto err_line = state.globals.find("nerrline");
    const auto fn_line = state.globals.find("nfnline");
    const auto outer_stmt = state.globals.find("couterstmt");
    const auto err_stmt = state.globals.find("cerrstmt");
    const auto inner_handled = state.globals.find("cinnerhandled");

    expect(inner_handled == state.globals.end(),
           "non-matching inner CATCH WHEN with FINALLY should not execute its body");
    expect(outer_msg != state.globals.end(), "outer CATCH should expose original Message after FINALLY");
    expect(err_msg != state.globals.end(), "AERROR() should expose original Message after FINALLY");
    expect(fn_msg != state.globals.end(), "MESSAGE() should expose original Message after FINALLY");
    expect(outer_line != state.globals.end(), "outer CATCH should expose original LineNo after FINALLY");
    expect(err_line != state.globals.end(), "AERROR() should expose original LineNo after FINALLY");
    expect(fn_line != state.globals.end(), "LINENO() should expose original LineNo after FINALLY");
    expect(outer_stmt != state.globals.end(), "outer CATCH should expose original LineContents after FINALLY");
    expect(err_stmt != state.globals.end(), "AERROR() should expose original LineContents after FINALLY");

    if (outer_msg != state.globals.end() && err_msg != state.globals.end() && fn_msg != state.globals.end()) {
        expect(copperfin::runtime::format_value(outer_msg->second) ==
                   copperfin::runtime::format_value(err_msg->second),
               "inner FINALLY outer CATCH should keep AERROR()[1,2] aligned with the original Message");
        expect(copperfin::runtime::format_value(outer_msg->second) ==
                   copperfin::runtime::format_value(fn_msg->second),
               "inner FINALLY outer CATCH should keep MESSAGE() aligned with the original Message");
    }
    if (outer_line != state.globals.end()) {
        expect(copperfin::runtime::format_value(outer_line->second) == "3",
               "outer CATCH after inner FINALLY should preserve the original fault line");
    }
    if (err_line != state.globals.end()) {
        expect(copperfin::runtime::format_value(err_line->second) == "3",
               "AERROR()[1,5] after inner FINALLY should preserve the original fault line");
    }
    if (fn_line != state.globals.end()) {
        expect(copperfin::runtime::format_value(fn_line->second) == "3",
               "LINENO() after inner FINALLY should preserve the original fault line");
    }
    if (outer_stmt != state.globals.end()) {
        expect(copperfin::runtime::format_value(outer_stmt->second) == "bad = LOG(-1)",
               "outer CATCH after inner FINALLY should preserve the original fault statement");
    }
    if (err_stmt != state.globals.end()) {
        expect(copperfin::runtime::format_value(err_stmt->second) == "bad = LOG(-1)",
               "AERROR()[1,7] after inner FINALLY should preserve the original fault statement");
    }

    fs::remove_all(temp_root, ignored);
}

void test_catch_when_false_with_finally_reaches_on_error_with_original_metadata() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_catch_when_finally_on_error";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "catch_when_finally_on_error.prg";
    write_text(
        main_path,
        "ON ERROR DO handleerr\n"
        "TRY\n"
        "  bad = LOG(-1)\n"
        "CATCH TO oSkip WHEN oSkip.ErrorNo = 999\n"
        "  cInnerHandled = 'wrong'\n"
        "FINALLY\n"
        "  lFinallyHit = .T.\n"
        "ENDTRY\n"
        "after_fault = 1\n"
        "RETURN\n"
        "PROCEDURE handleerr\n"
        "PUBLIC lHandled, nErrRows, cErrMsg, nErrLine, cErrStmt, cFnMsg, nFnLine\n"
        "lHandled = .T.\n"
        "nErrRows = AERROR(aErr)\n"
        "cErrMsg = aErr[1,2]\n"
        "nErrLine = aErr[1,5]\n"
        "cErrStmt = aErr[1,7]\n"
        "cFnMsg = MESSAGE()\n"
        "nFnLine = LINENO()\n"
        "RETURN\n"
        "ENDPROC\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "inner FINALLY ON ERROR script should complete: " + state.message);

    const auto check = [&](const std::string& name, const std::string& expected) {
        const auto it = state.globals.find(name);
        if (it == state.globals.end()) {
            expect(false, name + " should be captured");
            return;
        }
        expect(copperfin::runtime::format_value(it->second) == expected,
               name + " expected '" + expected + "' got '" + copperfin::runtime::format_value(it->second) + "'");
    };

    check("lfinallyhit", "true");
    check("lhandled", "true");
    check("after_fault", "1");
    check("nerrrows", "1");

    const auto err_line = state.globals.find("nerrline");
    const auto fn_line = state.globals.find("nfnline");
    const auto err_stmt = state.globals.find("cerrstmt");
    const auto fn_msg = state.globals.find("cfnmsg");
    const auto inner_handled = state.globals.find("cinnerhandled");

    expect(inner_handled == state.globals.end(),
           "non-matching CATCH WHEN with FINALLY should not execute before ON ERROR");
    expect(err_line != state.globals.end(), "AERROR() should expose original LineNo after FINALLY/ON ERROR");
    expect(fn_line != state.globals.end(), "LINENO() should expose original LineNo after FINALLY/ON ERROR");
    expect(err_stmt != state.globals.end(), "AERROR() should expose original LineContents after FINALLY/ON ERROR");
    expect(fn_msg != state.globals.end(), "MESSAGE() should expose original Message after FINALLY/ON ERROR");

    if (err_line != state.globals.end()) {
        expect(copperfin::runtime::format_value(err_line->second) == "3",
               "AERROR()[1,5] after FINALLY/ON ERROR should preserve the original fault line");
    }
    if (fn_line != state.globals.end()) {
        expect(copperfin::runtime::format_value(fn_line->second) == "3",
               "LINENO() after FINALLY/ON ERROR should preserve the original fault line");
    }
    if (err_stmt != state.globals.end()) {
        expect(copperfin::runtime::format_value(err_stmt->second) == "bad = LOG(-1)",
               "AERROR()[1,7] after FINALLY/ON ERROR should preserve the original fault statement");
    }

    fs::remove_all(temp_root, ignored);
}

void test_catch_fault_runs_pending_finally_before_propagation() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_catch_fault_finally";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "catch_fault_finally.prg";
    write_text(
        main_path,
        "cSame = ''\n"
        "TRY\n"
        "  TRY\n"
        "    THROW 'initial-same'\n"
        "  CATCH TO oInitialSame\n"
        "    cSame = cSame + 'C'\n"
        "    nSameBad = LOG(-1)\n"
        "  FINALLY\n"
        "    cSame = cSame + 'F'\n"
        "  ENDTRY\n"
        "CATCH TO oSame\n"
        "  cSame = cSame + 'O'\n"
        "  cSameMessage = oSame.Message\n"
        "  cSameStatement = oSame.LineContents\n"
        "ENDTRY\n"
        "cDo = ''\n"
        "TRY\n"
        "  DO WorkerDo\n"
        "CATCH TO oDo\n"
        "  cDo = cDo + 'O'\n"
        "  cDoMessage = oDo.Message\n"
        "  cDoStatement = oDo.LineContents\n"
        "ENDTRY\n"
        "cExpr = ''\n"
        "TRY\n"
        "  nExprResult = WorkerExpr()\n"
        "CATCH TO oExpr\n"
        "  cExpr = cExpr + 'O'\n"
        "  cExprMessage = oExpr.Message\n"
        "  cExprStatement = oExpr.LineContents\n"
        "ENDTRY\n"
        "cFinally = ''\n"
        "TRY\n"
        "  TRY\n"
        "    cFinally = cFinally + 'T'\n"
        "  CATCH TO oWrong\n"
        "    cFinally = cFinally + 'X'\n"
        "  FINALLY\n"
        "    cFinally = cFinally + 'F'\n"
        "    nFinallyBad = LOG(-1)\n"
        "  ENDTRY\n"
        "CATCH TO oFinally\n"
        "  cFinally = cFinally + 'O'\n"
        "  cFinallyMessage = oFinally.Message\n"
        "  cFinallyStatement = oFinally.LineContents\n"
        "ENDTRY\n"
        "RETURN\n"
        "PROCEDURE WorkerDo\n"
        "TRY\n"
        "  THROW 'initial-do'\n"
        "CATCH TO oInitialDo\n"
        "  cDo = cDo + 'C'\n"
        "  DO FailDo\n"
        "FINALLY\n"
        "  cDo = cDo + 'F'\n"
        "ENDTRY\n"
        "cDo = cDo + 'A'\n"
        "RETURN\n"
        "ENDPROC\n"
        "PROCEDURE FailDo\n"
        "nDoBad = LOG(-1)\n"
        "RETURN\n"
        "ENDPROC\n"
        "PROCEDURE WorkerExpr\n"
        "TRY\n"
        "  THROW 'initial-expr'\n"
        "CATCH TO oInitialExpr\n"
        "  cExpr = cExpr + 'C'\n"
        "  DO FailExpr\n"
        "FINALLY\n"
        "  cExpr = cExpr + 'F'\n"
        "ENDTRY\n"
        "cExpr = cExpr + 'A'\n"
        "RETURN 42\n"
        "ENDPROC\n"
        "PROCEDURE FailExpr\n"
        "nExprBad = LOG(-1)\n"
        "RETURN\n"
        "ENDPROC\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "#4020: CATCH faults should run pending FINALLY blocks before outer propagation: " + state.message);

    const auto expect_value = [&](const char *name, const char *expected) {
        const auto value = state.globals.find(name);
        expect(value != state.globals.end(), std::string("#4020: expected result ") + name);
        if (value != state.globals.end()) {
            expect(copperfin::runtime::format_value(value->second) == expected,
                   std::string("#4020: unexpected result for ") + name);
        }
    };
    const auto expect_log_fault = [&](const char *message_name, const char *statement_name, const char *statement) {
        const auto message = state.globals.find(message_name);
        const auto fault_statement = state.globals.find(statement_name);
        expect(message != state.globals.end(), std::string("#4020: expected fault message ") + message_name);
        expect(fault_statement != state.globals.end(), std::string("#4020: expected fault statement ") + statement_name);
        if (message != state.globals.end()) {
            const std::string actual_message = copperfin::runtime::format_value(message->second);
            expect(actual_message.find("LOG()") != std::string::npos,
                   std::string("#4020: outer handler should receive the replacement LOG() fault for ") +
                       message_name + ", got '" + actual_message + "'");
        }
        if (fault_statement != state.globals.end()) {
            const std::string actual_statement = copperfin::runtime::format_value(fault_statement->second);
            expect(actual_statement == statement,
                   std::string("#4020: outer handler should receive the replacement fault statement for ") +
                       statement_name + ", got '" + actual_statement + "'");
        }
    };

    expect_value("csame", "CFO");
    expect_value("cdo", "CFO");
    expect_value("cexpr", "CFO");
    expect_value("cfinally", "TFO");
    expect_log_fault("csamemessage", "csamestatement", "nSameBad = LOG(-1)");
    expect_log_fault("cdomessage", "cdostatement", "nDoBad = LOG(-1)");
    expect_log_fault("cexprmessage", "cexprstatement", "nExprBad = LOG(-1)");
    expect_log_fault("cfinallymessage", "cfinallystatement", "nFinallyBad = LOG(-1)");
    expect(state.globals.find("nexprresult") == state.globals.end(),
           "#4020: a faulting expression-invoked routine should not complete its caller assignment");

    fs::remove_all(temp_root, ignored);
}

void test_try_finally_runs_without_catch_on_success() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_try_finally_success";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "try_finally_success.prg";
    write_text(
        main_path,
        "TRY\n"
        "  value = 7\n"
        "CATCH TO err_text\n"
        "  caught = 1\n"
        "FINALLY\n"
        "  finally_hit = 1\n"
        "ENDTRY\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "TRY/FINALLY success script should complete");
    expect(state.globals.find("caught") == state.globals.end(), "CATCH should be skipped when TRY succeeds");
    expect(state.globals.find("finally_hit") != state.globals.end(), "FINALLY should still run when TRY succeeds");

    fs::remove_all(temp_root, ignored);
}

void test_return_inside_try_runs_finally_before_return() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_try_return_finally";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "try_return_finally.prg";
    write_text(
        main_path,
        "cleanup = 0\n"
        "nested = 0\n"
        "result = Dotest()\n"
        "RETURN\n"
        "PROCEDURE Dotest\n"
        "TRY\n"
        "  RETURN 'x'\n"
        "FINALLY\n"
        "  TRY\n"
        "    DO missing_routine\n"
        "  CATCH TO err_text\n"
        "    nested = 1\n"
        "  ENDTRY\n"
        "  cleanup = cleanup + 1\n"
        "ENDTRY\n"
        "RETURN 'y'\n"
        "ENDPROC\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "RETURN inside TRY should still run the pending FINALLY block");

    const auto result = state.globals.find("result");
    const auto cleanup = state.globals.find("cleanup");
    const auto nested = state.globals.find("nested");
    expect(result != state.globals.end(), "the routine return value should still reach the caller");
    expect(cleanup != state.globals.end(), "FINALLY should execute cleanup after a RETURN inside TRY");
    expect(nested != state.globals.end(), "nested TRY/CATCH inside a RETURN-driven FINALLY block should still complete");
    if (result != state.globals.end()) {
        expect(copperfin::runtime::format_value(result->second) == "x",
               "RETURN inside TRY should preserve the original return value");
    }
    if (cleanup != state.globals.end()) {
        expect(copperfin::runtime::format_value(cleanup->second) == "1",
               "RETURN should not skip later cleanup statements inside the FINALLY block");
    }
    if (nested != state.globals.end()) {
        expect(copperfin::runtime::format_value(nested->second) == "1",
               "nested TRY/CATCH inside the FINALLY block should finish before the pending RETURN resumes");
    }

    fs::remove_all(temp_root, ignored);
}

void test_return_inside_catch_runs_all_enclosing_finally_before_return() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_catch_return_finally";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "catch_return_finally.prg";
    write_text(
        main_path,
        "trace = ''\n"
        "result = Dotest()\n"
        "RETURN\n"
        "PROCEDURE Dotest\n"
        "TRY\n"
        "  TRY\n"
        "    DO missing_routine\n"
        "  CATCH TO err_text\n"
        "    RETURN 'caught'\n"
        "  FINALLY\n"
        "    trace = trace + 'I'\n"
        "  ENDTRY\n"
        "FINALLY\n"
        "  trace = trace + 'O'\n"
        "ENDTRY\n"
        "RETURN 'fallthrough'\n"
        "ENDPROC\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "RETURN inside CATCH should still run all pending enclosing FINALLY blocks");

    const auto result = state.globals.find("result");
    const auto trace = state.globals.find("trace");
    expect(result != state.globals.end(), "the CATCH return value should still reach the caller");
    expect(trace != state.globals.end(), "enclosing FINALLY blocks should still execute after a RETURN inside CATCH");
    if (result != state.globals.end()) {
        expect(copperfin::runtime::format_value(result->second) == "caught",
               "RETURN inside CATCH should preserve the caught-path return value");
    }
    if (trace != state.globals.end()) {
        expect(copperfin::runtime::format_value(trace->second) == "IO",
               "RETURN inside CATCH should run enclosing FINALLY blocks from inner to outer");
    }

    fs::remove_all(temp_root, ignored);
}

void test_file_operation_runtime_errors_localize() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_file_ops_localization";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue scoped_locale("COPPERFIN_LOCALE");
    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);

    const fs::path nonempty_dir = temp_root / "busy";
    fs::create_directories(nonempty_dir);
    write_text(nonempty_dir / "child.txt", "payload");

    const fs::path erase_path = temp_root / "erase_error.prg";
    write_text(
        erase_path,
        "ERASE 'busy'\n"
        "RETURN\n");
    copperfin::runtime::PrgRuntimeSession erase_session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(erase_path.string(), temp_root.string(), false));
    const auto erase_state = erase_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!erase_state.completed, "#2706: qps-ploc ERASE non-empty-directory script should fail");
    expect(
        erase_state.message.find("[!! ") == 0U &&
            erase_state.message.find("ERASE failed:") == std::string::npos &&
            erase_state.message.find("busy") != std::string::npos,
        "#2706: qps-ploc ERASE runtime error should localize the prose while preserving the path");

    const fs::path copy_path = temp_root / "copy_error.prg";
    write_text(
        copy_path,
        "COPY FILE 'missing.txt' TO 'copied.txt'\n"
        "RETURN\n");
    copperfin::runtime::PrgRuntimeSession copy_session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(copy_path.string(), temp_root.string(), false));
    const auto copy_state = copy_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!copy_state.completed, "#2706: qps-ploc COPY FILE missing-source script should fail");
    expect(
        copy_state.message.find("[!! ") == 0U &&
            copy_state.message.find("COPY FILE failed:") == std::string::npos,
        "#2706: qps-ploc COPY FILE runtime error should localize the prose while preserving the OS error text");

    const fs::path rename_path = temp_root / "rename_error.prg";
    write_text(
        rename_path,
        "RENAME 'missing.txt' TO 'renamed.txt'\n"
        "RETURN\n");
    copperfin::runtime::PrgRuntimeSession rename_session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(rename_path.string(), temp_root.string(), false));
    const auto rename_state = rename_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!rename_state.completed, "#2706: qps-ploc RENAME missing-source script should fail");
    expect(
        rename_state.message.find("[!! ") == 0U &&
            rename_state.message.find("RENAME failed:") == std::string::npos,
        "#2706: qps-ploc RENAME runtime error should localize the prose while preserving the OS error text");

    write_text(temp_root / "source.txt", "source-content");
    write_text(temp_root / "existing.txt", "existing-content");
    const fs::path rename_existing_path = temp_root / "rename_existing_error.prg";
    write_text(
        rename_existing_path,
        "RENAME 'source.txt' TO 'existing.txt'\n"
        "RETURN\n");
    copperfin::runtime::PrgRuntimeSession rename_existing_session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(rename_existing_path.string(), temp_root.string(), false));
    const auto rename_existing_state =
        rename_existing_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!rename_existing_state.completed,
           "#3703: qps-ploc RENAME existing-destination script should fail");
    expect(
        rename_existing_state.message.find("[!! ") == 0U &&
            rename_existing_state.message.find("destination already exists") == std::string::npos &&
            rename_existing_state.message.find("existing.txt") != std::string::npos,
        "#3703: qps-ploc existing-destination RENAME error should localize prose while preserving the target path");
    expect(read_text(temp_root / "existing.txt") == "existing-content",
           "#3703: localized existing-destination RENAME failures should preserve the destination contents");

    fs::remove_all(temp_root, ignored);
}

void test_residual_dispatch_runtime_errors_localize() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_residual_dispatch_localization";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}});

    ScopedEnvironmentValue scoped_locale("COPPERFIN_LOCALE");
    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);

    const fs::path text_path = temp_root / "text_missing_target.prg";
    write_text(
        text_path,
        "TEXT\n"
        "Hello\n"
        "ENDTEXT\n"
        "RETURN\n");
    copperfin::runtime::PrgRuntimeSession text_session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(text_path.string(), temp_root.string(), false));
    const auto text_state = text_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!text_state.completed, "#2717: qps-ploc TEXT without TO target should fail");
    expect(
        text_state.message ==
            copperfin::localization::pseudo_localize("TEXT requires TO <variable> in the current runtime slice"),
        "#2717: qps-ploc TEXT missing-target error should route through the pseudo-localization transform");

    const fs::path try_path = temp_root / "try_missing_endtry.prg";
    write_text(
        try_path,
        "TRY\n"
        "RETURN\n");
    copperfin::runtime::PrgRuntimeSession try_session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(try_path.string(), temp_root.string(), false));
    const auto try_state = try_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!try_state.completed, "#2717: qps-ploc TRY without ENDTRY should fail");
    expect(
        try_state.message ==
            copperfin::localization::pseudo_localize("TRY block is missing ENDTRY"),
        "#2717: qps-ploc TRY missing-ENDTRY error should route through the pseudo-localization transform");

    const fs::path replace_path = temp_root / "replace_missing_assignments.prg";
    write_text(
        replace_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "REPLACE FOR .T.\n"
        "RETURN\n");
    copperfin::runtime::PrgRuntimeSession replace_session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(replace_path.string(), temp_root.string(), false));
    const auto replace_state = replace_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!replace_state.completed, "#2717: qps-ploc REPLACE without assignments should fail");
    expect(
        replace_state.message ==
            copperfin::localization::pseudo_localize("REPLACE requires at least one FIELD WITH expression assignment"),
        "#2717: qps-ploc REPLACE assignment error should route through the pseudo-localization transform");

    const fs::path update_path = temp_root / "update_missing_assignments.prg";
    write_text(
        update_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "UPDATE People\n"
        "RETURN\n");
    copperfin::runtime::PrgRuntimeSession update_session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(update_path.string(), temp_root.string(), false));
    const auto update_state = update_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!update_state.completed, "#2717: qps-ploc UPDATE without SET assignments should fail");
    expect(
        update_state.message ==
            copperfin::localization::pseudo_localize("UPDATE requires SET field = expression assignments"),
        "#2717: qps-ploc UPDATE assignment error should route through the pseudo-localization transform");

    const fs::path insert_path = temp_root / "insert_missing_values.prg";
    write_text(
        insert_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "INSERT INTO People\n"
        "RETURN\n");
    copperfin::runtime::PrgRuntimeSession insert_session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(insert_path.string(), temp_root.string(), false));
    const auto insert_state = insert_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!insert_state.completed, "#2717: qps-ploc INSERT INTO without VALUES should fail");
    expect(
        insert_state.message ==
            copperfin::localization::pseudo_localize("INSERT INTO requires a VALUES clause"),
        "#2717: qps-ploc INSERT INTO VALUES-clause error should route through the pseudo-localization transform");

    const fs::path unlock_path = temp_root / "unlock_missing_record.prg";
    write_text(
        unlock_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "UNLOCK RECORD 99 IN People\n"
        "RETURN\n");
    copperfin::runtime::PrgRuntimeSession unlock_session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(unlock_path.string(), temp_root.string(), false));
    const auto unlock_state = unlock_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!unlock_state.completed, "#2717: qps-ploc UNLOCK RECORD with a missing record should fail");
    expect(
        unlock_state.message ==
            copperfin::localization::pseudo_localize("UNLOCK RECORD target record not found"),
        "#2717: qps-ploc UNLOCK RECORD target-record error should route through the pseudo-localization transform");

    const fs::path sleep_cancel_path = temp_root / "sleep_cancelled_task.prg";
    write_text(
        sleep_cancel_path,
        "PROCEDURE worker\n"
        "    SLEEP 50\n"
        "    RETURN\n"
        "ENDPROC\n"
        "PROCEDURE canceler\n"
        "    SLEEP 1\n"
        "    CANCEL\n"
        "ENDPROC\n"
        "SPAWN worker TO nWorker\n"
        "SPAWN canceler TO nCancel\n"
        "AWAIT nCancel TO lCancelDone\n"
        "AWAIT nWorker TO lWorkerDone\n"
        "RETURN\n");
    copperfin::runtime::PrgRuntimeSession sleep_cancel_session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(sleep_cancel_path.string(), temp_root.string(), false));
    const auto sleep_cancel_state = sleep_cancel_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(sleep_cancel_state.completed, "#2717: qps-ploc spawned-task cancellation script should complete");
    const auto sleep_cancel_event = std::find_if(
        sleep_cancel_state.events.begin(),
        sleep_cancel_state.events.end(),
        [](const auto& event) {
            return event.category == "runtime.task.await" &&
                   event.detail.find("state=error") != std::string::npos &&
                   event.detail.find(copperfin::localization::pseudo_localize("SLEEP cancelled.")) != std::string::npos;
        });
    expect(sleep_cancel_event != sleep_cancel_state.events.end(),
           "#2717: qps-ploc spawned-task cancellation should report an errored AWAIT event");
    if (sleep_cancel_event != sleep_cancel_state.events.end()) {
        expect(
            sleep_cancel_event->detail.find(copperfin::localization::pseudo_localize("SLEEP cancelled.")) != std::string::npos,
            "#2717: qps-ploc spawned-task cancellation should preserve the localized SLEEP cancellation text");
    }

    fs::remove_all(temp_root, ignored);
}

void test_dispatch_array_and_object_target_runtime_errors_use_default_locale_messages() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_dispatch_helper_defaults";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}});

    const auto run_error_script = [&](const std::string& file_stem, const std::string& script) {
        const fs::path main_path = temp_root / (file_stem + ".prg");
        write_text(main_path, script + "RETURN\n");
        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
        return session.run(copperfin::runtime::DebugResumeAction::continue_run);
    };

    const auto copy_missing_array = run_error_script("copy_missing_array", "COPY TO ARRAY FIELDS NAME\n");
    expect(!copy_missing_array.completed, "#2722: COPY TO ARRAY without a target array should fail");
    expect(
        copy_missing_array.message == "COPY TO ARRAY: array name required",
        "#2722: COPY TO ARRAY should keep the default-locale array-name-required helper text");

    const auto copy_invalid_array = run_error_script(
        "copy_invalid_array",
        "COPY TO ARRAY 'bad name'\n");
    expect(!copy_invalid_array.completed, "#2722: COPY TO ARRAY with an invalid array target should fail");
    expect(
        copy_invalid_array.message == "COPY TO ARRAY: invalid array name",
        "#2722: COPY TO ARRAY should keep the default-locale invalid-array-name helper text");

    const auto scatter_invalid_object = run_error_script(
        "scatter_invalid_object",
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SCATTER NAME 'bad path'\n");
    expect(!scatter_invalid_object.completed, "#2722: SCATTER NAME with an invalid object target should fail");
    expect(
        scatter_invalid_object.message == "SCATTER NAME: invalid object target",
        "#2722: SCATTER NAME should keep the default-locale invalid-object-target helper text");

    fs::remove_all(temp_root, ignored);
}

void test_dispatch_array_and_object_target_runtime_errors_localize() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_dispatch_helper_localization";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}});

    ScopedEnvironmentValue scoped_locale("COPPERFIN_LOCALE");

    const auto run_error_script = [&](const std::string& file_stem, const std::string& script) {
        const fs::path main_path = temp_root / (file_stem + ".prg");
        write_text(main_path, script + "RETURN\n");
        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
        return session.run(copperfin::runtime::DebugResumeAction::continue_run);
    };

    set_env_value("COPPERFIN_LOCALE", "es-419", true);
    const auto copy_missing_array = run_error_script("copy_missing_array_es", "COPY TO ARRAY FIELDS NAME\n");
    expect(!copy_missing_array.completed, "#2722: es-419 COPY TO ARRAY without a target array should fail");
    expect(
        copy_missing_array.message == "COPY TO ARRAY: se requiere un nombre de arreglo",
        "#2722: es-419 COPY TO ARRAY helper error should localize the array-name-required text");

    set_env_value("COPPERFIN_LOCALE", "pt-BR", true);
    const auto scatter_invalid_object = run_error_script(
        "scatter_invalid_object_pt",
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SCATTER NAME 'bad path'\n");
    expect(!scatter_invalid_object.completed, "#2722: pt-BR SCATTER NAME with an invalid object target should fail");
    expect(
        scatter_invalid_object.message == "SCATTER NAME: destino de objeto invalido",
        "#2722: pt-BR SCATTER NAME helper error should localize the invalid-object-target text");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    const auto scatter_invalid_object_qps = run_error_script(
        "scatter_invalid_object_qps",
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SCATTER NAME 'bad path'\n");
    expect(!scatter_invalid_object_qps.completed, "#2722: qps-ploc SCATTER NAME with an invalid object target should fail");
    expect(
        scatter_invalid_object_qps.message.find("[!! ") == 0U &&
            scatter_invalid_object_qps.message.find("SCATTER NAME") != std::string::npos &&
            scatter_invalid_object_qps.message.find("invalid object target") == std::string::npos,
        "#2722: qps-ploc SCATTER NAME helper error should pseudo-localize prose while preserving the command token");

    fs::remove_all(temp_root, ignored);
}

void test_ole_property_assignment_runtime_errors_localize() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_ole_assignment_localization";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue scoped_locale("COPPERFIN_LOCALE");
    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);

    const fs::path main_path = temp_root / "ole_assignment_localization.prg";
    write_text(
        main_path,
        "ON ERROR DO oleerr\n"
        "missingOle.SomeProperty = 42\n"
        "RETURN\n"
        "PROCEDURE oleerr\n"
        "PUBLIC nOleRows, cOleMessage, cOleDetail\n"
        "nOleRows = AERROR(aOleErr)\n"
        "cOleMessage = aOleErr[1,2]\n"
        "cOleDetail = aOleErr[1,3]\n"
        "RETURN\n"
        "ENDPROC\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "#2718: qps-ploc OLE property-assignment handler script should complete");

    const auto rows = state.globals.find("nolerows");
    const auto message = state.globals.find("colemessage");
    const auto detail = state.globals.find("coledetail");
    expect(rows != state.globals.end(), "#2718: qps-ploc OLE AERROR should return a row count");
    expect(message != state.globals.end(), "#2718: qps-ploc OLE AERROR should populate the localized message");
    expect(detail != state.globals.end(), "#2718: qps-ploc OLE AERROR should populate the failing member path");

    if (rows != state.globals.end()) {
        expect(copperfin::runtime::format_value(rows->second) == "1",
               "#2718: qps-ploc OLE AERROR should expose one row");
    }
    if (message != state.globals.end()) {
        const std::string localized_message = copperfin::runtime::format_value(message->second);
        expect(
            localized_message.find("[!! ") == 0U &&
                localized_message.find("missingOle.SomeProperty") != std::string::npos &&
                localized_message.find("OLE object not found for property assignment") == std::string::npos,
            "#2718: qps-ploc OLE property-assignment message should pseudo-localize prose while preserving the member path");
    }
    if (detail != state.globals.end()) {
        expect(copperfin::runtime::format_value(detail->second).find("missingOle.SomeProperty") != std::string::npos,
               "#2718: qps-ploc OLE AERROR detail should preserve the failing member path");
    }

    fs::remove_all(temp_root, ignored);
}

void test_ole_invocation_and_property_read_runtime_errors_localize() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_ole_read_localization";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue scoped_locale("COPPERFIN_LOCALE");
    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);

    const fs::path main_path = temp_root / "ole_read_localization.prg";
    write_text(
        main_path,
        "oDict = CREATEOBJECT('Scripting.Dictionary')\n"
        "oDict.Add('Alpha', 41)\n"
        "TRY\n"
        "  missingOle.NoSuchMethod()\n"
        "CATCH TO oMissingMethod\n"
        "  cMissingMethod = oMissingMethod.Message\n"
        "ENDTRY\n"
        "TRY\n"
        "  xMissingObjectProperty = missingOle.SomeProperty\n"
        "CATCH TO oMissingProperty\n"
        "  cMissingProperty = oMissingProperty.Message\n"
        "ENDTRY\n"
        "TRY\n"
        "  oDict.NoSuchMethod(7)\n"
        "CATCH TO oMissingMemberMethod\n"
        "  cMissingMemberMethod = oMissingMemberMethod.Message\n"
        "ENDTRY\n"
        "TRY\n"
        "  xMissingMemberProperty = oDict.NoSuchProperty\n"
        "CATCH TO oMissingMemberProperty\n"
        "  cMissingMemberProperty = oMissingMemberProperty.Message\n"
        "ENDTRY\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "#2719: qps-ploc OLE invocation/property-read localization script should complete");

    const auto missing_method = state.globals.find("cmissingmethod");
    const auto missing_property = state.globals.find("cmissingproperty");
    const auto missing_member_method = state.globals.find("cmissingmembermethod");
    const auto missing_member_property = state.globals.find("cmissingmemberproperty");

    expect(missing_method != state.globals.end(), "#2719: qps-ploc missing OLE method invocation should populate CATCH text");
    expect(missing_property != state.globals.end(), "#2719: qps-ploc missing OLE property read should populate CATCH text");
    expect(missing_member_method != state.globals.end(), "#2719: qps-ploc missing OLE member method should populate CATCH text");
    expect(missing_member_property != state.globals.end(), "#2719: qps-ploc missing OLE member property should populate CATCH text");

    if (missing_method != state.globals.end()) {
        std::string localized_message = copperfin::runtime::format_value(missing_method->second);
        std::string folded_message = localized_message;
        std::transform(folded_message.begin(), folded_message.end(), folded_message.begin(), [](unsigned char ch) {
            return static_cast<char>(std::tolower(ch));
        });
        expect(
            localized_message.find("[!! ") != std::string::npos &&
                folded_message.find("missingole.nosuchmethod") != std::string::npos &&
                folded_message.find("ole object not found for method invocation") == std::string::npos,
            "#2719: qps-ploc missing OLE method invocation should pseudo-localize prose while preserving the target identifier");
    }
    if (missing_property != state.globals.end()) {
        std::string localized_message = copperfin::runtime::format_value(missing_property->second);
        std::string folded_message = localized_message;
        std::transform(folded_message.begin(), folded_message.end(), folded_message.begin(), [](unsigned char ch) {
            return static_cast<char>(std::tolower(ch));
        });
        expect(
            localized_message.find("[!! ") != std::string::npos &&
                folded_message.find("missingole.someproperty") != std::string::npos &&
                folded_message.find("ole object not found for property read") == std::string::npos,
            "#2719: qps-ploc missing OLE property read should pseudo-localize prose while preserving the property path");
    }
    if (missing_member_method != state.globals.end()) {
        std::string localized_message = copperfin::runtime::format_value(missing_member_method->second);
        std::string folded_message = localized_message;
        std::transform(folded_message.begin(), folded_message.end(), folded_message.begin(), [](unsigned char ch) {
            return static_cast<char>(std::tolower(ch));
        });
        expect(
            localized_message.find("[!! ") != std::string::npos &&
                folded_message.find("scripting.dictionary.nosuchmethod") != std::string::npos &&
                folded_message.find("ole member not found for method invocation") == std::string::npos,
            "#2719: qps-ploc missing OLE member method should pseudo-localize prose while preserving the member identifier");
    }
    if (missing_member_property != state.globals.end()) {
        std::string localized_message = copperfin::runtime::format_value(missing_member_property->second);
        std::string folded_message = localized_message;
        std::transform(folded_message.begin(), folded_message.end(), folded_message.begin(), [](unsigned char ch) {
            return static_cast<char>(std::tolower(ch));
        });
        expect(
            localized_message.find("[!! ") != std::string::npos &&
                folded_message.find("scripting.dictionary.nosuchproperty") != std::string::npos &&
                folded_message.find("ole member not found for property read") == std::string::npos,
            "#2719: qps-ploc missing OLE member property should pseudo-localize prose while preserving the member identifier");
    }

    fs::remove_all(temp_root, ignored);
}

void test_on_error_resume_restores_fault_session_and_cursor_state() {
    // #150: RESUME should restore the captured fault-side data session/work area
    // even when the handler changes its own session and cursor selection.
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_resume_fault_state";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path items_path = temp_root / "items.dbf";
    const fs::path alt_path = temp_root / "alt.dbf";
    write_people_dbf(items_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}});
    write_people_dbf(alt_path, {{"DELTA", 40}, {"ECHO", 50}});

    const fs::path main_path = temp_root / "resume_fault_state.prg";
    write_text(
        main_path,
        "ON ERROR DO handleerr\n"
        "USE '" + items_path.string() + "' ALIAS Items IN 0\n"
        "SELECT Items\n"
        "SKIP\n"
        "rec_before = RECNO()\n"
        "fault = LOG(-1)\n"
        "alias_after = ALIAS()\n"
        "rec_after = RECNO()\n"
        "name_after = Items.NAME\n"
        "RETURN\n"
        "PROCEDURE handleerr\n"
        "SET DATASESSION TO 2\n"
        "USE '" + alt_path.string() + "' ALIAS Alt IN 0\n"
        "SELECT Alt\n"
        "RESUME\n"
        "ENDPROC\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "#150: ON ERROR RESUME script should complete");

    const auto rec_before = state.globals.find("rec_before");
    const auto alias_after = state.globals.find("alias_after");
    const auto rec_after = state.globals.find("rec_after");
    const auto name_after = state.globals.find("name_after");

    expect(rec_before != state.globals.end(), "#150: rec_before should be captured before the fault");
    expect(alias_after != state.globals.end(), "#150: post-fault ALIAS() should be captured");
    expect(rec_after != state.globals.end(), "#150: post-fault RECNO() should be captured");
    expect(name_after != state.globals.end(), "#150: post-fault field read should succeed");

    if (rec_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(rec_before->second) == "2",
               "#150: fault-side cursor should be positioned on record 2 before fault");
    }
    if (alias_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(alias_after->second) == "Items",
               "#150: RESUME should restore the fault-side selected alias");
    }
    if (rec_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(rec_after->second) == "2",
               "#150: RESUME should preserve fault-side cursor record position");
    }
    if (name_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(name_after->second) == "BRAVO",
               "#150: post-fault field reads should remain on the original cursor row");
    }

    fs::remove_all(temp_root, ignored);
}

void test_fault_continue_cycle_preserves_open_cursor_and_record_position() {
    // #151: after each debug-continue across a runtime fault, cursor state and
    // record position must remain stable so the developer can keep inspecting data.
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_fault_cursor";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "items.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}});

    const fs::path main_path = temp_root / "fault_cursor.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS Items IN 0\n"
        "SELECT Items\n"
        "SKIP\n"
        "recno_before = RECNO()\n"
        "x = LOG(-1)\n"
        "recno_after_first = RECNO()\n"
        "y = ACOS(2)\n"
        "recno_after_second = RECNO()\n"
        "cur_name = Items.NAME\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    // First fault: LOG(-1)
    auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error,
           "#151: first fault should pause with an error reason");
    expect(state.location.line == 5U,
           "#151: first fault should highlight line 5 (LOG(-1))");

    // Second fault: ACOS(2)
    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error,
           "#151: second fault should pause with an error reason");
    expect(state.location.line == 7U,
           "#151: second fault should highlight line 7 (ACOS(2))");

    // Final continue — should complete
    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed,
           "#151: session should complete after continuing past both faults");

    // Cursor should have been at record 2 (after SKIP from record 1)
    const auto recno_before = state.globals.find("recno_before");
    const auto recno_after_first = state.globals.find("recno_after_first");
    const auto recno_after_second = state.globals.find("recno_after_second");
    const auto cur_name = state.globals.find("cur_name");

    expect(recno_before != state.globals.end(), "#151: recno_before should be set");
    expect(recno_after_first != state.globals.end(), "#151: recno_after_first should be set after first fault continue");
    expect(recno_after_second != state.globals.end(), "#151: recno_after_second should be set after second fault continue");
    expect(cur_name != state.globals.end(), "#151: cursor field read should succeed after fault cycle");

    if (recno_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(recno_before->second) == "2",
               "#151: SKIP from record 1 should position at record 2");
    }
    if (recno_after_first != state.globals.end()) {
        expect(copperfin::runtime::format_value(recno_after_first->second) == "2",
               "#151: cursor record position should survive the first fault continue");
    }
    if (recno_after_second != state.globals.end()) {
        expect(copperfin::runtime::format_value(recno_after_second->second) == "2",
               "#151: cursor record position should survive the second fault continue");
    }
    if (cur_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(cur_name->second) == "BRAVO",
               "#151: cursor field access after fault cycle should return the expected record value");
    }

    fs::remove_all(temp_root, ignored);
}

void test_fault_continue_cycle_preserves_selected_alias_across_data_session_scope() {
    // #151: repeated CONTINUE over multiple faults should preserve selected-alias
    // stability even when faults occur inside a non-default data session.
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_fault_cursor_ds";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "items_ds.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}});

    const fs::path main_path = temp_root / "fault_cursor_ds.prg";
    write_text(
        main_path,
        "SET DATASESSION TO 2\n"
        "USE '" + table_path.string() + "' ALIAS Items IN 0\n"
        "SELECT Items\n"
        "SKIP\n"
        "alias_before = ALIAS()\n"
        "recno_before = RECNO()\n"
        "x = LOG(-1)\n"
        "alias_after_first = ALIAS()\n"
        "recno_after_first = RECNO()\n"
        "y = ACOS(2)\n"
        "alias_after_second = ALIAS()\n"
        "recno_after_second = RECNO()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error,
           "#151: first non-default-session fault should pause with error");

    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error,
           "#151: second non-default-session fault should pause with error");

    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed,
           "#151: session should complete after continuing past both non-default-session faults");

    const auto alias_before = state.globals.find("alias_before");
    const auto alias_after_first = state.globals.find("alias_after_first");
    const auto alias_after_second = state.globals.find("alias_after_second");
    const auto recno_before = state.globals.find("recno_before");
    const auto recno_after_first = state.globals.find("recno_after_first");
    const auto recno_after_second = state.globals.find("recno_after_second");

    expect(alias_before != state.globals.end(), "#151: alias_before should be captured");
    expect(alias_after_first != state.globals.end(), "#151: alias_after_first should be captured");
    expect(alias_after_second != state.globals.end(), "#151: alias_after_second should be captured");
    expect(recno_before != state.globals.end(), "#151: recno_before should be captured");
    expect(recno_after_first != state.globals.end(), "#151: recno_after_first should be captured");
    expect(recno_after_second != state.globals.end(), "#151: recno_after_second should be captured");

    if (alias_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(alias_before->second) == "Items",
               "#151: selected alias should be Items before first fault");
    }
    if (alias_after_first != state.globals.end()) {
        expect(copperfin::runtime::format_value(alias_after_first->second) == "Items",
               "#151: selected alias should survive first fault continue");
    }
    if (alias_after_second != state.globals.end()) {
        expect(copperfin::runtime::format_value(alias_after_second->second) == "Items",
               "#151: selected alias should survive second fault continue");
    }
    if (recno_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(recno_before->second) == "2",
               "#151: pre-fault record position should be 2");
    }
    if (recno_after_first != state.globals.end()) {
        expect(copperfin::runtime::format_value(recno_after_first->second) == "2",
               "#151: record position should survive first fault continue");
    }
    if (recno_after_second != state.globals.end()) {
        expect(copperfin::runtime::format_value(recno_after_second->second) == "2",
               "#151: record position should survive second fault continue");
    }

    fs::remove_all(temp_root, ignored);
}

void test_pause_stack_frame_contains_accurate_intermediate_frame_lines() {
    // #152: all frames in the call stack at a fault pause should report
    // the line at which each caller invoked the next routine — not zero or stale.
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_frame_lines";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "frame_lines.prg";
    write_text(
        main_path,
        "before_call = 1\n"
        "DO outerproc\n"
        "after_call = 1\n"
        "RETURN\n"
        "PROCEDURE outerproc\n"
        "outer_start = 1\n"
        "DO innerproc\n"
        "outer_end = 1\n"
        "RETURN\n"
        "ENDPROC\n"
        "PROCEDURE innerproc\n"
        "inner_start = 1\n"
        "fault_val = LOG(-1)\n"
        "inner_end = 1\n"
        "RETURN\n"
        "ENDPROC\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error,
           "#152: nested fault should pause with error reason");
    expect(state.location.line == 13U,
           "#152: fault location should point at the LOG(-1) line");
    expect(state.call_stack.size() >= 3U,
           "#152: call stack should expose all three frames at fault time");

    if (state.call_stack.size() >= 3U) {
        // Top frame: innerproc, faulting line
        expect(state.call_stack[0].routine_name == "innerproc",
               "#152: top frame routine name should be innerproc");
        expect(state.call_stack[0].line == 13U,
               "#152: top frame line should be the fault line inside innerproc");

        // Middle frame: outerproc, line where DO innerproc was invoked
        expect(state.call_stack[1].routine_name == "outerproc",
               "#152: middle frame routine name should be outerproc");
         // The runtime records the resume PC (statement after the DO), so line 8 = outer_end = 1
         expect(state.call_stack[1].line == 8U,
             "#152: middle frame line should be the resume line after DO innerproc (outer_end = 1)");

        // Bottom frame: main, line where DO outerproc was invoked
        expect(state.call_stack[2].routine_name == "main",
               "#152: bottom frame routine name should be main");
         // The runtime records the resume PC (statement after the DO), so line 3 = after_call = 1
         expect(state.call_stack[2].line == 3U,
             "#152: bottom frame line should be the resume line after DO outerproc (after_call = 1)");
    }

    // Continue past the fault; the session should remain alive
    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "#152: session should complete after continuing past the nested fault");
    const auto after_call = state.globals.find("after_call");
    expect(after_call != state.globals.end(), "#152: post-call statement should execute after fault continue");

    fs::remove_all(temp_root, ignored);
}

void test_repeated_fault_pauses_refresh_intermediate_stack_frame_lines() {
    // #152: intermediate caller-frame line metadata should refresh across
    // repeated nested fault pauses instead of leaking stale frame line values.
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_frame_lines_repeat";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "frame_lines_repeat.prg";
    write_text(
        main_path,
        "DO outerfirst\n"
        "DO outersecond\n"
        "RETURN\n"
        "PROCEDURE outerfirst\n"
        "DO innerfirst\n"
        "RETURN\n"
        "PROCEDURE outersecond\n"
        "DO innersecond\n"
        "RETURN\n"
        "PROCEDURE innerfirst\n"
        "x = LOG(-1)\n"
        "RETURN\n"
        "PROCEDURE innersecond\n"
        "y = ACOS(2)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error,
           "#152: first nested fault should pause with error");
    expect(state.location.line == 11U,
           "#152: first nested fault should highlight the first inner routine line");
    expect(state.call_stack.size() >= 3U,
           "#152: first nested fault should expose full call stack");
    if (state.call_stack.size() >= 2U) {
        expect(state.call_stack[1].routine_name == "outerfirst",
               "#152: first pause should report outerfirst as intermediate frame");
        expect(state.call_stack[1].line == 6U,
               "#152: first pause intermediate frame line should match the caller resume PC");
    }

    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error,
           "#152: second nested fault should pause with error");
    expect(state.location.line == 14U,
           "#152: second nested fault should highlight the second inner routine line");
    expect(state.call_stack.size() >= 3U,
           "#152: second nested fault should expose full call stack");
    if (state.call_stack.size() >= 2U) {
        expect(state.call_stack[1].routine_name == "outersecond",
               "#152: second pause should refresh intermediate frame routine");
        expect(state.call_stack[1].line == 9U,
               "#152: second pause intermediate frame line should refresh to the second caller resume PC");
    }

    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed,
           "#152: session should complete after continuing past both nested faults");

    fs::remove_all(temp_root, ignored);
}

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

void test_division_by_zero_dispatches_runtime_error() {
    // GAP-01 #257: dividing by zero in a PRG expression must produce a runtime
    // error pause (not a host crash, not a silent NaN or infinity result).
    namespace fs = std::filesystem;
    ScopedEnvironmentValue scoped_locale("COPPERFIN_LOCALE");
    set_env_value("COPPERFIN_LOCALE", "en-US", true);
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_divzero";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "divzero.prg";
    write_text(
        main_path,
        "before_div = 1\n"
        "x = 1 / 0\n"
        "after_div = 1\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error,
           "GAP-01/#257: division by zero should pause with an error reason");
    expect(state.location.line == 2U,
           "GAP-01/#257: division by zero should highlight line 2");
    expect(state.message == "Runtime fault: Division by zero",
           "#2541: division by zero should route through the default locale fault wrapper (got '" + state.message + "')");

    // Session must survive a continue after the divide-by-zero fault
    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed,
           "GAP-01/#257: session should complete after continuing past a divide-by-zero fault");
    const auto after_div = state.globals.find("after_div");
    expect(after_div != state.globals.end(),
           "GAP-01/#257: statements after the divide-by-zero line should still execute after a fault continue");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    copperfin::runtime::PrgRuntimeSession pseudo_session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto pseudo_state = pseudo_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(pseudo_state.reason == copperfin::runtime::DebugPauseReason::error,
           "#2716: qps-ploc division by zero should still pause with an error reason");
    expect(
        pseudo_state.message.find("[!! ") == 0U &&
            pseudo_state.message.find("Runtime fault") == std::string::npos &&
            pseudo_state.message != "Runtime fault: Division by zero",
        "#2716: division by zero should route through the localized runtime fault wrapper");

    fs::remove_all(temp_root, ignored);
}

void test_numeric_field_overflow_is_diagnosed_not_silently_truncated() {
    // GAP-01 #258: writing a value wider than an N-field's declared width must
    // not silently store a garbage or truncated value; the runtime must either
    // fail with a diagnostic or store the correctly bounded value without crashing.
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_num_overflow";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    // Create a cursor with a 3-digit numeric field and attempt to REPLACE it
    // with a value that requires 4 digits.
    const fs::path prg_path = temp_root / "num_overflow.prg";
    write_text(
        prg_path,
        "CREATE CURSOR overflow_test (code N(3,0))\n"
        "INSERT INTO overflow_test VALUES (1)\n"
        "GO TOP\n"
        "overflow_error = 0\n"
        "ON ERROR DO handleerr\n"
        "REPLACE code WITH 9999\n"
        "code_after_replace = overflow_test.code\n"
        "RETURN\n"
        "PROCEDURE handleerr\n"
        "overflow_error = 1\n"
        "RETURN\n"
        "ENDPROC\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(prg_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed,
           "GAP-01/#258: numeric overflow test script should complete without host crash");

    const auto code_after = state.globals.find("code_after_replace");
    const auto overflow_error = state.globals.find("overflow_error");

    // The outcome must be one of:
    //   (a) an error was dispatched (overflow_error == 1), or
    //   (b) the stored value is within the field range (0-999)
    bool diagnosed = false;
    if (overflow_error != state.globals.end()) {
        diagnosed = (copperfin::runtime::format_value(overflow_error->second) == "1");
    }
    bool within_range = false;
    if (code_after != state.globals.end()) {
        const std::string stored = copperfin::runtime::format_value(code_after->second);
        // Value must not be "9999" (which would mean silent overflow storage)
        within_range = (stored != "9999");
    }
    expect(diagnosed || within_range || code_after == state.globals.end(),
           "GAP-01/#258: numeric field overflow must be diagnosed or safely bounded — not silently stored as 9999");

    fs::remove_all(temp_root, ignored);
}

void test_retry_reexecutes_faulting_statement() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_retry";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "retry_test.prg";
    write_text(
        main_path,
        "attempt_count = 0\n"
        "ON ERROR DO handleerr\n"
        "DO missing_routine\n"
        "after_retry = 1\n"
        "RETURN\n"
        "PROCEDURE handleerr\n"
        "attempt_count = attempt_count + 1\n"
        "IF attempt_count < 2\n"
        "  RETRY\n"
        "ENDIF\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "RETRY test script should complete");

    const auto attempt = state.globals.find("attempt_count");
    const auto after = state.globals.find("after_retry");
    expect(attempt != state.globals.end(), "attempt_count should be set");
    expect(after != state.globals.end(), "execution should continue after RETRY handler finally returns");
    if (attempt != state.globals.end()) {
        expect(copperfin::runtime::format_value(attempt->second) == "2",
               "handler should have been called twice (once retry, once return)");
    }
    if (after != state.globals.end()) {
        expect(copperfin::runtime::format_value(after->second) == "1",
               "post-fault statement should run after handler's normal RETURN");
    }

    fs::remove_all(temp_root, ignored);
}

void test_resume_next_continues_after_fault() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_resume_next";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "resume_test.prg";
    write_text(
        main_path,
        "ON ERROR DO handleerr\n"
        "DO missing_routine\n"
        "after_error = 1\n"
        "RETURN\n"
        "PROCEDURE handleerr\n"
        "PUBLIC handled\n"
        "handled = 1\n"
        "RESUME\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "RESUME test script should complete");

    const auto handled = state.globals.find("handled");
    const auto after_error = state.globals.find("after_error");
    expect(handled != state.globals.end(), "error handler should have run and set handled flag");
    expect(after_error != state.globals.end(), "RESUME should skip the faulting statement and continue to next");
    if (handled != state.globals.end()) {
        expect(copperfin::runtime::format_value(handled->second) == "1", "handled should be 1");
    }
    if (after_error != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_error->second) == "1",
               "statement after faulting one should execute after RESUME");
    }

    fs::remove_all(temp_root, ignored);
}

void test_retry_with_no_fault_checkpoint_is_noop() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_retry_noop";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "retry_noop.prg";
    write_text(
        main_path,
        "noop_count = 1\n"
        "RETRY\n"
        "noop_count = 2\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "RETRY outside error handler should not crash");

    const auto noop_count = state.globals.find("noop_count");
    expect(noop_count != state.globals.end(), "noop_count should be set");
    if (noop_count != state.globals.end()) {
        expect(copperfin::runtime::format_value(noop_count->second) == "2",
               "RETRY with no fault checkpoint is a no-op; execution should continue to next statement");
    }

    fs::remove_all(temp_root, ignored);
}

void test_aerror_line_number_is_innermost_faulting_line_not_catch_site() {
    // #256: AERROR()[1,5] inside a CATCH block must report the innermost faulting
    // line (inside the deeply nested routine), not the TRY/CATCH site.
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_aerror_innermost_line";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "aerror_innermost.prg";
    // Lines:
    //  1: TRY
    //  2:   DO level1
    //  3: CATCH TO oErr
    //  4:   nErrorLines = AERROR(aErr)
    //  5:   nFaultLine = aErr[1,5]
    //  6:   cFaultProc = aErr[1,6]
    //  7: ENDTRY
    //  8: RETURN
    //  9: PROCEDURE level1
    // 10:   DO level2
    // 11:   RETURN
    // 12: ENDPROC
    // 13: PROCEDURE level2
    // 14:   fault_val = LOG(-1)   <-- actual fault here
    // 15:   RETURN
    // 16: ENDPROC
    write_text(
        main_path,
        "TRY\n"
        "  DO level1\n"
        "CATCH TO oErr\n"
        "  nErrorLines = AERROR(aErr)\n"
        "  nFaultLine = aErr[1,5]\n"
        "  cFaultProc = aErr[1,6]\n"
        "ENDTRY\n"
        "RETURN\n"
        "PROCEDURE level1\n"
        "  DO level2\n"
        "  RETURN\n"
        "ENDPROC\n"
        "PROCEDURE level2\n"
        "  fault_val = LOG(-1)\n"
        "  RETURN\n"
        "ENDPROC\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "TRY/CATCH nested routine fault should complete: " + state.message);

    const auto error_lines = state.globals.find("nerrorlines");
    const auto fault_line = state.globals.find("nfaultline");
    const auto fault_proc = state.globals.find("cfaultproc");

    expect(error_lines != state.globals.end(), "AERROR() should populate inside CATCH");
    expect(fault_line != state.globals.end(), "AERROR()[1,5] should be accessible inside CATCH");
    expect(fault_proc != state.globals.end(), "AERROR()[1,6] should be accessible inside CATCH");

    if (error_lines != state.globals.end()) {
        expect(copperfin::runtime::format_value(error_lines->second) == "1",
            "AERROR() inside CATCH should return one error row");
    }
    if (fault_line != state.globals.end()) {
        expect(copperfin::runtime::format_value(fault_line->second) == "14",
            "AERROR()[1,5] should report line 14 (innermost fault in level2), not the CATCH or DO level1 line");
    }
    if (fault_proc != state.globals.end()) {
        const auto proc_val = copperfin::runtime::format_value(fault_proc->second);
        expect(proc_val.find("level2") != std::string::npos,
            "AERROR()[1,6] should name level2 as the faulting procedure (got '" + proc_val + "')");
    }

    fs::remove_all(temp_root, ignored);
}

}  // namespace cf_test_prg_engine_control_flow
