#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_visual_wordwrap_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_visual_wordwrap";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_visual_wordwrap.prg";
        write_text(
            main_path,
            "oCheckBox = CREATEOBJECT('CheckBox')\n"
            "oCommandButton = CREATEOBJECT('CommandButton')\n"
            "oLabel = CREATEOBJECT('Label')\n"
            "oOptionButton = CREATEOBJECT('OptionButton')\n"
            "lCheckBox = PEMSTATUS(oCheckBox, 'WordWrap', 1)\n"
            "lCommandButton = PEMSTATUS(oCommandButton, 'WordWrap', 1)\n"
            "lLabel = PEMSTATUS(oLabel, 'WordWrap', 1)\n"
            "lOptionButton = PEMSTATUS(oOptionButton, 'WordWrap', 1)\n"
            "lDefault = oCheckBox.WordWrap\n"
            "lGetPem = GETPEM(oLabel, 'WordWrap')\n"
            "lReadOnly = PEMSTATUS(oCommandButton, 'WordWrap', 5)\n"
            "oCheckBox.WordWrap = .T.\n"
            "lDirect = oCheckBox.WordWrap\n"
            "lSetPem = SETPEM(oCommandButton, 'WordWrap', .T.)\n"
            "lSetPemValue = oCommandButton.WordWrap\n"
            "lPutPem = PUTPEM(oLabel, 'WordWrap', .T.)\n"
            "lPutPemValue = oLabel.WordWrap\n"
            "lAddProperty = ADDPROPERTY(oOptionButton, 'WordWrap', .T.)\n"
            "lRemoveProperty = REMOVEPROPERTY(oOptionButton, 'WordWrap')\n"
            "oUnsupportedForm = CREATEOBJECT('Form')\n"
            "oUnsupportedComboBox = CREATEOBJECT('ComboBox')\n"
            "oUnsupportedTextBox = CREATEOBJECT('TextBox')\n"
            "oUnsupportedContainer = CREATEOBJECT('Container')\n"
            "oUnsupportedOptionGroup = CREATEOBJECT('OptionGroup')\n"
            "oUnsupportedImage = CREATEOBJECT('Image')\n"
            "oUnsupportedCustom = CREATEOBJECT('Custom')\n"
            "lForm = PEMSTATUS(oUnsupportedForm, 'WordWrap', 1)\n"
            "lComboBox = PEMSTATUS(oUnsupportedComboBox, 'WordWrap', 1)\n"
            "lTextBox = PEMSTATUS(oUnsupportedTextBox, 'WordWrap', 1)\n"
            "lContainer = PEMSTATUS(oUnsupportedContainer, 'WordWrap', 1)\n"
            "lOptionGroup = PEMSTATUS(oUnsupportedOptionGroup, 'WordWrap', 1)\n"
            "lImage = PEMSTATUS(oUnsupportedImage, 'WordWrap', 1)\n"
            "lCustom = PEMSTATUS(oUnsupportedCustom, 'WordWrap', 1)\n"
            "oDerived = CREATEOBJECT('DerivedWordWrapLabel')\n"
            "lDerived = oDerived.WordWrap\n"
            "lMember = .F.\n"
            "nMembers = AMEMBERS(aMembers, oDerived, 1)\n"
            "FOR i = 1 TO nMembers\n"
            "    IF UPPER(aMembers[i]) == 'WORDWRAP'\n"
            "        lMember = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "RETURN\n"
            "DEFINE CLASS DerivedWordWrapLabel AS Label\n"
            "    WordWrap = .T.\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native WordWrap property script should complete: ") + state.message +
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

        check("lcheckbox", "true");
        check("lcommandbutton", "true");
        check("llabel", "true");
        check("loptionbutton", "true");
        check("ldefault", "false");
        check("lgetpem", "false");
        check("lreadonly", "false");
        check("ldirect", "true");
        check("lsetpem", "true");
        check("lsetpemvalue", "true");
        check("lputpem", "true");
        check("lputpemvalue", "true");
        check("laddproperty", "false");
        check("lremoveproperty", "false");
        check("lform", "false");
        check("lcombobox", "false");
        check("ltextbox", "false");
        check("lcontainer", "false");
        check("loptiongroup", "false");
        check("limage", "false");
        check("lcustom", "false");
        check("lderived", "true");
        check("lmember", "true");
        expect(state.ole_objects.size() == 12U,
               "native WordWrap coverage should register documented controls and exclusions");

        fs::remove_all(temp_root, ignored);
    }
}
