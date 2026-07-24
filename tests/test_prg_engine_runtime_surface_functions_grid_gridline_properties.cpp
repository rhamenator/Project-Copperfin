#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_grid_gridline_properties_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_grid_gridline_properties";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_grid_gridline_properties.prg";
        write_text(
            main_path,
            "oGrid = CREATEOBJECT('Grid')\n"
            "oText = CREATEOBJECT('TextBox')\n"
            "lTextHasColor = PEMSTATUS(oText, 'GridLineColor', 1)\n"
            "lTextHasWidth = PEMSTATUS(oText, 'GridLineWidth', 1)\n"
            "lColorHas = PEMSTATUS(oGrid, 'GridLineColor', 1)\n"
            "lWidthHas = PEMSTATUS(oGrid, 'GridLineWidth', 1)\n"
            "lColorReadOnly = PEMSTATUS(oGrid, 'GridLineColor', 5)\n"
            "lWidthReadOnly = PEMSTATUS(oGrid, 'GridLineWidth', 5)\n"
            "nColorDefault = oGrid.GridLineColor\n"
            "nWidthDefault = oGrid.GridLineWidth\n"
            "oGrid.GridLineColor = 16777216\n"
            "oGrid.GridLineWidth = 0\n"
            "nColorDirect = oGrid.GridLineColor\n"
            "nWidthDirect = oGrid.GridLineWidth\n"
            "lColorSetPem = SETPEM(oGrid, 'GridLineColor', 123)\n"
            "lWidthSetPem = SETPEM(oGrid, 'GridLineWidth', 41)\n"
            "nColorSetPem = GETPEM(oGrid, 'GridLineColor')\n"
            "nWidthSetPem = GETPEM(oGrid, 'GridLineWidth')\n"
            "lColorPutPem = PUTPEM(oGrid, 'GridLineColor', -1)\n"
            "lWidthPutPem = PUTPEM(oGrid, 'GridLineWidth', 40)\n"
            "nColorPutPem = GETPEM(oGrid, 'GridLineColor')\n"
            "nWidthPutPem = GETPEM(oGrid, 'GridLineWidth')\n"
            "lColorAddProperty = ADDPROPERTY(oGrid, 'GridLineColor', 7)\n"
            "lColorRemoveProperty = REMOVEPROPERTY(oGrid, 'GridLineColor')\n"
            "lWidthAddProperty = ADDPROPERTY(oGrid, 'GridLineWidth', 7)\n"
            "lWidthRemoveProperty = REMOVEPROPERTY(oGrid, 'GridLineWidth')\n"
            "nPropMembers = AMEMBERS(aPropMembers, oGrid, 1)\n"
            "lPropHasColor = .F.\n"
            "lPropHasWidth = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'GRIDLINECOLOR'\n"
            "        lPropHasColor = .T.\n"
            "    ENDIF\n"
            "    IF UPPER(aPropMembers[i]) == 'GRIDLINEWIDTH'\n"
            "        lPropHasWidth = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedGridGridLineProperties')\n"
            "nDerivedColor = oDerived.GridLineColor\n"
            "nDerivedWidth = oDerived.GridLineWidth\n"
            "oDeclarative = CREATEOBJECT('DeclarativeGridGridLineProperties')\n"
            "nDeclarativeColor = oDeclarative.GridLineColor\n"
            "nDeclarativeWidth = oDeclarative.GridLineWidth\n"
            "RETURN\n"
            "DEFINE CLASS DerivedGridGridLineProperties AS Grid\n"
            "    PROCEDURE Init\n"
            "        THIS.GridLineColor = 456\n"
            "        THIS.GridLineWidth = 7\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS DeclarativeGridGridLineProperties AS Grid\n"
            "    GridLineColor = 789\n"
            "    GridLineWidth = 9\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Grid grid-line property script should complete: ") + state.message +
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

        check("ltexthascolor", "false");
        check("ltexthaswidth", "false");
        check("lcolorhas", "true");
        check("lwidthhas", "true");
        check("lcolorreadonly", "false");
        check("lwidthreadonly", "false");
        check("ncolordefault", "0");
        check("nwidthdefault", "1");
        check("ncolordirect", "16777215");
        check("nwidthdirect", "1");
        check("lcolorsetpem", "true");
        check("lwidthsetpem", "true");
        check("ncolorsetpem", "123");
        check("nwidthsetpem", "40");
        check("lcolorputpem", "true");
        check("lwidthputpem", "true");
        check("ncolorputpem", "0");
        check("nwidthputpem", "40");
        check("lcoloraddproperty", "false");
        check("lcolorremoveproperty", "false");
        check("lwidthaddproperty", "false");
        check("lwidthremoveproperty", "false");
        check("lprophascolor", "true");
        check("lprophaswidth", "true");
        check("nderivedcolor", "456");
        check("nderivedwidth", "7");
        check("ndeclarativecolor", "789");
        check("ndeclarativewidth", "9");
        expect(state.ole_objects.size() == 4U,
               "native Grid grid-line coverage should register Grid, TextBox, and derived Grid objects");

        fs::remove_all(temp_root, ignored);
    }
}
