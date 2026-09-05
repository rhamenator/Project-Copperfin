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

struct BoundedProcessResult;
struct PrivateWindowsBoundedProcessRequest;
struct PrivatePosixBoundedProcessRequest;

// Governing requirements: candidate RQ-CF-AGENT-026 through RQ-CF-AGENT-028.

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
// matches exactly. That final handle denies write, delete, and rename sharing;
// no-delete-share handles retain every renameable Windows directory below a
// handle-derived stable local-volume device root through the image parent, and a final
// reopen through that stable path must still match the exact image identity.
// Destruction deletes the handle-owned object before releasing
// the directory chain. No path or native handle is exposed by this portability
// seam.
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

    [[nodiscard]] const std::filesystem::path*
    windows_launch_target() const noexcept;
    [[nodiscard]] const std::filesystem::path*
    windows_native_launch_target() const noexcept;
    // POSIX only. Returns the retained, read+execute-only (0500) descriptor,
    // or -1 if unavailable/invalid. On Linux this is already-unlinked; no
    // path is ever exposed for it, and callers must exec it directly via
    // fexecve(). On macOS see posix_exec_in_child() below -- the image
    // stays linked there instead, and this descriptor is used only for a
    // same-process identity check immediately before exec, never for
    // /dev/fd-based exec.
    [[nodiscard]] int posix_descriptor() const noexcept;
    // POSIX only. Must be called only in the freshly-forked child,
    // immediately before exec, with argv/environment already fully built
    // (NUL-terminated, execve()-style). On success this call never returns
    // (execve() replaces the process image); on failure it returns false
    // with errno set, for the caller to report and _exit() as usual. On
    // Linux this execs posix_descriptor() directly (fexecve()). On macOS
    // it instead execs by real, still-linked path -- entirely internally;
    // no native path is ever exposed through this header's surface --
    // because macOS's fdescfs (/dev/fd) only permits path lookups (open,
    // access, exec) from the same process that opened the underlying
    // descriptor directly; a forked child that merely inherited it via
    // fork() is denied with EACCES even though the descriptor itself is
    // perfectly valid (confirmed empirically: fstat() on the inherited
    // descriptor succeeds and reports the correct file, while access()/
    // execve() via its /dev/fd path both fail identically). Immediately
    // before exec it performs its own (device, inode) identity check,
    // via a fresh same-process open of the retained real path, against
    // posix_descriptor(); this narrows, but -- unlike Linux's exec-by-
    // descriptor -- cannot fully eliminate, the TOCTOU window between
    // that check and exec.
    [[nodiscard]] bool posix_exec_in_child(
        char* const argv[], char* const environment[]) const noexcept;

    std::unique_ptr<Impl> impl_;

    friend struct PrivateExecutableImageMaterializationResult;
    // Bridges run_posix()'s (bounded_process.cpp, anonymous namespace)
    // forked-child exec step to posix_exec_in_child() above, since a
    // function with internal linkage cannot itself be named as a friend
    // here. See bounded_process_private.h for its declaration.
    friend bool posix_private_exec_override(
        void*, char* const[], char* const[]) noexcept;
    friend PrivateExecutableImageMaterializationResult
    materialize_private_executable_image_in_verified_parent(
        const std::filesystem::path&,
        std::uint64_t,
        std::uint64_t,
        std::uint64_t,
        const std::filesystem::path&,
        std::span<const std::uint8_t>) noexcept;
    friend BoundedProcessResult run_bounded_windows_private_executable(
        const PrivateExecutableImage&,
        const PrivateWindowsBoundedProcessRequest&) noexcept;
    friend BoundedProcessResult run_bounded_posix_private_executable(
        const PrivateExecutableImage&,
        const PrivatePosixBoundedProcessRequest&) noexcept;
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
