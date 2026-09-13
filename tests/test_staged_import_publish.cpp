// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/vfp/staged_import_publish.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>

namespace {

int failures = 0;

void expect(bool condition, std::string_view message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

namespace fs = std::filesystem;

void write_file(const fs::path& path, std::string_view contents) {
    std::ofstream stream(path, std::ios::binary | std::ios::trunc);
    stream << contents;
}

std::string read_file(const fs::path& path) {
    std::ifstream stream(path, std::ios::binary);
    return std::string(
        std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>());
}

fs::path make_scratch_dir(std::string_view name) {
    const fs::path dir = fs::temp_directory_path() / name;
    std::error_code ignored;
    fs::remove_all(dir, ignored);
    fs::create_directories(dir);
    return dir;
}

void test_open_succeeds_on_regular_file() {
    const fs::path dir = make_scratch_dir("copperfin_staged_import_publish_open_ok");
    const fs::path staged = dir / "staged.dbf";
    write_file(staged, "original bytes");

    const auto handle = copperfin::vfp::open_staged_import_file_for_publish(staged);
    expect(handle.valid(), "opening a freshly written regular file should succeed");

    std::error_code ignored;
    fs::remove_all(dir, ignored);
}

void test_open_rejects_missing_file() {
    const fs::path dir = make_scratch_dir("copperfin_staged_import_publish_open_missing");
    const fs::path staged = dir / "does-not-exist.dbf";

    const auto handle = copperfin::vfp::open_staged_import_file_for_publish(staged);
    expect(!handle.valid(), "opening a nonexistent staged path should fail closed");

    std::error_code ignored;
    fs::remove_all(dir, ignored);
}

void test_open_rejects_directory() {
    const fs::path dir = make_scratch_dir("copperfin_staged_import_publish_open_directory");
    const fs::path subdirectory = dir / "not-a-file.dbf";
    fs::create_directories(subdirectory);

    const auto handle = copperfin::vfp::open_staged_import_file_for_publish(subdirectory);
    expect(!handle.valid(), "opening a directory as a staged file should fail closed");

    std::error_code ignored;
    fs::remove_all(dir, ignored);
}

#if !defined(_WIN32)
void test_open_rejects_symlink() {
    const fs::path dir = make_scratch_dir("copperfin_staged_import_publish_open_symlink");
    const fs::path real_file = dir / "real.dbf";
    write_file(real_file, "real bytes");
    const fs::path staged_symlink = dir / "staged.dbf";
    fs::create_symlink(real_file, staged_symlink);

    const auto handle = copperfin::vfp::open_staged_import_file_for_publish(staged_symlink);
    expect(!handle.valid(), "opening a symlink as a staged file should fail closed (O_NOFOLLOW)");

    std::error_code ignored;
    fs::remove_all(dir, ignored);
}
#endif

// #5680: the core security property. publish_staged_import_file() must
// never let bytes a concurrent actor swapped into staged_path's name after
// opening reach `destination` under the original, verified name -- POSIX
// has no primitive to hard-link "the object the handle refers to" once its
// last directory entry is gone (confirmed empirically: the tempting
// /proc/self/fd + linkat(AT_SYMLINK_FOLLOW) trick does not survive an
// intervening unlink), so the achievable, still-sound property is fail-
// closed detection instead: a staged_path identity mismatch at publish
// time must refuse to publish anything for that entry, never the
// substituted content and never stale content under a name that no longer
// verifiably matches.
void test_publish_fails_closed_when_staged_path_is_swapped_before_publish() {
    const fs::path dir = make_scratch_dir("copperfin_staged_import_publish_swap_before_publish");
    const fs::path staged = dir / "staged.dbf";
    const fs::path destination = dir / "final.dbf";
    write_file(staged, "verified original bytes");

    const auto handle = copperfin::vfp::open_staged_import_file_for_publish(staged);
    expect(handle.valid(), "opening the staged file before the swap should succeed");

    // Simulate a concurrent replacement of staged_path between staging and
    // commit: remove the original name and put different content at the
    // same name.
    std::error_code remove_error;
    fs::remove(staged, remove_error);
    write_file(staged, "swapped-in malicious bytes");

    const bool published =
        copperfin::vfp::publish_staged_import_file(handle, staged, destination);
    expect(!published, "publish must fail closed once staged_path's identity no longer matches "
                        "what was verified at staging time");

    std::error_code exists_error;
    expect(!fs::exists(destination, exists_error),
           "a fail-closed publish must not create any file at the destination");

    std::error_code ignored;
    fs::remove_all(dir, ignored);
}

void test_publish_fails_if_destination_already_exists() {
    const fs::path dir = make_scratch_dir("copperfin_staged_import_publish_destination_exists");
    const fs::path staged = dir / "staged.dbf";
    const fs::path destination = dir / "final.dbf";
    write_file(staged, "staged bytes");
    write_file(destination, "pre-existing destination bytes");

    const auto handle = copperfin::vfp::open_staged_import_file_for_publish(staged);
    expect(handle.valid(), "opening the staged file should succeed");

    const bool published =
        copperfin::vfp::publish_staged_import_file(handle, staged, destination);
    expect(!published, "publish must fail closed when the destination already exists");
    expect(read_file(destination) == "pre-existing destination bytes",
           "a failed publish must leave the pre-existing destination untouched");

    std::error_code ignored;
    fs::remove_all(dir, ignored);
}

void test_publish_fails_on_invalid_handle() {
    const fs::path dir = make_scratch_dir("copperfin_staged_import_publish_invalid_handle");
    const fs::path destination = dir / "final.dbf";
    const copperfin::vfp::StagedImportFileHandle invalid_handle;

    const bool published = copperfin::vfp::publish_staged_import_file(
        invalid_handle, dir / "never-staged.dbf", destination);
    expect(!published, "publish must fail closed given a default-constructed (invalid) handle");

    std::error_code ignored;
    fs::remove_all(dir, ignored);
}

void test_remove_published_file_when_identity_matches() {
    const fs::path dir = make_scratch_dir("copperfin_staged_import_publish_remove_match");
    const fs::path staged = dir / "staged.dbf";
    const fs::path destination = dir / "final.dbf";
    write_file(staged, "committed bytes");

    const auto handle = copperfin::vfp::open_staged_import_file_for_publish(staged);
    expect(handle.valid(), "opening the staged file should succeed");
    expect(copperfin::vfp::publish_staged_import_file(handle, staged, destination),
           "publish should succeed for the positive-path removal test");

    const bool removed =
        copperfin::vfp::remove_published_import_file_if_identity_matches(handle, destination);
    expect(removed, "rollback should remove a destination that still matches published identity");

    std::error_code exists_error;
    expect(!fs::exists(destination, exists_error),
           "the destination should no longer exist after a matching-identity removal");

    fs::remove_all(dir, exists_error);
}

// #5679: the core security property. A destination replaced by a
// concurrent actor after this transaction published it there must survive
// rollback -- remove_published_import_file_if_identity_matches() must
// refuse to delete it, not blindly remove whatever now has that name.
void test_remove_published_file_preserves_replaced_destination() {
    const fs::path dir = make_scratch_dir("copperfin_staged_import_publish_remove_mismatch");
    const fs::path staged = dir / "staged.dbf";
    const fs::path destination = dir / "final.dbf";
    write_file(staged, "committed bytes");

    const auto handle = copperfin::vfp::open_staged_import_file_for_publish(staged);
    expect(handle.valid(), "opening the staged file should succeed");
    expect(copperfin::vfp::publish_staged_import_file(handle, staged, destination),
           "publish should succeed for the identity-mismatch removal test");

    // Simulate a concurrent replacement of the *final* published path
    // before this transaction's rollback runs.
    std::error_code remove_error;
    fs::remove(destination, remove_error);
    write_file(destination, "unrelated data created after publish");

    const bool removed =
        copperfin::vfp::remove_published_import_file_if_identity_matches(handle, destination);
    expect(!removed, "rollback must refuse to remove a destination whose identity has changed");
    expect(read_file(destination) == "unrelated data created after publish",
           "a refused rollback removal must leave the replacement file's content untouched");

    std::error_code ignored;
    fs::remove_all(dir, ignored);
}

void test_remove_published_file_fails_closed_when_destination_missing() {
    const fs::path dir = make_scratch_dir("copperfin_staged_import_publish_remove_missing");
    const fs::path staged = dir / "staged.dbf";
    const fs::path destination = dir / "final.dbf";
    write_file(staged, "committed bytes");

    const auto handle = copperfin::vfp::open_staged_import_file_for_publish(staged);
    expect(handle.valid(), "opening the staged file should succeed");
    expect(copperfin::vfp::publish_staged_import_file(handle, staged, destination),
           "publish should succeed for the missing-destination removal test");

    std::error_code remove_error;
    fs::remove(destination, remove_error);

    const bool removed =
        copperfin::vfp::remove_published_import_file_if_identity_matches(handle, destination);
    expect(!removed, "removal must report false, not crash, when the destination is already gone");

    std::error_code ignored;
    fs::remove_all(dir, ignored);
}

}  // namespace

int main() {
    test_open_succeeds_on_regular_file();
    test_open_rejects_missing_file();
    test_open_rejects_directory();
#if !defined(_WIN32)
    test_open_rejects_symlink();
#endif
    test_publish_fails_closed_when_staged_path_is_swapped_before_publish();
    test_publish_fails_if_destination_already_exists();
    test_publish_fails_on_invalid_handle();
    test_remove_published_file_when_identity_matches();
    test_remove_published_file_preserves_replaced_destination();
    test_remove_published_file_fails_closed_when_destination_missing();

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
