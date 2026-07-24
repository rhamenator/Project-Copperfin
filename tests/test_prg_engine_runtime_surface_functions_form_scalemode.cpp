#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_form_scalemode_defaults_mutates_and_stays_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_form_scalemode";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_form_scalemode.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('Form')\n"
            "oLabel = CREATEOBJECT('Label')\n"
            "lHas = PEMSTATUS(oForm, 'ScaleMode', 1)\n"
            "lReadOnly = PEMSTATUS(oForm, 'ScaleMode', 5)\n"
            "lLabelHas = PEMSTATUS(oLabel, 'ScaleMode', 1)\n"
            "nDefault = oForm.ScaleMode\n"
            "oForm.ScaleMode = 1.6\n"
            "nDirect = oForm.ScaleMode\n"
            "lSetPem = SETPEM(oForm, 'ScaleMode', 2)\n"
            "nSetPem = GETPEM(oForm, 'ScaleMode')\n"
            "lPutPem = PUTPEM(oForm, 'ScaleMode', 9)\n"
            "nPutPem = GETPEM(oForm, 'ScaleMode')\n"
            "oForm.ScaleMode = -3\n"
            "nNegative = oForm.ScaleMode\n"
            "lAddProperty = ADDPROPERTY(oForm, 'ScaleMode', 1)\n"
            "lRemoveProperty = REMOVEPROPERTY(oForm, 'ScaleMode')\n"
            "lMember = .F.\n"
            "nMembers = AMEMBERS(aMembers, oForm, 1)\n"
            "FOR i = 1 TO nMembers\n"
            "    IF UPPER(aMembers[i]) == 'SCALEMODE'\n"
            "        lMember = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedScaleForm')\n"
            "nDerived = oDerived.ScaleMode\n"
            "RETURN\n"
            "DEFINE CLASS DerivedScaleForm AS Form\n"
            "    ScaleMode = 3\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Form ScaleMode script should complete: ") + state.message +
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
        check("llabelhas", "false");
        check("ndefault", "0");
        check("ndirect", "2");
        check("lsetpem", "true");
        check("nsetpem", "2");
        check("lputpem", "true");
        check("nputpem", "3");
        check("nnegative", "0");
        check("laddproperty", "false");
        check("lremoveproperty", "false");
        check("lmember", "true");
        check("nderived", "3");
        expect(state.ole_objects.size() == 3U,
               "native Form ScaleMode coverage should register the base, non-owning Label, and derived Form");

        fs::remove_all(temp_root, ignored);
    }
}
