# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only

[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [ValidateNotNullOrEmpty()]
    [string]$EvidencePath
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$expectedVersion = "0.15.0"
$expectedExecutableSha256 = "e68e38e5b548f015dfc47c76d6cfbe67a610034408961f2b8693828b728999f8"

if ([string]::IsNullOrWhiteSpace($env:SCCACHE_PATH)) {
    throw "SCCACHE_PATH was not exported by the pinned setup action."
}

$candidates = @($env:SCCACHE_PATH, "$($env:SCCACHE_PATH).exe")
$sccachePath = $null
foreach ($candidate in $candidates) {
    if (Test-Path -LiteralPath $candidate -PathType Leaf) {
        $sccachePath = (Resolve-Path -LiteralPath $candidate).Path
        break
    }
}
if ($null -eq $sccachePath) {
    throw "The pinned sccache executable was not found at SCCACHE_PATH."
}

$actualSha256 = (Get-FileHash -LiteralPath $sccachePath -Algorithm SHA256).Hash.ToLowerInvariant()
if ($actualSha256 -ne $expectedExecutableSha256) {
    throw "The sccache executable SHA-256 does not match the reviewed v$expectedVersion artifact."
}

$versionOutput = (& $sccachePath --version 2>&1 | Out-String).Trim()
if ($LASTEXITCODE -ne 0 -or $versionOutput -notmatch "^sccache $([regex]::Escape($expectedVersion))($|\s)") {
    throw "The verified sccache executable did not report version $expectedVersion."
}

$evidenceFile = [System.IO.Path]::GetFullPath($EvidencePath)
$evidenceDirectory = Split-Path -Parent $evidenceFile
New-Item -ItemType Directory -Force -Path $evidenceDirectory | Out-Null

$evidence = [ordered]@{
    schema_version = 1
    kind = "windows-sccache-provenance"
    version = $expectedVersion
    executable_sha256 = $actualSha256
    executable_path = $sccachePath
    release_asset = "sccache-v0.15.0-x86_64-pc-windows-msvc.tar.gz"
    release_asset_sha256 = "b0b257a164bf438b2dea134ca7ded41c100f59a64b3bf275a202f1e8102ab217"
    setup_action_commit = "9e7fa8a12102821edf02ca5dbea1acd0f89a2696"
}
$evidence | ConvertTo-Json -Depth 4 | Set-Content -LiteralPath $evidenceFile -Encoding utf8

$env:SCCACHE_PATH = $sccachePath
if (-not [string]::IsNullOrWhiteSpace($env:GITHUB_ENV)) {
    Add-Content -LiteralPath $env:GITHUB_ENV -Value "SCCACHE_PATH=$sccachePath" -Encoding utf8
}

Write-Host "Verified sccache $expectedVersion executable SHA-256: $actualSha256"
