#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_textbox_hideselection_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_textbox_hideselection";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_textbox_hideselection.prg";
        write_text(
            main_path,
            "oText = CREATEOBJECT('TextBox')\n"
            "oEdit = CREATEOBJECT('EditBox')\n"
            "lHas = PEMSTATUS(oText, 'HideSelection', 1)\n"
            "lReadOnly = PEMSTATUS(oText, 'HideSelection', 5)\n"
            "lEditHas = PEMSTATUS(oEdit, 'HideSelection', 1)\n"
            "lDefault = oText.HideSelection\n"
            "oText.HideSelection = .F.\n"
            "lDirect = oText.HideSelection\n"
            "lSetPem = SETPEM(oText, 'HideSelection', .T.)\n"
            "lSetPemValue = GETPEM(oText, 'HideSelection')\n"
            "lPutPem = PUTPEM(oText, 'HideSelection', 0)\n"
            "lPutPemValue = GETPEM(oText, 'HideSelection')\n"
            "lAddProperty = ADDPROPERTY(oText, 'HideSelection', .F.)\n"
            "lRemoveProperty = REMOVEPROPERTY(oText, 'HideSelection')\n"
            "lMember = .F.\n"
            "nMembers = AMEMBERS(aMembers, oText, 1)\n"
            "FOR i = 1 TO nMembers\n"
            "    IF UPPER(aMembers[i]) == 'HIDESELECTION'\n"
            "        lMember = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedHideSelectionText')\n"
            "lDerived = oDerived.HideSelection\n"
            "RETURN\n"
            "DEFINE CLASS DerivedHideSelectionText AS TextBox\n"
            "    PROCEDURE Init\n"
            "        THIS.HideSelection = .F.\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native HideSelection script should complete: ") + state.message +
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
        check("ldefault", "true");
        check("ldirect", "false");
        check("lsetpem", "true");
        check("lsetpemvalue", "true");
        check("lputpem", "true");
        check("lputpemvalue", "false");
        check("laddproperty", "false");
        check("lremoveproperty", "false");
        check("lmember", "true");
        check("lderived", "false");
        expect(state.ole_objects.size() == 3U,
               "native HideSelection coverage should register the base, non-owning EditBox, and derived TextBox");

        fs::remove_all(temp_root, ignored);
    }
}
