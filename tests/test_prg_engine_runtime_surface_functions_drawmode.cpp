#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_visual_drawmode_defaults_mutates_and_stays_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_visual_drawmode";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_visual_drawmode.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('Form')\n"
            "oLine = CREATEOBJECT('Line')\n"
            "oShape = CREATEOBJECT('Shape')\n"
            "oImage = CREATEOBJECT('Image')\n"
            "lFormHas = PEMSTATUS(oForm, 'DrawMode', 1)\n"
            "lLineHas = PEMSTATUS(oLine, 'DrawMode', 1)\n"
            "lShapeHas = PEMSTATUS(oShape, 'DrawMode', 1)\n"
            "lImageHas = PEMSTATUS(oImage, 'DrawMode', 1)\n"
            "nDefault = oForm.DrawMode\n"
            "oLine.DrawMode = 7.6\n"
            "nDirect = oLine.DrawMode\n"
            "lSetPem = SETPEM(oShape, 'DrawMode', 16)\n"
            "nSetPem = GETPEM(oShape, 'DrawMode')\n"
            "lPutPem = PUTPEM(oForm, 'DrawMode', 0)\n"
            "nPutPem = GETPEM(oForm, 'DrawMode')\n"
            "oForm.DrawMode = -3\n"
            "nNegative = oForm.DrawMode\n"
            "lAddProperty = ADDPROPERTY(oForm, 'DrawMode', 1)\n"
            "lRemoveProperty = REMOVEPROPERTY(oForm, 'DrawMode')\n"
            "lMember = .F.\n"
            "nMembers = AMEMBERS(aMembers, oForm, 1)\n"
            "FOR i = 1 TO nMembers\n"
            "    IF UPPER(aMembers[i]) == 'DRAWMODE'\n"
            "        lMember = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedDrawLine')\n"
            "nDerived = oDerived.DrawMode\n"
            "RETURN\n"
            "DEFINE CLASS DerivedDrawLine AS Line\n"
            "    DrawMode = 2\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native DrawMode script should complete: ") + state.message +
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

        check("lformhas", "true");
        check("llinehas", "true");
        check("lshapehas", "true");
        check("limagehas", "false");
        check("ndefault", "13");
        check("ndirect", "8");
        check("lsetpem", "true");
        check("nsetpem", "16");
        check("lputpem", "true");
        check("nputpem", "13");
        check("nnegative", "13");
        check("laddproperty", "false");
        check("lremoveproperty", "false");
        check("lmember", "true");
        check("nderived", "2");
        expect(state.ole_objects.size() == 5U,
               "native DrawMode coverage should register Form, Line, Shape, Image, and derived Line");

        fs::remove_all(temp_root, ignored);
    }
}
