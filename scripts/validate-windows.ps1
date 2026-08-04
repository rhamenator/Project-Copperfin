# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only

param(
    [ValidateRange(1, 256)]
    [int]$BuildJobs = 2,
    [ValidateRange(1, 2)]
    [int]$TestJobs = 2
)

$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $PSScriptRoot
$buildDir = Join-Path $repoRoot "build"
$vsixDir = Join-Path $repoRoot "vsix"
$nativeProject = $repoRoot
$vsixProject = Join-Path $vsixDir "Copperfin.VisualStudio\Copperfin.VisualStudio.csproj"
$vsixArtifact = Join-Path $vsixDir "Copperfin.VisualStudio\bin\Release\net472\Copperfin.VisualStudio.vsix"
$vsixLocalizationTest = Join-Path $repoRoot "scripts\test-vsix-command-localization.ps1"
$studioProject = Join-Path $vsixDir "Copperfin.Studio\Copperfin.Studio.csproj"
$smokeProject = Join-Path $vsixDir "Copperfin.DesignerSmokeTests\Copperfin.DesignerSmokeTests.csproj"
$smokeExe = Join-Path $vsixDir "Copperfin.DesignerSmokeTests\bin\Release\net472\Copperfin.DesignerSmokeTests.exe"
$requiredDesignerSmokeScript = Join-Path $repoRoot "scripts\run-required-designer-smoke.ps1"
$deepSmokeScript = Join-Path $repoRoot "scripts\run-windows-deep-smoke.ps1"
$validationFailures = [System.Collections.Generic.List[string]]::new()

function Invoke-Step {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Name,
        [Parameter(Mandatory = $true)]
        [scriptblock]$Action,
        [switch]$ContinueOnFailure
    )

    Write-Host ""
    Write-Host "==> $Name" -ForegroundColor Cyan
    try {
        & $Action
        return $true
    }
    catch {
        $detail = $_.Exception.Message
        $validationFailures.Add("${Name}: $detail")
        Write-Host ("VALIDATION FAILURE [$Name]: $detail") -ForegroundColor Red
        if (-not $ContinueOnFailure) {
            throw
        }
        Write-Warning ("Continuing after [$Name]; the final validation result will remain failed.")
        return $false
    }
}

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

function Resolve-MSBuild {
    $command = Get-Command MSBuild.exe -ErrorAction SilentlyContinue
    if ($null -ne $command) {
        return $command.Source
    }

    $vswhere = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path $vswhere) {
        $resolved = & $vswhere -latest -requires Microsoft.Component.MSBuild -find "MSBuild\**\Bin\MSBuild.exe" | Select-Object -First 1
        if (-not [string]::IsNullOrWhiteSpace($resolved)) {
            return $resolved
        }
    }

    throw "MSBuild.exe could not be located. Install Visual Studio Build Tools or run from a Developer PowerShell."
}

$msbuild = Resolve-MSBuild

Invoke-Step -Name "Configure native build" -Action {
    Invoke-Checked -FilePath "cmake" -ArgumentList @(
        "-S", $nativeProject, "-B", $buildDir, "-A", "x64"
    )
}

Invoke-Step -Name "Build native binaries" -Action {
    Invoke-Checked -FilePath "cmake" -ArgumentList @("--build", $buildDir, "--config", "Release", "--parallel", "$BuildJobs")
}

Invoke-Step -Name "Run native CTest suite" -Action {
    Invoke-Checked -FilePath "ctest" -ArgumentList @(
        "--test-dir", $buildDir,
        "-C", "Release",
        "--output-on-failure",
        "--timeout", "180",
        "--parallel", "$TestJobs"
    )
}

Invoke-Step -Name "Build Visual Studio extension" -Action {
    Invoke-Checked -FilePath $msbuild -ArgumentList @($vsixProject, "/restore", "/t:Rebuild", "/p:Configuration=Release", "/p:DeployExtension=false")
}

Invoke-Step -Name "Verify localized command resources" -Action {
    Invoke-Checked -FilePath $vsixLocalizationTest -ArgumentList @(
        "-VsixPath", $vsixArtifact
    )
}

Invoke-Step -Name "Run managed VSIX behavior tests" -Action {
    Invoke-Checked -FilePath "dotnet" -ArgumentList @(
        "run",
        "--project", (Join-Path $vsixDir "Copperfin.LanguageServiceTests\Copperfin.StudioTargetSelectionTests.csproj"),
        "--configuration", "Release"
    )
}

Invoke-Step -Name "Run managed language-service tests" -Action {
    Invoke-Checked -FilePath "dotnet" -ArgumentList @(
        "run",
        "--project", (Join-Path $vsixDir "Copperfin.LanguageServiceTests\Copperfin.LanguageServiceTests.csproj"),
        "--configuration", "Release"
    )
}

Invoke-Step -Name "Run .NET Framework process-runner tests" -Action {
    Invoke-Checked -FilePath "dotnet" -ArgumentList @(
        "run",
        "--project", (Join-Path $vsixDir "Copperfin.ProcessRunnerNetFrameworkTests\Copperfin.ProcessRunnerNetFrameworkTests.csproj"),
        "--configuration", "Release"
    )
}

Invoke-Step -Name "Build standalone Studio shell" -Action {
    Invoke-Checked -FilePath $msbuild -ArgumentList @($studioProject, "/restore", "/t:Rebuild", "/p:Configuration=Release")
}

Invoke-Step -Name "Build designer smoke tests" -Action {
    Invoke-Checked -FilePath $msbuild -ArgumentList @($smokeProject, "/restore", "/t:Rebuild", "/p:Configuration=Release")
}

Invoke-Step -Name "Run designer smoke tests" -Action {
    Invoke-Checked -FilePath "pwsh" -ArgumentList @(
        "-NoProfile", "-File", $requiredDesignerSmokeScript, "-ExecutablePath", $smokeExe,
        "-TimeoutSeconds", "1800"
    )
} -ContinueOnFailure

Invoke-Step -Name "Run runtime package smoke test" -Action {
    Invoke-Checked -FilePath "pwsh" -ArgumentList @(
        "-NoProfile", "-File", $deepSmokeScript, "-Stage", "RuntimePackage"
    )
} -ContinueOnFailure

Invoke-Step -Name "Run PRG debugger smoke test" -Action {
    Invoke-Checked -FilePath "pwsh" -ArgumentList @(
        "-NoProfile", "-File", $deepSmokeScript, "-Stage", "PrgDebugger"
    )
} -ContinueOnFailure

Invoke-Step -Name "Run xAsset bootstrap smoke test" -Action {
    Invoke-Checked -FilePath "pwsh" -ArgumentList @(
        "-NoProfile", "-File", $deepSmokeScript, "-Stage", "XAsset"
    )
} -ContinueOnFailure

Invoke-Step -Name "Run report xAsset smoke test" -Action {
    Invoke-Checked -FilePath "pwsh" -ArgumentList @(
        "-NoProfile", "-File", $deepSmokeScript, "-Stage", "Report"
    )
} -ContinueOnFailure

Invoke-Step -Name "Run menu xAsset smoke test" -Action {
    Invoke-Checked -FilePath "pwsh" -ArgumentList @(
        "-NoProfile", "-File", $deepSmokeScript, "-Stage", "Menu"
    )
} -ContinueOnFailure

if ($validationFailures.Count -gt 0) {
    Write-Host ""
    Write-Host "Validation completed with $($validationFailures.Count) failure(s):" -ForegroundColor Red
    foreach ($failure in $validationFailures) {
        Write-Host " - $failure" -ForegroundColor Red
    }
    exit 1
}

Write-Host ""
Write-Host "Validation complete." -ForegroundColor Green
