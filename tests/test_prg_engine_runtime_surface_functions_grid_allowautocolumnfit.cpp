#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_grid_allowautocolumnfit_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_grid_allowautocolumnfit";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_grid_allowautocolumnfit.prg";
        write_text(
            main_path,
            "oGrid = CREATEOBJECT('Grid')\n"
            "oText = CREATEOBJECT('TextBox')\n"
            "lTextHas = PEMSTATUS(oText, 'AllowAutoColumnFit', 1)\n"
            "lHas = PEMSTATUS(oGrid, 'AllowAutoColumnFit', 1)\n"
            "lReadOnly = PEMSTATUS(oGrid, 'AllowAutoColumnFit', 5)\n"
            "lDefault = oGrid.AllowAutoColumnFit\n"
            "oGrid.AllowAutoColumnFit = 1\n"
            "lDirect = oGrid.AllowAutoColumnFit\n"
            "lSetPem = SETPEM(oGrid, 'AllowAutoColumnFit', 0)\n"
            "lAfterSetPem = GETPEM(oGrid, 'AllowAutoColumnFit')\n"
            "lPutPem = PUTPEM(oGrid, 'AllowAutoColumnFit', .T.)\n"
            "lAfterPutPem = GETPEM(oGrid, 'AllowAutoColumnFit')\n"
            "lAddProperty = ADDPROPERTY(oGrid, 'AllowAutoColumnFit', .F.)\n"
            "lRemoveProperty = REMOVEPROPERTY(oGrid, 'AllowAutoColumnFit')\n"
            "nPropMembers = AMEMBERS(aPropMembers, oGrid, 1)\n"
            "lPropHas = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'ALLOWAUTOCOLUMNFIT'\n"
            "        lPropHas = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedGridAllowAutoColumnFit')\n"
            "lDerived = oDerived.AllowAutoColumnFit\n"
            "oDeclarative = CREATEOBJECT('DeclarativeGridAllowAutoColumnFit')\n"
            "lDeclarative = oDeclarative.AllowAutoColumnFit\n"
            "RETURN\n"
            "DEFINE CLASS DerivedGridAllowAutoColumnFit AS Grid\n"
            "    PROCEDURE Init\n"
            "        THIS.AllowAutoColumnFit = 0\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS DeclarativeGridAllowAutoColumnFit AS Grid\n"
            "    AllowAutoColumnFit = 1\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Grid AllowAutoColumnFit script should complete: ") + state.message +
                   " @line=" + std::to_string(state.location.line));

        const auto check = [&](const std::string& name, const std::string& expected) {
            const auto found = state.globals.find(name);
            expect(found != state.globals.end(), name + " should be captured");
            if (found != state.globals.end()) {
                expect(copperfin::runtime::format_value(found->second) == expected,
                       name + " expected '" + expected + "' got '" +
                           copperfin::runtime::format_value(found->second) + "'");
            }
        };

        check("ltexthas", "false");
        check("lhas", "true");
        check("lreadonly", "false");
        check("ldefault", "false");
        check("ldirect", "true");
        check("lsetpem", "true");
        check("laftersetpem", "false");
        check("lputpem", "true");
        check("lafterputpem", "true");
        check("laddproperty", "false");
        check("lremoveproperty", "false");
        check("lprophas", "true");
        check("lderived", "false");
        check("ldeclarative", "true");
        expect(state.ole_objects.size() == 4U,
               "native AllowAutoColumnFit coverage should register Grid, TextBox, and derived Grid objects");

        fs::remove_all(temp_root, ignored);
    }
}
