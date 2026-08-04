# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

if(NOT DEFINED SOURCE_DIR OR "${SOURCE_DIR}" STREQUAL "")
    message(FATAL_ERROR "SOURCE_DIR is required")
endif()
if(NOT DEFINED BINARY_DIR OR "${BINARY_DIR}" STREQUAL "")
    message(FATAL_ERROR "BINARY_DIR is required")
endif()

function(require_text relative_path expected_text)
    file(READ "${SOURCE_DIR}/${relative_path}" contents)
    string(FIND "${contents}" "${expected_text}" match_index)
    if(match_index EQUAL -1)
        message(FATAL_ERROR "${relative_path} is missing required build-parallelism contract: ${expected_text}")
    endif()
endfunction()

foreach(workflow_path IN ITEMS
        .github/actions/native-validation/action.yml
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

require_text("scripts/measure-windows-validation.ps1" "[ValidateRange(0, 256)]")
require_text("scripts/measure-windows-validation.ps1" "[int]\$BuildJobs = 0")
require_text("scripts/measure-windows-validation.ps1" "minimum_free_physical_memory_bytes")
require_text("scripts/measure-windows-validation.ps1" "average_tracked_cpu_percent")

require_text(".github/actions/native-validation/action.yml" "default: '2'")
require_text(".github/actions/native-validation/action.yml" "if (\$buildJobs -notin @('2', '3'))")
require_text(".github/actions/native-validation/action.yml"
    "-CommandArguments @('--build', 'build', '--config', 'Release', '--parallel', '\${{ inputs.build_jobs }}')")
require_text(".github/actions/native-validation/action.yml"
    "-BuildJobs ([int]'\${{ inputs.build_jobs }}')")

require_text(".github/workflows/windows-deep-validation.yml" "build_jobs:")
require_text(".github/workflows/windows-deep-validation.yml" "default: '2'")
require_text(".github/workflows/windows-deep-validation.yml" "          - '2'")
require_text(".github/workflows/windows-deep-validation.yml" "          - '3'")
require_text(".github/workflows/windows-deep-validation.yml"
    "-CommandArguments @('--build', 'build', '--config', '\${{ inputs.build_configuration }}', '--parallel', '\${{ inputs.build_jobs }}')")
require_text(".github/workflows/windows-deep-validation.yml"
    "-BuildJobs ([int]'\${{ inputs.build_jobs }}')")

require_text("scripts/validate-posix.sh" "build_jobs=\${COPPERFIN_BUILD_JOBS:-2}")
require_text("scripts/validate-posix.sh" "cmake --build \"\$build_dir\" --parallel \"\$build_jobs\" \"\$@\"")
require_text("scripts/validate-posix.sh" "ctest --test-dir \"\$build_dir\" --output-on-failure --timeout 180 --parallel \"\$build_jobs\"")
require_text("scripts/validate-windows.ps1" "[int]\$BuildJobs = 2")
require_text("scripts/validate-windows.ps1" "\"--parallel\", \"\$BuildJobs\"")
require_text("README.md" "Native CMake validation defaults to two concurrent compile jobs")
foreach(native_workflow IN ITEMS
        .github/workflows/native-validation-linux.yml
        .github/workflows/native-validation-macos.yml
        .github/workflows/native-validation-windows.yml)
    require_text("${native_workflow}" "timeout-minutes: 120")
endforeach()

find_program(POWERSHELL_EXECUTABLE NAMES pwsh powershell)
if(POWERSHELL_EXECUTABLE)
    set(metrics_probe_root "${BINARY_DIR}/tests/windows-validation-metrics-contract")
    set(metrics_probe_summary "${metrics_probe_root}/github-step-summary.md")
    set(metrics_probe_environment "${metrics_probe_root}/github-environment.txt")
    file(REMOVE_RECURSE "${metrics_probe_root}")

    execute_process(
        COMMAND "${CMAKE_COMMAND}" -E env
            "GITHUB_STEP_SUMMARY=${metrics_probe_summary}"
            "GITHUB_ENV=${metrics_probe_environment}"
            "${POWERSHELL_EXECUTABLE}" -NoLogo -NoProfile -File
            "${SOURCE_DIR}/scripts/measure-windows-validation.ps1"
            -Mode Initialize
            -MetricsDirectory "${metrics_probe_root}"
        RESULT_VARIABLE initialize_result
        OUTPUT_VARIABLE initialize_output
        ERROR_VARIABLE initialize_error)
    if(NOT initialize_result EQUAL 0)
        message(FATAL_ERROR
            "Windows validation metric initialization probe failed:\n${initialize_output}\n${initialize_error}")
    endif()

    execute_process(
        COMMAND "${CMAKE_COMMAND}" -E env
            "GITHUB_STEP_SUMMARY=${metrics_probe_summary}"
            "GITHUB_ENV=${metrics_probe_environment}"
            "${POWERSHELL_EXECUTABLE}" -NoLogo -NoProfile -File
            "${SOURCE_DIR}/scripts/measure-windows-validation.ps1"
            -Mode Run
            -MetricsDirectory "${metrics_probe_root}"
            -Name "CMake version probe"
            -Category other
            -FilePath "${CMAKE_COMMAND}"
            -CommandArguments "--version"
        RESULT_VARIABLE run_result
        OUTPUT_VARIABLE run_output
        ERROR_VARIABLE run_error)
    if(NOT run_result EQUAL 0)
        message(FATAL_ERROR
            "Windows validation measured-command probe failed:\n${run_output}\n${run_error}")
    endif()

    execute_process(
        COMMAND "${CMAKE_COMMAND}" -E env
            "GITHUB_STEP_SUMMARY=${metrics_probe_summary}"
            "GITHUB_ENV=${metrics_probe_environment}"
            "${POWERSHELL_EXECUTABLE}" -NoLogo -NoProfile -File
            "${SOURCE_DIR}/scripts/measure-windows-validation.ps1"
            -Mode Finalize
            -MetricsDirectory "${metrics_probe_root}"
        RESULT_VARIABLE finalize_result
        OUTPUT_VARIABLE finalize_output
        ERROR_VARIABLE finalize_error)
    if(NOT finalize_result EQUAL 0)
        message(FATAL_ERROR
            "Windows validation metric finalization probe failed:\n${finalize_output}\n${finalize_error}")
    endif()

    foreach(metrics_file IN ITEMS
            run.json
            phase-cmake-version-probe.json
            summary.json)
        if(NOT EXISTS "${metrics_probe_root}/${metrics_file}")
            message(FATAL_ERROR "Windows validation metric probe did not emit ${metrics_file}")
        endif()
    endforeach()

    file(READ "${metrics_probe_root}/phase-cmake-version-probe.json" phase_metrics)
    foreach(required_field IN ITEMS
            "\"schema_version\": 1"
            "\"category\": \"other\""
            "\"elapsed_seconds\""
            "\"exit_code\": 0"
            "\"minimum_free_physical_memory_bytes\""
            "\"average_tracked_cpu_percent\""
            "\"logical_processors\""
            "\"total_physical_memory_bytes\"")
        string(FIND "${phase_metrics}" "${required_field}" field_index)
        if(field_index EQUAL -1)
            message(FATAL_ERROR
                "Windows validation phase metrics are missing required field: ${required_field}")
        endif()
    endforeach()

    file(READ "${metrics_probe_environment}" metrics_environment)
    string(FIND "${metrics_environment}" "COPPERFIN_VALIDATION_STARTED_UTC=" started_marker_index)
    if(started_marker_index EQUAL -1)
        message(FATAL_ERROR "Windows validation metric probe did not export its start marker")
    endif()

    file(READ "${metrics_probe_summary}" metrics_summary)
    foreach(required_summary_text IN ITEMS
            "## Windows validation metrics"
            "| Phase | Category | Jobs | Elapsed |"
            "| CMake version probe | other | n/a |"
            "### Category totals"
            "Measured workflow elapsed:")
        string(FIND "${metrics_summary}" "${required_summary_text}" summary_text_index)
        if(summary_text_index EQUAL -1)
            message(FATAL_ERROR
                "Windows validation metric probe summary is missing: ${required_summary_text}")
        endif()
    endforeach()

    file(REMOVE_RECURSE "${metrics_probe_root}")
else()
    message(STATUS "PowerShell is unavailable; Windows validation metric execution probes are skipped")
endif()
