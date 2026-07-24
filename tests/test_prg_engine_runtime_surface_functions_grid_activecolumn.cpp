#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_grid_activecolumn_defaults_read_only_and_stays_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_grid_activecolumn";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_grid_activecolumn.prg";
        write_text(
            main_path,
            "oGrid = CREATEOBJECT('Grid')\n"
            "oText = CREATEOBJECT('TextBox')\n"
            "lTextHas = PEMSTATUS(oText, 'ActiveColumn', 1)\n"
            "lHas = PEMSTATUS(oGrid, 'ActiveColumn', 1)\n"
            "lReadOnly = PEMSTATUS(oGrid, 'ActiveColumn', 5)\n"
            "nDefault = oGrid.ActiveColumn\n"
            "oGrid.ActiveColumn = 3\n"
            "nAfterDirect = oGrid.ActiveColumn\n"
            "lSetPem = SETPEM(oGrid, 'ActiveColumn', 4)\n"
            "nAfterSetPem = GETPEM(oGrid, 'ActiveColumn')\n"
            "lPutPem = PUTPEM(oGrid, 'ActiveColumn', 5)\n"
            "nAfterPutPem = GETPEM(oGrid, 'ActiveColumn')\n"
            "lAddProperty = ADDPROPERTY(oGrid, 'ActiveColumn', 8)\n"
            "lRemoveProperty = REMOVEPROPERTY(oGrid, 'ActiveColumn')\n"
            "nPropMembers = AMEMBERS(aPropMembers, oGrid, 1)\n"
            "lPropHas = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'ACTIVECOLUMN'\n"
            "        lPropHas = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedGridActiveColumn')\n"
            "nDerived = oDerived.ActiveColumn\n"
            "RETURN\n"
            "DEFINE CLASS DerivedGridActiveColumn AS Grid\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Grid ActiveColumn script should complete: ") + state.message +
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
        check("ndefault", "0");
        check("nafterdirect", "0");
        check("lsetpem", "false");
        check("naftersetpem", "0");
        check("lputpem", "false");
        check("nafterputpem", "0");
        check("laddproperty", "false");
        check("lremoveproperty", "false");
        check("lprophas", "true");
        check("nderived", "0");
        expect(state.ole_objects.size() == 3U,
               "native ActiveColumn coverage should register Grid, TextBox, and derived Grid objects");

        fs::remove_all(temp_root, ignored);
    }
}
