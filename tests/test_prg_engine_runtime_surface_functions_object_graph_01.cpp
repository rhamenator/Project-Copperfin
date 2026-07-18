#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_inherited_removeobject_detaches_declarative_external_base_subtree_and_preserves_classlibrary_provenance()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_inherited_removeobject_declarative_external_base_subtree";
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

        const fs::path main_path = temp_root / "inherited_removeobject_declarative_external_base_subtree.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oChild = oCreate.cmdSave\n"
            "oGrandchild = oChild.lblBadge\n"
            "cChildClassBeforeRemove = oChild.Class\n"
            "cChildBaseClassBeforeRemove = oChild.BaseClass\n"
            "cChildParentClassBeforeRemove = oChild.ParentClass\n"
            "cChildClassLibraryBeforeRemove = GETPEM(oChild, 'ClassLibrary')\n"
            "lChildHasClassLibraryBeforeRemove = PEMSTATUS(oChild, 'ClassLibrary', 1)\n"
            "nChildClassCountBeforeRemove = ACLASS(aChildClass, oChild)\n"
            "cChildClass1BeforeRemove = aChildClass[1]\n"
            "cChildClass2BeforeRemove = aChildClass[2]\n"
            "cChildClass3BeforeRemove = aChildClass[3]\n"
            "cChildClass4BeforeRemove = aChildClass[4]\n"
            "cChildClass5BeforeRemove = aChildClass[5]\n"
            "cChildCaptionBeforeRemove = oChild.Caption\n"
            "cGrandchildCaptionBeforeRemove = oGrandchild.Caption\n"
            "lRemoved = oCreate.RemoveObject('cmdSave')\n"
            "lRemovedMissing = oCreate.RemoveObject('cmdSave')\n"
            "lHasChildAfterRemove = PEMSTATUS(oCreate, 'cmdSave', 1)\n"
            "xRemovedChild = GETPEM(oCreate, 'cmdSave')\n"
            "lChildHasParentAfterRemove = PEMSTATUS(oChild, 'Parent', 1)\n"
            "xChildParentAfterRemove = GETPEM(oChild, 'Parent')\n"
            "lChildHasClassLibraryAfterRemove = PEMSTATUS(oChild, 'ClassLibrary', 1)\n"
            "cChildClassLibraryAfterRemove = GETPEM(oChild, 'ClassLibrary')\n"
            "lChildHasGrandchildAfterRemove = PEMSTATUS(oChild, 'lblBadge', 1)\n"
            "cChildCaptionAfterRemove = oChild.Caption\n"
            "cGrandchildCaptionAfterRemove = oGrandchild.Caption\n"
            "lGrandchildHasParentAfterRemove = PEMSTATUS(oGrandchild, 'Parent', 1)\n"
            "cGrandchildParentCaptionAfterRemove = oGrandchild.Parent.Caption\n"
            "cGrandchildFromChildAfterRemove = oChild.lblBadge.Caption\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 122)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ParentForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ChildForm AS ParentForm\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS ParentButton OF buttons.prg\n"
            "    Caption = 'Save'\n"
            "    ADD OBJECT lblBadge AS BadgeLabel\n"
            "ENDDEFINE\n"
            "DEFINE CLASS BadgeLabel AS Custom\n"
            "    Caption = 'Badge'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("inherited REMOVEOBJECT declarative external-base subtree script should complete: ") + state.message +
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

        check("cchildclassbeforeremove", "SaveButton");
        check("cchildbaseclassbeforeremove", "ParentButton");
        check("cchildparentclassbeforeremove", "ParentButton");
        check("cchildclasslibrarybeforeremove", button_library_path.string());
        check("lchildhasclasslibrarybeforeremove", "true");
        check("nchildclasscountbeforeremove", "5");
        check("cchildclass1beforeremove", "SAVEBUTTON");
        check("cchildclass2beforeremove", "PARENTBUTTON");
        check("cchildclass3beforeremove", "ROOTBUTTON");
        check("cchildclass4beforeremove", "CUSTOM");
        check("cchildclass5beforeremove", "OBJECT");
        check("cchildcaptionbeforeremove", "Save");
        check("cgrandchildcaptionbeforeremove", "Badge");
        check("lremoved", "true");
        check("lremovedmissing", "false");
        check("lhaschildafterremove", "false");
        check("lchildhasparentafterremove", "false");
        check("lchildhasclasslibraryafterremove", "true");
        check("cchildclasslibraryafterremove", button_library_path.string());
        check("lchildhasgrandchildafterremove", "true");
        check("cchildcaptionafterremove", "Save");
        check("cgrandchildcaptionafterremove", "Badge");
        check("lgrandchildhasparentafterremove", "true");
        check("cgrandchildparentcaptionafterremove", "Save");
        check("cgrandchildfromchildafterremove", "Badge");
        check("ldictset", "true");
        check("ndictcompare", "122");

        const auto removed_child = state.globals.find("xremovedchild");
        expect(removed_child != state.globals.end() &&
                   removed_child->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited REMOVEOBJECT declarative external-base subtree should invalidate GETPEM() on the owner's removed child slot");

        const auto child_parent_after_remove = state.globals.find("xchildparentafterremove");
        expect(child_parent_after_remove != state.globals.end() &&
                   child_parent_after_remove->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited REMOVEOBJECT declarative external-base subtree should clear the detached child's Parent");

        expect(state.ole_objects.size() == 4U,
               "inherited REMOVEOBJECT declarative external-base subtree should register owner, detached child subtree, and COM objects");
        if (state.ole_objects.size() == 4U)
        {
            const auto &owner_object = state.ole_objects[0];
            const auto &child_object = state.ole_objects[1];
            const auto &grandchild_object = state.ole_objects[2];
            expect(owner_object.prog_id == "ChildForm",
                   "inherited REMOVEOBJECT declarative external-base subtree should preserve derived owner identity");
            expect(!owner_object.properties.contains("cmdsave"),
                   "inherited REMOVEOBJECT declarative external-base subtree should detach the child from the derived owner");
            expect(child_object.prog_id == "SaveButton",
                   "inherited REMOVEOBJECT declarative external-base subtree should preserve child identity");
            expect(child_object.base_class_name == "ParentButton",
                   "inherited REMOVEOBJECT declarative external-base subtree should preserve the detached child's immediate base-class identity");
            expect(child_object.source == main_path.string(),
                   "inherited REMOVEOBJECT declarative external-base subtree should preserve the detached child's defining source path");
            expect(child_object.class_library == button_library_path.string(),
                   "inherited REMOVEOBJECT declarative external-base subtree should preserve the detached child's immediate external ClassLibrary path");
            expect(!child_object.properties.contains("parent"),
                   "inherited REMOVEOBJECT declarative external-base subtree should clear the detached child's parent property");
            expect(child_object.properties.contains("lblbadge"),
                   "inherited REMOVEOBJECT declarative external-base subtree should preserve the held grandchild on the detached child");
            expect(grandchild_object.prog_id == "BadgeLabel",
                   "inherited REMOVEOBJECT declarative external-base subtree should preserve detached descendant identity");
            expect(state.ole_objects[3].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while inherited REMOVEOBJECT declarative external-base subtree lands");
        }

        const bool has_removeobject_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.removeobject" &&
                   event.detail == "ChildForm.cmdsave";
        });
        expect(has_removeobject_event,
               "inherited REMOVEOBJECT declarative external-base subtree should emit a detachment event");

        const bool has_child_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "SaveButton.Destroy";
        });
        expect(!has_child_destroy_event,
               "inherited REMOVEOBJECT declarative external-base subtree should not destroy the detached child");

        const bool has_grandchild_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "BadgeLabel.Destroy";
        });
        expect(!has_grandchild_destroy_event,
               "inherited REMOVEOBJECT declarative external-base subtree should not destroy the detached descendant");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_removeobject_detaches_object_block_same_prg_subtree_and_preserves_held_subtree_state()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_removeobject_object_block_same_prg_subtree";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_removeobject_object_block_same_prg_subtree.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('DemoForm')\n"
            "oChild = oForm.cmdSave\n"
            "oSibling = oForm.cmdCancel\n"
            "oGrandchild = oChild.lblBadge\n"
            "cChildClassBeforeRemove = oChild.Class\n"
            "cChildBaseClassBeforeRemove = oChild.BaseClass\n"
            "cChildParentClassBeforeRemove = oChild.ParentClass\n"
            "xChildClassLibraryBeforeRemove = GETPEM(oChild, 'ClassLibrary')\n"
            "lChildHasClassLibraryBeforeRemove = PEMSTATUS(oChild, 'ClassLibrary', 1)\n"
            "nChildClassCountBeforeRemove = ACLASS(aChildClass, oChild)\n"
            "cChildClass1BeforeRemove = aChildClass[1]\n"
            "cChildClass2BeforeRemove = aChildClass[2]\n"
            "cChildClass3BeforeRemove = aChildClass[3]\n"
            "cChildClass4BeforeRemove = aChildClass[4]\n"
            "cChildCaptionBeforeRemove = oChild.Caption\n"
            "cSiblingCaptionBeforeRemove = oSibling.Caption\n"
            "cGrandchildCaptionBeforeRemove = oGrandchild.Caption\n"
            "lRemoved = oForm.RemoveObject('cmdSave')\n"
            "lRemovedMissing = oForm.RemoveObject('cmdSave')\n"
            "lHasChildAfterRemove = PEMSTATUS(oForm, 'cmdSave', 1)\n"
            "xRemovedChild = GETPEM(oForm, 'cmdSave')\n"
            "lHasSiblingAfterRemove = PEMSTATUS(oForm, 'cmdCancel', 1)\n"
            "cSiblingCaptionAfterRemove = oForm.cmdCancel.Caption\n"
            "lChildHasParentAfterRemove = PEMSTATUS(oChild, 'Parent', 1)\n"
            "xChildParentAfterRemove = GETPEM(oChild, 'Parent')\n"
            "lChildHasClassLibraryAfterRemove = PEMSTATUS(oChild, 'ClassLibrary', 1)\n"
            "xChildClassLibraryAfterRemove = GETPEM(oChild, 'ClassLibrary')\n"
            "lChildHasGrandchildAfterRemove = PEMSTATUS(oChild, 'lblBadge', 1)\n"
            "cChildCaptionAfterRemove = oChild.Caption\n"
            "cGrandchildCaptionAfterRemove = oGrandchild.Caption\n"
            "lGrandchildHasParentAfterRemove = PEMSTATUS(oGrandchild, 'Parent', 1)\n"
            "cGrandchildParentCaptionAfterRemove = oGrandchild.Parent.Caption\n"
            "cGrandchildFromChildAfterRemove = oChild.lblBadge.Caption\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 121)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    OBJECT cmdSave AS SaveButton\n"
            "        Caption = 'Commit'\n"
            "    ENDOBJECT\n"
            "    OBJECT cmdCancel AS CancelButton\n"
            "        Caption = 'Cancel'\n"
            "    ENDOBJECT\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ParentButton AS Custom\n"
            "    Caption = 'Save'\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS ParentButton\n"
            "    OBJECT lblBadge AS BadgeLabel\n"
            "        Caption = 'Badge'\n"
            "    ENDOBJECT\n"
            "ENDDEFINE\n"
            "DEFINE CLASS BadgeLabel AS Custom\n"
            "    Caption = 'Badge'\n"
            "ENDDEFINE\n"
            "DEFINE CLASS CancelButton AS Custom\n"
            "    Caption = 'Cancel'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native REMOVEOBJECT object-block same-PRG subtree script should complete: ") + state.message +
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

        check("cchildclassbeforeremove", "SaveButton");
        check("cchildbaseclassbeforeremove", "ParentButton");
        check("cchildparentclassbeforeremove", "ParentButton");
        check("lchildhasclasslibrarybeforeremove", "false");
        check("nchildclasscountbeforeremove", "4");
        check("cchildclass1beforeremove", "SAVEBUTTON");
        check("cchildclass2beforeremove", "PARENTBUTTON");
        check("cchildclass3beforeremove", "CUSTOM");
        check("cchildclass4beforeremove", "OBJECT");
        check("cchildcaptionbeforeremove", "Commit");
        check("csiblingcaptionbeforeremove", "Cancel");
        check("cgrandchildcaptionbeforeremove", "Badge");
        check("lremoved", "true");
        check("lremovedmissing", "false");
        check("lhaschildafterremove", "false");
        check("lhassiblingafterremove", "true");
        check("csiblingcaptionafterremove", "Cancel");
        check("lchildhasparentafterremove", "false");
        check("lchildhasclasslibraryafterremove", "false");
        check("lchildhasgrandchildafterremove", "true");
        check("cchildcaptionafterremove", "Commit");
        check("cgrandchildcaptionafterremove", "Badge");
        check("lgrandchildhasparentafterremove", "true");
        check("cgrandchildparentcaptionafterremove", "Commit");
        check("cgrandchildfromchildafterremove", "Badge");
        check("ldictset", "true");
        check("ndictcompare", "121");

        const auto child_class_library_before_remove = state.globals.find("xchildclasslibrarybeforeremove");
        expect(child_class_library_before_remove != state.globals.end() &&
                   child_class_library_before_remove->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native REMOVEOBJECT object-block same-PRG subtree should leave the child ClassLibrary empty before removal");

        const auto removed_child = state.globals.find("xremovedchild");
        expect(removed_child != state.globals.end() &&
                   removed_child->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native REMOVEOBJECT object-block same-PRG subtree should invalidate GETPEM() on the owner's removed child slot");

        const auto child_parent_after_remove = state.globals.find("xchildparentafterremove");
        expect(child_parent_after_remove != state.globals.end() &&
                   child_parent_after_remove->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native REMOVEOBJECT object-block same-PRG subtree should clear the detached child's Parent");

        const auto child_class_library_after_remove = state.globals.find("xchildclasslibraryafterremove");
        expect(child_class_library_after_remove != state.globals.end() &&
                   child_class_library_after_remove->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native REMOVEOBJECT object-block same-PRG subtree should keep the detached child's ClassLibrary empty");

        expect(state.ole_objects.size() == 5U,
               "native REMOVEOBJECT object-block same-PRG subtree should register owner, detached child subtree, surviving sibling, and COM objects");
        if (state.ole_objects.size() == 5U)
        {
            const auto &owner_object = state.ole_objects[0];
            const auto &child_object = state.ole_objects[1];
            const auto &grandchild_object = state.ole_objects[2];
            const auto &sibling_object = state.ole_objects[3];
            expect(owner_object.prog_id == "DemoForm",
                   "native REMOVEOBJECT object-block same-PRG subtree should preserve owner identity");
            expect(!owner_object.properties.contains("cmdsave"),
                   "native REMOVEOBJECT object-block same-PRG subtree should detach the child from the owner");
            expect(owner_object.properties.contains("cmdcancel"),
                   "native REMOVEOBJECT object-block same-PRG subtree should preserve the surviving sibling on the owner");
            expect(child_object.prog_id == "SaveButton",
                   "native REMOVEOBJECT object-block same-PRG subtree should preserve child identity");
            expect(child_object.base_class_name == "ParentButton",
                   "native REMOVEOBJECT object-block same-PRG subtree should preserve the detached child's immediate base-class identity");
            expect(child_object.source == main_path.string(),
                   "native REMOVEOBJECT object-block same-PRG subtree should preserve the detached child's defining source path");
            expect(child_object.class_library.empty(),
                   "native REMOVEOBJECT object-block same-PRG subtree should keep the detached child's ClassLibrary empty");
            expect(!child_object.properties.contains("parent"),
                   "native REMOVEOBJECT object-block same-PRG subtree should clear the detached child's parent property");
            expect(child_object.properties.contains("lblbadge"),
                   "native REMOVEOBJECT object-block same-PRG subtree should preserve the held grandchild on the detached child");
            expect(grandchild_object.prog_id == "BadgeLabel",
                   "native REMOVEOBJECT object-block same-PRG subtree should preserve detached descendant identity");
            expect(sibling_object.prog_id == "CancelButton",
                   "native REMOVEOBJECT object-block same-PRG subtree should preserve the surviving sibling object");
            expect(state.ole_objects[4].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native REMOVEOBJECT object-block same-PRG subtree lands");
        }

        const bool has_removeobject_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.removeobject" &&
                   event.detail == "DemoForm.cmdsave";
        });
        expect(has_removeobject_event,
               "native REMOVEOBJECT object-block same-PRG subtree should emit a detachment event");

        const bool has_child_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "SaveButton.Destroy";
        });
        expect(!has_child_destroy_event,
               "native REMOVEOBJECT object-block same-PRG subtree should not destroy the detached child");

        const bool has_grandchild_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "BadgeLabel.Destroy";
        });
        expect(!has_grandchild_destroy_event,
               "native REMOVEOBJECT object-block same-PRG subtree should not destroy the detached descendant");

        fs::remove_all(temp_root, ignored);
    }

    void test_inherited_removeobject_detaches_object_block_same_prg_subtree_and_preserves_held_subtree_state()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_inherited_removeobject_object_block_same_prg_subtree";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "inherited_removeobject_object_block_same_prg_subtree.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oChild = oCreate.cmdSave\n"
            "oSibling = oCreate.cmdCancel\n"
            "oGrandchild = oChild.lblBadge\n"
            "cChildClassBeforeRemove = oChild.Class\n"
            "cChildBaseClassBeforeRemove = oChild.BaseClass\n"
            "cChildParentClassBeforeRemove = oChild.ParentClass\n"
            "xChildClassLibraryBeforeRemove = GETPEM(oChild, 'ClassLibrary')\n"
            "lChildHasClassLibraryBeforeRemove = PEMSTATUS(oChild, 'ClassLibrary', 1)\n"
            "nChildClassCountBeforeRemove = ACLASS(aChildClass, oChild)\n"
            "cChildClass1BeforeRemove = aChildClass[1]\n"
            "cChildClass2BeforeRemove = aChildClass[2]\n"
            "cChildClass3BeforeRemove = aChildClass[3]\n"
            "cChildClass4BeforeRemove = aChildClass[4]\n"
            "cChildCaptionBeforeRemove = oChild.Caption\n"
            "cSiblingCaptionBeforeRemove = oSibling.Caption\n"
            "cGrandchildCaptionBeforeRemove = oGrandchild.Caption\n"
            "lRemoved = oCreate.RemoveObject('cmdSave')\n"
            "lRemovedMissing = oCreate.RemoveObject('cmdSave')\n"
            "lHasChildAfterRemove = PEMSTATUS(oCreate, 'cmdSave', 1)\n"
            "xRemovedChild = GETPEM(oCreate, 'cmdSave')\n"
            "lHasSiblingAfterRemove = PEMSTATUS(oCreate, 'cmdCancel', 1)\n"
            "cSiblingCaptionAfterRemove = oCreate.cmdCancel.Caption\n"
            "lChildHasParentAfterRemove = PEMSTATUS(oChild, 'Parent', 1)\n"
            "xChildParentAfterRemove = GETPEM(oChild, 'Parent')\n"
            "lChildHasClassLibraryAfterRemove = PEMSTATUS(oChild, 'ClassLibrary', 1)\n"
            "xChildClassLibraryAfterRemove = GETPEM(oChild, 'ClassLibrary')\n"
            "lChildHasGrandchildAfterRemove = PEMSTATUS(oChild, 'lblBadge', 1)\n"
            "cChildCaptionAfterRemove = oChild.Caption\n"
            "cGrandchildCaptionAfterRemove = oGrandchild.Caption\n"
            "lGrandchildHasParentAfterRemove = PEMSTATUS(oGrandchild, 'Parent', 1)\n"
            "cGrandchildParentCaptionAfterRemove = oGrandchild.Parent.Caption\n"
            "cGrandchildFromChildAfterRemove = oChild.lblBadge.Caption\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 122)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ParentForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    OBJECT cmdSave AS SaveButton\n"
            "        Caption = 'Commit'\n"
            "    ENDOBJECT\n"
            "    OBJECT cmdCancel AS CancelButton\n"
            "        Caption = 'Cancel'\n"
            "    ENDOBJECT\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ChildForm AS ParentForm\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ParentButton AS Custom\n"
            "    Caption = 'Save'\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS ParentButton\n"
            "    OBJECT lblBadge AS BadgeLabel\n"
            "        Caption = 'Badge'\n"
            "    ENDOBJECT\n"
            "ENDDEFINE\n"
            "DEFINE CLASS BadgeLabel AS Custom\n"
            "    Caption = 'Badge'\n"
            "ENDDEFINE\n"
            "DEFINE CLASS CancelButton AS Custom\n"
            "    Caption = 'Cancel'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("inherited REMOVEOBJECT object-block same-PRG subtree script should complete: ") + state.message +
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

        check("cchildclassbeforeremove", "SaveButton");
        check("cchildbaseclassbeforeremove", "ParentButton");
        check("cchildparentclassbeforeremove", "ParentButton");
        check("lchildhasclasslibrarybeforeremove", "false");
        check("nchildclasscountbeforeremove", "4");
        check("cchildclass1beforeremove", "SAVEBUTTON");
        check("cchildclass2beforeremove", "PARENTBUTTON");
        check("cchildclass3beforeremove", "CUSTOM");
        check("cchildclass4beforeremove", "OBJECT");
        check("cchildcaptionbeforeremove", "Commit");
        check("csiblingcaptionbeforeremove", "Cancel");
        check("cgrandchildcaptionbeforeremove", "Badge");
        check("lremoved", "true");
        check("lremovedmissing", "false");
        check("lhaschildafterremove", "false");
        check("lhassiblingafterremove", "true");
        check("csiblingcaptionafterremove", "Cancel");
        check("lchildhasparentafterremove", "false");
        check("lchildhasclasslibraryafterremove", "false");
        check("lchildhasgrandchildafterremove", "true");
        check("cchildcaptionafterremove", "Commit");
        check("cgrandchildcaptionafterremove", "Badge");
        check("lgrandchildhasparentafterremove", "true");
        check("cgrandchildparentcaptionafterremove", "Commit");
        check("cgrandchildfromchildafterremove", "Badge");
        check("ldictset", "true");
        check("ndictcompare", "122");

        const auto child_class_library_before_remove = state.globals.find("xchildclasslibrarybeforeremove");
        expect(child_class_library_before_remove != state.globals.end() &&
                   child_class_library_before_remove->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited REMOVEOBJECT object-block same-PRG subtree should leave the child ClassLibrary empty before removal");

        const auto removed_child = state.globals.find("xremovedchild");
        expect(removed_child != state.globals.end() &&
                   removed_child->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited REMOVEOBJECT object-block same-PRG subtree should invalidate GETPEM() on the derived owner's removed child slot");

        const auto child_parent_after_remove = state.globals.find("xchildparentafterremove");
        expect(child_parent_after_remove != state.globals.end() &&
                   child_parent_after_remove->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited REMOVEOBJECT object-block same-PRG subtree should clear the detached child's Parent");

        const auto child_class_library_after_remove = state.globals.find("xchildclasslibraryafterremove");
        expect(child_class_library_after_remove != state.globals.end() &&
                   child_class_library_after_remove->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited REMOVEOBJECT object-block same-PRG subtree should keep the detached child's ClassLibrary empty");

        expect(state.ole_objects.size() == 5U,
               "inherited REMOVEOBJECT object-block same-PRG subtree should register derived owner, detached child subtree, surviving sibling, and COM objects");
        if (state.ole_objects.size() == 5U)
        {
            const auto &owner_object = state.ole_objects[0];
            const auto &child_object = state.ole_objects[1];
            const auto &grandchild_object = state.ole_objects[2];
            const auto &sibling_object = state.ole_objects[3];
            expect(owner_object.prog_id == "ChildForm",
                   "inherited REMOVEOBJECT object-block same-PRG subtree should preserve derived owner identity");
            expect(!owner_object.properties.contains("cmdsave"),
                   "inherited REMOVEOBJECT object-block same-PRG subtree should detach the child from the derived owner");
            expect(owner_object.properties.contains("cmdcancel"),
                   "inherited REMOVEOBJECT object-block same-PRG subtree should preserve the surviving sibling on the derived owner");
            expect(child_object.prog_id == "SaveButton",
                   "inherited REMOVEOBJECT object-block same-PRG subtree should preserve child identity");
            expect(child_object.base_class_name == "ParentButton",
                   "inherited REMOVEOBJECT object-block same-PRG subtree should preserve the detached child's immediate base-class identity");
            expect(child_object.source == main_path.string(),
                   "inherited REMOVEOBJECT object-block same-PRG subtree should preserve the detached child's defining source path");
            expect(child_object.class_library.empty(),
                   "inherited REMOVEOBJECT object-block same-PRG subtree should keep the detached child's ClassLibrary empty");
            expect(!child_object.properties.contains("parent"),
                   "inherited REMOVEOBJECT object-block same-PRG subtree should clear the detached child's parent property");
            expect(child_object.properties.contains("lblbadge"),
                   "inherited REMOVEOBJECT object-block same-PRG subtree should preserve the held grandchild on the detached child");
            expect(grandchild_object.prog_id == "BadgeLabel",
                   "inherited REMOVEOBJECT object-block same-PRG subtree should preserve detached descendant identity");
            expect(sibling_object.prog_id == "CancelButton",
                   "inherited REMOVEOBJECT object-block same-PRG subtree should preserve the surviving sibling object");
            expect(state.ole_objects[4].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while inherited REMOVEOBJECT object-block same-PRG subtree lands");
        }

        const bool has_removeobject_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.removeobject" &&
                   event.detail == "ChildForm.cmdsave";
        });
        expect(has_removeobject_event,
               "inherited REMOVEOBJECT object-block same-PRG subtree should emit a detachment event");

        const bool has_child_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "SaveButton.Destroy";
        });
        expect(!has_child_destroy_event,
               "inherited REMOVEOBJECT object-block same-PRG subtree should not destroy the detached child");

        const bool has_grandchild_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "BadgeLabel.Destroy";
        });
        expect(!has_grandchild_destroy_event,
               "inherited REMOVEOBJECT object-block same-PRG subtree should not destroy the detached descendant");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_removeobject_detaches_object_block_external_child_subtree_and_preserves_held_subtree_state()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_removeobject_object_block_external_child_subtree";
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
            "    OBJECT lblBadge AS BadgeLabel\n"
            "        Caption = 'Badge'\n"
            "    ENDOBJECT\n"
            "ENDDEFINE\n"
            "DEFINE CLASS BadgeLabel AS Custom\n"
            "    Caption = 'Badge'\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "native_removeobject_object_block_external_child_subtree.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('DemoForm')\n"
            "oChild = oForm.cmdSave\n"
            "oSibling = oForm.cmdCancel\n"
            "oGrandchild = oChild.lblBadge\n"
            "cChildClassBeforeRemove = oChild.Class\n"
            "cChildBaseClassBeforeRemove = oChild.BaseClass\n"
            "cChildParentClassBeforeRemove = oChild.ParentClass\n"
            "xChildClassLibraryBeforeRemove = GETPEM(oChild, 'ClassLibrary')\n"
            "lChildHasClassLibraryBeforeRemove = PEMSTATUS(oChild, 'ClassLibrary', 1)\n"
            "nChildClassCountBeforeRemove = ACLASS(aChildClass, oChild)\n"
            "cChildClass1BeforeRemove = aChildClass[1]\n"
            "cChildClass2BeforeRemove = aChildClass[2]\n"
            "cChildClass3BeforeRemove = aChildClass[3]\n"
            "cChildClass4BeforeRemove = aChildClass[4]\n"
            "cChildClass5BeforeRemove = aChildClass[5]\n"
            "cChildCaptionBeforeRemove = oChild.Caption\n"
            "cSiblingCaptionBeforeRemove = oSibling.Caption\n"
            "cGrandchildCaptionBeforeRemove = oGrandchild.Caption\n"
            "lRemoved = oForm.RemoveObject('cmdSave')\n"
            "lRemovedMissing = oForm.RemoveObject('cmdSave')\n"
            "lHasChildAfterRemove = PEMSTATUS(oForm, 'cmdSave', 1)\n"
            "xRemovedChild = GETPEM(oForm, 'cmdSave')\n"
            "lHasSiblingAfterRemove = PEMSTATUS(oForm, 'cmdCancel', 1)\n"
            "cSiblingCaptionAfterRemove = oForm.cmdCancel.Caption\n"
            "lChildHasParentAfterRemove = PEMSTATUS(oChild, 'Parent', 1)\n"
            "xChildParentAfterRemove = GETPEM(oChild, 'Parent')\n"
            "lChildHasClassLibraryAfterRemove = PEMSTATUS(oChild, 'ClassLibrary', 1)\n"
            "xChildClassLibraryAfterRemove = GETPEM(oChild, 'ClassLibrary')\n"
            "lChildHasGrandchildAfterRemove = PEMSTATUS(oChild, 'lblBadge', 1)\n"
            "cChildCaptionAfterRemove = oChild.Caption\n"
            "cGrandchildCaptionAfterRemove = oGrandchild.Caption\n"
            "lGrandchildHasParentAfterRemove = PEMSTATUS(oGrandchild, 'Parent', 1)\n"
            "cGrandchildParentCaptionAfterRemove = oGrandchild.Parent.Caption\n"
            "cGrandchildFromChildAfterRemove = oChild.lblBadge.Caption\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 123)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    OBJECT cmdSave AS SaveButton OF buttons.prg\n"
            "        Caption = 'Commit'\n"
            "    ENDOBJECT\n"
            "    OBJECT cmdCancel AS CancelButton\n"
            "        Caption = 'Cancel'\n"
            "    ENDOBJECT\n"
            "ENDDEFINE\n"
            "DEFINE CLASS CancelButton AS Custom\n"
            "    Caption = 'Cancel'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native REMOVEOBJECT object-block external child subtree script should complete: ") + state.message +
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

        check("cchildclassbeforeremove", "SaveButton");
        check("cchildbaseclassbeforeremove", "ParentButton");
        check("cchildparentclassbeforeremove", "ParentButton");
        check("lchildhasclasslibrarybeforeremove", "false");
        check("nchildclasscountbeforeremove", "5");
        check("cchildclass1beforeremove", "SAVEBUTTON");
        check("cchildclass2beforeremove", "PARENTBUTTON");
        check("cchildclass3beforeremove", "ROOTBUTTON");
        check("cchildclass4beforeremove", "CUSTOM");
        check("cchildclass5beforeremove", "OBJECT");
        check("cchildcaptionbeforeremove", "Commit");
        check("csiblingcaptionbeforeremove", "Cancel");
        check("cgrandchildcaptionbeforeremove", "Badge");
        check("lremoved", "true");
        check("lremovedmissing", "false");
        check("lhaschildafterremove", "false");
        check("lhassiblingafterremove", "true");
        check("csiblingcaptionafterremove", "Cancel");
        check("lchildhasparentafterremove", "false");
        check("lchildhasclasslibraryafterremove", "false");
        check("lchildhasgrandchildafterremove", "true");
        check("cchildcaptionafterremove", "Commit");
        check("cgrandchildcaptionafterremove", "Badge");
        check("lgrandchildhasparentafterremove", "true");
        check("cgrandchildparentcaptionafterremove", "Commit");
        check("cgrandchildfromchildafterremove", "Badge");
        check("ldictset", "true");
        check("ndictcompare", "123");

        const auto child_class_library_before_remove = state.globals.find("xchildclasslibrarybeforeremove");
        expect(child_class_library_before_remove != state.globals.end() &&
                   child_class_library_before_remove->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native REMOVEOBJECT object-block external child subtree should leave the derived child ClassLibrary empty before removal");

        const auto removed_child = state.globals.find("xremovedchild");
        expect(removed_child != state.globals.end() &&
                   removed_child->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native REMOVEOBJECT object-block external child subtree should invalidate GETPEM() on the owner's removed child slot");

        const auto child_parent_after_remove = state.globals.find("xchildparentafterremove");
        expect(child_parent_after_remove != state.globals.end() &&
                   child_parent_after_remove->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native REMOVEOBJECT object-block external child subtree should clear the detached child's Parent");

        const auto child_class_library_after_remove = state.globals.find("xchildclasslibraryafterremove");
        expect(child_class_library_after_remove != state.globals.end() &&
                   child_class_library_after_remove->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native REMOVEOBJECT object-block external child subtree should keep the detached child's derived ClassLibrary empty");

        expect(state.ole_objects.size() == 5U,
               "native REMOVEOBJECT object-block external child subtree should register owner, detached child subtree, surviving sibling, and COM objects");
        if (state.ole_objects.size() == 5U)
        {
            const auto &owner_object = state.ole_objects[0];
            const auto &child_object = state.ole_objects[1];
            const auto &grandchild_object = state.ole_objects[2];
            const auto &sibling_object = state.ole_objects[3];
            expect(owner_object.prog_id == "DemoForm",
                   "native REMOVEOBJECT object-block external child subtree should preserve owner identity");
            expect(!owner_object.properties.contains("cmdsave"),
                   "native REMOVEOBJECT object-block external child subtree should detach the child from the owner");
            expect(owner_object.properties.contains("cmdcancel"),
                   "native REMOVEOBJECT object-block external child subtree should preserve the surviving sibling on the owner");
            expect(child_object.prog_id == "SaveButton",
                   "native REMOVEOBJECT object-block external child subtree should preserve child identity");
            expect(child_object.base_class_name == "ParentButton",
                   "native REMOVEOBJECT object-block external child subtree should preserve the detached child's immediate base-class identity");
            expect(child_object.source == button_library_path.string(),
                   "native REMOVEOBJECT object-block external child subtree should preserve the detached child's defining source path");
            expect(child_object.class_library.empty(),
                   "native REMOVEOBJECT object-block external child subtree should keep the detached child's derived ClassLibrary empty");
            expect(!child_object.properties.contains("parent"),
                   "native REMOVEOBJECT object-block external child subtree should clear the detached child's parent property");
            expect(child_object.properties.contains("lblbadge"),
                   "native REMOVEOBJECT object-block external child subtree should preserve the held grandchild on the detached child");
            expect(grandchild_object.prog_id == "BadgeLabel",
                   "native REMOVEOBJECT object-block external child subtree should preserve detached descendant identity");
            expect(sibling_object.prog_id == "CancelButton",
                   "native REMOVEOBJECT object-block external child subtree should preserve the surviving sibling object");
            expect(state.ole_objects[4].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native REMOVEOBJECT object-block external child subtree lands");
        }

        const bool has_removeobject_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.removeobject" &&
                   event.detail == "DemoForm.cmdsave";
        });
        expect(has_removeobject_event,
               "native REMOVEOBJECT object-block external child subtree should emit a detachment event");

        const bool has_child_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "SaveButton.Destroy";
        });
        expect(!has_child_destroy_event,
               "native REMOVEOBJECT object-block external child subtree should not destroy the detached child");

        const bool has_grandchild_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "BadgeLabel.Destroy";
        });
        expect(!has_grandchild_destroy_event,
               "native REMOVEOBJECT object-block external child subtree should not destroy the detached descendant");

        fs::remove_all(temp_root, ignored);
    }

}
