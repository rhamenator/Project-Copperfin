#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_inherited_external_parent_object_block_derived_startup_external_base_child_identity_metadata_resists_mutation()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_inherited_external_parent_object_block_derived_startup_external_base_identity";
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
            "    OBJECT cmdSave AS SaveButton\n"
            "        Caption = 'Commit'\n"
            "    ENDOBJECT\n"
            "    FUNCTION Save\n"
            "        THIS.Caption = THIS.Caption + '-Saved'\n"
            "        RETURN THIS.Caption\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "inherited_external_parent_object_block_derived_startup_external_base_identity.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oLeaf = NEWOBJECT('LeafForm')\n"
            "oCreateChild = oCreate.cmdSave\n"
            "oLeafChild = oLeaf.cmdSave\n"
            "lCreateChildClassReadOnly = PEMSTATUS(oCreateChild, 'Class', 5)\n"
            "lCreateChildBaseClassReadOnly = PEMSTATUS(oCreateChild, 'BaseClass', 5)\n"
            "lCreateChildParentClassReadOnly = PEMSTATUS(oCreateChild, 'ParentClass', 5)\n"
            "lCreateChildClassLibraryReadOnly = PEMSTATUS(oCreateChild, 'ClassLibrary', 5)\n"
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
            "cCreateChildClassLibraryAfter = oCreateChild.ClassLibrary\n"
            "cCreateChildClassReflectAfter = GETPEM(oCreateChild, 'Class')\n"
            "cCreateChildBaseClassReflectAfter = GETPEM(oCreateChild, 'BaseClass')\n"
            "cCreateChildParentClassReflectAfter = GETPEM(oCreateChild, 'ParentClass')\n"
            "cCreateChildClassLibraryReflectAfter = GETPEM(oCreateChild, 'ClassLibrary')\n"
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
            "lLeafChildClassLibraryReadOnly = PEMSTATUS(oLeafChild, 'ClassLibrary', 5)\n"
            "cLeafChildClassAfter = oLeafChild.Class\n"
            "cLeafChildBaseClassAfter = oLeafChild.BaseClass\n"
            "cLeafChildParentClassAfter = oLeafChild.ParentClass\n"
            "cLeafChildClassLibraryAfter = oLeafChild.ClassLibrary\n"
            "cLeafChildClassReflectAfter = GETPEM(oLeafChild, 'Class')\n"
            "cLeafChildBaseClassReflectAfter = GETPEM(oLeafChild, 'BaseClass')\n"
            "cLeafChildParentClassReflectAfter = GETPEM(oLeafChild, 'ParentClass')\n"
            "cLeafChildClassLibraryReflectAfter = GETPEM(oLeafChild, 'ClassLibrary')\n"
            "cCreateOwnerCaption = oCreateChild.OwnerCaption()\n"
            "cLeafOwnerCaption = oLeafChild.OwnerCaption()\n"
            "cCreateSavedCaption = oCreateChild.TriggerSave()\n"
            "cLeafSavedCaption = oLeafChild.TriggerSave()\n"
            "cCreateCaptionAfterSave = oCreate.Caption\n"
            "cLeafCaptionAfterSave = oLeaf.Caption\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 130)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n"
            "DEFINE CLASS LeafForm AS ChildForm\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS ParentButton OF buttons.prg\n"
            "    Caption = 'Save'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("inherited external-parent object-block derived-startup external-base identity script should complete: ") + state.message +
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
        check("lcreatechildclasslibraryreadonly", "true");
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
        check("ccreatechildclasslibraryafter", button_library_path.string());
        check("ccreatechildclassreflectafter", "SaveButton");
        check("ccreatechildbaseclassreflectafter", "ParentButton");
        check("ccreatechildparentclassreflectafter", "ParentButton");
        check("ccreatechildclasslibraryreflectafter", button_library_path.string());
        check("lremoveleafchildclass", "false");
        check("lremoveleafchildbaseclass", "false");
        check("lremoveleafchildparentclass", "false");
        check("lremoveleafchildclasslibrary", "false");
        check("lleafchildclassreadonly", "true");
        check("lleafchildbaseclassreadonly", "true");
        check("lleafchildparentclassreadonly", "true");
        check("lleafchildclasslibraryreadonly", "true");
        check("cleafchildclassafter", "SaveButton");
        check("cleafchildbaseclassafter", "ParentButton");
        check("cleafchildparentclassafter", "ParentButton");
        check("cleafchildclasslibraryafter", button_library_path.string());
        check("cleafchildclassreflectafter", "SaveButton");
        check("cleafchildbaseclassreflectafter", "ParentButton");
        check("cleafchildparentclassreflectafter", "ParentButton");
        check("cleafchildclasslibraryreflectafter", button_library_path.string());
        check("ccreateownercaption", "MainForm");
        check("cleafownercaption", "MainForm");
        check("ccreatesavedcaption", "MainForm-Saved");
        check("cleafsavedcaption", "MainForm-Saved");
        check("ccreatecaptionaftersave", "MainForm-Saved");
        check("cleafcaptionaftersave", "MainForm-Saved");
        check("ldictset", "true");
        check("ndictcompare", "130");

        expect(state.ole_objects.size() == 5U,
               "inherited external-parent object-block derived-startup external-base identity should register CREATEOBJECT parent/child, NEWOBJECT parent/child, and COM objects");
        if (state.ole_objects.size() == 5U)
        {
            const auto &create_parent = state.ole_objects[0];
            const auto &create_child = state.ole_objects[1];
            const auto &leaf_parent = state.ole_objects[2];
            const auto &leaf_child = state.ole_objects[3];
            expect(create_parent.prog_id == "ChildForm",
                   "inherited external-parent object-block derived-startup external-base identity should preserve CREATEOBJECT parent identity");
            expect(create_child.prog_id == "SaveButton",
                   "inherited external-parent object-block derived-startup external-base identity should preserve CREATEOBJECT child identity");
            expect(create_child.base_class_name == "ParentButton",
                   "inherited external-parent object-block derived-startup external-base identity should preserve the CREATEOBJECT child immediate external base");
            expect(create_child.source == main_path.string(),
                   "inherited external-parent object-block derived-startup external-base identity should preserve CREATEOBJECT child source identity");
            expect(create_child.class_library == button_library_path.string(),
                   "inherited external-parent object-block derived-startup external-base identity should preserve CREATEOBJECT child ClassLibrary path");
            expect(create_child.class_hierarchy.size() == 5U,
                   "inherited external-parent object-block derived-startup external-base identity should preserve CREATEOBJECT child class hierarchy");
            expect(!create_child.properties.contains("class"),
                   "inherited external-parent object-block derived-startup external-base identity should not materialize a CREATEOBJECT child Class shadow");
            expect(!create_child.properties.contains("baseclass"),
                   "inherited external-parent object-block derived-startup external-base identity should not materialize a CREATEOBJECT child BaseClass shadow");
            expect(!create_child.properties.contains("parentclass"),
                   "inherited external-parent object-block derived-startup external-base identity should not materialize a CREATEOBJECT child ParentClass shadow");
            expect(!create_child.properties.contains("classlibrary"),
                   "inherited external-parent object-block derived-startup external-base identity should not materialize a CREATEOBJECT child ClassLibrary shadow");
            expect(leaf_parent.prog_id == "LeafForm",
                   "inherited external-parent object-block derived-startup external-base identity should preserve NEWOBJECT leaf identity");
            expect(leaf_child.prog_id == "SaveButton",
                   "inherited external-parent object-block derived-startup external-base identity should preserve leaf child identity");
            expect(leaf_child.base_class_name == "ParentButton",
                   "inherited external-parent object-block derived-startup external-base identity should preserve the leaf child immediate external base");
            expect(leaf_child.source == main_path.string(),
                   "inherited external-parent object-block derived-startup external-base identity should preserve leaf child source identity");
            expect(leaf_child.class_library == button_library_path.string(),
                   "inherited external-parent object-block derived-startup external-base identity should preserve leaf child ClassLibrary path");
            expect(leaf_child.class_hierarchy.size() == 5U,
                   "inherited external-parent object-block derived-startup external-base identity should preserve leaf child class hierarchy");
            expect(!leaf_child.properties.contains("class"),
                   "inherited external-parent object-block derived-startup external-base identity should not materialize a leaf child Class shadow");
            expect(!leaf_child.properties.contains("baseclass"),
                   "inherited external-parent object-block derived-startup external-base identity should not materialize a leaf child BaseClass shadow");
            expect(!leaf_child.properties.contains("parentclass"),
                   "inherited external-parent object-block derived-startup external-base identity should not materialize a leaf child ParentClass shadow");
            expect(!leaf_child.properties.contains("classlibrary"),
                   "inherited external-parent object-block derived-startup external-base identity should not materialize a leaf child ClassLibrary shadow");
            expect(state.ole_objects[4].prog_id == "Scripting.Dictionary",
                   "COM behavior should remain stable while inherited external-parent object-block derived-startup external-base identity lands");
        }

        const bool has_save_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "ParentForm.Save";
        });
        expect(has_save_invoke_event,
               "inherited external-parent object-block derived-startup external-base identity should keep child owner dispatch usable after rejected mutations");

        fs::remove_all(temp_root, ignored);
    }

    void test_inherited_external_parent_object_block_derived_startup_external_base_dotted_access_resolves_live_child_chain()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_inherited_external_parent_object_block_derived_startup_external_base_dotted";
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
            "    OBJECT cmdSave AS SaveButton\n"
            "        Caption = 'Commit'\n"
            "    ENDOBJECT\n"
            "    FUNCTION Save\n"
            "        THIS.Caption = THIS.Caption + '-Saved'\n"
            "        RETURN THIS.Caption\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "inherited_external_parent_object_block_derived_startup_external_base_dotted.prg";
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
            "cCreateChildClassLibrary = oCreate.cmdSave.ClassLibrary\n"
            "cLeafChildClassLibrary = oLeaf.cmdSave.ClassLibrary\n"
            "lCreateChildHasParent = PEMSTATUS(oCreate.cmdSave, 'Parent', 1)\n"
            "lLeafChildHasParent = PEMSTATUS(oLeaf.cmdSave, 'Parent', 1)\n"
            "cCreateRootToken = oCreate.cmdSave.RootToken()\n"
            "cLeafRootToken = oLeaf.cmdSave.RootToken()\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 137)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n"
            "DEFINE CLASS LeafForm AS ChildForm\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS ParentButton OF buttons.prg\n"
            "    Caption = 'Save'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("inherited external-parent object-block derived-startup external-base dotted script should complete: ") + state.message +
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
        check("ccreatechildclasslibrary", button_library_path.string());
        check("cleafchildclasslibrary", button_library_path.string());
        check("lcreatechildhasparent", "true");
        check("lleafchildhasparent", "true");
        check("ccreateroottoken", "RootToken");
        check("cleafroottoken", "RootToken");
        check("ldictset", "true");
        check("ndictcompare", "137");

        expect(state.ole_objects.size() == 5U,
               "inherited external-parent object-block derived-startup external-base dotted script should register CREATEOBJECT parent/child, NEWOBJECT parent/child, and COM objects");
        if (state.ole_objects.size() == 5U)
        {
            const auto &create_parent = state.ole_objects[0];
            const auto &create_child = state.ole_objects[1];
            const auto &leaf_parent = state.ole_objects[2];
            const auto &leaf_child = state.ole_objects[3];
            expect(create_parent.prog_id == "ChildForm",
                   "inherited external-parent object-block derived-startup external-base dotted access should preserve CREATEOBJECT parent identity");
            expect(create_parent.source == main_path.string(),
                   "inherited external-parent object-block derived-startup external-base dotted access should preserve CREATEOBJECT parent source");
            expect(create_child.prog_id == "SaveButton",
                   "inherited external-parent object-block derived-startup external-base dotted access should preserve CREATEOBJECT child identity");
            expect(create_child.base_class_name == "ParentButton",
                   "inherited external-parent object-block derived-startup external-base dotted access should preserve the CREATEOBJECT child immediate external base");
            expect(create_child.source == main_path.string(),
                   "inherited external-parent object-block derived-startup external-base dotted access should preserve CREATEOBJECT child derived-startup source identity");
            expect(create_child.class_library == button_library_path.string(),
                   "inherited external-parent object-block derived-startup external-base dotted access should preserve CREATEOBJECT child ClassLibrary path");
            expect(leaf_parent.prog_id == "LeafForm",
                   "inherited external-parent object-block derived-startup external-base dotted access should preserve NEWOBJECT leaf identity");
            expect(leaf_parent.source == main_path.string(),
                   "inherited external-parent object-block derived-startup external-base dotted access should preserve NEWOBJECT leaf source");
            expect(leaf_child.prog_id == "SaveButton",
                   "inherited external-parent object-block derived-startup external-base dotted access should preserve leaf child identity");
            expect(leaf_child.base_class_name == "ParentButton",
                   "inherited external-parent object-block derived-startup external-base dotted access should preserve the leaf child immediate external base");
            expect(leaf_child.source == main_path.string(),
                   "inherited external-parent object-block derived-startup external-base dotted access should preserve leaf child derived-startup source identity");
            expect(leaf_child.class_library == button_library_path.string(),
                   "inherited external-parent object-block derived-startup external-base dotted access should preserve leaf child ClassLibrary path");

            const auto create_parent_caption = create_parent.properties.find("caption");
            const auto create_child_caption = create_child.properties.find("caption");
            const auto leaf_parent_caption = leaf_parent.properties.find("caption");
            const auto leaf_child_caption = leaf_child.properties.find("caption");
            if (create_parent_caption != create_parent.properties.end())
            {
                expect(copperfin::runtime::format_value(create_parent_caption->second) == "Done-Saved",
                       "inherited external-parent object-block derived-startup external-base dotted access should let dotted child method calls update the CREATEOBJECT owner");
            }
            else
            {
                expect(false, "inherited external-parent object-block derived-startup external-base dotted access should preserve the CREATEOBJECT owner caption");
            }
            if (create_child_caption != create_child.properties.end())
            {
                expect(copperfin::runtime::format_value(create_child_caption->second) == "Go",
                       "inherited external-parent object-block derived-startup external-base dotted access should preserve the CREATEOBJECT child dotted assignment");
            }
            else
            {
                expect(false, "inherited external-parent object-block derived-startup external-base dotted access should preserve the CREATEOBJECT child caption");
            }
            if (leaf_parent_caption != leaf_parent.properties.end())
            {
                expect(copperfin::runtime::format_value(leaf_parent_caption->second) == "Ready-Saved",
                       "inherited external-parent object-block derived-startup external-base dotted access should let dotted child method calls update the leaf owner");
            }
            else
            {
                expect(false, "inherited external-parent object-block derived-startup external-base dotted access should preserve the leaf owner caption");
            }
            if (leaf_child_caption != leaf_child.properties.end())
            {
                expect(copperfin::runtime::format_value(leaf_child_caption->second) == "Ship",
                       "inherited external-parent object-block derived-startup external-base dotted access should preserve the leaf child dotted assignment");
            }
            else
            {
                expect(false, "inherited external-parent object-block derived-startup external-base dotted access should preserve the leaf child caption");
            }
            expect(state.ole_objects[4].prog_id == "Scripting.Dictionary",
                   "COM behavior should remain stable while inherited external-parent object-block derived-startup external-base dotted access lands");
        }

        const bool has_addobject_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.addobject" &&
                   (event.detail == "ChildForm.cmdsave:SaveButton" ||
                    event.detail == "LeafForm.cmdsave:SaveButton");
        });
        expect(has_addobject_event,
               "inherited external-parent object-block derived-startup external-base dotted access should emit child materialization events");

        const bool has_owner_caption_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   (event.detail == "SaveButton.OwnerCaption" ||
                    event.detail == "ParentButton.OwnerCaption");
        });
        expect(has_owner_caption_invoke_event,
               "inherited external-parent object-block derived-startup external-base dotted access should dispatch direct dotted child method calls");

        const bool has_save_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "ParentForm.Save";
        });
        expect(has_save_invoke_event,
               "inherited external-parent object-block derived-startup external-base dotted access should keep child THISFORM owner dispatch stable");

        fs::remove_all(temp_root, ignored);
    }

    void test_inherited_external_base_addobject_external_child_base_surfaces_classlibrary_provenance()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_base_inherited_addobject_external_child_base_provenance";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path root_library_path = temp_root / "rootbuttons.prg";
        write_text(
            root_library_path,
            "DEFINE CLASS ParentButton AS Custom\n"
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

        const fs::path button_library_path = temp_root / "buttons.prg";
        write_text(
            button_library_path,
            "DEFINE CLASS SaveButton AS ParentButton OF rootbuttons.prg\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_base_inherited_addobject_external_child_base_provenance.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oChild = oCreate.cmdSave\n"
            "cChildClassLibrary = GETPEM(oChild, 'ClassLibrary')\n"
            "lChildHasClassLibrary = PEMSTATUS(oChild, 'ClassLibrary', 1)\n"
            "lChildClassLibraryReadOnly = PEMSTATUS(oChild, 'ClassLibrary', 5)\n"
            "cChildClassLibraryProp = oChild.ClassLibrary\n"
            "nMembersProps = AMEMBERS(aMembersProps, oChild, 1)\n"
            "cProp3 = aMembersProps[3]\n"
            "lSetChildClassLibrary = SETPEM(oChild, 'ClassLibrary', 'shadow.prg')\n"
            "oChild.ClassLibrary = 'shadow2.prg'\n"
            "cChildClassLibraryAfter = GETPEM(oChild, 'ClassLibrary')\n"
            "cChildClassLibraryPropAfter = oChild.ClassLibrary\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 69)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external-base inherited ADDOBJECT external child-base provenance script should complete: ") + state.message +
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

        check("cchildclasslibrary", root_library_path.string());
        check("lchildhasclasslibrary", "true");
        check("lchildclasslibraryreadonly", "true");
        check("cchildclasslibraryprop", root_library_path.string());
        check("cprop3", "CLASSLIBRARY");
        check("lsetchildclasslibrary", "false");
        check("cchildclasslibraryafter", root_library_path.string());
        check("cchildclasslibrarypropafter", root_library_path.string());
        check("ldictset", "true");
        check("ndictcompare", "69");

        expect(state.ole_objects.size() == 3U,
               "external-base inherited ADDOBJECT external child-base provenance should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &parent_object = state.ole_objects[0];
            const auto &child_object = state.ole_objects[1];
            expect(parent_object.prog_id == "ChildForm",
                   "external-base inherited ADDOBJECT external child-base provenance should preserve parent identity");
            expect(child_object.prog_id == "SaveButton",
                   "external-base inherited ADDOBJECT external child-base provenance should preserve child identity");
            expect(child_object.source == button_library_path.string(),
                   "external-base inherited ADDOBJECT external child-base provenance should preserve the child definition source path");
            expect(child_object.class_library == root_library_path.string(),
                   "external-base inherited ADDOBJECT external child-base provenance should preserve the external child ClassLibrary path");
            expect(!child_object.properties.contains("classlibrary"),
                   "external-base inherited ADDOBJECT external child-base provenance should not materialize a ClassLibrary shadow");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external-base inherited ADDOBJECT external child-base provenance lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_inherited_external_base_addobject_external_child_base_aclass_reflects_inheritance_chain()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_base_inherited_addobject_external_child_base_aclass";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path root_library_path = temp_root / "rootbuttons.prg";
        write_text(
            root_library_path,
            "DEFINE CLASS ParentButton AS Custom\n"
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

        const fs::path button_library_path = temp_root / "buttons.prg";
        write_text(
            button_library_path,
            "DEFINE CLASS SaveButton AS ParentButton OF rootbuttons.prg\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_base_inherited_addobject_external_child_base_aclass.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oChild = oCreate.cmdSave\n"
            "nClassCount = ACLASS(aClass, oChild)\n"
            "cClass1 = aClass[1]\n"
            "cClass2 = aClass[2]\n"
            "cClass3 = aClass[3]\n"
            "cClass4 = aClass[4]\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external-base inherited ADDOBJECT external child-base ACLASS script should complete: ") + state.message +
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

        check("nclasscount", "4");
        check("cclass1", "SAVEBUTTON");
        check("cclass2", "PARENTBUTTON");
        check("cclass3", "CUSTOM");
        check("cclass4", "OBJECT");

        expect(state.ole_objects.size() == 2U,
               "external-base inherited ADDOBJECT external child-base ACLASS should register parent and child objects");
        if (state.ole_objects.size() == 2U)
        {
            const auto &parent_object = state.ole_objects[0];
            const auto &child_object = state.ole_objects[1];
            expect(parent_object.prog_id == "ChildForm",
                   "external-base inherited ADDOBJECT external child-base ACLASS should preserve parent identity");
            expect(child_object.prog_id == "SaveButton",
                   "external-base inherited ADDOBJECT external child-base ACLASS should preserve child identity");
            expect(child_object.source == button_library_path.string(),
                   "external-base inherited ADDOBJECT external child-base ACLASS should preserve child source path");
            expect(child_object.class_library == root_library_path.string(),
                   "external-base inherited ADDOBJECT external child-base ACLASS should preserve external child ClassLibrary path");
            expect(child_object.class_hierarchy.size() == 4U,
                   "external-base inherited ADDOBJECT external child-base ACLASS should preserve runtime child class hierarchy");
            if (child_object.class_hierarchy.size() == 4U)
            {
                expect(child_object.class_hierarchy[0] == "SAVEBUTTON",
                       "external-base inherited ADDOBJECT external child-base ACLASS should store the derived child class first");
                expect(child_object.class_hierarchy[1] == "PARENTBUTTON",
                       "external-base inherited ADDOBJECT external child-base ACLASS should store the external parent class second");
                expect(child_object.class_hierarchy[2] == "CUSTOM",
                       "external-base inherited ADDOBJECT external child-base ACLASS should store the builtin base token");
                expect(child_object.class_hierarchy[3] == "OBJECT",
                       "external-base inherited ADDOBJECT external child-base ACLASS should store the terminal object token");
            }
        }

        fs::remove_all(temp_root, ignored);
    }

}
