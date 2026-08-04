// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/security/secret_provider.h"

#include "copperfin/platform/environment.h"
#include "localized_text.h"

#include <algorithm>
#include <cctype>

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

    const std::optional<std::string> value = platform::read_environment_variable(variable_name);
    if (!value.has_value()) {
        return {.ok = false,
                .value = {},
                .error = security_text("Security.Secret.Error.EnvironmentVariableNotFound", {{"variableName", variable_name}})};
    }

    if (value->empty()) {
        return {.ok = false,
                .value = {},
                .error = security_text("Security.Secret.Error.EnvironmentVariableEmpty", {{"variableName", variable_name}})};
    }

    return {.ok = true, .value = *value, .error = {}};
}

}  // namespace copperfin::security
