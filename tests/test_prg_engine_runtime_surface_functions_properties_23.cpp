#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_textbox_statusbartext_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_textbox_statusbartext";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_textbox_statusbartext.prg";
        write_text(
            main_path,
            "oText = CREATEOBJECT('TextBox')\n"
            "oEdit = CREATEOBJECT('EditBox')\n"
            "lTextDefault = oText.StatusBarText\n"
            "lEditHas = PEMSTATUS(oEdit, 'StatusBarText', 1)\n"
            "lTextHas = PEMSTATUS(oText, 'StatusBarText', 1)\n"
            "lReadOnly = PEMSTATUS(oText, 'StatusBarText', 5)\n"
            "oText.StatusBarText = 42\n"
            "lTextDirect = oText.StatusBarText\n"
            "lTextSetPem = SETPEM(oText, 'StatusBarText', 'Ready')\n"
            "lTextAfterSetPem = GETPEM(oText, 'StatusBarText')\n"
            "lTextPutPem = PUTPEM(oText, 'StatusBarText', 7)\n"
            "lTextAfterPutPem = GETPEM(oText, 'StatusBarText')\n"
            "lAddProperty = ADDPROPERTY(oText, 'StatusBarText', 'shadow')\n"
            "lRemoveProperty = REMOVEPROPERTY(oText, 'StatusBarText')\n"
            "nPropMembers = AMEMBERS(aPropMembers, oText, 1)\n"
            "lPropHas = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'STATUSBARTEXT'\n"
            "        lPropHas = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedStatusBarText')\n"
            "lDerived = oDerived.StatusBarText\n"
            "RETURN\n"
            "DEFINE CLASS DerivedStatusBarText AS TextBox\n"
            "    PROCEDURE Init\n"
            "        THIS.StatusBarText = 'Derived status'\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native TextBox StatusBarText script should complete: ") + state.message +
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

        check("ltextdefault", "");
        check("ledithas", "false");
        check("ltexthas", "true");
        check("lreadonly", "false");
        check("ltextdirect", "42");
        check("ltextsetpem", "true");
        check("ltextaftersetpem", "Ready");
        check("ltextputpem", "true");
        check("ltextafterputpem", "7");
        check("laddproperty", "false");
        check("lremoveproperty", "false");
        check("lprophas", "true");
        check("lderived", "Derived status");
        expect(state.ole_objects.size() == 3U,
               "native StatusBarText coverage should register TextBox, EditBox, and derived TextBox");

        fs::remove_all(temp_root, ignored);
    }
}
