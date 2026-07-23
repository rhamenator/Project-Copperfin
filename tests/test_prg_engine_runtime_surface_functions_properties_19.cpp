#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_textbox_tooltiptext_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_textbox_tooltiptext";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_textbox_tooltiptext.prg";
        write_text(
            main_path,
            "oText = CREATEOBJECT('TextBox')\n"
            "oEdit = CREATEOBJECT('EditBox')\n"
            "lTextDefault = oText.ToolTipText\n"
            "lEditHas = PEMSTATUS(oEdit, 'ToolTipText', 1)\n"
            "lTextHas = PEMSTATUS(oText, 'ToolTipText', 1)\n"
            "lReadOnly = PEMSTATUS(oText, 'ToolTipText', 5)\n"
            "oText.ToolTipText = 42\n"
            "lTextDirect = oText.ToolTipText\n"
            "lTextSetPem = SETPEM(oText, 'ToolTipText', 'Hover text')\n"
            "lTextAfterSetPem = GETPEM(oText, 'ToolTipText')\n"
            "lTextPutPem = PUTPEM(oText, 'ToolTipText', 7)\n"
            "lTextAfterPutPem = GETPEM(oText, 'ToolTipText')\n"
            "lAddProperty = ADDPROPERTY(oText, 'ToolTipText', 'shadow')\n"
            "lRemoveProperty = REMOVEPROPERTY(oText, 'ToolTipText')\n"
            "nPropMembers = AMEMBERS(aPropMembers, oText, 1)\n"
            "lPropHas = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'TOOLTIPTEXT'\n"
            "        lPropHas = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedToolTipText')\n"
            "lDerived = oDerived.ToolTipText\n"
            "RETURN\n"
            "DEFINE CLASS DerivedToolTipText AS TextBox\n"
            "    PROCEDURE Init\n"
            "        THIS.ToolTipText = 'Derived tip'\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native TextBox ToolTipText script should complete: ") + state.message +
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
        check("ltextaftersetpem", "Hover text");
        check("ltextputpem", "true");
        check("ltextafterputpem", "7");
        check("laddproperty", "false");
        check("lremoveproperty", "false");
        check("lprophas", "true");
        check("lderived", "Derived tip");
        expect(state.ole_objects.size() == 3U,
               "native ToolTipText coverage should register TextBox, EditBox, and derived TextBox");

        fs::remove_all(temp_root, ignored);
    }
}
