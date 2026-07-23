#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_textbox_dateformat_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_textbox_dateformat";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_textbox_dateformat.prg";
        write_text(
            main_path,
            "oText = CREATEOBJECT('TextBox')\n"
            "oEdit = CREATEOBJECT('EditBox')\n"
            "lDefault = oText.DateFormat\n"
            "lEditHas = PEMSTATUS(oEdit, 'DateFormat', 1)\n"
            "lHas = PEMSTATUS(oText, 'DateFormat', 1)\n"
            "lReadOnly = PEMSTATUS(oText, 'DateFormat', 5)\n"
            "oText.DateFormat = 14.4\n"
            "lDirect = oText.DateFormat\n"
            "lSetPem = SETPEM(oText, 'DateFormat', 3.6)\n"
            "lAfterSetPem = GETPEM(oText, 'DateFormat')\n"
            "lPutPem = PUTPEM(oText, 'DateFormat', -1)\n"
            "lAfterPutPem = GETPEM(oText, 'DateFormat')\n"
            "lAddProperty = ADDPROPERTY(oText, 'DateFormat', 5)\n"
            "lRemoveProperty = REMOVEPROPERTY(oText, 'DateFormat')\n"
            "nPropMembers = AMEMBERS(aPropMembers, oText, 1)\n"
            "lPropHas = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'DATEFORMAT'\n"
            "        lPropHas = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedDateFormat')\n"
            "lDerived = oDerived.DateFormat\n"
            "RETURN\n"
            "DEFINE CLASS DerivedDateFormat AS TextBox\n"
            "    PROCEDURE Init\n"
            "        THIS.DateFormat = 7.6\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native TextBox DateFormat script should complete: ") + state.message +
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
        check("ldirect", "14");
        check("lsetpem", "true");
        check("laftersetpem", "4");
        check("lputpem", "true");
        check("lafterputpem", "0");
        check("laddproperty", "false");
        check("lremoveproperty", "false");
        check("lprophas", "true");
        check("lderived", "8");
        expect(state.ole_objects.size() == 3U,
               "native DateFormat coverage should register TextBox, EditBox, and derived TextBox");

        fs::remove_all(temp_root, ignored);
    }
}
