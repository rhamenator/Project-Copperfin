#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_inherited_external_base_addobject_deeper_external_child_external_base_classlibrary_stays_read_only_to_setpem()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_base_inherited_addobject_deeper_external_child_external_base_setpem";
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

        const fs::path widget_library_path = temp_root / "widgetlib.prg";
        write_text(
            widget_library_path,
            "DEFINE CLASS ParentForm AS Custom\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS ParentButton OF buttons.prg\n"
            "    Caption = 'Save'\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_base_inherited_addobject_deeper_external_child_external_base_setpem.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oLeaf = NEWOBJECT('LeafForm')\n"
            "oChild = oCreate.cmdSave\n"
            "oLeafChild = oLeaf.cmdSave\n"
            "lChildClassReadOnly = PEMSTATUS(oChild, 'Class', 5)\n"
            "lChildBaseClassReadOnly = PEMSTATUS(oChild, 'BaseClass', 5)\n"
            "lChildParentClassReadOnly = PEMSTATUS(oChild, 'ParentClass', 5)\n"
            "lChildClassLibraryReadOnly = PEMSTATUS(oChild, 'ClassLibrary', 5)\n"
            "lLeafChildClassReadOnly = PEMSTATUS(oLeafChild, 'Class', 5)\n"
            "lLeafChildBaseClassReadOnly = PEMSTATUS(oLeafChild, 'BaseClass', 5)\n"
            "lLeafChildParentClassReadOnly = PEMSTATUS(oLeafChild, 'ParentClass', 5)\n"
            "lLeafChildClassLibraryReadOnly = PEMSTATUS(oLeafChild, 'ClassLibrary', 5)\n"
            "lSetChildClass = SETPEM(oChild, 'Class', 'OtherClass')\n"
            "lSetChildBaseClass = SETPEM(oChild, 'BaseClass', 'OtherBase')\n"
            "lSetChildParentClass = SETPEM(oChild, 'ParentClass', 'OtherParent')\n"
            "lSetChildClassLibrary = SETPEM(oChild, 'ClassLibrary', 'other.prg')\n"
            "lSetLeafChildClass = SETPEM(oLeafChild, 'Class', 'OtherClass')\n"
            "lSetLeafChildBaseClass = SETPEM(oLeafChild, 'BaseClass', 'OtherBase')\n"
            "lSetLeafChildParentClass = SETPEM(oLeafChild, 'ParentClass', 'OtherParent')\n"
            "lSetLeafChildClassLibrary = SETPEM(oLeafChild, 'ClassLibrary', 'other.prg')\n"
            "cChildClassAfter = GETPEM(oChild, 'Class')\n"
            "cChildBaseClassAfter = GETPEM(oChild, 'BaseClass')\n"
            "cChildParentClassAfter = GETPEM(oChild, 'ParentClass')\n"
            "cChildClassLibraryAfter = GETPEM(oChild, 'ClassLibrary')\n"
            "cChildClassLibraryPropAfter = oChild.ClassLibrary\n"
            "cLeafChildClassAfter = GETPEM(oLeafChild, 'Class')\n"
            "cLeafChildBaseClassAfter = GETPEM(oLeafChild, 'BaseClass')\n"
            "cLeafChildParentClassAfter = GETPEM(oLeafChild, 'ParentClass')\n"
            "cLeafChildClassLibraryAfter = GETPEM(oLeafChild, 'ClassLibrary')\n"
            "cLeafChildClassLibraryPropAfter = oLeafChild.ClassLibrary\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 83)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n"
            "DEFINE CLASS LeafForm AS ChildForm\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external-base inherited ADDOBJECT deeper external child external-base SETPEM script should complete: ") + state.message +
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
        check("lchildclasslibraryreadonly", "true");
        check("lleafchildclassreadonly", "true");
        check("lleafchildbaseclassreadonly", "true");
        check("lleafchildparentclassreadonly", "true");
        check("lleafchildclasslibraryreadonly", "true");
        check("lsetchildclass", "false");
        check("lsetchildbaseclass", "false");
        check("lsetchildparentclass", "false");
        check("lsetchildclasslibrary", "false");
        check("lsetleafchildclass", "false");
        check("lsetleafchildbaseclass", "false");
        check("lsetleafchildparentclass", "false");
        check("lsetleafchildclasslibrary", "false");
        check("cchildclassafter", "SaveButton");
        check("cchildbaseclassafter", "ParentButton");
        check("cchildparentclassafter", "ParentButton");
        check("cchildclasslibraryafter", button_library_path.string());
        check("cchildclasslibrarypropafter", button_library_path.string());
        check("cleafchildclassafter", "SaveButton");
        check("cleafchildbaseclassafter", "ParentButton");
        check("cleafchildparentclassafter", "ParentButton");
        check("cleafchildclasslibraryafter", button_library_path.string());
        check("cleafchildclasslibrarypropafter", button_library_path.string());
        check("ldictset", "true");
        check("ndictcompare", "83");

        expect(state.ole_objects.size() == 5U,
               "external-base inherited ADDOBJECT deeper external child external-base SETPEM should register CREATEOBJECT parent/child, NEWOBJECT parent/child, and COM objects");
        if (state.ole_objects.size() == 5U)
        {
            const auto &create_parent = state.ole_objects[0];
            const auto &create_child = state.ole_objects[1];
            const auto &leaf_parent = state.ole_objects[2];
            const auto &leaf_child = state.ole_objects[3];
            expect(create_parent.prog_id == "ChildForm",
                   "external-base inherited ADDOBJECT deeper external child external-base SETPEM should preserve CREATEOBJECT parent identity");
            expect(create_child.prog_id == "SaveButton",
                   "external-base inherited ADDOBJECT deeper external child external-base SETPEM should preserve CREATEOBJECT child identity");
            expect(create_child.base_class_name == "ParentButton",
                   "external-base inherited ADDOBJECT deeper external child external-base SETPEM should preserve CREATEOBJECT immediate child base-class identity");
            expect(create_child.source == widget_library_path.string(),
                   "external-base inherited ADDOBJECT deeper external child external-base SETPEM should preserve the CREATEOBJECT child source path");
            expect(create_child.class_library == button_library_path.string(),
                   "external-base inherited ADDOBJECT deeper external child external-base SETPEM should preserve the CREATEOBJECT immediate external ClassLibrary path");
            expect(create_child.class_hierarchy.size() == 5U,
                   "external-base inherited ADDOBJECT deeper external child external-base SETPEM should preserve the CREATEOBJECT child class hierarchy");
            expect(!create_child.properties.contains("class"),
                   "external-base inherited ADDOBJECT deeper external child external-base SETPEM should not materialize a writable CREATEOBJECT child Class shadow");
            expect(!create_child.properties.contains("baseclass"),
                   "external-base inherited ADDOBJECT deeper external child external-base SETPEM should not materialize a writable CREATEOBJECT child BaseClass shadow");
            expect(!create_child.properties.contains("parentclass"),
                   "external-base inherited ADDOBJECT deeper external child external-base SETPEM should not materialize a writable CREATEOBJECT child ParentClass shadow");
            expect(!create_child.properties.contains("classlibrary"),
                   "external-base inherited ADDOBJECT deeper external child external-base SETPEM should not materialize a writable CREATEOBJECT child ClassLibrary shadow");
            expect(leaf_parent.prog_id == "LeafForm",
                   "external-base inherited ADDOBJECT deeper external child external-base SETPEM should preserve NEWOBJECT leaf identity");
            expect(leaf_child.prog_id == "SaveButton",
                   "external-base inherited ADDOBJECT deeper external child external-base SETPEM should preserve NEWOBJECT leaf child identity");
            expect(leaf_child.base_class_name == "ParentButton",
                   "external-base inherited ADDOBJECT deeper external child external-base SETPEM should preserve NEWOBJECT leaf child immediate base-class identity");
            expect(leaf_child.source == widget_library_path.string(),
                   "external-base inherited ADDOBJECT deeper external child external-base SETPEM should preserve the NEWOBJECT leaf child source path");
            expect(leaf_child.class_library == button_library_path.string(),
                   "external-base inherited ADDOBJECT deeper external child external-base SETPEM should preserve the NEWOBJECT leaf child immediate external ClassLibrary path");
            expect(leaf_child.class_hierarchy.size() == 5U,
                   "external-base inherited ADDOBJECT deeper external child external-base SETPEM should preserve the NEWOBJECT leaf child class hierarchy");
            expect(!leaf_child.properties.contains("class"),
                   "external-base inherited ADDOBJECT deeper external child external-base SETPEM should not materialize a writable NEWOBJECT leaf child Class shadow");
            expect(!leaf_child.properties.contains("baseclass"),
                   "external-base inherited ADDOBJECT deeper external child external-base SETPEM should not materialize a writable NEWOBJECT leaf child BaseClass shadow");
            expect(!leaf_child.properties.contains("parentclass"),
                   "external-base inherited ADDOBJECT deeper external child external-base SETPEM should not materialize a writable NEWOBJECT leaf child ParentClass shadow");
            expect(!leaf_child.properties.contains("classlibrary"),
                   "external-base inherited ADDOBJECT deeper external child external-base SETPEM should not materialize a writable NEWOBJECT leaf child ClassLibrary shadow");
            expect(state.ole_objects[4].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external-base inherited ADDOBJECT deeper external child external-base SETPEM lands");
        }

        fs::remove_all(temp_root, ignored);
    }

}
