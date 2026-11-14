// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include <chrono>
#include <filesystem>

namespace copperfin::platform
{
    inline std::chrono::system_clock::time_point system_time_from_file_time(
        const std::filesystem::file_time_type &file_time)
    {
#if defined(_MSVC_STL_VERSION)
        // MSVC's file clock is Windows FILETIME: 100 ns ticks since
        // 1601-01-01. Its current C++20 library does not expose to_sys().
        constexpr auto windows_to_unix_epoch = std::chrono::seconds{11644473600LL};
        constexpr auto epoch_file_duration =
            std::chrono::duration_cast<std::filesystem::file_time_type::duration>(windows_to_unix_epoch);
        static_assert(epoch_file_duration.count() == 116444736000000000LL,
                      "MSVC file clock must retain the documented FILETIME Unix-epoch tick value");
        const auto unix_duration = file_time.time_since_epoch() - windows_to_unix_epoch;
        return std::chrono::system_clock::time_point{
            std::chrono::duration_cast<std::chrono::system_clock::duration>(unix_duration)};
#else
        return std::chrono::time_point_cast<std::chrono::system_clock::duration>(
            std::filesystem::file_time_type::clock::to_sys(file_time));
#endif
    }

    inline std::filesystem::file_time_type file_time_from_system_time(
        const std::chrono::system_clock::time_point &system_time)
    {
#if defined(_MSVC_STL_VERSION)
        constexpr auto windows_to_unix_epoch = std::chrono::seconds{11644473600LL};
        return std::filesystem::file_time_type{
            std::chrono::duration_cast<std::filesystem::file_time_type::duration>(
                system_time.time_since_epoch() + windows_to_unix_epoch)};
#else
        return std::filesystem::file_time_type::clock::from_sys(system_time);
#endif
    }
}
