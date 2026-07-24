#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_visual_borderwidth_defaults_mutates_and_stays_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_visual_borderwidth";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_visual_borderwidth.prg";
        write_text(
            main_path,
            "oContainer = CREATEOBJECT('Container')\n"
            "oLine = CREATEOBJECT('Line')\n"
            "oPageFrame = CREATEOBJECT('PageFrame')\n"
            "oShape = CREATEOBJECT('Shape')\n"
            "oForm = CREATEOBJECT('Form')\n"
            "lContainerHas = PEMSTATUS(oContainer, 'BorderWidth', 1)\n"
            "lLineHas = PEMSTATUS(oLine, 'BorderWidth', 1)\n"
            "lPageFrameHas = PEMSTATUS(oPageFrame, 'BorderWidth', 1)\n"
            "lShapeHas = PEMSTATUS(oShape, 'BorderWidth', 1)\n"
            "lFormHas = PEMSTATUS(oForm, 'BorderWidth', 1)\n"
            "nDefault = oShape.BorderWidth\n"
            "oShape.BorderWidth = 2.4\n"
            "nDirect = oShape.BorderWidth\n"
            "lSetPem = SETPEM(oLine, 'BorderWidth', 8192)\n"
            "nSetPem = GETPEM(oLine, 'BorderWidth')\n"
            "lPutPem = PUTPEM(oContainer, 'BorderWidth', 9000)\n"
            "nPutPem = GETPEM(oContainer, 'BorderWidth')\n"
            "oPageFrame.BorderWidth = -2\n"
            "nNegative = oPageFrame.BorderWidth\n"
            "lAddProperty = ADDPROPERTY(oShape, 'BorderWidth', 0)\n"
            "lRemoveProperty = REMOVEPROPERTY(oShape, 'BorderWidth')\n"
            "lMember = .F.\n"
            "nMembers = AMEMBERS(aMembers, oShape, 1)\n"
            "FOR i = 1 TO nMembers\n"
            "    IF UPPER(aMembers[i]) == 'BORDERWIDTH'\n"
            "        lMember = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedBorderShape')\n"
            "nDerived = oDerived.BorderWidth\n"
            "RETURN\n"
            "DEFINE CLASS DerivedBorderShape AS Shape\n"
            "    BorderWidth = 0\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native BorderWidth script should complete: ") + state.message +
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

        check("lcontainerhas", "true");
        check("llinehas", "true");
        check("lpageframehas", "true");
        check("lshapehas", "true");
        check("lformhas", "false");
        check("ndefault", "1");
        check("ndirect", "2");
        check("lsetpem", "true");
        check("nsetpem", "8192");
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
