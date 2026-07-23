#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_textbox_borderstyle_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_textbox_borderstyle";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_textbox_borderstyle.prg";
        write_text(
            main_path,
            "oText = CREATEOBJECT('TextBox')\n"
            "oEdit = CREATEOBJECT('EditBox')\n"
            "lHas = PEMSTATUS(oText, 'BorderStyle', 1)\n"
            "lReadOnly = PEMSTATUS(oText, 'BorderStyle', 5)\n"
            "lEditHas = PEMSTATUS(oEdit, 'BorderStyle', 1)\n"
            "nDefault = oText.BorderStyle\n"
            "oText.BorderStyle = 0.4\n"
            "nDirect = oText.BorderStyle\n"
            "lSetPem = SETPEM(oText, 'BorderStyle', 1)\n"
            "nSetPem = GETPEM(oText, 'BorderStyle')\n"
            "lPutPem = PUTPEM(oText, 'BorderStyle', 9)\n"
            "nPutPem = GETPEM(oText, 'BorderStyle')\n"
            "oText.BorderStyle = -2\n"
            "nNegative = oText.BorderStyle\n"
            "lAddProperty = ADDPROPERTY(oText, 'BorderStyle', 0)\n"
            "lRemoveProperty = REMOVEPROPERTY(oText, 'BorderStyle')\n"
            "lMember = .F.\n"
            "nMembers = AMEMBERS(aMembers, oText, 1)\n"
            "FOR i = 1 TO nMembers\n"
            "    IF UPPER(aMembers[i]) == 'BORDERSTYLE'\n"
            "        lMember = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedBorderStyleText')\n"
            "nDerived = oDerived.BorderStyle\n"
            "RETURN\n"
            "DEFINE CLASS DerivedBorderStyleText AS TextBox\n"
            "    PROCEDURE Init\n"
            "        THIS.BorderStyle = 0\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native BorderStyle script should complete: ") + state.message +
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
        check("ndefault", "1");
        check("ndirect", "0");
        check("lsetpem", "true");
        check("nsetpem", "1");
        check("lputpem", "true");
        check("nputpem", "1");
        check("nnegative", "1");
        check("laddproperty", "false");
        check("lremoveproperty", "false");
        check("lmember", "true");
        check("nderived", "0");
        expect(state.ole_objects.size() == 3U,
               "native BorderStyle coverage should register the base, non-owning EditBox, and derived TextBox");

        fs::remove_all(temp_root, ignored);
    }
}
