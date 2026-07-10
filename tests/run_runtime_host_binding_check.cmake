if(NOT DEFINED RUNTIME_HOST_EXECUTABLE)
    message(FATAL_ERROR "RUNTIME_HOST_EXECUTABLE is required for the runtime-host binding check.")
endif()
if(NOT DEFINED LOCALE_ROOT OR NOT IS_DIRECTORY "${LOCALE_ROOT}")
    message(FATAL_ERROR "LOCALE_ROOT must name the product localization catalog directory.")
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
string(REGEX REPLACE "[/\\\\]+$" "" temp_root "${temp_root}")

string(TIMESTAMP timestamp "%Y%m%d%H%M%S" UTC)
set(test_root "${temp_root}/copperfin_runtime_host_binding_${timestamp}")
set(builder_root "${test_root}/builder/DemoApp")
set(deployed_root "${test_root}/deployed")
set(content_root "${deployed_root}/content")
set(plugin_root "${content_root}/plugins")
get_filename_component(runtime_host_file_name "${RUNTIME_HOST_EXECUTABLE}" NAME)
get_filename_component(runtime_host_extension "${RUNTIME_HOST_EXECUTABLE}" EXT)
set(packaged_runtime_host "${deployed_root}/${runtime_host_file_name}")
set(packaged_entrypoint "${deployed_root}/DemoApp${runtime_host_extension}")

file(REMOVE_RECURSE "${test_root}")
file(MAKE_DIRECTORY "${builder_root}")
file(MAKE_DIRECTORY "${builder_root}/content")
file(MAKE_DIRECTORY "${plugin_root}")
file(WRITE "${builder_root}/content/main.prg" "? \"BUILDER_STARTUP_EXECUTED\"\nRETURN\n")

execute_process(
    COMMAND ${CMAKE_COMMAND} -E copy "${RUNTIME_HOST_EXECUTABLE}" "${packaged_runtime_host}"
    RESULT_VARIABLE copy_host_result
)
if(NOT copy_host_result EQUAL 0)
    message(FATAL_ERROR "Failed to copy runtime host into the deployed package.")
endif()

execute_process(
    COMMAND ${CMAKE_COMMAND} -E copy "${RUNTIME_HOST_EXECUTABLE}" "${packaged_entrypoint}"
    RESULT_VARIABLE copy_entrypoint_result
)
if(NOT copy_entrypoint_result EQUAL 0)
    message(FATAL_ERROR "Failed to copy packaged entrypoint executable.")
endif()

if(UNIX)
    execute_process(COMMAND chmod +x "${packaged_runtime_host}")
    execute_process(COMMAND chmod +x "${packaged_entrypoint}")
endif()

file(WRITE "${content_root}/main.prg" "RETURN\n")
file(WRITE "${plugin_root}/helper.dll" "plugin-payload")

file(SHA256 "${packaged_runtime_host}" runtime_host_hash)
file(SHA256 "${packaged_entrypoint}" native_entrypoint_hash)
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
extension_payload=${builder_root}/${runtime_host_file_name}|${runtime_host_hash}
extension_payload=${builder_root}/DemoApp${runtime_host_extension}|${native_entrypoint_hash}
extension_payload=${builder_root}/content/plugins/helper.dll|${helper_payload_hash}
")
file(WRITE "${deployed_root}/app.cfmanifest" "${manifest_text}")

execute_process(
    COMMAND "${packaged_entrypoint}"
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
if(run_output MATCHES "BUILDER_STARTUP_EXECUTED")
    message(FATAL_ERROR "Runtime host binding smoke executed the still-existing original build-root startup instead of the deployed package.\nstdout:\n${run_output}")
endif()

string(FIND "${run_output}" "${content_root}" content_root_position)
if(content_root_position EQUAL -1)
    message(FATAL_ERROR "Runtime host binding smoke did not report the rebound package content root.\nstdout:\n${run_output}")
endif()

string(FIND "${run_output}" "${deployed_root}/security_audit.log" audit_log_path_position)
if(audit_log_path_position EQUAL -1)
    message(FATAL_ERROR "Runtime host binding smoke did not report the rebound package-local audit log path.\nstdout:\n${run_output}")
endif()

if(NOT EXISTS "${deployed_root}/security_audit.log")
    message(FATAL_ERROR "Runtime host binding smoke did not write the rebound package-local audit log.")
endif()

set(separator_root "${test_root}/separator-deployed")
set(separator_content_root "${separator_root}/content")
file(MAKE_DIRECTORY "${separator_content_root}")
file(WRITE "${separator_content_root}/main.prg" "RETURN\n")
set(recorded_windows_root [=[C:\\builder\\PortableDemo]=])
set(separator_manifest_text
"manifest_version=1
project_title=PortableDemo
package_root=${recorded_windows_root}
content_root=${recorded_windows_root}\\\\content
working_directory=${recorded_windows_root}\\\\content
startup_item=main.prg
startup_source=${recorded_windows_root}\\\\content\\\\main.prg
security_enabled=false
security_role=
security_mode=native
dotnet_story=none
")
file(WRITE "${separator_root}/app.cfmanifest" "${separator_manifest_text}")

execute_process(
    COMMAND "${packaged_entrypoint}" --manifest "${separator_root}/app.cfmanifest"
    WORKING_DIRECTORY "${test_root}"
    RESULT_VARIABLE separator_run_result
    OUTPUT_VARIABLE separator_run_output
    ERROR_VARIABLE separator_run_error
)

if(NOT separator_run_result EQUAL 0)
    message(FATAL_ERROR "Runtime host Windows-separator binding smoke failed.\nstdout:\n${separator_run_output}\nstderr:\n${separator_run_error}")
endif()
if(NOT separator_run_output MATCHES "runtime\\.completed: true")
    message(FATAL_ERROR "Runtime host Windows-separator binding smoke did not complete.\nstdout:\n${separator_run_output}")
endif()
file(TO_NATIVE_PATH "${separator_content_root}/main.prg" separator_expected_startup)
string(FIND "${separator_run_output}" "${separator_expected_startup}" separator_startup_position)
if(separator_startup_position EQUAL -1)
    message(FATAL_ERROR "Runtime host did not rebind a serialized Windows startup path into the deployed package.\nstdout:\n${separator_run_output}")
endif()

execute_process(
    COMMAND ${CMAKE_COMMAND} -E copy "${RUNTIME_HOST_EXECUTABLE}" "${separator_root}/${runtime_host_file_name}"
    RESULT_VARIABLE separator_host_copy_result
)
if(NOT separator_host_copy_result EQUAL 0)
    message(FATAL_ERROR "Failed to stage the runtime host for the Windows-style payload diagnostic smoke.")
endif()
if(UNIX)
    execute_process(COMMAND chmod +x "${separator_root}/${runtime_host_file_name}")
endif()
set(separator_payload_manifest_text
"manifest_version=1
project_title=PortablePayloadDemo
package_root=${recorded_windows_root}
content_root=${recorded_windows_root}\\\\content
working_directory=${recorded_windows_root}\\\\content
startup_item=main.prg
startup_source=${recorded_windows_root}\\\\content\\\\main.prg
security_enabled=true
security_role=developer
security_mode=native
audit_log_path=${recorded_windows_root}\\\\security_audit.log
runtime_host_sha256=${runtime_host_hash}
extension_payload=${recorded_windows_root}\\\\${runtime_host_file_name}|${runtime_host_hash}
extension_payload=${recorded_windows_root}\\\\content\\\\plugins\\\\helper.dll|0000000000000000000000000000000000000000000000000000000000000000
dotnet_story=none
")
file(WRITE "${separator_root}/app.cfmanifest" "${separator_payload_manifest_text}")

execute_process(
    COMMAND ${CMAKE_COMMAND} -E env
        "COPPERFIN_LOCALE_DIR=${LOCALE_ROOT}"
        "COPPERFIN_LOCALE=en-US"
        "${packaged_entrypoint}" --manifest "${separator_root}/app.cfmanifest"
    WORKING_DIRECTORY "${test_root}"
    RESULT_VARIABLE separator_payload_result
    OUTPUT_VARIABLE separator_payload_output
    ERROR_VARIABLE separator_payload_error
)
if(NOT separator_payload_result EQUAL 8 OR
   NOT separator_payload_output MATCHES "Extension payload is missing from the package: helper\\.dll")
    message(FATAL_ERROR "Windows-style payload diagnostics did not preserve the portable payload basename.\nstdout:\n${separator_payload_output}\nstderr:\n${separator_payload_error}")
endif()

set(collision_builder_root "${test_root}/collision-builder/DemoApp")
set(collision_root "${test_root}/collision-deployed")
file(MAKE_DIRECTORY "${collision_root}/content")
file(WRITE "${collision_root}/main.prg" "? \"DECOY_EXECUTED\"\nRETURN\n")
set(collision_manifest_text
"manifest_version=1
project_title=CollisionDemo
package_root=${collision_builder_root}
content_root=${collision_builder_root}/content
working_directory=${collision_builder_root}/content
startup_item=main.prg
startup_source=${collision_builder_root}/content/main.prg
security_enabled=false
security_role=
security_mode=native
dotnet_story=none
")
file(WRITE "${collision_root}/app.cfmanifest" "${collision_manifest_text}")

execute_process(
    COMMAND ${CMAKE_COMMAND} -E env
        "COPPERFIN_LOCALE_DIR=${LOCALE_ROOT}"
        "COPPERFIN_LOCALE=en-US"
        "${packaged_entrypoint}" --manifest "${collision_root}/app.cfmanifest"
    WORKING_DIRECTORY "${test_root}"
    RESULT_VARIABLE collision_run_result
    OUTPUT_VARIABLE collision_run_output
    ERROR_VARIABLE collision_run_error
)

if(NOT collision_run_result EQUAL 4)
    message(FATAL_ERROR "Runtime host should reject a missing packaged startup with exit code 4.\nstdout:\n${collision_run_output}\nstderr:\n${collision_run_error}")
endif()
string(FIND "${collision_run_output}" "status: error" collision_status_position)
string(FIND "${collision_run_output}" "error: Startup source is missing from the package: main.prg" collision_error_position)
string(FIND "${collision_run_output}" "DECOY_EXECUTED" collision_decoy_position)
if(collision_status_position EQUAL -1 OR collision_error_position EQUAL -1 OR collision_decoy_position GREATER_EQUAL 0)
    message(FATAL_ERROR "Runtime host did not fail closed for a same-basename startup collision.\nstdout:\n${collision_run_output}")
endif()

execute_process(
    COMMAND ${CMAKE_COMMAND} -E env
        "COPPERFIN_LOCALE_DIR=${LOCALE_ROOT}"
        "COPPERFIN_LOCALE=pt-BR"
        "${packaged_entrypoint}" --manifest "${collision_root}/app.cfmanifest"
    WORKING_DIRECTORY "${test_root}"
    RESULT_VARIABLE localized_collision_result
    OUTPUT_VARIABLE localized_collision_output
    ERROR_VARIABLE localized_collision_error
)

if(NOT localized_collision_result EQUAL 4)
    message(FATAL_ERROR "Localized startup binding failure changed the invariant exit code.\nstdout:\n${localized_collision_output}\nstderr:\n${localized_collision_error}")
endif()
string(FIND "${localized_collision_output}" "status: error" localized_collision_status_position)
string(FIND "${localized_collision_output}" "erro: A origem de inicializacao esta ausente do pacote: main.prg" localized_collision_error_position)
string(FIND "${localized_collision_output}" "Startup source is missing from the package" localized_collision_english_position)
if(localized_collision_status_position EQUAL -1 OR
   localized_collision_error_position EQUAL -1 OR
   localized_collision_english_position GREATER_EQUAL 0)
    message(FATAL_ERROR "Runtime host startup binding failure did not localize without changing machine output.\nstdout:\n${localized_collision_output}")
endif()

file(MAKE_DIRECTORY "${test_root}/content")
file(WRITE "${test_root}/content/main.prg" "? \"CWD_DECOY_EXECUTED\"\nRETURN\n")
set(relative_collision_manifest_text
"manifest_version=1
project_title=RelativeCollisionDemo
package_root=
content_root=content
working_directory=content
startup_item=main.prg
startup_source=content/main.prg
security_enabled=false
security_role=
security_mode=native
dotnet_story=none
")
file(WRITE "${collision_root}/app.cfmanifest" "${relative_collision_manifest_text}")

execute_process(
    COMMAND ${CMAKE_COMMAND} -E env
        "COPPERFIN_LOCALE_DIR=${LOCALE_ROOT}"
        "COPPERFIN_LOCALE=en-US"
        "${packaged_entrypoint}" --manifest "${collision_root}/app.cfmanifest"
    WORKING_DIRECTORY "${test_root}"
    RESULT_VARIABLE relative_collision_result
    OUTPUT_VARIABLE relative_collision_output
    ERROR_VARIABLE relative_collision_error
)
if(NOT relative_collision_result EQUAL 4 OR relative_collision_output MATCHES "CWD_DECOY_EXECUTED")
    message(FATAL_ERROR "Runtime host accepted a startup path found only under the caller working directory.\nstdout:\n${relative_collision_output}\nstderr:\n${relative_collision_error}")
endif()

file(WRITE "${test_root}/outside.prg" "? \"OUTSIDE_DECOY_EXECUTED\"\nRETURN\n")
set(escaping_item_manifest_text
"manifest_version=1
project_title=EscapingItemDemo
package_root=${collision_builder_root}
content_root=${collision_builder_root}/content
working_directory=${collision_builder_root}/content
startup_item=../outside.prg
startup_source=../outside.prg
security_enabled=false
security_role=
security_mode=native
dotnet_story=none
")
file(WRITE "${collision_root}/app.cfmanifest" "${escaping_item_manifest_text}")

execute_process(
    COMMAND ${CMAKE_COMMAND} -E env
        "COPPERFIN_LOCALE_DIR=${LOCALE_ROOT}"
        "COPPERFIN_LOCALE=en-US"
        "${packaged_entrypoint}" --manifest "${collision_root}/app.cfmanifest"
    WORKING_DIRECTORY "${test_root}"
    RESULT_VARIABLE escaping_item_result
    OUTPUT_VARIABLE escaping_item_output
    ERROR_VARIABLE escaping_item_error
)
if(NOT escaping_item_result EQUAL 4 OR escaping_item_output MATCHES "OUTSIDE_DECOY_EXECUTED")
    message(FATAL_ERROR "Runtime host accepted startup metadata that escaped the package root.\nstdout:\n${escaping_item_output}\nstderr:\n${escaping_item_error}")
endif()

set(external_package_manifest_text
"manifest_version=1
project_title=ExternalPackageSourceDemo
package_root=${collision_builder_root}
content_root=${collision_builder_root}/content
working_directory=${collision_builder_root}/content
startup_item=outside.prg
startup_source=${test_root}/outside.prg
security_enabled=false
security_role=
security_mode=native
dotnet_story=none
")
file(WRITE "${collision_root}/app.cfmanifest" "${external_package_manifest_text}")

execute_process(
    COMMAND ${CMAKE_COMMAND} -E env
        "COPPERFIN_LOCALE_DIR=${LOCALE_ROOT}"
        "COPPERFIN_LOCALE=en-US"
        "${packaged_entrypoint}" --manifest "${collision_root}/app.cfmanifest"
    WORKING_DIRECTORY "${test_root}"
    RESULT_VARIABLE external_package_result
    OUTPUT_VARIABLE external_package_output
    ERROR_VARIABLE external_package_error
)
if(NOT external_package_result EQUAL 4 OR external_package_output MATCHES "OUTSIDE_DECOY_EXECUTED")
    message(FATAL_ERROR "Runtime package host executed an absolute startup source outside its recorded package root.\nstdout:\n${external_package_output}\nstderr:\n${external_package_error}")
endif()

set(external_content_root "${test_root}/external-content")
file(MAKE_DIRECTORY "${external_content_root}")
file(WRITE "${external_content_root}/main.prg" "? \"EXTERNAL_ROOT_EXECUTED\"\nRETURN\n")
set(external_root_manifest_text
"manifest_version=1
project_title=ExternalStartupRootDemo
package_root=${collision_builder_root}
content_root=${external_content_root}
working_directory=${external_content_root}
startup_item=main.prg
startup_source=
security_enabled=false
security_role=
security_mode=native
dotnet_story=none
")
file(WRITE "${collision_root}/app.cfmanifest" "${external_root_manifest_text}")

execute_process(
    COMMAND ${CMAKE_COMMAND} -E env
        "COPPERFIN_LOCALE_DIR=${LOCALE_ROOT}"
        "COPPERFIN_LOCALE=en-US"
        "${packaged_entrypoint}" --manifest "${collision_root}/app.cfmanifest"
    WORKING_DIRECTORY "${test_root}"
    RESULT_VARIABLE external_root_result
    OUTPUT_VARIABLE external_root_output
    ERROR_VARIABLE external_root_error
)
if(NOT external_root_result EQUAL 4 OR external_root_output MATCHES "EXTERNAL_ROOT_EXECUTED")
    message(FATAL_ERROR "Runtime package host executed startup_item from an external content or working root.\nstdout:\n${external_root_output}\nstderr:\n${external_root_error}")
endif()

file(WRITE "${test_root}/debug-source.prg" "? \"DEBUG_SOURCE_EXECUTED\"\nRETURN\n")
set(debug_manifest_text
"debug_manifest_version=2
project_title=ExternalDebugSourceDemo
package_root=${collision_builder_root}
content_root=${collision_builder_root}/content
working_directory=${test_root}
startup_item=debug-source.prg
startup_source=${test_root}/debug-source.prg
security_enabled=false
security_role=
security_mode=native
dotnet_story=none
")
file(WRITE "${collision_root}/app.cfdebug" "${debug_manifest_text}")

execute_process(
    COMMAND "${packaged_entrypoint}" --manifest "${collision_root}/app.cfdebug"
    WORKING_DIRECTORY "${test_root}"
    RESULT_VARIABLE debug_source_result
    OUTPUT_VARIABLE debug_source_output
    ERROR_VARIABLE debug_source_error
)
file(TO_NATIVE_PATH "${test_root}/debug-source.prg" expected_debug_source)
string(FIND "${debug_source_output}" "startup.source: ${expected_debug_source}" debug_source_path_position)
if(NOT debug_source_result EQUAL 0 OR
   debug_source_path_position EQUAL -1 OR
   NOT debug_source_output MATCHES "runtime\\.completed: true")
    message(FATAL_ERROR "Runtime host did not preserve the app.cfdebug external source-path contract.\nstdout:\n${debug_source_output}\nstderr:\n${debug_source_error}")
endif()

file(REMOVE_RECURSE "${test_root}")
