#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_visual_borderstyle_defaults_mutates_and_stays_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_visual_borderstyle";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_visual_borderstyle.prg";
        write_text(
            main_path,
            "oCombo = CREATEOBJECT('ComboBox')\n"
            "oGroup = CREATEOBJECT('CommandGroup')\n"
            "oEdit = CREATEOBJECT('EditBox')\n"
            "oImage = CREATEOBJECT('Image')\n"
            "oLabel = CREATEOBJECT('Label')\n"
            "oLine = CREATEOBJECT('Line')\n"
            "oOptions = CREATEOBJECT('OptionGroup')\n"
            "oShape = CREATEOBJECT('Shape')\n"
            "oSpinner = CREATEOBJECT('Spinner')\n"
            "lAll = PEMSTATUS(oCombo, 'BorderStyle', 1) AND PEMSTATUS(oGroup, 'BorderStyle', 1) AND PEMSTATUS(oEdit, 'BorderStyle', 1) AND PEMSTATUS(oImage, 'BorderStyle', 1) AND PEMSTATUS(oLabel, 'BorderStyle', 1) AND PEMSTATUS(oLine, 'BorderStyle', 1) AND PEMSTATUS(oOptions, 'BorderStyle', 1) AND PEMSTATUS(oShape, 'BorderStyle', 1) AND PEMSTATUS(oSpinner, 'BorderStyle', 1)\n"
            "nImageDefault = oImage.BorderStyle\n"
            "nLineDefault = oLine.BorderStyle\n"
            "nComboDefault = oCombo.BorderStyle\n"
            "oImage.BorderStyle = 1.2\n"
            "nImageDirect = oImage.BorderStyle\n"
            "oLine.BorderStyle = 5.6\n"
            "nLineDirect = oLine.BorderStyle\n"
            "lSetPem = SETPEM(oCombo, 'BorderStyle', 0)\n"
            "nSetPem = GETPEM(oCombo, 'BorderStyle')\n"
            "lPutPem = PUTPEM(oShape, 'BorderStyle', 9)\n"
            "nPutPem = GETPEM(oShape, 'BorderStyle')\n"
            "oLabel.BorderStyle = -1\n"
            "nLabelNegative = oLabel.BorderStyle\n"
            "lAddProperty = ADDPROPERTY(oShape, 'BorderStyle', 1)\n"
            "lRemoveProperty = REMOVEPROPERTY(oShape, 'BorderStyle')\n"
            "lMember = .F.\n"
            "nMembers = AMEMBERS(aMembers, oShape, 1)\n"
            "FOR i = 1 TO nMembers\n"
            "    IF UPPER(aMembers[i]) == 'BORDERSTYLE'\n"
            "        lMember = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedBorderStyleShape')\n"
            "nDerived = oDerived.BorderStyle\n"
            "RETURN\n"
            "DEFINE CLASS DerivedBorderStyleShape AS Shape\n"
            "    BorderStyle = 4\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native visual BorderStyle script should complete: ") + state.message +
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

        check("lall", "true");
        check("nimagedefault", "0");
        check("nlinedefault", "1");
        check("ncombodefault", "1");
        check("nimagedirect", "1");
        check("nlinedirect", "6");
        check("lsetpem", "true");
        check("nsetpem", "0");
        check("lputpem", "true");
        check("nputpem", "1");
        check("nlabelnegative", "0");
        check("laddproperty", "false");
        check("lremoveproperty", "false");
        check("lmember", "true");
        check("nderived", "4");

        fs::remove_all(temp_root, ignored);
    }
}
