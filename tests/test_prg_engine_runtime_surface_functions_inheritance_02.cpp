#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_same_prg_native_parentclass_reflects_through_getpem_and_pemstatus()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_parentclass_reflection";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_parentclass_reflection.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildWidget')\n"
            "oPlain = CREATEOBJECT('Empty')\n"
            "cParentClass = GETPEM(oCreate, 'ParentClass')\n"
            "lHasParentClass = PEMSTATUS(oCreate, 'ParentClass', 1)\n"
            "lParentClassReadOnly = PEMSTATUS(oCreate, 'ParentClass', 5)\n"
            "xPlainParentClass = GETPEM(oPlain, 'ParentClass')\n"
            "lPlainHasParentClass = PEMSTATUS(oPlain, 'ParentClass', 1)\n"
            "RETURN\n"
            "DEFINE CLASS RootWidget AS Custom\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ChildWidget AS RootWidget\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native ParentClass reflection script should complete: ") + state.message +
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

        check("cparentclass", "RootWidget");
        check("lhasparentclass", "true");
        check("lparentclassreadonly", "true");
        check("lplainhasparentclass", "false");

        const auto plain_parent_class = state.globals.find("xplainparentclass");
        expect(plain_parent_class != state.globals.end() &&
                   plain_parent_class->second.kind == copperfin::runtime::PrgValueKind::empty,
               "plain CREATEOBJECT should keep native ParentClass reflection empty");

        expect(state.ole_objects.size() == 2U,
               "native ParentClass reflection should register native and plain objects");
        if (state.ole_objects.size() == 2U)
        {
            const auto &native_object = state.ole_objects[0];
            expect(native_object.prog_id == "ChildWidget",
                   "native ParentClass reflection should preserve child class identity");
            expect(native_object.base_class_name == "RootWidget",
                   "native ParentClass reflection should preserve the immediate parent class name");

            expect(state.ole_objects[1].prog_id == "Empty",
                   "plain CREATEOBJECT should remain stable while native ParentClass reflection lands");
            expect(state.ole_objects[1].base_class_name.empty(),
                   "plain CREATEOBJECT should not fabricate native ParentClass metadata");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_external_prg_parentclass_reflects_through_getpem_and_pemstatus()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_prg_parentclass_reflection";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path library_path = temp_root / "widgetlib.prg";
        write_text(
            library_path,
            "DEFINE CLASS ParentWidget AS Custom\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_parentclass_reflection.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildWidget')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 33)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "cParentClass = GETPEM(oCreate, 'ParentClass')\n"
            "lHasParentClass = PEMSTATUS(oCreate, 'ParentClass', 1)\n"
            "lParentClassReadOnly = PEMSTATUS(oCreate, 'ParentClass', 5)\n"
            "RETURN\n"
            "DEFINE CLASS ChildWidget AS ParentWidget OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external-base ParentClass reflection script should complete: ") + state.message +
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

        check("cparentclass", "ParentWidget");
        check("lhasparentclass", "true");
        check("lparentclassreadonly", "true");
        check("ldictset", "true");
        check("ndictcompare", "33");

        expect(state.ole_objects.size() == 2U,
               "external-base ParentClass reflection should register native and COM objects");
        if (state.ole_objects.size() == 2U)
        {
            const auto &native_object = state.ole_objects[0];
            expect(native_object.prog_id == "ChildWidget",
                   "external-base ParentClass reflection should preserve child class identity");
            expect(native_object.base_class_name == "ParentWidget",
                   "external-base ParentClass reflection should preserve the immediate external parent class name");
            expect(native_object.class_library == library_path.string(),
                   "external-base ParentClass reflection should preserve external library provenance");

            expect(state.ole_objects[1].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external-base ParentClass reflection lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_same_prg_native_identity_metadata_appears_in_amembers()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_identity_amembers";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_identity_amembers.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildWidget')\n"
            "nMembersProps = AMEMBERS(aMembersProps, oCreate, 1)\n"
            "nMembersUnion = AMEMBERS(aMembersUnion, oCreate, 3)\n"
            "cProp1 = aMembersProps[1]\n"
            "cProp2 = aMembersProps[2]\n"
            "cProp3 = aMembersProps[3]\n"
            "cProp4 = aMembersProps[4]\n"
            "cUnion1 = aMembersUnion[1]\n"
            "cUnion4 = aMembersUnion[4]\n"
            "RETURN\n"
            "DEFINE CLASS ParentWidget AS Custom\n"
            "    Caption = 'Parent'\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ChildWidget AS ParentWidget\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native identity AMEMBERS script should complete: ") + state.message +
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

        check("nmembersprops", "4");
        check("nmembersunion", "11");
        check("cprop1", "BASECLASS");
        check("cprop2", "CAPTION");
        check("cprop3", "CLASS");
        check("cprop4", "PARENTCLASS");
        check("cunion1", "BASECLASS");
        check("cunion4", "PARENTCLASS");

        expect(state.ole_objects.size() == 1U,
               "native identity AMEMBERS should register one native object");
        if (state.ole_objects.size() == 1U)
        {
            expect(state.ole_objects[0].prog_id == "ChildWidget",
                   "native identity AMEMBERS should preserve child class identity");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_external_prg_base_identity_metadata_appears_in_amembers()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_prg_identity_amembers";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path library_path = temp_root / "widgetlib.prg";
        write_text(
            library_path,
            "DEFINE CLASS ParentWidget AS Custom\n"
            "    Caption = 'Parent'\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_identity_amembers.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildWidget')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 27)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "nMembersProps = AMEMBERS(aMembersProps, oCreate, 1)\n"
            "nMembersUnion = AMEMBERS(aMembersUnion, oCreate, 3)\n"
            "cProp1 = aMembersProps[1]\n"
            "cProp2 = aMembersProps[2]\n"
            "cProp3 = aMembersProps[3]\n"
            "cProp4 = aMembersProps[4]\n"
            "cProp5 = aMembersProps[5]\n"
            "cUnion1 = aMembersUnion[1]\n"
            "cUnion5 = aMembersUnion[5]\n"
            "RETURN\n"
            "DEFINE CLASS ChildWidget AS ParentWidget OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external identity AMEMBERS script should complete: ") + state.message +
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

        check("nmembersprops", "5");
        check("nmembersunion", "12");
        check("cprop1", "BASECLASS");
        check("cprop2", "CAPTION");
        check("cprop3", "CLASS");
        check("cprop4", "CLASSLIBRARY");
        check("cprop5", "PARENTCLASS");
        check("cunion1", "BASECLASS");
        check("cunion5", "PARENTCLASS");
        check("ldictset", "true");
        check("ndictcompare", "27");

        expect(state.ole_objects.size() == 2U,
               "external identity AMEMBERS should register native and COM objects");
        if (state.ole_objects.size() == 2U)
        {
            expect(state.ole_objects[0].prog_id == "ChildWidget",
                   "external identity AMEMBERS should preserve child class identity");
            expect(state.ole_objects[1].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external identity AMEMBERS lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_same_prg_child_aclass_reflects_inheritance_chain()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_child_aclass_inheritance";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_child_aclass_inheritance.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('DemoForm')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 60)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "nClassCount = ACLASS(aClass, oCreate.cmdSave)\n"
            "cClass1 = aClass[1]\n"
            "cClass2 = aClass[2]\n"
            "cClass3 = aClass[3]\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native child ACLASS inheritance script should complete: ") + state.message +
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

        check("nclasscount", "3");
        check("cclass1", "SAVEBUTTON");
        check("cclass2", "CUSTOM");
        check("cclass3", "OBJECT");
        check("ldictset", "true");
        check("ndictcompare", "60");

        expect(state.ole_objects.size() == 3U,
               "native child ACLASS inheritance should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "DemoForm",
                   "native child ACLASS inheritance should preserve parent form identity");
            const auto &child_object = state.ole_objects[1];
            expect(child_object.prog_id == "SaveButton",
                   "native child ACLASS inheritance should preserve child class identity");
            expect(child_object.class_hierarchy.size() == 3U,
                   "native child ACLASS inheritance should persist the child native class hierarchy");
            if (child_object.class_hierarchy.size() == 3U)
            {
                expect(child_object.class_hierarchy[0] == "SAVEBUTTON",
                       "native child ACLASS inheritance should store the child class first");
                expect(child_object.class_hierarchy[1] == "CUSTOM",
                       "native child ACLASS inheritance should store the builtin base second");
                expect(child_object.class_hierarchy[2] == "OBJECT",
                       "native child ACLASS inheritance should store the terminal object token");
            }
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native child ACLASS inheritance lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_external_base_child_aclass_reflects_inheritance_chain()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_prg_child_aclass_inheritance";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path library_path = temp_root / "widgetlib.prg";
        write_text(
            library_path,
            "DEFINE CLASS ParentForm AS Custom\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_child_aclass_inheritance.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 61)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "nClassCount = ACLASS(aClass, oCreate.cmdSave)\n"
            "cClass1 = aClass[1]\n"
            "cClass2 = aClass[2]\n"
            "cClass3 = aClass[3]\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external child ACLASS inheritance script should complete: ") + state.message +
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

        check("nclasscount", "3");
        check("cclass1", "SAVEBUTTON");
        check("cclass2", "CUSTOM");
        check("cclass3", "OBJECT");
        check("ldictset", "true");
        check("ndictcompare", "61");

        expect(state.ole_objects.size() == 3U,
               "external child ACLASS inheritance should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "ChildForm",
                   "external child ACLASS inheritance should preserve parent form identity");
            const auto &child_object = state.ole_objects[1];
            expect(child_object.prog_id == "SaveButton",
                   "external child ACLASS inheritance should preserve child class identity");
            expect(child_object.class_hierarchy.size() == 3U,
                   "external child ACLASS inheritance should persist the child native class hierarchy");
            if (child_object.class_hierarchy.size() == 3U)
            {
                expect(child_object.class_hierarchy[0] == "SAVEBUTTON",
                       "external child ACLASS inheritance should store the child class first");
                expect(child_object.class_hierarchy[1] == "CUSTOM",
                       "external child ACLASS inheritance should store the builtin base second");
                expect(child_object.class_hierarchy[2] == "OBJECT",
                       "external child ACLASS inheritance should store the terminal object token");
            }
            expect(child_object.source == library_path.string(),
                   "external child ACLASS inheritance should preserve the defining PRG path as child provenance");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external child ACLASS inheritance lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_same_prg_inherited_child_aclass_reflects_deeper_inheritance_chain()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_inherited_child_aclass_inheritance";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_inherited_child_aclass_inheritance.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('DemoForm')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 62)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "nClassCount = ACLASS(aClass, oCreate.cmdSave)\n"
            "cClass1 = aClass[1]\n"
            "cClass2 = aClass[2]\n"
            "cClass3 = aClass[3]\n"
            "cClass4 = aClass[4]\n"
            "cClass5 = aClass[5]\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS RootButton AS Custom\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ParentButton AS RootButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS ParentButton\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native inherited child ACLASS script should complete: ") + state.message +
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
        check("ldictset", "true");
        check("ndictcompare", "62");

        expect(state.ole_objects.size() == 3U,
               "native inherited child ACLASS should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "DemoForm",
                   "native inherited child ACLASS should preserve parent form identity");
            const auto &child_object = state.ole_objects[1];
            expect(child_object.prog_id == "SaveButton",
                   "native inherited child ACLASS should preserve child class identity");
            expect(child_object.class_hierarchy.size() == 5U,
                   "native inherited child ACLASS should persist the deeper child native class hierarchy");
            if (child_object.class_hierarchy.size() == 5U)
            {
                expect(child_object.class_hierarchy[0] == "SAVEBUTTON",
                       "native inherited child ACLASS should store the derived child class first");
                expect(child_object.class_hierarchy[1] == "PARENTBUTTON",
                       "native inherited child ACLASS should store the intermediate child class second");
                expect(child_object.class_hierarchy[2] == "ROOTBUTTON",
                       "native inherited child ACLASS should store the deeper child ancestor third");
                expect(child_object.class_hierarchy[3] == "CUSTOM",
                       "native inherited child ACLASS should store the builtin base token");
                expect(child_object.class_hierarchy[4] == "OBJECT",
                       "native inherited child ACLASS should store the terminal object token");
            }
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native inherited child ACLASS lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_external_base_inherited_child_aclass_reflects_deeper_inheritance_chain()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_prg_inherited_child_aclass_inheritance";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path library_path = temp_root / "widgetlib.prg";
        write_text(
            library_path,
            "DEFINE CLASS ParentForm AS Custom\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS RootButton AS Custom\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ParentButton AS RootButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS ParentButton\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_inherited_child_aclass_inheritance.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 63)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "nClassCount = ACLASS(aClass, oCreate.cmdSave)\n"
            "cClass1 = aClass[1]\n"
            "cClass2 = aClass[2]\n"
            "cClass3 = aClass[3]\n"
            "cClass4 = aClass[4]\n"
            "cClass5 = aClass[5]\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external inherited child ACLASS script should complete: ") + state.message +
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
        check("ldictset", "true");
        check("ndictcompare", "63");

        expect(state.ole_objects.size() == 3U,
               "external inherited child ACLASS should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "ChildForm",
                   "external inherited child ACLASS should preserve parent form identity");
            const auto &child_object = state.ole_objects[1];
            expect(child_object.prog_id == "SaveButton",
                   "external inherited child ACLASS should preserve child class identity");
            expect(child_object.class_hierarchy.size() == 5U,
                   "external inherited child ACLASS should persist the deeper child native class hierarchy");
            if (child_object.class_hierarchy.size() == 5U)
            {
                expect(child_object.class_hierarchy[0] == "SAVEBUTTON",
                       "external inherited child ACLASS should store the derived child class first");
                expect(child_object.class_hierarchy[1] == "PARENTBUTTON",
                       "external inherited child ACLASS should store the intermediate child class second");
                expect(child_object.class_hierarchy[2] == "ROOTBUTTON",
                       "external inherited child ACLASS should store the deeper child ancestor third");
                expect(child_object.class_hierarchy[3] == "CUSTOM",
                       "external inherited child ACLASS should store the builtin base token");
                expect(child_object.class_hierarchy[4] == "OBJECT",
                       "external inherited child ACLASS should store the terminal object token");
            }
            expect(child_object.source == library_path.string(),
                   "external inherited child ACLASS should preserve the defining PRG path as child provenance");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external inherited child ACLASS lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_same_prg_native_identity_metadata_stays_read_only_to_setpem()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_identity_setpem";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_identity_setpem.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildWidget')\n"
            "lSetClass = SETPEM(oCreate, 'Class', 'OtherClass')\n"
            "lSetBaseClass = SETPEM(oCreate, 'BaseClass', 'OtherBase')\n"
            "lSetParentClass = SETPEM(oCreate, 'ParentClass', 'OtherParent')\n"
            "lSetClassLibrary = SETPEM(oCreate, 'ClassLibrary', 'other.prg')\n"
            "cClassAfter = GETPEM(oCreate, 'Class')\n"
            "cBaseClassAfter = GETPEM(oCreate, 'BaseClass')\n"
            "cParentClassAfter = GETPEM(oCreate, 'ParentClass')\n"
            "xClassLibraryAfter = GETPEM(oCreate, 'ClassLibrary')\n"
            "RETURN\n"
            "DEFINE CLASS ParentWidget AS Custom\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ChildWidget AS ParentWidget\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native identity SETPEM script should complete: ") + state.message +
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

        check("lsetclass", "false");
        check("lsetbaseclass", "false");
        check("lsetparentclass", "false");
        check("lsetclasslibrary", "false");
        check("cclassafter", "ChildWidget");
        check("cbaseclassafter", "ParentWidget");
        check("cparentclassafter", "ParentWidget");

        const auto class_library_after = state.globals.find("xclasslibraryafter");
        expect(class_library_after != state.globals.end() &&
                   class_library_after->second.kind == copperfin::runtime::PrgValueKind::empty,
               "same-PRG identity SETPEM should leave ClassLibrary empty after failed mutation");

        expect(state.ole_objects.size() == 1U,
               "native identity SETPEM should register one native object");
        if (state.ole_objects.size() == 1U)
        {
            const auto &native_object = state.ole_objects[0];
            expect(native_object.prog_id == "ChildWidget",
                   "native identity SETPEM should preserve child class identity");
            expect(native_object.base_class_name == "ParentWidget",
                   "native identity SETPEM should preserve the immediate base class");
            expect(native_object.class_library.empty(),
                   "native identity SETPEM should preserve empty same-PRG class library provenance");
            expect(!native_object.properties.contains("class"),
                   "native identity SETPEM should not materialize a writable Class property shadow");
            expect(!native_object.properties.contains("baseclass"),
                   "native identity SETPEM should not materialize a writable BaseClass property shadow");
            expect(!native_object.properties.contains("parentclass"),
                   "native identity SETPEM should not materialize a writable ParentClass property shadow");
            expect(!native_object.properties.contains("classlibrary"),
                   "native identity SETPEM should not materialize a writable ClassLibrary property shadow");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_external_prg_identity_metadata_stays_read_only_to_setpem()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_prg_identity_setpem";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path library_path = temp_root / "widgetlib.prg";
        write_text(
            library_path,
            "DEFINE CLASS ParentWidget AS Custom\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_identity_setpem.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildWidget')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 28)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "lSetClass = SETPEM(oCreate, 'Class', 'OtherClass')\n"
            "lSetBaseClass = SETPEM(oCreate, 'BaseClass', 'OtherBase')\n"
            "lSetParentClass = SETPEM(oCreate, 'ParentClass', 'OtherParent')\n"
            "lSetClassLibrary = SETPEM(oCreate, 'ClassLibrary', 'other.prg')\n"
            "cClassAfter = GETPEM(oCreate, 'Class')\n"
            "cBaseClassAfter = GETPEM(oCreate, 'BaseClass')\n"
            "cParentClassAfter = GETPEM(oCreate, 'ParentClass')\n"
            "cClassLibraryAfter = GETPEM(oCreate, 'ClassLibrary')\n"
            "RETURN\n"
            "DEFINE CLASS ChildWidget AS ParentWidget OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external identity SETPEM script should complete: ") + state.message +
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

        check("lsetclass", "false");
        check("lsetbaseclass", "false");
        check("lsetparentclass", "false");
        check("lsetclasslibrary", "false");
        check("cclassafter", "ChildWidget");
        check("cbaseclassafter", "ParentWidget");
        check("cparentclassafter", "ParentWidget");
        check("cclasslibraryafter", library_path.string());
        check("ldictset", "true");
        check("ndictcompare", "28");

        expect(state.ole_objects.size() == 2U,
               "external identity SETPEM should register native and COM objects");
        if (state.ole_objects.size() == 2U)
        {
            const auto &native_object = state.ole_objects[0];
            expect(native_object.prog_id == "ChildWidget",
                   "external identity SETPEM should preserve child class identity");
            expect(native_object.base_class_name == "ParentWidget",
                   "external identity SETPEM should preserve the immediate external base class");
            expect(native_object.class_library == library_path.string(),
                   "external identity SETPEM should preserve external class library provenance");
            expect(!native_object.properties.contains("class"),
                   "external identity SETPEM should not materialize a writable Class property shadow");
            expect(!native_object.properties.contains("baseclass"),
                   "external identity SETPEM should not materialize a writable BaseClass property shadow");
            expect(!native_object.properties.contains("parentclass"),
                   "external identity SETPEM should not materialize a writable ParentClass property shadow");
            expect(!native_object.properties.contains("classlibrary"),
                   "external identity SETPEM should not materialize a writable ClassLibrary property shadow");

            expect(state.ole_objects[1].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external identity SETPEM lands");
        }

        fs::remove_all(temp_root, ignored);
    }

}
