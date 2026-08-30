# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
#
# Resets the build-tree fuzz corpus from the checked-in seeds immediately
# before every run, then executes the fuzz_dbf_header smoke invocation.
# libFuzzer treats its corpus argument as read-write and adds every
# newly-covering input it finds; without this reset, a second ctest
# invocation in the same build tree would run against a corpus already grown
# by the first run instead of only the fixed, checked-in seeds, making the
# documented fixed-seed smoke result non-reproducible across repeated runs.

foreach(required_variable IN ITEMS SEED_CORPUS_DIR CORPUS_DIR FUZZ_EXECUTABLE)
    if(NOT DEFINED ${required_variable} OR "${${required_variable}}" STREQUAL "")
        message(FATAL_ERROR "${required_variable} is required")
    endif()
endforeach()

if(NOT EXISTS "${SEED_CORPUS_DIR}")
    message(FATAL_ERROR "DBF header fuzz seed corpus is missing: ${SEED_CORPUS_DIR}")
endif()

file(REMOVE_RECURSE "${CORPUS_DIR}")
file(MAKE_DIRECTORY "${CORPUS_DIR}")
file(COPY "${SEED_CORPUS_DIR}/" DESTINATION "${CORPUS_DIR}")

execute_process(
    COMMAND "${FUZZ_EXECUTABLE}"
        -max_total_time=20 -seed=1 -runs=200000
        "${CORPUS_DIR}"
    RESULT_VARIABLE fuzz_result
    OUTPUT_VARIABLE fuzz_output
    ERROR_VARIABLE fuzz_error)
if(NOT fuzz_result EQUAL 0)
    message(FATAL_ERROR
        "DBF header fuzz smoke run failed (exit ${fuzz_result}):\n${fuzz_output}${fuzz_error}")
endif()

message(STATUS "DBF header fuzz smoke run passed")
