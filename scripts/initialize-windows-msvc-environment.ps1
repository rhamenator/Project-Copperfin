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

if (-not $IsWindows) {
    throw "The MSVC environment initializer is Windows-only."
}
if ([string]::IsNullOrWhiteSpace($env:GITHUB_ENV)) {
    throw "GITHUB_ENV is required so later workflow steps inherit the MSVC environment."
}

$programFilesX86 = [Environment]::GetEnvironmentVariable("ProgramFiles(x86)")
if ([string]::IsNullOrWhiteSpace($programFilesX86)) {
    throw "ProgramFiles(x86) is unavailable."
}

$vswherePath = Join-Path $programFilesX86 "Microsoft Visual Studio\Installer\vswhere.exe"
if (-not (Test-Path -LiteralPath $vswherePath -PathType Leaf)) {
    throw "vswhere.exe was not found in the Visual Studio Installer directory."
}

$installationPath = [string](& $vswherePath -latest -products * `
    -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
    -property installationPath | Select-Object -First 1)
$installationPath = $installationPath.Trim()
if ([string]::IsNullOrWhiteSpace($installationPath)) {
    throw "No Visual Studio installation with the x64 MSVC tools was found."
}

$devCommand = Join-Path $installationPath "Common7\Tools\VsDevCmd.bat"
if (-not (Test-Path -LiteralPath $devCommand -PathType Leaf)) {
    throw "VsDevCmd.bat is missing from the selected Visual Studio installation."
}

$commandLine = "`"$devCommand`" -no_logo -arch=x64 -host_arch=x64 && set"
$environmentLines = @(& $env:ComSpec /d /s /c $commandLine)
if ($LASTEXITCODE -ne 0) {
    throw "VsDevCmd.bat failed with exit code $LASTEXITCODE."
}

foreach ($line in $environmentLines) {
    $separator = $line.IndexOf('=')
    if ($separator -le 0) {
        continue
    }

    $name = $line.Substring(0, $separator)
    $value = $line.Substring($separator + 1)
    if ($name.StartsWith('=') -or $name -match '^(ACTIONS_|GITHUB_)') {
        continue
    }

    [Environment]::SetEnvironmentVariable($name, $value, "Process")
    Add-Content -LiteralPath $env:GITHUB_ENV -Value "$name=$value" -Encoding utf8
}

$compiler = Get-Command cl.exe -ErrorAction Stop
$compilerVersion = (& $compiler.Source 2>&1 | Out-String).Trim()
$compilerSha256 = (Get-FileHash -LiteralPath $compiler.Source -Algorithm SHA256).Hash.ToLowerInvariant()

$evidenceFile = [System.IO.Path]::GetFullPath($EvidencePath)
$evidenceDirectory = Split-Path -Parent $evidenceFile
New-Item -ItemType Directory -Force -Path $evidenceDirectory | Out-Null
$evidence = [ordered]@{
    schema_version = 1
    kind = "windows-msvc-identity"
    installation_path = $installationPath
    compiler_path = $compiler.Source
    compiler_sha256 = $compilerSha256
    compiler_banner = $compilerVersion
    architecture = "x64"
    host_architecture = "x64"
}
$evidence | ConvertTo-Json -Depth 4 | Set-Content -LiteralPath $evidenceFile -Encoding utf8

Write-Host "Initialized x64 MSVC environment from $installationPath"
