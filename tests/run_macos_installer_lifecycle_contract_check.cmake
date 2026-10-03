# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
# Traceability: RQ-CF-REL-007; DQ-macos-installer-lifecycle-scope;
# DV-macos-installer-lifecycle-contract; HZ-system-failure-01;
# HZ-data-corruption-01; HZ-doc-command-01.

if(NOT DEFINED SOURCE_DIR OR "${SOURCE_DIR}" STREQUAL "")
    message(FATAL_ERROR "SOURCE_DIR is required")
endif()

function(read_contract_file relative_path output_variable)
    set(path "${SOURCE_DIR}/${relative_path}")
    if(NOT EXISTS "${path}")
        message(FATAL_ERROR "macOS installer lifecycle contract file is missing: ${relative_path}")
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

set(script "scripts/test-macos-installer-lifecycle.sh")
set(workflow ".github/workflows/build-installers.yml")

foreach(traceability_file IN ITEMS
        CMakeLists.txt
        ${script}
        ${workflow}
        scripts/assemble-rc-candidate.py
        docs/contracts/rc-validation-manifest-v3.schema.json
        docs/32-recovered-requirements-traceability.md
        docs/35-rc1-evaluation-guide.md
        tests/run_macos_installer_lifecycle_contract_check.cmake)
    foreach(traceability_id IN ITEMS
            RQ-CF-REL-007
            DQ-macos-installer-lifecycle-scope
            DV-macos-installer-lifecycle-contract
            HZ-system-failure-01
            HZ-data-corruption-01
            HZ-doc-command-01)
        require_text("${traceability_file}" "${traceability_id}"
            "reverse traceability to ${traceability_id}")
    endforeach()
endforeach()

require_text("CMakeLists.txt"
    [=[set(CPACK_PRODUCTBUILD_IDENTIFIER "com.Copperfin.copperfin")]=]
    "stable productbuild identifier")
require_text("CMakeLists.txt" "CopperfinProductBuildIdentifier.txt"
    "generated productbuild identifier handoff")

require_text("${script}" "python3 - \"$process_timeout_seconds\""
    "bounded package and installed-product execution")
require_text("${script}" "start_new_session=True"
    "bounded process-tree isolation")
require_text("${script}" "os.killpg(process.pid, signal.SIGKILL)"
    "timed-out process-tree termination")
require_text("${script}" [=[installer -pkg "$package_path" -target /]=]
    "real productbuild package installation")
require_text("${script}" [=[pkgutil --expand "$package_path" "$expanded_package"]=]
    "outer distribution expansion")
require_text("${script}" [=[done <"$component_boms"]=]
    "nested component-BOM iteration")
require_text("${script}" [=[lsbom -s "$component_bom"]=]
    "nested component pre-install payload inventory")
require_text("${script}" [=[package / "Bom"]=]
    "regular component-BOM validation")
require_text("${script}" "expected_suffixes = (\"-Documentation.pkg\", \"-Unspecified.pkg\")"
    "exact nested component-package set")
require_text("${script}" "pkgutil --pkg-info"
    "component receipt verification")
require_text("${script}" "pkgutil --forget"
    "exact receipt cleanup")
require_text("${script}" "GITHUB_ACTIONS"
    "GitHub Actions mutation confinement")
require_text("${script}" "RUNNER_ENVIRONMENT"
    "GitHub-hosted runner confinement")
require_text("${script}" "--allow-system-mutation"
    "explicit system-mutation acknowledgement")
require_text("${script}" "entry == \"Applications\" or entry.startswith(\"Applications/\")"
    "payload-root confinement")
require_text("${script}"
    "Refusing to overwrite a pre-existing package-specific directory"
    "package-specific pre-install refusal")
require_text("${script}" "/Applications/bin/copperfin_inspect"
    "installed artifact inspection smoke")
require_text("${script}" "asset_family: program"
    "semantic artifact-family assertion")
require_text("${script}" "status: ok"
    "semantic successful-inspection assertion")
require_text("${script}" "/Applications/share/copperfin/locales/en-US/strings.json"
    "installed English locale-catalog verification")
require_text("${script}" "component_receipt_contract"
    "component-receipt evidence")
require_text("${script}" "developer_id_and_notarization"
    "truthful signing and notarization limitation")
require_text("${script}" "automated_gui"
    "truthful automated-GUI limitation")
require_text("${script}" "human_gui"
    "truthful human-GUI limitation")
require_text("${script}"
    "Package-created directory remains after bounded runner cleanup"
    "package-created directory residue rejection")
require_text("${script}" "macos-installer-lifecycle.json"
    "machine-readable lifecycle evidence")
require_text("${script}" "shasum -a 256"
    "exact package and external-fixture binding")
require_text("${script}" [=[find "$fixture_root" -depth -delete]=]
    "bounded runner-temporary cleanup")
forbid_text("${script}" "rm -rf" "recursive system deletion")

require_text("${workflow}" "name: Exercise macOS productbuild installer lifecycle"
    "hosted productbuild lifecycle step")
require_text("${workflow}" "scripts/test-macos-installer-lifecycle.sh"
    "lifecycle helper invocation")
require_text("${workflow}" "CopperfinProductBuildIdentifier.txt"
    "generated product identifier handoff")
require_text("${workflow}" "--allow-system-mutation"
    "explicit hosted system-mutation acknowledgement")
require_text("${workflow}" "\"GITHUB_ACTIONS=$GITHUB_ACTIONS\""
    "root-process GitHub Actions identity forwarding")
require_text("${workflow}" "\"RUNNER_ENVIRONMENT=$RUNNER_ENVIRONMENT\""
    "root-process hosted-runner identity forwarding")
require_text("${workflow}"
    "artifacts/macos-installer-lifecycle/macos-installer-lifecycle.json"
    "retained macOS lifecycle evidence")
require_text("${workflow}"
    "artifacts/macos-installer-lifecycle/macos-installer-lifecycle.log"
    "retained macOS lifecycle diagnostics")
require_text("${workflow}"
    "- name: Upload macOS installer artifacts\n        if: always()"
    "failure-path macOS lifecycle diagnostics upload")

require_text("scripts/assemble-rc-candidate.py"
    "require_macos_installer_lifecycle_evidence"
    "fail-closed macOS lifecycle evidence admission")
require_text("scripts/assemble-rc-candidate.py"
    "evidence/macos-installer-lifecycle.json"
    "macOS lifecycle evidence bundling")
read_contract_file("scripts/assemble-rc-candidate.py" assembler_contents)
string(FIND "${assembler_contents}"
    "def require_macos_installer_lifecycle_evidence" macos_validator_start)
string(FIND "${assembler_contents}"
    "def require_windows_vsix_lifecycle_evidence" macos_validator_end)
if(macos_validator_start EQUAL -1 OR macos_validator_end EQUAL -1 OR
   macos_validator_end LESS_EQUAL macos_validator_start)
    message(FATAL_ERROR "macOS lifecycle evidence validator boundaries are missing")
endif()
math(EXPR macos_validator_length "${macos_validator_end} - ${macos_validator_start}")
string(SUBSTRING "${assembler_contents}" ${macos_validator_start}
    ${macos_validator_length} macos_validator)
foreach(required_validator_text IN ITEMS
        "\"component_receipts\""
        "\"developer_id_and_notarization\""
        "\"automated_gui\""
        "\"human_gui\""
        "\"asset_family: program\""
        "\"status: ok\"")
    string(FIND "${macos_validator}" "${required_validator_text}" validator_offset)
    if(validator_offset EQUAL -1)
        message(FATAL_ERROR
            "macOS lifecycle evidence validator lacks ${required_validator_text}")
    endif()
endforeach()
require_text("docs/contracts/rc-validation-manifest-v3.schema.json"
    "\"macos_productbuild\": { \"const\": \"PASS\" }"
    "macOS productbuild lifecycle PASS schema")
forbid_text("scripts/assemble-rc-candidate.py"
    "\"macos_productbuild\": \"NOT_RUN\""
    "hardcoded unperformed macOS productbuild lifecycle result")

message(STATUS "macOS installer lifecycle contract passed")
