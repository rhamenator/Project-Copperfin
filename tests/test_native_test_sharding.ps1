# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# VR-CI-7181-001: synthetic policy checks AND actual tiny CTest execution.
param([string]$EvidenceDirectory = '')
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$repo = Split-Path $PSScriptRoot -Parent
. (Join-Path $repo 'scripts/native-test-shard-policy.ps1')
function Require {
    param([bool]$Condition, [string]$Description)
    if (-not $Condition) { throw $Description }
}
function Reject {
    param([scriptblock]$Action, [string]$Description)
    $rejected = $false
    try { & $Action | Out-Null } catch { $rejected = $true }
    Require $rejected $Description
}
function Make-Test {
    param([string]$Name)
    return [pscustomobject]@{ name = $Name; command = @('cmake', '-E', 'true'); properties = @(
        [pscustomobject]@{ name = 'RUN_SERIAL'; value = $true },
        [pscustomobject]@{ name = 'LABELS'; value = @('copperfin-isolation:schedule=serial') }) }
}
$checks = 0
foreach ($size in @(1, 2, 3, 10, 599, 600, 605)) {
    $tests = @(1..$size | ForEach-Object { Make-Test "test_$_" })
    $whole = Get-NativeTestShardPlan -Tests $tests
    Require ($whole.selected_names.Count -eq $size) 'Default covers the complete inventory.'
    ++$checks
    if ($size -ge 3) {
        $seen = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
        foreach ($shard in 1..3) {
            $plan = Get-NativeTestShardPlan -Tests $tests -ShardIndex $shard -ShardCount 3
            foreach ($name in $plan.selected_names) { Require ($seen.Add($name)) 'No shard overlap.' }
            $selected = @($tests | Where-Object { $plan.selected_names -ccontains $_.name })
            Assert-NativeTestShardSelection -FullTests $tests -SelectedTests $selected -Plan $plan
            ++$checks
        }
        Require ($seen.Count -eq $size) 'Full union, including uneven inventories.'
    }
}
$tests = @(1..6 | ForEach-Object { Make-Test "subject_$_" })
$plan = Get-NativeTestShardPlan -Tests $tests -ShardIndex 1 -ShardCount 3
Reject { Get-NativeTestShardPlan -Tests @() } 'Empty inventory rejected.'
Reject { Get-NativeTestShardPlan -Tests $tests -ShardIndex 2 -ShardCount 1 } 'Out-of-partition index rejected.'
Reject { Get-NativeTestShardPlan -Tests $tests -ShardCount 2 } 'Unbounded policy shape rejected.'
Reject { Get-NativeTestShardPlan -Tests @($tests[0], $tests[0], $tests[1]) } 'Duplicate names rejected.'
foreach ($property in @('DISABLED', 'FIXTURES_SETUP', 'FIXTURES_REQUIRED', 'FIXTURES_CLEANUP', 'DEPENDS')) {
    $invalid = Make-Test 'bad'
    $invalid.properties += [pscustomobject]@{ name = $property; value = $(if ($property -eq 'DISABLED') { $true } else { @('other') }) }
    Reject { Get-NativeTestShardPlan -Tests @($invalid) } "Unsafe property $property rejected."
}
$selected = @($tests[0], $tests[3])
Reject { Assert-NativeTestShardSelection $tests @($tests[0]) $plan } 'Omitted selected case rejected.'
Reject { Assert-NativeTestShardSelection $tests @($tests[0], $tests[0]) $plan } 'Duplicated selection rejected.'
$changed = Make-Test 'subject_4'; $changed.command = @('changed')
Reject { Assert-NativeTestShardSelection $tests @($tests[0], $changed) $plan } 'Changed command rejected.'
$changed = Make-Test 'subject_4'; $changed.properties[0].value = $false
Reject { Assert-NativeTestShardSelection $tests @($tests[0], $changed) $plan } 'Changed serial policy rejected.'
$goodXml = '<testsuite failures="0" skipped="0"><testcase name="subject_1" status="run"/><testcase name="subject_4" status="run"/></testsuite>'
Assert-NativeTestShardResults ([xml]$goodXml) $plan
foreach ($bad in @(
    $goodXml.Replace('subject_4', 'subject_1'),
    $goodXml.Replace('status="run"', 'status="notrun"'),
    $goodXml.Replace('failures="0"', 'failures="1"'),
    $goodXml.Replace('skipped="0"', 'skipped="1"'),
    $goodXml.Replace('<testcase name="subject_4" status="run"/>', ''),
    $goodXml.Replace('<testcase name="subject_4" status="run"/>', '<testcase name="subject_4" status="run"><error/></testcase>'))) {
    Reject { Assert-NativeTestShardResults ([xml]$bad) $plan } 'Bad execution report rejected.'
}
# Actual CTest selector/metadata/JUnit/exit behavior; no product test skipped.
New-Item -ItemType Directory -Force -Path (Join-Path $repo 'build') | Out-Null
$taskRoot = Join-Path $repo ("build/native-shard-test-" + [Guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $taskRoot | Out-Null
try {
    $cmake = (Get-Command cmake).Source.Replace('\', '/')
    $fixture = @(1..10 | ForEach-Object {
        'add_test(fixture_' + $_ + ' "' + $cmake + '" "-E" "true")'
        'set_tests_properties(fixture_' + $_ + ' PROPERTIES RUN_SERIAL TRUE LABELS "copperfin-isolation:schedule=serial")'
    })
    [IO.File]::WriteAllLines((Join-Path $taskRoot 'CTestTestfile.cmake'), $fixture)
    $runner = Join-Path $repo 'scripts/run-windows-native-tests.ps1'
    foreach ($shard in 1..3) {
        & $runner -BuildDirectory $taskRoot -ShardIndex $shard -ShardCount 3 -MetricsDirectory (Join-Path $taskRoot 'evidence')
    }
    if (-not [string]::IsNullOrWhiteSpace($EvidenceDirectory)) {
        if (Test-Path -LiteralPath $EvidenceDirectory) { throw 'Do not overwrite retained fixture evidence.' }
        Copy-Item -LiteralPath (Join-Path $taskRoot 'evidence') -Destination $EvidenceDirectory -Recurse
    }
    & $runner -BuildDirectory $taskRoot -MetricsDirectory (Join-Path $taskRoot 'whole')
    # Preserve native failing exit; all no-op positive cases cannot fake this.
    $fixture[0] = $fixture[0].Replace('"true"', '"false"')
    [IO.File]::WriteAllLines((Join-Path $taskRoot 'CTestTestfile.cmake'), $fixture)
    Reject { & $runner -BuildDirectory $taskRoot -ShardIndex 1 -ShardCount 3 -MetricsDirectory (Join-Path $taskRoot 'failure') } 'Actual failing CTest remains a failure.'
} finally {
    # Exact fresh test-owned GUID directory only; no repository/user cleanup.
    Remove-Item -LiteralPath $taskRoot -Recurse -Force
}
Write-Host "Native test sharding policy verified: $checks synthetic coverage checks, fail-closed mutations, actual three-shard/default CTest passes and actual failure propagation."
exit 0
