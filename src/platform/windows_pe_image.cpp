// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/windows_pe_image.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <limits>
#include <type_traits>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace copperfin::platform {
namespace {

constexpr std::uint16_t dos_signature = 0x5a4dU;
constexpr std::uint32_t pe_signature = 0x00004550U;
constexpr std::uint16_t pe32_magic = 0x010bU;
constexpr std::uint16_t pe32_plus_magic = 0x020bU;
constexpr std::uint16_t machine_x86 = 0x014cU;
constexpr std::uint16_t machine_x64 = 0x8664U;
constexpr std::uint16_t machine_arm64 = 0xaa64U;
constexpr std::uint16_t executable_image_flag = 0x0002U;
constexpr std::uint16_t system_image_flag = 0x1000U;
constexpr std::uint16_t dynamic_library_flag = 0x2000U;
constexpr std::uint16_t subsystem_windows_gui = 2U;
constexpr std::uint16_t subsystem_windows_console = 3U;
constexpr std::uint32_t section_executable_flag = 0x20000000U;
constexpr std::size_t maximum_section_count = 96U;
constexpr std::size_t coff_header_size = 20U;
constexpr std::size_t section_header_size = 40U;
constexpr std::size_t clr_directory_index = 14U;

class FileReader {
public:
    explicit FileReader(const std::filesystem::path& path) {
#if defined(_WIN32)
        handle_ = ::CreateFileW(
            path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_DELETE,
            nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (handle_ == INVALID_HANDLE_VALUE) {
            return;
        }
        LARGE_INTEGER size{};
        if (::GetFileSizeEx(handle_, &size) == 0 || size.QuadPart < 0) {
            ::CloseHandle(handle_);
            handle_ = INVALID_HANDLE_VALUE;
            return;
        }
        size_ = static_cast<std::uint64_t>(size.QuadPart);
#else
        stream_.open(path, std::ios::binary);
        if (!stream_) {
            return;
        }
        stream_.seekg(0, std::ios::end);
        const auto end = stream_.tellg();
        if (end < 0) {
            stream_.close();
            return;
        }
        size_ = static_cast<std::uint64_t>(end);
        stream_.seekg(0, std::ios::beg);
#endif
        open_ = true;
    }

    ~FileReader() {
#if defined(_WIN32)
        if (handle_ != INVALID_HANDLE_VALUE) {
            ::CloseHandle(handle_);
        }
#endif
    }

    FileReader(const FileReader&) = delete;
    FileReader& operator=(const FileReader&) = delete;

    [[nodiscard]] bool open() const noexcept { return open_; }
    [[nodiscard]] std::uint64_t size() const noexcept { return size_; }

    [[nodiscard]] bool read(
        const std::uint64_t offset,
        void* output,
        const std::size_t length) noexcept {
        if (!open_ || offset > size_ || length > size_ - offset) {
            return false;
        }
#if defined(_WIN32)
        if (length > static_cast<std::size_t>((std::numeric_limits<DWORD>::max)())) {
            return false;
        }
        LARGE_INTEGER position{};
        position.QuadPart = static_cast<LONGLONG>(offset);
        if (::SetFilePointerEx(handle_, position, nullptr, FILE_BEGIN) == 0) {
            return false;
        }
        DWORD read = 0U;
        return ::ReadFile(
                   handle_, output, static_cast<DWORD>(length), &read, nullptr) != 0 &&
            static_cast<std::size_t>(read) == length;
#else
        if (offset > static_cast<std::uint64_t>(
                         (std::numeric_limits<std::streamoff>::max)()) ||
            length > static_cast<std::size_t>(
                         (std::numeric_limits<std::streamsize>::max)())) {
            return false;
        }
        stream_.clear();
        stream_.seekg(static_cast<std::streamoff>(offset), std::ios::beg);
        if (!stream_) {
            return false;
        }
        stream_.read(static_cast<char*>(output), static_cast<std::streamsize>(length));
        return stream_.gcount() == static_cast<std::streamsize>(length);
#endif
    }

private:
    bool open_ = false;
    std::uint64_t size_ = 0U;
#if defined(_WIN32)
    HANDLE handle_ = INVALID_HANDLE_VALUE;
#else
    std::ifstream stream_;
#endif
};

template <typename Integer>
bool read_little_endian(
    FileReader& reader,
    const std::uint64_t offset,
    Integer& value) noexcept {
    static_assert(std::is_unsigned_v<Integer>);
    std::array<unsigned char, sizeof(Integer)> bytes{};
    if (!reader.read(offset, bytes.data(), bytes.size())) {
        return false;
    }
    value = 0U;
    for (std::size_t index = 0U; index < bytes.size(); ++index) {
        value |= static_cast<Integer>(bytes[index]) << (index * 8U);
    }
    return true;
}

bool checked_add(
    const std::uint64_t left,
    const std::uint64_t right,
    std::uint64_t& result) noexcept {
    if (right > (std::numeric_limits<std::uint64_t>::max)() - left) {
        return false;
    }
    result = left + right;
    return true;
}

WindowsPeMachine decode_machine(const std::uint16_t value) noexcept {
    switch (value) {
        case machine_x86:
            return WindowsPeMachine::x86;
        case machine_x64:
            return WindowsPeMachine::x64;
        case machine_arm64:
            return WindowsPeMachine::arm64;
        default:
            return WindowsPeMachine::unknown;
    }
}

WindowsPeSubsystem decode_subsystem(const std::uint16_t value) noexcept {
    switch (value) {
        case subsystem_windows_gui:
            return WindowsPeSubsystem::windows_gui;
        case subsystem_windows_console:
            return WindowsPeSubsystem::windows_console;
        default:
            return WindowsPeSubsystem::unsupported;
    }
}

bool section_table_is_valid(
    FileReader& reader,
    const std::uint64_t section_table_offset,
    const std::uint16_t section_count,
    const std::uint32_t entry_point,
    bool& entry_point_is_executable) noexcept {
    entry_point_is_executable = false;
    for (std::size_t index = 0U; index < section_count; ++index) {
        std::uint64_t section_offset = 0U;
        if (!checked_add(
                section_table_offset,
                index * section_header_size,
                section_offset)) {
            return false;
        }
        std::uint32_t virtual_size = 0U;
        std::uint32_t virtual_address = 0U;
        std::uint32_t raw_size = 0U;
        std::uint32_t raw_offset = 0U;
        std::uint32_t characteristics = 0U;
        if (!read_little_endian(reader, section_offset + 8U, virtual_size) ||
            !read_little_endian(reader, section_offset + 12U, virtual_address) ||
            !read_little_endian(reader, section_offset + 16U, raw_size) ||
            !read_little_endian(reader, section_offset + 20U, raw_offset) ||
            !read_little_endian(reader, section_offset + 36U, characteristics)) {
            return false;
        }
        if (raw_size != 0U &&
            (raw_offset > reader.size() || raw_size > reader.size() - raw_offset)) {
            return false;
        }
        const std::uint64_t span = virtual_size != 0U ? virtual_size : raw_size;
        const std::uint64_t section_end =
            static_cast<std::uint64_t>(virtual_address) + span;
        if (entry_point >= virtual_address && entry_point < section_end &&
            (characteristics & section_executable_flag) != 0U) {
            entry_point_is_executable = true;
        }
    }
    return true;
}

}  // namespace

namespace {

WindowsPeImageInspection inspect_windows_pe_image_impl(
    const std::filesystem::path& path) {
    FileReader reader(path);
    if (!reader.open()) {
        return {};
    }
    WindowsPeImageInspection result;
    result.status = WindowsPeImageStatus::invalid;

    std::uint16_t mz = 0U;
    std::uint32_t nt_offset32 = 0U;
    if (!read_little_endian(reader, 0U, mz) || mz != dos_signature ||
        !read_little_endian(reader, 0x3cU, nt_offset32)) {
        return result;
    }
    const std::uint64_t nt_offset = nt_offset32;
    std::uint32_t signature = 0U;
    std::uint64_t coff_offset = 0U;
    if (!read_little_endian(reader, nt_offset, signature) ||
        signature != pe_signature ||
        !checked_add(nt_offset, sizeof(signature), coff_offset)) {
        return result;
    }

    std::uint16_t raw_machine = 0U;
    std::uint16_t section_count = 0U;
    std::uint16_t optional_size = 0U;
    std::uint16_t characteristics = 0U;
    if (!read_little_endian(reader, coff_offset, raw_machine) ||
        !read_little_endian(reader, coff_offset + 2U, section_count) ||
        !read_little_endian(reader, coff_offset + 16U, optional_size) ||
        !read_little_endian(reader, coff_offset + 18U, characteristics) ||
        section_count == 0U || section_count > maximum_section_count) {
        return result;
    }

    std::uint64_t optional_offset = 0U;
    std::uint64_t section_table_offset = 0U;
    if (!checked_add(coff_offset, coff_header_size, optional_offset) ||
        !checked_add(optional_offset, optional_size, section_table_offset) ||
        section_table_offset > reader.size() ||
        section_count > (reader.size() - section_table_offset) / section_header_size) {
        return result;
    }

    std::uint16_t magic = 0U;
    std::uint32_t entry_point = 0U;
    std::uint16_t raw_subsystem = 0U;
    if (!read_little_endian(reader, optional_offset, magic) ||
        !read_little_endian(reader, optional_offset + 16U, entry_point) ||
        !read_little_endian(reader, optional_offset + 68U, raw_subsystem)) {
        return result;
    }
    const bool pe32_plus = magic == pe32_plus_magic;
    if (!pe32_plus && magic != pe32_magic) {
        return result;
    }
    result.machine = decode_machine(raw_machine);
    result.subsystem = decode_subsystem(raw_subsystem);
    result.pe32_plus = pe32_plus;
    if (result.machine == WindowsPeMachine::unknown ||
        (result.machine == WindowsPeMachine::x86 && pe32_plus) ||
        (result.machine != WindowsPeMachine::x86 && !pe32_plus)) {
        return result;
    }

    const std::size_t directory_count_offset = pe32_plus ? 108U : 92U;
    const std::size_t directory_table_offset = pe32_plus ? 112U : 96U;
    if (optional_size < directory_count_offset + sizeof(std::uint32_t)) {
        return result;
    }
    std::uint32_t directory_count = 0U;
    if (!read_little_endian(
            reader, optional_offset + directory_count_offset, directory_count)) {
        return result;
    }
    result.clr_directory_slot_present = directory_count > clr_directory_index &&
        optional_size >= directory_table_offset +
            (clr_directory_index + 1U) * 8U;
    if (result.clr_directory_slot_present) {
        std::uint32_t clr_rva = 0U;
        std::uint32_t clr_size = 0U;
        const std::uint64_t clr_offset = optional_offset + directory_table_offset +
            clr_directory_index * 8U;
        if (!read_little_endian(reader, clr_offset, clr_rva) ||
            !read_little_endian(reader, clr_offset + 4U, clr_size)) {
            return result;
        }
        result.managed = clr_rva != 0U && clr_size != 0U;
    }

    bool entry_point_is_executable = false;
    if (!section_table_is_valid(
            reader,
            section_table_offset,
            section_count,
            entry_point,
            entry_point_is_executable)) {
        return result;
    }
    if ((characteristics & dynamic_library_flag) != 0U) {
        result.status = WindowsPeImageStatus::dynamic_library;
        return result;
    }
    if ((characteristics & executable_image_flag) == 0U ||
        (characteristics & system_image_flag) != 0U ||
        result.subsystem == WindowsPeSubsystem::unsupported ||
        entry_point == 0U || !entry_point_is_executable) {
        return result;
    }
    result.status = WindowsPeImageStatus::executable;
    return result;
}

}  // namespace

WindowsPeImageInspection inspect_windows_pe_image(
    const std::filesystem::path& path) noexcept {
    try {
        return inspect_windows_pe_image_impl(path);
    } catch (...) {
        return {};
    }
}

WindowsPeMachine native_windows_pe_host_machine() noexcept {
#if defined(_WIN32)
    SYSTEM_INFO info{};
    ::GetNativeSystemInfo(&info);
    switch (info.wProcessorArchitecture) {
        case PROCESSOR_ARCHITECTURE_INTEL:
            return WindowsPeMachine::x86;
        case PROCESSOR_ARCHITECTURE_AMD64:
            return WindowsPeMachine::x64;
        case PROCESSOR_ARCHITECTURE_ARM64:
            return WindowsPeMachine::arm64;
        default:
            return WindowsPeMachine::unknown;
    }
#else
    return WindowsPeMachine::unknown;
#endif
}

bool windows_pe_image_is_launch_compatible(
    const WindowsPeImageInspection& image,
    const WindowsPeMachine host_machine) noexcept {
    if (image.status != WindowsPeImageStatus::executable ||
        image.subsystem == WindowsPeSubsystem::unsupported) {
        return false;
    }
    switch (host_machine) {
        case WindowsPeMachine::x86:
            return image.machine == WindowsPeMachine::x86;
        case WindowsPeMachine::x64:
            return image.machine == WindowsPeMachine::x86 ||
                image.machine == WindowsPeMachine::x64;
        case WindowsPeMachine::arm64:
            return image.machine == WindowsPeMachine::arm64;
        case WindowsPeMachine::unknown:
            return false;
    }
    return false;
}

}  // namespace copperfin::platform
