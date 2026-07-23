#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_textbox_disabled_colors_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_textbox_disabled_colors";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_textbox_disabled_colors.prg";
        write_text(
            main_path,
            "oText = CREATEOBJECT('TextBox')\n"
            "oEdit = CREATEOBJECT('EditBox')\n"
            "lBackDefault = oText.DisabledBackColor\n"
            "lForeDefault = oText.DisabledForeColor\n"
            "lEditBackHas = PEMSTATUS(oEdit, 'DisabledBackColor', 1)\n"
            "lEditForeHas = PEMSTATUS(oEdit, 'DisabledForeColor', 1)\n"
            "lBackHas = PEMSTATUS(oText, 'DisabledBackColor', 1)\n"
            "lForeHas = PEMSTATUS(oText, 'DisabledForeColor', 1)\n"
            "lBackReadOnly = PEMSTATUS(oText, 'DisabledBackColor', 5)\n"
            "lForeReadOnly = PEMSTATUS(oText, 'DisabledForeColor', 5)\n"
            "oText.DisabledBackColor = 7.9\n"
            "lBackDirect = oText.DisabledBackColor\n"
            "oText.DisabledForeColor = -2147483643\n"
            "lForeDirect = oText.DisabledForeColor\n"
            "lBackSetPem = SETPEM(oText, 'DisabledBackColor', -2147483640)\n"
            "lBackAfterSetPem = GETPEM(oText, 'DisabledBackColor')\n"
            "lForePutPem = PUTPEM(oText, 'DisabledForeColor', 42.5)\n"
            "lForeAfterPutPem = GETPEM(oText, 'DisabledForeColor')\n"
            "lBackAddProperty = ADDPROPERTY(oText, 'DisabledBackColor', 5)\n"
            "lBackRemoveProperty = REMOVEPROPERTY(oText, 'DisabledBackColor')\n"
            "lForeAddProperty = ADDPROPERTY(oText, 'DisabledForeColor', 5)\n"
            "lForeRemoveProperty = REMOVEPROPERTY(oText, 'DisabledForeColor')\n"
            "nPropMembers = AMEMBERS(aPropMembers, oText, 1)\n"
            "lBackPropHas = .F.\n"
            "lForePropHas = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'DISABLEDBACKCOLOR'\n"
            "        lBackPropHas = .T.\n"
            "    ENDIF\n"
            "    IF UPPER(aPropMembers[i]) == 'DISABLEDFORECOLOR'\n"
            "        lForePropHas = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedDisabledColors')\n"
            "lDerivedBack = oDerived.DisabledBackColor\n"
            "lDerivedFore = oDerived.DisabledForeColor\n"
            "RETURN\n"
            "DEFINE CLASS DerivedDisabledColors AS TextBox\n"
            "    PROCEDURE Init\n"
            "        THIS.DisabledBackColor = 101.9\n"
            "        THIS.DisabledForeColor = 202.9\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native TextBox disabled colors script should complete: ") + state.message +
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

        check("lbackdefault", "12632256");
        check("lforedefault", "8421504");
        check("leditbackhas", "false");
        check("leditforehas", "false");
        check("lbackhas", "true");
        check("lforehas", "true");
        check("lbackreadonly", "false");
        check("lforereadonly", "false");
        check("lbackdirect", "7");
        check("lforedirect", "-2147483643");
        check("lbacksetpem", "true");
        check("lbackaftersetpem", "-2147483640");
        check("lforeputpem", "true");
        check("lforeafterputpem", "42");
        check("lbackaddproperty", "false");
        check("lbackremoveproperty", "false");
        check("lforeaddproperty", "false");
        check("lforeremoveproperty", "false");
        check("lbackprophas", "true");
        check("lforeprophas", "true");
        check("lderivedback", "101");
        check("lderivedfore", "202");
        expect(state.ole_objects.size() == 3U,
               "native disabled color coverage should register TextBox, EditBox, and derived TextBox");

        fs::remove_all(temp_root, ignored);
    }
}
