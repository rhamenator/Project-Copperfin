# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

foreach(required_variable IN ITEMS TEST_ROOT TEST_BINARY_DIR)
    if(NOT DEFINED ${required_variable} OR "${${required_variable}}" STREQUAL "")
        message(FATAL_ERROR "${required_variable} is required")
    endif()
endforeach()

file(REMOVE_RECURSE "${TEST_ROOT}" "${TEST_BINARY_DIR}")
file(MAKE_DIRECTORY
    "${TEST_ROOT}/bin/studio"
    "${TEST_ROOT}/share/copperfin/locales/en-US"
    "${TEST_ROOT}/share/copperfin/locales/es-419"
    "${TEST_ROOT}/share/copperfin/locales/pt-BR"
    "${TEST_ROOT}/share/copperfin/locales/qps-ploc"
    "${TEST_BINARY_DIR}")
file(WRITE "${TEST_BINARY_DIR}/CMakeCache.txt"
    "COPPERFIN_NATIVE_POINTER_SIZE:INTERNAL=8\nCMAKE_GENERATOR_PLATFORM:INTERNAL=x64\n")
foreach(path IN ITEMS
        "bin/studio/Copperfin.Studio.exe"
        "bin/studio/Copperfin.Studio.exe.config"
        "bin/copperfin_studio_host.exe"
        "bin/copperfin_mcp_host.exe"
        "share/copperfin/locales/en-US/strings.json"
        "share/copperfin/locales/es-419/strings.json"
        "share/copperfin/locales/pt-BR/strings.json"
        "share/copperfin/locales/qps-ploc/strings.json")
    file(WRITE "${TEST_ROOT}/${path}" "contract fixture\n")
endforeach()

get_filename_component(source_root "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
file(RELATIVE_PATH relative_test_root "${source_root}" "${TEST_ROOT}")
file(RELATIVE_PATH relative_binary_dir "${source_root}" "${TEST_BINARY_DIR}")

execute_process(
    COMMAND "${CMAKE_COMMAND}"
        -DWIN32=TRUE
        "-DBINARY_DIR=${relative_binary_dir}"
        "-DINSTALL_ROOT=${relative_test_root}"
        -P "${CMAKE_CURRENT_LIST_DIR}/run_studio_install_contract_check.cmake"
    WORKING_DIRECTORY "${source_root}"
    RESULT_VARIABLE complete_result)
if(NOT complete_result EQUAL 0)
    message(FATAL_ERROR "Relative install-root Studio contract fixture should pass")
endif()

file(REMOVE "${TEST_ROOT}/bin/copperfin_mcp_host.exe")
execute_process(
    COMMAND "${CMAKE_COMMAND}"
        -DWIN32=TRUE
        "-DBINARY_DIR=${relative_binary_dir}"
        "-DINSTALL_ROOT=${relative_test_root}"
        -P "${CMAKE_CURRENT_LIST_DIR}/run_studio_install_contract_check.cmake"
    WORKING_DIRECTORY "${source_root}"
    RESULT_VARIABLE missing_mcp_host_result)
if(missing_mcp_host_result EQUAL 0)
    message(FATAL_ERROR "Studio contract fixture without the installed MCP host should fail")
endif()
file(WRITE "${TEST_ROOT}/bin/copperfin_mcp_host.exe" "contract fixture\n")

file(WRITE "${TEST_ROOT}/bin/studio/unexpected.txt" "unexpected\n")
execute_process(
    COMMAND "${CMAKE_COMMAND}"
        -DWIN32=TRUE
        "-DBINARY_DIR=${relative_binary_dir}"
        "-DINSTALL_ROOT=${relative_test_root}"
        -P "${CMAKE_CURRENT_LIST_DIR}/run_studio_install_contract_check.cmake"
    WORKING_DIRECTORY "${source_root}"
    RESULT_VARIABLE unexpected_result)
if(unexpected_result EQUAL 0)
    message(FATAL_ERROR "Unexpected Studio install fixture should fail")
endif()

file(REMOVE_RECURSE "${TEST_ROOT}" "${TEST_BINARY_DIR}")
message(STATUS "Studio install contract test passed")
