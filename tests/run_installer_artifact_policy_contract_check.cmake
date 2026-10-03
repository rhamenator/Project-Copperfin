# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
# Traceability: RQ-CF-REL-009; DQ-installer-artifact-retention-policy;
# DV-installer-artifact-retention-policy-contract; HZ-system-failure-01;
# HZ-doc-command-01.

if(NOT DEFINED SOURCE_DIR OR "${SOURCE_DIR}" STREQUAL "")
    message(FATAL_ERROR "SOURCE_DIR is required")
endif()

set(workflow_path "${SOURCE_DIR}/.github/workflows/build-installers.yml")
if(NOT EXISTS "${workflow_path}")
    message(FATAL_ERROR "Installer workflow is missing")
endif()

file(READ "${workflow_path}" workflow)
string(REPLACE "\r\n" "\n" workflow "${workflow}")

function(require_text expected_text description)
    string(FIND "${workflow}" "${expected_text}" match_index)
    if(match_index EQUAL -1)
        message(FATAL_ERROR "Installer workflow is missing ${description}")
    endif()
endfunction()

function(require_file_text relative_path expected_text description)
    set(path "${SOURCE_DIR}/${relative_path}")
    if(NOT EXISTS "${path}")
        message(FATAL_ERROR "Installer artifact policy file is missing: ${relative_path}")
    endif()
    file(READ "${path}" contents)
    string(FIND "${contents}" "${expected_text}" match_index)
    if(match_index EQUAL -1)
        message(FATAL_ERROR "${relative_path} is missing ${description}")
    endif()
endfunction()

function(require_count expected_text expected_count description)
    string(REGEX MATCHALL "${expected_text}" matches "${workflow}")
    list(LENGTH matches actual_count)
    if(NOT actual_count EQUAL expected_count)
        message(FATAL_ERROR
            "Installer workflow has ${actual_count} ${description}; expected ${expected_count}")
    endif()
endfunction()

function(require_job_upload_order job_start job_end success_upload failure_upload)
    string(FIND "${workflow}" "${job_start}" job_start_index)
    if(job_start_index EQUAL -1)
        message(FATAL_ERROR "Installer workflow is missing job ${job_start}")
    endif()

    if("${job_end}" STREQUAL "")
        string(LENGTH "${workflow}" job_end_index)
    else()
        string(FIND "${workflow}" "${job_end}" job_end_index)
        if(job_end_index EQUAL -1 OR job_end_index LESS job_start_index)
            message(FATAL_ERROR "Installer workflow has an invalid boundary for ${job_start}")
        endif()
    endif()

    math(EXPR job_length "${job_end_index} - ${job_start_index}")
    string(SUBSTRING "${workflow}" ${job_start_index} ${job_length} job)
    string(FIND "${job}" "- name: Create exact Corresponding Source archive" source_index)
    string(FIND "${job}" "- name: ${success_upload}" success_index)
    string(FIND "${job}" "- name: ${failure_upload}" failure_index)
    if(source_index EQUAL -1 OR success_index EQUAL -1 OR failure_index EQUAL -1)
        message(FATAL_ERROR "${job_start} is missing a required artifact step")
    endif()
    if(NOT source_index LESS success_index OR NOT success_index LESS failure_index)
        message(FATAL_ERROR
            "${job_start} must create source, attempt the success upload, then retain failure diagnostics")
    endif()
endfunction()

foreach(traceability_file IN ITEMS
        .github/workflows/build-installers.yml
        docs/32-recovered-requirements-traceability.md
        docs/35-rc1-evaluation-guide.md
        docs/safety/traceability-report-2026-10-03-installer-artifact-retention.md
        tests/run_installer_artifact_policy_contract_check.cmake)
    foreach(traceability_id IN ITEMS
            RQ-CF-REL-009
            DQ-installer-artifact-retention-policy
            DV-installer-artifact-retention-policy-contract
            HZ-system-failure-01
            HZ-doc-command-01)
        require_file_text("${traceability_file}" "${traceability_id}"
            "reverse traceability to ${traceability_id}")
    endforeach()
endforeach()

require_count("retention-days: \\$\\{\\{ github.event_name == 'pull_request' && 7 \\|\\| 30 \\}\\}"
    3 "tiered successful-installer retention declarations")
require_count("retention-days: 14" 3 "failure-diagnostic retention declarations")
require_count("if: success\\(\\)" 3 "success-only installer artifact uploads")
require_count("if: failure\\(\\)" 3 "failure-only diagnostic uploads")

require_text("copperfin-windows-installed-ui-failure-" "unique Windows failure artifact")
require_text("copperfin-macos-installer-failure-" "unique macOS failure artifact")
require_text("copperfin-linux-installer-failure-" "unique Linux failure artifact")
require_text("build/package/copperfin-*-Windows.exe" "Windows failure reproduction package")
require_text("build/package/copperfin-*-Darwin.pkg" "macOS failure reproduction package")
require_text("build/package/copperfin-*-Linux.deb" "Linux DEB failure reproduction package")
require_text("build/package/copperfin-*-Linux.rpm" "Linux RPM failure reproduction package")

require_count("if-no-files-found: warn" 3 "fail-open diagnostic uploads")
require_count("if-no-files-found: error" 3 "fail-closed successful installer uploads")

require_job_upload_order(
    "  windows-installer:" "  macos-installer:"
    "Upload Windows installer artifacts"
    "Upload Windows installed-UI failure diagnostics")
require_job_upload_order(
    "  macos-installer:" "  linux-deb-rpm-installers:"
    "Upload macOS installer artifacts"
    "Upload macOS installer failure diagnostics")
require_job_upload_order(
    "  linux-deb-rpm-installers:" ""
    "Upload Linux installer artifacts"
    "Upload Linux installer failure diagnostics")

message(STATUS "Installer artifact retention policy contract passed")
