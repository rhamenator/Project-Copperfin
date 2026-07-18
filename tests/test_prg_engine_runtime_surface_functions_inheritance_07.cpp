#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_external_base_child_parent_stays_read_only_to_setpem()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_prg_child_parent_setpem";
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

        const fs::path main_path = temp_root / "external_child_parent_setpem.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "lParentReadOnly = PEMSTATUS(oCreate.cmdSave, 'Parent', 5)\n"
            "lSetParent = SETPEM(oCreate.cmdSave, 'Parent', 'OtherParent')\n"
            "oParentRef = GETPEM(oCreate.cmdSave, 'Parent')\n"
            "cParentCaption = oParentRef.Caption\n"
            "lChildHasParent = PEMSTATUS(oCreate.cmdSave, 'Parent', 1)\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 40)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external child Parent SETPEM script should complete: ") + state.message +
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
        check("ndictcompare", "40");

        expect(state.ole_objects.size() == 3U,
               "external child Parent SETPEM should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "ChildForm",
                   "external child Parent SETPEM should preserve form identity");
            const auto &child_object = state.ole_objects[1];
            expect(child_object.prog_id == "SaveButton",
                   "external child Parent SETPEM should preserve child identity");
            const auto child_parent = child_object.properties.find("parent");
            if (child_parent != child_object.properties.end())
            {
                expect(child_parent->second.kind == copperfin::runtime::PrgValueKind::string,
                       "external child Parent SETPEM should keep the built-in PARENT object reference materialized");
                expect(child_parent->second.string_value == "object:ChildForm#" + std::to_string(state.ole_objects[0].handle),
                       "external child Parent SETPEM should not replace the built-in PARENT object reference");
            }
            else
            {
                expect(false, "external child Parent SETPEM should preserve the child PARENT reference");
            }
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external child Parent SETPEM lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_same_prg_child_parent_cannot_be_shadowed_through_addproperty()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_child_parent_addproperty";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_child_parent_addproperty.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('DemoForm')\n"
            "lAddParent = ADDPROPERTY(oCreate.cmdSave, 'Parent', 'OtherParent')\n"
            "oParentRef = GETPEM(oCreate.cmdSave, 'Parent')\n"
            "cParentCaption = oParentRef.Caption\n"
            "lChildHasParent = PEMSTATUS(oCreate.cmdSave, 'Parent', 1)\n"
            "lParentReadOnly = PEMSTATUS(oCreate.cmdSave, 'Parent', 5)\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 41)\n"
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
               std::string("native child Parent ADDPROPERTY script should complete: ") + state.message +
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

        check("laddparent", "false");
        check("cparentcaption", "MainForm");
        check("lchildhasparent", "true");
        check("lparentreadonly", "true");
        check("ldictset", "true");
        check("ndictcompare", "41");

        expect(state.ole_objects.size() == 3U,
               "native child Parent ADDPROPERTY should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "DemoForm",
                   "native child Parent ADDPROPERTY should preserve form identity");
            const auto &child_object = state.ole_objects[1];
            expect(child_object.prog_id == "SaveButton",
                   "native child Parent ADDPROPERTY should preserve child identity");
            const auto child_parent = child_object.properties.find("parent");
            if (child_parent != child_object.properties.end())
            {
                expect(child_parent->second.kind == copperfin::runtime::PrgValueKind::string,
                       "native child Parent ADDPROPERTY should keep the built-in PARENT object reference materialized");
                expect(child_parent->second.string_value == "object:DemoForm#" + std::to_string(state.ole_objects[0].handle),
                       "native child Parent ADDPROPERTY should not replace the built-in PARENT object reference");
            }
            else
            {
                expect(false, "native child Parent ADDPROPERTY should preserve the child PARENT reference");
            }
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native child Parent ADDPROPERTY lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_external_base_child_parent_cannot_be_shadowed_through_addproperty()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_prg_child_parent_addproperty";
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

        const fs::path main_path = temp_root / "external_child_parent_addproperty.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "lAddParent = ADDPROPERTY(oCreate.cmdSave, 'Parent', 'OtherParent')\n"
            "oParentRef = GETPEM(oCreate.cmdSave, 'Parent')\n"
            "cParentCaption = oParentRef.Caption\n"
            "lChildHasParent = PEMSTATUS(oCreate.cmdSave, 'Parent', 1)\n"
            "lParentReadOnly = PEMSTATUS(oCreate.cmdSave, 'Parent', 5)\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 42)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external child Parent ADDPROPERTY script should complete: ") + state.message +
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

        check("laddparent", "false");
        check("cparentcaption", "MainForm");
        check("lchildhasparent", "true");
        check("lparentreadonly", "true");
        check("ldictset", "true");
        check("ndictcompare", "42");

        expect(state.ole_objects.size() == 3U,
               "external child Parent ADDPROPERTY should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "ChildForm",
                   "external child Parent ADDPROPERTY should preserve form identity");
            const auto &child_object = state.ole_objects[1];
            expect(child_object.prog_id == "SaveButton",
                   "external child Parent ADDPROPERTY should preserve child identity");
            const auto child_parent = child_object.properties.find("parent");
            if (child_parent != child_object.properties.end())
            {
                expect(child_parent->second.kind == copperfin::runtime::PrgValueKind::string,
                       "external child Parent ADDPROPERTY should keep the built-in PARENT object reference materialized");
                expect(child_parent->second.string_value == "object:ChildForm#" + std::to_string(state.ole_objects[0].handle),
                       "external child Parent ADDPROPERTY should not replace the built-in PARENT object reference");
            }
            else
            {
                expect(false, "external child Parent ADDPROPERTY should preserve the child PARENT reference");
            }
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external child Parent ADDPROPERTY lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_same_prg_child_parent_stays_protected_from_removeproperty()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_child_parent_removeproperty";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_child_parent_removeproperty.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('DemoForm')\n"
            "lRemoveParent = REMOVEPROPERTY(oCreate.cmdSave, 'Parent')\n"
            "oParentRef = GETPEM(oCreate.cmdSave, 'Parent')\n"
            "cParentCaption = oParentRef.Caption\n"
            "lChildHasParent = PEMSTATUS(oCreate.cmdSave, 'Parent', 1)\n"
            "lParentReadOnly = PEMSTATUS(oCreate.cmdSave, 'Parent', 5)\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 43)\n"
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
               std::string("native child Parent REMOVEPROPERTY script should complete: ") + state.message +
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

        check("lremoveparent", "false");
        check("cparentcaption", "MainForm");
        check("lchildhasparent", "true");
        check("lparentreadonly", "true");
        check("ldictset", "true");
        check("ndictcompare", "43");

        expect(state.ole_objects.size() == 3U,
               "native child Parent REMOVEPROPERTY should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "DemoForm",
                   "native child Parent REMOVEPROPERTY should preserve form identity");
            const auto &child_object = state.ole_objects[1];
            expect(child_object.prog_id == "SaveButton",
                   "native child Parent REMOVEPROPERTY should preserve child identity");
            const auto child_parent = child_object.properties.find("parent");
            if (child_parent != child_object.properties.end())
            {
                expect(child_parent->second.kind == copperfin::runtime::PrgValueKind::string,
                       "native child Parent REMOVEPROPERTY should keep the built-in PARENT object reference materialized");
                expect(child_parent->second.string_value == "object:DemoForm#" + std::to_string(state.ole_objects[0].handle),
                       "native child Parent REMOVEPROPERTY should not erase the built-in PARENT object reference");
            }
            else
            {
                expect(false, "native child Parent REMOVEPROPERTY should preserve the child PARENT reference");
            }
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native child Parent REMOVEPROPERTY lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_external_base_child_parent_stays_protected_from_removeproperty()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_prg_child_parent_removeproperty";
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

        const fs::path main_path = temp_root / "external_child_parent_removeproperty.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "lRemoveParent = REMOVEPROPERTY(oCreate.cmdSave, 'Parent')\n"
            "oParentRef = GETPEM(oCreate.cmdSave, 'Parent')\n"
            "cParentCaption = oParentRef.Caption\n"
            "lChildHasParent = PEMSTATUS(oCreate.cmdSave, 'Parent', 1)\n"
            "lParentReadOnly = PEMSTATUS(oCreate.cmdSave, 'Parent', 5)\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 44)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external child Parent REMOVEPROPERTY script should complete: ") + state.message +
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

        check("lremoveparent", "false");
        check("cparentcaption", "MainForm");
        check("lchildhasparent", "true");
        check("lparentreadonly", "true");
        check("ldictset", "true");
        check("ndictcompare", "44");

        expect(state.ole_objects.size() == 3U,
               "external child Parent REMOVEPROPERTY should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "ChildForm",
                   "external child Parent REMOVEPROPERTY should preserve form identity");
            const auto &child_object = state.ole_objects[1];
            expect(child_object.prog_id == "SaveButton",
                   "external child Parent REMOVEPROPERTY should preserve child identity");
            const auto child_parent = child_object.properties.find("parent");
            if (child_parent != child_object.properties.end())
            {
                expect(child_parent->second.kind == copperfin::runtime::PrgValueKind::string,
                       "external child Parent REMOVEPROPERTY should keep the built-in PARENT object reference materialized");
                expect(child_parent->second.string_value == "object:ChildForm#" + std::to_string(state.ole_objects[0].handle),
                       "external child Parent REMOVEPROPERTY should not erase the built-in PARENT object reference");
            }
            else
            {
                expect(false, "external child Parent REMOVEPROPERTY should preserve the child PARENT reference");
            }
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external child Parent REMOVEPROPERTY lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_same_prg_child_parent_cannot_be_shadowed_through_direct_assignment()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_child_parent_direct_assignment";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_child_parent_direct_assignment.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('DemoForm')\n"
            "oCreate.cmdSave.Parent = 'OtherParent'\n"
            "oParentRef = GETPEM(oCreate.cmdSave, 'Parent')\n"
            "cParentCaption = oParentRef.Caption\n"
            "lChildHasParent = PEMSTATUS(oCreate.cmdSave, 'Parent', 1)\n"
            "lParentReadOnly = PEMSTATUS(oCreate.cmdSave, 'Parent', 5)\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 45)\n"
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
               std::string("native child Parent direct-assignment script should complete: ") + state.message +
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

        check("cparentcaption", "MainForm");
        check("lchildhasparent", "true");
        check("lparentreadonly", "true");
        check("ldictset", "true");
        check("ndictcompare", "45");

        expect(state.ole_objects.size() == 3U,
               "native child Parent direct assignment should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "DemoForm",
                   "native child Parent direct assignment should preserve form identity");
            const auto &child_object = state.ole_objects[1];
            expect(child_object.prog_id == "SaveButton",
                   "native child Parent direct assignment should preserve child identity");
            const auto child_parent = child_object.properties.find("parent");
            if (child_parent != child_object.properties.end())
            {
                expect(child_parent->second.kind == copperfin::runtime::PrgValueKind::string,
                       "native child Parent direct assignment should keep the built-in PARENT object reference materialized");
                expect(child_parent->second.string_value == "object:DemoForm#" + std::to_string(state.ole_objects[0].handle),
                       "native child Parent direct assignment should not replace the built-in PARENT object reference");
            }
            else
            {
                expect(false, "native child Parent direct assignment should preserve the child PARENT reference");
            }
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native child Parent direct assignment lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_external_base_child_parent_cannot_be_shadowed_through_direct_assignment()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_prg_child_parent_direct_assignment";
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

        const fs::path main_path = temp_root / "external_child_parent_direct_assignment.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oCreate.cmdSave.Parent = 'OtherParent'\n"
            "oParentRef = GETPEM(oCreate.cmdSave, 'Parent')\n"
            "cParentCaption = oParentRef.Caption\n"
            "lChildHasParent = PEMSTATUS(oCreate.cmdSave, 'Parent', 1)\n"
            "lParentReadOnly = PEMSTATUS(oCreate.cmdSave, 'Parent', 5)\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 46)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external child Parent direct-assignment script should complete: ") + state.message +
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

        check("cparentcaption", "MainForm");
        check("lchildhasparent", "true");
        check("lparentreadonly", "true");
        check("ldictset", "true");
        check("ndictcompare", "46");

        expect(state.ole_objects.size() == 3U,
               "external child Parent direct assignment should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "ChildForm",
                   "external child Parent direct assignment should preserve form identity");
            const auto &child_object = state.ole_objects[1];
            expect(child_object.prog_id == "SaveButton",
                   "external child Parent direct assignment should preserve child identity");
            const auto child_parent = child_object.properties.find("parent");
            if (child_parent != child_object.properties.end())
            {
                expect(child_parent->second.kind == copperfin::runtime::PrgValueKind::string,
                       "external child Parent direct assignment should keep the built-in PARENT object reference materialized");
                expect(child_parent->second.string_value == "object:ChildForm#" + std::to_string(state.ole_objects[0].handle),
                       "external child Parent direct assignment should not replace the built-in PARENT object reference");
            }
            else
            {
                expect(false, "external child Parent direct assignment should preserve the child PARENT reference");
            }
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external child Parent direct assignment lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_inherited_declarative_children_from_external_prg_bases_resolve_against_defining_library()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_external_base_inherited_children";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path library_path = temp_root / "widgetlib.prg";
        write_text(
            library_path,
            "DEFINE CLASS ParentForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    cInitChildCaption = ''\n"
            "    cInitOwnerCaption = ''\n"
            "    OBJECT cmdSave AS SaveButton\n"
            "        Caption = 'Commit'\n"
            "    ENDOBJECT\n"
            "    PROCEDURE Init\n"
            "        THIS.cInitChildCaption = THIS.cmdSave.Caption\n"
            "        THIS.cInitOwnerCaption = THIS.cmdSave.OwnerCaption()\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
            "    Caption = 'Save'\n"
            "    FUNCTION OwnerCaption\n"
            "        RETURN PARENT.Caption\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_base_inherited_children.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oNew = NEWOBJECT('LeafForm')\n"
            "lCreateHasChild = PEMSTATUS(oCreate, 'cmdSave', 1)\n"
            "lNewHasChild = PEMSTATUS(oNew, 'cmdSave', 1)\n"
            "cCreateInitChildCaption = oCreate.cInitChildCaption\n"
            "cNewInitChildCaption = oNew.cInitChildCaption\n"
            "cCreateInitOwnerCaption = oCreate.cInitOwnerCaption\n"
            "cNewInitOwnerCaption = oNew.cInitOwnerCaption\n"
            "cCreateChildCaption = oCreate.cmdSave.Caption\n"
            "cNewChildCaption = oNew.cmdSave.Caption\n"
            "cCreateOwnerCaption = oCreate.cmdSave.OwnerCaption()\n"
            "cNewOwnerCaption = oNew.cmdSave.OwnerCaption()\n"
            "lCreateChildHasParent = PEMSTATUS(oCreate.cmdSave, 'Parent', 1)\n"
            "lNewChildHasParent = PEMSTATUS(oNew.cmdSave, 'Parent', 1)\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 17)\n"
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
               std::string("external-base inherited declarative children script should complete: ") + state.message +
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
        check("ccreateinitchildcaption", "Commit");
        check("cnewinitchildcaption", "Commit");
        check("ccreateinitownercaption", "MainForm");
        check("cnewinitownercaption", "MainForm");
        check("ccreatechildcaption", "Commit");
        check("cnewchildcaption", "Commit");
        check("ccreateownercaption", "MainForm");
        check("cnewownercaption", "MainForm");
        check("lcreatechildhasparent", "true");
        check("lnewchildhasparent", "true");
        check("ldictset", "true");
        check("ndictcompare", "17");

        expect(state.ole_objects.size() == 5U,
               "external-base inherited declarative children script should register CREATEOBJECT parent/child, NEWOBJECT parent/child, and COM objects");
        if (state.ole_objects.size() == 5U)
        {
            const auto &create_parent = state.ole_objects[0];
            const auto &create_child = state.ole_objects[1];
            const auto &new_parent = state.ole_objects[2];
            const auto &new_child = state.ole_objects[3];
            expect(create_parent.prog_id == "ChildForm",
                   "external-base inherited declarative children should preserve CREATEOBJECT parent class identity");
            expect(create_child.prog_id == "SaveButton",
                   "external-base inherited declarative children should materialize the inherited child class");
            expect(create_child.source == library_path.string(),
                   "external-base inherited declarative children should resolve inherited child classes against the defining external PRG");
            expect(new_parent.prog_id == "LeafForm",
                   "external-base inherited declarative children should preserve NEWOBJECT leaf class identity");
            expect(new_child.prog_id == "SaveButton",
                   "external-base inherited declarative children should materialize the inherited child class for leaf instances");
            expect(new_child.source == library_path.string(),
                   "external-base inherited declarative children should preserve external PRG provenance for leaf child instances");
            const auto create_child_caption = create_child.properties.find("caption");
            const auto new_child_caption = new_child.properties.find("caption");
            if (create_child_caption != create_child.properties.end())
            {
                expect(copperfin::runtime::format_value(create_child_caption->second) == "Commit",
                       "external-base inherited declarative children should preserve inherited child property overrides");
            }
            else
            {
                expect(false, "external-base inherited declarative children should materialize inherited child caption overrides");
            }
            if (new_child_caption != new_child.properties.end())
            {
                expect(copperfin::runtime::format_value(new_child_caption->second) == "Commit",
                       "external-base inherited declarative children should preserve inherited child overrides on leaf instances");
            }
            else
            {
                expect(false, "external-base inherited declarative children should materialize inherited child caption overrides on leaf instances");
            }
            expect(state.ole_objects[4].prog_id == "Scripting.Dictionary",
                   "COM behavior should remain stable while external-base inherited declarative children land");
        }

        const bool has_addobject_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.addobject" &&
                   (event.detail == "ChildForm.cmdsave:SaveButton" ||
                    event.detail == "LeafForm.cmdsave:SaveButton");
        });
        expect(has_addobject_event,
               "external-base inherited declarative children should emit child materialization events");

        fs::remove_all(temp_root, ignored);
    }

}
