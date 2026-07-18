#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_inherited_external_base_addobject_external_child_base_dotted_access_resolves_live_child_chain()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_base_inherited_addobject_external_child_base_dotted";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path root_library_path = temp_root / "rootbuttons.prg";
        write_text(
            root_library_path,
            "DEFINE CLASS ParentButton AS Custom\n"
            "    FUNCTION RootToken\n"
            "        RETURN 'RootToken'\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        const fs::path button_library_path = temp_root / "buttons.prg";
        write_text(
            button_library_path,
            "DEFINE CLASS SaveButton AS ParentButton OF rootbuttons.prg\n"
            "    Caption = 'Save'\n"
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
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton', 'buttons.prg')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "    FUNCTION Save\n"
            "        THIS.Caption = THIS.Caption + '-Saved'\n"
            "        RETURN THIS.Caption\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_base_inherited_addobject_external_child_base_dotted.prg";
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
            "lDictSet = SETPEM(oDict, 'comparemode', 138)\n"
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
               std::string("external-base inherited ADDOBJECT external child-base dotted script should complete: ") + state.message +
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
        check("ccreatechildbaseclass", "ParentButton");
        check("cleafchildbaseclass", "ParentButton");
        check("ccreatechildclasslibrary", root_library_path.string());
        check("cleafchildclasslibrary", root_library_path.string());
        check("lcreatechildhasparent", "true");
        check("lleafchildhasparent", "true");
        check("ccreateroottoken", "RootToken");
        check("cleafroottoken", "RootToken");
        check("ldictset", "true");
        check("ndictcompare", "138");

        expect(state.ole_objects.size() == 5U,
               "external-base inherited ADDOBJECT external child-base dotted should register CREATEOBJECT parent/child, NEWOBJECT parent/child, and COM objects");
        if (state.ole_objects.size() == 5U)
        {
            const auto &create_parent = state.ole_objects[0];
            const auto &create_child = state.ole_objects[1];
            const auto &leaf_parent = state.ole_objects[2];
            const auto &leaf_child = state.ole_objects[3];
            expect(create_parent.prog_id == "ChildForm",
                   "external-base inherited ADDOBJECT external child-base dotted should preserve CREATEOBJECT parent identity");
            expect(create_parent.source == main_path.string(),
                   "external-base inherited ADDOBJECT external child-base dotted should preserve CREATEOBJECT parent source");
            expect(create_child.prog_id == "SaveButton",
                   "external-base inherited ADDOBJECT external child-base dotted should preserve CREATEOBJECT child identity");
            expect(create_child.base_class_name == "ParentButton",
                   "external-base inherited ADDOBJECT external child-base dotted should preserve the CREATEOBJECT child immediate external base");
            expect(create_child.source == button_library_path.string(),
                   "external-base inherited ADDOBJECT external child-base dotted should preserve CREATEOBJECT child definition source");
            expect(create_child.class_library == root_library_path.string(),
                   "external-base inherited ADDOBJECT external child-base dotted should preserve CREATEOBJECT child ClassLibrary path");
            expect(leaf_parent.prog_id == "LeafForm",
                   "external-base inherited ADDOBJECT external child-base dotted should preserve NEWOBJECT leaf identity");
            expect(leaf_parent.source == main_path.string(),
                   "external-base inherited ADDOBJECT external child-base dotted should preserve NEWOBJECT leaf source");
            expect(leaf_child.prog_id == "SaveButton",
                   "external-base inherited ADDOBJECT external child-base dotted should preserve leaf child identity");
            expect(leaf_child.base_class_name == "ParentButton",
                   "external-base inherited ADDOBJECT external child-base dotted should preserve the leaf child immediate external base");
            expect(leaf_child.source == button_library_path.string(),
                   "external-base inherited ADDOBJECT external child-base dotted should preserve leaf child definition source");
            expect(leaf_child.class_library == root_library_path.string(),
                   "external-base inherited ADDOBJECT external child-base dotted should preserve leaf child ClassLibrary path");

            const auto create_parent_caption = create_parent.properties.find("caption");
            const auto create_child_caption = create_child.properties.find("caption");
            const auto leaf_parent_caption = leaf_parent.properties.find("caption");
            const auto leaf_child_caption = leaf_child.properties.find("caption");
            if (create_parent_caption != create_parent.properties.end())
            {
                expect(copperfin::runtime::format_value(create_parent_caption->second) == "Done-Saved",
                       "external-base inherited ADDOBJECT external child-base dotted should let dotted child method calls update the CREATEOBJECT owner");
            }
            else
            {
                expect(false, "external-base inherited ADDOBJECT external child-base dotted should preserve the CREATEOBJECT owner caption");
            }
            if (create_child_caption != create_child.properties.end())
            {
                expect(copperfin::runtime::format_value(create_child_caption->second) == "Go",
                       "external-base inherited ADDOBJECT external child-base dotted should preserve the CREATEOBJECT child dotted assignment");
            }
            else
            {
                expect(false, "external-base inherited ADDOBJECT external child-base dotted should preserve the CREATEOBJECT child caption");
            }
            if (leaf_parent_caption != leaf_parent.properties.end())
            {
                expect(copperfin::runtime::format_value(leaf_parent_caption->second) == "Ready-Saved",
                       "external-base inherited ADDOBJECT external child-base dotted should let dotted child method calls update the leaf owner");
            }
            else
            {
                expect(false, "external-base inherited ADDOBJECT external child-base dotted should preserve the leaf owner caption");
            }
            if (leaf_child_caption != leaf_child.properties.end())
            {
                expect(copperfin::runtime::format_value(leaf_child_caption->second) == "Ship",
                       "external-base inherited ADDOBJECT external child-base dotted should preserve the leaf child dotted assignment");
            }
            else
            {
                expect(false, "external-base inherited ADDOBJECT external child-base dotted should preserve the leaf child caption");
            }
            expect(state.ole_objects[4].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external-base inherited ADDOBJECT external child-base dotted lands");
        }

        const bool has_addobject_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.addobject" &&
                   (event.detail == "ChildForm.cmdsave:SaveButton" ||
                    event.detail == "LeafForm.cmdsave:SaveButton");
        });
        expect(has_addobject_event,
               "external-base inherited ADDOBJECT external child-base dotted should emit child materialization events");

        const bool has_owner_caption_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   (event.detail == "SaveButton.OwnerCaption" ||
                    event.detail == "ParentButton.OwnerCaption");
        });
        expect(has_owner_caption_invoke_event,
               "external-base inherited ADDOBJECT external child-base dotted should dispatch direct dotted child method calls");

        const bool has_save_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "ParentForm.Save";
        });
        expect(has_save_invoke_event,
               "external-base inherited ADDOBJECT external child-base dotted should keep child THISFORM owner dispatch stable");

        fs::remove_all(temp_root, ignored);
    }

    void test_inherited_external_base_declarative_external_child_base_dotted_access_resolves_live_child_chain()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_base_inherited_declarative_external_child_base_dotted";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path root_library_path = temp_root / "rootbuttons.prg";
        write_text(
            root_library_path,
            "DEFINE CLASS ParentButton AS Custom\n"
            "    FUNCTION RootToken\n"
            "        RETURN 'RootToken'\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        const fs::path button_library_path = temp_root / "buttons.prg";
        write_text(
            button_library_path,
            "DEFINE CLASS SaveButton AS ParentButton OF rootbuttons.prg\n"
            "    Caption = 'Save'\n"
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
            "    ADD OBJECT cmdSave AS SaveButton OF buttons.prg\n"
            "    FUNCTION Save\n"
            "        THIS.Caption = THIS.Caption + '-Saved'\n"
            "        RETURN THIS.Caption\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_base_inherited_declarative_external_child_base_dotted.prg";
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
            "lDictSet = SETPEM(oDict, 'comparemode', 140)\n"
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
               std::string("external-base inherited declarative external child-base dotted script should complete: ") + state.message +
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
        check("ccreatechildbaseclass", "ParentButton");
        check("cleafchildbaseclass", "ParentButton");
        check("ccreatechildclasslibrary", root_library_path.string());
        check("cleafchildclasslibrary", root_library_path.string());
        check("lcreatechildhasparent", "true");
        check("lleafchildhasparent", "true");
        check("ccreateroottoken", "RootToken");
        check("cleafroottoken", "RootToken");
        check("ldictset", "true");
        check("ndictcompare", "140");

        expect(state.ole_objects.size() == 5U,
               "external-base inherited declarative external child-base dotted should register CREATEOBJECT parent/child, NEWOBJECT parent/child, and COM objects");
        if (state.ole_objects.size() == 5U)
        {
            const auto &create_parent = state.ole_objects[0];
            const auto &create_child = state.ole_objects[1];
            const auto &leaf_parent = state.ole_objects[2];
            const auto &leaf_child = state.ole_objects[3];
            expect(create_parent.prog_id == "ChildForm",
                   "external-base inherited declarative external child-base dotted should preserve CREATEOBJECT parent identity");
            expect(create_parent.source == main_path.string(),
                   "external-base inherited declarative external child-base dotted should preserve CREATEOBJECT parent source");
            expect(create_child.prog_id == "SaveButton",
                   "external-base inherited declarative external child-base dotted should preserve CREATEOBJECT child identity");
            expect(create_child.base_class_name == "ParentButton",
                   "external-base inherited declarative external child-base dotted should preserve the CREATEOBJECT child immediate external base");
            expect(create_child.source == button_library_path.string(),
                   "external-base inherited declarative external child-base dotted should preserve CREATEOBJECT child definition source");
            expect(create_child.class_library == root_library_path.string(),
                   "external-base inherited declarative external child-base dotted should preserve CREATEOBJECT child ClassLibrary path");
            expect(leaf_parent.prog_id == "LeafForm",
                   "external-base inherited declarative external child-base dotted should preserve NEWOBJECT leaf identity");
            expect(leaf_parent.source == main_path.string(),
                   "external-base inherited declarative external child-base dotted should preserve NEWOBJECT leaf source");
            expect(leaf_child.prog_id == "SaveButton",
                   "external-base inherited declarative external child-base dotted should preserve leaf child identity");
            expect(leaf_child.base_class_name == "ParentButton",
                   "external-base inherited declarative external child-base dotted should preserve the leaf child immediate external base");
            expect(leaf_child.source == button_library_path.string(),
                   "external-base inherited declarative external child-base dotted should preserve leaf child definition source");
            expect(leaf_child.class_library == root_library_path.string(),
                   "external-base inherited declarative external child-base dotted should preserve leaf child ClassLibrary path");

            const auto create_parent_caption = create_parent.properties.find("caption");
            const auto create_child_caption = create_child.properties.find("caption");
            const auto leaf_parent_caption = leaf_parent.properties.find("caption");
            const auto leaf_child_caption = leaf_child.properties.find("caption");
            if (create_parent_caption != create_parent.properties.end())
            {
                expect(copperfin::runtime::format_value(create_parent_caption->second) == "Done-Saved",
                       "external-base inherited declarative external child-base dotted should let dotted child method calls update the CREATEOBJECT owner");
            }
            else
            {
                expect(false, "external-base inherited declarative external child-base dotted should preserve the CREATEOBJECT owner caption");
            }
            if (create_child_caption != create_child.properties.end())
            {
                expect(copperfin::runtime::format_value(create_child_caption->second) == "Go",
                       "external-base inherited declarative external child-base dotted should preserve the CREATEOBJECT child dotted assignment");
            }
            else
            {
                expect(false, "external-base inherited declarative external child-base dotted should preserve the CREATEOBJECT child caption");
            }
            if (leaf_parent_caption != leaf_parent.properties.end())
            {
                expect(copperfin::runtime::format_value(leaf_parent_caption->second) == "Ready-Saved",
                       "external-base inherited declarative external child-base dotted should let dotted child method calls update the leaf owner");
            }
            else
            {
                expect(false, "external-base inherited declarative external child-base dotted should preserve the leaf owner caption");
            }
            if (leaf_child_caption != leaf_child.properties.end())
            {
                expect(copperfin::runtime::format_value(leaf_child_caption->second) == "Ship",
                       "external-base inherited declarative external child-base dotted should preserve the leaf child dotted assignment");
            }
            else
            {
                expect(false, "external-base inherited declarative external child-base dotted should preserve the leaf child caption");
            }
            expect(state.ole_objects[4].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external-base inherited declarative external child-base dotted lands");
        }

        const bool has_addobject_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.addobject" &&
                   (event.detail == "ChildForm.cmdsave:SaveButton" ||
                    event.detail == "LeafForm.cmdsave:SaveButton");
        });
        expect(has_addobject_event,
               "external-base inherited declarative external child-base dotted should emit child materialization events");

        const bool has_owner_caption_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   (event.detail == "SaveButton.OwnerCaption" ||
                    event.detail == "ParentButton.OwnerCaption");
        });
        expect(has_owner_caption_invoke_event,
               "external-base inherited declarative external child-base dotted should dispatch direct dotted child method calls");

        const bool has_save_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "ParentForm.Save";
        });
        expect(has_save_invoke_event,
               "external-base inherited declarative external child-base dotted should keep child THISFORM owner dispatch stable");

        fs::remove_all(temp_root, ignored);
    }

    void test_inherited_external_base_declarative_deeper_external_child_base_dotted_access_resolves_live_child_chain()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_base_inherited_declarative_deeper_external_child_base_dotted";
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
            "    ADD OBJECT cmdSave AS SaveButton OF buttons.prg\n"
            "    FUNCTION Save\n"
            "        THIS.Caption = THIS.Caption + '-Saved'\n"
            "        RETURN THIS.Caption\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_base_inherited_declarative_deeper_external_child_base_dotted.prg";
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
            "lDictSet = SETPEM(oDict, 'comparemode', 141)\n"
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
               std::string("external-base inherited declarative deeper external child-base dotted script should complete: ") + state.message +
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
        check("ccreatechildbaseclass", "ParentButton");
        check("cleafchildbaseclass", "ParentButton");
        check("lcreatechildhasparent", "true");
        check("lleafchildhasparent", "true");
        check("ccreateroottoken", "RootToken");
        check("cleafroottoken", "RootToken");
        check("ldictset", "true");
        check("ndictcompare", "141");

        const auto create_child_class_library = state.globals.find("xcreatechildclasslibrary");
        expect(create_child_class_library != state.globals.end() &&
                   create_child_class_library->second.kind == copperfin::runtime::PrgValueKind::empty,
               "external-base inherited declarative deeper external child-base dotted should keep the CREATEOBJECT child ClassLibrary empty through ordinary reads");
        const auto leaf_child_class_library = state.globals.find("xleafchildclasslibrary");
        expect(leaf_child_class_library != state.globals.end() &&
                   leaf_child_class_library->second.kind == copperfin::runtime::PrgValueKind::empty,
               "external-base inherited declarative deeper external child-base dotted should keep the leaf child ClassLibrary empty through ordinary reads");

        expect(state.ole_objects.size() == 5U,
               "external-base inherited declarative deeper external child-base dotted should register CREATEOBJECT parent/child, NEWOBJECT parent/child, and COM objects");
        if (state.ole_objects.size() == 5U)
        {
            const auto &create_parent = state.ole_objects[0];
            const auto &create_child = state.ole_objects[1];
            const auto &leaf_parent = state.ole_objects[2];
            const auto &leaf_child = state.ole_objects[3];
            expect(create_parent.prog_id == "ChildForm",
                   "external-base inherited declarative deeper external child-base dotted should preserve CREATEOBJECT parent identity");
            expect(create_parent.source == main_path.string(),
                   "external-base inherited declarative deeper external child-base dotted should preserve CREATEOBJECT parent source");
            expect(create_child.prog_id == "SaveButton",
                   "external-base inherited declarative deeper external child-base dotted should preserve CREATEOBJECT child identity");
            expect(create_child.base_class_name == "ParentButton",
                   "external-base inherited declarative deeper external child-base dotted should preserve the CREATEOBJECT child immediate external base");
            expect(create_child.source == button_library_path.string(),
                   "external-base inherited declarative deeper external child-base dotted should preserve CREATEOBJECT child definition source");
            expect(create_child.class_library.empty(),
                   "external-base inherited declarative deeper external child-base dotted should keep the CREATEOBJECT child ClassLibrary empty");
            expect(leaf_parent.prog_id == "LeafForm",
                   "external-base inherited declarative deeper external child-base dotted should preserve NEWOBJECT leaf identity");
            expect(leaf_parent.source == main_path.string(),
                   "external-base inherited declarative deeper external child-base dotted should preserve NEWOBJECT leaf source");
            expect(leaf_child.prog_id == "SaveButton",
                   "external-base inherited declarative deeper external child-base dotted should preserve leaf child identity");
            expect(leaf_child.base_class_name == "ParentButton",
                   "external-base inherited declarative deeper external child-base dotted should preserve the leaf child immediate external base");
            expect(leaf_child.source == button_library_path.string(),
                   "external-base inherited declarative deeper external child-base dotted should preserve leaf child definition source");
            expect(leaf_child.class_library.empty(),
                   "external-base inherited declarative deeper external child-base dotted should keep the leaf child ClassLibrary empty");

            const auto create_parent_caption = create_parent.properties.find("caption");
            const auto create_child_caption = create_child.properties.find("caption");
            const auto leaf_parent_caption = leaf_parent.properties.find("caption");
            const auto leaf_child_caption = leaf_child.properties.find("caption");
            if (create_parent_caption != create_parent.properties.end())
            {
                expect(copperfin::runtime::format_value(create_parent_caption->second) == "Done-Saved",
                       "external-base inherited declarative deeper external child-base dotted should let dotted child method calls update the CREATEOBJECT owner");
            }
            else
            {
                expect(false, "external-base inherited declarative deeper external child-base dotted should preserve the CREATEOBJECT owner caption");
            }
            if (create_child_caption != create_child.properties.end())
            {
                expect(copperfin::runtime::format_value(create_child_caption->second) == "Go",
                       "external-base inherited declarative deeper external child-base dotted should preserve the CREATEOBJECT child dotted assignment");
            }
            else
            {
                expect(false, "external-base inherited declarative deeper external child-base dotted should preserve the CREATEOBJECT child caption");
            }
            if (leaf_parent_caption != leaf_parent.properties.end())
            {
                expect(copperfin::runtime::format_value(leaf_parent_caption->second) == "Ready-Saved",
                       "external-base inherited declarative deeper external child-base dotted should let dotted child method calls update the leaf owner");
            }
            else
            {
                expect(false, "external-base inherited declarative deeper external child-base dotted should preserve the leaf owner caption");
            }
            if (leaf_child_caption != leaf_child.properties.end())
            {
                expect(copperfin::runtime::format_value(leaf_child_caption->second) == "Ship",
                       "external-base inherited declarative deeper external child-base dotted should preserve the leaf child dotted assignment");
            }
            else
            {
                expect(false, "external-base inherited declarative deeper external child-base dotted should preserve the leaf child caption");
            }
            expect(state.ole_objects[4].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external-base inherited declarative deeper external child-base dotted lands");
        }

        const bool has_addobject_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.addobject" &&
                   (event.detail == "ChildForm.cmdsave:SaveButton" ||
                    event.detail == "LeafForm.cmdsave:SaveButton");
        });
        expect(has_addobject_event,
               "external-base inherited declarative deeper external child-base dotted should emit child materialization events");

        const bool has_owner_caption_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   (event.detail == "SaveButton.OwnerCaption" ||
                    event.detail == "ParentButton.OwnerCaption");
        });
        expect(has_owner_caption_invoke_event,
               "external-base inherited declarative deeper external child-base dotted should dispatch direct dotted child method calls");

        const bool has_save_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "ParentForm.Save";
        });
        expect(has_save_invoke_event,
               "external-base inherited declarative deeper external child-base dotted should keep child THISFORM owner dispatch stable");

        fs::remove_all(temp_root, ignored);
    }

}
