#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_form_drawstyle_defaults_mutates_and_stays_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_form_drawstyle";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_form_drawstyle.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('Form')\n"
            "oLabel = CREATEOBJECT('Label')\n"
            "oShape = CREATEOBJECT('Shape')\n"
            "lFormHas = PEMSTATUS(oForm, 'DrawStyle', 1)\n"
            "lLabelHas = PEMSTATUS(oLabel, 'DrawStyle', 1)\n"
            "lShapeHas = PEMSTATUS(oShape, 'DrawStyle', 1)\n"
            "nDefault = oForm.DrawStyle\n"
            "oForm.DrawStyle = 1.6\n"
            "nDirect = oForm.DrawStyle\n"
            "lSetPem = SETPEM(oForm, 'DrawStyle', 6)\n"
            "nSetPem = GETPEM(oForm, 'DrawStyle')\n"
            "lPutPem = PUTPEM(oForm, 'DrawStyle', 99)\n"
            "nPutPem = GETPEM(oForm, 'DrawStyle')\n"
            "oForm.DrawStyle = -2\n"
            "nNegative = oForm.DrawStyle\n"
            "lAddProperty = ADDPROPERTY(oForm, 'DrawStyle', 1)\n"
            "lRemoveProperty = REMOVEPROPERTY(oForm, 'DrawStyle')\n"
            "lMember = .F.\n"
            "nMembers = AMEMBERS(aMembers, oForm, 1)\n"
            "FOR i = 1 TO nMembers\n"
            "    IF UPPER(aMembers[i]) == 'DRAWSTYLE'\n"
            "        lMember = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedDrawStyleForm')\n"
            "nDerived = oDerived.DrawStyle\n"
            "RETURN\n"
            "DEFINE CLASS DerivedDrawStyleForm AS Form\n"
            "    DrawStyle = 5\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native DrawStyle script should complete: ") + state.message +
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
        check("llabelhas", "false");
        check("lshapehas", "false");
        check("ndefault", "0");
        check("ndirect", "2");
        check("lsetpem", "true");
        check("nsetpem", "6");
        check("lputpem", "true");
        check("nputpem", "0");
        check("nnegative", "0");
        check("laddproperty", "false");
        check("lremoveproperty", "false");
        check("lmember", "true");
        check("nderived", "5");

        fs::remove_all(temp_root, ignored);
    }
}
