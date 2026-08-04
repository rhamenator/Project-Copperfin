# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only

if(NOT DEFINED SOURCE_DIR OR "${SOURCE_DIR}" STREQUAL "")
    message(FATAL_ERROR "SOURCE_DIR is required")
endif()

set(schema_path "${SOURCE_DIR}/docs/contracts/polyglot-route-registry-v1.schema.json")
set(example_path "${SOURCE_DIR}/docs/contracts/polyglot-route-registry-v1.json")
foreach(path IN ITEMS "${schema_path}" "${example_path}")
    if(NOT EXISTS "${path}")
        message(FATAL_ERROR "Polyglot route contract fixture is missing: ${path}")
    endif()
endforeach()

file(READ "${schema_path}" schema_json)
file(READ "${example_path}" example_json)
string(JSON schema_type ERROR_VARIABLE schema_error TYPE "${schema_json}")
string(JSON example_type ERROR_VARIABLE example_error TYPE "${example_json}")
if(schema_error OR NOT schema_type STREQUAL "OBJECT" OR
   example_error OR NOT example_type STREQUAL "OBJECT")
    message(FATAL_ERROR "Polyglot route contract fixtures must be JSON objects")
endif()

string(JSON required_count ERROR_VARIABLE required_error LENGTH "${schema_json}" required)
if(required_error OR required_count LESS 2)
    message(FATAL_ERROR "Polyglot route schema must require registry_version and routes")
endif()
string(JSON example_version ERROR_VARIABLE version_error GET "${example_json}" registry_version)
if(version_error OR NOT example_version STREQUAL "1.0")
    message(FATAL_ERROR "Polyglot route example must declare registry_version 1.0")
endif()
string(JSON routes_type ERROR_VARIABLE routes_error TYPE "${example_json}" routes)
string(JSON route_count ERROR_VARIABLE count_error LENGTH "${example_json}" routes)
if(routes_error OR NOT routes_type STREQUAL "ARRAY" OR count_error OR route_count LESS 2)
    message(FATAL_ERROR "Polyglot route example must contain at least two route entries")
endif()
string(JSON first_id_type ERROR_VARIABLE first_id_error TYPE "${example_json}" routes 0 capability_id)
string(JSON first_state_type ERROR_VARIABLE first_state_error TYPE "${example_json}" routes 0 state)
if(first_id_error OR NOT first_id_type STREQUAL "STRING" OR
   first_state_error OR NOT first_state_type STREQUAL "STRING")
    message(FATAL_ERROR "Polyglot route example entries must have typed machine fields")
endif()

message(STATUS "Polyglot Route Registry v1 schema and example passed")
