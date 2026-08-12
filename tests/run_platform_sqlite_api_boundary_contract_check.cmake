# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

if(NOT DEFINED SOURCE_DIR OR "${SOURCE_DIR}" STREQUAL "")
    message(FATAL_ERROR "SOURCE_DIR is required")
endif()

set(public_api_path "${SOURCE_DIR}/include/copperfin/platform/sqlite_api.h")
if(EXISTS "${public_api_path}")
    message(FATAL_ERROR
        "Raw SQLite native API shim must not exist in the public include tree")
endif()

file(GLOB_RECURSE public_headers
    "${SOURCE_DIR}/include/copperfin/*.h")
foreach(header_path IN LISTS public_headers)
    file(READ "${header_path}" header_text)
    foreach(forbidden IN ITEMS
            "_WIN32"
            "__APPLE__"
            "__linux__"
            "winsqlite3.h"
            "sqlite3.h"
            "__declspec(dllimport)")
        string(FIND "${header_text}" "${forbidden}" offset)
        if(NOT offset EQUAL -1)
            message(FATAL_ERROR
                "Public Copperfin header leaks native selection token ${forbidden}: ${header_path}")
        endif()
    endforeach()
endforeach()

set(private_api_path "${SOURCE_DIR}/src/platform/sqlite_api.h")
if(NOT EXISTS "${private_api_path}")
    message(FATAL_ERROR "Private SQLite connector API shim is missing")
endif()
file(READ "${private_api_path}" private_api_text)
foreach(required IN ITEMS
        "#if defined(_WIN32) && __has_include(<winsqlite3.h>)"
        "#include <winsqlite3.h>"
        "#elif defined(_WIN32)"
        "__declspec(dllimport) int sqlite3_open_v2("
        "#define SQLITE_OPEN_READONLY 0x00000001"
        "#define SQLITE_DBCONFIG_DEFENSIVE 1010"
        "#else\n#include <sqlite3.h>\n#endif")
    string(FIND "${private_api_text}" "${required}" offset)
    if(offset EQUAL -1)
        message(FATAL_ERROR
            "Private SQLite connector API shim is missing required token: ${required}")
    endif()
endforeach()

foreach(consumer IN ITEMS
        "src/platform/sqlite_federation_connector.cpp"
        "tests/test_sqlite_federation_connector.cpp"
        "tests/test_runtime_host_sqlite_federation.cpp")
    file(READ "${SOURCE_DIR}/${consumer}" consumer_text)
    string(FIND "${consumer_text}" "#include \"platform/sqlite_api.h\"" private_include_offset)
    if(private_include_offset EQUAL -1)
        message(FATAL_ERROR
            "SQLite API consumer does not use the private connector shim: ${consumer}")
    endif()
    string(FIND "${consumer_text}" "copperfin/platform/sqlite_api.h" public_include_offset)
    if(NOT public_include_offset EQUAL -1)
        message(FATAL_ERROR
            "SQLite API consumer still names the removed public shim: ${consumer}")
    endif()
endforeach()

file(READ "${SOURCE_DIR}/CMakeLists.txt" root_cmake_text)
string(FIND "${root_cmake_text}"
    [=[${CMAKE_CURRENT_SOURCE_DIR}/src]=]
    private_source_offset)
if(private_source_offset EQUAL -1)
    message(FATAL_ERROR "SQLite connector target lacks its private source include root")
endif()

file(READ "${SOURCE_DIR}/tests/CMakeLists.txt" tests_cmake_text)
foreach(required IN ITEMS
        [=[target_include_directories(test_sqlite_federation_connector PRIVATE
    ${CMAKE_SOURCE_DIR}/src)]=]
        [=[target_include_directories(test_runtime_host_sqlite_federation PRIVATE
    ${CMAKE_SOURCE_DIR}/src)]=])
    string(FIND "${tests_cmake_text}" "${required}" offset)
    if(offset EQUAL -1)
        message(FATAL_ERROR
            "SQLite test target lacks its exact private source include root: ${required}")
    endif()
endforeach()

message(STATUS "Private SQLite native API boundary contract passed")
