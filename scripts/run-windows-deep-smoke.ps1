# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only

[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [ValidateSet("RuntimePackage", "PrgDebugger", "XAsset", "Report", "Menu")]
    [string]$Stage,

    [ValidateSet("Debug", "Release")]
    [string]$BuildConfiguration = "Release"
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $PSScriptRoot
$buildRoot = Join-Path $repoRoot "build\$BuildConfiguration"
$buildHostExe = Join-Path $buildRoot "copperfin_build_host.exe"
$runtimeHostExe = Join-Path $buildRoot "copperfin_runtime_host.exe"
$configuredVfp9Root = $env:COPPERFIN_VFP9_ROOT
if ([string]::IsNullOrWhiteSpace($configuredVfp9Root)) {
    $configuredVfp9Root = "C:\Program Files (x86)\Microsoft Visual FoxPro 9"
}

$sampleProject = Join-Path $configuredVfp9Root "Samples\Solution\solution.pjx"
$booksForm = Join-Path $configuredVfp9Root "Wizards\Template\Books\Forms\books.scx"
$invoiceReport = Join-Path $configuredVfp9Root "Samples\Solution\Reports\invoice.frx"
$menuFile = Join-Path $configuredVfp9Root "Samples\Solution\Toledo\systray_shortcut.mnx"

function Invoke-Checked {
    param(
        [Parameter(Mandatory = $true)]
        [string]$FilePath,
        [string[]]$ArgumentList = @()
    )

    & $FilePath @ArgumentList
    if ($LASTEXITCODE -ne 0) {
        throw "Command failed: $FilePath $($ArgumentList -join ' ')"
    }
}

function Require-File {
    param([Parameter(Mandatory = $true)][string]$Path)

    if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) {
        throw "Required VFP9 smoke asset was not found: $Path. Set COPPERFIN_VFP9_ROOT to the installed VFP9 root."
    }
}

function New-SmokeRoot {
    param([Parameter(Mandatory = $true)][string]$Name)

    $root = Join-Path $repoRoot "artifacts\$Name"
    Remove-Item -Recurse -Force $root -ErrorAction SilentlyContinue
    New-Item -ItemType Directory -Force $root | Out-Null
    return $root
}

function Stage-SmokeAsset {
    param(
        [Parameter(Mandatory = $true)][string]$SourcePath,
        [Parameter(Mandatory = $true)][string]$DestinationRoot
    )

    Require-File $SourcePath
    $assetName = Split-Path -Leaf $SourcePath
    $destinationPath = Join-Path $DestinationRoot $assetName
    Copy-Item -LiteralPath $SourcePath -Destination $destinationPath -Force

    $sidecarExtension = switch ([System.IO.Path]::GetExtension($SourcePath).ToLowerInvariant()) {
        ".scx" { ".sct"; break }
        ".vcx" { ".vct"; break }
        ".frx" { ".frt"; break }
        ".lbx" { ".lbt"; break }
        ".mnx" { ".mnt"; break }
        ".pjx" { ".pjt"; break }
        default { $null }
    }
    if ($null -ne $sidecarExtension) {
        $sourceSidecar = [System.IO.Path]::ChangeExtension($SourcePath, $sidecarExtension)
        if (Test-Path -LiteralPath $sourceSidecar -PathType Leaf) {
            Copy-Item -LiteralPath $sourceSidecar -Destination (Join-Path $DestinationRoot (Split-Path -Leaf $sourceSidecar)) -Force
        }
    }

    return $destinationPath
}

function Write-DebugManifest {
    param(
        [Parameter(Mandatory = $true)][string]$Path,
        [Parameter(Mandatory = $true)][string]$ProjectTitle,
        [Parameter(Mandatory = $true)][string]$ProjectPath,
        [Parameter(Mandatory = $true)][string]$PackageRoot,
        [Parameter(Mandatory = $true)][string]$WorkingDirectory,
        [Parameter(Mandatory = $true)][string]$StartupItem,
        [Parameter(Mandatory = $true)][string]$StartupSource
    )

    @(
        "manifest_version=1",
        "project_title=$ProjectTitle",
        "project_path=$ProjectPath",
        "package_root=$PackageRoot",
        "content_root=$PackageRoot",
        "working_directory=$WorkingDirectory",
        "startup_item=$StartupItem",
        "startup_source=$StartupSource",
        "configuration=debug",
        "security_enabled=false",
        "security_mode=off",
        "dotnet_enabled=false",
        "dotnet_story="
    ) | Set-Content -LiteralPath $Path -Encoding utf8
}

if (-not (Test-Path -LiteralPath $buildHostExe -PathType Leaf)) {
    throw "Build host was not found: $buildHostExe"
}
if (-not (Test-Path -LiteralPath $runtimeHostExe -PathType Leaf)) {
    throw "Runtime host was not found: $runtimeHostExe"
}

switch ($Stage) {
    "RuntimePackage" {
        Require-File $sampleProject
        $smokeRoot = New-SmokeRoot "runtime-smoke-validation"
        # The secure build contract requires a role that includes build.execute.
        $env:COPPERFIN_SECURITY_ROLE = "build-engineer"
        Invoke-Checked -FilePath $buildHostExe -ArgumentList @(
            "build", "--project", $sampleProject, "--output-dir", $smokeRoot,
            "--configuration", "debug", "--enable-security", "--emit-dotnet-launcher",
            "--runtime-host", $runtimeHostExe
        )

        $packagedRoot = Join-Path $smokeRoot "SOLUTION"
        $launcherExe = Join-Path $packagedRoot "SOLUTION.exe"
        $manifestPath = Join-Path $packagedRoot "app.cfmanifest"
        Require-File $launcherExe
        Require-File $manifestPath
        Invoke-Checked -FilePath $launcherExe -ArgumentList @("--debug")
    }
    "PrgDebugger" {
        $smokeRoot = New-SmokeRoot "prg-debug-smoke"
        $prgPath = Join-Path $smokeRoot "main.prg"
        @(
            "x = 1",
            "DO localproc",
            "x = x + 1",
            "RETURN",
            "PROCEDURE localproc",
            "x = x + 2",
            "RETURN"
        ) | Set-Content -LiteralPath $prgPath -Encoding utf8
        $manifestPath = Join-Path $smokeRoot "app.cfmanifest"
        Write-DebugManifest $manifestPath "PRGDEBUG" "E:\Project-Copperfin\smoke.pjx" $smokeRoot $smokeRoot "main.prg" $prgPath
        Invoke-Checked -FilePath $runtimeHostExe -ArgumentList @(
            "--manifest", $manifestPath, "--debug", "--breakpoint", "2",
            "--debug-command", "continue", "--debug-command", "step",
            "--debug-command", "out", "--debug-command", "continue"
        )
    }
    "XAsset" {
        Require-File $booksForm
        $smokeRoot = New-SmokeRoot "xasset-debug-smoke"
        $stagedBooksForm = Stage-SmokeAsset $booksForm $smokeRoot
        $manifestPath = Join-Path $smokeRoot "app.cfmanifest"
        Write-DebugManifest $manifestPath "XASSETDEBUG" "E:\Project-Copperfin\xasset-smoke.pjx" $smokeRoot $smokeRoot (Split-Path -Leaf $stagedBooksForm) $stagedBooksForm
        Invoke-Checked -FilePath $runtimeHostExe -ArgumentList @(
            "--manifest", $manifestPath, "--debug", "--debug-command", "continue",
            "--debug-command", "invoke:frmbooks.release"
        )
    }
    "Report" {
        Require-File $invoiceReport
        $smokeRoot = New-SmokeRoot "report-debug-smoke"
        $stagedInvoiceReport = Stage-SmokeAsset $invoiceReport $smokeRoot
        $manifestPath = Join-Path $smokeRoot "app.cfmanifest"
        Write-DebugManifest $manifestPath "REPORTDEBUG" "E:\Project-Copperfin\report-smoke.pjx" $smokeRoot $smokeRoot (Split-Path -Leaf $stagedInvoiceReport) $stagedInvoiceReport
        Invoke-Checked -FilePath $runtimeHostExe -ArgumentList @(
            "--manifest", $manifestPath, "--debug", "--debug-command", "continue"
        )
    }
    "Menu" {
        Require-File $menuFile
        $smokeRoot = New-SmokeRoot "menu-debug-smoke"
        $stagedMenuFile = Stage-SmokeAsset $menuFile $smokeRoot
        $manifestPath = Join-Path $smokeRoot "app.cfmanifest"
        Write-DebugManifest $manifestPath "MENUDEBUG" "E:\Project-Copperfin\menu-smoke.pjx" $smokeRoot $smokeRoot (Split-Path -Leaf $stagedMenuFile) $stagedMenuFile
        Invoke-Checked -FilePath $runtimeHostExe -ArgumentList @(
            "--manifest", $manifestPath, "--debug", "--debug-command", "continue",
            "--debug-command", "select:shortcut.item1", "--debug-command", "select:shortcut.item3",
            "--debug-command", "select:thisitemha.item3"
        )
    }
}

Write-Host "Windows deep smoke stage passed: $Stage" -ForegroundColor Green
