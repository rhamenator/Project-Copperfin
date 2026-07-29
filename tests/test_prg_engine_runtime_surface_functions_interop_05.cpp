#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_external_prg_base_property_bindevent_dispatch_preserves_current_event_metadata()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_prg_property_bindevent";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path library_path = temp_root / "widgetlib.prg";
        write_text(
            library_path,
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
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_property_bindevent.prg";
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
            "DEFINE CLASS ChildWidget AS ParentWidget OF widgetlib.prg\n"
            "    cBacking = 'Child'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external-base property BINDEVENT script should complete: ") + state.message +
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

        expect(state.ole_objects.size() == 1U,
               "external-base property BINDEVENT script should register one native object");
        if (state.ole_objects.size() == 1U)
        {
            const auto &native_object = state.ole_objects[0];
            expect(native_object.prog_id == "ChildWidget",
                   "external-base property BINDEVENT should preserve child class identity");
            expect(native_object.class_library == library_path.string(),
                   "external-base property BINDEVENT should preserve external base class-library provenance");
        }

        const bool has_delegate_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.event.delegate" &&
                   (event.detail == "caption -> HandleCaption" ||
                    event.detail == "nraw -> HandleRaw");
        });
        expect(has_delegate_event,
               "external-base property BINDEVENT dispatch should emit delegate events");

        fs::remove_all(temp_root, ignored);
    }

    void test_same_prg_property_bindevent_object_method_delegates_preserve_current_event_metadata()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_property_bindevent_object_delegate";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "property_bindevent_object_delegate.prg";
        write_text(
            main_path,
            "cLog = ''\n"
            "oCreate = CREATEOBJECT('ChildWidget')\n"
            "oHandler = CREATEOBJECT('HandlerWidget')\n"
            "nBindCaption = BINDEVENT(oCreate, 'Caption', oHandler, 'HandleCaption')\n"
            "nBindRaw = BINDEVENT(oCreate, 'nRaw', oHandler, 'HandleRaw', 1)\n"
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
            "nCaptionCalls = oHandler.nCaptionCalls\n"
            "nRawCalls = oHandler.nRawCalls\n"
            "nCaptionFirstRows = oHandler.nCaptionFirstRows\n"
            "lCaptionFirstSource = oHandler.lCaptionFirstSource\n"
            "nRawFirstRows = oHandler.nRawFirstRows\n"
            "lRawFirstSource = oHandler.lRawFirstSource\n"
            "cHandlerLog = oHandler.cLog\n"
            "RETURN\n"
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
            "ENDDEFINE\n"
            "DEFINE CLASS HandlerWidget AS Custom\n"
            "    nCaptionCalls = 0\n"
            "    nRawCalls = 0\n"
            "    nCaptionFirstRows = 0\n"
            "    lCaptionFirstSource = .F.\n"
            "    nRawFirstRows = 0\n"
            "    lRawFirstSource = .F.\n"
            "    cLog = ''\n"
            "    FUNCTION HandleCaption\n"
            "        LPARAMETERS tuValue\n"
            "        THIS.nCaptionCalls = THIS.nCaptionCalls + 1\n"
            "        nRows = AEVENTS(aCurrent, 0)\n"
            "        THIS.cLog = THIS.cLog + '[caption:' + aCurrent[2] + ':' + TRANSFORM(aCurrent[3]) + ':' + TRANSFORM(PCOUNT()) + ':' + IIF(PCOUNT() = 0, 'none', TRANSFORM(tuValue)) + ']'\n"
            "        IF THIS.nCaptionCalls = 1\n"
            "            THIS.nCaptionFirstRows = nRows\n"
            "            THIS.lCaptionFirstSource = COMPOBJ(aCurrent[1], oCreate)\n"
            "        ENDIF\n"
            "        RETURN\n"
            "    ENDFUNC\n"
            "    FUNCTION HandleRaw\n"
            "        LPARAMETERS tuValue\n"
            "        THIS.nRawCalls = THIS.nRawCalls + 1\n"
            "        nRows = AEVENTS(aCurrentRaw, 0)\n"
            "        THIS.cLog = THIS.cLog + '[raw:' + aCurrentRaw[2] + ':' + TRANSFORM(aCurrentRaw[3]) + ':' + TRANSFORM(PCOUNT()) + ':' + IIF(PCOUNT() = 0, 'none', TRANSFORM(tuValue)) + ']'\n"
            "        IF THIS.nRawCalls = 1\n"
            "            THIS.nRawFirstRows = nRows\n"
            "            THIS.lRawFirstSource = COMPOBJ(aCurrentRaw[1], oCreate)\n"
            "        ENDIF\n"
            "        RETURN\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("property BINDEVENT object-delegate script should complete: ") + state.message +
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
        check("chandlerlog",
              "[caption:caption:2:0:none][caption:caption:2:1:Set][caption:caption:2:0:none][raw:nraw:2:0:none][raw:nraw:2:1:9][raw:nraw:2:0:none][caption:caption:2:0:none][caption:caption:2:1:PemSet][caption:caption:2:0:none][raw:nraw:2:1:11][raw:nraw:2:0:none]");

        expect(state.ole_objects.size() == 2U,
               "property BINDEVENT object-delegate script should register source and handler objects");
        if (state.ole_objects.size() == 2U)
        {
            expect(state.ole_objects[0].prog_id == "ChildWidget",
                   "property BINDEVENT object-delegate should preserve source class identity");
            expect(state.ole_objects[1].prog_id == "HandlerWidget",
                   "property BINDEVENT object-delegate should preserve handler class identity");
        }

        const bool has_delegate_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.event.delegate" &&
                   (event.detail == "caption -> HandlerWidget.HandleCaption" ||
                    event.detail == "nraw -> HandlerWidget.HandleRaw");
        });
        expect(has_delegate_event,
               "property BINDEVENT object-method delegates should emit delegate events");

        fs::remove_all(temp_root, ignored);
    }

    void test_external_prg_base_property_bindevent_object_method_delegates_preserve_current_event_metadata()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_prg_property_bindevent_object_delegate";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path handler_library_path = temp_root / "handlerlib.prg";
        write_text(
            handler_library_path,
            "DEFINE CLASS ParentHandler AS Custom\n"
            "    nCaptionCalls = 0\n"
            "    nRawCalls = 0\n"
            "    nCaptionFirstRows = 0\n"
            "    lCaptionFirstSource = .F.\n"
            "    nRawFirstRows = 0\n"
            "    lRawFirstSource = .F.\n"
            "    cLog = ''\n"
            "    FUNCTION HandleCaption\n"
            "        LPARAMETERS tuValue\n"
            "        THIS.nCaptionCalls = THIS.nCaptionCalls + 1\n"
            "        nRows = AEVENTS(aCurrent, 0)\n"
            "        THIS.cLog = THIS.cLog + '[caption:' + aCurrent[2] + ':' + TRANSFORM(aCurrent[3]) + ':' + TRANSFORM(PCOUNT()) + ':' + IIF(PCOUNT() = 0, 'none', TRANSFORM(tuValue)) + ']'\n"
            "        IF THIS.nCaptionCalls = 1\n"
            "            THIS.nCaptionFirstRows = nRows\n"
            "            THIS.lCaptionFirstSource = COMPOBJ(aCurrent[1], oCreate)\n"
            "        ENDIF\n"
            "        RETURN\n"
            "    ENDFUNC\n"
            "    FUNCTION HandleRaw\n"
            "        LPARAMETERS tuValue\n"
            "        THIS.nRawCalls = THIS.nRawCalls + 1\n"
            "        nRows = AEVENTS(aCurrentRaw, 0)\n"
            "        THIS.cLog = THIS.cLog + '[raw:' + aCurrentRaw[2] + ':' + TRANSFORM(aCurrentRaw[3]) + ':' + TRANSFORM(PCOUNT()) + ':' + IIF(PCOUNT() = 0, 'none', TRANSFORM(tuValue)) + ']'\n"
            "        IF THIS.nRawCalls = 1\n"
            "            THIS.nRawFirstRows = nRows\n"
            "            THIS.lRawFirstSource = COMPOBJ(aCurrentRaw[1], oCreate)\n"
            "        ENDIF\n"
            "        RETURN\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_property_bindevent_object_delegate.prg";
        write_text(
            main_path,
            "cLog = ''\n"
            "oCreate = CREATEOBJECT('ChildWidget')\n"
            "oHandler = CREATEOBJECT('HandlerWidget')\n"
            "nBindCaption = BINDEVENT(oCreate, 'Caption', oHandler, 'HandleCaption')\n"
            "nBindRaw = BINDEVENT(oCreate, 'nRaw', oHandler, 'HandleRaw', 1)\n"
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
            "nCaptionCalls = oHandler.nCaptionCalls\n"
            "nRawCalls = oHandler.nRawCalls\n"
            "nCaptionFirstRows = oHandler.nCaptionFirstRows\n"
            "lCaptionFirstSource = oHandler.lCaptionFirstSource\n"
            "nRawFirstRows = oHandler.nRawFirstRows\n"
            "lRawFirstSource = oHandler.lRawFirstSource\n"
            "cHandlerLog = oHandler.cLog\n"
            "RETURN\n"
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
            "ENDDEFINE\n"
            "DEFINE CLASS HandlerWidget AS ParentHandler OF handlerlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external-base property BINDEVENT object-delegate script should complete: ") + state.message +
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
        check("chandlerlog",
              "[caption:caption:2:0:none][caption:caption:2:1:Set][caption:caption:2:0:none][raw:nraw:2:0:none][raw:nraw:2:1:9][raw:nraw:2:0:none][caption:caption:2:0:none][caption:caption:2:1:PemSet][caption:caption:2:0:none][raw:nraw:2:1:11][raw:nraw:2:0:none]");

        expect(state.ole_objects.size() == 2U,
               "external-base property BINDEVENT object-delegate script should register source and handler objects");
        if (state.ole_objects.size() == 2U)
        {
            expect(state.ole_objects[0].prog_id == "ChildWidget",
                   "external-base property BINDEVENT object-delegate should preserve source class identity");
            expect(state.ole_objects[1].prog_id == "HandlerWidget",
                   "external-base property BINDEVENT object-delegate should preserve handler class identity");
            expect(state.ole_objects[1].class_library == handler_library_path.string(),
                   "external-base property BINDEVENT object-delegate should preserve external handler class-library provenance");
        }

        const bool has_delegate_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.event.delegate" &&
                   (event.detail == "caption -> ParentHandler.HandleCaption" ||
                    event.detail == "nraw -> ParentHandler.HandleRaw");
        });
        expect(has_delegate_event,
               "external-base property BINDEVENT object-method delegates should emit inherited delegate events");

        fs::remove_all(temp_root, ignored);
    }

    void test_codepage_and_misc_runtime_surface_functions()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_codepage_misc";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "cpmisc.prg";
        write_text(
            main_path,
            // CPCURRENT
            "nCpCurrent      = CPCURRENT()\n"
            "nCpCurrentAnsi  = CPCURRENT(0)\n"
            "nCpCurrentOs    = CPCURRENT(1)\n"
            "nCpCurrentUni   = CPCURRENT(2)\n"
            "nCpCurrentBad   = CPCURRENT(99)\n"
            "nCpConfiguredBefore = CPCURRENT()\n"
            // VFP9 documents CODEPAGE only in CONFIG.FPW; this command must
            // not mutate the startup configuration value.
            "SET CODEPAGE TO 1251\n"
            "nCpConfigured    = CPCURRENT()\n"
            "nCpConfiguredZero = CPCURRENT(0)\n"
            "nCpConfiguredHost = CPCURRENT(1)\n"
            "cCpConfiguredSet = SET('CODEPAGE')\n"
            "SET DATASESSION TO 2\n"
            "nCpSessionTwoDefault = CPCURRENT()\n"
            "SET CODEPAGE TO 932\n"
            "nCpSessionTwoAfterSet = CPCURRENT()\n"
            "SET DATASESSION TO 1\n"
            "nCpSessionOneRestored = CPCURRENT()\n"
            "SET CODEPAGE TO 99999\n"
            "nCpInvalidRetains = CPCURRENT()\n"
            // CPCONVERT identity pass-through
            "cCpConverted    = CPCONVERT(1252, 1252, 'hello')\n"
            // CPDBF with no open work area should fall back to 0
            "nCpDbf          = CPDBF()\n"
            // GETPICT headless contract
            "cPict           = GETPICT('Select Image', 'current.bmp')\n"
            // GETCOLOR headless contract
            "nColor          = GETCOLOR(255, 'Pick Accent')\n"
            // GETFONT headless contract
            "cFont           = GETFONT('Arial', 12, 'B')\n"
            // VARREAD headless contract
            "cVarRead        = VARREAD()\n"
            // NEWID unique identifiers
            "cId1            = NEWID()\n"
            "cId2            = NEWID()\n"
            "lIdsDistinct    = cId1 <> cId2\n"
            "nIdLen          = LEN(cId1)\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "codepage/misc script should complete");

        const auto check = [&](const std::string &name, const std::string &expected)
        {
            const auto it = state.globals.find(name);
            if (it == state.globals.end())
            {
                expect(false, name + " variable not found");
                return;
            }
            const std::string actual = copperfin::runtime::format_value(it->second);
            expect(actual == expected, name + ": expected \"" + expected + "\", got \"" + actual + "\"");
        };

        const std::string expected_host_code_page_text = std::to_string(expected_host_code_page());
        const std::string expected_host_oem_code_page_text = std::to_string(expected_host_oem_code_page());

        check("ncpcurrent",     expected_host_code_page_text);
        check("ncpcurrentansi", expected_host_code_page_text);
        check("ncpcurrentos",   expected_host_code_page_text);
        check("ncpcurrentuni",  expected_host_oem_code_page_text);
        check("ncpcurrentbad",  expected_host_code_page_text);
        check("ncpconfiguredbefore", expected_host_code_page_text);
        check("ncpconfigured", expected_host_code_page_text);
        check("ncpconfiguredzero", expected_host_code_page_text);
        check("ncpconfiguredhost", expected_host_code_page_text);
        check("ccpconfiguredset", expected_host_code_page_text);
        check("ncpsessiontwodefault", expected_host_code_page_text);
        check("ncpsessiontwoafterset", expected_host_code_page_text);
        check("ncpsessiononerestored", expected_host_code_page_text);
        check("ncpinvalidretains", expected_host_code_page_text);
        check("ccpconverted",   "hello");
        check("ncpdbf",         "0");
        check("cpict",          "current.bmp");
        check("ncolor",         "255");
        check("cfont",          "Arial");
        check("cvarread",       "");
        check("lidsdistinct",   "true");
        // UUID: 8-4-4-4-12 hex = 36 characters
        check("nidlen", "36");

        const auto find_event = [&](const std::string& category) -> const copperfin::runtime::RuntimeEvent* {
            for (const auto& event : state.events) {
                if (event.category == category) {
                    return &event;
                }
            }
            return nullptr;
        };

        const auto* getpict_event = find_event("runtime.getpict");
        const auto* getcolor_event = find_event("runtime.getcolor");
        const auto* getfont_event = find_event("runtime.getfont");
        const auto* varread_event = find_event("runtime.varread");
        expect(getpict_event != nullptr, "GETPICT function should emit a runtime.getpict event");
        expect(getcolor_event != nullptr, "GETCOLOR function should emit a runtime.getcolor event");
        expect(getfont_event != nullptr, "GETFONT function should emit a runtime.getfont event");
        expect(varread_event != nullptr, "VARREAD function should emit a runtime.varread event");

        if (getpict_event != nullptr) {
            expect(getpict_event->detail.find("mode=headless") != std::string::npos,
                "GETPICT event should declare the headless compatibility mode");
            expect(getpict_event->detail.find("title=\"Select Image\"") != std::string::npos,
                "GETPICT event should include the requested title");
            expect(getpict_event->detail.find("current=\"current.bmp\"") != std::string::npos,
                "GETPICT event should include the current file selection");
            expect(getpict_event->detail.find("result=\"current.bmp\"") != std::string::npos,
                "GETPICT event should include the deterministic fallback result");
        }
        if (getcolor_event != nullptr) {
            expect(getcolor_event->detail.find("mode=headless") != std::string::npos,
                "GETCOLOR event should declare the headless compatibility mode");
            expect(getcolor_event->detail.find("default=255") != std::string::npos,
                "GETCOLOR event should include the requested default color");
            expect(getcolor_event->detail.find("title=\"Pick Accent\"") != std::string::npos,
                "GETCOLOR event should include the requested title");
            expect(getcolor_event->detail.find("result=255") != std::string::npos,
                "GETCOLOR event should include the deterministic fallback result");
        }
        if (getfont_event != nullptr) {
            expect(getfont_event->detail.find("mode=headless") != std::string::npos,
                "GETFONT event should declare the headless compatibility mode");
            expect(getfont_event->detail.find("name=\"Arial\"") != std::string::npos,
                "GETFONT event should include the requested font name");
            expect(getfont_event->detail.find("size=12") != std::string::npos,
                "GETFONT event should include the requested font size");
            expect(getfont_event->detail.find("style=\"B\"") != std::string::npos,
                "GETFONT event should include the requested font style");
            expect(getfont_event->detail.find("result=\"Arial\"") != std::string::npos,
                "GETFONT event should include the deterministic fallback result");
        }
        if (varread_event != nullptr) {
            expect(varread_event->detail.find("mode=headless") != std::string::npos,
                "VARREAD event should declare the headless compatibility mode");
            expect(varread_event->detail.find("active=false") != std::string::npos,
                "VARREAD event should report that no interactive read is active");
            expect(varread_event->detail.find("result=\"\"") != std::string::npos,
                "VARREAD event should include the deterministic empty-string result");
        }

        const fs::path configured_root = temp_root / "configured";
        fs::create_directories(configured_root);
        const fs::path configured_path = configured_root / "cpcurrent.prg";
        write_text(configured_root / "config.fpw", "CODEPAGE = 1251\n");
        write_text(
            configured_path,
            "nConfigured = CPCURRENT()\n"
            "nConfiguredZero = CPCURRENT(0)\n"
            "nConfiguredHost = CPCURRENT(1)\n"
            "nConfiguredOem = CPCURRENT(2)\n"
            "cConfiguredSet = SET('CODEPAGE')\n"
            "SET CODEPAGE TO 932\n"
            "nAfterLiveSet = CPCURRENT()\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession configured_session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(configured_path.string(), configured_root.string()));
        const auto configured_state = configured_session.run(
            copperfin::runtime::DebugResumeAction::continue_run);
        expect(configured_state.completed, "CONFIG.FPW CPCURRENT script should complete");
        const auto check_configured = [&](const std::string &name, const std::string &expected)
        {
            const auto it = configured_state.globals.find(name);
            if (it == configured_state.globals.end())
            {
                expect(false, name + " configured variable not found");
                return;
            }
            const std::string actual = copperfin::runtime::format_value(it->second);
            expect(actual == expected, name + ": expected \"" + expected + "\", got \"" + actual + "\"");
        };
        check_configured("nconfigured", "1251");
        check_configured("nconfiguredzero", "1251");
        check_configured("nconfiguredhost", expected_host_code_page_text);
        check_configured("nconfiguredoem", expected_host_oem_code_page_text);
        check_configured("cconfiguredset", "1251");
        check_configured("nafterliveset", "1251");

        write_text(configured_root / "config.fpw", "CODEPAGE = AUTO\n");
        copperfin::runtime::PrgRuntimeSession auto_session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(configured_path.string(), configured_root.string()));
        const auto auto_state = auto_session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(auto_state.completed, "CONFIG.FPW AUTO CPCURRENT script should complete");
        const auto auto_configured = auto_state.globals.find("nconfigured");
        const auto auto_after_live_set = auto_state.globals.find("nafterliveset");
        expect(auto_configured != auto_state.globals.end() &&
                   copperfin::runtime::format_value(auto_configured->second) == expected_host_code_page_text,
               "CONFIG.FPW CODEPAGE=AUTO should use the host code page");
        expect(auto_after_live_set != auto_state.globals.end() &&
                   copperfin::runtime::format_value(auto_after_live_set->second) == expected_host_code_page_text,
               "CODEPAGE=AUTO should remain stable after a live SET CODEPAGE command");

        fs::remove_all(temp_root, ignored);
    }

    void test_cpdbf_reads_table_code_page_metadata()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_cpdbf_metadata";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path people_path = temp_root / "people.dbf";
        const fs::path other_path = temp_root / "other.dbf";
        write_people_dbf(people_path, {{"ALICE", 30}, {"BOB", 25}});
        write_people_dbf(other_path, {{"CAROL", 35}});
        set_dbf_code_page_mark(people_path, 0x03U);
        set_dbf_code_page_mark(other_path, 0x00U);

        const fs::path main_path = temp_root / "cpdbf_metadata.prg";
        write_text(
            main_path,
            "USE '" + people_path.string() + "' ALIAS people IN 0\n"
            "USE '" + other_path.string() + "' ALIAS other IN 0\n"
            "nPeopleArea      = SELECT('people')\n"
            "nMissingArea     = CPDBF(99)\n"
            "SELECT people\n"
            "nCurrentCp       = CPDBF()\n"
            "nPeopleAliasCp   = CPDBF('people')\n"
            "nPeopleAreaCp    = CPDBF(nPeopleArea)\n"
            "SELECT other\n"
            "nOtherAliasCp    = CPDBF('other')\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "CPDBF metadata script should complete");

        const auto check = [&](const std::string &name, const std::string &expected)
        {
            const auto it = state.globals.find(name);
            if (it == state.globals.end())
            {
                expect(false, name + " variable not found");
                return;
            }
            const std::string actual = copperfin::runtime::format_value(it->second);
            expect(actual == expected, name + ": expected \"" + expected + "\", got \"" + actual + "\"");
        };

        check("ncurrentcp", "1252");
        check("npeoplealiascp", "1252");
        check("npeopleareacp", "1252");
        check("notheraliascp", "0");
        check("nmissingarea", "0");

        fs::remove_all(temp_root, ignored);
    }

    void test_cpdbf_missing_alias_reports_error()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_cpdbf_missing_alias";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path people_path = temp_root / "people.dbf";
        write_people_dbf(people_path, {{"ALICE", 30}});
        set_dbf_code_page_mark(people_path, 0x03U);

        const fs::path main_path = temp_root / "cpdbf_missing_alias.prg";
        write_text(
            main_path,
            "USE '" + people_path.string() + "' ALIAS people IN 0\n"
            "nCpMissing = CPDBF('missing')\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string(), false));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.reason == copperfin::runtime::DebugPauseReason::error,
               "CPDBF missing alias should pause with an error");
        expect(state.message == "Runtime fault: CPDBF target alias not found: missing",
               "CPDBF missing alias should report the localized alias-missing message (got '" +
                   state.message + "')");

        fs::remove_all(temp_root, ignored);
    }

    void test_cpconvert_uses_host_code_page_conversion_with_safe_fallbacks()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_cpconvert";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "cpconvert.prg";
        write_text(
            main_path,
            "cIdentity         = CPCONVERT(1252, 1252, 'hello')\n"
            "cSupported        = CPCONVERT(437, 1252, CHR(142))\n"
            "cInvalid          = CPCONVERT(99999, 1252, 'hello')\n"
            "cUnsupportedHost  = CPCONVERT(620, 1252, 'A')\n"
            "cBinaryIdentity   = CPCONVERT(1252, 1252, CHR(0) + CHR(255))\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "CPCONVERT script should complete");

        const auto require_string = [&](const std::string &name) -> std::string
        {
            const auto it = state.globals.find(name);
            if (it == state.globals.end())
            {
                expect(false, name + " variable not found");
                return {};
            }
            expect(it->second.kind == copperfin::runtime::PrgValueKind::string,
                   name + " should remain a string value");
            return it->second.kind == copperfin::runtime::PrgValueKind::string
                       ? it->second.string_value
                       : std::string{};
        };

        expect(require_string("cidentity") == "hello",
               "same-codepage CPCONVERT should preserve plain text");

        const std::string supported_input(1, static_cast<char>(0x8E));
        const std::optional<std::string> expected_supported =
            expected_host_code_page_conversion(437, 1252, supported_input);
        expect(expected_supported.has_value(),
               "host conversion helper should support the CPCONVERT 437->1252 regression");
        if (expected_supported.has_value())
        {
            expect(require_string("csupported") == *expected_supported,
                   "supported CPCONVERT should follow host conversion semantics");
        }

        expect(require_string("cinvalid") == "hello",
               "invalid CPCONVERT code pages should fall back to the original byte sequence");

        const std::string unsupported_input = "A";
        const std::optional<std::string> expected_unsupported =
            is_supported_vfp_code_page(620)
                ? expected_host_code_page_conversion(620, 1252, unsupported_input)
                : std::nullopt;
        const std::string actual_unsupported = require_string("cunsupportedhost");
        if (expected_unsupported.has_value())
        {
            expect(actual_unsupported == *expected_unsupported,
                   "host-supported CPCONVERT 620->1252 conversion should succeed");
        }
        else
        {
            expect(actual_unsupported == unsupported_input,
                   "unsupported-host CPCONVERT should fall back to the original byte sequence");
        }

        const std::string binary_identity = require_string("cbinaryidentity");
        expect(binary_identity.size() == 2U,
               "same-codepage CPCONVERT should preserve embedded NUL and trailing bytes");
        if (binary_identity.size() == 2U)
        {
            expect(static_cast<unsigned char>(binary_identity[0]) == 0x00U &&
                       static_cast<unsigned char>(binary_identity[1]) == 0xFFU,
                   "same-codepage CPCONVERT should preserve binary string bytes exactly");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_lookup_expression_function()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_lookup";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        // Create a people.dbf with NAME and AGE (using existing helper)
        const fs::path people_path = temp_root / "people.dbf";
        const fs::path people_cdx  = temp_root / "people.cdx";
        write_people_dbf(people_path, {{"ALICE", 30}, {"BOB", 25}, {"CAROL", 35}});
        write_synthetic_cdx(people_cdx, "NAME", "UPPER(NAME)");

        const fs::path main_path = temp_root / "lookup_test.prg";
        write_text(
            main_path,
            "USE '" + people_path.string() + "' ALIAS people IN 0\n"
            "SET ORDER TO TAG NAME\n"
            // LOOKUP found: return AGE of BOB
            "nFound = LOOKUP(people.AGE, 'BOB', 'people', 'NAME')\n"
            // LOOKUP not found: returns .F. (boolean false)
            "cMissing = LOOKUP(people.NAME, 'ZZZZ', 'people', 'NAME')\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "lookup test script should complete");

        const auto check = [&](const std::string &name, const std::string &expected)
        {
            const auto it = state.globals.find(name);
            if (it == state.globals.end())
            {
                expect(false, name + " variable not found");
                return;
            }
            const std::string actual = copperfin::runtime::format_value(it->second);
            expect(actual == expected, name + ": expected \"" + expected + "\", got \"" + actual + "\"");
        };

        // LOOKUP miss always returns .F.
        check("cmissing", "false");
        check("nfound", "25");

        fs::remove_all(temp_root, ignored);
    }

}
