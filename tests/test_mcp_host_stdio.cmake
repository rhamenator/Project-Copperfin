# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

if(NOT DEFINED MCP_HOST OR NOT DEFINED TEST_WORK_DIR)
    message(FATAL_ERROR "MCP_HOST and TEST_WORK_DIR are required")
endif()

set(input_path "${TEST_WORK_DIR}/mcp-host-stdio-input.jsonl")
file(WRITE "${input_path}"
    "{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"server/discover\",\"params\":{\"_meta\":{\"io.modelcontextprotocol/protocolVersion\":\"2026-07-28\",\"io.modelcontextprotocol/clientCapabilities\":{}}}}\n"
    "{\"jsonrpc\":\"2.0\",\"method\":\"notifications/cancelled\",\"params\":{}}\n"
    "{\"jsonrpc\":\"2.0\",\"id\":2,\"method\":\"tools/list\",\"params\":{\"_meta\":{\"io.modelcontextprotocol/protocolVersion\":\"2026-07-28\",\"io.modelcontextprotocol/clientCapabilities\":{}}}}\n"
    "{\"jsonrpc\":\"2.0\",\"id\":3,\"method\":\"tools/call\",\"params\":{\"_meta\":{\"io.modelcontextprotocol/protocolVersion\":\"2026-07-28\",\"io.modelcontextprotocol/clientCapabilities\":{}},\"name\":\"copperfin.parse_dbf_header\",\"arguments\":{\"headerHex\":\"037e010201000000200001000000000000000000000000000000000001030000\"}}}\n")

execute_process(
    COMMAND "${CMAKE_COMMAND}" -E env
        "COPPERFIN_SECURITY_ROLE=runtime-operator" "${MCP_HOST}"
    INPUT_FILE "${input_path}"
    OUTPUT_VARIABLE stdout_text
    ERROR_VARIABLE stderr_text
    RESULT_VARIABLE exit_code
    TIMEOUT 10
)
file(REMOVE "${input_path}")

if(NOT exit_code EQUAL 0)
    message(FATAL_ERROR "MCP host exited ${exit_code}: ${stderr_text}")
endif()
string(REGEX MATCHALL "[^\n]+" output_lines "${stdout_text}")
list(LENGTH output_lines output_count)
if(NOT output_count EQUAL 3)
    message(FATAL_ERROR "Expected three response lines, got ${output_count}: ${stdout_text}")
endif()
foreach(output_line IN LISTS output_lines)
    if(NOT output_line MATCHES "^\\{\"jsonrpc\":\"2\\.0\",\"id\":[123],\"result\":")
        message(FATAL_ERROR "Non-protocol stdout line: ${output_line}")
    endif()
endforeach()
if(NOT stdout_text MATCHES "serverInfo" OR
   NOT stdout_text MATCHES "copperfin\\.parse_dbf_header")
    message(FATAL_ERROR "Expected discovery and tool-list output: ${stdout_text}")
endif()
if(NOT stderr_text MATCHES
    "copperfin\\.audit event=ai\\.mcp_invoked tool=copperfin\\.parse_dbf_header outcome=success")
    message(FATAL_ERROR "Expected content-free MCP tool audit event: ${stderr_text}")
endif()

execute_process(
    COMMAND "${CMAKE_COMMAND}" -E env
        --unset=COPPERFIN_SECURITY_ROLE "${MCP_HOST}"
    INPUT_FILE "${CMAKE_CURRENT_LIST_FILE}"
    OUTPUT_VARIABLE denied_stdout
    ERROR_VARIABLE denied_stderr
    RESULT_VARIABLE denied_exit_code
    TIMEOUT 10
)
if(NOT denied_exit_code EQUAL 7 OR NOT denied_stdout STREQUAL "" OR
   NOT denied_stderr MATCHES
       "mcp\\.security\\.permission_denied permission=ai\\.mcp")
    message(FATAL_ERROR
        "Default MCP role did not fail closed: exit=${denied_exit_code}; stdout=${denied_stdout}; stderr=${denied_stderr}")
endif()
