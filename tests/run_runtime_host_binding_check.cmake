if(NOT DEFINED RUNTIME_HOST_EXECUTABLE)
    message(FATAL_ERROR "RUNTIME_HOST_EXECUTABLE is required for the runtime-host binding check.")
endif()

set(temp_root "$ENV{TMPDIR}")
if(temp_root STREQUAL "")
    set(temp_root "$ENV{TEMP}")
endif()
if(temp_root STREQUAL "")
    set(temp_root "$ENV{TMP}")
endif()
if(temp_root STREQUAL "")
    set(temp_root "/tmp")
endif()

string(TIMESTAMP timestamp "%Y%m%d%H%M%S" UTC)
set(test_root "${temp_root}/copperfin_runtime_host_binding_${timestamp}")
set(builder_root "${test_root}/builder/DemoApp")
set(deployed_root "${test_root}/deployed")
set(content_root "${deployed_root}/content")
set(plugin_root "${content_root}/plugins")

file(REMOVE_RECURSE "${test_root}")
file(MAKE_DIRECTORY "${builder_root}")
file(MAKE_DIRECTORY "${plugin_root}")

execute_process(
    COMMAND ${CMAKE_COMMAND} -E copy "${RUNTIME_HOST_EXECUTABLE}" "${deployed_root}/copperfin_runtime_host.exe"
    RESULT_VARIABLE copy_host_result
)
if(NOT copy_host_result EQUAL 0)
    message(FATAL_ERROR "Failed to copy runtime host into the deployed package.")
endif()

execute_process(
    COMMAND ${CMAKE_COMMAND} -E copy "${RUNTIME_HOST_EXECUTABLE}" "${deployed_root}/DemoApp.exe"
    RESULT_VARIABLE copy_entrypoint_result
)
if(NOT copy_entrypoint_result EQUAL 0)
    message(FATAL_ERROR "Failed to copy packaged entrypoint executable.")
endif()

if(UNIX)
    execute_process(COMMAND chmod +x "${deployed_root}/copperfin_runtime_host.exe")
    execute_process(COMMAND chmod +x "${deployed_root}/DemoApp.exe")
endif()

file(WRITE "${content_root}/main.prg" "RETURN\n")
file(WRITE "${plugin_root}/helper.dll" "plugin-payload")

file(SHA256 "${deployed_root}/copperfin_runtime_host.exe" runtime_host_hash)
file(SHA256 "${deployed_root}/DemoApp.exe" native_entrypoint_hash)
file(SHA256 "${plugin_root}/helper.dll" helper_payload_hash)

set(manifest_text
"manifest_version=1
project_title=DemoApp
project_path=${builder_root}/demo.pjx
package_root=${builder_root}
content_root=${builder_root}/content
working_directory=${builder_root}/content
startup_item=main.prg
startup_source=${builder_root}/content/main.prg
configuration=debug
security_enabled=true
security_role=developer
security_mode=native
audit_log_path=${builder_root}/security_audit.log
runtime_host_sha256=${runtime_host_hash}
launcher_mode=native_runtime_host
launcher_fallback=none
extension_payload=${builder_root}/copperfin_runtime_host.exe|${runtime_host_hash}
extension_payload=${builder_root}/DemoApp.exe|${native_entrypoint_hash}
extension_payload=${builder_root}/content/plugins/helper.dll|${helper_payload_hash}
")
file(WRITE "${deployed_root}/app.cfmanifest" "${manifest_text}")

execute_process(
    COMMAND "${deployed_root}/DemoApp.exe"
    WORKING_DIRECTORY "${test_root}"
    RESULT_VARIABLE run_result
    OUTPUT_VARIABLE run_output
    ERROR_VARIABLE run_error
)

if(NOT run_result EQUAL 0)
    message(FATAL_ERROR "Runtime host binding smoke failed.\nstdout:\n${run_output}\nstderr:\n${run_error}")
endif()

if(NOT run_output MATCHES "runtime\\.completed: true")
    message(FATAL_ERROR "Runtime host binding smoke did not complete successfully.\nstdout:\n${run_output}")
endif()

string(REPLACE "\\" "\\\\" escaped_content_root "${content_root}")
if(NOT run_output MATCHES "working\\.directory: ${escaped_content_root}")
    message(FATAL_ERROR "Runtime host binding smoke did not report the rebound package content root.\nstdout:\n${run_output}")
endif()

if(NOT EXISTS "${deployed_root}/security_audit.log")
    message(FATAL_ERROR "Runtime host binding smoke did not write the rebound package-local audit log.")
endif()

file(REMOVE_RECURSE "${test_root}")
