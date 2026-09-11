# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
#
# Exports every Form, Report, and standalone Module in an Access
# database as Application.SaveAsText output, for Copperfin's
# access_saveastext_design.cpp parser (#5477) to consume. See this
# directory's own README.md for status and scope.
#
# Usage: powershell -ExecutionPolicy Bypass -File export_access_design.ps1 <db-path> <output-dir>
#
# Security: forces AutomationSecurity to msoAutomationSecurityForceDisable
# (3) BEFORE opening the database, so an AutoExec macro or startup form
# in an untrusted/malicious input file cannot execute under this
# automation's authority. This is a hard requirement, not optional
# hardening -- see docs/78-access-forms-reports-vba-storage-
# reconnaissance.md's "Security requirement for any automation helper"
# section. Verified this does not change SaveAsText's own output
# (byte-for-byte identical export with and without the property set).
param(
    [Parameter(Mandatory = $true)][string]$DatabasePath,
    [Parameter(Mandatory = $true)][string]$OutputDirectory
)

$ErrorActionPreference = "Stop"

if (-not (Test-Path -LiteralPath $DatabasePath)) {
    Write-Error "Database not found: $DatabasePath"
    exit 1
}
New-Item -ItemType Directory -Force -Path $OutputDirectory | Out-Null

function ConvertTo-SafeFileName([string]$name) {
    return ($name -replace '[\\/:*?"<>|]', '_')
}

# Replacing invalid filename characters is not injective -- distinct
# Access object names (e.g. "A/B" and "A:B") can sanitize to the same
# string. Tracks every path this run has already produced and appends a
# deterministic numeric disambiguator on collision, so two distinct
# objects can never silently share one output file.
$usedPaths = New-Object 'System.Collections.Generic.HashSet[string]'
function Get-UniqueOutputPath([string]$directory, [string]$baseName) {
    $candidate = Join-Path $directory "$baseName.txt"
    $suffix = 2
    while ($usedPaths.Contains($candidate)) {
        $candidate = Join-Path $directory "$baseName ($suffix).txt"
        $suffix++
    }
    [void]$usedPaths.Add($candidate)
    return $candidate
}

$manifest = New-Object System.Collections.ArrayList
$access = $null
try {
    $access = New-Object -ComObject Access.Application
    $access.Visible = $false
    $access.UserControl = $false
    # msoAutomationSecurityForceDisable = 3.
    $access.AutomationSecurity = 3
    $access.OpenCurrentDatabase($DatabasePath, $false)

    # acForm=2, acReport=3, acModule=5 (Application.SaveAsText's own
    # ObjectType constants -- see docs/79's own grammar notes). All
    # three kinds are enumerated because most real-world VBA lives in
    # form/report code-behind, not standalone modules -- see docs/78's
    # own "complete VBA-extraction" note.
    $exports = @(
        @{ Kind = "Form"; Type = 2; Items = $access.CurrentProject.AllForms }
        @{ Kind = "Report"; Type = 3; Items = $access.CurrentProject.AllReports }
        @{ Kind = "Module"; Type = 5; Items = $access.CurrentProject.AllModules }
    )

    foreach ($export in $exports) {
        foreach ($item in $export.Items) {
            $name = $item.Name
            $safeName = ConvertTo-SafeFileName $name
            $outFile = Get-UniqueOutputPath $OutputDirectory "$($export.Kind)_$safeName"
            try {
                $access.SaveAsText($export.Type, $name, $outFile)
                [void]$manifest.Add([PSCustomObject]@{
                    Kind = $export.Kind
                    Name = $name
                    File = $outFile
                    Ok = $true
                    Error = ""
                })
            } catch {
                [void]$manifest.Add([PSCustomObject]@{
                    Kind = $export.Kind
                    Name = $name
                    File = $outFile
                    Ok = $false
                    Error = $_.Exception.Message
                })
            }
        }
    }

    $access.CloseCurrentDatabase()
} finally {
    if ($null -ne $access) {
        try { $access.Quit() } catch {}
    }
}

$manifestPath = Join-Path $OutputDirectory "manifest.json"
# ConvertTo-Json's own single-element-collapses-to-a-bare-object
# behavior (the reason -AsArray exists in PowerShell 6.2+, unavailable
# on this script's Windows PowerShell 5.1 target) applies to *pipeline*
# input (e.g. `$manifest | ConvertTo-Json`), not to an array passed via
# -InputObject the way this script already does. -InputObject
# @($manifest) -Depth 4 already serializes a one-element array as a
# proper `[ {...} ]`, directly confirmed against a real PowerShell
# engine -- an earlier version of this script wrapped that already-
# correct output in a second, redundant pair of brackets for exactly
# one object, producing `[[{...}]]`, a real bug caught in #5562's own
# review (chatgpt-codex-connector and copilot-pull-request-reviewer,
# independently). Only the genuinely different zero-element case (an
# empty collection produces no ConvertTo-Json output at all) still
# needs its own explicit branch.
if ($manifest.Count -eq 0) {
    $manifestJson = "[]"
} else {
    $manifestJson = ConvertTo-Json -InputObject @($manifest) -Depth 4
}
$manifestJson | Out-File -FilePath $manifestPath -Encoding utf8
Write-Output "Exported $($manifest.Count) object(s). Manifest: $manifestPath"
# Wrapped in @(...) so a filtered result of exactly one item is still
# treated as a one-element array rather than an unwrapped scalar
# PSCustomObject (which has no .Count property, silently evaluating the
# check below as "no failures" and letting a real failure exit 0).
$failures = @($manifest | Where-Object { -not $_.Ok })
if ($failures.Count -gt 0) {
    Write-Output "$($failures.Count) object(s) failed to export:"
    $failures | ForEach-Object { Write-Output "  $($_.Kind) '$($_.Name)': $($_.Error)" }
    exit 1
}
exit 0
