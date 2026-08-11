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

PhysicalPathContainmentFailure inspect_direct_components(
    const std::filesystem::path& root,
    const std::filesystem::path& relative_path) {
#if defined(_WIN32)
    auto inspect = [](const std::filesystem::path& candidate) {
        const DWORD attributes = ::GetFileAttributesW(candidate.c_str());
        if (attributes == INVALID_FILE_ATTRIBUTES) {
            return PhysicalPathContainmentFailure::path_unavailable;
        }
        return (attributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0U
            ? PhysicalPathContainmentFailure::indirect_component
            : PhysicalPathContainmentFailure::none;
    };

    if (const DWORD root_attributes = ::GetFileAttributesW(root.c_str());
        root_attributes == INVALID_FILE_ATTRIBUTES) {
        return PhysicalPathContainmentFailure::root_unavailable;
    }
    std::filesystem::path current = root;
    for (const auto& part : relative_path) {
        if (part == ".") {
            continue;
        }
        current /= part;
        if (const auto failure = inspect(current);
            failure != PhysicalPathContainmentFailure::none) {
            return failure;
        }
    }
#else
    struct stat root_status{};
    if (::stat(root.c_str(), &root_status) != 0) {
        return PhysicalPathContainmentFailure::root_unavailable;
    }

    std::filesystem::path current = root;
    for (const auto& part : relative_path) {
        if (part == ".") {
            continue;
        }
        current /= part;
        struct stat current_status{};
        if (::lstat(current.c_str(), &current_status) != 0) {
            return PhysicalPathContainmentFailure::path_unavailable;
        }
        if (S_ISLNK(current_status.st_mode)) {
            return PhysicalPathContainmentFailure::indirect_component;
        }
        if (current_status.st_dev != root_status.st_dev) {
            return PhysicalPathContainmentFailure::cross_device_component;
        }
    }
#endif
    return PhysicalPathContainmentFailure::none;
}

std::optional<PhysicalPathIdentity> read_direct_identity(
    const std::filesystem::path& path) {
#if defined(_WIN32)
    const HANDLE handle = ::CreateFileW(
        path.c_str(),
        FILE_READ_ATTRIBUTES,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        nullptr,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT,
        nullptr);
    if (handle == INVALID_HANDLE_VALUE) {
        return std::nullopt;
    }

    BY_HANDLE_FILE_INFORMATION information{};
    const bool read = ::GetFileInformationByHandle(handle, &information) != 0;
    ::CloseHandle(handle);
    if (!read || (information.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0U) {
        return std::nullopt;
    }

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
        .link_count = information.nNumberOfLinks
    };
#else
    struct stat status{};
    if (::lstat(path.c_str(), &status) != 0 || S_ISLNK(status.st_mode)) {
        return std::nullopt;
    }
#if defined(__APPLE__)
    const std::uint64_t modified_ticks =
        static_cast<std::uint64_t>(status.st_mtimespec.tv_sec) * 1'000'000'000ULL +
        static_cast<std::uint64_t>(status.st_mtimespec.tv_nsec);
#else
    const std::uint64_t modified_ticks =
        static_cast<std::uint64_t>(status.st_mtim.tv_sec) * 1'000'000'000ULL +
        static_cast<std::uint64_t>(status.st_mtim.tv_nsec);
#endif
    return PhysicalPathIdentity{
        .storage_id = static_cast<std::uint64_t>(status.st_dev),
        .file_id = static_cast<std::uint64_t>(status.st_ino),
        .file_size = static_cast<std::uint64_t>(status.st_size),
        .modified_ticks = modified_ticks,
        .link_count = static_cast<std::uint64_t>(status.st_nlink)
    };
#endif
}

PhysicalFileSnapshotResult failed_snapshot(
    const PhysicalPathContainmentFailure failure) {
    return {
        .ok = false,
        .bytes = {},
        .containment = failed_result(failure),
        .failure = failure
    };
}

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

    if (const auto component_failure =
            inspect_direct_components(component_root, *lexical_relative);
        component_failure != PhysicalPathContainmentFailure::none) {
        return failed_result(component_failure);
    }

    const std::filesystem::path canonical_path =
        std::filesystem::canonical(absolute_path, filesystem_error);
    if (filesystem_error) {
        return failed_result(PhysicalPathContainmentFailure::path_unavailable);
    }
    if (!contained_relative_path(canonical_path, canonical_root).has_value()) {
        return failed_result(PhysicalPathContainmentFailure::outside_root);
    }

    const auto identity = read_direct_identity(canonical_path);
    if (!identity.has_value()) {
        return failed_result(PhysicalPathContainmentFailure::indirect_component);
    }

    return {
        .allowed = true,
        .canonical_path = canonical_path,
        .identity = *identity,
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
    const PhysicalPathIdentity before_identity{
        .storage_id = before_information.dwVolumeSerialNumber,
        .file_id =
            (static_cast<std::uint64_t>(before_information.nFileIndexHigh) << 32U) |
            before_information.nFileIndexLow,
        .file_size =
            (static_cast<std::uint64_t>(before_information.nFileSizeHigh) << 32U) |
            before_information.nFileSizeLow,
        .modified_ticks =
            (static_cast<std::uint64_t>(before_information.ftLastWriteTime.dwHighDateTime) << 32U) |
            before_information.ftLastWriteTime.dwLowDateTime,
        .link_count = before_information.nNumberOfLinks
    };
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
    const PhysicalPathIdentity after_identity{
        .storage_id = after_information.dwVolumeSerialNumber,
        .file_id =
            (static_cast<std::uint64_t>(after_information.nFileIndexHigh) << 32U) |
            after_information.nFileIndexLow,
        .file_size =
            (static_cast<std::uint64_t>(after_information.nFileSizeHigh) << 32U) |
            after_information.nFileSizeLow,
        .modified_ticks =
            (static_cast<std::uint64_t>(after_information.ftLastWriteTime.dwHighDateTime) << 32U) |
            after_information.ftLastWriteTime.dwLowDateTime,
        .link_count = after_information.nNumberOfLinks
    };
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
#if defined(__APPLE__)
    const std::uint64_t before_modified_ticks =
        static_cast<std::uint64_t>(before_status.st_mtimespec.tv_sec) * 1'000'000'000ULL +
        static_cast<std::uint64_t>(before_status.st_mtimespec.tv_nsec);
#else
    const std::uint64_t before_modified_ticks =
        static_cast<std::uint64_t>(before_status.st_mtim.tv_sec) * 1'000'000'000ULL +
        static_cast<std::uint64_t>(before_status.st_mtim.tv_nsec);
#endif
    const PhysicalPathIdentity before_identity{
        .storage_id = static_cast<std::uint64_t>(before_status.st_dev),
        .file_id = static_cast<std::uint64_t>(before_status.st_ino),
        .file_size = static_cast<std::uint64_t>(before_status.st_size),
        .modified_ticks = before_modified_ticks,
        .link_count = static_cast<std::uint64_t>(before_status.st_nlink)
    };
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
    ::close(descriptor);
    if (!after_read) {
        return failed_snapshot(PhysicalPathContainmentFailure::read_failed);
    }
#if defined(__APPLE__)
    const std::uint64_t after_modified_ticks =
        static_cast<std::uint64_t>(after_status.st_mtimespec.tv_sec) * 1'000'000'000ULL +
        static_cast<std::uint64_t>(after_status.st_mtimespec.tv_nsec);
#else
    const std::uint64_t after_modified_ticks =
        static_cast<std::uint64_t>(after_status.st_mtim.tv_sec) * 1'000'000'000ULL +
        static_cast<std::uint64_t>(after_status.st_mtim.tv_nsec);
#endif
    const PhysicalPathIdentity after_identity{
        .storage_id = static_cast<std::uint64_t>(after_status.st_dev),
        .file_id = static_cast<std::uint64_t>(after_status.st_ino),
        .file_size = static_cast<std::uint64_t>(after_status.st_size),
        .modified_ticks = after_modified_ticks,
        .link_count = static_cast<std::uint64_t>(after_status.st_nlink)
    };
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
