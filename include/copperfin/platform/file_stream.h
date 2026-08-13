// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <string_view>

namespace copperfin::platform {

// Opens a standard C stream for a native filesystem path. The caller owns the
// returned stream and the interpretation of its mode.
std::FILE* open_file_stream(
    const std::filesystem::path& path,
    std::string_view mode);

// Resizes the file underlying an open standard C stream. Returns zero on
// success and nonzero on failure while preserving errno for caller policy.
int resize_file_stream(std::FILE* stream, std::uint64_t size);

}  // namespace copperfin::platform
