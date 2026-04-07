$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $PSScriptRoot
$buildDir = Join-Path $repoRoot "build"
$vsixDir = Join-Path $repoRoot "vsix"
$nativeProject = $repoRoot
$vsixProject = Join-Path $vsixDir "Copperfin.VisualStudio\Copperfin.VisualStudio.csproj"
$studioProject = Join-Path $vsixDir "Copperfin.Studio\Copperfin.Studio.csproj"
$smokeProject = Join-Path $vsixDir "Copperfin.DesignerSmokeTests\Copperfin.DesignerSmokeTests.csproj"
$smokeExe = Join-Path $vsixDir "Copperfin.DesignerSmokeTests\bin\Release\net472\Copperfin.DesignerSmokeTests.exe"

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

Write-Host ""
Write-Host "Validation complete." -ForegroundColor Green
