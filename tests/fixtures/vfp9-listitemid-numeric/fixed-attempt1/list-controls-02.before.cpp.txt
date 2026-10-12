#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_list_controls_addlistitem_newitemid_and_multicolumn_list_stay_coherent()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_list_controls_addlistitem";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_list_controls_addlistitem.prg";
        write_text(
            main_path,
            "oPlainCombo = CREATEOBJECT('ComboBox')\n"
            "oPlainCombo.ColumnCount = 3\n"
            "lHasAddListItem = PEMSTATUS(oPlainCombo, 'AddListItem', 1)\n"
            "lGetAddListItem = GETPEM(oPlainCombo, 'AddListItem')\n"
            "lHasNewItemId = PEMSTATUS(oPlainCombo, 'NewItemId', 1)\n"
            "lNewItemIdReadOnly = PEMSTATUS(oPlainCombo, 'NewItemId', 5)\n"
            "xNewItemIdBefore = GETPEM(oPlainCombo, 'NewItemId')\n"
            "nPlainMethodCount = AMEMBERS(aPlainMethods, oPlainCombo, 2)\n"
            "nPlainHasAddListItem = ASCAN(aPlainMethods, 'ADDLISTITEM')\n"
            "nFirstAdd = oPlainCombo.AddListItem('Cleveland')\n"
            "nFirstItemId = oPlainCombo.NewItemId\n"
            "nSecondCol1 = oPlainCombo.AddListItem('Ohio', oPlainCombo.NewItemId, 2)\n"
            "nThirdCol1 = oPlainCombo.AddListItem('44122', oPlainCombo.NewItemId, 3)\n"
            "nSecondAdd = oPlainCombo.AddListItem('Buffalo')\n"
            "nSecondItemId = oPlainCombo.NewItemId\n"
            "nSecondCol2 = oPlainCombo.AddListItem('New York', oPlainCombo.NewItemId, 2)\n"
            "nSecondCol3 = oPlainCombo.AddListItem('14228', oPlainCombo.NewItemId, 3)\n"
            "nCountAfterAdds = oPlainCombo.ListCount\n"
            "xNewItemIdAfterAdds = GETPEM(oPlainCombo, 'NewItemId')\n"
            "cRow1Col1 = oPlainCombo.List(1)\n"
            "cRow1Col2 = oPlainCombo.List(1, 2)\n"
            "cRow1Col3 = oPlainCombo.List(1, 3)\n"
            "cRow2Col1 = oPlainCombo.List(2)\n"
            "cRow2Col2 = oPlainCombo.List(2, 2)\n"
            "cRow2Col3 = oPlainCombo.List(2, 3)\n"
            "cMissingCol = oPlainCombo.List(2, 4)\n"
            "oPlainCombo.ListIndex = 2\n"
            "cDisplayBeforeRemove = oPlainCombo.DisplayValue\n"
            "oPlainCombo.RemoveItem(1)\n"
            "nCountAfterRemove = oPlainCombo.ListCount\n"
            "nIndexAfterRemove = oPlainCombo.ListIndex\n"
            "cDisplayAfterRemove = oPlainCombo.DisplayValue\n"
            "cRemainingCol1 = oPlainCombo.List(1)\n"
            "cRemainingCol2 = oPlainCombo.List(1, 2)\n"
            "cRemainingCol3 = oPlainCombo.List(1, 3)\n"
            "lSetPemNewItemId = SETPEM(oPlainCombo, 'NewItemId', 99)\n"
            "lAddPropertyNewItemId = ADDPROPERTY(oPlainCombo, 'NewItemId', 99)\n"
            "lRemovePropertyNewItemId = REMOVEPROPERTY(oPlainCombo, 'NewItemId')\n"
            "oSeedList = CREATEOBJECT('SeededList')\n"
            "nSeedCount = oSeedList.ListCount\n"
            "nSeedItemId = oSeedList.NewItemId\n"
            "cSeedRow1Col1 = oSeedList.List(1)\n"
            "cSeedRow1Col2 = oSeedList.List(1, 2)\n"
            "cSeedRow2Col1 = oSeedList.List(2)\n"
            "cSeedRow2Col2 = oSeedList.List(2, 2)\n"
            "cSeedDisplay = oSeedList.DisplayValue\n"
            "RETURN\n"
            "DEFINE CLASS SeededList AS ListBox\n"
            "    ColumnCount = 2\n"
            "    PROCEDURE Init\n"
            "        THIS.AddListItem('North')\n"
            "        THIS.AddListItem('N', THIS.NewItemId, 2)\n"
            "        THIS.AddListItem('South')\n"
            "        THIS.AddListItem('S', THIS.NewItemId, 2)\n"
            "        THIS.ListIndex = 2\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native AddListItem/NewItemId multicolumn list-control script should complete: ") + state.message +
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

        check("lhasaddlistitem", "true");
        check("lgetaddlistitem", "true");
        check("lhasnewitemid", "true");
        check("lnewitemidreadonly", "true");
        check("xnewitemidbefore", "0");
        check("nfirstadd", "1");
        check("nfirstitemid", "1");
        check("nsecondcol1", "1");
        check("nthirdcol1", "1");
        check("nsecondadd", "2");
        check("nseconditemid", "2");
        check("nsecondcol2", "2");
        check("nsecondcol3", "2");
        check("ncountafteradds", "2");
        check("xnewitemidafteradds", "2");
        check("crow1col1", "Cleveland");
        check("crow1col2", "Ohio");
        check("crow1col3", "44122");
        check("crow2col1", "Buffalo");
        check("crow2col2", "New York");
        check("crow2col3", "14228");
        check("cmissingcol", "");
        check("cdisplaybeforeremove", "Buffalo");
        check("ncountafterremove", "1");
        check("nindexafterremove", "1");
        check("cdisplayafterremove", "Buffalo");
        check("cremainingcol1", "Buffalo");
        check("cremainingcol2", "New York");
        check("cremainingcol3", "14228");
        check("lsetpemnewitemid", "false");
        check("laddpropertynewitemid", "false");
        check("lremovepropertynewitemid", "false");
        check("nseedcount", "2");
        check("nseeditemid", "2");
        check("cseedrow1col1", "North");
        check("cseedrow1col2", "N");
        check("cseedrow2col1", "South");
        check("cseedrow2col2", "S");
        check("cseeddisplay", "South");

        const auto plain_has_addlistitem = state.globals.find("nplainhasaddlistitem");
        expect(plain_has_addlistitem != state.globals.end(),
               "nPlainHasAddListItem variable should be present");
        if (plain_has_addlistitem != state.globals.end())
        {
            expect(copperfin::runtime::format_value(plain_has_addlistitem->second) != "0",
                   "AMEMBERS(..., 2) should expose the native AddListItem builtin for ComboBox");
        }

        expect(state.ole_objects.size() == 2U,
               "native AddListItem/NewItemId coverage should register plain and derived list controls");
        if (state.ole_objects.size() == 2U)
        {
            const auto &plain_combo = state.ole_objects[0];
            const auto &seed_list = state.ole_objects[1];

            const auto plain_newitemid = plain_combo.properties.find("newitemid");
            const auto seed_newitemid = seed_list.properties.find("newitemid");

            expect(plain_combo.list_rows.size() == 1U,
                   "plain ComboBox AddListItem coverage should preserve one multicolumn row after removal");
            if (plain_combo.list_rows.size() == 1U)
            {
                expect(plain_combo.list_rows[0].size() == 3U,
                       "plain ComboBox AddListItem coverage should preserve three columns on the remaining row");
                if (plain_combo.list_rows[0].size() == 3U)
                {
                    expect(copperfin::runtime::format_value(plain_combo.list_rows[0][0]) == "Buffalo",
                           "plain ComboBox AddListItem coverage should preserve remaining row column 1");
                    expect(copperfin::runtime::format_value(plain_combo.list_rows[0][1]) == "New York",
                           "plain ComboBox AddListItem coverage should preserve remaining row column 2");
                    expect(copperfin::runtime::format_value(plain_combo.list_rows[0][2]) == "14228",
                           "plain ComboBox AddListItem coverage should preserve remaining row column 3");
                }
            }
            expect(plain_combo.collection_items.size() == 1U &&
                       copperfin::runtime::format_value(plain_combo.collection_items[0]) == "Buffalo",
                   "plain ComboBox AddListItem coverage should keep first-column runtime items synchronized");
            expect(plain_newitemid != plain_combo.properties.end() &&
                       copperfin::runtime::format_value(plain_newitemid->second) == "2",
                   "plain ComboBox AddListItem coverage should preserve the most recently added item id");

            expect(seed_list.list_rows.size() == 2U,
                   "derived ListBox AddListItem coverage should preserve Init-time multicolumn rows");
            if (seed_list.list_rows.size() == 2U)
            {
                expect(seed_list.list_rows[0].size() == 2U &&
                           copperfin::runtime::format_value(seed_list.list_rows[0][1]) == "N",
                       "derived ListBox AddListItem coverage should preserve row 1 column 2");
                expect(seed_list.list_rows[1].size() == 2U &&
                           copperfin::runtime::format_value(seed_list.list_rows[1][1]) == "S",
                       "derived ListBox AddListItem coverage should preserve row 2 column 2");
            }
            expect(seed_newitemid != seed_list.properties.end() &&
                       copperfin::runtime::format_value(seed_newitemid->second) == "2",
                   "derived ListBox AddListItem coverage should preserve the most recently added item id");
        }

        const bool has_addlistitem_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.addlistitem";
        });
        expect(has_addlistitem_event,
               "native AddListItem coverage should emit representative list-control method events");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_list_controls_newindex_stays_coherent()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_list_controls_newindex";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_list_controls_newindex.prg";
        write_text(
            main_path,
            "oPlainCombo = CREATEOBJECT('ComboBox')\n"
            "lHasNewIndex = PEMSTATUS(oPlainCombo, 'NewIndex', 1)\n"
            "lNewIndexReadOnly = PEMSTATUS(oPlainCombo, 'NewIndex', 5)\n"
            "xNewIndexBefore = GETPEM(oPlainCombo, 'NewIndex')\n"
            "nPlainPropCount = AMEMBERS(aPlainProps, oPlainCombo, 1)\n"
            "nPlainHasNewIndex = ASCAN(aPlainProps, 'NEWINDEX')\n"
            "oPlainCombo.AddItem('March')\n"
            "nNewIndexAfterAdd1 = oPlainCombo.NewIndex\n"
            "oPlainCombo.AddItem('April')\n"
            "nNewIndexAfterAdd2 = oPlainCombo.NewIndex\n"
            "oPlainCombo.AddItem('February', 1)\n"
            "nNewIndexAfterInsert = oPlainCombo.NewIndex\n"
            "oPlainCombo.RemoveItem(1)\n"
            "nNewIndexAfterRemoveInserted = oPlainCombo.NewIndex\n"
            "oPlainCombo.AddListItem('Alpha', 10)\n"
            "nNewIndexAfterAddListItem1 = oPlainCombo.NewIndex\n"
            "oPlainCombo.AddListItem('Beta', 20)\n"
            "nNewIndexAfterAddListItem2 = oPlainCombo.NewIndex\n"
            "oPlainCombo.RemoveListItem(10)\n"
            "nNewIndexAfterRemoveListItem = oPlainCombo.NewIndex\n"
            "lSetPemNewIndex = SETPEM(oPlainCombo, 'NewIndex', 99)\n"
            "lAddPropertyNewIndex = ADDPROPERTY(oPlainCombo, 'NewIndex', 99)\n"
            "lRemovePropertyNewIndex = REMOVEPROPERTY(oPlainCombo, 'NewIndex')\n"
            "oSeedList = CREATEOBJECT('SeededList')\n"
            "nSeedCount = oSeedList.ListCount\n"
            "nSeedNewIndex = oSeedList.NewIndex\n"
            "cSeedItem1 = oSeedList.List(1)\n"
            "cSeedItem2 = oSeedList.List(2)\n"
            "RETURN\n"
            "DEFINE CLASS SeededList AS ListBox\n"
            "    PROCEDURE Init\n"
            "        THIS.AddItem('North')\n"
            "        THIS.AddItem('South')\n"
            "        THIS.AddItem('East', 2)\n"
            "        THIS.RemoveItem(1)\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native NewIndex list-control script should complete: ") + state.message +
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

        check("lhasnewindex", "true");
        check("lnewindexreadonly", "true");
        check("xnewindexbefore", "0");
        check("nnewindexafteradd1", "1");
        check("nnewindexafteradd2", "2");
        check("nnewindexafterinsert", "1");
        check("nnewindexafterremoveinserted", "0");
        check("nnewindexafteraddlistitem1", "3");
        check("nnewindexafteraddlistitem2", "4");
        check("nnewindexafterremovelistitem", "3");
        check("lsetpemnewindex", "false");
        check("laddpropertynewindex", "false");
        check("lremovepropertynewindex", "false");
        check("nseedcount", "2");
        check("nseednewindex", "1");
        check("cseeditem1", "East");
        check("cseeditem2", "South");

        const auto plain_has_newindex = state.globals.find("nplainhasnewindex");
        expect(plain_has_newindex != state.globals.end(),
               "nPlainHasNewIndex variable should be present");
        if (plain_has_newindex != state.globals.end())
        {
            expect(copperfin::runtime::format_value(plain_has_newindex->second) != "0",
                   "AMEMBERS(..., 1) should expose the native NewIndex builtin for ComboBox");
        }

        expect(state.ole_objects.size() == 2U,
               "native NewIndex coverage should register plain and derived list controls");
        if (state.ole_objects.size() == 2U)
        {
            const auto &plain_combo = state.ole_objects[0];
            const auto &seed_list = state.ole_objects[1];

            const auto plain_newindex = plain_combo.properties.find("newindex");
            const auto seed_newindex = seed_list.properties.find("newindex");

            expect(plain_combo.collection_items.size() == 3U,
                   "plain ComboBox NewIndex coverage should preserve the expected remaining row count");
            expect(plain_newindex != plain_combo.properties.end() &&
                       copperfin::runtime::format_value(plain_newindex->second) == "3",
                   "plain ComboBox NewIndex coverage should keep the built-in property synchronized after RemoveListItem()");
            expect(seed_list.collection_items.size() == 2U,
                   "derived ListBox NewIndex coverage should preserve Init-time insert/remove row state");
            expect(seed_newindex != seed_list.properties.end() &&
                       copperfin::runtime::format_value(seed_newindex->second) == "1",
                   "derived ListBox NewIndex coverage should keep the built-in property synchronized during Init");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_native_list_controls_newindex_addressed_writable_list_cells_stay_coherent()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root =
            fs::temp_directory_path() / "copperfin_native_list_controls_newindex_writable_list_cells";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_list_controls_newindex_writable_list_cells.prg";
        write_text(
            main_path,
            "oPlainCombo = CREATEOBJECT('ComboBox')\n"
            "oPlainCombo.ColumnCount = 3\n"
            "oPlainCombo.AddItem('Alpha')\n"
            "oPlainCombo.List(oPlainCombo.NewIndex, 2) = 'A'\n"
            "oPlainCombo.List(oPlainCombo.NewIndex, 3) = '101'\n"
            "oPlainCombo.AddItem('Delta')\n"
            "oPlainCombo.List(oPlainCombo.NewIndex, 2) = 'D'\n"
            "oPlainCombo.List(oPlainCombo.NewIndex, 3) = '404'\n"
            "oPlainCombo.AddItem('Charlie', 2)\n"
            "nNewIndexAfterInsert = oPlainCombo.NewIndex\n"
            "oPlainCombo.List(oPlainCombo.NewIndex, 2) = 'C'\n"
            "oPlainCombo.List(oPlainCombo.NewIndex, 3) = '303'\n"
            "oPlainCombo.ListIndex = oPlainCombo.NewIndex\n"
            "cDisplayBeforeShift = oPlainCombo.DisplayValue\n"
            "oPlainCombo.RemoveItem(1)\n"
            "nNewIndexAfterShift = oPlainCombo.NewIndex\n"
            "nListIndexAfterShift = oPlainCombo.ListIndex\n"
            "oPlainCombo.List(oPlainCombo.NewIndex, 2) = 'C-shift'\n"
            "oPlainCombo.List(oPlainCombo.NewIndex, 3) = '313'\n"
            "cDisplayAfterShift = oPlainCombo.DisplayValue\n"
            "oPlainCombo.AddItem('Beta', 2)\n"
            "nNewIndexAfterReinsert = oPlainCombo.NewIndex\n"
            "oPlainCombo.List(oPlainCombo.NewIndex) = 'Beta Prime'\n"
            "oPlainCombo.List(oPlainCombo.NewIndex, 2) = 'B'\n"
            "oPlainCombo.List(oPlainCombo.NewIndex, 3) = '202'\n"
            "oPlainCombo.ListIndex = oPlainCombo.NewIndex\n"
            "cDisplayAfterReinsert = oPlainCombo.DisplayValue\n"
            "oPlainCombo.RemoveItem(3)\n"
            "nFinalCount = oPlainCombo.ListCount\n"
            "nFinalNewIndex = oPlainCombo.NewIndex\n"
            "nFinalListIndex = oPlainCombo.ListIndex\n"
            "cFinalDisplay = oPlainCombo.DisplayValue\n"
            "cRow1Col1 = oPlainCombo.List(1)\n"
            "cRow1Col2 = oPlainCombo.List(1, 2)\n"
            "cRow1Col3 = oPlainCombo.List(1, 3)\n"
            "cRow2Col1 = oPlainCombo.List(2)\n"
            "cRow2Col2 = oPlainCombo.List(2, 2)\n"
            "cRow2Col3 = oPlainCombo.List(2, 3)\n"
            "cRow3Col1 = oPlainCombo.List(3)\n"
            "oSeedList = CREATEOBJECT('SeededList')\n"
            "nSeedCount = oSeedList.ListCount\n"
            "nSeedNewIndex = oSeedList.NewIndex\n"
            "cSeedRow1Col1 = oSeedList.List(1)\n"
            "cSeedRow1Col2 = oSeedList.List(1, 2)\n"
            "cSeedRow1Col3 = oSeedList.List(1, 3)\n"
            "cSeedRow2Col1 = oSeedList.List(2)\n"
            "cSeedRow2Col2 = oSeedList.List(2, 2)\n"
            "cSeedRow2Col3 = oSeedList.List(2, 3)\n"
            "cSeedRow3Col1 = oSeedList.List(3)\n"
            "RETURN\n"
            "DEFINE CLASS SeededList AS ListBox\n"
            "    ColumnCount = 3\n"
            "    PROCEDURE Init\n"
            "        THIS.AddItem('North')\n"
            "        THIS.List(THIS.NewIndex, 2) = 'N'\n"
            "        THIS.AddItem('West')\n"
            "        THIS.List(THIS.NewIndex, 2) = 'W'\n"
            "        THIS.AddItem('South', 2)\n"
            "        THIS.List(THIS.NewIndex, 2) = 'S'\n"
            "        THIS.RemoveItem(1)\n"
            "        THIS.List(THIS.NewIndex, 3) = 'pivot'\n"
            "        THIS.AddItem('East', 2)\n"
            "        THIS.List(THIS.NewIndex, 2) = 'E'\n"
            "        THIS.List(THIS.NewIndex, 3) = 'tail'\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native NewIndex-addressed writable List() script should complete: ") +
                   state.message + " @line=" + std::to_string(state.location.line));

        const auto check = [&](const std::string& name, const std::string& expected) {
            const auto it = state.globals.find(name);
            if (it == state.globals.end())
            {
                expect(false, name + " variable not found");
                return;
            }
            expect(copperfin::runtime::format_value(it->second) == expected,
                   name + " expected '" + expected + "' got '" +
                       copperfin::runtime::format_value(it->second) + "'");
        };

        check("nnewindexafterinsert", "2");
        check("cdisplaybeforeshift", "Charlie");
        check("nnewindexaftershift", "1");
        check("nlistindexaftershift", "1");
        check("cdisplayaftershift", "Charlie");
        check("nnewindexafterreinsert", "2");
        check("cdisplayafterreinsert", "Beta Prime");
        check("nfinalcount", "2");
        check("nfinalnewindex", "2");
        check("nfinallistindex", "2");
        check("cfinaldisplay", "Beta Prime");
        check("crow1col1", "Charlie");
        check("crow1col2", "C-shift");
        check("crow1col3", "313");
        check("crow2col1", "Beta Prime");
        check("crow2col2", "B");
        check("crow2col3", "202");
        check("crow3col1", "");
        check("nseedcount", "3");
        check("nseednewindex", "2");
        check("cseedrow1col1", "South");
        check("cseedrow1col2", "S");
        check("cseedrow1col3", "pivot");
        check("cseedrow2col1", "East");
        check("cseedrow2col2", "E");
        check("cseedrow2col3", "tail");
        check("cseedrow3col1", "West");

        expect(state.ole_objects.size() == 2U,
               "native NewIndex-addressed writable List() coverage should register plain and derived list controls");
        if (state.ole_objects.size() == 2U)
        {
            const auto& plain_combo = state.ole_objects[0];
            const auto& seed_list = state.ole_objects[1];

            const auto plain_newindex = plain_combo.properties.find("newindex");
            const auto plain_listindex = plain_combo.properties.find("listindex");
            const auto plain_display = plain_combo.properties.find("displayvalue");
            const auto seed_newindex = seed_list.properties.find("newindex");

            expect(plain_combo.list_rows.size() == 2U,
                   "plain ComboBox NewIndex-addressed writable List() coverage should preserve shifted rows");
            if (plain_combo.list_rows.size() == 2U)
            {
                expect(plain_combo.list_rows[0].size() >= 3U &&
                           copperfin::runtime::format_value(plain_combo.list_rows[0][0]) == "Charlie" &&
                           copperfin::runtime::format_value(plain_combo.list_rows[0][1]) == "C-shift" &&
                           copperfin::runtime::format_value(plain_combo.list_rows[0][2]) == "313",
                       "plain ComboBox NewIndex-addressed writes should follow the shifted latest-added row");
                expect(plain_combo.list_rows[1].size() >= 3U &&
                           copperfin::runtime::format_value(plain_combo.list_rows[1][0]) == "Beta Prime" &&
                           copperfin::runtime::format_value(plain_combo.list_rows[1][1]) == "B" &&
                           copperfin::runtime::format_value(plain_combo.list_rows[1][2]) == "202",
                       "plain ComboBox NewIndex-addressed writes should land on the reinserted latest row");
            }
            expect(plain_newindex != plain_combo.properties.end() &&
                       copperfin::runtime::format_value(plain_newindex->second) == "2",
                   "plain ComboBox NewIndex-addressed coverage should keep NewIndex synchronized after churn");
            expect(plain_listindex != plain_combo.properties.end() &&
                       copperfin::runtime::format_value(plain_listindex->second) == "2",
                   "plain ComboBox NewIndex-addressed coverage should keep ListIndex synchronized after churn");
            expect(plain_display != plain_combo.properties.end() &&
                       copperfin::runtime::format_value(plain_display->second) == "Beta Prime",
                   "plain ComboBox NewIndex-addressed coverage should keep DisplayValue synchronized after churn");
            expect(seed_list.list_rows.size() == 3U,
                   "derived ListBox NewIndex-addressed writable List() coverage should preserve churned Init rows");
            if (seed_list.list_rows.size() == 3U)
            {
                expect(seed_list.list_rows[0].size() >= 3U &&
                           copperfin::runtime::format_value(seed_list.list_rows[0][0]) == "South" &&
                           copperfin::runtime::format_value(seed_list.list_rows[0][2]) == "pivot",
                       "derived ListBox NewIndex-addressed writes should follow the shifted row during Init");
                expect(seed_list.list_rows[1].size() >= 3U &&
                           copperfin::runtime::format_value(seed_list.list_rows[1][0]) == "East" &&
                           copperfin::runtime::format_value(seed_list.list_rows[1][1]) == "E" &&
                           copperfin::runtime::format_value(seed_list.list_rows[1][2]) == "tail",
                       "derived ListBox NewIndex-addressed writes should land on the latest inserted row during Init");
            }
            expect(seed_newindex != seed_list.properties.end() &&
                       copperfin::runtime::format_value(seed_newindex->second) == "2",
                   "derived ListBox NewIndex-addressed coverage should keep NewIndex synchronized during Init churn");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_native_list_controls_listitemid_selection_stays_coherent()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_list_controls_listitemid";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_list_controls_listitemid.prg";
        write_text(
            main_path,
            "oPlainCombo = CREATEOBJECT('ComboBox')\n"
            "oPlainCombo.ColumnCount = 2\n"
            "lHasListItemId = PEMSTATUS(oPlainCombo, 'ListItemID', 1)\n"
            "lListItemIdReadOnly = PEMSTATUS(oPlainCombo, 'ListItemID', 5)\n"
            "xListItemIdBefore = GETPEM(oPlainCombo, 'ListItemID')\n"
            "nPlainPropCount = AMEMBERS(aPlainProps, oPlainCombo, 1)\n"
            "nPlainHasListItemId = ASCAN(aPlainProps, 'LISTITEMID')\n"
            "oPlainCombo.AddListItem('Alpha')\n"
            "nFirstItemId = oPlainCombo.NewItemId\n"
            "oPlainCombo.AddListItem('A', oPlainCombo.NewItemId, 2)\n"
            "oPlainCombo.AddListItem('Beta')\n"
            "nSecondItemId = oPlainCombo.NewItemId\n"
            "oPlainCombo.AddListItem('B', oPlainCombo.NewItemId, 2)\n"
            "oPlainCombo.ListItemID = nSecondItemId\n"
            "nIndexAfterDirectSet = oPlainCombo.ListIndex\n"
            "cDisplayAfterDirectSet = oPlainCombo.DisplayValue\n"
            "xListItemIdAfterDirectSet = GETPEM(oPlainCombo, 'ListItemID')\n"
            "lSetPemFirst = SETPEM(oPlainCombo, 'ListItemID', nFirstItemId)\n"
            "nIndexAfterSetPem = oPlainCombo.ListIndex\n"
            "cDisplayAfterSetPem = oPlainCombo.DisplayValue\n"
            "xListItemIdAfterSetPem = GETPEM(oPlainCombo, 'ListItemID')\n"
            "lSetPemMissing = SETPEM(oPlainCombo, 'ListItemID', 77)\n"
            "nIndexAfterMissing = oPlainCombo.ListIndex\n"
            "lSetPemZero = SETPEM(oPlainCombo, 'ListItemID', 0)\n"
            "nIndexAfterZero = oPlainCombo.ListIndex\n"
            "cDisplayAfterZero = oPlainCombo.DisplayValue\n"
            "xListItemIdAfterZero = GETPEM(oPlainCombo, 'ListItemID')\n"
            "oPlainCombo.ListItemID = nSecondItemId\n"
            "oPlainCombo.RemoveItem(1)\n"
            "nIndexAfterRemove = oPlainCombo.ListIndex\n"
            "nListItemIdAfterRemove = oPlainCombo.ListItemID\n"
            "cDisplayAfterRemove = oPlainCombo.DisplayValue\n"
            "cRemainingCol2 = oPlainCombo.List(1, 2)\n"
            "lAddPropertyListItemId = ADDPROPERTY(oPlainCombo, 'ListItemID', 9)\n"
            "lRemovePropertyListItemId = REMOVEPROPERTY(oPlainCombo, 'ListItemID')\n"
            "oSeedList = CREATEOBJECT('SeededList')\n"
            "nSeedListItemId = oSeedList.ListItemID\n"
            "xSeedGetPem = GETPEM(oSeedList, 'ListItemID')\n"
            "lSeedSetPem = SETPEM(oSeedList, 'ListItemID', 1)\n"
            "nSeedIndexAfterSetPem = oSeedList.ListIndex\n"
            "nSeedListItemIdAfterSetPem = oSeedList.ListItemID\n"
            "cSeedDisplayAfterSetPem = oSeedList.DisplayValue\n"
            "RETURN\n"
            "DEFINE CLASS SeededList AS ListBox\n"
            "    ColumnCount = 2\n"
            "    PROCEDURE Init\n"
            "        THIS.AddListItem('North')\n"
            "        THIS.AddListItem('N', THIS.NewItemId, 2)\n"
            "        THIS.AddListItem('South')\n"
            "        THIS.AddListItem('S', THIS.NewItemId, 2)\n"
            "        THIS.ListIndex = 2\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native ListItemID list-control script should complete: ") + state.message +
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

        check("lhaslistitemid", "true");
        check("llistitemidreadonly", "false");
        check("xlistitemidbefore", "0");
        check("nfirstitemid", "1");
        check("nseconditemid", "2");
        check("nindexafterdirectset", "2");
        check("cdisplayafterdirectset", "Beta");
        check("xlistitemidafterdirectset", "2");
        check("lsetpemfirst", "true");
        check("nindexaftersetpem", "1");
        check("cdisplayaftersetpem", "Alpha");
        check("xlistitemidaftersetpem", "1");
        check("lsetpemmissing", "false");
        check("nindexaftermissing", "1");
        check("lsetpemzero", "true");
        check("nindexafterzero", "0");
        check("cdisplayafterzero", "");
        check("xlistitemidafterzero", "0");
        check("nindexafterremove", "1");
        check("nlistitemidafterremove", "2");
        check("cdisplayafterremove", "Beta");
        check("cremainingcol2", "B");
        check("laddpropertylistitemid", "false");
        check("lremovepropertylistitemid", "false");
        check("nseedlistitemid", "2");
        check("xseedgetpem", "2");
        check("lseedsetpem", "true");
        check("nseedindexaftersetpem", "1");
        check("nseedlistitemidaftersetpem", "1");
        check("cseeddisplayaftersetpem", "North");

        const auto plain_has_listitemid = state.globals.find("nplainhaslistitemid");
        expect(plain_has_listitemid != state.globals.end(),
               "nPlainHasListItemId variable should be present");
        if (plain_has_listitemid != state.globals.end())
        {
            expect(copperfin::runtime::format_value(plain_has_listitemid->second) != "0",
                   "AMEMBERS(..., 1) should expose the native ListItemID builtin for ComboBox");
        }

        expect(state.ole_objects.size() == 2U,
               "native ListItemID coverage should register plain and derived list controls");
        if (state.ole_objects.size() == 2U)
        {
            const auto &plain_combo = state.ole_objects[0];
            const auto &seed_list = state.ole_objects[1];

            const auto plain_listitemid = plain_combo.properties.find("listitemid");
            const auto plain_listindex = plain_combo.properties.find("listindex");
            const auto seed_listitemid = seed_list.properties.find("listitemid");
            const auto seed_listindex = seed_list.properties.find("listindex");

            expect(plain_combo.collection_item_keys.size() == 1U &&
                       plain_combo.collection_item_keys[0] == "2",
                   "plain ComboBox ListItemID coverage should preserve the surviving item id after removal");
            expect(plain_listitemid != plain_combo.properties.end() &&
                       copperfin::runtime::format_value(plain_listitemid->second) == "2",
                   "plain ComboBox ListItemID coverage should keep the built-in property synchronized");
            expect(plain_listindex != plain_combo.properties.end() &&
                       copperfin::runtime::format_value(plain_listindex->second) == "1",
                   "plain ComboBox ListItemID coverage should shift ListIndex to the surviving selected row");
            expect(seed_listitemid != seed_list.properties.end() &&
                       copperfin::runtime::format_value(seed_listitemid->second) == "1",
                   "derived ListBox ListItemID coverage should keep the built-in property synchronized after SETPEM()");
            expect(seed_listindex != seed_list.properties.end() &&
                       copperfin::runtime::format_value(seed_listindex->second) == "1",
                   "derived ListBox ListItemID coverage should keep ListIndex synchronized after SETPEM()");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_native_list_controls_listitem_itemid_addressing_stays_coherent()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root =
            fs::temp_directory_path() / "copperfin_native_list_controls_listitem";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_list_controls_listitem.prg";
        write_text(
            main_path,
            "oPlain = CREATEOBJECT('ComboBox')\n"
            "oPlain.ColumnCount = 2\n"
            "lHasListItem = PEMSTATUS(oPlain, 'ListItem', 1)\n"
            "lListItemReadOnly = PEMSTATUS(oPlain, 'ListItem', 5)\n"
            "nPropCount = AMEMBERS(aProps, oPlain, 1)\n"
            "nHasListItem = ASCAN(aProps, 'LISTITEM')\n"
            "oPlain.AddListItem('Zulu', 10)\n"
            "oPlain.AddListItem('Z', 10, 2)\n"
            "oPlain.AddListItem('Alpha', 20)\n"
            "oPlain.AddListItem('A', 20, 2)\n"
            "oPlain.AddListItem('Echo', 30)\n"
            "oPlain.AddListItem('E', 30, 2)\n"
            "cItem10BeforeSort = oPlain.ListItem(10)\n"
            "cItem20BeforeSort = oPlain.ListItem[20]\n"
            "oPlain.Sorted = .T.\n"
            "cDisplay1AfterSort = oPlain.List(1)\n"
            "cDisplay3AfterSort = oPlain.List(3)\n"
            "cItem10AfterSort = oPlain.ListItem[10]\n"
            "cItem20Col2AfterSort = oPlain.ListItem(20, 2)\n"
            "nLookupId = 10\n"
            "cItem10FromVarRead = oPlain.ListItem[m.nLookupId]\n"
            "nTargetId = 10\n"
            "oPlain.ListItem[m.nTargetId, 2] = 'Zulu Prime'\n"
            "cItem10Col2AfterDirectWrite = oPlain.ListItem(10, 2)\n"
            "cDisplayRow3Col2BeforeSetPem = oPlain.List(3, 2)\n"
            "lSetPem20 = SETPEM(oPlain, 'ListItem(20,2)', 'Alpha Prime')\n"
            "xGetPem20 = GETPEM(oPlain, 'ListItem[20,2]')\n"
            "cDisplayRow1Col2AfterSetPem = oPlain.List(1, 2)\n"
            "cDisplayRow3Col2AfterDirectWrite = oPlain.List(3, 2)\n"
            "cMissingItem = oPlain.ListItem(77)\n"
            "lSetPemMissing = SETPEM(oPlain, 'ListItem(77)', 'Ghost')\n"
            "lAddPropertyListItem = ADDPROPERTY(oPlain, 'ListItem', 'shadow')\n"
            "lAddPropertyListItemCell = ADDPROPERTY(oPlain, 'ListItem(20)', 'shadow')\n"
            "lRemovePropertyListItem = REMOVEPROPERTY(oPlain, 'ListItem')\n"
            "lRemovePropertyListItemCell = REMOVEPROPERTY(oPlain, 'ListItem[20]')\n"
            "oPlain.RemoveListItem(20)\n"
            "cItem20AfterRemove = oPlain.ListItem(20)\n"
            "cItem10AfterRemove = oPlain.ListItem[10]\n"
            "cDisplay1AfterRemove = oPlain.List(1)\n"
            "oSeed = CREATEOBJECT('SeededList')\n"
            "cSeedItem100 = oSeed.ListItem(100)\n"
            "cSeedItem200Col2 = oSeed.ListItem[200,2]\n"
            "RETURN\n"
            "DEFINE CLASS SeededList AS ListBox\n"
            "    ColumnCount = 2\n"
            "    PROCEDURE Init\n"
            "        THIS.AddListItem('North', 200)\n"
            "        THIS.AddListItem('N', 200, 2)\n"
            "        THIS.AddListItem('East', 100)\n"
            "        THIS.AddListItem('E', 100, 2)\n"
            "        THIS.Sorted = .T.\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native ListItem list-control script should complete: ") + state.message +
                   " @line=" + std::to_string(state.location.line));

        const auto check = [&](const std::string& name, const std::string& expected)
        {
            const auto it = state.globals.find(name);
            if (it == state.globals.end())
            {
                expect(false, name + " variable not found");
                return;
            }
            expect(copperfin::runtime::format_value(it->second) == expected,
                   name + " expected '" + expected + "' got '" +
                       copperfin::runtime::format_value(it->second) + "'");
        };

        check("lhaslistitem", "true");
        check("llistitemreadonly", "false");
        check("citem10beforesort", "Zulu");
        check("citem20beforesort", "Alpha");
        check("cdisplay1aftersort", "Alpha");
        check("cdisplay3aftersort", "Zulu");
        check("citem10aftersort", "Zulu");
        check("citem20col2aftersort", "A");
        check("citem10fromvarread", "Zulu");
        check("citem10col2afterdirectwrite", "Zulu Prime");
        check("cdisplayrow3col2beforesetpem", "Zulu Prime");
        check("lsetpem20", "true");
        check("xgetpem20", "Alpha Prime");
        check("cdisplayrow1col2aftersetpem", "Alpha Prime");
        check("cdisplayrow3col2afterdirectwrite", "Zulu Prime");
        check("cmissingitem", "");
        check("lsetpemmissing", "false");
        check("laddpropertylistitem", "false");
        check("laddpropertylistitemcell", "false");
        check("lremovepropertylistitem", "false");
        check("lremovepropertylistitemcell", "false");
        check("citem20afterremove", "");
        check("citem10afterremove", "Zulu");
        check("cdisplay1afterremove", "Echo");
        check("cseeditem100", "East");
        check("cseeditem200col2", "N");

        const auto plain_has_listitem = state.globals.find("nhaslistitem");
        expect(plain_has_listitem != state.globals.end(),
               "nHasListItem variable should be present");
        if (plain_has_listitem != state.globals.end())
        {
            expect(copperfin::runtime::format_value(plain_has_listitem->second) != "0",
                   "AMEMBERS(..., 1) should expose the native ListItem builtin for ComboBox");
        }

        expect(state.ole_objects.size() == 2U,
               "native ListItem coverage should register plain and derived list controls");
        if (state.ole_objects.size() == 2U)
        {
            const auto& plain_combo = state.ole_objects[0];
            const auto& seed_list = state.ole_objects[1];

            expect(plain_combo.collection_item_keys.size() == 2U &&
                       plain_combo.collection_item_keys[0] == "30" &&
                       plain_combo.collection_item_keys[1] == "10",
                   "plain ComboBox ListItem coverage should preserve item IDs after sorted removal");
            expect(plain_combo.list_rows.size() == 2U &&
                       copperfin::runtime::format_value(plain_combo.list_rows[0][0]) == "Echo" &&
                       plain_combo.list_rows[0].size() >= 2U &&
                       copperfin::runtime::format_value(plain_combo.list_rows[0][1]) == "E" &&
                       copperfin::runtime::format_value(plain_combo.list_rows[1][0]) == "Zulu" &&
                       plain_combo.list_rows[1].size() >= 2U &&
                       copperfin::runtime::format_value(plain_combo.list_rows[1][1]) == "Zulu Prime",
                   "plain ComboBox ListItem coverage should keep item-ID writes attached to the same logical rows");
            expect(seed_list.collection_item_keys.size() == 2U &&
                       seed_list.collection_item_keys[0] == "100" &&
                       seed_list.collection_item_keys[1] == "200",
                   "derived ListBox ListItem coverage should preserve sorted Init-time item ID order");
        }

        fs::remove_all(temp_root, ignored);
    }

}
