#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_list_controls_additem_builtin_populates_runtime_items()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_list_controls_additem";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_list_controls_additem.prg";
        write_text(
            main_path,
            "oPlainCombo = CREATEOBJECT('ComboBox')\n"
            "lPlainHasAddItem = PEMSTATUS(oPlainCombo, 'AddItem', 1)\n"
            "lPlainGetAddItem = GETPEM(oPlainCombo, 'AddItem')\n"
            "nPlainMethodCount = AMEMBERS(aPlainMethods, oPlainCombo, 2)\n"
            "nPlainHasAddItem = ASCAN(aPlainMethods, 'ADDITEM')\n"
            "nPlainInsert1 = oPlainCombo.AddItem('March')\n"
            "nPlainInsert2 = oPlainCombo.AddItem('April')\n"
            "nPlainInsertBefore = oPlainCombo.AddItem('February', 1)\n"
            "oPlainCombo.ListIndex = 2\n"
            "cPlainSelected = oPlainCombo.DisplayValue\n"
            "oPlainList = CREATEOBJECT('ListBox')\n"
            "nListInsert1 = oPlainList.AddItem('2024')\n"
            "nListInsert2 = oPlainList.AddItem('2026')\n"
            "nListInsertMiddle = oPlainList.AddItem('2025', 2)\n"
            "oPlainList.ListIndex = 2\n"
            "cListSelected = oPlainList.DisplayValue\n"
            "oSeedCombo = CREATEOBJECT('SeededCombo')\n"
            "oSeedList = CREATEOBJECT('SeededList')\n"
            "cSeedComboSelected = oSeedCombo.DisplayValue\n"
            "cSeedListSelected = oSeedList.DisplayValue\n"
            "RETURN\n"
            "DEFINE CLASS SeededCombo AS ComboBox\n"
            "    PROCEDURE Init\n"
            "        THIS.AddItem('Alpha')\n"
            "        THIS.AddItem('Gamma')\n"
            "        THIS.AddItem('Beta', 2)\n"
            "        THIS.ListIndex = 2\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SeededList AS ListBox\n"
            "    PROCEDURE Init\n"
            "        THIS.AddItem('North')\n"
            "        THIS.AddItem('South')\n"
            "        THIS.ListIndex = 2\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native AddItem list-control script should complete: ") + state.message +
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

        check("lplainhasadditem", "true");
        check("lplaingetadditem", "true");
        check("nplaininsert1", "1");
        check("nplaininsert2", "2");
        check("nplaininsertbefore", "1");
        check("cplainselected", "March");
        check("nlistinsert1", "1");
        check("nlistinsert2", "2");
        check("nlistinsertmiddle", "2");
        check("clistselected", "2025");
        check("cseedcomboselected", "Beta");
        check("cseedlistselected", "South");

        const auto plain_has_additem = state.globals.find("nplainhasadditem");
        expect(plain_has_additem != state.globals.end(),
               "nPlainHasAddItem variable should be present");
        if (plain_has_additem != state.globals.end())
        {
            expect(copperfin::runtime::format_value(plain_has_additem->second) != "0",
                   "AMEMBERS(..., 2) should expose the native AddItem builtin for ComboBox");
        }

        expect(state.ole_objects.size() == 4U,
               "native AddItem list-control script should register plain and derived combo/list objects");
        if (state.ole_objects.size() == 4U)
        {
            const auto &plain_combo = state.ole_objects[0];
            const auto &plain_list = state.ole_objects[1];
            const auto &seed_combo = state.ole_objects[2];
            const auto &seed_list = state.ole_objects[3];

            const auto expect_items = [&](const copperfin::runtime::RuntimeOleObjectState &runtime_object,
                                          const std::vector<std::string> &expected_items,
                                          const std::string &message_prefix)
            {
                expect(runtime_object.collection_items.size() == expected_items.size(),
                       message_prefix + " should preserve the expected AddItem row count");
                if (runtime_object.collection_items.size() != expected_items.size())
                {
                    return;
                }

                for (std::size_t index = 0; index < expected_items.size(); ++index)
                {
                    expect(copperfin::runtime::format_value(runtime_object.collection_items[index]) == expected_items[index],
                           message_prefix + " item " + std::to_string(index + 1U) + " expected '" +
                               expected_items[index] + "'");
                }
            };

            expect(plain_combo.prog_id == "ComboBox",
                   "plain ComboBox AddItem coverage should preserve the built-in class identity");
            expect(plain_list.prog_id == "ListBox",
                   "plain ListBox AddItem coverage should preserve the built-in class identity");
            expect(seed_combo.prog_id == "SeededCombo",
                   "derived ComboBox AddItem coverage should preserve the PRG class identity");
            expect(seed_list.prog_id == "SeededList",
                   "derived ListBox AddItem coverage should preserve the PRG class identity");

            expect_items(plain_combo, {"February", "March", "April"},
                         "plain ComboBox AddItem coverage");
            expect_items(plain_list, {"2024", "2025", "2026"},
                         "plain ListBox AddItem coverage");
            expect_items(seed_combo, {"Alpha", "Beta", "Gamma"},
                         "derived ComboBox AddItem coverage");
            expect_items(seed_list, {"North", "South"},
                         "derived ListBox AddItem coverage");

            const auto plain_combo_listindex = plain_combo.properties.find("listindex");
            const auto plain_combo_display = plain_combo.properties.find("displayvalue");
            const auto plain_list_display = plain_list.properties.find("displayvalue");
            const auto seed_combo_display = seed_combo.properties.find("displayvalue");
            const auto seed_list_display = seed_list.properties.find("displayvalue");
            expect(plain_combo_listindex != plain_combo.properties.end() &&
                       copperfin::runtime::format_value(plain_combo_listindex->second) == "2",
                   "plain ComboBox AddItem coverage should preserve the selected ListIndex");
            expect(plain_combo_display != plain_combo.properties.end() &&
                       copperfin::runtime::format_value(plain_combo_display->second) == "March",
                   "plain ComboBox AddItem coverage should synchronize DisplayValue from the selected item");
            expect(plain_list_display != plain_list.properties.end() &&
                       copperfin::runtime::format_value(plain_list_display->second) == "2025",
                   "plain ListBox AddItem coverage should synchronize DisplayValue from the selected item");
            expect(seed_combo_display != seed_combo.properties.end() &&
                       copperfin::runtime::format_value(seed_combo_display->second) == "Beta",
                   "derived ComboBox AddItem coverage should preserve Init-time selected display text");
            expect(seed_list_display != seed_list.properties.end() &&
                       copperfin::runtime::format_value(seed_list_display->second) == "South",
                   "derived ListBox AddItem coverage should preserve Init-time selected display text");
        }

        const bool has_additem_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.additem";
        });
        expect(has_additem_event,
               "native AddItem coverage should emit representative list-control method events");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_list_controls_clear_builtin_clears_rows_and_honors_rowsourcetype_gate()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_list_controls_clear";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_list_controls_clear.prg";
        write_text(
            main_path,
            "oPlainCombo = CREATEOBJECT('ComboBox')\n"
            "lPlainHasClear = PEMSTATUS(oPlainCombo, 'Clear', 1)\n"
            "lPlainGetClear = GETPEM(oPlainCombo, 'Clear')\n"
            "nPlainMethodCount = AMEMBERS(aPlainMethods, oPlainCombo, 2)\n"
            "nPlainHasClear = ASCAN(aPlainMethods, 'CLEAR')\n"
            "oPlainCombo.AddItem('Alpha')\n"
            "oPlainCombo.AddItem('Beta')\n"
            "oPlainCombo.ListIndex = 2\n"
            "oPlainCombo.Clear()\n"
            "nPlainComboCountAfterClear = oPlainCombo.ListCount\n"
            "nPlainComboIndexAfterClear = oPlainCombo.ListIndex\n"
            "nPlainComboItemIdAfterClear = oPlainCombo.ListItemID\n"
            "nPlainComboNewIndexAfterClear = oPlainCombo.NewIndex\n"
            "nPlainComboNewItemIdAfterClear = oPlainCombo.NewItemId\n"
            "cPlainComboDisplayAfterClear = oPlainCombo.DisplayValue\n"
            "cPlainComboValueAfterClear = oPlainCombo.Value\n"
            "oPlainList = CREATEOBJECT('ListBox')\n"
            "oPlainList.MultiSelect = .T.\n"
            "oPlainList.AddItem('North')\n"
            "oPlainList.AddItem('South')\n"
            "oPlainList.Selected(1) = .T.\n"
            "oPlainList.Selected(2) = .T.\n"
            "oPlainList.Clear()\n"
            "nPlainListCountAfterClear = oPlainList.ListCount\n"
            "nPlainListIndexAfterClear = oPlainList.ListIndex\n"
            "nPlainListItemIdAfterClear = oPlainList.ListItemID\n"
            "cPlainListDisplayAfterClear = oPlainList.DisplayValue\n"
            "cPlainListValueAfterClear = oPlainList.Value\n"
            "lPlainListSelected1AfterClear = oPlainList.Selected(1)\n"
            "oValueCombo = CREATEOBJECT('ComboBox')\n"
            "oValueCombo.RowSourceType = 1\n"
            "oValueCombo.AddItem('March')\n"
            "oValueCombo.AddItem('April')\n"
            "oValueCombo.ListIndex = 2\n"
            "oValueCombo.Clear()\n"
            "nValueComboCountAfterClear = oValueCombo.ListCount\n"
            "nValueComboIndexAfterClear = oValueCombo.ListIndex\n"
            "cValueComboDisplayAfterClear = oValueCombo.DisplayValue\n"
            "oSeedCombo = CREATEOBJECT('SeededCombo')\n"
            "oSeedList = CREATEOBJECT('SeededList')\n"
            "nSeedComboCount = oSeedCombo.ListCount\n"
            "nSeedListCount = oSeedList.ListCount\n"
            "cSeedComboDisplay = oSeedCombo.DisplayValue\n"
            "cSeedListDisplay = oSeedList.DisplayValue\n"
            "RETURN\n"
            "DEFINE CLASS SeededCombo AS ComboBox\n"
            "    PROCEDURE Init\n"
            "        THIS.AddItem('One')\n"
            "        THIS.AddItem('Two')\n"
            "        THIS.ListIndex = 2\n"
            "        THIS.Clear()\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SeededList AS ListBox\n"
            "    MultiSelect = .T.\n"
            "    PROCEDURE Init\n"
            "        THIS.AddItem('East')\n"
            "        THIS.AddItem('West')\n"
            "        THIS.Selected(2) = .T.\n"
            "        THIS.Clear()\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Clear list-control script should complete: ") + state.message +
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

        check("lplainhasclear", "true");
        check("lplaingetclear", "true");
        check("nplaincombocountafterclear", "0");
        check("nplaincomboindexafterclear", "0");
        check("nplaincomboitemidafterclear", "0");
        check("nplaincombonewindexafterclear", "0");
        check("nplaincombonewitemidafterclear", "0");
        check("cplaincombodisplayafterclear", "");
        check("cplaincombovalueafterclear", "");
        check("nplainlistcountafterclear", "0");
        check("nplainlistindexafterclear", "0");
        check("nplainlistitemidafterclear", "0");
        check("cplainlistdisplayafterclear", "");
        check("cplainlistvalueafterclear", "");
        check("lplainlistselected1afterclear", "false");
        check("nvaluecombocountafterclear", "2");
        check("nvaluecomboindexafterclear", "2");
        check("cvaluecombodisplayafterclear", "April");
        check("nseedcombocount", "0");
        check("nseedlistcount", "0");
        check("cseedcombodisplay", "");
        check("cseedlistdisplay", "");

        const auto plain_has_clear = state.globals.find("nplainhasclear");
        expect(plain_has_clear != state.globals.end(),
               "nPlainHasClear variable should be present");
        if (plain_has_clear != state.globals.end())
        {
            expect(copperfin::runtime::format_value(plain_has_clear->second) != "0",
                   "AMEMBERS(..., 2) should expose the native Clear builtin for ComboBox");
        }

        expect(state.ole_objects.size() == 5U,
               "native Clear coverage should register plain, gated, and derived combo/list objects");
        if (state.ole_objects.size() == 5U)
        {
            const auto &plain_combo = state.ole_objects[0];
            const auto &plain_list = state.ole_objects[1];
            const auto &value_combo = state.ole_objects[2];
            const auto &seed_combo = state.ole_objects[3];
            const auto &seed_list = state.ole_objects[4];

            expect(plain_combo.collection_items.empty(),
                   "plain ComboBox Clear coverage should remove all runtime items");
            expect(plain_list.collection_items.empty(),
                   "plain ListBox Clear coverage should remove all runtime items");
            expect(seed_combo.collection_items.empty(),
                   "derived ComboBox Clear coverage should preserve Init-time clear");
            expect(seed_list.collection_items.empty(),
                   "derived ListBox Clear coverage should preserve Init-time clear");

            expect(value_combo.collection_items.size() == 2U,
                   "RowSourceType 1 Clear coverage should leave existing runtime items unchanged");
            if (value_combo.collection_items.size() == 2U)
            {
                expect(copperfin::runtime::format_value(value_combo.collection_items[0]) == "March",
                       "RowSourceType 1 Clear coverage should preserve the first item");
                expect(copperfin::runtime::format_value(value_combo.collection_items[1]) == "April",
                       "RowSourceType 1 Clear coverage should preserve the second item");
            }

            const auto plain_combo_listcount = plain_combo.properties.find("listcount");
            const auto plain_combo_listindex = plain_combo.properties.find("listindex");
            const auto plain_combo_display = plain_combo.properties.find("displayvalue");
            const auto plain_combo_newindex = plain_combo.properties.find("newindex");
            const auto plain_combo_newitemid = plain_combo.properties.find("newitemid");
            expect(plain_combo_listcount != plain_combo.properties.end() &&
                       copperfin::runtime::format_value(plain_combo_listcount->second) == "0",
                   "plain ComboBox Clear coverage should keep ListCount synchronized");
            expect(plain_combo_listindex != plain_combo.properties.end() &&
                       copperfin::runtime::format_value(plain_combo_listindex->second) == "0",
                   "plain ComboBox Clear coverage should clear ListIndex");
            expect(plain_combo_display != plain_combo.properties.end() &&
                       copperfin::runtime::format_value(plain_combo_display->second).empty(),
                   "plain ComboBox Clear coverage should clear DisplayValue");
            expect(plain_combo_newindex != plain_combo.properties.end() &&
                       copperfin::runtime::format_value(plain_combo_newindex->second) == "0",
                   "plain ComboBox Clear coverage should reset NewIndex");
            expect(plain_combo_newitemid != plain_combo.properties.end() &&
                       copperfin::runtime::format_value(plain_combo_newitemid->second) == "0",
                   "plain ComboBox Clear coverage should reset NewItemId");
        }

        const bool has_clear_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.clear";
        });
        expect(has_clear_event,
               "native Clear coverage should emit representative list-control method events");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_list_controls_multicolumn_additem_followthrough_stays_coherent()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_list_controls_additem_multicolumn";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_list_controls_additem_multicolumn.prg";
        write_text(
            main_path,
            "oPlainCombo = CREATEOBJECT('ComboBox')\n"
            "oPlainCombo.ColumnCount = 3\n"
            "nInsertCol2 = oPlainCombo.AddItem('Ohio', 1, 2)\n"
            "nNewIndexAfterCol2 = oPlainCombo.NewIndex\n"
            "nInsertCol3 = oPlainCombo.AddItem('44122', 1, 3)\n"
            "nNewIndexAfterCol3 = oPlainCombo.NewIndex\n"
            "nInsertCol1 = oPlainCombo.AddItem('Cleveland', 2, 1)\n"
            "nNewIndexAfterCol1 = oPlainCombo.NewIndex\n"
            "nCountAfterAdds = oPlainCombo.ListCount\n"
            "cRow1Col1 = oPlainCombo.List(1)\n"
            "cRow1Col2 = oPlainCombo.List(1, 2)\n"
            "cRow1Col3 = oPlainCombo.List(1, 3)\n"
            "cRow2Col1 = oPlainCombo.List(2)\n"
            "cRow2Col2 = oPlainCombo.List(2, 2)\n"
            "cRow2Col3 = oPlainCombo.List(2, 3)\n"
            "cRow3Col1 = oPlainCombo.List(3)\n"
            "cRow3Col2 = oPlainCombo.List(3, 2)\n"
            "cRow3Col3 = oPlainCombo.List(3, 3)\n"
            "oPlainCombo.RemoveItem(1)\n"
            "nNewIndexAfterRemove = oPlainCombo.NewIndex\n"
            "nCountAfterRemove = oPlainCombo.ListCount\n"
            "cRemain1Col1 = oPlainCombo.List(1)\n"
            "cRemain1Col2 = oPlainCombo.List(1, 2)\n"
            "cRemain2Col1 = oPlainCombo.List(2)\n"
            "cRemain2Col2 = oPlainCombo.List(2, 2)\n"
            "oSeedList = CREATEOBJECT('SeededList')\n"
            "nSeedCount = oSeedList.ListCount\n"
            "nSeedNewIndex = oSeedList.NewIndex\n"
            "cSeedRow1Col1 = oSeedList.List(1)\n"
            "cSeedRow1Col2 = oSeedList.List(1, 2)\n"
            "cSeedRow2Col1 = oSeedList.List(2)\n"
            "cSeedRow2Col2 = oSeedList.List(2, 2)\n"
            "RETURN\n"
            "DEFINE CLASS SeededList AS ListBox\n"
            "    ColumnCount = 2\n"
            "    PROCEDURE Init\n"
            "        THIS.AddItem('East', 1, 2)\n"
            "        THIS.AddItem('North', 1, 1)\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native multicolumn AddItem list-control script should complete: ") + state.message +
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

        check("ninsertcol2", "1");
        check("nnewindexaftercol2", "1");
        check("ninsertcol3", "1");
        check("nnewindexaftercol3", "1");
        check("ninsertcol1", "2");
        check("nnewindexaftercol1", "2");
        check("ncountafteradds", "3");
        check("crow1col1", "");
        check("crow1col2", "");
        check("crow1col3", "44122");
        check("crow2col1", "Cleveland");
        check("crow2col2", "");
        check("crow2col3", "");
        check("crow3col1", "");
        check("crow3col2", "Ohio");
        check("crow3col3", "");
        check("nnewindexafterremove", "1");
        check("ncountafterremove", "2");
        check("cremain1col1", "Cleveland");
        check("cremain1col2", "");
        check("cremain2col1", "");
        check("cremain2col2", "Ohio");
        check("nseedcount", "2");
        check("nseednewindex", "1");
        check("cseedrow1col1", "North");
        check("cseedrow1col2", "");
        check("cseedrow2col1", "");
        check("cseedrow2col2", "East");

        expect(state.ole_objects.size() == 2U,
               "native multicolumn AddItem coverage should register plain and derived list controls");
        if (state.ole_objects.size() == 2U)
        {
            const auto &plain_combo = state.ole_objects[0];
            const auto &seed_list = state.ole_objects[1];

            const auto plain_newindex = plain_combo.properties.find("newindex");
            const auto seed_newindex = seed_list.properties.find("newindex");

            expect(plain_combo.list_rows.size() == 2U,
                   "plain ComboBox multicolumn AddItem coverage should preserve the expected remaining row count");
            if (plain_combo.list_rows.size() == 2U)
            {
                expect(plain_combo.list_rows[0].size() >= 1U &&
                           copperfin::runtime::format_value(plain_combo.list_rows[0][0]) == "Cleveland",
                       "plain ComboBox multicolumn AddItem coverage should keep the surviving first-column row");
                expect(plain_combo.list_rows[1].size() >= 2U &&
                           copperfin::runtime::format_value(plain_combo.list_rows[1][1]) == "Ohio",
                       "plain ComboBox multicolumn AddItem coverage should keep the surviving second-column row");
            }
            expect(plain_newindex != plain_combo.properties.end() &&
                       copperfin::runtime::format_value(plain_newindex->second) == "1",
                   "plain ComboBox multicolumn AddItem coverage should keep NewIndex synchronized after row removal");
            expect(seed_list.list_rows.size() == 2U,
                   "derived ListBox multicolumn AddItem coverage should preserve Init-time multicolumn rows");
            expect(seed_newindex != seed_list.properties.end() &&
                       copperfin::runtime::format_value(seed_newindex->second) == "1",
                   "derived ListBox multicolumn AddItem coverage should keep NewIndex synchronized during Init");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_native_list_controls_writable_list_cells_stay_coherent()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_list_controls_writable_list_cells";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_list_controls_writable_list_cells.prg";
        write_text(
            main_path,
            "oPlainCombo = CREATEOBJECT('ComboBox')\n"
            "oPlainCombo.ColumnCount = 3\n"
            "oPlainCombo.AddItem('Alpha')\n"
            "oPlainCombo.List(oPlainCombo.NewIndex, 2) = 'A'\n"
            "oPlainCombo.List(oPlainCombo.NewIndex, 3) = '111'\n"
            "oPlainCombo.AddItem('Beta')\n"
            "oPlainCombo.List(oPlainCombo.NewIndex, 2) = 'B'\n"
            "oPlainCombo.List(oPlainCombo.NewIndex, 3) = '222'\n"
            "oPlainCombo.ListIndex = 2\n"
            "oPlainCombo.List(oPlainCombo.ListIndex) = 'Beta Prime'\n"
            "lSetPemCol2 = SETPEM(oPlainCombo, 'List(2,2)', 'Bee')\n"
            "xGetPemCol2 = GETPEM(oPlainCombo, 'List(2,2)')\n"
            "lAddPropertyListCell = ADDPROPERTY(oPlainCombo, 'List(2,2)', 'shadow')\n"
            "lRemovePropertyListCell = REMOVEPROPERTY(oPlainCombo, 'List(2,2)')\n"
            "nCountAfterWrites = oPlainCombo.ListCount\n"
            "nNewIndexAfterWrites = oPlainCombo.NewIndex\n"
            "cDisplayAfterWrites = oPlainCombo.DisplayValue\n"
            "cRow1Col1 = oPlainCombo.List(1)\n"
            "cRow1Col2 = oPlainCombo.List(1, 2)\n"
            "cRow1Col3 = oPlainCombo.List(1, 3)\n"
            "cRow2Col1 = oPlainCombo.List(2)\n"
            "cRow2Col2 = oPlainCombo.List(2, 2)\n"
            "cRow2Col3 = oPlainCombo.List(2, 3)\n"
            "oSeedList = CREATEOBJECT('SeededList')\n"
            "nSeedNewIndex = oSeedList.NewIndex\n"
            "cSeedRow1Col1 = oSeedList.List(1)\n"
            "cSeedRow1Col2 = oSeedList.List(1, 2)\n"
            "cSeedRow2Col1 = oSeedList.List(2)\n"
            "cSeedRow2Col2 = oSeedList.List(2, 2)\n"
            "RETURN\n"
            "DEFINE CLASS SeededList AS ListBox\n"
            "    ColumnCount = 2\n"
            "    PROCEDURE Init\n"
            "        THIS.AddItem('North')\n"
            "        THIS.List(THIS.NewIndex, 2) = 'N'\n"
            "        THIS.AddItem('South')\n"
            "        THIS.List(THIS.NewIndex, 2) = 'S'\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native writable List() list-control script should complete: ") + state.message +
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

        check("lsetpemcol2", "true");
        check("xgetpemcol2", "Bee");
        check("laddpropertylistcell", "false");
        check("lremovepropertylistcell", "false");
        check("ncountafterwrites", "2");
        check("nnewindexafterwrites", "2");
        check("cdisplayafterwrites", "Beta Prime");
        check("crow1col1", "Alpha");
        check("crow1col2", "A");
        check("crow1col3", "111");
        check("crow2col1", "Beta Prime");
        check("crow2col2", "Bee");
        check("crow2col3", "222");
        check("nseednewindex", "2");
        check("cseedrow1col1", "North");
        check("cseedrow1col2", "N");
        check("cseedrow2col1", "South");
        check("cseedrow2col2", "S");

        expect(state.ole_objects.size() == 2U,
               "native writable List() coverage should register plain and derived list controls");
        if (state.ole_objects.size() == 2U)
        {
            const auto &plain_combo = state.ole_objects[0];
            const auto &seed_list = state.ole_objects[1];

            const auto plain_display = plain_combo.properties.find("displayvalue");
            const auto plain_newindex = plain_combo.properties.find("newindex");
            const auto seed_newindex = seed_list.properties.find("newindex");

            expect(plain_combo.list_rows.size() == 2U,
                   "plain ComboBox writable List() coverage should preserve the expected row count");
            if (plain_combo.list_rows.size() == 2U)
            {
                expect(plain_combo.list_rows[0].size() >= 3U &&
                           copperfin::runtime::format_value(plain_combo.list_rows[0][1]) == "A" &&
                           copperfin::runtime::format_value(plain_combo.list_rows[0][2]) == "111",
                       "plain ComboBox writable List() coverage should preserve row 1 multicolumn writes");
                expect(plain_combo.list_rows[1].size() >= 3U &&
                           copperfin::runtime::format_value(plain_combo.list_rows[1][0]) == "Beta Prime" &&
                           copperfin::runtime::format_value(plain_combo.list_rows[1][1]) == "Bee" &&
                           copperfin::runtime::format_value(plain_combo.list_rows[1][2]) == "222",
                       "plain ComboBox writable List() coverage should preserve row 2 direct and SETPEM writes");
            }
            expect(plain_display != plain_combo.properties.end() &&
                       copperfin::runtime::format_value(plain_display->second) == "Beta Prime",
                   "plain ComboBox writable List() coverage should keep DisplayValue synchronized");
            expect(plain_newindex != plain_combo.properties.end() &&
                       copperfin::runtime::format_value(plain_newindex->second) == "2",
                   "plain ComboBox writable List() coverage should preserve NewIndex after cell writes");
            expect(seed_list.list_rows.size() == 2U,
                   "derived ListBox writable List() coverage should preserve Init-time multicolumn writes");
            expect(seed_newindex != seed_list.properties.end() &&
                       copperfin::runtime::format_value(seed_newindex->second) == "2",
                   "derived ListBox writable List() coverage should preserve NewIndex during Init-time cell writes");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_native_list_controls_listcount_list_and_removeitem_stay_coherent()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_list_controls_listcount_removeitem";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_list_controls_listcount_removeitem.prg";
        write_text(
            main_path,
            "oPlainCombo = CREATEOBJECT('ComboBox')\n"
            "lHasListCount = PEMSTATUS(oPlainCombo, 'ListCount', 1)\n"
            "lListCountReadOnly = PEMSTATUS(oPlainCombo, 'ListCount', 5)\n"
            "lHasRemoveItem = PEMSTATUS(oPlainCombo, 'RemoveItem', 1)\n"
            "xListCountBefore = GETPEM(oPlainCombo, 'ListCount')\n"
            "oPlainCombo.AddItem('March')\n"
            "oPlainCombo.AddItem('April')\n"
            "oPlainCombo.AddItem('February', 1)\n"
            "nCountAfterAdds = oPlainCombo.ListCount\n"
            "xListCountAfterAdds = GETPEM(oPlainCombo, 'ListCount')\n"
            "cItem1 = oPlainCombo.List(1)\n"
            "cItem2 = oPlainCombo.List(2, 1)\n"
            "cMissing = oPlainCombo.List(4)\n"
            "oPlainCombo.ListIndex = 2\n"
            "cBeforeRemoveDisplay = oPlainCombo.DisplayValue\n"
            "oPlainCombo.RemoveItem(1)\n"
            "nCountAfterRemove1 = oPlainCombo.ListCount\n"
            "nIndexAfterRemove1 = oPlainCombo.ListIndex\n"
            "cDisplayAfterRemove1 = oPlainCombo.DisplayValue\n"
            "cItem1AfterRemove1 = oPlainCombo.List(1)\n"
            "cItem2AfterRemove1 = oPlainCombo.List(2)\n"
            "oPlainCombo.ListIndex = 2\n"
            "oPlainCombo.RemoveItem(2)\n"
            "nCountAfterRemove2 = oPlainCombo.ListCount\n"
            "nIndexAfterRemove2 = oPlainCombo.ListIndex\n"
            "cDisplayAfterRemove2 = oPlainCombo.DisplayValue\n"
            "oPlainCombo.RemoveItem(1)\n"
            "nCountAfterRemove3 = oPlainCombo.ListCount\n"
            "nIndexAfterRemove3 = oPlainCombo.ListIndex\n"
            "cDisplayAfterRemove3 = oPlainCombo.DisplayValue\n"
            "lSetPemListCount = SETPEM(oPlainCombo, 'ListCount', 99)\n"
            "lAddPropertyListCount = ADDPROPERTY(oPlainCombo, 'ListCount', 99)\n"
            "lRemovePropertyListCount = REMOVEPROPERTY(oPlainCombo, 'ListCount')\n"
            "oSeedList = CREATEOBJECT('SeededList')\n"
            "nSeedCount = oSeedList.ListCount\n"
            "cSeedItem1 = oSeedList.List(1)\n"
            "cSeedItem2 = oSeedList.List(2)\n"
            "nSeedIndex = oSeedList.ListIndex\n"
            "cSeedDisplay = oSeedList.DisplayValue\n"
            "RETURN\n"
            "DEFINE CLASS SeededList AS ListBox\n"
            "    PROCEDURE Init\n"
            "        THIS.AddItem('North')\n"
            "        THIS.AddItem('South')\n"
            "        THIS.AddItem('East')\n"
            "        THIS.ListIndex = 3\n"
            "        THIS.RemoveItem(2)\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native ListCount/List/RemoveItem list-control script should complete: ") + state.message +
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

        check("lhaslistcount", "true");
        check("llistcountreadonly", "true");
        check("lhasremoveitem", "true");
        check("xlistcountbefore", "0");
        check("ncountafteradds", "3");
        check("xlistcountafteradds", "3");
        check("citem1", "February");
        check("citem2", "March");
        check("cmissing", "");
        check("cbeforeremovedisplay", "March");
        check("ncountafterremove1", "2");
        check("nindexafterremove1", "1");
        check("cdisplayafterremove1", "March");
        check("citem1afterremove1", "March");
        check("citem2afterremove1", "April");
        check("ncountafterremove2", "1");
        check("nindexafterremove2", "1");
        check("cdisplayafterremove2", "March");
        check("ncountafterremove3", "0");
        check("nindexafterremove3", "0");
        check("cdisplayafterremove3", "");
        check("lsetpemlistcount", "false");
        check("laddpropertylistcount", "false");
        check("lremovepropertylistcount", "false");
        check("nseedcount", "2");
        check("cseeditem1", "North");
        check("cseeditem2", "East");
        check("nseedindex", "2");
        check("cseeddisplay", "East");

        expect(state.ole_objects.size() == 2U,
               "native ListCount/List/RemoveItem coverage should register plain and derived list controls");
        if (state.ole_objects.size() == 2U)
        {
            const auto &plain_combo = state.ole_objects[0];
            const auto &seed_list = state.ole_objects[1];

            const auto plain_listcount = plain_combo.properties.find("listcount");
            const auto plain_display = plain_combo.properties.find("displayvalue");
            const auto seed_listcount = seed_list.properties.find("listcount");
            const auto seed_display = seed_list.properties.find("displayvalue");

            expect(plain_combo.collection_items.empty(),
                   "plain ComboBox RemoveItem coverage should leave no runtime rows after all removals");
            expect(plain_listcount != plain_combo.properties.end() &&
                       copperfin::runtime::format_value(plain_listcount->second) == "0",
                   "plain ComboBox RemoveItem coverage should keep ListCount synchronized");
            expect(plain_display != plain_combo.properties.end() &&
                       copperfin::runtime::format_value(plain_display->second).empty(),
                   "plain ComboBox RemoveItem coverage should clear DisplayValue when selection becomes empty");
            expect(seed_list.collection_items.size() == 2U,
                   "derived ListBox RemoveItem coverage should preserve Init-time row removals");
            expect(seed_listcount != seed_list.properties.end() &&
                       copperfin::runtime::format_value(seed_listcount->second) == "2",
                   "derived ListBox RemoveItem coverage should keep ListCount synchronized");
            expect(seed_display != seed_list.properties.end() &&
                       copperfin::runtime::format_value(seed_display->second) == "East",
                   "derived ListBox RemoveItem coverage should preserve the shifted selected display text");
        }

        const bool has_removeitem_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.removeitem";
        });
        expect(has_removeitem_event,
               "native RemoveItem coverage should emit representative list-control method events");

        fs::remove_all(temp_root, ignored);
    }

}
