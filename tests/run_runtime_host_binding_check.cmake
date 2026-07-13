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
set(test_root "${temp_root}/copperfin runtime host binding % bang!_${timestamp}")

function(create_directory_indirection target_path link_path result_variable)
    if(WIN32)
        cmake_path(NATIVE_PATH target_path NORMALIZE native_target_path)
        cmake_path(NATIVE_PATH link_path NORMALIZE native_link_path)
        set(indirection_command
            "$ErrorActionPreference = 'Stop'; [void](New-Item -ItemType Junction -Path $env:COPPERFIN_TEST_LINK_PATH -Target $env:COPPERFIN_TEST_TARGET_PATH)")
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E env
                "COPPERFIN_TEST_LINK_PATH=${native_link_path}"
                "COPPERFIN_TEST_TARGET_PATH=${native_target_path}"
                powershell.exe -NoLogo -NoProfile -NonInteractive -Command "${indirection_command}"
            RESULT_VARIABLE indirection_result
            OUTPUT_VARIABLE indirection_output
            ERROR_VARIABLE indirection_error
        )
    else()
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E create_symlink "${target_path}" "${link_path}"
            RESULT_VARIABLE indirection_result
            OUTPUT_VARIABLE indirection_output
            ERROR_VARIABLE indirection_error
        )
    endif()
    if(NOT indirection_result EQUAL 0)
        message(STATUS
            "Directory indirection creation failed.\n"
            "target: ${target_path}\n"
            "link: ${link_path}\n"
            "stdout:\n${indirection_output}\n"
            "stderr:\n${indirection_error}")
    endif()
    set(${result_variable} "${indirection_result}" PARENT_SCOPE)
endfunction()

function(remove_directory_indirection link_path)
    if(WIN32)
        cmake_path(NATIVE_PATH link_path NORMALIZE native_link_path)
        set(removal_command
            "$ErrorActionPreference = 'Stop'; [System.IO.Directory]::Delete($env:COPPERFIN_TEST_LINK_PATH, $false)")
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E env
                "COPPERFIN_TEST_LINK_PATH=${native_link_path}"
                powershell.exe -NoLogo -NoProfile -NonInteractive -Command "${removal_command}"
            RESULT_VARIABLE removal_result
            OUTPUT_VARIABLE removal_output
            ERROR_VARIABLE removal_error
        )
    else()
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E rm -- "${link_path}"
            RESULT_VARIABLE removal_result
            OUTPUT_VARIABLE removal_output
            ERROR_VARIABLE removal_error
        )
    endif()
    if(NOT removal_result EQUAL 0)
        message(FATAL_ERROR
            "Directory indirection removal failed.\n"
            "link: ${link_path}\n"
            "stdout:\n${removal_output}\n"
            "stderr:\n${removal_error}")
    endif()
endfunction()

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

set(packaged_startup_source
"#INCLUDE 'verified.h'
PUBLIC snapshotProof
snapshotProof = SNAPSHOT_PROOF
RETURN
")
file(WRITE "${content_root}/main.prg" "${packaged_startup_source}")
file(WRITE "${content_root}/verified.h" "#DEFINE SNAPSHOT_PROOF 'VERIFIED_PRG_SNAPSHOT'\n")
file(WRITE "${plugin_root}/helper.dll" "plugin-payload")

file(SHA256 "${packaged_runtime_host}" runtime_host_hash)
file(SHA256 "${packaged_entrypoint}" native_entrypoint_hash)
file(SHA256 "${plugin_root}/helper.dll" helper_payload_hash)
file(SHA256 "${content_root}/main.prg" startup_asset_hash)
file(SHA256 "${content_root}/verified.h" startup_include_hash)

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
asset=1|main.prg|${builder_root}/content/main.prg|Program|false|true|${startup_asset_hash}|true
extension_payload=${builder_root}/${runtime_host_file_name}|${runtime_host_hash}
extension_payload=${builder_root}/DemoApp${runtime_host_extension}|${native_entrypoint_hash}
extension_payload=${builder_root}/content/plugins/helper.dll|${helper_payload_hash}
extension_payload=${builder_root}/content/verified.h|${startup_include_hash}
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

file(REAL_PATH "${content_root}" canonical_content_root)
cmake_path(NATIVE_PATH canonical_content_root NORMALIZE expected_content_root)
string(FIND "${run_output}" "${expected_content_root}" content_root_position)
if(content_root_position EQUAL -1)
    message(FATAL_ERROR
        "Runtime host binding smoke did not report the rebound package content root.\n"
        "expected: ${expected_content_root}\n"
        "stdout:\n${run_output}")
endif()

if(NOT EXISTS "${deployed_root}/security_audit.log")
    message(FATAL_ERROR "Runtime host binding smoke did not write the rebound package-local audit log.")
endif()

file(REAL_PATH "${deployed_root}/security_audit.log" expected_audit_log_path)
cmake_path(NATIVE_PATH expected_audit_log_path NORMALIZE expected_audit_log_path)
string(FIND "${run_output}" "${expected_audit_log_path}" audit_log_path_position)
if(audit_log_path_position EQUAL -1)
    message(FATAL_ERROR
        "Runtime host binding smoke did not report the rebound package-local audit log path.\n"
        "expected: ${expected_audit_log_path}\n"
        "stdout:\n${run_output}")
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
file(REAL_PATH "${separator_content_root}/main.prg" separator_canonical_startup)
cmake_path(NATIVE_PATH separator_canonical_startup NORMALIZE separator_expected_startup)
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

set(package_alias "${test_root}/deployed-alias")
create_directory_indirection("${deployed_root}" "${package_alias}" package_alias_result)
if(NOT package_alias_result EQUAL 0)
    message(FATAL_ERROR "Unable to create the package-root symlink or junction required by the runtime-host containment regression.")
endif()

execute_process(
    COMMAND "${package_alias}/DemoApp${runtime_host_extension}" --manifest "${package_alias}/app.cfmanifest"
    WORKING_DIRECTORY "${test_root}"
    RESULT_VARIABLE package_alias_run_result
    OUTPUT_VARIABLE package_alias_run_output
    ERROR_VARIABLE package_alias_run_error
)
if(NOT package_alias_run_result EQUAL 0 OR
   NOT package_alias_run_output MATCHES "runtime\\.completed: true")
    message(FATAL_ERROR "Runtime host rejected a valid relocated package reached through its package-root symlink or junction.\nstdout:\n${package_alias_run_output}\nstderr:\n${package_alias_run_error}")
endif()
remove_directory_indirection("${package_alias}")

string(REPLACE "security_role=developer" "security_role=runtime-operator" snapshot_manifest_text "${manifest_text}")
file(WRITE "${deployed_root}/app.cfmanifest" "${snapshot_manifest_text}")
file(TO_CMAKE_PATH "${content_root}/main.prg" startup_watch_path)
execute_process(
    COMMAND ${CMAKE_COMMAND} -E env
        "COPPERFIN_LOCALE_DIR=${LOCALE_ROOT}"
        "COPPERFIN_LOCALE=en-US"
        "${packaged_entrypoint}"
        --debug
        --breakpoint 2
        --debug-command continue
        --debug-command "watch:STRTOFILE('RETURN','${startup_watch_path}')"
        --debug-command continue
    WORKING_DIRECTORY "${test_root}"
    RESULT_VARIABLE verified_prg_snapshot_result
    OUTPUT_VARIABLE verified_prg_snapshot_output
    ERROR_VARIABLE verified_prg_snapshot_error
)
file(READ "${content_root}/main.prg" watched_startup_contents)
if(NOT verified_prg_snapshot_result EQUAL 0 OR
   NOT watched_startup_contents STREQUAL "RETURN" OR
   NOT verified_prg_snapshot_output MATCHES "debug\\.global\\.snapshotproof: VERIFIED_PRG_SNAPSHOT")
    message(FATAL_ERROR "Runtime host did not execute verified PRG bytes after the live package source changed while paused.\nstdout:\n${verified_prg_snapshot_output}\nstderr:\n${verified_prg_snapshot_error}")
endif()
file(WRITE "${content_root}/main.prg" "${packaged_startup_source}")
file(WRITE "${deployed_root}/app.cfmanifest" "${manifest_text}")

file(APPEND "${content_root}/main.prg" "? \"MUTATED_AFTER_PACKAGING\"\n")
execute_process(
    COMMAND ${CMAKE_COMMAND} -E env
        "COPPERFIN_LOCALE_DIR=${LOCALE_ROOT}"
        "COPPERFIN_LOCALE=en-US"
        "${packaged_entrypoint}"
    WORKING_DIRECTORY "${test_root}"
    RESULT_VARIABLE mutated_asset_result
    OUTPUT_VARIABLE mutated_asset_output
    ERROR_VARIABLE mutated_asset_error
)
if(NOT mutated_asset_result EQUAL 8 OR
   NOT mutated_asset_output MATCHES "Packaged asset hash mismatch: main\\.prg" OR
   mutated_asset_output MATCHES "MUTATED_AFTER_PACKAGING")
    message(FATAL_ERROR "Runtime host did not reject a packaged startup asset changed after its security digest was recorded.\nstdout:\n${mutated_asset_output}\nstderr:\n${mutated_asset_error}")
endif()
file(WRITE "${content_root}/main.prg" "${packaged_startup_source}")

string(
    REPLACE
    "asset=1|main.prg|${builder_root}/content/main.prg|Program|false|true|${startup_asset_hash}|true"
    "asset=1|main.prg|${builder_root}/content/main.prg|Program|false|true|${startup_asset_hash}|false"
    missing_startup_digest_manifest_text
    "${manifest_text}"
)
string(APPEND missing_startup_digest_manifest_text
    "extension_payload=${builder_root}/content/main.prg|${startup_asset_hash}\n")
file(WRITE "${deployed_root}/app.cfmanifest" "${missing_startup_digest_manifest_text}")
execute_process(
    COMMAND ${CMAKE_COMMAND} -E env
        "COPPERFIN_LOCALE_DIR=${LOCALE_ROOT}"
        "COPPERFIN_LOCALE=en-US"
        "${packaged_entrypoint}"
    WORKING_DIRECTORY "${test_root}"
    RESULT_VARIABLE missing_startup_digest_result
    OUTPUT_VARIABLE missing_startup_digest_output
    ERROR_VARIABLE missing_startup_digest_error
)
if(NOT missing_startup_digest_result EQUAL 8 OR
   NOT missing_startup_digest_output MATCHES "Security-enabled startup is missing a verified package digest: main\\.prg")
    message(FATAL_ERROR "Runtime host did not require a declared copied-asset digest for security startup.\nstdout:\n${missing_startup_digest_output}\nstderr:\n${missing_startup_digest_error}")
endif()
file(WRITE "${deployed_root}/app.cfmanifest" "${manifest_text}")

string(
    REPLACE
    "extension_payload=${builder_root}/content/verified.h|${startup_include_hash}\n"
    ""
    missing_include_digest_manifest_text
    "${manifest_text}"
)
file(WRITE "${deployed_root}/app.cfmanifest" "${missing_include_digest_manifest_text}")
execute_process(
    COMMAND ${CMAKE_COMMAND} -E env
        "COPPERFIN_LOCALE_DIR=${LOCALE_ROOT}"
        "COPPERFIN_LOCALE=en-US"
        "${packaged_entrypoint}"
    WORKING_DIRECTORY "${test_root}"
    RESULT_VARIABLE missing_include_digest_result
    OUTPUT_VARIABLE missing_include_digest_output
    ERROR_VARIABLE missing_include_digest_error
)
if(NOT missing_include_digest_result EQUAL 8 OR
   NOT missing_include_digest_output MATCHES "Verified package source is unavailable: main\\.prg")
    message(FATAL_ERROR "Runtime host did not fail closed for an unverified packaged PRG include.\nstdout:\n${missing_include_digest_output}\nstderr:\n${missing_include_digest_error}")
endif()
file(WRITE "${deployed_root}/app.cfmanifest" "${manifest_text}")

set(indirection_root "${test_root}/indirection-deployed")
set(indirection_content_root "${indirection_root}/content")
set(indirection_outside_root "${test_root}/indirection-outside")
set(indirection_link "${indirection_content_root}/linked-content")
set(indirection_builder_root "${test_root}/indirection-builder/DemoApp")
file(MAKE_DIRECTORY "${indirection_content_root}")
file(MAKE_DIRECTORY "${indirection_outside_root}")
file(WRITE "${indirection_content_root}/main.prg" "RETURN\n")
file(WRITE "${indirection_outside_root}/outside.prg" "? \"INDIRECTION_TARGET_EXECUTED\"\nRETURN\n")
create_directory_indirection("${indirection_outside_root}" "${indirection_link}" interior_indirection_result)
if(NOT interior_indirection_result EQUAL 0)
    message(FATAL_ERROR "Unable to create the interior symlink or junction required by the runtime-host containment regression.")
endif()

set(indirect_startup_manifest_text
"manifest_version=1
project_title=IndirectStartupDemo
package_root=${indirection_builder_root}
content_root=${indirection_builder_root}/content
working_directory=${indirection_builder_root}/content
startup_item=linked-content/outside.prg
startup_source=${indirection_builder_root}/content/linked-content/outside.prg
security_enabled=false
security_role=
security_mode=native
dotnet_story=none
")
file(WRITE "${indirection_root}/app.cfmanifest" "${indirect_startup_manifest_text}")

execute_process(
    COMMAND ${CMAKE_COMMAND} -E env
        "COPPERFIN_LOCALE_DIR=${LOCALE_ROOT}"
        "COPPERFIN_LOCALE=es-419"
        "${packaged_entrypoint}" --manifest "${indirection_root}/app.cfmanifest"
    WORKING_DIRECTORY "${test_root}"
    RESULT_VARIABLE indirect_startup_result
    OUTPUT_VARIABLE indirect_startup_output
    ERROR_VARIABLE indirect_startup_error
)
if(NOT indirect_startup_result EQUAL 4 OR
   NOT indirect_startup_output MATCHES "status: error" OR
   NOT indirect_startup_output MATCHES "La ruta del paquete no supero la validacion de contencion fisica: outside\\.prg" OR
   indirect_startup_output MATCHES "INDIRECTION_TARGET_EXECUTED")
    message(FATAL_ERROR "Runtime host did not fail closed with a localized error for an indirect startup path.\nstdout:\n${indirect_startup_output}\nstderr:\n${indirect_startup_error}")
endif()

set(final_component_indirection "${indirection_content_root}/final-linked.prg")
create_directory_indirection(
    "${indirection_outside_root}"
    "${final_component_indirection}"
    final_component_indirection_result
)
if(NOT final_component_indirection_result EQUAL 0)
    message(FATAL_ERROR "Unable to create the final-component symlink or junction required by the runtime-host containment regression.")
endif()
set(final_component_manifest_text
"manifest_version=1
project_title=FinalComponentIndirectionDemo
package_root=${indirection_builder_root}
content_root=${indirection_builder_root}/content
working_directory=${indirection_builder_root}/content
startup_item=final-linked.prg
startup_source=${indirection_builder_root}/content/final-linked.prg
security_enabled=false
security_role=
security_mode=native
dotnet_story=none
")
file(WRITE "${indirection_root}/app.cfmanifest" "${final_component_manifest_text}")
execute_process(
    COMMAND ${CMAKE_COMMAND} -E env
        "COPPERFIN_LOCALE_DIR=${LOCALE_ROOT}"
        "COPPERFIN_LOCALE=en-US"
        "${packaged_entrypoint}" --manifest "${indirection_root}/app.cfmanifest"
    WORKING_DIRECTORY "${test_root}"
    RESULT_VARIABLE final_component_result
    OUTPUT_VARIABLE final_component_output
    ERROR_VARIABLE final_component_error
)
if(NOT final_component_result EQUAL 4 OR
   NOT final_component_output MATCHES "Package path failed physical containment validation: final-linked\\.prg")
    message(FATAL_ERROR "Runtime host did not reject a final-component symlink or Windows junction.\nstdout:\n${final_component_output}\nstderr:\n${final_component_error}")
endif()
remove_directory_indirection("${final_component_indirection}")

execute_process(
    COMMAND ${CMAKE_COMMAND} -E copy "${RUNTIME_HOST_EXECUTABLE}" "${indirection_root}/${runtime_host_file_name}"
    RESULT_VARIABLE indirection_host_copy_result
)
if(NOT indirection_host_copy_result EQUAL 0)
    message(FATAL_ERROR "Failed to stage the runtime host for the indirect security-payload regression.")
endif()
if(UNIX)
    execute_process(COMMAND chmod +x "${indirection_root}/${runtime_host_file_name}")
endif()
file(SHA256 "${indirection_root}/${runtime_host_file_name}" indirection_runtime_host_hash)
file(SHA256 "${indirection_outside_root}/outside.prg" indirect_payload_hash)
set(indirect_payload_manifest_text
"manifest_version=1
project_title=IndirectPayloadDemo
package_root=${indirection_builder_root}
content_root=${indirection_builder_root}/content
working_directory=${indirection_builder_root}/content
startup_item=main.prg
startup_source=${indirection_builder_root}/content/main.prg
security_enabled=true
security_role=developer
security_mode=native
runtime_host_sha256=${indirection_runtime_host_hash}
extension_payload=${indirection_builder_root}/content/linked-content/outside.prg|${indirect_payload_hash}
dotnet_story=none
")
file(WRITE "${indirection_root}/app.cfmanifest" "${indirect_payload_manifest_text}")

execute_process(
    COMMAND ${CMAKE_COMMAND} -E env
        "COPPERFIN_LOCALE_DIR=${LOCALE_ROOT}"
        "COPPERFIN_LOCALE=en-US"
        "${packaged_entrypoint}" --manifest "${indirection_root}/app.cfmanifest"
    WORKING_DIRECTORY "${test_root}"
    RESULT_VARIABLE indirect_payload_result
    OUTPUT_VARIABLE indirect_payload_output
    ERROR_VARIABLE indirect_payload_error
)
if(NOT indirect_payload_result EQUAL 8 OR
   NOT indirect_payload_output MATCHES "status: error" OR
   NOT indirect_payload_output MATCHES "Package path failed physical containment validation: outside\\.prg")
    message(FATAL_ERROR "Runtime host did not fail closed for an indirect security payload.\nstdout:\n${indirect_payload_output}\nstderr:\n${indirect_payload_error}")
endif()

set(indirect_xasset_companion "${indirection_content_root}/redirected.sct")
create_directory_indirection(
    "${indirection_outside_root}"
    "${indirect_xasset_companion}"
    indirect_xasset_companion_result
)
if(NOT indirect_xasset_companion_result EQUAL 0)
    message(FATAL_ERROR "Unable to create the xAsset companion symlink or Windows junction required by the runtime-host containment regression.")
endif()
set(indirect_xasset_companion_manifest_text
"manifest_version=1
project_title=IndirectXAssetCompanionDemo
package_root=${indirection_builder_root}
content_root=${indirection_builder_root}/content
working_directory=${indirection_builder_root}/content
startup_item=main.prg
startup_source=${indirection_builder_root}/content/main.prg
security_enabled=true
security_role=developer
security_mode=native
runtime_host_sha256=${indirection_runtime_host_hash}
extension_payload=${indirection_builder_root}/content/redirected.sct|0000000000000000000000000000000000000000000000000000000000000000
dotnet_story=none
")
file(WRITE "${indirection_root}/app.cfmanifest" "${indirect_xasset_companion_manifest_text}")
execute_process(
    COMMAND ${CMAKE_COMMAND} -E env
        "COPPERFIN_LOCALE_DIR=${LOCALE_ROOT}"
        "COPPERFIN_LOCALE=en-US"
        "${packaged_entrypoint}" --manifest "${indirection_root}/app.cfmanifest"
    WORKING_DIRECTORY "${test_root}"
    RESULT_VARIABLE indirect_xasset_companion_run_result
    OUTPUT_VARIABLE indirect_xasset_companion_output
    ERROR_VARIABLE indirect_xasset_companion_error
)
if(NOT indirect_xasset_companion_run_result EQUAL 8 OR
   NOT indirect_xasset_companion_output MATCHES "status: error" OR
   NOT indirect_xasset_companion_output MATCHES "Package path failed physical containment validation: redirected\\.sct")
    message(FATAL_ERROR "Runtime host did not fail closed for an xAsset companion symlink or Windows junction.\nstdout:\n${indirect_xasset_companion_output}\nstderr:\n${indirect_xasset_companion_error}")
endif()
remove_directory_indirection("${indirect_xasset_companion}")

file(WRITE "${indirection_content_root}/customers.dbf" "writable data seed")
file(SHA256 "${indirection_content_root}/customers.dbf" indirect_data_asset_hash)
set(indirect_data_companion "${indirection_content_root}/customers.fpt")
create_directory_indirection(
    "${indirection_outside_root}"
    "${indirect_data_companion}"
    indirect_data_companion_result
)
if(NOT indirect_data_companion_result EQUAL 0)
    message(FATAL_ERROR "Unable to create the writable data companion symlink or Windows junction required by the runtime-host containment regression.")
endif()
set(indirect_data_companion_manifest_text
"manifest_version=3
project_title=IndirectDataCompanionDemo
package_root=${indirection_builder_root}
content_root=${indirection_builder_root}/content
working_directory=${indirection_builder_root}/content
startup_item=main.prg
startup_source=${indirection_builder_root}/content/main.prg
security_enabled=true
security_role=developer
security_mode=native
runtime_host_sha256=${indirection_runtime_host_hash}
data_policy=package_writable
asset=2|customers.dbf|${indirection_builder_root}/content/customers.dbf|Table|false|true|${indirect_data_asset_hash}|true
data_asset=${indirection_builder_root}/content/customers.dbf|package_writable
data_payload=${indirection_builder_root}/content/customers.fpt|package_writable|0000000000000000000000000000000000000000000000000000000000000000
dotnet_story=none
")
file(WRITE "${indirection_root}/app.cfmanifest" "${indirect_data_companion_manifest_text}")
execute_process(
    COMMAND ${CMAKE_COMMAND} -E env
        "COPPERFIN_LOCALE_DIR=${LOCALE_ROOT}"
        "COPPERFIN_LOCALE=en-US"
        "${packaged_entrypoint}" --manifest "${indirection_root}/app.cfmanifest"
    WORKING_DIRECTORY "${test_root}"
    RESULT_VARIABLE indirect_data_companion_run_result
    OUTPUT_VARIABLE indirect_data_companion_output
    ERROR_VARIABLE indirect_data_companion_error
)
if(NOT indirect_data_companion_run_result EQUAL 8 OR
   NOT indirect_data_companion_output MATCHES "status: error" OR
   NOT indirect_data_companion_output MATCHES "Package path failed physical containment validation: customers\\.fpt")
    message(FATAL_ERROR "Runtime host did not fail closed for a writable data companion symlink or Windows junction.\nstdout:\n${indirect_data_companion_output}\nstderr:\n${indirect_data_companion_error}")
endif()
remove_directory_indirection("${indirect_data_companion}")

file(REMOVE "${indirection_content_root}/customers.dbf")
set(indirect_data_asset "${indirection_content_root}/customers.dbf")
create_directory_indirection(
    "${indirection_outside_root}"
    "${indirect_data_asset}"
    indirect_data_asset_result
)
if(NOT indirect_data_asset_result EQUAL 0)
    message(FATAL_ERROR "Unable to create the writable data primary symlink or Windows junction required by the runtime-host containment regression.")
endif()
execute_process(
    COMMAND ${CMAKE_COMMAND} -E env
        "COPPERFIN_LOCALE_DIR=${LOCALE_ROOT}"
        "COPPERFIN_LOCALE=en-US"
        "${packaged_entrypoint}" --manifest "${indirection_root}/app.cfmanifest"
    WORKING_DIRECTORY "${test_root}"
    RESULT_VARIABLE indirect_data_asset_run_result
    OUTPUT_VARIABLE indirect_data_asset_output
    ERROR_VARIABLE indirect_data_asset_error
)
if(NOT indirect_data_asset_run_result EQUAL 8 OR
   NOT indirect_data_asset_output MATCHES "status: error" OR
   NOT indirect_data_asset_output MATCHES "Package path failed physical containment validation: customers\\.dbf")
    message(FATAL_ERROR "Runtime host did not fail closed for a writable data primary symlink or Windows junction.\nstdout:\n${indirect_data_asset_output}\nstderr:\n${indirect_data_asset_error}")
endif()
remove_directory_indirection("${indirect_data_asset}")
remove_directory_indirection("${indirection_link}")

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
    COMMAND "${packaged_entrypoint}" --manifest "${collision_root}/app.cfdebug" --debug
    WORKING_DIRECTORY "${test_root}"
    RESULT_VARIABLE debug_source_result
    OUTPUT_VARIABLE debug_source_output
    ERROR_VARIABLE debug_source_error
)
set(expected_debug_source "${test_root}/debug-source.prg")
cmake_path(NATIVE_PATH expected_debug_source NORMALIZE expected_debug_source)
string(FIND "${debug_source_output}" "startup.source: ${expected_debug_source}" debug_source_path_position)
if(NOT debug_source_result EQUAL 0 OR
   debug_source_path_position EQUAL -1 OR
   NOT debug_source_output MATCHES "debug\\.reason: completed")
    message(FATAL_ERROR "Runtime host did not preserve the explicitly authorized app.cfdebug external source-path contract.\nstdout:\n${debug_source_output}\nstderr:\n${debug_source_error}")
endif()

file(REMOVE_RECURSE "${test_root}")
