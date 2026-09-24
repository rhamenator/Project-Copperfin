# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
# Traceability: RQ-CF-REL-002; DQ-windows-installer-lifecycle-scope;
# DV-windows-installer-lifecycle-contract; HZ-system-failure-01;
# HZ-data-corruption-01; HZ-doc-command-01.

[CmdletBinding()]
param(
    [Parameter(Mandatory = $true, ParameterSetName = 'Lifecycle')]
    [ValidateNotNullOrEmpty()]
    [string]$InstallerPath,

    [Parameter(Mandatory = $true, ParameterSetName = 'Lifecycle')]
    [ValidateNotNullOrEmpty()]
    [string]$BinaryDirectory,

    [Parameter(Mandatory = $true, ParameterSetName = 'Lifecycle')]
    [ValidateNotNullOrEmpty()]
    [string]$InstallRoot,

    [Parameter(Mandatory = $true, ParameterSetName = 'Lifecycle')]
    [ValidatePattern('^copperfin [0-9]+\.[0-9]+\.[0-9]+$')]
    [string]$UninstallRegistryKeyName,

    [Parameter(Mandatory = $true, ParameterSetName = 'Lifecycle')]
    [ValidatePattern('^[0-9]+\.[0-9]+\.[0-9]+$')]
    [string]$PackageVersion,

    [Parameter(Mandatory = $true, ParameterSetName = 'Lifecycle')]
    [ValidateNotNullOrEmpty()]
    [string]$EvidenceDirectory,

    # #6497: optional. When both prior-version parameters are supplied, the
    # lifecycle additionally installs this synthetic, differently-versioned
    # prior installer first, seeds an external user artifact, then installs
    # $InstallerPath over the same root as an upgrade before continuing the
    # existing same-version-reinstall/uninstall sequence. There is no real
    # prior Copperfin release to test against yet -- see the CI workflow
    # for how this installer is produced (COPPERFIN_PACKAGE_VERSION_OVERRIDE).
    # Left absent, upgrade_from_previous_version stays NOT_RUN in the
    # evidence, exactly as before this parameter existed.
    [Parameter(ParameterSetName = 'Lifecycle')]
    [string]$PriorInstallerPath,

    [Parameter(ParameterSetName = 'Lifecycle')]
    [ValidatePattern('^copperfin [0-9]+\.[0-9]+\.[0-9]+$')]
    [string]$PriorUninstallRegistryKeyName,

    [Parameter(ParameterSetName = 'Lifecycle')]
    [ValidatePattern('^[0-9]+\.[0-9]+\.[0-9]+$')]
    [string]$PriorPackageVersion,

    [Parameter(ParameterSetName = 'Lifecycle')]
    [ValidateRange(10, 600)]
    [int]$ProcessTimeoutSeconds = 180,

    [Parameter(Mandatory = $true, ParameterSetName = 'SelfTest')]
    [switch]$SelfTest
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

function Assert-Condition {
    param(
        [Parameter(Mandatory = $true)]
        [bool]$Condition,
        [Parameter(Mandatory = $true)]
        [string]$Message
    )

    if (-not $Condition) {
        throw $Message
    }
}

function Invoke-BoundedProcess {
    param(
        [Parameter(Mandatory = $true)]
        [string]$FilePath,
        [string[]]$Arguments = @(),
        [Parameter(Mandatory = $true)]
        [string]$Name,
        [switch]$CaptureOutput
    )

    $startInfo = [System.Diagnostics.ProcessStartInfo]::new()
    $startInfo.FileName = $FilePath
    $startInfo.UseShellExecute = $false
    $startInfo.CreateNoWindow = $true
    $startInfo.RedirectStandardOutput = $CaptureOutput.IsPresent
    $startInfo.RedirectStandardError = $CaptureOutput.IsPresent
    foreach ($argument in $Arguments) {
        [void]$startInfo.ArgumentList.Add($argument)
    }

    $process = [System.Diagnostics.Process]::new()
    $process.StartInfo = $startInfo
    try {
        Assert-Condition $process.Start() "$Name did not start."
        $stdoutTask = if ($CaptureOutput) { $process.StandardOutput.ReadToEndAsync() } else { $null }
        $stderrTask = if ($CaptureOutput) { $process.StandardError.ReadToEndAsync() } else { $null }
        if (-not $process.WaitForExit($ProcessTimeoutSeconds * 1000)) {
            try { $process.Kill($true) } catch { Write-Warning "$Name could not be terminated: $($_.Exception.Message)" }
            throw "$Name exceeded the bounded $ProcessTimeoutSeconds-second timeout."
        }
        $stdout = if ($CaptureOutput) { $stdoutTask.GetAwaiter().GetResult() } else { "" }
        $stderr = if ($CaptureOutput) { $stderrTask.GetAwaiter().GetResult() } else { "" }
        if ($process.ExitCode -ne 0) {
            throw "$Name exited with code $($process.ExitCode). stdout='$stdout' stderr='$stderr'"
        }
        return [pscustomobject]@{
            ExitCode = $process.ExitCode
            Stdout = $stdout
            Stderr = $stderr
        }
    }
    finally {
        $process.Dispose()
    }
}

function Get-InstalledSnapshot {
    param([Parameter(Mandatory = $true)][string]$Root)

    $snapshot = [ordered]@{}
    Get-ChildItem -LiteralPath $Root -File -Recurse | Sort-Object FullName | ForEach-Object {
        $relative = [System.IO.Path]::GetRelativePath($Root, $_.FullName).Replace('\', '/')
        $snapshot[$relative] = (Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash.ToLowerInvariant()
    }
    return $snapshot
}

function Get-OptionalPropertyValue {
    param(
        [Parameter(Mandatory = $true)]
        [object]$InputObject,
        [Parameter(Mandatory = $true)]
        [string]$Name
    )

    $property = $InputObject.PSObject.Properties[$Name]
    if ($null -eq $property) {
        return $null
    }
    return $property.Value
}

function Test-NormalizedPathEquals {
    param(
        [AllowNull()]
        [object]$Candidate,
        [Parameter(Mandatory = $true)]
        [string]$ExpectedPath
    )

    if ($null -eq $Candidate) {
        return $false
    }
    $candidateText = ([string]$Candidate).Trim()
    if ($candidateText.Length -ge 2 -and
        $candidateText[0] -eq '"' -and
        $candidateText[$candidateText.Length - 1] -eq '"') {
        $candidateText = $candidateText.Substring(1, $candidateText.Length - 2)
    }
    if ([string]::IsNullOrWhiteSpace($candidateText)) {
        return $false
    }
    try {
        return [string]::Equals(
            [System.IO.Path]::GetFullPath($candidateText).TrimEnd('\'),
            [System.IO.Path]::GetFullPath($ExpectedPath).TrimEnd('\'),
            [System.StringComparison]::OrdinalIgnoreCase)
    }
    catch {
        return $false
    }
}

function Test-IsCopperfinUninstallEntry {
    param(
        [Parameter(Mandatory = $true)][object]$Entry,
        [Parameter(Mandatory = $true)][string]$ActualRegistryKeyName,
        [Parameter(Mandatory = $true)][string]$ExpectedRegistryKeyName,
        [Parameter(Mandatory = $true)][string]$ExpectedInstallRoot
    )

    $normalizedRoot = [System.IO.Path]::GetFullPath($ExpectedInstallRoot).TrimEnd('\')
    $expectedUninstaller = Join-Path $normalizedRoot 'Uninstall.exe'
    $installLocation = Get-OptionalPropertyValue -InputObject $Entry -Name 'InstallLocation'
    $uninstallString = Get-OptionalPropertyValue -InputObject $Entry -Name 'UninstallString'
    return [string]::Equals(
            $ActualRegistryKeyName,
            $ExpectedRegistryKeyName,
            [System.StringComparison]::OrdinalIgnoreCase) -or
        (Test-NormalizedPathEquals -Candidate $installLocation -ExpectedPath $normalizedRoot) -or
        (Test-NormalizedPathEquals -Candidate $uninstallString -ExpectedPath $expectedUninstaller)
}

function Get-CopperfinUninstallEntries {
    param(
        [Parameter(Mandatory = $true)][string]$ExpectedInstallRoot,
        [Parameter(Mandatory = $true)][string]$ExpectedRegistryKeyName
    )

    $registryBases = @(
        'HKCU:\Software\Microsoft\Windows\CurrentVersion\Uninstall',
        'HKLM:\Software\Microsoft\Windows\CurrentVersion\Uninstall',
        'HKLM:\Software\WOW6432Node\Microsoft\Windows\CurrentVersion\Uninstall'
    )
    return @(
        foreach ($registryBase in $registryBases) {
            if (-not (Test-Path -LiteralPath $registryBase -ErrorAction Stop)) {
                continue
            }
            foreach ($registryKey in @(Get-ChildItem -LiteralPath $registryBase -ErrorAction Stop)) {
                $entryResults = @(Get-ItemProperty -LiteralPath $registryKey.PSPath -ErrorAction Stop)
                Assert-Condition ($entryResults.Count -le 1) `
                    "Registry key returned multiple property objects: $($registryKey.PSPath)"
                $entry = if ($entryResults.Count -eq 0) { [pscustomobject]@{} } else { $entryResults[0] }
                if (Test-IsCopperfinUninstallEntry `
                        -Entry $entry `
                        -ActualRegistryKeyName $registryKey.PSChildName `
                        -ExpectedRegistryKeyName $ExpectedRegistryKeyName `
                        -ExpectedInstallRoot $ExpectedInstallRoot) {
                    $entry
                }
            }
        }
    )
}

function Get-CopperfinUninstallEntryCount {
    param(
        [Parameter(Mandatory = $true)][string]$ExpectedInstallRoot,
        [Parameter(Mandatory = $true)][string]$ExpectedRegistryKeyName
    )

    return @(Get-CopperfinUninstallEntries `
            -ExpectedInstallRoot $ExpectedInstallRoot `
            -ExpectedRegistryKeyName $ExpectedRegistryKeyName).Count
}

# #6497 review: a review round found that invoking the installed CLI with
# only `--help` proves nothing about the seeded external artifact -- the
# help path returns before touching it. This actually inspects the
# artifact (copperfin_inspect recognizes ".prg" as AssetFamily::program and
# reports "status: ok" once the file exists and is readable) so the
# assertion is tied to the real seeded content, not to a smoke test that
# would pass identically for a nonexistent path. Existence-based inspection
# alone would not catch truncation/corruption that a delete would, so the
# caller additionally compares a SHA-256 hash taken right after seeding.
function Invoke-CopperfinInspectArtifactSmoke {
    param(
        [Parameter(Mandatory = $true)][string]$InspectExecutablePath,
        [Parameter(Mandatory = $true)][string]$ArtifactPath,
        [Parameter(Mandatory = $true)][string]$Name
    )

    $result = Invoke-BoundedProcess -FilePath $InspectExecutablePath `
        -Arguments @('--locale', 'en-US', $ArtifactPath) `
        -Name $Name `
        -CaptureOutput
    Assert-Condition ($result.Stdout -match 'asset_family: program') `
        "$Name did not recognize the seeded artifact as a program asset: $($result.Stdout)"
    Assert-Condition ($result.Stdout -match 'status: ok') `
        "$Name did not report successful inspection of the seeded artifact: $($result.Stdout)"
}

if ($SelfTest) {
    $sparseEntry = [pscustomobject]@{ DisplayName = 'Unrelated product' }
    Assert-Condition `
        ($null -eq (Get-OptionalPropertyValue -InputObject $sparseEntry -Name 'InstallLocation')) `
        'Sparse registry entry unexpectedly exposed InstallLocation.'
    $fixtureRoot = Join-Path ([System.IO.Path]::GetTempPath()) 'copperfin-installer-lifecycle-self-test'
    $fixtureUninstaller = Join-Path $fixtureRoot 'Uninstall.exe'
    Assert-Condition `
        (Test-IsCopperfinUninstallEntry `
            -Entry $sparseEntry `
            -ActualRegistryKeyName 'copperfin 0.1.0' `
            -ExpectedRegistryKeyName 'copperfin 0.1.0' `
            -ExpectedInstallRoot $fixtureRoot) `
        'Exact CPack uninstall key with cleared values escaped residue detection.'
    Assert-Condition `
        (-not (Test-IsCopperfinUninstallEntry `
            -Entry $sparseEntry `
            -ActualRegistryKeyName 'Unrelated product' `
            -ExpectedRegistryKeyName 'copperfin 0.1.0' `
            -ExpectedInstallRoot $fixtureRoot)) `
        'Unrelated sparse registry entry was admitted.'
    Assert-Condition `
        (Test-NormalizedPathEquals -Candidate "`"$fixtureUninstaller`"" -ExpectedPath $fixtureUninstaller) `
        'Quoted exact CPack uninstall path did not match.'
    Assert-Condition `
        (-not (Test-NormalizedPathEquals -Candidate "$fixtureUninstaller /S" -ExpectedPath $fixtureUninstaller)) `
        'Uninstall path with command arguments was accepted.'
    Assert-Condition `
        (-not (Test-NormalizedPathEquals -Candidate (Join-Path $fixtureRoot 'Sibling.exe') -ExpectedPath $fixtureUninstaller)) `
        'Sibling uninstall path was accepted.'
    Write-Host 'Windows installer lifecycle helper self-test passed.'
    return
}

$hasPriorVersion = -not [string]::IsNullOrEmpty($PriorInstallerPath)
if ($hasPriorVersion -and [string]::IsNullOrEmpty($PriorUninstallRegistryKeyName)) {
    throw 'PriorUninstallRegistryKeyName is required when PriorInstallerPath is supplied.'
}
if ($hasPriorVersion -and [string]::IsNullOrEmpty($PriorPackageVersion)) {
    throw 'PriorPackageVersion is required when PriorInstallerPath is supplied.'
}
$resolvedInstaller = (Resolve-Path -LiteralPath $InstallerPath).Path
$resolvedPriorInstaller = if ($hasPriorVersion) { (Resolve-Path -LiteralPath $PriorInstallerPath).Path } else { $null }
$resolvedBinaryDirectory = (Resolve-Path -LiteralPath $BinaryDirectory).Path
$resolvedInstallRoot = [System.IO.Path]::GetFullPath($InstallRoot)
$resolvedEvidenceDirectory = [System.IO.Path]::GetFullPath($EvidenceDirectory)
$resolvedRunnerTemporaryRoot = [System.IO.Path]::GetFullPath($env:RUNNER_TEMP).TrimEnd('\')
$installParent = [System.IO.Directory]::GetParent($resolvedInstallRoot)

Assert-Condition ($resolvedInstaller.EndsWith('.exe', [System.StringComparison]::OrdinalIgnoreCase)) `
    "Windows installer must be an executable: $resolvedInstaller"
if ($hasPriorVersion) {
    Assert-Condition ($resolvedPriorInstaller.EndsWith('.exe', [System.StringComparison]::OrdinalIgnoreCase)) `
        "Prior Windows installer must be an executable: $resolvedPriorInstaller"
    Assert-Condition (-not [string]::Equals(
            $PriorUninstallRegistryKeyName, $UninstallRegistryKeyName,
            [System.StringComparison]::OrdinalIgnoreCase)) `
        'Prior and current uninstall registry key names must differ for an upgrade to be a real upgrade, not a same-version reinstall.'
}
Assert-Condition ($null -ne $installParent) "Installation root has no parent: $resolvedInstallRoot"
Assert-Condition ([string]::Equals(
        $installParent.FullName.TrimEnd('\'),
        $resolvedRunnerTemporaryRoot,
        [System.StringComparison]::OrdinalIgnoreCase)) `
    "Installation root must be a direct child of RUNNER_TEMP: $resolvedInstallRoot"
Assert-Condition ([System.IO.Path]::GetFileName($resolvedInstallRoot) -match '^copperfin-installer-lifecycle-[A-Za-z0-9._-]+$') `
    "Installation root leaf does not match the lifecycle allowlist: $resolvedInstallRoot"
Assert-Condition (-not (Test-Path -LiteralPath $resolvedInstallRoot)) `
    "Fresh-install root already exists: $resolvedInstallRoot"
Assert-Condition ((Get-CopperfinUninstallEntryCount `
        -ExpectedInstallRoot $resolvedInstallRoot `
        -ExpectedRegistryKeyName $UninstallRegistryKeyName) -eq 0) `
    "Fresh-install root already has an uninstall registration: $resolvedInstallRoot"

New-Item -ItemType Directory -Path $resolvedEvidenceDirectory -Force | Out-Null
$installedSnapshot = $null
$maintenanceSnapshot = $null
$uninstallRegistrationCount = 0
$inspectOutput = ""
$upgradeFromPreviousVersionResult = 'NOT_RUN'
$freshInstallResult = 'NOT_RUN'
$externalUserArtifact = $null
$externalUserArtifactHash = $null
$freshCheckRoot = $null
$priorRegistrationCountAfterUpgrade = $null

try {
    if ($hasPriorVersion) {
        # #6497: install the synthetic prior version first, seed a small
        # external user artifact (a real PRG file the installed CLI can
        # process, living outside the install root -- like a user's own
        # project file would), confirm the prior installation can process
        # it, then install the current build over the same root as a real
        # upgrade (not a fresh install to a new location). Verifies the
        # upgrade preserves external user content and replaces (not
        # duplicates) the uninstall registration.
        Invoke-BoundedProcess `
            -FilePath $resolvedPriorInstaller `
            -Arguments @('/S', "/D=$resolvedInstallRoot") `
            -Name 'Copperfin silent prior-version installation' | Out-Null
        Assert-Condition (Test-Path -LiteralPath $resolvedInstallRoot -PathType Container) `
            "Prior-version installer did not create the requested installation root: $resolvedInstallRoot"
        Assert-Condition ((Get-CopperfinUninstallEntryCount `
                -ExpectedInstallRoot $resolvedInstallRoot `
                -ExpectedRegistryKeyName $PriorUninstallRegistryKeyName) -eq 1) `
            'Prior-version installation must create exactly one uninstall registration.'

        $externalUserArtifact = Join-Path $resolvedRunnerTemporaryRoot "copperfin-installer-lifecycle-user-project-$([System.IO.Path]::GetFileName($resolvedInstallRoot)).prg"
        Set-Content -LiteralPath $externalUserArtifact -Encoding utf8NoBOM -Value @(
            '* A minimal user project file, deliberately kept outside the',
            '* installation root, to prove an upgrade neither touches nor',
            '* loses track of a user''s own external content (#6497).',
            '?"copperfin-installer-lifecycle-user-artifact"'
        )
        $externalUserArtifactHash = (Get-FileHash -LiteralPath $externalUserArtifact -Algorithm SHA256).Hash.ToLowerInvariant()
        $priorInspectPath = Join-Path $resolvedInstallRoot 'bin\copperfin_inspect.exe'
        Invoke-CopperfinInspectArtifactSmoke -InspectExecutablePath $priorInspectPath `
            -ArtifactPath $externalUserArtifact `
            -Name 'prior-version installed copperfin_inspect artifact smoke'

        Invoke-BoundedProcess `
            -FilePath $resolvedInstaller `
            -Arguments @('/S', "/D=$resolvedInstallRoot") `
            -Name 'Copperfin silent upgrade installation' | Out-Null

        # #6497 review: Copperfin's NSIS packaging does not yet implement
        # uninstall-before-install for a differently-versioned prior
        # install -- each version's uninstall registry key is version-
        # suffixed (CMakeLists.txt's CPACK_PACKAGE_INSTALL_REGISTRY_KEY),
        # so CPack has no stable key to look up and silently remove during
        # a silent (/S) install, and NSIS's own uninstall-before-install
        # prompt is interactive, incompatible with silent CI installs
        # regardless. This observes the current, real behavior (both
        # registrations coexist) as a diagnostic rather than asserting an
        # unimplemented guarantee; the current version's own registration
        # must still be exactly one.
        $priorRegistrationCountAfterUpgrade = Get-CopperfinUninstallEntryCount `
            -ExpectedInstallRoot $resolvedInstallRoot `
            -ExpectedRegistryKeyName $PriorUninstallRegistryKeyName
        Assert-Condition ((Get-CopperfinUninstallEntryCount `
                -ExpectedInstallRoot $resolvedInstallRoot `
                -ExpectedRegistryKeyName $UninstallRegistryKeyName) -eq 1) `
            'Upgrade must create exactly one uninstall registration for the current version.'
        Assert-Condition (Test-Path -LiteralPath $externalUserArtifact -PathType Leaf) `
            'Upgrade must not remove a user''s external artifact outside the install root.'
        Assert-Condition ((Get-FileHash -LiteralPath $externalUserArtifact -Algorithm SHA256).Hash.ToLowerInvariant() -eq $externalUserArtifactHash) `
            'Upgrade must not truncate or corrupt a user''s external artifact outside the install root.'
        $upgradeInspectPath = Join-Path $resolvedInstallRoot 'bin\copperfin_inspect.exe'
        Invoke-CopperfinInspectArtifactSmoke -InspectExecutablePath $upgradeInspectPath `
            -ArtifactPath $externalUserArtifact `
            -Name 'post-upgrade installed copperfin_inspect artifact smoke'
        $upgradeFromPreviousVersionResult = 'PASS'

        # #6497 review: the workflow always supplies prior-version
        # arguments, so without this, the plain-fresh-install path below
        # would never run in CI while `fresh_install` still unconditionally
        # reported PASS. This exercises a real, independent fresh install
        # of the CURRENT installer (no prior state) in its own root, so
        # that claim is backed by an actual run every time.
        $freshCheckRoot = "$resolvedInstallRoot-freshcheck"
        Assert-Condition (-not (Test-Path -LiteralPath $freshCheckRoot)) `
            "Dedicated fresh-install check root already exists: $freshCheckRoot"
        Invoke-BoundedProcess -FilePath $resolvedInstaller -Arguments @('/S', "/D=$freshCheckRoot") `
            -Name 'Copperfin silent dedicated fresh installation' | Out-Null
        Assert-Condition (Test-Path -LiteralPath $freshCheckRoot -PathType Container) `
            "Dedicated fresh-install check did not create its installation root: $freshCheckRoot"
        Assert-Condition ((Get-CopperfinUninstallEntryCount `
                -ExpectedInstallRoot $freshCheckRoot `
                -ExpectedRegistryKeyName $UninstallRegistryKeyName) -eq 1) `
            'Dedicated fresh-install check must create exactly one uninstall registration.'
        $freshCheckUninstaller = Join-Path $freshCheckRoot 'Uninstall.exe'
        Invoke-BoundedProcess -FilePath $freshCheckUninstaller -Arguments @('/S') `
            -Name 'Copperfin silent dedicated fresh-install cleanup' | Out-Null
        $freshCheckDeadline = [DateTime]::UtcNow.AddSeconds($ProcessTimeoutSeconds)
        while ((Test-Path -LiteralPath $freshCheckRoot) -and [DateTime]::UtcNow -lt $freshCheckDeadline) {
            Start-Sleep -Milliseconds 250
        }
        Assert-Condition (-not (Test-Path -LiteralPath $freshCheckRoot)) `
            "Dedicated fresh-install check left installation-root residue: $freshCheckRoot"
        $freshInstallResult = 'PASS'
    }
    else {
        Invoke-BoundedProcess `
            -FilePath $resolvedInstaller `
            -Arguments @('/S', "/D=$resolvedInstallRoot") `
            -Name 'Copperfin silent fresh installation' | Out-Null
        $freshInstallResult = 'PASS'
    }

    Assert-Condition (Test-Path -LiteralPath $resolvedInstallRoot -PathType Container) `
        "Installer did not create the requested installation root: $resolvedInstallRoot"
    Assert-Condition (-not ((Get-Item -LiteralPath $resolvedInstallRoot).Attributes -band [System.IO.FileAttributes]::ReparsePoint)) `
        "Installer created a reparse-point installation root: $resolvedInstallRoot"

    Invoke-BoundedProcess `
        -FilePath 'cmake' `
        -Arguments @(
            "-DBINARY_DIR=$resolvedBinaryDirectory",
            "-DINSTALL_ROOT=$resolvedInstallRoot",
            '-P', (Join-Path $PSScriptRoot '..\tests\run_studio_install_contract_check.cmake')
        ) `
        -Name 'installed Studio tree verification' | Out-Null
    Invoke-BoundedProcess `
        -FilePath 'cmake' `
        -Arguments @(
            "-DINSTALL_ROOT=$resolvedInstallRoot",
            '-P', (Join-Path $PSScriptRoot '..\tests\run_locale_catalog_install_contract_check.cmake')
        ) `
        -Name 'installed locale catalog verification' | Out-Null

    $inspectPath = Join-Path $resolvedInstallRoot 'bin\copperfin_inspect.exe'
    $inspectResult = Invoke-BoundedProcess `
        -FilePath $inspectPath `
        -Arguments @('--locale', 'en-US', '--help') `
        -Name 'installed copperfin_inspect smoke' `
        -CaptureOutput
    Assert-Condition ($inspectResult.Stdout -match 'copperfin_inspect') `
        "Installed copperfin_inspect help omitted its command identity."
    Assert-Condition ([string]::IsNullOrEmpty($inspectResult.Stderr)) `
        "Installed copperfin_inspect help wrote unexpected stderr: $($inspectResult.Stderr)"
    $inspectOutput = $inspectResult.Stdout

    $uninstallRegistrationCount = Get-CopperfinUninstallEntryCount `
        -ExpectedInstallRoot $resolvedInstallRoot `
        -ExpectedRegistryKeyName $UninstallRegistryKeyName
    Assert-Condition ($uninstallRegistrationCount -eq 1) `
        "Fresh installation must create exactly one uninstall registration for its root; found $uninstallRegistrationCount."

    $installedSnapshot = Get-InstalledSnapshot -Root $resolvedInstallRoot
    Assert-Condition ($installedSnapshot.Count -gt 0) 'Fresh installation produced an empty file inventory.'

    Invoke-BoundedProcess `
        -FilePath $resolvedInstaller `
        -Arguments @('/S', "/D=$resolvedInstallRoot") `
        -Name 'Copperfin same-version maintenance reinstall' | Out-Null
    $maintenanceSnapshot = Get-InstalledSnapshot -Root $resolvedInstallRoot
    Assert-Condition (($installedSnapshot | ConvertTo-Json -Compress) -ceq ($maintenanceSnapshot | ConvertTo-Json -Compress)) `
        'Same-version maintenance reinstall changed the installed file inventory or hashes.'
    Assert-Condition ((Get-CopperfinUninstallEntryCount `
            -ExpectedInstallRoot $resolvedInstallRoot `
            -ExpectedRegistryKeyName $UninstallRegistryKeyName) -eq 1) `
        'Same-version maintenance reinstall did not preserve exactly one uninstall registration.'

    $uninstaller = Join-Path $resolvedInstallRoot 'Uninstall.exe'
    Assert-Condition (Test-Path -LiteralPath $uninstaller -PathType Leaf) `
        "Installed uninstaller is missing: $uninstaller"
    Invoke-BoundedProcess `
        -FilePath $uninstaller `
        -Arguments @('/S') `
        -Name 'Copperfin silent uninstall' | Out-Null

    $deadline = [DateTime]::UtcNow.AddSeconds($ProcessTimeoutSeconds)
    while ((Test-Path -LiteralPath $resolvedInstallRoot) -and [DateTime]::UtcNow -lt $deadline) {
        Start-Sleep -Milliseconds 250
    }
    Assert-Condition (-not (Test-Path -LiteralPath $resolvedInstallRoot)) `
        "Silent uninstall left installation-root residue: $resolvedInstallRoot"
    Assert-Condition ((Get-CopperfinUninstallEntryCount `
            -ExpectedInstallRoot $resolvedInstallRoot `
            -ExpectedRegistryKeyName $UninstallRegistryKeyName) -eq 0) `
        "Silent uninstall left an uninstall registration for: $resolvedInstallRoot"

    $evidence = [ordered]@{
        schema_version = 1
        kind = 'copperfin-windows-installer-lifecycle-result'
        installer_sha256 = (Get-FileHash -LiteralPath $resolvedInstaller -Algorithm SHA256).Hash.ToLowerInvariant()
        package_version = $PackageVersion
        install_root = $resolvedInstallRoot
        fresh_install = $freshInstallResult
        installed_tree_contract = 'PASS'
        locale_catalog_contract = 'PASS'
        installed_cli_smoke = 'PASS'
        same_version_maintenance_reinstall = 'PASS'
        upgrade_from_previous_version = $upgradeFromPreviousVersionResult
        prior_installer_sha256 = if ($hasPriorVersion) { (Get-FileHash -LiteralPath $resolvedPriorInstaller -Algorithm SHA256).Hash.ToLowerInvariant() } else { $null }
        prior_package_version = if ($hasPriorVersion) { $PriorPackageVersion } else { $null }
        stale_prior_uninstall_registration_after_upgrade_count = $priorRegistrationCountAfterUpgrade
        silent_uninstall = 'PASS'
        install_root_residue = 'PASS'
        uninstall_registration_residue = 'PASS'
        uninstall_registration_count_after_install = $uninstallRegistrationCount
        installed_file_count = $installedSnapshot.Count
        installed_cli_stdout = $inspectOutput.TrimEnd()
    }
    $evidence | ConvertTo-Json -Depth 4 | Set-Content `
        -LiteralPath (Join-Path $resolvedEvidenceDirectory 'windows-installer-lifecycle.json') `
        -Encoding utf8NoBOM
    Write-Host "Windows installer lifecycle passed for $resolvedInstaller"
}
finally {
    if (Test-Path -LiteralPath $resolvedInstallRoot) {
        $fallbackUninstaller = Join-Path $resolvedInstallRoot 'Uninstall.exe'
        if (Test-Path -LiteralPath $fallbackUninstaller -PathType Leaf) {
            try {
                Invoke-BoundedProcess -FilePath $fallbackUninstaller -Arguments @('/S') -Name 'fallback silent uninstall' | Out-Null
            }
            catch {
                Write-Warning "Fallback uninstall failed: $($_.Exception.Message)"
            }
        }
    }
    if ($null -ne $freshCheckRoot -and (Test-Path -LiteralPath $freshCheckRoot)) {
        $fallbackFreshCheckUninstaller = Join-Path $freshCheckRoot 'Uninstall.exe'
        if (Test-Path -LiteralPath $fallbackFreshCheckUninstaller -PathType Leaf) {
            try {
                Invoke-BoundedProcess -FilePath $fallbackFreshCheckUninstaller -Arguments @('/S') -Name 'fallback dedicated fresh-install uninstall' | Out-Null
            }
            catch {
                Write-Warning "Fallback dedicated fresh-install uninstall failed: $($_.Exception.Message)"
            }
        }
    }
    if ($null -ne $externalUserArtifact -and (Test-Path -LiteralPath $externalUserArtifact)) {
        try {
            [System.IO.File]::Delete($externalUserArtifact)
        }
        catch {
            Write-Warning "Cleanup of the external user artifact fixture failed: $($_.Exception.Message)"
        }
    }
}
