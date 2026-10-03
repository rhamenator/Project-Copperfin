# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
# Traceability: RQ-CF-REL-002; RQ-CF-REL-008;
# DQ-windows-installer-lifecycle-scope; DQ-windows-installed-ui-scope;
# DV-windows-installer-lifecycle-contract; DV-windows-installed-ui-contract;
# HZ-system-failure-01;
# HZ-data-corruption-01; HZ-doc-command-01.

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
set(ui_script "scripts/test-windows-installed-studio-ui.ps1")
set(workflow ".github/workflows/build-installers.yml")

foreach(traceability_file IN ITEMS
        CMakeLists.txt
        scripts/assemble-rc-candidate.py
        scripts/test-windows-installer-lifecycle.ps1
        scripts/test-windows-installed-studio-ui.ps1
        docs/contracts/rc-validation-manifest-v3.schema.json
        .github/workflows/build-installers.yml
        docs/35-rc1-evaluation-guide.md
        tests/run_windows_installer_lifecycle_contract_check.cmake)
    foreach(traceability_id IN ITEMS
            RQ-CF-REL-002
            DQ-windows-installer-lifecycle-scope
            DV-windows-installer-lifecycle-contract
            HZ-system-failure-01
            HZ-data-corruption-01
            HZ-doc-command-01)
        require_text("${traceability_file}" "${traceability_id}"
            "reverse traceability to ${traceability_id}")
    endforeach()
endforeach()

foreach(traceability_file IN ITEMS
        scripts/assemble-rc-candidate.py
        scripts/test-windows-installer-lifecycle.ps1
        scripts/test-windows-installed-studio-ui.ps1
        docs/contracts/rc-validation-manifest-v3.schema.json
        .github/workflows/build-installers.yml
        docs/35-rc1-evaluation-guide.md
        docs/32-recovered-requirements-traceability.md
        tests/run_windows_installer_lifecycle_contract_check.cmake)
    foreach(traceability_id IN ITEMS
            RQ-CF-REL-008
            DQ-windows-installed-ui-scope
            DV-windows-installed-ui-contract)
        require_text("${traceability_file}" "${traceability_id}"
            "reverse traceability to ${traceability_id}")
    endforeach()
endforeach()

require_text("${script}" "[ValidateRange(10, 600)]" "bounded process-timeout contract")
require_text("${script}" "WaitForExit($TimeoutSeconds * 1000)" "bounded child-process wait")
require_text("${script}" "Kill($true)" "timed-out process-tree termination")
require_text("${script}" "Fresh-install root already exists" "fresh-root precondition")
require_text("${script}" "Installation root must be a direct child of RUNNER_TEMP" "runner-temporary-root boundary")
require_text("${script}" "^copperfin-installer-lifecycle-[A-Za-z0-9._-]+$" "installation-root leaf allowlist")
require_text("${script}" "@('/S', \"/D=$resolvedInstallRoot\")" "silent NSIS install with final destination argument")
require_text("${script}" "run_studio_install_contract_check.cmake" "installed Studio tree verification")
require_text("${script}" "run_locale_catalog_install_contract_check.cmake" "installed locale verification")
require_text("${script}" "@('--locale', 'en-US', '--help')" "installed executable smoke")
require_text("${script}" "bin\\studio\\Copperfin.Studio.exe" "installed managed Studio identity")
require_text("${script}" "test-windows-installed-studio-ui.ps1" "installed Studio UI helper invocation")
require_text("${script}" "'-NoLogo', '-NoProfile', '-NonInteractive', '-STA'"
    "STA Windows PowerShell UI Automation host")
require_text("${script}" "installed_studio_automated_gui = 'PASS'"
    "installed Studio automated-GUI evidence")
require_text("${script}" "human_gui = 'NOT_RUN'" "honest human-GUI limitation")
require_text("${script}" "windows-installed-studio-ui.json"
    "installed Studio semantic UI evidence handoff")
require_text("${script}" "$installedStudioUiWrapperTimeoutSeconds = $installedStudioUiTimeoutSeconds + 30"
    "installed UI diagnostic and cleanup headroom")
require_text("${script}" "-TimeoutSeconds $installedStudioUiWrapperTimeoutSeconds"
    "installed UI wrapper-specific timeout")
require_text("${script}" "same-version maintenance reinstall" "maintenance reinstall execution")
require_text("${script}" "$upgradeFromPreviousVersionResult = 'NOT_RUN'" "honest upgrade limitation default")
require_text("${script}" "upgrade_from_previous_version = $upgradeFromPreviousVersionResult" "conditional upgrade-result evidence")
require_text("${script}" "$freshInstallResult = 'NOT_RUN'" "honest fresh-install limitation default")
require_text("${script}" "fresh_install = $freshInstallResult" "conditional fresh-install evidence")
require_text("${script}" "Dedicated fresh-install check" "independent current-installer fresh-install exercise")
require_text("${script}" "Invoke-CopperfinInspectArtifactSmoke" "seeded-artifact inspection smoke, not a bare --help")
require_text("${script}" "external artifact outside the install root" "external-artifact survival assertion")
require_text("${script}" "Upgrade must not truncate or corrupt a user" "external-artifact content-fidelity assertion")
require_text("${script}" "Get-CopperfinUninstallEntries" "uninstall-registration inspection")
require_text("${script}" "return @(Get-CopperfinUninstallEntries"
    "strict-mode-safe empty uninstall-entry counting")
require_text("${script}" "Get-CopperfinUninstallEntryCount"
    "centralized uninstall-entry count use")
forbid_text("${script}" "(Get-CopperfinUninstallEntries -ExpectedInstallRoot"
    "strict-mode-unsafe direct uninstall-entry count")
require_text("${script}" "PSObject.Properties[$Name]" "strict-mode-safe optional registry property lookup")
require_text("${script}" "Get-OptionalPropertyValue -InputObject $Entry -Name 'InstallLocation'"
    "optional InstallLocation lookup")
require_text("${script}" "Get-OptionalPropertyValue -InputObject $Entry -Name 'UninstallString'"
    "CPack uninstall-string lookup")
require_text("${script}" "Join-Path $normalizedRoot 'Uninstall.exe'"
    "exact expected CPack uninstaller identity")
require_text("${script}" "Test-NormalizedPathEquals -Candidate $uninstallString -ExpectedPath $expectedUninstaller"
    "exact uninstall-string path comparison")
require_text("${script}" "[System.StringComparison]::OrdinalIgnoreCase"
    "exact case-insensitive CPack uninstall-key identity")
require_text("${script}" "Get-ChildItem -LiteralPath $registryBase -ErrorAction Stop"
    "fail-closed uninstall-key enumeration")
require_text("${script}" "@(Get-ItemProperty -LiteralPath $registryKey.PSPath -ErrorAction Stop)"
    "fail-closed uninstall-entry read")
require_text("${script}" "if ($entryResults.Count -eq 0) { [pscustomobject]@{} }"
    "successful empty registry-key normalization")
forbid_text("${script}" "ErrorAction SilentlyContinue" "suppressed registry-read failure")
require_text("${script}" "Exact CPack uninstall key with cleared values escaped residue detection"
    "sparse exact-key executable regression")
require_text("${workflow}" "CopperfinPackageInstallRegistryKey.txt"
    "generated exact CPack uninstall-key input")
require_text("${workflow}" "UninstallRegistryKeyName = $uninstallRegistryKeyName"
    "exact CPack uninstall-key workflow handoff")
require_text("${workflow}" "PackageVersion           = $packageVersion"
    "current package-version workflow handoff")
require_text("${workflow}" "$lifecycleArguments['PriorPackageVersion'] = $env:PRIOR_PACKAGE_VERSION"
    "prior package-version workflow handoff")
require_text("CMakeLists.txt" "set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY \"\${CPACK_PACKAGE_NAME} \${CPACK_PACKAGE_VERSION}\")"
    "explicit version-bound CPack uninstall-key identity")
require_text("CMakeLists.txt" "CopperfinPackageInstallRegistryKey.txt"
    "generated CPack uninstall-key contract")
forbid_text("${script}" "$_.InstallLocation" "strict-mode-unsafe optional registry property access")
forbid_text("${script}" "$_.UninstallString" "strict-mode-unsafe optional uninstall-string access")
require_text("${script}" "Silent uninstall left installation-root residue" "filesystem residue failure")
require_text("${script}" "Silent uninstall left an uninstall registration" "registry residue failure")
require_text("${script}" "windows-installer-lifecycle.json" "machine-readable retained evidence")
forbid_text("${script}" "Remove-Item" "unbounded direct deletion")
forbid_text("${script}" "Remove-ItemProperty" "registry deletion")
forbid_text("${script}" "Remove-Item -Recurse" "recursive cleanup masking installer residue")

require_text("${ui_script}" "[ValidateRange(10, 300)]" "bounded UI timeout")
require_text("${ui_script}" "AutomationElement]::ProcessIdProperty"
    "exact installed-process UI Automation binding")
require_text("${ui_script}" "if ($process.HasExited)"
    "exit-code access guarded by observed process exit")
require_text("${ui_script}" "[System.Windows.Automation.ControlType]::Pane"
    "currently exposed named Studio surfaces (#6913)")
require_text("${ui_script}" "SendMessageTimeout"
    "bounded native Refresh button action")
require_text("${ui_script}" "'BUTTON'"
    "native Refresh button-class validation")
require_text("${ui_script}" "[System.Security.Cryptography.SHA256]::Create()"
    "Windows PowerShell-compatible evidence hashing")
forbid_text("${ui_script}" "Get-FileHash"
    "module-autoload-dependent evidence hashing")
require_text("${ui_script}" "[System.Windows.Automation.WindowPattern]::Pattern"
    "semantic window-close action")
require_text("${ui_script}" "CopyFromScreen" "failure screenshot capture")
require_text("${ui_script}" "windows-installed-studio-ui-diagnostic.json"
    "failure UI-tree diagnostics")
require_text("${ui_script}" "taskkill.exe\" /PID $process.Id /T /F"
    "bounded failed-process-tree termination")
require_text("${ui_script}" "human_gui = 'NOT_RUN'" "honest human-GUI limitation")
forbid_text("${ui_script}" "SendKeys" "coordinate/keystroke-driven UI automation")
forbid_text("${ui_script}" "mouse_event" "coordinate-driven mouse automation")

find_program(POWERSHELL_EXECUTABLE NAMES pwsh)
if(POWERSHELL_EXECUTABLE)
    execute_process(
        COMMAND "${POWERSHELL_EXECUTABLE}" -NoLogo -NoProfile
            -File "${SOURCE_DIR}/${script}" -SelfTest
        RESULT_VARIABLE helper_result
        OUTPUT_VARIABLE helper_stdout
        ERROR_VARIABLE helper_stderr)
    if(NOT helper_result EQUAL 0)
        message(FATAL_ERROR
            "Windows installer lifecycle helper self-test failed (${helper_result}):\n"
            "stdout=${helper_stdout}\nstderr=${helper_stderr}")
    endif()
    string(FIND "${helper_stdout}" "Windows installer lifecycle helper self-test passed."
        helper_success_offset)
    if(helper_success_offset EQUAL -1)
        message(FATAL_ERROR "Windows installer lifecycle helper self-test omitted its success marker")
    endif()
endif()

require_text("${workflow}" "name: Exercise Windows installer lifecycle" "hosted lifecycle step")
require_text("${workflow}" "workflow_dispatch:" "manual exact-head validation trigger")
require_text("${workflow}" "copperfin-*-Windows.exe" "exact NSIS artifact selection")
require_text("${workflow}" "InstallerPath            = $installer[0].FullName" "selected-installer binding")
forbid_text("${workflow}" "@lifecycleArguments = @(" "array splat binds the lifecycle parameters positionally (#6696)")
require_text("${workflow}" "-DCOPPERFIN_PACKAGE_VERSION_OVERRIDE=0.0.1'" "quoted synthetic prior-version override (#6696)")
require_text("${workflow}" "copperfin-installer-lifecycle-$env:GITHUB_RUN_ID-$env:GITHUB_RUN_ATTEMPT"
    "run-scoped installation root")
require_text("${workflow}" "artifacts/windows-installer-lifecycle/windows-installer-lifecycle.json"
    "retained lifecycle result upload")
require_text("${workflow}" "name: Upload Windows installed-UI failure diagnostics"
    "failure-only UI diagnostic upload")
require_text("${workflow}" "if: failure()" "failure-only UI diagnostic retention")
require_text("${workflow}" "artifacts/windows-installer-lifecycle/windows-installed-studio-ui.json"
    "retained installed Studio UI result upload")

message(STATUS "Windows installer lifecycle contract passed")
