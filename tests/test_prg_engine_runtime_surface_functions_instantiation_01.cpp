#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_createobject_instantiates_native_prg_class_and_preserves_plain_object_creation()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_createobject_native_prg_class";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "createobject_native_class.prg";
        write_text(
            main_path,
            "oWidget = CREATEOBJECT('MyWidget')\n"
            "cCaption = oWidget.Caption\n"
            "nCount = oWidget.nCount\n"
            "lHasInit = GETPEM(oWidget, 'Init')\n"
            "lHasCaption = PEMSTATUS(oWidget, 'Caption', 1)\n"
            "oPlain = CREATEOBJECT('Empty')\n"
            "oPlain.Extra = 'plain'\n"
            "cPlain = oPlain.Extra\n"
            "RETURN\n"
            "DEFINE CLASS MyWidget AS Custom\n"
            "    Caption = 'Demo'\n"
            "    nCount = 3\n"
            "    PROCEDURE Init\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native CREATEOBJECT script should complete: ") + state.message +
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

        expect(state.globals.count("owidget") && state.globals.at("owidget").kind == copperfin::runtime::PrgValueKind::string,
               "CREATEOBJECT('MyWidget') should return a string object ref");
        check("ccaption", "Demo");
        check("ncount", "3");
        check("lhasinit", "true");
        check("lhascaption", "true");

        expect(state.globals.count("oplain") && state.globals.at("oplain").kind == copperfin::runtime::PrgValueKind::string,
               "CREATEOBJECT('Empty') should still return a string object ref");
        check("cplain", "plain");

        expect(state.ole_objects.size() == 2U,
               "native CREATEOBJECT plus plain CREATEOBJECT should register two runtime objects");
        if (state.ole_objects.size() == 2U)
        {
            expect(state.ole_objects[0].prog_id == "MyWidget",
                   "native CREATEOBJECT should preserve the PRG class name in runtime object state");
            expect(state.ole_objects[0].source == main_path.string(),
                   "native CREATEOBJECT should preserve the defining PRG path as object provenance");
            expect(state.ole_objects[1].prog_id == "Empty",
                   "plain CREATEOBJECT should still preserve the requested non-class prog id");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_native_prg_class_dimension_properties_are_per_instance_arrays()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_class_dimension_properties";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_class_dimension_properties.prg";
        write_text(
            main_path,
            "oFirst = CREATEOBJECT('ArrayHolder')\n"
            "oSecond = CREATEOBJECT('ArrayHolder')\n"
            "cType = TYPE('oFirst.aValues')\n"
            "nRows = ALEN(oFirst.aValues, 1)\n"
            "nColumns = ALEN(oFirst.aValues, 2)\n"
            "oFirst.aValues[1, 2] = 'first'\n"
            "oSecond.aValues[1, 2] = 'second'\n"
            "cFirst = oFirst.aValues[1, 2]\n"
            "cSecond = oSecond.aValues[1, 2]\n"
            "RETURN\n"
            "DEFINE CLASS ArrayHolder AS Custom\n"
            "    DIMENSION aValues[2, 2]\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native class DIMENSION script should complete: ") + state.message +
                   " @line=" + std::to_string(state.location.line));

        const auto check = [&](const std::string &name, const std::string &expected)
        {
            const auto it = state.globals.find(name);
            expect(it != state.globals.end(), name + " variable should be present");
            if (it != state.globals.end())
            {
                expect(copperfin::runtime::format_value(it->second) == expected,
                       name + " expected '" + expected + "' got '" + copperfin::runtime::format_value(it->second) + "'");
            }
        };

        check("ctype", "A");
        check("nrows", "2");
        check("ncolumns", "2");
        check("cfirst", "first");
        check("csecond", "second");

        fs::remove_all(temp_root, ignored);
    }

    void test_newobject_instantiates_native_prg_class_and_preserves_ole_newobject()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_newobject_native_prg_class";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "newobject_native_class.prg";
        write_text(
            main_path,
            "oWidget = NEWOBJECT('MyWidget')\n"
            "cName = oWidget.Name\n"
            "lHasSave = GETPEM(oWidget, 'CanSave')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lSetCompare = SETPEM(oDict, 'comparemode', 2)\n"
            "nCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS MyWidget AS Custom\n"
            "    Name = 'Widget'\n"
            "    FUNCTION CanSave\n"
            "        RETURN .T.\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native NEWOBJECT script should complete: ") + state.message +
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

        expect(state.globals.count("owidget") && state.globals.at("owidget").kind == copperfin::runtime::PrgValueKind::string,
               "NEWOBJECT('MyWidget') should return a string object ref");
        check("cname", "Widget");
        check("lhassave", "true");

        expect(state.globals.count("odict") && state.globals.at("odict").kind == copperfin::runtime::PrgValueKind::string,
               "NEWOBJECT('Scripting.Dictionary', 'vbscript.dll') should still return a string object ref");
        check("lsetcompare", "true");
        check("ncompare", "2");

        expect(state.ole_objects.size() == 2U,
               "native and OLE NEWOBJECT calls should both register runtime objects");
        if (state.ole_objects.size() == 2U)
        {
            expect(state.ole_objects[0].prog_id == "MyWidget",
                   "native NEWOBJECT should preserve the PRG class name in runtime object state");
            expect(state.ole_objects[0].source == main_path.string(),
                   "native NEWOBJECT should preserve the defining PRG path as object provenance");
            expect(state.ole_objects[1].prog_id == "Scripting.Dictionary",
                   "OLE NEWOBJECT should still preserve the requested COM class");
            expect(state.ole_objects[1].source == "vbscript.dll",
                   "OLE NEWOBJECT should still preserve the requested library source");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_createobject_and_newobject_instantiate_same_prg_session_native_class()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_session_class";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_session_class.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('WorkerSession')\n"
            "oLeaf = NEWOBJECT('WorkerSession')\n"
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
            "lDictSet = SETPEM(oDict, 'comparemode', 140)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS WorkerSession AS Session\n"
            "    Caption = 'Worker'\n"
            "    FUNCTION Describe\n"
            "        LPARAMETERS tcPrefix\n"
            "        RETURN tcPrefix + ':' + THIS.Caption\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Session-base class script should complete: ") + state.message +
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

        check("ccreatecaption", "Worker");
        check("cleafcaption", "Worker");
        check("ccreatedescribe", "prefix:Worker");
        check("cleafdescribe", "leaf:Worker");
        check("ccreatebaseclass", "Session");
        check("cleafbaseclass", "Session");
        check("ccreateclass", "WorkerSession");
        check("cleafclass", "WorkerSession");
        check("lcreatehasdescribe", "true");
        check("lleafhasdescribe", "true");
        check("lcreatehasbaseclass", "true");
        check("lleafhasbaseclass", "true");
        check("ldictset", "true");
        check("ndictcompare", "140");

        expect(state.ole_objects.size() == 3U,
               "native Session-base CREATEOBJECT/NEWOBJECT plus COM NEWOBJECT should register three runtime objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &create_object = state.ole_objects[0];
            const auto &leaf_object = state.ole_objects[1];
            expect(create_object.prog_id == "WorkerSession",
                   "native Session-base CREATEOBJECT should preserve the PRG class name");
            expect(create_object.source == main_path.string(),
                   "native Session-base CREATEOBJECT should preserve defining PRG provenance");
            expect(create_object.base_class_name == "Session",
                   "native Session-base CREATEOBJECT should preserve the builtin Session base token");
            expect(create_object.class_library.empty(),
                   "native Session-base CREATEOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(create_object.class_hierarchy.size() == 3U,
                   "native Session-base CREATEOBJECT should persist native class hierarchy including Session");
            if (create_object.class_hierarchy.size() == 3U)
            {
                expect(create_object.class_hierarchy[0] == "WORKERSESSION",
                       "native Session-base CREATEOBJECT should store the native class first");
                expect(create_object.class_hierarchy[1] == "SESSION",
                       "native Session-base CREATEOBJECT should store the builtin Session base second");
                expect(create_object.class_hierarchy[2] == "OBJECT",
                       "native Session-base CREATEOBJECT should store the terminal OBJECT token third");
            }

            expect(leaf_object.prog_id == "WorkerSession",
                   "native Session-base NEWOBJECT should preserve the PRG class name");
            expect(leaf_object.source == main_path.string(),
                   "native Session-base NEWOBJECT should preserve defining PRG provenance");
            expect(leaf_object.base_class_name == "Session",
                   "native Session-base NEWOBJECT should preserve the builtin Session base token");
            expect(leaf_object.class_library.empty(),
                   "native Session-base NEWOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(leaf_object.class_hierarchy.size() == 3U,
                   "native Session-base NEWOBJECT should persist native class hierarchy including Session");
            if (leaf_object.class_hierarchy.size() == 3U)
            {
                expect(leaf_object.class_hierarchy[0] == "WORKERSESSION",
                       "native Session-base NEWOBJECT should store the native class first");
                expect(leaf_object.class_hierarchy[1] == "SESSION",
                       "native Session-base NEWOBJECT should store the builtin Session base second");
                expect(leaf_object.class_hierarchy[2] == "OBJECT",
                       "native Session-base NEWOBJECT should store the terminal OBJECT token third");
            }

            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while Session-base native activation lands");
        }

        const bool has_describe_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "WorkerSession.Describe";
        });
        expect(has_describe_invoke_event,
               "native Session-base activation should dispatch ordinary native method calls");

        fs::remove_all(temp_root, ignored);
    }

    void test_createobject_and_newobject_instantiate_same_prg_session_olepublic_native_class()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_session_olepublic_class";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_session_olepublic_class.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('WorkerSession')\n"
            "oLeaf = NEWOBJECT('WorkerSession')\n"
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
            "lDictSet = SETPEM(oDict, 'comparemode', 141)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS WorkerSession AS Session OLEPUBLIC\n"
            "    Caption = 'Worker'\n"
            "    FUNCTION Describe\n"
            "        LPARAMETERS tcPrefix\n"
            "        RETURN tcPrefix + ':' + THIS.Caption\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Session OLEPUBLIC class script should complete: ") + state.message +
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

        check("ccreatecaption", "Worker");
        check("cleafcaption", "Worker");
        check("ccreatedescribe", "prefix:Worker");
        check("cleafdescribe", "leaf:Worker");
        check("ccreatebaseclass", "Session");
        check("cleafbaseclass", "Session");
        check("ccreateclass", "WorkerSession");
        check("cleafclass", "WorkerSession");
        check("lcreatehasdescribe", "true");
        check("lleafhasdescribe", "true");
        check("lcreatehasbaseclass", "true");
        check("lleafhasbaseclass", "true");
        check("ldictset", "true");
        check("ndictcompare", "141");

        expect(state.ole_objects.size() == 3U,
               "native Session OLEPUBLIC CREATEOBJECT/NEWOBJECT plus COM NEWOBJECT should register three runtime objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &create_object = state.ole_objects[0];
            const auto &leaf_object = state.ole_objects[1];
            expect(create_object.prog_id == "WorkerSession",
                   "native Session OLEPUBLIC CREATEOBJECT should preserve the PRG class name");
            expect(create_object.source == main_path.string(),
                   "native Session OLEPUBLIC CREATEOBJECT should preserve defining PRG provenance");
            expect(create_object.base_class_name == "Session",
                   "native Session OLEPUBLIC CREATEOBJECT should preserve the builtin Session base token");
            expect(create_object.class_library.empty(),
                   "native Session OLEPUBLIC CREATEOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(create_object.class_hierarchy.size() == 3U,
                   "native Session OLEPUBLIC CREATEOBJECT should persist native class hierarchy including Session");
            if (create_object.class_hierarchy.size() == 3U)
            {
                expect(create_object.class_hierarchy[0] == "WORKERSESSION",
                       "native Session OLEPUBLIC CREATEOBJECT should store the native class first");
                expect(create_object.class_hierarchy[1] == "SESSION",
                       "native Session OLEPUBLIC CREATEOBJECT should store the builtin Session base second");
                expect(create_object.class_hierarchy[2] == "OBJECT",
                       "native Session OLEPUBLIC CREATEOBJECT should store the terminal OBJECT token third");
            }

            expect(leaf_object.prog_id == "WorkerSession",
                   "native Session OLEPUBLIC NEWOBJECT should preserve the PRG class name");
            expect(leaf_object.source == main_path.string(),
                   "native Session OLEPUBLIC NEWOBJECT should preserve defining PRG provenance");
            expect(leaf_object.base_class_name == "Session",
                   "native Session OLEPUBLIC NEWOBJECT should preserve the builtin Session base token");
            expect(leaf_object.class_library.empty(),
                   "native Session OLEPUBLIC NEWOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(leaf_object.class_hierarchy.size() == 3U,
                   "native Session OLEPUBLIC NEWOBJECT should persist native class hierarchy including Session");
            if (leaf_object.class_hierarchy.size() == 3U)
            {
                expect(leaf_object.class_hierarchy[0] == "WORKERSESSION",
                       "native Session OLEPUBLIC NEWOBJECT should store the native class first");
                expect(leaf_object.class_hierarchy[1] == "SESSION",
                       "native Session OLEPUBLIC NEWOBJECT should store the builtin Session base second");
                expect(leaf_object.class_hierarchy[2] == "OBJECT",
                       "native Session OLEPUBLIC NEWOBJECT should store the terminal OBJECT token third");
            }

            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while Session OLEPUBLIC native activation lands");
        }

        const bool has_describe_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "WorkerSession.Describe";
        });
        expect(has_describe_invoke_event,
               "native Session OLEPUBLIC activation should dispatch ordinary native method calls");

        fs::remove_all(temp_root, ignored);
    }

    void test_createobject_and_newobject_instantiate_same_prg_form_native_class()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_form_class";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_form_class.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('WorkerForm')\n"
            "oLeaf = NEWOBJECT('WorkerForm')\n"
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
            "lDictSet = SETPEM(oDict, 'comparemode', 142)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS WorkerForm AS Form\n"
            "    Caption = 'Worker'\n"
            "    FUNCTION Describe\n"
            "        LPARAMETERS tcPrefix\n"
            "        RETURN tcPrefix + ':' + THIS.Caption\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Form-base class script should complete: ") + state.message +
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

        check("ccreatecaption", "Worker");
        check("cleafcaption", "Worker");
        check("ccreatedescribe", "prefix:Worker");
        check("cleafdescribe", "leaf:Worker");
        check("ccreatebaseclass", "Form");
        check("cleafbaseclass", "Form");
        check("ccreateclass", "WorkerForm");
        check("cleafclass", "WorkerForm");
        check("lcreatehasdescribe", "true");
        check("lleafhasdescribe", "true");
        check("lcreatehasbaseclass", "true");
        check("lleafhasbaseclass", "true");
        check("ldictset", "true");
        check("ndictcompare", "142");

        expect(state.ole_objects.size() == 3U,
               "native Form-base CREATEOBJECT/NEWOBJECT plus COM NEWOBJECT should register three runtime objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &create_object = state.ole_objects[0];
            const auto &leaf_object = state.ole_objects[1];
            expect(create_object.prog_id == "WorkerForm",
                   "native Form-base CREATEOBJECT should preserve the PRG class name");
            expect(create_object.source == main_path.string(),
                   "native Form-base CREATEOBJECT should preserve defining PRG provenance");
            expect(create_object.base_class_name == "Form",
                   "native Form-base CREATEOBJECT should preserve the builtin Form base token");
            expect(create_object.class_library.empty(),
                   "native Form-base CREATEOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(create_object.class_hierarchy.size() == 3U,
                   "native Form-base CREATEOBJECT should persist native class hierarchy including Form");
            if (create_object.class_hierarchy.size() == 3U)
            {
                expect(create_object.class_hierarchy[0] == "WORKERFORM",
                       "native Form-base CREATEOBJECT should store the native class first");
                expect(create_object.class_hierarchy[1] == "FORM",
                       "native Form-base CREATEOBJECT should store the builtin Form base second");
                expect(create_object.class_hierarchy[2] == "OBJECT",
                       "native Form-base CREATEOBJECT should store the terminal OBJECT token third");
            }

            expect(leaf_object.prog_id == "WorkerForm",
                   "native Form-base NEWOBJECT should preserve the PRG class name");
            expect(leaf_object.source == main_path.string(),
                   "native Form-base NEWOBJECT should preserve defining PRG provenance");
            expect(leaf_object.base_class_name == "Form",
                   "native Form-base NEWOBJECT should preserve the builtin Form base token");
            expect(leaf_object.class_library.empty(),
                   "native Form-base NEWOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(leaf_object.class_hierarchy.size() == 3U,
                   "native Form-base NEWOBJECT should persist native class hierarchy including Form");
            if (leaf_object.class_hierarchy.size() == 3U)
            {
                expect(leaf_object.class_hierarchy[0] == "WORKERFORM",
                       "native Form-base NEWOBJECT should store the native class first");
                expect(leaf_object.class_hierarchy[1] == "FORM",
                       "native Form-base NEWOBJECT should store the builtin Form base second");
                expect(leaf_object.class_hierarchy[2] == "OBJECT",
                       "native Form-base NEWOBJECT should store the terminal OBJECT token third");
            }

            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while Form-base native activation lands");
        }

        const bool has_describe_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "WorkerForm.Describe";
        });
        expect(has_describe_invoke_event,
               "native Form-base activation should dispatch ordinary native method calls");

        fs::remove_all(temp_root, ignored);
    }

    void test_createobject_and_newobject_instantiate_same_prg_formset_native_class()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_formset_class";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_formset_class.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('WorkerFormSet')\n"
            "oLeaf = NEWOBJECT('WorkerFormSet')\n"
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
            "lDictSet = SETPEM(oDict, 'comparemode', 143)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS WorkerFormSet AS FormSet\n"
            "    Caption = 'WorkerSet'\n"
            "    FUNCTION Describe\n"
            "        LPARAMETERS tcPrefix\n"
            "        RETURN tcPrefix + ':' + THIS.Caption\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native FormSet-base class script should complete: ") + state.message +
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

        check("ccreatecaption", "WorkerSet");
        check("cleafcaption", "WorkerSet");
        check("ccreatedescribe", "prefix:WorkerSet");
        check("cleafdescribe", "leaf:WorkerSet");
        check("ccreatebaseclass", "FormSet");
        check("cleafbaseclass", "FormSet");
        check("ccreateclass", "WorkerFormSet");
        check("cleafclass", "WorkerFormSet");
        check("lcreatehasdescribe", "true");
        check("lleafhasdescribe", "true");
        check("lcreatehasbaseclass", "true");
        check("lleafhasbaseclass", "true");
        check("ldictset", "true");
        check("ndictcompare", "143");

        expect(state.ole_objects.size() == 3U,
               "native FormSet-base CREATEOBJECT/NEWOBJECT plus COM NEWOBJECT should register three runtime objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &create_object = state.ole_objects[0];
            const auto &leaf_object = state.ole_objects[1];
            expect(create_object.prog_id == "WorkerFormSet",
                   "native FormSet-base CREATEOBJECT should preserve the PRG class name");
            expect(create_object.source == main_path.string(),
                   "native FormSet-base CREATEOBJECT should preserve defining PRG provenance");
            expect(create_object.base_class_name == "FormSet",
                   "native FormSet-base CREATEOBJECT should preserve the builtin FormSet base token");
            expect(create_object.class_library.empty(),
                   "native FormSet-base CREATEOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(create_object.class_hierarchy.size() == 3U,
                   "native FormSet-base CREATEOBJECT should persist native class hierarchy including FormSet");
            if (create_object.class_hierarchy.size() == 3U)
            {
                expect(create_object.class_hierarchy[0] == "WORKERFORMSET",
                       "native FormSet-base CREATEOBJECT should store the native class first");
                expect(create_object.class_hierarchy[1] == "FORMSET",
                       "native FormSet-base CREATEOBJECT should store the builtin FormSet base second");
                expect(create_object.class_hierarchy[2] == "OBJECT",
                       "native FormSet-base CREATEOBJECT should store the terminal OBJECT token third");
            }

            expect(leaf_object.prog_id == "WorkerFormSet",
                   "native FormSet-base NEWOBJECT should preserve the PRG class name");
            expect(leaf_object.source == main_path.string(),
                   "native FormSet-base NEWOBJECT should preserve defining PRG provenance");
            expect(leaf_object.base_class_name == "FormSet",
                   "native FormSet-base NEWOBJECT should preserve the builtin FormSet base token");
            expect(leaf_object.class_library.empty(),
                   "native FormSet-base NEWOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(leaf_object.class_hierarchy.size() == 3U,
                   "native FormSet-base NEWOBJECT should persist native class hierarchy including FormSet");
            if (leaf_object.class_hierarchy.size() == 3U)
            {
                expect(leaf_object.class_hierarchy[0] == "WORKERFORMSET",
                       "native FormSet-base NEWOBJECT should store the native class first");
                expect(leaf_object.class_hierarchy[1] == "FORMSET",
                       "native FormSet-base NEWOBJECT should store the builtin FormSet base second");
                expect(leaf_object.class_hierarchy[2] == "OBJECT",
                       "native FormSet-base NEWOBJECT should store the terminal OBJECT token third");
            }

            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while FormSet-base native activation lands");
        }

        const bool has_describe_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "WorkerFormSet.Describe";
        });
        expect(has_describe_invoke_event,
               "native FormSet-base activation should dispatch ordinary native method calls");

        fs::remove_all(temp_root, ignored);
    }

}
