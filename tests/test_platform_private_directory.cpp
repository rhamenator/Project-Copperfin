// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/private_directory.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#if !defined(_WIN32)
#include <sys/stat.h>
#endif

namespace {

using copperfin::platform::PrivateDirectoryFailure;
using copperfin::platform::create_private_directory;
using copperfin::platform::verify_private_directory;

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
        root = std::filesystem::temp_directory_path() /
            ("copperfin-private-directory-" + std::to_string(suffix));
        std::filesystem::create_directory(root);
    }

    ~TempTree() {
        std::error_code ignored;
        std::filesystem::remove_all(root, ignored);
    }

    std::filesystem::path root;
};

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
    }
}

}  // namespace

int main() {
    test_creation_and_verification();
    test_invalid_and_wrong_kind_inputs();
    if (failures != 0) {
        std::cerr << failures << " private-directory checks failed\n";
        return 1;
    }
    std::cout << "private-directory checks passed\n";
    return 0;
}
