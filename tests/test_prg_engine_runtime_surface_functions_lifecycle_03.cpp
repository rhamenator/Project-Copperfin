#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_createobject_and_newobject_instantiate_same_prg_exception_native_class()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_exception_class";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_exception_class.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('WorkerException')\n"
            "oLeaf = NEWOBJECT('WorkerException')\n"
            "cCreateContext = oCreate.cContext\n"
            "cLeafContext = oLeaf.cContext\n"
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
            "lDictSet = SETPEM(oDict, 'comparemode', 175)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS WorkerException AS Exception\n"
            "    cContext = 'workerexception'\n"
            "    FUNCTION Describe\n"
            "        LPARAMETERS tcPrefix\n"
            "        RETURN tcPrefix + ':' + THIS.cContext\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Exception-base class script should complete: ") + state.message +
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

        check("ccreatecontext", "workerexception");
        check("cleafcontext", "workerexception");
        check("ccreatedescribe", "prefix:workerexception");
        check("cleafdescribe", "leaf:workerexception");
        check("ccreatebaseclass", "Exception");
        check("cleafbaseclass", "Exception");
        check("ccreateclass", "WorkerException");
        check("cleafclass", "WorkerException");
        check("lcreatehasdescribe", "true");
        check("lleafhasdescribe", "true");
        check("lcreatehasbaseclass", "true");
        check("lleafhasbaseclass", "true");
        check("ldictset", "true");
        check("ndictcompare", "175");

        expect(state.ole_objects.size() == 3U,
               "native Exception-base CREATEOBJECT/NEWOBJECT plus COM NEWOBJECT should register three runtime objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &create_object = state.ole_objects[0];
            const auto &leaf_object = state.ole_objects[1];
            expect(create_object.prog_id == "WorkerException",
                   "native Exception-base CREATEOBJECT should preserve the PRG class name");
            expect(create_object.source == main_path.string(),
                   "native Exception-base CREATEOBJECT should preserve defining PRG provenance");
            expect(create_object.base_class_name == "Exception",
                   "native Exception-base CREATEOBJECT should preserve the builtin Exception base token");
            expect(create_object.class_library.empty(),
                   "native Exception-base CREATEOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(create_object.class_hierarchy.size() == 3U,
                   "native Exception-base CREATEOBJECT should persist native class hierarchy including Exception");
            if (create_object.class_hierarchy.size() == 3U)
            {
                expect(create_object.class_hierarchy[0] == "WORKEREXCEPTION",
                       "native Exception-base CREATEOBJECT should store the native class first");
                expect(create_object.class_hierarchy[1] == "EXCEPTION",
                       "native Exception-base CREATEOBJECT should store the builtin Exception base second");
                expect(create_object.class_hierarchy[2] == "OBJECT",
                       "native Exception-base CREATEOBJECT should store the terminal OBJECT token third");
            }

            expect(leaf_object.prog_id == "WorkerException",
                   "native Exception-base NEWOBJECT should preserve the PRG class name");
            expect(leaf_object.source == main_path.string(),
                   "native Exception-base NEWOBJECT should preserve defining PRG provenance");
            expect(leaf_object.base_class_name == "Exception",
                   "native Exception-base NEWOBJECT should preserve the builtin Exception base token");
            expect(leaf_object.class_library.empty(),
                   "native Exception-base NEWOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(leaf_object.class_hierarchy.size() == 3U,
                   "native Exception-base NEWOBJECT should persist native class hierarchy including Exception");
            if (leaf_object.class_hierarchy.size() == 3U)
            {
                expect(leaf_object.class_hierarchy[0] == "WORKEREXCEPTION",
                       "native Exception-base NEWOBJECT should store the native class first");
                expect(leaf_object.class_hierarchy[1] == "EXCEPTION",
                       "native Exception-base NEWOBJECT should store the builtin Exception base second");
                expect(leaf_object.class_hierarchy[2] == "OBJECT",
                       "native Exception-base NEWOBJECT should store the terminal OBJECT token third");
            }

            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while Exception-base native activation lands");
        }

        const bool has_describe_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "WorkerException.Describe";
        });
        expect(has_describe_invoke_event,
               "native Exception-base activation should dispatch ordinary native method calls");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_addobject_materializes_child_objects_and_child_methods_see_parent()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_addobject";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_addobject.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('DemoForm')\n"
            "oChild = oForm.cmdSave\n"
            "lHasChild = PEMSTATUS(oForm, 'cmdSave', 1)\n"
            "lChildHasParent = PEMSTATUS(oChild, 'Parent', 1)\n"
            "lChildAdded = oForm.lChildAdded\n"
            "cFormCaption = oForm.Caption\n"
            "cChildCaption = oChild.Caption\n"
            "oParent = oChild.Parent\n"
            "cParentCaption = oParent.Caption\n"
            "cOwnerCaption = oChild.OwnerCaption()\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 13)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    lChildAdded = .F.\n"
            "    PROCEDURE Init\n"
            "        THIS.lChildAdded = THIS.AddObject('cmdSave', 'SaveButton')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
            "    Caption = 'Save'\n"
            "    FUNCTION OwnerCaption\n"
            "        RETURN PARENT.Caption\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native ADDOBJECT script should complete: ") + state.message +
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

        check("lhaschild", "true");
        check("lchildhasparent", "true");
        check("lchildadded", "true");
        check("cformcaption", "MainForm");
        check("cchildcaption", "Save");
        check("cparentcaption", "MainForm");
        check("cownercaption", "MainForm");
        check("ldictset", "true");
        check("ndictcompare", "13");

        expect(state.ole_objects.size() == 3U,
               "native ADDOBJECT script should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &parent_object = state.ole_objects[0];
            const auto &child_object = state.ole_objects[1];
            expect(parent_object.prog_id == "DemoForm",
                   "native ADDOBJECT should preserve the parent class identity");
            expect(child_object.prog_id == "SaveButton",
                   "native ADDOBJECT should preserve the child class identity");
            expect(parent_object.properties.contains("cmdsave"),
                   "native ADDOBJECT should materialize the child reference on the parent");
            const auto child_parent = child_object.properties.find("parent");
            if (child_parent != child_object.properties.end())
            {
                expect(child_parent->second.kind == copperfin::runtime::PrgValueKind::string,
                       "native ADDOBJECT should persist a PARENT object reference on the child");
            }
            else
            {
                expect(false, "native ADDOBJECT should materialize the child PARENT reference");
            }
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM behavior should remain stable while native ADDOBJECT lands");
        }

        const bool has_addobject_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.addobject" &&
                   event.detail == "DemoForm.cmdsave:SaveButton";
        });
        expect(has_addobject_event,
               "native ADDOBJECT should emit child-activation events");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_addobject_materializes_external_prg_child_objects_and_preserves_init_flow()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_addobject_external_library";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path library_path = temp_root / "buttons.prg";
        write_text(
            library_path,
            "DEFINE CLASS SaveButton AS Custom\n"
            "    Caption = 'Save'\n"
            "    nSeed = 0\n"
            "    PROCEDURE Init\n"
            "        LPARAMETERS tnSeed\n"
            "        THIS.nSeed = tnSeed\n"
            "        THIS.Caption = THIS.Caption + '-' + ALLTRIM(STR(tnSeed))\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "    FUNCTION OwnerCaption\n"
            "        RETURN PARENT.Caption\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "native_addobject_external_library.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('DemoForm')\n"
            "oChild = oForm.cmdSave\n"
            "lHasChild = PEMSTATUS(oForm, 'cmdSave', 1)\n"
            "lChildHasParent = PEMSTATUS(oChild, 'Parent', 1)\n"
            "lChildAdded = oForm.lChildAdded\n"
            "cInitChildCaption = oForm.cInitChildCaption\n"
            "cInitOwnerCaption = oForm.cInitOwnerCaption\n"
            "nChildSeed = oChild.nSeed\n"
            "cChildCaption = oChild.Caption\n"
            "cOwnerCaption = oChild.OwnerCaption()\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 14)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    lChildAdded = .F.\n"
            "    cInitChildCaption = ''\n"
            "    cInitOwnerCaption = ''\n"
            "    PROCEDURE Init\n"
            "        THIS.lChildAdded = THIS.AddObject('cmdSave', 'SaveButton', 'buttons.prg', 7)\n"
            "        THIS.cInitChildCaption = THIS.cmdSave.Caption\n"
            "        THIS.cInitOwnerCaption = THIS.cmdSave.OwnerCaption()\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native external-library ADDOBJECT script should complete: ") + state.message +
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

        check("lhaschild", "true");
        check("lchildhasparent", "true");
        check("lchildadded", "true");
        check("cinitchildcaption", "Save-7");
        check("cinitownercaption", "MainForm");
        check("nchildseed", "7");
        check("cchildcaption", "Save-7");
        check("cownercaption", "MainForm");
        check("ldictset", "true");
        check("ndictcompare", "14");

        expect(state.ole_objects.size() == 3U,
               "native external-library ADDOBJECT script should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &parent_object = state.ole_objects[0];
            const auto &child_object = state.ole_objects[1];
            expect(parent_object.prog_id == "DemoForm",
                   "native external-library ADDOBJECT should preserve the parent class identity");
            expect(child_object.prog_id == "SaveButton",
                   "native external-library ADDOBJECT should preserve the child class identity");
            expect(child_object.source == library_path.string(),
                   "native external-library ADDOBJECT should preserve the resolved PRG library path as child provenance");
            expect(parent_object.properties.contains("cmdsave"),
                   "native external-library ADDOBJECT should materialize the child reference on the parent");
            const auto child_parent = child_object.properties.find("parent");
            const auto child_seed = child_object.properties.find("nseed");
            if (child_parent != child_object.properties.end())
            {
                expect(child_parent->second.kind == copperfin::runtime::PrgValueKind::string,
                       "native external-library ADDOBJECT should persist a PARENT object reference on the child");
            }
            else
            {
                expect(false, "native external-library ADDOBJECT should materialize the child PARENT reference");
            }
            if (child_seed != child_object.properties.end())
            {
                expect(copperfin::runtime::format_value(child_seed->second) == "7",
                       "native external-library ADDOBJECT should preserve child Init constructor values");
            }
            else
            {
                expect(false, "native external-library ADDOBJECT should materialize child Init state");
            }

            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM behavior should remain stable while external-library ADDOBJECT lands");
        }

        const bool has_addobject_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.addobject" &&
                   event.detail == "DemoForm.cmdsave:SaveButton";
        });
        expect(has_addobject_event,
               "native external-library ADDOBJECT should emit child materialization events");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_addobject_external_child_base_surfaces_classlibrary_provenance()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_addobject_external_child_base_provenance";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path root_library_path = temp_root / "rootbuttons.prg";
        write_text(
            root_library_path,
            "DEFINE CLASS ParentButton AS Custom\n"
            "ENDDEFINE\n");

        const fs::path button_library_path = temp_root / "buttons.prg";
        write_text(
            button_library_path,
            "DEFINE CLASS SaveButton AS ParentButton OF rootbuttons.prg\n"
            "    Caption = 'Save'\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "native_addobject_external_child_base_provenance.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('DemoForm')\n"
            "oChild = oForm.cmdSave\n"
            "cChildClassLibrary = GETPEM(oChild, 'ClassLibrary')\n"
            "lChildHasClassLibrary = PEMSTATUS(oChild, 'ClassLibrary', 1)\n"
            "lChildClassLibraryReadOnly = PEMSTATUS(oChild, 'ClassLibrary', 5)\n"
            "cChildClassLibraryProp = oChild.ClassLibrary\n"
            "nMembersProps = AMEMBERS(aMembersProps, oChild, 1)\n"
            "cProp4 = aMembersProps[4]\n"
            "lSetChildClassLibrary = SETPEM(oChild, 'ClassLibrary', 'shadow.prg')\n"
            "oChild.ClassLibrary = 'shadow2.prg'\n"
            "cChildClassLibraryAfter = GETPEM(oChild, 'ClassLibrary')\n"
            "cChildClassLibraryPropAfter = oChild.ClassLibrary\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 68)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton', 'buttons.prg')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native ADDOBJECT external child-base provenance script should complete: ") + state.message +
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

        check("cchildclasslibrary", root_library_path.string());
        check("lchildhasclasslibrary", "true");
        check("lchildclasslibraryreadonly", "true");
        check("cchildclasslibraryprop", root_library_path.string());
        check("nmembersprops", "6");
        check("cprop4", "CLASSLIBRARY");
        check("lsetchildclasslibrary", "false");
        check("cchildclasslibraryafter", root_library_path.string());
        check("cchildclasslibrarypropafter", root_library_path.string());
        check("ldictset", "true");
        check("ndictcompare", "68");

        expect(state.ole_objects.size() == 3U,
               "native ADDOBJECT external child-base provenance should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &parent_object = state.ole_objects[0];
            const auto &child_object = state.ole_objects[1];
            expect(parent_object.prog_id == "DemoForm",
                   "native ADDOBJECT external child-base provenance should preserve parent identity");
            expect(child_object.prog_id == "SaveButton",
                   "native ADDOBJECT external child-base provenance should preserve child identity");
            expect(child_object.source == button_library_path.string(),
                   "native ADDOBJECT external child-base provenance should preserve the child definition source path");
            expect(child_object.class_library == root_library_path.string(),
                   "native ADDOBJECT external child-base provenance should preserve the external child ClassLibrary path");
            expect(!child_object.properties.contains("classlibrary"),
                   "native ADDOBJECT external child-base provenance should not materialize a ClassLibrary shadow");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native ADDOBJECT external child-base provenance lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_native_addobject_external_child_base_aclass_reflects_inheritance_chain()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_addobject_external_child_base_aclass";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path root_library_path = temp_root / "rootbuttons.prg";
        write_text(
            root_library_path,
            "DEFINE CLASS ParentButton AS Custom\n"
            "ENDDEFINE\n");

        const fs::path button_library_path = temp_root / "buttons.prg";
        write_text(
            button_library_path,
            "DEFINE CLASS SaveButton AS ParentButton OF rootbuttons.prg\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "native_addobject_external_child_base_aclass.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('DemoForm')\n"
            "oChild = oForm.cmdSave\n"
            "nClassCount = ACLASS(aClass, oChild)\n"
            "cClass1 = aClass[1]\n"
            "cClass2 = aClass[2]\n"
            "cClass3 = aClass[3]\n"
            "cClass4 = aClass[4]\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton', 'buttons.prg')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native ADDOBJECT external child-base ACLASS script should complete: ") + state.message +
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

        check("nclasscount", "4");
        check("cclass1", "SAVEBUTTON");
        check("cclass2", "PARENTBUTTON");
        check("cclass3", "CUSTOM");
        check("cclass4", "OBJECT");

        expect(state.ole_objects.size() == 2U,
               "native ADDOBJECT external child-base ACLASS should register parent and child objects");
        if (state.ole_objects.size() == 2U)
        {
            const auto &child_object = state.ole_objects[1];
            expect(child_object.prog_id == "SaveButton",
                   "native ADDOBJECT external child-base ACLASS should preserve child identity");
            expect(child_object.class_library == root_library_path.string(),
                   "native ADDOBJECT external child-base ACLASS should preserve external child ClassLibrary path");
            expect(child_object.class_hierarchy.size() == 4U,
                   "native ADDOBJECT external child-base ACLASS should preserve runtime child class hierarchy");
            if (child_object.class_hierarchy.size() == 4U)
            {
                expect(child_object.class_hierarchy[0] == "SAVEBUTTON",
                       "native ADDOBJECT external child-base ACLASS should store the derived child class first");
                expect(child_object.class_hierarchy[1] == "PARENTBUTTON",
                       "native ADDOBJECT external child-base ACLASS should store the external parent class second");
                expect(child_object.class_hierarchy[2] == "CUSTOM",
                       "native ADDOBJECT external child-base ACLASS should store the builtin base token");
                expect(child_object.class_hierarchy[3] == "OBJECT",
                       "native ADDOBJECT external child-base ACLASS should store the terminal object token");
            }
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_native_addobject_deeper_external_child_base_aclass_reflects_inheritance_chain()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_addobject_deeper_external_child_base_aclass";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path root_library_path = temp_root / "rootbuttons.prg";
        write_text(
            root_library_path,
            "DEFINE CLASS RootButton AS Custom\n"
            "ENDDEFINE\n");

        const fs::path button_library_path = temp_root / "buttons.prg";
        write_text(
            button_library_path,
            "DEFINE CLASS ParentButton AS RootButton OF rootbuttons.prg\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS ParentButton\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "native_addobject_deeper_external_child_base_aclass.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('DemoForm')\n"
            "oChild = oForm.cmdSave\n"
            "nClassCount = ACLASS(aClass, oChild)\n"
            "cClass1 = aClass[1]\n"
            "cClass2 = aClass[2]\n"
            "cClass3 = aClass[3]\n"
            "cClass4 = aClass[4]\n"
            "cClass5 = aClass[5]\n"
            "xChildClassLibrary = GETPEM(oChild, 'ClassLibrary')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton', 'buttons.prg')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native ADDOBJECT deeper external child-base ACLASS script should complete: ") + state.message +
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

        check("nclasscount", "5");
        check("cclass1", "SAVEBUTTON");
        check("cclass2", "PARENTBUTTON");
        check("cclass3", "ROOTBUTTON");
        check("cclass4", "CUSTOM");
        check("cclass5", "OBJECT");
        const auto child_class_library = state.globals.find("xchildclasslibrary");
        expect(child_class_library != state.globals.end() &&
                   child_class_library->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native ADDOBJECT deeper external child-base ACLASS should leave the derived child ClassLibrary empty");

        expect(state.ole_objects.size() == 2U,
               "native ADDOBJECT deeper external child-base ACLASS should register parent and child objects");
        if (state.ole_objects.size() == 2U)
        {
            const auto &child_object = state.ole_objects[1];
            expect(child_object.prog_id == "SaveButton",
                   "native ADDOBJECT deeper external child-base ACLASS should preserve child identity");
            expect(child_object.source == button_library_path.string(),
                   "native ADDOBJECT deeper external child-base ACLASS should preserve the child definition source path");
            expect(child_object.class_library.empty(),
                   "native ADDOBJECT deeper external child-base ACLASS should leave the derived child ClassLibrary empty");
            expect(child_object.class_hierarchy.size() == 5U,
                   "native ADDOBJECT deeper external child-base ACLASS should preserve the deeper runtime child class hierarchy");
            if (child_object.class_hierarchy.size() == 5U)
            {
                expect(child_object.class_hierarchy[0] == "SAVEBUTTON",
                       "native ADDOBJECT deeper external child-base ACLASS should store the derived child class first");
                expect(child_object.class_hierarchy[1] == "PARENTBUTTON",
                       "native ADDOBJECT deeper external child-base ACLASS should store the intermediate external parent second");
                expect(child_object.class_hierarchy[2] == "ROOTBUTTON",
                       "native ADDOBJECT deeper external child-base ACLASS should store the deeper external ancestor third");
                expect(child_object.class_hierarchy[3] == "CUSTOM",
                       "native ADDOBJECT deeper external child-base ACLASS should store the builtin base token");
                expect(child_object.class_hierarchy[4] == "OBJECT",
                       "native ADDOBJECT deeper external child-base ACLASS should store the terminal object token");
            }
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_native_class_body_add_object_materializes_children_before_init()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_class_body_addobject";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_class_body_addobject.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('DemoForm')\n"
            "oNew = NEWOBJECT('DemoForm')\n"
            "lCreateHasChild = PEMSTATUS(oCreate, 'cmdSave', 1)\n"
            "lNewHasChild = PEMSTATUS(oNew, 'cmdSave', 1)\n"
            "lCreateInitSawChild = oCreate.lInitSawChild\n"
            "lNewInitSawChild = oNew.lInitSawChild\n"
            "cCreateInitChildCaption = oCreate.cInitChildCaption\n"
            "cNewInitChildCaption = oNew.cInitChildCaption\n"
            "cCreateOwnerCaption = oCreate.cmdSave.OwnerCaption()\n"
            "cNewOwnerCaption = oNew.cmdSave.OwnerCaption()\n"
            "lCreateChildHasParent = PEMSTATUS(oCreate.cmdSave, 'Parent', 1)\n"
            "lNewChildHasParent = PEMSTATUS(oNew.cmdSave, 'Parent', 1)\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 13)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    lInitSawChild = .F.\n"
            "    cInitChildCaption = ''\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "    PROCEDURE Init\n"
            "        THIS.lInitSawChild = PEMSTATUS(THIS, 'cmdSave', 1)\n"
            "        IF THIS.lInitSawChild\n"
            "            THIS.cInitChildCaption = THIS.cmdSave.Caption\n"
            "        ENDIF\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
            "    Caption = 'Save'\n"
            "    FUNCTION OwnerCaption\n"
            "        RETURN PARENT.Caption\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native class-body ADD OBJECT script should complete: ") + state.message +
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

        check("lcreatehaschild", "true");
        check("lnewhaschild", "true");
        check("lcreateinitsawchild", "true");
        check("lnewinitsawchild", "true");
        check("ccreateinitchildcaption", "Save");
        check("cnewinitchildcaption", "Save");
        check("ccreateownercaption", "MainForm");
        check("cnewownercaption", "MainForm");
        check("lcreatechildhasparent", "true");
        check("lnewchildhasparent", "true");
        check("ldictset", "true");
        check("ndictcompare", "13");

        expect(state.ole_objects.size() == 5U,
               "native class-body ADD OBJECT script should register CREATEOBJECT, NEWOBJECT, child, child, and COM objects");
        if (state.ole_objects.size() == 5U)
        {
            expect(state.ole_objects[0].prog_id == "DemoForm",
                   "class-body ADD OBJECT should preserve the CREATEOBJECT parent class identity");
            expect(state.ole_objects[1].prog_id == "SaveButton",
                   "class-body ADD OBJECT should materialize the CREATEOBJECT child class");
            expect(state.ole_objects[2].prog_id == "DemoForm",
                   "class-body ADD OBJECT should preserve the NEWOBJECT parent class identity");
            expect(state.ole_objects[3].prog_id == "SaveButton",
                   "class-body ADD OBJECT should materialize the NEWOBJECT child class");
            expect(state.ole_objects[4].prog_id == "Scripting.Dictionary",
                   "COM behavior should remain stable while class-body ADD OBJECT lands");
            expect(state.ole_objects[0].properties.contains("cmdsave"),
                   "class-body ADD OBJECT should materialize the child reference on the CREATEOBJECT parent");
            expect(state.ole_objects[2].properties.contains("cmdsave"),
                   "class-body ADD OBJECT should materialize the child reference on the NEWOBJECT parent");
        }

        const std::size_t addobject_event_count = static_cast<std::size_t>(std::count_if(
            state.events.begin(),
            state.events.end(),
            [](const auto &event)
            {
                return event.category == "prg.object.addobject" &&
                       event.detail == "DemoForm.cmdsave:SaveButton";
            }));
        expect(addobject_event_count == 2U,
               "class-body ADD OBJECT should emit addobject events for both CREATEOBJECT and NEWOBJECT activation");

        fs::remove_all(temp_root, ignored);
    }

}
