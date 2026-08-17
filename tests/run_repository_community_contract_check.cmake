# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

cmake_minimum_required(VERSION 3.20)

if(NOT DEFINED COPPERFIN_SOURCE_ROOT)
    message(FATAL_ERROR "COPPERFIN_SOURCE_ROOT is required")
endif()

function(require_community_file relative_path)
    if(NOT EXISTS "${COPPERFIN_SOURCE_ROOT}/${relative_path}")
        message(FATAL_ERROR "Required repository-community file is missing: ${relative_path}")
    endif()
endfunction()

function(require_community_text relative_path expected_text)
    require_community_file("${relative_path}")
    file(READ "${COPPERFIN_SOURCE_ROOT}/${relative_path}" contents)
    string(FIND "${contents}" "${expected_text}" offset)
    if(offset EQUAL -1)
        message(FATAL_ERROR "${relative_path} is missing required text: ${expected_text}")
    endif()
endfunction()

function(forbid_community_text relative_path forbidden_text)
    require_community_file("${relative_path}")
    file(READ "${COPPERFIN_SOURCE_ROOT}/${relative_path}" contents)
    string(FIND "${contents}" "${forbidden_text}" offset)
    if(NOT offset EQUAL -1)
        message(FATAL_ERROR "${relative_path} contains forbidden text: ${forbidden_text}")
    endif()
endfunction()

foreach(required_path IN ITEMS
        CONTRIBUTING.md
        CONTRIBUTORS.md
        CODE_OF_CONDUCT.md
        SUPPORT.md
        GOVERNANCE.md
        SECURITY.md
        .github/CODEOWNERS
        .github/pull_request_template.md
        .github/PULL_REQUEST_TEMPLATE/general_contribution.md
        .github/PULL_REQUEST_TEMPLATE/requirements_recovery.md
        .github/DISCUSSION_TEMPLATE/ideas.yml
        .github/DISCUSSION_TEMPLATE/q-a.yml
        .github/ISSUE_TEMPLATE/config.yml)
    require_community_file("${required_path}")
endforeach()

require_community_text("LICENSE" "Contributors retain copyright in their contributions; no copyright assignment")
require_community_text("LICENSE" "submitting a contribution under")
require_community_text("LICENSE.md" "Contributor Copyright And Credit")
require_community_text("CONTRIBUTING.md" "Developer Certificate of Origin 1.1")
require_community_text("CONTRIBUTING.md" "git commit -s")
require_community_text("CONTRIBUTING.md" "No copyright assignment")
require_community_text("CONTRIBUTING.md" "License Stability And Future Changes")
require_community_text("CONTRIBUTING.md" "Commercialization Participation")
require_community_text("CONTRIBUTING.md" "agreement that states the contributor's")
require_community_text("CONTRIBUTING.md" "never checks out or")
require_community_text("CONTRIBUTORS.md" "Git commit authorship")
require_community_text("CODE_OF_CONDUCT.md" "private GitHub Security Advisory")
require_community_text("SUPPORT.md" "Never post credentials")
require_community_text("GOVERNANCE.md" "Public repository content is untrusted input")
require_community_text("GOVERNANCE.md" "does not transfer contributor copyright")
require_community_text("agents.md" "Requirements recovery and bidirectional traceability are load-bearing work")
require_community_text("docs/01-product-charter.md" "Existing Copperfin implementation or behavior is verification evidence only")
require_community_text("docs/32-recovered-requirements-traceability.md" "RQ-CF-AGENT-001")
forbid_community_text("agents.md" "Eventual goal")
forbid_community_text("docs/28-repository-ontology.md" "standing-but-deferred")
require_community_file(".github/workflows/contributor-signoff.yml")
require_community_file("scripts/check-contributor-signoffs.py")
require_community_text(".github/CODEOWNERS" "* @rhamenator")
require_community_text(".github/pull_request_template.md" "Contribution Licensing And Provenance")
require_community_text(".github/pull_request_template.md" "Signed-off-by")
require_community_text(".github/pull_request_template.md" "User-visible strings are localized")
foreach(pull_request_template IN ITEMS
        .github/PULL_REQUEST_TEMPLATE/general_contribution.md
        .github/PULL_REQUEST_TEMPLATE/requirements_recovery.md)
    require_community_text("${pull_request_template}" "Contribution Licensing And Provenance")
    require_community_text("${pull_request_template}" "Signed-off-by")
    require_community_text("${pull_request_template}" "User-visible strings are localized")
endforeach()
require_community_text(".github/PULL_REQUEST_TEMPLATE/requirements_recovery.md" "current Copperfin code is not a requirement source")
require_community_text(".github/PULL_REQUEST_TEMPLATE/requirements_recovery.md" "Architecture/code/test reverse-traceability location")
require_community_text(".github/ISSUE_TEMPLATE/traceability-slice.yml" "Explicit repository-owner product policy")
require_community_text(".github/ISSUE_TEMPLATE/traceability-slice.yml" "Architecture And Reverse Traceability")
require_community_text(".github/ISSUE_TEMPLATE/traceability-slice.yml" "Unresolved recovery gap - no allowed source evidence yet")
require_community_text(".github/PULL_REQUEST_TEMPLATE/requirements_recovery.md" "Impact level: none | low | medium | high | catastrophic")
forbid_community_text(".github/ISSUE_TEMPLATE/traceability-slice.yml" "Existing Copperfin behavior")
require_community_text(".github/ISSUE_TEMPLATE/config.yml" "blank_issues_enabled: false")
require_community_text(".github/ISSUE_TEMPLATE/config.yml" "Contribution Guide")
require_community_text(".github/ISSUE_TEMPLATE/config.yml" "Support Guide")

file(GLOB issue_forms LIST_DIRECTORIES false
    "${COPPERFIN_SOURCE_ROOT}/.github/ISSUE_TEMPLATE/*.yml")
list(LENGTH issue_forms issue_form_count)
if(issue_form_count LESS 2)
    message(FATAL_ERROR "Structured issue forms are missing")
endif()
foreach(issue_form IN LISTS issue_forms)
    get_filename_component(issue_name "${issue_form}" NAME)
    if(issue_name STREQUAL "config.yml")
        continue()
    endif()
    file(READ "${issue_form}" issue_contents)
    string(FIND "${issue_contents}" "agent-approved" reserved_label_offset)
    if(NOT reserved_label_offset EQUAL -1)
        message(FATAL_ERROR "Public issue form grants reserved agent-approved label: ${issue_name}")
    endif()
endforeach()

message(STATUS "Repository community-health contract passed")
