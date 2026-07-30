# Copyright (c) 2026 Richard M. Hamilton. All rights reserved.
# Licensed under the Project Copperfin Source-Available License or
# Commercial License. See LICENSE.md in the repository root.

[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$RegistryHeaderPath,
    [Parameter(Mandatory = $true)]
    [string]$SigningKeyPath,
    [Parameter(Mandatory = $true)]
    [string]$SignerKeyId,
    [Parameter(Mandatory = $true)]
    [string]$RepositoryRoot,
    [Parameter(Mandatory = $true)]
    [string]$OutputPath
)

$ErrorActionPreference = "Stop"

function Resolve-RequiredFile {
    param([string]$Path, [string]$Description)
    if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) {
        throw "$Description does not exist as a regular file: $Path"
    }
    return [System.IO.Path]::GetFullPath((Resolve-Path -LiteralPath $Path).Path)
}

function Test-PathInsideRoot {
    param([string]$Candidate, [string]$Root)
    $rootPrefix = $Root.TrimEnd([char[]]@('\', '/')) + [System.IO.Path]::DirectorySeparatorChar
    return $Candidate.Equals($Root, [System.StringComparison]::OrdinalIgnoreCase) -or
        $Candidate.StartsWith($rootPrefix, [System.StringComparison]::OrdinalIgnoreCase)
}

$resolvedRoot = [System.IO.Path]::GetFullPath((Resolve-Path -LiteralPath $RepositoryRoot).Path)
$registryPath = Resolve-RequiredFile $RegistryHeaderPath "Launcher trust registry header"
$signingKeyPath = Resolve-RequiredFile $SigningKeyPath "Launcher trust signing key"

if (Test-PathInsideRoot $registryPath $resolvedRoot) {
    throw "Launcher trust registry must be supplied outside the repository checkout."
}
if (Test-PathInsideRoot $signingKeyPath $resolvedRoot) {
    throw "Launcher trust signing key must remain outside the repository checkout."
}
if ($registryPath.Equals($signingKeyPath, [System.StringComparison]::OrdinalIgnoreCase)) {
    throw "Launcher trust registry and signing key must be different files."
}

$registryText = [System.IO.File]::ReadAllText($registryPath)
$signingKeyText = [System.IO.File]::ReadAllText($signingKeyPath)
if ($registryText -match '(?i)BEGIN\s+(RSA |EC |OPENSSH )?PRIVATE KEY') {
    throw "Launcher trust registry must contain public keys only."
}
if ($registryText -notmatch 'kKnownLauncherInventoryTrustedKeys' -or
    $registryText -notmatch 'LauncherInventoryTrustedKey' -or
    $registryText -notmatch 'std::array\s*<\s*LauncherInventoryTrustedKey\s*,\s*[1-9][0-9]*\s*>') {
    throw "Launcher trust registry must define a non-empty kKnownLauncherInventoryTrustedKeys array."
}
if ($signingKeyText -notmatch '(?i)BEGIN\s+PRIVATE KEY') {
    throw "Launcher trust signing key must be a PEM private-key reference."
}
if ($SignerKeyId -notmatch '^[A-Za-z0-9._-]+$') {
    throw "Launcher trust signer key ID is invalid."
}
$escapedSignerKeyId = [regex]::Escape($SignerKeyId)
if ($registryText -notmatch ('"' + $escapedSignerKeyId + '"')) {
    throw "Launcher trust signer key ID is absent from the approved registry."
}

$registryEntryMatch = [regex]::Match(
    $registryText,
    'std::array\s*<\s*LauncherInventoryTrustedKey\s*,\s*([1-9][0-9]*)\s*>')
$registryEntryCount = [int]$registryEntryMatch.Groups[1].Value
$output = [ordered]@{
    schema_version = 1
    kind = "windows-launcher-trust-provisioning"
    trust_enforced = $true
    registry_valid = $true
    registry_entry_count = $registryEntryCount
    signing_key_present = $true
    signer_key_id = $SignerKeyId
    signing_key_registry_binding = $true
    private_material_outside_repository = $true
}
$outputParent = Split-Path -Parent $OutputPath
if ([string]::IsNullOrWhiteSpace($outputParent)) {
    $outputParent = "."
}
New-Item -ItemType Directory -Force -Path $outputParent | Out-Null
$output | ConvertTo-Json -Depth 4 | Set-Content -LiteralPath $OutputPath -Encoding utf8

if (-not [string]::IsNullOrWhiteSpace($env:GITHUB_ENV)) {
    "COPPERFIN_LAUNCHER_TRUST_REGISTRY_HEADER=$registryPath" |
        Out-File -FilePath $env:GITHUB_ENV -Encoding utf8 -Append
    "COPPERFIN_LAUNCHER_TRUST_SIGNING_KEY_REF=$signingKeyPath" |
        Out-File -FilePath $env:GITHUB_ENV -Encoding utf8 -Append
    "COPPERFIN_TRUST_SIGNER_KEY_ID=$SignerKeyId" |
        Out-File -FilePath $env:GITHUB_ENV -Encoding utf8 -Append
}
if (-not [string]::IsNullOrWhiteSpace($env:GITHUB_OUTPUT)) {
    "registry_valid=true" | Out-File -FilePath $env:GITHUB_OUTPUT -Encoding utf8 -Append
}

Write-Host "Validated external launcher trust registry with $registryEntryCount key record(s)."
