#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_same_prg_native_bare_helper_calls_resolve_to_current_instance_before_top_level_routines()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_helper_calls";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_helper_calls.prg";
        write_text(
            main_path,
            "nSeed = 4\n"
            "oCreate = CREATEOBJECT('ChildWidget', @nSeed)\n"
            "oPlain = CREATEOBJECT('Empty')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "oPlain.Extra = 'plain'\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 8)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "cDescribe = oCreate.Describe('prefix')\n"
            "nSeedAfter = nSeed\n"
            "cCaption = oCreate.Caption\n"
            "nStored = oCreate.nValue\n"
            "lInitRan = oCreate.lInitRan\n"
            "cPlain = oPlain.Extra\n"
            "RETURN\n"
            "FUNCTION BuildCaption\n"
            "    LPARAMETERS tcPrefix\n"
            "    RETURN 'top-level-' + tcPrefix\n"
            "ENDFUNC\n"
            "DEFINE CLASS ParentWidget AS Custom\n"
            "    Caption = 'Parent'\n"
            "    nValue = 0\n"
            "    lInitRan = .F.\n"
            "    PROCEDURE Init\n"
            "        LPARAMETERS tnSeed\n"
            "        FinishInit(@tnSeed)\n"
            "        RETURN THIS.Caption\n"
            "    ENDPROC\n"
            "    PROCEDURE FinishInit\n"
            "        LPARAMETERS tnSeed\n"
            "        tnSeed = tnSeed + 3\n"
            "        THIS.nValue = tnSeed\n"
            "        THIS.Caption = THIS.Caption + '-Init'\n"
            "        THIS.lInitRan = .T.\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "    FUNCTION Describe\n"
            "        LPARAMETERS tcPrefix\n"
            "        RETURN BuildCaption(tcPrefix)\n"
            "    ENDFUNC\n"
            "    FUNCTION BuildCaption\n"
            "        LPARAMETERS tcPrefix\n"
            "        RETURN tcPrefix + ':' + THIS.Caption\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ChildWidget AS ParentWidget\n"
            "    Caption = 'Child'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native bare helper-call script should complete: ") + state.message +
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

        check("cdescribe", "prefix:Child-Init");
        check("nseedafter", "7");
        check("ccaption", "Child-Init");
        check("nstored", "7");
        check("linitran", "true");
        check("cplain", "plain");
        check("ldictset", "true");
        check("ndictcompare", "8");

        expect(state.ole_objects.size() == 3U,
               "native bare helper-call script should register native, plain, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &native_object = state.ole_objects[0];
            expect(native_object.prog_id == "ChildWidget",
                   "native bare helper-call resolution should preserve child class identity");
            const auto caption = native_object.properties.find("caption");
            const auto value = native_object.properties.find("nvalue");
            const auto init_ran = native_object.properties.find("linitran");
            if (caption != native_object.properties.end())
            {
                expect(copperfin::runtime::format_value(caption->second) == "Child-Init",
                       "native bare helper-call resolution should preserve helper-updated caption state");
            }
            else
            {
                expect(false, "native bare helper-call resolution should materialize helper-updated caption state");
            }
            if (value != native_object.properties.end())
            {
                expect(copperfin::runtime::format_value(value->second) == "7",
                       "native bare helper-call resolution should preserve helper by-reference updates from Init");
            }
            else
            {
                expect(false, "native bare helper-call resolution should materialize helper-updated numeric state");
            }
            if (init_ran != native_object.properties.end())
            {
                expect(copperfin::runtime::format_value(init_ran->second) == "true",
                       "native bare helper-call resolution should preserve helper-set Init flags");
            }
            else
            {
                expect(false, "native bare helper-call resolution should materialize helper-set Init flags");
            }

            expect(state.ole_objects[1].prog_id == "Empty",
                   "plain CREATEOBJECT should remain stable while native helper-method resolution lands");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native helper-method resolution lands");
        }

        const bool has_helper_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   (event.detail == "ParentWidget.FinishInit" ||
                    event.detail == "ParentWidget.BuildCaption");
        });
        expect(has_helper_invoke_event,
               "native bare helper-call resolution should emit helper-method invoke events");

        fs::remove_all(temp_root, ignored);
    }

    void test_inherited_external_prg_base_methods_resolve_bare_helper_calls_against_defining_library()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_external_base_helper_calls";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path library_path = temp_root / "widgetlib.prg";
        write_text(
            library_path,
            "DEFINE CLASS ParentWidget AS Custom\n"
            "    Caption = 'Parent'\n"
            "    nValue = 0\n"
            "    lInitRan = .F.\n"
            "    PROCEDURE Init\n"
            "        LPARAMETERS tnSeed\n"
            "        FinishInit(@tnSeed)\n"
            "        RETURN THIS.Caption\n"
            "    ENDPROC\n"
            "    PROCEDURE FinishInit\n"
            "        LPARAMETERS tnSeed\n"
            "        tnSeed = tnSeed + 3\n"
            "        THIS.nValue = tnSeed\n"
            "        THIS.Caption = THIS.Caption + '-Init'\n"
            "        THIS.lInitRan = .T.\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "    FUNCTION Describe\n"
            "        LPARAMETERS tcPrefix\n"
            "        RETURN BuildCaption(tcPrefix)\n"
            "    ENDFUNC\n"
            "    FUNCTION BuildCaption\n"
            "        LPARAMETERS tcPrefix\n"
            "        RETURN tcPrefix + ':' + THIS.Caption\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_base_helper_calls.prg";
        write_text(
            main_path,
            "nSeed = 4\n"
            "oCreate = CREATEOBJECT('ChildWidget', @nSeed)\n"
            "oPlain = CREATEOBJECT('Empty')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "oPlain.Extra = 'plain'\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 19)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "cDescribe = oCreate.Describe('prefix')\n"
            "nSeedAfter = nSeed\n"
            "cCaption = oCreate.Caption\n"
            "nStored = oCreate.nValue\n"
            "lInitRan = oCreate.lInitRan\n"
            "cPlain = oPlain.Extra\n"
            "RETURN\n"
            "FUNCTION BuildCaption\n"
            "    LPARAMETERS tcPrefix\n"
            "    RETURN 'top-level-' + tcPrefix\n"
            "ENDFUNC\n"
            "DEFINE CLASS ChildWidget AS ParentWidget OF widgetlib.prg\n"
            "    Caption = 'Child'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external-base bare helper-call script should complete: ") + state.message +
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

        check("cdescribe", "prefix:Child-Init");
        check("nseedafter", "7");
        check("ccaption", "Child-Init");
        check("nstored", "7");
        check("linitran", "true");
        check("cplain", "plain");
        check("ldictset", "true");
        check("ndictcompare", "19");

        expect(state.ole_objects.size() == 3U,
               "external-base bare helper-call script should register native, plain, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &native_object = state.ole_objects[0];
            expect(native_object.prog_id == "ChildWidget",
                   "external-base bare helper-call resolution should preserve child class identity");
            const auto caption = native_object.properties.find("caption");
            const auto value = native_object.properties.find("nvalue");
            const auto init_ran = native_object.properties.find("linitran");
            if (caption != native_object.properties.end())
            {
                expect(copperfin::runtime::format_value(caption->second) == "Child-Init",
                       "external-base bare helper-call resolution should preserve helper-updated caption state");
            }
            else
            {
                expect(false, "external-base bare helper-call resolution should materialize helper-updated caption state");
            }
            if (value != native_object.properties.end())
            {
                expect(copperfin::runtime::format_value(value->second) == "7",
                       "external-base bare helper-call resolution should preserve helper by-reference updates from Init");
            }
            else
            {
                expect(false, "external-base bare helper-call resolution should materialize helper-updated numeric state");
            }
            if (init_ran != native_object.properties.end())
            {
                expect(copperfin::runtime::format_value(init_ran->second) == "true",
                       "external-base bare helper-call resolution should preserve helper-set Init flags");
            }
            else
            {
                expect(false, "external-base bare helper-call resolution should materialize helper-set Init flags");
            }

            expect(state.ole_objects[1].prog_id == "Empty",
                   "plain CREATEOBJECT should remain stable while external-base helper-method resolution lands");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external-base helper-method resolution lands");
        }

        const bool has_helper_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   (event.detail == "ParentWidget.FinishInit" ||
                    event.detail == "ParentWidget.BuildCaption");
        });
        expect(has_helper_invoke_event,
               "external-base bare helper-call resolution should emit helper-method invoke events");

        fs::remove_all(temp_root, ignored);
    }

    void test_same_prg_inherited_methods_dispatch_self_calls_to_most_derived_overrides()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_inherited_self_dispatch";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "inherited_self_dispatch.prg";
        write_text(
            main_path,
            "oChild = CREATEOBJECT('ChildWidget')\n"
            "cLog = oChild.cLog\n"
            "RETURN\n"
            "DEFINE CLASS ParentWidget AS Custom\n"
            "    cLog = ''\n"
            "    PROCEDURE Init\n"
            "        THIS.RunViaThis()\n"
            "        RunBare()\n"
            "    ENDPROC\n"
            "    PROCEDURE RunViaThis\n"
            "        THIS.cLog = THIS.cLog + '[ParentThis]'\n"
            "    ENDPROC\n"
            "    PROCEDURE RunBare\n"
            "        THIS.cLog = THIS.cLog + '[ParentBare]'\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ChildWidget AS ParentWidget\n"
            "    PROCEDURE RunViaThis\n"
            "        THIS.cLog = THIS.cLog + '[ChildThis]'\n"
            "    ENDPROC\n"
            "    PROCEDURE RunBare\n"
            "        THIS.cLog = THIS.cLog + '[ChildBare]'\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("same-PRG inherited self-dispatch script should complete: ") + state.message +
                   " @line=" + std::to_string(state.location.line));

        const auto log_found = state.globals.find("clog");
        expect(log_found != state.globals.end(),
               "same-PRG inherited self-dispatch should materialize the log result");
        if (log_found != state.globals.end())
        {
            expect(copperfin::runtime::format_value(log_found->second) == "[ChildThis][ChildBare]",
                   "same-PRG inherited self-dispatch should route THIS.Method() and bare self-calls to the child override");
        }

        const bool has_child_invoke_events = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   (event.detail == "ChildWidget.RunViaThis" ||
                    event.detail == "ChildWidget.RunBare");
        });
        expect(has_child_invoke_events,
               "same-PRG inherited self-dispatch should emit invoke events for the most-derived overrides");

        fs::remove_all(temp_root, ignored);
    }

    void test_external_base_inherited_methods_dispatch_self_calls_to_most_derived_overrides()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_external_inherited_self_dispatch";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path library_path = temp_root / "widgetlib.prg";
        write_text(
            library_path,
            "DEFINE CLASS ParentWidget AS Custom\n"
            "    cLog = ''\n"
            "    PROCEDURE Init\n"
            "        THIS.RunViaThis()\n"
            "        RunBare()\n"
            "    ENDPROC\n"
            "    PROCEDURE RunViaThis\n"
            "        THIS.cLog = THIS.cLog + '[ParentThis]'\n"
            "    ENDPROC\n"
            "    PROCEDURE RunBare\n"
            "        THIS.cLog = THIS.cLog + '[ParentBare]'\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_inherited_self_dispatch.prg";
        write_text(
            main_path,
            "oChild = CREATEOBJECT('ChildWidget')\n"
            "cLog = oChild.cLog\n"
            "RETURN\n"
            "DEFINE CLASS ChildWidget AS ParentWidget OF widgetlib.prg\n"
            "    PROCEDURE RunViaThis\n"
            "        THIS.cLog = THIS.cLog + '[ChildThis]'\n"
            "    ENDPROC\n"
            "    PROCEDURE RunBare\n"
            "        THIS.cLog = THIS.cLog + '[ChildBare]'\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external-base inherited self-dispatch script should complete: ") + state.message +
                   " @line=" + std::to_string(state.location.line));

        const auto log_found = state.globals.find("clog");
        expect(log_found != state.globals.end(),
               "external-base inherited self-dispatch should materialize the log result");
        if (log_found != state.globals.end())
        {
            expect(copperfin::runtime::format_value(log_found->second) == "[ChildThis][ChildBare]",
                   "external-base inherited self-dispatch should route THIS.Method() and bare self-calls to the child override");
        }

        const bool has_child_invoke_events = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   (event.detail == "ChildWidget.RunViaThis" ||
                    event.detail == "ChildWidget.RunBare");
        });
        expect(has_child_invoke_events,
               "external-base inherited self-dispatch should emit invoke events for the most-derived overrides");

        fs::remove_all(temp_root, ignored);
    }

    void test_external_prg_base_methods_support_dodefault_dispatch()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_external_base_dodefault";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path library_path = temp_root / "widgetlib.prg";
        write_text(
            library_path,
            "DEFINE CLASS ParentWidget AS Custom\n"
            "    Caption = 'Parent'\n"
            "    nValue = 0\n"
            "    lParentInit = .F.\n"
            "    PROCEDURE Init\n"
            "        LPARAMETERS tnSeed\n"
            "        tnSeed = tnSeed + 2\n"
            "        THIS.nValue = tnSeed\n"
            "        THIS.Caption = THIS.Caption + '-P'\n"
            "        THIS.lParentInit = .T.\n"
            "        RETURN THIS.Caption\n"
            "    ENDPROC\n"
            "    FUNCTION Describe\n"
            "        LPARAMETERS tcPrefix\n"
            "        RETURN tcPrefix + ':' + THIS.Caption\n"
            "    ENDFUNC\n"
            "    FUNCTION Who\n"
            "        RETURN 'Parent'\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_base_dodefault.prg";
        write_text(
            main_path,
            "nSeed = 5\n"
            "oCreate = CREATEOBJECT('ChildWidget', @nSeed)\n"
            "oPlain = CREATEOBJECT('Empty')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "oPlain.Extra = 'plain'\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 20)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "cDescribe = oCreate.Describe('prefix')\n"
            "cWho = oCreate.Who()\n"
            "nSeedAfter = nSeed\n"
            "cCaption = oCreate.Caption\n"
            "nStored = oCreate.nValue\n"
            "lParentInit = oCreate.lParentInit\n"
            "lChildInit = oCreate.lChildInit\n"
            "cPlain = oPlain.Extra\n"
            "RETURN\n"
            "DEFINE CLASS ChildWidget AS ParentWidget OF widgetlib.prg\n"
            "    Caption = 'Child'\n"
            "    lChildInit = .F.\n"
            "    PROCEDURE Init\n"
            "        LPARAMETERS tnSeed\n"
            "        LOCAL lcBaseCaption\n"
            "        lcBaseCaption = DODEFAULT(@tnSeed)\n"
            "        THIS.Caption = lcBaseCaption + '-C'\n"
            "        THIS.lChildInit = .T.\n"
            "        RETURN THIS.Caption\n"
            "    ENDPROC\n"
            "    FUNCTION Describe\n"
            "        LPARAMETERS tcPrefix\n"
            "        RETURN DODEFAULT(tcPrefix) + ':Child'\n"
            "    ENDFUNC\n"
            "    FUNCTION Who\n"
            "        RETURN DODEFAULT() + '+Child'\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external-base DODEFAULT script should complete: ") + state.message +
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

        check("cdescribe", "prefix:Child-P-C:Child");
        check("cwho", "Parent+Child");
        check("nseedafter", "7");
        check("ccaption", "Child-P-C");
        check("nstored", "7");
        check("lparentinit", "true");
        check("lchildinit", "true");
        check("cplain", "plain");
        check("ldictset", "true");
        check("ndictcompare", "20");

        expect(state.ole_objects.size() == 3U,
               "external-base DODEFAULT script should register native, plain, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &native_object = state.ole_objects[0];
            expect(native_object.prog_id == "ChildWidget",
                   "external-base DODEFAULT should preserve child class identity");
            const auto caption = native_object.properties.find("caption");
            const auto value = native_object.properties.find("nvalue");
            const auto parent_init = native_object.properties.find("lparentinit");
            const auto child_init = native_object.properties.find("lchildinit");
            expect(caption != native_object.properties.end(),
                   "external-base DODEFAULT should preserve child/base Init-updated caption state");
            expect(value != native_object.properties.end(),
                   "external-base DODEFAULT should preserve by-reference Init-updated numeric state");
            expect(parent_init != native_object.properties.end(),
                   "external-base DODEFAULT should preserve parent Init state");
            expect(child_init != native_object.properties.end(),
                   "external-base DODEFAULT should preserve child Init state");
            if (caption != native_object.properties.end())
            {
                expect(copperfin::runtime::format_value(caption->second) == "Child-P-C",
                       "external-base DODEFAULT should compose child Init logic after the external base Init");
            }
            if (value != native_object.properties.end())
            {
                expect(copperfin::runtime::format_value(value->second) == "7",
                       "external-base DODEFAULT should preserve external base Init by-reference write-back results");
            }
            if (parent_init != native_object.properties.end())
            {
                expect(copperfin::runtime::format_value(parent_init->second) == "true",
                       "external-base DODEFAULT should run the external parent Init through the base-call path");
            }
            if (child_init != native_object.properties.end())
            {
                expect(copperfin::runtime::format_value(child_init->second) == "true",
                       "external-base DODEFAULT should continue child Init logic after the base-call path");
            }

            expect(state.ole_objects[1].prog_id == "Empty",
                   "plain CREATEOBJECT should remain stable while external-base DODEFAULT lands");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external-base DODEFAULT lands");
        }

        const bool has_base_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.baseinvoke" &&
                   (event.detail == "ParentWidget.Init" ||
                    event.detail == "ParentWidget.Describe" ||
                    event.detail == "ParentWidget.Who");
        });
        expect(has_base_invoke_event,
               "external-base DODEFAULT should emit a base-invoke runtime event");

        fs::remove_all(temp_root, ignored);
    }

    void test_external_prg_base_methods_reflect_through_getpem_pemstatus_and_amembers()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_external_base_method_reflection";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path library_path = temp_root / "widgetlib.prg";
        write_text(
            library_path,
            "DEFINE CLASS ParentWidget AS Custom\n"
            "    Caption = 'Parent'\n"
            "    FUNCTION Describe\n"
            "        LPARAMETERS tcPrefix\n"
            "        RETURN tcPrefix + ':' + THIS.Caption\n"
            "    ENDFUNC\n"
            "    FUNCTION Who\n"
            "        RETURN 'Parent'\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_base_method_reflection.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildWidget')\n"
            "oPlain = CREATEOBJECT('Empty')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "oPlain.Extra = 'plain'\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 23)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "lHasDescribe = GETPEM(oCreate, 'Describe')\n"
            "lHasWho = PEMSTATUS(oCreate, 'Who', 1)\n"
            "lHasPing = PEMSTATUS(oCreate, 'Ping', 1)\n"
            "nMembersMethods = AMEMBERS(aMembersMethods, oCreate, 2)\n"
            "cMethod1 = aMembersMethods[1]\n"
            "cMethod2 = aMembersMethods[2]\n"
            "cMethod3 = aMembersMethods[3]\n"
            "cDescribe = oCreate.Describe('prefix')\n"
            "cWho = oCreate.Who()\n"
            "cPing = oCreate.Ping()\n"
            "cPlain = oPlain.Extra\n"
            "RETURN\n"
            "DEFINE CLASS ChildWidget AS ParentWidget OF widgetlib.prg\n"
            "    Caption = 'Child'\n"
            "    FUNCTION Ping\n"
            "        RETURN 'Child'\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external-base method reflection script should complete: ") + state.message +
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

        check("lhasdescribe", "true");
        check("lhaswho", "true");
        check("lhasping", "true");
        check("nmembersmethods", "10");
        check("cmethod1", "DESCRIBE");
        check("cmethod2", "PING");
        check("cmethod3", "READEXPRESSION");
        check("cdescribe", "prefix:Child");
        check("cwho", "Parent");
        check("cping", "Child");
        check("cplain", "plain");
        check("ldictset", "true");
        check("ndictcompare", "23");

        expect(state.ole_objects.size() == 3U,
               "external-base method reflection should register native, plain, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &native_object = state.ole_objects[0];
            expect(native_object.prog_id == "ChildWidget",
                   "external-base method reflection should preserve child class identity");
            expect(native_object.source == main_path.string(),
                   "external-base method reflection should preserve the derived class provenance");
            expect(std::find(native_object.methods.begin(), native_object.methods.end(), "Describe") != native_object.methods.end(),
                   "external-base method reflection should materialize inherited methods in runtime object metadata");
            expect(std::find(native_object.methods.begin(), native_object.methods.end(), "Who") != native_object.methods.end(),
                   "external-base method reflection should preserve external-base methods in runtime object metadata");
            expect(std::find(native_object.methods.begin(), native_object.methods.end(), "Ping") != native_object.methods.end(),
                   "external-base method reflection should preserve derived methods in runtime object metadata");
            expect(state.ole_objects[1].prog_id == "Empty",
                   "plain CREATEOBJECT should remain stable while external-base method reflection lands");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external-base method reflection lands");
        }

        const bool has_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   (event.detail == "ParentWidget.Describe" ||
                    event.detail == "ParentWidget.Who" ||
                    event.detail == "ChildWidget.Ping");
        });
        expect(has_invoke_event,
               "external-base method reflection should emit inherited and derived method invoke events");

        fs::remove_all(temp_root, ignored);
    }

}
