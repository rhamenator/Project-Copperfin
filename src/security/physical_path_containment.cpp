// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/security/physical_path_containment.h"

#include "copperfin/platform/path.h"

#include <array>
#include <cerrno>
#include <cstdint>
#include <limits>
#include <optional>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace copperfin::security {
namespace {

PhysicalPathContainmentResult failed_result(
    const PhysicalPathContainmentFailure failure) {
    return {
        .allowed = false,
        .canonical_path = {},
        .identity = {},
        .failure = failure
    };
}

bool relative_path_is_contained(const std::filesystem::path& relative) {
    if (relative.empty() || relative.is_absolute()) {
        return false;
    }
    for (const auto& part : relative) {
        if (part == "..") {
            return false;
        }
    }
    return true;
}

std::optional<std::filesystem::path> contained_relative_path(
    const std::filesystem::path& path,
    const std::filesystem::path& root) {
#if defined(_WIN32)
    auto path_iterator = path.begin();
    for (auto root_iterator = root.begin(); root_iterator != root.end();
         ++root_iterator, ++path_iterator) {
        if (path_iterator == path.end()) {
            return std::nullopt;
        }
        if (!copperfin::platform::path_component_equal_for_platform(
                *path_iterator,
                *root_iterator)) {
            return std::nullopt;
        }
    }

    std::filesystem::path relative;
    for (; path_iterator != path.end(); ++path_iterator) {
        relative /= *path_iterator;
    }
    if (relative.empty()) {
        relative = ".";
    }
#else
    const std::filesystem::path relative = path.lexically_relative(root);
#endif
    return relative_path_is_contained(relative)
        ? std::optional<std::filesystem::path>(relative)
        : std::nullopt;
}

#if defined(_WIN32)
PhysicalPathIdentity identity_from_handle_information(
    const BY_HANDLE_FILE_INFORMATION& information) {
    return PhysicalPathIdentity{
        .storage_id = information.dwVolumeSerialNumber,
        .file_id =
            (static_cast<std::uint64_t>(information.nFileIndexHigh) << 32U) |
            information.nFileIndexLow,
        .file_size =
            (static_cast<std::uint64_t>(information.nFileSizeHigh) << 32U) |
            information.nFileSizeLow,
        .modified_ticks =
            (static_cast<std::uint64_t>(information.ftLastWriteTime.dwHighDateTime) << 32U) |
            information.ftLastWriteTime.dwLowDateTime,
        .link_count = information.nNumberOfLinks,
        .creation_ticks =
            (static_cast<std::uint64_t>(information.ftCreationTime.dwHighDateTime) << 32U) |
            information.ftCreationTime.dwLowDateTime
    };
}
#else
// descriptor is used only for a best-effort, descriptor-relative creation-time
// lookup on Linux (statx with AT_EMPTY_PATH) so that call is bound to the
// exact object already open on descriptor rather than re-resolving a path
// string -- consistent with the rest of this file no longer trusting a
// second path-based lookup for anything security-relevant.
PhysicalPathIdentity identity_from_descriptor(
    int descriptor, const struct stat& status) {
#if defined(__APPLE__)
    static_cast<void>(descriptor);
    const std::uint64_t modified_ticks =
        static_cast<std::uint64_t>(status.st_mtimespec.tv_sec) * 1'000'000'000ULL +
        static_cast<std::uint64_t>(status.st_mtimespec.tv_nsec);
    const std::uint64_t creation_ticks =
        static_cast<std::uint64_t>(status.st_birthtimespec.tv_sec) *
            1'000'000'000ULL +
        static_cast<std::uint64_t>(status.st_birthtimespec.tv_nsec);
#else
    const std::uint64_t modified_ticks =
        static_cast<std::uint64_t>(status.st_mtim.tv_sec) * 1'000'000'000ULL +
        static_cast<std::uint64_t>(status.st_mtim.tv_nsec);
    std::uint64_t creation_ticks = 0U;
#if defined(__linux__) && defined(STATX_BTIME)
    struct statx extended_status {};
    if (::statx(
            descriptor, "", AT_EMPTY_PATH, STATX_BTIME,
            &extended_status) == 0 &&
        (extended_status.stx_mask & STATX_BTIME) != 0U &&
        extended_status.stx_btime.tv_sec >= 0 &&
        extended_status.stx_btime.tv_nsec < 1'000'000'000U) {
        creation_ticks =
            static_cast<std::uint64_t>(extended_status.stx_btime.tv_sec) *
                1'000'000'000ULL +
            extended_status.stx_btime.tv_nsec;
    }
#endif
#endif
    return PhysicalPathIdentity{
        .storage_id = static_cast<std::uint64_t>(status.st_dev),
        .file_id = static_cast<std::uint64_t>(status.st_ino),
        .file_size = static_cast<std::uint64_t>(status.st_size),
        .modified_ticks = modified_ticks,
        .link_count = static_cast<std::uint64_t>(status.st_nlink),
        .creation_ticks = creation_ticks
    };
}
#endif

PhysicalFileSnapshotResult failed_snapshot(
    const PhysicalPathContainmentFailure failure) {
    return {
        .ok = false,
        .bytes = {},
        .containment = failed_result(failure),
        .failure = failure
    };
}

// Splits a relative path into single, slash-free components, skipping "."
// entries. relative_path_is_contained() has already rejected ".." and
// absolute paths before this is ever called.
std::vector<std::filesystem::path> relative_components(
    const std::filesystem::path& relative_path) {
    std::vector<std::filesystem::path> parts;
    for (const auto& part : relative_path) {
        if (part != ".") {
            parts.push_back(part);
        }
    }
    return parts;
}

#if defined(_WIN32)

class ScopedFileHandle {
public:
    ScopedFileHandle() = default;
    explicit ScopedFileHandle(HANDLE handle) noexcept : handle_(handle) {}
    ScopedFileHandle(const ScopedFileHandle&) = delete;
    ScopedFileHandle& operator=(const ScopedFileHandle&) = delete;
    ScopedFileHandle(ScopedFileHandle&& other) noexcept : handle_(other.release()) {}
    ScopedFileHandle& operator=(ScopedFileHandle&& other) noexcept {
        if (this != &other) {
            reset(other.release());
        }
        return *this;
    }
    ~ScopedFileHandle() { reset(INVALID_HANDLE_VALUE); }

    [[nodiscard]] HANDLE get() const noexcept { return handle_; }
    [[nodiscard]] bool valid() const noexcept { return handle_ != INVALID_HANDLE_VALUE; }
    HANDLE release() noexcept {
        const HANDLE value = handle_;
        handle_ = INVALID_HANDLE_VALUE;
        return value;
    }
    void reset(HANDLE handle) noexcept {
        if (handle_ != INVALID_HANDLE_VALUE) {
            ::CloseHandle(handle_);
        }
        handle_ = handle;
    }

private:
    HANDLE handle_ = INVALID_HANDLE_VALUE;
};

struct WalkedHandle {
    ScopedFileHandle handle;
    PhysicalPathContainmentFailure failure = PhysicalPathContainmentFailure::none;
};

// Opens root/relative_path one component at a time, rejecting a reparse
// point or a cross-volume component at the exact moment each is opened
// rather than in a separate pass. This does not give the same kernel-level
// atomicity as the POSIX openat()/O_NOFOLLOW chain below -- Win32 has no
// "open this name relative to an already-open parent handle" primitive
// without native NT APIs -- but it removes the second, wholesale
// std::filesystem::canonical() re-walk of the same untrusted relative path
// that the original implementation performed after this check, which was
// the actual exploitable gap: a component swapped after this walk starts
// is caught by the *next* component's own reparse-point check on the
// then-current filesystem state, rather than silently surviving into an
// entirely separate, later resolution pass.
WalkedHandle walk_contained_path(
    const std::filesystem::path& root,
    const std::filesystem::path& relative_path) {
    ScopedFileHandle current(::CreateFileW(
        root.c_str(),
        FILE_READ_ATTRIBUTES,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        nullptr,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT,
        nullptr));
    if (!current.valid()) {
        return {ScopedFileHandle(), PhysicalPathContainmentFailure::root_unavailable};
    }
    BY_HANDLE_FILE_INFORMATION root_information{};
    if (::GetFileInformationByHandle(current.get(), &root_information) == 0 ||
        (root_information.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0U) {
        return {ScopedFileHandle(), PhysicalPathContainmentFailure::root_unavailable};
    }

    std::filesystem::path accumulated = root;
    for (const auto& part : relative_components(relative_path)) {
        accumulated /= part;
        ScopedFileHandle next(::CreateFileW(
            accumulated.c_str(),
            FILE_READ_ATTRIBUTES,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            nullptr,
            OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT,
            nullptr));
        if (!next.valid()) {
            return {ScopedFileHandle(), PhysicalPathContainmentFailure::path_unavailable};
        }
        BY_HANDLE_FILE_INFORMATION information{};
        if (::GetFileInformationByHandle(next.get(), &information) == 0) {
            return {ScopedFileHandle(), PhysicalPathContainmentFailure::path_unavailable};
        }
        if ((information.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0U) {
            return {ScopedFileHandle(), PhysicalPathContainmentFailure::indirect_component};
        }
        if (information.dwVolumeSerialNumber != root_information.dwVolumeSerialNumber) {
            return {ScopedFileHandle(), PhysicalPathContainmentFailure::cross_device_component};
        }
        current = std::move(next);
    }
    return {std::move(current), PhysicalPathContainmentFailure::none};
}

#else  // POSIX

class ScopedDescriptor {
public:
    ScopedDescriptor() = default;
    explicit ScopedDescriptor(int descriptor) noexcept : descriptor_(descriptor) {}
    ScopedDescriptor(const ScopedDescriptor&) = delete;
    ScopedDescriptor& operator=(const ScopedDescriptor&) = delete;
    ScopedDescriptor(ScopedDescriptor&& other) noexcept : descriptor_(other.release()) {}
    ScopedDescriptor& operator=(ScopedDescriptor&& other) noexcept {
        if (this != &other) {
            reset(other.release());
        }
        return *this;
    }
    ~ScopedDescriptor() { reset(-1); }

    [[nodiscard]] int get() const noexcept { return descriptor_; }
    [[nodiscard]] bool valid() const noexcept { return descriptor_ >= 0; }
    int release() noexcept {
        const int value = descriptor_;
        descriptor_ = -1;
        return value;
    }
    void reset(int descriptor) noexcept {
        if (descriptor_ >= 0) {
            ::close(descriptor_);
        }
        descriptor_ = descriptor;
    }

private:
    int descriptor_ = -1;
};

struct WalkedDescriptor {
    ScopedDescriptor descriptor;
    PhysicalPathContainmentFailure failure = PhysicalPathContainmentFailure::none;
};

// Opens root/relative_path one component at a time via openat() with
// O_NOFOLLOW, carrying the previous component's descriptor forward as the
// directory relative to which the next component is opened. Rejecting a
// symlink and opening the next component happen in the same syscall for
// every step, so there is no window between "this component was verified
// non-indirect" and "this is what gets opened" for an attacker to exploit
// by swapping a component after it was checked but before it was used --
// unlike a design that walks the path once to check it, then resolves it
// again separately (e.g. via a second std::filesystem::canonical() call)
// to actually open it.
WalkedDescriptor walk_contained_path(
    const std::filesystem::path& root,
    const std::filesystem::path& relative_path) {
    ScopedDescriptor current(::open(root.c_str(), O_RDONLY | O_CLOEXEC));
    if (!current.valid()) {
        return {ScopedDescriptor(), PhysicalPathContainmentFailure::root_unavailable};
    }
    struct stat root_status{};
    if (::fstat(current.get(), &root_status) != 0) {
        return {ScopedDescriptor(), PhysicalPathContainmentFailure::root_unavailable};
    }

    const auto parts = relative_components(relative_path);
    for (std::size_t index = 0U; index < parts.size(); ++index) {
        const bool is_last = (index + 1U == parts.size());
        // Intermediate components must be directories; O_DIRECTORY rejects a
        // regular file (or anything else) at that step explicitly rather
        // than relying on a later openat() to fail with ENOTDIR. The final
        // component is left unconstrained: callers use this for both file
        // targets and directory roots.
        const int flags = O_RDONLY | O_CLOEXEC | O_NOFOLLOW | (is_last ? 0 : O_DIRECTORY);
        const int opened = ::openat(current.get(), parts[index].c_str(), flags);
        if (opened < 0) {
            // O_NOFOLLOW alone makes a symlink component fail with ELOOP.
            // Combined with O_DIRECTORY (non-last components), the kernel
            // instead reports ENOTDIR for a symlink, since it cannot confirm
            // the eventual target is a directory without following it --
            // verified empirically, not merely inferred from documentation.
            // Either way, this component is not a plain, directly-usable
            // directory/file; preserve that as indirect_component rather
            // than folding it into a generic "unavailable".
            return {
                ScopedDescriptor(),
                (errno == ELOOP || errno == ENOTDIR)
                    ? PhysicalPathContainmentFailure::indirect_component
                    : PhysicalPathContainmentFailure::path_unavailable
            };
        }
        ScopedDescriptor next(opened);
        struct stat next_status{};
        if (::fstat(next.get(), &next_status) != 0) {
            return {ScopedDescriptor(), PhysicalPathContainmentFailure::path_unavailable};
        }
        if (next_status.st_dev != root_status.st_dev) {
            return {ScopedDescriptor(), PhysicalPathContainmentFailure::cross_device_component};
        }
        current = std::move(next);
    }
    return {std::move(current), PhysicalPathContainmentFailure::none};
}

// Reconstructs an absolute path string for the exact object walk_contained_path
// already verified and opened, by reading back the kernel's own record of
// that descriptor's path rather than recomputing it independently (which
// could disagree with what was actually opened). This is used only for the
// path string callers retain for display/further lookups; the trust
// decision itself is bound to the descriptor's fstat() identity, not to
// this string.
std::optional<std::filesystem::path> path_of_descriptor(int descriptor) {
#if defined(__APPLE__)
    std::array<char, PATH_MAX> buffer{};
    if (::fcntl(descriptor, F_GETPATH, buffer.data()) != 0) {
        return std::nullopt;
    }
    return std::filesystem::path(buffer.data());
#else
    std::error_code error;
    auto resolved = std::filesystem::read_symlink(
        "/proc/self/fd/" + std::to_string(descriptor), error);
    if (error) {
        return std::nullopt;
    }
    return resolved;
#endif
}

#endif  // POSIX

}  // namespace

PhysicalPathContainmentResult inspect_physical_path_containment(
    const std::filesystem::path& path,
    const std::filesystem::path& root) {
    std::error_code filesystem_error;
    const std::filesystem::path absolute_root =
        std::filesystem::absolute(root, filesystem_error).lexically_normal();
    if (filesystem_error) {
        return failed_result(PhysicalPathContainmentFailure::root_unavailable);
    }
    const std::filesystem::path absolute_path =
        std::filesystem::absolute(path, filesystem_error).lexically_normal();
    if (filesystem_error) {
        return failed_result(PhysicalPathContainmentFailure::path_unavailable);
    }
    const std::filesystem::path canonical_root =
        std::filesystem::canonical(absolute_root, filesystem_error);
    if (filesystem_error) {
        return failed_result(PhysicalPathContainmentFailure::root_unavailable);
    }

    std::filesystem::path component_root = absolute_root;
    std::optional<std::filesystem::path> lexical_relative =
        contained_relative_path(absolute_path, absolute_root);
    if (!lexical_relative.has_value()) {
        lexical_relative = contained_relative_path(absolute_path, canonical_root);
        component_root = canonical_root;
    }
    if (!lexical_relative.has_value()) {
        std::filesystem::path candidate_root = absolute_path;
        while (!candidate_root.empty()) {
            std::error_code equivalent_error;
            if (std::filesystem::equivalent(
                    candidate_root,
                    canonical_root,
                    equivalent_error) &&
                !equivalent_error) {
                lexical_relative = contained_relative_path(absolute_path, candidate_root);
                component_root = candidate_root;
                break;
            }

            const std::filesystem::path parent = candidate_root.parent_path();
            if (parent.empty() || parent == candidate_root) {
                break;
            }
            candidate_root = parent;
        }
    }
    if (!lexical_relative.has_value()) {
        return failed_result(PhysicalPathContainmentFailure::outside_root);
    }

    auto walked = walk_contained_path(component_root, *lexical_relative);
    if (walked.failure != PhysicalPathContainmentFailure::none) {
        return failed_result(walked.failure);
    }

#if defined(_WIN32)
    BY_HANDLE_FILE_INFORMATION information{};
    if (::GetFileInformationByHandle(walked.handle.get(), &information) == 0) {
        return failed_result(PhysicalPathContainmentFailure::path_unavailable);
    }
    if ((information.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0U) {
        return failed_result(PhysicalPathContainmentFailure::indirect_component);
    }
    const PhysicalPathIdentity identity = identity_from_handle_information(information);
    const std::filesystem::path canonical_path = component_root / *lexical_relative;
#else
    struct stat status{};
    if (::fstat(walked.descriptor.get(), &status) != 0) {
        return failed_result(PhysicalPathContainmentFailure::path_unavailable);
    }
    const PhysicalPathIdentity identity =
        identity_from_descriptor(walked.descriptor.get(), status);
    const auto resolved_path = path_of_descriptor(walked.descriptor.get());
    if (!resolved_path.has_value()) {
        return failed_result(PhysicalPathContainmentFailure::path_unavailable);
    }
    const std::filesystem::path canonical_path = *resolved_path;
#endif

    if (!contained_relative_path(canonical_path, canonical_root).has_value()) {
        return failed_result(PhysicalPathContainmentFailure::outside_root);
    }

    return {
        .allowed = true,
        .canonical_path = canonical_path,
        .identity = identity,
        .failure = PhysicalPathContainmentFailure::none
    };
}

PhysicalFileSnapshotResult read_physically_contained_file_snapshot(
    const PhysicalPathContainmentResult& expected,
    const std::filesystem::path& root) {
    return read_physically_contained_file_snapshot(
        expected, root, (std::numeric_limits<std::uint64_t>::max)());
}

PhysicalFileSnapshotResult read_physically_contained_file_snapshot(
    const PhysicalPathContainmentResult& expected,
    const std::filesystem::path& root,
    const std::uint64_t maximum_bytes) {
    if (!expected.allowed || expected.canonical_path.empty()) {
        return failed_snapshot(PhysicalPathContainmentFailure::path_unavailable);
    }
    if (expected.identity.file_size > maximum_bytes) {
        return failed_snapshot(
            PhysicalPathContainmentFailure::size_limit_exceeded);
    }

    std::string bytes;
#if defined(_WIN32)
    const HANDLE handle = ::CreateFileW(
        expected.canonical_path.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OPEN_REPARSE_POINT,
        nullptr);
    if (handle == INVALID_HANDLE_VALUE) {
        return failed_snapshot(PhysicalPathContainmentFailure::path_unavailable);
    }

    BY_HANDLE_FILE_INFORMATION before_information{};
    if (::GetFileInformationByHandle(handle, &before_information) == 0 ||
        (before_information.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0U) {
        ::CloseHandle(handle);
        return failed_snapshot(PhysicalPathContainmentFailure::indirect_component);
    }
    if ((before_information.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0U) {
        ::CloseHandle(handle);
        return failed_snapshot(PhysicalPathContainmentFailure::not_regular_file);
    }
    const PhysicalPathIdentity before_identity =
        identity_from_handle_information(before_information);
    if (before_identity != expected.identity) {
        ::CloseHandle(handle);
        return failed_snapshot(PhysicalPathContainmentFailure::identity_changed);
    }

    std::array<char, 64U * 1024U> buffer{};
    for (;;) {
        DWORD bytes_read = 0U;
        if (::ReadFile(handle, buffer.data(), static_cast<DWORD>(buffer.size()), &bytes_read, nullptr) == 0) {
            ::CloseHandle(handle);
            return failed_snapshot(PhysicalPathContainmentFailure::read_failed);
        }
        if (bytes_read == 0U) {
            break;
        }
        if (bytes_read > maximum_bytes ||
            bytes.size() > maximum_bytes - bytes_read) {
            ::CloseHandle(handle);
            return failed_snapshot(
                PhysicalPathContainmentFailure::size_limit_exceeded);
        }
        bytes.append(buffer.data(), bytes_read);
    }

    BY_HANDLE_FILE_INFORMATION after_information{};
    const bool after_read = ::GetFileInformationByHandle(handle, &after_information) != 0;
    ::CloseHandle(handle);
    if (!after_read) {
        return failed_snapshot(PhysicalPathContainmentFailure::read_failed);
    }
    const PhysicalPathIdentity after_identity =
        identity_from_handle_information(after_information);
#else
    const int descriptor = ::open(
        expected.canonical_path.c_str(),
        O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
    if (descriptor < 0) {
        return failed_snapshot(PhysicalPathContainmentFailure::path_unavailable);
    }

    struct stat before_status{};
    if (::fstat(descriptor, &before_status) != 0) {
        ::close(descriptor);
        return failed_snapshot(PhysicalPathContainmentFailure::read_failed);
    }
    if (!S_ISREG(before_status.st_mode)) {
        ::close(descriptor);
        return failed_snapshot(PhysicalPathContainmentFailure::not_regular_file);
    }
    const PhysicalPathIdentity before_identity =
        identity_from_descriptor(descriptor, before_status);
    if (before_identity != expected.identity) {
        ::close(descriptor);
        return failed_snapshot(PhysicalPathContainmentFailure::identity_changed);
    }

    std::array<char, 64U * 1024U> buffer{};
    for (;;) {
        const ssize_t bytes_read = ::read(descriptor, buffer.data(), buffer.size());
        if (bytes_read < 0) {
            if (errno == EINTR) {
                continue;
            }
            ::close(descriptor);
            return failed_snapshot(PhysicalPathContainmentFailure::read_failed);
        }
        if (bytes_read == 0) {
            break;
        }
        const auto byte_count = static_cast<std::uint64_t>(bytes_read);
        if (byte_count > maximum_bytes ||
            bytes.size() > maximum_bytes - byte_count) {
            ::close(descriptor);
            return failed_snapshot(
                PhysicalPathContainmentFailure::size_limit_exceeded);
        }
        bytes.append(buffer.data(), static_cast<std::size_t>(bytes_read));
    }

    struct stat after_status{};
    const bool after_read = ::fstat(descriptor, &after_status) == 0;
    if (!after_read) {
        ::close(descriptor);
        return failed_snapshot(PhysicalPathContainmentFailure::read_failed);
    }
    // Must run before closing descriptor: on Linux, identity_from_descriptor()
    // queries creation time via statx() against this same descriptor.
    const PhysicalPathIdentity after_identity =
        identity_from_descriptor(descriptor, after_status);
    ::close(descriptor);
#endif

    if (after_identity != expected.identity || bytes.size() != expected.identity.file_size) {
        return failed_snapshot(PhysicalPathContainmentFailure::identity_changed);
    }

    const auto after_containment = inspect_physical_path_containment(
        expected.canonical_path,
        root);
    if (!after_containment.allowed ||
        after_containment.canonical_path != expected.canonical_path ||
        after_containment.identity != expected.identity) {
        return failed_snapshot(PhysicalPathContainmentFailure::identity_changed);
    }

    return {
        .ok = true,
        .bytes = std::move(bytes),
        .containment = after_containment,
        .failure = PhysicalPathContainmentFailure::none
    };
}

}  // namespace copperfin::security
