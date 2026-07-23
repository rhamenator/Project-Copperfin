#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_textbox_selected_colors_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_textbox_selected_colors";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_textbox_selected_colors.prg";
        write_text(
            main_path,
            "oText = CREATEOBJECT('TextBox')\n"
            "oEdit = CREATEOBJECT('EditBox')\n"
            "lBackDefault = oText.SelectedBackColor\n"
            "lForeDefault = oText.SelectedForeColor\n"
            "lEditBackHas = PEMSTATUS(oEdit, 'SelectedBackColor', 1)\n"
            "lEditForeHas = PEMSTATUS(oEdit, 'SelectedForeColor', 1)\n"
            "lBackHas = PEMSTATUS(oText, 'SelectedBackColor', 1)\n"
            "lForeHas = PEMSTATUS(oText, 'SelectedForeColor', 1)\n"
            "lBackReadOnly = PEMSTATUS(oText, 'SelectedBackColor', 5)\n"
            "lForeReadOnly = PEMSTATUS(oText, 'SelectedForeColor', 5)\n"
            "oText.SelectedBackColor = 7.9\n"
            "lBackDirect = oText.SelectedBackColor\n"
            "oText.SelectedForeColor = -2147483643\n"
            "lForeDirect = oText.SelectedForeColor\n"
            "lBackSetPem = SETPEM(oText, 'SelectedBackColor', -2147483640)\n"
            "lBackAfterSetPem = GETPEM(oText, 'SelectedBackColor')\n"
            "lForePutPem = PUTPEM(oText, 'SelectedForeColor', 42.5)\n"
            "lForeAfterPutPem = GETPEM(oText, 'SelectedForeColor')\n"
            "lBackAddProperty = ADDPROPERTY(oText, 'SelectedBackColor', 5)\n"
            "lBackRemoveProperty = REMOVEPROPERTY(oText, 'SelectedBackColor')\n"
            "lForeAddProperty = ADDPROPERTY(oText, 'SelectedForeColor', 5)\n"
            "lForeRemoveProperty = REMOVEPROPERTY(oText, 'SelectedForeColor')\n"
            "nPropMembers = AMEMBERS(aPropMembers, oText, 1)\n"
            "lBackPropHas = .F.\n"
            "lForePropHas = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'SELECTEDBACKCOLOR'\n"
            "        lBackPropHas = .T.\n"
            "    ENDIF\n"
            "    IF UPPER(aPropMembers[i]) == 'SELECTEDFORECOLOR'\n"
            "        lForePropHas = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedSelectedColors')\n"
            "lDerivedBack = oDerived.SelectedBackColor\n"
            "lDerivedFore = oDerived.SelectedForeColor\n"
            "RETURN\n"
            "DEFINE CLASS DerivedSelectedColors AS TextBox\n"
            "    PROCEDURE Init\n"
            "        THIS.SelectedBackColor = 101.9\n"
            "        THIS.SelectedForeColor = 202.9\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native TextBox selected colors script should complete: ") + state.message +
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

        check("lbackdefault", "8421504");
        check("lforedefault", "16777215");
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
               "native selected color coverage should register TextBox, EditBox, and derived TextBox");

        fs::remove_all(temp_root, ignored);
    }
}
