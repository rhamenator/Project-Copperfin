// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#ifndef COPPERFIN_TEST_ENVIRONMENT_SUPPORT_H
#define COPPERFIN_TEST_ENVIRONMENT_SUPPORT_H

#include "copperfin/platform/environment.h"

#include <cstdlib>
#include <filesystem>
#include <optional>
#include <string>
#include <utility>

namespace copperfin::test_support {

inline bool is_filesystem_environment_variable(const std::string& name) {
    return name == "COPPERFIN_LOCALE_DIR";
}

inline std::string path_to_utf8_string(const std::filesystem::path& value) {
    const auto encoded = value.u8string();
    std::string decoded;
    decoded.reserve(encoded.size());
    for (const char8_t ch : encoded) {
        decoded.push_back(static_cast<char>(ch));
    }
    return decoded;
}

inline std::filesystem::path path_from_utf8_string(const std::string& value) {
    std::u8string encoded;
    encoded.reserve(value.size());
    for (const unsigned char ch : value) {
        encoded.push_back(static_cast<char8_t>(ch));
    }
    return std::filesystem::path(encoded);
}

inline std::optional<std::string> getenv_optional(const std::string& name) {
    if (is_filesystem_environment_variable(name)) {
        const auto value = copperfin::platform::read_environment_path(name);
        return value.has_value()
            ? std::optional<std::string>(path_to_utf8_string(*value))
            : std::nullopt;
    }
    return copperfin::platform::read_environment_variable(name);
}

inline std::string getenv_value(const std::string& name) {
    const auto value = getenv_optional(name);
    return value.has_value() ? *value : std::string();
}

inline std::optional<std::filesystem::path> getenv_path_optional(const std::string& name) {
    return copperfin::platform::read_environment_path(name);
}

inline std::filesystem::path getenv_path(const std::string& name) {
    const auto value = getenv_path_optional(name);
    return value.has_value() ? *value : std::filesystem::path();
}

inline void set_env_value(const std::string& name, const std::string& value, bool has_value) {
    if (is_filesystem_environment_variable(name)) {
        if (has_value) {
            (void)copperfin::platform::write_environment_path(
                name,
                path_from_utf8_string(value));
        } else {
            (void)copperfin::platform::clear_environment_path(name);
        }
        return;
    }
    if (has_value) {
        (void)copperfin::platform::write_environment_variable(name, value);
    } else {
        (void)copperfin::platform::clear_environment_variable(name);
    }
}

inline std::string prepare_shell_command_for_system(const std::string& command) {
#if defined(_WIN32)
    return "\"" + command + "\"";
#else
    return command;
#endif
}

inline int run_shell_command(const std::string& command) {
    const std::string prepared_command = prepare_shell_command_for_system(command);
    return std::system(prepared_command.c_str());
}

struct ScopedEnvironmentValue {
    std::string name;
    std::optional<std::string> original_value;

    explicit ScopedEnvironmentValue(std::string environment_name, bool clear_existing = true)
        : name(std::move(environment_name)),
          original_value(name.empty() ? std::nullopt : getenv_optional(name)) {
        if (clear_existing && !name.empty()) {
            clear();
        }
    }

    ScopedEnvironmentValue(std::string environment_name, std::string initial_value)
        : ScopedEnvironmentValue(std::move(environment_name), false) {
        set(initial_value);
    }

    ScopedEnvironmentValue(std::string environment_name, const char* initial_value)
        : ScopedEnvironmentValue(
              std::move(environment_name),
              initial_value == nullptr ? std::string() : std::string(initial_value)) {}

    void set(const std::string& value) const {
        if (name.empty()) {
            return;
        }
        set_env_value(name, value, true);
    }

    void clear() const {
        if (name.empty()) {
            return;
        }
        set_env_value(name, "", false);
    }

    ~ScopedEnvironmentValue() {
        if (original_value.has_value()) {
            set_env_value(name, *original_value, true);
        } else {
            clear();
        }
    }
};

struct ScopedEnvironmentPath {
    std::string name;
    std::optional<std::filesystem::path> original_value;

    explicit ScopedEnvironmentPath(std::string environment_name, bool clear_existing = true)
        : name(std::move(environment_name)),
          original_value(name.empty() ? std::nullopt : getenv_path_optional(name)) {
        if (clear_existing && !name.empty()) {
            clear();
        }
    }

    ScopedEnvironmentPath(std::string environment_name, const std::filesystem::path& initial_value)
        : ScopedEnvironmentPath(std::move(environment_name), false) {
        set(initial_value);
    }

    void set(const std::filesystem::path& value) const {
        if (!name.empty()) {
            (void)copperfin::platform::write_environment_path(name, value);
        }
    }

    void clear() const {
        if (!name.empty()) {
            (void)copperfin::platform::clear_environment_path(name);
        }
    }

    ~ScopedEnvironmentPath() {
        if (original_value.has_value()) {
            set(*original_value);
        } else {
            clear();
        }
    }
};

}  // namespace copperfin::test_support

#endif
