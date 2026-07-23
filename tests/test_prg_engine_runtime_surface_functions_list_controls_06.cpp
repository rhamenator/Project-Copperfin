#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_listbox_autohidescrollbar_property_stays_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_listbox_autohidescrollbar";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_listbox_autohidescrollbar.prg";
        write_text(
            main_path,
            "oPlain = CREATEOBJECT('ListBox')\n"
            "oCombo = CREATEOBJECT('ComboBox')\n"
            "lPlainHas = PEMSTATUS(oPlain, 'AutoHideScrollBar', 1)\n"
            "lPlainReadOnly = PEMSTATUS(oPlain, 'AutoHideScrollBar', 5)\n"
            "nPlainBefore = oPlain.AutoHideScrollBar\n"
            "lComboHas = PEMSTATUS(oCombo, 'AutoHideScrollBar', 1)\n"
            "oPlain.AutoHideScrollBar = 1\n"
            "nPlainDirect = oPlain.AutoHideScrollBar\n"
            "lPutPem = PUTPEM(oPlain, 'AutoHideScrollBar', 1)\n"
            "nPlainPutPem = GETPEM(oPlain, 'AutoHideScrollBar')\n"
            "lSetPem = SETPEM(oPlain, 'AutoHideScrollBar', 0)\n"
            "nPlainSetPem = GETPEM(oPlain, 'AutoHideScrollBar')\n"
            "oPlain.AutoHideScrollBar = 2\n"
            "nPlainInvalid = oPlain.AutoHideScrollBar\n"
            "lAddProperty = ADDPROPERTY(oPlain, 'AutoHideScrollBar', 1)\n"
            "lRemoveProperty = REMOVEPROPERTY(oPlain, 'AutoHideScrollBar')\n"
            "nPropMembers = AMEMBERS(aPropMembers, oPlain, 1)\n"
            "lPropHas = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'AUTOHIDESCROLLBAR'\n"
            "        lPropHas = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedAutoHideList')\n"
            "nDerived = oDerived.AutoHideScrollBar\n"
            "RETURN\n"
            "DEFINE CLASS DerivedAutoHideList AS ListBox\n"
            "    PROCEDURE Init\n"
            "        THIS.AutoHideScrollBar = 1\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native AutoHideScrollBar script should complete: ") + state.message +
                   " @line=" + std::to_string(state.location.line));

        const auto check = [&](const std::string &name, const std::string &expected)
        {
            const auto it = state.globals.find(name);
            if (it == state.globals.end())
            {
                expect(false, name + " variable not found");
                return;
            }
            expect(copperfin::runtime::format_value(it->second) == expected,
                   name + " expected '" + expected + "' got '" + copperfin::runtime::format_value(it->second) + "'");
        };

        check("lplainhas", "true");
        check("lplainreadonly", "false");
        check("nplainbefore", "0");
        check("lcombohas", "false");
        check("nplaindirect", "1");
        check("lputpem", "true");
        check("nplainputpem", "1");
        check("lsetpem", "true");
        check("nplainsetpem", "0");
        check("nplaininvalid", "0");
        check("laddproperty", "false");
        check("lremoveproperty", "false");
        check("lprophas", "true");
        check("nderived", "1");
        expect(state.ole_objects.size() == 3U,
               "native AutoHideScrollBar coverage should register plain ListBox, ComboBox, and derived ListBox");

        fs::remove_all(temp_root, ignored);
    }
}
