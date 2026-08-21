// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_prg_engine_control_flow_support.h"

namespace cf_test_prg_engine_control_flow {

void test_aerror_populates_structured_runtime_error_array() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_aerror";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 21}, {"BRAVO", 28}});

    const fs::path main_path = temp_root / "aerror.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SELECT People\n"
        "ON ERROR DO handleerr\n"
        "DO missing_target\n"
        "after_error = 'continued'\n"
        "RETURN\n"
        "PROCEDURE handleerr\n"
        "PUBLIC nErrorRows, nErrorCols, nErrorCode, cErrorMessage, cErrorParam, nErrorWorkArea, nErrorArrayLine, cErrorArrayProgram, cErrorArrayStatement, cSys2018, cErrorProgram, nErrorLine, cErrorHandler\n"
        "SELECT 0\n"
        "nErrorRows = AERROR(aErr)\n"
        "nErrorCols = ALEN(aErr, 2)\n"
        "nErrorCode = aErr[1,1]\n"
        "cErrorMessage = aErr[1,2]\n"
        "cErrorParam = aErr[1,3]\n"
        "nErrorWorkArea = aErr[1,4]\n"
        "nErrorArrayLine = aErr[1,5]\n"
        "cErrorArrayProgram = aErr[1,6]\n"
        "cErrorArrayStatement = aErr[1,7]\n"
        "cSys2018 = SYS(2018)\n"
        "cErrorProgram = PROGRAM()\n"
        "nErrorLine = LINENO()\n"
        "cErrorHandler = ON('ERROR')\n"
        "RETURN\n"
        "ENDPROC\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "AERROR() handler script should complete");

    const auto rows = state.globals.find("nerrorrows");
    const auto cols = state.globals.find("nerrorcols");
    const auto code = state.globals.find("nerrorcode");
    const auto message = state.globals.find("cerrormessage");
    const auto parameter = state.globals.find("cerrorparam");
    const auto work_area = state.globals.find("nerrorworkarea");
    const auto array_line = state.globals.find("nerrorarrayline");
    const auto array_program = state.globals.find("cerrorarrayprogram");
    const auto array_statement = state.globals.find("cerrorarraystatement");
    const auto sys2018 = state.globals.find("csys2018");
    const auto line = state.globals.find("nerrorline");
    const auto program = state.globals.find("cerrorprogram");
    const auto handler = state.globals.find("cerrorhandler");
    const auto after_error = state.globals.find("after_error");

    expect(rows != state.globals.end(), "AERROR() should return a row count");
    expect(cols != state.globals.end(), "AERROR() array should expose a column count");
    expect(code != state.globals.end(), "AERROR() should populate the error code column");
    expect(message != state.globals.end(), "AERROR() should populate the message column");
    expect(parameter != state.globals.end(), "AERROR() should populate the error parameter column");
    expect(work_area != state.globals.end(), "AERROR() should populate the work-area column");
    expect(array_line != state.globals.end(), "AERROR() should populate the source-line column for normal runtime errors");
    expect(array_program != state.globals.end(), "AERROR() should populate the procedure column for normal runtime errors");
    expect(array_statement != state.globals.end(), "AERROR() should populate the faulting-statement column for normal runtime errors");
    expect(sys2018 != state.globals.end(), "SYS(2018) should expose the uppercase error parameter");
    expect(line != state.globals.end(), "AERROR() should populate the failing line column");
    expect(program != state.globals.end(), "AERROR() should populate the procedure column");
    expect(handler != state.globals.end(), "AERROR() should populate the ON ERROR handler column");
    expect(after_error != state.globals.end(), "ON ERROR script should continue after handler dispatch");

    if (rows != state.globals.end()) {
        expect(copperfin::runtime::format_value(rows->second) == "1", "AERROR() should return one first-pass error row");
    }
    if (cols != state.globals.end()) {
        expect(copperfin::runtime::format_value(cols->second) == "7", "AERROR() should expose the VFP seven-column shape");
    }
    if (code != state.globals.end()) {
        expect(copperfin::runtime::format_value(code->second) == "1001", "AERROR() should expose a distinct resolve-failure error code");
    }
    if (message != state.globals.end()) {
        expect(copperfin::runtime::format_value(message->second).find("Unable to resolve DO target") != std::string::npos,
            "AERROR() message should describe the failing runtime command");
    }
    if (parameter != state.globals.end()) {
        expect(copperfin::runtime::format_value(parameter->second) == "missing_target",
            "AERROR() parameter column should preserve the mixed-case runtime error parameter");
    }
    if (work_area != state.globals.end()) {
        expect(copperfin::runtime::format_value(work_area->second) == "1",
            "AERROR() work-area column should capture the faulting work area even if the handler changes selection");
    }
    if (array_line != state.globals.end()) {
        expect(copperfin::runtime::format_value(array_line->second) == "4",
            "AERROR() line column should capture the faulting source line");
    }
    if (array_program != state.globals.end()) {
        expect(copperfin::runtime::format_value(array_program->second) == "main",
            "AERROR() procedure column should capture the faulting routine");
    }
    if (array_statement != state.globals.end()) {
        expect(copperfin::runtime::format_value(array_statement->second) == "DO missing_target",
            "AERROR() statement column should capture the failing source text");
    }
    if (sys2018 != state.globals.end()) {
        expect(copperfin::runtime::format_value(sys2018->second) == "MISSING_TARGET",
            "SYS(2018) should expose the uppercase runtime error parameter");
    }
    if (line != state.globals.end()) {
        expect(copperfin::runtime::format_value(line->second) == "4",
            "AERROR() line column should report the failing source line");
    }
    if (program != state.globals.end()) {
        expect(copperfin::runtime::format_value(program->second) == "main",
            "AERROR() procedure column should report the failing routine");
    }
    if (handler != state.globals.end()) {
        expect(copperfin::runtime::format_value(handler->second).find("DO handleerr") != std::string::npos,
            "AERROR() handler column should preserve the active ON ERROR clause");
    }
    fs::remove_all(temp_root, ignored);
}

}  // namespace cf_test_prg_engine_control_flow
