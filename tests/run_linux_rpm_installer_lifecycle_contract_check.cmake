# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
# Traceability: RQ-CF-REL-006; DQ-linux-rpm-installer-lifecycle-scope;
# DV-linux-rpm-installer-lifecycle-contract; HZ-system-failure-01;
# HZ-data-corruption-01; HZ-doc-command-01.

if(NOT DEFINED SOURCE_DIR OR "${SOURCE_DIR}" STREQUAL "")
    message(FATAL_ERROR "SOURCE_DIR is required")
endif()

function(read_contract_file relative_path output_variable)
    set(path "${SOURCE_DIR}/${relative_path}")
    if(NOT EXISTS "${path}")
        message(FATAL_ERROR "Linux RPM lifecycle contract file is missing: ${relative_path}")
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

set(script "scripts/test-linux-rpm-installer-lifecycle.sh")
set(workflow ".github/workflows/build-installers.yml")

foreach(traceability_file IN ITEMS
        ${script}
        ${workflow}
        scripts/assemble-rc-candidate.py
        docs/contracts/rc-validation-manifest-v3.schema.json
        docs/32-recovered-requirements-traceability.md
        tests/run_linux_rpm_installer_lifecycle_contract_check.cmake)
    foreach(traceability_id IN ITEMS
            RQ-CF-REL-006
            DQ-linux-rpm-installer-lifecycle-scope
            DV-linux-rpm-installer-lifecycle-contract
            HZ-system-failure-01
            HZ-data-corruption-01
            HZ-doc-command-01)
        require_text("${traceability_file}" "${traceability_id}"
            "reverse traceability to ${traceability_id}")
    endforeach()
endforeach()

require_text("${script}" "COPPERFIN_RPM_CONTAINER" "dedicated container authority gate")
require_text("${script}" "/.dockerenv" "container identity check")
require_text("${script}" "docker\\.io/library/fedora@sha256:" "digest-pinned Fedora identity")
require_text("${script}" "--allow-container-mutation" "explicit container-mutation acknowledgement")
require_text("${script}" "timeout --signal=KILL" "bounded package and process execution")
require_text("${script}" "rpm --install" "native RPM package installation")
require_text("${script}" "rpm --replacepkgs --install" "native RPM same-version reinstall")
require_text("${script}" "rpm --verify" "native RPM payload verification")
require_text("${script}" "rpm --erase" "native RPM package removal")
require_text("${script}" "rpm --query" "native RPM package database verification")
require_text("${script}" "copperfin_inspect" "installed artifact inspection smoke")
require_text("${script}" "asset_family: program" "semantic artifact-family assertion")
require_text("${script}" "status: ok" "semantic successful-inspection assertion")
require_text("${script}" "/usr/share/copperfin/locales/en-US/strings.json"
    "installed locale-catalog verification")
require_text("${script}" "linux-rpm-installer-lifecycle.json"
    "machine-readable RPM lifecycle evidence")
require_text("${script}" "CONTAINER_IMAGE_REFERENCE" "container reference evidence binding")
require_text("${script}" "CONTAINER_IMAGE_ID" "resolved container image evidence binding")
require_text("${script}" "sha256sum" "exact installer and installed-file binding")
require_text("${script}" "find \"$fixture_root\" -depth -delete"
    "bounded container-temporary cleanup")
forbid_text("${script}" "rm -rf" "unbounded recursive deletion")

require_text("${workflow}" "name: Exercise Linux RPM installer lifecycle"
    "hosted RPM container lifecycle step")
require_text("${workflow}" "docker.io/library/fedora@sha256:"
    "digest-pinned Fedora container")
require_text("${workflow}"
    "docker.io/library/fedora@sha256:99e203b80b1c3d8f7e161ec10a68fd02b081ef83a3963553e513c82846b97814"
    "exact admitted Fedora container digest")
require_text("${workflow}" "--platform linux/amd64" "explicit package architecture")
require_text("${workflow}" "--pull=never" "post-pull digest-only container execution")
require_text("${workflow}" "timeout --signal=KILL 60 docker image inspect"
    "bounded resolved-container identity lookup")
require_text("${workflow}" "--cidfile" "daemon-owned container identity capture")
require_text("${workflow}" "trap cleanup_rpm_container EXIT"
    "all-path daemon-container cleanup")
require_text("${workflow}" "docker rm --force"
    "forced daemon-container cleanup after client timeout")
require_text("${workflow}" "timeout --signal=KILL 60 docker rm --force"
    "separately bounded daemon-container cleanup")
require_text("${workflow}" "scripts/test-linux-rpm-installer-lifecycle.sh"
    "RPM lifecycle helper invocation")
require_text("${workflow}" "--allow-container-mutation"
    "explicit container-mutation acknowledgement")
require_text("${workflow}" "artifacts/linux-rpm-installer-lifecycle/linux-rpm-installer-lifecycle.json"
    "retained RPM lifecycle evidence")
require_text("${workflow}" "artifacts/linux-rpm-installer-lifecycle/linux-rpm-installer-lifecycle.log"
    "retained RPM lifecycle diagnostics")

require_text("scripts/assemble-rc-candidate.py" "require_linux_rpm_installer_lifecycle_evidence"
    "fail-closed RPM lifecycle evidence admission")
require_text("scripts/assemble-rc-candidate.py"
    "99e203b80b1c3d8f7e161ec10a68fd02b081ef83a3963553e513c82846b97814"
    "exact Fedora container digest admission")
require_text("scripts/assemble-rc-candidate.py" "evidence/linux-rpm-installer-lifecycle.json"
    "RPM lifecycle evidence bundling")
require_text("docs/contracts/rc-validation-manifest-v3.schema.json"
    "\"linux_rpm\": { \"const\": \"PASS\" }"
    "Linux RPM lifecycle PASS schema")
forbid_text("scripts/assemble-rc-candidate.py" "\"linux_rpm\": \"NOT_RUN\""
    "hardcoded unperformed Linux RPM lifecycle result")

message(STATUS "Linux RPM installer lifecycle contract passed")
