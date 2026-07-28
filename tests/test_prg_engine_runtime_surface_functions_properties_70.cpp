#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_form_size_limits_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_form_size_limits";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_form_size_limits.prg";
        write_text(
            main_path,
            "oBaseForm = CREATEOBJECT('Form')\n"
            "oControl = CREATEOBJECT('CommandButton')\n"
            "lBaseHasMinWidth = PEMSTATUS(oBaseForm, 'MinWidth', 1)\n"
            "lBaseHasMinHeight = PEMSTATUS(oBaseForm, 'MinHeight', 1)\n"
            "lBaseHasMaxWidth = PEMSTATUS(oBaseForm, 'MaxWidth', 1)\n"
            "lBaseHasMaxHeight = PEMSTATUS(oBaseForm, 'MaxHeight', 1)\n"
            "lControlHasMinWidth = PEMSTATUS(oControl, 'MinWidth', 1)\n"
            "lBaseMinWidthBefore = oBaseForm.MinWidth\n"
            "xBaseGetPemBefore = GETPEM(oBaseForm, 'MinHeight')\n"
            "oBaseForm.MinWidth = 120.6\n"
            "lBaseMinWidthAfterDirectAssign = oBaseForm.MinWidth\n"
            "lBaseSetPem = SETPEM(oBaseForm, 'MinHeight', 240.4)\n"
            "lBaseMinHeightAfterSetPem = oBaseForm.MinHeight\n"
            "lBasePutPem = PUTPEM(oBaseForm, 'MaxWidth', 640.8)\n"
            "lBaseMaxWidthAfterPutPem = oBaseForm.MaxWidth\n"
            "oBaseForm.MaxHeight = 99999\n"
            "lBaseMaxHeightClamped = oBaseForm.MaxHeight\n"
            "oBaseForm.MinHeight = -42\n"
            "lBaseMinHeightUnlimited = oBaseForm.MinHeight\n"
            "lBaseAddProperty = ADDPROPERTY(oBaseForm, 'MinWidth', .F.)\n"
            "lBaseRemoveProperty = REMOVEPROPERTY(oBaseForm, 'MaxHeight')\n"
            "lScreenHasMinWidth = PEMSTATUS(_SCREEN, 'MinWidth', 1)\n"
            "lScreenHasMinHeight = PEMSTATUS(_VFP, 'MinHeight', 1)\n"
            "lScreenMinWidthBefore = _SCREEN.MinWidth\n"
            "_SCREEN.MinWidth = 800.4\n"
            "lVfpMinWidthAfterDirectAssign = _VFP.MinWidth\n"
            "lScreenSetPem = SETPEM(_SCREEN, 'MaxHeight', 900.5)\n"
            "lVfpMaxHeightAfterSetPem = _VFP.MaxHeight\n"
            "lScreenPutPem = PUTPEM(_VFP, 'MinHeight', 480.2)\n"
            "lScreenMinHeightAfterPutPem = _SCREEN.MinHeight\n"
            "lScreenAddProperty = ADDPROPERTY(_SCREEN, 'MaxWidth', .F.)\n"
            "lScreenRemoveProperty = REMOVEPROPERTY(_VFP, 'MinWidth')\n"
            "nScreenPropMembers = AMEMBERS(aScreenPropMembers, _SCREEN, 1)\n"
            "lScreenPropHasMinWidth = .F.\n"
            "FOR i = 1 TO nScreenPropMembers\n"
            "    IF UPPER(aScreenPropMembers[i]) == 'MINWIDTH'\n"
            "        lScreenPropHasMinWidth = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DemoForm')\n"
            "lDerivedMinWidth = oDerived.MinWidth\n"
            "lDerivedMaxHeight = oDerived.MaxHeight\n"
            "nPropMembers = AMEMBERS(aPropMembers, oDerived, 1)\n"
            "lPropHasMaxWidth = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'MAXWIDTH'\n"
            "        lPropHasMaxWidth = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Form\n"
            "    MinWidth = 16\n"
            "    MaxHeight = 1024\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Form size-limit property script should complete: ") + state.message +
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

        check("lbasehasminwidth", "true");
        check("lbasehasminheight", "true");
        check("lbasehasmaxwidth", "true");
        check("lbasehasmaxheight", "true");
        check("lcontrolhasminwidth", "false");
        check("lbaseminwidthbefore", "-1");
        check("xbasegetpembefore", "-1");
        check("lbaseminwidthafterdirectassign", "121");
        check("lbasesetpem", "true");
        check("lbaseminheightaftersetpem", "240");
        check("lbaseputpem", "true");
        check("lbasemaxwidthafterputpem", "641");
        check("lbasemaxheightclamped", "32767");
        check("lbaseminheightunlimited", "-1");
        check("lbaseaddproperty", "false");
        check("lbaseremoveproperty", "false");
        check("lscreenhasminwidth", "true");
        check("lscreenhasminheight", "true");
        check("lscreenminwidthbefore", "-1");
        check("lvfpminwidthafterdirectassign", "800");
        check("lscreensetpem", "true");
        check("lvfpmaxheightaftersetpem", "901");
        check("lscreenputpem", "true");
        check("lscreenminheightafterputpem", "480");
        check("lscreenaddproperty", "false");
        check("lscreenremoveproperty", "false");
        check("lscreenprophasminwidth", "true");
        check("lderivedminwidth", "16");
        check("lderivedmaxheight", "1024");
        check("lprophasmaxwidth", "true");

        fs::remove_all(temp_root, ignored);
    }
}
