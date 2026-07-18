#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_inherited_external_base_addobject_deeper_external_child_identity_metadata_cannot_be_shadowed_through_direct_assignment()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_base_inherited_addobject_deeper_external_child_identity_direct_assignment";
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

        const fs::path main_path = temp_root / "external_base_inherited_addobject_deeper_external_child_identity_direct_assignment.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oChild = oCreate.cmdSave\n"
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
            "lDictSet = SETPEM(oDict, 'comparemode', 79)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external-base inherited ADDOBJECT deeper external child identity direct-assignment script should complete: ") + state.message +
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
        check("ndictcompare", "79");

        const auto child_class_library_after = state.globals.find("xchildclasslibraryafter");
        expect(child_class_library_after != state.globals.end() &&
                   child_class_library_after->second.kind == copperfin::runtime::PrgValueKind::empty,
               "external-base inherited ADDOBJECT deeper external child identity direct assignment should keep the derived child ClassLibrary empty");

        expect(state.ole_objects.size() == 3U,
               "external-base inherited ADDOBJECT deeper external child identity direct assignment should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &parent_object = state.ole_objects[0];
            const auto &child_object = state.ole_objects[1];
            expect(parent_object.prog_id == "ChildForm",
                   "external-base inherited ADDOBJECT deeper external child identity direct assignment should preserve parent identity");
            expect(child_object.prog_id == "SaveButton",
                   "external-base inherited ADDOBJECT deeper external child identity direct assignment should preserve child identity");
            expect(child_object.base_class_name == "ParentButton",
                   "external-base inherited ADDOBJECT deeper external child identity direct assignment should preserve the immediate child base-class identity");
            expect(child_object.source == button_library_path.string(),
                   "external-base inherited ADDOBJECT deeper external child identity direct assignment should preserve the child definition source path");
            expect(child_object.class_library.empty(),
                   "external-base inherited ADDOBJECT deeper external child identity direct assignment should keep the derived child ClassLibrary empty");
            expect(child_object.class_hierarchy.size() == 5U,
                   "external-base inherited ADDOBJECT deeper external child identity direct assignment should preserve the deeper runtime child class hierarchy");
            expect(!child_object.properties.contains("class"),
                   "external-base inherited ADDOBJECT deeper external child identity direct assignment should not materialize a child Class shadow property");
            expect(!child_object.properties.contains("baseclass"),
                   "external-base inherited ADDOBJECT deeper external child identity direct assignment should not materialize a child BaseClass shadow property");
            expect(!child_object.properties.contains("parentclass"),
                   "external-base inherited ADDOBJECT deeper external child identity direct assignment should not materialize a child ParentClass shadow property");
            expect(!child_object.properties.contains("classlibrary"),
                   "external-base inherited ADDOBJECT deeper external child identity direct assignment should not materialize a child ClassLibrary shadow property");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external-base inherited ADDOBJECT deeper external child identity direct assignment lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_native_addobject_deeper_external_child_external_base_provenance_stays_coherent()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_addobject_deeper_external_child_external_base_provenance";
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

        const fs::path main_path = temp_root / "native_addobject_deeper_external_child_external_base_provenance.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('DemoForm')\n"
            "oChild = oForm.cmdSave\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 80)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "cChildClass = oChild.Class\n"
            "cChildBaseClass = oChild.BaseClass\n"
            "cChildParentClass = oChild.ParentClass\n"
            "cChildClassLibrary = oChild.ClassLibrary\n"
            "cChildClassReflect = GETPEM(oChild, 'Class')\n"
            "cChildBaseClassReflect = GETPEM(oChild, 'BaseClass')\n"
            "cChildParentClassReflect = GETPEM(oChild, 'ParentClass')\n"
            "cChildClassLibraryReflect = GETPEM(oChild, 'ClassLibrary')\n"
            "lChildHasClass = PEMSTATUS(oChild, 'Class', 1)\n"
            "lChildHasBaseClass = PEMSTATUS(oChild, 'BaseClass', 1)\n"
            "lChildHasParentClass = PEMSTATUS(oChild, 'ParentClass', 1)\n"
            "lChildHasClassLibrary = PEMSTATUS(oChild, 'ClassLibrary', 1)\n"
            "lChildClassReadOnly = PEMSTATUS(oChild, 'Class', 5)\n"
            "lChildBaseClassReadOnly = PEMSTATUS(oChild, 'BaseClass', 5)\n"
            "lChildParentClassReadOnly = PEMSTATUS(oChild, 'ParentClass', 5)\n"
            "lChildClassLibraryReadOnly = PEMSTATUS(oChild, 'ClassLibrary', 5)\n"
            "nMembersProps = AMEMBERS(aMembersProps, oChild, 1)\n"
            "nMembersUnion = AMEMBERS(aMembersUnion, oChild, 3)\n"
            "cProp1 = aMembersProps[1]\n"
            "cProp2 = aMembersProps[2]\n"
            "cProp3 = aMembersProps[3]\n"
            "cProp4 = aMembersProps[4]\n"
            "cProp5 = aMembersProps[5]\n"
            "cProp6 = aMembersProps[6]\n"
            "cUnion4 = aMembersUnion[4]\n"
            "nClassCount = ACLASS(aClass, oChild)\n"
            "cClass1 = aClass[1]\n"
            "cClass2 = aClass[2]\n"
            "cClass3 = aClass[3]\n"
            "cClass4 = aClass[4]\n"
            "cClass5 = aClass[5]\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS ParentButton OF buttons.prg\n"
            "    Caption = 'Save'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native ADDOBJECT deeper external child external-base provenance script should complete: ") + state.message +
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
        check("cchildclassreflect", "SaveButton");
        check("cchildbaseclassreflect", "ParentButton");
        check("cchildparentclassreflect", "ParentButton");
        check("cchildclasslibraryreflect", button_library_path.string());
        check("lchildhasclass", "true");
        check("lchildhasbaseclass", "true");
        check("lchildhasparentclass", "true");
        check("lchildhasclasslibrary", "true");
        check("lchildclassreadonly", "true");
        check("lchildbaseclassreadonly", "true");
        check("lchildparentclassreadonly", "true");
        check("lchildclasslibraryreadonly", "true");
        check("nmembersprops", "6");
        check("nmembersunion", "13");
        check("cprop1", "BASECLASS");
        check("cprop2", "CAPTION");
        check("cprop3", "CLASS");
        check("cprop4", "CLASSLIBRARY");
        check("cprop5", "PARENT");
        check("cprop6", "PARENTCLASS");
        check("cunion4", "CLASSLIBRARY");
        check("nclasscount", "5");
        check("cclass1", "SAVEBUTTON");
        check("cclass2", "PARENTBUTTON");
        check("cclass3", "ROOTBUTTON");
        check("cclass4", "CUSTOM");
        check("cclass5", "OBJECT");
        check("ldictset", "true");
        check("ndictcompare", "80");

        expect(state.ole_objects.size() == 3U,
               "native ADDOBJECT deeper external child external-base provenance should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &parent_object = state.ole_objects[0];
            const auto &child_object = state.ole_objects[1];
            expect(parent_object.prog_id == "DemoForm",
                   "native ADDOBJECT deeper external child external-base provenance should preserve parent identity");
            expect(child_object.prog_id == "SaveButton",
                   "native ADDOBJECT deeper external child external-base provenance should preserve child identity");
            expect(child_object.base_class_name == "ParentButton",
                   "native ADDOBJECT deeper external child external-base provenance should preserve immediate child base-class identity");
            expect(child_object.source == main_path.string(),
                   "native ADDOBJECT deeper external child external-base provenance should preserve the defining child-class source path");
            expect(child_object.class_library == button_library_path.string(),
                   "native ADDOBJECT deeper external child external-base provenance should preserve the immediate external ClassLibrary path");
            expect(child_object.class_hierarchy.size() == 5U,
                   "native ADDOBJECT deeper external child external-base provenance should preserve the deeper runtime child class hierarchy");
            expect(!child_object.properties.contains("classlibrary"),
                   "native ADDOBJECT deeper external child external-base provenance should not materialize a child ClassLibrary shadow");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native ADDOBJECT deeper external child external-base provenance lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_native_declarative_deeper_external_child_external_base_provenance_stays_coherent()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_declarative_deeper_external_child_external_base_provenance";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path root_library_path = temp_root / "rootbuttons.prg";
        write_text(
            root_library_path,
            "DEFINE CLASS RootButton AS Custom\n"
            "    FUNCTION RootToken\n"
            "        RETURN 'RootToken'\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        const fs::path button_library_path = temp_root / "buttons.prg";
        write_text(
            button_library_path,
            "DEFINE CLASS ParentButton AS RootButton OF rootbuttons.prg\n"
            "    FUNCTION OwnerCaption\n"
            "        RETURN PARENT.Caption\n"
            "    ENDFUNC\n"
            "    FUNCTION TriggerSave\n"
            "        RETURN THISFORM.Save()\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "native_declarative_deeper_external_child_external_base_provenance.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('DemoForm')\n"
            "oChild = oForm.cmdSave\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 155)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "cInitChildCaption = oForm.cInitChildCaption\n"
            "cInitOwnerCaption = oForm.cInitOwnerCaption\n"
            "cChildClass = oChild.Class\n"
            "cChildBaseClass = oChild.BaseClass\n"
            "cChildParentClass = oChild.ParentClass\n"
            "cChildClassLibrary = oChild.ClassLibrary\n"
            "cChildClassReflect = GETPEM(oChild, 'Class')\n"
            "cChildBaseClassReflect = GETPEM(oChild, 'BaseClass')\n"
            "cChildParentClassReflect = GETPEM(oChild, 'ParentClass')\n"
            "cChildClassLibraryReflect = GETPEM(oChild, 'ClassLibrary')\n"
            "lChildHasClass = PEMSTATUS(oChild, 'Class', 1)\n"
            "lChildHasBaseClass = PEMSTATUS(oChild, 'BaseClass', 1)\n"
            "lChildHasParentClass = PEMSTATUS(oChild, 'ParentClass', 1)\n"
            "lChildHasClassLibrary = PEMSTATUS(oChild, 'ClassLibrary', 1)\n"
            "lChildClassReadOnly = PEMSTATUS(oChild, 'Class', 5)\n"
            "lChildBaseClassReadOnly = PEMSTATUS(oChild, 'BaseClass', 5)\n"
            "lChildParentClassReadOnly = PEMSTATUS(oChild, 'ParentClass', 5)\n"
            "lChildClassLibraryReadOnly = PEMSTATUS(oChild, 'ClassLibrary', 5)\n"
            "nMembersProps = AMEMBERS(aMembersProps, oChild, 1)\n"
            "nMembersUnion = AMEMBERS(aMembersUnion, oChild, 3)\n"
            "cProp1 = aMembersProps[1]\n"
            "cProp2 = aMembersProps[2]\n"
            "cProp3 = aMembersProps[3]\n"
            "cProp4 = aMembersProps[4]\n"
            "cProp5 = aMembersProps[5]\n"
            "cProp6 = aMembersProps[6]\n"
            "nClassCount = ACLASS(aClass, oChild)\n"
            "cClass1 = aClass[1]\n"
            "cClass2 = aClass[2]\n"
            "cClass3 = aClass[3]\n"
            "cClass4 = aClass[4]\n"
            "cClass5 = aClass[5]\n"
            "cOwnerCaption = oChild.OwnerCaption()\n"
            "cSavedCaption = oChild.TriggerSave()\n"
            "cCaptionAfterSave = oForm.Caption\n"
            "lChildHasParent = PEMSTATUS(oChild, 'Parent', 1)\n"
            "cRootToken = oChild.RootToken()\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    cInitChildCaption = ''\n"
            "    cInitOwnerCaption = ''\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "    PROCEDURE Init\n"
            "        THIS.cInitChildCaption = THIS.cmdSave.Caption\n"
            "        THIS.cInitOwnerCaption = THIS.cmdSave.OwnerCaption()\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "    FUNCTION Save\n"
            "        THIS.Caption = THIS.Caption + '-Saved'\n"
            "        RETURN THIS.Caption\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS ParentButton OF buttons.prg\n"
            "    Caption = 'Save'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native declarative deeper external child external-base provenance script should complete: ") + state.message +
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

        check("cinitchildcaption", "Save");
        check("cinitownercaption", "MainForm");
        check("cchildclass", "SaveButton");
        check("cchildbaseclass", "ParentButton");
        check("cchildparentclass", "ParentButton");
        check("cchildclasslibrary", button_library_path.string());
        check("cchildclassreflect", "SaveButton");
        check("cchildbaseclassreflect", "ParentButton");
        check("cchildparentclassreflect", "ParentButton");
        check("cchildclasslibraryreflect", button_library_path.string());
        check("lchildhasclass", "true");
        check("lchildhasbaseclass", "true");
        check("lchildhasparentclass", "true");
        check("lchildhasclasslibrary", "true");
        check("lchildclassreadonly", "true");
        check("lchildbaseclassreadonly", "true");
        check("lchildparentclassreadonly", "true");
        check("lchildclasslibraryreadonly", "true");
        check("nmembersprops", "6");
        check("cprop1", "BASECLASS");
        check("cprop2", "CAPTION");
        check("cprop3", "CLASS");
        check("cprop4", "CLASSLIBRARY");
        check("cprop5", "PARENT");
        check("cprop6", "PARENTCLASS");
        check("nclasscount", "5");
        check("cclass1", "SAVEBUTTON");
        check("cclass2", "PARENTBUTTON");
        check("cclass3", "ROOTBUTTON");
        check("cclass4", "CUSTOM");
        check("cclass5", "OBJECT");
        check("cownercaption", "MainForm");
        check("csavedcaption", "MainForm-Saved");
        check("ccaptionaftersave", "MainForm-Saved");
        check("lchildhasparent", "true");
        check("croottoken", "RootToken");
        check("ldictset", "true");
        check("ndictcompare", "155");

        const auto members_union = state.globals.find("nmembersunion");
        expect(members_union != state.globals.end() &&
                   std::stoi(copperfin::runtime::format_value(members_union->second)) >= 9,
               "native declarative deeper external child external-base provenance should keep union member enumeration including inherited child methods");

        expect(state.ole_objects.size() == 3U,
               "native declarative deeper external child external-base provenance should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &parent_object = state.ole_objects[0];
            const auto &child_object = state.ole_objects[1];
            expect(parent_object.prog_id == "DemoForm",
                   "native declarative deeper external child external-base provenance should preserve parent identity");
            expect(parent_object.source == main_path.string(),
                   "native declarative deeper external child external-base provenance should preserve parent source");
            expect(child_object.prog_id == "SaveButton",
                   "native declarative deeper external child external-base provenance should preserve child identity");
            expect(child_object.base_class_name == "ParentButton",
                   "native declarative deeper external child external-base provenance should preserve immediate child base-class identity");
            expect(child_object.source == main_path.string(),
                   "native declarative deeper external child external-base provenance should preserve the defining child-class source path");
            expect(child_object.class_library == button_library_path.string(),
                   "native declarative deeper external child external-base provenance should preserve the immediate external ClassLibrary path");
            expect(child_object.class_hierarchy.size() == 5U,
                   "native declarative deeper external child external-base provenance should preserve the deeper runtime child class hierarchy");
            expect(!child_object.properties.contains("class"),
                   "native declarative deeper external child external-base provenance should not materialize a child Class shadow");
            expect(!child_object.properties.contains("baseclass"),
                   "native declarative deeper external child external-base provenance should not materialize a child BaseClass shadow");
            expect(!child_object.properties.contains("parentclass"),
                   "native declarative deeper external child external-base provenance should not materialize a child ParentClass shadow");
            expect(!child_object.properties.contains("classlibrary"),
                   "native declarative deeper external child external-base provenance should not materialize a child ClassLibrary shadow");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native declarative deeper external child external-base provenance lands");
        }

        const bool has_addobject_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.addobject" &&
                   event.detail == "DemoForm.cmdsave:SaveButton";
        });
        expect(has_addobject_event,
               "native declarative deeper external child external-base provenance should emit child materialization events");

        const bool has_save_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "DemoForm.Save";
        });
        expect(has_save_invoke_event,
               "native declarative deeper external child external-base provenance should keep child THISFORM owner dispatch usable after ordinary reads and reflection");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_declarative_deeper_external_child_external_base_provenance_resists_mutation()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_declarative_deeper_external_child_external_base_mutation";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path root_library_path = temp_root / "rootbuttons.prg";
        write_text(
            root_library_path,
            "DEFINE CLASS RootButton AS Custom\n"
            "    FUNCTION RootToken\n"
            "        RETURN 'RootToken'\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        const fs::path button_library_path = temp_root / "buttons.prg";
        write_text(
            button_library_path,
            "DEFINE CLASS ParentButton AS RootButton OF rootbuttons.prg\n"
            "    FUNCTION OwnerCaption\n"
            "        RETURN PARENT.Caption\n"
            "    ENDFUNC\n"
            "    FUNCTION TriggerSave\n"
            "        RETURN THISFORM.Save()\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "native_declarative_deeper_external_child_external_base_mutation.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('DemoForm')\n"
            "oChild = oForm.cmdSave\n"
            "cInitChildCaption = oForm.cInitChildCaption\n"
            "cInitOwnerCaption = oForm.cInitOwnerCaption\n"
            "lChildClassReadOnly = PEMSTATUS(oChild, 'Class', 5)\n"
            "lChildBaseClassReadOnly = PEMSTATUS(oChild, 'BaseClass', 5)\n"
            "lChildParentClassReadOnly = PEMSTATUS(oChild, 'ParentClass', 5)\n"
            "lChildClassLibraryReadOnly = PEMSTATUS(oChild, 'ClassLibrary', 5)\n"
            "lSetChildClass = SETPEM(oChild, 'Class', 'OtherClass')\n"
            "lSetChildBaseClass = SETPEM(oChild, 'BaseClass', 'OtherBase')\n"
            "lSetChildParentClass = SETPEM(oChild, 'ParentClass', 'OtherParent')\n"
            "lSetChildClassLibrary = SETPEM(oChild, 'ClassLibrary', 'other.prg')\n"
            "lAddChildClass = ADDPROPERTY(oChild, 'Class', 'OtherClass')\n"
            "lAddChildBaseClass = ADDPROPERTY(oChild, 'BaseClass', 'OtherBase')\n"
            "lAddChildParentClass = ADDPROPERTY(oChild, 'ParentClass', 'OtherParent')\n"
            "lAddChildClassLibrary = ADDPROPERTY(oChild, 'ClassLibrary', 'other.prg')\n"
            "lRemoveChildClass = REMOVEPROPERTY(oChild, 'Class')\n"
            "lRemoveChildBaseClass = REMOVEPROPERTY(oChild, 'BaseClass')\n"
            "lRemoveChildParentClass = REMOVEPROPERTY(oChild, 'ParentClass')\n"
            "lRemoveChildClassLibrary = REMOVEPROPERTY(oChild, 'ClassLibrary')\n"
            "oChild.Class = 'OtherClass'\n"
            "oChild.BaseClass = 'OtherBase'\n"
            "oChild.ParentClass = 'OtherParent'\n"
            "oChild.ClassLibrary = 'other.prg'\n"
            "cChildClassAfter = oChild.Class\n"
            "cChildBaseClassAfter = oChild.BaseClass\n"
            "cChildParentClassAfter = oChild.ParentClass\n"
            "cChildClassLibraryAfter = oChild.ClassLibrary\n"
            "cChildClassReflectAfter = GETPEM(oChild, 'Class')\n"
            "cChildBaseClassReflectAfter = GETPEM(oChild, 'BaseClass')\n"
            "cChildParentClassReflectAfter = GETPEM(oChild, 'ParentClass')\n"
            "cChildClassLibraryReflectAfter = GETPEM(oChild, 'ClassLibrary')\n"
            "cOwnerCaption = oChild.OwnerCaption()\n"
            "cSavedCaption = oChild.TriggerSave()\n"
            "cCaptionAfterSave = oForm.Caption\n"
            "cRootToken = oChild.RootToken()\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 156)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    cInitChildCaption = ''\n"
            "    cInitOwnerCaption = ''\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "    PROCEDURE Init\n"
            "        THIS.cInitChildCaption = THIS.cmdSave.Caption\n"
            "        THIS.cInitOwnerCaption = THIS.cmdSave.OwnerCaption()\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "    FUNCTION Save\n"
            "        THIS.Caption = THIS.Caption + '-Saved'\n"
            "        RETURN THIS.Caption\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS ParentButton OF buttons.prg\n"
            "    Caption = 'Save'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native declarative deeper external child external-base mutation script should complete: ") + state.message +
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

        check("cinitchildcaption", "Save");
        check("cinitownercaption", "MainForm");
        check("lchildclassreadonly", "true");
        check("lchildbaseclassreadonly", "true");
        check("lchildparentclassreadonly", "true");
        check("lchildclasslibraryreadonly", "true");
        check("lsetchildclass", "false");
        check("lsetchildbaseclass", "false");
        check("lsetchildparentclass", "false");
        check("lsetchildclasslibrary", "false");
        check("laddchildclass", "false");
        check("laddchildbaseclass", "false");
        check("laddchildparentclass", "false");
        check("laddchildclasslibrary", "false");
        check("lremovechildclass", "false");
        check("lremovechildbaseclass", "false");
        check("lremovechildparentclass", "false");
        check("lremovechildclasslibrary", "false");
        check("cchildclassafter", "SaveButton");
        check("cchildbaseclassafter", "ParentButton");
        check("cchildparentclassafter", "ParentButton");
        check("cchildclasslibraryafter", button_library_path.string());
        check("cchildclassreflectafter", "SaveButton");
        check("cchildbaseclassreflectafter", "ParentButton");
        check("cchildparentclassreflectafter", "ParentButton");
        check("cchildclasslibraryreflectafter", button_library_path.string());
        check("cownercaption", "MainForm");
        check("csavedcaption", "MainForm-Saved");
        check("ccaptionaftersave", "MainForm-Saved");
        check("croottoken", "RootToken");
        check("ldictset", "true");
        check("ndictcompare", "156");

        expect(state.ole_objects.size() == 3U,
               "native declarative deeper external child external-base mutation should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &parent_object = state.ole_objects[0];
            const auto &child_object = state.ole_objects[1];
            expect(parent_object.prog_id == "DemoForm",
                   "native declarative deeper external child external-base mutation should preserve parent identity");
            expect(child_object.prog_id == "SaveButton",
                   "native declarative deeper external child external-base mutation should preserve child identity");
            expect(child_object.base_class_name == "ParentButton",
                   "native declarative deeper external child external-base mutation should preserve immediate child base-class identity");
            expect(child_object.source == main_path.string(),
                   "native declarative deeper external child external-base mutation should preserve defining child-class source path");
            expect(child_object.class_library == button_library_path.string(),
                   "native declarative deeper external child external-base mutation should preserve immediate external ClassLibrary path");
            expect(child_object.class_hierarchy.size() == 5U,
                   "native declarative deeper external child external-base mutation should preserve child class hierarchy");
            expect(!child_object.properties.contains("class"),
                   "native declarative deeper external child external-base mutation should not materialize a child Class shadow");
            expect(!child_object.properties.contains("baseclass"),
                   "native declarative deeper external child external-base mutation should not materialize a child BaseClass shadow");
            expect(!child_object.properties.contains("parentclass"),
                   "native declarative deeper external child external-base mutation should not materialize a child ParentClass shadow");
            expect(!child_object.properties.contains("classlibrary"),
                   "native declarative deeper external child external-base mutation should not materialize a child ClassLibrary shadow");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native declarative deeper external child external-base mutation lands");
        }

        const bool has_addobject_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.addobject" &&
                   event.detail == "DemoForm.cmdsave:SaveButton";
        });
        expect(has_addobject_event,
               "native declarative deeper external child external-base mutation should emit child materialization events");

        const bool has_save_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "DemoForm.Save";
        });
        expect(has_save_invoke_event,
               "native declarative deeper external child external-base mutation should keep child owner dispatch usable after rejected mutations");

        fs::remove_all(temp_root, ignored);
    }

}
