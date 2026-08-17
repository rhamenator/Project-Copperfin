// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace copperfin::migration {

// Governing requirement: RQ-CF-MIGRATION-001.
// This boundary inventories a caller-selected project tree. It reads metadata
// only; it rejects a symbolic-link root, never follows discovered symbolic
// links, and never reads, executes, or changes files.
struct ProjectInventoryEntry {
    std::string relative_path;
    std::string asset_kind;
    std::uintmax_t size_bytes = 0U;
};

struct ProjectInventoryResult {
    bool complete = false;
    std::vector<ProjectInventoryEntry> entries;
    std::vector<std::string> skipped_symlinks;
    std::string diagnostic_code;
};

[[nodiscard]] ProjectInventoryResult build_project_inventory(
    const std::filesystem::path& trusted_absolute_project_root);

// Produces the deterministic schema-version-1 inventory.json payload. The
// payload contains only project-relative names, asset classifications, sizes,
// and skipped symbolic-link names; it does not include the absolute root.
[[nodiscard]] std::string serialize_project_inventory_json(
    const ProjectInventoryResult& inventory);

}  // namespace copperfin::migration
