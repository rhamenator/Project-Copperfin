# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

if(NOT DEFINED SOURCE_DIR OR "${SOURCE_DIR}" STREQUAL "")
    message(FATAL_ERROR "SOURCE_DIR is required")
endif()

set(header_path "${SOURCE_DIR}/include/copperfin/platform/environment.h")
set(source_path "${SOURCE_DIR}/src/platform/environment.cpp")
foreach(path IN ITEMS "${header_path}" "${source_path}")
    if(NOT EXISTS "${path}")
        message(FATAL_ERROR "Portable environment boundary file is missing: ${path}")
    endif()
endforeach()

file(READ "${header_path}" header_text)
string(TOLOWER "${header_text}" header_text_lower)
foreach(forbidden IN ITEMS
        "copperfin/platform/path.h"
        "<cstdlib>"
        "<mutex>"
        "_WIN32"
        "wchar_t"
        "getenv"
        "setenv"
        "unsetenv"
        "_wdupenv_s"
        "_wputenv_s")
    string(TOLOWER "${forbidden}" forbidden_lower)
    string(FIND "${header_text_lower}" "${forbidden_lower}" offset)
    if(NOT offset EQUAL -1)
        message(FATAL_ERROR
            "Public environment header leaks platform implementation token: ${forbidden}")
    endif()
endforeach()

foreach(required IN ITEMS
        "std::optional<std::string> read_environment_variable("
        "std::string read_environment_variable_or_empty("
        "std::optional<std::filesystem::path> read_environment_path("
        "bool write_environment_variable("
        "bool write_environment_path("
        "bool clear_environment_variable("
        "bool clear_environment_path(")
    string(FIND "${header_text}" "${required}" offset)
    if(offset EQUAL -1)
        message(FATAL_ERROR "Public environment header is missing declaration: ${required}")
    endif()
endforeach()

file(READ "${source_path}" source_text)
foreach(required IN ITEMS
        "#include \"copperfin/platform/environment.h\""
        "#include \"copperfin/platform/path.h\""
        "#if defined(_WIN32)"
        "_wdupenv_s"
        "_wputenv_s"
        "std::getenv"
        "return setenv(key.c_str(), assigned_value.c_str(), 1) == 0;"
        "return unsetenv(key.c_str()) == 0;"
        "environment_mutex")
    string(FIND "${source_text}" "${required}" offset)
    if(offset EQUAL -1)
        message(FATAL_ERROR
            "Private environment implementation is missing required token: ${required}")
    endif()
endforeach()

file(READ "${SOURCE_DIR}/CMakeLists.txt" cmake_text)
string(FIND "${cmake_text}" "src/platform/environment.cpp" source_offset)
if(source_offset EQUAL -1)
    message(FATAL_ERROR
        "Private environment implementation is not registered in cf_platform_support")
endif()

message(STATUS "Portable public environment boundary contract passed")
