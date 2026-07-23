#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_textbox_century_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_textbox_century";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_textbox_century.prg";
        write_text(
            main_path,
            "oText = CREATEOBJECT('TextBox')\n"
            "oEdit = CREATEOBJECT('EditBox')\n"
            "lDefault = oText.Century\n"
            "lEditHas = PEMSTATUS(oEdit, 'Century', 1)\n"
            "lHas = PEMSTATUS(oText, 'Century', 1)\n"
            "lReadOnly = PEMSTATUS(oText, 'Century', 5)\n"
            "oText.Century = 0.4\n"
            "lDirect = oText.Century\n"
            "lSetPem = SETPEM(oText, 'Century', 2.6)\n"
            "lAfterSetPem = GETPEM(oText, 'Century')\n"
            "lPutPem = PUTPEM(oText, 'Century', 9)\n"
            "lAfterPutPem = GETPEM(oText, 'Century')\n"
            "lAddProperty = ADDPROPERTY(oText, 'Century', 5)\n"
            "lRemoveProperty = REMOVEPROPERTY(oText, 'Century')\n"
            "nPropMembers = AMEMBERS(aPropMembers, oText, 1)\n"
            "lPropHas = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'CENTURY'\n"
            "        lPropHas = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedCentury')\n"
            "lDerived = oDerived.Century\n"
            "RETURN\n"
            "DEFINE CLASS DerivedCentury AS TextBox\n"
            "    PROCEDURE Init\n"
            "        THIS.Century = 1.6\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native TextBox Century script should complete: ") + state.message +
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

        check("ldefault", "1");
        check("ledithas", "false");
        check("lhas", "true");
        check("lreadonly", "false");
        check("ldirect", "0");
        check("lsetpem", "true");
        check("laftersetpem", "1");
        check("lputpem", "true");
        check("lafterputpem", "1");
        check("laddproperty", "false");
        check("lremoveproperty", "false");
        check("lprophas", "true");
        check("lderived", "2");
        expect(state.ole_objects.size() == 3U,
               "native Century coverage should register TextBox, EditBox, and derived TextBox");

        fs::remove_all(temp_root, ignored);
    }
}
