#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_visual_dynamicfontstyle_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_column_dynamicfontstyle";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_column_dynamicfontstyle.prg";
        write_text(
            main_path,
            "oColumn = CREATEOBJECT('Column')\n"
            "oHeader = CREATEOBJECT('Header')\n"
            "oControl = CREATEOBJECT('CommandButton')\n"
            "lItalicHas = PEMSTATUS(oColumn, 'DynamicFontItalic', 1)\n"
            "lStrikeHas = PEMSTATUS(oColumn, 'DynamicFontStrikeThru', 1)\n"
            "lUnderlineHas = PEMSTATUS(oColumn, 'DynamicFontUnderline', 1)\n"
            "lHeaderHas = PEMSTATUS(oHeader, 'DynamicFontItalic', 1)\n"
            "lControlHas = PEMSTATUS(oControl, 'DynamicFontUnderline', 1)\n"
            "cItalicDefault = oColumn.DynamicFontItalic\n"
            "cStrikeDefault = oColumn.DynamicFontStrikeThru\n"
            "cUnderlineDefault = oColumn.DynamicFontUnderline\n"
            "oColumn.DynamicFontItalic = 'Value > 0'\n"
            "cItalicDirect = oColumn.DynamicFontItalic\n"
            "lStrikeSetPem = SETPEM(oColumn, 'DynamicFontStrikeThru', 'RECNO() = 1')\n"
            "cStrikeAfterSetPem = GETPEM(oColumn, 'DynamicFontStrikeThru')\n"
            "lUnderlinePutPem = PUTPEM(oColumn, 'DynamicFontUnderline', 'Value = 1')\n"
            "cUnderlineAfterPutPem = GETPEM(oColumn, 'DynamicFontUnderline')\n"
            "lItalicAdd = ADDPROPERTY(oColumn, 'DynamicFontItalic', 'shadow')\n"
            "lStrikeRemove = REMOVEPROPERTY(oColumn, 'DynamicFontStrikeThru')\n"
            "lUnderlineAdd = ADDPROPERTY(oColumn, 'DynamicFontUnderline', 'shadow')\n"
            "lItalicMember = .F.\n"
            "lStrikeMember = .F.\n"
            "lUnderlineMember = .F.\n"
            "nMembers = AMEMBERS(aMembers, oColumn, 1)\n"
            "FOR i = 1 TO nMembers\n"
            "    IF UPPER(aMembers[i]) == 'DYNAMICFONTITALIC'\n"
            "        lItalicMember = .T.\n"
            "    ENDIF\n"
            "    IF UPPER(aMembers[i]) == 'DYNAMICFONTSTRIKETHRU'\n"
            "        lStrikeMember = .T.\n"
            "    ENDIF\n"
            "    IF UPPER(aMembers[i]) == 'DYNAMICFONTUNDERLINE'\n"
            "        lUnderlineMember = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oGrid = CREATEOBJECT('Grid')\n"
            "oGrid.ColumnCount = 1\n"
            "oGrid.Column1.DynamicFontItalic = 'grid-italic'\n"
            "cGrid = oGrid.Column1.DynamicFontItalic\n"
            "oDerived = CREATEOBJECT('DerivedDynamicFontStyleColumn')\n"
            "cDerived = oDerived.DynamicFontUnderline\n"
            "RETURN\n"
            "DEFINE CLASS DerivedDynamicFontStyleColumn AS Column\n"
            "    DynamicFontUnderline = 'derived-underline'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native dynamic font-style script should complete: ") + state.message +
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

        check("litalichas", "true");
        check("lstrikehas", "true");
        check("lunderlinehas", "true");
        check("lheaderhas", "false");
        check("lcontrolhas", "false");
        check("citalicdefault", ".F.");
        check("cstrikedefault", ".F.");
        check("cunderlinedefault", ".F.");
        check("citalicdirect", "Value > 0");
        check("lstrikesetpem", "true");
        check("cstrikeaftersetpem", "RECNO() = 1");
        check("lunderlineputpem", "true");
        check("cunderlineafterputpem", "Value = 1");
        check("litalicadd", "false");
        check("lstrikeremove", "false");
        check("lunderlineadd", "false");
        check("litalicmember", "true");
        check("lstrikemember", "true");
        check("lunderlinemember", "true");
        check("cgrid", "grid-italic");
        check("cderived", "derived-underline");
        expect(state.ole_objects.size() >= 5U,
               "native dynamic font-style coverage should register the column, excluded objects, grid, and derived column");

        fs::remove_all(temp_root, ignored);
    }
}
