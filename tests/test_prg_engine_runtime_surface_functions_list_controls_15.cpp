#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_text_controls_integralheight_property_stays_read_only()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_text_integralheight";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_text_integralheight.prg";
        write_text(
            main_path,
            "oEdit = CREATEOBJECT('EditBox')\n"
            "oText = CREATEOBJECT('TextBox')\n"
            "lEditDefault = oEdit.IntegralHeight\n"
            "lTextDefault = oText.IntegralHeight\n"
            "lEditHas = PEMSTATUS(oEdit, 'IntegralHeight', 1)\n"
            "lTextHas = PEMSTATUS(oText, 'IntegralHeight', 1)\n"
            "lEditReadOnly = PEMSTATUS(oEdit, 'IntegralHeight', 5)\n"
            "lTextReadOnly = PEMSTATUS(oText, 'IntegralHeight', 5)\n"
            "oEdit.IntegralHeight = .T.\n"
            "lEditAfterDirect = oEdit.IntegralHeight\n"
            "lTextSetPem = SETPEM(oText, 'IntegralHeight', .T.)\n"
            "lTextAfterSetPem = GETPEM(oText, 'IntegralHeight')\n"
            "lEditPutPem = PUTPEM(oEdit, 'IntegralHeight', .T.)\n"
            "lEditAfterPutPem = GETPEM(oEdit, 'IntegralHeight')\n"
            "lEditAddProperty = ADDPROPERTY(oEdit, 'IntegralHeight', .T.)\n"
            "lTextRemoveProperty = REMOVEPROPERTY(oText, 'IntegralHeight')\n"
            "nPropMembers = AMEMBERS(aPropMembers, oText, 1)\n"
            "lPropHas = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'INTEGRALHEIGHT'\n"
            "        lPropHas = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedIntegralHeightText')\n"
            "lDerived = oDerived.IntegralHeight\n"
            "RETURN\n"
            "DEFINE CLASS DerivedIntegralHeightText AS TextBox\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native text IntegralHeight script should complete: ") + state.message +
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

        check("leditdefault", "false");
        check("ltextdefault", "false");
        check("ledithas", "true");
        check("ltexthas", "true");
        check("leditreadonly", "true");
        check("ltextreadonly", "true");
        check("leditafterdirect", "false");
        check("ltextsetpem", "false");
        check("ltextaftersetpem", "false");
        check("leditputpem", "false");
        check("leditafterputpem", "false");
        check("leditaddproperty", "false");
        check("ltextremoveproperty", "false");
        check("lprophas", "true");
        check("lderived", "false");
        expect(state.ole_objects.size() == 3U,
               "native text IntegralHeight coverage should register EditBox, TextBox, and derived TextBox");

        fs::remove_all(temp_root, ignored);
    }
}
