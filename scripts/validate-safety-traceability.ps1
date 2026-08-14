# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

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

function Get-ReviewComments {
    param(
        $Issue,
        $Headers
    )

    $comments = @()
    $commentCount = [int]$Issue.comments
    if ($commentCount -gt 0) {
        $pageCount = [int][Math]::Ceiling($commentCount / 100.0)
        for ($page = 1; $page -le $pageCount; $page++) {
            $commentsUri = "$($Issue.comments_url)?per_page=100&page=$page"
            $comments += @(Invoke-RestMethod -Uri $commentsUri -Headers $Headers -Method Get)
        }
    }

    return @($comments)
}

function Get-ReviewCommentFingerprint {
    param($Comments)

    $canonical = @($Comments | ForEach-Object {
        [pscustomobject]@{
            id = [string]$_.id
            updated_at = [string]$_.updated_at
            author = [string]$_.user.login
            body = [string]$_.body
        }
    } | Sort-Object -Property id)
    return Get-TextSha256 -Value ($canonical | ConvertTo-Json -Depth 4 -Compress)
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
        $stableResponse = $null
        for ($attempt = 1; $attempt -le 3; $attempt++) {
            $response = Invoke-RestMethod -Uri $uri -Headers $headers -Method Get
            $commentCount = [int]$response.comments
            $reviewComments = @(Get-ReviewComments -Issue $response -Headers $headers)

            # Bracket the second comment pass with issue reads. Issue metadata
            # alone does not reliably expose an in-flight comment edit, while a
            # comment pass alone cannot expose an in-flight issue-body edit.
            $latestResponse = Invoke-RestMethod -Uri $uri -Headers $headers -Method Get
            $latestReviewComments = @(Get-ReviewComments -Issue $latestResponse -Headers $headers)
            $finalResponse = Invoke-RestMethod -Uri $uri -Headers $headers -Method Get
            if ([string]$latestResponse.body -eq [string]$response.body -and
                [string]$finalResponse.body -eq [string]$latestResponse.body -and
                [int]$latestResponse.comments -eq $commentCount -and
                [int]$finalResponse.comments -eq [int]$latestResponse.comments -and
                [string]$latestResponse.updated_at -eq [string]$response.updated_at -and
                [string]$finalResponse.updated_at -eq [string]$latestResponse.updated_at -and
                (Get-ReviewCommentFingerprint -Comments $latestReviewComments) -eq
                    (Get-ReviewCommentFingerprint -Comments $reviewComments)) {
                $finalResponse | Add-Member -NotePropertyName review_comments -NotePropertyValue @($latestReviewComments) -Force
                $stableResponse = $finalResponse
                break
            }
        }

        if ($null -eq $stableResponse) {
            throw "Issue #$num changed while review evidence was being loaded; rerun validation against a stable snapshot."
        }
        $issues += $stableResponse
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
        "(?ims)^\s*#{2,3}\s*$escapedHeading\s*\r?\n(?<content>.*?)(?=^\s*#{2,3}\s|\z)")
    if (-not $match.Success) {
        return ""
    }

    return $match.Groups["content"].Value
}

function Get-SeverityClassification {
    param([string]$Body)

    $severityPattern = '(none|low|medium|high|catastrophic)'
    $section = Get-MarkdownSection -Body $Body -Heading "Potential Severity If Misused"
    $match = [regex]::Match($section, "(?im)^\s*(?:[-*]\s*)?$severityPattern\b[^\r\n]*$")
    if ($match.Success) {
        return $match.Groups[1].Value.ToLowerInvariant()
    }

    # Retain compatibility with evidence created before the issue form emitted
    # the severity as its own Markdown section.
    $match = [regex]::Match(
        $Body,
        "(?im)^\s*(?:potential\s+)?severity(?:\s+if\s+misused)?\s*:\s*$severityPattern\s*$")
    if ($match.Success) {
        return $match.Groups[1].Value.ToLowerInvariant()
    }

    return ""
}

function Get-EvidenceField {
    param(
        [string]$Text,
        [string]$Name
    )

    $escapedName = [regex]::Escape($Name)
    $match = [regex]::Match($Text, "(?im)^\s*$escapedName\s*:\s*(?<value>[^\r\n]*?)\s*$")
    if (-not $match.Success) {
        return ""
    }

    return $match.Groups["value"].Value.Trim()
}

function Get-GitHubLogin {
    param([string]$Value)

    $login = $Value.Trim().TrimStart('@')
    if ($login -notmatch '^(?=.{1,39}$)[A-Za-z0-9](?:[A-Za-z0-9-]*[A-Za-z0-9])?$') {
        return ""
    }

    if ($login -match '^(?i:me|self|author|maintainer|owner|reviewer|repository-?maintainer|second-qualified-reviewer)$') {
        return ""
    }

    return $login.ToLowerInvariant()
}

function Get-TextSha256 {
    param([AllowEmptyString()][string]$Value)

    $sha256 = [System.Security.Cryptography.SHA256]::Create()
    try {
        $bytes = [System.Text.Encoding]::UTF8.GetBytes($Value)
        return ([System.BitConverter]::ToString($sha256.ComputeHash($bytes))).Replace("-", "").ToLowerInvariant()
    } finally {
        $sha256.Dispose()
    }
}

function Test-MeaningfulReviewEvidence {
    param([string]$Value)

    $trimmed = $Value.Trim()
    if ($trimmed.Length -lt 12 -or $trimmed -match '^<[^>]+>$') {
        return $false
    }

    $normalized = [regex]::Replace($trimmed.ToLowerInvariant(), '[^a-z0-9]+', ' ').Trim()
    $rejectedTokens = @{
        "absent" = $true
        "blocked" = $true
        "deferred" = $true
        "incomplete" = $true
        "later" = $true
        "missing" = $true
        "none" = $true
        "pending" = $true
        "placeholder" = $true
        "skipped" = $true
        "tbd" = $true
        "todo" = $true
        "unavailable" = $true
        "unchecked" = $true
        "unqualified" = $true
        "unverified" = $true
        "unknown" = $true
    }
    foreach ($token in ($normalized -split '\s+')) {
        if ($rejectedTokens.ContainsKey($token)) {
            return $false
        }
    }

    $evidenceSubject = '(?:automation|check|checks|ci|pipeline|run|test|tests|verification|workflow)'
    $outcomeLink = '(?:(?:run|job|check|checks|status|conclusion|outcome|result|was|were|is|are)\s+){0,3}'
    $unsuccessfulOutcome = '(?:failed|failure|not\s+successful|unsuccessful)'
    if ($trimmed -match '^(?i:n\s*/?\s*a)$' -or
        $normalized -match "\b(?:failed\s+$evidenceSubject|$evidenceSubject(?:\s+[a-z0-9]+){0,3}\s+(?:did|does|do)\s+not\s+(?:pass|succeed)|$evidenceSubject\s+$outcomeLink$unsuccessfulOutcome|(?:conclusion|outcome|result|status)\s+$outcomeLink$unsuccessfulOutcome)\b" -or
        $normalized -match '^(?:no|not|never|without)\s+(?:applicable|automation|available|check|checked|completed|done|evidence|provided|qualification|qualified|review|run|test|verification|verified)\b' -or
        $normalized -match '\b(?:not|un)(?:available|checked|qualified|verified)\b') {
        return $false
    }

    return $true
}

function Test-AuthenticatedIndependentReview {
    param(
        $Issue,
        [string]$Reviewer
    )

    if ([string]::IsNullOrWhiteSpace($Reviewer)) {
        return $false
    }

    $issueBodySha256 = Get-TextSha256 -Value ([string]$Issue.body)

    foreach ($comment in @($Issue.review_comments)) {
        $commentAuthor = Get-GitHubLogin -Value ([string]$comment.user.login)
        $commentAuthorType = ([string]$comment.user.type).ToLowerInvariant()
        if ($commentAuthor -ne $Reviewer -or $commentAuthorType -ne "user") {
            continue
        }

        $signOff = Get-MarkdownSection -Body ([string]$comment.body) -Heading "Independent Review Sign-Off"
        $signOffReviewer = Get-GitHubLogin -Value (Get-EvidenceField -Text $signOff -Name "reviewer")
        $signOffQualification = Get-EvidenceField -Text $signOff -Name "qualification"
        $signOffVerification = Get-EvidenceField -Text $signOff -Name "verification"
        $signOffIssueBodySha256 = (Get-EvidenceField -Text $signOff -Name "reviewed issue body sha256").ToLowerInvariant()
        $signOffResult = (Get-EvidenceField -Text $signOff -Name "result").ToLowerInvariant()
        if ([string]::IsNullOrWhiteSpace($signOffResult)) {
            $signOffResult = (Get-EvidenceField -Text $signOff -Name "status").ToLowerInvariant()
        }

        if ($signOffReviewer -eq $commentAuthor -and
            (Test-MeaningfulReviewEvidence -Value $signOffQualification) -and
            (Test-MeaningfulReviewEvidence -Value $signOffVerification) -and
            $signOffIssueBodySha256 -match '^[a-f0-9]{64}$' -and
            $signOffIssueBodySha256 -eq $issueBodySha256 -and
            $signOffResult -match '^(?:approved|passed)$') {
            return $true
        }
    }

    return $false
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
    $severity = Get-SeverityClassification -Body $body
    $authorLogin = Get-GitHubLogin -Value ([string]$issue.user.login)
    $reviewEvidence = Get-MarkdownSection -Body $body -Heading "Review Evidence"
    $legacyIndependentReviewEvidence = Get-MarkdownSection -Body $body -Heading "Independent Review Evidence"
    $hasCurrentReviewEvidence = -not [string]::IsNullOrWhiteSpace($reviewEvidence)
    $hasLegacyIndependentReviewEvidence = -not [string]::IsNullOrWhiteSpace($legacyIndependentReviewEvidence)
    $hasReviewEvidence = $hasCurrentReviewEvidence -or $hasLegacyIndependentReviewEvidence

    $currentMode = (Get-EvidenceField -Text $reviewEvidence -Name "mode").ToLowerInvariant()
    $currentReviewer = Get-GitHubLogin -Value (Get-EvidenceField -Text $reviewEvidence -Name "reviewer")
    $currentVerification = Get-EvidenceField -Text $reviewEvidence -Name "verification"
    $currentAutomation = Get-EvidenceField -Text $reviewEvidence -Name "automated evidence"
    $currentResult = (Get-EvidenceField -Text $reviewEvidence -Name "result").ToLowerInvariant()
    if ([string]::IsNullOrWhiteSpace($currentResult)) {
        $currentResult = (Get-EvidenceField -Text $reviewEvidence -Name "status").ToLowerInvariant()
    }
    $currentApproved = $currentResult -match '^(?:approved|passed)$'
    $currentSelfReview = $currentMode -eq "maintainer self-review"
    $currentIndependentReview = $currentMode -eq "independent human review"
    $hasStructuredCurrentReview = $hasCurrentReviewEvidence -and
        -not [string]::IsNullOrWhiteSpace($authorLogin) -and
        -not [string]::IsNullOrWhiteSpace($currentReviewer) -and
        (Test-MeaningfulReviewEvidence -Value $currentVerification) -and
        $currentApproved
    $hasStructuredCurrentSelfReview = $hasStructuredCurrentReview -and
        $currentSelfReview -and $currentReviewer -eq $authorLogin -and
        (Test-MeaningfulReviewEvidence -Value $currentAutomation)
    $hasStructuredCurrentIndependentReview = $hasStructuredCurrentReview -and
        $currentIndependentReview -and $currentReviewer -ne $authorLogin
    $hasAuthenticatedCurrentIndependentReview = $hasStructuredCurrentIndependentReview -and
        (Test-AuthenticatedIndependentReview -Issue $issue -Reviewer $currentReviewer)
    $hasValidCurrentReview = $hasStructuredCurrentSelfReview -or
        $hasAuthenticatedCurrentIndependentReview

    $legacyReviewer = Get-GitHubLogin -Value (Get-EvidenceField -Text $legacyIndependentReviewEvidence -Name "reviewer")
    $legacyVerification = Get-EvidenceField -Text $legacyIndependentReviewEvidence -Name "verification"
    $legacyResult = (Get-EvidenceField -Text $legacyIndependentReviewEvidence -Name "result").ToLowerInvariant()
    if ([string]::IsNullOrWhiteSpace($legacyResult)) {
        $legacyResult = (Get-EvidenceField -Text $legacyIndependentReviewEvidence -Name "status").ToLowerInvariant()
    }
    $hasStructuredLegacyIndependentReview = $hasLegacyIndependentReviewEvidence -and
        -not [string]::IsNullOrWhiteSpace($authorLogin) -and
        -not [string]::IsNullOrWhiteSpace($legacyReviewer) -and
        $legacyReviewer -ne $authorLogin -and
        (Test-MeaningfulReviewEvidence -Value $legacyVerification) -and
        $legacyResult -match '^(?:approved|passed)$'
    $hasAuthenticatedLegacyIndependentReview = $hasStructuredLegacyIndependentReview -and
        (Test-AuthenticatedIndependentReview -Issue $issue -Reviewer $legacyReviewer)
    $hasValidLegacyIndependentReview = $hasAuthenticatedLegacyIndependentReview

    $hasValidReviewEvidence = $hasValidCurrentReview -or $hasValidLegacyIndependentReview
    $hasApprovedIndependentReview =
        $hasAuthenticatedCurrentIndependentReview -or
        $hasValidLegacyIndependentReview
    $hasUnauthenticatedIndependentClaim =
        ($hasStructuredCurrentIndependentReview -and -not $hasAuthenticatedCurrentIndependentReview) -or
        ($hasStructuredLegacyIndependentReview -and -not $hasAuthenticatedLegacyIndependentReview)

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
            if ($id -eq "HZ-NONE") {
                if (-not $hasExplicitNoHazard -or $row.HzIds.Count -ne 1) {
                    $issueErrors += "Mapping row references HZ-NONE outside the sole explicit no-hazard path: $($row.Raw)"
                }
                continue
            }
            if (-not $issueHazards.ContainsKey($id)) {
                $issueErrors += "Mapping row references undeclared HZ identifier: $id"
            }
            if (-not $knownHazards.ContainsKey($id)) {
                $issueErrors += "Mapping row references unknown hazard identifier: $id (not found in hazard register)."
            }
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
    if (-not $hasReviewEvidence) {
        $issueErrors += "Missing Review Evidence."
    } elseif ($hasUnauthenticatedIndependentClaim) {
        $issueErrors += "Independent Human Review requires an approved structured sign-off comment authored by the named distinct reviewer."
    } elseif (-not $hasValidReviewEvidence) {
        $issueErrors += "Review Evidence must record an approved structured mode, issue-author or distinct reviewer identity as applicable, verification, and automated evidence for maintainer self-review."
    } elseif (($severity -eq "high" -or $severity -eq "catastrophic") -and -not $hasApprovedIndependentReview) {
        $issueErrors += "Severity '$severity' requires approved Independent Human Review evidence from a second qualified reviewer."
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
    if ([string]::IsNullOrWhiteSpace($severity)) {
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
