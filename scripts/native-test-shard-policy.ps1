# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# RQ-CF-CI-WINDOWS-NATIVE-BUDGET-001 / VR-CI-7181-001.
# Pure full-inventory partition/selection checks; never change test properties.
Set-StrictMode -Version Latest

function Get-NativeTestShardPlan {
    param(
        [Parameter(Mandatory)][AllowEmptyCollection()][object[]]$Tests,
        [ValidateRange(1, 3)][int]$ShardIndex = 1,
        [ValidateSet(1, 3)][int]$ShardCount = 1
    )
    if ($ShardIndex -gt $ShardCount -or $Tests.Count -lt $ShardCount) {
        throw 'A shard must be within the bounded count and contain tests.'
    }
    $names = [Collections.Generic.List[string]]::new()
    $unique = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    foreach ($test in $Tests) {
        $nameProperty = $test.PSObject.Properties['name']
        if ($null -eq $nameProperty -or $nameProperty.Value -isnot [string] -or
            [string]::IsNullOrWhiteSpace($nameProperty.Value) -or -not $unique.Add($nameProperty.Value)) {
            throw 'The complete CTest inventory needs nonempty unique names.'
        }
        $properties = $test.PSObject.Properties['properties']
        if ($null -eq $properties) { throw 'CTest property metadata is required.' }
        foreach ($property in $properties.Value) {
            if ($property.name -eq 'DISABLED' -and $property.value -eq $true) {
                throw 'Disabled tests cannot be counted as full execution evidence.'
            }
            if ($property.name -in @('FIXTURES_SETUP', 'FIXTURES_REQUIRED', 'FIXTURES_CLEANUP', 'DEPENDS') -and
                @($property.value).Count -gt 0 -and -not [string]::IsNullOrWhiteSpace([string]$property.value)) {
                throw 'Cross-test fixture/dependency semantics require a separately reviewed partition.'
            }
        }
        $names.Add($nameProperty.Value)
    }
    $partitions = @()
    for ($shard = 1; $shard -le $ShardCount; ++$shard) {
        $members = [Collections.Generic.List[string]]::new()
        for ($index = $shard - 1; $index -lt $names.Count; $index += $ShardCount) {
            $members.Add($names[$index])
        }
        $partitions += ,$members.ToArray()
    }
    $covered = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    foreach ($partition in $partitions) {
        foreach ($name in $partition) {
            if (-not $covered.Add($name)) { throw 'Shard overlap is not permitted.' }
        }
    }
    if ($covered.Count -ne $names.Count) { throw 'Shard coverage is incomplete.' }
    return [pscustomobject]@{
        schema_version = 1
        full_count = $names.Count
        shard_index = $ShardIndex
        shard_count = $ShardCount
        full_names = $names.ToArray()
        selected_names = $partitions[$ShardIndex - 1]
        partitions = $partitions
        ctest_selector = "$ShardIndex,,$ShardCount"
    }
}

function Assert-NativeTestShardSelection {
    param([object[]]$FullTests, [object[]]$SelectedTests, [object]$Plan)
    $actualNames = @($SelectedTests | ForEach-Object { $_.name })
    if (($actualNames | ConvertTo-Json -Compress) -cne ($Plan.selected_names | ConvertTo-Json -Compress)) {
        throw 'Actual CTest selection differs from the complete-inventory partition.'
    }
    $fullByName = [Collections.Generic.Dictionary[string, object]]::new([StringComparer]::Ordinal)
    foreach ($test in $FullTests) { $fullByName[$test.name] = $test }
    foreach ($test in $SelectedTests) {
        $original = $fullByName[$test.name]
        $actualCommand = $test.PSObject.Properties['command']
        $originalCommand = $original.PSObject.Properties['command']
        if (($null -eq $actualCommand) -ne ($null -eq $originalCommand) -or
            ($null -ne $actualCommand -and
             (ConvertTo-Json -InputObject $actualCommand.Value -Compress) -cne
             (ConvertTo-Json -InputObject $originalCommand.Value -Compress))) {
            throw 'A selected test command changed.'
        }
        if ((ConvertTo-Json -InputObject @($test.properties) -Depth 100 -Compress) -cne
            (ConvertTo-Json -InputObject @($original.properties) -Depth 100 -Compress)) {
            throw 'A selected test property changed, including its isolation/serial policy.'
        }
    }
}

function Assert-NativeTestShardResults {
    param([xml]$Results, [object]$Plan)
    $cases = @($Results.testsuite.testcase)
    if ($cases.Count -ne $Plan.selected_names.Count -or
        [int]$Results.testsuite.failures -ne 0 -or [int]$Results.testsuite.skipped -ne 0) {
        throw 'Every selected test must complete successfully; missing/skipped/failed cases are not evidence.'
    }
    $actual = @($cases | ForEach-Object { [string]$_.name } | Sort-Object -CaseSensitive)
    $expected = @($Plan.selected_names | Sort-Object -CaseSensitive)
    if (($actual | ConvertTo-Json -Compress) -cne ($expected | ConvertTo-Json -Compress)) {
        throw 'The complete selected name multiset must match actual execution.'
    }
    foreach ($case in $cases) {
        if ($case.status -ne 'run' -or $null -ne $case.SelectSingleNode('failure|error|skipped')) {
            throw 'A selected test did not complete successfully.'
        }
    }
}
