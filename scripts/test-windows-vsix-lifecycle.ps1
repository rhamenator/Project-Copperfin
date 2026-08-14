# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
# Traceability: RQ-CF-REL-003; DQ-windows-vsix-lifecycle-scope;
# DV-windows-vsix-lifecycle-contract; HZ-system-failure-01;
# HZ-data-corruption-01; HZ-doc-command-01.

[CmdletBinding()]
param(
    [Parameter(Mandatory = $true, ParameterSetName = 'Lifecycle')]
    [ValidateNotNullOrEmpty()]
    [string]$VsixPath,

    [Parameter(Mandatory = $true, ParameterSetName = 'Lifecycle')]
    [ValidateNotNullOrEmpty()]
    [string]$EvidenceDirectory,

    [Parameter(Mandatory = $true, ParameterSetName = 'Lifecycle')]
    [ValidateNotNullOrEmpty()]
    [string]$DriverVsixPath,

    [Parameter(ParameterSetName = 'Lifecycle')]
    [ValidateRange(30, 600)]
    [int]$ProcessTimeoutSeconds = 360,

    [Parameter(ParameterSetName = 'Lifecycle')]
    [ValidateRange(60, 600)]
    [int]$InstallerTimeoutSeconds = 600,

    [Parameter(Mandatory = $true, ParameterSetName = 'SelfTest')]
    [switch]$SelfTest
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

function Assert-Condition {
    param([bool]$Condition, [string]$Message)
    if (-not $Condition) { throw $Message }
}

function Invoke-BoundedProcess {
    param(
        [Parameter(Mandatory = $true)][string]$FilePath,
        [string[]]$Arguments = @(),
        [Parameter(Mandatory = $true)][string]$Name,
        [ValidateRange(30, 600)][int]$TimeoutSeconds = $ProcessTimeoutSeconds
    )

    $startInfo = [System.Diagnostics.ProcessStartInfo]::new()
    $startInfo.FileName = $FilePath
    $startInfo.UseShellExecute = $false
    $startInfo.CreateNoWindow = $true
    $startInfo.RedirectStandardOutput = $true
    $startInfo.RedirectStandardError = $true
    foreach ($argument in $Arguments) { [void]$startInfo.ArgumentList.Add($argument) }
    $process = [System.Diagnostics.Process]::new()
    $process.StartInfo = $startInfo
    try {
        Assert-Condition $process.Start() "$Name did not start."
        $stdoutTask = $process.StandardOutput.ReadToEndAsync()
        $stderrTask = $process.StandardError.ReadToEndAsync()
        if (-not $process.WaitForExit($TimeoutSeconds * 1000)) {
            try { $process.Kill($true) } catch { Write-Warning "$Name could not be terminated: $($_.Exception.Message)" }
            throw "$Name exceeded the bounded $TimeoutSeconds-second timeout."
        }
        $stdout = $stdoutTask.GetAwaiter().GetResult()
        $stderr = $stderrTask.GetAwaiter().GetResult()
        if ($process.ExitCode -ne 0) {
            throw "$Name exited with code $($process.ExitCode). stdout='$stdout' stderr='$stderr'"
        }
        return [pscustomobject]@{ Stdout = $stdout; Stderr = $stderr }
    }
    finally { $process.Dispose() }
}

function Invoke-RecordedInstallerOperation {
    param(
        [Parameter(Mandatory = $true)][string[]]$Arguments,
        [Parameter(Mandatory = $true)][string]$Name,
        [Parameter(Mandatory = $true)][string]$Operation,
        [Parameter(Mandatory = $true)][string]$TargetExtensionId
    )

    $startedAt = [DateTime]::UtcNow
    $outcome = 'ERROR'
    $diagnosticMessage = ''
    try {
        $result = Invoke-BoundedProcess -FilePath $vsixInstaller -Arguments $Arguments -Name $Name `
            -TimeoutSeconds $InstallerTimeoutSeconds
        $outcome = 'PASS'
        return $result
    }
    catch {
        $diagnosticMessage = $_.Exception.Message
        throw
    }
    finally {
        $finishedAt = [DateTime]::UtcNow
        $installerOperations.Add([pscustomobject][ordered]@{
            operation = $Operation
            outcome = $outcome
            started_at_utc = $startedAt.ToString('o')
            finished_at_utc = $finishedAt.ToString('o')
            duration_seconds = [Math]::Round(($finishedAt - $startedAt).TotalSeconds, 3)
            timeout_seconds = $InstallerTimeoutSeconds
            target_extension_id = $TargetExtensionId
            diagnostic = $diagnosticMessage
        })
        [ordered]@{
            schema_version = 1
            kind = 'copperfin-windows-vsix-installer-operations'
            vsix_sha256 = $vsixSha256
            lifecycle_driver_vsix_sha256 = $driverVsixSha256
            visual_studio_instance_id = $instanceId
            extension_id = $packageIdentity.Id
            lifecycle_driver_extension_id = $driverIdentity.Id
            operations = @($installerOperations)
        } | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath $installerDiagnosticsPath -Encoding utf8NoBOM
    }
}

function Read-VsixIdentity {
    param([Parameter(Mandatory = $true)][string]$ManifestPath)
    [xml]$manifest = Get-Content -LiteralPath $ManifestPath -Raw -ErrorAction Stop
    $namespace = [System.Xml.XmlNamespaceManager]::new($manifest.NameTable)
    $namespace.AddNamespace('v', 'http://schemas.microsoft.com/developer/vsx-schema/2011')
    $identity = $manifest.SelectSingleNode('/v:PackageManifest/v:Metadata/v:Identity', $namespace)
    Assert-Condition ($null -ne $identity) "VSIX manifest has no identity: $ManifestPath"
    return [pscustomobject]@{
        Id = [string]$identity.Id
        Version = [string]$identity.Version
    }
}

function Get-CopperfinPackageLoadState {
    param([Parameter(Mandatory = $true)][string]$ActivityLogPath)
    [xml]$activity = Get-Content -LiteralPath $ActivityLogPath -Raw -ErrorAction Stop
    $packageGuid = '{1DE4E419-0DE5-4FB7-9C0F-C0212D97D4A5}'
    $successfulLoad = $false
    $matchingErrors = @()
    foreach ($entry in @($activity.SelectNodes('/activity/entry'))) {
        $type = [string]$entry.type
        $description = [string]$entry.description
        $guidNode = $entry.SelectSingleNode('guid')
        $pathNode = $entry.SelectSingleNode('path')
        $recordNode = $entry.SelectSingleNode('record')
        $guid = if ($null -eq $guidNode) { '' } else { [string]$guidNode.InnerText }
        $path = if ($null -eq $pathNode) { '' } else { [string]$pathNode.InnerText }
        $record = if ($null -eq $recordNode) { '?' } else { [string]$recordNode.InnerText }
        if ($type -eq 'Information' -and
                $description -eq 'End package load [CopperfinPackage]' -and
                $guid.Equals($packageGuid, [System.StringComparison]::OrdinalIgnoreCase)) {
            $successfulLoad = $true
        }
        if ($type -eq 'Error' -and
                ($guid.Equals($packageGuid, [System.StringComparison]::OrdinalIgnoreCase) -or
                 $description -match '(?i)Copperfin' -or
                 $path -match '(?i)Copperfin')) {
            $matchingErrors += "record=$record description='$description'"
        }
    }
    return [pscustomobject]@{
        SuccessfulLoad = $successfulLoad
        MatchingErrors = $matchingErrors
    }
}

function Get-ExtensionDirectories {
    param(
        [Parameter(Mandatory = $true)][string]$VisualStudioInstanceId,
        [Parameter(Mandatory = $true)][string]$VisualStudioRegistryVersion,
        [Parameter(Mandatory = $true)][string]$ExtensionId
    )
    $visualStudioRoot = Join-Path $env:LOCALAPPDATA 'Microsoft\VisualStudio'
    if (-not (Test-Path -LiteralPath $visualStudioRoot -PathType Container)) { return @() }
    return @(
        Get-ChildItem -LiteralPath $visualStudioRoot -Directory -ErrorAction Stop |
            Where-Object { $_.Name -like "$VisualStudioRegistryVersion`_$VisualStudioInstanceId*" } |
            ForEach-Object {
                $extensionRoot = Join-Path $_.FullName 'Extensions'
                if (Test-Path -LiteralPath $extensionRoot -PathType Container) {
                    Get-ChildItem -LiteralPath $extensionRoot -Filter 'extension.vsixmanifest' -File -Recurse -ErrorAction Stop |
                        Where-Object {
                            try { (Read-VsixIdentity -ManifestPath $_.FullName).Id -eq $ExtensionId }
                            catch { throw "Installed VSIX manifest could not be read: $($_.FullName): $($_.Exception.Message)" }
                        } |
                        ForEach-Object { $_.Directory.FullName }
                }
            } |
            Sort-Object -Unique
    )
}

if ($SelfTest) {
    $fixtureRoot = Join-Path ([System.IO.Path]::GetTempPath()) `
        "copperfin-vsix-lifecycle-self-test-$([Guid]::NewGuid().ToString('N'))"
    $manifestPath = Join-Path $fixtureRoot 'extension.vsixmanifest'
    [System.IO.Directory]::CreateDirectory($fixtureRoot) | Out-Null
    try {
        @'
<PackageManifest Version="2.0.0" xmlns="http://schemas.microsoft.com/developer/vsx-schema/2011">
  <Metadata><Identity Id="Copperfin.VisualStudio" Version="0.1.0" Language="en-US" Publisher="Copperfin" /></Metadata>
</PackageManifest>
'@ | Set-Content -LiteralPath $manifestPath -Encoding utf8NoBOM
        $identity = Read-VsixIdentity -ManifestPath $manifestPath
        Assert-Condition ($identity.Id -eq 'Copperfin.VisualStudio') 'Self-test rejected the expected VSIX identity.'
        Assert-Condition ($identity.Version -eq '0.1.0') 'Self-test rejected the expected VSIX version.'
        $successfulLog = Join-Path $fixtureRoot 'successful-activity.xml'
        $failedLog = Join-Path $fixtureRoot 'failed-activity.xml'
        @'
<activity><entry><record>1</record><type>Information</type><source>VisualStudio</source><description>End package load [CopperfinPackage]</description><guid>{1DE4E419-0DE5-4FB7-9C0F-C0212D97D4A5}</guid></entry></activity>
'@ | Set-Content -LiteralPath $successfulLog -Encoding utf8NoBOM
        @'
<activity><entry><record>2</record><type>Information</type><source>VisualStudio</source><description>End package load [CopperfinPackage]</description><guid>{1DE4E419-0DE5-4FB7-9C0F-C0212D97D4A5}</guid></entry><entry><record>3</record><type>Error</type><source>VisualStudio</source><description>CreateInstance failed for CopperfinPackage</description><guid>{1DE4E419-0DE5-4FB7-9C0F-C0212D97D4A5}</guid></entry></activity>
'@ | Set-Content -LiteralPath $failedLog -Encoding utf8NoBOM
        $successfulState = Get-CopperfinPackageLoadState -ActivityLogPath $successfulLog
        $failedState = Get-CopperfinPackageLoadState -ActivityLogPath $failedLog
        Assert-Condition ($successfulState.SuccessfulLoad -and $successfulState.MatchingErrors.Count -eq 0) `
            'Self-test rejected an explicit successful Copperfin package-load record.'
        Assert-Condition ($failedState.SuccessfulLoad -and $failedState.MatchingErrors.Count -eq 1) `
            'Self-test did not preserve a Copperfin error alongside a nominal success record.'
    }
    finally { [System.IO.Directory]::Delete($fixtureRoot, $true) }
    Write-Host 'Windows VSIX lifecycle helper self-test passed.'
    return
}

$resolvedVsix = (Resolve-Path -LiteralPath $VsixPath).Path
$resolvedDriverVsix = (Resolve-Path -LiteralPath $DriverVsixPath).Path
$resolvedEvidenceDirectory = [System.IO.Path]::GetFullPath($EvidenceDirectory)
$resolvedRunnerTemporaryRoot = [System.IO.Path]::GetFullPath($env:RUNNER_TEMP).TrimEnd('\')
$evidenceParent = [System.IO.Directory]::GetParent($resolvedEvidenceDirectory)
Assert-Condition ($resolvedVsix.EndsWith('.vsix', [System.StringComparison]::OrdinalIgnoreCase)) `
    "Lifecycle input must be a VSIX: $resolvedVsix"
Assert-Condition ($resolvedDriverVsix.EndsWith('.vsix', [System.StringComparison]::OrdinalIgnoreCase)) `
    "Lifecycle-driver input must be a VSIX: $resolvedDriverVsix"
Assert-Condition (-not [string]::Equals($resolvedVsix, $resolvedDriverVsix,
        [System.StringComparison]::OrdinalIgnoreCase)) `
    'Product and lifecycle-driver VSIX paths must differ.'
Assert-Condition ($null -ne $evidenceParent) "Evidence directory has no parent: $resolvedEvidenceDirectory"
Assert-Condition ([string]::Equals($evidenceParent.FullName.TrimEnd('\'), $resolvedRunnerTemporaryRoot,
        [System.StringComparison]::OrdinalIgnoreCase)) `
    "Evidence directory must be a direct child of RUNNER_TEMP: $resolvedEvidenceDirectory"
Assert-Condition ([System.IO.Path]::GetFileName($resolvedEvidenceDirectory) -match '^copperfin-vsix-lifecycle-[A-Za-z0-9._-]+$') `
    "Evidence-directory leaf does not match the lifecycle allowlist: $resolvedEvidenceDirectory"
Assert-Condition (-not (Test-Path -LiteralPath $resolvedEvidenceDirectory)) `
    "Evidence directory already exists: $resolvedEvidenceDirectory"

$archiveRoot = Join-Path $resolvedEvidenceDirectory 'package'
$driverArchiveRoot = Join-Path $resolvedEvidenceDirectory 'driver-package'
[System.IO.Directory]::CreateDirectory($archiveRoot) | Out-Null
[System.IO.Directory]::CreateDirectory($driverArchiveRoot) | Out-Null
[System.IO.Compression.ZipFile]::ExtractToDirectory($resolvedVsix, $archiveRoot)
[System.IO.Compression.ZipFile]::ExtractToDirectory($resolvedDriverVsix, $driverArchiveRoot)
$packageIdentity = Read-VsixIdentity -ManifestPath (Join-Path $archiveRoot 'extension.vsixmanifest')
Assert-Condition ($packageIdentity.Id -eq 'Copperfin.VisualStudio') `
    "Unexpected VSIX identity: $($packageIdentity.Id)"
$driverIdentity = Read-VsixIdentity -ManifestPath (Join-Path $driverArchiveRoot 'extension.vsixmanifest')
Assert-Condition ($driverIdentity.Id -eq 'Copperfin.VisualStudio.LifecycleDriver') `
    "Unexpected lifecycle-driver VSIX identity: $($driverIdentity.Id)"

$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
Assert-Condition (Test-Path -LiteralPath $vswhere -PathType Leaf) "vswhere.exe is unavailable: $vswhere"
$instanceJson = (& $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.CoreEditor -format json) -join "`n"
Assert-Condition ($LASTEXITCODE -eq 0) 'vswhere.exe failed.'
$instances = @($instanceJson | ConvertFrom-Json)
Assert-Condition ($instances.Count -eq 1) "Expected exactly one selected Visual Studio instance, found $($instances.Count)."
$instance = $instances[0]
$instanceId = [string]$instance.instanceId
$installationPath = [string]$instance.installationPath
$installationVersion = [string]$instance.installationVersion
Assert-Condition ($installationVersion -match '^(?<major>[0-9]+)\.') `
    "Visual Studio installation version is invalid: $installationVersion"
$registryVersion = "$($Matches.major).0"
$devenv = Join-Path $installationPath 'Common7\IDE\devenv.exe'
$vsixInstaller = Join-Path $installationPath 'Common7\IDE\VSIXInstaller.exe'
Assert-Condition (Test-Path -LiteralPath $devenv -PathType Leaf) "devenv.exe is unavailable: $devenv"
Assert-Condition (Test-Path -LiteralPath $vsixInstaller -PathType Leaf) "VSIXInstaller.exe is unavailable: $vsixInstaller"

$existing = @(Get-ExtensionDirectories `
        -VisualStudioInstanceId $instanceId `
        -VisualStudioRegistryVersion $registryVersion `
        -ExtensionId $packageIdentity.Id)
Assert-Condition ($existing.Count -eq 0) 'Copperfin VSIX is already installed in the selected runner instance.'
$existingDriver = @(Get-ExtensionDirectories `
        -VisualStudioInstanceId $instanceId `
        -VisualStudioRegistryVersion $registryVersion `
        -ExtensionId $driverIdentity.Id)
Assert-Condition ($existingDriver.Count -eq 0) `
    'Copperfin lifecycle-driver VSIX is already installed in the selected runner instance.'
$vsixSha256 = (Get-FileHash -LiteralPath $resolvedVsix -Algorithm SHA256).Hash.ToLowerInvariant()
$driverVsixSha256 = (Get-FileHash -LiteralPath $resolvedDriverVsix -Algorithm SHA256).Hash.ToLowerInvariant()
$registrationActivityLog = Join-Path $resolvedEvidenceDirectory 'ActivityLog-registration.xml'
$activityLog = Join-Path $resolvedEvidenceDirectory 'ActivityLog.xml'
$fixturePrg = Join-Path $resolvedEvidenceDirectory 'lifecycle-smoke.prg'
$installerDiagnosticsPath = Join-Path $resolvedEvidenceDirectory 'vsix-installer-operations.json'
$driverResultPath = Join-Path $resolvedEvidenceDirectory 'vsix-lifecycle-driver.json'
$installerOperations = [System.Collections.Generic.List[object]]::new()
[System.IO.File]::WriteAllText($fixturePrg, "? 'Copperfin VSIX lifecycle smoke'`r`n", [System.Text.UTF8Encoding]::new($false))

$installedDirectory = $null
$installedDriverDirectory = $null
$registrationProcess = $null
$ideProcess = $null
$primaryFailure = $null
$cleanupFailure = $null
$driverCleanupFailure = $null
$residueInventoryFailure = $null
try {
    Write-Host 'VSIX lifecycle phase: install'
    Invoke-RecordedInstallerOperation -Arguments @(
        '/quiet', "/instanceIds:$instanceId", $resolvedVsix
    ) -Name 'Copperfin VSIX installation' -Operation 'install' `
        -TargetExtensionId $packageIdentity.Id | Out-Null

    Write-Host 'VSIX lifecycle phase: install test-only in-process driver'
    Invoke-RecordedInstallerOperation -Arguments @(
        '/quiet', "/instanceIds:$instanceId", $resolvedDriverVsix
    ) -Name 'Copperfin lifecycle-driver VSIX installation' -Operation 'driver_install' `
        -TargetExtensionId $driverIdentity.Id | Out-Null

    $installed = @(Get-ExtensionDirectories `
            -VisualStudioInstanceId $instanceId `
            -VisualStudioRegistryVersion $registryVersion `
            -ExtensionId $packageIdentity.Id)
    Assert-Condition ($installed.Count -eq 1) "Expected one installed Copperfin VSIX directory, found $($installed.Count)."
    $installedDirectory = $installed[0]
    $installedIdentity = Read-VsixIdentity -ManifestPath (Join-Path $installedDirectory 'extension.vsixmanifest')
    Assert-Condition ($installedIdentity.Version -eq $packageIdentity.Version) `
        "Installed VSIX version '$($installedIdentity.Version)' differs from package '$($packageIdentity.Version)'."
    Assert-Condition (Test-Path -LiteralPath (Join-Path $installedDirectory 'Copperfin.VisualStudio.dll') -PathType Leaf) `
        'Installed VSIX is missing Copperfin.VisualStudio.dll.'
    $installedDrivers = @(Get-ExtensionDirectories `
            -VisualStudioInstanceId $instanceId `
            -VisualStudioRegistryVersion $registryVersion `
            -ExtensionId $driverIdentity.Id)
    Assert-Condition ($installedDrivers.Count -eq 1) `
        "Expected one installed lifecycle-driver VSIX directory, found $($installedDrivers.Count)."
    $installedDriverDirectory = $installedDrivers[0]
    $installedDriverIdentity = Read-VsixIdentity -ManifestPath `
        (Join-Path $installedDriverDirectory 'extension.vsixmanifest')
    Assert-Condition ($installedDriverIdentity.Version -eq $driverIdentity.Version) `
        "Installed lifecycle-driver version '$($installedDriverIdentity.Version)' differs from package '$($driverIdentity.Version)'."
    Assert-Condition (Test-Path -LiteralPath `
            (Join-Path $installedDriverDirectory 'Copperfin.VisualStudio.LifecycleDriver.dll') -PathType Leaf) `
        'Installed lifecycle-driver VSIX is missing its package assembly.'

    Write-Host 'VSIX lifecycle phase: refresh Visual Studio package registration'
    Invoke-BoundedProcess `
        -FilePath $devenv `
        -Arguments @('/updateconfiguration') `
        -Name 'Visual Studio package-registration refresh' | Out-Null

    Write-Host 'VSIX lifecycle phase: prime per-user package registration'
    $registrationStartInfo = [System.Diagnostics.ProcessStartInfo]::new()
    $registrationStartInfo.FileName = $devenv
    $registrationStartInfo.UseShellExecute = $false
    $registrationStartInfo.WorkingDirectory = $resolvedEvidenceDirectory
    foreach ($argument in @('/NoSplash', '/Log', $registrationActivityLog)) {
        [void]$registrationStartInfo.ArgumentList.Add($argument)
    }
    $registrationProcess = [System.Diagnostics.Process]::Start($registrationStartInfo)
    Assert-Condition ($null -ne $registrationProcess) 'Visual Studio registration-prime launch did not start.'
    $registrationDeadline = [DateTime]::UtcNow.AddSeconds($ProcessTimeoutSeconds)
    while ([DateTime]::UtcNow -lt $registrationDeadline -and -not $registrationProcess.HasExited) {
        $registrationProcess.Refresh()
        if ($registrationProcess.MainWindowHandle -ne [IntPtr]::Zero) { break }
        Start-Sleep -Milliseconds 500
    }
    Assert-Condition (-not $registrationProcess.HasExited) `
        "Visual Studio exited during registration priming (exit $($registrationProcess.ExitCode))."
    Assert-Condition ($registrationProcess.MainWindowHandle -ne [IntPtr]::Zero) `
        'Visual Studio did not expose a registration-prime main window within the bounded interval.'
    Assert-Condition $registrationProcess.WaitForInputIdle(30000) `
        'Visual Studio registration-prime process did not reach input-idle state within the bounded interval.'
    Assert-Condition $registrationProcess.CloseMainWindow() `
        'Visual Studio registration-prime main window rejected the close request.'
    if ($registrationProcess.WaitForExit(15000)) {
        Assert-Condition ($registrationProcess.ExitCode -eq 0) `
            "Visual Studio registration-prime process exited with code $($registrationProcess.ExitCode)."
    }
    else {
        Write-Warning 'Visual Studio registration-prime process did not exit after the bounded close request; terminating its process tree.'
        try { $registrationProcess.Kill($true) }
        catch { throw "Visual Studio registration-prime process could not be terminated: $($_.Exception.Message)" }
        Assert-Condition $registrationProcess.WaitForExit(15000) `
            'Visual Studio registration-prime process tree did not terminate within the bounded cleanup interval.'
    }
    Assert-Condition (Test-Path -LiteralPath $registrationActivityLog -PathType Leaf) `
        'Visual Studio registration-prime ActivityLog was not retained.'
    [xml]$registrationActivity = Get-Content -LiteralPath $registrationActivityLog -Raw -ErrorAction Stop
    $installedPkgDef = [System.IO.Path]::GetFullPath((Join-Path $installedDirectory 'Copperfin.VisualStudio.pkgdef'))
    $matchingPkgDefImports = @($registrationActivity.activity.entry | Where-Object {
        [string]$_.description -eq 'Importing pkgdef file' -and
        [string]::Equals([System.IO.Path]::GetFullPath([string]$_.path), $installedPkgDef,
            [System.StringComparison]::OrdinalIgnoreCase)
    })
    Assert-Condition ($matchingPkgDefImports.Count -ge 1) `
        "Registration-prime ActivityLog did not prove import of the installed Copperfin pkgdef: $installedPkgDef"
    $installedDriverPkgDef = [System.IO.Path]::GetFullPath(
        (Join-Path $installedDriverDirectory 'Copperfin.VisualStudio.LifecycleDriver.pkgdef'))
    $matchingDriverPkgDefImports = @($registrationActivity.activity.entry | Where-Object {
        [string]$_.description -eq 'Importing pkgdef file' -and
        [string]::Equals([System.IO.Path]::GetFullPath([string]$_.path), $installedDriverPkgDef,
            [System.StringComparison]::OrdinalIgnoreCase)
    })
    Assert-Condition ($matchingDriverPkgDefImports.Count -ge 1) `
        "Registration-prime ActivityLog did not prove import of the test-only lifecycle-driver pkgdef: $installedDriverPkgDef"
    $registrationProcess.Dispose()
    $registrationProcess = $null

    Write-Host 'VSIX lifecycle phase: launch evidence Visual Studio'
    $startInfo = [System.Diagnostics.ProcessStartInfo]::new()
    $startInfo.FileName = $devenv
    $startInfo.UseShellExecute = $false
    $startInfo.WorkingDirectory = $resolvedEvidenceDirectory
    $startInfo.EnvironmentVariables['COPPERFIN_VSIX_LIFECYCLE_DRIVER_RESULT'] = $driverResultPath
    $startInfo.EnvironmentVariables['COPPERFIN_VSIX_LIFECYCLE_DRIVER_PRG'] = $fixturePrg
    foreach ($argument in @('/NoSplash', '/Log', $activityLog)) {
        [void]$startInfo.ArgumentList.Add($argument)
    }
    $ideProcess = [System.Diagnostics.Process]::Start($startInfo)
    Assert-Condition ($null -ne $ideProcess) 'Visual Studio lifecycle smoke did not start.'
    $startupDeadline = [DateTime]::UtcNow.AddSeconds($ProcessTimeoutSeconds)
    while ([DateTime]::UtcNow -lt $startupDeadline -and -not $ideProcess.HasExited) {
        $ideProcess.Refresh()
        if ($ideProcess.MainWindowHandle -ne [IntPtr]::Zero) { break }
        Start-Sleep -Milliseconds 500
    }
    Assert-Condition (-not $ideProcess.HasExited) `
        "Visual Studio exited during lifecycle startup (exit $($ideProcess.ExitCode))."
    Assert-Condition ($ideProcess.MainWindowHandle -ne [IntPtr]::Zero) `
        'Visual Studio did not expose a main window within the bounded startup interval.'

    $automationScript = Join-Path $resolvedEvidenceDirectory 'observe-visual-studio-surface.ps1'
    [System.IO.File]::WriteAllText($automationScript, @'
param(
    [Parameter(Mandatory = $true)][int]$ExpectedProcessId,
    [Parameter(Mandatory = $true)][string]$ExpectedName,
    [string]$AlternateExpectedName = '',
    [switch]$AllowProcessWindowTitlePrefix,
    [Parameter(Mandatory = $true)][string]$DiagnosticPath,
    [Parameter(Mandatory = $true)][string]$EvidenceDescription,
    [Parameter(Mandatory = $true)][int]$TimeoutSeconds
)
$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName UIAutomationClient
Add-Type -AssemblyName UIAutomationTypes
$deadline = [DateTime]::UtcNow.AddSeconds($TimeoutSeconds)
$expectedNames = @($ExpectedName)
if (-not [string]::IsNullOrWhiteSpace($AlternateExpectedName)) {
    $expectedNames += $AlternateExpectedName
}
$observed = $false
$lastAutomationError = ''
$lastProcessWindowName = ''
while ([DateTime]::UtcNow -lt $deadline -and -not $observed) {
    try {
        $processCondition = [System.Windows.Automation.PropertyCondition]::new(
            [System.Windows.Automation.AutomationElement]::ProcessIdProperty, $ExpectedProcessId)
        $ide = [System.Windows.Automation.AutomationElement]::RootElement.FindFirst(
            [System.Windows.Automation.TreeScope]::Children, $processCondition)
        if ($null -ne $ide) {
            $lastProcessWindowName = [string]$ide.Current.Name
            if ($AllowProcessWindowTitlePrefix -and
                    $ide.Current.Name.StartsWith("$ExpectedName - ", [System.StringComparison]::OrdinalIgnoreCase)) {
                $observed = $true
                break
            }
            foreach ($expectedName in $expectedNames) {
                $nameCondition = [System.Windows.Automation.PropertyCondition]::new(
                    [System.Windows.Automation.AutomationElement]::NameProperty, $expectedName)
                $match = $ide.FindFirst([System.Windows.Automation.TreeScope]::Descendants, $nameCondition)
                if ($null -ne $match) {
                    $observed = $true
                    break
                }
            }
            if ($observed) {
                break
            }
        }
    }
    catch { $lastAutomationError = $_.Exception.Message }
    Start-Sleep -Milliseconds 500
}
$diagnostic = [ordered]@{
    schema_version = 1
    expected_process_id = $ExpectedProcessId
    expected_surface = $ExpectedName
    surface_observed = $observed
    last_process_window_name = $lastProcessWindowName
    last_automation_error = $lastAutomationError
}
$diagnostic | ConvertTo-Json -Depth 3 | Set-Content -LiteralPath $DiagnosticPath -Encoding utf8
if (-not $observed) {
    throw "$EvidenceDescription was not observable in Visual Studio process $ExpectedProcessId. Expected one of: $($expectedNames -join ', '). Last process window name: '$lastProcessWindowName'. Last UI Automation error: $lastAutomationError"
}
'@, [System.Text.UTF8Encoding]::new($false))
    $windowsPowerShell = Join-Path $env:SystemRoot 'System32\WindowsPowerShell\v1.0\powershell.exe'
    Assert-Condition (Test-Path -LiteralPath $windowsPowerShell -PathType Leaf) `
        "Windows PowerShell is unavailable for UI Automation observation: $windowsPowerShell"
    $automationTimeoutSeconds = [Math]::Max(30, $ProcessTimeoutSeconds - 30)
    Write-Host 'VSIX lifecycle phase: await test-only in-process driver dispatch'
    $driverDeadline = [DateTime]::UtcNow.AddSeconds(60)
    while ([DateTime]::UtcNow -lt $driverDeadline -and
            -not (Test-Path -LiteralPath $driverResultPath -PathType Leaf)) {
        Assert-Condition (-not $ideProcess.HasExited) `
            "Visual Studio exited before lifecycle-driver dispatch (exit $($ideProcess.ExitCode))."
        Start-Sleep -Milliseconds 250
    }
    Assert-Condition (Test-Path -LiteralPath $driverResultPath -PathType Leaf) `
        'Test-only in-process lifecycle driver did not retain its dispatch result within 60 seconds.'
    $driverResult = Get-Content -LiteralPath $driverResultPath -Raw -ErrorAction Stop | ConvertFrom-Json
    Assert-Condition ([int]$driverResult.schema_version -eq 1) `
        'Lifecycle-driver result has an unsupported schema version.'
    Assert-Condition ([string]$driverResult.kind -eq 'copperfin-windows-vsix-lifecycle-driver') `
        'Lifecycle-driver result has an unexpected kind.'
    Assert-Condition ([string]$driverResult.product_command_group -eq '4b56ff76-d352-4027-bb18-ef4c759d260b') `
        'Lifecycle-driver result does not bind the Copperfin command group.'
    Assert-Condition ([uint32]$driverResult.product_command_id -eq 0x0300) `
        'Lifecycle-driver result does not bind Copperfin.ShowCommandWindow.'
    Assert-Condition ([int]$driverResult.command_post_hresult -eq 0) `
        "Lifecycle-driver command dispatch failed with HRESULT $($driverResult.command_post_hresult): $($driverResult.diagnostic)"
    Assert-Condition ([bool]$driverResult.prg_open_requested) `
        'Lifecycle driver did not request the exact runner-owned PRG.'
    Assert-Condition ([string]$driverResult.outcome -eq 'PASS') `
        "Lifecycle-driver dispatch failed: $($driverResult.diagnostic)"

    Write-Host 'VSIX lifecycle phase: observe exact installed Copperfin command surface'
    Invoke-BoundedProcess `
        -FilePath $windowsPowerShell `
        -Arguments @('-NoLogo', '-NoProfile', '-NonInteractive', '-STA', '-File', $automationScript,
            '-ExpectedProcessId', "$($ideProcess.Id)", '-ExpectedName', 'Copperfin Command',
            '-DiagnosticPath', (Join-Path $resolvedEvidenceDirectory 'ui-automation-command.json'),
            '-EvidenceDescription', 'Registered Copperfin Command surface',
            '-TimeoutSeconds', "$automationTimeoutSeconds") `
        -Name 'Copperfin command-surface UI Automation observation' | Out-Null

    Write-Host 'VSIX lifecycle phase: observe startup-opened runner-owned PRG'
    Invoke-BoundedProcess `
        -FilePath $windowsPowerShell `
        -Arguments @('-NoLogo', '-NoProfile', '-NonInteractive', '-STA', '-File', $automationScript,
            '-ExpectedProcessId', "$($ideProcess.Id)",
            '-ExpectedName', [System.IO.Path]::GetFileName($fixturePrg),
            '-AlternateExpectedName', [System.IO.Path]::GetFullPath($fixturePrg),
            '-AllowProcessWindowTitlePrefix',
            '-DiagnosticPath', (Join-Path $resolvedEvidenceDirectory 'ui-automation-prg.json'),
            '-EvidenceDescription', 'Exact runner-owned PRG document tab',
            '-TimeoutSeconds', "$automationTimeoutSeconds") `
        -Name 'Copperfin startup PRG document UI Automation observation' | Out-Null
    Start-Sleep -Seconds 2
    try { [void]$ideProcess.CloseMainWindow() } catch {}
    if (-not $ideProcess.WaitForExit(15000)) {
        try { $ideProcess.Kill($true) } catch { Write-Warning "Visual Studio could not be terminated after evidence collection: $($_.Exception.Message)" }
        [void]$ideProcess.WaitForExit(15000)
    }

    $deadline = [DateTime]::UtcNow.AddSeconds(60)
    $packageState = $null
    while ([DateTime]::UtcNow -lt $deadline) {
        if (Test-Path -LiteralPath $activityLog -PathType Leaf) {
            try {
                $packageState = Get-CopperfinPackageLoadState -ActivityLogPath $activityLog
                break
            }
            catch { Start-Sleep -Milliseconds 500 }
        }
        Start-Sleep -Milliseconds 500
    }
    Assert-Condition ($null -ne $packageState) 'Visual Studio ActivityLog was not readable after bounded IDE shutdown.'
    Assert-Condition ($packageState.MatchingErrors.Count -eq 0) `
        "ActivityLog contains Copperfin package-load errors: $($packageState.MatchingErrors -join '; ')"
    Assert-Condition $packageState.SuccessfulLoad `
        'ActivityLog did not contain an explicit successful End package load [CopperfinPackage] record.'
}
catch {
    $primaryFailure = $_
}
finally {
    if ($null -ne $registrationProcess) {
        if (-not $registrationProcess.HasExited) {
            try { [void]$registrationProcess.CloseMainWindow() } catch {}
            if (-not $registrationProcess.WaitForExit(15000)) {
                try { $registrationProcess.Kill($true) } catch { Write-Warning "Visual Studio registration-prime process could not be terminated: $($_.Exception.Message)" }
                [void]$registrationProcess.WaitForExit(15000)
            }
        }
        $registrationProcess.Dispose()
    }
    if ($null -ne $ideProcess) {
        if (-not $ideProcess.HasExited) {
            try { [void]$ideProcess.CloseMainWindow() } catch {}
            if (-not $ideProcess.WaitForExit(15000)) {
                try { $ideProcess.Kill($true) } catch { Write-Warning "Visual Studio could not be terminated: $($_.Exception.Message)" }
                [void]$ideProcess.WaitForExit(15000)
            }
        }
        $ideProcess.Dispose()
    }
    if ($null -eq $installedDirectory) {
        try {
            $cleanupCandidates = @(Get-ExtensionDirectories `
                    -VisualStudioInstanceId $instanceId `
                    -VisualStudioRegistryVersion $registryVersion `
                    -ExtensionId $packageIdentity.Id)
            if ($cleanupCandidates.Count -gt 0) {
                $installedDirectory = $cleanupCandidates[0]
                Write-Warning 'Discovered an installed Copperfin extension during failure cleanup.'
            }
        }
        catch { Write-Warning "Installed-extension cleanup discovery failed: $($_.Exception.Message)" }
    }
    if ($null -eq $installedDriverDirectory) {
        try {
            $driverCleanupCandidates = @(Get-ExtensionDirectories `
                    -VisualStudioInstanceId $instanceId `
                    -VisualStudioRegistryVersion $registryVersion `
                    -ExtensionId $driverIdentity.Id)
            if ($driverCleanupCandidates.Count -gt 0) {
                $installedDriverDirectory = $driverCleanupCandidates[0]
                Write-Warning 'Discovered an installed lifecycle-driver extension during failure cleanup.'
            }
        }
        catch { Write-Warning "Lifecycle-driver cleanup discovery failed: $($_.Exception.Message)" }
    }
    if ($null -ne $installedDriverDirectory) {
        Write-Host 'VSIX lifecycle phase: uninstall test-only in-process driver'
        try {
            Invoke-RecordedInstallerOperation -Arguments @(
                '/quiet', "/instanceIds:$instanceId", '/uninstall:Copperfin.VisualStudio.LifecycleDriver'
            ) -Name 'Copperfin lifecycle-driver VSIX uninstall' -Operation 'driver_uninstall' `
                -TargetExtensionId $driverIdentity.Id | Out-Null
        }
        catch {
            $driverCleanupFailure = $_
            Write-Warning "Lifecycle-driver uninstall cleanup failed: $($_.Exception.Message)"
        }
    }
    if ($null -ne $installedDirectory) {
        Write-Host 'VSIX lifecycle phase: uninstall'
        try {
            Invoke-RecordedInstallerOperation -Arguments @(
                '/quiet', "/instanceIds:$instanceId", '/uninstall:Copperfin.VisualStudio'
            ) -Name 'Copperfin VSIX uninstall' -Operation 'uninstall' `
                -TargetExtensionId $packageIdentity.Id | Out-Null
        }
        catch {
            $cleanupFailure = $_
            Write-Warning "VSIX uninstall cleanup failed after evidence collection: $($_.Exception.Message)"
        }
    }
}

$residue = @()
$driverResidue = @()
try {
    $residue = @(Get-ExtensionDirectories `
            -VisualStudioInstanceId $instanceId `
            -VisualStudioRegistryVersion $registryVersion `
            -ExtensionId $packageIdentity.Id)
    $driverResidue = @(Get-ExtensionDirectories `
            -VisualStudioInstanceId $instanceId `
            -VisualStudioRegistryVersion $registryVersion `
            -ExtensionId $driverIdentity.Id)
}
catch {
    $residueInventoryFailure = $_
}
$residueFailure = if ($residue.Count -eq 0) { '' } else {
    "VSIX uninstall left installed extension residue: $($residue -join ', ')"
}
$driverResidueFailure = if ($driverResidue.Count -eq 0) { '' } else {
    "Lifecycle-driver uninstall left installed extension residue: $($driverResidue -join ', ')"
}
if ($null -ne $primaryFailure) {
    $failureMessage = "VSIX lifecycle failed: $($primaryFailure.Exception.Message)"
    if ($null -ne $cleanupFailure) {
        $failureMessage += " Cleanup also failed: $($cleanupFailure.Exception.Message)"
    }
    if ($null -ne $driverCleanupFailure) {
        $failureMessage += " Lifecycle-driver cleanup also failed: $($driverCleanupFailure.Exception.Message)"
    }
    if ($null -ne $residueInventoryFailure) {
        $failureMessage += " Residue inventory also failed: $($residueInventoryFailure.Exception.Message)"
    }
    if (-not [string]::IsNullOrWhiteSpace($residueFailure)) {
        $failureMessage += " $residueFailure"
    }
    if (-not [string]::IsNullOrWhiteSpace($driverResidueFailure)) {
        $failureMessage += " $driverResidueFailure"
    }
    if ($null -ne $residueInventoryFailure) {
        $failureMessage += " Residue inventory also failed: $($residueInventoryFailure.Exception.Message)"
    }
    throw $failureMessage
}
if ($null -ne $residueInventoryFailure) {
    throw "VSIX residue inventory failed: $($residueInventoryFailure.Exception.Message)"
}
if ($null -ne $cleanupFailure) {
    $failureMessage = "VSIX lifecycle cleanup failed: $($cleanupFailure.Exception.Message)"
    if (-not [string]::IsNullOrWhiteSpace($residueFailure)) {
        $failureMessage += " $residueFailure"
    }
    throw $failureMessage
}
if ($null -ne $driverCleanupFailure) {
    $failureMessage = "Lifecycle-driver cleanup failed: $($driverCleanupFailure.Exception.Message)"
    if (-not [string]::IsNullOrWhiteSpace($driverResidueFailure)) {
        $failureMessage += " $driverResidueFailure"
    }
    throw $failureMessage
}
Assert-Condition ($residue.Count -eq 0) "VSIX uninstall left installed extension residue: $($residue -join ', ')"
Assert-Condition ($driverResidue.Count -eq 0) `
    "Lifecycle-driver uninstall left installed extension residue: $($driverResidue -join ', ')"

$result = [ordered]@{
    schema_version = 1
    kind = 'copperfin-windows-vsix-lifecycle-result'
    vsix_sha256 = $vsixSha256
    extension_id = $packageIdentity.Id
    extension_version = $packageIdentity.Version
    visual_studio_instance_id = $instanceId
    installation = 'PASS'
    package_registration_and_load = 'PASS'
    extension_version_check = 'PASS'
    supported_prg_open_and_command = 'PASS'
    same_version_reinstall = 'NOT_RUN'
    upgrade_from_previous_version = 'NOT_RUN'
    disablement = 'NOT_RUN'
    uninstall = 'PASS'
    extension_residue_check = 'PASS'
    development_checkout_dependency = 'PASS'
}
$result | ConvertTo-Json -Depth 4 | Set-Content -LiteralPath (Join-Path $resolvedEvidenceDirectory 'windows-vsix-lifecycle.json') -Encoding utf8NoBOM
Write-Host "Windows VSIX lifecycle passed for $($packageIdentity.Id) $($packageIdentity.Version), SHA-256 $vsixSha256."
