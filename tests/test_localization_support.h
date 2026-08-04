#pragma once
// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "copperfin/localization/localization.h"
#include "copperfin/platform/extensibility_model.h"
#include "copperfin/runtime/runtime_pipeline.h"
#include "copperfin/runtime/prg_engine.h"
#include "copperfin/security/security_model.h"
#include "copperfin/studio/project_workspace.h"
#include "prg_engine_test_support.h"
#include "test_environment_support.h"
#include "test_locale_catalog_environment_support.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#if defined(_WIN32)
#include <cstdio>
#else
#include <cstdio>
#endif

namespace {

using namespace copperfin::test_support;

class ScopedTempDirectory {
public:
    explicit ScopedTempDirectory(const std::string& name) {
        static std::size_t sequence = 0U;
        root_ = std::filesystem::temp_directory_path() /
            (name + "_" +
             std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()) + "_" +
             std::to_string(sequence++));
        std::filesystem::create_directories(root_);
    }

    ~ScopedTempDirectory() {
        std::error_code ignored;
        std::filesystem::remove_all(root_, ignored);
    }

    const std::filesystem::path& path() const {
        return root_;
    }

private:
    std::filesystem::path root_;
};

class ScopedCurrentPath {
public:
    explicit ScopedCurrentPath(const std::filesystem::path& path)
        : original_path_(std::filesystem::current_path()) {
        std::filesystem::current_path(path);
    }

    ~ScopedCurrentPath() {
        std::error_code ignored;
        std::filesystem::current_path(original_path_, ignored);
    }

private:
    std::filesystem::path original_path_;
};

#if !defined(_WIN32)
class ScopedPathPermissions {
public:
    explicit ScopedPathPermissions(const std::filesystem::path& path)
        : path_(path) {
        std::error_code error;
        original_permissions_ = std::filesystem::status(path_, error).permissions();
        active_ = !error;
    }

    bool replace(std::filesystem::perms permissions) {
        std::error_code error;
        std::filesystem::permissions(
            path_,
            permissions,
            std::filesystem::perm_options::replace,
            error);
        return !error;
    }

    void restore() {
        if (!active_) {
            return;
        }
        std::error_code ignored;
        std::filesystem::permissions(
            path_,
            original_permissions_,
            std::filesystem::perm_options::replace,
            ignored);
        active_ = false;
    }

    ~ScopedPathPermissions() {
        restore();
    }

private:
    std::filesystem::path path_;
    std::filesystem::perms original_permissions_ = std::filesystem::perms::unknown;
    bool active_ = false;
};
#endif

void write_catalog(const std::filesystem::path& root, const std::string& locale, const std::string& json) {
    const std::filesystem::path locale_root = root / locale;
    std::filesystem::create_directories(locale_root);
    write_text(locale_root / "strings.json", json);
}

[[maybe_unused]] std::string shell_quote(const std::string& value) {
#ifdef _WIN32
    return "\"" + value + "\"";
#else
    std::string quoted = "'";
    for (const char ch : value) {
        if (ch == '\'') {
            quoted += "'\\''";
        } else {
            quoted.push_back(ch);
        }
    }
    quoted += "'";
    return quoted;
#endif
}

[[maybe_unused]] std::string run_command_capture(const std::string& command) {
    std::string output;
    const std::string prepared_command = copperfin::test_support::prepare_shell_command_for_system(command);
#ifdef _WIN32
    FILE* pipe = _popen(prepared_command.c_str(), "r");
#else
    FILE* pipe = popen(prepared_command.c_str(), "r");
#endif
    if (pipe == nullptr) {
        return output;
    }
    char buffer[256];
    while (std::fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        output += buffer;
    }
#ifdef _WIN32
    _pclose(pipe);
#else
    pclose(pipe);
#endif
    return output;
}

[[maybe_unused]] void seed_test_catalogs(const std::filesystem::path& root) {
    write_catalog(
        root,
        "en-US",
        "{\n"
        "  \"Command.Inspect\": \"Inspect\",\n"
        "  \"Diagnostic.ExpectedTokenBeforeToken\": \"Expected {expectedToken} before {actualToken}.\",\n"
        "  \"Help.LocaleOption\": \"Select the user-interface locale.\",\n"
        "  \"Inspect.Usage\": \"Usage: {commandName} [{localeOption} {localeValue}] {assetPathArgument}\",\n"
        "  \"Test.Unicode\": \"Cafe \xC3\xA9 \xD0\x9F\xD1\x80\xD0\xB8\xD0\xB2\xD0\xB5\xD1\x82 {name}\"\n"
        "}\n");
    write_catalog(root, "es-419", "{}\n");
    write_catalog(root, "pt-BR", "{}\n");
    write_catalog(root, "qps-ploc", "{}\n");
}


}  // namespace

using namespace copperfin::test_support;
