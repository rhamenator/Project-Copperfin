// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "runtime_pipeline_support.h"

#if !defined(_WIN32)

#include <array>
#include <cerrno>
#include <chrono>
#include <dirent.h>
#include <fcntl.h>
#include <fstream>
#include <limits>
#include <string>
#include <string_view>
#include <sys/stat.h>
#include <unistd.h>

#if defined(COPPERFIN_ENABLE_RUNTIME_PIPELINE_TEST_HOOKS)
#include <iostream>
#endif

namespace copperfin::runtime::runtime_pipeline_detail {
namespace {

struct FdRelativePath {
    int descriptor = -1;
    std::filesystem::path relative;
};

std::optional<FdRelativePath> parse_fd_backed_path(
    const std::filesystem::path& path) {
    const std::string value = copperfin::platform::path_to_utf8_string(path);
    std::string_view suffix;
    if (value.rfind("/proc/self/fd/", 0U) == 0U) {
        suffix = std::string_view(value).substr(14U);
    } else if (value.rfind("/dev/fd/", 0U) == 0U) {
        suffix = std::string_view(value).substr(8U);
    } else {
        return std::nullopt;
    }
    if (suffix.empty() || suffix.front() < '0' || suffix.front() > '9') {
        return std::nullopt;
    }
    int descriptor = 0;
    std::size_t digit_count = 0U;
    while (digit_count < suffix.size() &&
           suffix[digit_count] >= '0' && suffix[digit_count] <= '9') {
        const int digit = suffix[digit_count] - '0';
        if (descriptor >
            (std::numeric_limits<int>::max() - digit) / 10) {
            return std::nullopt;
        }
        descriptor = descriptor * 10 + digit;
        ++digit_count;
    }
    if (digit_count == 0U ||
        (digit_count < suffix.size() && suffix[digit_count] != '/')) {
        return std::nullopt;
    }
    const std::string relative_text = digit_count == suffix.size()
        ? std::string()
        : std::string(suffix.substr(digit_count + 1U));
    return FdRelativePath{
        .descriptor = descriptor,
        .relative = relative_text.empty()
            ? std::filesystem::path{}
            : copperfin::platform::path_from_utf8_string(relative_text)};
}

bool read_descriptor(int descriptor, std::string& contents) {
    std::array<char, 64U * 1024U> buffer{};
    for (;;) {
        const ssize_t count = ::read(descriptor, buffer.data(), buffer.size());
        if (count < 0 && errno == EINTR) {
            continue;
        }
        if (count < 0) {
            return false;
        }
        if (count == 0) {
            return true;
        }
        contents.append(buffer.data(), static_cast<std::size_t>(count));
    }
}

bool read_file_bytes(
    const std::filesystem::path& source,
    std::string& contents) {
    contents.clear();
    if (const auto parsed = parse_fd_backed_path(source); parsed.has_value()) {
        int parent_descriptor = ::dup(parsed->descriptor);
        if (parent_descriptor < 0) {
            return false;
        }
        const std::filesystem::path relative = parsed->relative;
        if (relative.empty()) {
            const bool read_successfully = read_descriptor(parent_descriptor, contents);
            (void)::close(parent_descriptor);
            if (!read_successfully) {
                contents.clear();
            }
            return read_successfully;
        }
        for (const auto& component : relative.parent_path()) {
            if (component == ".") {
                continue;
            }
            if (component == "..") {
                (void)::close(parent_descriptor);
                return false;
            }
            const std::string name =
                copperfin::platform::path_to_utf8_string(component);
            const int child_descriptor = ::openat(
                parent_descriptor,
                name.c_str(),
                O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
            if (child_descriptor < 0) {
                (void)::close(parent_descriptor);
                return false;
            }
            (void)::close(parent_descriptor);
            parent_descriptor = child_descriptor;
        }
        const std::string file_name =
            copperfin::platform::path_to_utf8_string(relative.filename());
        const int file_descriptor = ::openat(
            parent_descriptor,
            file_name.c_str(),
            O_RDONLY | O_NOFOLLOW | O_CLOEXEC);
        (void)::close(parent_descriptor);
        if (file_descriptor < 0) {
            return false;
        }
        const bool read_successfully = read_descriptor(file_descriptor, contents);
        (void)::close(file_descriptor);
        if (!read_successfully) {
            contents.clear();
        }
        return read_successfully;
    }

    std::ifstream input(source, std::ios::binary);
    if (!input) {
        return false;
    }
    input.seekg(0, std::ios::end);
    const std::streamoff size = input.tellg();
    if (size < 0) {
        return false;
    }
    input.seekg(0, std::ios::beg);
    contents.resize(static_cast<std::size_t>(size));
    input.read(contents.data(), static_cast<std::streamsize>(contents.size()));
    return input.good() || input.eof();
}

bool open_destination_parent(
    const FdRelativePath& destination,
    int& parent_descriptor) {
    parent_descriptor = ::dup(destination.descriptor);
    if (parent_descriptor < 0) {
        return false;
    }
    for (const auto& component : destination.relative.parent_path()) {
        if (component == ".") {
            continue;
        }
        if (component == "..") {
            (void)::close(parent_descriptor);
            parent_descriptor = -1;
            return false;
        }
        const std::string name =
            copperfin::platform::path_to_utf8_string(component);
        int child_descriptor = ::openat(
            parent_descriptor,
            name.c_str(),
            O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
        if (child_descriptor < 0 && errno == ENOENT &&
            ::mkdirat(parent_descriptor, name.c_str(), 0700) == 0) {
            child_descriptor = ::openat(
                parent_descriptor,
                name.c_str(),
                O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
        }
        if (child_descriptor < 0) {
            (void)::close(parent_descriptor);
            parent_descriptor = -1;
            return false;
        }
        (void)::close(parent_descriptor);
        parent_descriptor = child_descriptor;
    }
    return true;
}

std::string temporary_name() {
    return ".copperfin-copy.tmp." +
        std::to_string(static_cast<unsigned long long>(
            std::chrono::steady_clock::now().time_since_epoch().count())) +
        "." + std::to_string(static_cast<unsigned long long>(::getpid()));
}

#if defined(COPPERFIN_ENABLE_RUNTIME_PIPELINE_TEST_HOOKS)
void trace_fd_copy_failure(
    const std::string_view stage,
    const std::filesystem::path& destination,
    const int saved_errno) {
    std::cerr << "RUNTIME_PIPELINE_FD_COPY_FAILURE stage=" << stage
              << " errno=" << saved_errno
              << " path=" << copperfin::platform::path_to_utf8_string(destination)
              << "\n";
}
#else
void trace_fd_copy_failure(
    const std::string_view,
    const std::filesystem::path&,
    const int) {
}
#endif

}  // namespace

bool is_fd_backed_runtime_path(const std::filesystem::path& path) {
    return parse_fd_backed_path(path).has_value();
}

bool try_copy_file_if_exists_fd_backed(
    const std::filesystem::path& source,
    const std::filesystem::path& destination,
    bool& handled,
    std::string& error) {
    handled = false;
    const auto parsed_destination = parse_fd_backed_path(destination);
    if (!parsed_destination.has_value()) {
        return true;
    }
    handled = true;

    std::string contents;
    if (!read_file_bytes(source, contents)) {
        trace_fd_copy_failure("source-read", destination, errno);
        error = runtime_text(
            "Runtime.Package.Error.CopyFileFailed",
            {{"path", copperfin::platform::path_to_utf8_string(destination)}});
        return false;
    }

    int parent_descriptor = -1;
    if (!open_destination_parent(*parsed_destination, parent_descriptor)) {
        trace_fd_copy_failure("destination-parent", destination, errno);
        error = runtime_text(
            "Runtime.Package.Error.CreateDirectoryFailed",
            {{"path", copperfin::platform::path_to_utf8_string(destination.parent_path())}});
        return false;
    }
    const std::string file_name =
        copperfin::platform::path_to_utf8_string(parsed_destination->relative.filename());
    struct stat existing{};
    if (::fstatat(
            parent_descriptor,
            file_name.c_str(),
            &existing,
            AT_SYMLINK_NOFOLLOW) == 0 &&
        (!S_ISREG(existing.st_mode) || existing.st_nlink != 1)) {
        trace_fd_copy_failure("existing-destination", destination, errno);
        (void)::close(parent_descriptor);
        error = runtime_text(
            "Runtime.Package.Error.CopyFileFailed",
            {{"path", copperfin::platform::path_to_utf8_string(destination)}});
        return false;
    }

    const std::string temporary = temporary_name();
    const int temporary_descriptor = ::openat(
        parent_descriptor,
        temporary.c_str(),
        O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC,
        0600);
    if (temporary_descriptor < 0) {
        trace_fd_copy_failure("temporary-open", destination, errno);
        (void)::close(parent_descriptor);
        error = runtime_text(
            "Runtime.Package.Error.CopyFileFailed",
            {{"path", copperfin::platform::path_to_utf8_string(destination)}});
        return false;
    }
    struct stat source_status{};
    const bool source_status_available = ::stat(source.c_str(), &source_status) == 0;
    const mode_t executable_bits = source_status_available
        ? source_status.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)
        : 0;
    if (executable_bits != 0 &&
        ::fchmod(
            temporary_descriptor,
            S_IRUSR | S_IWUSR | executable_bits) != 0) {
        const int saved_errno = errno;
        trace_fd_copy_failure("temporary-permissions", destination, saved_errno);
        (void)::close(temporary_descriptor);
        (void)::unlinkat(parent_descriptor, temporary.c_str(), 0);
        (void)::close(parent_descriptor);
        error = runtime_text(
            "Runtime.Package.Error.CopyFileFailed",
            {{"path", copperfin::platform::path_to_utf8_string(destination)}});
        return false;
    }
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
            const int saved_errno = count < 0 ? errno : 0;
            trace_fd_copy_failure("temporary-write", destination, saved_errno);
            (void)::close(temporary_descriptor);
            (void)::unlinkat(parent_descriptor, temporary.c_str(), 0);
            (void)::close(parent_descriptor);
            error = runtime_text(
                "Runtime.Package.Error.CopyFileFailed",
                {{"path", copperfin::platform::path_to_utf8_string(destination)}});
            return false;
        }
        offset += static_cast<std::size_t>(count);
    }
    if (::fsync(temporary_descriptor) != 0) {
        const int saved_errno = errno;
        trace_fd_copy_failure("temporary-fsync", destination, saved_errno);
        (void)::close(temporary_descriptor);
        (void)::unlinkat(parent_descriptor, temporary.c_str(), 0);
        (void)::close(parent_descriptor);
        error = runtime_text(
            "Runtime.Package.Error.CopyFileFailed",
            {{"path", copperfin::platform::path_to_utf8_string(destination)}});
        return false;
    }
    if (::close(temporary_descriptor) != 0) {
        const int saved_errno = errno;
        trace_fd_copy_failure("temporary-close", destination, saved_errno);
        (void)::unlinkat(parent_descriptor, temporary.c_str(), 0);
        (void)::close(parent_descriptor);
        error = runtime_text(
            "Runtime.Package.Error.CopyFileFailed",
            {{"path", copperfin::platform::path_to_utf8_string(destination)}});
        return false;
    }
    if (::renameat(
            parent_descriptor,
            temporary.c_str(),
            parent_descriptor,
            file_name.c_str()) != 0) {
        const int saved_errno = errno;
        trace_fd_copy_failure("destination-rename", destination, saved_errno);
        (void)::unlinkat(parent_descriptor, temporary.c_str(), 0);
        (void)::close(parent_descriptor);
        error = runtime_text(
            "Runtime.Package.Error.CopyFileFailed",
            {{"path", copperfin::platform::path_to_utf8_string(destination)}});
        return false;
    }
    struct stat copied{};
    const bool valid = ::fstatat(
        parent_descriptor,
        file_name.c_str(),
        &copied,
        AT_SYMLINK_NOFOLLOW) == 0 &&
        S_ISREG(copied.st_mode) && copied.st_nlink == 1;
    (void)::fsync(parent_descriptor);
    (void)::close(parent_descriptor);
    if (!valid) {
        trace_fd_copy_failure("destination-identity", destination, errno);
        error = runtime_text(
            "Runtime.Package.Error.CopyFileFailed",
            {{"path", copperfin::platform::path_to_utf8_string(destination)}});
        return false;
    }
    return true;
}

bool try_read_file_fd_backed(
    const std::filesystem::path& source,
    bool& handled,
    std::string& contents) {
    handled = false;
    if (!parse_fd_backed_path(source).has_value()) {
        contents.clear();
        return true;
    }
    handled = true;
    return read_file_bytes(source, contents);
}

bool collect_fd_regular_files_at(
    const int directory_descriptor,
    const std::filesystem::path& prefix,
    std::vector<std::filesystem::path>& relative_files) {
    const int duplicate = ::dup(directory_descriptor);
    if (duplicate < 0) {
        return false;
    }
    DIR* directory = ::fdopendir(duplicate);
    if (directory == nullptr) {
        (void)::close(duplicate);
        return false;
    }

    bool success = true;
    for (;;) {
        errno = 0;
        const dirent* entry = ::readdir(directory);
        if (entry == nullptr) {
            if (errno != 0) {
                success = false;
            }
            break;
        }
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
            success = false;
            break;
        }
        const std::filesystem::path child =
            prefix / copperfin::platform::path_from_utf8_string(name);
        if (S_ISREG(information.st_mode)) {
            relative_files.push_back(child);
            continue;
        }
        if (!S_ISDIR(information.st_mode)) {
            continue;
        }

        const int child_descriptor = ::openat(
            directory_descriptor,
            name.c_str(),
            O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
        if (child_descriptor < 0 ||
            !collect_fd_regular_files_at(child_descriptor, child, relative_files)) {
            if (child_descriptor >= 0) {
                (void)::close(child_descriptor);
            }
            success = false;
            break;
        }
        (void)::close(child_descriptor);
    }
    (void)::closedir(directory);
    return success;
}

bool try_collect_fd_backed_regular_files(
    const std::filesystem::path& root,
    bool& handled,
    std::vector<std::filesystem::path>& relative_files) {
    handled = false;
    relative_files.clear();
    const auto parsed = parse_fd_backed_path(root);
    if (!parsed.has_value()) {
        return true;
    }
    handled = true;

    int directory_descriptor = ::dup(parsed->descriptor);
    if (directory_descriptor < 0) {
        return false;
    }
    for (const auto& component : parsed->relative) {
        if (component == ".") {
            continue;
        }
        if (component == "..") {
            (void)::close(directory_descriptor);
            return false;
        }
        const std::string name =
            copperfin::platform::path_to_utf8_string(component);
        const int child_descriptor = ::openat(
            directory_descriptor,
            name.c_str(),
            O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
        if (child_descriptor < 0) {
            (void)::close(directory_descriptor);
            return false;
        }
        (void)::close(directory_descriptor);
        directory_descriptor = child_descriptor;
    }

    const bool success = collect_fd_regular_files_at(
        directory_descriptor,
        std::filesystem::path{},
        relative_files);
    (void)::close(directory_descriptor);
    return success;
}

bool try_write_text_file_fd_backed(
    const std::filesystem::path& destination,
    const std::string& contents,
    bool& handled,
    std::string& error) {
    handled = false;
    const auto parsed_destination = parse_fd_backed_path(destination);
    if (!parsed_destination.has_value()) {
        return true;
    }
    handled = true;
    int parent_descriptor = -1;
    if (!open_destination_parent(*parsed_destination, parent_descriptor)) {
        error = runtime_text(
            "Runtime.Package.Error.CreateDirectoryFailed",
            {{"path", copperfin::platform::path_to_utf8_string(destination.parent_path())}});
        return false;
    }
    const std::string file_name =
        copperfin::platform::path_to_utf8_string(parsed_destination->relative.filename());
    const std::string temporary = temporary_name();
    const int temporary_descriptor = ::openat(
        parent_descriptor,
        temporary.c_str(),
        O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC,
        0600);
    if (temporary_descriptor < 0) {
        (void)::close(parent_descriptor);
        error = runtime_text(
            "Runtime.Package.Error.CreateFileFailed",
            {{"path", copperfin::platform::path_to_utf8_string(destination)}});
        return false;
    }
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
            (void)::close(temporary_descriptor);
            (void)::unlinkat(parent_descriptor, temporary.c_str(), 0);
            (void)::close(parent_descriptor);
            error = runtime_text(
                "Runtime.Package.Error.WriteFileFailed",
                {{"path", copperfin::platform::path_to_utf8_string(destination)}});
            return false;
        }
        offset += static_cast<std::size_t>(count);
    }
    if (::fsync(temporary_descriptor) != 0 ||
        ::close(temporary_descriptor) != 0 ||
        ::renameat(
            parent_descriptor,
            temporary.c_str(),
            parent_descriptor,
            file_name.c_str()) != 0) {
        (void)::unlinkat(parent_descriptor, temporary.c_str(), 0);
        (void)::close(parent_descriptor);
        error = runtime_text(
            "Runtime.Package.Error.WriteFileFailed",
            {{"path", copperfin::platform::path_to_utf8_string(destination)}});
        return false;
    }
    (void)::fsync(parent_descriptor);
    (void)::close(parent_descriptor);
    return true;
}

}  // namespace copperfin::runtime::runtime_pipeline_detail

#endif
