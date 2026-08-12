# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

if(NOT DEFINED SOURCE_DIR OR "${SOURCE_DIR}" STREQUAL "")
    message(FATAL_ERROR "SOURCE_DIR is required")
endif()

set(header_path "${SOURCE_DIR}/include/copperfin/platform/disk_space.h")
set(source_path "${SOURCE_DIR}/src/platform/disk_space.cpp")
set(runtime_source_path "${SOURCE_DIR}/src/runtime/prg_engine_runtime_surface_functions.cpp")
set(runtime_helpers_path "${SOURCE_DIR}/src/runtime/prg_engine_runtime_surface_platform_helpers.inl")
set(root_build_path "${SOURCE_DIR}/CMakeLists.txt")
set(test_build_path "${SOURCE_DIR}/tests/CMakeLists.txt")
set(workflow_path "${SOURCE_DIR}/.github/workflows/generated-launcher-validation.yml")

foreach(path IN ITEMS
        "${header_path}"
        "${source_path}"
        "${runtime_source_path}"
        "${runtime_helpers_path}"
        "${root_build_path}"
        "${test_build_path}"
        "${workflow_path}")
    if(NOT EXISTS "${path}")
        message(FATAL_ERROR "Disk-space boundary input is missing: ${path}")
    endif()
endforeach()

file(READ "${header_path}" header_text)
file(READ "${source_path}" source_text)
file(READ "${runtime_source_path}" runtime_source_text)
file(READ "${runtime_helpers_path}" runtime_helpers_text)
file(READ "${root_build_path}" root_build_text)
file(READ "${test_build_path}" test_build_text)
file(READ "${workflow_path}" workflow_text)

function(require_text contents expected description)
    string(FIND "${contents}" "${expected}" offset)
    if(offset EQUAL -1)
        message(FATAL_ERROR "Missing ${description}: ${expected}")
    endif()
endfunction()

function(forbid_text contents forbidden description)
    string(FIND "${contents}" "${forbidden}" offset)
    if(NOT offset EQUAL -1)
        message(FATAL_ERROR "Forbidden ${description}: ${forbidden}")
    endif()
endfunction()

foreach(token IN ITEMS
        "_WIN32"
        "windows.h"
        "statvfs"
        "DWORD"
        "GetFileAttributes"
        "GetVolumePathName"
        "GetDiskFreeSpace")
    forbid_text("${header_text}" "${token}" "native token in portable disk-space API")
endforeach()

foreach(token IN ITEMS
        "#include <windows.h>"
        "#include <sys/statvfs.h>"
        "GetFileAttributes"
        "GetVolumePathName"
        "GetDiskFreeSpace"
        "statvfs"
        "std::filesystem::space(")
    forbid_text("${runtime_source_text}${runtime_helpers_text}"
        "${token}"
        "native disk-space implementation in PRG runtime")
endforeach()

foreach(token IN ITEMS
        "std::optional<std::uintmax_t> available_disk_bytes("
        "std::optional<std::uintmax_t> disk_allocation_unit_bytes(")
    require_text("${header_text}" "${token}" "portable disk-space contract")
endforeach()

foreach(token IN ITEMS
        "std::filesystem::space(path, error)"
        "#if defined(_WIN32)"
        "#include <windows.h>"
        "::GetFileAttributesW("
        "::GetVolumePathNameW("
        "::GetDiskFreeSpaceW("
        "#include <sys/statvfs.h>"
        "::statvfs(")
    require_text("${source_text}" "${token}" "private cross-platform disk-space implementation")
endforeach()

require_text("${runtime_helpers_text}"
    "copperfin::platform::available_disk_bytes("
    "portable DISKSPACE and SYS(2020) delegation")
require_text("${runtime_helpers_text}"
    "copperfin::platform::disk_allocation_unit_bytes(path);"
    "portable SYS(2022) allocation-unit delegation")
require_text("${root_build_text}"
    "src/platform/disk_space.cpp"
    "platform-support disk-space source registration")
require_text("${test_build_text}"
    "test_platform_disk_space cf_platform_support"
    "disk-space behavior test registration")
require_text("${workflow_text}"
    "test_platform_disk_space_boundary_contract"
    "hosted disk-space boundary scheduling")
require_text("${workflow_text}"
    "test_platform_disk_space"
    "hosted disk-space behavior scheduling")

message(STATUS "Portable disk-space boundary contract passed")
