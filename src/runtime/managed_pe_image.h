// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#if defined(_WIN32)

#include <filesystem>

namespace copperfin::runtime
{
    enum class PortableExecutableKind
    {
        unreadable,
        invalid,
        native,
        managed,
    };

    [[nodiscard]] PortableExecutableKind inspect_portable_executable(
        const std::filesystem::path &path) noexcept;
}

#endif
