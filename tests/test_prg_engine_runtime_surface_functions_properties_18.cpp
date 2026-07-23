#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_textbox_enablehyperlinks_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_textbox_enablehyperlinks";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_textbox_enablehyperlinks.prg";
        write_text(
            main_path,
            "oText = CREATEOBJECT('TextBox')\n"
            "oEdit = CREATEOBJECT('EditBox')\n"
            "lHas = PEMSTATUS(oText, 'EnableHyperlinks', 1)\n"
            "lReadOnly = PEMSTATUS(oText, 'EnableHyperlinks', 5)\n"
            "lEditHas = PEMSTATUS(oEdit, 'EnableHyperlinks', 1)\n"
            "lDefault = oText.EnableHyperlinks\n"
            "oText.EnableHyperlinks = .T.\n"
            "lDirect = oText.EnableHyperlinks\n"
            "lSetPem = SETPEM(oText, 'EnableHyperlinks', .F.)\n"
            "lSetPemValue = GETPEM(oText, 'EnableHyperlinks')\n"
            "lPutPem = PUTPEM(oText, 'EnableHyperlinks', 1)\n"
            "lPutPemValue = GETPEM(oText, 'EnableHyperlinks')\n"
            "lAddProperty = ADDPROPERTY(oText, 'EnableHyperlinks', .F.)\n"
            "lRemoveProperty = REMOVEPROPERTY(oText, 'EnableHyperlinks')\n"
            "lMember = .F.\n"
            "nMembers = AMEMBERS(aMembers, oText, 1)\n"
            "FOR i = 1 TO nMembers\n"
            "    IF UPPER(aMembers[i]) == 'ENABLEHYPERLINKS'\n"
            "        lMember = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedEnableHyperlinksText')\n"
            "lDerived = oDerived.EnableHyperlinks\n"
            "RETURN\n"
            "DEFINE CLASS DerivedEnableHyperlinksText AS TextBox\n"
            "    PROCEDURE Init\n"
            "        THIS.EnableHyperlinks = .T.\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native EnableHyperlinks script should complete: ") + state.message +
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
        check("ldefault", "false");
        check("ldirect", "true");
        check("lsetpem", "true");
        check("lsetpemvalue", "false");
        check("lputpem", "true");
        check("lputpemvalue", "true");
        check("laddproperty", "false");
        check("lremoveproperty", "false");
        check("lmember", "true");
        check("lderived", "true");
        expect(state.ole_objects.size() == 3U,
               "native EnableHyperlinks coverage should register the base, non-owning EditBox, and derived TextBox");

        fs::remove_all(temp_root, ignored);
    }
}
