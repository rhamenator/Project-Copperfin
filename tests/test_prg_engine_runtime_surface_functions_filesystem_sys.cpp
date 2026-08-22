#include "test_prg_engine_runtime_surface_functions_support.h"

#include <future>
#include <locale>
#include <set>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace copperfin::runtime_surface_tests
{
    namespace
    {
        class grouped_numpunct final : public std::numpunct<char>
        {
        protected:
            char do_decimal_point() const override { return ','; }
            char do_thousands_sep() const override { return '.'; }
            std::string do_grouping() const override { return "\3"; }
        };

        class global_locale_guard final
        {
        public:
            explicit global_locale_guard(const std::locale &replacement)
                : previous_(std::locale::global(replacement))
            {
            }

            ~global_locale_guard()
            {
                std::locale::global(previous_);
            }

            global_locale_guard(const global_locale_guard &) = delete;
            global_locale_guard &operator=(const global_locale_guard &) = delete;

        private:
            std::locale previous_;
        };
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
        const fs::path hidden_file_path = temp_root / ".hidden_file.txt";
        write_text(hidden_file_path, "hidden file");
#if defined(_WIN32)
        const DWORD hidden_file_attributes = ::GetFileAttributesW(hidden_file_path.c_str());
        expect(hidden_file_attributes != INVALID_FILE_ATTRIBUTES &&
                   ::SetFileAttributesW(
                       hidden_file_path.c_str(), hidden_file_attributes | FILE_ATTRIBUTE_HIDDEN) != 0,
               "FILE() hidden-attribute fixture should be marked Hidden on Windows");
#endif

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
            "lHiddenFileDefault = FILE('.hidden_file.txt')\n"
            "lHiddenFileZero = FILE('.hidden_file.txt', 0)\n"
            "lHiddenFileWithFlags = FILE('.hidden_file.txt', 1)\n"
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
        check("lhiddenfiledefault", "false");
        check("lhiddenfilezero", "false");
        check("lhiddenfilewithflags", "true");

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

    void test_directory_expression_function()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_directory";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path plain_dir = temp_root / "plain_dir";
        fs::create_directories(plain_dir);
        const fs::path nested_dir = temp_root / "nested" / "child_dir";
        fs::create_directories(nested_dir);
        const fs::path hidden_dir = temp_root / ".hidden_dir";
        fs::create_directories(hidden_dir);
#if defined(_WIN32)
        const DWORD hidden_dir_attributes = ::GetFileAttributesW(hidden_dir.c_str());
        expect(hidden_dir_attributes != INVALID_FILE_ATTRIBUTES &&
                   ::SetFileAttributesW(
                       hidden_dir.c_str(), hidden_dir_attributes | FILE_ATTRIBUTE_HIDDEN) != 0,
               "DIRECTORY() hidden-attribute fixture should be marked Hidden on Windows");
#endif
        const fs::path plain_file = temp_root / "plain_file.txt";
        write_text(plain_file, "not a directory");

        // Only discoverable via SET PATH -- DIRECTORY() must NOT search it,
        // unlike FILE()/FILESIZE(), per the mounted VFP9 help.
        const fs::path path_probe_dir = temp_root / "path_probe";
        const fs::path path_only_dir = path_probe_dir / "path_only_dir";
        fs::create_directories(path_only_dir);

        const fs::path main_path = temp_root / "directory_test.prg";
        write_text(
            main_path,
            "lPlainDir = DIRECTORY('plain_dir')\n"
            "lNestedDir = DIRECTORY('nested/child_dir')\n"
            "lAbsoluteDir = DIRECTORY('" + plain_dir.string() + "')\n"
            "lMissingDir = DIRECTORY('does-not-exist-xyz')\n"
            "lPlainFileAsDir = DIRECTORY('plain_file.txt')\n"
            "lHiddenDirDefault = DIRECTORY('.hidden_dir')\n"
            "lHiddenDirDefaultExplicit = DIRECTORY('.hidden_dir', 0)\n"
            "lHiddenDirShown = DIRECTORY('.hidden_dir', 1)\n"
            "lHiddenDirTrailingSlash = DIRECTORY('.hidden_dir/')\n"
            "lPathOnlyDirBefore = DIRECTORY('path_only_dir')\n"
            "SET PATH TO '" + path_probe_dir.string() + "'\n"
            "lPathOnlyDirAfter = DIRECTORY('path_only_dir')\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "DIRECTORY() test script should complete: " + state.message);

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

        check("lplaindir", "true");
        check("lnesteddir", "true");
        check("labsolutedir", "true");
        check("lmissingdir", "false");
        check("lplainfileasdir", "false");
        check("lhiddendirdefault", "false");
        check("lhiddendirdefaultexplicit", "false");
        check("lhiddendirshown", "true");
        check("lhiddendirtrailingslash", "false");
        check("lpathonlydirbefore", "false");
        check("lpathonlydirafter", "false");

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
        const std::locale grouping_locale(std::locale::classic(), new grouped_numpunct());
        global_locale_guard locale_guard(grouping_locale);
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

}
