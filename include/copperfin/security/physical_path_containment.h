// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include <cstdint>
#include <filesystem>
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

    bool operator==(const PhysicalPathIdentity&) const = default;
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

}  // namespace copperfin::security
