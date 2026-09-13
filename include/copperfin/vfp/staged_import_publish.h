// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include <filesystem>
#include <memory>

namespace copperfin::vfp {

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
        const std::filesystem::path& staged_path);
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
// fall back to path-based publication for that entry.
[[nodiscard]] StagedImportFileHandle open_staged_import_file_for_publish(
    const std::filesystem::path& staged_path);

// Publishes the file `handle` was opened against as a new hard link at
// `destination`, which must not already exist. On Windows, `handle` was
// opened denying other processes write/delete/rename access to
// `staged_path` (FILE_SHARE_READ only), so `staged_path` is guaranteed to
// still name the original verified file at the moment of this call and
// publication is fully race-free. On POSIX, no such share-deny exists, and
// no primitive lets this function hard-link "the object `handle` refers
// to" once its last directory entry has been removed -- instead this
// re-verifies `staged_path`'s identity against what `handle` captured at
// staging time immediately before linking, and fails closed (rather than
// publishing either stale or substituted content) if it no longer matches,
// narrowing the race to the two syscalls immediately around that check
// instead of the whole staging-to-commit window. Returns false, leaving
// `destination` untouched, on any failure, including if `destination`
// already exists or `staged_path`'s identity no longer matches.
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

}  // namespace copperfin::vfp
