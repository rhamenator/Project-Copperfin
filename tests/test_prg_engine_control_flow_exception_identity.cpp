// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "test_prg_engine_control_flow_support.h"

namespace cf_test_prg_engine_control_flow {

void test_throw_preserves_exception_derived_object_identity() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() /
                               "copperfin_prg_throw_exception_identity";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "throw_exception_identity.prg";
    write_text(
        main_path,
        "oThrown = CREATEOBJECT('CustomException')\n"
        "oThrown.ErrorCode = 42\n"
        "TRY\n"
        "  THROW oThrown\n"
        "CATCH TO oCaught\n"
        "  cCaughtClass = oCaught.Class\n"
        "  cCaughtBaseClass = oCaught.BaseClass\n"
        "  nCaughtErrorCode = oCaught.ErrorCode\n"
        "  lSameObject = COMPOBJ(oCaught, oThrown)\n"
        "ENDTRY\n"
        "RETURN\n"
        "DEFINE CLASS CustomException AS Exception\n"
        "    ErrorCode = 0\n"
        "ENDDEFINE\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed,
           "THROW of an Exception-derived object should complete through CATCH: " + state.message);

    const auto check = [&](const std::string &name, const std::string &expected) {
        const auto found = state.globals.find(name);
        if (found == state.globals.end()) {
            expect(false, name + " should be captured");
            return;
        }
        expect(copperfin::runtime::format_value(found->second) == expected,
               name + " expected '" + expected + "' got '" +
                   copperfin::runtime::format_value(found->second) + "'");
    };

    check("ccaughtclass", "CustomException");
    check("ccaughtbaseclass", "Exception");
    check("ncaughterrorcode", "42");
    check("lsameobject", "true");
    expect(state.ole_objects.size() == 1U,
           "caught Exception-derived object should not be replaced by a synthetic wrapper");
    if (state.ole_objects.size() == 1U) {
        expect(state.ole_objects.front().prog_id == "CustomException",
               "caught Exception-derived object should retain its native class identity");
    }

    fs::remove_all(temp_root, ignored);
}

}  // namespace cf_test_prg_engine_control_flow
