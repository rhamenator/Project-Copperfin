#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_inherited_external_base_addobject_deeper_external_child_identity_metadata_stays_read_only_to_setpem()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_base_inherited_addobject_deeper_external_child_identity_setpem";
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
            "    Caption = 'Save'\n"
            "ENDDEFINE\n");

        const fs::path widget_library_path = temp_root / "widgetlib.prg";
        write_text(
            widget_library_path,
            "DEFINE CLASS ParentForm AS Custom\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton', 'buttons.prg')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_base_inherited_addobject_deeper_external_child_identity_setpem.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oChild = oCreate.cmdSave\n"
            "lChildClassReadOnly = PEMSTATUS(oChild, 'Class', 5)\n"
            "lChildBaseClassReadOnly = PEMSTATUS(oChild, 'BaseClass', 5)\n"
            "lChildParentClassReadOnly = PEMSTATUS(oChild, 'ParentClass', 5)\n"
            "lSetChildClass = SETPEM(oChild, 'Class', 'OtherClass')\n"
            "lSetChildBaseClass = SETPEM(oChild, 'BaseClass', 'OtherBase')\n"
            "lSetChildParentClass = SETPEM(oChild, 'ParentClass', 'OtherParent')\n"
            "lSetChildClassLibrary = SETPEM(oChild, 'ClassLibrary', 'other.prg')\n"
            "cChildClassAfter = GETPEM(oChild, 'Class')\n"
            "cChildBaseClassAfter = GETPEM(oChild, 'BaseClass')\n"
            "cChildParentClassAfter = GETPEM(oChild, 'ParentClass')\n"
            "xChildClassLibraryAfter = GETPEM(oChild, 'ClassLibrary')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 73)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external-base inherited ADDOBJECT deeper external child identity SETPEM script should complete: ") + state.message +
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

        check("lchildclassreadonly", "true");
        check("lchildbaseclassreadonly", "true");
        check("lchildparentclassreadonly", "true");
        check("lsetchildclass", "false");
        check("lsetchildbaseclass", "false");
        check("lsetchildparentclass", "false");
        check("lsetchildclasslibrary", "false");
        check("cchildclassafter", "SaveButton");
        check("cchildbaseclassafter", "ParentButton");
        check("cchildparentclassafter", "ParentButton");
        check("ldictset", "true");
        check("ndictcompare", "73");

        const auto child_class_library_after = state.globals.find("xchildclasslibraryafter");
        expect(child_class_library_after != state.globals.end() &&
                   child_class_library_after->second.kind == copperfin::runtime::PrgValueKind::empty,
               "external-base inherited ADDOBJECT deeper external child identity SETPEM should keep the derived child ClassLibrary empty after failed mutation");

        expect(state.ole_objects.size() == 3U,
               "external-base inherited ADDOBJECT deeper external child identity SETPEM should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &parent_object = state.ole_objects[0];
            const auto &child_object = state.ole_objects[1];
            expect(parent_object.prog_id == "ChildForm",
                   "external-base inherited ADDOBJECT deeper external child identity SETPEM should preserve parent identity");
            expect(child_object.prog_id == "SaveButton",
                   "external-base inherited ADDOBJECT deeper external child identity SETPEM should preserve child identity");
            expect(child_object.base_class_name == "ParentButton",
                   "external-base inherited ADDOBJECT deeper external child identity SETPEM should preserve the immediate child base-class identity");
            expect(child_object.source == button_library_path.string(),
                   "external-base inherited ADDOBJECT deeper external child identity SETPEM should preserve the child definition source path");
            expect(child_object.class_library.empty(),
                   "external-base inherited ADDOBJECT deeper external child identity SETPEM should keep the derived child ClassLibrary empty");
            expect(child_object.class_hierarchy.size() == 5U,
                   "external-base inherited ADDOBJECT deeper external child identity SETPEM should preserve the deeper runtime child class hierarchy");
            expect(!child_object.properties.contains("class"),
                   "external-base inherited ADDOBJECT deeper external child identity SETPEM should not materialize a writable child Class shadow");
            expect(!child_object.properties.contains("baseclass"),
                   "external-base inherited ADDOBJECT deeper external child identity SETPEM should not materialize a writable child BaseClass shadow");
            expect(!child_object.properties.contains("parentclass"),
                   "external-base inherited ADDOBJECT deeper external child identity SETPEM should not materialize a writable child ParentClass shadow");
            expect(!child_object.properties.contains("classlibrary"),
                   "external-base inherited ADDOBJECT deeper external child identity SETPEM should not materialize a writable child ClassLibrary shadow");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external-base inherited ADDOBJECT deeper external child identity SETPEM lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_native_addobject_deeper_external_child_identity_metadata_cannot_be_shadowed_through_addproperty()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_addobject_deeper_external_child_identity_addproperty";
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
            "    Caption = 'Save'\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "native_addobject_deeper_external_child_identity_addproperty.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('DemoForm')\n"
            "oChild = oForm.cmdSave\n"
            "lAddChildClass = ADDPROPERTY(oChild, 'Class', 'OtherClass')\n"
            "lAddChildBaseClass = ADDPROPERTY(oChild, 'BaseClass', 'OtherBase')\n"
            "lAddChildParentClass = ADDPROPERTY(oChild, 'ParentClass', 'OtherParent')\n"
            "lAddChildClassLibrary = ADDPROPERTY(oChild, 'ClassLibrary', 'other.prg')\n"
            "cChildClassAfter = GETPEM(oChild, 'Class')\n"
            "cChildBaseClassAfter = GETPEM(oChild, 'BaseClass')\n"
            "cChildParentClassAfter = GETPEM(oChild, 'ParentClass')\n"
            "xChildClassLibraryAfter = GETPEM(oChild, 'ClassLibrary')\n"
            "lChildClassReadOnly = PEMSTATUS(oChild, 'Class', 5)\n"
            "lChildBaseClassReadOnly = PEMSTATUS(oChild, 'BaseClass', 5)\n"
            "lChildParentClassReadOnly = PEMSTATUS(oChild, 'ParentClass', 5)\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 74)\n"
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
               std::string("native ADDOBJECT deeper external child identity ADDPROPERTY script should complete: ") + state.message +
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

        check("laddchildclass", "false");
        check("laddchildbaseclass", "false");
        check("laddchildparentclass", "false");
        check("laddchildclasslibrary", "false");
        check("cchildclassafter", "SaveButton");
        check("cchildbaseclassafter", "ParentButton");
        check("cchildparentclassafter", "ParentButton");
        check("lchildclassreadonly", "true");
        check("lchildbaseclassreadonly", "true");
        check("lchildparentclassreadonly", "true");
        check("ldictset", "true");
        check("ndictcompare", "74");

        const auto child_class_library_after = state.globals.find("xchildclasslibraryafter");
        expect(child_class_library_after != state.globals.end() &&
                   child_class_library_after->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native ADDOBJECT deeper external child identity ADDPROPERTY should keep the derived child ClassLibrary empty after failed shadow creation");

        expect(state.ole_objects.size() == 3U,
               "native ADDOBJECT deeper external child identity ADDPROPERTY should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &parent_object = state.ole_objects[0];
            const auto &child_object = state.ole_objects[1];
            expect(parent_object.prog_id == "DemoForm",
                   "native ADDOBJECT deeper external child identity ADDPROPERTY should preserve parent identity");
            expect(child_object.prog_id == "SaveButton",
                   "native ADDOBJECT deeper external child identity ADDPROPERTY should preserve child identity");
            expect(child_object.base_class_name == "ParentButton",
                   "native ADDOBJECT deeper external child identity ADDPROPERTY should preserve the immediate child base-class identity");
            expect(child_object.source == button_library_path.string(),
                   "native ADDOBJECT deeper external child identity ADDPROPERTY should preserve the child definition source path");
            expect(child_object.class_library.empty(),
                   "native ADDOBJECT deeper external child identity ADDPROPERTY should keep the derived child ClassLibrary empty");
            expect(child_object.class_hierarchy.size() == 5U,
                   "native ADDOBJECT deeper external child identity ADDPROPERTY should preserve the deeper runtime child class hierarchy");
            expect(!child_object.properties.contains("class"),
                   "native ADDOBJECT deeper external child identity ADDPROPERTY should not materialize a child Class shadow");
            expect(!child_object.properties.contains("baseclass"),
                   "native ADDOBJECT deeper external child identity ADDPROPERTY should not materialize a child BaseClass shadow");
            expect(!child_object.properties.contains("parentclass"),
                   "native ADDOBJECT deeper external child identity ADDPROPERTY should not materialize a child ParentClass shadow");
            expect(!child_object.properties.contains("classlibrary"),
                   "native ADDOBJECT deeper external child identity ADDPROPERTY should not materialize a child ClassLibrary shadow");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native ADDOBJECT deeper external child identity ADDPROPERTY lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_inherited_external_base_addobject_deeper_external_child_identity_metadata_cannot_be_shadowed_through_addproperty()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_base_inherited_addobject_deeper_external_child_identity_addproperty";
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
            "    Caption = 'Save'\n"
            "ENDDEFINE\n");

        const fs::path widget_library_path = temp_root / "widgetlib.prg";
        write_text(
            widget_library_path,
            "DEFINE CLASS ParentForm AS Custom\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton', 'buttons.prg')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_base_inherited_addobject_deeper_external_child_identity_addproperty.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oChild = oCreate.cmdSave\n"
            "lAddChildClass = ADDPROPERTY(oChild, 'Class', 'OtherClass')\n"
            "lAddChildBaseClass = ADDPROPERTY(oChild, 'BaseClass', 'OtherBase')\n"
            "lAddChildParentClass = ADDPROPERTY(oChild, 'ParentClass', 'OtherParent')\n"
            "lAddChildClassLibrary = ADDPROPERTY(oChild, 'ClassLibrary', 'other.prg')\n"
            "cChildClassAfter = GETPEM(oChild, 'Class')\n"
            "cChildBaseClassAfter = GETPEM(oChild, 'BaseClass')\n"
            "cChildParentClassAfter = GETPEM(oChild, 'ParentClass')\n"
            "xChildClassLibraryAfter = GETPEM(oChild, 'ClassLibrary')\n"
            "lChildClassReadOnly = PEMSTATUS(oChild, 'Class', 5)\n"
            "lChildBaseClassReadOnly = PEMSTATUS(oChild, 'BaseClass', 5)\n"
            "lChildParentClassReadOnly = PEMSTATUS(oChild, 'ParentClass', 5)\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 75)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external-base inherited ADDOBJECT deeper external child identity ADDPROPERTY script should complete: ") + state.message +
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

        check("laddchildclass", "false");
        check("laddchildbaseclass", "false");
        check("laddchildparentclass", "false");
        check("laddchildclasslibrary", "false");
        check("cchildclassafter", "SaveButton");
        check("cchildbaseclassafter", "ParentButton");
        check("cchildparentclassafter", "ParentButton");
        check("lchildclassreadonly", "true");
        check("lchildbaseclassreadonly", "true");
        check("lchildparentclassreadonly", "true");
        check("ldictset", "true");
        check("ndictcompare", "75");

        const auto child_class_library_after = state.globals.find("xchildclasslibraryafter");
        expect(child_class_library_after != state.globals.end() &&
                   child_class_library_after->second.kind == copperfin::runtime::PrgValueKind::empty,
               "external-base inherited ADDOBJECT deeper external child identity ADDPROPERTY should keep the derived child ClassLibrary empty after failed shadow creation");

        expect(state.ole_objects.size() == 3U,
               "external-base inherited ADDOBJECT deeper external child identity ADDPROPERTY should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &parent_object = state.ole_objects[0];
            const auto &child_object = state.ole_objects[1];
            expect(parent_object.prog_id == "ChildForm",
                   "external-base inherited ADDOBJECT deeper external child identity ADDPROPERTY should preserve parent identity");
            expect(child_object.prog_id == "SaveButton",
                   "external-base inherited ADDOBJECT deeper external child identity ADDPROPERTY should preserve child identity");
            expect(child_object.base_class_name == "ParentButton",
                   "external-base inherited ADDOBJECT deeper external child identity ADDPROPERTY should preserve the immediate child base-class identity");
            expect(child_object.source == button_library_path.string(),
                   "external-base inherited ADDOBJECT deeper external child identity ADDPROPERTY should preserve the child definition source path");
            expect(child_object.class_library.empty(),
                   "external-base inherited ADDOBJECT deeper external child identity ADDPROPERTY should keep the derived child ClassLibrary empty");
            expect(child_object.class_hierarchy.size() == 5U,
                   "external-base inherited ADDOBJECT deeper external child identity ADDPROPERTY should preserve the deeper runtime child class hierarchy");
            expect(!child_object.properties.contains("class"),
                   "external-base inherited ADDOBJECT deeper external child identity ADDPROPERTY should not materialize a child Class shadow");
            expect(!child_object.properties.contains("baseclass"),
                   "external-base inherited ADDOBJECT deeper external child identity ADDPROPERTY should not materialize a child BaseClass shadow");
            expect(!child_object.properties.contains("parentclass"),
                   "external-base inherited ADDOBJECT deeper external child identity ADDPROPERTY should not materialize a child ParentClass shadow");
            expect(!child_object.properties.contains("classlibrary"),
                   "external-base inherited ADDOBJECT deeper external child identity ADDPROPERTY should not materialize a child ClassLibrary shadow");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external-base inherited ADDOBJECT deeper external child identity ADDPROPERTY lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_native_addobject_deeper_external_child_identity_metadata_cannot_be_erased_through_removeproperty()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_addobject_deeper_external_child_identity_removeproperty";
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
            "    Caption = 'Save'\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "native_addobject_deeper_external_child_identity_removeproperty.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('DemoForm')\n"
            "oChild = oForm.cmdSave\n"
            "lRemoveChildClass = REMOVEPROPERTY(oChild, 'Class')\n"
            "lRemoveChildBaseClass = REMOVEPROPERTY(oChild, 'BaseClass')\n"
            "lRemoveChildParentClass = REMOVEPROPERTY(oChild, 'ParentClass')\n"
            "lRemoveChildClassLibrary = REMOVEPROPERTY(oChild, 'ClassLibrary')\n"
            "cChildClassAfter = GETPEM(oChild, 'Class')\n"
            "cChildBaseClassAfter = GETPEM(oChild, 'BaseClass')\n"
            "cChildParentClassAfter = GETPEM(oChild, 'ParentClass')\n"
            "xChildClassLibraryAfter = GETPEM(oChild, 'ClassLibrary')\n"
            "lChildClassReadOnly = PEMSTATUS(oChild, 'Class', 5)\n"
            "lChildBaseClassReadOnly = PEMSTATUS(oChild, 'BaseClass', 5)\n"
            "lChildParentClassReadOnly = PEMSTATUS(oChild, 'ParentClass', 5)\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 76)\n"
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
               std::string("native ADDOBJECT deeper external child identity REMOVEPROPERTY script should complete: ") + state.message +
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

        check("lremovechildclass", "false");
        check("lremovechildbaseclass", "false");
        check("lremovechildparentclass", "false");
        check("lremovechildclasslibrary", "false");
        check("cchildclassafter", "SaveButton");
        check("cchildbaseclassafter", "ParentButton");
        check("cchildparentclassafter", "ParentButton");
        check("lchildclassreadonly", "true");
        check("lchildbaseclassreadonly", "true");
        check("lchildparentclassreadonly", "true");
        check("ldictset", "true");
        check("ndictcompare", "76");

        const auto child_class_library_after = state.globals.find("xchildclasslibraryafter");
        expect(child_class_library_after != state.globals.end() &&
                   child_class_library_after->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native ADDOBJECT deeper external child identity REMOVEPROPERTY should keep the derived child ClassLibrary empty after failed removal");

        expect(state.ole_objects.size() == 3U,
               "native ADDOBJECT deeper external child identity REMOVEPROPERTY should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &parent_object = state.ole_objects[0];
            const auto &child_object = state.ole_objects[1];
            expect(parent_object.prog_id == "DemoForm",
                   "native ADDOBJECT deeper external child identity REMOVEPROPERTY should preserve parent identity");
            expect(child_object.prog_id == "SaveButton",
                   "native ADDOBJECT deeper external child identity REMOVEPROPERTY should preserve child identity");
            expect(child_object.base_class_name == "ParentButton",
                   "native ADDOBJECT deeper external child identity REMOVEPROPERTY should preserve the immediate child base-class identity");
            expect(child_object.source == button_library_path.string(),
                   "native ADDOBJECT deeper external child identity REMOVEPROPERTY should preserve the child definition source path");
            expect(child_object.class_library.empty(),
                   "native ADDOBJECT deeper external child identity REMOVEPROPERTY should keep the derived child ClassLibrary empty");
            expect(child_object.class_hierarchy.size() == 5U,
                   "native ADDOBJECT deeper external child identity REMOVEPROPERTY should preserve the deeper runtime child class hierarchy");
            expect(!child_object.properties.contains("class"),
                   "native ADDOBJECT deeper external child identity REMOVEPROPERTY should not materialize a child Class shadow");
            expect(!child_object.properties.contains("baseclass"),
                   "native ADDOBJECT deeper external child identity REMOVEPROPERTY should not materialize a child BaseClass shadow");
            expect(!child_object.properties.contains("parentclass"),
                   "native ADDOBJECT deeper external child identity REMOVEPROPERTY should not materialize a child ParentClass shadow");
            expect(!child_object.properties.contains("classlibrary"),
                   "native ADDOBJECT deeper external child identity REMOVEPROPERTY should not materialize a child ClassLibrary shadow");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native ADDOBJECT deeper external child identity REMOVEPROPERTY lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_inherited_external_base_addobject_deeper_external_child_identity_metadata_cannot_be_erased_through_removeproperty()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_base_inherited_addobject_deeper_external_child_identity_removeproperty";
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
            "    Caption = 'Save'\n"
            "ENDDEFINE\n");

        const fs::path widget_library_path = temp_root / "widgetlib.prg";
        write_text(
            widget_library_path,
            "DEFINE CLASS ParentForm AS Custom\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton', 'buttons.prg')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_base_inherited_addobject_deeper_external_child_identity_removeproperty.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oChild = oCreate.cmdSave\n"
            "lRemoveChildClass = REMOVEPROPERTY(oChild, 'Class')\n"
            "lRemoveChildBaseClass = REMOVEPROPERTY(oChild, 'BaseClass')\n"
            "lRemoveChildParentClass = REMOVEPROPERTY(oChild, 'ParentClass')\n"
            "lRemoveChildClassLibrary = REMOVEPROPERTY(oChild, 'ClassLibrary')\n"
            "cChildClassAfter = GETPEM(oChild, 'Class')\n"
            "cChildBaseClassAfter = GETPEM(oChild, 'BaseClass')\n"
            "cChildParentClassAfter = GETPEM(oChild, 'ParentClass')\n"
            "xChildClassLibraryAfter = GETPEM(oChild, 'ClassLibrary')\n"
            "lChildClassReadOnly = PEMSTATUS(oChild, 'Class', 5)\n"
            "lChildBaseClassReadOnly = PEMSTATUS(oChild, 'BaseClass', 5)\n"
            "lChildParentClassReadOnly = PEMSTATUS(oChild, 'ParentClass', 5)\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 77)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external-base inherited ADDOBJECT deeper external child identity REMOVEPROPERTY script should complete: ") + state.message +
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

        check("lremovechildclass", "false");
        check("lremovechildbaseclass", "false");
        check("lremovechildparentclass", "false");
        check("lremovechildclasslibrary", "false");
        check("cchildclassafter", "SaveButton");
        check("cchildbaseclassafter", "ParentButton");
        check("cchildparentclassafter", "ParentButton");
        check("lchildclassreadonly", "true");
        check("lchildbaseclassreadonly", "true");
        check("lchildparentclassreadonly", "true");
        check("ldictset", "true");
        check("ndictcompare", "77");

        const auto child_class_library_after = state.globals.find("xchildclasslibraryafter");
        expect(child_class_library_after != state.globals.end() &&
                   child_class_library_after->second.kind == copperfin::runtime::PrgValueKind::empty,
               "external-base inherited ADDOBJECT deeper external child identity REMOVEPROPERTY should keep the derived child ClassLibrary empty after failed removal");

        expect(state.ole_objects.size() == 3U,
               "external-base inherited ADDOBJECT deeper external child identity REMOVEPROPERTY should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &parent_object = state.ole_objects[0];
            const auto &child_object = state.ole_objects[1];
            expect(parent_object.prog_id == "ChildForm",
                   "external-base inherited ADDOBJECT deeper external child identity REMOVEPROPERTY should preserve parent identity");
            expect(child_object.prog_id == "SaveButton",
                   "external-base inherited ADDOBJECT deeper external child identity REMOVEPROPERTY should preserve child identity");
            expect(child_object.base_class_name == "ParentButton",
                   "external-base inherited ADDOBJECT deeper external child identity REMOVEPROPERTY should preserve the immediate child base-class identity");
            expect(child_object.source == button_library_path.string(),
                   "external-base inherited ADDOBJECT deeper external child identity REMOVEPROPERTY should preserve the child definition source path");
            expect(child_object.class_library.empty(),
                   "external-base inherited ADDOBJECT deeper external child identity REMOVEPROPERTY should keep the derived child ClassLibrary empty");
            expect(child_object.class_hierarchy.size() == 5U,
                   "external-base inherited ADDOBJECT deeper external child identity REMOVEPROPERTY should preserve the deeper runtime child class hierarchy");
            expect(!child_object.properties.contains("class"),
                   "external-base inherited ADDOBJECT deeper external child identity REMOVEPROPERTY should not materialize a child Class shadow");
            expect(!child_object.properties.contains("baseclass"),
                   "external-base inherited ADDOBJECT deeper external child identity REMOVEPROPERTY should not materialize a child BaseClass shadow");
            expect(!child_object.properties.contains("parentclass"),
                   "external-base inherited ADDOBJECT deeper external child identity REMOVEPROPERTY should not materialize a child ParentClass shadow");
            expect(!child_object.properties.contains("classlibrary"),
                   "external-base inherited ADDOBJECT deeper external child identity REMOVEPROPERTY should not materialize a child ClassLibrary shadow");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external-base inherited ADDOBJECT deeper external child identity REMOVEPROPERTY lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_native_addobject_deeper_external_child_identity_metadata_cannot_be_shadowed_through_direct_assignment()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_addobject_deeper_external_child_identity_direct_assignment";
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
            "    Caption = 'Save'\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "native_addobject_deeper_external_child_identity_direct_assignment.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('DemoForm')\n"
            "oChild = oForm.cmdSave\n"
            "oChild.Class = 'OtherClass'\n"
            "oChild.BaseClass = 'OtherBase'\n"
            "oChild.ParentClass = 'OtherParent'\n"
            "oChild.ClassLibrary = 'other.prg'\n"
            "cChildClassAfter = GETPEM(oChild, 'Class')\n"
            "cChildBaseClassAfter = GETPEM(oChild, 'BaseClass')\n"
            "cChildParentClassAfter = GETPEM(oChild, 'ParentClass')\n"
            "xChildClassLibraryAfter = GETPEM(oChild, 'ClassLibrary')\n"
            "lChildClassReadOnly = PEMSTATUS(oChild, 'Class', 5)\n"
            "lChildBaseClassReadOnly = PEMSTATUS(oChild, 'BaseClass', 5)\n"
            "lChildParentClassReadOnly = PEMSTATUS(oChild, 'ParentClass', 5)\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 78)\n"
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
               std::string("native ADDOBJECT deeper external child identity direct-assignment script should complete: ") + state.message +
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

        check("cchildclassafter", "SaveButton");
        check("cchildbaseclassafter", "ParentButton");
        check("cchildparentclassafter", "ParentButton");
        check("lchildclassreadonly", "true");
        check("lchildbaseclassreadonly", "true");
        check("lchildparentclassreadonly", "true");
        check("ldictset", "true");
        check("ndictcompare", "78");

        const auto child_class_library_after = state.globals.find("xchildclasslibraryafter");
        expect(child_class_library_after != state.globals.end() &&
                   child_class_library_after->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native ADDOBJECT deeper external child identity direct assignment should keep the derived child ClassLibrary empty");

        expect(state.ole_objects.size() == 3U,
               "native ADDOBJECT deeper external child identity direct assignment should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &parent_object = state.ole_objects[0];
            const auto &child_object = state.ole_objects[1];
            expect(parent_object.prog_id == "DemoForm",
                   "native ADDOBJECT deeper external child identity direct assignment should preserve parent identity");
            expect(child_object.prog_id == "SaveButton",
                   "native ADDOBJECT deeper external child identity direct assignment should preserve child identity");
            expect(child_object.base_class_name == "ParentButton",
                   "native ADDOBJECT deeper external child identity direct assignment should preserve the immediate child base-class identity");
            expect(child_object.source == button_library_path.string(),
                   "native ADDOBJECT deeper external child identity direct assignment should preserve the child definition source path");
            expect(child_object.class_library.empty(),
                   "native ADDOBJECT deeper external child identity direct assignment should keep the derived child ClassLibrary empty");
            expect(child_object.class_hierarchy.size() == 5U,
                   "native ADDOBJECT deeper external child identity direct assignment should preserve the deeper runtime child class hierarchy");
            expect(!child_object.properties.contains("class"),
                   "native ADDOBJECT deeper external child identity direct assignment should not materialize a child Class shadow property");
            expect(!child_object.properties.contains("baseclass"),
                   "native ADDOBJECT deeper external child identity direct assignment should not materialize a child BaseClass shadow property");
            expect(!child_object.properties.contains("parentclass"),
                   "native ADDOBJECT deeper external child identity direct assignment should not materialize a child ParentClass shadow property");
            expect(!child_object.properties.contains("classlibrary"),
                   "native ADDOBJECT deeper external child identity direct assignment should not materialize a child ClassLibrary shadow property");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native ADDOBJECT deeper external child identity direct assignment lands");
        }

        fs::remove_all(temp_root, ignored);
    }

}
