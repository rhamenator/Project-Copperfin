# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
# Traceability: RQ-CF-REL-003; DQ-windows-vsix-lifecycle-scope;
# DV-windows-vsix-lifecycle-contract; HZ-system-failure-01;
# HZ-data-corruption-01; HZ-doc-command-01.

if(NOT DEFINED SOURCE_DIR)
    message(FATAL_ERROR "SOURCE_DIR is required")
endif()

function(require_text relative_path expected_text description)
    file(STRINGS "${SOURCE_DIR}/${relative_path}" lines)
    set(found FALSE)
    foreach(line IN LISTS lines)
        string(FIND "${line}" "${expected_text}" match_index)
        if(NOT match_index EQUAL -1)
            set(found TRUE)
            break()
        endif()
    endforeach()
    if(NOT found)
        message(FATAL_ERROR "${relative_path} is missing ${description}")
    endif()
endfunction()

set(script "scripts/test-windows-vsix-lifecycle.ps1")
set(workflow ".github/workflows/build-vsix.yml")
foreach(path IN ITEMS
        ${script}
        ${workflow}
        scripts/assemble-rc-candidate.py
        tests/run_windows_vsix_lifecycle_contract_check.cmake)
    foreach(id IN ITEMS RQ-CF-REL-003 DQ-windows-vsix-lifecycle-scope
            DV-windows-vsix-lifecycle-contract HZ-system-failure-01
            HZ-data-corruption-01 HZ-doc-command-01)
        require_text("${path}" "${id}" "reverse traceability to ${id}")
    endforeach()
endforeach()

require_text("${script}" "[ValidateRange(30, 600)]" "bounded process timeout")
require_text("${script}" "ProcessTimeoutSeconds * 1000" "bounded VSIXInstaller wait")
require_text("${script}" "Kill($true)" "timed-out process-tree termination")
require_text("${script}" "-requires Microsoft.VisualStudio.Component.CoreEditor" "Visual Studio component selection")
require_text("${script}" "\"/instanceIds:$instanceId\"" "exact Visual Studio instance targeting")
require_text("${script}" "'/quiet', \"/instanceIds:$instanceId\"" "noninteractive instance-scoped VSIX operation")
require_text("${script}" "installationVersion -match" "version-independent Visual Studio profile selection")
require_text("${script}" "Copperfin VSIX is already installed" "clean-runner precondition")
require_text("${script}" "'/updateconfiguration'" "post-install package-registration refresh")
require_text("${script}" "Copperfin Command" "representative installed command")
require_text("${script}" "MainWindowHandle" "bounded IDE readiness observation")
require_text("${script}" "UIAutomationClient" "Windows UI Automation evidence boundary")
require_text("${script}" "ProcessIdProperty, $ExpectedProcessId" "launched-process UI identity binding")
require_text("${script}" "StartsWith(\"$ExpectedName - \"" "exact active-document window-title boundary")
require_text("${script}" "Exact runner-owned PRG document tab" "exact PRG-open proof")
require_text("${script}" "invoke-visual-studio-menu-command.ps1" "registered-menu command helper")
require_text("${script}" "-MenuName', 'Tools'" "exact Visual Studio menu boundary")
require_text("${script}" "-CommandName', 'Copperfin Command'" "exact registered command caption")
require_text("${script}" "[System.Windows.Automation.InvokePattern]::Pattern" "user-visible command invocation")
require_text("${script}" "Registered Copperfin Command surface" "same-process command-surface proof")
require_text("${script}" "'/Edit', $fixturePrg" "explicit running-IDE document-open request")
require_text("${script}" "lifecycle-smoke.prg" "runner-owned PRG open fixture")
require_text("${script}" "End package load [CopperfinPackage]" "explicit successful package-load proof")
require_text("${script}" "MatchingErrors.Count -eq 0" "Copperfin package-load error rejection")
require_text("${script}" "'/uninstall:Copperfin.VisualStudio'" "exact extension uninstall identity")
require_text("${script}" "same_version_reinstall = 'NOT_RUN'" "honest maintenance limitation")
require_text("${script}" "upgrade_from_previous_version = 'NOT_RUN'" "honest upgrade limitation")
require_text("${script}" "disablement = 'NOT_RUN'" "honest disablement limitation")
require_text("${script}" "WorkingDirectory = $resolvedEvidenceDirectory" "checkout-independent IDE working directory")
require_text("${script}" "development_checkout_dependency = 'PASS'" "checkout-independence evidence")
require_text("${script}" "windows-vsix-lifecycle.json" "machine-readable retained evidence")
require_text("${workflow}" "workflow_dispatch:" "manual exact-head validation trigger")
require_text("${workflow}" "runs-on: windows-2022" "stable VSSDK-17 lifecycle host")
require_text("${workflow}" "Exercise Windows VSIX lifecycle" "hosted lifecycle step")
require_text("${workflow}" "/nodeReuse:false" "MSBuild descendant shutdown before installation")
require_text("${workflow}" "windows-vsix-lifecycle.json" "retained lifecycle evidence upload")
require_text("${workflow}" "Upload Windows VSIX lifecycle diagnostics" "retained failure diagnostics")
require_text("${workflow}" "always()" "failure-path evidence staging")
file(READ "${SOURCE_DIR}/${script}" script_contents)
string(FIND "${script_contents}" "VSIX lifecycle phase: invoke registered Copperfin command through Tools menu" command_phase_offset)
string(FIND "${script_contents}" "VSIX lifecycle phase: open runner-owned PRG through running IDE" document_phase_offset)
if(command_phase_offset EQUAL -1 OR document_phase_offset EQUAL -1 OR
        NOT command_phase_offset LESS document_phase_offset)
    message(FATAL_ERROR "registered package command must load before the PRG is opened through the running IDE")
endif()
file(READ "${SOURCE_DIR}/${workflow}" workflow_contents)
string(FIND "${workflow_contents}" "Exercise Windows VSIX lifecycle" lifecycle_offset)
string(FIND "${workflow_contents}" "Run managed VSIX behavior tests" managed_test_offset)
if(lifecycle_offset EQUAL -1 OR managed_test_offset EQUAL -1 OR
        NOT lifecycle_offset LESS managed_test_offset)
    message(FATAL_ERROR "VSIX lifecycle must run before managed test processes can remain")
endif()
require_text("scripts/assemble-rc-candidate.py" "require_windows_vsix_lifecycle_evidence" "fail-closed RC evidence ingestion")
require_text("docs/contracts/rc-validation-manifest-v3.schema.json" "windows_disablement" "manifest disablement field")

find_program(POWERSHELL_EXECUTABLE NAMES pwsh)
if(POWERSHELL_EXECUTABLE)
    execute_process(COMMAND "${POWERSHELL_EXECUTABLE}" -NoLogo -NoProfile
        -File "${SOURCE_DIR}/${script}" -SelfTest
        RESULT_VARIABLE result OUTPUT_VARIABLE stdout ERROR_VARIABLE stderr)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "VSIX lifecycle helper self-test failed (${result}):\n${stdout}\n${stderr}")
    endif()
endif()

message(STATUS "Windows VSIX lifecycle contract passed")
