#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_inherited_owner_release_recursively_destroys_object_block_external_base_child_subtree_and_invalidates_owner_chain()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_inherited_release_owner_object_block_external_base_child_subtree";
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

        const fs::path main_path = temp_root / "inherited_release_owner_object_block_external_base_child_subtree.prg";
        write_text(
            main_path,
            "cDestroyLog = ''\n"
            "cChildParentCaptionDuringDestroy = ''\n"
            "cGrandchildParentCaptionDuringDestroy = ''\n"
            "nFormDestroyCount = 0\n"
            "nChildDestroyCount = 0\n"
            "nGrandchildDestroyCount = 0\n"
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oChild = oCreate.cmdSave\n"
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
            "lHasChildBeforeRelease = PEMSTATUS(oCreate, 'cmdSave', 1)\n"
            "cChildCaptionBeforeRelease = oChild.Caption\n"
            "cGrandchildCaptionBeforeRelease = oGrandchild.Caption\n"
            "lReleased = oCreate.Release()\n"
            "cDestroyLogAfter = cDestroyLog\n"
            "cChildParentCaptionAfter = cChildParentCaptionDuringDestroy\n"
            "cGrandchildParentCaptionAfter = cGrandchildParentCaptionDuringDestroy\n"
            "nFormDestroyCountAfter = nFormDestroyCount\n"
            "nChildDestroyCountAfter = nChildDestroyCount\n"
            "nGrandchildDestroyCountAfter = nGrandchildDestroyCount\n"
            "lHasChildAfterRelease = PEMSTATUS(oCreate, 'cmdSave', 1)\n"
            "xChildAfterRelease = GETPEM(oCreate, 'cmdSave')\n"
            "lChildHasParentAfterRelease = PEMSTATUS(oChild, 'Parent', 1)\n"
            "xChildParentAfterRelease = GETPEM(oChild, 'Parent')\n"
            "lChildHasClassLibraryAfterRelease = PEMSTATUS(oChild, 'ClassLibrary', 1)\n"
            "xChildClassLibraryAfterRelease = GETPEM(oChild, 'ClassLibrary')\n"
            "lGrandchildHasParentAfterRelease = PEMSTATUS(oGrandchild, 'Parent', 1)\n"
            "xGrandchildParentAfterRelease = GETPEM(oGrandchild, 'Parent')\n"
            "lFormHasCaptionAfterRelease = PEMSTATUS(oCreate, 'Caption', 1)\n"
            "xFormCaptionAfterRelease = GETPEM(oCreate, 'Caption')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 122)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ParentForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    OBJECT cmdSave AS SaveButton\n"
            "        Caption = 'Commit'\n"
            "    ENDOBJECT\n"
            "    PROCEDURE Destroy\n"
            "        cDestroyLog = cDestroyLog + 'form>'\n"
            "        nFormDestroyCount = nFormDestroyCount + 1\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ChildForm AS ParentForm\n"
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
               std::string("inherited owner Release object-block external-base child subtree script should complete: ") + state.message +
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
        check("lhaschildbeforerelease", "true");
        check("cchildcaptionbeforerelease", "Commit");
        check("cgrandchildcaptionbeforerelease", "Badge");
        check("lreleased", "true");
        check("cdestroylogafter", "badge>child>form>");
        check("cchildparentcaptionafter", "MainForm");
        check("cgrandchildparentcaptionafter", "Commit");
        check("nformdestroycountafter", "1");
        check("nchilddestroycountafter", "1");
        check("ngrandchilddestroycountafter", "1");
        check("lhaschildafterrelease", "false");
        check("lchildhasparentafterrelease", "false");
        check("lchildhasclasslibraryafterrelease", "false");
        check("lgrandchildhasparentafterrelease", "false");
        check("lformhascaptionafterrelease", "false");
        check("ldictset", "true");
        check("ndictcompare", "122");

        const auto child_after_release = state.globals.find("xchildafterrelease");
        expect(child_after_release != state.globals.end() &&
                   child_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited owner Release object-block external-base child subtree should invalidate GETPEM() for the released child slot");

        const auto child_parent_after_release = state.globals.find("xchildparentafterrelease");
        expect(child_parent_after_release != state.globals.end() &&
                   child_parent_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited owner Release object-block external-base child subtree should invalidate GETPEM() for the released child's Parent");

        const auto child_class_library_after_release = state.globals.find("xchildclasslibraryafterrelease");
        expect(child_class_library_after_release != state.globals.end() &&
                   child_class_library_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited owner Release object-block external-base child subtree should invalidate GETPEM() for the released child's ClassLibrary");

        const auto grandchild_parent_after_release = state.globals.find("xgrandchildparentafterrelease");
        expect(grandchild_parent_after_release != state.globals.end() &&
                   grandchild_parent_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited owner Release object-block external-base child subtree should invalidate GETPEM() for the released grandchild's Parent");

        const auto form_caption_after_release = state.globals.find("xformcaptionafterrelease");
        expect(form_caption_after_release != state.globals.end() &&
                   form_caption_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited owner Release object-block external-base child subtree should invalidate GETPEM() for the released owner");

        expect(state.ole_objects.size() == 1U,
               "inherited owner Release object-block external-base child subtree should leave only the COM object registered after recursive release");
        if (state.ole_objects.size() == 1U)
        {
            expect(state.ole_objects[0].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while inherited owner object-block external-base Release lands");
        }

        const bool has_grandchild_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "BadgeLabel.Destroy";
        });
        expect(has_grandchild_destroy_event,
               "inherited owner Release object-block external-base child subtree should dispatch the grandchild Destroy method");

        const bool has_child_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "SaveButton.Destroy";
        });
        expect(has_child_destroy_event,
               "inherited owner Release object-block external-base child subtree should dispatch the child Destroy method");

        const bool has_form_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "ParentForm.Destroy";
        });
        expect(has_form_destroy_event,
               "inherited owner Release object-block external-base child subtree should dispatch the owner Destroy method");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_owner_release_recursively_destroys_object_block_external_child_subtree_and_invalidates_owner_chain()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_release_owner_object_block_external_child_subtree";
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

        const fs::path main_path = temp_root / "native_release_owner_object_block_external_child_subtree.prg";
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
            "cChildClass5BeforeRelease = aChildClass[5]\n"
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
            "lDictSet = SETPEM(oDict, 'comparemode', 105)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    OBJECT cmdSave AS SaveButton OF buttons.prg\n"
            "        Caption = 'Commit'\n"
            "    ENDOBJECT\n"
            "    PROCEDURE Destroy\n"
            "        cDestroyLog = cDestroyLog + 'form>'\n"
            "        nFormDestroyCount = nFormDestroyCount + 1\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native owner Release object-block external child subtree script should complete: ") + state.message +
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
        check("lhaschildbeforerelease", "true");
        check("cchildcaptionbeforerelease", "Commit");
        check("cgrandchildcaptionbeforerelease", "Badge");
        check("lreleased", "true");
        check("cdestroylogafter", "badge>child>form>");
        check("cchildparentcaptionafter", "MainForm");
        check("cgrandchildparentcaptionafter", "Commit");
        check("nformdestroycountafter", "1");
        check("nchilddestroycountafter", "1");
        check("ngrandchilddestroycountafter", "1");
        check("lhaschildafterrelease", "false");
        check("lchildhasparentafterrelease", "false");
        check("lchildhasclasslibraryafterrelease", "false");
        check("lgrandchildhasparentafterrelease", "false");
        check("lformhascaptionafterrelease", "false");
        check("ldictset", "true");
        check("ndictcompare", "105");

        const auto child_after_release = state.globals.find("xchildafterrelease");
        expect(child_after_release != state.globals.end() &&
                   child_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native owner Release object-block external child subtree should invalidate GETPEM() for the released child slot");

        const auto child_parent_after_release = state.globals.find("xchildparentafterrelease");
        expect(child_parent_after_release != state.globals.end() &&
                   child_parent_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native owner Release object-block external child subtree should invalidate GETPEM() for the released child's Parent");

        const auto child_class_library_after_release = state.globals.find("xchildclasslibraryafterrelease");
        expect(child_class_library_after_release != state.globals.end() &&
                   child_class_library_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native owner Release object-block external child subtree should invalidate GETPEM() for the released child's ClassLibrary");

        const auto grandchild_parent_after_release = state.globals.find("xgrandchildparentafterrelease");
        expect(grandchild_parent_after_release != state.globals.end() &&
                   grandchild_parent_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native owner Release object-block external child subtree should invalidate GETPEM() for the released grandchild's Parent");

        const auto form_caption_after_release = state.globals.find("xformcaptionafterrelease");
        expect(form_caption_after_release != state.globals.end() &&
                   form_caption_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native owner Release object-block external child subtree should invalidate GETPEM() for the released owner");

        expect(state.ole_objects.size() == 1U,
               "native owner Release object-block external child subtree should leave only the COM object registered after recursive release");
        if (state.ole_objects.size() == 1U)
        {
            expect(state.ole_objects[0].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native owner object-block external-child Release lands");
        }

        const bool has_grandchild_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "BadgeLabel.Destroy";
        });
        expect(has_grandchild_destroy_event,
               "native owner Release object-block external child subtree should dispatch the grandchild Destroy method");

        const bool has_child_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "SaveButton.Destroy";
        });
        expect(has_child_destroy_event,
               "native owner Release object-block external child subtree should dispatch the child Destroy method");

        const bool has_form_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "DemoForm.Destroy";
        });
        expect(has_form_destroy_event,
               "native owner Release object-block external child subtree should dispatch the owner Destroy method");

        fs::remove_all(temp_root, ignored);
    }

    void test_inherited_owner_release_recursively_destroys_object_block_external_child_subtree_and_invalidates_owner_chain()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_inherited_release_owner_object_block_external_child_subtree";
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
            "    PROCEDURE Destroy\n"
            "        cDestroyLog = cDestroyLog + 'form>'\n"
            "        nFormDestroyCount = nFormDestroyCount + 1\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "inherited_release_owner_object_block_external_child_subtree.prg";
        write_text(
            main_path,
            "cDestroyLog = ''\n"
            "cChildParentCaptionDuringDestroy = ''\n"
            "cGrandchildParentCaptionDuringDestroy = ''\n"
            "nFormDestroyCount = 0\n"
            "nChildDestroyCount = 0\n"
            "nGrandchildDestroyCount = 0\n"
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oChild = oCreate.cmdSave\n"
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
            "lHasChildBeforeRelease = PEMSTATUS(oCreate, 'cmdSave', 1)\n"
            "cChildCaptionBeforeRelease = oChild.Caption\n"
            "cGrandchildCaptionBeforeRelease = oGrandchild.Caption\n"
            "lReleased = oCreate.Release()\n"
            "cDestroyLogAfter = cDestroyLog\n"
            "cChildParentCaptionAfter = cChildParentCaptionDuringDestroy\n"
            "cGrandchildParentCaptionAfter = cGrandchildParentCaptionDuringDestroy\n"
            "nFormDestroyCountAfter = nFormDestroyCount\n"
            "nChildDestroyCountAfter = nChildDestroyCount\n"
            "nGrandchildDestroyCountAfter = nGrandchildDestroyCount\n"
            "lHasChildAfterRelease = PEMSTATUS(oCreate, 'cmdSave', 1)\n"
            "xChildAfterRelease = GETPEM(oCreate, 'cmdSave')\n"
            "lChildHasParentAfterRelease = PEMSTATUS(oChild, 'Parent', 1)\n"
            "xChildParentAfterRelease = GETPEM(oChild, 'Parent')\n"
            "lChildHasClassLibraryAfterRelease = PEMSTATUS(oChild, 'ClassLibrary', 1)\n"
            "xChildClassLibraryAfterRelease = GETPEM(oChild, 'ClassLibrary')\n"
            "lGrandchildHasParentAfterRelease = PEMSTATUS(oGrandchild, 'Parent', 1)\n"
            "xGrandchildParentAfterRelease = GETPEM(oGrandchild, 'Parent')\n"
            "lFormHasCaptionAfterRelease = PEMSTATUS(oCreate, 'Caption', 1)\n"
            "xFormCaptionAfterRelease = GETPEM(oCreate, 'Caption')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 106)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("inherited owner Release object-block external child subtree script should complete: ") + state.message +
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
        check("lhaschildbeforerelease", "true");
        check("cchildcaptionbeforerelease", "Commit");
        check("cgrandchildcaptionbeforerelease", "Badge");
        check("lreleased", "true");
        check("cdestroylogafter", "badge>child>form>");
        check("cchildparentcaptionafter", "MainForm");
        check("cgrandchildparentcaptionafter", "Commit");
        check("nformdestroycountafter", "1");
        check("nchilddestroycountafter", "1");
        check("ngrandchilddestroycountafter", "1");
        check("lhaschildafterrelease", "false");
        check("lchildhasparentafterrelease", "false");
        check("lchildhasclasslibraryafterrelease", "false");
        check("lgrandchildhasparentafterrelease", "false");
        check("lformhascaptionafterrelease", "false");
        check("ldictset", "true");
        check("ndictcompare", "106");

        const auto child_after_release = state.globals.find("xchildafterrelease");
        expect(child_after_release != state.globals.end() &&
                   child_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited owner Release object-block external child subtree should invalidate GETPEM() for the released child slot");

        const auto child_parent_after_release = state.globals.find("xchildparentafterrelease");
        expect(child_parent_after_release != state.globals.end() &&
                   child_parent_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited owner Release object-block external child subtree should invalidate GETPEM() for the released child's Parent");

        const auto child_class_library_after_release = state.globals.find("xchildclasslibraryafterrelease");
        expect(child_class_library_after_release != state.globals.end() &&
                   child_class_library_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited owner Release object-block external child subtree should invalidate GETPEM() for the released child's ClassLibrary");

        const auto grandchild_parent_after_release = state.globals.find("xgrandchildparentafterrelease");
        expect(grandchild_parent_after_release != state.globals.end() &&
                   grandchild_parent_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited owner Release object-block external child subtree should invalidate GETPEM() for the released grandchild's Parent");

        const auto form_caption_after_release = state.globals.find("xformcaptionafterrelease");
        expect(form_caption_after_release != state.globals.end() &&
                   form_caption_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited owner Release object-block external child subtree should invalidate GETPEM() for the released owner");

        expect(state.ole_objects.size() == 1U,
               "inherited owner Release object-block external child subtree should leave only the COM object registered after recursive release");
        if (state.ole_objects.size() == 1U)
        {
            expect(state.ole_objects[0].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while inherited owner object-block external-child Release lands");
        }

        const bool has_grandchild_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "BadgeLabel.Destroy";
        });
        expect(has_grandchild_destroy_event,
               "inherited owner Release object-block external child subtree should dispatch the grandchild Destroy method");

        const bool has_child_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "SaveButton.Destroy";
        });
        expect(has_child_destroy_event,
               "inherited owner Release object-block external child subtree should dispatch the child Destroy method");

        const bool has_form_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "ParentForm.Destroy";
        });
        expect(has_form_destroy_event,
               "inherited owner Release object-block external child subtree should dispatch the owner Destroy method");

        fs::remove_all(temp_root, ignored);
    }

    void test_newobject_with_explicit_prg_library_activates_native_class_and_preserves_explicit_targets()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_newobject_explicit_prg_library";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path library_path = temp_root / "WidgetLib.PRG";
        write_text(
            library_path,
            "DEFINE CLASS LibraryWidget AS Custom\n"
            "    Caption = 'Library'\n"
            "    lInitRan = .F.\n"
            "    nValue = 1\n"
            "    PROCEDURE Init\n"
            "        LPARAMETERS tnSeed\n"
            "        tnSeed = tnSeed + 2\n"
            "        THIS.Caption = 'LibraryInit'\n"
            "        THIS.lInitRan = .T.\n"
            "        THIS.nValue = tnSeed\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "newobject_explicit_prg_library.prg";
        write_text(
            main_path,
            "nSeed = 4\n"
            "oWidget = NEWOBJECT('LibraryWidget', 'widgetlib.prg', @nSeed)\n"
            "cCaption = oWidget.Caption\n"
            "lInitRan = oWidget.lInitRan\n"
            "nStored = oWidget.nValue\n"
            "nSeedAfter = nSeed\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 12)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "oRemote = NEWOBJECT('Session', 'app.vcx', '', '', .F., 'AppServer01')\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("explicit PRG NEWOBJECT script should complete: ") + state.message +
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

        check("ccaption", "LibraryInit");
        check("linitran", "true");
        check("nstored", "6");
        check("nseedafter", "6");
        check("ldictset", "true");
        check("ndictcompare", "12");

        expect(state.ole_objects.size() == 3U,
               "explicit PRG NEWOBJECT script should register native, COM, and server-targeted objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &native_object = state.ole_objects[0];
            expect(native_object.prog_id == "LibraryWidget",
                   "explicit PRG NEWOBJECT should preserve the external class identity");
            expect(native_object.source == library_path.string(),
                   "explicit PRG NEWOBJECT should preserve the resolved PRG library path as object provenance");

            const auto caption = native_object.properties.find("caption");
            const auto init_ran = native_object.properties.find("linitran");
            const auto value = native_object.properties.find("nvalue");
            if (caption != native_object.properties.end())
            {
                expect(copperfin::runtime::format_value(caption->second) == "LibraryInit",
                       "explicit PRG NEWOBJECT should persist Init-updated caption state");
            }
            else
            {
                expect(false, "explicit PRG NEWOBJECT should materialize the caption property");
            }
            if (init_ran != native_object.properties.end())
            {
                expect(copperfin::runtime::format_value(init_ran->second) == "true",
                       "explicit PRG NEWOBJECT should persist Init-updated flags");
            }
            else
            {
                expect(false, "explicit PRG NEWOBJECT should materialize the Init flag");
            }
            if (value != native_object.properties.end())
            {
                expect(copperfin::runtime::format_value(value->second) == "6",
                       "explicit PRG NEWOBJECT should persist constructor/by-reference updates");
            }
            else
            {
                expect(false, "explicit PRG NEWOBJECT should materialize the numeric property");
            }

            expect(state.ole_objects[1].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while explicit PRG library activation lands");
            expect(state.ole_objects[2].prog_id == "Session",
                   "server-targeted NEWOBJECT should remain stable while explicit PRG library activation lands");
            expect(state.ole_objects[2].source == "app.vcx@AppServer01",
                   "server-targeted NEWOBJECT should preserve library/server source metadata");
        }

        const bool has_native_init_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.init" &&
                   event.detail == "LibraryWidget.Init";
        });
        expect(has_native_init_event,
               "explicit PRG NEWOBJECT should emit native Init events");

        const bool has_remote_detail = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "ole.newobject" &&
                   event.detail == "Session:app.vcx@AppServer01";
        });
        expect(has_remote_detail,
               "server-targeted NEWOBJECT should preserve event detail while explicit PRG library activation lands");

        fs::remove_all(temp_root, ignored);
    }

}
