#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_release_detaches_contained_child_while_owner_and_sibling_remain_alive()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_release_direct_child";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_release_direct_child.prg";
        write_text(
            main_path,
            "cDestroyLog = ''\n"
            "cChildParentCaptionDuringDestroy = ''\n"
            "cGrandchildParentCaptionDuringDestroy = ''\n"
            "nFormDestroyCount = 0\n"
            "nChildDestroyCount = 0\n"
            "nGrandchildDestroyCount = 0\n"
            "oForm = CREATEOBJECT('DemoForm')\n"
            "oChild = oForm.cmdSave\n"
            "oSibling = oForm.cmdCancel\n"
            "oGrandchild = oChild.lblBadge\n"
            "cOwnerCaptionBeforeRelease = oForm.Caption\n"
            "cSiblingCaptionBeforeRelease = oSibling.Caption\n"
            "cGrandchildCaptionBeforeRelease = oGrandchild.Caption\n"
            "lReleased = oChild.Release()\n"
            "cDestroyLogAfter = cDestroyLog\n"
            "cChildParentCaptionAfter = cChildParentCaptionDuringDestroy\n"
            "cGrandchildParentCaptionAfter = cGrandchildParentCaptionDuringDestroy\n"
            "nFormDestroyCountAfter = nFormDestroyCount\n"
            "nChildDestroyCountAfter = nChildDestroyCount\n"
            "nGrandchildDestroyCountAfter = nGrandchildDestroyCount\n"
            "cOwnerCaptionAfterRelease = oForm.Caption\n"
            "lOwnerStillHasSave = PEMSTATUS(oForm, 'cmdSave', 1)\n"
            "xReleasedChild = GETPEM(oForm, 'cmdSave')\n"
            "lOwnerStillHasCancel = PEMSTATUS(oForm, 'cmdCancel', 1)\n"
            "cSiblingCaptionAfterRelease = oForm.cmdCancel.Caption\n"
            "lHeldChildHasParentAfterRelease = PEMSTATUS(oChild, 'Parent', 1)\n"
            "xHeldChildParentAfterRelease = GETPEM(oChild, 'Parent')\n"
            "lHeldChildHasCaptionAfterRelease = PEMSTATUS(oChild, 'Caption', 1)\n"
            "xHeldChildCaptionAfterRelease = GETPEM(oChild, 'Caption')\n"
            "lHeldGrandchildHasParentAfterRelease = PEMSTATUS(oGrandchild, 'Parent', 1)\n"
            "xHeldGrandchildParentAfterRelease = GETPEM(oGrandchild, 'Parent')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 92)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton')\n"
            "        THIS.AddObject('cmdCancel', 'CancelButton')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "    PROCEDURE Destroy\n"
            "        nFormDestroyCount = nFormDestroyCount + 1\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
            "    Caption = 'Save'\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('lblBadge', 'BadgeLabel')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "    PROCEDURE Destroy\n"
            "        cDestroyLog = cDestroyLog + 'child>'\n"
            "        cChildParentCaptionDuringDestroy = PARENT.Caption\n"
            "        nChildDestroyCount = nChildDestroyCount + 1\n"
            "        RETURN\n"
            "    ENDPROC\n"
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
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Release direct-child script should complete: ") + state.message +
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

        check("cownercaptionbeforerelease", "MainForm");
        check("csiblingcaptionbeforerelease", "Cancel");
        check("cgrandchildcaptionbeforerelease", "Badge");
        check("lreleased", "true");
        check("cdestroylogafter", "badge>child>");
        check("cchildparentcaptionafter", "MainForm");
        check("cgrandchildparentcaptionafter", "Save");
        check("nformdestroycountafter", "0");
        check("nchilddestroycountafter", "1");
        check("ngrandchilddestroycountafter", "1");
        check("cownercaptionafterrelease", "MainForm");
        check("lownerstillhassave", "false");
        check("lownerstillhascancel", "true");
        check("csiblingcaptionafterrelease", "Cancel");
        check("lheldchildhasparentafterrelease", "false");
        check("lheldchildhascaptionafterrelease", "false");
        check("lheldgrandchildhasparentafterrelease", "false");
        check("ldictset", "true");
        check("ndictcompare", "92");

        const auto released_child = state.globals.find("xreleasedchild");
        expect(released_child != state.globals.end() &&
                   released_child->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native Release direct child should invalidate GETPEM() on the owner's released child slot");

        const auto held_child_parent = state.globals.find("xheldchildparentafterrelease");
        expect(held_child_parent != state.globals.end() &&
                   held_child_parent->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native Release direct child should invalidate GETPEM() on the held child's Parent");

        const auto held_child_caption = state.globals.find("xheldchildcaptionafterrelease");
        expect(held_child_caption != state.globals.end() &&
                   held_child_caption->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native Release direct child should invalidate GETPEM() on the held released child");

        const auto held_grandchild_parent = state.globals.find("xheldgrandchildparentafterrelease");
        expect(held_grandchild_parent != state.globals.end() &&
                   held_grandchild_parent->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native Release direct child should invalidate GETPEM() on the released grandchild's Parent");

        expect(state.ole_objects.size() == 3U,
               "native Release direct child should leave owner, surviving sibling, and COM objects registered");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "DemoForm",
                   "native Release direct child should keep the owner object alive");
            expect(!state.ole_objects[0].properties.contains("cmdsave"),
                   "native Release direct child should detach the released child from the owner");
            expect(state.ole_objects[0].properties.contains("cmdcancel"),
                   "native Release direct child should preserve the surviving sibling on the owner");
            expect(state.ole_objects[1].prog_id == "CancelButton",
                   "native Release direct child should preserve the surviving sibling object");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native direct-child Release lands");
        }

        const bool has_child_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "SaveButton.Destroy";
        });
        expect(has_child_destroy_event,
               "native Release direct child should dispatch the child Destroy method");

        const bool has_grandchild_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "BadgeLabel.Destroy";
        });
        expect(has_grandchild_destroy_event,
               "native Release direct child should dispatch descendant Destroy methods");

        const bool has_form_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "DemoForm.Destroy";
        });
        expect(!has_form_destroy_event,
               "native Release direct child should not destroy the owner");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_release_detaches_external_base_contained_child_while_owner_and_sibling_remain_alive()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_release_external_base_child";
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

        const fs::path main_path = temp_root / "native_release_external_base_child.prg";
        write_text(
            main_path,
            "cDestroyLog = ''\n"
            "cChildParentCaptionDuringDestroy = ''\n"
            "nChildDestroyCount = 0\n"
            "oForm = CREATEOBJECT('DemoForm')\n"
            "oChild = oForm.cmdSave\n"
            "oSibling = oForm.cmdCancel\n"
            "cChildClassLibraryBeforeRelease = oChild.ClassLibrary\n"
            "cSiblingCaptionBeforeRelease = oSibling.Caption\n"
            "lReleased = oChild.Release()\n"
            "cDestroyLogAfter = cDestroyLog\n"
            "cChildParentCaptionAfter = cChildParentCaptionDuringDestroy\n"
            "nChildDestroyCountAfter = nChildDestroyCount\n"
            "cOwnerCaptionAfterRelease = oForm.Caption\n"
            "lOwnerStillHasSave = PEMSTATUS(oForm, 'cmdSave', 1)\n"
            "xReleasedChild = GETPEM(oForm, 'cmdSave')\n"
            "lOwnerStillHasCancel = PEMSTATUS(oForm, 'cmdCancel', 1)\n"
            "cSiblingCaptionAfterRelease = oForm.cmdCancel.Caption\n"
            "lHeldChildHasParentAfterRelease = PEMSTATUS(oChild, 'Parent', 1)\n"
            "xHeldChildParentAfterRelease = GETPEM(oChild, 'Parent')\n"
            "lHeldChildHasClassLibraryAfterRelease = PEMSTATUS(oChild, 'ClassLibrary', 1)\n"
            "xHeldChildClassLibraryAfterRelease = GETPEM(oChild, 'ClassLibrary')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 93)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton')\n"
            "        THIS.AddObject('cmdCancel', 'CancelButton')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS ParentButton OF buttons.prg\n"
            "    Caption = 'Save'\n"
            "    PROCEDURE Destroy\n"
            "        cDestroyLog = cDestroyLog + 'child>'\n"
            "        cChildParentCaptionDuringDestroy = PARENT.Caption\n"
            "        nChildDestroyCount = nChildDestroyCount + 1\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS CancelButton AS Custom\n"
            "    Caption = 'Cancel'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Release external-base child script should complete: ") + state.message +
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

        check("cchildclasslibrarybeforerelease", button_library_path.string());
        check("csiblingcaptionbeforerelease", "Cancel");
        check("lreleased", "true");
        check("cdestroylogafter", "child>");
        check("cchildparentcaptionafter", "MainForm");
        check("nchilddestroycountafter", "1");
        check("cownercaptionafterrelease", "MainForm");
        check("lownerstillhassave", "false");
        check("lownerstillhascancel", "true");
        check("csiblingcaptionafterrelease", "Cancel");
        check("lheldchildhasparentafterrelease", "false");
        check("lheldchildhasclasslibraryafterrelease", "false");
        check("ldictset", "true");
        check("ndictcompare", "93");

        const auto released_child = state.globals.find("xreleasedchild");
        expect(released_child != state.globals.end() &&
                   released_child->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native Release external-base child should invalidate GETPEM() on the owner's released child slot");

        const auto held_child_parent = state.globals.find("xheldchildparentafterrelease");
        expect(held_child_parent != state.globals.end() &&
                   held_child_parent->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native Release external-base child should invalidate GETPEM() on the held child's Parent");

        const auto held_child_class_library = state.globals.find("xheldchildclasslibraryafterrelease");
        expect(held_child_class_library != state.globals.end() &&
                   held_child_class_library->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native Release external-base child should invalidate GETPEM() on the held released child's ClassLibrary");

        expect(state.ole_objects.size() == 3U,
               "native Release external-base child should leave owner, surviving sibling, and COM objects registered");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "DemoForm",
                   "native Release external-base child should keep the owner object alive");
            expect(state.ole_objects[0].source == main_path.string(),
                   "native Release external-base child should preserve the owner source path");
            expect(!state.ole_objects[0].properties.contains("cmdsave"),
                   "native Release external-base child should detach the released child from the owner");
            expect(state.ole_objects[0].properties.contains("cmdcancel"),
                   "native Release external-base child should preserve the surviving sibling on the owner");
            expect(state.ole_objects[1].prog_id == "CancelButton",
                   "native Release external-base child should preserve the surviving sibling object");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native external-base child Release lands");
        }

        const bool has_child_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "SaveButton.Destroy";
        });
        expect(has_child_destroy_event,
               "native Release external-base child should dispatch the child Destroy method");

        fs::remove_all(temp_root, ignored);
    }

    void test_inherited_external_base_release_detaches_external_base_contained_child_while_owner_and_sibling_remain_alive()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_inherited_external_release_external_base_child";
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
            "    Caption = 'MainForm'\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton')\n"
            "        THIS.AddObject('cmdCancel', 'CancelButton')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS ParentButton OF buttons.prg\n"
            "    Caption = 'Save'\n"
            "    PROCEDURE Destroy\n"
            "        cDestroyLog = cDestroyLog + 'child>'\n"
            "        cChildParentCaptionDuringDestroy = PARENT.Caption\n"
            "        nChildDestroyCount = nChildDestroyCount + 1\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS CancelButton AS Custom\n"
            "    Caption = 'Cancel'\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "inherited_external_release_external_base_child.prg";
        write_text(
            main_path,
            "cDestroyLog = ''\n"
            "cChildParentCaptionDuringDestroy = ''\n"
            "nChildDestroyCount = 0\n"
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oChild = oCreate.cmdSave\n"
            "oSibling = oCreate.cmdCancel\n"
            "cChildClassLibraryBeforeRelease = oChild.ClassLibrary\n"
            "cSiblingCaptionBeforeRelease = oSibling.Caption\n"
            "lReleased = oChild.Release()\n"
            "cDestroyLogAfter = cDestroyLog\n"
            "cChildParentCaptionAfter = cChildParentCaptionDuringDestroy\n"
            "nChildDestroyCountAfter = nChildDestroyCount\n"
            "cOwnerCaptionAfterRelease = oCreate.Caption\n"
            "lOwnerStillHasSave = PEMSTATUS(oCreate, 'cmdSave', 1)\n"
            "xReleasedChild = GETPEM(oCreate, 'cmdSave')\n"
            "lOwnerStillHasCancel = PEMSTATUS(oCreate, 'cmdCancel', 1)\n"
            "cSiblingCaptionAfterRelease = oCreate.cmdCancel.Caption\n"
            "lHeldChildHasParentAfterRelease = PEMSTATUS(oChild, 'Parent', 1)\n"
            "xHeldChildParentAfterRelease = GETPEM(oChild, 'Parent')\n"
            "lHeldChildHasClassLibraryAfterRelease = PEMSTATUS(oChild, 'ClassLibrary', 1)\n"
            "xHeldChildClassLibraryAfterRelease = GETPEM(oChild, 'ClassLibrary')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 94)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("inherited external Release external-base child script should complete: ") + state.message +
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

        check("cchildclasslibrarybeforerelease", button_library_path.string());
        check("csiblingcaptionbeforerelease", "Cancel");
        check("lreleased", "true");
        check("cdestroylogafter", "child>");
        check("cchildparentcaptionafter", "MainForm");
        check("nchilddestroycountafter", "1");
        check("cownercaptionafterrelease", "MainForm");
        check("lownerstillhassave", "false");
        check("lownerstillhascancel", "true");
        check("csiblingcaptionafterrelease", "Cancel");
        check("lheldchildhasparentafterrelease", "false");
        check("lheldchildhasclasslibraryafterrelease", "false");
        check("ldictset", "true");
        check("ndictcompare", "94");

        const auto released_child = state.globals.find("xreleasedchild");
        expect(released_child != state.globals.end() &&
                   released_child->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited external Release external-base child should invalidate GETPEM() on the owner's released child slot");

        const auto held_child_parent = state.globals.find("xheldchildparentafterrelease");
        expect(held_child_parent != state.globals.end() &&
                   held_child_parent->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited external Release external-base child should invalidate GETPEM() on the held child's Parent");

        const auto held_child_class_library = state.globals.find("xheldchildclasslibraryafterrelease");
        expect(held_child_class_library != state.globals.end() &&
                   held_child_class_library->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited external Release external-base child should invalidate GETPEM() on the held released child's ClassLibrary");

        expect(state.ole_objects.size() == 3U,
               "inherited external Release external-base child should leave owner, surviving sibling, and COM objects registered");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "ChildForm",
                   "inherited external Release external-base child should keep the owner object alive");
            expect(state.ole_objects[0].source == main_path.string(),
                   "inherited external Release external-base child should preserve the owner source path");
            expect(!state.ole_objects[0].properties.contains("cmdsave"),
                   "inherited external Release external-base child should detach the released child from the owner");
            expect(state.ole_objects[0].properties.contains("cmdcancel"),
                   "inherited external Release external-base child should preserve the surviving sibling on the owner");
            expect(state.ole_objects[1].prog_id == "CancelButton",
                   "inherited external Release external-base child should preserve the surviving sibling object");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while inherited external child Release lands");
        }

        const bool has_child_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "SaveButton.Destroy";
        });
        expect(has_child_destroy_event,
               "inherited external Release external-base child should dispatch the child Destroy method");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_release_detaches_external_base_contained_child_subtree_while_owner_and_sibling_remain_alive()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_release_external_base_child_subtree";
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

        const fs::path main_path = temp_root / "native_release_external_base_child_subtree.prg";
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
            "cChildClassLibraryBeforeRelease = oChild.ClassLibrary\n"
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
            "lDictSet = SETPEM(oDict, 'comparemode', 95)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton')\n"
            "        THIS.AddObject('cmdCancel', 'CancelButton')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS ParentButton OF buttons.prg\n"
            "    Caption = 'Save'\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('lblBadge', 'BadgeLabel')\n"
            "        RETURN\n"
            "    ENDPROC\n"
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

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Release external-base child subtree script should complete: ") + state.message +
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

        check("cchildclasslibrarybeforerelease", button_library_path.string());
        check("cgrandchildcaptionbeforerelease", "Badge");
        check("csiblingcaptionbeforerelease", "Cancel");
        check("lreleased", "true");
        check("cdestroylogafter", "badge>child>");
        check("cchildparentcaptionafter", "MainForm");
        check("cgrandchildparentcaptionafter", "Save");
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
        check("ndictcompare", "95");

        const auto released_child = state.globals.find("xreleasedchild");
        expect(released_child != state.globals.end() &&
                   released_child->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native Release external-base child subtree should invalidate GETPEM() on the owner's released child slot");

        const auto held_child_parent = state.globals.find("xheldchildparentafterrelease");
        expect(held_child_parent != state.globals.end() &&
                   held_child_parent->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native Release external-base child subtree should invalidate GETPEM() on the held child's Parent");

        const auto held_child_class_library = state.globals.find("xheldchildclasslibraryafterrelease");
        expect(held_child_class_library != state.globals.end() &&
                   held_child_class_library->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native Release external-base child subtree should invalidate GETPEM() on the held child's ClassLibrary");

        const auto held_grandchild_parent = state.globals.find("xheldgrandchildparentafterrelease");
        expect(held_grandchild_parent != state.globals.end() &&
                   held_grandchild_parent->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native Release external-base child subtree should invalidate GETPEM() on the released grandchild's Parent");

        expect(state.ole_objects.size() == 3U,
               "native Release external-base child subtree should leave owner, surviving sibling, and COM objects registered");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "DemoForm",
                   "native Release external-base child subtree should keep the owner object alive");
            expect(!state.ole_objects[0].properties.contains("cmdsave"),
                   "native Release external-base child subtree should detach the released child from the owner");
            expect(state.ole_objects[0].properties.contains("cmdcancel"),
                   "native Release external-base child subtree should preserve the surviving sibling on the owner");
            expect(state.ole_objects[1].prog_id == "CancelButton",
                   "native Release external-base child subtree should preserve the surviving sibling object");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native external-base subtree Release lands");
        }

        const bool has_child_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "SaveButton.Destroy";
        });
        expect(has_child_destroy_event,
               "native Release external-base child subtree should dispatch the child Destroy method");

        const bool has_grandchild_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "BadgeLabel.Destroy";
        });
        expect(has_grandchild_destroy_event,
               "native Release external-base child subtree should dispatch descendant Destroy methods");

        fs::remove_all(temp_root, ignored);
    }

}
