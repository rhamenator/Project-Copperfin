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
        message(FATAL_ERROR "${relative_path} is missing required PE-image boundary text: ${expected}")
    endif()
endfunction()

require_text(CMakeLists.txt "src/platform/windows_pe_image.cpp")
require_text(CMakeLists.txt "cf_platform_support")
require_text(include/copperfin/platform/windows_pe_image.h
    "inspect_windows_pe_image")
require_text(include/copperfin/platform/windows_pe_image.h
    "windows_pe_image_is_launch_compatible")
require_text(src/platform/windows_pe_image.cpp
    "FILE_SHARE_READ | FILE_SHARE_DELETE")
require_text(src/platform/windows_pe_image.cpp
    "WindowsPeReadSharing::allow_write_sharing")
require_text(src/platform/windows_pe_image.cpp
    "dynamic_library_flag")
require_text(src/platform/windows_pe_image.cpp
    "section_executable_flag")
require_text(src/security/workspace_agent_process_containment.cpp
    "workspace_agent.process_executable_image_invalid")
require_text(src/security/workspace_agent_process_containment.cpp
    "workspace_agent.process_executable_image_not_launchable")
require_text(src/security/workspace_agent_process_containment.cpp
    "workspace_agent.process_executable_machine_incompatible")
require_text(src/security/workspace_agent_process_containment.cpp
    "workspace_agent.process_executable_changed_during_image_inspection")
require_text(src/runtime/managed_pe_image.cpp
    "platform::WindowsPeReadSharing::allow_write_sharing")
require_text(tests/test_windows_pe_image.cpp "RQ-CF-AGENT-017")
require_text(tests/test_windows_pe_image.cpp
    "WindowsPeReadSharing::allow_write_sharing")
require_text(tests/test_workspace_agent_process_containment.cpp
    "workspace_agent.process_executable_image_invalid")
require_text(tests/test_workspace_agent_isolated_environment.cpp
    "Windows PE fixture copy failed")
require_text(.github/workflows/windows-environment-validation.yml
    "test_windows_pe_image test_windows_com_event_adapter test_workspace_agent_process_containment")

message(STATUS "Windows PE-image boundary contract passed")
