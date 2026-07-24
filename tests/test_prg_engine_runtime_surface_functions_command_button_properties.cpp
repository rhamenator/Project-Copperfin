#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_commandbutton_default_cancel_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_commandbutton_default_cancel";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_commandbutton_default_cancel.prg";
        write_text(
            main_path,
            "oButton = CREATEOBJECT('CommandButton')\n"
            "lDefaultHas = PEMSTATUS(oButton, 'Default', 1)\n"
            "lCancelHas = PEMSTATUS(oButton, 'Cancel', 1)\n"
            "lDefaultReadOnly = PEMSTATUS(oButton, 'Default', 5)\n"
            "lCancelReadOnly = PEMSTATUS(oButton, 'Cancel', 5)\n"
            "lDefaultMember = .F.\n"
            "lCancelMember = .F.\n"
            "nMembers = AMEMBERS(aMembers, oButton, 1)\n"
            "FOR i = 1 TO nMembers\n"
            "    IF UPPER(aMembers[i]) == 'DEFAULT'\n"
            "        lDefaultMember = .T.\n"
            "    ENDIF\n"
            "    IF UPPER(aMembers[i]) == 'CANCEL'\n"
            "        lCancelMember = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "lDefaultInitial = oButton.Default\n"
            "lCancelInitial = GETPEM(oButton, 'Cancel')\n"
            "oButton.Default = 1.6\n"
            "lDefaultDirect = oButton.Default\n"
            "lCancelSetPem = SETPEM(oButton, 'Cancel', 1)\n"
            "lCancelAfterSetPem = GETPEM(oButton, 'Cancel')\n"
            "lDefaultPutPem = PUTPEM(oButton, 'Default', 0)\n"
            "lDefaultAfterPutPem = GETPEM(oButton, 'Default')\n"
            "lAddDefault = ADDPROPERTY(oButton, 'Default', .T.)\n"
            "lRemoveCancel = REMOVEPROPERTY(oButton, 'Cancel')\n"
            "oDerived = CREATEOBJECT('DerivedCommandButton')\n"
            "lDerivedDefault = oDerived.Default\n"
            "lDerivedCancel = oDerived.Cancel\n"
            "RETURN\n"
            "DEFINE CLASS DerivedCommandButton AS CommandButton\n"
            "    Default = .T.\n"
            "    Cancel = 1.6\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native CommandButton Default/Cancel script should complete: ") + state.message +
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

        check("ldefaulthas", "true");
        check("lcancelhas", "true");
        check("ldefaultreadonly", "false");
        check("lcancelreadonly", "false");
        check("ldefaultmember", "true");
        check("lcancelmember", "true");
        check("ldefaultinitial", "false");
        check("lcancelinitial", "false");
        check("ldefaultdirect", "true");
        check("lcancelsetpem", "true");
        check("lcancelaftersetpem", "true");
        check("ldefaultputpem", "true");
        check("ldefaultafterputpem", "false");
        check("ladddefault", "false");
        check("lremovecancel", "false");
        check("lderiveddefault", "true");
        check("lderivedcancel", "true");
        expect(state.ole_objects.size() == 2U,
               "native CommandButton Default/Cancel coverage should register the base and derived objects");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_commandbutton_style_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_commandbutton_style";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_commandbutton_style.prg";
        write_text(
            main_path,
            "oButton = CREATEOBJECT('CommandButton')\n"
            "lHas = PEMSTATUS(oButton, 'Style', 1)\n"
            "lReadOnly = PEMSTATUS(oButton, 'Style', 5)\n"
            "nInitial = oButton.Style\n"
            "oButton.Style = 1\n"
            "nDirect = oButton.Style\n"
            "lSetPem = SETPEM(oButton, 'Style', 4)\n"
            "nAfterSetPem = GETPEM(oButton, 'Style')\n"
            "lPutPem = PUTPEM(oButton, 'Style', -1)\n"
            "nAfterPutPem = GETPEM(oButton, 'Style')\n"
            "lAdd = ADDPROPERTY(oButton, 'Style', 1)\n"
            "lRemove = REMOVEPROPERTY(oButton, 'Style')\n"
            "lMember = .F.\n"
            "nMembers = AMEMBERS(aMembers, oButton, 1)\n"
            "FOR i = 1 TO nMembers\n"
            "    IF UPPER(aMembers[i]) == 'STYLE'\n"
            "        lMember = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedCommandButtonStyle')\n"
            "nDerived = oDerived.Style\n"
            "RETURN\n"
            "DEFINE CLASS DerivedCommandButtonStyle AS CommandButton\n"
            "    Style = 1\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native CommandButton Style script should complete: ") + state.message +
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
        check("ninitial", "0");
        check("ndirect", "1");
        check("lsetpem", "true");
        check("naftersetpem", "0");
        check("lputpem", "true");
        check("nafterputpem", "0");
        check("ladd", "false");
        check("lremove", "false");
        check("lmember", "true");
        check("nderived", "1");
        expect(state.ole_objects.size() == 2U,
               "native CommandButton Style coverage should register the base and derived objects");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_commandbutton_picture_layout_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_commandbutton_picture_layout";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_commandbutton_picture_layout.prg";
        write_text(
            main_path,
            "oButton = CREATEOBJECT('CommandButton')\n"
            "lMarginHas = PEMSTATUS(oButton, 'PictureMargin', 1)\n"
            "lPositionHas = PEMSTATUS(oButton, 'PicturePosition', 1)\n"
            "lSpacingHas = PEMSTATUS(oButton, 'PictureSpacing', 1)\n"
            "nMarginInitial = oButton.PictureMargin\n"
            "nPositionInitial = GETPEM(oButton, 'PicturePosition')\n"
            "nSpacingInitial = oButton.PictureSpacing\n"
            "oButton.PictureMargin = 42.6\n"
            "nMarginDirect = oButton.PictureMargin\n"
            "lSetPosition = SETPEM(oButton, 'PicturePosition', 14)\n"
            "nPositionAfterSetPem = GETPEM(oButton, 'PicturePosition')\n"
            "lPutSpacing = PUTPEM(oButton, 'PictureSpacing', 65535)\n"
            "nSpacingAfterPutPem = GETPEM(oButton, 'PictureSpacing')\n"
            "lSetMarginInvalid = SETPEM(oButton, 'PictureMargin', -1)\n"
            "nMarginAfterInvalid = GETPEM(oButton, 'PictureMargin')\n"
            "lPutPositionInvalid = PUTPEM(oButton, 'PicturePosition', 15)\n"
            "nPositionAfterInvalid = GETPEM(oButton, 'PicturePosition')\n"
            "lAddSpacing = ADDPROPERTY(oButton, 'PictureSpacing', 1)\n"
            "lRemoveMargin = REMOVEPROPERTY(oButton, 'PictureMargin')\n"
            "lMarginMember = .F.\n"
            "lPositionMember = .F.\n"
            "lSpacingMember = .F.\n"
            "nMembers = AMEMBERS(aMembers, oButton, 1)\n"
            "FOR i = 1 TO nMembers\n"
            "    IF UPPER(aMembers[i]) == 'PICTUREMARGIN'\n"
            "        lMarginMember = .T.\n"
            "    ENDIF\n"
            "    IF UPPER(aMembers[i]) == 'PICTUREPOSITION'\n"
            "        lPositionMember = .T.\n"
            "    ENDIF\n"
            "    IF UPPER(aMembers[i]) == 'PICTURESPACING'\n"
            "        lSpacingMember = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oCheck = CREATEOBJECT('CheckBox')\n"
            "lCheckMargin = PEMSTATUS(oCheck, 'PictureMargin', 1)\n"
            "oDerived = CREATEOBJECT('DerivedCommandButtonPictureLayout')\n"
            "nDerivedMargin = oDerived.PictureMargin\n"
            "nDerivedPosition = oDerived.PicturePosition\n"
            "nDerivedSpacing = oDerived.PictureSpacing\n"
            "RETURN\n"
            "DEFINE CLASS DerivedCommandButtonPictureLayout AS CommandButton\n"
            "    PictureMargin = 7\n"
            "    PicturePosition = 12\n"
            "    PictureSpacing = 9\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native CommandButton picture-layout script should complete: ") + state.message +
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

        check("lmarginhas", "true");
        check("lpositionhas", "true");
        check("lspacinghas", "true");
        check("nmargininitial", "0");
        check("npositioninitial", "13");
        check("nspacinginitial", "0");
        check("nmargindirect", "43");
        check("lsetposition", "true");
        check("npositionaftersetpem", "14");
        check("lputspacing", "true");
        check("nspacingafterputpem", "65535");
        check("lsetmargininvalid", "true");
        check("nmarginafterinvalid", "0");
        check("lputpositioninvalid", "true");
        check("npositionafterinvalid", "13");
        check("laddspacing", "false");
        check("lremovemargin", "false");
        check("lmarginmember", "true");
        check("lpositionmember", "true");
        check("lspacingmember", "true");
        check("lcheckmargin", "false");
        check("nderivedmargin", "7");
        check("nderivedposition", "12");
        check("nderivedspacing", "9");
        expect(state.ole_objects.size() == 3U,
               "native CommandButton picture-layout coverage should register the button, check box, and derived objects");

        fs::remove_all(temp_root, ignored);
    }
}
