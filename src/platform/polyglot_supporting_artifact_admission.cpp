// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/polyglot_supporting_artifact_admission.h"

#include "copperfin/platform/path.h"
#include "copperfin/platform/polyglot_route_registry.h"
#include "copperfin/security/sha256.h"

#if defined(COPPERFIN_ENABLE_POLYGLOT_SUPPORTING_ARTIFACT_ADMISSION_TEST_HOOKS)
#include "copperfin/platform/polyglot_supporting_artifact_admission_test_hooks.h"
#endif

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string_view>
#include <utility>

namespace copperfin::platform {

namespace {

#if defined(COPPERFIN_ENABLE_POLYGLOT_SUPPORTING_ARTIFACT_ADMISSION_TEST_HOOKS)
// See polyglot_supporting_artifact_admission_test_hooks.h. Relaxed ordering
// is sufficient: this exists only for single-threaded test setup/teardown
// around a call to admit_polyglot_supporting_artifact(), never for
// production synchronization.
std::atomic<void (*)()> post_read_test_hook{nullptr};
#endif

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

#if defined(COPPERFIN_ENABLE_POLYGLOT_SUPPORTING_ARTIFACT_ADMISSION_TEST_HOOKS)
void set_polyglot_supporting_artifact_admission_post_read_test_hook_for_testing(
    void (*hook)()) {
    post_read_test_hook.store(hook, std::memory_order_relaxed);
}
#endif

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

    // Atomic check-and-open primitive (issue #5409/#5420/#5427): the read
    // below is bound to the exact object this walk verified, never reopened
    // by path string. The inline expected_sha256 comparison protects the
    // bytes read via the handle, but does not by itself guarantee
    // resolved_path_ still resolves to that same object once this function
    // returns: a rename/replace during the read lets the handle-bound read
    // still correctly match expected_sha256 (it's reading the original,
    // unchanged object), while resolved_path_ now points at a different
    // one. This function's own returned admission (resolved_path_,
    // artifact_sha256_, containment_) must be self-consistent regardless of
    // whether a given caller later revalidates before use, so an
    // independent post-read path re-walk is required here -- via
    // revalidate_physical_path_containment_after_read(), called explicitly
    // below (not the combined _and_revalidate_path() convenience function)
    // so the hash comparison keeps running first, preserving this
    // function's pre-existing hash-mismatch-before-rename diagnostic
    // precedence (issue #5434, consolidating what used to be a hand-rolled
    // re-walk here and in runtime_pipeline_launcher_artifact_inventory.cpp;
    // originally found by adversarial review on this PR).
    auto handle = security::inspect_and_open_physically_contained_path(
        path_from_utf8_string(request.artifact_path),
        path_from_utf8_string(request.allowed_root));
    result.containment_ = handle.result();
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
    const auto snapshot = security::read_physically_contained_file_snapshot_from_handle(
        handle, result.maximum_bytes_);
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
    void (*post_read_hook)() = nullptr;
#if defined(COPPERFIN_ENABLE_POLYGLOT_SUPPORTING_ARTIFACT_ADMISSION_TEST_HOOKS)
    post_read_hook = post_read_test_hook.exchange(nullptr, std::memory_order_relaxed);
#endif
    const auto revalidated = security::revalidate_physical_path_containment_after_read(
        snapshot, path_from_utf8_string(request.allowed_root), post_read_hook);
    if (!revalidated.ok) {
        // Denied with artifact_changed, not containment_denied: a rename/
        // replace caught here is a materially different security event
        // from "the path was never allowed" (containment_denied's meaning
        // at the top of this function) and reusing that code would mask a
        // genuine TOCTOU-attack signal from security audit logs. Mirrors
        // the established polyglot.artifact.changed_during_admission
        // convention in the sibling polyglot_artifact_admission.cpp, and
        // reuses the same PolyglotSupportingArtifactAdmissionError::artifact_changed
        // value this file's own revalidate_polyglot_supporting_artifact_admission()
        // already uses for its analogous checks -- found by a second
        // adversarial review pass on PR #5428/issue #5427.
        return deny(
            PolyglotSupportingArtifactAdmissionError::artifact_changed,
            "polyglot.supporting_artifact.changed_during_admission");
    }

    result.containment_ = revalidated.containment;
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

    // Atomic check-and-open primitive (issue #5409): the read below is
    // bound to the exact object this fresh walk verifies, never reopened by
    // the stored resolved_path_ string the way the prior implementation
    // reused admission.containment_ directly. Compared against
    // admission.containment_.identity (the admission-time identity), not
    // any identity read_physically_contained_file_snapshot_from_handle()
    // checks internally -- that internal check is a read-time freshness
    // check against this fresh walk's own open-time identity, not against
    // the admission-time identity this function exists to revalidate.
    auto handle = security::inspect_and_open_physically_contained_path(
        path_from_utf8_string(admission.resolved_path_),
        path_from_utf8_string(admission.allowed_root_));
    const auto& current = handle.result();
    if (!current.allowed ||
        current.canonical_path != path_from_utf8_string(admission.resolved_path_) ||
        current.identity != admission.containment_.identity) {
        return revoke(
            PolyglotSupportingArtifactAdmissionError::artifact_changed,
            "polyglot.supporting_artifact.changed_before_execution");
    }
    // _and_revalidate_path, not the plain handle read: this function's
    // whole purpose is to hand its caller a resolved_path()/artifact_sha256()
    // pair it can trust immediately afterward, so it needs its own
    // final-containment guarantee -- that resolved_path_ still names the
    // read object -- rather than depending on the caller to repeat that
    // check (same defense-in-depth rationale as #5459's
    // snapshot_workspace_file() migration). A path-changed-after-read
    // failure falls into the same !snapshot.ok branch below, which already
    // reports the accurate "changed_before_execution" diagnostic for it.
    void (*post_read_hook)() = nullptr;
#if defined(COPPERFIN_ENABLE_POLYGLOT_SUPPORTING_ARTIFACT_ADMISSION_TEST_HOOKS)
    post_read_hook = post_read_test_hook.exchange(nullptr, std::memory_order_relaxed);
#endif
    const auto snapshot =
        security::read_physically_contained_file_snapshot_from_handle_and_revalidate_path(
            handle,
            path_from_utf8_string(admission.allowed_root_),
            admission.maximum_bytes_,
            post_read_hook);
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
