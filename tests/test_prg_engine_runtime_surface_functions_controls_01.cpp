#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_createobject_and_newobject_instantiate_same_prg_grid_native_class()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_grid_class";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_grid_class.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('WorkerGrid')\n"
            "oLeaf = NEWOBJECT('WorkerGrid')\n"
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
            "lDictSet = SETPEM(oDict, 'comparemode', 150)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS WorkerGrid AS Grid\n"
            "    Caption = 'WorkerGrid'\n"
            "    FUNCTION Describe\n"
            "        LPARAMETERS tcPrefix\n"
            "        RETURN tcPrefix + ':' + THIS.Caption\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Grid-base class script should complete: ") + state.message +
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

        check("ccreatecaption", "WorkerGrid");
        check("cleafcaption", "WorkerGrid");
        check("ccreatedescribe", "prefix:WorkerGrid");
        check("cleafdescribe", "leaf:WorkerGrid");
        check("ccreatebaseclass", "Grid");
        check("cleafbaseclass", "Grid");
        check("ccreateclass", "WorkerGrid");
        check("cleafclass", "WorkerGrid");
        check("lcreatehasdescribe", "true");
        check("lleafhasdescribe", "true");
        check("lcreatehasbaseclass", "true");
        check("lleafhasbaseclass", "true");
        check("ldictset", "true");
        check("ndictcompare", "150");

        expect(state.ole_objects.size() == 3U,
               "native Grid-base CREATEOBJECT/NEWOBJECT plus COM NEWOBJECT should register three runtime objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &create_object = state.ole_objects[0];
            const auto &leaf_object = state.ole_objects[1];
            expect(create_object.prog_id == "WorkerGrid",
                   "native Grid-base CREATEOBJECT should preserve the PRG class name");
            expect(create_object.source == main_path.string(),
                   "native Grid-base CREATEOBJECT should preserve defining PRG provenance");
            expect(create_object.base_class_name == "Grid",
                   "native Grid-base CREATEOBJECT should preserve the builtin Grid base token");
            expect(create_object.class_library.empty(),
                   "native Grid-base CREATEOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(create_object.class_hierarchy.size() == 3U,
                   "native Grid-base CREATEOBJECT should persist native class hierarchy including Grid");
            if (create_object.class_hierarchy.size() == 3U)
            {
                expect(create_object.class_hierarchy[0] == "WORKERGRID",
                       "native Grid-base CREATEOBJECT should store the native class first");
                expect(create_object.class_hierarchy[1] == "GRID",
                       "native Grid-base CREATEOBJECT should store the builtin Grid base second");
                expect(create_object.class_hierarchy[2] == "OBJECT",
                       "native Grid-base CREATEOBJECT should store the terminal OBJECT token third");
            }

            expect(leaf_object.prog_id == "WorkerGrid",
                   "native Grid-base NEWOBJECT should preserve the PRG class name");
            expect(leaf_object.source == main_path.string(),
                   "native Grid-base NEWOBJECT should preserve defining PRG provenance");
            expect(leaf_object.base_class_name == "Grid",
                   "native Grid-base NEWOBJECT should preserve the builtin Grid base token");
            expect(leaf_object.class_library.empty(),
                   "native Grid-base NEWOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(leaf_object.class_hierarchy.size() == 3U,
                   "native Grid-base NEWOBJECT should persist native class hierarchy including Grid");
            if (leaf_object.class_hierarchy.size() == 3U)
            {
                expect(leaf_object.class_hierarchy[0] == "WORKERGRID",
                       "native Grid-base NEWOBJECT should store the native class first");
                expect(leaf_object.class_hierarchy[1] == "GRID",
                       "native Grid-base NEWOBJECT should store the builtin Grid base second");
                expect(leaf_object.class_hierarchy[2] == "OBJECT",
                       "native Grid-base NEWOBJECT should store the terminal OBJECT token third");
            }

            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while Grid-base native activation lands");
        }

        const bool has_describe_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "WorkerGrid.Describe";
        });
        expect(has_describe_invoke_event,
               "native Grid-base activation should dispatch ordinary native method calls");

        fs::remove_all(temp_root, ignored);
    }

    void test_createobject_and_newobject_instantiate_same_prg_column_native_class()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_column_class";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_column_class.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('WorkerColumn')\n"
            "oLeaf = NEWOBJECT('WorkerColumn')\n"
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
            "lDictSet = SETPEM(oDict, 'comparemode', 151)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS WorkerColumn AS Column\n"
            "    Caption = 'WorkerColumn'\n"
            "    FUNCTION Describe\n"
            "        LPARAMETERS tcPrefix\n"
            "        RETURN tcPrefix + ':' + THIS.Caption\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Column-base class script should complete: ") + state.message +
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

        check("ccreatecaption", "WorkerColumn");
        check("cleafcaption", "WorkerColumn");
        check("ccreatedescribe", "prefix:WorkerColumn");
        check("cleafdescribe", "leaf:WorkerColumn");
        check("ccreatebaseclass", "Column");
        check("cleafbaseclass", "Column");
        check("ccreateclass", "WorkerColumn");
        check("cleafclass", "WorkerColumn");
        check("lcreatehasdescribe", "true");
        check("lleafhasdescribe", "true");
        check("lcreatehasbaseclass", "true");
        check("lleafhasbaseclass", "true");
        check("ldictset", "true");
        check("ndictcompare", "151");

        expect(state.ole_objects.size() == 3U,
               "native Column-base CREATEOBJECT/NEWOBJECT plus COM NEWOBJECT should register three runtime objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &create_object = state.ole_objects[0];
            const auto &leaf_object = state.ole_objects[1];
            expect(create_object.prog_id == "WorkerColumn",
                   "native Column-base CREATEOBJECT should preserve the PRG class name");
            expect(create_object.source == main_path.string(),
                   "native Column-base CREATEOBJECT should preserve defining PRG provenance");
            expect(create_object.base_class_name == "Column",
                   "native Column-base CREATEOBJECT should preserve the builtin Column base token");
            expect(create_object.class_library.empty(),
                   "native Column-base CREATEOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(create_object.class_hierarchy.size() == 3U,
                   "native Column-base CREATEOBJECT should persist native class hierarchy including Column");
            if (create_object.class_hierarchy.size() == 3U)
            {
                expect(create_object.class_hierarchy[0] == "WORKERCOLUMN",
                       "native Column-base CREATEOBJECT should store the native class first");
                expect(create_object.class_hierarchy[1] == "COLUMN",
                       "native Column-base CREATEOBJECT should store the builtin Column base second");
                expect(create_object.class_hierarchy[2] == "OBJECT",
                       "native Column-base CREATEOBJECT should store the terminal OBJECT token third");
            }

            expect(leaf_object.prog_id == "WorkerColumn",
                   "native Column-base NEWOBJECT should preserve the PRG class name");
            expect(leaf_object.source == main_path.string(),
                   "native Column-base NEWOBJECT should preserve defining PRG provenance");
            expect(leaf_object.base_class_name == "Column",
                   "native Column-base NEWOBJECT should preserve the builtin Column base token");
            expect(leaf_object.class_library.empty(),
                   "native Column-base NEWOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(leaf_object.class_hierarchy.size() == 3U,
                   "native Column-base NEWOBJECT should persist native class hierarchy including Column");
            if (leaf_object.class_hierarchy.size() == 3U)
            {
                expect(leaf_object.class_hierarchy[0] == "WORKERCOLUMN",
                       "native Column-base NEWOBJECT should store the native class first");
                expect(leaf_object.class_hierarchy[1] == "COLUMN",
                       "native Column-base NEWOBJECT should store the builtin Column base second");
                expect(leaf_object.class_hierarchy[2] == "OBJECT",
                       "native Column-base NEWOBJECT should store the terminal OBJECT token third");
            }

            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while Column-base native activation lands");
        }

        const bool has_describe_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "WorkerColumn.Describe";
        });
        expect(has_describe_invoke_event,
               "native Column-base activation should dispatch ordinary native method calls");

        fs::remove_all(temp_root, ignored);
    }

    void test_createobject_and_newobject_instantiate_same_prg_header_native_class()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_header_class";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_header_class.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('WorkerHeader')\n"
            "oLeaf = NEWOBJECT('WorkerHeader')\n"
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
            "lDictSet = SETPEM(oDict, 'comparemode', 152)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS WorkerHeader AS Header\n"
            "    Caption = 'WorkerHeader'\n"
            "    FUNCTION Describe\n"
            "        LPARAMETERS tcPrefix\n"
            "        RETURN tcPrefix + ':' + THIS.Caption\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Header-base class script should complete: ") + state.message +
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

        check("ccreatecaption", "WorkerHeader");
        check("cleafcaption", "WorkerHeader");
        check("ccreatedescribe", "prefix:WorkerHeader");
        check("cleafdescribe", "leaf:WorkerHeader");
        check("ccreatebaseclass", "Header");
        check("cleafbaseclass", "Header");
        check("ccreateclass", "WorkerHeader");
        check("cleafclass", "WorkerHeader");
        check("lcreatehasdescribe", "true");
        check("lleafhasdescribe", "true");
        check("lcreatehasbaseclass", "true");
        check("lleafhasbaseclass", "true");
        check("ldictset", "true");
        check("ndictcompare", "152");

        expect(state.ole_objects.size() == 3U,
               "native Header-base CREATEOBJECT/NEWOBJECT plus COM NEWOBJECT should register three runtime objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &create_object = state.ole_objects[0];
            const auto &leaf_object = state.ole_objects[1];
            expect(create_object.prog_id == "WorkerHeader",
                   "native Header-base CREATEOBJECT should preserve the PRG class name");
            expect(create_object.source == main_path.string(),
                   "native Header-base CREATEOBJECT should preserve defining PRG provenance");
            expect(create_object.base_class_name == "Header",
                   "native Header-base CREATEOBJECT should preserve the builtin Header base token");
            expect(create_object.class_library.empty(),
                   "native Header-base CREATEOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(create_object.class_hierarchy.size() == 3U,
                   "native Header-base CREATEOBJECT should persist native class hierarchy including Header");
            if (create_object.class_hierarchy.size() == 3U)
            {
                expect(create_object.class_hierarchy[0] == "WORKERHEADER",
                       "native Header-base CREATEOBJECT should store the native class first");
                expect(create_object.class_hierarchy[1] == "HEADER",
                       "native Header-base CREATEOBJECT should store the builtin Header base second");
                expect(create_object.class_hierarchy[2] == "OBJECT",
                       "native Header-base CREATEOBJECT should store the terminal OBJECT token third");
            }

            expect(leaf_object.prog_id == "WorkerHeader",
                   "native Header-base NEWOBJECT should preserve the PRG class name");
            expect(leaf_object.source == main_path.string(),
                   "native Header-base NEWOBJECT should preserve defining PRG provenance");
            expect(leaf_object.base_class_name == "Header",
                   "native Header-base NEWOBJECT should preserve the builtin Header base token");
            expect(leaf_object.class_library.empty(),
                   "native Header-base NEWOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(leaf_object.class_hierarchy.size() == 3U,
                   "native Header-base NEWOBJECT should persist native class hierarchy including Header");
            if (leaf_object.class_hierarchy.size() == 3U)
            {
                expect(leaf_object.class_hierarchy[0] == "WORKERHEADER",
                       "native Header-base NEWOBJECT should store the native class first");
                expect(leaf_object.class_hierarchy[1] == "HEADER",
                       "native Header-base NEWOBJECT should store the builtin Header base second");
                expect(leaf_object.class_hierarchy[2] == "OBJECT",
                       "native Header-base NEWOBJECT should store the terminal OBJECT token third");
            }

            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while Header-base native activation lands");
        }

        const bool has_describe_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "WorkerHeader.Describe";
        });
        expect(has_describe_invoke_event,
               "native Header-base activation should dispatch ordinary native method calls");

        fs::remove_all(temp_root, ignored);
    }

    void test_createobject_and_newobject_instantiate_same_prg_combobox_native_class()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_combobox_class";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_combobox_class.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('WorkerComboBox')\n"
            "oLeaf = NEWOBJECT('WorkerComboBox')\n"
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
            "lDictSet = SETPEM(oDict, 'comparemode', 153)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS WorkerComboBox AS ComboBox\n"
            "    Caption = 'WorkerComboBox'\n"
            "    FUNCTION Describe\n"
            "        LPARAMETERS tcPrefix\n"
            "        RETURN tcPrefix + ':' + THIS.Caption\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native ComboBox-base class script should complete: ") + state.message +
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

        check("ccreatecaption", "WorkerComboBox");
        check("cleafcaption", "WorkerComboBox");
        check("ccreatedescribe", "prefix:WorkerComboBox");
        check("cleafdescribe", "leaf:WorkerComboBox");
        check("ccreatebaseclass", "ComboBox");
        check("cleafbaseclass", "ComboBox");
        check("ccreateclass", "WorkerComboBox");
        check("cleafclass", "WorkerComboBox");
        check("lcreatehasdescribe", "true");
        check("lleafhasdescribe", "true");
        check("lcreatehasbaseclass", "true");
        check("lleafhasbaseclass", "true");
        check("ldictset", "true");
        check("ndictcompare", "153");

        expect(state.ole_objects.size() == 3U,
               "native ComboBox-base CREATEOBJECT/NEWOBJECT plus COM NEWOBJECT should register three runtime objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &create_object = state.ole_objects[0];
            const auto &leaf_object = state.ole_objects[1];
            expect(create_object.prog_id == "WorkerComboBox",
                   "native ComboBox-base CREATEOBJECT should preserve the PRG class name");
            expect(create_object.source == main_path.string(),
                   "native ComboBox-base CREATEOBJECT should preserve defining PRG provenance");
            expect(create_object.base_class_name == "ComboBox",
                   "native ComboBox-base CREATEOBJECT should preserve the builtin ComboBox base token");
            expect(create_object.class_library.empty(),
                   "native ComboBox-base CREATEOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(create_object.class_hierarchy.size() == 3U,
                   "native ComboBox-base CREATEOBJECT should persist native class hierarchy including ComboBox");
            if (create_object.class_hierarchy.size() == 3U)
            {
                expect(create_object.class_hierarchy[0] == "WORKERCOMBOBOX",
                       "native ComboBox-base CREATEOBJECT should store the native class first");
                expect(create_object.class_hierarchy[1] == "COMBOBOX",
                       "native ComboBox-base CREATEOBJECT should store the builtin ComboBox base second");
                expect(create_object.class_hierarchy[2] == "OBJECT",
                       "native ComboBox-base CREATEOBJECT should store the terminal OBJECT token third");
            }

            expect(leaf_object.prog_id == "WorkerComboBox",
                   "native ComboBox-base NEWOBJECT should preserve the PRG class name");
            expect(leaf_object.source == main_path.string(),
                   "native ComboBox-base NEWOBJECT should preserve defining PRG provenance");
            expect(leaf_object.base_class_name == "ComboBox",
                   "native ComboBox-base NEWOBJECT should preserve the builtin ComboBox base token");
            expect(leaf_object.class_library.empty(),
                   "native ComboBox-base NEWOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(leaf_object.class_hierarchy.size() == 3U,
                   "native ComboBox-base NEWOBJECT should persist native class hierarchy including ComboBox");
            if (leaf_object.class_hierarchy.size() == 3U)
            {
                expect(leaf_object.class_hierarchy[0] == "WORKERCOMBOBOX",
                       "native ComboBox-base NEWOBJECT should store the native class first");
                expect(leaf_object.class_hierarchy[1] == "COMBOBOX",
                       "native ComboBox-base NEWOBJECT should store the builtin ComboBox base second");
                expect(leaf_object.class_hierarchy[2] == "OBJECT",
                       "native ComboBox-base NEWOBJECT should store the terminal OBJECT token third");
            }

            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while ComboBox-base native activation lands");
        }

        const bool has_describe_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "WorkerComboBox.Describe";
        });
        expect(has_describe_invoke_event,
               "native ComboBox-base activation should dispatch ordinary native method calls");

        fs::remove_all(temp_root, ignored);
    }

    void test_createobject_and_newobject_instantiate_same_prg_listbox_native_class()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_listbox_class";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_listbox_class.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('WorkerListBox')\n"
            "oLeaf = NEWOBJECT('WorkerListBox')\n"
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
            "lDictSet = SETPEM(oDict, 'comparemode', 154)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS WorkerListBox AS ListBox\n"
            "    Caption = 'WorkerListBox'\n"
            "    FUNCTION Describe\n"
            "        LPARAMETERS tcPrefix\n"
            "        RETURN tcPrefix + ':' + THIS.Caption\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native ListBox-base class script should complete: ") + state.message +
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

        check("ccreatecaption", "WorkerListBox");
        check("cleafcaption", "WorkerListBox");
        check("ccreatedescribe", "prefix:WorkerListBox");
        check("cleafdescribe", "leaf:WorkerListBox");
        check("ccreatebaseclass", "ListBox");
        check("cleafbaseclass", "ListBox");
        check("ccreateclass", "WorkerListBox");
        check("cleafclass", "WorkerListBox");
        check("lcreatehasdescribe", "true");
        check("lleafhasdescribe", "true");
        check("lcreatehasbaseclass", "true");
        check("lleafhasbaseclass", "true");
        check("ldictset", "true");
        check("ndictcompare", "154");

        expect(state.ole_objects.size() == 3U,
               "native ListBox-base CREATEOBJECT/NEWOBJECT plus COM NEWOBJECT should register three runtime objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &create_object = state.ole_objects[0];
            const auto &leaf_object = state.ole_objects[1];
            expect(create_object.prog_id == "WorkerListBox",
                   "native ListBox-base CREATEOBJECT should preserve the PRG class name");
            expect(create_object.source == main_path.string(),
                   "native ListBox-base CREATEOBJECT should preserve defining PRG provenance");
            expect(create_object.base_class_name == "ListBox",
                   "native ListBox-base CREATEOBJECT should preserve the builtin ListBox base token");
            expect(create_object.class_library.empty(),
                   "native ListBox-base CREATEOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(create_object.class_hierarchy.size() == 3U,
                   "native ListBox-base CREATEOBJECT should persist native class hierarchy including ListBox");
            if (create_object.class_hierarchy.size() == 3U)
            {
                expect(create_object.class_hierarchy[0] == "WORKERLISTBOX",
                       "native ListBox-base CREATEOBJECT should store the native class first");
                expect(create_object.class_hierarchy[1] == "LISTBOX",
                       "native ListBox-base CREATEOBJECT should store the builtin ListBox base second");
                expect(create_object.class_hierarchy[2] == "OBJECT",
                       "native ListBox-base CREATEOBJECT should store the terminal OBJECT token third");
            }

            expect(leaf_object.prog_id == "WorkerListBox",
                   "native ListBox-base NEWOBJECT should preserve the PRG class name");
            expect(leaf_object.source == main_path.string(),
                   "native ListBox-base NEWOBJECT should preserve defining PRG provenance");
            expect(leaf_object.base_class_name == "ListBox",
                   "native ListBox-base NEWOBJECT should preserve the builtin ListBox base token");
            expect(leaf_object.class_library.empty(),
                   "native ListBox-base NEWOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(leaf_object.class_hierarchy.size() == 3U,
                   "native ListBox-base NEWOBJECT should persist native class hierarchy including ListBox");
            if (leaf_object.class_hierarchy.size() == 3U)
            {
                expect(leaf_object.class_hierarchy[0] == "WORKERLISTBOX",
                       "native ListBox-base NEWOBJECT should store the native class first");
                expect(leaf_object.class_hierarchy[1] == "LISTBOX",
                       "native ListBox-base NEWOBJECT should store the builtin ListBox base second");
                expect(leaf_object.class_hierarchy[2] == "OBJECT",
                       "native ListBox-base NEWOBJECT should store the terminal OBJECT token third");
            }

            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while ListBox-base native activation lands");
        }

        const bool has_describe_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "WorkerListBox.Describe";
        });
        expect(has_describe_invoke_event,
               "native ListBox-base activation should dispatch ordinary native method calls");

        fs::remove_all(temp_root, ignored);
    }

    void test_createobject_and_newobject_instantiate_same_prg_checkbox_native_class()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_checkbox_class";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_checkbox_class.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('WorkerCheckBox')\n"
            "oLeaf = NEWOBJECT('WorkerCheckBox')\n"
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
            "lDictSet = SETPEM(oDict, 'comparemode', 155)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS WorkerCheckBox AS CheckBox\n"
            "    Caption = 'WorkerCheckBox'\n"
            "    FUNCTION Describe\n"
            "        LPARAMETERS tcPrefix\n"
            "        RETURN tcPrefix + ':' + THIS.Caption\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native CheckBox-base class script should complete: ") + state.message +
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

        check("ccreatecaption", "WorkerCheckBox");
        check("cleafcaption", "WorkerCheckBox");
        check("ccreatedescribe", "prefix:WorkerCheckBox");
        check("cleafdescribe", "leaf:WorkerCheckBox");
        check("ccreatebaseclass", "CheckBox");
        check("cleafbaseclass", "CheckBox");
        check("ccreateclass", "WorkerCheckBox");
        check("cleafclass", "WorkerCheckBox");
        check("lcreatehasdescribe", "true");
        check("lleafhasdescribe", "true");
        check("lcreatehasbaseclass", "true");
        check("lleafhasbaseclass", "true");
        check("ldictset", "true");
        check("ndictcompare", "155");

        expect(state.ole_objects.size() == 3U,
               "native CheckBox-base CREATEOBJECT/NEWOBJECT plus COM NEWOBJECT should register three runtime objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &create_object = state.ole_objects[0];
            const auto &leaf_object = state.ole_objects[1];
            expect(create_object.prog_id == "WorkerCheckBox",
                   "native CheckBox-base CREATEOBJECT should preserve the PRG class name");
            expect(create_object.source == main_path.string(),
                   "native CheckBox-base CREATEOBJECT should preserve defining PRG provenance");
            expect(create_object.base_class_name == "CheckBox",
                   "native CheckBox-base CREATEOBJECT should preserve the builtin CheckBox base token");
            expect(create_object.class_library.empty(),
                   "native CheckBox-base CREATEOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(create_object.class_hierarchy.size() == 3U,
                   "native CheckBox-base CREATEOBJECT should persist native class hierarchy including CheckBox");
            if (create_object.class_hierarchy.size() == 3U)
            {
                expect(create_object.class_hierarchy[0] == "WORKERCHECKBOX",
                       "native CheckBox-base CREATEOBJECT should store the native class first");
                expect(create_object.class_hierarchy[1] == "CHECKBOX",
                       "native CheckBox-base CREATEOBJECT should store the builtin CheckBox base second");
                expect(create_object.class_hierarchy[2] == "OBJECT",
                       "native CheckBox-base CREATEOBJECT should store the terminal OBJECT token third");
            }

            expect(leaf_object.prog_id == "WorkerCheckBox",
                   "native CheckBox-base NEWOBJECT should preserve the PRG class name");
            expect(leaf_object.source == main_path.string(),
                   "native CheckBox-base NEWOBJECT should preserve defining PRG provenance");
            expect(leaf_object.base_class_name == "CheckBox",
                   "native CheckBox-base NEWOBJECT should preserve the builtin CheckBox base token");
            expect(leaf_object.class_library.empty(),
                   "native CheckBox-base NEWOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(leaf_object.class_hierarchy.size() == 3U,
                   "native CheckBox-base NEWOBJECT should persist native class hierarchy including CheckBox");
            if (leaf_object.class_hierarchy.size() == 3U)
            {
                expect(leaf_object.class_hierarchy[0] == "WORKERCHECKBOX",
                       "native CheckBox-base NEWOBJECT should store the native class first");
                expect(leaf_object.class_hierarchy[1] == "CHECKBOX",
                       "native CheckBox-base NEWOBJECT should store the builtin CheckBox base second");
                expect(leaf_object.class_hierarchy[2] == "OBJECT",
                       "native CheckBox-base NEWOBJECT should store the terminal OBJECT token third");
            }

            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while CheckBox-base native activation lands");
        }

        const bool has_describe_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "WorkerCheckBox.Describe";
        });
        expect(has_describe_invoke_event,
               "native CheckBox-base activation should dispatch ordinary native method calls");

        fs::remove_all(temp_root, ignored);
    }

}
