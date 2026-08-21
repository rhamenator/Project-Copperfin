// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/migration/project_inventory.h"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace {
int failures = 0;

void expect(const bool condition, const std::string& message) {
    if (!condition) {
        ++failures;
        std::cerr << "FAIL: " << message << '\n';
    }
}

void write_file(const std::filesystem::path& path, const std::string& bytes) {
    std::ofstream output(path, std::ios::binary);
    output << bytes;
}

}  // namespace

int main() {
    namespace fs = std::filesystem;
    const fs::path root = fs::temp_directory_path() / "copperfin_project_inventory";
    std::error_code ignored;
    fs::remove_all(root, ignored);
    fs::create_directories(root / "nested");
    write_file(root / "zebra.PRG", "RETURN\n");
    write_file(root / "nested" / "invoice.FRX", "report");
    write_file(root / "nested" / "notes.txt", "text");
#if !defined(_WIN32)
    // Windows forbids double quotes in path components. The serializer's
    // escaping contract is covered below with an explicit in-memory entry.
    write_file(root / "quote\".dbf", "dbf");
#endif

    const fs::path outside = root.parent_path() / "copperfin_project_inventory_outside.prg";
    write_file(outside, "outside");
    std::error_code symlink_error;
    fs::create_symlink(outside, root / "nested" / "outside-link.prg", symlink_error);
    const fs::path outside_directory = root.parent_path() / "copperfin_project_inventory_outside_directory";
    fs::create_directories(outside_directory);
    write_file(outside_directory / "hidden.prg", "hidden");
    std::error_code directory_symlink_error;
    fs::create_directory_symlink(
        outside_directory, root / "nested" / "outside-directory", directory_symlink_error);

    const auto inventory = copperfin::migration::build_project_inventory(fs::absolute(root));
    expect(inventory.complete && inventory.diagnostic_code == "migration.inventory.complete",
           "an accessible direct project root should produce a complete inventory");
    constexpr std::size_t expected_direct_files =
#if defined(_WIN32)
        3U;
#else
        4U;
#endif
    expect(inventory.entries.size() == expected_direct_files,
           "only direct regular files should be inventoried");
    if (inventory.entries.size() == expected_direct_files) {
        expect(inventory.entries[0].relative_path == "nested/invoice.FRX" &&
                   inventory.entries[0].asset_kind == "report",
               "entries should be path-sorted and classify report assets case-insensitively");
        expect(inventory.entries[1].relative_path == "nested/notes.txt" &&
                   inventory.entries[1].asset_kind == "other",
               "unknown extensions should remain explicit inventory entries");
#if !defined(_WIN32)
        expect(inventory.entries[2].relative_path == "quote\".dbf" &&
                   inventory.entries[2].asset_kind == "table",
               "inventory should retain project-relative names without parsing contents");
#endif
        expect(inventory.entries.back().relative_path == "zebra.PRG" &&
                   inventory.entries.back().asset_kind == "prg",
               "PRG classification should be case-insensitive");
    }
    if (!symlink_error) {
        expect(std::find(inventory.skipped_symlinks.begin(), inventory.skipped_symlinks.end(),
                         "nested/outside-link.prg") != inventory.skipped_symlinks.end(),
               "a file symlink must be reported rather than inventoried");
    }
    if (!directory_symlink_error) {
        expect(std::find(inventory.skipped_symlinks.begin(), inventory.skipped_symlinks.end(),
                         "nested/outside-directory") != inventory.skipped_symlinks.end() &&
                   std::none_of(inventory.entries.begin(), inventory.entries.end(), [](const auto& entry) {
                       return entry.relative_path == "nested/outside-directory/hidden.prg";
                   }),
               "a directory symlink must be reported but never traversed or inventoried");
    }

    const std::string serialized = copperfin::migration::serialize_project_inventory_json(inventory);
    expect(serialized.find("\"schemaVersion\": 1") != std::string::npos &&
               serialized.find(fs::absolute(root).generic_string()) == std::string::npos,
           "inventory JSON should be versioned, deterministic, and omit the absolute root");
    copperfin::migration::ProjectInventoryResult escaping_probe;
    escaping_probe.complete = true;
    escaping_probe.diagnostic_code = "migration.inventory.complete";
    escaping_probe.entries.push_back({"quote\".dbf", "table", 3U});
    const std::string escaped = copperfin::migration::serialize_project_inventory_json(escaping_probe);
    expect(escaped.find("\"quote\\\".dbf\"") != std::string::npos,
           "inventory JSON must escape portable entry names independently of host filename rules");

    const auto relative = copperfin::migration::build_project_inventory("relative");
    expect(!relative.complete && relative.entries.empty() &&
               relative.diagnostic_code == "migration.inventory.invalid_root",
           "relative project roots must fail before filesystem traversal");

#if !defined(_WIN32) && !defined(__APPLE__)
    // APFS normalizes the fixture's raw invalid byte before std::filesystem
    // exposes the directory entry. Linux preserves it, so retain the
    // fail-closed regression there rather than asserting an invalid-name
    // precondition that macOS cannot provide.
    const fs::path non_utf8_root = fs::temp_directory_path() / "copperfin_project_inventory_non_utf8";
    fs::remove_all(non_utf8_root, ignored);
    fs::create_directories(non_utf8_root);
    write_file(non_utf8_root / fs::path("invalid-\xff.prg"), "opaque");
    const auto non_utf8 = copperfin::migration::build_project_inventory(fs::absolute(non_utf8_root));
    expect(!non_utf8.complete && non_utf8.entries.empty() &&
               non_utf8.diagnostic_code == "migration.inventory.scan_incomplete",
           "a non-UTF-8 POSIX name must fail closed rather than produce invalid JSON");
    fs::remove_all(non_utf8_root, ignored);
#endif

    fs::remove_all(root, ignored);
    fs::remove(outside, ignored);
    fs::remove_all(outside_directory, ignored);
    if (failures != 0) {
        return EXIT_FAILURE;
    }
    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
