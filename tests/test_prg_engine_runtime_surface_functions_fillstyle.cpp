#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_visual_fillstyle_defaults_mutates_and_stays_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_visual_fillstyle";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_visual_fillstyle.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('Form')\n"
            "oShape = CREATEOBJECT('Shape')\n"
            "oLabel = CREATEOBJECT('Label')\n"
            "lFormHas = PEMSTATUS(oForm, 'FillStyle', 1)\n"
            "lShapeHas = PEMSTATUS(oShape, 'FillStyle', 1)\n"
            "lLabelHas = PEMSTATUS(oLabel, 'FillStyle', 1)\n"
            "nDefault = oShape.FillStyle\n"
            "oShape.FillStyle = 2.4\n"
            "nDirect = oShape.FillStyle\n"
            "lSetPem = SETPEM(oForm, 'FillStyle', 7)\n"
            "nSetPem = GETPEM(oForm, 'FillStyle')\n"
            "lPutPem = PUTPEM(oShape, 'FillStyle', 99)\n"
            "nPutPem = GETPEM(oShape, 'FillStyle')\n"
            "oShape.FillStyle = -2\n"
            "nNegative = oShape.FillStyle\n"
            "lAddProperty = ADDPROPERTY(oShape, 'FillStyle', 0)\n"
            "lRemoveProperty = REMOVEPROPERTY(oShape, 'FillStyle')\n"
            "lMember = .F.\n"
            "nMembers = AMEMBERS(aMembers, oShape, 1)\n"
            "FOR i = 1 TO nMembers\n"
            "    IF UPPER(aMembers[i]) == 'FILLSTYLE'\n"
            "        lMember = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedFillShape')\n"
            "nDerived = oDerived.FillStyle\n"
            "RETURN\n"
            "DEFINE CLASS DerivedFillShape AS Shape\n"
            "    FillStyle = 0\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native FillStyle script should complete: ") + state.message +
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
        check("lshapehas", "true");
        check("llabelhas", "false");
        check("ndefault", "1");
        check("ndirect", "2");
        check("lsetpem", "true");
        check("nsetpem", "7");
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
