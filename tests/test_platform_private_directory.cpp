// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/private_directory.h"
#include "copperfin/platform/private_executable_image.h"

#include <chrono>
#include <cerrno>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <sys/stat.h>
#if defined(__APPLE__)
#include <membership.h>
#include <sys/acl.h>
#include <unistd.h>
#elif defined(__linux__)
#include <linux/posix_acl.h>
#include <linux/posix_acl_xattr.h>
#include <sys/xattr.h>
#endif
#endif

namespace {

using copperfin::platform::PrivateDirectoryFailure;
using copperfin::platform::create_private_directory;
using copperfin::platform::create_private_directory_in_verified_parent;
using copperfin::platform::remove_empty_private_directory_in_verified_parent;
using copperfin::platform::verify_private_directory;
using copperfin::platform::PrivateExecutableImageFailure;
using copperfin::platform::materialize_private_executable_image_in_verified_parent;

int failures = 0;

void expect(const bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
}

class TempTree {
public:
    TempTree() {
        const auto suffix = std::chrono::steady_clock::now()
                                .time_since_epoch()
                                .count();
        root = std::filesystem::canonical(
                   std::filesystem::temp_directory_path()) /
            ("copperfin-private-directory-" + std::to_string(suffix));
        std::filesystem::create_directory(root);
#if !defined(_WIN32)
        std::filesystem::permissions(
            root,
            std::filesystem::perms::owner_all,
            std::filesystem::perm_options::replace);
#endif
    }

    ~TempTree() {
        std::error_code ignored;
        std::filesystem::remove_all(root, ignored);
    }

    std::filesystem::path root;
};

struct TestDirectoryIdentity {
    std::uint64_t storage_id = 0U;
    std::uint64_t file_id = 0U;
};

TestDirectoryIdentity directory_identity(const std::filesystem::path& path) {
#if defined(_WIN32)
    const HANDLE directory = ::CreateFileW(
        path.c_str(), FILE_READ_ATTRIBUTES,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT, nullptr);
    if (directory == INVALID_HANDLE_VALUE) {
        return {};
    }
    BY_HANDLE_FILE_INFORMATION information{};
    const bool inspected =
        ::GetFileInformationByHandle(directory, &information) != 0;
    ::CloseHandle(directory);
    if (!inspected) {
        return {};
    }
    return {
        .storage_id = information.dwVolumeSerialNumber,
        .file_id =
            (static_cast<std::uint64_t>(information.nFileIndexHigh) << 32U) |
            information.nFileIndexLow};
#else
    struct stat status{};
    if (::stat(path.c_str(), &status) != 0) {
        return {};
    }
    return {
        .storage_id = static_cast<std::uint64_t>(status.st_dev),
        .file_id = static_cast<std::uint64_t>(status.st_ino)};
#endif
}

void test_creation_and_verification() {
    TempTree tree;
    const auto private_root = tree.root / "private-root";
    const auto created = create_private_directory(private_root);
    expect(created.ok && created.failure == PrivateDirectoryFailure::none &&
               verify_private_directory(private_root).ok,
           "RQ-CF-AGENT-014: creation must produce a directly verifiable private directory");

    const auto child = private_root / "child";
    expect(create_private_directory(child).ok &&
               verify_private_directory(child).ok,
           "RQ-CF-AGENT-014: a private directory must support an explicit private child");

    const auto identity = directory_identity(private_root);
    const auto bound_child = create_private_directory_in_verified_parent(
        private_root, identity.storage_id, identity.file_id, "bound-child");
    expect(bound_child.ok &&
               verify_private_directory(private_root / "bound-child").ok,
           "RQ-CF-AGENT-014: identity-bound creation must create one direct private child");

    const auto removed = remove_empty_private_directory_in_verified_parent(
        private_root, identity.storage_id, identity.file_id, "bound-child",
        bound_child.storage_id, bound_child.file_id);
    expect(removed.ok &&
               !std::filesystem::exists(private_root / "bound-child"),
           "RQ-CF-AGENT-020: exact empty private child cleanup must remove only the identity-bound directory");

    const auto retained = create_private_directory_in_verified_parent(
        private_root, identity.storage_id, identity.file_id, "retained-child");
    std::ofstream(private_root / "retained-child" / "content") << "retain\n";
    const auto nonempty = remove_empty_private_directory_in_verified_parent(
        private_root, identity.storage_id, identity.file_id, "retained-child",
        retained.storage_id, retained.file_id);
    expect(!nonempty.ok && nonempty.failure == PrivateDirectoryFailure::not_empty &&
               std::filesystem::exists(
                   private_root / "retained-child" / "content"),
           "RQ-CF-AGENT-020: cleanup must never traverse or remove directory content");

    const auto wrong_target = remove_empty_private_directory_in_verified_parent(
        private_root, identity.storage_id, identity.file_id, "retained-child",
        retained.storage_id, retained.file_id ^ 1U);
    expect(!wrong_target.ok &&
               wrong_target.failure == PrivateDirectoryFailure::verification_failed &&
               std::filesystem::exists(private_root / "retained-child"),
           "RQ-CF-AGENT-020: target identity mismatch must preserve the directory");
    const auto wrong_parent = remove_empty_private_directory_in_verified_parent(
        private_root, identity.storage_id, identity.file_id ^ 1U,
        "retained-child", retained.storage_id, retained.file_id);
    expect(!wrong_parent.ok &&
               wrong_parent.failure ==
                   PrivateDirectoryFailure::parent_identity_changed &&
               std::filesystem::exists(private_root / "retained-child"),
           "RQ-CF-AGENT-020: parent identity mismatch must preserve the directory");
    const auto ambiguous = remove_empty_private_directory_in_verified_parent(
        private_root, identity.storage_id, identity.file_id, "../retained-child",
        retained.storage_id, retained.file_id);
    expect(!ambiguous.ok &&
               ambiguous.failure == PrivateDirectoryFailure::invalid_path &&
               std::filesystem::exists(private_root / "retained-child"),
           "RQ-CF-AGENT-020: ambiguous cleanup leaves must fail without removal");
    const auto mismatched = create_private_directory_in_verified_parent(
        private_root, identity.storage_id, identity.file_id ^ 1U,
        "mismatched-child");
    expect(!mismatched.ok &&
               mismatched.failure ==
                   PrivateDirectoryFailure::parent_identity_changed &&
               !std::filesystem::exists(private_root / "mismatched-child"),
           "RQ-CF-AGENT-014: parent identity mismatch must fail before child creation");

    const auto duplicate = create_private_directory(child);
    expect(!duplicate.ok &&
               duplicate.failure == PrivateDirectoryFailure::already_exists,
           "RQ-CF-AGENT-014: existing directories must never be adopted or modified");

    const auto ordinary = private_root / "ordinary";
    std::filesystem::create_directory(ordinary);
#if !defined(_WIN32)
    std::filesystem::permissions(
        ordinary,
        std::filesystem::perms::owner_all |
            std::filesystem::perms::group_read |
            std::filesystem::perms::group_exec,
        std::filesystem::perm_options::replace);
#endif
    expect(!verify_private_directory(ordinary).ok,
           "RQ-CF-AGENT-014: inherited or broadened directory security must fail verification");
    const auto ordinary_create = create_private_directory(ordinary);
    expect(!ordinary_create.ok &&
               ordinary_create.failure == PrivateDirectoryFailure::already_exists &&
               !verify_private_directory(ordinary).ok,
           "RQ-CF-AGENT-014: creation must not repair an existing broadened directory");

#if !defined(_WIN32)
    const auto special = private_root / "special";
    expect(create_private_directory(special).ok,
           "RQ-CF-AGENT-014: the special-mode fixture must start private");
    std::filesystem::permissions(
        special,
        std::filesystem::perms::owner_all |
            std::filesystem::perms::set_gid,
        std::filesystem::perm_options::replace);
    expect(!verify_private_directory(special).ok,
           "RQ-CF-AGENT-014: setuid, setgid, and sticky permission bits must violate exact mode 0700");

    const auto restricted = private_root / "restricted-umask";
    const mode_t prior_umask = ::umask(0700);
    const auto restricted_result = create_private_directory(restricted);
    ::umask(prior_umask);
    expect(!restricted_result.ok &&
               restricted_result.failure ==
                   PrivateDirectoryFailure::verification_failed &&
               std::filesystem::is_directory(restricted) &&
               !verify_private_directory(restricted).ok,
           "RQ-CF-AGENT-014: a restrictive owner-bit umask must fail closed without path-based repair or deletion");

    const auto writable_parent = tree.root / "writable-parent";
    std::filesystem::create_directory(writable_parent);
    std::filesystem::permissions(
        writable_parent,
        std::filesystem::perms::all,
        std::filesystem::perm_options::replace);
    const auto rejected_child = writable_parent / "rejected-child";
    const auto rejected_result = create_private_directory(rejected_child);
    expect(!rejected_result.ok &&
               rejected_result.failure == PrivateDirectoryFailure::access_denied &&
               !std::filesystem::exists(rejected_child),
           "RQ-CF-AGENT-014: a non-sticky parent writable by another principal must fail before creation");
    std::filesystem::permissions(
        writable_parent,
        std::filesystem::perms::all |
            std::filesystem::perms::sticky_bit,
        std::filesystem::perm_options::replace);
    const auto sticky_child = writable_parent / "sticky-child";
    expect(create_private_directory(sticky_child).ok &&
               verify_private_directory(sticky_child).ok,
           "RQ-CF-AGENT-014: a trusted sticky parent must retain safe leaf creation");

    const auto acl_directory = private_root / "extended-acl";
    expect(create_private_directory(acl_directory).ok,
           "RQ-CF-AGENT-014: the extended-ACL fixture must start private");
#if defined(__APPLE__)
    acl_t acl = ::acl_init(1);
    acl_entry_t entry = nullptr;
    uuid_t qualifier{};
    acl_permset_t permissions = nullptr;
    const bool acl_installed = acl != nullptr &&
        ::mbr_uid_to_uuid(::geteuid(), qualifier) == 0 &&
        ::acl_create_entry(&acl, &entry) == 0 &&
        ::acl_set_tag_type(entry, ACL_EXTENDED_ALLOW) == 0 &&
        ::acl_set_qualifier(entry, qualifier) == 0 &&
        ::acl_get_permset(entry, &permissions) == 0 &&
        ::acl_clear_perms(permissions) == 0 &&
        ::acl_add_perm(permissions, ACL_READ_DATA) == 0 &&
        ::acl_set_permset(entry, permissions) == 0 &&
        ::acl_set_file(
            acl_directory.c_str(), ACL_TYPE_EXTENDED, acl) == 0;
    if (acl != nullptr) {
        (void)::acl_free(acl);
    }
    expect(acl_installed && !verify_private_directory(acl_directory).ok,
           "RQ-CF-AGENT-014: a macOS extended ACL must violate private verification");
#elif defined(__linux__)
    struct DefaultAclBlob {
        posix_acl_xattr_header header{};
        posix_acl_xattr_entry entries[3]{};
    } acl_blob;
    acl_blob.header.a_version = POSIX_ACL_XATTR_VERSION;
    acl_blob.entries[0] = {
        .e_tag = ACL_USER_OBJ,
        .e_perm = 7U,
        .e_id = static_cast<__le32>(ACL_UNDEFINED_ID)};
    acl_blob.entries[1] = {
        .e_tag = ACL_GROUP_OBJ,
        .e_perm = 0U,
        .e_id = static_cast<__le32>(ACL_UNDEFINED_ID)};
    acl_blob.entries[2] = {
        .e_tag = ACL_OTHER,
        .e_perm = 0U,
        .e_id = static_cast<__le32>(ACL_UNDEFINED_ID)};
    const int acl_result = ::setxattr(
        acl_directory.c_str(), "system.posix_acl_default",
        &acl_blob, sizeof(acl_blob), 0);
    const int acl_error = errno;
    expect(
        (acl_result == 0 && !verify_private_directory(acl_directory).ok) ||
            (acl_result == -1 &&
                (acl_error == ENOTSUP || acl_error == EOPNOTSUPP)),
        "RQ-CF-AGENT-014: a Linux POSIX ACL must violate private verification when the filesystem supports ACLs");
#endif
#endif
}

void test_invalid_and_wrong_kind_inputs() {
    TempTree tree;
    expect(create_private_directory({}).failure ==
               PrivateDirectoryFailure::invalid_path &&
               create_private_directory("relative").failure ==
                   PrivateDirectoryFailure::invalid_path,
           "RQ-CF-AGENT-014: empty and relative creation paths must fail closed");

    const auto missing_parent = tree.root / "missing" / "child";
    expect(create_private_directory(missing_parent).failure ==
               PrivateDirectoryFailure::parent_unavailable,
           "RQ-CF-AGENT-014: creation must not fabricate intermediate directories");

    const auto dotted = tree.root / "." / "dotted";
    expect(create_private_directory(dotted).failure ==
               PrivateDirectoryFailure::invalid_path &&
               !std::filesystem::exists(tree.root / "dotted"),
           "RQ-CF-AGENT-014: dot-component ambiguity must fail without creating a leaf");

    const auto file = tree.root / "file";
    std::ofstream(file, std::ios::binary) << "not a directory\n";
    const auto file_create = create_private_directory(file);
    expect(!verify_private_directory(file).ok && !file_create.ok &&
               file_create.failure == PrivateDirectoryFailure::already_exists,
           "RQ-CF-AGENT-014: a regular file must fail verification and remain unmodified");

    const auto target = tree.root / "target";
    expect(create_private_directory(target).ok,
           "RQ-CF-AGENT-014: symlink fixture target must be private");
    const auto indirect = tree.root / "indirect";
    std::error_code link_error;
    std::filesystem::create_directory_symlink(target, indirect, link_error);
    if (!link_error) {
        expect(!verify_private_directory(indirect).ok,
               "RQ-CF-AGENT-014: symbolic links and directory reparse points must fail closed");

        const auto indirect_child = indirect / "child";
        const auto indirect_creation =
            create_private_directory(indirect_child);
        expect(!indirect_creation.ok &&
                   indirect_creation.failure ==
                       PrivateDirectoryFailure::parent_unavailable &&
                   !std::filesystem::exists(target / "child"),
               "RQ-CF-AGENT-014: creation must reject a symbolic-link parent without modifying its target");

        const auto direct_child = target / "direct-child";
        expect(create_private_directory(direct_child).ok &&
                   !verify_private_directory(indirect / "direct-child").ok,
               "RQ-CF-AGENT-014: verification must reject a private leaf reached through a symbolic-link parent");
    }
}

void test_exact_private_executable_image_materialization() {
    TempTree tree;
    const auto private_root = tree.root / "image-root";
    const auto created = create_private_directory(private_root);
    expect(created.ok,
           "RQ-CF-AGENT-026: the image fixture must create a private parent");
    if (!created.ok) {
        return;
    }
    const auto identity = directory_identity(private_root);
#if defined(_WIN32)
    const std::filesystem::path leaf = "copperfin-image-1.exe";
#else
    const std::filesystem::path leaf = "copperfin-image-1.bin";
#endif
    const std::vector<std::uint8_t> bytes{
        0x43U, 0x6fU, 0x70U, 0x70U, 0x65U, 0x72U, 0x66U, 0x69U, 0x6eU};
    auto materialized =
        materialize_private_executable_image_in_verified_parent(
            private_root, identity.storage_id, identity.file_id, leaf, bytes);
    expect(materialized.materialized && materialized.image.has_value() &&
               materialized.image->valid() &&
               materialized.failure == PrivateExecutableImageFailure::none,
           "RQ-CF-AGENT-026: exact bytes must become one owned executable image");
#if defined(_WIN32)
    std::ifstream retained(private_root / leaf, std::ios::binary);
    const std::vector<std::uint8_t> retained_bytes{
        std::istreambuf_iterator<char>(retained),
        std::istreambuf_iterator<char>()};
    expect(retained_bytes == bytes,
           "RQ-CF-AGENT-026: the retained Windows image must contain only the supplied bytes");
    const HANDLE denied_writer = ::CreateFileW(
        (private_root / leaf).c_str(), GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    expect(denied_writer == INVALID_HANDLE_VALUE,
           "RQ-CF-AGENT-026: a live Windows image must deny cooperating writers");
    if (denied_writer != INVALID_HANDLE_VALUE) {
        (void)::CloseHandle(denied_writer);
    }
    retained.close();
#else
    expect(!std::filesystem::exists(private_root / leaf),
           "RQ-CF-AGENT-026: a POSIX image must leave no mutable pathname after creation");
#endif
    materialized.image.reset();
    expect(!std::filesystem::exists(private_root / leaf),
           "RQ-CF-AGENT-026: image destruction must remove the exact retained object");

    const auto wrong_parent =
        materialize_private_executable_image_in_verified_parent(
            private_root, identity.storage_id, identity.file_id ^ 1U,
            "wrong-parent-image", bytes);
    expect(!wrong_parent.materialized && !wrong_parent.image.has_value() &&
               wrong_parent.failure ==
                   PrivateExecutableImageFailure::parent_identity_changed &&
               !std::filesystem::exists(private_root / "wrong-parent-image"),
           "RQ-CF-AGENT-026: parent replacement must fail before image creation");

    const auto existing = private_root / "existing-image";
    std::ofstream(existing, std::ios::binary) << "preserve";
    const auto collision =
        materialize_private_executable_image_in_verified_parent(
            private_root, identity.storage_id, identity.file_id,
            "existing-image", bytes);
    std::ifstream preserved(existing, std::ios::binary);
    const std::string preserved_text{
        std::istreambuf_iterator<char>(preserved),
        std::istreambuf_iterator<char>()};
    expect(!collision.materialized && !collision.image.has_value() &&
               collision.failure == PrivateExecutableImageFailure::already_exists &&
               preserved_text == "preserve",
           "RQ-CF-AGENT-026: an existing leaf must be preserved rather than adopted or overwritten");
}

}  // namespace

int main() {
    test_creation_and_verification();
    test_invalid_and_wrong_kind_inputs();
    test_exact_private_executable_image_materialization();
    if (failures != 0) {
        std::cerr << failures << " private-directory checks failed\n";
        return 1;
    }
    std::cout << "private-directory checks passed\n";
    return 0;
}
