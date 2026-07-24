#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_visual_drag_properties_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_visual_drag";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_visual_drag.prg";
        write_text(
            main_path,
            "oList = CREATEOBJECT('ListBox')\n"
            "oButton = CREATEOBJECT('CommandButton')\n"
            "oCustom = CREATEOBJECT('Custom')\n"
            "nListDefault = oList.DragMode\n"
            "cListIconDefault = GETPEM(oList, 'DragIcon')\n"
            "lListHasMode = PEMSTATUS(oList, 'DragMode', 1)\n"
            "lListHasIcon = PEMSTATUS(oList, 'DragIcon', 1)\n"
            "lButtonHasMode = PEMSTATUS(oButton, 'DragMode', 1)\n"
            "lButtonHasIcon = PEMSTATUS(oButton, 'DragIcon', 1)\n"
            "lCustomHasMode = PEMSTATUS(oCustom, 'DragMode', 1)\n"
            "lCustomHasIcon = PEMSTATUS(oCustom, 'DragIcon', 1)\n"
            "oList.DragMode = 2\n"
            "nDirectMode = oList.DragMode\n"
            "oList.DragIcon = 'icons\\can-drop.cur'\n"
            "cDirectIcon = oList.DragIcon\n"
            "lSetMode = SETPEM(oList, 'DragMode', 3)\n"
            "nSetMode = GETPEM(oList, 'DragMode')\n"
            "lPutIcon = PUTPEM(oButton, 'DragIcon', 'icons\\button.cur')\n"
            "cPutIcon = GETPEM(oButton, 'DragIcon')\n"
            "lSetNegative = SETPEM(oList, 'DragMode', -1)\n"
            "nNormalized = oList.DragMode\n"
            "lAddMode = ADDPROPERTY(oList, 'DragMode', 1)\n"
            "lRemoveIcon = REMOVEPROPERTY(oList, 'DragIcon')\n"
            "lModeMember = .F.\n"
            "lIconMember = .F.\n"
            "nMembers = AMEMBERS(aMembers, oList, 1)\n"
            "FOR i = 1 TO nMembers\n"
            "    IF UPPER(aMembers[i]) == 'DRAGMODE'\n"
            "        lModeMember = .T.\n"
            "    ENDIF\n"
            "    IF UPPER(aMembers[i]) == 'DRAGICON'\n"
            "        lIconMember = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedDragList')\n"
            "nDerivedMode = oDerived.DragMode\n"
            "cDerivedIcon = oDerived.DragIcon\n"
            "RETURN\n"
            "DEFINE CLASS DerivedDragList AS ListBox\n"
            "    DragMode = 1\n"
            "    DragIcon = 'icons\\class.cur'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native drag property script should complete: ") + state.message +
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

        check("nlistdefault", "0");
        check("clisticondefault", "");
        check("llisthasmode", "true");
        check("llisthasicon", "true");
        check("lbuttonhasmode", "true");
        check("lbuttonhasicon", "true");
        check("lcustomhasmode", "false");
        check("lcustomhasicon", "false");
        check("ndirectmode", "2");
        check("cdirecticon", "icons\\can-drop.cur");
        check("lsetmode", "true");
        check("nsetmode", "3");
        check("lputicon", "true");
        check("cputicon", "icons\\button.cur");
        check("lsetnegative", "true");
        check("nnormalized", "0");
        check("laddmode", "false");
        check("lremoveicon", "false");
        check("lmodemember", "true");
        check("liconmember", "true");
        check("nderivedmode", "1");
        check("cderivedicon", "icons\\class.cur");
        expect(state.ole_objects.size() == 4U,
               "native drag coverage should register the list, button, custom, and derived objects");

        fs::remove_all(temp_root, ignored);
    }
}
