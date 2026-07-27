# Copyright (c) 2026 Richard M. Hamilton. All rights reserved.
# Licensed under the Project Copperfin Source-Available License or
# Commercial License. See LICENSE.md in the repository root.

[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [ValidateNotNullOrEmpty()]
    [string]$VsixPath
)

$ErrorActionPreference = "Stop"

Add-Type -AssemblyName System.IO.Compression.FileSystem

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

$resolvedVsixPath = (Resolve-Path -LiteralPath $VsixPath).Path
$extractRoot = Join-Path ([System.IO.Path]::GetTempPath()) ("copperfin-vsix-localization-" + [guid]::NewGuid())

$expectedSatellites = @(
    @{
        RelativePath = "es/Copperfin.VisualStudio.resources.dll"
        Culture = "es"
        ResourceName = "CopperfinCommandResources.es.resources"
        CommandTableKey = "Copperfin.es.CTMENU"
    },
    @{
        RelativePath = "pt/Copperfin.VisualStudio.resources.dll"
        Culture = "pt"
        ResourceName = "CopperfinCommandResources.pt.resources"
        CommandTableKey = "Copperfin.pt.CTMENU"
    },
    @{
        RelativePath = "qps-ploc/Copperfin.VisualStudio.resources.dll"
        Culture = "qps-ploc"
        ResourceName = "CopperfinCommandResources.qps-ploc.resources"
        CommandTableKey = "Copperfin.qps-ploc.CTMENU"
    }
)

try {
    [System.IO.Compression.ZipFile]::ExtractToDirectory($resolvedVsixPath, $extractRoot)

    $mainAssemblyPath = Join-Path $extractRoot "Copperfin.VisualStudio.dll"
    Assert-Condition (Test-Path -LiteralPath $mainAssemblyPath -PathType Leaf) `
        "VSIX is missing Copperfin.VisualStudio.dll: $resolvedVsixPath"

    $pkgdefPath = Join-Path $extractRoot "Copperfin.VisualStudio.pkgdef"
    Assert-Condition (Test-Path -LiteralPath $pkgdefPath -PathType Leaf) `
        "VSIX is missing Copperfin.VisualStudio.pkgdef: $resolvedVsixPath"
    $pkgdefText = Get-Content -LiteralPath $pkgdefPath -Raw
    $expectedCodeBasePattern = '(?m)^\s*"CodeBase"\s*=\s*"\$PackageFolder\$\\Copperfin\.VisualStudio\.dll"\s*$'
    Assert-Condition ($pkgdefText -match $expectedCodeBasePattern) `
        "VSIX pkgdef is missing the exact Copperfin.VisualStudio.dll CodeBase registration: $pkgdefPath"

    foreach ($extension in @(".pjx", ".scx", ".vcx", ".frx", ".lbx", ".mnx")) {
        $expectedEditorExtensionPattern = '(?im)"{0}"\s*=\s*dword:00000064' -f [regex]::Escape($extension)
        Assert-Condition ($pkgdefText -match $expectedEditorExtensionPattern) `
            "VSIX pkgdef is missing Copperfin default editor registration for $($extension): $pkgdefPath"
    }
    $designerLogicalViewGuid = "7651A702-06E5-11D1-8EBD-00A0C90F26EA"
    Assert-Condition ($pkgdefText -match [regex]::Escape($designerLogicalViewGuid)) `
        "VSIX pkgdef is missing the trusted Copperfin Designer logical view: $pkgdefPath"

    foreach ($expected in $expectedSatellites) {
        $relativePath = $expected.RelativePath.Replace('/', [System.IO.Path]::DirectorySeparatorChar)
        $satellitePath = Join-Path $extractRoot $relativePath
        Assert-Condition (Test-Path -LiteralPath $satellitePath -PathType Leaf) `
            "VSIX is missing localized command satellite: $($expected.RelativePath)"

        $assembly = [System.Reflection.Assembly]::LoadFile($satellitePath)
        $actualCulture = $assembly.GetName().CultureName
        Assert-Condition ([string]::Equals($actualCulture, $expected.Culture, [System.StringComparison]::OrdinalIgnoreCase)) `
            "Satellite $($expected.RelativePath) has culture '$actualCulture'; expected '$($expected.Culture)'"

        $resourceNames = $assembly.GetManifestResourceNames()
        Assert-Condition ($resourceNames -ccontains $expected.ResourceName) `
            "Satellite $($expected.RelativePath) is missing resource $($expected.ResourceName)"

        $resourceStream = $assembly.GetManifestResourceStream($expected.ResourceName)
        Assert-Condition ($null -ne $resourceStream) `
            "Satellite $($expected.RelativePath) could not open resource $($expected.ResourceName)"

        $reader = [System.Resources.ResourceReader]::new($resourceStream)
        try {
            $resources = [System.Collections.Generic.Dictionary[string, object]]::new(
                [System.StringComparer]::Ordinal)
            foreach ($entry in $reader) {
                $resources.Add($entry.Key, $entry.Value)
            }

            foreach ($resourceKey in @("Menus.ctmenu", $expected.CommandTableKey)) {
                Assert-Condition ($resources.ContainsKey($resourceKey)) `
                    "Satellite $($expected.RelativePath) is missing command-table resource $resourceKey"
                $resourceValue = $resources[$resourceKey]
                Assert-Condition ($resourceValue -is [byte[]] -and $resourceValue.Length -gt 0) `
                    "Satellite $($expected.RelativePath) has an empty or invalid command-table resource $resourceKey"
            }
        }
        finally {
            $reader.Dispose()
            $resourceStream.Dispose()
        }
    }

    Write-Host "Verified localized command resources in $resolvedVsixPath"
}
finally {
    Remove-Item -LiteralPath $extractRoot -Recurse -Force -ErrorAction SilentlyContinue
}
