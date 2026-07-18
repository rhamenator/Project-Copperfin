#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_removeobject_detaches_runtime_created_same_prg_subtree_and_preserves_held_subtree_state()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_removeobject_runtime_created_same_prg_subtree";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_removeobject_runtime_created_same_prg_subtree.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('DemoForm')\n"
            "oChild = oForm.cmdSave\n"
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
            "cGrandchildCaptionBeforeRemove = oGrandchild.Caption\n"
            "lRemoved = oForm.RemoveObject('cmdSave')\n"
            "lRemovedMissing = oForm.RemoveObject('cmdSave')\n"
            "lHasChildAfterRemove = PEMSTATUS(oForm, 'cmdSave', 1)\n"
            "xRemovedChild = GETPEM(oForm, 'cmdSave')\n"
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
            "lDictSet = SETPEM(oDict, 'comparemode', 129)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ParentButton AS Custom\n"
            "    Caption = 'Save'\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS ParentButton\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('lblBadge', 'BadgeLabel')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS BadgeLabel AS Custom\n"
            "    Caption = 'Badge'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native REMOVEOBJECT runtime-created same-PRG subtree script should complete: ") + state.message +
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
        check("cchildcaptionbeforeremove", "Save");
        check("cgrandchildcaptionbeforeremove", "Badge");
        check("lremoved", "true");
        check("lremovedmissing", "false");
        check("lhaschildafterremove", "false");
        check("lchildhasparentafterremove", "false");
        check("lchildhasclasslibraryafterremove", "false");
        check("lchildhasgrandchildafterremove", "true");
        check("cchildcaptionafterremove", "Save");
        check("cgrandchildcaptionafterremove", "Badge");
        check("lgrandchildhasparentafterremove", "true");
        check("cgrandchildparentcaptionafterremove", "Save");
        check("cgrandchildfromchildafterremove", "Badge");
        check("ldictset", "true");
        check("ndictcompare", "129");

        const auto child_class_library_before_remove = state.globals.find("xchildclasslibrarybeforeremove");
        expect(child_class_library_before_remove != state.globals.end() &&
                   child_class_library_before_remove->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native REMOVEOBJECT runtime-created same-PRG subtree should leave the child ClassLibrary empty before removal");

        const auto removed_child = state.globals.find("xremovedchild");
        expect(removed_child != state.globals.end() &&
                   removed_child->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native REMOVEOBJECT runtime-created same-PRG subtree should invalidate GETPEM() on the owner's removed child slot");

        const auto child_parent_after_remove = state.globals.find("xchildparentafterremove");
        expect(child_parent_after_remove != state.globals.end() &&
                   child_parent_after_remove->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native REMOVEOBJECT runtime-created same-PRG subtree should clear the detached child's Parent");

        const auto child_class_library_after_remove = state.globals.find("xchildclasslibraryafterremove");
        expect(child_class_library_after_remove != state.globals.end() &&
                   child_class_library_after_remove->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native REMOVEOBJECT runtime-created same-PRG subtree should keep the detached child's ClassLibrary empty");

        expect(state.ole_objects.size() == 4U,
               "native REMOVEOBJECT runtime-created same-PRG subtree should register owner, detached child subtree, and COM objects");
        if (state.ole_objects.size() == 4U)
        {
            const auto &owner_object = state.ole_objects[0];
            const auto &child_object = state.ole_objects[1];
            const auto &grandchild_object = state.ole_objects[2];
            expect(owner_object.prog_id == "DemoForm",
                   "native REMOVEOBJECT runtime-created same-PRG subtree should preserve owner identity");
            expect(!owner_object.properties.contains("cmdsave"),
                   "native REMOVEOBJECT runtime-created same-PRG subtree should detach the child from the owner");
            expect(child_object.prog_id == "SaveButton",
                   "native REMOVEOBJECT runtime-created same-PRG subtree should preserve child identity");
            expect(child_object.base_class_name == "ParentButton",
                   "native REMOVEOBJECT runtime-created same-PRG subtree should preserve the detached child's immediate base-class identity");
            expect(child_object.source == main_path.string(),
                   "native REMOVEOBJECT runtime-created same-PRG subtree should preserve the detached child's defining source path");
            expect(child_object.class_library.empty(),
                   "native REMOVEOBJECT runtime-created same-PRG subtree should keep the detached child's ClassLibrary empty");
            expect(!child_object.properties.contains("parent"),
                   "native REMOVEOBJECT runtime-created same-PRG subtree should clear the detached child's parent property");
            expect(child_object.properties.contains("lblbadge"),
                   "native REMOVEOBJECT runtime-created same-PRG subtree should preserve the held grandchild on the detached child");
            expect(grandchild_object.prog_id == "BadgeLabel",
                   "native REMOVEOBJECT runtime-created same-PRG subtree should preserve detached descendant identity");
            expect(state.ole_objects[3].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native REMOVEOBJECT runtime-created same-PRG subtree lands");
        }

        const bool has_removeobject_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.removeobject" &&
                   event.detail == "DemoForm.cmdsave";
        });
        expect(has_removeobject_event,
               "native REMOVEOBJECT runtime-created same-PRG subtree should emit a detachment event");

        const bool has_child_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "SaveButton.Destroy";
        });
        expect(!has_child_destroy_event,
               "native REMOVEOBJECT runtime-created same-PRG subtree should not destroy the detached child");

        const bool has_grandchild_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "BadgeLabel.Destroy";
        });
        expect(!has_grandchild_destroy_event,
               "native REMOVEOBJECT runtime-created same-PRG subtree should not destroy the detached descendant");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_removeobject_detaches_declarative_same_prg_subtree_and_preserves_held_subtree_state()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_removeobject_declarative_same_prg_subtree";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_removeobject_declarative_same_prg_subtree.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('DemoForm')\n"
            "oChild = oForm.cmdSave\n"
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
            "cGrandchildCaptionBeforeRemove = oGrandchild.Caption\n"
            "lRemoved = oForm.RemoveObject('cmdSave')\n"
            "lRemovedMissing = oForm.RemoveObject('cmdSave')\n"
            "lHasChildAfterRemove = PEMSTATUS(oForm, 'cmdSave', 1)\n"
            "xRemovedChild = GETPEM(oForm, 'cmdSave')\n"
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
            "lDictSet = SETPEM(oDict, 'comparemode', 131)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ParentButton AS Custom\n"
            "    Caption = 'Save'\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS ParentButton\n"
            "    ADD OBJECT lblBadge AS BadgeLabel\n"
            "ENDDEFINE\n"
            "DEFINE CLASS BadgeLabel AS Custom\n"
            "    Caption = 'Badge'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native REMOVEOBJECT declarative same-PRG subtree script should complete: ") + state.message +
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
        check("cchildcaptionbeforeremove", "Save");
        check("cgrandchildcaptionbeforeremove", "Badge");
        check("lremoved", "true");
        check("lremovedmissing", "false");
        check("lhaschildafterremove", "false");
        check("lchildhasparentafterremove", "false");
        check("lchildhasclasslibraryafterremove", "false");
        check("lchildhasgrandchildafterremove", "true");
        check("cchildcaptionafterremove", "Save");
        check("cgrandchildcaptionafterremove", "Badge");
        check("lgrandchildhasparentafterremove", "true");
        check("cgrandchildparentcaptionafterremove", "Save");
        check("cgrandchildfromchildafterremove", "Badge");
        check("ldictset", "true");
        check("ndictcompare", "131");

        const auto child_class_library_before_remove = state.globals.find("xchildclasslibrarybeforeremove");
        expect(child_class_library_before_remove != state.globals.end() &&
                   child_class_library_before_remove->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native REMOVEOBJECT declarative same-PRG subtree should leave the child ClassLibrary empty before removal");

        const auto removed_child = state.globals.find("xremovedchild");
        expect(removed_child != state.globals.end() &&
                   removed_child->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native REMOVEOBJECT declarative same-PRG subtree should invalidate GETPEM() on the owner's removed child slot");

        const auto child_parent_after_remove = state.globals.find("xchildparentafterremove");
        expect(child_parent_after_remove != state.globals.end() &&
                   child_parent_after_remove->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native REMOVEOBJECT declarative same-PRG subtree should clear the detached child's Parent");

        const auto child_class_library_after_remove = state.globals.find("xchildclasslibraryafterremove");
        expect(child_class_library_after_remove != state.globals.end() &&
                   child_class_library_after_remove->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native REMOVEOBJECT declarative same-PRG subtree should keep the detached child's ClassLibrary empty");

        expect(state.ole_objects.size() == 4U,
               "native REMOVEOBJECT declarative same-PRG subtree should register owner, detached child subtree, and COM objects");
        if (state.ole_objects.size() == 4U)
        {
            const auto &owner_object = state.ole_objects[0];
            const auto &child_object = state.ole_objects[1];
            const auto &grandchild_object = state.ole_objects[2];
            expect(owner_object.prog_id == "DemoForm",
                   "native REMOVEOBJECT declarative same-PRG subtree should preserve owner identity");
            expect(!owner_object.properties.contains("cmdsave"),
                   "native REMOVEOBJECT declarative same-PRG subtree should detach the child from the owner");
            expect(child_object.prog_id == "SaveButton",
                   "native REMOVEOBJECT declarative same-PRG subtree should preserve child identity");
            expect(child_object.base_class_name == "ParentButton",
                   "native REMOVEOBJECT declarative same-PRG subtree should preserve the detached child's immediate base-class identity");
            expect(child_object.source == main_path.string(),
                   "native REMOVEOBJECT declarative same-PRG subtree should preserve the detached child's defining source path");
            expect(child_object.class_library.empty(),
                   "native REMOVEOBJECT declarative same-PRG subtree should keep the detached child's ClassLibrary empty");
            expect(!child_object.properties.contains("parent"),
                   "native REMOVEOBJECT declarative same-PRG subtree should clear the detached child's parent property");
            expect(child_object.properties.contains("lblbadge"),
                   "native REMOVEOBJECT declarative same-PRG subtree should preserve the held grandchild on the detached child");
            expect(grandchild_object.prog_id == "BadgeLabel",
                   "native REMOVEOBJECT declarative same-PRG subtree should preserve detached descendant identity");
            expect(state.ole_objects[3].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native REMOVEOBJECT declarative same-PRG subtree lands");
        }

        const bool has_removeobject_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.removeobject" &&
                   event.detail == "DemoForm.cmdsave";
        });
        expect(has_removeobject_event,
               "native REMOVEOBJECT declarative same-PRG subtree should emit a detachment event");

        const bool has_child_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "SaveButton.Destroy";
        });
        expect(!has_child_destroy_event,
               "native REMOVEOBJECT declarative same-PRG subtree should not destroy the detached child");

        const bool has_grandchild_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "BadgeLabel.Destroy";
        });
        expect(!has_grandchild_destroy_event,
               "native REMOVEOBJECT declarative same-PRG subtree should not destroy the detached descendant");

        fs::remove_all(temp_root, ignored);
    }

    void test_inherited_removeobject_detaches_declarative_same_prg_subtree_and_preserves_held_subtree_state()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_inherited_removeobject_declarative_same_prg_subtree";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "inherited_removeobject_declarative_same_prg_subtree.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oChild = oCreate.cmdSave\n"
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
            "cGrandchildCaptionBeforeRemove = oGrandchild.Caption\n"
            "lRemoved = oCreate.RemoveObject('cmdSave')\n"
            "lRemovedMissing = oCreate.RemoveObject('cmdSave')\n"
            "lHasChildAfterRemove = PEMSTATUS(oCreate, 'cmdSave', 1)\n"
            "xRemovedChild = GETPEM(oCreate, 'cmdSave')\n"
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
            "lDictSet = SETPEM(oDict, 'comparemode', 129)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ParentForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ChildForm AS ParentForm\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ParentButton AS Custom\n"
            "    Caption = 'Save'\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS ParentButton\n"
            "    ADD OBJECT lblBadge AS BadgeLabel\n"
            "ENDDEFINE\n"
            "DEFINE CLASS BadgeLabel AS Custom\n"
            "    Caption = 'Badge'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("inherited REMOVEOBJECT declarative same-PRG subtree script should complete: ") + state.message +
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
        check("cchildcaptionbeforeremove", "Save");
        check("cgrandchildcaptionbeforeremove", "Badge");
        check("lremoved", "true");
        check("lremovedmissing", "false");
        check("lhaschildafterremove", "false");
        check("lchildhasparentafterremove", "false");
        check("lchildhasclasslibraryafterremove", "false");
        check("lchildhasgrandchildafterremove", "true");
        check("cchildcaptionafterremove", "Save");
        check("cgrandchildcaptionafterremove", "Badge");
        check("lgrandchildhasparentafterremove", "true");
        check("cgrandchildparentcaptionafterremove", "Save");
        check("cgrandchildfromchildafterremove", "Badge");
        check("ldictset", "true");
        check("ndictcompare", "129");

        const auto child_class_library_before_remove = state.globals.find("xchildclasslibrarybeforeremove");
        expect(child_class_library_before_remove != state.globals.end() &&
                   child_class_library_before_remove->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited REMOVEOBJECT declarative same-PRG subtree should leave the child ClassLibrary empty before removal");

        const auto removed_child = state.globals.find("xremovedchild");
        expect(removed_child != state.globals.end() &&
                   removed_child->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited REMOVEOBJECT declarative same-PRG subtree should invalidate GETPEM() on the owner's removed child slot");

        const auto child_parent_after_remove = state.globals.find("xchildparentafterremove");
        expect(child_parent_after_remove != state.globals.end() &&
                   child_parent_after_remove->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited REMOVEOBJECT declarative same-PRG subtree should clear the detached child's Parent");

        const auto child_class_library_after_remove = state.globals.find("xchildclasslibraryafterremove");
        expect(child_class_library_after_remove != state.globals.end() &&
                   child_class_library_after_remove->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited REMOVEOBJECT declarative same-PRG subtree should keep the detached child's ClassLibrary empty");

        expect(state.ole_objects.size() == 4U,
               "inherited REMOVEOBJECT declarative same-PRG subtree should register owner, detached child subtree, and COM objects");
        if (state.ole_objects.size() == 4U)
        {
            const auto &owner_object = state.ole_objects[0];
            const auto &child_object = state.ole_objects[1];
            const auto &grandchild_object = state.ole_objects[2];
            expect(owner_object.prog_id == "ChildForm",
                   "inherited REMOVEOBJECT declarative same-PRG subtree should preserve derived owner identity");
            expect(!owner_object.properties.contains("cmdsave"),
                   "inherited REMOVEOBJECT declarative same-PRG subtree should detach the child from the derived owner");
            expect(child_object.prog_id == "SaveButton",
                   "inherited REMOVEOBJECT declarative same-PRG subtree should preserve child identity");
            expect(child_object.base_class_name == "ParentButton",
                   "inherited REMOVEOBJECT declarative same-PRG subtree should preserve the detached child's immediate base-class identity");
            expect(child_object.source == main_path.string(),
                   "inherited REMOVEOBJECT declarative same-PRG subtree should preserve the detached child's defining source path");
            expect(child_object.class_library.empty(),
                   "inherited REMOVEOBJECT declarative same-PRG subtree should keep the detached child's ClassLibrary empty");
            expect(!child_object.properties.contains("parent"),
                   "inherited REMOVEOBJECT declarative same-PRG subtree should clear the detached child's parent property");
            expect(child_object.properties.contains("lblbadge"),
                   "inherited REMOVEOBJECT declarative same-PRG subtree should preserve the held grandchild on the detached child");
            expect(grandchild_object.prog_id == "BadgeLabel",
                   "inherited REMOVEOBJECT declarative same-PRG subtree should preserve detached descendant identity");
            expect(state.ole_objects[3].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while inherited REMOVEOBJECT declarative same-PRG subtree lands");
        }

        const bool has_removeobject_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.removeobject" &&
                   event.detail == "ChildForm.cmdsave";
        });
        expect(has_removeobject_event,
               "inherited REMOVEOBJECT declarative same-PRG subtree should emit a detachment event");

        const bool has_child_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "SaveButton.Destroy";
        });
        expect(!has_child_destroy_event,
               "inherited REMOVEOBJECT declarative same-PRG subtree should not destroy the detached child");

        const bool has_grandchild_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "BadgeLabel.Destroy";
        });
        expect(!has_grandchild_destroy_event,
               "inherited REMOVEOBJECT declarative same-PRG subtree should not destroy the detached descendant");

        fs::remove_all(temp_root, ignored);
    }

    void test_inherited_removeobject_detaches_runtime_created_same_prg_subtree_and_preserves_held_subtree_state()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_inherited_removeobject_runtime_created_same_prg_subtree";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "inherited_removeobject_runtime_created_same_prg_subtree.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oChild = oCreate.cmdSave\n"
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
            "cGrandchildCaptionBeforeRemove = oGrandchild.Caption\n"
            "lRemoved = oCreate.RemoveObject('cmdSave')\n"
            "lRemovedMissing = oCreate.RemoveObject('cmdSave')\n"
            "lHasChildAfterRemove = PEMSTATUS(oCreate, 'cmdSave', 1)\n"
            "xRemovedChild = GETPEM(oCreate, 'cmdSave')\n"
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
            "lDictSet = SETPEM(oDict, 'comparemode', 130)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ParentForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ChildForm AS ParentForm\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ParentButton AS Custom\n"
            "    Caption = 'Save'\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS ParentButton\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('lblBadge', 'BadgeLabel')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS BadgeLabel AS Custom\n"
            "    Caption = 'Badge'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("inherited REMOVEOBJECT runtime-created same-PRG subtree script should complete: ") + state.message +
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
        check("cchildcaptionbeforeremove", "Save");
        check("cgrandchildcaptionbeforeremove", "Badge");
        check("lremoved", "true");
        check("lremovedmissing", "false");
        check("lhaschildafterremove", "false");
        check("lchildhasparentafterremove", "false");
        check("lchildhasclasslibraryafterremove", "false");
        check("lchildhasgrandchildafterremove", "true");
        check("cchildcaptionafterremove", "Save");
        check("cgrandchildcaptionafterremove", "Badge");
        check("lgrandchildhasparentafterremove", "true");
        check("cgrandchildparentcaptionafterremove", "Save");
        check("cgrandchildfromchildafterremove", "Badge");
        check("ldictset", "true");
        check("ndictcompare", "130");

        const auto child_class_library_before_remove = state.globals.find("xchildclasslibrarybeforeremove");
        expect(child_class_library_before_remove != state.globals.end() &&
                   child_class_library_before_remove->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited REMOVEOBJECT runtime-created same-PRG subtree should leave the child ClassLibrary empty before removal");

        const auto removed_child = state.globals.find("xremovedchild");
        expect(removed_child != state.globals.end() &&
                   removed_child->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited REMOVEOBJECT runtime-created same-PRG subtree should invalidate GETPEM() on the owner's removed child slot");

        const auto child_parent_after_remove = state.globals.find("xchildparentafterremove");
        expect(child_parent_after_remove != state.globals.end() &&
                   child_parent_after_remove->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited REMOVEOBJECT runtime-created same-PRG subtree should clear the detached child's Parent");

        const auto child_class_library_after_remove = state.globals.find("xchildclasslibraryafterremove");
        expect(child_class_library_after_remove != state.globals.end() &&
                   child_class_library_after_remove->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited REMOVEOBJECT runtime-created same-PRG subtree should keep the detached child's ClassLibrary empty");

        expect(state.ole_objects.size() == 4U,
               "inherited REMOVEOBJECT runtime-created same-PRG subtree should register owner, detached child subtree, and COM objects");
        if (state.ole_objects.size() == 4U)
        {
            const auto &owner_object = state.ole_objects[0];
            const auto &child_object = state.ole_objects[1];
            const auto &grandchild_object = state.ole_objects[2];
            expect(owner_object.prog_id == "ChildForm",
                   "inherited REMOVEOBJECT runtime-created same-PRG subtree should preserve derived owner identity");
            expect(!owner_object.properties.contains("cmdsave"),
                   "inherited REMOVEOBJECT runtime-created same-PRG subtree should detach the child from the derived owner");
            expect(child_object.prog_id == "SaveButton",
                   "inherited REMOVEOBJECT runtime-created same-PRG subtree should preserve child identity");
            expect(child_object.base_class_name == "ParentButton",
                   "inherited REMOVEOBJECT runtime-created same-PRG subtree should preserve the detached child's immediate base-class identity");
            expect(child_object.source == main_path.string(),
                   "inherited REMOVEOBJECT runtime-created same-PRG subtree should preserve the detached child's defining source path");
            expect(child_object.class_library.empty(),
                   "inherited REMOVEOBJECT runtime-created same-PRG subtree should keep the detached child's ClassLibrary empty");
            expect(!child_object.properties.contains("parent"),
                   "inherited REMOVEOBJECT runtime-created same-PRG subtree should clear the detached child's parent property");
            expect(child_object.properties.contains("lblbadge"),
                   "inherited REMOVEOBJECT runtime-created same-PRG subtree should preserve the held grandchild on the detached child");
            expect(grandchild_object.prog_id == "BadgeLabel",
                   "inherited REMOVEOBJECT runtime-created same-PRG subtree should preserve detached descendant identity");
            expect(state.ole_objects[3].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while inherited REMOVEOBJECT runtime-created same-PRG subtree lands");
        }

        const bool has_removeobject_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.removeobject" &&
                   event.detail == "ChildForm.cmdsave";
        });
        expect(has_removeobject_event,
               "inherited REMOVEOBJECT runtime-created same-PRG subtree should emit a detachment event");

        const bool has_child_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "SaveButton.Destroy";
        });
        expect(!has_child_destroy_event,
               "inherited REMOVEOBJECT runtime-created same-PRG subtree should not destroy the detached child");

        const bool has_grandchild_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "BadgeLabel.Destroy";
        });
        expect(!has_grandchild_destroy_event,
               "inherited REMOVEOBJECT runtime-created same-PRG subtree should not destroy the detached descendant");

        fs::remove_all(temp_root, ignored);
    }

}
