#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_createobject_and_newobject_instantiate_same_prg_relation_native_class()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_relation_class";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_relation_class.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('WorkerRelation')\n"
            "oLeaf = NEWOBJECT('WorkerRelation')\n"
            "cCreateChildAlias = oCreate.ChildAlias\n"
            "cLeafChildAlias = oLeaf.ChildAlias\n"
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
            "lDictSet = SETPEM(oDict, 'comparemode', 168)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS WorkerRelation AS Relation\n"
            "    ChildAlias = 'orders'\n"
            "    FUNCTION Describe\n"
            "        LPARAMETERS tcPrefix\n"
            "        RETURN tcPrefix + ':' + THIS.ChildAlias\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Relation-base class script should complete: ") + state.message +
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

        check("ccreatechildalias", "orders");
        check("cleafchildalias", "orders");
        check("ccreatedescribe", "prefix:orders");
        check("cleafdescribe", "leaf:orders");
        check("ccreatebaseclass", "Relation");
        check("cleafbaseclass", "Relation");
        check("ccreateclass", "WorkerRelation");
        check("cleafclass", "WorkerRelation");
        check("lcreatehasdescribe", "true");
        check("lleafhasdescribe", "true");
        check("lcreatehasbaseclass", "true");
        check("lleafhasbaseclass", "true");
        check("ldictset", "true");
        check("ndictcompare", "168");

        expect(state.ole_objects.size() == 3U,
               "native Relation-base CREATEOBJECT/NEWOBJECT plus COM NEWOBJECT should register three runtime objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &create_object = state.ole_objects[0];
            const auto &leaf_object = state.ole_objects[1];
            expect(create_object.prog_id == "WorkerRelation",
                   "native Relation-base CREATEOBJECT should preserve the PRG class name");
            expect(create_object.source == main_path.string(),
                   "native Relation-base CREATEOBJECT should preserve defining PRG provenance");
            expect(create_object.base_class_name == "Relation",
                   "native Relation-base CREATEOBJECT should preserve the builtin Relation base token");
            expect(create_object.class_library.empty(),
                   "native Relation-base CREATEOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(create_object.class_hierarchy.size() == 3U,
                   "native Relation-base CREATEOBJECT should persist native class hierarchy including Relation");
            if (create_object.class_hierarchy.size() == 3U)
            {
                expect(create_object.class_hierarchy[0] == "WORKERRELATION",
                       "native Relation-base CREATEOBJECT should store the native class first");
                expect(create_object.class_hierarchy[1] == "RELATION",
                       "native Relation-base CREATEOBJECT should store the builtin Relation base second");
                expect(create_object.class_hierarchy[2] == "OBJECT",
                       "native Relation-base CREATEOBJECT should store the terminal OBJECT token third");
            }

            expect(leaf_object.prog_id == "WorkerRelation",
                   "native Relation-base NEWOBJECT should preserve the PRG class name");
            expect(leaf_object.source == main_path.string(),
                   "native Relation-base NEWOBJECT should preserve defining PRG provenance");
            expect(leaf_object.base_class_name == "Relation",
                   "native Relation-base NEWOBJECT should preserve the builtin Relation base token");
            expect(leaf_object.class_library.empty(),
                   "native Relation-base NEWOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(leaf_object.class_hierarchy.size() == 3U,
                   "native Relation-base NEWOBJECT should persist native class hierarchy including Relation");
            if (leaf_object.class_hierarchy.size() == 3U)
            {
                expect(leaf_object.class_hierarchy[0] == "WORKERRELATION",
                       "native Relation-base NEWOBJECT should store the native class first");
                expect(leaf_object.class_hierarchy[1] == "RELATION",
                       "native Relation-base NEWOBJECT should store the builtin Relation base second");
                expect(leaf_object.class_hierarchy[2] == "OBJECT",
                       "native Relation-base NEWOBJECT should store the terminal OBJECT token third");
            }

            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while Relation-base native activation lands");
        }

        const bool has_describe_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "WorkerRelation.Describe";
        });
        expect(has_describe_invoke_event,
               "native Relation-base activation should dispatch ordinary native method calls");

        fs::remove_all(temp_root, ignored);
    }

    void test_createobject_and_newobject_instantiate_same_prg_separator_native_class()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_separator_class";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_separator_class.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('WorkerSeparator')\n"
            "oLeaf = NEWOBJECT('WorkerSeparator')\n"
            "cCreateTag = oCreate.Tag\n"
            "cLeafTag = oLeaf.Tag\n"
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
            "lDictSet = SETPEM(oDict, 'comparemode', 169)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS WorkerSeparator AS Separator\n"
            "    Tag = 'separator'\n"
            "    FUNCTION Describe\n"
            "        LPARAMETERS tcPrefix\n"
            "        RETURN tcPrefix + ':' + THIS.Tag\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Separator-base class script should complete: ") + state.message +
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

        check("ccreatetag", "separator");
        check("cleaftag", "separator");
        check("ccreatedescribe", "prefix:separator");
        check("cleafdescribe", "leaf:separator");
        check("ccreatebaseclass", "Separator");
        check("cleafbaseclass", "Separator");
        check("ccreateclass", "WorkerSeparator");
        check("cleafclass", "WorkerSeparator");
        check("lcreatehasdescribe", "true");
        check("lleafhasdescribe", "true");
        check("lcreatehasbaseclass", "true");
        check("lleafhasbaseclass", "true");
        check("ldictset", "true");
        check("ndictcompare", "169");

        expect(state.ole_objects.size() == 3U,
               "native Separator-base CREATEOBJECT/NEWOBJECT plus COM NEWOBJECT should register three runtime objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &create_object = state.ole_objects[0];
            const auto &leaf_object = state.ole_objects[1];
            expect(create_object.prog_id == "WorkerSeparator",
                   "native Separator-base CREATEOBJECT should preserve the PRG class name");
            expect(create_object.source == main_path.string(),
                   "native Separator-base CREATEOBJECT should preserve defining PRG provenance");
            expect(create_object.base_class_name == "Separator",
                   "native Separator-base CREATEOBJECT should preserve the builtin Separator base token");
            expect(create_object.class_library.empty(),
                   "native Separator-base CREATEOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(create_object.class_hierarchy.size() == 3U,
                   "native Separator-base CREATEOBJECT should persist native class hierarchy including Separator");
            if (create_object.class_hierarchy.size() == 3U)
            {
                expect(create_object.class_hierarchy[0] == "WORKERSEPARATOR",
                       "native Separator-base CREATEOBJECT should store the native class first");
                expect(create_object.class_hierarchy[1] == "SEPARATOR",
                       "native Separator-base CREATEOBJECT should store the builtin Separator base second");
                expect(create_object.class_hierarchy[2] == "OBJECT",
                       "native Separator-base CREATEOBJECT should store the terminal OBJECT token third");
            }

            expect(leaf_object.prog_id == "WorkerSeparator",
                   "native Separator-base NEWOBJECT should preserve the PRG class name");
            expect(leaf_object.source == main_path.string(),
                   "native Separator-base NEWOBJECT should preserve defining PRG provenance");
            expect(leaf_object.base_class_name == "Separator",
                   "native Separator-base NEWOBJECT should preserve the builtin Separator base token");
            expect(leaf_object.class_library.empty(),
                   "native Separator-base NEWOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(leaf_object.class_hierarchy.size() == 3U,
                   "native Separator-base NEWOBJECT should persist native class hierarchy including Separator");
            if (leaf_object.class_hierarchy.size() == 3U)
            {
                expect(leaf_object.class_hierarchy[0] == "WORKERSEPARATOR",
                       "native Separator-base NEWOBJECT should store the native class first");
                expect(leaf_object.class_hierarchy[1] == "SEPARATOR",
                       "native Separator-base NEWOBJECT should store the builtin Separator base second");
                expect(leaf_object.class_hierarchy[2] == "OBJECT",
                       "native Separator-base NEWOBJECT should store the terminal OBJECT token third");
            }

            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while Separator-base native activation lands");
        }

        const bool has_describe_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "WorkerSeparator.Describe";
        });
        expect(has_describe_invoke_event,
               "native Separator-base activation should dispatch ordinary native method calls");

        fs::remove_all(temp_root, ignored);
    }

    void test_createobject_and_newobject_instantiate_same_prg_toolbar_native_class()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_toolbar_class";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_toolbar_class.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('WorkerToolBar')\n"
            "oLeaf = NEWOBJECT('WorkerToolBar')\n"
            "cCreateCaption = oCreate.Caption\n"
            "cLeafCaption = oLeaf.Caption\n"
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
            "lDictSet = SETPEM(oDict, 'comparemode', 170)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS WorkerToolBar AS ToolBar\n"
            "    Caption = 'WorkerToolBar'\n"
            "    FUNCTION Describe\n"
            "        LPARAMETERS tcPrefix\n"
            "        RETURN tcPrefix + ':' + THIS.Caption\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native ToolBar-base class script should complete: ") + state.message +
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

        check("ccreatecaption", "WorkerToolBar");
        check("cleafcaption", "WorkerToolBar");
        check("ccreatedescribe", "prefix:WorkerToolBar");
        check("cleafdescribe", "leaf:WorkerToolBar");
        check("ccreatebaseclass", "ToolBar");
        check("cleafbaseclass", "ToolBar");
        check("ccreateclass", "WorkerToolBar");
        check("cleafclass", "WorkerToolBar");
        check("lcreatehasdescribe", "true");
        check("lleafhasdescribe", "true");
        check("lcreatehasbaseclass", "true");
        check("lleafhasbaseclass", "true");
        check("ldictset", "true");
        check("ndictcompare", "170");

        expect(state.ole_objects.size() == 3U,
               "native ToolBar-base CREATEOBJECT/NEWOBJECT plus COM NEWOBJECT should register three runtime objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &create_object = state.ole_objects[0];
            const auto &leaf_object = state.ole_objects[1];
            expect(create_object.prog_id == "WorkerToolBar",
                   "native ToolBar-base CREATEOBJECT should preserve the PRG class name");
            expect(create_object.source == main_path.string(),
                   "native ToolBar-base CREATEOBJECT should preserve defining PRG provenance");
            expect(create_object.base_class_name == "ToolBar",
                   "native ToolBar-base CREATEOBJECT should preserve the builtin ToolBar base token");
            expect(create_object.class_library.empty(),
                   "native ToolBar-base CREATEOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(create_object.class_hierarchy.size() == 3U,
                   "native ToolBar-base CREATEOBJECT should persist native class hierarchy including ToolBar");
            if (create_object.class_hierarchy.size() == 3U)
            {
                expect(create_object.class_hierarchy[0] == "WORKERTOOLBAR",
                       "native ToolBar-base CREATEOBJECT should store the native class first");
                expect(create_object.class_hierarchy[1] == "TOOLBAR",
                       "native ToolBar-base CREATEOBJECT should store the builtin ToolBar base second");
                expect(create_object.class_hierarchy[2] == "OBJECT",
                       "native ToolBar-base CREATEOBJECT should store the terminal OBJECT token third");
            }

            expect(leaf_object.prog_id == "WorkerToolBar",
                   "native ToolBar-base NEWOBJECT should preserve the PRG class name");
            expect(leaf_object.source == main_path.string(),
                   "native ToolBar-base NEWOBJECT should preserve defining PRG provenance");
            expect(leaf_object.base_class_name == "ToolBar",
                   "native ToolBar-base NEWOBJECT should preserve the builtin ToolBar base token");
            expect(leaf_object.class_library.empty(),
                   "native ToolBar-base NEWOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(leaf_object.class_hierarchy.size() == 3U,
                   "native ToolBar-base NEWOBJECT should persist native class hierarchy including ToolBar");
            if (leaf_object.class_hierarchy.size() == 3U)
            {
                expect(leaf_object.class_hierarchy[0] == "WORKERTOOLBAR",
                       "native ToolBar-base NEWOBJECT should store the native class first");
                expect(leaf_object.class_hierarchy[1] == "TOOLBAR",
                       "native ToolBar-base NEWOBJECT should store the builtin ToolBar base second");
                expect(leaf_object.class_hierarchy[2] == "OBJECT",
                       "native ToolBar-base NEWOBJECT should store the terminal OBJECT token third");
            }

            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while ToolBar-base native activation lands");
        }

        const bool has_describe_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "WorkerToolBar.Describe";
        });
        expect(has_describe_invoke_event,
               "native ToolBar-base activation should dispatch ordinary native method calls");

        fs::remove_all(temp_root, ignored);
    }

    void test_createobject_and_newobject_instantiate_same_prg_statusbar_native_class()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_statusbar_class";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_statusbar_class.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('WorkerStatusBar')\n"
            "oLeaf = NEWOBJECT('WorkerStatusBar')\n"
            "cCreateCaption = oCreate.Caption\n"
            "cLeafCaption = oLeaf.Caption\n"
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
            "lDictSet = SETPEM(oDict, 'comparemode', 171)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS WorkerStatusBar AS StatusBar\n"
            "    Caption = 'WorkerStatusBar'\n"
            "    FUNCTION Describe\n"
            "        LPARAMETERS tcPrefix\n"
            "        RETURN tcPrefix + ':' + THIS.Caption\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native StatusBar-base class script should complete: ") + state.message +
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

        check("ccreatecaption", "WorkerStatusBar");
        check("cleafcaption", "WorkerStatusBar");
        check("ccreatedescribe", "prefix:WorkerStatusBar");
        check("cleafdescribe", "leaf:WorkerStatusBar");
        check("ccreatebaseclass", "StatusBar");
        check("cleafbaseclass", "StatusBar");
        check("ccreateclass", "WorkerStatusBar");
        check("cleafclass", "WorkerStatusBar");
        check("lcreatehasdescribe", "true");
        check("lleafhasdescribe", "true");
        check("lcreatehasbaseclass", "true");
        check("lleafhasbaseclass", "true");
        check("ldictset", "true");
        check("ndictcompare", "171");

        expect(state.ole_objects.size() == 3U,
               "native StatusBar-base CREATEOBJECT/NEWOBJECT plus COM NEWOBJECT should register three runtime objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &create_object = state.ole_objects[0];
            const auto &leaf_object = state.ole_objects[1];
            expect(create_object.prog_id == "WorkerStatusBar",
                   "native StatusBar-base CREATEOBJECT should preserve the PRG class name");
            expect(create_object.source == main_path.string(),
                   "native StatusBar-base CREATEOBJECT should preserve defining PRG provenance");
            expect(create_object.base_class_name == "StatusBar",
                   "native StatusBar-base CREATEOBJECT should preserve the builtin StatusBar base token");
            expect(create_object.class_library.empty(),
                   "native StatusBar-base CREATEOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(create_object.class_hierarchy.size() == 3U,
                   "native StatusBar-base CREATEOBJECT should persist native class hierarchy including StatusBar");
            if (create_object.class_hierarchy.size() == 3U)
            {
                expect(create_object.class_hierarchy[0] == "WORKERSTATUSBAR",
                       "native StatusBar-base CREATEOBJECT should store the native class first");
                expect(create_object.class_hierarchy[1] == "STATUSBAR",
                       "native StatusBar-base CREATEOBJECT should store the builtin StatusBar base second");
                expect(create_object.class_hierarchy[2] == "OBJECT",
                       "native StatusBar-base CREATEOBJECT should store the terminal OBJECT token third");
            }

            expect(leaf_object.prog_id == "WorkerStatusBar",
                   "native StatusBar-base NEWOBJECT should preserve the PRG class name");
            expect(leaf_object.source == main_path.string(),
                   "native StatusBar-base NEWOBJECT should preserve defining PRG provenance");
            expect(leaf_object.base_class_name == "StatusBar",
                   "native StatusBar-base NEWOBJECT should preserve the builtin StatusBar base token");
            expect(leaf_object.class_library.empty(),
                   "native StatusBar-base NEWOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(leaf_object.class_hierarchy.size() == 3U,
                   "native StatusBar-base NEWOBJECT should persist native class hierarchy including StatusBar");
            if (leaf_object.class_hierarchy.size() == 3U)
            {
                expect(leaf_object.class_hierarchy[0] == "WORKERSTATUSBAR",
                       "native StatusBar-base NEWOBJECT should store the native class first");
                expect(leaf_object.class_hierarchy[1] == "STATUSBAR",
                       "native StatusBar-base NEWOBJECT should store the builtin StatusBar base second");
                expect(leaf_object.class_hierarchy[2] == "OBJECT",
                       "native StatusBar-base NEWOBJECT should store the terminal OBJECT token third");
            }

            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while StatusBar-base native activation lands");
        }

        const bool has_describe_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "WorkerStatusBar.Describe";
        });
        expect(has_describe_invoke_event,
               "native StatusBar-base activation should dispatch ordinary native method calls");

        fs::remove_all(temp_root, ignored);
    }

    void test_createobject_and_newobject_instantiate_same_prg_projecthook_native_class()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_projecthook_class";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_projecthook_class.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('WorkerProjectHook')\n"
            "oLeaf = NEWOBJECT('WorkerProjectHook')\n"
            "cCreateTag = oCreate.Tag\n"
            "cLeafTag = oLeaf.Tag\n"
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
            "lDictSet = SETPEM(oDict, 'comparemode', 172)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS WorkerProjectHook AS ProjectHook\n"
            "    Tag = 'projecthook'\n"
            "    FUNCTION Describe\n"
            "        LPARAMETERS tcPrefix\n"
            "        RETURN tcPrefix + ':' + THIS.Tag\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native ProjectHook-base class script should complete: ") + state.message +
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

        check("ccreatetag", "projecthook");
        check("cleaftag", "projecthook");
        check("ccreatedescribe", "prefix:projecthook");
        check("cleafdescribe", "leaf:projecthook");
        check("ccreatebaseclass", "ProjectHook");
        check("cleafbaseclass", "ProjectHook");
        check("ccreateclass", "WorkerProjectHook");
        check("cleafclass", "WorkerProjectHook");
        check("lcreatehasdescribe", "true");
        check("lleafhasdescribe", "true");
        check("lcreatehasbaseclass", "true");
        check("lleafhasbaseclass", "true");
        check("ldictset", "true");
        check("ndictcompare", "172");

        expect(state.ole_objects.size() == 3U,
               "native ProjectHook-base CREATEOBJECT/NEWOBJECT plus COM NEWOBJECT should register three runtime objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &create_object = state.ole_objects[0];
            const auto &leaf_object = state.ole_objects[1];
            expect(create_object.prog_id == "WorkerProjectHook",
                   "native ProjectHook-base CREATEOBJECT should preserve the PRG class name");
            expect(create_object.source == main_path.string(),
                   "native ProjectHook-base CREATEOBJECT should preserve defining PRG provenance");
            expect(create_object.base_class_name == "ProjectHook",
                   "native ProjectHook-base CREATEOBJECT should preserve the builtin ProjectHook base token");
            expect(create_object.class_library.empty(),
                   "native ProjectHook-base CREATEOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(create_object.class_hierarchy.size() == 3U,
                   "native ProjectHook-base CREATEOBJECT should persist native class hierarchy including ProjectHook");
            if (create_object.class_hierarchy.size() == 3U)
            {
                expect(create_object.class_hierarchy[0] == "WORKERPROJECTHOOK",
                       "native ProjectHook-base CREATEOBJECT should store the native class first");
                expect(create_object.class_hierarchy[1] == "PROJECTHOOK",
                       "native ProjectHook-base CREATEOBJECT should store the builtin ProjectHook base second");
                expect(create_object.class_hierarchy[2] == "OBJECT",
                       "native ProjectHook-base CREATEOBJECT should store the terminal OBJECT token third");
            }

            expect(leaf_object.prog_id == "WorkerProjectHook",
                   "native ProjectHook-base NEWOBJECT should preserve the PRG class name");
            expect(leaf_object.source == main_path.string(),
                   "native ProjectHook-base NEWOBJECT should preserve defining PRG provenance");
            expect(leaf_object.base_class_name == "ProjectHook",
                   "native ProjectHook-base NEWOBJECT should preserve the builtin ProjectHook base token");
            expect(leaf_object.class_library.empty(),
                   "native ProjectHook-base NEWOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(leaf_object.class_hierarchy.size() == 3U,
                   "native ProjectHook-base NEWOBJECT should persist native class hierarchy including ProjectHook");
            if (leaf_object.class_hierarchy.size() == 3U)
            {
                expect(leaf_object.class_hierarchy[0] == "WORKERPROJECTHOOK",
                       "native ProjectHook-base NEWOBJECT should store the native class first");
                expect(leaf_object.class_hierarchy[1] == "PROJECTHOOK",
                       "native ProjectHook-base NEWOBJECT should store the builtin ProjectHook base second");
                expect(leaf_object.class_hierarchy[2] == "OBJECT",
                       "native ProjectHook-base NEWOBJECT should store the terminal OBJECT token third");
            }

            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while ProjectHook-base native activation lands");
        }

        const bool has_describe_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "WorkerProjectHook.Describe";
        });
        expect(has_describe_invoke_event,
               "native ProjectHook-base activation should dispatch ordinary native method calls");

        fs::remove_all(temp_root, ignored);
    }

    void test_createobject_and_newobject_instantiate_same_prg_cursoradapter_native_class()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_cursoradapter_class";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_cursoradapter_class.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('WorkerCursorAdapter')\n"
            "oLeaf = NEWOBJECT('WorkerCursorAdapter')\n"
            "cCreateAlias = oCreate.Alias\n"
            "cLeafAlias = oLeaf.Alias\n"
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
            "lDictSet = SETPEM(oDict, 'comparemode', 173)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS WorkerCursorAdapter AS CursorAdapter\n"
            "    Alias = 'workercursor'\n"
            "    FUNCTION Describe\n"
            "        LPARAMETERS tcPrefix\n"
            "        RETURN tcPrefix + ':' + THIS.Alias\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native CursorAdapter-base class script should complete: ") + state.message +
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

        check("ccreatealias", "workercursor");
        check("cleafalias", "workercursor");
        check("ccreatedescribe", "prefix:workercursor");
        check("cleafdescribe", "leaf:workercursor");
        check("ccreatebaseclass", "CursorAdapter");
        check("cleafbaseclass", "CursorAdapter");
        check("ccreateclass", "WorkerCursorAdapter");
        check("cleafclass", "WorkerCursorAdapter");
        check("lcreatehasdescribe", "true");
        check("lleafhasdescribe", "true");
        check("lcreatehasbaseclass", "true");
        check("lleafhasbaseclass", "true");
        check("ldictset", "true");
        check("ndictcompare", "173");

        expect(state.ole_objects.size() == 3U,
               "native CursorAdapter-base CREATEOBJECT/NEWOBJECT plus COM NEWOBJECT should register three runtime objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &create_object = state.ole_objects[0];
            const auto &leaf_object = state.ole_objects[1];
            expect(create_object.prog_id == "WorkerCursorAdapter",
                   "native CursorAdapter-base CREATEOBJECT should preserve the PRG class name");
            expect(create_object.source == main_path.string(),
                   "native CursorAdapter-base CREATEOBJECT should preserve defining PRG provenance");
            expect(create_object.base_class_name == "CursorAdapter",
                   "native CursorAdapter-base CREATEOBJECT should preserve the builtin CursorAdapter base token");
            expect(create_object.class_library.empty(),
                   "native CursorAdapter-base CREATEOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(create_object.class_hierarchy.size() == 3U,
                   "native CursorAdapter-base CREATEOBJECT should persist native class hierarchy including CursorAdapter");
            if (create_object.class_hierarchy.size() == 3U)
            {
                expect(create_object.class_hierarchy[0] == "WORKERCURSORADAPTER",
                       "native CursorAdapter-base CREATEOBJECT should store the native class first");
                expect(create_object.class_hierarchy[1] == "CURSORADAPTER",
                       "native CursorAdapter-base CREATEOBJECT should store the builtin CursorAdapter base second");
                expect(create_object.class_hierarchy[2] == "OBJECT",
                       "native CursorAdapter-base CREATEOBJECT should store the terminal OBJECT token third");
            }

            expect(leaf_object.prog_id == "WorkerCursorAdapter",
                   "native CursorAdapter-base NEWOBJECT should preserve the PRG class name");
            expect(leaf_object.source == main_path.string(),
                   "native CursorAdapter-base NEWOBJECT should preserve defining PRG provenance");
            expect(leaf_object.base_class_name == "CursorAdapter",
                   "native CursorAdapter-base NEWOBJECT should preserve the builtin CursorAdapter base token");
            expect(leaf_object.class_library.empty(),
                   "native CursorAdapter-base NEWOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(leaf_object.class_hierarchy.size() == 3U,
                   "native CursorAdapter-base NEWOBJECT should persist native class hierarchy including CursorAdapter");
            if (leaf_object.class_hierarchy.size() == 3U)
            {
                expect(leaf_object.class_hierarchy[0] == "WORKERCURSORADAPTER",
                       "native CursorAdapter-base NEWOBJECT should store the native class first");
                expect(leaf_object.class_hierarchy[1] == "CURSORADAPTER",
                       "native CursorAdapter-base NEWOBJECT should store the builtin CursorAdapter base second");
                expect(leaf_object.class_hierarchy[2] == "OBJECT",
                       "native CursorAdapter-base NEWOBJECT should store the terminal OBJECT token third");
            }

            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while CursorAdapter-base native activation lands");
        }

        const bool has_describe_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "WorkerCursorAdapter.Describe";
        });
        expect(has_describe_invoke_event,
               "native CursorAdapter-base activation should dispatch ordinary native method calls");

        fs::remove_all(temp_root, ignored);
    }

}
