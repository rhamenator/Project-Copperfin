# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

cmake_minimum_required(VERSION 3.20)

if(NOT DEFINED INVENTORY_PATH OR "${INVENTORY_PATH}" STREQUAL "")
    message(FATAL_ERROR "INVENTORY_PATH is required")
endif()
if(NOT EXISTS "${INVENTORY_PATH}")
    message(FATAL_ERROR "Native test isolation inventory is missing: ${INVENTORY_PATH}")
endif()
foreach(required_path_variable IN ITEMS
        ISOLATION_SOURCE_PATH TEST_REGISTRATION_SOURCE_PATH)
    if(NOT DEFINED ${required_path_variable} OR
       "${${required_path_variable}}" STREQUAL "" OR
       NOT EXISTS "${${required_path_variable}}")
        message(FATAL_ERROR "${required_path_variable} must name an existing file")
    endif()
endforeach()

file(STRINGS "${INVENTORY_PATH}" inventory_lines)
list(LENGTH inventory_lines inventory_line_count)
if(inventory_line_count LESS 4)
    message(FATAL_ERROR "Native test isolation inventory contains no test rows")
endif()

list(POP_FRONT inventory_lines isolation_source_record)
list(POP_FRONT inventory_lines test_registration_source_record)
list(POP_FRONT inventory_lines inventory_header)
file(SHA256 "${ISOLATION_SOURCE_PATH}" current_isolation_source_sha256)
file(SHA256 "${TEST_REGISTRATION_SOURCE_PATH}"
    current_test_registration_source_sha256)
if(NOT isolation_source_record STREQUAL
   "isolation_source_sha256\t${current_isolation_source_sha256}")
    message(FATAL_ERROR
        "Native test isolation inventory is stale for ${ISOLATION_SOURCE_PATH}")
endif()
if(NOT test_registration_source_record STREQUAL
   "test_registration_source_sha256\t${current_test_registration_source_sha256}")
    message(FATAL_ERROR
        "Native test isolation inventory is stale for ${TEST_REGISTRATION_SOURCE_PATH}")
endif()
if(NOT inventory_header STREQUAL
   "schema_version\ttest\trun_serial\tresource_lock\tlabels")
    message(FATAL_ERROR "Native test isolation inventory header is invalid")
endif()

function(require_allowed_axis_value test_name labels_csv prefix)
    set(allowed_values ${ARGN})
    string(REPLACE "," ";" axis_labels "${labels_csv}")
    string(LENGTH "${prefix}" prefix_length)
    foreach(axis_label IN LISTS axis_labels)
        string(FIND "${axis_label}" "${prefix}" prefix_index)
        if(prefix_index EQUAL 0)
            string(SUBSTRING "${axis_label}" ${prefix_length} -1 axis_value)
            if(NOT "${axis_value}" IN_LIST allowed_values)
                message(FATAL_ERROR
                    "Native test ${test_name} has invalid ${prefix}${axis_value}")
            endif()
            return()
        endif()
    endforeach()
    message(FATAL_ERROR "Native test ${test_name} lacks required axis ${prefix}")
endfunction()

set(required_label_prefixes
    "copperfin-isolation:filesystem="
    "copperfin-isolation:environment="
    "copperfin-isolation:child-processes="
    "copperfin-isolation:network="
    "copperfin-isolation:samples="
    "copperfin-isolation:platform="
    "copperfin-isolation:resources="
    "copperfin-isolation:audit="
    "copperfin-isolation:schedule="
)
set(test_names)
set(pending_tests)
foreach(inventory_line IN LISTS inventory_lines)
    string(REPLACE "\t" ";" columns "${inventory_line}")
    list(LENGTH columns column_count)
    if(NOT column_count EQUAL 5)
        message(FATAL_ERROR "Native test isolation row must contain five columns: ${inventory_line}")
    endif()

    list(GET columns 0 schema_version)
    list(GET columns 1 test_name)
    list(GET columns 2 run_serial)
    list(GET columns 3 resource_lock)
    list(GET columns 4 labels_csv)
    if(NOT schema_version STREQUAL "1")
        message(FATAL_ERROR "Native test ${test_name} has unsupported isolation schema ${schema_version}")
    endif()
    if(NOT test_name MATCHES "^test_[A-Za-z0-9_]+$")
        message(FATAL_ERROR "Native test isolation inventory contains an invalid test name: ${test_name}")
    endif()
    if(test_name IN_LIST test_names)
        message(FATAL_ERROR "Native test isolation inventory duplicates ${test_name}")
    endif()
    list(APPEND test_names "${test_name}")
    if(NOT run_serial STREQUAL "TRUE" AND NOT run_serial STREQUAL "FALSE")
        message(FATAL_ERROR "Native test ${test_name} has invalid RUN_SERIAL value ${run_serial}")
    endif()

    string(REPLACE "," ";" labels "${labels_csv}")
    foreach(required_prefix IN LISTS required_label_prefixes)
        set(matching_labels)
        foreach(label IN LISTS labels)
            string(FIND "${label}" "${required_prefix}" prefix_index)
            if(prefix_index EQUAL 0)
                list(APPEND matching_labels "${label}")
            endif()
        endforeach()
        list(LENGTH matching_labels matching_label_count)
        if(NOT matching_label_count EQUAL 1)
            message(FATAL_ERROR
                "Native test ${test_name} must contain exactly one ${required_prefix} label")
        endif()
    endforeach()

    require_allowed_axis_value("${test_name}" "${labels_csv}"
        "copperfin-isolation:filesystem="
        none read-only process-owned test-owned-unique fixed-build-tree
        fixed-resource-locked fixed-shared-family shared-build-tree unverified)
    require_allowed_axis_value("${test_name}" "${labels_csv}"
        "copperfin-isolation:environment="
        none scoped-process child-scoped unverified)
    require_allowed_axis_value("${test_name}" "${labels_csv}"
        "copperfin-isolation:child-processes="
        none bounded bounded-optional unverified)
    require_allowed_axis_value("${test_name}" "${labels_csv}"
        "copperfin-isolation:network="
        none disabled-probes possible-package-restore unverified)
    require_allowed_axis_value("${test_name}" "${labels_csv}"
        "copperfin-isolation:samples="
        none read-only owned-copy unverified)
    require_allowed_axis_value("${test_name}" "${labels_csv}"
        "copperfin-isolation:platform="
        portable configured powershell-conditional dotnet-conditional
        toolchain-conditional)
    require_allowed_axis_value("${test_name}" "${labels_csv}"
        "copperfin-isolation:resources=" none lock unverified)
    require_allowed_axis_value("${test_name}" "${labels_csv}"
        "copperfin-isolation:audit=" complete pending)
    require_allowed_axis_value("${test_name}" "${labels_csv}"
        "copperfin-isolation:schedule=" parallel-safe serial)

    if(run_serial STREQUAL "TRUE")
        if(NOT "copperfin-isolation:schedule=serial" IN_LIST labels)
            message(FATAL_ERROR "Serial native test ${test_name} lacks its schedule label")
        endif()
    elseif(NOT "copperfin-isolation:schedule=parallel-safe" IN_LIST labels)
        message(FATAL_ERROR "Parallel native test ${test_name} lacks its schedule label")
    endif()

    if("copperfin-isolation:resources=lock" IN_LIST labels)
        if(resource_lock STREQUAL "none")
            message(FATAL_ERROR "Locked native test ${test_name} lacks its RESOURCE_LOCK")
        endif()
    elseif(NOT resource_lock STREQUAL "none")
        message(FATAL_ERROR
            "Native test ${test_name} has unexpected RESOURCE_LOCK ${resource_lock}")
    endif()

    if("copperfin-isolation:audit=pending" IN_LIST labels)
        list(APPEND pending_tests "${test_name}")
    else()
        foreach(label IN LISTS labels)
            if(label MATCHES "^copperfin-isolation:[^=]+=unverified$")
                message(FATAL_ERROR
                    "Completed native test audit ${test_name} retains ${label}")
            endif()
        endforeach()
    endif()
endforeach()

if(pending_tests)
    list(JOIN pending_tests ", " pending_test_list)
    message(FATAL_ERROR
        "Native test isolation audit is incomplete for: ${pending_test_list}")
endif()

list(LENGTH test_names test_count)
message(STATUS "Native test isolation inventory contract passed for ${test_count} tests")
