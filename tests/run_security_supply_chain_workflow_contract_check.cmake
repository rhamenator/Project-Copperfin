# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only

if(NOT DEFINED SOURCE_DIR OR "${SOURCE_DIR}" STREQUAL "")
    message(FATAL_ERROR "SOURCE_DIR is required")
endif()

set(workflow_path "${SOURCE_DIR}/.github/workflows/security-supply-chain.yml")
if(NOT EXISTS "${workflow_path}")
    message(FATAL_ERROR "Security supply-chain workflow is missing")
endif()

file(READ "${workflow_path}" workflow)
string(REPLACE "\r\n" "\n" workflow "${workflow}")

function(require_text expected_text description)
    string(FIND "${workflow}" "${expected_text}" match_index)
    if(match_index EQUAL -1)
        message(FATAL_ERROR "Security supply-chain workflow is missing ${description}")
    endif()
endfunction()

function(forbid_text forbidden_text description)
    string(FIND "${workflow}" "${forbidden_text}" match_index)
    if(NOT match_index EQUAL -1)
        message(FATAL_ERROR "Security supply-chain workflow contains forbidden ${description}")
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
            "Security supply-chain workflow must contain ${expected_count} ${description}; found ${actual_count}")
    endif()
endfunction()

require_text("permissions:\n      contents: read" "least-privilege contents permission")
forbid_text("security-events:" "unused security-events permission")
require_text("uses: actions/checkout@de0fac2e4500dabe0009e67214ff5f5447ce83dd # v6.0.2"
    "pinned checkout action")
require_text("uses: anchore/sbom-action@e22c389904149dbc22b58101806040fa8d37a610 # v0.24.0"
    "pinned SBOM action")
require_text("syft-version: v1.50.0"
    "explicit available Syft release pin")
require_text("uses: actions/upload-artifact@043fb46d1a93c77aae656e7c1c64a875d1fc6a0a # v7.0.1"
    "pinned authoritative artifact uploader")
require_text("uses: aquasecurity/trivy-action@57a97c7e7821a5776cebc9bb87c984fa69cba8f1 # v0.35.0"
    "pinned Trivy action")
require_text("run: cmake -DSOURCE_DIR=\"$GITHUB_WORKSPACE\" -P tests/run_security_supply_chain_workflow_contract_check.cmake"
    "self-validating workflow contract step")
require_text("upload-artifact: \"false\"" "disabled Anchore artifact upload")
require_text("upload-release-assets: \"false\"" "disabled Anchore release-asset upload")
require_text("name: cyclonedx-sbom" "stable SBOM artifact name")
require_text("path: sbom.cdx.json" "stable SBOM artifact path")
require_text("retention-days: 0" "repository-default artifact retention")
require_text("if-no-files-found: error" "required SBOM file behavior")
require_text_count("uses: actions/upload-artifact@" 1 "authoritative explicit SBOM upload owner")

message(STATUS "Security supply-chain workflow contract check passed")
