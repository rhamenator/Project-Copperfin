# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

if(NOT DEFINED SOURCE_DIR OR "${SOURCE_DIR}" STREQUAL "")
    message(FATAL_ERROR "SOURCE_DIR is required")
endif()

file(GLOB_RECURSE action_files
    "${SOURCE_DIR}/.github/workflows/*.yml"
    "${SOURCE_DIR}/.github/workflows/*.yaml"
    "${SOURCE_DIR}/.github/actions/*/action.yml"
    "${SOURCE_DIR}/.github/actions/*/action.yaml")
if(NOT action_files)
    message(FATAL_ERROR "No GitHub Actions workflow or local action files were found")
endif()

set(external_action_count 0)
foreach(action_file IN LISTS action_files)
    file(STRINGS "${action_file}" action_lines REGEX "^[ \t]*uses:[ \t]*")
    foreach(action_line IN LISTS action_lines)
        if(action_line MATCHES "^[ \t]*uses:[ \t]*\\./")
            continue()
        endif()

        if(NOT action_line MATCHES "^[ \t]*uses:[ \t]*([^ \t]+)@([^ \t]+)")
            message(FATAL_ERROR
                "External GitHub Action reference is malformed in ${action_file}: ${action_line}")
        endif()
        set(action_name "${CMAKE_MATCH_1}")
        set(action_ref "${CMAKE_MATCH_2}")
        if(NOT action_name MATCHES "^[A-Za-z0-9_.-]+/[A-Za-z0-9_.-]+$")
            message(FATAL_ERROR
                "External GitHub Action must use owner/repository syntax in ${action_file}: ${action_line}")
        endif()
        string(LENGTH "${action_ref}" action_ref_length)
        if(NOT action_ref_length EQUAL 40 OR
                NOT action_ref MATCHES "^[0-9A-Fa-f]+$")
            message(FATAL_ERROR
                "External GitHub Action must use a full commit SHA in ${action_file}: ${action_line}")
        endif()
        if(NOT action_line MATCHES "#[ \t]*v[0-9]")
            message(FATAL_ERROR
                "Pinned external GitHub Action must retain its release tag comment in ${action_file}: ${action_line}")
        endif()
        math(EXPR external_action_count "${external_action_count} + 1")
    endforeach()
endforeach()

if(external_action_count EQUAL 0)
    message(FATAL_ERROR "No external GitHub Actions were found to audit")
endif()

set(non_product_trigger_filter [=[on:
  push:
    branches: [main]
    paths-ignore:
      - ".agent-channel/**"
      - "docs/**"
      - "**/*.md"
      - "**/*.txt"
  pull_request:
    branches: [main]
    paths-ignore:
      - ".agent-channel/**"
      - "docs/**"
      - "**/*.md"
      - "**/*.txt"
]=])
foreach(filtered_workflow IN ITEMS
        .github/workflows/build-installers.yml
        .github/workflows/build-vsix.yml
        .github/workflows/managed-ui-validation-linux.yml
        .github/workflows/security-supply-chain.yml)
    file(READ "${SOURCE_DIR}/${filtered_workflow}" filtered_contents)
    string(REPLACE "\r\n" "\n" filtered_contents "${filtered_contents}")
    string(FIND "${filtered_contents}" "${non_product_trigger_filter}" filter_index)
    if(filter_index EQUAL -1)
        message(FATAL_ERROR
            "${filtered_workflow} must ignore docs, text, and agent-channel-only changes")
    endif()
endforeach()

file(READ
    "${SOURCE_DIR}/.github/workflows/generated-launcher-validation.yml"
    generated_launcher_workflow)
string(REPLACE "\r\n" "\n" generated_launcher_workflow
    "${generated_launcher_workflow}")
foreach(required_text IN ITEMS
        "branches: [main, v1-development]"
        "samples/polyglot-dotnet-candidate/**"
        "samples/polyglot-python-sidecar/**"
        "tests/test_polyglot_dotnet_candidate.cpp"
        "tests/test_polyglot_python_sidecar.cpp"
        "test_generated_launcher_process test_polyglot_dotnet_candidate"
        "test_generated_launcher_posix_process test_polyglot_dotnet_candidate"
        "test_polyglot_dotnet_candidate test_polyglot_python_sidecar"
        "test_generated_launcher_process|test_polyglot_dotnet_candidate"
        "test_generated_launcher_posix_process|test_polyglot_dotnet_candidate"
        "test_polyglot_dotnet_candidate|test_polyglot_python_sidecar")
    string(FIND "${generated_launcher_workflow}" "${required_text}" required_index)
    if(required_index EQUAL -1)
        message(FATAL_ERROR
            "Generated-launcher workflow is missing polyglot candidate evidence: ${required_text}")
    endif()
endforeach()

file(READ
    "${SOURCE_DIR}/.github/actions/native-validation/action.yml"
    shared_native_action)
foreach(required_text IN ITEMS
        "Run macOS SET POINT locale matrix"
        [=[if: ${{ inputs.platform == 'macos' }}]=]
        "test_prg_engine_control_flow|test_prg_engine_string_math_functions"
        "C en_US.UTF-8 pt_BR.UTF-8 de_DE.UTF-8"
        "LC_ALL=\"$locale\"")
    string(FIND "${shared_native_action}" "${required_text}" required_index)
    if(required_index EQUAL -1)
        message(FATAL_ERROR
            "Shared native validation is missing macOS SET POINT locale evidence: ${required_text}")
    endif()
endforeach()

file(GLOB workflow_files
    "${SOURCE_DIR}/.github/workflows/*.yml"
    "${SOURCE_DIR}/.github/workflows/*.yaml")
foreach(workflow_file IN LISTS workflow_files)
    file(READ "${workflow_file}" workflow)
    string(REPLACE "\r\n" "\n" workflow "${workflow}")
    if(NOT workflow MATCHES "(^|\n)[ \t]*permissions:[ \t]*\n")
        message(FATAL_ERROR "Workflow lacks an explicit permissions block: ${workflow_file}")
    endif()
    if(workflow MATCHES "(^|\n)[ \t]+[A-Za-z0-9_-]+:[ \t]*write([ \t]*\n|$)")
        message(FATAL_ERROR "Workflow grants an unsupported write permission: ${workflow_file}")
    endif()
endforeach()

message(STATUS
    "GitHub Actions contract passed: ${external_action_count} external actions are immutably pinned and workflows are read-only")
