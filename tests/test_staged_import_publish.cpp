// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/vfp/staged_import_publish.h"
#include "copperfin/platform/private_directory.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>
#if !defined(_WIN32)
#include <sys/stat.h>
#endif

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

void test_private_staging_directory_is_new_and_restricted() {
    const fs::path dir = make_scratch_dir("copperfin_staged_import_private_directory");
#if !defined(_WIN32)
    // Force an untrusted destination parent regardless of runner umask.
    fs::permissions(dir, fs::perms::group_write, fs::perm_options::add);
#endif
    auto first = copperfin::vfp::create_private_import_staging_directory(dir);
    auto second = copperfin::vfp::create_private_import_staging_directory(dir);
    expect(first.has_value() && second.has_value() && first->path() != second->path(),
           "each import must receive a newly created staging directory");
    if (first.has_value()) {
        expect(copperfin::platform::verify_private_directory(first->path()).ok,
               "first staging directory must satisfy the platform privacy contract");
#if !defined(_WIN32)
        struct stat destination_status{};
        struct stat staging_status{};
        expect(::stat(dir.c_str(), &destination_status) == 0 &&
                   ::stat(first->path().c_str(), &staging_status) == 0 &&
                   destination_status.st_dev == staging_status.st_dev,
               "staging and destination must remain on the same volume");
        expect(first->path().parent_path() != dir,
               "an untrusted destination parent must be skipped");
#endif
    }
    if (second.has_value()) {
        expect(copperfin::platform::verify_private_directory(second->path()).ok,
               "second staging directory must satisfy the platform privacy contract");
    }
    const fs::path missing_parent = dir / "missing";
    expect(!copperfin::vfp::create_private_import_staging_directory(missing_parent).has_value(),
           "staging must not create or adopt an unverified parent directory");
    expect(!fs::exists(missing_parent), "failed staging must leave a missing parent absent");
    std::error_code ignored;
    if (first.has_value()) {
        first->release();
        fs::remove_all(first->path(), ignored);
    }
    if (second.has_value()) {
        second->release();
        fs::remove_all(second->path(), ignored);
    }
    fs::remove_all(dir, ignored);
}

#if !defined(_WIN32)
void test_private_staging_accepts_indirect_destination_parent() {
    const fs::path dir = make_scratch_dir("copperfin_staged_import_indirect_parent");
    const fs::path real = dir / "real";
    const fs::path alias = dir / "alias";
    fs::create_directory(real);
    fs::create_directory_symlink(real, alias);
    auto staged = copperfin::vfp::create_private_import_staging_directory(alias);
    expect(staged.has_value(), "an existing symlink alias to the destination must be resolved");
    if (staged.has_value()) {
        expect(copperfin::platform::verify_private_directory(staged->path()).ok,
               "indirect destination parent must still produce private staging");
        std::error_code ignored;
        staged->release();
        fs::remove_all(staged->path(), ignored);
    }
    std::error_code ignored;
    fs::remove_all(dir, ignored);
}
#endif

#if defined(_WIN32)
void test_private_staging_lease_blocks_parent_rename() {
    const fs::path dir = make_scratch_dir("copperfin_staged_import_locked_parent");
    const fs::path moved = dir.parent_path() / "copperfin_staged_import_locked_parent_moved";
    std::error_code ignored;
    fs::remove_all(moved, ignored);
    auto staged = copperfin::vfp::create_private_import_staging_directory(dir);
    expect(staged.has_value(), "Windows staging must pin its directory chain");
    if (staged.has_value()) {
        std::error_code rename_error;
        fs::rename(dir, moved, rename_error);
        expect(static_cast<bool>(rename_error),
               "Windows must deny parent replacement while staging is active");
        staged->release();
        fs::remove_all(staged->path(), ignored);
    }
    fs::remove_all(dir, ignored);
    fs::remove_all(moved, ignored);
}
#endif

void test_open_succeeds_on_regular_file() {
    const fs::path dir = make_scratch_dir("copperfin_staged_import_publish_open_ok");
    const fs::path staged = dir / "staged.dbf";
    write_file(staged, "original bytes");

    const auto handle = copperfin::vfp::open_staged_import_file_for_publish(
        staged, "52c3935626c104b2cbc9031291a1c4d56614c38f52072a361d658a58a9c48698");
    expect(handle.valid(), "opening a freshly written regular file should succeed");

    std::error_code ignored;
    fs::remove_all(dir, ignored);
}

void test_open_rejects_replacement_before_handle_acquisition() {
    const fs::path dir = make_scratch_dir("copperfin_staged_import_preopen_swap");
    const fs::path staged = dir / "staged.dbf";
    // The expected digest was captured from the writer's in-memory bytes.
    // Simulate a replacement before the importer's first read handle opens.
    write_file(staged, "swapped-in malicious bytes");
    const auto handle = copperfin::vfp::open_staged_import_file_for_publish(
        staged, "925180315c34a6f857b152519752f58b338c8791cab51d73d9806a8c8d939dcc");
    expect(!handle.valid(), "a file substituted before handle acquisition must be rejected");
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

// #5680: removal of the staged file's last name must fail closed rather
// than publishing a newly created file at that name.
void test_publish_fails_closed_when_staged_path_is_swapped_before_publish() {
    const fs::path dir = make_scratch_dir("copperfin_staged_import_publish_swap_before_publish");
    const fs::path staged = dir / "staged.dbf";
    const fs::path destination = dir / "final.dbf";
    write_file(staged, "verified original bytes");

    auto handle = copperfin::vfp::open_staged_import_file_for_publish(staged);
    expect(handle.valid(), "opening the staged file before the swap should succeed");

    // Simulate a concurrent replacement of staged_path between staging and
    // commit: remove the original name and put different content at the
    // same name.
    std::error_code remove_error;
    fs::remove(staged, remove_error);
    write_file(staged, "swapped-in malicious bytes");

    const bool published =
        copperfin::vfp::publish_staged_import_file(handle, staged, destination);
#if defined(_WIN32)
    // Windows' FILE_SHARE_READ-only handle prevents the removal and the
    // attempted truncating write. The original must remain publishable.
    expect(static_cast<bool>(remove_error),
           "Windows must deny removal while the staged identity handle is open");
    expect(read_file(staged) == "verified original bytes",
           "Windows must deny the attempted replacement write");
    expect(published, "Windows should publish the protected original file");
    expect(read_file(destination) == "verified original bytes",
           "Windows must never publish attempted replacement bytes");
#elif defined(__APPLE__)
    if (published) {
        expect(read_file(destination) == "verified original bytes",
               "macOS must never clone replacement bytes from a rebound staged name");
    } else {
        expect(!fs::exists(destination),
               "an unlinked macOS source must fail closed without a final path");
    }
#else
    expect(!published, "publish must fail closed once staged_path's identity no longer matches "
                        "what was verified at staging time");

    std::error_code exists_error;
    expect(!fs::exists(destination, exists_error),
           "a fail-closed publish must not create any file at the destination");
#endif

    handle = copperfin::vfp::StagedImportFileHandle{};
    std::error_code ignored;
    fs::remove_all(dir, ignored);
}

#if defined(__APPLE__)
void test_clone_rejects_source_mutation_after_verification() {
    const fs::path dir = make_scratch_dir("copperfin_staged_import_clone_source_mutation");
    const fs::path staged = dir / "staged.dbf";
    const fs::path destination = dir / "final.dbf";
    write_file(staged, "verified original bytes");
    auto handle = copperfin::vfp::open_staged_import_file_for_publish(staged);
    expect(handle.valid(), "opening the original before mutation should succeed");
    write_file(staged, "mutated source contents");
    expect(!copperfin::vfp::publish_staged_import_file(handle, staged, destination),
           "macOS must reject a clone whose bytes differ from those verified");
    expect(!fs::exists(destination),
           "a rejected source mutation must not leave substituted final bytes");
    handle = copperfin::vfp::StagedImportFileHandle{};
    std::error_code ignored;
    fs::remove_all(dir, ignored);
}
#endif

#if defined(__linux__) || defined(__APPLE__)
void test_publish_uses_retained_descriptor_after_staged_name_is_rebound() {
    const fs::path dir = make_scratch_dir("copperfin_staged_import_retained_descriptor");
    const fs::path staged = dir / "staged.dbf";
    const fs::path retained_name = dir / "retained-original.dbf";
    const fs::path destination = dir / "final.dbf";
    write_file(staged, "verified original bytes");
    auto handle = copperfin::vfp::open_staged_import_file_for_publish(staged);
    expect(handle.valid(), "opening the staged file should pin its original identity");
    fs::rename(staged, retained_name);
    write_file(staged, "swapped-in malicious bytes");
    expect(copperfin::vfp::publish_staged_import_file(handle, staged, destination),
           "the retained descriptor should publish the original after its name is rebound");
    expect(read_file(destination) == "verified original bytes",
           "a rebound staged name must never substitute its bytes into the final path");
    handle = copperfin::vfp::StagedImportFileHandle{};
    std::error_code ignored;
    fs::remove_all(dir, ignored);
}
#endif

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

// #5681/#5682: the positive path -- every handle released, every staged
// file and the staging directory itself actually removed, reported as
// complete success.
void test_release_and_remove_staged_files_removes_everything_on_success() {
    const fs::path dir = make_scratch_dir("copperfin_staged_import_publish_release_success");
    const fs::path staging_dir = dir / "staging";
    fs::create_directories(staging_dir);
    const fs::path staged_a = staging_dir / "a.dbf";
    const fs::path staged_b = staging_dir / "b.dbf";
    write_file(staged_a, "a bytes");
    write_file(staged_b, "b bytes");

    std::vector<copperfin::vfp::StagedImportFileHandle> handles;
    handles.push_back(copperfin::vfp::open_staged_import_file_for_publish(staged_a));
    handles.push_back(copperfin::vfp::open_staged_import_file_for_publish(staged_b));
    expect(handles[0].valid() && handles[1].valid(), "opening both staged files should succeed");
    const std::vector<fs::path> staged_paths{staged_a, staged_b};

    const bool removed_everything =
        copperfin::vfp::release_and_remove_staged_files(handles, staged_paths, staging_dir);
    expect(removed_everything,
           "release_and_remove_staged_files should report complete success on the ordinary path");

    std::error_code exists_error;
    expect(!fs::exists(staging_dir, exists_error),
           "the staging directory should no longer exist after a successful cleanup");

    std::error_code ignored;
    fs::remove_all(dir, ignored);
}

// #5681/#5682: the core property these two issues are about -- when a
// staged file (and consequently the staging directory that holds it)
// cannot actually be removed, release_and_remove_staged_files() must
// report that honestly (false) rather than silently discarding the
// error the way the pre-fix code did. Denying the staging directory's own
// permissions blocks unlinking its children (POSIX: unlink() needs
// write+execute on the *parent*, not the child itself), reliably forcing
// a real removal failure; gracefully skipped when running as root, where
// permission bits do not restrict access, matching the same defensive
// pattern used elsewhere in this test suite (see #4354-style tests in
// test_vfp_assets.cpp) rather than asserting a platform-dependent outcome.
void test_release_and_remove_staged_files_reports_incomplete_cleanup() {
#if !defined(_WIN32)
    const fs::path dir = make_scratch_dir("copperfin_staged_import_publish_release_incomplete");
    const fs::path staging_dir = dir / "staging";
    fs::create_directories(staging_dir);
    const fs::path staged_file = staging_dir / "staged.dbf";
    write_file(staged_file, "staged bytes");

    std::vector<copperfin::vfp::StagedImportFileHandle> handles;
    handles.push_back(copperfin::vfp::open_staged_import_file_for_publish(staged_file));
    expect(handles.back().valid(), "opening the staged file should succeed");
    const std::vector<fs::path> staged_paths{staged_file};

    std::error_code permission_error;
    fs::permissions(staging_dir, fs::perms::none, fs::perm_options::replace, permission_error);
    expect(!permission_error, "removing the staging directory's own permissions should succeed");

    std::ofstream access_probe(staged_file, std::ios::app);
    const bool access_is_denied = !access_probe.good();
    access_probe.close();

    if (access_is_denied) {
        const bool removed_everything = copperfin::vfp::release_and_remove_staged_files(
            handles, staged_paths, staging_dir);
        expect(!removed_everything,
               "release_and_remove_staged_files must report incomplete cleanup when a staged file "
               "cannot actually be removed, rather than silently claiming success");
    }

    std::error_code restore_error;
    fs::permissions(staging_dir, fs::perms::owner_all, fs::perm_options::replace, restore_error);
    std::error_code ignored;
    fs::remove_all(dir, ignored);
#endif
}

}  // namespace

int main() {
    test_private_staging_directory_is_new_and_restricted();
#if !defined(_WIN32)
    test_private_staging_accepts_indirect_destination_parent();
#else
    test_private_staging_lease_blocks_parent_rename();
#endif
    test_open_succeeds_on_regular_file();
    test_open_rejects_replacement_before_handle_acquisition();
    test_open_rejects_missing_file();
    test_open_rejects_directory();
#if !defined(_WIN32)
    test_open_rejects_symlink();
#endif
    test_publish_fails_closed_when_staged_path_is_swapped_before_publish();
#if defined(__APPLE__)
    test_clone_rejects_source_mutation_after_verification();
#endif
#if defined(__linux__) || defined(__APPLE__)
    test_publish_uses_retained_descriptor_after_staged_name_is_rebound();
#endif
    test_publish_fails_if_destination_already_exists();
    test_publish_fails_on_invalid_handle();
    test_remove_published_file_when_identity_matches();
    test_remove_published_file_preserves_replaced_destination();
    test_remove_published_file_fails_closed_when_destination_missing();
    test_release_and_remove_staged_files_removes_everything_on_success();
    test_release_and_remove_staged_files_reports_incomplete_cleanup();

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
