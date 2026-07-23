#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_grid_allowheadersizing_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_grid_allowheadersizing";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_grid_allowheadersizing.prg";
        write_text(
            main_path,
            "oGrid = CREATEOBJECT('Grid')\n"
            "oText = CREATEOBJECT('TextBox')\n"
            "lTextHas = PEMSTATUS(oText, 'AllowHeaderSizing', 1)\n"
            "lHas = PEMSTATUS(oGrid, 'AllowHeaderSizing', 1)\n"
            "lReadOnly = PEMSTATUS(oGrid, 'AllowHeaderSizing', 5)\n"
            "lDefault = oGrid.AllowHeaderSizing\n"
            "oGrid.AllowHeaderSizing = 0\n"
            "lDirect = oGrid.AllowHeaderSizing\n"
            "lSetPem = SETPEM(oGrid, 'AllowHeaderSizing', 1)\n"
            "lAfterSetPem = GETPEM(oGrid, 'AllowHeaderSizing')\n"
            "lPutPem = PUTPEM(oGrid, 'AllowHeaderSizing', 0)\n"
            "lAfterPutPem = GETPEM(oGrid, 'AllowHeaderSizing')\n"
            "lAddProperty = ADDPROPERTY(oGrid, 'AllowHeaderSizing', .T.)\n"
            "lRemoveProperty = REMOVEPROPERTY(oGrid, 'AllowHeaderSizing')\n"
            "nPropMembers = AMEMBERS(aPropMembers, oGrid, 1)\n"
            "lPropHas = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'ALLOWHEADERSIZING'\n"
            "        lPropHas = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedGridAllowHeaderSizing')\n"
            "lDerived = oDerived.AllowHeaderSizing\n"
            "oDeclarative = CREATEOBJECT('DeclarativeGridAllowHeaderSizing')\n"
            "lDeclarative = oDeclarative.AllowHeaderSizing\n"
            "RETURN\n"
            "DEFINE CLASS DerivedGridAllowHeaderSizing AS Grid\n"
            "    PROCEDURE Init\n"
            "        THIS.AllowHeaderSizing = 0\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS DeclarativeGridAllowHeaderSizing AS Grid\n"
            "    AllowHeaderSizing = 0\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Grid AllowHeaderSizing script should complete: ") + state.message +
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
        check("ldefault", "true");
        check("ldirect", "false");
        check("lsetpem", "true");
        check("laftersetpem", "true");
        check("lputpem", "true");
        check("lafterputpem", "false");
        check("laddproperty", "false");
        check("lremoveproperty", "false");
        check("lprophas", "true");
        check("lderived", "false");
        check("ldeclarative", "false");
        expect(state.ole_objects.size() == 4U,
               "native AllowHeaderSizing coverage should register Grid, TextBox, and derived Grid objects");

        fs::remove_all(temp_root, ignored);
    }
}
