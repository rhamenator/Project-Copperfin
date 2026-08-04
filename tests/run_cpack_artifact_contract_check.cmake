# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

foreach(required_variable IN ITEMS
        COPPERFIN_ARTIFACT_DIR
        COPPERFIN_VERSION_FILE
        COPPERFIN_EXPECTED_ARTIFACT_SUFFIXES)
    if(NOT DEFINED ${required_variable} OR "${${required_variable}}" STREQUAL "")
        message(FATAL_ERROR "${required_variable} is required")
    endif()
endforeach()

get_filename_component(artifact_dir "${COPPERFIN_ARTIFACT_DIR}" ABSOLUTE
    BASE_DIR "${CMAKE_CURRENT_LIST_DIR}/..")
if(NOT IS_DIRECTORY "${artifact_dir}")
    message(FATAL_ERROR "CPack artifact directory does not exist: ${artifact_dir}")
endif()

get_filename_component(version_file "${COPPERFIN_VERSION_FILE}" ABSOLUTE
    BASE_DIR "${CMAKE_CURRENT_LIST_DIR}/..")

if(NOT EXISTS "${version_file}")
    message(FATAL_ERROR "Generated package version file does not exist: ${version_file}")
endif()
file(READ "${version_file}" package_version)
string(STRIP "${package_version}" package_version)
if(NOT package_version MATCHES "^[0-9]+\\.[0-9]+\\.[0-9]+$")
    message(FATAL_ERROR "Generated package version is invalid: '${package_version}'")
endif()

set(expected_artifacts)
foreach(artifact_suffix IN LISTS COPPERFIN_EXPECTED_ARTIFACT_SUFFIXES)
    list(APPEND expected_artifacts "copperfin-${package_version}-${artifact_suffix}")
endforeach()
list(SORT expected_artifacts)
list(REMOVE_DUPLICATES expected_artifacts)
list(LENGTH expected_artifacts expected_count)
if(expected_count EQUAL 0)
    message(FATAL_ERROR "CPack artifact contract must name at least one expected file")
endif()

foreach(artifact IN LISTS expected_artifacts)
    if(artifact MATCHES "[/\\\\]" OR artifact STREQUAL "." OR artifact STREQUAL "..")
        message(FATAL_ERROR "Expected CPack artifact must be a direct file name: ${artifact}")
    endif()
    set(artifact_path "${artifact_dir}/${artifact}")
    if(NOT EXISTS "${artifact_path}" OR
            IS_DIRECTORY "${artifact_path}" OR
            IS_SYMLINK "${artifact_path}")
        message(FATAL_ERROR "Expected CPack artifact is missing or not a regular file: ${artifact_path}")
    endif()
    file(SIZE "${artifact_path}" artifact_size)
    if(artifact_size LESS_EQUAL 0)
        message(FATAL_ERROR "Expected CPack artifact is empty: ${artifact_path}")
    endif()
endforeach()

file(GLOB artifact_entries RELATIVE "${artifact_dir}" "${artifact_dir}/*")
foreach(entry IN LISTS artifact_entries)
    if(IS_DIRECTORY "${artifact_dir}/${entry}")
        message(FATAL_ERROR "CPack output directory contains an unexpected directory: ${entry}")
    endif()
    list(FIND expected_artifacts "${entry}" expected_index)
    if(expected_index EQUAL -1)
        message(FATAL_ERROR "CPack output directory contains an unexpected artifact: ${entry}")
    endif()
endforeach()

list(LENGTH artifact_entries actual_count)
if(NOT actual_count EQUAL expected_count)
    message(FATAL_ERROR
        "CPack artifact count mismatch: expected ${expected_count}, found ${actual_count}")
endif()

message(STATUS "CPack artifact contract passed: ${artifact_dir}")
