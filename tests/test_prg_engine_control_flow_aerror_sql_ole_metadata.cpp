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

}  // namespace cf_test_prg_engine_control_flow
