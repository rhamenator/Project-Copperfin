# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
# Traceability: RQ-CF-REL-001; DQ-rc-evidence-v2-scope-separation;
# DV-rc-evidence-v2-workflow-contract; RQ-CF-REL-004;
# DQ-rc-launcher-trust-exception; DV-rc-launcher-trust-exception-contract;
# HZ-system-failure-01; HZ-doc-command-01.

cmake_minimum_required(VERSION 3.20)

if(NOT DEFINED SOURCE_DIR OR "${SOURCE_DIR}" STREQUAL "")
    message(FATAL_ERROR "SOURCE_DIR is required")
endif()

set(workflow_path "${SOURCE_DIR}/.github/workflows/rc-candidate-assembly.yml")
if(NOT EXISTS "${workflow_path}")
    message(FATAL_ERROR "RC candidate assembly workflow is missing")
endif()
file(READ "${workflow_path}" workflow)

set(manifest_schema_path "${SOURCE_DIR}/docs/contracts/rc-validation-manifest-v3.schema.json")
if(NOT EXISTS "${manifest_schema_path}")
    message(FATAL_ERROR "Active RC validation manifest schema is missing")
endif()
file(READ "${manifest_schema_path}" manifest_schema)

function(require_text expected description)
    string(FIND "${workflow}" "${expected}" offset)
    if(offset EQUAL -1)
        message(FATAL_ERROR "RC candidate workflow is missing ${description}: ${expected}")
    endif()
endfunction()

function(require_text_count expected expected_count description)
    string(LENGTH "${expected}" expected_length)
    string(LENGTH "${workflow}" original_length)
    string(REPLACE "${expected}" "" stripped "${workflow}")
    string(LENGTH "${stripped}" stripped_length)
    math(EXPR removed_length "${original_length} - ${stripped_length}")
    math(EXPR actual_count "${removed_length} / ${expected_length}")
    if(NOT actual_count EQUAL expected_count)
        message(FATAL_ERROR
            "RC candidate workflow must contain ${expected_count} ${description}; found ${actual_count}")
    endif()
endfunction()

function(require_manifest_schema_text expected description)
    string(FIND "${manifest_schema}" "${expected}" offset)
    if(offset EQUAL -1)
        message(FATAL_ERROR "Active RC validation manifest schema is missing ${description}: ${expected}")
    endif()
endfunction()

require_text("workflow_dispatch:" "manual-only dispatch")
require_text("RQ-CF-REL-001" "reverse requirement traceability")
require_text("DV-rc-evidence-v2-workflow-contract" "reverse verification traceability")
require_text("candidate_tag:" "explicit candidate tag input")
require_text("expected_tag=\"$COPPERFIN_CANDIDATE_TAG\"" "exact requested RC tag binding")
require_text("^v0\\.1\\.0-rc\\.[1-9][0-9]*$" "sequential RC tag allowlist")
require_text("GITHUB_REF_TYPE\" != \"tag" "tag-ref guard")
require_text("GITHUB_REF_NAME\" != \"$expected_tag" "requested tag/ref equality guard")
require_text("git rev-parse \"refs/tags/$expected_tag^{commit}\"" "peeled tag target check")
require_text("checked_out_revision\" != \"$GITHUB_SHA" "checkout revision check")
require_text("permissions:\n  contents: read" "read-only workflow permissions")
require_text("uses: ./.github/workflows/native-release-readiness.yml" "native release matrix")
require_text("uses: ./.github/workflows/managed-ui-validation-linux.yml" "managed UI validation")
require_text("uses: ./.github/workflows/build-installers.yml" "installer build matrix")
require_text("uses: ./.github/workflows/build-vsix.yml" "Visual Studio VSIX build")
require_text("uses: ./.github/workflows/security-supply-chain.yml" "security and SBOM gate")
require_text("scripts/assemble-rc-candidate.py" "fail-closed bundle assembler")
require_text("--output-dir \"copperfin-$COPPERFIN_CANDIDATE_TAG\"" "validated candidate-specific bundle directory")
require_text("uses: ./.github/actions/create-release-source" "authoritative Corresponding Source generation")
require_text("output-directory: rc-inputs/copperfin-release-source" "isolated Corresponding Source input")
require_text("name: copperfin-\${{ inputs.candidate_tag }}-evaluation-bundle" "candidate-specific evaluation artifact name")
require_text("path: copperfin-\${{ inputs.candidate_tag }}" "candidate-specific evaluation artifact path")
require_text("retention-days: 90" "explicit artifact expiry")
require_text("if-no-files-found: error" "fail-closed artifact upload")
require_text_count("uses: actions/download-artifact@3e5f45b2cfb9172054b4087a40e8e0b5a5461e7c # v8"
    5 "immutable download-artifact pins")
require_text_count("uses: actions/upload-artifact@043fb46d1a93c77aae656e7c1c64a875d1fc6a0a # v7.0.1"
    1 "authoritative RC bundle upload")
require_manifest_schema_text("\"schema_version\": { \"const\": 3 }" "schema-v3 identity")
require_manifest_schema_text("\"windows_installed_cli_smoke\"" "Windows installer lifecycle evidence")
require_manifest_schema_text("\"windows_supported_prg_open_and_command\"" "Windows VSIX lifecycle evidence")
require_manifest_schema_text("\"windows_launcher_release_trust\": { \"const\": \"RC_TEST_EXCEPTION\" }"
    "explicit non-release Windows launcher-trust exception")
require_manifest_schema_text("\"RC_TEST_EXCEPTION\"" "closed RC exception status vocabulary")

set(rc_guide_path "${SOURCE_DIR}/docs/35-rc1-evaluation-guide.md")
file(READ "${rc_guide_path}" rc_guide)
string(REPLACE "\r\n" "\n" rc_guide "${rc_guide}")
string(REPLACE "\n" " " rc_guide_normalized "${rc_guide}")
foreach(required_guide_text IN ITEMS
        "31630819119"
        "signing.windows_launcher_release_trust"
        "RC_TEST_EXCEPTION"
        "does **not** authenticate the contents of this RC bundle"
        "cannot satisfy the Release 1.0 launcher-trust gate"
        "evidence from a different run must not be inferred into the bundle")
    string(FIND "${rc_guide_normalized}" "${required_guide_text}" guide_text_offset)
    if(guide_text_offset EQUAL -1)
        message(FATAL_ERROR
            "RC evaluation guide lacks current launcher-trust scope wording: ${required_guide_text}")
    endif()
endforeach()

foreach(traceability_file IN ITEMS
        scripts/assemble-rc-candidate.py
        docs/contracts/rc-validation-manifest-v3.schema.json
        docs/35-rc1-evaluation-guide.md
        tests/run_rc_candidate_workflow_contract_check.cmake)
    file(READ "${SOURCE_DIR}/${traceability_file}" traceability_contents)
    foreach(traceability_id IN ITEMS
            RQ-CF-REL-001
            DQ-rc-evidence-v2-scope-separation
            RQ-CF-REL-004
            DQ-rc-launcher-trust-exception
            DV-rc-launcher-trust-exception-contract
            HZ-system-failure-01
            HZ-doc-command-01)
        string(FIND "${traceability_contents}" "${traceability_id}" traceability_offset)
        if(traceability_offset EQUAL -1)
            message(FATAL_ERROR
                "${traceability_file} lacks reverse traceability to ${traceability_id}")
        endif()
    endforeach()
endforeach()

foreach(reusable_workflow IN ITEMS
        build-installers.yml
        build-vsix.yml
        managed-ui-validation-linux.yml
        native-release-readiness.yml
        security-supply-chain.yml)
    set(reusable_path "${SOURCE_DIR}/.github/workflows/${reusable_workflow}")
    file(READ "${reusable_path}" reusable_contents)
    string(FIND "${reusable_contents}" "workflow_call:" workflow_call_offset)
    if(workflow_call_offset EQUAL -1)
        message(FATAL_ERROR "${reusable_workflow} is not callable from the RC workflow")
    endif()
endforeach()

foreach(forbidden_text IN ITEMS
        "secrets."
        "environment: release"
        "gh release create"
        "softprops/action-gh-release"
        "BEGIN PRIVATE KEY")
    string(FIND "${workflow}" "${forbidden_text}" forbidden_offset)
    if(NOT forbidden_offset EQUAL -1)
        message(FATAL_ERROR "RC candidate workflow contains forbidden release/secret text: ${forbidden_text}")
    endif()
endforeach()
