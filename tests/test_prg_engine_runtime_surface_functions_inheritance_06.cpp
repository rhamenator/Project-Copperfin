#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_same_prg_child_external_base_classlibrary_survives_identity_mutation_guards()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_child_external_base_classlibrary_guards";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path button_library_path = temp_root / "buttons.prg";
        write_text(
            button_library_path,
            "DEFINE CLASS ParentButton AS Custom\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "native_child_external_base_classlibrary_guards.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('DemoForm')\n"
            "lSetChildClassLibrary = SETPEM(oCreate.cmdSave, 'ClassLibrary', 'other.prg')\n"
            "lAddChildClassLibrary = ADDPROPERTY(oCreate.cmdSave, 'ClassLibrary', 'other.prg')\n"
            "lRemoveChildClassLibrary = REMOVEPROPERTY(oCreate.cmdSave, 'ClassLibrary')\n"
            "oCreate.cmdSave.ClassLibrary = 'shadow.prg'\n"
            "cChildClassLibraryAfter = GETPEM(oCreate.cmdSave, 'ClassLibrary')\n"
            "cChildClassLibraryPropAfter = oCreate.cmdSave.ClassLibrary\n"
            "lChildHasClassLibrary = PEMSTATUS(oCreate.cmdSave, 'ClassLibrary', 1)\n"
            "lChildClassLibraryReadOnly = PEMSTATUS(oCreate.cmdSave, 'ClassLibrary', 5)\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 66)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS ParentButton OF buttons.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native child external-base classlibrary guard script should complete: ") + state.message +
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

        check("lsetchildclasslibrary", "false");
        check("laddchildclasslibrary", "false");
        check("lremovechildclasslibrary", "false");
        check("cchildclasslibraryafter", button_library_path.string());
        check("cchildclasslibrarypropafter", button_library_path.string());
        check("lchildhasclasslibrary", "true");
        check("lchildclasslibraryreadonly", "true");
        check("ldictset", "true");
        check("ndictcompare", "66");

        expect(state.ole_objects.size() == 3U,
               "native child external-base classlibrary guards should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "DemoForm",
                   "native child external-base classlibrary guards should preserve form identity");
            const auto &child_object = state.ole_objects[1];
            expect(child_object.prog_id == "SaveButton",
                   "native child external-base classlibrary guards should preserve child class identity");
            expect(child_object.class_library == button_library_path.string(),
                   "native child external-base classlibrary guards should preserve the child external class-library path");
            expect(!child_object.properties.contains("classlibrary"),
                   "native child external-base classlibrary guards should not materialize a child ClassLibrary shadow");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native child external-base classlibrary guards land");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_external_base_child_external_base_classlibrary_survives_identity_mutation_guards()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_prg_child_external_base_classlibrary_guards";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path button_library_path = temp_root / "buttons.prg";
        write_text(
            button_library_path,
            "DEFINE CLASS ParentButton AS Custom\n"
            "ENDDEFINE\n");

        const fs::path form_library_path = temp_root / "widgetlib.prg";
        write_text(
            form_library_path,
            "DEFINE CLASS ParentForm AS Custom\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS ParentButton OF buttons.prg\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_child_external_base_classlibrary_guards.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "lSetChildClassLibrary = SETPEM(oCreate.cmdSave, 'ClassLibrary', 'other.prg')\n"
            "lAddChildClassLibrary = ADDPROPERTY(oCreate.cmdSave, 'ClassLibrary', 'other.prg')\n"
            "lRemoveChildClassLibrary = REMOVEPROPERTY(oCreate.cmdSave, 'ClassLibrary')\n"
            "oCreate.cmdSave.ClassLibrary = 'shadow.prg'\n"
            "cChildClassLibraryAfter = GETPEM(oCreate.cmdSave, 'ClassLibrary')\n"
            "cChildClassLibraryPropAfter = oCreate.cmdSave.ClassLibrary\n"
            "lChildHasClassLibrary = PEMSTATUS(oCreate.cmdSave, 'ClassLibrary', 1)\n"
            "lChildClassLibraryReadOnly = PEMSTATUS(oCreate.cmdSave, 'ClassLibrary', 5)\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 67)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external child external-base classlibrary guard script should complete: ") + state.message +
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

        check("lsetchildclasslibrary", "false");
        check("laddchildclasslibrary", "false");
        check("lremovechildclasslibrary", "false");
        check("cchildclasslibraryafter", button_library_path.string());
        check("cchildclasslibrarypropafter", button_library_path.string());
        check("lchildhasclasslibrary", "true");
        check("lchildclasslibraryreadonly", "true");
        check("ldictset", "true");
        check("ndictcompare", "67");

        expect(state.ole_objects.size() == 3U,
               "external child external-base classlibrary guards should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "ChildForm",
                   "external child external-base classlibrary guards should preserve form identity");
            const auto &child_object = state.ole_objects[1];
            expect(child_object.prog_id == "SaveButton",
                   "external child external-base classlibrary guards should preserve child class identity");
            expect(child_object.class_library == button_library_path.string(),
                   "external child external-base classlibrary guards should preserve the child external class-library path");
            expect(child_object.source == form_library_path.string(),
                   "external child external-base classlibrary guards should preserve the defining child-class source path");
            expect(!child_object.properties.contains("classlibrary"),
                   "external child external-base classlibrary guards should not materialize a child ClassLibrary shadow");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external child external-base classlibrary guards land");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_same_prg_child_deeper_external_base_provenance_stays_coherent()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_child_deeper_external_base_provenance";
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
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "native_child_deeper_external_base_provenance.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('DemoForm')\n"
            "lSetChildClassLibrary = SETPEM(oCreate.cmdSave, 'ClassLibrary', 'shadow.prg')\n"
            "oCreate.cmdSave.ClassLibrary = 'shadow2.prg'\n"
            "cChildClassLibrary = GETPEM(oCreate.cmdSave, 'ClassLibrary')\n"
            "cChildBaseClass = GETPEM(oCreate.cmdSave, 'BaseClass')\n"
            "cChildClassLibraryProp = oCreate.cmdSave.ClassLibrary\n"
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
            "DEFINE CLASS SaveButton AS ParentButton OF buttons.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native child deeper external-base provenance script should complete: ") + state.message +
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

        check("lsetchildclasslibrary", "false");
        check("cchildclasslibrary", button_library_path.string());
        check("cchildbaseclass", "ParentButton");
        check("cchildclasslibraryprop", button_library_path.string());
        check("nclasscount", "5");
        check("cclass1", "SAVEBUTTON");
        check("cclass2", "PARENTBUTTON");
        check("cclass3", "ROOTBUTTON");
        check("cclass4", "CUSTOM");
        check("cclass5", "OBJECT");

        expect(state.ole_objects.size() == 2U,
               "native child deeper external-base provenance should register parent and child objects");
        if (state.ole_objects.size() == 2U)
        {
            expect(state.ole_objects[0].prog_id == "DemoForm",
                   "native child deeper external-base provenance should preserve form identity");
            const auto &child_object = state.ole_objects[1];
            expect(child_object.prog_id == "SaveButton",
                   "native child deeper external-base provenance should preserve child class identity");
            expect(child_object.class_library == button_library_path.string(),
                   "native child deeper external-base provenance should keep the immediate external class-library path");
            expect(child_object.class_hierarchy.size() == 5U,
                   "native child deeper external-base provenance should preserve the deeper child class hierarchy");
            expect(!child_object.properties.contains("classlibrary"),
                   "native child deeper external-base provenance should not materialize a child ClassLibrary shadow");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_external_base_child_deeper_external_base_provenance_stays_coherent()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_prg_child_deeper_external_base_provenance";
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
            "ENDDEFINE\n");

        const fs::path form_library_path = temp_root / "widgetlib.prg";
        write_text(
            form_library_path,
            "DEFINE CLASS ParentForm AS Custom\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS ParentButton OF buttons.prg\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_child_deeper_external_base_provenance.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "lSetChildClassLibrary = SETPEM(oCreate.cmdSave, 'ClassLibrary', 'shadow.prg')\n"
            "oCreate.cmdSave.ClassLibrary = 'shadow2.prg'\n"
            "cChildClassLibrary = GETPEM(oCreate.cmdSave, 'ClassLibrary')\n"
            "cChildBaseClass = GETPEM(oCreate.cmdSave, 'BaseClass')\n"
            "cChildClassLibraryProp = oCreate.cmdSave.ClassLibrary\n"
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
               std::string("external child deeper external-base provenance script should complete: ") + state.message +
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

        check("lsetchildclasslibrary", "false");
        check("cchildclasslibrary", button_library_path.string());
        check("cchildbaseclass", "ParentButton");
        check("cchildclasslibraryprop", button_library_path.string());
        check("nclasscount", "5");
        check("cclass1", "SAVEBUTTON");
        check("cclass2", "PARENTBUTTON");
        check("cclass3", "ROOTBUTTON");
        check("cclass4", "CUSTOM");
        check("cclass5", "OBJECT");

        expect(state.ole_objects.size() == 2U,
               "external child deeper external-base provenance should register form and child objects");
        if (state.ole_objects.size() == 2U)
        {
            expect(state.ole_objects[0].prog_id == "ChildForm",
                   "external child deeper external-base provenance should preserve form identity");
            const auto &child_object = state.ole_objects[1];
            expect(child_object.prog_id == "SaveButton",
                   "external child deeper external-base provenance should preserve child class identity");
            expect(child_object.class_library == button_library_path.string(),
                   "external child deeper external-base provenance should keep the immediate external class-library path");
            expect(child_object.source == form_library_path.string(),
                   "external child deeper external-base provenance should preserve the defining child-class source path");
            expect(child_object.class_hierarchy.size() == 5U,
                   "external child deeper external-base provenance should preserve the deeper child class hierarchy");
            expect(!child_object.properties.contains("classlibrary"),
                   "external child deeper external-base provenance should not materialize a child ClassLibrary shadow");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_same_prg_child_identity_metadata_appears_in_amembers()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_child_identity_amembers";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_child_identity_amembers.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('DemoForm')\n"
            "nMembersProps = AMEMBERS(aMembersProps, oCreate.cmdSave, 1)\n"
            "nMembersUnion = AMEMBERS(aMembersUnion, oCreate.cmdSave, 3)\n"
            "cProp1 = aMembersProps[1]\n"
            "cProp2 = aMembersProps[2]\n"
            "cProp3 = aMembersProps[3]\n"
            "cProp4 = aMembersProps[4]\n"
            "cProp5 = aMembersProps[5]\n"
            "cUnion1 = aMembersUnion[1]\n"
            "cUnion5 = aMembersUnion[5]\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 49)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
            "    Caption = 'Save'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native child identity AMEMBERS script should complete: ") + state.message +
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
        check("cprop4", "PARENT");
        check("cprop5", "PARENTCLASS");
        check("cunion1", "BASECLASS");
        check("cunion5", "PARENTCLASS");
        check("ldictset", "true");
        check("ndictcompare", "49");

        expect(state.ole_objects.size() == 3U,
               "native child identity AMEMBERS should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &child_object = state.ole_objects[1];
            expect(child_object.prog_id == "SaveButton",
                   "native child identity AMEMBERS should preserve child class identity");
            expect(child_object.base_class_name == "Custom",
                   "native child identity AMEMBERS should preserve child base-class identity");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native child identity AMEMBERS lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_external_base_child_identity_metadata_appears_in_amembers()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_prg_child_identity_amembers";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path library_path = temp_root / "widgetlib.prg";
        write_text(
            library_path,
            "DEFINE CLASS ParentForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
            "    Caption = 'Save'\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_child_identity_amembers.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "nMembersProps = AMEMBERS(aMembersProps, oCreate.cmdSave, 1)\n"
            "nMembersUnion = AMEMBERS(aMembersUnion, oCreate.cmdSave, 3)\n"
            "cProp1 = aMembersProps[1]\n"
            "cProp2 = aMembersProps[2]\n"
            "cProp3 = aMembersProps[3]\n"
            "cProp4 = aMembersProps[4]\n"
            "cProp5 = aMembersProps[5]\n"
            "cUnion1 = aMembersUnion[1]\n"
            "cUnion5 = aMembersUnion[5]\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 50)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external child identity AMEMBERS script should complete: ") + state.message +
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
        check("cprop4", "PARENT");
        check("cprop5", "PARENTCLASS");
        check("cunion1", "BASECLASS");
        check("cunion5", "PARENTCLASS");
        check("ldictset", "true");
        check("ndictcompare", "50");

        expect(state.ole_objects.size() == 3U,
               "external child identity AMEMBERS should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &child_object = state.ole_objects[1];
            expect(child_object.prog_id == "SaveButton",
                   "external child identity AMEMBERS should preserve child class identity");
            expect(child_object.base_class_name == "Custom",
                   "external child identity AMEMBERS should preserve child base-class identity");
            expect(child_object.class_library.empty(),
                   "external child identity AMEMBERS should keep child class-library provenance empty when the child class itself has no external base");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external child identity AMEMBERS lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_same_prg_child_parent_appears_in_amembers()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_child_parent_amembers";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_child_parent_amembers.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('DemoForm')\n"
            "nMembersProps = AMEMBERS(aMembersProps, oCreate.cmdSave, 1)\n"
            "nMembersUnion = AMEMBERS(aMembersUnion, oCreate.cmdSave, 3)\n"
            "lChildHasParent = PEMSTATUS(oCreate.cmdSave, 'Parent', 1)\n"
            "cProp1 = aMembersProps[1]\n"
            "cProp2 = aMembersProps[2]\n"
            "cProp3 = aMembersProps[3]\n"
            "cProp4 = aMembersProps[4]\n"
            "cProp5 = aMembersProps[5]\n"
            "cUnion1 = aMembersUnion[1]\n"
            "cUnion5 = aMembersUnion[5]\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 37)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
            "    Caption = 'Save'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native child Parent AMEMBERS script should complete: ") + state.message +
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
        check("lchildhasparent", "true");
        check("cprop1", "BASECLASS");
        check("cprop2", "CAPTION");
        check("cprop3", "CLASS");
        check("cprop4", "PARENT");
        check("cprop5", "PARENTCLASS");
        check("cunion1", "BASECLASS");
        check("cunion5", "PARENTCLASS");
        check("ldictset", "true");
        check("ndictcompare", "37");

        expect(state.ole_objects.size() == 3U,
               "native child Parent AMEMBERS should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "DemoForm",
                   "native child Parent AMEMBERS should preserve form identity");
            expect(state.ole_objects[1].prog_id == "SaveButton",
                   "native child Parent AMEMBERS should preserve child identity");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native child Parent AMEMBERS lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_external_base_child_parent_appears_in_amembers()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_prg_child_parent_amembers";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path library_path = temp_root / "widgetlib.prg";
        write_text(
            library_path,
            "DEFINE CLASS ParentForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
            "    Caption = 'Save'\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_child_parent_amembers.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "nMembersProps = AMEMBERS(aMembersProps, oCreate.cmdSave, 1)\n"
            "nMembersUnion = AMEMBERS(aMembersUnion, oCreate.cmdSave, 3)\n"
            "lChildHasParent = PEMSTATUS(oCreate.cmdSave, 'Parent', 1)\n"
            "cProp1 = aMembersProps[1]\n"
            "cProp2 = aMembersProps[2]\n"
            "cProp3 = aMembersProps[3]\n"
            "cProp4 = aMembersProps[4]\n"
            "cProp5 = aMembersProps[5]\n"
            "cProp6 = aMembersProps[6]\n"
            "cUnion1 = aMembersUnion[1]\n"
            "cUnion5 = aMembersUnion[5]\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 38)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external child Parent AMEMBERS script should complete: ") + state.message +
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
        check("lchildhasparent", "true");
        check("cprop1", "BASECLASS");
        check("cprop2", "CAPTION");
        check("cprop3", "CLASS");
        check("cprop4", "PARENT");
        check("cprop5", "PARENTCLASS");
        check("cunion1", "BASECLASS");
        check("cunion5", "PARENTCLASS");
        check("ldictset", "true");
        check("ndictcompare", "38");

        expect(state.ole_objects.size() == 3U,
               "external child Parent AMEMBERS should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "ChildForm",
                   "external child Parent AMEMBERS should preserve form identity");
            expect(state.ole_objects[1].prog_id == "SaveButton",
                   "external child Parent AMEMBERS should preserve child identity");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external child Parent AMEMBERS lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_same_prg_child_parent_stays_read_only_to_setpem()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_child_parent_setpem";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_child_parent_setpem.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('DemoForm')\n"
            "lParentReadOnly = PEMSTATUS(oCreate.cmdSave, 'Parent', 5)\n"
            "lSetParent = SETPEM(oCreate.cmdSave, 'Parent', 'OtherParent')\n"
            "oParentRef = GETPEM(oCreate.cmdSave, 'Parent')\n"
            "cParentCaption = oParentRef.Caption\n"
            "lChildHasParent = PEMSTATUS(oCreate.cmdSave, 'Parent', 1)\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 39)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native child Parent SETPEM script should complete: ") + state.message +
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

        check("lparentreadonly", "true");
        check("lsetparent", "false");
        check("cparentcaption", "MainForm");
        check("lchildhasparent", "true");
        check("ldictset", "true");
        check("ndictcompare", "39");

        expect(state.ole_objects.size() == 3U,
               "native child Parent SETPEM should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "DemoForm",
                   "native child Parent SETPEM should preserve form identity");
            const auto &child_object = state.ole_objects[1];
            expect(child_object.prog_id == "SaveButton",
                   "native child Parent SETPEM should preserve child identity");
            const auto child_parent = child_object.properties.find("parent");
            if (child_parent != child_object.properties.end())
            {
                expect(child_parent->second.kind == copperfin::runtime::PrgValueKind::string,
                       "native child Parent SETPEM should keep the built-in PARENT object reference materialized");
                expect(child_parent->second.string_value == "object:DemoForm#" + std::to_string(state.ole_objects[0].handle),
                       "native child Parent SETPEM should not replace the built-in PARENT object reference");
            }
            else
            {
                expect(false, "native child Parent SETPEM should preserve the child PARENT reference");
            }
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native child Parent SETPEM lands");
        }

        fs::remove_all(temp_root, ignored);
    }

}
