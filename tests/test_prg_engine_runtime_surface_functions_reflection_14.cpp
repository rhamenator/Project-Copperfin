#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_object_block_deeper_external_child_external_base_dotted_access_resolves_live_child_chain()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_object_block_deeper_external_child_external_base_dotted";
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

        const fs::path main_path = temp_root / "native_object_block_deeper_external_child_external_base_dotted.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('DemoForm')\n"
            "cChildCaptionBefore = oForm.cmdSave.Caption\n"
            "cParentCaptionBefore = oForm.cmdSave.Parent.Caption\n"
            "oForm.cmdSave.Caption = 'Go'\n"
            "oForm.cmdSave.Parent.Caption = 'Done'\n"
            "cChildCaptionAfter = oForm.cmdSave.Caption\n"
            "cOwnerCaption = oForm.cmdSave.OwnerCaption()\n"
            "cSavedCaption = oForm.cmdSave.TriggerSave()\n"
            "cParentCaptionAfterSave = oForm.Caption\n"
            "cChildBaseClass = oForm.cmdSave.BaseClass\n"
            "cChildClassLibrary = oForm.cmdSave.ClassLibrary\n"
            "lChildHasParent = PEMSTATUS(oForm.cmdSave, 'Parent', 1)\n"
            "cRootToken = oForm.cmdSave.RootToken()\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 155)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    OBJECT cmdSave AS SaveButton\n"
            "        Caption = 'Commit'\n"
            "    ENDOBJECT\n"
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
               std::string("native object-block deeper external child external-base dotted script should complete: ") + state.message +
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

        check("cchildcaptionbefore", "Commit");
        check("cparentcaptionbefore", "MainForm");
        check("cchildcaptionafter", "Go");
        check("cownercaption", "Done");
        check("csavedcaption", "Done-Saved");
        check("cparentcaptionaftersave", "Done-Saved");
        check("cchildbaseclass", "ParentButton");
        check("cchildclasslibrary", button_library_path.string());
        check("lchildhasparent", "true");
        check("croottoken", "RootToken");
        check("ldictset", "true");
        check("ndictcompare", "155");

        expect(state.ole_objects.size() == 3U,
               "native object-block deeper external child external-base dotted should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &parent_object = state.ole_objects[0];
            const auto &child_object = state.ole_objects[1];
            expect(parent_object.prog_id == "DemoForm",
                   "native object-block deeper external child external-base dotted should preserve parent identity");
            expect(parent_object.source == main_path.string(),
                   "native object-block deeper external child external-base dotted should preserve parent source");
            expect(child_object.prog_id == "SaveButton",
                   "native object-block deeper external child external-base dotted should preserve child identity");
            expect(child_object.base_class_name == "ParentButton",
                   "native object-block deeper external child external-base dotted should preserve immediate child base-class identity");
            expect(child_object.source == main_path.string(),
                   "native object-block deeper external child external-base dotted should preserve the defining child-class source path");
            expect(child_object.class_library == button_library_path.string(),
                   "native object-block deeper external child external-base dotted should preserve the immediate external ClassLibrary path");
            expect(child_object.class_hierarchy.size() == 5U,
                   "native object-block deeper external child external-base dotted should preserve child class hierarchy");

            const auto parent_caption = parent_object.properties.find("caption");
            const auto child_caption = child_object.properties.find("caption");
            if (parent_caption != parent_object.properties.end())
            {
                expect(copperfin::runtime::format_value(parent_caption->second) == "Done-Saved",
                       "native object-block deeper external child external-base dotted should let dotted child method calls update the owner");
            }
            else
            {
                expect(false, "native object-block deeper external child external-base dotted should preserve the owner caption");
            }
            if (child_caption != child_object.properties.end())
            {
                expect(copperfin::runtime::format_value(child_caption->second) == "Go",
                       "native object-block deeper external child external-base dotted should preserve the child dotted assignment");
            }
            else
            {
                expect(false, "native object-block deeper external child external-base dotted should preserve the child caption");
            }
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native object-block deeper external child external-base dotted lands");
        }

        const bool has_addobject_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.addobject" &&
                   event.detail == "DemoForm.cmdsave:SaveButton";
        });
        expect(has_addobject_event,
               "native object-block deeper external child external-base dotted should emit child materialization events");

        const bool has_owner_caption_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   (event.detail == "SaveButton.OwnerCaption" ||
                    event.detail == "ParentButton.OwnerCaption");
        });
        expect(has_owner_caption_invoke_event,
               "native object-block deeper external child external-base dotted should dispatch direct dotted child method calls");

        const bool has_save_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "DemoForm.Save";
        });
        expect(has_save_invoke_event,
               "native object-block deeper external child external-base dotted should keep child THISFORM owner dispatch stable");

        fs::remove_all(temp_root, ignored);
    }

    void test_inherited_external_base_addobject_deeper_external_child_external_base_provenance_stays_coherent()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_base_inherited_addobject_deeper_external_child_external_base_provenance";
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

        const fs::path main_path = temp_root / "external_base_inherited_addobject_deeper_external_child_external_base_provenance.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oLeaf = NEWOBJECT('LeafForm')\n"
            "oChild = oCreate.cmdSave\n"
            "oLeafChild = oLeaf.cmdSave\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 81)\n"
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
            "cLeafChildClass = oLeafChild.Class\n"
            "cLeafChildBaseClass = oLeafChild.BaseClass\n"
            "cLeafChildParentClass = oLeafChild.ParentClass\n"
            "cLeafChildClassLibrary = oLeafChild.ClassLibrary\n"
            "cLeafChildClassReflect = GETPEM(oLeafChild, 'Class')\n"
            "cLeafChildBaseClassReflect = GETPEM(oLeafChild, 'BaseClass')\n"
            "cLeafChildParentClassReflect = GETPEM(oLeafChild, 'ParentClass')\n"
            "cLeafChildClassLibraryReflect = GETPEM(oLeafChild, 'ClassLibrary')\n"
            "lLeafChildHasClass = PEMSTATUS(oLeafChild, 'Class', 1)\n"
            "lLeafChildHasBaseClass = PEMSTATUS(oLeafChild, 'BaseClass', 1)\n"
            "lLeafChildHasParentClass = PEMSTATUS(oLeafChild, 'ParentClass', 1)\n"
            "lLeafChildHasClassLibrary = PEMSTATUS(oLeafChild, 'ClassLibrary', 1)\n"
            "lLeafChildClassReadOnly = PEMSTATUS(oLeafChild, 'Class', 5)\n"
            "lLeafChildBaseClassReadOnly = PEMSTATUS(oLeafChild, 'BaseClass', 5)\n"
            "lLeafChildParentClassReadOnly = PEMSTATUS(oLeafChild, 'ParentClass', 5)\n"
            "lLeafChildClassLibraryReadOnly = PEMSTATUS(oLeafChild, 'ClassLibrary', 5)\n"
            "nLeafMembersProps = AMEMBERS(aLeafMembersProps, oLeafChild, 1)\n"
            "nLeafMembersUnion = AMEMBERS(aLeafMembersUnion, oLeafChild, 3)\n"
            "cLeafProp1 = aLeafMembersProps[1]\n"
            "cLeafProp2 = aLeafMembersProps[2]\n"
            "cLeafProp3 = aLeafMembersProps[3]\n"
            "cLeafProp4 = aLeafMembersProps[4]\n"
            "cLeafProp5 = aLeafMembersProps[5]\n"
            "cLeafProp6 = aLeafMembersProps[6]\n"
            "cLeafUnion4 = aLeafMembersUnion[4]\n"
            "nLeafClassCount = ACLASS(aLeafClass, oLeafChild)\n"
            "cLeafClass1 = aLeafClass[1]\n"
            "cLeafClass2 = aLeafClass[2]\n"
            "cLeafClass3 = aLeafClass[3]\n"
            "cLeafClass4 = aLeafClass[4]\n"
            "cLeafClass5 = aLeafClass[5]\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n"
            "DEFINE CLASS LeafForm AS ChildForm\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external-base inherited ADDOBJECT deeper external child external-base provenance script should complete: ") + state.message +
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
        check("cleafchildclass", "SaveButton");
        check("cleafchildbaseclass", "ParentButton");
        check("cleafchildparentclass", "ParentButton");
        check("cleafchildclasslibrary", button_library_path.string());
        check("cleafchildclassreflect", "SaveButton");
        check("cleafchildbaseclassreflect", "ParentButton");
        check("cleafchildparentclassreflect", "ParentButton");
        check("cleafchildclasslibraryreflect", button_library_path.string());
        check("lleafchildhasclass", "true");
        check("lleafchildhasbaseclass", "true");
        check("lleafchildhasparentclass", "true");
        check("lleafchildhasclasslibrary", "true");
        check("lleafchildclassreadonly", "true");
        check("lleafchildbaseclassreadonly", "true");
        check("lleafchildparentclassreadonly", "true");
        check("lleafchildclasslibraryreadonly", "true");
        check("nleafmembersprops", "6");
        check("nleafmembersunion", "13");
        check("cleafprop1", "BASECLASS");
        check("cleafprop2", "CAPTION");
        check("cleafprop3", "CLASS");
        check("cleafprop4", "CLASSLIBRARY");
        check("cleafprop5", "PARENT");
        check("cleafprop6", "PARENTCLASS");
        check("cleafunion4", "CLASSLIBRARY");
        check("nleafclasscount", "5");
        check("cleafclass1", "SAVEBUTTON");
        check("cleafclass2", "PARENTBUTTON");
        check("cleafclass3", "ROOTBUTTON");
        check("cleafclass4", "CUSTOM");
        check("cleafclass5", "OBJECT");
        check("ldictset", "true");
        check("ndictcompare", "81");

        expect(state.ole_objects.size() == 5U,
               "external-base inherited ADDOBJECT deeper external child external-base provenance should register CREATEOBJECT parent/child, NEWOBJECT parent/child, and COM objects");
        if (state.ole_objects.size() == 5U)
        {
            const auto &create_parent = state.ole_objects[0];
            const auto &create_child = state.ole_objects[1];
            const auto &leaf_parent = state.ole_objects[2];
            const auto &leaf_child = state.ole_objects[3];
            expect(create_parent.prog_id == "ChildForm",
                   "external-base inherited ADDOBJECT deeper external child external-base provenance should preserve CREATEOBJECT parent identity");
            expect(create_child.prog_id == "SaveButton",
                   "external-base inherited ADDOBJECT deeper external child external-base provenance should preserve CREATEOBJECT child identity");
            expect(create_child.base_class_name == "ParentButton",
                   "external-base inherited ADDOBJECT deeper external child external-base provenance should preserve CREATEOBJECT immediate child base-class identity");
            expect(create_child.source == widget_library_path.string(),
                   "external-base inherited ADDOBJECT deeper external child external-base provenance should preserve the CREATEOBJECT child source path");
            expect(create_child.class_library == button_library_path.string(),
                   "external-base inherited ADDOBJECT deeper external child external-base provenance should preserve the CREATEOBJECT immediate external ClassLibrary path");
            expect(create_child.class_hierarchy.size() == 5U,
                   "external-base inherited ADDOBJECT deeper external child external-base provenance should preserve the CREATEOBJECT child class hierarchy");
            expect(!create_child.properties.contains("classlibrary"),
                   "external-base inherited ADDOBJECT deeper external child external-base provenance should not materialize a CREATEOBJECT child ClassLibrary shadow");
            expect(leaf_parent.prog_id == "LeafForm",
                   "external-base inherited ADDOBJECT deeper external child external-base provenance should preserve NEWOBJECT leaf identity");
            expect(leaf_parent.source == main_path.string(),
                   "external-base inherited ADDOBJECT deeper external child external-base provenance should preserve NEWOBJECT leaf source");
            expect(leaf_child.prog_id == "SaveButton",
                   "external-base inherited ADDOBJECT deeper external child external-base provenance should preserve NEWOBJECT leaf child identity");
            expect(leaf_child.base_class_name == "ParentButton",
                   "external-base inherited ADDOBJECT deeper external child external-base provenance should preserve NEWOBJECT leaf child immediate base-class identity");
            expect(leaf_child.source == widget_library_path.string(),
                   "external-base inherited ADDOBJECT deeper external child external-base provenance should preserve the NEWOBJECT leaf child source path");
            expect(leaf_child.class_library == button_library_path.string(),
                   "external-base inherited ADDOBJECT deeper external child external-base provenance should preserve the NEWOBJECT leaf child immediate external ClassLibrary path");
            expect(leaf_child.class_hierarchy.size() == 5U,
                   "external-base inherited ADDOBJECT deeper external child external-base provenance should preserve the NEWOBJECT leaf child class hierarchy");
            expect(!leaf_child.properties.contains("classlibrary"),
                   "external-base inherited ADDOBJECT deeper external child external-base provenance should not materialize a NEWOBJECT leaf child ClassLibrary shadow");
            expect(state.ole_objects[4].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external-base inherited ADDOBJECT deeper external child external-base provenance lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_inherited_external_base_declarative_deeper_external_child_external_base_provenance_stays_coherent()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_base_inherited_declarative_deeper_external_child_external_base_provenance";
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

        const fs::path widget_library_path = temp_root / "widgetlib.prg";
        write_text(
            widget_library_path,
            "DEFINE CLASS ParentForm AS Custom\n"
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

        const fs::path main_path = temp_root / "external_base_inherited_declarative_deeper_external_child_external_base_provenance.prg";
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
            "cCreateChildClassLibrary = oChild.ClassLibrary\n"
            "cCreateChildClassReflect = GETPEM(oChild, 'Class')\n"
            "cCreateChildBaseClassReflect = GETPEM(oChild, 'BaseClass')\n"
            "cCreateChildParentClassReflect = GETPEM(oChild, 'ParentClass')\n"
            "cCreateChildClassLibraryReflect = GETPEM(oChild, 'ClassLibrary')\n"
            "lCreateChildHasClass = PEMSTATUS(oChild, 'Class', 1)\n"
            "lCreateChildHasBaseClass = PEMSTATUS(oChild, 'BaseClass', 1)\n"
            "lCreateChildHasParentClass = PEMSTATUS(oChild, 'ParentClass', 1)\n"
            "lCreateChildHasClassLibrary = PEMSTATUS(oChild, 'ClassLibrary', 1)\n"
            "lCreateChildClassReadOnly = PEMSTATUS(oChild, 'Class', 5)\n"
            "lCreateChildBaseClassReadOnly = PEMSTATUS(oChild, 'BaseClass', 5)\n"
            "lCreateChildParentClassReadOnly = PEMSTATUS(oChild, 'ParentClass', 5)\n"
            "lCreateChildClassLibraryReadOnly = PEMSTATUS(oChild, 'ClassLibrary', 5)\n"
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
            "cCreateOwnerCaption = oChild.OwnerCaption()\n"
            "cLeafOwnerCaption = oLeafChild.OwnerCaption()\n"
            "cCreateSavedCaption = oChild.TriggerSave()\n"
            "cLeafSavedCaption = oLeafChild.TriggerSave()\n"
            "cCreateCaptionAfterSave = oCreate.Caption\n"
            "cLeafCaptionAfterSave = oLeaf.Caption\n"
            "cLeafChildClassLibrary = oLeafChild.ClassLibrary\n"
            "cLeafChildBaseClass = oLeafChild.BaseClass\n"
            "lCreateChildHasParent = PEMSTATUS(oChild, 'Parent', 1)\n"
            "lLeafChildHasParent = PEMSTATUS(oLeafChild, 'Parent', 1)\n"
            "cRootToken = oChild.RootToken()\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 158)\n"
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
               std::string("external-base inherited declarative deeper external child external-base provenance script should complete: ") + state.message +
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

        check("ccreateinitchildcaption", "Save");
        check("cleafinitchildcaption", "Save");
        check("ccreateinitownercaption", "MainForm");
        check("cleafinitownercaption", "MainForm");
        check("ccreatechildclass", "SaveButton");
        check("ccreatechildbaseclass", "ParentButton");
        check("ccreatechildparentclass", "ParentButton");
        check("ccreatechildclasslibrary", button_library_path.string());
        check("ccreatechildclassreflect", "SaveButton");
        check("ccreatechildbaseclassreflect", "ParentButton");
        check("ccreatechildparentclassreflect", "ParentButton");
        check("ccreatechildclasslibraryreflect", button_library_path.string());
        check("lcreatechildhasclass", "true");
        check("lcreatechildhasbaseclass", "true");
        check("lcreatechildhasparentclass", "true");
        check("lcreatechildhasclasslibrary", "true");
        check("lcreatechildclassreadonly", "true");
        check("lcreatechildbaseclassreadonly", "true");
        check("lcreatechildparentclassreadonly", "true");
        check("lcreatechildclasslibraryreadonly", "true");
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
        check("ccreateownercaption", "MainForm");
        check("cleafownercaption", "MainForm");
        check("ccreatesavedcaption", "MainForm-Saved");
        check("cleafsavedcaption", "MainForm-Saved");
        check("ccreatecaptionaftersave", "MainForm-Saved");
        check("cleafcaptionaftersave", "MainForm-Saved");
        check("cleafchildclasslibrary", button_library_path.string());
        check("cleafchildbaseclass", "ParentButton");
        check("lcreatechildhasparent", "true");
        check("lleafchildhasparent", "true");
        check("croottoken", "RootToken");
        check("ldictset", "true");
        check("ndictcompare", "158");

        const auto members_union = state.globals.find("nmembersunion");
        expect(members_union != state.globals.end() &&
                   std::stoi(copperfin::runtime::format_value(members_union->second)) >= 9,
               "external-base inherited declarative deeper external child external-base provenance should keep union member enumeration including inherited child methods");

        expect(state.ole_objects.size() == 5U,
               "external-base inherited declarative deeper external child external-base provenance should register CREATEOBJECT parent/child, NEWOBJECT parent/child, and COM objects");
        if (state.ole_objects.size() == 5U)
        {
            const auto &create_parent = state.ole_objects[0];
            const auto &create_child = state.ole_objects[1];
            const auto &leaf_parent = state.ole_objects[2];
            const auto &leaf_child = state.ole_objects[3];
            expect(create_parent.prog_id == "ChildForm",
                   "external-base inherited declarative deeper external child external-base provenance should preserve CREATEOBJECT parent identity");
            expect(create_parent.source == main_path.string(),
                   "external-base inherited declarative deeper external child external-base provenance should preserve CREATEOBJECT parent source");
            expect(create_child.prog_id == "SaveButton",
                   "external-base inherited declarative deeper external child external-base provenance should preserve CREATEOBJECT child identity");
            expect(create_child.base_class_name == "ParentButton",
                   "external-base inherited declarative deeper external child external-base provenance should preserve immediate child base-class identity");
            expect(create_child.source == widget_library_path.string(),
                   "external-base inherited declarative deeper external child external-base provenance should preserve the defining child-class source path");
            expect(create_child.class_library == button_library_path.string(),
                   "external-base inherited declarative deeper external child external-base provenance should preserve the immediate external ClassLibrary path");
            expect(create_child.class_hierarchy.size() == 5U,
                   "external-base inherited declarative deeper external child external-base provenance should preserve the deeper runtime child class hierarchy");
            expect(!create_child.properties.contains("classlibrary"),
                   "external-base inherited declarative deeper external child external-base provenance should not materialize a CREATEOBJECT child ClassLibrary shadow");
            expect(leaf_parent.prog_id == "LeafForm",
                   "external-base inherited declarative deeper external child external-base provenance should preserve NEWOBJECT leaf identity");
            expect(leaf_parent.source == main_path.string(),
                   "external-base inherited declarative deeper external child external-base provenance should preserve NEWOBJECT leaf source");
            expect(leaf_child.prog_id == "SaveButton",
                   "external-base inherited declarative deeper external child external-base provenance should preserve leaf child identity");
            expect(leaf_child.base_class_name == "ParentButton",
                   "external-base inherited declarative deeper external child external-base provenance should preserve the leaf child immediate external base");
            expect(leaf_child.source == widget_library_path.string(),
                   "external-base inherited declarative deeper external child external-base provenance should preserve the leaf child defining source path");
            expect(leaf_child.class_library == button_library_path.string(),
                   "external-base inherited declarative deeper external child external-base provenance should preserve the leaf child immediate external ClassLibrary path");
            expect(!leaf_child.properties.contains("classlibrary"),
                   "external-base inherited declarative deeper external child external-base provenance should not materialize a leaf child ClassLibrary shadow");
            expect(state.ole_objects[4].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external-base inherited declarative deeper external child external-base provenance lands");
        }

        const bool has_addobject_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.addobject" &&
                   (event.detail == "ChildForm.cmdsave:SaveButton" ||
                    event.detail == "LeafForm.cmdsave:SaveButton");
        });
        expect(has_addobject_event,
               "external-base inherited declarative deeper external child external-base provenance should emit child materialization events");

        const bool has_save_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "ParentForm.Save";
        });
        expect(has_save_invoke_event,
               "external-base inherited declarative deeper external child external-base provenance should keep child owner dispatch usable after ordinary reads and reflection");

        fs::remove_all(temp_root, ignored);
    }

}
