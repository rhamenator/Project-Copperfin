#pragma once

#include <string>

namespace copperfin::security {

struct SecretResolveResult {
    bool ok = false;
    std::string value;
    std::string error;
};

[[nodiscard]] SecretResolveResult resolve_secret_reference(const std::string& reference);

}  // namespace copperfin::security
