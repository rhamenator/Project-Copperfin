# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only

[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [ValidateNotNullOrEmpty()]
    [string]$Name,

    [Parameter(Mandatory = $true)]
    [ValidateNotNullOrEmpty()]
    [string]$OutputPath,

    [ValidateRange(0, [long]::MaxValue)]
    [long]$MinimumHits = 0,

    [ValidateRange(0, [long]::MaxValue)]
    [long]$MinimumMisses = 0,

    [ValidateRange(0, [long]::MaxValue)]
    [long]$MaximumWriteErrors = 0
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Get-CountTotal {
    param($PerLanguageCount)

    if ($null -eq $PerLanguageCount -or $null -eq $PerLanguageCount.counts) {
        return [long]0
    }

    $total = [long]0
    foreach ($property in $PerLanguageCount.counts.PSObject.Properties) {
        $total += [long]$property.Value
    }
    return $total
}

if ([string]::IsNullOrWhiteSpace($env:SCCACHE_PATH)) {
    throw "SCCACHE_PATH is required."
}

$statsJson = (& $env:SCCACHE_PATH --show-stats --stats-format=json 2>&1 | Out-String).Trim()
if ($LASTEXITCODE -ne 0) {
    throw "sccache statistics failed with exit code $LASTEXITCODE."
}

$payload = $statsJson | ConvertFrom-Json
if ($null -eq $payload.stats) {
    throw "sccache statistics did not contain the expected stats object."
}

$hits = Get-CountTotal $payload.stats.cache_hits
$misses = Get-CountTotal $payload.stats.cache_misses
$requests = [long]$payload.stats.compile_requests
$readErrors = [long]$payload.stats.cache_read_errors
$writeErrors = [long]$payload.stats.cache_write_errors
$otherErrors = Get-CountTotal $payload.stats.cache_errors
$errors = $readErrors + $writeErrors + $otherErrors

$evidence = [ordered]@{
    schema_version = 1
    kind = "windows-sccache-stats"
    name = $Name
    captured_at_utc = [DateTime]::UtcNow.ToString("o")
    compile_requests = $requests
    cacheable_requests = $hits + $misses
    executed_requests = [long]$payload.stats.requests_executed
    cache_hits = $hits
    cache_misses = $misses
    cache_errors = $errors
    cache_read_errors = $readErrors
    cache_write_errors = $writeErrors
    other_cache_errors = $otherErrors
    cache_location = [string]$payload.cache_location
    cache_size = $payload.cache_size
    max_cache_size = $payload.max_cache_size
    sccache_version = [string]$payload.version
    raw_stats = $payload.stats
}

$outputFile = [System.IO.Path]::GetFullPath($OutputPath)
$outputDirectory = Split-Path -Parent $outputFile
New-Item -ItemType Directory -Force -Path $outputDirectory | Out-Null
$evidence | ConvertTo-Json -Depth 12 | Set-Content -LiteralPath $outputFile -Encoding utf8

if ($hits -lt $MinimumHits) {
    throw "$Name recorded $hits cache hit(s); at least $MinimumHits were required."
}
if ($misses -lt $MinimumMisses) {
    throw "$Name recorded $misses cache miss(es); at least $MinimumMisses were required."
}
if ($readErrors -ne 0 -or $otherErrors -ne 0) {
    throw "$Name recorded $readErrors read error(s) and $otherErrors other cache error(s)."
}
if ($writeErrors -gt $MaximumWriteErrors) {
    throw "$Name recorded $writeErrors cache write error(s); at most $MaximumWriteErrors were allowed."
}

Write-Host "${Name}: requests=$requests hits=$hits misses=$misses read-errors=$readErrors write-errors=$writeErrors other-errors=$otherErrors"
