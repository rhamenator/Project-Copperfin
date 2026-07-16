# Copyright (c) 2026 Richard M. Hamilton. All rights reserved.
# Licensed under the Project Copperfin Source-Available License or
# Commercial License. See LICENSE.md in the repository root.

[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [ValidateNotNullOrEmpty()]
    [string]$EvidencePath
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Get-CountTotal {
    param($PerLanguageCount)

    $total = [long]0
    if ($null -ne $PerLanguageCount -and $null -ne $PerLanguageCount.counts) {
        foreach ($property in $PerLanguageCount.counts.PSObject.Properties) {
            $total += [long]$property.Value
        }
    }
    return $total
}

function Stop-SccacheServer {
    & $env:SCCACHE_PATH --stop-server 2>&1 | Out-Null
}

function Reset-SccacheStats {
    & $env:SCCACHE_PATH --zero-stats 2>&1 | Out-Null
    if ($LASTEXITCODE -ne 0) {
        throw "sccache could not reset its statistics."
    }
}

function Get-SccacheStats {
    $json = (& $env:SCCACHE_PATH --show-stats --stats-format=json 2>&1 | Out-String).Trim()
    if ($LASTEXITCODE -ne 0) {
        throw "sccache could not report contract-probe statistics."
    }
    return ($json | ConvertFrom-Json).stats
}

function Invoke-ProbeCompile {
    param(
        [Parameter(Mandatory = $true)][string]$ObjectPath,
        [string[]]$Definitions = @(),
        [switch]$AllowFailure
    )

    Remove-Item -LiteralPath $ObjectPath -Force -ErrorAction SilentlyContinue
    $arguments = @(
        "cl.exe", "/nologo", "/c", "/EHsc", "/std:c++20", "/Z7", "/Brepro",
        "/I$probeRoot"
    ) + $Definitions + @($sourcePath, "/Fo$ObjectPath")
    & $env:SCCACHE_PATH @arguments 2>&1 | Write-Host
    $exitCode = $LASTEXITCODE
    if ($exitCode -ne 0 -and -not $AllowFailure) {
        throw "The sccache contract probe compile failed with exit code $exitCode."
    }
    return $exitCode
}

if (-not $IsWindows) {
    throw "The sccache compiler contract probe is Windows-only."
}
if ([string]::IsNullOrWhiteSpace($env:SCCACHE_PATH)) {
    throw "SCCACHE_PATH is required."
}
Get-Command cl.exe -ErrorAction Stop | Out-Null

$evidenceFile = [System.IO.Path]::GetFullPath($EvidencePath)
$evidenceDirectory = Split-Path -Parent $evidenceFile
$probeRoot = Join-Path $evidenceDirectory "sccache-contract-probe"
$cacheRoot = Join-Path $probeRoot "cache"
$sourcePath = Join-Path $probeRoot "cache_probe.cpp"
$headerPath = Join-Path $probeRoot "cache_probe_generated.h"
$objectPath = Join-Path $probeRoot "cache_probe.obj"

Remove-Item -LiteralPath $probeRoot -Recurse -Force -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Force -Path $cacheRoot | Out-Null

$savedEnvironment = @{}
foreach ($name in @(
        "SCCACHE_GHA_ENABLED", "SCCACHE_GHA_RW_MODE", "SCCACHE_GHA_VERSION",
        "SCCACHE_DIR", "SCCACHE_BASEDIRS", "SCCACHE_CACHE_SIZE")) {
    $savedEnvironment[$name] = [Environment]::GetEnvironmentVariable($name, "Process")
}

$results = [ordered]@{}
try {
    Stop-SccacheServer
    Remove-Item Env:SCCACHE_GHA_ENABLED -ErrorAction SilentlyContinue
    Remove-Item Env:SCCACHE_GHA_RW_MODE -ErrorAction SilentlyContinue
    Remove-Item Env:SCCACHE_GHA_VERSION -ErrorAction SilentlyContinue
    $env:SCCACHE_DIR = $cacheRoot
    $env:SCCACHE_BASEDIRS = $probeRoot
    $env:SCCACHE_CACHE_SIZE = "100M"

    "#define COPPERFIN_GENERATED_VALUE 7" |
        Set-Content -LiteralPath $headerPath -Encoding ascii
    @(
        '#include "cache_probe_generated.h"',
        '#ifndef COPPERFIN_FLAG_VALUE',
        '#define COPPERFIN_FLAG_VALUE 3',
        '#endif',
        'int copperfin_cache_probe() { return COPPERFIN_GENERATED_VALUE + COPPERFIN_FLAG_VALUE; }'
    ) | Set-Content -LiteralPath $sourcePath -Encoding ascii

    Reset-SccacheStats
    Invoke-ProbeCompile -ObjectPath $objectPath | Out-Null
    $coldStats = Get-SccacheStats
    $results.cold = [ordered]@{
        hits = Get-CountTotal $coldStats.cache_hits
        misses = Get-CountTotal $coldStats.cache_misses
    }
    if ($results.cold.misses -ne 1 -or $results.cold.hits -ne 0) {
        throw "The first focused compile did not produce exactly one cache miss."
    }

    Stop-SccacheServer
    Reset-SccacheStats
    Invoke-ProbeCompile -ObjectPath $objectPath | Out-Null
    $repeatStats = Get-SccacheStats
    $results.repeat = [ordered]@{
        hits = Get-CountTotal $repeatStats.cache_hits
        misses = Get-CountTotal $repeatStats.cache_misses
    }
    if ($results.repeat.hits -ne 1 -or $results.repeat.misses -ne 0) {
        throw "The identical focused compile did not produce exactly one cache hit."
    }

    Stop-SccacheServer
    Reset-SccacheStats
    Add-Content -LiteralPath $sourcePath -Value "// source invalidation" -Encoding ascii
    Invoke-ProbeCompile -ObjectPath $objectPath | Out-Null
    $sourceStats = Get-SccacheStats
    $results.source_change = [ordered]@{
        hits = Get-CountTotal $sourceStats.cache_hits
        misses = Get-CountTotal $sourceStats.cache_misses
    }
    if ($results.source_change.misses -ne 1) {
        throw "A source change did not invalidate the focused compiler-cache entry."
    }

    Stop-SccacheServer
    Reset-SccacheStats
    "#define COPPERFIN_GENERATED_VALUE 11" |
        Set-Content -LiteralPath $headerPath -Encoding ascii
    Invoke-ProbeCompile -ObjectPath $objectPath | Out-Null
    $headerStats = Get-SccacheStats
    $results.generated_header_change = [ordered]@{
        hits = Get-CountTotal $headerStats.cache_hits
        misses = Get-CountTotal $headerStats.cache_misses
    }
    if ($results.generated_header_change.misses -ne 1) {
        throw "A generated-header change did not invalidate the focused compiler-cache entry."
    }

    Stop-SccacheServer
    Reset-SccacheStats
    Invoke-ProbeCompile -ObjectPath $objectPath -Definitions @("/DCOPPERFIN_FLAG_VALUE=19") | Out-Null
    $flagStats = Get-SccacheStats
    $results.flag_change = [ordered]@{
        hits = Get-CountTotal $flagStats.cache_hits
        misses = Get-CountTotal $flagStats.cache_misses
    }
    if ($results.flag_change.misses -ne 1) {
        throw "A compiler-flag change did not invalidate the focused compiler-cache entry."
    }

    Stop-SccacheServer
    Remove-Item -LiteralPath $cacheRoot -Recurse -Force
    New-Item -ItemType Directory -Force -Path $cacheRoot | Out-Null
    Reset-SccacheStats
    Invoke-ProbeCompile -ObjectPath $objectPath -Definitions @("/DCOPPERFIN_FLAG_VALUE=23") | Out-Null
    $expectedObjectSha256 = (Get-FileHash -LiteralPath $objectPath -Algorithm SHA256).Hash.ToLowerInvariant()
    Stop-SccacheServer

    $preprocessorCacheRoot = Join-Path $cacheRoot "preprocessor"
    $cacheFiles = @(Get-ChildItem -LiteralPath $cacheRoot -Recurse -File |
        Where-Object {
            $_.Length -gt 0 -and
            $_.Name -notmatch '\.(lock|tmp)$' -and
            -not $_.FullName.StartsWith(
                $preprocessorCacheRoot,
                [StringComparison]::OrdinalIgnoreCase)
        })
    if ($cacheFiles.Count -ne 1) {
        throw "The isolated malformed-cache probe expected one cache object; found $($cacheFiles.Count)."
    }
    [System.IO.File]::WriteAllBytes($cacheFiles[0].FullName, [byte[]](0x43, 0x46, 0x00, 0xff))

    Reset-SccacheStats
    $malformedExitCode = Invoke-ProbeCompile -ObjectPath $objectPath `
        -Definitions @("/DCOPPERFIN_FLAG_VALUE=23") -AllowFailure
    $malformedStats = Get-SccacheStats
    $malformedReadErrors = [long]$malformedStats.cache_read_errors
    $malformedRejected = $malformedExitCode -ne 0
    $recompiled = $false
    if (-not $malformedRejected -and (Test-Path -LiteralPath $objectPath -PathType Leaf)) {
        $actualObjectSha256 = (Get-FileHash -LiteralPath $objectPath -Algorithm SHA256).Hash.ToLowerInvariant()
        $recompiled = $malformedReadErrors -gt 0 -and $actualObjectSha256 -eq $expectedObjectSha256
    }
    if (-not $malformedRejected -and -not $recompiled) {
        throw "A malformed cache object was neither rejected nor detectably recompiled from source."
    }
    $results.malformed_cache = [ordered]@{
        exit_code = $malformedExitCode
        cache_read_errors = $malformedReadErrors
        build_rejected = $malformedRejected
        recompiled_from_source = $recompiled
    }
} finally {
    Stop-SccacheServer
    foreach ($name in $savedEnvironment.Keys) {
        $value = $savedEnvironment[$name]
        if ($null -eq $value) {
            [Environment]::SetEnvironmentVariable($name, $null, "Process")
        } else {
            [Environment]::SetEnvironmentVariable($name, $value, "Process")
        }
    }
}

New-Item -ItemType Directory -Force -Path $evidenceDirectory | Out-Null
$evidence = [ordered]@{
    schema_version = 1
    kind = "windows-sccache-contract-probe"
    completed_at_utc = [DateTime]::UtcNow.ToString("o")
    results = $results
}
$evidence | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $evidenceFile -Encoding utf8
Write-Host "Focused sccache hit, invalidation, and malformed-entry contracts passed."
