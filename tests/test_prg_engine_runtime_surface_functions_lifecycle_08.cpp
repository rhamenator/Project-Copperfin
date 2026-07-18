#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_removeobject_detaches_runtime_created_external_base_subtree_and_preserves_classlibrary_provenance()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_removeobject_runtime_created_external_base_subtree";
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

        const fs::path main_path = temp_root / "native_removeobject_runtime_created_external_base_subtree.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('DemoForm')\n"
            "oChild = oForm.cmdSave\n"
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
            "lRemoved = oForm.RemoveObject('cmdSave')\n"
            "lRemovedMissing = oForm.RemoveObject('cmdSave')\n"
            "lHasChildAfterRemove = PEMSTATUS(oForm, 'cmdSave', 1)\n"
            "xRemovedChild = GETPEM(oForm, 'cmdSave')\n"
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
            "lDictSet = SETPEM(oDict, 'comparemode', 126)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS ParentButton OF buttons.prg\n"
            "    Caption = 'Save'\n"
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
               std::string("native REMOVEOBJECT runtime-created external-base subtree script should complete: ") + state.message +
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
        check("ndictcompare", "126");

        const auto removed_child = state.globals.find("xremovedchild");
        expect(removed_child != state.globals.end() &&
                   removed_child->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native REMOVEOBJECT runtime-created external-base subtree should invalidate GETPEM() on the owner's removed child slot");

        const auto child_parent_after_remove = state.globals.find("xchildparentafterremove");
        expect(child_parent_after_remove != state.globals.end() &&
                   child_parent_after_remove->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native REMOVEOBJECT runtime-created external-base subtree should clear the detached child's Parent");

        expect(state.ole_objects.size() == 4U,
               "native REMOVEOBJECT runtime-created external-base subtree should register owner, detached child subtree, and COM objects");
        if (state.ole_objects.size() == 4U)
        {
            const auto &owner_object = state.ole_objects[0];
            const auto &child_object = state.ole_objects[1];
            const auto &grandchild_object = state.ole_objects[2];
            expect(owner_object.prog_id == "DemoForm",
                   "native REMOVEOBJECT runtime-created external-base subtree should preserve owner identity");
            expect(!owner_object.properties.contains("cmdsave"),
                   "native REMOVEOBJECT runtime-created external-base subtree should detach the child from the owner");
            expect(child_object.prog_id == "SaveButton",
                   "native REMOVEOBJECT runtime-created external-base subtree should preserve child identity");
            expect(child_object.base_class_name == "ParentButton",
                   "native REMOVEOBJECT runtime-created external-base subtree should preserve the detached child's immediate base-class identity");
            expect(child_object.source == main_path.string(),
                   "native REMOVEOBJECT runtime-created external-base subtree should preserve the detached child's defining source path");
            expect(child_object.class_library == button_library_path.string(),
                   "native REMOVEOBJECT runtime-created external-base subtree should preserve the detached child's immediate external ClassLibrary path");
            expect(!child_object.properties.contains("parent"),
                   "native REMOVEOBJECT runtime-created external-base subtree should clear the detached child's parent property");
            expect(child_object.properties.contains("lblbadge"),
                   "native REMOVEOBJECT runtime-created external-base subtree should preserve the held grandchild on the detached child");
            expect(grandchild_object.prog_id == "BadgeLabel",
                   "native REMOVEOBJECT runtime-created external-base subtree should preserve detached descendant identity");
            expect(state.ole_objects[3].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native REMOVEOBJECT runtime-created external-base subtree lands");
        }

        const bool has_removeobject_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.removeobject" &&
                   event.detail == "DemoForm.cmdsave";
        });
        expect(has_removeobject_event,
               "native REMOVEOBJECT runtime-created external-base subtree should emit a detachment event");

        const bool has_child_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "SaveButton.Destroy";
        });
        expect(!has_child_destroy_event,
               "native REMOVEOBJECT runtime-created external-base subtree should not destroy the detached child");

        const bool has_grandchild_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "BadgeLabel.Destroy";
        });
        expect(!has_grandchild_destroy_event,
               "native REMOVEOBJECT runtime-created external-base subtree should not destroy the detached descendant");

        fs::remove_all(temp_root, ignored);
    }

}
