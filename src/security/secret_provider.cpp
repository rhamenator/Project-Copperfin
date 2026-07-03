// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "copperfin/security/secret_provider.h"

#include "localized_text.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>

namespace copperfin::security {

namespace {

bool is_valid_secret_variable_name(const std::string& value) {
    if (value.empty()) {
        return false;
    }

    return std::none_of(value.begin(), value.end(), [](unsigned char ch) {
        return std::isspace(ch) != 0 || std::iscntrl(ch) != 0 || ch == '=';
    });
}

}  // namespace

SecretResolveResult resolve_secret_reference(const std::string& reference) {
    constexpr const char* kEnvPrefix = "env:";
    if (reference.rfind(kEnvPrefix, 0) != 0) {
        return {.ok = false, .value = {}, .error = security_text("Security.Secret.Error.InvalidReferenceFormat")};
    }

    const std::string variable_name = reference.substr(4U);
    if (variable_name.empty()) {
        return {.ok = false, .value = {}, .error = security_text("Security.Secret.Error.VariableNameEmpty")};
    }
    if (!is_valid_secret_variable_name(variable_name)) {
        return {.ok = false, .value = {}, .error = security_text("Security.Secret.Error.VariableNameInvalid")};
    }

    std::string value;
#ifdef _WIN32
    char* raw = nullptr;
    std::size_t length = 0;
    if (_dupenv_s(&raw, &length, variable_name.c_str()) != 0 || raw == nullptr) {
        return {.ok = false,
                .value = {},
                .error = security_text("Security.Secret.Error.EnvironmentVariableNotFound", {{"variableName", variable_name}})};
    }
    value = raw;
    std::free(raw);
#else
    const char* raw = std::getenv(variable_name.c_str());
    if (raw == nullptr) {
        return {.ok = false,
                .value = {},
                .error = security_text("Security.Secret.Error.EnvironmentVariableNotFound", {{"variableName", variable_name}})};
    }
    value = raw;
#endif

    if (value.empty()) {
        return {.ok = false,
                .value = {},
                .error = security_text("Security.Secret.Error.EnvironmentVariableEmpty", {{"variableName", variable_name}})};
    }

    return {.ok = true, .value = value, .error = {}};
}

}  // namespace copperfin::security
