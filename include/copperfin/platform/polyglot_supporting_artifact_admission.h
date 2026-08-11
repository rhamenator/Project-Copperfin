// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include "copperfin/security/physical_path_containment.h"

#include <cstdint>
#include <string>

namespace copperfin::platform {

enum class PolyglotSupportingArtifactAdmissionError {
    none,
    invalid_capability_id,
    invalid_expected_sha256,
    invalid_maximum_bytes,
    artifact_path_required,
    allowed_root_required,
    containment_denied,
    artifact_too_large,
    read_failed,
    hash_mismatch,
    artifact_changed
};

struct PolyglotSupportingArtifactAdmissionRequest {
    std::string capability_id;
    std::string artifact_path;
    std::string allowed_root;
    std::string expected_sha256;
    std::uint64_t maximum_bytes = 1024U * 1024U;
};

class PolyglotSupportingArtifactAdmissionResult;

[[nodiscard]] PolyglotSupportingArtifactAdmissionResult
admit_polyglot_supporting_artifact(
    const PolyglotSupportingArtifactAdmissionRequest& request);
[[nodiscard]] bool revalidate_polyglot_supporting_artifact_admission(
    PolyglotSupportingArtifactAdmissionResult& admission);

class PolyglotSupportingArtifactAdmissionResult {
public:
    [[nodiscard]] bool ok() const noexcept {
        return admitted_ &&
            error_ == PolyglotSupportingArtifactAdmissionError::none;
    }

    [[nodiscard]] PolyglotSupportingArtifactAdmissionError error()
        const noexcept {
        return error_;
    }

    [[nodiscard]] const std::string& error_code() const noexcept {
        return error_code_;
    }

    [[nodiscard]] const std::string& capability_id() const noexcept {
        return capability_id_;
    }

    [[nodiscard]] const std::string& resolved_path() const noexcept {
        return resolved_path_;
    }

    [[nodiscard]] const std::string& artifact_sha256() const noexcept {
        return artifact_sha256_;
    }

private:
    bool admitted_ = false;
    PolyglotSupportingArtifactAdmissionError error_ =
        PolyglotSupportingArtifactAdmissionError::none;
    std::string error_code_;
    std::string capability_id_;
    std::string allowed_root_;
    std::string resolved_path_;
    std::string artifact_sha256_;
    std::uint64_t maximum_bytes_ = 0U;
    security::PhysicalPathContainmentResult containment_;

    PolyglotSupportingArtifactAdmissionResult() = default;

    friend PolyglotSupportingArtifactAdmissionResult
    admit_polyglot_supporting_artifact(
        const PolyglotSupportingArtifactAdmissionRequest& request);
    friend bool revalidate_polyglot_supporting_artifact_admission(
        PolyglotSupportingArtifactAdmissionResult& admission);
};

// Admits a non-executable file such as a Python sidecar script beneath one
// explicit physical root. Admission and pre-launch revalidation bind the
// same physical file identity and exact lowercase SHA-256 digest.

}  // namespace copperfin::platform
