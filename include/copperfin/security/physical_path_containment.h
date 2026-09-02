// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>

namespace copperfin::security {

enum class PhysicalPathContainmentFailure {
    none,
    root_unavailable,
    path_unavailable,
    outside_root,
    indirect_component,
    cross_device_component,
    identity_changed,
    not_regular_file,
    size_limit_exceeded,
    read_failed,
    // Distinct from identity_changed: identity_changed can come from a read
    // itself detecting a mid-read content change (see
    // read_physically_contained_file_snapshot[_from_handle]()'s own before/
    // after identity checks), while this value comes only from
    // revalidate_physical_path_containment_after_read()'s independent
    // post-read path re-walk finding a different object -- a caller
    // classifying "was this specifically a rename/replace observed after
    // the read completed" must not conflate the two (issue #5434 review
    // finding).
    path_changed_after_read
};

struct PhysicalPathIdentity {
    std::uint64_t storage_id = 0U;
    std::uint64_t file_id = 0U;
    std::uint64_t file_size = 0U;
    std::uint64_t modified_ticks = 0U;
    std::uint64_t link_count = 0U;
    // Stable object-creation time when the platform/filesystem supplies it.
    // Directory authority that must survive legitimate child mutations may
    // require this field in addition to storage/file identity.
    std::uint64_t creation_ticks = 0U;

    // Excludes link_count: meaningful for a caller re-resolving a path by
    // string (a link-count change can indicate a different object was
    // resolved), but not a content-mutation signal for a caller already
    // holding a live handle/descriptor to the exact object -- unlinking a
    // path a process holds open drops that object's own link_count without
    // changing any of its content. See operator== below and
    // read_physically_contained_file_snapshot_from_handle()'s use of this
    // method, which is why the two deliberately differ by exactly this one
    // field rather than being independent comparisons that could drift.
    [[nodiscard]] bool content_equal(const PhysicalPathIdentity& other) const noexcept {
        return storage_id == other.storage_id && file_id == other.file_id &&
            file_size == other.file_size &&
            modified_ticks == other.modified_ticks;
    }

    bool operator==(const PhysicalPathIdentity& other) const noexcept {
        return content_equal(other) && link_count == other.link_count;
    }
};

struct PhysicalPathContainmentResult {
    bool allowed = false;
    std::filesystem::path canonical_path;
    PhysicalPathIdentity identity;
    PhysicalPathContainmentFailure failure = PhysicalPathContainmentFailure::path_unavailable;
};

struct PhysicalFileSnapshotResult {
    bool ok = false;
    std::string bytes;
    PhysicalPathContainmentResult containment;
    PhysicalPathContainmentFailure failure = PhysicalPathContainmentFailure::read_failed;
};

[[nodiscard]] PhysicalPathContainmentResult inspect_physical_path_containment(
    const std::filesystem::path& path,
    const std::filesystem::path& root);

[[nodiscard]] PhysicalFileSnapshotResult read_physically_contained_file_snapshot(
    const PhysicalPathContainmentResult& expected,
    const std::filesystem::path& root);

[[nodiscard]] PhysicalFileSnapshotResult read_physically_contained_file_snapshot(
    const PhysicalPathContainmentResult& expected,
    const std::filesystem::path& root,
    std::uint64_t maximum_bytes);

// Move-only handle produced by inspect_and_open_physically_contained_path().
// Carries the same fields as PhysicalPathContainmentResult (via result())
// plus a live, platform-native handle/descriptor to the exact verified
// object, so read_physically_contained_file_snapshot_from_handle() can read
// from it directly instead of reopening PhysicalPathContainmentResult's
// canonical_path by string (see issue #5409). The native handle type itself
// is not named here to keep this public header platform-agnostic -- this
// codebase's public headers must not leak platform-selection tokens (see
// src/platform/scoped_resource.h and PR #5418's fix for that same trap);
// the handle is usable only through the two functions declared alongside
// this class.
class PhysicalPathContainmentHandle {
public:
    PhysicalPathContainmentHandle() noexcept;
    ~PhysicalPathContainmentHandle();
    PhysicalPathContainmentHandle(const PhysicalPathContainmentHandle&) = delete;
    PhysicalPathContainmentHandle& operator=(const PhysicalPathContainmentHandle&) = delete;
    PhysicalPathContainmentHandle(PhysicalPathContainmentHandle&&) noexcept;
    PhysicalPathContainmentHandle& operator=(PhysicalPathContainmentHandle&&) noexcept;

    [[nodiscard]] const PhysicalPathContainmentResult& result() const noexcept;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;

    friend PhysicalPathContainmentHandle inspect_and_open_physically_contained_path(
        const std::filesystem::path& path,
        const std::filesystem::path& root);
    friend PhysicalFileSnapshotResult read_physically_contained_file_snapshot_from_handle(
        const PhysicalPathContainmentHandle& handle,
        std::uint64_t maximum_bytes);

    explicit PhysicalPathContainmentHandle(std::unique_ptr<Impl> impl) noexcept;
};

// Performs the identical verified walk as inspect_physical_path_containment()
// but keeps the native handle/descriptor open on success instead of closing
// it, so a caller that intends to read immediately after checking can do so
// via read_physically_contained_file_snapshot_from_handle() without any
// intervening re-resolution by path string. Prefer
// inspect_physical_path_containment() for check-only callers -- this keeps a
// native OS resource open until the returned handle is destroyed.
[[nodiscard]] PhysicalPathContainmentHandle inspect_and_open_physically_contained_path(
    const std::filesystem::path& path,
    const std::filesystem::path& root);

// Reads the file bytes directly from handle's live native handle/descriptor
// (from inspect_and_open_physically_contained_path()), never reopening by
// path string. Fails with path_unavailable if handle does not carry an
// allowed, open result. Re-checks identity both before and after the read
// (catching truncation/append during the read itself) but performs no
// further independent path re-walk, since the read is structurally bound to
// the exact object the walk verified for the whole call -- unlike the
// string-reopening read_physically_contained_file_snapshot() above, there is
// no window in which the object being read could differ from the object the
// check verified.
[[nodiscard]] PhysicalFileSnapshotResult read_physically_contained_file_snapshot_from_handle(
    const PhysicalPathContainmentHandle& handle,
    std::uint64_t maximum_bytes);

[[nodiscard]] PhysicalFileSnapshotResult read_physically_contained_file_snapshot_from_handle(
    const PhysicalPathContainmentHandle& handle);

// Performs an independent post-read re-walk of root to confirm
// snapshot.containment.canonical_path (from a prior successful read --
// snapshot.ok must be true) still resolves to the same object. A caller
// whose result is stored or compared *by name* later (rather than reused
// immediately via the same still-open handle that produced the read) needs
// this extra guarantee, or a rename/replace during the read can leave it
// holding a digest/identity for an object the path no longer actually
// names. Fails with path_changed_after_read (never identity_changed, and
// never any failure the read itself could produce) when the re-walk finds
// a different object, so this specific outcome is identifiable distinctly
// from an ordinary read failure (issue #5434, consolidating the identical
// hand-rolled re-walk pair from issues #5426 and #5427). Returns snapshot
// unchanged on success.
//
// post_read_hook, if non-null, runs once before the re-walk -- the same
// injection point issues #5426/#5427's own hand-rolled versions used for
// their single-shot test hooks (see e.g. runtime_pipeline_test_hooks.h),
// preserved here so callers keep that capability without this shared
// primitive depending on any caller's own test-hook machinery or build
// flag.
//
// Call this directly, after your own caller-specific checks on snapshot's
// bytes (e.g. a hash comparison), if their relative order matters to your
// diagnostics -- see admit_polyglot_supporting_artifact()'s use of this
// split form to preserve its pre-existing hash-mismatch-before-rename
// precedence. Otherwise, prefer the combined convenience function below.
[[nodiscard]] PhysicalFileSnapshotResult
revalidate_physical_path_containment_after_read(
    const PhysicalFileSnapshotResult& snapshot,
    const std::filesystem::path& root,
    void (*post_read_hook)() = nullptr);

// Reads via handle (as read_physically_contained_file_snapshot_from_handle()
// above), then revalidate_physical_path_containment_after_read() in one
// call, for a caller with no caller-specific check that needs to run
// between the two (see that function's own doc comment for callers that
// do, and for what path_changed_after_read means).
//
// Prefer plain read_physically_contained_file_snapshot_from_handle() for
// callers that revalidate independently at use time instead (e.g. an
// #5421-style migration that re-verifies immediately before every use) and
// don't need the extra walk at all.
[[nodiscard]] PhysicalFileSnapshotResult
read_physically_contained_file_snapshot_from_handle_and_revalidate_path(
    const PhysicalPathContainmentHandle& handle,
    const std::filesystem::path& root,
    std::uint64_t maximum_bytes,
    void (*post_read_hook)() = nullptr);

[[nodiscard]] PhysicalFileSnapshotResult
read_physically_contained_file_snapshot_from_handle_and_revalidate_path(
    const PhysicalPathContainmentHandle& handle,
    const std::filesystem::path& root,
    void (*post_read_hook)() = nullptr);

}  // namespace copperfin::security
