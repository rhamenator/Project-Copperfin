#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_list_controls_selectedid_selection_stays_coherent()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_list_controls_selectedid";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_list_controls_selectedid.prg";
        write_text(
            main_path,
            "oPlainCombo = CREATEOBJECT('ComboBox')\n"
            "oPlainCombo.ColumnCount = 2\n"
            "oPlainCombo.AddListItem('Alpha', 11)\n"
            "oPlainCombo.AddListItem('A', 11, 2)\n"
            "oPlainCombo.AddListItem('Beta', 22)\n"
            "oPlainCombo.AddListItem('B', 22, 2)\n"
            "oPlainCombo.AddListItem('Gamma', 33)\n"
            "oPlainCombo.AddListItem('C', 33, 2)\n"
            "nTargetItemId = 22\n"
            "oPlainCombo.SelectedID(11) = .T.\n"
            "oPlainCombo.SelectedID(m.nTargetItemId) = .T.\n"
            "lSelected11 = oPlainCombo.SelectedID(11)\n"
            "lSelected22 = oPlainCombo.SelectedID(22)\n"
            "lSelected33 = oPlainCombo.SelectedID(33)\n"
            "nListItemIdAfterSet = oPlainCombo.ListItemID\n"
            "nListIndexAfterSet = oPlainCombo.ListIndex\n"
            "cDisplayAfterSet = oPlainCombo.DisplayValue\n"
            "oPlainCombo.RemoveItem(1)\n"
            "lSelected22AfterRemove = oPlainCombo.SelectedID(22)\n"
            "nListItemIdAfterRemove = oPlainCombo.ListItemID\n"
            "nListIndexAfterRemove = oPlainCombo.ListIndex\n"
            "cDisplayAfterRemove = oPlainCombo.DisplayValue\n"
            "oPlainCombo.SelectedID(22) = .F.\n"
            "lSelected22AfterClear = oPlainCombo.SelectedID(22)\n"
            "nListItemIdAfterClear = oPlainCombo.ListItemID\n"
            "nListIndexAfterClear = oPlainCombo.ListIndex\n"
            "cDisplayAfterClear = oPlainCombo.DisplayValue\n"
            "lSetPemSelected33 = SETPEM(oPlainCombo, 'SelectedID(33)', .T.)\n"
            "lSelected33AfterSetPem = oPlainCombo.SelectedID(33)\n"
            "nListItemIdAfterSetPem = oPlainCombo.ListItemID\n"
            "nListIndexAfterSetPem = oPlainCombo.ListIndex\n"
            "lSetPemMissing = SETPEM(oPlainCombo, 'SelectedID(99)', .T.)\n"
            "oSeedList = CREATEOBJECT('SeededList')\n"
            "lSeedSelected100 = oSeedList.SelectedID(100)\n"
            "lSeedSelected200 = oSeedList.SelectedID(200)\n"
            "nSeedListItemId = oSeedList.ListItemID\n"
            "nSeedListIndex = oSeedList.ListIndex\n"
            "cSeedDisplay = oSeedList.DisplayValue\n"
            "RETURN\n"
            "DEFINE CLASS SeededList AS ListBox\n"
            "    PROCEDURE Init\n"
            "        THIS.AddListItem('North', 100)\n"
            "        THIS.AddListItem('South', 200)\n"
            "        THIS.SelectedID(200) = .T.\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native SelectedID list-control script should complete: ") + state.message +
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

        check("lselected11", "false");
        check("lselected22", "true");
        check("lselected33", "false");
        check("nlistitemidafterset", "22");
        check("nlistindexafterset", "2");
        check("cdisplayafterset", "Beta");
        check("lselected22afterremove", "true");
        check("nlistitemidafterremove", "22");
        check("nlistindexafterremove", "1");
        check("cdisplayafterremove", "Beta");
        check("lselected22afterclear", "false");
        check("nlistitemidafterclear", "0");
        check("nlistindexafterclear", "0");
        check("cdisplayafterclear", "");
        check("lsetpemselected33", "true");
        check("lselected33aftersetpem", "true");
        check("nlistitemidaftersetpem", "33");
        check("nlistindexaftersetpem", "2");
        check("lsetpemmissing", "false");
        check("lseedselected100", "false");
        check("lseedselected200", "true");
        check("nseedlistitemid", "200");
        check("nseedlistindex", "2");
        check("cseeddisplay", "South");

        expect(state.ole_objects.size() == 2U,
               "native SelectedID coverage should register plain and derived list controls");
        if (state.ole_objects.size() == 2U)
        {
            const auto &plain_combo = state.ole_objects[0];
            const auto &seed_list = state.ole_objects[1];

            const auto plain_listitemid = plain_combo.properties.find("listitemid");
            const auto plain_listindex = plain_combo.properties.find("listindex");
            const auto seed_listitemid = seed_list.properties.find("listitemid");
            const auto seed_listindex = seed_list.properties.find("listindex");

            expect(plain_combo.collection_item_keys.size() == 2U &&
                       plain_combo.collection_item_keys[0] == "22" &&
                       plain_combo.collection_item_keys[1] == "33",
                   "plain ComboBox SelectedID coverage should preserve surviving item ids after removal");
            expect(plain_combo.list_selected.size() == 2U &&
                       !plain_combo.list_selected[0] &&
                       plain_combo.list_selected[1],
                   "plain ComboBox SelectedID coverage should move the active selection bit to the surviving item-id row");
            expect(plain_listitemid != plain_combo.properties.end() &&
                       copperfin::runtime::format_value(plain_listitemid->second) == "33",
                   "plain ComboBox SelectedID coverage should keep ListItemID synchronized after SETPEM()");
            expect(plain_listindex != plain_combo.properties.end() &&
                       copperfin::runtime::format_value(plain_listindex->second) == "2",
                   "plain ComboBox SelectedID coverage should keep ListIndex synchronized after SETPEM()");
            expect(seed_list.list_selected.size() == 2U &&
                       !seed_list.list_selected[0] &&
                       seed_list.list_selected[1],
                   "derived ListBox SelectedID coverage should preserve Init-time item-id selection bits");
            expect(seed_listitemid != seed_list.properties.end() &&
                       copperfin::runtime::format_value(seed_listitemid->second) == "200",
                   "derived ListBox SelectedID coverage should keep ListItemID synchronized during Init");
            expect(seed_listindex != seed_list.properties.end() &&
                       copperfin::runtime::format_value(seed_listindex->second) == "2",
                   "derived ListBox SelectedID coverage should keep ListIndex synchronized during Init");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_native_list_controls_removelistitem_stays_coherent()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_list_controls_removelistitem";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_list_controls_removelistitem.prg";
        write_text(
            main_path,
            "oPlainCombo = CREATEOBJECT('ComboBox')\n"
            "oPlainCombo.ColumnCount = 2\n"
            "lHasRemoveListItem = PEMSTATUS(oPlainCombo, 'RemoveListItem', 1)\n"
            "lGetRemoveListItem = GETPEM(oPlainCombo, 'RemoveListItem')\n"
            "nPlainMethodCount = AMEMBERS(aPlainMethods, oPlainCombo, 2)\n"
            "nPlainHasRemoveListItem = ASCAN(aPlainMethods, 'REMOVELISTITEM')\n"
            "oPlainCombo.AddListItem('Alpha', 10)\n"
            "oPlainCombo.AddListItem('A', 10, 2)\n"
            "oPlainCombo.AddListItem('Beta', 20)\n"
            "oPlainCombo.AddListItem('B', 20, 2)\n"
            "oPlainCombo.AddListItem('Gamma', 30)\n"
            "oPlainCombo.AddListItem('C', 30, 2)\n"
            "oPlainCombo.ListItemID = 20\n"
            "oPlainCombo.RemoveListItem(10)\n"
            "nCountAfterRemove10 = oPlainCombo.ListCount\n"
            "nListIndexAfterRemove10 = oPlainCombo.ListIndex\n"
            "nListItemIdAfterRemove10 = oPlainCombo.ListItemID\n"
            "cDisplayAfterRemove10 = oPlainCombo.DisplayValue\n"
            "cCol2AfterRemove10 = oPlainCombo.List(1, 2)\n"
            "oPlainCombo.RemoveListItem(20)\n"
            "nCountAfterRemove20 = oPlainCombo.ListCount\n"
            "nListIndexAfterRemove20 = oPlainCombo.ListIndex\n"
            "nListItemIdAfterRemove20 = oPlainCombo.ListItemID\n"
            "cDisplayAfterRemove20 = oPlainCombo.DisplayValue\n"
            "cCol1AfterRemove20 = oPlainCombo.List(1)\n"
            "oPlainCombo.RemoveListItem(77)\n"
            "nCountAfterMissing = oPlainCombo.ListCount\n"
            "nListItemIdAfterMissing = oPlainCombo.ListItemID\n"
            "oSeedList = CREATEOBJECT('SeededList')\n"
            "nSeedCount = oSeedList.ListCount\n"
            "nSeedIndex = oSeedList.ListIndex\n"
            "nSeedListItemId = oSeedList.ListItemID\n"
            "cSeedDisplay = oSeedList.DisplayValue\n"
            "lSeedSelected100 = oSeedList.SelectedID(100)\n"
            "lSeedSelected300 = oSeedList.SelectedID(300)\n"
            "RETURN\n"
            "DEFINE CLASS SeededList AS ListBox\n"
            "    PROCEDURE Init\n"
            "        THIS.AddListItem('North', 100)\n"
            "        THIS.AddListItem('South', 200)\n"
            "        THIS.AddListItem('East', 300)\n"
            "        THIS.ListItemID = 300\n"
            "        THIS.RemoveListItem(200)\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native RemoveListItem list-control script should complete: ") + state.message +
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

        check("lhasremovelistitem", "true");
        check("lgetremovelistitem", "true");
        check("ncountafterremove10", "2");
        check("nlistindexafterremove10", "1");
        check("nlistitemidafterremove10", "20");
        check("cdisplayafterremove10", "Beta");
        check("ccol2afterremove10", "B");
        check("ncountafterremove20", "1");
        check("nlistindexafterremove20", "1");
        check("nlistitemidafterremove20", "30");
        check("cdisplayafterremove20", "Gamma");
        check("ccol1afterremove20", "Gamma");
        check("ncountaftermissing", "1");
        check("nlistitemidaftermissing", "30");
        check("nseedcount", "2");
        check("nseedindex", "2");
        check("nseedlistitemid", "300");
        check("cseeddisplay", "East");
        check("lseedselected100", "false");
        check("lseedselected300", "true");

        const auto plain_has_removelistitem = state.globals.find("nplainhasremovelistitem");
        expect(plain_has_removelistitem != state.globals.end(),
               "nPlainHasRemoveListItem variable should be present");
        if (plain_has_removelistitem != state.globals.end())
        {
            expect(copperfin::runtime::format_value(plain_has_removelistitem->second) != "0",
                   "AMEMBERS(..., 2) should expose the native RemoveListItem builtin for ComboBox");
        }

        expect(state.ole_objects.size() == 2U,
               "native RemoveListItem coverage should register plain and derived list controls");
        if (state.ole_objects.size() == 2U)
        {
            const auto &plain_combo = state.ole_objects[0];
            const auto &seed_list = state.ole_objects[1];

            const auto plain_listitemid = plain_combo.properties.find("listitemid");
            const auto plain_listindex = plain_combo.properties.find("listindex");
            const auto seed_listitemid = seed_list.properties.find("listitemid");
            const auto seed_listindex = seed_list.properties.find("listindex");

            expect(plain_combo.collection_item_keys.size() == 1U &&
                       plain_combo.collection_item_keys[0] == "30",
                   "plain ComboBox RemoveListItem coverage should preserve the surviving item id");
            expect(plain_combo.list_selected.size() == 1U &&
                       plain_combo.list_selected[0],
                   "plain ComboBox RemoveListItem coverage should keep the surviving selected row active");
            expect(plain_listitemid != plain_combo.properties.end() &&
                       copperfin::runtime::format_value(plain_listitemid->second) == "30",
                   "plain ComboBox RemoveListItem coverage should keep ListItemID synchronized");
            expect(plain_listindex != plain_combo.properties.end() &&
                       copperfin::runtime::format_value(plain_listindex->second) == "1",
                   "plain ComboBox RemoveListItem coverage should keep ListIndex synchronized");
            expect(seed_list.collection_item_keys.size() == 2U &&
                       seed_list.collection_item_keys[0] == "100" &&
                       seed_list.collection_item_keys[1] == "300",
                   "derived ListBox RemoveListItem coverage should preserve surviving item ids after Init-time removal");
            expect(seed_listitemid != seed_list.properties.end() &&
                       copperfin::runtime::format_value(seed_listitemid->second) == "300",
                   "derived ListBox RemoveListItem coverage should keep ListItemID synchronized during Init");
            expect(seed_listindex != seed_list.properties.end() &&
                       copperfin::runtime::format_value(seed_listindex->second) == "2",
                   "derived ListBox RemoveListItem coverage should keep ListIndex synchronized during Init");
        }

        const bool has_removelistitem_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.removelistitem";
        });
        expect(has_removelistitem_event,
               "native RemoveListItem coverage should emit representative list-control method events");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_listbox_selected_property_stays_coherent()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_listbox_selected";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_listbox_selected.prg";
        write_text(
            main_path,
            "oPlain = CREATEOBJECT('ListBox')\n"
            "oPlain.MultiSelect = .T.\n"
            "oPlain.AddItem('Alpha')\n"
            "oPlain.AddItem('Beta')\n"
            "oPlain.AddItem('Gamma')\n"
            "nSelectSlot = 2\n"
            "oPlain.Selected(1) = .T.\n"
            "oPlain.Selected(m.nSelectSlot) = .T.\n"
            "lSelected1 = oPlain.Selected(1)\n"
            "lSelected2 = oPlain.Selected(2)\n"
            "lSelected3 = oPlain.Selected(3)\n"
            "nIndexAfterSecondSelect = oPlain.ListIndex\n"
            "cDisplayAfterSecondSelect = oPlain.DisplayValue\n"
            "oPlain.Selected(2) = .F.\n"
            "lSelected2AfterClear = oPlain.Selected(2)\n"
            "nIndexAfterClear = oPlain.ListIndex\n"
            "nItemIdAfterClear = oPlain.ListItemID\n"
            "cValueAfterClear = oPlain.Value\n"
            "cDisplayAfterClear = oPlain.DisplayValue\n"
            "oPlain.Selected(3) = .T.\n"
            "oPlain.RemoveItem(2)\n"
            "nCountAfterRemove = oPlain.ListCount\n"
            "lSelected1AfterRemove = oPlain.Selected(1)\n"
            "lSelected2AfterRemove = oPlain.Selected(2)\n"
            "nIndexAfterRemove = oPlain.ListIndex\n"
            "cDisplayAfterRemove = oPlain.DisplayValue\n"
            "lSetPemSelected1 = SETPEM(oPlain, 'Selected(1)', .T.)\n"
            "lSelected1AfterSetPem = oPlain.Selected(1)\n"
            "lSetPemMissing = SETPEM(oPlain, 'Selected(9)', .T.)\n"
            "oSeed = CREATEOBJECT('SeededList')\n"
            "lSeedRow1 = oSeed.Selected(1)\n"
            "lSeedRow2 = oSeed.Selected(2)\n"
            "nSeedIndex = oSeed.ListIndex\n"
            "cSeedDisplay = oSeed.DisplayValue\n"
            "RETURN\n"
            "DEFINE CLASS SeededList AS ListBox\n"
            "    MultiSelect = .T.\n"
            "    PROCEDURE Init\n"
            "        THIS.AddItem('North')\n"
            "        THIS.AddItem('South')\n"
            "        THIS.Selected(1) = .T.\n"
            "        THIS.Selected(2) = .T.\n"
            "        THIS.Selected(2) = .F.\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native ListBox Selected property script should complete: ") + state.message +
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

        check("lselected1", "true");
        check("lselected2", "true");
        check("lselected3", "false");
        check("nindexaftersecondselect", "2");
        check("cdisplayaftersecondselect", "Beta");
        check("lselected2afterclear", "false");
        check("nindexafterclear", "2");
        check("nitemidafterclear", "2");
        check("cvalueafterclear", "Beta");
        check("cdisplayafterclear", "Beta");
        check("ncountafterremove", "2");
        check("lselected1afterremove", "true");
        check("lselected2afterremove", "true");
        check("nindexafterremove", "2");
        check("cdisplayafterremove", "Gamma");
        check("lsetpemselected1", "true");
        check("lselected1aftersetpem", "true");
        check("lsetpemmissing", "false");
        check("lseedrow1", "true");
        check("lseedrow2", "false");
        check("nseedindex", "2");
        check("cseeddisplay", "South");

        expect(state.ole_objects.size() == 2U,
               "native ListBox Selected coverage should register plain and derived list boxes");
        if (state.ole_objects.size() == 2U)
        {
            const auto &plain_list = state.ole_objects[0];
            const auto &seed_list = state.ole_objects[1];

            const auto plain_listindex = plain_list.properties.find("listindex");
            const auto seed_listindex = seed_list.properties.find("listindex");

            expect(plain_list.list_selected.size() == 2U,
                   "plain ListBox Selected coverage should preserve two selection-state slots after removal");
            if (plain_list.list_selected.size() == 2U)
            {
                expect(plain_list.list_selected[0],
                       "plain ListBox Selected coverage should preserve the first selection bit after removal");
                expect(plain_list.list_selected[1],
                       "plain ListBox Selected coverage should shift the later selection bit with the removed row");
            }
            expect(plain_listindex != plain_list.properties.end() &&
                       copperfin::runtime::format_value(plain_listindex->second) == "1",
                   "plain ListBox Selected coverage should keep ListIndex synchronized after the final SETPEM() selection change");
            expect(seed_list.list_selected.size() == 2U &&
                       seed_list.list_selected[0] &&
                       !seed_list.list_selected[1],
                   "derived ListBox Selected coverage should preserve Init-time selection bits");
            expect(seed_listindex != seed_list.properties.end() &&
                       copperfin::runtime::format_value(seed_listindex->second) == "2",
                   "derived ListBox Selected coverage should keep ListIndex synchronized during Init");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_native_listbox_multiselect_property_controls_selection_mode()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_listbox_multiselect";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_listbox_multiselect.prg";
        write_text(
            main_path,
            "oPlain = CREATEOBJECT('ListBox')\n"
            "lPlainHasMultiSelect = PEMSTATUS(oPlain, 'MultiSelect', 1)\n"
            "lPlainMultiSelectReadOnly = PEMSTATUS(oPlain, 'MultiSelect', 5)\n"
            "nPlainBefore = oPlain.MultiSelect\n"
            "xPlainGetPemBefore = GETPEM(oPlain, 'MultiSelect')\n"
            "oPlain.AddItem('Alpha')\n"
            "oPlain.AddItem('Beta')\n"
            "oPlain.AddItem('Gamma')\n"
            "oPlain.Selected(1) = .T.\n"
            "oPlain.Selected(2) = .T.\n"
            "lSelected1Single = oPlain.Selected(1)\n"
            "lSelected2Single = oPlain.Selected(2)\n"
            "nIndexAfterSingle = oPlain.ListIndex\n"
            "cDisplayAfterSingle = oPlain.DisplayValue\n"
            "oPlain.Selected(2) = .F.\n"
            "lSetPemOne = SETPEM(oPlain, 'MultiSelect', 1)\n"
            "nAfterSetPemOne = oPlain.MultiSelect\n"
            "oPlain.Selected(1) = .T.\n"
            "oPlain.Selected(3) = .T.\n"
            "lSelected1Multi = oPlain.Selected(1)\n"
            "lSelected2Multi = oPlain.Selected(2)\n"
            "lSelected3Multi = oPlain.Selected(3)\n"
            "nIndexAfterMulti = oPlain.ListIndex\n"
            "cDisplayAfterMulti = oPlain.DisplayValue\n"
            "oPlain.MultiSelect = 2\n"
            "nAfterDirectTwo = oPlain.MultiSelect\n"
            "oPlain.Selected(2) = .T.\n"
            "nSelectedTwoMode = oPlain.Selected(2)\n"
            "oPlain.MultiSelect = 0\n"
            "nAfterDirectZero = oPlain.MultiSelect\n"
            "lSelected1AfterDisable = oPlain.Selected(1)\n"
            "lSelected2AfterDisable = oPlain.Selected(2)\n"
            "lSelected3AfterDisable = oPlain.Selected(3)\n"
            "nIndexAfterDisable = oPlain.ListIndex\n"
            "cDisplayAfterDisable = oPlain.DisplayValue\n"
            "oPlain.MultiSelect = 1.75\n"
            "nAfterFractional = oPlain.MultiSelect\n"
            "oPlain.MultiSelect = 99\n"
            "nAfterInvalid = oPlain.MultiSelect\n"
            "lPlainAddProperty = ADDPROPERTY(oPlain, 'MultiSelect', .T.)\n"
            "lPlainRemoveProperty = REMOVEPROPERTY(oPlain, 'MultiSelect')\n"
            "nPropMembers = AMEMBERS(aPropMembers, oPlain, 1)\n"
            "lPropHasMultiSelect = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'MULTISELECT'\n"
            "        lPropHasMultiSelect = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oSeed = CREATEOBJECT('SeededList')\n"
            "nSeedMultiSelect = oSeed.MultiSelect\n"
            "lSeedRow1 = oSeed.Selected(1)\n"
            "lSeedRow2 = oSeed.Selected(2)\n"
            "nSeedIndex = oSeed.ListIndex\n"
            "cSeedDisplay = oSeed.DisplayValue\n"
            "RETURN\n"
            "DEFINE CLASS SeededList AS ListBox\n"
            "    MultiSelect = .T.\n"
            "    PROCEDURE Init\n"
            "        THIS.AddItem('North')\n"
            "        THIS.AddItem('South')\n"
            "        THIS.Selected(1) = .T.\n"
            "        THIS.Selected(2) = .T.\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native ListBox MultiSelect property script should complete: ") + state.message +
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

        check("lplainhasmultiselect", "true");
        check("lplainmultiselectreadonly", "false");
        check("nplainbefore", "0");
        check("xplaingetpembefore", "0");
        check("lselected1single", "false");
        check("lselected2single", "true");
        check("nindexaftersingle", "2");
        check("cdisplayaftersingle", "Beta");
        check("lsetpemone", "true");
        check("naftersetpemone", "1");
        check("lselected1multi", "true");
        check("lselected2multi", "false");
        check("lselected3multi", "true");
        check("nindexaftermulti", "3");
        check("cdisplayaftermulti", "Gamma");
        check("nafterdirecttwo", "2");
        check("nselectedtwomode", "true");
        check("nafterdirectzero", "0");
        check("lselected1afterdisable", "false");
        check("lselected2afterdisable", "true");
        check("lselected3afterdisable", "false");
        check("nindexafterdisable", "2");
        check("cdisplayafterdisable", "Beta");
        check("nafterfractional", "1");
        check("nafterinvalid", "0");
        check("lplainaddproperty", "false");
        check("lplainremoveproperty", "false");
        check("lprophasmultiselect", "true");
        check("nseedmultiselect", "1");
        check("lseedrow1", "true");
        check("lseedrow2", "true");
        check("nseedindex", "2");
        check("cseeddisplay", "South");

        expect(state.ole_objects.size() == 2U,
               "native ListBox MultiSelect coverage should register plain and derived list boxes");
        if (state.ole_objects.size() == 2U)
        {
            const auto &plain_list = state.ole_objects[0];
            const auto &seed_list = state.ole_objects[1];
            const auto plain_multiselect = plain_list.properties.find("multiselect");
            const auto plain_listindex = plain_list.properties.find("listindex");
            const auto seed_multiselect = seed_list.properties.find("multiselect");
            const auto seed_listindex = seed_list.properties.find("listindex");

            expect(plain_multiselect != plain_list.properties.end() &&
                       copperfin::runtime::format_value(plain_multiselect->second) == "0",
                   "plain ListBox MultiSelect coverage should preserve the canonical single-selection mode");
            expect(plain_list.list_selected.size() == 3U,
                   "plain ListBox MultiSelect coverage should preserve three selection-state slots");
            if (plain_list.list_selected.size() == 3U)
            {
                expect(!plain_list.list_selected[0] &&
                           plain_list.list_selected[1] &&
                           !plain_list.list_selected[2],
                       "plain ListBox MultiSelect coverage should collapse selection bits back to the active row when disabled");
            }
            expect(plain_listindex != plain_list.properties.end() &&
                       copperfin::runtime::format_value(plain_listindex->second) == "2",
                   "plain ListBox MultiSelect coverage should keep ListIndex synchronized after disabling multiselect");
            expect(seed_multiselect != seed_list.properties.end() &&
                       copperfin::runtime::format_value(seed_multiselect->second) == "1",
                   "derived ListBox MultiSelect coverage should preserve declarative multiple-selection mode");
            expect(seed_list.list_selected.size() == 2U &&
                       seed_list.list_selected[0] &&
                       seed_list.list_selected[1],
                   "derived ListBox MultiSelect coverage should preserve Init-time multiple selection bits");
            expect(seed_listindex != seed_list.properties.end() &&
                       copperfin::runtime::format_value(seed_listindex->second) == "2",
                   "derived ListBox MultiSelect coverage should keep ListIndex synchronized during Init");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_native_list_controls_sorted_property_stays_coherent()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_list_control_sorted";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_list_control_sorted.prg";
        write_text(
            main_path,
            "oPlain = CREATEOBJECT('ComboBox')\n"
            "lPlainHasSorted = PEMSTATUS(oPlain, 'Sorted', 1)\n"
            "lPlainSortedReadOnly = PEMSTATUS(oPlain, 'Sorted', 5)\n"
            "lPlainBefore = oPlain.Sorted\n"
            "xPlainGetPemBefore = GETPEM(oPlain, 'Sorted')\n"
            "oPlain.AddItem('Zulu')\n"
            "oPlain.AddItem('alpha')\n"
            "oPlain.AddItem('Echo')\n"
            "cUnsorted1 = oPlain.List(1)\n"
            "cUnsorted2 = oPlain.List(2)\n"
            "cUnsorted3 = oPlain.List(3)\n"
            "oPlain.ListIndex = 2\n"
            "nListItemIdBeforeSort = oPlain.ListItemID\n"
            "lSetPemSorted = SETPEM(oPlain, 'Sorted', .T.)\n"
            "lAfterSetPemSorted = oPlain.Sorted\n"
            "cSorted1 = oPlain.List(1)\n"
            "cSorted2 = oPlain.List(2)\n"
            "cSorted3 = oPlain.List(3)\n"
            "nIndexAfterSort = oPlain.ListIndex\n"
            "cDisplayAfterSort = oPlain.DisplayValue\n"
            "nListItemIdAfterSort = oPlain.ListItemID\n"
            "nNewIndexAfterSort = oPlain.NewIndex\n"
            "nAddBravo = oPlain.AddItem('Bravo')\n"
            "nNewIndexAfterBravo = oPlain.NewIndex\n"
            "cAfterBravo1 = oPlain.List(1)\n"
            "cAfterBravo2 = oPlain.List(2)\n"
            "cAfterBravo3 = oPlain.List(3)\n"
            "cAfterBravo4 = oPlain.List(4)\n"
            "nIndexAfterBravo = oPlain.ListIndex\n"
            "cDisplayAfterBravo = oPlain.DisplayValue\n"
            "nListItemIdAfterBravo = oPlain.ListItemID\n"
            "nAddDeltaId = oPlain.AddListItem('Delta', 44)\n"
            "nNewIndexAfterDelta = oPlain.NewIndex\n"
            "nDeltaCol2 = oPlain.AddListItem('D', 44, 2)\n"
            "cAfterDelta1 = oPlain.List(1)\n"
            "cAfterDelta2 = oPlain.List(2)\n"
            "cAfterDelta3 = oPlain.List(3)\n"
            "cAfterDelta4 = oPlain.List(4)\n"
            "cAfterDelta5 = oPlain.List(5)\n"
            "cDeltaCol2 = oPlain.List(2, 2)\n"
            "nCountAfterDelta = oPlain.ListCount\n"
            "nIndexAfterDelta = oPlain.ListIndex\n"
            "cDisplayAfterDelta = oPlain.DisplayValue\n"
            "nListItemIdAfterDelta = oPlain.ListItemID\n"
            "oPlain.Sorted = .F.\n"
            "lAfterDirectFalse = oPlain.Sorted\n"
            "cAfterDisable1 = oPlain.List(1)\n"
            "cAfterDisable2 = oPlain.List(2)\n"
            "cAfterDisable3 = oPlain.List(3)\n"
            "cAfterDisable4 = oPlain.List(4)\n"
            "cAfterDisable5 = oPlain.List(5)\n"
            "lPlainAddProperty = ADDPROPERTY(oPlain, 'Sorted', .T.)\n"
            "lPlainRemoveProperty = REMOVEPROPERTY(oPlain, 'Sorted')\n"
            "nPropMembers = AMEMBERS(aPropMembers, oPlain, 1)\n"
            "lPropHasSorted = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'SORTED'\n"
            "        lPropHasSorted = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oSeed = CREATEOBJECT('SeededSortedList')\n"
            "lSeedSorted = oSeed.Sorted\n"
            "cSeed1 = oSeed.List(1)\n"
            "cSeed2 = oSeed.List(2)\n"
            "cSeed3 = oSeed.List(3)\n"
            "lSeedSel1 = oSeed.Selected(1)\n"
            "lSeedSel2 = oSeed.Selected(2)\n"
            "lSeedSel3 = oSeed.Selected(3)\n"
            "nSeedIndex = oSeed.ListIndex\n"
            "cSeedDisplay = oSeed.DisplayValue\n"
            "RETURN\n"
            "DEFINE CLASS SeededSortedList AS ListBox\n"
            "    MultiSelect = .T.\n"
            "    PROCEDURE Init\n"
            "        THIS.AddItem('Zulu')\n"
            "        THIS.AddItem('alpha')\n"
            "        THIS.AddItem('Echo')\n"
            "        THIS.Selected(1) = .T.\n"
            "        THIS.Selected(3) = .T.\n"
            "        THIS.Sorted = .T.\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Sorted list-control script should complete: ") + state.message +
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

        check("lplainhassorted", "true");
        check("lplainsortedreadonly", "false");
        check("lplainbefore", "false");
        check("xplaingetpembefore", "false");
        check("cunsorted1", "Zulu");
        check("cunsorted2", "alpha");
        check("cunsorted3", "Echo");
        check("nlistitemidbeforesort", "2");
        check("lsetpemsorted", "true");
        check("laftersetpemsorted", "true");
        check("csorted1", "Echo");
        check("csorted2", "Zulu");
        check("csorted3", "alpha");
        check("nindexaftersort", "3");
        check("cdisplayaftersort", "alpha");
        check("nlistitemidaftersort", "2");
        check("nnewindexaftersort", "1");
        check("naddbravo", "1");
        check("nnewindexafterbravo", "1");
        check("cafterbravo1", "Bravo");
        check("cafterbravo2", "Echo");
        check("cafterbravo3", "Zulu");
        check("cafterbravo4", "alpha");
        check("nindexafterbravo", "4");
        check("cdisplayafterbravo", "alpha");
        check("nlistitemidafterbravo", "2");
        check("nadddeltaid", "44");
        check("nnewindexafterdelta", "2");
        check("ndeltacol2", "44");
        check("cafterdelta1", "Bravo");
        check("cafterdelta2", "Delta");
        check("cafterdelta3", "Echo");
        check("cafterdelta4", "Zulu");
        check("cafterdelta5", "alpha");
        check("cdeltacol2", "D");
        check("ncountafterdelta", "5");
        check("nindexafterdelta", "5");
        check("cdisplayafterdelta", "alpha");
        check("nlistitemidafterdelta", "2");
        check("lafterdirectfalse", "false");
        check("cafterdisable1", "Bravo");
        check("cafterdisable2", "Delta");
        check("cafterdisable3", "Echo");
        check("cafterdisable4", "Zulu");
        check("cafterdisable5", "alpha");
        check("lplainaddproperty", "false");
        check("lplainremoveproperty", "false");
        check("lprophassorted", "true");
        check("lseedsorted", "true");
        check("cseed1", "Echo");
        check("cseed2", "Zulu");
        check("cseed3", "alpha");
        check("lseedsel1", "true");
        check("lseedsel2", "true");
        check("lseedsel3", "false");
        check("nseedindex", "1");
        check("cseeddisplay", "Echo");

        expect(state.ole_objects.size() == 2U,
               "native Sorted coverage should register plain and derived list controls");
        if (state.ole_objects.size() == 2U)
        {
            const auto &plain_combo = state.ole_objects[0];
            const auto &seed_list = state.ole_objects[1];

            const auto plain_sorted = plain_combo.properties.find("sorted");
            const auto plain_newindex = plain_combo.properties.find("newindex");
            const auto plain_listindex = plain_combo.properties.find("listindex");
            const auto seed_sorted = seed_list.properties.find("sorted");
            const auto seed_listindex = seed_list.properties.find("listindex");

            expect(plain_sorted != plain_combo.properties.end() &&
                       copperfin::runtime::format_value(plain_sorted->second) == "false",
                   "plain ComboBox Sorted coverage should preserve the built-in property after direct disable");
            expect(plain_newindex != plain_combo.properties.end() &&
                       copperfin::runtime::format_value(plain_newindex->second) == "2",
                   "plain ComboBox Sorted coverage should keep NewIndex synchronized with the last sorted insertion");
            expect(plain_listindex != plain_combo.properties.end() &&
                       copperfin::runtime::format_value(plain_listindex->second) == "5",
                   "plain ComboBox Sorted coverage should keep ListIndex synchronized with the active item after sorted insertions");
            expect(plain_combo.list_rows.size() == 5U &&
                       copperfin::runtime::format_value(plain_combo.list_rows[0][0]) == "Bravo" &&
                       copperfin::runtime::format_value(plain_combo.list_rows[1][0]) == "Delta" &&
                       plain_combo.list_rows[1].size() >= 2U &&
                       copperfin::runtime::format_value(plain_combo.list_rows[1][1]) == "D" &&
                       copperfin::runtime::format_value(plain_combo.list_rows[4][0]) == "alpha",
                   "plain ComboBox Sorted coverage should preserve sorted row order and multicolumn metadata");
            expect(plain_combo.list_selected.size() == 5U &&
                       !plain_combo.list_selected[0] &&
                       !plain_combo.list_selected[1] &&
                       !plain_combo.list_selected[2] &&
                       !plain_combo.list_selected[3] &&
                       plain_combo.list_selected[4],
                   "plain ComboBox Sorted coverage should keep the active selection bit attached to the same item as rows reorder");
            expect(seed_sorted != seed_list.properties.end() &&
                       copperfin::runtime::format_value(seed_sorted->second) == "true",
                   "derived ListBox Sorted coverage should preserve Init-time sorted enablement");
            expect(seed_listindex != seed_list.properties.end() &&
                       copperfin::runtime::format_value(seed_listindex->second) == "1",
                   "derived ListBox Sorted coverage should keep ListIndex synchronized after sorted reordering");
            expect(seed_list.list_rows.size() == 3U &&
                       copperfin::runtime::format_value(seed_list.list_rows[0][0]) == "Echo" &&
                       copperfin::runtime::format_value(seed_list.list_rows[1][0]) == "Zulu" &&
                       copperfin::runtime::format_value(seed_list.list_rows[2][0]) == "alpha",
                   "derived ListBox Sorted coverage should preserve case-sensitive sorted row order");
            expect(seed_list.list_selected.size() == 3U &&
                       seed_list.list_selected[0] &&
                       seed_list.list_selected[1] &&
                       !seed_list.list_selected[2],
                   "derived ListBox Sorted coverage should move multiselect bits with their rows during sorting");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_native_listbox_moverbars_property_stays_gated_and_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_listbox_moverbars";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_listbox_moverbars.prg";
        write_text(
            main_path,
            "oPlain = CREATEOBJECT('ListBox')\n"
            "oCombo = CREATEOBJECT('ComboBox')\n"
            "lPlainHasMoverBars = PEMSTATUS(oPlain, 'MoverBars', 1)\n"
            "lPlainMoverBarsReadOnly = PEMSTATUS(oPlain, 'MoverBars', 5)\n"
            "lPlainBefore = oPlain.MoverBars\n"
            "xPlainGetPemBefore = GETPEM(oPlain, 'MoverBars')\n"
            "lComboHasMoverBars = PEMSTATUS(oCombo, 'MoverBars', 1)\n"
            "oPlain.MoverBars = .T.\n"
            "lPlainAfterDirect = oPlain.MoverBars\n"
            "lSetPemFalse = SETPEM(oPlain, 'MoverBars', .F.)\n"
            "lAfterSetPemFalse = oPlain.MoverBars\n"
            "oPlain.RowSourceType = 1\n"
            "lSetPemRowOne = SETPEM(oPlain, 'MoverBars', .T.)\n"
            "lRowOne = oPlain.MoverBars\n"
            "oPlain.RowSourceType = 5\n"
            "lUnsupportedRead = oPlain.MoverBars\n"
            "lUnsupportedSetPem = SETPEM(oPlain, 'MoverBars', .T.)\n"
            "oPlain.MoverBars = .T.\n"
            "lUnsupportedAfterWrites = oPlain.MoverBars\n"
            "oPlain.RowSourceType = 0\n"
            "lAfterReturnToValue = oPlain.MoverBars\n"
            "lSetPemAfterReturn = SETPEM(oPlain, 'MoverBars', .T.)\n"
            "lAfterRestore = oPlain.MoverBars\n"
            "lPlainAddProperty = ADDPROPERTY(oPlain, 'MoverBars', .F.)\n"
            "lPlainRemoveProperty = REMOVEPROPERTY(oPlain, 'MoverBars')\n"
            "nPropMembers = AMEMBERS(aPropMembers, oPlain, 1)\n"
            "lPropHasMoverBars = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'MOVERBARS'\n"
            "        lPropHasMoverBars = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedMoverList')\n"
            "lDerived = oDerived.MoverBars\n"
            "RETURN\n"
            "DEFINE CLASS DerivedMoverList AS ListBox\n"
            "    PROCEDURE Init\n"
            "        THIS.RowSourceType = 1\n"
            "        THIS.MoverBars = .T.\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native MoverBars list-control script should complete: ") + state.message +
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

        check("lplainhasmoverbars", "true");
        check("lplainmoverbarsreadonly", "false");
        check("lplainbefore", "false");
        check("xplaingetpembefore", "false");
        check("lcombohasmoverbars", "false");
        check("lplainafterdirect", "true");
        check("lsetpemfalse", "true");
        check("laftersetpemfalse", "false");
        check("lsetpemrowone", "true");
        check("lrowone", "true");
        check("lunsupportedread", "false");
        check("lunsupportedsetpem", "false");
        check("lunsupportedafterwrites", "false");
        check("lafterreturntovalue", "false");
        check("lsetpemafterreturn", "true");
        check("lafterrestore", "true");
        check("lplainaddproperty", "false");
        check("lplainremoveproperty", "false");
        check("lprophasmoverbars", "true");
        check("lderived", "true");

        expect(state.ole_objects.size() == 3U,
               "native MoverBars coverage should register plain ListBox, ComboBox, and derived ListBox");
        if (state.ole_objects.size() == 3U)
        {
            const auto &plain_list = state.ole_objects[0];
            const auto &combo = state.ole_objects[1];
            const auto &derived_list = state.ole_objects[2];
            const auto plain_moverbars = plain_list.properties.find("moverbars");
            const auto combo_moverbars = combo.properties.find("moverbars");
            const auto derived_moverbars = derived_list.properties.find("moverbars");

            expect(plain_moverbars != plain_list.properties.end() &&
                       copperfin::runtime::format_value(plain_moverbars->second) == "true",
                   "plain ListBox MoverBars coverage should preserve the final supported write");
            expect(combo_moverbars == combo.properties.end(),
                   "ComboBox MoverBars coverage should not seed a ListBox-only property");
            expect(derived_moverbars != derived_list.properties.end() &&
                       copperfin::runtime::format_value(derived_moverbars->second) == "true",
                   "derived ListBox MoverBars coverage should preserve Init-time enablement");
        }

        fs::remove_all(temp_root, ignored);
    }

}
