// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "runtime_pipeline_support.h"

#if defined(COPPERFIN_ENABLE_RUNTIME_PIPELINE_TEST_HOOKS)
#include "runtime_pipeline_test_hooks.h"
#endif

#include <algorithm>
#include <array>
#include <atomic>
#include <cerrno>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <locale>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <dirent.h>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace copperfin::runtime {
namespace {

constexpr std::string_view kPackageBackupSuffix = ".copperfin-previous";
constexpr std::string_view kPackageTransactionMarker = ".copperfin-materializing";
constexpr std::string_view kPackageTransactionOwnerSuffix = ".owner";
constexpr std::string_view kPackageTransactionDeferredPhase = "phase=awaiting_primary_output\n";

#if defined(COPPERFIN_ENABLE_RUNTIME_PIPELINE_TEST_HOOKS)
void trace_content_identity_failure(const std::filesystem::path& path) {
    const int saved_errno = errno;
    std::cerr << "RUNTIME_PIPELINE_CONTENT_ROOT_FAILURE stage=adopt-content"
              << " errno=" << saved_errno
              << " path=" << copperfin::platform::path_to_utf8_string(path)
              << "\n";
}

void trace_transaction_begin_failure(
    const std::string_view stage,
    const std::filesystem::path& path) {
    std::cerr << "RUNTIME_PIPELINE_TRANSACTION_BEGIN_FAILURE stage=" << stage
              << " errno=" << errno
              << " path=" << copperfin::platform::path_to_utf8_string(path)
              << "\n";
}
#endif

#if !defined(COPPERFIN_ENABLE_RUNTIME_PIPELINE_TEST_HOOKS)
void trace_transaction_begin_failure(
    const std::string_view,
    const std::filesystem::path&) {
}
#endif

std::atomic<unsigned long long> native_wrapper_build_sequence{0};

bool path_exists_without_error(const std::filesystem::path& path) {
    std::error_code error;
    return std::filesystem::exists(path, error) && !error;
}

#if defined(COPPERFIN_ENABLE_RUNTIME_PIPELINE_TEST_HOOKS)
std::atomic_bool force_package_backup_cleanup_warning{false};
std::mutex package_materialization_pause_mutex;
std::condition_variable package_materialization_pause_condition;
bool package_materialization_pause_requested = false;
bool package_materialization_pause_entered = false;
bool package_materialization_pause_released = false;
std::mutex package_content_materialization_pause_mutex;
std::condition_variable package_content_materialization_pause_condition;
bool package_content_materialization_pause_requested = false;
bool package_content_materialization_pause_entered = false;
bool package_content_materialization_pause_released = false;
std::mutex package_content_parent_open_pause_mutex;
std::condition_variable package_content_parent_open_pause_condition;
bool package_content_parent_open_pause_requested = false;
bool package_content_parent_open_pause_entered = false;
bool package_content_parent_open_pause_released = false;
#endif

class PackageRootTransactionLock {
public:
    PackageRootTransactionLock() = default;
    PackageRootTransactionLock(const PackageRootTransactionLock&) = delete;
    PackageRootTransactionLock& operator=(const PackageRootTransactionLock&) = delete;

    ~PackageRootTransactionLock() {
#if defined(_WIN32)
        if (mutex_owned_ && mutex_handle_ != nullptr) {
            (void)::ReleaseMutex(static_cast<HANDLE>(mutex_handle_));
        }
        if (mutex_handle_ != nullptr) {
            (void)::CloseHandle(static_cast<HANDLE>(mutex_handle_));
        }
#else
        if (descriptor_ >= 0) {
            (void)::flock(descriptor_, LOCK_UN);
            (void)::close(descriptor_);
        }
#endif
    }

    bool acquire(
        const std::filesystem::path& lock_identity_path,
        const std::string& identity) {
#if defined(_WIN32)
        (void)lock_identity_path;
        const std::wstring mutex_name = L"Local\\CopperfinPackageRoot-" +
            std::wstring(identity.begin(), identity.end());
        mutex_handle_ = static_cast<void*>(::CreateMutexW(nullptr, FALSE, mutex_name.c_str()));
        if (mutex_handle_ == nullptr) {
            return false;
        }
        const DWORD wait_result = ::WaitForSingleObject(
            static_cast<HANDLE>(mutex_handle_),
            0U);
        if (wait_result != WAIT_OBJECT_0 && wait_result != WAIT_ABANDONED) {
            return false;
        }
        mutex_owned_ = true;
        return true;
#else
        const std::filesystem::path lock_path = copperfin::platform::path_from_utf8_string(
            copperfin::platform::path_to_utf8_string(lock_identity_path) + ".copperfin-lock");
        const std::string expected_contents =
            "copperfin_package_lock=1\nidentity=" + identity + "\n";
        std::string temporary_path = copperfin::platform::path_to_utf8_string(lock_path) + ".tmp-XXXXXX";
        std::vector<char> temporary_path_buffer(
            temporary_path.begin(), temporary_path.end());
        temporary_path_buffer.push_back('\0');
        const int temporary_descriptor = ::mkstemp(temporary_path_buffer.data());
        if (temporary_descriptor < 0) {
            return false;
        }
        if (::fcntl(temporary_descriptor, F_SETFD, FD_CLOEXEC) != 0) {
            (void)::close(temporary_descriptor);
            (void)::unlink(temporary_path_buffer.data());
            return false;
        }
        const std::filesystem::path private_lock_path(temporary_path_buffer.data());
        bool temporary_ready = false;
        bool published = false;
        std::size_t offset = 0U;
        while (offset < expected_contents.size()) {
            const ssize_t written = ::write(
                temporary_descriptor,
                expected_contents.data() + offset,
                expected_contents.size() - offset);
            if (written < 0 && errno == EINTR) {
                continue;
            }
            if (written <= 0) {
                break;
            }
            offset += static_cast<std::size_t>(written);
        }
        if (offset == expected_contents.size() && ::fsync(temporary_descriptor) == 0) {
            temporary_ready = true;
        }
        if (!temporary_ready) {
            (void)::close(temporary_descriptor);
            (void)::unlink(private_lock_path.c_str());
            return false;
        }

        if (::link(private_lock_path.c_str(), lock_path.c_str()) == 0) {
            (void)::unlink(private_lock_path.c_str());
            descriptor_ = temporary_descriptor;
            published = true;
        } else if (errno == EEXIST) {
            (void)::close(temporary_descriptor);
            (void)::unlink(private_lock_path.c_str());
            descriptor_ = ::open(
                lock_path.c_str(),
                O_RDWR | O_CLOEXEC | O_NOFOLLOW);
        } else {
            (void)::close(temporary_descriptor);
            (void)::unlink(private_lock_path.c_str());
            return false;
        }
        if (descriptor_ < 0) {
            return false;
        }

        if (!published) {
            char contents[256]{};
            const ssize_t read_count = ::read(descriptor_, contents, sizeof(contents));
            if (read_count != static_cast<ssize_t>(expected_contents.size()) ||
                std::string(contents, static_cast<std::size_t>(read_count)) != expected_contents) {
                (void)::close(descriptor_);
                descriptor_ = -1;
                return false;
            }
        }
        if (::flock(descriptor_, LOCK_EX | LOCK_NB) != 0) {
            (void)::close(descriptor_);
            descriptor_ = -1;
            return false;
        }
        return true;
#endif
    }

private:
#if defined(_WIN32)
    void* mutex_handle_ = nullptr;
    bool mutex_owned_ = false;
#else
    int descriptor_ = -1;
#endif
};

class PackageParentIdentity {
public:
    PackageParentIdentity() = default;
    PackageParentIdentity(const PackageParentIdentity&) = delete;
    PackageParentIdentity& operator=(const PackageParentIdentity&) = delete;

    ~PackageParentIdentity() {
#if defined(_WIN32)
        if (directory_handle_ != nullptr) {
            (void)::CloseHandle(static_cast<HANDLE>(directory_handle_));
        }
#else
        if (descriptor_ >= 0) {
            (void)::close(descriptor_);
        }
#endif
    }

    bool acquire(const std::filesystem::path& parent) {
        std::error_code absolute_error;
        parent_path_ = std::filesystem::absolute(parent, absolute_error).lexically_normal();
        if (absolute_error) {
            parent_path_ = parent.lexically_normal();
        }
#if defined(_WIN32)
        directory_handle_ = static_cast<void*>(::CreateFileW(
            parent_path_.c_str(),
            FILE_LIST_DIRECTORY | FILE_READ_ATTRIBUTES,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            nullptr,
            OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT,
            nullptr));
        if (directory_handle_ == nullptr ||
            directory_handle_ == INVALID_HANDLE_VALUE) {
            directory_handle_ = nullptr;
            return false;
        }
        BY_HANDLE_FILE_INFORMATION information{};
        if (::GetFileInformationByHandle(
                static_cast<HANDLE>(directory_handle_),
                &information) == 0 ||
            (information.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0U ||
            (information.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0U) {
            (void)::CloseHandle(static_cast<HANDLE>(directory_handle_));
            directory_handle_ = nullptr;
            return false;
        }
        volume_id_ = information.dwVolumeSerialNumber;
        file_id_ = (static_cast<std::uint64_t>(information.nFileIndexHigh) << 32U) |
            information.nFileIndexLow;
        return true;
#else
        descriptor_ = ::open(
            parent_path_.c_str(),
            O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
        struct stat information{};
        if (descriptor_ < 0 || ::fstat(descriptor_, &information) != 0 ||
            !S_ISDIR(information.st_mode)) {
            if (descriptor_ >= 0) {
                (void)::close(descriptor_);
                descriptor_ = -1;
            }
            return false;
        }
        device_id_ = information.st_dev;
        inode_id_ = information.st_ino;
        return true;
#endif
    }

    [[nodiscard]] bool still_same() const {
#if defined(_WIN32)
        if (directory_handle_ == nullptr) {
            return false;
        }
        BY_HANDLE_FILE_INFORMATION pinned{};
        if (::GetFileInformationByHandle(
                static_cast<HANDLE>(directory_handle_),
                &pinned) == 0) {
            return false;
        }
        HANDLE current_handle = ::CreateFileW(
            parent_path_.c_str(),
            FILE_READ_ATTRIBUTES,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            nullptr,
            OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT,
            nullptr);
        if (current_handle == INVALID_HANDLE_VALUE) {
            return false;
        }
        BY_HANDLE_FILE_INFORMATION current{};
        const bool same =
            ::GetFileInformationByHandle(current_handle, &current) != 0 &&
            (current.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0U &&
            (current.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) == 0U &&
            current.dwVolumeSerialNumber == volume_id_ &&
            ((static_cast<std::uint64_t>(current.nFileIndexHigh) << 32U) |
             current.nFileIndexLow) == file_id_;
        (void)::CloseHandle(current_handle);
        return same;
#else
        if (descriptor_ < 0) {
            return false;
        }
        struct stat current{};
        return ::lstat(parent_path_.c_str(), &current) == 0 &&
            S_ISDIR(current.st_mode) &&
            current.st_dev == device_id_ &&
            current.st_ino == inode_id_;
#endif
    }

    [[nodiscard]] std::filesystem::path stable_parent_path() const {
#if defined(_WIN32)
        return parent_path_;
#else
        if (descriptor_ < 0) {
            return parent_path_;
        }
        const std::filesystem::path proc_path =
            std::filesystem::path("/proc/self/fd") / std::to_string(descriptor_);
        std::error_code proc_error;
        if (std::filesystem::exists(proc_path, proc_error) && !proc_error) {
            return proc_path;
        }
        return std::filesystem::path("/dev/fd") / std::to_string(descriptor_);
#endif
    }

#if !defined(_WIN32)
    bool open_child_directory(
        const std::filesystem::path& path,
        int& child_descriptor) const {
        child_descriptor = -1;
        if (descriptor_ < 0) {
            return false;
        }
        child_descriptor = ::openat(
            descriptor_,
            copperfin::platform::path_to_utf8_string(path.filename()).c_str(),
            O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
        return child_descriptor >= 0;
    }

    bool child_is_directory(const std::filesystem::path& path) const {
        int child_descriptor = -1;
        if (!open_child_directory(path, child_descriptor)) {
            return false;
        }
        return ::close(child_descriptor) == 0;
    }

    bool child_entry_exists(
        const std::filesystem::path& path,
        std::error_code& error) const {
        struct stat information{};
        if (::fstatat(
                descriptor_,
                copperfin::platform::path_to_utf8_string(path.filename()).c_str(),
                &information,
                AT_SYMLINK_NOFOLLOW) == 0) {
            error.clear();
            return true;
        }
        if (errno == ENOENT) {
            error.clear();
            return false;
        }
        error = std::error_code(errno, std::generic_category());
        return false;
    }

    bool child_is_regular(const std::filesystem::path& path) const {
        struct stat information{};
        return ::fstatat(
                   descriptor_,
                   copperfin::platform::path_to_utf8_string(path.filename()).c_str(),
                   &information,
                   AT_SYMLINK_NOFOLLOW) == 0 &&
            S_ISREG(information.st_mode);
    }

    bool read_child_regular_file(
        const std::filesystem::path& path,
        std::string& contents) const {
        contents.clear();
        if (descriptor_ < 0) {
            return false;
        }
        const std::string leaf =
            copperfin::platform::path_to_utf8_string(path.filename());
#if defined(__APPLE__)
        // macOS reports ELOOP for the no-follow openat form used here even
        // for the regular marker files created by this transaction. Open the
        // descriptor, then reject symlinks through fstatat and bind the read
        // to the descriptor identity before and after consumption.
        const int opened_descriptor = ::openat(
            descriptor_,
            leaf.c_str(),
            O_RDONLY | O_CLOEXEC);
#else
        const int opened_descriptor = ::openat(
            descriptor_,
            leaf.c_str(),
            O_RDONLY | O_NOFOLLOW | O_CLOEXEC);
#endif
        if (opened_descriptor < 0) {
            return false;
        }
        struct stat before{};
        if (::fstat(opened_descriptor, &before) != 0 || !S_ISREG(before.st_mode)) {
            (void)::close(opened_descriptor);
            return false;
        }
#if defined(__APPLE__)
        struct stat entry_before_read{};
        if (::fstatat(
                descriptor_,
                leaf.c_str(),
                &entry_before_read,
                AT_SYMLINK_NOFOLLOW) != 0 ||
            !S_ISREG(entry_before_read.st_mode) ||
            before.st_dev != entry_before_read.st_dev ||
            before.st_ino != entry_before_read.st_ino) {
            (void)::close(opened_descriptor);
            return false;
        }
#endif

        std::array<char, 4096U> buffer{};
        bool read_successfully = true;
        for (;;) {
            const ssize_t count = ::read(opened_descriptor, buffer.data(), buffer.size());
            if (count < 0 && errno == EINTR) {
                continue;
            }
            if (count < 0) {
                read_successfully = false;
                break;
            }
            if (count == 0) {
                break;
            }
            contents.append(buffer.data(), static_cast<std::size_t>(count));
        }

        struct stat after{};
        if (!read_successfully || ::fstat(opened_descriptor, &after) != 0 ||
            !S_ISREG(after.st_mode) || before.st_dev != after.st_dev ||
            before.st_ino != after.st_ino) {
            contents.clear();
            read_successfully = false;
        }
        if (::close(opened_descriptor) != 0) {
            contents.clear();
            read_successfully = false;
        }
        return read_successfully;
    }

    bool adopt_descriptor(
        const int descriptor,
        std::filesystem::path path) {
        if (descriptor < 0) {
            return false;
        }
        struct stat information{};
        if (::fstat(descriptor, &information) != 0 ||
            !S_ISDIR(information.st_mode)) {
            (void)::close(descriptor);
            return false;
        }
        descriptor_ = descriptor;
        parent_path_ = std::move(path);
        device_id_ = information.st_dev;
        inode_id_ = information.st_ino;
        return true;
    }

    bool create_child_directory(const std::filesystem::path& path) const {
        if (descriptor_ < 0) {
            return false;
        }
        const std::string leaf =
            copperfin::platform::path_to_utf8_string(path.filename());
        if (::mkdirat(descriptor_, leaf.c_str(), 0700) == 0) {
            return true;
        }
        if (errno != EEXIST) {
            return false;
        }
        struct stat information{};
        return ::fstatat(
                   descriptor_,
                   leaf.c_str(),
                   &information,
                   AT_SYMLINK_NOFOLLOW) == 0 &&
            S_ISDIR(information.st_mode);
    }

    bool create_child_directory_under(
        const std::filesystem::path& parent,
        const std::string_view child) const {
        int parent_descriptor = -1;
        if (!open_child_directory(parent, parent_descriptor)) {
            return false;
        }
        const std::string child_name(child);
        const bool created = ::mkdirat(parent_descriptor, child_name.c_str(), 0700) == 0;
        if (!created && errno != EEXIST) {
            (void)::close(parent_descriptor);
            return false;
        }
        struct stat information{};
        const bool is_directory = ::fstatat(
            parent_descriptor,
            child_name.c_str(),
            &information,
            AT_SYMLINK_NOFOLLOW) == 0 &&
            S_ISDIR(information.st_mode);
        (void)::close(parent_descriptor);
        return is_directory;
    }

    bool remove_child(
        const std::filesystem::path& path,
        std::error_code& error) const {
        if (descriptor_ < 0) {
            error = std::make_error_code(std::errc::bad_file_descriptor);
            return false;
        }
        const std::string leaf =
            copperfin::platform::path_to_utf8_string(path.filename());
        if (::unlinkat(descriptor_, leaf.c_str(), 0) == 0 || errno == ENOENT) {
            error.clear();
            return true;
        }
        error = std::error_code(errno, std::generic_category());
        return false;
    }

    bool remove_child_tree(
        const std::filesystem::path& path,
        std::error_code& error) const {
        if (descriptor_ < 0) {
            error = std::make_error_code(std::errc::bad_file_descriptor);
            return false;
        }
        errno = 0;
        const bool removed = remove_tree_at(
            descriptor_,
            copperfin::platform::path_to_utf8_string(path.filename()));
        if (removed) {
            error.clear();
            return true;
        }
        error = std::error_code(errno == 0 ? EIO : errno, std::generic_category());
        return false;
    }

    bool rename_child(
        const std::filesystem::path& source,
        const std::filesystem::path& destination,
        std::error_code& error) const {
        if (descriptor_ < 0) {
            error = std::make_error_code(std::errc::bad_file_descriptor);
            return false;
        }
        if (::renameat(
                descriptor_,
                copperfin::platform::path_to_utf8_string(source.filename()).c_str(),
                descriptor_,
                copperfin::platform::path_to_utf8_string(destination.filename()).c_str()) == 0) {
            error.clear();
            return true;
        }
        error = std::error_code(errno, std::generic_category());
        return false;
    }

    bool write_text_file_atomically(
        const std::filesystem::path& path,
        const std::string& contents,
        std::string& error) const {
        if (descriptor_ < 0) {
            trace_transaction_begin_failure("marker-parent-descriptor", path);
            error = runtime_text(
                "Runtime.Package.Error.CreateFileFailed",
                {{"path", copperfin::platform::path_to_utf8_string(path)}});
            return false;
        }

        const std::string target_name =
            copperfin::platform::path_to_utf8_string(path.filename());
        static std::atomic<unsigned long long> sequence{0U};
        const std::string temporary_name =
            target_name + ".tmp." +
            std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()) +
            "." + std::to_string(sequence.fetch_add(1U, std::memory_order_relaxed));
        const int temporary_descriptor = ::openat(
            descriptor_,
            temporary_name.c_str(),
            O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC,
            0600);
        if (temporary_descriptor < 0) {
            trace_transaction_begin_failure("marker-open-temp", path);
            error = runtime_text(
                "Runtime.Package.Error.CreateFileFailed",
                {{"path", copperfin::platform::path_to_utf8_string(path)}});
            return false;
        }

        bool written = true;
        std::size_t offset = 0U;
        while (offset < contents.size()) {
            const ssize_t count = ::write(
                temporary_descriptor,
                contents.data() + offset,
                contents.size() - offset);
            if (count < 0 && errno == EINTR) {
                continue;
            }
            if (count <= 0) {
                written = false;
                break;
            }
            offset += static_cast<std::size_t>(count);
        }
        if (!written || ::fsync(temporary_descriptor) != 0 ||
            ::close(temporary_descriptor) != 0) {
            trace_transaction_begin_failure("marker-write-or-sync", path);
            (void)::close(temporary_descriptor);
            (void)::unlinkat(descriptor_, temporary_name.c_str(), 0);
            error = runtime_text(
                "Runtime.Package.Error.WriteFileFailed",
                {{"path", copperfin::platform::path_to_utf8_string(path)}});
            return false;
        }
        if (::renameat(
                descriptor_,
                temporary_name.c_str(),
                descriptor_,
                target_name.c_str()) != 0) {
            trace_transaction_begin_failure("marker-rename", path);
            (void)::unlinkat(descriptor_, temporary_name.c_str(), 0);
            error = runtime_text(
                "Runtime.Package.Error.WriteFileFailed",
                {{"path", copperfin::platform::path_to_utf8_string(path)}});
            return false;
        }
        return true;
    }
#endif

#if !defined(_WIN32)
    bool rollback_at_pinned_parent(
        const std::string& package_leaf,
        const std::string& backup_leaf,
        const std::string& marker_leaf,
        const std::string& owner_leaf,
        const bool restore_previous) const {
        if (descriptor_ < 0 ||
            !remove_tree_at(descriptor_, package_leaf) ||
            !remove_leaf_at(descriptor_, marker_leaf)) {
            return false;
        }
        if (restore_previous &&
            (::renameat(
                 descriptor_,
                 backup_leaf.c_str(),
                 descriptor_,
                 package_leaf.c_str()) != 0 ||
             !remove_leaf_at(descriptor_, owner_leaf))) {
            return false;
        }
        return true;
    }
#endif

private:
#if !defined(_WIN32)
    static bool remove_leaf_at(const int parent_descriptor, const std::string& leaf) {
        return ::unlinkat(parent_descriptor, leaf.c_str(), 0) == 0 || errno == ENOENT;
    }

    static bool remove_directory_contents(const int directory_descriptor) {
        const int duplicate = ::dup(directory_descriptor);
        if (duplicate < 0) {
            return false;
        }
        DIR* directory = ::fdopendir(duplicate);
        if (directory == nullptr) {
            (void)::close(duplicate);
            return false;
        }
        bool ok = true;
        errno = 0;
        while (const dirent* entry = ::readdir(directory)) {
            const std::string name(entry->d_name);
            if (name == "." || name == "..") {
                continue;
            }
            struct stat information{};
            if (::fstatat(
                    directory_descriptor,
                    name.c_str(),
                    &information,
                    AT_SYMLINK_NOFOLLOW) != 0) {
                ok = false;
                break;
            }
            if (S_ISDIR(information.st_mode)) {
                const int child = ::openat(
                    directory_descriptor,
                    name.c_str(),
                    O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
                if (child < 0 || !remove_directory_contents(child)) {
                    if (child >= 0) {
                        (void)::close(child);
                    }
                    ok = false;
                    break;
                }
                if (::close(child) != 0 ||
                    ::unlinkat(directory_descriptor, name.c_str(), AT_REMOVEDIR) != 0) {
                    ok = false;
                    break;
                }
            } else if (!remove_leaf_at(directory_descriptor, name)) {
                ok = false;
                break;
            }
            errno = 0;
        }
        if (errno != 0) {
            ok = false;
        }
        if (::closedir(directory) != 0) {
            ok = false;
        }
        return ok;
    }

    static bool remove_tree_at(const int parent_descriptor, const std::string& leaf) {
        struct stat information{};
        if (::fstatat(
                parent_descriptor,
                leaf.c_str(),
                &information,
                AT_SYMLINK_NOFOLLOW) != 0) {
            return errno == ENOENT;
        }
        if (!S_ISDIR(information.st_mode)) {
            return remove_leaf_at(parent_descriptor, leaf);
        }
        const int directory = ::openat(
            parent_descriptor,
            leaf.c_str(),
            O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
        if (directory < 0 || !remove_directory_contents(directory)) {
            if (directory >= 0) {
                (void)::close(directory);
            }
            return false;
        }
        if (::close(directory) != 0) {
            return false;
        }
        return ::unlinkat(parent_descriptor, leaf.c_str(), AT_REMOVEDIR) == 0 ||
            errno == ENOENT;
    }
#endif

    std::filesystem::path parent_path_;
#if defined(_WIN32)
    void* directory_handle_ = nullptr;
    std::uint64_t volume_id_ = 0U;
    std::uint64_t file_id_ = 0U;
#else
    int descriptor_ = -1;
    dev_t device_id_ = 0;
    ino_t inode_id_ = 0;
#endif
};

class PackageRootTransaction {
public:
    enum class Mode {
        begin_new,
        resume_deferred,
    };

    explicit PackageRootTransaction(
        std::filesystem::path package_root,
        const Mode mode = Mode::begin_new)
        : package_root_(std::move(package_root)),
          backup_root_(copperfin::platform::path_from_utf8_string(
              copperfin::platform::path_to_utf8_string(package_root_) + std::string(kPackageBackupSuffix))),
          marker_path_(copperfin::platform::path_from_utf8_string(
              copperfin::platform::path_to_utf8_string(package_root_) + std::string(kPackageTransactionMarker))),
          backup_owner_path_(copperfin::platform::path_from_utf8_string(
              copperfin::platform::path_to_utf8_string(backup_root_) + std::string(kPackageTransactionOwnerSuffix))),
          mode_(mode),
          transaction_identity_(
              "copperfin_package_transaction=1\npackage_root=" +
              runtime_pipeline_detail::canonical_casefolded_path_identity(package_root_) + "\n") {
    }

    PackageRootTransaction(const PackageRootTransaction&) = delete;
    PackageRootTransaction& operator=(const PackageRootTransaction&) = delete;

    ~PackageRootTransaction() {
        if (active_) {
            std::string ignored;
            (void)rollback(ignored);
        }
    }

    bool begin(std::string& error) {
        std::error_code filesystem_error;
        const std::filesystem::path parent = package_root_.parent_path();
        if (!parent.empty()) {
            std::filesystem::create_directories(parent, filesystem_error);
            if (filesystem_error) {
                trace_transaction_begin_failure("create-parent", parent);
                error = runtime_text(
                    "Runtime.Package.Error.PackageTransactionStartFailed",
                    {{"path", copperfin::platform::path_to_utf8_string(package_root_)}});
                return false;
            }
        }
        if (!parent_identity_.acquire(parent)) {
            trace_transaction_begin_failure("acquire-parent", parent);
            error = runtime_text(
                "Runtime.Package.Error.PackageTransactionStartFailed",
                {{"path", copperfin::platform::path_to_utf8_string(package_root_)}});
            return false;
        }

        std::filesystem::path lock_identity_path = package_root_;
        std::error_code identity_path_error;
        const std::filesystem::path absolute_identity_path =
            std::filesystem::absolute(package_root_, identity_path_error).lexically_normal();
        if (!identity_path_error) {
            const std::filesystem::path weak_identity_path =
                std::filesystem::weakly_canonical(absolute_identity_path, identity_path_error);
            if (!identity_path_error) {
                lock_identity_path = weak_identity_path;
            } else {
                identity_path_error.clear();
                lock_identity_path = absolute_identity_path;
            }
        }
        std::string lock_identity_value =
            copperfin::platform::path_to_utf8_string(lock_identity_path.lexically_normal());
#if defined(_WIN32)
        lock_identity_value =
            runtime_pipeline_detail::canonical_casefolded_path_identity(package_root_);
#endif
        const auto lock_identity = security::sha256_hex_for_text(lock_identity_value);
        if (!lock_identity.ok || !transaction_lock_.acquire(lock_identity_path, lock_identity.hex_digest)) {
            trace_transaction_begin_failure("acquire-lock", lock_identity_path);
            error = runtime_text(
                "Runtime.Package.Error.PackageTransactionStartFailed",
                {{"path", copperfin::platform::path_to_utf8_string(package_root_)}});
            return false;
        }
        if (!parent_identity_.still_same()) {
            trace_transaction_begin_failure("parent-rebound", parent);
            error = runtime_text(
                "Runtime.Package.Error.PackageTransactionStartFailed",
                {{"path", copperfin::platform::path_to_utf8_string(parent)}});
            return false;
        }

#if defined(_WIN32)
        bool interrupted_backup_exists =
            directory_entry_exists(pinned_path(backup_root_), filesystem_error);
#else
        bool interrupted_backup_exists =
            parent_identity_.child_entry_exists(backup_root_, filesystem_error);
#endif
        if (filesystem_error) {
            error = runtime_text(
                "Runtime.Package.Error.PackageTransactionStartFailed",
                {{"path", copperfin::platform::path_to_utf8_string(backup_root_)}});
            return false;
        }
#if defined(_WIN32)
        bool package_exists = directory_entry_exists(pinned_path(package_root_), filesystem_error);
#else
        bool package_exists =
            parent_identity_.child_entry_exists(package_root_, filesystem_error);
#endif
        if (filesystem_error) {
            error = runtime_text(
                "Runtime.Package.Error.PackageTransactionStartFailed",
                {{"path", copperfin::platform::path_to_utf8_string(package_root_)}});
            return false;
        }
#if defined(_WIN32)
        bool transaction_marker_exists =
            directory_entry_exists(pinned_path(marker_path_), filesystem_error);
#else
        bool transaction_marker_exists =
            parent_identity_.child_entry_exists(marker_path_, filesystem_error);
#endif
        if (filesystem_error) {
            error = runtime_text(
                "Runtime.Package.Error.PackageTransactionStartFailed",
                {{"path", copperfin::platform::path_to_utf8_string(marker_path_)}});
            return false;
        }
        if (transaction_marker_exists && !is_owned_transaction_file(pinned_path(marker_path_))) {
            error = runtime_text(
                "Runtime.Package.Error.PackageTransactionStartFailed",
                {{"path", copperfin::platform::path_to_utf8_string(marker_path_)}});
            return false;
        }
#if defined(_WIN32)
        bool backup_owner_exists =
            directory_entry_exists(pinned_path(backup_owner_path_), filesystem_error);
#else
        bool backup_owner_exists =
            parent_identity_.child_entry_exists(backup_owner_path_, filesystem_error);
#endif
        if (filesystem_error) {
            error = runtime_text(
                "Runtime.Package.Error.PackageTransactionStartFailed",
                {{"path", copperfin::platform::path_to_utf8_string(backup_owner_path_)}});
            return false;
        }
        if (backup_owner_exists && !is_owned_transaction_file(pinned_path(backup_owner_path_))) {
            error = runtime_text(
                "Runtime.Package.Error.PackageTransactionStartFailed",
                {{"path", copperfin::platform::path_to_utf8_string(backup_owner_path_)}});
            return false;
        }
        if (!interrupted_backup_exists && backup_owner_exists) {
            if (!ensure_parent_identity(
                    error,
                    "Runtime.Package.Error.PackageTransactionStartFailed")) {
                return false;
            }
#if defined(_WIN32)
            std::filesystem::remove(pinned_path(backup_owner_path_), filesystem_error);
#else
            parent_identity_.remove_child(backup_owner_path_, filesystem_error);
#endif
            if (filesystem_error) {
                error = runtime_text(
                    "Runtime.Package.Error.PackageTransactionStartFailed",
                    {{"path", copperfin::platform::path_to_utf8_string(backup_owner_path_)}});
                return false;
            }
            backup_owner_exists = false;
        }
        if (interrupted_backup_exists && !backup_owner_exists) {
            error = runtime_text(
                "Runtime.Package.Error.PackageTransactionStartFailed",
                {{"path", copperfin::platform::path_to_utf8_string(backup_root_)}});
            return false;
        }

        if (mode_ == Mode::resume_deferred) {
#if defined(_WIN32)
            const bool package_is_directory =
                is_direct_directory(pinned_path(package_root_), filesystem_error);
            const bool deferred_marker_matches =
                transaction_marker_exists &&
                read_text_file(pinned_path(marker_path_)) ==
                    transaction_identity_ + std::string(kPackageTransactionDeferredPhase);
#else
            const bool package_is_directory =
                parent_identity_.child_is_directory(package_root_);
            std::string deferred_marker_contents;
            const bool deferred_marker_matches =
                transaction_marker_exists &&
                parent_identity_.read_child_regular_file(
                    marker_path_,
                    deferred_marker_contents) &&
                deferred_marker_contents ==
                    transaction_identity_ + std::string(kPackageTransactionDeferredPhase);
#endif
            if (!deferred_marker_matches ||
                !package_exists ||
                !package_is_directory ||
                filesystem_error) {
                error = runtime_text(
                    "Runtime.Package.Error.PackageTransactionStartFailed",
                    {{"path", copperfin::platform::path_to_utf8_string(package_root_)}});
                return false;
            }
            if (interrupted_backup_exists &&
#if defined(_WIN32)
                (!is_direct_directory(pinned_path(backup_root_), filesystem_error) ||
#else
                (!parent_identity_.child_is_directory(backup_root_) ||
#endif
                 filesystem_error)) {
                error = runtime_text(
                    "Runtime.Package.Error.PackageTransactionStartFailed",
                    {{"path", copperfin::platform::path_to_utf8_string(backup_root_)}});
                return false;
            }
            had_previous_package_ = interrupted_backup_exists;
            active_ = true;
            return true;
        }

        if (interrupted_backup_exists) {
#if defined(_WIN32)
            if (!is_direct_directory(pinned_path(backup_root_), filesystem_error) || filesystem_error) {
#else
            if (!parent_identity_.child_is_directory(backup_root_)) {
#endif
                error = runtime_text(
                    "Runtime.Package.Error.PackageTransactionStartFailed",
                    {{"path", copperfin::platform::path_to_utf8_string(backup_root_)}});
                return false;
            }
            bool partial_package = false;
            if (package_exists) {
#if defined(_WIN32)
                const bool package_is_directory =
                    std::filesystem::is_directory(pinned_path(package_root_), filesystem_error);
#else
                const bool package_is_directory =
                    parent_identity_.child_is_directory(package_root_);
#endif
                if (filesystem_error) {
                    error = runtime_text(
                        "Runtime.Package.Error.PackageTransactionStartFailed",
                        {{"path", copperfin::platform::path_to_utf8_string(package_root_)}});
                    return false;
                }
                if (!package_is_directory) {
                    partial_package = true;
                } else {
                    partial_package = transaction_marker_exists;
                }
            }

            if (partial_package) {
                if (!ensure_parent_identity(
                        error,
                        "Runtime.Package.Error.PackageTransactionStartFailed")) {
                    return false;
                }
#if defined(_WIN32)
                std::filesystem::remove_all(pinned_path(package_root_), filesystem_error);
#else
                parent_identity_.remove_child_tree(package_root_, filesystem_error);
#endif
                if (filesystem_error) {
                    error = runtime_text(
                        "Runtime.Package.Error.PackageTransactionStartFailed",
                        {{"path", copperfin::platform::path_to_utf8_string(package_root_)}});
                    return false;
                }
                package_exists = false;
                had_previous_package_ = true;
            } else if (package_exists) {
                if (!ensure_parent_identity(
                        error,
                        "Runtime.Package.Error.PackageTransactionStartFailed")) {
                    return false;
                }
#if defined(_WIN32)
                std::filesystem::remove_all(pinned_path(backup_root_), filesystem_error);
#else
                parent_identity_.remove_child_tree(backup_root_, filesystem_error);
#endif
                if (filesystem_error) {
                    error = runtime_text(
                        "Runtime.Package.Error.PackageTransactionStartFailed",
                        {{"path", copperfin::platform::path_to_utf8_string(backup_root_)}});
                    return false;
                }
#if defined(_WIN32)
                std::filesystem::remove(pinned_path(backup_owner_path_), filesystem_error);
#else
                parent_identity_.remove_child(backup_owner_path_, filesystem_error);
#endif
                if (filesystem_error) {
                    error = runtime_text(
                        "Runtime.Package.Error.PackageTransactionStartFailed",
                        {{"path", copperfin::platform::path_to_utf8_string(backup_owner_path_)}});
                    return false;
                }
                interrupted_backup_exists = false;
            } else {
                had_previous_package_ = true;
            }
        }

        if (!interrupted_backup_exists && transaction_marker_exists) {
            if (package_exists) {
                if (!ensure_parent_identity(
                        error,
                        "Runtime.Package.Error.PackageTransactionStartFailed")) {
                    return false;
                }
#if defined(_WIN32)
                std::filesystem::remove_all(pinned_path(package_root_), filesystem_error);
#else
                parent_identity_.remove_child_tree(package_root_, filesystem_error);
#endif
                if (filesystem_error) {
                    error = runtime_text(
                        "Runtime.Package.Error.PackageTransactionStartFailed",
                        {{"path", copperfin::platform::path_to_utf8_string(package_root_)}});
                    return false;
                }
                package_exists = false;
            }
            if (!ensure_parent_identity(
                    error,
                    "Runtime.Package.Error.PackageTransactionStartFailed")) {
                return false;
            }
#if defined(_WIN32)
            std::filesystem::remove(pinned_path(marker_path_), filesystem_error);
#else
            parent_identity_.remove_child(marker_path_, filesystem_error);
#endif
            if (filesystem_error) {
                error = runtime_text(
                    "Runtime.Package.Error.PackageTransactionStartFailed",
                    {{"path", copperfin::platform::path_to_utf8_string(marker_path_)}});
                return false;
            }
            transaction_marker_exists = false;
        }

        if (!interrupted_backup_exists && package_exists) {
#if defined(_WIN32)
            const bool package_is_directory =
                std::filesystem::is_directory(pinned_path(package_root_), filesystem_error);
#else
            const bool package_is_directory =
                parent_identity_.child_is_directory(package_root_);
#endif
            if (filesystem_error || !package_is_directory) {
                error = runtime_text(
                    "Runtime.Package.Error.PackageTransactionStartFailed",
                    {{"path", copperfin::platform::path_to_utf8_string(package_root_)}});
                return false;
            }
            std::string owner_error;
            if (!write_owned_transaction_file_atomically(backup_owner_path_, owner_error)) {
                error = owner_error.empty()
                    ? runtime_text(
                          "Runtime.Package.Error.PackageTransactionStartFailed",
                          {{"path", copperfin::platform::path_to_utf8_string(package_root_)}})
                    : owner_error;
                return false;
            }
            if (!ensure_parent_identity(
                    error,
                    "Runtime.Package.Error.PackageTransactionStartFailed")) {
                std::error_code ignored;
#if defined(_WIN32)
                std::filesystem::remove(pinned_path(backup_owner_path_), ignored);
#else
                parent_identity_.remove_child(backup_owner_path_, ignored);
#endif
                return false;
            }
#if defined(_WIN32)
            std::filesystem::rename(
                pinned_path(package_root_),
                pinned_path(backup_root_),
                filesystem_error);
#else
            parent_identity_.rename_child(package_root_, backup_root_, filesystem_error);
#endif
            if (filesystem_error) {
                std::error_code ignored;
#if defined(_WIN32)
                std::filesystem::remove(pinned_path(backup_owner_path_), ignored);
#else
                parent_identity_.remove_child(backup_owner_path_, ignored);
#endif
                error = runtime_text(
                    "Runtime.Package.Error.PackageTransactionStartFailed",
                    {{"path", copperfin::platform::path_to_utf8_string(package_root_)}});
                return false;
            }
            had_previous_package_ = true;
        }

        active_ = true;
        if (!ensure_parent_identity(
                error,
                "Runtime.Package.Error.PackageTransactionStartFailed")) {
            std::string ignored;
            (void)rollback(ignored);
            return false;
        }
        if (!transaction_marker_exists) {
            std::string marker_error;
            if (!write_owned_transaction_file_atomically(marker_path_, marker_error)) {
                std::string ignored;
                (void)rollback(ignored);
                error = marker_error.empty()
                    ? runtime_text(
                          "Runtime.Package.Error.PackageTransactionStartFailed",
                          {{"path", copperfin::platform::path_to_utf8_string(marker_path_)}})
                    : marker_error;
                return false;
            }
        }
        if (!ensure_parent_identity(
                error,
                "Runtime.Package.Error.PackageTransactionStartFailed")) {
            std::string ignored;
            (void)rollback(ignored);
            return false;
        }
#if defined(_WIN32)
        std::filesystem::create_directories(pinned_path(package_root_), filesystem_error);
#else
        const bool package_root_ready =
            parent_identity_.create_child_directory(package_root_);
        if (!package_root_ready) {
            trace_transaction_begin_failure("create-package-root", package_root_);
            filesystem_error = std::make_error_code(std::errc::io_error);
        }
#endif
        if (filesystem_error) {
            std::string ignored;
            (void)rollback(ignored);
            error = runtime_text(
                "Runtime.Package.Error.PackageTransactionStartFailed",
                {{"path", copperfin::platform::path_to_utf8_string(package_root_)}});
            return false;
        }
        return true;
    }

    bool validate_parent_identity_for_materialization(std::string& error) const {
        return ensure_parent_identity(
            error,
            "Runtime.Package.Error.PackageTransactionStartFailed");
    }

#if !defined(_WIN32)
    bool create_pinned_child_directory(
        const std::filesystem::path& parent,
        const std::string_view child) const {
        return parent_identity_.create_child_directory_under(parent, child);
    }

#endif

    [[nodiscard]] RuntimePackagePlan pinned_filesystem_plan(
        const RuntimePackagePlan& logical_plan) const {
        RuntimePackagePlan filesystem_plan = logical_plan;
#if !defined(_WIN32)
        const std::filesystem::path logical_root =
            copperfin::platform::path_from_utf8_string(logical_plan.package_root);
        const std::filesystem::path pinned_root =
            parent_identity_.stable_parent_path() / logical_root.filename();
        const auto pin_path = [&](std::string& value) {
            if (value.empty()) {
                return;
            }
            const std::filesystem::path candidate = copperfin::platform::path_from_utf8_string(value);
            const std::filesystem::path relative =
                candidate.lexically_relative(logical_root);
            if (candidate == logical_root) {
                value = copperfin::platform::path_to_utf8_string(pinned_root);
            } else if (!relative.empty() && !relative.is_absolute()) {
                bool escapes = false;
                for (const auto& component : relative) {
                    if (component == "..") {
                        escapes = true;
                        break;
                    }
                }
                if (!escapes) {
                    value = copperfin::platform::path_to_utf8_string(
                        (pinned_root / relative).lexically_normal());
                }
            }
        };
        for (std::string* value : {
                 &filesystem_plan.package_root,
                 &filesystem_plan.content_root,
                 &filesystem_plan.manifest_path,
                 &filesystem_plan.debug_manifest_path,
                 &filesystem_plan.ast_manifest_path,
                 &filesystem_plan.ir_manifest_path,
                 &filesystem_plan.transpiled_csharp_path,
                 &filesystem_plan.launcher_project_path,
                 &filesystem_plan.launcher_source_path,
                 &filesystem_plan.launcher_output_path,
                 &filesystem_plan.module_definition_path,
                 &filesystem_plan.native_wrapper_source_path,
                 &filesystem_plan.native_wrapper_cmake_path,
                 &filesystem_plan.native_wrapper_build_script_path,
                 &filesystem_plan.native_wrapper_build_powershell_path,
                 &filesystem_plan.library_api_manifest_path,
                 &filesystem_plan.fll_api_manifest_path,
                 &filesystem_plan.fxp_token_manifest_path,
                 &filesystem_plan.app_archive_manifest_path,
                 &filesystem_plan.runtime_host_destination_path,
                 &filesystem_plan.working_directory,
                 &filesystem_plan.audit_log_path,
                 &filesystem_plan.debug_plan.manifest_path,
                 &filesystem_plan.debug_plan.working_directory}) {
            pin_path(*value);
        }
        for (std::string& source_root : filesystem_plan.debug_plan.source_roots) {
            pin_path(source_root);
        }
#endif
        return filesystem_plan;
    }

    bool rollback(std::string& error) {
        if (!active_) {
            return true;
        }

#if !defined(_WIN32)
        if (!parent_identity_.still_same()) {
            error = runtime_text(
                "Runtime.Package.Error.PackageRollbackFailed",
                {{"path", copperfin::platform::path_to_utf8_string(package_root_)}});
            if (!parent_identity_.rollback_at_pinned_parent(
                    copperfin::platform::path_to_utf8_string(package_root_.filename()),
                    copperfin::platform::path_to_utf8_string(backup_root_.filename()),
                    copperfin::platform::path_to_utf8_string(marker_path_.filename()),
                    copperfin::platform::path_to_utf8_string(backup_owner_path_.filename()),
                    had_previous_package_)) {
                return false;
            }
            active_ = false;
            return true;
        }
        if (!parent_identity_.rollback_at_pinned_parent(
                copperfin::platform::path_to_utf8_string(package_root_.filename()),
                copperfin::platform::path_to_utf8_string(backup_root_.filename()),
                copperfin::platform::path_to_utf8_string(marker_path_.filename()),
                copperfin::platform::path_to_utf8_string(backup_owner_path_.filename()),
                had_previous_package_)) {
            error = runtime_text(
                "Runtime.Package.Error.PackageRollbackFailed",
                {{"path", copperfin::platform::path_to_utf8_string(package_root_)}});
            return false;
        }
        active_ = false;
        return true;
#else
        if (!ensure_parent_identity(
                error,
                "Runtime.Package.Error.PackageRollbackFailed")) {
            return false;
        }

        std::error_code filesystem_error;
        const bool package_exists = std::filesystem::exists(
            pinned_path(package_root_),
            filesystem_error);
        if (filesystem_error) {
            error = runtime_text(
                "Runtime.Package.Error.PackageRollbackFailed",
                {{"path", copperfin::platform::path_to_utf8_string(package_root_)}});
            return false;
        }
        if (package_exists) {
            if (!ensure_parent_identity(
                    error,
                    "Runtime.Package.Error.PackageRollbackFailed")) {
                return false;
            }
            std::filesystem::remove_all(pinned_path(package_root_), filesystem_error);
        }
        if (filesystem_error) {
            error = runtime_text(
                "Runtime.Package.Error.PackageRollbackFailed",
                {{"path", copperfin::platform::path_to_utf8_string(package_root_)}});
            return false;
        }

        if (!ensure_parent_identity(
                error,
                "Runtime.Package.Error.PackageRollbackFailed")) {
            return false;
        }
        std::filesystem::remove(pinned_path(marker_path_), filesystem_error);
        if (filesystem_error) {
            error = runtime_text(
                "Runtime.Package.Error.PackageRollbackFailed",
                {{"path", copperfin::platform::path_to_utf8_string(marker_path_)}});
            return false;
        }

        if (had_previous_package_) {
            if (!ensure_parent_identity(
                    error,
                    "Runtime.Package.Error.PackageRollbackFailed")) {
                return false;
            }
            std::filesystem::rename(
                pinned_path(backup_root_),
                pinned_path(package_root_),
                filesystem_error);
            if (filesystem_error) {
                error = runtime_text(
                    "Runtime.Package.Error.PackageRollbackFailed",
                    {{"path", copperfin::platform::path_to_utf8_string(backup_root_)}});
                return false;
            }
            active_ = false;
            std::filesystem::remove(pinned_path(backup_owner_path_), filesystem_error);
            if (filesystem_error) {
                error = runtime_text(
                    "Runtime.Package.Error.PackageRollbackFailed",
                    {{"path", copperfin::platform::path_to_utf8_string(backup_owner_path_)}});
                return false;
            }
            return true;
        }

        active_ = false;
        return true;
#endif
    }

    bool commit(std::string& error, std::string& warning) {
        if (!active_) {
            return true;
        }

#if !defined(_WIN32)
        if (!ensure_parent_identity(
                error,
                "Runtime.Package.Error.PackageTransactionStartFailed")) {
            return false;
        }
        std::error_code filesystem_error;
        if (!parent_identity_.remove_child(marker_path_, filesystem_error)) {
            error = runtime_text(
                "Runtime.Package.Error.PackageTransactionStartFailed",
                {{"path", copperfin::platform::path_to_utf8_string(package_root_)}});
            return false;
        }
        if (!had_previous_package_) {
            active_ = false;
            return true;
        }
        if (!parent_identity_.remove_child_tree(backup_root_, filesystem_error)) {
            active_ = false;
            warning = runtime_text(
                "Runtime.Package.Warning.PackageBackupCleanupFailed",
                {{"path", copperfin::platform::path_to_utf8_string(backup_root_)}});
            return true;
        }
        if (!parent_identity_.remove_child(backup_owner_path_, filesystem_error)) {
            warning = runtime_text(
                "Runtime.Package.Warning.PackageBackupCleanupFailed",
                {{"path", copperfin::platform::path_to_utf8_string(backup_owner_path_)}});
        }
        active_ = false;
#if defined(COPPERFIN_ENABLE_RUNTIME_PIPELINE_TEST_HOOKS)
        if (force_package_backup_cleanup_warning.exchange(
                false,
                std::memory_order_relaxed)) {
            warning = runtime_text(
                "Runtime.Package.Warning.PackageBackupCleanupFailed",
                {{"path", copperfin::platform::path_to_utf8_string(backup_root_)}});
        }
#endif
        return true;
#else
        if (!ensure_parent_identity(
                error,
                "Runtime.Package.Error.PackageTransactionStartFailed")) {
            return false;
        }

        std::error_code filesystem_error;
        std::filesystem::remove(pinned_path(marker_path_), filesystem_error);
        if (filesystem_error) {
            error = runtime_text(
                "Runtime.Package.Error.PackageTransactionStartFailed",
                {{"path", copperfin::platform::path_to_utf8_string(package_root_)}});
            return false;
        }

        if (!had_previous_package_) {
            active_ = false;
            return true;
        }

        if (!ensure_parent_identity(
                error,
                "Runtime.Package.Error.PackageTransactionStartFailed")) {
            return false;
        }
        std::filesystem::remove_all(pinned_path(backup_root_), filesystem_error);
        if (filesystem_error) {
            active_ = false;
            warning = runtime_text(
                "Runtime.Package.Warning.PackageBackupCleanupFailed",
                {{"path", copperfin::platform::path_to_utf8_string(backup_root_)}});
            return true;
        }

        std::filesystem::remove(pinned_path(backup_owner_path_), filesystem_error);
        if (filesystem_error) {
            warning = runtime_text(
                "Runtime.Package.Warning.PackageBackupCleanupFailed",
                {{"path", copperfin::platform::path_to_utf8_string(backup_owner_path_)}});
        }
        active_ = false;
#if defined(COPPERFIN_ENABLE_RUNTIME_PIPELINE_TEST_HOOKS)
        if (force_package_backup_cleanup_warning.exchange(
                false,
                std::memory_order_relaxed)) {
            warning = runtime_text(
                "Runtime.Package.Warning.PackageBackupCleanupFailed",
                {{"path", copperfin::platform::path_to_utf8_string(backup_root_)}});
        }
#endif
        return true;
#endif
    }

    bool defer_until_primary_output(std::string& error) {
        if (!active_) {
            return false;
        }
        if (!ensure_parent_identity(
                error,
                "Runtime.Package.Error.PackageTransactionStartFailed")) {
            return false;
        }
        std::string marker_error;
        if (!write_owned_transaction_file_atomically(
                marker_path_,
                marker_error,
                transaction_identity_ + std::string(kPackageTransactionDeferredPhase))) {
            error = marker_error.empty()
                ? runtime_text(
                      "Runtime.Package.Error.PackageTransactionStartFailed",
                      {{"path", copperfin::platform::path_to_utf8_string(marker_path_)}})
                : marker_error;
            return false;
        }
        active_ = false;
        return true;
    }

private:
    [[nodiscard]] std::filesystem::path pinned_path(
        const std::filesystem::path& logical_path) const {
#if defined(_WIN32)
        return logical_path;
#else
        return parent_identity_.stable_parent_path() / logical_path.filename();
#endif
    }

    bool ensure_parent_identity(
        std::string& error,
        const std::string_view error_key) const {
        if (parent_identity_.still_same()) {
            return true;
        }
        trace_transaction_begin_failure("parent-identity", package_root_);
        error = runtime_text(error_key, {{"path", copperfin::platform::path_to_utf8_string(package_root_)}});
        return false;
    }

    bool write_owned_transaction_file_atomically(
        const std::filesystem::path& path,
        std::string& error,
        const std::string& contents = {}) const {
        if (!parent_identity_.still_same()) {
            trace_transaction_begin_failure("owned-file-parent", path);
            error = runtime_text(
                "Runtime.Package.Error.PackageTransactionStartFailed",
                {{"path", copperfin::platform::path_to_utf8_string(path)}});
            return false;
        }
#if !defined(_WIN32)
        return parent_identity_.write_text_file_atomically(
            path,
            contents.empty() ? transaction_identity_ : contents,
            error);
#else
        static std::atomic<unsigned long long> sequence{0U};
        const auto timestamp = std::chrono::steady_clock::now().time_since_epoch().count();
        const std::filesystem::path pinned_target = pinned_path(path);
        const std::filesystem::path temporary_path =
            copperfin::platform::path_from_utf8_string(
                copperfin::platform::path_to_utf8_string(pinned_target) + ".tmp." +
                std::to_string(timestamp) + "." +
                std::to_string(sequence.fetch_add(1U, std::memory_order_relaxed)));

        std::error_code filesystem_error;
        if (directory_entry_exists(temporary_path, filesystem_error) || filesystem_error) {
            trace_transaction_begin_failure("owned-file-temp", temporary_path);
            error = runtime_text(
                "Runtime.Package.Error.PackageTransactionStartFailed",
                {{"path", copperfin::platform::path_to_utf8_string(temporary_path)}});
            return false;
        }

        std::string write_error;
        if (!write_text_file(
                temporary_path,
                contents.empty() ? transaction_identity_ : contents,
                write_error)) {
            trace_transaction_begin_failure("owned-file-write", path);
            std::filesystem::remove(temporary_path, filesystem_error);
            error = write_error.empty()
                ? runtime_text(
                      "Runtime.Package.Error.PackageTransactionStartFailed",
                      {{"path", copperfin::platform::path_to_utf8_string(path)}})
                : write_error;
            return false;
        }

        std::filesystem::rename(temporary_path, pinned_target, filesystem_error);
        if (!filesystem_error) {
            return true;
        }

        std::error_code ignored;
        std::filesystem::remove(temporary_path, ignored);
        error = runtime_text(
            "Runtime.Package.Error.PackageTransactionStartFailed",
            {{"path", copperfin::platform::path_to_utf8_string(path)}});
        trace_transaction_begin_failure("owned-file-rename", path);
        return false;
#endif
    }

    static bool directory_entry_exists(
        const std::filesystem::path& path,
        std::error_code& error) {
        const std::filesystem::file_status status =
            std::filesystem::symlink_status(path, error);
        if (error == std::errc::no_such_file_or_directory) {
            error.clear();
            return false;
        }
        if (error) {
            trace_transaction_begin_failure("directory-entry", path);
        }
        return !error && status.type() != std::filesystem::file_type::not_found;
    }

    static bool is_direct_directory(
        const std::filesystem::path& path,
        std::error_code& error) {
        const std::filesystem::file_status status =
            std::filesystem::symlink_status(path, error);
        if (error) {
            trace_transaction_begin_failure("direct-directory", path);
        }
        return !error && status.type() == std::filesystem::file_type::directory;
    }

    bool is_owned_transaction_file(const std::filesystem::path& path) const {
#if !defined(_WIN32)
        std::string contents;
        if (!parent_identity_.read_child_regular_file(path, contents)) {
            trace_transaction_begin_failure("owned-file-status", path);
            return false;
        }
#else
        const DWORD attributes = ::GetFileAttributesW(path.c_str());
        if (attributes == INVALID_FILE_ATTRIBUTES ||
            (attributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT)) != 0U) {
            trace_transaction_begin_failure("owned-file-status", path);
            return false;
        }
#endif
#if defined(_WIN32)
        const std::string contents = read_text_file(path);
#endif
        if (contents == transaction_identity_ ||
            contents == transaction_identity_ + std::string(kPackageTransactionDeferredPhase)) {
            return true;
        }
#if defined(_WIN32)
        const std::string prefix =
            "copperfin_package_transaction=1\npackage_root=";
        if (contents.rfind(prefix, 0U) != 0U ||
            contents.size() <= prefix.size() ||
            contents.back() != '\n') {
            return false;
        }

        const std::string recorded_path = contents.substr(
            prefix.size(),
            contents.size() - prefix.size() - 1U);
        return !recorded_path.empty() &&
               recorded_path.find('\n') == std::string::npos &&
               runtime_pipeline_detail::canonical_casefolded_path_identity(
                   copperfin::platform::path_from_utf8_string(recorded_path)) ==
                   runtime_pipeline_detail::canonical_casefolded_path_identity(package_root_);
#else
        return false;
#endif
    }

    std::filesystem::path package_root_;
    std::filesystem::path backup_root_;
    std::filesystem::path marker_path_;
    std::filesystem::path backup_owner_path_;
    Mode mode_ = Mode::begin_new;
    std::string transaction_identity_;
    PackageRootTransactionLock transaction_lock_;
    PackageParentIdentity parent_identity_;
    bool had_previous_package_ = false;
    bool active_ = false;
};

}  // namespace

#if defined(COPPERFIN_ENABLE_RUNTIME_PIPELINE_TEST_HOOKS)
namespace test_hooks {

void force_package_backup_cleanup_warning_once() {
    force_package_backup_cleanup_warning.store(true, std::memory_order_relaxed);
}

void arm_package_materialization_pause_after_begin() {
    std::lock_guard<std::mutex> lock(package_materialization_pause_mutex);
    package_materialization_pause_requested = true;
    package_materialization_pause_entered = false;
    package_materialization_pause_released = false;
}

bool wait_for_package_materialization_pause() {
    std::unique_lock<std::mutex> lock(package_materialization_pause_mutex);
    return package_materialization_pause_condition.wait_for(lock, std::chrono::seconds(10), [] {
        return package_materialization_pause_entered;
    });
}

void release_package_materialization_pause() {
    {
        std::lock_guard<std::mutex> lock(package_materialization_pause_mutex);
        package_materialization_pause_released = true;
    }
    package_materialization_pause_condition.notify_all();
}

void arm_package_content_materialization_pause_before_first_asset() {
    std::lock_guard<std::mutex> lock(package_content_materialization_pause_mutex);
    package_content_materialization_pause_requested = true;
    package_content_materialization_pause_entered = false;
    package_content_materialization_pause_released = false;
}

bool wait_for_package_content_materialization_pause() {
    std::unique_lock<std::mutex> lock(package_content_materialization_pause_mutex);
    return package_content_materialization_pause_condition.wait_for(
        lock,
        std::chrono::seconds(10),
        [] {
            return package_content_materialization_pause_entered;
        });
}

void release_package_content_materialization_pause() {
    {
        std::lock_guard<std::mutex> lock(package_content_materialization_pause_mutex);
        package_content_materialization_pause_released = true;
    }
    package_content_materialization_pause_condition.notify_all();
}

void pause_before_package_content_parent_open() {
    std::unique_lock<std::mutex> lock(package_content_parent_open_pause_mutex);
    if (!package_content_parent_open_pause_requested) {
        return;
    }
    package_content_parent_open_pause_entered = true;
    package_content_parent_open_pause_condition.notify_all();
    package_content_parent_open_pause_condition.wait(
        lock,
        [] {
            return package_content_parent_open_pause_released;
        });
    package_content_parent_open_pause_requested = false;
    package_content_parent_open_pause_entered = false;
    package_content_parent_open_pause_released = false;
}

void arm_package_content_parent_open_pause() {
    std::lock_guard<std::mutex> lock(package_content_parent_open_pause_mutex);
    package_content_parent_open_pause_requested = true;
    package_content_parent_open_pause_entered = false;
    package_content_parent_open_pause_released = false;
}

bool wait_for_package_content_parent_open_pause() {
    std::unique_lock<std::mutex> lock(package_content_parent_open_pause_mutex);
    return package_content_parent_open_pause_condition.wait_for(
        lock,
        std::chrono::seconds(10),
        [] {
            return package_content_parent_open_pause_entered;
        });
}

void release_package_content_parent_open_pause() {
    {
        std::lock_guard<std::mutex> lock(package_content_parent_open_pause_mutex);
        package_content_parent_open_pause_released = true;
    }
    package_content_parent_open_pause_condition.notify_all();
}

}  // namespace test_hooks
#endif

const char* build_configuration_name(BuildConfiguration configuration) {
    switch (configuration) {
        case BuildConfiguration::debug:
            return "debug";
        case BuildConfiguration::release:
            return "release";
    }
    return "debug";
}

BuildConfiguration parse_build_configuration(const std::string& value) {
    return trim_copy(value) == "release"
        ? BuildConfiguration::release
        : BuildConfiguration::debug;
}

const char* build_output_kind_name(BuildOutputKind output_kind) {
    switch (output_kind) {
        case BuildOutputKind::executable:
            return "executable";
        case BuildOutputKind::app:
            return "app";
        case BuildOutputKind::dll:
            return "dll";
        case BuildOutputKind::fll:
            return "fll";
        case BuildOutputKind::fxp:
            return "fxp";
        case BuildOutputKind::ocx:
            return "ocx";
        case BuildOutputKind::unknown:
            return "unknown";
    }
    return "unknown";
}


static RuntimeMaterializeResult materialize_runtime_package_in_fresh_root(
    const PackageRootTransaction& transaction,
    const RuntimePackagePlan& plan,
    RuntimePackagePlan filesystem_plan,
    const security::NativeSecurityProfile& security_profile,
    const platform::ExtensibilityProfile& extensibility_profile,
    const std::string& runtime_host_source_path) {
    const std::filesystem::path runtime_host_source =
        copperfin::platform::path_from_utf8_string(runtime_host_source_path);
    std::string error;
    if (!transaction.validate_parent_identity_for_materialization(error)) {
        return {.ok = false, .error = error};
    }
    std::error_code directory_error;
#if defined(_WIN32)
    std::filesystem::create_directories(
        copperfin::platform::path_from_utf8_string(filesystem_plan.package_root),
        directory_error);
    if (directory_error) {
        return {.ok = false, .error = runtime_text("Runtime.Package.Error.CreatePackageRootFailed")};
    }
#endif
    if (!transaction.validate_parent_identity_for_materialization(error)) {
        return {.ok = false, .error = error};
    }
    PackageParentIdentity content_identity;
    const std::filesystem::path content_root_path =
        copperfin::platform::path_from_utf8_string(filesystem_plan.content_root);
#if !defined(_WIN32)
    int content_descriptor = -1;
#endif
    if (!prepare_package_content_root(
            copperfin::platform::path_from_utf8_string(filesystem_plan.package_root),
            copperfin::platform::path_from_utf8_string(filesystem_plan.content_root),
            error,
#if !defined(_WIN32)
            &content_descriptor
#else
            nullptr
#endif
        )) {
        return {
            .ok = false,
            .error = error};
    }
#if !defined(_WIN32)
    const bool content_identity_acquired = content_descriptor >= 0
        ? content_identity.adopt_descriptor(content_descriptor, content_root_path)
        : content_identity.acquire(content_root_path);
#else
    const bool content_identity_acquired = content_identity.acquire(content_root_path);
#endif
    if (!content_identity_acquired) {
#if defined(COPPERFIN_ENABLE_RUNTIME_PIPELINE_TEST_HOOKS)
        trace_content_identity_failure(content_root_path);
#endif
        return {
            .ok = false,
            .error = runtime_text(
                "Runtime.Package.Error.CreateContentRootFailed")};
    }
    filesystem_plan.content_root = copperfin::platform::path_to_utf8_string(
        content_identity.stable_parent_path());
#if defined(COPPERFIN_ENABLE_RUNTIME_PIPELINE_TEST_HOOKS)
    {
        std::unique_lock<std::mutex> pause_lock(
            package_content_materialization_pause_mutex);
        if (package_content_materialization_pause_requested) {
            package_content_materialization_pause_entered = true;
            package_content_materialization_pause_condition.notify_all();
            package_content_materialization_pause_condition.wait(
                pause_lock,
                [] {
                    return package_content_materialization_pause_released;
                });
            package_content_materialization_pause_requested = false;
            package_content_materialization_pause_entered = false;
            package_content_materialization_pause_released = false;
        }
    }
#endif
    if (plan.emit_dotnet_launcher) {
        if (!transaction.validate_parent_identity_for_materialization(error)) {
            return {.ok = false, .error = error};
        }
#if defined(_WIN32)
        std::filesystem::create_directories(
            copperfin::platform::path_from_utf8_string(filesystem_plan.launcher_project_path).parent_path(),
            directory_error);
        if (directory_error) {
            return {.ok = false, .error = runtime_text("Runtime.Package.Error.CreateLauncherDirectoryFailed")};
        }
#else
        if (!transaction.create_pinned_child_directory(
                copperfin::platform::path_from_utf8_string(filesystem_plan.package_root),
                "launcher")) {
            return {.ok = false, .error = runtime_text("Runtime.Package.Error.CreateLauncherDirectoryFailed")};
        }
#endif
    }

    RuntimePackagePlan materialized_plan = plan;
    if (plan.planning_warnings_captured) {
        const std::size_t planning_warning_count = std::min(
            plan.planning_warning_count,
            plan.warnings.size());
        materialized_plan.warnings.assign(
            plan.warnings.begin(),
            plan.warnings.begin() + planning_warning_count);
    }
    materialized_plan.planning_warning_count = materialized_plan.warnings.size();
    materialized_plan.planning_warnings_captured = true;
    materialized_plan.primary_output_materialized = false;
    materialized_plan.launcher_artifacts.clear();
    materialized_plan.runtime_host_sha256.clear();
    materialized_plan.compiler_contract_digests.clear();
    materialized_plan.extension_payload_digests.clear();
    materialized_plan.writable_data_payload_digests.clear();
    materialized_plan.startup_source_path.clear();
    const auto logical_package_path = [&](const std::filesystem::path& physical_path) {
        const std::filesystem::path content_relative =
            physical_path.lexically_relative(
                copperfin::platform::path_from_utf8_string(filesystem_plan.content_root));
        if (!content_relative.empty() && !content_relative.is_absolute()) {
            bool escapes_content = false;
            for (const auto& component : content_relative) {
                if (component == "..") {
                    escapes_content = true;
                    break;
                }
            }
            if (!escapes_content) {
                return (copperfin::platform::path_from_utf8_string(plan.content_root) / content_relative)
                    .lexically_normal();
            }
        }
        const std::filesystem::path relative =
            physical_path.lexically_relative(
                copperfin::platform::path_from_utf8_string(filesystem_plan.package_root));
        if (!relative.empty() && !relative.is_absolute()) {
            bool escapes = false;
            for (const auto& component : relative) {
                if (component == "..") {
                    escapes = true;
                    break;
                }
            }
            if (!escapes) {
                return (copperfin::platform::path_from_utf8_string(plan.package_root) / relative)
                    .lexically_normal();
            }
        }
        return physical_path;
    };
    const auto sha256_for_materialized_file =
        [&](const std::filesystem::path& physical_path) {
#if !defined(_WIN32)
            std::string contents;
            bool fd_read_handled = false;
            if (try_read_file_fd_backed(
                    physical_path,
                    fd_read_handled,
                    contents) &&
                fd_read_handled) {
                return security::sha256_hex_for_text(contents);
            }
#endif
            return security::sha256_hex_for_file(
                copperfin::platform::path_to_utf8_string(physical_path));
        };
    const auto append_pinned_digest =
        [&](std::vector<RuntimeArtifactDigest>& digests,
            const std::filesystem::path& physical_path,
            std::string& digest_error) {
            if (physical_path.empty()) {
                return true;
            }
#if !defined(_WIN32)
            bool fd_source_handled = false;
            std::string fd_source_contents;
            const bool fd_source_readable = try_read_file_fd_backed(
                physical_path,
                fd_source_handled,
                fd_source_contents);
            if (fd_source_handled && !fd_source_readable) {
                digest_error = runtime_text(
                    "Runtime.Package.Error.OpenFileFailed",
                    {{"path", copperfin::platform::path_to_utf8_string(physical_path)}});
                return false;
            }
            if (!fd_source_handled && !path_exists_without_error(physical_path)) {
                return true;
            }
#else
            if (!path_exists_without_error(physical_path)) {
                return true;
            }
#endif
            const auto digest = sha256_for_materialized_file(physical_path);
            if (!digest.ok) {
                digest_error = digest.error;
                return false;
            }
            const std::string logical_path = copperfin::platform::path_to_utf8_string(
                logical_package_path(physical_path));
            const auto existing = std::find_if(
                digests.begin(),
                digests.end(),
                [&](const RuntimeArtifactDigest& entry) {
                    return entry.path == logical_path;
                });
            if (existing != digests.end()) {
                existing->sha256 = digest.hex_digest;
            } else {
                digests.push_back({.path = logical_path, .sha256 = digest.hex_digest});
            }
            return true;
        };
    for (auto& asset : materialized_plan.assets) {
        asset.staged_path.clear();
        asset.copied = false;
        asset.sha256.clear();
        asset.exists =
            asset.source_resolution_error.empty() &&
            !asset.source_path.empty() &&
            source_path_exists_on_host(asset.source_path);
        if (asset.required_for_runtime && !asset.source_resolution_error.empty()) {
            return {.ok = false, .error = asset.source_resolution_error};
        }
        if (asset.required_for_runtime && !asset.exists) {
            return {
                .ok = false,
                .error = runtime_text(
                    "Runtime.Package.Error.SourceFileMissing",
                    {{"path", asset.source_path}})};
        }
        if (!should_stage_asset(asset)) {
            continue;
        }

        if (!transaction.validate_parent_identity_for_materialization(error)) {
            return {.ok = false, .error = error};
        }

        std::filesystem::path destination;
        if (!copy_file_to_package_content(
                copperfin::platform::path_from_utf8_string(asset.source_path),
                copperfin::platform::path_from_utf8_string(filesystem_plan.package_root),
                copperfin::platform::path_from_utf8_string(filesystem_plan.content_root),
                copperfin::platform::path_from_utf8_string(asset.relative_path),
                destination,
                error)) {
            if (asset.required_for_runtime) {
                return {.ok = false, .error = error};
            }
            if (destination.empty()) {
                asset.relative_path =
                    "record_" + std::to_string(asset.record_index) + ".asset";
                asset.staged_path.clear();
            } else {
                asset.staged_path = copperfin::platform::path_to_utf8_string(
                    logical_package_path(destination));
            }
            materialized_plan.warnings.push_back(error);
            continue;
        }
        asset.staged_path = copperfin::platform::path_to_utf8_string(
            logical_package_path(destination));
        if (asset.required_for_runtime) {
            materialized_plan.startup_source_path = asset.staged_path;
        }
        if (!transaction.validate_parent_identity_for_materialization(error)) {
            return {.ok = false, .error = error};
        }
        const auto companion_copy_result =
            copy_companion_files_if_present(
                asset,
                copperfin::platform::path_from_utf8_string(filesystem_plan.package_root),
                copperfin::platform::path_from_utf8_string(filesystem_plan.content_root),
                materialized_plan.warnings);
        if (!companion_copy_result.ok) {
            return {.ok = false, .error = companion_copy_result.error};
        }
        for (const auto& companion : companion_copy_result.copied_paths) {
            auto& digest_surface = asset.package_writable
                ? materialized_plan.writable_data_payload_digests
                : materialized_plan.extension_payload_digests;
            if (!append_pinned_digest(
                    digest_surface,
                    companion,
                    error)) {
                return {.ok = false, .error = error};
            }
        }
        asset.copied = true;

        const auto digest = sha256_for_materialized_file(destination);
        if (!digest.ok) {
            return {.ok = false, .error = digest.error};
        }
        asset.sha256 = digest.hex_digest;

        if (is_extension_payload_path(destination)) {
            materialized_plan.extension_payload_digests.push_back({
                .path = copperfin::platform::path_to_utf8_string(
                    logical_package_path(destination)),
                .sha256 = digest.hex_digest
            });
        }
    }

    if (is_library_output_kind(plan.output_kind)) {
        if (!transaction.validate_parent_identity_for_materialization(error)) {
            return {.ok = false, .error = error};
        }
        if (!copy_file_if_exists(
                runtime_host_source,
                filesystem_plan.runtime_host_destination_path,
                error)) {
            return {.ok = false, .error = error};
        }
#if !defined(_WIN32)
        std::error_code runtime_host_permissions_error;
        const auto runtime_host_source_status = std::filesystem::status(
            runtime_host_source,
            runtime_host_permissions_error);
        if (runtime_host_permissions_error) {
            return {.ok = false,
                    .error = runtime_text(
                        "Runtime.Package.Error.CopyFileFailed",
                        {{"path", filesystem_plan.runtime_host_destination_path}})};
        }
        const auto executable_permissions = runtime_host_source_status.permissions() &
            (std::filesystem::perms::owner_exec |
             std::filesystem::perms::group_exec |
             std::filesystem::perms::others_exec);
        if (executable_permissions != std::filesystem::perms::none &&
            !is_fd_backed_runtime_path(copperfin::platform::path_from_utf8_string(
                filesystem_plan.runtime_host_destination_path))) {
            std::filesystem::permissions(
                copperfin::platform::path_from_utf8_string(
                    filesystem_plan.runtime_host_destination_path),
                executable_permissions,
                std::filesystem::perm_options::add,
                runtime_host_permissions_error);
            if (runtime_host_permissions_error) {
                return {.ok = false,
                        .error = runtime_text(
                            "Runtime.Package.Error.CopyFileFailed",
                            {{"path", filesystem_plan.runtime_host_destination_path}})};
            }
        }
#endif
        if (!append_pinned_digest(
                materialized_plan.extension_payload_digests,
                copperfin::platform::path_from_utf8_string(
                    filesystem_plan.runtime_host_destination_path),
                error)) {
            return {.ok = false, .error = error};
        }
        const auto runtime_host_digest = sha256_for_materialized_file(
            copperfin::platform::path_from_utf8_string(
                filesystem_plan.runtime_host_destination_path));
        if (!runtime_host_digest.ok) {
            return {.ok = false, .error = runtime_host_digest.error};
        }
        materialized_plan.runtime_host_sha256 = runtime_host_digest.hex_digest;

        if (!transaction.validate_parent_identity_for_materialization(error)) {
            return {.ok = false, .error = error};
        }
#if defined(_WIN32)
        std::filesystem::create_directories(
            copperfin::platform::path_from_utf8_string(filesystem_plan.native_wrapper_source_path).parent_path(),
            directory_error);
        if (directory_error) {
            return {.ok = false, .error = runtime_text("Runtime.Package.Error.CreateNativeWrapperDirectoryFailed")};
        }
#else
        if (!transaction.create_pinned_child_directory(
                copperfin::platform::path_from_utf8_string(filesystem_plan.package_root),
                "wrapper")) {
            return {.ok = false, .error = runtime_text("Runtime.Package.Error.CreateNativeWrapperDirectoryFailed")};
        }
#endif
        if (!write_text_file(
                filesystem_plan.module_definition_path,
                build_module_definition_source(materialized_plan),
                error)) {
            return {.ok = false, .error = error};
        }
        if (!append_pinned_digest(
                materialized_plan.compiler_contract_digests,
                filesystem_plan.module_definition_path,
                error)) {
            return {.ok = false, .error = error};
        }
        if (!write_text_file(
                filesystem_plan.native_wrapper_source_path,
                build_native_wrapper_source(materialized_plan),
                error)) {
            return {.ok = false, .error = error};
        }
        if (!append_pinned_digest(
                materialized_plan.compiler_contract_digests,
                filesystem_plan.native_wrapper_source_path,
                error)) {
            return {.ok = false, .error = error};
        }
        if (!write_text_file(
                filesystem_plan.native_wrapper_cmake_path,
                build_native_wrapper_cmake_source(materialized_plan),
                error)) {
            return {.ok = false, .error = error};
        }
        if (!append_pinned_digest(
                materialized_plan.compiler_contract_digests,
                copperfin::platform::path_from_utf8_string(
                    filesystem_plan.native_wrapper_cmake_path),
                error)) {
            return {.ok = false, .error = error};
        }
        if (!write_text_file(
                filesystem_plan.native_wrapper_build_script_path,
                build_native_wrapper_shell_script_source(),
                error)) {
            return {.ok = false, .error = error};
        }
        if (!append_pinned_digest(
                materialized_plan.compiler_contract_digests,
                copperfin::platform::path_from_utf8_string(
                    filesystem_plan.native_wrapper_build_script_path),
                error)) {
            return {.ok = false, .error = error};
        }
        if (!write_text_file(
                filesystem_plan.native_wrapper_build_powershell_path,
                build_native_wrapper_powershell_script_source(),
                error)) {
            return {.ok = false, .error = error};
        }
        if (!append_pinned_digest(
                materialized_plan.compiler_contract_digests,
                copperfin::platform::path_from_utf8_string(
                    filesystem_plan.native_wrapper_build_powershell_path),
                error)) {
            return {.ok = false, .error = error};
        }
        if (plan.output_kind == BuildOutputKind::dll || plan.output_kind == BuildOutputKind::ocx) {
            if (!write_text_file(
                    filesystem_plan.library_api_manifest_path,
                    build_library_api_manifest_source(materialized_plan),
                    error)) {
                return {.ok = false, .error = error};
            }
            if (!append_pinned_digest(
                    materialized_plan.compiler_contract_digests,
                    copperfin::platform::path_from_utf8_string(
                        filesystem_plan.library_api_manifest_path),
                    error)) {
                return {.ok = false, .error = error};
            }
        }
        if (plan.output_kind == BuildOutputKind::fll) {
            if (!write_text_file(
                    filesystem_plan.fll_api_manifest_path,
                    build_fll_api_manifest_source(materialized_plan),
                    error)) {
                return {.ok = false, .error = error};
            }
            if (!append_pinned_digest(
                    materialized_plan.compiler_contract_digests,
                    copperfin::platform::path_from_utf8_string(
                        filesystem_plan.fll_api_manifest_path),
                    error)) {
                return {.ok = false, .error = error};
            }
        }
    } else if (plan.output_kind == BuildOutputKind::fxp) {
        if (!transaction.validate_parent_identity_for_materialization(error)) {
            return {.ok = false, .error = error};
        }
        const std::string fxp_token_manifest = build_fxp_token_manifest_source(materialized_plan);
        if (!write_text_file(filesystem_plan.fxp_token_manifest_path, fxp_token_manifest, error)) {
            return {.ok = false, .error = error};
        }
        if (!append_pinned_digest(
                materialized_plan.compiler_contract_digests,
                copperfin::platform::path_from_utf8_string(
                    filesystem_plan.fxp_token_manifest_path),
                error)) {
            return {.ok = false, .error = error};
        }
        if (!write_fxp_primary_output_contract(
                materialized_plan,
                fxp_token_manifest,
                filesystem_plan.launcher_output_path,
                error)) {
            return {.ok = false, .error = error};
        }
        if (!append_pinned_digest(
                materialized_plan.extension_payload_digests,
                copperfin::platform::path_from_utf8_string(
                    filesystem_plan.launcher_output_path),
                error)) {
            return {.ok = false, .error = error};
        }
        materialized_plan.primary_output_materialized = true;
    } else if (plan.output_kind == BuildOutputKind::app) {
        if (!transaction.validate_parent_identity_for_materialization(error)) {
            return {.ok = false, .error = error};
        }
        if (!write_text_file(
                filesystem_plan.app_archive_manifest_path,
                build_app_archive_manifest_source(materialized_plan),
                error)) {
            return {.ok = false, .error = error};
        }
        if (!append_pinned_digest(
                materialized_plan.compiler_contract_digests,
                filesystem_plan.app_archive_manifest_path,
                error)) {
            return {.ok = false, .error = error};
        }
        RuntimePackagePlan filesystem_materialized_plan = materialized_plan;
        filesystem_materialized_plan.package_root = filesystem_plan.package_root;
        filesystem_materialized_plan.content_root = filesystem_plan.content_root;
        filesystem_materialized_plan.app_archive_manifest_path =
            filesystem_plan.app_archive_manifest_path;
        filesystem_materialized_plan.launcher_output_path =
            filesystem_plan.launcher_output_path;
        for (auto& asset : filesystem_materialized_plan.assets) {
            if (!asset.staged_path.empty()) {
                asset.staged_path = copperfin::platform::path_to_utf8_string((
                    copperfin::platform::path_from_utf8_string(filesystem_plan.content_root) /
                    copperfin::platform::path_from_utf8_string(asset.relative_path)).lexically_normal());
            }
        }
        if (!write_app_archive_primary_output(
                materialized_plan,
                filesystem_materialized_plan,
                error)) {
            return {.ok = false, .error = error};
        }
        if (!append_pinned_digest(
                materialized_plan.extension_payload_digests,
                copperfin::platform::path_from_utf8_string(
                    filesystem_plan.launcher_output_path),
                error)) {
            return {.ok = false, .error = error};
        }
        materialized_plan.primary_output_materialized = true;
    } else {
        if (!transaction.validate_parent_identity_for_materialization(error)) {
            return {.ok = false, .error = error};
        }
        if (!copy_file_if_exists(
                runtime_host_source,
                filesystem_plan.runtime_host_destination_path,
                error)) {
            return {.ok = false, .error = error};
        }
#if !defined(_WIN32)
        std::error_code runtime_host_permissions_error;
        const auto runtime_host_source_status = std::filesystem::status(
            runtime_host_source,
            runtime_host_permissions_error);
        if (runtime_host_permissions_error) {
            return {.ok = false,
                    .error = runtime_text(
                        "Runtime.Package.Error.CopyFileFailed",
                        {{"path", filesystem_plan.runtime_host_destination_path}})};
        }
        const auto executable_permissions = runtime_host_source_status.permissions() &
            (std::filesystem::perms::owner_exec |
             std::filesystem::perms::group_exec |
             std::filesystem::perms::others_exec);
        if (executable_permissions != std::filesystem::perms::none &&
            !is_fd_backed_runtime_path(copperfin::platform::path_from_utf8_string(
                filesystem_plan.runtime_host_destination_path))) {
            std::filesystem::permissions(
                copperfin::platform::path_from_utf8_string(
                    filesystem_plan.runtime_host_destination_path),
                executable_permissions,
                std::filesystem::perm_options::add,
                runtime_host_permissions_error);
            if (runtime_host_permissions_error) {
                return {.ok = false,
                        .error = runtime_text(
                            "Runtime.Package.Error.CopyFileFailed",
                            {{"path", filesystem_plan.runtime_host_destination_path}})};
            }
        }
#endif

        const auto runtime_host_digest = sha256_for_materialized_file(
            copperfin::platform::path_from_utf8_string(
                filesystem_plan.runtime_host_destination_path));
        if (!runtime_host_digest.ok) {
            return {.ok = false, .error = runtime_host_digest.error};
        }
        materialized_plan.runtime_host_sha256 = runtime_host_digest.hex_digest;
        materialized_plan.extension_payload_digests.push_back({
            .path = copperfin::platform::path_to_utf8_string(
                logical_package_path(copperfin::platform::path_from_utf8_string(
                    filesystem_plan.runtime_host_destination_path))),
            .sha256 = runtime_host_digest.hex_digest
        });

        if (!plan.emit_dotnet_launcher) {
            if (!copy_file_if_exists(
                    copperfin::platform::path_from_utf8_string(
                        filesystem_plan.runtime_host_destination_path),
                    copperfin::platform::path_from_utf8_string(
                        filesystem_plan.launcher_output_path),
                    error)) {
                return {.ok = false, .error = error};
            }

            const auto native_entrypoint_digest = sha256_for_materialized_file(
                copperfin::platform::path_from_utf8_string(
                    filesystem_plan.launcher_output_path));
            if (!native_entrypoint_digest.ok) {
                return {.ok = false, .error = native_entrypoint_digest.error};
            }
            materialized_plan.extension_payload_digests.push_back({
            .path = copperfin::platform::path_to_utf8_string(
                logical_package_path(copperfin::platform::path_from_utf8_string(
                    filesystem_plan.launcher_output_path))),
                .sha256 = native_entrypoint_digest.hex_digest
            });
            materialized_plan.primary_output_materialized = true;
        }
    }

    if (plan.emit_dotnet_launcher) {
        if (!transaction.validate_parent_identity_for_materialization(error)) {
            return {.ok = false, .error = error};
        }
        if (!write_text_file(
                filesystem_plan.launcher_project_path,
                build_launcher_project_source(plan),
                error)) {
            return {.ok = false, .error = error};
        }
        if (!write_text_file(
                filesystem_plan.launcher_source_path,
                build_launcher_program_source(plan),
                error)) {
            return {.ok = false, .error = error};
        }
    }

    if (!transaction.validate_parent_identity_for_materialization(error) ||
        !write_text_file(
            filesystem_plan.ast_manifest_path,
            build_ast_manifest_source(materialized_plan),
            error)) {
        return {.ok = false, .error = error};
    }
    if (!append_pinned_digest(
                materialized_plan.compiler_contract_digests,
                copperfin::platform::path_from_utf8_string(
                    filesystem_plan.ast_manifest_path),
            error)) {
        return {.ok = false, .error = error};
    }
    if (!transaction.validate_parent_identity_for_materialization(error) ||
        !write_text_file(
            filesystem_plan.ir_manifest_path,
            build_ir_manifest_source(materialized_plan),
            error)) {
        return {.ok = false, .error = error};
    }
    if (!append_pinned_digest(
                materialized_plan.compiler_contract_digests,
                copperfin::platform::path_from_utf8_string(
                    filesystem_plan.ir_manifest_path),
            error)) {
        return {.ok = false, .error = error};
    }
    if (plan.requested_dotnet_launcher &&
        (!transaction.validate_parent_identity_for_materialization(error) ||
         !write_text_file(
             filesystem_plan.transpiled_csharp_path,
             build_csharp_transpilation_source(materialized_plan),
             error))) {
        return {.ok = false, .error = error};
    }
    if (plan.requested_dotnet_launcher &&
        !append_pinned_digest(
            materialized_plan.compiler_contract_digests,
            copperfin::platform::path_from_utf8_string(
                filesystem_plan.transpiled_csharp_path),
            error)) {
        return {.ok = false, .error = error};
    }
    if (!transaction.validate_parent_identity_for_materialization(error) ||
        !write_text_file(
            filesystem_plan.manifest_path,
            build_runtime_manifest_text(materialized_plan, security_profile, extensibility_profile),
            error)) {
        return {.ok = false, .error = error};
    }
    if (!transaction.validate_parent_identity_for_materialization(error) ||
        !write_text_file(
            filesystem_plan.debug_manifest_path,
            build_debug_manifest_text(materialized_plan, security_profile, extensibility_profile),
            error)) {
        return {.ok = false, .error = error};
    }

    return {.ok = true, .plan = std::move(materialized_plan), .error = {}};
}

RuntimeMaterializeResult materialize_runtime_package(
    const RuntimePackagePlan& plan,
    const security::NativeSecurityProfile& security_profile,
    const platform::ExtensibilityProfile& extensibility_profile,
    const std::string& runtime_host_source_path) {
    std::string error;
    if (!validate_public_output_artifact_name(plan, error)) {
        return {.ok = false, .error = error};
    }
    if (!plan.ok) {
        return {.ok = false, .error = runtime_text("Runtime.Package.Error.PlanInvalid")};
    }
    if ((is_native_host_output_kind(plan.output_kind) || is_library_output_kind(plan.output_kind)) &&
        !validate_runtime_host_source_path(plan, runtime_host_source_path, error)) {
        return {.ok = false, .error = error};
    }

    PackageRootTransaction transaction(
        copperfin::platform::path_from_utf8_string(plan.package_root));
    if (!transaction.begin(error)) {
        return {.ok = false, .error = error};
    }

#if defined(COPPERFIN_ENABLE_RUNTIME_PIPELINE_TEST_HOOKS)
    {
        std::unique_lock<std::mutex> pause_lock(package_materialization_pause_mutex);
        if (package_materialization_pause_requested) {
            package_materialization_pause_entered = true;
            package_materialization_pause_condition.notify_all();
            package_materialization_pause_condition.wait(
                pause_lock,
                [] {
                    return package_materialization_pause_released;
                });
            package_materialization_pause_requested = false;
            package_materialization_pause_entered = false;
            package_materialization_pause_released = false;
        }
    }
#endif

    RuntimeMaterializeResult result = materialize_runtime_package_in_fresh_root(
        transaction,
        plan,
        transaction.pinned_filesystem_plan(plan),
        security_profile,
        extensibility_profile,
        runtime_host_source_path);
    if (!result.ok) {
        std::string rollback_error;
        if (!transaction.rollback(rollback_error)) {
            result.error = rollback_error + "\n" + result.error;
        }
        return result;
    }

    std::string commit_error;
    std::string cleanup_warning;
    const bool transaction_deferred =
        plan.emit_dotnet_launcher || is_library_output_kind(plan.output_kind);
    if (transaction_deferred) {
        if (!transaction.defer_until_primary_output(commit_error)) {
            std::string rollback_error;
            if (!transaction.rollback(rollback_error)) {
                commit_error = rollback_error + "\n" + commit_error;
            }
            return {.ok = false, .error = commit_error};
        }
    } else if (!transaction.commit(commit_error, cleanup_warning)) {
        std::string rollback_error;
        if (!transaction.rollback(rollback_error)) {
            commit_error = rollback_error + "\n" + commit_error;
        }
        return {.ok = false, .error = commit_error};
    }
    if (!cleanup_warning.empty()) {
        result.plan.warnings.push_back(cleanup_warning);
        if (!write_runtime_manifest_pair_atomically(
            transaction.pinned_filesystem_plan(result.plan),
            build_runtime_manifest_text(result.plan, security_profile, extensibility_profile),
            build_debug_manifest_text(result.plan, security_profile, extensibility_profile),
            error)) {
            result.plan.warnings.push_back(
                runtime_text(
                    "Runtime.Package.Warning.ManifestPairRewriteFailed",
                    {{"path", result.plan.package_root}}) +
                " " + error);
            return {.ok = true, .plan = std::move(result.plan), .error = {}};
        }
    }

    return result;
}

RuntimeBuildResult finalize_runtime_package_primary_output(
    const RuntimePackagePlan& plan,
    const security::NativeSecurityProfile& security_profile,
    const platform::ExtensibilityProfile& extensibility_profile) {
    std::string error;
    if (!validate_public_output_artifact_name(plan, error)) {
        return {.ok = false, .error = error};
    }
    if (!plan.ok) {
        return {.ok = false, .error = runtime_text("Runtime.Package.Error.PlanInvalid")};
    }
    std::error_code primary_output_exists_error;
    if (!plan.emit_dotnet_launcher &&
        (!std::filesystem::exists(
             copperfin::platform::path_from_utf8_string(plan.launcher_output_path),
             primary_output_exists_error) ||
         primary_output_exists_error)) {
        return {.ok = false, .error = runtime_text("Runtime.Package.Error.PrimaryOutputMissing")};
    }

    std::optional<PackageRootTransaction> package_transaction;
    if ((plan.emit_dotnet_launcher || is_library_output_kind(plan.output_kind)) &&
        !plan.primary_output_materialized) {
        package_transaction.emplace(
            copperfin::platform::path_from_utf8_string(plan.package_root),
            PackageRootTransaction::Mode::resume_deferred);
        if (!package_transaction->begin(error)) {
            return {.ok = false, .error = error};
        }
    }

    const auto rollback_package_transaction = [&](std::string& operation_error) {
        if (!package_transaction) {
            return;
        }
        std::string rollback_error;
        if (!package_transaction->rollback(rollback_error)) {
            operation_error = rollback_error + "\n" + operation_error;
        }
    };

    RuntimePackagePlan finalized_plan = plan;
    if (plan.emit_dotnet_launcher) {
        std::erase_if(
            finalized_plan.extension_payload_digests,
            [&](const RuntimeArtifactDigest& digest) {
                return is_launcher_owned_digest(digest, plan);
            });
        if (!inventory_generated_launcher_artifacts(
                plan,
                finalized_plan.launcher_artifacts,
                error)) {
            rollback_package_transaction(error);
            return {.ok = false, .error = error};
        }
    } else {
        finalized_plan.launcher_artifacts.clear();
        std::erase_if(
            finalized_plan.extension_payload_digests,
            [&](const RuntimeArtifactDigest& digest) {
                return digest.path == plan.launcher_output_path;
            });
        if (!append_runtime_artifact_digest(
                finalized_plan.extension_payload_digests,
                plan.launcher_output_path,
                error)) {
            rollback_package_transaction(error);
            return {.ok = false, .error = error};
        }
    }
    finalized_plan.primary_output_materialized = true;
    if (!write_runtime_manifest_pair_atomically(
            finalized_plan,
            build_runtime_manifest_text(finalized_plan, security_profile, extensibility_profile),
            build_debug_manifest_text(finalized_plan, security_profile, extensibility_profile),
            error)) {
        rollback_package_transaction(error);
        return {.ok = false, .error = error};
    }

    if (package_transaction) {
        std::string commit_warning;
        if (!package_transaction->commit(error, commit_warning)) {
            rollback_package_transaction(error);
            return {.ok = false, .error = error};
        }
        if (!commit_warning.empty()) {
            finalized_plan.warnings.push_back(commit_warning);
            std::string warning_rewrite_error;
            if (!write_runtime_manifest_pair_atomically(
                    finalized_plan,
                    build_runtime_manifest_text(
                        finalized_plan,
                        security_profile,
                        extensibility_profile),
                    build_debug_manifest_text(
                        finalized_plan,
                        security_profile,
                        extensibility_profile),
                    warning_rewrite_error)) {
                finalized_plan.warnings.push_back(
                    runtime_text(
                        "Runtime.Package.Warning.ManifestPairRewriteFailed",
                        {{"path", finalized_plan.package_root}}) +
                    " " + warning_rewrite_error);
            }
        }
    }

    return {.ok = true, .plan = std::move(finalized_plan), .error = {}};
}

RuntimeBuildResult abort_runtime_package_transaction(
    const RuntimePackagePlan& plan) {
    std::string error;
    if (!validate_public_output_artifact_name(plan, error)) {
        return {.ok = false, .error = error};
    }
    if (!plan.ok) {
        return {.ok = false, .error = runtime_text("Runtime.Package.Error.PlanInvalid")};
    }
    if (!plan.emit_dotnet_launcher && !is_library_output_kind(plan.output_kind)) {
        return {.ok = true, .plan = plan, .error = {}};
    }

    PackageRootTransaction transaction(
        copperfin::platform::path_from_utf8_string(plan.package_root),
        PackageRootTransaction::Mode::resume_deferred);
    if (!transaction.begin(error)) {
        return {.ok = false, .error = error};
    }
    if (!transaction.rollback(error)) {
        return {.ok = false, .error = error};
    }
    return {.ok = true, .plan = plan, .error = {}};
}

RuntimeBuildResult build_runtime_package_primary_output(
    const RuntimePackagePlan& plan,
    const security::NativeSecurityProfile& security_profile,
    const platform::ExtensibilityProfile& extensibility_profile) {
    std::string error;
    if (!validate_public_output_artifact_name(plan, error)) {
        return {.ok = false, .error = error};
    }
    if (!plan.ok) {
        return {.ok = false, .error = runtime_text("Runtime.Package.Error.PlanInvalid")};
    }
    if (!is_library_output_kind(plan.output_kind)) {
        return {.ok = false, .error = runtime_text("Runtime.Package.Error.PrimaryOutputRequiresLibraryOutput")};
    }
    if (!path_exists_without_error(
            copperfin::platform::path_from_utf8_string(plan.native_wrapper_cmake_path))) {
        return {.ok = false, .error = runtime_text("Runtime.Package.Error.NativeWrapperCMakeMissing")};
    }

    RuntimePackagePlan built_plan = plan;
    const std::filesystem::path source_root =
        copperfin::platform::path_from_utf8_string(plan.native_wrapper_cmake_path).parent_path();
    const std::filesystem::path original_build_root = source_root / "cmake_pipeline_build";
    const std::filesystem::path staging_root =
        std::filesystem::temp_directory_path() /
        ("copperfin-native-wrapper-" +
#if defined(_WIN32)
            std::to_string(static_cast<unsigned long long>(::GetCurrentProcessId())) +
#else
            std::to_string(static_cast<unsigned long long>(::getpid())) +
#endif
            "-" +
            std::to_string(++native_wrapper_build_sequence));
    const std::filesystem::path staging_package_root = staging_root / "package";
    const std::filesystem::path staging_source_root = staging_package_root / "wrapper";
    const std::filesystem::path build_root = staging_source_root / "cmake_pipeline_build";
    const std::filesystem::path configure_log_path = build_root / "cmake-configure.log";
    const std::filesystem::path build_log_path = build_root / "cmake-build.log";
    const std::filesystem::path staged_output_path = staging_package_root /
        copperfin::platform::path_from_utf8_string(plan.launcher_output_path).filename();
    // CMake-generated Makefiles interpret shell-like path text; build from a
    // private safe-path copy and publish only the requested primary artifact.
    std::error_code ignored;
    std::filesystem::remove_all(original_build_root, ignored);
    std::filesystem::remove_all(staging_root, ignored);
    std::filesystem::remove(
        copperfin::platform::path_from_utf8_string(plan.launcher_output_path),
        ignored);
    std::filesystem::create_directories(staging_package_root, ignored);
    std::filesystem::copy(
        source_root,
        staging_source_root,
        std::filesystem::copy_options::recursive |
            std::filesystem::copy_options::overwrite_existing,
        ignored);
    if (!ignored) {
        std::filesystem::create_directories(build_root, ignored);
    }
    if (!ignored) {
        const std::filesystem::path module_definition_path =
            copperfin::platform::path_from_utf8_string(plan.module_definition_path);
        if (path_exists_without_error(module_definition_path)) {
            std::filesystem::copy_file(
                module_definition_path,
                staging_package_root / module_definition_path.filename(),
                std::filesystem::copy_options::overwrite_existing,
                ignored);
        }
    }
    if (ignored) {
        return {.ok = false, .error = runtime_text("Runtime.Package.Error.CreateNativeWrapperBuildDirectoryFailed")};
    }

    const NativeWrapperProcessResult configure_result = run_native_wrapper_process(
#if defined(_WIN32)
        "cmake.exe",
#else
        "cmake",
#endif
        {
            "-S",
            copperfin::platform::path_to_utf8_string(staging_source_root),
            "-B",
            copperfin::platform::path_to_utf8_string(build_root)},
        configure_log_path);
    if (!configure_result.started || configure_result.exit_code != 0) {
        error = runtime_text("Runtime.Package.Error.NativeWrapperPrimaryOutputConfigureFailed");
        if (path_exists_without_error(configure_log_path)) {
            error += ":\n" + read_text_file(configure_log_path);
        }
        std::filesystem::remove_all(staging_root, ignored);
        return {.ok = false, .error = error};
    }

    const NativeWrapperProcessResult build_result = run_native_wrapper_process(
#if defined(_WIN32)
        "cmake.exe",
#else
        "cmake",
#endif
        {
            "--build",
            copperfin::platform::path_to_utf8_string(build_root)},
        build_log_path);
    if (!build_result.started || build_result.exit_code != 0) {
        error = runtime_text("Runtime.Package.Error.NativeWrapperPrimaryOutputBuildFailed");
        if (path_exists_without_error(build_log_path)) {
            error += ":\n" + read_text_file(build_log_path);
        }
        std::filesystem::remove_all(staging_root, ignored);
        return {.ok = false, .error = error};
    }

    if (!path_exists_without_error(staged_output_path)) {
        std::filesystem::remove_all(staging_root, ignored);
        return {.ok = false, .error = runtime_text("Runtime.Package.Error.NativeWrapperPrimaryOutputMissing")};
    }

    std::error_code copy_error;
    std::filesystem::copy_file(
        staged_output_path,
        copperfin::platform::path_from_utf8_string(plan.launcher_output_path),
        std::filesystem::copy_options::overwrite_existing,
        copy_error);
    std::error_code cleanup_error;
    std::filesystem::remove_all(staging_root, cleanup_error);
    if (copy_error) {
        return {.ok = false, .error = runtime_text("Runtime.Package.Error.NativeWrapperPrimaryOutputMissing")};
    }

    return finalize_runtime_package_primary_output(
        built_plan,
        security_profile,
        extensibility_profile);
}

}  // namespace copperfin::runtime
