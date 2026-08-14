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
file(READ "${SOURCE_DIR}/scripts/validate-safety-traceability.ps1" validator_source)
string(REPLACE "\r\n" "\n" validator_source "${validator_source}")

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

function(require_validator_text expected_text description)
    string(FIND "${validator_source}" "${expected_text}" match_index)
    if(match_index EQUAL -1)
        message(FATAL_ERROR "Safety validator is missing ${description}")
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

function(assert_stale_independent_review_rejected)
    set(fixture "${CMAKE_CURRENT_BINARY_DIR}/safety-traceability-stale-independent-review-issues.json")
    set(report "${CMAKE_CURRENT_BINARY_DIR}/safety-traceability-stale-independent-review-report.json")
    file(READ "${SOURCE_DIR}/tests/fixtures/safety_traceability_high_independent_review_issues.json" fixture_contents)
    string(REPLACE
        "c8a66858f9b59db08b46950b69ab911f2e247cca3799a50e2686285e06401a21"
        "0000000000000000000000000000000000000000000000000000000000000000"
        fixture_contents "${fixture_contents}")
    file(WRITE "${fixture}" "${fixture_contents}")
    execute_process(
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoLogo -NoProfile -NonInteractive -File
            "${SOURCE_DIR}/scripts/validate-safety-traceability.ps1"
            -IssueJsonPath "${fixture}"
            -ReportPath "${report}"
        RESULT_VARIABLE result
        OUTPUT_VARIABLE standard_output
        ERROR_VARIABLE standard_error)
    if(result EQUAL 0)
        message(FATAL_ERROR "Safety validator accepted a sign-off for a different issue-body revision")
    endif()
    set(all_output "${standard_output}\n${standard_error}")
    string(FIND "${all_output}" "requires an approved structured sign-off comment authored by the named distinct reviewer"
        stale_review_index)
    if(stale_review_index EQUAL -1)
        message(FATAL_ERROR
            "Safety validator did not reject the stale independent-review sign-off: ${all_output}")
    endif()
    file(REMOVE "${fixture}" "${report}")
endfunction()

function(assert_high_signoff_mutation_rejected search_text replacement_text slug)
    set(fixture "${CMAKE_CURRENT_BINARY_DIR}/safety-traceability-${slug}-issues.json")
    set(report "${CMAKE_CURRENT_BINARY_DIR}/safety-traceability-${slug}-report.json")
    file(READ "${SOURCE_DIR}/tests/fixtures/safety_traceability_high_independent_review_issues.json" source_contents)
    string(REPLACE "${search_text}" "${replacement_text}" fixture_contents "${source_contents}")
    if("${fixture_contents}" STREQUAL "${source_contents}")
        message(FATAL_ERROR "Safety mutation ${slug} did not alter its source fixture")
    endif()
    file(WRITE "${fixture}" "${fixture_contents}")
    execute_process(
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoLogo -NoProfile -NonInteractive -File
            "${SOURCE_DIR}/scripts/validate-safety-traceability.ps1"
            -IssueJsonPath "${fixture}"
            -ReportPath "${report}"
        RESULT_VARIABLE result
        OUTPUT_VARIABLE standard_output
        ERROR_VARIABLE standard_error)
    if(result EQUAL 0)
        message(FATAL_ERROR "Safety validator accepted invalid independent-review evidence mutation ${slug}")
    endif()
    set(all_output "${standard_output}\n${standard_error}")
    string(FIND "${all_output}" "requires an approved structured sign-off comment authored by the named distinct reviewer"
        placeholder_evidence_index)
    if(placeholder_evidence_index EQUAL -1)
        message(FATAL_ERROR
            "Safety validator did not reject independent-review evidence mutation ${slug}: ${all_output}")
    endif()
    file(REMOVE "${fixture}" "${report}")
endfunction()

function(assert_hidden_high_signoff_rejected wrapper_start wrapper_end slug)
    set(fixture "${CMAKE_CURRENT_BINARY_DIR}/safety-traceability-${slug}-issues.json")
    set(report "${CMAKE_CURRENT_BINARY_DIR}/safety-traceability-${slug}-report.json")
    file(READ "${SOURCE_DIR}/tests/fixtures/safety_traceability_high_independent_review_issues.json" source_contents)
    string(REPLACE
        "## Independent Review Sign-Off"
        "${wrapper_start}## Independent Review Sign-Off"
        fixture_contents "${source_contents}")
    string(REPLACE
        "reviewed issue body sha256: c8a66858f9b59db08b46950b69ab911f2e247cca3799a50e2686285e06401a21\\nresult: approved"
        "reviewed issue body sha256: c8a66858f9b59db08b46950b69ab911f2e247cca3799a50e2686285e06401a21\\nresult: approved${wrapper_end}"
        fixture_contents "${fixture_contents}")
    file(WRITE "${fixture}" "${fixture_contents}")
    execute_process(
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoLogo -NoProfile -NonInteractive -File
            "${SOURCE_DIR}/scripts/validate-safety-traceability.ps1"
            -IssueJsonPath "${fixture}"
            -ReportPath "${report}"
        RESULT_VARIABLE result
        OUTPUT_VARIABLE standard_output
        ERROR_VARIABLE standard_error)
    if(result EQUAL 0)
        message(FATAL_ERROR "Safety validator accepted non-rendered sign-off evidence ${slug}")
    endif()
    file(REMOVE "${fixture}" "${report}")
endfunction()

function(assert_high_signoff_mutation_accepted search_text replacement_text slug)
    set(fixture "${CMAKE_CURRENT_BINARY_DIR}/safety-traceability-${slug}-issues.json")
    set(report "${CMAKE_CURRENT_BINARY_DIR}/safety-traceability-${slug}-report.json")
    file(READ "${SOURCE_DIR}/tests/fixtures/safety_traceability_high_independent_review_issues.json" source_contents)
    string(REPLACE "${search_text}" "${replacement_text}" fixture_contents "${source_contents}")
    if("${fixture_contents}" STREQUAL "${source_contents}")
        message(FATAL_ERROR "Safety mutation ${slug} did not alter its source fixture")
    endif()
    file(WRITE "${fixture}" "${fixture_contents}")
    execute_process(
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoLogo -NoProfile -NonInteractive -File
            "${SOURCE_DIR}/scripts/validate-safety-traceability.ps1"
            -IssueJsonPath "${fixture}"
            -ReportPath "${report}"
        RESULT_VARIABLE result
        OUTPUT_VARIABLE standard_output
        ERROR_VARIABLE standard_error)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR
            "Safety validator rejected valid independent-review evidence mutation ${slug}: ${standard_output}\n${standard_error}")
    endif()
    file(REMOVE "${fixture}" "${report}")
endfunction()

function(assert_withdrawn_independent_review_rejected)
    set(report "${CMAKE_CURRENT_BINARY_DIR}/safety-traceability-withdrawn-independent-review-report.json")
    execute_process(
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoLogo -NoProfile -NonInteractive -File
            "${SOURCE_DIR}/scripts/validate-safety-traceability.ps1"
            -IssueJsonPath "${SOURCE_DIR}/tests/fixtures/safety_traceability_high_withdrawn_review_issues.json"
            -ReportPath "${report}"
        RESULT_VARIABLE result
        OUTPUT_VARIABLE standard_output
        ERROR_VARIABLE standard_error)
    if(result EQUAL 0)
        message(FATAL_ERROR "Safety validator accepted an independent review withdrawn by its reviewer")
    endif()
    set(all_output "${standard_output}\n${standard_error}")
    string(FIND "${all_output}" "requires an approved structured sign-off comment authored by the named distinct reviewer"
        withdrawn_review_index)
    if(withdrawn_review_index EQUAL -1)
        message(FATAL_ERROR
            "Safety validator did not report the withdrawn independent review: ${all_output}")
    endif()
    file(REMOVE "${report}")
endfunction()

function(assert_edited_withdrawal_precedence)
    set(report "${CMAKE_CURRENT_BINARY_DIR}/safety-traceability-edited-withdrawal-report.json")
    execute_process(
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoLogo -NoProfile -NonInteractive -File
            "${SOURCE_DIR}/scripts/validate-safety-traceability.ps1"
            -IssueJsonPath "${SOURCE_DIR}/tests/fixtures/safety_traceability_high_edited_withdrawal_issues.json"
            -ReportPath "${report}"
        RESULT_VARIABLE result
        OUTPUT_VARIABLE standard_output
        ERROR_VARIABLE standard_error)
    if(result EQUAL 0)
        message(FATAL_ERROR "Safety validator ignored the latest edited review withdrawal")
    endif()
    set(all_output "${standard_output}\n${standard_error}")
    string(FIND "${all_output}" "requires an approved structured sign-off comment authored by the named distinct reviewer"
        withdrawal_index)
    if(withdrawal_index EQUAL -1)
        message(FATAL_ERROR
            "Safety validator did not report the latest edited withdrawal: ${all_output}")
    endif()
    file(REMOVE "${report}")
endfunction()

function(assert_equal_timestamp_signoffs_rejected)
    set(fixture "${CMAKE_CURRENT_BINARY_DIR}/safety-traceability-equal-timestamp-signoffs-issues.json")
    set(report "${CMAKE_CURRENT_BINARY_DIR}/safety-traceability-equal-timestamp-signoffs-report.json")
    file(READ "${SOURCE_DIR}/tests/fixtures/safety_traceability_high_edited_withdrawal_issues.json" fixture_contents)
    string(REPLACE "2026-08-14T12:03:00Z" "2026-08-14T12:02:00Z" fixture_contents "${fixture_contents}")
    file(WRITE "${fixture}" "${fixture_contents}")
    execute_process(
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoLogo -NoProfile -NonInteractive -File
            "${SOURCE_DIR}/scripts/validate-safety-traceability.ps1"
            -IssueJsonPath "${fixture}"
            -ReportPath "${report}"
        RESULT_VARIABLE result
        OUTPUT_VARIABLE standard_output
        ERROR_VARIABLE standard_error)
    if(result EQUAL 0)
        message(FATAL_ERROR "Safety validator accepted conflicting equal-timestamp sign-offs")
    endif()
    file(REMOVE "${fixture}" "${report}")
endfunction()

function(assert_joined_latest_withdrawal_rejected)
    set(fixture "${CMAKE_CURRENT_BINARY_DIR}/safety-traceability-joined-latest-withdrawal-issues.json")
    set(report "${CMAKE_CURRENT_BINARY_DIR}/safety-traceability-joined-latest-withdrawal-report.json")
    file(READ "${SOURCE_DIR}/tests/fixtures/safety_traceability_high_withdrawn_review_issues.json" fixture_contents)
    string(REPLACE
        "<custom-review data-note=\\\">\\\">\\n\\n## Independent Review Sign-Off"
        "<custom-review data-note=\\\">\\\">\\n\\n## Independent Review Sign-O<!-- -->ff"
        fixture_contents "${fixture_contents}")
    file(WRITE "${fixture}" "${fixture_contents}")
    execute_process(
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoLogo -NoProfile -NonInteractive -File
            "${SOURCE_DIR}/scripts/validate-safety-traceability.ps1"
            -IssueJsonPath "${fixture}"
            -ReportPath "${report}"
        RESULT_VARIABLE result
        OUTPUT_VARIABLE standard_output
        ERROR_VARIABLE standard_error)
    if(result EQUAL 0)
        message(FATAL_ERROR "Safety validator fell back after a joined-heading latest withdrawal")
    endif()
    file(REMOVE "${fixture}" "${report}")
endfunction()

function(assert_newer_signoff_supersedes_older_timestamp_tie)
    set(fixture "${CMAKE_CURRENT_BINARY_DIR}/safety-traceability-newer-after-tie-issues.json")
    set(report "${CMAKE_CURRENT_BINARY_DIR}/safety-traceability-newer-after-tie-report.json")
    file(READ "${SOURCE_DIR}/tests/fixtures/safety_traceability_high_edited_withdrawal_issues.json" fixture_contents)
    string(REPLACE "2026-08-14T12:03:00Z" "2026-08-14T12:02:00Z" fixture_contents "${fixture_contents}")
    string(REPLACE [=[      }
    ],
    "body":]=] [=[      },
      {
        "id": 3,
        "created_at": "2026-08-14T12:04:00Z",
        "updated_at": "2026-08-14T12:04:00Z",
        "user": { "login": "copperfin-reviewer", "type": "User" },
        "body": "## Independent Review Sign-Off\n\nreviewer: copperfin-reviewer\nqualification: qualified safety-documentation reviewer\nqualification result: qualified\nverification: procedure correctness and failure boundaries\nverification result: passed\nreviewed issue body sha256: c8a66858f9b59db08b46950b69ab911f2e247cca3799a50e2686285e06401a21\nresult: approved"
      }
    ],
    "body":]=] fixture_contents "${fixture_contents}")
    file(WRITE "${fixture}" "${fixture_contents}")
    execute_process(
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoLogo -NoProfile -NonInteractive -File
            "${SOURCE_DIR}/scripts/validate-safety-traceability.ps1"
            -IssueJsonPath "${fixture}"
            -ReportPath "${report}"
        RESULT_VARIABLE result
        OUTPUT_VARIABLE standard_output
        ERROR_VARIABLE standard_error)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR
            "Safety validator let an older timestamp tie block a newer sign-off: ${standard_output}\n${standard_error}")
    endif()
    file(REMOVE "${fixture}" "${report}")
endfunction()

function(assert_entity_heading_latest_withdrawal_rejected)
    set(fixture "${CMAKE_CURRENT_BINARY_DIR}/safety-traceability-entity-heading-withdrawal-issues.json")
    set(report "${CMAKE_CURRENT_BINARY_DIR}/safety-traceability-entity-heading-withdrawal-report.json")
    file(READ "${SOURCE_DIR}/tests/fixtures/safety_traceability_high_withdrawn_review_issues.json" fixture_contents)
    string(REPLACE
        "<custom-review data-note=\\\">\\\">\\n\\n## Independent Review Sign-Off"
        "<custom-review data-note=\\\">\\\">\\n\\n## Independent Review Sign-&#79;ff"
        fixture_contents "${fixture_contents}")
    file(WRITE "${fixture}" "${fixture_contents}")
    execute_process(
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoLogo -NoProfile -NonInteractive -File
            "${SOURCE_DIR}/scripts/validate-safety-traceability.ps1"
            -IssueJsonPath "${fixture}"
            -ReportPath "${report}"
        RESULT_VARIABLE result
        OUTPUT_VARIABLE standard_output
        ERROR_VARIABLE standard_error)
    if(result EQUAL 0)
        message(FATAL_ERROR "Safety validator ignored an entity-rendered latest withdrawal heading")
    endif()
    file(REMOVE "${fixture}" "${report}")
endfunction()

function(assert_reference_heading_latest_withdrawal_rejected)
    set(fixture "${CMAKE_CURRENT_BINARY_DIR}/safety-traceability-reference-heading-withdrawal-issues.json")
    set(report "${CMAKE_CURRENT_BINARY_DIR}/safety-traceability-reference-heading-withdrawal-report.json")
    file(READ "${SOURCE_DIR}/tests/fixtures/safety_traceability_high_withdrawn_review_issues.json" fixture_contents)
    string(REPLACE
        "<custom-review data-note=\\\">\\\">\\n\\n## Independent Review Sign-Off"
        "<custom-review data-note=\\\">\\\">\\n\\n## [Independent Review Sign-Off][q]\\n\\n[q]: https://example.com/sign-off \\\"review evidence\\\""
        fixture_contents "${fixture_contents}")
    file(WRITE "${fixture}" "${fixture_contents}")
    execute_process(
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoLogo -NoProfile -NonInteractive -File
            "${SOURCE_DIR}/scripts/validate-safety-traceability.ps1"
            -IssueJsonPath "${fixture}"
            -ReportPath "${report}"
        RESULT_VARIABLE result
        OUTPUT_VARIABLE standard_output
        ERROR_VARIABLE standard_error)
    if(result EQUAL 0)
        message(FATAL_ERROR "Safety validator ignored a reference-linked latest withdrawal heading")
    endif()
    file(REMOVE "${fixture}" "${report}")
endfunction()

function(assert_low_self_review_mutation_rejected search_text replacement_text slug)
    set(fixture "${CMAKE_CURRENT_BINARY_DIR}/safety-traceability-${slug}-issues.json")
    set(report "${CMAKE_CURRENT_BINARY_DIR}/safety-traceability-${slug}-report.json")
    file(READ "${SOURCE_DIR}/tests/fixtures/safety_traceability_low_self_review_issues.json" source_contents)
    string(REPLACE "${search_text}" "${replacement_text}" fixture_contents "${source_contents}")
    if("${fixture_contents}" STREQUAL "${source_contents}")
        message(FATAL_ERROR "Safety mutation ${slug} did not alter its source fixture")
    endif()
    file(WRITE "${fixture}" "${fixture_contents}")
    execute_process(
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoLogo -NoProfile -NonInteractive -File
            "${SOURCE_DIR}/scripts/validate-safety-traceability.ps1"
            -IssueJsonPath "${fixture}"
            -ReportPath "${report}"
        RESULT_VARIABLE result
        OUTPUT_VARIABLE standard_output
        ERROR_VARIABLE standard_error)
    if(result EQUAL 0)
        message(FATAL_ERROR "Safety validator accepted invalid self-review evidence mutation ${slug}")
    endif()
    set(all_output "${standard_output}\n${standard_error}")
    string(FIND "${all_output}" "Review Evidence must record an approved structured mode"
        placeholder_evidence_index)
    if(placeholder_evidence_index EQUAL -1)
        message(FATAL_ERROR
            "Safety validator did not reject self-review evidence mutation ${slug}: ${all_output}")
    endif()
    file(REMOVE "${fixture}" "${report}")
endfunction()

function(assert_affirmative_negative_guarantee_accepted replacement_text slug)
    set(fixture "${CMAKE_CURRENT_BINARY_DIR}/safety-traceability-${slug}-issues.json")
    set(report "${CMAKE_CURRENT_BINARY_DIR}/safety-traceability-${slug}-report.json")
    file(READ "${SOURCE_DIR}/tests/fixtures/safety_traceability_low_self_review_issues.json" source_contents)
    string(REPLACE
        "verification: procedure and rendered guidance checked"
        "verification: ${replacement_text}"
        fixture_contents "${source_contents}")
    file(WRITE "${fixture}" "${fixture_contents}")
    execute_process(
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoLogo -NoProfile -NonInteractive -File
            "${SOURCE_DIR}/scripts/validate-safety-traceability.ps1"
            -IssueJsonPath "${fixture}"
            -ReportPath "${report}"
        RESULT_VARIABLE result
        OUTPUT_VARIABLE standard_output
        ERROR_VARIABLE standard_error)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR
            "Safety validator rejected an affirmative negative safety guarantee: ${standard_output}\n${standard_error}")
    endif()
    file(REMOVE "${fixture}" "${report}")
endfunction()

function(assert_placeholder_and_negated_review_evidence_rejected)
    assert_high_signoff_mutation_rejected(
        "qualification: qualified safety-documentation reviewer"
        "qualification: pending review"
        "placeholder-signoff-qualification")
    assert_high_signoff_mutation_rejected(
        "qualification: qualified safety-documentation reviewer"
        "qualification: p&#101;nding review"
        "entity-encoded-signoff-qualification")
    assert_high_signoff_mutation_rejected(
        "qualification: qualified safety-documentation reviewer"
        "qualification: [pending review][q]\\n\\n[q]: https://example.com/qualification"
        "reference-linked-placeholder-qualification")
    assert_high_signoff_mutation_rejected(
        "qualification: qualified safety-documentation reviewer"
        "qualification: [pending review][]\\n\\n[pending review]: https://example.com/qualification"
        "collapsed-reference-placeholder-qualification")
    assert_high_signoff_mutation_rejected(
        "qualification: qualified safety-documentation reviewer"
        "qualification: [pending review]\\n\\n[pending review]: https://example.com/qualification"
        "shortcut-reference-placeholder-qualification")
    assert_high_signoff_mutation_rejected(
        "qualification: qualified safety-documentation reviewer"
        "qualification: [pending \\\\[review\\\\]][q]\\n\\n[q]: https://example.com/qualification"
        "escaped-bracket-reference-placeholder-qualification")
    assert_high_signoff_mutation_rejected(
        "qualification: qualified safety-documentation reviewer"
        "qualification: [pending [review]][q]\\n\\n[q]: https://example.com/qualification"
        "nested-bracket-reference-placeholder-qualification")
    assert_high_signoff_mutation_rejected(
        "qualification: qualified safety-documentation reviewer"
        "qualification: [pending [review]](https://example.com/qualification)"
        "nested-bracket-inline-link-placeholder-qualification")
    assert_high_signoff_mutation_rejected(
        "qualification: qualified safety-documentation reviewer\\nqualification result: qualified\\nverification: procedure correctness and failure boundaries"
        "qualification: qualified safety-documentation reviewer\\nqualification result: qualified\\nverification: n/a"
        "placeholder-signoff-verification")
    assert_high_signoff_mutation_rejected(
        "qualification: qualified safety-documentation reviewer"
        "qualification: not qualified"
        "negated-signoff-qualification")
    assert_high_signoff_mutation_rejected(
        "qualification: qualified safety-documentation reviewer"
        "qualification: not-qualified reviewer"
        "punctuated-negated-signoff-qualification")
    assert_high_signoff_mutation_rejected(
        "qualification: qualified safety-documentation reviewer"
        "qualification: reviewer is not qualified"
        "embedded-negated-signoff-qualification")
    assert_high_signoff_mutation_rejected(
        "qualification: qualified safety-documentation reviewer"
        "qualification: reviewer cannot be verified as qualified"
        "cannot-signoff-qualification")
    assert_high_signoff_mutation_rejected(
        "qualification: qualified safety-documentation reviewer"
        "qualification: reviewer can't be considered qualified"
        "cant-signoff-qualification")
    assert_high_signoff_mutation_rejected(
        "qualification: qualified safety-documentation reviewer"
        "qualification: reviewer isn't qualified"
        "contracted-negated-signoff-qualification")
    assert_high_signoff_mutation_rejected(
        "qualification: qualified safety-documentation reviewer"
        "qualification: reviewer isn't independently qualified"
        "modified-contracted-negated-signoff-qualification")
    assert_high_signoff_mutation_rejected(
        "qualification: qualified safety-documentation reviewer"
        "qualification: reviewer is unqualified for safety review"
        "reviewer-first-unqualified-signoff-qualification")
    assert_high_signoff_mutation_rejected(
        "qualification: qualified safety-documentation reviewer"
        "qualification: reviewer remains unqualified for this change"
        "reviewer-remains-unqualified-signoff-qualification")
    assert_high_signoff_mutation_rejected(
        "qualification: qualified safety-documentation reviewer"
        "qualification: reviewer is unqualified for safety review despite having passed a general course"
        "reviewer-unqualified-with-unrelated-success-signoff-qualification")
    assert_high_signoff_mutation_rejected(
        "qualification: qualified safety-documentation reviewer"
        "qualification: unavailable at this time"
        "unavailable-signoff-qualification")
    assert_high_signoff_mutation_rejected(
        "qualification: qualified safety-documentation reviewer\\nqualification result: qualified\\nverification: procedure correctness and failure boundaries"
        "qualification: qualified safety-documentation reviewer\\nqualification result: qualified\\nverification: not verified"
        "negated-signoff-verification")
    assert_high_signoff_mutation_rejected(
        "qualification: qualified safety-documentation reviewer\\nqualification result: qualified\\nverification: procedure correctness and failure boundaries"
        "qualification: qualified safety-documentation reviewer\\nqualification result: qualified\\nverification: n&#111;t verified"
        "entity-encoded-negated-signoff-verification")
    assert_high_signoff_mutation_rejected(
        "qualification: qualified safety-documentation reviewer\\nqualification result: qualified\\nverification: procedure correctness and failure boundaries"
        "qualification: qualified safety-documentation reviewer\\nqualification result: qualified\\nverification: these changes were not verified"
        "embedded-negated-signoff-verification")
    assert_high_signoff_mutation_rejected(
        "qualification: qualified safety-documentation reviewer\\nqualification result: qualified\\nverification: procedure correctness and failure boundaries"
        "qualification: qualified safety-documentation reviewer\\nqualification result: qualified\\nverification: these changes weren't fully verified"
        "modified-contracted-negated-signoff-verification")
    assert_high_signoff_mutation_rejected(
        "qualification: qualified safety-documentation reviewer\\nqualification result: qualified\\nverification: procedure correctness and failure boundaries"
        "qualification: qualified safety-documentation reviewer\\nqualification result: qualified\\nverification: these changes cannot be verified"
        "cannot-signoff-verification")
    assert_high_signoff_mutation_rejected(
        "qualification: qualified safety-documentation reviewer"
        "qualification: qualified safety-documentation reviewer\\nqualification: reviewer is not qualified"
        "duplicate-signoff-qualification")
    assert_high_signoff_mutation_rejected(
        "verification result: passed"
        "verification result: failed"
        "failed-signoff-verification-result")
    assert_high_signoff_mutation_rejected(
        "qualification result: qualified"
        "qualification result: unqualified"
        "unqualified-signoff-qualification-result")
    assert_high_signoff_mutation_rejected(
        "qualification result: qualified"
        "qualification result: qualified\\nqualification status: qualified"
        "duplicate-signoff-qualification-outcome-alias")
    assert_high_signoff_mutation_rejected(
        "verification result: passed"
        "verification result: passed\\nverification status: failed"
        "duplicate-signoff-verification-outcome-alias")
    assert_high_signoff_mutation_rejected(
        "reviewed issue body sha256: c8a66858f9b59db08b46950b69ab911f2e247cca3799a50e2686285e06401a21\\nresult: approved"
        "reviewed issue body sha256: c8a66858f9b59db08b46950b69ab911f2e247cca3799a50e2686285e06401a21\\nresult: approved\\nresult: pending\\nstatus: approved"
        "duplicate-signoff-result-aliases")
    assert_high_signoff_mutation_rejected(
        "reviewed issue body sha256: c8a66858f9b59db08b46950b69ab911f2e247cca3799a50e2686285e06401a21\\nresult: approved"
        "reviewed issue body sha256: c8a66858f9b59db08b46950b69ab911f2e247cca3799a50e2686285e06401a21\\nresult: approved\\n\\n## Independent Review Sign-Off\\n\\nreviewer: copperfin-reviewer\\nqualification: qualified safety-documentation reviewer\\nqualification result: qualified\\nverification: procedure correctness and failure boundaries\\nverification result: passed\\nreviewed issue body sha256: c8a66858f9b59db08b46950b69ab911f2e247cca3799a50e2686285e06401a21\\nresult: rejected"
        "duplicate-signoff-section-withdrawal")
    assert_hidden_high_signoff_rejected("```text\\n" "\\n```" "fenced-signoff")
    assert_hidden_high_signoff_rejected("<!--\\n" "\\n-->" "html-comment-signoff")
    assert_hidden_high_signoff_rejected("    " "" "indented-signoff-heading")
    assert_hidden_high_signoff_rejected("<pre>\\n" "\\n</pre>" "pre-html-block-signoff")
    assert_hidden_high_signoff_rejected("<div>\\n" "\\n</div>" "div-html-block-signoff")
    assert_hidden_high_signoff_rejected("<?review\\n" "\\n?>" "processing-instruction-signoff")
    assert_hidden_high_signoff_rejected("<!REVIEW\\n" "\\n>" "declaration-signoff")
    assert_hidden_high_signoff_rejected("<![CDATA[\\n" "\\n]]>" "cdata-signoff")
    assert_hidden_high_signoff_rejected("<custom-review>\\n" "\\n</custom-review>" "custom-html-block-signoff")
    assert_hidden_high_signoff_rejected("</custom-review>\\n" "" "closing-html-block-signoff")
    assert_hidden_high_signoff_rejected("<custom-review data-note=\\\">\\\">\\n" "\\n</custom-review>" "quoted-angle-html-block-signoff")
    assert_hidden_high_signoff_rejected("<x@y_>\\n" "" "invalid-email-autolink-signoff")
    assert_hidden_high_signoff_rejected("<https:\tevidence>\\n" "" "control-character-uri-autolink-signoff")
    assert_hidden_high_signoff_rejected("`<!--``\\n" "\\n-->" "mismatched-code-span-comment-opener")
    assert_hidden_high_signoff_rejected("\\`<!--\\n" "\\n-->`" "escaped-backtick-comment-opener")
    assert_high_signoff_mutation_rejected(
        "reviewer: copperfin-reviewer\\nqualification: qualified safety-documentation reviewer\\nqualification result: qualified\\nverification: procedure correctness and failure boundaries\\nverification result: passed\\nreviewed issue body sha256: c8a66858f9b59db08b46950b69ab911f2e247cca3799a50e2686285e06401a21\\nresult: approved"
        "`\\nreviewer: copperfin-reviewer\\nqualification: qualified safety-documentation reviewer\\nqualification result: qualified\\nverification: procedure correctness and failure boundaries\\nverification result: passed\\nreviewed issue body sha256: c8a66858f9b59db08b46950b69ab911f2e247cca3799a50e2686285e06401a21\\nresult: approved\\n`"
        "multiline-code-span-signoff-fields")
    assert_high_signoff_mutation_rejected(
        "reviewer: copperfin-reviewer\\nqualification: qualified safety-documentation reviewer\\nqualification result: qualified\\nverification: procedure correctness and failure boundaries\\nverification result: passed\\nreviewed issue body sha256: c8a66858f9b59db08b46950b69ab911f2e247cca3799a50e2686285e06401a21\\nresult: approved"
        "`\\n<https://example.com>\\nreviewer: copperfin-reviewer\\nqualification: qualified safety-documentation reviewer\\nqualification result: qualified\\nverification: procedure correctness and failure boundaries\\nverification result: passed\\nreviewed issue body sha256: c8a66858f9b59db08b46950b69ab911f2e247cca3799a50e2686285e06401a21\\nresult: approved\\n`"
        "multiline-code-span-across-uri-autolink")
    assert_high_signoff_mutation_rejected(
        "## Independent Review Sign-Off"
        "`unmatched paragraph tick\\n\\n<custom-review>\\n`\\n\\n</custom-review>\\n\\n## Independent Review Sign-Off"
        "unmatched-code-span-before-html-block")
    assert_high_signoff_mutation_rejected(
        "## Independent Review Sign-Off"
        "`unmatched paragraph tick\\n<div>`\\n## Independent Review Sign-Off"
        "unmatched-code-span-before-block-tag-html")
    assert_high_signoff_mutation_rejected(
        "## Independent Review Sign-Off"
        "`unmatched paragraph tick\\n</div>`\\n## Independent Review Sign-Off"
        "unmatched-code-span-before-closing-block-tag-html")
    assert_high_signoff_mutation_rejected(
        "## Independent Review Sign-Off"
        "`unmatched paragraph tick\\n</div\\n`\\n## Independent Review Sign-Off"
        "unmatched-code-span-before-eol-closing-block-tag-html")
    assert_high_signoff_mutation_rejected(
        "## Independent Review Sign-Off"
        "`unmatched paragraph tick\\n<pre\\n`\\n## Independent Review Sign-Off"
        "unmatched-code-span-before-eol-block-tag-html")
    assert_high_signoff_mutation_rejected(
        "## Independent Review Sign-Off"
        "##Independent Review Sign-Off"
        "atx-heading-without-space")
    assert_high_signoff_mutation_accepted(
        "## Independent Review Sign-Off"
        "## Independent Review *Sign-Off*"
        "emphasized-signoff-heading")
    assert_high_signoff_mutation_accepted(
        "## Independent Review Sign-Off"
        "## [Independent Review Sign-Off](https://example.com/sign-off \\\"review evidence\\\")"
        "titled-inline-link-signoff-heading")
    assert_high_signoff_mutation_accepted(
        "## Independent Review Sign-Off"
        "## [Independent Review Sign-Off](review(v2))"
        "balanced-bare-destination-signoff-heading")
    assert_high_signoff_mutation_accepted(
        "## Independent Review Sign-Off"
        "## [Independent Review Sign-Off](<https://example.com/review>)"
        "angle-destination-signoff-heading")
    assert_high_signoff_mutation_accepted(
        "## Independent Review Sign-Off"
        "## [Independent Review Sign-Off](review\\\\<v2)"
        "escaped-punctuation-destination-signoff-heading")
    assert_high_signoff_mutation_accepted(
        "## Independent Review Sign-Off"
        "## [Independent Review Sign-Off](https://example.com/sign-off 'review evidence')"
        "single-quoted-title-signoff-heading")
    assert_high_signoff_mutation_accepted(
        "## Independent Review Sign-Off"
        "## [Independent Review Sign-Off](https://example.com/sign-off (review evidence))"
        "parenthesis-title-signoff-heading")
    assert_high_signoff_mutation_accepted(
        "## Independent Review Sign-Off"
        "## [Independent Review Sign-Off](https://example.com/sign-off \\\"review ) evidence\\\")"
        "quoted-title-parenthesis-signoff-heading")
    assert_high_signoff_mutation_rejected(
        "## Independent Review Sign-Off"
        "## Independent Review &#42;Sign-Off&#42;"
        "entity-produced-literal-heading-delimiters")
    assert_high_signoff_mutation_rejected(
        "## Independent Review Sign-Off"
        "## [Independent Review Sign-Off][undefined]"
        "undefined-reference-signoff-heading")
    assert_high_signoff_mutation_rejected(
        "## Independent Review Sign-Off"
        "## [Independent Review Sign-Off][q]\\n\\n[q]: <"
        "invalid-definition-reference-signoff-heading")
    assert_high_signoff_mutation_rejected(
        "## Independent Review Sign-Off"
        "## [Independent Review Sign-Off](bad destination)"
        "invalid-inline-destination-signoff-heading")
    assert_high_signoff_mutation_rejected(
        "## Independent Review Sign-Off"
        "<!-- withdrawn -->## Independent Review Sign-Off"
        "comment-removal-heading-promotion")
    assert_high_signoff_mutation_accepted(
        "## Independent Review Sign-Off"
        "## Independent Review Sign-Off <!-- evidence note -->"
        "atx-heading-trailing-html-comment")
    assert_high_signoff_mutation_accepted(
        "qualification: qualified safety-documentation reviewer"
        "qualification: qualified safety-documentation reviewer\\n\\n```xml\\n<test-result status=\\\"passed\\\"/>\\n```"
        "fenced-raw-html-example")
    assert_high_signoff_mutation_accepted(
        "qualification: qualified safety-documentation reviewer"
        "qualification: qualified safety-documentation reviewer\\n\\n```text\\n<!-- unmatched literal comment marker\\n```"
        "fenced-unmatched-comment-example")
    assert_high_signoff_mutation_accepted(
        "qualification: qualified safety-documentation reviewer"
        "qualification: qualified safety-documentation reviewer with literal `<!--` example"
        "inline-code-comment-example")
    assert_high_signoff_mutation_accepted(
        "qualification: qualified safety-documentation reviewer"
        "qualification: qualified safety-documentation reviewer with `literal\\\\` code"
        "backslash-before-code-span-closer")
    assert_high_signoff_mutation_accepted(
        "qualification: qualified safety-documentation reviewer"
        "qualification: qualified safety-documentation reviewer\\n\\n    <test-result status=\\\"passed\\\"/>"
        "indented-raw-html-example")
    assert_high_signoff_mutation_accepted(
        "qualification: qualified safety-documentation reviewer"
        "qualification: qualified safety-documentation reviewer\\n\\n<https://example.com/review-evidence>"
        "markdown-autolink-evidence")
    assert_high_signoff_mutation_accepted(
        "qualification: qualified safety-documentation reviewer"
        "qualification: qualified safety-documentation reviewer\\n\\n<reviewer@example.com>"
        "markdown-email-autolink-evidence")
    assert_high_signoff_mutation_accepted(
        "## Independent Review Sign-Off"
        "## Independent Review Sign-Off ##"
        "atx-heading-closing-sequence")
    assert_high_signoff_mutation_accepted(
        "qualification: qualified safety-documentation reviewer"
        "qualification: reviewer is not only qualified but experienced in recovery procedures"
        "affirmative-not-only-qualified-signoff-qualification")
    assert_low_self_review_mutation_rejected(
        "verification: procedure and rendered guidance checked"
        "verification: pending review"
        "placeholder-self-review-verification")
    assert_low_self_review_mutation_rejected(
        "automated evidence: focused documentation contracts pass"
        "automated evidence: pending review"
        "placeholder-self-review-automation")
    assert_low_self_review_mutation_rejected(
        "automated evidence: focused documentation contracts pass"
        "automated evidence: failed verification run"
        "failed-self-review-automation")
    assert_low_self_review_mutation_rejected(
        "automated evidence: focused documentation contracts pass"
        "automated evidence: verification run failed"
        "suffix-failed-self-review-automation")
    assert_low_self_review_mutation_rejected(
        "automated evidence: focused documentation contracts pass"
        "automated evidence: GitHub Actions workflow failed"
        "workflow-failed-self-review-automation")
    assert_low_self_review_mutation_rejected(
        "automated evidence: focused documentation contracts pass"
        "automated evidence: GitHub Actions workflow conclusion: failure"
        "workflow-failure-self-review-automation")
    assert_low_self_review_mutation_rejected(
        "automated evidence: focused documentation contracts pass"
        "automated evidence: CI checks did not pass"
        "nonadjacent-negated-self-review-automation")
    assert_low_self_review_mutation_rejected(
        "automated evidence: focused documentation contracts pass"
        "automated evidence: workflow status was failure"
        "nonadjacent-failure-self-review-automation")
    assert_low_self_review_mutation_rejected(
        "automated evidence: focused documentation contracts pass"
        "automated evidence: CI checks were unsuccessful"
        "conjugated-unsuccessful-self-review-automation")
    assert_low_self_review_mutation_rejected(
        "automated evidence: focused documentation contracts pass"
        "automated evidence: workflow did not succeed"
        "negated-succeed-self-review-automation")
    assert_low_self_review_mutation_rejected(
        "automated evidence: focused documentation contracts pass"
        "automated evidence: CI checks have failed"
        "perfect-have-failed-self-review-automation")
    assert_low_self_review_mutation_rejected(
        "automated evidence: focused documentation contracts pass"
        "automated evidence: tests had failed"
        "perfect-had-failed-self-review-automation")
    assert_low_self_review_mutation_rejected(
        "automated evidence: focused documentation contracts pass"
        "automated evidence: workflow has been unsuccessful"
        "perfect-been-unsuccessful-self-review-automation")
    assert_low_self_review_mutation_rejected(
        "automated evidence: focused documentation contracts pass"
        "automated evidence: workflow has not succeeded"
        "perfect-negated-succeeded-self-review-automation")
    assert_low_self_review_mutation_rejected(
        "automated evidence: focused documentation contracts pass"
        "automated evidence: workflow has not been successful"
        "perfect-not-been-successful-self-review-automation")
    assert_low_self_review_mutation_rejected(
        "automated evidence: focused documentation contracts pass"
        "automated evidence: CI checks had not been successful"
        "perfect-had-not-been-successful-self-review-automation")
    assert_low_self_review_mutation_rejected(
        "automated evidence: focused documentation contracts pass"
        "automated evidence: workflow has not yet been successful"
        "modified-perfect-not-successful-self-review-automation")
    assert_low_self_review_mutation_rejected(
        "automated evidence: focused documentation contracts pass"
        "automated evidence: CI checks have never passed"
        "perfect-never-passed-self-review-automation")
    assert_low_self_review_mutation_rejected(
        "automated evidence: focused documentation contracts pass"
        "automated evidence: build has failed"
        "build-failed-self-review-automation")
    assert_low_self_review_mutation_rejected(
        "automated evidence: focused documentation contracts pass"
        "automated evidence: GitHub Actions job has failed"
        "job-failed-self-review-automation")
    assert_low_self_review_mutation_rejected(
        "automated evidence: focused documentation contracts pass"
        "automated evidence: CI checks have not all passed"
        "quantified-not-passed-self-review-automation")
    assert_low_self_review_mutation_rejected(
        "automated evidence: focused documentation contracts pass"
        "automated evidence: CI checks have yet to pass"
        "yet-to-pass-self-review-automation")
    assert_low_self_review_mutation_rejected(
        "automated evidence: focused documentation contracts pass"
        "automated evidence: lint has failed"
        "lint-failed-self-review-automation")
    assert_low_self_review_mutation_rejected(
        "automated evidence: focused documentation contracts pass"
        "automated evidence: security scan has failed"
        "scan-failed-self-review-automation")
    assert_low_self_review_mutation_rejected(
        "automated evidence: focused documentation contracts pass"
        "automated evidence: deployment has failed"
        "deployment-failed-self-review-automation")
    assert_low_self_review_mutation_rejected(
        "automated evidence: focused documentation contracts pass"
        "automated evidence: test result: failure case 7"
        "failure-case-outcome-self-review-automation")
    assert_low_self_review_mutation_rejected(
        "verification: procedure and rendered guidance checked"
        "verification: reviewer cannot based on any of the currently available evidence be considered qualified"
        "unbounded-negated-self-review-verification")
    assert_low_self_review_mutation_rejected(
        "automated evidence: focused documentation contracts pass"
        "automated evidence: CI has not across all required platform specific jobs passed"
        "unbounded-negated-self-review-automation")
    assert_low_self_review_mutation_rejected(
        "verification result: passed"
        "verification result: failed"
        "failed-self-review-verification-result")
    assert_low_self_review_mutation_rejected(
        "verification result: passed"
        "verification result: passed\\nverification status: failed"
        "duplicate-self-review-verification-outcome-alias")
    assert_low_self_review_mutation_rejected(
        "automated evidence result: passed"
        "automated evidence result: cancelled"
        "cancelled-self-review-automation-result")
    assert_low_self_review_mutation_rejected(
        "automated evidence: focused documentation contracts pass"
        "automated evidence: GitHub Actions workflow was cancelled"
        "cancelled-producer-self-review-automation")
    assert_low_self_review_mutation_rejected(
        "automated evidence: focused documentation contracts pass"
        "automated evidence: security scan was cancelled"
        "cancelled-scan-self-review-automation")
    assert_low_self_review_mutation_rejected(
        "automated evidence: focused documentation contracts pass"
        "automated evidence: workflow has timed out"
        "auxiliary-timeout-self-review-automation")
    assert_low_self_review_mutation_rejected(
        "automated evidence: focused documentation contracts pass"
        "automated evidence: workflow was cancelled rather than passed"
        "cancelled-rather-than-passed-self-review-automation")
    assert_low_self_review_mutation_rejected(
        "automated evidence: focused documentation contracts pass"
        "automated evidence: cancelled-request cleanup tests passed; deployment timed out"
        "later-timeout-self-review-automation")
    assert_low_self_review_mutation_rejected(
        "verification: procedure and rendered guidance checked"
        "verification: the verification job timed out"
        "timed-out-producer-self-review-verification")
    assert_low_self_review_mutation_rejected(
        "verification: procedure and rendered guidance checked"
        "verification: review deferred until later"
        "subject-deferred-self-review-verification")
    assert_low_self_review_mutation_rejected(
        "automated evidence: focused documentation contracts pass"
        "automated evidence: automation unavailable pending rerun"
        "subject-unavailable-self-review-automation")
    assert_low_self_review_mutation_rejected(
        "automated evidence: focused documentation contracts pass"
        "automated evidence: automation is incomplete while incomplete-transaction recovery tests passed"
        "unbound-compound-scope-self-review-automation")
    assert_low_self_review_mutation_rejected(
        "automated evidence: focused documentation contracts pass"
        "automated evidence: automation is incomplete-transaction recovery tests passed"
        "copular-compound-scope-self-review-automation")
    assert_low_self_review_mutation_rejected(
        "automated evidence: focused documentation contracts pass"
        "automated evidence: workflow was cancelled while recovery tests passed"
        "unbound-cancellation-success-self-review-automation")
    assert_low_self_review_mutation_rejected(
        "automated evidence: focused documentation contracts pass"
        "automated evidence: GitHub Actions workflow that was cancelled"
        "producer-relative-cancellation-self-review-automation")
    assert_low_self_review_mutation_rejected(
        "automated evidence: focused documentation contracts pass"
        "automated evidence: workflow was cancelled-request cleanup tests passed"
        "producer-compound-cancellation-self-review-automation")
    assert_low_self_review_mutation_rejected(
        "automated evidence result: passed"
        "automated evidence result: passed\\nautomated evidence status: failed"
        "duplicate-self-review-automation-outcome-alias")
    assert_low_self_review_mutation_rejected(
        "result: approved"
        "result: approved\\nstatus: approved"
        "duplicate-self-review-result-alias")
    assert_affirmative_negative_guarantee_accepted(
        "checks have never failed"
        "affirmative-never-failed-automation")
    assert_affirmative_negative_guarantee_accepted(
        "cancelled-request cleanup tests passed"
        "affirmative-cancelled-request-scope")
    assert_affirmative_negative_guarantee_accepted(
        "timeout recovery tests passed"
        "affirmative-timeout-recovery-scope")
    assert_affirmative_negative_guarantee_accepted(
        "confirmed recovery does not require external verification services"
        "affirmative-no-external-verification-scope")
    assert_affirmative_negative_guarantee_accepted(
        "verified no residual evidence remains after rollback"
        "affirmative-no-residual-evidence-scope")
    assert_affirmative_negative_guarantee_accepted(
        "tests for requests that were cancelled passed"
        "affirmative-cancelled-request-prose")
    assert_affirmative_negative_guarantee_accepted(
        "cleanup behavior for requests that were cancelled"
        "affirmative-relative-cancelled-request-scope")
    assert_affirmative_negative_guarantee_accepted(
        "verified timed out requests are rolled back and recovery tests passed"
        "affirmative-timed-out-request-prose")
    assert_affirmative_negative_guarantee_accepted(
        "blocked-request cleanup and rollback behavior"
        "affirmative-blocked-request-scope")
    assert_affirmative_negative_guarantee_accepted(
        "incomplete-transaction recovery boundaries"
        "affirmative-incomplete-transaction-scope")
    assert_affirmative_negative_guarantee_accepted(
        "test incomplete-transaction recovery passed"
        "affirmative-subject-incomplete-transaction-scope")
    assert_affirmative_negative_guarantee_accepted(
        "automation blocked-request cleanup tests passed"
        "affirmative-subject-blocked-request-scope")
    assert_affirmative_negative_guarantee_accepted(
        "confirmed rollback does not mutate user data"
        "affirmative-does-not-guarantee")
    assert_affirmative_negative_guarantee_accepted(
        "verified rollback never mutates user data"
        "affirmative-never-guarantee")
    assert_affirmative_negative_guarantee_accepted(
        "verified failed requests never expose partial output"
        "affirmative-failed-input-guarantee")
    assert_affirmative_negative_guarantee_accepted(
        "verification of failed requests never exposes partial output"
        "affirmative-failed-input-verification")
endfunction()

function(assert_pending_legacy_high_review_rejected)
    set(report "${CMAKE_CURRENT_BINARY_DIR}/safety-traceability-high-pending-legacy-review-report.json")
    execute_process(
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoLogo -NoProfile -NonInteractive -File
            "${SOURCE_DIR}/scripts/validate-safety-traceability.ps1"
            -IssueJsonPath "${SOURCE_DIR}/tests/fixtures/safety_traceability_high_pending_legacy_review_issues.json"
            -ReportPath "${report}"
        RESULT_VARIABLE result
        OUTPUT_VARIABLE standard_output
        ERROR_VARIABLE standard_error)
    if(result EQUAL 0)
        message(FATAL_ERROR "Safety validator accepted pending legacy high-severity review as approval")
    endif()
    set(all_output "${standard_output}\n${standard_error}")
    string(FIND "${all_output}" "Review Evidence must record an approved structured mode"
        pending_review_index)
    if(pending_review_index EQUAL -1)
        message(FATAL_ERROR
            "Safety validator did not preserve high severity while rejecting pending legacy review: ${all_output}")
    endif()
    file(REMOVE "${report}")
endfunction()

function(assert_issue_form_heading_fixture)
    set(report "${CMAKE_CURRENT_BINARY_DIR}/safety-traceability-issue-form-heading-report.json")
    execute_process(
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoLogo -NoProfile -NonInteractive -File
            "${SOURCE_DIR}/scripts/validate-safety-traceability.ps1"
            -IssueJsonPath "${SOURCE_DIR}/tests/fixtures/safety_traceability_issue_form_heading_issues.json"
            -ReportPath "${report}"
        RESULT_VARIABLE result
        OUTPUT_VARIABLE standard_output
        ERROR_VARIABLE standard_error)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR
            "Safety validator rejected GitHub issue-form level-three headings:\n${standard_output}\n${standard_error}")
    endif()
    file(REMOVE "${report}")
endfunction()

function(assert_issue_form_severity_mutation_rejected replacement_text slug)
    set(fixture "${CMAKE_CURRENT_BINARY_DIR}/safety-traceability-${slug}-issues.json")
    set(report "${CMAKE_CURRENT_BINARY_DIR}/safety-traceability-${slug}-report.json")
    file(READ "${SOURCE_DIR}/tests/fixtures/safety_traceability_issue_form_heading_issues.json" source_contents)
    string(REPLACE
        "### Potential Severity If Misused\\n\\nlow"
        "${replacement_text}"
        fixture_contents "${source_contents}")
    if("${fixture_contents}" STREQUAL "${source_contents}")
        message(FATAL_ERROR "Severity mutation ${slug} did not alter its source fixture")
    endif()
    file(WRITE "${fixture}" "${fixture_contents}")
    execute_process(
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoLogo -NoProfile -NonInteractive -File
            "${SOURCE_DIR}/scripts/validate-safety-traceability.ps1"
            -IssueJsonPath "${fixture}"
            -ReportPath "${report}"
        RESULT_VARIABLE result
        OUTPUT_VARIABLE standard_output
        ERROR_VARIABLE standard_error)
    if(result EQUAL 0)
        message(FATAL_ERROR "Safety validator accepted ambiguous or hidden severity mutation ${slug}")
    endif()
    file(REMOVE "${fixture}" "${report}")
endfunction()

function(assert_issue_form_body_mutation_rejected search_text replacement_text slug)
    set(fixture "${CMAKE_CURRENT_BINARY_DIR}/safety-traceability-${slug}-issues.json")
    set(report "${CMAKE_CURRENT_BINARY_DIR}/safety-traceability-${slug}-report.json")
    file(READ "${SOURCE_DIR}/tests/fixtures/safety_traceability_issue_form_heading_issues.json" source_contents)
    string(REPLACE "${search_text}" "${replacement_text}" fixture_contents "${source_contents}")
    if("${fixture_contents}" STREQUAL "${source_contents}")
        message(FATAL_ERROR "Issue-body mutation ${slug} did not alter its source fixture")
    endif()
    file(WRITE "${fixture}" "${fixture_contents}")
    execute_process(
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoLogo -NoProfile -NonInteractive -File
            "${SOURCE_DIR}/scripts/validate-safety-traceability.ps1"
            -IssueJsonPath "${fixture}"
            -ReportPath "${report}"
        RESULT_VARIABLE result
        OUTPUT_VARIABLE standard_output
        ERROR_VARIABLE standard_error)
    if(result EQUAL 0)
        message(FATAL_ERROR "Safety validator accepted ambiguous issue-body mutation ${slug}")
    endif()
    file(REMOVE "${fixture}" "${report}")
endfunction()

function(assert_issue_form_body_mutation_accepted search_text replacement_text slug)
    set(fixture "${CMAKE_CURRENT_BINARY_DIR}/safety-traceability-${slug}-issues.json")
    set(report "${CMAKE_CURRENT_BINARY_DIR}/safety-traceability-${slug}-report.json")
    file(READ "${SOURCE_DIR}/tests/fixtures/safety_traceability_issue_form_heading_issues.json" source_contents)
    string(REPLACE "${search_text}" "${replacement_text}" fixture_contents "${source_contents}")
    if("${fixture_contents}" STREQUAL "${source_contents}")
        message(FATAL_ERROR "Issue-body mutation ${slug} did not alter its source fixture")
    endif()
    file(WRITE "${fixture}" "${fixture_contents}")
    execute_process(
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoLogo -NoProfile -NonInteractive -File
            "${SOURCE_DIR}/scripts/validate-safety-traceability.ps1"
            -IssueJsonPath "${fixture}"
            -ReportPath "${report}"
        RESULT_VARIABLE result
        OUTPUT_VARIABLE standard_output
        ERROR_VARIABLE standard_error)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR
            "Safety validator rejected valid issue-body mutation ${slug}: ${standard_output}\n${standard_error}")
    endif()
    file(REMOVE "${fixture}" "${report}")
endfunction()

function(assert_placeholder_high_reviewer_rejected)
    set(report "${CMAKE_CURRENT_BINARY_DIR}/safety-traceability-high-placeholder-reviewer-report.json")
    execute_process(
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoLogo -NoProfile -NonInteractive -File
            "${SOURCE_DIR}/scripts/validate-safety-traceability.ps1"
            -IssueJsonPath "${SOURCE_DIR}/tests/fixtures/safety_traceability_high_placeholder_reviewer_issues.json"
            -ReportPath "${report}"
        RESULT_VARIABLE result
        OUTPUT_VARIABLE standard_output
        ERROR_VARIABLE standard_error)
    if(result EQUAL 0)
        message(FATAL_ERROR "Safety validator accepted a placeholder as the high-severity independent reviewer")
    endif()
    set(all_output "${standard_output}\n${standard_error}")
    string(FIND "${all_output}" "Review Evidence must record an approved structured mode"
        placeholder_reviewer_index)
    if(placeholder_reviewer_index EQUAL -1)
        message(FATAL_ERROR
            "Safety validator did not reject the placeholder independent reviewer: ${all_output}")
    endif()
    file(REMOVE "${report}")
endfunction()

function(assert_same_author_independent_review_rejected)
    set(report "${CMAKE_CURRENT_BINARY_DIR}/safety-traceability-high-same-author-review-report.json")
    execute_process(
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoLogo -NoProfile -NonInteractive -File
            "${SOURCE_DIR}/scripts/validate-safety-traceability.ps1"
            -IssueJsonPath "${SOURCE_DIR}/tests/fixtures/safety_traceability_high_same_author_review_issues.json"
            -ReportPath "${report}"
        RESULT_VARIABLE result
        OUTPUT_VARIABLE standard_output
        ERROR_VARIABLE standard_error)
    if(result EQUAL 0)
        message(FATAL_ERROR "Safety validator accepted the issue author as an independent reviewer")
    endif()
    set(all_output "${standard_output}\n${standard_error}")
    string(FIND "${all_output}" "Review Evidence must record an approved structured mode"
        same_author_index)
    if(same_author_index EQUAL -1)
        message(FATAL_ERROR
            "Safety validator did not reject the issue author as independent reviewer: ${all_output}")
    endif()
    file(REMOVE "${report}")
endfunction()

function(assert_unattested_independent_review_rejected)
    set(report "${CMAKE_CURRENT_BINARY_DIR}/safety-traceability-high-unattested-review-report.json")
    execute_process(
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoLogo -NoProfile -NonInteractive -File
            "${SOURCE_DIR}/scripts/validate-safety-traceability.ps1"
            -IssueJsonPath "${SOURCE_DIR}/tests/fixtures/safety_traceability_high_unattested_review_issues.json"
            -ReportPath "${report}"
        RESULT_VARIABLE result
        OUTPUT_VARIABLE standard_output
        ERROR_VARIABLE standard_error)
    if(result EQUAL 0)
        message(FATAL_ERROR "Safety validator accepted an independent-review claim without reviewer-authored sign-off")
    endif()
    set(all_output "${standard_output}\n${standard_error}")
    string(FIND "${all_output}" "requires an approved structured sign-off comment authored by the named distinct reviewer"
        unattested_review_index)
    if(unattested_review_index EQUAL -1)
        message(FATAL_ERROR
            "Safety validator did not reject the unattested independent-review claim: ${all_output}")
    endif()
    file(REMOVE "${report}")
endfunction()

function(assert_incomplete_low_review_rejected)
    set(report "${CMAKE_CURRENT_BINARY_DIR}/safety-traceability-low-incomplete-review-report.json")
    execute_process(
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoLogo -NoProfile -NonInteractive -File
            "${SOURCE_DIR}/scripts/validate-safety-traceability.ps1"
            -IssueJsonPath "${SOURCE_DIR}/tests/fixtures/safety_traceability_low_incomplete_review_issues.json"
            -ReportPath "${report}"
        RESULT_VARIABLE result
        OUTPUT_VARIABLE standard_output
        ERROR_VARIABLE standard_error)
    if(result EQUAL 0)
        message(FATAL_ERROR "Safety validator accepted incomplete low-severity self-review evidence")
    endif()
    set(all_output "${standard_output}\n${standard_error}")
    string(FIND "${all_output}" "Review Evidence must record an approved structured mode"
        incomplete_review_index)
    if(incomplete_review_index EQUAL -1)
        message(FATAL_ERROR
            "Safety validator did not reject incomplete low-severity review evidence: ${all_output}")
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
require_validator_text(
    [=[$latestResponse = Invoke-RestMethod -Uri $uri -Headers $headers -Method Get]=]
    "post-comment issue re-fetch")
require_validator_text(
    [=[$latestReviewComments = @(Get-ReviewComments -Issue $latestResponse -Headers $headers)]=]
    "post-pagination comment re-fetch")
require_validator_text(
    [=[$finalResponse = Invoke-RestMethod -Uri $uri -Headers $headers -Method Get]=]
    "post-second-comment-pass issue re-fetch")
require_validator_text(
    [=[Get-ReviewCommentFingerprint -Comments $latestReviewComments]=]
    "stable comment-content fingerprint")
require_validator_text(
    [=[Issue #$num changed while review evidence was being loaded]=]
    "unstable live-snapshot rejection")

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
    assert_stale_independent_review_rejected()
    assert_withdrawn_independent_review_rejected()
    assert_edited_withdrawal_precedence()
    assert_equal_timestamp_signoffs_rejected()
    assert_joined_latest_withdrawal_rejected()
    assert_newer_signoff_supersedes_older_timestamp_tie()
    assert_entity_heading_latest_withdrawal_rejected()
    assert_reference_heading_latest_withdrawal_rejected()
    assert_placeholder_and_negated_review_evidence_rejected()
    assert_pending_legacy_high_review_rejected()
    assert_issue_form_heading_fixture()
    assert_issue_form_severity_mutation_rejected(
        "```text\\n### Potential Severity If Misused\\n\\nlow\\n```\\n\\n### Potential Severity If Misused\\n\\nhigh"
        "fenced-low-before-rendered-high-severity")
    assert_issue_form_severity_mutation_rejected(
        "<!--\\n### Potential Severity If Misused\\n\\nlow\\n-->\\n\\n### Potential Severity If Misused\\n\\nhigh"
        "commented-low-before-rendered-high-severity")
    assert_issue_form_severity_mutation_rejected(
        "<!--\\n### Potential Severity If Misused\\n\\nhigh\\n-->\\n\\n### Potential Severity If Misused\\n\\nlow"
        "commented-high-before-rendered-low-severity")
    assert_issue_form_severity_mutation_rejected(
        "### Potential Severity If Misused\\n\\n<!-- high\\n\\n### Hidden Boundary\\n\\ncommented detail -->\\n\\nlow"
        "commented-high-hidden-heading-before-rendered-low-severity")
    assert_issue_form_severity_mutation_rejected(
        "### Potential Severity If Misused\\n\\nlow\\n\\n<!--\\n### Hidden Boundary\\n\\nhigh\\n-->"
        "visible-low-before-commented-hidden-heading-high-severity")
    assert_issue_form_severity_mutation_rejected(
        "### Potential Severity If Misused\\n\\nlow\\n\\n### Potential Severity If Misused\\n\\nhigh"
        "duplicate-rendered-severity")
    assert_issue_form_severity_mutation_rejected(
        "### Potential Severity If Misused\\n\\nlow\\nhigh"
        "duplicate-values-in-rendered-severity")
    assert_issue_form_severity_mutation_rejected(
        "### Potential Severity If Misused\\n\\nlow or high"
        "multiple-values-on-one-severity-line")
    assert_issue_form_severity_mutation_rejected(
        "### Potential Severity If Misused\\n\\nlow or h<!-- -->igh"
        "inline-comment-joined-severity-token")
    assert_issue_form_severity_mutation_rejected(
        "### Potential Severity If Misused\\n\\nlow or h<!-- --><!-- -->igh"
        "consecutive-inline-comments-joined-severity-token")
    assert_issue_form_severity_mutation_rejected(
        "### Potential Severity If Misused\\n\\nlow or h&#105;gh"
        "entity-encoded-severity-token")
    assert_issue_form_severity_mutation_rejected(
        "### Potential Severity If Misused\\n\\nlow or h**igh**"
        "emphasis-split-severity-token")
    assert_issue_form_severity_mutation_rejected(
        "### Potential Severity If Misused\\n\\nlow\\n\\n```text\\nhigh\\n```"
        "fenced-value-inside-rendered-severity")
    assert_issue_form_severity_mutation_rejected(
        "### Potential Severity If Misused\\n\\nlow\\n\\n<!-- high -->"
        "commented-value-inside-rendered-severity")
    assert_issue_form_severity_mutation_rejected(
        "### Potential Severity If Misused\\n\\nlow\\n\\nSeverity: high"
        "mixed-section-and-legacy-severity")
    assert_issue_form_body_mutation_rejected(
        "### Review Evidence"
        "### Review Evidence\\n\\nmode: maintainer self-review\\nreviewer: rhamenator\\nverification: procedure and rendered guidance checked\\nverification result: passed\\nautomated evidence: focused documentation contracts pass\\nautomated evidence result: passed\\nresult: approved\\n\\n### Review Evidence"
        "duplicate-rendered-review-evidence")
    assert_issue_form_body_mutation_rejected(
        "### DQ/DV/HZ Mapping"
        "### DQ/DV/HZ Mapping\\n\\n| Documentation requirement | Verification evidence | Controlled hazards |\\n| --- | --- | --- |\\n| DQ-issue-form-heading | DV-issue-form-heading | HZ-NONE |\\n\\n### DQ/DV/HZ Mapping"
        "duplicate-rendered-mapping")
    assert_issue_form_body_mutation_rejected(
        "### Procedural Delta Map"
        "### Procedural Delta Map\\n\\nNo operator procedure changes.\\n\\n### Procedural Delta Map"
        "duplicate-rendered-procedural-delta")
    assert_issue_form_body_mutation_rejected(
        "### Review Evidence"
        "## Independent Review Evidence\\n\\nreviewer: copperfin-reviewer\\nverification: historical review was withdrawn\\nresult: rejected\\n\\n### Review Evidence"
        "mixed-current-and-legacy-review-schemas")
    assert_issue_form_body_mutation_rejected(
        "### Review Evidence"
        "### Review Evidence\\n\\nmode: maintainer self-review\\nreviewer: rhamenator\\nverification: procedure and rendered guidance checked\\nverification result: passed\\nautomated evidence: focused documentation contracts pass\\nautomated evidence result: passed\\nresult: rejected\\n\\n### Review Evidence\\n\\n## Independent Review Evidence\\n\\nreviewer: copperfin-reviewer\\nverification: historical independent review\\nresult: approved"
        "duplicate-current-with-legacy-review-schema")
    assert_issue_form_body_mutation_accepted(
        "No operator procedure changes."
        "No operator procedure changes.\\n\\n<details>\\n<summary>Additional context</summary>\\n\\nThis free-form note is not review evidence.\\n\\n</details>"
        "unrelated-details-block")
    assert_issue_form_body_mutation_accepted(
        "Walkthrough confirmed the rendered guidance."
        "Walkthrough confirmed the rendered guidance.\\n\\n```text\\nrecovery step transcript\\n```\\n\\n    indented transcript detail"
        "code-blocks-preserve-severity-boundaries")
    assert_placeholder_high_reviewer_rejected()
    assert_same_author_independent_review_rejected()
    assert_unattested_independent_review_rejected()
    assert_incomplete_low_review_rejected()
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
