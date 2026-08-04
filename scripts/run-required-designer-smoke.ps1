# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [ValidateNotNullOrEmpty()]
    [string]$ExecutablePath,

    [ValidateRange(1, 7200)]
    [int]$TimeoutSeconds = 1800
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

if (-not (Test-Path -LiteralPath $ExecutablePath -PathType Leaf)) {
    throw "Designer smoke executable was not found: $ExecutablePath"
}

$tempRoot = Join-Path ([System.IO.Path]::GetTempPath()) ("copperfin-designer-smoke-" + [Guid]::NewGuid().ToString("N"))
New-Item -ItemType Directory -Force -Path $tempRoot | Out-Null
$stdoutPath = Join-Path $tempRoot "stdout.log"
$stderrPath = Join-Path $tempRoot "stderr.log"
$statusPath = Join-Path $tempRoot "status.txt"
$process = $null

try {
    $process = Start-Process -FilePath $ExecutablePath -ArgumentList @("--status-file", ('"' + $statusPath + '"')) -PassThru -NoNewWindow -RedirectStandardOutput $stdoutPath -RedirectStandardError $stderrPath
    $timeoutMilliseconds = $TimeoutSeconds * 1000
    $timedOut = -not $process.WaitForExit($timeoutMilliseconds)
    if ($timedOut) {
        try {
            $process.Kill()
        }
        catch {
            Write-Warning ("Unable to terminate timed-out designer smoke process {0}: {1}" -f $process.Id, $_.Exception.Message)
        }
        [void]$process.WaitForExit(5000)
    }
    if (-not $timedOut) {
        $process.WaitForExit()
    }
    $stdout = if (Test-Path -LiteralPath $stdoutPath) { Get-Content -Raw -LiteralPath $stdoutPath } else { "" }
    $stderr = if (Test-Path -LiteralPath $stderrPath) { Get-Content -Raw -LiteralPath $stderrPath } else { "" }
    if (-not [string]::IsNullOrEmpty($stdout)) {
        Write-Output $stdout.TrimEnd()
    }
    if (-not [string]::IsNullOrEmpty($stderr)) {
        Write-Output $stderr.TrimEnd()
    }
    if ($timedOut) {
        Write-Output "DESIGNER_SMOKE_RESULT: kind=timeout"
        throw "Designer smoke executable timed out after $TimeoutSeconds second(s)."
    }
    if ($process.ExitCode -ne 0) {
        $status = if (Test-Path -LiteralPath $statusPath) {
            (Get-Content -Raw -LiteralPath $statusPath).Trim()
        }
        else {
            ""
        }
        $kind = switch ($status) {
            "completed" { "product-failure"; break }
            "invalid" { "harness-failure"; break }
            default { "interrupted" }
        }
        Write-Output ("DESIGNER_SMOKE_RESULT: kind={0}; exit_code={1}; status={2}" -f $kind, $process.ExitCode, ($status -replace "\s+", "<none>"))
        throw "Designer smoke executable failed with classified result '$kind' and exit code $($process.ExitCode)."
    }
    $skipLines = @(($stdout + "`n" + $stderr) -split "`r?`n" |
        Where-Object { $_ -match "^\s*SKIP:" })
    if ($skipLines.Count -gt 0) {
        Write-Warning (
            "Designer smoke reported {0} fixture-dependent skip(s); required executable and assertion checks passed." -f
            $skipLines.Count)
    }
    $status = if (Test-Path -LiteralPath $statusPath) {
        (Get-Content -Raw -LiteralPath $statusPath).Trim()
    }
    else {
        ""
    }
    if ($status -ne "completed") {
        Write-Output ("DESIGNER_SMOKE_RESULT: kind=harness-failure; exit_code=0; status={0}" -f ($status -replace "\s+", "<none>"))
        throw "Designer smoke executable exited successfully without a completed status marker."
    }
    Write-Output "DESIGNER_SMOKE_RESULT: kind=passed"
}
finally {
    if ($null -ne $process) {
        try {
            if (-not $process.HasExited) {
                $process.Kill()
                [void]$process.WaitForExit(5000)
            }
        }
        catch {
            Write-Warning ("Unable to terminate designer smoke process {0} during cleanup: {1}" -f $process.Id, $_.Exception.Message)
        }
        finally {
            $process.Dispose()
        }
    }
    Remove-Item -Recurse -Force -LiteralPath $tempRoot -ErrorAction SilentlyContinue
}
