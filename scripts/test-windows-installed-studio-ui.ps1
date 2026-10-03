# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
# Traceability: RQ-CF-REL-002; RQ-CF-REL-008;
# DQ-windows-installer-lifecycle-scope; DQ-windows-installed-ui-scope;
# DV-windows-installer-lifecycle-contract; DV-windows-installed-ui-contract;
# HZ-system-failure-01;
# HZ-data-corruption-01; HZ-doc-command-01.

[CmdletBinding()]
param(
    [Parameter(Mandatory = $true, ParameterSetName = 'Lifecycle')]
    [ValidateNotNullOrEmpty()]
    [string]$StudioPath,

    [Parameter(Mandatory = $true, ParameterSetName = 'Lifecycle')]
    [ValidateNotNullOrEmpty()]
    [string]$FixturePath,

    [Parameter(Mandatory = $true, ParameterSetName = 'Lifecycle')]
    [ValidateNotNullOrEmpty()]
    [string]$EvidenceDirectory,

    [Parameter(ParameterSetName = 'Lifecycle')]
    [ValidateRange(10, 300)]
    [int]$TimeoutSeconds = 90,

    [Parameter(Mandatory = $true, ParameterSetName = 'SelfTest')]
    [switch]$SelfTest
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

function Assert-Condition {
    param(
        [Parameter(Mandatory = $true)][bool]$Condition,
        [Parameter(Mandatory = $true)][string]$Message
    )

    if (-not $Condition) {
        throw $Message
    }
}

function Normalize-AccessibleName {
    param([AllowNull()][object]$Name)

    if ($null -eq $Name) {
        return ''
    }
    return ([string]$Name).Replace('&', '').Trim()
}

if ($SelfTest) {
    Assert-Condition ((Normalize-AccessibleName '&File') -ceq 'File') `
        'Accessible-name normalization did not remove a WinForms mnemonic.'
    Assert-Condition ((Normalize-AccessibleName '  Copperfin Command  ') -ceq 'Copperfin Command') `
        'Accessible-name normalization did not trim surrounding space.'
    Assert-Condition ((Normalize-AccessibleName $null) -ceq '') `
        'Accessible-name normalization did not handle a missing name.'
    Write-Host 'Windows installed Studio UI helper self-test passed.'
    return
}

Add-Type -AssemblyName UIAutomationClient
Add-Type -AssemblyName UIAutomationTypes
Add-Type -AssemblyName System.Drawing
Add-Type -AssemblyName System.Windows.Forms

function Find-SemanticElement {
    param(
        [Parameter(Mandatory = $true)]
        [System.Windows.Automation.AutomationElement]$Root,
        [Parameter(Mandatory = $true)]
        [System.Windows.Automation.ControlType]$ControlType,
        [Parameter(Mandatory = $true)]
        [string]$ExpectedName
    )

    $condition = [System.Windows.Automation.PropertyCondition]::new(
        [System.Windows.Automation.AutomationElement]::ProcessIdProperty,
        $Root.Current.ProcessId)
    $candidates = [System.Windows.Automation.AutomationElement]::RootElement.FindAll(
        [System.Windows.Automation.TreeScope]::Descendants,
        $condition)
    for ($index = 0; $index -lt $candidates.Count; ++$index) {
        $candidate = $candidates.Item($index)
        if ($candidate.Current.ControlType.Id -ne $ControlType.Id) {
            continue
        }
        if ([string]::Equals(
                (Normalize-AccessibleName $candidate.Current.Name),
                (Normalize-AccessibleName $ExpectedName),
                [System.StringComparison]::OrdinalIgnoreCase)) {
            return $candidate
        }
    }
    return $null
}

function Wait-SemanticElement {
    param(
        [Parameter(Mandatory = $true)]
        [System.Windows.Automation.AutomationElement]$Root,
        [Parameter(Mandatory = $true)]
        [System.Windows.Automation.ControlType]$ControlType,
        [Parameter(Mandatory = $true)]
        [string]$ExpectedName,
        [Parameter(Mandatory = $true)]
        [DateTime]$Deadline
    )

    while ([DateTime]::UtcNow -lt $Deadline) {
        $candidate = Find-SemanticElement -Root $Root -ControlType $ControlType -ExpectedName $ExpectedName
        if ($null -ne $candidate) {
            return $candidate
        }
        Start-Sleep -Milliseconds 250
    }
    throw "Timed out waiting for $($ControlType.ProgrammaticName) '$ExpectedName'."
}

function Select-SemanticTab {
    param(
        [Parameter(Mandatory = $true)]
        [System.Windows.Automation.AutomationElement]$Element,
        [Parameter(Mandatory = $true)]
        [string]$Description
    )

    $patternObject = $null
    Assert-Condition `
        ($Element.TryGetCurrentPattern(
            [System.Windows.Automation.SelectionItemPattern]::Pattern,
            [ref]$patternObject)) `
        "$Description does not expose SelectionItemPattern."
    $pattern = [System.Windows.Automation.SelectionItemPattern]$patternObject
    $pattern.Select()
    Assert-Condition $pattern.Current.IsSelected `
        "$Description did not become selected after semantic selection."
}

function Get-UiTreeSnapshot {
    param([AllowNull()][System.Windows.Automation.AutomationElement]$Root)

    if ($null -eq $Root) {
        return @()
    }
    $snapshot = @()
    try {
        $processCondition = [System.Windows.Automation.PropertyCondition]::new(
            [System.Windows.Automation.AutomationElement]::ProcessIdProperty,
            $Root.Current.ProcessId)
        $elements = [System.Windows.Automation.AutomationElement]::RootElement.FindAll(
            [System.Windows.Automation.TreeScope]::Descendants,
            $processCondition)
        $limit = [Math]::Min($elements.Count, 300)
        for ($index = 0; $index -lt $limit; ++$index) {
            try {
                $element = $elements.Item($index)
                $snapshot += [ordered]@{
                    name = [string]$element.Current.Name
                    control_type = [string]$element.Current.ControlType.ProgrammaticName
                    automation_id = [string]$element.Current.AutomationId
                    enabled = [bool]$element.Current.IsEnabled
                }
            }
            catch {
                $snapshot += [ordered]@{ error = $_.Exception.Message }
            }
        }
    }
    catch {
        $snapshot += [ordered]@{ error = $_.Exception.Message }
    }
    return @($snapshot)
}

function Save-UiScreenshot {
    param(
        [AllowNull()][System.Windows.Automation.AutomationElement]$Root,
        [Parameter(Mandatory = $true)][string]$Path
    )

    $rectangle = [System.Windows.Forms.SystemInformation]::VirtualScreen
    if ($null -ne $Root) {
        try {
            $bounds = $Root.Current.BoundingRectangle
            if ($bounds.Width -ge 1 -and $bounds.Height -ge 1) {
                $rectangle = [System.Drawing.Rectangle]::FromLTRB(
                    [int][Math]::Floor($bounds.Left),
                    [int][Math]::Floor($bounds.Top),
                    [int][Math]::Ceiling($bounds.Right),
                    [int][Math]::Ceiling($bounds.Bottom))
            }
        }
        catch {
        }
    }
    Assert-Condition ($rectangle.Width -ge 1 -and $rectangle.Height -ge 1) `
        'No capturable desktop bounds were available.'
    $bitmap = [System.Drawing.Bitmap]::new($rectangle.Width, $rectangle.Height)
    $graphics = [System.Drawing.Graphics]::FromImage($bitmap)
    try {
        $graphics.CopyFromScreen(
            $rectangle.Left,
            $rectangle.Top,
            0,
            0,
            $rectangle.Size,
            [System.Drawing.CopyPixelOperation]::SourceCopy)
        $bitmap.Save($Path, [System.Drawing.Imaging.ImageFormat]::Png)
    }
    finally {
        $graphics.Dispose()
        $bitmap.Dispose()
    }
}

$resolvedStudio = (Resolve-Path -LiteralPath $StudioPath).Path
$resolvedFixture = (Resolve-Path -LiteralPath $FixturePath).Path
$resolvedEvidenceDirectory = [System.IO.Path]::GetFullPath($EvidenceDirectory)
Assert-Condition ($resolvedStudio.EndsWith('Copperfin.Studio.exe', [System.StringComparison]::OrdinalIgnoreCase)) `
    "Installed Studio path has an unexpected identity: $resolvedStudio"
Assert-Condition ($resolvedFixture.EndsWith('.prg', [System.StringComparison]::OrdinalIgnoreCase)) `
    "Installed Studio UI fixture must be a PRG file: $resolvedFixture"
New-Item -ItemType Directory -Path $resolvedEvidenceDirectory -Force | Out-Null

$startInfo = [System.Diagnostics.ProcessStartInfo]::new()
$startInfo.FileName = $resolvedStudio
$startInfo.Arguments = "--locale en-US `"$resolvedFixture`""
$startInfo.UseShellExecute = $false
$process = [System.Diagnostics.Process]::new()
$process.StartInfo = $startInfo
$processStarted = $false
$root = $null
$observedWindowTitle = ''
$semanticControls = @()
$success = $false

try {
    Assert-Condition $process.Start() 'Installed Copperfin Studio did not start.'
    $processStarted = $true
    $deadline = [DateTime]::UtcNow.AddSeconds($TimeoutSeconds)
    $processCondition = [System.Windows.Automation.PropertyCondition]::new(
        [System.Windows.Automation.AutomationElement]::ProcessIdProperty,
        $process.Id)
    while ([DateTime]::UtcNow -lt $deadline -and -not $process.HasExited) {
        $root = [System.Windows.Automation.AutomationElement]::RootElement.FindFirst(
            [System.Windows.Automation.TreeScope]::Children,
            $processCondition)
        if ($null -ne $root) {
            break
        }
        Start-Sleep -Milliseconds 250
        $process.Refresh()
    }
    Assert-Condition (-not $process.HasExited) `
        "Installed Copperfin Studio exited before UI automation (exit $($process.ExitCode))."
    Assert-Condition ($null -ne $root) `
        'Installed Copperfin Studio did not expose a top-level UI Automation window.'
    Assert-Condition ($root.Current.Name.StartsWith('Copperfin Studio', [System.StringComparison]::Ordinal)) `
        "Installed Copperfin Studio exposed an unexpected window title: '$($root.Current.Name)'."
    $observedWindowTitle = [string]$root.Current.Name

    $documentName = [System.IO.Path]::GetFileName($resolvedFixture)
    $documentTab = Wait-SemanticElement -Root $root `
        -ControlType ([System.Windows.Automation.ControlType]::TabItem) `
        -ExpectedName $documentName -Deadline $deadline
    Select-SemanticTab -Element $documentTab -Description "runner-owned document tab '$documentName'"
    $semanticControls += [ordered]@{
        name = $documentName
        control_type = 'ControlType.TabItem'
        action = 'SelectionItem.Select'
    }

    $commandTab = Wait-SemanticElement -Root $root `
        -ControlType ([System.Windows.Automation.ControlType]::TabItem) `
        -ExpectedName 'Copperfin Command' -Deadline $deadline
    Select-SemanticTab -Element $commandTab -Description 'Copperfin Command tab'
    $semanticControls += [ordered]@{
        name = 'Copperfin Command'
        control_type = 'ControlType.TabItem'
        action = 'SelectionItem.Select'
    }

    $fileMenu = Wait-SemanticElement -Root $root `
        -ControlType ([System.Windows.Automation.ControlType]::MenuItem) `
        -ExpectedName 'File' -Deadline $deadline
    $expandObject = $null
    Assert-Condition `
        ($fileMenu.TryGetCurrentPattern(
            [System.Windows.Automation.ExpandCollapsePattern]::Pattern,
            [ref]$expandObject)) `
        'File menu does not expose ExpandCollapsePattern.'
    ([System.Windows.Automation.ExpandCollapsePattern]$expandObject).Expand()
    $semanticControls += [ordered]@{
        name = 'File'
        control_type = 'ControlType.MenuItem'
        action = 'ExpandCollapse.Expand'
    }

    $exitItem = Wait-SemanticElement -Root $root `
        -ControlType ([System.Windows.Automation.ControlType]::MenuItem) `
        -ExpectedName 'Exit' -Deadline $deadline
    $invokeObject = $null
    Assert-Condition `
        ($exitItem.TryGetCurrentPattern(
            [System.Windows.Automation.InvokePattern]::Pattern,
            [ref]$invokeObject)) `
        'Exit menu item does not expose InvokePattern.'
    ([System.Windows.Automation.InvokePattern]$invokeObject).Invoke()
    $semanticControls += [ordered]@{
        name = 'Exit'
        control_type = 'ControlType.MenuItem'
        action = 'Invoke.Invoke'
    }

    $remainingMilliseconds = [Math]::Max(
        1,
        [int][Math]::Ceiling(($deadline - [DateTime]::UtcNow).TotalMilliseconds))
    Assert-Condition $process.WaitForExit($remainingMilliseconds) `
        "Installed Copperfin Studio did not exit within the bounded $TimeoutSeconds-second UI lifecycle."
    Assert-Condition ($process.ExitCode -eq 0) `
        "Installed Copperfin Studio exited with code $($process.ExitCode)."

    $evidence = [ordered]@{
        schema_version = 1
        kind = 'copperfin-windows-installed-studio-ui-result'
        studio_sha256 = (Get-FileHash -LiteralPath $resolvedStudio -Algorithm SHA256).Hash.ToLowerInvariant()
        fixture_sha256 = (Get-FileHash -LiteralPath $resolvedFixture -Algorithm SHA256).Hash.ToLowerInvariant()
        window_title = $observedWindowTitle
        semantic_controls = @($semanticControls)
        automated_gui = 'PASS'
        graceful_exit = 'PASS'
        human_gui = 'NOT_RUN'
    }
    $evidence | ConvertTo-Json -Depth 6 | Set-Content `
        -LiteralPath (Join-Path $resolvedEvidenceDirectory 'windows-installed-studio-ui.json') `
        -Encoding utf8
    $success = $true
    Write-Host "Installed Copperfin Studio semantic UI lifecycle passed for $resolvedStudio"
}
catch {
    $primaryError = $_
    $screenshotPath = Join-Path $resolvedEvidenceDirectory 'windows-installed-studio-ui-failure.png'
    $screenshotResult = 'NOT_CAPTURED'
    try {
        Save-UiScreenshot -Root $root -Path $screenshotPath
        $screenshotResult = [System.IO.Path]::GetFileName($screenshotPath)
    }
    catch {
        $screenshotResult = "CAPTURE_FAILED: $($_.Exception.Message)"
    }
    $diagnostic = [ordered]@{
        schema_version = 1
        kind = 'copperfin-windows-installed-studio-ui-diagnostic'
        error = $primaryError.Exception.Message
        process_id = if ($processStarted) { $process.Id } else { $null }
        process_exited = if ($processStarted) { $process.HasExited } else { $null }
        screenshot = $screenshotResult
        ui_tree = @(Get-UiTreeSnapshot -Root $root)
    }
    $diagnostic | ConvertTo-Json -Depth 8 | Set-Content `
        -LiteralPath (Join-Path $resolvedEvidenceDirectory 'windows-installed-studio-ui-diagnostic.json') `
        -Encoding utf8
    throw $primaryError
}
finally {
    if ($null -ne $process) {
        if ($processStarted -and -not $success -and -not $process.HasExited) {
            try { [void]$process.CloseMainWindow() } catch {}
            if (-not $process.WaitForExit(5000)) {
                try {
                    & "$env:SystemRoot\System32\taskkill.exe" /PID $process.Id /T /F | Out-Null
                }
                catch {
                    Write-Warning "Installed Studio process tree could not be terminated: $($_.Exception.Message)"
                }
            }
        }
        $process.Dispose()
    }
}
