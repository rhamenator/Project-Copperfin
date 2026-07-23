#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_combobox_displaycount_property_stays_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_combobox_displaycount";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_combobox_displaycount.prg";
        write_text(
            main_path,
            "oCombo = CREATEOBJECT('ComboBox')\n"
            "oList = CREATEOBJECT('ListBox')\n"
            "nDefault = oCombo.DisplayCount\n"
            "lComboHas = PEMSTATUS(oCombo, 'DisplayCount', 1)\n"
            "lListHas = PEMSTATUS(oList, 'DisplayCount', 1)\n"
            "lReadOnly = PEMSTATUS(oCombo, 'DisplayCount', 5)\n"
            "oCombo.DisplayCount = 4\n"
            "nDirect = oCombo.DisplayCount\n"
            "lSetPem = SETPEM(oCombo, 'DisplayCount', 2)\n"
            "nSetPem = GETPEM(oCombo, 'DisplayCount')\n"
            "lPutPem = PUTPEM(oCombo, 'DisplayCount', 3)\n"
            "nPutPem = GETPEM(oCombo, 'DisplayCount')\n"
            "oCombo.DisplayCount = -1\n"
            "nInvalid = oCombo.DisplayCount\n"
            "lAddProperty = ADDPROPERTY(oCombo, 'DisplayCount', 5)\n"
            "lRemoveProperty = REMOVEPROPERTY(oCombo, 'DisplayCount')\n"
            "nPropMembers = AMEMBERS(aPropMembers, oCombo, 1)\n"
            "lPropHas = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'DISPLAYCOUNT'\n"
            "        lPropHas = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedDisplayCombo')\n"
            "nDerived = oDerived.DisplayCount\n"
            "RETURN\n"
            "DEFINE CLASS DerivedDisplayCombo AS ComboBox\n"
            "    PROCEDURE Init\n"
            "        THIS.DisplayCount = 7\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native DisplayCount script should complete: ") + state.message +
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
        check("lcombohas", "true");
        check("llisthas", "false");
        check("lreadonly", "false");
        check("ndirect", "4");
        check("lsetpem", "true");
        check("nsetpem", "2");
        check("lputpem", "true");
        check("nputpem", "3");
        check("ninvalid", "0");
        check("laddproperty", "false");
        check("lremoveproperty", "false");
        check("lprophas", "true");
        check("nderived", "7");
        expect(state.ole_objects.size() == 3U,
               "native DisplayCount coverage should register ComboBox, ListBox, and derived ComboBox");

        fs::remove_all(temp_root, ignored);
    }
}
