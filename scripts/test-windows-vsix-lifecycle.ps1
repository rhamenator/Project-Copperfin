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
        [Parameter(Mandatory = $true)][string]$Operation
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
            diagnostic = $diagnosticMessage
        })
        [ordered]@{
            schema_version = 1
            kind = 'copperfin-windows-vsix-installer-operations'
            vsix_sha256 = $vsixSha256
            visual_studio_instance_id = $instanceId
            extension_id = $packageIdentity.Id
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
        $scriptSource = Get-Content -LiteralPath $PSCommandPath -Raw -ErrorAction Stop
        $nativeSourceMatches = @([regex]::Matches(
            $scriptSource, '(?s)Add-Type -TypeDefinition @"(.*?)"@') | Where-Object {
                $_.Groups[1].Value.Contains('namespace CopperfinLifecycle')
            })
        Assert-Condition ($nativeSourceMatches.Count -eq 1) `
            'Self-test could not locate the generated Win32 input source.'
        Add-Type -TypeDefinition $nativeSourceMatches[0].Groups[1].Value
        Assert-Condition ($null -ne ('CopperfinLifecycle.NativeMethods' -as [type])) `
            'Self-test did not compile the generated Win32 input source.'
        $inputType = [CopperfinLifecycle.NativeMethods].Assembly.GetType('CopperfinLifecycle.INPUT')
        $expectedInputSize = if ([IntPtr]::Size -eq 8) { 40 } else { 28 }
        Assert-Condition ([Runtime.InteropServices.Marshal]::SizeOf([System.Type]$inputType) -eq $expectedInputSize) `
            'Self-test found an invalid Win32 INPUT structure size.'
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
$registrationActivityLog = Join-Path $resolvedEvidenceDirectory 'ActivityLog-registration.xml'
$activityLog = Join-Path $resolvedEvidenceDirectory 'ActivityLog.xml'
$fixturePrg = Join-Path $resolvedEvidenceDirectory 'lifecycle-smoke.prg'
$installerDiagnosticsPath = Join-Path $resolvedEvidenceDirectory 'vsix-installer-operations.json'
$installerOperations = [System.Collections.Generic.List[object]]::new()
[System.IO.File]::WriteAllText($fixturePrg, "? 'Copperfin VSIX lifecycle smoke'`r`n", [System.Text.UTF8Encoding]::new($false))

$installedDirectory = $null
$registrationProcess = $null
$ideProcess = $null
$primaryFailure = $null
$cleanupFailure = $null
$residueInventoryFailure = $null
try {
    Write-Host 'VSIX lifecycle phase: install'
    Invoke-RecordedInstallerOperation -Arguments @(
        '/quiet', "/instanceIds:$instanceId", $resolvedVsix
    ) -Name 'Copperfin VSIX installation' -Operation 'install' | Out-Null

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
    $registrationProcess.Dispose()
    $registrationProcess = $null

    Write-Host 'VSIX lifecycle phase: launch evidence Visual Studio'
    $startInfo = [System.Diagnostics.ProcessStartInfo]::new()
    $startInfo.FileName = $devenv
    $startInfo.UseShellExecute = $false
    $startInfo.WorkingDirectory = $resolvedEvidenceDirectory
    foreach ($argument in @('/NoSplash', '/Log', $activityLog, $fixturePrg)) {
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
    [string]$InvokeCanonicalCommand = '',
    [switch]$RequestCommandWindowOnly,
    [switch]$SubmitCanonicalCommandOnly,
    [switch]$CommandWindowAlreadyRequested,
    [Parameter(Mandatory = $true)][string]$DiagnosticPath,
    [Parameter(Mandatory = $true)][string]$EvidenceDescription,
    [Parameter(Mandatory = $true)][int]$TimeoutSeconds
)
$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName UIAutomationClient
Add-Type -AssemblyName UIAutomationTypes
Add-Type -TypeDefinition @"
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace CopperfinLifecycle {
    [StructLayout(LayoutKind.Sequential)]
    internal struct MOUSEINPUT {
        internal int dx;
        internal int dy;
        internal uint mouseData;
        internal uint dwFlags;
        internal uint time;
        internal UIntPtr dwExtraInfo;
    }

    [StructLayout(LayoutKind.Sequential)]
    internal struct KEYBDINPUT {
        internal ushort wVk;
        internal ushort wScan;
        internal uint dwFlags;
        internal uint time;
        internal UIntPtr dwExtraInfo;
    }

    [StructLayout(LayoutKind.Sequential)]
    internal struct HARDWAREINPUT {
        internal uint uMsg;
        internal ushort wParamL;
        internal ushort wParamH;
    }

    [StructLayout(LayoutKind.Explicit)]
    internal struct INPUTUNION {
        [FieldOffset(0)] internal MOUSEINPUT mi;
        [FieldOffset(0)] internal KEYBDINPUT ki;
        [FieldOffset(0)] internal HARDWAREINPUT hi;
    }

    [StructLayout(LayoutKind.Sequential)]
    internal struct INPUT {
        internal uint type;
        internal INPUTUNION data;
    }

    public static class NativeMethods {
        private const uint INPUT_KEYBOARD = 1;
        private const uint KEYEVENTF_KEYUP = 0x0002;
        private const uint KEYEVENTF_UNICODE = 0x0004;
        private const ushort VK_CONTROL = 0x11;
        private const ushort VK_MENU = 0x12;
        private const ushort VK_A = 0x41;
        private const ushort VK_RETURN = 0x0D;

        [DllImport("user32.dll")]
        public static extern bool SetForegroundWindow(IntPtr hWnd);

        [DllImport("user32.dll")]
        public static extern IntPtr GetForegroundWindow();

        [DllImport("user32.dll")]
        public static extern uint GetWindowThreadProcessId(IntPtr hWnd, out uint processId);

        [DllImport("user32.dll", SetLastError = true)]
        private static extern uint SendInput(uint inputCount, INPUT[] inputs, int inputSize);

        private static INPUT VirtualKey(ushort key, bool keyUp) {
            INPUT input = new INPUT();
            input.type = INPUT_KEYBOARD;
            input.data.ki.wVk = key;
            input.data.ki.dwFlags = keyUp ? KEYEVENTF_KEYUP : 0;
            return input;
        }

        private static INPUT Unicode(char value, bool keyUp) {
            INPUT input = new INPUT();
            input.type = INPUT_KEYBOARD;
            input.data.ki.wScan = value;
            input.data.ki.dwFlags = KEYEVENTF_UNICODE | (keyUp ? KEYEVENTF_KEYUP : 0);
            return input;
        }

        private static uint Inject(INPUT[] inputs) {
            return SendInput((uint)inputs.Length, inputs, Marshal.SizeOf(typeof(INPUT)));
        }

        public static uint SendCommandWindowShortcut() {
            return Inject(new INPUT[] {
                VirtualKey(VK_CONTROL, false), VirtualKey(VK_MENU, false),
                VirtualKey(VK_A, false), VirtualKey(VK_A, true),
                VirtualKey(VK_MENU, true), VirtualKey(VK_CONTROL, true)
            });
        }

        public static uint SendUnicodeTextAndEnter(string value) {
            List<INPUT> inputs = new List<INPUT>();
            foreach (char character in value) {
                inputs.Add(Unicode(character, false));
                inputs.Add(Unicode(character, true));
            }
            inputs.Add(VirtualKey(VK_RETURN, false));
            inputs.Add(VirtualKey(VK_RETURN, true));
            return Inject(inputs.ToArray());
        }
    }
}
"@
$deadline = [DateTime]::UtcNow.AddSeconds($TimeoutSeconds)
$expectedNames = @($ExpectedName)
if (-not [string]::IsNullOrWhiteSpace($AlternateExpectedName)) {
    $expectedNames += $AlternateExpectedName
}
$observed = $false
$lastAutomationError = ''
$lastProcessWindowName = ''
$requiresInput = $RequestCommandWindowOnly -or -not [string]::IsNullOrWhiteSpace($InvokeCanonicalCommand)
$commandSubmitted = -not $requiresInput
$foregroundProcessVerified = $false
$commandWindowShortcutSent = $false
$shortcutInputEventsExpected = 0
$shortcutInputEventsInjected = 0
$commandInputForegroundVerified = $false
$canonicalCommandSubmitted = $false
$commandInputEventsExpected = 0
$commandInputEventsInjected = 0
while ([DateTime]::UtcNow -lt $deadline -and -not $observed) {
    try {
        $processCondition = [System.Windows.Automation.PropertyCondition]::new(
            [System.Windows.Automation.AutomationElement]::ProcessIdProperty, $ExpectedProcessId)
        $ide = [System.Windows.Automation.AutomationElement]::RootElement.FindFirst(
            [System.Windows.Automation.TreeScope]::Children, $processCondition)
        if ($null -ne $ide) {
            $lastProcessWindowName = [string]$ide.Current.Name
            if (-not $commandSubmitted) {
                $ideWindowHandle = [IntPtr]::new($ide.Current.NativeWindowHandle)
                if ([CopperfinLifecycle.NativeMethods]::SetForegroundWindow($ideWindowHandle)) {
                    $foregroundDeadline = [DateTime]::UtcNow.AddSeconds(5)
                    while ([DateTime]::UtcNow -lt $foregroundDeadline -and -not $foregroundProcessVerified) {
                        [uint32]$foregroundProcessId = 0
                        $foregroundWindow = [CopperfinLifecycle.NativeMethods]::GetForegroundWindow()
                        [void][CopperfinLifecycle.NativeMethods]::GetWindowThreadProcessId(
                            $foregroundWindow, [ref]$foregroundProcessId)
                        $foregroundProcessVerified = $foregroundProcessId -eq $ExpectedProcessId
                        if (-not $foregroundProcessVerified) { Start-Sleep -Milliseconds 100 }
                    }
                }
                if ($foregroundProcessVerified -and -not $CommandWindowAlreadyRequested) {
                    Start-Sleep -Seconds 2
                    $shortcutInputEventsExpected = 6
                    $shortcutInputEventsInjected = [CopperfinLifecycle.NativeMethods]::SendCommandWindowShortcut()
                    $commandWindowShortcutSent = $shortcutInputEventsInjected -eq $shortcutInputEventsExpected
                    if (-not $commandWindowShortcutSent) {
                        $nativeError = [Runtime.InteropServices.Marshal]::GetLastWin32Error()
                        throw "SendInput inserted $shortcutInputEventsInjected of $shortcutInputEventsExpected Command Window shortcut events (Win32 error $nativeError)."
                    }
                    Start-Sleep -Seconds 1
                    if ($RequestCommandWindowOnly) { break }
                }
                if ($foregroundProcessVerified -and -not $RequestCommandWindowOnly) {
                    [uint32]$commandInputProcessId = 0
                    $commandInputWindow = [CopperfinLifecycle.NativeMethods]::GetForegroundWindow()
                    [void][CopperfinLifecycle.NativeMethods]::GetWindowThreadProcessId(
                        $commandInputWindow, [ref]$commandInputProcessId)
                    $commandInputForegroundVerified = $commandInputProcessId -eq $ExpectedProcessId
                    if ($commandInputForegroundVerified) {
                        # The greater-than character is the Command Window's displayed
                        # prompt. It is not part of the command entered by the user.
                        $commandText = $InvokeCanonicalCommand
                        $commandInputEventsExpected = (2 * $commandText.Length) + 2
                        $commandInputEventsInjected = [CopperfinLifecycle.NativeMethods]::SendUnicodeTextAndEnter($commandText)
                        $canonicalCommandSubmitted = $commandInputEventsInjected -eq $commandInputEventsExpected
                        if (-not $canonicalCommandSubmitted) {
                            $nativeError = [Runtime.InteropServices.Marshal]::GetLastWin32Error()
                            throw "SendInput inserted $commandInputEventsInjected of $commandInputEventsExpected canonical-command events (Win32 error $nativeError)."
                        }
                        $commandSubmitted = $true
                        if ($SubmitCanonicalCommandOnly) { break }
                    }
                }
            }
            if (-not $commandSubmitted) {
                Start-Sleep -Milliseconds 500
                continue
            }
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
    invoke_canonical_command = $InvokeCanonicalCommand
    request_command_window_only = [bool]$RequestCommandWindowOnly
    submission_only = [bool]$SubmitCanonicalCommandOnly
    command_window_already_requested = [bool]$CommandWindowAlreadyRequested
    foreground_process_verified = $foregroundProcessVerified
    command_window_shortcut_sent = $commandWindowShortcutSent
    shortcut_input_events_expected = $shortcutInputEventsExpected
    shortcut_input_events_injected = $shortcutInputEventsInjected
    command_input_foreground_verified = $commandInputForegroundVerified
    canonical_command_submitted = $canonicalCommandSubmitted
    command_input_events_expected = $commandInputEventsExpected
    command_input_events_injected = $commandInputEventsInjected
    surface_observed = $observed
    last_process_window_name = $lastProcessWindowName
    last_automation_error = $lastAutomationError
}
$diagnostic | ConvertTo-Json -Depth 3 | Set-Content -LiteralPath $DiagnosticPath -Encoding utf8
if ($RequestCommandWindowOnly) {
    if (-not $commandWindowShortcutSent) {
        throw "$EvidenceDescription was not requested in Visual Studio process $ExpectedProcessId within the bounded interval. Foreground process verified: $foregroundProcessVerified."
    }
    return
}
if ($SubmitCanonicalCommandOnly) {
    if (-not $canonicalCommandSubmitted) {
        throw "$EvidenceDescription was not submitted to Visual Studio process $ExpectedProcessId within the bounded interval. Foreground process verified: $foregroundProcessVerified. Command Window shortcut sent: $commandWindowShortcutSent. Command-input foreground verified: $commandInputForegroundVerified."
    }
    return
}
if (-not $observed) {
    throw "$EvidenceDescription was not observable in Visual Studio process $ExpectedProcessId. Expected one of: $($expectedNames -join ', '). Foreground process verified: $foregroundProcessVerified. Command Window shortcut sent: $commandWindowShortcutSent. Command-input foreground verified: $commandInputForegroundVerified. Canonical command submitted: $canonicalCommandSubmitted. Last process window name: '$lastProcessWindowName'. Last UI Automation error: $lastAutomationError"
}
'@, [System.Text.UTF8Encoding]::new($false))
    $windowsPowerShell = Join-Path $env:SystemRoot 'System32\WindowsPowerShell\v1.0\powershell.exe'
    Assert-Condition (Test-Path -LiteralPath $windowsPowerShell -PathType Leaf) `
        "Windows PowerShell is unavailable for UI Automation observation: $windowsPowerShell"
    $automationTimeoutSeconds = [Math]::Max(30, $ProcessTimeoutSeconds - 30)
    Write-Host 'VSIX lifecycle phase: request Visual Studio Command Window'
    Invoke-BoundedProcess `
        -FilePath $windowsPowerShell `
        -Arguments @('-NoLogo', '-NoProfile', '-NonInteractive', '-STA', '-File', $automationScript,
            '-ExpectedProcessId', "$($ideProcess.Id)", '-ExpectedName', 'Copperfin Command',
            '-RequestCommandWindowOnly',
            '-DiagnosticPath', (Join-Path $resolvedEvidenceDirectory 'ui-automation-command-window-input.json'),
            '-EvidenceDescription', 'Visual Studio Command Window',
            '-TimeoutSeconds', "$automationTimeoutSeconds") `
        -Name 'Visual Studio Command Window request' | Out-Null
    Start-Sleep -Seconds 2

    Write-Host 'VSIX lifecycle phase: repeat and release queued Command Window request'
    Invoke-BoundedProcess `
        -FilePath $windowsPowerShell `
        -Arguments @('-NoLogo', '-NoProfile', '-NonInteractive', '-STA', '-File', $automationScript,
            '-ExpectedProcessId', "$($ideProcess.Id)", '-ExpectedName', 'Copperfin Command',
            '-RequestCommandWindowOnly',
            '-DiagnosticPath', (Join-Path $resolvedEvidenceDirectory 'ui-automation-command-window-dispatch.json'),
            '-EvidenceDescription', 'Repeated Visual Studio Command Window request',
            '-TimeoutSeconds', "$automationTimeoutSeconds") `
        -Name 'Repeated Visual Studio Command Window request' | Out-Null
    Start-Sleep -Seconds 10

    Write-Host 'VSIX lifecycle phase: submit exact installed Copperfin command'
    Invoke-BoundedProcess `
        -FilePath $windowsPowerShell `
        -Arguments @('-NoLogo', '-NoProfile', '-NonInteractive', '-STA', '-File', $automationScript,
            '-ExpectedProcessId', "$($ideProcess.Id)", '-ExpectedName', 'Copperfin Command',
            '-InvokeCanonicalCommand', 'Copperfin.ShowCommandWindow',
            '-SubmitCanonicalCommandOnly',
            '-CommandWindowAlreadyRequested',
            '-DiagnosticPath', (Join-Path $resolvedEvidenceDirectory 'ui-automation-command-input.json'),
            '-EvidenceDescription', 'Canonical Copperfin command',
            '-TimeoutSeconds', "$automationTimeoutSeconds") `
        -Name 'Copperfin canonical command submission' | Out-Null

    Write-Host 'VSIX lifecycle phase: settle post-canonical Command Window readiness'
    Start-Sleep -Seconds 10

    Write-Host 'VSIX lifecycle phase: release queued canonical command with built-in Command Window request'
    Invoke-BoundedProcess `
        -FilePath $windowsPowerShell `
        -Arguments @('-NoLogo', '-NoProfile', '-NonInteractive', '-STA', '-File', $automationScript,
            '-ExpectedProcessId', "$($ideProcess.Id)", '-ExpectedName', 'Copperfin Command',
            '-RequestCommandWindowOnly',
            '-DiagnosticPath', (Join-Path $resolvedEvidenceDirectory 'ui-automation-command-input-dispatch.json'),
            '-EvidenceDescription', 'Post-command Visual Studio Command Window request',
            '-TimeoutSeconds', "$automationTimeoutSeconds") `
        -Name 'Post-command Visual Studio Command Window request' | Out-Null

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
            $cleanupCandidates = @(Get-CopperfinExtensionDirectories `
                    -VisualStudioInstanceId $instanceId `
                    -VisualStudioRegistryVersion $registryVersion)
            if ($cleanupCandidates.Count -gt 0) {
                $installedDirectory = $cleanupCandidates[0]
                Write-Warning 'Discovered an installed Copperfin extension during failure cleanup.'
            }
        }
        catch { Write-Warning "Installed-extension cleanup discovery failed: $($_.Exception.Message)" }
    }
    if ($null -ne $installedDirectory) {
        Write-Host 'VSIX lifecycle phase: uninstall'
        try {
            Invoke-RecordedInstallerOperation -Arguments @(
                '/quiet', "/instanceIds:$instanceId", '/uninstall:Copperfin.VisualStudio'
            ) -Name 'Copperfin VSIX uninstall' -Operation 'uninstall' | Out-Null
        }
        catch {
            $cleanupFailure = $_
            Write-Warning "VSIX uninstall cleanup failed after evidence collection: $($_.Exception.Message)"
        }
    }
}

$residue = @()
try {
    $residue = @(Get-CopperfinExtensionDirectories `
            -VisualStudioInstanceId $instanceId `
            -VisualStudioRegistryVersion $registryVersion)
}
catch {
    $residueInventoryFailure = $_
}
$residueFailure = if ($residue.Count -eq 0) { '' } else {
    "VSIX uninstall left installed extension residue: $($residue -join ', ')"
}
if ($null -ne $primaryFailure) {
    $failureMessage = "VSIX lifecycle failed: $($primaryFailure.Exception.Message)"
    if ($null -ne $cleanupFailure) {
        $failureMessage += " Cleanup also failed: $($cleanupFailure.Exception.Message)"
    }
    if ($null -ne $residueInventoryFailure) {
        $failureMessage += " Residue inventory also failed: $($residueInventoryFailure.Exception.Message)"
    }
    if (-not [string]::IsNullOrWhiteSpace($residueFailure)) {
        $failureMessage += " $residueFailure"
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
