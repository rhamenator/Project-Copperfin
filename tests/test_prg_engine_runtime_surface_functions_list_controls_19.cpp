#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_list_controls_selected_item_colors_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_list_selected_item_colors";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_list_selected_item_colors.prg";
        write_text(
            main_path,
            "oList = CREATEOBJECT('ListBox')\n"
            "oCombo = CREATEOBJECT('ComboBox')\n"
            "lListDefaultBack = oList.SelectedItemBackColor\n"
            "lListDefaultFore = oList.SelectedItemForeColor\n"
            "lComboDefaultBack = GETPEM(oCombo, 'SelectedItemBackColor')\n"
            "lComboDefaultFore = GETPEM(oCombo, 'SelectedItemForeColor')\n"
            "lListHasBack = PEMSTATUS(oList, 'SelectedItemBackColor', 1)\n"
            "lListHasFore = PEMSTATUS(oList, 'SelectedItemForeColor', 1)\n"
            "oList.SelectedItemBackColor = 123.9\n"
            "oList.SelectedItemForeColor = 456.9\n"
            "lListDirectBack = oList.SelectedItemBackColor\n"
            "lListDirectFore = oList.SelectedItemForeColor\n"
            "lSetPem = SETPEM(oList, 'SelectedItemBackColor', 789.8)\n"
            "lAfterSetPem = GETPEM(oList, 'SelectedItemBackColor')\n"
            "lPutPem = PUTPEM(oList, 'SelectedItemForeColor', 987.7)\n"
            "lAfterPutPem = GETPEM(oList, 'SelectedItemForeColor')\n"
            "lAddProperty = ADDPROPERTY(oList, 'SelectedItemBackColor', 1)\n"
            "lRemoveProperty = REMOVEPROPERTY(oList, 'SelectedItemForeColor')\n"
            "nPropMembers = AMEMBERS(aPropMembers, oList, 1)\n"
            "lPropHasBack = .F.\n"
            "lPropHasFore = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'SELECTEDITEMBACKCOLOR'\n"
            "        lPropHasBack = .T.\n"
            "    ENDIF\n"
            "    IF UPPER(aPropMembers[i]) == 'SELECTEDITEMFORECOLOR'\n"
            "        lPropHasFore = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedSelectedItemColorsList')\n"
            "lDerivedBack = oDerived.SelectedItemBackColor\n"
            "lDerivedFore = oDerived.SelectedItemForeColor\n"
            "RETURN\n"
            "DEFINE CLASS DerivedSelectedItemColorsList AS ListBox\n"
            "    PROCEDURE Init\n"
            "        THIS.SelectedItemBackColor = 111.9\n"
            "        THIS.SelectedItemForeColor = 222.9\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native selected item color script should complete: ") + state.message +
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

        check("llistdefaultback", "8421504");
        check("llistdefaultfore", "16777215");
        check("lcombodefaultback", "8421504");
        check("lcombodefaultfore", "16777215");
        check("llisthasback", "true");
        check("llisthasfore", "true");
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

        fs::remove_all(temp_root, ignored);
    }
}
