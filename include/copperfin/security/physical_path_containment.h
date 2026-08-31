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
    read_failed
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

    bool operator==(const PhysicalPathIdentity& other) const noexcept {
        return storage_id == other.storage_id && file_id == other.file_id &&
            file_size == other.file_size &&
            modified_ticks == other.modified_ticks &&
            link_count == other.link_count;
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

}  // namespace copperfin::security
