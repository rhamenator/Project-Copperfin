# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

cmake_minimum_required(VERSION 3.20)

if(NOT DEFINED SOURCE_DIR OR "${SOURCE_DIR}" STREQUAL "")
    message(FATAL_ERROR "SOURCE_DIR is required")
endif()

function(require_text relative_path expected)
    file(READ "${SOURCE_DIR}/${relative_path}" contents)
    string(FIND "${contents}" "${expected}" offset)
    if(offset EQUAL -1)
        message(FATAL_ERROR "${relative_path} is missing required process-parser boundary text: ${expected}")
    endif()
endfunction()

require_text(CMakeLists.txt "src/security/workspace_agent_process_parser.cpp")
require_text(include/copperfin/security/workspace_agent_process_parser.h
    "WorkspaceAgentProcessArgumentParserContract")
require_text(include/copperfin/security/workspace_agent_process_parser.h
    "self_contained_parser_image_v1")
require_text(include/copperfin/security/workspace_agent_process_parser.h
    "Trusted product-host configuration only")
require_text(include/copperfin/security/workspace_agent_process_parser.h
    "PhysicalPathIdentity expected_identity")
require_text(include/copperfin/security/workspace_agent_process_parser.h
    "std::string expected_sha256")
require_text(include/copperfin/security/workspace_agent_process_parser.h
    "workspace_agent_maximum_windows_process_parser_image_bytes")
require_text(src/security/workspace_agent_process_parser.cpp
    "captured.identity != binding.expected_identity")
require_text(src/security/workspace_agent_process_parser.cpp
    "constant_time_equal(digest.hex_digest, binding.expected_sha256)")
require_text(src/security/workspace_agent_process_parser.cpp
    "supported_dependency_contract(binding.dependency_contract)")
require_text(src/security/workspace_agent_process_parser.cpp
    "workspace_agent.process_argument_parser_contents_changed")
require_text(src/security/workspace_agent_process_parser.cpp
    "workspace_agent.process_argument_parser_not_trusted")
require_text(src/security/workspace_agent_process_parser.cpp
    "workspace_agent.process_argument_parser_identity_changed")
require_text(src/security/workspace_agent_session.cpp
    "workspace_agent.process_argument_parser_authority_unavailable")
require_text(src/security/workspace_agent_session.cpp
    "final_parser_contract != parser_contract")
require_text(include/copperfin/security/workspace_agent_session.h
    "revalidate_serialized_process_invocation_for_launch")
require_text(src/security/workspace_agent_session.cpp
    "same_serialized_invocation(preliminary, admitted_plan)")
require_text(src/security/workspace_agent_session.cpp
    "read_physically_contained_file_snapshot")
require_text(src/security/workspace_agent_session.cpp
    "final_digest.hex_digest != digest.hex_digest")
require_text(src/security/workspace_agent_session.cpp
    "workspace_agent.process_launch_revalidation_stale_invocation")
require_text(tests/test_workspace_agent_process_parser.cpp "RQ-CF-AGENT-018")
require_text(tests/test_workspace_agent_isolated_environment.cpp
    "workspace_agent.process_argument_parser_authority_unavailable")
require_text(tests/test_workspace_agent_isolated_environment.cpp
    "RQ-CF-AGENT-019")
require_text(.github/workflows/windows-environment-validation.yml
    "test_workspace_agent_process_parser test_workspace_agent_isolated_environment")
require_text(.github/workflows/generated-launcher-validation.yml
    "test_workspace_agent_process_parser test_workspace_agent_isolated_environment")

message(STATUS "Workspace-agent process-parser boundary contract passed")
