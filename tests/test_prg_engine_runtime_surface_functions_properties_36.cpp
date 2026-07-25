#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_form_keypreview_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_form_keypreview";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_form_keypreview.prg";
        write_text(
            main_path,
            "oBaseForm = CREATEOBJECT('Form')\n"
            "lBaseHasKeyPreview = PEMSTATUS(oBaseForm, 'KeyPreview', 1)\n"
            "lBaseKeyPreviewReadOnly = PEMSTATUS(oBaseForm, 'KeyPreview', 5)\n"
            "lBaseBefore = oBaseForm.KeyPreview\n"
            "xBaseGetPemBefore = GETPEM(oBaseForm, 'KeyPreview')\n"
            "oBaseForm.KeyPreview = .T.\n"
            "lBaseAfterDirectAssign = oBaseForm.KeyPreview\n"
            "lBaseSetPem = SETPEM(oBaseForm, 'KeyPreview', .F.)\n"
            "lBaseAfterSetPem = oBaseForm.KeyPreview\n"
            "lBasePutPem = PUTPEM(oBaseForm, 'KeyPreview', 1)\n"
            "lBaseAfterPutPem = oBaseForm.KeyPreview\n"
            "lBaseAddProperty = ADDPROPERTY(oBaseForm, 'KeyPreview', .F.)\n"
            "lBaseRemoveProperty = REMOVEPROPERTY(oBaseForm, 'KeyPreview')\n"
            "oDerived = CREATEOBJECT('DemoForm')\n"
            "lDerivedBefore = oDerived.KeyPreview\n"
            "cChildBefore = oDerived.cmdSave.ReadKeyPreview()\n"
            "oDerived.cmdSave.DisableKeyPreview()\n"
            "lDerivedAfterChild = oDerived.KeyPreview\n"
            "xDerivedGetPem = GETPEM(oDerived, 'KeyPreview')\n"
            "nPropMembers = AMEMBERS(aPropMembers, oDerived, 1)\n"
            "lPropHasKeyPreview = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'KEYPREVIEW'\n"
            "        lPropHasKeyPreview = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "RETURN\n"
            "DEFINE CLASS SaveButton AS CommandButton\n"
            "    FUNCTION ReadKeyPreview\n"
            "        RETURN IIF(THISFORM.KeyPreview, 'T', 'F')\n"
            "    ENDFUNC\n"
            "    PROCEDURE DisableKeyPreview\n"
            "        THISFORM.KeyPreview = .F.\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS DemoForm AS Form\n"
            "    KeyPreview = .T.\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Form KeyPreview property script should complete: ") + state.message +
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

        check("lbasehaskeypreview", "true");
        check("lbasekeypreviewreadonly", "false");
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
        check("cchildbefore", "T");
        check("lderivedafterchild", "false");
        check("xderivedgetpem", "false");
        check("lprophaskeypreview", "true");

        fs::remove_all(temp_root, ignored);
    }
}
