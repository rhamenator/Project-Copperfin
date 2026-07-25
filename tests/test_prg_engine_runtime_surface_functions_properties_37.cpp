#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_visual_fontcharset_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_visual_fontcharset";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_visual_fontcharset.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('Form')\n"
            "nFormDefault = oForm.FontCharSet\n"
            "lFormHas = PEMSTATUS(oForm, 'FontCharSet', 1)\n"
            "lFormReadOnly = PEMSTATUS(oForm, 'FontCharSet', 5)\n"
            "oForm.FontCharSet = 17.6\n"
            "nDirect = oForm.FontCharSet\n"
            "lSetPem = SETPEM(oForm, 'FontCharSet', -5)\n"
            "nSetPem = GETPEM(oForm, 'FontCharSet')\n"
            "lPutPem = PUTPEM(oForm, 'FontCharSet', '29.4')\n"
            "nPutPem = GETPEM(oForm, 'FontCharSet')\n"
            "lAddProperty = ADDPROPERTY(oForm, 'FontCharSet', 3)\n"
            "lRemoveProperty = REMOVEPROPERTY(oForm, 'FontCharSet')\n"
            "oTextBox = CREATEOBJECT('TextBox')\n"
            "nTextBoxDefault = oTextBox.FontCharSet\n"
            "oDerived = CREATEOBJECT('DerivedFontCharSet')\n"
            "nDerived = oDerived.FontCharSet\n"
            "nMembers = AMEMBERS(aMembers, oDerived, 1)\n"
            "lPropHas = .F.\n"
            "FOR i = 1 TO nMembers\n"
            "    IF UPPER(aMembers[i]) == 'FONTCHARSET'\n"
            "        lPropHas = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "RETURN\n"
            "DEFINE CLASS DerivedFontCharSet AS Label\n"
            "    PROCEDURE Init\n"
            "        THIS.FontCharSet = 204\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native FontCharSet script should complete: ") + state.message +
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

        check("nformdefault", "1");
        check("lformhas", "true");
        check("lformreadonly", "false");
        check("ndirect", "18");
        check("lsetpem", "true");
        check("nsetpem", "0");
        check("lputpem", "true");
        check("nputpem", "29");
        check("laddproperty", "false");
        check("lremoveproperty", "false");
        check("ntextboxdefault", "1");
        check("nderived", "204");
        check("lprophas", "true");
        expect(state.ole_objects.size() == 3U,
               "native FontCharSet coverage should register Form, TextBox, and derived Label");

        fs::remove_all(temp_root, ignored);
    }
}
