// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/localization/localization.h"
#include "copperfin/platform/environment.h"
#include "copperfin/platform/executable_path.h"
#include "test_environment_support.h"
#include "test_locale_catalog_environment_support.h"

#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

bool paths_are_filesystem_equivalent(
    const std::filesystem::path& actual,
    const std::filesystem::path& expected) {
    std::error_code error;
    return std::filesystem::equivalent(actual, expected, error) && !error;
}

void test_platform_environment_round_trips_values() {
    const std::string key = "COPPERFIN_TEST_PLATFORM_ENVIRONMENT";
    copperfin::test_support::ScopedEnvironmentValue scoped(key);

    expect(!copperfin::platform::read_environment_variable(key).has_value(),
           "#3214: clearing a test environment variable should leave no readable value");
    expect(copperfin::platform::write_environment_variable(key, ""),
           "#4010: shared platform environment helper should accept an empty assignment");
    const auto empty = copperfin::platform::read_environment_variable(key);
#if defined(_WIN32)
    expect(!empty.has_value(),
           "#4010: Windows empty assignment should preserve _putenv_s removal semantics");
#else
    expect(empty.has_value() && empty->empty(),
           "#4010: POSIX empty assignment should remain distinct from a missing variable");
#endif
    expect(copperfin::platform::write_environment_variable(key, "alpha beta"),
           "#3214: shared platform environment helper should set variables");

    const auto assigned = copperfin::platform::read_environment_variable(key);
    expect(assigned.has_value() && *assigned == "alpha beta",
           "#3214: shared platform environment helper should read back assigned values");

    expect(copperfin::platform::clear_environment_variable(key),
           "#3214: shared platform environment helper should clear variables");
    expect(!copperfin::platform::read_environment_variable(key).has_value(),
           "#3214: cleared environment variables should not report stale values");
}

void test_platform_environment_round_trips_unicode_values() {
    const std::string key = "COPPERFIN_TEST_PLATFORM_ENVIRONMENT_UNICODE";
    const std::string value = "caf\xC3\xA9-\xE7\x8C\xAB";
    copperfin::test_support::ScopedEnvironmentValue scoped(key);

    expect(copperfin::platform::write_environment_variable(key, value),
           "#4318: shared environment helper should accept UTF-8 values");
    const auto assigned = copperfin::platform::read_environment_variable(key);
    expect(assigned.has_value() && *assigned == value,
           "#4318: shared environment helper should round-trip UTF-8 values exactly");

    expect(copperfin::platform::clear_environment_variable(key),
           "#4318: Unicode environment fixture should clear through the shared helper");
}

void test_platform_environment_rejects_empty_names() {
    expect(!copperfin::platform::read_environment_variable("").has_value(),
           "#3214: shared platform environment helper should reject empty read keys");
    expect(!copperfin::platform::write_environment_variable("", "value"),
           "#3214: shared platform environment helper should reject empty write keys");
    expect(!copperfin::platform::clear_environment_variable(""),
           "#3214: shared platform environment helper should reject empty clear keys");
    expect(!copperfin::platform::read_environment_path("").has_value(),
           "#4005: filesystem environment helper should reject empty read keys");
    expect(!copperfin::platform::write_environment_path("", std::filesystem::path("value")),
           "#4005: filesystem environment helper should reject empty write keys");
    expect(!copperfin::platform::clear_environment_path(""),
           "#4005: filesystem environment helper should reject empty clear keys");
}

void test_platform_environment_rejects_unsafe_names_and_embedded_nuls() {
    const std::string invalid_name = "COPPERFIN_TEST_PLATFORM_ENVIRONMENT=INVALID";
    const std::string embedded_nul_name =
        std::string("COPPERFIN_TEST_PLATFORM_ENVIRONMENT") + '\0' + "TRUNCATED";
    const std::string embedded_nul_value = std::string("value") + '\0' + "TRUNCATED";
    const std::filesystem::path embedded_nul_path(
        std::string("path") + '\0' + "TRUNCATED");

    expect(!copperfin::platform::read_environment_variable(invalid_name).has_value(),
           "#3214: environment reads should reject names containing '='");
    expect(!copperfin::platform::write_environment_variable(invalid_name, "value"),
           "#3214: environment writes should reject names containing '='");
    expect(!copperfin::platform::clear_environment_variable(invalid_name),
           "#3214: environment clears should reject names containing '='");
    expect(!copperfin::platform::read_environment_variable(embedded_nul_name).has_value(),
           "#3214: environment reads should reject embedded-NUL names");
    expect(!copperfin::platform::write_environment_variable(embedded_nul_name, "value"),
           "#3214: environment writes should reject embedded-NUL names");
    expect(!copperfin::platform::write_environment_variable(
               "COPPERFIN_TEST_PLATFORM_ENVIRONMENT_NUL_VALUE", embedded_nul_value),
           "#3214: environment writes should reject embedded-NUL values");
    expect(!copperfin::platform::read_environment_path(invalid_name).has_value(),
           "#4005: filesystem environment reads should reject invalid names");
    expect(!copperfin::platform::write_environment_path(
               invalid_name, std::filesystem::path("value")),
           "#4005: filesystem environment writes should reject invalid names");
    expect(!copperfin::platform::write_environment_path(
               "COPPERFIN_TEST_PLATFORM_ENVIRONMENT_NUL_PATH", embedded_nul_path),
           "#4005: filesystem environment writes should reject embedded-NUL paths");
    expect(!copperfin::platform::clear_environment_path(invalid_name),
           "#4005: filesystem environment clears should reject invalid names");
}

void test_running_executable_path_resolves_current_process(const char* invocation_path) {
    namespace fs = std::filesystem;

    const fs::path resolved = copperfin::platform::resolve_running_executable_path(
        invocation_path == nullptr ? fs::path() : fs::path(invocation_path));
    std::error_code type_error;
    expect(!resolved.empty() && fs::is_regular_file(resolved, type_error),
           "#4013: running executable discovery should resolve the current process image");

    if (invocation_path != nullptr && *invocation_path != '\0') {
        const fs::path invocation =
            copperfin::platform::resolve_executable_invocation_path(invocation_path);
        std::error_code equivalent_error;
        expect(!invocation.empty() && fs::equivalent(resolved, invocation, equivalent_error),
               "#4013: OS process-image discovery should agree with the launched test path");
    }
}

void test_scoped_environment_support_uses_shared_platform_helpers() {
    const std::string key = "COPPERFIN_TEST_PLATFORM_ENVIRONMENT_SCOPED";
    copperfin::test_support::ScopedEnvironmentValue original(key);
    expect(copperfin::platform::write_environment_variable(key, "original"),
           "#3214: shared platform environment helper should seed scoped test fixtures");

    {
        copperfin::test_support::ScopedEnvironmentValue override_value(key, "override");
        const auto during_scope = copperfin::platform::read_environment_variable(key);
        expect(during_scope.has_value() && *during_scope == "override",
               "#3214: scoped test environment helper should expose the temporary override");
    }

    const auto restored = copperfin::platform::read_environment_variable(key);
    expect(restored.has_value() && *restored == "original",
           "#3214: scoped test environment helper should restore the original environment value");
}

void test_platform_environment_serializes_concurrent_access() {
    const std::string key = "COPPERFIN_TEST_PLATFORM_ENVIRONMENT_CONCURRENT";
    const std::string alpha = "alpha:" + std::string(1024, 'A');
    const std::string beta = "beta:" + std::string(1024, 'B');
    constexpr int writer_count = 2;
    constexpr int reader_count = 4;
    constexpr int writer_iterations = 20000;
    copperfin::test_support::ScopedEnvironmentValue scoped(key);

    std::atomic<int> ready_count{0};
    std::atomic<int> readers_started{0};
    std::atomic<int> writers_remaining{writer_count};
    std::atomic<std::size_t> read_count{0};
    std::atomic<bool> start{false};
    std::atomic<bool> operation_failed{false};
    std::atomic<bool> invalid_value_observed{false};
    std::vector<std::thread> threads;
    threads.reserve(writer_count + reader_count);

    for (int writer_index = 0; writer_index < writer_count; ++writer_index) {
        threads.emplace_back([&, writer_index]() {
            ready_count.fetch_add(1);
            while (!start.load()) {
                std::this_thread::yield();
            }

            for (int iteration = 0; iteration < writer_iterations; ++iteration) {
                bool succeeded = false;
                switch ((iteration + writer_index) % 4) {
                    case 0:
                        succeeded = copperfin::platform::write_environment_variable(key, alpha);
                        break;
                    case 1:
                        succeeded = copperfin::platform::write_environment_variable(key, beta);
                        break;
                    case 2:
                        succeeded = copperfin::platform::write_environment_variable(key, "");
                        break;
                    default:
                        succeeded = copperfin::platform::clear_environment_variable(key);
                        break;
                }
                if (!succeeded) {
                    operation_failed.store(true);
                }
                if (iteration == 0) {
                    while (readers_started.load() != reader_count) {
                        std::this_thread::yield();
                    }
                }
            }
            writers_remaining.fetch_sub(1);
        });
    }

    for (int reader_index = 0; reader_index < reader_count; ++reader_index) {
        threads.emplace_back([&]() {
            ready_count.fetch_add(1);
            while (!start.load()) {
                std::this_thread::yield();
            }

            bool announced = false;
            do {
                const auto value = copperfin::platform::read_environment_variable(key);
                if (value.has_value() && *value != alpha && *value != beta && !value->empty()) {
                    invalid_value_observed.store(true);
                }
                read_count.fetch_add(1);
                if (!announced) {
                    readers_started.fetch_add(1);
                    announced = true;
                }
            } while (writers_remaining.load() > 0);
        });
    }

    while (ready_count.load() != writer_count + reader_count) {
        std::this_thread::yield();
    }
    start.store(true);

    for (auto& thread : threads) {
        thread.join();
    }

    expect(!operation_failed.load(),
           "#4010: concurrent shared-helper writes and clears should succeed");
    expect(!invalid_value_observed.load(),
           "#4010: concurrent reads should observe only complete present, empty, or missing values");
    expect(read_count.load() >= reader_count,
           "#4010: concurrent stress coverage should execute reads while writers are active");
}

void test_shell_command_preparation_preserves_platform_quoting_contract() {
    const std::string command = "\"C:\\Copperfin Tests\\host.exe\" \"argument value\" > \"output file.txt\"";
    const std::string prepared = copperfin::test_support::prepare_shell_command_for_system(command);
#if defined(_WIN32)
    expect(prepared == "\"" + command + "\"",
           "#3896: Windows system commands should wrap the complete already-quoted command line");
#else
    expect(prepared == command,
           "#3896: POSIX system commands should preserve their existing shell command line");
#endif
}

std::filesystem::path make_locale_fixture_root(const std::string& name) {
    namespace fs = std::filesystem;

    const fs::path root =
        fs::temp_directory_path() /
        (name + "_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    std::error_code ignored;
    fs::remove_all(root, ignored);
    fs::create_directories(root / "resources" / "locales" / "en-US");
    std::ofstream(root / "resources" / "locales" / "en-US" / "strings.json") << "{}\n";
    fs::create_directories(root / "nested" / "build" / "tests");
    return root;
}

struct ScopedCurrentPath {
    std::filesystem::path original_path = std::filesystem::current_path();

    explicit ScopedCurrentPath(const std::filesystem::path& path) {
        std::filesystem::current_path(path);
    }

    ~ScopedCurrentPath() {
        std::error_code ignored;
        std::filesystem::current_path(original_path, ignored);
    }
};

#if !defined(_WIN32)
void test_posix_path_unset_and_empty_components() {
    namespace fs = std::filesystem;

    const auto default_path = copperfin::platform::default_posix_search_path();
    expect(default_path.has_value() &&
               (default_path->find("/bin") != std::string::npos ||
                default_path->find("/usr/bin") != std::string::npos),
           "#4372: POSIX default search path should remain available when PATH is unset");

    const fs::path root = fs::temp_directory_path() /
        ("copperfin_path_search_" +
         std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    const std::string probe_name = "copperfin_path_probe_" +
        std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    const fs::path probe_path = root / probe_name;
    std::error_code fixture_error;
    fs::create_directories(root, fixture_error);
    expect(!fixture_error, "#4338: POSIX PATH fixture directory should be created");
    std::ofstream(probe_path) << "#!/bin/sh\nexit 0\n";
    fs::permissions(
        probe_path,
        fs::perms::owner_exec | fs::perms::group_exec | fs::perms::others_exec,
        fs::perm_options::add,
        fixture_error);
    expect(!fixture_error, "#4338: POSIX PATH fixture should be executable");

    {
        ScopedCurrentPath current_path(root);
        copperfin::test_support::ScopedEnvironmentValue path("PATH");
        const fs::path unset_result =
            copperfin::platform::resolve_executable_invocation_path(probe_name);
        expect(unset_result == fs::path(probe_name),
               "#4338: an unset POSIX PATH should not resolve a bare invocation from the current directory");

        path.set("");
        const fs::path empty_result =
            copperfin::platform::resolve_executable_invocation_path(probe_name);
        expect(paths_are_filesystem_equivalent(empty_result, probe_path),
               "#4338: an explicit empty POSIX PATH component should search the current directory");
    }

    fs::remove_all(root, fixture_error);
}
#endif

void test_default_locale_environment_preserves_valid_override_and_restores_values() {
    namespace fs = std::filesystem;

    const fs::path override_root = make_locale_fixture_root("copperfin_valid_locale_override_tests");
    const fs::path fallback_root = make_locale_fixture_root("copperfin_valid_locale_fallback_tests");
    const std::string locale_dir = (override_root / "resources" / "locales").string();
    copperfin::test_support::ScopedEnvironmentValue original_locale("COPPERFIN_LOCALE");
    copperfin::test_support::ScopedEnvironmentValue original_locale_dir("COPPERFIN_LOCALE_DIR");
    copperfin::test_support::set_env_value("COPPERFIN_LOCALE", "pt-BR", true);
    copperfin::test_support::set_env_value("COPPERFIN_LOCALE_DIR", locale_dir, true);

    {
        ScopedCurrentPath current_path(fallback_root / "nested" / "build" / "tests");
        copperfin::test_support::ScopedDefaultLocaleCatalogEnvironment default_locale;
        expect(copperfin::test_support::getenv_value("COPPERFIN_LOCALE") == "en-US",
               "#3997: default locale scope should select en-US");
        expect(copperfin::test_support::getenv_value("COPPERFIN_LOCALE_DIR") == locale_dir,
               "#3997: default locale scope should preserve a valid explicit catalog root");
    }

    expect(copperfin::test_support::getenv_value("COPPERFIN_LOCALE") == "pt-BR",
           "#3997: default locale scope should restore the caller locale");
    expect(copperfin::test_support::getenv_value("COPPERFIN_LOCALE_DIR") == locale_dir,
           "#3997: default locale scope should restore the caller catalog root");
    std::error_code ignored;
    fs::remove_all(override_root, ignored);
    fs::remove_all(fallback_root, ignored);
}

void test_default_locale_environment_falls_back_for_invalid_or_missing_override() {
    namespace fs = std::filesystem;

    const fs::path root = make_locale_fixture_root("copperfin_locale_override_fallback_tests");
    const std::string fallback_dir = (root / "resources" / "locales").string();
    std::vector<std::pair<std::string, std::string>> invalid_dirs{
        {(root / "missing-locales").string(), "missing"},
        {(root / "locale-file").string(), "file-valued"},
        {(root / "incomplete-locales").string(), "incomplete"}
    };
    std::ofstream(root / "locale-file") << "not a locale root\n";
    fs::create_directories(root / "incomplete-locales" / "en-US");
#if !defined(_WIN32)
    const fs::path inaccessible_root = root / "inaccessible-locales";
    const fs::path inaccessible_catalog = inaccessible_root / "en-US" / "strings.json";
    fs::create_directories(inaccessible_catalog.parent_path());
    std::ofstream(inaccessible_catalog) << "{}\n";
    std::error_code permission_error;
    fs::permissions(
        inaccessible_catalog,
        fs::perms::none,
        fs::perm_options::replace,
        permission_error);
    expect(!permission_error,
           "#3997: inaccessible locale override fixture should remove catalog permissions");
    invalid_dirs.emplace_back(inaccessible_root.string(), "inaccessible");
#endif
    copperfin::test_support::ScopedEnvironmentValue original_locale("COPPERFIN_LOCALE");
    copperfin::test_support::ScopedEnvironmentValue original_locale_dir("COPPERFIN_LOCALE_DIR");
    copperfin::test_support::set_env_value("COPPERFIN_LOCALE", "pt-BR", true);
    {
        ScopedCurrentPath current_path(root / "nested" / "build" / "tests");

        for (const auto& [invalid_dir, description] : invalid_dirs) {
            copperfin::test_support::set_env_value("COPPERFIN_LOCALE_DIR", invalid_dir, true);
            {
                copperfin::test_support::ScopedDefaultLocaleCatalogEnvironment default_locale;
                expect(paths_are_filesystem_equivalent(
                           copperfin::test_support::getenv_value("COPPERFIN_LOCALE_DIR"),
                           fallback_dir),
                       "#3997: " + description + " locale override should fall back to ancestor discovery");
            }
            expect(copperfin::test_support::getenv_value("COPPERFIN_LOCALE") == "pt-BR",
                   "#3997: " + description + " locale override should restore the caller locale");
            expect(copperfin::test_support::getenv_value("COPPERFIN_LOCALE_DIR") == invalid_dir,
                   "#3997: " + description + " locale override should be restored after fallback scope");
        }

        copperfin::test_support::set_env_value("COPPERFIN_LOCALE_DIR", "", false);
        {
            copperfin::test_support::ScopedDefaultLocaleCatalogEnvironment default_locale;
            expect(paths_are_filesystem_equivalent(
                       copperfin::test_support::getenv_value("COPPERFIN_LOCALE_DIR"),
                       fallback_dir),
                   "#3997: missing locale override should fall back to ancestor discovery");
        }
        expect(copperfin::test_support::getenv_value("COPPERFIN_LOCALE") == "pt-BR",
               "#3997: missing locale override should restore the caller locale");
        expect(!copperfin::test_support::getenv_optional("COPPERFIN_LOCALE_DIR").has_value(),
               "#3997: missing locale override should remain missing after fallback scope");
    }

    std::error_code ignored;
    fs::remove_all(root, ignored);
}

void test_locale_catalog_root_preserves_non_ascii_environment_paths(
    const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

#if defined(_WIN32)
    fs::path fixture_name = L"copperfin_\u0416_\u6F22_locale_tests_";
#else
    fs::path fixture_name = "copperfin_\xD0\x96_\xE6\xBC\xA2_locale_tests_";
#endif
    fixture_name += std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    const fs::path root = fs::temp_directory_path() / fixture_name;
    const fs::path locale_root = root / "resources" / "locales";
    const fs::path prior_value = root / "prior-locale-root";
    fs::create_directories(locale_root / "en-US");
    std::ofstream(locale_root / "en-US" / "strings.json")
        << "{\"Test.NonAsciiLocaleRoot\":\"loaded\","
           "\"RuntimeHost.Usage.Manifest\":\"UNICODE_LOCALE_ROOT {commandName}\"}\n";

    copperfin::test_support::ScopedEnvironmentPath original_locale_dir("COPPERFIN_LOCALE_DIR");
    expect(copperfin::platform::write_environment_path("COPPERFIN_LOCALE_DIR", prior_value),
           "#4005: filesystem environment helper should seed a prior path value");
    {
        copperfin::test_support::ScopedEnvironmentValue legacy_locale_dir("COPPERFIN_LOCALE_DIR");
        expect(!copperfin::platform::read_environment_path("COPPERFIN_LOCALE_DIR").has_value(),
               "#4005: legacy test scope should clear locale roots through the path boundary");
    }
    expect(copperfin::platform::read_environment_path("COPPERFIN_LOCALE_DIR") == prior_value,
           "#4005: legacy test scope should restore a Unicode path through the path boundary");
    {
        copperfin::test_support::ScopedEnvironmentPath configured_locale_dir(
            "COPPERFIN_LOCALE_DIR",
            locale_root);
        const auto configured = copperfin::platform::read_environment_path("COPPERFIN_LOCALE_DIR");
        expect(configured.has_value() && *configured == locale_root,
               "#4005: filesystem environment helper should round-trip a non-ASCII path exactly");
        expect(copperfin::localization::resolve_catalog_root() == locale_root,
               "#4005: locale-root resolution should preserve a non-ASCII configured path");
        const auto catalog = copperfin::localization::load_catalogs(locale_root, "en-US");
        expect(catalog.translate("Test.NonAsciiLocaleRoot") == "loaded",
               "#4005: localization should load a catalog beneath the non-ASCII configured root");
        {
            copperfin::test_support::ScopedDefaultLocaleCatalogEnvironment default_locale;
            expect(copperfin::test_support::getenv_path("COPPERFIN_LOCALE_DIR") == locale_root,
                   "#4005: default locale scope should preserve a usable non-ASCII catalog root");
        }
        expect(copperfin::platform::read_environment_path("COPPERFIN_LOCALE_DIR") == locale_root,
               "#4005: default locale scope should restore the exact non-ASCII path value");

        if (!runtime_host_path.empty()) {
            copperfin::test_support::ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "en-US");
            const fs::path output_path = fs::temp_directory_path() /
                ("copperfin_unicode_locale_host_" +
                 std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()) +
                 ".log");
            const std::string command =
                "\"" + runtime_host_path + "\" > \"" + output_path.string() + "\" 2>&1";
            (void)copperfin::test_support::run_shell_command(command);
            std::ifstream output_stream(output_path, std::ios::binary);
            const std::string output{
                std::istreambuf_iterator<char>(output_stream),
                std::istreambuf_iterator<char>()};
            expect(output.find("UNICODE_LOCALE_ROOT copperfin_runtime_host") != std::string::npos,
                   "#4005: runtime host should load usage text from the non-ASCII catalog root");
            std::error_code remove_error;
            fs::remove(output_path, remove_error);
        }
    }
    expect(copperfin::platform::read_environment_path("COPPERFIN_LOCALE_DIR") == prior_value,
           "#4005: filesystem environment scope should restore a present prior path exactly");

    expect(copperfin::platform::clear_environment_path("COPPERFIN_LOCALE_DIR"),
           "#4005: filesystem environment helper should clear the seeded prior path");
    {
        copperfin::test_support::ScopedEnvironmentPath configured_locale_dir(
            "COPPERFIN_LOCALE_DIR",
            locale_root);
        expect(copperfin::localization::resolve_catalog_root() == locale_root,
               "#4005: non-ASCII locale-root resolution should remain stable from a missing prior value");
    }
    expect(!copperfin::platform::read_environment_path("COPPERFIN_LOCALE_DIR").has_value(),
           "#4005: filesystem environment scope should restore a missing prior path as missing");

    std::error_code ignored;
    fs::remove_all(root, ignored);
}

}  // namespace

int main(int argc, char** argv) {
    test_platform_environment_round_trips_values();
    test_platform_environment_round_trips_unicode_values();
    test_platform_environment_rejects_empty_names();
    test_platform_environment_rejects_unsafe_names_and_embedded_nuls();
    test_running_executable_path_resolves_current_process(argc > 0 ? argv[0] : nullptr);
#if !defined(_WIN32)
    test_posix_path_unset_and_empty_components();
#endif
    test_scoped_environment_support_uses_shared_platform_helpers();
    test_platform_environment_serializes_concurrent_access();
    test_shell_command_preparation_preserves_platform_quoting_contract();
    test_default_locale_environment_preserves_valid_override_and_restores_values();
    test_default_locale_environment_falls_back_for_invalid_or_missing_override();
    test_locale_catalog_root_preserves_non_ascii_environment_paths(argc > 1 ? argv[1] : std::string());

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return 1;
    }

    std::cout << "All tests passed.\n";
    return 0;
}
