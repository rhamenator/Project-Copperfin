// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "copperfin/runtime/prg_engine.h"
#include "prg_engine_test_support.h"
#include "copperfin/vfp/dbf_table.h"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <system_error>

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

namespace
{

    using namespace copperfin::test_support;

    void test_expression_runtime_surface_extensions()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_runtime_surface_extensions";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "runtime_surface_extensions.prg";
        write_text(
            main_path,
            "x = 5\n"
            "cExistingFile = 'runtime_surface_extensions.prg'\n"
            "DIMENSION aValues[2]\n"
            "aValues[1] = 'A'\n"
            "aValues[2] = 'B'\n"
            "lFileHit = FILE(cExistingFile)\n"
            "lFileMiss = FILE('missing-file.prg')\n"
            "nEval = EVALUATE('x + 7')\n"
            "cEvalExpr = 'x + 9'\n"
            "nEvalMacro = EVALUATE(&cEvalExpr)\n"
            "cEvalExprHolder = 'cEvalExpr'\n"
            "nEvalNestedMacro = EVALUATE(&cEvalExprHolder)\n"
            "cEvalExprDeepHolder = 'cEvalExprHolder'\n"
            "nEvalSecondHopMacro = EVALUATE(&cEvalExprDeepHolder)\n"
            "cSysScript = SYS(16)\n"
            "cSysDefault = SYS(999)\n"
            "cHome = HOME()\n"
            "cOs = OS()\n"
            "nDiskSpace = DISKSPACE()\n"
            "lDiskSpacePositive = nDiskSpace > 0\n"
            "nDriveType = DRIVETYPE()\n"
            "nMissingDriveType = DRIVETYPE('missing-path')\n"
            "cSysCurrent = SYS(2003)\n"
            "cSysTemp = SYS(2023)\n"
            "cSysDisk = SYS(2020)\n"
            "lSysDiskPositive = VAL(cSysDisk) > 0\n"
            "cTransformDefault = TRANSFORM(x)\n"
            "cTransformPicture = TRANSFORM(3.14159, '999.00')\n"
            "cTransformUpper = TRANSFORM('legacy', '@!')\n"
            "cTypeArray = TYPE('aValues')\n"
            "cTypeUnknown = TYPE('notDefinedAnywhere')\n"
            "nCastInt = CAST(7.9 AS INTEGER)\n"
            "nCastByte = CAST(513 AS BYTE)\n"
            "cCastString = CAST(123 AS STRING)\n"
            "lCastBool = CAST(0 AS LOGICAL)\n"
            "nBitAnd = BITAND(6, 3)\n"
            "nBitAndMulti = BITAND(15, 7, 3)\n"
            "nBitOr = BITOR(6, 3)\n"
            "nBitOrMulti = BITOR(1, 4, 8)\n"
            "nBitXor = BITXOR(6, 3)\n"
            "nBitXorMulti = BITXOR(1, 3, 7)\n"
            "nBitNot = BITNOT(0)\n"
            "nBitClear = BITCLEAR(7, 1)\n"
            "nBitSet = BITSET(4, 1)\n"
            "lBitTestHit = BITTEST(6, 1)\n"
            "lBitTestMiss = BITTEST(6, 0)\n"
            "nBitHigh = BITSET(0, 31)\n"
            "nBitLShift = BITLSHIFT(3, 2)\n"
            "nBitRShift = BITRSHIFT(16, 2)\n"
            "cPacked = BINTOC(16909060, 4)\n"
            "nUnpacked = CTOBIN(cPacked, 'N')\n"
            "nCursorProp = CURSORGETPROP('Buffering')\n"
            "nVersion = VERSION()\n"
            "nVersionArg = VERSION(1)\n"
            "lNumLock = NUMLOCK()\n"
            "cOnErrorDefault = ON('ERROR')\n"
            "cOnShutdownDefault = ON('SHUTDOWN')\n"
            "nMessageBox = MESSAGEBOX('hi')\n"
            "cDefaultPath = SET('PATH')\n"
            "cDefaultDecimals = SET('DECIMALS')\n"
            "cDefaultCollate = SET('COLLATE')\n"
            "cDefaultFdow = SET('FDOW')\n"
            "cDefaultFweek = SET('FWEEK')\n"
            "cDefaultPoint = SET('POINT')\n"
            "cDefaultSeparator = SET('SEPARATOR')\n"
            "cDefaultCurrency = SET('CURRENCY')\n"
            "cPathTarget = '/tmp/copperfin'\n"
            "cMarkTarget = '-'\n"
            "nDecimalsTarget = 4\n"
            "cCollateTarget = 'machine'\n"
            "lNullTarget = .T.\n"
            "cPathTargetHolder = 'cPathTarget'\n"
            "cPathTargetDeepHolder = 'cPathTargetHolder'\n"
            "cMarkTargetHolder = 'cMarkTarget'\n"
            "cMarkTargetDeepHolder = 'cMarkTargetHolder'\n"
            "cDecimalsTargetHolder = 'nDecimalsTarget'\n"
            "cDecimalsTargetDeepHolder = 'cDecimalsTargetHolder'\n"
            "cCollateTargetHolder = 'cCollateTarget'\n"
            "cCollateTargetDeepHolder = 'cCollateTargetHolder'\n"
            "cNullTargetHolder = 'lNullTarget'\n"
            "cNullTargetDeepHolder = 'cNullTargetHolder'\n"
            "lAnsiTarget = .F.\n"
            "cAnsiTargetHolder = 'lAnsiTarget'\n"
            "cAnsiTargetDeepHolder = 'cAnsiTargetHolder'\n"
            "SET PATH TO '/tmp/copperfin'\n"
            "cPathValue = SET('PATH')\n"
            "SET PATH TO cPathTarget\n"
            "cPathFromVariable = SET('PATH')\n"
            "SET PATH TO &cPathTargetDeepHolder\n"
            "cPathFromSecondHopMacro = SET('PATH')\n"
            "cEvalPathExpr = \"SET('PATH')\"\n"
            "cEvalPathMacro = EVALUATE(&cEvalPathExpr)\n"
            "cEvalPathExprHolder = 'cEvalPathExpr'\n"
            "cEvalPathExprDeepHolder = 'cEvalPathExprHolder'\n"
            "cEvalPathSecondHopMacro = EVALUATE(&cEvalPathExprDeepHolder)\n"
            "SET MARK TO cMarkTarget\n"
            "cMarkFromVariable = SET('MARK')\n"
            "SET MARK TO &cMarkTargetDeepHolder\n"
            "cMarkFromSecondHopMacro = SET('MARK')\n"
            "SET DECIMALS TO nDecimalsTarget\n"
            "cDecimalsFromVariable = SET('DECIMALS')\n"
            "SET DECIMALS TO &cDecimalsTargetDeepHolder\n"
            "cDecimalsFromSecondHopMacro = SET('DECIMALS')\n"
            "SET COLLATE TO cCollateTarget\n"
            "cCollateFromVariable = SET('COLLATE')\n"
            "SET COLLATE TO &cCollateTargetDeepHolder\n"
            "cCollateFromSecondHopMacro = SET('COLLATE')\n"
            "SET FDOW TO 7\n"
            "cFdowFromSet = SET('FDOW')\n"
            "SET FWEEK TO 4\n"
            "cFweekFromSet = SET('FWEEK')\n"
            "SET POINT TO ';'\n"
            "cPointFromSet = SET('POINT')\n"
            "SET SEPARATOR TO ':'\n"
            "cSeparatorFromSet = SET('SEPARATOR')\n"
            "SET CURRENCY TO 'USD'\n"
            "cCurrencyFromSet = SET('CURRENCY')\n"
            "SET NULL TO lNullTarget\n"
            "cNullFromVariable = SET('NULL')\n"
            "SET NULL TO &cNullTargetDeepHolder\n"
            "cNullFromSecondHopMacro = SET('NULL')\n"
            "SET ANSI ON\n"
            "cAnsiOn = SET('ANSI')\n"
            "SET ANSI TO &cAnsiTargetDeepHolder\n"
            "cAnsiFromSecondHopMacro = SET('ANSI')\n"
            "SET ANSI ON\n"
            "SET DATASESSION TO 2\n"
            "cCollateSession2 = SET('COLLATE')\n"
            "cFdowSession2 = SET('FDOW')\n"
            "cFweekSession2 = SET('FWEEK')\n"
            "cPointSession2 = SET('POINT')\n"
            "cSeparatorSession2 = SET('SEPARATOR')\n"
            "cCurrencySession2 = SET('CURRENCY')\n"
            "cNullSession2 = SET('NULL')\n"
            "cAnsiSession2 = SET('ANSI')\n"
            "SET DATASESSION TO 1\n"
            "cCollateRestored = SET('COLLATE')\n"
            "cFdowRestored = SET('FDOW')\n"
            "cFweekRestored = SET('FWEEK')\n"
            "cPointRestored = SET('POINT')\n"
            "cSeparatorRestored = SET('SEPARATOR')\n"
            "cCurrencyRestored = SET('CURRENCY')\n"
            "cNullRestored = SET('NULL')\n"
            "cAnsiRestored = SET('ANSI')\n"
            "ON ERROR DO somehandler\n"
            "cOnErrorHandler = ON('ERROR')\n"
            "ON SHUTDOWN CLOSE DATABASES ALL\n"
            "cOnShutdownHandler = ON('SHUTDOWN')\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "runtime surface extension script should complete");

        const auto check = [&](const std::string& name, const std::string& expected)
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

        check("neval", "12");
        check("nevalmacro", "14");
        check("nevalnestedmacro", "14");
        check("nevalsecondhopmacro", "14");
        check("cevalpathsecondhopmacro", "/tmp/copperfin");
        check("cexistingfile", "runtime_surface_extensions.prg");
        check("lfilehit", "true");
        check("lfilemiss", "false");
        check("csysscript", main_path.string());
        check("csysdefault", "0");
        check("chome", temp_root.string());
        check("ldiskspacepositive", "true");
        check("ndrivetype", "3");
        check("nmissingdrivetype", "0");
        check("csyscurrent", temp_root.string());
        check("lsysdiskpositive", "true");
        check("ctransformdefault", "5");
        check("ctransformpicture", "3.14");
        check("ctransformupper", "LEGACY");
        check("ctypearray", "A");
        check("ctypeunknown", "U");
        check("ncastint", "7");
        check("ncastbyte", "1");
        check("ccaststring", "123");
        check("lcastbool", "false");
        check("nbitand", "2");
        check("nbitandmulti", "3");
        check("nbitor", "7");
        check("nbitormulti", "13");
        check("nbitxor", "5");
        check("nbitxormulti", "5");
        check("nbitnot", "-1");
        check("nbitclear", "5");
        check("nbitset", "6");
        check("lbittesthit", "true");
        check("lbittestmiss", "false");
        check("nbithigh", "-2147483648");
        check("nbitlshift", "12");
        check("nbitrshift", "4");
        check("nunpacked", "16909060");
        check("ncursorprop", "0");
        check("nversion", "9");
        check("nversionarg", "0");
        check("lnumlock", "false");
        check("conerrordefault", "");
        check("conshutdowndefault", "");
        check("nmessagebox", "1");
        check("cdefaultpath", "");
        check("cdefaultdecimals", "2");
        check("cdefaultcollate", "MACHINE");
        check("cdefaultfdow", "1");
        check("cdefaultfweek", "1");
        check("cdefaultpoint", ".");
        check("cdefaultseparator", ",");
        check("cdefaultcurrency", "$");
        check("cpathvalue", "/tmp/copperfin");
        check("cpathfromvariable", "/tmp/copperfin");
        check("cpathfromsecondhopmacro", "/tmp/copperfin");
        check("cevalpathmacro", "/tmp/copperfin");
        check("cmarkfromvariable", "-");
        check("cmarkfromsecondhopmacro", "-");
        check("cdecimalsfromvariable", "4");
        check("cdecimalsfromsecondhopmacro", "4");
        check("ccollatefromvariable", "MACHINE");
        check("ccollatefromsecondhopmacro", "MACHINE");
        check("cfdowfromset", "7");
        check("cfweekfromset", "3");
        check("cpointfromset", ";");
        check("cseparatorfromset", ":");
        check("ccurrencyfromset", "USD");
        check("cnullfromvariable", "ON");
        check("cnullfromsecondhopmacro", "ON");
        check("cansion", "ON");
        check("cansifromsecondhopmacro", "OFF");
        check("ccollatesession2", "MACHINE");
        check("cfdowsession2", "1");
        check("cfweeksession2", "1");
        check("cpointsession2", ".");
        check("cseparatorsession2", ",");
        check("ccurrencysession2", "$");
        check("cnullsession2", "OFF");
        check("cansisession2", "OFF");
        check("ccollaterestored", "MACHINE");
        check("cfdowrestored", "7");
        check("cfweekrestored", "3");
        check("cpointrestored", ";");
        check("cseparatorrestored", ":");
        check("ccurrencyrestored", "USD");
        check("cnullrestored", "ON");
        check("cansirestored", "ON");
        check("conerrorhandler", "DO somehandler");
        check("conshutdownhandler", "CLOSE DATABASES ALL");

        const auto os_value = state.globals.find("cos");
        expect(os_value != state.globals.end() && !copperfin::runtime::format_value(os_value->second).empty(),
               "OS() should expose a non-empty host OS label");
        const auto temp_value = state.globals.find("csystemp");
        expect(temp_value != state.globals.end() && !copperfin::runtime::format_value(temp_value->second).empty(),
               "SYS(2023) should expose a non-empty temporary path");

        fs::remove_all(temp_root, ignored);
    }

    void test_filesize_expression_function()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_filesize";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        // Create a test file with known content
        const fs::path test_file_path = temp_root / "testfile.txt";
        const std::string test_content = "Hello, World! This is a test file.";
        write_text(test_file_path, test_content);

        // Create a file only discoverable via SET PATH
        const fs::path path_probe_dir = temp_root / "path_probe";
        fs::create_directories(path_probe_dir);
        const fs::path path_only_file_path = path_probe_dir / "path_only.txt";
        const std::string path_only_content = "Found only through SET PATH";
        write_text(path_only_file_path, path_only_content);

        const fs::path main_path = temp_root / "filesize_test.prg";
        write_text(
            main_path,
            "cTestFile = 'testfile.txt'\n"
            "nFileSize = FILESIZE(cTestFile)\n"
            "nMissingFile = FILESIZE('missing-file.txt')\n"
            "nEmptyArg = FILESIZE()\n"
            "nAbsolutePath = FILESIZE('" + test_file_path.string() + "')\n"
            "lPathFileBefore = FILE('path_only.txt')\n"
            "SET PATH TO '" + path_probe_dir.string() + "'\n"
            "lPathFileAfter = FILE('path_only.txt')\n"
            "nPathFileSize = FILESIZE('path_only.txt')\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "FILESIZE() test script should complete");

        const auto check = [&](const std::string& name, const std::string& expected)
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

        // Test file size matches the test content length
        check("nfilesize", std::to_string(test_content.length()));
        
        // Missing file should return 0
        check("nmissingfile", "0");
        
        // Empty argument should return 0
        check("nemptyarg", "0");
        
        // Absolute path should also work
        check("nabsolutepath", std::to_string(test_content.length()));

        // SET PATH resolution should make path-only file discoverable
        check("lpathfilebefore", "false");
        check("lpathfileafter", "true");
        check("npathfilesize", std::to_string(path_only_content.length()));

        fs::remove_all(temp_root, ignored);
    }

    void test_recsize_reclength_expression_functions()
    {
        // Simple test to validate RECSIZE/RECLENGTH functionality
        // For now, just verify the functions exist and return 0 for non-existent cursors
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_recsize";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "recsize_test.prg";
        write_text(
            main_path,
            "nRecSizeClosed = RECSIZE()\n"
            "nRecLengthClosed = RECLENGTH()\n"
            "nRecSizeNoArea = RECSIZE('nonexistent')\n"
            "nRecLengthNoArea = RECLENGTH('nonexistent')\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "RECSIZE/RECLENGTH test script should complete");

        const auto check = [&](const std::string& name, const std::string& expected)
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

        // All values should be 0 since we're not using any tables
        check("nrecsizeclosed", "0");
        check("nreclengthclosed", "0");
        check("nrecsizenoarea", "0");
        check("nreclengthnoarea", "0");

        fs::remove_all(temp_root, ignored);
    }

    void test_environment_and_sys_introspection_functions()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_env_sys_helpers";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const std::string env_name = "COPPERFIN_RUNTIME_SURFACE_ENV_HELPER";
        const fs::path main_path = temp_root / "env_and_sys_introspection.prg";
        write_text(
            main_path,
            "nPCountMain = PCOUNT()\n"
            "DO pcount_helper WITH 10, 'x', .T.\n"
            "lPutEnvSet = PUTENV('" + env_name + "', 'runtime-surface-value')\n"
            "cGetEnvSet = GETENV('" + env_name + "')\n"
            "lPutEnvClear = PUTENV('" + env_name + "', '')\n"
            "cGetEnvCleared = GETENV('" + env_name + "')\n"
            "cSys3 = SYS(3)\n"
            "cSys7 = SYS(7)\n"
            "cSys11 = SYS(11)\n"
            "cSys13 = SYS(13)\n"
            "RETURN\n"
            "\n"
            "PROCEDURE pcount_helper\n"
            "LPARAMETERS p1, p2, p3, p4\n"
            "nPCountRoutine = PCOUNT()\n"
            "RETURN\n"
            "ENDPROC\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "environment and SYS helper script should complete");

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

        check("npcountmain", "0");
        check("npcountroutine", "3");
        check("lputenvset", "true");
        check("cgetenvset", "runtime-surface-value");
        check("lputenvclear", "true");
        check("cgetenvcleared", "");
        check("csys11", "0");
        check("csys13", "0");

        const auto sys3_value = state.globals.find("csys3");
        expect(sys3_value != state.globals.end() && !copperfin::runtime::format_value(sys3_value->second).empty(),
               "SYS(3) should expose a non-empty runtime build token");

        const auto sys7_value = state.globals.find("csys7");
        expect(sys7_value != state.globals.end() && !copperfin::runtime::format_value(sys7_value->second).empty(),
               "SYS(7) should expose a non-empty host descriptor");

        fs::remove_all(temp_root, ignored);
    }

    void test_object_reflection_runtime_surface_functions()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_object_reflection";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "object_reflection_runtime_surface.prg";
        write_text(
            main_path,
            "oOne = CREATEOBJECT('Scripting.Dictionary')\n"
            "oTwo = CREATEOBJECT('Scripting.Dictionary')\n"
            "lCompDiff = COMPOBJ(oOne, oTwo)\n"
            "lCompSame = COMPOBJ(oOne, oOne)\n"
            "lCompNotObject = COMPOBJ('x', 'x')\n"
            "nMembersAll = AMEMBERS(aMembersOut, oOne, 0)\n"
            "nMembersProperties = AMEMBERS(aMembersProps, oOne, 1)\n"
            "nMembersMethods = AMEMBERS(aMembersMethods, oOne, 2)\n"
            "nMembersEvents = AMEMBERS(aMembersEvents, oOne, 4)\n"
            "nMembersUnion = AMEMBERS(aMembersUnion, oOne, 3)\n"
            "cMemberAllFirst = aMembersOut[1]\n"
            "cMemberAllLast = aMembersOut[nMembersAll]\n"
            "cMemberMethodsFirst = aMembersMethods[1]\n"
            "cMemberMethodsLast = aMembersMethods[nMembersMethods]\n"
            "cMemberPropsFirst = aMembersProps[1]\n"
            "cMemberPropsLast = aMembersProps[nMembersProperties]\n"
            "lPemMissing = PEMSTATUS(oOne, 'missingprop', 1)\n"
            "lPemMethodExists = PEMSTATUS(oOne, 'add', 1)\n"
            "lPemPropertyExists = PEMSTATUS(oOne, 'comparemode', 1)\n"
            "lPemReadOnlyCount = PEMSTATUS(oOne, 'count', 5)\n"
            "lPemReadOnlyCompareMode = PEMSTATUS(oOne, 'comparemode', 5)\n"
            "lPemReadOnlyMethod = PEMSTATUS(oOne, 'add', 5)\n"
            "lAdd = ADDPROPERTY(oOne, 'SampleProp', 42)\n"
            "lPemExistsAfterAdd = PEMSTATUS(oOne, 'SampleProp', 1)\n"
            "nMembersAfterAddProperties = AMEMBERS(aMembersPropsAfterAdd, oOne, 1)\n"
            "nClassCount = ACLASS(aClass, oOne)\n"
            "cClassFirst = aClass[1]\n"
            "cClassSecond = aClass[2]\n"
            "lRemove = REMOVEPROPERTY(oOne, 'SampleProp')\n"
            "lPemExistsAfterRemove = PEMSTATUS(oOne, 'SampleProp', 1)\n"
            "lRemoveMissing = REMOVEPROPERTY(oOne, 'SampleProp')\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
         expect(state.completed,
               std::string("object reflection runtime-surface script should complete: ") + state.message +
                   " @line=" + std::to_string(state.location.line) +
                   " stmt='" + state.statement_text + "'");

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

        check("lcompdiff", "false");
        check("lcompsame", "true");
        check("lcompnotobject", "false");
        check("lpemmissing", "false");
        check("lpemmethodexists", "true");
        check("lpempropertyexists", "true");
        check("lpemreadonlycount", "true");
        check("lpemreadonlycomparemode", "false");
        check("lpemreadonlymethod", "false");
        check("ladd", "true");
        check("lpemexistsafteradd", "true");
        check("lremove", "true");
        check("lpemexistsafterremove", "false");
        check("lremovemissing", "false");

        check("nmembersall", "9");
        check("nmembersproperties", "2");
        check("nmembersmethods", "7");
        check("nmembersevents", "0");
        check("nmembersunion", "9");
        check("cmemberallfirst", "ADD");
        check("cmemberalllast", "REMOVEALL");
        check("cmembermethodsfirst", "ADD");
        check("cmembermethodslast", "REMOVEALL");
        check("cmemberpropsfirst", "COMPAREMODE");
        check("cmemberpropslast", "COUNT");
        check("nmembersafteraddproperties", "3");
        check("cclassfirst", "DICTIONARY");
        check("cclasssecond", "OBJECT");

        const auto class_count = state.globals.find("nclasscount");
        const double class_count_value =
            class_count == state.globals.end() ? -1.0 : std::stod(copperfin::runtime::format_value(class_count->second));
         expect(class_count != state.globals.end() && class_count_value == 2.0,
             "ACLASS() should return [class, OBJECT] with two rows");

        fs::remove_all(temp_root, ignored);
    }

    void test_cursor_xml_round_trip_runtime_surface_functions()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_cursor_xml_round_trip";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "cursor_xml_round_trip.prg";
        write_text(
            main_path,
            "CREATE CURSOR SourceXml (ID N(4,0), NAME C(20))\n"
            "INSERT INTO SourceXml (ID, NAME) VALUES (1, 'ALPHA')\n"
            "INSERT INTO SourceXml (ID, NAME) VALUES (2, 'BETA')\n"
            "INSERT INTO SourceXml (ID, NAME) VALUES (3, 'GAMMA')\n"
            "cXml = CURSORTOXML('SourceXml')\n"
            "lWriteOk = CURSORTOXML('SourceXml', 'round_trip.xml')\n"
            "nLoaded = XMLTOCURSOR(cXml, 'DestXml')\n"
            "SELECT DestXml\n"
            "nDestCount = RECCOUNT()\n"
            "GO TOP\n"
            "nFirstId = ID\n"
            "cFirstName = NAME\n"
            "GO BOTTOM\n"
            "nLastId = ID\n"
            "cLastName = NAME\n"
            "nLoadedFromFile = XMLTOCURSOR('round_trip.xml', 'DestFile')\n"
            "SELECT DestFile\n"
            "nFileCount = RECCOUNT()\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("CURSORTOXML/XMLTOCURSOR round-trip script should complete: ") + state.message +
                   " @line=" + std::to_string(state.location.line) +
                   " stmt='" + state.statement_text + "'");

        const auto check = [&](const std::string& name, const std::string& expected)
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

        const auto xml_text = state.globals.find("cxml");
        expect(xml_text != state.globals.end() &&
                   copperfin::runtime::format_value(xml_text->second).find("<CopperfinCursor") != std::string::npos,
               "CURSORTOXML() should return Copperfin XML text when output target is omitted");
        check("lwriteok", "true");
        check("nloaded", "3");
        check("nloadedfromfile", "3");
        check("ndestcount", "3");
        check("nfilecount", "3");
        check("nfirstid", "1");
        check("cfirstname", "ALPHA");
        check("nlastid", "3");
        check("clastname", "GAMMA");

        expect(std::count_if(state.events.begin(), state.events.end(), [](const auto& event)
        {
            return event.category == "runtime.cursortoxml" && event.detail.find("rows=3") != std::string::npos;
        }) >= 2,
               "CURSORTOXML() should emit runtime.cursortoxml events with row counts");
        expect(std::count_if(state.events.begin(), state.events.end(), [](const auto& event)
        {
            return event.category == "runtime.xmltocursor" && event.detail.find("rows=3") != std::string::npos;
        }) >= 2,
               "XMLTOCURSOR() should emit runtime.xmltocursor events with row counts");

        fs::remove_all(temp_root, ignored);
    }

    void test_cursor_xml_invalid_input_runtime_surface_functions()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_cursor_xml_invalid";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "cursor_xml_invalid.prg";
        write_text(
            main_path,
            "nBadLoad = XMLTOCURSOR('<NotCopperfin />', 'BadDest')\n"
            "lBadExport = CURSORTOXML('MissingAlias', 'missing_output.xml')\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "invalid CURSORTOXML/XMLTOCURSOR script should complete safely");

        const auto check = [&](const std::string& name, const std::string& expected)
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

        check("nbadload", "0");
        check("lbadexport", "false");
        expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event)
        {
            return event.category == "runtime.warning";
        }),
               "invalid XML helper input should emit runtime.warning event(s)");

        fs::remove_all(temp_root, ignored);
    }

    void test_newobject_getpem_setpem_compobj_functions()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_newobject_getpem_setpem";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "newobject_getpem_setpem.prg";
        write_text(
            main_path,
            "oa = NEWOBJECT('Scripting.Dictionary')\n"
            "ob = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lsamesame = COMPOBJ(oa, oa)\n"
            "ldiffab = COMPOBJ(oa, ob)\n"
            "lnullleft = COMPOBJ(.NULL., oa)\n"
            "lnullright = COMPOBJ(oa, .NULL.)\n"
            "lbothnull = COMPOBJ(.NULL., .NULL.)\n"
            "ngetprop = GETPEM(oa, 'comparemode')\n"
            "lgetmethod = GETPEM(oa, 'add')\n"
            "xgetmissing = GETPEM(oa, 'nosuchprop')\n"
            "lsetprop = SETPEM(oa, 'comparemode', 1)\n"
            "ngetpropafterset = GETPEM(oa, 'comparemode')\n"
            "lsetreadonly = SETPEM(oa, 'count', 99)\n"
            "lsetunknown = SETPEM(oa, 'nosuchprop', 42)\n"
            "lsetmethod = SETPEM(oa, 'add', 'MyAddProc')\n"
            "cgetmethodafterset = GETPEM(oa, 'add')\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("NEWOBJECT/GETPEM/SETPEM script should complete: ") + state.message +
                   " @line=" + std::to_string(state.location.line));

        const auto check = [&](const std::string& name, const std::string& expected)
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

        // NEWOBJECT creates valid object refs
        expect(state.globals.count("oa") && state.globals.at("oa").kind == copperfin::runtime::PrgValueKind::string,
               "NEWOBJECT('Scripting.Dictionary') should return a string object ref");
        expect(state.globals.count("ob") && state.globals.at("ob").kind == copperfin::runtime::PrgValueKind::string,
               "NEWOBJECT('Scripting.Dictionary', 'vbscript.dll') should return a string object ref");

        // NEWOBJECT should have emitted ole.newobject events
        const bool has_newobject_event = std::any_of(state.events.begin(), state.events.end(), [](const auto& ev)
        {
            return ev.category == "ole.newobject";
        });
        expect(has_newobject_event, "NEWOBJECT() should emit ole.newobject event");

        // COMPOBJ
        check("lsamesame", "true");
        check("ldiffab", "false");
        check("lnullleft", "false");
        check("lnullright", "false");
        check("lbothnull", "false");

        // GETPEM — property, method presence, missing member
        check("ngetprop", "0");        // comparemode default = 0
        check("lgetmethod", "true");   // 'add' is a method → .T.
        // missing returns empty (format_value of empty is "")
        {
            const auto it = state.globals.find("xgetmissing");
            expect(it != state.globals.end() && it->second.kind == copperfin::runtime::PrgValueKind::empty,
                   "GETPEM unknown member should return empty (.NULL.)");
        }

        // SETPEM
        check("lsetprop", "true");           // setting comparemode succeeds
        check("ngetpropafterset", "1");      // comparemode now 1
        check("lsetreadonly", "false");      // count is read-only → .F.
        check("lsetunknown", "false");       // unknown property → .F.
        check("lsetmethod", "true");         // setting method ref succeeds
        check("cgetmethodafterset", "MyAddProc");  // method ref retrievable as property

        fs::remove_all(temp_root, ignored);
    }

    void test_createobject_instantiates_native_prg_class_and_preserves_plain_object_creation()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_createobject_native_prg_class";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "createobject_native_class.prg";
        write_text(
            main_path,
            "oWidget = CREATEOBJECT('MyWidget')\n"
            "cCaption = oWidget.Caption\n"
            "nCount = oWidget.nCount\n"
            "lHasInit = GETPEM(oWidget, 'Init')\n"
            "lHasCaption = PEMSTATUS(oWidget, 'Caption', 1)\n"
            "oPlain = CREATEOBJECT('Empty')\n"
            "oPlain.Extra = 'plain'\n"
            "cPlain = oPlain.Extra\n"
            "RETURN\n"
            "DEFINE CLASS MyWidget AS Custom\n"
            "    Caption = 'Demo'\n"
            "    nCount = 3\n"
            "    PROCEDURE Init\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native CREATEOBJECT script should complete: ") + state.message +
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

        expect(state.globals.count("owidget") && state.globals.at("owidget").kind == copperfin::runtime::PrgValueKind::string,
               "CREATEOBJECT('MyWidget') should return a string object ref");
        check("ccaption", "Demo");
        check("ncount", "3");
        check("lhasinit", "true");
        check("lhascaption", "true");

        expect(state.globals.count("oplain") && state.globals.at("oplain").kind == copperfin::runtime::PrgValueKind::string,
               "CREATEOBJECT('Empty') should still return a string object ref");
        check("cplain", "plain");

        expect(state.ole_objects.size() == 2U,
               "native CREATEOBJECT plus plain CREATEOBJECT should register two runtime objects");
        if (state.ole_objects.size() == 2U)
        {
            expect(state.ole_objects[0].prog_id == "MyWidget",
                   "native CREATEOBJECT should preserve the PRG class name in runtime object state");
            expect(state.ole_objects[0].source == main_path.string(),
                   "native CREATEOBJECT should preserve the defining PRG path as object provenance");
            expect(state.ole_objects[1].prog_id == "Empty",
                   "plain CREATEOBJECT should still preserve the requested non-class prog id");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_newobject_instantiates_native_prg_class_and_preserves_ole_newobject()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_newobject_native_prg_class";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "newobject_native_class.prg";
        write_text(
            main_path,
            "oWidget = NEWOBJECT('MyWidget')\n"
            "cName = oWidget.Name\n"
            "lHasSave = GETPEM(oWidget, 'CanSave')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lSetCompare = SETPEM(oDict, 'comparemode', 2)\n"
            "nCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS MyWidget AS Custom\n"
            "    Name = 'Widget'\n"
            "    FUNCTION CanSave\n"
            "        RETURN .T.\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native NEWOBJECT script should complete: ") + state.message +
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

        expect(state.globals.count("owidget") && state.globals.at("owidget").kind == copperfin::runtime::PrgValueKind::string,
               "NEWOBJECT('MyWidget') should return a string object ref");
        check("cname", "Widget");
        check("lhassave", "true");

        expect(state.globals.count("odict") && state.globals.at("odict").kind == copperfin::runtime::PrgValueKind::string,
               "NEWOBJECT('Scripting.Dictionary', 'vbscript.dll') should still return a string object ref");
        check("lsetcompare", "true");
        check("ncompare", "2");

        expect(state.ole_objects.size() == 2U,
               "native and OLE NEWOBJECT calls should both register runtime objects");
        if (state.ole_objects.size() == 2U)
        {
            expect(state.ole_objects[0].prog_id == "MyWidget",
                   "native NEWOBJECT should preserve the PRG class name in runtime object state");
            expect(state.ole_objects[0].source == main_path.string(),
                   "native NEWOBJECT should preserve the defining PRG path as object provenance");
            expect(state.ole_objects[1].prog_id == "Scripting.Dictionary",
                   "OLE NEWOBJECT should still preserve the requested COM class");
            expect(state.ole_objects[1].source == "vbscript.dll",
                   "OLE NEWOBJECT should still preserve the requested library source");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_native_addobject_materializes_child_objects_and_child_methods_see_parent()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_addobject";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_addobject.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('DemoForm')\n"
            "oChild = oForm.cmdSave\n"
            "lHasChild = PEMSTATUS(oForm, 'cmdSave', 1)\n"
            "lChildHasParent = PEMSTATUS(oChild, 'Parent', 1)\n"
            "lChildAdded = oForm.lChildAdded\n"
            "cFormCaption = oForm.Caption\n"
            "cChildCaption = oChild.Caption\n"
            "oParent = oChild.Parent\n"
            "cParentCaption = oParent.Caption\n"
            "cOwnerCaption = oChild.OwnerCaption()\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 13)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    lChildAdded = .F.\n"
            "    PROCEDURE Init\n"
            "        THIS.lChildAdded = THIS.AddObject('cmdSave', 'SaveButton')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
            "    Caption = 'Save'\n"
            "    FUNCTION OwnerCaption\n"
            "        RETURN PARENT.Caption\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native ADDOBJECT script should complete: ") + state.message +
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

        check("lhaschild", "true");
        check("lchildhasparent", "true");
        check("lchildadded", "true");
        check("cformcaption", "MainForm");
        check("cchildcaption", "Save");
        check("cparentcaption", "MainForm");
        check("cownercaption", "MainForm");
        check("ldictset", "true");
        check("ndictcompare", "13");

        expect(state.ole_objects.size() == 3U,
               "native ADDOBJECT script should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &parent_object = state.ole_objects[0];
            const auto &child_object = state.ole_objects[1];
            expect(parent_object.prog_id == "DemoForm",
                   "native ADDOBJECT should preserve the parent class identity");
            expect(child_object.prog_id == "SaveButton",
                   "native ADDOBJECT should preserve the child class identity");
            expect(parent_object.properties.contains("cmdsave"),
                   "native ADDOBJECT should materialize the child reference on the parent");
            const auto child_parent = child_object.properties.find("parent");
            if (child_parent != child_object.properties.end())
            {
                expect(child_parent->second.kind == copperfin::runtime::PrgValueKind::string,
                       "native ADDOBJECT should persist a PARENT object reference on the child");
            }
            else
            {
                expect(false, "native ADDOBJECT should materialize the child PARENT reference");
            }
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM behavior should remain stable while native ADDOBJECT lands");
        }

        const bool has_addobject_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.addobject" &&
                   event.detail == "DemoForm.cmdsave:SaveButton";
        });
        expect(has_addobject_event,
               "native ADDOBJECT should emit child-activation events");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_addobject_materializes_external_prg_child_objects_and_preserves_init_flow()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_addobject_external_library";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path library_path = temp_root / "buttons.prg";
        write_text(
            library_path,
            "DEFINE CLASS SaveButton AS Custom\n"
            "    Caption = 'Save'\n"
            "    nSeed = 0\n"
            "    PROCEDURE Init\n"
            "        LPARAMETERS tnSeed\n"
            "        THIS.nSeed = tnSeed\n"
            "        THIS.Caption = THIS.Caption + '-' + ALLTRIM(STR(tnSeed))\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "    FUNCTION OwnerCaption\n"
            "        RETURN PARENT.Caption\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "native_addobject_external_library.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('DemoForm')\n"
            "oChild = oForm.cmdSave\n"
            "lHasChild = PEMSTATUS(oForm, 'cmdSave', 1)\n"
            "lChildHasParent = PEMSTATUS(oChild, 'Parent', 1)\n"
            "lChildAdded = oForm.lChildAdded\n"
            "cInitChildCaption = oForm.cInitChildCaption\n"
            "cInitOwnerCaption = oForm.cInitOwnerCaption\n"
            "nChildSeed = oChild.nSeed\n"
            "cChildCaption = oChild.Caption\n"
            "cOwnerCaption = oChild.OwnerCaption()\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 14)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    lChildAdded = .F.\n"
            "    cInitChildCaption = ''\n"
            "    cInitOwnerCaption = ''\n"
            "    PROCEDURE Init\n"
            "        THIS.lChildAdded = THIS.AddObject('cmdSave', 'SaveButton', 'buttons.prg', 7)\n"
            "        THIS.cInitChildCaption = THIS.cmdSave.Caption\n"
            "        THIS.cInitOwnerCaption = THIS.cmdSave.OwnerCaption()\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native external-library ADDOBJECT script should complete: ") + state.message +
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

        check("lhaschild", "true");
        check("lchildhasparent", "true");
        check("lchildadded", "true");
        check("cinitchildcaption", "Save-7");
        check("cinitownercaption", "MainForm");
        check("nchildseed", "7");
        check("cchildcaption", "Save-7");
        check("cownercaption", "MainForm");
        check("ldictset", "true");
        check("ndictcompare", "14");

        expect(state.ole_objects.size() == 3U,
               "native external-library ADDOBJECT script should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &parent_object = state.ole_objects[0];
            const auto &child_object = state.ole_objects[1];
            expect(parent_object.prog_id == "DemoForm",
                   "native external-library ADDOBJECT should preserve the parent class identity");
            expect(child_object.prog_id == "SaveButton",
                   "native external-library ADDOBJECT should preserve the child class identity");
            expect(child_object.source == library_path.string(),
                   "native external-library ADDOBJECT should preserve the resolved PRG library path as child provenance");
            expect(parent_object.properties.contains("cmdsave"),
                   "native external-library ADDOBJECT should materialize the child reference on the parent");
            const auto child_parent = child_object.properties.find("parent");
            const auto child_seed = child_object.properties.find("nseed");
            if (child_parent != child_object.properties.end())
            {
                expect(child_parent->second.kind == copperfin::runtime::PrgValueKind::string,
                       "native external-library ADDOBJECT should persist a PARENT object reference on the child");
            }
            else
            {
                expect(false, "native external-library ADDOBJECT should materialize the child PARENT reference");
            }
            if (child_seed != child_object.properties.end())
            {
                expect(copperfin::runtime::format_value(child_seed->second) == "7",
                       "native external-library ADDOBJECT should preserve child Init constructor values");
            }
            else
            {
                expect(false, "native external-library ADDOBJECT should materialize child Init state");
            }

            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM behavior should remain stable while external-library ADDOBJECT lands");
        }

        const bool has_addobject_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.addobject" &&
                   event.detail == "DemoForm.cmdsave:SaveButton";
        });
        expect(has_addobject_event,
               "native external-library ADDOBJECT should emit child materialization events");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_addobject_external_child_base_surfaces_classlibrary_provenance()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_addobject_external_child_base_provenance";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path root_library_path = temp_root / "rootbuttons.prg";
        write_text(
            root_library_path,
            "DEFINE CLASS ParentButton AS Custom\n"
            "ENDDEFINE\n");

        const fs::path button_library_path = temp_root / "buttons.prg";
        write_text(
            button_library_path,
            "DEFINE CLASS SaveButton AS ParentButton OF rootbuttons.prg\n"
            "    Caption = 'Save'\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "native_addobject_external_child_base_provenance.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('DemoForm')\n"
            "oChild = oForm.cmdSave\n"
            "cChildClassLibrary = GETPEM(oChild, 'ClassLibrary')\n"
            "lChildHasClassLibrary = PEMSTATUS(oChild, 'ClassLibrary', 1)\n"
            "lChildClassLibraryReadOnly = PEMSTATUS(oChild, 'ClassLibrary', 5)\n"
            "cChildClassLibraryProp = oChild.ClassLibrary\n"
            "nMembersProps = AMEMBERS(aMembersProps, oChild, 1)\n"
            "cProp4 = aMembersProps[4]\n"
            "lSetChildClassLibrary = SETPEM(oChild, 'ClassLibrary', 'shadow.prg')\n"
            "oChild.ClassLibrary = 'shadow2.prg'\n"
            "cChildClassLibraryAfter = GETPEM(oChild, 'ClassLibrary')\n"
            "cChildClassLibraryPropAfter = oChild.ClassLibrary\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 68)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton', 'buttons.prg')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native ADDOBJECT external child-base provenance script should complete: ") + state.message +
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

        check("cchildclasslibrary", root_library_path.string());
        check("lchildhasclasslibrary", "true");
        check("lchildclasslibraryreadonly", "true");
        check("cchildclasslibraryprop", root_library_path.string());
        check("nmembersprops", "6");
        check("cprop4", "CLASSLIBRARY");
        check("lsetchildclasslibrary", "false");
        check("cchildclasslibraryafter", root_library_path.string());
        check("cchildclasslibrarypropafter", root_library_path.string());
        check("ldictset", "true");
        check("ndictcompare", "68");

        expect(state.ole_objects.size() == 3U,
               "native ADDOBJECT external child-base provenance should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &parent_object = state.ole_objects[0];
            const auto &child_object = state.ole_objects[1];
            expect(parent_object.prog_id == "DemoForm",
                   "native ADDOBJECT external child-base provenance should preserve parent identity");
            expect(child_object.prog_id == "SaveButton",
                   "native ADDOBJECT external child-base provenance should preserve child identity");
            expect(child_object.source == button_library_path.string(),
                   "native ADDOBJECT external child-base provenance should preserve the child definition source path");
            expect(child_object.class_library == root_library_path.string(),
                   "native ADDOBJECT external child-base provenance should preserve the external child ClassLibrary path");
            expect(!child_object.properties.contains("classlibrary"),
                   "native ADDOBJECT external child-base provenance should not materialize a ClassLibrary shadow");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native ADDOBJECT external child-base provenance lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_native_addobject_external_child_base_aclass_reflects_inheritance_chain()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_addobject_external_child_base_aclass";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path root_library_path = temp_root / "rootbuttons.prg";
        write_text(
            root_library_path,
            "DEFINE CLASS ParentButton AS Custom\n"
            "ENDDEFINE\n");

        const fs::path button_library_path = temp_root / "buttons.prg";
        write_text(
            button_library_path,
            "DEFINE CLASS SaveButton AS ParentButton OF rootbuttons.prg\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "native_addobject_external_child_base_aclass.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('DemoForm')\n"
            "oChild = oForm.cmdSave\n"
            "nClassCount = ACLASS(aClass, oChild)\n"
            "cClass1 = aClass[1]\n"
            "cClass2 = aClass[2]\n"
            "cClass3 = aClass[3]\n"
            "cClass4 = aClass[4]\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton', 'buttons.prg')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native ADDOBJECT external child-base ACLASS script should complete: ") + state.message +
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

        check("nclasscount", "4");
        check("cclass1", "SAVEBUTTON");
        check("cclass2", "PARENTBUTTON");
        check("cclass3", "CUSTOM");
        check("cclass4", "OBJECT");

        expect(state.ole_objects.size() == 2U,
               "native ADDOBJECT external child-base ACLASS should register parent and child objects");
        if (state.ole_objects.size() == 2U)
        {
            const auto &child_object = state.ole_objects[1];
            expect(child_object.prog_id == "SaveButton",
                   "native ADDOBJECT external child-base ACLASS should preserve child identity");
            expect(child_object.class_library == root_library_path.string(),
                   "native ADDOBJECT external child-base ACLASS should preserve external child ClassLibrary path");
            expect(child_object.class_hierarchy.size() == 4U,
                   "native ADDOBJECT external child-base ACLASS should preserve runtime child class hierarchy");
            if (child_object.class_hierarchy.size() == 4U)
            {
                expect(child_object.class_hierarchy[0] == "SAVEBUTTON",
                       "native ADDOBJECT external child-base ACLASS should store the derived child class first");
                expect(child_object.class_hierarchy[1] == "PARENTBUTTON",
                       "native ADDOBJECT external child-base ACLASS should store the external parent class second");
                expect(child_object.class_hierarchy[2] == "CUSTOM",
                       "native ADDOBJECT external child-base ACLASS should store the builtin base token");
                expect(child_object.class_hierarchy[3] == "OBJECT",
                       "native ADDOBJECT external child-base ACLASS should store the terminal object token");
            }
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_native_addobject_deeper_external_child_base_aclass_reflects_inheritance_chain()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_addobject_deeper_external_child_base_aclass";
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
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "native_addobject_deeper_external_child_base_aclass.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('DemoForm')\n"
            "oChild = oForm.cmdSave\n"
            "nClassCount = ACLASS(aClass, oChild)\n"
            "cClass1 = aClass[1]\n"
            "cClass2 = aClass[2]\n"
            "cClass3 = aClass[3]\n"
            "cClass4 = aClass[4]\n"
            "cClass5 = aClass[5]\n"
            "xChildClassLibrary = GETPEM(oChild, 'ClassLibrary')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton', 'buttons.prg')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native ADDOBJECT deeper external child-base ACLASS script should complete: ") + state.message +
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

        check("nclasscount", "5");
        check("cclass1", "SAVEBUTTON");
        check("cclass2", "PARENTBUTTON");
        check("cclass3", "ROOTBUTTON");
        check("cclass4", "CUSTOM");
        check("cclass5", "OBJECT");
        const auto child_class_library = state.globals.find("xchildclasslibrary");
        expect(child_class_library != state.globals.end() &&
                   child_class_library->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native ADDOBJECT deeper external child-base ACLASS should leave the derived child ClassLibrary empty");

        expect(state.ole_objects.size() == 2U,
               "native ADDOBJECT deeper external child-base ACLASS should register parent and child objects");
        if (state.ole_objects.size() == 2U)
        {
            const auto &child_object = state.ole_objects[1];
            expect(child_object.prog_id == "SaveButton",
                   "native ADDOBJECT deeper external child-base ACLASS should preserve child identity");
            expect(child_object.source == button_library_path.string(),
                   "native ADDOBJECT deeper external child-base ACLASS should preserve the child definition source path");
            expect(child_object.class_library.empty(),
                   "native ADDOBJECT deeper external child-base ACLASS should leave the derived child ClassLibrary empty");
            expect(child_object.class_hierarchy.size() == 5U,
                   "native ADDOBJECT deeper external child-base ACLASS should preserve the deeper runtime child class hierarchy");
            if (child_object.class_hierarchy.size() == 5U)
            {
                expect(child_object.class_hierarchy[0] == "SAVEBUTTON",
                       "native ADDOBJECT deeper external child-base ACLASS should store the derived child class first");
                expect(child_object.class_hierarchy[1] == "PARENTBUTTON",
                       "native ADDOBJECT deeper external child-base ACLASS should store the intermediate external parent second");
                expect(child_object.class_hierarchy[2] == "ROOTBUTTON",
                       "native ADDOBJECT deeper external child-base ACLASS should store the deeper external ancestor third");
                expect(child_object.class_hierarchy[3] == "CUSTOM",
                       "native ADDOBJECT deeper external child-base ACLASS should store the builtin base token");
                expect(child_object.class_hierarchy[4] == "OBJECT",
                       "native ADDOBJECT deeper external child-base ACLASS should store the terminal object token");
            }
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_native_class_body_add_object_materializes_children_before_init()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_class_body_addobject";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_class_body_addobject.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('DemoForm')\n"
            "oNew = NEWOBJECT('DemoForm')\n"
            "lCreateHasChild = PEMSTATUS(oCreate, 'cmdSave', 1)\n"
            "lNewHasChild = PEMSTATUS(oNew, 'cmdSave', 1)\n"
            "lCreateInitSawChild = oCreate.lInitSawChild\n"
            "lNewInitSawChild = oNew.lInitSawChild\n"
            "cCreateInitChildCaption = oCreate.cInitChildCaption\n"
            "cNewInitChildCaption = oNew.cInitChildCaption\n"
            "cCreateOwnerCaption = oCreate.cmdSave.OwnerCaption()\n"
            "cNewOwnerCaption = oNew.cmdSave.OwnerCaption()\n"
            "lCreateChildHasParent = PEMSTATUS(oCreate.cmdSave, 'Parent', 1)\n"
            "lNewChildHasParent = PEMSTATUS(oNew.cmdSave, 'Parent', 1)\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 13)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    lInitSawChild = .F.\n"
            "    cInitChildCaption = ''\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "    PROCEDURE Init\n"
            "        THIS.lInitSawChild = PEMSTATUS(THIS, 'cmdSave', 1)\n"
            "        IF THIS.lInitSawChild\n"
            "            THIS.cInitChildCaption = THIS.cmdSave.Caption\n"
            "        ENDIF\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
            "    Caption = 'Save'\n"
            "    FUNCTION OwnerCaption\n"
            "        RETURN PARENT.Caption\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native class-body ADD OBJECT script should complete: ") + state.message +
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

        check("lcreatehaschild", "true");
        check("lnewhaschild", "true");
        check("lcreateinitsawchild", "true");
        check("lnewinitsawchild", "true");
        check("ccreateinitchildcaption", "Save");
        check("cnewinitchildcaption", "Save");
        check("ccreateownercaption", "MainForm");
        check("cnewownercaption", "MainForm");
        check("lcreatechildhasparent", "true");
        check("lnewchildhasparent", "true");
        check("ldictset", "true");
        check("ndictcompare", "13");

        expect(state.ole_objects.size() == 5U,
               "native class-body ADD OBJECT script should register CREATEOBJECT, NEWOBJECT, child, child, and COM objects");
        if (state.ole_objects.size() == 5U)
        {
            expect(state.ole_objects[0].prog_id == "DemoForm",
                   "class-body ADD OBJECT should preserve the CREATEOBJECT parent class identity");
            expect(state.ole_objects[1].prog_id == "SaveButton",
                   "class-body ADD OBJECT should materialize the CREATEOBJECT child class");
            expect(state.ole_objects[2].prog_id == "DemoForm",
                   "class-body ADD OBJECT should preserve the NEWOBJECT parent class identity");
            expect(state.ole_objects[3].prog_id == "SaveButton",
                   "class-body ADD OBJECT should materialize the NEWOBJECT child class");
            expect(state.ole_objects[4].prog_id == "Scripting.Dictionary",
                   "COM behavior should remain stable while class-body ADD OBJECT lands");
            expect(state.ole_objects[0].properties.contains("cmdsave"),
                   "class-body ADD OBJECT should materialize the child reference on the CREATEOBJECT parent");
            expect(state.ole_objects[2].properties.contains("cmdsave"),
                   "class-body ADD OBJECT should materialize the child reference on the NEWOBJECT parent");
        }

        const std::size_t addobject_event_count = static_cast<std::size_t>(std::count_if(
            state.events.begin(),
            state.events.end(),
            [](const auto &event)
            {
                return event.category == "prg.object.addobject" &&
                       event.detail == "DemoForm.cmdsave:SaveButton";
            }));
        expect(addobject_event_count == 2U,
               "class-body ADD OBJECT should emit addobject events for both CREATEOBJECT and NEWOBJECT activation");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_class_body_add_object_with_property_clauses_materialize_before_parent_init()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_class_body_addobject_with";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_class_body_addobject_with.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('DemoForm')\n"
            "oNew = NEWOBJECT('DemoForm')\n"
            "cCreateInitChildCaption = oCreate.cInitChildCaption\n"
            "cNewInitChildCaption = oNew.cInitChildCaption\n"
            "nCreateInitChildPriority = oCreate.nInitChildPriority\n"
            "nNewInitChildPriority = oNew.nInitChildPriority\n"
            "cCreateChildCaption = oCreate.cmdSave.Caption\n"
            "cNewChildCaption = oNew.cmdSave.Caption\n"
            "nCreateChildPriority = oCreate.cmdSave.nPriority\n"
            "nNewChildPriority = oNew.cmdSave.nPriority\n"
            "cCreateOwnerCaption = oCreate.cmdSave.OwnerCaption()\n"
            "cNewOwnerCaption = oNew.cmdSave.OwnerCaption()\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 13)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    cInitChildCaption = ''\n"
            "    nInitChildPriority = 0\n"
            "    ADD OBJECT cmdSave AS SaveButton WITH Caption = 'Commit', nPriority = 7\n"
            "    PROCEDURE Init\n"
            "        THIS.cInitChildCaption = THIS.cmdSave.Caption\n"
            "        THIS.nInitChildPriority = THIS.cmdSave.nPriority\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
            "    Caption = 'Save'\n"
            "    FUNCTION OwnerCaption\n"
            "        RETURN PARENT.Caption\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native class-body ADD OBJECT WITH script should complete: ") + state.message +
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

        check("ccreateinitchildcaption", "Commit");
        check("cnewinitchildcaption", "Commit");
        check("ncreateinitchildpriority", "7");
        check("nnewinitchildpriority", "7");
        check("ccreatechildcaption", "Commit");
        check("cnewchildcaption", "Commit");
        check("ncreatechildpriority", "7");
        check("nnewchildpriority", "7");
        check("ccreateownercaption", "MainForm");
        check("cnewownercaption", "MainForm");
        check("ldictset", "true");
        check("ndictcompare", "13");

        expect(state.ole_objects.size() == 5U,
               "native class-body ADD OBJECT WITH script should register CREATEOBJECT, NEWOBJECT, child, child, and COM objects");
        if (state.ole_objects.size() == 5U)
        {
            expect(state.ole_objects[1].prog_id == "SaveButton",
                   "class-body ADD OBJECT WITH should materialize the CREATEOBJECT child class");
            expect(state.ole_objects[3].prog_id == "SaveButton",
                   "class-body ADD OBJECT WITH should materialize the NEWOBJECT child class");
            const auto create_child_caption = state.ole_objects[1].properties.find("caption");
            const auto create_child_priority = state.ole_objects[1].properties.find("npriority");
            const auto new_child_caption = state.ole_objects[3].properties.find("caption");
            const auto new_child_priority = state.ole_objects[3].properties.find("npriority");
            expect(create_child_caption != state.ole_objects[1].properties.end() &&
                       copperfin::runtime::format_value(create_child_caption->second) == "Commit",
                   "class-body ADD OBJECT WITH should apply caption override to the CREATEOBJECT child");
            expect(create_child_priority != state.ole_objects[1].properties.end() &&
                       copperfin::runtime::format_value(create_child_priority->second) == "7",
                   "class-body ADD OBJECT WITH should apply numeric override to the CREATEOBJECT child");
            expect(new_child_caption != state.ole_objects[3].properties.end() &&
                       copperfin::runtime::format_value(new_child_caption->second) == "Commit",
                   "class-body ADD OBJECT WITH should apply caption override to the NEWOBJECT child");
            expect(new_child_priority != state.ole_objects[3].properties.end() &&
                       copperfin::runtime::format_value(new_child_priority->second) == "7",
                   "class-body ADD OBJECT WITH should apply numeric override to the NEWOBJECT child");
            expect(state.ole_objects[4].prog_id == "Scripting.Dictionary",
                   "COM behavior should remain stable while class-body ADD OBJECT WITH lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_native_class_body_object_blocks_materialize_children_before_parent_init()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_class_body_object_block";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_class_body_object_block.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('DemoForm')\n"
            "oNew = NEWOBJECT('DemoForm')\n"
            "cCreateInitChildCaption = oCreate.cInitChildCaption\n"
            "cNewInitChildCaption = oNew.cInitChildCaption\n"
            "nCreateInitChildPriority = oCreate.nInitChildPriority\n"
            "nNewInitChildPriority = oNew.nInitChildPriority\n"
            "cCreateChildCaption = oCreate.cmdSave.Caption\n"
            "cNewChildCaption = oNew.cmdSave.Caption\n"
            "nCreateChildPriority = oCreate.cmdSave.nPriority\n"
            "nNewChildPriority = oNew.cmdSave.nPriority\n"
            "cCreateOwnerCaption = oCreate.cmdSave.OwnerCaption()\n"
            "cNewOwnerCaption = oNew.cmdSave.OwnerCaption()\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 13)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    cInitChildCaption = ''\n"
            "    nInitChildPriority = 0\n"
            "    OBJECT cmdSave AS SaveButton\n"
            "        Caption = 'Commit'\n"
            "        nPriority = 7\n"
            "    ENDOBJECT\n"
            "    PROCEDURE Init\n"
            "        THIS.cInitChildCaption = THIS.cmdSave.Caption\n"
            "        THIS.nInitChildPriority = THIS.cmdSave.nPriority\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
            "    Caption = 'Save'\n"
            "    FUNCTION OwnerCaption\n"
            "        RETURN PARENT.Caption\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native class-body OBJECT block script should complete: ") + state.message +
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

        check("ccreateinitchildcaption", "Commit");
        check("cnewinitchildcaption", "Commit");
        check("ncreateinitchildpriority", "7");
        check("nnewinitchildpriority", "7");
        check("ccreatechildcaption", "Commit");
        check("cnewchildcaption", "Commit");
        check("ncreatechildpriority", "7");
        check("nnewchildpriority", "7");
        check("ccreateownercaption", "MainForm");
        check("cnewownercaption", "MainForm");
        check("ldictset", "true");
        check("ndictcompare", "13");

        expect(state.ole_objects.size() == 5U,
               "native class-body OBJECT block script should register CREATEOBJECT, NEWOBJECT, child, child, and COM objects");
        if (state.ole_objects.size() == 5U)
        {
            expect(state.ole_objects[1].prog_id == "SaveButton",
                   "class-body OBJECT block should materialize the CREATEOBJECT child class");
            expect(state.ole_objects[3].prog_id == "SaveButton",
                   "class-body OBJECT block should materialize the NEWOBJECT child class");
            const auto create_child_caption = state.ole_objects[1].properties.find("caption");
            const auto create_child_priority = state.ole_objects[1].properties.find("npriority");
            const auto new_child_caption = state.ole_objects[3].properties.find("caption");
            const auto new_child_priority = state.ole_objects[3].properties.find("npriority");
            expect(create_child_caption != state.ole_objects[1].properties.end() &&
                       copperfin::runtime::format_value(create_child_caption->second) == "Commit",
                   "class-body OBJECT block should apply caption override to the CREATEOBJECT child");
            expect(create_child_priority != state.ole_objects[1].properties.end() &&
                       copperfin::runtime::format_value(create_child_priority->second) == "7",
                   "class-body OBJECT block should apply numeric override to the CREATEOBJECT child");
            expect(new_child_caption != state.ole_objects[3].properties.end() &&
                       copperfin::runtime::format_value(new_child_caption->second) == "Commit",
                   "class-body OBJECT block should apply caption override to the NEWOBJECT child");
            expect(new_child_priority != state.ole_objects[3].properties.end() &&
                       copperfin::runtime::format_value(new_child_priority->second) == "7",
                   "class-body OBJECT block should apply numeric override to the NEWOBJECT child");
            expect(state.ole_objects[4].prog_id == "Scripting.Dictionary",
                   "COM behavior should remain stable while class-body OBJECT block lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_declarative_children_materialize_from_external_prg_sources_before_parent_init()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_declarative_external_children";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path library_path = temp_root / "buttons.prg";
        write_text(
            library_path,
            "DEFINE CLASS SaveButton AS Custom\n"
            "    Caption = 'Save'\n"
            "    FUNCTION OwnerCaption\n"
            "        RETURN PARENT.Caption\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ArchiveButton AS Custom\n"
            "    Caption = 'Archive'\n"
            "    FUNCTION OwnerCaption\n"
            "        RETURN PARENT.Caption\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "native_declarative_external_children.prg";
        write_text(
            main_path,
            "oAdd = CREATEOBJECT('DemoFormAdd')\n"
            "oBlock = NEWOBJECT('DemoFormBlock')\n"
            "cAddInitChildCaption = oAdd.cInitChildCaption\n"
            "cBlockInitChildCaption = oBlock.cInitChildCaption\n"
            "cAddChildCaption = oAdd.cmdSave.Caption\n"
            "cBlockChildCaption = oBlock.cmdArchive.Caption\n"
            "cAddOwnerCaption = oAdd.cmdSave.OwnerCaption()\n"
            "cBlockOwnerCaption = oBlock.cmdArchive.OwnerCaption()\n"
            "lAddChildHasParent = PEMSTATUS(oAdd.cmdSave, 'Parent', 1)\n"
            "lBlockChildHasParent = PEMSTATUS(oBlock.cmdArchive, 'Parent', 1)\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 13)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoFormAdd AS Custom\n"
            "    Caption = 'MainFormAdd'\n"
            "    cInitChildCaption = ''\n"
            "    ADD OBJECT cmdSave AS SaveButton OF buttons.prg\n"
            "    PROCEDURE Init\n"
            "        THIS.cInitChildCaption = THIS.cmdSave.Caption\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS DemoFormBlock AS Custom\n"
            "    Caption = 'MainFormBlock'\n"
            "    cInitChildCaption = ''\n"
            "    OBJECT cmdArchive AS ArchiveButton OF buttons.prg\n"
            "        Caption = 'ArchiveNow'\n"
            "    ENDOBJECT\n"
            "    PROCEDURE Init\n"
            "        THIS.cInitChildCaption = THIS.cmdArchive.Caption\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("declarative external child script should complete: ") + state.message +
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

        check("caddinitchildcaption", "Save");
        check("cblockinitchildcaption", "ArchiveNow");
        check("caddchildcaption", "Save");
        check("cblockchildcaption", "ArchiveNow");
        check("caddownercaption", "MainFormAdd");
        check("cblockownercaption", "MainFormBlock");
        check("laddchildhasparent", "true");
        check("lblockchildhasparent", "true");
        check("ldictset", "true");
        check("ndictcompare", "13");

        expect(state.ole_objects.size() == 5U,
               "declarative external child script should register CREATEOBJECT parent/child, NEWOBJECT parent/child, and COM objects");
        if (state.ole_objects.size() == 5U)
        {
            expect(state.ole_objects[0].prog_id == "DemoFormAdd",
                   "declarative external child script should preserve the ADD parent class identity");
            expect(state.ole_objects[1].prog_id == "SaveButton",
                   "declarative external child script should materialize the one-line external child class");
            expect(state.ole_objects[1].source == library_path.string(),
                   "declarative external child script should preserve the resolved one-line external child source path");
            expect(state.ole_objects[2].prog_id == "DemoFormBlock",
                   "declarative external child script should preserve the block parent class identity");
            expect(state.ole_objects[3].prog_id == "ArchiveButton",
                   "declarative external child script should materialize the block external child class");
            expect(state.ole_objects[3].source == library_path.string(),
                   "declarative external child script should preserve the resolved block external child source path");
            const auto block_child_caption = state.ole_objects[3].properties.find("caption");
            expect(block_child_caption != state.ole_objects[3].properties.end() &&
                       copperfin::runtime::format_value(block_child_caption->second) == "ArchiveNow",
                   "declarative external child script should still apply block child property overrides from the parent PRG");
            expect(state.ole_objects[4].prog_id == "Scripting.Dictionary",
                   "COM behavior should remain stable while declarative external child activation lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_native_child_methods_resolve_thisform_through_parent_chain()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_thisform";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_thisform.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('DemoForm')\n"
            "oChild = oForm.cmdSave\n"
            "cOwnerCaption = oChild.OwnerCaption()\n"
            "cSavedCaption = oChild.TriggerSave()\n"
            "cFormCaptionAfterSave = oForm.Caption\n"
            "oFormRef = oChild.OwnerRef()\n"
            "cOwnerRefCaption = oFormRef.Caption\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "    FUNCTION Save\n"
            "        THIS.Caption = THIS.Caption + '-Saved'\n"
            "        RETURN THIS.Caption\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
            "    FUNCTION OwnerCaption\n"
            "        RETURN THISFORM.Caption\n"
            "    ENDFUNC\n"
            "    FUNCTION TriggerSave\n"
            "        RETURN THISFORM.Save()\n"
            "    ENDFUNC\n"
            "    FUNCTION OwnerRef\n"
            "        RETURN THISFORM\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native THISFORM script should complete: ") + state.message +
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

        check("cownercaption", "MainForm");
        check("csavedcaption", "MainForm-Saved");
        check("cformcaptionaftersave", "MainForm-Saved");
        check("cownerrefcaption", "MainForm-Saved");

        expect(state.ole_objects.size() == 2U,
               "native THISFORM script should register parent and child objects");
        if (state.ole_objects.size() == 2U)
        {
            const auto &parent_object = state.ole_objects[0];
            const auto caption = parent_object.properties.find("caption");
            if (caption != parent_object.properties.end())
            {
                expect(copperfin::runtime::format_value(caption->second) == "MainForm-Saved",
                       "native THISFORM should let child methods update the owning form");
            }
            else
            {
                expect(false, "native THISFORM should preserve the owning form caption property");
            }
        }

        const bool has_save_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "DemoForm.Save";
        });
        expect(has_save_invoke_event,
               "native THISFORM should route child method calls into parent methods");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_child_methods_resolve_thisformset_through_owner_chain()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_thisformset";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_thisformset.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('DemoForm')\n"
            "oChild = oForm.cmdSave\n"
            "cOwnerCaption = oChild.OwnerCaption()\n"
            "cSavedCaption = oChild.TriggerSave()\n"
            "cFormCaptionAfterSave = oForm.Caption\n"
            "oFormRef = oChild.OwnerRef()\n"
            "cOwnerRefCaption = oFormRef.Caption\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "    FUNCTION Save\n"
            "        THIS.Caption = THIS.Caption + '-Saved'\n"
            "        RETURN THIS.Caption\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
            "    FUNCTION OwnerCaption\n"
            "        RETURN THISFORMSET.Caption\n"
            "    ENDFUNC\n"
            "    FUNCTION TriggerSave\n"
            "        RETURN THISFORMSET.Save()\n"
            "    ENDFUNC\n"
            "    FUNCTION OwnerRef\n"
            "        RETURN THISFORMSET\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native THISFORMSET script should complete: ") + state.message +
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

        check("cownercaption", "MainForm");
        check("csavedcaption", "MainForm-Saved");
        check("cformcaptionaftersave", "MainForm-Saved");
        check("cownerrefcaption", "MainForm-Saved");

        expect(state.ole_objects.size() == 2U,
               "native THISFORMSET script should register parent and child objects");
        if (state.ole_objects.size() == 2U)
        {
            const auto &parent_object = state.ole_objects[0];
            const auto caption = parent_object.properties.find("caption");
            if (caption != parent_object.properties.end())
            {
                expect(copperfin::runtime::format_value(caption->second) == "MainForm-Saved",
                       "native THISFORMSET should let child methods update the owning form");
            }
            else
            {
                expect(false, "native THISFORMSET should preserve the owning form caption property");
            }
        }

        const bool has_save_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "DemoForm.Save";
        });
        expect(has_save_invoke_event,
               "native THISFORMSET should route child method calls into parent methods");

        fs::remove_all(temp_root, ignored);
    }

    void test_external_prg_base_child_methods_resolve_parent_thisform_and_thisformset()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_prg_child_method_ownership";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path library_path = temp_root / "buttons.prg";
        write_text(
            library_path,
            "DEFINE CLASS ParentButton AS Custom\n"
            "    FUNCTION ParentCaption\n"
            "        RETURN PARENT.Caption\n"
            "    ENDFUNC\n"
            "    FUNCTION FormCaption\n"
            "        RETURN THISFORM.Caption\n"
            "    ENDFUNC\n"
            "    FUNCTION FormsetCaption\n"
            "        RETURN THISFORMSET.Caption\n"
            "    ENDFUNC\n"
            "    FUNCTION TriggerFormSave\n"
            "        RETURN THISFORM.Save()\n"
            "    ENDFUNC\n"
            "    FUNCTION TriggerFormsetSave\n"
            "        RETURN THISFORMSET.Save()\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_child_method_ownership.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('DemoForm')\n"
            "oChild = oForm.cmdSave\n"
            "cParentCaption = oChild.ParentCaption()\n"
            "cFormCaption = oChild.FormCaption()\n"
            "cFormsetCaption = oChild.FormsetCaption()\n"
            "cSavedCaption = oChild.TriggerFormSave()\n"
            "cFormCaptionAfterSave = oForm.Caption\n"
            "cSavedCaption2 = oChild.TriggerFormsetSave()\n"
            "cFormCaptionAfterSave2 = oForm.Caption\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "    FUNCTION Save\n"
            "        THIS.Caption = THIS.Caption + '-Saved'\n"
            "        RETURN THIS.Caption\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS ParentButton OF buttons.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external-base child ownership script should complete: ") + state.message +
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

        check("cparentcaption", "MainForm");
        check("cformcaption", "MainForm");
        check("cformsetcaption", "MainForm");
        check("csavedcaption", "MainForm-Saved");
        check("cformcaptionaftersave", "MainForm-Saved");
        check("csavedcaption2", "MainForm-Saved-Saved");
        check("cformcaptionaftersave2", "MainForm-Saved-Saved");

        expect(state.ole_objects.size() == 2U,
               "external-base child ownership script should register parent and child objects");
        if (state.ole_objects.size() == 2U)
        {
            const auto &parent_object = state.ole_objects[0];
            const auto &child_object = state.ole_objects[1];
            expect(parent_object.prog_id == "DemoForm",
                   "external-base child ownership should preserve parent class identity");
            expect(child_object.prog_id == "SaveButton",
                   "external-base child ownership should preserve child class identity");
            const auto caption = parent_object.properties.find("caption");
            if (caption != parent_object.properties.end())
            {
                expect(copperfin::runtime::format_value(caption->second) == "MainForm-Saved-Saved",
                       "external-base child ownership should let inherited child methods update the owning form");
            }
            else
            {
                expect(false, "external-base child ownership should preserve the owning form caption property");
            }
        }

        const bool has_save_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "DemoForm.Save";
        });
        expect(has_save_invoke_event,
               "external-base child ownership should route inherited child method calls into parent methods");

        fs::remove_all(temp_root, ignored);
    }

    void test_dotted_native_child_chains_traverse_contained_objects()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_child_chain";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_child_chain.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('DemoForm')\n"
            "cChildCaption = oForm.cmdSave.Caption\n"
            "oParent = oForm.cmdSave.Parent\n"
            "cParentCaption = oForm.cmdSave.Parent.Caption\n"
            "cOwnerCaption = oForm.cmdSave.OwnerCaption()\n"
            "cSavedCaption = oForm.cmdSave.TriggerSave()\n"
            "cFormCaptionAfterSave = oForm.Caption\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 14)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "    FUNCTION Save\n"
            "        THIS.Caption = THIS.Caption + '-Saved'\n"
            "        RETURN THIS.Caption\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
            "    Caption = 'Save'\n"
            "    FUNCTION OwnerCaption\n"
            "        RETURN PARENT.Caption\n"
            "    ENDFUNC\n"
            "    FUNCTION TriggerSave\n"
            "        RETURN THISFORM.Save()\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native child-chain script should complete: ") + state.message +
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

        check("cchildcaption", "Save");
        check("cparentcaption", "MainForm");
        check("cownercaption", "MainForm");
        check("csavedcaption", "MainForm-Saved");
        check("cformcaptionaftersave", "MainForm-Saved");
        check("ldictset", "true");
        check("ndictcompare", "14");

        expect(state.ole_objects.size() == 3U,
               "native child-chain script should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto parent_caption = state.ole_objects[0].properties.find("caption");
            if (parent_caption != state.ole_objects[0].properties.end())
            {
                expect(copperfin::runtime::format_value(parent_caption->second) == "MainForm-Saved",
                       "native child-chain traversal should let dotted child method calls update the owning form");
            }
            else
            {
                expect(false, "native child-chain traversal should preserve the owning form caption");
            }
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM behavior should remain stable while native child-chain traversal lands");
        }

        const bool has_child_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "SaveButton.OwnerCaption";
        });
        expect(has_child_invoke_event,
               "native child-chain traversal should dispatch direct dotted child method calls");

        fs::remove_all(temp_root, ignored);
    }

    void test_dotted_native_child_assignments_traverse_contained_objects()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_child_chain_assignment";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_child_chain_assignment.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('DemoForm')\n"
            "oForm.cmdSave.Caption = 'Go'\n"
            "oForm.cmdSave.Parent.Caption = 'Done'\n"
            "cChildCaption = oForm.cmdSave.Caption\n"
            "cParentCaption = oForm.Caption\n"
            "cOwnerCaption = oForm.cmdSave.OwnerCaption()\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 15)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
            "    Caption = 'Save'\n"
            "    FUNCTION OwnerCaption\n"
            "        RETURN PARENT.Caption\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native child-chain assignment script should complete: ") + state.message +
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

        check("cchildcaption", "Go");
        check("cparentcaption", "Done");
        check("cownercaption", "Done");
        check("ldictset", "true");
        check("ndictcompare", "15");

        expect(state.ole_objects.size() == 3U,
               "native child-chain assignment script should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto parent_caption = state.ole_objects[0].properties.find("caption");
            const auto child_caption = state.ole_objects[1].properties.find("caption");
            if (parent_caption != state.ole_objects[0].properties.end())
            {
                expect(copperfin::runtime::format_value(parent_caption->second) == "Done",
                       "native child-chain assignment should update the parent through the dotted chain");
            }
            else
            {
                expect(false, "native child-chain assignment should preserve the parent caption property");
            }
            if (child_caption != state.ole_objects[1].properties.end())
            {
                expect(copperfin::runtime::format_value(child_caption->second) == "Go",
                       "native child-chain assignment should update the contained child through the dotted chain");
            }
            else
            {
                expect(false, "native child-chain assignment should preserve the child caption property");
            }
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM behavior should remain stable while native child-chain assignment lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_native_removeobject_detaches_child_and_clears_parent_reference()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_removeobject";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_removeobject.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('DemoForm')\n"
            "oChild = oForm.cmdSave\n"
            "cChildCaptionBeforeRemove = oChild.Caption\n"
            "lRemoved = oForm.RemoveObject('cmdSave')\n"
            "lRemovedMissing = oForm.RemoveObject('cmdSave')\n"
            "lHasChildAfterRemove = PEMSTATUS(oForm, 'cmdSave', 1)\n"
            "xRemovedChild = GETPEM(oForm, 'cmdSave')\n"
            "lChildHasParentAfterRemove = PEMSTATUS(oChild, 'Parent', 1)\n"
            "cChildCaptionAfterRemove = oChild.Caption\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 16)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
            "    Caption = 'Save'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native REMOVEOBJECT script should complete: ") + state.message +
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

        check("cchildcaptionbeforeremove", "Save");
        check("lremoved", "true");
        check("lremovedmissing", "false");
        check("lhaschildafterremove", "false");
        check("lchildhasparentafterremove", "false");
        check("cchildcaptionafterremove", "Save");
        check("ldictset", "true");
        check("ndictcompare", "16");

        const auto removed_child = state.globals.find("xremovedchild");
        expect(removed_child != state.globals.end() &&
                   removed_child->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native REMOVEOBJECT should make GETPEM() return empty for the removed child");

        expect(state.ole_objects.size() == 3U,
               "native REMOVEOBJECT script should register parent, detached child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            expect(!state.ole_objects[0].properties.contains("cmdsave"),
                   "native REMOVEOBJECT should remove the child reference from the parent");
            expect(!state.ole_objects[1].properties.contains("parent"),
                   "native REMOVEOBJECT should clear the detached child's parent reference");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM behavior should remain stable while native REMOVEOBJECT lands");
        }

        const bool has_removeobject_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.removeobject" &&
                   event.detail == "DemoForm.cmdsave";
        });
        expect(has_removeobject_event,
               "native REMOVEOBJECT should emit detachment events");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_release_invokes_destroy_and_invalidates_standalone_object()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_release_standalone";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_release_standalone.prg";
        write_text(
            main_path,
            "nDestroyCount = 0\n"
            "oWidget = CREATEOBJECT('Widget')\n"
            "cCaptionBeforeRelease = oWidget.Caption\n"
            "lHadCaptionBeforeRelease = PEMSTATUS(oWidget, 'Caption', 1)\n"
            "lReleased = oWidget.Release()\n"
            "nDestroyCountAfter = nDestroyCount\n"
            "lHasCaptionAfterRelease = PEMSTATUS(oWidget, 'Caption', 1)\n"
            "xCaptionAfterRelease = GETPEM(oWidget, 'Caption')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 90)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS Widget AS Custom\n"
            "    Caption = 'Standalone'\n"
            "    PROCEDURE Destroy\n"
            "        nDestroyCount = nDestroyCount + 1\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Release standalone script should complete: ") + state.message +
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

        check("ccaptionbeforerelease", "Standalone");
        check("lhadcaptionbeforerelease", "true");
        check("lreleased", "true");
        check("ndestroycountafter", "1");
        check("lhascaptionafterrelease", "false");
        check("ldictset", "true");
        check("ndictcompare", "90");

        const auto caption_after_release = state.globals.find("xcaptionafterrelease");
        expect(caption_after_release != state.globals.end() &&
                   caption_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native Release standalone should invalidate GETPEM() on the released object");

        expect(state.ole_objects.size() == 1U,
               "native Release standalone should leave only the COM object registered after release");
        if (state.ole_objects.size() == 1U)
        {
            expect(state.ole_objects[0].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native standalone Release lands");
        }

        const bool has_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "Widget.Destroy";
        });
        expect(has_destroy_event,
               "native Release standalone should dispatch the native Destroy method");

        const bool has_release_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.release" &&
                   event.detail == "Widget";
        });
        expect(has_release_event,
               "native Release standalone should emit a release event");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_release_recursively_destroys_contained_children_and_invalidates_owner_chain()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_release_contained";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_release_contained.prg";
        write_text(
            main_path,
            "cDestroyLog = ''\n"
            "cChildParentCaptionDuringDestroy = ''\n"
            "nFormDestroyCount = 0\n"
            "nChildDestroyCount = 0\n"
            "oForm = CREATEOBJECT('DemoForm')\n"
            "oChild = oForm.cmdSave\n"
            "lHasChildBeforeRelease = PEMSTATUS(oForm, 'cmdSave', 1)\n"
            "cChildCaptionBeforeRelease = oChild.Caption\n"
            "lReleased = oForm.Release()\n"
            "cDestroyLogAfter = cDestroyLog\n"
            "cChildParentCaptionAfter = cChildParentCaptionDuringDestroy\n"
            "nFormDestroyCountAfter = nFormDestroyCount\n"
            "nChildDestroyCountAfter = nChildDestroyCount\n"
            "lHasChildAfterRelease = PEMSTATUS(oForm, 'cmdSave', 1)\n"
            "xChildAfterRelease = GETPEM(oForm, 'cmdSave')\n"
            "lChildHasParentAfterRelease = PEMSTATUS(oChild, 'Parent', 1)\n"
            "xChildParentAfterRelease = GETPEM(oChild, 'Parent')\n"
            "lFormHasCaptionAfterRelease = PEMSTATUS(oForm, 'Caption', 1)\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 91)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "    PROCEDURE Destroy\n"
            "        cDestroyLog = cDestroyLog + 'form>'\n"
            "        nFormDestroyCount = nFormDestroyCount + 1\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
            "    Caption = 'Save'\n"
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
               std::string("native Release contained script should complete: ") + state.message +
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

        check("lhaschildbeforerelease", "true");
        check("cchildcaptionbeforerelease", "Save");
        check("lreleased", "true");
        check("cdestroylogafter", "child>form>");
        check("cchildparentcaptionafter", "MainForm");
        check("nformdestroycountafter", "1");
        check("nchilddestroycountafter", "1");
        check("lhaschildafterrelease", "false");
        check("lchildhasparentafterrelease", "false");
        check("lformhascaptionafterrelease", "false");
        check("ldictset", "true");
        check("ndictcompare", "91");

        const auto child_after_release = state.globals.find("xchildafterrelease");
        expect(child_after_release != state.globals.end() &&
                   child_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native Release contained should invalidate GETPEM() for the released child member");

        const auto child_parent_after_release = state.globals.find("xchildparentafterrelease");
        expect(child_parent_after_release != state.globals.end() &&
                   child_parent_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native Release contained should invalidate GETPEM() for the released child's Parent");

        expect(state.ole_objects.size() == 1U,
               "native Release contained should leave only the COM object registered after recursive release");
        if (state.ole_objects.size() == 1U)
        {
            expect(state.ole_objects[0].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native contained Release lands");
        }

        const bool has_child_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "SaveButton.Destroy";
        });
        expect(has_child_destroy_event,
               "native Release contained should dispatch the child Destroy method");

        const bool has_form_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "DemoForm.Destroy";
        });
        expect(has_form_destroy_event,
               "native Release contained should dispatch the parent Destroy method");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_release_detaches_contained_child_while_owner_and_sibling_remain_alive()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_release_direct_child";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_release_direct_child.prg";
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
            "oSibling = oForm.cmdCancel\n"
            "oGrandchild = oChild.lblBadge\n"
            "cOwnerCaptionBeforeRelease = oForm.Caption\n"
            "cSiblingCaptionBeforeRelease = oSibling.Caption\n"
            "cGrandchildCaptionBeforeRelease = oGrandchild.Caption\n"
            "lReleased = oChild.Release()\n"
            "cDestroyLogAfter = cDestroyLog\n"
            "cChildParentCaptionAfter = cChildParentCaptionDuringDestroy\n"
            "cGrandchildParentCaptionAfter = cGrandchildParentCaptionDuringDestroy\n"
            "nFormDestroyCountAfter = nFormDestroyCount\n"
            "nChildDestroyCountAfter = nChildDestroyCount\n"
            "nGrandchildDestroyCountAfter = nGrandchildDestroyCount\n"
            "cOwnerCaptionAfterRelease = oForm.Caption\n"
            "lOwnerStillHasSave = PEMSTATUS(oForm, 'cmdSave', 1)\n"
            "xReleasedChild = GETPEM(oForm, 'cmdSave')\n"
            "lOwnerStillHasCancel = PEMSTATUS(oForm, 'cmdCancel', 1)\n"
            "cSiblingCaptionAfterRelease = oForm.cmdCancel.Caption\n"
            "lHeldChildHasParentAfterRelease = PEMSTATUS(oChild, 'Parent', 1)\n"
            "xHeldChildParentAfterRelease = GETPEM(oChild, 'Parent')\n"
            "lHeldChildHasCaptionAfterRelease = PEMSTATUS(oChild, 'Caption', 1)\n"
            "xHeldChildCaptionAfterRelease = GETPEM(oChild, 'Caption')\n"
            "lHeldGrandchildHasParentAfterRelease = PEMSTATUS(oGrandchild, 'Parent', 1)\n"
            "xHeldGrandchildParentAfterRelease = GETPEM(oGrandchild, 'Parent')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 92)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton')\n"
            "        THIS.AddObject('cmdCancel', 'CancelButton')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "    PROCEDURE Destroy\n"
            "        nFormDestroyCount = nFormDestroyCount + 1\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
            "    Caption = 'Save'\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('lblBadge', 'BadgeLabel')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "    PROCEDURE Destroy\n"
            "        cDestroyLog = cDestroyLog + 'child>'\n"
            "        cChildParentCaptionDuringDestroy = PARENT.Caption\n"
            "        nChildDestroyCount = nChildDestroyCount + 1\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS CancelButton AS Custom\n"
            "    Caption = 'Cancel'\n"
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

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Release direct-child script should complete: ") + state.message +
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

        check("cownercaptionbeforerelease", "MainForm");
        check("csiblingcaptionbeforerelease", "Cancel");
        check("cgrandchildcaptionbeforerelease", "Badge");
        check("lreleased", "true");
        check("cdestroylogafter", "badge>child>");
        check("cchildparentcaptionafter", "MainForm");
        check("cgrandchildparentcaptionafter", "Save");
        check("nformdestroycountafter", "0");
        check("nchilddestroycountafter", "1");
        check("ngrandchilddestroycountafter", "1");
        check("cownercaptionafterrelease", "MainForm");
        check("lownerstillhassave", "false");
        check("lownerstillhascancel", "true");
        check("csiblingcaptionafterrelease", "Cancel");
        check("lheldchildhasparentafterrelease", "false");
        check("lheldchildhascaptionafterrelease", "false");
        check("lheldgrandchildhasparentafterrelease", "false");
        check("ldictset", "true");
        check("ndictcompare", "92");

        const auto released_child = state.globals.find("xreleasedchild");
        expect(released_child != state.globals.end() &&
                   released_child->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native Release direct child should invalidate GETPEM() on the owner's released child slot");

        const auto held_child_parent = state.globals.find("xheldchildparentafterrelease");
        expect(held_child_parent != state.globals.end() &&
                   held_child_parent->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native Release direct child should invalidate GETPEM() on the held child's Parent");

        const auto held_child_caption = state.globals.find("xheldchildcaptionafterrelease");
        expect(held_child_caption != state.globals.end() &&
                   held_child_caption->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native Release direct child should invalidate GETPEM() on the held released child");

        const auto held_grandchild_parent = state.globals.find("xheldgrandchildparentafterrelease");
        expect(held_grandchild_parent != state.globals.end() &&
                   held_grandchild_parent->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native Release direct child should invalidate GETPEM() on the released grandchild's Parent");

        expect(state.ole_objects.size() == 3U,
               "native Release direct child should leave owner, surviving sibling, and COM objects registered");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "DemoForm",
                   "native Release direct child should keep the owner object alive");
            expect(!state.ole_objects[0].properties.contains("cmdsave"),
                   "native Release direct child should detach the released child from the owner");
            expect(state.ole_objects[0].properties.contains("cmdcancel"),
                   "native Release direct child should preserve the surviving sibling on the owner");
            expect(state.ole_objects[1].prog_id == "CancelButton",
                   "native Release direct child should preserve the surviving sibling object");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native direct-child Release lands");
        }

        const bool has_child_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "SaveButton.Destroy";
        });
        expect(has_child_destroy_event,
               "native Release direct child should dispatch the child Destroy method");

        const bool has_grandchild_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "BadgeLabel.Destroy";
        });
        expect(has_grandchild_destroy_event,
               "native Release direct child should dispatch descendant Destroy methods");

        const bool has_form_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "DemoForm.Destroy";
        });
        expect(!has_form_destroy_event,
               "native Release direct child should not destroy the owner");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_release_detaches_external_base_contained_child_while_owner_and_sibling_remain_alive()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_release_external_base_child";
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

        const fs::path main_path = temp_root / "native_release_external_base_child.prg";
        write_text(
            main_path,
            "cDestroyLog = ''\n"
            "cChildParentCaptionDuringDestroy = ''\n"
            "nChildDestroyCount = 0\n"
            "oForm = CREATEOBJECT('DemoForm')\n"
            "oChild = oForm.cmdSave\n"
            "oSibling = oForm.cmdCancel\n"
            "cChildClassLibraryBeforeRelease = oChild.ClassLibrary\n"
            "cSiblingCaptionBeforeRelease = oSibling.Caption\n"
            "lReleased = oChild.Release()\n"
            "cDestroyLogAfter = cDestroyLog\n"
            "cChildParentCaptionAfter = cChildParentCaptionDuringDestroy\n"
            "nChildDestroyCountAfter = nChildDestroyCount\n"
            "cOwnerCaptionAfterRelease = oForm.Caption\n"
            "lOwnerStillHasSave = PEMSTATUS(oForm, 'cmdSave', 1)\n"
            "xReleasedChild = GETPEM(oForm, 'cmdSave')\n"
            "lOwnerStillHasCancel = PEMSTATUS(oForm, 'cmdCancel', 1)\n"
            "cSiblingCaptionAfterRelease = oForm.cmdCancel.Caption\n"
            "lHeldChildHasParentAfterRelease = PEMSTATUS(oChild, 'Parent', 1)\n"
            "xHeldChildParentAfterRelease = GETPEM(oChild, 'Parent')\n"
            "lHeldChildHasClassLibraryAfterRelease = PEMSTATUS(oChild, 'ClassLibrary', 1)\n"
            "xHeldChildClassLibraryAfterRelease = GETPEM(oChild, 'ClassLibrary')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 93)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton')\n"
            "        THIS.AddObject('cmdCancel', 'CancelButton')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS ParentButton OF buttons.prg\n"
            "    Caption = 'Save'\n"
            "    PROCEDURE Destroy\n"
            "        cDestroyLog = cDestroyLog + 'child>'\n"
            "        cChildParentCaptionDuringDestroy = PARENT.Caption\n"
            "        nChildDestroyCount = nChildDestroyCount + 1\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS CancelButton AS Custom\n"
            "    Caption = 'Cancel'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Release external-base child script should complete: ") + state.message +
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

        check("cchildclasslibrarybeforerelease", button_library_path.string());
        check("csiblingcaptionbeforerelease", "Cancel");
        check("lreleased", "true");
        check("cdestroylogafter", "child>");
        check("cchildparentcaptionafter", "MainForm");
        check("nchilddestroycountafter", "1");
        check("cownercaptionafterrelease", "MainForm");
        check("lownerstillhassave", "false");
        check("lownerstillhascancel", "true");
        check("csiblingcaptionafterrelease", "Cancel");
        check("lheldchildhasparentafterrelease", "false");
        check("lheldchildhasclasslibraryafterrelease", "false");
        check("ldictset", "true");
        check("ndictcompare", "93");

        const auto released_child = state.globals.find("xreleasedchild");
        expect(released_child != state.globals.end() &&
                   released_child->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native Release external-base child should invalidate GETPEM() on the owner's released child slot");

        const auto held_child_parent = state.globals.find("xheldchildparentafterrelease");
        expect(held_child_parent != state.globals.end() &&
                   held_child_parent->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native Release external-base child should invalidate GETPEM() on the held child's Parent");

        const auto held_child_class_library = state.globals.find("xheldchildclasslibraryafterrelease");
        expect(held_child_class_library != state.globals.end() &&
                   held_child_class_library->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native Release external-base child should invalidate GETPEM() on the held released child's ClassLibrary");

        expect(state.ole_objects.size() == 3U,
               "native Release external-base child should leave owner, surviving sibling, and COM objects registered");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "DemoForm",
                   "native Release external-base child should keep the owner object alive");
            expect(state.ole_objects[0].source == main_path.string(),
                   "native Release external-base child should preserve the owner source path");
            expect(!state.ole_objects[0].properties.contains("cmdsave"),
                   "native Release external-base child should detach the released child from the owner");
            expect(state.ole_objects[0].properties.contains("cmdcancel"),
                   "native Release external-base child should preserve the surviving sibling on the owner");
            expect(state.ole_objects[1].prog_id == "CancelButton",
                   "native Release external-base child should preserve the surviving sibling object");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native external-base child Release lands");
        }

        const bool has_child_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "SaveButton.Destroy";
        });
        expect(has_child_destroy_event,
               "native Release external-base child should dispatch the child Destroy method");

        fs::remove_all(temp_root, ignored);
    }

    void test_inherited_external_base_release_detaches_external_base_contained_child_while_owner_and_sibling_remain_alive()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_inherited_external_release_external_base_child";
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

        const fs::path widget_library_path = temp_root / "widgetlib.prg";
        write_text(
            widget_library_path,
            "DEFINE CLASS ParentForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton')\n"
            "        THIS.AddObject('cmdCancel', 'CancelButton')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS ParentButton OF buttons.prg\n"
            "    Caption = 'Save'\n"
            "    PROCEDURE Destroy\n"
            "        cDestroyLog = cDestroyLog + 'child>'\n"
            "        cChildParentCaptionDuringDestroy = PARENT.Caption\n"
            "        nChildDestroyCount = nChildDestroyCount + 1\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS CancelButton AS Custom\n"
            "    Caption = 'Cancel'\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "inherited_external_release_external_base_child.prg";
        write_text(
            main_path,
            "cDestroyLog = ''\n"
            "cChildParentCaptionDuringDestroy = ''\n"
            "nChildDestroyCount = 0\n"
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oChild = oCreate.cmdSave\n"
            "oSibling = oCreate.cmdCancel\n"
            "cChildClassLibraryBeforeRelease = oChild.ClassLibrary\n"
            "cSiblingCaptionBeforeRelease = oSibling.Caption\n"
            "lReleased = oChild.Release()\n"
            "cDestroyLogAfter = cDestroyLog\n"
            "cChildParentCaptionAfter = cChildParentCaptionDuringDestroy\n"
            "nChildDestroyCountAfter = nChildDestroyCount\n"
            "cOwnerCaptionAfterRelease = oCreate.Caption\n"
            "lOwnerStillHasSave = PEMSTATUS(oCreate, 'cmdSave', 1)\n"
            "xReleasedChild = GETPEM(oCreate, 'cmdSave')\n"
            "lOwnerStillHasCancel = PEMSTATUS(oCreate, 'cmdCancel', 1)\n"
            "cSiblingCaptionAfterRelease = oCreate.cmdCancel.Caption\n"
            "lHeldChildHasParentAfterRelease = PEMSTATUS(oChild, 'Parent', 1)\n"
            "xHeldChildParentAfterRelease = GETPEM(oChild, 'Parent')\n"
            "lHeldChildHasClassLibraryAfterRelease = PEMSTATUS(oChild, 'ClassLibrary', 1)\n"
            "xHeldChildClassLibraryAfterRelease = GETPEM(oChild, 'ClassLibrary')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 94)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("inherited external Release external-base child script should complete: ") + state.message +
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

        check("cchildclasslibrarybeforerelease", button_library_path.string());
        check("csiblingcaptionbeforerelease", "Cancel");
        check("lreleased", "true");
        check("cdestroylogafter", "child>");
        check("cchildparentcaptionafter", "MainForm");
        check("nchilddestroycountafter", "1");
        check("cownercaptionafterrelease", "MainForm");
        check("lownerstillhassave", "false");
        check("lownerstillhascancel", "true");
        check("csiblingcaptionafterrelease", "Cancel");
        check("lheldchildhasparentafterrelease", "false");
        check("lheldchildhasclasslibraryafterrelease", "false");
        check("ldictset", "true");
        check("ndictcompare", "94");

        const auto released_child = state.globals.find("xreleasedchild");
        expect(released_child != state.globals.end() &&
                   released_child->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited external Release external-base child should invalidate GETPEM() on the owner's released child slot");

        const auto held_child_parent = state.globals.find("xheldchildparentafterrelease");
        expect(held_child_parent != state.globals.end() &&
                   held_child_parent->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited external Release external-base child should invalidate GETPEM() on the held child's Parent");

        const auto held_child_class_library = state.globals.find("xheldchildclasslibraryafterrelease");
        expect(held_child_class_library != state.globals.end() &&
                   held_child_class_library->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited external Release external-base child should invalidate GETPEM() on the held released child's ClassLibrary");

        expect(state.ole_objects.size() == 3U,
               "inherited external Release external-base child should leave owner, surviving sibling, and COM objects registered");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "ChildForm",
                   "inherited external Release external-base child should keep the owner object alive");
            expect(state.ole_objects[0].source == main_path.string(),
                   "inherited external Release external-base child should preserve the owner source path");
            expect(!state.ole_objects[0].properties.contains("cmdsave"),
                   "inherited external Release external-base child should detach the released child from the owner");
            expect(state.ole_objects[0].properties.contains("cmdcancel"),
                   "inherited external Release external-base child should preserve the surviving sibling on the owner");
            expect(state.ole_objects[1].prog_id == "CancelButton",
                   "inherited external Release external-base child should preserve the surviving sibling object");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while inherited external child Release lands");
        }

        const bool has_child_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "SaveButton.Destroy";
        });
        expect(has_child_destroy_event,
               "inherited external Release external-base child should dispatch the child Destroy method");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_release_detaches_external_base_contained_child_subtree_while_owner_and_sibling_remain_alive()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_release_external_base_child_subtree";
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

        const fs::path main_path = temp_root / "native_release_external_base_child_subtree.prg";
        write_text(
            main_path,
            "cDestroyLog = ''\n"
            "cChildParentCaptionDuringDestroy = ''\n"
            "cGrandchildParentCaptionDuringDestroy = ''\n"
            "nChildDestroyCount = 0\n"
            "nGrandchildDestroyCount = 0\n"
            "oForm = CREATEOBJECT('DemoForm')\n"
            "oChild = oForm.cmdSave\n"
            "oSibling = oForm.cmdCancel\n"
            "oGrandchild = oChild.lblBadge\n"
            "cChildClassLibraryBeforeRelease = oChild.ClassLibrary\n"
            "cGrandchildCaptionBeforeRelease = oGrandchild.Caption\n"
            "cSiblingCaptionBeforeRelease = oSibling.Caption\n"
            "lReleased = oChild.Release()\n"
            "cDestroyLogAfter = cDestroyLog\n"
            "cChildParentCaptionAfter = cChildParentCaptionDuringDestroy\n"
            "cGrandchildParentCaptionAfter = cGrandchildParentCaptionDuringDestroy\n"
            "nChildDestroyCountAfter = nChildDestroyCount\n"
            "nGrandchildDestroyCountAfter = nGrandchildDestroyCount\n"
            "cOwnerCaptionAfterRelease = oForm.Caption\n"
            "lOwnerStillHasSave = PEMSTATUS(oForm, 'cmdSave', 1)\n"
            "xReleasedChild = GETPEM(oForm, 'cmdSave')\n"
            "lOwnerStillHasCancel = PEMSTATUS(oForm, 'cmdCancel', 1)\n"
            "cSiblingCaptionAfterRelease = oForm.cmdCancel.Caption\n"
            "lHeldChildHasParentAfterRelease = PEMSTATUS(oChild, 'Parent', 1)\n"
            "xHeldChildParentAfterRelease = GETPEM(oChild, 'Parent')\n"
            "lHeldChildHasClassLibraryAfterRelease = PEMSTATUS(oChild, 'ClassLibrary', 1)\n"
            "xHeldChildClassLibraryAfterRelease = GETPEM(oChild, 'ClassLibrary')\n"
            "lHeldGrandchildHasParentAfterRelease = PEMSTATUS(oGrandchild, 'Parent', 1)\n"
            "xHeldGrandchildParentAfterRelease = GETPEM(oGrandchild, 'Parent')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 95)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton')\n"
            "        THIS.AddObject('cmdCancel', 'CancelButton')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS ParentButton OF buttons.prg\n"
            "    Caption = 'Save'\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('lblBadge', 'BadgeLabel')\n"
            "        RETURN\n"
            "    ENDPROC\n"
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
            "ENDDEFINE\n"
            "DEFINE CLASS CancelButton AS Custom\n"
            "    Caption = 'Cancel'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Release external-base child subtree script should complete: ") + state.message +
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

        check("cchildclasslibrarybeforerelease", button_library_path.string());
        check("cgrandchildcaptionbeforerelease", "Badge");
        check("csiblingcaptionbeforerelease", "Cancel");
        check("lreleased", "true");
        check("cdestroylogafter", "badge>child>");
        check("cchildparentcaptionafter", "MainForm");
        check("cgrandchildparentcaptionafter", "Save");
        check("nchilddestroycountafter", "1");
        check("ngrandchilddestroycountafter", "1");
        check("cownercaptionafterrelease", "MainForm");
        check("lownerstillhassave", "false");
        check("lownerstillhascancel", "true");
        check("csiblingcaptionafterrelease", "Cancel");
        check("lheldchildhasparentafterrelease", "false");
        check("lheldchildhasclasslibraryafterrelease", "false");
        check("lheldgrandchildhasparentafterrelease", "false");
        check("ldictset", "true");
        check("ndictcompare", "95");

        const auto released_child = state.globals.find("xreleasedchild");
        expect(released_child != state.globals.end() &&
                   released_child->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native Release external-base child subtree should invalidate GETPEM() on the owner's released child slot");

        const auto held_child_parent = state.globals.find("xheldchildparentafterrelease");
        expect(held_child_parent != state.globals.end() &&
                   held_child_parent->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native Release external-base child subtree should invalidate GETPEM() on the held child's Parent");

        const auto held_child_class_library = state.globals.find("xheldchildclasslibraryafterrelease");
        expect(held_child_class_library != state.globals.end() &&
                   held_child_class_library->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native Release external-base child subtree should invalidate GETPEM() on the held child's ClassLibrary");

        const auto held_grandchild_parent = state.globals.find("xheldgrandchildparentafterrelease");
        expect(held_grandchild_parent != state.globals.end() &&
                   held_grandchild_parent->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native Release external-base child subtree should invalidate GETPEM() on the released grandchild's Parent");

        expect(state.ole_objects.size() == 3U,
               "native Release external-base child subtree should leave owner, surviving sibling, and COM objects registered");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "DemoForm",
                   "native Release external-base child subtree should keep the owner object alive");
            expect(!state.ole_objects[0].properties.contains("cmdsave"),
                   "native Release external-base child subtree should detach the released child from the owner");
            expect(state.ole_objects[0].properties.contains("cmdcancel"),
                   "native Release external-base child subtree should preserve the surviving sibling on the owner");
            expect(state.ole_objects[1].prog_id == "CancelButton",
                   "native Release external-base child subtree should preserve the surviving sibling object");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native external-base subtree Release lands");
        }

        const bool has_child_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "SaveButton.Destroy";
        });
        expect(has_child_destroy_event,
               "native Release external-base child subtree should dispatch the child Destroy method");

        const bool has_grandchild_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "BadgeLabel.Destroy";
        });
        expect(has_grandchild_destroy_event,
               "native Release external-base child subtree should dispatch descendant Destroy methods");

        fs::remove_all(temp_root, ignored);
    }

    void test_inherited_external_base_release_detaches_external_base_contained_child_subtree_while_owner_and_sibling_remain_alive()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_inherited_external_release_external_base_child_subtree";
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

        const fs::path widget_library_path = temp_root / "widgetlib.prg";
        write_text(
            widget_library_path,
            "DEFINE CLASS ParentForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton')\n"
            "        THIS.AddObject('cmdCancel', 'CancelButton')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS ParentButton OF buttons.prg\n"
            "    Caption = 'Save'\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('lblBadge', 'BadgeLabel')\n"
            "        RETURN\n"
            "    ENDPROC\n"
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
            "ENDDEFINE\n"
            "DEFINE CLASS CancelButton AS Custom\n"
            "    Caption = 'Cancel'\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "inherited_external_release_external_base_child_subtree.prg";
        write_text(
            main_path,
            "cDestroyLog = ''\n"
            "cChildParentCaptionDuringDestroy = ''\n"
            "cGrandchildParentCaptionDuringDestroy = ''\n"
            "nChildDestroyCount = 0\n"
            "nGrandchildDestroyCount = 0\n"
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oChild = oCreate.cmdSave\n"
            "oSibling = oCreate.cmdCancel\n"
            "oGrandchild = oChild.lblBadge\n"
            "cChildClassLibraryBeforeRelease = oChild.ClassLibrary\n"
            "cGrandchildCaptionBeforeRelease = oGrandchild.Caption\n"
            "cSiblingCaptionBeforeRelease = oSibling.Caption\n"
            "lReleased = oChild.Release()\n"
            "cDestroyLogAfter = cDestroyLog\n"
            "cChildParentCaptionAfter = cChildParentCaptionDuringDestroy\n"
            "cGrandchildParentCaptionAfter = cGrandchildParentCaptionDuringDestroy\n"
            "nChildDestroyCountAfter = nChildDestroyCount\n"
            "nGrandchildDestroyCountAfter = nGrandchildDestroyCount\n"
            "cOwnerCaptionAfterRelease = oCreate.Caption\n"
            "lOwnerStillHasSave = PEMSTATUS(oCreate, 'cmdSave', 1)\n"
            "xReleasedChild = GETPEM(oCreate, 'cmdSave')\n"
            "lOwnerStillHasCancel = PEMSTATUS(oCreate, 'cmdCancel', 1)\n"
            "cSiblingCaptionAfterRelease = oCreate.cmdCancel.Caption\n"
            "lHeldChildHasParentAfterRelease = PEMSTATUS(oChild, 'Parent', 1)\n"
            "xHeldChildParentAfterRelease = GETPEM(oChild, 'Parent')\n"
            "lHeldChildHasClassLibraryAfterRelease = PEMSTATUS(oChild, 'ClassLibrary', 1)\n"
            "xHeldChildClassLibraryAfterRelease = GETPEM(oChild, 'ClassLibrary')\n"
            "lHeldGrandchildHasParentAfterRelease = PEMSTATUS(oGrandchild, 'Parent', 1)\n"
            "xHeldGrandchildParentAfterRelease = GETPEM(oGrandchild, 'Parent')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 96)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("inherited external Release external-base child subtree script should complete: ") + state.message +
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

        check("cchildclasslibrarybeforerelease", button_library_path.string());
        check("cgrandchildcaptionbeforerelease", "Badge");
        check("csiblingcaptionbeforerelease", "Cancel");
        check("lreleased", "true");
        check("cdestroylogafter", "badge>child>");
        check("cchildparentcaptionafter", "MainForm");
        check("cgrandchildparentcaptionafter", "Save");
        check("nchilddestroycountafter", "1");
        check("ngrandchilddestroycountafter", "1");
        check("cownercaptionafterrelease", "MainForm");
        check("lownerstillhassave", "false");
        check("lownerstillhascancel", "true");
        check("csiblingcaptionafterrelease", "Cancel");
        check("lheldchildhasparentafterrelease", "false");
        check("lheldchildhasclasslibraryafterrelease", "false");
        check("lheldgrandchildhasparentafterrelease", "false");
        check("ldictset", "true");
        check("ndictcompare", "96");

        const auto released_child = state.globals.find("xreleasedchild");
        expect(released_child != state.globals.end() &&
                   released_child->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited external Release external-base child subtree should invalidate GETPEM() on the owner's released child slot");

        const auto held_child_parent = state.globals.find("xheldchildparentafterrelease");
        expect(held_child_parent != state.globals.end() &&
                   held_child_parent->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited external Release external-base child subtree should invalidate GETPEM() on the held child's Parent");

        const auto held_child_class_library = state.globals.find("xheldchildclasslibraryafterrelease");
        expect(held_child_class_library != state.globals.end() &&
                   held_child_class_library->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited external Release external-base child subtree should invalidate GETPEM() on the held child's ClassLibrary");

        const auto held_grandchild_parent = state.globals.find("xheldgrandchildparentafterrelease");
        expect(held_grandchild_parent != state.globals.end() &&
                   held_grandchild_parent->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited external Release external-base child subtree should invalidate GETPEM() on the released grandchild's Parent");

        expect(state.ole_objects.size() == 3U,
               "inherited external Release external-base child subtree should leave owner, surviving sibling, and COM objects registered");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "ChildForm",
                   "inherited external Release external-base child subtree should keep the owner object alive");
            expect(!state.ole_objects[0].properties.contains("cmdsave"),
                   "inherited external Release external-base child subtree should detach the released child from the owner");
            expect(state.ole_objects[0].properties.contains("cmdcancel"),
                   "inherited external Release external-base child subtree should preserve the surviving sibling on the owner");
            expect(state.ole_objects[1].prog_id == "CancelButton",
                   "inherited external Release external-base child subtree should preserve the surviving sibling object");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while inherited external subtree Release lands");
        }

        const bool has_child_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "SaveButton.Destroy";
        });
        expect(has_child_destroy_event,
               "inherited external Release external-base child subtree should dispatch the child Destroy method");

        const bool has_grandchild_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "BadgeLabel.Destroy";
        });
        expect(has_grandchild_destroy_event,
               "inherited external Release external-base child subtree should dispatch descendant Destroy methods");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_owner_release_recursively_destroys_runtime_created_same_prg_child_subtree_and_invalidates_owner_chain()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_release_owner_runtime_created_same_prg_child_subtree";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_release_owner_runtime_created_same_prg_child_subtree.prg";
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
            "lDictSet = SETPEM(oDict, 'comparemode', 115)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "    PROCEDURE Destroy\n"
            "        cDestroyLog = cDestroyLog + 'form>'\n"
            "        nFormDestroyCount = nFormDestroyCount + 1\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ParentButton AS Custom\n"
            "    Caption = 'Save'\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS ParentButton\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('lblBadge', 'BadgeLabel')\n"
            "        RETURN\n"
            "    ENDPROC\n"
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

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native owner Release runtime-created same-PRG child subtree script should complete: ") + state.message +
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
        check("nchildclasscountbeforerelease", "4");
        check("cchildclass1beforerelease", "SAVEBUTTON");
        check("cchildclass2beforerelease", "PARENTBUTTON");
        check("cchildclass3beforerelease", "CUSTOM");
        check("cchildclass4beforerelease", "OBJECT");
        check("lhaschildbeforerelease", "true");
        check("cchildcaptionbeforerelease", "Save");
        check("cgrandchildcaptionbeforerelease", "Badge");
        check("lreleased", "true");
        check("cdestroylogafter", "badge>child>form>");
        check("cchildparentcaptionafter", "MainForm");
        check("cgrandchildparentcaptionafter", "Save");
        check("nformdestroycountafter", "1");
        check("nchilddestroycountafter", "1");
        check("ngrandchilddestroycountafter", "1");
        check("lhaschildafterrelease", "false");
        check("lchildhasparentafterrelease", "false");
        check("lchildhasclasslibraryafterrelease", "false");
        check("lgrandchildhasparentafterrelease", "false");
        check("lformhascaptionafterrelease", "false");
        check("ldictset", "true");
        check("ndictcompare", "115");

        const auto child_after_release = state.globals.find("xchildafterrelease");
        expect(child_after_release != state.globals.end() &&
                   child_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native owner Release runtime-created same-PRG child subtree should invalidate GETPEM() for the released child slot");

        const auto child_parent_after_release = state.globals.find("xchildparentafterrelease");
        expect(child_parent_after_release != state.globals.end() &&
                   child_parent_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native owner Release runtime-created same-PRG child subtree should invalidate GETPEM() for the released child's Parent");

        const auto child_class_library_after_release = state.globals.find("xchildclasslibraryafterrelease");
        expect(child_class_library_after_release != state.globals.end() &&
                   child_class_library_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native owner Release runtime-created same-PRG child subtree should invalidate GETPEM() for the released child's ClassLibrary");

        const auto grandchild_parent_after_release = state.globals.find("xgrandchildparentafterrelease");
        expect(grandchild_parent_after_release != state.globals.end() &&
                   grandchild_parent_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native owner Release runtime-created same-PRG child subtree should invalidate GETPEM() for the released grandchild's Parent");

        const auto form_caption_after_release = state.globals.find("xformcaptionafterrelease");
        expect(form_caption_after_release != state.globals.end() &&
                   form_caption_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native owner Release runtime-created same-PRG child subtree should invalidate GETPEM() for the released owner");

        expect(state.ole_objects.size() == 1U,
               "native owner Release runtime-created same-PRG child subtree should leave only the COM object registered after recursive release");
        if (state.ole_objects.size() == 1U)
        {
            expect(state.ole_objects[0].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native owner runtime-created same-PRG Release lands");
        }

        const bool has_grandchild_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "BadgeLabel.Destroy";
        });
        expect(has_grandchild_destroy_event,
               "native owner Release runtime-created same-PRG child subtree should dispatch the grandchild Destroy method");

        const bool has_child_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "SaveButton.Destroy";
        });
        expect(has_child_destroy_event,
               "native owner Release runtime-created same-PRG child subtree should dispatch the child Destroy method");

        const bool has_form_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "DemoForm.Destroy";
        });
        expect(has_form_destroy_event,
               "native owner Release runtime-created same-PRG child subtree should dispatch the owner Destroy method");

        fs::remove_all(temp_root, ignored);
    }

    void test_inherited_owner_release_recursively_destroys_runtime_created_same_prg_child_subtree_and_invalidates_owner_chain()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_inherited_release_owner_runtime_created_same_prg_child_subtree";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "inherited_release_owner_runtime_created_same_prg_child_subtree.prg";
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
            "lDictSet = SETPEM(oDict, 'comparemode', 116)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ParentForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "    PROCEDURE Destroy\n"
            "        cDestroyLog = cDestroyLog + 'form>'\n"
            "        nFormDestroyCount = nFormDestroyCount + 1\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ChildForm AS ParentForm\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ParentButton AS Custom\n"
            "    Caption = 'Save'\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS ParentButton\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('lblBadge', 'BadgeLabel')\n"
            "        RETURN\n"
            "    ENDPROC\n"
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

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("inherited owner Release runtime-created same-PRG child subtree script should complete: ") + state.message +
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
        check("nchildclasscountbeforerelease", "4");
        check("cchildclass1beforerelease", "SAVEBUTTON");
        check("cchildclass2beforerelease", "PARENTBUTTON");
        check("cchildclass3beforerelease", "CUSTOM");
        check("cchildclass4beforerelease", "OBJECT");
        check("lhaschildbeforerelease", "true");
        check("cchildcaptionbeforerelease", "Save");
        check("cgrandchildcaptionbeforerelease", "Badge");
        check("lreleased", "true");
        check("cdestroylogafter", "badge>child>form>");
        check("cchildparentcaptionafter", "MainForm");
        check("cgrandchildparentcaptionafter", "Save");
        check("nformdestroycountafter", "1");
        check("nchilddestroycountafter", "1");
        check("ngrandchilddestroycountafter", "1");
        check("lhaschildafterrelease", "false");
        check("lchildhasparentafterrelease", "false");
        check("lchildhasclasslibraryafterrelease", "false");
        check("lgrandchildhasparentafterrelease", "false");
        check("lformhascaptionafterrelease", "false");
        check("ldictset", "true");
        check("ndictcompare", "116");

        const auto child_after_release = state.globals.find("xchildafterrelease");
        expect(child_after_release != state.globals.end() &&
                   child_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited owner Release runtime-created same-PRG child subtree should invalidate GETPEM() for the released child slot");

        const auto child_parent_after_release = state.globals.find("xchildparentafterrelease");
        expect(child_parent_after_release != state.globals.end() &&
                   child_parent_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited owner Release runtime-created same-PRG child subtree should invalidate GETPEM() for the released child's Parent");

        const auto child_class_library_after_release = state.globals.find("xchildclasslibraryafterrelease");
        expect(child_class_library_after_release != state.globals.end() &&
                   child_class_library_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited owner Release runtime-created same-PRG child subtree should invalidate GETPEM() for the released child's ClassLibrary");

        const auto grandchild_parent_after_release = state.globals.find("xgrandchildparentafterrelease");
        expect(grandchild_parent_after_release != state.globals.end() &&
                   grandchild_parent_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited owner Release runtime-created same-PRG child subtree should invalidate GETPEM() for the released grandchild's Parent");

        const auto form_caption_after_release = state.globals.find("xformcaptionafterrelease");
        expect(form_caption_after_release != state.globals.end() &&
                   form_caption_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited owner Release runtime-created same-PRG child subtree should invalidate GETPEM() for the released owner");

        expect(state.ole_objects.size() == 1U,
               "inherited owner Release runtime-created same-PRG child subtree should leave only the COM object registered after recursive release");
        if (state.ole_objects.size() == 1U)
        {
            expect(state.ole_objects[0].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while inherited owner runtime-created same-PRG Release lands");
        }

        const bool has_grandchild_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "BadgeLabel.Destroy";
        });
        expect(has_grandchild_destroy_event,
               "inherited owner Release runtime-created same-PRG child subtree should dispatch the grandchild Destroy method");

        const bool has_child_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "SaveButton.Destroy";
        });
        expect(has_child_destroy_event,
               "inherited owner Release runtime-created same-PRG child subtree should dispatch the child Destroy method");

        const bool has_form_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "ParentForm.Destroy";
        });
        expect(has_form_destroy_event,
               "inherited owner Release runtime-created same-PRG child subtree should dispatch the owner Destroy method");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_owner_release_recursively_destroys_runtime_created_external_child_subtree_and_invalidates_owner_chain()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_release_owner_runtime_created_external_child_subtree";
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

        const fs::path main_path = temp_root / "native_release_owner_runtime_created_external_child_subtree.prg";
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
            "lDictSet = SETPEM(oDict, 'comparemode', 113)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "    PROCEDURE Destroy\n"
            "        cDestroyLog = cDestroyLog + 'form>'\n"
            "        nFormDestroyCount = nFormDestroyCount + 1\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS ParentButton OF buttons.prg\n"
            "    Caption = 'Save'\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('lblBadge', 'BadgeLabel')\n"
            "        RETURN\n"
            "    ENDPROC\n"
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

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native owner Release runtime-created external child subtree script should complete: ") + state.message +
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
        check("cchildcaptionbeforerelease", "Save");
        check("cgrandchildcaptionbeforerelease", "Badge");
        check("lreleased", "true");
        check("cdestroylogafter", "badge>child>form>");
        check("cchildparentcaptionafter", "MainForm");
        check("cgrandchildparentcaptionafter", "Save");
        check("nformdestroycountafter", "1");
        check("nchilddestroycountafter", "1");
        check("ngrandchilddestroycountafter", "1");
        check("lhaschildafterrelease", "false");
        check("lchildhasparentafterrelease", "false");
        check("lchildhasclasslibraryafterrelease", "false");
        check("lgrandchildhasparentafterrelease", "false");
        check("lformhascaptionafterrelease", "false");
        check("ldictset", "true");
        check("ndictcompare", "113");

        const auto child_after_release = state.globals.find("xchildafterrelease");
        expect(child_after_release != state.globals.end() &&
                   child_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native owner Release runtime-created external child subtree should invalidate GETPEM() for the released child slot");

        const auto child_parent_after_release = state.globals.find("xchildparentafterrelease");
        expect(child_parent_after_release != state.globals.end() &&
                   child_parent_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native owner Release runtime-created external child subtree should invalidate GETPEM() for the released child's Parent");

        const auto child_class_library_after_release = state.globals.find("xchildclasslibraryafterrelease");
        expect(child_class_library_after_release != state.globals.end() &&
                   child_class_library_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native owner Release runtime-created external child subtree should invalidate GETPEM() for the released child's ClassLibrary");

        const auto grandchild_parent_after_release = state.globals.find("xgrandchildparentafterrelease");
        expect(grandchild_parent_after_release != state.globals.end() &&
                   grandchild_parent_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native owner Release runtime-created external child subtree should invalidate GETPEM() for the released grandchild's Parent");

        const auto form_caption_after_release = state.globals.find("xformcaptionafterrelease");
        expect(form_caption_after_release != state.globals.end() &&
                   form_caption_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native owner Release runtime-created external child subtree should invalidate GETPEM() for the released owner");

        expect(state.ole_objects.size() == 1U,
               "native owner Release runtime-created external child subtree should leave only the COM object registered after recursive release");
        if (state.ole_objects.size() == 1U)
        {
            expect(state.ole_objects[0].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native owner runtime-created external Release lands");
        }

        const bool has_grandchild_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "BadgeLabel.Destroy";
        });
        expect(has_grandchild_destroy_event,
               "native owner Release runtime-created external child subtree should dispatch the grandchild Destroy method");

        const bool has_child_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "SaveButton.Destroy";
        });
        expect(has_child_destroy_event,
               "native owner Release runtime-created external child subtree should dispatch the child Destroy method");

        const bool has_form_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "DemoForm.Destroy";
        });
        expect(has_form_destroy_event,
               "native owner Release runtime-created external child subtree should dispatch the owner Destroy method");

        fs::remove_all(temp_root, ignored);
    }

    void test_inherited_owner_release_recursively_destroys_runtime_created_external_child_subtree_and_invalidates_owner_chain()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_inherited_release_owner_runtime_created_external_child_subtree";
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

        const fs::path widget_library_path = temp_root / "widgetlib.prg";
        write_text(
            widget_library_path,
            "DEFINE CLASS ParentForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "    PROCEDURE Destroy\n"
            "        cDestroyLog = cDestroyLog + 'form>'\n"
            "        nFormDestroyCount = nFormDestroyCount + 1\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS ParentButton OF buttons.prg\n"
            "    Caption = 'Save'\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('lblBadge', 'BadgeLabel')\n"
            "        RETURN\n"
            "    ENDPROC\n"
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

        const fs::path main_path = temp_root / "inherited_release_owner_runtime_created_external_child_subtree.prg";
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
            "lDictSet = SETPEM(oDict, 'comparemode', 114)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("inherited owner Release runtime-created external child subtree script should complete: ") + state.message +
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
        check("cchildcaptionbeforerelease", "Save");
        check("cgrandchildcaptionbeforerelease", "Badge");
        check("lreleased", "true");
        check("cdestroylogafter", "badge>child>form>");
        check("cchildparentcaptionafter", "MainForm");
        check("cgrandchildparentcaptionafter", "Save");
        check("nformdestroycountafter", "1");
        check("nchilddestroycountafter", "1");
        check("ngrandchilddestroycountafter", "1");
        check("lhaschildafterrelease", "false");
        check("lchildhasparentafterrelease", "false");
        check("lchildhasclasslibraryafterrelease", "false");
        check("lgrandchildhasparentafterrelease", "false");
        check("lformhascaptionafterrelease", "false");
        check("ldictset", "true");
        check("ndictcompare", "114");

        const auto child_after_release = state.globals.find("xchildafterrelease");
        expect(child_after_release != state.globals.end() &&
                   child_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited owner Release runtime-created external child subtree should invalidate GETPEM() for the released child slot");

        const auto child_parent_after_release = state.globals.find("xchildparentafterrelease");
        expect(child_parent_after_release != state.globals.end() &&
                   child_parent_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited owner Release runtime-created external child subtree should invalidate GETPEM() for the released child's Parent");

        const auto child_class_library_after_release = state.globals.find("xchildclasslibraryafterrelease");
        expect(child_class_library_after_release != state.globals.end() &&
                   child_class_library_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited owner Release runtime-created external child subtree should invalidate GETPEM() for the released child's ClassLibrary");

        const auto grandchild_parent_after_release = state.globals.find("xgrandchildparentafterrelease");
        expect(grandchild_parent_after_release != state.globals.end() &&
                   grandchild_parent_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited owner Release runtime-created external child subtree should invalidate GETPEM() for the released grandchild's Parent");

        const auto form_caption_after_release = state.globals.find("xformcaptionafterrelease");
        expect(form_caption_after_release != state.globals.end() &&
                   form_caption_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited owner Release runtime-created external child subtree should invalidate GETPEM() for the released owner");

        expect(state.ole_objects.size() == 1U,
               "inherited owner Release runtime-created external child subtree should leave only the COM object registered after recursive release");
        if (state.ole_objects.size() == 1U)
        {
            expect(state.ole_objects[0].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while inherited owner runtime-created external Release lands");
        }

        const bool has_grandchild_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "BadgeLabel.Destroy";
        });
        expect(has_grandchild_destroy_event,
               "inherited owner Release runtime-created external child subtree should dispatch the grandchild Destroy method");

        const bool has_child_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "SaveButton.Destroy";
        });
        expect(has_child_destroy_event,
               "inherited owner Release runtime-created external child subtree should dispatch the child Destroy method");

        const bool has_form_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "ParentForm.Destroy";
        });
        expect(has_form_destroy_event,
               "inherited owner Release runtime-created external child subtree should dispatch the owner Destroy method");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_release_detaches_declarative_external_base_child_subtree_while_owner_and_sibling_remain_alive()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_release_declarative_external_base_child_subtree";
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
            "    ADD OBJECT lblBadge AS BadgeLabel\n"
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

        const fs::path main_path = temp_root / "native_release_declarative_external_base_child_subtree.prg";
        write_text(
            main_path,
            "cDestroyLog = ''\n"
            "cChildParentCaptionDuringDestroy = ''\n"
            "cGrandchildParentCaptionDuringDestroy = ''\n"
            "nChildDestroyCount = 0\n"
            "nGrandchildDestroyCount = 0\n"
            "oForm = CREATEOBJECT('DemoForm')\n"
            "oChild = oForm.cmdSave\n"
            "oSibling = oForm.cmdCancel\n"
            "oGrandchild = oChild.lblBadge\n"
            "cChildClassLibraryBeforeRelease = oChild.ClassLibrary\n"
            "cGrandchildCaptionBeforeRelease = oGrandchild.Caption\n"
            "cSiblingCaptionBeforeRelease = oSibling.Caption\n"
            "lReleased = oChild.Release()\n"
            "cDestroyLogAfter = cDestroyLog\n"
            "cChildParentCaptionAfter = cChildParentCaptionDuringDestroy\n"
            "cGrandchildParentCaptionAfter = cGrandchildParentCaptionDuringDestroy\n"
            "nChildDestroyCountAfter = nChildDestroyCount\n"
            "nGrandchildDestroyCountAfter = nGrandchildDestroyCount\n"
            "cOwnerCaptionAfterRelease = oForm.Caption\n"
            "lOwnerStillHasSave = PEMSTATUS(oForm, 'cmdSave', 1)\n"
            "xReleasedChild = GETPEM(oForm, 'cmdSave')\n"
            "lOwnerStillHasCancel = PEMSTATUS(oForm, 'cmdCancel', 1)\n"
            "cSiblingCaptionAfterRelease = oForm.cmdCancel.Caption\n"
            "lHeldChildHasParentAfterRelease = PEMSTATUS(oChild, 'Parent', 1)\n"
            "xHeldChildParentAfterRelease = GETPEM(oChild, 'Parent')\n"
            "lHeldChildHasClassLibraryAfterRelease = PEMSTATUS(oChild, 'ClassLibrary', 1)\n"
            "xHeldChildClassLibraryAfterRelease = GETPEM(oChild, 'ClassLibrary')\n"
            "lHeldGrandchildHasParentAfterRelease = PEMSTATUS(oGrandchild, 'Parent', 1)\n"
            "xHeldGrandchildParentAfterRelease = GETPEM(oGrandchild, 'Parent')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 97)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    ADD OBJECT cmdSave AS SaveButton OF buttons.prg\n"
            "    ADD OBJECT cmdCancel AS CancelButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS CancelButton AS Custom\n"
            "    Caption = 'Cancel'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Release declarative external-base child subtree script should complete: ") + state.message +
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

        check("cchildclasslibrarybeforerelease", "");
        check("cgrandchildcaptionbeforerelease", "Badge");
        check("csiblingcaptionbeforerelease", "Cancel");
        check("lreleased", "true");
        check("cdestroylogafter", "badge>child>");
        check("cchildparentcaptionafter", "MainForm");
        check("cgrandchildparentcaptionafter", "Save");
        check("nchilddestroycountafter", "1");
        check("ngrandchilddestroycountafter", "1");
        check("cownercaptionafterrelease", "MainForm");
        check("lownerstillhassave", "false");
        check("lownerstillhascancel", "true");
        check("csiblingcaptionafterrelease", "Cancel");
        check("lheldchildhasparentafterrelease", "false");
        check("lheldchildhasclasslibraryafterrelease", "false");
        check("lheldgrandchildhasparentafterrelease", "false");
        check("ldictset", "true");
        check("ndictcompare", "97");

        const auto released_child = state.globals.find("xreleasedchild");
        expect(released_child != state.globals.end() &&
                   released_child->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native Release declarative external-base child subtree should invalidate GETPEM() on the owner's released child slot");

        const auto held_child_parent = state.globals.find("xheldchildparentafterrelease");
        expect(held_child_parent != state.globals.end() &&
                   held_child_parent->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native Release declarative external-base child subtree should invalidate GETPEM() on the held child's Parent");

        const auto held_child_class_library = state.globals.find("xheldchildclasslibraryafterrelease");
        expect(held_child_class_library != state.globals.end() &&
                   held_child_class_library->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native Release declarative external-base child subtree should invalidate GETPEM() on the held child's ClassLibrary");

        const auto held_grandchild_parent = state.globals.find("xheldgrandchildparentafterrelease");
        expect(held_grandchild_parent != state.globals.end() &&
                   held_grandchild_parent->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native Release declarative external-base child subtree should invalidate GETPEM() on the released grandchild's Parent");

        expect(state.ole_objects.size() == 3U,
               "native Release declarative external-base child subtree should leave owner, surviving sibling, and COM objects registered");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "DemoForm",
                   "native Release declarative external-base child subtree should keep the owner object alive");
            expect(!state.ole_objects[0].properties.contains("cmdsave"),
                   "native Release declarative external-base child subtree should detach the released child from the owner");
            expect(state.ole_objects[0].properties.contains("cmdcancel"),
                   "native Release declarative external-base child subtree should preserve the surviving sibling on the owner");
            expect(state.ole_objects[1].prog_id == "CancelButton",
                   "native Release declarative external-base child subtree should preserve the surviving sibling object");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native declarative external-base subtree Release lands");
        }

        const bool has_child_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "SaveButton.Destroy";
        });
        expect(has_child_destroy_event,
               "native Release declarative external-base child subtree should dispatch the child Destroy method");

        const bool has_grandchild_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "BadgeLabel.Destroy";
        });
        expect(has_grandchild_destroy_event,
               "native Release declarative external-base child subtree should dispatch descendant Destroy methods");

        fs::remove_all(temp_root, ignored);
    }

    void test_inherited_declarative_release_detaches_external_base_child_subtree_while_owner_and_sibling_remain_alive()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_inherited_declarative_release_external_base_child_subtree";
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
            "    ADD OBJECT lblBadge AS BadgeLabel\n"
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
            "    ADD OBJECT cmdSave AS SaveButton OF buttons.prg\n"
            "    ADD OBJECT cmdCancel AS CancelButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS CancelButton AS Custom\n"
            "    Caption = 'Cancel'\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "inherited_declarative_release_external_base_child_subtree.prg";
        write_text(
            main_path,
            "cDestroyLog = ''\n"
            "cChildParentCaptionDuringDestroy = ''\n"
            "cGrandchildParentCaptionDuringDestroy = ''\n"
            "nChildDestroyCount = 0\n"
            "nGrandchildDestroyCount = 0\n"
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oChild = oCreate.cmdSave\n"
            "oSibling = oCreate.cmdCancel\n"
            "oGrandchild = oChild.lblBadge\n"
            "cChildClassLibraryBeforeRelease = oChild.ClassLibrary\n"
            "cGrandchildCaptionBeforeRelease = oGrandchild.Caption\n"
            "cSiblingCaptionBeforeRelease = oSibling.Caption\n"
            "lReleased = oChild.Release()\n"
            "cDestroyLogAfter = cDestroyLog\n"
            "cChildParentCaptionAfter = cChildParentCaptionDuringDestroy\n"
            "cGrandchildParentCaptionAfter = cGrandchildParentCaptionDuringDestroy\n"
            "nChildDestroyCountAfter = nChildDestroyCount\n"
            "nGrandchildDestroyCountAfter = nGrandchildDestroyCount\n"
            "cOwnerCaptionAfterRelease = oCreate.Caption\n"
            "lOwnerStillHasSave = PEMSTATUS(oCreate, 'cmdSave', 1)\n"
            "xReleasedChild = GETPEM(oCreate, 'cmdSave')\n"
            "lOwnerStillHasCancel = PEMSTATUS(oCreate, 'cmdCancel', 1)\n"
            "cSiblingCaptionAfterRelease = oCreate.cmdCancel.Caption\n"
            "lHeldChildHasParentAfterRelease = PEMSTATUS(oChild, 'Parent', 1)\n"
            "xHeldChildParentAfterRelease = GETPEM(oChild, 'Parent')\n"
            "lHeldChildHasClassLibraryAfterRelease = PEMSTATUS(oChild, 'ClassLibrary', 1)\n"
            "xHeldChildClassLibraryAfterRelease = GETPEM(oChild, 'ClassLibrary')\n"
            "lHeldGrandchildHasParentAfterRelease = PEMSTATUS(oGrandchild, 'Parent', 1)\n"
            "xHeldGrandchildParentAfterRelease = GETPEM(oGrandchild, 'Parent')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 98)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("inherited declarative Release external-base child subtree script should complete: ") + state.message +
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

        check("cchildclasslibrarybeforerelease", "");
        check("cgrandchildcaptionbeforerelease", "Badge");
        check("csiblingcaptionbeforerelease", "Cancel");
        check("lreleased", "true");
        check("cdestroylogafter", "badge>child>");
        check("cchildparentcaptionafter", "MainForm");
        check("cgrandchildparentcaptionafter", "Save");
        check("nchilddestroycountafter", "1");
        check("ngrandchilddestroycountafter", "1");
        check("cownercaptionafterrelease", "MainForm");
        check("lownerstillhassave", "false");
        check("lownerstillhascancel", "true");
        check("csiblingcaptionafterrelease", "Cancel");
        check("lheldchildhasparentafterrelease", "false");
        check("lheldchildhasclasslibraryafterrelease", "false");
        check("lheldgrandchildhasparentafterrelease", "false");
        check("ldictset", "true");
        check("ndictcompare", "98");

        const auto released_child = state.globals.find("xreleasedchild");
        expect(released_child != state.globals.end() &&
                   released_child->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited declarative Release external-base child subtree should invalidate GETPEM() on the owner's released child slot");

        const auto held_child_parent = state.globals.find("xheldchildparentafterrelease");
        expect(held_child_parent != state.globals.end() &&
                   held_child_parent->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited declarative Release external-base child subtree should invalidate GETPEM() on the held child's Parent");

        const auto held_child_class_library = state.globals.find("xheldchildclasslibraryafterrelease");
        expect(held_child_class_library != state.globals.end() &&
                   held_child_class_library->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited declarative Release external-base child subtree should invalidate GETPEM() on the held child's ClassLibrary");

        const auto held_grandchild_parent = state.globals.find("xheldgrandchildparentafterrelease");
        expect(held_grandchild_parent != state.globals.end() &&
                   held_grandchild_parent->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited declarative Release external-base child subtree should invalidate GETPEM() on the released grandchild's Parent");

        expect(state.ole_objects.size() == 3U,
               "inherited declarative Release external-base child subtree should leave owner, surviving sibling, and COM objects registered");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "ChildForm",
                   "inherited declarative Release external-base child subtree should keep the owner object alive");
            expect(!state.ole_objects[0].properties.contains("cmdsave"),
                   "inherited declarative Release external-base child subtree should detach the released child from the owner");
            expect(state.ole_objects[0].properties.contains("cmdcancel"),
                   "inherited declarative Release external-base child subtree should preserve the surviving sibling on the owner");
            expect(state.ole_objects[1].prog_id == "CancelButton",
                   "inherited declarative Release external-base child subtree should preserve the surviving sibling object");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while inherited declarative subtree Release lands");
        }

        const bool has_child_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "SaveButton.Destroy";
        });
        expect(has_child_destroy_event,
               "inherited declarative Release external-base child subtree should dispatch the child Destroy method");

        const bool has_grandchild_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "BadgeLabel.Destroy";
        });
        expect(has_grandchild_destroy_event,
               "inherited declarative Release external-base child subtree should dispatch descendant Destroy methods");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_release_detaches_declarative_same_prg_child_subtree_while_owner_and_sibling_remain_alive()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_release_declarative_same_prg_child_subtree";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_release_declarative_same_prg_child_subtree.prg";
        write_text(
            main_path,
            "cDestroyLog = ''\n"
            "cChildParentCaptionDuringDestroy = ''\n"
            "cGrandchildParentCaptionDuringDestroy = ''\n"
            "nChildDestroyCount = 0\n"
            "nGrandchildDestroyCount = 0\n"
            "oForm = CREATEOBJECT('DemoForm')\n"
            "oChild = oForm.cmdSave\n"
            "oSibling = oForm.cmdCancel\n"
            "oGrandchild = oChild.lblBadge\n"
            "cChildClassLibraryBeforeRelease = oChild.ClassLibrary\n"
            "cGrandchildCaptionBeforeRelease = oGrandchild.Caption\n"
            "cSiblingCaptionBeforeRelease = oSibling.Caption\n"
            "lReleased = oChild.Release()\n"
            "cDestroyLogAfter = cDestroyLog\n"
            "cChildParentCaptionAfter = cChildParentCaptionDuringDestroy\n"
            "cGrandchildParentCaptionAfter = cGrandchildParentCaptionDuringDestroy\n"
            "nChildDestroyCountAfter = nChildDestroyCount\n"
            "nGrandchildDestroyCountAfter = nGrandchildDestroyCount\n"
            "cOwnerCaptionAfterRelease = oForm.Caption\n"
            "lOwnerStillHasSave = PEMSTATUS(oForm, 'cmdSave', 1)\n"
            "xReleasedChild = GETPEM(oForm, 'cmdSave')\n"
            "lOwnerStillHasCancel = PEMSTATUS(oForm, 'cmdCancel', 1)\n"
            "cSiblingCaptionAfterRelease = oForm.cmdCancel.Caption\n"
            "lHeldChildHasParentAfterRelease = PEMSTATUS(oChild, 'Parent', 1)\n"
            "xHeldChildParentAfterRelease = GETPEM(oChild, 'Parent')\n"
            "lHeldChildHasClassLibraryAfterRelease = PEMSTATUS(oChild, 'ClassLibrary', 1)\n"
            "xHeldChildClassLibraryAfterRelease = GETPEM(oChild, 'ClassLibrary')\n"
            "lHeldGrandchildHasParentAfterRelease = PEMSTATUS(oGrandchild, 'Parent', 1)\n"
            "xHeldGrandchildParentAfterRelease = GETPEM(oGrandchild, 'Parent')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 99)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "    ADD OBJECT cmdCancel AS CancelButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
            "    Caption = 'Save'\n"
            "    ADD OBJECT lblBadge AS BadgeLabel\n"
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
            "ENDDEFINE\n"
            "DEFINE CLASS CancelButton AS Custom\n"
            "    Caption = 'Cancel'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Release declarative same-PRG child subtree script should complete: ") + state.message +
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

        check("cchildclasslibrarybeforerelease", "");
        check("cgrandchildcaptionbeforerelease", "Badge");
        check("csiblingcaptionbeforerelease", "Cancel");
        check("lreleased", "true");
        check("cdestroylogafter", "badge>child>");
        check("cchildparentcaptionafter", "MainForm");
        check("cgrandchildparentcaptionafter", "Save");
        check("nchilddestroycountafter", "1");
        check("ngrandchilddestroycountafter", "1");
        check("cownercaptionafterrelease", "MainForm");
        check("lownerstillhassave", "false");
        check("lownerstillhascancel", "true");
        check("csiblingcaptionafterrelease", "Cancel");
        check("lheldchildhasparentafterrelease", "false");
        check("lheldchildhasclasslibraryafterrelease", "false");
        check("lheldgrandchildhasparentafterrelease", "false");
        check("ldictset", "true");
        check("ndictcompare", "99");

        const auto released_child = state.globals.find("xreleasedchild");
        expect(released_child != state.globals.end() &&
                   released_child->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native Release declarative same-PRG child subtree should invalidate GETPEM() on the owner's released child slot");

        const auto held_child_parent = state.globals.find("xheldchildparentafterrelease");
        expect(held_child_parent != state.globals.end() &&
                   held_child_parent->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native Release declarative same-PRG child subtree should invalidate GETPEM() on the held child's Parent");

        const auto held_child_class_library = state.globals.find("xheldchildclasslibraryafterrelease");
        expect(held_child_class_library != state.globals.end() &&
                   held_child_class_library->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native Release declarative same-PRG child subtree should invalidate GETPEM() on the held child's ClassLibrary");

        const auto held_grandchild_parent = state.globals.find("xheldgrandchildparentafterrelease");
        expect(held_grandchild_parent != state.globals.end() &&
                   held_grandchild_parent->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native Release declarative same-PRG child subtree should invalidate GETPEM() on the released grandchild's Parent");

        expect(state.ole_objects.size() == 3U,
               "native Release declarative same-PRG child subtree should leave owner, surviving sibling, and COM objects registered");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "DemoForm",
                   "native Release declarative same-PRG child subtree should keep the owner object alive");
            expect(!state.ole_objects[0].properties.contains("cmdsave"),
                   "native Release declarative same-PRG child subtree should detach the released child from the owner");
            expect(state.ole_objects[0].properties.contains("cmdcancel"),
                   "native Release declarative same-PRG child subtree should preserve the surviving sibling on the owner");
            expect(state.ole_objects[1].prog_id == "CancelButton",
                   "native Release declarative same-PRG child subtree should preserve the surviving sibling object");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native declarative same-PRG subtree Release lands");
        }

        const bool has_child_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "SaveButton.Destroy";
        });
        expect(has_child_destroy_event,
               "native Release declarative same-PRG child subtree should dispatch the child Destroy method");

        const bool has_grandchild_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "BadgeLabel.Destroy";
        });
        expect(has_grandchild_destroy_event,
               "native Release declarative same-PRG child subtree should dispatch descendant Destroy methods");

        fs::remove_all(temp_root, ignored);
    }

    void test_inherited_same_prg_release_detaches_declarative_child_subtree_while_owner_and_sibling_remain_alive()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_inherited_same_prg_release_declarative_child_subtree";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "inherited_same_prg_release_declarative_child_subtree.prg";
        write_text(
            main_path,
            "cDestroyLog = ''\n"
            "cChildParentCaptionDuringDestroy = ''\n"
            "cGrandchildParentCaptionDuringDestroy = ''\n"
            "nChildDestroyCount = 0\n"
            "nGrandchildDestroyCount = 0\n"
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oChild = oCreate.cmdSave\n"
            "oSibling = oCreate.cmdCancel\n"
            "oGrandchild = oChild.lblBadge\n"
            "cChildClassLibraryBeforeRelease = oChild.ClassLibrary\n"
            "cGrandchildCaptionBeforeRelease = oGrandchild.Caption\n"
            "cSiblingCaptionBeforeRelease = oSibling.Caption\n"
            "lReleased = oChild.Release()\n"
            "cDestroyLogAfter = cDestroyLog\n"
            "cChildParentCaptionAfter = cChildParentCaptionDuringDestroy\n"
            "cGrandchildParentCaptionAfter = cGrandchildParentCaptionDuringDestroy\n"
            "nChildDestroyCountAfter = nChildDestroyCount\n"
            "nGrandchildDestroyCountAfter = nGrandchildDestroyCount\n"
            "cOwnerCaptionAfterRelease = oCreate.Caption\n"
            "lOwnerStillHasSave = PEMSTATUS(oCreate, 'cmdSave', 1)\n"
            "xReleasedChild = GETPEM(oCreate, 'cmdSave')\n"
            "lOwnerStillHasCancel = PEMSTATUS(oCreate, 'cmdCancel', 1)\n"
            "cSiblingCaptionAfterRelease = oCreate.cmdCancel.Caption\n"
            "lHeldChildHasParentAfterRelease = PEMSTATUS(oChild, 'Parent', 1)\n"
            "xHeldChildParentAfterRelease = GETPEM(oChild, 'Parent')\n"
            "lHeldChildHasClassLibraryAfterRelease = PEMSTATUS(oChild, 'ClassLibrary', 1)\n"
            "xHeldChildClassLibraryAfterRelease = GETPEM(oChild, 'ClassLibrary')\n"
            "lHeldGrandchildHasParentAfterRelease = PEMSTATUS(oGrandchild, 'Parent', 1)\n"
            "xHeldGrandchildParentAfterRelease = GETPEM(oGrandchild, 'Parent')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 100)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ParentForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "    ADD OBJECT cmdCancel AS CancelButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ChildForm AS ParentForm\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
            "    Caption = 'Save'\n"
            "    ADD OBJECT lblBadge AS BadgeLabel\n"
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
            "ENDDEFINE\n"
            "DEFINE CLASS CancelButton AS Custom\n"
            "    Caption = 'Cancel'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("inherited same-PRG Release declarative child subtree script should complete: ") + state.message +
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

        check("cchildclasslibrarybeforerelease", "");
        check("cgrandchildcaptionbeforerelease", "Badge");
        check("csiblingcaptionbeforerelease", "Cancel");
        check("lreleased", "true");
        check("cdestroylogafter", "badge>child>");
        check("cchildparentcaptionafter", "MainForm");
        check("cgrandchildparentcaptionafter", "Save");
        check("nchilddestroycountafter", "1");
        check("ngrandchilddestroycountafter", "1");
        check("cownercaptionafterrelease", "MainForm");
        check("lownerstillhassave", "false");
        check("lownerstillhascancel", "true");
        check("csiblingcaptionafterrelease", "Cancel");
        check("lheldchildhasparentafterrelease", "false");
        check("lheldchildhasclasslibraryafterrelease", "false");
        check("lheldgrandchildhasparentafterrelease", "false");
        check("ldictset", "true");
        check("ndictcompare", "100");

        const auto released_child = state.globals.find("xreleasedchild");
        expect(released_child != state.globals.end() &&
                   released_child->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited same-PRG Release declarative child subtree should invalidate GETPEM() on the owner's released child slot");

        const auto held_child_parent = state.globals.find("xheldchildparentafterrelease");
        expect(held_child_parent != state.globals.end() &&
                   held_child_parent->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited same-PRG Release declarative child subtree should invalidate GETPEM() on the held child's Parent");

        const auto held_child_class_library = state.globals.find("xheldchildclasslibraryafterrelease");
        expect(held_child_class_library != state.globals.end() &&
                   held_child_class_library->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited same-PRG Release declarative child subtree should invalidate GETPEM() on the held child's ClassLibrary");

        const auto held_grandchild_parent = state.globals.find("xheldgrandchildparentafterrelease");
        expect(held_grandchild_parent != state.globals.end() &&
                   held_grandchild_parent->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited same-PRG Release declarative child subtree should invalidate GETPEM() on the released grandchild's Parent");

        expect(state.ole_objects.size() == 3U,
               "inherited same-PRG Release declarative child subtree should leave owner, surviving sibling, and COM objects registered");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "ChildForm",
                   "inherited same-PRG Release declarative child subtree should keep the owner object alive");
            expect(!state.ole_objects[0].properties.contains("cmdsave"),
                   "inherited same-PRG Release declarative child subtree should detach the released child from the owner");
            expect(state.ole_objects[0].properties.contains("cmdcancel"),
                   "inherited same-PRG Release declarative child subtree should preserve the surviving sibling on the owner");
            expect(state.ole_objects[1].prog_id == "CancelButton",
                   "inherited same-PRG Release declarative child subtree should preserve the surviving sibling object");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while inherited same-PRG declarative subtree Release lands");
        }

        const bool has_child_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "SaveButton.Destroy";
        });
        expect(has_child_destroy_event,
               "inherited same-PRG Release declarative child subtree should dispatch the child Destroy method");

        const bool has_grandchild_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "BadgeLabel.Destroy";
        });
        expect(has_grandchild_destroy_event,
               "inherited same-PRG Release declarative child subtree should dispatch descendant Destroy methods");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_owner_release_recursively_destroys_declarative_same_prg_child_subtree_and_invalidates_owner_chain()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_release_owner_declarative_same_prg_child_subtree";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_release_owner_declarative_same_prg_child_subtree.prg";
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
            "lDictSet = SETPEM(oDict, 'comparemode', 109)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "    PROCEDURE Destroy\n"
            "        cDestroyLog = cDestroyLog + 'form>'\n"
            "        nFormDestroyCount = nFormDestroyCount + 1\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ParentButton AS Custom\n"
            "    Caption = 'Save'\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS ParentButton\n"
            "    ADD OBJECT lblBadge AS BadgeLabel\n"
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

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native owner Release declarative same-PRG child subtree script should complete: ") + state.message +
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
        check("nchildclasscountbeforerelease", "4");
        check("cchildclass1beforerelease", "SAVEBUTTON");
        check("cchildclass2beforerelease", "PARENTBUTTON");
        check("cchildclass3beforerelease", "CUSTOM");
        check("cchildclass4beforerelease", "OBJECT");
        check("lhaschildbeforerelease", "true");
        check("cchildcaptionbeforerelease", "Save");
        check("cgrandchildcaptionbeforerelease", "Badge");
        check("lreleased", "true");
        check("cdestroylogafter", "badge>child>form>");
        check("cchildparentcaptionafter", "MainForm");
        check("cgrandchildparentcaptionafter", "Save");
        check("nformdestroycountafter", "1");
        check("nchilddestroycountafter", "1");
        check("ngrandchilddestroycountafter", "1");
        check("lhaschildafterrelease", "false");
        check("lchildhasparentafterrelease", "false");
        check("lchildhasclasslibraryafterrelease", "false");
        check("lgrandchildhasparentafterrelease", "false");
        check("lformhascaptionafterrelease", "false");
        check("ldictset", "true");
        check("ndictcompare", "109");

        const auto child_after_release = state.globals.find("xchildafterrelease");
        expect(child_after_release != state.globals.end() &&
                   child_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native owner Release declarative same-PRG child subtree should invalidate GETPEM() for the released child slot");

        const auto child_parent_after_release = state.globals.find("xchildparentafterrelease");
        expect(child_parent_after_release != state.globals.end() &&
                   child_parent_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native owner Release declarative same-PRG child subtree should invalidate GETPEM() for the released child's Parent");

        const auto child_class_library_after_release = state.globals.find("xchildclasslibraryafterrelease");
        expect(child_class_library_after_release != state.globals.end() &&
                   child_class_library_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native owner Release declarative same-PRG child subtree should invalidate GETPEM() for the released child's ClassLibrary");

        const auto grandchild_parent_after_release = state.globals.find("xgrandchildparentafterrelease");
        expect(grandchild_parent_after_release != state.globals.end() &&
                   grandchild_parent_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native owner Release declarative same-PRG child subtree should invalidate GETPEM() for the released grandchild's Parent");

        const auto form_caption_after_release = state.globals.find("xformcaptionafterrelease");
        expect(form_caption_after_release != state.globals.end() &&
                   form_caption_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native owner Release declarative same-PRG child subtree should invalidate GETPEM() for the released owner");

        expect(state.ole_objects.size() == 1U,
               "native owner Release declarative same-PRG child subtree should leave only the COM object registered after recursive release");
        if (state.ole_objects.size() == 1U)
        {
            expect(state.ole_objects[0].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native owner declarative same-PRG Release lands");
        }

        const bool has_grandchild_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "BadgeLabel.Destroy";
        });
        expect(has_grandchild_destroy_event,
               "native owner Release declarative same-PRG child subtree should dispatch the grandchild Destroy method");

        const bool has_child_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "SaveButton.Destroy";
        });
        expect(has_child_destroy_event,
               "native owner Release declarative same-PRG child subtree should dispatch the child Destroy method");

        const bool has_form_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "DemoForm.Destroy";
        });
        expect(has_form_destroy_event,
               "native owner Release declarative same-PRG child subtree should dispatch the owner Destroy method");

        fs::remove_all(temp_root, ignored);
    }

    void test_inherited_owner_release_recursively_destroys_declarative_same_prg_child_subtree_and_invalidates_owner_chain()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_inherited_release_owner_declarative_same_prg_child_subtree";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "inherited_release_owner_declarative_same_prg_child_subtree.prg";
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
            "lDictSet = SETPEM(oDict, 'comparemode', 110)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ParentForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "    PROCEDURE Destroy\n"
            "        cDestroyLog = cDestroyLog + 'form>'\n"
            "        nFormDestroyCount = nFormDestroyCount + 1\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ChildForm AS ParentForm\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ParentButton AS Custom\n"
            "    Caption = 'Save'\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS ParentButton\n"
            "    ADD OBJECT lblBadge AS BadgeLabel\n"
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

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("inherited owner Release declarative same-PRG child subtree script should complete: ") + state.message +
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
        check("nchildclasscountbeforerelease", "4");
        check("cchildclass1beforerelease", "SAVEBUTTON");
        check("cchildclass2beforerelease", "PARENTBUTTON");
        check("cchildclass3beforerelease", "CUSTOM");
        check("cchildclass4beforerelease", "OBJECT");
        check("lhaschildbeforerelease", "true");
        check("cchildcaptionbeforerelease", "Save");
        check("cgrandchildcaptionbeforerelease", "Badge");
        check("lreleased", "true");
        check("cdestroylogafter", "badge>child>form>");
        check("cchildparentcaptionafter", "MainForm");
        check("cgrandchildparentcaptionafter", "Save");
        check("nformdestroycountafter", "1");
        check("nchilddestroycountafter", "1");
        check("ngrandchilddestroycountafter", "1");
        check("lhaschildafterrelease", "false");
        check("lchildhasparentafterrelease", "false");
        check("lchildhasclasslibraryafterrelease", "false");
        check("lgrandchildhasparentafterrelease", "false");
        check("lformhascaptionafterrelease", "false");
        check("ldictset", "true");
        check("ndictcompare", "110");

        const auto child_after_release = state.globals.find("xchildafterrelease");
        expect(child_after_release != state.globals.end() &&
                   child_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited owner Release declarative same-PRG child subtree should invalidate GETPEM() for the released child slot");

        const auto child_parent_after_release = state.globals.find("xchildparentafterrelease");
        expect(child_parent_after_release != state.globals.end() &&
                   child_parent_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited owner Release declarative same-PRG child subtree should invalidate GETPEM() for the released child's Parent");

        const auto child_class_library_after_release = state.globals.find("xchildclasslibraryafterrelease");
        expect(child_class_library_after_release != state.globals.end() &&
                   child_class_library_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited owner Release declarative same-PRG child subtree should invalidate GETPEM() for the released child's ClassLibrary");

        const auto grandchild_parent_after_release = state.globals.find("xgrandchildparentafterrelease");
        expect(grandchild_parent_after_release != state.globals.end() &&
                   grandchild_parent_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited owner Release declarative same-PRG child subtree should invalidate GETPEM() for the released grandchild's Parent");

        const auto form_caption_after_release = state.globals.find("xformcaptionafterrelease");
        expect(form_caption_after_release != state.globals.end() &&
                   form_caption_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited owner Release declarative same-PRG child subtree should invalidate GETPEM() for the released owner");

        expect(state.ole_objects.size() == 1U,
               "inherited owner Release declarative same-PRG child subtree should leave only the COM object registered after recursive release");
        if (state.ole_objects.size() == 1U)
        {
            expect(state.ole_objects[0].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while inherited owner declarative same-PRG Release lands");
        }

        const bool has_grandchild_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "BadgeLabel.Destroy";
        });
        expect(has_grandchild_destroy_event,
               "inherited owner Release declarative same-PRG child subtree should dispatch the grandchild Destroy method");

        const bool has_child_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "SaveButton.Destroy";
        });
        expect(has_child_destroy_event,
               "inherited owner Release declarative same-PRG child subtree should dispatch the child Destroy method");

        const bool has_form_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "ParentForm.Destroy";
        });
        expect(has_form_destroy_event,
               "inherited owner Release declarative same-PRG child subtree should dispatch the owner Destroy method");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_owner_release_recursively_destroys_declarative_external_child_subtree_and_invalidates_owner_chain()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_release_owner_declarative_external_child_subtree";
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
            "    ADD OBJECT lblBadge AS BadgeLabel\n"
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

        const fs::path main_path = temp_root / "native_release_owner_declarative_external_child_subtree.prg";
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
            "lDictSet = SETPEM(oDict, 'comparemode', 111)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    ADD OBJECT cmdSave AS SaveButton OF buttons.prg\n"
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
               std::string("native owner Release declarative external child subtree script should complete: ") + state.message +
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
        check("cchildcaptionbeforerelease", "Save");
        check("cgrandchildcaptionbeforerelease", "Badge");
        check("lreleased", "true");
        check("cdestroylogafter", "badge>child>form>");
        check("cchildparentcaptionafter", "MainForm");
        check("cgrandchildparentcaptionafter", "Save");
        check("nformdestroycountafter", "1");
        check("nchilddestroycountafter", "1");
        check("ngrandchilddestroycountafter", "1");
        check("lhaschildafterrelease", "false");
        check("lchildhasparentafterrelease", "false");
        check("lchildhasclasslibraryafterrelease", "false");
        check("lgrandchildhasparentafterrelease", "false");
        check("lformhascaptionafterrelease", "false");
        check("ldictset", "true");
        check("ndictcompare", "111");

        const auto child_after_release = state.globals.find("xchildafterrelease");
        expect(child_after_release != state.globals.end() &&
                   child_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native owner Release declarative external child subtree should invalidate GETPEM() for the released child slot");

        const auto child_parent_after_release = state.globals.find("xchildparentafterrelease");
        expect(child_parent_after_release != state.globals.end() &&
                   child_parent_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native owner Release declarative external child subtree should invalidate GETPEM() for the released child's Parent");

        const auto child_class_library_after_release = state.globals.find("xchildclasslibraryafterrelease");
        expect(child_class_library_after_release != state.globals.end() &&
                   child_class_library_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native owner Release declarative external child subtree should invalidate GETPEM() for the released child's ClassLibrary");

        const auto grandchild_parent_after_release = state.globals.find("xgrandchildparentafterrelease");
        expect(grandchild_parent_after_release != state.globals.end() &&
                   grandchild_parent_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native owner Release declarative external child subtree should invalidate GETPEM() for the released grandchild's Parent");

        const auto form_caption_after_release = state.globals.find("xformcaptionafterrelease");
        expect(form_caption_after_release != state.globals.end() &&
                   form_caption_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native owner Release declarative external child subtree should invalidate GETPEM() for the released owner");

        expect(state.ole_objects.size() == 1U,
               "native owner Release declarative external child subtree should leave only the COM object registered after recursive release");
        if (state.ole_objects.size() == 1U)
        {
            expect(state.ole_objects[0].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native owner declarative external Release lands");
        }

        const bool has_grandchild_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "BadgeLabel.Destroy";
        });
        expect(has_grandchild_destroy_event,
               "native owner Release declarative external child subtree should dispatch the grandchild Destroy method");

        const bool has_child_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "SaveButton.Destroy";
        });
        expect(has_child_destroy_event,
               "native owner Release declarative external child subtree should dispatch the child Destroy method");

        const bool has_form_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "DemoForm.Destroy";
        });
        expect(has_form_destroy_event,
               "native owner Release declarative external child subtree should dispatch the owner Destroy method");

        fs::remove_all(temp_root, ignored);
    }

    void test_inherited_owner_release_recursively_destroys_declarative_external_child_subtree_and_invalidates_owner_chain()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_inherited_release_owner_declarative_external_child_subtree";
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
            "    ADD OBJECT lblBadge AS BadgeLabel\n"
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
            "    ADD OBJECT cmdSave AS SaveButton OF buttons.prg\n"
            "    PROCEDURE Destroy\n"
            "        cDestroyLog = cDestroyLog + 'form>'\n"
            "        nFormDestroyCount = nFormDestroyCount + 1\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "inherited_release_owner_declarative_external_child_subtree.prg";
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
            "lDictSet = SETPEM(oDict, 'comparemode', 112)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("inherited owner Release declarative external child subtree script should complete: ") + state.message +
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
        check("cchildcaptionbeforerelease", "Save");
        check("cgrandchildcaptionbeforerelease", "Badge");
        check("lreleased", "true");
        check("cdestroylogafter", "badge>child>form>");
        check("cchildparentcaptionafter", "MainForm");
        check("cgrandchildparentcaptionafter", "Save");
        check("nformdestroycountafter", "1");
        check("nchilddestroycountafter", "1");
        check("ngrandchilddestroycountafter", "1");
        check("lhaschildafterrelease", "false");
        check("lchildhasparentafterrelease", "false");
        check("lchildhasclasslibraryafterrelease", "false");
        check("lgrandchildhasparentafterrelease", "false");
        check("lformhascaptionafterrelease", "false");
        check("ldictset", "true");
        check("ndictcompare", "112");

        const auto child_after_release = state.globals.find("xchildafterrelease");
        expect(child_after_release != state.globals.end() &&
                   child_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited owner Release declarative external child subtree should invalidate GETPEM() for the released child slot");

        const auto child_parent_after_release = state.globals.find("xchildparentafterrelease");
        expect(child_parent_after_release != state.globals.end() &&
                   child_parent_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited owner Release declarative external child subtree should invalidate GETPEM() for the released child's Parent");

        const auto child_class_library_after_release = state.globals.find("xchildclasslibraryafterrelease");
        expect(child_class_library_after_release != state.globals.end() &&
                   child_class_library_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited owner Release declarative external child subtree should invalidate GETPEM() for the released child's ClassLibrary");

        const auto grandchild_parent_after_release = state.globals.find("xgrandchildparentafterrelease");
        expect(grandchild_parent_after_release != state.globals.end() &&
                   grandchild_parent_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited owner Release declarative external child subtree should invalidate GETPEM() for the released grandchild's Parent");

        const auto form_caption_after_release = state.globals.find("xformcaptionafterrelease");
        expect(form_caption_after_release != state.globals.end() &&
                   form_caption_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited owner Release declarative external child subtree should invalidate GETPEM() for the released owner");

        expect(state.ole_objects.size() == 1U,
               "inherited owner Release declarative external child subtree should leave only the COM object registered after recursive release");
        if (state.ole_objects.size() == 1U)
        {
            expect(state.ole_objects[0].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while inherited owner declarative external Release lands");
        }

        const bool has_grandchild_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "BadgeLabel.Destroy";
        });
        expect(has_grandchild_destroy_event,
               "inherited owner Release declarative external child subtree should dispatch the grandchild Destroy method");

        const bool has_child_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "SaveButton.Destroy";
        });
        expect(has_child_destroy_event,
               "inherited owner Release declarative external child subtree should dispatch the child Destroy method");

        const bool has_form_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "ParentForm.Destroy";
        });
        expect(has_form_destroy_event,
               "inherited owner Release declarative external child subtree should dispatch the owner Destroy method");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_release_detaches_object_block_child_subtree_while_owner_and_sibling_remain_alive()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_release_object_block_child_subtree";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_release_object_block_child_subtree.prg";
        write_text(
            main_path,
            "cDestroyLog = ''\n"
            "cChildParentCaptionDuringDestroy = ''\n"
            "cGrandchildParentCaptionDuringDestroy = ''\n"
            "nChildDestroyCount = 0\n"
            "nGrandchildDestroyCount = 0\n"
            "oForm = CREATEOBJECT('DemoForm')\n"
            "oChild = oForm.cmdSave\n"
            "oSibling = oForm.cmdCancel\n"
            "oGrandchild = oChild.lblBadge\n"
            "cChildClassLibraryBeforeRelease = oChild.ClassLibrary\n"
            "cGrandchildCaptionBeforeRelease = oGrandchild.Caption\n"
            "cSiblingCaptionBeforeRelease = oSibling.Caption\n"
            "lReleased = oChild.Release()\n"
            "cDestroyLogAfter = cDestroyLog\n"
            "cChildParentCaptionAfter = cChildParentCaptionDuringDestroy\n"
            "cGrandchildParentCaptionAfter = cGrandchildParentCaptionDuringDestroy\n"
            "nChildDestroyCountAfter = nChildDestroyCount\n"
            "nGrandchildDestroyCountAfter = nGrandchildDestroyCount\n"
            "cOwnerCaptionAfterRelease = oForm.Caption\n"
            "lOwnerStillHasSave = PEMSTATUS(oForm, 'cmdSave', 1)\n"
            "xReleasedChild = GETPEM(oForm, 'cmdSave')\n"
            "lOwnerStillHasCancel = PEMSTATUS(oForm, 'cmdCancel', 1)\n"
            "cSiblingCaptionAfterRelease = oForm.cmdCancel.Caption\n"
            "lHeldChildHasParentAfterRelease = PEMSTATUS(oChild, 'Parent', 1)\n"
            "xHeldChildParentAfterRelease = GETPEM(oChild, 'Parent')\n"
            "lHeldChildHasClassLibraryAfterRelease = PEMSTATUS(oChild, 'ClassLibrary', 1)\n"
            "xHeldChildClassLibraryAfterRelease = GETPEM(oChild, 'ClassLibrary')\n"
            "lHeldGrandchildHasParentAfterRelease = PEMSTATUS(oGrandchild, 'Parent', 1)\n"
            "xHeldGrandchildParentAfterRelease = GETPEM(oGrandchild, 'Parent')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 101)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    OBJECT cmdSave AS SaveButton\n"
            "        Caption = 'Commit'\n"
            "    ENDOBJECT\n"
            "    OBJECT cmdCancel AS CancelButton\n"
            "        Caption = 'Cancel'\n"
            "    ENDOBJECT\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
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
            "ENDDEFINE\n"
            "DEFINE CLASS CancelButton AS Custom\n"
            "    Caption = 'Cancel'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Release object-block child subtree script should complete: ") + state.message +
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

        check("cchildclasslibrarybeforerelease", "");
        check("cgrandchildcaptionbeforerelease", "Badge");
        check("csiblingcaptionbeforerelease", "Cancel");
        check("lreleased", "true");
        check("cdestroylogafter", "badge>child>");
        check("cchildparentcaptionafter", "MainForm");
        check("cgrandchildparentcaptionafter", "Commit");
        check("nchilddestroycountafter", "1");
        check("ngrandchilddestroycountafter", "1");
        check("cownercaptionafterrelease", "MainForm");
        check("lownerstillhassave", "false");
        check("lownerstillhascancel", "true");
        check("csiblingcaptionafterrelease", "Cancel");
        check("lheldchildhasparentafterrelease", "false");
        check("lheldchildhasclasslibraryafterrelease", "false");
        check("lheldgrandchildhasparentafterrelease", "false");
        check("ldictset", "true");
        check("ndictcompare", "101");

        const auto released_child = state.globals.find("xreleasedchild");
        expect(released_child != state.globals.end() &&
                   released_child->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native Release object-block child subtree should invalidate GETPEM() on the owner's released child slot");

        const auto held_child_parent = state.globals.find("xheldchildparentafterrelease");
        expect(held_child_parent != state.globals.end() &&
                   held_child_parent->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native Release object-block child subtree should invalidate GETPEM() on the held child's Parent");

        const auto held_child_class_library = state.globals.find("xheldchildclasslibraryafterrelease");
        expect(held_child_class_library != state.globals.end() &&
                   held_child_class_library->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native Release object-block child subtree should invalidate GETPEM() on the held child's ClassLibrary");

        const auto held_grandchild_parent = state.globals.find("xheldgrandchildparentafterrelease");
        expect(held_grandchild_parent != state.globals.end() &&
                   held_grandchild_parent->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native Release object-block child subtree should invalidate GETPEM() on the released grandchild's Parent");

        expect(state.ole_objects.size() == 3U,
               "native Release object-block child subtree should leave owner, surviving sibling, and COM objects registered");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "DemoForm",
                   "native Release object-block child subtree should keep the owner object alive");
            expect(!state.ole_objects[0].properties.contains("cmdsave"),
                   "native Release object-block child subtree should detach the released child from the owner");
            expect(state.ole_objects[0].properties.contains("cmdcancel"),
                   "native Release object-block child subtree should preserve the surviving sibling on the owner");
            expect(state.ole_objects[1].prog_id == "CancelButton",
                   "native Release object-block child subtree should preserve the surviving sibling object");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native object-block subtree Release lands");
        }

        const bool has_child_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "SaveButton.Destroy";
        });
        expect(has_child_destroy_event,
               "native Release object-block child subtree should dispatch the child Destroy method");

        const bool has_grandchild_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "BadgeLabel.Destroy";
        });
        expect(has_grandchild_destroy_event,
               "native Release object-block child subtree should dispatch descendant Destroy methods");

        fs::remove_all(temp_root, ignored);
    }

    void test_inherited_external_base_release_detaches_object_block_child_subtree_while_owner_and_sibling_remain_alive()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_inherited_external_release_object_block_child_subtree";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path library_path = temp_root / "widgetlib.prg";
        write_text(
            library_path,
            "DEFINE CLASS ParentForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    OBJECT cmdSave AS SaveButton\n"
            "        Caption = 'Commit'\n"
            "    ENDOBJECT\n"
            "    OBJECT cmdCancel AS CancelButton\n"
            "        Caption = 'Cancel'\n"
            "    ENDOBJECT\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
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
            "ENDDEFINE\n"
            "DEFINE CLASS CancelButton AS Custom\n"
            "    Caption = 'Cancel'\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "inherited_external_release_object_block_child_subtree.prg";
        write_text(
            main_path,
            "cDestroyLog = ''\n"
            "cChildParentCaptionDuringDestroy = ''\n"
            "cGrandchildParentCaptionDuringDestroy = ''\n"
            "nChildDestroyCount = 0\n"
            "nGrandchildDestroyCount = 0\n"
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oChild = oCreate.cmdSave\n"
            "oSibling = oCreate.cmdCancel\n"
            "oGrandchild = oChild.lblBadge\n"
            "cChildClassLibraryBeforeRelease = oChild.ClassLibrary\n"
            "cGrandchildCaptionBeforeRelease = oGrandchild.Caption\n"
            "cSiblingCaptionBeforeRelease = oSibling.Caption\n"
            "lReleased = oChild.Release()\n"
            "cDestroyLogAfter = cDestroyLog\n"
            "cChildParentCaptionAfter = cChildParentCaptionDuringDestroy\n"
            "cGrandchildParentCaptionAfter = cGrandchildParentCaptionDuringDestroy\n"
            "nChildDestroyCountAfter = nChildDestroyCount\n"
            "nGrandchildDestroyCountAfter = nGrandchildDestroyCount\n"
            "cOwnerCaptionAfterRelease = oCreate.Caption\n"
            "lOwnerStillHasSave = PEMSTATUS(oCreate, 'cmdSave', 1)\n"
            "xReleasedChild = GETPEM(oCreate, 'cmdSave')\n"
            "lOwnerStillHasCancel = PEMSTATUS(oCreate, 'cmdCancel', 1)\n"
            "cSiblingCaptionAfterRelease = oCreate.cmdCancel.Caption\n"
            "lHeldChildHasParentAfterRelease = PEMSTATUS(oChild, 'Parent', 1)\n"
            "xHeldChildParentAfterRelease = GETPEM(oChild, 'Parent')\n"
            "lHeldChildHasClassLibraryAfterRelease = PEMSTATUS(oChild, 'ClassLibrary', 1)\n"
            "xHeldChildClassLibraryAfterRelease = GETPEM(oChild, 'ClassLibrary')\n"
            "lHeldGrandchildHasParentAfterRelease = PEMSTATUS(oGrandchild, 'Parent', 1)\n"
            "xHeldGrandchildParentAfterRelease = GETPEM(oGrandchild, 'Parent')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 102)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("inherited external Release object-block child subtree script should complete: ") + state.message +
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

        check("cchildclasslibrarybeforerelease", "");
        check("cgrandchildcaptionbeforerelease", "Badge");
        check("csiblingcaptionbeforerelease", "Cancel");
        check("lreleased", "true");
        check("cdestroylogafter", "badge>child>");
        check("cchildparentcaptionafter", "MainForm");
        check("cgrandchildparentcaptionafter", "Commit");
        check("nchilddestroycountafter", "1");
        check("ngrandchilddestroycountafter", "1");
        check("cownercaptionafterrelease", "MainForm");
        check("lownerstillhassave", "false");
        check("lownerstillhascancel", "true");
        check("csiblingcaptionafterrelease", "Cancel");
        check("lheldchildhasparentafterrelease", "false");
        check("lheldchildhasclasslibraryafterrelease", "false");
        check("lheldgrandchildhasparentafterrelease", "false");
        check("ldictset", "true");
        check("ndictcompare", "102");

        const auto released_child = state.globals.find("xreleasedchild");
        expect(released_child != state.globals.end() &&
                   released_child->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited external Release object-block child subtree should invalidate GETPEM() on the owner's released child slot");

        const auto held_child_parent = state.globals.find("xheldchildparentafterrelease");
        expect(held_child_parent != state.globals.end() &&
                   held_child_parent->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited external Release object-block child subtree should invalidate GETPEM() on the held child's Parent");

        const auto held_child_class_library = state.globals.find("xheldchildclasslibraryafterrelease");
        expect(held_child_class_library != state.globals.end() &&
                   held_child_class_library->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited external Release object-block child subtree should invalidate GETPEM() on the held child's ClassLibrary");

        const auto held_grandchild_parent = state.globals.find("xheldgrandchildparentafterrelease");
        expect(held_grandchild_parent != state.globals.end() &&
                   held_grandchild_parent->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited external Release object-block child subtree should invalidate GETPEM() on the released grandchild's Parent");

        expect(state.ole_objects.size() == 3U,
               "inherited external Release object-block child subtree should leave owner, surviving sibling, and COM objects registered");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "ChildForm",
                   "inherited external Release object-block child subtree should keep the owner object alive");
            expect(!state.ole_objects[0].properties.contains("cmdsave"),
                   "inherited external Release object-block child subtree should detach the released child from the owner");
            expect(state.ole_objects[0].properties.contains("cmdcancel"),
                   "inherited external Release object-block child subtree should preserve the surviving sibling on the owner");
            expect(state.ole_objects[1].prog_id == "CancelButton",
                   "inherited external Release object-block child subtree should preserve the surviving sibling object");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while inherited external object-block subtree Release lands");
        }

        const bool has_child_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "SaveButton.Destroy";
        });
        expect(has_child_destroy_event,
               "inherited external Release object-block child subtree should dispatch the child Destroy method");

        const bool has_grandchild_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "BadgeLabel.Destroy";
        });
        expect(has_grandchild_destroy_event,
               "inherited external Release object-block child subtree should dispatch descendant Destroy methods");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_release_detaches_object_block_external_child_subtree_while_owner_and_sibling_remain_alive()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_release_object_block_external_child_subtree";
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

        const fs::path main_path = temp_root / "native_release_object_block_external_child_subtree.prg";
        write_text(
            main_path,
            "cDestroyLog = ''\n"
            "cChildParentCaptionDuringDestroy = ''\n"
            "cGrandchildParentCaptionDuringDestroy = ''\n"
            "nChildDestroyCount = 0\n"
            "nGrandchildDestroyCount = 0\n"
            "oForm = CREATEOBJECT('DemoForm')\n"
            "oChild = oForm.cmdSave\n"
            "oSibling = oForm.cmdCancel\n"
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
            "cGrandchildCaptionBeforeRelease = oGrandchild.Caption\n"
            "cSiblingCaptionBeforeRelease = oSibling.Caption\n"
            "lReleased = oChild.Release()\n"
            "cDestroyLogAfter = cDestroyLog\n"
            "cChildParentCaptionAfter = cChildParentCaptionDuringDestroy\n"
            "cGrandchildParentCaptionAfter = cGrandchildParentCaptionDuringDestroy\n"
            "nChildDestroyCountAfter = nChildDestroyCount\n"
            "nGrandchildDestroyCountAfter = nGrandchildDestroyCount\n"
            "cOwnerCaptionAfterRelease = oForm.Caption\n"
            "lOwnerStillHasSave = PEMSTATUS(oForm, 'cmdSave', 1)\n"
            "xReleasedChild = GETPEM(oForm, 'cmdSave')\n"
            "lOwnerStillHasCancel = PEMSTATUS(oForm, 'cmdCancel', 1)\n"
            "cSiblingCaptionAfterRelease = oForm.cmdCancel.Caption\n"
            "lHeldChildHasParentAfterRelease = PEMSTATUS(oChild, 'Parent', 1)\n"
            "xHeldChildParentAfterRelease = GETPEM(oChild, 'Parent')\n"
            "lHeldChildHasClassLibraryAfterRelease = PEMSTATUS(oChild, 'ClassLibrary', 1)\n"
            "xHeldChildClassLibraryAfterRelease = GETPEM(oChild, 'ClassLibrary')\n"
            "lHeldGrandchildHasParentAfterRelease = PEMSTATUS(oGrandchild, 'Parent', 1)\n"
            "xHeldGrandchildParentAfterRelease = GETPEM(oGrandchild, 'Parent')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 103)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    OBJECT cmdSave AS SaveButton OF buttons.prg\n"
            "        Caption = 'Commit'\n"
            "    ENDOBJECT\n"
            "    OBJECT cmdCancel AS CancelButton\n"
            "        Caption = 'Cancel'\n"
            "    ENDOBJECT\n"
            "ENDDEFINE\n"
            "DEFINE CLASS CancelButton AS Custom\n"
            "    Caption = 'Cancel'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Release object-block external child subtree script should complete: ") + state.message +
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
        check("cgrandchildcaptionbeforerelease", "Badge");
        check("csiblingcaptionbeforerelease", "Cancel");
        check("lreleased", "true");
        check("cdestroylogafter", "badge>child>");
        check("cchildparentcaptionafter", "MainForm");
        check("cgrandchildparentcaptionafter", "Commit");
        check("nchilddestroycountafter", "1");
        check("ngrandchilddestroycountafter", "1");
        check("cownercaptionafterrelease", "MainForm");
        check("lownerstillhassave", "false");
        check("lownerstillhascancel", "true");
        check("csiblingcaptionafterrelease", "Cancel");
        check("lheldchildhasparentafterrelease", "false");
        check("lheldchildhasclasslibraryafterrelease", "false");
        check("lheldgrandchildhasparentafterrelease", "false");
        check("ldictset", "true");
        check("ndictcompare", "103");

        const auto released_child = state.globals.find("xreleasedchild");
        expect(released_child != state.globals.end() &&
                   released_child->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native Release object-block external child subtree should invalidate GETPEM() on the owner's released child slot");

        const auto held_child_parent = state.globals.find("xheldchildparentafterrelease");
        expect(held_child_parent != state.globals.end() &&
                   held_child_parent->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native Release object-block external child subtree should invalidate GETPEM() on the held child's Parent");

        const auto held_child_class_library = state.globals.find("xheldchildclasslibraryafterrelease");
        expect(held_child_class_library != state.globals.end() &&
                   held_child_class_library->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native Release object-block external child subtree should invalidate GETPEM() on the held child's ClassLibrary");

        const auto held_grandchild_parent = state.globals.find("xheldgrandchildparentafterrelease");
        expect(held_grandchild_parent != state.globals.end() &&
                   held_grandchild_parent->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native Release object-block external child subtree should invalidate GETPEM() on the released grandchild's Parent");

        expect(state.ole_objects.size() == 3U,
               "native Release object-block external child subtree should leave owner, surviving sibling, and COM objects registered");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "DemoForm",
                   "native Release object-block external child subtree should keep the owner object alive");
            expect(state.ole_objects[0].source == main_path.string(),
                   "native Release object-block external child subtree should preserve the owner source path");
            expect(!state.ole_objects[0].properties.contains("cmdsave"),
                   "native Release object-block external child subtree should detach the released child from the owner");
            expect(state.ole_objects[0].properties.contains("cmdcancel"),
                   "native Release object-block external child subtree should preserve the surviving sibling on the owner");
            expect(state.ole_objects[1].prog_id == "CancelButton",
                   "native Release object-block external child subtree should preserve the surviving sibling object");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native object-block external-child subtree Release lands");
        }

        const bool has_child_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "SaveButton.Destroy";
        });
        expect(has_child_destroy_event,
               "native Release object-block external child subtree should dispatch the child Destroy method");

        const bool has_grandchild_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "BadgeLabel.Destroy";
        });
        expect(has_grandchild_destroy_event,
               "native Release object-block external child subtree should dispatch descendant Destroy methods");

        fs::remove_all(temp_root, ignored);
    }

    void test_inherited_external_base_release_detaches_object_block_external_child_subtree_while_owner_and_sibling_remain_alive()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_inherited_external_release_object_block_external_child_subtree";
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
            "    OBJECT cmdCancel AS CancelButton\n"
            "        Caption = 'Cancel'\n"
            "    ENDOBJECT\n"
            "ENDDEFINE\n"
            "DEFINE CLASS CancelButton AS Custom\n"
            "    Caption = 'Cancel'\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "inherited_external_release_object_block_external_child_subtree.prg";
        write_text(
            main_path,
            "cDestroyLog = ''\n"
            "cChildParentCaptionDuringDestroy = ''\n"
            "cGrandchildParentCaptionDuringDestroy = ''\n"
            "nChildDestroyCount = 0\n"
            "nGrandchildDestroyCount = 0\n"
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oChild = oCreate.cmdSave\n"
            "oSibling = oCreate.cmdCancel\n"
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
            "cGrandchildCaptionBeforeRelease = oGrandchild.Caption\n"
            "cSiblingCaptionBeforeRelease = oSibling.Caption\n"
            "lReleased = oChild.Release()\n"
            "cDestroyLogAfter = cDestroyLog\n"
            "cChildParentCaptionAfter = cChildParentCaptionDuringDestroy\n"
            "cGrandchildParentCaptionAfter = cGrandchildParentCaptionDuringDestroy\n"
            "nChildDestroyCountAfter = nChildDestroyCount\n"
            "nGrandchildDestroyCountAfter = nGrandchildDestroyCount\n"
            "cOwnerCaptionAfterRelease = oCreate.Caption\n"
            "lOwnerStillHasSave = PEMSTATUS(oCreate, 'cmdSave', 1)\n"
            "xReleasedChild = GETPEM(oCreate, 'cmdSave')\n"
            "lOwnerStillHasCancel = PEMSTATUS(oCreate, 'cmdCancel', 1)\n"
            "cSiblingCaptionAfterRelease = oCreate.cmdCancel.Caption\n"
            "lHeldChildHasParentAfterRelease = PEMSTATUS(oChild, 'Parent', 1)\n"
            "xHeldChildParentAfterRelease = GETPEM(oChild, 'Parent')\n"
            "lHeldChildHasClassLibraryAfterRelease = PEMSTATUS(oChild, 'ClassLibrary', 1)\n"
            "xHeldChildClassLibraryAfterRelease = GETPEM(oChild, 'ClassLibrary')\n"
            "lHeldGrandchildHasParentAfterRelease = PEMSTATUS(oGrandchild, 'Parent', 1)\n"
            "xHeldGrandchildParentAfterRelease = GETPEM(oGrandchild, 'Parent')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 104)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("inherited external Release object-block external child subtree script should complete: ") + state.message +
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
        check("cgrandchildcaptionbeforerelease", "Badge");
        check("csiblingcaptionbeforerelease", "Cancel");
        check("lreleased", "true");
        check("cdestroylogafter", "badge>child>");
        check("cchildparentcaptionafter", "MainForm");
        check("cgrandchildparentcaptionafter", "Commit");
        check("nchilddestroycountafter", "1");
        check("ngrandchilddestroycountafter", "1");
        check("cownercaptionafterrelease", "MainForm");
        check("lownerstillhassave", "false");
        check("lownerstillhascancel", "true");
        check("csiblingcaptionafterrelease", "Cancel");
        check("lheldchildhasparentafterrelease", "false");
        check("lheldchildhasclasslibraryafterrelease", "false");
        check("lheldgrandchildhasparentafterrelease", "false");
        check("ldictset", "true");
        check("ndictcompare", "104");

        const auto released_child = state.globals.find("xreleasedchild");
        expect(released_child != state.globals.end() &&
                   released_child->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited external Release object-block external child subtree should invalidate GETPEM() on the owner's released child slot");

        const auto held_child_parent = state.globals.find("xheldchildparentafterrelease");
        expect(held_child_parent != state.globals.end() &&
                   held_child_parent->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited external Release object-block external child subtree should invalidate GETPEM() on the held child's Parent");

        const auto held_child_class_library = state.globals.find("xheldchildclasslibraryafterrelease");
        expect(held_child_class_library != state.globals.end() &&
                   held_child_class_library->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited external Release object-block external child subtree should invalidate GETPEM() on the held child's ClassLibrary");

        const auto held_grandchild_parent = state.globals.find("xheldgrandchildparentafterrelease");
        expect(held_grandchild_parent != state.globals.end() &&
                   held_grandchild_parent->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited external Release object-block external child subtree should invalidate GETPEM() on the released grandchild's Parent");

        expect(state.ole_objects.size() == 3U,
               "inherited external Release object-block external child subtree should leave owner, surviving sibling, and COM objects registered");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "ChildForm",
                   "inherited external Release object-block external child subtree should keep the owner object alive");
            expect(state.ole_objects[0].source == main_path.string(),
                   "inherited external Release object-block external child subtree should preserve the derived owner source path");
            expect(!state.ole_objects[0].properties.contains("cmdsave"),
                   "inherited external Release object-block external child subtree should detach the released child from the owner");
            expect(state.ole_objects[0].properties.contains("cmdcancel"),
                   "inherited external Release object-block external child subtree should preserve the surviving sibling on the owner");
            expect(state.ole_objects[1].prog_id == "CancelButton",
                   "inherited external Release object-block external child subtree should preserve the surviving sibling object");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while inherited external object-block external-child subtree Release lands");
        }

        const bool has_child_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "SaveButton.Destroy";
        });
        expect(has_child_destroy_event,
               "inherited external Release object-block external child subtree should dispatch the child Destroy method");

        const bool has_grandchild_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "BadgeLabel.Destroy";
        });
        expect(has_grandchild_destroy_event,
               "inherited external Release object-block external child subtree should dispatch descendant Destroy methods");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_owner_release_recursively_destroys_same_prg_object_block_child_subtree_and_invalidates_owner_chain()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_release_owner_same_prg_object_block_child_subtree";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_release_owner_same_prg_object_block_child_subtree.prg";
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
            "lDictSet = SETPEM(oDict, 'comparemode', 107)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
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
            "DEFINE CLASS ParentButton AS Custom\n"
            "    Caption = 'Save'\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS ParentButton\n"
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

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native owner Release same-PRG object-block child subtree script should complete: ") + state.message +
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
        check("nchildclasscountbeforerelease", "4");
        check("cchildclass1beforerelease", "SAVEBUTTON");
        check("cchildclass2beforerelease", "PARENTBUTTON");
        check("cchildclass3beforerelease", "CUSTOM");
        check("cchildclass4beforerelease", "OBJECT");
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
        check("ndictcompare", "107");

        const auto child_after_release = state.globals.find("xchildafterrelease");
        expect(child_after_release != state.globals.end() &&
                   child_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native owner Release same-PRG object-block child subtree should invalidate GETPEM() for the released child slot");

        const auto child_parent_after_release = state.globals.find("xchildparentafterrelease");
        expect(child_parent_after_release != state.globals.end() &&
                   child_parent_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native owner Release same-PRG object-block child subtree should invalidate GETPEM() for the released child's Parent");

        const auto child_class_library_after_release = state.globals.find("xchildclasslibraryafterrelease");
        expect(child_class_library_after_release != state.globals.end() &&
                   child_class_library_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native owner Release same-PRG object-block child subtree should invalidate GETPEM() for the released child's ClassLibrary");

        const auto grandchild_parent_after_release = state.globals.find("xgrandchildparentafterrelease");
        expect(grandchild_parent_after_release != state.globals.end() &&
                   grandchild_parent_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native owner Release same-PRG object-block child subtree should invalidate GETPEM() for the released grandchild's Parent");

        const auto form_caption_after_release = state.globals.find("xformcaptionafterrelease");
        expect(form_caption_after_release != state.globals.end() &&
                   form_caption_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native owner Release same-PRG object-block child subtree should invalidate GETPEM() for the released owner");

        expect(state.ole_objects.size() == 1U,
               "native owner Release same-PRG object-block child subtree should leave only the COM object registered after recursive release");
        if (state.ole_objects.size() == 1U)
        {
            expect(state.ole_objects[0].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native owner same-PRG object-block Release lands");
        }

        const bool has_grandchild_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "BadgeLabel.Destroy";
        });
        expect(has_grandchild_destroy_event,
               "native owner Release same-PRG object-block child subtree should dispatch the grandchild Destroy method");

        const bool has_child_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "SaveButton.Destroy";
        });
        expect(has_child_destroy_event,
               "native owner Release same-PRG object-block child subtree should dispatch the child Destroy method");

        const bool has_form_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "DemoForm.Destroy";
        });
        expect(has_form_destroy_event,
               "native owner Release same-PRG object-block child subtree should dispatch the owner Destroy method");

        fs::remove_all(temp_root, ignored);
    }

    void test_inherited_owner_release_recursively_destroys_same_prg_object_block_child_subtree_and_invalidates_owner_chain()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_inherited_release_owner_same_prg_object_block_child_subtree";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "inherited_release_owner_same_prg_object_block_child_subtree.prg";
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
            "lDictSet = SETPEM(oDict, 'comparemode', 108)\n"
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
            "DEFINE CLASS ParentButton AS Custom\n"
            "    Caption = 'Save'\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS ParentButton\n"
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

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("inherited owner Release same-PRG object-block child subtree script should complete: ") + state.message +
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
        check("nchildclasscountbeforerelease", "4");
        check("cchildclass1beforerelease", "SAVEBUTTON");
        check("cchildclass2beforerelease", "PARENTBUTTON");
        check("cchildclass3beforerelease", "CUSTOM");
        check("cchildclass4beforerelease", "OBJECT");
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
        check("ndictcompare", "108");

        const auto child_after_release = state.globals.find("xchildafterrelease");
        expect(child_after_release != state.globals.end() &&
                   child_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited owner Release same-PRG object-block child subtree should invalidate GETPEM() for the released child slot");

        const auto child_parent_after_release = state.globals.find("xchildparentafterrelease");
        expect(child_parent_after_release != state.globals.end() &&
                   child_parent_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited owner Release same-PRG object-block child subtree should invalidate GETPEM() for the released child's Parent");

        const auto child_class_library_after_release = state.globals.find("xchildclasslibraryafterrelease");
        expect(child_class_library_after_release != state.globals.end() &&
                   child_class_library_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited owner Release same-PRG object-block child subtree should invalidate GETPEM() for the released child's ClassLibrary");

        const auto grandchild_parent_after_release = state.globals.find("xgrandchildparentafterrelease");
        expect(grandchild_parent_after_release != state.globals.end() &&
                   grandchild_parent_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited owner Release same-PRG object-block child subtree should invalidate GETPEM() for the released grandchild's Parent");

        const auto form_caption_after_release = state.globals.find("xformcaptionafterrelease");
        expect(form_caption_after_release != state.globals.end() &&
                   form_caption_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited owner Release same-PRG object-block child subtree should invalidate GETPEM() for the released owner");

        expect(state.ole_objects.size() == 1U,
               "inherited owner Release same-PRG object-block child subtree should leave only the COM object registered after recursive release");
        if (state.ole_objects.size() == 1U)
        {
            expect(state.ole_objects[0].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while inherited owner same-PRG object-block Release lands");
        }

        const bool has_grandchild_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "BadgeLabel.Destroy";
        });
        expect(has_grandchild_destroy_event,
               "inherited owner Release same-PRG object-block child subtree should dispatch the grandchild Destroy method");

        const bool has_child_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "SaveButton.Destroy";
        });
        expect(has_child_destroy_event,
               "inherited owner Release same-PRG object-block child subtree should dispatch the child Destroy method");

        const bool has_form_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "ParentForm.Destroy";
        });
        expect(has_form_destroy_event,
               "inherited owner Release same-PRG object-block child subtree should dispatch the owner Destroy method");

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

        const fs::path library_path = temp_root / "widgetlib.prg";
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

    void test_same_prg_native_class_inheritance_applies_parent_defaults_methods_and_init()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_class_inheritance";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_class_inheritance.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildWidget')\n"
            "oNew = NEWOBJECT('LeafWidget')\n"
            "oPlain = CREATEOBJECT('Empty')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "oPlain.Extra = 'plain'\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 6)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "cCreateDescribe = oCreate.Describe()\n"
            "cCreateWho = oCreate.Who()\n"
            "cNewDescribe = oNew.Describe()\n"
            "cNewWho = oNew.Who()\n"
            "cPlain = oPlain.Extra\n"
            "nCreateBase = oCreate.nBase\n"
            "nCreateChild = oCreate.nChild\n"
            "lCreateInitRan = oCreate.lInitRan\n"
            "nNewBase = oNew.nBase\n"
            "nNewChild = oNew.nChild\n"
            "lNewInitRan = oNew.lInitRan\n"
            "RETURN\n"
            "DEFINE CLASS ParentWidget AS Custom\n"
            "    Caption = 'Parent'\n"
            "    nBase = 3\n"
            "    lInitRan = .F.\n"
            "    PROCEDURE Init\n"
            "        THIS.Caption = THIS.Caption + '-Init'\n"
            "        THIS.lInitRan = .T.\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "    FUNCTION Describe\n"
            "        RETURN THIS.Caption\n"
            "    ENDFUNC\n"
            "    FUNCTION Who\n"
            "        RETURN 'Parent'\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ChildWidget AS ParentWidget\n"
            "    Caption = 'Child'\n"
            "    nChild = 7\n"
            "    FUNCTION Who\n"
            "        RETURN 'Child'\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS LeafWidget AS ChildWidget\n"
            "    nBase = 11\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native class inheritance script should complete: ") + state.message +
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

        check("ccreatedescribe", "Child-Init");
        check("ccreatewho", "Child");
        check("cnewdescribe", "Child-Init");
        check("cnewwho", "Child");
        check("cplain", "plain");
        check("ncreatebase", "3");
        check("ncreatechild", "7");
        check("lcreateinitran", "true");
        check("nnewbase", "11");
        check("nnewchild", "7");
        check("lnewinitran", "true");
        check("ldictset", "true");
        check("ndictcompare", "6");

        expect(state.ole_objects.size() == 4U,
               "native class inheritance script should register CREATEOBJECT, NEWOBJECT, plain, and COM objects");
        if (state.ole_objects.size() == 4U)
        {
            const auto &create_object = state.ole_objects[0];
            expect(create_object.prog_id == "ChildWidget",
                   "native class inheritance should preserve child class identity");
            const auto create_caption = create_object.properties.find("caption");
            const auto create_base = create_object.properties.find("nbase");
            const auto create_child = create_object.properties.find("nchild");
            const auto create_init = create_object.properties.find("linitran");
            expect(create_caption != create_object.properties.end(),
                   "native class inheritance should materialize inherited Init-updated caption state");
            expect(create_base != create_object.properties.end(),
                   "native class inheritance should materialize inherited parent properties");
            expect(create_child != create_object.properties.end(),
                   "native class inheritance should keep child-local properties");
            expect(create_init != create_object.properties.end(),
                   "native class inheritance should materialize inherited Init-written flags");
            if (create_caption != create_object.properties.end())
            {
                expect(copperfin::runtime::format_value(create_caption->second) == "Child-Init",
                       "native class inheritance should let inherited Init see child-overridden properties");
            }
            if (create_base != create_object.properties.end())
            {
                expect(copperfin::runtime::format_value(create_base->second) == "3",
                       "native class inheritance should keep parent default properties on child instances");
            }
            if (create_child != create_object.properties.end())
            {
                expect(copperfin::runtime::format_value(create_child->second) == "7",
                       "native class inheritance should keep child-local default properties");
            }
            if (create_init != create_object.properties.end())
            {
                expect(copperfin::runtime::format_value(create_init->second) == "true",
                       "native class inheritance should run inherited Init when the child does not override it");
            }
            expect(std::find(create_object.methods.begin(), create_object.methods.end(), "Describe") != create_object.methods.end(),
                   "native class inheritance should expose inherited methods on the runtime object");
            expect(std::find(create_object.methods.begin(), create_object.methods.end(), "Who") != create_object.methods.end(),
                   "native class inheritance should retain child overrides in the runtime method list");
            expect(std::find(create_object.methods.begin(), create_object.methods.end(), "Init") != create_object.methods.end(),
                   "native class inheritance should expose inherited Init in runtime member enumeration");

            const auto &new_object = state.ole_objects[1];
            expect(new_object.prog_id == "LeafWidget",
                   "multilevel native class inheritance should preserve leaf class identity");
            const auto new_base = new_object.properties.find("nbase");
            const auto new_child = new_object.properties.find("nchild");
            if (new_base != new_object.properties.end())
            {
                expect(copperfin::runtime::format_value(new_base->second) == "11",
                       "multilevel native class inheritance should let leaf defaults override inherited parent values");
            }
            else
            {
                expect(false, "multilevel native class inheritance should materialize leaf override properties");
            }
            if (new_child != new_object.properties.end())
            {
                expect(copperfin::runtime::format_value(new_child->second) == "7",
                       "multilevel native class inheritance should retain intermediate inherited properties");
            }
            else
            {
                expect(false, "multilevel native class inheritance should materialize intermediate inherited properties");
            }

            expect(state.ole_objects[2].prog_id == "Empty",
                   "plain CREATEOBJECT should remain stable while native inheritance lands");
            expect(state.ole_objects[3].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native inheritance lands");
        }

        const bool has_inherited_init_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.init" &&
                   event.detail == "ParentWidget.Init";
        });
        expect(has_inherited_init_event,
               "native class inheritance should emit the inherited Init event when the parent Init runs");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_class_inheritance_loads_external_prg_base_sources()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_external_base_inheritance";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path library_path = temp_root / "widgetlib.prg";
        write_text(
            library_path,
            "DEFINE CLASS ParentWidget AS Custom\n"
            "    Caption = 'Parent'\n"
            "    nBase = 4\n"
            "    lInitRan = .F.\n"
            "    PROCEDURE Init\n"
            "        THIS.Caption = THIS.Caption + '-Init'\n"
            "        THIS.lInitRan = .T.\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "    FUNCTION Describe\n"
            "        RETURN THIS.Caption\n"
            "    ENDFUNC\n"
            "    FUNCTION Who\n"
            "        RETURN 'Parent'\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "native_external_base_inheritance.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildWidget')\n"
            "oNew = NEWOBJECT('LeafWidget')\n"
            "oPlain = CREATEOBJECT('Empty')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "oPlain.Extra = 'plain'\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 16)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "cCreateDescribe = oCreate.Describe()\n"
            "cCreateWho = oCreate.Who()\n"
            "cNewDescribe = oNew.Describe()\n"
            "cNewWho = oNew.Who()\n"
            "cPlain = oPlain.Extra\n"
            "nCreateBase = oCreate.nBase\n"
            "nCreateChild = oCreate.nChild\n"
            "lCreateInitRan = oCreate.lInitRan\n"
            "nNewBase = oNew.nBase\n"
            "nNewChild = oNew.nChild\n"
            "lNewInitRan = oNew.lInitRan\n"
            "RETURN\n"
            "DEFINE CLASS ChildWidget AS ParentWidget OF widgetlib.prg\n"
            "    Caption = 'Child'\n"
            "    nChild = 9\n"
            "    FUNCTION Who\n"
            "        RETURN 'Child'\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS LeafWidget AS ChildWidget\n"
            "    nBase = 12\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external base class inheritance script should complete: ") + state.message +
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

        check("ccreatedescribe", "Child-Init");
        check("ccreatewho", "Child");
        check("cnewdescribe", "Child-Init");
        check("cnewwho", "Child");
        check("cplain", "plain");
        check("ncreatebase", "4");
        check("ncreatechild", "9");
        check("lcreateinitran", "true");
        check("nnewbase", "12");
        check("nnewchild", "9");
        check("lnewinitran", "true");
        check("ldictset", "true");
        check("ndictcompare", "16");

        expect(state.ole_objects.size() == 4U,
               "external base class inheritance should register CREATEOBJECT, NEWOBJECT, plain, and COM objects");
        if (state.ole_objects.size() == 4U)
        {
            const auto &create_object = state.ole_objects[0];
            expect(create_object.prog_id == "ChildWidget",
                   "external base class inheritance should preserve child class identity");
            expect(create_object.source == main_path.string(),
                   "external base class inheritance should preserve the child class source as object provenance");

            const auto create_caption = create_object.properties.find("caption");
            const auto create_base = create_object.properties.find("nbase");
            const auto create_child = create_object.properties.find("nchild");
            const auto create_init = create_object.properties.find("linitran");
            expect(create_caption != create_object.properties.end(),
                   "external base class inheritance should materialize inherited Init-updated caption state");
            expect(create_base != create_object.properties.end(),
                   "external base class inheritance should materialize parent defaults from the external PRG library");
            expect(create_child != create_object.properties.end(),
                   "external base class inheritance should keep child-local properties");
            expect(create_init != create_object.properties.end(),
                   "external base class inheritance should materialize inherited Init flags from the external PRG library");
            if (create_caption != create_object.properties.end())
            {
                expect(copperfin::runtime::format_value(create_caption->second) == "Child-Init",
                       "external base class inheritance should let inherited external Init see child-overridden properties");
            }
            if (create_base != create_object.properties.end())
            {
                expect(copperfin::runtime::format_value(create_base->second) == "4",
                       "external base class inheritance should keep parent defaults from the external PRG library");
            }
            if (create_child != create_object.properties.end())
            {
                expect(copperfin::runtime::format_value(create_child->second) == "9",
                       "external base class inheritance should keep child-local default properties");
            }
            if (create_init != create_object.properties.end())
            {
                expect(copperfin::runtime::format_value(create_init->second) == "true",
                       "external base class inheritance should run inherited external Init when the child does not override it");
            }
            expect(std::find(create_object.methods.begin(), create_object.methods.end(), "Describe") != create_object.methods.end(),
                   "external base class inheritance should expose inherited external methods on the runtime object");
            expect(std::find(create_object.methods.begin(), create_object.methods.end(), "Who") != create_object.methods.end(),
                   "external base class inheritance should retain child overrides in the runtime method list");
            expect(std::find(create_object.methods.begin(), create_object.methods.end(), "Init") != create_object.methods.end(),
                   "external base class inheritance should expose inherited external Init in runtime member enumeration");

            const auto &new_object = state.ole_objects[1];
            expect(new_object.prog_id == "LeafWidget",
                   "external base class inheritance should preserve leaf class identity");
            const auto new_base = new_object.properties.find("nbase");
            const auto new_child = new_object.properties.find("nchild");
            if (new_base != new_object.properties.end())
            {
                expect(copperfin::runtime::format_value(new_base->second) == "12",
                       "external base class inheritance should let leaf defaults override external parent values");
            }
            else
            {
                expect(false, "external base class inheritance should materialize leaf override properties");
            }
            if (new_child != new_object.properties.end())
            {
                expect(copperfin::runtime::format_value(new_child->second) == "9",
                       "external base class inheritance should retain inherited child properties above the external base");
            }
            else
            {
                expect(false, "external base class inheritance should materialize inherited child properties");
            }

            expect(state.ole_objects[2].prog_id == "Empty",
                   "plain CREATEOBJECT should remain stable while external base inheritance lands");
            expect(state.ole_objects[3].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external base inheritance lands");
        }

        const bool has_external_inherited_init_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.init" &&
                   event.detail == "ParentWidget.Init";
        });
        expect(has_external_inherited_init_event,
               "external base class inheritance should emit the inherited external Init event when the parent Init runs");

        fs::remove_all(temp_root, ignored);
    }

    void test_same_prg_native_aclass_reflects_inheritance_chain()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_aclass_inheritance";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_aclass_inheritance.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildWidget')\n"
            "oPlain = CREATEOBJECT('Empty')\n"
            "nClassCount = ACLASS(aClass, oCreate)\n"
            "cClass1 = aClass[1]\n"
            "cClass2 = aClass[2]\n"
            "cClass3 = aClass[3]\n"
            "cClass4 = aClass[4]\n"
            "cClass5 = aClass[5]\n"
            "RETURN\n"
            "DEFINE CLASS RootWidget AS Custom\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ParentWidget AS RootWidget\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ChildWidget AS ParentWidget\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native ACLASS inheritance script should complete: ") + state.message +
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

        check("nclasscount", "5");
        check("cclass1", "CHILDWIDGET");
        check("cclass2", "PARENTWIDGET");
        check("cclass3", "ROOTWIDGET");
        check("cclass4", "CUSTOM");
        check("cclass5", "OBJECT");

        expect(state.ole_objects.size() == 2U,
               "native ACLASS inheritance should register native and plain objects");
        if (state.ole_objects.size() == 2U)
        {
            const auto &native_object = state.ole_objects[0];
            expect(native_object.prog_id == "ChildWidget",
                   "native ACLASS inheritance should preserve child class identity");
            expect(native_object.class_hierarchy.size() == 5U,
                   "native ACLASS inheritance should persist the native class hierarchy on runtime objects");
            if (native_object.class_hierarchy.size() == 5U)
            {
                expect(native_object.class_hierarchy[0] == "CHILDWIDGET",
                       "native ACLASS inheritance should store the derived class first");
                expect(native_object.class_hierarchy[1] == "PARENTWIDGET",
                       "native ACLASS inheritance should store the immediate parent second");
                expect(native_object.class_hierarchy[2] == "ROOTWIDGET",
                       "native ACLASS inheritance should store deeper native ancestors");
                expect(native_object.class_hierarchy[3] == "CUSTOM",
                       "native ACLASS inheritance should preserve the builtin base token");
                expect(native_object.class_hierarchy[4] == "OBJECT",
                       "native ACLASS inheritance should preserve the terminal object token");
            }

            expect(state.ole_objects[1].prog_id == "Empty",
                   "plain CREATEOBJECT should remain stable while native ACLASS inheritance lands");
            expect(state.ole_objects[1].class_hierarchy.empty(),
                   "plain CREATEOBJECT should keep the native-only class hierarchy empty");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_external_prg_base_aclass_reflects_inheritance_chain()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_prg_aclass_inheritance";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path library_path = temp_root / "widgetlib.prg";
        write_text(
            library_path,
            "DEFINE CLASS RootWidget AS Custom\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ParentWidget AS RootWidget\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_aclass_inheritance.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildWidget')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 24)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "nClassCount = ACLASS(aClass, oCreate)\n"
            "cClass1 = aClass[1]\n"
            "cClass2 = aClass[2]\n"
            "cClass3 = aClass[3]\n"
            "cClass4 = aClass[4]\n"
            "cClass5 = aClass[5]\n"
            "RETURN\n"
            "DEFINE CLASS ChildWidget AS ParentWidget OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external-base ACLASS inheritance script should complete: ") + state.message +
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

        check("nclasscount", "5");
        check("cclass1", "CHILDWIDGET");
        check("cclass2", "PARENTWIDGET");
        check("cclass3", "ROOTWIDGET");
        check("cclass4", "CUSTOM");
        check("cclass5", "OBJECT");
        check("ldictset", "true");
        check("ndictcompare", "24");

        expect(state.ole_objects.size() == 2U,
               "external-base ACLASS inheritance should register native and COM objects");
        if (state.ole_objects.size() == 2U)
        {
            const auto &native_object = state.ole_objects[0];
            expect(native_object.prog_id == "ChildWidget",
                   "external-base ACLASS inheritance should preserve child class identity");
            expect(native_object.class_hierarchy.size() == 5U,
                   "external-base ACLASS inheritance should persist the external-base class hierarchy");
            if (native_object.class_hierarchy.size() == 5U)
            {
                expect(native_object.class_hierarchy[0] == "CHILDWIDGET",
                       "external-base ACLASS inheritance should store the derived class first");
                expect(native_object.class_hierarchy[1] == "PARENTWIDGET",
                       "external-base ACLASS inheritance should store the inherited external parent second");
                expect(native_object.class_hierarchy[2] == "ROOTWIDGET",
                       "external-base ACLASS inheritance should store deeper external native ancestors");
                expect(native_object.class_hierarchy[3] == "CUSTOM",
                       "external-base ACLASS inheritance should preserve the builtin base token");
                expect(native_object.class_hierarchy[4] == "OBJECT",
                       "external-base ACLASS inheritance should preserve the terminal object token");
            }

            expect(state.ole_objects[1].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external-base ACLASS inheritance lands");
            expect(state.ole_objects[1].class_hierarchy.empty(),
                   "COM NEWOBJECT should keep the native-only class hierarchy empty");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_same_prg_native_baseclass_reflects_through_getpem_and_pemstatus()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_baseclass_reflection";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_baseclass_reflection.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildWidget')\n"
            "oPlain = CREATEOBJECT('Empty')\n"
            "cBaseClass = GETPEM(oCreate, 'BaseClass')\n"
            "lHasBaseClass = PEMSTATUS(oCreate, 'BaseClass', 1)\n"
            "lBaseClassReadOnly = PEMSTATUS(oCreate, 'BaseClass', 5)\n"
            "xClassLibrary = GETPEM(oCreate, 'ClassLibrary')\n"
            "lHasClassLibrary = PEMSTATUS(oCreate, 'ClassLibrary', 1)\n"
            "RETURN\n"
            "DEFINE CLASS RootWidget AS Custom\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ChildWidget AS RootWidget\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native BaseClass reflection script should complete: ") + state.message +
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

        check("cbaseclass", "RootWidget");
        check("lhasbaseclass", "true");
        check("lbaseclassreadonly", "true");
        check("lhasclasslibrary", "false");

        const auto class_library = state.globals.find("xclasslibrary");
        expect(class_library != state.globals.end() &&
                   class_library->second.kind == copperfin::runtime::PrgValueKind::empty,
               "same-PRG BaseClass reflection should leave ClassLibrary empty");

        expect(state.ole_objects.size() == 2U,
               "native BaseClass reflection should register native and plain objects");
        if (state.ole_objects.size() == 2U)
        {
            const auto &native_object = state.ole_objects[0];
            expect(native_object.prog_id == "ChildWidget",
                   "native BaseClass reflection should preserve child class identity");
            expect(native_object.base_class_name == "RootWidget",
                   "native BaseClass reflection should persist the immediate native base class name");
            expect(native_object.class_library.empty(),
                   "native BaseClass reflection should keep same-PRG class library provenance empty");

            expect(state.ole_objects[1].prog_id == "Empty",
                   "plain CREATEOBJECT should remain stable while native BaseClass reflection lands");
            expect(state.ole_objects[1].base_class_name.empty(),
                   "plain CREATEOBJECT should not fabricate native base-class metadata");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_external_prg_base_identity_reflects_through_getpem_and_pemstatus()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_prg_base_identity_reflection";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path library_path = temp_root / "widgetlib.prg";
        write_text(
            library_path,
            "DEFINE CLASS ParentWidget AS Custom\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_base_identity_reflection.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildWidget')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 25)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "cBaseClass = GETPEM(oCreate, 'BaseClass')\n"
            "cClassLibrary = GETPEM(oCreate, 'ClassLibrary')\n"
            "lHasBaseClass = PEMSTATUS(oCreate, 'BaseClass', 1)\n"
            "lHasClassLibrary = PEMSTATUS(oCreate, 'ClassLibrary', 1)\n"
            "lClassLibraryReadOnly = PEMSTATUS(oCreate, 'ClassLibrary', 5)\n"
            "RETURN\n"
            "DEFINE CLASS ChildWidget AS ParentWidget OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external-base identity reflection script should complete: ") + state.message +
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

        check("cbaseclass", "ParentWidget");
        check("cclasslibrary", library_path.string());
        check("lhasbaseclass", "true");
        check("lhasclasslibrary", "true");
        check("lclasslibraryreadonly", "true");
        check("ldictset", "true");
        check("ndictcompare", "25");

        expect(state.ole_objects.size() == 2U,
               "external-base identity reflection should register native and COM objects");
        if (state.ole_objects.size() == 2U)
        {
            const auto &native_object = state.ole_objects[0];
            expect(native_object.prog_id == "ChildWidget",
                   "external-base identity reflection should preserve child class identity");
            expect(native_object.base_class_name == "ParentWidget",
                   "external-base identity reflection should persist the immediate external base class name");
            expect(native_object.class_library == library_path.string(),
                   "external-base identity reflection should persist the resolved external class library path");

            expect(state.ole_objects[1].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external-base identity reflection lands");
            expect(state.ole_objects[1].base_class_name.empty(),
                   "COM NEWOBJECT should not fabricate native base-class metadata");
            expect(state.ole_objects[1].class_library.empty(),
                   "COM NEWOBJECT should not fabricate native class-library metadata");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_same_prg_native_class_reflects_through_getpem_and_pemstatus()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_class_reflection";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_class_reflection.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildWidget')\n"
            "oPlain = CREATEOBJECT('Empty')\n"
            "cClass = GETPEM(oCreate, 'Class')\n"
            "lHasClass = PEMSTATUS(oCreate, 'Class', 1)\n"
            "lClassReadOnly = PEMSTATUS(oCreate, 'Class', 5)\n"
            "xPlainClass = GETPEM(oPlain, 'Class')\n"
            "lPlainHasClass = PEMSTATUS(oPlain, 'Class', 1)\n"
            "RETURN\n"
            "DEFINE CLASS ParentWidget AS Custom\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ChildWidget AS ParentWidget\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Class reflection script should complete: ") + state.message +
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

        check("cclass", "ChildWidget");
        check("lhasclass", "true");
        check("lclassreadonly", "true");
        check("lplainhasclass", "false");

        const auto plain_class = state.globals.find("xplainclass");
        expect(plain_class != state.globals.end() &&
                   plain_class->second.kind == copperfin::runtime::PrgValueKind::empty,
               "plain CREATEOBJECT should keep native Class reflection empty");

        expect(state.ole_objects.size() == 2U,
               "native Class reflection should register native and plain objects");
        if (state.ole_objects.size() == 2U)
        {
            expect(state.ole_objects[0].prog_id == "ChildWidget",
                   "native Class reflection should preserve child class identity");
            expect(state.ole_objects[0].class_hierarchy.size() == 4U,
                   "native Class reflection should keep native hierarchy metadata intact");
            expect(state.ole_objects[1].prog_id == "Empty",
                   "plain CREATEOBJECT should remain stable while native Class reflection lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_external_prg_base_class_reflects_through_getpem_and_pemstatus()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_prg_class_reflection";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path library_path = temp_root / "widgetlib.prg";
        write_text(
            library_path,
            "DEFINE CLASS ParentWidget AS Custom\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_class_reflection.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildWidget')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 26)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "cClass = GETPEM(oCreate, 'Class')\n"
            "lHasClass = PEMSTATUS(oCreate, 'Class', 1)\n"
            "lClassReadOnly = PEMSTATUS(oCreate, 'Class', 5)\n"
            "RETURN\n"
            "DEFINE CLASS ChildWidget AS ParentWidget OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external-base Class reflection script should complete: ") + state.message +
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

        check("cclass", "ChildWidget");
        check("lhasclass", "true");
        check("lclassreadonly", "true");
        check("ldictset", "true");
        check("ndictcompare", "26");

        expect(state.ole_objects.size() == 2U,
               "external-base Class reflection should register native and COM objects");
        if (state.ole_objects.size() == 2U)
        {
            expect(state.ole_objects[0].prog_id == "ChildWidget",
                   "external-base Class reflection should preserve child class identity");
            expect(state.ole_objects[0].class_library == library_path.string(),
                   "external-base Class reflection should preserve external library provenance");
            expect(state.ole_objects[1].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external-base Class reflection lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_same_prg_native_parentclass_reflects_through_getpem_and_pemstatus()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_parentclass_reflection";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_parentclass_reflection.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildWidget')\n"
            "oPlain = CREATEOBJECT('Empty')\n"
            "cParentClass = GETPEM(oCreate, 'ParentClass')\n"
            "lHasParentClass = PEMSTATUS(oCreate, 'ParentClass', 1)\n"
            "lParentClassReadOnly = PEMSTATUS(oCreate, 'ParentClass', 5)\n"
            "xPlainParentClass = GETPEM(oPlain, 'ParentClass')\n"
            "lPlainHasParentClass = PEMSTATUS(oPlain, 'ParentClass', 1)\n"
            "RETURN\n"
            "DEFINE CLASS RootWidget AS Custom\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ChildWidget AS RootWidget\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native ParentClass reflection script should complete: ") + state.message +
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

        check("cparentclass", "RootWidget");
        check("lhasparentclass", "true");
        check("lparentclassreadonly", "true");
        check("lplainhasparentclass", "false");

        const auto plain_parent_class = state.globals.find("xplainparentclass");
        expect(plain_parent_class != state.globals.end() &&
                   plain_parent_class->second.kind == copperfin::runtime::PrgValueKind::empty,
               "plain CREATEOBJECT should keep native ParentClass reflection empty");

        expect(state.ole_objects.size() == 2U,
               "native ParentClass reflection should register native and plain objects");
        if (state.ole_objects.size() == 2U)
        {
            const auto &native_object = state.ole_objects[0];
            expect(native_object.prog_id == "ChildWidget",
                   "native ParentClass reflection should preserve child class identity");
            expect(native_object.base_class_name == "RootWidget",
                   "native ParentClass reflection should preserve the immediate parent class name");

            expect(state.ole_objects[1].prog_id == "Empty",
                   "plain CREATEOBJECT should remain stable while native ParentClass reflection lands");
            expect(state.ole_objects[1].base_class_name.empty(),
                   "plain CREATEOBJECT should not fabricate native ParentClass metadata");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_external_prg_parentclass_reflects_through_getpem_and_pemstatus()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_prg_parentclass_reflection";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path library_path = temp_root / "widgetlib.prg";
        write_text(
            library_path,
            "DEFINE CLASS ParentWidget AS Custom\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_parentclass_reflection.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildWidget')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 33)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "cParentClass = GETPEM(oCreate, 'ParentClass')\n"
            "lHasParentClass = PEMSTATUS(oCreate, 'ParentClass', 1)\n"
            "lParentClassReadOnly = PEMSTATUS(oCreate, 'ParentClass', 5)\n"
            "RETURN\n"
            "DEFINE CLASS ChildWidget AS ParentWidget OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external-base ParentClass reflection script should complete: ") + state.message +
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

        check("cparentclass", "ParentWidget");
        check("lhasparentclass", "true");
        check("lparentclassreadonly", "true");
        check("ldictset", "true");
        check("ndictcompare", "33");

        expect(state.ole_objects.size() == 2U,
               "external-base ParentClass reflection should register native and COM objects");
        if (state.ole_objects.size() == 2U)
        {
            const auto &native_object = state.ole_objects[0];
            expect(native_object.prog_id == "ChildWidget",
                   "external-base ParentClass reflection should preserve child class identity");
            expect(native_object.base_class_name == "ParentWidget",
                   "external-base ParentClass reflection should preserve the immediate external parent class name");
            expect(native_object.class_library == library_path.string(),
                   "external-base ParentClass reflection should preserve external library provenance");

            expect(state.ole_objects[1].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external-base ParentClass reflection lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_same_prg_native_identity_metadata_appears_in_amembers()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_identity_amembers";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_identity_amembers.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildWidget')\n"
            "nMembersProps = AMEMBERS(aMembersProps, oCreate, 1)\n"
            "nMembersUnion = AMEMBERS(aMembersUnion, oCreate, 3)\n"
            "cProp1 = aMembersProps[1]\n"
            "cProp2 = aMembersProps[2]\n"
            "cProp3 = aMembersProps[3]\n"
            "cProp4 = aMembersProps[4]\n"
            "cUnion1 = aMembersUnion[1]\n"
            "cUnion4 = aMembersUnion[4]\n"
            "RETURN\n"
            "DEFINE CLASS ParentWidget AS Custom\n"
            "    Caption = 'Parent'\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ChildWidget AS ParentWidget\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native identity AMEMBERS script should complete: ") + state.message +
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

        check("nmembersprops", "4");
        check("nmembersunion", "4");
        check("cprop1", "BASECLASS");
        check("cprop2", "CAPTION");
        check("cprop3", "CLASS");
        check("cprop4", "PARENTCLASS");
        check("cunion1", "BASECLASS");
        check("cunion4", "PARENTCLASS");

        expect(state.ole_objects.size() == 1U,
               "native identity AMEMBERS should register one native object");
        if (state.ole_objects.size() == 1U)
        {
            expect(state.ole_objects[0].prog_id == "ChildWidget",
                   "native identity AMEMBERS should preserve child class identity");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_external_prg_base_identity_metadata_appears_in_amembers()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_prg_identity_amembers";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path library_path = temp_root / "widgetlib.prg";
        write_text(
            library_path,
            "DEFINE CLASS ParentWidget AS Custom\n"
            "    Caption = 'Parent'\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_identity_amembers.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildWidget')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 27)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "nMembersProps = AMEMBERS(aMembersProps, oCreate, 1)\n"
            "nMembersUnion = AMEMBERS(aMembersUnion, oCreate, 3)\n"
            "cProp1 = aMembersProps[1]\n"
            "cProp2 = aMembersProps[2]\n"
            "cProp3 = aMembersProps[3]\n"
            "cProp4 = aMembersProps[4]\n"
            "cProp5 = aMembersProps[5]\n"
            "cUnion1 = aMembersUnion[1]\n"
            "cUnion5 = aMembersUnion[5]\n"
            "RETURN\n"
            "DEFINE CLASS ChildWidget AS ParentWidget OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external identity AMEMBERS script should complete: ") + state.message +
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

        check("nmembersprops", "5");
        check("nmembersunion", "5");
        check("cprop1", "BASECLASS");
        check("cprop2", "CAPTION");
        check("cprop3", "CLASS");
        check("cprop4", "CLASSLIBRARY");
        check("cprop5", "PARENTCLASS");
        check("cunion1", "BASECLASS");
        check("cunion5", "PARENTCLASS");
        check("ldictset", "true");
        check("ndictcompare", "27");

        expect(state.ole_objects.size() == 2U,
               "external identity AMEMBERS should register native and COM objects");
        if (state.ole_objects.size() == 2U)
        {
            expect(state.ole_objects[0].prog_id == "ChildWidget",
                   "external identity AMEMBERS should preserve child class identity");
            expect(state.ole_objects[1].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external identity AMEMBERS lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_same_prg_child_aclass_reflects_inheritance_chain()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_child_aclass_inheritance";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_child_aclass_inheritance.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('DemoForm')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 60)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "nClassCount = ACLASS(aClass, oCreate.cmdSave)\n"
            "cClass1 = aClass[1]\n"
            "cClass2 = aClass[2]\n"
            "cClass3 = aClass[3]\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native child ACLASS inheritance script should complete: ") + state.message +
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

        check("nclasscount", "3");
        check("cclass1", "SAVEBUTTON");
        check("cclass2", "CUSTOM");
        check("cclass3", "OBJECT");
        check("ldictset", "true");
        check("ndictcompare", "60");

        expect(state.ole_objects.size() == 3U,
               "native child ACLASS inheritance should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "DemoForm",
                   "native child ACLASS inheritance should preserve parent form identity");
            const auto &child_object = state.ole_objects[1];
            expect(child_object.prog_id == "SaveButton",
                   "native child ACLASS inheritance should preserve child class identity");
            expect(child_object.class_hierarchy.size() == 3U,
                   "native child ACLASS inheritance should persist the child native class hierarchy");
            if (child_object.class_hierarchy.size() == 3U)
            {
                expect(child_object.class_hierarchy[0] == "SAVEBUTTON",
                       "native child ACLASS inheritance should store the child class first");
                expect(child_object.class_hierarchy[1] == "CUSTOM",
                       "native child ACLASS inheritance should store the builtin base second");
                expect(child_object.class_hierarchy[2] == "OBJECT",
                       "native child ACLASS inheritance should store the terminal object token");
            }
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native child ACLASS inheritance lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_external_base_child_aclass_reflects_inheritance_chain()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_prg_child_aclass_inheritance";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path library_path = temp_root / "widgetlib.prg";
        write_text(
            library_path,
            "DEFINE CLASS ParentForm AS Custom\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_child_aclass_inheritance.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 61)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "nClassCount = ACLASS(aClass, oCreate.cmdSave)\n"
            "cClass1 = aClass[1]\n"
            "cClass2 = aClass[2]\n"
            "cClass3 = aClass[3]\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external child ACLASS inheritance script should complete: ") + state.message +
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

        check("nclasscount", "3");
        check("cclass1", "SAVEBUTTON");
        check("cclass2", "CUSTOM");
        check("cclass3", "OBJECT");
        check("ldictset", "true");
        check("ndictcompare", "61");

        expect(state.ole_objects.size() == 3U,
               "external child ACLASS inheritance should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "ChildForm",
                   "external child ACLASS inheritance should preserve parent form identity");
            const auto &child_object = state.ole_objects[1];
            expect(child_object.prog_id == "SaveButton",
                   "external child ACLASS inheritance should preserve child class identity");
            expect(child_object.class_hierarchy.size() == 3U,
                   "external child ACLASS inheritance should persist the child native class hierarchy");
            if (child_object.class_hierarchy.size() == 3U)
            {
                expect(child_object.class_hierarchy[0] == "SAVEBUTTON",
                       "external child ACLASS inheritance should store the child class first");
                expect(child_object.class_hierarchy[1] == "CUSTOM",
                       "external child ACLASS inheritance should store the builtin base second");
                expect(child_object.class_hierarchy[2] == "OBJECT",
                       "external child ACLASS inheritance should store the terminal object token");
            }
            expect(child_object.source == library_path.string(),
                   "external child ACLASS inheritance should preserve the defining PRG path as child provenance");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external child ACLASS inheritance lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_same_prg_inherited_child_aclass_reflects_deeper_inheritance_chain()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_inherited_child_aclass_inheritance";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_inherited_child_aclass_inheritance.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('DemoForm')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 62)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "nClassCount = ACLASS(aClass, oCreate.cmdSave)\n"
            "cClass1 = aClass[1]\n"
            "cClass2 = aClass[2]\n"
            "cClass3 = aClass[3]\n"
            "cClass4 = aClass[4]\n"
            "cClass5 = aClass[5]\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS RootButton AS Custom\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ParentButton AS RootButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS ParentButton\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native inherited child ACLASS script should complete: ") + state.message +
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

        check("nclasscount", "5");
        check("cclass1", "SAVEBUTTON");
        check("cclass2", "PARENTBUTTON");
        check("cclass3", "ROOTBUTTON");
        check("cclass4", "CUSTOM");
        check("cclass5", "OBJECT");
        check("ldictset", "true");
        check("ndictcompare", "62");

        expect(state.ole_objects.size() == 3U,
               "native inherited child ACLASS should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "DemoForm",
                   "native inherited child ACLASS should preserve parent form identity");
            const auto &child_object = state.ole_objects[1];
            expect(child_object.prog_id == "SaveButton",
                   "native inherited child ACLASS should preserve child class identity");
            expect(child_object.class_hierarchy.size() == 5U,
                   "native inherited child ACLASS should persist the deeper child native class hierarchy");
            if (child_object.class_hierarchy.size() == 5U)
            {
                expect(child_object.class_hierarchy[0] == "SAVEBUTTON",
                       "native inherited child ACLASS should store the derived child class first");
                expect(child_object.class_hierarchy[1] == "PARENTBUTTON",
                       "native inherited child ACLASS should store the intermediate child class second");
                expect(child_object.class_hierarchy[2] == "ROOTBUTTON",
                       "native inherited child ACLASS should store the deeper child ancestor third");
                expect(child_object.class_hierarchy[3] == "CUSTOM",
                       "native inherited child ACLASS should store the builtin base token");
                expect(child_object.class_hierarchy[4] == "OBJECT",
                       "native inherited child ACLASS should store the terminal object token");
            }
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native inherited child ACLASS lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_external_base_inherited_child_aclass_reflects_deeper_inheritance_chain()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_prg_inherited_child_aclass_inheritance";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path library_path = temp_root / "widgetlib.prg";
        write_text(
            library_path,
            "DEFINE CLASS ParentForm AS Custom\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS RootButton AS Custom\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ParentButton AS RootButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS ParentButton\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_inherited_child_aclass_inheritance.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 63)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "nClassCount = ACLASS(aClass, oCreate.cmdSave)\n"
            "cClass1 = aClass[1]\n"
            "cClass2 = aClass[2]\n"
            "cClass3 = aClass[3]\n"
            "cClass4 = aClass[4]\n"
            "cClass5 = aClass[5]\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external inherited child ACLASS script should complete: ") + state.message +
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

        check("nclasscount", "5");
        check("cclass1", "SAVEBUTTON");
        check("cclass2", "PARENTBUTTON");
        check("cclass3", "ROOTBUTTON");
        check("cclass4", "CUSTOM");
        check("cclass5", "OBJECT");
        check("ldictset", "true");
        check("ndictcompare", "63");

        expect(state.ole_objects.size() == 3U,
               "external inherited child ACLASS should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "ChildForm",
                   "external inherited child ACLASS should preserve parent form identity");
            const auto &child_object = state.ole_objects[1];
            expect(child_object.prog_id == "SaveButton",
                   "external inherited child ACLASS should preserve child class identity");
            expect(child_object.class_hierarchy.size() == 5U,
                   "external inherited child ACLASS should persist the deeper child native class hierarchy");
            if (child_object.class_hierarchy.size() == 5U)
            {
                expect(child_object.class_hierarchy[0] == "SAVEBUTTON",
                       "external inherited child ACLASS should store the derived child class first");
                expect(child_object.class_hierarchy[1] == "PARENTBUTTON",
                       "external inherited child ACLASS should store the intermediate child class second");
                expect(child_object.class_hierarchy[2] == "ROOTBUTTON",
                       "external inherited child ACLASS should store the deeper child ancestor third");
                expect(child_object.class_hierarchy[3] == "CUSTOM",
                       "external inherited child ACLASS should store the builtin base token");
                expect(child_object.class_hierarchy[4] == "OBJECT",
                       "external inherited child ACLASS should store the terminal object token");
            }
            expect(child_object.source == library_path.string(),
                   "external inherited child ACLASS should preserve the defining PRG path as child provenance");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external inherited child ACLASS lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_same_prg_native_identity_metadata_stays_read_only_to_setpem()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_identity_setpem";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_identity_setpem.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildWidget')\n"
            "lSetClass = SETPEM(oCreate, 'Class', 'OtherClass')\n"
            "lSetBaseClass = SETPEM(oCreate, 'BaseClass', 'OtherBase')\n"
            "lSetParentClass = SETPEM(oCreate, 'ParentClass', 'OtherParent')\n"
            "lSetClassLibrary = SETPEM(oCreate, 'ClassLibrary', 'other.prg')\n"
            "cClassAfter = GETPEM(oCreate, 'Class')\n"
            "cBaseClassAfter = GETPEM(oCreate, 'BaseClass')\n"
            "cParentClassAfter = GETPEM(oCreate, 'ParentClass')\n"
            "xClassLibraryAfter = GETPEM(oCreate, 'ClassLibrary')\n"
            "RETURN\n"
            "DEFINE CLASS ParentWidget AS Custom\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ChildWidget AS ParentWidget\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native identity SETPEM script should complete: ") + state.message +
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

        check("lsetclass", "false");
        check("lsetbaseclass", "false");
        check("lsetparentclass", "false");
        check("lsetclasslibrary", "false");
        check("cclassafter", "ChildWidget");
        check("cbaseclassafter", "ParentWidget");
        check("cparentclassafter", "ParentWidget");

        const auto class_library_after = state.globals.find("xclasslibraryafter");
        expect(class_library_after != state.globals.end() &&
                   class_library_after->second.kind == copperfin::runtime::PrgValueKind::empty,
               "same-PRG identity SETPEM should leave ClassLibrary empty after failed mutation");

        expect(state.ole_objects.size() == 1U,
               "native identity SETPEM should register one native object");
        if (state.ole_objects.size() == 1U)
        {
            const auto &native_object = state.ole_objects[0];
            expect(native_object.prog_id == "ChildWidget",
                   "native identity SETPEM should preserve child class identity");
            expect(native_object.base_class_name == "ParentWidget",
                   "native identity SETPEM should preserve the immediate base class");
            expect(native_object.class_library.empty(),
                   "native identity SETPEM should preserve empty same-PRG class library provenance");
            expect(!native_object.properties.contains("class"),
                   "native identity SETPEM should not materialize a writable Class property shadow");
            expect(!native_object.properties.contains("baseclass"),
                   "native identity SETPEM should not materialize a writable BaseClass property shadow");
            expect(!native_object.properties.contains("parentclass"),
                   "native identity SETPEM should not materialize a writable ParentClass property shadow");
            expect(!native_object.properties.contains("classlibrary"),
                   "native identity SETPEM should not materialize a writable ClassLibrary property shadow");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_external_prg_identity_metadata_stays_read_only_to_setpem()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_prg_identity_setpem";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path library_path = temp_root / "widgetlib.prg";
        write_text(
            library_path,
            "DEFINE CLASS ParentWidget AS Custom\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_identity_setpem.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildWidget')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 28)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "lSetClass = SETPEM(oCreate, 'Class', 'OtherClass')\n"
            "lSetBaseClass = SETPEM(oCreate, 'BaseClass', 'OtherBase')\n"
            "lSetParentClass = SETPEM(oCreate, 'ParentClass', 'OtherParent')\n"
            "lSetClassLibrary = SETPEM(oCreate, 'ClassLibrary', 'other.prg')\n"
            "cClassAfter = GETPEM(oCreate, 'Class')\n"
            "cBaseClassAfter = GETPEM(oCreate, 'BaseClass')\n"
            "cParentClassAfter = GETPEM(oCreate, 'ParentClass')\n"
            "cClassLibraryAfter = GETPEM(oCreate, 'ClassLibrary')\n"
            "RETURN\n"
            "DEFINE CLASS ChildWidget AS ParentWidget OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external identity SETPEM script should complete: ") + state.message +
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

        check("lsetclass", "false");
        check("lsetbaseclass", "false");
        check("lsetparentclass", "false");
        check("lsetclasslibrary", "false");
        check("cclassafter", "ChildWidget");
        check("cbaseclassafter", "ParentWidget");
        check("cparentclassafter", "ParentWidget");
        check("cclasslibraryafter", library_path.string());
        check("ldictset", "true");
        check("ndictcompare", "28");

        expect(state.ole_objects.size() == 2U,
               "external identity SETPEM should register native and COM objects");
        if (state.ole_objects.size() == 2U)
        {
            const auto &native_object = state.ole_objects[0];
            expect(native_object.prog_id == "ChildWidget",
                   "external identity SETPEM should preserve child class identity");
            expect(native_object.base_class_name == "ParentWidget",
                   "external identity SETPEM should preserve the immediate external base class");
            expect(native_object.class_library == library_path.string(),
                   "external identity SETPEM should preserve external class library provenance");
            expect(!native_object.properties.contains("class"),
                   "external identity SETPEM should not materialize a writable Class property shadow");
            expect(!native_object.properties.contains("baseclass"),
                   "external identity SETPEM should not materialize a writable BaseClass property shadow");
            expect(!native_object.properties.contains("parentclass"),
                   "external identity SETPEM should not materialize a writable ParentClass property shadow");
            expect(!native_object.properties.contains("classlibrary"),
                   "external identity SETPEM should not materialize a writable ClassLibrary property shadow");

            expect(state.ole_objects[1].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external identity SETPEM lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_same_prg_native_identity_metadata_stays_protected_from_removeproperty()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_identity_removeproperty";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_identity_removeproperty.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildWidget')\n"
            "lRemoveClass = REMOVEPROPERTY(oCreate, 'Class')\n"
            "lRemoveBaseClass = REMOVEPROPERTY(oCreate, 'BaseClass')\n"
            "lRemoveParentClass = REMOVEPROPERTY(oCreate, 'ParentClass')\n"
            "lRemoveClassLibrary = REMOVEPROPERTY(oCreate, 'ClassLibrary')\n"
            "cClassAfter = GETPEM(oCreate, 'Class')\n"
            "cBaseClassAfter = GETPEM(oCreate, 'BaseClass')\n"
            "cParentClassAfter = GETPEM(oCreate, 'ParentClass')\n"
            "xClassLibraryAfter = GETPEM(oCreate, 'ClassLibrary')\n"
            "RETURN\n"
            "DEFINE CLASS ParentWidget AS Custom\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ChildWidget AS ParentWidget\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native identity REMOVEPROPERTY script should complete: ") + state.message +
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

        check("lremoveclass", "false");
        check("lremovebaseclass", "false");
        check("lremoveparentclass", "false");
        check("lremoveclasslibrary", "false");
        check("cclassafter", "ChildWidget");
        check("cbaseclassafter", "ParentWidget");
        check("cparentclassafter", "ParentWidget");

        const auto class_library_after = state.globals.find("xclasslibraryafter");
        expect(class_library_after != state.globals.end() &&
                   class_library_after->second.kind == copperfin::runtime::PrgValueKind::empty,
               "same-PRG identity REMOVEPROPERTY should leave ClassLibrary empty after failed removal");

        expect(state.ole_objects.size() == 1U,
               "native identity REMOVEPROPERTY should register one native object");
        if (state.ole_objects.size() == 1U)
        {
            const auto &native_object = state.ole_objects[0];
            expect(native_object.prog_id == "ChildWidget",
                   "native identity REMOVEPROPERTY should preserve child class identity");
            expect(native_object.base_class_name == "ParentWidget",
                   "native identity REMOVEPROPERTY should preserve the immediate base class");
            expect(native_object.class_library.empty(),
                   "native identity REMOVEPROPERTY should preserve empty same-PRG class library provenance");
            expect(!native_object.properties.contains("class"),
                   "native identity REMOVEPROPERTY should not materialize a writable Class property shadow");
            expect(!native_object.properties.contains("baseclass"),
                   "native identity REMOVEPROPERTY should not materialize a writable BaseClass property shadow");
            expect(!native_object.properties.contains("parentclass"),
                   "native identity REMOVEPROPERTY should not materialize a writable ParentClass property shadow");
            expect(!native_object.properties.contains("classlibrary"),
                   "native identity REMOVEPROPERTY should not materialize a writable ClassLibrary property shadow");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_external_prg_identity_metadata_stays_protected_from_removeproperty()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_prg_identity_removeproperty";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path library_path = temp_root / "widgetlib.prg";
        write_text(
            library_path,
            "DEFINE CLASS ParentWidget AS Custom\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_identity_removeproperty.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildWidget')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 29)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "lRemoveClass = REMOVEPROPERTY(oCreate, 'Class')\n"
            "lRemoveBaseClass = REMOVEPROPERTY(oCreate, 'BaseClass')\n"
            "lRemoveParentClass = REMOVEPROPERTY(oCreate, 'ParentClass')\n"
            "lRemoveClassLibrary = REMOVEPROPERTY(oCreate, 'ClassLibrary')\n"
            "cClassAfter = GETPEM(oCreate, 'Class')\n"
            "cBaseClassAfter = GETPEM(oCreate, 'BaseClass')\n"
            "cParentClassAfter = GETPEM(oCreate, 'ParentClass')\n"
            "cClassLibraryAfter = GETPEM(oCreate, 'ClassLibrary')\n"
            "RETURN\n"
            "DEFINE CLASS ChildWidget AS ParentWidget OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external identity REMOVEPROPERTY script should complete: ") + state.message +
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

        check("lremoveclass", "false");
        check("lremovebaseclass", "false");
        check("lremoveparentclass", "false");
        check("lremoveclasslibrary", "false");
        check("cclassafter", "ChildWidget");
        check("cbaseclassafter", "ParentWidget");
        check("cparentclassafter", "ParentWidget");
        check("cclasslibraryafter", library_path.string());
        check("ldictset", "true");
        check("ndictcompare", "29");

        expect(state.ole_objects.size() == 2U,
               "external identity REMOVEPROPERTY should register native and COM objects");
        if (state.ole_objects.size() == 2U)
        {
            const auto &native_object = state.ole_objects[0];
            expect(native_object.prog_id == "ChildWidget",
                   "external identity REMOVEPROPERTY should preserve child class identity");
            expect(native_object.base_class_name == "ParentWidget",
                   "external identity REMOVEPROPERTY should preserve the immediate external base class");
            expect(native_object.class_library == library_path.string(),
                   "external identity REMOVEPROPERTY should preserve external class library provenance");
            expect(!native_object.properties.contains("class"),
                   "external identity REMOVEPROPERTY should not materialize a writable Class property shadow");
            expect(!native_object.properties.contains("baseclass"),
                   "external identity REMOVEPROPERTY should not materialize a writable BaseClass property shadow");
            expect(!native_object.properties.contains("parentclass"),
                   "external identity REMOVEPROPERTY should not materialize a writable ParentClass property shadow");
            expect(!native_object.properties.contains("classlibrary"),
                   "external identity REMOVEPROPERTY should not materialize a writable ClassLibrary property shadow");

            expect(state.ole_objects[1].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external identity REMOVEPROPERTY lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_same_prg_native_identity_metadata_cannot_be_shadowed_through_addproperty()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_identity_addproperty";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_identity_addproperty.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildWidget')\n"
            "lAddClass = ADDPROPERTY(oCreate, 'Class', 'OtherClass')\n"
            "lAddBaseClass = ADDPROPERTY(oCreate, 'BaseClass', 'OtherBase')\n"
            "lAddParentClass = ADDPROPERTY(oCreate, 'ParentClass', 'OtherParent')\n"
            "lAddClassLibrary = ADDPROPERTY(oCreate, 'ClassLibrary', 'other.prg')\n"
            "cClassAfter = GETPEM(oCreate, 'Class')\n"
            "cBaseClassAfter = GETPEM(oCreate, 'BaseClass')\n"
            "cParentClassAfter = GETPEM(oCreate, 'ParentClass')\n"
            "xClassLibraryAfter = GETPEM(oCreate, 'ClassLibrary')\n"
            "RETURN\n"
            "DEFINE CLASS ParentWidget AS Custom\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ChildWidget AS ParentWidget\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native identity ADDPROPERTY script should complete: ") + state.message +
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

        check("laddclass", "false");
        check("laddbaseclass", "false");
        check("laddparentclass", "false");
        check("laddclasslibrary", "false");
        check("cclassafter", "ChildWidget");
        check("cbaseclassafter", "ParentWidget");
        check("cparentclassafter", "ParentWidget");

        const auto class_library_after = state.globals.find("xclasslibraryafter");
        expect(class_library_after != state.globals.end() &&
                   class_library_after->second.kind == copperfin::runtime::PrgValueKind::empty,
               "same-PRG identity ADDPROPERTY should leave ClassLibrary empty after rejected shadow creation");

        expect(state.ole_objects.size() == 1U,
               "native identity ADDPROPERTY should register one native object");
        if (state.ole_objects.size() == 1U)
        {
            const auto &native_object = state.ole_objects[0];
            expect(native_object.prog_id == "ChildWidget",
                   "native identity ADDPROPERTY should preserve child class identity");
            expect(native_object.base_class_name == "ParentWidget",
                   "native identity ADDPROPERTY should preserve the immediate base class");
            expect(native_object.class_library.empty(),
                   "native identity ADDPROPERTY should preserve empty same-PRG class library provenance");
            expect(!native_object.properties.contains("class"),
                   "native identity ADDPROPERTY should not materialize a Class shadow property");
            expect(!native_object.properties.contains("baseclass"),
                   "native identity ADDPROPERTY should not materialize a BaseClass shadow property");
            expect(!native_object.properties.contains("parentclass"),
                   "native identity ADDPROPERTY should not materialize a ParentClass shadow property");
            expect(!native_object.properties.contains("classlibrary"),
                   "native identity ADDPROPERTY should not materialize a ClassLibrary shadow property");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_external_prg_identity_metadata_cannot_be_shadowed_through_addproperty()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_prg_identity_addproperty";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path library_path = temp_root / "widgetlib.prg";
        write_text(
            library_path,
            "DEFINE CLASS ParentWidget AS Custom\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_identity_addproperty.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildWidget')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 30)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "lAddClass = ADDPROPERTY(oCreate, 'Class', 'OtherClass')\n"
            "lAddBaseClass = ADDPROPERTY(oCreate, 'BaseClass', 'OtherBase')\n"
            "lAddParentClass = ADDPROPERTY(oCreate, 'ParentClass', 'OtherParent')\n"
            "lAddClassLibrary = ADDPROPERTY(oCreate, 'ClassLibrary', 'other.prg')\n"
            "cClassAfter = GETPEM(oCreate, 'Class')\n"
            "cBaseClassAfter = GETPEM(oCreate, 'BaseClass')\n"
            "cParentClassAfter = GETPEM(oCreate, 'ParentClass')\n"
            "cClassLibraryAfter = GETPEM(oCreate, 'ClassLibrary')\n"
            "RETURN\n"
            "DEFINE CLASS ChildWidget AS ParentWidget OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external identity ADDPROPERTY script should complete: ") + state.message +
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

        check("laddclass", "false");
        check("laddbaseclass", "false");
        check("laddparentclass", "false");
        check("laddclasslibrary", "false");
        check("cclassafter", "ChildWidget");
        check("cbaseclassafter", "ParentWidget");
        check("cparentclassafter", "ParentWidget");
        check("cclasslibraryafter", library_path.string());
        check("ldictset", "true");
        check("ndictcompare", "30");

        expect(state.ole_objects.size() == 2U,
               "external identity ADDPROPERTY should register native and COM objects");
        if (state.ole_objects.size() == 2U)
        {
            const auto &native_object = state.ole_objects[0];
            expect(native_object.prog_id == "ChildWidget",
                   "external identity ADDPROPERTY should preserve child class identity");
            expect(native_object.base_class_name == "ParentWidget",
                   "external identity ADDPROPERTY should preserve the immediate external base class");
            expect(native_object.class_library == library_path.string(),
                   "external identity ADDPROPERTY should preserve external class library provenance");
            expect(!native_object.properties.contains("class"),
                   "external identity ADDPROPERTY should not materialize a Class shadow property");
            expect(!native_object.properties.contains("baseclass"),
                   "external identity ADDPROPERTY should not materialize a BaseClass shadow property");
            expect(!native_object.properties.contains("parentclass"),
                   "external identity ADDPROPERTY should not materialize a ParentClass shadow property");
            expect(!native_object.properties.contains("classlibrary"),
                   "external identity ADDPROPERTY should not materialize a ClassLibrary shadow property");

            expect(state.ole_objects[1].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external identity ADDPROPERTY lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_same_prg_native_identity_metadata_cannot_be_shadowed_through_direct_assignment()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_identity_direct_assignment";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_identity_direct_assignment.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildWidget')\n"
            "oCreate.Class = 'OtherClass'\n"
            "oCreate.BaseClass = 'OtherBase'\n"
            "oCreate.ParentClass = 'OtherParent'\n"
            "oCreate.ClassLibrary = 'other.prg'\n"
            "cClassAfter = GETPEM(oCreate, 'Class')\n"
            "cBaseClassAfter = GETPEM(oCreate, 'BaseClass')\n"
            "cParentClassAfter = GETPEM(oCreate, 'ParentClass')\n"
            "xClassLibraryAfter = GETPEM(oCreate, 'ClassLibrary')\n"
            "RETURN\n"
            "DEFINE CLASS ParentWidget AS Custom\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ChildWidget AS ParentWidget\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native identity direct-assignment script should complete: ") + state.message +
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

        check("cclassafter", "ChildWidget");
        check("cbaseclassafter", "ParentWidget");
        check("cparentclassafter", "ParentWidget");

        const auto class_library_after = state.globals.find("xclasslibraryafter");
        expect(class_library_after != state.globals.end() &&
                   class_library_after->second.kind == copperfin::runtime::PrgValueKind::empty,
               "same-PRG identity direct assignment should leave ClassLibrary empty");

        expect(state.ole_objects.size() == 1U,
               "native identity direct assignment should register one native object");
        if (state.ole_objects.size() == 1U)
        {
            const auto &native_object = state.ole_objects[0];
            expect(native_object.prog_id == "ChildWidget",
                   "native identity direct assignment should preserve child class identity");
            expect(native_object.base_class_name == "ParentWidget",
                   "native identity direct assignment should preserve the immediate base class");
            expect(native_object.class_library.empty(),
                   "native identity direct assignment should preserve empty same-PRG class library provenance");
            expect(!native_object.properties.contains("class"),
                   "native identity direct assignment should not materialize a Class shadow property");
            expect(!native_object.properties.contains("baseclass"),
                   "native identity direct assignment should not materialize a BaseClass shadow property");
            expect(!native_object.properties.contains("parentclass"),
                   "native identity direct assignment should not materialize a ParentClass shadow property");
            expect(!native_object.properties.contains("classlibrary"),
                   "native identity direct assignment should not materialize a ClassLibrary shadow property");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_external_prg_identity_metadata_cannot_be_shadowed_through_direct_assignment()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_prg_identity_direct_assignment";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path library_path = temp_root / "widgetlib.prg";
        write_text(
            library_path,
            "DEFINE CLASS ParentWidget AS Custom\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_identity_direct_assignment.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildWidget')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 31)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "oCreate.Class = 'OtherClass'\n"
            "oCreate.BaseClass = 'OtherBase'\n"
            "oCreate.ParentClass = 'OtherParent'\n"
            "oCreate.ClassLibrary = 'other.prg'\n"
            "cClassAfter = GETPEM(oCreate, 'Class')\n"
            "cBaseClassAfter = GETPEM(oCreate, 'BaseClass')\n"
            "cParentClassAfter = GETPEM(oCreate, 'ParentClass')\n"
            "cClassLibraryAfter = GETPEM(oCreate, 'ClassLibrary')\n"
            "RETURN\n"
            "DEFINE CLASS ChildWidget AS ParentWidget OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external identity direct-assignment script should complete: ") + state.message +
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

        check("cclassafter", "ChildWidget");
        check("cbaseclassafter", "ParentWidget");
        check("cparentclassafter", "ParentWidget");
        check("cclasslibraryafter", library_path.string());
        check("ldictset", "true");
        check("ndictcompare", "31");

        expect(state.ole_objects.size() == 2U,
               "external identity direct assignment should register native and COM objects");
        if (state.ole_objects.size() == 2U)
        {
            const auto &native_object = state.ole_objects[0];
            expect(native_object.prog_id == "ChildWidget",
                   "external identity direct assignment should preserve child class identity");
            expect(native_object.base_class_name == "ParentWidget",
                   "external identity direct assignment should preserve the immediate external base class");
            expect(native_object.class_library == library_path.string(),
                   "external identity direct assignment should preserve external class library provenance");
            expect(!native_object.properties.contains("class"),
                   "external identity direct assignment should not materialize a Class shadow property");
            expect(!native_object.properties.contains("baseclass"),
                   "external identity direct assignment should not materialize a BaseClass shadow property");
            expect(!native_object.properties.contains("parentclass"),
                   "external identity direct assignment should not materialize a ParentClass shadow property");
            expect(!native_object.properties.contains("classlibrary"),
                   "external identity direct assignment should not materialize a ClassLibrary shadow property");

            expect(state.ole_objects[1].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external identity direct assignment lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_same_prg_native_identity_metadata_reads_through_ordinary_properties()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_identity_property_reads";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_identity_property_reads.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildWidget')\n"
            "cClass = oCreate.Class\n"
            "cBaseClass = oCreate.BaseClass\n"
            "RETURN\n"
            "DEFINE CLASS ParentWidget AS Custom\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ChildWidget AS ParentWidget\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native identity property-read script should complete: ") + state.message +
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

        check("cclass", "ChildWidget");
        check("cbaseclass", "ParentWidget");

        expect(state.ole_objects.size() == 1U,
               "native identity property reads should register one native object");
        if (state.ole_objects.size() == 1U)
        {
            expect(state.ole_objects[0].prog_id == "ChildWidget",
                   "native identity property reads should preserve child class identity");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_external_prg_identity_metadata_reads_through_ordinary_properties()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_prg_identity_property_reads";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path library_path = temp_root / "widgetlib.prg";
        write_text(
            library_path,
            "DEFINE CLASS ParentWidget AS Custom\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_identity_property_reads.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildWidget')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 32)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "cClass = oCreate.Class\n"
            "cBaseClass = oCreate.BaseClass\n"
            "cClassLibrary = oCreate.ClassLibrary\n"
            "RETURN\n"
            "DEFINE CLASS ChildWidget AS ParentWidget OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external identity property-read script should complete: ") + state.message +
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

        check("cclass", "ChildWidget");
        check("cbaseclass", "ParentWidget");
        check("cclasslibrary", library_path.string());
        check("ldictset", "true");
        check("ndictcompare", "32");

        expect(state.ole_objects.size() == 2U,
               "external identity property reads should register native and COM objects");
        if (state.ole_objects.size() == 2U)
        {
            expect(state.ole_objects[0].prog_id == "ChildWidget",
                   "external identity property reads should preserve child class identity");
            expect(state.ole_objects[1].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external identity property reads land");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_same_prg_native_parentclass_reads_through_ordinary_properties()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_parentclass_property_reads";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_parentclass_property_reads.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildWidget')\n"
            "cParentClass = oCreate.ParentClass\n"
            "RETURN\n"
            "DEFINE CLASS ParentWidget AS Custom\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ChildWidget AS ParentWidget\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native ParentClass property-read script should complete: ") + state.message +
                   " @line=" + std::to_string(state.location.line));

        const auto it = state.globals.find("cparentclass");
        if (it == state.globals.end())
        {
            expect(false, "cparentclass variable not found");
        }
        else
        {
            expect(copperfin::runtime::format_value(it->second) == "ParentWidget",
                   std::string("cparentclass expected 'ParentWidget' got '") +
                       copperfin::runtime::format_value(it->second) + "'");
        }

        expect(state.ole_objects.size() == 1U,
               "native ParentClass property reads should register one native object");
        if (state.ole_objects.size() == 1U)
        {
            expect(state.ole_objects[0].prog_id == "ChildWidget",
                   "native ParentClass property reads should preserve child class identity");
            expect(state.ole_objects[0].base_class_name == "ParentWidget",
                   "native ParentClass property reads should preserve the immediate parent class name");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_external_prg_parentclass_reads_through_ordinary_properties()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_prg_parentclass_property_reads";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path library_path = temp_root / "widgetlib.prg";
        write_text(
            library_path,
            "DEFINE CLASS ParentWidget AS Custom\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_parentclass_property_reads.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildWidget')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 34)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "cParentClass = oCreate.ParentClass\n"
            "RETURN\n"
            "DEFINE CLASS ChildWidget AS ParentWidget OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external ParentClass property-read script should complete: ") + state.message +
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

        check("cparentclass", "ParentWidget");
        check("ldictset", "true");
        check("ndictcompare", "34");

        expect(state.ole_objects.size() == 2U,
               "external ParentClass property reads should register native and COM objects");
        if (state.ole_objects.size() == 2U)
        {
            expect(state.ole_objects[0].prog_id == "ChildWidget",
                   "external ParentClass property reads should preserve child class identity");
            expect(state.ole_objects[0].base_class_name == "ParentWidget",
                   "external ParentClass property reads should preserve the immediate external parent class name");
            expect(state.ole_objects[1].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external ParentClass property reads land");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_same_prg_child_identity_metadata_reads_through_ordinary_properties()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_child_identity_property_reads";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_child_identity_property_reads.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('DemoForm')\n"
            "cChildClass = oCreate.cmdSave.Class\n"
            "cChildBaseClass = oCreate.cmdSave.BaseClass\n"
            "cChildParentClass = oCreate.cmdSave.ParentClass\n"
            "xChildClassLibrary = oCreate.cmdSave.ClassLibrary\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native child identity property-read script should complete: ") + state.message +
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

        check("cchildclass", "SaveButton");
        check("cchildbaseclass", "Custom");
        check("cchildparentclass", "Custom");

        const auto child_class_library = state.globals.find("xchildclasslibrary");
        expect(child_class_library != state.globals.end() &&
                   child_class_library->second.kind == copperfin::runtime::PrgValueKind::empty,
               "same-PRG child identity property reads should leave child ClassLibrary empty");

        expect(state.ole_objects.size() == 2U,
               "native child identity property reads should register parent and child objects");
        if (state.ole_objects.size() == 2U)
        {
            expect(state.ole_objects[0].prog_id == "DemoForm",
                   "native child identity property reads should preserve form identity");
            expect(state.ole_objects[1].prog_id == "SaveButton",
                   "native child identity property reads should preserve child class identity");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_external_base_child_identity_metadata_reads_through_ordinary_properties()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_prg_child_identity_property_reads";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path library_path = temp_root / "widgetlib.prg";
        write_text(
            library_path,
            "DEFINE CLASS ParentForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_child_identity_property_reads.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 51)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "cChildClass = oCreate.cmdSave.Class\n"
            "cChildBaseClass = oCreate.cmdSave.BaseClass\n"
            "cChildParentClass = oCreate.cmdSave.ParentClass\n"
            "xChildClassLibrary = oCreate.cmdSave.ClassLibrary\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external child identity property-read script should complete: ") + state.message +
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

        check("cchildclass", "SaveButton");
        check("cchildbaseclass", "Custom");
        check("cchildparentclass", "Custom");
        check("ldictset", "true");
        check("ndictcompare", "51");

        const auto child_class_library = state.globals.find("xchildclasslibrary");
        expect(child_class_library != state.globals.end() &&
                   child_class_library->second.kind == copperfin::runtime::PrgValueKind::empty,
               "external child identity property reads should leave child ClassLibrary empty when the child class itself has no external base");

        expect(state.ole_objects.size() == 3U,
               "external child identity property reads should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "ChildForm",
                   "external child identity property reads should preserve form identity");
            expect(state.ole_objects[1].prog_id == "SaveButton",
                   "external child identity property reads should preserve child class identity");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external child identity property reads land");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_same_prg_child_identity_metadata_stays_read_only_to_setpem()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_child_identity_setpem";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_child_identity_setpem.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('DemoForm')\n"
            "lChildClassReadOnly = PEMSTATUS(oCreate.cmdSave, 'Class', 5)\n"
            "lChildBaseClassReadOnly = PEMSTATUS(oCreate.cmdSave, 'BaseClass', 5)\n"
            "lChildParentClassReadOnly = PEMSTATUS(oCreate.cmdSave, 'ParentClass', 5)\n"
            "lSetChildClass = SETPEM(oCreate.cmdSave, 'Class', 'OtherClass')\n"
            "lSetChildBaseClass = SETPEM(oCreate.cmdSave, 'BaseClass', 'OtherBase')\n"
            "lSetChildParentClass = SETPEM(oCreate.cmdSave, 'ParentClass', 'OtherParent')\n"
            "lSetChildClassLibrary = SETPEM(oCreate.cmdSave, 'ClassLibrary', 'other.prg')\n"
            "cChildClassAfter = GETPEM(oCreate.cmdSave, 'Class')\n"
            "cChildBaseClassAfter = GETPEM(oCreate.cmdSave, 'BaseClass')\n"
            "cChildParentClassAfter = GETPEM(oCreate.cmdSave, 'ParentClass')\n"
            "xChildClassLibraryAfter = GETPEM(oCreate.cmdSave, 'ClassLibrary')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 52)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native child identity SETPEM script should complete: ") + state.message +
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

        check("lchildclassreadonly", "true");
        check("lchildbaseclassreadonly", "true");
        check("lchildparentclassreadonly", "true");
        check("lsetchildclass", "false");
        check("lsetchildbaseclass", "false");
        check("lsetchildparentclass", "false");
        check("lsetchildclasslibrary", "false");
        check("cchildclassafter", "SaveButton");
        check("cchildbaseclassafter", "Custom");
        check("cchildparentclassafter", "Custom");
        check("ldictset", "true");
        check("ndictcompare", "52");

        const auto child_class_library_after = state.globals.find("xchildclasslibraryafter");
        expect(child_class_library_after != state.globals.end() &&
                   child_class_library_after->second.kind == copperfin::runtime::PrgValueKind::empty,
               "same-PRG child identity SETPEM should leave child ClassLibrary empty after failed mutation");

        expect(state.ole_objects.size() == 3U,
               "native child identity SETPEM should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "DemoForm",
                   "native child identity SETPEM should preserve parent form identity");
            const auto &child_object = state.ole_objects[1];
            expect(child_object.prog_id == "SaveButton",
                   "native child identity SETPEM should preserve child class identity");
            expect(child_object.base_class_name == "Custom",
                   "native child identity SETPEM should preserve child base-class identity");
            expect(child_object.class_library.empty(),
                   "native child identity SETPEM should preserve empty same-PRG child class-library provenance");
            expect(!child_object.properties.contains("class"),
                   "native child identity SETPEM should not materialize a writable child Class shadow");
            expect(!child_object.properties.contains("baseclass"),
                   "native child identity SETPEM should not materialize a writable child BaseClass shadow");
            expect(!child_object.properties.contains("parentclass"),
                   "native child identity SETPEM should not materialize a writable child ParentClass shadow");
            expect(!child_object.properties.contains("classlibrary"),
                   "native child identity SETPEM should not materialize a writable child ClassLibrary shadow");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native child identity SETPEM lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_external_base_child_identity_metadata_stays_read_only_to_setpem()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_prg_child_identity_setpem";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path library_path = temp_root / "widgetlib.prg";
        write_text(
            library_path,
            "DEFINE CLASS ParentForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_child_identity_setpem.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "lChildClassReadOnly = PEMSTATUS(oCreate.cmdSave, 'Class', 5)\n"
            "lChildBaseClassReadOnly = PEMSTATUS(oCreate.cmdSave, 'BaseClass', 5)\n"
            "lChildParentClassReadOnly = PEMSTATUS(oCreate.cmdSave, 'ParentClass', 5)\n"
            "lSetChildClass = SETPEM(oCreate.cmdSave, 'Class', 'OtherClass')\n"
            "lSetChildBaseClass = SETPEM(oCreate.cmdSave, 'BaseClass', 'OtherBase')\n"
            "lSetChildParentClass = SETPEM(oCreate.cmdSave, 'ParentClass', 'OtherParent')\n"
            "lSetChildClassLibrary = SETPEM(oCreate.cmdSave, 'ClassLibrary', 'other.prg')\n"
            "cChildClassAfter = GETPEM(oCreate.cmdSave, 'Class')\n"
            "cChildBaseClassAfter = GETPEM(oCreate.cmdSave, 'BaseClass')\n"
            "cChildParentClassAfter = GETPEM(oCreate.cmdSave, 'ParentClass')\n"
            "xChildClassLibraryAfter = GETPEM(oCreate.cmdSave, 'ClassLibrary')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 53)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external child identity SETPEM script should complete: ") + state.message +
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

        check("lchildclassreadonly", "true");
        check("lchildbaseclassreadonly", "true");
        check("lchildparentclassreadonly", "true");
        check("lsetchildclass", "false");
        check("lsetchildbaseclass", "false");
        check("lsetchildparentclass", "false");
        check("lsetchildclasslibrary", "false");
        check("cchildclassafter", "SaveButton");
        check("cchildbaseclassafter", "Custom");
        check("cchildparentclassafter", "Custom");
        check("ldictset", "true");
        check("ndictcompare", "53");

        const auto child_class_library_after = state.globals.find("xchildclasslibraryafter");
        expect(child_class_library_after != state.globals.end() &&
                   child_class_library_after->second.kind == copperfin::runtime::PrgValueKind::empty,
               "external child identity SETPEM should leave child ClassLibrary empty when the child class itself has no external base");

        expect(state.ole_objects.size() == 3U,
               "external child identity SETPEM should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "ChildForm",
                   "external child identity SETPEM should preserve parent form identity");
            const auto &child_object = state.ole_objects[1];
            expect(child_object.prog_id == "SaveButton",
                   "external child identity SETPEM should preserve child class identity");
            expect(child_object.base_class_name == "Custom",
                   "external child identity SETPEM should preserve child base-class identity");
            expect(child_object.class_library.empty(),
                   "external child identity SETPEM should keep child class-library provenance empty when the child class itself has no external base");
            expect(child_object.source == library_path.string(),
                   "external child identity SETPEM should preserve the defining PRG path as child provenance");
            expect(!child_object.properties.contains("class"),
                   "external child identity SETPEM should not materialize a writable child Class shadow");
            expect(!child_object.properties.contains("baseclass"),
                   "external child identity SETPEM should not materialize a writable child BaseClass shadow");
            expect(!child_object.properties.contains("parentclass"),
                   "external child identity SETPEM should not materialize a writable child ParentClass shadow");
            expect(!child_object.properties.contains("classlibrary"),
                   "external child identity SETPEM should not materialize a writable child ClassLibrary shadow");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external child identity SETPEM lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_same_prg_child_identity_metadata_cannot_be_shadowed_through_addproperty()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_child_identity_addproperty";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_child_identity_addproperty.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('DemoForm')\n"
            "lAddChildClass = ADDPROPERTY(oCreate.cmdSave, 'Class', 'OtherClass')\n"
            "lAddChildBaseClass = ADDPROPERTY(oCreate.cmdSave, 'BaseClass', 'OtherBase')\n"
            "lAddChildParentClass = ADDPROPERTY(oCreate.cmdSave, 'ParentClass', 'OtherParent')\n"
            "lAddChildClassLibrary = ADDPROPERTY(oCreate.cmdSave, 'ClassLibrary', 'other.prg')\n"
            "cChildClassAfter = GETPEM(oCreate.cmdSave, 'Class')\n"
            "cChildBaseClassAfter = GETPEM(oCreate.cmdSave, 'BaseClass')\n"
            "cChildParentClassAfter = GETPEM(oCreate.cmdSave, 'ParentClass')\n"
            "xChildClassLibraryAfter = GETPEM(oCreate.cmdSave, 'ClassLibrary')\n"
            "lChildClassReadOnly = PEMSTATUS(oCreate.cmdSave, 'Class', 5)\n"
            "lChildBaseClassReadOnly = PEMSTATUS(oCreate.cmdSave, 'BaseClass', 5)\n"
            "lChildParentClassReadOnly = PEMSTATUS(oCreate.cmdSave, 'ParentClass', 5)\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 54)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native child identity ADDPROPERTY script should complete: ") + state.message +
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

        check("laddchildclass", "false");
        check("laddchildbaseclass", "false");
        check("laddchildparentclass", "false");
        check("laddchildclasslibrary", "false");
        check("cchildclassafter", "SaveButton");
        check("cchildbaseclassafter", "Custom");
        check("cchildparentclassafter", "Custom");
        check("lchildclassreadonly", "true");
        check("lchildbaseclassreadonly", "true");
        check("lchildparentclassreadonly", "true");
        check("ldictset", "true");
        check("ndictcompare", "54");

        const auto child_class_library_after = state.globals.find("xchildclasslibraryafter");
        expect(child_class_library_after != state.globals.end() &&
                   child_class_library_after->second.kind == copperfin::runtime::PrgValueKind::empty,
               "same-PRG child identity ADDPROPERTY should leave child ClassLibrary empty after failed shadow creation");

        expect(state.ole_objects.size() == 3U,
               "native child identity ADDPROPERTY should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "DemoForm",
                   "native child identity ADDPROPERTY should preserve parent form identity");
            const auto &child_object = state.ole_objects[1];
            expect(child_object.prog_id == "SaveButton",
                   "native child identity ADDPROPERTY should preserve child class identity");
            expect(child_object.base_class_name == "Custom",
                   "native child identity ADDPROPERTY should preserve child base-class identity");
            expect(child_object.class_library.empty(),
                   "native child identity ADDPROPERTY should preserve empty same-PRG child class-library provenance");
            expect(!child_object.properties.contains("class"),
                   "native child identity ADDPROPERTY should not materialize a child Class shadow");
            expect(!child_object.properties.contains("baseclass"),
                   "native child identity ADDPROPERTY should not materialize a child BaseClass shadow");
            expect(!child_object.properties.contains("parentclass"),
                   "native child identity ADDPROPERTY should not materialize a child ParentClass shadow");
            expect(!child_object.properties.contains("classlibrary"),
                   "native child identity ADDPROPERTY should not materialize a child ClassLibrary shadow");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native child identity ADDPROPERTY lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_external_base_child_identity_metadata_cannot_be_shadowed_through_addproperty()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_prg_child_identity_addproperty";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path library_path = temp_root / "widgetlib.prg";
        write_text(
            library_path,
            "DEFINE CLASS ParentForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_child_identity_addproperty.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "lAddChildClass = ADDPROPERTY(oCreate.cmdSave, 'Class', 'OtherClass')\n"
            "lAddChildBaseClass = ADDPROPERTY(oCreate.cmdSave, 'BaseClass', 'OtherBase')\n"
            "lAddChildParentClass = ADDPROPERTY(oCreate.cmdSave, 'ParentClass', 'OtherParent')\n"
            "lAddChildClassLibrary = ADDPROPERTY(oCreate.cmdSave, 'ClassLibrary', 'other.prg')\n"
            "cChildClassAfter = GETPEM(oCreate.cmdSave, 'Class')\n"
            "cChildBaseClassAfter = GETPEM(oCreate.cmdSave, 'BaseClass')\n"
            "cChildParentClassAfter = GETPEM(oCreate.cmdSave, 'ParentClass')\n"
            "xChildClassLibraryAfter = GETPEM(oCreate.cmdSave, 'ClassLibrary')\n"
            "lChildClassReadOnly = PEMSTATUS(oCreate.cmdSave, 'Class', 5)\n"
            "lChildBaseClassReadOnly = PEMSTATUS(oCreate.cmdSave, 'BaseClass', 5)\n"
            "lChildParentClassReadOnly = PEMSTATUS(oCreate.cmdSave, 'ParentClass', 5)\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 55)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external child identity ADDPROPERTY script should complete: ") + state.message +
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

        check("laddchildclass", "false");
        check("laddchildbaseclass", "false");
        check("laddchildparentclass", "false");
        check("laddchildclasslibrary", "false");
        check("cchildclassafter", "SaveButton");
        check("cchildbaseclassafter", "Custom");
        check("cchildparentclassafter", "Custom");
        check("lchildclassreadonly", "true");
        check("lchildbaseclassreadonly", "true");
        check("lchildparentclassreadonly", "true");
        check("ldictset", "true");
        check("ndictcompare", "55");

        const auto child_class_library_after = state.globals.find("xchildclasslibraryafter");
        expect(child_class_library_after != state.globals.end() &&
                   child_class_library_after->second.kind == copperfin::runtime::PrgValueKind::empty,
               "external child identity ADDPROPERTY should leave child ClassLibrary empty when the child class itself has no external base");

        expect(state.ole_objects.size() == 3U,
               "external child identity ADDPROPERTY should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "ChildForm",
                   "external child identity ADDPROPERTY should preserve parent form identity");
            const auto &child_object = state.ole_objects[1];
            expect(child_object.prog_id == "SaveButton",
                   "external child identity ADDPROPERTY should preserve child class identity");
            expect(child_object.base_class_name == "Custom",
                   "external child identity ADDPROPERTY should preserve child base-class identity");
            expect(child_object.class_library.empty(),
                   "external child identity ADDPROPERTY should keep child class-library provenance empty when the child class itself has no external base");
            expect(child_object.source == library_path.string(),
                   "external child identity ADDPROPERTY should preserve the defining PRG path as child provenance");
            expect(!child_object.properties.contains("class"),
                   "external child identity ADDPROPERTY should not materialize a child Class shadow");
            expect(!child_object.properties.contains("baseclass"),
                   "external child identity ADDPROPERTY should not materialize a child BaseClass shadow");
            expect(!child_object.properties.contains("parentclass"),
                   "external child identity ADDPROPERTY should not materialize a child ParentClass shadow");
            expect(!child_object.properties.contains("classlibrary"),
                   "external child identity ADDPROPERTY should not materialize a child ClassLibrary shadow");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external child identity ADDPROPERTY lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_same_prg_child_identity_metadata_stays_protected_from_removeproperty()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_child_identity_removeproperty";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_child_identity_removeproperty.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('DemoForm')\n"
            "lRemoveChildClass = REMOVEPROPERTY(oCreate.cmdSave, 'Class')\n"
            "lRemoveChildBaseClass = REMOVEPROPERTY(oCreate.cmdSave, 'BaseClass')\n"
            "lRemoveChildParentClass = REMOVEPROPERTY(oCreate.cmdSave, 'ParentClass')\n"
            "lRemoveChildClassLibrary = REMOVEPROPERTY(oCreate.cmdSave, 'ClassLibrary')\n"
            "cChildClassAfter = GETPEM(oCreate.cmdSave, 'Class')\n"
            "cChildBaseClassAfter = GETPEM(oCreate.cmdSave, 'BaseClass')\n"
            "cChildParentClassAfter = GETPEM(oCreate.cmdSave, 'ParentClass')\n"
            "xChildClassLibraryAfter = GETPEM(oCreate.cmdSave, 'ClassLibrary')\n"
            "lChildClassReadOnly = PEMSTATUS(oCreate.cmdSave, 'Class', 5)\n"
            "lChildBaseClassReadOnly = PEMSTATUS(oCreate.cmdSave, 'BaseClass', 5)\n"
            "lChildParentClassReadOnly = PEMSTATUS(oCreate.cmdSave, 'ParentClass', 5)\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 56)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native child identity REMOVEPROPERTY script should complete: ") + state.message +
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

        check("lremovechildclass", "false");
        check("lremovechildbaseclass", "false");
        check("lremovechildparentclass", "false");
        check("lremovechildclasslibrary", "false");
        check("cchildclassafter", "SaveButton");
        check("cchildbaseclassafter", "Custom");
        check("cchildparentclassafter", "Custom");
        check("lchildclassreadonly", "true");
        check("lchildbaseclassreadonly", "true");
        check("lchildparentclassreadonly", "true");
        check("ldictset", "true");
        check("ndictcompare", "56");

        const auto child_class_library_after = state.globals.find("xchildclasslibraryafter");
        expect(child_class_library_after != state.globals.end() &&
                   child_class_library_after->second.kind == copperfin::runtime::PrgValueKind::empty,
               "same-PRG child identity REMOVEPROPERTY should leave child ClassLibrary empty after failed removal");

        expect(state.ole_objects.size() == 3U,
               "native child identity REMOVEPROPERTY should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "DemoForm",
                   "native child identity REMOVEPROPERTY should preserve parent form identity");
            const auto &child_object = state.ole_objects[1];
            expect(child_object.prog_id == "SaveButton",
                   "native child identity REMOVEPROPERTY should preserve child class identity");
            expect(child_object.base_class_name == "Custom",
                   "native child identity REMOVEPROPERTY should preserve child base-class identity");
            expect(child_object.class_library.empty(),
                   "native child identity REMOVEPROPERTY should preserve empty same-PRG child class-library provenance");
            expect(!child_object.properties.contains("class"),
                   "native child identity REMOVEPROPERTY should not materialize a child Class shadow");
            expect(!child_object.properties.contains("baseclass"),
                   "native child identity REMOVEPROPERTY should not materialize a child BaseClass shadow");
            expect(!child_object.properties.contains("parentclass"),
                   "native child identity REMOVEPROPERTY should not materialize a child ParentClass shadow");
            expect(!child_object.properties.contains("classlibrary"),
                   "native child identity REMOVEPROPERTY should not materialize a child ClassLibrary shadow");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native child identity REMOVEPROPERTY lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_external_base_child_identity_metadata_stays_protected_from_removeproperty()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_prg_child_identity_removeproperty";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path library_path = temp_root / "widgetlib.prg";
        write_text(
            library_path,
            "DEFINE CLASS ParentForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_child_identity_removeproperty.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "lRemoveChildClass = REMOVEPROPERTY(oCreate.cmdSave, 'Class')\n"
            "lRemoveChildBaseClass = REMOVEPROPERTY(oCreate.cmdSave, 'BaseClass')\n"
            "lRemoveChildParentClass = REMOVEPROPERTY(oCreate.cmdSave, 'ParentClass')\n"
            "lRemoveChildClassLibrary = REMOVEPROPERTY(oCreate.cmdSave, 'ClassLibrary')\n"
            "cChildClassAfter = GETPEM(oCreate.cmdSave, 'Class')\n"
            "cChildBaseClassAfter = GETPEM(oCreate.cmdSave, 'BaseClass')\n"
            "cChildParentClassAfter = GETPEM(oCreate.cmdSave, 'ParentClass')\n"
            "xChildClassLibraryAfter = GETPEM(oCreate.cmdSave, 'ClassLibrary')\n"
            "lChildClassReadOnly = PEMSTATUS(oCreate.cmdSave, 'Class', 5)\n"
            "lChildBaseClassReadOnly = PEMSTATUS(oCreate.cmdSave, 'BaseClass', 5)\n"
            "lChildParentClassReadOnly = PEMSTATUS(oCreate.cmdSave, 'ParentClass', 5)\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 57)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external child identity REMOVEPROPERTY script should complete: ") + state.message +
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

        check("lremovechildclass", "false");
        check("lremovechildbaseclass", "false");
        check("lremovechildparentclass", "false");
        check("lremovechildclasslibrary", "false");
        check("cchildclassafter", "SaveButton");
        check("cchildbaseclassafter", "Custom");
        check("cchildparentclassafter", "Custom");
        check("lchildclassreadonly", "true");
        check("lchildbaseclassreadonly", "true");
        check("lchildparentclassreadonly", "true");
        check("ldictset", "true");
        check("ndictcompare", "57");

        const auto child_class_library_after = state.globals.find("xchildclasslibraryafter");
        expect(child_class_library_after != state.globals.end() &&
                   child_class_library_after->second.kind == copperfin::runtime::PrgValueKind::empty,
               "external child identity REMOVEPROPERTY should leave child ClassLibrary empty when the child class itself has no external base");

        expect(state.ole_objects.size() == 3U,
               "external child identity REMOVEPROPERTY should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "ChildForm",
                   "external child identity REMOVEPROPERTY should preserve parent form identity");
            const auto &child_object = state.ole_objects[1];
            expect(child_object.prog_id == "SaveButton",
                   "external child identity REMOVEPROPERTY should preserve child class identity");
            expect(child_object.base_class_name == "Custom",
                   "external child identity REMOVEPROPERTY should preserve child base-class identity");
            expect(child_object.class_library.empty(),
                   "external child identity REMOVEPROPERTY should keep child class-library provenance empty when the child class itself has no external base");
            expect(child_object.source == library_path.string(),
                   "external child identity REMOVEPROPERTY should preserve the defining PRG path as child provenance");
            expect(!child_object.properties.contains("class"),
                   "external child identity REMOVEPROPERTY should not materialize a child Class shadow");
            expect(!child_object.properties.contains("baseclass"),
                   "external child identity REMOVEPROPERTY should not materialize a child BaseClass shadow");
            expect(!child_object.properties.contains("parentclass"),
                   "external child identity REMOVEPROPERTY should not materialize a child ParentClass shadow");
            expect(!child_object.properties.contains("classlibrary"),
                   "external child identity REMOVEPROPERTY should not materialize a child ClassLibrary shadow");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external child identity REMOVEPROPERTY lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_same_prg_child_identity_metadata_cannot_be_shadowed_through_direct_assignment()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_child_identity_direct_assignment";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_child_identity_direct_assignment.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('DemoForm')\n"
            "oCreate.cmdSave.Class = 'OtherClass'\n"
            "oCreate.cmdSave.BaseClass = 'OtherBase'\n"
            "oCreate.cmdSave.ParentClass = 'OtherParent'\n"
            "oCreate.cmdSave.ClassLibrary = 'other.prg'\n"
            "cChildClassAfter = GETPEM(oCreate.cmdSave, 'Class')\n"
            "cChildBaseClassAfter = GETPEM(oCreate.cmdSave, 'BaseClass')\n"
            "cChildParentClassAfter = GETPEM(oCreate.cmdSave, 'ParentClass')\n"
            "xChildClassLibraryAfter = GETPEM(oCreate.cmdSave, 'ClassLibrary')\n"
            "lChildClassReadOnly = PEMSTATUS(oCreate.cmdSave, 'Class', 5)\n"
            "lChildBaseClassReadOnly = PEMSTATUS(oCreate.cmdSave, 'BaseClass', 5)\n"
            "lChildParentClassReadOnly = PEMSTATUS(oCreate.cmdSave, 'ParentClass', 5)\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 58)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native child identity direct-assignment script should complete: ") + state.message +
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

        check("cchildclassafter", "SaveButton");
        check("cchildbaseclassafter", "Custom");
        check("cchildparentclassafter", "Custom");
        check("lchildclassreadonly", "true");
        check("lchildbaseclassreadonly", "true");
        check("lchildparentclassreadonly", "true");
        check("ldictset", "true");
        check("ndictcompare", "58");

        const auto child_class_library_after = state.globals.find("xchildclasslibraryafter");
        expect(child_class_library_after != state.globals.end() &&
                   child_class_library_after->second.kind == copperfin::runtime::PrgValueKind::empty,
               "same-PRG child identity direct assignment should leave child ClassLibrary empty");

        expect(state.ole_objects.size() == 3U,
               "native child identity direct assignment should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "DemoForm",
                   "native child identity direct assignment should preserve parent form identity");
            const auto &child_object = state.ole_objects[1];
            expect(child_object.prog_id == "SaveButton",
                   "native child identity direct assignment should preserve child class identity");
            expect(child_object.base_class_name == "Custom",
                   "native child identity direct assignment should preserve child base-class identity");
            expect(child_object.class_library.empty(),
                   "native child identity direct assignment should preserve empty same-PRG child class-library provenance");
            expect(!child_object.properties.contains("class"),
                   "native child identity direct assignment should not materialize a child Class shadow property");
            expect(!child_object.properties.contains("baseclass"),
                   "native child identity direct assignment should not materialize a child BaseClass shadow property");
            expect(!child_object.properties.contains("parentclass"),
                   "native child identity direct assignment should not materialize a child ParentClass shadow property");
            expect(!child_object.properties.contains("classlibrary"),
                   "native child identity direct assignment should not materialize a child ClassLibrary shadow property");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native child identity direct assignment lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_external_base_child_identity_metadata_cannot_be_shadowed_through_direct_assignment()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_prg_child_identity_direct_assignment";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path library_path = temp_root / "widgetlib.prg";
        write_text(
            library_path,
            "DEFINE CLASS ParentForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_child_identity_direct_assignment.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 59)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "oCreate.cmdSave.Class = 'OtherClass'\n"
            "oCreate.cmdSave.BaseClass = 'OtherBase'\n"
            "oCreate.cmdSave.ParentClass = 'OtherParent'\n"
            "oCreate.cmdSave.ClassLibrary = 'other.prg'\n"
            "cChildClassAfter = GETPEM(oCreate.cmdSave, 'Class')\n"
            "cChildBaseClassAfter = GETPEM(oCreate.cmdSave, 'BaseClass')\n"
            "cChildParentClassAfter = GETPEM(oCreate.cmdSave, 'ParentClass')\n"
            "xChildClassLibraryAfter = GETPEM(oCreate.cmdSave, 'ClassLibrary')\n"
            "lChildClassReadOnly = PEMSTATUS(oCreate.cmdSave, 'Class', 5)\n"
            "lChildBaseClassReadOnly = PEMSTATUS(oCreate.cmdSave, 'BaseClass', 5)\n"
            "lChildParentClassReadOnly = PEMSTATUS(oCreate.cmdSave, 'ParentClass', 5)\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external child identity direct-assignment script should complete: ") + state.message +
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

        check("cchildclassafter", "SaveButton");
        check("cchildbaseclassafter", "Custom");
        check("cchildparentclassafter", "Custom");
        check("lchildclassreadonly", "true");
        check("lchildbaseclassreadonly", "true");
        check("lchildparentclassreadonly", "true");
        check("ldictset", "true");
        check("ndictcompare", "59");

        const auto child_class_library_after = state.globals.find("xchildclasslibraryafter");
        expect(child_class_library_after != state.globals.end() &&
                   child_class_library_after->second.kind == copperfin::runtime::PrgValueKind::empty,
               "external child identity direct assignment should leave child ClassLibrary empty when the child class itself has no external base");

        expect(state.ole_objects.size() == 3U,
               "external child identity direct assignment should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "ChildForm",
                   "external child identity direct assignment should preserve parent form identity");
            const auto &child_object = state.ole_objects[1];
            expect(child_object.prog_id == "SaveButton",
                   "external child identity direct assignment should preserve child class identity");
            expect(child_object.base_class_name == "Custom",
                   "external child identity direct assignment should preserve child base-class identity");
            expect(child_object.class_library.empty(),
                   "external child identity direct assignment should keep child class-library provenance empty when the child class itself has no external base");
            expect(child_object.source == library_path.string(),
                   "external child identity direct assignment should preserve the defining PRG path as child provenance");
            expect(!child_object.properties.contains("class"),
                   "external child identity direct assignment should not materialize a child Class shadow property");
            expect(!child_object.properties.contains("baseclass"),
                   "external child identity direct assignment should not materialize a child BaseClass shadow property");
            expect(!child_object.properties.contains("parentclass"),
                   "external child identity direct assignment should not materialize a child ParentClass shadow property");
            expect(!child_object.properties.contains("classlibrary"),
                   "external child identity direct assignment should not materialize a child ClassLibrary shadow property");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external child identity direct assignment lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_same_prg_child_parent_reflects_through_getpem_and_pemstatus()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_child_parent_reflection";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_child_parent_reflection.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('DemoForm')\n"
            "oParentRef = GETPEM(oCreate.cmdSave, 'Parent')\n"
            "lChildHasParent = PEMSTATUS(oCreate.cmdSave, 'Parent', 1)\n"
            "cParentCaption = oParentRef.Caption\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 35)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native child Parent reflection script should complete: ") + state.message +
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

        check("lchildhasparent", "true");
        check("cparentcaption", "MainForm");
        check("ldictset", "true");
        check("ndictcompare", "35");

        expect(state.ole_objects.size() == 3U,
               "native child Parent reflection should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "DemoForm",
                   "native child Parent reflection should preserve form identity");
            expect(state.ole_objects[1].prog_id == "SaveButton",
                   "native child Parent reflection should preserve child identity");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native child Parent reflection lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_external_base_child_parent_reflects_through_getpem_and_pemstatus()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_prg_child_parent_reflection";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path library_path = temp_root / "widgetlib.prg";
        write_text(
            library_path,
            "DEFINE CLASS ParentForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_child_parent_reflection.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oParentRef = GETPEM(oCreate.cmdSave, 'Parent')\n"
            "lChildHasParent = PEMSTATUS(oCreate.cmdSave, 'Parent', 1)\n"
            "cParentCaption = oParentRef.Caption\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 36)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external child Parent reflection script should complete: ") + state.message +
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

        check("lchildhasparent", "true");
        check("cparentcaption", "MainForm");
        check("ldictset", "true");
        check("ndictcompare", "36");

        expect(state.ole_objects.size() == 3U,
               "external child Parent reflection should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "ChildForm",
                   "external child Parent reflection should preserve form identity");
            expect(state.ole_objects[1].prog_id == "SaveButton",
                   "external child Parent reflection should preserve child identity");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external child Parent reflection lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_same_prg_child_identity_reflects_through_getpem_and_pemstatus()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_child_identity_reflection";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_child_identity_reflection.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('DemoForm')\n"
            "cChildClass = GETPEM(oCreate.cmdSave, 'Class')\n"
            "cChildBaseClass = GETPEM(oCreate.cmdSave, 'BaseClass')\n"
            "cChildParentClass = GETPEM(oCreate.cmdSave, 'ParentClass')\n"
            "xChildClassLibrary = GETPEM(oCreate.cmdSave, 'ClassLibrary')\n"
            "lChildHasClass = PEMSTATUS(oCreate.cmdSave, 'Class', 1)\n"
            "lChildHasBaseClass = PEMSTATUS(oCreate.cmdSave, 'BaseClass', 1)\n"
            "lChildHasParentClass = PEMSTATUS(oCreate.cmdSave, 'ParentClass', 1)\n"
            "lChildHasClassLibrary = PEMSTATUS(oCreate.cmdSave, 'ClassLibrary', 1)\n"
            "lChildClassReadOnly = PEMSTATUS(oCreate.cmdSave, 'Class', 5)\n"
            "lChildBaseClassReadOnly = PEMSTATUS(oCreate.cmdSave, 'BaseClass', 5)\n"
            "lChildParentClassReadOnly = PEMSTATUS(oCreate.cmdSave, 'ParentClass', 5)\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 47)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native child identity reflection script should complete: ") + state.message +
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

        check("cchildclass", "SaveButton");
        check("cchildbaseclass", "Custom");
        check("cchildparentclass", "Custom");
        check("lchildhasclass", "true");
        check("lchildhasbaseclass", "true");
        check("lchildhasparentclass", "true");
        check("lchildhasclasslibrary", "false");
        check("lchildclassreadonly", "true");
        check("lchildbaseclassreadonly", "true");
        check("lchildparentclassreadonly", "true");
        check("ldictset", "true");
        check("ndictcompare", "47");

        const auto child_class_library = state.globals.find("xchildclasslibrary");
        expect(child_class_library != state.globals.end() &&
                   child_class_library->second.kind == copperfin::runtime::PrgValueKind::empty,
               "same-PRG child identity reflection should leave child ClassLibrary empty");

        expect(state.ole_objects.size() == 3U,
               "native child identity reflection should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &child_object = state.ole_objects[1];
            expect(child_object.prog_id == "SaveButton",
                   "native child identity reflection should preserve child class identity");
            expect(child_object.base_class_name == "Custom",
                   "native child identity reflection should preserve child base-class identity");
            expect(child_object.class_library.empty(),
                   "native child identity reflection should keep same-PRG child class-library provenance empty");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native child identity reflection lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_external_base_child_identity_reflects_through_getpem_and_pemstatus()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_prg_child_identity_reflection";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path library_path = temp_root / "widgetlib.prg";
        write_text(
            library_path,
            "DEFINE CLASS ParentForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_child_identity_reflection.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 48)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "cChildClass = GETPEM(oCreate.cmdSave, 'Class')\n"
            "cChildBaseClass = GETPEM(oCreate.cmdSave, 'BaseClass')\n"
            "cChildParentClass = GETPEM(oCreate.cmdSave, 'ParentClass')\n"
            "xChildClassLibrary = GETPEM(oCreate.cmdSave, 'ClassLibrary')\n"
            "lChildHasClass = PEMSTATUS(oCreate.cmdSave, 'Class', 1)\n"
            "lChildHasBaseClass = PEMSTATUS(oCreate.cmdSave, 'BaseClass', 1)\n"
            "lChildHasParentClass = PEMSTATUS(oCreate.cmdSave, 'ParentClass', 1)\n"
            "lChildHasClassLibrary = PEMSTATUS(oCreate.cmdSave, 'ClassLibrary', 1)\n"
            "lChildClassReadOnly = PEMSTATUS(oCreate.cmdSave, 'Class', 5)\n"
            "lChildBaseClassReadOnly = PEMSTATUS(oCreate.cmdSave, 'BaseClass', 5)\n"
            "lChildParentClassReadOnly = PEMSTATUS(oCreate.cmdSave, 'ParentClass', 5)\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external child identity reflection script should complete: ") + state.message +
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

        check("cchildclass", "SaveButton");
        check("cchildbaseclass", "Custom");
        check("cchildparentclass", "Custom");
        check("lchildhasclass", "true");
        check("lchildhasbaseclass", "true");
        check("lchildhasparentclass", "true");
        check("lchildhasclasslibrary", "false");
        check("lchildclassreadonly", "true");
        check("lchildbaseclassreadonly", "true");
        check("lchildparentclassreadonly", "true");
        check("ldictset", "true");
        check("ndictcompare", "48");

        const auto child_class_library = state.globals.find("xchildclasslibrary");
        expect(child_class_library != state.globals.end() &&
                   child_class_library->second.kind == copperfin::runtime::PrgValueKind::empty,
               "external-base child identity reflection should leave child ClassLibrary empty when the child class itself has no external base");

        expect(state.ole_objects.size() == 3U,
               "external child identity reflection should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &child_object = state.ole_objects[1];
            expect(child_object.prog_id == "SaveButton",
                   "external child identity reflection should preserve child class identity");
            expect(child_object.base_class_name == "Custom",
                   "external child identity reflection should preserve child base-class identity");
            expect(child_object.class_library.empty(),
                   "external child identity reflection should keep child class-library provenance empty when the child class itself has no external base");
            expect(child_object.source == library_path.string(),
                   "external child identity reflection should preserve the defining PRG path as child provenance");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external child identity reflection lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_same_prg_child_external_base_provenance_surfaces_through_identity_metadata()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_child_external_base_provenance";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path button_library_path = temp_root / "buttons.prg";
        write_text(
            button_library_path,
            "DEFINE CLASS ParentButton AS Custom\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "native_child_external_base_provenance.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('DemoForm')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 64)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "cChildClass = GETPEM(oCreate.cmdSave, 'Class')\n"
            "cChildBaseClass = GETPEM(oCreate.cmdSave, 'BaseClass')\n"
            "cChildParentClass = GETPEM(oCreate.cmdSave, 'ParentClass')\n"
            "cChildClassLibrary = GETPEM(oCreate.cmdSave, 'ClassLibrary')\n"
            "lChildHasClassLibrary = PEMSTATUS(oCreate.cmdSave, 'ClassLibrary', 1)\n"
            "lChildClassLibraryReadOnly = PEMSTATUS(oCreate.cmdSave, 'ClassLibrary', 5)\n"
            "cChildClassLibraryProp = oCreate.cmdSave.ClassLibrary\n"
            "nMembersProps = AMEMBERS(aMembersProps, oCreate.cmdSave, 1)\n"
            "nMembersUnion = AMEMBERS(aMembersUnion, oCreate.cmdSave, 3)\n"
            "cProp1 = aMembersProps[1]\n"
            "cProp2 = aMembersProps[2]\n"
            "cProp3 = aMembersProps[3]\n"
            "cProp4 = aMembersProps[4]\n"
            "cProp5 = aMembersProps[5]\n"
            "cUnion3 = aMembersUnion[3]\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS ParentButton OF buttons.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native child external-base provenance script should complete: ") + state.message +
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

        check("cchildclass", "SaveButton");
        check("cchildbaseclass", "ParentButton");
        check("cchildparentclass", "ParentButton");
        check("cchildclasslibrary", button_library_path.string());
        check("lchildhasclasslibrary", "true");
        check("lchildclasslibraryreadonly", "true");
        check("cchildclasslibraryprop", button_library_path.string());
        check("nmembersprops", "5");
        check("nmembersunion", "5");
        check("cprop1", "BASECLASS");
        check("cprop2", "CLASS");
        check("cprop3", "CLASSLIBRARY");
        check("cprop4", "PARENT");
        check("cprop5", "PARENTCLASS");
        check("cunion3", "CLASSLIBRARY");
        check("ldictset", "true");
        check("ndictcompare", "64");

        expect(state.ole_objects.size() == 3U,
               "native child external-base provenance should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "DemoForm",
                   "native child external-base provenance should preserve form identity");
            const auto &child_object = state.ole_objects[1];
            expect(child_object.prog_id == "SaveButton",
                   "native child external-base provenance should preserve child class identity");
            expect(child_object.base_class_name == "ParentButton",
                   "native child external-base provenance should preserve child base-class identity");
            expect(child_object.class_library == button_library_path.string(),
                   "native child external-base provenance should preserve the child external class-library path");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native child external-base provenance lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_external_base_child_external_base_provenance_surfaces_through_identity_metadata()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_prg_child_external_base_provenance";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path button_library_path = temp_root / "buttons.prg";
        write_text(
            button_library_path,
            "DEFINE CLASS ParentButton AS Custom\n"
            "ENDDEFINE\n");

        const fs::path form_library_path = temp_root / "widgetlib.prg";
        write_text(
            form_library_path,
            "DEFINE CLASS ParentForm AS Custom\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS ParentButton OF buttons.prg\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_child_external_base_provenance.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 65)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "cChildClass = GETPEM(oCreate.cmdSave, 'Class')\n"
            "cChildBaseClass = GETPEM(oCreate.cmdSave, 'BaseClass')\n"
            "cChildParentClass = GETPEM(oCreate.cmdSave, 'ParentClass')\n"
            "cChildClassLibrary = GETPEM(oCreate.cmdSave, 'ClassLibrary')\n"
            "lChildHasClassLibrary = PEMSTATUS(oCreate.cmdSave, 'ClassLibrary', 1)\n"
            "lChildClassLibraryReadOnly = PEMSTATUS(oCreate.cmdSave, 'ClassLibrary', 5)\n"
            "cChildClassLibraryProp = oCreate.cmdSave.ClassLibrary\n"
            "nMembersProps = AMEMBERS(aMembersProps, oCreate.cmdSave, 1)\n"
            "nMembersUnion = AMEMBERS(aMembersUnion, oCreate.cmdSave, 3)\n"
            "cProp1 = aMembersProps[1]\n"
            "cProp2 = aMembersProps[2]\n"
            "cProp3 = aMembersProps[3]\n"
            "cProp4 = aMembersProps[4]\n"
            "cProp5 = aMembersProps[5]\n"
            "cUnion3 = aMembersUnion[3]\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external child external-base provenance script should complete: ") + state.message +
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

        check("cchildclass", "SaveButton");
        check("cchildbaseclass", "ParentButton");
        check("cchildparentclass", "ParentButton");
        check("cchildclasslibrary", button_library_path.string());
        check("lchildhasclasslibrary", "true");
        check("lchildclasslibraryreadonly", "true");
        check("cchildclasslibraryprop", button_library_path.string());
        check("nmembersprops", "5");
        check("nmembersunion", "5");
        check("cprop1", "BASECLASS");
        check("cprop2", "CLASS");
        check("cprop3", "CLASSLIBRARY");
        check("cprop4", "PARENT");
        check("cprop5", "PARENTCLASS");
        check("cunion3", "CLASSLIBRARY");
        check("ldictset", "true");
        check("ndictcompare", "65");

        expect(state.ole_objects.size() == 3U,
               "external child external-base provenance should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "ChildForm",
                   "external child external-base provenance should preserve form identity");
            const auto &child_object = state.ole_objects[1];
            expect(child_object.prog_id == "SaveButton",
                   "external child external-base provenance should preserve child class identity");
            expect(child_object.base_class_name == "ParentButton",
                   "external child external-base provenance should preserve child base-class identity");
            expect(child_object.class_library == button_library_path.string(),
                   "external child external-base provenance should preserve the child external class-library path");
            expect(child_object.source == form_library_path.string(),
                   "external child external-base provenance should preserve the defining child-class source path");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external child external-base provenance lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_same_prg_child_external_base_classlibrary_survives_identity_mutation_guards()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_child_external_base_classlibrary_guards";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path button_library_path = temp_root / "buttons.prg";
        write_text(
            button_library_path,
            "DEFINE CLASS ParentButton AS Custom\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "native_child_external_base_classlibrary_guards.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('DemoForm')\n"
            "lSetChildClassLibrary = SETPEM(oCreate.cmdSave, 'ClassLibrary', 'other.prg')\n"
            "lAddChildClassLibrary = ADDPROPERTY(oCreate.cmdSave, 'ClassLibrary', 'other.prg')\n"
            "lRemoveChildClassLibrary = REMOVEPROPERTY(oCreate.cmdSave, 'ClassLibrary')\n"
            "oCreate.cmdSave.ClassLibrary = 'shadow.prg'\n"
            "cChildClassLibraryAfter = GETPEM(oCreate.cmdSave, 'ClassLibrary')\n"
            "cChildClassLibraryPropAfter = oCreate.cmdSave.ClassLibrary\n"
            "lChildHasClassLibrary = PEMSTATUS(oCreate.cmdSave, 'ClassLibrary', 1)\n"
            "lChildClassLibraryReadOnly = PEMSTATUS(oCreate.cmdSave, 'ClassLibrary', 5)\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 66)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS ParentButton OF buttons.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native child external-base classlibrary guard script should complete: ") + state.message +
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

        check("lsetchildclasslibrary", "false");
        check("laddchildclasslibrary", "false");
        check("lremovechildclasslibrary", "false");
        check("cchildclasslibraryafter", button_library_path.string());
        check("cchildclasslibrarypropafter", button_library_path.string());
        check("lchildhasclasslibrary", "true");
        check("lchildclasslibraryreadonly", "true");
        check("ldictset", "true");
        check("ndictcompare", "66");

        expect(state.ole_objects.size() == 3U,
               "native child external-base classlibrary guards should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "DemoForm",
                   "native child external-base classlibrary guards should preserve form identity");
            const auto &child_object = state.ole_objects[1];
            expect(child_object.prog_id == "SaveButton",
                   "native child external-base classlibrary guards should preserve child class identity");
            expect(child_object.class_library == button_library_path.string(),
                   "native child external-base classlibrary guards should preserve the child external class-library path");
            expect(!child_object.properties.contains("classlibrary"),
                   "native child external-base classlibrary guards should not materialize a child ClassLibrary shadow");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native child external-base classlibrary guards land");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_external_base_child_external_base_classlibrary_survives_identity_mutation_guards()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_prg_child_external_base_classlibrary_guards";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path button_library_path = temp_root / "buttons.prg";
        write_text(
            button_library_path,
            "DEFINE CLASS ParentButton AS Custom\n"
            "ENDDEFINE\n");

        const fs::path form_library_path = temp_root / "widgetlib.prg";
        write_text(
            form_library_path,
            "DEFINE CLASS ParentForm AS Custom\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS ParentButton OF buttons.prg\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_child_external_base_classlibrary_guards.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "lSetChildClassLibrary = SETPEM(oCreate.cmdSave, 'ClassLibrary', 'other.prg')\n"
            "lAddChildClassLibrary = ADDPROPERTY(oCreate.cmdSave, 'ClassLibrary', 'other.prg')\n"
            "lRemoveChildClassLibrary = REMOVEPROPERTY(oCreate.cmdSave, 'ClassLibrary')\n"
            "oCreate.cmdSave.ClassLibrary = 'shadow.prg'\n"
            "cChildClassLibraryAfter = GETPEM(oCreate.cmdSave, 'ClassLibrary')\n"
            "cChildClassLibraryPropAfter = oCreate.cmdSave.ClassLibrary\n"
            "lChildHasClassLibrary = PEMSTATUS(oCreate.cmdSave, 'ClassLibrary', 1)\n"
            "lChildClassLibraryReadOnly = PEMSTATUS(oCreate.cmdSave, 'ClassLibrary', 5)\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 67)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external child external-base classlibrary guard script should complete: ") + state.message +
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

        check("lsetchildclasslibrary", "false");
        check("laddchildclasslibrary", "false");
        check("lremovechildclasslibrary", "false");
        check("cchildclasslibraryafter", button_library_path.string());
        check("cchildclasslibrarypropafter", button_library_path.string());
        check("lchildhasclasslibrary", "true");
        check("lchildclasslibraryreadonly", "true");
        check("ldictset", "true");
        check("ndictcompare", "67");

        expect(state.ole_objects.size() == 3U,
               "external child external-base classlibrary guards should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "ChildForm",
                   "external child external-base classlibrary guards should preserve form identity");
            const auto &child_object = state.ole_objects[1];
            expect(child_object.prog_id == "SaveButton",
                   "external child external-base classlibrary guards should preserve child class identity");
            expect(child_object.class_library == button_library_path.string(),
                   "external child external-base classlibrary guards should preserve the child external class-library path");
            expect(child_object.source == form_library_path.string(),
                   "external child external-base classlibrary guards should preserve the defining child-class source path");
            expect(!child_object.properties.contains("classlibrary"),
                   "external child external-base classlibrary guards should not materialize a child ClassLibrary shadow");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external child external-base classlibrary guards land");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_same_prg_child_deeper_external_base_provenance_stays_coherent()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_child_deeper_external_base_provenance";
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

        const fs::path main_path = temp_root / "native_child_deeper_external_base_provenance.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('DemoForm')\n"
            "lSetChildClassLibrary = SETPEM(oCreate.cmdSave, 'ClassLibrary', 'shadow.prg')\n"
            "oCreate.cmdSave.ClassLibrary = 'shadow2.prg'\n"
            "cChildClassLibrary = GETPEM(oCreate.cmdSave, 'ClassLibrary')\n"
            "cChildBaseClass = GETPEM(oCreate.cmdSave, 'BaseClass')\n"
            "cChildClassLibraryProp = oCreate.cmdSave.ClassLibrary\n"
            "nClassCount = ACLASS(aClass, oCreate.cmdSave)\n"
            "cClass1 = aClass[1]\n"
            "cClass2 = aClass[2]\n"
            "cClass3 = aClass[3]\n"
            "cClass4 = aClass[4]\n"
            "cClass5 = aClass[5]\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS ParentButton OF buttons.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native child deeper external-base provenance script should complete: ") + state.message +
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

        check("lsetchildclasslibrary", "false");
        check("cchildclasslibrary", button_library_path.string());
        check("cchildbaseclass", "ParentButton");
        check("cchildclasslibraryprop", button_library_path.string());
        check("nclasscount", "5");
        check("cclass1", "SAVEBUTTON");
        check("cclass2", "PARENTBUTTON");
        check("cclass3", "ROOTBUTTON");
        check("cclass4", "CUSTOM");
        check("cclass5", "OBJECT");

        expect(state.ole_objects.size() == 2U,
               "native child deeper external-base provenance should register parent and child objects");
        if (state.ole_objects.size() == 2U)
        {
            expect(state.ole_objects[0].prog_id == "DemoForm",
                   "native child deeper external-base provenance should preserve form identity");
            const auto &child_object = state.ole_objects[1];
            expect(child_object.prog_id == "SaveButton",
                   "native child deeper external-base provenance should preserve child class identity");
            expect(child_object.class_library == button_library_path.string(),
                   "native child deeper external-base provenance should keep the immediate external class-library path");
            expect(child_object.class_hierarchy.size() == 5U,
                   "native child deeper external-base provenance should preserve the deeper child class hierarchy");
            expect(!child_object.properties.contains("classlibrary"),
                   "native child deeper external-base provenance should not materialize a child ClassLibrary shadow");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_external_base_child_deeper_external_base_provenance_stays_coherent()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_prg_child_deeper_external_base_provenance";
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

        const fs::path form_library_path = temp_root / "widgetlib.prg";
        write_text(
            form_library_path,
            "DEFINE CLASS ParentForm AS Custom\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS ParentButton OF buttons.prg\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_child_deeper_external_base_provenance.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "lSetChildClassLibrary = SETPEM(oCreate.cmdSave, 'ClassLibrary', 'shadow.prg')\n"
            "oCreate.cmdSave.ClassLibrary = 'shadow2.prg'\n"
            "cChildClassLibrary = GETPEM(oCreate.cmdSave, 'ClassLibrary')\n"
            "cChildBaseClass = GETPEM(oCreate.cmdSave, 'BaseClass')\n"
            "cChildClassLibraryProp = oCreate.cmdSave.ClassLibrary\n"
            "nClassCount = ACLASS(aClass, oCreate.cmdSave)\n"
            "cClass1 = aClass[1]\n"
            "cClass2 = aClass[2]\n"
            "cClass3 = aClass[3]\n"
            "cClass4 = aClass[4]\n"
            "cClass5 = aClass[5]\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external child deeper external-base provenance script should complete: ") + state.message +
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

        check("lsetchildclasslibrary", "false");
        check("cchildclasslibrary", button_library_path.string());
        check("cchildbaseclass", "ParentButton");
        check("cchildclasslibraryprop", button_library_path.string());
        check("nclasscount", "5");
        check("cclass1", "SAVEBUTTON");
        check("cclass2", "PARENTBUTTON");
        check("cclass3", "ROOTBUTTON");
        check("cclass4", "CUSTOM");
        check("cclass5", "OBJECT");

        expect(state.ole_objects.size() == 2U,
               "external child deeper external-base provenance should register form and child objects");
        if (state.ole_objects.size() == 2U)
        {
            expect(state.ole_objects[0].prog_id == "ChildForm",
                   "external child deeper external-base provenance should preserve form identity");
            const auto &child_object = state.ole_objects[1];
            expect(child_object.prog_id == "SaveButton",
                   "external child deeper external-base provenance should preserve child class identity");
            expect(child_object.class_library == button_library_path.string(),
                   "external child deeper external-base provenance should keep the immediate external class-library path");
            expect(child_object.source == form_library_path.string(),
                   "external child deeper external-base provenance should preserve the defining child-class source path");
            expect(child_object.class_hierarchy.size() == 5U,
                   "external child deeper external-base provenance should preserve the deeper child class hierarchy");
            expect(!child_object.properties.contains("classlibrary"),
                   "external child deeper external-base provenance should not materialize a child ClassLibrary shadow");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_same_prg_child_identity_metadata_appears_in_amembers()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_child_identity_amembers";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_child_identity_amembers.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('DemoForm')\n"
            "nMembersProps = AMEMBERS(aMembersProps, oCreate.cmdSave, 1)\n"
            "nMembersUnion = AMEMBERS(aMembersUnion, oCreate.cmdSave, 3)\n"
            "cProp1 = aMembersProps[1]\n"
            "cProp2 = aMembersProps[2]\n"
            "cProp3 = aMembersProps[3]\n"
            "cProp4 = aMembersProps[4]\n"
            "cProp5 = aMembersProps[5]\n"
            "cUnion1 = aMembersUnion[1]\n"
            "cUnion5 = aMembersUnion[5]\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 49)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
            "    Caption = 'Save'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native child identity AMEMBERS script should complete: ") + state.message +
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

        check("nmembersprops", "5");
        check("nmembersunion", "5");
        check("cprop1", "BASECLASS");
        check("cprop2", "CAPTION");
        check("cprop3", "CLASS");
        check("cprop4", "PARENT");
        check("cprop5", "PARENTCLASS");
        check("cunion1", "BASECLASS");
        check("cunion5", "PARENTCLASS");
        check("ldictset", "true");
        check("ndictcompare", "49");

        expect(state.ole_objects.size() == 3U,
               "native child identity AMEMBERS should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &child_object = state.ole_objects[1];
            expect(child_object.prog_id == "SaveButton",
                   "native child identity AMEMBERS should preserve child class identity");
            expect(child_object.base_class_name == "Custom",
                   "native child identity AMEMBERS should preserve child base-class identity");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native child identity AMEMBERS lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_external_base_child_identity_metadata_appears_in_amembers()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_prg_child_identity_amembers";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path library_path = temp_root / "widgetlib.prg";
        write_text(
            library_path,
            "DEFINE CLASS ParentForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
            "    Caption = 'Save'\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_child_identity_amembers.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "nMembersProps = AMEMBERS(aMembersProps, oCreate.cmdSave, 1)\n"
            "nMembersUnion = AMEMBERS(aMembersUnion, oCreate.cmdSave, 3)\n"
            "cProp1 = aMembersProps[1]\n"
            "cProp2 = aMembersProps[2]\n"
            "cProp3 = aMembersProps[3]\n"
            "cProp4 = aMembersProps[4]\n"
            "cProp5 = aMembersProps[5]\n"
            "cUnion1 = aMembersUnion[1]\n"
            "cUnion5 = aMembersUnion[5]\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 50)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external child identity AMEMBERS script should complete: ") + state.message +
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

        check("nmembersprops", "5");
        check("nmembersunion", "5");
        check("cprop1", "BASECLASS");
        check("cprop2", "CAPTION");
        check("cprop3", "CLASS");
        check("cprop4", "PARENT");
        check("cprop5", "PARENTCLASS");
        check("cunion1", "BASECLASS");
        check("cunion5", "PARENTCLASS");
        check("ldictset", "true");
        check("ndictcompare", "50");

        expect(state.ole_objects.size() == 3U,
               "external child identity AMEMBERS should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &child_object = state.ole_objects[1];
            expect(child_object.prog_id == "SaveButton",
                   "external child identity AMEMBERS should preserve child class identity");
            expect(child_object.base_class_name == "Custom",
                   "external child identity AMEMBERS should preserve child base-class identity");
            expect(child_object.class_library.empty(),
                   "external child identity AMEMBERS should keep child class-library provenance empty when the child class itself has no external base");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external child identity AMEMBERS lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_same_prg_child_parent_appears_in_amembers()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_child_parent_amembers";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_child_parent_amembers.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('DemoForm')\n"
            "nMembersProps = AMEMBERS(aMembersProps, oCreate.cmdSave, 1)\n"
            "nMembersUnion = AMEMBERS(aMembersUnion, oCreate.cmdSave, 3)\n"
            "lChildHasParent = PEMSTATUS(oCreate.cmdSave, 'Parent', 1)\n"
            "cProp1 = aMembersProps[1]\n"
            "cProp2 = aMembersProps[2]\n"
            "cProp3 = aMembersProps[3]\n"
            "cProp4 = aMembersProps[4]\n"
            "cProp5 = aMembersProps[5]\n"
            "cUnion1 = aMembersUnion[1]\n"
            "cUnion5 = aMembersUnion[5]\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 37)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
            "    Caption = 'Save'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native child Parent AMEMBERS script should complete: ") + state.message +
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

        check("nmembersprops", "5");
        check("nmembersunion", "5");
        check("lchildhasparent", "true");
        check("cprop1", "BASECLASS");
        check("cprop2", "CAPTION");
        check("cprop3", "CLASS");
        check("cprop4", "PARENT");
        check("cprop5", "PARENTCLASS");
        check("cunion1", "BASECLASS");
        check("cunion5", "PARENTCLASS");
        check("ldictset", "true");
        check("ndictcompare", "37");

        expect(state.ole_objects.size() == 3U,
               "native child Parent AMEMBERS should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "DemoForm",
                   "native child Parent AMEMBERS should preserve form identity");
            expect(state.ole_objects[1].prog_id == "SaveButton",
                   "native child Parent AMEMBERS should preserve child identity");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native child Parent AMEMBERS lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_external_base_child_parent_appears_in_amembers()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_prg_child_parent_amembers";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path library_path = temp_root / "widgetlib.prg";
        write_text(
            library_path,
            "DEFINE CLASS ParentForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
            "    Caption = 'Save'\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_child_parent_amembers.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "nMembersProps = AMEMBERS(aMembersProps, oCreate.cmdSave, 1)\n"
            "nMembersUnion = AMEMBERS(aMembersUnion, oCreate.cmdSave, 3)\n"
            "lChildHasParent = PEMSTATUS(oCreate.cmdSave, 'Parent', 1)\n"
            "cProp1 = aMembersProps[1]\n"
            "cProp2 = aMembersProps[2]\n"
            "cProp3 = aMembersProps[3]\n"
            "cProp4 = aMembersProps[4]\n"
            "cProp5 = aMembersProps[5]\n"
            "cProp6 = aMembersProps[6]\n"
            "cUnion1 = aMembersUnion[1]\n"
            "cUnion5 = aMembersUnion[5]\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 38)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external child Parent AMEMBERS script should complete: ") + state.message +
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

        check("nmembersprops", "5");
        check("nmembersunion", "5");
        check("lchildhasparent", "true");
        check("cprop1", "BASECLASS");
        check("cprop2", "CAPTION");
        check("cprop3", "CLASS");
        check("cprop4", "PARENT");
        check("cprop5", "PARENTCLASS");
        check("cunion1", "BASECLASS");
        check("cunion5", "PARENTCLASS");
        check("ldictset", "true");
        check("ndictcompare", "38");

        expect(state.ole_objects.size() == 3U,
               "external child Parent AMEMBERS should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "ChildForm",
                   "external child Parent AMEMBERS should preserve form identity");
            expect(state.ole_objects[1].prog_id == "SaveButton",
                   "external child Parent AMEMBERS should preserve child identity");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external child Parent AMEMBERS lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_same_prg_child_parent_stays_read_only_to_setpem()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_child_parent_setpem";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_child_parent_setpem.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('DemoForm')\n"
            "lParentReadOnly = PEMSTATUS(oCreate.cmdSave, 'Parent', 5)\n"
            "lSetParent = SETPEM(oCreate.cmdSave, 'Parent', 'OtherParent')\n"
            "oParentRef = GETPEM(oCreate.cmdSave, 'Parent')\n"
            "cParentCaption = oParentRef.Caption\n"
            "lChildHasParent = PEMSTATUS(oCreate.cmdSave, 'Parent', 1)\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 39)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native child Parent SETPEM script should complete: ") + state.message +
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

        check("lparentreadonly", "true");
        check("lsetparent", "false");
        check("cparentcaption", "MainForm");
        check("lchildhasparent", "true");
        check("ldictset", "true");
        check("ndictcompare", "39");

        expect(state.ole_objects.size() == 3U,
               "native child Parent SETPEM should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "DemoForm",
                   "native child Parent SETPEM should preserve form identity");
            const auto &child_object = state.ole_objects[1];
            expect(child_object.prog_id == "SaveButton",
                   "native child Parent SETPEM should preserve child identity");
            const auto child_parent = child_object.properties.find("parent");
            if (child_parent != child_object.properties.end())
            {
                expect(child_parent->second.kind == copperfin::runtime::PrgValueKind::string,
                       "native child Parent SETPEM should keep the built-in PARENT object reference materialized");
                expect(child_parent->second.string_value == "object:DemoForm#" + std::to_string(state.ole_objects[0].handle),
                       "native child Parent SETPEM should not replace the built-in PARENT object reference");
            }
            else
            {
                expect(false, "native child Parent SETPEM should preserve the child PARENT reference");
            }
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native child Parent SETPEM lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_external_base_child_parent_stays_read_only_to_setpem()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_prg_child_parent_setpem";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path library_path = temp_root / "widgetlib.prg";
        write_text(
            library_path,
            "DEFINE CLASS ParentForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_child_parent_setpem.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "lParentReadOnly = PEMSTATUS(oCreate.cmdSave, 'Parent', 5)\n"
            "lSetParent = SETPEM(oCreate.cmdSave, 'Parent', 'OtherParent')\n"
            "oParentRef = GETPEM(oCreate.cmdSave, 'Parent')\n"
            "cParentCaption = oParentRef.Caption\n"
            "lChildHasParent = PEMSTATUS(oCreate.cmdSave, 'Parent', 1)\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 40)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external child Parent SETPEM script should complete: ") + state.message +
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

        check("lparentreadonly", "true");
        check("lsetparent", "false");
        check("cparentcaption", "MainForm");
        check("lchildhasparent", "true");
        check("ldictset", "true");
        check("ndictcompare", "40");

        expect(state.ole_objects.size() == 3U,
               "external child Parent SETPEM should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "ChildForm",
                   "external child Parent SETPEM should preserve form identity");
            const auto &child_object = state.ole_objects[1];
            expect(child_object.prog_id == "SaveButton",
                   "external child Parent SETPEM should preserve child identity");
            const auto child_parent = child_object.properties.find("parent");
            if (child_parent != child_object.properties.end())
            {
                expect(child_parent->second.kind == copperfin::runtime::PrgValueKind::string,
                       "external child Parent SETPEM should keep the built-in PARENT object reference materialized");
                expect(child_parent->second.string_value == "object:ChildForm#" + std::to_string(state.ole_objects[0].handle),
                       "external child Parent SETPEM should not replace the built-in PARENT object reference");
            }
            else
            {
                expect(false, "external child Parent SETPEM should preserve the child PARENT reference");
            }
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external child Parent SETPEM lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_same_prg_child_parent_cannot_be_shadowed_through_addproperty()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_child_parent_addproperty";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_child_parent_addproperty.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('DemoForm')\n"
            "lAddParent = ADDPROPERTY(oCreate.cmdSave, 'Parent', 'OtherParent')\n"
            "oParentRef = GETPEM(oCreate.cmdSave, 'Parent')\n"
            "cParentCaption = oParentRef.Caption\n"
            "lChildHasParent = PEMSTATUS(oCreate.cmdSave, 'Parent', 1)\n"
            "lParentReadOnly = PEMSTATUS(oCreate.cmdSave, 'Parent', 5)\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 41)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native child Parent ADDPROPERTY script should complete: ") + state.message +
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

        check("laddparent", "false");
        check("cparentcaption", "MainForm");
        check("lchildhasparent", "true");
        check("lparentreadonly", "true");
        check("ldictset", "true");
        check("ndictcompare", "41");

        expect(state.ole_objects.size() == 3U,
               "native child Parent ADDPROPERTY should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "DemoForm",
                   "native child Parent ADDPROPERTY should preserve form identity");
            const auto &child_object = state.ole_objects[1];
            expect(child_object.prog_id == "SaveButton",
                   "native child Parent ADDPROPERTY should preserve child identity");
            const auto child_parent = child_object.properties.find("parent");
            if (child_parent != child_object.properties.end())
            {
                expect(child_parent->second.kind == copperfin::runtime::PrgValueKind::string,
                       "native child Parent ADDPROPERTY should keep the built-in PARENT object reference materialized");
                expect(child_parent->second.string_value == "object:DemoForm#" + std::to_string(state.ole_objects[0].handle),
                       "native child Parent ADDPROPERTY should not replace the built-in PARENT object reference");
            }
            else
            {
                expect(false, "native child Parent ADDPROPERTY should preserve the child PARENT reference");
            }
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native child Parent ADDPROPERTY lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_external_base_child_parent_cannot_be_shadowed_through_addproperty()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_prg_child_parent_addproperty";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path library_path = temp_root / "widgetlib.prg";
        write_text(
            library_path,
            "DEFINE CLASS ParentForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_child_parent_addproperty.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "lAddParent = ADDPROPERTY(oCreate.cmdSave, 'Parent', 'OtherParent')\n"
            "oParentRef = GETPEM(oCreate.cmdSave, 'Parent')\n"
            "cParentCaption = oParentRef.Caption\n"
            "lChildHasParent = PEMSTATUS(oCreate.cmdSave, 'Parent', 1)\n"
            "lParentReadOnly = PEMSTATUS(oCreate.cmdSave, 'Parent', 5)\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 42)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external child Parent ADDPROPERTY script should complete: ") + state.message +
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

        check("laddparent", "false");
        check("cparentcaption", "MainForm");
        check("lchildhasparent", "true");
        check("lparentreadonly", "true");
        check("ldictset", "true");
        check("ndictcompare", "42");

        expect(state.ole_objects.size() == 3U,
               "external child Parent ADDPROPERTY should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "ChildForm",
                   "external child Parent ADDPROPERTY should preserve form identity");
            const auto &child_object = state.ole_objects[1];
            expect(child_object.prog_id == "SaveButton",
                   "external child Parent ADDPROPERTY should preserve child identity");
            const auto child_parent = child_object.properties.find("parent");
            if (child_parent != child_object.properties.end())
            {
                expect(child_parent->second.kind == copperfin::runtime::PrgValueKind::string,
                       "external child Parent ADDPROPERTY should keep the built-in PARENT object reference materialized");
                expect(child_parent->second.string_value == "object:ChildForm#" + std::to_string(state.ole_objects[0].handle),
                       "external child Parent ADDPROPERTY should not replace the built-in PARENT object reference");
            }
            else
            {
                expect(false, "external child Parent ADDPROPERTY should preserve the child PARENT reference");
            }
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external child Parent ADDPROPERTY lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_same_prg_child_parent_stays_protected_from_removeproperty()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_child_parent_removeproperty";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_child_parent_removeproperty.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('DemoForm')\n"
            "lRemoveParent = REMOVEPROPERTY(oCreate.cmdSave, 'Parent')\n"
            "oParentRef = GETPEM(oCreate.cmdSave, 'Parent')\n"
            "cParentCaption = oParentRef.Caption\n"
            "lChildHasParent = PEMSTATUS(oCreate.cmdSave, 'Parent', 1)\n"
            "lParentReadOnly = PEMSTATUS(oCreate.cmdSave, 'Parent', 5)\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 43)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native child Parent REMOVEPROPERTY script should complete: ") + state.message +
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

        check("lremoveparent", "false");
        check("cparentcaption", "MainForm");
        check("lchildhasparent", "true");
        check("lparentreadonly", "true");
        check("ldictset", "true");
        check("ndictcompare", "43");

        expect(state.ole_objects.size() == 3U,
               "native child Parent REMOVEPROPERTY should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "DemoForm",
                   "native child Parent REMOVEPROPERTY should preserve form identity");
            const auto &child_object = state.ole_objects[1];
            expect(child_object.prog_id == "SaveButton",
                   "native child Parent REMOVEPROPERTY should preserve child identity");
            const auto child_parent = child_object.properties.find("parent");
            if (child_parent != child_object.properties.end())
            {
                expect(child_parent->second.kind == copperfin::runtime::PrgValueKind::string,
                       "native child Parent REMOVEPROPERTY should keep the built-in PARENT object reference materialized");
                expect(child_parent->second.string_value == "object:DemoForm#" + std::to_string(state.ole_objects[0].handle),
                       "native child Parent REMOVEPROPERTY should not erase the built-in PARENT object reference");
            }
            else
            {
                expect(false, "native child Parent REMOVEPROPERTY should preserve the child PARENT reference");
            }
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native child Parent REMOVEPROPERTY lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_external_base_child_parent_stays_protected_from_removeproperty()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_prg_child_parent_removeproperty";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path library_path = temp_root / "widgetlib.prg";
        write_text(
            library_path,
            "DEFINE CLASS ParentForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_child_parent_removeproperty.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "lRemoveParent = REMOVEPROPERTY(oCreate.cmdSave, 'Parent')\n"
            "oParentRef = GETPEM(oCreate.cmdSave, 'Parent')\n"
            "cParentCaption = oParentRef.Caption\n"
            "lChildHasParent = PEMSTATUS(oCreate.cmdSave, 'Parent', 1)\n"
            "lParentReadOnly = PEMSTATUS(oCreate.cmdSave, 'Parent', 5)\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 44)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external child Parent REMOVEPROPERTY script should complete: ") + state.message +
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

        check("lremoveparent", "false");
        check("cparentcaption", "MainForm");
        check("lchildhasparent", "true");
        check("lparentreadonly", "true");
        check("ldictset", "true");
        check("ndictcompare", "44");

        expect(state.ole_objects.size() == 3U,
               "external child Parent REMOVEPROPERTY should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "ChildForm",
                   "external child Parent REMOVEPROPERTY should preserve form identity");
            const auto &child_object = state.ole_objects[1];
            expect(child_object.prog_id == "SaveButton",
                   "external child Parent REMOVEPROPERTY should preserve child identity");
            const auto child_parent = child_object.properties.find("parent");
            if (child_parent != child_object.properties.end())
            {
                expect(child_parent->second.kind == copperfin::runtime::PrgValueKind::string,
                       "external child Parent REMOVEPROPERTY should keep the built-in PARENT object reference materialized");
                expect(child_parent->second.string_value == "object:ChildForm#" + std::to_string(state.ole_objects[0].handle),
                       "external child Parent REMOVEPROPERTY should not erase the built-in PARENT object reference");
            }
            else
            {
                expect(false, "external child Parent REMOVEPROPERTY should preserve the child PARENT reference");
            }
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external child Parent REMOVEPROPERTY lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_same_prg_child_parent_cannot_be_shadowed_through_direct_assignment()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_child_parent_direct_assignment";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_child_parent_direct_assignment.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('DemoForm')\n"
            "oCreate.cmdSave.Parent = 'OtherParent'\n"
            "oParentRef = GETPEM(oCreate.cmdSave, 'Parent')\n"
            "cParentCaption = oParentRef.Caption\n"
            "lChildHasParent = PEMSTATUS(oCreate.cmdSave, 'Parent', 1)\n"
            "lParentReadOnly = PEMSTATUS(oCreate.cmdSave, 'Parent', 5)\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 45)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native child Parent direct-assignment script should complete: ") + state.message +
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

        check("cparentcaption", "MainForm");
        check("lchildhasparent", "true");
        check("lparentreadonly", "true");
        check("ldictset", "true");
        check("ndictcompare", "45");

        expect(state.ole_objects.size() == 3U,
               "native child Parent direct assignment should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "DemoForm",
                   "native child Parent direct assignment should preserve form identity");
            const auto &child_object = state.ole_objects[1];
            expect(child_object.prog_id == "SaveButton",
                   "native child Parent direct assignment should preserve child identity");
            const auto child_parent = child_object.properties.find("parent");
            if (child_parent != child_object.properties.end())
            {
                expect(child_parent->second.kind == copperfin::runtime::PrgValueKind::string,
                       "native child Parent direct assignment should keep the built-in PARENT object reference materialized");
                expect(child_parent->second.string_value == "object:DemoForm#" + std::to_string(state.ole_objects[0].handle),
                       "native child Parent direct assignment should not replace the built-in PARENT object reference");
            }
            else
            {
                expect(false, "native child Parent direct assignment should preserve the child PARENT reference");
            }
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native child Parent direct assignment lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_external_base_child_parent_cannot_be_shadowed_through_direct_assignment()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_prg_child_parent_direct_assignment";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path library_path = temp_root / "widgetlib.prg";
        write_text(
            library_path,
            "DEFINE CLASS ParentForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_child_parent_direct_assignment.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oCreate.cmdSave.Parent = 'OtherParent'\n"
            "oParentRef = GETPEM(oCreate.cmdSave, 'Parent')\n"
            "cParentCaption = oParentRef.Caption\n"
            "lChildHasParent = PEMSTATUS(oCreate.cmdSave, 'Parent', 1)\n"
            "lParentReadOnly = PEMSTATUS(oCreate.cmdSave, 'Parent', 5)\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 46)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external child Parent direct-assignment script should complete: ") + state.message +
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

        check("cparentcaption", "MainForm");
        check("lchildhasparent", "true");
        check("lparentreadonly", "true");
        check("ldictset", "true");
        check("ndictcompare", "46");

        expect(state.ole_objects.size() == 3U,
               "external child Parent direct assignment should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            expect(state.ole_objects[0].prog_id == "ChildForm",
                   "external child Parent direct assignment should preserve form identity");
            const auto &child_object = state.ole_objects[1];
            expect(child_object.prog_id == "SaveButton",
                   "external child Parent direct assignment should preserve child identity");
            const auto child_parent = child_object.properties.find("parent");
            if (child_parent != child_object.properties.end())
            {
                expect(child_parent->second.kind == copperfin::runtime::PrgValueKind::string,
                       "external child Parent direct assignment should keep the built-in PARENT object reference materialized");
                expect(child_parent->second.string_value == "object:ChildForm#" + std::to_string(state.ole_objects[0].handle),
                       "external child Parent direct assignment should not replace the built-in PARENT object reference");
            }
            else
            {
                expect(false, "external child Parent direct assignment should preserve the child PARENT reference");
            }
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external child Parent direct assignment lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_inherited_declarative_children_from_external_prg_bases_resolve_against_defining_library()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_external_base_inherited_children";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path library_path = temp_root / "widgetlib.prg";
        write_text(
            library_path,
            "DEFINE CLASS ParentForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    cInitChildCaption = ''\n"
            "    cInitOwnerCaption = ''\n"
            "    OBJECT cmdSave AS SaveButton\n"
            "        Caption = 'Commit'\n"
            "    ENDOBJECT\n"
            "    PROCEDURE Init\n"
            "        THIS.cInitChildCaption = THIS.cmdSave.Caption\n"
            "        THIS.cInitOwnerCaption = THIS.cmdSave.OwnerCaption()\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
            "    Caption = 'Save'\n"
            "    FUNCTION OwnerCaption\n"
            "        RETURN PARENT.Caption\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_base_inherited_children.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oNew = NEWOBJECT('LeafForm')\n"
            "lCreateHasChild = PEMSTATUS(oCreate, 'cmdSave', 1)\n"
            "lNewHasChild = PEMSTATUS(oNew, 'cmdSave', 1)\n"
            "cCreateInitChildCaption = oCreate.cInitChildCaption\n"
            "cNewInitChildCaption = oNew.cInitChildCaption\n"
            "cCreateInitOwnerCaption = oCreate.cInitOwnerCaption\n"
            "cNewInitOwnerCaption = oNew.cInitOwnerCaption\n"
            "cCreateChildCaption = oCreate.cmdSave.Caption\n"
            "cNewChildCaption = oNew.cmdSave.Caption\n"
            "cCreateOwnerCaption = oCreate.cmdSave.OwnerCaption()\n"
            "cNewOwnerCaption = oNew.cmdSave.OwnerCaption()\n"
            "lCreateChildHasParent = PEMSTATUS(oCreate.cmdSave, 'Parent', 1)\n"
            "lNewChildHasParent = PEMSTATUS(oNew.cmdSave, 'Parent', 1)\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 17)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n"
            "DEFINE CLASS LeafForm AS ChildForm\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external-base inherited declarative children script should complete: ") + state.message +
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

        check("lcreatehaschild", "true");
        check("lnewhaschild", "true");
        check("ccreateinitchildcaption", "Commit");
        check("cnewinitchildcaption", "Commit");
        check("ccreateinitownercaption", "MainForm");
        check("cnewinitownercaption", "MainForm");
        check("ccreatechildcaption", "Commit");
        check("cnewchildcaption", "Commit");
        check("ccreateownercaption", "MainForm");
        check("cnewownercaption", "MainForm");
        check("lcreatechildhasparent", "true");
        check("lnewchildhasparent", "true");
        check("ldictset", "true");
        check("ndictcompare", "17");

        expect(state.ole_objects.size() == 5U,
               "external-base inherited declarative children script should register CREATEOBJECT parent/child, NEWOBJECT parent/child, and COM objects");
        if (state.ole_objects.size() == 5U)
        {
            const auto &create_parent = state.ole_objects[0];
            const auto &create_child = state.ole_objects[1];
            const auto &new_parent = state.ole_objects[2];
            const auto &new_child = state.ole_objects[3];
            expect(create_parent.prog_id == "ChildForm",
                   "external-base inherited declarative children should preserve CREATEOBJECT parent class identity");
            expect(create_child.prog_id == "SaveButton",
                   "external-base inherited declarative children should materialize the inherited child class");
            expect(create_child.source == library_path.string(),
                   "external-base inherited declarative children should resolve inherited child classes against the defining external PRG");
            expect(new_parent.prog_id == "LeafForm",
                   "external-base inherited declarative children should preserve NEWOBJECT leaf class identity");
            expect(new_child.prog_id == "SaveButton",
                   "external-base inherited declarative children should materialize the inherited child class for leaf instances");
            expect(new_child.source == library_path.string(),
                   "external-base inherited declarative children should preserve external PRG provenance for leaf child instances");
            const auto create_child_caption = create_child.properties.find("caption");
            const auto new_child_caption = new_child.properties.find("caption");
            if (create_child_caption != create_child.properties.end())
            {
                expect(copperfin::runtime::format_value(create_child_caption->second) == "Commit",
                       "external-base inherited declarative children should preserve inherited child property overrides");
            }
            else
            {
                expect(false, "external-base inherited declarative children should materialize inherited child caption overrides");
            }
            if (new_child_caption != new_child.properties.end())
            {
                expect(copperfin::runtime::format_value(new_child_caption->second) == "Commit",
                       "external-base inherited declarative children should preserve inherited child overrides on leaf instances");
            }
            else
            {
                expect(false, "external-base inherited declarative children should materialize inherited child caption overrides on leaf instances");
            }
            expect(state.ole_objects[4].prog_id == "Scripting.Dictionary",
                   "COM behavior should remain stable while external-base inherited declarative children land");
        }

        const bool has_addobject_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.addobject" &&
                   (event.detail == "ChildForm.cmdsave:SaveButton" ||
                    event.detail == "LeafForm.cmdsave:SaveButton");
        });
        expect(has_addobject_event,
               "external-base inherited declarative children should emit child materialization events");

        fs::remove_all(temp_root, ignored);
    }

    void test_inherited_external_prg_base_methods_resolve_addobject_children_against_defining_library()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_external_base_inherited_addobject";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path library_path = temp_root / "widgetlib.prg";
        write_text(
            library_path,
            "DEFINE CLASS ParentForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    cInitChildCaption = ''\n"
            "    cInitOwnerCaption = ''\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton')\n"
            "        THIS.cInitChildCaption = THIS.cmdSave.Caption\n"
            "        THIS.cInitOwnerCaption = THIS.cmdSave.OwnerCaption()\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
            "    Caption = 'Save'\n"
            "    FUNCTION OwnerCaption\n"
            "        RETURN PARENT.Caption\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_base_inherited_addobject.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oNew = NEWOBJECT('LeafForm')\n"
            "lCreateHasChild = PEMSTATUS(oCreate, 'cmdSave', 1)\n"
            "lNewHasChild = PEMSTATUS(oNew, 'cmdSave', 1)\n"
            "cCreateInitChildCaption = oCreate.cInitChildCaption\n"
            "cNewInitChildCaption = oNew.cInitChildCaption\n"
            "cCreateInitOwnerCaption = oCreate.cInitOwnerCaption\n"
            "cNewInitOwnerCaption = oNew.cInitOwnerCaption\n"
            "cCreateChildCaption = oCreate.cmdSave.Caption\n"
            "cNewChildCaption = oNew.cmdSave.Caption\n"
            "cCreateOwnerCaption = oCreate.cmdSave.OwnerCaption()\n"
            "cNewOwnerCaption = oNew.cmdSave.OwnerCaption()\n"
            "lCreateChildHasParent = PEMSTATUS(oCreate.cmdSave, 'Parent', 1)\n"
            "lNewChildHasParent = PEMSTATUS(oNew.cmdSave, 'Parent', 1)\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 18)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n"
            "DEFINE CLASS LeafForm AS ChildForm\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external-base inherited ADDOBJECT script should complete: ") + state.message +
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

        check("lcreatehaschild", "true");
        check("lnewhaschild", "true");
        check("ccreateinitchildcaption", "Save");
        check("cnewinitchildcaption", "Save");
        check("ccreateinitownercaption", "MainForm");
        check("cnewinitownercaption", "MainForm");
        check("ccreatechildcaption", "Save");
        check("cnewchildcaption", "Save");
        check("ccreateownercaption", "MainForm");
        check("cnewownercaption", "MainForm");
        check("lcreatechildhasparent", "true");
        check("lnewchildhasparent", "true");
        check("ldictset", "true");
        check("ndictcompare", "18");

        expect(state.ole_objects.size() == 5U,
               "external-base inherited ADDOBJECT script should register CREATEOBJECT parent/child, NEWOBJECT parent/child, and COM objects");
        if (state.ole_objects.size() == 5U)
        {
            const auto &create_parent = state.ole_objects[0];
            const auto &create_child = state.ole_objects[1];
            const auto &new_parent = state.ole_objects[2];
            const auto &new_child = state.ole_objects[3];
            expect(create_parent.prog_id == "ChildForm",
                   "external-base inherited ADDOBJECT should preserve CREATEOBJECT parent class identity");
            expect(create_child.prog_id == "SaveButton",
                   "external-base inherited ADDOBJECT should materialize the inherited child class");
            expect(create_child.source == library_path.string(),
                   "external-base inherited ADDOBJECT should resolve child classes against the inherited method's defining external PRG");
            expect(new_parent.prog_id == "LeafForm",
                   "external-base inherited ADDOBJECT should preserve NEWOBJECT leaf class identity");
            expect(new_child.prog_id == "SaveButton",
                   "external-base inherited ADDOBJECT should materialize inherited children for leaf instances");
            expect(new_child.source == library_path.string(),
                   "external-base inherited ADDOBJECT should preserve external PRG provenance for leaf child instances");
            expect(state.ole_objects[4].prog_id == "Scripting.Dictionary",
                   "COM behavior should remain stable while external-base inherited ADDOBJECT lands");
        }

        const bool has_addobject_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.addobject" &&
                   (event.detail == "ChildForm.cmdsave:SaveButton" ||
                    event.detail == "LeafForm.cmdsave:SaveButton");
        });
        expect(has_addobject_event,
               "external-base inherited ADDOBJECT should emit child materialization events");

        fs::remove_all(temp_root, ignored);
    }

    void test_inherited_external_base_addobject_external_child_base_surfaces_classlibrary_provenance()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_base_inherited_addobject_external_child_base_provenance";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path root_library_path = temp_root / "rootbuttons.prg";
        write_text(
            root_library_path,
            "DEFINE CLASS ParentButton AS Custom\n"
            "ENDDEFINE\n");

        const fs::path widget_library_path = temp_root / "widgetlib.prg";
        write_text(
            widget_library_path,
            "DEFINE CLASS ParentForm AS Custom\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton', 'buttons.prg')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        const fs::path button_library_path = temp_root / "buttons.prg";
        write_text(
            button_library_path,
            "DEFINE CLASS SaveButton AS ParentButton OF rootbuttons.prg\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_base_inherited_addobject_external_child_base_provenance.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oChild = oCreate.cmdSave\n"
            "cChildClassLibrary = GETPEM(oChild, 'ClassLibrary')\n"
            "lChildHasClassLibrary = PEMSTATUS(oChild, 'ClassLibrary', 1)\n"
            "lChildClassLibraryReadOnly = PEMSTATUS(oChild, 'ClassLibrary', 5)\n"
            "cChildClassLibraryProp = oChild.ClassLibrary\n"
            "nMembersProps = AMEMBERS(aMembersProps, oChild, 1)\n"
            "cProp3 = aMembersProps[3]\n"
            "lSetChildClassLibrary = SETPEM(oChild, 'ClassLibrary', 'shadow.prg')\n"
            "oChild.ClassLibrary = 'shadow2.prg'\n"
            "cChildClassLibraryAfter = GETPEM(oChild, 'ClassLibrary')\n"
            "cChildClassLibraryPropAfter = oChild.ClassLibrary\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 69)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external-base inherited ADDOBJECT external child-base provenance script should complete: ") + state.message +
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

        check("cchildclasslibrary", root_library_path.string());
        check("lchildhasclasslibrary", "true");
        check("lchildclasslibraryreadonly", "true");
        check("cchildclasslibraryprop", root_library_path.string());
        check("cprop3", "CLASSLIBRARY");
        check("lsetchildclasslibrary", "false");
        check("cchildclasslibraryafter", root_library_path.string());
        check("cchildclasslibrarypropafter", root_library_path.string());
        check("ldictset", "true");
        check("ndictcompare", "69");

        expect(state.ole_objects.size() == 3U,
               "external-base inherited ADDOBJECT external child-base provenance should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &parent_object = state.ole_objects[0];
            const auto &child_object = state.ole_objects[1];
            expect(parent_object.prog_id == "ChildForm",
                   "external-base inherited ADDOBJECT external child-base provenance should preserve parent identity");
            expect(child_object.prog_id == "SaveButton",
                   "external-base inherited ADDOBJECT external child-base provenance should preserve child identity");
            expect(child_object.source == button_library_path.string(),
                   "external-base inherited ADDOBJECT external child-base provenance should preserve the child definition source path");
            expect(child_object.class_library == root_library_path.string(),
                   "external-base inherited ADDOBJECT external child-base provenance should preserve the external child ClassLibrary path");
            expect(!child_object.properties.contains("classlibrary"),
                   "external-base inherited ADDOBJECT external child-base provenance should not materialize a ClassLibrary shadow");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external-base inherited ADDOBJECT external child-base provenance lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_inherited_external_base_addobject_external_child_base_aclass_reflects_inheritance_chain()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_base_inherited_addobject_external_child_base_aclass";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path root_library_path = temp_root / "rootbuttons.prg";
        write_text(
            root_library_path,
            "DEFINE CLASS ParentButton AS Custom\n"
            "ENDDEFINE\n");

        const fs::path widget_library_path = temp_root / "widgetlib.prg";
        write_text(
            widget_library_path,
            "DEFINE CLASS ParentForm AS Custom\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton', 'buttons.prg')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        const fs::path button_library_path = temp_root / "buttons.prg";
        write_text(
            button_library_path,
            "DEFINE CLASS SaveButton AS ParentButton OF rootbuttons.prg\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_base_inherited_addobject_external_child_base_aclass.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oChild = oCreate.cmdSave\n"
            "nClassCount = ACLASS(aClass, oChild)\n"
            "cClass1 = aClass[1]\n"
            "cClass2 = aClass[2]\n"
            "cClass3 = aClass[3]\n"
            "cClass4 = aClass[4]\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external-base inherited ADDOBJECT external child-base ACLASS script should complete: ") + state.message +
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

        check("nclasscount", "4");
        check("cclass1", "SAVEBUTTON");
        check("cclass2", "PARENTBUTTON");
        check("cclass3", "CUSTOM");
        check("cclass4", "OBJECT");

        expect(state.ole_objects.size() == 2U,
               "external-base inherited ADDOBJECT external child-base ACLASS should register parent and child objects");
        if (state.ole_objects.size() == 2U)
        {
            const auto &parent_object = state.ole_objects[0];
            const auto &child_object = state.ole_objects[1];
            expect(parent_object.prog_id == "ChildForm",
                   "external-base inherited ADDOBJECT external child-base ACLASS should preserve parent identity");
            expect(child_object.prog_id == "SaveButton",
                   "external-base inherited ADDOBJECT external child-base ACLASS should preserve child identity");
            expect(child_object.source == button_library_path.string(),
                   "external-base inherited ADDOBJECT external child-base ACLASS should preserve child source path");
            expect(child_object.class_library == root_library_path.string(),
                   "external-base inherited ADDOBJECT external child-base ACLASS should preserve external child ClassLibrary path");
            expect(child_object.class_hierarchy.size() == 4U,
                   "external-base inherited ADDOBJECT external child-base ACLASS should preserve runtime child class hierarchy");
            if (child_object.class_hierarchy.size() == 4U)
            {
                expect(child_object.class_hierarchy[0] == "SAVEBUTTON",
                       "external-base inherited ADDOBJECT external child-base ACLASS should store the derived child class first");
                expect(child_object.class_hierarchy[1] == "PARENTBUTTON",
                       "external-base inherited ADDOBJECT external child-base ACLASS should store the external parent class second");
                expect(child_object.class_hierarchy[2] == "CUSTOM",
                       "external-base inherited ADDOBJECT external child-base ACLASS should store the builtin base token");
                expect(child_object.class_hierarchy[3] == "OBJECT",
                       "external-base inherited ADDOBJECT external child-base ACLASS should store the terminal object token");
            }
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_inherited_external_base_addobject_deeper_external_child_base_aclass_reflects_inheritance_chain()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_base_inherited_addobject_deeper_external_child_base_aclass";
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
            "ENDDEFINE\n");

        const fs::path widget_library_path = temp_root / "widgetlib.prg";
        write_text(
            widget_library_path,
            "DEFINE CLASS ParentForm AS Custom\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton', 'buttons.prg')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_base_inherited_addobject_deeper_external_child_base_aclass.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oChild = oCreate.cmdSave\n"
            "nClassCount = ACLASS(aClass, oChild)\n"
            "cClass1 = aClass[1]\n"
            "cClass2 = aClass[2]\n"
            "cClass3 = aClass[3]\n"
            "cClass4 = aClass[4]\n"
            "cClass5 = aClass[5]\n"
            "xChildClassLibrary = GETPEM(oChild, 'ClassLibrary')\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external-base inherited ADDOBJECT deeper external child-base ACLASS script should complete: ") + state.message +
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

        check("nclasscount", "5");
        check("cclass1", "SAVEBUTTON");
        check("cclass2", "PARENTBUTTON");
        check("cclass3", "ROOTBUTTON");
        check("cclass4", "CUSTOM");
        check("cclass5", "OBJECT");
        const auto child_class_library = state.globals.find("xchildclasslibrary");
        expect(child_class_library != state.globals.end() &&
                   child_class_library->second.kind == copperfin::runtime::PrgValueKind::empty,
               "external-base inherited ADDOBJECT deeper external child-base ACLASS should leave the derived child ClassLibrary empty");

        expect(state.ole_objects.size() == 2U,
               "external-base inherited ADDOBJECT deeper external child-base ACLASS should register parent and child objects");
        if (state.ole_objects.size() == 2U)
        {
            const auto &parent_object = state.ole_objects[0];
            const auto &child_object = state.ole_objects[1];
            expect(parent_object.prog_id == "ChildForm",
                   "external-base inherited ADDOBJECT deeper external child-base ACLASS should preserve parent identity");
            expect(child_object.prog_id == "SaveButton",
                   "external-base inherited ADDOBJECT deeper external child-base ACLASS should preserve child identity");
            expect(child_object.source == button_library_path.string(),
                   "external-base inherited ADDOBJECT deeper external child-base ACLASS should preserve the child definition source path");
            expect(child_object.class_library.empty(),
                   "external-base inherited ADDOBJECT deeper external child-base ACLASS should leave the derived child ClassLibrary empty");
            expect(child_object.class_hierarchy.size() == 5U,
                   "external-base inherited ADDOBJECT deeper external child-base ACLASS should preserve the deeper runtime child class hierarchy");
            if (child_object.class_hierarchy.size() == 5U)
            {
                expect(child_object.class_hierarchy[0] == "SAVEBUTTON",
                       "external-base inherited ADDOBJECT deeper external child-base ACLASS should store the derived child class first");
                expect(child_object.class_hierarchy[1] == "PARENTBUTTON",
                       "external-base inherited ADDOBJECT deeper external child-base ACLASS should store the intermediate external parent second");
                expect(child_object.class_hierarchy[2] == "ROOTBUTTON",
                       "external-base inherited ADDOBJECT deeper external child-base ACLASS should store the deeper external ancestor third");
                expect(child_object.class_hierarchy[3] == "CUSTOM",
                       "external-base inherited ADDOBJECT deeper external child-base ACLASS should store the builtin base token");
                expect(child_object.class_hierarchy[4] == "OBJECT",
                       "external-base inherited ADDOBJECT deeper external child-base ACLASS should store the terminal object token");
            }
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_native_addobject_deeper_external_child_identity_surfaces_stay_coherent()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_addobject_deeper_external_child_identity";
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
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "native_addobject_deeper_external_child_identity.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('DemoForm')\n"
            "oChild = oForm.cmdSave\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 70)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "cChildClass = oChild.Class\n"
            "cChildBaseClass = oChild.BaseClass\n"
            "cChildParentClass = oChild.ParentClass\n"
            "xChildClassLibrary = oChild.ClassLibrary\n"
            "cChildClassReflect = GETPEM(oChild, 'Class')\n"
            "cChildBaseClassReflect = GETPEM(oChild, 'BaseClass')\n"
            "cChildParentClassReflect = GETPEM(oChild, 'ParentClass')\n"
            "xChildClassLibraryReflect = GETPEM(oChild, 'ClassLibrary')\n"
            "lChildHasClass = PEMSTATUS(oChild, 'Class', 1)\n"
            "lChildHasBaseClass = PEMSTATUS(oChild, 'BaseClass', 1)\n"
            "lChildHasParentClass = PEMSTATUS(oChild, 'ParentClass', 1)\n"
            "lChildHasClassLibrary = PEMSTATUS(oChild, 'ClassLibrary', 1)\n"
            "lChildClassReadOnly = PEMSTATUS(oChild, 'Class', 5)\n"
            "lChildBaseClassReadOnly = PEMSTATUS(oChild, 'BaseClass', 5)\n"
            "lChildParentClassReadOnly = PEMSTATUS(oChild, 'ParentClass', 5)\n"
            "nMembersProps = AMEMBERS(aMembersProps, oChild, 1)\n"
            "nMembersUnion = AMEMBERS(aMembersUnion, oChild, 3)\n"
            "cProp1 = aMembersProps[1]\n"
            "cProp2 = aMembersProps[2]\n"
            "cProp3 = aMembersProps[3]\n"
            "cProp4 = aMembersProps[4]\n"
            "cProp5 = aMembersProps[5]\n"
            "cUnion1 = aMembersUnion[1]\n"
            "cUnion5 = aMembersUnion[5]\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton', 'buttons.prg')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native ADDOBJECT deeper external child identity script should complete: ") + state.message +
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

        check("cchildclass", "SaveButton");
        check("cchildbaseclass", "ParentButton");
        check("cchildparentclass", "ParentButton");
        check("cchildclassreflect", "SaveButton");
        check("cchildbaseclassreflect", "ParentButton");
        check("cchildparentclassreflect", "ParentButton");
        check("lchildhasclass", "true");
        check("lchildhasbaseclass", "true");
        check("lchildhasparentclass", "true");
        check("lchildhasclasslibrary", "false");
        check("lchildclassreadonly", "true");
        check("lchildbaseclassreadonly", "true");
        check("lchildparentclassreadonly", "true");
        check("nmembersprops", "5");
        check("nmembersunion", "5");
        check("cprop1", "BASECLASS");
        check("cprop2", "CAPTION");
        check("cprop3", "CLASS");
        check("cprop4", "PARENT");
        check("cprop5", "PARENTCLASS");
        check("cunion1", "BASECLASS");
        check("cunion5", "PARENTCLASS");
        check("ldictset", "true");
        check("ndictcompare", "70");

        const auto child_class_library = state.globals.find("xchildclasslibrary");
        expect(child_class_library != state.globals.end() &&
                   child_class_library->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native ADDOBJECT deeper external child identity should leave the derived child ClassLibrary empty through ordinary reads");
        const auto child_class_library_reflect = state.globals.find("xchildclasslibraryreflect");
        expect(child_class_library_reflect != state.globals.end() &&
                   child_class_library_reflect->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native ADDOBJECT deeper external child identity should leave the derived child ClassLibrary empty through GETPEM");

        expect(state.ole_objects.size() == 3U,
               "native ADDOBJECT deeper external child identity should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &parent_object = state.ole_objects[0];
            const auto &child_object = state.ole_objects[1];
            expect(parent_object.prog_id == "DemoForm",
                   "native ADDOBJECT deeper external child identity should preserve parent identity");
            expect(child_object.prog_id == "SaveButton",
                   "native ADDOBJECT deeper external child identity should preserve child identity");
            expect(child_object.base_class_name == "ParentButton",
                   "native ADDOBJECT deeper external child identity should preserve the immediate child base-class identity");
            expect(child_object.source == button_library_path.string(),
                   "native ADDOBJECT deeper external child identity should preserve the child definition source path");
            expect(child_object.class_library.empty(),
                   "native ADDOBJECT deeper external child identity should leave the derived child ClassLibrary empty");
            expect(child_object.class_hierarchy.size() == 5U,
                   "native ADDOBJECT deeper external child identity should preserve the deeper runtime child class hierarchy");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native ADDOBJECT deeper external child identity lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_inherited_external_base_addobject_deeper_external_child_identity_surfaces_stay_coherent()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_base_inherited_addobject_deeper_external_child_identity";
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
            "ENDDEFINE\n");

        const fs::path widget_library_path = temp_root / "widgetlib.prg";
        write_text(
            widget_library_path,
            "DEFINE CLASS ParentForm AS Custom\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton', 'buttons.prg')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_base_inherited_addobject_deeper_external_child_identity.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oChild = oCreate.cmdSave\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 71)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "cChildClass = oChild.Class\n"
            "cChildBaseClass = oChild.BaseClass\n"
            "cChildParentClass = oChild.ParentClass\n"
            "xChildClassLibrary = oChild.ClassLibrary\n"
            "cChildClassReflect = GETPEM(oChild, 'Class')\n"
            "cChildBaseClassReflect = GETPEM(oChild, 'BaseClass')\n"
            "cChildParentClassReflect = GETPEM(oChild, 'ParentClass')\n"
            "xChildClassLibraryReflect = GETPEM(oChild, 'ClassLibrary')\n"
            "lChildHasClass = PEMSTATUS(oChild, 'Class', 1)\n"
            "lChildHasBaseClass = PEMSTATUS(oChild, 'BaseClass', 1)\n"
            "lChildHasParentClass = PEMSTATUS(oChild, 'ParentClass', 1)\n"
            "lChildHasClassLibrary = PEMSTATUS(oChild, 'ClassLibrary', 1)\n"
            "lChildClassReadOnly = PEMSTATUS(oChild, 'Class', 5)\n"
            "lChildBaseClassReadOnly = PEMSTATUS(oChild, 'BaseClass', 5)\n"
            "lChildParentClassReadOnly = PEMSTATUS(oChild, 'ParentClass', 5)\n"
            "nMembersProps = AMEMBERS(aMembersProps, oChild, 1)\n"
            "nMembersUnion = AMEMBERS(aMembersUnion, oChild, 3)\n"
            "cProp1 = aMembersProps[1]\n"
            "cProp2 = aMembersProps[2]\n"
            "cProp3 = aMembersProps[3]\n"
            "cProp4 = aMembersProps[4]\n"
            "cProp5 = aMembersProps[5]\n"
            "cUnion1 = aMembersUnion[1]\n"
            "cUnion5 = aMembersUnion[5]\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external-base inherited ADDOBJECT deeper external child identity script should complete: ") + state.message +
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

        check("cchildclass", "SaveButton");
        check("cchildbaseclass", "ParentButton");
        check("cchildparentclass", "ParentButton");
        check("cchildclassreflect", "SaveButton");
        check("cchildbaseclassreflect", "ParentButton");
        check("cchildparentclassreflect", "ParentButton");
        check("lchildhasclass", "true");
        check("lchildhasbaseclass", "true");
        check("lchildhasparentclass", "true");
        check("lchildhasclasslibrary", "false");
        check("lchildclassreadonly", "true");
        check("lchildbaseclassreadonly", "true");
        check("lchildparentclassreadonly", "true");
        check("nmembersprops", "5");
        check("nmembersunion", "5");
        check("cprop1", "BASECLASS");
        check("cprop2", "CAPTION");
        check("cprop3", "CLASS");
        check("cprop4", "PARENT");
        check("cprop5", "PARENTCLASS");
        check("cunion1", "BASECLASS");
        check("cunion5", "PARENTCLASS");
        check("ldictset", "true");
        check("ndictcompare", "71");

        const auto child_class_library = state.globals.find("xchildclasslibrary");
        expect(child_class_library != state.globals.end() &&
                   child_class_library->second.kind == copperfin::runtime::PrgValueKind::empty,
               "external-base inherited ADDOBJECT deeper external child identity should leave the derived child ClassLibrary empty through ordinary reads");
        const auto child_class_library_reflect = state.globals.find("xchildclasslibraryreflect");
        expect(child_class_library_reflect != state.globals.end() &&
                   child_class_library_reflect->second.kind == copperfin::runtime::PrgValueKind::empty,
               "external-base inherited ADDOBJECT deeper external child identity should leave the derived child ClassLibrary empty through GETPEM");

        expect(state.ole_objects.size() == 3U,
               "external-base inherited ADDOBJECT deeper external child identity should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &parent_object = state.ole_objects[0];
            const auto &child_object = state.ole_objects[1];
            expect(parent_object.prog_id == "ChildForm",
                   "external-base inherited ADDOBJECT deeper external child identity should preserve parent identity");
            expect(child_object.prog_id == "SaveButton",
                   "external-base inherited ADDOBJECT deeper external child identity should preserve child identity");
            expect(child_object.base_class_name == "ParentButton",
                   "external-base inherited ADDOBJECT deeper external child identity should preserve the immediate child base-class identity");
            expect(child_object.source == button_library_path.string(),
                   "external-base inherited ADDOBJECT deeper external child identity should preserve the child definition source path");
            expect(child_object.class_library.empty(),
                   "external-base inherited ADDOBJECT deeper external child identity should leave the derived child ClassLibrary empty");
            expect(child_object.class_hierarchy.size() == 5U,
                   "external-base inherited ADDOBJECT deeper external child identity should preserve the deeper runtime child class hierarchy");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external-base inherited ADDOBJECT deeper external child identity lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_native_addobject_deeper_external_child_identity_metadata_stays_read_only_to_setpem()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_addobject_deeper_external_child_identity_setpem";
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
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "native_addobject_deeper_external_child_identity_setpem.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('DemoForm')\n"
            "oChild = oForm.cmdSave\n"
            "lChildClassReadOnly = PEMSTATUS(oChild, 'Class', 5)\n"
            "lChildBaseClassReadOnly = PEMSTATUS(oChild, 'BaseClass', 5)\n"
            "lChildParentClassReadOnly = PEMSTATUS(oChild, 'ParentClass', 5)\n"
            "lSetChildClass = SETPEM(oChild, 'Class', 'OtherClass')\n"
            "lSetChildBaseClass = SETPEM(oChild, 'BaseClass', 'OtherBase')\n"
            "lSetChildParentClass = SETPEM(oChild, 'ParentClass', 'OtherParent')\n"
            "lSetChildClassLibrary = SETPEM(oChild, 'ClassLibrary', 'other.prg')\n"
            "cChildClassAfter = GETPEM(oChild, 'Class')\n"
            "cChildBaseClassAfter = GETPEM(oChild, 'BaseClass')\n"
            "cChildParentClassAfter = GETPEM(oChild, 'ParentClass')\n"
            "xChildClassLibraryAfter = GETPEM(oChild, 'ClassLibrary')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 72)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton', 'buttons.prg')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native ADDOBJECT deeper external child identity SETPEM script should complete: ") + state.message +
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

        check("lchildclassreadonly", "true");
        check("lchildbaseclassreadonly", "true");
        check("lchildparentclassreadonly", "true");
        check("lsetchildclass", "false");
        check("lsetchildbaseclass", "false");
        check("lsetchildparentclass", "false");
        check("lsetchildclasslibrary", "false");
        check("cchildclassafter", "SaveButton");
        check("cchildbaseclassafter", "ParentButton");
        check("cchildparentclassafter", "ParentButton");
        check("ldictset", "true");
        check("ndictcompare", "72");

        const auto child_class_library_after = state.globals.find("xchildclasslibraryafter");
        expect(child_class_library_after != state.globals.end() &&
                   child_class_library_after->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native ADDOBJECT deeper external child identity SETPEM should keep the derived child ClassLibrary empty after failed mutation");

        expect(state.ole_objects.size() == 3U,
               "native ADDOBJECT deeper external child identity SETPEM should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &parent_object = state.ole_objects[0];
            const auto &child_object = state.ole_objects[1];
            expect(parent_object.prog_id == "DemoForm",
                   "native ADDOBJECT deeper external child identity SETPEM should preserve parent identity");
            expect(child_object.prog_id == "SaveButton",
                   "native ADDOBJECT deeper external child identity SETPEM should preserve child identity");
            expect(child_object.base_class_name == "ParentButton",
                   "native ADDOBJECT deeper external child identity SETPEM should preserve the immediate child base-class identity");
            expect(child_object.source == button_library_path.string(),
                   "native ADDOBJECT deeper external child identity SETPEM should preserve the child definition source path");
            expect(child_object.class_library.empty(),
                   "native ADDOBJECT deeper external child identity SETPEM should keep the derived child ClassLibrary empty");
            expect(child_object.class_hierarchy.size() == 5U,
                   "native ADDOBJECT deeper external child identity SETPEM should preserve the deeper runtime child class hierarchy");
            expect(!child_object.properties.contains("class"),
                   "native ADDOBJECT deeper external child identity SETPEM should not materialize a writable child Class shadow");
            expect(!child_object.properties.contains("baseclass"),
                   "native ADDOBJECT deeper external child identity SETPEM should not materialize a writable child BaseClass shadow");
            expect(!child_object.properties.contains("parentclass"),
                   "native ADDOBJECT deeper external child identity SETPEM should not materialize a writable child ParentClass shadow");
            expect(!child_object.properties.contains("classlibrary"),
                   "native ADDOBJECT deeper external child identity SETPEM should not materialize a writable child ClassLibrary shadow");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native ADDOBJECT deeper external child identity SETPEM lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_inherited_external_base_addobject_deeper_external_child_identity_metadata_stays_read_only_to_setpem()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_base_inherited_addobject_deeper_external_child_identity_setpem";
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
            "ENDDEFINE\n");

        const fs::path widget_library_path = temp_root / "widgetlib.prg";
        write_text(
            widget_library_path,
            "DEFINE CLASS ParentForm AS Custom\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton', 'buttons.prg')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_base_inherited_addobject_deeper_external_child_identity_setpem.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oChild = oCreate.cmdSave\n"
            "lChildClassReadOnly = PEMSTATUS(oChild, 'Class', 5)\n"
            "lChildBaseClassReadOnly = PEMSTATUS(oChild, 'BaseClass', 5)\n"
            "lChildParentClassReadOnly = PEMSTATUS(oChild, 'ParentClass', 5)\n"
            "lSetChildClass = SETPEM(oChild, 'Class', 'OtherClass')\n"
            "lSetChildBaseClass = SETPEM(oChild, 'BaseClass', 'OtherBase')\n"
            "lSetChildParentClass = SETPEM(oChild, 'ParentClass', 'OtherParent')\n"
            "lSetChildClassLibrary = SETPEM(oChild, 'ClassLibrary', 'other.prg')\n"
            "cChildClassAfter = GETPEM(oChild, 'Class')\n"
            "cChildBaseClassAfter = GETPEM(oChild, 'BaseClass')\n"
            "cChildParentClassAfter = GETPEM(oChild, 'ParentClass')\n"
            "xChildClassLibraryAfter = GETPEM(oChild, 'ClassLibrary')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 73)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external-base inherited ADDOBJECT deeper external child identity SETPEM script should complete: ") + state.message +
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

        check("lchildclassreadonly", "true");
        check("lchildbaseclassreadonly", "true");
        check("lchildparentclassreadonly", "true");
        check("lsetchildclass", "false");
        check("lsetchildbaseclass", "false");
        check("lsetchildparentclass", "false");
        check("lsetchildclasslibrary", "false");
        check("cchildclassafter", "SaveButton");
        check("cchildbaseclassafter", "ParentButton");
        check("cchildparentclassafter", "ParentButton");
        check("ldictset", "true");
        check("ndictcompare", "73");

        const auto child_class_library_after = state.globals.find("xchildclasslibraryafter");
        expect(child_class_library_after != state.globals.end() &&
                   child_class_library_after->second.kind == copperfin::runtime::PrgValueKind::empty,
               "external-base inherited ADDOBJECT deeper external child identity SETPEM should keep the derived child ClassLibrary empty after failed mutation");

        expect(state.ole_objects.size() == 3U,
               "external-base inherited ADDOBJECT deeper external child identity SETPEM should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &parent_object = state.ole_objects[0];
            const auto &child_object = state.ole_objects[1];
            expect(parent_object.prog_id == "ChildForm",
                   "external-base inherited ADDOBJECT deeper external child identity SETPEM should preserve parent identity");
            expect(child_object.prog_id == "SaveButton",
                   "external-base inherited ADDOBJECT deeper external child identity SETPEM should preserve child identity");
            expect(child_object.base_class_name == "ParentButton",
                   "external-base inherited ADDOBJECT deeper external child identity SETPEM should preserve the immediate child base-class identity");
            expect(child_object.source == button_library_path.string(),
                   "external-base inherited ADDOBJECT deeper external child identity SETPEM should preserve the child definition source path");
            expect(child_object.class_library.empty(),
                   "external-base inherited ADDOBJECT deeper external child identity SETPEM should keep the derived child ClassLibrary empty");
            expect(child_object.class_hierarchy.size() == 5U,
                   "external-base inherited ADDOBJECT deeper external child identity SETPEM should preserve the deeper runtime child class hierarchy");
            expect(!child_object.properties.contains("class"),
                   "external-base inherited ADDOBJECT deeper external child identity SETPEM should not materialize a writable child Class shadow");
            expect(!child_object.properties.contains("baseclass"),
                   "external-base inherited ADDOBJECT deeper external child identity SETPEM should not materialize a writable child BaseClass shadow");
            expect(!child_object.properties.contains("parentclass"),
                   "external-base inherited ADDOBJECT deeper external child identity SETPEM should not materialize a writable child ParentClass shadow");
            expect(!child_object.properties.contains("classlibrary"),
                   "external-base inherited ADDOBJECT deeper external child identity SETPEM should not materialize a writable child ClassLibrary shadow");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external-base inherited ADDOBJECT deeper external child identity SETPEM lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_native_addobject_deeper_external_child_identity_metadata_cannot_be_shadowed_through_addproperty()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_addobject_deeper_external_child_identity_addproperty";
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
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "native_addobject_deeper_external_child_identity_addproperty.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('DemoForm')\n"
            "oChild = oForm.cmdSave\n"
            "lAddChildClass = ADDPROPERTY(oChild, 'Class', 'OtherClass')\n"
            "lAddChildBaseClass = ADDPROPERTY(oChild, 'BaseClass', 'OtherBase')\n"
            "lAddChildParentClass = ADDPROPERTY(oChild, 'ParentClass', 'OtherParent')\n"
            "lAddChildClassLibrary = ADDPROPERTY(oChild, 'ClassLibrary', 'other.prg')\n"
            "cChildClassAfter = GETPEM(oChild, 'Class')\n"
            "cChildBaseClassAfter = GETPEM(oChild, 'BaseClass')\n"
            "cChildParentClassAfter = GETPEM(oChild, 'ParentClass')\n"
            "xChildClassLibraryAfter = GETPEM(oChild, 'ClassLibrary')\n"
            "lChildClassReadOnly = PEMSTATUS(oChild, 'Class', 5)\n"
            "lChildBaseClassReadOnly = PEMSTATUS(oChild, 'BaseClass', 5)\n"
            "lChildParentClassReadOnly = PEMSTATUS(oChild, 'ParentClass', 5)\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 74)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton', 'buttons.prg')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native ADDOBJECT deeper external child identity ADDPROPERTY script should complete: ") + state.message +
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

        check("laddchildclass", "false");
        check("laddchildbaseclass", "false");
        check("laddchildparentclass", "false");
        check("laddchildclasslibrary", "false");
        check("cchildclassafter", "SaveButton");
        check("cchildbaseclassafter", "ParentButton");
        check("cchildparentclassafter", "ParentButton");
        check("lchildclassreadonly", "true");
        check("lchildbaseclassreadonly", "true");
        check("lchildparentclassreadonly", "true");
        check("ldictset", "true");
        check("ndictcompare", "74");

        const auto child_class_library_after = state.globals.find("xchildclasslibraryafter");
        expect(child_class_library_after != state.globals.end() &&
                   child_class_library_after->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native ADDOBJECT deeper external child identity ADDPROPERTY should keep the derived child ClassLibrary empty after failed shadow creation");

        expect(state.ole_objects.size() == 3U,
               "native ADDOBJECT deeper external child identity ADDPROPERTY should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &parent_object = state.ole_objects[0];
            const auto &child_object = state.ole_objects[1];
            expect(parent_object.prog_id == "DemoForm",
                   "native ADDOBJECT deeper external child identity ADDPROPERTY should preserve parent identity");
            expect(child_object.prog_id == "SaveButton",
                   "native ADDOBJECT deeper external child identity ADDPROPERTY should preserve child identity");
            expect(child_object.base_class_name == "ParentButton",
                   "native ADDOBJECT deeper external child identity ADDPROPERTY should preserve the immediate child base-class identity");
            expect(child_object.source == button_library_path.string(),
                   "native ADDOBJECT deeper external child identity ADDPROPERTY should preserve the child definition source path");
            expect(child_object.class_library.empty(),
                   "native ADDOBJECT deeper external child identity ADDPROPERTY should keep the derived child ClassLibrary empty");
            expect(child_object.class_hierarchy.size() == 5U,
                   "native ADDOBJECT deeper external child identity ADDPROPERTY should preserve the deeper runtime child class hierarchy");
            expect(!child_object.properties.contains("class"),
                   "native ADDOBJECT deeper external child identity ADDPROPERTY should not materialize a child Class shadow");
            expect(!child_object.properties.contains("baseclass"),
                   "native ADDOBJECT deeper external child identity ADDPROPERTY should not materialize a child BaseClass shadow");
            expect(!child_object.properties.contains("parentclass"),
                   "native ADDOBJECT deeper external child identity ADDPROPERTY should not materialize a child ParentClass shadow");
            expect(!child_object.properties.contains("classlibrary"),
                   "native ADDOBJECT deeper external child identity ADDPROPERTY should not materialize a child ClassLibrary shadow");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native ADDOBJECT deeper external child identity ADDPROPERTY lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_inherited_external_base_addobject_deeper_external_child_identity_metadata_cannot_be_shadowed_through_addproperty()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_base_inherited_addobject_deeper_external_child_identity_addproperty";
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
            "ENDDEFINE\n");

        const fs::path widget_library_path = temp_root / "widgetlib.prg";
        write_text(
            widget_library_path,
            "DEFINE CLASS ParentForm AS Custom\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton', 'buttons.prg')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_base_inherited_addobject_deeper_external_child_identity_addproperty.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oChild = oCreate.cmdSave\n"
            "lAddChildClass = ADDPROPERTY(oChild, 'Class', 'OtherClass')\n"
            "lAddChildBaseClass = ADDPROPERTY(oChild, 'BaseClass', 'OtherBase')\n"
            "lAddChildParentClass = ADDPROPERTY(oChild, 'ParentClass', 'OtherParent')\n"
            "lAddChildClassLibrary = ADDPROPERTY(oChild, 'ClassLibrary', 'other.prg')\n"
            "cChildClassAfter = GETPEM(oChild, 'Class')\n"
            "cChildBaseClassAfter = GETPEM(oChild, 'BaseClass')\n"
            "cChildParentClassAfter = GETPEM(oChild, 'ParentClass')\n"
            "xChildClassLibraryAfter = GETPEM(oChild, 'ClassLibrary')\n"
            "lChildClassReadOnly = PEMSTATUS(oChild, 'Class', 5)\n"
            "lChildBaseClassReadOnly = PEMSTATUS(oChild, 'BaseClass', 5)\n"
            "lChildParentClassReadOnly = PEMSTATUS(oChild, 'ParentClass', 5)\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 75)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external-base inherited ADDOBJECT deeper external child identity ADDPROPERTY script should complete: ") + state.message +
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

        check("laddchildclass", "false");
        check("laddchildbaseclass", "false");
        check("laddchildparentclass", "false");
        check("laddchildclasslibrary", "false");
        check("cchildclassafter", "SaveButton");
        check("cchildbaseclassafter", "ParentButton");
        check("cchildparentclassafter", "ParentButton");
        check("lchildclassreadonly", "true");
        check("lchildbaseclassreadonly", "true");
        check("lchildparentclassreadonly", "true");
        check("ldictset", "true");
        check("ndictcompare", "75");

        const auto child_class_library_after = state.globals.find("xchildclasslibraryafter");
        expect(child_class_library_after != state.globals.end() &&
                   child_class_library_after->second.kind == copperfin::runtime::PrgValueKind::empty,
               "external-base inherited ADDOBJECT deeper external child identity ADDPROPERTY should keep the derived child ClassLibrary empty after failed shadow creation");

        expect(state.ole_objects.size() == 3U,
               "external-base inherited ADDOBJECT deeper external child identity ADDPROPERTY should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &parent_object = state.ole_objects[0];
            const auto &child_object = state.ole_objects[1];
            expect(parent_object.prog_id == "ChildForm",
                   "external-base inherited ADDOBJECT deeper external child identity ADDPROPERTY should preserve parent identity");
            expect(child_object.prog_id == "SaveButton",
                   "external-base inherited ADDOBJECT deeper external child identity ADDPROPERTY should preserve child identity");
            expect(child_object.base_class_name == "ParentButton",
                   "external-base inherited ADDOBJECT deeper external child identity ADDPROPERTY should preserve the immediate child base-class identity");
            expect(child_object.source == button_library_path.string(),
                   "external-base inherited ADDOBJECT deeper external child identity ADDPROPERTY should preserve the child definition source path");
            expect(child_object.class_library.empty(),
                   "external-base inherited ADDOBJECT deeper external child identity ADDPROPERTY should keep the derived child ClassLibrary empty");
            expect(child_object.class_hierarchy.size() == 5U,
                   "external-base inherited ADDOBJECT deeper external child identity ADDPROPERTY should preserve the deeper runtime child class hierarchy");
            expect(!child_object.properties.contains("class"),
                   "external-base inherited ADDOBJECT deeper external child identity ADDPROPERTY should not materialize a child Class shadow");
            expect(!child_object.properties.contains("baseclass"),
                   "external-base inherited ADDOBJECT deeper external child identity ADDPROPERTY should not materialize a child BaseClass shadow");
            expect(!child_object.properties.contains("parentclass"),
                   "external-base inherited ADDOBJECT deeper external child identity ADDPROPERTY should not materialize a child ParentClass shadow");
            expect(!child_object.properties.contains("classlibrary"),
                   "external-base inherited ADDOBJECT deeper external child identity ADDPROPERTY should not materialize a child ClassLibrary shadow");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external-base inherited ADDOBJECT deeper external child identity ADDPROPERTY lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_native_addobject_deeper_external_child_identity_metadata_cannot_be_erased_through_removeproperty()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_addobject_deeper_external_child_identity_removeproperty";
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
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "native_addobject_deeper_external_child_identity_removeproperty.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('DemoForm')\n"
            "oChild = oForm.cmdSave\n"
            "lRemoveChildClass = REMOVEPROPERTY(oChild, 'Class')\n"
            "lRemoveChildBaseClass = REMOVEPROPERTY(oChild, 'BaseClass')\n"
            "lRemoveChildParentClass = REMOVEPROPERTY(oChild, 'ParentClass')\n"
            "lRemoveChildClassLibrary = REMOVEPROPERTY(oChild, 'ClassLibrary')\n"
            "cChildClassAfter = GETPEM(oChild, 'Class')\n"
            "cChildBaseClassAfter = GETPEM(oChild, 'BaseClass')\n"
            "cChildParentClassAfter = GETPEM(oChild, 'ParentClass')\n"
            "xChildClassLibraryAfter = GETPEM(oChild, 'ClassLibrary')\n"
            "lChildClassReadOnly = PEMSTATUS(oChild, 'Class', 5)\n"
            "lChildBaseClassReadOnly = PEMSTATUS(oChild, 'BaseClass', 5)\n"
            "lChildParentClassReadOnly = PEMSTATUS(oChild, 'ParentClass', 5)\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 76)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton', 'buttons.prg')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native ADDOBJECT deeper external child identity REMOVEPROPERTY script should complete: ") + state.message +
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

        check("lremovechildclass", "false");
        check("lremovechildbaseclass", "false");
        check("lremovechildparentclass", "false");
        check("lremovechildclasslibrary", "false");
        check("cchildclassafter", "SaveButton");
        check("cchildbaseclassafter", "ParentButton");
        check("cchildparentclassafter", "ParentButton");
        check("lchildclassreadonly", "true");
        check("lchildbaseclassreadonly", "true");
        check("lchildparentclassreadonly", "true");
        check("ldictset", "true");
        check("ndictcompare", "76");

        const auto child_class_library_after = state.globals.find("xchildclasslibraryafter");
        expect(child_class_library_after != state.globals.end() &&
                   child_class_library_after->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native ADDOBJECT deeper external child identity REMOVEPROPERTY should keep the derived child ClassLibrary empty after failed removal");

        expect(state.ole_objects.size() == 3U,
               "native ADDOBJECT deeper external child identity REMOVEPROPERTY should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &parent_object = state.ole_objects[0];
            const auto &child_object = state.ole_objects[1];
            expect(parent_object.prog_id == "DemoForm",
                   "native ADDOBJECT deeper external child identity REMOVEPROPERTY should preserve parent identity");
            expect(child_object.prog_id == "SaveButton",
                   "native ADDOBJECT deeper external child identity REMOVEPROPERTY should preserve child identity");
            expect(child_object.base_class_name == "ParentButton",
                   "native ADDOBJECT deeper external child identity REMOVEPROPERTY should preserve the immediate child base-class identity");
            expect(child_object.source == button_library_path.string(),
                   "native ADDOBJECT deeper external child identity REMOVEPROPERTY should preserve the child definition source path");
            expect(child_object.class_library.empty(),
                   "native ADDOBJECT deeper external child identity REMOVEPROPERTY should keep the derived child ClassLibrary empty");
            expect(child_object.class_hierarchy.size() == 5U,
                   "native ADDOBJECT deeper external child identity REMOVEPROPERTY should preserve the deeper runtime child class hierarchy");
            expect(!child_object.properties.contains("class"),
                   "native ADDOBJECT deeper external child identity REMOVEPROPERTY should not materialize a child Class shadow");
            expect(!child_object.properties.contains("baseclass"),
                   "native ADDOBJECT deeper external child identity REMOVEPROPERTY should not materialize a child BaseClass shadow");
            expect(!child_object.properties.contains("parentclass"),
                   "native ADDOBJECT deeper external child identity REMOVEPROPERTY should not materialize a child ParentClass shadow");
            expect(!child_object.properties.contains("classlibrary"),
                   "native ADDOBJECT deeper external child identity REMOVEPROPERTY should not materialize a child ClassLibrary shadow");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native ADDOBJECT deeper external child identity REMOVEPROPERTY lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_inherited_external_base_addobject_deeper_external_child_identity_metadata_cannot_be_erased_through_removeproperty()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_base_inherited_addobject_deeper_external_child_identity_removeproperty";
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
            "ENDDEFINE\n");

        const fs::path widget_library_path = temp_root / "widgetlib.prg";
        write_text(
            widget_library_path,
            "DEFINE CLASS ParentForm AS Custom\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton', 'buttons.prg')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_base_inherited_addobject_deeper_external_child_identity_removeproperty.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oChild = oCreate.cmdSave\n"
            "lRemoveChildClass = REMOVEPROPERTY(oChild, 'Class')\n"
            "lRemoveChildBaseClass = REMOVEPROPERTY(oChild, 'BaseClass')\n"
            "lRemoveChildParentClass = REMOVEPROPERTY(oChild, 'ParentClass')\n"
            "lRemoveChildClassLibrary = REMOVEPROPERTY(oChild, 'ClassLibrary')\n"
            "cChildClassAfter = GETPEM(oChild, 'Class')\n"
            "cChildBaseClassAfter = GETPEM(oChild, 'BaseClass')\n"
            "cChildParentClassAfter = GETPEM(oChild, 'ParentClass')\n"
            "xChildClassLibraryAfter = GETPEM(oChild, 'ClassLibrary')\n"
            "lChildClassReadOnly = PEMSTATUS(oChild, 'Class', 5)\n"
            "lChildBaseClassReadOnly = PEMSTATUS(oChild, 'BaseClass', 5)\n"
            "lChildParentClassReadOnly = PEMSTATUS(oChild, 'ParentClass', 5)\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 77)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external-base inherited ADDOBJECT deeper external child identity REMOVEPROPERTY script should complete: ") + state.message +
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

        check("lremovechildclass", "false");
        check("lremovechildbaseclass", "false");
        check("lremovechildparentclass", "false");
        check("lremovechildclasslibrary", "false");
        check("cchildclassafter", "SaveButton");
        check("cchildbaseclassafter", "ParentButton");
        check("cchildparentclassafter", "ParentButton");
        check("lchildclassreadonly", "true");
        check("lchildbaseclassreadonly", "true");
        check("lchildparentclassreadonly", "true");
        check("ldictset", "true");
        check("ndictcompare", "77");

        const auto child_class_library_after = state.globals.find("xchildclasslibraryafter");
        expect(child_class_library_after != state.globals.end() &&
                   child_class_library_after->second.kind == copperfin::runtime::PrgValueKind::empty,
               "external-base inherited ADDOBJECT deeper external child identity REMOVEPROPERTY should keep the derived child ClassLibrary empty after failed removal");

        expect(state.ole_objects.size() == 3U,
               "external-base inherited ADDOBJECT deeper external child identity REMOVEPROPERTY should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &parent_object = state.ole_objects[0];
            const auto &child_object = state.ole_objects[1];
            expect(parent_object.prog_id == "ChildForm",
                   "external-base inherited ADDOBJECT deeper external child identity REMOVEPROPERTY should preserve parent identity");
            expect(child_object.prog_id == "SaveButton",
                   "external-base inherited ADDOBJECT deeper external child identity REMOVEPROPERTY should preserve child identity");
            expect(child_object.base_class_name == "ParentButton",
                   "external-base inherited ADDOBJECT deeper external child identity REMOVEPROPERTY should preserve the immediate child base-class identity");
            expect(child_object.source == button_library_path.string(),
                   "external-base inherited ADDOBJECT deeper external child identity REMOVEPROPERTY should preserve the child definition source path");
            expect(child_object.class_library.empty(),
                   "external-base inherited ADDOBJECT deeper external child identity REMOVEPROPERTY should keep the derived child ClassLibrary empty");
            expect(child_object.class_hierarchy.size() == 5U,
                   "external-base inherited ADDOBJECT deeper external child identity REMOVEPROPERTY should preserve the deeper runtime child class hierarchy");
            expect(!child_object.properties.contains("class"),
                   "external-base inherited ADDOBJECT deeper external child identity REMOVEPROPERTY should not materialize a child Class shadow");
            expect(!child_object.properties.contains("baseclass"),
                   "external-base inherited ADDOBJECT deeper external child identity REMOVEPROPERTY should not materialize a child BaseClass shadow");
            expect(!child_object.properties.contains("parentclass"),
                   "external-base inherited ADDOBJECT deeper external child identity REMOVEPROPERTY should not materialize a child ParentClass shadow");
            expect(!child_object.properties.contains("classlibrary"),
                   "external-base inherited ADDOBJECT deeper external child identity REMOVEPROPERTY should not materialize a child ClassLibrary shadow");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external-base inherited ADDOBJECT deeper external child identity REMOVEPROPERTY lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_native_addobject_deeper_external_child_identity_metadata_cannot_be_shadowed_through_direct_assignment()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_addobject_deeper_external_child_identity_direct_assignment";
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
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "native_addobject_deeper_external_child_identity_direct_assignment.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('DemoForm')\n"
            "oChild = oForm.cmdSave\n"
            "oChild.Class = 'OtherClass'\n"
            "oChild.BaseClass = 'OtherBase'\n"
            "oChild.ParentClass = 'OtherParent'\n"
            "oChild.ClassLibrary = 'other.prg'\n"
            "cChildClassAfter = GETPEM(oChild, 'Class')\n"
            "cChildBaseClassAfter = GETPEM(oChild, 'BaseClass')\n"
            "cChildParentClassAfter = GETPEM(oChild, 'ParentClass')\n"
            "xChildClassLibraryAfter = GETPEM(oChild, 'ClassLibrary')\n"
            "lChildClassReadOnly = PEMSTATUS(oChild, 'Class', 5)\n"
            "lChildBaseClassReadOnly = PEMSTATUS(oChild, 'BaseClass', 5)\n"
            "lChildParentClassReadOnly = PEMSTATUS(oChild, 'ParentClass', 5)\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 78)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton', 'buttons.prg')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native ADDOBJECT deeper external child identity direct-assignment script should complete: ") + state.message +
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

        check("cchildclassafter", "SaveButton");
        check("cchildbaseclassafter", "ParentButton");
        check("cchildparentclassafter", "ParentButton");
        check("lchildclassreadonly", "true");
        check("lchildbaseclassreadonly", "true");
        check("lchildparentclassreadonly", "true");
        check("ldictset", "true");
        check("ndictcompare", "78");

        const auto child_class_library_after = state.globals.find("xchildclasslibraryafter");
        expect(child_class_library_after != state.globals.end() &&
                   child_class_library_after->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native ADDOBJECT deeper external child identity direct assignment should keep the derived child ClassLibrary empty");

        expect(state.ole_objects.size() == 3U,
               "native ADDOBJECT deeper external child identity direct assignment should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &parent_object = state.ole_objects[0];
            const auto &child_object = state.ole_objects[1];
            expect(parent_object.prog_id == "DemoForm",
                   "native ADDOBJECT deeper external child identity direct assignment should preserve parent identity");
            expect(child_object.prog_id == "SaveButton",
                   "native ADDOBJECT deeper external child identity direct assignment should preserve child identity");
            expect(child_object.base_class_name == "ParentButton",
                   "native ADDOBJECT deeper external child identity direct assignment should preserve the immediate child base-class identity");
            expect(child_object.source == button_library_path.string(),
                   "native ADDOBJECT deeper external child identity direct assignment should preserve the child definition source path");
            expect(child_object.class_library.empty(),
                   "native ADDOBJECT deeper external child identity direct assignment should keep the derived child ClassLibrary empty");
            expect(child_object.class_hierarchy.size() == 5U,
                   "native ADDOBJECT deeper external child identity direct assignment should preserve the deeper runtime child class hierarchy");
            expect(!child_object.properties.contains("class"),
                   "native ADDOBJECT deeper external child identity direct assignment should not materialize a child Class shadow property");
            expect(!child_object.properties.contains("baseclass"),
                   "native ADDOBJECT deeper external child identity direct assignment should not materialize a child BaseClass shadow property");
            expect(!child_object.properties.contains("parentclass"),
                   "native ADDOBJECT deeper external child identity direct assignment should not materialize a child ParentClass shadow property");
            expect(!child_object.properties.contains("classlibrary"),
                   "native ADDOBJECT deeper external child identity direct assignment should not materialize a child ClassLibrary shadow property");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native ADDOBJECT deeper external child identity direct assignment lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_inherited_external_base_addobject_deeper_external_child_identity_metadata_cannot_be_shadowed_through_direct_assignment()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_base_inherited_addobject_deeper_external_child_identity_direct_assignment";
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
            "ENDDEFINE\n");

        const fs::path widget_library_path = temp_root / "widgetlib.prg";
        write_text(
            widget_library_path,
            "DEFINE CLASS ParentForm AS Custom\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton', 'buttons.prg')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_base_inherited_addobject_deeper_external_child_identity_direct_assignment.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oChild = oCreate.cmdSave\n"
            "oChild.Class = 'OtherClass'\n"
            "oChild.BaseClass = 'OtherBase'\n"
            "oChild.ParentClass = 'OtherParent'\n"
            "oChild.ClassLibrary = 'other.prg'\n"
            "cChildClassAfter = GETPEM(oChild, 'Class')\n"
            "cChildBaseClassAfter = GETPEM(oChild, 'BaseClass')\n"
            "cChildParentClassAfter = GETPEM(oChild, 'ParentClass')\n"
            "xChildClassLibraryAfter = GETPEM(oChild, 'ClassLibrary')\n"
            "lChildClassReadOnly = PEMSTATUS(oChild, 'Class', 5)\n"
            "lChildBaseClassReadOnly = PEMSTATUS(oChild, 'BaseClass', 5)\n"
            "lChildParentClassReadOnly = PEMSTATUS(oChild, 'ParentClass', 5)\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 79)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external-base inherited ADDOBJECT deeper external child identity direct-assignment script should complete: ") + state.message +
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

        check("cchildclassafter", "SaveButton");
        check("cchildbaseclassafter", "ParentButton");
        check("cchildparentclassafter", "ParentButton");
        check("lchildclassreadonly", "true");
        check("lchildbaseclassreadonly", "true");
        check("lchildparentclassreadonly", "true");
        check("ldictset", "true");
        check("ndictcompare", "79");

        const auto child_class_library_after = state.globals.find("xchildclasslibraryafter");
        expect(child_class_library_after != state.globals.end() &&
                   child_class_library_after->second.kind == copperfin::runtime::PrgValueKind::empty,
               "external-base inherited ADDOBJECT deeper external child identity direct assignment should keep the derived child ClassLibrary empty");

        expect(state.ole_objects.size() == 3U,
               "external-base inherited ADDOBJECT deeper external child identity direct assignment should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &parent_object = state.ole_objects[0];
            const auto &child_object = state.ole_objects[1];
            expect(parent_object.prog_id == "ChildForm",
                   "external-base inherited ADDOBJECT deeper external child identity direct assignment should preserve parent identity");
            expect(child_object.prog_id == "SaveButton",
                   "external-base inherited ADDOBJECT deeper external child identity direct assignment should preserve child identity");
            expect(child_object.base_class_name == "ParentButton",
                   "external-base inherited ADDOBJECT deeper external child identity direct assignment should preserve the immediate child base-class identity");
            expect(child_object.source == button_library_path.string(),
                   "external-base inherited ADDOBJECT deeper external child identity direct assignment should preserve the child definition source path");
            expect(child_object.class_library.empty(),
                   "external-base inherited ADDOBJECT deeper external child identity direct assignment should keep the derived child ClassLibrary empty");
            expect(child_object.class_hierarchy.size() == 5U,
                   "external-base inherited ADDOBJECT deeper external child identity direct assignment should preserve the deeper runtime child class hierarchy");
            expect(!child_object.properties.contains("class"),
                   "external-base inherited ADDOBJECT deeper external child identity direct assignment should not materialize a child Class shadow property");
            expect(!child_object.properties.contains("baseclass"),
                   "external-base inherited ADDOBJECT deeper external child identity direct assignment should not materialize a child BaseClass shadow property");
            expect(!child_object.properties.contains("parentclass"),
                   "external-base inherited ADDOBJECT deeper external child identity direct assignment should not materialize a child ParentClass shadow property");
            expect(!child_object.properties.contains("classlibrary"),
                   "external-base inherited ADDOBJECT deeper external child identity direct assignment should not materialize a child ClassLibrary shadow property");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external-base inherited ADDOBJECT deeper external child identity direct assignment lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_native_addobject_deeper_external_child_external_base_provenance_stays_coherent()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_addobject_deeper_external_child_external_base_provenance";
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

        const fs::path main_path = temp_root / "native_addobject_deeper_external_child_external_base_provenance.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('DemoForm')\n"
            "oChild = oForm.cmdSave\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 80)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "cChildClass = oChild.Class\n"
            "cChildBaseClass = oChild.BaseClass\n"
            "cChildParentClass = oChild.ParentClass\n"
            "cChildClassLibrary = oChild.ClassLibrary\n"
            "cChildClassReflect = GETPEM(oChild, 'Class')\n"
            "cChildBaseClassReflect = GETPEM(oChild, 'BaseClass')\n"
            "cChildParentClassReflect = GETPEM(oChild, 'ParentClass')\n"
            "cChildClassLibraryReflect = GETPEM(oChild, 'ClassLibrary')\n"
            "lChildHasClass = PEMSTATUS(oChild, 'Class', 1)\n"
            "lChildHasBaseClass = PEMSTATUS(oChild, 'BaseClass', 1)\n"
            "lChildHasParentClass = PEMSTATUS(oChild, 'ParentClass', 1)\n"
            "lChildHasClassLibrary = PEMSTATUS(oChild, 'ClassLibrary', 1)\n"
            "lChildClassReadOnly = PEMSTATUS(oChild, 'Class', 5)\n"
            "lChildBaseClassReadOnly = PEMSTATUS(oChild, 'BaseClass', 5)\n"
            "lChildParentClassReadOnly = PEMSTATUS(oChild, 'ParentClass', 5)\n"
            "lChildClassLibraryReadOnly = PEMSTATUS(oChild, 'ClassLibrary', 5)\n"
            "nMembersProps = AMEMBERS(aMembersProps, oChild, 1)\n"
            "nMembersUnion = AMEMBERS(aMembersUnion, oChild, 3)\n"
            "cProp1 = aMembersProps[1]\n"
            "cProp2 = aMembersProps[2]\n"
            "cProp3 = aMembersProps[3]\n"
            "cProp4 = aMembersProps[4]\n"
            "cProp5 = aMembersProps[5]\n"
            "cProp6 = aMembersProps[6]\n"
            "cUnion4 = aMembersUnion[4]\n"
            "nClassCount = ACLASS(aClass, oChild)\n"
            "cClass1 = aClass[1]\n"
            "cClass2 = aClass[2]\n"
            "cClass3 = aClass[3]\n"
            "cClass4 = aClass[4]\n"
            "cClass5 = aClass[5]\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS ParentButton OF buttons.prg\n"
            "    Caption = 'Save'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native ADDOBJECT deeper external child external-base provenance script should complete: ") + state.message +
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

        check("cchildclass", "SaveButton");
        check("cchildbaseclass", "ParentButton");
        check("cchildparentclass", "ParentButton");
        check("cchildclasslibrary", button_library_path.string());
        check("cchildclassreflect", "SaveButton");
        check("cchildbaseclassreflect", "ParentButton");
        check("cchildparentclassreflect", "ParentButton");
        check("cchildclasslibraryreflect", button_library_path.string());
        check("lchildhasclass", "true");
        check("lchildhasbaseclass", "true");
        check("lchildhasparentclass", "true");
        check("lchildhasclasslibrary", "true");
        check("lchildclassreadonly", "true");
        check("lchildbaseclassreadonly", "true");
        check("lchildparentclassreadonly", "true");
        check("lchildclasslibraryreadonly", "true");
        check("nmembersprops", "6");
        check("nmembersunion", "6");
        check("cprop1", "BASECLASS");
        check("cprop2", "CAPTION");
        check("cprop3", "CLASS");
        check("cprop4", "CLASSLIBRARY");
        check("cprop5", "PARENT");
        check("cprop6", "PARENTCLASS");
        check("cunion4", "CLASSLIBRARY");
        check("nclasscount", "5");
        check("cclass1", "SAVEBUTTON");
        check("cclass2", "PARENTBUTTON");
        check("cclass3", "ROOTBUTTON");
        check("cclass4", "CUSTOM");
        check("cclass5", "OBJECT");
        check("ldictset", "true");
        check("ndictcompare", "80");

        expect(state.ole_objects.size() == 3U,
               "native ADDOBJECT deeper external child external-base provenance should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &parent_object = state.ole_objects[0];
            const auto &child_object = state.ole_objects[1];
            expect(parent_object.prog_id == "DemoForm",
                   "native ADDOBJECT deeper external child external-base provenance should preserve parent identity");
            expect(child_object.prog_id == "SaveButton",
                   "native ADDOBJECT deeper external child external-base provenance should preserve child identity");
            expect(child_object.base_class_name == "ParentButton",
                   "native ADDOBJECT deeper external child external-base provenance should preserve immediate child base-class identity");
            expect(child_object.source == main_path.string(),
                   "native ADDOBJECT deeper external child external-base provenance should preserve the defining child-class source path");
            expect(child_object.class_library == button_library_path.string(),
                   "native ADDOBJECT deeper external child external-base provenance should preserve the immediate external ClassLibrary path");
            expect(child_object.class_hierarchy.size() == 5U,
                   "native ADDOBJECT deeper external child external-base provenance should preserve the deeper runtime child class hierarchy");
            expect(!child_object.properties.contains("classlibrary"),
                   "native ADDOBJECT deeper external child external-base provenance should not materialize a child ClassLibrary shadow");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native ADDOBJECT deeper external child external-base provenance lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_inherited_external_base_addobject_deeper_external_child_external_base_provenance_stays_coherent()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_base_inherited_addobject_deeper_external_child_external_base_provenance";
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

        const fs::path widget_library_path = temp_root / "widgetlib.prg";
        write_text(
            widget_library_path,
            "DEFINE CLASS ParentForm AS Custom\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS ParentButton OF buttons.prg\n"
            "    Caption = 'Save'\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_base_inherited_addobject_deeper_external_child_external_base_provenance.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oChild = oCreate.cmdSave\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 81)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "cChildClass = oChild.Class\n"
            "cChildBaseClass = oChild.BaseClass\n"
            "cChildParentClass = oChild.ParentClass\n"
            "cChildClassLibrary = oChild.ClassLibrary\n"
            "cChildClassReflect = GETPEM(oChild, 'Class')\n"
            "cChildBaseClassReflect = GETPEM(oChild, 'BaseClass')\n"
            "cChildParentClassReflect = GETPEM(oChild, 'ParentClass')\n"
            "cChildClassLibraryReflect = GETPEM(oChild, 'ClassLibrary')\n"
            "lChildHasClass = PEMSTATUS(oChild, 'Class', 1)\n"
            "lChildHasBaseClass = PEMSTATUS(oChild, 'BaseClass', 1)\n"
            "lChildHasParentClass = PEMSTATUS(oChild, 'ParentClass', 1)\n"
            "lChildHasClassLibrary = PEMSTATUS(oChild, 'ClassLibrary', 1)\n"
            "lChildClassReadOnly = PEMSTATUS(oChild, 'Class', 5)\n"
            "lChildBaseClassReadOnly = PEMSTATUS(oChild, 'BaseClass', 5)\n"
            "lChildParentClassReadOnly = PEMSTATUS(oChild, 'ParentClass', 5)\n"
            "lChildClassLibraryReadOnly = PEMSTATUS(oChild, 'ClassLibrary', 5)\n"
            "nMembersProps = AMEMBERS(aMembersProps, oChild, 1)\n"
            "nMembersUnion = AMEMBERS(aMembersUnion, oChild, 3)\n"
            "cProp1 = aMembersProps[1]\n"
            "cProp2 = aMembersProps[2]\n"
            "cProp3 = aMembersProps[3]\n"
            "cProp4 = aMembersProps[4]\n"
            "cProp5 = aMembersProps[5]\n"
            "cProp6 = aMembersProps[6]\n"
            "cUnion4 = aMembersUnion[4]\n"
            "nClassCount = ACLASS(aClass, oChild)\n"
            "cClass1 = aClass[1]\n"
            "cClass2 = aClass[2]\n"
            "cClass3 = aClass[3]\n"
            "cClass4 = aClass[4]\n"
            "cClass5 = aClass[5]\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external-base inherited ADDOBJECT deeper external child external-base provenance script should complete: ") + state.message +
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

        check("cchildclass", "SaveButton");
        check("cchildbaseclass", "ParentButton");
        check("cchildparentclass", "ParentButton");
        check("cchildclasslibrary", button_library_path.string());
        check("cchildclassreflect", "SaveButton");
        check("cchildbaseclassreflect", "ParentButton");
        check("cchildparentclassreflect", "ParentButton");
        check("cchildclasslibraryreflect", button_library_path.string());
        check("lchildhasclass", "true");
        check("lchildhasbaseclass", "true");
        check("lchildhasparentclass", "true");
        check("lchildhasclasslibrary", "true");
        check("lchildclassreadonly", "true");
        check("lchildbaseclassreadonly", "true");
        check("lchildparentclassreadonly", "true");
        check("lchildclasslibraryreadonly", "true");
        check("nmembersprops", "6");
        check("nmembersunion", "6");
        check("cprop1", "BASECLASS");
        check("cprop2", "CAPTION");
        check("cprop3", "CLASS");
        check("cprop4", "CLASSLIBRARY");
        check("cprop5", "PARENT");
        check("cprop6", "PARENTCLASS");
        check("cunion4", "CLASSLIBRARY");
        check("nclasscount", "5");
        check("cclass1", "SAVEBUTTON");
        check("cclass2", "PARENTBUTTON");
        check("cclass3", "ROOTBUTTON");
        check("cclass4", "CUSTOM");
        check("cclass5", "OBJECT");
        check("ldictset", "true");
        check("ndictcompare", "81");

        expect(state.ole_objects.size() == 3U,
               "external-base inherited ADDOBJECT deeper external child external-base provenance should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &parent_object = state.ole_objects[0];
            const auto &child_object = state.ole_objects[1];
            expect(parent_object.prog_id == "ChildForm",
                   "external-base inherited ADDOBJECT deeper external child external-base provenance should preserve parent identity");
            expect(child_object.prog_id == "SaveButton",
                   "external-base inherited ADDOBJECT deeper external child external-base provenance should preserve child identity");
            expect(child_object.base_class_name == "ParentButton",
                   "external-base inherited ADDOBJECT deeper external child external-base provenance should preserve immediate child base-class identity");
            expect(child_object.source == widget_library_path.string(),
                   "external-base inherited ADDOBJECT deeper external child external-base provenance should preserve the defining child-class source path");
            expect(child_object.class_library == button_library_path.string(),
                   "external-base inherited ADDOBJECT deeper external child external-base provenance should preserve the immediate external ClassLibrary path");
            expect(child_object.class_hierarchy.size() == 5U,
                   "external-base inherited ADDOBJECT deeper external child external-base provenance should preserve the deeper runtime child class hierarchy");
            expect(!child_object.properties.contains("classlibrary"),
                   "external-base inherited ADDOBJECT deeper external child external-base provenance should not materialize a child ClassLibrary shadow");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external-base inherited ADDOBJECT deeper external child external-base provenance lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_native_addobject_deeper_external_child_external_base_classlibrary_stays_read_only_to_setpem()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_addobject_deeper_external_child_external_base_setpem";
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

        const fs::path main_path = temp_root / "native_addobject_deeper_external_child_external_base_setpem.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('DemoForm')\n"
            "oChild = oForm.cmdSave\n"
            "lChildClassReadOnly = PEMSTATUS(oChild, 'Class', 5)\n"
            "lChildBaseClassReadOnly = PEMSTATUS(oChild, 'BaseClass', 5)\n"
            "lChildParentClassReadOnly = PEMSTATUS(oChild, 'ParentClass', 5)\n"
            "lChildClassLibraryReadOnly = PEMSTATUS(oChild, 'ClassLibrary', 5)\n"
            "lSetChildClass = SETPEM(oChild, 'Class', 'OtherClass')\n"
            "lSetChildBaseClass = SETPEM(oChild, 'BaseClass', 'OtherBase')\n"
            "lSetChildParentClass = SETPEM(oChild, 'ParentClass', 'OtherParent')\n"
            "lSetChildClassLibrary = SETPEM(oChild, 'ClassLibrary', 'other.prg')\n"
            "cChildClassAfter = GETPEM(oChild, 'Class')\n"
            "cChildBaseClassAfter = GETPEM(oChild, 'BaseClass')\n"
            "cChildParentClassAfter = GETPEM(oChild, 'ParentClass')\n"
            "cChildClassLibraryAfter = GETPEM(oChild, 'ClassLibrary')\n"
            "cChildClassLibraryPropAfter = oChild.ClassLibrary\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 82)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS ParentButton OF buttons.prg\n"
            "    Caption = 'Save'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native ADDOBJECT deeper external child external-base SETPEM script should complete: ") + state.message +
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

        check("lchildclassreadonly", "true");
        check("lchildbaseclassreadonly", "true");
        check("lchildparentclassreadonly", "true");
        check("lchildclasslibraryreadonly", "true");
        check("lsetchildclass", "false");
        check("lsetchildbaseclass", "false");
        check("lsetchildparentclass", "false");
        check("lsetchildclasslibrary", "false");
        check("cchildclassafter", "SaveButton");
        check("cchildbaseclassafter", "ParentButton");
        check("cchildparentclassafter", "ParentButton");
        check("cchildclasslibraryafter", button_library_path.string());
        check("cchildclasslibrarypropafter", button_library_path.string());
        check("ldictset", "true");
        check("ndictcompare", "82");

        expect(state.ole_objects.size() == 3U,
               "native ADDOBJECT deeper external child external-base SETPEM should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &parent_object = state.ole_objects[0];
            const auto &child_object = state.ole_objects[1];
            expect(parent_object.prog_id == "DemoForm",
                   "native ADDOBJECT deeper external child external-base SETPEM should preserve parent identity");
            expect(child_object.prog_id == "SaveButton",
                   "native ADDOBJECT deeper external child external-base SETPEM should preserve child identity");
            expect(child_object.base_class_name == "ParentButton",
                   "native ADDOBJECT deeper external child external-base SETPEM should preserve immediate child base-class identity");
            expect(child_object.source == main_path.string(),
                   "native ADDOBJECT deeper external child external-base SETPEM should preserve the defining child-class source path");
            expect(child_object.class_library == button_library_path.string(),
                   "native ADDOBJECT deeper external child external-base SETPEM should preserve the immediate external ClassLibrary path");
            expect(child_object.class_hierarchy.size() == 5U,
                   "native ADDOBJECT deeper external child external-base SETPEM should preserve the deeper runtime child class hierarchy");
            expect(!child_object.properties.contains("class"),
                   "native ADDOBJECT deeper external child external-base SETPEM should not materialize a writable child Class shadow");
            expect(!child_object.properties.contains("baseclass"),
                   "native ADDOBJECT deeper external child external-base SETPEM should not materialize a writable child BaseClass shadow");
            expect(!child_object.properties.contains("parentclass"),
                   "native ADDOBJECT deeper external child external-base SETPEM should not materialize a writable child ParentClass shadow");
            expect(!child_object.properties.contains("classlibrary"),
                   "native ADDOBJECT deeper external child external-base SETPEM should not materialize a writable child ClassLibrary shadow");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native ADDOBJECT deeper external child external-base SETPEM lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_inherited_external_base_addobject_deeper_external_child_external_base_classlibrary_stays_read_only_to_setpem()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_base_inherited_addobject_deeper_external_child_external_base_setpem";
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

        const fs::path widget_library_path = temp_root / "widgetlib.prg";
        write_text(
            widget_library_path,
            "DEFINE CLASS ParentForm AS Custom\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS ParentButton OF buttons.prg\n"
            "    Caption = 'Save'\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_base_inherited_addobject_deeper_external_child_external_base_setpem.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oChild = oCreate.cmdSave\n"
            "lChildClassReadOnly = PEMSTATUS(oChild, 'Class', 5)\n"
            "lChildBaseClassReadOnly = PEMSTATUS(oChild, 'BaseClass', 5)\n"
            "lChildParentClassReadOnly = PEMSTATUS(oChild, 'ParentClass', 5)\n"
            "lChildClassLibraryReadOnly = PEMSTATUS(oChild, 'ClassLibrary', 5)\n"
            "lSetChildClass = SETPEM(oChild, 'Class', 'OtherClass')\n"
            "lSetChildBaseClass = SETPEM(oChild, 'BaseClass', 'OtherBase')\n"
            "lSetChildParentClass = SETPEM(oChild, 'ParentClass', 'OtherParent')\n"
            "lSetChildClassLibrary = SETPEM(oChild, 'ClassLibrary', 'other.prg')\n"
            "cChildClassAfter = GETPEM(oChild, 'Class')\n"
            "cChildBaseClassAfter = GETPEM(oChild, 'BaseClass')\n"
            "cChildParentClassAfter = GETPEM(oChild, 'ParentClass')\n"
            "cChildClassLibraryAfter = GETPEM(oChild, 'ClassLibrary')\n"
            "cChildClassLibraryPropAfter = oChild.ClassLibrary\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 83)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external-base inherited ADDOBJECT deeper external child external-base SETPEM script should complete: ") + state.message +
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

        check("lchildclassreadonly", "true");
        check("lchildbaseclassreadonly", "true");
        check("lchildparentclassreadonly", "true");
        check("lchildclasslibraryreadonly", "true");
        check("lsetchildclass", "false");
        check("lsetchildbaseclass", "false");
        check("lsetchildparentclass", "false");
        check("lsetchildclasslibrary", "false");
        check("cchildclassafter", "SaveButton");
        check("cchildbaseclassafter", "ParentButton");
        check("cchildparentclassafter", "ParentButton");
        check("cchildclasslibraryafter", button_library_path.string());
        check("cchildclasslibrarypropafter", button_library_path.string());
        check("ldictset", "true");
        check("ndictcompare", "83");

        expect(state.ole_objects.size() == 3U,
               "external-base inherited ADDOBJECT deeper external child external-base SETPEM should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &parent_object = state.ole_objects[0];
            const auto &child_object = state.ole_objects[1];
            expect(parent_object.prog_id == "ChildForm",
                   "external-base inherited ADDOBJECT deeper external child external-base SETPEM should preserve parent identity");
            expect(child_object.prog_id == "SaveButton",
                   "external-base inherited ADDOBJECT deeper external child external-base SETPEM should preserve child identity");
            expect(child_object.base_class_name == "ParentButton",
                   "external-base inherited ADDOBJECT deeper external child external-base SETPEM should preserve immediate child base-class identity");
            expect(child_object.source == widget_library_path.string(),
                   "external-base inherited ADDOBJECT deeper external child external-base SETPEM should preserve the defining child-class source path");
            expect(child_object.class_library == button_library_path.string(),
                   "external-base inherited ADDOBJECT deeper external child external-base SETPEM should preserve the immediate external ClassLibrary path");
            expect(child_object.class_hierarchy.size() == 5U,
                   "external-base inherited ADDOBJECT deeper external child external-base SETPEM should preserve the deeper runtime child class hierarchy");
            expect(!child_object.properties.contains("class"),
                   "external-base inherited ADDOBJECT deeper external child external-base SETPEM should not materialize a writable child Class shadow");
            expect(!child_object.properties.contains("baseclass"),
                   "external-base inherited ADDOBJECT deeper external child external-base SETPEM should not materialize a writable child BaseClass shadow");
            expect(!child_object.properties.contains("parentclass"),
                   "external-base inherited ADDOBJECT deeper external child external-base SETPEM should not materialize a writable child ParentClass shadow");
            expect(!child_object.properties.contains("classlibrary"),
                   "external-base inherited ADDOBJECT deeper external child external-base SETPEM should not materialize a writable child ClassLibrary shadow");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external-base inherited ADDOBJECT deeper external child external-base SETPEM lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_native_addobject_deeper_external_child_external_base_classlibrary_cannot_be_shadowed_through_addproperty()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_addobject_deeper_external_child_external_base_addproperty";
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

        const fs::path main_path = temp_root / "native_addobject_deeper_external_child_external_base_addproperty.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('DemoForm')\n"
            "oChild = oForm.cmdSave\n"
            "lAddChildClass = ADDPROPERTY(oChild, 'Class', 'OtherClass')\n"
            "lAddChildBaseClass = ADDPROPERTY(oChild, 'BaseClass', 'OtherBase')\n"
            "lAddChildParentClass = ADDPROPERTY(oChild, 'ParentClass', 'OtherParent')\n"
            "lAddChildClassLibrary = ADDPROPERTY(oChild, 'ClassLibrary', 'other.prg')\n"
            "cChildClassAfter = GETPEM(oChild, 'Class')\n"
            "cChildBaseClassAfter = GETPEM(oChild, 'BaseClass')\n"
            "cChildParentClassAfter = GETPEM(oChild, 'ParentClass')\n"
            "cChildClassLibraryAfter = GETPEM(oChild, 'ClassLibrary')\n"
            "cChildClassLibraryPropAfter = oChild.ClassLibrary\n"
            "lChildClassReadOnly = PEMSTATUS(oChild, 'Class', 5)\n"
            "lChildBaseClassReadOnly = PEMSTATUS(oChild, 'BaseClass', 5)\n"
            "lChildParentClassReadOnly = PEMSTATUS(oChild, 'ParentClass', 5)\n"
            "lChildClassLibraryReadOnly = PEMSTATUS(oChild, 'ClassLibrary', 5)\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 84)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS ParentButton OF buttons.prg\n"
            "    Caption = 'Save'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native ADDOBJECT deeper external child external-base ADDPROPERTY script should complete: ") + state.message +
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

        check("laddchildclass", "false");
        check("laddchildbaseclass", "false");
        check("laddchildparentclass", "false");
        check("laddchildclasslibrary", "false");
        check("cchildclassafter", "SaveButton");
        check("cchildbaseclassafter", "ParentButton");
        check("cchildparentclassafter", "ParentButton");
        check("cchildclasslibraryafter", button_library_path.string());
        check("cchildclasslibrarypropafter", button_library_path.string());
        check("lchildclassreadonly", "true");
        check("lchildbaseclassreadonly", "true");
        check("lchildparentclassreadonly", "true");
        check("lchildclasslibraryreadonly", "true");
        check("ldictset", "true");
        check("ndictcompare", "84");

        expect(state.ole_objects.size() == 3U,
               "native ADDOBJECT deeper external child external-base ADDPROPERTY should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &parent_object = state.ole_objects[0];
            const auto &child_object = state.ole_objects[1];
            expect(parent_object.prog_id == "DemoForm",
                   "native ADDOBJECT deeper external child external-base ADDPROPERTY should preserve parent identity");
            expect(child_object.prog_id == "SaveButton",
                   "native ADDOBJECT deeper external child external-base ADDPROPERTY should preserve child identity");
            expect(child_object.base_class_name == "ParentButton",
                   "native ADDOBJECT deeper external child external-base ADDPROPERTY should preserve immediate child base-class identity");
            expect(child_object.source == main_path.string(),
                   "native ADDOBJECT deeper external child external-base ADDPROPERTY should preserve the defining child-class source path");
            expect(child_object.class_library == button_library_path.string(),
                   "native ADDOBJECT deeper external child external-base ADDPROPERTY should preserve the immediate external ClassLibrary path");
            expect(child_object.class_hierarchy.size() == 5U,
                   "native ADDOBJECT deeper external child external-base ADDPROPERTY should preserve the deeper runtime child class hierarchy");
            expect(!child_object.properties.contains("class"),
                   "native ADDOBJECT deeper external child external-base ADDPROPERTY should not materialize a child Class shadow");
            expect(!child_object.properties.contains("baseclass"),
                   "native ADDOBJECT deeper external child external-base ADDPROPERTY should not materialize a child BaseClass shadow");
            expect(!child_object.properties.contains("parentclass"),
                   "native ADDOBJECT deeper external child external-base ADDPROPERTY should not materialize a child ParentClass shadow");
            expect(!child_object.properties.contains("classlibrary"),
                   "native ADDOBJECT deeper external child external-base ADDPROPERTY should not materialize a child ClassLibrary shadow");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native ADDOBJECT deeper external child external-base ADDPROPERTY lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_inherited_external_base_addobject_deeper_external_child_external_base_classlibrary_cannot_be_shadowed_through_addproperty()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_base_inherited_addobject_deeper_external_child_external_base_addproperty";
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

        const fs::path widget_library_path = temp_root / "widgetlib.prg";
        write_text(
            widget_library_path,
            "DEFINE CLASS ParentForm AS Custom\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS ParentButton OF buttons.prg\n"
            "    Caption = 'Save'\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_base_inherited_addobject_deeper_external_child_external_base_addproperty.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oChild = oCreate.cmdSave\n"
            "lAddChildClass = ADDPROPERTY(oChild, 'Class', 'OtherClass')\n"
            "lAddChildBaseClass = ADDPROPERTY(oChild, 'BaseClass', 'OtherBase')\n"
            "lAddChildParentClass = ADDPROPERTY(oChild, 'ParentClass', 'OtherParent')\n"
            "lAddChildClassLibrary = ADDPROPERTY(oChild, 'ClassLibrary', 'other.prg')\n"
            "cChildClassAfter = GETPEM(oChild, 'Class')\n"
            "cChildBaseClassAfter = GETPEM(oChild, 'BaseClass')\n"
            "cChildParentClassAfter = GETPEM(oChild, 'ParentClass')\n"
            "cChildClassLibraryAfter = GETPEM(oChild, 'ClassLibrary')\n"
            "cChildClassLibraryPropAfter = oChild.ClassLibrary\n"
            "lChildClassReadOnly = PEMSTATUS(oChild, 'Class', 5)\n"
            "lChildBaseClassReadOnly = PEMSTATUS(oChild, 'BaseClass', 5)\n"
            "lChildParentClassReadOnly = PEMSTATUS(oChild, 'ParentClass', 5)\n"
            "lChildClassLibraryReadOnly = PEMSTATUS(oChild, 'ClassLibrary', 5)\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 85)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external-base inherited ADDOBJECT deeper external child external-base ADDPROPERTY script should complete: ") + state.message +
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

        check("laddchildclass", "false");
        check("laddchildbaseclass", "false");
        check("laddchildparentclass", "false");
        check("laddchildclasslibrary", "false");
        check("cchildclassafter", "SaveButton");
        check("cchildbaseclassafter", "ParentButton");
        check("cchildparentclassafter", "ParentButton");
        check("cchildclasslibraryafter", button_library_path.string());
        check("cchildclasslibrarypropafter", button_library_path.string());
        check("lchildclassreadonly", "true");
        check("lchildbaseclassreadonly", "true");
        check("lchildparentclassreadonly", "true");
        check("lchildclasslibraryreadonly", "true");
        check("ldictset", "true");
        check("ndictcompare", "85");

        expect(state.ole_objects.size() == 3U,
               "external-base inherited ADDOBJECT deeper external child external-base ADDPROPERTY should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &parent_object = state.ole_objects[0];
            const auto &child_object = state.ole_objects[1];
            expect(parent_object.prog_id == "ChildForm",
                   "external-base inherited ADDOBJECT deeper external child external-base ADDPROPERTY should preserve parent identity");
            expect(child_object.prog_id == "SaveButton",
                   "external-base inherited ADDOBJECT deeper external child external-base ADDPROPERTY should preserve child identity");
            expect(child_object.base_class_name == "ParentButton",
                   "external-base inherited ADDOBJECT deeper external child external-base ADDPROPERTY should preserve immediate child base-class identity");
            expect(child_object.source == widget_library_path.string(),
                   "external-base inherited ADDOBJECT deeper external child external-base ADDPROPERTY should preserve the defining child-class source path");
            expect(child_object.class_library == button_library_path.string(),
                   "external-base inherited ADDOBJECT deeper external child external-base ADDPROPERTY should preserve the immediate external ClassLibrary path");
            expect(child_object.class_hierarchy.size() == 5U,
                   "external-base inherited ADDOBJECT deeper external child external-base ADDPROPERTY should preserve the deeper runtime child class hierarchy");
            expect(!child_object.properties.contains("class"),
                   "external-base inherited ADDOBJECT deeper external child external-base ADDPROPERTY should not materialize a child Class shadow");
            expect(!child_object.properties.contains("baseclass"),
                   "external-base inherited ADDOBJECT deeper external child external-base ADDPROPERTY should not materialize a child BaseClass shadow");
            expect(!child_object.properties.contains("parentclass"),
                   "external-base inherited ADDOBJECT deeper external child external-base ADDPROPERTY should not materialize a child ParentClass shadow");
            expect(!child_object.properties.contains("classlibrary"),
                   "external-base inherited ADDOBJECT deeper external child external-base ADDPROPERTY should not materialize a child ClassLibrary shadow");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external-base inherited ADDOBJECT deeper external child external-base ADDPROPERTY lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_native_addobject_deeper_external_child_external_base_classlibrary_cannot_be_erased_through_removeproperty()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_addobject_deeper_external_child_external_base_removeproperty";
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

        const fs::path main_path = temp_root / "native_addobject_deeper_external_child_external_base_removeproperty.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('DemoForm')\n"
            "oChild = oForm.cmdSave\n"
            "lRemoveChildClass = REMOVEPROPERTY(oChild, 'Class')\n"
            "lRemoveChildBaseClass = REMOVEPROPERTY(oChild, 'BaseClass')\n"
            "lRemoveChildParentClass = REMOVEPROPERTY(oChild, 'ParentClass')\n"
            "lRemoveChildClassLibrary = REMOVEPROPERTY(oChild, 'ClassLibrary')\n"
            "cChildClassAfter = GETPEM(oChild, 'Class')\n"
            "cChildBaseClassAfter = GETPEM(oChild, 'BaseClass')\n"
            "cChildParentClassAfter = GETPEM(oChild, 'ParentClass')\n"
            "cChildClassLibraryAfter = GETPEM(oChild, 'ClassLibrary')\n"
            "cChildClassLibraryPropAfter = oChild.ClassLibrary\n"
            "lChildClassReadOnly = PEMSTATUS(oChild, 'Class', 5)\n"
            "lChildBaseClassReadOnly = PEMSTATUS(oChild, 'BaseClass', 5)\n"
            "lChildParentClassReadOnly = PEMSTATUS(oChild, 'ParentClass', 5)\n"
            "lChildClassLibraryReadOnly = PEMSTATUS(oChild, 'ClassLibrary', 5)\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 86)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS ParentButton OF buttons.prg\n"
            "    Caption = 'Save'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native ADDOBJECT deeper external child external-base REMOVEPROPERTY script should complete: ") + state.message +
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

        check("lremovechildclass", "false");
        check("lremovechildbaseclass", "false");
        check("lremovechildparentclass", "false");
        check("lremovechildclasslibrary", "false");
        check("cchildclassafter", "SaveButton");
        check("cchildbaseclassafter", "ParentButton");
        check("cchildparentclassafter", "ParentButton");
        check("cchildclasslibraryafter", button_library_path.string());
        check("cchildclasslibrarypropafter", button_library_path.string());
        check("lchildclassreadonly", "true");
        check("lchildbaseclassreadonly", "true");
        check("lchildparentclassreadonly", "true");
        check("lchildclasslibraryreadonly", "true");
        check("ldictset", "true");
        check("ndictcompare", "86");

        expect(state.ole_objects.size() == 3U,
               "native ADDOBJECT deeper external child external-base REMOVEPROPERTY should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &parent_object = state.ole_objects[0];
            const auto &child_object = state.ole_objects[1];
            expect(parent_object.prog_id == "DemoForm",
                   "native ADDOBJECT deeper external child external-base REMOVEPROPERTY should preserve parent identity");
            expect(child_object.prog_id == "SaveButton",
                   "native ADDOBJECT deeper external child external-base REMOVEPROPERTY should preserve child identity");
            expect(child_object.base_class_name == "ParentButton",
                   "native ADDOBJECT deeper external child external-base REMOVEPROPERTY should preserve immediate child base-class identity");
            expect(child_object.source == main_path.string(),
                   "native ADDOBJECT deeper external child external-base REMOVEPROPERTY should preserve the defining child-class source path");
            expect(child_object.class_library == button_library_path.string(),
                   "native ADDOBJECT deeper external child external-base REMOVEPROPERTY should preserve the immediate external ClassLibrary path");
            expect(child_object.class_hierarchy.size() == 5U,
                   "native ADDOBJECT deeper external child external-base REMOVEPROPERTY should preserve the deeper runtime child class hierarchy");
            expect(!child_object.properties.contains("class"),
                   "native ADDOBJECT deeper external child external-base REMOVEPROPERTY should not materialize a child Class shadow");
            expect(!child_object.properties.contains("baseclass"),
                   "native ADDOBJECT deeper external child external-base REMOVEPROPERTY should not materialize a child BaseClass shadow");
            expect(!child_object.properties.contains("parentclass"),
                   "native ADDOBJECT deeper external child external-base REMOVEPROPERTY should not materialize a child ParentClass shadow");
            expect(!child_object.properties.contains("classlibrary"),
                   "native ADDOBJECT deeper external child external-base REMOVEPROPERTY should not materialize a child ClassLibrary shadow");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native ADDOBJECT deeper external child external-base REMOVEPROPERTY lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_inherited_external_base_addobject_deeper_external_child_external_base_classlibrary_cannot_be_erased_through_removeproperty()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_base_inherited_addobject_deeper_external_child_external_base_removeproperty";
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

        const fs::path widget_library_path = temp_root / "widgetlib.prg";
        write_text(
            widget_library_path,
            "DEFINE CLASS ParentForm AS Custom\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS ParentButton OF buttons.prg\n"
            "    Caption = 'Save'\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_base_inherited_addobject_deeper_external_child_external_base_removeproperty.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oChild = oCreate.cmdSave\n"
            "lRemoveChildClass = REMOVEPROPERTY(oChild, 'Class')\n"
            "lRemoveChildBaseClass = REMOVEPROPERTY(oChild, 'BaseClass')\n"
            "lRemoveChildParentClass = REMOVEPROPERTY(oChild, 'ParentClass')\n"
            "lRemoveChildClassLibrary = REMOVEPROPERTY(oChild, 'ClassLibrary')\n"
            "cChildClassAfter = GETPEM(oChild, 'Class')\n"
            "cChildBaseClassAfter = GETPEM(oChild, 'BaseClass')\n"
            "cChildParentClassAfter = GETPEM(oChild, 'ParentClass')\n"
            "cChildClassLibraryAfter = GETPEM(oChild, 'ClassLibrary')\n"
            "cChildClassLibraryPropAfter = oChild.ClassLibrary\n"
            "lChildClassReadOnly = PEMSTATUS(oChild, 'Class', 5)\n"
            "lChildBaseClassReadOnly = PEMSTATUS(oChild, 'BaseClass', 5)\n"
            "lChildParentClassReadOnly = PEMSTATUS(oChild, 'ParentClass', 5)\n"
            "lChildClassLibraryReadOnly = PEMSTATUS(oChild, 'ClassLibrary', 5)\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 87)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external-base inherited ADDOBJECT deeper external child external-base REMOVEPROPERTY script should complete: ") + state.message +
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

        check("lremovechildclass", "false");
        check("lremovechildbaseclass", "false");
        check("lremovechildparentclass", "false");
        check("lremovechildclasslibrary", "false");
        check("cchildclassafter", "SaveButton");
        check("cchildbaseclassafter", "ParentButton");
        check("cchildparentclassafter", "ParentButton");
        check("cchildclasslibraryafter", button_library_path.string());
        check("cchildclasslibrarypropafter", button_library_path.string());
        check("lchildclassreadonly", "true");
        check("lchildbaseclassreadonly", "true");
        check("lchildparentclassreadonly", "true");
        check("lchildclasslibraryreadonly", "true");
        check("ldictset", "true");
        check("ndictcompare", "87");

        expect(state.ole_objects.size() == 3U,
               "external-base inherited ADDOBJECT deeper external child external-base REMOVEPROPERTY should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &parent_object = state.ole_objects[0];
            const auto &child_object = state.ole_objects[1];
            expect(parent_object.prog_id == "ChildForm",
                   "external-base inherited ADDOBJECT deeper external child external-base REMOVEPROPERTY should preserve parent identity");
            expect(child_object.prog_id == "SaveButton",
                   "external-base inherited ADDOBJECT deeper external child external-base REMOVEPROPERTY should preserve child identity");
            expect(child_object.base_class_name == "ParentButton",
                   "external-base inherited ADDOBJECT deeper external child external-base REMOVEPROPERTY should preserve immediate child base-class identity");
            expect(child_object.source == widget_library_path.string(),
                   "external-base inherited ADDOBJECT deeper external child external-base REMOVEPROPERTY should preserve the defining child-class source path");
            expect(child_object.class_library == button_library_path.string(),
                   "external-base inherited ADDOBJECT deeper external child external-base REMOVEPROPERTY should preserve the immediate external ClassLibrary path");
            expect(child_object.class_hierarchy.size() == 5U,
                   "external-base inherited ADDOBJECT deeper external child external-base REMOVEPROPERTY should preserve the deeper runtime child class hierarchy");
            expect(!child_object.properties.contains("class"),
                   "external-base inherited ADDOBJECT deeper external child external-base REMOVEPROPERTY should not materialize a child Class shadow");
            expect(!child_object.properties.contains("baseclass"),
                   "external-base inherited ADDOBJECT deeper external child external-base REMOVEPROPERTY should not materialize a child BaseClass shadow");
            expect(!child_object.properties.contains("parentclass"),
                   "external-base inherited ADDOBJECT deeper external child external-base REMOVEPROPERTY should not materialize a child ParentClass shadow");
            expect(!child_object.properties.contains("classlibrary"),
                   "external-base inherited ADDOBJECT deeper external child external-base REMOVEPROPERTY should not materialize a child ClassLibrary shadow");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external-base inherited ADDOBJECT deeper external child external-base REMOVEPROPERTY lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_native_addobject_deeper_external_child_external_base_classlibrary_cannot_be_shadowed_through_direct_assignment()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_addobject_deeper_external_child_external_base_direct_assignment";
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

        const fs::path main_path = temp_root / "native_addobject_deeper_external_child_external_base_direct_assignment.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('DemoForm')\n"
            "oChild = oForm.cmdSave\n"
            "oChild.Class = 'OtherClass'\n"
            "oChild.BaseClass = 'OtherBase'\n"
            "oChild.ParentClass = 'OtherParent'\n"
            "oChild.ClassLibrary = 'other.prg'\n"
            "cChildClassAfter = GETPEM(oChild, 'Class')\n"
            "cChildBaseClassAfter = GETPEM(oChild, 'BaseClass')\n"
            "cChildParentClassAfter = GETPEM(oChild, 'ParentClass')\n"
            "cChildClassLibraryAfter = GETPEM(oChild, 'ClassLibrary')\n"
            "cChildClassLibraryPropAfter = oChild.ClassLibrary\n"
            "lChildClassReadOnly = PEMSTATUS(oChild, 'Class', 5)\n"
            "lChildBaseClassReadOnly = PEMSTATUS(oChild, 'BaseClass', 5)\n"
            "lChildParentClassReadOnly = PEMSTATUS(oChild, 'ParentClass', 5)\n"
            "lChildClassLibraryReadOnly = PEMSTATUS(oChild, 'ClassLibrary', 5)\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 88)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS ParentButton OF buttons.prg\n"
            "    Caption = 'Save'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native ADDOBJECT deeper external child external-base direct-assignment script should complete: ") + state.message +
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

        check("cchildclassafter", "SaveButton");
        check("cchildbaseclassafter", "ParentButton");
        check("cchildparentclassafter", "ParentButton");
        check("cchildclasslibraryafter", button_library_path.string());
        check("cchildclasslibrarypropafter", button_library_path.string());
        check("lchildclassreadonly", "true");
        check("lchildbaseclassreadonly", "true");
        check("lchildparentclassreadonly", "true");
        check("lchildclasslibraryreadonly", "true");
        check("ldictset", "true");
        check("ndictcompare", "88");

        expect(state.ole_objects.size() == 3U,
               "native ADDOBJECT deeper external child external-base direct assignment should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &parent_object = state.ole_objects[0];
            const auto &child_object = state.ole_objects[1];
            expect(parent_object.prog_id == "DemoForm",
                   "native ADDOBJECT deeper external child external-base direct assignment should preserve parent identity");
            expect(child_object.prog_id == "SaveButton",
                   "native ADDOBJECT deeper external child external-base direct assignment should preserve child identity");
            expect(child_object.base_class_name == "ParentButton",
                   "native ADDOBJECT deeper external child external-base direct assignment should preserve immediate child base-class identity");
            expect(child_object.source == main_path.string(),
                   "native ADDOBJECT deeper external child external-base direct assignment should preserve the defining child-class source path");
            expect(child_object.class_library == button_library_path.string(),
                   "native ADDOBJECT deeper external child external-base direct assignment should preserve the immediate external ClassLibrary path");
            expect(child_object.class_hierarchy.size() == 5U,
                   "native ADDOBJECT deeper external child external-base direct assignment should preserve the deeper runtime child class hierarchy");
            expect(!child_object.properties.contains("class"),
                   "native ADDOBJECT deeper external child external-base direct assignment should not materialize a child Class shadow");
            expect(!child_object.properties.contains("baseclass"),
                   "native ADDOBJECT deeper external child external-base direct assignment should not materialize a child BaseClass shadow");
            expect(!child_object.properties.contains("parentclass"),
                   "native ADDOBJECT deeper external child external-base direct assignment should not materialize a child ParentClass shadow");
            expect(!child_object.properties.contains("classlibrary"),
                   "native ADDOBJECT deeper external child external-base direct assignment should not materialize a child ClassLibrary shadow");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native ADDOBJECT deeper external child external-base direct assignment lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_inherited_external_base_addobject_deeper_external_child_external_base_classlibrary_cannot_be_shadowed_through_direct_assignment()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_base_inherited_addobject_deeper_external_child_external_base_direct_assignment";
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

        const fs::path widget_library_path = temp_root / "widgetlib.prg";
        write_text(
            widget_library_path,
            "DEFINE CLASS ParentForm AS Custom\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS ParentButton OF buttons.prg\n"
            "    Caption = 'Save'\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_base_inherited_addobject_deeper_external_child_external_base_direct_assignment.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oChild = oCreate.cmdSave\n"
            "oChild.Class = 'OtherClass'\n"
            "oChild.BaseClass = 'OtherBase'\n"
            "oChild.ParentClass = 'OtherParent'\n"
            "oChild.ClassLibrary = 'other.prg'\n"
            "cChildClassAfter = GETPEM(oChild, 'Class')\n"
            "cChildBaseClassAfter = GETPEM(oChild, 'BaseClass')\n"
            "cChildParentClassAfter = GETPEM(oChild, 'ParentClass')\n"
            "cChildClassLibraryAfter = GETPEM(oChild, 'ClassLibrary')\n"
            "cChildClassLibraryPropAfter = oChild.ClassLibrary\n"
            "lChildClassReadOnly = PEMSTATUS(oChild, 'Class', 5)\n"
            "lChildBaseClassReadOnly = PEMSTATUS(oChild, 'BaseClass', 5)\n"
            "lChildParentClassReadOnly = PEMSTATUS(oChild, 'ParentClass', 5)\n"
            "lChildClassLibraryReadOnly = PEMSTATUS(oChild, 'ClassLibrary', 5)\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 89)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external-base inherited ADDOBJECT deeper external child external-base direct-assignment script should complete: ") + state.message +
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

        check("cchildclassafter", "SaveButton");
        check("cchildbaseclassafter", "ParentButton");
        check("cchildparentclassafter", "ParentButton");
        check("cchildclasslibraryafter", button_library_path.string());
        check("cchildclasslibrarypropafter", button_library_path.string());
        check("lchildclassreadonly", "true");
        check("lchildbaseclassreadonly", "true");
        check("lchildparentclassreadonly", "true");
        check("lchildclasslibraryreadonly", "true");
        check("ldictset", "true");
        check("ndictcompare", "89");

        expect(state.ole_objects.size() == 3U,
               "external-base inherited ADDOBJECT deeper external child external-base direct assignment should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &parent_object = state.ole_objects[0];
            const auto &child_object = state.ole_objects[1];
            expect(parent_object.prog_id == "ChildForm",
                   "external-base inherited ADDOBJECT deeper external child external-base direct assignment should preserve parent identity");
            expect(child_object.prog_id == "SaveButton",
                   "external-base inherited ADDOBJECT deeper external child external-base direct assignment should preserve child identity");
            expect(child_object.base_class_name == "ParentButton",
                   "external-base inherited ADDOBJECT deeper external child external-base direct assignment should preserve immediate child base-class identity");
            expect(child_object.source == widget_library_path.string(),
                   "external-base inherited ADDOBJECT deeper external child external-base direct assignment should preserve the defining child-class source path");
            expect(child_object.class_library == button_library_path.string(),
                   "external-base inherited ADDOBJECT deeper external child external-base direct assignment should preserve the immediate external ClassLibrary path");
            expect(child_object.class_hierarchy.size() == 5U,
                   "external-base inherited ADDOBJECT deeper external child external-base direct assignment should preserve the deeper runtime child class hierarchy");
            expect(!child_object.properties.contains("class"),
                   "external-base inherited ADDOBJECT deeper external child external-base direct assignment should not materialize a child Class shadow");
            expect(!child_object.properties.contains("baseclass"),
                   "external-base inherited ADDOBJECT deeper external child external-base direct assignment should not materialize a child BaseClass shadow");
            expect(!child_object.properties.contains("parentclass"),
                   "external-base inherited ADDOBJECT deeper external child external-base direct assignment should not materialize a child ParentClass shadow");
            expect(!child_object.properties.contains("classlibrary"),
                   "external-base inherited ADDOBJECT deeper external child external-base direct assignment should not materialize a child ClassLibrary shadow");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external-base inherited ADDOBJECT deeper external child external-base direct assignment lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_same_prg_native_dodefault_dispatches_base_methods_and_preserves_byref_init_flow()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_dodefault";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_dodefault.prg";
        write_text(
            main_path,
            "nSeed = 5\n"
            "oCreate = CREATEOBJECT('ChildWidget', @nSeed)\n"
            "oPlain = CREATEOBJECT('Empty')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "oPlain.Extra = 'plain'\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 7)\n"
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
            "ENDDEFINE\n"
            "DEFINE CLASS ChildWidget AS ParentWidget\n"
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
               std::string("native DODEFAULT script should complete: ") + state.message +
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
        check("ndictcompare", "7");

        expect(state.ole_objects.size() == 3U,
               "native DODEFAULT script should register native, plain, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &native_object = state.ole_objects[0];
            expect(native_object.prog_id == "ChildWidget",
                   "native DODEFAULT should preserve child class identity");
            const auto caption = native_object.properties.find("caption");
            const auto value = native_object.properties.find("nvalue");
            const auto parent_init = native_object.properties.find("lparentinit");
            const auto child_init = native_object.properties.find("lchildinit");
            expect(caption != native_object.properties.end(),
                   "native DODEFAULT should preserve child/base Init-updated caption state");
            expect(value != native_object.properties.end(),
                   "native DODEFAULT should preserve by-reference Init-updated numeric state");
            expect(parent_init != native_object.properties.end(),
                   "native DODEFAULT should preserve parent Init state");
            expect(child_init != native_object.properties.end(),
                   "native DODEFAULT should preserve child Init state");
            if (caption != native_object.properties.end())
            {
                expect(copperfin::runtime::format_value(caption->second) == "Child-P-C",
                       "native DODEFAULT should compose child Init logic after base Init");
            }
            if (value != native_object.properties.end())
            {
                expect(copperfin::runtime::format_value(value->second) == "7",
                       "native DODEFAULT should preserve base Init by-reference write-back results");
            }
            if (parent_init != native_object.properties.end())
            {
                expect(copperfin::runtime::format_value(parent_init->second) == "true",
                       "native DODEFAULT should run parent Init through the base-call path");
            }
            if (child_init != native_object.properties.end())
            {
                expect(copperfin::runtime::format_value(child_init->second) == "true",
                       "native DODEFAULT should continue child Init logic after the base-call path");
            }

            expect(state.ole_objects[1].prog_id == "Empty",
                   "plain CREATEOBJECT should remain stable while native DODEFAULT lands");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native DODEFAULT lands");
        }

        const bool has_base_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.baseinvoke" &&
                   (event.detail == "ParentWidget.Init" ||
                    event.detail == "ParentWidget.Describe" ||
                    event.detail == "ParentWidget.Who");
        });
        expect(has_base_invoke_event,
               "native DODEFAULT should emit a base-invoke runtime event");

        fs::remove_all(temp_root, ignored);
    }

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
        check("nmembersmethods", "3");
        check("cmethod1", "DESCRIBE");
        check("cmethod2", "PING");
        check("cmethod3", "WHO");
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
        check("nmembersunion", "10");
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
        check("nmembersunion", "12");
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
            "nCpCurrentUni   = CPCURRENT(2)\n"
            // CPCONVERT identity pass-through
            "cCpConverted    = CPCONVERT(1252, 1252, 'hello')\n"
            // CPDBF first-pass stub
            "nCpDbf          = CPDBF()\n"
            // GETPICT headless stub
            "cPict           = GETPICT('Select Image')\n"
            // GETCOLOR headless stub
            "nColor          = GETCOLOR()\n"
            // GETFONT headless stub
            "cFont           = GETFONT('Arial')\n"
            // VARREAD headless stub
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

        check("ncpcurrent",     "1252");
        check("ncpcurrentansi", "1252");
        check("ncpcurrentuni",  "65001");
        check("ccpconverted",   "hello");
        check("ncpdbf",         "1252");
        check("cpict",          "");
        check("ncolor",         "0");
        check("cfont",          "");
        check("cvarread",       "");
        check("lidsdistinct",   "true");
        // UUID: 8-4-4-4-12 hex = 36 characters
        check("nidlen", "36");

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

    void test_lookup_expression_function_supports_sql_cursors()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_lookup_sql";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path people_path = temp_root / "people.dbf";
        write_people_dbf(people_path, {{"ALICE", 30}, {"BOB", 25}});

        const fs::path main_path = temp_root / "lookup_sql_test.prg";
        write_text(
            main_path,
            "USE '" + people_path.string() + "' ALIAS people IN 0\n"
            "nConn = SQLCONNECT('dsn=Northwind')\n"
            "nExec = SQLEXEC(nConn, 'select * from customers', 'sqlcust')\n"
            "SELECT people\n"
            "nSelectedBefore = SELECT()\n"
            "cAliasBefore = ALIAS()\n"
            "nLookupAmount = LOOKUP(sqlcust.AMOUNT, 'BRAVO', 'sqlcust', 'NAME')\n"
            "lLookupMissing = LOOKUP(sqlcust.NAME, 'ZZZZ', 'sqlcust', 'NAME')\n"
            "nSqlRec = RECNO('sqlcust')\n"
            "nSelectedAfter = SELECT()\n"
            "cAliasAfter = ALIAS()\n"
            "lDisc = SQLDISCONNECT(nConn)\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "SQL LOOKUP test script should complete");
        expect(state.sql_connections.empty(), "SQL LOOKUP test should disconnect its SQL handle");

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

        check("nselectedbefore", "1");
        check("caliasbefore", "people");
        check("nlookupamount", "20");
        check("llookupmissing", "false");
        check("nselectedafter", "1");
        check("caliasafter", "people");
        check("ldisc", "1");
        {
            const auto it = state.globals.find("nsqlrec");
            expect(it != state.globals.end(), "nsqlrec from SQL LOOKUP should be set");
            if (it != state.globals.end())
            {
                const double recno = std::stod(copperfin::runtime::format_value(it->second));
                expect(recno > 0.0,
                       "SQL LOOKUP should leave the targeted SQL cursor on a found record");
            }
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_lookup_supports_macro_alias_and_tag_arguments()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_lookup_target_context";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path people_path = temp_root / "people.dbf";
        const fs::path people_cdx  = temp_root / "people.cdx";
        write_people_dbf(people_path, {{"ALICE", 30}, {"BOB", 25}, {"CAROL", 35}});
        write_synthetic_cdx(people_cdx, "NAME", "UPPER(NAME)");

        const fs::path main_path = temp_root / "lookup_macro_arguments.prg";
        write_text(
            main_path,
            "USE '" + people_path.string() + "' ALIAS people IN 0\n"
            "USE '" + people_path.string() + "' ALIAS other AGAIN IN 0\n"
            "SELECT other\n"
            "SET ORDER TO TAG NAME IN people\n"
            "cAlias = 'people'\n"
            "cAliasHolder = 'cAlias'\n"
            "cAliasDeepHolder = 'cAliasHolder'\n"
            "cTag = 'NAME'\n"
            "cTagHolder = 'cTag'\n"
            "cTagDeepHolder = 'cTagHolder'\n"
            "cFound = LOOKUP(people.NAME, 'BOB', cAlias, cTag)\n"
            "cFoundNested = LOOKUP(people.NAME, 'BOB', &cAliasHolder, &cTagHolder)\n"
            "cFoundSecondHop = LOOKUP(people.NAME, 'BOB', &cAliasDeepHolder, &cTagDeepHolder)\n"
            "nSelectedAfter = SELECT()\n"
            "cAliasAfter = ALIAS()\n"
            "nPeopleRec = RECNO('people')\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "LOOKUP macro-argument test should complete");

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

        check("cfound", "BOB");
        check("cfoundnested", "BOB");
        check("cfoundsecondhop", "BOB");
        check("nselectedafter", "2");
        check("caliasafter", "other");
        check("npeoplerec", "2");

        fs::remove_all(temp_root, ignored);
    }

    void test_lookup_supports_macro_return_and_search_expressions()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_lookup_macro_exprs";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path people_path = temp_root / "people.dbf";
        const fs::path people_cdx  = temp_root / "people.cdx";
        write_people_dbf(people_path, {{"ALICE", 30}, {"BOB", 25}, {"CAROL", 35}});
        write_synthetic_cdx(people_cdx, "NAME", "UPPER(NAME)");

        const fs::path main_path = temp_root / "lookup_macro_exprs.prg";
        write_text(
            main_path,
            "USE '" + people_path.string() + "' ALIAS people IN 0\n"
            "USE '" + people_path.string() + "' ALIAS other AGAIN IN 0\n"
            "SELECT other\n"
            "SET ORDER TO TAG NAME IN people\n"
            "cAlias = 'people'\n"
            "cAliasHolder = 'cAlias'\n"
            "cAliasDeepHolder = 'cAliasHolder'\n"
            "cTag = 'NAME'\n"
            "cTagHolder = 'cTag'\n"
            "cTagDeepHolder = 'cTagHolder'\n"
            "cReturnExpr = 'people.NAME'\n"
            "cReturnExprHolder = 'cReturnExpr'\n"
            "cReturnExprDeepHolder = 'cReturnExprHolder'\n"
            "cAgeExpr = 'people.AGE'\n"
            "cAgeExprHolder = 'cAgeExpr'\n"
            "cAgeExprDeepHolder = 'cAgeExprHolder'\n"
            "cSearchExpr = 'BOB'\n"
            "cSearchExprHolder = 'cSearchExpr'\n"
            "cSearchExprDeepHolder = 'cSearchExprHolder'\n"
            "cFound = LOOKUP(&cReturnExpr, &cSearchExpr, cAlias, cTag)\n"
            "cFoundNested = LOOKUP(&cReturnExprHolder, &cSearchExprHolder, cAlias, cTag)\n"
            "cFoundSecondHop = LOOKUP(&cReturnExprDeepHolder, &cSearchExprDeepHolder, &cAliasDeepHolder, &cTagDeepHolder)\n"
            "nFoundAge = LOOKUP(&cAgeExprDeepHolder, &cSearchExprDeepHolder, &cAliasDeepHolder, &cTagDeepHolder)\n"
            "nSelectedAfter = SELECT()\n"
            "cAliasAfter = ALIAS()\n"
            "nPeopleRec = RECNO('people')\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "LOOKUP macro return/search-expression test should complete");

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

        check("cfound", "BOB");
        check("cfoundnested", "BOB");
        check("cfoundsecondhop", "BOB");
        check("nfoundage", "25");
        check("nselectedafter", "2");
        check("caliasafter", "other");
        check("npeoplerec", "2");

        fs::remove_all(temp_root, ignored);
    }

    void test_lookup_supports_macro_arguments_on_sql_cursors()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_lookup_sql_macro_exprs";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path people_path = temp_root / "people.dbf";
        write_people_dbf(people_path, {{"ALICE", 30}, {"BOB", 25}});

        const fs::path main_path = temp_root / "lookup_sql_macro_exprs.prg";
        write_text(
            main_path,
            "USE '" + people_path.string() + "' ALIAS people IN 0\n"
            "nConn = SQLCONNECT('dsn=Northwind')\n"
            "nExec = SQLEXEC(nConn, 'select * from customers', 'sqlcust')\n"
            "SELECT people\n"
            "cAlias = 'sqlcust'\n"
            "cAliasHolder = 'cAlias'\n"
            "cAliasDeepHolder = 'cAliasHolder'\n"
            "cTag = 'NAME'\n"
            "cTagHolder = 'cTag'\n"
            "cTagDeepHolder = 'cTagHolder'\n"
            "cReturnExpr = 'sqlcust.AMOUNT'\n"
            "cReturnExprHolder = 'cReturnExpr'\n"
            "cReturnExprDeepHolder = 'cReturnExprHolder'\n"
            "cSearchExpr = 'BRAVO'\n"
            "cSearchExprHolder = 'cSearchExpr'\n"
            "cSearchExprDeepHolder = 'cSearchExprHolder'\n"
            "cMissingSearch = 'ZZZZ'\n"
            "cMissingSearchHolder = 'cMissingSearch'\n"
            "cMissingSearchDeepHolder = 'cMissingSearchHolder'\n"
            "nLookupAmount = LOOKUP(&cReturnExpr, &cSearchExpr, &cAlias, &cTag)\n"
            "nLookupAmountNested = LOOKUP(&cReturnExpr, &cSearchExpr, &cAliasHolder, &cTagHolder)\n"
            "nLookupAmountSecondHop = LOOKUP(&cReturnExpr, &cSearchExpr, &cAliasDeepHolder, &cTagDeepHolder)\n"
            "nLookupAmountExprSecondHop = LOOKUP(&cReturnExprDeepHolder, &cSearchExprDeepHolder, &cAlias, &cTag)\n"
            "nLookupAmountAllSecondHop = LOOKUP(&cReturnExprDeepHolder, &cSearchExprDeepHolder, &cAliasDeepHolder, &cTagDeepHolder)\n"
            "nLookupAmountMissAllSecondHop = LOOKUP(&cReturnExprDeepHolder, &cMissingSearchDeepHolder, &cAliasDeepHolder, &cTagDeepHolder)\n"
            "nSqlRec = RECNO('sqlcust')\n"
            "nSelectedAfter = SELECT()\n"
            "cAliasAfter = ALIAS()\n"
            "lDisc = SQLDISCONNECT(nConn)\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "SQL LOOKUP macro-argument test should complete");
        expect(state.sql_connections.empty(), "SQL LOOKUP macro-argument test should disconnect its SQL handle");

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

        check("nlookupamount", "20");
        check("nlookupamountnested", "20");
        check("nlookupamountsecondhop", "20");
        check("nlookupamountexprsecondhop", "20");
        check("nlookupamountallsecondhop", "20");
        check("nlookupamountmissallsecondhop", "0");
        check("nselectedafter", "1");
        check("caliasafter", "people");
        check("ldisc", "1");
        {
            const auto it = state.globals.find("nsqlrec");
            expect(it != state.globals.end(), "nsqlrec from SQL LOOKUP macro-argument test should be set");
            if (it != state.globals.end())
            {
                const double recno = std::stod(copperfin::runtime::format_value(it->second));
                expect(recno > 0.0,
                       "SQL LOOKUP with macro-expanded arguments should leave the targeted SQL cursor on a found record");
            }
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_lookup_macro_return_expressions_preserve_typed_defaults_on_miss()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_lookup_macro_miss_defaults";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path people_path = temp_root / "people.dbf";
        const fs::path people_cdx  = temp_root / "people.cdx";
        write_people_dbf(people_path, {{"ALICE", 30}, {"BOB", 25}, {"CAROL", 35}});
        write_synthetic_cdx(people_cdx, "NAME", "UPPER(NAME)");

        const fs::path main_path = temp_root / "lookup_macro_miss_defaults.prg";
        write_text(
            main_path,
            "USE '" + people_path.string() + "' ALIAS people IN 0\n"
            "SET ORDER TO TAG NAME IN people\n"
            "cNumberExpr = 'AGE + 0'\n"
            "cNumberExprHolder = 'cNumberExpr'\n"
            "cNumberExprDeepHolder = 'cNumberExprHolder'\n"
            "cTextExpr = 'LEFT(NAME, 4)'\n"
            "cTextExprHolder = 'cTextExpr'\n"
            "cTextExprDeepHolder = 'cTextExprHolder'\n"
            "cMissingSearch = 'ZZZZ'\n"
            "cMissingSearchHolder = 'cMissingSearch'\n"
            "cMissingSearchDeepHolder = 'cMissingSearchHolder'\n"
            "cAlias = 'people'\n"
            "cAliasHolder = 'cAlias'\n"
            "cAliasDeepHolder = 'cAliasHolder'\n"
            "cTag = 'NAME'\n"
            "cTagHolder = 'cTag'\n"
            "cTagDeepHolder = 'cTagHolder'\n"
            "nMissing = LOOKUP(&cNumberExpr, &cMissingSearch, 'people', 'NAME')\n"
            "lMissing = LOOKUP(&cTextExpr, &cMissingSearch, 'people', 'NAME')\n"
            "nMissingSecondHop = LOOKUP(&cNumberExprDeepHolder, &cMissingSearchDeepHolder, &cAliasDeepHolder, &cTagDeepHolder)\n"
            "lMissingSecondHop = LOOKUP(&cTextExprDeepHolder, &cMissingSearchDeepHolder, &cAliasDeepHolder, &cTagDeepHolder)\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "LOOKUP macro miss-default test should complete: " + state.message);

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

        check("nmissing", "0");
        check("lmissing", "false");
        check("nmissingsecondhop", "0");
        check("lmissingsecondhop", "false");

        fs::remove_all(temp_root, ignored);
    }

    void test_createobject_property_assignment_round_trips()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_ole_property_roundtrip";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "ole_property_roundtrip.prg";
        write_text(
            main_path,
            "oDict = CREATEOBJECT('Scripting.Dictionary')\n"
            "oDict.CompareMode = 1\n"
            "nCompareMode = oDict.CompareMode\n"
            "oDict.Caption = 'Copperfin'\n"
            "cCaption = oDict.Caption\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "OLE property round-trip script should complete");

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

        check("ncomparemode", "1");
        check("ccaption", "Copperfin");
        expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "ole.set"; }),
               "OLE property round-trip should emit ole.set events");
        expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "ole.get"; }),
               "OLE property round-trip should emit ole.get events");

        fs::remove_all(temp_root, ignored);
    }

    void test_scripting_dictionary_collection_methods()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_scripting_dictionary_methods";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "scripting_dictionary_methods.prg";
        write_text(
            main_path,
            "oDict = CREATEOBJECT('Scripting.Dictionary')\n"
            "oDict.Add('Alpha', 41)\n"
            "lExistsBefore = oDict.Exists('Alpha')\n"
            "nItem = oDict.Item('Alpha')\n"
            "nCountAfterAdd = oDict.Count\n"
            "oDict.Remove('Alpha')\n"
            "lExistsAfterRemove = oDict.Exists('Alpha')\n"
            "oDict.Add('Beta', 5)\n"
            "oDict.Add('Gamma', 7)\n"
            "nCountBeforeClear = oDict.Count\n"
            "oDict.RemoveAll()\n"
            "nCountAfterClear = oDict.Count\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "Scripting.Dictionary method script should complete");

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

        check("lexistsbefore", "true");
        check("nitem", "41");
        check("ncountafteradd", "1");
        check("lexistsafterremove", "false");
        check("ncountbeforeclear", "2");
        check("ncountafterclear", "0");
        expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "ole.invoke"; }),
               "Scripting.Dictionary methods should emit ole.invoke events");

        fs::remove_all(temp_root, ignored);
    }

    void test_newobject_preserves_library_and_server_targeting()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_newobject_server_targeting";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "newobject_server_targeting.prg";
        write_text(
            main_path,
            "oRemote = NEWOBJECT('Session', 'app.vcx', '', '', .F., 'AppServer01')\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "NEWOBJECT server-targeting script should complete");
        expect(state.ole_objects.size() == 1U, "NEWOBJECT server-targeting script should register one object");
        if (!state.ole_objects.empty())
        {
            expect(state.ole_objects.front().prog_id == "Session",
                   "NEWOBJECT should preserve the requested class name as the OLE prog_id");
            expect(state.ole_objects.front().source == "app.vcx@AppServer01",
                   "NEWOBJECT should preserve library/server targeting metadata in object source");
        }
        expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
                   return event.category == "ole.newobject" &&
                          event.detail == "Session:app.vcx@AppServer01";
               }),
               "NEWOBJECT should emit library/server detail in ole.newobject events");

        fs::remove_all(temp_root, ignored);
    }

    void test_getobject_reuses_existing_class_and_source_targets()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_getobject_reuse";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "getobject_reuse.prg";
        write_text(
            main_path,
            "oWord1 = CREATEOBJECT('Word.Application')\n"
            "oWord2 = GETOBJECT('', 'Word.Application')\n"
            "lSameRunning = COMPOBJ(oWord1, oWord2)\n"
            "oDoc1 = GETOBJECT('sample.doc', 'Word.Application')\n"
            "oDoc2 = GETOBJECT('sample.doc', 'Word.Application')\n"
            "lSameDocument = COMPOBJ(oDoc1, oDoc2)\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "GETOBJECT reuse script should complete");

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

        check("lsamerunning", "true");
        check("lsamedocument", "true");
        expect(state.ole_objects.size() == 2U,
               "GETOBJECT reuse should attach to existing targets instead of registering duplicate objects");
        if (state.ole_objects.size() == 2U)
        {
            expect(state.ole_objects[0].prog_id == "Word.Application",
                   "GETOBJECT class-target reuse should preserve the requested class");
            expect(state.ole_objects[1].prog_id == "Word.Application",
                   "GETOBJECT file/class activation should preserve the requested class");
            expect(state.ole_objects[1].source == "sample.doc",
                   "GETOBJECT file/class activation should preserve the requested source file");
        }
        expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
                   return event.category == "ole.getobject" &&
                          event.detail == "sample.doc -> Word.Application";
               }),
               "GETOBJECT should emit resolved source/class detail in ole.getobject events");

        fs::remove_all(temp_root, ignored);
    }

} // namespace

int main()
{
    test_expression_runtime_surface_extensions();
    test_filesize_expression_function();
    test_recsize_reclength_expression_functions();
    test_environment_and_sys_introspection_functions();
    test_object_reflection_runtime_surface_functions();
    test_cursor_xml_round_trip_runtime_surface_functions();
    test_cursor_xml_invalid_input_runtime_surface_functions();
    test_newobject_getpem_setpem_compobj_functions();
    test_createobject_instantiates_native_prg_class_and_preserves_plain_object_creation();
    test_newobject_instantiates_native_prg_class_and_preserves_ole_newobject();
    test_native_addobject_materializes_child_objects_and_child_methods_see_parent();
    test_native_class_body_add_object_materializes_children_before_init();
    test_native_class_body_add_object_with_property_clauses_materialize_before_parent_init();
    test_native_class_body_object_blocks_materialize_children_before_parent_init();
    test_declarative_children_materialize_from_external_prg_sources_before_parent_init();
    test_native_child_methods_resolve_thisform_through_parent_chain();
    test_native_child_methods_resolve_thisformset_through_owner_chain();
    test_external_prg_base_child_methods_resolve_parent_thisform_and_thisformset();
    test_dotted_native_child_chains_traverse_contained_objects();
    test_dotted_native_child_assignments_traverse_contained_objects();
    test_native_removeobject_detaches_child_and_clears_parent_reference();
    test_native_release_invokes_destroy_and_invalidates_standalone_object();
    test_native_release_recursively_destroys_contained_children_and_invalidates_owner_chain();
    test_native_release_detaches_contained_child_while_owner_and_sibling_remain_alive();
    test_native_owner_release_recursively_destroys_same_prg_object_block_child_subtree_and_invalidates_owner_chain();
    test_inherited_owner_release_recursively_destroys_same_prg_object_block_child_subtree_and_invalidates_owner_chain();
    test_native_owner_release_recursively_destroys_object_block_external_child_subtree_and_invalidates_owner_chain();
    test_inherited_owner_release_recursively_destroys_object_block_external_child_subtree_and_invalidates_owner_chain();
    test_native_release_detaches_external_base_contained_child_while_owner_and_sibling_remain_alive();
    test_inherited_external_base_release_detaches_external_base_contained_child_while_owner_and_sibling_remain_alive();
    test_native_release_detaches_external_base_contained_child_subtree_while_owner_and_sibling_remain_alive();
    test_inherited_external_base_release_detaches_external_base_contained_child_subtree_while_owner_and_sibling_remain_alive();
    test_native_owner_release_recursively_destroys_runtime_created_same_prg_child_subtree_and_invalidates_owner_chain();
    test_inherited_owner_release_recursively_destroys_runtime_created_same_prg_child_subtree_and_invalidates_owner_chain();
    test_native_owner_release_recursively_destroys_runtime_created_external_child_subtree_and_invalidates_owner_chain();
    test_inherited_owner_release_recursively_destroys_runtime_created_external_child_subtree_and_invalidates_owner_chain();
    test_native_release_detaches_declarative_external_base_child_subtree_while_owner_and_sibling_remain_alive();
    test_inherited_declarative_release_detaches_external_base_child_subtree_while_owner_and_sibling_remain_alive();
    test_native_release_detaches_declarative_same_prg_child_subtree_while_owner_and_sibling_remain_alive();
    test_inherited_same_prg_release_detaches_declarative_child_subtree_while_owner_and_sibling_remain_alive();
    test_native_owner_release_recursively_destroys_declarative_same_prg_child_subtree_and_invalidates_owner_chain();
    test_inherited_owner_release_recursively_destroys_declarative_same_prg_child_subtree_and_invalidates_owner_chain();
    test_native_owner_release_recursively_destroys_declarative_external_child_subtree_and_invalidates_owner_chain();
    test_inherited_owner_release_recursively_destroys_declarative_external_child_subtree_and_invalidates_owner_chain();
    test_native_release_detaches_object_block_child_subtree_while_owner_and_sibling_remain_alive();
    test_inherited_external_base_release_detaches_object_block_child_subtree_while_owner_and_sibling_remain_alive();
    test_native_release_detaches_object_block_external_child_subtree_while_owner_and_sibling_remain_alive();
    test_inherited_external_base_release_detaches_object_block_external_child_subtree_while_owner_and_sibling_remain_alive();
    test_newobject_with_explicit_prg_library_activates_native_class_and_preserves_explicit_targets();
    test_native_prg_object_methods_bind_this_and_persist_instance_state();
    test_native_prg_init_runs_during_object_creation_and_preserves_plain_creation();
    test_createobject_arguments_flow_into_native_init_while_newobject_and_non_native_creation_stay_stable();
    test_newobject_arguments_flow_into_native_init_while_createobject_and_com_newobject_stay_stable();
    test_native_object_method_and_init_preserve_by_reference_argument_updates();
    test_native_addobject_materializes_external_prg_child_objects_and_preserves_init_flow();
    test_native_addobject_external_child_base_surfaces_classlibrary_provenance();
    test_native_addobject_external_child_base_aclass_reflects_inheritance_chain();
    test_native_addobject_deeper_external_child_base_aclass_reflects_inheritance_chain();
    test_same_prg_native_class_inheritance_applies_parent_defaults_methods_and_init();
    test_native_class_inheritance_loads_external_prg_base_sources();
    test_same_prg_native_aclass_reflects_inheritance_chain();
    test_external_prg_base_aclass_reflects_inheritance_chain();
    test_same_prg_child_aclass_reflects_inheritance_chain();
    test_external_base_child_aclass_reflects_inheritance_chain();
    test_same_prg_inherited_child_aclass_reflects_deeper_inheritance_chain();
    test_external_base_inherited_child_aclass_reflects_deeper_inheritance_chain();
    test_same_prg_native_baseclass_reflects_through_getpem_and_pemstatus();
    test_external_prg_base_identity_reflects_through_getpem_and_pemstatus();
    test_same_prg_native_class_reflects_through_getpem_and_pemstatus();
    test_external_prg_base_class_reflects_through_getpem_and_pemstatus();
    test_same_prg_native_parentclass_reflects_through_getpem_and_pemstatus();
    test_external_prg_parentclass_reflects_through_getpem_and_pemstatus();
    test_same_prg_native_identity_metadata_appears_in_amembers();
    test_external_prg_base_identity_metadata_appears_in_amembers();
    test_same_prg_native_identity_metadata_stays_read_only_to_setpem();
    test_external_prg_identity_metadata_stays_read_only_to_setpem();
    test_same_prg_native_identity_metadata_stays_protected_from_removeproperty();
    test_external_prg_identity_metadata_stays_protected_from_removeproperty();
    test_same_prg_native_identity_metadata_cannot_be_shadowed_through_addproperty();
    test_external_prg_identity_metadata_cannot_be_shadowed_through_addproperty();
    test_same_prg_native_identity_metadata_cannot_be_shadowed_through_direct_assignment();
    test_external_prg_identity_metadata_cannot_be_shadowed_through_direct_assignment();
    test_same_prg_native_identity_metadata_reads_through_ordinary_properties();
    test_external_prg_identity_metadata_reads_through_ordinary_properties();
    test_same_prg_native_parentclass_reads_through_ordinary_properties();
    test_external_prg_parentclass_reads_through_ordinary_properties();
    test_same_prg_child_identity_metadata_reads_through_ordinary_properties();
    test_external_base_child_identity_metadata_reads_through_ordinary_properties();
    test_same_prg_child_identity_metadata_stays_read_only_to_setpem();
    test_external_base_child_identity_metadata_stays_read_only_to_setpem();
    test_same_prg_child_identity_metadata_cannot_be_shadowed_through_addproperty();
    test_external_base_child_identity_metadata_cannot_be_shadowed_through_addproperty();
    test_same_prg_child_identity_metadata_stays_protected_from_removeproperty();
    test_external_base_child_identity_metadata_stays_protected_from_removeproperty();
    test_same_prg_child_identity_metadata_cannot_be_shadowed_through_direct_assignment();
    test_external_base_child_identity_metadata_cannot_be_shadowed_through_direct_assignment();
    test_same_prg_child_parent_reflects_through_getpem_and_pemstatus();
    test_external_base_child_parent_reflects_through_getpem_and_pemstatus();
    test_same_prg_child_identity_reflects_through_getpem_and_pemstatus();
    test_external_base_child_identity_reflects_through_getpem_and_pemstatus();
    test_same_prg_child_external_base_provenance_surfaces_through_identity_metadata();
    test_external_base_child_external_base_provenance_surfaces_through_identity_metadata();
    test_same_prg_child_external_base_classlibrary_survives_identity_mutation_guards();
    test_external_base_child_external_base_classlibrary_survives_identity_mutation_guards();
    test_same_prg_child_deeper_external_base_provenance_stays_coherent();
    test_external_base_child_deeper_external_base_provenance_stays_coherent();
    test_same_prg_child_identity_metadata_appears_in_amembers();
    test_external_base_child_identity_metadata_appears_in_amembers();
    test_same_prg_child_parent_appears_in_amembers();
    test_external_base_child_parent_appears_in_amembers();
    test_same_prg_child_parent_stays_read_only_to_setpem();
    test_external_base_child_parent_stays_read_only_to_setpem();
    test_same_prg_child_parent_cannot_be_shadowed_through_addproperty();
    test_external_base_child_parent_cannot_be_shadowed_through_addproperty();
    test_same_prg_child_parent_stays_protected_from_removeproperty();
    test_external_base_child_parent_stays_protected_from_removeproperty();
    test_same_prg_child_parent_cannot_be_shadowed_through_direct_assignment();
    test_external_base_child_parent_cannot_be_shadowed_through_direct_assignment();
    test_inherited_declarative_children_from_external_prg_bases_resolve_against_defining_library();
    test_inherited_external_prg_base_methods_resolve_addobject_children_against_defining_library();
    test_inherited_external_base_addobject_external_child_base_surfaces_classlibrary_provenance();
    test_inherited_external_base_addobject_external_child_base_aclass_reflects_inheritance_chain();
    test_inherited_external_base_addobject_deeper_external_child_base_aclass_reflects_inheritance_chain();
    test_native_addobject_deeper_external_child_identity_surfaces_stay_coherent();
    test_inherited_external_base_addobject_deeper_external_child_identity_surfaces_stay_coherent();
    test_native_addobject_deeper_external_child_identity_metadata_stays_read_only_to_setpem();
    test_inherited_external_base_addobject_deeper_external_child_identity_metadata_stays_read_only_to_setpem();
    test_native_addobject_deeper_external_child_identity_metadata_cannot_be_shadowed_through_addproperty();
    test_inherited_external_base_addobject_deeper_external_child_identity_metadata_cannot_be_shadowed_through_addproperty();
    test_native_addobject_deeper_external_child_identity_metadata_cannot_be_erased_through_removeproperty();
    test_inherited_external_base_addobject_deeper_external_child_identity_metadata_cannot_be_erased_through_removeproperty();
    test_native_addobject_deeper_external_child_identity_metadata_cannot_be_shadowed_through_direct_assignment();
    test_inherited_external_base_addobject_deeper_external_child_identity_metadata_cannot_be_shadowed_through_direct_assignment();
    test_native_addobject_deeper_external_child_external_base_provenance_stays_coherent();
    test_inherited_external_base_addobject_deeper_external_child_external_base_provenance_stays_coherent();
    test_native_addobject_deeper_external_child_external_base_classlibrary_stays_read_only_to_setpem();
    test_inherited_external_base_addobject_deeper_external_child_external_base_classlibrary_stays_read_only_to_setpem();
    test_native_addobject_deeper_external_child_external_base_classlibrary_cannot_be_shadowed_through_addproperty();
    test_inherited_external_base_addobject_deeper_external_child_external_base_classlibrary_cannot_be_shadowed_through_addproperty();
    test_native_addobject_deeper_external_child_external_base_classlibrary_cannot_be_erased_through_removeproperty();
    test_inherited_external_base_addobject_deeper_external_child_external_base_classlibrary_cannot_be_erased_through_removeproperty();
    test_native_addobject_deeper_external_child_external_base_classlibrary_cannot_be_shadowed_through_direct_assignment();
    test_inherited_external_base_addobject_deeper_external_child_external_base_classlibrary_cannot_be_shadowed_through_direct_assignment();
    test_same_prg_native_dodefault_dispatches_base_methods_and_preserves_byref_init_flow();
    test_same_prg_native_bare_helper_calls_resolve_to_current_instance_before_top_level_routines();
    test_inherited_external_prg_base_methods_resolve_bare_helper_calls_against_defining_library();
    test_external_prg_base_methods_support_dodefault_dispatch();
    test_external_prg_base_methods_reflect_through_getpem_pemstatus_and_amembers();
    test_same_prg_native_access_assign_methods_virtualize_ordinary_property_reads_and_writes();
    test_native_accessor_backed_properties_reflect_through_getpem_pemstatus_and_amembers();
    test_external_prg_base_accessor_backed_properties_dispatch_and_reflect();
    test_native_accessor_backed_properties_setpem_routes_through_assign_methods();
    test_external_prg_base_accessor_backed_properties_setpem_routes_through_assign_methods();
    test_codepage_and_misc_runtime_surface_functions();
    test_lookup_expression_function();
    test_lookup_expression_function_supports_sql_cursors();
    test_lookup_supports_macro_alias_and_tag_arguments();
    test_lookup_supports_macro_return_and_search_expressions();
    test_lookup_supports_macro_arguments_on_sql_cursors();
    test_lookup_macro_return_expressions_preserve_typed_defaults_on_miss();
    test_createobject_property_assignment_round_trips();
    test_scripting_dictionary_collection_methods();
    test_newobject_preserves_library_and_server_targeting();
    test_getobject_reuses_existing_class_and_source_targets();

    if (test_failures() != 0)
    {
        std::cerr << test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
