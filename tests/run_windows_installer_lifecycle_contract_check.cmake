# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

if(NOT DEFINED SOURCE_DIR OR "${SOURCE_DIR}" STREQUAL "")
    message(FATAL_ERROR "SOURCE_DIR is required")
endif()

function(read_contract_file relative_path output_variable)
    set(path "${SOURCE_DIR}/${relative_path}")
    if(NOT EXISTS "${path}")
        message(FATAL_ERROR "Windows installer lifecycle contract file is missing: ${relative_path}")
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

set(script "scripts/test-windows-installer-lifecycle.ps1")
set(workflow ".github/workflows/build-installers.yml")

require_text("${script}" "[ValidateRange(10, 600)]" "bounded process-timeout contract")
require_text("${script}" "WaitForExit($ProcessTimeoutSeconds * 1000)" "bounded child-process wait")
require_text("${script}" "Kill($true)" "timed-out process-tree termination")
require_text("${script}" "Fresh-install root already exists" "fresh-root precondition")
require_text("${script}" "Installation root must be a direct child of RUNNER_TEMP" "runner-temporary-root boundary")
require_text("${script}" "^copperfin-installer-lifecycle-[A-Za-z0-9._-]+$" "installation-root leaf allowlist")
require_text("${script}" "@('/S', \"/D=$resolvedInstallRoot\")" "silent NSIS install with final destination argument")
require_text("${script}" "run_studio_install_contract_check.cmake" "installed Studio tree verification")
require_text("${script}" "run_locale_catalog_install_contract_check.cmake" "installed locale verification")
require_text("${script}" "@('--locale', 'en-US', '--help')" "installed executable smoke")
require_text("${script}" "same-version maintenance reinstall" "maintenance reinstall execution")
require_text("${script}" "upgrade_from_previous_version = 'NOT_RUN'" "honest upgrade limitation")
require_text("${script}" "Get-CopperfinUninstallEntries" "uninstall-registration inspection")
require_text("${script}" "Silent uninstall left installation-root residue" "filesystem residue failure")
require_text("${script}" "Silent uninstall left an uninstall registration" "registry residue failure")
require_text("${script}" "windows-installer-lifecycle.json" "machine-readable retained evidence")
forbid_text("${script}" "Remove-Item" "unbounded direct deletion")
forbid_text("${script}" "Remove-ItemProperty" "registry deletion")
forbid_text("${script}" "Remove-Item -Recurse" "recursive cleanup masking installer residue")

require_text("${workflow}" "name: Exercise Windows installer lifecycle" "hosted lifecycle step")
require_text("${workflow}" "workflow_dispatch:" "manual exact-head validation trigger")
require_text("${workflow}" "copperfin-*-Windows.exe" "exact NSIS artifact selection")
require_text("${workflow}" "-InstallerPath $installer[0].FullName" "selected-installer binding")
require_text("${workflow}" "copperfin-installer-lifecycle-$env:GITHUB_RUN_ID-$env:GITHUB_RUN_ATTEMPT"
    "run-scoped installation root")
require_text("${workflow}" "artifacts/windows-installer-lifecycle/windows-installer-lifecycle.json"
    "retained lifecycle result upload")

message(STATUS "Windows installer lifecycle contract passed")
