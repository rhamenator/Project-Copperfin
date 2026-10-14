# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# RQ-CF-CI-WINDOWS-NATIVE-BUDGET-001 / VR-CI-7181-001/002.
[CmdletBinding()]
param(
    [string]$BuildDirectory = 'build',
    [string]$Configuration = 'Release',
    [ValidateRange(1, 3)][int]$ShardIndex = 1,
    [ValidateSet(1, 3)][int]$ShardCount = 1,
    [Parameter(Mandatory)][string]$MetricsDirectory,
    [switch]$InventoryOnly
)
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'native-test-shard-policy.ps1')
if ($ShardIndex -gt $ShardCount) { throw 'Shard index exceeds the bounded count.' }
$evidenceRoot = [IO.Path]::GetFullPath((Join-Path $MetricsDirectory "shard-$ShardIndex-of-$ShardCount"))
New-Item -ItemType Directory -Force -Path $evidenceRoot | Out-Null
$commonArguments = @('--test-dir', $BuildDirectory, '-C', $Configuration)
function Read-CtestInventory {
    param([string[]]$Selection = @())
    $raw = (& ctest @commonArguments --show-only=json-v1 @Selection | Out-String)
    if ($LASTEXITCODE -ne 0) { throw 'CTest inventory generation failed.' }
    return [pscustomobject]@{ raw = $raw; data = ($raw | ConvertFrom-Json) }
}
$full = Read-CtestInventory
$plan = Get-NativeTestShardPlan -Tests @($full.data.tests) -ShardIndex $ShardIndex -ShardCount $ShardCount
$selected = Read-CtestInventory -Selection @('-I', $plan.ctest_selector)
Assert-NativeTestShardSelection -FullTests @($full.data.tests) -SelectedTests @($selected.data.tests) -Plan $plan
$full.raw | Set-Content -LiteralPath (Join-Path $evidenceRoot 'full-inventory.json') -Encoding utf8
$selected.raw | Set-Content -LiteralPath (Join-Path $evidenceRoot 'selected-inventory.json') -Encoding utf8
$plan | ConvertTo-Json -Depth 100 | Set-Content -LiteralPath (Join-Path $evidenceRoot 'selection-plan.json') -Encoding utf8
Write-Host ("Verified shard {0}/{1}: {2} of {3} tests; complete partition covers all tests exactly once." -f
    $ShardIndex, $ShardCount, $plan.selected_names.Count, $plan.full_count)
if ($InventoryOnly) {
    Write-Host 'Inventory-only check: no test execution or native qualification claimed.'
    return
}
foreach ($test in $selected.data.tests) {
    $command = $test.PSObject.Properties['command']
    if ($null -eq $command -or @($command.Value).Count -eq 0) {
        throw 'Every executing test requires a built command, not an unbuilt inventory.'
    }
}
$resultsPath = Join-Path $evidenceRoot 'results.xml'
& (Join-Path $PSScriptRoot 'measure-windows-validation.ps1') -Mode Run -MetricsDirectory $MetricsDirectory `
    -Name "Run native test suite shard $ShardIndex of $ShardCount" -Category native-test -FilePath ctest `
    -CommandArguments ($commonArguments + @('--output-on-failure', '--timeout', '180', '--parallel', '2',
        '-I', $plan.ctest_selector, '--output-junit', $resultsPath)) -SampleResources
Assert-NativeTestShardResults -Results ([xml](Get-Content -Raw -LiteralPath $resultsPath)) -Plan $plan
$after = Read-CtestInventory -Selection @('-I', $plan.ctest_selector)
Assert-NativeTestShardSelection -FullTests @($full.data.tests) -SelectedTests @($after.data.tests) -Plan $plan
$after.raw | Set-Content -LiteralPath (Join-Path $evidenceRoot 'selected-inventory-after.json') -Encoding utf8
Write-Host 'All selected tests completed successfully; commands/properties remained unchanged.'
