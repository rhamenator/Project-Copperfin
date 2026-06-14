#include "copperfin/security/secret_provider.h"

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
        return {.ok = false, .error = "Secret reference must use env:<NAME> format."};
    }

    const std::string variable_name = reference.substr(4U);
    if (variable_name.empty()) {
        return {.ok = false, .error = "Secret reference variable name is empty."};
    }
    if (!is_valid_secret_variable_name(variable_name)) {
        return {.ok = false, .error = "Secret reference variable name contains invalid characters."};
    }

    std::string value;
#ifdef _WIN32
    char* raw = nullptr;
    std::size_t length = 0;
    if (_dupenv_s(&raw, &length, variable_name.c_str()) != 0 || raw == nullptr) {
        return {.ok = false, .error = "Secret environment variable was not found: " + variable_name};
    }
    value = raw;
    std::free(raw);
#else
    const char* raw = std::getenv(variable_name.c_str());
    if (raw == nullptr) {
        return {.ok = false, .error = "Secret environment variable was not found: " + variable_name};
    }
    value = raw;
#endif

    if (value.empty()) {
        return {.ok = false, .error = "Secret environment variable is empty: " + variable_name};
    }

    return {.ok = true, .value = value};
}

}  // namespace copperfin::security
