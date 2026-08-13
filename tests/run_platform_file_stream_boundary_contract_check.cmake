# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

if(NOT DEFINED SOURCE_DIR OR "${SOURCE_DIR}" STREQUAL "")
    message(FATAL_ERROR "SOURCE_DIR is required")
endif()

set(header_path "${SOURCE_DIR}/include/copperfin/platform/file_stream.h")
set(source_path "${SOURCE_DIR}/src/platform/file_stream.cpp")
set(runtime_path "${SOURCE_DIR}/src/runtime/prg_engine_file_io_functions.cpp")
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
        message(FATAL_ERROR "File-stream boundary input is missing: ${path}")
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
        "io.h"
        "unistd.h"
        "_wfopen"
        "_chsize_s"
        "ftruncate"
        "_fileno"
        "fileno")
    forbid_text("${header_text}" "${token}" "native token in portable file-stream API")
    forbid_text("${runtime_text}" "${token}" "native file-stream implementation in PRG runtime")
endforeach()

foreach(token IN ITEMS
        "std::FILE* open_file_stream("
        "int resize_file_stream(")
    require_text("${header_text}" "${token}" "portable file-stream contract")
endforeach()
foreach(token IN ITEMS
        "::_wfopen("
        "::_fileno("
        "::_chsize_s("
        "::fileno("
        "::ftruncate(")
    require_text("${source_text}" "${token}" "private file-stream implementation")
endforeach()
require_text("${runtime_text}"
    "copperfin::platform::open_file_stream(path, mode)"
    "FOPEN platform delegation")
require_text("${runtime_text}"
    "copperfin::platform::open_file_stream(path, \"wb+\")"
    "missing read/write file platform delegation")
require_text("${runtime_text}"
    "const int result = copperfin::platform::resize_file_stream("
    "FCHSIZE platform delegation")
require_text("${root_build_text}"
    "src/platform/file_stream.cpp"
    "platform-support file-stream source registration")
require_text("${test_build_text}"
    "test_platform_file_stream cf_platform_support"
    "file-stream behavior test registration")
require_text("${workflow_text}"
    "test_platform_file_stream_boundary_contract"
    "hosted file-stream boundary scheduling")
require_text("${workflow_text}"
    "test_platform_file_stream"
    "hosted file-stream behavior scheduling")
require_text("${workflow_text}"
    "test_prg_engine_file_io_functions"
    "hosted PRG file-I/O consumer scheduling")

message(STATUS "Portable file-stream boundary contract passed")
