// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/security/audit_stream.h"

#include "copperfin/platform/path.h"
#include "copperfin/security/sha256.h"
#include "localized_text.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cctype>
#include <cstdint>
#include <cwctype>
#include <filesystem>
#include <fstream>
#include <limits>
#include <optional>
#include <sstream>
#include <system_error>
#include <vector>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <cerrno>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace copperfin::security {

namespace {

std::string now_utc_compact() {
    const auto now = std::chrono::system_clock::now().time_since_epoch();
    const auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
    return std::to_string(millis);
}

std::string escape_field(std::string value) {
    for (char& ch : value) {
        if (ch == '|' || ch == '\n' || ch == '\r') {
            ch = ' ';
        }
    }
    return value;
}

struct AuditTailReadResult {
    bool ok = false;
    std::string hash;
    std::string error;
};

std::vector<std::string> split_audit_line(const std::string& line) {
    std::vector<std::string> tokens;
    std::istringstream stream(line);
    std::string token;
    while (std::getline(stream, token, '|')) {
        tokens.push_back(token);
    }
    return tokens;
}

bool is_sha256_hex(const std::string& value) {
    return value.size() == 64U &&
        std::all_of(value.begin(), value.end(), [](const unsigned char ch) {
            return std::isxdigit(ch) != 0;
        });
}

AuditTailReadResult read_last_hash_from_text(const std::string& text) {
    if (text.empty()) {
        return {
            .ok = true,
            .hash = "GENESIS",
            .error = {}};
    }

    const char final_byte = text.back();
    std::istringstream input(text);
    std::string line;
    std::string last_line;
    while (std::getline(input, line)) {
        if (!line.empty()) {
            last_line = line;
        }
    }
    if (input.bad()) {
        return {
            .ok = false,
            .hash = {},
            .error = security_text("Security.Audit.Error.ReadExistingLogFailed")};
    }

    if (last_line.empty()) {
        return {
            .ok = true,
            .hash = "GENESIS",
            .error = {}};
    }
    if (final_byte != '\n') {
        return {
            .ok = false,
            .hash = {},
            .error = security_text("Security.Audit.Error.InvalidExistingLogTail")};
    }

    const auto fields = split_audit_line(last_line);
    if (fields.size() != 5U ||
        fields[0].empty() ||
        (fields[3] != "GENESIS" && !is_sha256_hex(fields[3])) ||
        !is_sha256_hex(fields[4])) {
        return {
            .ok = false,
            .hash = {},
            .error = security_text("Security.Audit.Error.InvalidExistingLogTail")};
    }

    return {
        .ok = true,
        .hash = fields[4],
        .error = {}};
}

std::string compute_entry_hash(const std::string& timestamp,
                              const std::string& event_name,
                              const std::string& detail,
                              const std::string& previous_hash) {
    const std::string payload = timestamp + "|" + event_name + "|" + detail + "|" + previous_hash;
    const auto hash = sha256_hex_for_text(payload);
    return hash.ok ? hash.hex_digest : std::string{};
}

bool relative_path_is_contained(const std::filesystem::path& path) {
    if (path.empty() || path.is_absolute()) {
        return false;
    }
    for (const auto& part : path) {
        if (part == "..") {
            return false;
        }
    }
    return true;
}

struct ContainedAuditPath {
    std::filesystem::path canonical_root;
    std::filesystem::path relative_path;
};

std::optional<ContainedAuditPath> resolve_contained_audit_path(
    const std::string& log_path,
    const std::string& package_root) {
    std::error_code error;
    const std::filesystem::path canonical_root =
        std::filesystem::canonical(
            copperfin::platform::path_from_utf8_string(package_root),
            error);
    if (error) {
        return std::nullopt;
    }
    const std::filesystem::path normalized_log_path =
        std::filesystem::weakly_canonical(
            copperfin::platform::path_from_utf8_string(log_path),
            error);
    if (error) {
        return std::nullopt;
    }
    std::filesystem::path relative_path;
#if defined(_WIN32)
    auto log_part = normalized_log_path.begin();
    bool root_matches = true;
    for (auto root_part = canonical_root.begin(); root_part != canonical_root.end(); ++root_part, ++log_part) {
        if (log_part == normalized_log_path.end() ||
            !copperfin::platform::path_component_equal_for_platform(*log_part, *root_part)) {
            root_matches = false;
            break;
        }
    }
    if (root_matches) {
        for (; log_part != normalized_log_path.end(); ++log_part) {
            relative_path /= *log_part;
        }
    }
#else
    relative_path = normalized_log_path.lexically_relative(canonical_root);
#endif
    if (!relative_path_is_contained(relative_path) || relative_path.filename().empty()) {
        return std::nullopt;
    }
    return ContainedAuditPath{
        .canonical_root = canonical_root,
        .relative_path = relative_path
    };
}

AuditAppendResult prepare_audit_line(
    const std::string& existing_text,
    const std::string& event_name,
    const std::string& detail,
    std::string& line) {
    const auto tail = read_last_hash_from_text(existing_text);
    if (!tail.ok) {
        return {.ok = false, .error = tail.error, .entry_hash = {}};
    }

    const std::string timestamp = now_utc_compact();
    const std::string safe_event = escape_field(event_name);
    const std::string safe_detail = escape_field(detail);
    const std::string signed_payload =
        timestamp + "|" + safe_event + "|" + safe_detail + "|" + tail.hash;
    const auto hash = sha256_hex_for_text(signed_payload);
    if (!hash.ok) {
        return {.ok = false, .error = hash.error, .entry_hash = {}};
    }

    line = signed_payload + "|" + hash.hex_digest + "\n";
    return {.ok = true, .error = {}, .entry_hash = hash.hex_digest};
}

std::atomic<std::uint64_t> audit_temp_sequence{0U};

#if defined(_WIN32)

class ScopedHandle {
public:
    explicit ScopedHandle(HANDLE handle = INVALID_HANDLE_VALUE) : handle_(handle) {}
    ~ScopedHandle() {
        if (handle_ != INVALID_HANDLE_VALUE) {
            ::CloseHandle(handle_);
        }
    }
    ScopedHandle(const ScopedHandle&) = delete;
    ScopedHandle& operator=(const ScopedHandle&) = delete;
    ScopedHandle(ScopedHandle&& other) noexcept : handle_(other.handle_) {
        other.handle_ = INVALID_HANDLE_VALUE;
    }
    ScopedHandle& operator=(ScopedHandle&& other) noexcept {
        if (this != &other) {
            if (handle_ != INVALID_HANDLE_VALUE) {
                ::CloseHandle(handle_);
            }
            handle_ = other.handle_;
            other.handle_ = INVALID_HANDLE_VALUE;
        }
        return *this;
    }
    [[nodiscard]] HANDLE get() const { return handle_; }
    [[nodiscard]] bool valid() const { return handle_ != INVALID_HANDLE_VALUE; }

private:
    HANDLE handle_ = INVALID_HANDLE_VALUE;
};

class ScopedNamedMutex {
public:
    explicit ScopedNamedMutex(const std::wstring& name) {
        handle_ = ::CreateMutexW(nullptr, FALSE, name.c_str());
        if (handle_ == nullptr) {
            return;
        }
        const DWORD wait_result = ::WaitForSingleObject(handle_, INFINITE);
        locked_ = wait_result == WAIT_OBJECT_0 || wait_result == WAIT_ABANDONED;
    }
    ~ScopedNamedMutex() {
        if (locked_) {
            (void)::ReleaseMutex(handle_);
        }
        if (handle_ != nullptr) {
            ::CloseHandle(handle_);
        }
    }
    ScopedNamedMutex(const ScopedNamedMutex&) = delete;
    ScopedNamedMutex& operator=(const ScopedNamedMutex&) = delete;
    [[nodiscard]] bool locked() const { return locked_; }

private:
    HANDLE handle_ = nullptr;
    bool locked_ = false;
};

std::optional<std::wstring> audit_mutex_name(const std::filesystem::path& leaf_path) {
    std::wstring normalized_path = leaf_path.native();
    std::transform(
        normalized_path.begin(),
        normalized_path.end(),
        normalized_path.begin(),
        [](const wchar_t ch) { return std::towlower(ch); });
    const std::string path_bytes(
        reinterpret_cast<const char*>(normalized_path.data()),
        normalized_path.size() * sizeof(wchar_t));
    const auto digest = sha256_hex_for_text(path_bytes);
    if (!digest.ok) {
        return std::nullopt;
    }
    return L"Global\\CopperfinAudit-" +
        std::wstring(digest.hex_digest.begin(), digest.hex_digest.end());
}

AuditAppendResult append_contained_audit_event(
    const ContainedAuditPath& path,
    const std::string& event_name,
    const std::string& detail) {
    std::vector<ScopedHandle> directory_handles;
    ScopedHandle root_handle(::CreateFileW(
        path.canonical_root.c_str(),
        FILE_READ_ATTRIBUTES,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT,
        nullptr));
    if (!root_handle.valid()) {
        return {.ok = false, .error = security_text("Security.Audit.Error.OpenLogForAppendFailed"), .entry_hash = {}};
    }
    BY_HANDLE_FILE_INFORMATION root_information{};
    if (::GetFileInformationByHandle(root_handle.get(), &root_information) == 0 ||
        (root_information.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0U ||
        (root_information.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0U) {
        return {.ok = false, .error = security_text("Security.Audit.Error.OpenLogForAppendFailed"), .entry_hash = {}};
    }
    directory_handles.push_back(std::move(root_handle));

    std::filesystem::path current = path.canonical_root;
    const std::filesystem::path parent_relative = path.relative_path.parent_path();
    for (const auto& part : parent_relative) {
        if (part == ".") {
            continue;
        }
        current /= part;
        if (::CreateDirectoryW(current.c_str(), nullptr) == 0) {
            const DWORD create_error = ::GetLastError();
            if (create_error != ERROR_ALREADY_EXISTS) {
                return {.ok = false, .error = security_text("Security.Audit.Error.CreateLogDirectoryFailed"), .entry_hash = {}};
            }
        }

        ScopedHandle directory_handle(::CreateFileW(
            current.c_str(),
            FILE_READ_ATTRIBUTES,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            nullptr,
            OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT,
            nullptr));
        BY_HANDLE_FILE_INFORMATION directory_information{};
        if (!directory_handle.valid() ||
            ::GetFileInformationByHandle(directory_handle.get(), &directory_information) == 0 ||
            (directory_information.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0U ||
            (directory_information.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0U ||
            directory_information.dwVolumeSerialNumber != root_information.dwVolumeSerialNumber) {
            return {.ok = false, .error = security_text("Security.Audit.Error.OpenLogForAppendFailed"), .entry_hash = {}};
        }
        directory_handles.push_back(std::move(directory_handle));
    }

    const std::filesystem::path leaf_path = current / path.relative_path.filename();
    const auto mutex_name = audit_mutex_name(leaf_path);
    if (!mutex_name.has_value()) {
        return {.ok = false, .error = security_text("Security.Audit.Error.OpenLogForAppendFailed"), .entry_hash = {}};
    }
    ScopedNamedMutex audit_mutex(*mutex_name);
    if (!audit_mutex.locked()) {
        return {.ok = false, .error = security_text("Security.Audit.Error.OpenLogForAppendFailed"), .entry_hash = {}};
    }

    std::string existing_text;
    {
        ScopedHandle file_handle(::CreateFileW(
            leaf_path.c_str(),
            GENERIC_READ,
            FILE_SHARE_READ,
            nullptr,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OPEN_REPARSE_POINT,
            nullptr));
        if (!file_handle.valid()) {
            const DWORD open_error = ::GetLastError();
            if (open_error != ERROR_FILE_NOT_FOUND && open_error != ERROR_PATH_NOT_FOUND) {
                return {.ok = false, .error = security_text("Security.Audit.Error.OpenLogForAppendFailed"), .entry_hash = {}};
            }
        } else {
            BY_HANDLE_FILE_INFORMATION file_information{};
            if (::GetFileInformationByHandle(file_handle.get(), &file_information) == 0 ||
                (file_information.dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT)) != 0U ||
                file_information.dwVolumeSerialNumber != root_information.dwVolumeSerialNumber ||
                file_information.nNumberOfLinks != 1U) {
                return {.ok = false, .error = security_text("Security.Audit.Error.OpenLogForAppendFailed"), .entry_hash = {}};
            }

            LARGE_INTEGER file_size{};
            if (::GetFileSizeEx(file_handle.get(), &file_size) == 0 ||
                file_size.QuadPart < 0 ||
                static_cast<unsigned long long>(file_size.QuadPart) >
                    static_cast<unsigned long long>(std::numeric_limits<std::size_t>::max())) {
                return {.ok = false, .error = security_text("Security.Audit.Error.ReadExistingLogFailed"), .entry_hash = {}};
            }
            existing_text.assign(static_cast<std::size_t>(file_size.QuadPart), '\0');
            std::size_t read_offset = 0U;
            while (read_offset < existing_text.size()) {
                const DWORD request = static_cast<DWORD>(std::min<std::size_t>(
                    existing_text.size() - read_offset,
                    std::numeric_limits<DWORD>::max()));
                DWORD read_count = 0U;
                if (::ReadFile(
                        file_handle.get(),
                        existing_text.data() + read_offset,
                        request,
                        &read_count,
                        nullptr) == 0 ||
                    read_count == 0U) {
                    return {.ok = false, .error = security_text("Security.Audit.Error.ReadExistingLogFailed"), .entry_hash = {}};
                }
                read_offset += read_count;
            }
        }
    }

    std::string line;
    const AuditAppendResult prepared = prepare_audit_line(existing_text, event_name, detail, line);
    if (!prepared.ok) {
        return prepared;
    }
    const std::string updated_text = existing_text + line;
    std::filesystem::path temp_path;
    ScopedHandle temp_handle;
    for (int attempt = 0; attempt < 64 && !temp_handle.valid(); ++attempt) {
        temp_path = current /
            (L".copperfin-audit-" + std::to_wstring(::GetCurrentProcessId()) + L"-" +
             std::to_wstring(audit_temp_sequence.fetch_add(1U, std::memory_order_relaxed)) + L".tmp");
        temp_handle = ScopedHandle(::CreateFileW(
            temp_path.c_str(),
            GENERIC_WRITE,
            0,
            nullptr,
            CREATE_NEW,
            FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_OPEN_REPARSE_POINT,
            nullptr));
        if (!temp_handle.valid() && ::GetLastError() != ERROR_FILE_EXISTS) {
            return {.ok = false, .error = security_text("Security.Audit.Error.OpenLogForAppendFailed"), .entry_hash = {}};
        }
    }
    if (!temp_handle.valid()) {
        return {.ok = false, .error = security_text("Security.Audit.Error.OpenLogForAppendFailed"), .entry_hash = {}};
    }
    bool temp_write_ok = true;
    std::size_t write_offset = 0U;
    while (write_offset < updated_text.size()) {
        const DWORD request = static_cast<DWORD>(std::min<std::size_t>(
            updated_text.size() - write_offset,
            std::numeric_limits<DWORD>::max()));
        DWORD written = 0U;
        if (::WriteFile(
                temp_handle.get(),
                updated_text.data() + write_offset,
                request,
                &written,
                nullptr) == 0 ||
            written == 0U) {
            temp_write_ok = false;
            break;
        }
        write_offset += written;
    }
    if (temp_write_ok && ::FlushFileBuffers(temp_handle.get()) == 0) {
        temp_write_ok = false;
    }
    temp_handle = ScopedHandle{};
    if (!temp_write_ok) {
        (void)::DeleteFileW(temp_path.c_str());
        return {.ok = false, .error = security_text("Security.Audit.Error.AppendLogEntryFailed"), .entry_hash = {}};
    }
    if (::MoveFileExW(
            temp_path.c_str(),
            leaf_path.c_str(),
            MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) == 0) {
        (void)::DeleteFileW(temp_path.c_str());
        return {.ok = false, .error = security_text("Security.Audit.Error.AppendLogEntryFailed"), .entry_hash = {}};
    }
    return prepared;
}

#else

class ScopedFileDescriptor {
public:
    explicit ScopedFileDescriptor(int descriptor = -1) : descriptor_(descriptor) {}
    ~ScopedFileDescriptor() {
        if (descriptor_ >= 0) {
            ::close(descriptor_);
        }
    }
    ScopedFileDescriptor(const ScopedFileDescriptor&) = delete;
    ScopedFileDescriptor& operator=(const ScopedFileDescriptor&) = delete;
    ScopedFileDescriptor(ScopedFileDescriptor&& other) noexcept : descriptor_(other.descriptor_) {
        other.descriptor_ = -1;
    }
    ScopedFileDescriptor& operator=(ScopedFileDescriptor&& other) noexcept {
        if (this != &other) {
            if (descriptor_ >= 0) {
                ::close(descriptor_);
            }
            descriptor_ = other.descriptor_;
            other.descriptor_ = -1;
        }
        return *this;
    }
    [[nodiscard]] int get() const { return descriptor_; }
    [[nodiscard]] bool valid() const { return descriptor_ >= 0; }

private:
    int descriptor_ = -1;
};

AuditAppendResult append_contained_audit_event(
    const ContainedAuditPath& path,
    const std::string& event_name,
    const std::string& detail) {
    ScopedFileDescriptor current_directory(::open(
        path.canonical_root.c_str(),
        O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC));
    struct stat root_status{};
    if (!current_directory.valid() ||
        ::fstat(current_directory.get(), &root_status) != 0 ||
        !S_ISDIR(root_status.st_mode)) {
        return {.ok = false, .error = security_text("Security.Audit.Error.OpenLogForAppendFailed"), .entry_hash = {}};
    }

    for (const auto& part : path.relative_path.parent_path()) {
        if (part == ".") {
            continue;
        }
        const std::string component = copperfin::platform::path_to_utf8_string(part);
        int next_descriptor = ::openat(
            current_directory.get(),
            component.c_str(),
            O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
        if (next_descriptor < 0 && errno == ENOENT) {
            if (::mkdirat(current_directory.get(), component.c_str(), 0700) != 0 &&
                errno != EEXIST) {
                return {.ok = false, .error = security_text("Security.Audit.Error.CreateLogDirectoryFailed"), .entry_hash = {}};
            }
            next_descriptor = ::openat(
                current_directory.get(),
                component.c_str(),
                O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
        }
        ScopedFileDescriptor next_directory(next_descriptor);
        struct stat directory_status{};
        if (!next_directory.valid() ||
            ::fstat(next_directory.get(), &directory_status) != 0 ||
            !S_ISDIR(directory_status.st_mode) ||
            directory_status.st_dev != root_status.st_dev) {
            return {.ok = false, .error = security_text("Security.Audit.Error.OpenLogForAppendFailed"), .entry_hash = {}};
        }
        current_directory = std::move(next_directory);
    }

    if (::flock(current_directory.get(), LOCK_EX) != 0) {
        return {.ok = false, .error = security_text("Security.Audit.Error.OpenLogForAppendFailed"), .entry_hash = {}};
    }

    const std::string leaf_name =
        copperfin::platform::path_to_utf8_string(path.relative_path.filename());
    std::string existing_text;
    ScopedFileDescriptor existing_descriptor(::openat(
        current_directory.get(),
        leaf_name.c_str(),
        O_RDONLY | O_NOFOLLOW | O_CLOEXEC));
    if (!existing_descriptor.valid()) {
        if (errno != ENOENT) {
            return {.ok = false, .error = security_text("Security.Audit.Error.OpenLogForAppendFailed"), .entry_hash = {}};
        }
    } else {
        struct stat file_status{};
        if (::fstat(existing_descriptor.get(), &file_status) != 0 ||
            !S_ISREG(file_status.st_mode) ||
            file_status.st_dev != root_status.st_dev ||
            file_status.st_nlink != 1) {
            return {.ok = false, .error = security_text("Security.Audit.Error.OpenLogForAppendFailed"), .entry_hash = {}};
        }
        if (file_status.st_size < 0 ||
            static_cast<std::uintmax_t>(file_status.st_size) >
                static_cast<std::uintmax_t>(std::numeric_limits<std::size_t>::max())) {
            return {.ok = false, .error = security_text("Security.Audit.Error.ReadExistingLogFailed"), .entry_hash = {}};
        }
        existing_text.assign(static_cast<std::size_t>(file_status.st_size), '\0');
        std::size_t read_offset = 0U;
        while (read_offset < existing_text.size()) {
            const ssize_t read_count = ::pread(
                existing_descriptor.get(),
                existing_text.data() + read_offset,
                existing_text.size() - read_offset,
                static_cast<off_t>(read_offset));
            if (read_count < 0 && errno == EINTR) {
                continue;
            }
            if (read_count <= 0) {
                return {.ok = false, .error = security_text("Security.Audit.Error.ReadExistingLogFailed"), .entry_hash = {}};
            }
            read_offset += static_cast<std::size_t>(read_count);
        }
    }

    std::string line;
    const AuditAppendResult prepared = prepare_audit_line(existing_text, event_name, detail, line);
    if (!prepared.ok) {
        return prepared;
    }
    const std::string updated_text = existing_text + line;
    std::string temp_name;
    ScopedFileDescriptor temp_descriptor;
    for (int attempt = 0; attempt < 64 && !temp_descriptor.valid(); ++attempt) {
        temp_name = ".copperfin-audit-" + std::to_string(::getpid()) + "-" +
            std::to_string(audit_temp_sequence.fetch_add(1U, std::memory_order_relaxed)) + ".tmp";
        temp_descriptor = ScopedFileDescriptor(::openat(
            current_directory.get(),
            temp_name.c_str(),
            O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC,
            0600));
        if (!temp_descriptor.valid() && errno != EEXIST) {
            return {.ok = false, .error = security_text("Security.Audit.Error.OpenLogForAppendFailed"), .entry_hash = {}};
        }
    }
    if (!temp_descriptor.valid()) {
        return {.ok = false, .error = security_text("Security.Audit.Error.OpenLogForAppendFailed"), .entry_hash = {}};
    }

    std::size_t write_offset = 0U;
    while (write_offset < updated_text.size()) {
        const ssize_t write_count = ::write(
            temp_descriptor.get(),
            updated_text.data() + write_offset,
            updated_text.size() - write_offset);
        if (write_count < 0 && errno == EINTR) {
            continue;
        }
        if (write_count <= 0) {
            (void)::unlinkat(current_directory.get(), temp_name.c_str(), 0);
            return {.ok = false, .error = security_text("Security.Audit.Error.AppendLogEntryFailed"), .entry_hash = {}};
        }
        write_offset += static_cast<std::size_t>(write_count);
    }
    if (::fsync(temp_descriptor.get()) != 0) {
        (void)::unlinkat(current_directory.get(), temp_name.c_str(), 0);
        return {.ok = false, .error = security_text("Security.Audit.Error.AppendLogEntryFailed"), .entry_hash = {}};
    }
    if (::renameat(
            current_directory.get(),
            temp_name.c_str(),
            current_directory.get(),
            leaf_name.c_str()) != 0) {
        (void)::unlinkat(current_directory.get(), temp_name.c_str(), 0);
        return {.ok = false, .error = security_text("Security.Audit.Error.AppendLogEntryFailed"), .entry_hash = {}};
    }
    if (::fsync(current_directory.get()) != 0) {
        return {.ok = false, .error = security_text("Security.Audit.Error.AppendLogEntryFailed"), .entry_hash = {}};
    }
    return prepared;
}

#endif

}  // namespace

AuditAppendResult append_immutable_audit_event(
    const std::string& log_path,
    const std::string& event_name,
    const std::string& detail) {
    const std::filesystem::path native_log_path = copperfin::platform::path_from_utf8_string(log_path);
    std::error_code error;
    std::filesystem::create_directories(native_log_path.parent_path(), error);
    if (error) {
        return {.ok = false, .error = security_text("Security.Audit.Error.CreateLogDirectoryFailed"), .entry_hash = {}};
    }
    const std::filesystem::path audit_root = native_log_path.parent_path().empty()
        ? std::filesystem::path(".")
        : native_log_path.parent_path();
    std::error_code leaf_status_error;
    const auto leaf_status = std::filesystem::symlink_status(native_log_path, leaf_status_error);
    if (!leaf_status_error && std::filesystem::is_directory(leaf_status)) {
        return {.ok = false, .error = security_text("Security.Audit.Error.ReadExistingLogFailed"), .entry_hash = {}};
    }
    const auto contained_path = resolve_contained_audit_path(
        log_path,
        copperfin::platform::path_to_utf8_string(audit_root));
    if (!contained_path.has_value()) {
        return {.ok = false, .error = security_text("Security.Audit.Error.OpenLogForAppendFailed"), .entry_hash = {}};
    }
    return append_contained_audit_event(*contained_path, event_name, detail);
}

AuditAppendResult append_immutable_audit_event_to_contained_file(
    const std::string& log_path,
    const std::string& package_root,
    const std::string& event_name,
    const std::string& detail) {
    const auto contained_path = resolve_contained_audit_path(log_path, package_root);
    if (!contained_path.has_value()) {
        return {.ok = false, .error = security_text("Security.Audit.Error.OpenLogForAppendFailed"), .entry_hash = {}};
    }
    return append_contained_audit_event(*contained_path, event_name, detail);
}

AuditChainVerifyResult verify_immutable_audit_chain(const std::string& log_path) {
    const std::filesystem::path native_log_path = copperfin::platform::path_from_utf8_string(log_path);
    std::ifstream input(native_log_path, std::ios::binary);
    if (!input) {
        return {.ok = true, .error = {}, .entries = 0U};
    }

    std::string line;
    std::size_t line_number = 0U;
    std::string previous_hash = "GENESIS";
    std::size_t verified = 0U;
    while (std::getline(input, line)) {
        if (line.empty()) {
            continue;
        }
        ++line_number;

        const auto fields = split_audit_line(line);
        if (fields.size() != 5U) {
            return {.ok = false,
                    .error = security_text(
                        "Security.Audit.Error.MalformedLine",
                        {{"lineNumber", std::to_string(line_number)}}),
                    .entries = verified};
        }

        const auto& timestamp = fields[0];
        const auto& event_name = fields[1];
        const auto& detail = fields[2];
        const auto& expected_previous_hash = fields[3];
        const auto& observed_hash = fields[4];
        if (expected_previous_hash != previous_hash) {
            return {.ok = false,
                    .error = security_text(
                        "Security.Audit.Error.ChainBrokenPreviousHashMismatch",
                        {{"lineNumber", std::to_string(line_number)}}),
                    .entries = verified};
        }

        const auto calculated_hash = compute_entry_hash(timestamp, event_name, detail, expected_previous_hash);
        if (calculated_hash.empty()) {
            return {.ok = false,
                    .error = security_text(
                        "Security.Audit.Error.ComputeHashAtLineFailed",
                        {{"lineNumber", std::to_string(line_number)}}),
                    .entries = verified};
        }
        if (calculated_hash != observed_hash) {
            return {.ok = false,
                    .error = security_text(
                        "Security.Audit.Error.HashMismatchAtLine",
                        {{"lineNumber", std::to_string(line_number)}}),
                    .entries = verified};
        }

        previous_hash = observed_hash;
        ++verified;
    }

    return {.ok = true, .error = {}, .entries = verified};
}

}  // namespace copperfin::security
