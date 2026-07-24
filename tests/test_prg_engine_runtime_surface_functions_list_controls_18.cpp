#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_list_controls_item_colors_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_list_item_colors";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_list_item_colors.prg";
        write_text(
            main_path,
            "oList = CREATEOBJECT('ListBox')\n"
            "oCombo = CREATEOBJECT('ComboBox')\n"
            "lListDefaultBack = oList.ItemBackColor\n"
            "lListDefaultFore = oList.ItemForeColor\n"
            "lComboDefaultBack = GETPEM(oCombo, 'ItemBackColor')\n"
            "lComboDefaultFore = GETPEM(oCombo, 'ItemForeColor')\n"
            "lListHasBack = PEMSTATUS(oList, 'ItemBackColor', 1)\n"
            "lListHasFore = PEMSTATUS(oList, 'ItemForeColor', 1)\n"
            "oList.ItemBackColor = 123.9\n"
            "oList.ItemForeColor = 456.9\n"
            "lListDirectBack = oList.ItemBackColor\n"
            "lListDirectFore = oList.ItemForeColor\n"
            "lSetPem = SETPEM(oList, 'ItemBackColor', 789.8)\n"
            "lAfterSetPem = GETPEM(oList, 'ItemBackColor')\n"
            "lPutPem = PUTPEM(oList, 'ItemForeColor', 987.7)\n"
            "lAfterPutPem = GETPEM(oList, 'ItemForeColor')\n"
            "lAddProperty = ADDPROPERTY(oList, 'ItemBackColor', 1)\n"
            "lRemoveProperty = REMOVEPROPERTY(oList, 'ItemForeColor')\n"
            "nPropMembers = AMEMBERS(aPropMembers, oList, 1)\n"
            "lPropHasBack = .F.\n"
            "lPropHasFore = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'ITEMBACKCOLOR'\n"
            "        lPropHasBack = .T.\n"
            "    ENDIF\n"
            "    IF UPPER(aPropMembers[i]) == 'ITEMFORECOLOR'\n"
            "        lPropHasFore = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedItemColorsList')\n"
            "lDerivedBack = oDerived.ItemBackColor\n"
            "lDerivedFore = oDerived.ItemForeColor\n"
            "oText = CREATEOBJECT('TextBox')\n"
            "lTextHasBack = PEMSTATUS(oText, 'ItemBackColor', 1)\n"
            "RETURN\n"
            "DEFINE CLASS DerivedItemColorsList AS ListBox\n"
            "    PROCEDURE Init\n"
            "        THIS.ItemBackColor = 111.9\n"
            "        THIS.ItemForeColor = 222.9\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native item color script should complete: ") + state.message +
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

        check("llistdefaultback", "16777215");
        check("llistdefaultfore", "0");
        check("lcombodefaultback", "16777215");
        check("lcombodefaultfore", "0");
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
        check("ltexthasback", "false");
        expect(state.ole_objects.size() == 4U,
               "native item color coverage should register ListBox, ComboBox, derived ListBox, and TextBox");

        fs::remove_all(temp_root, ignored);
    }
}
