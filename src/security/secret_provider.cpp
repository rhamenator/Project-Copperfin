#include "copperfin/security/secret_provider.h"

#include <cstdlib>

namespace copperfin::security {

SecretResolveResult resolve_secret_reference(const std::string& reference) {
    constexpr const char* kEnvPrefix = "env:";
    if (reference.rfind(kEnvPrefix, 0) != 0) {
        return {.ok = false, .error = "Secret reference must use env:<NAME> format."};
    }

    const std::string variable_name = reference.substr(4U);
    if (variable_name.empty()) {
        return {.ok = false, .error = "Secret reference variable name is empty."};
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
