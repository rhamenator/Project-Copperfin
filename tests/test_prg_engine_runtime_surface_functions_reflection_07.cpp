#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_inherited_external_base_object_block_deeper_external_child_base_dotted_access_resolves_live_child_chain()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_base_inherited_object_block_deeper_external_child_base_dotted";
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
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS ParentButton\n"
            "    Caption = 'Save'\n"
            "ENDDEFINE\n");

        const fs::path widget_library_path = temp_root / "widgetlib.prg";
        write_text(
            widget_library_path,
            "DEFINE CLASS ParentForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    OBJECT cmdSave AS SaveButton OF buttons.prg\n"
            "        Caption = 'Commit'\n"
            "    ENDOBJECT\n"
            "    FUNCTION Save\n"
            "        THIS.Caption = THIS.Caption + '-Saved'\n"
            "        RETURN THIS.Caption\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_base_inherited_object_block_deeper_external_child_base_dotted.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oLeaf = NEWOBJECT('LeafForm')\n"
            "cCreateChildCaptionBefore = oCreate.cmdSave.Caption\n"
            "cLeafChildCaptionBefore = oLeaf.cmdSave.Caption\n"
            "cCreateParentCaptionBefore = oCreate.cmdSave.Parent.Caption\n"
            "cLeafParentCaptionBefore = oLeaf.cmdSave.Parent.Caption\n"
            "oCreate.cmdSave.Caption = 'Go'\n"
            "oLeaf.cmdSave.Caption = 'Ship'\n"
            "oCreate.cmdSave.Parent.Caption = 'Done'\n"
            "oLeaf.cmdSave.Parent.Caption = 'Ready'\n"
            "cCreateChildCaptionAfter = oCreate.cmdSave.Caption\n"
            "cLeafChildCaptionAfter = oLeaf.cmdSave.Caption\n"
            "cCreateOwnerCaption = oCreate.cmdSave.OwnerCaption()\n"
            "cLeafOwnerCaption = oLeaf.cmdSave.OwnerCaption()\n"
            "cCreateSavedCaption = oCreate.cmdSave.TriggerSave()\n"
            "cLeafSavedCaption = oLeaf.cmdSave.TriggerSave()\n"
            "cCreateCaptionAfterSave = oCreate.Caption\n"
            "cLeafCaptionAfterSave = oLeaf.Caption\n"
            "cCreateChildBaseClass = oCreate.cmdSave.BaseClass\n"
            "cLeafChildBaseClass = oLeaf.cmdSave.BaseClass\n"
            "xCreateChildClassLibrary = oCreate.cmdSave.ClassLibrary\n"
            "xLeafChildClassLibrary = oLeaf.cmdSave.ClassLibrary\n"
            "lCreateChildHasParent = PEMSTATUS(oCreate.cmdSave, 'Parent', 1)\n"
            "lLeafChildHasParent = PEMSTATUS(oLeaf.cmdSave, 'Parent', 1)\n"
            "cCreateRootToken = oCreate.cmdSave.RootToken()\n"
            "cLeafRootToken = oLeaf.cmdSave.RootToken()\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 142)\n"
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
               std::string("external-base inherited object-block deeper external child-base dotted script should complete: ") + state.message +
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

        check("ccreatechildcaptionbefore", "Commit");
        check("cleafchildcaptionbefore", "Commit");
        check("ccreateparentcaptionbefore", "MainForm");
        check("cleafparentcaptionbefore", "MainForm");
        check("ccreatechildcaptionafter", "Go");
        check("cleafchildcaptionafter", "Ship");
        check("ccreateownercaption", "Done");
        check("cleafownercaption", "Ready");
        check("ccreatesavedcaption", "Done-Saved");
        check("cleafsavedcaption", "Ready-Saved");
        check("ccreatecaptionaftersave", "Done-Saved");
        check("cleafcaptionaftersave", "Ready-Saved");
        check("ccreatechildbaseclass", "ParentButton");
        check("cleafchildbaseclass", "ParentButton");
        check("lcreatechildhasparent", "true");
        check("lleafchildhasparent", "true");
        check("ccreateroottoken", "RootToken");
        check("cleafroottoken", "RootToken");
        check("ldictset", "true");
        check("ndictcompare", "142");

        const auto create_child_class_library = state.globals.find("xcreatechildclasslibrary");
        expect(create_child_class_library != state.globals.end() &&
                   create_child_class_library->second.kind == copperfin::runtime::PrgValueKind::empty,
               "external-base inherited object-block deeper external child-base dotted should keep the CREATEOBJECT child ClassLibrary empty through ordinary reads");
        const auto leaf_child_class_library = state.globals.find("xleafchildclasslibrary");
        expect(leaf_child_class_library != state.globals.end() &&
                   leaf_child_class_library->second.kind == copperfin::runtime::PrgValueKind::empty,
               "external-base inherited object-block deeper external child-base dotted should keep the leaf child ClassLibrary empty through ordinary reads");

        expect(state.ole_objects.size() == 5U,
               "external-base inherited object-block deeper external child-base dotted should register CREATEOBJECT parent/child, NEWOBJECT parent/child, and COM objects");
        if (state.ole_objects.size() == 5U)
        {
            const auto &create_parent = state.ole_objects[0];
            const auto &create_child = state.ole_objects[1];
            const auto &leaf_parent = state.ole_objects[2];
            const auto &leaf_child = state.ole_objects[3];
            expect(create_parent.prog_id == "ChildForm",
                   "external-base inherited object-block deeper external child-base dotted should preserve CREATEOBJECT parent identity");
            expect(create_parent.source == main_path.string(),
                   "external-base inherited object-block deeper external child-base dotted should preserve CREATEOBJECT parent source");
            expect(create_child.prog_id == "SaveButton",
                   "external-base inherited object-block deeper external child-base dotted should preserve CREATEOBJECT child identity");
            expect(create_child.base_class_name == "ParentButton",
                   "external-base inherited object-block deeper external child-base dotted should preserve the CREATEOBJECT child immediate external base");
            expect(create_child.source == button_library_path.string(),
                   "external-base inherited object-block deeper external child-base dotted should preserve CREATEOBJECT child definition source");
            expect(create_child.class_library.empty(),
                   "external-base inherited object-block deeper external child-base dotted should keep the CREATEOBJECT child ClassLibrary empty");
            expect(leaf_parent.prog_id == "LeafForm",
                   "external-base inherited object-block deeper external child-base dotted should preserve NEWOBJECT leaf identity");
            expect(leaf_parent.source == main_path.string(),
                   "external-base inherited object-block deeper external child-base dotted should preserve NEWOBJECT leaf source");
            expect(leaf_child.prog_id == "SaveButton",
                   "external-base inherited object-block deeper external child-base dotted should preserve leaf child identity");
            expect(leaf_child.base_class_name == "ParentButton",
                   "external-base inherited object-block deeper external child-base dotted should preserve the leaf child immediate external base");
            expect(leaf_child.source == button_library_path.string(),
                   "external-base inherited object-block deeper external child-base dotted should preserve leaf child definition source");
            expect(leaf_child.class_library.empty(),
                   "external-base inherited object-block deeper external child-base dotted should keep the leaf child ClassLibrary empty");

            const auto create_parent_caption = create_parent.properties.find("caption");
            const auto create_child_caption = create_child.properties.find("caption");
            const auto leaf_parent_caption = leaf_parent.properties.find("caption");
            const auto leaf_child_caption = leaf_child.properties.find("caption");
            if (create_parent_caption != create_parent.properties.end())
            {
                expect(copperfin::runtime::format_value(create_parent_caption->second) == "Done-Saved",
                       "external-base inherited object-block deeper external child-base dotted should let dotted child method calls update the CREATEOBJECT owner");
            }
            else
            {
                expect(false, "external-base inherited object-block deeper external child-base dotted should preserve the CREATEOBJECT owner caption");
            }
            if (create_child_caption != create_child.properties.end())
            {
                expect(copperfin::runtime::format_value(create_child_caption->second) == "Go",
                       "external-base inherited object-block deeper external child-base dotted should preserve the CREATEOBJECT child dotted assignment");
            }
            else
            {
                expect(false, "external-base inherited object-block deeper external child-base dotted should preserve the CREATEOBJECT child caption");
            }
            if (leaf_parent_caption != leaf_parent.properties.end())
            {
                expect(copperfin::runtime::format_value(leaf_parent_caption->second) == "Ready-Saved",
                       "external-base inherited object-block deeper external child-base dotted should let dotted child method calls update the leaf owner");
            }
            else
            {
                expect(false, "external-base inherited object-block deeper external child-base dotted should preserve the leaf owner caption");
            }
            if (leaf_child_caption != leaf_child.properties.end())
            {
                expect(copperfin::runtime::format_value(leaf_child_caption->second) == "Ship",
                       "external-base inherited object-block deeper external child-base dotted should preserve the leaf child dotted assignment");
            }
            else
            {
                expect(false, "external-base inherited object-block deeper external child-base dotted should preserve the leaf child caption");
            }
            expect(state.ole_objects[4].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external-base inherited object-block deeper external child-base dotted lands");
        }

        const bool has_addobject_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.addobject" &&
                   (event.detail == "ChildForm.cmdsave:SaveButton" ||
                    event.detail == "LeafForm.cmdsave:SaveButton");
        });
        expect(has_addobject_event,
               "external-base inherited object-block deeper external child-base dotted should emit child materialization events");

        const bool has_owner_caption_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   (event.detail == "SaveButton.OwnerCaption" ||
                    event.detail == "ParentButton.OwnerCaption");
        });
        expect(has_owner_caption_invoke_event,
               "external-base inherited object-block deeper external child-base dotted should dispatch direct dotted child method calls");

        const bool has_save_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "ParentForm.Save";
        });
        expect(has_save_invoke_event,
               "external-base inherited object-block deeper external child-base dotted should keep child THISFORM owner dispatch stable");

        fs::remove_all(temp_root, ignored);
    }

    void test_inherited_external_base_object_block_deeper_external_child_identity_and_provenance_surfaces_stay_coherent()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_base_inherited_object_block_deeper_external_child_identity";
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
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS ParentButton\n"
            "    Caption = 'Save'\n"
            "ENDDEFINE\n");

        const fs::path widget_library_path = temp_root / "widgetlib.prg";
        write_text(
            widget_library_path,
            "DEFINE CLASS ParentForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    cInitChildCaption = ''\n"
            "    cInitOwnerCaption = ''\n"
            "    OBJECT cmdSave AS SaveButton OF buttons.prg\n"
            "        Caption = 'Commit'\n"
            "    ENDOBJECT\n"
            "    PROCEDURE Init\n"
            "        THIS.cInitChildCaption = THIS.cmdSave.Caption\n"
            "        THIS.cInitOwnerCaption = THIS.cmdSave.OwnerCaption()\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "    FUNCTION Save\n"
            "        THIS.Caption = THIS.Caption + '-Saved'\n"
            "        RETURN THIS.Caption\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_base_inherited_object_block_deeper_external_child_identity.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oLeaf = NEWOBJECT('LeafForm')\n"
            "oChild = oCreate.cmdSave\n"
            "oLeafChild = oLeaf.cmdSave\n"
            "cCreateInitChildCaption = oCreate.cInitChildCaption\n"
            "cLeafInitChildCaption = oLeaf.cInitChildCaption\n"
            "cCreateInitOwnerCaption = oCreate.cInitOwnerCaption\n"
            "cLeafInitOwnerCaption = oLeaf.cInitOwnerCaption\n"
            "cCreateChildClass = oChild.Class\n"
            "cCreateChildBaseClass = oChild.BaseClass\n"
            "cCreateChildParentClass = oChild.ParentClass\n"
            "xCreateChildClassLibrary = oChild.ClassLibrary\n"
            "cCreateChildClassReflect = GETPEM(oChild, 'Class')\n"
            "cCreateChildBaseClassReflect = GETPEM(oChild, 'BaseClass')\n"
            "cCreateChildParentClassReflect = GETPEM(oChild, 'ParentClass')\n"
            "xCreateChildClassLibraryReflect = GETPEM(oChild, 'ClassLibrary')\n"
            "lCreateChildHasClass = PEMSTATUS(oChild, 'Class', 1)\n"
            "lCreateChildHasBaseClass = PEMSTATUS(oChild, 'BaseClass', 1)\n"
            "lCreateChildHasParentClass = PEMSTATUS(oChild, 'ParentClass', 1)\n"
            "lCreateChildHasClassLibrary = PEMSTATUS(oChild, 'ClassLibrary', 1)\n"
            "lCreateChildClassReadOnly = PEMSTATUS(oChild, 'Class', 5)\n"
            "lCreateChildBaseClassReadOnly = PEMSTATUS(oChild, 'BaseClass', 5)\n"
            "lCreateChildParentClassReadOnly = PEMSTATUS(oChild, 'ParentClass', 5)\n"
            "nMembersProps = AMEMBERS(aMembersProps, oChild, 1)\n"
            "nMembersUnion = AMEMBERS(aMembersUnion, oChild, 3)\n"
            "cProp1 = aMembersProps[1]\n"
            "cProp2 = aMembersProps[2]\n"
            "cProp3 = aMembersProps[3]\n"
            "cProp4 = aMembersProps[4]\n"
            "cProp5 = aMembersProps[5]\n"
            "cUnion1 = aMembersUnion[1]\n"
            "cUnion5 = aMembersUnion[5]\n"
            "nClassCount = ACLASS(aClass, oChild)\n"
            "cClass1 = aClass[1]\n"
            "cClass2 = aClass[2]\n"
            "cClass3 = aClass[3]\n"
            "cClass4 = aClass[4]\n"
            "cClass5 = aClass[5]\n"
            "cCreateOwnerCaption = oChild.OwnerCaption()\n"
            "cLeafOwnerCaption = oLeafChild.OwnerCaption()\n"
            "cCreateSavedCaption = oChild.TriggerSave()\n"
            "cLeafSavedCaption = oLeafChild.TriggerSave()\n"
            "cCreateCaptionAfterSave = oCreate.Caption\n"
            "cLeafCaptionAfterSave = oLeaf.Caption\n"
            "cLeafChildClass = oLeafChild.Class\n"
            "cLeafChildBaseClass = oLeafChild.BaseClass\n"
            "cLeafChildParentClass = oLeafChild.ParentClass\n"
            "xLeafChildClassLibrary = oLeafChild.ClassLibrary\n"
            "lCreateChildHasParent = PEMSTATUS(oChild, 'Parent', 1)\n"
            "lLeafChildHasParent = PEMSTATUS(oLeafChild, 'Parent', 1)\n"
            "cCreateRootToken = oChild.RootToken()\n"
            "cLeafRootToken = oLeafChild.RootToken()\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 146)\n"
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
               std::string("external-base inherited object-block deeper external child identity script should complete: ") + state.message +
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

        check("ccreateinitchildcaption", "Commit");
        check("cleafinitchildcaption", "Commit");
        check("ccreateinitownercaption", "MainForm");
        check("cleafinitownercaption", "MainForm");
        check("ccreatechildclass", "SaveButton");
        check("ccreatechildbaseclass", "ParentButton");
        check("ccreatechildparentclass", "ParentButton");
        check("ccreatechildclassreflect", "SaveButton");
        check("ccreatechildbaseclassreflect", "ParentButton");
        check("ccreatechildparentclassreflect", "ParentButton");
        check("lcreatechildhasclass", "true");
        check("lcreatechildhasbaseclass", "true");
        check("lcreatechildhasparentclass", "true");
        check("lcreatechildhasclasslibrary", "false");
        check("lcreatechildclassreadonly", "true");
        check("lcreatechildbaseclassreadonly", "true");
        check("lcreatechildparentclassreadonly", "true");
        check("nmembersprops", "5");
        check("cprop1", "BASECLASS");
        check("cprop2", "CAPTION");
        check("cprop3", "CLASS");
        check("cprop4", "PARENT");
        check("cprop5", "PARENTCLASS");
        check("cunion1", "BASECLASS");
        check("nclasscount", "5");
        check("cclass1", "SAVEBUTTON");
        check("cclass2", "PARENTBUTTON");
        check("cclass3", "ROOTBUTTON");
        check("cclass4", "CUSTOM");
        check("cclass5", "OBJECT");
        check("ccreateownercaption", "MainForm");
        check("cleafownercaption", "MainForm");
        check("ccreatesavedcaption", "MainForm-Saved");
        check("cleafsavedcaption", "MainForm-Saved");
        check("ccreatecaptionaftersave", "MainForm-Saved");
        check("cleafcaptionaftersave", "MainForm-Saved");
        check("cleafchildclass", "SaveButton");
        check("cleafchildbaseclass", "ParentButton");
        check("cleafchildparentclass", "ParentButton");
        check("lcreatechildhasparent", "true");
        check("lleafchildhasparent", "true");
        check("ccreateroottoken", "RootToken");
        check("cleafroottoken", "RootToken");
        check("ldictset", "true");
        check("ndictcompare", "146");

        const auto create_child_class_library = state.globals.find("xcreatechildclasslibrary");
        expect(create_child_class_library != state.globals.end() &&
                   create_child_class_library->second.kind == copperfin::runtime::PrgValueKind::empty,
               "external-base inherited object-block deeper external child identity should leave the CREATEOBJECT child ClassLibrary empty through ordinary reads");
        const auto create_child_class_library_reflect = state.globals.find("xcreatechildclasslibraryreflect");
        expect(create_child_class_library_reflect != state.globals.end() &&
                   create_child_class_library_reflect->second.kind == copperfin::runtime::PrgValueKind::empty,
               "external-base inherited object-block deeper external child identity should leave the CREATEOBJECT child ClassLibrary empty through GETPEM");
        const auto leaf_child_class_library = state.globals.find("xleafchildclasslibrary");
        expect(leaf_child_class_library != state.globals.end() &&
                   leaf_child_class_library->second.kind == copperfin::runtime::PrgValueKind::empty,
               "external-base inherited object-block deeper external child identity should leave the leaf child ClassLibrary empty through ordinary reads");
        const auto members_union = state.globals.find("nmembersunion");
        expect(members_union != state.globals.end() &&
                   std::stoi(copperfin::runtime::format_value(members_union->second)) >= 8,
               "external-base inherited object-block deeper external child identity should keep union member enumeration including child methods");

        expect(state.ole_objects.size() == 5U,
               "external-base inherited object-block deeper external child identity should register CREATEOBJECT parent/child, NEWOBJECT parent/child, and COM objects");
        if (state.ole_objects.size() == 5U)
        {
            const auto &create_parent = state.ole_objects[0];
            const auto &create_child = state.ole_objects[1];
            const auto &leaf_parent = state.ole_objects[2];
            const auto &leaf_child = state.ole_objects[3];
            expect(create_parent.prog_id == "ChildForm",
                   "external-base inherited object-block deeper external child identity should preserve CREATEOBJECT parent identity");
            expect(create_parent.source == main_path.string(),
                   "external-base inherited object-block deeper external child identity should preserve CREATEOBJECT parent source");
            expect(create_child.prog_id == "SaveButton",
                   "external-base inherited object-block deeper external child identity should preserve CREATEOBJECT child identity");
            expect(create_child.base_class_name == "ParentButton",
                   "external-base inherited object-block deeper external child identity should preserve the CREATEOBJECT child immediate external base");
            expect(create_child.source == button_library_path.string(),
                   "external-base inherited object-block deeper external child identity should preserve CREATEOBJECT child definition source");
            expect(create_child.class_library.empty(),
                   "external-base inherited object-block deeper external child identity should leave the CREATEOBJECT child ClassLibrary empty");
            expect(create_child.class_hierarchy.size() == 5U,
                   "external-base inherited object-block deeper external child identity should preserve CREATEOBJECT child class hierarchy");
            if (create_child.class_hierarchy.size() == 5U)
            {
                expect(create_child.class_hierarchy[0] == "SAVEBUTTON",
                       "external-base inherited object-block deeper external child identity should store the CREATEOBJECT child class first");
                expect(create_child.class_hierarchy[1] == "PARENTBUTTON",
                       "external-base inherited object-block deeper external child identity should store the CREATEOBJECT immediate external base second");
                expect(create_child.class_hierarchy[2] == "ROOTBUTTON",
                       "external-base inherited object-block deeper external child identity should store the CREATEOBJECT deeper external ancestor third");
                expect(create_child.class_hierarchy[3] == "CUSTOM",
                       "external-base inherited object-block deeper external child identity should store the CREATEOBJECT builtin base token fourth");
                expect(create_child.class_hierarchy[4] == "OBJECT",
                       "external-base inherited object-block deeper external child identity should store the CREATEOBJECT terminal object token fifth");
            }
            expect(!create_child.properties.contains("class"),
                   "external-base inherited object-block deeper external child identity should not materialize a CREATEOBJECT child Class shadow");
            expect(!create_child.properties.contains("baseclass"),
                   "external-base inherited object-block deeper external child identity should not materialize a CREATEOBJECT child BaseClass shadow");
            expect(!create_child.properties.contains("parentclass"),
                   "external-base inherited object-block deeper external child identity should not materialize a CREATEOBJECT child ParentClass shadow");
            expect(!create_child.properties.contains("classlibrary"),
                   "external-base inherited object-block deeper external child identity should not materialize a CREATEOBJECT child ClassLibrary shadow");
            expect(leaf_parent.prog_id == "LeafForm",
                   "external-base inherited object-block deeper external child identity should preserve NEWOBJECT leaf identity");
            expect(leaf_parent.source == main_path.string(),
                   "external-base inherited object-block deeper external child identity should preserve NEWOBJECT leaf source");
            expect(leaf_child.prog_id == "SaveButton",
                   "external-base inherited object-block deeper external child identity should preserve leaf child identity");
            expect(leaf_child.base_class_name == "ParentButton",
                   "external-base inherited object-block deeper external child identity should preserve the leaf child immediate external base");
            expect(leaf_child.source == button_library_path.string(),
                   "external-base inherited object-block deeper external child identity should preserve leaf child definition source");
            expect(leaf_child.class_library.empty(),
                   "external-base inherited object-block deeper external child identity should leave the leaf child ClassLibrary empty");
            expect(leaf_child.class_hierarchy.size() == 5U,
                   "external-base inherited object-block deeper external child identity should preserve leaf child class hierarchy");
            expect(!leaf_child.properties.contains("class"),
                   "external-base inherited object-block deeper external child identity should not materialize a leaf child Class shadow");
            expect(!leaf_child.properties.contains("baseclass"),
                   "external-base inherited object-block deeper external child identity should not materialize a leaf child BaseClass shadow");
            expect(!leaf_child.properties.contains("parentclass"),
                   "external-base inherited object-block deeper external child identity should not materialize a leaf child ParentClass shadow");
            expect(!leaf_child.properties.contains("classlibrary"),
                   "external-base inherited object-block deeper external child identity should not materialize a leaf child ClassLibrary shadow");
            expect(state.ole_objects[4].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external-base inherited object-block deeper external child identity lands");
        }

        const bool has_addobject_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.addobject" &&
                   (event.detail == "ChildForm.cmdsave:SaveButton" ||
                    event.detail == "LeafForm.cmdsave:SaveButton");
        });
        expect(has_addobject_event,
               "external-base inherited object-block deeper external child identity should emit child materialization events");

        const bool has_save_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "ParentForm.Save";
        });
        expect(has_save_invoke_event,
               "external-base inherited object-block deeper external child identity should keep child THISFORM owner dispatch usable after ordinary reads and reflection");

        fs::remove_all(temp_root, ignored);
    }

    void test_inherited_external_base_object_block_deeper_external_child_identity_and_provenance_resist_mutation()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_base_inherited_object_block_deeper_external_child_identity_mutation";
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
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS ParentButton\n"
            "    Caption = 'Save'\n"
            "ENDDEFINE\n");

        const fs::path widget_library_path = temp_root / "widgetlib.prg";
        write_text(
            widget_library_path,
            "DEFINE CLASS ParentForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    OBJECT cmdSave AS SaveButton OF buttons.prg\n"
            "        Caption = 'Commit'\n"
            "    ENDOBJECT\n"
            "    FUNCTION Save\n"
            "        THIS.Caption = THIS.Caption + '-Saved'\n"
            "        RETURN THIS.Caption\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_base_inherited_object_block_deeper_external_child_identity_mutation.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oLeaf = NEWOBJECT('LeafForm')\n"
            "oCreateChild = oCreate.cmdSave\n"
            "oLeafChild = oLeaf.cmdSave\n"
            "lCreateChildClassReadOnly = PEMSTATUS(oCreateChild, 'Class', 5)\n"
            "lCreateChildBaseClassReadOnly = PEMSTATUS(oCreateChild, 'BaseClass', 5)\n"
            "lCreateChildParentClassReadOnly = PEMSTATUS(oCreateChild, 'ParentClass', 5)\n"
            "lSetCreateChildClass = SETPEM(oCreateChild, 'Class', 'OtherClass')\n"
            "lSetCreateChildBaseClass = SETPEM(oCreateChild, 'BaseClass', 'OtherBase')\n"
            "lSetCreateChildParentClass = SETPEM(oCreateChild, 'ParentClass', 'OtherParent')\n"
            "lSetCreateChildClassLibrary = SETPEM(oCreateChild, 'ClassLibrary', 'other.prg')\n"
            "lAddCreateChildClass = ADDPROPERTY(oCreateChild, 'Class', 'OtherClass')\n"
            "lAddCreateChildBaseClass = ADDPROPERTY(oCreateChild, 'BaseClass', 'OtherBase')\n"
            "lAddCreateChildParentClass = ADDPROPERTY(oCreateChild, 'ParentClass', 'OtherParent')\n"
            "lAddCreateChildClassLibrary = ADDPROPERTY(oCreateChild, 'ClassLibrary', 'other.prg')\n"
            "cCreateChildClassAfter = oCreateChild.Class\n"
            "cCreateChildBaseClassAfter = oCreateChild.BaseClass\n"
            "cCreateChildParentClassAfter = oCreateChild.ParentClass\n"
            "xCreateChildClassLibraryAfter = oCreateChild.ClassLibrary\n"
            "cCreateChildClassReflectAfter = GETPEM(oCreateChild, 'Class')\n"
            "cCreateChildBaseClassReflectAfter = GETPEM(oCreateChild, 'BaseClass')\n"
            "cCreateChildParentClassReflectAfter = GETPEM(oCreateChild, 'ParentClass')\n"
            "xCreateChildClassLibraryReflectAfter = GETPEM(oCreateChild, 'ClassLibrary')\n"
            "lRemoveLeafChildClass = REMOVEPROPERTY(oLeafChild, 'Class')\n"
            "lRemoveLeafChildBaseClass = REMOVEPROPERTY(oLeafChild, 'BaseClass')\n"
            "lRemoveLeafChildParentClass = REMOVEPROPERTY(oLeafChild, 'ParentClass')\n"
            "lRemoveLeafChildClassLibrary = REMOVEPROPERTY(oLeafChild, 'ClassLibrary')\n"
            "oLeafChild.Class = 'OtherClass'\n"
            "oLeafChild.BaseClass = 'OtherBase'\n"
            "oLeafChild.ParentClass = 'OtherParent'\n"
            "oLeafChild.ClassLibrary = 'other.prg'\n"
            "lLeafChildClassReadOnly = PEMSTATUS(oLeafChild, 'Class', 5)\n"
            "lLeafChildBaseClassReadOnly = PEMSTATUS(oLeafChild, 'BaseClass', 5)\n"
            "lLeafChildParentClassReadOnly = PEMSTATUS(oLeafChild, 'ParentClass', 5)\n"
            "cLeafChildClassAfter = oLeafChild.Class\n"
            "cLeafChildBaseClassAfter = oLeafChild.BaseClass\n"
            "cLeafChildParentClassAfter = oLeafChild.ParentClass\n"
            "xLeafChildClassLibraryAfter = oLeafChild.ClassLibrary\n"
            "cLeafChildClassReflectAfter = GETPEM(oLeafChild, 'Class')\n"
            "cLeafChildBaseClassReflectAfter = GETPEM(oLeafChild, 'BaseClass')\n"
            "cLeafChildParentClassReflectAfter = GETPEM(oLeafChild, 'ParentClass')\n"
            "xLeafChildClassLibraryReflectAfter = GETPEM(oLeafChild, 'ClassLibrary')\n"
            "cCreateOwnerCaption = oCreateChild.OwnerCaption()\n"
            "cLeafOwnerCaption = oLeafChild.OwnerCaption()\n"
            "cCreateSavedCaption = oCreateChild.TriggerSave()\n"
            "cLeafSavedCaption = oLeafChild.TriggerSave()\n"
            "cCreateCaptionAfterSave = oCreate.Caption\n"
            "cLeafCaptionAfterSave = oLeaf.Caption\n"
            "cCreateRootToken = oCreateChild.RootToken()\n"
            "cLeafRootToken = oLeafChild.RootToken()\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 147)\n"
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
               std::string("external-base inherited object-block deeper external child identity mutation script should complete: ") + state.message +
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

        check("lcreatechildclassreadonly", "true");
        check("lcreatechildbaseclassreadonly", "true");
        check("lcreatechildparentclassreadonly", "true");
        check("lsetcreatechildclass", "false");
        check("lsetcreatechildbaseclass", "false");
        check("lsetcreatechildparentclass", "false");
        check("lsetcreatechildclasslibrary", "false");
        check("laddcreatechildclass", "false");
        check("laddcreatechildbaseclass", "false");
        check("laddcreatechildparentclass", "false");
        check("laddcreatechildclasslibrary", "false");
        check("ccreatechildclassafter", "SaveButton");
        check("ccreatechildbaseclassafter", "ParentButton");
        check("ccreatechildparentclassafter", "ParentButton");
        check("ccreatechildclassreflectafter", "SaveButton");
        check("ccreatechildbaseclassreflectafter", "ParentButton");
        check("ccreatechildparentclassreflectafter", "ParentButton");
        check("lremoveleafchildclass", "false");
        check("lremoveleafchildbaseclass", "false");
        check("lremoveleafchildparentclass", "false");
        check("lremoveleafchildclasslibrary", "false");
        check("lleafchildclassreadonly", "true");
        check("lleafchildbaseclassreadonly", "true");
        check("lleafchildparentclassreadonly", "true");
        check("cleafchildclassafter", "SaveButton");
        check("cleafchildbaseclassafter", "ParentButton");
        check("cleafchildparentclassafter", "ParentButton");
        check("cleafchildclassreflectafter", "SaveButton");
        check("cleafchildbaseclassreflectafter", "ParentButton");
        check("cleafchildparentclassreflectafter", "ParentButton");
        check("ccreateownercaption", "MainForm");
        check("cleafownercaption", "MainForm");
        check("ccreatesavedcaption", "MainForm-Saved");
        check("cleafsavedcaption", "MainForm-Saved");
        check("ccreatecaptionaftersave", "MainForm-Saved");
        check("cleafcaptionaftersave", "MainForm-Saved");
        check("ccreateroottoken", "RootToken");
        check("cleafroottoken", "RootToken");
        check("ldictset", "true");
        check("ndictcompare", "147");

        const auto create_child_class_library_after = state.globals.find("xcreatechildclasslibraryafter");
        expect(create_child_class_library_after != state.globals.end() &&
                   create_child_class_library_after->second.kind == copperfin::runtime::PrgValueKind::empty,
               "external-base inherited object-block deeper external child identity mutation should keep the CREATEOBJECT child ClassLibrary empty after rejected mutation");
        const auto create_child_class_library_reflect_after = state.globals.find("xcreatechildclasslibraryreflectafter");
        expect(create_child_class_library_reflect_after != state.globals.end() &&
                   create_child_class_library_reflect_after->second.kind == copperfin::runtime::PrgValueKind::empty,
               "external-base inherited object-block deeper external child identity mutation should keep the CREATEOBJECT child ClassLibrary empty through reflection after rejected mutation");
        const auto leaf_child_class_library_after = state.globals.find("xleafchildclasslibraryafter");
        expect(leaf_child_class_library_after != state.globals.end() &&
                   leaf_child_class_library_after->second.kind == copperfin::runtime::PrgValueKind::empty,
               "external-base inherited object-block deeper external child identity mutation should keep the leaf child ClassLibrary empty after rejected mutation");
        const auto leaf_child_class_library_reflect_after = state.globals.find("xleafchildclasslibraryreflectafter");
        expect(leaf_child_class_library_reflect_after != state.globals.end() &&
                   leaf_child_class_library_reflect_after->second.kind == copperfin::runtime::PrgValueKind::empty,
               "external-base inherited object-block deeper external child identity mutation should keep the leaf child ClassLibrary empty through reflection after rejected mutation");

        expect(state.ole_objects.size() == 5U,
               "external-base inherited object-block deeper external child identity mutation should register CREATEOBJECT parent/child, NEWOBJECT parent/child, and COM objects");
        if (state.ole_objects.size() == 5U)
        {
            const auto &create_parent = state.ole_objects[0];
            const auto &create_child = state.ole_objects[1];
            const auto &leaf_parent = state.ole_objects[2];
            const auto &leaf_child = state.ole_objects[3];
            expect(create_parent.prog_id == "ChildForm",
                   "external-base inherited object-block deeper external child identity mutation should preserve CREATEOBJECT parent identity");
            expect(create_child.prog_id == "SaveButton",
                   "external-base inherited object-block deeper external child identity mutation should preserve CREATEOBJECT child identity");
            expect(create_child.base_class_name == "ParentButton",
                   "external-base inherited object-block deeper external child identity mutation should preserve the CREATEOBJECT child immediate external base");
            expect(create_child.source == button_library_path.string(),
                   "external-base inherited object-block deeper external child identity mutation should preserve CREATEOBJECT child definition source");
            expect(create_child.class_library.empty(),
                   "external-base inherited object-block deeper external child identity mutation should keep the CREATEOBJECT child ClassLibrary empty");
            expect(create_child.class_hierarchy.size() == 5U,
                   "external-base inherited object-block deeper external child identity mutation should preserve CREATEOBJECT child class hierarchy");
            expect(!create_child.properties.contains("class"),
                   "external-base inherited object-block deeper external child identity mutation should not materialize a CREATEOBJECT child Class shadow");
            expect(!create_child.properties.contains("baseclass"),
                   "external-base inherited object-block deeper external child identity mutation should not materialize a CREATEOBJECT child BaseClass shadow");
            expect(!create_child.properties.contains("parentclass"),
                   "external-base inherited object-block deeper external child identity mutation should not materialize a CREATEOBJECT child ParentClass shadow");
            expect(!create_child.properties.contains("classlibrary"),
                   "external-base inherited object-block deeper external child identity mutation should not materialize a CREATEOBJECT child ClassLibrary shadow");
            expect(leaf_parent.prog_id == "LeafForm",
                   "external-base inherited object-block deeper external child identity mutation should preserve NEWOBJECT leaf identity");
            expect(leaf_child.prog_id == "SaveButton",
                   "external-base inherited object-block deeper external child identity mutation should preserve leaf child identity");
            expect(leaf_child.base_class_name == "ParentButton",
                   "external-base inherited object-block deeper external child identity mutation should preserve the leaf child immediate external base");
            expect(leaf_child.source == button_library_path.string(),
                   "external-base inherited object-block deeper external child identity mutation should preserve leaf child definition source");
            expect(leaf_child.class_library.empty(),
                   "external-base inherited object-block deeper external child identity mutation should keep the leaf child ClassLibrary empty");
            expect(leaf_child.class_hierarchy.size() == 5U,
                   "external-base inherited object-block deeper external child identity mutation should preserve leaf child class hierarchy");
            expect(!leaf_child.properties.contains("class"),
                   "external-base inherited object-block deeper external child identity mutation should not materialize a leaf child Class shadow");
            expect(!leaf_child.properties.contains("baseclass"),
                   "external-base inherited object-block deeper external child identity mutation should not materialize a leaf child BaseClass shadow");
            expect(!leaf_child.properties.contains("parentclass"),
                   "external-base inherited object-block deeper external child identity mutation should not materialize a leaf child ParentClass shadow");
            expect(!leaf_child.properties.contains("classlibrary"),
                   "external-base inherited object-block deeper external child identity mutation should not materialize a leaf child ClassLibrary shadow");
            expect(state.ole_objects[4].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external-base inherited object-block deeper external child identity mutation lands");
        }

        const bool has_save_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "ParentForm.Save";
        });
        expect(has_save_invoke_event,
               "external-base inherited object-block deeper external child identity mutation should keep child owner dispatch usable after rejected mutations");

        fs::remove_all(temp_root, ignored);
    }

}
