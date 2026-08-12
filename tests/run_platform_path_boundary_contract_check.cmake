# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

if(NOT DEFINED SOURCE_DIR OR "${SOURCE_DIR}" STREQUAL "")
    message(FATAL_ERROR "SOURCE_DIR is required")
endif()

set(header_path "${SOURCE_DIR}/include/copperfin/platform/path.h")
set(source_path "${SOURCE_DIR}/src/platform/path.cpp")
set(runtime_helper_path "${SOURCE_DIR}/src/runtime/prg_engine_helpers.cpp")
foreach(path IN ITEMS "${header_path}" "${source_path}" "${runtime_helper_path}")
    if(NOT EXISTS "${path}")
        message(FATAL_ERROR "Portable path boundary file is missing: ${path}")
    endif()
endforeach()

file(READ "${runtime_helper_path}" runtime_helper_text)
foreach(forbidden IN ITEMS
        "#include <windows.h>"
        "CompareStringOrdinal"
        "CSTR_EQUAL")
    string(FIND "${runtime_helper_text}" "${forbidden}" offset)
    if(NOT offset EQUAL -1)
        message(FATAL_ERROR
            "PRG helper retains native path-comparison implementation: ${forbidden}")
    endif()
endforeach()
string(FIND "${runtime_helper_text}"
    "copperfin::platform::path_equal_case_insensitive("
    runtime_delegation_offset)
if(runtime_delegation_offset EQUAL -1)
    message(FATAL_ERROR "PRG helper does not delegate VFP path identity to cf_platform_support")
endif()

file(READ "${header_path}" header_text)
string(FIND "${header_text}"
    "bool path_equal_case_insensitive("
    path_identity_declaration_offset)
if(path_identity_declaration_offset EQUAL -1)
    message(FATAL_ERROR
        "Public path header does not declare the VFP path-identity boundary")
endif()
string(TOLOWER "${header_text}" header_text_lower)
foreach(forbidden IN ITEMS
        "windows.h"
        "winsock2.h"
        "shellapi.h"
        "shlwapi.h"
        "_WIN32"
        "widechartomultibyte"
        "multibytetowidechar"
        "comparestringordinal"
        "lcmapstringex")
    string(TOLOWER "${forbidden}" forbidden_lower)
    string(FIND "${header_text_lower}" "${forbidden_lower}" offset)
    if(NOT offset EQUAL -1)
        message(FATAL_ERROR
            "Public path header leaks platform implementation token: ${forbidden}")
    endif()
endforeach()

file(READ "${source_path}" source_text)
foreach(required IN ITEMS
        "#include <windows.h>"
        "path_to_utf8_string"
        "path_from_utf8_string"
        "path_component_equal_for_platform"
        "path_equal_case_insensitive"
        "left_value.size() > maximum_api_length"
        "right_value.size() > maximum_api_length"
        "Unicode case table"
        "Windows filesystem's case-insensitive behavior"
        "CharLowerBuffW reports zero on failure")
    string(FIND "${source_text}" "${required}" offset)
    if(offset EQUAL -1)
        message(FATAL_ERROR
            "Private path implementation is missing required token: ${required}")
    endif()
endforeach()
string(FIND "${source_text}"
    "return path_component_equal_for_platform(normalized_left, normalized_right);"
    whole_path_windows_delegation_offset)
if(whole_path_windows_delegation_offset EQUAL -1)
    message(FATAL_ERROR
        "Windows whole-path identity does not reuse the complete Unicode comparison boundary")
endif()

file(READ "${SOURCE_DIR}/CMakeLists.txt" cmake_text)
string(FIND "${cmake_text}" "src/platform/path.cpp" path_source_offset)
if(path_source_offset EQUAL -1)
    message(FATAL_ERROR "Private path implementation is not registered in the platform library")
endif()
string(FIND "${cmake_text}"
    "target_link_libraries(cf_platform_support PRIVATE user32)"
    user32_link_offset)
if(user32_link_offset EQUAL -1)
    message(FATAL_ERROR "Windows path implementation must declare its User32 dependency")
endif()

message(STATUS "Portable public path boundary contract passed")
