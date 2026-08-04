# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only

[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [ValidateSet("Initialize", "Run", "Finalize")]
    [string]$Mode,

    [Parameter(Mandatory = $true)]
    [ValidateNotNullOrEmpty()]
    [string]$MetricsDirectory,

    [string]$Name = "",

    [ValidateSet("configure", "native-compile", "native-test", "managed-compile", "managed-test", "other")]
    [string]$Category = "other",

    [string]$FilePath = "",

    [string[]]$CommandArguments = @(),

    [ValidateRange(0, 256)]
    [int]$BuildJobs = 0,

    [switch]$SampleResources,

    [ValidateRange(1, 60)]
    [int]$SampleIntervalSeconds = 2
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$metricsRoot = [System.IO.Path]::GetFullPath(
    $(if ([System.IO.Path]::IsPathRooted($MetricsDirectory)) {
        $MetricsDirectory
    } else {
        Join-Path (Get-Location) $MetricsDirectory
    }))
New-Item -ItemType Directory -Force -Path $metricsRoot | Out-Null

function Get-RunnerCapacity {
    $logicalProcessors = [Environment]::ProcessorCount
    $totalPhysicalMemoryBytes = $null
    $operatingSystem = [System.Runtime.InteropServices.RuntimeInformation]::OSDescription

    if ([System.Runtime.InteropServices.RuntimeInformation]::IsOSPlatform(
            [System.Runtime.InteropServices.OSPlatform]::Windows)) {
        $computerSystem = Get-CimInstance -ClassName Win32_ComputerSystem
        $logicalProcessors = [int]$computerSystem.NumberOfLogicalProcessors
        $totalPhysicalMemoryBytes = [long]$computerSystem.TotalPhysicalMemory
    }

    return [ordered]@{
        operating_system = $operatingSystem
        logical_processors = $logicalProcessors
        total_physical_memory_bytes = $totalPhysicalMemoryBytes
    }
}

function ConvertTo-SafeSlug {
    param([Parameter(Mandatory = $true)][string]$Value)

    $slug = $Value.Trim().ToLowerInvariant() -replace '[^a-z0-9]+', '-'
    $slug = $slug.Trim('-')
    if ([string]::IsNullOrWhiteSpace($slug)) {
        throw "A metrics phase name must contain at least one ASCII letter or digit."
    }
    return $slug
}

function Format-Duration {
    param([double]$Seconds)

    return [TimeSpan]::FromSeconds($Seconds).ToString("hh\:mm\:ss")
}

function Format-Bytes {
    param($Bytes)

    if ($null -eq $Bytes -or [long]$Bytes -lt 0) {
        return "n/a"
    }

    return ("{0:N2} GiB" -f ([long]$Bytes / 1GB))
}

function Add-SummaryLine {
    param([Parameter(Mandatory = $true)][AllowEmptyString()][string]$Line)

    if (-not [string]::IsNullOrWhiteSpace($env:GITHUB_STEP_SUMMARY)) {
        Add-Content -Path $env:GITHUB_STEP_SUMMARY -Value $Line -Encoding utf8
    }
}

function Read-PhaseMetrics {
    return @(Get-ChildItem -Path $metricsRoot -Filter "phase-*.json" -File -ErrorAction SilentlyContinue |
        Sort-Object Name |
        ForEach-Object { Get-Content -Raw -Path $_.FullName | ConvertFrom-Json })
}

if ($Mode -eq "Initialize") {
    $startedAtUtc = [DateTime]::UtcNow.ToString("o")
    $runner = Get-RunnerCapacity
    $metadata = [ordered]@{
        schema_version = 1
        kind = "windows-validation-run"
        started_at_utc = $startedAtUtc
        runner = $runner
    }
    $metadata | ConvertTo-Json -Depth 5 | Set-Content -Path (Join-Path $metricsRoot "run.json") -Encoding utf8

    if (-not [string]::IsNullOrWhiteSpace($env:GITHUB_ENV)) {
        Add-Content -Path $env:GITHUB_ENV -Value "COPPERFIN_VALIDATION_STARTED_UTC=$startedAtUtc" -Encoding utf8
    }

    Add-SummaryLine "<!-- copperfin-windows-validation-metrics -->"
    Add-SummaryLine "## Windows validation metrics"
    Add-SummaryLine ""
    Add-SummaryLine ("Runner: {0} logical processors, {1} physical memory." -f
        $runner.logical_processors,
        $(if ($null -eq $runner.total_physical_memory_bytes) { "n/a" } else { Format-Bytes $runner.total_physical_memory_bytes }))
    Add-SummaryLine ""
    Add-SummaryLine "| Phase | Category | Jobs | Elapsed | Minimum free memory | Peak tracked working set | Average tracked CPU | Peak tracked CPU | Exit |"
    Add-SummaryLine "|---|---|---:|---:|---:|---:|---:|---:|---:|"
    Write-Host "Windows validation metrics initialized at $startedAtUtc"
    return
}

if ($Mode -eq "Finalize") {
    $runPath = Join-Path $metricsRoot "run.json"
    if (-not (Test-Path $runPath)) {
        Write-Warning "Windows validation run metadata was not found; metric finalization is skipped: $runPath"
        Add-SummaryLine ""
        Add-SummaryLine "Windows validation metric finalization was skipped because initialization did not emit run metadata."
        return
    }

    $run = Get-Content -Raw -Path $runPath | ConvertFrom-Json
    $finishedAtUtc = [DateTime]::UtcNow
    $startedAtUtc = if ($run.started_at_utc -is [DateTime]) {
        [DateTime]$run.started_at_utc
    } else {
        [DateTime]::Parse(
            [string]$run.started_at_utc,
            [Globalization.CultureInfo]::InvariantCulture,
            [Globalization.DateTimeStyles]::RoundtripKind)
    }
    $phases = Read-PhaseMetrics
    $elapsedSeconds = ($finishedAtUtc - $startedAtUtc).TotalSeconds
    $successful = @($phases | Where-Object { $_.exit_code -eq 0 }).Count

    $summary = [ordered]@{
        schema_version = 1
        kind = "windows-validation-summary"
        started_at_utc = $startedAtUtc.ToString("o")
        finished_at_utc = $finishedAtUtc.ToString("o")
        elapsed_seconds = [Math]::Round($elapsedSeconds, 3)
        phase_count = $phases.Count
        successful_phase_count = $successful
        categories = @($phases |
            Group-Object category |
            Sort-Object Name |
            ForEach-Object {
                [ordered]@{
                    category = $_.Name
                    elapsed_seconds = [Math]::Round((($_.Group | Measure-Object elapsed_seconds -Sum).Sum), 3)
                    phase_count = $_.Count
                }
            })
    }
    $summary | ConvertTo-Json -Depth 6 | Set-Content -Path (Join-Path $metricsRoot "summary.json") -Encoding utf8

    Add-SummaryLine ""
    Add-SummaryLine "### Category totals"
    Add-SummaryLine ""
    Add-SummaryLine "| Category | Phases | Elapsed |"
    Add-SummaryLine "|---|---:|---:|"
    foreach ($categorySummary in $summary.categories) {
        Add-SummaryLine ("| {0} | {1} | {2} |" -f
            $categorySummary.category,
            $categorySummary.phase_count,
            (Format-Duration $categorySummary.elapsed_seconds))
    }
    Add-SummaryLine ""
    Add-SummaryLine ("Measured workflow elapsed: **{0}** across {1} recorded phase(s)." -f
        (Format-Duration $elapsedSeconds),
        $phases.Count)
    Write-Host ("Windows validation metrics finalized: {0} across {1} phase(s)" -f
        (Format-Duration $elapsedSeconds),
        $phases.Count)
    return
}

if ([string]::IsNullOrWhiteSpace($Name)) {
    throw "-Name is required in Run mode."
}
if ([string]::IsNullOrWhiteSpace($FilePath)) {
    throw "-FilePath is required in Run mode."
}

$slug = ConvertTo-SafeSlug $Name
$phasePath = Join-Path $metricsRoot "phase-$slug.json"
$samplePath = Join-Path $metricsRoot ".phase-$slug-samples.csv"
$runner = Get-RunnerCapacity
$runningOnWindows = [System.Runtime.InteropServices.RuntimeInformation]::IsOSPlatform(
    [System.Runtime.InteropServices.OSPlatform]::Windows)
$sampler = $null
$sampleResourceData = $SampleResources.IsPresent -and $runningOnWindows

if ($sampleResourceData) {
    $trackedProcessNames = @(
        "cl", "cmake", "csc", "dotnet", "link", "mspdbsrv", "msbuild", "ninja", "vbc", "vbcscompiler"
    )
    $sampler = Start-Job -ArgumentList @(
        $samplePath,
        $SampleIntervalSeconds,
        [int]$runner.logical_processors,
        ($trackedProcessNames -join ",")
    ) -ScriptBlock {
        param($OutputPath, $IntervalSeconds, $LogicalProcessors, $TrackedProcessNamesCsv)

        Set-StrictMode -Version Latest
        $ErrorActionPreference = "SilentlyContinue"
        $trackedProcessNames = @($TrackedProcessNamesCsv.Split(','))
        "timestamp_utc,free_physical_memory_bytes,tracked_working_set_bytes,tracked_process_count,tracked_cpu_percent" |
            Set-Content -Path $OutputPath -Encoding utf8
        $previousCpuById = @{}
        $previousSampleAt = [DateTime]::UtcNow
        $hasPreviousSample = $false

        while ($true) {
            $sampledAt = [DateTime]::UtcNow
            $elapsedSeconds = [Math]::Max(($sampledAt - $previousSampleAt).TotalSeconds, 0.001)
            $processes = @(Get-Process | Where-Object {
                $trackedProcessNames -contains $_.ProcessName.ToLowerInvariant()
            })
            $workingSetBytes = [long]0
            $cpuDeltaSeconds = [double]0
            $currentCpuById = @{}

            foreach ($process in $processes) {
                try {
                    $processId = [int]$process.Id
                    $cpuSeconds = [double]$process.CPU
                    $workingSetBytes += [long]$process.WorkingSet64
                    $currentCpuById[$processId] = $cpuSeconds
                    if ($previousCpuById.ContainsKey($processId)) {
                        $cpuDeltaSeconds += [Math]::Max($cpuSeconds - [double]$previousCpuById[$processId], 0.0)
                    } elseif ($hasPreviousSample) {
                        $cpuDeltaSeconds += [Math]::Max($cpuSeconds, 0.0)
                    }
                } catch {
                }
            }

            $cpuPercent = 100.0 * $cpuDeltaSeconds / ($elapsedSeconds * [Math]::Max([int]$LogicalProcessors, 1))
            $operatingSystem = Get-CimInstance -ClassName Win32_OperatingSystem
            $freeMemoryBytes = [long]$operatingSystem.FreePhysicalMemory * 1KB
            $line = [string]::Format(
                [Globalization.CultureInfo]::InvariantCulture,
                "{0:o},{1},{2},{3},{4:F2}",
                $sampledAt,
                $freeMemoryBytes,
                $workingSetBytes,
                $processes.Count,
                $cpuPercent)
            Add-Content -Path $OutputPath -Value $line -Encoding utf8
            $previousCpuById = $currentCpuById
            $previousSampleAt = $sampledAt
            $hasPreviousSample = $true
            Start-Sleep -Seconds $IntervalSeconds
        }
    }

    $sampleReadyDeadline = [DateTime]::UtcNow.AddSeconds(10)
    while (-not (Test-Path $samplePath) -and [DateTime]::UtcNow -lt $sampleReadyDeadline) {
        Start-Sleep -Milliseconds 100
    }
}

$startedAtUtc = [DateTime]::UtcNow
$stopwatch = [Diagnostics.Stopwatch]::StartNew()
$exitCode = 0
$commandError = $null
Write-Host ""
Write-Host ("==> {0} [{1}]" -f $Name, $Category) -ForegroundColor Cyan
Write-Host ("Command: {0} {1}" -f $FilePath, ($CommandArguments -join " "))

try {
    $global:LASTEXITCODE = 0
    & $FilePath @CommandArguments
    if ($null -ne $LASTEXITCODE) {
        $exitCode = [int]$LASTEXITCODE
    } elseif (-not $?) {
        $exitCode = 1
    }
} catch {
    $exitCode = 1
    $commandError = $_
} finally {
    $stopwatch.Stop()
    if ($null -ne $sampler) {
        Stop-Job -Job $sampler -ErrorAction SilentlyContinue
        Remove-Job -Job $sampler -Force -ErrorAction SilentlyContinue
    }
}

$samples = @()
if (Test-Path $samplePath) {
    $samples = @(Import-Csv -Path $samplePath)
    Remove-Item -Force -Path $samplePath -ErrorAction SilentlyContinue
}

$minimumFreeMemoryBytes = $null
$peakTrackedWorkingSetBytes = $null
$averageTrackedCpuPercent = $null
$peakTrackedCpuPercent = $null
if ($samples.Count -gt 0) {
    $minimumFreeMemoryBytes = [long](($samples | Measure-Object free_physical_memory_bytes -Minimum).Minimum)
    $peakTrackedWorkingSetBytes = [long](($samples | Measure-Object tracked_working_set_bytes -Maximum).Maximum)
    $averageTrackedCpuPercent = [Math]::Round(
        [double](($samples | Measure-Object tracked_cpu_percent -Average).Average),
        2)
    $peakTrackedCpuPercent = [Math]::Round(
        [double](($samples | Measure-Object tracked_cpu_percent -Maximum).Maximum),
        2)
}

$finishedAtUtc = [DateTime]::UtcNow
$phase = [ordered]@{
    schema_version = 1
    kind = "windows-validation-phase"
    name = $Name
    category = $Category
    command = $FilePath
    arguments = @($CommandArguments)
    build_jobs = $(if ($BuildJobs -gt 0) { $BuildJobs } else { $null })
    started_at_utc = $startedAtUtc.ToString("o")
    finished_at_utc = $finishedAtUtc.ToString("o")
    elapsed_seconds = [Math]::Round($stopwatch.Elapsed.TotalSeconds, 3)
    exit_code = $exitCode
    sample_interval_seconds = $(if ($sampleResourceData) { $SampleIntervalSeconds } else { $null })
    resource_sample_count = $samples.Count
    minimum_free_physical_memory_bytes = $minimumFreeMemoryBytes
    peak_tracked_working_set_bytes = $peakTrackedWorkingSetBytes
    average_tracked_cpu_percent = $averageTrackedCpuPercent
    peak_tracked_cpu_percent = $peakTrackedCpuPercent
    runner = $runner
}
$phase | ConvertTo-Json -Depth 6 | Set-Content -Path $phasePath -Encoding utf8

$safeName = $Name.Replace("|", "\|")
$jobsText = if ($BuildJobs -gt 0) { $BuildJobs.ToString() } else { "n/a" }
$minimumFreeMemoryText = if ($null -eq $minimumFreeMemoryBytes) { "n/a" } else { Format-Bytes $minimumFreeMemoryBytes }
$peakWorkingSetText = if ($null -eq $peakTrackedWorkingSetBytes) { "n/a" } else { Format-Bytes $peakTrackedWorkingSetBytes }
$averageCpuText = if ($null -eq $averageTrackedCpuPercent) { "n/a" } else { "{0:N2}%" -f $averageTrackedCpuPercent }
$peakCpuText = if ($null -eq $peakTrackedCpuPercent) { "n/a" } else { "{0:N2}%" -f $peakTrackedCpuPercent }
Add-SummaryLine ("| {0} | {1} | {2} | {3} | {4} | {5} | {6} | {7} | {8} |" -f
    $safeName,
    $Category,
    $jobsText,
    (Format-Duration $stopwatch.Elapsed.TotalSeconds),
    $minimumFreeMemoryText,
    $peakWorkingSetText,
    $averageCpuText,
    $peakCpuText,
    $exitCode)

Write-Host ("Phase metrics: elapsed={0}; samples={1}; minimum-free-memory={2}; peak-tracked-working-set={3}; average-tracked-cpu={4}; peak-tracked-cpu={5}; exit={6}" -f
    (Format-Duration $stopwatch.Elapsed.TotalSeconds),
    $samples.Count,
    $minimumFreeMemoryText,
    $peakWorkingSetText,
    $averageCpuText,
    $peakCpuText,
    $exitCode)

if ($null -ne $commandError) {
    throw $commandError
}
if ($exitCode -ne 0) {
    throw "Command failed with exit code ${exitCode}: $FilePath $($CommandArguments -join ' ')"
}
