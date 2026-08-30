# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

if(NOT DEFINED SOURCE_DIR OR "${SOURCE_DIR}" STREQUAL "")
    message(FATAL_ERROR "SOURCE_DIR is required")
endif()

set(header_path "${SOURCE_DIR}/include/copperfin/platform/private_directory.h")
set(source_path "${SOURCE_DIR}/src/platform/private_directory.cpp")
set(environment_header_path
    "${SOURCE_DIR}/include/copperfin/security/workspace_agent_environment.h")
set(consumer_path "${SOURCE_DIR}/src/security/workspace_agent_environment.cpp")
set(environment_test_path
    "${SOURCE_DIR}/tests/test_workspace_agent_isolated_environment.cpp")
set(environment_session_layout_lifecycle_test_path
    "${SOURCE_DIR}/tests/test_workspace_agent_isolated_environment_session_layout_lifecycle.inl")
set(root_build_path "${SOURCE_DIR}/CMakeLists.txt")
set(test_build_path "${SOURCE_DIR}/tests/CMakeLists.txt")
set(workflow_path "${SOURCE_DIR}/.github/workflows/generated-launcher-validation.yml")
set(windows_workflow_path "${SOURCE_DIR}/.github/workflows/windows-environment-validation.yml")

foreach(path IN ITEMS
        "${header_path}"
        "${source_path}"
        "${environment_header_path}"
        "${consumer_path}"
        "${environment_test_path}"
        "${environment_session_layout_lifecycle_test_path}"
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
file(READ "${environment_header_path}" environment_header_text)
file(READ "${consumer_path}" consumer_text)
file(READ "${environment_test_path}" environment_test_text)
file(READ "${environment_session_layout_lifecycle_test_path}"
    environment_session_layout_lifecycle_test_text)
string(APPEND environment_test_text
    "\n${environment_session_layout_lifecycle_test_text}")
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

foreach(token IN ITEMS "::RemoveDirectoryW(" "::rmdir(")
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
foreach(token IN ITEMS
        "remove_empty_private_directory_in_verified_parent"
        "::SetFileInformationByHandle("
        "FileDispositionInfo"
        "FILE_READ_ATTRIBUTES | READ_CONTROL | DELETE")
    require_text("${source_text}" "${token}"
        "identity-bound empty-directory cleanup")
endforeach()
require_text_count("${source_text}"
    "windows_parent_components_are_direct(path)" 2
    "pre-create and verification Windows parent-reparse rejection")

foreach(token IN ITEMS
        "::openat("
        "O_NOFOLLOW"
        "::mkdirat("
        "::unlinkat("
        "AT_REMOVEDIR"
        "::fstat("
        "descriptor_has_no_extended_acl"
        "ACL_TYPE_EXTENDED"
        "system.posix_acl_access"
        "system.posix_acl_default"
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
foreach(token IN ITEMS
        "cleanup_empty_session_layout"
        "workspace_agent.environment_session_layout_cleanup_invalid_receipt"
        "workspace_agent.environment_session_layout_cleanup_not_empty"
        "workspace_agent.environment_session_layout_cleaned")
    require_text("${consumer_text}" "${token}"
        "identity-bound workspace-agent layout cleanup")
endforeach()
foreach(token IN ITEMS
        "struct CleanupReceipt"
        "std::shared_ptr<const CleanupReceipt> cleanup_receipt_"
        "std::shared_ptr<const std::uint8_t> boundary_authority"
        "std::shared_ptr<const std::uint8_t> cleanup_authority_"
        "const WorkspaceAgentIsolatedEnvironmentBoundary&) = delete"
        "WorkspaceAgentIsolatedEnvironmentBoundary&&) noexcept = default")
    require_text("${environment_header_text}" "${token}"
        "opaque boundary-bound cleanup receipt")
endforeach()
foreach(token IN ITEMS
        "receipt->boundary_authority != cleanup_authority_"
        "receipt->session_generation != preparation.session_generation")
    require_text("${consumer_text}" "${token}"
        "cleanup receipt provenance validation")
endforeach()
foreach(token IN ITEMS
        "HasPublicSessionDirectoryIdentity"
        "HasPublicChildDirectoryIdentities"
        "!std::is_copy_constructible_v<WorkspaceAgentIsolatedEnvironmentBoundary>"
        "public status fields must not forge cleanup authority"
        "receipt must remain bound to the boundary")
    require_text("${environment_test_text}" "${token}"
        "opaque-receipt regression coverage")
endforeach()
require_text_count("${consumer_text}"
    "remove_empty_private_directory_in_verified_parent(" 2
    "identity-bound child and generation cleanup")
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
        "tests/test_workspace_agent_isolated_environment_session_layout_lifecycle.inl"
        "tests/run_platform_private_directory_boundary_contract_check.cmake")
    require_text_count("${workflow_text}" "${path}" 1
        "direct-push private-layout path-filter coverage")
endforeach()
require_text("${workflow_text}" [=[  pull_request:
    branches: [main, v1-development]
  workflow_dispatch:]=]
    "unconditional protected-branch pull-request private-layout coverage")

message(STATUS "Portable private-directory boundary contract passed")
