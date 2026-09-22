// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/vfp/staged_import_publish.h"
#include "copperfin/platform/private_directory.h"

#include "../platform/scoped_resource.h"

#include <algorithm>
#include <array>
#include <cerrno>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <system_error>
#include <utility>

#if !defined(_WIN32)
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#endif
#if defined(_WIN32)
#include <bcrypt.h>
#elif defined(__linux__)
#include <sys/random.h>
#elif defined(__APPLE__)
#include <stdlib.h>
#endif

namespace copperfin::vfp {

std::optional<std::filesystem::path> create_private_import_staging_directory(
    const std::filesystem::path& destination_parent) {
    namespace fs = std::filesystem;
    std::error_code absolute_error;
    const fs::path parent = fs::absolute(
        destination_parent.empty() ? fs::path(".") : destination_parent,
        absolute_error).lexically_normal();
    if (absolute_error || parent.empty()) {
        return std::nullopt;
    }
    static constexpr char hex[] = "0123456789abcdef";
#if !defined(_WIN32)
    struct stat destination_status{};
    if (::stat(parent.c_str(), &destination_status) != 0 ||
        !S_ISDIR(destination_status.st_mode)) {
        return std::nullopt;
    }
#endif
    for (fs::path candidate_parent = parent;; candidate_parent = candidate_parent.parent_path()) {
#if !defined(_WIN32)
        // A group-writable destination may still live below a trusted
        // same-volume ancestor (for example, /tmp). Put the private staging
        // directory there rather than weakening its parent trust contract.
        struct stat candidate_status{};
        if (::stat(candidate_parent.c_str(), &candidate_status) != 0 ||
            !S_ISDIR(candidate_status.st_mode) ||
            candidate_status.st_dev != destination_status.st_dev) {
            return std::nullopt;
        }
#endif
        bool try_parent = false;
        for (int attempt = 0; attempt < 8; ++attempt) {
            std::array<unsigned char, 16U> bytes{};
#if defined(_WIN32)
            if (::BCryptGenRandom(nullptr, bytes.data(), static_cast<ULONG>(bytes.size()),
                                  BCRYPT_USE_SYSTEM_PREFERRED_RNG) < 0) {
                return std::nullopt;
            }
#elif defined(__linux__)
            std::size_t offset = 0U;
            while (offset < bytes.size()) {
                const ssize_t count = ::getrandom(bytes.data() + offset, bytes.size() - offset, 0);
                if (count < 0 && errno == EINTR) {
                    continue;
                }
                if (count <= 0) {
                    return std::nullopt;
                }
                offset += static_cast<std::size_t>(count);
            }
#elif defined(__APPLE__)
            ::arc4random_buf(bytes.data(), bytes.size());
#else
            return std::nullopt;
#endif
            std::string leaf = ".copperfin-import-";
            leaf.reserve(leaf.size() + bytes.size() * 2U);
            for (const unsigned char byte : bytes) {
                leaf.push_back(hex[(byte >> 4U) & 0x0fU]);
                leaf.push_back(hex[byte & 0x0fU]);
            }
            const fs::path candidate = candidate_parent / leaf;
            const auto created = copperfin::platform::create_private_directory(candidate);
            if (created.ok) {
                return candidate;
            }
            if (created.failure == copperfin::platform::PrivateDirectoryFailure::already_exists) {
                continue;
            }
#if !defined(_WIN32)
            if (created.failure == copperfin::platform::PrivateDirectoryFailure::access_denied) {
                try_parent = true;
                break;
            }
#endif
            return std::nullopt;
        }
        if (!try_parent || candidate_parent == candidate_parent.root_path()) {
            return std::nullopt;
        }
    }
}

class StagedImportFileHandle::Impl {
public:
#if defined(_WIN32)
    copperfin::platform::ScopedHandle handle;
    std::uint64_t volume_serial = 0U;
    std::uint64_t file_index = 0U;
#else
    copperfin::platform::ScopedFd fd;
    dev_t device = 0;
    ino_t inode = 0;
#endif
};

StagedImportFileHandle::StagedImportFileHandle() noexcept = default;
StagedImportFileHandle::StagedImportFileHandle(std::unique_ptr<Impl> impl) noexcept
    : impl_(std::move(impl)) {}
StagedImportFileHandle::~StagedImportFileHandle() = default;
StagedImportFileHandle::StagedImportFileHandle(StagedImportFileHandle&&) noexcept = default;
StagedImportFileHandle& StagedImportFileHandle::operator=(StagedImportFileHandle&&) noexcept =
    default;

bool StagedImportFileHandle::valid() const noexcept {
    if (!impl_) {
        return false;
    }
#if defined(_WIN32)
    return impl_->handle.valid();
#else
    return impl_->fd.valid();
#endif
}

bool release_and_remove_staged_files(
    std::vector<StagedImportFileHandle>& handles,
    const std::vector<std::filesystem::path>& staged_paths,
    const std::filesystem::path& staging_dir) {
    bool all_removed = true;
    const std::size_t count = std::min(handles.size(), staged_paths.size());
    for (std::size_t index = 0U; index < count; ++index) {
        // #5941 PR review, and #5681/#5682: release before removing, for
        // exactly the reason documented on StagedImportFileHandle's own
        // callers -- a still-open handle (Windows: FILE_SHARE_READ only)
        // would otherwise make this very removal fail with a sharing
        // violation against the caller's own handle.
        handles[index] = StagedImportFileHandle{};
        std::error_code remove_error;
        std::filesystem::remove(staged_paths[index], remove_error);
        if (remove_error) {
            all_removed = false;
        }
    }
    std::error_code remove_all_error;
    std::filesystem::remove_all(staging_dir, remove_all_error);
    if (remove_all_error) {
        all_removed = false;
    }
    return all_removed;
}

#if defined(_WIN32)

namespace {

// Opens `path` for identity-bound publish/removal: denies other processes
// write and delete/rename sharing for as long as the returned handle stays
// open (FILE_SHARE_READ only), and rejects anything but an ordinary
// regular file. A single CreateFileW call with FILE_FLAG_OPEN_REPARSE_POINT
// opens a reparse point (symlink/junction/mount point) itself rather than
// transparently following it -- this flag has no effect when `path` is an
// ordinary file, so it is always safe to pass, and checking attributes on
// this same handle (rather than a separate probe-then-reopen pair) avoids
// an otherwise-unnecessary TOCTOU gap between the check and the handle this
// function actually returns. Matches the same reparse-point guard already
// used by runtime_pipeline_package_transaction.cpp for the same reason.
copperfin::platform::ScopedHandle open_exclusive_regular_file(
    const std::filesystem::path& path, DWORD desired_access) {
    copperfin::platform::ScopedHandle handle(::CreateFileW(
        path.c_str(), desired_access, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
        FILE_FLAG_OPEN_REPARSE_POINT, nullptr));
    if (!handle.valid()) {
        return {};
    }
    BY_HANDLE_FILE_INFORMATION info{};
    if (::GetFileInformationByHandle(handle.get(), &info) == 0 ||
        (info.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0U ||
        (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0U) {
        return {};
    }
    return handle;
}

bool file_identity_from_handle(
    HANDLE handle, std::uint64_t& volume_serial, std::uint64_t& file_index) {
    BY_HANDLE_FILE_INFORMATION info{};
    if (::GetFileInformationByHandle(handle, &info) == 0) {
        return false;
    }
    volume_serial = info.dwVolumeSerialNumber;
    file_index = (static_cast<std::uint64_t>(info.nFileIndexHigh) << 32U) | info.nFileIndexLow;
    return true;
}

}  // namespace

StagedImportFileHandle open_staged_import_file_for_publish(
    const std::filesystem::path& staged_path) {
    auto handle = open_exclusive_regular_file(staged_path, GENERIC_READ | DELETE);
    if (!handle.valid()) {
        return {};
    }
    auto impl = std::make_unique<StagedImportFileHandle::Impl>();
    if (!file_identity_from_handle(handle.get(), impl->volume_serial, impl->file_index)) {
        return {};
    }
    impl->handle = std::move(handle);
    return StagedImportFileHandle(std::move(impl));
}

bool publish_staged_import_file(
    const StagedImportFileHandle& handle,
    const std::filesystem::path& staged_path,
    const std::filesystem::path& destination) {
    if (!handle.valid()) {
        return false;
    }
    // handle's own share mode (FILE_SHARE_READ only) has denied every other
    // process write/delete/rename access to staged_path since it was
    // opened, so staged_path is guaranteed to still name the original
    // verified file here -- see this function's header doc comment.
    return ::CreateHardLinkW(destination.c_str(), staged_path.c_str(), nullptr) != 0;
}

bool remove_published_import_file_if_identity_matches(
    const StagedImportFileHandle& handle,
    const std::filesystem::path& published_path) {
    if (!handle.valid()) {
        return false;
    }
    const std::uint64_t expected_volume_serial = handle.impl_->volume_serial;
    const std::uint64_t expected_file_index = handle.impl_->file_index;
    // #5941 PR review (chatgpt-codex-connector, P1): Windows enforces
    // sharing across every hard-linked name to one file object, not per
    // name -- reopening published_path with DELETE access below would
    // otherwise always fail with a sharing violation against handle's own
    // still-open FILE_SHARE_READ-only handle to the very same file object
    // (staged_path and published_path are hard links to it after a
    // successful publish), making every rollback on Windows fail
    // unconditionally. The identity values captured above do not depend
    // on the handle staying open, so releasing it first is safe and lets
    // the reopen below succeed.
    handle.impl_->handle.reset();

    auto reopened = open_exclusive_regular_file(published_path, GENERIC_READ | DELETE);
    if (!reopened.valid()) {
        return false;
    }
    std::uint64_t volume_serial = 0U;
    std::uint64_t file_index = 0U;
    if (!file_identity_from_handle(reopened.get(), volume_serial, file_index)) {
        return false;
    }
    if (volume_serial != expected_volume_serial || file_index != expected_file_index) {
        return false;
    }
    FILE_DISPOSITION_INFO disposition{};
    disposition.DeleteFile = TRUE;
    return ::SetFileInformationByHandle(
        reopened.get(), FileDispositionInfo, &disposition, sizeof(disposition)) != 0;
}

#else  // POSIX

StagedImportFileHandle open_staged_import_file_for_publish(
    const std::filesystem::path& staged_path) {
    const int raw_fd = ::open(
        staged_path.c_str(), O_RDONLY | O_NOFOLLOW | O_CLOEXEC);
    if (raw_fd < 0) {
        return {};
    }
    copperfin::platform::ScopedFd fd(raw_fd);
    struct stat status{};
    if (::fstat(fd.get(), &status) != 0 || !S_ISREG(status.st_mode)) {
        return {};
    }
    auto impl = std::make_unique<StagedImportFileHandle::Impl>();
    impl->device = status.st_dev;
    impl->inode = status.st_ino;
    // Held open for the handle's entire lifetime (not just used to capture
    // identity here): as long as this descriptor stays open, the kernel
    // cannot free this specific inode number for reuse by an unrelated
    // file, even if staged_path is later unlinked -- closing this
    // prematurely would let a later re-check below (or in
    // remove_published_import_file_if_identity_matches()) be fooled by an
    // unrelated file that happens to land on the same, since-recycled
    // inode number.
    impl->fd = std::move(fd);
    return StagedImportFileHandle(std::move(impl));
}

bool publish_staged_import_file(
    const StagedImportFileHandle& handle,
    const std::filesystem::path& staged_path,
    const std::filesystem::path& destination) {
    if (!handle.valid()) {
        return false;
    }
#if defined(__linux__) && defined(AT_EMPTY_PATH)
    // Linux links directly from the descriptor retained since verification.
    // Rebinding staged_path cannot redirect this operation to substituted
    // bytes. If the original inode lost its last name, linkat fails closed.
    // Filesystems that do not support descriptor-based hard links likewise
    // fail closed; falling back to a pathname would restore the source race.
    (void)staged_path;
    return ::linkat(handle.impl_->fd.get(), "", AT_FDCWD,
                    destination.c_str(), AT_EMPTY_PATH) == 0;
#else
    // Other POSIX systems have no equivalent descriptor-based hard-link
    // primitive. The owner-private staging directory excludes other users;
    // re-open and compare identity immediately before the pathname link.
    // Same-authority mutation between these calls remains a documented
    // platform limitation and cannot be claimed as race-free.
    const int raw_fd = ::open(staged_path.c_str(), O_RDONLY | O_NOFOLLOW | O_CLOEXEC);
    if (raw_fd < 0) {
        return false;
    }
    copperfin::platform::ScopedFd recheck_fd(raw_fd);
    struct stat status{};
    if (::fstat(recheck_fd.get(), &status) != 0 || !S_ISREG(status.st_mode)) {
        return false;
    }
    if (status.st_dev != handle.impl_->device || status.st_ino != handle.impl_->inode) {
        return false;
    }
    recheck_fd.reset();
    return ::link(staged_path.c_str(), destination.c_str()) == 0;
#endif
}

bool remove_published_import_file_if_identity_matches(
    const StagedImportFileHandle& handle,
    const std::filesystem::path& published_path) {
    if (!handle.valid()) {
        return false;
    }
    const int raw_fd = ::open(
        published_path.c_str(), O_RDONLY | O_NOFOLLOW | O_CLOEXEC);
    if (raw_fd < 0) {
        return false;
    }
    copperfin::platform::ScopedFd fd(raw_fd);
    struct stat status{};
    if (::fstat(fd.get(), &status) != 0 || !S_ISREG(status.st_mode)) {
        return false;
    }
    if (status.st_dev != handle.impl_->device || status.st_ino != handle.impl_->inode) {
        return false;
    }
    // A residual TOCTOU gap remains between this fstat() and the unlink()
    // below: POSIX has no atomic "unlink this path only if it still names
    // this exact inode" primitive available to an unprivileged process (the
    // fd this function just opened does not pin the *directory entry*, only
    // the inode -- unlinking always re-resolves published_path by name).
    // This narrows the window from the entire commit/rollback transaction
    // (the prior behavior) to the two syscalls immediately below, which is
    // the best available mitigation in standard POSIX without additional
    // privilege or a private mount namespace.
    fd.reset();
    return ::unlink(published_path.c_str()) == 0;
}

#endif

}  // namespace copperfin::vfp
