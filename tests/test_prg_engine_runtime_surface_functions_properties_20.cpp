#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_textbox_margin_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_textbox_margin";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_textbox_margin.prg";
        write_text(
            main_path,
            "oText = CREATEOBJECT('TextBox')\n"
            "oEdit = CREATEOBJECT('EditBox')\n"
            "lTextDefault = oText.Margin\n"
            "lEditHas = PEMSTATUS(oEdit, 'Margin', 1)\n"
            "lTextHas = PEMSTATUS(oText, 'Margin', 1)\n"
            "lReadOnly = PEMSTATUS(oText, 'Margin', 5)\n"
            "oText.Margin = 3.6\n"
            "lTextDirect = oText.Margin\n"
            "lTextSetPem = SETPEM(oText, 'Margin', 8.2)\n"
            "lTextAfterSetPem = GETPEM(oText, 'Margin')\n"
            "lTextPutPem = PUTPEM(oText, 'Margin', -4)\n"
            "lTextAfterPutPem = GETPEM(oText, 'Margin')\n"
            "oText.Margin = -1\n"
            "lTextNormalized = oText.Margin\n"
            "lAddProperty = ADDPROPERTY(oText, 'Margin', 5)\n"
            "lRemoveProperty = REMOVEPROPERTY(oText, 'Margin')\n"
            "nPropMembers = AMEMBERS(aPropMembers, oText, 1)\n"
            "lPropHas = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'MARGIN'\n"
            "        lPropHas = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedMarginTextBox')\n"
            "lDerived = oDerived.Margin\n"
            "RETURN\n"
            "DEFINE CLASS DerivedMarginTextBox AS TextBox\n"
            "    PROCEDURE Init\n"
            "        THIS.Margin = 11.9\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native TextBox Margin script should complete: ") + state.message +
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

        check("ltextdefault", "0");
        check("ledithas", "false");
        check("ltexthas", "true");
        check("lreadonly", "false");
        check("ltextdirect", "4");
        check("ltextsetpem", "true");
        check("ltextaftersetpem", "8");
        check("ltextputpem", "true");
        check("ltextafterputpem", "0");
        check("ltextnormalized", "0");
        check("laddproperty", "false");
        check("lremoveproperty", "false");
        check("lprophas", "true");
        check("lderived", "12");
        expect(state.ole_objects.size() == 3U,
               "native Margin coverage should register TextBox, EditBox, and derived TextBox");

        fs::remove_all(temp_root, ignored);
    }
}
