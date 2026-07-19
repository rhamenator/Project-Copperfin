// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "runtime_pipeline_support.h"

#if !defined(_WIN32)

#include <array>
#include <cerrno>
#include <chrono>
#include <fcntl.h>
#include <fstream>
#include <limits>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

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

}  // namespace

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
        error = runtime_text(
            "Runtime.Package.Error.CopyFileFailed",
            {{"path", copperfin::platform::path_to_utf8_string(destination)}});
        return false;
    }

    int parent_descriptor = -1;
    if (!open_destination_parent(*parsed_destination, parent_descriptor)) {
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
        error = runtime_text(
            "Runtime.Package.Error.CopyFileFailed",
            {{"path", copperfin::platform::path_to_utf8_string(destination)}});
        return false;
    }
    return true;
}

}  // namespace copperfin::runtime::runtime_pipeline_detail

#endif
