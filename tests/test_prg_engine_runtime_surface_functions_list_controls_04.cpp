#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_list_controls_file_rowsource_materializes_masks()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_list_control_file_rowsource";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root / "nested");
        write_text(temp_root / "alpha.txt", "alpha");
        write_text(temp_root / "Bravo.TXT", "bravo");
        write_text(temp_root / "ignored.prg", "RETURN\n");
        fs::create_directories(temp_root / "folder.txt");
        write_text(temp_root / "nested" / "child.txt", "child");
        const fs::path unicode_file = temp_root / copperfin::platform::path_from_utf8_string("caf\xC3\xA9.txt");
        write_text(unicode_file, "cafe");

        const fs::path main_path = temp_root / "file_rowsource.prg";
        write_text(
            main_path,
            "oCombo = CREATEOBJECT('ComboBox')\n"
            "oCombo.RowSourceType = 7\n"
            "oCombo.RowSource = '*.txt'\n"
            "oCombo.Requery()\n"
            "nComboCount = oCombo.ListCount\n"
            "cComboFirst = oCombo.List(1)\n"
            "cComboLast = oCombo.List(oCombo.ListCount)\n"
            "oList = CREATEOBJECT('ListBox')\n"
            "oList.RowSourceType = 7\n"
            "oList.RowSource = 'nested/child.?xt'\n"
            "oList.Requery()\n"
            "nListCount = oList.ListCount\n"
            "cListOnly = oList.List(1)\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path, temp_root));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native file RowSource script should complete: ") + state.message +
                   " @line=" + std::to_string(state.location.line));

        const auto check = [&](const std::string &name, const std::string &expected)
        {
            const auto found = state.globals.find(name);
            expect(found != state.globals.end(), name + " should be captured");
            if (found != state.globals.end())
            {
                expect(copperfin::runtime::format_value(found->second) == expected,
                       name + " expected '" + expected + "' got '" +
                           copperfin::runtime::format_value(found->second) + "'");
            }
        };

        check("ncombocount", "3");
        check("ccombofirst", "alpha.txt");
        check("ccombolast", "caf\xC3\xA9.txt");
        check("nlistcount", "1");
        check("clistonly", "child.txt");
        expect(state.ole_objects.size() == 2U,
               "file RowSource coverage should register ComboBox and ListBox objects");
        if (state.ole_objects.size() == 2U)
        {
            expect(state.ole_objects[0].collection_items.size() == 3U &&
                       state.ole_objects[0].list_rows.size() == 3U,
                   "file RowSource ComboBox should materialize matching regular files only");
            expect(state.ole_objects[1].collection_items.size() == 1U &&
                       copperfin::runtime::format_value(state.ole_objects[1].collection_items.front()) == "child.txt",
                   "file RowSource ListBox should materialize an explicit directory mask");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_native_list_controls_field_structure_rowsource_materializes_fields()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_list_control_structure_rowsource";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "structure_rowsource.prg";
        write_text(
            main_path,
            "CREATE CURSOR lookup (NAME C(12), AMOUNT N(8,2), ACTIVE L)\n"
            "oCombo = CREATEOBJECT('ComboBox')\n"
            "oCombo.RowSourceType = 8\n"
            "oCombo.RowSource = ''\n"
            "oCombo.Requery()\n"
            "nComboCount = oCombo.ListCount\n"
            "cComboFirst = oCombo.List(1)\n"
            "cComboLast = oCombo.List(oCombo.ListCount)\n"
            "oList = CREATEOBJECT('ListBox')\n"
            "oList.RowSourceType = 8\n"
            "oList.RowSource = 'lookup'\n"
            "oList.Requery()\n"
            "nListCount = oList.ListCount\n"
            "cListFirst = oList.List(1)\n"
            "cListLast = oList.List(oList.ListCount)\n"
            "oMissing = CREATEOBJECT('ListBox')\n"
            "oMissing.RowSourceType = 8\n"
            "oMissing.RowSource = 'missing_alias'\n"
            "oMissing.Requery()\n"
            "nMissingCount = oMissing.ListCount\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path, temp_root));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native field-structure RowSource script should complete: ") + state.message +
                   " @line=" + std::to_string(state.location.line));

        const auto check = [&](const std::string &name, const std::string &expected)
        {
            const auto found = state.globals.find(name);
            expect(found != state.globals.end(), name + " should be captured");
            if (found != state.globals.end())
            {
                expect(copperfin::runtime::format_value(found->second) == expected,
                       name + " expected '" + expected + "' got '" +
                           copperfin::runtime::format_value(found->second) + "'");
            }
        };

        check("ncombocount", "3");
        check("ccombofirst", "NAME");
        check("ccombolast", "ACTIVE");
        check("nlistcount", "3");
        check("clistfirst", "NAME");
        check("clistlast", "ACTIVE");
        check("nmissingcount", "0");
        expect(state.ole_objects.size() == 3U,
               "field-structure RowSource coverage should register both populated and missing-source controls");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].list_rows.size() == 3U &&
                       state.ole_objects[1].list_rows.size() == 3U &&
                       state.ole_objects[2].list_rows.empty(),
                   "field-structure RowSource should materialize descriptor order and empty missing sources");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_native_list_controls_collection_rowsource_materializes_members()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_list_control_collection_rowsource";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "collection_rowsource.prg";
        write_text(
            main_path,
            "colItems = CREATEOBJECT('Collection')\n"
            "colItems.Add('Alpha', 'alpha')\n"
            "colItems.Add('Beta')\n"
            "oForm = CREATEOBJECT('Form')\n"
            "oForm.Caption = 'Window'\n"
            "oForm.Name = 'frmOne'\n"
            "colItems.Add(oForm, 'form')\n"
            "oCombo = CREATEOBJECT('ComboBox')\n"
            "oCombo.RowSourceType = 10\n"
            "oCombo.RowSource = 'colItems'\n"
            "oCombo.Requery()\n"
            "nComboCount = oCombo.ListCount\n"
            "cComboScalar = oCombo.List(1)\n"
            "cComboObject = oCombo.List(3)\n"
            "oList = CREATEOBJECT('ListBox')\n"
            "oList.RowSourceType = 10\n"
            "oList.RowSource = 'colItems, Caption, Name'\n"
            "oList.Requery()\n"
            "nListCount = oList.ListCount\n"
            "cListScalar = oList.List(1, 1)\n"
            "cListScalarProperty = oList.List(1, 2)\n"
            "cListObjectCaption = oList.List(3, 1)\n"
            "cListObjectName = oList.List(3, 2)\n"
            "oMissing = CREATEOBJECT('ListBox')\n"
            "oMissing.RowSourceType = 10\n"
            "oMissing.RowSource = 'notACollection'\n"
            "oMissing.Requery()\n"
            "nMissingCount = oMissing.ListCount\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path, temp_root));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native collection RowSource script should complete: ") + state.message +
                   " @line=" + std::to_string(state.location.line));

        const auto check = [&](const std::string &name, const std::string &expected)
        {
            const auto found = state.globals.find(name);
            expect(found != state.globals.end(), name + " should be captured");
            if (found != state.globals.end())
            {
                expect(copperfin::runtime::format_value(found->second) == expected,
                       name + " expected '" + expected + "' got '" +
                           copperfin::runtime::format_value(found->second) + "'");
            }
        };

        check("ncombocount", "3");
        check("ccomboscalar", "Alpha");
        check("ccomboobject", "(Object)");
        check("nlistcount", "3");
        check("clistscalar", "Alpha");
        check("clistscalarproperty", "");
        check("clistobjectcaption", "Window");
        check("clistobjectname", "Form");
        check("nmissingcount", "0");
        expect(state.ole_objects.size() == 5U,
               "collection RowSource coverage should register collection, form, and three controls");
        if (state.ole_objects.size() == 5U)
        {
            expect(state.ole_objects[2].list_rows.size() == 3U &&
                       state.ole_objects[3].list_rows.size() == 3U &&
                       state.ole_objects[4].list_rows.empty(),
                   "collection RowSource should preserve collection order and empty missing sources");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_native_list_controls_itemdata_stays_coherent()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_list_control_itemdata";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_list_control_itemdata.prg";
        write_text(
            main_path,
            "oPlain = CREATEOBJECT('ComboBox')\n"
            "lHasItemData = PEMSTATUS(oPlain, 'ItemData', 1)\n"
            "lItemDataReadOnly = PEMSTATUS(oPlain, 'ItemData', 5)\n"
            "nPropertyCount = AMEMBERS(aProperties, oPlain, 1)\n"
            "lPropertyHasItemData = .F.\n"
            "FOR i = 1 TO nPropertyCount\n"
            "    IF UPPER(aProperties[i]) == 'ITEMDATA'\n"
            "        lPropertyHasItemData = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "nMethodCount = AMEMBERS(aMethods, oPlain, 2)\n"
            "nItemDataMethodIndex = ASCAN(aMethods, 'ITEMDATA')\n"
            "oPlain.AddItem('Zulu')\n"
            "oPlain.ItemData(1) = 30\n"
            "oPlain.AddItem('Alpha')\n"
            "oPlain.ItemData[2] = 10\n"
            "oPlain.AddItem('Echo')\n"
            "lSetPem = SETPEM(oPlain, 'ItemData(3)', 20)\n"
            "nBeforeSort1 = oPlain.ItemData(1)\n"
            "nBeforeSort2 = oPlain.ItemData(2)\n"
            "nBeforeSort3 = GETPEM(oPlain, 'ItemData[3]')\n"
            "oPlain.Sorted = .T.\n"
            "nAfterSort1 = oPlain.ItemData(1)\n"
            "nAfterSort2 = oPlain.ItemData(2)\n"
            "nAfterSort3 = oPlain.ItemData(3)\n"
            "oPlain.RemoveItem(2)\n"
            "nAfterRemove1 = oPlain.ItemData(1)\n"
            "nAfterRemove2 = oPlain.ItemData(2)\n"
            "nInvalidItemData = oPlain.ItemData(9)\n"
            "oPlain.Clear()\n"
            "nAfterClear = oPlain.ItemData(1)\n"
            "oSeed = CREATEOBJECT('SeededList')\n"
            "nSeed1 = oSeed.ItemData(1)\n"
            "nSeed2 = oSeed.ItemData(2)\n"
            "RETURN\n"
            "DEFINE CLASS SeededList AS ListBox\n"
            "    PROCEDURE Init\n"
            "        THIS.AddItem('North')\n"
            "        THIS.ItemData(1) = 100\n"
            "        THIS.AddItem('East')\n"
            "        THIS.ItemData(2) = 200\n"
            "        THIS.Sorted = .T.\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native ItemData list-control script should complete: ") + state.message +
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

        check("lhasitemdata", "true");
        check("litemdatareadonly", "false");
        check("lpropertyhasitemdata", "true");
        check("nitemdatamethodindex", "0");
        check("lsetpem", "true");
        check("nbeforesort1", "30");
        check("nbeforesort2", "10");
        check("nbeforesort3", "20");
        check("naftersort1", "10");
        check("naftersort2", "20");
        check("naftersort3", "30");
        check("nafterremove1", "10");
        check("nafterremove2", "30");
        check("ninvaliditemdata", "0");
        check("nafterclear", "0");
        check("nseed1", "200");
        check("nseed2", "100");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_list_controls_topitemid_stays_coherent()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_list_control_topitemid";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_list_control_topitemid.prg";
        write_text(
            main_path,
            "oCombo = CREATEOBJECT('ComboBox')\n"
            "lComboHasTopItemId = PEMSTATUS(oCombo, 'TopItemID', 1)\n"
            "lComboTopItemIdReadOnly = PEMSTATUS(oCombo, 'TopItemID', 5)\n"
            "nComboMethodCount = AMEMBERS(aComboMethods, oCombo, 2)\n"
            "nComboMethodIndex = ASCAN(aComboMethods, 'TOPITEMID')\n"
            "oCombo.AddListItem('Zulu', 30)\n"
            "nComboTopItemId = oCombo.TopItemID\n"
            "lComboSetTopItemId = SETPEM(oCombo, 'TopItemID', 30)\n"
            "nComboTopAfterSet = oCombo.TopItemID\n"
            "oList = CREATEOBJECT('ListBox')\n"
            "lListHasTopItemId = PEMSTATUS(oList, 'TopItemID', 1)\n"
            "lListTopItemIdReadOnly = PEMSTATUS(oList, 'TopItemID', 5)\n"
            "nListPropertyCount = AMEMBERS(aListProperties, oList, 1)\n"
            "lListPropertyHasTopItemId = ASCAN(aListProperties, 'TOPITEMID') > 0\n"
            "oList.AddListItem('Zulu', 30)\n"
            "oList.AddListItem('Alpha', 10)\n"
            "oList.AddListItem('Echo', 20)\n"
            "nListTopInitial = oList.TopItemID\n"
            "lListSetTopItemId = SETPEM(oList, 'TopItemID', 20)\n"
            "nListTopAfterSet = GETPEM(oList, 'TopItemID')\n"
            "lListSetInvalidTopItemId = SETPEM(oList, 'TopItemID', 999)\n"
            "nListTopAfterInvalidSet = oList.TopItemID\n"
            "oList.Sorted = .T.\n"
            "nListTopAfterSort = oList.TopItemID\n"
            "oList.RemoveListItem(20)\n"
            "nListTopAfterRemove = oList.TopItemID\n"
            "oList.AddItem('Bravo')\n"
            "nListTopAfterAdd = oList.TopItemID\n"
            "oList.Clear()\n"
            "nListTopAfterClear = oList.TopItemID\n"
            "oSeed = CREATEOBJECT('SeededList')\n"
            "nSeedTopItemId = oSeed.TopItemID\n"
            "RETURN\n"
            "DEFINE CLASS SeededList AS ListBox\n"
            "    PROCEDURE Init\n"
            "        THIS.AddListItem('North', 200)\n"
            "        THIS.AddListItem('East', 100)\n"
            "        THIS.TopItemID = 100\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native TopItemID list-control script should complete: ") + state.message +
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

        check("lcombohastopitemid", "true");
        check("lcombotopitemidreadonly", "true");
        check("ncombotopitemid", "30");
        check("lcombosettopitemid", "false");
        check("ncombotopafterset", "30");
        check("ncombomethodindex", "0");
        check("llisthastopitemid", "true");
        check("llisttopitemidreadonly", "false");
        check("llistpropertyhastopitemid", "true");
        check("nlisttopinitial", "30");
        check("llistsettopitemid", "true");
        check("nlisttopafterset", "20");
        check("llistsetinvalidtopitemid", "false");
        check("nlisttopafterinvalidset", "20");
        check("nlisttopaftersort", "20");
        check("nlisttopafterremove", "10");
        check("nlisttopafteradd", "10");
        check("nlisttopafterclear", "0");
        check("nseedtopitemid", "100");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_list_controls_topindex_stays_coherent()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_list_control_topindex";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_list_control_topindex.prg";
        write_text(
            main_path,
            "oCombo = CREATEOBJECT('ComboBox')\n"
            "lComboHasTopIndex = PEMSTATUS(oCombo, 'TopIndex', 1)\n"
            "lComboTopIndexReadOnly = PEMSTATUS(oCombo, 'TopIndex', 5)\n"
            "oCombo.AddListItem('Zulu', 30)\n"
            "oCombo.AddListItem('Alpha', 10)\n"
            "nComboTopIndex = oCombo.TopIndex\n"
            "lComboSetTopIndex = SETPEM(oCombo, 'TopIndex', 2)\n"
            "nComboTopAfterSet = oCombo.TopIndex\n"
            "oList = CREATEOBJECT('ListBox')\n"
            "lListHasTopIndex = PEMSTATUS(oList, 'TopIndex', 1)\n"
            "lListTopIndexReadOnly = PEMSTATUS(oList, 'TopIndex', 5)\n"
            "oList.AddListItem('Zulu', 30)\n"
            "oList.AddListItem('Alpha', 10)\n"
            "oList.AddListItem('Echo', 20)\n"
            "nListTopInitial = oList.TopIndex\n"
            "lListSetTopIndex = SETPEM(oList, 'TopIndex', 3)\n"
            "nListTopAfterSet = GETPEM(oList, 'TopIndex')\n"
            "nListTopItemAfterSet = oList.TopItemID\n"
            "lListSetInvalidTopIndex = SETPEM(oList, 'TopIndex', 99)\n"
            "nListTopAfterInvalidSet = oList.TopIndex\n"
            "oList.Sorted = .T.\n"
            "nListTopAfterSort = oList.TopIndex\n"
            "nListTopItemAfterSort = oList.TopItemID\n"
            "oList.RemoveListItem(20)\n"
            "nListTopAfterRemove = oList.TopIndex\n"
            "nListTopItemAfterRemove = oList.TopItemID\n"
            "oList.Clear()\n"
            "nListTopAfterClear = oList.TopIndex\n"
            "oSeed = CREATEOBJECT('SeededList')\n"
            "nSeedTopIndex = oSeed.TopIndex\n"
            "nSeedTopItemId = oSeed.TopItemID\n"
            "RETURN\n"
            "DEFINE CLASS SeededList AS ListBox\n"
            "    PROCEDURE Init\n"
            "        THIS.AddListItem('North', 200)\n"
            "        THIS.AddListItem('East', 100)\n"
            "        THIS.TopIndex = 2\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native TopIndex list-control script should complete: ") + state.message +
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

        check("lcombohastopindex", "true");
        check("lcombotopindexreadonly", "true");
        check("ncombotopindex", "1");
        check("lcombosettopindex", "false");
        check("ncombotopafterset", "1");
        check("llisthastopindex", "true");
        check("llisttopindexreadonly", "false");
        check("nlisttopinitial", "1");
        check("llistsettopindex", "true");
        check("nlisttopafterset", "3");
        check("nlisttopitemafterset", "20");
        check("llistsetinvalidtopindex", "false");
        check("nlisttopafterinvalidset", "3");
        check("nlisttopaftersort", "2");
        check("nlisttopitemaftersort", "20");
        check("nlisttopafterremove", "1");
        check("nlisttopitemafterremove", "10");
        check("nlisttopafterclear", "0");
        check("nseedtopindex", "2");
        check("nseedtopitemid", "100");

        fs::remove_all(temp_root, ignored);
    }

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

    void test_native_listbox_moveitem_preserves_row_identity_and_state()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_listbox_moveitem";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "moveitem.prg";
        write_text(
            main_path,
            "oList = CREATEOBJECT('ListBox')\n"
            "oList.AddItem('Alpha')\n"
            "oList.AddItem('Bravo')\n"
            "oList.AddItem('Charlie')\n"
            "oList.ItemData(2) = 42\n"
            "oList.Selected(2) = .T.\n"
            "oList.ListIndex = 2\n"
            "oList.TopIndex = 2\n"
            "oList.MoveItem(2, -1)\n"
            "cAfterUpFirst = oList.List(1)\n"
            "cAfterUpSecond = oList.List(2)\n"
            "nAfterUpData = oList.ItemData(1)\n"
            "lAfterUpSelected = oList.Selected(1)\n"
            "nAfterUpListIndex = oList.ListIndex\n"
            "nAfterUpTopIndex = oList.TopIndex\n"
            "nAfterUpTopItemID = oList.TopItemID\n"
            "oList.MoveItem(1, 99)\n"
            "cAfterDownLast = oList.List(oList.ListCount)\n"
            "nAfterDownListIndex = oList.ListIndex\n"
            "nAfterDownTopIndex = oList.TopIndex\n"
            "nAfterDownTopItemID = oList.TopItemID\n"
            "oList.RowSourceType = 5\n"
            "oList.MoveItem(1, 1)\n"
            "cUnsupportedFirst = oList.List(1)\n"
            "oValueList = CREATEOBJECT('ListBox')\n"
            "oValueList.RowSourceType = 1\n"
            "oValueList.RowSource = 'One,Two,Three'\n"
            "oValueList.Requery()\n"
            "oValueList.MoveItem(1, 2)\n"
            "cValueMovedLast = oValueList.List(3)\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path, temp_root));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native ListBox MoveItem script should complete: ") + state.message +
                   " @line=" + std::to_string(state.location.line));

        const auto check = [&](const std::string& name, const std::string& expected)
        {
            const auto found = state.globals.find(name);
            expect(found != state.globals.end(), name + " should be captured");
            if (found != state.globals.end())
            {
                expect(copperfin::runtime::format_value(found->second) == expected,
                       name + " expected '" + expected + "' got '" +
                           copperfin::runtime::format_value(found->second) + "'");
            }
        };

        check("cafterupfirst", "Bravo");
        check("cafterupsecond", "Alpha");
        check("nafterupdata", "42");
        check("lafterupselected", "true");
        check("nafteruplistindex", "1");
        check("nafteruptopindex", "1");
        check("nafteruptopitemid", "2");
        check("cafterdownlast", "Bravo");
        check("nafterdownlistindex", "3");
        check("nafterdowntopindex", "3");
        check("nafterdowntopitemid", "2");
        check("cunsupportedfirst", "Alpha");
        check("cvaluemovedlast", "One");
        expect(state.ole_objects.size() == 2U,
               "native ListBox MoveItem coverage should register two list controls");
        if (state.ole_objects.size() == 2U)
        {
            expect(state.ole_objects.front().collection_item_keys.size() == 3U &&
                       state.ole_objects.front().collection_item_keys[2] == "2",
                   "MoveItem should preserve the moved row's stable item ID at its new slot");
            expect(state.ole_objects.back().collection_item_keys.size() == 3U &&
                       state.ole_objects.back().collection_item_keys[2] == "1",
                   "RowSourceType 1 MoveItem should preserve the moved row's stable item ID");
        }

        fs::remove_all(temp_root, ignored);
    }

}
