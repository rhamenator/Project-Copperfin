#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_same_prg_native_class_inheritance_applies_parent_defaults_methods_and_init()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_class_inheritance";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_class_inheritance.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildWidget')\n"
            "oNew = NEWOBJECT('LeafWidget')\n"
            "oPlain = CREATEOBJECT('Empty')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "oPlain.Extra = 'plain'\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 6)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "cCreateDescribe = oCreate.Describe()\n"
            "cCreateWho = oCreate.Who()\n"
            "cNewDescribe = oNew.Describe()\n"
            "cNewWho = oNew.Who()\n"
            "cPlain = oPlain.Extra\n"
            "nCreateBase = oCreate.nBase\n"
            "nCreateChild = oCreate.nChild\n"
            "lCreateInitRan = oCreate.lInitRan\n"
            "nNewBase = oNew.nBase\n"
            "nNewChild = oNew.nChild\n"
            "lNewInitRan = oNew.lInitRan\n"
            "RETURN\n"
            "DEFINE CLASS ParentWidget AS Custom\n"
            "    Caption = 'Parent'\n"
            "    nBase = 3\n"
            "    lInitRan = .F.\n"
            "    PROCEDURE Init\n"
            "        THIS.Caption = THIS.Caption + '-Init'\n"
            "        THIS.lInitRan = .T.\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "    FUNCTION Describe\n"
            "        RETURN THIS.Caption\n"
            "    ENDFUNC\n"
            "    FUNCTION Who\n"
            "        RETURN 'Parent'\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ChildWidget AS ParentWidget\n"
            "    Caption = 'Child'\n"
            "    nChild = 7\n"
            "    FUNCTION Who\n"
            "        RETURN 'Child'\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS LeafWidget AS ChildWidget\n"
            "    nBase = 11\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native class inheritance script should complete: ") + state.message +
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

        check("ccreatedescribe", "Child-Init");
        check("ccreatewho", "Child");
        check("cnewdescribe", "Child-Init");
        check("cnewwho", "Child");
        check("cplain", "plain");
        check("ncreatebase", "3");
        check("ncreatechild", "7");
        check("lcreateinitran", "true");
        check("nnewbase", "11");
        check("nnewchild", "7");
        check("lnewinitran", "true");
        check("ldictset", "true");
        check("ndictcompare", "6");

        expect(state.ole_objects.size() == 4U,
               "native class inheritance script should register CREATEOBJECT, NEWOBJECT, plain, and COM objects");
        if (state.ole_objects.size() == 4U)
        {
            const auto &create_object = state.ole_objects[0];
            expect(create_object.prog_id == "ChildWidget",
                   "native class inheritance should preserve child class identity");
            const auto create_caption = create_object.properties.find("caption");
            const auto create_base = create_object.properties.find("nbase");
            const auto create_child = create_object.properties.find("nchild");
            const auto create_init = create_object.properties.find("linitran");
            expect(create_caption != create_object.properties.end(),
                   "native class inheritance should materialize inherited Init-updated caption state");
            expect(create_base != create_object.properties.end(),
                   "native class inheritance should materialize inherited parent properties");
            expect(create_child != create_object.properties.end(),
                   "native class inheritance should keep child-local properties");
            expect(create_init != create_object.properties.end(),
                   "native class inheritance should materialize inherited Init-written flags");
            if (create_caption != create_object.properties.end())
            {
                expect(copperfin::runtime::format_value(create_caption->second) == "Child-Init",
                       "native class inheritance should let inherited Init see child-overridden properties");
            }
            if (create_base != create_object.properties.end())
            {
                expect(copperfin::runtime::format_value(create_base->second) == "3",
                       "native class inheritance should keep parent default properties on child instances");
            }
            if (create_child != create_object.properties.end())
            {
                expect(copperfin::runtime::format_value(create_child->second) == "7",
                       "native class inheritance should keep child-local default properties");
            }
            if (create_init != create_object.properties.end())
            {
                expect(copperfin::runtime::format_value(create_init->second) == "true",
                       "native class inheritance should run inherited Init when the child does not override it");
            }
            expect(std::find(create_object.methods.begin(), create_object.methods.end(), "Describe") != create_object.methods.end(),
                   "native class inheritance should expose inherited methods on the runtime object");
            expect(std::find(create_object.methods.begin(), create_object.methods.end(), "Who") != create_object.methods.end(),
                   "native class inheritance should retain child overrides in the runtime method list");
            expect(std::find(create_object.methods.begin(), create_object.methods.end(), "Init") != create_object.methods.end(),
                   "native class inheritance should expose inherited Init in runtime member enumeration");

            const auto &new_object = state.ole_objects[1];
            expect(new_object.prog_id == "LeafWidget",
                   "multilevel native class inheritance should preserve leaf class identity");
            const auto new_base = new_object.properties.find("nbase");
            const auto new_child = new_object.properties.find("nchild");
            if (new_base != new_object.properties.end())
            {
                expect(copperfin::runtime::format_value(new_base->second) == "11",
                       "multilevel native class inheritance should let leaf defaults override inherited parent values");
            }
            else
            {
                expect(false, "multilevel native class inheritance should materialize leaf override properties");
            }
            if (new_child != new_object.properties.end())
            {
                expect(copperfin::runtime::format_value(new_child->second) == "7",
                       "multilevel native class inheritance should retain intermediate inherited properties");
            }
            else
            {
                expect(false, "multilevel native class inheritance should materialize intermediate inherited properties");
            }

            expect(state.ole_objects[2].prog_id == "Empty",
                   "plain CREATEOBJECT should remain stable while native inheritance lands");
            expect(state.ole_objects[3].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native inheritance lands");
        }

        const bool has_inherited_init_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.init" &&
                   event.detail == "ParentWidget.Init";
        });
        expect(has_inherited_init_event,
               "native class inheritance should emit the inherited Init event when the parent Init runs");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_class_inheritance_loads_external_prg_base_sources()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_external_base_inheritance";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path library_path = temp_root / "widgetlib.prg";
        write_text(
            library_path,
            "DEFINE CLASS ParentWidget AS Custom\n"
            "    Caption = 'Parent'\n"
            "    nBase = 4\n"
            "    lInitRan = .F.\n"
            "    PROCEDURE Init\n"
            "        THIS.Caption = THIS.Caption + '-Init'\n"
            "        THIS.lInitRan = .T.\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "    FUNCTION Describe\n"
            "        RETURN THIS.Caption\n"
            "    ENDFUNC\n"
            "    FUNCTION Who\n"
            "        RETURN 'Parent'\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "native_external_base_inheritance.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildWidget')\n"
            "oNew = NEWOBJECT('LeafWidget')\n"
            "oPlain = CREATEOBJECT('Empty')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "oPlain.Extra = 'plain'\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 16)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "cCreateDescribe = oCreate.Describe()\n"
            "cCreateWho = oCreate.Who()\n"
            "cNewDescribe = oNew.Describe()\n"
            "cNewWho = oNew.Who()\n"
            "cPlain = oPlain.Extra\n"
            "nCreateBase = oCreate.nBase\n"
            "nCreateChild = oCreate.nChild\n"
            "lCreateInitRan = oCreate.lInitRan\n"
            "nNewBase = oNew.nBase\n"
            "nNewChild = oNew.nChild\n"
            "lNewInitRan = oNew.lInitRan\n"
            "RETURN\n"
            "DEFINE CLASS ChildWidget AS ParentWidget OF widgetlib.prg\n"
            "    Caption = 'Child'\n"
            "    nChild = 9\n"
            "    FUNCTION Who\n"
            "        RETURN 'Child'\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS LeafWidget AS ChildWidget\n"
            "    nBase = 12\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external base class inheritance script should complete: ") + state.message +
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

        check("ccreatedescribe", "Child-Init");
        check("ccreatewho", "Child");
        check("cnewdescribe", "Child-Init");
        check("cnewwho", "Child");
        check("cplain", "plain");
        check("ncreatebase", "4");
        check("ncreatechild", "9");
        check("lcreateinitran", "true");
        check("nnewbase", "12");
        check("nnewchild", "9");
        check("lnewinitran", "true");
        check("ldictset", "true");
        check("ndictcompare", "16");

        expect(state.ole_objects.size() == 4U,
               "external base class inheritance should register CREATEOBJECT, NEWOBJECT, plain, and COM objects");
        if (state.ole_objects.size() == 4U)
        {
            const auto &create_object = state.ole_objects[0];
            expect(create_object.prog_id == "ChildWidget",
                   "external base class inheritance should preserve child class identity");
            expect(create_object.source == main_path.string(),
                   "external base class inheritance should preserve the child class source as object provenance");

            const auto create_caption = create_object.properties.find("caption");
            const auto create_base = create_object.properties.find("nbase");
            const auto create_child = create_object.properties.find("nchild");
            const auto create_init = create_object.properties.find("linitran");
            expect(create_caption != create_object.properties.end(),
                   "external base class inheritance should materialize inherited Init-updated caption state");
            expect(create_base != create_object.properties.end(),
                   "external base class inheritance should materialize parent defaults from the external PRG library");
            expect(create_child != create_object.properties.end(),
                   "external base class inheritance should keep child-local properties");
            expect(create_init != create_object.properties.end(),
                   "external base class inheritance should materialize inherited Init flags from the external PRG library");
            if (create_caption != create_object.properties.end())
            {
                expect(copperfin::runtime::format_value(create_caption->second) == "Child-Init",
                       "external base class inheritance should let inherited external Init see child-overridden properties");
            }
            if (create_base != create_object.properties.end())
            {
                expect(copperfin::runtime::format_value(create_base->second) == "4",
                       "external base class inheritance should keep parent defaults from the external PRG library");
            }
            if (create_child != create_object.properties.end())
            {
                expect(copperfin::runtime::format_value(create_child->second) == "9",
                       "external base class inheritance should keep child-local default properties");
            }
            if (create_init != create_object.properties.end())
            {
                expect(copperfin::runtime::format_value(create_init->second) == "true",
                       "external base class inheritance should run inherited external Init when the child does not override it");
            }
            expect(std::find(create_object.methods.begin(), create_object.methods.end(), "Describe") != create_object.methods.end(),
                   "external base class inheritance should expose inherited external methods on the runtime object");
            expect(std::find(create_object.methods.begin(), create_object.methods.end(), "Who") != create_object.methods.end(),
                   "external base class inheritance should retain child overrides in the runtime method list");
            expect(std::find(create_object.methods.begin(), create_object.methods.end(), "Init") != create_object.methods.end(),
                   "external base class inheritance should expose inherited external Init in runtime member enumeration");

            const auto &new_object = state.ole_objects[1];
            expect(new_object.prog_id == "LeafWidget",
                   "external base class inheritance should preserve leaf class identity");
            const auto new_base = new_object.properties.find("nbase");
            const auto new_child = new_object.properties.find("nchild");
            if (new_base != new_object.properties.end())
            {
                expect(copperfin::runtime::format_value(new_base->second) == "12",
                       "external base class inheritance should let leaf defaults override external parent values");
            }
            else
            {
                expect(false, "external base class inheritance should materialize leaf override properties");
            }
            if (new_child != new_object.properties.end())
            {
                expect(copperfin::runtime::format_value(new_child->second) == "9",
                       "external base class inheritance should retain inherited child properties above the external base");
            }
            else
            {
                expect(false, "external base class inheritance should materialize inherited child properties");
            }

            expect(state.ole_objects[2].prog_id == "Empty",
                   "plain CREATEOBJECT should remain stable while external base inheritance lands");
            expect(state.ole_objects[3].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external base inheritance lands");
        }

        const bool has_external_inherited_init_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.init" &&
                   event.detail == "ParentWidget.Init";
        });
        expect(has_external_inherited_init_event,
               "external base class inheritance should emit the inherited external Init event when the parent Init runs");

        fs::remove_all(temp_root, ignored);
    }

    void test_same_prg_native_aclass_reflects_inheritance_chain()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_aclass_inheritance";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_aclass_inheritance.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildWidget')\n"
            "oPlain = CREATEOBJECT('Empty')\n"
            "nClassCount = ACLASS(aClass, oCreate)\n"
            "cClass1 = aClass[1]\n"
            "cClass2 = aClass[2]\n"
            "cClass3 = aClass[3]\n"
            "cClass4 = aClass[4]\n"
            "cClass5 = aClass[5]\n"
            "RETURN\n"
            "DEFINE CLASS RootWidget AS Custom\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ParentWidget AS RootWidget\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ChildWidget AS ParentWidget\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native ACLASS inheritance script should complete: ") + state.message +
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
        check("cclass1", "CHILDWIDGET");
        check("cclass2", "PARENTWIDGET");
        check("cclass3", "ROOTWIDGET");
        check("cclass4", "CUSTOM");
        check("cclass5", "OBJECT");

        expect(state.ole_objects.size() == 2U,
               "native ACLASS inheritance should register native and plain objects");
        if (state.ole_objects.size() == 2U)
        {
            const auto &native_object = state.ole_objects[0];
            expect(native_object.prog_id == "ChildWidget",
                   "native ACLASS inheritance should preserve child class identity");
            expect(native_object.class_hierarchy.size() == 5U,
                   "native ACLASS inheritance should persist the native class hierarchy on runtime objects");
            if (native_object.class_hierarchy.size() == 5U)
            {
                expect(native_object.class_hierarchy[0] == "CHILDWIDGET",
                       "native ACLASS inheritance should store the derived class first");
                expect(native_object.class_hierarchy[1] == "PARENTWIDGET",
                       "native ACLASS inheritance should store the immediate parent second");
                expect(native_object.class_hierarchy[2] == "ROOTWIDGET",
                       "native ACLASS inheritance should store deeper native ancestors");
                expect(native_object.class_hierarchy[3] == "CUSTOM",
                       "native ACLASS inheritance should preserve the builtin base token");
                expect(native_object.class_hierarchy[4] == "OBJECT",
                       "native ACLASS inheritance should preserve the terminal object token");
            }

            expect(state.ole_objects[1].prog_id == "Empty",
                   "plain CREATEOBJECT should remain stable while native ACLASS inheritance lands");
            expect(state.ole_objects[1].class_hierarchy.empty(),
                   "plain CREATEOBJECT should keep the native-only class hierarchy empty");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_external_prg_base_aclass_reflects_inheritance_chain()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_prg_aclass_inheritance";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path library_path = temp_root / "widgetlib.prg";
        write_text(
            library_path,
            "DEFINE CLASS RootWidget AS Custom\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ParentWidget AS RootWidget\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_aclass_inheritance.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildWidget')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 24)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "nClassCount = ACLASS(aClass, oCreate)\n"
            "cClass1 = aClass[1]\n"
            "cClass2 = aClass[2]\n"
            "cClass3 = aClass[3]\n"
            "cClass4 = aClass[4]\n"
            "cClass5 = aClass[5]\n"
            "RETURN\n"
            "DEFINE CLASS ChildWidget AS ParentWidget OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external-base ACLASS inheritance script should complete: ") + state.message +
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
        check("cclass1", "CHILDWIDGET");
        check("cclass2", "PARENTWIDGET");
        check("cclass3", "ROOTWIDGET");
        check("cclass4", "CUSTOM");
        check("cclass5", "OBJECT");
        check("ldictset", "true");
        check("ndictcompare", "24");

        expect(state.ole_objects.size() == 2U,
               "external-base ACLASS inheritance should register native and COM objects");
        if (state.ole_objects.size() == 2U)
        {
            const auto &native_object = state.ole_objects[0];
            expect(native_object.prog_id == "ChildWidget",
                   "external-base ACLASS inheritance should preserve child class identity");
            expect(native_object.class_hierarchy.size() == 5U,
                   "external-base ACLASS inheritance should persist the external-base class hierarchy");
            if (native_object.class_hierarchy.size() == 5U)
            {
                expect(native_object.class_hierarchy[0] == "CHILDWIDGET",
                       "external-base ACLASS inheritance should store the derived class first");
                expect(native_object.class_hierarchy[1] == "PARENTWIDGET",
                       "external-base ACLASS inheritance should store the inherited external parent second");
                expect(native_object.class_hierarchy[2] == "ROOTWIDGET",
                       "external-base ACLASS inheritance should store deeper external native ancestors");
                expect(native_object.class_hierarchy[3] == "CUSTOM",
                       "external-base ACLASS inheritance should preserve the builtin base token");
                expect(native_object.class_hierarchy[4] == "OBJECT",
                       "external-base ACLASS inheritance should preserve the terminal object token");
            }

            expect(state.ole_objects[1].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external-base ACLASS inheritance lands");
            expect(state.ole_objects[1].class_hierarchy.empty(),
                   "COM NEWOBJECT should keep the native-only class hierarchy empty");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_same_prg_native_baseclass_reflects_through_getpem_and_pemstatus()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_baseclass_reflection";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_baseclass_reflection.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildWidget')\n"
            "oPlain = CREATEOBJECT('Empty')\n"
            "cBaseClass = GETPEM(oCreate, 'BaseClass')\n"
            "lHasBaseClass = PEMSTATUS(oCreate, 'BaseClass', 1)\n"
            "lBaseClassReadOnly = PEMSTATUS(oCreate, 'BaseClass', 5)\n"
            "xClassLibrary = GETPEM(oCreate, 'ClassLibrary')\n"
            "lHasClassLibrary = PEMSTATUS(oCreate, 'ClassLibrary', 1)\n"
            "RETURN\n"
            "DEFINE CLASS RootWidget AS Custom\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ChildWidget AS RootWidget\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native BaseClass reflection script should complete: ") + state.message +
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

        check("cbaseclass", "RootWidget");
        check("lhasbaseclass", "true");
        check("lbaseclassreadonly", "true");
        check("lhasclasslibrary", "false");

        const auto class_library = state.globals.find("xclasslibrary");
        expect(class_library != state.globals.end() &&
                   class_library->second.kind == copperfin::runtime::PrgValueKind::empty,
               "same-PRG BaseClass reflection should leave ClassLibrary empty");

        expect(state.ole_objects.size() == 2U,
               "native BaseClass reflection should register native and plain objects");
        if (state.ole_objects.size() == 2U)
        {
            const auto &native_object = state.ole_objects[0];
            expect(native_object.prog_id == "ChildWidget",
                   "native BaseClass reflection should preserve child class identity");
            expect(native_object.base_class_name == "RootWidget",
                   "native BaseClass reflection should persist the immediate native base class name");
            expect(native_object.class_library.empty(),
                   "native BaseClass reflection should keep same-PRG class library provenance empty");

            expect(state.ole_objects[1].prog_id == "Empty",
                   "plain CREATEOBJECT should remain stable while native BaseClass reflection lands");
            expect(state.ole_objects[1].base_class_name.empty(),
                   "plain CREATEOBJECT should not fabricate native base-class metadata");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_external_prg_base_identity_reflects_through_getpem_and_pemstatus()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_prg_base_identity_reflection";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path library_path = temp_root / "widgetlib.prg";
        write_text(
            library_path,
            "DEFINE CLASS ParentWidget AS Custom\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_base_identity_reflection.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildWidget')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 25)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "cBaseClass = GETPEM(oCreate, 'BaseClass')\n"
            "cClassLibrary = GETPEM(oCreate, 'ClassLibrary')\n"
            "lHasBaseClass = PEMSTATUS(oCreate, 'BaseClass', 1)\n"
            "lHasClassLibrary = PEMSTATUS(oCreate, 'ClassLibrary', 1)\n"
            "lClassLibraryReadOnly = PEMSTATUS(oCreate, 'ClassLibrary', 5)\n"
            "RETURN\n"
            "DEFINE CLASS ChildWidget AS ParentWidget OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external-base identity reflection script should complete: ") + state.message +
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

        check("cbaseclass", "ParentWidget");
        check("cclasslibrary", library_path.string());
        check("lhasbaseclass", "true");
        check("lhasclasslibrary", "true");
        check("lclasslibraryreadonly", "true");
        check("ldictset", "true");
        check("ndictcompare", "25");

        expect(state.ole_objects.size() == 2U,
               "external-base identity reflection should register native and COM objects");
        if (state.ole_objects.size() == 2U)
        {
            const auto &native_object = state.ole_objects[0];
            expect(native_object.prog_id == "ChildWidget",
                   "external-base identity reflection should preserve child class identity");
            expect(native_object.base_class_name == "ParentWidget",
                   "external-base identity reflection should persist the immediate external base class name");
            expect(native_object.class_library == library_path.string(),
                   "external-base identity reflection should persist the resolved external class library path");

            expect(state.ole_objects[1].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external-base identity reflection lands");
            expect(state.ole_objects[1].base_class_name.empty(),
                   "COM NEWOBJECT should not fabricate native base-class metadata");
            expect(state.ole_objects[1].class_library.empty(),
                   "COM NEWOBJECT should not fabricate native class-library metadata");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_same_prg_native_class_reflects_through_getpem_and_pemstatus()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_class_reflection";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_class_reflection.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildWidget')\n"
            "oPlain = CREATEOBJECT('Empty')\n"
            "cClass = GETPEM(oCreate, 'Class')\n"
            "lHasClass = PEMSTATUS(oCreate, 'Class', 1)\n"
            "lClassReadOnly = PEMSTATUS(oCreate, 'Class', 5)\n"
            "xPlainClass = GETPEM(oPlain, 'Class')\n"
            "lPlainHasClass = PEMSTATUS(oPlain, 'Class', 1)\n"
            "RETURN\n"
            "DEFINE CLASS ParentWidget AS Custom\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ChildWidget AS ParentWidget\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Class reflection script should complete: ") + state.message +
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
        check("lhasclass", "true");
        check("lclassreadonly", "true");
        check("lplainhasclass", "false");

        const auto plain_class = state.globals.find("xplainclass");
        expect(plain_class != state.globals.end() &&
                   plain_class->second.kind == copperfin::runtime::PrgValueKind::empty,
               "plain CREATEOBJECT should keep native Class reflection empty");

        expect(state.ole_objects.size() == 2U,
               "native Class reflection should register native and plain objects");
        if (state.ole_objects.size() == 2U)
        {
            expect(state.ole_objects[0].prog_id == "ChildWidget",
                   "native Class reflection should preserve child class identity");
            expect(state.ole_objects[0].class_hierarchy.size() == 4U,
                   "native Class reflection should keep native hierarchy metadata intact");
            expect(state.ole_objects[1].prog_id == "Empty",
                   "plain CREATEOBJECT should remain stable while native Class reflection lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_external_prg_base_class_reflects_through_getpem_and_pemstatus()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_prg_class_reflection";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path library_path = temp_root / "widgetlib.prg";
        write_text(
            library_path,
            "DEFINE CLASS ParentWidget AS Custom\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_class_reflection.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildWidget')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 26)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "cClass = GETPEM(oCreate, 'Class')\n"
            "lHasClass = PEMSTATUS(oCreate, 'Class', 1)\n"
            "lClassReadOnly = PEMSTATUS(oCreate, 'Class', 5)\n"
            "RETURN\n"
            "DEFINE CLASS ChildWidget AS ParentWidget OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external-base Class reflection script should complete: ") + state.message +
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
        check("lhasclass", "true");
        check("lclassreadonly", "true");
        check("ldictset", "true");
        check("ndictcompare", "26");

        expect(state.ole_objects.size() == 2U,
               "external-base Class reflection should register native and COM objects");
        if (state.ole_objects.size() == 2U)
        {
            expect(state.ole_objects[0].prog_id == "ChildWidget",
                   "external-base Class reflection should preserve child class identity");
            expect(state.ole_objects[0].class_library == library_path.string(),
                   "external-base Class reflection should preserve external library provenance");
            expect(state.ole_objects[1].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external-base Class reflection lands");
        }

        fs::remove_all(temp_root, ignored);
    }

}
