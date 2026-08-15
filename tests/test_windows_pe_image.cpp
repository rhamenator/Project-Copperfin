// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/windows_pe_image.h"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace {

using copperfin::platform::WindowsPeImageInspection;
using copperfin::platform::WindowsPeImageStatus;
using copperfin::platform::WindowsPeMachine;
using copperfin::platform::WindowsPeSubsystem;
using copperfin::platform::inspect_windows_pe_image;
using copperfin::platform::windows_pe_image_is_launch_compatible;

int failures = 0;

void expect(const bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
}

template <typename Integer>
void write_le(
    std::vector<std::uint8_t>& bytes,
    const std::size_t offset,
    Integer value) {
    for (std::size_t index = 0U; index < sizeof(Integer); ++index) {
        bytes.at(offset + index) = static_cast<std::uint8_t>(value & 0xffU);
        value = static_cast<Integer>(value >> 8U);
    }
}

struct FixtureOptions {
    WindowsPeMachine machine = WindowsPeMachine::x64;
    bool pe32_plus = true;
    bool dynamic_library = false;
    bool executable_flag = true;
    bool system_image = false;
    std::uint16_t subsystem = 3U;
    bool managed = false;
    bool clr_slot = true;
    bool executable_entry_section = true;
    bool zero_entry_point = false;
};

std::uint16_t raw_machine(const WindowsPeMachine machine) {
    switch (machine) {
        case WindowsPeMachine::x86:
            return 0x014cU;
        case WindowsPeMachine::x64:
            return 0x8664U;
        case WindowsPeMachine::arm64:
            return 0xaa64U;
        case WindowsPeMachine::unknown:
            return 0U;
    }
    return 0U;
}

std::vector<std::uint8_t> make_fixture(const FixtureOptions& options = {}) {
    constexpr std::size_t nt_offset = 0x80U;
    constexpr std::size_t coff_offset = nt_offset + 4U;
    constexpr std::size_t optional_offset = coff_offset + 20U;
    const std::size_t optional_size = options.pe32_plus ? 240U : 224U;
    const std::size_t section_offset = optional_offset + optional_size;
    std::vector<std::uint8_t> bytes(0x400U, 0U);
    write_le<std::uint16_t>(bytes, 0U, 0x5a4dU);
    write_le<std::uint32_t>(bytes, 0x3cU, nt_offset);
    write_le<std::uint32_t>(bytes, nt_offset, 0x00004550U);
    write_le<std::uint16_t>(bytes, coff_offset, raw_machine(options.machine));
    write_le<std::uint16_t>(bytes, coff_offset + 2U, 1U);
    write_le<std::uint16_t>(
        bytes, coff_offset + 16U, static_cast<std::uint16_t>(optional_size));
    std::uint16_t characteristics = 0U;
    if (options.executable_flag) {
        characteristics |= 0x0002U;
    }
    if (options.system_image) {
        characteristics |= 0x1000U;
    }
    if (options.dynamic_library) {
        characteristics |= 0x2000U;
    }
    write_le(bytes, coff_offset + 18U, characteristics);
    write_le<std::uint16_t>(
        bytes, optional_offset, options.pe32_plus ? 0x020bU : 0x010bU);
    write_le<std::uint32_t>(
        bytes, optional_offset + 16U, options.zero_entry_point ? 0U : 0x1000U);
    write_le<std::uint16_t>(
        bytes, optional_offset + 68U, options.subsystem);
    const std::size_t directory_count_offset =
        optional_offset + (options.pe32_plus ? 108U : 92U);
    const std::size_t directory_table_offset =
        optional_offset + (options.pe32_plus ? 112U : 96U);
    write_le<std::uint32_t>(
        bytes, directory_count_offset, options.clr_slot ? 16U : 14U);
    if (options.managed && options.clr_slot) {
        write_le<std::uint32_t>(
            bytes, directory_table_offset + 14U * 8U, 0x1100U);
        write_le<std::uint32_t>(
            bytes, directory_table_offset + 14U * 8U + 4U, 0x48U);
    }
    write_le<std::uint32_t>(bytes, section_offset + 8U, 0x100U);
    write_le<std::uint32_t>(bytes, section_offset + 12U, 0x1000U);
    write_le<std::uint32_t>(bytes, section_offset + 16U, 0x200U);
    write_le<std::uint32_t>(bytes, section_offset + 20U, 0x200U);
    write_le<std::uint32_t>(
        bytes,
        section_offset + 36U,
        options.executable_entry_section ? 0x60000020U : 0x40000040U);
    return bytes;
}

std::filesystem::path unique_root() {
    return std::filesystem::temp_directory_path() /
        ("copperfin-windows-pe-image-" +
         std::to_string(
             std::chrono::steady_clock::now().time_since_epoch().count()));
}

void write_file(
    const std::filesystem::path& path,
    const std::vector<std::uint8_t>& bytes) {
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    output.write(
        reinterpret_cast<const char*>(bytes.data()),
        static_cast<std::streamsize>(bytes.size()));
}

WindowsPeImageInspection inspect_fixture(
    const std::filesystem::path& root,
    const std::string& name,
    const FixtureOptions& options = {}) {
    const auto path = root / name;
    write_file(path, make_fixture(options));
    return inspect_windows_pe_image(path);
}

void test_valid_images(const std::filesystem::path& root) {
    const auto x86 = inspect_fixture(
        root, "x86.exe", {.machine = WindowsPeMachine::x86, .pe32_plus = false});
    expect(
        x86.status == WindowsPeImageStatus::executable &&
            x86.machine == WindowsPeMachine::x86 && !x86.pe32_plus &&
            x86.subsystem == WindowsPeSubsystem::windows_console &&
            !x86.managed && x86.clr_directory_slot_present,
        "RQ-CF-AGENT-017: a structurally launchable PE32 x86 console image should be classified exactly");

    const auto x64_managed = inspect_fixture(
        root,
        "x64-managed.exe",
        {.machine = WindowsPeMachine::x64, .pe32_plus = true, .managed = true});
    expect(
        x64_managed.status == WindowsPeImageStatus::executable &&
            x64_managed.machine == WindowsPeMachine::x64 &&
            x64_managed.pe32_plus && x64_managed.managed,
        "RQ-CF-AGENT-017: a launchable managed PE32+ x64 image should retain its CLR classification");

    const auto without_clr_slot = inspect_fixture(
        root,
        "x64-no-clr-slot.exe",
        {.machine = WindowsPeMachine::x64, .pe32_plus = true, .clr_slot = false});
    expect(
        without_clr_slot.status == WindowsPeImageStatus::executable &&
            !without_clr_slot.managed &&
            !without_clr_slot.clr_directory_slot_present,
        "RQ-CF-AGENT-017: a native image need not declare a CLR directory slot to be launchable");

    const auto gui = inspect_fixture(
        root,
        "x64-gui.exe",
        {.machine = WindowsPeMachine::x64, .pe32_plus = true, .subsystem = 2U});
    expect(
        gui.status == WindowsPeImageStatus::executable &&
            gui.subsystem == WindowsPeSubsystem::windows_gui,
        "RQ-CF-AGENT-017: a structurally launchable Windows GUI image should be admitted");
}

void test_fail_closed_images(const std::filesystem::path& root) {
    const auto missing = inspect_windows_pe_image(root / "missing.exe");
    expect(
        missing.status == WindowsPeImageStatus::unreadable,
        "RQ-CF-AGENT-017: a missing image should be distinct from malformed bytes");

    const auto text_path = root / "script.cmd";
    write_file(text_path, {'e', 'c', 'h', 'o'});
    expect(
        inspect_windows_pe_image(text_path).status == WindowsPeImageStatus::invalid,
        "RQ-CF-AGENT-017: shell-dispatch text must not classify as a direct PE image");

    const auto dll = inspect_fixture(
        root,
        "library.dll",
        {.machine = WindowsPeMachine::x64,
         .pe32_plus = true,
         .dynamic_library = true,
         .zero_entry_point = true});
    expect(
        dll.status == WindowsPeImageStatus::dynamic_library,
        "RQ-CF-AGENT-017: a DLL should be recognized but never admitted as a process image");

    const auto unsupported_subsystem = inspect_fixture(
        root,
        "unsupported.exe",
        {.machine = WindowsPeMachine::x64,
         .pe32_plus = true,
         .subsystem = 7U});
    expect(
        unsupported_subsystem.status == WindowsPeImageStatus::invalid,
        "RQ-CF-AGENT-017: a non-GUI/non-console subsystem should fail launchability");

    const auto no_executable_flag = inspect_fixture(
        root,
        "not-executable.exe",
        {.machine = WindowsPeMachine::x64,
         .pe32_plus = true,
         .executable_flag = false});
    expect(
        no_executable_flag.status == WindowsPeImageStatus::invalid,
        "RQ-CF-AGENT-017: the PE executable-image characteristic is mandatory");

    const auto system_image = inspect_fixture(
        root,
        "system-image.exe",
        {.machine = WindowsPeMachine::x64,
         .pe32_plus = true,
         .system_image = true});
    expect(
        system_image.status == WindowsPeImageStatus::invalid,
        "RQ-CF-AGENT-017: a Windows system image must not become an agent child process");

    const auto nonexecutable_entry = inspect_fixture(
        root,
        "nonexec-entry.exe",
        {.machine = WindowsPeMachine::x64,
         .pe32_plus = true,
         .executable_entry_section = false});
    expect(
        nonexecutable_entry.status == WindowsPeImageStatus::invalid,
        "RQ-CF-AGENT-017: the entry point must belong to an executable section");

    auto mismatch = make_fixture(
        {.machine = WindowsPeMachine::x86, .pe32_plus = true});
    const auto mismatch_path = root / "machine-magic-mismatch.exe";
    write_file(mismatch_path, mismatch);
    expect(
        inspect_windows_pe_image(mismatch_path).status ==
            WindowsPeImageStatus::invalid,
        "RQ-CF-AGENT-017: machine and PE32/PE32+ magic must agree");

    auto truncated_raw = make_fixture();
    truncated_raw.resize(0x300U);
    const auto truncated_path = root / "truncated-raw.exe";
    write_file(truncated_path, truncated_raw);
    expect(
        inspect_windows_pe_image(truncated_path).status ==
            WindowsPeImageStatus::invalid,
        "RQ-CF-AGENT-017: section raw-data ranges must remain within the file");

    for (const auto section_count : {std::uint16_t{0U}, std::uint16_t{97U}}) {
        auto invalid_sections = make_fixture();
        write_le<std::uint16_t>(invalid_sections, 0x86U, section_count);
        const auto section_path = root /
            ("invalid-section-count-" + std::to_string(section_count) + ".exe");
        write_file(section_path, invalid_sections);
        expect(
            inspect_windows_pe_image(section_path).status ==
                WindowsPeImageStatus::invalid,
            "RQ-CF-AGENT-017: section counts outside 1 through 96 must fail closed");
    }
}

void test_host_compatibility(const std::filesystem::path& root) {
    const auto x86 = inspect_fixture(
        root, "compat-x86.exe", {.machine = WindowsPeMachine::x86, .pe32_plus = false});
    const auto x64 = inspect_fixture(root, "compat-x64.exe");
    const auto arm64 = inspect_fixture(
        root,
        "compat-arm64.exe",
        {.machine = WindowsPeMachine::arm64, .pe32_plus = true});
    expect(
        windows_pe_image_is_launch_compatible(x86, WindowsPeMachine::x86) &&
            !windows_pe_image_is_launch_compatible(x64, WindowsPeMachine::x86),
        "RQ-CF-AGENT-017: an x86 host should admit only x86 process images");
    expect(
        windows_pe_image_is_launch_compatible(x86, WindowsPeMachine::x64) &&
            windows_pe_image_is_launch_compatible(x64, WindowsPeMachine::x64) &&
            !windows_pe_image_is_launch_compatible(arm64, WindowsPeMachine::x64),
        "RQ-CF-AGENT-017: an x64 host should admit x86 and x64 but not ARM64 images");
    expect(
        windows_pe_image_is_launch_compatible(arm64, WindowsPeMachine::arm64) &&
            !windows_pe_image_is_launch_compatible(x64, WindowsPeMachine::arm64) &&
            !windows_pe_image_is_launch_compatible(
                arm64, WindowsPeMachine::unknown),
        "RQ-CF-AGENT-017: ARM64 and unknown-host compatibility should fail closed outside the direct native case");
}

}  // namespace

int main() {
    const auto root = unique_root();
    std::error_code error;
    std::filesystem::create_directories(root, error);
    expect(!error, "the PE-image test root should be created");
    if (!error) {
        test_valid_images(root);
        test_fail_closed_images(root);
        test_host_compatibility(root);
    }
    std::filesystem::remove_all(root, error);
    if (failures != 0) {
        std::cerr << failures << " Windows PE-image test(s) failed\n";
        return 1;
    }
    std::cout << "Windows PE-image inspection tests passed\n";
    return 0;
}
