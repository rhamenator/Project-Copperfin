#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_external_base_child_identity_metadata_stays_protected_from_removeproperty()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_prg_child_identity_removeproperty";
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
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_child_identity_removeproperty.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "lRemoveChildClass = REMOVEPROPERTY(oCreate.cmdSave, 'Class')\n"
            "lRemoveChildBaseClass = REMOVEPROPERTY(oCreate.cmdSave, 'BaseClass')\n"
            "lRemoveChildParentClass = REMOVEPROPERTY(oCreate.cmdSave, 'ParentClass')\n"
            "lRemoveChildClassLibrary = REMOVEPROPERTY(oCreate.cmdSave, 'ClassLibrary')\n"
            "cChildClassAfter = GETPEM(oCreate.cmdSave, 'Class')\n"
            "cChildBaseClassAfter = GETPEM(oCreate.cmdSave, 'BaseClass')\n"
            "cChildParentClassAfter = GETPEM(oCreate.cmdSave, 'ParentClass')\n"
            "xChildClassLibraryAfter = GETPEM(oCreate.cmdSave, 'ClassLibrary')\n"
            "lChildClassReadOnly = PEMSTATUS(oCreate.cmdSave, 'Class', 5)\n"
            "lChildBaseClassReadOnly = PEMSTATUS(oCreate.cmdSave, 'BaseClass', 5)\n"
            "lChildParentClassReadOnly = PEMSTATUS(oCreate.cmdSave, 'ParentClass', 5)\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 57)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external child identity REMOVEPROPERTY script should complete: ") + state.message +
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
        check("cchildbaseclassafter", "Custom");
        check("cchildparentclassafter", "Custom");
        check("lchildclassreadonly", "true");
        check("lchildbaseclassreadonly", "true");
        check("lchildparentclassreadonly", "true");
        check("ldictset", "true");
        check("ndictcompare", "57");

        const auto child_class_library_after = state.globals.find("xchildclasslibraryafter");
        expect(child_class_library_after != state.globals.end() &&
                   child_class_library_after->second.kind == copperfin::runtime::PrgValueKind::empty,
               "external child identity REMOVEPROPERTY should leave child ClassLibrary empty when the child class itself has no external base");

        expect(state.ole_objects.size() == 3U,
               "external child identity REMOVEPROPERTY should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "ChildForm",
                   "external child identity REMOVEPROPERTY should preserve parent form identity");
            const auto &child_object = state.ole_objects[1];
            expect(child_object.prog_id == "SaveButton",
                   "external child identity REMOVEPROPERTY should preserve child class identity");
            expect(child_object.base_class_name == "Custom",
                   "external child identity REMOVEPROPERTY should preserve child base-class identity");
            expect(child_object.class_library.empty(),
                   "external child identity REMOVEPROPERTY should keep child class-library provenance empty when the child class itself has no external base");
            expect(child_object.source == library_path.string(),
                   "external child identity REMOVEPROPERTY should preserve the defining PRG path as child provenance");
            expect(!child_object.properties.contains("class"),
                   "external child identity REMOVEPROPERTY should not materialize a child Class shadow");
            expect(!child_object.properties.contains("baseclass"),
                   "external child identity REMOVEPROPERTY should not materialize a child BaseClass shadow");
            expect(!child_object.properties.contains("parentclass"),
                   "external child identity REMOVEPROPERTY should not materialize a child ParentClass shadow");
            expect(!child_object.properties.contains("classlibrary"),
                   "external child identity REMOVEPROPERTY should not materialize a child ClassLibrary shadow");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external child identity REMOVEPROPERTY lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_same_prg_child_identity_metadata_cannot_be_shadowed_through_direct_assignment()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_child_identity_direct_assignment";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_child_identity_direct_assignment.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('DemoForm')\n"
            "oCreate.cmdSave.Class = 'OtherClass'\n"
            "oCreate.cmdSave.BaseClass = 'OtherBase'\n"
            "oCreate.cmdSave.ParentClass = 'OtherParent'\n"
            "oCreate.cmdSave.ClassLibrary = 'other.prg'\n"
            "cChildClassAfter = GETPEM(oCreate.cmdSave, 'Class')\n"
            "cChildBaseClassAfter = GETPEM(oCreate.cmdSave, 'BaseClass')\n"
            "cChildParentClassAfter = GETPEM(oCreate.cmdSave, 'ParentClass')\n"
            "xChildClassLibraryAfter = GETPEM(oCreate.cmdSave, 'ClassLibrary')\n"
            "lChildClassReadOnly = PEMSTATUS(oCreate.cmdSave, 'Class', 5)\n"
            "lChildBaseClassReadOnly = PEMSTATUS(oCreate.cmdSave, 'BaseClass', 5)\n"
            "lChildParentClassReadOnly = PEMSTATUS(oCreate.cmdSave, 'ParentClass', 5)\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 58)\n"
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
               std::string("native child identity direct-assignment script should complete: ") + state.message +
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
        check("cchildbaseclassafter", "Custom");
        check("cchildparentclassafter", "Custom");
        check("lchildclassreadonly", "true");
        check("lchildbaseclassreadonly", "true");
        check("lchildparentclassreadonly", "true");
        check("ldictset", "true");
        check("ndictcompare", "58");

        const auto child_class_library_after = state.globals.find("xchildclasslibraryafter");
        expect(child_class_library_after != state.globals.end() &&
                   child_class_library_after->second.kind == copperfin::runtime::PrgValueKind::empty,
               "same-PRG child identity direct assignment should leave child ClassLibrary empty");

        expect(state.ole_objects.size() == 3U,
               "native child identity direct assignment should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "DemoForm",
                   "native child identity direct assignment should preserve parent form identity");
            const auto &child_object = state.ole_objects[1];
            expect(child_object.prog_id == "SaveButton",
                   "native child identity direct assignment should preserve child class identity");
            expect(child_object.base_class_name == "Custom",
                   "native child identity direct assignment should preserve child base-class identity");
            expect(child_object.class_library.empty(),
                   "native child identity direct assignment should preserve empty same-PRG child class-library provenance");
            expect(!child_object.properties.contains("class"),
                   "native child identity direct assignment should not materialize a child Class shadow property");
            expect(!child_object.properties.contains("baseclass"),
                   "native child identity direct assignment should not materialize a child BaseClass shadow property");
            expect(!child_object.properties.contains("parentclass"),
                   "native child identity direct assignment should not materialize a child ParentClass shadow property");
            expect(!child_object.properties.contains("classlibrary"),
                   "native child identity direct assignment should not materialize a child ClassLibrary shadow property");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native child identity direct assignment lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_external_base_child_identity_metadata_cannot_be_shadowed_through_direct_assignment()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_prg_child_identity_direct_assignment";
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
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_child_identity_direct_assignment.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 59)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "oCreate.cmdSave.Class = 'OtherClass'\n"
            "oCreate.cmdSave.BaseClass = 'OtherBase'\n"
            "oCreate.cmdSave.ParentClass = 'OtherParent'\n"
            "oCreate.cmdSave.ClassLibrary = 'other.prg'\n"
            "cChildClassAfter = GETPEM(oCreate.cmdSave, 'Class')\n"
            "cChildBaseClassAfter = GETPEM(oCreate.cmdSave, 'BaseClass')\n"
            "cChildParentClassAfter = GETPEM(oCreate.cmdSave, 'ParentClass')\n"
            "xChildClassLibraryAfter = GETPEM(oCreate.cmdSave, 'ClassLibrary')\n"
            "lChildClassReadOnly = PEMSTATUS(oCreate.cmdSave, 'Class', 5)\n"
            "lChildBaseClassReadOnly = PEMSTATUS(oCreate.cmdSave, 'BaseClass', 5)\n"
            "lChildParentClassReadOnly = PEMSTATUS(oCreate.cmdSave, 'ParentClass', 5)\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external child identity direct-assignment script should complete: ") + state.message +
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
        check("cchildbaseclassafter", "Custom");
        check("cchildparentclassafter", "Custom");
        check("lchildclassreadonly", "true");
        check("lchildbaseclassreadonly", "true");
        check("lchildparentclassreadonly", "true");
        check("ldictset", "true");
        check("ndictcompare", "59");

        const auto child_class_library_after = state.globals.find("xchildclasslibraryafter");
        expect(child_class_library_after != state.globals.end() &&
                   child_class_library_after->second.kind == copperfin::runtime::PrgValueKind::empty,
               "external child identity direct assignment should leave child ClassLibrary empty when the child class itself has no external base");

        expect(state.ole_objects.size() == 3U,
               "external child identity direct assignment should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "ChildForm",
                   "external child identity direct assignment should preserve parent form identity");
            const auto &child_object = state.ole_objects[1];
            expect(child_object.prog_id == "SaveButton",
                   "external child identity direct assignment should preserve child class identity");
            expect(child_object.base_class_name == "Custom",
                   "external child identity direct assignment should preserve child base-class identity");
            expect(child_object.class_library.empty(),
                   "external child identity direct assignment should keep child class-library provenance empty when the child class itself has no external base");
            expect(child_object.source == library_path.string(),
                   "external child identity direct assignment should preserve the defining PRG path as child provenance");
            expect(!child_object.properties.contains("class"),
                   "external child identity direct assignment should not materialize a child Class shadow property");
            expect(!child_object.properties.contains("baseclass"),
                   "external child identity direct assignment should not materialize a child BaseClass shadow property");
            expect(!child_object.properties.contains("parentclass"),
                   "external child identity direct assignment should not materialize a child ParentClass shadow property");
            expect(!child_object.properties.contains("classlibrary"),
                   "external child identity direct assignment should not materialize a child ClassLibrary shadow property");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external child identity direct assignment lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_same_prg_child_parent_reflects_through_getpem_and_pemstatus()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_child_parent_reflection";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_child_parent_reflection.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('DemoForm')\n"
            "oParentRef = GETPEM(oCreate.cmdSave, 'Parent')\n"
            "lChildHasParent = PEMSTATUS(oCreate.cmdSave, 'Parent', 1)\n"
            "cParentCaption = oParentRef.Caption\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 35)\n"
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
               std::string("native child Parent reflection script should complete: ") + state.message +
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

        check("lchildhasparent", "true");
        check("cparentcaption", "MainForm");
        check("ldictset", "true");
        check("ndictcompare", "35");

        expect(state.ole_objects.size() == 3U,
               "native child Parent reflection should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "DemoForm",
                   "native child Parent reflection should preserve form identity");
            expect(state.ole_objects[1].prog_id == "SaveButton",
                   "native child Parent reflection should preserve child identity");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native child Parent reflection lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_external_base_child_parent_reflects_through_getpem_and_pemstatus()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_prg_child_parent_reflection";
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
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_child_parent_reflection.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oParentRef = GETPEM(oCreate.cmdSave, 'Parent')\n"
            "lChildHasParent = PEMSTATUS(oCreate.cmdSave, 'Parent', 1)\n"
            "cParentCaption = oParentRef.Caption\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 36)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external child Parent reflection script should complete: ") + state.message +
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

        check("lchildhasparent", "true");
        check("cparentcaption", "MainForm");
        check("ldictset", "true");
        check("ndictcompare", "36");

        expect(state.ole_objects.size() == 3U,
               "external child Parent reflection should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "ChildForm",
                   "external child Parent reflection should preserve form identity");
            expect(state.ole_objects[1].prog_id == "SaveButton",
                   "external child Parent reflection should preserve child identity");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external child Parent reflection lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_same_prg_child_identity_reflects_through_getpem_and_pemstatus()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_child_identity_reflection";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_child_identity_reflection.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('DemoForm')\n"
            "cChildClass = GETPEM(oCreate.cmdSave, 'Class')\n"
            "cChildBaseClass = GETPEM(oCreate.cmdSave, 'BaseClass')\n"
            "cChildParentClass = GETPEM(oCreate.cmdSave, 'ParentClass')\n"
            "xChildClassLibrary = GETPEM(oCreate.cmdSave, 'ClassLibrary')\n"
            "lChildHasClass = PEMSTATUS(oCreate.cmdSave, 'Class', 1)\n"
            "lChildHasBaseClass = PEMSTATUS(oCreate.cmdSave, 'BaseClass', 1)\n"
            "lChildHasParentClass = PEMSTATUS(oCreate.cmdSave, 'ParentClass', 1)\n"
            "lChildHasClassLibrary = PEMSTATUS(oCreate.cmdSave, 'ClassLibrary', 1)\n"
            "lChildClassReadOnly = PEMSTATUS(oCreate.cmdSave, 'Class', 5)\n"
            "lChildBaseClassReadOnly = PEMSTATUS(oCreate.cmdSave, 'BaseClass', 5)\n"
            "lChildParentClassReadOnly = PEMSTATUS(oCreate.cmdSave, 'ParentClass', 5)\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 47)\n"
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
               std::string("native child identity reflection script should complete: ") + state.message +
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

        check("cchildclass", "SaveButton");
        check("cchildbaseclass", "Custom");
        check("cchildparentclass", "Custom");
        check("lchildhasclass", "true");
        check("lchildhasbaseclass", "true");
        check("lchildhasparentclass", "true");
        check("lchildhasclasslibrary", "false");
        check("lchildclassreadonly", "true");
        check("lchildbaseclassreadonly", "true");
        check("lchildparentclassreadonly", "true");
        check("ldictset", "true");
        check("ndictcompare", "47");

        const auto child_class_library = state.globals.find("xchildclasslibrary");
        expect(child_class_library != state.globals.end() &&
                   child_class_library->second.kind == copperfin::runtime::PrgValueKind::empty,
               "same-PRG child identity reflection should leave child ClassLibrary empty");

        expect(state.ole_objects.size() == 3U,
               "native child identity reflection should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &child_object = state.ole_objects[1];
            expect(child_object.prog_id == "SaveButton",
                   "native child identity reflection should preserve child class identity");
            expect(child_object.base_class_name == "Custom",
                   "native child identity reflection should preserve child base-class identity");
            expect(child_object.class_library.empty(),
                   "native child identity reflection should keep same-PRG child class-library provenance empty");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native child identity reflection lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_external_base_child_identity_reflects_through_getpem_and_pemstatus()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_prg_child_identity_reflection";
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
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_child_identity_reflection.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 48)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "cChildClass = GETPEM(oCreate.cmdSave, 'Class')\n"
            "cChildBaseClass = GETPEM(oCreate.cmdSave, 'BaseClass')\n"
            "cChildParentClass = GETPEM(oCreate.cmdSave, 'ParentClass')\n"
            "xChildClassLibrary = GETPEM(oCreate.cmdSave, 'ClassLibrary')\n"
            "lChildHasClass = PEMSTATUS(oCreate.cmdSave, 'Class', 1)\n"
            "lChildHasBaseClass = PEMSTATUS(oCreate.cmdSave, 'BaseClass', 1)\n"
            "lChildHasParentClass = PEMSTATUS(oCreate.cmdSave, 'ParentClass', 1)\n"
            "lChildHasClassLibrary = PEMSTATUS(oCreate.cmdSave, 'ClassLibrary', 1)\n"
            "lChildClassReadOnly = PEMSTATUS(oCreate.cmdSave, 'Class', 5)\n"
            "lChildBaseClassReadOnly = PEMSTATUS(oCreate.cmdSave, 'BaseClass', 5)\n"
            "lChildParentClassReadOnly = PEMSTATUS(oCreate.cmdSave, 'ParentClass', 5)\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external child identity reflection script should complete: ") + state.message +
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

        check("cchildclass", "SaveButton");
        check("cchildbaseclass", "Custom");
        check("cchildparentclass", "Custom");
        check("lchildhasclass", "true");
        check("lchildhasbaseclass", "true");
        check("lchildhasparentclass", "true");
        check("lchildhasclasslibrary", "false");
        check("lchildclassreadonly", "true");
        check("lchildbaseclassreadonly", "true");
        check("lchildparentclassreadonly", "true");
        check("ldictset", "true");
        check("ndictcompare", "48");

        const auto child_class_library = state.globals.find("xchildclasslibrary");
        expect(child_class_library != state.globals.end() &&
                   child_class_library->second.kind == copperfin::runtime::PrgValueKind::empty,
               "external-base child identity reflection should leave child ClassLibrary empty when the child class itself has no external base");

        expect(state.ole_objects.size() == 3U,
               "external child identity reflection should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &child_object = state.ole_objects[1];
            expect(child_object.prog_id == "SaveButton",
                   "external child identity reflection should preserve child class identity");
            expect(child_object.base_class_name == "Custom",
                   "external child identity reflection should preserve child base-class identity");
            expect(child_object.class_library.empty(),
                   "external child identity reflection should keep child class-library provenance empty when the child class itself has no external base");
            expect(child_object.source == library_path.string(),
                   "external child identity reflection should preserve the defining PRG path as child provenance");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external child identity reflection lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_same_prg_child_external_base_provenance_surfaces_through_identity_metadata()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_child_external_base_provenance";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path button_library_path = temp_root / "buttons.prg";
        write_text(
            button_library_path,
            "DEFINE CLASS ParentButton AS Custom\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "native_child_external_base_provenance.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('DemoForm')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 64)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "cChildClass = GETPEM(oCreate.cmdSave, 'Class')\n"
            "cChildBaseClass = GETPEM(oCreate.cmdSave, 'BaseClass')\n"
            "cChildParentClass = GETPEM(oCreate.cmdSave, 'ParentClass')\n"
            "cChildClassLibrary = GETPEM(oCreate.cmdSave, 'ClassLibrary')\n"
            "lChildHasClassLibrary = PEMSTATUS(oCreate.cmdSave, 'ClassLibrary', 1)\n"
            "lChildClassLibraryReadOnly = PEMSTATUS(oCreate.cmdSave, 'ClassLibrary', 5)\n"
            "cChildClassLibraryProp = oCreate.cmdSave.ClassLibrary\n"
            "nMembersProps = AMEMBERS(aMembersProps, oCreate.cmdSave, 1)\n"
            "nMembersUnion = AMEMBERS(aMembersUnion, oCreate.cmdSave, 3)\n"
            "cProp1 = aMembersProps[1]\n"
            "cProp2 = aMembersProps[2]\n"
            "cProp3 = aMembersProps[3]\n"
            "cProp4 = aMembersProps[4]\n"
            "cProp5 = aMembersProps[5]\n"
            "cUnion3 = aMembersUnion[3]\n"
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
               std::string("native child external-base provenance script should complete: ") + state.message +
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

        check("cchildclass", "SaveButton");
        check("cchildbaseclass", "ParentButton");
        check("cchildparentclass", "ParentButton");
        check("cchildclasslibrary", button_library_path.string());
        check("lchildhasclasslibrary", "true");
        check("lchildclasslibraryreadonly", "true");
        check("cchildclasslibraryprop", button_library_path.string());
        check("nmembersprops", "5");
        check("nmembersunion", "12");
        check("cprop1", "BASECLASS");
        check("cprop2", "CLASS");
        check("cprop3", "CLASSLIBRARY");
        check("cprop4", "PARENT");
        check("cprop5", "PARENTCLASS");
        check("cunion3", "CLASSLIBRARY");
        check("ldictset", "true");
        check("ndictcompare", "64");

        expect(state.ole_objects.size() == 3U,
               "native child external-base provenance should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "DemoForm",
                   "native child external-base provenance should preserve form identity");
            const auto &child_object = state.ole_objects[1];
            expect(child_object.prog_id == "SaveButton",
                   "native child external-base provenance should preserve child class identity");
            expect(child_object.base_class_name == "ParentButton",
                   "native child external-base provenance should preserve child base-class identity");
            expect(child_object.class_library == button_library_path.string(),
                   "native child external-base provenance should preserve the child external class-library path");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native child external-base provenance lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_external_base_child_external_base_provenance_surfaces_through_identity_metadata()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_prg_child_external_base_provenance";
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

        const fs::path main_path = temp_root / "external_child_external_base_provenance.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 65)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "cChildClass = GETPEM(oCreate.cmdSave, 'Class')\n"
            "cChildBaseClass = GETPEM(oCreate.cmdSave, 'BaseClass')\n"
            "cChildParentClass = GETPEM(oCreate.cmdSave, 'ParentClass')\n"
            "cChildClassLibrary = GETPEM(oCreate.cmdSave, 'ClassLibrary')\n"
            "lChildHasClassLibrary = PEMSTATUS(oCreate.cmdSave, 'ClassLibrary', 1)\n"
            "lChildClassLibraryReadOnly = PEMSTATUS(oCreate.cmdSave, 'ClassLibrary', 5)\n"
            "cChildClassLibraryProp = oCreate.cmdSave.ClassLibrary\n"
            "nMembersProps = AMEMBERS(aMembersProps, oCreate.cmdSave, 1)\n"
            "nMembersUnion = AMEMBERS(aMembersUnion, oCreate.cmdSave, 3)\n"
            "cProp1 = aMembersProps[1]\n"
            "cProp2 = aMembersProps[2]\n"
            "cProp3 = aMembersProps[3]\n"
            "cProp4 = aMembersProps[4]\n"
            "cProp5 = aMembersProps[5]\n"
            "cUnion3 = aMembersUnion[3]\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external child external-base provenance script should complete: ") + state.message +
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

        check("cchildclass", "SaveButton");
        check("cchildbaseclass", "ParentButton");
        check("cchildparentclass", "ParentButton");
        check("cchildclasslibrary", button_library_path.string());
        check("lchildhasclasslibrary", "true");
        check("lchildclasslibraryreadonly", "true");
        check("cchildclasslibraryprop", button_library_path.string());
        check("nmembersprops", "5");
        check("nmembersunion", "12");
        check("cprop1", "BASECLASS");
        check("cprop2", "CLASS");
        check("cprop3", "CLASSLIBRARY");
        check("cprop4", "PARENT");
        check("cprop5", "PARENTCLASS");
        check("cunion3", "CLASSLIBRARY");
        check("ldictset", "true");
        check("ndictcompare", "65");

        expect(state.ole_objects.size() == 3U,
               "external child external-base provenance should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "ChildForm",
                   "external child external-base provenance should preserve form identity");
            const auto &child_object = state.ole_objects[1];
            expect(child_object.prog_id == "SaveButton",
                   "external child external-base provenance should preserve child class identity");
            expect(child_object.base_class_name == "ParentButton",
                   "external child external-base provenance should preserve child base-class identity");
            expect(child_object.class_library == button_library_path.string(),
                   "external child external-base provenance should preserve the child external class-library path");
            expect(child_object.source == form_library_path.string(),
                   "external child external-base provenance should preserve the defining child-class source path");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external child external-base provenance lands");
        }

        fs::remove_all(temp_root, ignored);
    }

}
