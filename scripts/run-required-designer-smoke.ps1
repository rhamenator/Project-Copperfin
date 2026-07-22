# Copyright (c) 2026 Richard M. Hamilton. All rights reserved.
# Licensed under the Project Copperfin Source-Available License or
# Commercial License. See LICENSE.md in the repository root.

[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [ValidateNotNullOrEmpty()]
    [string]$ExecutablePath
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

try {
    $process = Start-Process -FilePath $ExecutablePath -ArgumentList @() -Wait -PassThru -NoNewWindow -RedirectStandardOutput $stdoutPath -RedirectStandardError $stderrPath
    $stdout = if (Test-Path -LiteralPath $stdoutPath) { Get-Content -Raw -LiteralPath $stdoutPath } else { "" }
    $stderr = if (Test-Path -LiteralPath $stderrPath) { Get-Content -Raw -LiteralPath $stderrPath } else { "" }
    if (-not [string]::IsNullOrEmpty($stdout)) {
        Write-Output $stdout.TrimEnd()
    }
    if (-not [string]::IsNullOrEmpty($stderr)) {
        Write-Output $stderr.TrimEnd()
    }
    if ($process.ExitCode -ne 0) {
        throw "Designer smoke executable failed with exit code $($process.ExitCode)."
    }
    $skipLines = @(($stdout + "`n" + $stderr) -split "`r?`n" |
        Where-Object { $_ -match "^\s*SKIP:" })
    if ($skipLines.Count -gt 0) {
        Write-Warning (
            "Designer smoke reported {0} fixture-dependent skip(s); required executable and assertion checks passed." -f
            $skipLines.Count)
    }
}
finally {
    Remove-Item -Recurse -Force -LiteralPath $tempRoot -ErrorAction SilentlyContinue
}
