// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "native_declared_library.h"

#if defined(_WIN32)

#include "managed_pe_image.h"
#include "prg_engine_helpers.h"

#include <array>
#include <cstddef>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

namespace copperfin::runtime
{
    namespace
    {
        class ModuleOwner final
        {
        public:
            explicit ModuleOwner(HMODULE module = nullptr) noexcept : module_(module) {}

            ~ModuleOwner() noexcept
            {
                if (module_ != nullptr)
                {
                    FreeLibrary(module_);
                }
            }

            ModuleOwner(const ModuleOwner &) = delete;
            ModuleOwner &operator=(const ModuleOwner &) = delete;
            ModuleOwner(ModuleOwner &&) = delete;
            ModuleOwner &operator=(ModuleOwner &&) = delete;

            [[nodiscard]] HMODULE get() const noexcept
            {
                return module_;
            }

            [[nodiscard]] HMODULE release() noexcept
            {
                const HMODULE module = module_;
                module_ = nullptr;
                return module;
            }

        private:
            HMODULE module_ = nullptr;
        };

        std::filesystem::path loaded_module_path(HMODULE module, const std::filesystem::path &fallback_path)
        {
            std::wstring module_path(32768U, L'\0');
            const DWORD length = GetModuleFileNameW(
                module,
                module_path.data(),
                static_cast<DWORD>(module_path.size()));
            if (length > 0U && length < module_path.size())
            {
                module_path.resize(length);
                return std::filesystem::path(module_path);
            }
            return fallback_path;
        }

        std::string path_to_utf8_string(const std::filesystem::path &path)
        {
            const std::u8string encoded = path.u8string();
            return std::string(
                reinterpret_cast<const char *>(encoded.data()),
                encoded.size());
        }

        std::string system_error_message(DWORD error)
        {
            char message_buffer[256]{};
            (void)FormatMessageA(
                FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                nullptr,
                error,
                0,
                message_buffer,
                sizeof(message_buffer) - 1U,
                nullptr);
            return std::string(message_buffer);
        }

        FARPROC resolve_native_export(
            HMODULE module,
            const std::string &function_name,
            const std::string &parameter_types,
            bool allow_ansi_fallback,
            NativeDeclaredLibraryLoadResult &result)
        {
#if defined(_WIN64)
            (void)parameter_types;
#endif
            const std::array<std::string, 2U> export_names{function_name, function_name + "A"};
            const std::size_t export_name_count = allow_ansi_fallback ? export_names.size() : 1U;
            for (std::size_t export_index = 0U; export_index < export_name_count; ++export_index)
            {
                const std::string &export_name = export_names[export_index];
                FARPROC procedure = GetProcAddress(module, export_name.c_str());
                if (procedure != nullptr)
                {
                    result.native_cdecl = false;
                    result.resolved_function_name = export_name;
                    return procedure;
                }

                procedure = GetProcAddress(module, ("_" + export_name).c_str());
                if (procedure != nullptr)
                {
#if !defined(_WIN64)
                    result.native_cdecl = true;
#endif
                    result.resolved_function_name = export_name;
                    return procedure;
                }
#if !defined(_WIN64)
                const std::string stack_suffix = "@" + std::to_string(
                    declared_dll_x86_stdcall_stack_bytes(parameter_types));
                procedure = GetProcAddress(module, ("_" + export_name + stack_suffix).c_str());
                if (procedure == nullptr)
                {
                    procedure = GetProcAddress(module, (export_name + stack_suffix).c_str());
                }
                if (procedure != nullptr)
                {
                    result.native_cdecl = false;
                    result.resolved_function_name = export_name;
                    return procedure;
                }
#endif
            }
            return nullptr;
        }

        void retain_native_result(
            NativeDeclaredLibraryLoadResult &result,
            HMODULE module,
            FARPROC procedure,
            const std::filesystem::path &fallback_path)
        {
            result.kind = NativeDeclaredLibraryKind::native;
            result.module_handle = reinterpret_cast<std::uintptr_t>(module);
            result.function_address = reinterpret_cast<std::uintptr_t>(procedure);
            result.loaded_module_path = path_to_utf8_string(loaded_module_path(module, fallback_path));
        }
    }

    NativeDeclaredLibraryLoadResult load_native_declared_library(
        const std::filesystem::path &requested_path,
        bool use_win32api_modules,
        const std::string &function_name,
        const std::string &parameter_types)
    {
        NativeDeclaredLibraryLoadResult result;
        result.resolved_function_name = function_name;

        if (use_win32api_modules)
        {
            constexpr std::array<const wchar_t *, 5U> win32api_modules{
                L"Kernel32.dll",
                L"Gdi32.dll",
                L"User32.dll",
                L"Mpr.dll",
                L"Advapi32.dll",
            };
            std::wstring system_directory_buffer(32768U, L'\0');
            const UINT system_directory_length = GetSystemDirectoryW(
                system_directory_buffer.data(),
                static_cast<UINT>(system_directory_buffer.size()));
            if (system_directory_length > 0U &&
                system_directory_length < system_directory_buffer.size())
            {
                system_directory_buffer.resize(system_directory_length);
                const std::filesystem::path system_directory(system_directory_buffer);
                for (const wchar_t *module_name : win32api_modules)
                {
                    const std::filesystem::path module_path = system_directory / module_name;
                    ModuleOwner candidate(LoadLibraryW(module_path.c_str()));
                    if (candidate.get() == nullptr)
                    {
                        continue;
                    }
                    FARPROC procedure = resolve_native_export(
                        candidate.get(),
                        function_name,
                        parameter_types,
                        true,
                        result);
                    if (procedure != nullptr)
                    {
                        retain_native_result(result, candidate.get(), procedure, module_path);
                        (void)candidate.release();
                        return result;
                    }
                }
            }

            result.kind = NativeDeclaredLibraryKind::function_not_found;
            return result;
        }

        const std::wstring requested_wide_path = requested_path.wstring();
        ModuleOwner module(LoadLibraryW(requested_wide_path.c_str()));
        const DWORD load_error = module.get() == nullptr ? GetLastError() : ERROR_SUCCESS;

        std::filesystem::path inspection_path;
        if (module.get() != nullptr)
        {
            inspection_path = loaded_module_path(module.get(), requested_path);
        }
        else if (requested_path.is_absolute() || requested_path.has_parent_path())
        {
            inspection_path = requested_path;
        }
        else
        {
            std::wstring search_result(32768U, L'\0');
            const DWORD search_length = SearchPathW(
                nullptr,
                requested_wide_path.c_str(),
                nullptr,
                static_cast<DWORD>(search_result.size()),
                search_result.data(),
                nullptr);
            if (search_length > 0U && search_length < search_result.size())
            {
                search_result.resize(search_length);
                inspection_path = std::filesystem::path(search_result);
            }
        }

        const bool is_dotnet_assembly =
            !inspection_path.empty() &&
            inspect_portable_executable(inspection_path) == PortableExecutableKind::managed;
        FARPROC procedure = module.get() == nullptr
                                ? nullptr
                                : resolve_native_export(
                                      module.get(),
                                      function_name,
                                      parameter_types,
                                      true,
                                      result);
        if (procedure != nullptr)
        {
            retain_native_result(result, module.get(), procedure, requested_path);
            (void)module.release();
            return result;
        }

        if (is_dotnet_assembly)
        {
            result.kind = NativeDeclaredLibraryKind::managed;
            result.loaded_module_path = path_to_utf8_string(inspection_path);
            return result;
        }

        if (module.get() == nullptr)
        {
            result.kind = NativeDeclaredLibraryKind::cannot_load;
            result.system_error_message = system_error_message(load_error);
            return result;
        }

        result.kind = NativeDeclaredLibraryKind::function_not_found;
        return result;
    }

    void release_native_declared_library(std::uintptr_t module_handle) noexcept
    {
        if (module_handle != 0U)
        {
            FreeLibrary(reinterpret_cast<HMODULE>(module_handle));
        }
    }
}

#endif
