# Copyright © 2026 Richard M. Hamilton. All rights reserved.
# Licensed under the Project Copperfin Source-Available License or
# Commercial License. See LICENSE.md in the repository root.

Set-StrictMode -Version Latest

function Get-CopperfinObjectPropertyValue {
    param(
        [AllowNull()]
        [object]$InputObject,

        [Parameter(Mandatory = $true)]
        [string]$Name
    )

    if ($null -eq $InputObject) {
        return $null
    }

    $property = $InputObject.PSObject.Properties[$Name]
    if ($null -eq $property) {
        return $null
    }

    return $property.Value
}

function Get-CopperfinRepositoryOwner {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory = $true)]
        [string]$Repository
    )

    if ($Repository -notmatch '^(?<owner>[^/\s]+)/[^/\s]+$') {
        throw "Repository must use the owner/name form."
    }

    return $Matches.owner
}

function Test-CopperfinAgentIssueApproved {
    [CmdletBinding()]
    param(
        [AllowNull()]
        [object]$Issue,

        [Parameter(Mandatory = $true)]
        [string]$TrustedOwner,

        [string]$RequiredLabel = 'agent-approved'
    )

    if ($null -eq $Issue -or
        [string]::IsNullOrWhiteSpace($TrustedOwner) -or
        [string]::IsNullOrWhiteSpace($RequiredLabel)) {
        return $false
    }

    $state = Get-CopperfinObjectPropertyValue -InputObject $Issue -Name 'state'
    if (-not [string]::Equals(
            [string]$state,
            'OPEN',
            [System.StringComparison]::OrdinalIgnoreCase)) {
        return $false
    }

    $number = Get-CopperfinObjectPropertyValue -InputObject $Issue -Name 'number'
    $parsedNumber = 0
    if ($null -eq $number -or
        -not [int]::TryParse([string]$number, [ref]$parsedNumber) -or
        $parsedNumber -le 0) {
        return $false
    }

    $author = Get-CopperfinObjectPropertyValue -InputObject $Issue -Name 'author'
    $authorLogin = Get-CopperfinObjectPropertyValue -InputObject $author -Name 'login'
    if (-not [string]::Equals(
            [string]$authorLogin,
            $TrustedOwner,
            [System.StringComparison]::OrdinalIgnoreCase)) {
        return $false
    }

    $labels = Get-CopperfinObjectPropertyValue -InputObject $Issue -Name 'labels'
    if ($null -eq $labels -or $labels -is [string]) {
        return $false
    }

    foreach ($label in @($labels)) {
        $labelName = Get-CopperfinObjectPropertyValue -InputObject $label -Name 'name'
        if ([string]::Equals(
                [string]$labelName,
                $RequiredLabel,
                [System.StringComparison]::OrdinalIgnoreCase)) {
            return $true
        }
    }

    return $false
}

function Assert-CopperfinAgentIssueApproved {
    [CmdletBinding()]
    param(
        [AllowNull()]
        [object]$Issue,

        [Parameter(Mandatory = $true)]
        [string]$TrustedOwner,

        [string]$RequiredLabel = 'agent-approved'
    )

    if (-not (Test-CopperfinAgentIssueApproved `
            -Issue $Issue `
            -TrustedOwner $TrustedOwner `
            -RequiredLabel $RequiredLabel)) {
        throw "Issue is not an approved owner-authored open agent execution issue."
    }
}

function Select-CopperfinAgentApprovedIssue {
    [CmdletBinding()]
    param(
        [AllowNull()]
        [object[]]$Issues,

        [Parameter(Mandatory = $true)]
        [string]$TrustedOwner,

        [string]$RequiredLabel = 'agent-approved'
    )

    foreach ($issue in @($Issues)) {
        if (Test-CopperfinAgentIssueApproved `
                -Issue $issue `
                -TrustedOwner $TrustedOwner `
                -RequiredLabel $RequiredLabel) {
            Write-Output $issue
        }
    }
}

function ConvertTo-CopperfinAgentIssuePromptLine {
    [CmdletBinding()]
    param(
        [AllowNull()]
        [object[]]$Issues,

        [Parameter(Mandatory = $true)]
        [string]$TrustedOwner,

        [string]$RequiredLabel = 'agent-approved'
    )

    foreach ($issue in @($Issues)) {
        Assert-CopperfinAgentIssueApproved `
            -Issue $issue `
            -TrustedOwner $TrustedOwner `
            -RequiredLabel $RequiredLabel

        $number = [int](Get-CopperfinObjectPropertyValue -InputObject $issue -Name 'number')
        $titleValue = Get-CopperfinObjectPropertyValue -InputObject $issue -Name 'title'
        if ($null -eq $titleValue -or [string]::IsNullOrWhiteSpace([string]$titleValue)) {
            throw "Approved issue content is missing a title."
        }

        $title = [string]$titleValue
        $title = $title.Replace("`r", ' ').Replace("`n", ' ')
        "- #{0}: {1}" -f $number, $title
    }
}

Export-ModuleMember -Function @(
    'Get-CopperfinRepositoryOwner',
    'Test-CopperfinAgentIssueApproved',
    'Assert-CopperfinAgentIssueApproved',
    'Select-CopperfinAgentApprovedIssue',
    'ConvertTo-CopperfinAgentIssuePromptLine'
)
