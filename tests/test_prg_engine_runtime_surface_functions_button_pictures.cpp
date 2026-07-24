#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_button_state_picture_properties_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_button_pictures";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_button_pictures.prg";
        write_text(
            main_path,
            "oButton = CREATEOBJECT('CommandButton')\n"
            "oCheck = CREATEOBJECT('CheckBox')\n"
            "oImage = CREATEOBJECT('Image')\n"
            "oLabel = CREATEOBJECT('Label')\n"
            "cButtonDownDefault = oButton.DownPicture\n"
            "cButtonDisabledDefault = GETPEM(oButton, 'DisabledPicture')\n"
            "lButtonDownHas = PEMSTATUS(oButton, 'DownPicture', 1)\n"
            "lButtonDisabledHas = PEMSTATUS(oButton, 'DisabledPicture', 1)\n"
            "lCheckDownHas = PEMSTATUS(oCheck, 'DownPicture', 1)\n"
            "lCheckDisabledHas = PEMSTATUS(oCheck, 'DisabledPicture', 1)\n"
            "lImageDownHas = PEMSTATUS(oImage, 'DownPicture', 1)\n"
            "lLabelDisabledHas = PEMSTATUS(oLabel, 'DisabledPicture', 1)\n"
            "oButton.DownPicture = 'buttons\\save_down.bmp'\n"
            "cDirectDown = oButton.DownPicture\n"
            "lSetDisabled = SETPEM(oButton, 'DisabledPicture', 'buttons\\save_disabled.bmp')\n"
            "cSetDisabled = GETPEM(oButton, 'DisabledPicture')\n"
            "lPutDown = PUTPEM(oCheck, 'DownPicture', 'buttons\\check_down.bmp')\n"
            "cPutDown = GETPEM(oCheck, 'DownPicture')\n"
            "lAddDown = ADDPROPERTY(oButton, 'DownPicture', 'shadow.bmp')\n"
            "lRemoveDisabled = REMOVEPROPERTY(oButton, 'DisabledPicture')\n"
            "lDownMember = .F.\n"
            "lDisabledMember = .F.\n"
            "nMembers = AMEMBERS(aMembers, oButton, 1)\n"
            "FOR i = 1 TO nMembers\n"
            "    IF UPPER(aMembers[i]) == 'DOWNPICTURE'\n"
            "        lDownMember = .T.\n"
            "    ENDIF\n"
            "    IF UPPER(aMembers[i]) == 'DISABLEDPICTURE'\n"
            "        lDisabledMember = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedButtonPictures')\n"
            "cDerivedDown = oDerived.DownPicture\n"
            "cDerivedDisabled = oDerived.DisabledPicture\n"
            "RETURN\n"
            "DEFINE CLASS DerivedButtonPictures AS CommandButton\n"
            "    DownPicture = 'buttons\\class_down.bmp'\n"
            "    DisabledPicture = 'buttons\\class_disabled.bmp'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native button picture script should complete: ") + state.message +
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

        check("cbuttondowndefault", "");
        check("cbuttondisableddefault", "");
        check("lbuttondownhas", "true");
        check("lbuttondisabledhas", "true");
        check("lcheckdownhas", "true");
        check("lcheckdisabledhas", "true");
        check("limagedownhas", "false");
        check("llabeldisabledhas", "false");
        check("cdirectdown", "buttons\\save_down.bmp");
        check("lsetdisabled", "true");
        check("csetdisabled", "buttons\\save_disabled.bmp");
        check("lputdown", "true");
        check("cputdown", "buttons\\check_down.bmp");
        check("ladddown", "false");
        check("lremovedisabled", "false");
        check("ldownmember", "true");
        check("ldisabledmember", "true");
        check("cderiveddown", "buttons\\class_down.bmp");
        check("cderiveddisabled", "buttons\\class_disabled.bmp");
        expect(state.ole_objects.size() == 5U,
               "native button picture coverage should register four base objects and the derived object");

        fs::remove_all(temp_root, ignored);
    }
}
