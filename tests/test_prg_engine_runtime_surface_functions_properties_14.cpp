#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_textbox_specialeffect_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_textbox_specialeffect";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_textbox_specialeffect.prg";
        write_text(
            main_path,
            "oText = CREATEOBJECT('TextBox')\n"
            "oEdit = CREATEOBJECT('EditBox')\n"
            "lHas = PEMSTATUS(oText, 'SpecialEffect', 1)\n"
            "lReadOnly = PEMSTATUS(oText, 'SpecialEffect', 5)\n"
            "lEditHas = PEMSTATUS(oEdit, 'SpecialEffect', 1)\n"
            "nDefault = oText.SpecialEffect\n"
            "oText.SpecialEffect = 1.6\n"
            "nDirect = oText.SpecialEffect\n"
            "lSetPem = SETPEM(oText, 'SpecialEffect', 2)\n"
            "nSetPem = GETPEM(oText, 'SpecialEffect')\n"
            "lPutPem = PUTPEM(oText, 'SpecialEffect', 9)\n"
            "nPutPem = GETPEM(oText, 'SpecialEffect')\n"
            "oText.SpecialEffect = -3\n"
            "nNegative = oText.SpecialEffect\n"
            "lAddProperty = ADDPROPERTY(oText, 'SpecialEffect', 1)\n"
            "lRemoveProperty = REMOVEPROPERTY(oText, 'SpecialEffect')\n"
            "lMember = .F.\n"
            "nMembers = AMEMBERS(aMembers, oText, 1)\n"
            "FOR i = 1 TO nMembers\n"
            "    IF UPPER(aMembers[i]) == 'SPECIALEFFECT'\n"
            "        lMember = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedSpecialEffectText')\n"
            "nDerived = oDerived.SpecialEffect\n"
            "RETURN\n"
            "DEFINE CLASS DerivedSpecialEffectText AS TextBox\n"
            "    PROCEDURE Init\n"
            "        THIS.SpecialEffect = 0\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native SpecialEffect script should complete: ") + state.message +
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
        check("nsetpem", "2");
        check("lputpem", "true");
        check("nputpem", "2");
        check("nnegative", "0");
        check("laddproperty", "false");
        check("lremoveproperty", "false");
        check("lmember", "true");
        check("nderived", "0");
        expect(state.ole_objects.size() == 3U,
               "native SpecialEffect coverage should register the base, non-owning EditBox, and derived TextBox");

        fs::remove_all(temp_root, ignored);
    }
}
