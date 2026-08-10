// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/polyglot_artifact_admission.h"

#include "copperfin/platform/polyglot_route_registry.h"
#include "copperfin/security/sha256.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <utility>

namespace copperfin::platform {

namespace {

bool valid_capability_id(std::string_view capability_id) {
    const auto validation = load_polyglot_route_registry({
        PolyglotRouteConfig{
            .capability_id = std::string(capability_id),
            .state = "off",
            .canary_percentage = 0U}});
    return validation.ok();
}

bool valid_sha256(std::string_view digest) noexcept {
    return digest.size() == 64U &&
        std::all_of(digest.begin(), digest.end(), [](char character) {
            return (character >= '0' && character <= '9') ||
                (character >= 'a' && character <= 'f');
        });
}

bool constant_time_equal(
    std::string_view left,
    std::string_view right) noexcept {
    if (left.size() != right.size()) {
        return false;
    }
    std::uint8_t difference = 0U;
    for (std::size_t index = 0U; index < left.size(); ++index) {
        difference |= static_cast<std::uint8_t>(left[index]) ^
            static_cast<std::uint8_t>(right[index]);
    }
    return difference == 0U;
}

bool same_file_identity(
    const security::ExternalProcessFileIdentity& left,
    const security::ExternalProcessFileIdentity& right) noexcept {
    return left.first == right.first && left.second == right.second;
}

}  // namespace

PolyglotArtifactAdmissionResult admit_polyglot_artifact(
    const PolyglotArtifactAdmissionRequest& request) {
    PolyglotArtifactAdmissionResult result;
    result.capability_id_ = request.capability_id;
    result.process_policy_ = request.process_policy;
    const auto deny = [&](
                          PolyglotArtifactAdmissionError error,
                          const char* error_code) {
        result.admitted_ = false;
        result.error_ = error;
        result.error_code_ = error_code;
        result.artifact_sha256_.clear();
        result.authorization_.allowed = false;
        return result;
    };

    if (!valid_capability_id(request.capability_id)) {
        return deny(
            PolyglotArtifactAdmissionError::invalid_capability_id,
            "polyglot.artifact.invalid_capability_id");
    }
    if (!valid_sha256(request.expected_sha256)) {
        return deny(
            PolyglotArtifactAdmissionError::invalid_expected_sha256,
            "polyglot.artifact.invalid_expected_sha256");
    }
    if (request.process_policy.allowed_path_roots.empty()) {
        return deny(
            PolyglotArtifactAdmissionError::allowed_root_required,
            "polyglot.artifact.allowed_root_required");
    }

    result.authorization_ =
        security::authorize_external_process(result.process_policy_);
    if (!result.authorization_.allowed) {
        return deny(
            PolyglotArtifactAdmissionError::authorization_denied,
            "polyglot.artifact.authorization_denied");
    }

    const auto digest =
        security::sha256_hex_for_file(result.authorization_.resolved_path);
    if (!digest.ok) {
        return deny(
            PolyglotArtifactAdmissionError::hash_failed,
            "polyglot.artifact.hash_failed");
    }
    if (!security::revalidate_external_process_authorization(
            result.authorization_)) {
        return deny(
            PolyglotArtifactAdmissionError::artifact_changed,
            "polyglot.artifact.changed_during_admission");
    }
    if (!constant_time_equal(digest.hex_digest, request.expected_sha256)) {
        return deny(
            PolyglotArtifactAdmissionError::hash_mismatch,
            "polyglot.artifact.sha256_mismatch");
    }

    result.admitted_ = true;
    result.error_ = PolyglotArtifactAdmissionError::none;
    result.error_code_ = "polyglot.artifact.admitted";
    result.artifact_sha256_ = digest.hex_digest;
    return result;
}

bool revalidate_polyglot_artifact_admission(
    PolyglotArtifactAdmissionResult& admission) {
    if (!admission.ok()) {
        return false;
    }
    const auto revoke = [&](
                            PolyglotArtifactAdmissionError error,
                            const char* error_code) {
        admission.admitted_ = false;
        admission.error_ = error;
        admission.error_code_ = error_code;
        admission.artifact_sha256_.clear();
        admission.authorization_.allowed = false;
        return false;
    };
    if (admission.authorization_.resolved_path.empty() ||
        !valid_capability_id(admission.capability_id_) ||
        !valid_sha256(admission.artifact_sha256_) ||
        admission.process_policy_.allowed_path_roots.empty()) {
        return revoke(
            PolyglotArtifactAdmissionError::artifact_changed,
            "polyglot.artifact.invalid_admission");
    }

    auto current_authorization =
        security::authorize_external_process(admission.process_policy_);
    if (!current_authorization.allowed) {
        return revoke(
            PolyglotArtifactAdmissionError::authorization_denied,
            "polyglot.artifact.policy_denied_before_execution");
    }
    if (!same_file_identity(
            current_authorization.file_identity,
            admission.authorization_.file_identity)) {
        return revoke(
            PolyglotArtifactAdmissionError::artifact_changed,
            "polyglot.artifact.changed_before_execution");
    }

    const auto digest = security::sha256_hex_for_file(
        current_authorization.resolved_path);
    if (!digest.ok) {
        return revoke(
            PolyglotArtifactAdmissionError::hash_failed,
            "polyglot.artifact.revalidation_hash_failed");
    }
    if (!constant_time_equal(digest.hex_digest, admission.artifact_sha256_)) {
        return revoke(
            PolyglotArtifactAdmissionError::artifact_changed,
            "polyglot.artifact.contents_changed_before_execution");
    }
    if (!security::revalidate_external_process_authorization(
            current_authorization)) {
        return revoke(
            PolyglotArtifactAdmissionError::artifact_changed,
            "polyglot.artifact.changed_during_revalidation");
    }
    admission.authorization_ = std::move(current_authorization);
    return true;
}

}  // namespace copperfin::platform
