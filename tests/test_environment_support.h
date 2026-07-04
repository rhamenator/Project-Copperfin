// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#ifndef COPPERFIN_TEST_ENVIRONMENT_SUPPORT_H
#define COPPERFIN_TEST_ENVIRONMENT_SUPPORT_H

#include <cstdlib>
#include <optional>
#include <string>
#include <utility>

namespace copperfin::test_support {

inline std::optional<std::string> getenv_optional(const std::string& name) {
#if defined(_WIN32)
    char* raw = nullptr;
    std::size_t length = 0;
    if (_dupenv_s(&raw, &length, name.c_str()) != 0 || raw == nullptr) {
        return std::nullopt;
    }
    std::string value(raw);
    std::free(raw);
    return value;
#else
    const char* raw = std::getenv(name.c_str());
    if (raw == nullptr) {
        return std::nullopt;
    }
    return std::string(raw);
#endif
}

inline std::string getenv_value(const std::string& name) {
    const auto value = getenv_optional(name);
    return value.has_value() ? *value : std::string();
}

inline void set_env_value(const std::string& name, const std::string& value, bool has_value) {
#if defined(_WIN32)
    if (has_value) {
        _putenv_s(name.c_str(), value.c_str());
    } else {
        _putenv_s(name.c_str(), "");
    }
#else
    if (has_value) {
        setenv(name.c_str(), value.c_str(), 1);
    } else {
        unsetenv(name.c_str());
    }
#endif
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

}  // namespace copperfin::test_support

#endif
