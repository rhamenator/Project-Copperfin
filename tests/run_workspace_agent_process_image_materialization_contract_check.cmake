# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

cmake_minimum_required(VERSION 3.20)

if(NOT DEFINED SOURCE_DIR OR "${SOURCE_DIR}" STREQUAL "")
    message(FATAL_ERROR "SOURCE_DIR is required")
endif()

function(require_text relative_path expected description)
    file(READ "${SOURCE_DIR}/${relative_path}" contents)
    string(FIND "${contents}" "${expected}" offset)
    if(offset EQUAL -1)
        message(FATAL_ERROR "${relative_path} is missing ${description}: ${expected}")
    endif()
endfunction()

function(forbid_text relative_path forbidden description)
    file(READ "${SOURCE_DIR}/${relative_path}" contents)
    string(FIND "${contents}" "${forbidden}" offset)
    if(NOT offset EQUAL -1)
        message(FATAL_ERROR "${relative_path} contains forbidden ${description}: ${forbidden}")
    endif()
endfunction()

set(platform_header include/copperfin/platform/private_executable_image.h)
set(platform_source src/platform/private_executable_image.cpp)
set(environment_header include/copperfin/security/workspace_agent_environment.h)
set(environment_source src/security/workspace_agent_environment.cpp)
set(session_header include/copperfin/security/workspace_agent_session.h)
set(session_source src/security/workspace_agent_session.cpp)
set(platform_test tests/test_platform_private_directory.cpp)
set(environment_test tests/test_workspace_agent_isolated_environment.cpp)

foreach(path IN ITEMS
        ${platform_header}
        ${platform_source}
        ${environment_header}
        ${environment_source}
        ${session_header}
        ${session_source}
        ${platform_test}
        ${environment_test})
    if(NOT EXISTS "${SOURCE_DIR}/${path}")
        message(FATAL_ERROR "Process-image materialization input is missing: ${path}")
    endif()
endforeach()

require_text(CMakeLists.txt "src/platform/private_executable_image.cpp"
    "platform source registration")
require_text(tests/CMakeLists.txt
    "test_workspace_agent_process_image_materialization_contract"
    "contract test registration")

foreach(token IN ITEMS
        "class PrivateExecutableImage"
        "PrivateExecutableImage(const PrivateExecutableImage&) = delete"
        "materialize_private_executable_image_in_verified_parent"
        "std::span<const std::uint8_t>")
    require_text(${platform_header} "${token}" "opaque portable image contract")
endforeach()
foreach(token IN ITEMS "HANDLE" "int descriptor" "native_handle" "path()")
    forbid_text(${platform_header} "${token}" "public native image authority")
endforeach()

foreach(token IN ITEMS
        "::CreateFileW("
        "CREATE_NEW"
        "GENERIC_READ | GENERIC_WRITE | DELETE"
        "FILE_SHARE_READ"
        "::SetFileInformationByHandle("
        "FileDispositionInfo"
        "::openat("
        "O_EXCL"
        "O_NOFOLLOW"
        "::unlinkat("
        "::fchmod(image_descriptor, 0500)"
        "status.st_nlink == 0"
        "native_matches_bytes")
    require_text(${platform_source} "${token}" "exact private-image primitive")
endforeach()
require_text(${platform_source}
    "parent_handle, expected_parent_storage_id"
    "Windows parent identity bracketing")
require_text(${platform_source}
    "parent_descriptor, expected_parent_storage_id"
    "POSIX parent identity bracketing")

foreach(token IN ITEMS
        "class WorkspaceAgentMaterializedProcessImage"
        "const WorkspaceAgentMaterializedProcessImage&) = delete"
        "materialize_process_image(")
    require_text(${environment_header} "${token}" "opaque environment materialization boundary")
endforeach()
foreach(token IN ITEMS
        "receipt->boundary_authority != cleanup_authority_"
        "same_directory_object("
        "materialize_private_executable_image_in_verified_parent("
        "std::span<const std::uint8_t> snapshot")
    require_text(${environment_source} "${token}" "receipt-bound image materialization")
endforeach()

foreach(token IN ITEMS
        "class WorkspaceAgentMaterializedProcessLaunch"
        "const WorkspaceAgentMaterializedProcessLaunch&) = delete"
        "materialize_process_launch_candidate(")
    require_text(${session_header} "${token}" "opaque controller materialization boundary")
endforeach()
foreach(token IN ITEMS
        "process_launch_controller_authority_"
        "candidate.impl_->controller_authority !="
        "Serialize the complete one-attempt materialization with stop/start."
        "candidate.impl_->pins.executable_snapshot_for_materialization()"
        "process_environment_boundary_->materialize_process_image("
        "workspace_agent.process_launch_revalidation_pinning_unavailable")
    require_text(${session_source} "${token}" "controller-only one-attempt consumption")
endforeach()

foreach(token IN ITEMS
        "RQ-CF-AGENT-026"
        "PrivateExecutableImage"
        "materialize_private_executable_image_in_verified_parent")
    require_text(${platform_test} "${token}" "platform materialization regression")
endforeach()
foreach(token IN ITEMS
        "RQ-CF-AGENT-026"
        "WorkspaceAgentMaterializedProcessLaunch"
        "materialize_process_launch_candidate("
        "cross-controller"
        "materialization must not silently weaken the invariant execution denial")
    require_text(${environment_test} "${token}" "controller materialization regression")
endforeach()

foreach(workflow IN ITEMS
        .github/workflows/windows-environment-validation.yml
        .github/workflows/generated-launcher-validation.yml)
    require_text(${workflow}
        "test_workspace_agent_process_image_materialization_contract"
        "hosted materialization-contract scheduling")
endforeach()

message(STATUS "Workspace-agent exact-snapshot materialization contract passed")
