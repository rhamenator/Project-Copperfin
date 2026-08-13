// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#define _CRT_SECURE_NO_WARNINGS
#include "copperfin/platform/file_stream.h"

#include <cerrno>
#include <limits>
#include <string>

#if defined(_WIN32)
#include <io.h>
#else
#include <unistd.h>
#endif

namespace copperfin::platform {

std::FILE* open_file_stream(
    const std::filesystem::path& path,
    const std::string_view mode) {
#if defined(_WIN32)
    std::wstring wide_mode;
    wide_mode.reserve(mode.size());
    for (const unsigned char ch : mode) {
        wide_mode.push_back(static_cast<wchar_t>(ch));
    }
    return ::_wfopen(path.c_str(), wide_mode.c_str());
#else
    const std::string native_mode(mode);
    return std::fopen(path.c_str(), native_mode.c_str());
#endif
}

int resize_file_stream(std::FILE* stream, const std::uint64_t size) {
    if (stream == nullptr) {
        errno = EBADF;
        return -1;
    }
#if defined(_WIN32)
    if (size > static_cast<std::uint64_t>((std::numeric_limits<std::int64_t>::max)())) {
        errno = EFBIG;
        return -1;
    }
    const int descriptor = ::_fileno(stream);
    if (descriptor < 0) {
        errno = EBADF;
        return -1;
    }
    return ::_chsize_s(descriptor, static_cast<std::int64_t>(size));
#else
    if (size > static_cast<std::uint64_t>((std::numeric_limits<off_t>::max)())) {
        errno = EFBIG;
        return -1;
    }
    const int descriptor = ::fileno(stream);
    if (descriptor < 0) {
        errno = EBADF;
        return -1;
    }
    return ::ftruncate(descriptor, static_cast<off_t>(size));
#endif
}

}  // namespace copperfin::platform
