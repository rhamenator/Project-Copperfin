#include "test_prg_engine_runtime_surface_functions_support.h"

#include <future>
#include <set>

namespace copperfin::runtime_surface_tests
{
    void test_array_element_native_property_expression_access()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_array_element_native_property";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "array_element_native_property.prg";
        write_text(
            main_path,
            "DIMENSION aObjects[1]\n"
            "aObjects[1] = CREATEOBJECT('ArrayPropertyWidget')\n"
            "wbcpOrigin = 'QFORM'\n"
            "lSuppressedMatch = .F. AND NOT LOWER(m.aObjects[1].BaseClass) == 'form'\n"
            "lQualifiedMatch = TYPE('m.wbcpOrigin') == 'C' AND UPPER(m.wbcpOrigin) == 'QFORM' AND NOT LOWER(m.aObjects[1].BaseClass) == 'form'\n"
            "lPlainMatch = NOT LOWER(aObjects[1].BaseClass) == 'form'\n"
            "RETURN\n"
            "DEFINE CLASS ArrayPropertyWidget AS Custom\n"
            "ENDDEFINE\n");

        const auto state = copperfin::runtime::PrgRuntimeSession::create(
                               make_runtime_session_options(main_path.string(), temp_root.string()))
                               .run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("array-element native-property expression script should complete: ") + state.message);

        const auto qualified = state.globals.find("lqualifiedmatch");
        const auto suppressed = state.globals.find("lsuppressedmatch");
        const auto plain = state.globals.find("lplainmatch");
        expect(suppressed != state.globals.end() &&
                   copperfin::runtime::format_value(suppressed->second) == "false",
               "short-circuited array-element property should be parsed without evaluation");
        expect(qualified != state.globals.end() &&
                   copperfin::runtime::format_value(qualified->second) == "true",
               "fully evaluated m.-qualified array-element property should resolve through LOWER()");
        expect(plain != state.globals.end() &&
                   copperfin::runtime::format_value(plain->second) == "true",
               "unparenthesized NOT should apply after the LOWER() comparison");

        fs::remove_all(temp_root, ignored);
    }

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
            "cSysCluster = SYS(2022)\n"
            "cSysClusterPath = SYS(2022, 'runtime_surface_extensions.prg')\n"
            "cSysClusterNestedPath = SYS(2022, 'runtime_surface_probe\\nested_probe.prg')\n"
            "cSysClusterMissing = SYS(2022, 'missing-cluster-path')\n"
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
            expected_home += static_cast<char>(fs::path::preferred_separator);
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
        const auto cluster_value = state.globals.find("csyscluster");
        const auto cluster_path_value = state.globals.find("csysclusterpath");
        const auto cluster_nested_path_value = state.globals.find("csysclusternestedpath");
        expect(cluster_value != state.globals.end() && cluster_value->second.kind == copperfin::runtime::PrgValueKind::string &&
                   std::stoull(copperfin::runtime::format_value(cluster_value->second)) > 0U,
               "SYS(2022) should return a positive cluster size as character text");
        expect(cluster_path_value != state.globals.end() && cluster_path_value->second.kind == copperfin::runtime::PrgValueKind::string &&
                   copperfin::runtime::format_value(cluster_path_value->second) == copperfin::runtime::format_value(cluster_value->second),
               "SYS(2022) should resolve a supplied path to the same filesystem cluster size");
        expect(cluster_nested_path_value != state.globals.end() &&
                   cluster_nested_path_value->second.kind == copperfin::runtime::PrgValueKind::string &&
                   copperfin::runtime::format_value(cluster_nested_path_value->second) ==
                       copperfin::runtime::format_value(cluster_value->second),
               "SYS(2022) should resolve a nested supplied file to the same filesystem cluster size");
        const auto cluster_missing_value = state.globals.find("csysclustermissing");
        expect(cluster_missing_value != state.globals.end() &&
                   cluster_missing_value->second.kind == copperfin::runtime::PrgValueKind::string &&
                   copperfin::runtime::format_value(cluster_missing_value->second) == "0",
               "SYS(2022) should return character zero for an unavailable path");

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
        const fs::path config_path = temp_root / "config.fpw";
        write_text(config_path, "CODEPAGE=1252\n");

        const std::string env_name = "COPPERFIN_RUNTIME_SURFACE_ENV_HELPER";
        const std::string unicode_env_name = "COPPERFIN_RUNTIME_SURFACE_ENV_UNICODE";
        const std::string unicode_env_value = "caf\xC3\xA9-\xE7\x8C\xAB";
        const fs::path main_path = temp_root / "env_and_sys_introspection.prg";
        write_text(
            main_path,
            "PUBLIC nPCountRoutine, nParametersRoutine\n"
            "nPCountMain = PCOUNT()\n"
            "nParametersMain = PARAMETERS()\n"
            "DO pcount_helper WITH 10, 'x', .T.\n"
            "lPutEnvSet = PUTENV('" + env_name + "', 'runtime-surface-value')\n"
            "cGetEnvSet = GETENV('" + env_name + "')\n"
            "lPutEnvUnicode = PUTENV('" + unicode_env_name + "', '" + unicode_env_value + "')\n"
            "cGetEnvUnicode = GETENV('" + unicode_env_name + "')\n"
            "lPutEnvClear = PUTENV('" + env_name + "', '')\n"
            "cGetEnvCleared = GETENV('" + env_name + "')\n"
            "cSys3 = SYS(3)\n"
            "cSys3Second = SYS(3)\n"
            "cSys2015 = SYS(2015)\n"
            "cSys2015Second = SYS(2015)\n"
            "cSys7 = SYS(7)\n"
            "cSys11 = SYS(11)\n"
            "cSys13 = SYS(13)\n"
            "cSys3004Default = SYS(3004)\n"
            "cSys3006German = SYS(3006, 1031)\n"
            "cSys3004German = SYS(3004)\n"
            "cSys3006Restore = SYS(3006, VAL(cSys3004Default))\n"
            "cSys3004Restored = SYS(3004)\n"
            "cSys3006NoArgument = SYS(3006)\n"
            "cSys2019 = SYS(2019)\n"
            "cSys2019External = SYS(2019, 1)\n"
            "cSys2019Internal = SYS(2019, 2)\n"
            "RETURN\n"
            "\n"
            "PROCEDURE pcount_helper\n"
            "LPARAMETERS p1, p2, p3, p4\n"
            "nPCountRoutine = PCOUNT()\n"
            "nParametersRoutine = PARAMETERS()\n"
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
        check("nparametersmain", "0");
        check("nparametersroutine", "3");
        check("lputenvset", "true");
        check("cgetenvset", "runtime-surface-value");
        check("lputenvunicode", "true");
        check("cgetenvunicode", unicode_env_value);
        check("lputenvclear", "true");
        check("cgetenvcleared", "");
        check("csys3004default", "1033");
        check("csys3006german", "");
        check("csys3004german", "1031");
        check("csys3006restore", "");
        check("csys3004restored", "1033");
        check("csys3006noargument", "");
        check("csys2019", config_path.string());
        check("csys2019external", config_path.string());
        check("csys2019internal", "");
        const auto sys2019_value = state.globals.find("csys2019");
        expect(sys2019_value != state.globals.end() &&
                   sys2019_value->second.kind == copperfin::runtime::PrgValueKind::string,
               "SYS(2019) should retain its documented character return type");
        check("csys11", "0");
        check("csys13", "0");

        const auto sys3_value = state.globals.find("csys3");
        const auto sys3_second_value = state.globals.find("csys3second");
        if (sys3_value == state.globals.end() || sys3_second_value == state.globals.end())
        {
            expect(false, "SYS(3) should expose two temporary filename values");
        }
        else
        {
            const std::string first = copperfin::runtime::format_value(sys3_value->second);
            const std::string second = copperfin::runtime::format_value(sys3_second_value->second);
            const auto legal_name = [](const std::string& value) {
                return value.size() == 8U &&
                    std::isdigit(static_cast<unsigned char>(value.front())) != 0 &&
                    std::all_of(value.begin(), value.end(), [](unsigned char ch) {
                        return std::isdigit(ch) != 0;
                    });
            };
            expect(legal_name(first), "SYS(3) should return an eight-digit legal filename component");
            expect(legal_name(second), "a second SYS(3) value should remain a legal filename component");
            expect(first != second, "successive SYS(3) calls should not collide in one runtime session");
        }

        const auto sys2015_value = state.globals.find("csys2015");
        const auto sys2015_second_value = state.globals.find("csys2015second");
        if (sys2015_value == state.globals.end() || sys2015_second_value == state.globals.end())
        {
            expect(false, "SYS(2015) should expose two unique procedure names");
        }
        else
        {
            const std::string first = copperfin::runtime::format_value(sys2015_value->second);
            const std::string second = copperfin::runtime::format_value(sys2015_second_value->second);
            const auto legal_name = [](const std::string& value) {
                return value.size() == 10U && value.front() == '_' &&
                    std::all_of(value.begin() + 1, value.end(), [](unsigned char ch) {
                        return std::isdigit(ch) != 0 ||
                            (ch >= 'A' && ch <= 'Z');
                    });
            };
            expect(legal_name(first), "SYS(2015) should return a ten-character VFP identifier");
            expect(legal_name(second), "a second SYS(2015) value should remain a legal VFP identifier");
            expect(first != second, "successive SYS(2015) calls should not collide in one runtime session");
        }

        const auto sys7_value = state.globals.find("csys7");
        expect(sys7_value != state.globals.end() && !copperfin::runtime::format_value(sys7_value->second).empty(),
               "SYS(7) should expose a non-empty host descriptor");

        const auto collect_concurrent_values = [&](long long sys_code)
        {
            constexpr std::size_t worker_count = 16U;
            const fs::path unique_value_path =
                temp_root / ("concurrent_unique_value_" + std::to_string(sys_code) + ".prg");
            write_text(
                unique_value_path,
                "cValue = SYS(" + std::to_string(sys_code) + ")\nRETURN\n");
            std::vector<std::future<std::string>> workers;
            workers.reserve(worker_count);
            for (std::size_t worker = 0U; worker < worker_count; ++worker)
            {
                workers.emplace_back(std::async(
                    std::launch::async,
                    [unique_value_path, temp_root]()
                    {
                        const fs::path source_path = unique_value_path;
                        const auto concurrent_state = copperfin::runtime::PrgRuntimeSession::create(
                                                          make_runtime_session_options(
                                                              source_path.string(),
                                                              temp_root.string()))
                                                          .run(copperfin::runtime::DebugResumeAction::continue_run);
                        const auto value = concurrent_state.globals.find("cvalue");
                        return concurrent_state.completed && value != concurrent_state.globals.end()
                            ? copperfin::runtime::format_value(value->second)
                            : std::string{};
                    }));
            }

            std::set<std::string> values;
            for (auto &worker : workers)
            {
                const std::string value = worker.get();
                expect(!value.empty(), "concurrent SYS generator session should complete");
                values.insert(value);
            }
            return values.size();
        };

        expect(collect_concurrent_values(3) == 16U,
               "concurrent SYS(3) calls should produce distinct values");
        expect(collect_concurrent_values(2015) == 16U,
               "concurrent SYS(2015) calls should produce distinct values");

        const fs::path isolated_path = temp_root / "sys3004_isolated.prg";
        write_text(isolated_path, "cLocale = SYS(3004)\nRETURN\n");
        const auto isolated_state = copperfin::runtime::PrgRuntimeSession::create(
                                         make_runtime_session_options(isolated_path.string(), temp_root.string()))
                                         .run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(isolated_state.completed, "a fresh runtime session should complete SYS(3004) isolation check");
        const auto isolated_locale = isolated_state.globals.find("clocale");
        expect(isolated_locale != isolated_state.globals.end() &&
                   copperfin::runtime::format_value(isolated_locale->second) == "1033",
               "SYS(3006) changes must remain isolated to the originating runtime session");

        const fs::path no_config_root = temp_root / "sys2019_no_config";
        fs::create_directories(no_config_root);
        const fs::path no_config_path = no_config_root / "sys2019_no_config.prg";
        write_text(no_config_path, "cConfig = SYS(2019)\nRETURN\n");
        const auto no_config_state = copperfin::runtime::PrgRuntimeSession::create(
                                          make_runtime_session_options(no_config_path.string(), no_config_root.string()))
                                          .run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(no_config_state.completed, "a no-config runtime session should complete SYS(2019) isolation check");
        const auto no_config_value = no_config_state.globals.find("cconfig");
        expect(no_config_value != no_config_state.globals.end() &&
                   copperfin::runtime::format_value(no_config_value->second).empty(),
               "SYS(2019) should not inherit another session's configuration path");

        fs::remove_all(temp_root, ignored);
    }

    void test_sys2029_reports_dbf_table_type()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sys2029";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path table_path = temp_root / "physical.dbf";
        write_simple_dbf(table_path, {"ALPHA"});
        const fs::path main_path = temp_root / "sys2029.prg";
        write_text(
            main_path,
            "nNoTable = SYS(2029)\n"
            "USE 'physical.dbf' ALIAS Physical2029\n"
            "nCurrentTableType = SYS(2029)\n"
            "nAliasTableType = SYS(2029, 'physical2029')\n"
            "nUnknownTableType = SYS(2029, 'missing2029')\n"
            "USE IN Physical2029\n"
            "nAfterCloseTableType = SYS(2029)\n"
            "RETURN\n");

        const auto state = copperfin::runtime::PrgRuntimeSession::create(
                               make_runtime_session_options(main_path.string(), temp_root.string()))
                               .run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("SYS(2029) script should complete: ") + state.message);

        const auto check_number = [&](const std::string& name, double expected)
        {
            const auto found = state.globals.find(name);
            expect(found != state.globals.end(), name + " should be assigned");
            if (found != state.globals.end())
            {
                expect(found->second.kind == copperfin::runtime::PrgValueKind::number,
                       name + " should retain numeric SYS(2029) type");
                expect(found->second.number_value == expected,
                       name + " expected numeric value " + std::to_string(expected) + " got " +
                           std::to_string(found->second.number_value));
            }
        };

        check_number("nnotable", 0.0);
        check_number("ncurrenttabletype", 48.0);
        check_number("naliastabletype", 48.0);
        check_number("nunknowntabletype", 0.0);
        check_number("nafterclosetabletype", 0.0);

        fs::remove_all(temp_root, ignored);
    }

    void test_sys2014_returns_minimum_runtime_path()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sys2014";
        const fs::path base_path = temp_root / "base";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(base_path / "nested");

        const fs::path main_path = base_path / "sys2014.prg";
        write_text(
            main_path,
            "cCurrent = SYS(2014, 'nested/child.prg')\n"
            "cParent = SYS(2014, '../outside/file.prg')\n"
            "cExplicit = SYS(2014, 'nested/child.prg', '.')\n"
            "cExplicitParent = SYS(2014, '../outside/file.prg', 'nested')\n"
            "cMissing = SYS(2014, 'not-created/file.prg')\n"
            "RETURN\n");

        const auto state = copperfin::runtime::PrgRuntimeSession::create(
                               make_runtime_session_options(main_path.string(), base_path.string()))
                               .run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("SYS(2014) script should complete: ") + state.message);

        const std::string separator(1U, fs::path::preferred_separator);
        const auto check = [&](const std::string& name, const std::string& expected)
        {
            const auto found = state.globals.find(name);
            expect(found != state.globals.end(), name + " should be assigned");
            if (found != state.globals.end())
            {
                expect(copperfin::runtime::format_value(found->second) == expected,
                       name + " expected '" + expected + "' got '" +
                           copperfin::runtime::format_value(found->second) + "'");
            }
        };

        check("ccurrent", "nested" + separator + "child.prg");
        check("cparent", ".." + separator + "outside" + separator + "file.prg");
        check("cexplicit", "nested" + separator + "child.prg");
        check("cexplicitparent", ".." + separator + ".." + separator + "outside" + separator + "file.prg");
        check("cmissing", "not-created" + separator + "file.prg");

        fs::remove_all(temp_root, ignored);
    }

    void test_sys2000_enumerates_wildcard_file_matches()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sys2000";
        const fs::path base_path = temp_root / "base";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(base_path / "nested");
        write_text(base_path / "a.txt", "a");
        write_text(base_path / "B.txt", "b");
        write_text(base_path / "ignore.prg", "prg");
        write_text(base_path / "nested" / "child.txt", "child");

        const fs::path main_path = base_path / "sys2000.prg";
        write_text(
            main_path,
            "cFirst = SYS(2000, '*.txt')\n"
            "cSecond = SYS(2000, '*.txt', 1)\n"
            "cExhausted = SYS(2000, '*.txt', 1)\n"
            "cReset = SYS(2000, '*.txt')\n"
            "cSessionOneBeforeSwitch = SYS(2000, '*.txt', 1)\n"
            "SET DATASESSION TO 2\n"
            "cSessionTwoFirst = SYS(2000, '*.txt')\n"
            "cSessionTwoSecond = SYS(2000, '*.txt', 1)\n"
            "SET DATASESSION TO 1\n"
            "cSessionOneAfterSwitch = SYS(2000, '*.txt', 1)\n"
            "cPrg = SYS(2000, '*.prg')\n"
            "cMissing = SYS(2000, 'missing/*.txt')\n"
            "cNested = SYS(2000, 'nested/*.txt')\n"
            "RETURN\n");

        const auto state = copperfin::runtime::PrgRuntimeSession::create(
                               make_runtime_session_options(main_path.string(), base_path.string()))
                               .run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("SYS(2000) script should complete: ") + state.message);

        const auto check = [&](const std::string& name, const std::string& expected)
        {
            const auto found = state.globals.find(name);
            expect(found != state.globals.end(), name + " should be assigned");
            if (found != state.globals.end())
            {
                expect(copperfin::runtime::format_value(found->second) == expected,
                       name + " expected '" + expected + "' got '" +
                           copperfin::runtime::format_value(found->second) + "'");
            }
        };

        check("cfirst", "a.txt");
        check("csecond", "B.txt");
        check("cexhausted", "");
        check("creset", "a.txt");
        check("csessiononebeforeswitch", "B.txt");
        check("csessiontwofirst", "a.txt");
        check("csessiontwosecond", "B.txt");
        check("csessiononeafterswitch", "");
        check("cprg", "ignore.prg");
        check("cmissing", "");
        check("cnested", "child.txt");

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

    void test_program_reports_active_name_and_stack_depth()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_program_stack_introspection";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "program_stack.prg";
        write_text(
            main_path,
            "cMainProgram = PROGRAM()\n"
            "nMainDepth = PROGRAM(-1)\n"
            "PUBLIC cNestedProgram, nNestedDepth\n"
            "DO CaptureProgramStack\n"
            "RETURN\n"
            "PROCEDURE CaptureProgramStack\n"
            "cNestedProgram = PROGRAM()\n"
            "nNestedDepth = PROGRAM(-1)\n"
            "RETURN\n");

        const auto state = copperfin::runtime::PrgRuntimeSession::create(
                               make_runtime_session_options(main_path.string(), temp_root.string()))
                               .run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               "PROGRAM() stack-introspection script should complete: " + state.message);

        const auto value = [&](const std::string& name)
        {
            const auto it = state.globals.find(name);
            expect(it != state.globals.end(), name + " should be captured");
            return it == state.globals.end()
                       ? std::string{}
                       : copperfin::runtime::format_value(it->second);
        };
        expect(value("cmainprogram") == "main",
               "PROGRAM() should report the active main routine");
        expect(value("cnestedprogram") == "CaptureProgramStack",
               "PROGRAM() should report the active nested routine");
        const long long main_depth = std::stoll(value("nmaindepth"));
        const long long nested_depth = std::stoll(value("nnesteddepth"));
        expect(main_depth >= 1, "PROGRAM(-1) should report at least the active frame");
        expect(nested_depth == main_depth + 1,
               "PROGRAM(-1) should increase by one for a nested DO frame");

        fs::remove_all(temp_root, ignored);
    }

    void test_indexed_program_and_sys16_stack_introspection()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_indexed_program_stack_introspection";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "indexed_program_stack.prg";
        write_text(
            main_path,
            "cMainName0 = PROGRAM(0)\n"
            "cMainName1 = PROGRAM(1)\n"
            "cMainName2 = PROGRAM(2)\n"
            "cMainPath0 = SYS(16, 0)\n"
            "cMainPath1 = SYS(16, 1)\n"
            "cMainPath2 = SYS(16, 2)\n"
            "PUBLIC cNestedName0, cNestedName1, cNestedName2, cNestedName3, "
            "cNestedPath0, cNestedPath1, cNestedPath2, cNestedPath3\n"
            "DO CaptureIndexedProgramStack\n"
            "RETURN\n"
            "PROCEDURE CaptureIndexedProgramStack\n"
            "cNestedName0 = PROGRAM(0)\n"
            "cNestedName1 = PROGRAM(1)\n"
            "cNestedName2 = PROGRAM(2)\n"
            "cNestedName3 = PROGRAM(3)\n"
            "cNestedPath0 = SYS(16, 0)\n"
            "cNestedPath1 = SYS(16, 1)\n"
            "cNestedPath2 = SYS(16, 2)\n"
            "cNestedPath3 = SYS(16, 3)\n"
            "RETURN\n");

        const auto state = copperfin::runtime::PrgRuntimeSession::create(
                               make_runtime_session_options(main_path.string(), temp_root.string()))
                               .run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               "indexed PROGRAM/SYS(16) stack-introspection script should complete: " + state.message);

        const auto value = [&](const std::string &name)
        {
            const auto it = state.globals.find(name);
            expect(it != state.globals.end(), name + " should be captured");
            return it == state.globals.end()
                       ? std::string{}
                       : copperfin::runtime::format_value(it->second);
        };
        expect(value("cmainname0") == "main", "PROGRAM(0) should report the master routine");
        expect(value("cmainname1") == "main", "PROGRAM(1) should report the master routine");
        expect(value("cmainname2").empty(), "out-of-range PROGRAM(2) should return an empty string");
        expect(value("cnestedname0") == "main", "nested PROGRAM(0) should report the master routine");
        expect(value("cnestedname1") == "main", "nested PROGRAM(1) should report the master routine");
        expect(value("cnestedname2") == "CaptureIndexedProgramStack",
               "nested PROGRAM(2) should report the active routine at level two");
        expect(value("cnestedname3").empty(), "out-of-range nested PROGRAM(3) should be empty");

        const auto expect_same_source_file = [&](const std::string &name)
        {
            expect(fs::path(value(name)).filename() == main_path.filename(),
                   name + " should resolve to the executing PRG file");
        };
        expect_same_source_file("cmainpath0");
        expect_same_source_file("cmainpath1");
        expect(value("cmainpath2").empty(), "out-of-range SYS(16,2) should return an empty string");
        expect_same_source_file("cnestedpath0");
        expect_same_source_file("cnestedpath1");
        expect_same_source_file("cnestedpath2");
        expect(value("cnestedpath3").empty(), "out-of-range nested SYS(16,3) should be empty");

        fs::remove_all(temp_root, ignored);
    }

    void test_sys16_preserves_procedure_context()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_sys16_procedure_context";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "sys16_procedure_context.prg";
        write_text(
            main_path,
            "cEntryPath = SYS(16)\n"
            "cEntryIndexedPath = SYS(16, 1)\n"
            "PUBLIC cCurrentContext, cIndexedContext, cIndexedEntryPath\n"
            "DO CaptureSys16Context\n"
            "RETURN\n"
            "PROCEDURE CaptureSys16Context\n"
            "cCurrentContext = SYS(16)\n"
            "cIndexedContext = SYS(16, 2)\n"
            "cIndexedEntryPath = SYS(16, 1)\n"
            "RETURN\n");

        const auto state = copperfin::runtime::PrgRuntimeSession::create(
                               make_runtime_session_options(main_path.string(), temp_root.string()))
                               .run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               "SYS(16) procedure-context script should complete: " + state.message);

        const auto value = [&](const std::string& name)
        {
            const auto it = state.globals.find(name);
            expect(it != state.globals.end(), name + " should be captured");
            return it == state.globals.end()
                       ? std::string{}
                       : copperfin::runtime::format_value(it->second);
        };
        const std::string expected_context =
            "PROCEDURE CaptureSys16Context " + main_path.string();
        expect(value("centrypath") == main_path.string(),
               "entry SYS(16) should remain a plain file path");
        expect(value("centryindexedpath") == main_path.string(),
               "entry indexed SYS(16) should remain a plain file path");
        expect(value("ccurrentcontext") == expected_context,
               "current SYS(16) should preserve procedure context");
        expect(value("cindexedcontext") == expected_context,
               "indexed SYS(16) should preserve procedure context");
        expect(value("cindexedentrypath") == main_path.string(),
               "indexed SYS(16) should preserve entry-frame path behavior");

        fs::remove_all(temp_root, ignored);
    }

    void test_singular_lparameter_binds_object_method_text_for_concatenation()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_singular_lparameter_object_method";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "singular_lparameter.prg";
        write_text(
            main_path,
            "oAppender = CREATEOBJECT('Appender')\n"
            "oAppender.Append('first')\n"
            "oAppender.Append('second')\n"
            "cLog = oAppender.Log\n"
            "RETURN\n"
            "DEFINE CLASS Appender AS Custom\n"
            "    Log = ''\n"
            "    PROCEDURE Append\n"
            "        LPARAMETER tcText\n"
            "        THIS.Log = THIS.Log + CHR(13) + m.tcText\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        const auto state = copperfin::runtime::PrgRuntimeSession::create(
                               make_runtime_session_options(main_path.string(), temp_root.string()))
                               .run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               "singular LPARAMETER object-method script should complete: " + state.message);
        const auto log = state.globals.find("clog");
        expect(log != state.globals.end(), "singular LPARAMETER object method should expose its log");
        if (log != state.globals.end())
        {
            expect(copperfin::runtime::format_value(log->second) == "\rfirst\rsecond",
                   "singular LPARAMETER should bind text for repeated character concatenation (got " +
                       copperfin::runtime::format_value(log->second) + ")");
        }

        fs::remove_all(temp_root, ignored);
    }

}
