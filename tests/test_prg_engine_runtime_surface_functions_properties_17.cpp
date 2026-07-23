#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_textbox_autocomplete_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_textbox_autocomplete";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_textbox_autocomplete.prg";
        write_text(
            main_path,
            "oText = CREATEOBJECT('TextBox')\n"
            "oEdit = CREATEOBJECT('EditBox')\n"
            "lHas = PEMSTATUS(oText, 'AutoComplete', 1)\n"
            "lReadOnly = PEMSTATUS(oText, 'AutoComplete', 5)\n"
            "lEditHas = PEMSTATUS(oEdit, 'AutoComplete', 1)\n"
            "nDefault = oText.AutoComplete\n"
            "oText.AutoComplete = 1.6\n"
            "nDirect = oText.AutoComplete\n"
            "lSetPem = SETPEM(oText, 'AutoComplete', 4)\n"
            "nSetPem = GETPEM(oText, 'AutoComplete')\n"
            "lPutPem = PUTPEM(oText, 'AutoComplete', 9)\n"
            "nPutPem = GETPEM(oText, 'AutoComplete')\n"
            "oText.AutoComplete = -1\n"
            "nNegative = oText.AutoComplete\n"
            "lAddProperty = ADDPROPERTY(oText, 'AutoComplete', 2)\n"
            "lRemoveProperty = REMOVEPROPERTY(oText, 'AutoComplete')\n"
            "lMember = .F.\n"
            "nMembers = AMEMBERS(aMembers, oText, 1)\n"
            "FOR i = 1 TO nMembers\n"
            "    IF UPPER(aMembers[i]) == 'AUTOCOMPLETE'\n"
            "        lMember = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedAutoCompleteText')\n"
            "nDerived = oDerived.AutoComplete\n"
            "RETURN\n"
            "DEFINE CLASS DerivedAutoCompleteText AS TextBox\n"
            "    PROCEDURE Init\n"
            "        THIS.AutoComplete = 3\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native AutoComplete script should complete: ") + state.message +
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
        check("ndefault", "0");
        check("ndirect", "2");
        check("lsetpem", "true");
        check("nsetpem", "4");
        check("lputpem", "true");
        check("nputpem", "0");
        check("nnegative", "0");
        check("laddproperty", "false");
        check("lremoveproperty", "false");
        check("lmember", "true");
        check("nderived", "3");
        expect(state.ole_objects.size() == 3U,
               "native AutoComplete coverage should register the base, non-owning EditBox, and derived TextBox");

        fs::remove_all(temp_root, ignored);
    }
}
