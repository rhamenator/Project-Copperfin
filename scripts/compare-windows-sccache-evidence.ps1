# Copyright (c) 2026 Richard M. Hamilton. All rights reserved.
# Licensed under the Project Copperfin Source-Available License or
# Commercial License. See LICENSE.md in the repository root.

[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [ValidateNotNullOrEmpty()]
    [string]$MetricsDirectory,

    [ValidateRange(1.0, 100.0)]
    [double]$MinimumImprovementPercent = 25.0
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$metricsRoot = [System.IO.Path]::GetFullPath($MetricsDirectory)
$coldPhasePath = Join-Path $metricsRoot "phase-build-cold-native-targets.json"
$warmPhasePath = Join-Path $metricsRoot "phase-build-warm-native-targets.json"
$coldStatsPath = Join-Path $metricsRoot "sccache-cold.json"
$warmStatsPath = Join-Path $metricsRoot "sccache-warm.json"

foreach ($requiredPath in @($coldPhasePath, $warmPhasePath, $coldStatsPath, $warmStatsPath)) {
    if (-not (Test-Path -LiteralPath $requiredPath -PathType Leaf)) {
        throw "Compiler-cache comparison evidence is missing: $requiredPath"
    }
}

$coldPhase = Get-Content -Raw -LiteralPath $coldPhasePath | ConvertFrom-Json
$warmPhase = Get-Content -Raw -LiteralPath $warmPhasePath | ConvertFrom-Json
$coldStats = Get-Content -Raw -LiteralPath $coldStatsPath | ConvertFrom-Json
$warmStats = Get-Content -Raw -LiteralPath $warmStatsPath | ConvertFrom-Json

$coldSeconds = [double]$coldPhase.elapsed_seconds
$warmSeconds = [double]$warmPhase.elapsed_seconds
if ($coldSeconds -le 0.0 -or $warmSeconds -le 0.0) {
    throw "Compiler-cache phase durations must be positive."
}

$improvementPercent = 100.0 * ($coldSeconds - $warmSeconds) / $coldSeconds
$warmRequests = [long]$warmStats.compile_requests
$warmHits = [long]$warmStats.cache_hits
$warmHitPercent = if ($warmRequests -gt 0) { 100.0 * $warmHits / $warmRequests } else { 0.0 }

$evidence = [ordered]@{
    schema_version = 1
    kind = "windows-sccache-evaluation"
    cold_compile_seconds = [Math]::Round($coldSeconds, 3)
    warm_compile_seconds = [Math]::Round($warmSeconds, 3)
    compile_improvement_percent = [Math]::Round($improvementPercent, 2)
    cold_cache_misses = [long]$coldStats.cache_misses
    warm_compile_requests = $warmRequests
    warm_cache_hits = $warmHits
    warm_cache_misses = [long]$warmStats.cache_misses
    warm_hit_percent = [Math]::Round($warmHitPercent, 2)
    minimum_improvement_percent = $MinimumImprovementPercent
    materially_faster = $improvementPercent -ge $MinimumImprovementPercent
}
$outputPath = Join-Path $metricsRoot "sccache-evaluation.json"
$evidence | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath $outputPath -Encoding utf8

if (-not [string]::IsNullOrWhiteSpace($env:GITHUB_STEP_SUMMARY)) {
    @(
        "",
        "## MSVC compiler-cache evaluation",
        "",
        "| Cold compile | Warm compile | Improvement | Warm hits | Warm misses |",
        "|---:|---:|---:|---:|---:|",
        ("| {0:N3} s | {1:N3} s | {2:N2}% | {3} | {4} |" -f `
            $coldSeconds, $warmSeconds, $improvementPercent, $warmHits, [long]$warmStats.cache_misses)
    ) | Add-Content -LiteralPath $env:GITHUB_STEP_SUMMARY -Encoding utf8
}

if ($warmHits -le 0) {
    throw "The warm full build did not record a compiler-cache hit."
}
if ($improvementPercent -lt $MinimumImprovementPercent) {
    throw ("Warm compilation improved by {0:N2}%; at least {1:N2}% was required." -f `
        $improvementPercent, $MinimumImprovementPercent)
}

Write-Host ("Warm compilation improved by {0:N2}% with {1} cache hit(s)." -f `
    $improvementPercent, $warmHits)
