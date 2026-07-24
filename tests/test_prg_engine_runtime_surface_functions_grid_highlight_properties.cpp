#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_grid_highlight_properties_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_grid_highlight_properties";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_grid_highlight_properties.prg";
        write_text(
            main_path,
            "oGrid = CREATEOBJECT('Grid')\n"
            "oText = CREATEOBJECT('TextBox')\n"
            "lTextHasStyle = PEMSTATUS(oText, 'HighlightStyle', 1)\n"
            "lTextHasWidth = PEMSTATUS(oText, 'HighlightRowLineWidth', 1)\n"
            "lStyleHas = PEMSTATUS(oGrid, 'HighlightStyle', 1)\n"
            "lWidthHas = PEMSTATUS(oGrid, 'HighlightRowLineWidth', 1)\n"
            "lStyleReadOnly = PEMSTATUS(oGrid, 'HighlightStyle', 5)\n"
            "lWidthReadOnly = PEMSTATUS(oGrid, 'HighlightRowLineWidth', 5)\n"
            "nStyleDefault = oGrid.HighlightStyle\n"
            "nWidthDefault = oGrid.HighlightRowLineWidth\n"
            "oGrid.HighlightStyle = 3\n"
            "oGrid.HighlightRowLineWidth = -1\n"
            "nStyleDirect = oGrid.HighlightStyle\n"
            "nWidthDirect = oGrid.HighlightRowLineWidth\n"
            "lStyleSetPem = SETPEM(oGrid, 'HighlightStyle', 1)\n"
            "lWidthSetPem = SETPEM(oGrid, 'HighlightRowLineWidth', 8)\n"
            "nStyleSetPem = GETPEM(oGrid, 'HighlightStyle')\n"
            "nWidthSetPem = GETPEM(oGrid, 'HighlightRowLineWidth')\n"
            "lStylePutPem = PUTPEM(oGrid, 'HighlightStyle', -1)\n"
            "lWidthPutPem = PUTPEM(oGrid, 'HighlightRowLineWidth', 7)\n"
            "nStylePutPem = GETPEM(oGrid, 'HighlightStyle')\n"
            "nWidthPutPem = GETPEM(oGrid, 'HighlightRowLineWidth')\n"
            "lStyleAddProperty = ADDPROPERTY(oGrid, 'HighlightStyle', 2)\n"
            "lStyleRemoveProperty = REMOVEPROPERTY(oGrid, 'HighlightStyle')\n"
            "lWidthAddProperty = ADDPROPERTY(oGrid, 'HighlightRowLineWidth', 2)\n"
            "lWidthRemoveProperty = REMOVEPROPERTY(oGrid, 'HighlightRowLineWidth')\n"
            "nPropMembers = AMEMBERS(aPropMembers, oGrid, 1)\n"
            "lPropHasStyle = .F.\n"
            "lPropHasWidth = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'HIGHLIGHTSTYLE'\n"
            "        lPropHasStyle = .T.\n"
            "    ENDIF\n"
            "    IF UPPER(aPropMembers[i]) == 'HIGHLIGHTROWLINEWIDTH'\n"
            "        lPropHasWidth = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedGridHighlightProperties')\n"
            "nDerivedStyle = oDerived.HighlightStyle\n"
            "nDerivedWidth = oDerived.HighlightRowLineWidth\n"
            "oDeclarative = CREATEOBJECT('DeclarativeGridHighlightProperties')\n"
            "nDeclarativeStyle = oDeclarative.HighlightStyle\n"
            "nDeclarativeWidth = oDeclarative.HighlightRowLineWidth\n"
            "RETURN\n"
            "DEFINE CLASS DerivedGridHighlightProperties AS Grid\n"
            "    PROCEDURE Init\n"
            "        THIS.HighlightStyle = 2\n"
            "        THIS.HighlightRowLineWidth = 6\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS DeclarativeGridHighlightProperties AS Grid\n"
            "    HighlightStyle = 1\n"
            "    HighlightRowLineWidth = 5\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Grid highlight property script should complete: ") + state.message +
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

        check("ltexthasstyle", "false");
        check("ltexthaswidth", "false");
        check("lstylehas", "true");
        check("lwidthhas", "true");
        check("lstylereadonly", "false");
        check("lwidthreadonly", "false");
        check("nstyledefault", "0");
        check("nwidthdefault", "1");
        check("nstyledirect", "2");
        check("nwidthdirect", "0");
        check("lstylesetpem", "true");
        check("lwidthsetpem", "true");
        check("nstylesetpem", "1");
        check("nwidthsetpem", "7");
        check("lstyleputpem", "true");
        check("lwidthputpem", "true");
        check("nstyleputpem", "0");
        check("nwidthputpem", "7");
        check("lstyleaddproperty", "false");
        check("lstyleremoveproperty", "false");
        check("lwidthaddproperty", "false");
        check("lwidthremoveproperty", "false");
        check("lprophasstyle", "true");
        check("lprophaswidth", "true");
        check("nderivedstyle", "2");
        check("nderivedwidth", "6");
        check("ndeclarativestyle", "1");
        check("ndeclarativewidth", "5");
        expect(state.ole_objects.size() == 4U,
               "native Grid highlight coverage should register Grid, TextBox, and derived Grid objects");

        fs::remove_all(temp_root, ignored);
    }
}
