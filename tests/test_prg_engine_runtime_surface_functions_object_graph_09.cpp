#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_inherited_external_base_release_detaches_object_block_child_subtree_while_owner_and_sibling_remain_alive()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_inherited_external_release_object_block_child_subtree";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path library_path = temp_root / "widgetlib.prg";
        write_text(
            library_path,
            "DEFINE CLASS ParentForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    OBJECT cmdSave AS SaveButton\n"
            "        Caption = 'Commit'\n"
            "    ENDOBJECT\n"
            "    OBJECT cmdCancel AS CancelButton\n"
            "        Caption = 'Cancel'\n"
            "    ENDOBJECT\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
            "    Caption = 'Save'\n"
            "    OBJECT lblBadge AS BadgeLabel\n"
            "        Caption = 'Badge'\n"
            "    ENDOBJECT\n"
            "    PROCEDURE Destroy\n"
            "        cDestroyLog = cDestroyLog + 'child>'\n"
            "        cChildParentCaptionDuringDestroy = PARENT.Caption\n"
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
            "ENDDEFINE\n"
            "DEFINE CLASS CancelButton AS Custom\n"
            "    Caption = 'Cancel'\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "inherited_external_release_object_block_child_subtree.prg";
        write_text(
            main_path,
            "cDestroyLog = ''\n"
            "cChildParentCaptionDuringDestroy = ''\n"
            "cGrandchildParentCaptionDuringDestroy = ''\n"
            "nChildDestroyCount = 0\n"
            "nGrandchildDestroyCount = 0\n"
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oChild = oCreate.cmdSave\n"
            "oSibling = oCreate.cmdCancel\n"
            "oGrandchild = oChild.lblBadge\n"
            "cChildClassLibraryBeforeRelease = oChild.ClassLibrary\n"
            "cGrandchildCaptionBeforeRelease = oGrandchild.Caption\n"
            "cSiblingCaptionBeforeRelease = oSibling.Caption\n"
            "lReleased = oChild.Release()\n"
            "cDestroyLogAfter = cDestroyLog\n"
            "cChildParentCaptionAfter = cChildParentCaptionDuringDestroy\n"
            "cGrandchildParentCaptionAfter = cGrandchildParentCaptionDuringDestroy\n"
            "nChildDestroyCountAfter = nChildDestroyCount\n"
            "nGrandchildDestroyCountAfter = nGrandchildDestroyCount\n"
            "cOwnerCaptionAfterRelease = oCreate.Caption\n"
            "lOwnerStillHasSave = PEMSTATUS(oCreate, 'cmdSave', 1)\n"
            "xReleasedChild = GETPEM(oCreate, 'cmdSave')\n"
            "lOwnerStillHasCancel = PEMSTATUS(oCreate, 'cmdCancel', 1)\n"
            "cSiblingCaptionAfterRelease = oCreate.cmdCancel.Caption\n"
            "lHeldChildHasParentAfterRelease = PEMSTATUS(oChild, 'Parent', 1)\n"
            "xHeldChildParentAfterRelease = GETPEM(oChild, 'Parent')\n"
            "lHeldChildHasClassLibraryAfterRelease = PEMSTATUS(oChild, 'ClassLibrary', 1)\n"
            "xHeldChildClassLibraryAfterRelease = GETPEM(oChild, 'ClassLibrary')\n"
            "lHeldGrandchildHasParentAfterRelease = PEMSTATUS(oGrandchild, 'Parent', 1)\n"
            "xHeldGrandchildParentAfterRelease = GETPEM(oGrandchild, 'Parent')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 102)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("inherited external Release object-block child subtree script should complete: ") + state.message +
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

        check("cchildclasslibrarybeforerelease", "");
        check("cgrandchildcaptionbeforerelease", "Badge");
        check("csiblingcaptionbeforerelease", "Cancel");
        check("lreleased", "true");
        check("cdestroylogafter", "badge>child>");
        check("cchildparentcaptionafter", "MainForm");
        check("cgrandchildparentcaptionafter", "Commit");
        check("nchilddestroycountafter", "1");
        check("ngrandchilddestroycountafter", "1");
        check("cownercaptionafterrelease", "MainForm");
        check("lownerstillhassave", "false");
        check("lownerstillhascancel", "true");
        check("csiblingcaptionafterrelease", "Cancel");
        check("lheldchildhasparentafterrelease", "false");
        check("lheldchildhasclasslibraryafterrelease", "false");
        check("lheldgrandchildhasparentafterrelease", "false");
        check("ldictset", "true");
        check("ndictcompare", "102");

        const auto released_child = state.globals.find("xreleasedchild");
        expect(released_child != state.globals.end() &&
                   released_child->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited external Release object-block child subtree should invalidate GETPEM() on the owner's released child slot");

        const auto held_child_parent = state.globals.find("xheldchildparentafterrelease");
        expect(held_child_parent != state.globals.end() &&
                   held_child_parent->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited external Release object-block child subtree should invalidate GETPEM() on the held child's Parent");

        const auto held_child_class_library = state.globals.find("xheldchildclasslibraryafterrelease");
        expect(held_child_class_library != state.globals.end() &&
                   held_child_class_library->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited external Release object-block child subtree should invalidate GETPEM() on the held child's ClassLibrary");

        const auto held_grandchild_parent = state.globals.find("xheldgrandchildparentafterrelease");
        expect(held_grandchild_parent != state.globals.end() &&
                   held_grandchild_parent->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited external Release object-block child subtree should invalidate GETPEM() on the released grandchild's Parent");

        expect(state.ole_objects.size() == 3U,
               "inherited external Release object-block child subtree should leave owner, surviving sibling, and COM objects registered");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "ChildForm",
                   "inherited external Release object-block child subtree should keep the owner object alive");
            expect(!state.ole_objects[0].properties.contains("cmdsave"),
                   "inherited external Release object-block child subtree should detach the released child from the owner");
            expect(state.ole_objects[0].properties.contains("cmdcancel"),
                   "inherited external Release object-block child subtree should preserve the surviving sibling on the owner");
            expect(state.ole_objects[1].prog_id == "CancelButton",
                   "inherited external Release object-block child subtree should preserve the surviving sibling object");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while inherited external object-block subtree Release lands");
        }

        const bool has_child_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "SaveButton.Destroy";
        });
        expect(has_child_destroy_event,
               "inherited external Release object-block child subtree should dispatch the child Destroy method");

        const bool has_grandchild_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "BadgeLabel.Destroy";
        });
        expect(has_grandchild_destroy_event,
               "inherited external Release object-block child subtree should dispatch descendant Destroy methods");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_release_detaches_object_block_external_child_subtree_while_owner_and_sibling_remain_alive()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_release_object_block_external_child_subtree";
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
            "        cChildParentCaptionDuringDestroy = PARENT.Caption\n"
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

        const fs::path main_path = temp_root / "native_release_object_block_external_child_subtree.prg";
        write_text(
            main_path,
            "cDestroyLog = ''\n"
            "cChildParentCaptionDuringDestroy = ''\n"
            "cGrandchildParentCaptionDuringDestroy = ''\n"
            "nChildDestroyCount = 0\n"
            "nGrandchildDestroyCount = 0\n"
            "oForm = CREATEOBJECT('DemoForm')\n"
            "oChild = oForm.cmdSave\n"
            "oSibling = oForm.cmdCancel\n"
            "oGrandchild = oChild.lblBadge\n"
            "cChildClassBeforeRelease = oChild.Class\n"
            "cChildBaseClassBeforeRelease = oChild.BaseClass\n"
            "cChildParentClassBeforeRelease = oChild.ParentClass\n"
            "cChildClassLibraryBeforeRelease = oChild.ClassLibrary\n"
            "nChildClassCountBeforeRelease = ACLASS(aChildClass, oChild)\n"
            "cChildClass1BeforeRelease = aChildClass[1]\n"
            "cChildClass2BeforeRelease = aChildClass[2]\n"
            "cChildClass3BeforeRelease = aChildClass[3]\n"
            "cChildClass4BeforeRelease = aChildClass[4]\n"
            "cChildClass5BeforeRelease = aChildClass[5]\n"
            "cGrandchildCaptionBeforeRelease = oGrandchild.Caption\n"
            "cSiblingCaptionBeforeRelease = oSibling.Caption\n"
            "lReleased = oChild.Release()\n"
            "cDestroyLogAfter = cDestroyLog\n"
            "cChildParentCaptionAfter = cChildParentCaptionDuringDestroy\n"
            "cGrandchildParentCaptionAfter = cGrandchildParentCaptionDuringDestroy\n"
            "nChildDestroyCountAfter = nChildDestroyCount\n"
            "nGrandchildDestroyCountAfter = nGrandchildDestroyCount\n"
            "cOwnerCaptionAfterRelease = oForm.Caption\n"
            "lOwnerStillHasSave = PEMSTATUS(oForm, 'cmdSave', 1)\n"
            "xReleasedChild = GETPEM(oForm, 'cmdSave')\n"
            "lOwnerStillHasCancel = PEMSTATUS(oForm, 'cmdCancel', 1)\n"
            "cSiblingCaptionAfterRelease = oForm.cmdCancel.Caption\n"
            "lHeldChildHasParentAfterRelease = PEMSTATUS(oChild, 'Parent', 1)\n"
            "xHeldChildParentAfterRelease = GETPEM(oChild, 'Parent')\n"
            "lHeldChildHasClassLibraryAfterRelease = PEMSTATUS(oChild, 'ClassLibrary', 1)\n"
            "xHeldChildClassLibraryAfterRelease = GETPEM(oChild, 'ClassLibrary')\n"
            "lHeldGrandchildHasParentAfterRelease = PEMSTATUS(oGrandchild, 'Parent', 1)\n"
            "xHeldGrandchildParentAfterRelease = GETPEM(oGrandchild, 'Parent')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 103)\n"
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
               std::string("native Release object-block external child subtree script should complete: ") + state.message +
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
        check("cgrandchildcaptionbeforerelease", "Badge");
        check("csiblingcaptionbeforerelease", "Cancel");
        check("lreleased", "true");
        check("cdestroylogafter", "badge>child>");
        check("cchildparentcaptionafter", "MainForm");
        check("cgrandchildparentcaptionafter", "Commit");
        check("nchilddestroycountafter", "1");
        check("ngrandchilddestroycountafter", "1");
        check("cownercaptionafterrelease", "MainForm");
        check("lownerstillhassave", "false");
        check("lownerstillhascancel", "true");
        check("csiblingcaptionafterrelease", "Cancel");
        check("lheldchildhasparentafterrelease", "false");
        check("lheldchildhasclasslibraryafterrelease", "false");
        check("lheldgrandchildhasparentafterrelease", "false");
        check("ldictset", "true");
        check("ndictcompare", "103");

        const auto released_child = state.globals.find("xreleasedchild");
        expect(released_child != state.globals.end() &&
                   released_child->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native Release object-block external child subtree should invalidate GETPEM() on the owner's released child slot");

        const auto held_child_parent = state.globals.find("xheldchildparentafterrelease");
        expect(held_child_parent != state.globals.end() &&
                   held_child_parent->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native Release object-block external child subtree should invalidate GETPEM() on the held child's Parent");

        const auto held_child_class_library = state.globals.find("xheldchildclasslibraryafterrelease");
        expect(held_child_class_library != state.globals.end() &&
                   held_child_class_library->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native Release object-block external child subtree should invalidate GETPEM() on the held child's ClassLibrary");

        const auto held_grandchild_parent = state.globals.find("xheldgrandchildparentafterrelease");
        expect(held_grandchild_parent != state.globals.end() &&
                   held_grandchild_parent->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native Release object-block external child subtree should invalidate GETPEM() on the released grandchild's Parent");

        expect(state.ole_objects.size() == 3U,
               "native Release object-block external child subtree should leave owner, surviving sibling, and COM objects registered");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "DemoForm",
                   "native Release object-block external child subtree should keep the owner object alive");
            expect(state.ole_objects[0].source == main_path.string(),
                   "native Release object-block external child subtree should preserve the owner source path");
            expect(!state.ole_objects[0].properties.contains("cmdsave"),
                   "native Release object-block external child subtree should detach the released child from the owner");
            expect(state.ole_objects[0].properties.contains("cmdcancel"),
                   "native Release object-block external child subtree should preserve the surviving sibling on the owner");
            expect(state.ole_objects[1].prog_id == "CancelButton",
                   "native Release object-block external child subtree should preserve the surviving sibling object");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native object-block external-child subtree Release lands");
        }

        const bool has_child_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "SaveButton.Destroy";
        });
        expect(has_child_destroy_event,
               "native Release object-block external child subtree should dispatch the child Destroy method");

        const bool has_grandchild_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "BadgeLabel.Destroy";
        });
        expect(has_grandchild_destroy_event,
               "native Release object-block external child subtree should dispatch descendant Destroy methods");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_release_detaches_object_block_external_base_child_subtree_while_owner_and_sibling_remain_alive()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_release_object_block_external_base_child_subtree";
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

        const fs::path main_path = temp_root / "native_release_object_block_external_base_child_subtree.prg";
        write_text(
            main_path,
            "cDestroyLog = ''\n"
            "cChildParentCaptionDuringDestroy = ''\n"
            "cGrandchildParentCaptionDuringDestroy = ''\n"
            "nChildDestroyCount = 0\n"
            "nGrandchildDestroyCount = 0\n"
            "oForm = CREATEOBJECT('DemoForm')\n"
            "oChild = oForm.cmdSave\n"
            "oSibling = oForm.cmdCancel\n"
            "oGrandchild = oChild.lblBadge\n"
            "cChildClassBeforeRelease = oChild.Class\n"
            "cChildBaseClassBeforeRelease = oChild.BaseClass\n"
            "cChildParentClassBeforeRelease = oChild.ParentClass\n"
            "cChildClassLibraryBeforeRelease = oChild.ClassLibrary\n"
            "nChildClassCountBeforeRelease = ACLASS(aChildClass, oChild)\n"
            "cChildClass1BeforeRelease = aChildClass[1]\n"
            "cChildClass2BeforeRelease = aChildClass[2]\n"
            "cChildClass3BeforeRelease = aChildClass[3]\n"
            "cChildClass4BeforeRelease = aChildClass[4]\n"
            "cChildClass5BeforeRelease = aChildClass[5]\n"
            "cChildCaptionBeforeRelease = oChild.Caption\n"
            "cGrandchildCaptionBeforeRelease = oGrandchild.Caption\n"
            "cSiblingCaptionBeforeRelease = oSibling.Caption\n"
            "lReleased = oChild.Release()\n"
            "cDestroyLogAfter = cDestroyLog\n"
            "cChildParentCaptionAfter = cChildParentCaptionDuringDestroy\n"
            "cGrandchildParentCaptionAfter = cGrandchildParentCaptionDuringDestroy\n"
            "nChildDestroyCountAfter = nChildDestroyCount\n"
            "nGrandchildDestroyCountAfter = nGrandchildDestroyCount\n"
            "cOwnerCaptionAfterRelease = oForm.Caption\n"
            "lOwnerStillHasSave = PEMSTATUS(oForm, 'cmdSave', 1)\n"
            "xReleasedChild = GETPEM(oForm, 'cmdSave')\n"
            "lOwnerStillHasCancel = PEMSTATUS(oForm, 'cmdCancel', 1)\n"
            "cSiblingCaptionAfterRelease = oForm.cmdCancel.Caption\n"
            "lHeldChildHasParentAfterRelease = PEMSTATUS(oChild, 'Parent', 1)\n"
            "xHeldChildParentAfterRelease = GETPEM(oChild, 'Parent')\n"
            "lHeldChildHasClassLibraryAfterRelease = PEMSTATUS(oChild, 'ClassLibrary', 1)\n"
            "xHeldChildClassLibraryAfterRelease = GETPEM(oChild, 'ClassLibrary')\n"
            "lHeldGrandchildHasParentAfterRelease = PEMSTATUS(oGrandchild, 'Parent', 1)\n"
            "xHeldGrandchildParentAfterRelease = GETPEM(oGrandchild, 'Parent')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 119)\n"
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
            "DEFINE CLASS CancelButton AS Custom\n"
            "    Caption = 'Cancel'\n"
            "ENDDEFINE\n"
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
            "        cChildParentCaptionDuringDestroy = PARENT.Caption\n"
            "        nChildDestroyCount = nChildDestroyCount + 1\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Release object-block external-base child subtree script should complete: ") + state.message +
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
        check("cchildcaptionbeforerelease", "Commit");
        check("cgrandchildcaptionbeforerelease", "Badge");
        check("csiblingcaptionbeforerelease", "Cancel");
        check("lreleased", "true");
        check("cdestroylogafter", "badge>child>");
        check("cchildparentcaptionafter", "MainForm");
        check("cgrandchildparentcaptionafter", "Commit");
        check("nchilddestroycountafter", "1");
        check("ngrandchilddestroycountafter", "1");
        check("cownercaptionafterrelease", "MainForm");
        check("lownerstillhassave", "false");
        check("lownerstillhascancel", "true");
        check("csiblingcaptionafterrelease", "Cancel");
        check("lheldchildhasparentafterrelease", "false");
        check("lheldchildhasclasslibraryafterrelease", "false");
        check("lheldgrandchildhasparentafterrelease", "false");
        check("ldictset", "true");
        check("ndictcompare", "119");

        const auto released_child = state.globals.find("xreleasedchild");
        expect(released_child != state.globals.end() &&
                   released_child->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native Release object-block external-base child subtree should invalidate GETPEM() on the owner's released child slot");

        const auto held_child_parent = state.globals.find("xheldchildparentafterrelease");
        expect(held_child_parent != state.globals.end() &&
                   held_child_parent->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native Release object-block external-base child subtree should invalidate GETPEM() on the held child's Parent");

        const auto held_child_class_library = state.globals.find("xheldchildclasslibraryafterrelease");
        expect(held_child_class_library != state.globals.end() &&
                   held_child_class_library->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native Release object-block external-base child subtree should invalidate GETPEM() on the held child's ClassLibrary");

        const auto held_grandchild_parent = state.globals.find("xheldgrandchildparentafterrelease");
        expect(held_grandchild_parent != state.globals.end() &&
                   held_grandchild_parent->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native Release object-block external-base child subtree should invalidate GETPEM() on the released grandchild's Parent");

        expect(state.ole_objects.size() == 3U,
               "native Release object-block external-base child subtree should leave owner, surviving sibling, and COM objects registered");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "DemoForm",
                   "native Release object-block external-base child subtree should keep the owner object alive");
            expect(state.ole_objects[0].source == main_path.string(),
                   "native Release object-block external-base child subtree should preserve the owner source path");
            expect(!state.ole_objects[0].properties.contains("cmdsave"),
                   "native Release object-block external-base child subtree should detach the released child from the owner");
            expect(state.ole_objects[0].properties.contains("cmdcancel"),
                   "native Release object-block external-base child subtree should preserve the surviving sibling on the owner");
            expect(state.ole_objects[1].prog_id == "CancelButton",
                   "native Release object-block external-base child subtree should preserve the surviving sibling object");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native object-block external-base subtree Release lands");
        }

        const bool has_child_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "SaveButton.Destroy";
        });
        expect(has_child_destroy_event,
               "native Release object-block external-base child subtree should dispatch the child Destroy method");

        const bool has_grandchild_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "BadgeLabel.Destroy";
        });
        expect(has_grandchild_destroy_event,
               "native Release object-block external-base child subtree should dispatch descendant Destroy methods");

        fs::remove_all(temp_root, ignored);
    }

    void test_inherited_external_base_release_detaches_object_block_external_child_subtree_while_owner_and_sibling_remain_alive()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_inherited_external_release_object_block_external_child_subtree";
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
            "        cChildParentCaptionDuringDestroy = PARENT.Caption\n"
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

        const fs::path widget_library_path = temp_root / "widgetlib.prg";
        write_text(
            widget_library_path,
            "DEFINE CLASS ParentForm AS Custom\n"
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

        const fs::path main_path = temp_root / "inherited_external_release_object_block_external_child_subtree.prg";
        write_text(
            main_path,
            "cDestroyLog = ''\n"
            "cChildParentCaptionDuringDestroy = ''\n"
            "cGrandchildParentCaptionDuringDestroy = ''\n"
            "nChildDestroyCount = 0\n"
            "nGrandchildDestroyCount = 0\n"
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oChild = oCreate.cmdSave\n"
            "oSibling = oCreate.cmdCancel\n"
            "oGrandchild = oChild.lblBadge\n"
            "cChildClassBeforeRelease = oChild.Class\n"
            "cChildBaseClassBeforeRelease = oChild.BaseClass\n"
            "cChildParentClassBeforeRelease = oChild.ParentClass\n"
            "cChildClassLibraryBeforeRelease = oChild.ClassLibrary\n"
            "nChildClassCountBeforeRelease = ACLASS(aChildClass, oChild)\n"
            "cChildClass1BeforeRelease = aChildClass[1]\n"
            "cChildClass2BeforeRelease = aChildClass[2]\n"
            "cChildClass3BeforeRelease = aChildClass[3]\n"
            "cChildClass4BeforeRelease = aChildClass[4]\n"
            "cChildClass5BeforeRelease = aChildClass[5]\n"
            "cGrandchildCaptionBeforeRelease = oGrandchild.Caption\n"
            "cSiblingCaptionBeforeRelease = oSibling.Caption\n"
            "lReleased = oChild.Release()\n"
            "cDestroyLogAfter = cDestroyLog\n"
            "cChildParentCaptionAfter = cChildParentCaptionDuringDestroy\n"
            "cGrandchildParentCaptionAfter = cGrandchildParentCaptionDuringDestroy\n"
            "nChildDestroyCountAfter = nChildDestroyCount\n"
            "nGrandchildDestroyCountAfter = nGrandchildDestroyCount\n"
            "cOwnerCaptionAfterRelease = oCreate.Caption\n"
            "lOwnerStillHasSave = PEMSTATUS(oCreate, 'cmdSave', 1)\n"
            "xReleasedChild = GETPEM(oCreate, 'cmdSave')\n"
            "lOwnerStillHasCancel = PEMSTATUS(oCreate, 'cmdCancel', 1)\n"
            "cSiblingCaptionAfterRelease = oCreate.cmdCancel.Caption\n"
            "lHeldChildHasParentAfterRelease = PEMSTATUS(oChild, 'Parent', 1)\n"
            "xHeldChildParentAfterRelease = GETPEM(oChild, 'Parent')\n"
            "lHeldChildHasClassLibraryAfterRelease = PEMSTATUS(oChild, 'ClassLibrary', 1)\n"
            "xHeldChildClassLibraryAfterRelease = GETPEM(oChild, 'ClassLibrary')\n"
            "lHeldGrandchildHasParentAfterRelease = PEMSTATUS(oGrandchild, 'Parent', 1)\n"
            "xHeldGrandchildParentAfterRelease = GETPEM(oGrandchild, 'Parent')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 104)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("inherited external Release object-block external child subtree script should complete: ") + state.message +
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
        check("cgrandchildcaptionbeforerelease", "Badge");
        check("csiblingcaptionbeforerelease", "Cancel");
        check("lreleased", "true");
        check("cdestroylogafter", "badge>child>");
        check("cchildparentcaptionafter", "MainForm");
        check("cgrandchildparentcaptionafter", "Commit");
        check("nchilddestroycountafter", "1");
        check("ngrandchilddestroycountafter", "1");
        check("cownercaptionafterrelease", "MainForm");
        check("lownerstillhassave", "false");
        check("lownerstillhascancel", "true");
        check("csiblingcaptionafterrelease", "Cancel");
        check("lheldchildhasparentafterrelease", "false");
        check("lheldchildhasclasslibraryafterrelease", "false");
        check("lheldgrandchildhasparentafterrelease", "false");
        check("ldictset", "true");
        check("ndictcompare", "104");

        const auto released_child = state.globals.find("xreleasedchild");
        expect(released_child != state.globals.end() &&
                   released_child->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited external Release object-block external child subtree should invalidate GETPEM() on the owner's released child slot");

        const auto held_child_parent = state.globals.find("xheldchildparentafterrelease");
        expect(held_child_parent != state.globals.end() &&
                   held_child_parent->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited external Release object-block external child subtree should invalidate GETPEM() on the held child's Parent");

        const auto held_child_class_library = state.globals.find("xheldchildclasslibraryafterrelease");
        expect(held_child_class_library != state.globals.end() &&
                   held_child_class_library->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited external Release object-block external child subtree should invalidate GETPEM() on the held child's ClassLibrary");

        const auto held_grandchild_parent = state.globals.find("xheldgrandchildparentafterrelease");
        expect(held_grandchild_parent != state.globals.end() &&
                   held_grandchild_parent->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited external Release object-block external child subtree should invalidate GETPEM() on the released grandchild's Parent");

        expect(state.ole_objects.size() == 3U,
               "inherited external Release object-block external child subtree should leave owner, surviving sibling, and COM objects registered");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "ChildForm",
                   "inherited external Release object-block external child subtree should keep the owner object alive");
            expect(state.ole_objects[0].source == main_path.string(),
                   "inherited external Release object-block external child subtree should preserve the derived owner source path");
            expect(!state.ole_objects[0].properties.contains("cmdsave"),
                   "inherited external Release object-block external child subtree should detach the released child from the owner");
            expect(state.ole_objects[0].properties.contains("cmdcancel"),
                   "inherited external Release object-block external child subtree should preserve the surviving sibling on the owner");
            expect(state.ole_objects[1].prog_id == "CancelButton",
                   "inherited external Release object-block external child subtree should preserve the surviving sibling object");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while inherited external object-block external-child subtree Release lands");
        }

        const bool has_child_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "SaveButton.Destroy";
        });
        expect(has_child_destroy_event,
               "inherited external Release object-block external child subtree should dispatch the child Destroy method");

        const bool has_grandchild_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "BadgeLabel.Destroy";
        });
        expect(has_grandchild_destroy_event,
               "inherited external Release object-block external child subtree should dispatch descendant Destroy methods");

        fs::remove_all(temp_root, ignored);
    }

}
