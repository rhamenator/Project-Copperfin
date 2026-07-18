#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_createobject_and_newobject_instantiate_same_prg_collection_native_class()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_collection_class";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_collection_class.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('WorkerCollection')\n"
            "oLeaf = NEWOBJECT('WorkerCollection')\n"
            "cCreateLabel = oCreate.cLabel\n"
            "cLeafLabel = oLeaf.cLabel\n"
            "nCreateCountBefore = oCreate.Count\n"
            "nLeafCountBefore = oLeaf.Count\n"
            "oCreate.Add('alpha')\n"
            "oCreate.Add('beta')\n"
            "oLeaf.Add('gamma')\n"
            "nCreateCountAfter = oCreate.Count\n"
            "nLeafCountAfter = oLeaf.Count\n"
            "cCreateDescribe = oCreate.Describe('prefix')\n"
            "cLeafDescribe = oLeaf.Describe('leaf')\n"
            "cCreateBaseClass = oCreate.BaseClass\n"
            "cLeafBaseClass = oLeaf.BaseClass\n"
            "cCreateClass = oCreate.Class\n"
            "cLeafClass = oLeaf.Class\n"
            "lCreateHasDescribe = GETPEM(oCreate, 'Describe')\n"
            "lLeafHasDescribe = GETPEM(oLeaf, 'Describe')\n"
            "lCreateHasBaseClass = PEMSTATUS(oCreate, 'BaseClass', 1)\n"
            "lLeafHasBaseClass = PEMSTATUS(oLeaf, 'BaseClass', 1)\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 174)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS WorkerCollection AS Collection\n"
            "    cLabel = 'workercollection'\n"
            "    FUNCTION Describe\n"
            "        LPARAMETERS tcPrefix\n"
            "        RETURN tcPrefix + ':' + THIS.cLabel\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Collection-base class script should complete: ") + state.message +
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

        check("ccreatelabel", "workercollection");
        check("cleaflabel", "workercollection");
        check("ncreatecountbefore", "0");
        check("nleafcountbefore", "0");
        check("ncreatecountafter", "2");
        check("nleafcountafter", "1");
        check("ccreatedescribe", "prefix:workercollection");
        check("cleafdescribe", "leaf:workercollection");
        check("ccreatebaseclass", "Collection");
        check("cleafbaseclass", "Collection");
        check("ccreateclass", "WorkerCollection");
        check("cleafclass", "WorkerCollection");
        check("lcreatehasdescribe", "true");
        check("lleafhasdescribe", "true");
        check("lcreatehasbaseclass", "true");
        check("lleafhasbaseclass", "true");
        check("ldictset", "true");
        check("ndictcompare", "174");

        expect(state.ole_objects.size() == 3U,
               "native Collection-base CREATEOBJECT/NEWOBJECT plus COM NEWOBJECT should register three runtime objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &create_object = state.ole_objects[0];
            const auto &leaf_object = state.ole_objects[1];
            expect(create_object.prog_id == "WorkerCollection",
                   "native Collection-base CREATEOBJECT should preserve the PRG class name");
            expect(create_object.source == main_path.string(),
                   "native Collection-base CREATEOBJECT should preserve defining PRG provenance");
            expect(create_object.base_class_name == "Collection",
                   "native Collection-base CREATEOBJECT should preserve the builtin Collection base token");
            expect(create_object.class_library.empty(),
                   "native Collection-base CREATEOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(create_object.class_hierarchy.size() == 3U,
                   "native Collection-base CREATEOBJECT should persist native class hierarchy including Collection");
            if (create_object.class_hierarchy.size() == 3U)
            {
                expect(create_object.class_hierarchy[0] == "WORKERCOLLECTION",
                       "native Collection-base CREATEOBJECT should store the native class first");
                expect(create_object.class_hierarchy[1] == "COLLECTION",
                       "native Collection-base CREATEOBJECT should store the builtin Collection base second");
                expect(create_object.class_hierarchy[2] == "OBJECT",
                       "native Collection-base CREATEOBJECT should store the terminal OBJECT token third");
            }

            expect(leaf_object.prog_id == "WorkerCollection",
                   "native Collection-base NEWOBJECT should preserve the PRG class name");
            expect(leaf_object.source == main_path.string(),
                   "native Collection-base NEWOBJECT should preserve defining PRG provenance");
            expect(leaf_object.base_class_name == "Collection",
                   "native Collection-base NEWOBJECT should preserve the builtin Collection base token");
            expect(leaf_object.class_library.empty(),
                   "native Collection-base NEWOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(leaf_object.class_hierarchy.size() == 3U,
                   "native Collection-base NEWOBJECT should persist native class hierarchy including Collection");
            if (leaf_object.class_hierarchy.size() == 3U)
            {
                expect(leaf_object.class_hierarchy[0] == "WORKERCOLLECTION",
                       "native Collection-base NEWOBJECT should store the native class first");
                expect(leaf_object.class_hierarchy[1] == "COLLECTION",
                       "native Collection-base NEWOBJECT should store the builtin Collection base second");
                expect(leaf_object.class_hierarchy[2] == "OBJECT",
                       "native Collection-base NEWOBJECT should store the terminal OBJECT token third");
            }

            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while Collection-base native activation lands");
        }

        const bool has_describe_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "WorkerCollection.Describe";
        });
        expect(has_describe_invoke_event,
               "native Collection-base activation should dispatch ordinary native method calls");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_collection_default_item_invocation_routes_bare_and_member_path_calls()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_collection_default_item";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_collection_default_item.prg";
        write_text(
            main_path,
            "DIMENSION aNames[2]\n"
            "aNames[1] = 'array-one'\n"
            "aNames[2] = 'array-two'\n"
            "oHost = CREATEOBJECT('HostBox')\n"
            "oItems = oHost.oItems\n"
            "oItems.Add('alpha')\n"
            "oItems.Add('beta', 'second')\n"
            "cBareIndex = oItems(1)\n"
            "cMemberIndex = oHost.oItems(2)\n"
            "cMemberKey = oHost.oItems('second')\n"
            "cExplicitKey = oHost.oItems.Item('second')\n"
            "cArrayValue = aNames(2)\n"
            "nFunctionValue = DoubleIt(3)\n"
            "RETURN\n"
            "FUNCTION DoubleIt\n"
            "    LPARAMETERS tnValue\n"
            "    RETURN tnValue * 2\n"
            "ENDFUNC\n"
            "DEFINE CLASS WorkerCollection AS Collection\n"
            "ENDDEFINE\n"
            "DEFINE CLASS HostBox AS Custom\n"
            "    oItems = .NULL.\n"
            "    PROCEDURE Init\n"
            "        THIS.oItems = CREATEOBJECT('WorkerCollection')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Collection default-item script should complete: ") + state.message +
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

        check("cbareindex", "alpha");
        check("cmemberindex", "beta");
        check("cmemberkey", "beta");
        check("cexplicitkey", "beta");
        check("carrayvalue", "array-two");
        check("nfunctionvalue", "6");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_collection_default_item_calls_preserve_member_chains()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_collection_default_item_member_chain";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_collection_default_item_member_chain.prg";
        write_text(
            main_path,
            "DIMENSION aNames[2]\n"
            "aNames[1] = 'array-one'\n"
            "aNames[2] = 'array-two'\n"
            "oForm = CREATEOBJECT('MainForm')\n"
            "cArrayValue = aNames(2)\n"
            "nFunctionValue = DoubleIt(3)\n"
            "cFirstControlBaseClass = oForm.Controls(1).BaseClass\n"
            "cGridColumnCurrentControl = oForm.grdLedger.Columns(1).CurrentControl\n"
            "cNestedGridColumnCurrentControl = oForm.Controls(1).Columns(1).CurrentControl\n"
            "RETURN\n"
            "FUNCTION DoubleIt\n"
            "    LPARAMETERS tnValue\n"
            "    RETURN tnValue * 2\n"
            "ENDFUNC\n"
            "DEFINE CLASS SaveButton AS CommandButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS LedgerGrid AS Grid\n"
            "    ColumnCount = 1\n"
            "ENDDEFINE\n"
            "DEFINE CLASS MainForm AS Form\n"
            "    ADD OBJECT grdLedger AS LedgerGrid\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Collection default-item member-chain script should complete: ") + state.message +
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

        check("carrayvalue", "array-two");
        check("nfunctionvalue", "6");
        check("cfirstcontrolbaseclass", "Grid");
        check("cgridcolumncurrentcontrol", "Text1");
        check("cnestedgridcolumncurrentcontrol", "Text1");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_collection_duplicate_key_raises_without_mutation()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_collection_duplicate_key";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_collection_duplicate_key.prg";
        write_text(
            main_path,
            "oItems = CREATEOBJECT('Collection')\n"
            "oItems.Add('first', 'shared')\n"
            "nBeforeCount = oItems.Count\n"
            "TRY\n"
            "    oItems.Add('second', 'shared')\n"
            "    lCaught = .F.\n"
            "CATCH TO oError\n"
            "    lCaught = .T.\n"
            "    cCaughtMessage = oError.Message\n"
            "ENDTRY\n"
            "nAfterCount = oItems.Count\n"
            "cAfterItem = oItems.Item('shared')\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Collection duplicate-key script should complete: ") + state.message +
                   " @line=" + std::to_string(state.location.line));

        const auto check = [&](const std::string& name, const std::string& expected)
        {
            const auto it = state.globals.find(name);
            expect(it != state.globals.end(), name + " variable not found");
            if (it != state.globals.end())
            {
                expect(copperfin::runtime::format_value(it->second) == expected,
                       name + " expected '" + expected + "' got '" + copperfin::runtime::format_value(it->second) + "'");
            }
        };

        check("lcaught", "true");
        check("nbeforecount", "1");
        check("naftercount", "1");
        check("cafteritem", "first");
        check("ccaughtmessage", "Runtime fault: Collection key value is not unique.");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_collection_subscript_access_routes_to_items_and_preserves_member_chains()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_collection_subscript_item";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_collection_subscript_item.prg";
        write_text(
            main_path,
            "DIMENSION aNames[2]\n"
            "aNames[1] = 'array-one'\n"
            "aNames[2] = 'array-two'\n"
            "oForm = CREATEOBJECT('MainForm')\n"
            "cArrayByBracket = aNames[2]\n"
            "cFirstControlBaseClass = oForm.Controls[1].BaseClass\n"
            "cSecondObjectBaseClass = oForm.Objects[2].BaseClass\n"
            "oDefaultItem = oForm.Controls(1)\n"
            "cDefaultItemBaseClass = oDefaultItem.BaseClass\n"
            "cGridColumnName = oForm.grdLedger.Columns[1].Name\n"
            "cGridColumnCurrentControl = oForm.grdLedger.Columns[1].CurrentControl\n"
            "cNestedGridColumnCurrentControl = oForm.Controls[1].Columns[1].CurrentControl\n"
            "RETURN\n"
            "DEFINE CLASS SaveButton AS CommandButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS LedgerGrid AS Grid\n"
            "    ColumnCount = 1\n"
            "ENDDEFINE\n"
            "DEFINE CLASS MainForm AS Form\n"
            "    ADD OBJECT grdLedger AS LedgerGrid\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Collection subscript script should complete: ") + state.message +
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

        check("carraybybracket", "array-two");
        check("cfirstcontrolbaseclass", "Grid");
        check("csecondobjectbaseclass", "CommandButton");
        check("cdefaultitembaseclass", "Grid");
        check("cgridcolumnname", "Column1");
        check("cgridcolumncurrentcontrol", "Text1");
        check("cnestedgridcolumncurrentcontrol", "Text1");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_objects_child_collection_reflects_count_item_and_foreach_without_leaking_hidden_runtime_surfaces()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_objects_child_collection";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_objects_child_collection.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('DemoForm')\n"
            "lHasObjects = PEMSTATUS(oForm, 'Objects', 1)\n"
            "nObjectsCount = oForm.Objects.Count\n"
            "oFirst = oForm.Objects(1)\n"
            "cFirstTag = oFirst.cTag\n"
            "cSecondTag = oForm.Objects[2].cTag\n"
            "cLoop = ''\n"
            "FOR EACH oChild IN oForm.Objects FOXOBJECT\n"
            "    cLoop = cLoop + IIF(EMPTY(cLoop), '', ',') + oChild.cTag\n"
            "ENDFOR\n"
            "lRemoved = oForm.RemoveObject('zzSave')\n"
            "nObjectsCountAfterRemove = oForm.Objects.Count\n"
            "oRemaining = oForm.Objects(1)\n"
            "cRemainingTag = oRemaining.cTag\n"
            "RETURN\n"
            "DEFINE CLASS SaveButton AS CommandButton\n"
            "    cTag = 'save'\n"
            "ENDDEFINE\n"
            "DEFINE CLASS BadgeLabel AS Label\n"
            "    cTag = 'badge'\n"
            "ENDDEFINE\n"
            "DEFINE CLASS DemoForm AS Form\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('zzSave', 'SaveButton')\n"
            "        THIS.AddObject('aaBadge', 'BadgeLabel')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Objects child-collection script should complete: ") + state.message +
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

        check("lhasobjects", "true");
        check("nobjectscount", "2");
        check("cfirsttag", "save");
        check("csecondtag", "badge");
        check("cloop", "save,badge");
        check("lremoved", "true");
        check("nobjectscountafterremove", "1");
        check("cremainingtag", "badge");

        expect(state.ole_objects.size() == 3U,
               "native Objects child-collection surface should keep owner and detached child state without exporting the hidden synthetic collection");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "DemoForm",
                   "native Objects child-collection surface should keep the owner object visible in exported runtime state");
            const bool leaked_collection = std::any_of(state.ole_objects.begin(), state.ole_objects.end(), [](const auto &object_state)
            {
                return object_state.prog_id == "Collection";
            });
            expect(!leaked_collection,
                   "native Objects child-collection surface should not leak the hidden synthetic Collection object into exported runtime state");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_native_controls_child_collection_reflects_count_item_and_foreach_without_leaking_hidden_runtime_surfaces()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_controls_child_collection";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_controls_child_collection.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('DemoForm')\n"
            "lHasControls = PEMSTATUS(oForm, 'Controls', 1)\n"
            "nControlsCount = oForm.Controls.Count\n"
            "oFirst = oForm.Controls(1)\n"
            "cFirstTag = oFirst.cTag\n"
            "cSecondTag = oForm.Controls[2].cTag\n"
            "cLoop = ''\n"
            "FOR EACH oChild IN oForm.Controls FOXOBJECT\n"
            "    cLoop = cLoop + IIF(EMPTY(cLoop), '', ',') + oChild.cTag\n"
            "ENDFOR\n"
            "lRemoved = oForm.RemoveObject('zzSave')\n"
            "nControlsCountAfterRemove = oForm.Controls.Count\n"
            "oRemaining = oForm.Controls(1)\n"
            "cRemainingTag = oRemaining.cTag\n"
            "RETURN\n"
            "DEFINE CLASS SaveButton AS CommandButton\n"
            "    cTag = 'save'\n"
            "ENDDEFINE\n"
            "DEFINE CLASS BadgeLabel AS Label\n"
            "    cTag = 'badge'\n"
            "ENDDEFINE\n"
            "DEFINE CLASS DemoForm AS Form\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('zzSave', 'SaveButton')\n"
            "        THIS.AddObject('aaBadge', 'BadgeLabel')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Controls child-collection script should complete: ") + state.message +
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

        check("lhascontrols", "true");
        check("ncontrolscount", "2");
        check("cfirsttag", "save");
        check("csecondtag", "badge");
        check("cloop", "save,badge");
        check("lremoved", "true");
        check("ncontrolscountafterremove", "1");
        check("cremainingtag", "badge");

        expect(state.ole_objects.size() == 3U,
               "native Controls child-collection surface should keep owner and detached child state without exporting the hidden synthetic collections");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "DemoForm",
                   "native Controls child-collection surface should keep the owner object visible in exported runtime state");
            const bool leaked_collection = std::any_of(state.ole_objects.begin(), state.ole_objects.end(), [](const auto &object_state)
            {
                return object_state.prog_id == "Collection";
            });
            expect(!leaked_collection,
                   "native Controls child-collection surface should not leak the hidden synthetic Collection objects into exported runtime state");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_native_pageframe_pages_child_collection_reflects_count_item_and_foreach_without_leaking_hidden_runtime_surfaces()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_pageframe_pages_collection";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_pageframe_pages_collection.prg";
        write_text(
            main_path,
            "oFrame = CREATEOBJECT('DemoPageFrame')\n"
            "lHasPages = PEMSTATUS(oFrame, 'Pages', 1)\n"
            "nPageCount = oFrame.PageCount\n"
            "nPagesCount = oFrame.Pages.Count\n"
            "oFirst = oFrame.Pages(1)\n"
            "cFirstTag = oFirst.cTag\n"
            "cSecondTag = oFrame.Pages[2].cTag\n"
            "cLoop = ''\n"
            "FOR EACH oPage IN oFrame.Pages FOXOBJECT\n"
            "    cLoop = cLoop + IIF(EMPTY(cLoop), '', ',') + oPage.cTag\n"
            "ENDFOR\n"
            "RETURN\n"
            "DEFINE CLASS AlphaPage AS Page\n"
            "    cTag = 'alpha'\n"
            "ENDDEFINE\n"
            "DEFINE CLASS BetaPage AS Page\n"
            "    cTag = 'beta'\n"
            "ENDDEFINE\n"
            "DEFINE CLASS DemoPageFrame AS PageFrame\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('pgAlpha', 'AlphaPage')\n"
            "        THIS.AddObject('pgBeta', 'BetaPage')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native PageFrame Pages child-collection script should complete: ") + state.message +
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

        check("lhaspages", "true");
        check("npagecount", "2");
        check("npagescount", "2");
        check("cfirsttag", "alpha");
        check("csecondtag", "beta");
        check("cloop", "alpha,beta");

        expect(state.ole_objects.size() == 3U,
               "native PageFrame Pages child-collection surface should keep owner and page state without exporting the hidden synthetic collection");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "DemoPageFrame",
                   "native PageFrame Pages child-collection surface should keep the owner object visible in exported runtime state");
            const bool leaked_collection = std::any_of(state.ole_objects.begin(), state.ole_objects.end(), [](const auto &object_state)
            {
                return object_state.prog_id == "Collection";
            });
            expect(!leaked_collection,
                   "native PageFrame Pages child-collection surface should not leak the hidden synthetic Collection object into exported runtime state");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_native_pageframe_pagecount_grows_shrinks_and_preserves_builtin_reflection()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_pagecount_property";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_pagecount_property.prg";
        write_text(
            main_path,
            "oFrame = CREATEOBJECT('DemoPageFrame')\n"
            "nPageCountDirect = oFrame.PageCount\n"
            "nPageCountCollection = oFrame.Pages.Count\n"
            "xPageCountGetPem = GETPEM(oFrame, 'PageCount')\n"
            "lHasPageCount = PEMSTATUS(oFrame, 'PageCount', 1)\n"
            "lPageCountReadOnly = PEMSTATUS(oFrame, 'PageCount', 5)\n"
            "nPropMembers = AMEMBERS(aPropMembers, oFrame, 1)\n"
            "nUnionMembers = AMEMBERS(aUnionMembers, oFrame, 3)\n"
            "lPropHasPageCount = .F.\n"
            "lUnionHasPageCount = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'PAGECOUNT'\n"
            "        lPropHasPageCount = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "FOR i = 1 TO nUnionMembers\n"
            "    IF UPPER(aUnionMembers[i]) == 'PAGECOUNT'\n"
            "        lUnionHasPageCount = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "lSetPageCount = SETPEM(oFrame, 'PageCount', 2)\n"
            "nPageCountAfterSetPem = oFrame.PageCount\n"
            "nPagesCountAfterSetPem = oFrame.Pages.Count\n"
            "cRemainingSecondTag = oFrame.Pages(2).cTag\n"
            "lAddPageCount = ADDPROPERTY(oFrame, 'PageCount', 99)\n"
            "lRemovePageCount = REMOVEPROPERTY(oFrame, 'PageCount')\n"
            "oGenerated = CREATEOBJECT('PageFrame')\n"
            "oGenerated.PageCount = 12\n"
            "nGeneratedPageCountAfterGrow = oGenerated.PageCount\n"
            "nGeneratedPagesCountAfterGrow = oGenerated.Pages.Count\n"
            "cGeneratedFourthPageName = oGenerated.Pages(4).Name\n"
            "cGeneratedTenthPageName = oGenerated.Pages(10).Name\n"
            "oGenerated.PageCount = 105\n"
            "nGeneratedPageCountAfterHighClamp = oGenerated.PageCount\n"
            "cGeneratedLastPageName = oGenerated.Pages(99).Name\n"
            "RETURN\n"
            "DEFINE CLASS AlphaPage AS Page\n"
            "    cTag = 'alpha'\n"
            "ENDDEFINE\n"
            "DEFINE CLASS BetaPage AS Page\n"
            "    cTag = 'beta'\n"
            "ENDDEFINE\n"
            "DEFINE CLASS GammaPage AS Page\n"
            "    cTag = 'gamma'\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('lblGone', 'Label')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS DemoPageFrame AS PageFrame\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('pgAlpha', 'AlphaPage')\n"
            "        THIS.AddObject('pgBeta', 'BetaPage')\n"
            "        THIS.AddObject('pgGamma', 'GammaPage')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native PageCount property script should complete: ") + state.message +
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

        check("npagecountdirect", "3");
        check("npagecountcollection", "3");
        check("xpagecountgetpem", "3");
        check("lhaspagecount", "true");
        check("lpagecountreadonly", "false");
        check("lprophaspagecount", "true");
        check("lunionhaspagecount", "true");
        check("lsetpagecount", "true");
        check("npagecountaftersetpem", "2");
        check("npagescountaftersetpem", "2");
        check("cremainingsecondtag", "beta");
        check("laddpagecount", "false");
        check("lremovepagecount", "false");
        check("ngeneratedpagecountaftergrow", "12");
        check("ngeneratedpagescountaftergrow", "12");
        check("cgeneratedfourthpagename", "Page4");
        check("cgeneratedtenthpagename", "Page10");
        check("ngeneratedpagecountafterhighclamp", "99");
        check("cgeneratedlastpagename", "Page99");

        const bool leaked_gamma_page = std::any_of(
            state.ole_objects.begin(),
            state.ole_objects.end(),
            [](const auto &object_state)
            {
                return object_state.prog_id == "GammaPage";
            });
        expect(!leaked_gamma_page,
               "native PageCount shrink should erase truncated page objects from exported runtime state");
        const bool leaked_gone_label = std::any_of(
            state.ole_objects.begin(),
            state.ole_objects.end(),
            [](const auto &object_state)
            {
                if (object_state.prog_id != "Label")
                {
                    return false;
                }
                const auto name = object_state.properties.find("name");
                return name != object_state.properties.end() &&
                       copperfin::runtime::format_value(name->second) == "lblGone";
            });
        expect(!leaked_gone_label,
               "native PageCount shrink should erase nested objects owned by truncated pages");

        fs::remove_all(temp_root, ignored);
    }

}
