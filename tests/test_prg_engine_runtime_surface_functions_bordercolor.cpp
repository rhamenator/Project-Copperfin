#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_visual_bordercolor_defaults_mutates_and_stays_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_visual_bordercolor";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_visual_bordercolor.prg";
        write_text(
            main_path,
            "oContainer = CREATEOBJECT('Container')\n"
            "oLine = CREATEOBJECT('Line')\n"
            "oPageFrame = CREATEOBJECT('PageFrame')\n"
            "oShape = CREATEOBJECT('Shape')\n"
            "oForm = CREATEOBJECT('Form')\n"
            "oOle = CREATEOBJECT('OleControl')\n"
            "lContainerHas = PEMSTATUS(oContainer, 'BorderColor', 1)\n"
            "lLineHas = PEMSTATUS(oLine, 'BorderColor', 1)\n"
            "lPageFrameHas = PEMSTATUS(oPageFrame, 'BorderColor', 1)\n"
            "lShapeHas = PEMSTATUS(oShape, 'BorderColor', 1)\n"
            "lFormHas = PEMSTATUS(oForm, 'BorderColor', 1)\n"
            "lOleHas = PEMSTATUS(oOle, 'BorderColor', 1)\n"
            "nDefault = oShape.BorderColor\n"
            "oShape.BorderColor = 123.9\n"
            "nDirect = oShape.BorderColor\n"
            "lSetPem = SETPEM(oLine, 'BorderColor', -2147483640)\n"
            "nSetPem = GETPEM(oLine, 'BorderColor')\n"
            "lPutPem = PUTPEM(oContainer, 'BorderColor', 456.8)\n"
            "nPutPem = GETPEM(oContainer, 'BorderColor')\n"
            "oPageFrame.BorderColor = 789.4\n"
            "nFraction = oPageFrame.BorderColor\n"
            "lAddProperty = ADDPROPERTY(oShape, 'BorderColor', 0)\n"
            "lRemoveProperty = REMOVEPROPERTY(oShape, 'BorderColor')\n"
            "lMember = .F.\n"
            "nMembers = AMEMBERS(aMembers, oShape, 1)\n"
            "FOR i = 1 TO nMembers\n"
            "    IF UPPER(aMembers[i]) == 'BORDERCOLOR'\n"
            "        lMember = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedBorderShape')\n"
            "nDerived = oDerived.BorderColor\n"
            "RETURN\n"
            "DEFINE CLASS DerivedBorderShape AS Shape\n"
            "    BorderColor = 7\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native BorderColor script should complete: ") + state.message +
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
        check("lolehas", "false");
        check("ndefault", "0");
        check("ndirect", "123");
        check("lsetpem", "true");
        check("nsetpem", "-2147483640");
        check("lputpem", "true");
        check("nputpem", "456");
        check("nfraction", "789");
        check("laddproperty", "false");
        check("lremoveproperty", "false");
        check("lmember", "true");
        check("nderived", "7");

        fs::remove_all(temp_root, ignored);
    }
}
