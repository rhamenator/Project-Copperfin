# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

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
    param([switch]$Cleanup)

    $output = (& $env:SCCACHE_PATH --stop-server 2>&1 | Out-String).Trim()
    if ($LASTEXITCODE -ne 0) {
        if ($output -match "(?m)^sccache: error: couldn't connect to server\r?$") {
            return
        }
        $message = "sccache could not stop its server cleanly."
        if (-not [string]::IsNullOrWhiteSpace($output)) {
            $message = "sccache could not stop its server cleanly: $output"
        }
        if ($Cleanup) {
            Write-Warning $message
            return
        }
        throw $message
    }
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
        [string]$CompilerPath = "cl.exe",
        [string[]]$Definitions = @(),
        [string[]]$CompilerArguments = @(),
        [switch]$AllowFailure
    )

    Remove-Item -LiteralPath $ObjectPath -Force -ErrorAction SilentlyContinue
    $arguments = @(
        $CompilerPath, "/nologo", "/c", "/EHsc", "/std:c++20", "/Z7", "/Brepro",
        "/I$probeRoot"
    ) + $CompilerArguments + $Definitions + @($sourcePath, "/Fo$ObjectPath")
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
$x64CompilerPath = (Get-Command cl.exe -ErrorAction Stop).Source
if ([string]::IsNullOrWhiteSpace($env:llvmX64)) {
    throw "The initialized Visual Studio environment did not expose its x64 LLVM tool directory."
}
$clangCompilerPath = Join-Path $env:llvmX64 "clang-cl.exe"
if (-not (Test-Path -LiteralPath $clangCompilerPath -PathType Leaf)) {
    throw "The Visual Studio x64 clang-cl compiler was not found at $clangCompilerPath."
}
if ([string]::IsNullOrWhiteSpace($env:VCToolsInstallDir)) {
    throw "VCToolsInstallDir is required to locate the x86 MSVC compiler."
}
$x86CompilerPath = Join-Path $env:VCToolsInstallDir "bin\Hostx64\x86\cl.exe"
if (-not (Test-Path -LiteralPath $x86CompilerPath -PathType Leaf)) {
    throw "The x86 MSVC compiler was not found at $x86CompilerPath."
}

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
    Reset-SccacheStats
    Invoke-ProbeCompile -ObjectPath $objectPath `
        -Definitions @("/DCOPPERFIN_FLAG_VALUE=19") -CompilerArguments @("/O2") | Out-Null
    $configurationStats = Get-SccacheStats
    $results.configuration_change = [ordered]@{
        hits = Get-CountTotal $configurationStats.cache_hits
        misses = Get-CountTotal $configurationStats.cache_misses
    }
    if ($results.configuration_change.misses -ne 1) {
        throw "A build-configuration change did not invalidate the focused compiler-cache entry."
    }

    Stop-SccacheServer
    Reset-SccacheStats
    Invoke-ProbeCompile -ObjectPath $objectPath -CompilerPath $x64CompilerPath `
        -Definitions @("/DCOPPERFIN_FLAG_VALUE=29") -CompilerArguments @("/O2") | Out-Null
    $x64Stats = Get-SccacheStats
    if ((Get-CountTotal $x64Stats.cache_misses) -ne 1) {
        throw "The architecture baseline did not produce exactly one cache miss."
    }

    Stop-SccacheServer
    Reset-SccacheStats
    $x86ObjectPath = Join-Path $probeRoot "cache_probe_x86.obj"
    Invoke-ProbeCompile -ObjectPath $x86ObjectPath -CompilerPath $x86CompilerPath `
        -Definitions @("/DCOPPERFIN_FLAG_VALUE=29") -CompilerArguments @("/O2") | Out-Null
    $architectureStats = Get-SccacheStats
    $results.architecture_change = [ordered]@{
        hits = Get-CountTotal $architectureStats.cache_hits
        misses = Get-CountTotal $architectureStats.cache_misses
        x64_compiler_sha256 = (Get-FileHash -LiteralPath $x64CompilerPath -Algorithm SHA256).Hash.ToLowerInvariant()
        x86_compiler_sha256 = (Get-FileHash -LiteralPath $x86CompilerPath -Algorithm SHA256).Hash.ToLowerInvariant()
    }
    if ($results.architecture_change.misses -ne 1) {
        throw "An MSVC target-architecture change did not invalidate the focused compiler-cache entry."
    }

    Stop-SccacheServer
    Reset-SccacheStats
    Invoke-ProbeCompile -ObjectPath $objectPath -CompilerPath $x64CompilerPath `
        -Definitions @("/DCOPPERFIN_FLAG_VALUE=31") -CompilerArguments @("/O2") | Out-Null
    $compilerBaselineStats = Get-SccacheStats
    if ((Get-CountTotal $compilerBaselineStats.cache_misses) -ne 1) {
        throw "The compiler-identity baseline did not produce exactly one cache miss."
    }

    Stop-SccacheServer
    Reset-SccacheStats
    $clangObjectPath = Join-Path $probeRoot "cache_probe_clang.obj"
    Invoke-ProbeCompile -ObjectPath $clangObjectPath -CompilerPath $clangCompilerPath `
        -Definitions @("/DCOPPERFIN_FLAG_VALUE=31") -CompilerArguments @("/O2") | Out-Null
    $compilerStats = Get-SccacheStats
    $results.compiler_change = [ordered]@{
        hits = Get-CountTotal $compilerStats.cache_hits
        misses = Get-CountTotal $compilerStats.cache_misses
        msvc_sha256 = (Get-FileHash -LiteralPath $x64CompilerPath -Algorithm SHA256).Hash.ToLowerInvariant()
        clang_cl_sha256 = (Get-FileHash -LiteralPath $clangCompilerPath -Algorithm SHA256).Hash.ToLowerInvariant()
    }
    if ($results.compiler_change.misses -ne 1) {
        throw "A compiler-identity change did not invalidate the focused compiler-cache entry."
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
    $malformedHits = Get-CountTotal $malformedStats.cache_hits
    $malformedMisses = Get-CountTotal $malformedStats.cache_misses
    $malformedRejected = $malformedExitCode -ne 0
    $recompiled = $false
    $actualObjectSha256 = $null
    if (-not $malformedRejected -and (Test-Path -LiteralPath $objectPath -PathType Leaf)) {
        $actualObjectSha256 = (Get-FileHash -LiteralPath $objectPath -Algorithm SHA256).Hash.ToLowerInvariant()
        $recompiled = $malformedHits -eq 0 -and $malformedMisses -eq 1 -and `
            $actualObjectSha256 -eq $expectedObjectSha256
    }
    if (-not $malformedRejected -and -not $recompiled) {
        throw "A malformed cache object was neither rejected nor recompiled as one clean cache miss " +
            "(hits=$malformedHits, misses=$malformedMisses, read-errors=$malformedReadErrors, " +
            "expected-sha256=$expectedObjectSha256, actual-sha256=$actualObjectSha256)."
    }
    $results.malformed_cache = [ordered]@{
        exit_code = $malformedExitCode
        cache_hits = $malformedHits
        cache_misses = $malformedMisses
        cache_read_errors = $malformedReadErrors
        build_rejected = $malformedRejected
        recompiled_from_source = $recompiled
        expected_object_sha256 = $expectedObjectSha256
        actual_object_sha256 = $actualObjectSha256
    }
} finally {
    try {
        Stop-SccacheServer -Cleanup
    } finally {
        foreach ($name in $savedEnvironment.Keys) {
            $value = $savedEnvironment[$name]
            if ($null -eq $value) {
                [Environment]::SetEnvironmentVariable($name, $null, "Process")
            } else {
                [Environment]::SetEnvironmentVariable($name, $value, "Process")
            }
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
