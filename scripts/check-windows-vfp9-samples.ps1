# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

[CmdletBinding()]
param(
    [string]$Root = "",
    [switch]$Require,
    [Parameter(Mandatory = $true)]
    [string]$OutputPath
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

if ([string]::IsNullOrWhiteSpace($Root)) {
    $Root = $env:COPPERFIN_VFP9_ROOT
}
if ([string]::IsNullOrWhiteSpace($Root)) {
    $Root = "C:\Program Files (x86)\Microsoft Visual FoxPro 9"
}

$requiredPaths = @(
    (Join-Path $Root "Samples\Solution\solution.pjx"),
    (Join-Path $Root "Wizards\Template\Books\Forms\books.scx"),
    (Join-Path $Root "Samples\Solution\Reports\invoice.frx"),
    (Join-Path $Root "Samples\Solution\Toledo\systray_shortcut.mnx")
)
$missingPaths = @($requiredPaths | Where-Object {
    -not (Test-Path -LiteralPath $_ -PathType Leaf)
})
$available = $missingPaths.Count -eq 0

$result = [ordered]@{
    schema_version = 1
    kind = "windows-vfp9-sample-prerequisite"
    root = $Root
    required_mode = $Require.IsPresent
    available = $available
    required_paths = $requiredPaths
    missing_paths = $missingPaths
}
$parent = Split-Path -Parent $OutputPath
New-Item -ItemType Directory -Force -Path $parent | Out-Null
$result | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath $OutputPath -Encoding utf8

if (-not [string]::IsNullOrWhiteSpace($env:GITHUB_ENV)) {
    "COPPERFIN_VFP9_ROOT=$Root" | Out-File -FilePath $env:GITHUB_ENV -Encoding utf8 -Append
}
if (-not [string]::IsNullOrWhiteSpace($env:GITHUB_OUTPUT)) {
    "available=$($available.ToString().ToLowerInvariant())" | Out-File -FilePath $env:GITHUB_OUTPUT -Encoding utf8 -Append
}

if ($available) {
    Write-Host "VFP9 sample prerequisite is available: $Root" -ForegroundColor Green
} elseif ($Require) {
    throw "Required VFP9 sample prerequisite is unavailable. See $OutputPath and set COPPERFIN_VFP9_ROOT to the installed VFP9 root."
} else {
    Write-Warning "VFP9 sample smoke is skipped because required assets are unavailable. See $OutputPath; no VFP9 sample coverage is claimed."
}
