#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_expression_runtime_surface_extensions()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_runtime_surface_extensions";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);
        const fs::path nested_probe_dir = temp_root / "runtime_surface_probe";
        fs::create_directories(nested_probe_dir);
        write_text(nested_probe_dir / "nested_probe.prg", "RETURN\n");

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
            "nBackslashDiskSpace = DISKSPACE('runtime_surface_probe\\nested_probe.prg')\n"
            "lBackslashDiskSpacePositive = nBackslashDiskSpace > 0\n"
            "nBackslashDriveType = DRIVETYPE('runtime_surface_probe\\nested_probe.prg')\n"
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
        std::string expected_home = temp_root.string();
        if (!expected_home.empty() && expected_home.back() != '/' && expected_home.back() != '\\') {
            expected_home += fs::path::preferred_separator;
        }
        check("chome", expected_home);
        check("ldiskspacepositive", "true");
        check("ndrivetype", "3");
        check("nmissingdrivetype", "0");
        check("lbackslashdiskspacepositive", "true");
        check("nbackslashdrivetype", "3");
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
        const fs::path nested_path_probe_dir = path_probe_dir / "nested";
        fs::create_directories(nested_path_probe_dir);
        const fs::path nested_path_only_file_path = nested_path_probe_dir / "path_only_backslash.txt";
        const std::string nested_path_only_content = "Found through backslash path";
        write_text(nested_path_only_file_path, nested_path_only_content);
        const fs::path direct_directory = temp_root / "direct_directory.txt";
        const fs::path nested_directory = temp_root / "relative" / "nested_directory.txt";
        const fs::path absolute_directory = temp_root / "absolute_directory.txt";
        const fs::path path_only_directory = path_probe_dir / "path_only_directory.txt";
        const fs::path nested_path_only_directory = nested_path_probe_dir / "path_only_directory.txt";
        const fs::path shadowing_directory = temp_root / "shadowed.txt";
        fs::create_directories(direct_directory);
        fs::create_directories(nested_directory);
        fs::create_directories(absolute_directory);
        fs::create_directories(path_only_directory);
        fs::create_directories(nested_path_only_directory);
        fs::create_directories(shadowing_directory);
        const std::string shadowed_file_content = "SET PATH regular file";
        write_text(path_probe_dir / "shadowed.txt", shadowed_file_content);

        const fs::path main_path = temp_root / "filesize_test.prg";
        write_text(
            main_path,
            "cTestFile = 'testfile.txt'\n"
            "nFileSize = FILESIZE(cTestFile)\n"
            "nMissingFile = FILESIZE('missing-file.txt')\n"
            "nEmptyArg = FILESIZE()\n"
            "nAbsolutePath = FILESIZE('" + test_file_path.string() + "')\n"
            "lDirectDirectory = FILE('direct_directory.txt')\n"
            "lRelativeDirectory = FILE('relative/nested_directory.txt')\n"
            "lAbsoluteDirectory = FILE('" + absolute_directory.string() + "')\n"
            "lAbsoluteFile = FILE('" + test_file_path.string() + "')\n"
            "lEmptyFile = FILE('')\n"
            "lPathFileBefore = FILE('path_only.txt')\n"
            "SET PATH TO '" + path_probe_dir.string() + "'\n"
            "lPathFileAfter = FILE('path_only.txt')\n"
            "lPathDirectory = FILE('path_only_directory.txt')\n"
            "lPathShadowedFile = FILE('shadowed.txt')\n"
            "nPathFileSize = FILESIZE('path_only.txt')\n"
            "nShadowedFileSize = FILESIZE('shadowed.txt')\n"
            "lBackslashPathFile = FILE('nested\\path_only_backslash.txt')\n"
            "lBackslashPathDirectory = FILE('nested\\path_only_directory.txt')\n"
            "nBackslashPathFileSize = FILESIZE('nested\\path_only_backslash.txt')\n"
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

        // FILE() should reject directories regardless of how the path is resolved.
        check("ldirectdirectory", "false");
        check("lrelativedirectory", "false");
        check("labsolutedirectory", "false");
        check("labsolutefile", "true");
        check("lemptyfile", "false");

        // SET PATH resolution should make path-only file discoverable
        check("lpathfilebefore", "false");
        check("lpathfileafter", "true");
        check("lpathdirectory", "false");
        check("lpathshadowedfile", "true");
        check("npathfilesize", std::to_string(path_only_content.length()));
        check("nshadowedfilesize", std::to_string(shadowed_file_content.length()));
        check("lbackslashpathfile", "true");
        check("lbackslashpathdirectory", "false");
        check("nbackslashpathfilesize", std::to_string(nested_path_only_content.length()));

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
        const std::string unicode_env_name = "COPPERFIN_RUNTIME_SURFACE_ENV_UNICODE";
        const std::string unicode_env_value = "caf\xC3\xA9-\xE7\x8C\xAB";
        const fs::path main_path = temp_root / "env_and_sys_introspection.prg";
        write_text(
            main_path,
            "PUBLIC nPCountRoutine\n"
            "nPCountMain = PCOUNT()\n"
            "DO pcount_helper WITH 10, 'x', .T.\n"
            "lPutEnvSet = PUTENV('" + env_name + "', 'runtime-surface-value')\n"
            "cGetEnvSet = GETENV('" + env_name + "')\n"
            "lPutEnvUnicode = PUTENV('" + unicode_env_name + "', '" + unicode_env_value + "')\n"
            "cGetEnvUnicode = GETENV('" + unicode_env_name + "')\n"
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
        check("lputenvunicode", "true");
        check("cgetenvunicode", unicode_env_value);
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
            "nMembersComFlagged = AMEMBERS(aMembersComFlagged, oOne, 0, 'G')\n"
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

        check("nmembersall", "13");
        check("nmembersproperties", "2");
        check("nmembersmethods", "11");
        check("nmembersevents", "0");
        check("nmembersunion", "13");
        check("nmemberscomflagged", "13");
        check("cmemberallfirst", "ADD");
        check("cmemberalllast", "WRITEMETHOD");
        check("cmembermethodsfirst", "ADD");
        check("cmembermethodslast", "WRITEMETHOD");
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

    void test_native_prg_amembers_visibility_filters()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_amembers_visibility";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_prg_amembers_visibility.prg";
        write_text(
            main_path,
            "oDemo = CREATEOBJECT('VisibilityDemo')\n"
            "nAll = AMEMBERS(aAll, oDemo, 0)\n"
            "nProtected = AMEMBERS(aProtected, oDemo, 0, 'P')\n"
            "nHidden = AMEMBERS(aHidden, oDemo, 0, 'H')\n"
            "nPublic = AMEMBERS(aPublic, oDemo, 0, 'G')\n"
            "nProtectedProperties = AMEMBERS(aProtectedProperties, oDemo, 1, 'P')\n"
            "nHiddenProperties = AMEMBERS(aHiddenProperties, oDemo, 1, 'H')\n"
            "nPublicProperties = AMEMBERS(aPublicProperties, oDemo, 1, 'G')\n"
            "nProtectedMethods = AMEMBERS(aProtectedMethods, oDemo, 2, 'P')\n"
            "nHiddenMethods = AMEMBERS(aHiddenMethods, oDemo, 2, 'H')\n"
            "nPublicMethods = AMEMBERS(aPublicMethods, oDemo, 2, 'G')\n"
            "nUnknownFlag = AMEMBERS(aUnknownFlag, oDemo, 0, 'X')\n"
            "nCompoundFlag = AMEMBERS(aCompoundFlag, oDemo, 0, 'PH')\n"
            "cAllFirst = aAll[1]\n"
            "cAllSecond = aAll[2]\n"
            "cAllThird = aAll[3]\n"
            "cAllFourth = aAll[4]\n"
            "cAllFifth = aAll[5]\n"
            "cAllSixth = aAll[6]\n"
            "cAllSeventh = aAll[7]\n"
            "cAllEighth = aAll[8]\n"
            "cAllNinth = aAll[9]\n"
            "cAllTenth = aAll[10]\n"
            "cAllEleventh = aAll[11]\n"
            "cAllTwelfth = aAll[12]\n"
            "cAllThirteenth = aAll[13]\n"
            "cAllFourteenth = aAll[14]\n"
            "cAllFifteenth = aAll[15]\n"
            "cAllSixteenth = aAll[16]\n"
            "lAllHasProtected = ASCAN(aAll, 'PROTECTEDVALUE') > 0 AND ASCAN(aAll, 'PROTECTEDMETHOD') > 0\n"
            "lAllHasHidden = ASCAN(aAll, 'HIDDENVALUE') > 0 AND ASCAN(aAll, 'HIDDENMETHOD') > 0\n"
            "lAllHasPublic = ASCAN(aAll, 'PUBLICVALUE') > 0 AND ASCAN(aAll, 'PUBLICMETHOD') > 0\n"
            "lProtectedOnly = ASCAN(aProtected, 'PROTECTEDVALUE') > 0 AND ASCAN(aProtected, 'PROTECTEDMETHOD') > 0 AND ASCAN(aProtected, 'PUBLICVALUE') = 0 AND ASCAN(aProtected, 'HIDDENVALUE') = 0\n"
            "lHiddenOnly = ASCAN(aHidden, 'HIDDENVALUE') > 0 AND ASCAN(aHidden, 'HIDDENMETHOD') > 0 AND ASCAN(aHidden, 'PUBLICVALUE') = 0 AND ASCAN(aHidden, 'PROTECTEDVALUE') = 0\n"
            "lPublicOnly = ASCAN(aPublic, 'PUBLICVALUE') > 0 AND ASCAN(aPublic, 'PUBLICMETHOD') > 0 AND ASCAN(aPublic, 'PROTECTEDVALUE') = 0 AND ASCAN(aPublic, 'HIDDENVALUE') = 0\n"
            "lAllCount = nAll = 16 AND nProtected = 2 AND nHidden = 2 AND nPublic = 12\n"
            "lFallbackFlags = nUnknownFlag = nAll AND nCompoundFlag = nAll AND aUnknownFlag[1] = aAll[1] AND aUnknownFlag[nUnknownFlag] = aAll[nAll] AND aCompoundFlag[1] = aAll[1] AND aCompoundFlag[nCompoundFlag] = aAll[nAll]\n"
            "lPropertyModes = nProtectedProperties >= 1 AND nHiddenProperties >= 1 AND nPublicProperties >= 1 AND ASCAN(aProtectedProperties, 'PROTECTEDVALUE') > 0 AND ASCAN(aHiddenProperties, 'HIDDENVALUE') > 0 AND ASCAN(aPublicProperties, 'PUBLICVALUE') > 0\n"
            "lMethodModes = nProtectedMethods >= 1 AND nHiddenMethods >= 1 AND nPublicMethods >= 1 AND ASCAN(aProtectedMethods, 'PROTECTEDMETHOD') > 0 AND ASCAN(aHiddenMethods, 'HIDDENMETHOD') > 0 AND ASCAN(aPublicMethods, 'PUBLICMETHOD') > 0\n"
            "RETURN\n"
            "DEFINE CLASS VisibilityDemo AS Custom\n"
            "    PROTECTED ProtectedValue\n"
            "    HIDDEN HiddenValue\n"
            "    ProtectedValue = 1\n"
            "    HiddenValue = 2\n"
            "    PublicValue = 3\n"
            "    PROTECTED PROCEDURE ProtectedMethod\n"
            "        RETURN THIS.ProtectedValue\n"
            "    ENDPROC\n"
            "    HIDDEN FUNCTION HiddenMethod\n"
            "        RETURN THIS.HiddenValue\n"
            "    ENDFUNC\n"
            "    PROCEDURE PublicMethod\n"
            "        RETURN THIS.PublicValue\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native AMEMBERS visibility script should complete: ") + state.message +
                   " @line=" + std::to_string(state.location.line));

        const auto check = [&](const std::string& name, const std::string& expected) {
            const auto found = state.globals.find(name);
            expect(found != state.globals.end(), name + " variable should be present");
            if (found != state.globals.end()) {
                expect(copperfin::runtime::format_value(found->second) == expected,
                       name + " expected '" + expected + "' got '" +
                           copperfin::runtime::format_value(found->second) + "'");
            }
        };

        check("lallcount", "true");
        check("lallhasprotected", "true");
        check("lallhashidden", "true");
        check("lallhaspublic", "true");
        check("lprotectedonly", "true");
        check("lhiddenonly", "true");
        check("lpubliconly", "true");
        check("lpropertymodes", "true");
        check("lmethodmodes", "true");
        check("lfallbackflags", "true");
        check("callfirst", "BASECLASS");
        check("callsecond", "CLASS");
        check("callthird", "HIDDENMETHOD");
        check("callfourth", "HIDDENVALUE");
        check("callfifth", "PARENTCLASS");
        check("callsixth", "PROTECTEDMETHOD");
        check("callseventh", "PROTECTEDVALUE");
        check("calleighth", "PUBLICMETHOD");
        check("callninth", "PUBLICVALUE");
        check("calltenth", "READEXPRESSION");
        check("calleleventh", "READMETHOD");
        check("calltwelfth", "REFRESH");
        check("callthirteenth", "RELEASE");
        check("callfourteenth", "RESETTODEFAULT");
        check("callfifteenth", "WRITEEXPRESSION");
        check("callsixteenth", "WRITEMETHOD");

        fs::remove_all(temp_root, ignored);
    }

    void test_common_native_oop_function_abbreviations()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_oop_function_abbreviations";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_oop_function_abbreviations.prg";
        write_text(
            main_path,
            "oAliasEmpty = CREATEOBJ('Empty')\n"
            "lAliasAdd = ADDPROP(oAliasEmpty, 'ShortProp', 42)\n"
            "lAliasHasProp = PEMSTATUS(oAliasEmpty, 'ShortProp', 1)\n"
            "nAliasPropValue = oAliasEmpty.ShortProp\n"
            "oAliasWidget = CREATEOBJ('AliasWidget')\n"
            "cAliasCaption = oAliasWidget.Caption\n"
            "cAliasClass = oAliasWidget.Class\n"
            "cAliasBaseClass = oAliasWidget.BaseClass\n"
            "cAliasDescribe = oAliasWidget.Describe('abbr')\n"
            "RETURN\n"
            "DEFINE CLASS AliasWidget AS Custom\n"
            "    Caption = 'AliasWidget'\n"
            "    FUNCTION Describe\n"
            "        LPARAMETERS tcPrefix\n"
            "        RETURN tcPrefix + ':' + THIS.Caption\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("common native OOP abbreviation script should complete: ") + state.message +
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

        check("laliasadd", "true");
        check("laliashasprop", "true");
        check("naliaspropvalue", "42");
        check("caliascaption", "AliasWidget");
        check("caliasclass", "AliasWidget");
        check("caliasbaseclass", "Custom");
        check("caliasdescribe", "abbr:AliasWidget");

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
            "lWriteBackslashOk = CURSORTOXML('SourceXml', 'xml_store\\round_trip.xml')\n"
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
            "nLoadedFromBackslashFile = XMLTOCURSOR('xml_store\\round_trip.xml', 'DestBackslash')\n"
            "SELECT DestBackslash\n"
            "nBackslashFileCount = RECCOUNT()\n"
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
        check("lwritebackslashok", "true");
        check("nloaded", "3");
        check("nloadedfromfile", "3");
        check("nloadedfrombackslashfile", "3");
        check("ndestcount", "3");
        check("nfilecount", "3");
        check("nbackslashfilecount", "3");
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

    void test_cursor_xml_verified_file_bytes_are_enforced()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_cursor_xml_verified_bytes";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path xml_path = temp_root / "verified.xml";
        const std::string verified_xml =
            "<CopperfinCursor alias=\"Verified\">\n"
            "  <Fields>\n"
            "    <Field name=\"ID\" type=\"N\" width=\"4\" decimals=\"0\" />\n"
            "    <Field name=\"NAME\" type=\"C\" width=\"20\" decimals=\"0\" />\n"
            "  </Fields>\n"
            "  <Rows>\n"
            "    <Row><Col>1</Col><Col>VERIFIED</Col></Row>\n"
            "  </Rows>\n"
            "</CopperfinCursor>\n";
        const std::string tampered_xml =
            "<CopperfinCursor alias=\"Tampered\">\n"
            "  <Fields>\n"
            "    <Field name=\"ID\" type=\"N\" width=\"4\" decimals=\"0\" />\n"
            "    <Field name=\"NAME\" type=\"C\" width=\"20\" decimals=\"0\" />\n"
            "  </Fields>\n"
            "  <Rows>\n"
            "    <Row><Col>9</Col><Col>TAMPERED</Col></Row>\n"
            "    <Row><Col>10</Col><Col>UNTRUSTED</Col></Row>\n"
            "  </Rows>\n"
            "</CopperfinCursor>\n";
        write_text(xml_path, tampered_xml);

        const fs::path strict_program = temp_root / "strict.prg";
        write_text(
            strict_program,
            "nLoaded = XMLTOCURSOR('verified.xml', 'Verified')\n"
            "nCount = RECCOUNT('Verified')\n"
            "RETURN\n");
        copperfin::runtime::RuntimeSessionOptions strict_options =
            make_runtime_session_options(strict_program.string(), temp_root.string());
        strict_options.verified_file_byte_overrides.emplace(xml_path.string(), verified_xml);
        strict_options.require_verified_file_byte_overrides = true;
        const auto strict_state = copperfin::runtime::PrgRuntimeSession::create(strict_options)
                                       .run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(strict_state.completed,
               "strict XMLTOCURSOR verified-byte script should complete: " + strict_state.message);
        const std::string strict_loaded = copperfin::runtime::format_value(strict_state.globals.at("nloaded"));
        const std::string strict_count = copperfin::runtime::format_value(strict_state.globals.at("ncount"));
        std::string strict_events;
        for (const auto &event : strict_state.events)
        {
            strict_events += event.category + ":" + event.detail + ";";
        }
        expect(strict_loaded == "1",
               "strict XMLTOCURSOR should load the verified row count (got " + strict_loaded + ", events=" + strict_events + ")");
        expect(strict_count == "1",
               "strict XMLTOCURSOR should ignore tampered disk rows (got " + strict_count + ", events=" + strict_events + ")");

        const fs::path missing_program = temp_root / "missing.prg";
        write_text(missing_program, "nLoaded = XMLTOCURSOR('verified.xml', 'Missing')\nRETURN\n");
        copperfin::runtime::RuntimeSessionOptions missing_options =
            make_runtime_session_options(missing_program.string(), temp_root.string());
        missing_options.require_verified_file_byte_overrides = true;
        const auto missing_state = copperfin::runtime::PrgRuntimeSession::create(missing_options)
                                       .run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(missing_state.completed,
               "strict XMLTOCURSOR missing-byte script should fail safely: " + missing_state.message);
        expect(copperfin::runtime::format_value(missing_state.globals.at("nloaded")) == "0",
               "strict XMLTOCURSOR should reject an unverified file path");
        expect(std::any_of(missing_state.events.begin(), missing_state.events.end(), [](const auto &event)
        {
            return event.category == "runtime.warning" &&
                   event.detail.find("Verified package bytes are unavailable") != std::string::npos;
        }),
               "strict XMLTOCURSOR rejection should emit the verified-byte warning");

        const fs::path non_strict_program = temp_root / "non_strict.prg";
        write_text(
            non_strict_program,
            "nLoaded = XMLTOCURSOR('verified.xml', 'Tampered')\n"
            "nCount = RECCOUNT('Tampered')\n"
            "RETURN\n");
        const auto non_strict_state = copperfin::runtime::PrgRuntimeSession::create(
                                           make_runtime_session_options(non_strict_program.string(), temp_root.string()))
                                           .run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(non_strict_state.completed,
               "non-strict XMLTOCURSOR file script should complete: " + non_strict_state.message);
        expect(copperfin::runtime::format_value(non_strict_state.globals.at("nloaded")) == "2" &&
                   copperfin::runtime::format_value(non_strict_state.globals.at("ncount")) == "2",
               "non-strict XMLTOCURSOR should preserve direct file loading");

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

}
