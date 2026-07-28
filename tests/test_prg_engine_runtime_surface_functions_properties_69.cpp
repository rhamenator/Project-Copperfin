#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_form_forecolor_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_form_forecolor";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_form_forecolor.prg";
        write_text(
            main_path,
            "oBaseForm = CREATEOBJECT('Form')\n"
            "oControl = CREATEOBJECT('CommandButton')\n"
            "lBaseHasForeColor = PEMSTATUS(oBaseForm, 'ForeColor', 1)\n"
            "lControlHasForeColor = PEMSTATUS(oControl, 'ForeColor', 1)\n"
            "lBaseBefore = oBaseForm.ForeColor\n"
            "xBaseGetPemBefore = GETPEM(oBaseForm, 'ForeColor')\n"
            "oBaseForm.ForeColor = 255\n"
            "lBaseAfterDirectAssign = oBaseForm.ForeColor\n"
            "lBaseSetPem = SETPEM(oBaseForm, 'ForeColor', 65536)\n"
            "lBaseAfterSetPem = oBaseForm.ForeColor\n"
            "lBasePutPem = PUTPEM(oBaseForm, 'ForeColor', 16711680)\n"
            "lBaseAfterPutPem = oBaseForm.ForeColor\n"
            "lBaseAddProperty = ADDPROPERTY(oBaseForm, 'ForeColor', 0)\n"
            "lBaseRemoveProperty = REMOVEPROPERTY(oBaseForm, 'ForeColor')\n"
            "oDerived = CREATEOBJECT('DemoForm')\n"
            "lDerivedBefore = oDerived.ForeColor\n"
            "nPropMembers = AMEMBERS(aPropMembers, oDerived, 1)\n"
            "lPropHasForeColor = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'FORECOLOR'\n"
            "        lPropHasForeColor = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Form\n"
            "    ForeColor = 128\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Form ForeColor property script should complete: ") + state.message +
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

        check("lbasehasforecolor", "true");
        check("lcontrolhasforecolor", "false");
        check("lbasebefore", "0");
        check("xbasegetpembefore", "0");
        check("lbaseafterdirectassign", "255");
        check("lbasesetpem", "true");
        check("lbaseaftersetpem", "65536");
        check("lbaseputpem", "true");
        check("lbaseafterputpem", "16711680");
        check("lbaseaddproperty", "false");
        check("lbaseremoveproperty", "false");
        check("lderivedbefore", "128");
        check("lprophasforecolor", "true");

        fs::remove_all(temp_root, ignored);
    }
}
