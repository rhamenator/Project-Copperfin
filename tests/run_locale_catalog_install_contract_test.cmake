# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

cmake_minimum_required(VERSION 3.21)

if(NOT DEFINED TEST_ROOT OR "${TEST_ROOT}" STREQUAL "")
    message(FATAL_ERROR "TEST_ROOT is required")
endif()

file(REMOVE_RECURSE "${TEST_ROOT}")
set(source_root "${TEST_ROOT}/source")
set(source_locales "${source_root}/resources/locales")
set(installed_root "${TEST_ROOT}/installed")
set(installed_locales "${installed_root}/share/copperfin/locales")
set(inventory "version 1\nen-US source\nes-419 unreviewed\npt-BR unreviewed\nfr-FR unreviewed\nqps-ploc pseudo\n")
file(MAKE_DIRECTORY "${source_locales}" "${installed_locales}")
file(WRITE "${source_locales}/inventory.txt" "${inventory}")
file(WRITE "${installed_locales}/inventory.txt" "${inventory}")
foreach(locale IN ITEMS en-US es-419 pt-BR fr-FR qps-ploc)
    file(MAKE_DIRECTORY "${source_locales}/${locale}" "${installed_locales}/${locale}")
    file(WRITE "${source_locales}/${locale}/strings.json" "{\"test\":\"${locale}\"}\n")
    file(COPY_FILE "${source_locales}/${locale}/strings.json" "${installed_locales}/${locale}/strings.json")
endforeach()

function(check_install should_pass case_name expected_error)
    execute_process(
        COMMAND "${CMAKE_COMMAND}"
            "-DINSTALL_ROOT=${installed_root}"
            "-DSOURCE_DIR=${source_root}"
            -P "${CMAKE_CURRENT_LIST_DIR}/run_locale_catalog_install_contract_check.cmake"
        RESULT_VARIABLE result
        OUTPUT_VARIABLE output ERROR_VARIABLE error
    )
    if(should_pass AND NOT result EQUAL 0)
        message(FATAL_ERROR "Valid locale install fixture failed: ${case_name}")
    elseif(NOT should_pass AND result EQUAL 0)
        message(FATAL_ERROR "Invalid locale install fixture passed: ${case_name}")
    elseif(NOT should_pass)
        string(FIND "${output}${error}" "${expected_error}" error_index)
        if(error_index EQUAL -1)
            message(FATAL_ERROR "${case_name} failed for the wrong reason: ${output}${error}")
        endif()
    endif()
endfunction()

check_install(TRUE "valid fifth locale" "")
execute_process(
    COMMAND "${CMAKE_COMMAND}" "-DINSTALL_ROOT=${installed_root}"
        -P "${CMAKE_CURRENT_LIST_DIR}/run_locale_catalog_install_contract_check.cmake"
    RESULT_VARIABLE missing_source_result ERROR_VARIABLE missing_source_error
)
if(missing_source_result EQUAL 0 OR NOT missing_source_error MATCHES "SOURCE_DIR is required")
    message(FATAL_ERROR "Omitted SOURCE_DIR did not fail closed")
endif()
file(REMOVE "${source_locales}/fr-FR/strings.json")
check_install(FALSE "missing declared source catalog" "Declared source catalog is missing")
file(WRITE "${source_locales}/fr-FR/strings.json" "{\"test\":\"fr-FR\"}\n")
file(APPEND "${installed_locales}/inventory.txt" "# drift\n")
check_install(FALSE "changed installed inventory" "Installed locale inventory differs from source")
file(WRITE "${installed_locales}/inventory.txt" "${inventory}")
file(REMOVE "${installed_locales}/fr-FR/strings.json")
check_install(FALSE "missing fifth locale" "is missing or not a regular file")
file(WRITE "${installed_locales}/fr-FR/strings.json" "")
check_install(FALSE "empty fifth locale" "is empty")
file(COPY_FILE "${source_locales}/fr-FR/strings.json" "${installed_locales}/fr-FR/strings.json")
file(WRITE "${installed_locales}/fr-FR/strings.json" "{\"test\":\"changed\"}\n")
check_install(FALSE "changed fifth locale" "differs from source")
file(COPY_FILE "${source_locales}/fr-FR/strings.json" "${installed_locales}/fr-FR/strings.json")
file(MAKE_DIRECTORY "${installed_locales}/undeclared")
check_install(FALSE "undeclared locale directory" "Undeclared or malformed installed locale")
file(REMOVE_RECURSE "${installed_locales}/undeclared")
file(WRITE "${installed_locales}/fr-FR/notes.txt" "stray\n")
check_install(FALSE "stray per-locale file" "Undeclared installed locale content")
file(REMOVE "${installed_locales}/fr-FR/notes.txt")
file(APPEND "${installed_locales}/inventory.txt" "../../escape unreviewed\n")
file(APPEND "${source_locales}/inventory.txt" "../../escape unreviewed\n")
check_install(FALSE "malformed inventory" "Malformed locale inventory entry")
file(REMOVE_RECURSE "${TEST_ROOT}")
message(STATUS "Locale catalog install contract test passed")
