// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include <cstdint>
#include <filesystem>

namespace copperfin::platform {

// Governing requirements: RQ-CF-AGENT-014 and candidate RQ-CF-AGENT-020.

enum class PrivateDirectoryFailure : std::uint32_t {
    none = 0U,
    invalid_path,
    parent_unavailable,
    already_exists,
    access_denied,
    parent_identity_changed,
    security_unavailable,
    creation_failed,
    verification_failed,
    not_empty,
    removal_failed
};

struct PrivateDirectoryResult {
    bool ok = false;
    PrivateDirectoryFailure failure = PrivateDirectoryFailure::invalid_path;
    std::uint64_t storage_id = 0U;
    std::uint64_t file_id = 0U;
};

// Creates exactly one absolute directory leaf. The parent must already exist.
// POSIX requires an extended-ACL-free, effective-user- or root-owned immediate
// parent that either denies group/other writes or applies sticky rename
// protection, then creates extended-ACL-free effective-user-owned mode 0700.
// Windows uses a protected DACL granting inheritable full control only to the
// process user and LocalSystem. Existing objects are never adopted or modified.
[[nodiscard]] PrivateDirectoryResult create_private_directory(
    const std::filesystem::path& path) noexcept;

// Creates one direct leaf only after opening the expected private parent and
// matching its platform storage/file identity. POSIX creates relative to that
// bound descriptor. Windows repeats the parent-handle identity check around
// its public full-path creation API; callers must retain the documented
// same-authority/full-path limitation.
[[nodiscard]] PrivateDirectoryResult create_private_directory_in_verified_parent(
    const std::filesystem::path& parent,
    std::uint64_t expected_storage_id,
    std::uint64_t expected_file_id,
    const std::filesystem::path& leaf) noexcept;

// Verifies the same platform privacy contract without changing the object.
// Symbolic links, reparse points, non-directories, inherited/broad Windows
// DACLs, foreign POSIX ownership, POSIX extended ACLs, and non-0700 POSIX modes
// fail closed.
[[nodiscard]] PrivateDirectoryResult verify_private_directory(
    const std::filesystem::path& path) noexcept;

// Removes one exact, empty, private direct child after binding the expected
// private parent and matching both platform identities. It never traverses or
// removes child content. Windows deletes the identity-bound directory through
// its open handle. POSIX retains the verified child descriptor while issuing
// the direct-parent unlink; callers must preserve the documented
// same-authority leaf-name race limitation until a platform-independent
// descriptor deletion primitive or sandbox exclusion is available.
[[nodiscard]] PrivateDirectoryResult
remove_empty_private_directory_in_verified_parent(
    const std::filesystem::path& parent,
    std::uint64_t expected_parent_storage_id,
    std::uint64_t expected_parent_file_id,
    const std::filesystem::path& leaf,
    std::uint64_t expected_directory_storage_id,
    std::uint64_t expected_directory_file_id) noexcept;

}  // namespace copperfin::platform
