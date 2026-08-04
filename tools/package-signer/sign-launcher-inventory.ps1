# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$InputPath,
    [Parameter(Mandatory = $true)]
    [string]$OutputPath,
    [Parameter(Mandatory = $true)]
    [string]$KeyRef,
    [Parameter(Mandatory = $true)]
    [string]$RepositoryRoot
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

$input = Resolve-RequiredFile $InputPath "Launcher inventory envelope"
$key = Resolve-RequiredFile $KeyRef "External signing key"
$root = [System.IO.Path]::GetFullPath((Resolve-Path -LiteralPath $RepositoryRoot).Path)
$output = [System.IO.Path]::GetFullPath($OutputPath)
if ($input.Equals($output, [System.StringComparison]::OrdinalIgnoreCase)) {
    throw "Input and output paths must differ."
}
if (Test-PathInsideRoot $key $root) {
    throw "Refusing a signing key inside the repository checkout."
}
if (-not (Get-Command openssl -ErrorAction SilentlyContinue)) {
    throw "OpenSSL is required for external Ed25519 signing."
}

$bytes = [System.IO.File]::ReadAllBytes($input)
if ($bytes.Length -eq 0 -or $bytes[$bytes.Length - 1] -ne 10 -or $bytes -contains 13) {
    throw "Input envelope must use canonical LF line endings and end with LF."
}
$strictUtf8 = [System.Text.UTF8Encoding]::new($false, $true)
try {
    $text = $strictUtf8.GetString($bytes)
} catch {
    throw "Input envelope must be valid UTF-8."
}
if ($text.EndsWith("`n`n", [System.StringComparison]::Ordinal)) {
    throw "Input envelope must end with exactly one LF."
}
$lines = $text.Split([char]10, [System.StringSplitOptions]::None)
if ($lines.Length -lt 6 -or
    $lines[0] -ne "launcher_inventory_version=1" -or
    $lines[1] -ne "hash_algorithm=sha256" -or
    $lines[2] -ne "signature_algorithm=ed25519") {
    throw "Input is not a canonical version 1 launcher inventory envelope."
}
$signerPrefix = "signer_key_id="
if (-not $lines[3].StartsWith($signerPrefix, [System.StringComparison]::Ordinal)) {
    throw "Input is missing signer_key_id."
}
$signerKeyId = $lines[3].Substring($signerPrefix.Length)
if ($signerKeyId -notmatch '^[A-Za-z0-9._-]+$') {
    throw "Input contains an invalid signer_key_id."
}

$outputParent = Split-Path -Parent $output
if (-not (Test-Path -LiteralPath $outputParent -PathType Container)) {
    throw "Output directory does not exist: $outputParent"
}
$rawSignature = Join-Path $outputParent (".app.cftrust.raw." + [Guid]::NewGuid().ToString("N"))
$temporaryOutput = Join-Path $outputParent (".app.cftrust.sig." + [Guid]::NewGuid().ToString("N"))
try {
    & openssl pkeyutl -sign -rawin -inkey $key -in $input -out $rawSignature
    if ($LASTEXITCODE -ne 0) {
        throw "OpenSSL failed to sign the launcher inventory."
    }
    $signatureBytes = [System.IO.File]::ReadAllBytes($rawSignature)
    if ($signatureBytes.Length -ne 64) {
        throw "OpenSSL did not produce a canonical 64-byte Ed25519 signature."
    }
    $sidecar = "launcher_signature_version=1`n" +
        "signature_algorithm=ed25519`n" +
        "signer_key_id=$signerKeyId`n" +
        "signature_base64=$([Convert]::ToBase64String($signatureBytes))`n"
    [System.IO.File]::WriteAllText(
        $temporaryOutput,
        $sidecar,
        [System.Text.UTF8Encoding]::new($false))
    [System.IO.File]::Move($temporaryOutput, $output, $true)
} finally {
    Remove-Item -LiteralPath $rawSignature -Force -ErrorAction SilentlyContinue
    Remove-Item -LiteralPath $temporaryOutput -Force -ErrorAction SilentlyContinue
}
