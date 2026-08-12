// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/disk_space.h"

#include <cwchar>
#include <string>
#include <system_error>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#else
#include <sys/statvfs.h>
#endif

namespace copperfin::platform {

std::optional<std::uintmax_t> available_disk_bytes(
    const std::filesystem::path& path) {
    std::error_code error;
    const std::filesystem::space_info info = std::filesystem::space(path, error);
    if (error) {
        return std::nullopt;
    }
    return info.available;
}

std::optional<std::uintmax_t> disk_allocation_unit_bytes(
    const std::filesystem::path& path) {
#if defined(_WIN32)
    if (::GetFileAttributesW(path.c_str()) == INVALID_FILE_ATTRIBUTES) {
        return std::nullopt;
    }

    // GetDiskFreeSpaceW requires the owning volume root rather than an
    // arbitrary file or directory path.
    std::wstring volume_root(32768U, L'\0');
    if (!::GetVolumePathNameW(
            path.c_str(),
            volume_root.data(),
            static_cast<DWORD>(volume_root.size()))) {
        return std::nullopt;
    }
    volume_root.resize(std::wcslen(volume_root.c_str()));

    DWORD sectors_per_cluster = 0U;
    DWORD bytes_per_sector = 0U;
    if (!::GetDiskFreeSpaceW(
            volume_root.c_str(),
            &sectors_per_cluster,
            &bytes_per_sector,
            nullptr,
            nullptr) ||
        sectors_per_cluster == 0U || bytes_per_sector == 0U) {
        return std::nullopt;
    }
    return static_cast<std::uintmax_t>(sectors_per_cluster) *
           static_cast<std::uintmax_t>(bytes_per_sector);
#else
    struct statvfs info {};
    if (::statvfs(path.c_str(), &info) != 0 || info.f_frsize == 0U) {
        return std::nullopt;
    }
    return static_cast<std::uintmax_t>(info.f_frsize);
#endif
}

}  // namespace copperfin::platform
