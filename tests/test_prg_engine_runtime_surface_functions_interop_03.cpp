#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_runtime_olecontrol_objectverbs_surfaces_remain_coherent()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_olecontrol_objectverbs";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "runtime_olecontrol_objectverbs.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('MainForm')\n"
            "lAdded = oForm.AddObject('axHost', 'OleControl', 'MSComctlLib.ListViewCtrl')\n"
            "nHostObjectVerbsCount = oForm.axHost.ObjectVerbsCount\n"
            "xHostObjectVerbsCountGetPem = GETPEM(oForm.axHost, 'ObjectVerbsCount')\n"
            "lHostHasObjectVerbs = PEMSTATUS(oForm.axHost, 'ObjectVerbs', 1)\n"
            "lHostHasObjectVerbsCount = PEMSTATUS(oForm.axHost, 'ObjectVerbsCount', 1)\n"
            "lHostObjectVerbsReadOnly = PEMSTATUS(oForm.axHost, 'ObjectVerbs', 5)\n"
            "lHostObjectVerbsCountReadOnly = PEMSTATUS(oForm.axHost, 'ObjectVerbsCount', 5)\n"
            "cHostVerb0 = oForm.axHost.ObjectVerbs(0)\n"
            "cHostVerb1 = oForm.axHost.ObjectVerbs(1)\n"
            "xHostVerb2 = oForm.axHost.ObjectVerbs(2)\n"
            "lSetHostObjectVerbs = SETPEM(oForm.axHost, 'ObjectVerbs', 'play')\n"
            "lSetHostObjectVerbsCount = SETPEM(oForm.axHost, 'ObjectVerbsCount', 3)\n"
            "lRemoveHostObjectVerbs = REMOVEPROPERTY(oForm.axHost, 'ObjectVerbs')\n"
            "lRemoveHostObjectVerbsCount = REMOVEPROPERTY(oForm.axHost, 'ObjectVerbsCount')\n"
            "lHostDoVerbAfterInspect = oForm.axHost.DoVerb(cHostVerb0)\n"
            "oDoc = CREATEOBJECT('ExcelDoc')\n"
            "nDocObjectVerbsCount = oDoc.ObjectVerbsCount\n"
            "xDocObjectVerbsCountGetPem = GETPEM(oDoc, 'ObjectVerbsCount')\n"
            "lDocHasObjectVerbs = PEMSTATUS(oDoc, 'ObjectVerbs', 1)\n"
            "lDocHasObjectVerbsCount = PEMSTATUS(oDoc, 'ObjectVerbsCount', 1)\n"
            "lDocObjectVerbsReadOnly = PEMSTATUS(oDoc, 'ObjectVerbs', 5)\n"
            "lDocObjectVerbsCountReadOnly = PEMSTATUS(oDoc, 'ObjectVerbsCount', 5)\n"
            "cDocVerb0 = oDoc.ObjectVerbs(0)\n"
            "cDocVerb1 = oDoc.ObjectVerbs(1)\n"
            "xDocVerb2 = oDoc.ObjectVerbs(2)\n"
            "lSetDocObjectVerbs = SETPEM(oDoc, 'ObjectVerbs', 'play')\n"
            "lSetDocObjectVerbsCount = SETPEM(oDoc, 'ObjectVerbsCount', 3)\n"
            "lRemoveDocObjectVerbs = REMOVEPROPERTY(oDoc, 'ObjectVerbs')\n"
            "lRemoveDocObjectVerbsCount = REMOVEPROPERTY(oDoc, 'ObjectVerbsCount')\n"
            "lDocDoVerbAfterInspect = oDoc.DoVerb(cDocVerb1)\n"
            "RETURN\n"
            "DEFINE CLASS MainForm AS Form\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ExcelDoc AS OLEControl\n"
            "    OLEClass = 'Excel.Sheet'\n"
            "    DocumentFile = 'C:\\EXCEL\\BOOK1.XLS'\n"
            "    OLETypeAllowed = 1\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("OleControl ObjectVerbs script should complete: ") + state.message);

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

        check("ladded", "true");
        check("nhostobjectverbscount", "2");
        check("xhostobjectverbscountgetpem", "2");
        check("lhosthasobjectverbs", "true");
        check("lhosthasobjectverbscount", "true");
        check("lhostobjectverbsreadonly", "true");
        check("lhostobjectverbscountreadonly", "true");
        check("chostverb0", "edit");
        check("chostverb1", "open");
        check("xhostverb2", "");
        check("lsethostobjectverbs", "false");
        check("lsethostobjectverbscount", "false");
        check("lremovehostobjectverbs", "false");
        check("lremovehostobjectverbscount", "false");
        check("lhostdoverbafterinspect", "true");

        check("ndocobjectverbscount", "2");
        check("xdocobjectverbscountgetpem", "2");
        check("ldochasobjectverbs", "true");
        check("ldochasobjectverbscount", "true");
        check("ldocobjectverbsreadonly", "true");
        check("ldocobjectverbscountreadonly", "true");
        check("cdocverb0", "edit");
        check("cdocverb1", "open");
        check("xdocverb2", "");
        check("lsetdocobjectverbs", "false");
        check("lsetdocobjectverbscount", "false");
        check("lremovedocobjectverbs", "false");
        check("lremovedocobjectverbscount", "false");
        check("ldocdoverbafterinspect", "true");

        expect(std::any_of(state.events.begin(), state.events.end(), [](const auto &event) {
                   return event.category == "ole.invoke" &&
                          event.detail == "OleControl.doverb:-1";
               }),
               "ObjectVerbs('edit') inspection slice should keep host named-verb DoVerb activation coherent");
        expect(std::any_of(state.events.begin(), state.events.end(), [](const auto &event) {
                   return event.category == "ole.invoke" &&
                          event.detail == "ExcelDoc.doverb:-2";
               }),
               "ObjectVerbs('open') inspection slice should keep class-defined named-verb DoVerb activation coherent");

        fs::remove_all(temp_root, ignored);
    }

    void test_runtime_olecontrol_direct_contained_member_routing_remains_coherent()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_olecontrol_direct_members";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "runtime_olecontrol_direct_members.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('MainForm')\n"
            "lAdded = oForm.AddObject('axHost', 'OleControl', 'MSComctlLib.ListViewCtrl')\n"
            "oForm.axHost.Left = 25\n"
            "nHostDirectLeft = oForm.axHost.Left\n"
            "nHostObjectLeft = oForm.axHost.Object.Left\n"
            "cHostDirectCompose = oForm.axHost.Compose()\n"
            "cHostObjectCompose = oForm.axHost.Object.Compose()\n"
            "oDoc = CREATEOBJECT('ExcelDoc')\n"
            "oDoc.Visible = .T.\n"
            "lDocDirectVisible = oDoc.Visible\n"
            "lDocObjectVisible = oDoc.Object.Visible\n"
            "cDocDirectCompose = oDoc.Compose()\n"
            "cDocObjectCompose = oDoc.Object.Compose()\n"
            "RETURN\n"
            "DEFINE CLASS MainForm AS Form\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ExcelDoc AS OLEControl\n"
            "    OLEClass = 'Excel.Sheet'\n"
            "    DocumentFile = 'C:\\EXCEL\\BOOK1.XLS'\n"
            "    OLETypeAllowed = 1\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("OleControl direct contained-member routing script should complete: ") + state.message);

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

        check("ladded", "true");
        check("nhostdirectleft", "25");
        check("nhostobjectleft", "25");
        check("chostdirectcompose", "ole:MSComctlLib.ListViewCtrl.compose");
        check("chostobjectcompose", "ole:MSComctlLib.ListViewCtrl.compose");
        check("ldocdirectvisible", "true");
        check("ldocobjectvisible", "true");
        check("cdocdirectcompose", "ole:Excel.Sheet.compose");
        check("cdocobjectcompose", "ole:Excel.Sheet.compose");

        fs::remove_all(temp_root, ignored);
    }

    void test_runtime_olecontrol_application_conflict_paths_require_object_remain_coherent()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_olecontrol_application_conflicts";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "runtime_olecontrol_application_conflicts.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('MainForm')\n"
            "lAdded = oForm.AddObject('axHost', 'OleControl', 'Excel.Sheet')\n"
            "cHostApplicationName = oForm.axHost.Application.Name\n"
            "cHostObjectApplicationName = oForm.axHost.Object.Application.Name\n"
            "cHostApplicationQuit = oForm.axHost.Application.Quit()\n"
            "cHostObjectApplicationQuit = oForm.axHost.Object.Application.Quit()\n"
            "lHostHasApplication = PEMSTATUS(oForm.axHost, 'Application', 1)\n"
            "lHostApplicationReadOnly = PEMSTATUS(oForm.axHost, 'Application', 5)\n"
            "lSetHostApplication = SETPEM(oForm.axHost, 'Application', 7)\n"
            "lRemoveHostApplication = REMOVEPROPERTY(oForm.axHost, 'Application')\n"
            "oForm.axHost.Left = 25\n"
            "nHostDirectLeft = oForm.axHost.Left\n"
            "oDoc = CREATEOBJECT('ExcelDoc')\n"
            "cDocApplicationName = oDoc.Application.Name\n"
            "cDocObjectApplicationName = oDoc.Object.Application.Name\n"
            "cDocApplicationQuit = oDoc.Application.Quit()\n"
            "cDocObjectApplicationQuit = oDoc.Object.Application.Quit()\n"
            "lDocHasApplication = PEMSTATUS(oDoc, 'Application', 1)\n"
            "lDocApplicationReadOnly = PEMSTATUS(oDoc, 'Application', 5)\n"
            "lSetDocApplication = SETPEM(oDoc, 'Application', 8)\n"
            "lRemoveDocApplication = REMOVEPROPERTY(oDoc, 'Application')\n"
            "oDoc.Visible = .T.\n"
            "lDocDirectVisible = oDoc.Visible\n"
            "RETURN\n"
            "DEFINE CLASS MainForm AS Form\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ExcelDoc AS OLEControl\n"
            "    OLEClass = 'Excel.Sheet'\n"
            "    DocumentFile = 'C:\\EXCEL\\BOOK1.XLS'\n"
            "    OLETypeAllowed = 1\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("OleControl Application conflict script should complete: ") + state.message);

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

        check("ladded", "true");
        check("chostapplicationname", "Microsoft Visual FoxPro");
        check("chostobjectapplicationname", "Microsoft Excel");
        check("chostapplicationquit", "ole:Microsoft Visual FoxPro.quit");
        check("chostobjectapplicationquit", "ole:Excel.Application.quit");
        check("lhosthasapplication", "true");
        check("lhostapplicationreadonly", "true");
        check("lsethostapplication", "false");
        check("lremovehostapplication", "false");
        check("nhostdirectleft", "25");

        check("cdocapplicationname", "Microsoft Visual FoxPro");
        check("cdocobjectapplicationname", "Microsoft Excel");
        check("cdocapplicationquit", "ole:Microsoft Visual FoxPro.quit");
        check("cdocobjectapplicationquit", "ole:Excel.Application.quit");
        check("ldochasapplication", "true");
        check("ldocapplicationreadonly", "true");
        check("lsetdocapplication", "false");
        check("lremovedocapplication", "false");
        check("ldocdirectvisible", "true");

        fs::remove_all(temp_root, ignored);
    }

    void test_runtime_olecontrol_explicit_object_reference_assignment_remains_coherent()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_olecontrol_object_reference_assignment";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "runtime_olecontrol_object_reference_assignment.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('MainForm')\n"
            "lAddImage = oForm.AddObject('oleImageList', 'OleControl', 'MSComctlLib.ImageListCtrl')\n"
            "lAddTree = oForm.AddObject('oleTree', 'OleControl', 'MSComctlLib.TreeCtrl')\n"
            "oHostImageRef = oForm.oleImageList.Object\n"
            "oForm.oleTree.ImageList = oForm.oleImageList.Object\n"
            "oHostTreeImageListDirect = oForm.oleTree.ImageList\n"
            "oHostTreeImageListObject = oForm.oleTree.Object.ImageList\n"
            "cHostTreeCompose = oForm.oleTree.Compose()\n"
            "oImageDoc = CREATEOBJECT('ImageListHost')\n"
            "oTreeDoc = CREATEOBJECT('TreeHost')\n"
            "oDocImageRef = oImageDoc.Object\n"
            "oTreeDoc.ImageList = oImageDoc.Object\n"
            "oDocTreeImageListDirect = oTreeDoc.ImageList\n"
            "oDocTreeImageListObject = oTreeDoc.Object.ImageList\n"
            "cDocTreeCompose = oTreeDoc.Compose()\n"
            "RETURN\n"
            "DEFINE CLASS MainForm AS Form\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ImageListHost AS OLEControl\n"
            "    OLEClass = 'MSComctlLib.ImageListCtrl'\n"
            "ENDDEFINE\n"
            "DEFINE CLASS TreeHost AS OLEControl\n"
            "    OLEClass = 'MSComctlLib.TreeCtrl'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("OleControl explicit Object-reference assignment script should complete: ") + state.message);

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

        check("laddimage", "true");
        check("laddtree", "true");
        check("chosttreecompose", "ole:MSComctlLib.TreeCtrl.compose");
        check("cdoctreecompose", "ole:MSComctlLib.TreeCtrl.compose");

        const auto expect_same = [&](const std::string &left_name, const std::string &right_name, const std::string &message)
        {
            const auto left = state.globals.find(left_name);
            const auto right = state.globals.find(right_name);
            expect(left != state.globals.end(), left_name + " variable not found");
            expect(right != state.globals.end(), right_name + " variable not found");
            if (left != state.globals.end() && right != state.globals.end())
            {
                expect(copperfin::runtime::format_value(left->second) == copperfin::runtime::format_value(right->second),
                       message);
            }
        };

        expect_same("ohostimageref",
                    "ohosttreeimagelistdirect",
                    "Direct host ImageList assignment should preserve the explicit .Object source reference");
        expect_same("ohostimageref",
                    "ohosttreeimagelistobject",
                    "Nested host Object.ImageList should preserve the explicit .Object source reference");
        expect_same("odocimageref",
                    "odoctreeimagelistdirect",
                    "Direct class-defined ImageList assignment should preserve the explicit .Object source reference");
        expect_same("odocimageref",
                    "odoctreeimagelistobject",
                    "Nested class-defined Object.ImageList should preserve the explicit .Object source reference");

        fs::remove_all(temp_root, ignored);
    }

    void test_runtime_olecontrol_direct_member_reflection_remains_coherent()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_olecontrol_direct_member_reflection";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "runtime_olecontrol_direct_member_reflection.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('MainForm')\n"
            "lAdded = oForm.AddObject('axHost', 'OleControl', 'MSComctlLib.ListViewCtrl')\n"
            "oForm.axHost.Left = 25\n"
            "xHostDirectLeftGetPem = GETPEM(oForm.axHost, 'Left')\n"
            "xHostObjectLeftGetPem = GETPEM(oForm.axHost.Object, 'Left')\n"
            "lHostHasLeft = PEMSTATUS(oForm.axHost, 'Left', 1)\n"
            "lHostObjectHasLeft = PEMSTATUS(oForm.axHost.Object, 'Left', 1)\n"
            "lHostLeftReadOnly = PEMSTATUS(oForm.axHost, 'Left', 5)\n"
            "lHostObjectLeftReadOnly = PEMSTATUS(oForm.axHost.Object, 'Left', 5)\n"
            "oDoc = CREATEOBJECT('ExcelDoc')\n"
            "oDoc.Visible = .T.\n"
            "xDocDirectVisibleGetPem = GETPEM(oDoc, 'Visible')\n"
            "xDocObjectVisibleGetPem = GETPEM(oDoc.Object, 'Visible')\n"
            "lDocHasVisible = PEMSTATUS(oDoc, 'Visible', 1)\n"
            "lDocObjectHasVisible = PEMSTATUS(oDoc.Object, 'Visible', 1)\n"
            "lDocVisibleReadOnly = PEMSTATUS(oDoc, 'Visible', 5)\n"
            "lDocObjectVisibleReadOnly = PEMSTATUS(oDoc.Object, 'Visible', 5)\n"
            "RETURN\n"
            "DEFINE CLASS MainForm AS Form\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ExcelDoc AS OLEControl\n"
            "    OLEClass = 'Excel.Sheet'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("OleControl direct-member reflection script should complete: ") + state.message);

        const auto expect_same = [&](const std::string &left_name, const std::string &right_name, const std::string &message)
        {
            const auto left = state.globals.find(left_name);
            const auto right = state.globals.find(right_name);
            expect(left != state.globals.end(), left_name + " variable not found");
            expect(right != state.globals.end(), right_name + " variable not found");
            if (left != state.globals.end() && right != state.globals.end())
            {
                expect(copperfin::runtime::format_value(left->second) == copperfin::runtime::format_value(right->second),
                       message);
            }
        };

        expect_same("xhostdirectleftgetpem",
                    "xhostobjectleftgetpem",
                    "Direct host GETPEM() reflection should match the explicit .Object Left reflection");
        expect_same("lhosthasleft",
                    "lhostobjecthasleft",
                    "Direct host PEMSTATUS(..., 1) should match the explicit .Object Left reflection");
        expect_same("lhostleftreadonly",
                    "lhostobjectleftreadonly",
                    "Direct host PEMSTATUS(..., 5) should match the explicit .Object Left mutability reflection");
        expect_same("xdocdirectvisiblegetpem",
                    "xdocobjectvisiblegetpem",
                    "Direct class-defined GETPEM() reflection should match the explicit .Object Visible reflection");
        expect_same("ldochasvisible",
                    "ldocobjecthasvisible",
                    "Direct class-defined PEMSTATUS(..., 1) should match the explicit .Object Visible reflection");
        expect_same("ldocvisiblereadonly",
                    "ldocobjectvisiblereadonly",
                    "Direct class-defined PEMSTATUS(..., 5) should match the explicit .Object Visible mutability reflection");

        fs::remove_all(temp_root, ignored);
    }

    void test_runtime_olecontrol_direct_member_setpem_remains_coherent()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_olecontrol_direct_member_setpem";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "runtime_olecontrol_direct_member_setpem.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('MainForm')\n"
            "lAdded = oForm.AddObject('axHost', 'OleControl', 'MSComctlLib.ListViewCtrl')\n"
            "lSetHostLeft = SETPEM(oForm.axHost, 'Left', 33)\n"
            "nHostDirectLeft = oForm.axHost.Left\n"
            "nHostObjectLeft = oForm.axHost.Object.Left\n"
            "xHostDirectLeftGetPem = GETPEM(oForm.axHost, 'Left')\n"
            "xHostObjectLeftGetPem = GETPEM(oForm.axHost.Object, 'Left')\n"
            "oDoc = CREATEOBJECT('ExcelDoc')\n"
            "lSetDocVisible = SETPEM(oDoc, 'Visible', .T.)\n"
            "lDocDirectVisible = oDoc.Visible\n"
            "lDocObjectVisible = oDoc.Object.Visible\n"
            "xDocDirectVisibleGetPem = GETPEM(oDoc, 'Visible')\n"
            "xDocObjectVisibleGetPem = GETPEM(oDoc.Object, 'Visible')\n"
            "RETURN\n"
            "DEFINE CLASS MainForm AS Form\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ExcelDoc AS OLEControl\n"
            "    OLEClass = 'Excel.Sheet'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("OleControl direct-member SETPEM script should complete: ") + state.message);

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

        const auto expect_same = [&](const std::string &left_name, const std::string &right_name, const std::string &message)
        {
            const auto left = state.globals.find(left_name);
            const auto right = state.globals.find(right_name);
            expect(left != state.globals.end(), left_name + " variable not found");
            expect(right != state.globals.end(), right_name + " variable not found");
            if (left != state.globals.end() && right != state.globals.end())
            {
                expect(copperfin::runtime::format_value(left->second) == copperfin::runtime::format_value(right->second),
                       message);
            }
        };

        check("lsethostleft", "true");
        check("lsetdocvisible", "true");
        check("nhostdirectleft", "33");
        check("nhostobjectleft", "33");
        check("ldocdirectvisible", "true");
        check("ldocobjectvisible", "true");

        expect_same("xhostdirectleftgetpem",
                    "xhostobjectleftgetpem",
                    "Direct host GETPEM() should match the explicit .Object Left state after SETPEM()");
        expect_same("xdocdirectvisiblegetpem",
                    "xdocobjectvisiblegetpem",
                    "Direct class-defined GETPEM() should match the explicit .Object Visible state after SETPEM()");

        fs::remove_all(temp_root, ignored);
    }

    void test_runtime_olecontrol_direct_member_amembers_remains_coherent()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_olecontrol_direct_member_amembers";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "runtime_olecontrol_direct_member_amembers.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('MainForm')\n"
            "lAdded = oForm.AddObject('axHost', 'OleControl', 'MSComctlLib.ListViewCtrl')\n"
            "nHostProps = AMEMBERS(aHostProps, oForm.axHost, 1)\n"
            "nHostMethods = AMEMBERS(aHostMethods, oForm.axHost, 2)\n"
            "nHostObjectProps = AMEMBERS(aHostObjectProps, oForm.axHost.Object, 1)\n"
            "nHostObjectMethods = AMEMBERS(aHostObjectMethods, oForm.axHost.Object, 2)\n"
            "nHostHasLeft = ASCAN(aHostProps, 'LEFT')\n"
            "nHostHasCompose = ASCAN(aHostMethods, 'COMPOSE')\n"
            "nHostObjectHasLeft = ASCAN(aHostObjectProps, 'LEFT')\n"
            "nHostObjectHasCompose = ASCAN(aHostObjectMethods, 'COMPOSE')\n"
            "oDoc = CREATEOBJECT('ExcelDoc')\n"
            "nDocProps = AMEMBERS(aDocProps, oDoc, 1)\n"
            "nDocMethods = AMEMBERS(aDocMethods, oDoc, 2)\n"
            "nDocObjectProps = AMEMBERS(aDocObjectProps, oDoc.Object, 1)\n"
            "nDocObjectMethods = AMEMBERS(aDocObjectMethods, oDoc.Object, 2)\n"
            "nDocHasVisible = ASCAN(aDocProps, 'VISIBLE')\n"
            "nDocHasCompose = ASCAN(aDocMethods, 'COMPOSE')\n"
            "nDocObjectHasVisible = ASCAN(aDocObjectProps, 'VISIBLE')\n"
            "nDocObjectHasCompose = ASCAN(aDocObjectMethods, 'COMPOSE')\n"
            "RETURN\n"
            "DEFINE CLASS MainForm AS Form\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ExcelDoc AS OLEControl\n"
            "    OLEClass = 'Excel.Sheet'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("OleControl direct-member AMEMBERS script should complete: ") + state.message);

        const auto expect_positive = [&](const std::string &name, const std::string &message)
        {
            const auto it = state.globals.find(name);
            expect(it != state.globals.end(), name + " variable not found");
            if (it != state.globals.end())
            {
                expect(copperfin::runtime::format_value(it->second) != "0",
                       message + " expected a positive ASCAN() result");
            }
        };

        expect_positive("nhostobjecthasleft",
                        "Explicit .Object AMEMBERS(..., 1) should expose the representative Left property");
        expect_positive("nhostobjecthascompose",
                        "Explicit .Object AMEMBERS(..., 2) should expose the representative Compose method");
        expect_positive("nhosthasleft",
                        "Direct host AMEMBERS(..., 1) should expose the representative Left property");
        expect_positive("nhosthascompose",
                        "Direct host AMEMBERS(..., 2) should expose the representative Compose method");
        expect_positive("ndocobjecthasvisible",
                        "Explicit .Object AMEMBERS(..., 1) should expose the representative Visible property");
        expect_positive("ndocobjecthascompose",
                        "Explicit .Object AMEMBERS(..., 2) should expose the representative Compose method");
        expect_positive("ndochasvisible",
                        "Direct class-defined AMEMBERS(..., 1) should expose the representative Visible property");
        expect_positive("ndochascompose",
                        "Direct class-defined AMEMBERS(..., 2) should expose the representative Compose method");

        fs::remove_all(temp_root, ignored);
    }

    void test_runtime_olecontrol_direct_member_shadowing_stays_blocked()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_olecontrol_direct_member_shadowing";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "runtime_olecontrol_direct_member_shadowing.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('MainForm')\n"
            "lAdded = oForm.AddObject('axHost', 'OleControl', 'MSComctlLib.ListViewCtrl')\n"
            "lAddHostLeft = ADDPROPERTY(oForm.axHost, 'Left', 99)\n"
            "lRemoveHostLeft = REMOVEPROPERTY(oForm.axHost, 'Left')\n"
            "oForm.axHost.Left = 25\n"
            "nHostDirectLeft = oForm.axHost.Left\n"
            "nHostObjectLeft = oForm.axHost.Object.Left\n"
            "xHostDirectLeftGetPem = GETPEM(oForm.axHost, 'Left')\n"
            "nHostProps = AMEMBERS(aHostProps, oForm.axHost, 1)\n"
            "nHostHasLeft = ASCAN(aHostProps, 'LEFT')\n"
            "oDoc = CREATEOBJECT('ExcelDoc')\n"
            "lAddDocVisible = ADDPROPERTY(oDoc, 'Visible', .F.)\n"
            "lRemoveDocVisible = REMOVEPROPERTY(oDoc, 'Visible')\n"
            "oDoc.Visible = .T.\n"
            "lDocDirectVisible = oDoc.Visible\n"
            "lDocObjectVisible = oDoc.Object.Visible\n"
            "xDocDirectVisibleGetPem = GETPEM(oDoc, 'Visible')\n"
            "nDocProps = AMEMBERS(aDocProps, oDoc, 1)\n"
            "nDocHasVisible = ASCAN(aDocProps, 'VISIBLE')\n"
            "RETURN\n"
            "DEFINE CLASS MainForm AS Form\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ExcelDoc AS OLEControl\n"
            "    OLEClass = 'Excel.Sheet'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("OleControl direct-member shadowing script should complete: ") + state.message);

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

        check("laddhostleft", "false");
        check("lremovehostleft", "false");
        check("nhostdirectleft", "25");
        check("nhostobjectleft", "25");
        check("xhostdirectleftgetpem", "25");
        check("ladddocvisible", "false");
        check("lremovedocvisible", "false");
        check("ldocdirectvisible", "true");
        check("ldocobjectvisible", "true");
        check("xdocdirectvisiblegetpem", "true");

        const auto expect_positive = [&](const std::string &name, const std::string &message)
        {
            const auto it = state.globals.find(name);
            expect(it != state.globals.end(), name + " variable not found");
            if (it != state.globals.end())
            {
                expect(copperfin::runtime::format_value(it->second) != "0",
                       message + " expected a positive ASCAN() result");
            }
        };

        expect_positive("nhosthasleft",
                        "Direct host AMEMBERS(..., 1) should continue exposing Left after blocked shadow/remove attempts");
        expect_positive("ndochasvisible",
                        "Direct class-defined AMEMBERS(..., 1) should continue exposing Visible after blocked shadow/remove attempts");

        fs::remove_all(temp_root, ignored);
    }

    void test_runtime_native_visual_controls_without_hwnd_fail_deterministically()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_missing_hwnd_controls";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "runtime_missing_hwnd_controls.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('MainForm')\n"
            "lButtonHasHwnd = PEMSTATUS(oForm.cmdSave, 'hWnd', 1)\n"
            "xButtonGetPem = GETPEM(oForm.cmdSave, 'hWnd')\n"
            "lButtonSetHwnd = SETPEM(oForm.cmdSave, 'hWnd', 77)\n"
            "TRY\n"
            "  xButtonDirect = oForm.cmdSave.hWnd\n"
            "CATCH TO oButtonErr\n"
            "  cButtonErr = oButtonErr.Message\n"
            "ENDTRY\n"
            "lTextHasHwnd = PEMSTATUS(oForm.txtName, 'hWnd', 1)\n"
            "xTextGetPem = GETPEM(oForm.txtName, 'hWnd')\n"
            "lTextSetHwnd = SETPEM(oForm.txtName, 'hWnd', 88)\n"
            "TRY\n"
            "  xTextDirect = oForm.txtName.hWnd\n"
            "CATCH TO oTextErr\n"
            "  cTextErr = oTextErr.Message\n"
            "ENDTRY\n"
            "lContainerHasHwnd = PEMSTATUS(oForm.cntHost, 'hWnd', 1)\n"
            "xContainerGetPem = GETPEM(oForm.cntHost, 'hWnd')\n"
            "lContainerSetHwnd = SETPEM(oForm.cntHost, 'hWnd', 99)\n"
            "TRY\n"
            "  xContainerDirect = oForm.cntHost.hWnd\n"
            "CATCH TO oContainerErr\n"
            "  cContainerErr = oContainerErr.Message\n"
            "ENDTRY\n"
            "RETURN\n"
            "DEFINE CLASS MainForm AS Form\n"
            "    ADD OBJECT cmdSave AS CommandButton\n"
            "    ADD OBJECT txtName AS TextBox\n"
            "    ADD OBJECT cntHost AS Container\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("Unsupported native control hWnd script should complete: ") + state.message);

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

        check("lbuttonhashwnd", "false");
        check("xbuttongetpem", "");
        check("lbuttonsethwnd", "false");
        check("ltexthashwnd", "false");
        check("xtextgetpem", "");
        check("ltextsethwnd", "false");
        check("lcontainerhashwnd", "false");
        check("xcontainergetpem", "");
        check("lcontainersethwnd", "false");

        const auto button_error = state.globals.find("cbuttonerr");
        const auto text_error = state.globals.find("ctexterr");
        const auto container_error = state.globals.find("ccontainererr");
        expect(button_error != state.globals.end(),
               "Unsupported CommandButton hWnd read should populate CATCH text");
        expect(text_error != state.globals.end(),
               "Unsupported TextBox hWnd read should populate CATCH text");
        expect(container_error != state.globals.end(),
               "Unsupported Container hWnd read should populate CATCH text");

        if (button_error != state.globals.end())
        {
            const std::string message = copperfin::runtime::format_value(button_error->second);
            const std::string folded = copperfin::test_support::lowercase_copy(message);
            expect(folded.find("commandbutton.hwnd") != std::string::npos,
                   "Unsupported CommandButton hWnd read should preserve the missing member identifier");
        }
        if (text_error != state.globals.end())
        {
            const std::string message = copperfin::runtime::format_value(text_error->second);
            const std::string folded = copperfin::test_support::lowercase_copy(message);
            expect(folded.find("textbox.hwnd") != std::string::npos,
                   "Unsupported TextBox hWnd read should preserve the missing member identifier");
        }
        if (container_error != state.globals.end())
        {
            const std::string message = copperfin::runtime::format_value(container_error->second);
            const std::string folded = copperfin::test_support::lowercase_copy(message);
            expect(folded.find("container.hwnd") != std::string::npos,
                   "Unsupported Container hWnd read should preserve the missing member identifier");
        }

        fs::remove_all(temp_root, ignored);
    }

}
