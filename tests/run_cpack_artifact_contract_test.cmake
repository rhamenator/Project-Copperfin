# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

if(NOT DEFINED TEST_ROOT OR "${TEST_ROOT}" STREQUAL "")
    message(FATAL_ERROR "TEST_ROOT is required")
endif()

file(REMOVE_RECURSE "${TEST_ROOT}")
file(MAKE_DIRECTORY "${TEST_ROOT}/build/package")
file(WRITE "${TEST_ROOT}/build/CopperfinPackageVersion.txt" "1.2.3\n")
file(WRITE "${TEST_ROOT}/build/package/copperfin-1.2.3-Linux.deb" "package\n")
file(WRITE "${TEST_ROOT}/build/package/copperfin-1.2.3-Linux.tar.gz" "package\n")

get_filename_component(source_root "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
file(RELATIVE_PATH relative_test_root "${source_root}" "${TEST_ROOT}")
execute_process(
    COMMAND "${CMAKE_COMMAND}"
        "-DCOPPERFIN_ARTIFACT_DIR:PATH=${relative_test_root}/build/package"
        "-DCOPPERFIN_VERSION_FILE:FILEPATH=${relative_test_root}/build/CopperfinPackageVersion.txt"
        "-DCOPPERFIN_EXPECTED_ARTIFACT_SUFFIXES=Linux.deb;Linux.tar.gz"
        -P "${CMAKE_CURRENT_LIST_DIR}/run_cpack_artifact_contract_check.cmake"
    WORKING_DIRECTORY "${source_root}"
    RESULT_VARIABLE complete_result)
if(NOT complete_result EQUAL 0)
    message(FATAL_ERROR "Relative CPack artifact fixture should pass")
endif()

file(REMOVE "${TEST_ROOT}/build/CopperfinPackageVersion.txt")
execute_process(
    COMMAND "${CMAKE_COMMAND}"
        "-DCOPPERFIN_ARTIFACT_DIR:PATH=${relative_test_root}/build/package"
        "-DCOPPERFIN_VERSION_FILE:FILEPATH=${relative_test_root}/build/CopperfinPackageVersion.txt"
        "-DCOPPERFIN_EXPECTED_ARTIFACT_SUFFIXES=Linux.deb;Linux.tar.gz"
        -P "${CMAKE_CURRENT_LIST_DIR}/run_cpack_artifact_contract_check.cmake"
    WORKING_DIRECTORY "${source_root}"
    RESULT_VARIABLE missing_version_result)
if(missing_version_result EQUAL 0)
    message(FATAL_ERROR "Missing CPack version file should fail the contract")
endif()

file(MAKE_DIRECTORY "${TEST_ROOT}/build")
file(WRITE "${TEST_ROOT}/build/CopperfinPackageVersion.txt" "not-a-version\n")
execute_process(
    COMMAND "${CMAKE_COMMAND}"
        "-DCOPPERFIN_ARTIFACT_DIR:PATH=${relative_test_root}/build/package"
        "-DCOPPERFIN_VERSION_FILE:FILEPATH=${relative_test_root}/build/CopperfinPackageVersion.txt"
        "-DCOPPERFIN_EXPECTED_ARTIFACT_SUFFIXES=Linux.deb;Linux.tar.gz"
        -P "${CMAKE_CURRENT_LIST_DIR}/run_cpack_artifact_contract_check.cmake"
    WORKING_DIRECTORY "${source_root}"
    RESULT_VARIABLE invalid_version_result)
if(invalid_version_result EQUAL 0)
    message(FATAL_ERROR "Invalid CPack version file should fail the contract")
endif()

file(REMOVE_RECURSE "${TEST_ROOT}")
message(STATUS "CPack artifact contract test passed")
