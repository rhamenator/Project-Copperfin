// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#pragma once

#include <cstdlib>
#include <optional>
#include <string>
#include <string_view>

namespace copperfin::platform {

inline std::optional<std::string> read_environment_variable(std::string_view name) {
    if (name.empty()) {
        return std::nullopt;
    }

    const std::string key(name);
#if defined(_WIN32)
    char* raw = nullptr;
    std::size_t length = 0;
    if (_dupenv_s(&raw, &length, key.c_str()) != 0 || raw == nullptr) {
        return std::nullopt;
    }
    std::string value(raw);
    std::free(raw);
    return value;
#else
    if (const char* raw = std::getenv(key.c_str()); raw != nullptr) {
        return std::string(raw);
    }
    return std::nullopt;
#endif
}

inline std::string read_environment_variable_or_empty(std::string_view name) {
    const auto value = read_environment_variable(name);
    return value.has_value() ? *value : std::string{};
}

}  // namespace copperfin::platform
