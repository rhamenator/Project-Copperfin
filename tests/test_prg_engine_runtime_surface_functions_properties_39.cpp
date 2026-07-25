#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_visual_anchor_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_visual_anchor";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_visual_anchor.prg";
        write_text(
            main_path,
            "oButton = CREATEOBJECT('CommandButton')\n"
            "oTextBox = CREATEOBJECT('TextBox')\n"
            "oContainer = CREATEOBJECT('Container')\n"
            "oForm = CREATEOBJECT('Form')\n"
            "oToolbar = CREATEOBJECT('ToolBar')\n"
            "oCustom = CREATEOBJECT('Custom')\n"
            "nDefault = oButton.Anchor\n"
            "nGetPem = GETPEM(oButton, 'Anchor')\n"
            "lHas = PEMSTATUS(oButton, 'Anchor', 1)\n"
            "lReadOnly = PEMSTATUS(oButton, 'Anchor', 5)\n"
            "oButton.Anchor = 10\n"
            "nDirect = oButton.Anchor\n"
            "lSetPem = SETPEM(oButton, 'Anchor', 260)\n"
            "nSetPem = GETPEM(oButton, 'Anchor')\n"
            "lPutPem = PUTPEM(oButton, 'Anchor', 17)\n"
            "nConflict = oButton.Anchor\n"
            "oButton.Anchor = 1024\n"
            "nOutOfRange = oButton.Anchor\n"
            "lAddProperty = ADDPROPERTY(oButton, 'Anchor', 1)\n"
            "lRemoveProperty = REMOVEPROPERTY(oButton, 'Anchor')\n"
            "lTextBoxHas = PEMSTATUS(oTextBox, 'Anchor', 1)\n"
            "lContainerHas = PEMSTATUS(oContainer, 'Anchor', 1)\n"
            "lFormHas = PEMSTATUS(oForm, 'Anchor', 1)\n"
            "lToolbarHas = PEMSTATUS(oToolbar, 'Anchor', 1)\n"
            "lCustomHas = PEMSTATUS(oCustom, 'Anchor', 1)\n"
            "oDerived = CREATEOBJECT('DerivedAnchorButton')\n"
            "nDerived = oDerived.Anchor\n"
            "lMember = .F.\n"
            "nMembers = AMEMBERS(aMembers, oDerived, 1)\n"
            "FOR i = 1 TO nMembers\n"
            "    IF UPPER(aMembers[i]) == 'ANCHOR'\n"
            "        lMember = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "RETURN\n"
            "DEFINE CLASS DerivedAnchorButton AS CommandButton\n"
            "    Anchor = 12\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Anchor property script should complete: ") + state.message +
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

        check("ndefault", "0");
        check("ngetpem", "0");
        check("lhas", "true");
        check("lreadonly", "false");
        check("ndirect", "10");
        check("lsetpem", "true");
        check("nsetpem", "260");
        check("lputpem", "true");
        check("nconflict", "0");
        check("noutofrange", "0");
        check("laddproperty", "false");
        check("lremoveproperty", "false");
        check("ltextboxhas", "true");
        check("lcontainerhas", "true");
        check("lformhas", "false");
        check("ltoolbarhas", "false");
        check("lcustomhas", "false");
        check("nderived", "12");
        check("lmember", "true");
        expect(state.ole_objects.size() == 7U,
               "native Anchor coverage should register the supported controls and exclusions");

        fs::remove_all(temp_root, ignored);
    }
}
