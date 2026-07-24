#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_visual_autosize_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_visual_autosize";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_visual_autosize.prg";
        write_text(
            main_path,
            "oButton = CREATEOBJECT('CommandButton')\n"
            "oLabel = CREATEOBJECT('Label')\n"
            "oGroup = CREATEOBJECT('OptionGroup')\n"
            "oImage = CREATEOBJECT('Image')\n"
            "oList = CREATEOBJECT('ListBox')\n"
            "lButtonDefault = oButton.AutoSize\n"
            "lLabelDefault = GETPEM(oLabel, 'AutoSize')\n"
            "lButtonHas = PEMSTATUS(oButton, 'AutoSize', 1)\n"
            "lLabelHas = PEMSTATUS(oLabel, 'AutoSize', 1)\n"
            "lGroupHas = PEMSTATUS(oGroup, 'AutoSize', 1)\n"
            "lImageHas = PEMSTATUS(oImage, 'AutoSize', 1)\n"
            "lListHas = PEMSTATUS(oList, 'AutoSize', 1)\n"
            "oButton.AutoSize = .T.\n"
            "lDirect = oButton.AutoSize\n"
            "lSet = SETPEM(oLabel, 'AutoSize', 1)\n"
            "lAfterSet = GETPEM(oLabel, 'AutoSize')\n"
            "lPut = PUTPEM(oGroup, 'AutoSize', 0)\n"
            "lAfterPut = oGroup.AutoSize\n"
            "lAdd = ADDPROPERTY(oButton, 'AutoSize', .F.)\n"
            "lRemove = REMOVEPROPERTY(oButton, 'AutoSize')\n"
            "lMember = .F.\n"
            "nMembers = AMEMBERS(aMembers, oButton, 1)\n"
            "FOR i = 1 TO nMembers\n"
            "    IF UPPER(aMembers[i]) == 'AUTOSIZE'\n"
            "        lMember = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedAutoLabel')\n"
            "lDerived = oDerived.AutoSize\n"
            "RETURN\n"
            "DEFINE CLASS DerivedAutoLabel AS Label\n"
            "    AutoSize = .T.\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native AutoSize script should complete: ") + state.message +
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

        check("lbuttondefault", "false");
        check("llabeldefault", "false");
        check("lbuttonhas", "true");
        check("llabelhas", "true");
        check("lgrouphas", "true");
        check("limagehas", "false");
        check("llisthas", "false");
        check("ldirect", "true");
        check("lset", "true");
        check("lafterset", "1");
        check("lput", "true");
        check("lafterput", "0");
        check("ladd", "false");
        check("lremove", "false");
        check("lmember", "true");
        check("lderived", "true");
        expect(state.ole_objects.size() == 6U,
               "native AutoSize coverage should register five base objects and the derived object");

        fs::remove_all(temp_root, ignored);
    }
}
