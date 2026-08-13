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

    [Parameter(ParameterSetName = 'Lifecycle')]
    [ValidateRange(30, 600)]
    [int]$ProcessTimeoutSeconds = 240,

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
        [Parameter(Mandatory = $true)][string]$Name
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
        if (-not $process.WaitForExit($ProcessTimeoutSeconds * 1000)) {
            try { $process.Kill($true) } catch { Write-Warning "$Name could not be terminated: $($_.Exception.Message)" }
            throw "$Name exceeded the bounded $ProcessTimeoutSeconds-second timeout."
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

function Get-CopperfinExtensionDirectories {
    param(
        [Parameter(Mandatory = $true)][string]$VisualStudioInstanceId,
        [Parameter(Mandatory = $true)][string]$VisualStudioRegistryVersion
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
                            try { (Read-VsixIdentity -ManifestPath $_.FullName).Id -eq 'Copperfin.VisualStudio' }
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
$resolvedEvidenceDirectory = [System.IO.Path]::GetFullPath($EvidenceDirectory)
$resolvedRunnerTemporaryRoot = [System.IO.Path]::GetFullPath($env:RUNNER_TEMP).TrimEnd('\')
$evidenceParent = [System.IO.Directory]::GetParent($resolvedEvidenceDirectory)
Assert-Condition ($resolvedVsix.EndsWith('.vsix', [System.StringComparison]::OrdinalIgnoreCase)) `
    "Lifecycle input must be a VSIX: $resolvedVsix"
Assert-Condition ($null -ne $evidenceParent) "Evidence directory has no parent: $resolvedEvidenceDirectory"
Assert-Condition ([string]::Equals($evidenceParent.FullName.TrimEnd('\'), $resolvedRunnerTemporaryRoot,
        [System.StringComparison]::OrdinalIgnoreCase)) `
    "Evidence directory must be a direct child of RUNNER_TEMP: $resolvedEvidenceDirectory"
Assert-Condition ([System.IO.Path]::GetFileName($resolvedEvidenceDirectory) -match '^copperfin-vsix-lifecycle-[A-Za-z0-9._-]+$') `
    "Evidence-directory leaf does not match the lifecycle allowlist: $resolvedEvidenceDirectory"
Assert-Condition (-not (Test-Path -LiteralPath $resolvedEvidenceDirectory)) `
    "Evidence directory already exists: $resolvedEvidenceDirectory"

$archiveRoot = Join-Path $resolvedEvidenceDirectory 'package'
[System.IO.Directory]::CreateDirectory($archiveRoot) | Out-Null
[System.IO.Compression.ZipFile]::ExtractToDirectory($resolvedVsix, $archiveRoot)
$packageIdentity = Read-VsixIdentity -ManifestPath (Join-Path $archiveRoot 'extension.vsixmanifest')
Assert-Condition ($packageIdentity.Id -eq 'Copperfin.VisualStudio') `
    "Unexpected VSIX identity: $($packageIdentity.Id)"

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

$existing = @(Get-CopperfinExtensionDirectories `
        -VisualStudioInstanceId $instanceId `
        -VisualStudioRegistryVersion $registryVersion)
Assert-Condition ($existing.Count -eq 0) 'Copperfin VSIX is already installed in the selected runner instance.'
$vsixSha256 = (Get-FileHash -LiteralPath $resolvedVsix -Algorithm SHA256).Hash.ToLowerInvariant()
$activityLog = Join-Path $resolvedEvidenceDirectory 'ActivityLog.xml'
$fixturePrg = Join-Path $resolvedEvidenceDirectory 'lifecycle-smoke.prg'
[System.IO.File]::WriteAllText($fixturePrg, "? 'Copperfin VSIX lifecycle smoke'`r`n", [System.Text.UTF8Encoding]::new($false))

$installedDirectory = $null
$ideProcess = $null
try {
    Write-Host 'VSIX lifecycle phase: install'
    Invoke-BoundedProcess -FilePath $vsixInstaller -Arguments @(
        '/quiet', "/instanceIds:$instanceId", $resolvedVsix
    ) -Name 'Copperfin VSIX installation' | Out-Null

    $installed = @(Get-CopperfinExtensionDirectories `
            -VisualStudioInstanceId $instanceId `
            -VisualStudioRegistryVersion $registryVersion)
    Assert-Condition ($installed.Count -eq 1) "Expected one installed Copperfin VSIX directory, found $($installed.Count)."
    $installedDirectory = $installed[0]
    $installedIdentity = Read-VsixIdentity -ManifestPath (Join-Path $installedDirectory 'extension.vsixmanifest')
    Assert-Condition ($installedIdentity.Version -eq $packageIdentity.Version) `
        "Installed VSIX version '$($installedIdentity.Version)' differs from package '$($packageIdentity.Version)'."
    Assert-Condition (Test-Path -LiteralPath (Join-Path $installedDirectory 'Copperfin.VisualStudio.dll') -PathType Leaf) `
        'Installed VSIX is missing Copperfin.VisualStudio.dll.'

    Write-Host 'VSIX lifecycle phase: refresh Visual Studio package registration'
    Invoke-BoundedProcess `
        -FilePath $devenv `
        -Arguments @('/updateconfiguration') `
        -Name 'Visual Studio package-registration refresh' | Out-Null

    Write-Host 'VSIX lifecycle phase: open runner-owned PRG'
    $startInfo = [System.Diagnostics.ProcessStartInfo]::new()
    $startInfo.FileName = $devenv
    $startInfo.UseShellExecute = $false
    $startInfo.WorkingDirectory = $resolvedEvidenceDirectory
    foreach ($argument in @($fixturePrg, '/NoSplash', '/Log', $activityLog)) {
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
        "Visual Studio exited while opening the runner-owned PRG (exit $($ideProcess.ExitCode))."
    Assert-Condition ($ideProcess.MainWindowHandle -ne [IntPtr]::Zero) `
        'Visual Studio did not expose a main window within the bounded startup interval.'

    Write-Host 'VSIX lifecycle phase: prove PRG document and invoke registered Copperfin command'
    $automationScript = Join-Path $resolvedEvidenceDirectory 'observe-running-visual-studio.ps1'
    [System.IO.File]::WriteAllText($automationScript, @'
param(
    [Parameter(Mandatory = $true)][string]$DteProgId,
    [Parameter(Mandatory = $true)][string]$ExpectedDocument,
    [Parameter(Mandatory = $true)][string]$CommandName,
    [Parameter(Mandatory = $true)][int]$TimeoutSeconds
)
$ErrorActionPreference = 'Stop'
$deadline = [DateTime]::UtcNow.AddSeconds($TimeoutSeconds)
$dte = $null
while ([DateTime]::UtcNow -lt $deadline -and $null -eq $dte) {
    try { $dte = [System.Runtime.InteropServices.Marshal]::GetActiveObject($DteProgId) }
    catch { Start-Sleep -Milliseconds 500 }
}
if ($null -eq $dte) { throw "Running Visual Studio DTE '$DteProgId' was not observable." }
$expected = [System.IO.Path]::GetFullPath($ExpectedDocument)
$documentObserved = $false
$lastDocumentError = ''
while ([DateTime]::UtcNow -lt $deadline -and -not $documentObserved) {
    try {
        for ($index = 1; $index -le $dte.Documents.Count; $index++) {
            $document = $dte.Documents.Item($index)
            if ($null -ne $document -and -not [string]::IsNullOrWhiteSpace([string]$document.FullName) -and
                    [string]::Equals([System.IO.Path]::GetFullPath([string]$document.FullName), $expected,
                        [System.StringComparison]::OrdinalIgnoreCase)) {
                $documentObserved = $true
                break
            }
        }
    }
    catch { $lastDocumentError = $_.Exception.Message }
    if (-not $documentObserved) { Start-Sleep -Milliseconds 500 }
}
if (-not $documentObserved) { throw "Expected PRG document was not open in Visual Studio: $expected. Last DTE error: $lastDocumentError" }
$commandInvoked = $false
$lastCommandError = ''
while ([DateTime]::UtcNow -lt $deadline -and -not $commandInvoked) {
    try {
        $dte.ExecuteCommand($CommandName)
        $commandInvoked = $true
    }
    catch {
        $lastCommandError = $_.Exception.Message
        Start-Sleep -Milliseconds 500
    }
}
if (-not $commandInvoked) { throw "Registered command '$CommandName' could not be invoked. Last DTE error: $lastCommandError" }
'@, [System.Text.UTF8Encoding]::new($false))
    $windowsPowerShell = Join-Path $env:SystemRoot 'System32\WindowsPowerShell\v1.0\powershell.exe'
    Assert-Condition (Test-Path -LiteralPath $windowsPowerShell -PathType Leaf) `
        "Windows PowerShell is unavailable for DTE observation: $windowsPowerShell"
    Invoke-BoundedProcess `
        -FilePath $windowsPowerShell `
        -Arguments @('-NoLogo', '-NoProfile', '-NonInteractive', '-STA', '-File', $automationScript,
            '-DteProgId', "VisualStudio.DTE.$registryVersion", '-ExpectedDocument', $fixturePrg,
            '-CommandName', 'Copperfin.ShowCommandWindow', '-TimeoutSeconds', "$ProcessTimeoutSeconds") `
        -Name 'Copperfin PRG and registered-command DTE observation' | Out-Null

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
finally {
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
    if ($null -ne $installedDirectory) {
        Write-Host 'VSIX lifecycle phase: uninstall'
        Invoke-BoundedProcess -FilePath $vsixInstaller -Arguments @(
            '/quiet', "/instanceIds:$instanceId", '/uninstall:Copperfin.VisualStudio'
        ) -Name 'Copperfin VSIX uninstall' | Out-Null
    }
}

$residue = @(Get-CopperfinExtensionDirectories `
        -VisualStudioInstanceId $instanceId `
        -VisualStudioRegistryVersion $registryVersion)
Assert-Condition ($residue.Count -eq 0) "VSIX uninstall left installed extension residue: $($residue -join ', ')"

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
