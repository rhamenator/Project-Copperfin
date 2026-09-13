// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/vfp/staged_import_publish.h"

#include "../platform/scoped_resource.h"

#include <cstdint>
#include <memory>
#include <string>
#include <utility>

#if !defined(_WIN32)
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace copperfin::vfp {

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
    auto reopened = open_exclusive_regular_file(published_path, GENERIC_READ | DELETE);
    if (!reopened.valid()) {
        return false;
    }
    std::uint64_t volume_serial = 0U;
    std::uint64_t file_index = 0U;
    if (!file_identity_from_handle(reopened.get(), volume_serial, file_index)) {
        return false;
    }
    if (volume_serial != handle.impl_->volume_serial || file_index != handle.impl_->file_index) {
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
    // Unlike a read, publication cannot be bound purely to the already-open
    // descriptor: POSIX has no primitive that hard-links "the object this
    // descriptor refers to" once its last directory entry has been
    // removed. (The tempting /proc/self/fd or /dev/fd + linkat(..,
    // AT_SYMLINK_FOLLOW) trick was tried and confirmed, empirically, not to
    // work for this case -- the kernel deliberately refuses to resurrect a
    // fully unlinked inode this way once its link count reaches zero,
    // unlike the AT_EMPTY_PATH form reserved for CAP_DAC_READ_SEARCH/
    // O_TMPFILE use.) Instead, re-open staged_path fresh (a second,
    // independent descriptor from the one held since staging) and compare
    // its identity to what was captured then; a mismatch means a
    // concurrent actor replaced staged_path (by unlink+recreate, rename-
    // over, or otherwise) since it was opened, in which case this fails
    // the publish closed rather than linking either the original (no
    // longer nameable under this path) or the substituted content. A
    // residual race remains between this check and the link() call below,
    // narrowed to two syscalls -- the same class of limitation documented
    // on remove_published_import_file_if_identity_matches() below, and
    // the best available mitigation in standard POSIX without additional
    // privilege.
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
