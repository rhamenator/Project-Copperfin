# Copyright © 2026 Richard M. Hamilton. All rights reserved.
# Licensed under the Project Copperfin Source-Available License or
# Commercial License. See LICENSE.md in the repository root.

foreach(required_variable IN ITEMS ARTIFACT_DIR EXPECTED_ARTIFACTS)
    if(NOT DEFINED ${required_variable} OR "${${required_variable}}" STREQUAL "")
        message(FATAL_ERROR "${required_variable} is required")
    endif()
endforeach()

if(NOT IS_DIRECTORY "${ARTIFACT_DIR}")
    message(FATAL_ERROR "CPack artifact directory does not exist: ${ARTIFACT_DIR}")
endif()

# CMake's RELATIVE glob base must be absolute when this script is invoked with
# the relative paths used by the platform installer workflows.
get_filename_component(ARTIFACT_DIR "${ARTIFACT_DIR}" ABSOLUTE)

set(expected_artifacts ${EXPECTED_ARTIFACTS})
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
    set(artifact_path "${ARTIFACT_DIR}/${artifact}")
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

file(GLOB artifact_entries RELATIVE "${ARTIFACT_DIR}" "${ARTIFACT_DIR}/*")
foreach(entry IN LISTS artifact_entries)
    if(IS_DIRECTORY "${ARTIFACT_DIR}/${entry}")
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

message(STATUS "CPack artifact contract passed: ${ARTIFACT_DIR}")
