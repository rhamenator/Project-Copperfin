#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_same_prg_native_bindevent_property_access_and_assign_dispatch_preserve_current_event_metadata()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_bindevent_property";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_bindevent_property.prg";
        write_text(
            main_path,
            "PUBLIC nCaptionFirstRows, lCaptionFirstSource, nRawFirstRows, lRawFirstSource\n"
            "cLog = ''\n"
            "nCaptionCalls = 0\n"
            "nRawCalls = 0\n"
            "oCreate = CREATEOBJECT('ChildWidget')\n"
            "nBindCaption = BINDEVENT(oCreate, 'Caption', 'HandleCaption')\n"
            "nBindRaw = BINDEVENT(oCreate, 'nRaw', 'HandleRaw', 1)\n"
            "cCaptionBefore = oCreate.Caption\n"
            "oCreate.Caption = 'Set'\n"
            "cCaptionAfter = oCreate.Caption\n"
            "nRawBefore = oCreate.nRaw\n"
            "oCreate.nRaw = 9\n"
            "nRawAfter = oCreate.nRaw\n"
            "cGetCaption = GETPEM(oCreate, 'Caption')\n"
            "lSetCaption = SETPEM(oCreate, 'Caption', 'PemSet')\n"
            "cAfterSetPem = oCreate.Caption\n"
            "lSetRaw = SETPEM(oCreate, 'nRaw', 11)\n"
            "nAfterSetPemRaw = oCreate.nRaw\n"
            "RETURN\n"
            "PROCEDURE HandleCaption\n"
            "    LPARAMETERS tuValue\n"
            "    nCaptionCalls = nCaptionCalls + 1\n"
            "    nRows = AEVENTS(aCurrent, 0)\n"
            "    cLog = cLog + '[caption:' + aCurrent[2] + ':' + TRANSFORM(aCurrent[3]) + ':' + TRANSFORM(PCOUNT()) + ':' + IIF(PCOUNT() = 0, 'none', TRANSFORM(tuValue)) + ']'\n"
            "    IF nCaptionCalls = 1\n"
            "        nCaptionFirstRows = nRows\n"
            "        lCaptionFirstSource = COMPOBJ(aCurrent[1], oCreate)\n"
            "    ENDIF\n"
            "    RETURN\n"
            "ENDPROC\n"
            "PROCEDURE HandleRaw\n"
            "    LPARAMETERS tuValue\n"
            "    nRawCalls = nRawCalls + 1\n"
            "    nRows = AEVENTS(aCurrentRaw, 0)\n"
            "    cLog = cLog + '[raw:' + aCurrentRaw[2] + ':' + TRANSFORM(aCurrentRaw[3]) + ':' + TRANSFORM(PCOUNT()) + ':' + IIF(PCOUNT() = 0, 'none', TRANSFORM(tuValue)) + ']'\n"
            "    IF nRawCalls = 1\n"
            "        nRawFirstRows = nRows\n"
            "        lRawFirstSource = COMPOBJ(aCurrentRaw[1], oCreate)\n"
            "    ENDIF\n"
            "    RETURN\n"
            "ENDPROC\n"
            "DEFINE CLASS ParentWidget AS Custom\n"
            "    cBacking = 'Parent'\n"
            "    nRaw = 5\n"
            "    FUNCTION Caption_Access\n"
            "        RETURN THIS.cBacking + ':A'\n"
            "    ENDFUNC\n"
            "    PROCEDURE Caption_Assign\n"
            "        LPARAMETERS tcValue\n"
            "        THIS.cBacking = tcValue + ':S'\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ChildWidget AS ParentWidget\n"
            "    cBacking = 'Child'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native BINDEVENT property script should complete: ") + state.message +
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

        check("nbindcaption", "1");
        check("nbindraw", "1");
        check("ccaptionbefore", "Child:A");
        check("ccaptionafter", "Set:S:A");
        check("nrawbefore", "5");
        check("nrawafter", "9");
        check("cgetcaption", "Set:S:A");
        check("lsetcaption", "true");
        check("caftersetpem", "PemSet:S:A");
        check("lsetraw", "true");
        check("naftersetpemraw", "11");
        check("ncaptioncalls", "6");
        check("nrawcalls", "5");
        check("ncaptionfirstrows", "3");
        check("lcaptionfirstsource", "true");
        check("nrawfirstrows", "3");
        check("lrawfirstsource", "true");
        check("clog",
              "[caption:caption:2:0:none][caption:caption:2:1:Set][caption:caption:2:0:none][raw:nraw:2:0:none][raw:nraw:2:1:9][raw:nraw:2:0:none][caption:caption:2:0:none][caption:caption:2:1:PemSet][caption:caption:2:0:none][raw:nraw:2:1:11][raw:nraw:2:0:none]");

        const bool has_delegate_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.event.delegate" &&
                   (event.detail == "caption -> HandleCaption" ||
                    event.detail == "nraw -> HandleRaw");
        });
        expect(has_delegate_event,
               "native property BINDEVENT dispatch should emit delegate events");

        fs::remove_all(temp_root, ignored);
    }

    void test_same_prg_native_bindevent_property_handler_fault_does_not_disable_future_dispatch()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_property_bindevent_fault_recovery";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_property_bindevent_fault_recovery.prg";
        write_text(
            main_path,
            "nCaptionCalls = 0\n"
            "nRawCalls = 0\n"
            "oCreate = CREATEOBJECT('ChildWidget')\n"
            "nBindCaption = BINDEVENT(oCreate, 'Caption', 'HandleCaption')\n"
            "nBindRaw = BINDEVENT(oCreate, 'nRaw', 'HandleRaw', 1)\n"
            "lCaptionCaught = .F.\n"
            "TRY\n"
            "    cCaptionFirst = oCreate.Caption\n"
            "CATCH TO oCaptionErr\n"
            "    lCaptionCaught = .T.\n"
            "    cCaptionError = oCaptionErr.Message\n"
            "ENDTRY\n"
            "cCaptionSecond = oCreate.Caption\n"
            "lRawCaught = .F.\n"
            "TRY\n"
            "    oCreate.nRaw = 7\n"
            "CATCH TO oRawErr\n"
            "    lRawCaught = .T.\n"
            "    cRawError = oRawErr.Message\n"
            "ENDTRY\n"
            "oCreate.nRaw = 9\n"
            "nRawFinal = oCreate.nRaw\n"
            "RETURN\n"
            "PROCEDURE HandleCaption\n"
            "    LPARAMETERS tuValue\n"
            "    nCaptionCalls = nCaptionCalls + 1\n"
            "    IF nCaptionCalls = 1\n"
            "        1 / 0\n"
            "    ENDIF\n"
            "    RETURN\n"
            "ENDPROC\n"
            "PROCEDURE HandleRaw\n"
            "    LPARAMETERS tuValue\n"
            "    nRawCalls = nRawCalls + 1\n"
            "    IF nRawCalls = 1\n"
            "        1 / 0\n"
            "    ENDIF\n"
            "    RETURN\n"
            "ENDPROC\n"
            "DEFINE CLASS ParentWidget AS Custom\n"
            "    cBacking = 'Parent'\n"
            "    nRaw = 5\n"
            "    FUNCTION Caption_Access\n"
            "        RETURN THIS.cBacking + ':A'\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ChildWidget AS ParentWidget\n"
            "    cBacking = 'Child'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native property BINDEVENT fault-recovery script should complete: ") + state.message +
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

        check("nbindcaption", "1");
        check("nbindraw", "1");
        check("lcaptioncaught", "true");
        check("ccaptionsecond", "Child:A");
        check("ncaptioncalls", "2");
        check("lrawcaught", "true");
        check("nrawfinal", "9");
        check("nrawcalls", "3");

        const auto caption_error = state.globals.find("ccaptionerror");
        const auto raw_error = state.globals.find("crawerror");
        expect(caption_error != state.globals.end(),
               "faulting property-read delegate should populate the CATCH error message");
        expect(raw_error != state.globals.end(),
               "faulting property-write delegate should populate the CATCH error message");

        fs::remove_all(temp_root, ignored);
    }

    void test_same_prg_native_access_assign_methods_virtualize_ordinary_property_reads_and_writes()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_access_assign";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_access_assign.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildWidget')\n"
            "oPlain = CREATEOBJECT('Empty')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "oPlain.Extra = 'plain'\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 9)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "cCaptionBefore = oCreate.Caption\n"
            "oCreate.Caption = 'Set'\n"
            "cCaptionAfter = oCreate.Caption\n"
            "cDescribe = oCreate.Describe()\n"
            "nAssignCount = oCreate.nAssignCount\n"
            "nRawBefore = oCreate.nRaw\n"
            "oCreate.nRaw = 9\n"
            "nRawAfter = oCreate.nRaw\n"
            "cBacking = oCreate.cBacking\n"
            "cPlain = oPlain.Extra\n"
            "RETURN\n"
            "DEFINE CLASS ParentWidget AS Custom\n"
            "    cBacking = 'Parent'\n"
            "    nAssignCount = 0\n"
            "    nRaw = 5\n"
            "    FUNCTION Caption_Access\n"
            "        RETURN THIS.cBacking + ':A'\n"
            "    ENDFUNC\n"
            "    PROCEDURE Caption_Assign\n"
            "        LPARAMETERS tcValue\n"
            "        THIS.cBacking = tcValue + ':S'\n"
            "        THIS.nAssignCount = THIS.nAssignCount + 1\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "    FUNCTION Describe\n"
            "        RETURN THIS.Caption + ':' + TRANSFORM(THIS.nAssignCount)\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ChildWidget AS ParentWidget\n"
            "    cBacking = 'Child'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native ACCESS/ASSIGN script should complete: ") + state.message +
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

        check("ccaptionbefore", "Child:A");
        check("ccaptionafter", "Set:S:A");
        check("cdescribe", "Set:S:A:1");
        check("nassigncount", "1");
        check("nrawbefore", "5");
        check("nrawafter", "9");
        check("cbacking", "Set:S");
        check("cplain", "plain");
        check("ldictset", "true");
        check("ndictcompare", "9");

        expect(state.ole_objects.size() == 3U,
               "native ACCESS/ASSIGN script should register native, plain, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &native_object = state.ole_objects[0];
            expect(native_object.prog_id == "ChildWidget",
                   "native ACCESS/ASSIGN should preserve child class identity");
            const auto backing = native_object.properties.find("cbacking");
            const auto assign_count = native_object.properties.find("nassigncount");
            const auto raw_value = native_object.properties.find("nraw");
            if (backing != native_object.properties.end())
            {
                expect(copperfin::runtime::format_value(backing->second) == "Set:S",
                       "native ACCESS/ASSIGN should let ASSIGN methods update backing state");
            }
            else
            {
                expect(false, "native ACCESS/ASSIGN should materialize updated backing state");
            }
            if (assign_count != native_object.properties.end())
            {
                expect(copperfin::runtime::format_value(assign_count->second) == "1",
                       "native ACCESS/ASSIGN should preserve ASSIGN-side state updates");
            }
            else
            {
                expect(false, "native ACCESS/ASSIGN should materialize assign-count state");
            }
            if (raw_value != native_object.properties.end())
            {
                expect(copperfin::runtime::format_value(raw_value->second) == "9",
                       "native ACCESS/ASSIGN should preserve raw-property fallback when no accessor exists");
            }
            else
            {
                expect(false, "native ACCESS/ASSIGN should materialize raw-property fallback state");
            }

            expect(state.ole_objects[1].prog_id == "Empty",
                   "plain CREATEOBJECT should remain stable while native ACCESS/ASSIGN lands");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native ACCESS/ASSIGN lands");
        }

        const bool has_accessor_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   (event.detail == "ParentWidget.Caption_Access" ||
                    event.detail == "ParentWidget.Caption_Assign");
        });
        expect(has_accessor_invoke_event,
               "native ACCESS/ASSIGN should emit accessor-method invoke events");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_assigner_same_property_write_uses_raw_storage()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_reentrant_assign";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_reentrant_assign.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ReportBuilderLike')\n"
            "oCreate.QuietMode = !INLIST(1, 0, 4)\n"
            "lQuietMode = oCreate.QuietMode\n"
            "RETURN\n"
            "DEFINE CLASS ReportBuilderLike AS Custom\n"
            "    QuietMode = .F.\n"
            "    PROCEDURE QuietMode_Assign\n"
            "        LPARAMETERS tvNewVal\n"
            "        IF VARTYPE(tvNewVal) = 'L'\n"
            "            THIS.quietmode = m.tvNewVal\n"
            "        ENDIF\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("same-property ASSIGN script should complete: ") + state.message +
                   " @line=" + std::to_string(state.location.line));

        const auto quiet_mode = state.globals.find("lquietmode");
        expect(quiet_mode != state.globals.end(),
               "same-property ASSIGN script should expose the assigned value");
        if (quiet_mode != state.globals.end())
        {
            expect(copperfin::runtime::format_value(quiet_mode->second) == "true",
                   "same-property ASSIGN should commit the value through raw storage");
        }

        expect(state.ole_objects.size() == 1U,
               "same-property ASSIGN script should create one native object");
        if (state.ole_objects.size() == 1U)
        {
            const auto property = state.ole_objects[0].properties.find("quietmode");
            expect(property != state.ole_objects[0].properties.end(),
                   "same-property ASSIGN should materialize the raw property value");
            if (property != state.ole_objects[0].properties.end())
            {
                expect(copperfin::runtime::format_value(property->second) == "true",
                       "same-property ASSIGN should preserve the logical value");
            }
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_native_accessor_backed_properties_reflect_through_getpem_pemstatus_and_amembers()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_accessor_reflection";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_accessor_reflection.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildWidget')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 10)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "cGetCaption = GETPEM(oCreate, 'Caption')\n"
            "cGetStatus = GETPEM(oCreate, 'Status')\n"
            "lHasCaption = PEMSTATUS(oCreate, 'Caption', 1)\n"
            "lHasStatus = PEMSTATUS(oCreate, 'Status', 1)\n"
            "lCaptionReadOnly = PEMSTATUS(oCreate, 'Caption', 5)\n"
            "lStatusReadOnly = PEMSTATUS(oCreate, 'Status', 5)\n"
            "nMembersProps = AMEMBERS(aMembersProps, oCreate, 1)\n"
            "nMembersUnion = AMEMBERS(aMembersUnion, oCreate, 3)\n"
            "cProp1 = aMembersProps[1]\n"
            "cProp2 = aMembersProps[2]\n"
            "cProp4 = aMembersProps[4]\n"
            "RETURN\n"
            "DEFINE CLASS ParentWidget AS Custom\n"
            "    cBacking = 'Parent'\n"
            "    cStatusBacking = 'Ready'\n"
            "    FUNCTION Caption_Access\n"
            "        RETURN THIS.cBacking + ':A'\n"
            "    ENDFUNC\n"
            "    PROCEDURE Caption_Assign\n"
            "        LPARAMETERS tcValue\n"
            "        THIS.cBacking = tcValue + ':S'\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "    FUNCTION Status_Access\n"
            "        RETURN THIS.cStatusBacking + ':R'\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ChildWidget AS ParentWidget\n"
            "    cBacking = 'Child'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native accessor reflection script should complete: ") + state.message +
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

        check("cgetcaption", "Child:A");
        check("cgetstatus", "Ready:R");
        check("lhascaption", "true");
        check("lhasstatus", "true");
        check("lcaptionreadonly", "false");
        check("lstatusreadonly", "true");
        check("nmembersprops", "7");
        check("nmembersunion", "17");
        check("cprop1", "BASECLASS");
        check("cprop2", "CAPTION");
        check("cprop4", "CLASS");
        check("ldictset", "true");
        check("ndictcompare", "10");

        expect(state.ole_objects.size() == 2U,
               "native accessor reflection script should register native and COM objects");
        if (state.ole_objects.size() == 2U)
        {
            expect(state.ole_objects[0].prog_id == "ChildWidget",
                   "native accessor reflection should preserve child class identity");
            expect(state.ole_objects[1].prog_id == "Scripting.Dictionary",
                   "COM object reflection should remain stable while native accessor reflection lands");
        }
    }

    void test_external_prg_base_accessor_backed_properties_dispatch_and_reflect()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_external_accessor_reflection";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path library_path = temp_root / "widgetlib.prg";
        write_text(
            library_path,
            "DEFINE CLASS ParentWidget AS Custom\n"
            "    cBacking = 'Parent'\n"
            "    cStatusBacking = 'Ready'\n"
            "    nAssignCount = 0\n"
            "    FUNCTION Caption_Access\n"
            "        RETURN THIS.cBacking + ':A'\n"
            "    ENDFUNC\n"
            "    PROCEDURE Caption_Assign\n"
            "        LPARAMETERS tcValue\n"
            "        THIS.cBacking = tcValue + ':S'\n"
            "        THIS.nAssignCount = THIS.nAssignCount + 1\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "    FUNCTION Status_Access\n"
            "        RETURN THIS.cStatusBacking + ':R'\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_accessor_reflection.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildWidget')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 21)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "cCaptionBefore = oCreate.Caption\n"
            "oCreate.Caption = 'Set'\n"
            "cCaptionAfter = oCreate.Caption\n"
            "cBacking = oCreate.cBacking\n"
            "nAssignCount = oCreate.nAssignCount\n"
            "cGetCaption = GETPEM(oCreate, 'Caption')\n"
            "cGetStatus = GETPEM(oCreate, 'Status')\n"
            "lHasCaption = PEMSTATUS(oCreate, 'Caption', 1)\n"
            "lHasStatus = PEMSTATUS(oCreate, 'Status', 1)\n"
            "lCaptionReadOnly = PEMSTATUS(oCreate, 'Caption', 5)\n"
            "lStatusReadOnly = PEMSTATUS(oCreate, 'Status', 5)\n"
            "nMembersProps = AMEMBERS(aMembersProps, oCreate, 1)\n"
            "nMembersUnion = AMEMBERS(aMembersUnion, oCreate, 3)\n"
            "cProp1 = aMembersProps[1]\n"
            "cProp2 = aMembersProps[2]\n"
            "cProp4 = aMembersProps[4]\n"
            "RETURN\n"
            "DEFINE CLASS ChildWidget AS ParentWidget OF widgetlib.prg\n"
            "    cBacking = 'Child'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external-base accessor reflection script should complete: ") + state.message +
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

        check("ccaptionbefore", "Child:A");
        check("ccaptionafter", "Set:S:A");
        check("cbacking", "Set:S");
        check("nassigncount", "1");
        check("cgetcaption", "Set:S:A");
        check("cgetstatus", "Ready:R");
        check("lhascaption", "true");
        check("lhasstatus", "true");
        check("lcaptionreadonly", "false");
        check("lstatusreadonly", "true");
        check("nmembersprops", "9");
        check("nmembersunion", "19");
        check("cprop1", "BASECLASS");
        check("cprop2", "CAPTION");
        check("cprop4", "CLASS");
        check("ldictset", "true");
        check("ndictcompare", "21");

        expect(state.ole_objects.size() == 2U,
               "external-base accessor reflection script should register native and COM objects");
        if (state.ole_objects.size() == 2U)
        {
            const auto &native_object = state.ole_objects[0];
            expect(native_object.prog_id == "ChildWidget",
                   "external-base accessor reflection should preserve child class identity");
            const auto backing = native_object.properties.find("cbacking");
            const auto assign_count = native_object.properties.find("nassigncount");
            if (backing != native_object.properties.end())
            {
                expect(copperfin::runtime::format_value(backing->second) == "Set:S",
                       "external-base accessor reflection should preserve inherited assign-side backing state");
            }
            else
            {
                expect(false, "external-base accessor reflection should materialize inherited assign-side backing state");
            }
            if (assign_count != native_object.properties.end())
            {
                expect(copperfin::runtime::format_value(assign_count->second) == "1",
                       "external-base accessor reflection should preserve inherited assign-count state");
            }
            else
            {
                expect(false, "external-base accessor reflection should materialize inherited assign-count state");
            }
            expect(state.ole_objects[1].prog_id == "Scripting.Dictionary",
                   "COM reflection should remain stable while external-base accessor coverage lands");
        }

        const bool has_accessor_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   (event.detail == "ParentWidget.Caption_Access" ||
                    event.detail == "ParentWidget.Caption_Assign" ||
                    event.detail == "ParentWidget.Status_Access");
        });
        expect(has_accessor_invoke_event,
               "external-base accessor reflection should emit inherited accessor-method invoke events");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_accessor_backed_properties_setpem_routes_through_assign_methods()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_accessor_setpem";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_accessor_setpem.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildWidget')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lSetCaption = SETPEM(oCreate, 'Caption', 'Renamed')\n"
            "cBackingAfterAssign = oCreate.cBacking\n"
            "cCaptionAfterAssign = oCreate.Caption\n"
            "cGetCaptionAfterAssign = GETPEM(oCreate, 'Caption')\n"
            "lSetStatus = SETPEM(oCreate, 'Status', 'Blocked')\n"
            "cStatusAfterFailedSet = GETPEM(oCreate, 'Status')\n"
            "lSetRawBacking = SETPEM(oCreate, 'cBacking', 'Direct')\n"
            "cBackingAfterRawSet = oCreate.cBacking\n"
            "cCaptionAfterRawSet = oCreate.Caption\n"
            "lSetDict = SETPEM(oDict, 'comparemode', 11)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ParentWidget AS Custom\n"
            "    cBacking = 'Parent'\n"
            "    cStatusBacking = 'Ready'\n"
            "    FUNCTION Caption_Access\n"
            "        RETURN THIS.cBacking + ':A'\n"
            "    ENDFUNC\n"
            "    PROCEDURE Caption_Assign\n"
            "        LPARAMETERS tcValue\n"
            "        THIS.cBacking = tcValue + ':S'\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "    FUNCTION Status_Access\n"
            "        RETURN THIS.cStatusBacking + ':R'\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ChildWidget AS ParentWidget\n"
            "    cBacking = 'Child'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native accessor SETPEM script should complete: ") + state.message +
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

        check("lsetcaption", "true");
        check("cbackingafterassign", "Renamed:S");
        check("ccaptionafterassign", "Renamed:S:A");
        check("cgetcaptionafterassign", "Renamed:S:A");
        check("lsetstatus", "false");
        check("cstatusafterfailedset", "Ready:R");
        check("lsetrawbacking", "true");
        check("cbackingafterrawset", "Direct");
        check("ccaptionafterrawset", "Direct:A");
        check("lsetdict", "true");
        check("ndictcompare", "11");

        expect(state.ole_objects.size() == 2U,
               "native accessor SETPEM script should register native and COM objects");
        if (state.ole_objects.size() == 2U)
        {
            expect(state.ole_objects[0].prog_id == "ChildWidget",
                   "native accessor SETPEM should preserve child class identity");
            expect(state.ole_objects[1].prog_id == "Scripting.Dictionary",
                   "COM object SETPEM behavior should remain stable while native assigner routing lands");

            const auto backing = state.ole_objects[0].properties.find("cbacking");
            const auto status = state.ole_objects[0].properties.find("cstatusbacking");
            if (backing != state.ole_objects[0].properties.end())
            {
                expect(copperfin::runtime::format_value(backing->second) == "Direct",
                       "native accessor SETPEM should leave final raw backing state visible");
            }
            else
            {
                expect(false, "native accessor SETPEM should preserve the raw backing property");
            }
            if (status != state.ole_objects[0].properties.end())
            {
                expect(copperfin::runtime::format_value(status->second) == "Ready",
                       "native accessor SETPEM should not mutate access-only backing state");
            }
            else
            {
                expect(false, "native accessor SETPEM should preserve the access-only backing property");
            }
        }

        const bool has_assign_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "ParentWidget.Caption_Assign";
        });
        expect(has_assign_invoke_event,
               "native accessor SETPEM should emit native assigner invoke events");

        fs::remove_all(temp_root, ignored);
    }

    void test_external_prg_base_accessor_backed_properties_setpem_routes_through_assign_methods()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_prg_accessor_setpem";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path library_path = temp_root / "widgetlib.prg";
        write_text(
            library_path,
            "DEFINE CLASS ParentWidget AS Custom\n"
            "    cBacking = 'Parent'\n"
            "    cStatusBacking = 'Ready'\n"
            "    FUNCTION Caption_Access\n"
            "        RETURN THIS.cBacking + ':A'\n"
            "    ENDFUNC\n"
            "    PROCEDURE Caption_Assign\n"
            "        LPARAMETERS tcValue\n"
            "        THIS.cBacking = tcValue + ':S'\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "    FUNCTION Status_Access\n"
            "        RETURN THIS.cStatusBacking + ':R'\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_accessor_setpem.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildWidget')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lSetCaption = SETPEM(oCreate, 'Caption', 'Renamed')\n"
            "cBackingAfterAssign = oCreate.cBacking\n"
            "cCaptionAfterAssign = oCreate.Caption\n"
            "cGetCaptionAfterAssign = GETPEM(oCreate, 'Caption')\n"
            "lSetStatus = SETPEM(oCreate, 'Status', 'Blocked')\n"
            "cStatusAfterFailedSet = GETPEM(oCreate, 'Status')\n"
            "lSetRawBacking = SETPEM(oCreate, 'cBacking', 'Direct')\n"
            "cBackingAfterRawSet = oCreate.cBacking\n"
            "cCaptionAfterRawSet = oCreate.Caption\n"
            "lSetDict = SETPEM(oDict, 'comparemode', 22)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ChildWidget AS ParentWidget OF widgetlib.prg\n"
            "    cBacking = 'Child'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external-base accessor SETPEM script should complete: ") + state.message +
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

        check("lsetcaption", "true");
        check("cbackingafterassign", "Renamed:S");
        check("ccaptionafterassign", "Renamed:S:A");
        check("cgetcaptionafterassign", "Renamed:S:A");
        check("lsetstatus", "false");
        check("cstatusafterfailedset", "Ready:R");
        check("lsetrawbacking", "true");
        check("cbackingafterrawset", "Direct");
        check("ccaptionafterrawset", "Direct:A");
        check("lsetdict", "true");
        check("ndictcompare", "22");

        expect(state.ole_objects.size() == 2U,
               "external-base accessor SETPEM script should register native and COM objects");
        if (state.ole_objects.size() == 2U)
        {
            expect(state.ole_objects[0].prog_id == "ChildWidget",
                   "external-base accessor SETPEM should preserve child class identity");
            expect(state.ole_objects[1].prog_id == "Scripting.Dictionary",
                   "COM object SETPEM behavior should remain stable while external-base assigner routing lands");

            const auto backing = state.ole_objects[0].properties.find("cbacking");
            const auto status = state.ole_objects[0].properties.find("cstatusbacking");
            if (backing != state.ole_objects[0].properties.end())
            {
                expect(copperfin::runtime::format_value(backing->second) == "Direct",
                       "external-base accessor SETPEM should leave final raw backing state visible");
            }
            else
            {
                expect(false, "external-base accessor SETPEM should preserve the raw backing property");
            }
            if (status != state.ole_objects[0].properties.end())
            {
                expect(copperfin::runtime::format_value(status->second) == "Ready",
                       "external-base accessor SETPEM should not mutate inherited access-only backing state");
            }
            else
            {
                expect(false, "external-base accessor SETPEM should preserve the inherited access-only backing property");
            }
        }

        const bool has_assign_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "ParentWidget.Caption_Assign";
        });
        expect(has_assign_invoke_event,
               "external-base accessor SETPEM should emit inherited assigner invoke events");

        fs::remove_all(temp_root, ignored);
    }

}
