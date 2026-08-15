# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

if(NOT DEFINED SOURCE_DIR OR "${SOURCE_DIR}" STREQUAL "")
    message(FATAL_ERROR "SOURCE_DIR is required")
endif()

set(header_path "${SOURCE_DIR}/include/copperfin/platform/private_directory.h")
set(source_path "${SOURCE_DIR}/src/platform/private_directory.cpp")
set(consumer_path "${SOURCE_DIR}/src/security/workspace_agent_environment.cpp")
set(root_build_path "${SOURCE_DIR}/CMakeLists.txt")
set(test_build_path "${SOURCE_DIR}/tests/CMakeLists.txt")
set(workflow_path "${SOURCE_DIR}/.github/workflows/generated-launcher-validation.yml")
set(windows_workflow_path "${SOURCE_DIR}/.github/workflows/windows-environment-validation.yml")

foreach(path IN ITEMS
        "${header_path}"
        "${source_path}"
        "${consumer_path}"
        "${root_build_path}"
        "${test_build_path}"
        "${workflow_path}"
        "${windows_workflow_path}")
    if(NOT EXISTS "${path}")
        message(FATAL_ERROR "Private-directory boundary input is missing: ${path}")
    endif()
endforeach()

file(READ "${header_path}" header_text)
file(READ "${source_path}" source_text)
file(READ "${consumer_path}" consumer_text)
file(READ "${root_build_path}" root_build_text)
file(READ "${test_build_path}" test_build_text)
file(READ "${workflow_path}" workflow_text)
file(READ "${windows_workflow_path}" windows_workflow_text)

function(require_text contents expected description)
    string(FIND "${contents}" "${expected}" offset)
    if(offset EQUAL -1)
        message(FATAL_ERROR "Missing ${description}: ${expected}")
    endif()
endfunction()

function(forbid_text contents forbidden description)
    string(FIND "${contents}" "${forbidden}" offset)
    if(NOT offset EQUAL -1)
        message(FATAL_ERROR "Forbidden ${description}: ${forbidden}")
    endif()
endfunction()

function(require_text_count contents expected minimum description)
    set(remaining "${contents}")
    set(count 0)
    while(1)
        string(FIND "${remaining}" "${expected}" offset)
        if(offset EQUAL -1)
            break()
        endif()
        math(EXPR next_offset "${offset} + 1")
        string(SUBSTRING "${remaining}" ${next_offset} -1 remaining)
        math(EXPR count "${count} + 1")
    endwhile()
    if(count LESS minimum)
        message(FATAL_ERROR
            "Missing ${description}: expected at least ${minimum} occurrences of ${expected}, found ${count}")
    endif()
endfunction()

foreach(token IN ITEMS
        "_WIN32"
        "windows.h"
        "CreateDirectory"
        "SECURITY_DESCRIPTOR"
        "mkdir(")
    forbid_text("${header_text}" "${token}" "native token in portable private-directory API")
endforeach()

foreach(token IN ITEMS
        "::RemoveDirectoryW("
        "::rmdir(")
    forbid_text("${source_text}" "${token}"
        "path-based cleanup of an unverified directory")
endforeach()

foreach(token IN ITEMS
        "::CreateDirectoryW("
        "::SetEntriesInAclW("
        "SE_DACL_PROTECTED"
        "::GetSecurityInfo("
        "FILE_ATTRIBUTE_REPARSE_POINT"
        "header->AceFlags !="
        "ace->Mask != FILE_ALL_ACCESS")
    require_text("${source_text}" "${token}" "private platform implementation")
endforeach()
require_text_count("${source_text}"
    "windows_parent_components_are_direct(path)" 2
    "pre-create and verification Windows parent-reparse rejection")

foreach(token IN ITEMS
        "::openat("
        "O_NOFOLLOW"
        "::mkdirat("
        "::fstat("
        "descriptor_is_trusted_creation_parent"
        "(status.st_mode & S_ISVTX)"
        "status.st_uid == ::geteuid()"
        "(status.st_mode & 07777) == 0700")
    require_text("${source_text}" "${token}"
        "descriptor-relative POSIX private-directory operation")
endforeach()

forbid_text("${consumer_text}"
    "copperfin::platform::create_private_directory("
    "unbound workspace-agent layout preparation")
require_text_count("${consumer_text}"
    "copperfin::platform::create_private_directory_in_verified_parent("
    2
    "identity-bound workspace-agent root and child creation")
require_text("${consumer_text}"
    "session_created.storage_id"
    "identity-bound workspace-agent child creation")
require_text("${consumer_text}"
    "copperfin::platform::verify_private_directory("
    "workspace-agent layout verification delegation")
require_text("${consumer_text}"
    "workspace_agent.environment_session_layout_unrepresentable"
    "pre-creation derived-environment denial")
string(FIND "${consumer_text}" "const auto proposed_entries =" proposed_offset)
string(FIND "${consumer_text}"
    "copperfin::platform::create_private_directory_in_verified_parent("
    creation_offset)
string(FIND "${consumer_text}"
    "const std::string_view preparation_identity_failure ="
    configured_identity_offset)
if(proposed_offset EQUAL -1 OR configured_identity_offset EQUAL -1 OR
   creation_offset EQUAL -1 OR NOT proposed_offset LESS configured_identity_offset OR
   NOT configured_identity_offset LESS creation_offset)
    message(FATAL_ERROR
        "Derived environment and configured identities must be validated before session-root creation")
endif()
require_text("${root_build_text}"
    "src/platform/private_directory.cpp"
    "platform-support source registration")
require_text("${root_build_text}"
    "target_link_libraries(cf_platform_support PRIVATE advapi32)"
    "Windows ACL library ownership")
require_text("${test_build_text}"
    "test_platform_private_directory cf_platform_support"
    "private-directory behavior test registration")
foreach(contents IN ITEMS "${workflow_text}" "${windows_workflow_text}")
    require_text("${contents}"
        "test_platform_private_directory_boundary_contract"
        "hosted private-directory boundary scheduling")
    require_text("${contents}"
        "test_platform_private_directory"
        "hosted private-directory behavior scheduling")
    require_text("${contents}"
        "test_workspace_agent_isolated_environment"
        "hosted workspace-agent consumer scheduling")
endforeach()
foreach(path IN ITEMS
        "tests/test_platform_private_directory.cpp"
        "tests/test_workspace_agent_isolated_environment.cpp"
        "tests/run_platform_private_directory_boundary_contract_check.cmake")
    require_text_count("${workflow_text}" "${path}" 2
        "push and pull-request private-layout path-filter coverage")
endforeach()

message(STATUS "Portable private-directory boundary contract passed")
