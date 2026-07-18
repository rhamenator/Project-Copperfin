#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_same_prg_native_parentclass_reads_through_ordinary_properties()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_parentclass_property_reads";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_parentclass_property_reads.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildWidget')\n"
            "cParentClass = oCreate.ParentClass\n"
            "RETURN\n"
            "DEFINE CLASS ParentWidget AS Custom\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ChildWidget AS ParentWidget\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native ParentClass property-read script should complete: ") + state.message +
                   " @line=" + std::to_string(state.location.line));

        const auto it = state.globals.find("cparentclass");
        if (it == state.globals.end())
        {
            expect(false, "cparentclass variable not found");
        }
        else
        {
            expect(copperfin::runtime::format_value(it->second) == "ParentWidget",
                   std::string("cparentclass expected 'ParentWidget' got '") +
                       copperfin::runtime::format_value(it->second) + "'");
        }

        expect(state.ole_objects.size() == 1U,
               "native ParentClass property reads should register one native object");
        if (state.ole_objects.size() == 1U)
        {
            expect(state.ole_objects[0].prog_id == "ChildWidget",
                   "native ParentClass property reads should preserve child class identity");
            expect(state.ole_objects[0].base_class_name == "ParentWidget",
                   "native ParentClass property reads should preserve the immediate parent class name");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_external_prg_parentclass_reads_through_ordinary_properties()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_prg_parentclass_property_reads";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path library_path = temp_root / "widgetlib.prg";
        write_text(
            library_path,
            "DEFINE CLASS ParentWidget AS Custom\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_parentclass_property_reads.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildWidget')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 34)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "cParentClass = oCreate.ParentClass\n"
            "RETURN\n"
            "DEFINE CLASS ChildWidget AS ParentWidget OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external ParentClass property-read script should complete: ") + state.message +
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
        check("ldictset", "true");
        check("ndictcompare", "34");

        expect(state.ole_objects.size() == 2U,
               "external ParentClass property reads should register native and COM objects");
        if (state.ole_objects.size() == 2U)
        {
            expect(state.ole_objects[0].prog_id == "ChildWidget",
                   "external ParentClass property reads should preserve child class identity");
            expect(state.ole_objects[0].base_class_name == "ParentWidget",
                   "external ParentClass property reads should preserve the immediate external parent class name");
            expect(state.ole_objects[1].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external ParentClass property reads land");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_same_prg_child_identity_metadata_reads_through_ordinary_properties()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_child_identity_property_reads";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_child_identity_property_reads.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('DemoForm')\n"
            "cChildClass = oCreate.cmdSave.Class\n"
            "cChildBaseClass = oCreate.cmdSave.BaseClass\n"
            "cChildParentClass = oCreate.cmdSave.ParentClass\n"
            "xChildClassLibrary = oCreate.cmdSave.ClassLibrary\n"
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
               std::string("native child identity property-read script should complete: ") + state.message +
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

        const auto child_class_library = state.globals.find("xchildclasslibrary");
        expect(child_class_library != state.globals.end() &&
                   child_class_library->second.kind == copperfin::runtime::PrgValueKind::empty,
               "same-PRG child identity property reads should leave child ClassLibrary empty");

        expect(state.ole_objects.size() == 2U,
               "native child identity property reads should register parent and child objects");
        if (state.ole_objects.size() == 2U)
        {
            expect(state.ole_objects[0].prog_id == "DemoForm",
                   "native child identity property reads should preserve form identity");
            expect(state.ole_objects[1].prog_id == "SaveButton",
                   "native child identity property reads should preserve child class identity");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_external_base_child_identity_metadata_reads_through_ordinary_properties()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_prg_child_identity_property_reads";
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

        const fs::path main_path = temp_root / "external_child_identity_property_reads.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 51)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "cChildClass = oCreate.cmdSave.Class\n"
            "cChildBaseClass = oCreate.cmdSave.BaseClass\n"
            "cChildParentClass = oCreate.cmdSave.ParentClass\n"
            "xChildClassLibrary = oCreate.cmdSave.ClassLibrary\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external child identity property-read script should complete: ") + state.message +
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
        check("ldictset", "true");
        check("ndictcompare", "51");

        const auto child_class_library = state.globals.find("xchildclasslibrary");
        expect(child_class_library != state.globals.end() &&
                   child_class_library->second.kind == copperfin::runtime::PrgValueKind::empty,
               "external child identity property reads should leave child ClassLibrary empty when the child class itself has no external base");

        expect(state.ole_objects.size() == 3U,
               "external child identity property reads should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "ChildForm",
                   "external child identity property reads should preserve form identity");
            expect(state.ole_objects[1].prog_id == "SaveButton",
                   "external child identity property reads should preserve child class identity");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external child identity property reads land");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_same_prg_child_identity_metadata_stays_read_only_to_setpem()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_child_identity_setpem";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_child_identity_setpem.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('DemoForm')\n"
            "lChildClassReadOnly = PEMSTATUS(oCreate.cmdSave, 'Class', 5)\n"
            "lChildBaseClassReadOnly = PEMSTATUS(oCreate.cmdSave, 'BaseClass', 5)\n"
            "lChildParentClassReadOnly = PEMSTATUS(oCreate.cmdSave, 'ParentClass', 5)\n"
            "lSetChildClass = SETPEM(oCreate.cmdSave, 'Class', 'OtherClass')\n"
            "lSetChildBaseClass = SETPEM(oCreate.cmdSave, 'BaseClass', 'OtherBase')\n"
            "lSetChildParentClass = SETPEM(oCreate.cmdSave, 'ParentClass', 'OtherParent')\n"
            "lSetChildClassLibrary = SETPEM(oCreate.cmdSave, 'ClassLibrary', 'other.prg')\n"
            "cChildClassAfter = GETPEM(oCreate.cmdSave, 'Class')\n"
            "cChildBaseClassAfter = GETPEM(oCreate.cmdSave, 'BaseClass')\n"
            "cChildParentClassAfter = GETPEM(oCreate.cmdSave, 'ParentClass')\n"
            "xChildClassLibraryAfter = GETPEM(oCreate.cmdSave, 'ClassLibrary')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 52)\n"
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
               std::string("native child identity SETPEM script should complete: ") + state.message +
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
        check("cchildbaseclassafter", "Custom");
        check("cchildparentclassafter", "Custom");
        check("ldictset", "true");
        check("ndictcompare", "52");

        const auto child_class_library_after = state.globals.find("xchildclasslibraryafter");
        expect(child_class_library_after != state.globals.end() &&
                   child_class_library_after->second.kind == copperfin::runtime::PrgValueKind::empty,
               "same-PRG child identity SETPEM should leave child ClassLibrary empty after failed mutation");

        expect(state.ole_objects.size() == 3U,
               "native child identity SETPEM should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "DemoForm",
                   "native child identity SETPEM should preserve parent form identity");
            const auto &child_object = state.ole_objects[1];
            expect(child_object.prog_id == "SaveButton",
                   "native child identity SETPEM should preserve child class identity");
            expect(child_object.base_class_name == "Custom",
                   "native child identity SETPEM should preserve child base-class identity");
            expect(child_object.class_library.empty(),
                   "native child identity SETPEM should preserve empty same-PRG child class-library provenance");
            expect(!child_object.properties.contains("class"),
                   "native child identity SETPEM should not materialize a writable child Class shadow");
            expect(!child_object.properties.contains("baseclass"),
                   "native child identity SETPEM should not materialize a writable child BaseClass shadow");
            expect(!child_object.properties.contains("parentclass"),
                   "native child identity SETPEM should not materialize a writable child ParentClass shadow");
            expect(!child_object.properties.contains("classlibrary"),
                   "native child identity SETPEM should not materialize a writable child ClassLibrary shadow");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native child identity SETPEM lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_external_base_child_identity_metadata_stays_read_only_to_setpem()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_prg_child_identity_setpem";
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

        const fs::path main_path = temp_root / "external_child_identity_setpem.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "lChildClassReadOnly = PEMSTATUS(oCreate.cmdSave, 'Class', 5)\n"
            "lChildBaseClassReadOnly = PEMSTATUS(oCreate.cmdSave, 'BaseClass', 5)\n"
            "lChildParentClassReadOnly = PEMSTATUS(oCreate.cmdSave, 'ParentClass', 5)\n"
            "lSetChildClass = SETPEM(oCreate.cmdSave, 'Class', 'OtherClass')\n"
            "lSetChildBaseClass = SETPEM(oCreate.cmdSave, 'BaseClass', 'OtherBase')\n"
            "lSetChildParentClass = SETPEM(oCreate.cmdSave, 'ParentClass', 'OtherParent')\n"
            "lSetChildClassLibrary = SETPEM(oCreate.cmdSave, 'ClassLibrary', 'other.prg')\n"
            "cChildClassAfter = GETPEM(oCreate.cmdSave, 'Class')\n"
            "cChildBaseClassAfter = GETPEM(oCreate.cmdSave, 'BaseClass')\n"
            "cChildParentClassAfter = GETPEM(oCreate.cmdSave, 'ParentClass')\n"
            "xChildClassLibraryAfter = GETPEM(oCreate.cmdSave, 'ClassLibrary')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 53)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external child identity SETPEM script should complete: ") + state.message +
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
        check("cchildbaseclassafter", "Custom");
        check("cchildparentclassafter", "Custom");
        check("ldictset", "true");
        check("ndictcompare", "53");

        const auto child_class_library_after = state.globals.find("xchildclasslibraryafter");
        expect(child_class_library_after != state.globals.end() &&
                   child_class_library_after->second.kind == copperfin::runtime::PrgValueKind::empty,
               "external child identity SETPEM should leave child ClassLibrary empty when the child class itself has no external base");

        expect(state.ole_objects.size() == 3U,
               "external child identity SETPEM should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "ChildForm",
                   "external child identity SETPEM should preserve parent form identity");
            const auto &child_object = state.ole_objects[1];
            expect(child_object.prog_id == "SaveButton",
                   "external child identity SETPEM should preserve child class identity");
            expect(child_object.base_class_name == "Custom",
                   "external child identity SETPEM should preserve child base-class identity");
            expect(child_object.class_library.empty(),
                   "external child identity SETPEM should keep child class-library provenance empty when the child class itself has no external base");
            expect(child_object.source == library_path.string(),
                   "external child identity SETPEM should preserve the defining PRG path as child provenance");
            expect(!child_object.properties.contains("class"),
                   "external child identity SETPEM should not materialize a writable child Class shadow");
            expect(!child_object.properties.contains("baseclass"),
                   "external child identity SETPEM should not materialize a writable child BaseClass shadow");
            expect(!child_object.properties.contains("parentclass"),
                   "external child identity SETPEM should not materialize a writable child ParentClass shadow");
            expect(!child_object.properties.contains("classlibrary"),
                   "external child identity SETPEM should not materialize a writable child ClassLibrary shadow");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external child identity SETPEM lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_same_prg_child_identity_metadata_cannot_be_shadowed_through_addproperty()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_child_identity_addproperty";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_child_identity_addproperty.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('DemoForm')\n"
            "lAddChildClass = ADDPROPERTY(oCreate.cmdSave, 'Class', 'OtherClass')\n"
            "lAddChildBaseClass = ADDPROPERTY(oCreate.cmdSave, 'BaseClass', 'OtherBase')\n"
            "lAddChildParentClass = ADDPROPERTY(oCreate.cmdSave, 'ParentClass', 'OtherParent')\n"
            "lAddChildClassLibrary = ADDPROPERTY(oCreate.cmdSave, 'ClassLibrary', 'other.prg')\n"
            "cChildClassAfter = GETPEM(oCreate.cmdSave, 'Class')\n"
            "cChildBaseClassAfter = GETPEM(oCreate.cmdSave, 'BaseClass')\n"
            "cChildParentClassAfter = GETPEM(oCreate.cmdSave, 'ParentClass')\n"
            "xChildClassLibraryAfter = GETPEM(oCreate.cmdSave, 'ClassLibrary')\n"
            "lChildClassReadOnly = PEMSTATUS(oCreate.cmdSave, 'Class', 5)\n"
            "lChildBaseClassReadOnly = PEMSTATUS(oCreate.cmdSave, 'BaseClass', 5)\n"
            "lChildParentClassReadOnly = PEMSTATUS(oCreate.cmdSave, 'ParentClass', 5)\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 54)\n"
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
               std::string("native child identity ADDPROPERTY script should complete: ") + state.message +
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
        check("cchildbaseclassafter", "Custom");
        check("cchildparentclassafter", "Custom");
        check("lchildclassreadonly", "true");
        check("lchildbaseclassreadonly", "true");
        check("lchildparentclassreadonly", "true");
        check("ldictset", "true");
        check("ndictcompare", "54");

        const auto child_class_library_after = state.globals.find("xchildclasslibraryafter");
        expect(child_class_library_after != state.globals.end() &&
                   child_class_library_after->second.kind == copperfin::runtime::PrgValueKind::empty,
               "same-PRG child identity ADDPROPERTY should leave child ClassLibrary empty after failed shadow creation");

        expect(state.ole_objects.size() == 3U,
               "native child identity ADDPROPERTY should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "DemoForm",
                   "native child identity ADDPROPERTY should preserve parent form identity");
            const auto &child_object = state.ole_objects[1];
            expect(child_object.prog_id == "SaveButton",
                   "native child identity ADDPROPERTY should preserve child class identity");
            expect(child_object.base_class_name == "Custom",
                   "native child identity ADDPROPERTY should preserve child base-class identity");
            expect(child_object.class_library.empty(),
                   "native child identity ADDPROPERTY should preserve empty same-PRG child class-library provenance");
            expect(!child_object.properties.contains("class"),
                   "native child identity ADDPROPERTY should not materialize a child Class shadow");
            expect(!child_object.properties.contains("baseclass"),
                   "native child identity ADDPROPERTY should not materialize a child BaseClass shadow");
            expect(!child_object.properties.contains("parentclass"),
                   "native child identity ADDPROPERTY should not materialize a child ParentClass shadow");
            expect(!child_object.properties.contains("classlibrary"),
                   "native child identity ADDPROPERTY should not materialize a child ClassLibrary shadow");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native child identity ADDPROPERTY lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_external_base_child_identity_metadata_cannot_be_shadowed_through_addproperty()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_prg_child_identity_addproperty";
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

        const fs::path main_path = temp_root / "external_child_identity_addproperty.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "lAddChildClass = ADDPROPERTY(oCreate.cmdSave, 'Class', 'OtherClass')\n"
            "lAddChildBaseClass = ADDPROPERTY(oCreate.cmdSave, 'BaseClass', 'OtherBase')\n"
            "lAddChildParentClass = ADDPROPERTY(oCreate.cmdSave, 'ParentClass', 'OtherParent')\n"
            "lAddChildClassLibrary = ADDPROPERTY(oCreate.cmdSave, 'ClassLibrary', 'other.prg')\n"
            "cChildClassAfter = GETPEM(oCreate.cmdSave, 'Class')\n"
            "cChildBaseClassAfter = GETPEM(oCreate.cmdSave, 'BaseClass')\n"
            "cChildParentClassAfter = GETPEM(oCreate.cmdSave, 'ParentClass')\n"
            "xChildClassLibraryAfter = GETPEM(oCreate.cmdSave, 'ClassLibrary')\n"
            "lChildClassReadOnly = PEMSTATUS(oCreate.cmdSave, 'Class', 5)\n"
            "lChildBaseClassReadOnly = PEMSTATUS(oCreate.cmdSave, 'BaseClass', 5)\n"
            "lChildParentClassReadOnly = PEMSTATUS(oCreate.cmdSave, 'ParentClass', 5)\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 55)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external child identity ADDPROPERTY script should complete: ") + state.message +
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
        check("cchildbaseclassafter", "Custom");
        check("cchildparentclassafter", "Custom");
        check("lchildclassreadonly", "true");
        check("lchildbaseclassreadonly", "true");
        check("lchildparentclassreadonly", "true");
        check("ldictset", "true");
        check("ndictcompare", "55");

        const auto child_class_library_after = state.globals.find("xchildclasslibraryafter");
        expect(child_class_library_after != state.globals.end() &&
                   child_class_library_after->second.kind == copperfin::runtime::PrgValueKind::empty,
               "external child identity ADDPROPERTY should leave child ClassLibrary empty when the child class itself has no external base");

        expect(state.ole_objects.size() == 3U,
               "external child identity ADDPROPERTY should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "ChildForm",
                   "external child identity ADDPROPERTY should preserve parent form identity");
            const auto &child_object = state.ole_objects[1];
            expect(child_object.prog_id == "SaveButton",
                   "external child identity ADDPROPERTY should preserve child class identity");
            expect(child_object.base_class_name == "Custom",
                   "external child identity ADDPROPERTY should preserve child base-class identity");
            expect(child_object.class_library.empty(),
                   "external child identity ADDPROPERTY should keep child class-library provenance empty when the child class itself has no external base");
            expect(child_object.source == library_path.string(),
                   "external child identity ADDPROPERTY should preserve the defining PRG path as child provenance");
            expect(!child_object.properties.contains("class"),
                   "external child identity ADDPROPERTY should not materialize a child Class shadow");
            expect(!child_object.properties.contains("baseclass"),
                   "external child identity ADDPROPERTY should not materialize a child BaseClass shadow");
            expect(!child_object.properties.contains("parentclass"),
                   "external child identity ADDPROPERTY should not materialize a child ParentClass shadow");
            expect(!child_object.properties.contains("classlibrary"),
                   "external child identity ADDPROPERTY should not materialize a child ClassLibrary shadow");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external child identity ADDPROPERTY lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_same_prg_child_identity_metadata_stays_protected_from_removeproperty()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_child_identity_removeproperty";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_child_identity_removeproperty.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('DemoForm')\n"
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
            "lDictSet = SETPEM(oDict, 'comparemode', 56)\n"
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
               std::string("native child identity REMOVEPROPERTY script should complete: ") + state.message +
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
        check("ndictcompare", "56");

        const auto child_class_library_after = state.globals.find("xchildclasslibraryafter");
        expect(child_class_library_after != state.globals.end() &&
                   child_class_library_after->second.kind == copperfin::runtime::PrgValueKind::empty,
               "same-PRG child identity REMOVEPROPERTY should leave child ClassLibrary empty after failed removal");

        expect(state.ole_objects.size() == 3U,
               "native child identity REMOVEPROPERTY should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "DemoForm",
                   "native child identity REMOVEPROPERTY should preserve parent form identity");
            const auto &child_object = state.ole_objects[1];
            expect(child_object.prog_id == "SaveButton",
                   "native child identity REMOVEPROPERTY should preserve child class identity");
            expect(child_object.base_class_name == "Custom",
                   "native child identity REMOVEPROPERTY should preserve child base-class identity");
            expect(child_object.class_library.empty(),
                   "native child identity REMOVEPROPERTY should preserve empty same-PRG child class-library provenance");
            expect(!child_object.properties.contains("class"),
                   "native child identity REMOVEPROPERTY should not materialize a child Class shadow");
            expect(!child_object.properties.contains("baseclass"),
                   "native child identity REMOVEPROPERTY should not materialize a child BaseClass shadow");
            expect(!child_object.properties.contains("parentclass"),
                   "native child identity REMOVEPROPERTY should not materialize a child ParentClass shadow");
            expect(!child_object.properties.contains("classlibrary"),
                   "native child identity REMOVEPROPERTY should not materialize a child ClassLibrary shadow");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native child identity REMOVEPROPERTY lands");
        }

        fs::remove_all(temp_root, ignored);
    }

}
