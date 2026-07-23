#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_textbox_datemark_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_textbox_datemark";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_textbox_datemark.prg";
        write_text(
            main_path,
            "oText = CREATEOBJECT('TextBox')\n"
            "oEdit = CREATEOBJECT('EditBox')\n"
            "lDefault = oText.DateMark\n"
            "lEditHas = PEMSTATUS(oEdit, 'DateMark', 1)\n"
            "lHas = PEMSTATUS(oText, 'DateMark', 1)\n"
            "lReadOnly = PEMSTATUS(oText, 'DateMark', 5)\n"
            "oText.DateMark = ' ' \n"
            "lSpace = oText.DateMark\n"
            "lSetPem = SETPEM(oText, 'DateMark', '-')\n"
            "lAfterSetPem = GETPEM(oText, 'DateMark')\n"
            "lPutPem = PUTPEM(oText, 'DateMark', 123)\n"
            "lAfterPutPem = GETPEM(oText, 'DateMark')\n"
            "lAddProperty = ADDPROPERTY(oText, 'DateMark', '/')\n"
            "lRemoveProperty = REMOVEPROPERTY(oText, 'DateMark')\n"
            "nPropMembers = AMEMBERS(aPropMembers, oText, 1)\n"
            "lPropHas = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'DATEMARK'\n"
            "        lPropHas = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedDateMark')\n"
            "lDerived = oDerived.DateMark\n"
            "RETURN\n"
            "DEFINE CLASS DerivedDateMark AS TextBox\n"
            "    PROCEDURE Init\n"
            "        THIS.DateMark = '.'\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native TextBox DateMark script should complete: ") + state.message +
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

        check("ldefault", "");
        check("ledithas", "false");
        check("lhas", "true");
        check("lreadonly", "false");
        check("lspace", " ");
        check("lsetpem", "true");
        check("laftersetpem", "-");
        check("lputpem", "true");
        check("lafterputpem", "123");
        check("laddproperty", "false");
        check("lremoveproperty", "false");
        check("lprophas", "true");
        check("lderived", ".");
        expect(state.ole_objects.size() == 3U,
               "native DateMark coverage should register TextBox, EditBox, and derived TextBox");

        fs::remove_all(temp_root, ignored);
    }
}
