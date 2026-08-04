# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)] [string]$FixtureExecutable,
    [Parameter(Mandatory = $true)] [string]$LauncherGuard,
    [Parameter(Mandatory = $true)] [string]$SignerScript,
    [Parameter(Mandatory = $true)] [string]$SigningKeyPath,
    [Parameter(Mandatory = $true)] [string]$SignerKeyId,
    [Parameter(Mandatory = $true)] [string]$RepositoryRoot,
    [Parameter(Mandatory = $true)] [string]$OutputPath
)

$ErrorActionPreference = "Stop"
$utf8 = [System.Text.UTF8Encoding]::new($false)
$packageRoot = Join-Path $env:RUNNER_TEMP ("copperfin-launcher-trust-" + [Guid]::NewGuid().ToString("N"))
$markerPath = Join-Path $packageRoot "managed-apphost-started.marker"
$guardPath = Join-Path $packageRoot "Copperfin.TrustValidation.exe"
$manifestPath = Join-Path $packageRoot "app.cfmanifest"
$envelopePath = Join-Path $packageRoot "app.cftrust"
$signaturePath = Join-Path $packageRoot "app.cftrust.sig"
$results = [System.Collections.Generic.List[object]]::new()

function Write-ExactText {
    param([string]$Path, [string]$Text)
    [System.IO.File]::WriteAllText($Path, $Text, $utf8)
}

function Invoke-GuardCase {
    param(
        [string]$Name,
        [int]$ExpectedExitCode,
        [bool]$ExpectInternalStart
    )
    Remove-Item -LiteralPath $markerPath -Force -ErrorAction SilentlyContinue
    $env:COPPERFIN_TEST_LAUNCHER_TRUST_MARKER = $markerPath
    try {
        & $guardPath
        $exitCode = $LASTEXITCODE
    } finally {
        Remove-Item Env:COPPERFIN_TEST_LAUNCHER_TRUST_MARKER -ErrorAction SilentlyContinue
    }
    $started = Test-Path -LiteralPath $markerPath -PathType Leaf
    if ($exitCode -ne $ExpectedExitCode -or $started -ne $ExpectInternalStart) {
        throw "Launcher trust case '$Name' failed: exit=$exitCode started=$started"
    }
    $results.Add([ordered]@{
        case = $Name
        exit_code = $exitCode
        internal_apphost_started = $started
        status = "passed"
    })
}

function Replace-Manifest {
    param([string[]]$Lines)
    Write-ExactText $manifestPath (($Lines -join "`n") + "`n")
}

try {
    & $FixtureExecutable --prepare $packageRoot $LauncherGuard $SignerKeyId
    if ($LASTEXITCODE -ne 0) {
        throw "Failed to prepare finalized launcher trust fixture."
    }
    & $SignerScript `
        -InputPath $envelopePath `
        -OutputPath $signaturePath `
        -KeyRef $SigningKeyPath `
        -RepositoryRoot $RepositoryRoot
    if ($LASTEXITCODE -ne 0 -or -not (Test-Path -LiteralPath $signaturePath -PathType Leaf)) {
        throw "External launcher inventory signer did not produce app.cftrust.sig."
    }

    $originalManifest = [System.IO.File]::ReadAllText($manifestPath)
    $originalSignature = [System.IO.File]::ReadAllText($signaturePath)
    $inventoryLines = @($originalManifest.Split([char]10) | Where-Object { $_.StartsWith("launcher_artifact=") })
    if ($inventoryLines.Count -lt 5) {
        throw "Finalized launcher fixture did not contain the required inventory."
    }
    $inventoryEvidence = @($inventoryLines | ForEach-Object {
        $fields = $_.Substring("launcher_artifact=".Length).Split('|')
        if ($fields.Count -ne 3 -or $fields[2] -notmatch '^[0-9a-f]{64}$') {
            throw "Finalized launcher fixture contained a malformed inventory record."
        }
        [ordered]@{
            package_relative_name = $fields[0]
            role = $fields[1]
            sha256 = $fields[2]
        }
    })

    Invoke-GuardCase "valid-signed-launch" 0 $true

    $dllPath = Join-Path $packageRoot "Copperfin.GeneratedLauncher.dll"
    $originalDll = [System.IO.File]::ReadAllBytes($dllPath)
    Write-ExactText $dllPath "modified launcher dll`n"
    Invoke-GuardCase "modified-artifact" 4 $false
    [System.IO.File]::WriteAllBytes($dllPath, $originalDll)

    $depsPath = Join-Path $packageRoot "Copperfin.GeneratedLauncher.deps.json"
    $removedDepsPath = "$depsPath.removed"
    Move-Item -LiteralPath $depsPath -Destination $removedDepsPath
    Invoke-GuardCase "removed-artifact" 4 $false
    Move-Item -LiteralPath $removedDepsPath -Destination $depsPath

    $manifestLines = @($originalManifest.TrimEnd([char[]]@("`r", "`n")).Split([char]10))
    Replace-Manifest ($manifestLines | Where-Object { $_ -ne $inventoryLines[1] })
    Invoke-GuardCase "removed-inventory-record" 4 $false
    Write-ExactText $manifestPath $originalManifest

    Replace-Manifest ($manifestLines + $inventoryLines[0])
    Invoke-GuardCase "duplicate-inventory-record" 4 $false
    Write-ExactText $manifestPath $originalManifest

    $ambiguousFields = $inventoryLines[0].Split('|')
    $inventoryPrefix = "launcher_artifact="
    $ambiguousPath = $ambiguousFields[0].Substring($inventoryPrefix.Length)
    $ambiguousFields[0] = $inventoryPrefix + $ambiguousPath.ToUpperInvariant()
    Replace-Manifest ($manifestLines + ($ambiguousFields -join '|'))
    Invoke-GuardCase "case-ambiguous-inventory-record" 4 $false
    Write-ExactText $manifestPath $originalManifest

    $signatureMatch = [regex]::Match($originalSignature, '(?m)^signature_base64=(.)')
    if (-not $signatureMatch.Success) {
        throw "Signed launcher fixture did not contain a signature payload."
    }
    $replacementCharacter = if ($signatureMatch.Groups[1].Value -eq 'A') { 'B' } else { 'A' }
    $modifiedSignature = $originalSignature.Remove(
        $signatureMatch.Groups[1].Index,
        1).Insert($signatureMatch.Groups[1].Index, $replacementCharacter)
    Write-ExactText $signaturePath $modifiedSignature
    Invoke-GuardCase "modified-signature-sidecar" 4 $false
    Write-ExactText $signaturePath $originalSignature

    Remove-Item -LiteralPath $signaturePath -Force
    Invoke-GuardCase "removed-signature-sidecar" 4 $false
    Write-ExactText $signaturePath $originalSignature

    $evidence = [ordered]@{
        schema_version = 1
        kind = "windows-launcher-trust-validation"
        trust_enforced = $true
        signer_key_id = $SignerKeyId
        repository_commit = $env:GITHUB_SHA
        workflow_run_id = $env:GITHUB_RUN_ID
        artifacts = $inventoryEvidence
        cases = $results
    }
    $outputParent = Split-Path -Parent $OutputPath
    New-Item -ItemType Directory -Force -Path $outputParent | Out-Null
    [System.IO.File]::WriteAllText(
        [System.IO.Path]::GetFullPath($OutputPath),
        ($evidence | ConvertTo-Json -Depth 6),
        $utf8)
} finally {
    Remove-Item Env:COPPERFIN_TEST_LAUNCHER_TRUST_MARKER -ErrorAction SilentlyContinue
    Remove-Item -LiteralPath $packageRoot -Recurse -Force -ErrorAction SilentlyContinue
}
