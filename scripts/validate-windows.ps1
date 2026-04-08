$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $PSScriptRoot
$buildDir = Join-Path $repoRoot "build"
$vsixDir = Join-Path $repoRoot "vsix"
$nativeProject = $repoRoot
$vsixProject = Join-Path $vsixDir "Copperfin.VisualStudio\Copperfin.VisualStudio.csproj"
$studioProject = Join-Path $vsixDir "Copperfin.Studio\Copperfin.Studio.csproj"
$smokeProject = Join-Path $vsixDir "Copperfin.DesignerSmokeTests\Copperfin.DesignerSmokeTests.csproj"
$smokeExe = Join-Path $vsixDir "Copperfin.DesignerSmokeTests\bin\Release\net472\Copperfin.DesignerSmokeTests.exe"
$sampleProject = "C:\Program Files (x86)\Microsoft Visual FoxPro 9\Samples\Solution\solution.pjx"
$runtimeSmokeRoot = Join-Path $repoRoot "artifacts\runtime-smoke-validation"
$buildHostExe = Join-Path $buildDir "Release\copperfin_build_host.exe"
$runtimeHostExe = Join-Path $buildDir "Release\copperfin_runtime_host.exe"

function Invoke-Step {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Name,
        [Parameter(Mandatory = $true)]
        [scriptblock]$Action
    )

    Write-Host ""
    Write-Host "==> $Name" -ForegroundColor Cyan
    & $Action
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

$msbuild = (Get-Command MSBuild.exe -ErrorAction Stop).Source

Invoke-Step -Name "Configure native build" -Action {
    Invoke-Checked -FilePath "cmake" -ArgumentList @("-S", $nativeProject, "-B", $buildDir)
}

Invoke-Step -Name "Build native binaries" -Action {
    Invoke-Checked -FilePath "cmake" -ArgumentList @("--build", $buildDir, "--config", "Release")
}

Invoke-Step -Name "Run native CTest suite" -Action {
    Invoke-Checked -FilePath "ctest" -ArgumentList @("--test-dir", $buildDir, "-C", "Release", "--output-on-failure")
}

Invoke-Step -Name "Build Visual Studio extension" -Action {
    Invoke-Checked -FilePath $msbuild -ArgumentList @($vsixProject, "/restore", "/t:Rebuild", "/p:Configuration=Release", "/p:DeployExtension=false")
}

Invoke-Step -Name "Build standalone Studio shell" -Action {
    Invoke-Checked -FilePath $msbuild -ArgumentList @($studioProject, "/restore", "/t:Rebuild", "/p:Configuration=Release")
}

Invoke-Step -Name "Build designer smoke tests" -Action {
    Invoke-Checked -FilePath $msbuild -ArgumentList @($smokeProject, "/restore", "/t:Rebuild", "/p:Configuration=Release")
}

Invoke-Step -Name "Run designer smoke tests" -Action {
    Invoke-Checked -FilePath $smokeExe
}

Invoke-Step -Name "Run runtime package smoke test" -Action {
    Remove-Item -Recurse -Force $runtimeSmokeRoot -ErrorAction SilentlyContinue
    New-Item -ItemType Directory -Force $runtimeSmokeRoot | Out-Null

    Invoke-Checked -FilePath $buildHostExe -ArgumentList @(
        "build",
        "--project", $sampleProject,
        "--output-dir", $runtimeSmokeRoot,
        "--configuration", "debug",
        "--enable-security",
        "--emit-dotnet-launcher",
        "--runtime-host", $runtimeHostExe
    )

    $packagedRoot = Join-Path $runtimeSmokeRoot "SOLUTION"
    $launcherExe = Join-Path $packagedRoot "SOLUTION.exe"
    $manifestPath = Join-Path $packagedRoot "app.cfmanifest"

    if (-not (Test-Path $launcherExe)) {
        throw "Expected generated launcher was not found: $launcherExe"
    }
    if (-not (Test-Path $manifestPath)) {
        throw "Expected runtime manifest was not found: $manifestPath"
    }

    Invoke-Checked -FilePath $launcherExe -ArgumentList @("--debug")
}

Invoke-Step -Name "Run PRG debugger smoke test" -Action {
    $prgSmokeRoot = Join-Path $repoRoot "artifacts\prg-debug-smoke"
    Remove-Item -Recurse -Force $prgSmokeRoot -ErrorAction SilentlyContinue
    New-Item -ItemType Directory -Force $prgSmokeRoot | Out-Null

    $prgPath = Join-Path $prgSmokeRoot "main.prg"
    @(
        "x = 1",
        "DO localproc",
        "x = x + 1",
        "RETURN",
        "PROCEDURE localproc",
        "x = x + 2",
        "RETURN"
    ) | Set-Content -Path $prgPath

    $manifestPath = Join-Path $prgSmokeRoot "app.cfmanifest"
    @(
        "manifest_version=1",
        "project_title=PRGDEBUG",
        "project_path=E:\Project-Copperfin\smoke.pjx",
        "package_root=$prgSmokeRoot",
        "content_root=$prgSmokeRoot",
        "working_directory=$prgSmokeRoot",
        "startup_item=main.prg",
        "startup_source=$prgPath",
        "configuration=debug",
        "security_enabled=false",
        "security_mode=off",
        "dotnet_enabled=false",
        "dotnet_story="
    ) | Set-Content -Path $manifestPath

    Invoke-Checked -FilePath $runtimeHostExe -ArgumentList @(
        "--manifest", $manifestPath,
        "--debug",
        "--breakpoint", "2",
        "--debug-command", "continue",
        "--debug-command", "step",
        "--debug-command", "out",
        "--debug-command", "continue"
    )
}

Write-Host ""
Write-Host "Validation complete." -ForegroundColor Green
