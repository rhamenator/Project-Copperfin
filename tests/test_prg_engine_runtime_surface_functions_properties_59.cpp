#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_visual_dynamicfontbold_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_column_dynamicfontbold";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_column_dynamicfontbold.prg";
        write_text(
            main_path,
            "oColumn = CREATEOBJECT('Column')\n"
            "oHeader = CREATEOBJECT('Header')\n"
            "oControl = CREATEOBJECT('CommandButton')\n"
            "lHas = PEMSTATUS(oColumn, 'DynamicFontBold', 1)\n"
            "lHeaderHas = PEMSTATUS(oHeader, 'DynamicFontBold', 1)\n"
            "lControlHas = PEMSTATUS(oControl, 'DynamicFontBold', 1)\n"
            "lReadOnly = PEMSTATUS(oColumn, 'DynamicFontBold', 5)\n"
            "cDefault = oColumn.DynamicFontBold\n"
            "oColumn.DynamicFontBold = 'IIF(Value > 0, .T., .F.)'\n"
            "cDirect = oColumn.DynamicFontBold\n"
            "lSetPem = SETPEM(oColumn, 'DynamicFontBold', 'Value = 1')\n"
            "cSetPem = GETPEM(oColumn, 'DynamicFontBold')\n"
            "lPutPem = PUTPEM(oColumn, 'DynamicFontBold', 'TRANSFORM(Value)')\n"
            "cPutPem = GETPEM(oColumn, 'DynamicFontBold')\n"
            "lAddProperty = ADDPROPERTY(oColumn, 'DynamicFontBold', 'shadow')\n"
            "lRemoveProperty = REMOVEPROPERTY(oColumn, 'DynamicFontBold')\n"
            "lMember = .F.\n"
            "nMembers = AMEMBERS(aMembers, oColumn, 1)\n"
            "FOR i = 1 TO nMembers\n"
            "    IF UPPER(aMembers[i]) == 'DYNAMICFONTBOLD'\n"
            "        lMember = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oGrid = CREATEOBJECT('Grid')\n"
            "oGrid.ColumnCount = 1\n"
            "oGrid.Column1.DynamicFontBold = 'grid-bold'\n"
            "cGrid = oGrid.Column1.DynamicFontBold\n"
            "oDerived = CREATEOBJECT('DerivedDynamicFontBoldColumn')\n"
            "cDerived = oDerived.DynamicFontBold\n"
            "RETURN\n"
            "DEFINE CLASS DerivedDynamicFontBoldColumn AS Column\n"
            "    DynamicFontBold = 'derived-bold'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native DynamicFontBold script should complete: ") + state.message +
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

        check("lhas", "true");
        check("lheaderhas", "false");
        check("lcontrolhas", "false");
        check("lreadonly", "false");
        check("cdefault", "");
        check("cdirect", "IIF(Value > 0, .T., .F.)");
        check("lsetpem", "true");
        check("csetpem", "Value = 1");
        check("lputpem", "true");
        check("cputpem", "TRANSFORM(Value)");
        check("laddproperty", "false");
        check("lremoveproperty", "false");
        check("lmember", "true");
        check("cgrid", "grid-bold");
        check("cderived", "derived-bold");
        expect(state.ole_objects.size() >= 5U,
               "native DynamicFontBold coverage should register the column, excluded objects, grid, and derived column");

        fs::remove_all(temp_root, ignored);
    }
}
