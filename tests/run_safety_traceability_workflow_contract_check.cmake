# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

if(NOT DEFINED SOURCE_DIR OR "${SOURCE_DIR}" STREQUAL "")
    message(FATAL_ERROR "SOURCE_DIR is required")
endif()

set(workflow_path "${SOURCE_DIR}/.github/workflows/safety-traceability-gate.yml")
if(NOT EXISTS "${workflow_path}")
    message(FATAL_ERROR "Safety traceability workflow is missing")
endif()

file(READ "${workflow_path}" workflow)
string(REPLACE "\r\n" "\n" workflow "${workflow}")

function(require_text expected_text description)
    string(FIND "${workflow}" "${expected_text}" match_index)
    if(match_index EQUAL -1)
        message(FATAL_ERROR "Safety traceability workflow is missing ${description}")
    endif()
endfunction()

function(forbid_text forbidden_text description)
    string(FIND "${workflow}" "${forbidden_text}" match_index)
    if(NOT match_index EQUAL -1)
        message(FATAL_ERROR "Safety traceability workflow contains forbidden ${description}")
    endif()
endfunction()

function(require_text_count expected_text expected_count description)
    string(LENGTH "${expected_text}" expected_length)
    string(LENGTH "${workflow}" original_length)
    string(REPLACE "${expected_text}" "" stripped_workflow "${workflow}")
    string(LENGTH "${stripped_workflow}" stripped_length)
    math(EXPR removed_length "${original_length} - ${stripped_length}")
    math(EXPR actual_count "${removed_length} / ${expected_length}")
    if(NOT actual_count EQUAL expected_count)
        message(FATAL_ERROR
            "Safety traceability workflow must contain ${expected_count} ${description}; found ${actual_count}")
    endif()
endfunction()

function(assert_probe_is_environment_data probe)
    set(environment_binding "ISSUE_NUMBERS: ${probe}")
    set(invocation [=[-IssueNumbers $env:ISSUE_NUMBERS]=])
    string(FIND "${invocation}" "${probe}" probe_index)
    if(NOT probe_index EQUAL -1)
        message(FATAL_ERROR "Safety dispatch probe escaped its environment-data boundary")
    endif()
    string(FIND "${environment_binding}" "${probe}" binding_index)
    if(binding_index EQUAL -1)
        message(FATAL_ERROR "Safety dispatch probe was not retained as environment data")
    endif()
endfunction()

function(assert_invalid_issue_numbers probe)
    execute_process(
        COMMAND
            "${CMAKE_COMMAND}" -E env "GITHUB_TOKEN=fixture-token"
            "${POWERSHELL_EXECUTABLE}" -NoLogo -NoProfile -NonInteractive -File
            "${SOURCE_DIR}/scripts/validate-safety-traceability.ps1"
            -Repository "owner/repository"
            -IssueNumbers "${probe}"
        RESULT_VARIABLE result
        OUTPUT_VARIABLE standard_output
        ERROR_VARIABLE standard_error)
    if(result EQUAL 0)
        message(FATAL_ERROR "Safety validator accepted a hostile issue-number probe")
    endif()
    set(all_output "${standard_output}\n${standard_error}")
    string(FIND "${all_output}" "is not numeric" numeric_error_index)
    if(numeric_error_index EQUAL -1)
        message(FATAL_ERROR
            "Safety validator did not reject a hostile issue-number probe before API access: ${all_output}")
    endif()
endfunction()

function(assert_invalid_mapping_fixture)
    set(invalid_report "${CMAKE_CURRENT_BINARY_DIR}/safety-traceability-invalid-mapping-report.json")
    execute_process(
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoLogo -NoProfile -NonInteractive -File
            "${SOURCE_DIR}/scripts/validate-safety-traceability.ps1"
            -IssueJsonPath "${SOURCE_DIR}/tests/fixtures/safety_traceability_invalid_mapping_issues.json"
            -ReportPath "${invalid_report}"
        RESULT_VARIABLE result
        OUTPUT_VARIABLE standard_output
        ERROR_VARIABLE standard_error)
    if(result EQUAL 0)
        message(FATAL_ERROR "Safety validator accepted an invalid DQ/DV/HZ mapping fixture")
    endif()
    set(all_output "${standard_output}\n${standard_error}")
    string(FIND "${all_output}" "undeclared DV identifier" undeclared_dv_index)
    if(undeclared_dv_index EQUAL -1)
        message(FATAL_ERROR
            "Safety validator did not report the invalid DQ/DV/HZ mapping row: ${all_output}")
    endif()
    string(FIND "${all_output}" "not mapped" incomplete_mapping_index)
    if(incomplete_mapping_index EQUAL -1)
        message(FATAL_ERROR
            "Safety validator did not report the unmapped DQ identifier: ${all_output}")
    endif()
    file(REMOVE "${invalid_report}")
endfunction()

function(assert_no_hazard_fixture)
    set(no_hazard_report "${CMAKE_CURRENT_BINARY_DIR}/safety-traceability-no-hazard-report.json")
    execute_process(
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoLogo -NoProfile -NonInteractive -File
            "${SOURCE_DIR}/scripts/validate-safety-traceability.ps1"
            -IssueJsonPath "${SOURCE_DIR}/tests/fixtures/safety_traceability_no_hazard_issues.json"
            -ReportPath "${no_hazard_report}"
        RESULT_VARIABLE result
        OUTPUT_VARIABLE standard_output
        ERROR_VARIABLE standard_error)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR
            "Safety validator rejected the explicit HZ-NONE path:\n${standard_output}\n${standard_error}")
    endif()
    file(REMOVE "${no_hazard_report}")
endfunction()

function(assert_low_self_review_fixture)
    set(report "${CMAKE_CURRENT_BINARY_DIR}/safety-traceability-low-self-review-report.json")
    execute_process(
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoLogo -NoProfile -NonInteractive -File
            "${SOURCE_DIR}/scripts/validate-safety-traceability.ps1"
            -IssueJsonPath "${SOURCE_DIR}/tests/fixtures/safety_traceability_low_self_review_issues.json"
            -ReportPath "${report}"
        RESULT_VARIABLE result
        OUTPUT_VARIABLE standard_output
        ERROR_VARIABLE standard_error)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR
            "Safety validator rejected permitted low-severity maintainer self-review:\n${standard_output}\n${standard_error}")
    endif()
    file(REMOVE "${report}")
endfunction()

function(assert_high_self_review_rejected)
    set(report "${CMAKE_CURRENT_BINARY_DIR}/safety-traceability-high-self-review-report.json")
    execute_process(
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoLogo -NoProfile -NonInteractive -File
            "${SOURCE_DIR}/scripts/validate-safety-traceability.ps1"
            -IssueJsonPath "${SOURCE_DIR}/tests/fixtures/safety_traceability_high_self_review_issues.json"
            -ReportPath "${report}"
        RESULT_VARIABLE result
        OUTPUT_VARIABLE standard_output
        ERROR_VARIABLE standard_error)
    if(result EQUAL 0)
        message(FATAL_ERROR "Safety validator accepted high-severity maintainer self-review without independent human review")
    endif()
    set(all_output "${standard_output}\n${standard_error}")
    string(FIND "${all_output}" "requires approved Independent Human Review evidence from a second qualified reviewer"
        independent_review_index)
    if(independent_review_index EQUAL -1)
        message(FATAL_ERROR
            "Safety validator did not report the high-severity independent-review requirement: ${all_output}")
    endif()
    file(REMOVE "${report}")
endfunction()

function(assert_high_independent_review_fixture)
    set(report "${CMAKE_CURRENT_BINARY_DIR}/safety-traceability-high-independent-review-report.json")
    execute_process(
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoLogo -NoProfile -NonInteractive -File
            "${SOURCE_DIR}/scripts/validate-safety-traceability.ps1"
            -IssueJsonPath "${SOURCE_DIR}/tests/fixtures/safety_traceability_high_independent_review_issues.json"
            -ReportPath "${report}"
        RESULT_VARIABLE result
        OUTPUT_VARIABLE standard_output
        ERROR_VARIABLE standard_error)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR
            "Safety validator rejected approved high-severity independent human review:\n${standard_output}\n${standard_error}")
    endif()
    file(REMOVE "${report}")
endfunction()

function(assert_invalid_hz_none_row_fixture)
    set(invalid_report "${CMAKE_CURRENT_BINARY_DIR}/safety-traceability-invalid-hz-none-row-report.json")
    execute_process(
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoLogo -NoProfile -NonInteractive -File
            "${SOURCE_DIR}/scripts/validate-safety-traceability.ps1"
            -IssueJsonPath "${SOURCE_DIR}/tests/fixtures/safety_traceability_invalid_hz_none_row_issues.json"
            -ReportPath "${invalid_report}"
        RESULT_VARIABLE result
        OUTPUT_VARIABLE standard_output
        ERROR_VARIABLE standard_error)
    if(result EQUAL 0)
        message(FATAL_ERROR "Safety validator accepted a malformed HZ-NONE mapping row")
    endif()
    set(all_output "${standard_output}\n${standard_error}")
    string(FIND "${all_output}" "Mapping row must contain at least one DQ, DV, and HZ identifier"
        malformed_row_index)
    if(malformed_row_index EQUAL -1)
        message(FATAL_ERROR
            "Safety validator did not report the malformed HZ-NONE mapping row: ${all_output}")
    endif()
    file(REMOVE "${invalid_report}")
endfunction()

function(assert_invalid_mixed_hazard_fixture)
    set(invalid_report "${CMAKE_CURRENT_BINARY_DIR}/safety-traceability-invalid-mixed-hazard-report.json")
    execute_process(
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoLogo -NoProfile -NonInteractive -File
            "${SOURCE_DIR}/scripts/validate-safety-traceability.ps1"
            -IssueJsonPath "${SOURCE_DIR}/tests/fixtures/safety_traceability_invalid_mixed_hazard_issues.json"
            -ReportPath "${invalid_report}"
        RESULT_VARIABLE result
        OUTPUT_VARIABLE standard_output
        ERROR_VARIABLE standard_error)
    if(result EQUAL 0)
        message(FATAL_ERROR "Safety validator accepted a mixed HZ-NONE hazard declaration")
    endif()
    set(all_output "${standard_output}\n${standard_error}")
    string(FIND "${all_output}" "HZ-NONE cannot be combined with other hazard identifiers"
        mixed_hazard_index)
    if(mixed_hazard_index EQUAL -1)
        message(FATAL_ERROR
            "Safety validator did not reject the mixed HZ-NONE hazard declaration: ${all_output}")
    endif()
    file(REMOVE "${invalid_report}")
endfunction()

function(assert_invalid_row_hazard_fixture)
    set(invalid_report "${CMAKE_CURRENT_BINARY_DIR}/safety-traceability-invalid-row-hazard-report.json")
    execute_process(
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoLogo -NoProfile -NonInteractive -File
            "${SOURCE_DIR}/scripts/validate-safety-traceability.ps1"
            -IssueJsonPath "${SOURCE_DIR}/tests/fixtures/safety_traceability_invalid_row_hazard_issues.json"
            -ReportPath "${invalid_report}"
        RESULT_VARIABLE result
        OUTPUT_VARIABLE standard_output
        ERROR_VARIABLE standard_error)
    if(result EQUAL 0)
        message(FATAL_ERROR "Safety validator accepted undeclared or unknown mapping-row hazards")
    endif()
    set(all_output "${standard_output}\n${standard_error}")
    string(FIND "${all_output}" "undeclared HZ identifier" undeclared_hazard_index)
    if(undeclared_hazard_index EQUAL -1)
        message(FATAL_ERROR
            "Safety validator did not report the undeclared mapping-row hazard: ${all_output}")
    endif()
    string(FIND "${all_output}" "unknown hazard identifier" unknown_hazard_index)
    if(unknown_hazard_index EQUAL -1)
        message(FATAL_ERROR
            "Safety validator did not report the unknown mapping-row hazard: ${all_output}")
    endif()
    string(FIND "${all_output}" "outside the sole explicit no-hazard path" mixed_no_hazard_row_index)
    if(mixed_no_hazard_row_index EQUAL -1)
        message(FATAL_ERROR
            "Safety validator did not reject HZ-NONE mixed with a real row hazard: ${all_output}")
    endif()
    file(REMOVE "${invalid_report}")
endfunction()

require_text("uses: actions/checkout@de0fac2e4500dabe0009e67214ff5f5447ce83dd # v6.0.2"
    "pinned checkout action")
require_text("uses: actions/upload-artifact@043fb46d1a93c77aae656e7c1c64a875d1fc6a0a # v7.0.1"
    "pinned report uploader")
require_text("run: cmake -DSOURCE_DIR=\"$env:GITHUB_WORKSPACE\" -P tests/run_safety_traceability_workflow_contract_check.cmake"
    "self-validating dispatch-boundary step")
require_text("ISSUE_NUMBERS: \${{ inputs.issue_numbers }}" "environment-bound issue input")
require_text("-IssueNumbers $env:ISSUE_NUMBERS" "environment-bound PowerShell argument")
require_text("REQUIRE_CLOSED_ISSUES: \${{ inputs.require_closed_issues }}" "environment-bound closed-issue input")
require_text("REQUIRE_PRIMARY_HAZARDS: \${{ inputs.require_primary_hazards }}" "environment-bound primary-hazard input")
require_text_count("\${{ inputs.issue_numbers }}" 1 "issue-number dispatch interpolation")
forbid_text("-IssueNumbers \"\${{ inputs.issue_numbers }}\"" "quoted direct issue-number interpolation")

assert_probe_is_environment_data([=[2201"; Write-Output injected]=])
assert_probe_is_environment_data([=[2201; Write-Output injected]=])
assert_probe_is_environment_data([=[2201$env:RUNNER_TEMP]=])
assert_probe_is_environment_data([=[2201
Write-Output injected]=])
assert_probe_is_environment_data([=[2201 # injected comment]=])

find_program(POWERSHELL_EXECUTABLE NAMES pwsh powershell)
if(POWERSHELL_EXECUTABLE)
    set(classifier_report "${CMAKE_CURRENT_BINARY_DIR}/safety-traceability-classifier-report.json")
    execute_process(
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoLogo -NoProfile -NonInteractive -File
            "${SOURCE_DIR}/scripts/validate-safety-traceability.ps1"
            -IssueJsonPath "${SOURCE_DIR}/tests/fixtures/safety_traceability_classifier_issues.json"
            -ReportPath "${classifier_report}"
        RESULT_VARIABLE classifier_result
        OUTPUT_VARIABLE classifier_output
        ERROR_VARIABLE classifier_error)
    if(NOT classifier_result EQUAL 0)
        message(FATAL_ERROR
            "Safety validator classifier regression failed:\n${classifier_output}\n${classifier_error}")
    endif()
    file(READ "${classifier_report}" classifier_report_contents)
    string(FIND "${classifier_report_contents}" "\"validatedIssueCount\": 1" validated_count_index)
    if(validated_count_index EQUAL -1)
        message(FATAL_ERROR "Safety validator classified a safety-only implementation issue as documentation work")
    endif()
    file(REMOVE "${classifier_report}")

    assert_invalid_mapping_fixture()
    assert_no_hazard_fixture()
    assert_low_self_review_fixture()
    assert_high_self_review_rejected()
    assert_high_independent_review_fixture()
    assert_invalid_hz_none_row_fixture()
    assert_invalid_mixed_hazard_fixture()
    assert_invalid_row_hazard_fixture()

    assert_invalid_issue_numbers([=[2201"; Write-Output injected]=])
    assert_invalid_issue_numbers([=[2201; Write-Output injected]=])
    assert_invalid_issue_numbers([=[2201$env:RUNNER_TEMP]=])
    assert_invalid_issue_numbers([=[2201
Write-Output injected]=])
    assert_invalid_issue_numbers([=[2201 # injected comment]=])
else()
    message(STATUS "PowerShell is unavailable; hostile issue-number script probes are skipped")
endif()

message(STATUS "Safety traceability workflow contract check passed")
