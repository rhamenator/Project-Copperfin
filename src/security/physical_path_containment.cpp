// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/security/physical_path_containment.h"

#include "copperfin/platform/path.h"
#include "../platform/scoped_resource.h"

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

using copperfin::platform::ScopedHandle;

struct WalkedHandle {
    ScopedHandle handle;
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
// open_final_component_for_read widens only the last-opened component's
// access rights from FILE_READ_ATTRIBUTES to FILE_GENERIC_READ (a superset
// that adds FILE_READ_DATA), so a caller that intends to read the verified
// object's content via the handle this returns can do so without a second,
// separately-resolved open. Every non-final component along the walk still
// opens with FILE_READ_ATTRIBUTES only, matching the existing check-only
// behavior exactly -- widening those too would be unnecessary and would
// change the access this walk requests against every intermediate directory
// for callers that never read anything.
WalkedHandle walk_contained_path(
    const std::filesystem::path& root,
    const std::filesystem::path& relative_path,
    bool open_final_component_for_read = false) {
    const auto parts = relative_components(relative_path);
    // If relative_path resolves to the root itself (parts is empty), the
    // root's own handle is what a read-intending caller will read from.
    const DWORD root_access =
        (parts.empty() && open_final_component_for_read)
        ? FILE_GENERIC_READ
        : FILE_READ_ATTRIBUTES;
    ScopedHandle current(::CreateFileW(
        root.c_str(),
        root_access,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        nullptr,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT,
        nullptr));
    if (!current.valid()) {
        return {ScopedHandle(), PhysicalPathContainmentFailure::root_unavailable};
    }
    // The root itself is not rejected for being a reparse point: callers may
    // legitimately pass an already-resolved alias (e.g. a package alias
    // directory) as the trust boundary's root, and the original
    // implementation never rejected that case either. Reparse points are
    // rejected starting at the first path component *under* the root below,
    // which is what actually prevents an attacker-controlled component from
    // redirecting the walk outside the root.
    BY_HANDLE_FILE_INFORMATION root_information{};
    if (::GetFileInformationByHandle(current.get(), &root_information) == 0) {
        return {ScopedHandle(), PhysicalPathContainmentFailure::root_unavailable};
    }

    std::filesystem::path accumulated = root;
    for (std::size_t index = 0U; index < parts.size(); ++index) {
        accumulated /= parts[index];
        const bool is_last = (index + 1U == parts.size());
        const DWORD access =
            (is_last && open_final_component_for_read)
            ? FILE_GENERIC_READ
            : FILE_READ_ATTRIBUTES;
        ScopedHandle next(::CreateFileW(
            accumulated.c_str(),
            access,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            nullptr,
            OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT,
            nullptr));
        if (!next.valid()) {
            return {ScopedHandle(), PhysicalPathContainmentFailure::path_unavailable};
        }
        BY_HANDLE_FILE_INFORMATION information{};
        if (::GetFileInformationByHandle(next.get(), &information) == 0) {
            return {ScopedHandle(), PhysicalPathContainmentFailure::path_unavailable};
        }
        if ((information.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0U) {
            return {ScopedHandle(), PhysicalPathContainmentFailure::indirect_component};
        }
        // Intentional: brings Windows to parity with the POSIX walk's
        // pre-existing st_dev check below, which this Windows walk never had
        // before this rewrite. A component landing on a different volume --
        // e.g. a SUBST'd drive or another volume mounted under the root
        // without going through a reparse point -- is exactly the same class
        // of containment escape the POSIX side has always rejected; treating
        // it as a bug to revert rather than a correctness fix would leave
        // Windows strictly weaker than POSIX for no documented reason.
        if (information.dwVolumeSerialNumber != root_information.dwVolumeSerialNumber) {
            return {ScopedHandle(), PhysicalPathContainmentFailure::cross_device_component};
        }
        current = std::move(next);
    }
    return {std::move(current), PhysicalPathContainmentFailure::none};
}

// Reconstructs a path string for the exact object walk_contained_path
// already verified and opened, by reading back the kernel's own record of
// that handle's path rather than recomputing it independently. Mirrors
// path_of_descriptor() below for POSIX: without this, the Windows side
// re-derived canonical_path by string-concatenating component_root and
// lexical_relative, which is exactly the untrusted, pre-walk path the
// handle-based walk above exists to not trust -- making the subsequent
// contained_relative_path(canonical_path, canonical_root) check a
// tautology on Windows, since it recomputed rather than verified. See
// path_of_descriptor()'s comment above for what this string is (and is not)
// trusted for once a caller re-resolves it later, e.g. via
// read_physically_contained_file_snapshot().
std::optional<std::filesystem::path> path_of_handle(HANDLE handle) {
    std::array<wchar_t, 32768> buffer{};
    const DWORD length = ::GetFinalPathNameByHandleW(
        handle, buffer.data(), static_cast<DWORD>(buffer.size()),
        FILE_NAME_NORMALIZED | VOLUME_NAME_DOS);
    if (length == 0 || length >= buffer.size()) {
        return std::nullopt;
    }
    std::wstring resolved(buffer.data(), length);

    // VOLUME_NAME_DOS still returns the extended-length \\?\ namespace for
    // ordinary local/UNC paths on current Windows. canonical_root (produced
    // by std::filesystem::canonical()) and every caller of this API use the
    // ordinary drive-letter/UNC spelling, so leaving the prefix on would
    // make every containment comparison against canonical_root fail on its
    // very first component -- rejecting every legitimately contained path.
    // Mirrors the identical stripping already done for the same API call in
    // resolve_windows_host_spelling() (runtime_pipeline_file_io_and_classification.cpp).
    // A residual device-namespace path (not drive-letter or UNC) is treated
    // as unavailable rather than returned with a namespace canonical_root
    // could never match, per that same precedent.
    constexpr std::wstring_view extended_prefix = L"\\\\?\\";
    constexpr std::wstring_view extended_unc_prefix = L"\\\\?\\UNC\\";
    if (resolved.starts_with(extended_unc_prefix)) {
        resolved = L"\\\\" + resolved.substr(extended_unc_prefix.size());
    } else if (resolved.starts_with(extended_prefix) &&
               resolved.size() >= extended_prefix.size() + 2U &&
               ((resolved[extended_prefix.size()] >= L'A' &&
                 resolved[extended_prefix.size()] <= L'Z') ||
                (resolved[extended_prefix.size()] >= L'a' &&
                 resolved[extended_prefix.size()] <= L'z')) &&
               resolved[extended_prefix.size() + 1U] == L':') {
        resolved.erase(0U, extended_prefix.size());
    } else if (resolved.starts_with(extended_prefix)) {
        return std::nullopt;
    }
    return std::filesystem::path(resolved);
}

#else  // POSIX

using copperfin::platform::ScopedFd;

struct WalkedDescriptor {
    ScopedFd descriptor;
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
//
// open_final_component_for_read is accepted for signature parity with the
// Windows overload (both are called uniformly from inspect_and_walk()) but
// is otherwise unused here: O_RDONLY, already requested for every
// component below, already grants read access to file content, unlike
// Windows' FILE_READ_ATTRIBUTES, which does not.
WalkedDescriptor walk_contained_path(
    const std::filesystem::path& root,
    const std::filesystem::path& relative_path,
    [[maybe_unused]] bool open_final_component_for_read = false) {
    ScopedFd current(::open(root.c_str(), O_RDONLY | O_CLOEXEC));
    if (!current.valid()) {
        return {ScopedFd(), PhysicalPathContainmentFailure::root_unavailable};
    }
    struct stat root_status{};
    if (::fstat(current.get(), &root_status) != 0) {
        return {ScopedFd(), PhysicalPathContainmentFailure::root_unavailable};
    }

    const auto parts = relative_components(relative_path);
    for (std::size_t index = 0U; index < parts.size(); ++index) {
        const bool is_last = (index + 1U == parts.size());
        // Intermediate components must be directories; O_DIRECTORY rejects a
        // regular file (or anything else) at that step explicitly rather
        // than relying on a later openat() to fail with ENOTDIR. The final
        // component is left unconstrained: callers use this for both file
        // targets and directory roots. O_NONBLOCK ensures opening a FIFO
        // planted at an untrusted path returns immediately instead of
        // blocking the calling thread indefinitely waiting for a writer;
        // it has no effect on regular files or directories. Callers that
        // need a blocking read reopen the already-verified canonical path
        // through their own type-appropriate call.
        const int flags = O_RDONLY | O_CLOEXEC | O_NOFOLLOW | O_NONBLOCK |
            (is_last ? 0 : O_DIRECTORY);
        const int opened = ::openat(current.get(), parts[index].c_str(), flags);
        if (opened < 0) {
            // O_NOFOLLOW alone makes a symlink component fail with ELOOP.
            // Combined with O_DIRECTORY (non-last components), the kernel
            // instead reports ENOTDIR for a symlink, since it cannot confirm
            // the eventual target is a directory without following it --
            // verified empirically, not merely inferred from documentation.
            // ENOTDIR can also mean a genuine non-directory (e.g. a plain
            // file) is blocking the path with no symlink involved; an
            // AT_SYMLINK_NOFOLLOW-qualified fstatat() on the same
            // already-open parent disambiguates the two purely for
            // diagnostic accuracy -- it does not affect the fail-closed
            // outcome, which is identical either way, and reading (not
            // opening) that component's own metadata cannot itself hang.
            PhysicalPathContainmentFailure failure =
                PhysicalPathContainmentFailure::path_unavailable;
            if (errno == ELOOP) {
                failure = PhysicalPathContainmentFailure::indirect_component;
            } else if (errno == ENOTDIR) {
                struct stat blocked_status{};
                failure = (::fstatat(
                                current.get(), parts[index].c_str(),
                                &blocked_status, AT_SYMLINK_NOFOLLOW) == 0 &&
                            S_ISLNK(blocked_status.st_mode))
                    ? PhysicalPathContainmentFailure::indirect_component
                    : PhysicalPathContainmentFailure::path_unavailable;
            }
            return {ScopedFd(), failure};
        }
        ScopedFd next(opened);
        struct stat next_status{};
        if (::fstat(next.get(), &next_status) != 0) {
            return {ScopedFd(), PhysicalPathContainmentFailure::path_unavailable};
        }
        if (next_status.st_dev != root_status.st_dev) {
            return {ScopedFd(), PhysicalPathContainmentFailure::cross_device_component};
        }
        current = std::move(next);
    }
    return {std::move(current), PhysicalPathContainmentFailure::none};
}

// Reconstructs an absolute path string for the exact object walk_contained_path
// already verified and opened, by reading back the kernel's own record of
// that descriptor's path rather than recomputing it independently (which
// could disagree with what was actually opened). Within this function, this
// is what makes canonical_path trustworthy rather than tautological. It does
// NOT make every later use of that string equally race-free: this same
// string is what read_physically_contained_file_snapshot() reopens by path
// (not by descriptor) to actually read file bytes, so that function's own
// before/after PhysicalPathIdentity comparison -- not any property of this
// string itself -- is what re-establishes the binding for that reopen. See
// issue #5409 for the residual, narrow TOCTOU window that architecture still
// has and the plan to close it by carrying the descriptor/handle through
// instead.
// Linux path readback requires /proc/self/fd to be mounted; a minimal
// namespace/chroot without /proc will make this (and therefore
// inspect_physical_path_containment() as a whole) fail with
// path_unavailable rather than succeed. This is an accepted trade-off, not
// an oversight: the only portable alternative is re-resolving the untrusted
// path independently (e.g. via std::filesystem::canonical()), which is
// exactly the TOCTOU-vulnerable design this rewrite replaces -- there is no
// strictly-better fallback available without native OS-specific APIs.
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

// Single source of truth for both inspect_physical_path_containment() and
// inspect_and_open_physically_contained_path(): performs the exact same
// verified walk either way and additionally carries the native
// handle/descriptor forward on success, so a caller that wants to read
// immediately after checking (issue #5409) can do so without a second,
// separately-resolved open. inspect_physical_path_containment() discards
// the native member; inspect_and_open_physically_contained_path() keeps it.
struct InternalContainmentWalk {
    PhysicalPathContainmentResult result;
#if defined(_WIN32)
    ScopedHandle native;
#else
    ScopedFd native;
#endif
};

InternalContainmentWalk inspect_and_walk(
    const std::filesystem::path& path,
    const std::filesystem::path& root,
    const bool open_final_component_for_read) {
    std::error_code filesystem_error;
    const std::filesystem::path absolute_root =
        std::filesystem::absolute(root, filesystem_error).lexically_normal();
    if (filesystem_error) {
        return {failed_result(PhysicalPathContainmentFailure::root_unavailable), {}};
    }
    const std::filesystem::path absolute_path =
        std::filesystem::absolute(path, filesystem_error).lexically_normal();
    if (filesystem_error) {
        return {failed_result(PhysicalPathContainmentFailure::path_unavailable), {}};
    }
    const std::filesystem::path canonical_root =
        std::filesystem::canonical(absolute_root, filesystem_error);
    if (filesystem_error) {
        return {failed_result(PhysicalPathContainmentFailure::root_unavailable), {}};
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
        return {failed_result(PhysicalPathContainmentFailure::outside_root), {}};
    }

    auto walked = walk_contained_path(
        component_root, *lexical_relative, open_final_component_for_read);
    if (walked.failure != PhysicalPathContainmentFailure::none) {
        return {failed_result(walked.failure), {}};
    }

#if defined(_WIN32)
    BY_HANDLE_FILE_INFORMATION information{};
    if (::GetFileInformationByHandle(walked.handle.get(), &information) == 0) {
        return {failed_result(PhysicalPathContainmentFailure::path_unavailable), {}};
    }
    if ((information.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0U) {
        return {failed_result(PhysicalPathContainmentFailure::indirect_component), {}};
    }
    const PhysicalPathIdentity identity = identity_from_handle_information(information);
    const auto resolved_path = path_of_handle(walked.handle.get());
    if (!resolved_path.has_value()) {
        return {failed_result(PhysicalPathContainmentFailure::path_unavailable), {}};
    }
    const std::filesystem::path canonical_path = *resolved_path;
#else
    struct stat status{};
    if (::fstat(walked.descriptor.get(), &status) != 0) {
        return {failed_result(PhysicalPathContainmentFailure::path_unavailable), {}};
    }
    const PhysicalPathIdentity identity =
        identity_from_descriptor(walked.descriptor.get(), status);
    const auto resolved_path = path_of_descriptor(walked.descriptor.get());
    if (!resolved_path.has_value()) {
        return {failed_result(PhysicalPathContainmentFailure::path_unavailable), {}};
    }
    const std::filesystem::path canonical_path = *resolved_path;
#endif

    if (!contained_relative_path(canonical_path, canonical_root).has_value()) {
        return {failed_result(PhysicalPathContainmentFailure::outside_root), {}};
    }

    return {
        PhysicalPathContainmentResult{
            .allowed = true,
            .canonical_path = canonical_path,
            .identity = identity,
            .failure = PhysicalPathContainmentFailure::none},
#if defined(_WIN32)
        std::move(walked.handle)
#else
        std::move(walked.descriptor)
#endif
    };
}

}  // namespace

PhysicalPathContainmentResult inspect_physical_path_containment(
    const std::filesystem::path& path,
    const std::filesystem::path& root) {
    return inspect_and_walk(path, root, /*open_final_component_for_read=*/false).result;
}

class PhysicalPathContainmentHandle::Impl {
public:
    PhysicalPathContainmentResult result;
#if defined(_WIN32)
    ScopedHandle native;
#else
    ScopedFd native;
#endif
};

PhysicalPathContainmentHandle::PhysicalPathContainmentHandle() noexcept = default;
PhysicalPathContainmentHandle::~PhysicalPathContainmentHandle() = default;
PhysicalPathContainmentHandle::PhysicalPathContainmentHandle(
    PhysicalPathContainmentHandle&&) noexcept = default;
PhysicalPathContainmentHandle& PhysicalPathContainmentHandle::operator=(
    PhysicalPathContainmentHandle&&) noexcept = default;
PhysicalPathContainmentHandle::PhysicalPathContainmentHandle(
    std::unique_ptr<Impl> impl) noexcept
    : impl_(std::move(impl)) {}

const PhysicalPathContainmentResult& PhysicalPathContainmentHandle::result() const noexcept {
    static const PhysicalPathContainmentResult empty{};
    return impl_ != nullptr ? impl_->result : empty;
}

PhysicalPathContainmentHandle inspect_and_open_physically_contained_path(
    const std::filesystem::path& path,
    const std::filesystem::path& root) {
    auto walked = inspect_and_walk(path, root, /*open_final_component_for_read=*/true);
    auto impl = std::make_unique<PhysicalPathContainmentHandle::Impl>();
    impl->result = std::move(walked.result);
    impl->native = std::move(walked.native);
    return PhysicalPathContainmentHandle(std::move(impl));
}

PhysicalFileSnapshotResult read_physically_contained_file_snapshot_from_handle(
    const PhysicalPathContainmentHandle& handle) {
    return read_physically_contained_file_snapshot_from_handle(
        handle, (std::numeric_limits<std::uint64_t>::max)());
}

PhysicalFileSnapshotResult read_physically_contained_file_snapshot_from_handle(
    const PhysicalPathContainmentHandle& handle,
    const std::uint64_t maximum_bytes) {
    if (handle.impl_ == nullptr || !handle.impl_->result.allowed ||
        !handle.impl_->native.valid()) {
        return failed_snapshot(PhysicalPathContainmentFailure::path_unavailable);
    }
    const PhysicalPathContainmentResult& expected = handle.impl_->result;
    if (expected.identity.file_size > maximum_bytes) {
        return failed_snapshot(PhysicalPathContainmentFailure::size_limit_exceeded);
    }

    std::string bytes;
    PhysicalPathIdentity after_identity;
#if defined(_WIN32)
    const HANDLE native = handle.impl_->native.get();

    BY_HANDLE_FILE_INFORMATION before_information{};
    if (::GetFileInformationByHandle(native, &before_information) == 0) {
        return failed_snapshot(PhysicalPathContainmentFailure::read_failed);
    }
    if ((before_information.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0U) {
        return failed_snapshot(PhysicalPathContainmentFailure::indirect_component);
    }
    if ((before_information.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0U) {
        return failed_snapshot(PhysicalPathContainmentFailure::not_regular_file);
    }
    const PhysicalPathIdentity before_identity =
        identity_from_handle_information(before_information);
    if (!before_identity.content_equal(expected.identity)) {
        return failed_snapshot(PhysicalPathContainmentFailure::identity_changed);
    }
    // This function takes the handle by const reference rather than
    // consuming it, so nothing prevents either a second sequential call or
    // -- more seriously -- concurrent calls from separate threads on the
    // same handle. A single shared file-position cursor (ReadFile with a
    // nullptr OVERLAPPED, or ::read()) would make concurrent callers race
    // on that cursor: reads reads at this file's mutual offset are
    // observably corrupted (empirically reproduced with a concurrent-access
    // harness -- interleaved seeks/reads produced truncated or spuriously
    // identity_changed results). Reading at an explicit, per-call offset
    // (position-independent) instead makes concurrent calls on the same
    // handle safe by construction: each call tracks its own progress
    // locally and never mutates shared kernel-level position state.
    std::array<char, 64U * 1024U> buffer{};
    std::uint64_t offset = 0U;
    for (;;) {
        OVERLAPPED positioned{};
        positioned.Offset = static_cast<DWORD>(offset & 0xFFFFFFFFU);
        positioned.OffsetHigh = static_cast<DWORD>(offset >> 32U);
        DWORD bytes_read = 0U;
        if (::ReadFile(
                native, buffer.data(), static_cast<DWORD>(buffer.size()),
                &bytes_read, &positioned) == 0) {
            if (::GetLastError() == ERROR_HANDLE_EOF) {
                break;
            }
            return failed_snapshot(PhysicalPathContainmentFailure::read_failed);
        }
        if (bytes_read == 0U) {
            break;
        }
        if (bytes_read > maximum_bytes ||
            bytes.size() > maximum_bytes - bytes_read) {
            return failed_snapshot(
                PhysicalPathContainmentFailure::size_limit_exceeded);
        }
        bytes.append(buffer.data(), bytes_read);
        offset += bytes_read;
    }

    BY_HANDLE_FILE_INFORMATION after_information{};
    if (::GetFileInformationByHandle(native, &after_information) == 0) {
        return failed_snapshot(PhysicalPathContainmentFailure::read_failed);
    }
    after_identity = identity_from_handle_information(after_information);
#else
    const int native = handle.impl_->native.get();

    struct stat before_status{};
    if (::fstat(native, &before_status) != 0) {
        return failed_snapshot(PhysicalPathContainmentFailure::read_failed);
    }
    if (!S_ISREG(before_status.st_mode)) {
        return failed_snapshot(PhysicalPathContainmentFailure::not_regular_file);
    }
    const PhysicalPathIdentity before_identity =
        identity_from_descriptor(native, before_status);
    if (!before_identity.content_equal(expected.identity)) {
        return failed_snapshot(PhysicalPathContainmentFailure::identity_changed);
    }
    // This function takes the handle by const reference rather than
    // consuming it, so nothing prevents either a second sequential call or
    // -- more seriously -- concurrent calls from separate threads on the
    // same handle. A single shared file-position cursor (::read(), which
    // consumes and advances one process-wide-per-descriptor offset) would
    // make concurrent callers race on that cursor: reads at this file's
    // mutual offset are observably corrupted (empirically reproduced with a
    // concurrent-access harness -- interleaved seeks/reads produced
    // truncated or spuriously identity_changed results). Reading at an
    // explicit, per-call offset via ::pread() instead makes concurrent
    // calls on the same handle safe by construction: each call tracks its
    // own progress locally and never mutates the descriptor's shared
    // kernel-level position state.
    std::array<char, 64U * 1024U> buffer{};
    std::uint64_t offset = 0U;
    for (;;) {
        const ssize_t bytes_read = ::pread(
            native, buffer.data(), buffer.size(), static_cast<off_t>(offset));
        if (bytes_read < 0) {
            if (errno == EINTR) {
                continue;
            }
            return failed_snapshot(PhysicalPathContainmentFailure::read_failed);
        }
        if (bytes_read == 0) {
            break;
        }
        const auto byte_count = static_cast<std::uint64_t>(bytes_read);
        if (byte_count > maximum_bytes ||
            bytes.size() > maximum_bytes - byte_count) {
            return failed_snapshot(
                PhysicalPathContainmentFailure::size_limit_exceeded);
        }
        bytes.append(buffer.data(), static_cast<std::size_t>(bytes_read));
        offset += byte_count;
    }

    struct stat after_status{};
    if (::fstat(native, &after_status) != 0) {
        return failed_snapshot(PhysicalPathContainmentFailure::read_failed);
    }
    // Must run before any further use of native: on Linux,
    // identity_from_descriptor() queries creation time via statx() against
    // this same descriptor.
    after_identity = identity_from_descriptor(native, after_status);
#endif

    if (!after_identity.content_equal(expected.identity) ||
        bytes.size() != expected.identity.file_size) {
        return failed_snapshot(PhysicalPathContainmentFailure::identity_changed);
    }

    return {
        .ok = true,
        .bytes = std::move(bytes),
        .containment = expected,
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
    // O_NONBLOCK matches walk_contained_path()'s final-component open above:
    // a FIFO at this path must not hang the calling thread indefinitely
    // waiting for a writer. It has no effect on the regular file this path
    // is required to be (checked via S_ISREG immediately below); the
    // blocking, buffered ::read() loop further down still runs normally
    // once that check passes.
    const int descriptor = ::open(
        expected.canonical_path.c_str(),
        O_RDONLY | O_CLOEXEC | O_NOFOLLOW | O_NONBLOCK);
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
