# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

if(NOT DEFINED SOURCE_DIR OR "${SOURCE_DIR}" STREQUAL "")
    message(FATAL_ERROR "SOURCE_DIR is required")
endif()

set(header_path "${SOURCE_DIR}/include/copperfin/platform/file_version.h")
set(source_path "${SOURCE_DIR}/src/platform/file_version.cpp")
set(runtime_path "${SOURCE_DIR}/src/runtime/prg_engine_variables.inl")
set(root_build_path "${SOURCE_DIR}/CMakeLists.txt")
set(workflow_path "${SOURCE_DIR}/.github/workflows/generated-launcher-validation.yml")
foreach(path IN ITEMS
        "${header_path}"
        "${source_path}"
        "${runtime_path}"
        "${root_build_path}"
        "${workflow_path}")
    if(NOT EXISTS "${path}")
        message(FATAL_ERROR "File-version boundary input is missing: ${path}")
    endif()
endforeach()

file(READ "${header_path}" header_text)
file(READ "${source_path}" source_text)
file(READ "${runtime_path}" runtime_text)
file(READ "${root_build_path}" root_build_text)
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
        "winver.h"
        "DWORD"
        "WORD"
        "UINT"
        "LPVOID"
        "GetFileVersionInfo"
        "VerQueryValue"
        "WideCharToMultiByte")
    forbid_text("${header_text}" "${token}" "native token in portable file-version API")
endforeach()

foreach(token IN ITEMS
        "FileVersionArrayMetadata"
        "extract_file_version_metadata"
        "scan_utf16le_strings"
        "GetFileVersionInfo"
        "VerQueryValue"
        "WideCharToMultiByte")
    forbid_text("${runtime_text}" "${token}" "native file-version implementation in PRG interpreter")
endforeach()

foreach(token IN ITEMS
        "struct FileVersionMetadata"
        "std::string full_version = \"0.0.0.0\";"
        "std::string file_description;"
        "std::string company_name;"
        "std::string file_version = \"0.0.0.0\";"
        "std::string product_name;"
        "std::string product_version = \"0.0.0.0\";"
        "std::string trademark_or_copyright;"
        "FileVersionMetadata read_file_version_metadata(")
    require_text("${header_text}" "${token}" "portable file-version contract")
endforeach()

foreach(token IN ITEMS
        "#if defined(_WIN32)"
        "#include <windows.h>"
        "#include <winver.h>"
        "::GetFileVersionInfoSizeW("
        "::GetFileVersionInfoW("
        "::VerQueryValueW("
        "scan_utf16le_strings(bytes)"
        "path_to_utf8_string(path.filename())")
    require_text("${source_text}" "${token}" "private cross-platform file-version implementation")
endforeach()

require_text("${runtime_text}"
    "copperfin::platform::read_file_version_metadata(*metadata_path);"
    "portable AGETFILEVERSION delegation")
require_text("${runtime_text}"
    "materialize_verified_file_snapshot("
    "verified-snapshot admission before file-version inspection")
require_text("${root_build_text}"
    "src/platform/file_version.cpp"
    "platform-support file-version source registration")
require_text("${root_build_text}"
    "target_link_libraries(cf_platform_support PRIVATE version)"
    "private Windows version-resource linkage")
require_text("${workflow_text}"
    "test_platform_file_version_boundary_contract"
    "hosted file-version boundary scheduling")

message(STATUS "Portable file-version boundary contract passed")
