#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_textbox_format_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_textbox_format";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_textbox_format.prg";
        write_text(
            main_path,
            "oText = CREATEOBJECT('TextBox')\n"
            "oEdit = CREATEOBJECT('EditBox')\n"
            "lHas = PEMSTATUS(oText, 'Format', 1)\n"
            "lReadOnly = PEMSTATUS(oText, 'Format', 5)\n"
            "lEditHas = PEMSTATUS(oEdit, 'Format', 1)\n"
            "cDefault = oText.Format\n"
            "oText.Format = '@!'\n"
            "cDirect = oText.Format\n"
            "lSetPem = SETPEM(oText, 'Format', 123)\n"
            "cSetPem = GETPEM(oText, 'Format')\n"
            "lPutPem = PUTPEM(oText, 'Format', '@R')\n"
            "cPutPem = GETPEM(oText, 'Format')\n"
            "lAddProperty = ADDPROPERTY(oText, 'Format', 'X')\n"
            "lRemoveProperty = REMOVEPROPERTY(oText, 'Format')\n"
            "lMember = .F.\n"
            "nMembers = AMEMBERS(aMembers, oText, 1)\n"
            "FOR i = 1 TO nMembers\n"
            "    IF UPPER(aMembers[i]) == 'FORMAT'\n"
            "        lMember = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedFormatText')\n"
            "cDerived = oDerived.Format\n"
            "RETURN\n"
            "DEFINE CLASS DerivedFormatText AS TextBox\n"
            "    PROCEDURE Init\n"
            "        THIS.Format = '@L'\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Format script should complete: ") + state.message +
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
        check("cdefault", "");
        check("cdirect", "@!");
        check("lsetpem", "true");
        check("csetpem", "123");
        check("lputpem", "true");
        check("cputpem", "@R");
        check("laddproperty", "false");
        check("lremoveproperty", "false");
        check("lmember", "true");
        check("cderived", "@L");
        expect(state.ole_objects.size() == 3U,
               "native Format coverage should register the base, non-owning EditBox, and derived TextBox");

        fs::remove_all(temp_root, ignored);
    }
}
