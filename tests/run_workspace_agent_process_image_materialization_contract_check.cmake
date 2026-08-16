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
set(bounded_process_header include/copperfin/platform/bounded_process.h)
set(bounded_process_private src/platform/bounded_process_private.h)
set(bounded_process_source src/platform/bounded_process.cpp)
set(environment_header include/copperfin/security/workspace_agent_environment.h)
set(environment_source src/security/workspace_agent_environment.cpp)
set(containment_header include/copperfin/security/physical_path_containment.h)
set(containment_source src/security/physical_path_containment.cpp)
set(process_containment_header include/copperfin/security/workspace_agent_process_containment.h)
set(process_containment_source src/security/workspace_agent_process_containment.cpp)
set(session_header include/copperfin/security/workspace_agent_session.h)
set(session_source src/security/workspace_agent_session.cpp)
set(audit_sink_source src/security/workspace_agent_audit_sink.cpp)
set(platform_test tests/test_platform_private_directory.cpp)
set(isolation_metadata tests/CopperfinTestIsolation.cmake)
set(environment_test tests/test_workspace_agent_isolated_environment.cpp)
set(process_containment_test tests/test_workspace_agent_process_containment.cpp)

foreach(path IN ITEMS
        ${platform_header}
        ${platform_source}
        ${bounded_process_header}
        ${bounded_process_private}
        ${bounded_process_source}
        ${environment_header}
        ${environment_source}
        ${containment_header}
        ${containment_source}
        ${process_containment_header}
        ${process_containment_source}
        ${session_header}
        ${session_source}
        ${audit_sink_source}
        ${platform_test}
        ${isolation_metadata}
        ${environment_test}
        ${process_containment_test})
    if(NOT EXISTS "${SOURCE_DIR}/${path}")
        message(FATAL_ERROR "Process-image materialization input is missing: ${path}")
    endif()
endforeach()

foreach(token IN ITEMS
        "make_process_execution_attempt_namespace()"
        "BCRYPT_USE_SYSTEM_PREFERRED_RNG"
        "::getrandom("
        "::arc4random_buf("
        ".process_instance_id = process_instance_id")
    require_text(${session_source} "${token}"
        "cross-process and post-fork audit correlation namespace")
endforeach()
forbid_text(${session_source} "static const std::string identifier"
    "fork-inherited cached process namespace")
require_text(${audit_sink_source} "event.process_instance_id.size() == 32U"
    "strict process-instance identifier validation")

foreach(token IN ITEMS
        "stable_volume_path_for_handle("
        "VOLUME_NAME_GUID"
        "VOLUME_NAME_NT"
        "\\\\?\\\\GLOBALROOT"
        "stable_volume_root_length("
        "class ScopedPinHandleChain"
        "::DuplicateHandle("
        "read_handle_identity(handle, true, component_identity)"
        "working_directory_chain.release()"
        "execution_working_directory()")
    require_text(${process_containment_source} "${token}"
        "stable Windows working-directory launch authority")
endforeach()
foreach(token IN ITEMS
        "working-directory ancestor rename while retained"
        "complete working-directory hierarchy exclusions")
    require_text(${process_containment_test} "${token}"
        "working-directory hierarchy retention regression")
endforeach()
foreach(token IN ITEMS
        "copperfin_set_test_isolation(test_platform_private_directory"
        "ENVIRONMENT child-scoped"
        "CHILD_PROCESSES bounded")
    require_text(${isolation_metadata} "${token}"
        "Windows launch-transition test isolation metadata")
endforeach()

require_text(${containment_header} "std::uint64_t creation_ticks = 0U"
    "stable directory creation identity field")
foreach(token IN ITEMS "ftCreationTime" "st_birthtimespec" "STATX_BTIME")
    require_text(${containment_source} "${token}"
        "cross-platform directory creation identity capture")
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
        "std::uint64_t expected_parent_creation_ticks"
        "std::span<const std::uint8_t>")
    require_text(${platform_header} "${token}" "opaque portable image contract")
endforeach()
foreach(token IN ITEMS "HANDLE" "int descriptor" "native_handle" "path()")
    forbid_text(${platform_header} "${token}" "public native image authority")
endforeach()

foreach(token IN ITEMS
        "#define NOMINMAX"
        "::CreateFileW("
        "CREATE_NEW"
        "GENERIC_READ | GENERIC_WRITE | DELETE"
        "::OpenFileById("
        "FileIdType"
        "GENERIC_READ | DELETE"
        "PrivateExecutableImageFailure::launch_transition_failed"
        "FILE_SHARE_READ"
        "::SetFileInformationByHandle("
        "FileDispositionInfo"
        "::openat("
        "O_EXCL"
        "O_NOFOLLOW"
        "::unlinkat("
        "::fchmod(image_descriptor.get(), 0500)"
        "status.st_nlink == 0"
        "expected_creation_ticks != 0U"
        "AT_EMPTY_PATH | AT_SYMLINK_NOFOLLOW"
        "OwnedHandle parent_handle("
        "OwnedImageHandle image_handle("
        "OwnedDescriptor parent_descriptor("
        "OwnedLinkedImageDescriptor image_descriptor("
        "image_handle.close_without_delete()"
        "open_path_for_exact_image("
        "image_identity_matches(path_handle.get(), image_identity)"
        "OwnedDirectoryChain directory_chain"
        "directory_chain.lock(stable_image_path->parent_path())"
        "image_identity_matches(final_path_check.get(), image_identity)"
        "FILE_SHARE_READ | FILE_SHARE_DELETE"
        "launch_handle.release()"
        "image_descriptor.release()"
        "native_matches_bytes")
    require_text(${platform_source} "${token}" "exact private-image primitive")
endforeach()

foreach(token IN ITEMS
        "PrivateWindowsBoundedProcessRequest"
        "run_bounded_windows_private_executable("
        "CurrentProcessElevation")
    require_text(${bounded_process_private} "${token}"
        "private exact-image execution seam")
endforeach()
foreach(token IN ITEMS
        "::CreateProcessW("
        "polyglot.process.working_directory_path_unsupported"
        "polyglot.process.executable_path_unsupported"
        "CREATE_SUSPENDED"
        "PROC_THREAD_ATTRIBUTE_JOB_LIST"
        "::TerminateJobObject(job, 1U)"
        "::AssignProcessToJobObject("
        "launch_committed(launch_committed_context)"
        "current_process_elevation()"
        "image.windows_launch_target()"
        "request.transport.executable_path.empty()"
        "class WindowsPipeSecurityAttributes"
        "SE_GROUP_LOGON_ID"
        "WinRestrictedCodeSid"
        "::SetSecurityDescriptorDacl("
        "PROC_THREAD_ATTRIBUTE_HANDLE_LIST")
    require_text(${bounded_process_source} "${token}"
        "bounded Windows exact-image launch contract")
endforeach()
require_text(${platform_source}
    "parent_handle.get(), expected_parent_storage_id"
    "Windows parent identity bracketing")
require_text(${platform_source}
    "parent_descriptor.get(), expected_parent_storage_id"
    "POSIX parent identity bracketing")
require_text(${platform_source}
    "expected_parent_file_id, expected_parent_creation_ticks"
    "stable parent creation-identity bracketing")

foreach(token IN ITEMS
        "class WorkspaceAgentMaterializedProcessImage"
        "const WorkspaceAgentMaterializedProcessImage&) = delete"
        "materialize_process_image(")
    require_text(${environment_header} "${token}" "opaque environment materialization boundary")
endforeach()
foreach(token IN ITEMS
        "receipt->boundary_authority != cleanup_authority_"
        "same_directory_object("
        "left.creation_ticks != 0U"
        "temporary_identity.creation_ticks"
        "materialize_private_executable_image_in_verified_parent("
        "std::span<const std::uint8_t> snapshot")
    require_text(${environment_source} "${token}" "receipt-bound image materialization")
endforeach()

foreach(token IN ITEMS
        "class WorkspaceAgentMaterializedProcessLaunch"
        "const WorkspaceAgentMaterializedProcessLaunch&) = delete"
        "materialize_process_launch_candidate("
        "execute_materialized_process_launch("
        "workspace_agent_process_execution_max_timeout_ms"
        "WorkspaceAgentProcessExecutionControls")
    require_text(${session_header} "${token}" "opaque controller materialization boundary")
endforeach()
foreach(token IN ITEMS
        "process_launch_controller_authority_"
        "candidate.impl_->controller_authority !="
        "Serialize the complete one-attempt materialization with stop/start."
        "candidate.impl_->pins.executable_snapshot_for_materialization()"
        "process_environment_boundary_->materialize_process_image("
        "workspace_agent.process_launch_revalidation_pinning_unavailable"
        "workspace_agent.process_execution_requires_unrestricted_local"
        "workspace_agent.process_execution_elevated_host_denied"
        "allocate_process_execution_operation_id()"
        "execution_working_directory()"
        "next_process_execution_operation_id"
        "WorkspaceAgentSessionEventKind::process_launch_intent"
        "WorkspaceAgentSessionEventKind::process_launch_outcome"
        "workspace_agent.session_reentrant_audit_transition_denied"
        "workspace_agent.session_reentrant_cancellation_transition_denied"
        "release_launch_authority()"
        "run_bounded_windows_private_executable(")
    require_text(${session_source} "${token}" "controller-only one-attempt consumption")
endforeach()

foreach(token IN ITEMS
        "RQ-CF-AGENT-026"
        "RQ-CF-AGENT-027"
        "RQ-CF-AGENT-028"
        "CreateProcessW("
        "CreateFileMappingW("
        "image_path.c_str(), GENERIC_READ | GENERIC_WRITE,"
        "PAGE_READWRITE"
        "MapViewOfFile(mapping, FILE_MAP_WRITE"
        "ERROR_SHARING_VIOLATION"
        "ancestor-directory rename and replacement"
        "image destruction must release every retained ancestor-directory handle"
        "--rq027-child"
        "PrivateExecutableImage"
        "wrong-creation-image"
        "materialize_private_executable_image_in_verified_parent")
    require_text(${platform_test} "${token}" "platform materialization regression")
endforeach()
foreach(token IN ITEMS
        "RQ-CF-AGENT-026"
        "RQ-CF-AGENT-028"
        "WorkspaceAgentMaterializedProcessLaunch"
        "materialize_process_launch_candidate("
        "execute_materialized_process_launch("
        "process_execution_requires_unrestricted_local"
        "revocation lease before the bounded child exits"
        "intent-audit callback must not wait on its own retained launch lease"
        "cancellation callback must not wait on its own retained launch lease"
        "unrelated thread's stop must wait through a slow intent audit"
        "--workspace-agent-non-elevated-test-driver-v1"
        "CreateRestrictedToken("
        "LUA_TOKEN | DISABLE_MAX_PRIVILEGE"
        "execution_configuration()"
        "::GetWindowsDirectoryW("
        "cross-controller"
        "first_controller_operation_id"
        "output_working_directory_matches("
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
