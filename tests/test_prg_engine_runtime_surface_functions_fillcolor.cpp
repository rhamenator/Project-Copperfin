#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_visual_fillcolor_defaults_mutates_and_stays_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_visual_fillcolor";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_visual_fillcolor.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('Form')\n"
            "oShape = CREATEOBJECT('Shape')\n"
            "oLabel = CREATEOBJECT('Label')\n"
            "lFormHas = PEMSTATUS(oForm, 'FillColor', 1)\n"
            "lShapeHas = PEMSTATUS(oShape, 'FillColor', 1)\n"
            "lLabelHas = PEMSTATUS(oLabel, 'FillColor', 1)\n"
            "nDefault = oShape.FillColor\n"
            "oShape.FillColor = 123.9\n"
            "nDirect = oShape.FillColor\n"
            "lSetPem = SETPEM(oForm, 'FillColor', -2147483640)\n"
            "nSetPem = GETPEM(oForm, 'FillColor')\n"
            "lPutPem = PUTPEM(oShape, 'FillColor', 456.8)\n"
            "nPutPem = GETPEM(oShape, 'FillColor')\n"
            "oForm.FillColor = 789.4\n"
            "nFraction = oForm.FillColor\n"
            "lAddProperty = ADDPROPERTY(oShape, 'FillColor', 0)\n"
            "lRemoveProperty = REMOVEPROPERTY(oShape, 'FillColor')\n"
            "lMember = .F.\n"
            "nMembers = AMEMBERS(aMembers, oShape, 1)\n"
            "FOR i = 1 TO nMembers\n"
            "    IF UPPER(aMembers[i]) == 'FILLCOLOR'\n"
            "        lMember = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedFillShape')\n"
            "nDerived = oDerived.FillColor\n"
            "RETURN\n"
            "DEFINE CLASS DerivedFillShape AS Shape\n"
            "    FillColor = 7\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native FillColor script should complete: ") + state.message +
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
