#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_textbox_hours_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_textbox_hours";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_textbox_hours.prg";
        write_text(
            main_path,
            "oText = CREATEOBJECT('TextBox')\n"
            "oEdit = CREATEOBJECT('EditBox')\n"
            "lDefault = oText.Hours\n"
            "lEditHas = PEMSTATUS(oEdit, 'Hours', 1)\n"
            "lHas = PEMSTATUS(oText, 'Hours', 1)\n"
            "lReadOnly = PEMSTATUS(oText, 'Hours', 5)\n"
            "oText.Hours = 12.4\n"
            "lDirect = oText.Hours\n"
            "lSetPem = SETPEM(oText, 'Hours', 24.4)\n"
            "lAfterSetPem = GETPEM(oText, 'Hours')\n"
            "lPutPem = PUTPEM(oText, 'Hours', 99)\n"
            "lAfterPutPem = GETPEM(oText, 'Hours')\n"
            "lAddProperty = ADDPROPERTY(oText, 'Hours', 12)\n"
            "lRemoveProperty = REMOVEPROPERTY(oText, 'Hours')\n"
            "nPropMembers = AMEMBERS(aPropMembers, oText, 1)\n"
            "lPropHas = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'HOURS'\n"
            "        lPropHas = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedHours')\n"
            "lDerived = oDerived.Hours\n"
            "RETURN\n"
            "DEFINE CLASS DerivedHours AS TextBox\n"
            "    PROCEDURE Init\n"
            "        THIS.Hours = 23.6\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native TextBox Hours script should complete: ") + state.message +
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

        check("ldefault", "0");
        check("ledithas", "false");
        check("lhas", "true");
        check("lreadonly", "false");
        check("ldirect", "12");
        check("lsetpem", "true");
        check("laftersetpem", "24");
        check("lputpem", "true");
        check("lafterputpem", "0");
        check("laddproperty", "false");
        check("lremoveproperty", "false");
        check("lprophas", "true");
        check("lderived", "24");
        expect(state.ole_objects.size() == 3U,
               "native Hours coverage should register TextBox, EditBox, and derived TextBox");

        fs::remove_all(temp_root, ignored);
    }
}
