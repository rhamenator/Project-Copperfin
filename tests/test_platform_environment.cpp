// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "copperfin/platform/environment.h"
#include "test_environment_support.h"
#include "test_locale_catalog_environment_support.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
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

void test_platform_environment_round_trips_values() {
    const std::string key = "COPPERFIN_TEST_PLATFORM_ENVIRONMENT";
    copperfin::test_support::ScopedEnvironmentValue scoped(key);

    expect(!copperfin::platform::read_environment_variable(key).has_value(),
           "#3214: clearing a test environment variable should leave no readable value");
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

void test_platform_environment_rejects_empty_names() {
    expect(!copperfin::platform::read_environment_variable("").has_value(),
           "#3214: shared platform environment helper should reject empty read keys");
    expect(!copperfin::platform::write_environment_variable("", "value"),
           "#3214: shared platform environment helper should reject empty write keys");
    expect(!copperfin::platform::clear_environment_variable(""),
           "#3214: shared platform environment helper should reject empty clear keys");
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
    const std::vector<std::pair<std::string, std::string>> invalid_dirs{
        {(root / "missing-locales").string(), "missing"},
        {(root / "locale-file").string(), "file-valued"},
        {(root / "incomplete-locales").string(), "incomplete"}
    };
    std::ofstream(root / "locale-file") << "not a locale root\n";
    fs::create_directories(root / "incomplete-locales" / "en-US");
    copperfin::test_support::ScopedEnvironmentValue original_locale("COPPERFIN_LOCALE");
    copperfin::test_support::ScopedEnvironmentValue original_locale_dir("COPPERFIN_LOCALE_DIR");
    {
        ScopedCurrentPath current_path(root / "nested" / "build" / "tests");

        for (const auto& [invalid_dir, description] : invalid_dirs) {
            copperfin::test_support::set_env_value("COPPERFIN_LOCALE_DIR", invalid_dir, true);
            {
                copperfin::test_support::ScopedDefaultLocaleCatalogEnvironment default_locale;
                expect(copperfin::test_support::getenv_value("COPPERFIN_LOCALE_DIR") == fallback_dir,
                       "#3997: " + description + " locale override should fall back to ancestor discovery");
            }
            expect(copperfin::test_support::getenv_value("COPPERFIN_LOCALE_DIR") == invalid_dir,
                   "#3997: " + description + " locale override should be restored after fallback scope");
        }

        copperfin::test_support::set_env_value("COPPERFIN_LOCALE_DIR", "", false);
        {
            copperfin::test_support::ScopedDefaultLocaleCatalogEnvironment default_locale;
            expect(copperfin::test_support::getenv_value("COPPERFIN_LOCALE_DIR") == fallback_dir,
                   "#3997: missing locale override should fall back to ancestor discovery");
        }
        expect(!copperfin::test_support::getenv_optional("COPPERFIN_LOCALE_DIR").has_value(),
               "#3997: missing locale override should remain missing after fallback scope");
    }

    std::error_code ignored;
    fs::remove_all(root, ignored);
}

}  // namespace

int main() {
    test_platform_environment_round_trips_values();
    test_platform_environment_rejects_empty_names();
    test_scoped_environment_support_uses_shared_platform_helpers();
    test_shell_command_preparation_preserves_platform_quoting_contract();
    test_default_locale_environment_preserves_valid_override_and_restores_values();
    test_default_locale_environment_falls_back_for_invalid_or_missing_override();

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return 1;
    }

    std::cout << "All tests passed.\n";
    return 0;
}
