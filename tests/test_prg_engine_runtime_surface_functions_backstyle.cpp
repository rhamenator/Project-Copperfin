#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_visual_backstyle_defaults_mutates_and_stays_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_visual_backstyle";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_visual_backstyle.prg";
        write_text(
            main_path,
            "oCheck = CREATEOBJECT('CheckBox')\n"
            "oGroup = CREATEOBJECT('CommandGroup')\n"
            "oContainer = CREATEOBJECT('Container')\n"
            "oEdit = CREATEOBJECT('EditBox')\n"
            "oImage = CREATEOBJECT('Image')\n"
            "oLabel = CREATEOBJECT('Label')\n"
            "oOption = CREATEOBJECT('OptionButton')\n"
            "oOptions = CREATEOBJECT('OptionGroup')\n"
            "oPage = CREATEOBJECT('Page')\n"
            "oShape = CREATEOBJECT('Shape')\n"
            "oText = CREATEOBJECT('TextBox')\n"
            "oForm = CREATEOBJECT('Form')\n"
            "oLine = CREATEOBJECT('Line')\n"
            "oSpinner = CREATEOBJECT('Spinner')\n"
            "lCheckHas = PEMSTATUS(oCheck, 'BackStyle', 1)\n"
            "lGroupHas = PEMSTATUS(oGroup, 'BackStyle', 1)\n"
            "lContainerHas = PEMSTATUS(oContainer, 'BackStyle', 1)\n"
            "lEditHas = PEMSTATUS(oEdit, 'BackStyle', 1)\n"
            "lImageHas = PEMSTATUS(oImage, 'BackStyle', 1)\n"
            "lLabelHas = PEMSTATUS(oLabel, 'BackStyle', 1)\n"
            "lOptionHas = PEMSTATUS(oOption, 'BackStyle', 1)\n"
            "lOptionsHas = PEMSTATUS(oOptions, 'BackStyle', 1)\n"
            "lPageHas = PEMSTATUS(oPage, 'BackStyle', 1)\n"
            "lShapeHas = PEMSTATUS(oShape, 'BackStyle', 1)\n"
            "lTextHas = PEMSTATUS(oText, 'BackStyle', 1)\n"
            "lFormHas = PEMSTATUS(oForm, 'BackStyle', 1)\n"
            "lLineHas = PEMSTATUS(oLine, 'BackStyle', 1)\n"
            "lSpinnerHas = PEMSTATUS(oSpinner, 'BackStyle', 1)\n"
            "nDefault = oLabel.BackStyle\n"
            "oEdit.BackStyle = 0.4\n"
            "nDirect = oEdit.BackStyle\n"
            "lSetPem = SETPEM(oShape, 'BackStyle', 0)\n"
            "nSetPem = GETPEM(oShape, 'BackStyle')\n"
            "lPutPem = PUTPEM(oLabel, 'BackStyle', 9)\n"
            "nPutPem = GETPEM(oLabel, 'BackStyle')\n"
            "oLabel.BackStyle = -2\n"
            "nNegative = oLabel.BackStyle\n"
            "lAddProperty = ADDPROPERTY(oLabel, 'BackStyle', 0)\n"
            "lRemoveProperty = REMOVEPROPERTY(oLabel, 'BackStyle')\n"
            "lMember = .F.\n"
            "nMembers = AMEMBERS(aMembers, oLabel, 1)\n"
            "FOR i = 1 TO nMembers\n"
            "    IF UPPER(aMembers[i]) == 'BACKSTYLE'\n"
            "        lMember = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedBackStyleText')\n"
            "nDerived = oDerived.BackStyle\n"
            "RETURN\n"
            "DEFINE CLASS DerivedBackStyleText AS TextBox\n"
            "    BackStyle = 0\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native BackStyle script should complete: ") + state.message +
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

        check("lcheckhas", "true");
        check("lgrouphas", "true");
        check("lcontainerhas", "true");
        check("ledithas", "true");
        check("limagehas", "true");
        check("llabelhas", "true");
        check("loptionhas", "true");
        check("loptionshas", "true");
        check("lpagehas", "true");
        check("lshapehas", "true");
        check("ltexthas", "true");
        check("lformhas", "false");
        check("llinehas", "false");
        check("lspinnerhas", "false");
        check("ndefault", "1");
        check("ndirect", "0");
        check("lsetpem", "true");
        check("nsetpem", "0");
        check("lputpem", "true");
        check("nputpem", "1");
        check("nnegative", "1");
        check("laddproperty", "false");
        check("lremoveproperty", "false");
        check("lmember", "true");
        check("nderived", "0");

        fs::remove_all(temp_root, ignored);
    }
}
