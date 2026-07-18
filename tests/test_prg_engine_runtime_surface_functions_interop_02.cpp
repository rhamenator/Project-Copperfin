#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_runtime_olecontrol_hwnd_and_windows_message_binding_surfaces_remain_coherent()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_olecontrol_hwnd";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "runtime_olecontrol_hwnd.prg";
        write_text(
            main_path,
            "oHandler = CREATEOBJECT('HandleSink')\n"
            "oForm = CREATEOBJECT('MainForm')\n"
            "lAdded = oForm.AddObject('axHost', 'OleControl', 'MSComctlLib.ListViewCtrl')\n"
            "nHostHwnd = oForm.axHost.hWnd\n"
            "cHostOleClass = oForm.axHost.OLEClass\n"
            "xHostGetPem = GETPEM(oForm.axHost, 'hWnd')\n"
            "xHostOleClassGetPem = GETPEM(oForm.axHost, 'OLEClass')\n"
            "lHostHasHwnd = PEMSTATUS(oForm.axHost, 'hWnd', 1)\n"
            "lHostHasOleClass = PEMSTATUS(oForm.axHost, 'OLEClass', 1)\n"
            "lHostHwndReadOnly = PEMSTATUS(oForm.axHost, 'hWnd', 5)\n"
            "lHostOleClassReadOnly = PEMSTATUS(oForm.axHost, 'OLEClass', 5)\n"
            "lSetHostHwnd = SETPEM(oForm.axHost, 'hWnd', 88)\n"
            "lSetHostOleClass = SETPEM(oForm.axHost, 'OLEClass', 'Other.Control')\n"
            "lRemoveHostOleClass = REMOVEPROPERTY(oForm.axHost, 'OLEClass')\n"
            "cHostOleClassAfterSet = oForm.axHost.OLEClass\n"
            "cHostBaseClass = oForm.axHost.BaseClass\n"
            "nHostWHandle = SYS(2326, nHostHwnd)\n"
            "nHostRoundTrip = SYS(2327, nHostWHandle)\n"
            "nBindHost = BINDEVENT(nHostHwnd, 516, oHandler, 'HandleHost')\n"
            "READ EVENTS\n"
            "RETURN\n"
            "DEFINE CLASS MainForm AS Form\n"
            "ENDDEFINE\n"
            "DEFINE CLASS HandleSink AS Custom\n"
            "    FUNCTION HandleHost\n"
            "        LPARAMETERS tnHwnd, tnMessage, tnWParam, tnLParam\n"
            "        nHostDispatchHwnd = tnHwnd\n"
            "        nHostDispatchWHandle = SYS(2326, tnHwnd)\n"
            "        nHostDispatchRoundTrip = SYS(2327, nHostDispatchWHandle)\n"
            "        CLEAR EVENTS\n"
            "        RETURN tnMessage + 1000\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop,
               std::string("OleControl hWnd script should pause in READ EVENTS: ") + state.message);
        expect(state.waiting_for_events,
               "OleControl hWnd script should report waiting_for_events while paused");

        const auto host_hwnd_it = state.globals.find("nhosthwnd");
        expect(host_hwnd_it != state.globals.end(),
               "OleControl hWnd script should capture the host control hWnd before dispatch");
        const auto value_to_int64 = [](const copperfin::runtime::PrgValue &value) -> std::int64_t
        {
            switch (value.kind)
            {
            case copperfin::runtime::PrgValueKind::boolean:
                return value.boolean_value ? 1 : 0;
            case copperfin::runtime::PrgValueKind::number:
                return static_cast<std::int64_t>(value.number_value);
            case copperfin::runtime::PrgValueKind::int64:
                return value.int64_value;
            case copperfin::runtime::PrgValueKind::uint64:
                return static_cast<std::int64_t>(value.uint64_value);
            case copperfin::runtime::PrgValueKind::string:
                try
                {
                    return std::stoll(value.string_value);
                }
                catch (...)
                {
                    return 0;
                }
            case copperfin::runtime::PrgValueKind::empty:
                return 0;
            }
            return 0;
        };

        const std::intptr_t host_hwnd = host_hwnd_it == state.globals.end()
                                            ? 0
                                            : static_cast<std::intptr_t>(value_to_int64(host_hwnd_it->second));

        const auto dispatch = session.dispatch_windows_message(host_hwnd, 516, 7, 8);
        expect(dispatch.has_value(),
               "OleControl hWnd slice should dispatch through the representative ActiveX/OleControl hWnd");
        if (dispatch.has_value())
        {
            expect(*dispatch == 1516,
                   "OleControl hWnd dispatch should return the delegate message result");
        }

        state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("OleControl hWnd script should complete after CLEAR EVENTS: ") + state.message +
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
        const auto require_number = [&](const std::string &name) -> std::int64_t
        {
            const auto it = state.globals.find(name);
            if (it == state.globals.end())
            {
                expect(false, name + " variable not found");
                return 0;
            }
            return value_to_int64(it->second);
        };

        check("ladded", "true");
        check("chostoleclass", "MSComctlLib.ListViewCtrl");
        check("chostoleclassafterset", "MSComctlLib.ListViewCtrl");
        check("lhosthashwnd", "true");
        check("lhosthasoleclass", "true");
        check("lhosthwndreadonly", "true");
        check("lhostoleclassreadonly", "true");
        check("lsethosthwnd", "false");
        check("lsethostoleclass", "false");
        check("lremovehostoleclass", "false");
        check("chostbaseclass", "OleControl");
        check("nbindhost", "1");
        check("nhostdispatchhwnd", copperfin::runtime::format_value(state.globals.at("nhosthwnd")));

        const std::int64_t host_whandle = require_number("nhostwhandle");
        const std::int64_t host_hwnd_after = require_number("nhosthwnd");
        const std::int64_t host_getpem = require_number("xhostgetpem");
        const std::int64_t host_round_trip = require_number("nhostroundtrip");
        const std::int64_t host_dispatch_whandle = require_number("nhostdispatchwhandle");
        const std::int64_t host_dispatch_round_trip = require_number("nhostdispatchroundtrip");
        const auto host_oleclass_property = state.globals.find("chostoleclass");
        const auto host_oleclass_getpem = state.globals.find("xhostoleclassgetpem");
        expect(host_oleclass_property != state.globals.end(),
               "OleControl provenance slice should capture the host OLEClass through ordinary property access");
        expect(host_oleclass_getpem != state.globals.end(),
               "OleControl provenance slice should capture the host OLEClass through GETPEM()");

        expect(host_whandle > 0,
               "OleControl hWnd translation should expose a positive WHANDLE");
        expect(host_hwnd_after == host_getpem,
               "OleControl hWnd should read consistently through ordinary property access and GETPEM()");
        if (host_oleclass_property != state.globals.end() && host_oleclass_getpem != state.globals.end())
        {
            expect(copperfin::runtime::format_value(host_oleclass_property->second) ==
                       copperfin::runtime::format_value(host_oleclass_getpem->second),
                   "OleControl provenance should read consistently through ordinary property access and GETPEM()");
        }
        expect(host_round_trip == host_hwnd_after,
               "SYS(2326/2327) should round-trip the OleControl hWnd");
        expect(host_hwnd_after == 100000 + host_whandle,
               "OleControl hWnd should follow the deterministic modeled runtime mapping");
        expect(host_dispatch_whandle == host_whandle,
               "Dispatched OleControl hWnd should translate back to the same WHANDLE");
        expect(host_dispatch_round_trip == host_hwnd_after,
               "Dispatched OleControl hWnd should round-trip through SYS(2326/2327)");

        fs::remove_all(temp_root, ignored);
    }

    void test_runtime_olecontrol_documentfile_and_oletypeallowed_surfaces_remain_coherent()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_olecontrol_documentfile";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "runtime_olecontrol_documentfile.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('MainForm')\n"
            "lAdded = oForm.AddObject('axHost', 'OleControl', 'MSComctlLib.ListViewCtrl')\n"
            "cHostDocumentFile = oForm.axHost.DocumentFile\n"
            "nHostOleTypeAllowed = oForm.axHost.OLETypeAllowed\n"
            "xHostDocumentFileGetPem = GETPEM(oForm.axHost, 'DocumentFile')\n"
            "xHostOleTypeAllowedGetPem = GETPEM(oForm.axHost, 'OLETypeAllowed')\n"
            "lHostHasDocumentFile = PEMSTATUS(oForm.axHost, 'DocumentFile', 1)\n"
            "lHostHasOleTypeAllowed = PEMSTATUS(oForm.axHost, 'OLETypeAllowed', 1)\n"
            "lHostDocumentFileReadOnly = PEMSTATUS(oForm.axHost, 'DocumentFile', 5)\n"
            "lHostOleTypeAllowedReadOnly = PEMSTATUS(oForm.axHost, 'OLETypeAllowed', 5)\n"
            "lSetHostDocumentFile = SETPEM(oForm.axHost, 'DocumentFile', 'C:\\Temp\\Host.xls')\n"
            "lSetHostOleTypeAllowed = SETPEM(oForm.axHost, 'OLETypeAllowed', 1)\n"
            "lRemoveHostDocumentFile = REMOVEPROPERTY(oForm.axHost, 'DocumentFile')\n"
            "lRemoveHostOleTypeAllowed = REMOVEPROPERTY(oForm.axHost, 'OLETypeAllowed')\n"
            "cHostDocumentFileAfterSet = oForm.axHost.DocumentFile\n"
            "nHostOleTypeAllowedAfterSet = oForm.axHost.OLETypeAllowed\n"
            "oDoc = CREATEOBJECT('ExcelDoc')\n"
            "cDocDocumentFile = oDoc.DocumentFile\n"
            "nDocOleTypeAllowed = oDoc.OLETypeAllowed\n"
            "xDocDocumentFileGetPem = GETPEM(oDoc, 'DocumentFile')\n"
            "xDocOleTypeAllowedGetPem = GETPEM(oDoc, 'OLETypeAllowed')\n"
            "lDocHasDocumentFile = PEMSTATUS(oDoc, 'DocumentFile', 1)\n"
            "lDocHasOleTypeAllowed = PEMSTATUS(oDoc, 'OLETypeAllowed', 1)\n"
            "lDocDocumentFileReadOnly = PEMSTATUS(oDoc, 'DocumentFile', 5)\n"
            "lDocOleTypeAllowedReadOnly = PEMSTATUS(oDoc, 'OLETypeAllowed', 5)\n"
            "lSetDocDocumentFile = SETPEM(oDoc, 'DocumentFile', 'C:\\Temp\\Doc.xls')\n"
            "lSetDocOleTypeAllowed = SETPEM(oDoc, 'OLETypeAllowed', 0)\n"
            "lRemoveDocDocumentFile = REMOVEPROPERTY(oDoc, 'DocumentFile')\n"
            "lRemoveDocOleTypeAllowed = REMOVEPROPERTY(oDoc, 'OLETypeAllowed')\n"
            "cDocDocumentFileAfterSet = oDoc.DocumentFile\n"
            "nDocOleTypeAllowedAfterSet = oDoc.OLETypeAllowed\n"
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
               std::string("OleControl DocumentFile/OLETypeAllowed script should complete: ") + state.message);

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
        check("chostdocumentfile", "");
        check("xhostdocumentfilegetpem", "");
        check("nhostoletypeallowed", "-2");
        check("xhostoletypeallowedgetpem", "-2");
        check("lhosthasdocumentfile", "true");
        check("lhosthasoletypeallowed", "true");
        check("lhostdocumentfilereadonly", "true");
        check("lhostoletypeallowedreadonly", "true");
        check("lsethostdocumentfile", "false");
        check("lsethostoletypeallowed", "false");
        check("lremovehostdocumentfile", "false");
        check("lremovehostoletypeallowed", "false");
        check("chostdocumentfileafterset", "");
        check("nhostoletypeallowedafterset", "-2");

        check("cdocdocumentfile", "C:\\EXCEL\\BOOK1.XLS");
        check("xdocdocumentfilegetpem", "C:\\EXCEL\\BOOK1.XLS");
        check("ndocoletypeallowed", "1");
        check("xdocoletypeallowedgetpem", "1");
        check("ldochasdocumentfile", "true");
        check("ldochasoletypeallowed", "true");
        check("ldocdocumentfilereadonly", "true");
        check("ldocoletypeallowedreadonly", "true");
        check("lsetdocdocumentfile", "false");
        check("lsetdocoletypeallowed", "false");
        check("lremovedocdocumentfile", "false");
        check("lremovedocoletypeallowed", "false");
        check("cdocdocumentfileafterset", "C:\\EXCEL\\BOOK1.XLS");
        check("ndocoletypeallowedafterset", "1");

        fs::remove_all(temp_root, ignored);
    }

    void test_runtime_olecontrol_object_and_doverb_surfaces_remain_coherent()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_olecontrol_object";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "runtime_olecontrol_object.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('MainForm')\n"
            "lAdded = oForm.AddObject('axHost', 'OleControl', 'MSComctlLib.ListViewCtrl')\n"
            "oHostObject = oForm.axHost.Object\n"
            "xHostObjectGetPem = GETPEM(oForm.axHost, 'Object')\n"
            "lHostHasObject = PEMSTATUS(oForm.axHost, 'Object', 1)\n"
            "lHostObjectReadOnly = PEMSTATUS(oForm.axHost, 'Object', 5)\n"
            "lSetHostObject = SETPEM(oForm.axHost, 'Object', 77)\n"
            "lRemoveHostObject = REMOVEPROPERTY(oForm.axHost, 'Object')\n"
            "oForm.axHost.Object.Left = 25\n"
            "nHostObjectLeft = oForm.axHost.Object.Left\n"
            "cHostObjectCompose = oForm.axHost.Object.Compose()\n"
            "lHostDoVerbDefault = oForm.axHost.DoVerb()\n"
            "lHostDoVerbEdit = oForm.axHost.DoVerb(-1)\n"
            "oDoc = CREATEOBJECT('ExcelDoc')\n"
            "oDoc.Object.Visible = .T.\n"
            "oDocObject = oDoc.Object\n"
            "xDocObjectGetPem = GETPEM(oDoc, 'Object')\n"
            "lDocHasObject = PEMSTATUS(oDoc, 'Object', 1)\n"
            "lDocObjectReadOnly = PEMSTATUS(oDoc, 'Object', 5)\n"
            "lSetDocObject = SETPEM(oDoc, 'Object', 99)\n"
            "lRemoveDocObject = REMOVEPROPERTY(oDoc, 'Object')\n"
            "lDocVisible = oDoc.Object.Visible\n"
            "lDocDoVerbOpen = oDoc.DoVerb(-2)\n"
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
               std::string("OleControl Object/DoVerb script should complete: ") + state.message);

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
        check("lhosthasobject", "true");
        check("lhostobjectreadonly", "true");
        check("lsethostobject", "false");
        check("lremovehostobject", "false");
        check("nhostobjectleft", "25");
        check("chostobjectcompose", "ole:MSComctlLib.ListViewCtrl.compose");
        check("lhostdoverbdefault", "true");
        check("lhostdoverbedit", "true");
        check("ldochasobject", "true");
        check("ldocobjectreadonly", "true");
        check("lsetdocobject", "false");
        check("lremovedocobject", "false");
        check("ldocvisible", "true");
        check("ldocdoverbopen", "true");

        const auto host_object = state.globals.find("ohostobject");
        const auto host_object_getpem = state.globals.find("xhostobjectgetpem");
        expect(host_object != state.globals.end(),
               "OleControl Object slice should capture the host Object through ordinary property access");
        expect(host_object_getpem != state.globals.end(),
               "OleControl Object slice should capture the host Object through GETPEM()");
        if (host_object != state.globals.end() && host_object_getpem != state.globals.end())
        {
            expect(copperfin::runtime::format_value(host_object->second) ==
                       copperfin::runtime::format_value(host_object_getpem->second),
                   "OleControl Object should read consistently through ordinary property access and GETPEM()");
            expect(copperfin::runtime::format_value(host_object->second).find("object:MSComctlLib.ListViewCtrl#") == 0U,
                   "OleControl Object should resolve to a representative nested Automation object reference");
        }

        const auto doc_object = state.globals.find("odocobject");
        const auto doc_object_getpem = state.globals.find("xdocobjectgetpem");
        expect(doc_object != state.globals.end(),
               "Class-defined OleControl slice should capture the nested Object through ordinary property access");
        expect(doc_object_getpem != state.globals.end(),
               "Class-defined OleControl slice should capture the nested Object through GETPEM()");
        if (doc_object != state.globals.end() && doc_object_getpem != state.globals.end())
        {
            expect(copperfin::runtime::format_value(doc_object->second) ==
                       copperfin::runtime::format_value(doc_object_getpem->second),
                   "Class-defined OleControl Object should read consistently through ordinary property access and GETPEM()");
            expect(copperfin::runtime::format_value(doc_object->second).find("object:Excel.Sheet#") == 0U,
                   "Class-defined OleControl Object should resolve to the OLEClass-based nested Automation object reference");
        }

        bool saw_host_doverb = false;
        bool saw_doc_doverb = false;
        for (const auto &object_state : state.ole_objects)
        {
            if (object_state.prog_id == "MSComctlLib.ListViewCtrl" &&
                object_state.last_action == "activate:-1")
            {
                saw_host_doverb = true;
            }
            if (object_state.prog_id == "Excel.Sheet" &&
                object_state.last_action == "activate:-2")
            {
                saw_doc_doverb = true;
            }
        }
        expect(saw_host_doverb,
               "OleControl DoVerb slice should preserve activation intent on the nested ActiveX object state");
        expect(saw_doc_doverb,
               "Class-defined OleControl DoVerb slice should preserve activation intent on the nested Automation object state");

        expect(std::any_of(state.events.begin(), state.events.end(), [](const auto &event) {
                   return event.category == "ole.invoke" &&
                          event.detail == "OleControl.doverb:-1";
               }),
               "OleControl DoVerb slice should emit the representative activation event detail for the host edit verb");
        expect(std::any_of(state.events.begin(), state.events.end(), [](const auto &event) {
                   return event.category == "ole.invoke" &&
                          event.detail == "ExcelDoc.doverb:-2";
               }),
               "Class-defined OleControl DoVerb slice should emit the representative activation event detail for the open verb");

        fs::remove_all(temp_root, ignored);
    }

    void test_runtime_olecontrol_autoactivate_and_autoverbmenu_surfaces_remain_coherent()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_olecontrol_activation_policy";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "runtime_olecontrol_activation_policy.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('MainForm')\n"
            "lAdded = oForm.AddObject('axHost', 'OleControl', 'MSComctlLib.ListViewCtrl')\n"
            "nHostAutoActivate = oForm.axHost.AutoActivate\n"
            "lHostAutoVerbMenu = oForm.axHost.AutoVerbMenu\n"
            "xHostAutoActivateGetPem = GETPEM(oForm.axHost, 'AutoActivate')\n"
            "xHostAutoVerbMenuGetPem = GETPEM(oForm.axHost, 'AutoVerbMenu')\n"
            "lHostHasAutoActivate = PEMSTATUS(oForm.axHost, 'AutoActivate', 1)\n"
            "lHostHasAutoVerbMenu = PEMSTATUS(oForm.axHost, 'AutoVerbMenu', 1)\n"
            "lHostAutoActivateReadOnly = PEMSTATUS(oForm.axHost, 'AutoActivate', 5)\n"
            "lHostAutoVerbMenuReadOnly = PEMSTATUS(oForm.axHost, 'AutoVerbMenu', 5)\n"
            "lSetHostAutoActivate = SETPEM(oForm.axHost, 'AutoActivate', 0)\n"
            "lSetHostAutoVerbMenu = SETPEM(oForm.axHost, 'AutoVerbMenu', .F.)\n"
            "nHostAutoActivateAfterSet = oForm.axHost.AutoActivate\n"
            "lHostAutoVerbMenuAfterSet = oForm.axHost.AutoVerbMenu\n"
            "lHostDoVerbAfterPolicy = oForm.axHost.DoVerb(-1)\n"
            "oDoc = CREATEOBJECT('ExcelDoc')\n"
            "nDocAutoActivate = oDoc.AutoActivate\n"
            "lDocAutoVerbMenu = oDoc.AutoVerbMenu\n"
            "xDocAutoActivateGetPem = GETPEM(oDoc, 'AutoActivate')\n"
            "xDocAutoVerbMenuGetPem = GETPEM(oDoc, 'AutoVerbMenu')\n"
            "lDocHasAutoActivate = PEMSTATUS(oDoc, 'AutoActivate', 1)\n"
            "lDocHasAutoVerbMenu = PEMSTATUS(oDoc, 'AutoVerbMenu', 1)\n"
            "lDocAutoActivateReadOnly = PEMSTATUS(oDoc, 'AutoActivate', 5)\n"
            "lDocAutoVerbMenuReadOnly = PEMSTATUS(oDoc, 'AutoVerbMenu', 5)\n"
            "lSetDocAutoActivate = SETPEM(oDoc, 'AutoActivate', 3)\n"
            "lSetDocAutoVerbMenu = SETPEM(oDoc, 'AutoVerbMenu', .T.)\n"
            "nDocAutoActivateAfterSet = oDoc.AutoActivate\n"
            "lDocAutoVerbMenuAfterSet = oDoc.AutoVerbMenu\n"
            "lDocDoVerbAfterPolicy = oDoc.DoVerb(0)\n"
            "RETURN\n"
            "DEFINE CLASS MainForm AS Form\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ExcelDoc AS OLEControl\n"
            "    OLEClass = 'Excel.Sheet'\n"
            "    DocumentFile = 'C:\\EXCEL\\BOOK1.XLS'\n"
            "    OLETypeAllowed = 1\n"
            "    AutoActivate = 1\n"
            "    AutoVerbMenu = .F.\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("OleControl AutoActivate/AutoVerbMenu script should complete: ") + state.message);

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
        check("nhostautoactivate", "2");
        check("lhostautoverbmenu", "true");
        check("xhostautoactivategetpem", "2");
        check("xhostautoverbmenugetpem", "true");
        check("lhosthasautoactivate", "true");
        check("lhosthasautoverbmenu", "true");
        check("lhostautoactivatereadonly", "false");
        check("lhostautoverbmenureadonly", "false");
        check("lsethostautoactivate", "true");
        check("lsethostautoverbmenu", "true");
        check("nhostautoactivateafterset", "0");
        check("lhostautoverbmenuafterset", "false");
        check("lhostdoverbafterpolicy", "true");

        check("ndocautoactivate", "1");
        check("ldocautoverbmenu", "false");
        check("xdocautoactivategetpem", "1");
        check("xdocautoverbmenugetpem", "false");
        check("ldochasautoactivate", "true");
        check("ldochasautoverbmenu", "true");
        check("ldocautoactivatereadonly", "false");
        check("ldocautoverbmenureadonly", "false");
        check("lsetdocautoactivate", "true");
        check("lsetdocautoverbmenu", "true");
        check("ndocautoactivateafterset", "3");
        check("ldocautoverbmenuafterset", "true");
        check("ldocdoverbafterpolicy", "true");

        fs::remove_all(temp_root, ignored);
    }

    void test_runtime_olecontrol_timeout_policy_surfaces_remain_coherent()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_olecontrol_timeout_policy";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "runtime_olecontrol_timeout_policy.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('MainForm')\n"
            "lAdded = oForm.AddObject('axHost', 'OleControl', 'MSComctlLib.ListViewCtrl')\n"
            "nHostRequestPending = oForm.axHost.OLERequestPendingTimeout\n"
            "nHostServerBusy = oForm.axHost.OLEServerBusyTimeout\n"
            "lHostRaiseError = oForm.axHost.OLEServerBusyRaiseError\n"
            "xHostRequestPendingGetPem = GETPEM(oForm.axHost, 'OLERequestPendingTimeout')\n"
            "xHostServerBusyGetPem = GETPEM(oForm.axHost, 'OLEServerBusyTimeout')\n"
            "xHostRaiseErrorGetPem = GETPEM(oForm.axHost, 'OLEServerBusyRaiseError')\n"
            "lHostHasRequestPending = PEMSTATUS(oForm.axHost, 'OLERequestPendingTimeout', 1)\n"
            "lHostHasServerBusy = PEMSTATUS(oForm.axHost, 'OLEServerBusyTimeout', 1)\n"
            "lHostHasRaiseError = PEMSTATUS(oForm.axHost, 'OLEServerBusyRaiseError', 1)\n"
            "lHostRequestPendingReadOnly = PEMSTATUS(oForm.axHost, 'OLERequestPendingTimeout', 5)\n"
            "lHostServerBusyReadOnly = PEMSTATUS(oForm.axHost, 'OLEServerBusyTimeout', 5)\n"
            "lHostRaiseErrorReadOnly = PEMSTATUS(oForm.axHost, 'OLEServerBusyRaiseError', 5)\n"
            "lSetHostRequestPending = SETPEM(oForm.axHost, 'OLERequestPendingTimeout', 0)\n"
            "lSetHostServerBusy = SETPEM(oForm.axHost, 'OLEServerBusyTimeout', 1200)\n"
            "lSetHostRaiseError = SETPEM(oForm.axHost, 'OLEServerBusyRaiseError', .T.)\n"
            "nHostRequestPendingAfterSet = oForm.axHost.OLERequestPendingTimeout\n"
            "nHostServerBusyAfterSet = oForm.axHost.OLEServerBusyTimeout\n"
            "lHostRaiseErrorAfterSet = oForm.axHost.OLEServerBusyRaiseError\n"
            "oDoc = CREATEOBJECT('ExcelDoc')\n"
            "nDocRequestPending = oDoc.OLERequestPendingTimeout\n"
            "nDocServerBusy = oDoc.OLEServerBusyTimeout\n"
            "lDocRaiseError = oDoc.OLEServerBusyRaiseError\n"
            "xDocRequestPendingGetPem = GETPEM(oDoc, 'OLERequestPendingTimeout')\n"
            "xDocServerBusyGetPem = GETPEM(oDoc, 'OLEServerBusyTimeout')\n"
            "xDocRaiseErrorGetPem = GETPEM(oDoc, 'OLEServerBusyRaiseError')\n"
            "lDocHasRequestPending = PEMSTATUS(oDoc, 'OLERequestPendingTimeout', 1)\n"
            "lDocHasServerBusy = PEMSTATUS(oDoc, 'OLEServerBusyTimeout', 1)\n"
            "lDocHasRaiseError = PEMSTATUS(oDoc, 'OLEServerBusyRaiseError', 1)\n"
            "lDocRequestPendingReadOnly = PEMSTATUS(oDoc, 'OLERequestPendingTimeout', 5)\n"
            "lDocServerBusyReadOnly = PEMSTATUS(oDoc, 'OLEServerBusyTimeout', 5)\n"
            "lDocRaiseErrorReadOnly = PEMSTATUS(oDoc, 'OLEServerBusyRaiseError', 5)\n"
            "lSetDocRequestPending = SETPEM(oDoc, 'OLERequestPendingTimeout', 750)\n"
            "lSetDocServerBusy = SETPEM(oDoc, 'OLEServerBusyTimeout', 3200)\n"
            "lSetDocRaiseError = SETPEM(oDoc, 'OLEServerBusyRaiseError', .F.)\n"
            "nDocRequestPendingAfterSet = oDoc.OLERequestPendingTimeout\n"
            "nDocServerBusyAfterSet = oDoc.OLEServerBusyTimeout\n"
            "lDocRaiseErrorAfterSet = oDoc.OLEServerBusyRaiseError\n"
            "lHostDoVerbAfterTimeoutPolicy = oForm.axHost.DoVerb(-1)\n"
            "lDocDoVerbAfterTimeoutPolicy = oDoc.DoVerb(0)\n"
            "RETURN\n"
            "DEFINE CLASS MainForm AS Form\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ExcelDoc AS OLEControl\n"
            "    OLEClass = 'Excel.Sheet'\n"
            "    DocumentFile = 'C:\\EXCEL\\BOOK1.XLS'\n"
            "    OLETypeAllowed = 1\n"
            "    OLERequestPendingTimeout = 250\n"
            "    OLEServerBusyTimeout = 9000\n"
            "    OLEServerBusyRaiseError = .T.\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("OleControl timeout-policy script should complete: ") + state.message);

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
        check("nhostrequestpending", "5000");
        check("nhostserverbusy", "5000");
        check("lhostraiseerror", "false");
        check("xhostrequestpendinggetpem", "5000");
        check("xhostserverbusygetpem", "5000");
        check("xhostraiseerrorgetpem", "false");
        check("lhosthasrequestpending", "true");
        check("lhosthasserverbusy", "true");
        check("lhosthasraiseerror", "true");
        check("lhostrequestpendingreadonly", "false");
        check("lhostserverbusyreadonly", "false");
        check("lhostraiseerrorreadonly", "false");
        check("lsethostrequestpending", "true");
        check("lsethostserverbusy", "true");
        check("lsethostraiseerror", "true");
        check("nhostrequestpendingafterset", "0");
        check("nhostserverbusyafterset", "1200");
        check("lhostraiseerrorafterset", "true");

        check("ndocrequestpending", "250");
        check("ndocserverbusy", "9000");
        check("ldocraiseerror", "true");
        check("xdocrequestpendinggetpem", "250");
        check("xdocserverbusygetpem", "9000");
        check("xdocraiseerrorgetpem", "true");
        check("ldochasrequestpending", "true");
        check("ldochasserverbusy", "true");
        check("ldochasraiseerror", "true");
        check("ldocrequestpendingreadonly", "false");
        check("ldocserverbusyreadonly", "false");
        check("ldocraiseerrorreadonly", "false");
        check("lsetdocrequestpending", "true");
        check("lsetdocserverbusy", "true");
        check("lsetdocraiseerror", "true");
        check("ndocrequestpendingafterset", "750");
        check("ndocserverbusyafterset", "3200");
        check("ldocraiseerrorafterset", "false");
        check("lhostdoverbaftertimeoutpolicy", "true");
        check("ldocdoverbaftertimeoutpolicy", "true");

        fs::remove_all(temp_root, ignored);
    }

    void test_runtime_olecontrol_named_doverb_surfaces_remain_coherent()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_olecontrol_named_doverb";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "runtime_olecontrol_named_doverb.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('MainForm')\n"
            "lAdded = oForm.AddObject('axHost', 'OleControl', 'MSComctlLib.ListViewCtrl')\n"
            "lHostDoVerbNamedEdit = oForm.axHost.DoVerb(' edit ')\n"
            "lHostDoVerbNamedOpen = oForm.axHost.DoVerb('OPEN')\n"
            "oDoc = CREATEOBJECT('ExcelDoc')\n"
            "lDocDoVerbNamedEdit = oDoc.DoVerb('Edit')\n"
            "lDocDoVerbNamedOpen = oDoc.DoVerb(' open ')\n"
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
               std::string("OleControl named DoVerb script should complete: ") + state.message);

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
        check("lhostdoverbnamededit", "true");
        check("lhostdoverbnamedopen", "true");
        check("ldocdoverbnamededit", "true");
        check("ldocdoverbnamedopen", "true");

        bool saw_host_edit = false;
        bool saw_host_open = false;
        bool saw_doc_edit = false;
        bool saw_doc_open = false;
        for (const auto &object_state : state.ole_objects)
        {
            if (object_state.prog_id == "MSComctlLib.ListViewCtrl" &&
                object_state.last_action == "activate:-2")
            {
                saw_host_open = true;
            }
            if (object_state.prog_id == "Excel.Sheet" &&
                object_state.last_action == "activate:-2")
            {
                saw_doc_open = true;
            }
        }
        for (const auto &event : state.events)
        {
            if (event.category != "ole.invoke")
            {
                continue;
            }
            if (event.detail == "OleControl.doverb:-1")
            {
                saw_host_edit = true;
            }
            if (event.detail == "OleControl.doverb:-2")
            {
                saw_host_open = true;
            }
            if (event.detail == "ExcelDoc.doverb:-1")
            {
                saw_doc_edit = true;
            }
            if (event.detail == "ExcelDoc.doverb:-2")
            {
                saw_doc_open = true;
            }
        }

        expect(saw_host_edit,
               "Named host DoVerb('edit') should canonicalize to the representative edit activation verb");
        expect(saw_host_open,
               "Named host DoVerb('open') should canonicalize to the representative open activation verb");
        expect(saw_doc_edit,
               "Named class-defined DoVerb('edit') should canonicalize to the representative edit activation verb");
        expect(saw_doc_open,
               "Named class-defined DoVerb('open') should canonicalize to the representative open activation verb");

        fs::remove_all(temp_root, ignored);
    }

}
