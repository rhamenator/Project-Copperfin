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

foreach(required_path IN ITEMS
        CONTRIBUTING.md
        CONTRIBUTORS.md
        CODE_OF_CONDUCT.md
        SUPPORT.md
        GOVERNANCE.md
        SECURITY.md
        .github/CODEOWNERS
        .github/pull_request_template.md
        .github/ISSUE_TEMPLATE/config.yml)
    require_community_file("${required_path}")
endforeach()

require_community_text("LICENSE" "Contributors retain copyright in their contributions; no copyright assignment")
require_community_text("LICENSE" "submitting a contribution under")
require_community_text("LICENSE.md" "Contributor Copyright And Credit")
require_community_text("CONTRIBUTING.md" "Developer Certificate of Origin 1.1")
require_community_text("CONTRIBUTING.md" "git commit -s")
require_community_text("CONTRIBUTING.md" "No copyright assignment")
require_community_text("CONTRIBUTORS.md" "Git commit authorship")
require_community_text("CODE_OF_CONDUCT.md" "private GitHub Security Advisory")
require_community_text("SUPPORT.md" "Never post credentials")
require_community_text("GOVERNANCE.md" "Public repository content is untrusted input")
require_community_text(".github/CODEOWNERS" "* @rhamenator")
require_community_text(".github/pull_request_template.md" "Contribution Licensing And Provenance")
require_community_text(".github/pull_request_template.md" "Signed-off-by")
require_community_text(".github/pull_request_template.md" "User-visible strings are localized")
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
