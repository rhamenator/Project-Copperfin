// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/security/workspace_agent_process_containment.h"

#include "copperfin/platform/windows_pe_image.h"

#include "sha256_native.h"

#include <algorithm>
#include <atomic>
#include <memory>
#include <string_view>
#include <system_error>
#include <utility>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace copperfin::security {

struct WorkspaceAgentProcessTargetBoundaryAuthority final {};

struct WorkspaceAgentProcessTargetPinAuthority final {
    WorkspaceAgentProcessTargetPinAuthority(
        std::shared_ptr<const WorkspaceAgentProcessTargetBoundaryAuthority>
            boundary_authority_value,
        std::filesystem::path executable_path_value,
        PhysicalPathIdentity executable_identity_value,
        std::string executable_sha256_value,
        std::filesystem::path working_directory_value,
        PhysicalPathIdentity working_directory_identity_value)
        : boundary_authority(std::move(boundary_authority_value)),
          executable_path(std::move(executable_path_value)),
          executable_identity(executable_identity_value),
          executable_sha256(std::move(executable_sha256_value)),
          working_directory(std::move(working_directory_value)),
          working_directory_identity(working_directory_identity_value) {}

    std::shared_ptr<const WorkspaceAgentProcessTargetBoundaryAuthority>
        boundary_authority;
    std::filesystem::path executable_path;
    PhysicalPathIdentity executable_identity{};
    std::string executable_sha256;
    std::filesystem::path working_directory;
    PhysicalPathIdentity working_directory_identity{};
    std::atomic<bool> consumed = false;
};

class WorkspaceAgentProcessTargetPins::Impl {
public:
    [[nodiscard]] bool matches_target_identities(
        const PhysicalPathIdentity& expected_executable_identity,
        const PhysicalPathIdentity& expected_working_directory_identity) const
        noexcept;
#if defined(_WIN32)
    Impl(HANDLE workspace_root_value,
         HANDLE executable_value,
         std::vector<HANDLE> working_directory_chain_value,
         std::filesystem::path execution_working_directory_value,
         PhysicalPathIdentity executable_identity_value,
         std::string executable_sha256_value,
         std::vector<std::uint8_t> executable_snapshot_value) noexcept
        : workspace_root(workspace_root_value),
          executable(executable_value),
          working_directory_chain(std::move(working_directory_chain_value)),
          execution_working_directory_value(
              std::move(execution_working_directory_value)),
          executable_identity(executable_identity_value),
          executable_sha256(std::move(executable_sha256_value)),
          executable_snapshot(std::move(executable_snapshot_value)) {}

    ~Impl() {
        close(workspace_root);
        close(executable);
        for (auto iterator = working_directory_chain.rbegin();
             iterator != working_directory_chain.rend(); ++iterator) {
            close(*iterator);
        }
    }

    [[nodiscard]] bool valid() const noexcept {
        return is_valid(workspace_root) && is_valid(executable) &&
            !working_directory_chain.empty() &&
            is_valid(working_directory_chain.back()) &&
            execution_working_directory_value.is_absolute() &&
            executable_sha256.size() == 64U &&
            static_cast<std::uint64_t>(executable_snapshot.size()) ==
                executable_identity.file_size;
    }

    [[nodiscard]] WorkspaceAgentProcessTargetAuthenticationResult
    verify_executable_bytes();

    [[nodiscard]] const std::vector<std::uint8_t>& snapshot() const noexcept {
        return executable_snapshot;
    }

    [[nodiscard]] const std::filesystem::path& execution_working_directory()
        const noexcept {
        return execution_working_directory_value;
    }

private:
    static bool is_valid(HANDLE value) noexcept {
        return value != nullptr && value != INVALID_HANDLE_VALUE;
    }

    static void close(HANDLE value) noexcept {
        if (is_valid(value)) {
            ::CloseHandle(value);
        }
    }

    HANDLE workspace_root = INVALID_HANDLE_VALUE;
    HANDLE executable = INVALID_HANDLE_VALUE;
    std::vector<HANDLE> working_directory_chain;
    std::filesystem::path execution_working_directory_value;
    PhysicalPathIdentity executable_identity{};
    std::string executable_sha256;
    std::vector<std::uint8_t> executable_snapshot;
#else
    Impl(int workspace_root_value,
         int executable_value,
         int working_directory_value,
         PhysicalPathIdentity executable_identity_value,
         std::string executable_sha256_value,
         std::vector<std::uint8_t> executable_snapshot_value) noexcept
        : workspace_root(workspace_root_value),
          executable(executable_value),
          working_directory(working_directory_value),
          executable_identity(executable_identity_value),
          executable_sha256(std::move(executable_sha256_value)),
          executable_snapshot(std::move(executable_snapshot_value)) {}

    ~Impl() {
        close(workspace_root);
        close(executable);
        close(working_directory);
    }

    [[nodiscard]] bool valid() const noexcept {
        return workspace_root >= 0 && executable >= 0 && working_directory >= 0 &&
            executable_sha256.size() == 64U &&
            static_cast<std::uint64_t>(executable_snapshot.size()) ==
                executable_identity.file_size;
    }

    [[nodiscard]] WorkspaceAgentProcessTargetAuthenticationResult
    verify_executable_bytes();

    [[nodiscard]] const std::vector<std::uint8_t>& snapshot() const noexcept {
        return executable_snapshot;
    }

private:
    static void close(int value) noexcept {
        if (value >= 0) {
            ::close(value);
        }
    }

    int workspace_root = -1;
    int executable = -1;
    int working_directory = -1;
    PhysicalPathIdentity executable_identity{};
    std::string executable_sha256;
    std::vector<std::uint8_t> executable_snapshot;
#endif
};

WorkspaceAgentProcessTargetPins::WorkspaceAgentProcessTargetPins(
    std::unique_ptr<Impl> impl) noexcept
    : impl_(std::move(impl)) {}

WorkspaceAgentProcessTargetPins::WorkspaceAgentProcessTargetPins() = default;
WorkspaceAgentProcessTargetPins::~WorkspaceAgentProcessTargetPins() = default;
WorkspaceAgentProcessTargetPins::WorkspaceAgentProcessTargetPins(
    WorkspaceAgentProcessTargetPins&&) noexcept = default;
WorkspaceAgentProcessTargetPins& WorkspaceAgentProcessTargetPins::operator=(
    WorkspaceAgentProcessTargetPins&&) noexcept = default;

bool WorkspaceAgentProcessTargetPins::valid() const noexcept {
    return impl_ != nullptr && impl_->valid();
}

WorkspaceAgentProcessTargetAuthenticationResult
WorkspaceAgentProcessTargetPins::verify_executable_bytes() {
    if (impl_ == nullptr) {
        return {
            .authenticated = false,
            .diagnostic_code =
                "workspace_agent.process_executable_authentication_unavailable"};
    }
    return impl_->verify_executable_bytes();
}

bool WorkspaceAgentProcessTargetPins::matches_target_identities(
    const PhysicalPathIdentity& executable_identity,
    const PhysicalPathIdentity& working_directory_identity) const noexcept {
    return impl_ != nullptr && impl_->matches_target_identities(
        executable_identity, working_directory_identity);
}

const std::vector<std::uint8_t>*
WorkspaceAgentProcessTargetPins::executable_snapshot_for_materialization()
    const noexcept {
    return impl_ != nullptr && impl_->valid()
        ? &impl_->snapshot()
        : nullptr;
}

namespace {

WorkspaceAgentProcessTargetInspection denied(std::string diagnostic_code) {
    WorkspaceAgentProcessTargetInspection result;
    result.diagnostic_code = std::move(diagnostic_code);
    return result;
}

bool path_has_embedded_nul(const std::filesystem::path& path) {
    const auto& native = path.native();
    return native.find(typename std::filesystem::path::value_type{}) !=
        std::filesystem::path::string_type::npos;
}

bool path_has_dot_component(const std::filesystem::path& path) {
    for (const auto& component : path) {
        if (component == "." || component == "..") {
            return true;
        }
    }
    return false;
}

#if defined(_WIN32)
bool path_has_windows_device_or_stream_syntax(const std::filesystem::path& path) {
    const auto& native = path.native();
    if (native.rfind(L"\\\\?\\", 0U) == 0U ||
        native.rfind(L"\\\\.\\", 0U) == 0U ||
        native.rfind(L"\\??\\", 0U) == 0U) {
        return true;
    }
    const auto root_name = path.root_name();
    const auto& root_native = root_name.native();
    const auto is_separator = [](wchar_t value) {
        return value == L'\\' || value == L'/';
    };
    if (root_native.size() >= 2U && is_separator(root_native[0]) &&
        is_separator(root_native[1])) {
        return true;
    }
    for (const auto& component : path.relative_path()) {
        if (component.native().find(L':') != std::wstring::npos) {
            return true;
        }
    }
    return root_name.empty() && native.find(L':') != std::wstring::npos;
}
#else
bool path_has_windows_device_or_stream_syntax(const std::filesystem::path&) {
    return false;
}
#endif

bool strict_relative_executable_path(const std::filesystem::path& path) {
    return !path.empty() && !path_has_embedded_nul(path) &&
        !path.is_absolute() && !path.has_root_name() &&
        !path.has_root_directory() && !path_has_dot_component(path) &&
        !path.filename().empty() &&
        !path_has_windows_device_or_stream_syntax(path);
}

bool strict_relative_working_directory_path(const std::filesystem::path& path) {
    if (path == ".") {
        return true;
    }
    return !path.empty() && !path_has_embedded_nul(path) &&
        !path.is_absolute() && !path.has_root_name() &&
        !path.has_root_directory() && !path_has_dot_component(path) &&
        !path.filename().empty() &&
        !path_has_windows_device_or_stream_syntax(path);
}

bool strict_absolute_executable_path(const std::filesystem::path& path) {
    return !path.empty() && !path_has_embedded_nul(path) && path.is_absolute() &&
        !path_has_dot_component(path) && !path.filename().empty() &&
        !path_has_windows_device_or_stream_syntax(path);
}

bool strict_absolute_working_directory_path(const std::filesystem::path& path) {
    return !path.empty() && !path_has_embedded_nul(path) && path.is_absolute() &&
        !path_has_dot_component(path) &&
        !path_has_windows_device_or_stream_syntax(path);
}

bool path_is_direct_directory(const std::filesystem::path& path) {
#if defined(_WIN32)
    const DWORD attributes = ::GetFileAttributesW(path.c_str());
    return attributes != INVALID_FILE_ATTRIBUTES &&
        (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0U &&
        (attributes & FILE_ATTRIBUTE_REPARSE_POINT) == 0U;
#else
    struct stat status{};
    return ::lstat(path.c_str(), &status) == 0 && S_ISDIR(status.st_mode) &&
        !S_ISLNK(status.st_mode);
#endif
}

bool executable_is_eligible(const std::filesystem::path& path) {
    std::error_code filesystem_error;
    if (!std::filesystem::is_regular_file(path, filesystem_error) ||
        filesystem_error) {
        return false;
    }
#if defined(_WIN32)
    return true;
#else
    return ::access(path.c_str(), X_OK) == 0;
#endif
}

std::string executable_diagnostic_for_failure(
    PhysicalPathContainmentFailure failure) {
    switch (failure) {
        case PhysicalPathContainmentFailure::outside_root:
            return "workspace_agent.process_executable_outside_workspace";
        case PhysicalPathContainmentFailure::indirect_component:
            return "workspace_agent.process_executable_indirect_component";
        case PhysicalPathContainmentFailure::cross_device_component:
            return "workspace_agent.process_executable_cross_device_component";
        case PhysicalPathContainmentFailure::root_unavailable:
            return "workspace_agent.process_workspace_root_unavailable";
        case PhysicalPathContainmentFailure::none:
        case PhysicalPathContainmentFailure::path_unavailable:
        case PhysicalPathContainmentFailure::identity_changed:
        case PhysicalPathContainmentFailure::not_regular_file:
        case PhysicalPathContainmentFailure::size_limit_exceeded:
        case PhysicalPathContainmentFailure::read_failed:
            return "workspace_agent.process_executable_unavailable";
    }
    return "workspace_agent.process_executable_unavailable";
}

std::string working_directory_diagnostic_for_failure(
    PhysicalPathContainmentFailure failure) {
    switch (failure) {
        case PhysicalPathContainmentFailure::outside_root:
            return "workspace_agent.process_working_directory_outside_workspace";
        case PhysicalPathContainmentFailure::indirect_component:
            return "workspace_agent.process_working_directory_indirect_component";
        case PhysicalPathContainmentFailure::cross_device_component:
            return "workspace_agent.process_working_directory_cross_device_component";
        case PhysicalPathContainmentFailure::root_unavailable:
            return "workspace_agent.process_workspace_root_unavailable";
        case PhysicalPathContainmentFailure::none:
        case PhysicalPathContainmentFailure::path_unavailable:
        case PhysicalPathContainmentFailure::identity_changed:
        case PhysicalPathContainmentFailure::not_regular_file:
        case PhysicalPathContainmentFailure::size_limit_exceeded:
        case PhysicalPathContainmentFailure::read_failed:
            return "workspace_agent.process_working_directory_unavailable";
    }
    return "workspace_agent.process_working_directory_unavailable";
}

std::optional<PhysicalPathContainmentResult> inspect_executable(
    const std::filesystem::path& path,
    const std::filesystem::path& containment_root,
    WorkspaceAgentProcessTargetInspection& failure_result) {
    const auto containment = inspect_physical_path_containment(path, containment_root);
    if (!containment.allowed) {
        failure_result = denied(executable_diagnostic_for_failure(containment.failure));
        return std::nullopt;
    }
    if (!executable_is_eligible(containment.canonical_path)) {
        failure_result = denied("workspace_agent.process_executable_not_eligible");
        return std::nullopt;
    }
    if (containment.identity.link_count != 1U) {
        failure_result = denied("workspace_agent.process_executable_multiply_linked");
        return std::nullopt;
    }
#if defined(_WIN32)
    const auto image = platform::inspect_windows_pe_image(containment.canonical_path);
    if (image.status == platform::WindowsPeImageStatus::unreadable) {
        failure_result = denied("workspace_agent.process_executable_image_unreadable");
        return std::nullopt;
    }
    if (image.status == platform::WindowsPeImageStatus::invalid) {
        failure_result = denied("workspace_agent.process_executable_image_invalid");
        return std::nullopt;
    }
    if (image.status != platform::WindowsPeImageStatus::executable) {
        failure_result = denied("workspace_agent.process_executable_image_not_launchable");
        return std::nullopt;
    }
    if (!platform::windows_pe_image_is_launch_compatible(
            image, platform::native_windows_pe_host_machine())) {
        failure_result = denied("workspace_agent.process_executable_machine_incompatible");
        return std::nullopt;
    }
    const auto after_image = inspect_physical_path_containment(
        containment.canonical_path, containment_root);
    if (!after_image.allowed || after_image.identity != containment.identity) {
        failure_result = denied("workspace_agent.process_executable_changed_during_image_inspection");
        return std::nullopt;
    }
#endif
    return containment;
}

std::optional<PhysicalPathContainmentResult> inspect_working_directory(
    const std::filesystem::path& path,
    const std::filesystem::path& containment_root,
    WorkspaceAgentProcessTargetInspection& failure_result) {
    const auto containment = inspect_physical_path_containment(path, containment_root);
    if (!containment.allowed) {
        failure_result = denied(
            working_directory_diagnostic_for_failure(containment.failure));
        return std::nullopt;
    }
    std::error_code filesystem_error;
    if (!std::filesystem::is_directory(
            containment.canonical_path, filesystem_error) ||
        filesystem_error) {
        failure_result = denied(
            "workspace_agent.process_working_directory_not_directory");
        return std::nullopt;
    }
    return containment;
}

WorkspaceAgentProcessTargetInspection allowed(
    const PhysicalPathContainmentResult& executable,
    const PhysicalPathContainmentResult& working_directory) {
    WorkspaceAgentProcessTargetInspection result;
    result.allowed = true;
    result.canonical_executable_path = executable.canonical_path;
    result.executable_identity = executable.identity;
    result.canonical_working_directory = working_directory.canonical_path;
    result.working_directory_identity = working_directory.identity;
    result.diagnostic_code = "workspace_agent.process_target_allowed";
    return result;
}

#if defined(_WIN32)

bool read_handle_identity(
    const HANDLE handle,
    const bool expect_directory,
    PhysicalPathIdentity& identity) noexcept {
    BY_HANDLE_FILE_INFORMATION information{};
    if (::GetFileInformationByHandle(handle, &information) == 0 ||
        (information.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0U ||
        (((information.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0U) !=
         expect_directory)) {
        return false;
    }
    identity = {
        .storage_id = information.dwVolumeSerialNumber,
        .file_id =
            (static_cast<std::uint64_t>(information.nFileIndexHigh) << 32U) |
            information.nFileIndexLow,
        .file_size =
            (static_cast<std::uint64_t>(information.nFileSizeHigh) << 32U) |
            information.nFileSizeLow,
        .modified_ticks =
            (static_cast<std::uint64_t>(
                 information.ftLastWriteTime.dwHighDateTime) << 32U) |
            information.ftLastWriteTime.dwLowDateTime,
        .link_count = information.nNumberOfLinks};
    return true;
}

HANDLE open_pin_handle(
    const std::filesystem::path& path,
    const bool directory) noexcept {
    return ::CreateFileW(
        path.c_str(),
        directory ? FILE_READ_ATTRIBUTES : GENERIC_READ,
        FILE_SHARE_READ,
        nullptr,
        OPEN_EXISTING,
        (directory ? FILE_FLAG_BACKUP_SEMANTICS : FILE_ATTRIBUTE_NORMAL) |
            FILE_FLAG_OPEN_REPARSE_POINT,
        nullptr);
}

std::optional<std::size_t> stable_volume_root_length(
    const std::wstring_view path) noexcept {
    constexpr std::wstring_view guid_prefix = L"\\\\?\\Volume{";
    if (path.rfind(guid_prefix, 0U) == 0U) {
        const std::size_t close = path.find(L"}\\", guid_prefix.size());
        return close != std::wstring_view::npos && close > guid_prefix.size()
            ? std::optional<std::size_t>(close + 2U)
            : std::nullopt;
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
    return index != digits_begin && index < path.size() && path[index] == L'\\'
        ? std::optional<std::size_t>(index + 1U)
        : std::nullopt;
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
        return stable_volume_root_length(global).has_value()
            ? std::optional<std::filesystem::path>(
                  std::filesystem::path(std::move(global)))
            : std::nullopt;
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

std::optional<std::wstring> native_hard_disk_volume_root(
    const std::wstring_view path) noexcept {
    constexpr std::wstring_view prefix = L"\\Device\\HarddiskVolume";
    if (path.rfind(prefix, 0U) != 0U) {
        return std::nullopt;
    }
    std::size_t index = prefix.size();
    const std::size_t digits_begin = index;
    while (index < path.size() && path[index] >= L'0' && path[index] <= L'9') {
        ++index;
    }
    return index != digits_begin && index < path.size() && path[index] == L'\\'
        ? std::optional<std::wstring>(std::wstring(path.substr(0U, index)))
        : std::nullopt;
}

std::optional<std::wstring> query_dos_device(
    const std::wstring_view device) {
    constexpr DWORD maximum_path_characters = 32'768U;
    std::vector<wchar_t> buffer(512U);
    for (;;) {
        const DWORD written = ::QueryDosDeviceW(
            std::wstring(device).c_str(), buffer.data(),
            static_cast<DWORD>(buffer.size()));
        if (written != 0U) {
            return buffer.front() == L'\0'
                ? std::nullopt
                : std::optional<std::wstring>(std::wstring(buffer.data()));
        }
        if (::GetLastError() != ERROR_INSUFFICIENT_BUFFER ||
            buffer.size() >= maximum_path_characters) {
            return std::nullopt;
        }
        buffer.resize(std::min<std::size_t>(
            buffer.size() * 2U, maximum_path_characters));
    }
}

bool mount_manager_lists_drive_root(
    const HANDLE handle,
    const std::wstring_view drive_root) noexcept {
    try {
        const auto guid_path = final_path_for_handle(handle, VOLUME_NAME_GUID);
        const auto guid_root_length = guid_path.has_value()
            ? stable_volume_root_length(*guid_path)
            : std::nullopt;
        if (!guid_root_length.has_value()) {
            return false;
        }
        const std::wstring guid_root =
            guid_path->substr(0U, *guid_root_length);
        constexpr DWORD maximum_path_characters = 32'768U;
        std::vector<wchar_t> paths(512U);
        DWORD required = 0U;
        for (;;) {
            if (::GetVolumePathNamesForVolumeNameW(
                    guid_root.c_str(), paths.data(),
                    static_cast<DWORD>(paths.size()), &required) != FALSE) {
                break;
            }
            if (::GetLastError() != ERROR_MORE_DATA ||
                required <= paths.size() ||
                required > maximum_path_characters) {
                return false;
            }
            paths.resize(required);
        }
        const std::size_t used = std::min<std::size_t>(required, paths.size());
        std::size_t begin = 0U;
        while (begin < used && paths[begin] != L'\0') {
            std::size_t end = begin;
            while (end < used && paths[end] != L'\0') {
                ++end;
            }
            if (end == used) {
                return false;
            }
            if (end - begin == drive_root.size() &&
                ::CompareStringOrdinal(
                    paths.data() + begin, static_cast<int>(end - begin),
                    drive_root.data(), static_cast<int>(drive_root.size()),
                    TRUE) == CSTR_EQUAL) {
                return true;
            }
            begin = end + 1U;
        }
        return false;
    } catch (...) {
        return false;
    }
}

bool dos_volume_binding_matches_handle(
    const std::filesystem::path& dos_path,
    const HANDLE handle,
    const std::uint64_t expected_storage_id) noexcept {
    try {
        const std::wstring& path = dos_path.native();
        if (path.size() < 7U || path.rfind(L"\\\\?\\", 0U) != 0U ||
            path[5U] != L':' || path[6U] != L'\\') {
            return false;
        }
        const auto native_path = final_path_for_handle(handle, VOLUME_NAME_NT);
        const auto native_root = native_path.has_value()
            ? native_hard_disk_volume_root(*native_path)
            : std::nullopt;
        const std::wstring device{path[4U], L':'};
        const auto mapping = query_dos_device(device);
        const std::wstring drive_root{path[4U], L':', L'\\'};
        DWORD volume_serial = 0U;
        return native_root.has_value() && mapping.has_value() &&
            ::CompareStringOrdinal(
                mapping->c_str(), static_cast<int>(mapping->size()),
                native_root->c_str(), static_cast<int>(native_root->size()),
                TRUE) == CSTR_EQUAL &&
            ::GetDriveTypeW(drive_root.c_str()) == DRIVE_FIXED &&
            mount_manager_lists_drive_root(handle, drive_root) &&
            ::GetVolumeInformationW(
                drive_root.c_str(), nullptr, 0U, &volume_serial, nullptr,
                nullptr, nullptr, 0U) != FALSE &&
            volume_serial == expected_storage_id;
    } catch (...) {
        return false;
    }
}

void close_pin_handle(HANDLE handle) noexcept {
    if (handle != nullptr && handle != INVALID_HANDLE_VALUE) {
        ::CloseHandle(handle);
    }
}

#else

bool read_handle_identity(
    const int handle,
    const bool expect_directory,
    PhysicalPathIdentity& identity) noexcept {
    struct stat status{};
    if (::fstat(handle, &status) != 0 ||
        (expect_directory ? !S_ISDIR(status.st_mode) : !S_ISREG(status.st_mode))) {
        return false;
    }
#if defined(__APPLE__)
    const std::uint64_t modified_ticks =
        static_cast<std::uint64_t>(status.st_mtimespec.tv_sec) *
            1'000'000'000ULL +
        static_cast<std::uint64_t>(status.st_mtimespec.tv_nsec);
#else
    const std::uint64_t modified_ticks =
        static_cast<std::uint64_t>(status.st_mtim.tv_sec) * 1'000'000'000ULL +
        static_cast<std::uint64_t>(status.st_mtim.tv_nsec);
#endif
    identity = {
        .storage_id = static_cast<std::uint64_t>(status.st_dev),
        .file_id = static_cast<std::uint64_t>(status.st_ino),
        .file_size = static_cast<std::uint64_t>(status.st_size),
        .modified_ticks = modified_ticks,
        .link_count = static_cast<std::uint64_t>(status.st_nlink)};
    return true;
}

int open_pin_handle(
    const std::filesystem::path& path,
    const bool directory) noexcept {
    int flags = O_RDONLY | O_CLOEXEC | O_NOFOLLOW;
    flags |= directory ? O_DIRECTORY : O_NONBLOCK;
    return ::open(path.c_str(), flags);
}

void close_pin_handle(const int handle) noexcept {
    if (handle >= 0) {
        ::close(handle);
    }
}

#endif

class ScopedPinHandle {
public:
#if defined(_WIN32)
    using NativeHandle = HANDLE;
    [[nodiscard]] static NativeHandle invalid() noexcept {
        return INVALID_HANDLE_VALUE;
    }
#else
    using NativeHandle = int;
    [[nodiscard]] static constexpr NativeHandle invalid() noexcept {
        return -1;
    }
#endif

    explicit ScopedPinHandle(NativeHandle handle) noexcept : handle_(handle) {}
    ~ScopedPinHandle() { close_pin_handle(handle_); }
    ScopedPinHandle(const ScopedPinHandle&) = delete;
    ScopedPinHandle& operator=(const ScopedPinHandle&) = delete;

    [[nodiscard]] bool valid() const noexcept {
#if defined(_WIN32)
        return handle_ != nullptr && handle_ != INVALID_HANDLE_VALUE;
#else
        return handle_ >= 0;
#endif
    }

    [[nodiscard]] NativeHandle get() const noexcept { return handle_; }

    [[nodiscard]] NativeHandle release() noexcept {
        const NativeHandle released = handle_;
        handle_ = invalid();
        return released;
    }

private:
    NativeHandle handle_ = invalid();
};

#if defined(_WIN32)
class ScopedPinHandleChain {
public:
    ScopedPinHandleChain() = default;
    ~ScopedPinHandleChain() { reset(); }
    ScopedPinHandleChain(const ScopedPinHandleChain&) = delete;
    ScopedPinHandleChain& operator=(const ScopedPinHandleChain&) = delete;

    [[nodiscard]] bool lock(
        const std::filesystem::path& stable_directory,
        const HANDLE exact_directory_handle) noexcept {
        reset();
        try {
            const auto root_length =
                stable_volume_root_length(stable_directory.native());
            if (!root_length.has_value() ||
                *root_length > stable_directory.native().size()) {
                return false;
            }
            std::filesystem::path current(
                stable_directory.native().substr(0U, *root_length));
            const std::filesystem::path relative(
                stable_directory.native().substr(*root_length));
            if (relative.empty()) {
                HANDLE duplicate = INVALID_HANDLE_VALUE;
                if (exact_directory_handle == nullptr ||
                    exact_directory_handle == INVALID_HANDLE_VALUE ||
                    ::DuplicateHandle(
                        ::GetCurrentProcess(), exact_directory_handle,
                        ::GetCurrentProcess(), &duplicate, 0U, FALSE,
                        DUPLICATE_SAME_ACCESS) == FALSE) {
                    return false;
                }
                try {
                    handles_.push_back(duplicate);
                } catch (...) {
                    close_pin_handle(duplicate);
                    throw;
                }
                return true;
            }
            for (const auto& component : relative) {
                if (component.empty() || component == "." || component == "..") {
                    reset();
                    return false;
                }
                current /= component;
                const HANDLE handle = open_pin_handle(current, true);
                PhysicalPathIdentity component_identity{};
                if (handle == nullptr || handle == INVALID_HANDLE_VALUE ||
                    !read_handle_identity(handle, true, component_identity)) {
                    close_pin_handle(handle);
                    reset();
                    return false;
                }
                try {
                    handles_.push_back(handle);
                } catch (...) {
                    close_pin_handle(handle);
                    throw;
                }
            }
            return !handles_.empty();
        } catch (...) {
            reset();
            return false;
        }
    }

    [[nodiscard]] HANDLE back() const noexcept {
        return handles_.empty() ? INVALID_HANDLE_VALUE : handles_.back();
    }

    [[nodiscard]] std::vector<HANDLE> release() noexcept {
        return std::exchange(handles_, {});
    }

private:
    void reset() noexcept {
        for (auto iterator = handles_.rbegin(); iterator != handles_.rend();
             ++iterator) {
            close_pin_handle(*iterator);
        }
        handles_.clear();
    }

    std::vector<HANDLE> handles_;
};
#endif

#if defined(_WIN32)
std::intptr_t native_handle_value(const HANDLE handle) noexcept {
    return reinterpret_cast<std::intptr_t>(handle);
}
#else
std::intptr_t native_handle_value(const int handle) noexcept {
    return static_cast<std::intptr_t>(handle);
}
#endif

struct ExecutableAuthenticationSnapshot {
    bool authenticated = false;
    std::string sha256;
    std::string diagnostic_code;
    std::vector<std::uint8_t> bytes;
};

ExecutableAuthenticationSnapshot authenticate_open_executable(
    const ScopedPinHandle::NativeHandle handle,
    const PhysicalPathIdentity& expected_identity,
    const bool retain_snapshot) {
    if (expected_identity.file_size >
        workspace_agent_process_max_executable_bytes) {
        return {
            .authenticated = false,
            .sha256 = {},
            .diagnostic_code =
                "workspace_agent.process_executable_authentication_size_limit",
            .bytes = {}};
    }
    PhysicalPathIdentity before{};
    if (!read_handle_identity(handle, false, before) ||
        before != expected_identity) {
        return {
            .authenticated = false,
            .sha256 = {},
            .diagnostic_code =
                "workspace_agent.process_executable_authentication_changed",
            .bytes = {}};
    }
    auto snapshot = retain_snapshot
        ? sha256_snapshot_for_native_file(
              native_handle_value(handle),
              workspace_agent_process_max_executable_bytes)
        : NativeFileSha256SnapshotResult{
              .digest = sha256_hex_for_native_file(
                  native_handle_value(handle),
                  workspace_agent_process_max_executable_bytes),
              .bytes = {}};
    PhysicalPathIdentity after{};
    if (!snapshot.digest.ok || snapshot.digest.hex_digest.size() != 64U ||
        !read_handle_identity(handle, false, after) ||
        after != expected_identity) {
        return {
            .authenticated = false,
            .sha256 = {},
            .diagnostic_code =
                "workspace_agent.process_executable_authentication_changed",
            .bytes = {}};
    }
    return {
        .authenticated = true,
        .sha256 = std::move(snapshot.digest.hex_digest),
        .diagnostic_code =
            "workspace_agent.process_executable_authenticated",
        .bytes = retain_snapshot ? std::move(snapshot.bytes)
                                 : std::vector<std::uint8_t>{}};
}

ExecutableAuthenticationSnapshot authenticate_executable_path(
    const std::filesystem::path& path,
    const PhysicalPathIdentity& expected_identity) {
    ScopedPinHandle handle(open_pin_handle(path, false));
    if (!handle.valid()) {
        return {
            .authenticated = false,
            .sha256 = {},
            .diagnostic_code =
                "workspace_agent.process_executable_authentication_unavailable",
            .bytes = {}};
    }
    return authenticate_open_executable(handle.get(), expected_identity, false);
}

bool stable_directory_identity_matches(
    const PhysicalPathIdentity& actual,
    const std::uint64_t expected_storage_id,
    const std::uint64_t expected_file_id) noexcept {
    return actual.storage_id == expected_storage_id &&
        actual.file_id == expected_file_id;
}

}  // namespace

WorkspaceAgentProcessTargetAuthenticationResult
WorkspaceAgentProcessTargetPins::Impl::verify_executable_bytes() {
    if (!valid()) {
        return {
            .authenticated = false,
            .diagnostic_code =
                "workspace_agent.process_executable_authentication_unavailable"};
    }
    const auto current = sha256_hex_for_native_bytes(executable_snapshot);
    const bool matches = current.ok && current.hex_digest == executable_sha256;
    return {
        .authenticated = matches,
        .diagnostic_code = matches
            ? "workspace_agent.process_executable_authentication_matched"
            : "workspace_agent.process_executable_authentication_changed"};
}

bool WorkspaceAgentProcessTargetPins::Impl::matches_target_identities(
    const PhysicalPathIdentity& expected_executable_identity,
    const PhysicalPathIdentity& expected_working_directory_identity) const
    noexcept {
    if (!valid()) {
        return false;
    }
    PhysicalPathIdentity current_executable_identity{};
    PhysicalPathIdentity current_working_directory_identity{};
    return read_handle_identity(
               executable, false, current_executable_identity) &&
        read_handle_identity(
#if defined(_WIN32)
            working_directory_chain.back(), true,
#else
            working_directory, true,
#endif
            current_working_directory_identity) &&
        current_executable_identity == expected_executable_identity &&
        current_working_directory_identity ==
            expected_working_directory_identity;
}

const std::filesystem::path*
WorkspaceAgentProcessTargetPins::execution_working_directory() const noexcept {
#if defined(_WIN32)
    return valid() ? &impl_->execution_working_directory() : nullptr;
#else
    return nullptr;
#endif
}

WorkspaceAgentProcessTargetBoundary::WorkspaceAgentProcessTargetBoundary(
    std::filesystem::path canonical_workspace_root,
    std::uint64_t workspace_storage_id,
    std::uint64_t workspace_file_id)
    : canonical_workspace_root_(std::move(canonical_workspace_root)),
      workspace_storage_id_(workspace_storage_id),
      workspace_file_id_(workspace_file_id),
      pin_boundary_authority_(
          std::make_shared<WorkspaceAgentProcessTargetBoundaryAuthority>()) {}

std::optional<WorkspaceAgentProcessTargetBoundary>
WorkspaceAgentProcessTargetBoundary::create(
    const std::filesystem::path& trusted_absolute_workspace_root) {
    if (!strict_absolute_working_directory_path(trusted_absolute_workspace_root) ||
        !path_is_direct_directory(trusted_absolute_workspace_root)) {
        return std::nullopt;
    }
    const auto containment = inspect_physical_path_containment(
        trusted_absolute_workspace_root, trusted_absolute_workspace_root);
    if (!containment.allowed) {
        return std::nullopt;
    }
    std::error_code filesystem_error;
    if (!std::filesystem::is_directory(
            containment.canonical_path, filesystem_error) ||
        filesystem_error) {
        return std::nullopt;
    }
    return WorkspaceAgentProcessTargetBoundary(
        containment.canonical_path,
        containment.identity.storage_id,
        containment.identity.file_id);
}

bool WorkspaceAgentProcessTargetBoundary::workspace_root_identity_matches() const {
    if (!path_is_direct_directory(canonical_workspace_root_)) {
        return false;
    }
    const auto current = inspect_physical_path_containment(
        canonical_workspace_root_, canonical_workspace_root_);
    return current.allowed &&
        current.identity.storage_id == workspace_storage_id_ &&
        current.identity.file_id == workspace_file_id_;
}

WorkspaceAgentProcessTargetInspection
WorkspaceAgentProcessTargetBoundary::inspect_workspace_process(
    const std::filesystem::path& strict_relative_executable,
    const std::filesystem::path& strict_relative_working_directory) const {
    return inspect_workspace_process_impl(
        strict_relative_executable,
        strict_relative_working_directory,
        true);
}

WorkspaceAgentProcessTargetInspection
WorkspaceAgentProcessTargetBoundary::preflight_workspace_process(
    const std::filesystem::path& strict_relative_executable,
    const std::filesystem::path& strict_relative_working_directory) const {
    return inspect_workspace_process_impl(
        strict_relative_executable,
        strict_relative_working_directory,
        false);
}

WorkspaceAgentProcessTargetInspection
WorkspaceAgentProcessTargetBoundary::inspect_workspace_process_impl(
    const std::filesystem::path& strict_relative_executable,
    const std::filesystem::path& strict_relative_working_directory,
    const bool authorize_pinning) const {
    if (!strict_relative_executable_path(strict_relative_executable)) {
        return denied("workspace_agent.process_invalid_relative_executable");
    }
    if (!strict_relative_working_directory_path(
            strict_relative_working_directory)) {
        return denied("workspace_agent.process_invalid_relative_working_directory");
    }
    if (!workspace_root_identity_matches()) {
        return denied("workspace_agent.process_workspace_root_identity_changed");
    }

    WorkspaceAgentProcessTargetInspection failure_result;
    const auto executable = inspect_executable(
        canonical_workspace_root_ / strict_relative_executable,
        canonical_workspace_root_,
        failure_result);
    if (!executable.has_value()) {
        return failure_result;
    }
    ExecutableAuthenticationSnapshot authentication;
    if (authorize_pinning) {
        authentication = authenticate_executable_path(
            executable->canonical_path, executable->identity);
        if (!authentication.authenticated) {
            return denied(authentication.diagnostic_code);
        }
    }
    const auto working_directory = inspect_working_directory(
        strict_relative_working_directory == "."
            ? canonical_workspace_root_
            : canonical_workspace_root_ / strict_relative_working_directory,
        canonical_workspace_root_,
        failure_result);
    if (!working_directory.has_value()) {
        return failure_result;
    }
    if (!workspace_root_identity_matches()) {
        return denied("workspace_agent.process_workspace_root_identity_changed");
    }
    auto result = allowed(*executable, *working_directory);
    if (authorize_pinning) {
        result.pin_authority_ =
            std::make_shared<WorkspaceAgentProcessTargetPinAuthority>(
                pin_boundary_authority_,
                result.canonical_executable_path,
                result.executable_identity,
                std::move(authentication.sha256),
                result.canonical_working_directory,
                result.working_directory_identity);
    }
    return result;
}

WorkspaceAgentProcessTargetInspection
WorkspaceAgentProcessTargetBoundary::inspect_local_process(
    const std::filesystem::path& strict_absolute_executable,
    const std::filesystem::path& strict_absolute_working_directory) const {
    return inspect_local_process_impl(
        strict_absolute_executable,
        strict_absolute_working_directory,
        true);
}

WorkspaceAgentProcessTargetInspection
WorkspaceAgentProcessTargetBoundary::preflight_local_process(
    const std::filesystem::path& strict_absolute_executable,
    const std::filesystem::path& strict_absolute_working_directory) const {
    return inspect_local_process_impl(
        strict_absolute_executable,
        strict_absolute_working_directory,
        false);
}

WorkspaceAgentProcessTargetInspection
WorkspaceAgentProcessTargetBoundary::inspect_local_process_impl(
    const std::filesystem::path& strict_absolute_executable,
    const std::filesystem::path& strict_absolute_working_directory,
    const bool authorize_pinning) const {
    if (!strict_absolute_executable_path(strict_absolute_executable)) {
        return denied("workspace_agent.process_invalid_absolute_executable");
    }
    if (!strict_absolute_working_directory_path(
            strict_absolute_working_directory)) {
        return denied("workspace_agent.process_invalid_absolute_working_directory");
    }
    const std::filesystem::path executable_parent =
        strict_absolute_executable.parent_path();
    if (executable_parent.empty()) {
        return denied("workspace_agent.process_invalid_absolute_executable");
    }
    if (!path_is_direct_directory(strict_absolute_working_directory)) {
        return denied("workspace_agent.process_working_directory_indirect_component");
    }

    WorkspaceAgentProcessTargetInspection failure_result;
    const auto executable = inspect_executable(
        strict_absolute_executable, executable_parent, failure_result);
    if (!executable.has_value()) {
        return failure_result;
    }
    ExecutableAuthenticationSnapshot authentication;
    if (authorize_pinning) {
        authentication = authenticate_executable_path(
            executable->canonical_path, executable->identity);
        if (!authentication.authenticated) {
            return denied(authentication.diagnostic_code);
        }
    }
    const auto working_directory = inspect_working_directory(
        strict_absolute_working_directory,
        strict_absolute_working_directory,
        failure_result);
    if (!working_directory.has_value()) {
        return failure_result;
    }
    auto result = allowed(*executable, *working_directory);
    if (authorize_pinning) {
        result.pin_authority_ =
            std::make_shared<WorkspaceAgentProcessTargetPinAuthority>(
                pin_boundary_authority_,
                result.canonical_executable_path,
                result.executable_identity,
                std::move(authentication.sha256),
                result.canonical_working_directory,
                result.working_directory_identity);
    }
    return result;
}

WorkspaceAgentProcessTargetPinResult
WorkspaceAgentProcessTargetBoundary::pin_process_targets(
    const WorkspaceAgentProcessTargetInspection& inspection) const {
    WorkspaceAgentProcessTargetPinResult result;
    const auto authority = inspection.pin_authority_;
    if (!inspection.allowed || pin_boundary_authority_ == nullptr ||
        authority == nullptr ||
        authority->boundary_authority != pin_boundary_authority_) {
        result.diagnostic_code =
            "workspace_agent.process_target_pin_authority_unavailable";
        return result;
    }
    const bool already_consumed = authority->consumed.exchange(true);
    if (already_consumed ||
        authority->executable_path != inspection.canonical_executable_path ||
        authority->executable_identity != inspection.executable_identity ||
        authority->working_directory != inspection.canonical_working_directory ||
        authority->working_directory_identity !=
            inspection.working_directory_identity) {
        result.diagnostic_code =
            "workspace_agent.process_target_pin_authority_unavailable";
        return result;
    }

    ScopedPinHandle root_handle(
        open_pin_handle(canonical_workspace_root_, true));
    if (!root_handle.valid()) {
        result.diagnostic_code =
            "workspace_agent.process_target_pin_identity_changed";
        return result;
    }
    PhysicalPathIdentity root_identity{};
    if (!read_handle_identity(root_handle.get(), true, root_identity) ||
        !stable_directory_identity_matches(
            root_identity, workspace_storage_id_, workspace_file_id_)) {
        result.diagnostic_code =
            "workspace_agent.process_target_pin_identity_changed";
        return result;
    }

    ScopedPinHandle executable_handle(
        open_pin_handle(authority->executable_path, false));
    if (!executable_handle.valid()) {
        result.diagnostic_code =
            "workspace_agent.process_target_pin_identity_changed";
        return result;
    }
    PhysicalPathIdentity executable_identity{};
    if (!read_handle_identity(
            executable_handle.get(), false, executable_identity) ||
        executable_identity != authority->executable_identity) {
        result.diagnostic_code =
            "workspace_agent.process_target_pin_identity_changed";
        return result;
    }
    auto authentication = authenticate_open_executable(
        executable_handle.get(), executable_identity, true);
    if (!authentication.authenticated) {
        result.diagnostic_code = authentication.diagnostic_code;
        return result;
    }
    if (authentication.sha256 != authority->executable_sha256) {
        result.diagnostic_code =
            "workspace_agent.process_executable_authentication_changed";
        return result;
    }

    ScopedPinHandle working_directory_handle(
        open_pin_handle(authority->working_directory, true));
    if (!working_directory_handle.valid()) {
        result.diagnostic_code =
            "workspace_agent.process_target_pin_identity_changed";
        return result;
    }
    PhysicalPathIdentity working_directory_identity{};
    if (!read_handle_identity(
            working_directory_handle.get(), true, working_directory_identity) ||
        working_directory_identity != authority->working_directory_identity) {
        result.diagnostic_code =
            "workspace_agent.process_target_pin_identity_changed";
        return result;
    }

#if defined(_WIN32)
    const auto stable_working_directory =
        stable_volume_path_for_handle(working_directory_handle.get());
    const auto dos_working_directory =
        dos_volume_path_for_handle(working_directory_handle.get());
    ScopedPinHandleChain working_directory_chain;
    PhysicalPathIdentity chained_working_directory_identity{};
    if (!stable_working_directory.has_value() ||
        !dos_working_directory.has_value() ||
        !dos_volume_binding_matches_handle(
            *dos_working_directory, working_directory_handle.get(),
            working_directory_identity.storage_id) ||
        !working_directory_chain.lock(
            *stable_working_directory, working_directory_handle.get()) ||
        !read_handle_identity(
            working_directory_chain.back(), true,
            chained_working_directory_identity) ||
        chained_working_directory_identity != authority->working_directory_identity) {
        result.diagnostic_code =
            "workspace_agent.process_target_pin_identity_changed";
        return result;
    }
    ScopedPinHandle dos_working_directory_handle(
        open_pin_handle(*dos_working_directory, true));
    PhysicalPathIdentity dos_working_directory_identity{};
    if (!dos_working_directory_handle.valid() ||
        !read_handle_identity(
            dos_working_directory_handle.get(), true,
            dos_working_directory_identity) ||
        dos_working_directory_identity != authority->working_directory_identity ||
        !dos_volume_binding_matches_handle(
            *dos_working_directory, working_directory_handle.get(),
            working_directory_identity.storage_id)) {
        result.diagnostic_code =
            "workspace_agent.process_target_pin_identity_changed";
        return result;
    }
#endif

#if defined(_WIN32)
    // Finish every potentially throwing copy before the new-expression.
    // Allocation then precedes the no-throw raw-handle transfer, so failure
    // leaves the scoped chain responsible for closing every acquired handle.
    std::filesystem::path retained_working_directory =
        *dos_working_directory;
    std::string retained_executable_sha256 = authority->executable_sha256;
    auto impl = std::unique_ptr<WorkspaceAgentProcessTargetPins::Impl>(
        new WorkspaceAgentProcessTargetPins::Impl(
            root_handle.get(), executable_handle.get(),
            working_directory_chain.release(),
            std::move(retained_working_directory), executable_identity,
            std::move(retained_executable_sha256),
            std::move(authentication.bytes)));
#else
    auto impl = std::make_unique<WorkspaceAgentProcessTargetPins::Impl>(
        root_handle.get(),
        executable_handle.get(),
        working_directory_handle.get(),
        executable_identity,
        authority->executable_sha256,
        std::move(authentication.bytes));
#endif
    result.pins.emplace(WorkspaceAgentProcessTargetPins(std::move(impl)));
    static_cast<void>(root_handle.release());
    static_cast<void>(executable_handle.release());
#if !defined(_WIN32)
    static_cast<void>(working_directory_handle.release());
#endif
    result.pinned = result.pins->valid();
    result.diagnostic_code = result.pinned
        ? "workspace_agent.process_target_pins_acquired"
        : "workspace_agent.process_target_pin_identity_changed";
    return result;
}

}  // namespace copperfin::security
