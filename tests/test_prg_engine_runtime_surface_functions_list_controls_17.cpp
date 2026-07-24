#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_list_controls_disabled_item_colors_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_list_disabled_item_colors";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_list_disabled_item_colors.prg";
        write_text(
            main_path,
            "oList = CREATEOBJECT('ListBox')\n"
            "oCombo = CREATEOBJECT('ComboBox')\n"
            "lListDefaultBack = oList.DisabledItemBackColor\n"
            "lListDefaultFore = oList.DisabledItemForeColor\n"
            "lComboDefaultBack = GETPEM(oCombo, 'DisabledItemBackColor')\n"
            "lComboDefaultFore = GETPEM(oCombo, 'DisabledItemForeColor')\n"
            "lListHasBack = PEMSTATUS(oList, 'DisabledItemBackColor', 1)\n"
            "lListHasFore = PEMSTATUS(oList, 'DisabledItemForeColor', 1)\n"
            "lListReadOnly = PEMSTATUS(oList, 'DisabledItemBackColor', 5)\n"
            "oList.DisabledItemBackColor = 123.9\n"
            "oList.DisabledItemForeColor = 456.9\n"
            "lListDirectBack = oList.DisabledItemBackColor\n"
            "lListDirectFore = oList.DisabledItemForeColor\n"
            "lSetPem = SETPEM(oList, 'DisabledItemBackColor', 789.8)\n"
            "lAfterSetPem = GETPEM(oList, 'DisabledItemBackColor')\n"
            "lPutPem = PUTPEM(oList, 'DisabledItemForeColor', 987.7)\n"
            "lAfterPutPem = GETPEM(oList, 'DisabledItemForeColor')\n"
            "lAddProperty = ADDPROPERTY(oList, 'DisabledItemBackColor', 1)\n"
            "lRemoveProperty = REMOVEPROPERTY(oList, 'DisabledItemForeColor')\n"
            "nPropMembers = AMEMBERS(aPropMembers, oList, 1)\n"
            "lPropHasBack = .F.\n"
            "lPropHasFore = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'DISABLEDITEMBACKCOLOR'\n"
            "        lPropHasBack = .T.\n"
            "    ENDIF\n"
            "    IF UPPER(aPropMembers[i]) == 'DISABLEDITEMFORECOLOR'\n"
            "        lPropHasFore = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedDisabledColorsList')\n"
            "lDerivedBack = oDerived.DisabledItemBackColor\n"
            "lDerivedFore = oDerived.DisabledItemForeColor\n"
            "oText = CREATEOBJECT('TextBox')\n"
            "lTextHasBack = PEMSTATUS(oText, 'DisabledItemBackColor', 1)\n"
            "RETURN\n"
            "DEFINE CLASS DerivedDisabledColorsList AS ListBox\n"
            "    PROCEDURE Init\n"
            "        THIS.DisabledItemBackColor = 111.9\n"
            "        THIS.DisabledItemForeColor = 222.9\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native disabled item color script should complete: ") + state.message +
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

        check("llistdefaultback", "12632256");
        check("llistdefaultfore", "8421504");
        check("lcombodefaultback", "12632256");
        check("lcombodefaultfore", "8421504");
        check("llisthasback", "true");
        check("llisthasfore", "true");
        check("llistreadonly", "false");
        check("llistdirectback", "123");
        check("llistdirectfore", "456");
        check("lsetpem", "true");
        check("laftersetpem", "789");
        check("lputpem", "true");
        check("lafterputpem", "987");
        check("laddproperty", "false");
        check("lremoveproperty", "false");
        check("lprophasback", "true");
        check("lprophasfore", "true");
        check("lderivedback", "111");
        check("lderivedfore", "222");
        check("ltexthasback", "false");
        expect(state.ole_objects.size() == 4U,
               "native disabled item color coverage should register ListBox, ComboBox, derived ListBox, and TextBox");

        fs::remove_all(temp_root, ignored);
    }
}
