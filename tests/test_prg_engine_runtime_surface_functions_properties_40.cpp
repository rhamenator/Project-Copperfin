#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_visual_righttoleft_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_visual_righttoleft";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_visual_righttoleft.prg";
        write_text(
            main_path,
            "oCheckBox = CREATEOBJECT('CheckBox')\n"
            "oComboBox = CREATEOBJECT('ComboBox')\n"
            "oCommandButton = CREATEOBJECT('CommandButton')\n"
            "oEditBox = CREATEOBJECT('EditBox')\n"
            "oForm = CREATEOBJECT('Form')\n"
            "oGrid = CREATEOBJECT('Grid')\n"
            "oLabel = CREATEOBJECT('Label')\n"
            "oListBox = CREATEOBJECT('ListBox')\n"
            "oOptionButton = CREATEOBJECT('OptionButton')\n"
            "oPageFrame = CREATEOBJECT('PageFrame')\n"
            "oSpinner = CREATEOBJECT('Spinner')\n"
            "oTextBox = CREATEOBJECT('TextBox')\n"
            "lCheckBox = PEMSTATUS(oCheckBox, 'RightToLeft', 1)\n"
            "lComboBox = PEMSTATUS(oComboBox, 'RightToLeft', 1)\n"
            "lCommandButton = PEMSTATUS(oCommandButton, 'RightToLeft', 1)\n"
            "lEditBox = PEMSTATUS(oEditBox, 'RightToLeft', 1)\n"
            "lForm = PEMSTATUS(oForm, 'RightToLeft', 1)\n"
            "lGrid = PEMSTATUS(oGrid, 'RightToLeft', 1)\n"
            "lLabel = PEMSTATUS(oLabel, 'RightToLeft', 1)\n"
            "lListBox = PEMSTATUS(oListBox, 'RightToLeft', 1)\n"
            "lOptionButton = PEMSTATUS(oOptionButton, 'RightToLeft', 1)\n"
            "lPageFrame = PEMSTATUS(oPageFrame, 'RightToLeft', 1)\n"
            "lSpinner = PEMSTATUS(oSpinner, 'RightToLeft', 1)\n"
            "lTextBox = PEMSTATUS(oTextBox, 'RightToLeft', 1)\n"
            "lFormDefault = oForm.RightToLeft\n"
            "lTextBoxDefault = GETPEM(oTextBox, 'RightToLeft')\n"
            "lFormReadOnly = PEMSTATUS(oForm, 'RightToLeft', 5)\n"
            "oForm.RightToLeft = .F.\n"
            "lDirect = oForm.RightToLeft\n"
            "lSetPem = SETPEM(oTextBox, 'RightToLeft', .F.)\n"
            "lSetPemValue = oTextBox.RightToLeft\n"
            "lPutPem = PUTPEM(oLabel, 'RightToLeft', .F.)\n"
            "lPutPemValue = oLabel.RightToLeft\n"
            "lAddProperty = ADDPROPERTY(oForm, 'RightToLeft', .T.)\n"
            "lRemoveProperty = REMOVEPROPERTY(oForm, 'RightToLeft')\n"
            "oUnsupportedContainer = CREATEOBJECT('Container')\n"
            "oUnsupportedImage = CREATEOBJECT('Image')\n"
            "oUnsupportedLine = CREATEOBJECT('Line')\n"
            "oUnsupportedOptionGroup = CREATEOBJECT('OptionGroup')\n"
            "oUnsupportedPage = CREATEOBJECT('Page')\n"
            "oUnsupportedShape = CREATEOBJECT('Shape')\n"
            "oUnsupportedToolbar = CREATEOBJECT('ToolBar')\n"
            "oUnsupportedCustom = CREATEOBJECT('Custom')\n"
            "lContainer = PEMSTATUS(oUnsupportedContainer, 'RightToLeft', 1)\n"
            "lImage = PEMSTATUS(oUnsupportedImage, 'RightToLeft', 1)\n"
            "lLine = PEMSTATUS(oUnsupportedLine, 'RightToLeft', 1)\n"
            "lOptionGroup = PEMSTATUS(oUnsupportedOptionGroup, 'RightToLeft', 1)\n"
            "lPage = PEMSTATUS(oUnsupportedPage, 'RightToLeft', 1)\n"
            "lShape = PEMSTATUS(oUnsupportedShape, 'RightToLeft', 1)\n"
            "lToolbar = PEMSTATUS(oUnsupportedToolbar, 'RightToLeft', 1)\n"
            "lCustom = PEMSTATUS(oUnsupportedCustom, 'RightToLeft', 1)\n"
            "oDerived = CREATEOBJECT('DerivedRightToLeftForm')\n"
            "lDerived = oDerived.RightToLeft\n"
            "lMember = .F.\n"
            "nMembers = AMEMBERS(aMembers, oDerived, 1)\n"
            "FOR i = 1 TO nMembers\n"
            "    IF UPPER(aMembers[i]) == 'RIGHTTOLEFT'\n"
            "        lMember = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "RETURN\n"
            "DEFINE CLASS DerivedRightToLeftForm AS Form\n"
            "    RightToLeft = .F.\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native RightToLeft property script should complete: ") + state.message +
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
        check("lcombobox", "true");
        check("lcommandbutton", "true");
        check("leditbox", "true");
        check("lform", "true");
        check("lgrid", "true");
        check("llabel", "true");
        check("llistbox", "true");
        check("loptionbutton", "true");
        check("lpageframe", "true");
        check("lspinner", "true");
        check("ltextbox", "true");
        check("lformdefault", "true");
        check("ltextboxdefault", "true");
        check("lformreadonly", "false");
        check("ldirect", "false");
        check("lsetpem", "true");
        check("lsetpemvalue", "false");
        check("lputpem", "true");
        check("lputpemvalue", "false");
        check("laddproperty", "false");
        check("lremoveproperty", "false");
        check("lcontainer", "false");
        check("limage", "false");
        check("lline", "false");
        check("loptiongroup", "false");
        check("lpage", "false");
        check("lshape", "false");
        check("ltoolbar", "false");
        check("lcustom", "false");
        check("lderived", "false");
        check("lmember", "true");
        expect(state.ole_objects.size() == 21U,
               "native RightToLeft coverage should register documented controls and exclusions");

        fs::remove_all(temp_root, ignored);
    }
}
