#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_form_mdiform_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_form_mdiform";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_form_mdiform.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('Form')\n"
            "lHas = PEMSTATUS(oForm, 'MDIForm', 1)\n"
            "lReadOnly = PEMSTATUS(oForm, 'MDIForm', 5)\n"
            "lDefault = !oForm.MDIForm\n"
            "oForm.MDIForm = .T.\n"
            "lDirect = oForm.MDIForm\n"
            "lSetPem = SETPEM(oForm, 'MDIForm', .F.)\n"
            "lAfterSetPem = !GETPEM(oForm, 'MDIForm')\n"
            "lPutPem = PUTPEM(oForm, 'MDIForm', 1)\n"
            "lAfterPutPem = oForm.MDIForm\n"
            "lAdd = ADDPROPERTY(oForm, 'MDIForm', .F.)\n"
            "lRemove = REMOVEPROPERTY(oForm, 'MDIForm')\n"
            "lMember = .F.\n"
            "nMembers = AMEMBERS(aMembers, oForm, 1)\n"
            "FOR i = 1 TO nMembers\n"
            "    IF UPPER(aMembers[i]) == 'MDIFORM'\n"
            "        lMember = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedMdiForm')\n"
            "lDerived = oDerived.MDIForm\n"
            "RETURN\n"
            "DEFINE CLASS DerivedMdiForm AS Form\n"
            "    MDIForm = .T.\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Form MDIForm script should complete: ") + state.message +
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
        check("ldefault", "true");
        check("ldirect", "true");
        check("lsetpem", "true");
        check("laftersetpem", "true");
        check("lputpem", "true");
        check("lafterputpem", "true");
        check("ladd", "false");
        check("lremove", "false");
        check("lmember", "true");
        check("lderived", "true");

        fs::remove_all(temp_root, ignored);
    }
}
