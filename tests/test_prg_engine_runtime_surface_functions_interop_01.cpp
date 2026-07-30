#include "test_prg_engine_runtime_surface_functions_support.h"

#include <vector>

namespace copperfin::runtime_surface_tests
{
    void test_same_prg_native_raiseevent_handler_fault_does_not_disable_future_dispatch()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_raiseevent_fault_recovery";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_raiseevent_fault_recovery.prg";
        write_text(
            main_path,
            "cSequence = ''\n"
            "nHandlerCalls = 0\n"
            "nSourceCalls = 0\n"
            "oSource = CREATEOBJECT('SourceThing')\n"
            "nBind = BINDEVENT(oSource, 'Ping', 'HandlePing')\n"
            "lCaught = .F.\n"
            "TRY\n"
            "    lFirstRaised = RAISEEVENT(oSource, 'Ping', 1)\n"
            "CATCH TO oErr\n"
            "    lCaught = .T.\n"
            "    cFirstError = oErr.Message\n"
            "ENDTRY\n"
            "lSecondRaised = RAISEEVENT(oSource, 'Ping', 2)\n"
            "RETURN\n"
            "PROCEDURE HandlePing\n"
            "    LPARAMETERS tnValue\n"
            "    nHandlerCalls = nHandlerCalls + 1\n"
            "    cSequence = cSequence + '[handler:' + TRANSFORM(tnValue) + ']'\n"
            "    IF nHandlerCalls = 1\n"
            "        1 / 0\n"
            "    ENDIF\n"
            "    RETURN\n"
            "ENDPROC\n"
            "DEFINE CLASS SourceThing AS Custom\n"
            "    FUNCTION Ping\n"
            "        LPARAMETERS tnValue\n"
            "        nSourceCalls = nSourceCalls + 1\n"
            "        cSequence = cSequence + '[source:' + TRANSFORM(tnValue) + ']'\n"
            "        RETURN 'ping:' + TRANSFORM(tnValue)\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native RAISEEVENT fault-recovery script should complete: ") + state.message +
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

        check("nbind", "1");
        check("lcaught", "true");
        check("lsecondraised", "true");
        check("nhandlercalls", "2");
        check("nsourcecalls", "2");
        check("csequence", "[source:1][handler:1][source:2][handler:2]");

        const auto first_error = state.globals.find("cfirsterror");
        expect(first_error != state.globals.end(),
               "faulting RAISEEVENT handler should populate the CATCH error message");

        fs::remove_all(temp_root, ignored);
    }

    void test_same_prg_native_bindevent_method_handler_fault_does_not_disable_future_delegate_dispatch()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_method_bindevent_fault_recovery";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_method_bindevent_fault_recovery.prg";
        write_text(
            main_path,
            "cSequence = ''\n"
            "nHandlerCalls = 0\n"
            "nSourceCalls = 0\n"
            "oSource = CREATEOBJECT('SourceThing')\n"
            "nBind = BINDEVENT(oSource, 'Ping', 'HandlePing', 1)\n"
            "lCaught = .F.\n"
            "TRY\n"
            "    cFirstDirect = oSource.Ping(1)\n"
            "CATCH TO oErr\n"
            "    lCaught = .T.\n"
            "    cFirstError = oErr.Message\n"
            "ENDTRY\n"
            "cSecondDirect = oSource.Ping(2)\n"
            "RETURN\n"
            "PROCEDURE HandlePing\n"
            "    LPARAMETERS tnValue\n"
            "    nHandlerCalls = nHandlerCalls + 1\n"
            "    cSequence = cSequence + '[handler:' + TRANSFORM(tnValue) + ']'\n"
            "    IF nHandlerCalls = 1\n"
            "        1 / 0\n"
            "    ENDIF\n"
            "    RETURN\n"
            "ENDPROC\n"
            "DEFINE CLASS SourceThing AS Custom\n"
            "    FUNCTION Ping\n"
            "        LPARAMETERS tnValue\n"
            "        nSourceCalls = nSourceCalls + 1\n"
            "        cSequence = cSequence + '[source:' + TRANSFORM(tnValue) + ']'\n"
            "        RETURN 'ping:' + TRANSFORM(tnValue)\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native method BINDEVENT fault-recovery script should complete: ") + state.message +
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

        check("nbind", "1");
        check("lcaught", "true");
        check("cseconddirect", "ping:2");
        check("nhandlercalls", "2");
        check("nsourcecalls", "1");
        check("csequence", "[handler:1][handler:2][source:2]");

        const auto first_error = state.globals.find("cfirsterror");
        expect(first_error != state.globals.end(),
               "faulting method delegate should populate the CATCH error message");

        fs::remove_all(temp_root, ignored);
    }

    void test_same_prg_native_aevents_enumerates_same_session_bindings_without_clobbering_zero_row_targets()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_aevents";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_aevents.prg";
        write_text(
            main_path,
            "oSource = CREATEOBJECT('SourceThing')\n"
            "oHandler = CREATEOBJECT('HandlerThing')\n"
            "oPlain = CREATEOBJECT('Empty')\n"
            "nBind1 = BINDEVENT(oSource, 'Ping', oHandler, 'HandleBefore')\n"
            "nBind2 = BINDEVENT(oSource, 'Ping', 'HandlePing')\n"
            "nBind3 = BINDEVENT(oSource, 'AfterPing', oHandler, 'HandleAfter', 1)\n"
            "nBind4 = BINDEVENT(oSource, 'NoSimple', oHandler, 'HandleNoSimple', 2)\n"
            "nSourceRows = AEVENTS(aSourceEvents, oSource)\n"
            "nSourceCols = ALEN(aSourceEvents, 2)\n"
            "lSourceRow1HasHandler = COMPOBJ(aSourceEvents[1,2], oHandler)\n"
            "cSourceRow1Event = aSourceEvents[1,3]\n"
            "cSourceRow1Delegate = aSourceEvents[1,4]\n"
            "cSourceRow2TargetType = VARTYPE(aSourceEvents[2,2])\n"
            "cSourceRow2Event = aSourceEvents[2,3]\n"
            "cSourceRow2Delegate = aSourceEvents[2,4]\n"
            "nSourceRow3Flags = aSourceEvents[3,5]\n"
            "nSourceRow4Flags = aSourceEvents[4,5]\n"
            "nHandlerRows = AEVENTS(aHandlerEvents, oHandler)\n"
            "nHandlerCols = ALEN(aHandlerEvents, 2)\n"
            "lHandlerRow1IsSource = aHandlerEvents[1,1]\n"
            "lHandlerRow1MatchesSource = COMPOBJ(aHandlerEvents[1,2], oSource)\n"
            "cHandlerRow1Event = aHandlerEvents[1,3]\n"
            "cHandlerRow1Delegate = aHandlerEvents[1,4]\n"
            "DIMENSION aExisting[1]\n"
            "aExisting[1] = 'keep'\n"
            "nExistingRows = AEVENTS(aExisting, oPlain)\n"
            "cExistingAfter = aExisting[1]\n"
            "nExistingCols = ALEN(aExisting, 2)\n"
            "nMissingRows = AEVENTS(aMissing, oPlain)\n"
            "cMissingType = TYPE('aMissing')\n"
            "nUnbindSpecific = UNBINDEVENTS(oSource, 'Ping', oHandler, 'HandleBefore')\n"
            "nAfterSpecificRows = AEVENTS(aAfterSpecific, oSource)\n"
            "cAfterSpecificRow1Delegate = aAfterSpecific[1,4]\n"
            "cAfterSpecificRow1TargetType = VARTYPE(aAfterSpecific[1,2])\n"
            "nUnbindObject = UNBINDEVENTS(oHandler)\n"
            "nAfterObjectRows = AEVENTS(aAfterObject, oSource)\n"
            "cAfterObjectRow1Delegate = aAfterObject[1,4]\n"
            "nHandlerAfterObjectRows = AEVENTS(aHandlerAfterObject, oHandler)\n"
            "cHandlerAfterObjectType = TYPE('aHandlerAfterObject')\n"
            "RETURN\n"
            "PROCEDURE HandlePing\n"
            "    LPARAMETERS tnValue\n"
            "    RETURN tnValue\n"
            "ENDPROC\n"
            "DEFINE CLASS SourceThing AS Custom\n"
            "ENDDEFINE\n"
            "DEFINE CLASS HandlerThing AS Custom\n"
            "    FUNCTION HandleBefore\n"
            "        LPARAMETERS tnValue\n"
            "        RETURN tnValue\n"
            "    ENDFUNC\n"
            "    FUNCTION HandleAfter\n"
            "        LPARAMETERS tnValue\n"
            "        RETURN tnValue\n"
            "    ENDFUNC\n"
            "    FUNCTION HandleNoSimple\n"
            "        LPARAMETERS tnValue\n"
            "        RETURN tnValue\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native AEVENTS script should complete: ") + state.message +
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
        check("nsourcerows", "4");
        check("nsourcecols", "5");
        check("lsourcerow1hashandler", "true");
        check("csourcerow1event", "ping");
        check("csourcerow1delegate", "HandleBefore");
        check("csourcerow2targettype", "U");
        check("csourcerow2event", "ping");
        check("csourcerow2delegate", "HandlePing");
        check("nsourcerow3flags", "1");
        check("nsourcerow4flags", "2");
        check("nhandlerrows", "3");
        check("nhandlercols", "5");
        check("lhandlerrow1issource", "true");
        check("chandlerrow1event", "ping");
        check("chandlerrow1delegate", "HandleBefore");
        check("nexistingrows", "0");
        check("cexistingafter", "keep");
        check("nexistingcols", "1");
        check("nmissingrows", "0");
        check("cmissingtype", "U");
        check("nunbindspecific", "1");
        check("nafterspecificrows", "3");
        check("cafterspecificrow1delegate", "HandlePing");
        check("cafterspecificrow1targettype", "U");
        check("nunbindobject", "2");
        check("nafterobjectrows", "1");
        check("cafterobjectrow1delegate", "HandlePing");
        check("nhandlerafterobjectrows", "0");
        check("chandlerafterobjecttype", "U");

        const auto handler_match = state.globals.find("lsourcerow1hashandler");
        const auto source_match = state.globals.find("lhandlerrow1matchessource");
        expect(handler_match != state.globals.end(), "AEVENTS source/handler compare result should be captured");
        expect(source_match != state.globals.end(), "AEVENTS handler/source compare result should be captured");
        if (handler_match != state.globals.end())
        {
            expect(copperfin::runtime::format_value(handler_match->second) == "true",
                   "AEVENTS(aSourceEvents, oSource) should surface the bound handler object");
        }
        if (source_match != state.globals.end())
        {
            expect(copperfin::runtime::format_value(source_match->second) == "true",
                   "AEVENTS(aHandlerEvents, oHandler) should surface the bound source object");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_same_prg_native_aevents_zero_reports_current_event_metadata_during_delegate_dispatch()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_aevents_zero";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_aevents_zero.prg";
        write_text(
            main_path,
            "PUBLIC nFirstRows, nFirstLen, lFirstMatchesSource, cFirstEvent, nFirstType, nSecondRows, nSecondLen, lSecondMatchesSource, cSecondEvent, nSecondType\n"
            "nCaptureCount = 0\n"
            "cLog = ''\n"
            "oSource = CREATEOBJECT('SourceThing')\n"
            "oHandler = CREATEOBJECT('HandlerThing')\n"
            "nBindPing = BINDEVENT(oSource, 'Ping', oHandler, 'CaptureCurrent')\n"
            "nBindRaiseOnly = BINDEVENT(oSource, 'RaiseOnly', oHandler, 'CaptureCurrent')\n"
            "cDirect = oSource.Ping(12)\n"
            "lRaised = RAISEEVENT(oSource, 'RaiseOnly', 34)\n"
            "DIMENSION aExisting[1]\n"
            "aExisting[1] = 'keep'\n"
            "nOutsideRows = AEVENTS(aExisting, 0)\n"
            "cOutsideKeep = aExisting[1]\n"
            "nOutsideMissingRows = AEVENTS(aMissing, 0)\n"
            "cOutsideMissingType = TYPE('aMissing')\n"
            "RETURN\n"
            "DEFINE CLASS SourceThing AS Custom\n"
            "    FUNCTION Ping\n"
            "        LPARAMETERS tnValue\n"
            "        RETURN 'ping:' + TRANSFORM(tnValue)\n"
            "    ENDFUNC\n"
            "    FUNCTION RaiseOnly\n"
            "        LPARAMETERS tnValue\n"
            "        RETURN 'raise:' + TRANSFORM(tnValue)\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS HandlerThing AS Custom\n"
            "    FUNCTION CaptureCurrent\n"
            "        LPARAMETERS tnValue\n"
            "        nCaptureCount = nCaptureCount + 1\n"
            "        nRows = AEVENTS(aCurrent, 0)\n"
            "        IF nCaptureCount = 1\n"
            "            nFirstRows = nRows\n"
            "            nFirstLen = ALEN(aCurrent)\n"
            "            lFirstMatchesSource = COMPOBJ(aCurrent[1], oSource)\n"
            "            cFirstEvent = aCurrent[2]\n"
            "            nFirstType = aCurrent[3]\n"
            "            cLog = cLog + '[first:' + cFirstEvent + ':' + TRANSFORM(aCurrent[3]) + ':' + TRANSFORM(tnValue) + ']'\n"
            "        ELSE\n"
            "            nSecondRows = nRows\n"
            "            nSecondLen = ALEN(aCurrent)\n"
            "            lSecondMatchesSource = COMPOBJ(aCurrent[1], oSource)\n"
            "            cSecondEvent = aCurrent[2]\n"
            "            nSecondType = aCurrent[3]\n"
            "            cLog = cLog + '[second:' + cSecondEvent + ':' + TRANSFORM(aCurrent[3]) + ':' + TRANSFORM(tnValue) + ']'\n"
            "        ENDIF\n"
            "        RETURN tnValue\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native AEVENTS(..., 0) script should complete: ") + state.message +
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

        check("nbindping", "1");
        check("nbindraiseonly", "1");
        check("cdirect", "ping:12");
        check("lraised", "true");
        check("ncapturecount", "2");
        check("nfirstrows", "3");
        check("nfirstlen", "3");
        check("lfirstmatchessource", "true");
        check("cfirstevent", "ping");
        check("nfirsttype", "2");
        check("nsecondrows", "3");
        check("nsecondlen", "3");
        check("lsecondmatchessource", "true");
        check("csecondevent", "raiseonly");
        check("nsecondtype", "1");
        check("clog", "[first:ping:2:12][second:raiseonly:1:34]");
        check("noutsiderows", "0");
        check("coutsidekeep", "keep");
        check("noutsidemissingrows", "0");
        check("coutsidemissingtype", "U");

        fs::remove_all(temp_root, ignored);
    }

    void test_same_prg_native_windows_message_bindevent_and_aevents_dispatch_during_read_events()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_winmsg_bindevent";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_winmsg_bindevent.prg";
        write_text(
            main_path,
            "PUBLIC nExactRows, nExactLen, nExactCurrentHwnd, cExactCurrentEvent, nExactCurrentType, nAllRows, nAllLen, nAllCurrentHwnd, cAllCurrentEvent, nAllCurrentType\n"
            "nExactCalls = 0\n"
            "nAllCalls = 0\n"
            "cLog = ''\n"
            "oHandler = CREATEOBJECT('HandlerThing')\n"
            "nMain = MAINHWND()\n"
            "nBindExact = BINDEVENT(nMain, 274, oHandler, 'HandleExact', 3)\n"
            "nBindAll = BINDEVENT(0, 275, oHandler, 'HandleAll', 3)\n"
            "nMessageRows = AEVENTS(aMessageEvents, 1)\n"
            "nMessageCols = ALEN(aMessageEvents, 2)\n"
            "nRow1Hwnd = aMessageEvents[1,1]\n"
            "nRow1Msg = aMessageEvents[1,2]\n"
            "lRow1Handler = COMPOBJ(aMessageEvents[1,3], oHandler)\n"
            "cRow1Delegate = aMessageEvents[1,4]\n"
            "nRow2Hwnd = aMessageEvents[2,1]\n"
            "nRow2Msg = aMessageEvents[2,2]\n"
            "lRow2Handler = COMPOBJ(aMessageEvents[2,3], oHandler)\n"
            "cRow2Delegate = aMessageEvents[2,4]\n"
            "nUnbindExact = UNBINDEVENTS(nMain, 274)\n"
            "nAfterUnbindRows = AEVENTS(aAfterUnbind, 1)\n"
            "nRebindExact = BINDEVENT(nMain, 274, oHandler, 'HandleExact')\n"
            "nAfterRebindRows = AEVENTS(aAfterRebind, 1)\n"
            "nBindValidText = BINDEVENT(nMain, 276, oHandler, 'HandleText')\n"
            "nBindPartialText = BINDEVENT(nMain, 277, oHandler, 'HandleText')\n"
            "nBindGroupedText = BINDEVENT(nMain, 278, oHandler, 'HandleText')\n"
            "nBindOverflowText = BINDEVENT(nMain, 279, oHandler, 'HandleText')\n"
            "nBindSignedText = BINDEVENT(nMain, 280, oHandler, 'HandleText')\n"
            "READ EVENTS\n"
            "RETURN\n"
            "DEFINE CLASS HandlerThing AS Custom\n"
            "    FUNCTION HandleExact\n"
            "        LPARAMETERS tnHwnd, tnMessage, tnWParam, tnLParam\n"
            "        nExactCalls = nExactCalls + 1\n"
            "        nRows = AEVENTS(aCurrent, 0)\n"
            "        nExactRows = nRows\n"
            "        nExactLen = ALEN(aCurrent)\n"
            "        nExactCurrentHwnd = aCurrent[1]\n"
            "        cExactCurrentEvent = aCurrent[2]\n"
            "        nExactCurrentType = aCurrent[3]\n"
            "        cLog = cLog + '[exact:' + TRANSFORM(tnHwnd) + ':' + TRANSFORM(tnMessage) + ':' + TRANSFORM(tnWParam) + ':' + TRANSFORM(tnLParam) + ':' + TRANSFORM(aCurrent[3]) + ']'\n"
            "        RETURN tnWParam + tnLParam\n"
            "    ENDFUNC\n"
            "    FUNCTION HandleText\n"
            "        LPARAMETERS tnHwnd, tnMessage, tnWParam, tnLParam\n"
            "        DO CASE\n"
            "        CASE tnMessage = 276\n"
            "            RETURN '48'\n"
            "        CASE tnMessage = 277\n"
            "            RETURN '48junk'\n"
            "        CASE tnMessage = 278\n"
            "            RETURN '4.8'\n"
            "        CASE tnMessage = 279\n"
            "            RETURN '999999999999999999999999999999999'\n"
            "        OTHERWISE\n"
            "            RETURN '-7'\n"
            "        ENDCASE\n"
            "    ENDFUNC\n"
            "    FUNCTION HandleAll\n"
            "        LPARAMETERS tnHwnd, tnMessage, tnWParam, tnLParam\n"
            "        nAllCalls = nAllCalls + 1\n"
            "        nRows = AEVENTS(aCurrentAll, 0)\n"
            "        nAllRows = nRows\n"
            "        nAllLen = ALEN(aCurrentAll)\n"
            "        nAllCurrentHwnd = aCurrentAll[1]\n"
            "        cAllCurrentEvent = aCurrentAll[2]\n"
            "        nAllCurrentType = aCurrentAll[3]\n"
            "        cLog = cLog + '[all:' + TRANSFORM(tnHwnd) + ':' + TRANSFORM(tnMessage) + ':' + TRANSFORM(tnWParam) + ':' + TRANSFORM(tnLParam) + ':' + TRANSFORM(aCurrentAll[3]) + ']'\n"
            "        CLEAR EVENTS\n"
            "        RETURN tnMessage\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop,
               std::string("Windows-message BINDEVENT script should pause in READ EVENTS: ") + state.message);
        expect(state.waiting_for_events,
               "Windows-message BINDEVENT script should report waiting_for_events while paused");

        const auto exact_result = session.dispatch_windows_message(1001, 274, 16, 32);
        expect(exact_result.has_value(),
               "Windows-message BINDEVENT should dispatch the exact hWnd/message binding");
        if (exact_result.has_value())
        {
            expect(*exact_result == 48,
                   "Windows-message BINDEVENT exact delegate should return the integer handler result");
        }

        state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop,
               std::string("Windows-message exact dispatch should restore the READ EVENTS pause: ") + state.message);
        expect(state.waiting_for_events,
               "Windows-message exact dispatch should return the runtime to waiting_for_events");

        const auto dispatch_text_result = [&](int message, std::intptr_t expected, const std::string &label)
        {
            const auto result = session.dispatch_windows_message(1001, message, 0, 0);
            expect(result.has_value(), label + " should dispatch its exact Windows-message binding");
            if (result.has_value())
            {
                expect(*result == expected,
                       label + " expected " + std::to_string(expected) + " got " +
                           std::to_string(*result));
            }
            state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
            expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop &&
                       state.waiting_for_events,
                   label + " should restore the READ EVENTS pause");
        };
        dispatch_text_result(276, 48, "valid character message result");
        dispatch_text_result(277, 0, "partial character message result");
        dispatch_text_result(278, 0, "grouped/decimal character message result");
        dispatch_text_result(279, 0, "overflowing character message result");
        dispatch_text_result(280, -7, "signed character message result");

        const auto wildcard_result = session.dispatch_windows_message(2222, 275, 8, 4);
        expect(wildcard_result.has_value(),
               "Windows-message BINDEVENT should dispatch the wildcard hWnd binding");
        if (wildcard_result.has_value())
        {
            expect(*wildcard_result == 275,
                   "Windows-message BINDEVENT wildcard delegate should return the integer handler result");
        }

        state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("Windows-message BINDEVENT script should complete after CLEAR EVENTS: ") + state.message +
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

        check("nmain", "1001");
        check("nbindexact", "1");
        check("nbindall", "1");
        check("nbindvalidtext", "1");
        check("nbindpartialtext", "1");
        check("nbindgroupedtext", "1");
        check("nbindoverflowtext", "1");
        check("nbindsignedtext", "1");
        check("nmessagerows", "2");
        check("nmessagecols", "4");
        check("nrow1hwnd", "1001");
        check("nrow1msg", "274");
        check("lrow1handler", "true");
        check("crow1delegate", "HandleExact");
        check("nrow2hwnd", "0");
        check("nrow2msg", "275");
        check("lrow2handler", "true");
        check("crow2delegate", "HandleAll");
        check("nunbindexact", "1");
        check("nafterunbindrows", "1");
        check("nrebindexact", "1");
        check("nafterrebindrows", "2");
        check("nexactcalls", "1");
        check("nallcalls", "1");
        check("nexactrows", "3");
        check("nexactlen", "3");
        check("nexactcurrenthwnd", "1001");
        check("cexactcurrentevent", "274");
        check("nexactcurrenttype", "0");
        check("nallrows", "3");
        check("nalllen", "3");
        check("nallcurrenthwnd", "2222");
        check("callcurrentevent", "275");
        check("nallcurrenttype", "0");
        check("clog", "[exact:1001:274:16:32:0][all:2222:275:8:4:0]");

        const bool has_bind_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.event.bind" &&
                   (event.detail == "1001:274 -> HandleExact" ||
                    event.detail == "0:275 -> HandleAll");
        });
        expect(has_bind_event,
               "Windows-message BINDEVENT should emit runtime bind telemetry");

        const bool has_delegate_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.event.delegate" &&
                   (event.detail == "1001:274 -> HandlerThing.HandleExact" ||
                    event.detail == "2222:275 -> HandlerThing.HandleAll");
        });
        expect(has_delegate_event,
               "Windows-message BINDEVENT should emit delegate telemetry during dispatch");

        fs::remove_all(temp_root, ignored);
    }

    void test_runtime_hwnd_and_sys2326_sys2327_surfaces_bind_representative_window_objects()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_hwnd_sys2326";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "runtime_hwnd_sys2326.prg";
        write_text(
            main_path,
            "PUBLIC nFormDispatchHwnd, nFormDispatchWHandle, nFormDispatchRoundTrip, nScreenDispatchHwnd, nScreenDispatchWHandle, nScreenDispatchRoundTrip\n"
            "oHandler = CREATEOBJECT('HandleSink')\n"
            "oForm = CREATEOBJECT('MainForm')\n"
            "oToolbar = CREATEOBJECT('MainToolbar')\n"
            "nMainHwnd = MAINHWND()\n"
            "nScreenHwnd = _SCREEN.hWnd\n"
            "nVfpHwnd = _VFP.hWnd\n"
            "nFormHwnd = oForm.hWnd\n"
            "nToolbarHwnd = oToolbar.hWnd\n"
            "xFormGetPem = GETPEM(oForm, 'hWnd')\n"
            "xToolbarGetPem = GETPEM(oToolbar, 'hWnd')\n"
            "lFormHasHwnd = PEMSTATUS(oForm, 'hWnd', 1)\n"
            "lToolbarHasHwnd = PEMSTATUS(oToolbar, 'hWnd', 1)\n"
            "lFormHwndReadOnly = PEMSTATUS(oForm, 'hWnd', 5)\n"
            "lToolbarHwndReadOnly = PEMSTATUS(oToolbar, 'hWnd', 5)\n"
            "lSetFormHwnd = SETPEM(oForm, 'hWnd', 88)\n"
            "nScreenWHandle = SYS(2326, nScreenHwnd)\n"
            "nGroupedWHandle = SYS(2326, STR(nScreenHwnd) + '.0')\n"
            "nVfpWHandle = SYS(2326, nVfpHwnd)\n"
            "nFormWHandle = SYS(2326, nFormHwnd)\n"
            "nToolbarWHandle = SYS(2326, nToolbarHwnd)\n"
            "nScreenRoundTrip = SYS(2327, nScreenWHandle)\n"
            "nVfpRoundTrip = SYS(2327, nVfpWHandle)\n"
            "nFormRoundTrip = SYS(2327, nFormWHandle)\n"
            "nToolbarRoundTrip = SYS(2327, nToolbarWHandle)\n"
            "nUnknownWHandle = SYS(2326, 999999)\n"
            "nUnknownHwnd = SYS(2327, 999999)\n"
            "nBindForm = BINDEVENT(nFormHwnd, 513, oHandler, 'HandleForm')\n"
            "nBindScreen = BINDEVENT(nScreenHwnd, 514, oHandler, 'HandleScreen')\n"
            "READ EVENTS\n"
            "RETURN\n"
            "DEFINE CLASS MainForm AS Form\n"
            "ENDDEFINE\n"
            "DEFINE CLASS MainToolbar AS Toolbar\n"
            "ENDDEFINE\n"
            "DEFINE CLASS HandleSink AS Custom\n"
            "    FUNCTION HandleForm\n"
            "        LPARAMETERS tnHwnd, tnMessage, tnWParam, tnLParam\n"
            "        nFormDispatchHwnd = tnHwnd\n"
            "        nFormDispatchWHandle = SYS(2326, tnHwnd)\n"
            "        nFormDispatchRoundTrip = SYS(2327, nFormDispatchWHandle)\n"
            "        RETURN tnMessage\n"
            "    ENDFUNC\n"
            "    FUNCTION HandleScreen\n"
            "        LPARAMETERS tnHwnd, tnMessage, tnWParam, tnLParam\n"
            "        nScreenDispatchHwnd = tnHwnd\n"
            "        nScreenDispatchWHandle = SYS(2326, tnHwnd)\n"
            "        nScreenDispatchRoundTrip = SYS(2327, nScreenDispatchWHandle)\n"
            "        CLEAR EVENTS\n"
            "        RETURN tnMessage\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop,
               std::string("hWnd/SYS(2326) script should pause in READ EVENTS: ") + state.message);
        expect(state.waiting_for_events,
               "hWnd/SYS(2326) script should report waiting_for_events while paused");

        const auto form_hack = state.globals.find("nformhwnd");
        expect(form_hack != state.globals.end(),
               "hWnd/SYS(2326) script should capture the form hWnd before dispatch");
        const auto screen_hack = state.globals.find("nscreenhwnd");
        expect(screen_hack != state.globals.end(),
               "hWnd/SYS(2326) script should capture the _SCREEN hWnd before dispatch");
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

        const std::intptr_t form_hwnd = form_hack == state.globals.end()
                                            ? 0
                                            : static_cast<std::intptr_t>(value_to_int64(form_hack->second));
        const std::intptr_t screen_hwnd = screen_hack == state.globals.end()
                                              ? 0
                                              : static_cast<std::intptr_t>(value_to_int64(screen_hack->second));

        const auto form_dispatch = session.dispatch_windows_message(form_hwnd, 513, 1, 2);
        expect(form_dispatch.has_value(),
               "hWnd/SYS(2326) slice should dispatch through the form hWnd");
        if (form_dispatch.has_value())
        {
            expect(*form_dispatch == 513,
                   "Form hWnd dispatch should return the delegate message result");
        }

        state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop,
               std::string("Form hWnd dispatch should restore the READ EVENTS pause: ") + state.message);
        expect(state.waiting_for_events,
               "Form hWnd dispatch should return the runtime to waiting_for_events");

        const auto screen_dispatch = session.dispatch_windows_message(screen_hwnd, 514, 3, 4);
        expect(screen_dispatch.has_value(),
               "hWnd/SYS(2326) slice should dispatch through the _SCREEN hWnd");
        if (screen_dispatch.has_value())
        {
            expect(*screen_dispatch == 514,
                   "_SCREEN hWnd dispatch should return the delegate message result");
        }

        state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("hWnd/SYS(2326) script should complete after CLEAR EVENTS: ") + state.message +
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

        check("nmainhwnd", "1001");
        check("nscreenhwnd", "1001");
        check("nvfphwnd", "1000");
        check("lformhashwnd", "true");
        check("ltoolbarhashwnd", "true");
        check("lformhwndreadonly", "true");
        check("ltoolbarhwndreadonly", "true");
        check("lsetformhwnd", "false");
        check("nscreenwhandle", "900001");
        check("ngroupedwhandle", "0");
        check("nvfpwhandle", "900002");
        check("nscreenroundtrip", "1001");
        check("nvfproundtrip", "1000");
        check("nunknownwhandle", "0");
        check("nunknownhwnd", "0");
        check("nbindform", "1");
        check("nbindscreen", "1");
        check("nformdispatchhwnd", copperfin::runtime::format_value(state.globals.at("nformhwnd")));
        check("nscreendispatchhwnd", "1001");
        check("nscreendispatchwhandle", "900001");
        check("nscreendispatchroundtrip", "1001");

        const std::int64_t form_whandle = require_number("nformwhandle");
        const std::int64_t toolbar_whandle = require_number("ntoolbarwhandle");
        const std::int64_t form_hwnd_after = require_number("nformhwnd");
        const std::int64_t toolbar_hwnd_after = require_number("ntoolbarhwnd");
        const std::int64_t form_round_trip = require_number("nformroundtrip");
        const std::int64_t toolbar_round_trip = require_number("ntoolbarroundtrip");
        const std::int64_t form_getpem = require_number("xformgetpem");
        const std::int64_t toolbar_getpem = require_number("xtoolbargetpem");
        const std::int64_t form_dispatch_whandle = require_number("nformdispatchwhandle");
        const std::int64_t form_dispatch_round_trip = require_number("nformdispatchroundtrip");

        expect(form_whandle > 0,
               "Form hWnd translation should expose a positive WHANDLE");
        expect(toolbar_whandle > 0,
               "Toolbar hWnd translation should expose a positive WHANDLE");
        expect(form_hwnd_after == form_getpem,
               "Form hWnd should read consistently through ordinary property access and GETPEM()");
        expect(toolbar_hwnd_after == toolbar_getpem,
               "Toolbar hWnd should read consistently through ordinary property access and GETPEM()");
        expect(form_round_trip == form_hwnd_after,
               "SYS(2326/2327) should round-trip the form hWnd");
        expect(toolbar_round_trip == toolbar_hwnd_after,
               "SYS(2326/2327) should round-trip the toolbar hWnd");
        expect(form_hwnd_after == 100000 + form_whandle,
               "Form hWnd should follow the deterministic modeled runtime mapping");
        expect(toolbar_hwnd_after == 100000 + toolbar_whandle,
               "Toolbar hWnd should follow the deterministic modeled runtime mapping");
        expect(form_dispatch_whandle == form_whandle,
               "Dispatched form hWnd should translate back to the same WHANDLE");
        expect(form_dispatch_round_trip == form_hwnd_after,
               "Dispatched form hWnd should round-trip through SYS(2326/2327)");

        fs::remove_all(temp_root, ignored);
    }

    void test_same_prg_native_event_delegate_chain_preserves_context_and_recovers()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_event_delegate_chain";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_event_delegate_chain.prg";
        write_text(
            main_path,
            "PUBLIC nBridgeCalls, nHandlerCalls, nBridgeRows, nHandlerRows, nBridgeAfterRows, nBridgeAfterType, nAfterFaultRows, nReentryCalls, lBridgeSource, lBridgeSourceAfter, lHandlerSource, lReentry, cBridgeAfterEvent, cLog\n"
            "nBridgeCalls = 0\n"
            "nHandlerCalls = 0\n"
            "cLog = ''\n"
            "oSource = CREATEOBJECT('SourceThing')\n"
            "oBridge = CREATEOBJECT('BridgeThing')\n"
            "oTarget = CREATEOBJECT('TargetThing')\n"
            "oHandler = CREATEOBJECT('HandlerThing')\n"
            "nBindSource = BINDEVENT(oSource, 'Ping', oBridge, 'ForwardPing')\n"
            "nBindTarget = BINDEVENT(oTarget, 'Pong', oHandler, 'HandlePong')\n"
            "lCaught = .F.\n"
            "TRY\n"
            "    lFirst = RAISEEVENT(oSource, 'Ping', 7)\n"
            "CATCH TO oErr\n"
            "    lCaught = .T.\n"
            "    nAfterFaultRows = AEVENTS(aAfterFault, 0)\n"
            "ENDTRY\n"
            "lSecond = RAISEEVENT(oSource, 'Ping', 8)\n"
            "RETURN\n"
            "DEFINE CLASS SourceThing AS Custom\n"
            "    FUNCTION Ping\n"
            "        LPARAMETERS tnValue\n"
            "        RETURN tnValue\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS BridgeThing AS Custom\n"
            "    FUNCTION ForwardPing\n"
            "        LPARAMETERS tnValue\n"
            "        nBridgeCalls = nBridgeCalls + 1\n"
            "        nBridgeRows = AEVENTS(aBridgeCurrent, 0)\n"
            "        lBridgeSource = COMPOBJ(aBridgeCurrent[1], oSource)\n"
            "        cLog = cLog + '[bridge:' + aBridgeCurrent[2] + ':' + TRANSFORM(aBridgeCurrent[3]) + ':' + TRANSFORM(tnValue) + ']'\n"
            "        nReentryCalls = nReentryCalls + 1\n"
            "        lReentry = RAISEEVENT(oSource, 'Ping', tnValue)\n"
            "        lNested = RAISEEVENT(oTarget, 'Pong', tnValue + 1)\n"
            "        nBridgeAfterRows = AEVENTS(aBridgeAfter, 0)\n"
            "        cBridgeAfterEvent = aBridgeAfter[2]\n"
            "        nBridgeAfterType = aBridgeAfter[3]\n"
            "        lBridgeSourceAfter = COMPOBJ(aBridgeAfter[1], oSource)\n"
            "        RETURN lNested\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS TargetThing AS Custom\n"
            "    FUNCTION Pong\n"
            "        LPARAMETERS tnValue\n"
            "        RETURN tnValue\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS HandlerThing AS Custom\n"
            "    FUNCTION HandlePong\n"
            "        LPARAMETERS tnValue\n"
            "        nHandlerCalls = nHandlerCalls + 1\n"
            "        nHandlerRows = AEVENTS(aHandlerCurrent, 0)\n"
            "        lHandlerSource = COMPOBJ(aHandlerCurrent[1], oTarget)\n"
            "        cLog = cLog + '[handler:' + aHandlerCurrent[2] + ':' + TRANSFORM(aHandlerCurrent[3]) + ':' + TRANSFORM(tnValue) + ']'\n"
            "        IF nHandlerCalls = 1\n"
            "            1 / 0\n"
            "        ENDIF\n"
            "        RETURN tnValue\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native event delegate chain should recover after nested handler fault: ") + state.message +
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

        check("nbindsource", "1");
        check("nbindtarget", "1");
        check("lcaught", "true");
        check("lsecond", "true");
        check("nbridgecalls", "2");
        check("nhandlercalls", "2");
        check("nbridgerows", "3");
        check("nhandlerrows", "3");
        check("nbridgeafterrows", "3");
        check("nbridgeaftertype", "1");
        check("nafterfaultrows", "0");
        check("nreentrycalls", "2");
        check("lbridgesource", "true");
        check("lbridgesourceafter", "true");
        check("lhandlersource", "true");
        check("lreentry", "true");
        check("cbridgeafterevent", "ping");
        check("clog", "[bridge:ping:1:7][handler:pong:1:8][bridge:ping:1:8][handler:pong:1:9]");

        std::vector<std::string> delegate_details;
        for (const auto &event : state.events)
        {
            if (event.category == "prg.event.delegate")
            {
                expect(event.location.line > 0,
                       "nested native event delegate telemetry should retain a source line");
                expect(event.location.file_path == main_path.string(),
                       "nested native event delegate telemetry should retain the source path");
                delegate_details.push_back(event.detail);
            }
        }
        expect(delegate_details == std::vector<std::string>{
                                    "ping -> BridgeThing.ForwardPing",
                                    "pong -> HandlerThing.HandlePong",
                                    "ping -> BridgeThing.ForwardPing",
                                    "pong -> HandlerThing.HandlePong"},
               "nested native event dispatch should emit exactly-once delegate telemetry in dispatch order");

        fs::remove_all(temp_root, ignored);
    }

}
