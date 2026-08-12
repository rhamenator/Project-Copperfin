# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

if(NOT DEFINED SOURCE_DIR OR "${SOURCE_DIR}" STREQUAL "")
    message(FATAL_ERROR "SOURCE_DIR is required")
endif()

set(header_path "${SOURCE_DIR}/include/copperfin/platform/executable_path.h")
set(source_path "${SOURCE_DIR}/src/platform/executable_path.cpp")
foreach(path IN ITEMS "${header_path}" "${source_path}")
    if(NOT EXISTS "${path}")
        message(FATAL_ERROR "Executable-path boundary file is missing: ${path}")
    endif()
endforeach()

file(READ "${header_path}" header_text)
foreach(forbidden IN ITEMS
        "_WIN32"
        "windows.h"
        "unistd.h"
        "mach-o/dyld.h"
        "confstr"
        "GetModuleFileNameW"
        "readlink")
    string(FIND "${header_text}" "${forbidden}" offset)
    if(NOT offset EQUAL -1)
        message(FATAL_ERROR
            "Public executable-path header leaks platform implementation token: ${forbidden}")
    endif()
endforeach()

foreach(required IN ITEMS
        "std::optional<std::string> default_executable_search_path("
        "std::filesystem::path resolve_executable_invocation_path("
        "std::filesystem::path resolve_running_executable_path(")
    string(FIND "${header_text}" "${required}" offset)
    if(offset EQUAL -1)
        message(FATAL_ERROR
            "Public executable-path header is missing declaration: ${required}")
    endif()
endforeach()

file(READ "${source_path}" source_text)
foreach(required IN ITEMS
        "std::optional<std::string> default_executable_search_path() {\n#if defined(_WIN32)\n    return std::nullopt;"
        "confstr(_CS_PATH, nullptr, 0U)"
        "return std::string(\"/bin:/usr/bin\");"
        "GetModuleFileNameW("
        "_NSGetExecutablePath("
        "readlink(\"/proc/self/exe\"")
    string(FIND "${source_text}" "${required}" offset)
    if(offset EQUAL -1)
        message(FATAL_ERROR
            "Private executable-path implementation is missing required token: ${required}")
    endif()
endforeach()

file(READ "${SOURCE_DIR}/CMakeLists.txt" cmake_text)
string(FIND "${cmake_text}" "src/platform/executable_path.cpp" source_offset)
if(source_offset EQUAL -1)
    message(FATAL_ERROR
        "Private executable-path implementation is not registered in cf_platform_support")
endif()

message(STATUS "Portable public executable-path boundary contract passed")
