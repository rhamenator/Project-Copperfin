// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/private_directory.h"

#include <algorithm>
#include <optional>
#include <system_error>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <aclapi.h>

#include <array>
#include <vector>
#else
#include <cerrno>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace copperfin::platform {
namespace {

PrivateDirectoryResult failed(const PrivateDirectoryFailure failure) noexcept {
    return {.ok = false, .failure = failure};
}

bool valid_absolute_path(const std::filesystem::path& path) noexcept {
    if (path.empty() || !path.is_absolute() || path.filename().empty()) {
        return false;
    }
    const auto& native = path.native();
    if (native.find(typename std::filesystem::path::value_type{}) !=
        std::filesystem::path::string_type::npos) {
        return false;
    }
    return std::none_of(path.begin(), path.end(), [](const auto& component) {
        return component == "." || component == "..";
    });
}

#if defined(_WIN32)

class ScopedHandle {
public:
    explicit ScopedHandle(const HANDLE value = INVALID_HANDLE_VALUE) noexcept
        : value_(value) {}

    ~ScopedHandle() {
        if (value_ != INVALID_HANDLE_VALUE && value_ != nullptr) {
            ::CloseHandle(value_);
        }
    }

    ScopedHandle(const ScopedHandle&) = delete;
    ScopedHandle& operator=(const ScopedHandle&) = delete;

    [[nodiscard]] HANDLE get() const noexcept { return value_; }
    [[nodiscard]] bool valid() const noexcept {
        return value_ != INVALID_HANDLE_VALUE && value_ != nullptr;
    }

private:
    HANDLE value_ = INVALID_HANDLE_VALUE;
};

class ScopedLocalMemory {
public:
    explicit ScopedLocalMemory(void* value = nullptr) noexcept : value_(value) {}
    ~ScopedLocalMemory() {
        if (value_ != nullptr) {
            ::LocalFree(value_);
        }
    }

    ScopedLocalMemory(const ScopedLocalMemory&) = delete;
    ScopedLocalMemory& operator=(const ScopedLocalMemory&) = delete;

    [[nodiscard]] void* get() const noexcept { return value_; }

private:
    void* value_ = nullptr;
};

std::vector<unsigned char> current_user_sid() {
    HANDLE raw_token = nullptr;
    if (::OpenProcessToken(::GetCurrentProcess(), TOKEN_QUERY, &raw_token) == 0) {
        return {};
    }
    ScopedHandle token(raw_token);

    DWORD required = 0U;
    ::GetTokenInformation(token.get(), TokenUser, nullptr, 0U, &required);
    if (required == 0U || ::GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
        return {};
    }
    std::vector<unsigned char> token_buffer(required);
    if (::GetTokenInformation(
            token.get(), TokenUser, token_buffer.data(), required, &required) == 0) {
        return {};
    }
    const auto* token_user =
        reinterpret_cast<const TOKEN_USER*>(token_buffer.data());
    if (::IsValidSid(token_user->User.Sid) == 0) {
        return {};
    }
    const DWORD sid_size = ::GetLengthSid(token_user->User.Sid);
    std::vector<unsigned char> sid(sid_size);
    if (::CopySid(sid_size, sid.data(), token_user->User.Sid) == 0) {
        return {};
    }
    return sid;
}

std::vector<unsigned char> local_system_sid() {
    std::vector<unsigned char> sid(SECURITY_MAX_SID_SIZE);
    DWORD sid_size = static_cast<DWORD>(sid.size());
    if (::CreateWellKnownSid(
            WinLocalSystemSid, nullptr, sid.data(), &sid_size) == 0) {
        return {};
    }
    sid.resize(sid_size);
    return sid;
}

PrivateDirectoryFailure map_creation_error(const DWORD error) noexcept {
    switch (error) {
    case ERROR_ALREADY_EXISTS:
    case ERROR_FILE_EXISTS:
        return PrivateDirectoryFailure::already_exists;
    case ERROR_PATH_NOT_FOUND:
    case ERROR_INVALID_DRIVE:
        return PrivateDirectoryFailure::parent_unavailable;
    case ERROR_ACCESS_DENIED:
    case ERROR_PRIVILEGE_NOT_HELD:
        return PrivateDirectoryFailure::access_denied;
    case ERROR_NOT_SUPPORTED:
        return PrivateDirectoryFailure::security_unavailable;
    default:
        return PrivateDirectoryFailure::creation_failed;
    }
}

bool ace_grants_private_full_control(
    const void* raw_ace,
    const PSID user_sid,
    const PSID system_sid,
    bool& found_user,
    bool& found_system) noexcept {
    const auto* header = static_cast<const ACE_HEADER*>(raw_ace);
    if (header->AceType != ACCESS_ALLOWED_ACE_TYPE ||
        header->AceFlags !=
            (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE)) {
        return false;
    }
    const auto* ace = static_cast<const ACCESS_ALLOWED_ACE*>(raw_ace);
    if (ace->Mask != FILE_ALL_ACCESS) {
        return false;
    }
    const PSID sid = const_cast<DWORD*>(&ace->SidStart);
    if (::EqualSid(sid, user_sid) != 0) {
        if (found_user) {
            return false;
        }
        found_user = true;
        return true;
    }
    if (::EqualSid(sid, system_sid) != 0) {
        if (found_system) {
            return false;
        }
        found_system = true;
        return true;
    }
    return false;
}

bool windows_parent_components_are_direct(
    const std::filesystem::path& path) noexcept {
    try {
        std::filesystem::path current = path.root_path();
        if (current.empty()) {
            return false;
        }
        for (const auto& component : path.relative_path().parent_path()) {
            if (component.empty() || component == "." || component == "..") {
                return false;
            }
            current /= component;
            ScopedHandle directory(::CreateFileW(
                current.c_str(),
                FILE_READ_ATTRIBUTES,
                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                nullptr,
                OPEN_EXISTING,
                FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT,
                nullptr));
            if (!directory.valid()) {
                return false;
            }
            BY_HANDLE_FILE_INFORMATION information{};
            if (::GetFileInformationByHandle(
                    directory.get(), &information) == 0 ||
                (information.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0U ||
                (information.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0U) {
                return false;
            }
        }
        return true;
    } catch (...) {
        return false;
    }
}

bool windows_directory_identity_matches(
    const HANDLE directory,
    const std::uint64_t expected_storage_id,
    const std::uint64_t expected_file_id) noexcept {
    BY_HANDLE_FILE_INFORMATION information{};
    return ::GetFileInformationByHandle(directory, &information) != 0 &&
        information.dwVolumeSerialNumber == expected_storage_id &&
        ((static_cast<std::uint64_t>(information.nFileIndexHigh) << 32U) |
         information.nFileIndexLow) == expected_file_id;
}

#endif

#if !defined(_WIN32)

class ScopedFd {
public:
    explicit ScopedFd(const int value = -1) noexcept : value_(value) {}
    ~ScopedFd() {
        if (value_ >= 0) {
            ::close(value_);
        }
    }

    ScopedFd(const ScopedFd&) = delete;
    ScopedFd& operator=(const ScopedFd&) = delete;

    ScopedFd(ScopedFd&& other) noexcept : value_(other.value_) {
        other.value_ = -1;
    }
    ScopedFd& operator=(ScopedFd&& other) noexcept {
        if (this != &other) {
            if (value_ >= 0) {
                ::close(value_);
            }
            value_ = other.value_;
            other.value_ = -1;
        }
        return *this;
    }

    [[nodiscard]] int get() const noexcept { return value_; }
    [[nodiscard]] bool valid() const noexcept { return value_ >= 0; }

private:
    int value_ = -1;
};

constexpr int directory_open_flags() noexcept {
#if defined(__linux__) && defined(O_PATH)
    return O_PATH | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC;
#else
    return O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC;
#endif
}

struct BoundParent {
    ScopedFd descriptor;
    std::string leaf;
};

std::optional<BoundParent> bind_non_indirect_parent(
    const std::filesystem::path& path,
    int& failure_error) {
    failure_error = 0;
    ScopedFd current(::open("/", directory_open_flags()));
    if (!current.valid()) {
        failure_error = errno;
        return std::nullopt;
    }
    const auto relative = path.relative_path();
    const auto leaf_path = relative.filename();
    if (leaf_path.empty()) {
        failure_error = EINVAL;
        return std::nullopt;
    }
    for (const auto& component : relative.parent_path()) {
        const std::string name = component.native();
        if (name.empty() || name == "." || name == "..") {
            failure_error = EINVAL;
            return std::nullopt;
        }
        ScopedFd next(::openat(
            current.get(), name.c_str(), directory_open_flags()));
        if (!next.valid()) {
            failure_error = errno;
            return std::nullopt;
        }
        current = std::move(next);
    }
    return BoundParent{
        .descriptor = std::move(current),
        .leaf = leaf_path.native()};
}

bool descriptor_is_private_directory(const int descriptor) noexcept {
    struct stat status{};
    return ::fstat(descriptor, &status) == 0 && S_ISDIR(status.st_mode) &&
        status.st_uid == ::geteuid() && (status.st_mode & 07777) == 0700;
}

bool descriptor_identity_matches(
    const int descriptor,
    const std::uint64_t expected_storage_id,
    const std::uint64_t expected_file_id) noexcept {
    struct stat status{};
    return ::fstat(descriptor, &status) == 0 &&
        static_cast<std::uint64_t>(status.st_dev) == expected_storage_id &&
        static_cast<std::uint64_t>(status.st_ino) == expected_file_id;
}

PrivateDirectoryFailure map_posix_creation_error(const int error) noexcept {
    switch (error) {
    case EEXIST:
        return PrivateDirectoryFailure::already_exists;
    case ENOENT:
    case ENOTDIR:
    case ELOOP:
        return PrivateDirectoryFailure::parent_unavailable;
    case EACCES:
    case EPERM:
        return PrivateDirectoryFailure::access_denied;
    default:
        return PrivateDirectoryFailure::creation_failed;
    }
}

#endif

}  // namespace

PrivateDirectoryResult verify_private_directory(
    const std::filesystem::path& path) noexcept {
    if (!valid_absolute_path(path)) {
        return failed(PrivateDirectoryFailure::invalid_path);
    }

#if defined(_WIN32)
    if (!windows_parent_components_are_direct(path)) {
        return failed(PrivateDirectoryFailure::verification_failed);
    }
    const auto user_sid = current_user_sid();
    const auto system_sid = local_system_sid();
    if (user_sid.empty() || system_sid.empty()) {
        return failed(PrivateDirectoryFailure::security_unavailable);
    }

    ScopedHandle directory(::CreateFileW(
        path.c_str(),
        FILE_READ_ATTRIBUTES | READ_CONTROL,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        nullptr,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT,
        nullptr));
    if (!directory.valid()) {
        return failed(PrivateDirectoryFailure::verification_failed);
    }
    BY_HANDLE_FILE_INFORMATION information{};
    if (::GetFileInformationByHandle(directory.get(), &information) == 0 ||
        (information.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0U ||
        (information.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0U) {
        return failed(PrivateDirectoryFailure::verification_failed);
    }

    PSID owner = nullptr;
    PACL dacl = nullptr;
    PSECURITY_DESCRIPTOR raw_descriptor = nullptr;
    const DWORD security_result = ::GetSecurityInfo(
        directory.get(),
        SE_FILE_OBJECT,
        OWNER_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION,
        &owner,
        nullptr,
        &dacl,
        nullptr,
        &raw_descriptor);
    ScopedLocalMemory descriptor(raw_descriptor);
    if (security_result != ERROR_SUCCESS || descriptor.get() == nullptr ||
        owner == nullptr || dacl == nullptr ||
        ::EqualSid(owner, const_cast<unsigned char*>(user_sid.data())) == 0) {
        return failed(PrivateDirectoryFailure::verification_failed);
    }

    SECURITY_DESCRIPTOR_CONTROL control = 0U;
    DWORD revision = 0U;
    if (::GetSecurityDescriptorControl(
            descriptor.get(), &control, &revision) == 0 ||
        (control & SE_DACL_PRESENT) == 0U ||
        (control & SE_DACL_PROTECTED) == 0U) {
        return failed(PrivateDirectoryFailure::verification_failed);
    }

    ACL_SIZE_INFORMATION size_information{};
    if (::GetAclInformation(
            dacl,
            &size_information,
            static_cast<DWORD>(sizeof(size_information)),
            AclSizeInformation) == 0) {
        return failed(PrivateDirectoryFailure::verification_failed);
    }
    const bool user_is_system = ::EqualSid(
        const_cast<unsigned char*>(user_sid.data()),
        const_cast<unsigned char*>(system_sid.data())) != 0;
    const DWORD expected_aces = user_is_system ? 1U : 2U;
    if (size_information.AceCount != expected_aces) {
        return failed(PrivateDirectoryFailure::verification_failed);
    }

    bool found_user = false;
    bool found_system = false;
    for (DWORD index = 0U; index < size_information.AceCount; ++index) {
        void* ace = nullptr;
        if (::GetAce(dacl, index, &ace) == 0 ||
            !ace_grants_private_full_control(
                ace,
                const_cast<unsigned char*>(user_sid.data()),
                const_cast<unsigned char*>(system_sid.data()),
                found_user,
                found_system)) {
            return failed(PrivateDirectoryFailure::verification_failed);
        }
    }
    if (!found_user || (!user_is_system && !found_system)) {
        return failed(PrivateDirectoryFailure::verification_failed);
    }
#else
    int parent_error = 0;
    const auto parent = bind_non_indirect_parent(path, parent_error);
    (void)parent_error;
    if (!parent.has_value()) {
        return failed(PrivateDirectoryFailure::verification_failed);
    }
    ScopedFd directory(::openat(
        parent->descriptor.get(), parent->leaf.c_str(),
        directory_open_flags()));
    if (!directory.valid() ||
        !descriptor_is_private_directory(directory.get())) {
        return failed(PrivateDirectoryFailure::verification_failed);
    }
#endif

    return {.ok = true, .failure = PrivateDirectoryFailure::none};
}

PrivateDirectoryResult create_private_directory(
    const std::filesystem::path& path) noexcept {
    if (!valid_absolute_path(path)) {
        return failed(PrivateDirectoryFailure::invalid_path);
    }

#if defined(_WIN32)
    if (!windows_parent_components_are_direct(path)) {
        return failed(PrivateDirectoryFailure::parent_unavailable);
    }
    const auto user_sid = current_user_sid();
    const auto system_sid = local_system_sid();
    if (user_sid.empty() || system_sid.empty()) {
        return failed(PrivateDirectoryFailure::security_unavailable);
    }

    const bool user_is_system = ::EqualSid(
        const_cast<unsigned char*>(user_sid.data()),
        const_cast<unsigned char*>(system_sid.data())) != 0;
    std::array<EXPLICIT_ACCESSW, 2U> access{};
    const DWORD access_count = user_is_system ? 1U : 2U;
    for (DWORD index = 0U; index < access_count; ++index) {
        access[index].grfAccessPermissions = FILE_ALL_ACCESS;
        access[index].grfAccessMode = SET_ACCESS;
        access[index].grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
        ::BuildTrusteeWithSidW(
            &access[index].Trustee,
            index == 0U
                ? const_cast<unsigned char*>(user_sid.data())
                : const_cast<unsigned char*>(system_sid.data()));
    }

    PACL raw_acl = nullptr;
    if (::SetEntriesInAclW(access_count, access.data(), nullptr, &raw_acl) !=
        ERROR_SUCCESS) {
        return failed(PrivateDirectoryFailure::security_unavailable);
    }
    ScopedLocalMemory acl(raw_acl);
    (void)acl;

    SECURITY_DESCRIPTOR security_descriptor{};
    if (::InitializeSecurityDescriptor(
            &security_descriptor, SECURITY_DESCRIPTOR_REVISION) == 0 ||
        ::SetSecurityDescriptorOwner(
            &security_descriptor,
            const_cast<unsigned char*>(user_sid.data()),
            FALSE) == 0 ||
        ::SetSecurityDescriptorDacl(
            &security_descriptor, TRUE, raw_acl, FALSE) == 0 ||
        ::SetSecurityDescriptorControl(
            &security_descriptor,
            SE_DACL_PROTECTED,
            SE_DACL_PROTECTED) == 0) {
        return failed(PrivateDirectoryFailure::security_unavailable);
    }

    SECURITY_ATTRIBUTES attributes{
        .nLength = static_cast<DWORD>(sizeof(SECURITY_ATTRIBUTES)),
        .lpSecurityDescriptor = &security_descriptor,
        .bInheritHandle = FALSE
    };
    if (::CreateDirectoryW(path.c_str(), &attributes) == 0) {
        return failed(map_creation_error(::GetLastError()));
    }
#else
    int parent_error = 0;
    const auto parent = bind_non_indirect_parent(path, parent_error);
    if (!parent.has_value()) {
        return failed(map_posix_creation_error(parent_error));
    }
    if (::mkdirat(
            parent->descriptor.get(), parent->leaf.c_str(), 0700) != 0) {
        return failed(map_posix_creation_error(errno));
    }
    ScopedFd directory(::openat(
        parent->descriptor.get(), parent->leaf.c_str(),
        directory_open_flags()));
    if (!directory.valid() ||
        !descriptor_is_private_directory(directory.get())) {
        return failed(PrivateDirectoryFailure::verification_failed);
    }
#endif

#if defined(_WIN32)
    const PrivateDirectoryResult verified = verify_private_directory(path);
    if (!verified.ok) {
        return failed(PrivateDirectoryFailure::verification_failed);
    }
    return verified;
#else
    return {.ok = true, .failure = PrivateDirectoryFailure::none};
#endif
}

PrivateDirectoryResult create_private_directory_in_verified_parent(
    const std::filesystem::path& parent,
    const std::uint64_t expected_storage_id,
    const std::uint64_t expected_file_id,
    const std::filesystem::path& leaf) noexcept {
    if (!valid_absolute_path(parent) || leaf.empty() || leaf.is_absolute() ||
        leaf.has_parent_path() || leaf == "." || leaf == ".." ||
        leaf.native().find(typename std::filesystem::path::value_type{}) !=
            std::filesystem::path::string_type::npos) {
        return failed(PrivateDirectoryFailure::invalid_path);
    }

#if defined(_WIN32)
    if (!windows_parent_components_are_direct(parent)) {
        return failed(PrivateDirectoryFailure::parent_unavailable);
    }
    ScopedHandle bound_parent(::CreateFileW(
        parent.c_str(),
        FILE_READ_ATTRIBUTES | READ_CONTROL,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        nullptr,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT,
        nullptr));
    if (!bound_parent.valid() ||
        !windows_directory_identity_matches(
            bound_parent.get(), expected_storage_id, expected_file_id) ||
        !verify_private_directory(parent).ok) {
        return failed(PrivateDirectoryFailure::parent_identity_changed);
    }
    const auto created = create_private_directory(parent / leaf);
    if (!created.ok) {
        return created;
    }
    if (!windows_directory_identity_matches(
            bound_parent.get(), expected_storage_id, expected_file_id) ||
        !verify_private_directory(parent).ok) {
        return failed(PrivateDirectoryFailure::parent_identity_changed);
    }
    ScopedHandle created_directory(::CreateFileW(
        (parent / leaf).c_str(), FILE_READ_ATTRIBUTES,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT, nullptr));
    BY_HANDLE_FILE_INFORMATION created_information{};
    if (!created_directory.valid() ||
        ::GetFileInformationByHandle(
            created_directory.get(), &created_information) == 0 ||
        (created_information.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0U ||
        (created_information.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0U) {
        return failed(PrivateDirectoryFailure::verification_failed);
    }
    return {
        .ok = true,
        .failure = PrivateDirectoryFailure::none,
        .storage_id = created_information.dwVolumeSerialNumber,
        .file_id =
            (static_cast<std::uint64_t>(created_information.nFileIndexHigh) << 32U) |
            created_information.nFileIndexLow};
#else
    int parent_error = 0;
    const auto parent_binding =
        bind_non_indirect_parent(parent, parent_error);
    if (!parent_binding.has_value()) {
        return failed(map_posix_creation_error(parent_error));
    }
    ScopedFd bound_parent(::openat(
        parent_binding->descriptor.get(), parent_binding->leaf.c_str(),
        directory_open_flags()));
    if (!bound_parent.valid() ||
        !descriptor_is_private_directory(bound_parent.get()) ||
        !descriptor_identity_matches(
            bound_parent.get(), expected_storage_id, expected_file_id)) {
        return failed(PrivateDirectoryFailure::parent_identity_changed);
    }
    const std::string leaf_name = leaf.native();
    if (::mkdirat(bound_parent.get(), leaf_name.c_str(), 0700) != 0) {
        return failed(map_posix_creation_error(errno));
    }
    ScopedFd directory(::openat(
        bound_parent.get(), leaf_name.c_str(), directory_open_flags()));
    if (!directory.valid() ||
        !descriptor_is_private_directory(directory.get()) ||
        !descriptor_identity_matches(
            bound_parent.get(), expected_storage_id, expected_file_id)) {
        return failed(PrivateDirectoryFailure::verification_failed);
    }
    struct stat created_status{};
    if (::fstat(directory.get(), &created_status) != 0) {
        return failed(PrivateDirectoryFailure::verification_failed);
    }
    return {
        .ok = true,
        .failure = PrivateDirectoryFailure::none,
        .storage_id = static_cast<std::uint64_t>(created_status.st_dev),
        .file_id = static_cast<std::uint64_t>(created_status.st_ino)};
#endif
}

}  // namespace copperfin::platform
