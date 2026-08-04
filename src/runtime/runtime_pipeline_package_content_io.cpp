// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "runtime_pipeline_support.h"

#include "copperfin/security/physical_path_containment.h"
#if defined(COPPERFIN_ENABLE_RUNTIME_PIPELINE_TEST_HOOKS)
#include "runtime_pipeline_test_hooks.h"
#endif

#include <atomic>
#include <array>
#include <chrono>
#include <fstream>
#include <iostream>
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
#include <cerrno>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace copperfin::runtime::runtime_pipeline_detail {
namespace {

#if defined(COPPERFIN_ENABLE_RUNTIME_PIPELINE_TEST_HOOKS)
void trace_content_root_failure(
    const std::string_view stage,
    const std::filesystem::path& path) {
    const int saved_errno = errno;
    std::cerr << "RUNTIME_PIPELINE_CONTENT_ROOT_FAILURE stage=" << stage
              << " errno=" << saved_errno
              << " path=" << copperfin::platform::path_to_utf8_string(path)
              << "\n";
}
#else
void trace_content_root_failure(
    const std::string_view,
    const std::filesystem::path&) {
}
#endif

bool has_windows_drive_designator(const std::string& value) {
    return value.size() >= 2U &&
        std::isalpha(static_cast<unsigned char>(value[0])) != 0 &&
        value[1] == ':';
}

bool is_unc_path(const std::string& value) {
    return value.size() >= 2U &&
        ((value[0] == '\\' && value[1] == '\\') ||
         (value[0] == '/' && value[1] == '/'));
}

#if !defined(_WIN32)
bool is_fd_backed_path(const std::filesystem::path& path) {
    const std::string value = copperfin::platform::path_to_utf8_string(path);
    return value.rfind("/proc/self/fd/", 0U) == 0U ||
        value.rfind("/dev/fd/", 0U) == 0U;
}

std::optional<int> fd_from_path(const std::filesystem::path& path) {
    const std::string value = copperfin::platform::path_to_utf8_string(path);
    std::string_view suffix;
    if (value.rfind("/proc/self/fd/", 0U) == 0U) {
        suffix = std::string_view(value).substr(14U);
    } else if (value.rfind("/dev/fd/", 0U) == 0U) {
        suffix = std::string_view(value).substr(8U);
    } else {
        return std::nullopt;
    }
    if (suffix.empty() ||
        std::any_of(suffix.begin(), suffix.end(), [](const char value) {
            return value < '0' || value > '9';
        })) {
        return std::nullopt;
    }
    errno = 0;
    char* end = nullptr;
    const long descriptor = std::strtol(std::string(suffix).c_str(), &end, 10);
    if (errno != 0 || end == nullptr || *end != '\0' || descriptor < 0 ||
        descriptor > std::numeric_limits<int>::max()) {
        return std::nullopt;
    }
    return static_cast<int>(descriptor);
}
#endif

bool is_windows_reserved_device_name(std::string component) {
    const std::size_t extension = component.find('.');
    if (extension != std::string::npos) {
        component.erase(extension);
    }
    while (!component.empty() &&
           (component.back() == '.' || component.back() == ' ')) {
        component.pop_back();
    }
    std::transform(
        component.begin(),
        component.end(),
        component.begin(),
        [](const unsigned char ch) {
            return ch >= 'a' && ch <= 'z'
                ? static_cast<char>(ch - 'a' + 'A')
                : static_cast<char>(ch);
        });
    if (component == "CON" || component == "PRN" || component == "AUX" ||
        component == "NUL") {
        return true;
    }
    const bool numbered_device =
        component.rfind("COM", 0U) == 0U || component.rfind("LPT", 0U) == 0U;
    if (!numbered_device) {
        return false;
    }
    if (component.size() == 4U) {
        return component[3] >= '1' && component[3] <= '9';
    }
    if (component.size() != 5U ||
        static_cast<unsigned char>(component[3]) != 0xC2U) {
        return false;
    }
    const unsigned char superscript = static_cast<unsigned char>(component[4]);
    return superscript == 0xB9U || superscript == 0xB2U || superscript == 0xB3U;
}

bool is_portable_package_component(const std::filesystem::path& component) {
    const std::string value = copperfin::platform::path_to_utf8_string(component);
    if (value.empty() || value == "." || value == ".." ||
        value.back() == '.' || value.back() == ' ' ||
        is_windows_reserved_device_name(value)) {
        return false;
    }
    return std::none_of(value.begin(), value.end(), [](const unsigned char ch) {
        return ch < 0x20U || ch == '<' || ch == '>' || ch == ':' || ch == '"' ||
            ch == '|' || ch == '?' || ch == '*';
    });
}

std::optional<std::filesystem::path> admitted_relative_path(
    const std::filesystem::path& path) {
    std::string normalized = copperfin::platform::path_to_utf8_string(path);
    std::replace(normalized.begin(), normalized.end(), '\\', '/');
    const std::filesystem::path relative = copperfin::platform::path_from_utf8_string(normalized);
    if (normalized.empty() || has_windows_drive_designator(normalized) ||
        is_unc_path(normalized) || relative.has_root_path() ||
        relative.is_absolute() || relative.filename().empty()) {
        return std::nullopt;
    }
    for (const auto& component : relative) {
        if (!is_portable_package_component(component)) {
            return std::nullopt;
        }
    }
    return relative.lexically_normal();
}

enum class DirectDirectoryState {
    direct,
    rejected,
    unavailable
};

DirectDirectoryState inspect_direct_directory(const std::filesystem::path& path) {
#if defined(_WIN32)
    const DWORD attributes = ::GetFileAttributesW(path.c_str());
    if (attributes == INVALID_FILE_ATTRIBUTES) {
        return DirectDirectoryState::unavailable;
    }
    return (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0U &&
            (attributes & FILE_ATTRIBUTE_REPARSE_POINT) == 0U
        ? DirectDirectoryState::direct
        : DirectDirectoryState::rejected;
#else
    struct stat status{};
    const int result = is_fd_backed_path(path)
        ? ::stat(path.c_str(), &status)
        : ::lstat(path.c_str(), &status);
    if (result != 0) {
        return DirectDirectoryState::unavailable;
    }
    return S_ISDIR(status.st_mode)
        ? DirectDirectoryState::direct
        : DirectDirectoryState::rejected;
#endif
}

bool paths_equal_for_platform(
    const std::filesystem::path& left,
    const std::filesystem::path& right) {
#if defined(_WIN32)
    const std::wstring left_value = left.lexically_normal().native();
    const std::wstring right_value = right.lexically_normal().native();
    return ::CompareStringOrdinal(
               left_value.data(),
               static_cast<int>(left_value.size()),
               right_value.data(),
               static_cast<int>(right_value.size()),
               TRUE) == CSTR_EQUAL;
#else
    return left.lexically_normal() == right.lexically_normal();
#endif
}

bool is_containment_policy_rejection(
    const security::PhysicalPathContainmentFailure failure) {
    return failure == security::PhysicalPathContainmentFailure::outside_root ||
        failure == security::PhysicalPathContainmentFailure::indirect_component ||
        failure == security::PhysicalPathContainmentFailure::cross_device_component ||
        failure == security::PhysicalPathContainmentFailure::not_regular_file;
}

std::string rejected_destination(const std::filesystem::path& path) {
    return runtime_text(
        "Runtime.Package.Error.ContentDestinationRejected",
        {{"path", copperfin::platform::path_to_utf8_string(path)}});
}

std::string rejected_content_root(const std::filesystem::path& path) {
    return runtime_text(
        "Runtime.Package.Error.ContentRootRejected",
        {{"path", copperfin::platform::path_to_utf8_string(path)}});
}

std::string content_root_creation_failed() {
    return runtime_text("Runtime.Package.Error.CreateContentRootFailed");
}

std::string directory_creation_failed(const std::filesystem::path& path) {
    return runtime_text(
        "Runtime.Package.Error.CreateDirectoryFailed",
        {{"path", copperfin::platform::path_to_utf8_string(path)}});
}

std::string copy_file_failed(const std::filesystem::path& path) {
    return runtime_text(
        "Runtime.Package.Error.CopyFileFailed",
        {{"path", copperfin::platform::path_to_utf8_string(path)}});
}

std::string unique_temporary_name() {
    static std::atomic<unsigned long long> sequence{0U};
    const auto timestamp = std::chrono::steady_clock::now().time_since_epoch().count();
    return ".copperfin-asset-" + std::to_string(timestamp) + "-" +
        std::to_string(sequence.fetch_add(1U, std::memory_order_relaxed));
}

#if !defined(_WIN32)
bool write_source_to_descriptor(
    const std::filesystem::path& source,
    const int destination_descriptor) {
    std::ifstream input(source, std::ios::binary);
    if (!input) {
        return false;
    }
    std::array<char, 64U * 1024U> buffer{};
    while (input) {
        input.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
        const std::streamsize count = input.gcount();
        std::streamsize offset = 0;
        while (offset < count) {
            const ssize_t written = ::write(
                destination_descriptor,
                buffer.data() + offset,
                static_cast<std::size_t>(count - offset));
            if (written < 0 && errno == EINTR) {
                continue;
            }
            if (written <= 0) {
                return false;
            }
            offset += written;
        }
    }
    return input.eof();
}

bool copy_to_pinned_posix_parent(
    const std::filesystem::path& source,
    const std::filesystem::path& content_root,
    const std::filesystem::path& relative_path,
    std::filesystem::path& destination,
    std::string& error) {
    const auto root_descriptor = fd_from_path(content_root);
    if (!root_descriptor.has_value()) {
        return false;
    }
    destination = (content_root / relative_path).lexically_normal();
    int parent_descriptor = ::dup(*root_descriptor);
    if (parent_descriptor < 0) {
        error = copy_file_failed(destination);
        return false;
    }
    for (const auto& component : relative_path.parent_path()) {
        if (component == ".") {
            continue;
        }
        const std::string name = copperfin::platform::path_to_utf8_string(component);
        int child_descriptor = ::openat(
            parent_descriptor,
            name.c_str(),
            O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
        if (child_descriptor < 0 && errno == ENOENT &&
            ::mkdirat(parent_descriptor, name.c_str(), 0777) == 0) {
            child_descriptor = ::openat(
                parent_descriptor,
                name.c_str(),
                O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
        }
        if (child_descriptor < 0) {
            (void)::close(parent_descriptor);
            error = copy_file_failed(destination);
            return false;
        }
        (void)::close(parent_descriptor);
        parent_descriptor = child_descriptor;
    }

    struct stat existing_information{};
    if (::fstatat(
            parent_descriptor,
            copperfin::platform::path_to_utf8_string(relative_path.filename()).c_str(),
            &existing_information,
            AT_SYMLINK_NOFOLLOW) == 0) {
        if (!S_ISREG(existing_information.st_mode) ||
            existing_information.st_nlink != 1) {
            (void)::close(parent_descriptor);
            error = rejected_destination(destination);
            return false;
        }
    } else if (errno != ENOENT) {
        (void)::close(parent_descriptor);
        error = copy_file_failed(destination);
        return false;
    }

    const std::string temporary_name = unique_temporary_name();
    const int temporary_descriptor = ::openat(
        parent_descriptor,
        temporary_name.c_str(),
        O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC,
        0600);
    if (temporary_descriptor < 0 ||
        !write_source_to_descriptor(source, temporary_descriptor) ||
        ::fsync(temporary_descriptor) != 0) {
        if (temporary_descriptor >= 0) {
            (void)::close(temporary_descriptor);
        }
        (void)::unlinkat(parent_descriptor, temporary_name.c_str(), 0);
        (void)::close(parent_descriptor);
        error = copy_file_failed(destination);
        return false;
    }
    if (::close(temporary_descriptor) != 0 ||
        ::renameat(
            parent_descriptor,
            temporary_name.c_str(),
            parent_descriptor,
            copperfin::platform::path_to_utf8_string(relative_path.filename()).c_str()) != 0) {
        (void)::unlinkat(parent_descriptor, temporary_name.c_str(), 0);
        (void)::close(parent_descriptor);
        error = copy_file_failed(destination);
        return false;
    }
    struct stat copied_information{};
    const std::string destination_name =
        copperfin::platform::path_to_utf8_string(relative_path.filename());
    if (::fstatat(
            parent_descriptor,
            destination_name.c_str(),
            &copied_information,
            AT_SYMLINK_NOFOLLOW) != 0 ||
        !S_ISREG(copied_information.st_mode) ||
        copied_information.st_nlink != 1) {
        (void)::close(parent_descriptor);
        error = rejected_destination(destination);
        return false;
    }
    (void)::fsync(parent_descriptor);
    (void)::close(parent_descriptor);
    return true;
}
#else
bool write_source_to_handle(
    const std::filesystem::path& source,
    const HANDLE destination_handle) {
    std::ifstream input(source, std::ios::binary);
    if (!input) {
        return false;
    }
    std::array<char, 64U * 1024U> buffer{};
    while (input) {
        input.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
        const std::streamsize count = input.gcount();
        DWORD offset = 0U;
        while (offset < static_cast<DWORD>(count)) {
            DWORD written = 0U;
            if (::WriteFile(
                    destination_handle,
                    buffer.data() + offset,
                    static_cast<DWORD>(count) - offset,
                    &written,
                    nullptr) == 0 ||
                written == 0U) {
                return false;
            }
            offset += written;
        }
    }
    return input.eof();
}

class WindowsPinnedDirectoryChain {
public:
    WindowsPinnedDirectoryChain() = default;
    WindowsPinnedDirectoryChain(const WindowsPinnedDirectoryChain&) = delete;
    WindowsPinnedDirectoryChain& operator=(const WindowsPinnedDirectoryChain&) = delete;

    ~WindowsPinnedDirectoryChain() {
        for (const HANDLE handle : handles_) {
            (void)::CloseHandle(handle);
        }
    }

    bool open(
        const std::filesystem::path& content_root,
        const std::filesystem::path& relative_parent) {
        if (!open_direct_directory(content_root)) {
            return false;
        }
        std::filesystem::path current = content_root;
        for (const auto& component : relative_parent) {
            if (component == ".") {
                continue;
            }
            current /= component;
            if (!open_direct_directory(current)) {
                return false;
            }
        }
        return true;
    }

private:
    bool open_direct_directory(const std::filesystem::path& path) {
        const HANDLE handle = ::CreateFileW(
            path.c_str(),
            FILE_LIST_DIRECTORY | FILE_READ_ATTRIBUTES,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            nullptr,
            OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT,
            nullptr);
        if (handle == INVALID_HANDLE_VALUE) {
            return false;
        }
        BY_HANDLE_FILE_INFORMATION information{};
        const bool direct_directory =
            ::GetFileInformationByHandle(handle, &information) != 0 &&
            (information.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0U &&
            (information.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) == 0U;
        if (!direct_directory) {
            (void)::CloseHandle(handle);
            return false;
        }
        handles_.push_back(handle);
        return true;
    }

    std::vector<HANDLE> handles_;
};

bool copy_to_pinned_windows_parent(
    const std::filesystem::path& source,
    const std::filesystem::path& content_root,
    const std::filesystem::path& relative_path,
    std::filesystem::path& destination,
    std::string& error) {
    destination = (content_root / relative_path).lexically_normal();
    const std::filesystem::path parent =
        (content_root / relative_path.parent_path()).lexically_normal();
#if defined(COPPERFIN_ENABLE_RUNTIME_PIPELINE_TEST_HOOKS)
    test_hooks::pause_before_package_content_parent_open();
#endif
    WindowsPinnedDirectoryChain parent_chain;
    if (!parent_chain.open(content_root, relative_path.parent_path())) {
        error = copy_file_failed(destination);
        return false;
    }
    const HANDLE existing_handle = ::CreateFileW(
        destination.c_str(),
        FILE_READ_ATTRIBUTES,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OPEN_REPARSE_POINT,
        nullptr);
    if (existing_handle != INVALID_HANDLE_VALUE) {
        BY_HANDLE_FILE_INFORMATION existing_information{};
        const bool read_existing =
            ::GetFileInformationByHandle(existing_handle, &existing_information) != 0;
        (void)::CloseHandle(existing_handle);
        if (!read_existing) {
            error = copy_file_failed(destination);
            return false;
        }
        if ((existing_information.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0U ||
            (existing_information.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0U ||
            existing_information.nNumberOfLinks != 1U) {
            error = rejected_destination(destination);
            return false;
        }
    } else {
        const DWORD existing_error = ::GetLastError();
        if (existing_error != ERROR_FILE_NOT_FOUND &&
            existing_error != ERROR_PATH_NOT_FOUND) {
            error = copy_file_failed(destination);
            return false;
        }
    }

    const std::filesystem::path temporary = parent / unique_temporary_name();
    const HANDLE temporary_handle = ::CreateFileW(
        temporary.c_str(),
        GENERIC_WRITE,
        0U,
        nullptr,
        CREATE_NEW,
        FILE_ATTRIBUTE_TEMPORARY,
        nullptr);
    if (temporary_handle == INVALID_HANDLE_VALUE ||
        !write_source_to_handle(source, temporary_handle) ||
        ::FlushFileBuffers(temporary_handle) == 0) {
        if (temporary_handle != INVALID_HANDLE_VALUE) {
            (void)::CloseHandle(temporary_handle);
        }
        (void)::DeleteFileW(temporary.c_str());
        error = copy_file_failed(destination);
        return false;
    }
    const bool closed = ::CloseHandle(temporary_handle) != 0;
    const bool moved = closed &&
        ::MoveFileExW(
            temporary.c_str(),
            destination.c_str(),
            MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
    if (!moved) {
        (void)::DeleteFileW(temporary.c_str());
        error = copy_file_failed(destination);
        return false;
    }
    return true;
}
#endif

bool prepare_direct_parent(
    const std::filesystem::path& content_root,
    const std::filesystem::path& relative_parent,
    const std::filesystem::path& reported_destination,
    std::string& error) {
    const DirectDirectoryState root_state = inspect_direct_directory(content_root);
    if (root_state != DirectDirectoryState::direct) {
        error = root_state == DirectDirectoryState::rejected
            ? rejected_destination(reported_destination)
            : directory_creation_failed(content_root);
        return false;
    }
    const auto root_containment =
        security::inspect_physical_path_containment(content_root, content_root);
    if (!root_containment.allowed) {
        error = is_containment_policy_rejection(root_containment.failure)
            ? rejected_destination(reported_destination)
            : directory_creation_failed(content_root);
        return false;
    }

    std::filesystem::path current = content_root;
    for (const auto& component : relative_parent) {
        current /= component;
        std::error_code status_error;
        const auto status = std::filesystem::symlink_status(current, status_error);
        if (status_error && status_error != std::errc::no_such_file_or_directory) {
            error = directory_creation_failed(current);
            return false;
        }
        if (!std::filesystem::exists(status)) {
            std::error_code create_error;
            if (!std::filesystem::create_directory(current, create_error) || create_error) {
                error = directory_creation_failed(current);
                return false;
            }
        }
        const DirectDirectoryState current_state = inspect_direct_directory(current);
        if (current_state != DirectDirectoryState::direct) {
            error = current_state == DirectDirectoryState::rejected
                ? rejected_destination(reported_destination)
                : directory_creation_failed(current);
            return false;
        }
        const auto containment =
            security::inspect_physical_path_containment(current, content_root);
        if (!containment.allowed) {
            error = is_containment_policy_rejection(containment.failure)
                ? rejected_destination(reported_destination)
                : directory_creation_failed(current);
            return false;
        }
    }
    return true;
}

}  // namespace

bool prepare_package_content_root(
    const std::filesystem::path& package_root,
    const std::filesystem::path& content_root,
    std::string& error,
    int* content_descriptor_out) {
#if !defined(_WIN32)
    if (content_descriptor_out != nullptr) {
        *content_descriptor_out = -1;
    }
#else
    (void)content_descriptor_out;
#endif
    std::error_code filesystem_error;
    const std::filesystem::path absolute_package_root =
        std::filesystem::absolute(package_root, filesystem_error).lexically_normal();
    if (filesystem_error) {
        error = content_root_creation_failed();
        return false;
    }
    const std::filesystem::path absolute_content_root =
        std::filesystem::absolute(content_root, filesystem_error).lexically_normal();
    if (filesystem_error) {
        error = content_root_creation_failed();
        return false;
    }
#if !defined(_WIN32)
    if (const auto parent_descriptor = fd_from_path(absolute_package_root.parent_path());
        parent_descriptor.has_value()) {
        if (!paths_equal_for_platform(
                absolute_content_root,
                absolute_package_root / "content")) {
            error = rejected_content_root(content_root);
            return false;
        }
        const std::string package_leaf =
            copperfin::platform::path_to_utf8_string(absolute_package_root.filename());
        const int package_descriptor = ::openat(
            *parent_descriptor,
            package_leaf.c_str(),
            O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
        if (package_descriptor < 0) {
            trace_content_root_failure("open-package", absolute_package_root);
            error = content_root_creation_failed();
            return false;
        }
        const std::string content_leaf =
            copperfin::platform::path_to_utf8_string(absolute_content_root.filename());
        const bool created = ::mkdirat(package_descriptor, content_leaf.c_str(), 0700) == 0;
        if (!created && errno != EEXIST) {
            trace_content_root_failure("mkdir-content", absolute_content_root);
            (void)::close(package_descriptor);
            error = content_root_creation_failed();
            return false;
        }
        struct stat content_information{};
        const bool is_directory = ::fstatat(
            package_descriptor,
            content_leaf.c_str(),
            &content_information,
            AT_SYMLINK_NOFOLLOW) == 0 &&
            S_ISDIR(content_information.st_mode);
        if (!is_directory) {
            const int saved_errno = errno;
            (void)::close(package_descriptor);
            errno = saved_errno;
            trace_content_root_failure("stat-content", absolute_content_root);
            error = content_root_creation_failed();
            return false;
        }
        if (content_descriptor_out != nullptr) {
            *content_descriptor_out = ::openat(
                package_descriptor,
                content_leaf.c_str(),
                O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
            if (*content_descriptor_out < 0) {
                trace_content_root_failure("open-content", absolute_content_root);
                (void)::close(package_descriptor);
                error = content_root_creation_failed();
                return false;
            }
        }
        (void)::close(package_descriptor);
        return true;
    }
#endif
    const DirectDirectoryState package_root_state =
        inspect_direct_directory(absolute_package_root);
    if (!paths_equal_for_platform(
            absolute_content_root,
            absolute_package_root / "content")) {
        error = rejected_content_root(content_root);
        return false;
    }
    if (package_root_state != DirectDirectoryState::direct) {
        error = package_root_state == DirectDirectoryState::rejected
            ? rejected_content_root(content_root)
            : content_root_creation_failed();
        return false;
    }

    const auto package_containment = security::inspect_physical_path_containment(
        absolute_package_root,
        absolute_package_root);
    if (!package_containment.allowed) {
        error = is_containment_policy_rejection(package_containment.failure)
            ? rejected_content_root(content_root)
            : content_root_creation_failed();
        return false;
    }

    const auto status =
        std::filesystem::symlink_status(absolute_content_root, filesystem_error);
    if (filesystem_error == std::errc::no_such_file_or_directory) {
        filesystem_error.clear();
    } else if (filesystem_error) {
        error = content_root_creation_failed();
        return false;
    }
    if (!std::filesystem::exists(status) &&
        (!std::filesystem::create_directory(
             absolute_content_root,
             filesystem_error) ||
         filesystem_error)) {
        error = content_root_creation_failed();
        return false;
    }
    const DirectDirectoryState content_root_state =
        inspect_direct_directory(absolute_content_root);
    if (content_root_state != DirectDirectoryState::direct) {
        error = content_root_state == DirectDirectoryState::rejected
            ? rejected_content_root(content_root)
            : content_root_creation_failed();
        return false;
    }
    const auto content_containment = security::inspect_physical_path_containment(
        absolute_content_root,
        absolute_package_root);
    if (!content_containment.allowed) {
        error = is_containment_policy_rejection(content_containment.failure)
            ? rejected_content_root(content_root)
            : content_root_creation_failed();
        return false;
    }
    return true;
}

bool copy_file_to_package_content(
    const std::filesystem::path& source,
    const std::filesystem::path& package_root,
    const std::filesystem::path& content_root,
    const std::filesystem::path& relative_path,
    std::filesystem::path& destination,
    std::string& error) {
    destination.clear();
    const auto admitted = admitted_relative_path(relative_path);
    if (!admitted.has_value()) {
        error = rejected_destination(relative_path);
        return false;
    }
#if defined(_WIN32)
    constexpr bool use_pinned_write = true;
    if (!prepare_package_content_root(package_root, content_root, error)) {
        return false;
    }
#else
    const bool use_pinned_write = is_fd_backed_path(content_root);
    if (!use_pinned_write &&
        !prepare_package_content_root(package_root, content_root, error)) {
        return false;
    }
#endif

    std::error_code filesystem_error;
    const std::filesystem::path absolute_root =
        std::filesystem::absolute(content_root, filesystem_error).lexically_normal();
    if (filesystem_error) {
        error = directory_creation_failed(content_root);
        return false;
    }
#if defined(_WIN32)
    if (!prepare_direct_parent(
            absolute_root,
            admitted->parent_path(),
            content_root / *admitted,
            error)) {
        return false;
    }
#else
    if (!use_pinned_write &&
        !prepare_direct_parent(
            absolute_root,
            admitted->parent_path(),
            content_root / *admitted,
            error)) {
        return false;
    }
#endif
    destination = (content_root / *admitted).lexically_normal();
    const std::filesystem::path write_destination =
        (absolute_root / *admitted).lexically_normal();

    if (!use_pinned_write) {
        const auto status =
            std::filesystem::symlink_status(write_destination, filesystem_error);
        if (filesystem_error && filesystem_error != std::errc::no_such_file_or_directory) {
            error = copy_file_failed(destination);
            return false;
        }
        if (std::filesystem::exists(status)) {
            const auto containment =
                security::inspect_physical_path_containment(write_destination, absolute_root);
            if (!containment.allowed) {
                error = is_containment_policy_rejection(containment.failure)
                    ? rejected_destination(destination)
                    : copy_file_failed(destination);
                return false;
            }
            if (containment.identity.link_count != 1U ||
                !std::filesystem::is_regular_file(status)) {
                error = rejected_destination(destination);
                return false;
            }
        }
    }

#if defined(_WIN32)
    const bool write_with_pinned_parent = true;
#else
    const bool write_with_pinned_parent = use_pinned_write;
#endif
    if (write_with_pinned_parent) {
#if defined(_WIN32)
        if (!copy_to_pinned_windows_parent(
                source,
                content_root,
                *admitted,
                destination,
                error)) {
            return false;
        }
#else
        if (!copy_to_pinned_posix_parent(
                source,
                content_root,
                *admitted,
                destination,
                error)) {
            return false;
        }
#endif
#if defined(_WIN32)
        const auto copied =
            security::inspect_physical_path_containment(write_destination, absolute_root);
        if (!copied.allowed) {
            error = is_containment_policy_rejection(copied.failure)
                ? rejected_destination(destination)
                : copy_file_failed(destination);
            return false;
        }
        if (copied.identity.link_count != 1U) {
            error = rejected_destination(destination);
            return false;
        }
#endif
        return true;
    }

    if (!copy_file_if_exists(source, write_destination, error)) {
        return false;
    }
    const auto copied =
        security::inspect_physical_path_containment(write_destination, absolute_root);
    if (!copied.allowed) {
        error = is_containment_policy_rejection(copied.failure)
            ? rejected_destination(destination)
            : copy_file_failed(destination);
        return false;
    }
    if (copied.identity.link_count != 1U) {
        error = rejected_destination(destination);
        return false;
    }
    return true;
}

}  // namespace copperfin::runtime::runtime_pipeline_detail
