#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_inherited_external_parent_declarative_derived_startup_child_identity_metadata_resists_mutation()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_inherited_external_parent_declarative_derived_startup_child_identity";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path widget_library_path = temp_root / "widgetlib.prg";
        write_text(
            widget_library_path,
            "DEFINE CLASS ParentForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "    FUNCTION Save\n"
            "        THIS.Caption = THIS.Caption + '-Saved'\n"
            "        RETURN THIS.Caption\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "inherited_external_parent_declarative_derived_startup_child_identity.prg";
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
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 131)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n"
            "DEFINE CLASS LeafForm AS ChildForm\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
            "    Caption = 'Save'\n"
            "    FUNCTION OwnerCaption\n"
            "        RETURN PARENT.Caption\n"
            "    ENDFUNC\n"
            "    FUNCTION TriggerSave\n"
            "        RETURN THISFORM.Save()\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("inherited external-parent declarative derived-startup child identity script should complete: ") + state.message +
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
        check("ccreatechildbaseclassafter", "Custom");
        check("ccreatechildparentclassafter", "Custom");
        check("ccreatechildclassreflectafter", "SaveButton");
        check("ccreatechildbaseclassreflectafter", "Custom");
        check("ccreatechildparentclassreflectafter", "Custom");
        check("lremoveleafchildclass", "false");
        check("lremoveleafchildbaseclass", "false");
        check("lremoveleafchildparentclass", "false");
        check("lremoveleafchildclasslibrary", "false");
        check("lleafchildclassreadonly", "true");
        check("lleafchildbaseclassreadonly", "true");
        check("lleafchildparentclassreadonly", "true");
        check("cleafchildclassafter", "SaveButton");
        check("cleafchildbaseclassafter", "Custom");
        check("cleafchildparentclassafter", "Custom");
        check("cleafchildclassreflectafter", "SaveButton");
        check("cleafchildbaseclassreflectafter", "Custom");
        check("cleafchildparentclassreflectafter", "Custom");
        check("ccreateownercaption", "MainForm");
        check("cleafownercaption", "MainForm");
        check("ccreatesavedcaption", "MainForm-Saved");
        check("cleafsavedcaption", "MainForm-Saved");
        check("ccreatecaptionaftersave", "MainForm-Saved");
        check("cleafcaptionaftersave", "MainForm-Saved");
        check("ldictset", "true");
        check("ndictcompare", "131");

        const auto create_child_class_library_after = state.globals.find("xcreatechildclasslibraryafter");
        expect(create_child_class_library_after != state.globals.end() &&
                   create_child_class_library_after->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited external-parent declarative derived-startup child identity should leave the CREATEOBJECT child ClassLibrary empty after failed mutation");

        const auto create_child_class_library_reflect_after = state.globals.find("xcreatechildclasslibraryreflectafter");
        expect(create_child_class_library_reflect_after != state.globals.end() &&
                   create_child_class_library_reflect_after->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited external-parent declarative derived-startup child identity should leave the CREATEOBJECT child ClassLibrary empty through GETPEM() after failed mutation");

        const auto leaf_child_class_library_after = state.globals.find("xleafchildclasslibraryafter");
        expect(leaf_child_class_library_after != state.globals.end() &&
                   leaf_child_class_library_after->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited external-parent declarative derived-startup child identity should leave the leaf child ClassLibrary empty after rejected mutation");

        const auto leaf_child_class_library_reflect_after = state.globals.find("xleafchildclasslibraryreflectafter");
        expect(leaf_child_class_library_reflect_after != state.globals.end() &&
                   leaf_child_class_library_reflect_after->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited external-parent declarative derived-startup child identity should leave the leaf child ClassLibrary empty through GETPEM() after rejected mutation");

        expect(state.ole_objects.size() == 5U,
               "inherited external-parent declarative derived-startup child identity should register CREATEOBJECT parent/child, NEWOBJECT parent/child, and COM objects");
        if (state.ole_objects.size() == 5U)
        {
            const auto &create_parent = state.ole_objects[0];
            const auto &create_child = state.ole_objects[1];
            const auto &leaf_parent = state.ole_objects[2];
            const auto &leaf_child = state.ole_objects[3];
            expect(create_parent.prog_id == "ChildForm",
                   "inherited external-parent declarative derived-startup child identity should preserve CREATEOBJECT parent identity");
            expect(create_child.prog_id == "SaveButton",
                   "inherited external-parent declarative derived-startup child identity should preserve CREATEOBJECT child identity");
            expect(create_child.base_class_name == "Custom",
                   "inherited external-parent declarative derived-startup child identity should preserve the CREATEOBJECT child builtin base");
            expect(create_child.source == main_path.string(),
                   "inherited external-parent declarative derived-startup child identity should preserve CREATEOBJECT child source identity");
            expect(create_child.class_library.empty(),
                   "inherited external-parent declarative derived-startup child identity should preserve empty CREATEOBJECT child ClassLibrary semantics");
            expect(create_child.class_hierarchy.size() == 3U,
                   "inherited external-parent declarative derived-startup child identity should preserve CREATEOBJECT child class hierarchy");
            expect(!create_child.properties.contains("class"),
                   "inherited external-parent declarative derived-startup child identity should not materialize a CREATEOBJECT child Class shadow");
            expect(!create_child.properties.contains("baseclass"),
                   "inherited external-parent declarative derived-startup child identity should not materialize a CREATEOBJECT child BaseClass shadow");
            expect(!create_child.properties.contains("parentclass"),
                   "inherited external-parent declarative derived-startup child identity should not materialize a CREATEOBJECT child ParentClass shadow");
            expect(!create_child.properties.contains("classlibrary"),
                   "inherited external-parent declarative derived-startup child identity should not materialize a CREATEOBJECT child ClassLibrary shadow");
            expect(leaf_parent.prog_id == "LeafForm",
                   "inherited external-parent declarative derived-startup child identity should preserve NEWOBJECT leaf identity");
            expect(leaf_child.prog_id == "SaveButton",
                   "inherited external-parent declarative derived-startup child identity should preserve leaf child identity");
            expect(leaf_child.base_class_name == "Custom",
                   "inherited external-parent declarative derived-startup child identity should preserve the leaf child builtin base");
            expect(leaf_child.source == main_path.string(),
                   "inherited external-parent declarative derived-startup child identity should preserve leaf child source identity");
            expect(leaf_child.class_library.empty(),
                   "inherited external-parent declarative derived-startup child identity should preserve empty leaf child ClassLibrary semantics");
            expect(leaf_child.class_hierarchy.size() == 3U,
                   "inherited external-parent declarative derived-startup child identity should preserve leaf child class hierarchy");
            expect(!leaf_child.properties.contains("class"),
                   "inherited external-parent declarative derived-startup child identity should not materialize a leaf child Class shadow");
            expect(!leaf_child.properties.contains("baseclass"),
                   "inherited external-parent declarative derived-startup child identity should not materialize a leaf child BaseClass shadow");
            expect(!leaf_child.properties.contains("parentclass"),
                   "inherited external-parent declarative derived-startup child identity should not materialize a leaf child ParentClass shadow");
            expect(!leaf_child.properties.contains("classlibrary"),
                   "inherited external-parent declarative derived-startup child identity should not materialize a leaf child ClassLibrary shadow");
            expect(state.ole_objects[4].prog_id == "Scripting.Dictionary",
                   "COM behavior should remain stable while inherited external-parent declarative derived-startup child identity lands");
        }

        const bool has_save_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "ParentForm.Save";
        });
        expect(has_save_invoke_event,
               "inherited external-parent declarative derived-startup child identity should keep child owner dispatch usable after rejected mutations");

        fs::remove_all(temp_root, ignored);
    }

    void test_inherited_external_parent_declarative_derived_startup_child_dotted_access_resolves_live_child_chain()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_inherited_external_parent_declarative_derived_startup_child_dotted";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path widget_library_path = temp_root / "widgetlib.prg";
        write_text(
            widget_library_path,
            "DEFINE CLASS ParentForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "    FUNCTION Save\n"
            "        THIS.Caption = THIS.Caption + '-Saved'\n"
            "        RETURN THIS.Caption\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "inherited_external_parent_declarative_derived_startup_child_dotted.prg";
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
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 137)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n"
            "DEFINE CLASS LeafForm AS ChildForm\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
            "    Caption = 'Save'\n"
            "    FUNCTION OwnerCaption\n"
            "        RETURN PARENT.Caption\n"
            "    ENDFUNC\n"
            "    FUNCTION TriggerSave\n"
            "        RETURN THISFORM.Save()\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("inherited external-parent declarative derived-startup child dotted script should complete: ") + state.message +
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

        check("ccreatechildcaptionbefore", "Save");
        check("cleafchildcaptionbefore", "Save");
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
        check("ccreatechildbaseclass", "Custom");
        check("cleafchildbaseclass", "Custom");
        check("lcreatechildhasparent", "true");
        check("lleafchildhasparent", "true");
        check("ldictset", "true");
        check("ndictcompare", "137");

        const auto create_child_class_library = state.globals.find("xcreatechildclasslibrary");
        expect(create_child_class_library != state.globals.end() &&
                   create_child_class_library->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited external-parent declarative derived-startup child dotted access should leave the CREATEOBJECT child ClassLibrary empty");

        const auto leaf_child_class_library = state.globals.find("xleafchildclasslibrary");
        expect(leaf_child_class_library != state.globals.end() &&
                   leaf_child_class_library->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited external-parent declarative derived-startup child dotted access should leave the leaf child ClassLibrary empty");

        expect(state.ole_objects.size() == 5U,
               "inherited external-parent declarative derived-startup child dotted script should register CREATEOBJECT parent/child, NEWOBJECT parent/child, and COM objects");
        if (state.ole_objects.size() == 5U)
        {
            const auto &create_parent = state.ole_objects[0];
            const auto &create_child = state.ole_objects[1];
            const auto &leaf_parent = state.ole_objects[2];
            const auto &leaf_child = state.ole_objects[3];
            expect(create_parent.prog_id == "ChildForm",
                   "inherited external-parent declarative derived-startup child dotted access should preserve CREATEOBJECT parent identity");
            expect(create_parent.source == main_path.string(),
                   "inherited external-parent declarative derived-startup child dotted access should preserve CREATEOBJECT parent source");
            expect(create_child.prog_id == "SaveButton",
                   "inherited external-parent declarative derived-startup child dotted access should preserve CREATEOBJECT child identity");
            expect(create_child.base_class_name == "Custom",
                   "inherited external-parent declarative derived-startup child dotted access should preserve the CREATEOBJECT child builtin base");
            expect(create_child.source == main_path.string(),
                   "inherited external-parent declarative derived-startup child dotted access should preserve CREATEOBJECT child derived-startup source identity");
            expect(create_child.class_library.empty(),
                   "inherited external-parent declarative derived-startup child dotted access should preserve empty CREATEOBJECT child ClassLibrary semantics");
            expect(leaf_parent.prog_id == "LeafForm",
                   "inherited external-parent declarative derived-startup child dotted access should preserve NEWOBJECT leaf identity");
            expect(leaf_parent.source == main_path.string(),
                   "inherited external-parent declarative derived-startup child dotted access should preserve NEWOBJECT leaf source");
            expect(leaf_child.prog_id == "SaveButton",
                   "inherited external-parent declarative derived-startup child dotted access should preserve leaf child identity");
            expect(leaf_child.base_class_name == "Custom",
                   "inherited external-parent declarative derived-startup child dotted access should preserve the leaf child builtin base");
            expect(leaf_child.source == main_path.string(),
                   "inherited external-parent declarative derived-startup child dotted access should preserve leaf child derived-startup source identity");
            expect(leaf_child.class_library.empty(),
                   "inherited external-parent declarative derived-startup child dotted access should preserve empty leaf child ClassLibrary semantics");

            const auto create_parent_caption = create_parent.properties.find("caption");
            const auto create_child_caption = create_child.properties.find("caption");
            const auto leaf_parent_caption = leaf_parent.properties.find("caption");
            const auto leaf_child_caption = leaf_child.properties.find("caption");
            if (create_parent_caption != create_parent.properties.end())
            {
                expect(copperfin::runtime::format_value(create_parent_caption->second) == "Done-Saved",
                       "inherited external-parent declarative derived-startup child dotted access should let dotted child method calls update the CREATEOBJECT owner");
            }
            else
            {
                expect(false, "inherited external-parent declarative derived-startup child dotted access should preserve the CREATEOBJECT owner caption");
            }
            if (create_child_caption != create_child.properties.end())
            {
                expect(copperfin::runtime::format_value(create_child_caption->second) == "Go",
                       "inherited external-parent declarative derived-startup child dotted access should preserve the CREATEOBJECT child dotted assignment");
            }
            else
            {
                expect(false, "inherited external-parent declarative derived-startup child dotted access should preserve the CREATEOBJECT child caption");
            }
            if (leaf_parent_caption != leaf_parent.properties.end())
            {
                expect(copperfin::runtime::format_value(leaf_parent_caption->second) == "Ready-Saved",
                       "inherited external-parent declarative derived-startup child dotted access should let dotted child method calls update the leaf owner");
            }
            else
            {
                expect(false, "inherited external-parent declarative derived-startup child dotted access should preserve the leaf owner caption");
            }
            if (leaf_child_caption != leaf_child.properties.end())
            {
                expect(copperfin::runtime::format_value(leaf_child_caption->second) == "Ship",
                       "inherited external-parent declarative derived-startup child dotted access should preserve the leaf child dotted assignment");
            }
            else
            {
                expect(false, "inherited external-parent declarative derived-startup child dotted access should preserve the leaf child caption");
            }
            expect(state.ole_objects[4].prog_id == "Scripting.Dictionary",
                   "COM behavior should remain stable while inherited external-parent declarative derived-startup child dotted access lands");
        }

        const bool has_addobject_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.addobject" &&
                   (event.detail == "ChildForm.cmdsave:SaveButton" ||
                    event.detail == "LeafForm.cmdsave:SaveButton");
        });
        expect(has_addobject_event,
               "inherited external-parent declarative derived-startup child dotted access should emit child materialization events");

        const bool has_owner_caption_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "SaveButton.OwnerCaption";
        });
        expect(has_owner_caption_invoke_event,
               "inherited external-parent declarative derived-startup child dotted access should dispatch direct dotted child method calls");

        const bool has_save_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "ParentForm.Save";
        });
        expect(has_save_invoke_event,
               "inherited external-parent declarative derived-startup child dotted access should keep child THISFORM owner dispatch stable");

        fs::remove_all(temp_root, ignored);
    }

    void test_inherited_external_parent_declarative_derived_startup_child_preserves_external_base_provenance()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_inherited_external_parent_declarative_derived_startup_external_base";
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
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "inherited_external_parent_declarative_derived_startup_external_base.prg";
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
            "cUnion4 = aMembersUnion[4]\n"
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
            "cLeafChildClassLibrary = oLeafChild.ClassLibrary\n"
            "cLeafChildBaseClass = oLeafChild.BaseClass\n"
            "cLeafChildParentClass = oLeafChild.ParentClass\n"
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
            "nLeafClassCount = ACLASS(aLeafClass, oLeafChild)\n"
            "cLeafClass1 = aLeafClass[1]\n"
            "cLeafClass2 = aLeafClass[2]\n"
            "cLeafClass3 = aLeafClass[3]\n"
            "cLeafClass4 = aLeafClass[4]\n"
            "cLeafClass5 = aLeafClass[5]\n"
            "lCreateChildHasParent = PEMSTATUS(oChild, 'Parent', 1)\n"
            "lLeafChildHasParent = PEMSTATUS(oLeafChild, 'Parent', 1)\n"
            "cRootToken = oChild.RootToken()\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 131)\n"
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
               std::string("inherited external-parent declarative derived-startup external-base script should complete: ") + state.message +
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
        check("cleafchildclass", "SaveButton");
        check("cleafchildclasslibrary", button_library_path.string());
        check("cleafchildbaseclass", "ParentButton");
        check("cleafchildparentclass", "ParentButton");
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
        check("cleafprop1", "BASECLASS");
        check("cleafprop2", "CAPTION");
        check("cleafprop3", "CLASS");
        check("cleafprop4", "CLASSLIBRARY");
        check("cleafprop5", "PARENT");
        check("cleafprop6", "PARENTCLASS");
        check("nleafclasscount", "5");
        check("cleafclass1", "SAVEBUTTON");
        check("cleafclass2", "PARENTBUTTON");
        check("cleafclass3", "ROOTBUTTON");
        check("cleafclass4", "CUSTOM");
        check("cleafclass5", "OBJECT");
        check("lcreatechildhasparent", "true");
        check("lleafchildhasparent", "true");
        check("croottoken", "RootToken");
        check("ldictset", "true");
        check("ndictcompare", "131");

        const auto leaf_members_union = state.globals.find("nleafmembersunion");
        expect(leaf_members_union != state.globals.end() &&
                   std::stoi(copperfin::runtime::format_value(leaf_members_union->second)) >= 9,
               "inherited external-parent declarative derived-startup external-base should keep leaf union member enumeration including inherited child methods");

        expect(state.ole_objects.size() == 5U,
               "inherited external-parent declarative derived-startup external-base script should register CREATEOBJECT parent/child, NEWOBJECT parent/child, and COM objects");
        if (state.ole_objects.size() == 5U)
        {
            const auto &create_parent = state.ole_objects[0];
            const auto &create_child = state.ole_objects[1];
            const auto &leaf_parent = state.ole_objects[2];
            const auto &leaf_child = state.ole_objects[3];
            expect(create_parent.prog_id == "ChildForm",
                   "inherited external-parent declarative derived-startup external-base should preserve CREATEOBJECT parent identity");
            expect(create_parent.source == main_path.string(),
                   "inherited external-parent declarative derived-startup external-base should preserve CREATEOBJECT parent source");
            expect(create_child.prog_id == "SaveButton",
                   "inherited external-parent declarative derived-startup external-base should materialize the CREATEOBJECT child");
            expect(create_child.base_class_name == "ParentButton",
                   "inherited external-parent declarative derived-startup external-base should preserve the CREATEOBJECT child immediate external base");
            expect(create_child.source == main_path.string(),
                   "inherited external-parent declarative derived-startup external-base should preserve derived-startup child source identity");
            expect(create_child.class_library == button_library_path.string(),
                   "inherited external-parent declarative derived-startup external-base should preserve the CREATEOBJECT child external ClassLibrary path");
            expect(create_child.class_hierarchy.size() == 5U,
                   "inherited external-parent declarative derived-startup external-base should preserve the CREATEOBJECT child class hierarchy");
            if (create_child.class_hierarchy.size() == 5U)
            {
                expect(create_child.class_hierarchy[0] == "SAVEBUTTON",
                       "inherited external-parent declarative derived-startup external-base should store the CREATEOBJECT child class first");
                expect(create_child.class_hierarchy[1] == "PARENTBUTTON",
                       "inherited external-parent declarative derived-startup external-base should store the immediate external base second");
                expect(create_child.class_hierarchy[2] == "ROOTBUTTON",
                       "inherited external-parent declarative derived-startup external-base should store the deeper external base third");
                expect(create_child.class_hierarchy[3] == "CUSTOM",
                       "inherited external-parent declarative derived-startup external-base should store the builtin base token fourth");
                expect(create_child.class_hierarchy[4] == "OBJECT",
                       "inherited external-parent declarative derived-startup external-base should store the terminal object token fifth");
            }
            expect(!create_child.properties.contains("classlibrary"),
                   "inherited external-parent declarative derived-startup external-base should not materialize a CREATEOBJECT child ClassLibrary shadow");
            expect(leaf_parent.prog_id == "LeafForm",
                   "inherited external-parent declarative derived-startup external-base should preserve NEWOBJECT leaf identity");
            expect(leaf_parent.source == main_path.string(),
                   "inherited external-parent declarative derived-startup external-base should preserve NEWOBJECT leaf source");
            expect(leaf_child.prog_id == "SaveButton",
                   "inherited external-parent declarative derived-startup external-base should materialize the leaf child");
            expect(leaf_child.base_class_name == "ParentButton",
                   "inherited external-parent declarative derived-startup external-base should preserve the leaf child immediate external base");
            expect(leaf_child.source == main_path.string(),
                   "inherited external-parent declarative derived-startup external-base should preserve leaf child source identity");
            expect(leaf_child.class_library == button_library_path.string(),
                   "inherited external-parent declarative derived-startup external-base should preserve the leaf child external ClassLibrary path");
            expect(leaf_child.class_hierarchy.size() == 5U,
                   "inherited external-parent declarative derived-startup external-base should preserve the leaf child class hierarchy");
            if (leaf_child.class_hierarchy.size() == 5U)
            {
                expect(leaf_child.class_hierarchy[0] == "SAVEBUTTON",
                       "inherited external-parent declarative derived-startup external-base should store the leaf child class first");
                expect(leaf_child.class_hierarchy[1] == "PARENTBUTTON",
                       "inherited external-parent declarative derived-startup external-base should store the leaf immediate external base second");
                expect(leaf_child.class_hierarchy[2] == "ROOTBUTTON",
                       "inherited external-parent declarative derived-startup external-base should store the leaf deeper external base third");
                expect(leaf_child.class_hierarchy[3] == "CUSTOM",
                       "inherited external-parent declarative derived-startup external-base should store the leaf builtin base token fourth");
                expect(leaf_child.class_hierarchy[4] == "OBJECT",
                       "inherited external-parent declarative derived-startup external-base should store the leaf terminal object token fifth");
            }
            expect(!leaf_child.properties.contains("classlibrary"),
                   "inherited external-parent declarative derived-startup external-base should not materialize a leaf child ClassLibrary shadow");
            expect(state.ole_objects[4].prog_id == "Scripting.Dictionary",
                   "COM behavior should remain stable while inherited external-parent declarative derived-startup external-base lands");
        }

        const bool has_addobject_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.addobject" &&
                   (event.detail == "ChildForm.cmdsave:SaveButton" ||
                    event.detail == "LeafForm.cmdsave:SaveButton");
        });
        expect(has_addobject_event,
               "inherited external-parent declarative derived-startup external-base should emit child materialization events");

        const bool has_save_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "ParentForm.Save";
        });
        expect(has_save_invoke_event,
               "inherited external-parent declarative derived-startup external-base should keep materialized children usable through THISFORM owner dispatch");

        fs::remove_all(temp_root, ignored);
    }

}
