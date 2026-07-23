#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_textbox_themes_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_textbox_themes";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_textbox_themes.prg";
        write_text(
            main_path,
            "oText = CREATEOBJECT('TextBox')\n"
            "oEdit = CREATEOBJECT('EditBox')\n"
            "lTextDefault = oText.Themes\n"
            "lEditHas = PEMSTATUS(oEdit, 'Themes', 1)\n"
            "lTextHas = PEMSTATUS(oText, 'Themes', 1)\n"
            "lReadOnly = PEMSTATUS(oText, 'Themes', 5)\n"
            "oText.Themes = 0\n"
            "lTextDirect = oText.Themes\n"
            "lTextSetPem = SETPEM(oText, 'Themes', 1)\n"
            "lTextAfterSetPem = GETPEM(oText, 'Themes')\n"
            "lTextPutPem = PUTPEM(oText, 'Themes', 0)\n"
            "lTextAfterPutPem = GETPEM(oText, 'Themes')\n"
            "lAddProperty = ADDPROPERTY(oText, 'Themes', .T.)\n"
            "lRemoveProperty = REMOVEPROPERTY(oText, 'Themes')\n"
            "nPropMembers = AMEMBERS(aPropMembers, oText, 1)\n"
            "lPropHas = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'THEMES'\n"
            "        lPropHas = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedThemes')\n"
            "lDerived = oDerived.Themes\n"
            "RETURN\n"
            "DEFINE CLASS DerivedThemes AS TextBox\n"
            "    PROCEDURE Init\n"
            "        THIS.Themes = .F.\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native TextBox Themes script should complete: ") + state.message +
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

        check("ltextdefault", "true");
        check("ledithas", "false");
        check("ltexthas", "true");
        check("lreadonly", "false");
        check("ltextdirect", "false");
        check("ltextsetpem", "true");
        check("ltextaftersetpem", "true");
        check("ltextputpem", "true");
        check("ltextafterputpem", "false");
        check("laddproperty", "false");
        check("lremoveproperty", "false");
        check("lprophas", "true");
        check("lderived", "false");
        expect(state.ole_objects.size() == 3U,
               "native Themes coverage should register TextBox, EditBox, and derived TextBox");

        fs::remove_all(temp_root, ignored);
    }
}
