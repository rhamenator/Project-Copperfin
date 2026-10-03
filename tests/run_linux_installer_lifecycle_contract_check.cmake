# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
# Traceability: RQ-CF-REL-006; DQ-linux-installer-lifecycle-scope;
# DV-linux-installer-lifecycle-contract; HZ-system-failure-01;
# HZ-data-corruption-01; HZ-doc-command-01.

if(NOT DEFINED SOURCE_DIR OR "${SOURCE_DIR}" STREQUAL "")
    message(FATAL_ERROR "SOURCE_DIR is required")
endif()

function(read_contract_file relative_path output_variable)
    set(path "${SOURCE_DIR}/${relative_path}")
    if(NOT EXISTS "${path}")
        message(FATAL_ERROR "Linux installer lifecycle contract file is missing: ${relative_path}")
    endif()
    file(READ "${path}" contents)
    string(REPLACE "\r\n" "\n" contents "${contents}")
    set(${output_variable} "${contents}" PARENT_SCOPE)
endfunction()

function(require_text relative_path expected_text description)
    read_contract_file("${relative_path}" contents)
    string(FIND "${contents}" "${expected_text}" match_index)
    if(match_index EQUAL -1)
        message(FATAL_ERROR "${relative_path} is missing ${description}")
    endif()
endfunction()

function(forbid_text relative_path forbidden_text description)
    read_contract_file("${relative_path}" contents)
    string(FIND "${contents}" "${forbidden_text}" match_index)
    if(NOT match_index EQUAL -1)
        message(FATAL_ERROR "${relative_path} contains forbidden ${description}")
    endif()
endfunction()

function(require_text_count relative_path expected_text expected_count description)
    read_contract_file("${relative_path}" contents)
    string(LENGTH "${expected_text}" expected_length)
    string(LENGTH "${contents}" original_length)
    string(REPLACE "${expected_text}" "" stripped "${contents}")
    string(LENGTH "${stripped}" stripped_length)
    math(EXPR removed_length "${original_length} - ${stripped_length}")
    math(EXPR actual_count "${removed_length} / ${expected_length}")
    if(NOT actual_count EQUAL expected_count)
        message(FATAL_ERROR
            "${relative_path} must contain ${expected_count} ${description}; found ${actual_count}")
    endif()
endfunction()

set(script "scripts/test-linux-installer-lifecycle.sh")
set(workflow ".github/workflows/build-installers.yml")

foreach(traceability_file IN ITEMS
        ${script}
        ${workflow}
        scripts/assemble-rc-candidate.py
        docs/contracts/rc-validation-manifest-v3.schema.json
        docs/32-recovered-requirements-traceability.md
        tests/run_linux_installer_lifecycle_contract_check.cmake)
    foreach(traceability_id IN ITEMS
            RQ-CF-REL-006
            DQ-linux-installer-lifecycle-scope
            DV-linux-installer-lifecycle-contract
            HZ-system-failure-01
            HZ-data-corruption-01
            HZ-doc-command-01)
        require_text("${traceability_file}" "${traceability_id}"
            "reverse traceability to ${traceability_id}")
    endforeach()
endforeach()

require_text("${script}" "timeout --signal=KILL" "bounded package and process execution")
require_text("${script}" "dpkg --install" "real Debian package installation")
require_text("${script}" "dpkg --purge" "real Debian package removal")
require_text("${script}" "dpkg-query --show" "package database verification")
require_text_count("${script}"
    "if dpkg-query --show \"$package_name\" >/dev/null 2>&1; then"
    2 "fail-closed full package-record checks")
forbid_text("${script}" "db:Status-Abbrev"
    "partial package-status admission")
require_text("${script}" "GITHUB_ACTIONS" "GitHub Actions mutation confinement")
require_text("${script}" "RUNNER_ENVIRONMENT" "GitHub-hosted runner confinement")
require_text("${script}" "--allow-system-mutation" "explicit system-mutation acknowledgement")
require_text("${script}" "copperfin_inspect" "installed artifact inspection smoke")
require_text("${script}" "asset_family: program" "semantic artifact-family assertion")
require_text("${script}" "status: ok" "semantic successful-inspection assertion")
require_text("${script}" "/usr/share/copperfin/locales/en-US/strings.json"
    "installed locale-catalog verification")
require_text("${script}" "english_locale_catalog"
    "accurately scoped English locale evidence")
forbid_text("${script}" "locale_catalog_contract"
    "overbroad Linux locale-catalog evidence")
require_text("${script}" "same_version_maintenance_reinstall"
    "same-version maintenance evidence")
require_text("${script}" "external_artifact_survived"
    "external artifact survival evidence")
require_text("${script}" "linux-installer-lifecycle.json"
    "machine-readable lifecycle evidence")
require_text("${script}" "schema_version: 2"
    "DEB-only lifecycle evidence schema")
forbid_text("${script}" "rpm_lifecycle"
    "cross-package RPM claim in DEB evidence")
require_text("${script}" "sha256sum" "exact installer and installed-file binding")
require_text("${script}" "installed-directories.txt"
    "package-specific directory residue inventory")
require_text("${script}" "Package-specific directory remains after purge"
    "package-specific directory residue rejection")
require_text("${script}" "find \"$fixture_root\" -depth -delete"
    "bounded runner-temporary cleanup")
forbid_text("${script}" "rm -rf" "unbounded recursive deletion")

require_text("${workflow}" "name: Exercise Linux DEB installer lifecycle"
    "hosted Debian lifecycle step")
require_text("${workflow}" "scripts/test-linux-installer-lifecycle.sh"
    "lifecycle helper invocation")
require_text("${workflow}" "--allow-system-mutation"
    "explicit hosted system-mutation acknowledgement")
require_text("${workflow}" "\"GITHUB_ACTIONS=$GITHUB_ACTIONS\""
    "root-process GitHub Actions identity forwarding")
require_text("${workflow}" "\"RUNNER_ENVIRONMENT=$RUNNER_ENVIRONMENT\""
    "root-process hosted-runner identity forwarding")
require_text("${workflow}" "artifacts/linux-installer-lifecycle/linux-installer-lifecycle.json"
    "retained Linux lifecycle evidence")
require_text("${workflow}" "artifacts/linux-installer-lifecycle/linux-installer-lifecycle.log"
    "retained Linux lifecycle diagnostics")
require_text("${workflow}" "- name: Upload Linux installer failure diagnostics\n        if: failure()"
    "failure-only Linux lifecycle diagnostics upload")
require_text("${workflow}" "- name: Upload Linux installer artifacts\n        if: success()"
    "success-only Linux installer upload")

require_text("scripts/assemble-rc-candidate.py" "require_linux_installer_lifecycle_evidence"
    "fail-closed Linux lifecycle evidence admission")
require_text("scripts/assemble-rc-candidate.py" "evidence/linux-installer-lifecycle.json"
    "Linux lifecycle evidence bundling")
read_contract_file("scripts/assemble-rc-candidate.py" assembler_contents)
string(FIND "${assembler_contents}"
    "def require_linux_installer_lifecycle_evidence" linux_validator_start)
string(FIND "${assembler_contents}"
    "def require_windows_vsix_lifecycle_evidence" linux_validator_end)
if(linux_validator_start EQUAL -1 OR linux_validator_end EQUAL -1 OR
   linux_validator_end LESS_EQUAL linux_validator_start)
    message(FATAL_ERROR "Linux lifecycle evidence validator boundaries are missing")
endif()
math(EXPR linux_validator_length "${linux_validator_end} - ${linux_validator_start}")
string(SUBSTRING "${assembler_contents}" ${linux_validator_start}
    ${linux_validator_length} linux_validator)
string(FIND "${linux_validator}" "\"english_locale_catalog\""
    english_locale_key_offset)
string(FIND "${linux_validator}" "\"locale_catalog_contract\""
    overbroad_locale_key_offset)
if(english_locale_key_offset EQUAL -1 OR NOT overbroad_locale_key_offset EQUAL -1)
    message(FATAL_ERROR
        "Linux lifecycle evidence validator must use only english_locale_catalog")
endif()
require_text("docs/contracts/rc-validation-manifest-v3.schema.json"
    "\"linux_deb\": { \"const\": \"PASS\" }"
    "Linux DEB lifecycle PASS schema")
forbid_text("scripts/assemble-rc-candidate.py" "\"linux_deb\": \"NOT_RUN\""
    "hardcoded unperformed Linux DEB lifecycle result")

message(STATUS "Linux installer lifecycle contract passed")
