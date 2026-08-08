# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

cmake_minimum_required(VERSION 3.20)

if(NOT DEFINED SOURCE_DIR OR "${SOURCE_DIR}" STREQUAL "")
    message(FATAL_ERROR "SOURCE_DIR is required")
endif()

function(require_file relative_path)
    if(NOT EXISTS "${SOURCE_DIR}/${relative_path}")
        message(FATAL_ERROR "Required release-licensing file is missing: ${relative_path}")
    endif()
endfunction()

function(require_text relative_path expected_text)
    require_file("${relative_path}")
    file(READ "${SOURCE_DIR}/${relative_path}" contents)
    string(FIND "${contents}" "${expected_text}" offset)
    if(offset EQUAL -1)
        message(FATAL_ERROR "${relative_path} is missing release-licensing text: ${expected_text}")
    endif()
endfunction()

function(require_text_count relative_path expected_text expected_count)
    require_file("${relative_path}")
    file(READ "${SOURCE_DIR}/${relative_path}" contents)
    string(LENGTH "${expected_text}" expected_length)
    string(LENGTH "${contents}" original_length)
    string(REPLACE "${expected_text}" "" stripped_contents "${contents}")
    string(LENGTH "${stripped_contents}" stripped_length)
    math(EXPR removed_length "${original_length} - ${stripped_length}")
    math(EXPR actual_count "${removed_length} / ${expected_length}")
    if(NOT actual_count EQUAL expected_count)
        message(FATAL_ERROR "${relative_path} must contain ${expected_count} occurrences of '${expected_text}'; found ${actual_count}")
    endif()
endfunction()

set(exception_path "LICENSES/LicenseRef-Copperfin-Application-Runtime-Toolchain-Exception-1.0.txt")
set(metadata_path "docs/contracts/release-license-metadata.json")
foreach(required_path IN ITEMS
        LICENSE
        LICENSE.md
        SOURCE.md
        THIRD_PARTY_NOTICES.md
        ${exception_path}
        ${metadata_path}
        docs/34-human-authorship-and-assisted-development.md
        scripts/check-contributor-signoffs.py
        .github/actions/create-release-source/action.yml
        .github/workflows/contributor-signoff.yml)
    require_file("${required_path}")
endforeach()

file(READ "${SOURCE_DIR}/LICENSE" operative_license)
string(REPLACE "\r\n" "\n" operative_license "${operative_license}")
set(exception_marker "        COPPERFIN APPLICATION, RUNTIME, AND TOOLCHAIN EXCEPTION")
string(FIND "${operative_license}" "${exception_marker}" exception_offset)
if(exception_offset EQUAL -1)
    message(FATAL_ERROR "Operative license omits the Copperfin Exception")
endif()
string(SUBSTRING "${operative_license}" ${exception_offset} -1 operative_exception)
file(READ "${SOURCE_DIR}/${exception_path}" extracted_exception)
string(REPLACE "\r\n" "\n" extracted_exception "${extracted_exception}")
if(NOT operative_exception STREQUAL extracted_exception)
    message(FATAL_ERROR "Extracted LicenseRef text differs from the operative Exception in LICENSE")
endif()

file(READ "${SOURCE_DIR}/${metadata_path}" metadata)
string(JSON schema_version ERROR_VARIABLE metadata_error GET "${metadata}" schema_version)
if(metadata_error OR NOT schema_version EQUAL 1)
    message(FATAL_ERROR "Release-license metadata is not valid schema version 1 JSON: ${metadata_error}")
endif()
foreach(metadata_value IN ITEMS
        "GPL-3.0-only"
        "LicenseRef-Copperfin-Application-Runtime-Toolchain-Exception-1.0"
        "false"
        "THIRD_PARTY_NOTICES.md"
        "SOURCE.md")
    string(FIND "${metadata}" "${metadata_value}" metadata_offset)
    if(metadata_offset EQUAL -1)
        message(FATAL_ERROR "Release-license metadata omits ${metadata_value}")
    endif()
endforeach()

require_text("LICENSE" "Standard Support Material is Permitted Output only in the copy and")
require_text("LICENSE" "This rule does not withdraw paragraph 2(c)'s permission")
require_text("LICENSE.md" "Project Copperfin cannot place their")
require_text("LICENSE.md" "downstream distributor to remove an additional")
require_text("CONTRIBUTING.md" "incompatible future license")
require_text("CONTRIBUTING.md" "Commercialization Participation")
require_text("CONTRIBUTING.md" "agreement that states the contributor's")
require_text("GOVERNANCE.md" "does not transfer contributor copyright")
require_text("GOVERNANCE.md" "negotiate compensation afterward")
require_text("docs/34-human-authorship-and-assisted-development.md" "conceived, directed, constrained, reviewed, tested,")
require_text("docs/34-human-authorship-and-assisted-development.md" "does not claim that a prompt alone creates")
require_text("SOURCE.md" "Project-Copperfin-source-<40-character-git-revision>.zip")
require_text("SOURCE.md" "Do not substitute the current `main` branch")
require_text("THIRD_PARTY_NOTICES.md" "This notice may not be removed or altered from any source distribution")
require_text("src/licensing/third_party/ed25519_ref/ed25519.h" "Trimmed from upstream")

foreach(vsix_document IN ITEMS
        LICENSE
        SOURCE.md
        THIRD_PARTY_NOTICES.md
        LicenseRef-Copperfin-Application-Runtime-Toolchain-Exception-1.0.txt
        release-license-metadata.json)
    require_text("vsix/Copperfin.VisualStudio/Copperfin.VisualStudio.csproj" "<Link>${vsix_document}</Link>")
endforeach()
require_text_count("vsix/Copperfin.VisualStudio/Copperfin.VisualStudio.csproj" "<IncludeInVSIX>true</IncludeInVSIX>" 8)

require_text_count(".github/workflows/build-installers.yml" "uses: ./.github/actions/create-release-source" 3)
require_text_count(".github/workflows/build-installers.yml" "release-source/Project-Copperfin-source-\${{ github.sha }}.zip" 3)
require_text_count(".github/workflows/build-vsix.yml" "uses: ./.github/actions/create-release-source" 1)
require_text_count(".github/workflows/build-vsix.yml" "release-source/Project-Copperfin-source-\${{ github.sha }}.zip" 1)
require_text(".github/actions/create-release-source/action.yml" "git archive")
require_text(".github/actions/create-release-source/action.yml" "run_release_source_archive_contract_check.cmake")

require_text(".github/workflows/security-supply-chain.yml" "docs/contracts/release-license-metadata.json")
require_text(".github/workflows/security-supply-chain.yml" "LicenseRef-Copperfin-Application-Runtime-Toolchain-Exception-1.0.txt")
require_text("CMakeLists.txt" "SOURCE.md")
require_text("CMakeLists.txt" "THIRD_PARTY_NOTICES.md")
require_text("CMakeLists.txt" "DESTINATION share/copperfin/licenses")
require_text("CMakeLists.txt" "DESTINATION share/copperfin/contracts")

require_text(".github/workflows/contributor-signoff.yml" "pull_request_target:")
require_text(".github/workflows/contributor-signoff.yml" "Checkout trusted base policy")
require_text(".github/workflows/contributor-signoff.yml" "persist-credentials: false")
require_text(".github/workflows/contributor-signoff.yml" "Fetch untrusted commit objects without checking them out")
require_text(".github/workflows/contributor-signoff.yml" "scripts/check-contributor-signoffs.py")
require_text("CONTRIBUTING.md" "never checks out or")

message(STATUS "Release licensing, source, attribution, and contributor-signoff contract passed")
