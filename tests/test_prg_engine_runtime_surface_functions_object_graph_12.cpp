#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_typed_local_newobject_method_invocation_uses_local_storage()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_typed_local_newobject";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "typed_local_newobject.prg";
        write_text(
            main_path,
            "LOCAL oWidget AS TypedWidget OF typed_local_newobject.prg\n"
            "m.oWidget = NEWOBJECT('TypedWidget', 'typed_local_newobject.prg')\n"
            "cResult = m.oWidget.Rename('Updated')\n"
            "RETURN\n"
            "DEFINE CLASS TypedWidget AS Custom\n"
            "    Caption = 'Demo'\n"
            "    FUNCTION Rename\n"
            "        LPARAMETERS tcCaption\n"
            "        THIS.Caption = tcCaption\n"
            "        RETURN THIS.Caption\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("typed local NEWOBJECT method script should complete: ") + state.message +
                   " @line=" + std::to_string(state.location.line));

        const auto result = state.globals.find("cresult");
        expect(result != state.globals.end() && copperfin::runtime::format_value(result->second) == "Updated",
               "typed local NEWOBJECT method invocation should use the local object");
        expect(state.globals.find("owidget") == state.globals.end(),
               "m-qualified typed local assignment should not create a global object variable");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_prg_object_methods_bind_this_and_persist_instance_state()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_object_methods";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_object_methods.prg";
        write_text(
            main_path,
            "oWidget = CREATEOBJECT('MyWidget')\n"
            "cBefore = oWidget.Caption\n"
            "cRenameResult = oWidget.Rename('Updated')\n"
            "cAfter = oWidget.Caption\n"
            "nFirstCount = oWidget.CountUp()\n"
            "nSecondCount = oWidget.CountUp()\n"
            "RETURN\n"
            "DEFINE CLASS MyWidget AS Custom\n"
            "    Caption = 'Demo'\n"
            "    nCount = 0\n"
            "    FUNCTION Rename\n"
            "        LPARAMETERS tcCaption\n"
            "        THIS.Caption = tcCaption\n"
            "        RETURN THIS.Caption\n"
            "    ENDFUNC\n"
            "    FUNCTION CountUp\n"
            "        THIS.nCount = THIS.nCount + 1\n"
            "        RETURN THIS.nCount\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native object method script should complete: ") + state.message +
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

        check("cbefore", "Demo");
        check("crenameresult", "Updated");
        check("cafter", "Updated");
        check("nfirstcount", "1");
        check("nsecondcount", "2");

        expect(state.ole_objects.size() == 1U,
               "native object method script should reuse one instantiated runtime object");
        if (state.ole_objects.size() == 1U)
        {
            const auto &object = state.ole_objects.front();
            expect(object.prog_id == "MyWidget",
                   "native object method script should preserve class identity on the runtime object");
            const auto caption = object.properties.find("caption");
            expect(caption != object.properties.end(),
                   "THIS-bound method writes should persist updated object properties");
            if (caption != object.properties.end())
            {
                expect(copperfin::runtime::format_value(caption->second) == "Updated",
                       "THIS-bound method writes should persist the updated caption");
            }
            const auto count = object.properties.find("ncount");
            expect(count != object.properties.end(),
                   "THIS-bound method writes should persist numeric object properties");
            if (count != object.properties.end())
            {
                expect(copperfin::runtime::format_value(count->second) == "2",
                       "THIS-bound method writes should persist the incremented count");
            }
        }

        const bool has_native_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   (event.detail == "MyWidget.Rename" || event.detail == "MyWidget.CountUp");
        });
        expect(has_native_invoke_event,
               "native object methods should emit prg.object.invoke events");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_prg_init_runs_during_object_creation_and_preserves_plain_creation()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_init_lifecycle";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_init_lifecycle.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('CreateWidget')\n"
            "oNew = NEWOBJECT('NewWidget')\n"
            "cCreateCaption = oCreate.Caption\n"
            "lCreateInitRan = oCreate.lInitRan\n"
            "nCreateCount = oCreate.nCount\n"
            "cNewCaption = oNew.Caption\n"
            "lNewInitRan = oNew.lInitRan\n"
            "nNewCount = oNew.nCount\n"
            "oPlain = CREATEOBJECT('Empty')\n"
            "oPlain.Extra = 'plain'\n"
            "cPlain = oPlain.Extra\n"
            "RETURN\n"
            "DEFINE CLASS CreateWidget AS Custom\n"
            "    Caption = 'CreateBase'\n"
            "    lInitRan = .F.\n"
            "    nCount = 1\n"
            "    PROCEDURE Init\n"
            "        THIS.Caption = THIS.Caption + '-Init'\n"
            "        THIS.lInitRan = .T.\n"
            "        THIS.nCount = THIS.nCount + 1\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS NewWidget AS Custom\n"
            "    Caption = 'NewBase'\n"
            "    lInitRan = .F.\n"
            "    nCount = 10\n"
            "    FUNCTION Init\n"
            "        THIS.Caption = THIS.Caption + '-Init'\n"
            "        THIS.lInitRan = .T.\n"
            "        THIS.nCount = THIS.nCount + 5\n"
            "        RETURN THIS.nCount\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Init lifecycle script should complete: ") + state.message +
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

        check("ccreatecaption", "CreateBase-Init");
        check("lcreateinitran", "true");
        check("ncreatecount", "2");
        check("cnewcaption", "NewBase-Init");
        check("lnewinitran", "true");
        check("nnewcount", "15");
        check("cplain", "plain");

        expect(state.ole_objects.size() == 3U,
               "native Init lifecycle script should register two native objects plus one plain object");
        if (state.ole_objects.size() == 3U)
        {
            const auto &create_object = state.ole_objects[0];
            expect(create_object.prog_id == "CreateWidget",
                   "CREATEOBJECT native Init lifecycle should preserve class identity");
            const auto create_caption = create_object.properties.find("caption");
            expect(create_caption != create_object.properties.end(),
                   "CREATEOBJECT native Init lifecycle should persist caption updates");
            if (create_caption != create_object.properties.end())
            {
                expect(copperfin::runtime::format_value(create_caption->second) == "CreateBase-Init",
                       "CREATEOBJECT native Init lifecycle should apply Init updates after default property materialization");
            }

            const auto &new_object = state.ole_objects[1];
            expect(new_object.prog_id == "NewWidget",
                   "bare NEWOBJECT native Init lifecycle should preserve class identity");
            const auto new_count = new_object.properties.find("ncount");
            expect(new_count != new_object.properties.end(),
                   "bare NEWOBJECT native Init lifecycle should persist Init-written numeric properties");
            if (new_count != new_object.properties.end())
            {
                expect(copperfin::runtime::format_value(new_count->second) == "15",
                       "bare NEWOBJECT native Init lifecycle should run Init during creation");
            }

            expect(state.ole_objects[2].prog_id == "Empty",
                   "plain CREATEOBJECT should remain stable alongside native Init lifecycle");
        }

        const bool has_init_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.init" &&
                   (event.detail == "CreateWidget.Init" || event.detail == "NewWidget.Init");
        });
        expect(has_init_event,
               "native object creation should emit prg.object.init events when Init runs");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_prg_init_returning_false_prevents_object_creation()
    {
        namespace fs = std::filesystem;
        const std::array<std::pair<std::string, std::string>, 2U> scenarios = {{
            {"CREATEOBJECT", "CREATEOBJECT('RejectedWidget')"},
            {"NEWOBJECT", "NEWOBJECT('RejectedWidget')"},
        }};
        for (const auto &[operation, construction_expression] : scenarios)
        {
            const fs::path temp_root = fs::temp_directory_path() /
                ("copperfin_native_prg_init_rejection_" +
                 (operation == "CREATEOBJECT" ? std::string{"createobject"} : std::string{"newobject"}));
            std::error_code ignored;
            fs::remove_all(temp_root, ignored);
            fs::create_directories(temp_root);

            const fs::path main_path = temp_root / "native_init_rejection.prg";
            write_text(
                main_path,
                "oRejected = " + construction_expression + "\n"
                "lRejectedIsNull = ISNULL(oRejected)\n"
                "RETURN\n"
                "DEFINE CLASS RejectedWidget AS Container\n"
                "    ADD OBJECT Probe AS Custom\n"
                "    PROCEDURE Init\n"
                "        RETURN .F.\n"
                "    ENDPROC\n"
                "    PROCEDURE Destroy\n"
                "        RETURN\n"
                "    ENDPROC\n"
                "ENDDEFINE\n");

            copperfin::runtime::PrgRuntimeSession session =
                copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

            const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
            expect(state.completed,
                   operation + " should continue when a native Init explicitly returns .F.: " + state.message);
            expect(state.ole_objects.empty(),
                   operation + " should not retain a container or child object rejected by Init");
            const auto rejected = state.globals.find("orejected");
            expect(rejected != state.globals.end() && rejected->second.is_null,
                   operation + " should return a null object reference when Init rejects construction");
            const auto rejected_is_null = state.globals.find("lrejectedisnull");
            expect(rejected_is_null != state.globals.end() &&
                       copperfin::runtime::format_value(rejected_is_null->second) == "true",
                   operation + " should expose the rejected object reference as .NULL.");
            const bool has_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
            {
                return event.category == "prg.object.destroy" && event.detail == "RejectedWidget.Destroy";
            });
            expect(!has_destroy_event,
                   operation + " should not dispatch Destroy for an object rejected by Init");
            const std::string creation_category = operation == "CREATEOBJECT"
                ? "ole.createobject"
                : "ole.newobject";
            const bool has_creation_event = std::any_of(state.events.begin(), state.events.end(),
                [&creation_category](const auto &event)
                {
                    return event.category == creation_category &&
                           event.detail == "RejectedWidget";
                });
            expect(!has_creation_event,
                   operation + " should not emit an object-created event when Init rejects construction");
            const bool has_child_creation_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
            {
                return event.category == "prg.object.addobject" &&
                       event.detail.rfind("RejectedWidget.Probe:", 0U) == 0U;
            });
            expect(!has_child_creation_event,
                   operation + " should not retain a declarative child-created event after Init rejects construction");

            fs::remove_all(temp_root, ignored);
        }
    }

    void test_explicit_prg_newobject_init_returning_false_prevents_object_creation()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_explicit_prg_init_rejection";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path library_path = temp_root / "rejected_library.prg";
        write_text(
            library_path,
            "DEFINE CLASS RejectedLibraryWidget AS Custom\n"
            "    PROCEDURE Init\n"
            "        RETURN .F.\n"
            "    ENDPROC\n"
            "    PROCEDURE Destroy\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");
        const fs::path main_path = temp_root / "explicit_prg_init_rejection.prg";
        write_text(
            main_path,
            "oRejected = NEWOBJECT('RejectedLibraryWidget', 'rejected_library.prg')\n"
            "lRejectedIsNull = ISNULL(oRejected)\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               "explicit PRG NEWOBJECT should continue when Init returns .F.: " + state.message);
        expect(state.ole_objects.empty(),
               "explicit PRG NEWOBJECT should not retain an object rejected by Init");
        const auto rejected = state.globals.find("orejected");
        expect(rejected != state.globals.end() && rejected->second.is_null,
               "explicit PRG NEWOBJECT should return a null object reference when Init rejects construction");
        const auto rejected_is_null = state.globals.find("lrejectedisnull");
        expect(rejected_is_null != state.globals.end() &&
                   copperfin::runtime::format_value(rejected_is_null->second) == "true",
               "explicit PRG NEWOBJECT should expose the rejected reference as .NULL.");
        const bool has_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" && event.detail == "RejectedLibraryWidget.Destroy";
        });
        expect(!has_destroy_event,
               "an explicit PRG object rejected by Init should not dispatch Destroy");
        const bool has_creation_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "ole.newobject" && event.detail == "RejectedLibraryWidget:rejected_library.prg";
        });
        expect(!has_creation_event,
               "an explicit PRG object rejected by Init should not emit an object-created event");

        fs::remove_all(temp_root, ignored);
    }

    void test_createobject_arguments_flow_into_native_init_while_newobject_and_non_native_creation_stay_stable()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_createobject_init_args";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_createobject_init_args.prg";
        write_text(
            main_path,
            "oCtor = CREATEOBJECT('CtorWidget', 'Ctor', 4)\n"
            "oNew = NEWOBJECT('ZeroArgWidget')\n"
            "oPlain = CREATEOBJECT('Empty')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "oPlain.Extra = 'plain'\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 3)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "cCtorCaption = oCtor.Caption\n"
            "nCtorCount = oCtor.nCount\n"
            "lCtorInitRan = oCtor.lInitRan\n"
            "cNewCaption = oNew.Caption\n"
            "lNewInitRan = oNew.lInitRan\n"
            "cPlain = oPlain.Extra\n"
            "RETURN\n"
            "DEFINE CLASS CtorWidget AS Custom\n"
            "    Caption = 'Base'\n"
            "    nCount = 1\n"
            "    lInitRan = .F.\n"
            "    PROCEDURE Init\n"
            "        LPARAMETERS tcSuffix, tnDelta\n"
            "        THIS.Caption = THIS.Caption + '-' + tcSuffix\n"
            "        THIS.nCount = THIS.nCount + tnDelta\n"
            "        THIS.lInitRan = .T.\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ZeroArgWidget AS Custom\n"
            "    Caption = 'Zero'\n"
            "    lInitRan = .F.\n"
            "    PROCEDURE Init\n"
            "        THIS.Caption = THIS.Caption + '-Init'\n"
            "        THIS.lInitRan = .T.\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native CREATEOBJECT Init-args script should complete: ") + state.message +
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

        check("cctorcaption", "Base-Ctor");
        check("nctorcount", "5");
        check("lctorinitran", "true");
        check("cnewcaption", "Zero-Init");
        check("lnewinitran", "true");
        check("cplain", "plain");
        check("ldictset", "true");
        check("ndictcompare", "3");

        expect(state.ole_objects.size() == 4U,
               "CREATEOBJECT Init-args script should register native, native NEWOBJECT, plain, and COM objects");
        if (state.ole_objects.size() == 4U)
        {
            expect(state.ole_objects[0].prog_id == "CtorWidget",
                   "CREATEOBJECT Init-args script should preserve constructor-target class identity");
            const auto ctor_count = state.ole_objects[0].properties.find("ncount");
            expect(ctor_count != state.ole_objects[0].properties.end(),
                   "CREATEOBJECT Init-args script should persist Init-updated numeric state");
            if (ctor_count != state.ole_objects[0].properties.end())
            {
                expect(copperfin::runtime::format_value(ctor_count->second) == "5",
                       "CREATEOBJECT Init-args script should apply trailing constructor arguments inside Init");
            }

            expect(state.ole_objects[1].prog_id == "ZeroArgWidget",
                   "bare NEWOBJECT native activation should remain stable while CREATEOBJECT gains Init arguments");
            expect(state.ole_objects[2].prog_id == "Empty",
                   "plain CREATEOBJECT should remain stable while native Init gains constructor arguments");
            expect(state.ole_objects[3].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native CREATEOBJECT gains Init arguments");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_newobject_arguments_flow_into_native_init_while_createobject_and_com_newobject_stay_stable()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_newobject_init_args";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_newobject_init_args.prg";
        write_text(
            main_path,
            "oCtor = NEWOBJECT('CtorWidget', 'New', 6)\n"
            "oCreate = CREATEOBJECT('CreateWidget', 'Create', 2)\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 4)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "cCtorCaption = oCtor.Caption\n"
            "nCtorCount = oCtor.nCount\n"
            "lCtorInitRan = oCtor.lInitRan\n"
            "cCreateCaption = oCreate.Caption\n"
            "nCreateCount = oCreate.nCount\n"
            "lCreateInitRan = oCreate.lInitRan\n"
            "RETURN\n"
            "DEFINE CLASS CtorWidget AS Custom\n"
            "    Caption = 'Base'\n"
            "    nCount = 1\n"
            "    lInitRan = .F.\n"
            "    PROCEDURE Init\n"
            "        LPARAMETERS tcSuffix, tnDelta\n"
            "        THIS.Caption = THIS.Caption + '-' + tcSuffix\n"
            "        THIS.nCount = THIS.nCount + tnDelta\n"
            "        THIS.lInitRan = .T.\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS CreateWidget AS Custom\n"
            "    Caption = 'CreateBase'\n"
            "    nCount = 10\n"
            "    lInitRan = .F.\n"
            "    PROCEDURE Init\n"
            "        LPARAMETERS tcSuffix, tnDelta\n"
            "        THIS.Caption = THIS.Caption + '-' + tcSuffix\n"
            "        THIS.nCount = THIS.nCount + tnDelta\n"
            "        THIS.lInitRan = .T.\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native NEWOBJECT Init-args script should complete: ") + state.message +
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

        check("cctorcaption", "Base-New");
        check("nctorcount", "7");
        check("lctorinitran", "true");
        check("ccreatecaption", "CreateBase-Create");
        check("ncreatecount", "12");
        check("lcreateinitran", "true");
        check("ldictset", "true");
        check("ndictcompare", "4");

        expect(state.ole_objects.size() == 3U,
               "NEWOBJECT Init-args script should register native NEWOBJECT, native CREATEOBJECT, and COM NEWOBJECT objects");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "CtorWidget",
                   "native NEWOBJECT Init-args script should preserve constructor-target class identity");
            const auto ctor_count = state.ole_objects[0].properties.find("ncount");
            expect(ctor_count != state.ole_objects[0].properties.end(),
                   "native NEWOBJECT Init-args script should persist Init-updated numeric state");
            if (ctor_count != state.ole_objects[0].properties.end())
            {
                expect(copperfin::runtime::format_value(ctor_count->second) == "7",
                       "native NEWOBJECT Init-args script should apply trailing NEWOBJECT arguments inside Init");
            }

            expect(state.ole_objects[1].prog_id == "CreateWidget",
                   "CREATEOBJECT constructor-argument behavior should remain stable while NEWOBJECT gains parity");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT library activation should remain stable while native NEWOBJECT gains Init arguments");
            expect(state.ole_objects[2].source == "vbscript.dll",
                   "COM NEWOBJECT library source should remain stable while native NEWOBJECT gains Init arguments");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_native_object_method_and_init_preserve_by_reference_argument_updates()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_object_byref";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_object_byref.prg";
        write_text(
            main_path,
            "nCtorSeed = 7\n"
            "nMethodSeed = 10\n"
            "oCtor = CREATEOBJECT('CtorWidget', @nCtorSeed)\n"
            "oMethod = CREATEOBJECT('MethodWidget')\n"
            "nMethodResult = oMethod.Bump(@nMethodSeed)\n"
            "nCtorAfter = nCtorSeed\n"
            "nCtorStored = oCtor.nValue\n"
            "nMethodAfter = nMethodSeed\n"
            "nMethodStored = oMethod.nValue\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 5)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS CtorWidget AS Custom\n"
            "    nValue = 0\n"
            "    PROCEDURE Init\n"
            "        LPARAMETERS tnSeed\n"
            "        tnSeed = tnSeed + 3\n"
            "        THIS.nValue = tnSeed\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS MethodWidget AS Custom\n"
            "    nValue = 0\n"
            "    FUNCTION Bump\n"
            "        LPARAMETERS tnSeed\n"
            "        tnSeed = tnSeed + 5\n"
            "        THIS.nValue = tnSeed\n"
            "        RETURN tnSeed\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native object by-reference script should complete: ") + state.message +
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

        check("nctorafter", "10");
        check("nctorstored", "10");
        check("nmethodresult", "15");
        check("nmethodafter", "15");
        check("nmethodstored", "15");
        check("ldictset", "true");
        check("ndictcompare", "5");

        expect(state.ole_objects.size() == 3U,
               "native object by-reference script should register ctor, method, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto ctor_value = state.ole_objects[0].properties.find("nvalue");
            expect(ctor_value != state.ole_objects[0].properties.end(),
                   "automatic Init by-reference updates should persist onto the constructed object");
            if (ctor_value != state.ole_objects[0].properties.end())
            {
                expect(copperfin::runtime::format_value(ctor_value->second) == "10",
                       "automatic Init by-reference updates should persist updated constructor values");
            }

            const auto method_value = state.ole_objects[1].properties.find("nvalue");
            expect(method_value != state.ole_objects[1].properties.end(),
                   "native object method by-reference updates should persist onto the instance");
            if (method_value != state.ole_objects[1].properties.end())
            {
                expect(copperfin::runtime::format_value(method_value->second) == "15",
                       "native object method by-reference updates should persist updated method values");
            }

            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM object activation should remain stable while native by-reference parity lands");
        }

        fs::remove_all(temp_root, ignored);
    }

}
