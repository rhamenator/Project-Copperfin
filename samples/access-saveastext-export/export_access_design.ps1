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
            $outFile = Join-Path $OutputDirectory "$($export.Kind)_$safeName.txt"
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
$manifest | ConvertTo-Json -Depth 4 | Out-File -FilePath $manifestPath -Encoding utf8
Write-Output "Exported $($manifest.Count) object(s). Manifest: $manifestPath"
$failures = $manifest | Where-Object { -not $_.Ok }
if ($failures.Count -gt 0) {
    Write-Output "$($failures.Count) object(s) failed to export:"
    $failures | ForEach-Object { Write-Output "  $($_.Kind) '$($_.Name)': $($_.Error)" }
    exit 1
}
exit 0
