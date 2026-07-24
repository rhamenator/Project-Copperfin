#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_grid_relative_focus_defaults_read_only_and_stays_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_grid_relative_focus";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_grid_relative_focus.prg";
        write_text(
            main_path,
            "oGrid = CREATEOBJECT('Grid')\n"
            "oText = CREATEOBJECT('TextBox')\n"
            "lTextColumn = PEMSTATUS(oText, 'RelativeColumn', 1)\n"
            "lTextRow = PEMSTATUS(oText, 'RelativeRow', 1)\n"
            "lColumn = PEMSTATUS(oGrid, 'RelativeColumn', 1)\n"
            "lRow = PEMSTATUS(oGrid, 'RelativeRow', 1)\n"
            "lColumnReadOnly = PEMSTATUS(oGrid, 'RelativeColumn', 5)\n"
            "lRowReadOnly = PEMSTATUS(oGrid, 'RelativeRow', 5)\n"
            "nColumnDefault = oGrid.RelativeColumn\n"
            "nRowDefault = oGrid.RelativeRow\n"
            "oGrid.RelativeColumn = 3\n"
            "oGrid.RelativeRow = 4\n"
            "nColumnAfterDirect = oGrid.RelativeColumn\n"
            "nRowAfterDirect = oGrid.RelativeRow\n"
            "lColumnSetPem = SETPEM(oGrid, 'RelativeColumn', 5)\n"
            "lRowSetPem = SETPEM(oGrid, 'RelativeRow', 6)\n"
            "nColumnAfterSetPem = GETPEM(oGrid, 'RelativeColumn')\n"
            "nRowAfterSetPem = GETPEM(oGrid, 'RelativeRow')\n"
            "lColumnAddProperty = ADDPROPERTY(oGrid, 'RelativeColumn', 8)\n"
            "lRowAddProperty = ADDPROPERTY(oGrid, 'RelativeRow', 9)\n"
            "lColumnRemoveProperty = REMOVEPROPERTY(oGrid, 'RelativeColumn')\n"
            "lRowRemoveProperty = REMOVEPROPERTY(oGrid, 'RelativeRow')\n"
            "nPropMembers = AMEMBERS(aPropMembers, oGrid, 1)\n"
            "lColumnPropHas = .F.\n"
            "lRowPropHas = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'RELATIVECOLUMN'\n"
            "        lColumnPropHas = .T.\n"
            "    ENDIF\n"
            "    IF UPPER(aPropMembers[i]) == 'RELATIVEROW'\n"
            "        lRowPropHas = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedGridRelativeFocus')\n"
            "nDerivedColumn = oDerived.RelativeColumn\n"
            "nDerivedRow = oDerived.RelativeRow\n"
            "RETURN\n"
            "DEFINE CLASS DerivedGridRelativeFocus AS Grid\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Grid relative focus script should complete: ") + state.message +
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

        check("ltextcolumn", "false");
        check("ltextrow", "false");
        check("lcolumn", "true");
        check("lrow", "true");
        check("lcolumnreadonly", "true");
        check("lrowreadonly", "true");
        check("ncolumndefault", "0");
        check("nrowdefault", "0");
        check("ncolumnafterdirect", "0");
        check("nrowafterdirect", "0");
        check("lcolumnsetpem", "false");
        check("lrowsetpem", "false");
        check("ncolumnaftersetpem", "0");
        check("nrowaftersetpem", "0");
        check("lcolumnaddproperty", "false");
        check("lrowaddproperty", "false");
        check("lcolumnremoveproperty", "false");
        check("lrowremoveproperty", "false");
        check("lcolumnprophas", "true");
        check("lrowprophas", "true");
        check("nderivedcolumn", "0");
        check("nderivedrow", "0");
        expect(state.ole_objects.size() == 3U,
               "native relative focus coverage should register Grid, TextBox, and derived Grid objects");

        fs::remove_all(temp_root, ignored);
    }
}
