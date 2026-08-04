// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "runtime_pipeline_manifest_pair_io.h"

#include "copperfin/platform/path.h"

#include <algorithm>
#include <cerrno>
#include <cstddef>
#include <cstring>
#include <limits>
#include <string_view>
#include <vector>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/file.h>
#include <sys/stat.h>
#if defined(__APPLE__)
#include <stdio.h>
#endif
#if defined(__linux__)
#include <sys/syscall.h>
#endif
#include <unistd.h>
#endif

namespace copperfin::runtime::runtime_pipeline_detail {
namespace {

#if !defined(_WIN32)
struct FdBackedPath {
    bool matched = false;
    int descriptor = -1;
    std::string relative_path;
};

FdBackedPath parse_fd_backed_path(const std::filesystem::path& path) {
    const std::string value = copperfin::platform::path_to_utf8_string(path);
    constexpr std::string_view proc_prefix = "/proc/self/fd/";
    constexpr std::string_view dev_prefix = "/dev/fd/";
    const std::string_view prefix = value.rfind(proc_prefix, 0U) == 0U
        ? proc_prefix
        : value.rfind(dev_prefix, 0U) == 0U
            ? dev_prefix
            : std::string_view();
    if (prefix.empty()) {
        return {};
    }

    const std::size_t number_begin = prefix.size();
    const std::size_t separator = value.find('/', number_begin);
    const std::size_t number_end = separator == std::string::npos
        ? value.size()
        : separator;
    if (number_end == number_begin) {
        return {.matched = true, .descriptor = -1, .relative_path = {}};
    }

    unsigned long long descriptor_value = 0U;
    for (std::size_t index = number_begin; index < number_end; ++index) {
        const unsigned char character = static_cast<unsigned char>(value[index]);
        if (character < '0' || character > '9') {
            return {.matched = true, .descriptor = -1, .relative_path = {}};
        }
        const unsigned digit = character - '0';
        if (descriptor_value >
            (std::numeric_limits<unsigned long long>::max() - digit) / 10U) {
            return {.matched = true, .descriptor = -1, .relative_path = {}};
        }
        descriptor_value = descriptor_value * 10U + digit;
        if (descriptor_value > static_cast<unsigned long long>(std::numeric_limits<int>::max())) {
            return {.matched = true, .descriptor = -1, .relative_path = {}};
        }
    }

    return {
        .matched = true,
        .descriptor = static_cast<int>(descriptor_value),
        .relative_path = separator == std::string::npos
            ? std::string()
            : value.substr(separator + 1U)
    };
}

bool open_fd_backed_directory(
    const std::filesystem::path& path,
    int& descriptor) {
    const FdBackedPath parsed = parse_fd_backed_path(path);
    if (!parsed.matched || parsed.descriptor < 0) {
        return false;
    }

    int current = ::fcntl(parsed.descriptor, F_DUPFD_CLOEXEC, 0);
    if (current < 0) {
        return false;
    }
    if (!parsed.relative_path.empty()) {
        const std::filesystem::path relative_path(parsed.relative_path);
        for (const auto& component : relative_path) {
            const std::string name = copperfin::platform::path_to_utf8_string(component);
            if (name.empty() || name == ".") {
                continue;
            }
            if (name == "..") {
                (void)::close(current);
                return false;
            }

            const int child = ::openat(
                current,
                name.c_str(),
                O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
            (void)::close(current);
            if (child < 0) {
                return false;
            }
            current = child;
        }
    }

    struct stat information{};
    if (::fstat(current, &information) != 0 || !S_ISDIR(information.st_mode)) {
        (void)::close(current);
        return false;
    }
    descriptor = current;
    return true;
}
#endif

#if defined(_WIN32)
HANDLE as_handle(void* value) {
    return static_cast<HANDLE>(value);
}

bool same_file_identity(
    const BY_HANDLE_FILE_INFORMATION& left,
    const BY_HANDLE_FILE_INFORMATION& right) {
    return left.dwVolumeSerialNumber == right.dwVolumeSerialNumber &&
        left.nFileIndexHigh == right.nFileIndexHigh &&
        left.nFileIndexLow == right.nFileIndexLow &&
        left.nFileSizeHigh == right.nFileSizeHigh &&
        left.nFileSizeLow == right.nFileSizeLow &&
        left.ftLastWriteTime.dwHighDateTime == right.ftLastWriteTime.dwHighDateTime &&
        left.ftLastWriteTime.dwLowDateTime == right.ftLastWriteTime.dwLowDateTime;
}
#else
bool same_file_identity(const struct stat& left, const struct stat& right) {
#if defined(__APPLE__)
    const bool same_time =
        left.st_mtimespec.tv_sec == right.st_mtimespec.tv_sec &&
        left.st_mtimespec.tv_nsec == right.st_mtimespec.tv_nsec;
#else
    const bool same_time =
        left.st_mtim.tv_sec == right.st_mtim.tv_sec &&
        left.st_mtim.tv_nsec == right.st_mtim.tv_nsec;
#endif
    return left.st_dev == right.st_dev &&
        left.st_ino == right.st_ino &&
        left.st_size == right.st_size && same_time;
}
#endif

}  // namespace

ManifestPairDirectory::~ManifestPairDirectory() {
#if defined(_WIN32)
    if (mutex_owned_) {
        (void)::ReleaseMutex(as_handle(mutex_handle_));
    }
    if (mutex_handle_ != nullptr) {
        (void)::CloseHandle(as_handle(mutex_handle_));
    }
    if (directory_handle_ != nullptr) {
        (void)::CloseHandle(as_handle(directory_handle_));
    }
#else
    if (descriptor_ >= 0) {
        (void)::flock(descriptor_, LOCK_UN);
        (void)::close(descriptor_);
    }
#endif
}

bool ManifestPairDirectory::acquire(
    const std::filesystem::path& root,
    const std::string& transaction_identity) {
    root_ = root;
#if defined(_WIN32)
    const std::string mutex_name =
        "Local\\CopperfinManifestPair-" + transaction_identity;
    const std::wstring wide_mutex_name(mutex_name.begin(), mutex_name.end());
    mutex_handle_ = ::CreateMutexW(nullptr, FALSE, wide_mutex_name.c_str());
    if (mutex_handle_ == nullptr) {
        acquire_failure_ = ManifestPairDirectoryAcquireFailure::path_rejected;
        return false;
    }
    const DWORD wait_result = ::WaitForSingleObject(as_handle(mutex_handle_), 0U);
    if (wait_result != WAIT_OBJECT_0 && wait_result != WAIT_ABANDONED) {
        acquire_failure_ = ManifestPairDirectoryAcquireFailure::busy;
        return false;
    }
    mutex_owned_ = true;

    directory_handle_ = ::CreateFileW(
        root_.c_str(),
        FILE_LIST_DIRECTORY | FILE_READ_ATTRIBUTES,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT,
        nullptr);
    if (directory_handle_ == INVALID_HANDLE_VALUE) {
        directory_handle_ = nullptr;
        acquire_failure_ = ManifestPairDirectoryAcquireFailure::path_rejected;
        return false;
    }
    BY_HANDLE_FILE_INFORMATION information{};
    if (::GetFileInformationByHandle(as_handle(directory_handle_), &information) == 0 ||
        (information.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0U ||
        (information.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0U) {
        acquire_failure_ = ManifestPairDirectoryAcquireFailure::path_rejected;
        return false;
    }
    volume_id_ = information.dwVolumeSerialNumber;
#else
    (void)transaction_identity;
    const FdBackedPath fd_path = parse_fd_backed_path(root_);
    if (fd_path.matched) {
        if (!open_fd_backed_directory(root_, descriptor_)) {
            acquire_failure_ = ManifestPairDirectoryAcquireFailure::path_rejected;
            return false;
        }
    } else {
        descriptor_ = ::open(
            root_.c_str(),
            O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
    }
    struct stat information{};
    if (descriptor_ < 0 || ::fstat(descriptor_, &information) != 0 ||
        !S_ISDIR(information.st_mode)) {
        acquire_failure_ = ManifestPairDirectoryAcquireFailure::path_rejected;
        return false;
    }
    if (::flock(descriptor_, LOCK_EX | LOCK_NB) != 0) {
        acquire_failure_ = ManifestPairDirectoryAcquireFailure::busy;
        return false;
    }
    storage_id_ = static_cast<std::uint64_t>(information.st_dev);
#endif
    acquire_failure_ = ManifestPairDirectoryAcquireFailure::none;
    return true;
}

ManifestPairDirectoryAcquireFailure ManifestPairDirectory::acquire_failure() const {
    return acquire_failure_;
}

std::filesystem::path ManifestPairDirectory::full_path(
    const std::filesystem::path& leaf) const {
    return root_ / leaf;
}

bool ManifestPairDirectory::valid_leaf(const std::filesystem::path& leaf) const {
    if (leaf.empty() || leaf == "." || leaf == ".." || leaf.has_parent_path()) {
        return false;
    }
#if defined(_WIN32)
    return leaf.native().find(L':') == std::wstring::npos;
#else
    return true;
#endif
}

ManifestPairEntryKind ManifestPairDirectory::entry_kind(
    const std::filesystem::path& leaf) const {
    if (!valid_leaf(leaf)) {
        return ManifestPairEntryKind::unavailable;
    }
#if defined(_WIN32)
    const DWORD attributes = ::GetFileAttributesW(full_path(leaf).c_str());
    if (attributes == INVALID_FILE_ATTRIBUTES) {
        const DWORD error = ::GetLastError();
        return error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND
            ? ManifestPairEntryKind::missing
            : ManifestPairEntryKind::unavailable;
    }
    if ((attributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0U) {
        return ManifestPairEntryKind::indirect;
    }
    return (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0U
        ? ManifestPairEntryKind::directory
        : ManifestPairEntryKind::regular;
#else
    struct stat information{};
    if (::fstatat(
            descriptor_,
            leaf.c_str(),
            &information,
            AT_SYMLINK_NOFOLLOW) != 0) {
        return errno == ENOENT || errno == ENOTDIR
            ? ManifestPairEntryKind::missing
            : ManifestPairEntryKind::unavailable;
    }
    if (S_ISLNK(information.st_mode) ||
        static_cast<std::uint64_t>(information.st_dev) != storage_id_) {
        return ManifestPairEntryKind::indirect;
    }
    if (S_ISREG(information.st_mode)) {
        return ManifestPairEntryKind::regular;
    }
    if (S_ISDIR(information.st_mode)) {
        return ManifestPairEntryKind::directory;
    }
    return ManifestPairEntryKind::other;
#endif
}

bool ManifestPairDirectory::read_direct_file(
    const std::filesystem::path& leaf,
    std::string& bytes) const {
    bytes.clear();
    if (!valid_leaf(leaf)) {
        return false;
    }
#if defined(_WIN32)
    const HANDLE handle = ::CreateFileW(
        full_path(leaf).c_str(),
        GENERIC_READ,
        FILE_SHARE_READ,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OPEN_REPARSE_POINT,
        nullptr);
    if (handle == INVALID_HANDLE_VALUE) {
        return false;
    }
    BY_HANDLE_FILE_INFORMATION before{};
    LARGE_INTEGER size{};
    bool ok = ::GetFileInformationByHandle(handle, &before) != 0 &&
        (before.dwFileAttributes &
         (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT)) == 0U &&
        before.dwVolumeSerialNumber == volume_id_ &&
        ::GetFileSizeEx(handle, &size) != 0 && size.QuadPart >= 0 &&
        static_cast<unsigned long long>(size.QuadPart) <=
            static_cast<unsigned long long>(std::numeric_limits<std::size_t>::max());
    if (ok) {
        bytes.assign(static_cast<std::size_t>(size.QuadPart), '\0');
        std::size_t offset = 0U;
        while (offset < bytes.size()) {
            const DWORD request = static_cast<DWORD>(std::min<std::size_t>(
                bytes.size() - offset,
                static_cast<std::size_t>(std::numeric_limits<DWORD>::max())));
            DWORD read = 0U;
            if (::ReadFile(handle, bytes.data() + offset, request, &read, nullptr) == 0 ||
                read == 0U) {
                ok = false;
                break;
            }
            offset += read;
        }
    }
    BY_HANDLE_FILE_INFORMATION after{};
    if (!ok || ::GetFileInformationByHandle(handle, &after) == 0 ||
        !same_file_identity(before, after)) {
        ok = false;
    }
    (void)::CloseHandle(handle);
    return ok;
#else
#if defined(__APPLE__)
    // macOS can report ELOOP for this no-follow openat form even when the
    // transaction entry is a regular file. Verify the directory entry
    // without following links, then bind the read to the opened identity.
    const int file = ::openat(
        descriptor_,
        leaf.c_str(),
        O_RDONLY | O_CLOEXEC);
#else
    const int file = ::openat(
        descriptor_,
        leaf.c_str(),
        O_RDONLY | O_NOFOLLOW | O_CLOEXEC);
#endif
    if (file < 0) {
        return false;
    }
    struct stat before{};
    bool ok = ::fstat(file, &before) == 0 && S_ISREG(before.st_mode) &&
        static_cast<std::uint64_t>(before.st_dev) == storage_id_ &&
        before.st_size >= 0 &&
        static_cast<std::uintmax_t>(before.st_size) <=
            static_cast<std::uintmax_t>(std::numeric_limits<std::size_t>::max());
#if defined(__APPLE__)
    struct stat entry_before{};
    ok = ok &&
        ::fstatat(
            descriptor_,
            leaf.c_str(),
            &entry_before,
            AT_SYMLINK_NOFOLLOW) == 0 &&
        S_ISREG(entry_before.st_mode) &&
        same_file_identity(before, entry_before);
#endif
    if (ok) {
        bytes.assign(static_cast<std::size_t>(before.st_size), '\0');
        std::size_t offset = 0U;
        while (offset < bytes.size()) {
            const ssize_t read = ::pread(
                file,
                bytes.data() + offset,
                bytes.size() - offset,
                static_cast<off_t>(offset));
            if (read < 0 && errno == EINTR) {
                continue;
            }
            if (read <= 0) {
                ok = false;
                break;
            }
            offset += static_cast<std::size_t>(read);
        }
    }
    struct stat after{};
    struct stat entry{};
    if (!ok ||
        ::fstat(file, &after) != 0 || !same_file_identity(before, after) ||
        ::fstatat(descriptor_, leaf.c_str(), &entry, AT_SYMLINK_NOFOLLOW) != 0 ||
        !same_file_identity(after, entry)) {
        ok = false;
    }
    (void)::close(file);
    return ok;
#endif
}

bool ManifestPairDirectory::create_direct_file_and_flush(
    const std::filesystem::path& leaf,
    const std::string& contents) const {
    if (!valid_leaf(leaf)) {
        return false;
    }
#if defined(_WIN32)
    const HANDLE handle = ::CreateFileW(
        full_path(leaf).c_str(),
        GENERIC_WRITE,
        0,
        nullptr,
        CREATE_NEW,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OPEN_REPARSE_POINT |
            FILE_FLAG_WRITE_THROUGH,
        nullptr);
    if (handle == INVALID_HANDLE_VALUE) {
        return false;
    }
    bool ok = true;
    std::size_t offset = 0U;
    while (offset < contents.size()) {
        const DWORD request = static_cast<DWORD>(std::min<std::size_t>(
            contents.size() - offset,
            static_cast<std::size_t>(std::numeric_limits<DWORD>::max())));
        DWORD written = 0U;
        if (::WriteFile(handle, contents.data() + offset, request, &written, nullptr) == 0 ||
            written == 0U) {
            ok = false;
            break;
        }
        offset += written;
    }
    if (ok && ::FlushFileBuffers(handle) == 0) {
        ok = false;
    }
    (void)::CloseHandle(handle);
    if (!ok) {
        (void)::DeleteFileW(full_path(leaf).c_str());
    }
    return ok;
#else
    const int file = ::openat(
        descriptor_,
        leaf.c_str(),
        O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC,
        0666);
    if (file < 0) {
        return false;
    }
    bool ok = true;
    std::size_t offset = 0U;
    while (offset < contents.size()) {
        const ssize_t written = ::write(
            file,
            contents.data() + offset,
            contents.size() - offset);
        if (written < 0 && errno == EINTR) {
            continue;
        }
        if (written <= 0) {
            ok = false;
            break;
        }
        offset += static_cast<std::size_t>(written);
    }
    if (ok && ::fsync(file) != 0) {
        ok = false;
    }
    if (::close(file) != 0) {
        ok = false;
    }
    if (!ok || !synchronize_directory()) {
        (void)::unlinkat(descriptor_, leaf.c_str(), 0);
        (void)synchronize_directory();
        return false;
    }
    return true;
#endif
}

bool ManifestPairDirectory::move_direct_file_no_replace(
    const std::filesystem::path& source_leaf,
    const std::filesystem::path& destination_leaf) const {
    if (!valid_leaf(source_leaf) || !valid_leaf(destination_leaf) ||
        source_leaf == destination_leaf) {
        return false;
    }
#if defined(_WIN32)
    const HANDLE source = ::CreateFileW(
        full_path(source_leaf).c_str(),
        GENERIC_READ | GENERIC_WRITE | DELETE | FILE_READ_ATTRIBUTES,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OPEN_REPARSE_POINT |
            FILE_FLAG_WRITE_THROUGH,
        nullptr);
    if (source == INVALID_HANDLE_VALUE) {
        return false;
    }
    BY_HANDLE_FILE_INFORMATION information{};
    bool ok = ::GetFileInformationByHandle(source, &information) != 0 &&
        (information.dwFileAttributes &
         (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT)) == 0U &&
        information.dwVolumeSerialNumber == volume_id_;
    const std::wstring destination = full_path(destination_leaf).native();
    const std::size_t allocation = sizeof(FILE_RENAME_INFO) +
        destination.size() * sizeof(wchar_t);
    std::vector<std::byte> buffer(allocation);
    auto* rename_information = reinterpret_cast<FILE_RENAME_INFO*>(buffer.data());
    rename_information->ReplaceIfExists = FALSE;
    rename_information->RootDirectory = nullptr;
    rename_information->FileNameLength =
        static_cast<DWORD>(destination.size() * sizeof(wchar_t));
    std::memcpy(
        rename_information->FileName,
        destination.data(),
        destination.size() * sizeof(wchar_t));
    if (ok && ::SetFileInformationByHandle(
                  source,
                  FileRenameInfo,
                  rename_information,
                  static_cast<DWORD>(allocation)) == 0) {
        ok = false;
    }
    if (ok && ::FlushFileBuffers(source) == 0) {
        ok = false;
    }
    (void)::CloseHandle(source);
    return ok;
#elif defined(__APPLE__)
    return ::renameatx_np(
               descriptor_,
               source_leaf.c_str(),
               descriptor_,
               destination_leaf.c_str(),
               RENAME_EXCL) == 0 &&
        synchronize_directory();
#elif defined(__linux__) && defined(SYS_renameat2)
    constexpr unsigned int rename_no_replace = 1U;
    return ::syscall(
               SYS_renameat2,
               descriptor_,
               source_leaf.c_str(),
               descriptor_,
               destination_leaf.c_str(),
               rename_no_replace) == 0 &&
        synchronize_directory();
#else
    if (::linkat(
            descriptor_,
            source_leaf.c_str(),
            descriptor_,
            destination_leaf.c_str(),
            0) != 0) {
        return false;
    }
    if (::unlinkat(descriptor_, source_leaf.c_str(), 0) != 0) {
        (void)::unlinkat(descriptor_, destination_leaf.c_str(), 0);
        (void)synchronize_directory();
        return false;
    }
    return synchronize_directory();
#endif
}

bool ManifestPairDirectory::remove_direct_file(
    const std::filesystem::path& leaf) const {
    const ManifestPairEntryKind kind = entry_kind(leaf);
    if (kind == ManifestPairEntryKind::missing) {
        return true;
    }
    if (kind != ManifestPairEntryKind::regular) {
        return false;
    }
#if defined(_WIN32)
    const HANDLE handle = ::CreateFileW(
        full_path(leaf).c_str(),
        GENERIC_WRITE | DELETE | FILE_READ_ATTRIBUTES,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OPEN_REPARSE_POINT |
            FILE_FLAG_WRITE_THROUGH,
        nullptr);
    if (handle == INVALID_HANDLE_VALUE) {
        return false;
    }
    BY_HANDLE_FILE_INFORMATION information{};
    FILE_DISPOSITION_INFO disposition{TRUE};
    const bool ok = ::GetFileInformationByHandle(handle, &information) != 0 &&
        (information.dwFileAttributes &
         (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT)) == 0U &&
        information.dwVolumeSerialNumber == volume_id_ &&
        ::FlushFileBuffers(handle) != 0 &&
        ::SetFileInformationByHandle(
            handle,
            FileDispositionInfo,
            &disposition,
            sizeof(disposition)) != 0;
    return ::CloseHandle(handle) != 0 && ok;
#else
    return ::unlinkat(descriptor_, leaf.c_str(), 0) == 0 &&
        synchronize_directory();
#endif
}

bool ManifestPairDirectory::synchronize_directory() const {
#if defined(_WIN32)
    return true;
#else
    return ::fsync(descriptor_) == 0;
#endif
}

}  // namespace copperfin::runtime::runtime_pipeline_detail
