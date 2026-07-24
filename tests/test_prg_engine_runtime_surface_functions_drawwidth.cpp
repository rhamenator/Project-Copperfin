#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_form_drawwidth_defaults_mutates_and_stays_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_form_drawwidth";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_form_drawwidth.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('Form')\n"
            "oShape = CREATEOBJECT('Shape')\n"
            "lFormHas = PEMSTATUS(oForm, 'DrawWidth', 1)\n"
            "lShapeHas = PEMSTATUS(oShape, 'DrawWidth', 1)\n"
            "nDefault = oForm.DrawWidth\n"
            "oForm.DrawWidth = 2.4\n"
            "nDirect = oForm.DrawWidth\n"
            "lSetPem = SETPEM(oForm, 'DrawWidth', 32767)\n"
            "nSetPem = GETPEM(oForm, 'DrawWidth')\n"
            "lPutPem = PUTPEM(oForm, 'DrawWidth', 40000)\n"
            "nPutPem = GETPEM(oForm, 'DrawWidth')\n"
            "oForm.DrawWidth = 0\n"
            "nNonPositive = oForm.DrawWidth\n"
            "lAddProperty = ADDPROPERTY(oForm, 'DrawWidth', 1)\n"
            "lRemoveProperty = REMOVEPROPERTY(oForm, 'DrawWidth')\n"
            "lMember = .F.\n"
            "nMembers = AMEMBERS(aMembers, oForm, 1)\n"
            "FOR i = 1 TO nMembers\n"
            "    IF UPPER(aMembers[i]) == 'DRAWWIDTH'\n"
            "        lMember = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedDrawForm')\n"
            "nDerived = oDerived.DrawWidth\n"
            "RETURN\n"
            "DEFINE CLASS DerivedDrawForm AS Form\n"
            "    DrawWidth = 7\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native DrawWidth script should complete: ") + state.message +
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

        check("lformhas", "true");
        check("lshapehas", "false");
        check("ndefault", "1");
        check("ndirect", "2");
        check("lsetpem", "true");
        check("nsetpem", "32767");
        check("lputpem", "true");
        check("nputpem", "1");
        check("nnonpositive", "1");
        check("laddproperty", "false");
        check("lremoveproperty", "false");
        check("lmember", "true");
        check("nderived", "7");

        fs::remove_all(temp_root, ignored);
    }
}
