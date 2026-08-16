// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/private_executable_image.h"

#include "copperfin/platform/private_directory.h"

#include <algorithm>
#include <array>
#include <limits>
#include <utility>
#include <vector>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#else
#include <cerrno>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
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
            std::filesystem::path current = directory.root_path();
            if (current.empty()) {
                reset();
                return false;
            }
            // A drive or UNC share root cannot be renamed as a directory entry
            // and Windows may refuse a delete-denying open on the volume root.
            // Lock every actual directory entry below that stable root.
            for (const auto& component : directory.relative_path()) {
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
    return descriptor >= 0 && ::fstat(descriptor, &status) == 0 &&
        S_ISREG(status.st_mode) && status.st_uid == ::geteuid() &&
        (status.st_mode & 07777) == 0500 && status.st_nlink == 0 &&
        static_cast<std::uint64_t>(status.st_size) == expected_size;
}

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
        std::filesystem::path path_value,
        OwnedDirectoryChain directory_chain_value,
        std::size_t size_value) noexcept
        : handle(handle_value),
          path(std::move(path_value)),
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
    std::filesystem::path path;
    OwnedDirectoryChain directory_chain;
#else
    Impl(int descriptor_value, std::size_t size_value) noexcept
        : descriptor(descriptor_value), size(size_value) {}
    ~Impl() {
        if (descriptor >= 0) {
            (void)::close(descriptor);
        }
    }
    [[nodiscard]] bool valid() const noexcept {
        return exact_file_shape(descriptor, size);
    }
    [[nodiscard]] bool matches(
        const std::span<const std::uint8_t> expected) const noexcept {
        return expected.size() == size &&
            native_matches_bytes(descriptor, expected);
    }
    int descriptor = -1;
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
    return impl_ != nullptr && impl_->valid() ? &impl_->path : nullptr;
#else
    return nullptr;
#endif
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
            image_path, GENERIC_READ | DELETE, FILE_SHARE_READ));
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
        // CreateProcessW reopens the image by pathname. Retain a no-delete-share
        // handle for every renameable directory below the stable filesystem
        // root through the private parent, then repeat the leaf identity check. This prevents any
        // ancestor name from being renamed or replaced between materialization
        // and the loader's open of the authenticated leaf.
        OwnedDirectoryChain directory_chain;
        if (!directory_chain.lock(parent)) {
            result.failure =
                PrivateExecutableImageFailure::launch_transition_failed;
            return result;
        }
        OwnedHandle final_path_check(open_path_for_exact_image(
            image_path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_DELETE));
        if (!image_identity_matches(final_path_check.get(), image_identity) ||
            !image_identity_matches(launch_handle.get(), image_identity) ||
            !handle_identity_matches(
                parent_handle.get(), expected_parent_storage_id,
                expected_parent_file_id, expected_parent_creation_ticks,
                true)) {
            result.failure =
                PrivateExecutableImageFailure::launch_transition_failed;
            return result;
        }
        final_path_check.reset();
        parent_handle.reset();
        auto impl = std::make_unique<PrivateExecutableImage::Impl>(
            launch_handle.get(), image_path, std::move(directory_chain),
            bytes.size());
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
        auto impl = std::make_unique<PrivateExecutableImage::Impl>(
            image_descriptor.get(), bytes.size());
        (void)image_descriptor.release();
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
