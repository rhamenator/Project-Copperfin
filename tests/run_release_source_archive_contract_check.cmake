# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

cmake_minimum_required(VERSION 3.20)

foreach(required_variable IN ITEMS SOURCE_ARCHIVE EXPECTED_REVISION)
    if(NOT DEFINED ${required_variable} OR "${${required_variable}}" STREQUAL "")
        message(FATAL_ERROR "${required_variable} is required")
    endif()
endforeach()

string(LENGTH "${EXPECTED_REVISION}" expected_revision_length)
if(NOT expected_revision_length EQUAL 40 OR NOT EXPECTED_REVISION MATCHES "^[0-9a-f]+$")
    message(FATAL_ERROR "EXPECTED_REVISION must be a lowercase full Git object ID")
endif()

get_filename_component(source_archive "${SOURCE_ARCHIVE}" ABSOLUTE)
if(NOT EXISTS "${source_archive}" OR IS_DIRECTORY "${source_archive}" OR IS_SYMLINK "${source_archive}")
    message(FATAL_ERROR "Corresponding Source archive is missing or not a regular file: ${source_archive}")
endif()

get_filename_component(archive_name "${source_archive}" NAME)
set(expected_name "Project-Copperfin-source-${EXPECTED_REVISION}.zip")
if(NOT archive_name STREQUAL expected_name)
    message(FATAL_ERROR "Corresponding Source archive name mismatch: expected ${expected_name}, found ${archive_name}")
endif()

execute_process(
    COMMAND "${CMAKE_COMMAND}" -E tar tf "${source_archive}"
    RESULT_VARIABLE list_result
    OUTPUT_VARIABLE archive_listing
    ERROR_VARIABLE list_error
)
if(NOT list_result EQUAL 0)
    message(FATAL_ERROR "Cannot list Corresponding Source archive: ${list_error}")
endif()
string(REPLACE "\r\n" "\n" archive_listing "${archive_listing}")
string(REPLACE "\n" ";" archive_entries "${archive_listing}")

set(expected_prefix "Project-Copperfin-${EXPECTED_REVISION}/")
set(entry_count 0)
foreach(entry IN LISTS archive_entries)
    if(entry STREQUAL "")
        continue()
    endif()
    math(EXPR entry_count "${entry_count} + 1")
    string(FIND "${entry}" "${expected_prefix}" prefix_offset)
    if(NOT prefix_offset EQUAL 0)
        message(FATAL_ERROR "Source archive entry escapes the exact-revision prefix: ${entry}")
    endif()
    if(entry MATCHES "(^|/)\\.git(/|$)")
        message(FATAL_ERROR "Source archive contains Git administrative data: ${entry}")
    endif()
endforeach()
if(entry_count LESS 20)
    message(FATAL_ERROR "Corresponding Source archive is unexpectedly small: ${entry_count} entries")
endif()

foreach(required_entry IN ITEMS
        LICENSE
        SOURCE.md
        THIRD_PARTY_NOTICES.md
        LICENSES/LicenseRef-Copperfin-Application-Runtime-Toolchain-Exception-1.0.txt
        docs/contracts/release-license-metadata.json
        CMakeLists.txt
        .github/workflows/build-installers.yml
        .github/workflows/build-vsix.yml
        src/runtime/runtime_pipeline_public_api.cpp)
    string(FIND "${archive_listing}" "${expected_prefix}${required_entry}\n" required_offset)
    if(required_offset EQUAL -1)
        message(FATAL_ERROR "Corresponding Source archive omits ${required_entry}")
    endif()
endforeach()

message(STATUS "Exact-revision Corresponding Source archive contract passed: ${archive_name}")
