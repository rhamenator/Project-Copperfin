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


    // #6415: a property-read event handler (or _Access method) that releases
    // the source object used to leave the read dereferencing erased object
    // state. The read must fail catchably (error 1924) or, when the value was
    // already produced, return it; later handlers must not run on a freed source.
    void test_property_read_handler_releasing_source_fails_catchably()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_property_read_release_6415";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const auto global_text = [](const auto &state, const std::string &name) -> std::string {
            const auto found = state.globals.find(name);
            return found == state.globals.end() ? std::string("<missing>") : copperfin::runtime::format_value(found->second);
        };
        const fs::path main_path = temp_root / "property_read_release.prg";
        write_text(
            main_path,
            "PUBLIC oSource, oForm, nFirst, nSecond, nAfter\n"
            "nFirst = 0\n"
            "nSecond = 0\n"
            "nAfter = 0\n"
            "* 1: issue repro plus a second before-handler that must not run\n"
            "oSource = CREATEOBJECT('SourceThing')\n"
            "nBind1 = BINDEVENT(oSource, 'Caption', 'ReleaseSource', 1)\n"
            "nBind2 = BINDEVENT(oSource, 'Caption', 'SecondHandler', 1)\n"
            "nErrBefore = 0\n"
            "TRY\n"
            "    cBefore = oSource.Caption\n"
            "CATCH TO oErr\n"
            "    nErrBefore = oErr.ErrorNo\n"
            "ENDTRY\n"
            "* 2: an after-handler releases the source once the value is read\n"
            "oSource = CREATEOBJECT('SourceThing')\n"
            "nBind3 = BINDEVENT(oSource, 'Caption', 'AfterRelease', 0)\n"
            "nErrAfter = 0\n"
            "cAfter = ''\n"
            "TRY\n"
            "    cAfter = oSource.Caption\n"
            "CATCH TO oErr\n"
            "    nErrAfter = oErr.ErrorNo\n"
            "ENDTRY\n"
            "* 3: an _Access method releases its own object\n"
            "oSource = CREATEOBJECT('SelfReleasingAccess')\n"
            "nErrAccess = 0\n"
            "cAccess = ''\n"
            "TRY\n"
            "    cAccess = oSource.Caption\n"
            "CATCH TO oErr\n"
            "    nErrAccess = oErr.ErrorNo\n"
            "ENDTRY\n"
            "* 4: the handler releases the source's container\n"
            "oForm = CREATEOBJECT('HostForm')\n"
            "nBind4 = BINDEVENT(oForm.lblChild, 'Caption', 'ReleaseForm', 1)\n"
            "nErrContainer = 0\n"
            "TRY\n"
            "    cContainer = oForm.lblChild.Caption\n"
            "CATCH TO oErr\n"
            "    nErrContainer = oErr.ErrorNo\n"
            "ENDTRY\n"
            "* 5: a List() argument UDF releases the list before dispatch\n"
            "PUBLIC oList, nSelector\n"
            "nSelector = 0\n"
            "oList = CREATEOBJECT('ListBox')\n"
            "oList.AddItem('first')\n"
            "oList.AddItem('second')\n"
            "nErrSelector = 0\n"
            "cSelector = ''\n"
            "TRY\n"
            "    cSelector = oList.List(ReleaseList())\n"
            "CATCH TO oErr\n"
            "    nErrSelector = oErr.ErrorNo\n"
            "ENDTRY\n"
            "lAfter = .T.\n"
            "RETURN\n"
            "FUNCTION ReleaseList\n"
            "    nSelector = nSelector + 1\n"
            "    RELEASE oList\n"
            "    RETURN 1\n"
            "ENDFUNC\n"
            "PROCEDURE ReleaseSource\n"
            "    nFirst = nFirst + 1\n"
            "    oSource.Release()\n"
            "ENDPROC\n"
            "PROCEDURE SecondHandler\n"
            "    nSecond = nSecond + 1\n"
            "ENDPROC\n"
            "PROCEDURE AfterRelease\n"
            "    nAfter = nAfter + 1\n"
            "    oSource.Release()\n"
            "ENDPROC\n"
            "PROCEDURE ReleaseForm\n"
            "    oForm.Release()\n"
            "ENDPROC\n"
            "DEFINE CLASS SourceThing AS Custom\n"
            "    Caption = 'alive'\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SelfReleasingAccess AS Custom\n"
            "    Caption = 'alive'\n"
            "    PROCEDURE Caption_Access\n"
            "        THIS.Release()\n"
            "        RETURN 'accessed'\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS HostForm AS Form\n"
            "    ADD OBJECT lblChild AS Label WITH Caption = 'child'\n"
            "ENDDEFINE\n");

        auto session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string(), false));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "#6415: script should complete: " + state.message);
        expect(global_text(state, "lafter") == "true", "#6415: execution should continue after every read");
        expect(global_text(state, "nfirst") == "1", "#6415: the releasing before-handler should run once");
        expect(global_text(state, "nerrbefore") == "1924",
               "#6415: reading after a before-handler released the source should raise error 1924, got: " +
                   global_text(state, "nerrbefore"));
        expect(global_text(state, "nsecond") == "0",
               "#6415: a later handler must not run on a released source, got: " + global_text(state, "nsecond"));
        // #6538 review: once the source is gone the read never returns
        // normally, even with a value in hand, because callers still hold the
        // erased object reference.
        expect(global_text(state, "nafter") == "1" && global_text(state, "nerrafter") == "1924",
               "#6415: an after-handler releasing the source should raise error 1924, got: " +
                   global_text(state, "cafter") + " / error " + global_text(state, "nerrafter"));
        expect(global_text(state, "nerraccess") == "1924",
               "#6415: an _Access method that releases THIS should raise error 1924, got: " +
                   global_text(state, "caccess") + " / error " + global_text(state, "nerraccess"));
        // Direct List(<expr>) syntax evaluates its argument before dispatch,
        // so a releasing argument surfaces as the OLE "object not found for
        // method invocation" fault (1429) rather than reaching the
        // selector-text branch with an erased source.
        expect(global_text(state, "nerrselector") == "1429" && global_text(state, "nselector") == "1",
               "#6415: a List() argument releasing the list should fail catchably (1429), got: " +
                   global_text(state, "cselector") + " / error " + global_text(state, "nerrselector") +
                   " / selector calls " + global_text(state, "nselector"));
        // Copperfin may keep a released form's children alive until the
        // container is torn down, so either outcome is memory-safe; what
        // matters (checked under ASan) is never reading erased state.
        expect(global_text(state, "nerrcontainer") == "1924" ||
                   (global_text(state, "nerrcontainer") == "0" && global_text(state, "ccontainer") == "child"),
               "#6415: releasing the source's container must fail with 1924 or read the still-live child, got: " +
                   global_text(state, "ccontainer") + " / error " + global_text(state, "nerrcontainer"));

        fs::remove_all(temp_root, ignored);
    }


    // #6420: a BINDEVENT before-handler that releases the source object used
    // to leave method dispatch calling the source method body through erased
    // object state. It must fail catchably (1924) without running the body or
    // later before-handlers; after-handlers may still release the source
    // (RQ-CF-PRG-036) and the method result is returned.
    void test_method_before_handler_releasing_source_fails_catchably()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_method_bindevent_release_6420";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const auto global_text = [](const auto &state, const std::string &name) -> std::string {
            const auto found = state.globals.find(name);
            return found == state.globals.end() ? std::string("<missing>") : copperfin::runtime::format_value(found->second);
        };
        const fs::path main_path = temp_root / "method_bindevent_release.prg";
        write_text(
            main_path,
            "PUBLIC oSource, oHandler, nPings, nSecond, nAfter\n"
            "nPings = 0\n"
            "nSecond = 0\n"
            "nAfter = 0\n"
            "* 1: issue repro (object handler) plus a second before-handler\n"
            "oSource = CREATEOBJECT('SourceThing')\n"
            "oHandler = CREATEOBJECT('HandlerThing')\n"
            "nBind1 = BINDEVENT(oSource, 'Ping', oHandler, 'ReleaseSource', 1)\n"
            "nBind2 = BINDEVENT(oSource, 'Ping', oHandler, 'SecondHandler', 1)\n"
            "nErrBefore = 0\n"
            "xBefore = ''\n"
            "TRY\n"
            "    xBefore = oSource.Ping()\n"
            "CATCH TO oErr\n"
            "    nErrBefore = oErr.ErrorNo\n"
            "ENDTRY\n"
            "nPingsAfterBefore = nPings\n"
            "* 2: an after-handler releases the source once the method ran\n"
            "oSource = CREATEOBJECT('SourceThing')\n"
            "nBind3 = BINDEVENT(oSource, 'Ping', oHandler, 'AfterRelease', 0)\n"
            "nErrAfter = 0\n"
            "xAfter = ''\n"
            "TRY\n"
            "    xAfter = oSource.Ping()\n"
            "CATCH TO oErr\n"
            "    nErrAfter = oErr.ErrorNo\n"
            "ENDTRY\n"
            "* 3 (#6540 review): the source method releases itself\n"
            "oSelf = CREATEOBJECT('SelfReleasing')\n"
            "nBind4 = BINDEVENT(oSelf, 'Ping', oHandler, 'NoopHandler', 1)\n"
            "nErrSelf = 0\n"
            "xSelf = ''\n"
            "TRY\n"
            "    xSelf = oSelf.Ping()\n"
            "CATCH TO oErr\n"
            "    nErrSelf = oErr.ErrorNo\n"
            "ENDTRY\n"
            "* 4: the before-handler releases the source and creates a replacement\n"
            "nPings = 0\n"
            "oSource = CREATEOBJECT('SourceThing')\n"
            "nBind5 = BINDEVENT(oSource, 'Ping', oHandler, 'ReplaceSource', 1)\n"
            "nErrReplace = 0\n"
            "TRY\n"
            "    xReplace = oSource.Ping()\n"
            "CATCH TO oErr\n"
            "    nErrReplace = oErr.ErrorNo\n"
            "ENDTRY\n"
            "nPingsAfterReplace = nPings\n"
            "* 5: the before-handler unbinds itself, then releases the source\n"
            "nPings = 0\n"
            "oSource = CREATEOBJECT('SourceThing')\n"
            "nBind6 = BINDEVENT(oSource, 'Ping', oHandler, 'UnbindThenRelease', 1)\n"
            "nErrUnbind = 0\n"
            "TRY\n"
            "    xUnbind = oSource.Ping()\n"
            "CATCH TO oErr\n"
            "    nErrUnbind = oErr.ErrorNo\n"
            "ENDTRY\n"
            "nPingsAfterUnbind = nPings\n"
            "* 6: the before-handler removes the source from its container\n"
            "PUBLIC oBox\n"
            "nPings = 0\n"
            "oBox = CREATEOBJECT('BoxThing')\n"
            "nBind7 = BINDEVENT(oBox.oChild, 'Ping', oHandler, 'RemoveChild', 1)\n"
            "nErrBox = 0\n"
            "xBox = ''\n"
            "TRY\n"
            "    xBox = oBox.oChild.Ping()\n"
            "CATCH TO oErr\n"
            "    nErrBox = oErr.ErrorNo\n"
            "ENDTRY\n"
            "lAfter = .T.\n"
            "RETURN\n"
            "DEFINE CLASS SourceThing AS Custom\n"
            "    PROCEDURE Ping\n"
            "        nPings = nPings + 1\n"
            "        RETURN 'pong'\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS HandlerThing AS Custom\n"
            "    PROCEDURE ReleaseSource\n"
            "        oSource.Release()\n"
            "    ENDPROC\n"
            "    PROCEDURE SecondHandler\n"
            "        nSecond = nSecond + 1\n"
            "    ENDPROC\n"
            "    PROCEDURE AfterRelease\n"
            "        nAfter = nAfter + 1\n"
            "        oSource.Release()\n"
            "    ENDPROC\n"
            "    PROCEDURE NoopHandler\n"
            "    ENDPROC\n"
            "    PROCEDURE ReplaceSource\n"
            "        oSource.Release()\n"
            "        oSource = CREATEOBJECT('SourceThing')\n"
            "    ENDPROC\n"
            "    PROCEDURE UnbindThenRelease\n"
            "        UNBINDEVENTS(oSource, 'Ping', oHandler, 'UnbindThenRelease')\n"
            "        oSource.Release()\n"
            "    ENDPROC\n"
            "    PROCEDURE RemoveChild\n"
            "        oBox.RemoveObject('oChild')\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SelfReleasing AS Custom\n"
            "    PROCEDURE Ping\n"
            "        THIS.Release()\n"
            "        RETURN 'gone'\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS BoxThing AS Container\n"
            "    ADD OBJECT oChild AS SourceThing\n"
            "ENDDEFINE\n");

        auto session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string(), false));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "#6420: script should complete: " + state.message);
        expect(global_text(state, "lafter") == "true", "#6420: execution should continue after both calls");
        expect(global_text(state, "nerrbefore") == "1924",
               "#6420: a before-handler releasing the source should raise error 1924, got: " +
                   global_text(state, "nerrbefore"));
        expect(global_text(state, "npingsafterbefore") == "0",
               "#6420: the source method body must not run after its source was released, got: " +
                   global_text(state, "npingsafterbefore"));
        expect(global_text(state, "nsecond") == "0",
               "#6420: a later before-handler must not run on a released source, got: " + global_text(state, "nsecond"));
        expect(global_text(state, "nafter") == "1" && global_text(state, "nerrafter") == "0" &&
                   global_text(state, "xafter") == "pong",
               "#6420: an after-handler may release the source and the method result is returned, got: " +
                   global_text(state, "xafter") + " / error " + global_text(state, "nerrafter"));
        expect(global_text(state, "nerrself") == "0" && global_text(state, "xself") == "gone",
               "#6420: a source method that releases itself still returns its result (RQ-CF-PRG-036), got: " +
                   global_text(state, "xself") + " / error " + global_text(state, "nerrself"));
        expect(global_text(state, "nerrreplace") == "1924" && global_text(state, "npingsafterreplace") == "0",
               "#6420: release-then-create must not dispatch to the replacement object, got error " +
                   global_text(state, "nerrreplace") + " / pings " + global_text(state, "npingsafterreplace"));
        expect(global_text(state, "nerrunbind") == "1924" && global_text(state, "npingsafterunbind") == "0",
               "#6420: a handler that unbinds itself before releasing should still raise 1924, got error " +
                   global_text(state, "nerrunbind") + " / pings " + global_text(state, "npingsafterunbind"));
        // Copperfin may keep a removed child alive, so either outcome is
        // memory-safe (checked under ASan); dispatching through erased state
        // is not.
        expect(global_text(state, "nerrbox") == "1924" ||
                   (global_text(state, "nerrbox") == "0" && global_text(state, "xbox") == "pong"),
               "#6420: container removal must fail with 1924 or run on the still-live child, got: " +
                   global_text(state, "xbox") + " / error " + global_text(state, "nerrbox"));

        fs::remove_all(temp_root, ignored);
    }

}
