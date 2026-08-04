// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "managed_pe_image.h"

#if defined(_WIN32)

#include <cstddef>
#include <cstdint>
#include <limits>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

namespace copperfin::runtime
{
    PortableExecutableKind inspect_portable_executable(const std::filesystem::path &path) noexcept
    {
        const HANDLE file = CreateFileW(
            path.c_str(),
            GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            nullptr,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            nullptr);
        if (file == INVALID_HANDLE_VALUE)
        {
            return PortableExecutableKind::unreadable;
        }

        LARGE_INTEGER file_size_value{};
        if (GetFileSizeEx(file, &file_size_value) == 0 || file_size_value.QuadPart < 0)
        {
            CloseHandle(file);
            return PortableExecutableKind::unreadable;
        }
        const std::uint64_t file_size = static_cast<std::uint64_t>(file_size_value.QuadPart);

        const auto read_at = [&](std::uint64_t offset, void *buffer, std::size_t length) -> bool
        {
            if (offset > file_size || length > file_size - offset ||
                length > static_cast<std::size_t>((std::numeric_limits<DWORD>::max)()))
            {
                return false;
            }
            LARGE_INTEGER position{};
            position.QuadPart = static_cast<LONGLONG>(offset);
            if (SetFilePointerEx(file, position, nullptr, FILE_BEGIN) == 0)
            {
                return false;
            }
            DWORD bytes_read = 0U;
            return ReadFile(
                       file,
                       buffer,
                       static_cast<DWORD>(length),
                       &bytes_read,
                       nullptr) != 0 &&
                   static_cast<std::size_t>(bytes_read) == length;
        };

        PortableExecutableKind result = PortableExecutableKind::invalid;
        do
        {
            IMAGE_DOS_HEADER dos_header{};
            if (!read_at(0U, &dos_header, sizeof(dos_header)) ||
                dos_header.e_magic != IMAGE_DOS_SIGNATURE || dos_header.e_lfanew < 0)
            {
                break;
            }

            const std::uint64_t nt_offset = static_cast<std::uint64_t>(dos_header.e_lfanew);
            DWORD signature = 0U;
            IMAGE_FILE_HEADER file_header{};
            if (!read_at(nt_offset, &signature, sizeof(signature)) ||
                signature != IMAGE_NT_SIGNATURE ||
                !read_at(nt_offset + sizeof(signature), &file_header, sizeof(file_header)))
            {
                break;
            }

            const std::uint64_t optional_offset =
                nt_offset + sizeof(signature) + sizeof(file_header);
            WORD optional_magic = 0U;
            if (!read_at(optional_offset, &optional_magic, sizeof(optional_magic)))
            {
                break;
            }

            IMAGE_DATA_DIRECTORY clr_directory{};
            if (optional_magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC)
            {
                constexpr std::size_t required_optional_size =
                    offsetof(IMAGE_OPTIONAL_HEADER32, DataDirectory) +
                    (IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR + 1U) * sizeof(IMAGE_DATA_DIRECTORY);
                if (static_cast<std::size_t>(file_header.SizeOfOptionalHeader) < required_optional_size)
                {
                    break;
                }
                IMAGE_OPTIONAL_HEADER32 optional_header{};
                if (!read_at(optional_offset, &optional_header, required_optional_size) ||
                    optional_header.NumberOfRvaAndSizes <= IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR)
                {
                    break;
                }
                clr_directory = optional_header.DataDirectory[IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR];
            }
            else if (optional_magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
            {
                constexpr std::size_t required_optional_size =
                    offsetof(IMAGE_OPTIONAL_HEADER64, DataDirectory) +
                    (IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR + 1U) * sizeof(IMAGE_DATA_DIRECTORY);
                if (static_cast<std::size_t>(file_header.SizeOfOptionalHeader) < required_optional_size)
                {
                    break;
                }
                IMAGE_OPTIONAL_HEADER64 optional_header{};
                if (!read_at(optional_offset, &optional_header, required_optional_size) ||
                    optional_header.NumberOfRvaAndSizes <= IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR)
                {
                    break;
                }
                clr_directory = optional_header.DataDirectory[IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR];
            }
            else
            {
                break;
            }

            result = clr_directory.VirtualAddress != 0U && clr_directory.Size != 0U
                         ? PortableExecutableKind::managed
                         : PortableExecutableKind::native;
        } while (false);

        CloseHandle(file);
        return result;
    }
}

#endif
