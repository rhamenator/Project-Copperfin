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
require_text("${script}" "copperfin_inspect" "installed artifact inspection smoke")
require_text("${script}" "asset_family: program" "semantic artifact-family assertion")
require_text("${script}" "status: ok" "semantic successful-inspection assertion")
require_text("${script}" "/usr/share/copperfin/locales/en-US/strings.json"
    "installed locale-catalog verification")
require_text("${script}" "same_version_maintenance_reinstall"
    "same-version maintenance evidence")
require_text("${script}" "external_artifact_survived"
    "external artifact survival evidence")
require_text("${script}" "linux-installer-lifecycle.json"
    "machine-readable lifecycle evidence")
require_text("${script}" "sha256sum" "exact installer and installed-file binding")
require_text("${script}" "find \"$fixture_root\" -depth -delete"
    "bounded runner-temporary cleanup")
forbid_text("${script}" "rm -rf" "unbounded recursive deletion")

require_text("${workflow}" "name: Exercise Linux DEB installer lifecycle"
    "hosted Debian lifecycle step")
require_text("${workflow}" "scripts/test-linux-installer-lifecycle.sh"
    "lifecycle helper invocation")
require_text("${workflow}" "artifacts/linux-installer-lifecycle/linux-installer-lifecycle.json"
    "retained Linux lifecycle evidence")
require_text("${workflow}" "artifacts/linux-installer-lifecycle/linux-installer-lifecycle.log"
    "retained Linux lifecycle diagnostics")
require_text("${workflow}" "- name: Upload Linux installer artifacts\n        if: always()"
    "failure-path Linux lifecycle diagnostics upload")

require_text("scripts/assemble-rc-candidate.py" "require_linux_installer_lifecycle_evidence"
    "fail-closed Linux lifecycle evidence admission")
require_text("scripts/assemble-rc-candidate.py" "evidence/linux-installer-lifecycle.json"
    "Linux lifecycle evidence bundling")
require_text("docs/contracts/rc-validation-manifest-v3.schema.json"
    "\"linux_deb\": { \"const\": \"PASS\" }"
    "Linux DEB lifecycle PASS schema")
forbid_text("scripts/assemble-rc-candidate.py" "\"linux_deb\": \"NOT_RUN\""
    "hardcoded unperformed Linux DEB lifecycle result")

message(STATUS "Linux installer lifecycle contract passed")
