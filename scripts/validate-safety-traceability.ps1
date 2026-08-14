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

function Test-MarkdownLinkDestination {
    param([AllowEmptyString()][string]$Value)

    $candidate = $Value.Trim()
    $targetMatch = [regex]::Match(
        $candidate,
        '^(?<destination><(?:\\.|[^<>\\\r\n])*>|\S+)(?:[ \t]+(?<title>"(?:\\.|[^"\\\r\n])*"|''(?:\\.|[^''\\\r\n])*''|\((?:\\.|[^()\\\r\n])*\)))?$')
    if (-not $targetMatch.Success) {
        return $false
    }
    $destination = $targetMatch.Groups['destination'].Value
    if ($destination -match '^<(?:\\.|[^<>\\\r\n])*>$') {
        return $true
    }
    if ([string]::IsNullOrWhiteSpace($destination)) {
        return $false
    }

    $depth = 0
    for ($index = 0; $index -lt $destination.Length; $index++) {
        if ($destination[$index] -eq '\') {
            $index++
            continue
        }
        $characterCode = [int][char]$destination[$index]
        if ($characterCode -le 0x20 -or $characterCode -eq 0x7F -or
            $destination[$index] -eq '<' -or $destination[$index] -eq '>') {
            return $false
        }
        if ($destination[$index] -eq '(') {
            $depth++
        } elseif ($destination[$index] -eq ')') {
            $depth--
            if ($depth -lt 0) { return $false }
        }
    }
    return $depth -eq 0
}

function Get-RenderedInlineEvidenceText {
    param(
        [AllowEmptyString()][string]$Value,
        [switch]$NormalizeReferenceLinks,
        [AllowEmptyString()][string]$ReferenceText = '',
        [switch]$RequireDefinedReferences
    )

    $rendered = $Value
    $linkTextPattern = '(?:(?>[^\[\]\\\r\n]+)|\\.|(?<depth>\[)|(?<-depth>\]))*(?(depth)(?!))'
    $angleDestinationPattern = '<(?:\\.|[^<>\\\r\n])*>'
    $bareDestinationPattern = '(?:(?>[^\s()<>\\\r\n]+)|\\.|(?<paren>\()|(?<-paren>\)))*(?(paren)(?!))'
    $linkTitlePattern = '(?:"(?:\\.|[^"\\\r\n])*"|''(?:\\.|[^''\\\r\n])*''|\((?:\\.|[^()\\\r\n])*\))'
    $inlineTargetPattern = "(?:$angleDestinationPattern|$bareDestinationPattern)(?:[ \t]+$linkTitlePattern)?"
    $inlineLinkPattern = "!?\[(?<text>$linkTextPattern)\]\((?<destination>$inlineTargetPattern)\)"
    $rendered = [regex]::Replace(
        $rendered,
        $inlineLinkPattern,
        {
            param($match)
            if (Test-MarkdownLinkDestination -Value $match.Groups['destination'].Value) {
                return $match.Groups['text'].Value
            }
            return $match.Value
        })
    if ($NormalizeReferenceLinks) {
        $referenceLabels = @{}
        if (-not [string]::IsNullOrWhiteSpace($ReferenceText)) {
            foreach ($definition in [regex]::Matches(
                $ReferenceText,
                "(?m)^ {0,3}\[(?<label>(?:\\.|[^\]\\\r\n])+)\]:[ \t]*(?<destination>$inlineTargetPattern)[ \t]*$")) {
                if (-not (Test-MarkdownLinkDestination `
                        -Value $definition.Groups['destination'].Value)) {
                    continue
                }
                $normalizedLabel = [regex]::Replace(
                    $definition.Groups['label'].Value.ToLowerInvariant(),
                    '[ \t\r\n]+',
                    ' ').Trim()
                $referenceLabels[$normalizedLabel] = $true
            }
        }
        $rendered = [regex]::Replace(
            $rendered,
            "!?\[(?<text>$linkTextPattern)\]\[(?<label>$linkTextPattern)\]",
            {
                param($match)
                $label = $match.Groups['label'].Value
                if ([string]::IsNullOrEmpty($label)) {
                    $label = $match.Groups['text'].Value
                }
                $normalizedLabel = [regex]::Replace(
                    $label.ToLowerInvariant(),
                    '[ \t\r\n]+',
                    ' ').Trim()
                if ((-not $RequireDefinedReferences -and $referenceLabels.Count -eq 0) -or
                    $referenceLabels.ContainsKey($normalizedLabel)) {
                    return $match.Groups['text'].Value
                }
                return $match.Value
            })
        $rendered = [regex]::Replace(
            $rendered,
            "!?\[(?<text>$linkTextPattern)\]",
            {
                param($match)
                $normalizedLabel = [regex]::Replace(
                    $match.Groups['text'].Value.ToLowerInvariant(),
                    '[ \t\r\n]+',
                    ' ').Trim()
                if ((-not $RequireDefinedReferences -and $referenceLabels.Count -eq 0) -or
                    $referenceLabels.ContainsKey($normalizedLabel)) {
                    return $match.Groups['text'].Value
                }
                return $match.Value
            })
    }
    foreach ($pattern in @(
        '(?<!\*)\*\*(?=\S)(?<text>.*?\S)\*\*(?!\*)',
        '(?<!_)__(?=\S)(?<text>.*?\S)__(?!_)',
        '(?<!~)~~(?=\S)(?<text>.*?\S)~~(?!~)',
        '(?<!\*)\*(?=\S)(?<text>.*?\S)\*(?!\*)',
        '(?<!_)_(?=\S)(?<text>.*?\S)_(?!_)')) {
        $rendered = [regex]::Replace($rendered, $pattern, '${text}')
    }
    $rendered = [regex]::Replace(
        $rendered,
        '\\(?<escaped>[!"#$%&''()*+,\-./:;<=>?@\[\\\]^_`{|}~])',
        '${escaped}')
    return [System.Net.WebUtility]::HtmlDecode($rendered)
}

function Get-MarkdownSection {
    param(
        [string]$Body,
        [string]$Heading
    )

    $lines = @($Body -split '\r?\n')
    $targetIndexes = @()
    for ($index = 0; $index -lt $lines.Count; $index++) {
        $headingMatch = [regex]::Match(
            $lines[$index],
            '^ {0,3}#{2,3}[ \t]+(?<text>.*?)[ \t]*$')
        if (-not $headingMatch.Success) {
            continue
        }
        $renderedHeading = Get-RenderedInlineEvidenceText `
            -Value $headingMatch.Groups['text'].Value `
            -NormalizeReferenceLinks `
            -ReferenceText $Body `
            -RequireDefinedReferences
        $renderedHeading = [regex]::Replace(
            $renderedHeading,
            '[ \t]+#+[ \t]*$',
            '').Trim()
        if ($renderedHeading -eq $Heading) {
            $targetIndexes += $index
        }
    }
    if ($targetIndexes.Count -ne 1) {
        return ""
    }

    $startIndex = $targetIndexes[0] + 1
    $endIndex = $lines.Count
    for ($index = $startIndex; $index -lt $lines.Count; $index++) {
        if ($lines[$index] -match '^ {0,3}#{2,3}[ \t]+') {
            $endIndex = $index
            break
        }
    }
    if ($endIndex -le $startIndex) {
        return ""
    }

    return ($lines[$startIndex..($endIndex - 1)] -join "`n")
}

function Get-MarkdownHeadingCount {
    param(
        [string]$Body,
        [string]$Heading
    )

    $count = 0
    foreach ($line in ($Body -split '\r?\n')) {
        $headingMatch = [regex]::Match(
            $line,
            '^ {0,3}#{2,3}[ \t]+(?<text>.*?)[ \t]*$')
        if (-not $headingMatch.Success) {
            continue
        }
        $renderedHeading = Get-RenderedInlineEvidenceText `
            -Value $headingMatch.Groups['text'].Value `
            -NormalizeReferenceLinks `
            -ReferenceText $Body `
            -RequireDefinedReferences
        $renderedHeading = [regex]::Replace(
            $renderedHeading,
            '[ \t]+#+[ \t]*$',
            '').Trim()
        if ($renderedHeading -eq $Heading) {
            $count++
        }
    }
    return $count
}

function Get-RawMarkdownSectionAtRenderedBoundaries {
    param(
        [string]$RawBody,
        [string]$RenderedBody,
        [string]$Heading
    )

    $rawLines = @($RawBody -split '\r?\n')
    $renderedLines = @($RenderedBody -split '\r?\n')
    if ($rawLines.Count -ne $renderedLines.Count) {
        return ''
    }

    $escapedHeading = [regex]::Escape($Heading)
    $targetPattern = "^ {0,3}#{2,3}[ \t]+$escapedHeading(?:[ \t]+#+)?[ \t]*$"
    $nextHeadingPattern = '^ {0,3}#{2,3}[ \t]+'
    $targetIndexes = @()
    for ($index = 0; $index -lt $renderedLines.Count; $index++) {
        if ($renderedLines[$index] -match $targetPattern) {
            $targetIndexes += $index
        }
    }
    if ($targetIndexes.Count -ne 1) {
        return ''
    }

    $startIndex = $targetIndexes[0] + 1
    $endIndex = $renderedLines.Count
    for ($index = $startIndex; $index -lt $renderedLines.Count; $index++) {
        if ($renderedLines[$index] -match $nextHeadingPattern) {
            $endIndex = $index
            break
        }
    }
    if ($endIndex -le $startIndex) {
        return ''
    }

    return ($rawLines[$startIndex..($endIndex - 1)] -join "`n")
}

function Mask-MarkdownHtmlCommentsOutsideCode {
    param([AllowEmptyString()][string]$Body)

    $maskedLines = [System.Collections.Generic.List[string]]::new()
    $inFence = $false
    $fenceCharacter = ''
    $fenceLength = 0
    $inComment = $false
    $codeSpanLength = 0
    $paragraphInterruptHtmlTags = 'address|article|aside|base|basefont|blockquote|body|caption|center|col|colgroup|dd|details|dialog|dir|div|dl|dt|fieldset|figcaption|figure|footer|form|frame|frameset|h[1-6]|head|header|hr|html|iframe|legend|li|link|main|menu|menuitem|nav|noframes|ol|optgroup|option|p|param|pre|script|search|section|style|summary|table|tbody|td|textarea|tfoot|th|thead|title|tr|track|ul'

    foreach ($line in ($Body -split '\r?\n')) {
        $interruptsParagraph = $line -match '^ {0,3}(?:<!--|<\?|<!\[CDATA\[|<![A-Z]|`{3,}|~{3,})' -or
            $line -match "^ {0,3}</?(?:$paragraphInterruptHtmlTags)(?:\s|/?>|$)"
        if ($codeSpanLength -gt 0 -and
            ([string]::IsNullOrWhiteSpace($line) -or $interruptsParagraph)) {
            # A CommonMark code span cannot cross the blank line that ends its
            # paragraph or a block construct that interrupts that paragraph.
            # Treat an unmatched opener conservatively as masked text, then
            # resume block classification at the boundary.
            $codeSpanLength = 0
        }
        if ([string]::IsNullOrWhiteSpace($line)) {
            $maskedLines.Add($line)
            continue
        }

        if ($inFence -and -not $inComment -and $codeSpanLength -eq 0) {
            $maskedLines.Add(' ' * $line.Length)
            $escapedFenceCharacter = [regex]::Escape($fenceCharacter)
            if ($line -match "^ {0,3}$escapedFenceCharacter{$fenceLength,}[ \t]*$") {
                $inFence = $false
            }
            continue
        }

        if (-not $inComment -and $codeSpanLength -eq 0) {
            $fenceMatch = [regex]::Match($line, '^ {0,3}(?<fence>`{3,}|~{3,})')
            if ($fenceMatch.Success) {
                $maskedLines.Add(' ' * $line.Length)
                $inFence = $true
                $fenceCharacter = $fenceMatch.Groups['fence'].Value.Substring(0, 1)
                $fenceLength = $fenceMatch.Groups['fence'].Value.Length
                continue
            }

            if ($line -match '^(?: {4}|\t)') {
                $maskedLines.Add(' ' * $line.Length)
                continue
            }
        }

        $characters = $line.ToCharArray()
        $searchIndex = 0
        while ($searchIndex -lt $line.Length) {
            if ($inComment) {
                $commentStart = $searchIndex
                $commentEnd = $line.IndexOf('-->', $searchIndex, [System.StringComparison]::Ordinal)
                $maskEnd = if ($commentEnd -ge 0) { $commentEnd + 3 } else { $line.Length }
                while ($commentEnd -ge 0 -and $maskEnd -lt $line.Length -and
                    $line.IndexOf('<!--', $maskEnd, [System.StringComparison]::Ordinal) -eq $maskEnd) {
                    $commentEnd = $line.IndexOf('-->', $maskEnd + 4, [System.StringComparison]::Ordinal)
                    $maskEnd = if ($commentEnd -ge 0) { $commentEnd + 3 } else { $line.Length }
                }
                for ($index = $searchIndex; $index -lt $maskEnd; $index++) {
                    $characters[$index] = ' '
                }
                if ($commentEnd -lt 0) {
                    $searchIndex = $line.Length
                } else {
                    if ($commentStart -gt 0 -and $maskEnd -lt $line.Length -and
                        [char]::IsLetterOrDigit($line[$commentStart - 1]) -and
                        [char]::IsLetterOrDigit($line[$maskEnd])) {
                        # Removing this rendered inline comment would join two
                        # token fragments. Mark the body as ambiguous so the
                        # structured evidence path fails closed.
                        $characters[$commentStart] = [char]0xFFFD
                    }
                    $inComment = $false
                    $searchIndex = $maskEnd
                }
                continue
            }

            $precedingBackslashes = 0
            for ($probeIndex = $searchIndex - 1;
                $probeIndex -ge 0 -and $line[$probeIndex] -eq '\';
                $probeIndex--) {
                $precedingBackslashes++
            }
            $isEscaped = ($precedingBackslashes % 2) -eq 1

            if ($codeSpanLength -gt 0) {
                $characters[$searchIndex] = ' '
                # Backslash escapes do not apply inside a CommonMark code span;
                # an exact delimiter run closes even when preceded by '\'.
                if ($line[$searchIndex] -eq '`') {
                    $runEnd = $searchIndex
                    while ($runEnd -lt $line.Length -and $line[$runEnd] -eq '`') {
                        $characters[$runEnd] = ' '
                        $runEnd++
                    }
                    if (($runEnd - $searchIndex) -eq $codeSpanLength) {
                        $codeSpanLength = 0
                    }
                    $searchIndex = $runEnd
                } else {
                    $searchIndex++
                }
                continue
            }

            if (-not $isEscaped -and
                $line.IndexOf('<!--', $searchIndex, [System.StringComparison]::Ordinal) -eq $searchIndex) {
                $inComment = $true
                continue
            }

            if ($line[$searchIndex] -eq '`' -and -not $isEscaped) {
                $runEnd = $searchIndex
                while ($runEnd -lt $line.Length -and $line[$runEnd] -eq '`') {
                    $characters[$runEnd] = ' '
                    $runEnd++
                }
                $codeSpanLength = $runEnd - $searchIndex
                $searchIndex = $runEnd
                continue
            }

            $searchIndex++
        }
        $maskedLines.Add(($characters -join ''))
    }

    return ($maskedLines -join "`n")
}

function Get-RenderedMarkdownEvidenceText {
    param(
        [AllowEmptyString()][string]$Body,
        [switch]$RejectTopLevelRawHtml
    )

    $commentMaskedBody = Mask-MarkdownHtmlCommentsOutsideCode -Body $Body
    if ($commentMaskedBody.Contains([char]0xFFFD)) {
        return ''
    }
    $renderedLines = [System.Collections.Generic.List[string]]::new()
    $inFence = $false
    $fenceCharacter = ''
    $fenceLength = 0
    $rawHtmlEndToken = ''
    $rawHtmlUntilBlank = $false
    $rawHtmlBlockTags = 'address|article|aside|base|basefont|blockquote|body|caption|center|col|colgroup|dd|details|dialog|dir|div|dl|dt|fieldset|figcaption|figure|footer|form|frame|frameset|h[1-6]|head|header|hr|html|iframe|legend|li|link|main|menu|menuitem|nav|noframes|ol|optgroup|option|p|param|search|section|summary|table|tbody|td|tfoot|th|thead|title|tr|track|ul'
    $emailAutolinkPattern = '^<[A-Za-z0-9.!#$%&''*+/=?^_`{|}~-]+@[A-Za-z0-9](?:[A-Za-z0-9-]{0,61}[A-Za-z0-9])?(?:\.[A-Za-z0-9](?:[A-Za-z0-9-]{0,61}[A-Za-z0-9])?)*>[ \t]*$'

    foreach ($line in ($commentMaskedBody -split '\r?\n')) {
        if (-not [string]::IsNullOrEmpty($rawHtmlEndToken)) {
            if ($line.Contains($rawHtmlEndToken, [System.StringComparison]::OrdinalIgnoreCase)) {
                $rawHtmlEndToken = ''
            }
            $renderedLines.Add(' ' * $line.Length)
            continue
        }
        if ($rawHtmlUntilBlank) {
            if ([string]::IsNullOrWhiteSpace($line)) {
                $rawHtmlUntilBlank = $false
                $renderedLines.Add($line)
            } else {
                $renderedLines.Add(' ' * $line.Length)
            }
            continue
        }

        if ($inFence) {
            $escapedFenceCharacter = [regex]::Escape($fenceCharacter)
            if ($line -match "^ {0,3}$escapedFenceCharacter{$fenceLength,}[ \t]*$") {
                $inFence = $false
            }
            $renderedLines.Add(' ' * $line.Length)
            continue
        }

        $fenceMatch = [regex]::Match($line, '^ {0,3}(?<fence>`{3,}|~{3,})')
        if ($fenceMatch.Success) {
            $inFence = $true
            $fenceCharacter = $fenceMatch.Groups['fence'].Value.Substring(0, 1)
            $fenceLength = $fenceMatch.Groups['fence'].Value.Length
            $renderedLines.Add(' ' * $line.Length)
            continue
        }

        if ($line -match '^(?: {4}|\t)') {
            $renderedLines.Add(' ' * $line.Length)
            continue
        }

        if ($line -match '^ {0,3}<') {
            $trimmedHtmlLine = $line.TrimStart()
            $isAutolink = $trimmedHtmlLine -match '^<[A-Za-z][A-Za-z0-9+.-]{1,31}:[^\x00-\x20\x7F<>]*>[ \t]*$' -or
                $trimmedHtmlLine -match $emailAutolinkPattern
            if ($isAutolink) {
                $renderedLines.Add($line)
                continue
            }

            $isRawHtmlBlock = $false
            $typeOneMatch = [regex]::Match(
                $trimmedHtmlLine,
                '^<(?<tag>pre|script|style|textarea)(?:\s|>|$)',
                [System.Text.RegularExpressions.RegexOptions]::IgnoreCase)
            if ($typeOneMatch.Success) {
                $isRawHtmlBlock = $true
                $closingToken = "</$($typeOneMatch.Groups['tag'].Value)>"
                if (-not $trimmedHtmlLine.Contains($closingToken, [System.StringComparison]::OrdinalIgnoreCase)) {
                    $rawHtmlEndToken = $closingToken
                }
            } elseif ($trimmedHtmlLine.StartsWith('<?')) {
                $isRawHtmlBlock = $true
                if (-not $trimmedHtmlLine.Contains('?>')) { $rawHtmlEndToken = '?>' }
            } elseif ($trimmedHtmlLine.StartsWith('<![CDATA[', [System.StringComparison]::OrdinalIgnoreCase)) {
                $isRawHtmlBlock = $true
                if (-not $trimmedHtmlLine.Contains(']]>')) { $rawHtmlEndToken = ']]>' }
            } elseif ($trimmedHtmlLine -match '^<![A-Z]') {
                $isRawHtmlBlock = $true
                if (-not $trimmedHtmlLine.Contains('>')) { $rawHtmlEndToken = '>' }
            } elseif ($trimmedHtmlLine -match "^</?(?:$rawHtmlBlockTags)(?:\s|/?>)" -or
                $trimmedHtmlLine -match '^</?[A-Za-z][A-Za-z0-9-]*(?:\s+[^>]*)?/?>\s*$') {
                $isRawHtmlBlock = $true
                $rawHtmlUntilBlank = $true
            }

            # At a top-level line start, any angle-bracket construct that is
            # not a proven autolink is excluded conservatively. This covers
            # quoted '>' characters and future HTML forms without weakening
            # the sign-off boundary.
            if (-not $isRawHtmlBlock) {
                $isRawHtmlBlock = $true
                $rawHtmlUntilBlank = $true
            }

            if ($isRawHtmlBlock) {
                if ($RejectTopLevelRawHtml) {
                    return ''
                }
                $renderedLines.Add(' ' * $line.Length)
                continue
            }
        }

        $renderedLines.Add($line)
    }

    return ($renderedLines -join "`n")
}

function Test-UniqueRenderedSectionOrLegacyText {
    param(
        [string]$Body,
        [string]$Heading,
        [string]$LegacyPattern
    )

    $escapedHeading = [regex]::Escape($Heading)
    $headingPattern = "(?im)^ {0,3}#{2,3}[ \t]+$escapedHeading(?:[ \t]+#+)?[ \t]*$"
    $headingCount = [regex]::Matches($Body, $headingPattern).Count
    if ($headingCount -gt 1) {
        return $false
    }
    if ($headingCount -eq 1) {
        return -not [string]::IsNullOrWhiteSpace(
            (Get-MarkdownSection -Body $Body -Heading $Heading))
    }

    return $Body -match $LegacyPattern
}

function Get-SeverityClassification {
    param([string]$Body)

    $renderedBody = Get-RenderedMarkdownEvidenceText -Body $Body
    $severityPattern = '(none|low|medium|high|catastrophic)'
    $legacySeverityPattern = "(?im)^\s*(?:potential\s+)?severity(?:\s+if\s+misused)?\s*:\s*$severityPattern\s*$"
    $rawSeverityHeadingCount = Get-MarkdownHeadingCount `
        -Body $Body `
        -Heading 'Potential Severity If Misused'
    $severityHeadingCount = Get-MarkdownHeadingCount `
        -Body $renderedBody `
        -Heading 'Potential Severity If Misused'
    $rawLegacySeverityCount = [regex]::Matches($Body, $legacySeverityPattern).Count
    $renderedLegacySeverityMatches = [regex]::Matches($renderedBody, $legacySeverityPattern)
    if ($rawSeverityHeadingCount -ne $severityHeadingCount -or
        $rawLegacySeverityCount -ne $renderedLegacySeverityMatches.Count -or
        $severityHeadingCount -gt 1) {
        return ""
    }

    if ($severityHeadingCount -eq 1) {
        if ($renderedLegacySeverityMatches.Count -ne 0) {
            return ""
        }
        $rawSection = Get-RawMarkdownSectionAtRenderedBoundaries `
            -RawBody $Body `
            -RenderedBody $renderedBody `
            -Heading "Potential Severity If Misused"
        $section = Get-MarkdownSection -Body $renderedBody -Heading "Potential Severity If Misused"
        $decodedRawSection = Get-RenderedInlineEvidenceText `
            -Value $rawSection `
            -NormalizeReferenceLinks
        $decodedSection = Get-RenderedInlineEvidenceText `
            -Value $section `
            -NormalizeReferenceLinks
        $rawSectionSeverityMatches = [regex]::Matches(
            $decodedRawSection,
            "(?i)\b(?<severity>$severityPattern)\b")
        $sectionSeverityMatches = [regex]::Matches(
            $decodedSection,
            "(?i)\b(?<severity>$severityPattern)\b")
        if ($rawSectionSeverityMatches.Count -eq $sectionSeverityMatches.Count -and
            $sectionSeverityMatches.Count -eq 1 -and
            $rawSectionSeverityMatches[0].Groups['severity'].Value.Equals(
                $sectionSeverityMatches[0].Groups['severity'].Value,
                [System.StringComparison]::OrdinalIgnoreCase)) {
            return $sectionSeverityMatches[0].Groups['severity'].Value.ToLowerInvariant()
        }
        return ""
    }

    # Retain compatibility with evidence created before the issue form emitted
    # the severity as its own Markdown section.
    if ($renderedLegacySeverityMatches.Count -eq 1) {
        return $renderedLegacySeverityMatches[0].Groups[1].Value.ToLowerInvariant()
    }

    return ""
}

function Get-EvidenceField {
    param(
        [string]$Text,
        [string]$Name
    )

    $escapedName = [regex]::Escape($Name)
    $matches = [regex]::Matches($Text, "(?im)^\s*$escapedName\s*:\s*(?<value>[^\r\n]*?)\s*$")
    if ($matches.Count -ne 1) {
        return ""
    }

    return $matches[0].Groups["value"].Value.Trim()
}

function Get-EvidenceFieldGroup {
    param(
        [string]$Text,
        [string[]]$Names
    )

    $values = @()
    foreach ($name in $Names) {
        $escapedName = [regex]::Escape($name)
        foreach ($match in [regex]::Matches($Text, "(?im)^\s*$escapedName\s*:\s*(?<value>[^\r\n]*?)\s*$")) {
            $values += $match.Groups["value"].Value.Trim()
        }
    }

    if ($values.Count -ne 1) {
        return ""
    }
    return $values[0]
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

    $trimmed = (Get-RenderedInlineEvidenceText `
        -Value $Value `
        -NormalizeReferenceLinks).Trim()
    if ($trimmed.Length -lt 12 -or $trimmed -match '^<[^>]+>$') {
        return $false
    }

    $normalized = [regex]::Replace($trimmed.ToLowerInvariant(), '[^a-z0-9]+', ' ').Trim()
    $placeholderState = '^(?:absent|blocked|deferred|incomplete|later|missing|none|pending|placeholder|skipped|tbd|todo|unavailable|unchecked|unqualified|unverified|unknown)(?:\s+(?:at\s+this\s+time|automation|check|evidence|later|review|run|test|verification))?$'
    $placeholderPrefix = '^(?:absent|blocked|deferred|incomplete|later|missing|none|pending|placeholder|skipped|tbd|todo|unavailable|unchecked|unqualified|unverified|unknown)\b'
    $isCompoundScopePrefix = $trimmed.ToLowerInvariant() -match '^(?:blocked|incomplete)-[a-z0-9-]+\b'
    $subjectState = '\b(?:automation|changes|check|evidence|review|reviewer|run|test|verification|workflow)(?:\s+(?:is|was|has\s+been|remain(?:s|ed)?))?\s+(?:not\s+only\s+)?(?<state>absent|blocked|deferred|incomplete|missing|pending|placeholder|skipped|unavailable|unchecked|unqualified|unverified|unknown)\b'
    $hasSubjectStateWithoutPassingContext = $false
    foreach ($clause in ($trimmed -split '[.;:\r\n]+')) {
        $rawClause = $clause.ToLowerInvariant()
        foreach ($stateMatch in [regex]::Matches($rawClause, $subjectState)) {
            $state = $stateMatch.Groups['state'].Value
            $afterState = $rawClause.Substring($stateMatch.Index + $stateMatch.Length)
            $isPassingCompoundScope =
                $state -match '^(?:blocked|incomplete)$' -and
                $stateMatch.Value -notmatch '\b(?:is|was|has\s+been|remain(?:s|ed)?)\b' -and
                $afterState -match '^-[a-z0-9-]+\b.*\b(?:pass|passed|succeed|succeeded|successful|verified)\b'
            if (-not $isPassingCompoundScope) {
                $hasSubjectStateWithoutPassingContext = $true
                break
            }
        }
        if ($hasSubjectStateWithoutPassingContext) {
            break
        }
    }
    if ($normalized -match $placeholderState -or
        ($normalized -match $placeholderPrefix -and -not $isCompoundScopePrefix) -or
        $hasSubjectStateWithoutPassingContext) {
        return $false
    }

    # Outcome rejection is deliberately subject-independent. A producer name
    # (build, lint, scan, deployment, or a future tool) must not determine
    # whether explicitly unsuccessful evidence is admitted.
    $failureScrubbed = [regex]::Replace(
        $normalized,
        '\bfailure\s+boundaries\b',
        '')
    if ($normalized -match '\b(?:does\s+not|do\s+not|never|cannot|prevents?|contains?|isolates?|recovers?|rolls?\s+back)\b') {
        $failureScrubbed = [regex]::Replace(
            $failureScrubbed,
            '\bfailed\s+(?:attempt|attempts|input|inputs|operation|operations|request|requests|transaction|transactions)\b',
            '')
    }
    $failureScrubbed = [regex]::Replace($failureScrubbed, '\bnever\s+failed\b', '')

    $terminalOutcomeWithoutPassingContext = $false
    foreach ($clause in ($trimmed -split '[.;:\r\n]+')) {
        $rawClause = $clause.ToLowerInvariant()
        $normalizedClause = [regex]::Replace($clause.ToLowerInvariant(), '[^a-z0-9]+', ' ').Trim()
        foreach ($terminalOutcome in [regex]::Matches($rawClause, '\b(?:canceled|cancelled|timed[ -]+out|timeout)\b')) {
            $beforeOutcome = $rawClause.Substring(0, $terminalOutcome.Index)
            $afterOutcome = $rawClause.Substring($terminalOutcome.Index + $terminalOutcome.Length)
            $isCompoundScope = $afterOutcome.StartsWith('-')
            $isRelativeInputScope = $beforeOutcome -match '\b(?:attempts?|cases?|files?|inputs?|operations?|records?|requests?|transactions?)\s+(?:that|which)\s+(?:are|is|was|were)\s*$'
            $isExplicitOutcome =
                -not $isRelativeInputScope -and
                $beforeOutcome -match '\b[a-z0-9]+(?:[ \t]+[a-z0-9]+){0,5}[ \t]+(?:are|had|has|have|is|was|were|has[ \t]+been|have[ \t]+been|had[ \t]+been)[ \t]*$'
            $isPassingScope = $afterOutcome -match '^\s+(?!although\b|but\b|despite\b|rather\b|though\b|while\b|whereas\b)[a-z0-9-]+(?:\s+[a-z0-9-]+)*\s+(?:pass|passed|succeed|succeeded|successful|verified)\b'
            $hasNegativeContrast = $afterOutcome -match '^\s*(?:,\s*)?(?:not|rather\s+than|instead\s+of)\b'
            if ($isExplicitOutcome -or
                $hasNegativeContrast -or
                (-not $isCompoundScope -and -not $isRelativeInputScope -and -not $isPassingScope)) {
                $terminalOutcomeWithoutPassingContext = $true
                break
            }
        }
        if ($terminalOutcomeWithoutPassingContext) {
            break
        }
    }

    $terminalEvidenceState = '(?:available|checked|complete|completed|done|finish|finished|pass|passed|provided|qualified|reviewed|run|ran|succeed|succeeded|successful|verified)'
    $hasNegatedTerminalState = $false
    foreach ($clause in ($trimmed -split '[.;:\r\n]+')) {
        $normalizedClause = [regex]::Replace($clause.ToLowerInvariant(), '[^a-z0-9]+', ' ').Trim()
        $negationClause = $normalizedClause
        if ($negationClause -match '\bnot\s+only\b.*\bbut\b') {
            $negationClause = [regex]::Replace($negationClause, '\bnot\s+only\b', '')
        }
        if ($negationClause -match "\b(?:no|not|never|cannot)(?:\s+[a-z0-9]+)*\s+$terminalEvidenceState\b" -or
            $negationClause -match "\b(?:aren|isn|wasn|weren|hasn|haven|hadn|didn|doesn|don|can|cann|couldn|shouldn|wouldn)\s+t(?:\s+[a-z0-9]+)*\s+$terminalEvidenceState\b" -or
            $negationClause -match "\byet(?:\s+[a-z0-9]+)*\s+to\s+$terminalEvidenceState\b") {
            $hasNegatedTerminalState = $true
            break
        }
    }
    if ($trimmed -match '^(?i:n\s*/?\s*a)$' -or
        $failureScrubbed -match '\b(?:failed|failure|unsuccessful)\b' -or
        $terminalOutcomeWithoutPassingContext -or
        $normalized -match '^(?:no|not|never|without)\s+(?:applicable|automation|available|check|checked|completed|done|evidence|provided|qualification|qualified|review|run|test|verification|verified)\b' -or
        $hasNegatedTerminalState) {
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

    $reviewComments = @($Issue.review_comments)
    $indexedReviewComments = for ($index = 0; $index -lt $reviewComments.Count; $index++) {
        [pscustomobject]@{
            Comment = $reviewComments[$index]
            Index = $index
            LatestTimestamp = if (-not [string]::IsNullOrWhiteSpace(
                    ([string]$reviewComments[$index].updated_at))) {
                [string]$reviewComments[$index].updated_at
            } else {
                [string]$reviewComments[$index].created_at
            }
        }
    }
    $applicableTimestampRecords = @($indexedReviewComments | ForEach-Object {
        $candidateAuthor = Get-GitHubLogin -Value ([string]$_.Comment.user.login)
        if ($candidateAuthor -ne $Reviewer -or
            ([string]$_.Comment.user.type).ToLowerInvariant() -ne 'user' -or
            [string]::IsNullOrWhiteSpace($_.LatestTimestamp)) {
            return
        }
        $rawCandidateBody = [string]$_.Comment.body
        $candidatePositionMask = Mask-MarkdownHtmlCommentsOutsideCode -Body $rawCandidateBody
        $candidateHasAmbiguousJoin = $candidatePositionMask.Contains([char]0xFFFD)
        $candidateBody = Get-RenderedMarkdownEvidenceText `
            -Body $rawCandidateBody `
            -RejectTopLevelRawHtml
        $candidateHeadingCount = Get-MarkdownHeadingCount `
            -Body $candidateBody `
            -Heading 'Independent Review Sign-Off'
        if ($candidateHeadingCount -ne 1) {
            if ($candidateHasAmbiguousJoin) {
                $rawCandidateReviewer = Get-GitHubLogin -Value (
                    Get-EvidenceField -Text $rawCandidateBody -Name 'reviewer')
                $rawCandidateDigest = (Get-EvidenceField `
                    -Text $rawCandidateBody `
                    -Name 'reviewed issue body sha256').ToLowerInvariant()
                if ($rawCandidateReviewer -eq $candidateAuthor -and
                    $rawCandidateDigest -eq $issueBodySha256) {
                    [pscustomobject]@{ LatestTimestamp = $_.LatestTimestamp }
                }
            }
            return
        }
        $candidateSignOff = Get-MarkdownSection `
            -Body $candidateBody `
            -Heading 'Independent Review Sign-Off'
        $candidateReviewer = Get-GitHubLogin -Value (
            Get-EvidenceField -Text $candidateSignOff -Name 'reviewer')
        $candidateDigest = (Get-EvidenceField `
            -Text $candidateSignOff `
            -Name 'reviewed issue body sha256').ToLowerInvariant()
        if ($candidateReviewer -eq $candidateAuthor -and
            $candidateDigest -eq $issueBodySha256) {
            [pscustomobject]@{ LatestTimestamp = $_.LatestTimestamp }
        }
    })
    $latestApplicableTimestamp = @($applicableTimestampRecords |
        Sort-Object -Property LatestTimestamp -Descending |
        Select-Object -First 1).LatestTimestamp
    $ambiguousTimestampGroups = @($applicableTimestampRecords |
        Where-Object { $_.LatestTimestamp -eq $latestApplicableTimestamp } |
        Group-Object -Property LatestTimestamp |
        Where-Object { $_.Count -gt 1 })
    if ($ambiguousTimestampGroups.Count -gt 0) {
        return $false
    }
    $orderedReviewComments = @($indexedReviewComments | Sort-Object -Property `
        @{ Expression = { $_.LatestTimestamp }; Descending = $true }, `
        @{ Expression = { $_.Index }; Descending = $true })
    foreach ($entry in $orderedReviewComments) {
        $comment = $entry.Comment
        $commentAuthor = Get-GitHubLogin -Value ([string]$comment.user.login)
        $commentAuthorType = ([string]$comment.user.type).ToLowerInvariant()
        if ($commentAuthor -ne $Reviewer -or $commentAuthorType -ne "user") {
            continue
        }

        $rawCommentBody = [string]$comment.body
        $positionMaskedCommentBody = Mask-MarkdownHtmlCommentsOutsideCode -Body $rawCommentBody
        $hasAmbiguousInlineJoin = $positionMaskedCommentBody.Contains([char]0xFFFD)
        $rawSignOffHeadingCount = Get-MarkdownHeadingCount `
            -Body $rawCommentBody `
            -Heading 'Independent Review Sign-Off'
        $commentBody = Get-RenderedMarkdownEvidenceText -Body $rawCommentBody -RejectTopLevelRawHtml
        if ([string]::IsNullOrWhiteSpace($commentBody) -and
            ($rawSignOffHeadingCount -gt 0 -or $hasAmbiguousInlineJoin)) {
            return $false
        }
        $signOffHeadingCount = Get-MarkdownHeadingCount `
            -Body $commentBody `
            -Heading 'Independent Review Sign-Off'
        if ($signOffHeadingCount -eq 0) {
            continue
        }
        if ($signOffHeadingCount -ne 1) {
            return $false
        }

        $signOff = Get-MarkdownSection -Body $commentBody -Heading "Independent Review Sign-Off"
        $signOffReviewer = Get-GitHubLogin -Value (Get-EvidenceField -Text $signOff -Name "reviewer")
        $signOffQualification = Get-EvidenceField -Text $signOff -Name "qualification"
        $signOffQualificationResult = (Get-EvidenceFieldGroup -Text $signOff -Names @("qualification result", "qualification status")).ToLowerInvariant()
        $signOffVerification = Get-EvidenceField -Text $signOff -Name "verification"
        $signOffIssueBodySha256 = (Get-EvidenceField -Text $signOff -Name "reviewed issue body sha256").ToLowerInvariant()
        $signOffVerificationResult = (Get-EvidenceFieldGroup -Text $signOff -Names @("verification result", "verification status")).ToLowerInvariant()
        $signOffResult = (Get-EvidenceFieldGroup -Text $signOff -Names @("result", "status")).ToLowerInvariant()

        if ($signOffReviewer -eq $commentAuthor -and
            $signOffIssueBodySha256 -match '^[a-f0-9]{64}$' -and
            $signOffIssueBodySha256 -eq $issueBodySha256) {
            return (
                (Test-MeaningfulReviewEvidence -Value $signOffQualification) -and
                $signOffQualificationResult -eq "qualified" -and
                (Test-MeaningfulReviewEvidence -Value $signOffVerification) -and
                $signOffVerificationResult -eq "passed" -and
                $signOffResult -match '^(?:approved|passed)$')
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
    $renderedBody = Get-RenderedMarkdownEvidenceText -Body $body
    $severity = Get-SeverityClassification -Body $body
    $authorLogin = Get-GitHubLogin -Value ([string]$issue.user.login)
    $currentReviewHeadingCount = Get-MarkdownHeadingCount -Body $renderedBody -Heading "Review Evidence"
    $legacyReviewHeadingCount = Get-MarkdownHeadingCount -Body $renderedBody -Heading "Independent Review Evidence"
    $reviewEvidence = Get-MarkdownSection -Body $renderedBody -Heading "Review Evidence"
    $legacyIndependentReviewEvidence = Get-MarkdownSection -Body $renderedBody -Heading "Independent Review Evidence"
    $hasProceduralDeltaMap = Test-UniqueRenderedSectionOrLegacyText -Body $renderedBody -Heading "Procedural Delta Map" -LegacyPattern '(?im)^\s*Procedural\s+Delta\s+Map\s*$'
    $hasMisuseAnalysis = Test-UniqueRenderedSectionOrLegacyText -Body $renderedBody -Heading "Misuse Analysis" -LegacyPattern '(?im)^\s*Misuse\s+Analysis\s*$'
    $hasSimulationEvidence = Test-UniqueRenderedSectionOrLegacyText -Body $renderedBody -Heading "Simulation/Walkthrough Evidence" -LegacyPattern '(?im)^\s*(?:Simulation\b.*|.*\bWalkthrough\b.*)$'
    $hasRollbackAndNotificationPlan = Test-UniqueRenderedSectionOrLegacyText -Body $renderedBody -Heading "Rollback And Field Notification Plan" -LegacyPattern '(?im)^\s*Rollback\b.*\b(?:field\s+notification|notification\s+plan)\b'
    $hasCurrentReviewSchema = $currentReviewHeadingCount -gt 0
    $hasLegacyReviewSchema = $legacyReviewHeadingCount -gt 0
    $hasCurrentReviewEvidence = $currentReviewHeadingCount -eq 1 -and
        -not [string]::IsNullOrWhiteSpace($reviewEvidence)
    $hasLegacyIndependentReviewEvidence = $legacyReviewHeadingCount -eq 1 -and
        -not [string]::IsNullOrWhiteSpace($legacyIndependentReviewEvidence)
    $hasMixedReviewSchemas = $hasCurrentReviewSchema -and $hasLegacyReviewSchema
    $hasReviewEvidence = $hasCurrentReviewSchema -or $hasLegacyReviewSchema

    $currentMode = (Get-EvidenceField -Text $reviewEvidence -Name "mode").ToLowerInvariant()
    $currentReviewer = Get-GitHubLogin -Value (Get-EvidenceField -Text $reviewEvidence -Name "reviewer")
    $currentVerification = Get-EvidenceField -Text $reviewEvidence -Name "verification"
    $currentVerificationResult = (Get-EvidenceFieldGroup -Text $reviewEvidence -Names @("verification result", "verification status")).ToLowerInvariant()
    $currentAutomation = Get-EvidenceField -Text $reviewEvidence -Name "automated evidence"
    $currentAutomationResult = (Get-EvidenceFieldGroup -Text $reviewEvidence -Names @("automated evidence result", "automated evidence status")).ToLowerInvariant()
    $currentResult = (Get-EvidenceFieldGroup -Text $reviewEvidence -Names @("result", "status")).ToLowerInvariant()
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
        $currentVerificationResult -eq "passed" -and
        (Test-MeaningfulReviewEvidence -Value $currentAutomation) -and
        $currentAutomationResult -eq "passed"
    $hasStructuredCurrentIndependentReview = $hasStructuredCurrentReview -and
        $currentIndependentReview -and $currentReviewer -ne $authorLogin
    $hasAuthenticatedCurrentIndependentReview = $hasStructuredCurrentIndependentReview -and
        (Test-AuthenticatedIndependentReview -Issue $issue -Reviewer $currentReviewer)
    $hasValidCurrentReview = $hasStructuredCurrentSelfReview -or
        $hasAuthenticatedCurrentIndependentReview

    $legacyReviewer = Get-GitHubLogin -Value (Get-EvidenceField -Text $legacyIndependentReviewEvidence -Name "reviewer")
    $legacyVerification = Get-EvidenceField -Text $legacyIndependentReviewEvidence -Name "verification"
    $legacyResult = (Get-EvidenceFieldGroup -Text $legacyIndependentReviewEvidence -Names @("result", "status")).ToLowerInvariant()
    $hasStructuredLegacyIndependentReview = $hasLegacyIndependentReviewEvidence -and
        -not [string]::IsNullOrWhiteSpace($authorLogin) -and
        -not [string]::IsNullOrWhiteSpace($legacyReviewer) -and
        $legacyReviewer -ne $authorLogin -and
        (Test-MeaningfulReviewEvidence -Value $legacyVerification) -and
        $legacyResult -match '^(?:approved|passed)$'
    $hasAuthenticatedLegacyIndependentReview = $hasStructuredLegacyIndependentReview -and
        (Test-AuthenticatedIndependentReview -Issue $issue -Reviewer $legacyReviewer)
    $hasValidLegacyIndependentReview = $hasAuthenticatedLegacyIndependentReview

    $hasValidReviewEvidence = -not $hasMixedReviewSchemas -and
        ($hasValidCurrentReview -or $hasValidLegacyIndependentReview)
    $hasApprovedIndependentReview =
        -not $hasMixedReviewSchemas -and
        ($hasAuthenticatedCurrentIndependentReview -or
        $hasValidLegacyIndependentReview)
    $hasUnauthenticatedIndependentClaim =
        ($hasStructuredCurrentIndependentReview -and -not $hasAuthenticatedCurrentIndependentReview) -or
        ($hasStructuredLegacyIndependentReview -and -not $hasAuthenticatedLegacyIndependentReview)

    if ($enforceClosedIssues -and $state.ToLowerInvariant() -ne "closed") {
        $issueErrors += "Issue is not closed (state=$state)."
    }

    $dqMatches = [regex]::Matches($renderedBody, '\bDQ-[A-Za-z0-9-]+\b')
    $dvMatches = [regex]::Matches($renderedBody, '\bDV-[A-Za-z0-9-]+\b')

    $declaredDqIds = @(Get-TraceabilityIds -Text (Get-MarkdownSection -Body $renderedBody -Heading "Documentation Requirement IDs") -Prefix DQ)
    $declaredDvIds = @(Get-TraceabilityIds -Text (Get-MarkdownSection -Body $renderedBody -Heading "Documentation Verification IDs") -Prefix DV)
    $declaredHzIds = @(Get-TraceabilityIds -Text (Get-MarkdownSection -Body $renderedBody -Heading "Hazard Linkage IDs") -Prefix HZ)
    $hasNoHazardId = $declaredHzIds -contains "HZ-NONE"
    $hasExplicitNoHazard = $declaredHzIds.Count -eq 1 -and $hasNoHazardId
    $mappingSection = Get-MarkdownSection -Body $renderedBody -Heading "DQ/DV/HZ Mapping"
    $mappingRows = @(Get-TraceabilityMappingRows -Body $renderedBody)

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

    if (-not $hasProceduralDeltaMap) {
        $issueErrors += "Missing Procedural Delta Map section."
    }
    if (-not $hasMisuseAnalysis) {
        $issueErrors += "Missing Misuse Analysis section."
    }
    if (-not $hasReviewEvidence) {
        $issueErrors += "Missing Review Evidence."
    } elseif ($hasMixedReviewSchemas) {
        $issueErrors += "Review Evidence and legacy Independent Review Evidence sections are mutually exclusive."
    } elseif ($hasUnauthenticatedIndependentClaim) {
        $issueErrors += "Independent Human Review requires an approved structured sign-off comment authored by the named distinct reviewer."
    } elseif (-not $hasValidReviewEvidence) {
        $issueErrors += "Review Evidence must record an approved structured mode, issue-author or distinct reviewer identity as applicable, verification, and automated evidence for maintainer self-review."
    } elseif (($severity -eq "high" -or $severity -eq "catastrophic") -and -not $hasApprovedIndependentReview) {
        $issueErrors += "Severity '$severity' requires approved Independent Human Review evidence from a second qualified reviewer."
    }
    if (-not $hasSimulationEvidence) {
        $issueErrors += "Missing Simulation/Walkthrough evidence."
    }
    if (-not $hasRollbackAndNotificationPlan) {
        $issueErrors += "Missing rollback plan detail."
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
