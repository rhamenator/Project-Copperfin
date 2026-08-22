// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_prg_engine_control_flow_support.h"

namespace cf_test_prg_engine_control_flow {
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
