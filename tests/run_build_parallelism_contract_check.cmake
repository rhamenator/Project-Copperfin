# Copyright © 2026 Richard M. Hamilton. All rights reserved.
# Licensed under the Project Copperfin Source-Available License or
# Commercial License. See LICENSE.md in the repository root.

if(NOT DEFINED SOURCE_DIR OR "${SOURCE_DIR}" STREQUAL "")
    message(FATAL_ERROR "SOURCE_DIR is required")
endif()

function(require_text relative_path expected_text)
    file(READ "${SOURCE_DIR}/${relative_path}" contents)
    string(FIND "${contents}" "${expected_text}" match_index)
    if(match_index EQUAL -1)
        message(FATAL_ERROR "${relative_path} is missing required build-parallelism contract: ${expected_text}")
    endif()
endfunction()

foreach(workflow_path IN ITEMS
        .github/workflows/native-validation.yml
        .github/workflows/windows-deep-validation.yml
        .github/workflows/windows-x86-declare-validation.yml)
    file(STRINGS "${SOURCE_DIR}/${workflow_path}" workflow_lines)
    set(build_command_count 0)
    foreach(workflow_line IN LISTS workflow_lines)
        string(FIND "${workflow_line}" "cmake --build" build_command_index)
        if(NOT build_command_index EQUAL -1)
            math(EXPR build_command_count "${build_command_count} + 1")
            if(NOT workflow_line MATCHES "--parallel[ \t]+2([ \t]|$)")
                message(FATAL_ERROR "${workflow_path} contains an unbounded native build: ${workflow_line}")
            endif()
        endif()
    endforeach()
    if(build_command_count EQUAL 0)
        message(FATAL_ERROR "${workflow_path} no longer contains a native build command")
    endif()
endforeach()

require_text("scripts/validate-posix.sh" "build_jobs=\${COPPERFIN_BUILD_JOBS:-2}")
require_text("scripts/validate-posix.sh" "cmake --build \"\$build_dir\" --parallel \"\$build_jobs\" \"\$@\"")
require_text("scripts/validate-windows.ps1" "[int]\$BuildJobs = 2")
require_text("scripts/validate-windows.ps1" "\"--parallel\", \"\$BuildJobs\"")
require_text("README.md" "Native CMake validation defaults to two concurrent compile jobs")
require_text(".github/workflows/native-validation.yml" "timeout-minutes: 120")
