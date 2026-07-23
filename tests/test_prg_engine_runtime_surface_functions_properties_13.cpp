#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_textbox_maxlength_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_textbox_maxlength";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_textbox_maxlength.prg";
        write_text(
            main_path,
            "oText = CREATEOBJECT('TextBox')\n"
            "oEdit = CREATEOBJECT('EditBox')\n"
            "lHas = PEMSTATUS(oText, 'MaxLength', 1)\n"
            "lReadOnly = PEMSTATUS(oText, 'MaxLength', 5)\n"
            "lEditHas = PEMSTATUS(oEdit, 'MaxLength', 1)\n"
            "nDefault = oText.MaxLength\n"
            "nEditDefault = oEdit.MaxLength\n"
            "oText.MaxLength = 25.7\n"
            "nDirect = oText.MaxLength\n"
            "oEdit.MaxLength = 12.7\n"
            "nEditDirect = oEdit.MaxLength\n"
            "lSetPem = SETPEM(oText, 'MaxLength', 123)\n"
            "nSetPem = GETPEM(oText, 'MaxLength')\n"
            "lEditSetPem = SETPEM(oEdit, 'MaxLength', 17.9)\n"
            "nEditSetPem = GETPEM(oEdit, 'MaxLength')\n"
            "lPutPem = PUTPEM(oText, 'MaxLength', -4)\n"
            "nPutPem = GETPEM(oText, 'MaxLength')\n"
            "lEditPutPem = PUTPEM(oEdit, 'MaxLength', -8)\n"
            "nEditPutPem = GETPEM(oEdit, 'MaxLength')\n"
            "lAddProperty = ADDPROPERTY(oText, 'MaxLength', 99)\n"
            "lRemoveProperty = REMOVEPROPERTY(oText, 'MaxLength')\n"
            "lEditAddProperty = ADDPROPERTY(oEdit, 'MaxLength', 99)\n"
            "lEditRemoveProperty = REMOVEPROPERTY(oEdit, 'MaxLength')\n"
            "lMember = .F.\n"
            "nMembers = AMEMBERS(aTextMembers, oText, 1)\n"
            "FOR i = 1 TO nMembers\n"
            "    IF UPPER(aTextMembers[i]) == 'MAXLENGTH'\n"
            "        lMember = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "lEditMember = .F.\n"
            "nEditMembers = AMEMBERS(aEditMembers, oEdit, 1)\n"
            "FOR i = 1 TO nEditMembers\n"
            "    IF UPPER(aEditMembers[i]) == 'MAXLENGTH'\n"
            "        lEditMember = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedMaxLengthText')\n"
            "nDerived = oDerived.MaxLength\n"
            "oDerivedEdit = CREATEOBJECT('DerivedMaxLengthEdit')\n"
            "nDerivedEdit = oDerivedEdit.MaxLength\n"
            "RETURN\n"
            "DEFINE CLASS DerivedMaxLengthText AS TextBox\n"
            "    PROCEDURE Init\n"
            "        THIS.MaxLength = 8.9\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS DerivedMaxLengthEdit AS EditBox\n"
            "    PROCEDURE Init\n"
            "        THIS.MaxLength = 6.9\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native MaxLength script should complete: ") + state.message +
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
        check("ledithas", "true");
        check("ndefault", "0");
        check("neditdefault", "0");
        check("ndirect", "26");
        check("neditdirect", "13");
        check("lsetpem", "true");
        check("nsetpem", "123");
        check("leditsetpem", "true");
        check("neditsetpem", "18");
        check("lputpem", "true");
        check("nputpem", "0");
        check("leditputpem", "true");
        check("neditputpem", "0");
        check("laddproperty", "false");
        check("lremoveproperty", "false");
        check("leditaddproperty", "false");
        check("leditremoveproperty", "false");
        check("lmember", "true");
        check("leditmember", "true");
        check("nderived", "9");
        check("nderivededit", "7");
        expect(state.ole_objects.size() == 4U,
               "native MaxLength coverage should register both base controls and both derived controls");

        fs::remove_all(temp_root, ignored);
    }
}
