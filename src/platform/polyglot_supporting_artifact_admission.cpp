// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/polyglot_supporting_artifact_admission.h"

#include "copperfin/platform/path.h"
#include "copperfin/platform/polyglot_route_registry.h"
#include "copperfin/security/sha256.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string_view>
#include <utility>

namespace copperfin::platform {

namespace {

constexpr std::uint64_t absolute_maximum_supporting_artifact_bytes =
    16U * 1024U * 1024U;

bool valid_capability_id(const std::string_view capability_id) {
    return load_polyglot_route_registry({PolyglotRouteConfig{
        .capability_id = std::string(capability_id),
        .state = "off",
        .canary_percentage = 0U}}).ok();
}

bool valid_sha256(const std::string_view digest) noexcept {
    return digest.size() == 64U &&
        std::all_of(digest.begin(), digest.end(), [](const char character) {
            return (character >= '0' && character <= '9') ||
                (character >= 'a' && character <= 'f');
        });
}

bool constant_time_equal(
    const std::string_view left,
    const std::string_view right) noexcept {
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

}  // namespace

PolyglotSupportingArtifactAdmissionResult admit_polyglot_supporting_artifact(
    const PolyglotSupportingArtifactAdmissionRequest& request) {
    PolyglotSupportingArtifactAdmissionResult result;
    result.capability_id_ = request.capability_id;
    result.allowed_root_ = request.allowed_root;
    result.maximum_bytes_ = request.maximum_bytes;
    const auto deny = [&result](
                          const PolyglotSupportingArtifactAdmissionError error,
                          const char* const error_code) {
        result.admitted_ = false;
        result.error_ = error;
        result.error_code_ = error_code;
        result.resolved_path_.clear();
        result.artifact_sha256_.clear();
        result.containment_ = {};
        return result;
    };

    if (!valid_capability_id(request.capability_id)) {
        return deny(
            PolyglotSupportingArtifactAdmissionError::invalid_capability_id,
            "polyglot.supporting_artifact.invalid_capability_id");
    }
    if (!valid_sha256(request.expected_sha256)) {
        return deny(
            PolyglotSupportingArtifactAdmissionError::invalid_expected_sha256,
            "polyglot.supporting_artifact.invalid_expected_sha256");
    }
    if (request.maximum_bytes == 0U ||
        request.maximum_bytes > absolute_maximum_supporting_artifact_bytes) {
        return deny(
            PolyglotSupportingArtifactAdmissionError::invalid_maximum_bytes,
            "polyglot.supporting_artifact.invalid_maximum_bytes");
    }
    if (request.artifact_path.empty()) {
        return deny(
            PolyglotSupportingArtifactAdmissionError::artifact_path_required,
            "polyglot.supporting_artifact.path_required");
    }
    if (request.allowed_root.empty()) {
        return deny(
            PolyglotSupportingArtifactAdmissionError::allowed_root_required,
            "polyglot.supporting_artifact.allowed_root_required");
    }

    result.containment_ = security::inspect_physical_path_containment(
        path_from_utf8_string(request.artifact_path),
        path_from_utf8_string(request.allowed_root));
    if (!result.containment_.allowed) {
        return deny(
            PolyglotSupportingArtifactAdmissionError::containment_denied,
            "polyglot.supporting_artifact.containment_denied");
    }
    if (result.containment_.identity.file_size > result.maximum_bytes_) {
        return deny(
            PolyglotSupportingArtifactAdmissionError::artifact_too_large,
            "polyglot.supporting_artifact.size_limit_exceeded");
    }
    const auto snapshot = security::read_physically_contained_file_snapshot(
        result.containment_,
        path_from_utf8_string(result.allowed_root_),
        result.maximum_bytes_);
    if (!snapshot.ok) {
        return deny(
            PolyglotSupportingArtifactAdmissionError::read_failed,
            "polyglot.supporting_artifact.read_failed");
    }
    const auto digest = security::sha256_hex_for_text(snapshot.bytes);
    if (!digest.ok) {
        return deny(
            PolyglotSupportingArtifactAdmissionError::read_failed,
            "polyglot.supporting_artifact.hash_failed");
    }
    if (!constant_time_equal(digest.hex_digest, request.expected_sha256)) {
        return deny(
            PolyglotSupportingArtifactAdmissionError::hash_mismatch,
            "polyglot.supporting_artifact.sha256_mismatch");
    }

    result.containment_ = snapshot.containment;
    result.resolved_path_ = path_to_utf8_string(
        result.containment_.canonical_path);
    result.artifact_sha256_ = digest.hex_digest;
    result.admitted_ = true;
    result.error_ = PolyglotSupportingArtifactAdmissionError::none;
    result.error_code_ = "polyglot.supporting_artifact.admitted";
    return result;
}

bool revalidate_polyglot_supporting_artifact_admission(
    PolyglotSupportingArtifactAdmissionResult& admission) {
    if (!admission.ok()) {
        return false;
    }
    const auto revoke = [&admission](
                            const PolyglotSupportingArtifactAdmissionError error,
                            const char* const error_code) {
        admission.admitted_ = false;
        admission.error_ = error;
        admission.error_code_ = error_code;
        admission.resolved_path_.clear();
        admission.artifact_sha256_.clear();
        admission.containment_.allowed = false;
        return false;
    };
    if (!valid_capability_id(admission.capability_id_) ||
        admission.allowed_root_.empty() || admission.resolved_path_.empty() ||
        !valid_sha256(admission.artifact_sha256_) ||
        admission.maximum_bytes_ == 0U ||
        admission.maximum_bytes_ > absolute_maximum_supporting_artifact_bytes ||
        admission.containment_.identity.file_size > admission.maximum_bytes_) {
        return revoke(
            PolyglotSupportingArtifactAdmissionError::artifact_changed,
            "polyglot.supporting_artifact.invalid_admission");
    }

    const auto snapshot = security::read_physically_contained_file_snapshot(
        admission.containment_,
        path_from_utf8_string(admission.allowed_root_),
        admission.maximum_bytes_);
    if (!snapshot.ok) {
        return revoke(
            PolyglotSupportingArtifactAdmissionError::artifact_changed,
            "polyglot.supporting_artifact.changed_before_execution");
    }
    const auto digest = security::sha256_hex_for_text(snapshot.bytes);
    if (!digest.ok ||
        !constant_time_equal(digest.hex_digest, admission.artifact_sha256_)) {
        return revoke(
            PolyglotSupportingArtifactAdmissionError::artifact_changed,
            "polyglot.supporting_artifact.contents_changed_before_execution");
    }
    admission.containment_ = snapshot.containment;
    return true;
}

}  // namespace copperfin::platform
