// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <span>

namespace copperfin::platform {

// Governing requirements: candidate RQ-CF-AGENT-026 and RQ-CF-AGENT-027.

enum class PrivateExecutableImageFailure : std::uint32_t {
    none = 0U,
    invalid_request,
    parent_unavailable,
    parent_identity_changed,
    already_exists,
    access_denied,
    creation_failed,
    write_failed,
    verification_failed,
    launch_transition_failed,
    cleanup_failed
};

struct PrivateExecutableImageMaterializationResult;

// Move-only ownership of an exact executable byte image. POSIX implementations
// unlink the image before writing and retain only its descriptor. Windows
// replaces its write-capable creation handle with a read-only file-id identity
// anchor, then admits a linked-path read/delete handle only when its identity
// matches exactly. That final handle denies write, delete, and rename sharing
// and deletes the
// handle-owned object during destruction. No path or native handle is exposed
// by this portability seam.
class PrivateExecutableImage {
public:
    PrivateExecutableImage();
    ~PrivateExecutableImage();
    PrivateExecutableImage(PrivateExecutableImage&&) noexcept;
    PrivateExecutableImage& operator=(PrivateExecutableImage&&) noexcept;
    PrivateExecutableImage(const PrivateExecutableImage&) = delete;
    PrivateExecutableImage& operator=(const PrivateExecutableImage&) = delete;

    [[nodiscard]] bool valid() const noexcept;

private:
    class Impl;
    explicit PrivateExecutableImage(std::unique_ptr<Impl> impl) noexcept;

    [[nodiscard]] bool matches_bytes(
        std::span<const std::uint8_t> expected) const noexcept;

    std::unique_ptr<Impl> impl_;

    friend struct PrivateExecutableImageMaterializationResult;
    friend PrivateExecutableImageMaterializationResult
    materialize_private_executable_image_in_verified_parent(
        const std::filesystem::path&,
        std::uint64_t,
        std::uint64_t,
        std::uint64_t,
        const std::filesystem::path&,
        std::span<const std::uint8_t>) noexcept;
};

struct PrivateExecutableImageMaterializationResult {
    bool materialized = false;
    PrivateExecutableImageFailure failure =
        PrivateExecutableImageFailure::invalid_request;
    std::optional<PrivateExecutableImage> image;
};

// Creates exactly one absent direct file beneath an already-private parent
// whose storage/file/creation identity must match. The supplied bytes are
// written once and reread from the retained native object before success.
// Existing objects are never adopted, repaired, overwritten, or removed.
[[nodiscard]] PrivateExecutableImageMaterializationResult
materialize_private_executable_image_in_verified_parent(
    const std::filesystem::path& parent,
    std::uint64_t expected_parent_storage_id,
    std::uint64_t expected_parent_file_id,
    std::uint64_t expected_parent_creation_ticks,
    const std::filesystem::path& leaf,
    std::span<const std::uint8_t> bytes) noexcept;

}  // namespace copperfin::platform
