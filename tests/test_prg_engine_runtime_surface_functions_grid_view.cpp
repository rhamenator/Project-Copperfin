#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_grid_view_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_grid_view";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_grid_view.prg";
        write_text(
            main_path,
            "oGrid = CREATEOBJECT('Grid')\n"
            "oText = CREATEOBJECT('TextBox')\n"
            "lTextHas = PEMSTATUS(oText, 'View', 1)\n"
            "lHas = PEMSTATUS(oGrid, 'View', 1)\n"
            "lReadOnly = PEMSTATUS(oGrid, 'View', 5)\n"
            "nDefault = oGrid.View\n"
            "oGrid.View = 3\n"
            "nUnsplitDirect = oGrid.View\n"
            "lSetPem = SETPEM(oGrid, 'View', -1)\n"
            "nAfterSetPem = GETPEM(oGrid, 'View')\n"
            "lPutPem = PUTPEM(oGrid, 'View', 1)\n"
            "nAfterPutPem = GETPEM(oGrid, 'View')\n"
            "oGrid.Partition = 120\n"
            "oGrid.View = 3\n"
            "nSplitView = oGrid.View\n"
            "lAddProperty = ADDPROPERTY(oGrid, 'View', 0)\n"
            "lRemoveProperty = REMOVEPROPERTY(oGrid, 'View')\n"
            "nPropMembers = AMEMBERS(aPropMembers, oGrid, 1)\n"
            "lPropHas = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'VIEW'\n"
            "        lPropHas = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedGridView')\n"
            "nDerived = oDerived.View\n"
            "oDeclarative = CREATEOBJECT('DeclarativeGridView')\n"
            "nDeclarative = oDeclarative.View\n"
            "RETURN\n"
            "DEFINE CLASS DerivedGridView AS Grid\n"
            "    PROCEDURE Init\n"
            "        THIS.View = 2\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS DeclarativeGridView AS Grid\n"
            "    View = 1\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Grid View script should complete: ") + state.message +
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
        check("ndefault", "0");
        check("nunsplitdirect", "1");
        check("lsetpem", "true");
        check("naftersetpem", "0");
        check("lputpem", "true");
        check("nafterputpem", "1");
        check("nsplitview", "3");
        check("laddproperty", "false");
        check("lremoveproperty", "false");
        check("lprophas", "true");
        check("nderived", "1");
        check("ndeclarative", "1");
        expect(state.ole_objects.size() == 4U,
               "native Grid View coverage should register Grid, TextBox, and both derived Grid objects");

        fs::remove_all(temp_root, ignored);
    }
}
