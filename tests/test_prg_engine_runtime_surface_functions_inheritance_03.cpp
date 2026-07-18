#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_same_prg_native_identity_metadata_stays_protected_from_removeproperty()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_identity_removeproperty";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_identity_removeproperty.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildWidget')\n"
            "lRemoveClass = REMOVEPROPERTY(oCreate, 'Class')\n"
            "lRemoveBaseClass = REMOVEPROPERTY(oCreate, 'BaseClass')\n"
            "lRemoveParentClass = REMOVEPROPERTY(oCreate, 'ParentClass')\n"
            "lRemoveClassLibrary = REMOVEPROPERTY(oCreate, 'ClassLibrary')\n"
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
               std::string("native identity REMOVEPROPERTY script should complete: ") + state.message +
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

        check("lremoveclass", "false");
        check("lremovebaseclass", "false");
        check("lremoveparentclass", "false");
        check("lremoveclasslibrary", "false");
        check("cclassafter", "ChildWidget");
        check("cbaseclassafter", "ParentWidget");
        check("cparentclassafter", "ParentWidget");

        const auto class_library_after = state.globals.find("xclasslibraryafter");
        expect(class_library_after != state.globals.end() &&
                   class_library_after->second.kind == copperfin::runtime::PrgValueKind::empty,
               "same-PRG identity REMOVEPROPERTY should leave ClassLibrary empty after failed removal");

        expect(state.ole_objects.size() == 1U,
               "native identity REMOVEPROPERTY should register one native object");
        if (state.ole_objects.size() == 1U)
        {
            const auto &native_object = state.ole_objects[0];
            expect(native_object.prog_id == "ChildWidget",
                   "native identity REMOVEPROPERTY should preserve child class identity");
            expect(native_object.base_class_name == "ParentWidget",
                   "native identity REMOVEPROPERTY should preserve the immediate base class");
            expect(native_object.class_library.empty(),
                   "native identity REMOVEPROPERTY should preserve empty same-PRG class library provenance");
            expect(!native_object.properties.contains("class"),
                   "native identity REMOVEPROPERTY should not materialize a writable Class property shadow");
            expect(!native_object.properties.contains("baseclass"),
                   "native identity REMOVEPROPERTY should not materialize a writable BaseClass property shadow");
            expect(!native_object.properties.contains("parentclass"),
                   "native identity REMOVEPROPERTY should not materialize a writable ParentClass property shadow");
            expect(!native_object.properties.contains("classlibrary"),
                   "native identity REMOVEPROPERTY should not materialize a writable ClassLibrary property shadow");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_external_prg_identity_metadata_stays_protected_from_removeproperty()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_prg_identity_removeproperty";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path library_path = temp_root / "widgetlib.prg";
        write_text(
            library_path,
            "DEFINE CLASS ParentWidget AS Custom\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_identity_removeproperty.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildWidget')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 29)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "lRemoveClass = REMOVEPROPERTY(oCreate, 'Class')\n"
            "lRemoveBaseClass = REMOVEPROPERTY(oCreate, 'BaseClass')\n"
            "lRemoveParentClass = REMOVEPROPERTY(oCreate, 'ParentClass')\n"
            "lRemoveClassLibrary = REMOVEPROPERTY(oCreate, 'ClassLibrary')\n"
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
               std::string("external identity REMOVEPROPERTY script should complete: ") + state.message +
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

        check("lremoveclass", "false");
        check("lremovebaseclass", "false");
        check("lremoveparentclass", "false");
        check("lremoveclasslibrary", "false");
        check("cclassafter", "ChildWidget");
        check("cbaseclassafter", "ParentWidget");
        check("cparentclassafter", "ParentWidget");
        check("cclasslibraryafter", library_path.string());
        check("ldictset", "true");
        check("ndictcompare", "29");

        expect(state.ole_objects.size() == 2U,
               "external identity REMOVEPROPERTY should register native and COM objects");
        if (state.ole_objects.size() == 2U)
        {
            const auto &native_object = state.ole_objects[0];
            expect(native_object.prog_id == "ChildWidget",
                   "external identity REMOVEPROPERTY should preserve child class identity");
            expect(native_object.base_class_name == "ParentWidget",
                   "external identity REMOVEPROPERTY should preserve the immediate external base class");
            expect(native_object.class_library == library_path.string(),
                   "external identity REMOVEPROPERTY should preserve external class library provenance");
            expect(!native_object.properties.contains("class"),
                   "external identity REMOVEPROPERTY should not materialize a writable Class property shadow");
            expect(!native_object.properties.contains("baseclass"),
                   "external identity REMOVEPROPERTY should not materialize a writable BaseClass property shadow");
            expect(!native_object.properties.contains("parentclass"),
                   "external identity REMOVEPROPERTY should not materialize a writable ParentClass property shadow");
            expect(!native_object.properties.contains("classlibrary"),
                   "external identity REMOVEPROPERTY should not materialize a writable ClassLibrary property shadow");

            expect(state.ole_objects[1].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external identity REMOVEPROPERTY lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_same_prg_native_identity_metadata_cannot_be_shadowed_through_addproperty()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_identity_addproperty";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_identity_addproperty.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildWidget')\n"
            "lAddClass = ADDPROPERTY(oCreate, 'Class', 'OtherClass')\n"
            "lAddBaseClass = ADDPROPERTY(oCreate, 'BaseClass', 'OtherBase')\n"
            "lAddParentClass = ADDPROPERTY(oCreate, 'ParentClass', 'OtherParent')\n"
            "lAddClassLibrary = ADDPROPERTY(oCreate, 'ClassLibrary', 'other.prg')\n"
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
               std::string("native identity ADDPROPERTY script should complete: ") + state.message +
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

        check("laddclass", "false");
        check("laddbaseclass", "false");
        check("laddparentclass", "false");
        check("laddclasslibrary", "false");
        check("cclassafter", "ChildWidget");
        check("cbaseclassafter", "ParentWidget");
        check("cparentclassafter", "ParentWidget");

        const auto class_library_after = state.globals.find("xclasslibraryafter");
        expect(class_library_after != state.globals.end() &&
                   class_library_after->second.kind == copperfin::runtime::PrgValueKind::empty,
               "same-PRG identity ADDPROPERTY should leave ClassLibrary empty after rejected shadow creation");

        expect(state.ole_objects.size() == 1U,
               "native identity ADDPROPERTY should register one native object");
        if (state.ole_objects.size() == 1U)
        {
            const auto &native_object = state.ole_objects[0];
            expect(native_object.prog_id == "ChildWidget",
                   "native identity ADDPROPERTY should preserve child class identity");
            expect(native_object.base_class_name == "ParentWidget",
                   "native identity ADDPROPERTY should preserve the immediate base class");
            expect(native_object.class_library.empty(),
                   "native identity ADDPROPERTY should preserve empty same-PRG class library provenance");
            expect(!native_object.properties.contains("class"),
                   "native identity ADDPROPERTY should not materialize a Class shadow property");
            expect(!native_object.properties.contains("baseclass"),
                   "native identity ADDPROPERTY should not materialize a BaseClass shadow property");
            expect(!native_object.properties.contains("parentclass"),
                   "native identity ADDPROPERTY should not materialize a ParentClass shadow property");
            expect(!native_object.properties.contains("classlibrary"),
                   "native identity ADDPROPERTY should not materialize a ClassLibrary shadow property");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_external_prg_identity_metadata_cannot_be_shadowed_through_addproperty()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_prg_identity_addproperty";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path library_path = temp_root / "widgetlib.prg";
        write_text(
            library_path,
            "DEFINE CLASS ParentWidget AS Custom\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_identity_addproperty.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildWidget')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 30)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "lAddClass = ADDPROPERTY(oCreate, 'Class', 'OtherClass')\n"
            "lAddBaseClass = ADDPROPERTY(oCreate, 'BaseClass', 'OtherBase')\n"
            "lAddParentClass = ADDPROPERTY(oCreate, 'ParentClass', 'OtherParent')\n"
            "lAddClassLibrary = ADDPROPERTY(oCreate, 'ClassLibrary', 'other.prg')\n"
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
               std::string("external identity ADDPROPERTY script should complete: ") + state.message +
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

        check("laddclass", "false");
        check("laddbaseclass", "false");
        check("laddparentclass", "false");
        check("laddclasslibrary", "false");
        check("cclassafter", "ChildWidget");
        check("cbaseclassafter", "ParentWidget");
        check("cparentclassafter", "ParentWidget");
        check("cclasslibraryafter", library_path.string());
        check("ldictset", "true");
        check("ndictcompare", "30");

        expect(state.ole_objects.size() == 2U,
               "external identity ADDPROPERTY should register native and COM objects");
        if (state.ole_objects.size() == 2U)
        {
            const auto &native_object = state.ole_objects[0];
            expect(native_object.prog_id == "ChildWidget",
                   "external identity ADDPROPERTY should preserve child class identity");
            expect(native_object.base_class_name == "ParentWidget",
                   "external identity ADDPROPERTY should preserve the immediate external base class");
            expect(native_object.class_library == library_path.string(),
                   "external identity ADDPROPERTY should preserve external class library provenance");
            expect(!native_object.properties.contains("class"),
                   "external identity ADDPROPERTY should not materialize a Class shadow property");
            expect(!native_object.properties.contains("baseclass"),
                   "external identity ADDPROPERTY should not materialize a BaseClass shadow property");
            expect(!native_object.properties.contains("parentclass"),
                   "external identity ADDPROPERTY should not materialize a ParentClass shadow property");
            expect(!native_object.properties.contains("classlibrary"),
                   "external identity ADDPROPERTY should not materialize a ClassLibrary shadow property");

            expect(state.ole_objects[1].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external identity ADDPROPERTY lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_same_prg_native_default_property_stays_protected_from_addproperty_and_removeproperty()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_default_property_guards";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_default_property_guards.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildWidget')\n"
            "cRoleBefore = GETPEM(oCreate, 'cRole')\n"
            "lAddRole = ADDPROPERTY(oCreate, 'cRole', 'ShadowRole')\n"
            "cRoleAfterAdd = GETPEM(oCreate, 'cRole')\n"
            "lRemoveRole = REMOVEPROPERTY(oCreate, 'cRole')\n"
            "cRoleAfterRemove = GETPEM(oCreate, 'cRole')\n"
            "oCreate.cRole = 'AssignedRole'\n"
            "cRoleAfterAssign = GETPEM(oCreate, 'cRole')\n"
            "RETURN\n"
            "DEFINE CLASS ParentWidget AS Custom\n"
            "    cRole = 'ParentRole'\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ChildWidget AS ParentWidget\n"
            "    cRole = 'ChildRole'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native default-property guard script should complete: ") + state.message +
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

        check("crolebefore", "ChildRole");
        check("laddrole", "false");
        check("croleafteradd", "ChildRole");
        check("lremoverole", "false");
        check("croleafterremove", "ChildRole");
        check("croleafterassign", "AssignedRole");

        expect(state.ole_objects.size() == 1U,
               "native default-property guards should register one native object");
        if (state.ole_objects.size() == 1U)
        {
            const auto &native_object = state.ole_objects[0];
            expect(native_object.prog_id == "ChildWidget",
                   "native default-property guards should preserve child class identity");
            const auto property_value = native_object.properties.find("crole");
            expect(property_value != native_object.properties.end(),
                   "native default-property guards should keep the live class-defined property materialized");
            if (property_value != native_object.properties.end())
            {
                expect(copperfin::runtime::format_value(property_value->second) == "AssignedRole",
                       "native default-property guards should allow ordinary assignment after blocked helper mutations");
            }
            const auto default_value = native_object.default_properties.find("crole");
            expect(default_value != native_object.default_properties.end(),
                   "native default-property guards should snapshot class-defined defaults");
            if (default_value != native_object.default_properties.end())
            {
                expect(copperfin::runtime::format_value(default_value->second) == "ChildRole",
                       "native default-property guards should preserve the original class-defined default");
            }
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_external_prg_default_property_stays_protected_from_addproperty_and_removeproperty()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_prg_default_property_guards";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path library_path = temp_root / "widgetlib.prg";
        write_text(
            library_path,
            "DEFINE CLASS ParentWidget AS Custom\n"
            "    cRole = 'ParentRole'\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_default_property_guards.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildWidget')\n"
            "cRoleBefore = GETPEM(oCreate, 'cRole')\n"
            "lAddRole = ADDPROPERTY(oCreate, 'cRole', 'ShadowRole')\n"
            "cRoleAfterAdd = GETPEM(oCreate, 'cRole')\n"
            "lRemoveRole = REMOVEPROPERTY(oCreate, 'cRole')\n"
            "cRoleAfterRemove = GETPEM(oCreate, 'cRole')\n"
            "oCreate.cRole = 'AssignedRole'\n"
            "cRoleAfterAssign = GETPEM(oCreate, 'cRole')\n"
            "RETURN\n"
            "DEFINE CLASS ChildWidget AS ParentWidget OF widgetlib.prg\n"
            "    cRole = 'ChildRole'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external default-property guard script should complete: ") + state.message +
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

        check("crolebefore", "ChildRole");
        check("laddrole", "false");
        check("croleafteradd", "ChildRole");
        check("lremoverole", "false");
        check("croleafterremove", "ChildRole");
        check("croleafterassign", "AssignedRole");

        expect(state.ole_objects.size() == 1U,
               "external default-property guards should register one native object");
        if (state.ole_objects.size() == 1U)
        {
            const auto &native_object = state.ole_objects[0];
            expect(native_object.prog_id == "ChildWidget",
                   "external default-property guards should preserve child class identity");
            expect(native_object.class_library == library_path.string(),
                   "external default-property guards should preserve external class library provenance");
            const auto property_value = native_object.properties.find("crole");
            expect(property_value != native_object.properties.end(),
                   "external default-property guards should keep the live class-defined property materialized");
            if (property_value != native_object.properties.end())
            {
                expect(copperfin::runtime::format_value(property_value->second) == "AssignedRole",
                       "external default-property guards should allow ordinary assignment after blocked helper mutations");
            }
            const auto default_value = native_object.default_properties.find("crole");
            expect(default_value != native_object.default_properties.end(),
                   "external default-property guards should snapshot class-defined defaults");
            if (default_value != native_object.default_properties.end())
            {
                expect(copperfin::runtime::format_value(default_value->second) == "ChildRole",
                       "external default-property guards should preserve the original external class-defined default");
            }
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_same_prg_native_identity_metadata_cannot_be_shadowed_through_direct_assignment()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_identity_direct_assignment";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_identity_direct_assignment.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildWidget')\n"
            "oCreate.Class = 'OtherClass'\n"
            "oCreate.BaseClass = 'OtherBase'\n"
            "oCreate.ParentClass = 'OtherParent'\n"
            "oCreate.ClassLibrary = 'other.prg'\n"
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
               std::string("native identity direct-assignment script should complete: ") + state.message +
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

        check("cclassafter", "ChildWidget");
        check("cbaseclassafter", "ParentWidget");
        check("cparentclassafter", "ParentWidget");

        const auto class_library_after = state.globals.find("xclasslibraryafter");
        expect(class_library_after != state.globals.end() &&
                   class_library_after->second.kind == copperfin::runtime::PrgValueKind::empty,
               "same-PRG identity direct assignment should leave ClassLibrary empty");

        expect(state.ole_objects.size() == 1U,
               "native identity direct assignment should register one native object");
        if (state.ole_objects.size() == 1U)
        {
            const auto &native_object = state.ole_objects[0];
            expect(native_object.prog_id == "ChildWidget",
                   "native identity direct assignment should preserve child class identity");
            expect(native_object.base_class_name == "ParentWidget",
                   "native identity direct assignment should preserve the immediate base class");
            expect(native_object.class_library.empty(),
                   "native identity direct assignment should preserve empty same-PRG class library provenance");
            expect(!native_object.properties.contains("class"),
                   "native identity direct assignment should not materialize a Class shadow property");
            expect(!native_object.properties.contains("baseclass"),
                   "native identity direct assignment should not materialize a BaseClass shadow property");
            expect(!native_object.properties.contains("parentclass"),
                   "native identity direct assignment should not materialize a ParentClass shadow property");
            expect(!native_object.properties.contains("classlibrary"),
                   "native identity direct assignment should not materialize a ClassLibrary shadow property");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_external_prg_identity_metadata_cannot_be_shadowed_through_direct_assignment()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_prg_identity_direct_assignment";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path library_path = temp_root / "widgetlib.prg";
        write_text(
            library_path,
            "DEFINE CLASS ParentWidget AS Custom\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_identity_direct_assignment.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildWidget')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 31)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "oCreate.Class = 'OtherClass'\n"
            "oCreate.BaseClass = 'OtherBase'\n"
            "oCreate.ParentClass = 'OtherParent'\n"
            "oCreate.ClassLibrary = 'other.prg'\n"
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
               std::string("external identity direct-assignment script should complete: ") + state.message +
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

        check("cclassafter", "ChildWidget");
        check("cbaseclassafter", "ParentWidget");
        check("cparentclassafter", "ParentWidget");
        check("cclasslibraryafter", library_path.string());
        check("ldictset", "true");
        check("ndictcompare", "31");

        expect(state.ole_objects.size() == 2U,
               "external identity direct assignment should register native and COM objects");
        if (state.ole_objects.size() == 2U)
        {
            const auto &native_object = state.ole_objects[0];
            expect(native_object.prog_id == "ChildWidget",
                   "external identity direct assignment should preserve child class identity");
            expect(native_object.base_class_name == "ParentWidget",
                   "external identity direct assignment should preserve the immediate external base class");
            expect(native_object.class_library == library_path.string(),
                   "external identity direct assignment should preserve external class library provenance");
            expect(!native_object.properties.contains("class"),
                   "external identity direct assignment should not materialize a Class shadow property");
            expect(!native_object.properties.contains("baseclass"),
                   "external identity direct assignment should not materialize a BaseClass shadow property");
            expect(!native_object.properties.contains("parentclass"),
                   "external identity direct assignment should not materialize a ParentClass shadow property");
            expect(!native_object.properties.contains("classlibrary"),
                   "external identity direct assignment should not materialize a ClassLibrary shadow property");

            expect(state.ole_objects[1].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external identity direct assignment lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_same_prg_native_identity_metadata_reads_through_ordinary_properties()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_identity_property_reads";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_identity_property_reads.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildWidget')\n"
            "cClass = oCreate.Class\n"
            "cBaseClass = oCreate.BaseClass\n"
            "RETURN\n"
            "DEFINE CLASS ParentWidget AS Custom\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ChildWidget AS ParentWidget\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native identity property-read script should complete: ") + state.message +
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

        check("cclass", "ChildWidget");
        check("cbaseclass", "ParentWidget");

        expect(state.ole_objects.size() == 1U,
               "native identity property reads should register one native object");
        if (state.ole_objects.size() == 1U)
        {
            expect(state.ole_objects[0].prog_id == "ChildWidget",
                   "native identity property reads should preserve child class identity");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_external_prg_identity_metadata_reads_through_ordinary_properties()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_prg_identity_property_reads";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path library_path = temp_root / "widgetlib.prg";
        write_text(
            library_path,
            "DEFINE CLASS ParentWidget AS Custom\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_identity_property_reads.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildWidget')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 32)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "cClass = oCreate.Class\n"
            "cBaseClass = oCreate.BaseClass\n"
            "cClassLibrary = oCreate.ClassLibrary\n"
            "RETURN\n"
            "DEFINE CLASS ChildWidget AS ParentWidget OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external identity property-read script should complete: ") + state.message +
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

        check("cclass", "ChildWidget");
        check("cbaseclass", "ParentWidget");
        check("cclasslibrary", library_path.string());
        check("ldictset", "true");
        check("ndictcompare", "32");

        expect(state.ole_objects.size() == 2U,
               "external identity property reads should register native and COM objects");
        if (state.ole_objects.size() == 2U)
        {
            expect(state.ole_objects[0].prog_id == "ChildWidget",
                   "external identity property reads should preserve child class identity");
            expect(state.ole_objects[1].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external identity property reads land");
        }

        fs::remove_all(temp_root, ignored);
    }

}
