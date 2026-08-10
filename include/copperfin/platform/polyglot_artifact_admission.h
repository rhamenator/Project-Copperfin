// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include "copperfin/security/external_process_policy.h"

#include <string>

namespace copperfin::platform {

enum class PolyglotArtifactAdmissionError {
    none,
    invalid_capability_id,
    invalid_expected_sha256,
    allowed_root_required,
    authorization_denied,
    hash_failed,
    hash_mismatch,
    artifact_changed
};

struct PolyglotArtifactAdmissionRequest {
    std::string capability_id;
    security::ExternalProcessPolicy process_policy;
    std::string expected_sha256;
};

class PolyglotArtifactAdmissionResult;

[[nodiscard]] PolyglotArtifactAdmissionResult admit_polyglot_artifact(
    const PolyglotArtifactAdmissionRequest& request);
[[nodiscard]] bool revalidate_polyglot_artifact_admission(
    PolyglotArtifactAdmissionResult& admission);

class PolyglotArtifactAdmissionResult {
public:
    [[nodiscard]] bool ok() const noexcept {
        return admitted_ && error_ == PolyglotArtifactAdmissionError::none;
    }

    [[nodiscard]] PolyglotArtifactAdmissionError error() const noexcept {
        return error_;
    }

    [[nodiscard]] const std::string& error_code() const noexcept {
        return error_code_;
    }

    [[nodiscard]] const std::string& capability_id() const noexcept {
        return capability_id_;
    }

    [[nodiscard]] const std::string& artifact_sha256() const noexcept {
        return artifact_sha256_;
    }

    [[nodiscard]] const security::ExternalProcessAuthorizationResult& authorization()
        const noexcept {
        return authorization_;
    }

private:
    bool admitted_ = false;
    PolyglotArtifactAdmissionError error_ = PolyglotArtifactAdmissionError::none;
    std::string error_code_;
    std::string capability_id_;
    std::string artifact_sha256_;
    security::ExternalProcessPolicy process_policy_;
    security::ExternalProcessAuthorizationResult authorization_;

    PolyglotArtifactAdmissionResult() = default;

    friend PolyglotArtifactAdmissionResult admit_polyglot_artifact(
        const PolyglotArtifactAdmissionRequest& request);
    friend bool revalidate_polyglot_artifact_admission(
        PolyglotArtifactAdmissionResult& admission);
};

// Authorize and hash one external executable without launching it. The caller
// must revalidate the returned admission immediately before execution.
// Revalidation repeats the original process policy, physical identity, and
// exact-byte digest checks. A failure revokes the token in place.

}  // namespace copperfin::platform
