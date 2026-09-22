// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include <filesystem>
#include <memory>
#include <optional>
#include <string_view>
#include <vector>

namespace copperfin::vfp {

// Retains the stage directory's path and, on Windows, no-delete-share handles
// for its directory chain so another account cannot replace any component
// while the import writes through pathnames. Release those handles immediately
// before the materializer cleans up its own staged files and directory.
class PrivateImportStagingDirectory {
public:
    PrivateImportStagingDirectory() noexcept;
    ~PrivateImportStagingDirectory();
    PrivateImportStagingDirectory(const PrivateImportStagingDirectory&) = delete;
    PrivateImportStagingDirectory& operator=(const PrivateImportStagingDirectory&) = delete;
    PrivateImportStagingDirectory(PrivateImportStagingDirectory&&) noexcept;
    PrivateImportStagingDirectory& operator=(PrivateImportStagingDirectory&&) noexcept;

    [[nodiscard]] const std::filesystem::path& path() const noexcept;
    void release() noexcept;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
    explicit PrivateImportStagingDirectory(std::unique_ptr<Impl> impl) noexcept;
    friend std::optional<PrivateImportStagingDirectory>
    create_private_import_staging_directory(const std::filesystem::path& destination_parent);
};

// Creates an exclusively named, owner-private staging directory on the
// destination volume. If the destination parent permits other users to
// replace children on POSIX, an eligible trusted ancestor is used instead.
// The name comes from operating-system randomness; existing entries are never
// adopted. The returned lease must live through the entire staging phase.
[[nodiscard]] std::optional<PrivateImportStagingDirectory>
create_private_import_staging_directory(const std::filesystem::path& destination_parent);

// Move-only handle produced by open_staged_import_file_for_publish(), used
// by materialize_database_json_import_plan() (#5679/#5680) to bind a
// staged import file's identity to the exact bytes it verified, from just
// after staging through final publication and any later rollback. The
// native handle/descriptor type is deliberately not named here -- see
// copperfin::security::PhysicalPathContainmentHandle's own doc comment for
// the same reasoning -- and is usable only through the three functions
// declared alongside this class.
class StagedImportFileHandle {
public:
    StagedImportFileHandle() noexcept;
    ~StagedImportFileHandle();
    StagedImportFileHandle(const StagedImportFileHandle&) = delete;
    StagedImportFileHandle& operator=(const StagedImportFileHandle&) = delete;
    StagedImportFileHandle(StagedImportFileHandle&&) noexcept;
    StagedImportFileHandle& operator=(StagedImportFileHandle&&) noexcept;

    [[nodiscard]] bool valid() const noexcept;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;

    friend StagedImportFileHandle open_staged_import_file_for_publish(
        const std::filesystem::path& staged_path,
        std::string_view expected_sha256);
    friend bool publish_staged_import_file(
        const StagedImportFileHandle& handle,
        const std::filesystem::path& staged_path,
        const std::filesystem::path& destination);
    friend bool remove_published_import_file_if_identity_matches(
        const StagedImportFileHandle& handle,
        const std::filesystem::path& published_path);

    explicit StagedImportFileHandle(std::unique_ptr<Impl> impl) noexcept;
};

// Opens `staged_path` (must already exist as a regular file, not a
// symlink/reparse point or directory) and pins its identity via a live
// native handle/descriptor. Returns an invalid handle (valid() == false)
// on any failure -- a caller must fail the whole import closed rather than
// fall back to path-based publication for that entry. When provided,
// expected_sha256 is the digest of bytes assembled by the writer before any
// staging pathname existed. It is checked against this retained handle,
// detecting a replacement between the writer's close and this open.
[[nodiscard]] StagedImportFileHandle open_staged_import_file_for_publish(
    const std::filesystem::path& staged_path,
    std::string_view expected_sha256 = {});

// Publishes the file `handle` was opened against as a new hard link at
// `destination`, which must not already exist. On Windows, `handle` was
// opened denying other processes write/delete/rename access to
// `staged_path` (FILE_SHARE_READ only), so `staged_path` is guaranteed to
// still name the original verified file at the moment of this call and
// publication is fully race-free. Linux links from the retained descriptor
// with linkat(AT_EMPTY_PATH) or the capability-free /proc/self/fd form, so a
// swapped staged pathname cannot redirect the published bytes; an unlinked
// original or unsupported backing filesystem fails closed. Other POSIX
// systems use the owner-private staging
// namespace plus an immediate pathname identity recheck; same-authority
// mutation between recheck and link remains a platform limitation. Returns
// false, leaving `destination` untouched, on failure or collision.
[[nodiscard]] bool publish_staged_import_file(
    const StagedImportFileHandle& handle,
    const std::filesystem::path& staged_path,
    const std::filesystem::path& destination);

// Removes `published_path` only if it still refers to the exact object
// `handle` was opened against (captured once, at staging time, before
// publication) -- used to roll back a successful publish after a later
// commit step in the same transaction fails. Returns false, leaving
// `published_path` untouched, if identity cannot be confirmed (including
// if the path no longer exists or now names a different object). A caller
// must treat false as "could not safely reclaim this entry," not as a
// hard error to propagate past the rest of rollback.
[[nodiscard]] bool remove_published_import_file_if_identity_matches(
    const StagedImportFileHandle& handle,
    const std::filesystem::path& published_path);

// Releases every handle in `handles` (its own identity-binding protection
// is no longer needed once execution reaches this point -- every caller of
// this function is done deciding whether each corresponding entry gets
// published or rolled back), then removes `staged_paths[i]` for each
// released `handles[i]` (`handles.size()` must equal `staged_paths.size()`),
// and finally removes `staging_dir` itself. Every removal is attempted
// regardless of an earlier one failing (best-effort, not fail-fast).
// Returns true only if every one of those removals succeeded -- an
// already-absent path counts as success, matching
// std::filesystem::remove()'s own convention -- and false if any staged
// alias and/or the staging directory itself may still remain (#5681/
// #5682): a caller must not report this as complete cleanup, since a
// remaining staged file is a live hard-link alias to already-published
// data, not harmless empty scratch state.
[[nodiscard]] bool release_and_remove_staged_files(
    std::vector<StagedImportFileHandle>& handles,
    const std::vector<std::filesystem::path>& staged_paths,
    const std::filesystem::path& staging_dir);

}  // namespace copperfin::vfp
