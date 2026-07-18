#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_inherited_removeobject_detaches_runtime_created_external_child_subtree_and_preserves_empty_derived_classlibrary()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_inherited_removeobject_runtime_created_external_child_subtree";
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
            "    PROCEDURE Init\n"
            "        THIS.AddObject('lblBadge', 'BadgeLabel')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS BadgeLabel AS Custom\n"
            "    Caption = 'Badge'\n"
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
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "inherited_removeobject_runtime_created_external_child_subtree.prg";
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
            "xChildClassLibraryAfterRemove = GETPEM(oChild, 'ClassLibrary')\n"
            "lChildHasGrandchildAfterRemove = PEMSTATUS(oChild, 'lblBadge', 1)\n"
            "cChildCaptionAfterRemove = oChild.Caption\n"
            "cGrandchildCaptionAfterRemove = oGrandchild.Caption\n"
            "lGrandchildHasParentAfterRemove = PEMSTATUS(oGrandchild, 'Parent', 1)\n"
            "cGrandchildParentCaptionAfterRemove = oGrandchild.Parent.Caption\n"
            "cGrandchildFromChildAfterRemove = oChild.lblBadge.Caption\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 128)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("inherited REMOVEOBJECT runtime-created external-child subtree script should complete: ") + state.message +
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
        check("ndictcompare", "128");

        const auto child_class_library_before_remove = state.globals.find("xchildclasslibrarybeforeremove");
        expect(child_class_library_before_remove != state.globals.end() &&
                   child_class_library_before_remove->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited REMOVEOBJECT runtime-created external-child subtree should leave the derived child ClassLibrary empty before removal");

        const auto removed_child = state.globals.find("xremovedchild");
        expect(removed_child != state.globals.end() &&
                   removed_child->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited REMOVEOBJECT runtime-created external-child subtree should invalidate GETPEM() on the owner's removed child slot");

        const auto child_parent_after_remove = state.globals.find("xchildparentafterremove");
        expect(child_parent_after_remove != state.globals.end() &&
                   child_parent_after_remove->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited REMOVEOBJECT runtime-created external-child subtree should clear the detached child's Parent");

        const auto child_class_library_after_remove = state.globals.find("xchildclasslibraryafterremove");
        expect(child_class_library_after_remove != state.globals.end() &&
                   child_class_library_after_remove->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited REMOVEOBJECT runtime-created external-child subtree should keep the detached child's derived ClassLibrary empty");

        expect(state.ole_objects.size() == 4U,
               "inherited REMOVEOBJECT runtime-created external-child subtree should register owner, detached child subtree, and COM objects");
        if (state.ole_objects.size() == 4U)
        {
            const auto &owner_object = state.ole_objects[0];
            const auto &child_object = state.ole_objects[1];
            const auto &grandchild_object = state.ole_objects[2];
            expect(owner_object.prog_id == "ChildForm",
                   "inherited REMOVEOBJECT runtime-created external-child subtree should preserve derived owner identity");
            expect(!owner_object.properties.contains("cmdsave"),
                   "inherited REMOVEOBJECT runtime-created external-child subtree should detach the child from the derived owner");
            expect(child_object.prog_id == "SaveButton",
                   "inherited REMOVEOBJECT runtime-created external-child subtree should preserve child identity");
            expect(child_object.base_class_name == "ParentButton",
                   "inherited REMOVEOBJECT runtime-created external-child subtree should preserve the detached child's immediate base-class identity");
            expect(child_object.source == button_library_path.string(),
                   "inherited REMOVEOBJECT runtime-created external-child subtree should preserve the detached child's defining source path");
            expect(child_object.class_library.empty(),
                   "inherited REMOVEOBJECT runtime-created external-child subtree should keep the detached child's derived ClassLibrary empty");
            expect(!child_object.properties.contains("parent"),
                   "inherited REMOVEOBJECT runtime-created external-child subtree should clear the detached child's parent property");
            expect(child_object.properties.contains("lblbadge"),
                   "inherited REMOVEOBJECT runtime-created external-child subtree should preserve the held grandchild on the detached child");
            expect(grandchild_object.prog_id == "BadgeLabel",
                   "inherited REMOVEOBJECT runtime-created external-child subtree should preserve detached descendant identity");
            expect(state.ole_objects[3].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while inherited REMOVEOBJECT runtime-created external-child subtree lands");
        }

        const bool has_removeobject_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.removeobject" &&
                   event.detail == "ChildForm.cmdsave";
        });
        expect(has_removeobject_event,
               "inherited REMOVEOBJECT runtime-created external-child subtree should emit a detachment event");

        const bool has_child_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "SaveButton.Destroy";
        });
        expect(!has_child_destroy_event,
               "inherited REMOVEOBJECT runtime-created external-child subtree should not destroy the detached child");

        const bool has_grandchild_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "BadgeLabel.Destroy";
        });
        expect(!has_grandchild_destroy_event,
               "inherited REMOVEOBJECT runtime-created external-child subtree should not destroy the detached descendant");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_release_invokes_destroy_and_invalidates_standalone_object()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_release_standalone";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_release_standalone.prg";
        write_text(
            main_path,
            "nDestroyCount = 0\n"
            "oWidget = CREATEOBJECT('Widget')\n"
            "cCaptionBeforeRelease = oWidget.Caption\n"
            "lHadCaptionBeforeRelease = PEMSTATUS(oWidget, 'Caption', 1)\n"
            "lReleased = oWidget.Release()\n"
            "nDestroyCountAfter = nDestroyCount\n"
            "lHasCaptionAfterRelease = PEMSTATUS(oWidget, 'Caption', 1)\n"
            "xCaptionAfterRelease = GETPEM(oWidget, 'Caption')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 90)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS Widget AS Custom\n"
            "    Caption = 'Standalone'\n"
            "    PROCEDURE Destroy\n"
            "        nDestroyCount = nDestroyCount + 1\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Release standalone script should complete: ") + state.message +
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

        check("ccaptionbeforerelease", "Standalone");
        check("lhadcaptionbeforerelease", "true");
        check("lreleased", "true");
        check("ndestroycountafter", "1");
        check("lhascaptionafterrelease", "false");
        check("ldictset", "true");
        check("ndictcompare", "90");

        const auto caption_after_release = state.globals.find("xcaptionafterrelease");
        expect(caption_after_release != state.globals.end() &&
                   caption_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native Release standalone should invalidate GETPEM() on the released object");

        expect(state.ole_objects.size() == 1U,
               "native Release standalone should leave only the COM object registered after release");
        if (state.ole_objects.size() == 1U)
        {
            expect(state.ole_objects[0].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native standalone Release lands");
        }

        const bool has_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "Widget.Destroy";
        });
        expect(has_destroy_event,
               "native Release standalone should dispatch the native Destroy method");

        const bool has_release_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.release" &&
                   event.detail == "Widget";
        });
        expect(has_release_event,
               "native Release standalone should emit a release event");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_release_recursively_destroys_external_base_standalone_object_subtree_and_invalidates_released_chain()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_release_external_base_standalone_subtree";
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

        const fs::path main_path = temp_root / "native_release_external_base_standalone_subtree.prg";
        write_text(
            main_path,
            "cDestroyLog = ''\n"
            "cGrandchildParentCaptionDuringDestroy = ''\n"
            "nChildDestroyCount = 0\n"
            "nGrandchildDestroyCount = 0\n"
            "oWidget = CREATEOBJECT('SaveButton')\n"
            "oGrandchild = oWidget.lblBadge\n"
            "cChildClassBeforeRelease = oWidget.Class\n"
            "cChildBaseClassBeforeRelease = oWidget.BaseClass\n"
            "cChildParentClassBeforeRelease = oWidget.ParentClass\n"
            "cChildClassLibraryBeforeRelease = oWidget.ClassLibrary\n"
            "nChildClassCountBeforeRelease = ACLASS(aChildClass, oWidget)\n"
            "cChildClass1BeforeRelease = aChildClass[1]\n"
            "cChildClass2BeforeRelease = aChildClass[2]\n"
            "cChildClass3BeforeRelease = aChildClass[3]\n"
            "cChildClass4BeforeRelease = aChildClass[4]\n"
            "cChildClass5BeforeRelease = aChildClass[5]\n"
            "cChildCaptionBeforeRelease = oWidget.Caption\n"
            "cGrandchildCaptionBeforeRelease = oGrandchild.Caption\n"
            "lChildHasClassLibraryBeforeRelease = PEMSTATUS(oWidget, 'ClassLibrary', 1)\n"
            "lReleased = oWidget.Release()\n"
            "cDestroyLogAfter = cDestroyLog\n"
            "cGrandchildParentCaptionAfter = cGrandchildParentCaptionDuringDestroy\n"
            "nChildDestroyCountAfter = nChildDestroyCount\n"
            "nGrandchildDestroyCountAfter = nGrandchildDestroyCount\n"
            "lChildHasCaptionAfterRelease = PEMSTATUS(oWidget, 'Caption', 1)\n"
            "xChildCaptionAfterRelease = GETPEM(oWidget, 'Caption')\n"
            "lChildHasClassLibraryAfterRelease = PEMSTATUS(oWidget, 'ClassLibrary', 1)\n"
            "xChildClassLibraryAfterRelease = GETPEM(oWidget, 'ClassLibrary')\n"
            "lGrandchildHasParentAfterRelease = PEMSTATUS(oGrandchild, 'Parent', 1)\n"
            "xGrandchildParentAfterRelease = GETPEM(oGrandchild, 'Parent')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 123)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS BadgeLabel AS Custom\n"
            "    Caption = 'Badge'\n"
            "    PROCEDURE Destroy\n"
            "        cDestroyLog = cDestroyLog + 'badge>'\n"
            "        cGrandchildParentCaptionDuringDestroy = PARENT.Caption\n"
            "        nGrandchildDestroyCount = nGrandchildDestroyCount + 1\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS ParentButton OF buttons.prg\n"
            "    Caption = 'Save'\n"
            "    OBJECT lblBadge AS BadgeLabel\n"
            "        Caption = 'Badge'\n"
            "    ENDOBJECT\n"
            "    PROCEDURE Destroy\n"
            "        cDestroyLog = cDestroyLog + 'child>'\n"
            "        nChildDestroyCount = nChildDestroyCount + 1\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Release external-base standalone subtree script should complete: ") + state.message +
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

        check("cchildclassbeforerelease", "SaveButton");
        check("cchildbaseclassbeforerelease", "ParentButton");
        check("cchildparentclassbeforerelease", "ParentButton");
        check("cchildclasslibrarybeforerelease", button_library_path.string());
        check("nchildclasscountbeforerelease", "5");
        check("cchildclass1beforerelease", "SAVEBUTTON");
        check("cchildclass2beforerelease", "PARENTBUTTON");
        check("cchildclass3beforerelease", "ROOTBUTTON");
        check("cchildclass4beforerelease", "CUSTOM");
        check("cchildclass5beforerelease", "OBJECT");
        check("cchildcaptionbeforerelease", "Save");
        check("cgrandchildcaptionbeforerelease", "Badge");
        check("lchildhasclasslibrarybeforerelease", "true");
        check("lreleased", "true");
        check("cdestroylogafter", "badge>child>");
        check("cgrandchildparentcaptionafter", "Save");
        check("nchilddestroycountafter", "1");
        check("ngrandchilddestroycountafter", "1");
        check("lchildhascaptionafterrelease", "false");
        check("lchildhasclasslibraryafterrelease", "false");
        check("lgrandchildhasparentafterrelease", "false");
        check("ldictset", "true");
        check("ndictcompare", "123");

        const auto child_caption_after_release = state.globals.find("xchildcaptionafterrelease");
        expect(child_caption_after_release != state.globals.end() &&
                   child_caption_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native Release external-base standalone subtree should invalidate GETPEM() on the released object");

        const auto child_class_library_after_release = state.globals.find("xchildclasslibraryafterrelease");
        expect(child_class_library_after_release != state.globals.end() &&
                   child_class_library_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native Release external-base standalone subtree should invalidate GETPEM() on the released object's ClassLibrary");

        const auto grandchild_parent_after_release = state.globals.find("xgrandchildparentafterrelease");
        expect(grandchild_parent_after_release != state.globals.end() &&
                   grandchild_parent_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native Release external-base standalone subtree should invalidate GETPEM() on the released grandchild's Parent");

        expect(state.ole_objects.size() == 1U,
               "native Release external-base standalone subtree should leave only the COM object registered after release");
        if (state.ole_objects.size() == 1U)
        {
            expect(state.ole_objects[0].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native external-base standalone Release lands");
        }

        const bool has_grandchild_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "BadgeLabel.Destroy";
        });
        expect(has_grandchild_destroy_event,
               "native Release external-base standalone subtree should dispatch the grandchild Destroy method");

        const bool has_child_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "SaveButton.Destroy";
        });
        expect(has_child_destroy_event,
               "native Release external-base standalone subtree should dispatch the released object's Destroy method");

        const bool has_release_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.release" &&
                   event.detail == "SaveButton";
        });
        expect(has_release_event,
               "native Release external-base standalone subtree should emit a release event");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_release_recursively_destroys_external_child_standalone_object_subtree_and_invalidates_released_chain()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_release_external_child_standalone_subtree";
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
            "    PROCEDURE Destroy\n"
            "        cDestroyLog = cDestroyLog + 'child>'\n"
            "        nChildDestroyCount = nChildDestroyCount + 1\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS BadgeLabel AS Custom\n"
            "    Caption = 'Badge'\n"
            "    PROCEDURE Destroy\n"
            "        cDestroyLog = cDestroyLog + 'badge>'\n"
            "        cGrandchildParentCaptionDuringDestroy = PARENT.Caption\n"
            "        nGrandchildDestroyCount = nGrandchildDestroyCount + 1\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "native_release_external_child_standalone_subtree.prg";
        write_text(
            main_path,
            "cDestroyLog = ''\n"
            "cGrandchildParentCaptionDuringDestroy = ''\n"
            "nChildDestroyCount = 0\n"
            "nGrandchildDestroyCount = 0\n"
            "oWidget = NEWOBJECT('SaveButton', 'buttons.prg')\n"
            "oGrandchild = oWidget.lblBadge\n"
            "cChildClassBeforeRelease = oWidget.Class\n"
            "cChildBaseClassBeforeRelease = oWidget.BaseClass\n"
            "cChildParentClassBeforeRelease = oWidget.ParentClass\n"
            "cChildClassLibraryBeforeRelease = oWidget.ClassLibrary\n"
            "nChildClassCountBeforeRelease = ACLASS(aChildClass, oWidget)\n"
            "cChildClass1BeforeRelease = aChildClass[1]\n"
            "cChildClass2BeforeRelease = aChildClass[2]\n"
            "cChildClass3BeforeRelease = aChildClass[3]\n"
            "cChildClass4BeforeRelease = aChildClass[4]\n"
            "cChildClass5BeforeRelease = aChildClass[5]\n"
            "cChildCaptionBeforeRelease = oWidget.Caption\n"
            "cGrandchildCaptionBeforeRelease = oGrandchild.Caption\n"
            "lChildHasClassLibraryBeforeRelease = PEMSTATUS(oWidget, 'ClassLibrary', 1)\n"
            "lReleased = oWidget.Release()\n"
            "cDestroyLogAfter = cDestroyLog\n"
            "cGrandchildParentCaptionAfter = cGrandchildParentCaptionDuringDestroy\n"
            "nChildDestroyCountAfter = nChildDestroyCount\n"
            "nGrandchildDestroyCountAfter = nGrandchildDestroyCount\n"
            "lChildHasCaptionAfterRelease = PEMSTATUS(oWidget, 'Caption', 1)\n"
            "xChildCaptionAfterRelease = GETPEM(oWidget, 'Caption')\n"
            "lChildHasClassLibraryAfterRelease = PEMSTATUS(oWidget, 'ClassLibrary', 1)\n"
            "xChildClassLibraryAfterRelease = GETPEM(oWidget, 'ClassLibrary')\n"
            "lGrandchildHasParentAfterRelease = PEMSTATUS(oGrandchild, 'Parent', 1)\n"
            "xGrandchildParentAfterRelease = GETPEM(oGrandchild, 'Parent')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 124)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Release external-child standalone subtree script should complete: ") + state.message +
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

        check("cchildclassbeforerelease", "SaveButton");
        check("cchildbaseclassbeforerelease", "ParentButton");
        check("cchildparentclassbeforerelease", "ParentButton");
        check("cchildclasslibrarybeforerelease", "");
        check("nchildclasscountbeforerelease", "5");
        check("cchildclass1beforerelease", "SAVEBUTTON");
        check("cchildclass2beforerelease", "PARENTBUTTON");
        check("cchildclass3beforerelease", "ROOTBUTTON");
        check("cchildclass4beforerelease", "CUSTOM");
        check("cchildclass5beforerelease", "OBJECT");
        check("cchildcaptionbeforerelease", "Save");
        check("cgrandchildcaptionbeforerelease", "Badge");
        check("lchildhasclasslibrarybeforerelease", "false");
        check("lreleased", "true");
        check("cdestroylogafter", "badge>child>");
        check("cgrandchildparentcaptionafter", "Save");
        check("nchilddestroycountafter", "1");
        check("ngrandchilddestroycountafter", "1");
        check("lchildhascaptionafterrelease", "false");
        check("lchildhasclasslibraryafterrelease", "false");
        check("lgrandchildhasparentafterrelease", "false");
        check("ldictset", "true");
        check("ndictcompare", "124");

        const auto child_caption_after_release = state.globals.find("xchildcaptionafterrelease");
        expect(child_caption_after_release != state.globals.end() &&
                   child_caption_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native Release external-child standalone subtree should invalidate GETPEM() on the released object");

        const auto child_class_library_after_release = state.globals.find("xchildclasslibraryafterrelease");
        expect(child_class_library_after_release != state.globals.end() &&
                   child_class_library_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native Release external-child standalone subtree should invalidate GETPEM() on the released object's ClassLibrary");

        const auto grandchild_parent_after_release = state.globals.find("xgrandchildparentafterrelease");
        expect(grandchild_parent_after_release != state.globals.end() &&
                   grandchild_parent_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native Release external-child standalone subtree should invalidate GETPEM() on the released grandchild's Parent");

        expect(state.ole_objects.size() == 1U,
               "native Release external-child standalone subtree should leave only the COM object registered after release");
        if (state.ole_objects.size() == 1U)
        {
            expect(state.ole_objects[0].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native external-child standalone Release lands");
        }

        const bool has_grandchild_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "BadgeLabel.Destroy";
        });
        expect(has_grandchild_destroy_event,
               "native Release external-child standalone subtree should dispatch the grandchild Destroy method");

        const bool has_child_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "SaveButton.Destroy";
        });
        expect(has_child_destroy_event,
               "native Release external-child standalone subtree should dispatch the released object's Destroy method");

        const bool has_release_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.release" &&
                   event.detail == "SaveButton";
        });
        expect(has_release_event,
               "native Release external-child standalone subtree should emit a release event");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_release_recursively_destroys_contained_children_and_invalidates_owner_chain()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_release_contained";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_release_contained.prg";
        write_text(
            main_path,
            "cDestroyLog = ''\n"
            "cChildParentCaptionDuringDestroy = ''\n"
            "nFormDestroyCount = 0\n"
            "nChildDestroyCount = 0\n"
            "oForm = CREATEOBJECT('DemoForm')\n"
            "oChild = oForm.cmdSave\n"
            "lHasChildBeforeRelease = PEMSTATUS(oForm, 'cmdSave', 1)\n"
            "cChildCaptionBeforeRelease = oChild.Caption\n"
            "lReleased = oForm.Release()\n"
            "cDestroyLogAfter = cDestroyLog\n"
            "cChildParentCaptionAfter = cChildParentCaptionDuringDestroy\n"
            "nFormDestroyCountAfter = nFormDestroyCount\n"
            "nChildDestroyCountAfter = nChildDestroyCount\n"
            "lHasChildAfterRelease = PEMSTATUS(oForm, 'cmdSave', 1)\n"
            "xChildAfterRelease = GETPEM(oForm, 'cmdSave')\n"
            "lChildHasParentAfterRelease = PEMSTATUS(oChild, 'Parent', 1)\n"
            "xChildParentAfterRelease = GETPEM(oChild, 'Parent')\n"
            "lFormHasCaptionAfterRelease = PEMSTATUS(oForm, 'Caption', 1)\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 91)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "    PROCEDURE Destroy\n"
            "        cDestroyLog = cDestroyLog + 'form>'\n"
            "        nFormDestroyCount = nFormDestroyCount + 1\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
            "    Caption = 'Save'\n"
            "    PROCEDURE Destroy\n"
            "        cDestroyLog = cDestroyLog + 'child>'\n"
            "        cChildParentCaptionDuringDestroy = PARENT.Caption\n"
            "        nChildDestroyCount = nChildDestroyCount + 1\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Release contained script should complete: ") + state.message +
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

        check("lhaschildbeforerelease", "true");
        check("cchildcaptionbeforerelease", "Save");
        check("lreleased", "true");
        check("cdestroylogafter", "child>form>");
        check("cchildparentcaptionafter", "MainForm");
        check("nformdestroycountafter", "1");
        check("nchilddestroycountafter", "1");
        check("lhaschildafterrelease", "false");
        check("lchildhasparentafterrelease", "false");
        check("lformhascaptionafterrelease", "false");
        check("ldictset", "true");
        check("ndictcompare", "91");

        const auto child_after_release = state.globals.find("xchildafterrelease");
        expect(child_after_release != state.globals.end() &&
                   child_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native Release contained should invalidate GETPEM() for the released child member");

        const auto child_parent_after_release = state.globals.find("xchildparentafterrelease");
        expect(child_parent_after_release != state.globals.end() &&
                   child_parent_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native Release contained should invalidate GETPEM() for the released child's Parent");

        expect(state.ole_objects.size() == 1U,
               "native Release contained should leave only the COM object registered after recursive release");
        if (state.ole_objects.size() == 1U)
        {
            expect(state.ole_objects[0].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native contained Release lands");
        }

        const bool has_child_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "SaveButton.Destroy";
        });
        expect(has_child_destroy_event,
               "native Release contained should dispatch the child Destroy method");

        const bool has_form_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "DemoForm.Destroy";
        });
        expect(has_form_destroy_event,
               "native Release contained should dispatch the parent Destroy method");

        fs::remove_all(temp_root, ignored);
    }

}
