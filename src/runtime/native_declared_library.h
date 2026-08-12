// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

namespace copperfin::runtime
{
    enum class NativeDeclaredLibraryKind
    {
        native,
        managed,
        cannot_load,
        function_not_found,
    };

    struct NativeDeclaredLibraryLoadResult
    {
        NativeDeclaredLibraryKind kind = NativeDeclaredLibraryKind::cannot_load;
        std::uintptr_t module_handle = 0U;
        std::uintptr_t function_address = 0U;
        std::string loaded_module_path;
        std::string resolved_function_name;
        std::string system_error_message;
        bool native_cdecl = false;
    };

    [[nodiscard]] NativeDeclaredLibraryLoadResult load_native_declared_library(
        const std::filesystem::path &requested_path,
        bool use_win32api_modules,
        const std::string &function_name,
        const std::string &parameter_types);

    void release_native_declared_library(std::uintptr_t module_handle) noexcept;
}
