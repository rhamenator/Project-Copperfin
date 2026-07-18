#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_list_controls_itemid_index_conversion_methods_stay_coherent()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root =
            fs::temp_directory_path() / "copperfin_native_list_control_itemid_index_conversion";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_list_control_itemid_index_conversion.prg";
        write_text(
            main_path,
            "oPlain = CREATEOBJECT('ComboBox')\n"
            "lHasIndexToItemId = PEMSTATUS(oPlain, 'IndexToItemID', 1)\n"
            "lGetIndexToItemId = GETPEM(oPlain, 'IndexToItemID')\n"
            "lHasItemIdToIndex = PEMSTATUS(oPlain, 'ItemIDToIndex', 1)\n"
            "lGetItemIdToIndex = GETPEM(oPlain, 'ItemIDToIndex')\n"
            "nMethodCount = AMEMBERS(aMethods, oPlain, 2)\n"
            "nHasIndexToItemId = ASCAN(aMethods, 'INDEXTOITEMID')\n"
            "nHasItemIdToIndex = ASCAN(aMethods, 'ITEMIDTOINDEX')\n"
            "oPlain.AddListItem('Zulu', 10)\n"
            "oPlain.AddListItem('Alpha', 20)\n"
            "oPlain.AddListItem('Echo', 30)\n"
            "nIndex1IdBeforeSort = oPlain.IndexToItemID(1)\n"
            "nIndex2IdBeforeSort = oPlain.IndexToItemID(2)\n"
            "nItem20IndexBeforeSort = oPlain.ItemIDToIndex(20)\n"
            "oPlain.Sorted = .T.\n"
            "nIndex1IdAfterSort = oPlain.IndexToItemID(1)\n"
            "nIndex2IdAfterSort = oPlain.IndexToItemID(2)\n"
            "nIndex3IdAfterSort = oPlain.IndexToItemID(3)\n"
            "nItem10IndexAfterSort = oPlain.ItemIDToIndex(10)\n"
            "nItem20IndexAfterSort = oPlain.ItemIDToIndex(20)\n"
            "nItem30IndexAfterSort = oPlain.ItemIDToIndex(30)\n"
            "oPlain.RemoveListItem(30)\n"
            "nIndex1IdAfterRemove = oPlain.IndexToItemID(1)\n"
            "nIndex2IdAfterRemove = oPlain.IndexToItemID(2)\n"
            "nMissingIndexAfterRemove = oPlain.ItemIDToIndex(30)\n"
            "oSeed = CREATEOBJECT('SeededList')\n"
            "nSeedIndex1Id = oSeed.IndexToItemID(1)\n"
            "nSeedIndex2Id = oSeed.IndexToItemID(2)\n"
            "nSeedItem100Index = oSeed.ItemIDToIndex(100)\n"
            "nSeedItem200Index = oSeed.ItemIDToIndex(200)\n"
            "RETURN\n"
            "DEFINE CLASS SeededList AS ListBox\n"
            "    PROCEDURE Init\n"
            "        THIS.AddListItem('North', 200)\n"
            "        THIS.AddListItem('East', 100)\n"
            "        THIS.Sorted = .T.\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native IndexToItemID/ItemIDToIndex list-control script should complete: ") +
                   state.message + " @line=" + std::to_string(state.location.line));

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

        check("lhasindextoitemid", "true");
        check("lgetindextoitemid", "true");
        check("lhasitemidtoindex", "true");
        check("lgetitemidtoindex", "true");
        check("nindex1idbeforesort", "10");
        check("nindex2idbeforesort", "20");
        check("nitem20indexbeforesort", "2");
        check("nindex1idaftersort", "20");
        check("nindex2idaftersort", "30");
        check("nindex3idaftersort", "10");
        check("nitem10indexaftersort", "3");
        check("nitem20indexaftersort", "1");
        check("nitem30indexaftersort", "2");
        check("nindex1idafterremove", "20");
        check("nindex2idafterremove", "10");
        check("nmissingindexafterremove", "0");
        check("nseedindex1id", "100");
        check("nseedindex2id", "200");
        check("nseeditem100index", "1");
        check("nseeditem200index", "2");

        const auto plain_has_index_to_item_id = state.globals.find("nhasindextoitemid");
        expect(plain_has_index_to_item_id != state.globals.end(),
               "nHasIndexToItemId variable should be present");
        if (plain_has_index_to_item_id != state.globals.end())
        {
            expect(copperfin::runtime::format_value(plain_has_index_to_item_id->second) != "0",
                   "AMEMBERS(..., 2) should expose the native IndexToItemID builtin for ComboBox");
        }

        const auto plain_has_item_id_to_index = state.globals.find("nhasitemidtoindex");
        expect(plain_has_item_id_to_index != state.globals.end(),
               "nHasItemIdToIndex variable should be present");
        if (plain_has_item_id_to_index != state.globals.end())
        {
            expect(copperfin::runtime::format_value(plain_has_item_id_to_index->second) != "0",
                   "AMEMBERS(..., 2) should expose the native ItemIDToIndex builtin for ComboBox");
        }

        expect(state.ole_objects.size() == 2U,
               "native IndexToItemID/ItemIDToIndex coverage should register plain and derived list controls");
        if (state.ole_objects.size() == 2U)
        {
            const auto& plain_combo = state.ole_objects[0];
            const auto& seed_list = state.ole_objects[1];

            expect(plain_combo.collection_item_keys.size() == 2U &&
                       plain_combo.collection_item_keys[0] == "20" &&
                       plain_combo.collection_item_keys[1] == "10",
                   "plain ComboBox IndexToItemID/ItemIDToIndex coverage should preserve item IDs across sort and removal");
            expect(seed_list.collection_item_keys.size() == 2U &&
                       seed_list.collection_item_keys[0] == "100" &&
                       seed_list.collection_item_keys[1] == "200",
                   "derived ListBox IndexToItemID/ItemIDToIndex coverage should preserve sorted Init-time item ID order");
        }

        fs::remove_all(temp_root, ignored);
    }

}
