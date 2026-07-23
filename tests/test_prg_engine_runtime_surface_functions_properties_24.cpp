#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_textbox_strictdateentry_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_textbox_strictdateentry";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_textbox_strictdateentry.prg";
        write_text(
            main_path,
            "oText = CREATEOBJECT('TextBox')\n"
            "oEdit = CREATEOBJECT('EditBox')\n"
            "lTextDefault = oText.StrictDateEntry\n"
            "lEditHas = PEMSTATUS(oEdit, 'StrictDateEntry', 1)\n"
            "lTextHas = PEMSTATUS(oText, 'StrictDateEntry', 1)\n"
            "lReadOnly = PEMSTATUS(oText, 'StrictDateEntry', 5)\n"
            "oText.StrictDateEntry = 0\n"
            "lTextDirect = oText.StrictDateEntry\n"
            "lTextSetPem = SETPEM(oText, 'StrictDateEntry', 0.2)\n"
            "lTextAfterSetPem = GETPEM(oText, 'StrictDateEntry')\n"
            "lTextPutPem = PUTPEM(oText, 'StrictDateEntry', 9)\n"
            "lTextAfterPutPem = GETPEM(oText, 'StrictDateEntry')\n"
            "oText.StrictDateEntry = -4\n"
            "lTextInvalid = oText.StrictDateEntry\n"
            "lAddProperty = ADDPROPERTY(oText, 'StrictDateEntry', 0)\n"
            "lRemoveProperty = REMOVEPROPERTY(oText, 'StrictDateEntry')\n"
            "nPropMembers = AMEMBERS(aPropMembers, oText, 1)\n"
            "lPropHas = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'STRICTDATEENTRY'\n"
            "        lPropHas = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedStrictDateEntry')\n"
            "lDerived = oDerived.StrictDateEntry\n"
            "RETURN\n"
            "DEFINE CLASS DerivedStrictDateEntry AS TextBox\n"
            "    PROCEDURE Init\n"
            "        THIS.StrictDateEntry = 0\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native TextBox StrictDateEntry script should complete: ") + state.message +
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

        check("ltextdefault", "1");
        check("ledithas", "false");
        check("ltexthas", "true");
        check("lreadonly", "false");
        check("ltextdirect", "0");
        check("ltextsetpem", "true");
        check("ltextaftersetpem", "0");
        check("ltextputpem", "true");
        check("ltextafterputpem", "1");
        check("ltextinvalid", "1");
        check("laddproperty", "false");
        check("lremoveproperty", "false");
        check("lprophas", "true");
        check("lderived", "0");
        expect(state.ole_objects.size() == 3U,
               "native StrictDateEntry coverage should register TextBox, EditBox, and derived TextBox");

        fs::remove_all(temp_root, ignored);
    }
}
