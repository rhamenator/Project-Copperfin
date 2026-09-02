# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

# Installs and verifies the NSIS compiler used to build the Windows
# installer. The checksum-pinned portable archive is the primary,
# supply-chain-verified path; an ambient runner-image or Chocolatey
# install is used only if that pinned archive is genuinely unavailable
# (network/HTTP failure), and even then its makensis.exe is rejected
# unless it reports exactly the pinned version -- the only verification
# available for those paths, since no binary hash is known ahead of time
# for them. See issue #5433.

[CmdletBinding()]
param()

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$pinnedVersion = "3.12"
$pinnedArchiveName = "nsis-$pinnedVersion.zip"
$pinnedArchiveUri = "https://downloads.sourceforge.net/nsis/$pinnedArchiveName"
$pinnedArchiveSha256 = "56581f90db321581c5381193d796fffcf2d24b2f8fed2160a6c6a3baa67f2c4f"
$pinnedArchiveRootName = "nsis-$pinnedVersion"

$ambientRoot = "C:\Program Files (x86)\NSIS"
$fallbackRoot = Join-Path $env:RUNNER_TEMP $pinnedArchiveRootName
$fallbackArchive = Join-Path $env:RUNNER_TEMP $pinnedArchiveName

function Invoke-WithRetry {
    param(
        [Parameter(Mandatory = $true)][scriptblock]$Action,
        [int]$MaxAttempts = 3,
        [string]$Description = "operation"
    )

    for ($attempt = 1; $attempt -le $MaxAttempts; $attempt++) {
        try {
            & $Action
            return
        }
        catch {
            if ($attempt -ge $MaxAttempts) {
                throw
            }
            Write-Warning "$Description attempt $attempt failed: $($_.Exception.Message)"
            Start-Sleep -Seconds (5 * $attempt)
        }
    }
}

function Test-MakensisReportsPinnedVersion {
    param([Parameter(Mandatory = $true)][string]$MakensisPath)

    # Stderr is discarded (2>$null), not merged via 2>&1: with
    # $ErrorActionPreference = 'Stop' in scope, merging a native
    # command's stderr into the pipeline can turn even benign stderr
    # output into a script-terminating error before this check runs.
    $reportedVersion = ((& $MakensisPath /VERSION 2>$null) | Out-String).Trim()
    if ($LASTEXITCODE -ne 0) {
        return $null
    }
    if ($reportedVersion -notmatch "^v?$([Regex]::Escape($pinnedVersion))$") {
        return $null
    }
    return $reportedVersion
}

$nsisRoot = $null
$makensis = $null

# Primary path: the checksum-pinned portable archive. A hash mismatch
# after a successful download is treated as fatal, not retried -- it is
# a tampering/corruption signal, not a transient availability problem,
# and silently retrying (then falling back to a weaker verification
# path) could paper over exactly the supply-chain compromise this
# script exists to catch.
$pinnedArchiveAvailable = $false
$pinnedArchiveDownloaded = $false
try {
    Invoke-WithRetry -Description "Checksum-pinned portable NSIS download" -Action {
        Invoke-WebRequest -Uri $pinnedArchiveUri -OutFile $fallbackArchive -MaximumRedirection 5
    }
    $pinnedArchiveDownloaded = $true
}
catch {
    Write-Warning "Checksum-pinned portable NSIS archive unavailable: $($_.Exception.Message)"
}

if ($pinnedArchiveDownloaded) {
    # A hash mismatch here is NOT caught for fallback purposes: it is a
    # tampering/corruption signal, not a transient availability problem,
    # and silently falling back to a weaker verification path could
    # paper over exactly the supply-chain compromise this script exists
    # to catch. Left uncaught, this terminates the whole script
    # ($ErrorActionPreference = 'Stop').
    $archiveHash = (Get-FileHash -LiteralPath $fallbackArchive -Algorithm SHA256).Hash.ToLowerInvariant()
    if ($archiveHash -ne $pinnedArchiveSha256) {
        throw "NSIS archive SHA-256 mismatch: expected $pinnedArchiveSha256, got $archiveHash. " +
            "Refusing to fall back to an unverified source for what looks like a tampered or " +
            "corrupted download."
    }

    Expand-Archive -LiteralPath $fallbackArchive -DestinationPath $env:RUNNER_TEMP -Force
    $portableMakensis = Join-Path $fallbackRoot "makensis.exe"
    if (-not (Test-Path -LiteralPath $portableMakensis -PathType Leaf)) {
        throw "Portable NSIS archive did not provide $portableMakensis."
    }

    $makensis = $portableMakensis
    $nsisRoot = $fallbackRoot
    $pinnedArchiveAvailable = $true
}

if (-not $pinnedArchiveAvailable) {
    Write-Warning ("Falling back to an ambient runner-image or Chocolatey install. That binary " +
        "is still rejected below unless it reports the pinned NSIS version.")
    $nsisRoot = $ambientRoot
    $makensis = Join-Path $ambientRoot "makensis.exe"

    if (-not (Test-Path -LiteralPath $makensis -PathType Leaf)) {
        # Not version-pinned via `choco install --version`: the exact-match
        # check below already fully gates which version is trusted, so
        # pinning the choco install itself would add no security value
        # while making this fallback brittle against Chocolatey's own
        # version string formatting (e.g. a suffixed "3.12.20240115"-style
        # package version would never equal the plain pinned string).
        Invoke-WithRetry -Description "Chocolatey NSIS install" -Action {
            & choco install nsis -y --no-progress --retry-count=3 --retry-delay=5
            if ($LASTEXITCODE -ne 0 -or -not (Test-Path -LiteralPath $makensis -PathType Leaf)) {
                throw "choco install nsis exited with code $LASTEXITCODE."
            }
        }
    }

    if (-not (Test-Path -LiteralPath $makensis -PathType Leaf)) {
        throw "NSIS installation did not provide makensis.exe at $makensis, and the " +
            "checksum-pinned archive was unavailable."
    }
}

# Regardless of which path produced makensis.exe, verify it reports
# EXACTLY the pinned NSIS version (anchored match, not a substring or
# prefix) before it is trusted to build the installer.
$reportedVersion = Test-MakensisReportsPinnedVersion -MakensisPath $makensis
if ($null -eq $reportedVersion) {
    throw "makensis.exe at $makensis did not report the pinned version $pinnedVersion; " +
        "refusing to build the installer with an unverified compiler."
}

if ([string]::IsNullOrWhiteSpace($env:GITHUB_PATH)) {
    throw "GITHUB_PATH was not set; this script must run as a GitHub Actions step."
}
$nsisRoot | Out-File -FilePath $env:GITHUB_PATH -Encoding utf8 -Append
Write-Host "Using makensis.exe at $makensis (version: $reportedVersion)"
