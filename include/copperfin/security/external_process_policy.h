// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#pragma once

#include <string>
#include <vector>

namespace copperfin::security {

struct ExternalProcessPolicy {
    std::string executable_name;
    std::vector<std::string> allowed_path_roots;
    std::vector<std::string> allowed_publishers;
    bool require_trusted_signature = true;
};

struct ExternalProcessAuthorizationResult {
    bool allowed = false;
    std::string resolved_path;
    std::string error;
};

[[nodiscard]] ExternalProcessAuthorizationResult authorize_external_process(
    const ExternalProcessPolicy& policy);

}  // namespace copperfin::security
