#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_showtips_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_showtips";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_showtips.prg";
        write_text(
            main_path,
            "oBaseForm = CREATEOBJECT('Form')\n"
            "oControl = CREATEOBJECT('CommandButton')\n"
            "lScreenHasShowTips = PEMSTATUS(_SCREEN, 'ShowTips', 1)\n"
            "lVfpHasShowTips = PEMSTATUS(_VFP, 'ShowTips', 1)\n"
            "lScreenBefore = _SCREEN.ShowTips\n"
            "xScreenGetPemBefore = GETPEM(_SCREEN, 'ShowTips')\n"
            "_SCREEN.ShowTips = .T.\n"
            "lVfpAfterDirectAssign = _VFP.ShowTips\n"
            "lScreenSetPem = SETPEM(_SCREEN, 'ShowTips', .F.)\n"
            "lVfpAfterSetPem = _VFP.ShowTips\n"
            "lScreenPutPem = PUTPEM(_SCREEN, 'ShowTips', .T.)\n"
            "lScreenAfterPutPem = _SCREEN.ShowTips\n"
            "lScreenAddProperty = ADDPROPERTY(_SCREEN, 'ShowTips', .F.)\n"
            "lScreenRemoveProperty = REMOVEPROPERTY(_VFP, 'ShowTips')\n"
            "nScreenPropMembers = AMEMBERS(aScreenPropMembers, _SCREEN, 1)\n"
            "lScreenPropHasShowTips = .F.\n"
            "FOR i = 1 TO nScreenPropMembers\n"
            "    IF UPPER(aScreenPropMembers[i]) == 'SHOWTIPS'\n"
            "        lScreenPropHasShowTips = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oBaseToolbar = CREATEOBJECT('ToolBar')\n"
            "lToolbarHasShowTips = PEMSTATUS(oBaseToolbar, 'ShowTips', 1)\n"
            "lToolbarBefore = oBaseToolbar.ShowTips\n"
            "oBaseToolbar.ShowTips = .T.\n"
            "lToolbarAfterDirectAssign = oBaseToolbar.ShowTips\n"
            "lToolbarSetPem = SETPEM(oBaseToolbar, 'ShowTips', .F.)\n"
            "lToolbarAfterSetPem = oBaseToolbar.ShowTips\n"
            "lToolbarPutPem = PUTPEM(oBaseToolbar, 'ShowTips', .T.)\n"
            "lToolbarAfterPutPem = oBaseToolbar.ShowTips\n"
            "lToolbarAddProperty = ADDPROPERTY(oBaseToolbar, 'ShowTips', .F.)\n"
            "lToolbarRemoveProperty = REMOVEPROPERTY(oBaseToolbar, 'ShowTips')\n"
            "oDerivedToolbar = CREATEOBJECT('DemoToolBar')\n"
            "lDerivedToolbarBefore = oDerivedToolbar.ShowTips\n"
            "lBaseHasShowTips = PEMSTATUS(oBaseForm, 'ShowTips', 1)\n"
            "lControlHasShowTips = PEMSTATUS(oControl, 'ShowTips', 1)\n"
            "lBaseBefore = oBaseForm.ShowTips\n"
            "xBaseGetPemBefore = GETPEM(oBaseForm, 'ShowTips')\n"
            "oBaseForm.ShowTips = .T.\n"
            "lBaseAfterDirectAssign = oBaseForm.ShowTips\n"
            "lBaseSetPem = SETPEM(oBaseForm, 'ShowTips', .F.)\n"
            "lBaseAfterSetPem = oBaseForm.ShowTips\n"
            "lBasePutPem = PUTPEM(oBaseForm, 'ShowTips', .T.)\n"
            "lBaseAfterPutPem = oBaseForm.ShowTips\n"
            "lBaseAddProperty = ADDPROPERTY(oBaseForm, 'ShowTips', .F.)\n"
            "lBaseRemoveProperty = REMOVEPROPERTY(oBaseForm, 'ShowTips')\n"
            "oDerived = CREATEOBJECT('DemoForm')\n"
            "lDerivedBefore = oDerived.ShowTips\n"
            "nPropMembers = AMEMBERS(aPropMembers, oDerived, 1)\n"
            "lPropHasShowTips = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'SHOWTIPS'\n"
            "        lPropHasShowTips = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "nToolbarPropMembers = AMEMBERS(aToolbarPropMembers, oDerivedToolbar, 1)\n"
            "lToolbarPropHasShowTips = .F.\n"
            "FOR i = 1 TO nToolbarPropMembers\n"
            "    IF UPPER(aToolbarPropMembers[i]) == 'SHOWTIPS'\n"
            "        lToolbarPropHasShowTips = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Form\n"
            "    ShowTips = .T.\n"
            "ENDDEFINE\n"
            "DEFINE CLASS DemoToolBar AS ToolBar\n"
            "    ShowTips = .T.\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native ShowTips property script should complete: ") + state.message +
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

        check("lscreenhasshowtips", "true");
        check("lvfphasshowtips", "true");
        check("lscreenbefore", "false");
        check("xscreengetpembefore", "false");
        check("lvfpafterdirectassign", "true");
        check("lscreensetpem", "true");
        check("lvfpaftersetpem", "false");
        check("lscreenputpem", "true");
        check("lscreenafterputpem", "true");
        check("lscreenaddproperty", "false");
        check("lscreenremoveproperty", "false");
        check("lscreenprophasshowtips", "true");
        check("ltoolbarhasshowtips", "true");
        check("ltoolbarbefore", "false");
        check("ltoolbarafterdirectassign", "true");
        check("ltoolbarsetpem", "true");
        check("ltoolbaraftersetpem", "false");
        check("ltoolbarputpem", "true");
        check("ltoolbarafterputpem", "true");
        check("ltoolbaraddproperty", "false");
        check("ltoolbarremoveproperty", "false");
        check("lderivedtoolbarbefore", "true");
        check("ltoolbarprophasshowtips", "true");
        check("lbasehasshowtips", "true");
        check("lcontrolhasshowtips", "false");
        check("lbasebefore", "false");
        check("xbasegetpembefore", "false");
        check("lbaseafterdirectassign", "true");
        check("lbasesetpem", "true");
        check("lbaseaftersetpem", "false");
        check("lbaseputpem", "true");
        check("lbaseafterputpem", "true");
        check("lbaseaddproperty", "false");
        check("lbaseremoveproperty", "false");
        check("lderivedbefore", "true");
        check("lprophasshowtips", "true");

        fs::remove_all(temp_root, ignored);
    }
}
