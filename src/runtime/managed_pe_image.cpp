// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "managed_pe_image.h"

#if defined(_WIN32)

#include "copperfin/platform/windows_pe_image.h"

namespace copperfin::runtime
{
    PortableExecutableKind inspect_portable_executable(const std::filesystem::path &path) noexcept
    {
        const auto inspection = copperfin::platform::inspect_windows_pe_image(
            path,
            copperfin::platform::WindowsPeReadSharing::allow_write_sharing);
        if (inspection.status == copperfin::platform::WindowsPeImageStatus::unreadable)
        {
            return PortableExecutableKind::unreadable;
        }
        if (inspection.status == copperfin::platform::WindowsPeImageStatus::invalid ||
            !inspection.clr_directory_slot_present)
        {
            return PortableExecutableKind::invalid;
        }
        return inspection.managed
            ? PortableExecutableKind::managed
            : PortableExecutableKind::native;
    }
}

#endif
