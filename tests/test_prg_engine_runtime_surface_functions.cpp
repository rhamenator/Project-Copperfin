#include "copperfin/runtime/prg_engine.h"
#include "prg_engine_test_support.h"
#include "copperfin/vfp/dbf_table.h"

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
            "SET PATH TO '/tmp/copperfin'\n"
            "cPathValue = SET('PATH')\n"
            "ON ERROR DO somehandler\n"
            "cOnErrorHandler = ON('ERROR')\n"
            "ON SHUTDOWN CLOSE DATABASES ALL\n"
            "cOnShutdownHandler = ON('SHUTDOWN')\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
            .startup_path = main_path.string(),
            .working_directory = temp_root.string(),
            .stop_on_entry = false
        });

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
        check("cpathvalue", "TO '/tmp/copperfin'");
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

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
            .startup_path = main_path.string(),
            .working_directory = temp_root.string(),
            .stop_on_entry = false
        });

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

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
            .startup_path = main_path.string(),
            .working_directory = temp_root.string(),
            .stop_on_entry = false
        });

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

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
            .startup_path = main_path.string(),
            .working_directory = temp_root.string(),
            .stop_on_entry = false
        });

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

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
            .startup_path = main_path.string(),
            .working_directory = temp_root.string(),
            .stop_on_entry = false
        });

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

} // namespace

int main()
{
    test_expression_runtime_surface_extensions();
    test_filesize_expression_function();
    test_recsize_reclength_expression_functions();
    test_environment_and_sys_introspection_functions();
    test_object_reflection_runtime_surface_functions();

    if (test_failures() != 0)
    {
        std::cerr << test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
