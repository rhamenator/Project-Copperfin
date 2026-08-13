# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [ValidateNotNullOrEmpty()]
    [string]$InstallerPath,

    [Parameter(Mandatory = $true)]
    [ValidateNotNullOrEmpty()]
    [string]$BinaryDirectory,

    [Parameter(Mandatory = $true)]
    [ValidateNotNullOrEmpty()]
    [string]$InstallRoot,

    [Parameter(Mandatory = $true)]
    [ValidateNotNullOrEmpty()]
    [string]$EvidenceDirectory,

    [ValidateRange(10, 600)]
    [int]$ProcessTimeoutSeconds = 180
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

function Assert-Condition {
    param(
        [Parameter(Mandatory = $true)]
        [bool]$Condition,
        [Parameter(Mandatory = $true)]
        [string]$Message
    )

    if (-not $Condition) {
        throw $Message
    }
}

function Invoke-BoundedProcess {
    param(
        [Parameter(Mandatory = $true)]
        [string]$FilePath,
        [string[]]$Arguments = @(),
        [Parameter(Mandatory = $true)]
        [string]$Name,
        [switch]$CaptureOutput
    )

    $startInfo = [System.Diagnostics.ProcessStartInfo]::new()
    $startInfo.FileName = $FilePath
    $startInfo.UseShellExecute = $false
    $startInfo.CreateNoWindow = $true
    $startInfo.RedirectStandardOutput = $CaptureOutput.IsPresent
    $startInfo.RedirectStandardError = $CaptureOutput.IsPresent
    foreach ($argument in $Arguments) {
        [void]$startInfo.ArgumentList.Add($argument)
    }

    $process = [System.Diagnostics.Process]::new()
    $process.StartInfo = $startInfo
    try {
        Assert-Condition $process.Start() "$Name did not start."
        $stdoutTask = if ($CaptureOutput) { $process.StandardOutput.ReadToEndAsync() } else { $null }
        $stderrTask = if ($CaptureOutput) { $process.StandardError.ReadToEndAsync() } else { $null }
        if (-not $process.WaitForExit($ProcessTimeoutSeconds * 1000)) {
            try { $process.Kill($true) } catch { Write-Warning "$Name could not be terminated: $($_.Exception.Message)" }
            throw "$Name exceeded the bounded $ProcessTimeoutSeconds-second timeout."
        }
        $stdout = if ($CaptureOutput) { $stdoutTask.GetAwaiter().GetResult() } else { "" }
        $stderr = if ($CaptureOutput) { $stderrTask.GetAwaiter().GetResult() } else { "" }
        if ($process.ExitCode -ne 0) {
            throw "$Name exited with code $($process.ExitCode). stdout='$stdout' stderr='$stderr'"
        }
        return [pscustomobject]@{
            ExitCode = $process.ExitCode
            Stdout = $stdout
            Stderr = $stderr
        }
    }
    finally {
        $process.Dispose()
    }
}

function Get-InstalledSnapshot {
    param([Parameter(Mandatory = $true)][string]$Root)

    $snapshot = [ordered]@{}
    Get-ChildItem -LiteralPath $Root -File -Recurse | Sort-Object FullName | ForEach-Object {
        $relative = [System.IO.Path]::GetRelativePath($Root, $_.FullName).Replace('\', '/')
        $snapshot[$relative] = (Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash.ToLowerInvariant()
    }
    return $snapshot
}

function Get-CopperfinUninstallEntries {
    param([Parameter(Mandatory = $true)][string]$ExpectedInstallRoot)

    $normalizedRoot = [System.IO.Path]::GetFullPath($ExpectedInstallRoot).TrimEnd('\')
    $registryRoots = @(
        'HKCU:\Software\Microsoft\Windows\CurrentVersion\Uninstall\*',
        'HKLM:\Software\Microsoft\Windows\CurrentVersion\Uninstall\*',
        'HKLM:\Software\WOW6432Node\Microsoft\Windows\CurrentVersion\Uninstall\*'
    )
    return @(
        foreach ($registryRoot in $registryRoots) {
            Get-ItemProperty -Path $registryRoot -ErrorAction SilentlyContinue | Where-Object {
                $location = if ($null -eq $_.InstallLocation) { '' } else { [string]$_.InstallLocation }
                $normalizedLocation = $location.Trim('"').TrimEnd('\')
                if ([string]::IsNullOrWhiteSpace($normalizedLocation)) {
                    return $false
                }
                try {
                    return [string]::Equals(
                        [System.IO.Path]::GetFullPath($normalizedLocation),
                        $normalizedRoot,
                        [System.StringComparison]::OrdinalIgnoreCase)
                }
                catch {
                    return $false
                }
            }
        }
    )
}

$resolvedInstaller = (Resolve-Path -LiteralPath $InstallerPath).Path
$resolvedBinaryDirectory = (Resolve-Path -LiteralPath $BinaryDirectory).Path
$resolvedInstallRoot = [System.IO.Path]::GetFullPath($InstallRoot)
$resolvedEvidenceDirectory = [System.IO.Path]::GetFullPath($EvidenceDirectory)
$resolvedRunnerTemporaryRoot = [System.IO.Path]::GetFullPath($env:RUNNER_TEMP).TrimEnd('\')
$installParent = [System.IO.Directory]::GetParent($resolvedInstallRoot)

Assert-Condition ($resolvedInstaller.EndsWith('.exe', [System.StringComparison]::OrdinalIgnoreCase)) `
    "Windows installer must be an executable: $resolvedInstaller"
Assert-Condition ($null -ne $installParent) "Installation root has no parent: $resolvedInstallRoot"
Assert-Condition ([string]::Equals(
        $installParent.FullName.TrimEnd('\'),
        $resolvedRunnerTemporaryRoot,
        [System.StringComparison]::OrdinalIgnoreCase)) `
    "Installation root must be a direct child of RUNNER_TEMP: $resolvedInstallRoot"
Assert-Condition ([System.IO.Path]::GetFileName($resolvedInstallRoot) -match '^copperfin-installer-lifecycle-[A-Za-z0-9._-]+$') `
    "Installation root leaf does not match the lifecycle allowlist: $resolvedInstallRoot"
Assert-Condition (-not (Test-Path -LiteralPath $resolvedInstallRoot)) `
    "Fresh-install root already exists: $resolvedInstallRoot"
Assert-Condition ((Get-CopperfinUninstallEntries -ExpectedInstallRoot $resolvedInstallRoot).Count -eq 0) `
    "Fresh-install root already has an uninstall registration: $resolvedInstallRoot"

New-Item -ItemType Directory -Path $resolvedEvidenceDirectory -Force | Out-Null
$installedSnapshot = $null
$maintenanceSnapshot = $null
$uninstallRegistrationCount = 0
$inspectOutput = ""

try {
    Invoke-BoundedProcess `
        -FilePath $resolvedInstaller `
        -Arguments @('/S', "/D=$resolvedInstallRoot") `
        -Name 'Copperfin silent fresh installation' | Out-Null

    Assert-Condition (Test-Path -LiteralPath $resolvedInstallRoot -PathType Container) `
        "Installer did not create the requested installation root: $resolvedInstallRoot"
    Assert-Condition (-not ((Get-Item -LiteralPath $resolvedInstallRoot).Attributes -band [System.IO.FileAttributes]::ReparsePoint)) `
        "Installer created a reparse-point installation root: $resolvedInstallRoot"

    Invoke-BoundedProcess `
        -FilePath 'cmake' `
        -Arguments @(
            "-DBINARY_DIR=$resolvedBinaryDirectory",
            "-DINSTALL_ROOT=$resolvedInstallRoot",
            '-P', (Join-Path $PSScriptRoot '..\tests\run_studio_install_contract_check.cmake')
        ) `
        -Name 'installed Studio tree verification' | Out-Null
    Invoke-BoundedProcess `
        -FilePath 'cmake' `
        -Arguments @(
            "-DINSTALL_ROOT=$resolvedInstallRoot",
            '-P', (Join-Path $PSScriptRoot '..\tests\run_locale_catalog_install_contract_check.cmake')
        ) `
        -Name 'installed locale catalog verification' | Out-Null

    $inspectPath = Join-Path $resolvedInstallRoot 'bin\copperfin_inspect.exe'
    $inspectResult = Invoke-BoundedProcess `
        -FilePath $inspectPath `
        -Arguments @('--locale', 'en-US', '--help') `
        -Name 'installed copperfin_inspect smoke' `
        -CaptureOutput
    Assert-Condition ($inspectResult.Stdout -match 'copperfin_inspect') `
        "Installed copperfin_inspect help omitted its command identity."
    Assert-Condition ([string]::IsNullOrEmpty($inspectResult.Stderr)) `
        "Installed copperfin_inspect help wrote unexpected stderr: $($inspectResult.Stderr)"
    $inspectOutput = $inspectResult.Stdout

    $uninstallRegistrationCount = (Get-CopperfinUninstallEntries -ExpectedInstallRoot $resolvedInstallRoot).Count
    Assert-Condition ($uninstallRegistrationCount -eq 1) `
        "Fresh installation must create exactly one uninstall registration for its root; found $uninstallRegistrationCount."

    $installedSnapshot = Get-InstalledSnapshot -Root $resolvedInstallRoot
    Assert-Condition ($installedSnapshot.Count -gt 0) 'Fresh installation produced an empty file inventory.'

    Invoke-BoundedProcess `
        -FilePath $resolvedInstaller `
        -Arguments @('/S', "/D=$resolvedInstallRoot") `
        -Name 'Copperfin same-version maintenance reinstall' | Out-Null
    $maintenanceSnapshot = Get-InstalledSnapshot -Root $resolvedInstallRoot
    Assert-Condition (($installedSnapshot | ConvertTo-Json -Compress) -ceq ($maintenanceSnapshot | ConvertTo-Json -Compress)) `
        'Same-version maintenance reinstall changed the installed file inventory or hashes.'
    Assert-Condition ((Get-CopperfinUninstallEntries -ExpectedInstallRoot $resolvedInstallRoot).Count -eq 1) `
        'Same-version maintenance reinstall did not preserve exactly one uninstall registration.'

    $uninstaller = Join-Path $resolvedInstallRoot 'Uninstall.exe'
    Assert-Condition (Test-Path -LiteralPath $uninstaller -PathType Leaf) `
        "Installed uninstaller is missing: $uninstaller"
    Invoke-BoundedProcess `
        -FilePath $uninstaller `
        -Arguments @('/S') `
        -Name 'Copperfin silent uninstall' | Out-Null

    $deadline = [DateTime]::UtcNow.AddSeconds($ProcessTimeoutSeconds)
    while ((Test-Path -LiteralPath $resolvedInstallRoot) -and [DateTime]::UtcNow -lt $deadline) {
        Start-Sleep -Milliseconds 250
    }
    Assert-Condition (-not (Test-Path -LiteralPath $resolvedInstallRoot)) `
        "Silent uninstall left installation-root residue: $resolvedInstallRoot"
    Assert-Condition ((Get-CopperfinUninstallEntries -ExpectedInstallRoot $resolvedInstallRoot).Count -eq 0) `
        "Silent uninstall left an uninstall registration for: $resolvedInstallRoot"

    $evidence = [ordered]@{
        schema_version = 1
        kind = 'copperfin-windows-installer-lifecycle-result'
        installer_sha256 = (Get-FileHash -LiteralPath $resolvedInstaller -Algorithm SHA256).Hash.ToLowerInvariant()
        install_root = $resolvedInstallRoot
        fresh_install = 'PASS'
        installed_tree_contract = 'PASS'
        locale_catalog_contract = 'PASS'
        installed_cli_smoke = 'PASS'
        same_version_maintenance_reinstall = 'PASS'
        upgrade_from_previous_version = 'NOT_RUN'
        silent_uninstall = 'PASS'
        install_root_residue = 'PASS'
        uninstall_registration_residue = 'PASS'
        uninstall_registration_count_after_install = $uninstallRegistrationCount
        installed_file_count = $installedSnapshot.Count
        installed_cli_stdout = $inspectOutput.TrimEnd()
    }
    $evidence | ConvertTo-Json -Depth 4 | Set-Content `
        -LiteralPath (Join-Path $resolvedEvidenceDirectory 'windows-installer-lifecycle.json') `
        -Encoding utf8NoBOM
    Write-Host "Windows installer lifecycle passed for $resolvedInstaller"
}
finally {
    if (Test-Path -LiteralPath $resolvedInstallRoot) {
        $fallbackUninstaller = Join-Path $resolvedInstallRoot 'Uninstall.exe'
        if (Test-Path -LiteralPath $fallbackUninstaller -PathType Leaf) {
            try {
                Invoke-BoundedProcess -FilePath $fallbackUninstaller -Arguments @('/S') -Name 'fallback silent uninstall' | Out-Null
            }
            catch {
                Write-Warning "Fallback uninstall failed: $($_.Exception.Message)"
            }
        }
    }
}
