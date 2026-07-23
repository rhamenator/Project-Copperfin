#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_grid_headerheight_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_grid_headerheight";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_grid_headerheight.prg";
        write_text(
            main_path,
            "oGrid = CREATEOBJECT('Grid')\n"
            "oText = CREATEOBJECT('TextBox')\n"
            "lTextHas = PEMSTATUS(oText, 'HeaderHeight', 1)\n"
            "lHas = PEMSTATUS(oGrid, 'HeaderHeight', 1)\n"
            "lReadOnly = PEMSTATUS(oGrid, 'HeaderHeight', 5)\n"
            "nDefault = oGrid.HeaderHeight\n"
            "oGrid.HeaderHeight = 14.4\n"
            "nDirect = oGrid.HeaderHeight\n"
            "lSetPem = SETPEM(oGrid, 'HeaderHeight', 31.8)\n"
            "nSetPem = GETPEM(oGrid, 'HeaderHeight')\n"
            "lPutPem = PUTPEM(oGrid, 'HeaderHeight', -2)\n"
            "nPutPem = GETPEM(oGrid, 'HeaderHeight')\n"
            "lAddProperty = ADDPROPERTY(oGrid, 'HeaderHeight', 7)\n"
            "lRemoveProperty = REMOVEPROPERTY(oGrid, 'HeaderHeight')\n"
            "nPropMembers = AMEMBERS(aPropMembers, oGrid, 1)\n"
            "lPropHas = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'HEADERHEIGHT'\n"
            "        lPropHas = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedGridHeaderHeight')\n"
            "nDerived = oDerived.HeaderHeight\n"
            "oDeclarative = CREATEOBJECT('DeclarativeGridHeaderHeight')\n"
            "nDeclarative = oDeclarative.HeaderHeight\n"
            "RETURN\n"
            "DEFINE CLASS DerivedGridHeaderHeight AS Grid\n"
            "    PROCEDURE Init\n"
            "        THIS.HeaderHeight = 18.7\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS DeclarativeGridHeaderHeight AS Grid\n"
            "    HeaderHeight = 17.4\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Grid HeaderHeight script should complete: ") + state.message +
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
        check("ndefault", "20");
        check("ndirect", "14");
        check("lsetpem", "true");
        check("nsetpem", "32");
        check("lputpem", "true");
        check("nputpem", "0");
        check("laddproperty", "false");
        check("lremoveproperty", "false");
        check("lprophas", "true");
        check("nderived", "19");
        check("ndeclarative", "17");
        expect(state.ole_objects.size() == 4U,
               "native HeaderHeight coverage should register Grid, TextBox, and derived Grid objects");

        fs::remove_all(temp_root, ignored);
    }
}
