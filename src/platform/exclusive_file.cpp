// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/exclusive_file.h"

#include "copperfin/platform/path.h"

#include <algorithm>
#include <cstddef>
#include <limits>
#include <string>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>
#endif

namespace copperfin::platform {

bool write_new_durable_file(
    const std::filesystem::path& path,
    const std::string_view bytes) {
#if defined(_WIN32)
    const HANDLE handle = ::CreateFileW(
        path.c_str(),
        GENERIC_WRITE,
        0U,
        nullptr,
        CREATE_NEW,
        FILE_ATTRIBUTE_TEMPORARY,
        nullptr);
    if (handle == INVALID_HANDLE_VALUE) {
        return false;
    }

    bool written = true;
    std::size_t offset = 0U;
    while (offset < bytes.size()) {
        const DWORD requested = static_cast<DWORD>(std::min<std::size_t>(
            bytes.size() - offset,
            static_cast<std::size_t>((std::numeric_limits<DWORD>::max)())));
        DWORD transferred = 0U;
        if (::WriteFile(
                handle,
                bytes.data() + offset,
                requested,
                &transferred,
                nullptr) == 0 ||
            transferred == 0U) {
            written = false;
            break;
        }
        offset += transferred;
    }
    written = written && ::FlushFileBuffers(handle) != 0;
    return ::CloseHandle(handle) != 0 && written;
#else
    const std::string native_path = path_to_utf8_string(path);
    const int descriptor = ::open(
        native_path.c_str(),
        O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC,
        0600);
    if (descriptor < 0) {
        return false;
    }

    bool written = true;
    std::size_t offset = 0U;
    while (offset < bytes.size()) {
        const std::size_t requested = std::min<std::size_t>(
            bytes.size() - offset,
            static_cast<std::size_t>((std::numeric_limits<ssize_t>::max)()));
        const ssize_t transferred = ::write(
            descriptor,
            bytes.data() + offset,
            requested);
        if (transferred < 0 && errno == EINTR) {
            continue;
        }
        if (transferred <= 0) {
            written = false;
            break;
        }
        offset += static_cast<std::size_t>(transferred);
    }
    written = written && ::fsync(descriptor) == 0;
    return ::close(descriptor) == 0 && written;
#endif
}

}  // namespace copperfin::platform
