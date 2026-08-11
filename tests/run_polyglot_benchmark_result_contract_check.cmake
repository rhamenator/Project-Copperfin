# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

if(NOT DEFINED SOURCE_DIR OR "${SOURCE_DIR}" STREQUAL "")
    message(FATAL_ERROR "SOURCE_DIR is required")
endif()

set(schema_path "${SOURCE_DIR}/docs/contracts/polyglot-benchmark-result-v1.schema.json")
set(result_path "${SOURCE_DIR}/docs/contracts/polyglot-benchmark-result-v1.json")
foreach(path IN ITEMS "${schema_path}" "${result_path}")
    if(NOT EXISTS "${path}")
        message(FATAL_ERROR "Polyglot benchmark contract fixture is missing: ${path}")
    endif()
endforeach()

file(READ "${schema_path}" schema_json)
file(READ "${result_path}" result_json)
string(JSON schema_type ERROR_VARIABLE schema_error TYPE "${schema_json}")
string(JSON result_type ERROR_VARIABLE result_error TYPE "${result_json}")
if(schema_error OR NOT schema_type STREQUAL "OBJECT" OR
   result_error OR NOT result_type STREQUAL "OBJECT")
    message(FATAL_ERROR "Polyglot benchmark contract fixtures must be JSON objects")
endif()

string(JSON schema_version GET "${result_json}" schema_version)
string(JSON result_kind GET "${result_json}" kind)
string(JSON source_commit GET "${result_json}" source_commit)
string(LENGTH "${source_commit}" source_commit_length)
string(JSON measurement_count LENGTH "${result_json}" measurements)
string(JSON promotion GET "${result_json}" recommendation automatic_promotion)
if(NOT schema_version EQUAL 1 OR
   NOT result_kind STREQUAL "copperfin-polyglot-benchmark-result" OR
   NOT source_commit_length EQUAL 40 OR
   NOT source_commit MATCHES "^[0-9a-f]+$" OR
   NOT measurement_count EQUAL 3 OR promotion)
    message(FATAL_ERROR "Polyglot benchmark result identity or promotion contract is invalid")
endif()

set(expected_implementations direct-cpp cpp-dotnet-wrapper csharp-service)
foreach(index RANGE 0 2)
    list(GET expected_implementations ${index} expected)
    string(JSON implementation GET "${result_json}" measurements ${index} implementation)
    string(JSON samples GET "${result_json}" measurements ${index} sample_count)
    string(JSON failures GET "${result_json}" measurements ${index} failure_count)
    string(JSON mismatches GET "${result_json}" measurements ${index} parity_mismatch_count)
    if(NOT implementation STREQUAL expected OR
       NOT samples EQUAL 9 OR NOT failures EQUAL 0 OR NOT mismatches EQUAL 0)
        message(FATAL_ERROR "Polyglot benchmark evidence is incomplete for ${expected}")
    endif()
endforeach()

message(STATUS "Polyglot benchmark result v1 schema and evidence passed")
