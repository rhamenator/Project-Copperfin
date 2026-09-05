// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/private_executable_image.h"

#include "copperfin/platform/private_directory.h"

#include <algorithm>
#include <array>
#include <limits>
#include <string_view>
#include <utility>
#include <vector>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#else
#include <cerrno>
#include <cstdio>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
extern char** environ;
#endif

namespace copperfin::platform {
namespace {

bool valid_request_path(
    const std::filesystem::path& parent,
    const std::filesystem::path& leaf,
    const std::span<const std::uint8_t> bytes) noexcept {
    if (parent.empty() || !parent.is_absolute() || leaf.empty() ||
        leaf.is_absolute() || leaf.has_parent_path() || leaf == "." ||
        leaf == ".." || bytes.empty()) {
        return false;
    }
    const auto contains_nul = [](const std::filesystem::path& path) {
        return path.native().find(
                   typename std::filesystem::path::value_type{}) !=
            std::filesystem::path::string_type::npos;
    };
    if (contains_nul(parent) || contains_nul(leaf)) {
        return false;
    }
    return std::none_of(parent.begin(), parent.end(), [](const auto& component) {
        return component == "." || component == "..";
    });
}

#if defined(_WIN32)

bool handle_identity_matches(
    const HANDLE handle,
    const std::uint64_t expected_storage_id,
    const std::uint64_t expected_file_id,
    const std::uint64_t expected_creation_ticks,
    const bool directory) noexcept {
    BY_HANDLE_FILE_INFORMATION information{};
    return handle != nullptr && handle != INVALID_HANDLE_VALUE &&
        ::GetFileInformationByHandle(handle, &information) != 0 &&
        ((information.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0U) ==
            directory &&
        (information.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) == 0U &&
        information.dwVolumeSerialNumber == expected_storage_id &&
        ((static_cast<std::uint64_t>(information.nFileIndexHigh) << 32U) |
         information.nFileIndexLow) == expected_file_id &&
        expected_creation_ticks != 0U &&
        ((static_cast<std::uint64_t>(
              information.ftCreationTime.dwHighDateTime) << 32U) |
         information.ftCreationTime.dwLowDateTime) == expected_creation_ticks;
}

std::optional<std::size_t> stable_volume_root_length(
    const std::wstring_view path) noexcept {
    constexpr std::wstring_view guid_prefix = L"\\\\?\\Volume{";
    if (path.rfind(guid_prefix, 0U) == 0U) {
        const std::size_t close = path.find(L"}\\", guid_prefix.size());
        if (close != std::wstring_view::npos && close > guid_prefix.size()) {
            return close + 2U;
        }
        return std::nullopt;
    }
    constexpr std::wstring_view device_prefix =
        L"\\\\?\\GLOBALROOT\\Device\\HarddiskVolume";
    if (path.rfind(device_prefix, 0U) != 0U) {
        return std::nullopt;
    }
    std::size_t index = device_prefix.size();
    const std::size_t digits_begin = index;
    while (index < path.size() && path[index] >= L'0' && path[index] <= L'9') {
        ++index;
    }
    if (index == digits_begin || index >= path.size() || path[index] != L'\\') {
        return std::nullopt;
    }
    return index + 1U;
}

std::optional<std::wstring> final_path_for_handle(
    const HANDLE handle,
    const DWORD volume_name) {
    const DWORD flags = FILE_NAME_NORMALIZED | volume_name;
    constexpr DWORD maximum_path_characters = 32'768U;
    std::vector<wchar_t> buffer(512U);
    for (;;) {
        const DWORD written = ::GetFinalPathNameByHandleW(
            handle, buffer.data(), static_cast<DWORD>(buffer.size()), flags);
        if (written == 0U || written > maximum_path_characters) {
            return std::nullopt;
        }
        if (written >= buffer.size()) {
            buffer.resize(static_cast<std::size_t>(written) + 1U);
            continue;
        }
        return std::wstring(buffer.data(), static_cast<std::size_t>(written));
    }
}

std::optional<std::filesystem::path> stable_volume_path_for_handle(
    const HANDLE handle) noexcept {
    try {
        if (auto guid = final_path_for_handle(handle, VOLUME_NAME_GUID);
            guid.has_value() && stable_volume_root_length(*guid).has_value()) {
            return std::filesystem::path(std::move(*guid));
        }
        auto native = final_path_for_handle(handle, VOLUME_NAME_NT);
        if (!native.has_value() ||
            native->rfind(L"\\Device\\HarddiskVolume", 0U) != 0U) {
            return std::nullopt;
        }
        std::wstring global = L"\\\\?\\GLOBALROOT" + *native;
        if (!stable_volume_root_length(global).has_value()) {
            return std::nullopt;
        }
        return std::filesystem::path(std::move(global));
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<std::filesystem::path> dos_volume_path_for_handle(
    const HANDLE handle) noexcept {
    try {
        auto dos = final_path_for_handle(handle, VOLUME_NAME_DOS);
        if (!dos.has_value() || dos->size() < 7U ||
            dos->rfind(L"\\\\?\\", 0U) != 0U ||
            !(((*dos)[4U] >= L'A' && (*dos)[4U] <= L'Z') ||
              ((*dos)[4U] >= L'a' && (*dos)[4U] <= L'z')) ||
            (*dos)[5U] != L':' || (*dos)[6U] != L'\\') {
            return std::nullopt;
        }
        return std::filesystem::path(std::move(*dos));
    } catch (...) {
        return std::nullopt;
    }
}

bool exact_file_shape(
    const HANDLE handle,
    const std::size_t expected_size) noexcept {
    BY_HANDLE_FILE_INFORMATION information{};
    if (::GetFileInformationByHandle(handle, &information) == 0 ||
        (information.dwFileAttributes &
            (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT)) != 0U ||
        information.nNumberOfLinks != 1U) {
        return false;
    }
    const std::uint64_t size =
        (static_cast<std::uint64_t>(information.nFileSizeHigh) << 32U) |
        information.nFileSizeLow;
    return size == expected_size;
}

bool write_all(
    const HANDLE handle,
    const std::span<const std::uint8_t> bytes) noexcept {
    std::size_t offset = 0U;
    while (offset < bytes.size()) {
        const DWORD requested = static_cast<DWORD>(std::min<std::size_t>(
            bytes.size() - offset, std::numeric_limits<DWORD>::max()));
        DWORD written = 0U;
        if (::WriteFile(
                handle, bytes.data() + offset, requested, &written, nullptr) ==
                FALSE ||
            written == 0U) {
            return false;
        }
        offset += written;
    }
    return ::FlushFileBuffers(handle) != FALSE;
}

bool native_matches_bytes(
    const HANDLE handle,
    const std::span<const std::uint8_t> expected) noexcept {
    LARGE_INTEGER beginning{};
    if (::SetFilePointerEx(handle, beginning, nullptr, FILE_BEGIN) == FALSE) {
        return false;
    }
    std::array<std::uint8_t, 8192U> buffer{};
    std::size_t offset = 0U;
    while (offset < expected.size()) {
        const DWORD requested = static_cast<DWORD>(std::min<std::size_t>(
            buffer.size(), expected.size() - offset));
        DWORD read = 0U;
        if (::ReadFile(handle, buffer.data(), requested, &read, nullptr) == FALSE ||
            read == 0U ||
            !std::equal(
                buffer.begin(), buffer.begin() + read,
                expected.begin() + static_cast<std::ptrdiff_t>(offset))) {
            return false;
        }
        offset += read;
    }
    std::uint8_t extra = 0U;
    DWORD extra_read = 0U;
    const bool exact = ::ReadFile(handle, &extra, 1U, &extra_read, nullptr) != FALSE &&
        extra_read == 0U;
    (void)::SetFilePointerEx(handle, beginning, nullptr, FILE_BEGIN);
    return exact;
}

bool delete_exact_file(const HANDLE handle) noexcept {
    if (handle == nullptr || handle == INVALID_HANDLE_VALUE) {
        return false;
    }
    FILE_DISPOSITION_INFO disposition{.DeleteFile = TRUE};
    return ::SetFileInformationByHandle(
        handle, FileDispositionInfo, &disposition,
        static_cast<DWORD>(sizeof(disposition))) != FALSE;
}

struct WindowsImageIdentity {
    std::uint64_t storage_id = 0U;
    std::uint64_t file_id = 0U;
    std::uint64_t creation_ticks = 0U;
    std::uint64_t size = 0U;
};

bool capture_image_identity(
    const HANDLE handle,
    WindowsImageIdentity& identity) noexcept {
    BY_HANDLE_FILE_INFORMATION information{};
    if (handle == nullptr || handle == INVALID_HANDLE_VALUE ||
        ::GetFileInformationByHandle(handle, &information) == 0 ||
        (information.dwFileAttributes &
            (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT)) != 0U ||
        information.nNumberOfLinks != 1U) {
        return false;
    }
    identity.storage_id = information.dwVolumeSerialNumber;
    identity.file_id =
        (static_cast<std::uint64_t>(information.nFileIndexHigh) << 32U) |
        information.nFileIndexLow;
    identity.creation_ticks =
        (static_cast<std::uint64_t>(
             information.ftCreationTime.dwHighDateTime) << 32U) |
        information.ftCreationTime.dwLowDateTime;
    identity.size =
        (static_cast<std::uint64_t>(information.nFileSizeHigh) << 32U) |
        information.nFileSizeLow;
    return identity.file_id != 0U && identity.creation_ticks != 0U;
}

bool image_identity_matches(
    const HANDLE handle,
    const WindowsImageIdentity& expected) noexcept {
    WindowsImageIdentity observed;
    return capture_image_identity(handle, observed) &&
        observed.storage_id == expected.storage_id &&
        observed.file_id == expected.file_id &&
        observed.creation_ticks == expected.creation_ticks &&
        observed.size == expected.size;
}

HANDLE open_path_for_exact_image(
    const std::filesystem::path& path,
    const DWORD desired_access,
    const DWORD share_mode) noexcept {
    return ::CreateFileW(
        path.c_str(), desired_access, share_mode, nullptr, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OPEN_REPARSE_POINT, nullptr);
}

HANDLE reopen_image_by_id(
    const HANDLE volume_file,
    const WindowsImageIdentity& identity,
    const DWORD desired_access,
    const DWORD share_mode) noexcept {
    FILE_ID_DESCRIPTOR descriptor{};
    descriptor.dwSize = sizeof(descriptor);
    descriptor.Type = FileIdType;
    descriptor.FileId.QuadPart = static_cast<LONGLONG>(identity.file_id);
    return ::OpenFileById(
        volume_file, &descriptor, desired_access, share_mode, nullptr,
        FILE_FLAG_OPEN_REPARSE_POINT);
}

bool cleanup_image_by_id(
    const HANDLE volume_file,
    const WindowsImageIdentity& identity) noexcept {
    const HANDLE cleanup = reopen_image_by_id(
        volume_file, identity, FILE_READ_ATTRIBUTES | DELETE,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE);
    const bool cleaned = image_identity_matches(cleanup, identity) &&
        delete_exact_file(cleanup);
    if (cleanup != nullptr && cleanup != INVALID_HANDLE_VALUE) {
        (void)::CloseHandle(cleanup);
    }
    return cleaned;
}

class OwnedHandle {
public:
    explicit OwnedHandle(HANDLE handle_value = INVALID_HANDLE_VALUE) noexcept
        : handle_(handle_value) {}
    ~OwnedHandle() { reset(); }
    OwnedHandle(const OwnedHandle&) = delete;
    OwnedHandle& operator=(const OwnedHandle&) = delete;

    [[nodiscard]] HANDLE get() const noexcept { return handle_; }
    [[nodiscard]] HANDLE release() noexcept {
        const HANDLE released = handle_;
        handle_ = INVALID_HANDLE_VALUE;
        return released;
    }
    void reset() noexcept {
        if (handle_ != nullptr && handle_ != INVALID_HANDLE_VALUE) {
            (void)::CloseHandle(handle_);
        }
        handle_ = INVALID_HANDLE_VALUE;
    }

private:
    HANDLE handle_ = INVALID_HANDLE_VALUE;
};

class OwnedDirectoryChain {
public:
    OwnedDirectoryChain() = default;
    ~OwnedDirectoryChain() { reset(); }
    OwnedDirectoryChain(const OwnedDirectoryChain&) = delete;
    OwnedDirectoryChain& operator=(const OwnedDirectoryChain&) = delete;
    OwnedDirectoryChain(OwnedDirectoryChain&& other) noexcept
        : handles_(std::move(other.handles_)) {
        other.handles_.clear();
    }
    OwnedDirectoryChain& operator=(OwnedDirectoryChain&& other) noexcept {
        if (this != &other) {
            reset();
            handles_ = std::move(other.handles_);
            other.handles_.clear();
        }
        return *this;
    }

    [[nodiscard]] bool lock(const std::filesystem::path& directory) noexcept {
        reset();
        try {
            const auto root_length =
                stable_volume_root_length(directory.native());
            if (!root_length.has_value() || *root_length > directory.native().size()) {
                reset();
                return false;
            }
            std::filesystem::path current(
                directory.native().substr(0U, *root_length));
            const std::filesystem::path relative(
                directory.native().substr(*root_length));
            // The caller supplies a validated local-volume device path. Its
            // GUID or GLOBALROOT root cannot be redirected like a drive-letter,
            // SUBST, or mapped-drive name, and
            // Windows may refuse a delete-denying open on the volume root.
            // Lock every actual directory entry below that stable root.
            for (const auto& component : relative) {
                if (component.empty() || component == "." || component == "..") {
                    reset();
                    return false;
                }
                current /= component;
                if (!lock_one(current)) {
                    reset();
                    return false;
                }
            }
            return !handles_.empty();
        } catch (...) {
            reset();
            return false;
        }
    }

private:
    [[nodiscard]] bool lock_one(
        const std::filesystem::path& directory) {
        const HANDLE handle = ::CreateFileW(
            directory.c_str(), FILE_READ_ATTRIBUTES,
            FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT, nullptr);
        if (handle == nullptr || handle == INVALID_HANDLE_VALUE) {
            return false;
        }
        BY_HANDLE_FILE_INFORMATION information{};
        if (::GetFileInformationByHandle(handle, &information) == 0 ||
            (information.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0U ||
            (information.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0U) {
            (void)::CloseHandle(handle);
            return false;
        }
        try {
            handles_.push_back(handle);
        } catch (...) {
            (void)::CloseHandle(handle);
            throw;
        }
        return true;
    }

    void reset() noexcept {
        for (auto iterator = handles_.rbegin(); iterator != handles_.rend();
             ++iterator) {
            if (*iterator != nullptr && *iterator != INVALID_HANDLE_VALUE) {
                (void)::CloseHandle(*iterator);
            }
        }
        handles_.clear();
    }

    std::vector<HANDLE> handles_;
};

class OwnedImageHandle {
public:
    explicit OwnedImageHandle(
        HANDLE handle_value = INVALID_HANDLE_VALUE) noexcept
        : handle_(handle_value) {}
    ~OwnedImageHandle() {
        (void)delete_exact_file(handle_);
        if (handle_ != nullptr && handle_ != INVALID_HANDLE_VALUE) {
            (void)::CloseHandle(handle_);
        }
    }
    OwnedImageHandle(const OwnedImageHandle&) = delete;
    OwnedImageHandle& operator=(const OwnedImageHandle&) = delete;

    [[nodiscard]] HANDLE get() const noexcept { return handle_; }
    [[nodiscard]] HANDLE release() noexcept {
        const HANDLE released = handle_;
        handle_ = INVALID_HANDLE_VALUE;
        return released;
    }
    void close_without_delete() noexcept {
        if (handle_ != nullptr && handle_ != INVALID_HANDLE_VALUE) {
            (void)::CloseHandle(handle_);
        }
        handle_ = INVALID_HANDLE_VALUE;
    }

private:
    HANDLE handle_ = INVALID_HANDLE_VALUE;
};

#else

class OwnedDescriptor {
public:
    explicit OwnedDescriptor(const int descriptor_value = -1) noexcept
        : descriptor_(descriptor_value) {}
    ~OwnedDescriptor() { reset(); }
    OwnedDescriptor(const OwnedDescriptor&) = delete;
    OwnedDescriptor& operator=(const OwnedDescriptor&) = delete;

    [[nodiscard]] int get() const noexcept { return descriptor_; }
    [[nodiscard]] int release() noexcept {
        const int released = descriptor_;
        descriptor_ = -1;
        return released;
    }
    void reset() noexcept {
        if (descriptor_ >= 0) {
            (void)::close(descriptor_);
        }
        descriptor_ = -1;
    }

private:
    int descriptor_ = -1;
};

class OwnedLinkedImageDescriptor {
public:
    OwnedLinkedImageDescriptor(
        const int descriptor_value,
        const int parent_descriptor,
        const std::string* leaf_name) noexcept
        : descriptor_(descriptor_value),
          parent_descriptor_(parent_descriptor),
          leaf_name_(leaf_name),
          linked_(descriptor_value >= 0) {}
    ~OwnedLinkedImageDescriptor() {
        if (linked_ && parent_descriptor_ >= 0 && leaf_name_ != nullptr) {
            (void)::unlinkat(parent_descriptor_, leaf_name_->c_str(), 0);
        }
        if (descriptor_ >= 0) {
            (void)::close(descriptor_);
        }
    }
    OwnedLinkedImageDescriptor(const OwnedLinkedImageDescriptor&) = delete;
    OwnedLinkedImageDescriptor& operator=(
        const OwnedLinkedImageDescriptor&) = delete;

    [[nodiscard]] int get() const noexcept { return descriptor_; }
    void mark_unlinked() noexcept { linked_ = false; }
    [[nodiscard]] int release() noexcept {
        linked_ = false;
        const int released = descriptor_;
        descriptor_ = -1;
        return released;
    }

private:
    int descriptor_ = -1;
    int parent_descriptor_ = -1;
    const std::string* leaf_name_ = nullptr;
    bool linked_ = true;
};

bool descriptor_identity_matches(
    const int descriptor,
    const std::uint64_t expected_storage_id,
    const std::uint64_t expected_file_id,
    const std::uint64_t expected_creation_ticks) noexcept {
    struct stat status{};
    if (descriptor < 0 || ::fstat(descriptor, &status) != 0 ||
        !S_ISDIR(status.st_mode) ||
        static_cast<std::uint64_t>(status.st_dev) != expected_storage_id ||
        static_cast<std::uint64_t>(status.st_ino) != expected_file_id ||
        status.st_uid != ::geteuid() || (status.st_mode & 07777) != 0700 ||
        expected_creation_ticks == 0U) {
        return false;
    }
    std::uint64_t creation_ticks = 0U;
#if defined(__APPLE__)
    if (status.st_birthtimespec.tv_sec >= 0 &&
        status.st_birthtimespec.tv_nsec >= 0 &&
        status.st_birthtimespec.tv_nsec < 1'000'000'000L) {
        creation_ticks =
            static_cast<std::uint64_t>(status.st_birthtimespec.tv_sec) *
                1'000'000'000ULL +
            static_cast<std::uint64_t>(status.st_birthtimespec.tv_nsec);
    }
#elif defined(__linux__) && defined(STATX_BTIME) && defined(AT_EMPTY_PATH)
    struct statx extended_status {};
    if (::statx(
            descriptor, "", AT_EMPTY_PATH | AT_SYMLINK_NOFOLLOW, STATX_BTIME,
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
    return creation_ticks == expected_creation_ticks;
}

bool exact_file_shape(
    const int descriptor,
    const std::size_t expected_size) noexcept {
    struct stat status{};
    if (descriptor < 0 || ::fstat(descriptor, &status) != 0 ||
        !S_ISREG(status.st_mode) || status.st_uid != ::geteuid() ||
        (status.st_mode & 07777) != 0500 ||
        static_cast<std::uint64_t>(status.st_size) != expected_size) {
        return false;
    }
#if defined(__APPLE__)
    // RQ-CF-AGENT-031 (supersedes RQ-CF-AGENT-026's POSIX unlink clause
    // for macOS only): this image stays linked for its entire lifetime,
    // defended by a kernel-enforced code signature rather than
    // filesystem-namespace invisibility -- see the Impl class doc comment
    // below for the empirical evidence and full reasoning.
    return status.st_nlink == 1;
#else
    // RQ-CF-AGENT-026: this image must be unlinked before its materialized
    // authority is exposed.
    return status.st_nlink == 0;
#endif
}

#if defined(__APPLE__)
// Ad-hoc code-signs the file at `path` in place (RQ-CF-AGENT-031). No
// shell, no PATH search: /usr/bin/codesign is invoked directly by its
// fixed, well-known system location, matching this codebase's existing
// no-shell/no-path-search posture for launching trusted binaries (see
// RQ-CF-AGENT-028's Windows requirement). Ad-hoc signing (identity "-")
// has been supported since long before this project's minimum macOS
// target and needs no certificate, keychain, or network access. Blocks
// until codesign exits; returns false with errno left as whatever the
// fork/exec/wait sequence last set on failure (not meaningful beyond
// "signing did not succeed").
bool codesign_in_place(const std::string& path) noexcept {
    std::string executable = "/usr/bin/codesign";
    std::string sign_flag = "--sign";
    std::string identity = "-";
    std::string force_flag = "--force";
    std::string target = path;
    std::vector<char*> argv{
        executable.data(), sign_flag.data(), identity.data(),
        force_flag.data(), target.data(), nullptr};
    // TEMPORARY diagnostic instrumentation (RQ-CF-AGENT-031 CI round):
    // captures whether execve() itself failed (e.g. codesign missing at
    // this fixed path) versus codesign running and exiting nonzero, since
    // a bare bool return conflates the two.
    int exec_pipe[2]{-1, -1};
    if (::pipe(exec_pipe) != 0) {
        return false;
    }
    (void)::fcntl(exec_pipe[1], F_SETFD, FD_CLOEXEC);
    const pid_t child = ::fork();
    if (child < 0) {
        (void)::close(exec_pipe[0]);
        (void)::close(exec_pipe[1]);
        return false;
    }
    if (child == 0) {
        ::close(exec_pipe[0]);
        ::execve(argv[0], argv.data(), environ);
        const int exec_errno = errno;
        (void)::write(exec_pipe[1], &exec_errno, sizeof(exec_errno));
        _exit(127);
    }
    ::close(exec_pipe[1]);
    int exec_errno = 0;
    const ssize_t exec_errno_bytes =
        ::read(exec_pipe[0], &exec_errno, sizeof(exec_errno));
    ::close(exec_pipe[0]);
    int status = 0;
    pid_t waited;
    do {
        waited = ::waitpid(child, &status, 0);
    } while (waited < 0 && errno == EINTR);
    const bool exec_failed = exec_errno_bytes == sizeof(exec_errno);
    const bool succeeded =
        !exec_failed && waited == child && WIFEXITED(status) &&
        WEXITSTATUS(status) == 0;
    char diagnostic[256];
    const int diagnostic_length = ::snprintf(
        diagnostic, sizeof(diagnostic),
        "MACOS_MATERIALIZE_DIAG stage=codesign path=%s exec_failed=%d "
        "exec_errno=%d waited=%d exited=%d exit_code=%d signaled=%d "
        "term_sig=%d succeeded=%d\n",
        path.c_str(), exec_failed ? 1 : 0, exec_failed ? exec_errno : 0,
        waited == child ? 1 : 0, WIFEXITED(status) ? 1 : 0,
        WIFEXITED(status) ? WEXITSTATUS(status) : -1,
        WIFSIGNALED(status) ? 1 : 0,
        WIFSIGNALED(status) ? WTERMSIG(status) : -1, succeeded ? 1 : 0);
    if (diagnostic_length > 0) {
        (void)::write(
            STDERR_FILENO, diagnostic,
            static_cast<std::size_t>(diagnostic_length));
    }
    return succeeded;
}
#endif

bool write_all(
    const int descriptor,
    const std::span<const std::uint8_t> bytes) noexcept {
    std::size_t offset = 0U;
    while (offset < bytes.size()) {
        const ssize_t written = ::write(
            descriptor, bytes.data() + offset, bytes.size() - offset);
        if (written < 0 && errno == EINTR) {
            continue;
        }
        if (written <= 0) {
            return false;
        }
        offset += static_cast<std::size_t>(written);
    }
    return ::fsync(descriptor) == 0;
}

bool native_matches_bytes(
    const int descriptor,
    const std::span<const std::uint8_t> expected) noexcept {
    std::array<std::uint8_t, 8192U> buffer{};
    std::size_t offset = 0U;
    while (offset < expected.size()) {
        const std::size_t requested =
            std::min(buffer.size(), expected.size() - offset);
        const ssize_t read = ::pread(
            descriptor, buffer.data(), requested,
            static_cast<off_t>(offset));
        if (read < 0 && errno == EINTR) {
            continue;
        }
        if (read <= 0 ||
            !std::equal(
                buffer.begin(), buffer.begin() + read,
                expected.begin() + static_cast<std::ptrdiff_t>(offset))) {
            return false;
        }
        offset += static_cast<std::size_t>(read);
    }
    std::uint8_t extra = 0U;
    for (;;) {
        const ssize_t read = ::pread(
            descriptor, &extra, 1U, static_cast<off_t>(expected.size()));
        if (read < 0 && errno == EINTR) {
            continue;
        }
        return read == 0;
    }
}

#endif

}  // namespace

class PrivateExecutableImage::Impl {
public:
#if defined(_WIN32)
    Impl(
        HANDLE handle_value,
        std::filesystem::path stable_path_value,
        std::filesystem::path launch_path_value,
        std::filesystem::path native_path_value,
        OwnedDirectoryChain directory_chain_value,
        std::size_t size_value) noexcept
        : handle(handle_value),
          stable_path(std::move(stable_path_value)),
          launch_path(std::move(launch_path_value)),
          native_path(std::move(native_path_value)),
          directory_chain(std::move(directory_chain_value)),
          size(size_value) {}
    ~Impl() {
        (void)delete_exact_file(handle);
        if (handle != nullptr && handle != INVALID_HANDLE_VALUE) {
            (void)::CloseHandle(handle);
        }
    }
    [[nodiscard]] bool valid() const noexcept {
        return exact_file_shape(handle, size);
    }
    [[nodiscard]] bool matches(
        const std::span<const std::uint8_t> expected) const noexcept {
        return expected.size() == size && native_matches_bytes(handle, expected);
    }
    HANDLE handle = INVALID_HANDLE_VALUE;
    std::filesystem::path stable_path;
    std::filesystem::path launch_path;
    std::filesystem::path native_path;
    OwnedDirectoryChain directory_chain;
#else
    // `descriptor` is a read-only reopen of the materialized file (see the
    // materialization call site): Linux/macOS both refuse to execute a
    // file that is open for writing anywhere, keyed on the inode's writer
    // count, not on which fd a later exec call happens to use -- so the
    // original O_RDWR descriptor the bytes were written through is closed
    // for real at materialization time, and this read-only descriptor is
    // the only one that survives for the rest of the image's lifetime.
    //
    // On Linux (RQ-CF-AGENT-026) the file is also already unlinked, and
    // `size`/`execution_path` are the original approved byte count and
    // empty, respectively -- matches() compares against that exact
    // original span for the object's whole lifetime, since nothing further
    // modifies the file after materialization.
    //
    // On macOS (RQ-CF-AGENT-031, which supersedes RQ-CF-AGENT-026's POSIX
    // unlink clause for this platform only) the file instead stays linked
    // permanently at execution_path. macOS provides no supported way to
    // relink an already-unlinked file back into the namespace (confirmed
    // empirically: linkat() sourcing a hardlink from a /dev/fd path onto a
    // zero-link inode fails with EPERM), and no true fexecve() analog
    // either (its /dev/fd workaround for exec-by-descriptor only permits
    // path lookups from the same process that opened the descriptor
    // directly, denying a forked child that merely inherited it via
    // fork() -- confirmed empirically: fstat() on the inherited descriptor
    // succeeds and reports the correct file, while access()/execve() via
    // its /dev/fd path both fail identically with EACCES). Since
    // filesystem-namespace invisibility is unavailable, macOS instead
    // ad-hoc code-signs the file immediately after the exact-byte write is
    // verified (see materialize_private_executable_image_in_verified_
    // parent()), so that AMFI refuses to exec it at the kernel level if
    // its on-disk content no longer matches that signature -- unlike
    // permission bits, chflags, or ACLs, this cannot be silently defeated
    // by a same-UID attacker merely by owning the file (they would need to
    // forge a valid signature, not just relax a mode bit they control).
    // `size` is therefore the file's exact byte count AFTER signing, not
    // the original pre-signature span -- signing legitimately changes the
    // file's content and size, so ongoing validity from here on means
    // "still exactly what materialization sealed," not "still
    // byte-identical to the original caller-supplied span" (that exact
    // check already happened, once, against the pre-signature write,
    // inside materialize()).
    Impl(
        int descriptor_value, std::size_t size_value,
        std::filesystem::path execution_path_value = {}) noexcept
        : descriptor(descriptor_value),
          execution_path(std::move(execution_path_value)),
          size(size_value) {}
    ~Impl() {
        // On macOS, execution_path is the sole remaining directory entry
        // for this image; this object's destruction is the natural, single
        // point to remove it. Safe with respect to any in-flight exec of
        // it: PrivateExecutableImage is passed to run_bounded_posix_
        // private_executable() by const reference and outlives that
        // (synchronous) call by ordinary C++ lifetime rules, and that call
        // does not return until run_posix() does, which itself does not
        // return until any child it forked has either fully resolved its
        // own exec attempt or been fully reaped. On Linux execution_path
        // is always empty and this is a no-op.
        if (!execution_path.empty()) {
            (void)::unlink(execution_path.c_str());
        }
        if (descriptor >= 0) {
            (void)::close(descriptor);
        }
    }
    [[nodiscard]] bool valid() const noexcept {
        return exact_file_shape(descriptor, size);
    }
    [[nodiscard]] bool matches(
        const std::span<const std::uint8_t> expected) const noexcept {
#if defined(__APPLE__)
        // Not applicable post-signing -- see the class doc comment above.
        // Integrity from here on is enforced by AMFI at exec time, not by
        // comparing against the pre-signature span.
        static_cast<void>(expected);
        return valid();
#else
        return expected.size() == size &&
            native_matches_bytes(descriptor, expected);
#endif
    }
    int descriptor = -1;
    std::filesystem::path execution_path;
#endif
    std::size_t size = 0U;
};

PrivateExecutableImage::PrivateExecutableImage(std::unique_ptr<Impl> impl) noexcept
    : impl_(std::move(impl)) {}
PrivateExecutableImage::PrivateExecutableImage() = default;
PrivateExecutableImage::~PrivateExecutableImage() = default;
PrivateExecutableImage::PrivateExecutableImage(PrivateExecutableImage&&) noexcept =
    default;
PrivateExecutableImage& PrivateExecutableImage::operator=(
    PrivateExecutableImage&&) noexcept = default;

bool PrivateExecutableImage::valid() const noexcept {
    return impl_ != nullptr && impl_->valid();
}

bool PrivateExecutableImage::matches_bytes(
    const std::span<const std::uint8_t> expected) const noexcept {
    return impl_ != nullptr && impl_->matches(expected);
}

const std::filesystem::path*
PrivateExecutableImage::windows_launch_target() const noexcept {
#if defined(_WIN32)
    return impl_ != nullptr && impl_->valid()
        ? &impl_->launch_path
        : nullptr;
#else
    return nullptr;
#endif
}

const std::filesystem::path*
PrivateExecutableImage::windows_native_launch_target() const noexcept {
#if defined(_WIN32)
    return impl_ != nullptr && impl_->valid() ? &impl_->native_path : nullptr;
#else
    return nullptr;
#endif
}

int PrivateExecutableImage::posix_descriptor() const noexcept {
#if defined(_WIN32)
    return -1;
#else
    return impl_ != nullptr && impl_->valid() ? impl_->descriptor : -1;
#endif
}

bool PrivateExecutableImage::posix_exec_in_child(
    char* const argv[], char* const environment[]) const noexcept {
#if defined(_WIN32)
    static_cast<void>(argv);
    static_cast<void>(environment);
    return false;
#else
    if (impl_ == nullptr || !impl_->valid()) {
        errno = EBADF;
        return false;
    }
#if defined(__APPLE__)
    if (impl_->execution_path.empty()) {
        errno = EBADF;
        return false;
    }
    ::execve(impl_->execution_path.c_str(), argv, environment);
    return false;
#else
    ::fexecve(impl_->descriptor, argv, environment);
    return false;
#endif
#endif
}

bool posix_private_exec_override(
    void* context, char* const argv[], char* const environment[]) noexcept {
    return static_cast<const PrivateExecutableImage*>(context)
        ->posix_exec_in_child(argv, environment);
}

PrivateExecutableImageMaterializationResult
materialize_private_executable_image_in_verified_parent(
    const std::filesystem::path& parent,
    const std::uint64_t expected_parent_storage_id,
    const std::uint64_t expected_parent_file_id,
    const std::uint64_t expected_parent_creation_ticks,
    const std::filesystem::path& leaf,
    const std::span<const std::uint8_t> bytes) noexcept {
    PrivateExecutableImageMaterializationResult result;
    try {
        if (!valid_request_path(parent, leaf, bytes)) {
            return result;
        }
        if (!verify_private_directory(parent).ok) {
            result.failure = PrivateExecutableImageFailure::parent_unavailable;
            return result;
        }

#if defined(_WIN32)
        const std::filesystem::path image_path = parent / leaf;
        OwnedHandle parent_handle(::CreateFileW(
            parent.c_str(), FILE_READ_ATTRIBUTES | READ_CONTROL,
            FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT, nullptr));
        if (!handle_identity_matches(
                parent_handle.get(), expected_parent_storage_id,
                expected_parent_file_id, expected_parent_creation_ticks,
                true) ||
            !verify_private_directory(parent).ok) {
            result.failure = PrivateExecutableImageFailure::parent_identity_changed;
            return result;
        }
        const HANDLE created_image_handle = ::CreateFileW(
            image_path.c_str(), GENERIC_READ | GENERIC_WRITE | DELETE,
            FILE_SHARE_READ, nullptr, CREATE_NEW,
            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OPEN_REPARSE_POINT, nullptr);
        const DWORD create_error =
            created_image_handle == INVALID_HANDLE_VALUE
            ? ::GetLastError()
            : ERROR_SUCCESS;
        OwnedImageHandle image_handle(created_image_handle);
        if (image_handle.get() == INVALID_HANDLE_VALUE) {
            result.failure =
                create_error == ERROR_FILE_EXISTS || create_error == ERROR_ALREADY_EXISTS
                ? PrivateExecutableImageFailure::already_exists
                : create_error == ERROR_ACCESS_DENIED
                    ? PrivateExecutableImageFailure::access_denied
                    : PrivateExecutableImageFailure::creation_failed;
            return result;
        }
        const bool parent_stable = handle_identity_matches(
            parent_handle.get(), expected_parent_storage_id,
            expected_parent_file_id,
            expected_parent_creation_ticks, true);
        if (!parent_stable || !write_all(image_handle.get(), bytes) ||
            !exact_file_shape(image_handle.get(), bytes.size()) ||
            !native_matches_bytes(image_handle.get(), bytes)) {
            result.failure = parent_stable
                ? PrivateExecutableImageFailure::verification_failed
                : PrivateExecutableImageFailure::parent_identity_changed;
            return result;
        }
        WindowsImageIdentity image_identity;
        if (!capture_image_identity(image_handle.get(), image_identity) ||
            image_identity.size != bytes.size()) {
            result.failure = PrivateExecutableImageFailure::verification_failed;
            return result;
        }
        const auto stable_image_path =
            stable_volume_path_for_handle(image_handle.get());
        const auto dos_image_path =
            dos_volume_path_for_handle(image_handle.get());
        const auto native_image_path =
            final_path_for_handle(image_handle.get(), VOLUME_NAME_NT);
        if (!stable_image_path.has_value() || !dos_image_path.has_value() ||
            !native_image_path.has_value() ||
            native_image_path->rfind(L"\\Device\\HarddiskVolume", 0U) != 0U) {
            result.failure =
                PrivateExecutableImageFailure::launch_transition_failed;
            return result;
        }

        // CreateProcessW must not inherit the write-capable creation handle's
        // sharing obligation. Close that handle and recover the exact file
        // object through its volume-relative file id as a read-only identity
        // anchor. A cooperating
        // writer that wins the close/reopen interval either blocks this open
        // or changes bytes that the final verification rejects. Acquire the
        // final read/delete, delete-denying handle through the non-reparse leaf
        // only while the identity anchor is live and require exact equality,
        // so a rename/replacement winner cannot redirect a later loader open.
        image_handle.close_without_delete();
        OwnedHandle transition_handle(reopen_image_by_id(
            parent_handle.get(), image_identity, GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_DELETE));
        if (transition_handle.get() == INVALID_HANDLE_VALUE) {
            result.failure = cleanup_image_by_id(
                parent_handle.get(), image_identity)
                ? PrivateExecutableImageFailure::launch_transition_failed
                : PrivateExecutableImageFailure::cleanup_failed;
            return result;
        }
        if (!image_identity_matches(transition_handle.get(), image_identity)) {
            // A reused identifier must never make cleanup authority adopt a
            // different object. Close it without deletion and fail closed.
            result.failure =
                PrivateExecutableImageFailure::launch_transition_failed;
            return result;
        }
        OwnedHandle path_handle(open_path_for_exact_image(
            *stable_image_path, GENERIC_READ | DELETE, FILE_SHARE_READ));
        if (!image_identity_matches(path_handle.get(), image_identity)) {
            path_handle.reset();
            transition_handle.reset();
            result.failure = cleanup_image_by_id(
                parent_handle.get(), image_identity)
                ? PrivateExecutableImageFailure::launch_transition_failed
                : PrivateExecutableImageFailure::cleanup_failed;
            return result;
        }
        OwnedImageHandle launch_handle(path_handle.release());
        transition_handle.reset();
        if (!exact_file_shape(launch_handle.get(), bytes.size()) ||
            !native_matches_bytes(launch_handle.get(), bytes) ||
            !handle_identity_matches(
                parent_handle.get(), expected_parent_storage_id,
                expected_parent_file_id, expected_parent_creation_ticks,
                true)) {
            result.failure = PrivateExecutableImageFailure::verification_failed;
            return result;
        }
        // Retain a no-delete-share handle for every renameable directory below
        // the handle-derived stable local-volume root through the private
        // parent, then repeat both stable- and DOS-name identity checks. The
        // loader needs the DOS form, but bounded creation later verifies the
        // suspended process's native image name against this same retained
        // object before resume. Together these checks deny ancestor and DOS-
        // namespace redirection rather than trusting either pathname alone.
        OwnedDirectoryChain directory_chain;
        if (!directory_chain.lock(stable_image_path->parent_path())) {
            result.failure =
                PrivateExecutableImageFailure::launch_transition_failed;
            return result;
        }
        OwnedHandle final_path_check(open_path_for_exact_image(
            *stable_image_path, GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_DELETE));
        OwnedHandle launch_path_check(open_path_for_exact_image(
            *dos_image_path, GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_DELETE));
        if (!image_identity_matches(final_path_check.get(), image_identity) ||
            !image_identity_matches(
                launch_path_check.get(), image_identity) ||
            !image_identity_matches(launch_handle.get(), image_identity) ||
            !handle_identity_matches(
                parent_handle.get(), expected_parent_storage_id,
                expected_parent_file_id, expected_parent_creation_ticks,
                true)) {
            result.failure =
                PrivateExecutableImageFailure::launch_transition_failed;
            return result;
        }
        launch_path_check.reset();
        final_path_check.reset();
        parent_handle.reset();
        // Finish every potentially throwing copy before the new-expression.
        // Allocation then precedes the no-throw handle/chain transfers, so an
        // exception cannot strand raw Windows authority outside RAII ownership.
        std::filesystem::path retained_image_path = *stable_image_path;
        std::filesystem::path retained_launch_path = *dos_image_path;
        std::filesystem::path retained_native_path = *native_image_path;
        auto impl = std::unique_ptr<PrivateExecutableImage::Impl>(
            new PrivateExecutableImage::Impl(
                launch_handle.get(), std::move(retained_image_path),
                std::move(retained_launch_path),
                std::move(retained_native_path),
                std::move(directory_chain), bytes.size()));
        (void)launch_handle.release();
#else
        const std::string leaf_name = leaf.native();
        OwnedDescriptor parent_descriptor(::open(
            parent.c_str(), O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC));
        if (!descriptor_identity_matches(
                parent_descriptor.get(), expected_parent_storage_id,
                expected_parent_file_id, expected_parent_creation_ticks) ||
            !verify_private_directory(parent).ok) {
            result.failure = PrivateExecutableImageFailure::parent_identity_changed;
            return result;
        }
        const int created_image_descriptor = ::openat(
            parent_descriptor.get(), leaf_name.c_str(),
            O_RDWR | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC, 0000);
        const int create_error =
            created_image_descriptor < 0 ? errno : 0;
        OwnedLinkedImageDescriptor image_descriptor(
            created_image_descriptor, parent_descriptor.get(), &leaf_name);
        if (image_descriptor.get() < 0) {
            result.failure = create_error == EEXIST
                ? PrivateExecutableImageFailure::already_exists
                : create_error == EACCES || create_error == EPERM
                    ? PrivateExecutableImageFailure::access_denied
                    : PrivateExecutableImageFailure::creation_failed;
            return result;
        }
#if defined(__APPLE__)
        // RQ-CF-AGENT-031 supersedes only RQ-CF-AGENT-026's POSIX unlink
        // clause, for macOS only: this image stays linked at real_path for
        // its whole lifetime instead of being unlinked (see Impl's doc
        // comment for the full reasoning). image_descriptor's RAII default
        // (linked_ = true) still auto-unlinks it on every failure return
        // below, right up until release() near the end of this block hands
        // ownership of the sealed read-only descriptor to Impl.
        const bool parent_stable = descriptor_identity_matches(
            parent_descriptor.get(), expected_parent_storage_id,
            expected_parent_file_id, expected_parent_creation_ticks);
        // Seal to the final owner read/execute-only mode *before*
        // verification, exactly like Linux: exact_file_shape() requires
        // mode 0500 unconditionally, so verifying while the file is still
        // relaxed for codesign(1) below would always fail the mode check
        // (confirmed empirically: an earlier ordering that verified while
        // still 0600 failed shape_ok on every real macOS CI run, deter-
        // ministically, before codesign ever ran).
        const bool chmod_sealed_ok = parent_stable &&
            ::fchmod(image_descriptor.get(), 0500) == 0;
        const int chmod_sealed_errno = errno;
        const bool wrote_ok = chmod_sealed_ok && write_all(image_descriptor.get(), bytes);
        const bool pre_sign_shape_ok =
            wrote_ok && exact_file_shape(image_descriptor.get(), bytes.size());
        const bool pre_sign_bytes_ok =
            pre_sign_shape_ok && native_matches_bytes(image_descriptor.get(), bytes);
        {
            struct stat pre_sign_status {};
            const int pre_sign_fstat_rc =
                ::fstat(image_descriptor.get(), &pre_sign_status);
            char diagnostic[384];
            const int diagnostic_length = ::snprintf(
                diagnostic, sizeof(diagnostic),
                "MACOS_MATERIALIZE_DIAG stage=pre-sign parent_stable=%d "
                "chmod_sealed_ok=%d chmod_sealed_errno=%d wrote_ok=%d "
                "shape_ok=%d bytes_ok=%d fstat_rc=%d st_mode=%o "
                "st_nlink=%d st_size=%lld expected_size=%zu\n",
                parent_stable ? 1 : 0, chmod_sealed_ok ? 1 : 0,
                chmod_sealed_errno, wrote_ok ? 1 : 0,
                pre_sign_shape_ok ? 1 : 0, pre_sign_bytes_ok ? 1 : 0,
                pre_sign_fstat_rc,
                pre_sign_fstat_rc == 0
                    ? static_cast<unsigned>(pre_sign_status.st_mode)
                    : 0U,
                pre_sign_fstat_rc == 0
                    ? static_cast<int>(pre_sign_status.st_nlink)
                    : -1,
                pre_sign_fstat_rc == 0
                    ? static_cast<long long>(pre_sign_status.st_size)
                    : -1LL,
                bytes.size());
            if (diagnostic_length > 0) {
                (void)::write(
                    STDERR_FILENO, diagnostic,
                    static_cast<std::size_t>(diagnostic_length));
            }
        }
        if (!parent_stable || !chmod_sealed_ok || !wrote_ok || !pre_sign_shape_ok ||
            !pre_sign_bytes_ok) {
            result.failure = !parent_stable
                ? PrivateExecutableImageFailure::parent_identity_changed
                : PrivateExecutableImageFailure::verification_failed;
            return result;
        }
        // The exact-byte check above is the only point in this image's
        // life where its content is compared against the caller-supplied
        // `bytes` span: ad-hoc code-signing legitimately changes both the
        // file's content and size (see Impl's doc comment), so this must
        // happen before signing, against the raw, pre-signature write.
        const std::filesystem::path real_path = parent / leaf_name;
        // Briefly relax to owner rw so codesign(1) -- a fresh open() by
        // path -- can write the signature; restored to the verified,
        // sealed 0500 immediately after, regardless of signing outcome,
        // so this file is never left more permissive than sealed on any
        // failure path either.
        const bool chmod_rw_ok = ::fchmod(image_descriptor.get(), 0600) == 0;
        const int chmod_rw_errno = errno;
        const bool signed_ok = chmod_rw_ok && codesign_in_place(real_path.native());
        // codesign(1) does not necessarily rewrite the file in place: CI
        // evidence shows it can write a replacement and rename it over
        // the original (post-sign fstat showing a mode inherited from the
        // pre-sign 0600 relax above, on a real Mach-O binary, even though
        // this exact fchmod reported success) -- so image_descriptor may
        // now refer to a detached, no-longer-linked inode rather than
        // whatever codesign actually left at real_path. Restore via the
        // path, not the stale descriptor, to guarantee this reaches
        // whichever file is actually linked there now.
        const bool chmod_ro_ok = chmod_rw_ok && ::chmod(real_path.c_str(), 0500) == 0;
        const int chmod_ro_errno = errno;
        {
            char diagnostic[256];
            const int diagnostic_length = ::snprintf(
                diagnostic, sizeof(diagnostic),
                "MACOS_MATERIALIZE_DIAG stage=sign chmod_rw_ok=%d "
                "chmod_rw_errno=%d signed_ok=%d chmod_ro_ok=%d "
                "chmod_ro_errno=%d\n",
                chmod_rw_ok ? 1 : 0, chmod_rw_errno, signed_ok ? 1 : 0,
                chmod_ro_ok ? 1 : 0, chmod_ro_errno);
            if (diagnostic_length > 0) {
                (void)::write(
                    STDERR_FILENO, diagnostic,
                    static_cast<std::size_t>(diagnostic_length));
            }
        }
        if (!chmod_rw_ok || !signed_ok || !chmod_ro_ok) {
            parent_descriptor.reset();
            result.failure = PrivateExecutableImageFailure::verification_failed;
            return result;
        }
        parent_descriptor.reset();
        // image_descriptor was opened O_RDWR to write the executable's
        // bytes into it (then briefly relaxed to 0600 for codesign(1)
        // above); Linux and macOS both refuse to execute a file that is
        // still open for writing anywhere (ETXTBSY), keyed on the inode's
        // writer count, not on which fd a later exec call happens to use
        // -- so the O_RDWR original itself must actually close. Because
        // this image stays linked (unlike Linux's unlink-then-reopen-via-
        // /proc/self/fd), sealing it read-only is simply an ordinary
        // reopen by its real path -- ordinary paths have none of /dev/fd's
        // same-process restriction (see the header for why that matters
        // for this platform specifically).
        const int readonly_image_descriptor =
            ::open(real_path.c_str(), O_RDONLY | O_CLOEXEC);
        const int reopen_errno = errno;
        if (readonly_image_descriptor < 0) {
            char diagnostic[192];
            const int diagnostic_length = ::snprintf(
                diagnostic, sizeof(diagnostic),
                "MACOS_MATERIALIZE_DIAG stage=reopen rc=%d errno=%d\n",
                readonly_image_descriptor, reopen_errno);
            if (diagnostic_length > 0) {
                (void)::write(
                    STDERR_FILENO, diagnostic,
                    static_cast<std::size_t>(diagnostic_length));
            }
            result.failure = PrivateExecutableImageFailure::verification_failed;
            return result;
        }
        struct stat signed_status {};
        const int post_sign_fstat_rc =
            ::fstat(readonly_image_descriptor, &signed_status);
        const int post_sign_fstat_errno = errno;
        {
            char diagnostic[320];
            const int diagnostic_length = ::snprintf(
                diagnostic, sizeof(diagnostic),
                "MACOS_MATERIALIZE_DIAG stage=post-sign fstat_rc=%d "
                "fstat_errno=%d st_mode=%o st_uid=%d euid=%d st_nlink=%d "
                "st_size=%lld\n",
                post_sign_fstat_rc, post_sign_fstat_rc == 0 ? 0 : post_sign_fstat_errno,
                post_sign_fstat_rc == 0
                    ? static_cast<unsigned>(signed_status.st_mode)
                    : 0U,
                post_sign_fstat_rc == 0
                    ? static_cast<int>(signed_status.st_uid)
                    : -1,
                static_cast<int>(::geteuid()),
                post_sign_fstat_rc == 0
                    ? static_cast<int>(signed_status.st_nlink)
                    : -1,
                post_sign_fstat_rc == 0
                    ? static_cast<long long>(signed_status.st_size)
                    : -1LL);
            if (diagnostic_length > 0) {
                (void)::write(
                    STDERR_FILENO, diagnostic,
                    static_cast<std::size_t>(diagnostic_length));
            }
        }
        if (post_sign_fstat_rc != 0) {
            ::close(readonly_image_descriptor);
            result.failure = PrivateExecutableImageFailure::verification_failed;
            return result;
        }
        // `size` from here on is the file's exact byte count after
        // signing, not bytes.size() -- see Impl's doc comment.
        auto impl = std::make_unique<PrivateExecutableImage::Impl>(
            readonly_image_descriptor,
            static_cast<std::size_t>(signed_status.st_size), real_path);
        ::close(image_descriptor.release());
#else
        const bool unlinked = ::unlinkat(
            parent_descriptor.get(), leaf_name.c_str(), 0) == 0;
        if (!unlinked) {
            result.failure = PrivateExecutableImageFailure::cleanup_failed;
            return result;
        }
        image_descriptor.mark_unlinked();
        const bool parent_stable = descriptor_identity_matches(
            parent_descriptor.get(), expected_parent_storage_id,
            expected_parent_file_id, expected_parent_creation_ticks);
        parent_descriptor.reset();
        if (!parent_stable ||
            ::fchmod(image_descriptor.get(), 0500) != 0 ||
            !write_all(image_descriptor.get(), bytes) ||
            !exact_file_shape(image_descriptor.get(), bytes.size()) ||
            !native_matches_bytes(image_descriptor.get(), bytes)) {
            result.failure = !parent_stable
                ? PrivateExecutableImageFailure::parent_identity_changed
                : PrivateExecutableImageFailure::verification_failed;
            return result;
        }
        // image_descriptor was opened O_RDWR to write the executable's bytes
        // into it; Linux refuses to execute a file that is still open for
        // writing anywhere (ETXTBSY), keyed on the inode's writer count,
        // not on which fd a later exec call happens to use -- so a second
        // fd opened on the same still-writable file does not help on its
        // own; the O_RDWR original itself must actually close. Re-open the
        // same already-unlinked, already-verified file read-only via
        // /proc/self/fd -- a kernel-guaranteed alias to this exact open
        // file description, not a path-based lookup, so it carries none of
        // the TOCTOU risk a real path reopen would -- then release and
        // close the O_RDWR original, permanently sealing the image against
        // further writes for the rest of its lifetime. The read-only
        // reopen becomes the image's sole descriptor from here on, used
        // both for ongoing verification and for exec (fexecve()).
        const std::string reopen_path =
            "/proc/self/fd/" + std::to_string(image_descriptor.get());
        const int readonly_image_descriptor =
            ::open(reopen_path.c_str(), O_RDONLY | O_CLOEXEC);
        if (readonly_image_descriptor < 0) {
            result.failure = PrivateExecutableImageFailure::verification_failed;
            return result;
        }
        auto impl = std::make_unique<PrivateExecutableImage::Impl>(
            readonly_image_descriptor, bytes.size());
        ::close(image_descriptor.release());
#endif
#endif
        PrivateExecutableImage image(std::move(impl));
        if (!image.valid() || !image.matches_bytes(bytes)) {
            result.failure = PrivateExecutableImageFailure::verification_failed;
            return result;
        }
        result.image.emplace(std::move(image));
        result.materialized = true;
        result.failure = PrivateExecutableImageFailure::none;
        return result;
    } catch (...) {
        return result;
    }
}

}  // namespace copperfin::platform
