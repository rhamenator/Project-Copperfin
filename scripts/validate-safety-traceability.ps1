# Copyright © 2026 Richard M. Hamilton. All rights reserved.
# Licensed under the Project Copperfin Source-Available License or
# Commercial License. See LICENSE.md in the repository root.

param(
    [string]$Repository = $env:GITHUB_REPOSITORY,
    [string]$IssueNumbers,
    [string]$IssueJsonPath,
    [string]$HazardRegisterPath,
    [string]$ReportPath,
    [string]$RequirePrimaryHazardCoverage,
    [string]$RequireClosedIssues
)

$ErrorActionPreference = "Stop"

$enforcePrimaryHazardCoverage = if ($PSBoundParameters.ContainsKey('RequirePrimaryHazardCoverage')) {
    [System.Convert]::ToBoolean($RequirePrimaryHazardCoverage)
} else {
    $true
}

$enforceClosedIssues = if ($PSBoundParameters.ContainsKey('RequireClosedIssues')) {
    [System.Convert]::ToBoolean($RequireClosedIssues)
} else {
    $true
}

function Write-Fail {
    param([string]$Message)
    Write-Host "ERROR: $Message" -ForegroundColor Red
}

function Write-Info {
    param([string]$Message)
    Write-Host $Message -ForegroundColor Cyan
}

function Get-HazardIdsFromRegister {
    param([string]$Path)

    if (-not (Test-Path $Path)) {
        throw "Hazard register not found: $Path"
    }

    $content = Get-Content -Path $Path -Raw
    $hazardIdMatches = [regex]::Matches($content, '\bHZ-[A-Za-z0-9-]+\b')
    $ids = @{}
    foreach ($match in $hazardIdMatches) {
        $ids[$match.Value.ToUpperInvariant()] = $true
    }

    return @($ids.Keys | Sort-Object)
}

function Get-IssuesFromJson {
    param([string]$Path)

    if (-not (Test-Path $Path)) {
        throw "Issue JSON file not found: $Path"
    }

    $raw = Get-Content -Path $Path -Raw
    $parsed = $raw | ConvertFrom-Json

    if ($parsed -is [System.Array]) {
        return $parsed
    }

    if ($null -ne $parsed.items -and $parsed.items -is [System.Array]) {
        return $parsed.items
    }

    return @($parsed)
}

function Get-IssuesFromGitHub {
    param(
        [string]$Repo,
        [string]$Numbers
    )

    if ([string]::IsNullOrWhiteSpace($Repo)) {
        throw "Repository is required when loading from GitHub API (format: owner/repo)."
    }

    if ([string]::IsNullOrWhiteSpace($Numbers)) {
        throw "IssueNumbers is required when loading from GitHub API."
    }

    if ([string]::IsNullOrWhiteSpace($env:GITHUB_TOKEN)) {
        throw "GITHUB_TOKEN is required to fetch issue bodies from GitHub API."
    }

    $headers = @{
        Authorization = "Bearer $($env:GITHUB_TOKEN)"
        Accept = "application/vnd.github+json"
        "X-GitHub-Api-Version" = "2022-11-28"
    }

    $issues = @()
    $numberList = $Numbers -split '[,\s]+' | Where-Object { -not [string]::IsNullOrWhiteSpace($_) }

    foreach ($num in $numberList) {
        if ($num -notmatch '^\d+$') {
            throw "Issue number '$num' is not numeric."
        }
    }

    foreach ($num in $numberList) {
        $uri = "https://api.github.com/repos/$Repo/issues/$num"
        $response = Invoke-RestMethod -Uri $uri -Headers $headers -Method Get
        $issues += $response
    }

    return $issues
}

function Get-IssueLabelNames {
    param($Issue)

    $labels = @()
    if ($null -ne $Issue.labels) {
        foreach ($label in $Issue.labels) {
            if ($label -is [string]) {
                $labels += $label
            } elseif ($null -ne $label.name) {
                $labels += [string]$label.name
            }
        }
    }

    return $labels
}

function Test-SafetyDocIssue {
    param($Issue)

    $title = [string]$Issue.title
    $body = [string]$Issue.body
    $labels = Get-IssueLabelNames -Issue $Issue

    if ($title.ToUpperInvariant().Contains("[DOCS-SAFETY]")) {
        return $true
    }

    $labelSet = @{}
    foreach ($label in $labels) {
        $labelSet[$label.ToLowerInvariant()] = $true
    }

    # The broad safety label also marks implementation and test lanes. Only
    # the dedicated documentation signal should enter this evidence validator.
    if ($labelSet.ContainsKey("documentation")) {
        return $true
    }

    if ($body -match '\bDQ-[A-Za-z0-9-]+\b') {
        return $true
    }

    return $false
}

function Get-MarkdownSection {
    param(
        [string]$Body,
        [string]$Heading
    )

    $escapedHeading = [regex]::Escape($Heading)
    $match = [regex]::Match(
        $Body,
        "(?ims)^\s*##\s*$escapedHeading\s*\r?\n(?<content>.*?)(?=^\s*##\s|\z)")
    if (-not $match.Success) {
        return ""
    }

    return $match.Groups["content"].Value
}

function Get-TraceabilityIds {
    param(
        [AllowEmptyString()]
        [string]$Text,
        [Parameter(Mandatory = $true)]
        [ValidateSet("DQ", "DV", "HZ")]
        [string]$Prefix
    )

    $ids = @{}
    foreach ($match in [regex]::Matches($Text, "\b$Prefix-[A-Za-z0-9-]+\b")) {
        $ids[$match.Value.ToUpperInvariant()] = $true
    }

    return @($ids.Keys | Sort-Object)
}

function Get-TraceabilityMappingRows {
    param([string]$Body)

    $section = Get-MarkdownSection -Body $Body -Heading "DQ/DV/HZ Mapping"
    if ([string]::IsNullOrWhiteSpace($section)) {
        return @()
    }

    $rows = @()
    foreach ($line in ($section -split "\r?\n")) {
        $trimmed = $line.Trim()
        if ($trimmed -notmatch '^\|') {
            continue
        }

        $cells = @($trimmed.Trim('|') -split '\|') | ForEach-Object { $_.Trim() }
        if ($cells.Count -eq 3 -and $cells[0] -eq "Documentation requirement") {
            continue
        }
        if ($cells.Count -eq 3 -and ($cells -join "") -match '^[-:]+$') {
            continue
        }

        $rows += [pscustomobject]@{
            Raw = $trimmed
            Malformed = $cells.Count -ne 3
            DqIds = if ($cells.Count -ge 1) { @(Get-TraceabilityIds -Text $cells[0] -Prefix DQ) } else { @() }
            DvIds = if ($cells.Count -ge 2) { @(Get-TraceabilityIds -Text $cells[1] -Prefix DV) } else { @() }
            HzIds = if ($cells.Count -ge 3) { @(Get-TraceabilityIds -Text $cells[2] -Prefix HZ) } else { @() }
        }
    }

    return $rows
}

$repoRoot = Split-Path -Parent $PSScriptRoot
if ([string]::IsNullOrWhiteSpace($HazardRegisterPath)) {
    $HazardRegisterPath = Join-Path $repoRoot "docs/safety/hazard-register.md"
}

$hazardIds = Get-HazardIdsFromRegister -Path $HazardRegisterPath
if ($hazardIds.Count -eq 0) {
    throw "No HZ-* identifiers found in hazard register: $HazardRegisterPath"
}

$primaryHazardIds = @(
    "HZ-DATA-CORRUPTION-01",
    "HZ-SYSTEM-FAILURE-01",
    "HZ-RUNTIME-CRASH-01"
)

$issues = @()
if (-not [string]::IsNullOrWhiteSpace($IssueJsonPath)) {
    Write-Info "Loading issues from JSON: $IssueJsonPath"
    $issues = Get-IssuesFromJson -Path $IssueJsonPath
} else {
    Write-Info "Loading issues from GitHub API: $Repository ($IssueNumbers)"
    $issues = Get-IssuesFromGitHub -Repo $Repository -Numbers $IssueNumbers
}

if ($issues.Count -eq 0) {
    throw "No issues found for validation."
}

$knownHazards = @{}
foreach ($hazard in $hazardIds) {
    $knownHazards[$hazard] = $true
}

$results = @()
$errors = @()
$validatedCount = 0

foreach ($issue in $issues) {
    $number = [string]$issue.number
    $title = [string]$issue.title
    $state = [string]$issue.state
    $body = [string]$issue.body

    if (-not (Test-SafetyDocIssue -Issue $issue)) {
        $results += [pscustomobject]@{
            Number = $number
            Title = $title
            SafetyDocIssue = $false
            Status = "skipped"
            Notes = "Issue does not appear to be a safety-documentation traceability item."
        }
        continue
    }

    $validatedCount++
    $issueErrors = @()
    $upperBody = $body.ToUpperInvariant()

    if ($enforceClosedIssues -and $state.ToLowerInvariant() -ne "closed") {
        $issueErrors += "Issue is not closed (state=$state)."
    }

    $dqMatches = [regex]::Matches($body, '\bDQ-[A-Za-z0-9-]+\b')
    $dvMatches = [regex]::Matches($body, '\bDV-[A-Za-z0-9-]+\b')

    $declaredDqIds = @(Get-TraceabilityIds -Text (Get-MarkdownSection -Body $body -Heading "Documentation Requirement IDs") -Prefix DQ)
    $declaredDvIds = @(Get-TraceabilityIds -Text (Get-MarkdownSection -Body $body -Heading "Documentation Verification IDs") -Prefix DV)
    $declaredHzIds = @(Get-TraceabilityIds -Text (Get-MarkdownSection -Body $body -Heading "Hazard Linkage IDs") -Prefix HZ)
    $hasNoHazardId = $declaredHzIds -contains "HZ-NONE"
    $hasExplicitNoHazard = $declaredHzIds.Count -eq 1 -and $hasNoHazardId
    $mappingSection = Get-MarkdownSection -Body $body -Heading "DQ/DV/HZ Mapping"
    $mappingRows = @(Get-TraceabilityMappingRows -Body $body)

    if ($dqMatches.Count -eq 0) {
        $issueErrors += "Missing DQ-* identifier(s)."
    }
    if ($dvMatches.Count -eq 0) {
        $issueErrors += "Missing DV-* identifier(s)."
    }
    if ($declaredHzIds.Count -eq 0) {
        $issueErrors += "Missing Hazard Linkage IDs section."
    } elseif ($hasNoHazardId -and -not $hasExplicitNoHazard) {
        $issueErrors += "HZ-NONE cannot be combined with other hazard identifiers."
    }

    if ($declaredDqIds.Count -eq 0) {
        $issueErrors += "Missing Documentation Requirement IDs section."
    }
    if ($declaredDvIds.Count -eq 0) {
        $issueErrors += "Missing Documentation Verification IDs section."
    }
    if ([string]::IsNullOrWhiteSpace($mappingSection)) {
        $issueErrors += "Missing DQ/DV/HZ Mapping section."
    } elseif ($mappingRows.Count -eq 0) {
        $issueErrors += "DQ/DV/HZ Mapping section contains no data rows."
    }

    $issueHazards = @{}
    foreach ($id in $declaredHzIds) {
        $issueHazards[$id] = $true
        if ($id -ne "HZ-NONE" -and -not $knownHazards.ContainsKey($id)) {
            $issueErrors += "Unknown hazard identifier: $id (not found in hazard register)."
        }
    }

    $mappedDqIds = @{}
    $mappedDvIds = @{}
    $mappedHzIds = @{}
    foreach ($row in $mappingRows) {
        if ($row.Malformed) {
            $issueErrors += "Malformed DQ/DV/HZ mapping row: $($row.Raw)"
            continue
        }
        if ($row.DqIds.Count -eq 0 -or $row.DvIds.Count -eq 0 -or $row.HzIds.Count -eq 0) {
            $issueErrors += "Mapping row must contain at least one DQ, DV, and HZ identifier: $($row.Raw)"
            continue
        }

        foreach ($id in $row.DqIds) {
            $mappedDqIds[$id] = $true
            if ($declaredDqIds -notcontains $id) {
                $issueErrors += "Mapping row references undeclared DQ identifier: $id"
            }
        }
        foreach ($id in $row.DvIds) {
            $mappedDvIds[$id] = $true
            if ($declaredDvIds -notcontains $id) {
                $issueErrors += "Mapping row references undeclared DV identifier: $id"
            }
        }
        foreach ($id in $row.HzIds) {
            $mappedHzIds[$id] = $true
        }
    }

    foreach ($id in $declaredDqIds) {
        if (-not $mappedDqIds.ContainsKey($id)) {
            $issueErrors += "Declared DQ identifier is not mapped: $id"
        }
    }
    foreach ($id in $declaredDvIds) {
        if (-not $mappedDvIds.ContainsKey($id)) {
            $issueErrors += "Declared DV identifier is not mapped: $id"
        }
    }
    if (-not $hasExplicitNoHazard) {
        foreach ($id in $issueHazards.Keys) {
            if (-not $mappedHzIds.ContainsKey($id)) {
                $issueErrors += "Declared HZ identifier is not mapped: $id"
            }
        }
    }

    if ($enforcePrimaryHazardCoverage) {
        $hasPrimary = $false
        foreach ($primaryHazard in $primaryHazardIds) {
            if ($issueHazards.ContainsKey($primaryHazard)) {
                $hasPrimary = $true
                break
            }
        }

        if (-not $hasPrimary -and -not $hasExplicitNoHazard) {
            $issueErrors += "No primary hazard linked. Expected at least one of: $($primaryHazardIds -join ', ')."
        }
    }

    if ($upperBody -notmatch 'PROCEDURAL\s+DELTA\s+MAP') {
        $issueErrors += "Missing Procedural Delta Map section."
    }
    if ($upperBody -notmatch 'MISUSE\s+ANALYSIS') {
        $issueErrors += "Missing Misuse Analysis section."
    }
    if ($upperBody -notmatch 'INDEPENDENT\s+REVIEW') {
        $issueErrors += "Missing Independent Review evidence."
    }
    if ($upperBody -notmatch 'SIMULATION|WALKTHROUGH') {
        $issueErrors += "Missing Simulation/Walkthrough evidence."
    }
    if ($upperBody -notmatch 'ROLLBACK') {
        $issueErrors += "Missing rollback plan detail."
    }
    if ($upperBody -notmatch 'FIELD\s+NOTIFICATION|NOTIFICATION\s+PLAN') {
        $issueErrors += "Missing field-notification plan detail."
    }
    if ($upperBody -notmatch 'NONE|LOW|MEDIUM|HIGH|CATASTROPHIC') {
        $issueErrors += "Missing severity classification."
    }

    if ($issueErrors.Count -gt 0) {
        $errors += [pscustomobject]@{
            Number = $number
            Title = $title
            Errors = $issueErrors
        }
        $results += [pscustomobject]@{
            Number = $number
            Title = $title
            SafetyDocIssue = $true
            Status = "failed"
            Notes = ($issueErrors -join " | ")
        }
    } else {
        $results += [pscustomobject]@{
            Number = $number
            Title = $title
            SafetyDocIssue = $true
            Status = "passed"
            Notes = "All required documentation safety traceability checks passed."
        }
    }
}

Write-Host ""
Write-Host "Safety traceability validation summary:" -ForegroundColor Cyan
$results | Format-Table -AutoSize

if (-not [string]::IsNullOrWhiteSpace($ReportPath)) {
    $report = [pscustomobject]@{
        validatedAt = (Get-Date).ToString("o")
        repository = $Repository
        issueNumbers = $IssueNumbers
        hazardRegisterPath = $HazardRegisterPath
        validatedIssueCount = $validatedCount
        resultCount = $results.Count
        failedIssueCount = $errors.Count
        results = $results
        errors = $errors
    }

    $reportDir = Split-Path -Parent $ReportPath
    if (-not [string]::IsNullOrWhiteSpace($reportDir) -and -not (Test-Path $reportDir)) {
        New-Item -ItemType Directory -Path $reportDir -Force | Out-Null
    }

    $report | ConvertTo-Json -Depth 8 | Set-Content -Path $ReportPath
    Write-Info "Wrote validation report: $ReportPath"
}

if ($validatedCount -eq 0) {
    throw "No safety-documentation issues were validated. Provide safety/documentation issues or include DQ-* fields."
}

if ($errors.Count -gt 0) {
    foreach ($entry in $errors) {
        Write-Fail "Issue #$($entry.Number) $($entry.Title)"
        foreach ($err in $entry.Errors) {
            Write-Fail "  - $err"
        }
    }
    throw "Safety traceability validation failed."
}

Write-Host ""
Write-Host "Safety traceability validation passed." -ForegroundColor Green
