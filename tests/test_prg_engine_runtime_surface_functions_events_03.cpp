#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_same_prg_native_bindevent_raiseevent_and_unbindevents_dispatch_same_session_handlers()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_bindevent";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_bindevent.prg";
        write_text(
            main_path,
            "cSequence = ''\n"
            "nBeforeCalls = 0\n"
            "nRoutineCalls = 0\n"
            "nSourceCalls = 0\n"
            "nAfterCalls = 0\n"
            "nAfterSourceCalls = 0\n"
            "nNoSimpleCalls = 0\n"
            "nNoSimpleSourceCalls = 0\n"
            "oSource = CREATEOBJECT('SourceThing')\n"
            "oHandler = CREATEOBJECT('HandlerThing')\n"
            "oPlain = CREATEOBJECT('Empty')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "oPlain.Extra = 'plain'\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 29)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "nBind1 = BINDEVENT(oSource, 'Ping', oHandler, 'HandleBefore', 0)\n"
            "nBind2 = BINDEVENT(oSource, 'Ping', 'HandlePing', 1)\n"
            "nBind3 = BINDEVENT(oSource, 'AfterPing', oHandler, 'HandleAfter', 1)\n"
            "nBind4 = BINDEVENT(oSource, 'NoSimple', oHandler, 'HandleNoSimple', 3)\n"
            "cDirectPing = oSource.Ping(41)\n"
            "lRaisedAfter = RAISEEVENT(oSource, 'AfterPing', 7)\n"
            "cDirectNoSimple = oSource.NoSimple(5)\n"
            "lRaisedNoSimple = RAISEEVENT(oSource, 'NoSimple', 6)\n"
            "nUnbindSpecific = UNBINDEVENTS(oSource, 'Ping', oHandler, 'HandleBefore')\n"
            "lRaisedPingAfterUnbind = RAISEEVENT(oSource, 'Ping', 9)\n"
            "nUnbindObject = UNBINDEVENTS(oHandler)\n"
            "lRaisedAfterAfterObjectUnbind = RAISEEVENT(oSource, 'AfterPing', 8)\n"
            "cPlain = oPlain.Extra\n"
            "RETURN\n"
            "PROCEDURE HandlePing\n"
            "    LPARAMETERS tnValue\n"
            "    cSequence = cSequence + '[routine:' + TRANSFORM(tnValue) + ']'\n"
            "    nRoutineCalls = nRoutineCalls + 1\n"
            "    RETURN\n"
            "ENDPROC\n"
            "DEFINE CLASS SourceThing AS Custom\n"
            "    FUNCTION Ping\n"
            "        LPARAMETERS tnValue\n"
            "        cSequence = cSequence + '[source:' + TRANSFORM(tnValue) + ']'\n"
            "        nSourceCalls = nSourceCalls + 1\n"
            "        RETURN 'ping:' + TRANSFORM(tnValue)\n"
            "    ENDFUNC\n"
            "    FUNCTION AfterPing\n"
            "        LPARAMETERS tnValue\n"
            "        cSequence = cSequence + '[aftersource:' + TRANSFORM(tnValue) + ']'\n"
            "        nAfterSourceCalls = nAfterSourceCalls + 1\n"
            "        RETURN 'after:' + TRANSFORM(tnValue)\n"
            "    ENDFUNC\n"
            "    FUNCTION NoSimple\n"
            "        LPARAMETERS tnValue\n"
            "        cSequence = cSequence + '[nosimple:' + TRANSFORM(tnValue) + ']'\n"
            "        nNoSimpleSourceCalls = nNoSimpleSourceCalls + 1\n"
            "        RETURN 'nosimple:' + TRANSFORM(tnValue)\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS HandlerThing AS Custom\n"
            "    FUNCTION HandleBefore\n"
            "        LPARAMETERS tnValue\n"
            "        cSequence = cSequence + '[before:' + TRANSFORM(tnValue) + ']'\n"
            "        nBeforeCalls = nBeforeCalls + 1\n"
            "        RETURN\n"
            "    ENDFUNC\n"
            "    FUNCTION HandleAfter\n"
            "        LPARAMETERS tnValue\n"
            "        cSequence = cSequence + '[after:' + TRANSFORM(tnValue) + ']'\n"
            "        nAfterCalls = nAfterCalls + 1\n"
            "        RETURN\n"
            "    ENDFUNC\n"
            "    FUNCTION HandleNoSimple\n"
            "        LPARAMETERS tnValue\n"
            "        cSequence = cSequence + '[nosimplehandler:' + TRANSFORM(tnValue) + ']'\n"
            "        nNoSimpleCalls = nNoSimpleCalls + 1\n"
            "        RETURN\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native BINDEVENT/RAISEEVENT script should complete: ") + state.message +
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

        check("nbind1", "1");
        check("nbind2", "2");
        check("nbind3", "1");
        check("nbind4", "1");
        check("cdirectping", "ping:41");
        check("lraisedafter", "true");
        check("cdirectnosimple", "nosimple:5");
        check("lraisednosimple", "true");
        check("nunbindspecific", "1");
        check("lraisedpingafterunbind", "true");
        check("nunbindobject", "2");
        check("lraisedafterafterobjectunbind", "true");
        check("nbeforecalls", "1");
        check("nroutinecalls", "2");
        check("nsourcecalls", "2");
        check("naftercalls", "1");
        check("naftersourcecalls", "2");
        check("nnosimplecalls", "1");
        check("nnosimplesourcecalls", "2");
        check("csequence",
              "[before:41][source:41][routine:41][aftersource:7][after:7][nosimple:5][nosimple:6][nosimplehandler:6][source:9][routine:9][aftersource:8]");
        check("cplain", "plain");
        check("ldictset", "true");
        check("ndictcompare", "29");

        expect(state.ole_objects.size() == 4U,
               "native BINDEVENT/RAISEEVENT script should register source, handler, plain, and COM objects");
        if (state.ole_objects.size() == 4U)
        {
            expect(state.ole_objects[0].prog_id == "SourceThing",
                   "native BINDEVENT/RAISEEVENT should preserve the source class identity");
            expect(state.ole_objects[1].prog_id == "HandlerThing",
                   "native BINDEVENT/RAISEEVENT should preserve the handler class identity");
            expect(state.ole_objects[2].prog_id == "Empty",
                   "plain CREATEOBJECT should remain stable while native event binding lands");
            expect(state.ole_objects[3].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native event binding lands");
        }

        const bool has_bind_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.event.bind";
        });
        const bool has_raise_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.event.raise";
        });
        const bool has_delegate_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.event.delegate";
        });
        expect(has_bind_event,
               "native BINDEVENT should emit a runtime bind event");
        expect(has_raise_event,
               "native RAISEEVENT should emit a runtime raise event");
        expect(has_delegate_event,
               "native event dispatch should emit delegate invocation events");

        fs::remove_all(temp_root, ignored);
    }


    // #6547: BINDEVENT nFlags timing must match VFP9 (VFP 9 help table and a
    // real VFP 9.0 SP2 probe): bit 0 clear = delegate BEFORE the event code,
    // bit 0 set = delegate AFTER it; bit 1 set = a simple method call does not
    // trigger the delegate (RAISEEVENT still does). #3688 had this inverted.
    void test_bindevent_flag_timing_matches_vfp9()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_bindevent_flag_timing_6547";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);
        const auto global_text = [](const auto &state, const std::string &name) -> std::string {
            const auto found = state.globals.find(name);
            return found == state.globals.end() ? std::string("<missing>") : copperfin::runtime::format_value(found->second);
        };
        const fs::path main_path = temp_root / "bindevent_flag_timing.prg";
        write_text(
            main_path,
            "PUBLIC cLog\n"
            "oHandler = CREATEOBJECT('HandlerThing')\n"
            "DIMENSION aSimple[4], aRaised[4]\n"
            "FOR nFlags = 0 TO 3\n"
            "    oSource = CREATEOBJECT('SourceThing')\n"
            "    BINDEVENT(oSource, 'Ping', oHandler, 'LogHandler', nFlags)\n"
            "    cLog = ''\n"
            "    oSource.Ping()\n"
            "    aSimple[nFlags + 1] = cLog\n"
            "    cLog = ''\n"
            "    RAISEEVENT(oSource, 'Ping')\n"
            "    aRaised[nFlags + 1] = cLog\n"
            "ENDFOR\n"
            "cSimple0 = aSimple[1]\n"
            "cSimple1 = aSimple[2]\n"
            "cSimple2 = aSimple[3]\n"
            "cSimple3 = aSimple[4]\n"
            "cRaised0 = aRaised[1]\n"
            "cRaised1 = aRaised[2]\n"
            "cRaised2 = aRaised[3]\n"
            "cRaised3 = aRaised[4]\n"
            "RETURN\n"
            "DEFINE CLASS SourceThing AS Custom\n"
            "    PROCEDURE Ping\n"
            "        cLog = cLog + 'body;'\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS HandlerThing AS Custom\n"
            "    PROCEDURE LogHandler\n"
            "        cLog = cLog + 'handler;'\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");
        auto session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string(), false));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "#6547: flag timing script should complete: " + state.message);
        const std::vector<std::pair<std::string, std::string>> expected = {
            {"csimple0", "handler;body;"},   // 0: delegate before
            {"csimple1", "body;handler;"},   // 1: delegate after
            {"csimple2", "body;"},           // 2: simple call does not trigger
            {"csimple3", "body;"},           // 3: simple call does not trigger
            {"craised0", "handler;body;"},
            {"craised1", "body;handler;"},
            {"craised2", "handler;body;"},   // 2: RAISEEVENT triggers, delegate before
            {"craised3", "body;handler;"},   // 3: RAISEEVENT triggers, delegate after
        };
        for (const auto &[name, value] : expected)
        {
            expect(global_text(state, name) == value,
                   "#6547: " + name + " expected '" + value + "' got '" + global_text(state, name) + "'");
        }
        fs::remove_all(temp_root, ignored);
    }

}
