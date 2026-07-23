#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_visual_controltiptext_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_controltiptext";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_controltiptext.prg";
        write_text(
            main_path,
            "oPlain = CREATEOBJECT('ListBox')\n"
            "lHas = PEMSTATUS(oPlain, 'ControlTipText', 1)\n"
            "lReadOnly = PEMSTATUS(oPlain, 'ControlTipText', 5)\n"
            "cBefore = oPlain.ControlTipText\n"
            "xBefore = GETPEM(oPlain, 'ControlTipText')\n"
            "oPlain.ControlTipText = 'plain tip'\n"
            "cAfterDirect = oPlain.ControlTipText\n"
            "lSetPem = SETPEM(oPlain, 'ControlTipText', 'setpem tip')\n"
            "cAfterSetPem = GETPEM(oPlain, 'ControlTipText')\n"
            "lAddProperty = ADDPROPERTY(oPlain, 'ControlTipText', 'shadow')\n"
            "lRemoveProperty = REMOVEPROPERTY(oPlain, 'ControlTipText')\n"
            "nMembers = AMEMBERS(aMembers, oPlain, 1)\n"
            "lMember = .F.\n"
            "FOR i = 1 TO nMembers\n"
            "    IF UPPER(aMembers[i]) == 'CONTROLTIPTEXT'\n"
            "        lMember = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oForm = CREATEOBJECT('TipForm')\n"
            "cChildBefore = oForm.lstItems.ControlTipText\n"
            "cChildRead = oForm.cmdProbe.ReadTip()\n"
            "oForm.cmdProbe.SetTip()\n"
            "cChildAfter = oForm.lstItems.ControlTipText\n"
            "oDerived = CREATEOBJECT('DerivedTipCombo')\n"
            "cDerived = oDerived.ControlTipText\n"
            "RETURN\n"
            "DEFINE CLASS TipProbe AS CommandButton\n"
            "    FUNCTION ReadTip\n"
            "        RETURN THISFORM.lstItems.ControlTipText\n"
            "    ENDFUNC\n"
            "    PROCEDURE SetTip\n"
            "        THISFORM.lstItems.ControlTipText = 'child tip'\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS TipForm AS Form\n"
            "    ADD OBJECT lstItems AS ListBox WITH ControlTipText = 'initial tip'\n"
            "    ADD OBJECT cmdProbe AS TipProbe\n"
            "ENDDEFINE\n"
            "DEFINE CLASS DerivedTipCombo AS ComboBox\n"
            "    ControlTipText = 'derived tip'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native ControlTipText script should complete: ") + state.message +
                   " @line=" + std::to_string(state.location.line));

        const auto check = [&](const std::string &name, const std::string &expected)
        {
            const auto it = state.globals.find(name);
            if (it == state.globals.end())
            {
                expect(false, name + " variable not found");
                return;
            }
            expect(copperfin::runtime::format_value(it->second) == expected,
                   name + " expected '" + expected + "' got '" + copperfin::runtime::format_value(it->second) + "'");
        };

        check("lhas", "true");
        check("lreadonly", "false");
        check("cbefore", "");
        check("xbefore", "");
        check("cafterdirect", "plain tip");
        check("lsetpem", "true");
        check("caftersetpem", "setpem tip");
        check("laddproperty", "false");
        check("lremoveproperty", "false");
        check("lmember", "true");
        check("cchildbefore", "initial tip");
        check("cchildread", "initial tip");
        check("cchildafter", "child tip");
        check("cderived", "derived tip");

        fs::remove_all(temp_root, ignored);
    }
}
