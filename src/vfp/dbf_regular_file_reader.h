// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include "../platform/scoped_resource.h"

#include <algorithm>
#include <array>
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <limits>
#include <vector>

#if !defined(_WIN32)
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace copperfin::vfp::detail {

struct RegularFileReadResult {
    bool ok = false;
    std::vector<std::uint8_t> bytes;
};

// Opens without waiting for a FIFO writer, admits the opened object only when
// it is a regular file, and reads from that same descriptor/handle. Symlinks
// retain their existing follow-to-target behavior; their resolved target must
// be regular. max_bytes permits header-only callers to keep their bounded read.
inline RegularFileReadResult read_regular_file(
    const std::filesystem::path& path,
    std::size_t max_bytes = std::numeric_limits<std::size_t>::max()) {
    RegularFileReadResult result;
    std::array<std::uint8_t, 64U * 1024U> buffer{};

#if defined(_WIN32)
    platform::ScopedHandle handle(::CreateFileW(
        path.c_str(), GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr));
    if (!handle.valid() || ::GetFileType(handle.get()) != FILE_TYPE_DISK) {
        return result;
    }
    BY_HANDLE_FILE_INFORMATION information{};
    if (::GetFileInformationByHandle(handle.get(), &information) == 0 ||
        (information.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0U) {
        return result;
    }
    const std::uint64_t file_size =
        (static_cast<std::uint64_t>(information.nFileSizeHigh) << 32U) |
        information.nFileSizeLow;
    if (file_size <= max_bytes && file_size <= result.bytes.max_size()) {
        result.bytes.reserve(static_cast<std::size_t>(file_size));
    }
    while (result.bytes.size() < max_bytes) {
        const std::size_t requested = std::min(
            buffer.size(), max_bytes - result.bytes.size());
        DWORD bytes_read = 0U;
        if (::ReadFile(
                handle.get(), buffer.data(), static_cast<DWORD>(requested),
                &bytes_read, nullptr) == 0) {
            return {};
        }
        if (bytes_read == 0U) {
            break;
        }
        result.bytes.insert(
            result.bytes.end(), buffer.begin(), buffer.begin() + bytes_read);
    }
#else
    platform::ScopedFd descriptor(::open(
        path.c_str(), O_RDONLY | O_CLOEXEC | O_NONBLOCK));
    if (!descriptor.valid()) {
        return result;
    }
    struct stat status {};
    if (::fstat(descriptor.get(), &status) != 0 || !S_ISREG(status.st_mode)) {
        return result;
    }
    const int descriptor_flags = ::fcntl(descriptor.get(), F_GETFL);
    if (descriptor_flags < 0 ||
        ::fcntl(descriptor.get(), F_SETFL, descriptor_flags & ~O_NONBLOCK) != 0) {
        return result;
    }
    if (status.st_size > 0 &&
        static_cast<std::uintmax_t>(status.st_size) <= max_bytes &&
        static_cast<std::uintmax_t>(status.st_size) <= result.bytes.max_size()) {
        result.bytes.reserve(static_cast<std::size_t>(status.st_size));
    }
    while (result.bytes.size() < max_bytes) {
        const std::size_t requested = std::min(
            buffer.size(), max_bytes - result.bytes.size());
        const ssize_t bytes_read = ::read(
            descriptor.get(), buffer.data(), requested);
        if (bytes_read > 0) {
            result.bytes.insert(
                result.bytes.end(), buffer.begin(),
                buffer.begin() + bytes_read);
            continue;
        }
        if (bytes_read == 0) {
            break;
        }
        if (errno == EINTR) {
            continue;
        }
        return {};
    }
#endif

    result.ok = true;
    return result;
}

}  // namespace copperfin::vfp::detail
