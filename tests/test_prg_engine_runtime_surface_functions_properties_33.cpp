#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_textbox_text_defaults_read_only_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_textbox_text";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_textbox_text.prg";
        write_text(
            main_path,
            "oText = CREATEOBJECT('TextBox')\n"
            "oEdit = CREATEOBJECT('EditBox')\n"
            "lDefault = oText.Text\n"
            "lHas = PEMSTATUS(oText, 'Text', 1)\n"
            "lEditHas = PEMSTATUS(oEdit, 'Text', 1)\n"
            "lEditValueHas = PEMSTATUS(oEdit, 'Value', 1)\n"
            "lReadOnly = PEMSTATUS(oText, 'Text', 5)\n"
            "oText.Value = 'unformatted'\n"
            "lAfterValue = oText.Text\n"
            "lSet = SETPEM(oText, 'Text', 'changed')\n"
            "lPut = PUTPEM(oText, 'Text', 'changed')\n"
            "lAfterRejected = GETPEM(oText, 'Text')\n"
            "lAdd = ADDPROPERTY(oText, 'Text', 'shadow')\n"
            "lRemove = REMOVEPROPERTY(oText, 'Text')\n"
            "lEditValueAdd = ADDPROPERTY(oEdit, 'Value', 'shadow')\n"
            "lEditValueRemove = REMOVEPROPERTY(oEdit, 'Value')\n"
            "nPropMembers = AMEMBERS(aPropMembers, oText, 1)\n"
            "lPropHas = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'TEXT'\n"
            "        lPropHas = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedText')\n"
            "lDerived = oDerived.Text\n"
            "RETURN\n"
            "DEFINE CLASS DerivedText AS TextBox\n"
            "    PROCEDURE Init\n"
            "        THIS.Value = 'derived'\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native TextBox Text script should complete: ") + state.message +
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
        check("lhas", "true");
        check("ledithas", "true");
        check("leditvaluehas", "true");
        check("lreadonly", "true");
        check("laftervalue", "unformatted");
        check("lset", "false");
        check("lput", "false");
        check("lafterrejected", "unformatted");
        check("ladd", "false");
        check("lremove", "false");
        check("leditvalueadd", "false");
        check("leditvalueremove", "false");
        check("lprophas", "true");
        check("lderived", "derived");
        expect(state.ole_objects.size() == 3U,
               "native Text coverage should register TextBox, EditBox, and derived TextBox");

        fs::remove_all(temp_root, ignored);
    }
}
