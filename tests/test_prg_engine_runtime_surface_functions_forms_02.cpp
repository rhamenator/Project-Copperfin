#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_createobject_and_newobject_instantiate_same_prg_page_native_class()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_page_class";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_page_class.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('WorkerPage')\n"
            "oLeaf = NEWOBJECT('WorkerPage')\n"
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
            "lDictSet = SETPEM(oDict, 'comparemode', 148)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS WorkerPage AS Page\n"
            "    Caption = 'WorkerPage'\n"
            "    FUNCTION Describe\n"
            "        LPARAMETERS tcPrefix\n"
            "        RETURN tcPrefix + ':' + THIS.Caption\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Page-base class script should complete: ") + state.message +
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

        check("ccreatecaption", "WorkerPage");
        check("cleafcaption", "WorkerPage");
        check("ccreatedescribe", "prefix:WorkerPage");
        check("cleafdescribe", "leaf:WorkerPage");
        check("ccreatebaseclass", "Page");
        check("cleafbaseclass", "Page");
        check("ccreateclass", "WorkerPage");
        check("cleafclass", "WorkerPage");
        check("lcreatehasdescribe", "true");
        check("lleafhasdescribe", "true");
        check("lcreatehasbaseclass", "true");
        check("lleafhasbaseclass", "true");
        check("ldictset", "true");
        check("ndictcompare", "148");

        expect(state.ole_objects.size() == 3U,
               "native Page-base CREATEOBJECT/NEWOBJECT plus COM NEWOBJECT should register three runtime objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &create_object = state.ole_objects[0];
            const auto &leaf_object = state.ole_objects[1];
            expect(create_object.prog_id == "WorkerPage",
                   "native Page-base CREATEOBJECT should preserve the PRG class name");
            expect(create_object.source == main_path.string(),
                   "native Page-base CREATEOBJECT should preserve defining PRG provenance");
            expect(create_object.base_class_name == "Page",
                   "native Page-base CREATEOBJECT should preserve the builtin Page base token");
            expect(create_object.class_library.empty(),
                   "native Page-base CREATEOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(create_object.class_hierarchy.size() == 3U,
                   "native Page-base CREATEOBJECT should persist native class hierarchy including Page");
            if (create_object.class_hierarchy.size() == 3U)
            {
                expect(create_object.class_hierarchy[0] == "WORKERPAGE",
                       "native Page-base CREATEOBJECT should store the native class first");
                expect(create_object.class_hierarchy[1] == "PAGE",
                       "native Page-base CREATEOBJECT should store the builtin Page base second");
                expect(create_object.class_hierarchy[2] == "OBJECT",
                       "native Page-base CREATEOBJECT should store the terminal OBJECT token third");
            }

            expect(leaf_object.prog_id == "WorkerPage",
                   "native Page-base NEWOBJECT should preserve the PRG class name");
            expect(leaf_object.source == main_path.string(),
                   "native Page-base NEWOBJECT should preserve defining PRG provenance");
            expect(leaf_object.base_class_name == "Page",
                   "native Page-base NEWOBJECT should preserve the builtin Page base token");
            expect(leaf_object.class_library.empty(),
                   "native Page-base NEWOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(leaf_object.class_hierarchy.size() == 3U,
                   "native Page-base NEWOBJECT should persist native class hierarchy including Page");
            if (leaf_object.class_hierarchy.size() == 3U)
            {
                expect(leaf_object.class_hierarchy[0] == "WORKERPAGE",
                       "native Page-base NEWOBJECT should store the native class first");
                expect(leaf_object.class_hierarchy[1] == "PAGE",
                       "native Page-base NEWOBJECT should store the builtin Page base second");
                expect(leaf_object.class_hierarchy[2] == "OBJECT",
                       "native Page-base NEWOBJECT should store the terminal OBJECT token third");
            }

            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while Page-base native activation lands");
        }

        const bool has_describe_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "WorkerPage.Describe";
        });
        expect(has_describe_invoke_event,
               "native Page-base activation should dispatch ordinary native method calls");

        fs::remove_all(temp_root, ignored);
    }

    void test_createobject_and_newobject_instantiate_same_prg_pageframe_native_class()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_pageframe_class";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_pageframe_class.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('WorkerPageFrame')\n"
            "oLeaf = NEWOBJECT('WorkerPageFrame')\n"
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
            "lDictSet = SETPEM(oDict, 'comparemode', 149)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS WorkerPageFrame AS PageFrame\n"
            "    Caption = 'WorkerPageFrame'\n"
            "    FUNCTION Describe\n"
            "        LPARAMETERS tcPrefix\n"
            "        RETURN tcPrefix + ':' + THIS.Caption\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native PageFrame-base class script should complete: ") + state.message +
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

        check("ccreatecaption", "WorkerPageFrame");
        check("cleafcaption", "WorkerPageFrame");
        check("ccreatedescribe", "prefix:WorkerPageFrame");
        check("cleafdescribe", "leaf:WorkerPageFrame");
        check("ccreatebaseclass", "PageFrame");
        check("cleafbaseclass", "PageFrame");
        check("ccreateclass", "WorkerPageFrame");
        check("cleafclass", "WorkerPageFrame");
        check("lcreatehasdescribe", "true");
        check("lleafhasdescribe", "true");
        check("lcreatehasbaseclass", "true");
        check("lleafhasbaseclass", "true");
        check("ldictset", "true");
        check("ndictcompare", "149");

        expect(state.ole_objects.size() == 3U,
               "native PageFrame-base CREATEOBJECT/NEWOBJECT plus COM NEWOBJECT should register three runtime objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &create_object = state.ole_objects[0];
            const auto &leaf_object = state.ole_objects[1];
            expect(create_object.prog_id == "WorkerPageFrame",
                   "native PageFrame-base CREATEOBJECT should preserve the PRG class name");
            expect(create_object.source == main_path.string(),
                   "native PageFrame-base CREATEOBJECT should preserve defining PRG provenance");
            expect(create_object.base_class_name == "PageFrame",
                   "native PageFrame-base CREATEOBJECT should preserve the builtin PageFrame base token");
            expect(create_object.class_library.empty(),
                   "native PageFrame-base CREATEOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(create_object.class_hierarchy.size() == 3U,
                   "native PageFrame-base CREATEOBJECT should persist native class hierarchy including PageFrame");
            if (create_object.class_hierarchy.size() == 3U)
            {
                expect(create_object.class_hierarchy[0] == "WORKERPAGEFRAME",
                       "native PageFrame-base CREATEOBJECT should store the native class first");
                expect(create_object.class_hierarchy[1] == "PAGEFRAME",
                       "native PageFrame-base CREATEOBJECT should store the builtin PageFrame base second");
                expect(create_object.class_hierarchy[2] == "OBJECT",
                       "native PageFrame-base CREATEOBJECT should store the terminal OBJECT token third");
            }

            expect(leaf_object.prog_id == "WorkerPageFrame",
                   "native PageFrame-base NEWOBJECT should preserve the PRG class name");
            expect(leaf_object.source == main_path.string(),
                   "native PageFrame-base NEWOBJECT should preserve defining PRG provenance");
            expect(leaf_object.base_class_name == "PageFrame",
                   "native PageFrame-base NEWOBJECT should preserve the builtin PageFrame base token");
            expect(leaf_object.class_library.empty(),
                   "native PageFrame-base NEWOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(leaf_object.class_hierarchy.size() == 3U,
                   "native PageFrame-base NEWOBJECT should persist native class hierarchy including PageFrame");
            if (leaf_object.class_hierarchy.size() == 3U)
            {
                expect(leaf_object.class_hierarchy[0] == "WORKERPAGEFRAME",
                       "native PageFrame-base NEWOBJECT should store the native class first");
                expect(leaf_object.class_hierarchy[1] == "PAGEFRAME",
                       "native PageFrame-base NEWOBJECT should store the builtin PageFrame base second");
                expect(leaf_object.class_hierarchy[2] == "OBJECT",
                       "native PageFrame-base NEWOBJECT should store the terminal OBJECT token third");
            }

            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while PageFrame-base native activation lands");
        }

        const bool has_describe_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "WorkerPageFrame.Describe";
        });
        expect(has_describe_invoke_event,
               "native PageFrame-base activation should dispatch ordinary native method calls");

        fs::remove_all(temp_root, ignored);
    }

}
