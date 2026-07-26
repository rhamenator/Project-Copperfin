#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_grid_scrollbars_defaults_are_runtime_readonly_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_grid_scrollbars";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_grid_scrollbars.prg";
        write_text(
            main_path,
            "oGrid = CREATEOBJECT('Grid')\n"
            "oText = CREATEOBJECT('TextBox')\n"
            "lTextHas = PEMSTATUS(oText, 'ScrollBars', 1)\n"
            "lHas = PEMSTATUS(oGrid, 'ScrollBars', 1)\n"
            "lReadOnly = PEMSTATUS(oGrid, 'ScrollBars', 5)\n"
            "nDefault = oGrid.ScrollBars\n"
            "oGrid.ScrollBars = 1\n"
            "nDirect = oGrid.ScrollBars\n"
            "lSetPem = SETPEM(oGrid, 'ScrollBars', 2)\n"
            "nSetPem = GETPEM(oGrid, 'ScrollBars')\n"
            "lPutPem = PUTPEM(oGrid, 'ScrollBars', 0)\n"
            "nPutPem = GETPEM(oGrid, 'ScrollBars')\n"
            "lAddProperty = ADDPROPERTY(oGrid, 'ScrollBars', 1)\n"
            "lRemoveProperty = REMOVEPROPERTY(oGrid, 'ScrollBars')\n"
            "nPropMembers = AMEMBERS(aPropMembers, oGrid, 1)\n"
            "lPropHas = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'SCROLLBARS'\n"
            "        lPropHas = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedGridScrollBars')\n"
            "nDerived = oDerived.ScrollBars\n"
            "RETURN\n"
            "DEFINE CLASS DerivedGridScrollBars AS Grid\n"
            "    ScrollBars = 1\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Grid ScrollBars script should complete: ") + state.message +
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
        check("lreadonly", "true");
        check("ndefault", "3");
        check("ndirect", "3");
        check("lsetpem", "false");
        check("nsetpem", "3");
        check("lputpem", "false");
        check("nputpem", "3");
        check("laddproperty", "false");
        check("lremoveproperty", "false");
        check("lprophas", "true");
        check("nderived", "1");
        expect(state.ole_objects.size() >= 3U,
               "native ScrollBars coverage should register TextBox, Grid, and derived Grid objects");

        fs::remove_all(temp_root, ignored);
    }
}
