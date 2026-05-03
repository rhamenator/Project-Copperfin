param(
    [string]$Repository = $env:GITHUB_REPOSITORY,
    [string]$IssueNumbers,
    [string]$IssueJsonPath,
    [string]$HazardRegisterPath,
    [string]$ReportPath,
    [switch]$RequirePrimaryHazardCoverage = $true,
    [switch]$RequireClosedIssues = $true
)

$ErrorActionPreference = "Stop"

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
    $matches = [regex]::Matches($content, '\bHZ-[A-Za-z0-9-]+\b')
    $ids = @{}
    foreach ($match in $matches) {
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

    if ($labelSet.ContainsKey("safety") -or $labelSet.ContainsKey("documentation")) {
        return $true
    }

    if ($body -match '\bDQ-[A-Za-z0-9-]+\b') {
        return $true
    }

    return $false
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

    if ($RequireClosedIssues -and $state.ToLowerInvariant() -ne "closed") {
        $issueErrors += "Issue is not closed (state=$state)."
    }

    $dqMatches = [regex]::Matches($body, '\bDQ-[A-Za-z0-9-]+\b')
    $dvMatches = [regex]::Matches($body, '\bDV-[A-Za-z0-9-]+\b')
    $hzMatches = [regex]::Matches($body, '\bHZ-[A-Za-z0-9-]+\b')

    if ($dqMatches.Count -eq 0) {
        $issueErrors += "Missing DQ-* identifier(s)."
    }
    if ($dvMatches.Count -eq 0) {
        $issueErrors += "Missing DV-* identifier(s)."
    }
    if ($hzMatches.Count -eq 0 -and $upperBody -notmatch '\bHZ-NONE\b') {
        $issueErrors += "Missing HZ-* identifier(s) or explicit HZ-none rationale."
    }

    $issueHazards = @{}
    foreach ($match in $hzMatches) {
        $id = $match.Value.ToUpperInvariant()
        $issueHazards[$id] = $true
        if (-not $knownHazards.ContainsKey($id)) {
            $issueErrors += "Unknown hazard identifier: $id (not found in hazard register)."
        }
    }

    if ($RequirePrimaryHazardCoverage) {
        $hasPrimary = $false
        foreach ($primaryHazard in $primaryHazardIds) {
            if ($issueHazards.ContainsKey($primaryHazard)) {
                $hasPrimary = $true
                break
            }
        }

        if (-not $hasPrimary) {
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
