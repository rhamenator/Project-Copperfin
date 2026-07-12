// Copyright (c) 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

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
