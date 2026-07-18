#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_inherited_declarative_release_detaches_external_base_child_subtree_while_owner_and_sibling_remain_alive()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_inherited_declarative_release_external_base_child_subtree";
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
            "    ADD OBJECT lblBadge AS BadgeLabel\n"
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
            "    ADD OBJECT cmdSave AS SaveButton OF buttons.prg\n"
            "    ADD OBJECT cmdCancel AS CancelButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS CancelButton AS Custom\n"
            "    Caption = 'Cancel'\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "inherited_declarative_release_external_base_child_subtree.prg";
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
            "lDictSet = SETPEM(oDict, 'comparemode', 98)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("inherited declarative Release external-base child subtree script should complete: ") + state.message +
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
        check("ndictcompare", "98");

        const auto released_child = state.globals.find("xreleasedchild");
        expect(released_child != state.globals.end() &&
                   released_child->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited declarative Release external-base child subtree should invalidate GETPEM() on the owner's released child slot");

        const auto held_child_parent = state.globals.find("xheldchildparentafterrelease");
        expect(held_child_parent != state.globals.end() &&
                   held_child_parent->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited declarative Release external-base child subtree should invalidate GETPEM() on the held child's Parent");

        const auto held_child_class_library = state.globals.find("xheldchildclasslibraryafterrelease");
        expect(held_child_class_library != state.globals.end() &&
                   held_child_class_library->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited declarative Release external-base child subtree should invalidate GETPEM() on the held child's ClassLibrary");

        const auto held_grandchild_parent = state.globals.find("xheldgrandchildparentafterrelease");
        expect(held_grandchild_parent != state.globals.end() &&
                   held_grandchild_parent->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited declarative Release external-base child subtree should invalidate GETPEM() on the released grandchild's Parent");

        expect(state.ole_objects.size() == 3U,
               "inherited declarative Release external-base child subtree should leave owner, surviving sibling, and COM objects registered");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "ChildForm",
                   "inherited declarative Release external-base child subtree should keep the owner object alive");
            expect(!state.ole_objects[0].properties.contains("cmdsave"),
                   "inherited declarative Release external-base child subtree should detach the released child from the owner");
            expect(state.ole_objects[0].properties.contains("cmdcancel"),
                   "inherited declarative Release external-base child subtree should preserve the surviving sibling on the owner");
            expect(state.ole_objects[1].prog_id == "CancelButton",
                   "inherited declarative Release external-base child subtree should preserve the surviving sibling object");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while inherited declarative subtree Release lands");
        }

        const bool has_child_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "SaveButton.Destroy";
        });
        expect(has_child_destroy_event,
               "inherited declarative Release external-base child subtree should dispatch the child Destroy method");

        const bool has_grandchild_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "BadgeLabel.Destroy";
        });
        expect(has_grandchild_destroy_event,
               "inherited declarative Release external-base child subtree should dispatch descendant Destroy methods");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_release_detaches_declarative_same_prg_child_subtree_while_owner_and_sibling_remain_alive()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_release_declarative_same_prg_child_subtree";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_release_declarative_same_prg_child_subtree.prg";
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
            "lDictSet = SETPEM(oDict, 'comparemode', 99)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "    ADD OBJECT cmdCancel AS CancelButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
            "    Caption = 'Save'\n"
            "    ADD OBJECT lblBadge AS BadgeLabel\n"
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
               std::string("native Release declarative same-PRG child subtree script should complete: ") + state.message +
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
        check("ndictcompare", "99");

        const auto released_child = state.globals.find("xreleasedchild");
        expect(released_child != state.globals.end() &&
                   released_child->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native Release declarative same-PRG child subtree should invalidate GETPEM() on the owner's released child slot");

        const auto held_child_parent = state.globals.find("xheldchildparentafterrelease");
        expect(held_child_parent != state.globals.end() &&
                   held_child_parent->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native Release declarative same-PRG child subtree should invalidate GETPEM() on the held child's Parent");

        const auto held_child_class_library = state.globals.find("xheldchildclasslibraryafterrelease");
        expect(held_child_class_library != state.globals.end() &&
                   held_child_class_library->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native Release declarative same-PRG child subtree should invalidate GETPEM() on the held child's ClassLibrary");

        const auto held_grandchild_parent = state.globals.find("xheldgrandchildparentafterrelease");
        expect(held_grandchild_parent != state.globals.end() &&
                   held_grandchild_parent->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native Release declarative same-PRG child subtree should invalidate GETPEM() on the released grandchild's Parent");

        expect(state.ole_objects.size() == 3U,
               "native Release declarative same-PRG child subtree should leave owner, surviving sibling, and COM objects registered");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "DemoForm",
                   "native Release declarative same-PRG child subtree should keep the owner object alive");
            expect(!state.ole_objects[0].properties.contains("cmdsave"),
                   "native Release declarative same-PRG child subtree should detach the released child from the owner");
            expect(state.ole_objects[0].properties.contains("cmdcancel"),
                   "native Release declarative same-PRG child subtree should preserve the surviving sibling on the owner");
            expect(state.ole_objects[1].prog_id == "CancelButton",
                   "native Release declarative same-PRG child subtree should preserve the surviving sibling object");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native declarative same-PRG subtree Release lands");
        }

        const bool has_child_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "SaveButton.Destroy";
        });
        expect(has_child_destroy_event,
               "native Release declarative same-PRG child subtree should dispatch the child Destroy method");

        const bool has_grandchild_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "BadgeLabel.Destroy";
        });
        expect(has_grandchild_destroy_event,
               "native Release declarative same-PRG child subtree should dispatch descendant Destroy methods");

        fs::remove_all(temp_root, ignored);
    }

    void test_inherited_same_prg_release_detaches_declarative_child_subtree_while_owner_and_sibling_remain_alive()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_inherited_same_prg_release_declarative_child_subtree";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "inherited_same_prg_release_declarative_child_subtree.prg";
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
            "lDictSet = SETPEM(oDict, 'comparemode', 100)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ParentForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "    ADD OBJECT cmdCancel AS CancelButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ChildForm AS ParentForm\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
            "    Caption = 'Save'\n"
            "    ADD OBJECT lblBadge AS BadgeLabel\n"
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
               std::string("inherited same-PRG Release declarative child subtree script should complete: ") + state.message +
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
        check("ndictcompare", "100");

        const auto released_child = state.globals.find("xreleasedchild");
        expect(released_child != state.globals.end() &&
                   released_child->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited same-PRG Release declarative child subtree should invalidate GETPEM() on the owner's released child slot");

        const auto held_child_parent = state.globals.find("xheldchildparentafterrelease");
        expect(held_child_parent != state.globals.end() &&
                   held_child_parent->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited same-PRG Release declarative child subtree should invalidate GETPEM() on the held child's Parent");

        const auto held_child_class_library = state.globals.find("xheldchildclasslibraryafterrelease");
        expect(held_child_class_library != state.globals.end() &&
                   held_child_class_library->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited same-PRG Release declarative child subtree should invalidate GETPEM() on the held child's ClassLibrary");

        const auto held_grandchild_parent = state.globals.find("xheldgrandchildparentafterrelease");
        expect(held_grandchild_parent != state.globals.end() &&
                   held_grandchild_parent->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited same-PRG Release declarative child subtree should invalidate GETPEM() on the released grandchild's Parent");

        expect(state.ole_objects.size() == 3U,
               "inherited same-PRG Release declarative child subtree should leave owner, surviving sibling, and COM objects registered");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "ChildForm",
                   "inherited same-PRG Release declarative child subtree should keep the owner object alive");
            expect(!state.ole_objects[0].properties.contains("cmdsave"),
                   "inherited same-PRG Release declarative child subtree should detach the released child from the owner");
            expect(state.ole_objects[0].properties.contains("cmdcancel"),
                   "inherited same-PRG Release declarative child subtree should preserve the surviving sibling on the owner");
            expect(state.ole_objects[1].prog_id == "CancelButton",
                   "inherited same-PRG Release declarative child subtree should preserve the surviving sibling object");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while inherited same-PRG declarative subtree Release lands");
        }

        const bool has_child_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "SaveButton.Destroy";
        });
        expect(has_child_destroy_event,
               "inherited same-PRG Release declarative child subtree should dispatch the child Destroy method");

        const bool has_grandchild_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "BadgeLabel.Destroy";
        });
        expect(has_grandchild_destroy_event,
               "inherited same-PRG Release declarative child subtree should dispatch descendant Destroy methods");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_owner_release_recursively_destroys_declarative_same_prg_child_subtree_and_invalidates_owner_chain()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_release_owner_declarative_same_prg_child_subtree";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_release_owner_declarative_same_prg_child_subtree.prg";
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
            "lHasChildBeforeRelease = PEMSTATUS(oForm, 'cmdSave', 1)\n"
            "cChildCaptionBeforeRelease = oChild.Caption\n"
            "cGrandchildCaptionBeforeRelease = oGrandchild.Caption\n"
            "lReleased = oForm.Release()\n"
            "cDestroyLogAfter = cDestroyLog\n"
            "cChildParentCaptionAfter = cChildParentCaptionDuringDestroy\n"
            "cGrandchildParentCaptionAfter = cGrandchildParentCaptionDuringDestroy\n"
            "nFormDestroyCountAfter = nFormDestroyCount\n"
            "nChildDestroyCountAfter = nChildDestroyCount\n"
            "nGrandchildDestroyCountAfter = nGrandchildDestroyCount\n"
            "lHasChildAfterRelease = PEMSTATUS(oForm, 'cmdSave', 1)\n"
            "xChildAfterRelease = GETPEM(oForm, 'cmdSave')\n"
            "lChildHasParentAfterRelease = PEMSTATUS(oChild, 'Parent', 1)\n"
            "xChildParentAfterRelease = GETPEM(oChild, 'Parent')\n"
            "lChildHasClassLibraryAfterRelease = PEMSTATUS(oChild, 'ClassLibrary', 1)\n"
            "xChildClassLibraryAfterRelease = GETPEM(oChild, 'ClassLibrary')\n"
            "lGrandchildHasParentAfterRelease = PEMSTATUS(oGrandchild, 'Parent', 1)\n"
            "xGrandchildParentAfterRelease = GETPEM(oGrandchild, 'Parent')\n"
            "lFormHasCaptionAfterRelease = PEMSTATUS(oForm, 'Caption', 1)\n"
            "xFormCaptionAfterRelease = GETPEM(oForm, 'Caption')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 109)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "    PROCEDURE Destroy\n"
            "        cDestroyLog = cDestroyLog + 'form>'\n"
            "        nFormDestroyCount = nFormDestroyCount + 1\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ParentButton AS Custom\n"
            "    Caption = 'Save'\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS ParentButton\n"
            "    ADD OBJECT lblBadge AS BadgeLabel\n"
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

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native owner Release declarative same-PRG child subtree script should complete: ") + state.message +
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
        check("nchildclasscountbeforerelease", "4");
        check("cchildclass1beforerelease", "SAVEBUTTON");
        check("cchildclass2beforerelease", "PARENTBUTTON");
        check("cchildclass3beforerelease", "CUSTOM");
        check("cchildclass4beforerelease", "OBJECT");
        check("lhaschildbeforerelease", "true");
        check("cchildcaptionbeforerelease", "Save");
        check("cgrandchildcaptionbeforerelease", "Badge");
        check("lreleased", "true");
        check("cdestroylogafter", "badge>child>form>");
        check("cchildparentcaptionafter", "MainForm");
        check("cgrandchildparentcaptionafter", "Save");
        check("nformdestroycountafter", "1");
        check("nchilddestroycountafter", "1");
        check("ngrandchilddestroycountafter", "1");
        check("lhaschildafterrelease", "false");
        check("lchildhasparentafterrelease", "false");
        check("lchildhasclasslibraryafterrelease", "false");
        check("lgrandchildhasparentafterrelease", "false");
        check("lformhascaptionafterrelease", "false");
        check("ldictset", "true");
        check("ndictcompare", "109");

        const auto child_after_release = state.globals.find("xchildafterrelease");
        expect(child_after_release != state.globals.end() &&
                   child_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native owner Release declarative same-PRG child subtree should invalidate GETPEM() for the released child slot");

        const auto child_parent_after_release = state.globals.find("xchildparentafterrelease");
        expect(child_parent_after_release != state.globals.end() &&
                   child_parent_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native owner Release declarative same-PRG child subtree should invalidate GETPEM() for the released child's Parent");

        const auto child_class_library_after_release = state.globals.find("xchildclasslibraryafterrelease");
        expect(child_class_library_after_release != state.globals.end() &&
                   child_class_library_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native owner Release declarative same-PRG child subtree should invalidate GETPEM() for the released child's ClassLibrary");

        const auto grandchild_parent_after_release = state.globals.find("xgrandchildparentafterrelease");
        expect(grandchild_parent_after_release != state.globals.end() &&
                   grandchild_parent_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native owner Release declarative same-PRG child subtree should invalidate GETPEM() for the released grandchild's Parent");

        const auto form_caption_after_release = state.globals.find("xformcaptionafterrelease");
        expect(form_caption_after_release != state.globals.end() &&
                   form_caption_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native owner Release declarative same-PRG child subtree should invalidate GETPEM() for the released owner");

        expect(state.ole_objects.size() == 1U,
               "native owner Release declarative same-PRG child subtree should leave only the COM object registered after recursive release");
        if (state.ole_objects.size() == 1U)
        {
            expect(state.ole_objects[0].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native owner declarative same-PRG Release lands");
        }

        const bool has_grandchild_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "BadgeLabel.Destroy";
        });
        expect(has_grandchild_destroy_event,
               "native owner Release declarative same-PRG child subtree should dispatch the grandchild Destroy method");

        const bool has_child_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "SaveButton.Destroy";
        });
        expect(has_child_destroy_event,
               "native owner Release declarative same-PRG child subtree should dispatch the child Destroy method");

        const bool has_form_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "DemoForm.Destroy";
        });
        expect(has_form_destroy_event,
               "native owner Release declarative same-PRG child subtree should dispatch the owner Destroy method");

        fs::remove_all(temp_root, ignored);
    }

}
