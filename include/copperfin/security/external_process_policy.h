// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace copperfin::security {

struct ExternalProcessPolicy {
    std::string executable_name;
    std::vector<std::string> allowed_path_roots;
    std::vector<std::string> allowed_publishers;
    bool require_trusted_signature = true;
};

struct ExternalProcessFileIdentity {
    std::uint64_t first = 0;
    std::uint64_t second = 0;
};

struct ExternalProcessAuthorizationResult {
    bool allowed = false;
    std::string resolved_path;
    std::string error;
    ExternalProcessFileIdentity file_identity;
};

[[nodiscard]] ExternalProcessAuthorizationResult authorize_external_process(
    const ExternalProcessPolicy& policy);

// Recheck the physical executable selected by authorization immediately before
// launch. The result is marked denied when the selected file was replaced.
[[nodiscard]] bool revalidate_external_process_authorization(
    ExternalProcessAuthorizationResult& authorization);

}  // namespace copperfin::security
