#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_textbox_passwordchar_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_textbox_passwordchar";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_textbox_passwordchar.prg";
        write_text(
            main_path,
            "oText = CREATEOBJECT('TextBox')\n"
            "oEdit = CREATEOBJECT('EditBox')\n"
            "lHas = PEMSTATUS(oText, 'PasswordChar', 1)\n"
            "lReadOnly = PEMSTATUS(oText, 'PasswordChar', 5)\n"
            "lEditHas = PEMSTATUS(oEdit, 'PasswordChar', 1)\n"
            "cDefault = oText.PasswordChar\n"
            "oText.PasswordChar = '*'\n"
            "cDirect = oText.PasswordChar\n"
            "lSetPem = SETPEM(oText, 'PasswordChar', 123)\n"
            "cSetPem = GETPEM(oText, 'PasswordChar')\n"
            "lPutPem = PUTPEM(oText, 'PasswordChar', '#')\n"
            "cPutPem = GETPEM(oText, 'PasswordChar')\n"
            "lAddProperty = ADDPROPERTY(oText, 'PasswordChar', 'X')\n"
            "lRemoveProperty = REMOVEPROPERTY(oText, 'PasswordChar')\n"
            "lMember = .F.\n"
            "nMembers = AMEMBERS(aMembers, oText, 1)\n"
            "FOR i = 1 TO nMembers\n"
            "    IF UPPER(aMembers[i]) == 'PASSWORDCHAR'\n"
            "        lMember = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedPasswordText')\n"
            "cDerived = oDerived.PasswordChar\n"
            "RETURN\n"
            "DEFINE CLASS DerivedPasswordText AS TextBox\n"
            "    PROCEDURE Init\n"
            "        THIS.PasswordChar = '@'\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native PasswordChar script should complete: ") + state.message +
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
        check("lreadonly", "false");
        check("ledithas", "false");
        check("cdefault", "");
        check("cdirect", "*");
        check("lsetpem", "true");
        check("csetpem", "123");
        check("lputpem", "true");
        check("cputpem", "#");
        check("laddproperty", "false");
        check("lremoveproperty", "false");
        check("lmember", "true");
        check("cderived", "@");
        expect(state.ole_objects.size() == 3U,
               "native PasswordChar coverage should register the base, non-owning EditBox, and derived TextBox");

        fs::remove_all(temp_root, ignored);
    }
}
