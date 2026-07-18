// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "runtime_pipeline_support.h"

#if defined(COPPERFIN_ENABLE_RUNTIME_PIPELINE_TEST_HOOKS)
#include "runtime_pipeline_test_hooks.h"
#endif

#include <atomic>
#include <cerrno>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <optional>
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
#include <unistd.h>
#endif

namespace copperfin::runtime {
namespace {

constexpr int kRuntimeManifestVersion = 3;
constexpr int kDebugManifestVersion = 3;
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

    bool write_text_file_atomically(
        const std::filesystem::path& path,
        const std::string& contents,
        std::string& error) const {
        if (descriptor_ < 0) {
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
                error = runtime_text(
                    "Runtime.Package.Error.PackageTransactionStartFailed",
                    {{"path", copperfin::platform::path_to_utf8_string(package_root_)}});
                return false;
            }
        }
        if (!parent_identity_.acquire(parent)) {
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
            error = runtime_text(
                "Runtime.Package.Error.PackageTransactionStartFailed",
                {{"path", copperfin::platform::path_to_utf8_string(package_root_)}});
            return false;
        }
        if (!parent_identity_.still_same()) {
            error = runtime_text(
                "Runtime.Package.Error.PackageTransactionStartFailed",
                {{"path", copperfin::platform::path_to_utf8_string(parent)}});
            return false;
        }

        bool interrupted_backup_exists =
            directory_entry_exists(pinned_path(backup_root_), filesystem_error);
        if (filesystem_error) {
            error = runtime_text(
                "Runtime.Package.Error.PackageTransactionStartFailed",
                {{"path", copperfin::platform::path_to_utf8_string(backup_root_)}});
            return false;
        }
        bool package_exists = directory_entry_exists(pinned_path(package_root_), filesystem_error);
        if (filesystem_error) {
            error = runtime_text(
                "Runtime.Package.Error.PackageTransactionStartFailed",
                {{"path", copperfin::platform::path_to_utf8_string(package_root_)}});
            return false;
        }
        bool transaction_marker_exists = directory_entry_exists(pinned_path(marker_path_), filesystem_error);
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
        bool backup_owner_exists = directory_entry_exists(pinned_path(backup_owner_path_), filesystem_error);
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
            std::filesystem::remove(pinned_path(backup_owner_path_), filesystem_error);
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
            if (!transaction_marker_exists ||
                read_text_file(pinned_path(marker_path_)) !=
                    transaction_identity_ + std::string(kPackageTransactionDeferredPhase) ||
                !package_exists ||
                !is_direct_directory(pinned_path(package_root_), filesystem_error) ||
                filesystem_error) {
                error = runtime_text(
                    "Runtime.Package.Error.PackageTransactionStartFailed",
                    {{"path", copperfin::platform::path_to_utf8_string(package_root_)}});
                return false;
            }
            if (interrupted_backup_exists &&
                (!is_direct_directory(pinned_path(backup_root_), filesystem_error) ||
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
            if (!is_direct_directory(pinned_path(backup_root_), filesystem_error) || filesystem_error) {
                error = runtime_text(
                    "Runtime.Package.Error.PackageTransactionStartFailed",
                    {{"path", copperfin::platform::path_to_utf8_string(backup_root_)}});
                return false;
            }
            bool partial_package = false;
            if (package_exists) {
                const bool package_is_directory =
                    std::filesystem::is_directory(pinned_path(package_root_), filesystem_error);
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
                std::filesystem::remove_all(pinned_path(package_root_), filesystem_error);
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
                std::filesystem::remove_all(pinned_path(backup_root_), filesystem_error);
                if (filesystem_error) {
                    error = runtime_text(
                        "Runtime.Package.Error.PackageTransactionStartFailed",
                        {{"path", copperfin::platform::path_to_utf8_string(backup_root_)}});
                    return false;
                }
                std::filesystem::remove(pinned_path(backup_owner_path_), filesystem_error);
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
                std::filesystem::remove_all(pinned_path(package_root_), filesystem_error);
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
            std::filesystem::remove(pinned_path(marker_path_), filesystem_error);
            if (filesystem_error) {
                error = runtime_text(
                    "Runtime.Package.Error.PackageTransactionStartFailed",
                    {{"path", copperfin::platform::path_to_utf8_string(marker_path_)}});
                return false;
            }
            transaction_marker_exists = false;
        }

        if (!interrupted_backup_exists && package_exists) {
            if (filesystem_error || !std::filesystem::is_directory(pinned_path(package_root_), filesystem_error)) {
                error = runtime_text(
                    "Runtime.Package.Error.PackageTransactionStartFailed",
                    {{"path", copperfin::platform::path_to_utf8_string(package_root_)}});
                return false;
            }
            std::string owner_error;
            if (!write_owned_transaction_file_atomically(backup_owner_path_, owner_error)) {
                error = runtime_text(
                    "Runtime.Package.Error.PackageTransactionStartFailed",
                    {{"path", copperfin::platform::path_to_utf8_string(package_root_)}});
                return false;
            }
            if (!ensure_parent_identity(
                    error,
                    "Runtime.Package.Error.PackageTransactionStartFailed")) {
                std::error_code ignored;
                std::filesystem::remove(pinned_path(backup_owner_path_), ignored);
                return false;
            }
            std::filesystem::rename(
                pinned_path(package_root_),
                pinned_path(backup_root_),
                filesystem_error);
            if (filesystem_error) {
                std::error_code ignored;
                std::filesystem::remove(pinned_path(backup_owner_path_), ignored);
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

        if (!ensure_parent_identity(
                error,
                "Runtime.Package.Error.PackageRollbackFailed")) {
#if !defined(_WIN32)
            if (parent_identity_.rollback_at_pinned_parent(
                    copperfin::platform::path_to_utf8_string(package_root_.filename()),
                    copperfin::platform::path_to_utf8_string(backup_root_.filename()),
                    copperfin::platform::path_to_utf8_string(marker_path_.filename()),
                    copperfin::platform::path_to_utf8_string(backup_owner_path_.filename()),
                    had_previous_package_)) {
                active_ = false;
                return true;
            }
#endif
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
    }

    bool commit(std::string& error, std::string& warning) {
        if (!active_) {
            return true;
        }

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
        error = runtime_text(error_key, {{"path", copperfin::platform::path_to_utf8_string(package_root_)}});
        return false;
    }

    bool write_owned_transaction_file_atomically(
        const std::filesystem::path& path,
        std::string& error,
        const std::string& contents = {}) const {
        if (!parent_identity_.still_same()) {
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
            copperfin::platform::path_to_utf8_string(pinned_target) + ".tmp." +
            std::to_string(timestamp) + "." +
            std::to_string(sequence.fetch_add(1U, std::memory_order_relaxed));

        std::error_code filesystem_error;
        if (directory_entry_exists(temporary_path, filesystem_error) || filesystem_error) {
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
        return !error && status.type() != std::filesystem::file_type::not_found;
    }

    static bool is_direct_directory(
        const std::filesystem::path& path,
        std::error_code& error) {
        const std::filesystem::file_status status =
            std::filesystem::symlink_status(path, error);
        return !error && status.type() == std::filesystem::file_type::directory;
    }

    bool is_owned_transaction_file(const std::filesystem::path& path) const {
        std::error_code filesystem_error;
        const std::filesystem::file_status status =
            std::filesystem::symlink_status(path, filesystem_error);
        if (status.type() != std::filesystem::file_type::regular || filesystem_error) {
            return false;
        }
        const std::string contents = read_text_file(path);
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

RuntimePackagePlan create_runtime_package_plan(
    const studio::StudioDocumentModel& document,
    const studio::StudioProjectWorkspace& workspace,
    const security::NativeSecurityProfile& security_profile,
    const platform::ExtensibilityProfile& extensibility_profile,
    const std::string& output_root,
    BuildConfiguration configuration,
    bool enable_security,
    bool emit_dotnet_launcher) {
    RuntimePackagePlan plan;
    plan.project_path = document.path;
    plan.project_title = workspace.project_title.empty()
        ? copperfin::platform::path_to_utf8_string(
              copperfin::platform::path_from_utf8_string(document.path).stem())
        : workspace.project_title;
    plan.configuration = configuration;
    plan.security_enabled = enable_security;
    plan.output_kind = parse_build_output_kind(workspace.build_plan.output_kind);
    if (plan.output_kind == BuildOutputKind::unknown) {
        plan.output_kind = infer_build_output_kind_from_output_path(workspace.build_plan.output_path);
    }
    plan.requested_dotnet_launcher = emit_dotnet_launcher;
    plan.emit_dotnet_launcher =
        is_native_host_output_kind(plan.output_kind) &&
        emit_dotnet_launcher &&
        extensibility_profile.dotnet_output.available;
    if (is_library_output_kind(plan.output_kind)) {
        plan.launcher_mode = "foxpro_library_definition";
        plan.launcher_fallback = "library_binary_generation_pending";
    } else if (plan.output_kind == BuildOutputKind::app) {
        plan.launcher_mode = "foxpro_application_archive_contract";
        plan.launcher_fallback = "foxpro_app_binary_generation_pending";
    } else if (plan.output_kind == BuildOutputKind::fxp) {
        plan.launcher_mode = "foxpro_tokenized_contract";
        plan.launcher_fallback = "foxpro_fxp_binary_generation_pending";
    } else {
        plan.launcher_mode = plan.emit_dotnet_launcher ? "dotnet_launcher" : "native_runtime_host";
        plan.launcher_fallback =
            (plan.requested_dotnet_launcher && !plan.emit_dotnet_launcher)
                ? "dotnet_output_unavailable"
                : "none";
    }

    if (!workspace.available) {
        plan.warnings.push_back(runtime_text("Runtime.Package.Warning.ProjectWorkspaceUnavailable"));
        return plan;
    }

    const std::filesystem::path root = copperfin::platform::path_from_utf8_string(output_root);
    const std::filesystem::path package_root = root / sanitize_file_name(plan.project_title);
    const std::filesystem::path content_root = package_root / "content";
    plan.package_root = copperfin::platform::path_to_utf8_string(package_root);
    plan.content_root = copperfin::platform::path_to_utf8_string(content_root);
    plan.manifest_path = copperfin::platform::path_to_utf8_string(package_root / "app.cfmanifest");
    plan.debug_manifest_path = copperfin::platform::path_to_utf8_string(package_root / "app.cfdebug");
    plan.launcher_project_path = copperfin::platform::path_to_utf8_string(package_root / "launcher" / "Copperfin.GeneratedLauncher.csproj");
    plan.launcher_source_path = copperfin::platform::path_to_utf8_string(package_root / "launcher" / "Program.cs");
    const std::filesystem::path output_file_name(resolve_output_file_name(workspace, plan.project_title));
    plan.ast_manifest_path = copperfin::platform::path_to_utf8_string(package_root / (copperfin::platform::path_to_utf8_string(output_file_name) + ".ast.json"));
    plan.ir_manifest_path = copperfin::platform::path_to_utf8_string(package_root / (copperfin::platform::path_to_utf8_string(output_file_name) + ".ir.json"));
    plan.transpiled_csharp_path = copperfin::platform::path_to_utf8_string(package_root / (copperfin::platform::path_to_utf8_string(output_file_name) + ".transpiled.cs"));
    std::filesystem::path module_definition_file_name = output_file_name;
    module_definition_file_name.replace_extension(".def");
    plan.launcher_output_path = copperfin::platform::path_to_utf8_string(package_root / output_file_name);
    plan.module_definition_path = copperfin::platform::path_to_utf8_string(package_root / module_definition_file_name);
    if (is_library_output_kind(plan.output_kind)) {
        const std::filesystem::path wrapper_root = package_root / "wrapper";
        const std::string output_stem = copperfin::platform::path_to_utf8_string(output_file_name.stem());
        plan.native_wrapper_source_path = copperfin::platform::path_to_utf8_string(wrapper_root / (output_stem + "_wrapper.cpp"));
        plan.native_wrapper_cmake_path = copperfin::platform::path_to_utf8_string(wrapper_root / "CMakeLists.txt");
        plan.native_wrapper_build_script_path = copperfin::platform::path_to_utf8_string(wrapper_root / "build_wrapper.sh");
        plan.native_wrapper_build_powershell_path = copperfin::platform::path_to_utf8_string(wrapper_root / "build_wrapper.ps1");
    }
    if (plan.output_kind == BuildOutputKind::dll || plan.output_kind == BuildOutputKind::ocx) {
        std::filesystem::path library_api_manifest_file_name = output_file_name;
        library_api_manifest_file_name += ".api";
        plan.library_api_manifest_path = copperfin::platform::path_to_utf8_string(package_root / library_api_manifest_file_name);
    }
    if (plan.output_kind == BuildOutputKind::fll) {
        std::filesystem::path fll_api_manifest_file_name = output_file_name;
        fll_api_manifest_file_name += ".api";
        plan.fll_api_manifest_path = copperfin::platform::path_to_utf8_string(package_root / fll_api_manifest_file_name);
    }
    if (plan.output_kind == BuildOutputKind::fxp) {
        std::filesystem::path fxp_token_manifest_file_name = output_file_name;
        fxp_token_manifest_file_name += ".tokens";
        plan.fxp_token_manifest_path = copperfin::platform::path_to_utf8_string(package_root / fxp_token_manifest_file_name);
    }
    if (plan.output_kind == BuildOutputKind::app) {
        std::filesystem::path app_archive_manifest_file_name = output_file_name;
        app_archive_manifest_file_name += ".contents";
        plan.app_archive_manifest_path = copperfin::platform::path_to_utf8_string(package_root / app_archive_manifest_file_name);
    }
    plan.runtime_host_destination_path = copperfin::platform::path_to_utf8_string(package_root / runtime_host_file_name());
    std::string output_name_error;
    if (!validate_public_output_artifact_name(plan, output_name_error)) {
        plan.warnings.push_back(std::move(output_name_error));
        return plan;
    }
    plan.working_directory = copperfin::platform::path_to_utf8_string(content_root.lexically_normal());
    plan.startup_item = workspace.build_plan.startup_item;
    plan.security_role = resolve_security_role(enable_security);
    if (enable_security && !is_recognized_security_role(security_profile, plan.security_role)) {
        const std::string requested_role = plan.security_role;
        plan.security_role = "developer";
        if (!requested_role.empty()) {
            plan.warnings.push_back(runtime_text(
                "Runtime.Package.Warning.UnknownSecurityRoleRequested",
                {
                    {"requestedRole", requested_role},
                    {"defaultRole", plan.security_role}
                }));
        }
    }
    plan.audit_log_path = copperfin::platform::path_to_utf8_string(package_root / "security_audit.log");
    const std::string source_working_directory = resolve_working_directory(document, workspace);

    for (const auto& entry : workspace.entries) {
        RuntimePackageAsset asset;
        asset.record_index = entry.record_index;
        asset.required_for_runtime =
            entry.record_index == workspace.build_plan.startup_record_index;
        asset.source_path = resolve_project_item_source(
            document,
            entry,
            asset.required_for_runtime,
            asset.source_resolution_error);
        asset.exists =
            asset.source_resolution_error.empty() &&
            !asset.source_path.empty() &&
            source_path_exists_on_host(asset.source_path);
        asset.relative_path = relative_asset_path(
            document,
            entry,
            asset.source_path,
            asset.required_for_runtime && asset.exists);
        asset.staged_path = copperfin::platform::path_to_utf8_string(
            (content_root / copperfin::platform::path_from_utf8_string(asset.relative_path)).lexically_normal());
        asset.type_title = entry.type_title;
        asset.excluded = entry.excluded;
        if (asset.required_for_runtime) {
            plan.startup_source_path = asset.staged_path;
            plan.debug_plan.startup_source_path = asset.source_path;
        }
        asset.package_writable =
            !asset.required_for_runtime && is_writable_package_data_path(asset.source_path);
        if (!asset.exists && !entry.excluded && entry.group_id != "project") {
            if (!asset.source_resolution_error.empty()) {
                plan.warnings.push_back(asset.source_resolution_error);
            } else {
                plan.warnings.push_back(runtime_text(
                    "Runtime.Package.Warning.MissingProjectAsset",
                    {{"path", asset.source_path}}));
            }
        }
        plan.assets.push_back(std::move(asset));
    }

    if (plan.startup_source_path.empty()) {
        plan.warnings.push_back(runtime_text("Runtime.Package.Warning.StartupSourceUnresolved"));
    }
    if (plan.debug_plan.startup_source_path.empty()) {
        plan.warnings.push_back(runtime_text("Runtime.Package.Warning.DebugStartupSourceUnresolved"));
    }

    plan.debug_plan.manifest_path = plan.debug_manifest_path;
    plan.debug_plan.startup_item = plan.startup_item;
    plan.debug_plan.working_directory = source_working_directory;
    plan.debug_plan.source_roots = unique_non_empty_paths_preserve_order({
        source_working_directory,
        plan.content_root
    });
    const auto startup_asset = std::find_if(
        plan.assets.begin(),
        plan.assets.end(),
        [](const RuntimePackageAsset& asset) {
            return asset.required_for_runtime;
        });
    plan.debug_plan.supports_breakpoints =
        startup_asset != plan.assets.end() &&
        startup_asset->exists &&
        (is_prg_path(plan.debug_plan.startup_source_path) ||
         is_xasset_path(plan.debug_plan.startup_source_path));
    plan.debug_plan.supports_step_debugging = plan.debug_plan.supports_breakpoints;

    if (enable_security && !security_profile.available) {
        plan.warnings.push_back(runtime_text("Runtime.Package.Warning.SecurityProfileUnavailable"));
    }
    if (emit_dotnet_launcher && !extensibility_profile.dotnet_output.available) {
        plan.warnings.push_back(runtime_text("Runtime.Package.Warning.DotNetOutputProfileUnavailable"));
    }
    if (is_library_output_kind(plan.output_kind)) {
        plan.exported_symbols = collect_library_exported_symbols(plan);
        if (plan.exported_symbols.empty()) {
            plan.warnings.push_back(runtime_text("Runtime.Package.Warning.LibraryExportsUnresolved"));
        }
    } else if (plan.output_kind == BuildOutputKind::fxp) {
        const bool has_prg_asset = std::any_of(plan.assets.begin(), plan.assets.end(), [](const RuntimePackageAsset& asset) {
            return is_prg_path(asset.source_path);
        });
        if (!has_prg_asset) {
            plan.warnings.push_back(runtime_text("Runtime.Package.Warning.FxpSourcesUnresolved"));
        }
    }

    plan.planning_warning_count = plan.warnings.size();
    plan.planning_warnings_captured = true;
    plan.ok = true;
    return plan;
}

std::string build_runtime_manifest_text(
    const RuntimePackagePlan& plan,
    const security::NativeSecurityProfile& security_profile,
    const platform::ExtensibilityProfile& extensibility_profile) {
    std::ostringstream stream;
    stream << "manifest_version=" << kRuntimeManifestVersion << "\n";
    stream << "manifest_value_encoding=backslash-v1\n";
    stream << "project_title=" << quote_manifest_value(plan.project_title) << "\n";
    stream << "package_root=" << quote_manifest_value(plan.package_root) << "\n";
    stream << "content_root=" << quote_manifest_value(plan.content_root) << "\n";
    stream << "working_directory=" << quote_manifest_value(plan.working_directory) << "\n";
    stream << "startup_item=" << quote_manifest_value(plan.startup_item) << "\n";
    stream << "startup_source=" << quote_manifest_value(plan.startup_source_path) << "\n";
    stream << "configuration=" << build_configuration_name(plan.configuration) << "\n";
    stream << "output_kind=" << quote_manifest_value(build_output_kind_name(plan.output_kind)) << "\n";
    stream << "fll_loader_entrypoint="
           << quote_manifest_value(plan.output_kind == BuildOutputKind::fll ? std::string(kFllLoaderEntrypoint) : std::string()) << "\n";
    stream << "fll_registration_symbol="
           << quote_manifest_value(plan.output_kind == BuildOutputKind::fll ? std::string(kFllRegistrationSymbol) : std::string()) << "\n";
    stream << "fll_callable_signature="
           << quote_manifest_value(plan.output_kind == BuildOutputKind::fll ? std::string(kFllCallableSignature) : std::string()) << "\n";
    stream << "fll_default_return_helper="
           << quote_manifest_value(plan.output_kind == BuildOutputKind::fll ? std::string(kFllDefaultReturnHelper) : std::string()) << "\n";
    stream << "library_callable_convention="
           << quote_manifest_value((plan.output_kind == BuildOutputKind::dll || plan.output_kind == BuildOutputKind::ocx)
                                       ? std::string(kVfpLibraryCallableConvention)
                                       : std::string()) << "\n";
    stream << "security_enabled=" << (plan.security_enabled ? "true" : "false") << "\n";
    stream << "security_role=" << quote_manifest_value(plan.security_role) << "\n";
    stream << "security_mode=" << quote_manifest_value(security_profile.mode) << "\n";
    stream << "audit_log_path=" << quote_manifest_value(plan.audit_log_path) << "\n";
    stream << "runtime_host_sha256=" << quote_manifest_value(plan.runtime_host_sha256) << "\n";
    stream << "dotnet_story=" << quote_manifest_value(extensibility_profile.dotnet_output.primary_story) << "\n";
    stream << "data_policy=package_writable\n";

    append_runtime_asset_manifest_lines(stream, plan);
    append_writable_data_manifest_lines(stream, plan);

    for (const auto& digest : plan.extension_payload_digests) {
        stream << "extension_payload="
               << quote_manifest_value(digest.path) << "|"
               << quote_manifest_value(digest.sha256) << "\n";
    }
    // Provenance only: app.cfmanifest is consumed after these launcher files execute.
    for (const auto& artifact : plan.launcher_artifacts) {
        stream << "launcher_artifact="
               << quote_manifest_value(artifact.package_relative_path) << "|"
               << launcher_artifact_role_name(artifact.role) << "|"
               << quote_manifest_value(artifact.sha256) << "\n";
    }
    append_warning_manifest_lines(stream, plan);

    return stream.str();
}

std::string build_debug_manifest_text(
    const RuntimePackagePlan& plan,
    const security::NativeSecurityProfile& security_profile,
    const platform::ExtensibilityProfile& extensibility_profile) {
    std::ostringstream stream;
    stream << "debug_manifest_version=" << kDebugManifestVersion << "\n";
    stream << "manifest_value_encoding=backslash-v1\n";
    stream << "project_title=" << quote_manifest_value(plan.project_title) << "\n";
    stream << "project_path=" << quote_manifest_value(plan.project_path) << "\n";
    stream << "package_root=" << quote_manifest_value(plan.package_root) << "\n";
    stream << "content_root=" << quote_manifest_value(plan.content_root) << "\n";
    stream << "data_policy=package_writable\n";
    stream << "ast_manifest_path=" << quote_manifest_value(plan.ast_manifest_path) << "\n";
    stream << "ir_manifest_path=" << quote_manifest_value(plan.ir_manifest_path) << "\n";
    stream << "transpiled_csharp_path=" << quote_manifest_value(plan.transpiled_csharp_path) << "\n";
    stream << "configuration=" << build_configuration_name(plan.configuration) << "\n";
    stream << "security_enabled=" << (plan.security_enabled ? "true" : "false") << "\n";
    stream << "security_role=" << quote_manifest_value(plan.security_role) << "\n";
    stream << "security_mode=" << quote_manifest_value(security_profile.mode) << "\n";
    stream << "audit_log_path=" << quote_manifest_value(plan.audit_log_path) << "\n";
    stream << "runtime_host_sha256=" << quote_manifest_value(plan.runtime_host_sha256) << "\n";
    stream << "license_state=" << quote_manifest_value(plan.license_state) << "\n";
    stream << "license_type=" << quote_manifest_value(plan.license_type) << "\n";
    stream << "license_id=" << quote_manifest_value(plan.license_id) << "\n";
    stream << "license_licensee=" << quote_manifest_value(plan.license_licensee) << "\n";
    stream << "license_seats=" << plan.license_seats << "\n";
    stream << "license_subscription_expires=" << quote_manifest_value(plan.license_subscription_expires) << "\n";
    stream << "license_perpetual_max_major_version=" << plan.license_perpetual_max_major_version << "\n";
    stream << "security_roles=" << security_profile.roles.size() << "\n";
    stream << "startup_item=" << quote_manifest_value(plan.debug_plan.startup_item) << "\n";
    stream << "startup_source=" << quote_manifest_value(plan.debug_plan.startup_source_path) << "\n";
    stream << "working_directory=" << quote_manifest_value(plan.debug_plan.working_directory) << "\n";
    stream << "supports_breakpoints=" << (plan.debug_plan.supports_breakpoints ? "true" : "false") << "\n";
    stream << "supports_step_debugging=" << (plan.debug_plan.supports_step_debugging ? "true" : "false") << "\n";
    stream << "output_kind=" << quote_manifest_value(build_output_kind_name(plan.output_kind)) << "\n";
    stream << "primary_output_path=" << quote_manifest_value(plan.launcher_output_path) << "\n";
    stream << "primary_output_materialized=" << (plan.primary_output_materialized ? "true" : "false") << "\n";
    stream << "module_definition_path=" << quote_manifest_value(plan.module_definition_path) << "\n";
    stream << "native_wrapper_source_path=" << quote_manifest_value(plan.native_wrapper_source_path) << "\n";
    stream << "native_wrapper_cmake_path=" << quote_manifest_value(plan.native_wrapper_cmake_path) << "\n";
    stream << "native_wrapper_build_script_path=" << quote_manifest_value(plan.native_wrapper_build_script_path) << "\n";
    stream << "native_wrapper_build_powershell_path=" << quote_manifest_value(plan.native_wrapper_build_powershell_path) << "\n";
    stream << "library_api_manifest_path=" << quote_manifest_value(plan.library_api_manifest_path) << "\n";
    stream << "fll_api_manifest_path=" << quote_manifest_value(plan.fll_api_manifest_path) << "\n";
    stream << "fxp_token_manifest_path=" << quote_manifest_value(plan.fxp_token_manifest_path) << "\n";
    stream << "app_archive_manifest_path=" << quote_manifest_value(plan.app_archive_manifest_path) << "\n";
    stream << "fll_loader_entrypoint="
           << quote_manifest_value(plan.output_kind == BuildOutputKind::fll ? std::string(kFllLoaderEntrypoint) : std::string()) << "\n";
    stream << "fll_registration_symbol="
           << quote_manifest_value(plan.output_kind == BuildOutputKind::fll ? std::string(kFllRegistrationSymbol) : std::string()) << "\n";
    stream << "fll_callable_signature="
           << quote_manifest_value(plan.output_kind == BuildOutputKind::fll ? std::string(kFllCallableSignature) : std::string()) << "\n";
    stream << "fll_default_return_helper="
           << quote_manifest_value(plan.output_kind == BuildOutputKind::fll ? std::string(kFllDefaultReturnHelper) : std::string()) << "\n";
    stream << "library_callable_convention="
           << quote_manifest_value((plan.output_kind == BuildOutputKind::dll || plan.output_kind == BuildOutputKind::ocx)
                                       ? std::string(kVfpLibraryCallableConvention)
                                       : std::string()) << "\n";
    stream << "launcher_mode=" << quote_manifest_value(plan.launcher_mode) << "\n";
    stream << "launcher_fallback=" << quote_manifest_value(plan.launcher_fallback) << "\n";
    stream << "dotnet_enabled=" << (extensibility_profile.dotnet_output.available ? "true" : "false") << "\n";
    stream << "dotnet_story=" << quote_manifest_value(extensibility_profile.dotnet_output.primary_story) << "\n";
    stream << "dotnet_policy_allowlist=" << extensibility_profile.dotnet_output.policy.allowlist.size() << "\n";
    stream << "dotnet_policy_denylist=" << extensibility_profile.dotnet_output.policy.denylist.size() << "\n";
    stream << "dotnet_parity_matrix_entries=" << extensibility_profile.dotnet_output.parity_matrix.size() << "\n";
    stream << "dotnet_policy_allowlist_items=" << extensibility_profile.dotnet_output.policy.allowlist.size() << "\n";
    for (const auto& capability_id : extensibility_profile.dotnet_output.policy.allowlist) {
        stream << "dotnet_policy_allowlist_item=" << quote_manifest_value(capability_id) << "\n";
    }
    stream << "dotnet_policy_denylist_items=" << extensibility_profile.dotnet_output.policy.denylist.size() << "\n";
    for (const auto& capability_id : extensibility_profile.dotnet_output.policy.denylist) {
        stream << "dotnet_policy_denylist_item=" << quote_manifest_value(capability_id) << "\n";
    }
    stream << "dotnet_parity_matrix_count=" << extensibility_profile.dotnet_output.parity_matrix.size() << "\n";
    for (const auto& capability : extensibility_profile.dotnet_output.parity_matrix) {
        stream << "dotnet_parity_matrix_item="
               << quote_manifest_value(capability.id) << "|"
               << quote_manifest_value(capability.title) << "|"
               << dotnet_parity_tier_name(capability.tier) << "|"
               << quote_manifest_value(capability.rationale) << "|"
               << quote_manifest_value(capability.verification_reference) << "\n";
    }
    const platform::DotNetInteropCallDecision launcher_decision = platform::evaluate_dotnet_interop_call(
        extensibility_profile,
        platform::DotNetInteropCallRequest{
            .capability_id = "task-primitives",
            .estimated_latency_ms = 10U,
            .requires_reflection = false,
            .untrusted_input = false,
            .security_sensitive = false});
    stream << "dotnet_gateway_task_primitives=" << quote_manifest_value(launcher_decision.execution_path + ":" + launcher_decision.reason) << "\n";

    const platform::DotNetInteropCallDecision denied_decision = platform::evaluate_dotnet_interop_call(
        extensibility_profile,
        platform::DotNetInteropCallRequest{
            .capability_id = "unsafe-reflection-load",
            .estimated_latency_ms = 2U,
            .requires_reflection = true,
            .untrusted_input = true,
            .security_sensitive = true});
    stream << "dotnet_gateway_unsafe_reflection=" << quote_manifest_value(denied_decision.execution_path + ":" + denied_decision.reason) << "\n";
    stream << "language_integration_count=" << extensibility_profile.languages.size() << "\n";
    for (const auto& language : extensibility_profile.languages) {
        stream << "language_integration="
               << quote_manifest_value(language.id) << "|"
               << quote_manifest_value(language.title) << "|"
               << quote_manifest_value(language.integration_mode) << "|"
               << quote_manifest_value(language.trust_boundary) << "|"
               << quote_manifest_value(language.output_story) << "|"
               << (language.enabled_by_default ? "true" : "false") << "\n";
    }
    stream << "ai_feature_count=" << extensibility_profile.ai_features.size() << "\n";
    for (const auto& feature : extensibility_profile.ai_features) {
        stream << "ai_feature="
               << quote_manifest_value(feature.id) << "|"
               << quote_manifest_value(feature.title) << "|"
               << quote_manifest_value(feature.description) << "|"
               << quote_manifest_value(feature.trust_boundary) << "|"
               << (feature.enabled_by_default ? "true" : "false") << "\n";
    }
    stream << "extensibility_guardrail_count=" << extensibility_profile.guardrails.size() << "\n";
    for (const auto& guardrail : extensibility_profile.guardrails) {
        stream << "extensibility_guardrail=" << quote_manifest_value(guardrail) << "\n";
    }
    stream << "language_integrations=" << extensibility_profile.languages.size() << "\n";
    stream << "ai_features=" << extensibility_profile.ai_features.size() << "\n";
    stream << "source_roots=" << quote_manifest_value(join_strings(plan.debug_plan.source_roots)) << "\n";
    append_runtime_feature_flag_manifest_lines(stream, plan, security_profile);
    for (const auto& digest : plan.compiler_contract_digests) {
        stream << "compiler_contract="
               << quote_manifest_value(digest.path) << "|"
               << quote_manifest_value(digest.sha256) << "\n";
    }
    for (const auto& digest : plan.extension_payload_digests) {
        stream << "extension_payload="
               << quote_manifest_value(digest.path) << "|"
               << quote_manifest_value(digest.sha256) << "\n";
    }
    for (const auto& artifact : plan.launcher_artifacts) {
        stream << "launcher_artifact="
               << quote_manifest_value(artifact.package_relative_path) << "|"
               << launcher_artifact_role_name(artifact.role) << "|"
               << quote_manifest_value(artifact.sha256) << "\n";
    }
    for (const auto& symbol : plan.exported_symbols) {
        stream << "export_symbol=" << quote_manifest_value(symbol) << "\n";
    }
    append_runtime_asset_manifest_lines(stream, plan);
    append_writable_data_manifest_lines(stream, plan);
    append_warning_manifest_lines(stream, plan);
    append_library_function_manifest_lines(stream, plan, true);
    return stream.str();
}

static RuntimeMaterializeResult materialize_runtime_package_in_fresh_root(
    const PackageRootTransaction& transaction,
    const RuntimePackagePlan& plan,
    RuntimePackagePlan filesystem_plan,
    const security::NativeSecurityProfile& security_profile,
    const platform::ExtensibilityProfile& extensibility_profile,
    const std::string& runtime_host_source_path) {
    std::string error;
    if (!transaction.validate_parent_identity_for_materialization(error)) {
        return {.ok = false, .error = error};
    }
    std::error_code directory_error;
#if defined(_WIN32)
    std::filesystem::create_directories(filesystem_plan.package_root, directory_error);
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
            filesystem_plan.package_root,
            filesystem_plan.content_root,
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
        std::filesystem::create_directories(
            copperfin::platform::path_from_utf8_string(filesystem_plan.launcher_project_path).parent_path(),
            directory_error);
        if (directory_error) {
            return {.ok = false, .error = runtime_text("Runtime.Package.Error.CreateLauncherDirectoryFailed")};
        }
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
            physical_path.lexically_relative(filesystem_plan.content_root);
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
            physical_path.lexically_relative(filesystem_plan.package_root);
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
    const auto append_pinned_digest =
        [&](std::vector<RuntimeArtifactDigest>& digests,
            const std::filesystem::path& physical_path,
            std::string& digest_error) {
            if (physical_path.empty() || !std::filesystem::exists(physical_path)) {
                return true;
            }
            const auto digest = security::sha256_hex_for_file(
                copperfin::platform::path_to_utf8_string(physical_path));
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
                asset.source_path,
                filesystem_plan.package_root,
                filesystem_plan.content_root,
                asset.relative_path,
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
                filesystem_plan.package_root,
                filesystem_plan.content_root,
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

        const auto digest = security::sha256_hex_for_file(
            copperfin::platform::path_to_utf8_string(destination));
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
                runtime_host_source_path,
                filesystem_plan.runtime_host_destination_path,
                error)) {
            return {.ok = false, .error = error};
        }
        if (!append_pinned_digest(
                materialized_plan.extension_payload_digests,
                filesystem_plan.runtime_host_destination_path,
                error)) {
            return {.ok = false, .error = error};
        }
        const auto runtime_host_digest = security::sha256_hex_for_file(
            filesystem_plan.runtime_host_destination_path);
        if (!runtime_host_digest.ok) {
            return {.ok = false, .error = runtime_host_digest.error};
        }
        materialized_plan.runtime_host_sha256 = runtime_host_digest.hex_digest;

        if (!transaction.validate_parent_identity_for_materialization(error)) {
            return {.ok = false, .error = error};
        }
        std::filesystem::create_directories(
            copperfin::platform::path_from_utf8_string(filesystem_plan.native_wrapper_source_path).parent_path(),
            directory_error);
        if (directory_error) {
            return {.ok = false, .error = runtime_text("Runtime.Package.Error.CreateNativeWrapperDirectoryFailed")};
        }
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
                filesystem_plan.native_wrapper_cmake_path,
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
                filesystem_plan.native_wrapper_build_script_path,
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
                filesystem_plan.native_wrapper_build_powershell_path,
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
                    filesystem_plan.library_api_manifest_path,
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
                    filesystem_plan.fll_api_manifest_path,
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
                filesystem_plan.fxp_token_manifest_path,
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
                filesystem_plan.launcher_output_path,
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
                filesystem_plan.launcher_output_path,
                error)) {
            return {.ok = false, .error = error};
        }
        materialized_plan.primary_output_materialized = true;
    } else {
        if (!transaction.validate_parent_identity_for_materialization(error)) {
            return {.ok = false, .error = error};
        }
        if (!copy_file_if_exists(
                runtime_host_source_path,
                filesystem_plan.runtime_host_destination_path,
                error)) {
            return {.ok = false, .error = error};
        }

        const auto runtime_host_digest = security::sha256_hex_for_file(
            filesystem_plan.runtime_host_destination_path);
        if (!runtime_host_digest.ok) {
            return {.ok = false, .error = runtime_host_digest.error};
        }
        materialized_plan.runtime_host_sha256 = runtime_host_digest.hex_digest;
        materialized_plan.extension_payload_digests.push_back({
            .path = copperfin::platform::path_to_utf8_string(
                logical_package_path(filesystem_plan.runtime_host_destination_path)),
            .sha256 = runtime_host_digest.hex_digest
        });

        if (!plan.emit_dotnet_launcher) {
            if (!copy_file_if_exists(
                    filesystem_plan.runtime_host_destination_path,
                    filesystem_plan.launcher_output_path,
                    error)) {
                return {.ok = false, .error = error};
            }

            const auto native_entrypoint_digest = security::sha256_hex_for_file(
                filesystem_plan.launcher_output_path);
            if (!native_entrypoint_digest.ok) {
                return {.ok = false, .error = native_entrypoint_digest.error};
            }
            materialized_plan.extension_payload_digests.push_back({
            .path = copperfin::platform::path_to_utf8_string(
                logical_package_path(filesystem_plan.launcher_output_path)),
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
            filesystem_plan.ast_manifest_path,
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
            filesystem_plan.ir_manifest_path,
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
            filesystem_plan.transpiled_csharp_path,
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

    PackageRootTransaction transaction(plan.package_root);
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
    if (!plan.emit_dotnet_launcher && !std::filesystem::exists(plan.launcher_output_path)) {
        return {.ok = false, .error = runtime_text("Runtime.Package.Error.PrimaryOutputMissing")};
    }

    std::optional<PackageRootTransaction> package_transaction;
    if ((plan.emit_dotnet_launcher || is_library_output_kind(plan.output_kind)) &&
        !plan.primary_output_materialized) {
        package_transaction.emplace(
            plan.package_root,
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
        plan.package_root,
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
    if (!std::filesystem::exists(plan.native_wrapper_cmake_path)) {
        return {.ok = false, .error = runtime_text("Runtime.Package.Error.NativeWrapperCMakeMissing")};
    }

    RuntimePackagePlan built_plan = plan;
    const std::filesystem::path source_root =
        copperfin::platform::path_from_utf8_string(plan.native_wrapper_cmake_path).parent_path();
    const std::filesystem::path build_root = source_root / "cmake_pipeline_build";
    const std::filesystem::path configure_log_path = build_root / "cmake-configure.log";
    const std::filesystem::path build_log_path = build_root / "cmake-build.log";
    std::error_code ignored;
    std::filesystem::remove_all(build_root, ignored);
    std::filesystem::remove(plan.launcher_output_path, ignored);
    std::filesystem::create_directories(build_root, ignored);
    if (ignored) {
        return {.ok = false, .error = runtime_text("Runtime.Package.Error.CreateNativeWrapperBuildDirectoryFailed")};
    }

    const std::string configure_command =
        "cmake -S \"" + copperfin::platform::path_to_utf8_string(source_root) + "\" -B \"" +
        copperfin::platform::path_to_utf8_string(build_root) + "\" > \"" +
        copperfin::platform::path_to_utf8_string(configure_log_path) + "\" 2>&1";
    if (std::system(configure_command.c_str()) != 0) {
        error = runtime_text("Runtime.Package.Error.NativeWrapperPrimaryOutputConfigureFailed");
        if (std::filesystem::exists(configure_log_path)) {
            error += ":\n" + read_text_file(configure_log_path);
        }
        return {.ok = false, .error = error};
    }

    const std::string build_command =
        "cmake --build \"" + copperfin::platform::path_to_utf8_string(build_root) + "\" > \"" +
        copperfin::platform::path_to_utf8_string(build_log_path) + "\" 2>&1";
    if (std::system(build_command.c_str()) != 0) {
        error = runtime_text("Runtime.Package.Error.NativeWrapperPrimaryOutputBuildFailed");
        if (std::filesystem::exists(build_log_path)) {
            error += ":\n" + read_text_file(build_log_path);
        }
        return {.ok = false, .error = error};
    }

    if (!std::filesystem::exists(plan.launcher_output_path)) {
        return {.ok = false, .error = runtime_text("Runtime.Package.Error.NativeWrapperPrimaryOutputMissing")};
    }

    return finalize_runtime_package_primary_output(
        built_plan,
        security_profile,
        extensibility_profile);
}

}  // namespace copperfin::runtime
