# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

if(NOT DEFINED TEST_ROOT OR "${TEST_ROOT}" STREQUAL "")
    message(FATAL_ERROR "TEST_ROOT is required")
endif()

file(REMOVE_RECURSE "${TEST_ROOT}")
set(locale_root "${TEST_ROOT}/share/copperfin/locales")
foreach(locale IN ITEMS en-US es-419 pt-BR qps-ploc)
    file(MAKE_DIRECTORY "${locale_root}/${locale}")
    file(WRITE "${locale_root}/${locale}/strings.json" "{\"test\":\"${locale}\"}\n")
endforeach()

execute_process(
    COMMAND "${CMAKE_COMMAND}"
        "-DINSTALL_ROOT=${TEST_ROOT}"
        -P "${CMAKE_CURRENT_LIST_DIR}/run_locale_catalog_install_contract_check.cmake"
    RESULT_VARIABLE complete_result
)
if(NOT complete_result EQUAL 0)
    message(FATAL_ERROR "Complete locale catalog fixture should pass the install contract")
endif()

file(REMOVE "${locale_root}/pt-BR/strings.json")
execute_process(
    COMMAND "${CMAKE_COMMAND}"
        "-DINSTALL_ROOT=${TEST_ROOT}"
        -P "${CMAKE_CURRENT_LIST_DIR}/run_locale_catalog_install_contract_check.cmake"
    RESULT_VARIABLE incomplete_result
)
if(incomplete_result EQUAL 0)
    message(FATAL_ERROR "Incomplete locale catalog fixture should fail the install contract")
endif()

file(REMOVE_RECURSE "${TEST_ROOT}")
message(STATUS "Locale catalog install contract test passed")
