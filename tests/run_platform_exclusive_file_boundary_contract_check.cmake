# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

if(NOT DEFINED SOURCE_DIR OR "${SOURCE_DIR}" STREQUAL "")
    message(FATAL_ERROR "SOURCE_DIR is required")
endif()

set(header_path "${SOURCE_DIR}/include/copperfin/platform/exclusive_file.h")
set(source_path "${SOURCE_DIR}/src/platform/exclusive_file.cpp")
set(runtime_path "${SOURCE_DIR}/src/runtime/prg_engine_verified_file_security.inl")
set(root_build_path "${SOURCE_DIR}/CMakeLists.txt")
set(test_build_path "${SOURCE_DIR}/tests/CMakeLists.txt")
set(workflow_path "${SOURCE_DIR}/.github/workflows/generated-launcher-validation.yml")

foreach(path IN ITEMS
        "${header_path}"
        "${source_path}"
        "${runtime_path}"
        "${root_build_path}"
        "${test_build_path}"
        "${workflow_path}")
    if(NOT EXISTS "${path}")
        message(FATAL_ERROR "Exclusive-file boundary input is missing: ${path}")
    endif()
endforeach()

file(READ "${header_path}" header_text)
file(READ "${source_path}" source_text)
file(READ "${runtime_path}" runtime_text)
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
        "CreateFile"
        "WriteFile"
        "FlushFileBuffers"
        "O_EXCL"
        "O_NOFOLLOW"
        "fsync")
    forbid_text("${header_text}" "${token}" "native token in portable exclusive-file API")
endforeach()

foreach(token IN ITEMS
        "CreateFileW"
        "WriteFile"
        "FlushFileBuffers"
        "CloseHandle"
        "O_EXCL"
        "O_NOFOLLOW"
        "fsync("
        "::write(")
    forbid_text("${runtime_text}" "${token}"
        "native exclusive-file implementation in verified-file runtime")
endforeach()

require_text("${header_text}"
    "bool write_new_durable_file("
    "portable exclusive-file contract")
foreach(token IN ITEMS
        "::CreateFileW("
        "CREATE_NEW"
        "::WriteFile("
        "::FlushFileBuffers("
        "::CloseHandle("
        "O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC"
        "0600"
        "errno == EINTR"
        "::fsync(descriptor)"
        "::close(descriptor)")
    require_text("${source_text}" "${token}" "private exclusive-file implementation")
endforeach()
require_text("${runtime_text}"
    "return copperfin::platform::write_new_durable_file(path, bytes);"
    "verified-file snapshot delegation")
require_text("${root_build_text}"
    "src/platform/exclusive_file.cpp"
    "platform-support exclusive-file source registration")
require_text("${test_build_text}"
    "test_platform_exclusive_file cf_platform_support"
    "exclusive-file behavior test registration")
require_text("${workflow_text}"
    "test_platform_exclusive_file_boundary_contract"
    "hosted exclusive-file boundary scheduling")
require_text("${workflow_text}"
    "test_platform_exclusive_file"
    "hosted exclusive-file behavior scheduling")
require_text("${workflow_text}"
    "test_prg_engine_verified_dbf_security"
    "hosted verified-snapshot consumer scheduling")

message(STATUS "Portable exclusive-file boundary contract passed")
