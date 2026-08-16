// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/private_executable_image.h"

#include "copperfin/platform/private_directory.h"

#include <algorithm>
#include <array>
#include <limits>
#include <utility>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
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
    const bool directory) noexcept {
    BY_HANDLE_FILE_INFORMATION information{};
    return handle != nullptr && handle != INVALID_HANDLE_VALUE &&
        ::GetFileInformationByHandle(handle, &information) != 0 &&
        ((information.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0U) ==
            directory &&
        (information.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) == 0U &&
        information.dwVolumeSerialNumber == expected_storage_id &&
        ((static_cast<std::uint64_t>(information.nFileIndexHigh) << 32U) |
         information.nFileIndexLow) == expected_file_id;
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

void delete_exact_file(const HANDLE handle) noexcept {
    if (handle == nullptr || handle == INVALID_HANDLE_VALUE) {
        return;
    }
    FILE_DISPOSITION_INFO disposition{.DeleteFile = TRUE};
    (void)::SetFileInformationByHandle(
        handle, FileDispositionInfo, &disposition,
        static_cast<DWORD>(sizeof(disposition)));
}

#else

bool descriptor_identity_matches(
    const int descriptor,
    const std::uint64_t expected_storage_id,
    const std::uint64_t expected_file_id) noexcept {
    struct stat status{};
    return descriptor >= 0 && ::fstat(descriptor, &status) == 0 &&
        S_ISDIR(status.st_mode) &&
        static_cast<std::uint64_t>(status.st_dev) == expected_storage_id &&
        static_cast<std::uint64_t>(status.st_ino) == expected_file_id &&
        status.st_uid == ::geteuid() && (status.st_mode & 07777) == 0700;
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
    Impl(HANDLE handle_value, std::size_t size_value) noexcept
        : handle(handle_value), size(size_value) {}
    ~Impl() {
        delete_exact_file(handle);
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

PrivateExecutableImageMaterializationResult
materialize_private_executable_image_in_verified_parent(
    const std::filesystem::path& parent,
    const std::uint64_t expected_parent_storage_id,
    const std::uint64_t expected_parent_file_id,
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
        const HANDLE parent_handle = ::CreateFileW(
            parent.c_str(), FILE_READ_ATTRIBUTES | READ_CONTROL,
            FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT, nullptr);
        if (!handle_identity_matches(
                parent_handle, expected_parent_storage_id,
                expected_parent_file_id, true) ||
            !verify_private_directory(parent).ok) {
            if (parent_handle != nullptr && parent_handle != INVALID_HANDLE_VALUE) {
                (void)::CloseHandle(parent_handle);
            }
            result.failure = PrivateExecutableImageFailure::parent_identity_changed;
            return result;
        }
        const HANDLE image_handle = ::CreateFileW(
            (parent / leaf).c_str(), GENERIC_READ | GENERIC_WRITE | DELETE,
            FILE_SHARE_READ, nullptr, CREATE_NEW,
            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OPEN_REPARSE_POINT, nullptr);
        const DWORD create_error =
            image_handle == INVALID_HANDLE_VALUE ? ::GetLastError() : ERROR_SUCCESS;
        if (image_handle == INVALID_HANDLE_VALUE) {
            (void)::CloseHandle(parent_handle);
            result.failure =
                create_error == ERROR_FILE_EXISTS || create_error == ERROR_ALREADY_EXISTS
                ? PrivateExecutableImageFailure::already_exists
                : create_error == ERROR_ACCESS_DENIED
                    ? PrivateExecutableImageFailure::access_denied
                    : PrivateExecutableImageFailure::creation_failed;
            return result;
        }
        const bool parent_stable = handle_identity_matches(
            parent_handle, expected_parent_storage_id, expected_parent_file_id, true);
        (void)::CloseHandle(parent_handle);
        if (!parent_stable || !write_all(image_handle, bytes) ||
            !exact_file_shape(image_handle, bytes.size()) ||
            !native_matches_bytes(image_handle, bytes)) {
            delete_exact_file(image_handle);
            (void)::CloseHandle(image_handle);
            result.failure = parent_stable
                ? PrivateExecutableImageFailure::verification_failed
                : PrivateExecutableImageFailure::parent_identity_changed;
            return result;
        }
        auto impl = std::make_unique<PrivateExecutableImage::Impl>(
            image_handle, bytes.size());
#else
        const int parent_descriptor = ::open(
            parent.c_str(), O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
        if (!descriptor_identity_matches(
                parent_descriptor, expected_parent_storage_id,
                expected_parent_file_id) ||
            !verify_private_directory(parent).ok) {
            if (parent_descriptor >= 0) {
                (void)::close(parent_descriptor);
            }
            result.failure = PrivateExecutableImageFailure::parent_identity_changed;
            return result;
        }
        const std::string leaf_name = leaf.native();
        const int image_descriptor = ::openat(
            parent_descriptor, leaf_name.c_str(),
            O_RDWR | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC, 0000);
        if (image_descriptor < 0) {
            const int create_error = errno;
            (void)::close(parent_descriptor);
            result.failure = create_error == EEXIST
                ? PrivateExecutableImageFailure::already_exists
                : create_error == EACCES || create_error == EPERM
                    ? PrivateExecutableImageFailure::access_denied
                    : PrivateExecutableImageFailure::creation_failed;
            return result;
        }
        const bool unlinked = ::unlinkat(
            parent_descriptor, leaf_name.c_str(), 0) == 0;
        const bool parent_stable = descriptor_identity_matches(
            parent_descriptor, expected_parent_storage_id, expected_parent_file_id);
        (void)::close(parent_descriptor);
        if (!unlinked || !parent_stable ||
            ::fchmod(image_descriptor, 0500) != 0 ||
            !write_all(image_descriptor, bytes) ||
            !exact_file_shape(image_descriptor, bytes.size()) ||
            !native_matches_bytes(image_descriptor, bytes)) {
            (void)::close(image_descriptor);
            result.failure = !parent_stable
                ? PrivateExecutableImageFailure::parent_identity_changed
                : !unlinked
                    ? PrivateExecutableImageFailure::cleanup_failed
                    : PrivateExecutableImageFailure::verification_failed;
            return result;
        }
        auto impl = std::make_unique<PrivateExecutableImage::Impl>(
            image_descriptor, bytes.size());
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
