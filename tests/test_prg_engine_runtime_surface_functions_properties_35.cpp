#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_visual_whatsthishelpid_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_visual_whatsthishelpid";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_visual_whatsthishelpid.prg";
        write_text(
            main_path,
            "oText = CREATEOBJECT('TextBox')\n"
            "oEdit = CREATEOBJECT('EditBox')\n"
            "oButton = CREATEOBJECT('CommandButton')\n"
            "oForm = CREATEOBJECT('Form')\n"
            "lTextDefault = oText.WhatsThisHelpID\n"
            "lEditHas = PEMSTATUS(oEdit, 'WhatsThisHelpID', 1)\n"
            "lTextHas = PEMSTATUS(oText, 'WhatsThisHelpID', 1)\n"
            "lReadOnly = PEMSTATUS(oText, 'WhatsThisHelpID', 5)\n"
            "oText.WhatsThisHelpID = 7.9\n"
            "lTextDirect = oText.WhatsThisHelpID\n"
            "lTextSetPem = SETPEM(oText, 'WhatsThisHelpID', '19')\n"
            "lTextAfterSetPem = GETPEM(oText, 'WhatsThisHelpID')\n"
            "lButtonPutPem = PUTPEM(oButton, 'WhatsThisHelpID', 42.4)\n"
            "lButtonAfterPutPem = GETPEM(oButton, 'WhatsThisHelpID')\n"
            "oEdit.WhatsThisHelpID = -3\n"
            "lEditNegative = oEdit.WhatsThisHelpID\n"
            "oForm.WhatsThisHelpID = .NULL.\n"
            "lFormNull = oForm.WhatsThisHelpID\n"
            "lAddProperty = ADDPROPERTY(oText, 'WhatsThisHelpID', 99)\n"
            "lRemoveProperty = REMOVEPROPERTY(oText, 'WhatsThisHelpID')\n"
            "nPropMembers = AMEMBERS(aPropMembers, oText, 1)\n"
            "lPropHas = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'WHATSTHISHELPID'\n"
            "        lPropHas = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedWhatsThisHelp')\n"
            "lDerived = oDerived.WhatsThisHelpID\n"
            "RETURN\n"
            "DEFINE CLASS DerivedWhatsThisHelp AS CommandButton\n"
            "    PROCEDURE Init\n"
            "        THIS.WhatsThisHelpID = 88.6\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native visual WhatsThisHelpID script should complete: ") + state.message +
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
        check("ledithas", "true");
        check("ltexthas", "true");
        check("lreadonly", "false");
        check("ltextdirect", "8");
        check("ltextsetpem", "true");
        check("ltextaftersetpem", "19");
        check("lbuttonputpem", "true");
        check("lbuttonafterputpem", "42");
        check("leditnegative", "0");
        check("lformnull", "0");
        check("laddproperty", "false");
        check("lremoveproperty", "false");
        check("lprophas", "true");
        check("lderived", "89");
        expect(state.ole_objects.size() == 5U,
               "native WhatsThisHelpID coverage should register the four base visuals and derived CommandButton");

        fs::remove_all(temp_root, ignored);
    }
}
