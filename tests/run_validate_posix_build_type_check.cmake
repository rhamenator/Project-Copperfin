# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only

if(NOT UNIX)
    message(STATUS "POSIX shell validation probe is skipped outside UNIX")
    return()
endif()
if(NOT DEFINED SOURCE_DIR OR "${SOURCE_DIR}" STREQUAL "")
    message(FATAL_ERROR "SOURCE_DIR is required")
endif()
if(NOT DEFINED BINARY_DIR OR "${BINARY_DIR}" STREQUAL "")
    message(FATAL_ERROR "BINARY_DIR is required")
endif()

set(probe_root "${BINARY_DIR}/validate-posix-build-type")
set(probe_bin "${probe_root}/bin")
set(probe_build "${probe_root}/build")
set(cmake_log "${probe_root}/cmake.log")
set(ctest_log "${probe_root}/ctest.log")
file(REMOVE_RECURSE "${probe_root}")
file(MAKE_DIRECTORY "${probe_bin}" "${probe_build}" "${probe_root}/scripts")
file(COPY "${SOURCE_DIR}/scripts/validate-posix.sh" DESTINATION "${probe_root}/scripts")
file(CHMOD "${probe_root}/scripts/validate-posix.sh"
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
file(WRITE "${probe_build}/CMakeCache.txt" "CMAKE_BUILD_TYPE:STRING=Debug\n")

file(WRITE "${probe_bin}/cmake" [=[#!/bin/sh
set -eu
printf '%s\n' "$@" >> "$COPPERFIN_TEST_CMAKE_LOG"
]=])
file(WRITE "${probe_bin}/ctest" [=[#!/bin/sh
set -eu
printf '%s\n' "$@" >> "$COPPERFIN_TEST_CTEST_LOG"
]=])
file(CHMOD "${probe_bin}/cmake"
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
file(CHMOD "${probe_bin}/ctest"
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

execute_process(
    COMMAND "${CMAKE_COMMAND}" -E env
        "COPPERFIN_TEST_CMAKE_LOG=${cmake_log}"
        "COPPERFIN_TEST_CTEST_LOG=${ctest_log}"
        "COPPERFIN_BUILD_DIR=${probe_build}"
        "COPPERFIN_BUILD_TYPE=Release"
        "COPPERFIN_BUILD_JOBS=1"
        "PATH=${probe_bin}:$ENV{PATH}"
        /bin/sh "${probe_root}/scripts/validate-posix.sh"
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE error)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "validate-posix.sh probe failed:\n${output}\n${error}")
endif()

file(READ "${cmake_log}" cmake_invocations)
string(FIND "${cmake_invocations}" "-DCMAKE_BUILD_TYPE=Release" configure_index)
if(configure_index EQUAL -1)
    message(FATAL_ERROR "validate-posix.sh did not reconfigure a cached Debug build for Release:\n${cmake_invocations}")
endif()
string(FIND "${cmake_invocations}" "-G" generator_index)
if(NOT generator_index EQUAL -1)
    message(FATAL_ERROR "validate-posix.sh changed the generator while reconfiguring a cached build:\n${cmake_invocations}")
endif()
string(FIND "${cmake_invocations}" "--build" build_index)
if(build_index EQUAL -1)
    message(FATAL_ERROR "validate-posix.sh did not build after reconfiguration:\n${cmake_invocations}")
endif()
if(NOT EXISTS "${ctest_log}")
    message(FATAL_ERROR "validate-posix.sh did not run CTest after reconfiguration")
endif()
file(READ "${ctest_log}" ctest_invocations)
string(FIND "${ctest_invocations}" "--timeout\n180" timeout_index)
if(timeout_index EQUAL -1)
    message(FATAL_ERROR "validate-posix.sh did not bound each CTest case to 180 seconds:\n${ctest_invocations}")
endif()
string(FIND "${ctest_invocations}" "--parallel\n1" parallel_index)
if(parallel_index EQUAL -1)
    message(FATAL_ERROR "validate-posix.sh did not pass the configured test parallelism:\n${ctest_invocations}")
endif()

file(REMOVE_RECURSE "${probe_root}")
