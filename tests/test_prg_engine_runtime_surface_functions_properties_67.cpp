#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_form_zoombox_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_form_zoombox";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_form_zoombox.prg";
        write_text(
            main_path,
            "oBaseForm = CREATEOBJECT('Form')\n"
            "oControl = CREATEOBJECT('CommandButton')\n"
            "lBaseHasZoomBox = PEMSTATUS(oBaseForm, 'ZoomBox', 1)\n"
            "lControlHasZoomBox = PEMSTATUS(oControl, 'ZoomBox', 1)\n"
            "lBaseBefore = oBaseForm.ZoomBox\n"
            "xBaseGetPemBefore = GETPEM(oBaseForm, 'ZoomBox')\n"
            "oBaseForm.ZoomBox = .T.\n"
            "lBaseAfterDirectAssign = oBaseForm.ZoomBox\n"
            "lBaseSetPem = SETPEM(oBaseForm, 'ZoomBox', .F.)\n"
            "lBaseAfterSetPem = oBaseForm.ZoomBox\n"
            "lBasePutPem = PUTPEM(oBaseForm, 'ZoomBox', .T.)\n"
            "lBaseAfterPutPem = oBaseForm.ZoomBox\n"
            "lBaseAddProperty = ADDPROPERTY(oBaseForm, 'ZoomBox', .F.)\n"
            "lBaseRemoveProperty = REMOVEPROPERTY(oBaseForm, 'ZoomBox')\n"
            "oDerived = CREATEOBJECT('DemoForm')\n"
            "lDerivedBefore = oDerived.ZoomBox\n"
            "nPropMembers = AMEMBERS(aPropMembers, oDerived, 1)\n"
            "lPropHasZoomBox = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'ZOOMBOX'\n"
            "        lPropHasZoomBox = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Form\n"
            "    ZoomBox = .T.\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Form ZoomBox property script should complete: ") + state.message +
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

        check("lbasehaszoombox", "true");
        check("lcontrolhaszoombox", "false");
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
        check("lprophaszoombox", "true");

        fs::remove_all(temp_root, ignored);
    }
}
