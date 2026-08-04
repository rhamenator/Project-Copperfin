# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only

[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [ValidateNotNullOrEmpty()]
    [string]$MetricsDirectory,

    [ValidateRange(1.0, 100.0)]
    [double]$MinimumImprovementPercent = 25.0,

    [ValidateRange(1.0, 100.0)]
    [double]$MinimumColdMissPercent = 90.0,

    [ValidateRange(1.0, 100.0)]
    [double]$MinimumWarmHitPercent = 90.0
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
$coldRequests = [long]$coldStats.compile_requests
$coldMisses = [long]$coldStats.cache_misses
$coldCacheableRequests = [long]$coldStats.cache_hits + $coldMisses
$coldMissPercent = if ($coldCacheableRequests -gt 0) {
    100.0 * $coldMisses / $coldCacheableRequests
} else { 0.0 }
$warmRequests = [long]$warmStats.compile_requests
$warmHits = [long]$warmStats.cache_hits
$warmCacheableRequests = $warmHits + [long]$warmStats.cache_misses
$warmHitPercent = if ($warmCacheableRequests -gt 0) {
    100.0 * $warmHits / $warmCacheableRequests
} else { 0.0 }

$evidence = [ordered]@{
    schema_version = 1
    kind = "windows-sccache-evaluation"
    cold_compile_seconds = [Math]::Round($coldSeconds, 3)
    warm_compile_seconds = [Math]::Round($warmSeconds, 3)
    compile_improvement_percent = [Math]::Round($improvementPercent, 2)
    cold_compile_requests = $coldRequests
    cold_cacheable_requests = $coldCacheableRequests
    cold_cache_misses = $coldMisses
    cold_cache_write_errors = [long]$coldStats.raw_stats.cache_write_errors
    cold_miss_percent = [Math]::Round($coldMissPercent, 2)
    warm_compile_requests = $warmRequests
    warm_cacheable_requests = $warmCacheableRequests
    warm_cache_hits = $warmHits
    warm_cache_misses = [long]$warmStats.cache_misses
    warm_cache_write_errors = [long]$warmStats.raw_stats.cache_write_errors
    warm_hit_percent = [Math]::Round($warmHitPercent, 2)
    minimum_improvement_percent = $MinimumImprovementPercent
    minimum_cold_miss_percent = $MinimumColdMissPercent
    minimum_warm_hit_percent = $MinimumWarmHitPercent
    materially_faster = $improvementPercent -ge $MinimumImprovementPercent -and
        $coldMissPercent -ge $MinimumColdMissPercent -and
        $warmHitPercent -ge $MinimumWarmHitPercent
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

if ($coldMissPercent -lt $MinimumColdMissPercent) {
    throw ("The cold build miss ratio was {0:N2}%; at least {1:N2}% was required." -f `
        $coldMissPercent, $MinimumColdMissPercent)
}
if ($warmHitPercent -lt $MinimumWarmHitPercent) {
    throw ("The warm build hit ratio was {0:N2}%; at least {1:N2}% was required." -f `
        $warmHitPercent, $MinimumWarmHitPercent)
}
if ($improvementPercent -lt $MinimumImprovementPercent) {
    throw ("Warm compilation improved by {0:N2}%; at least {1:N2}% was required." -f `
        $improvementPercent, $MinimumImprovementPercent)
}

Write-Host ("Warm compilation improved by {0:N2}% with {1} cache hit(s)." -f `
    $improvementPercent, $warmHits)
