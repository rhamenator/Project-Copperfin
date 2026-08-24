cmake_minimum_required(VERSION 3.20)

project(Copperfin VERSION 0.1.0 LANGUAGES CXX C)

option(COPPERFIN_REQUIRE_X64
    "Require an x64 Windows toolchain for managed-product packaging"
    OFF)

# Keep the configure-time architecture result available to install-contract
# checks; CMake does not persist CMAKE_SIZEOF_VOID_P in its public cache.
set(COPPERFIN_NATIVE_POINTER_SIZE "${CMAKE_SIZEOF_VOID_P}" CACHE INTERNAL
    "Detected native pointer size used by Copperfin contract checks")

if(WIN32 AND COPPERFIN_REQUIRE_X64)
    if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
        message(FATAL_ERROR
            "Copperfin Windows builds require an x64 toolchain (CMAKE_SIZEOF_VOID_P must be 8)")
    endif()
endif()

include(CheckCXXCompilerFlag)
find_package(Threads REQUIRED)

# The environment boundary is header-only and used directly across native
# product and test targets, so every consumer inherits platform thread flags.
link_libraries(Threads::Threads)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    # CMake's Debug defaults place /RTC1 in CMAKE_CXX_FLAGS_DEBUG.  Managed
    # C++ rejects that option at command-line parse time, even if a target
    # appends /RTC-.  Move the default to target-scoped options so /clr
    # targets can exclude it altogether, like the explicit /EHsc policy.
    string(REGEX REPLACE "(^|[ \\t])/RTC1([ \\t]|$)" "\\1\\2"
        CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}" CACHE STRING
        "Flags used by the CXX compiler during DEBUG builds." FORCE)
    add_compile_options(
        /sdl
        "$<$<NOT:$<BOOL:$<TARGET_PROPERTY:COPPERFIN_DISABLE_CFG>>>:/guard:cf>"
        /Qspectre
        "$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CONFIG:Debug>,$<NOT:$<BOOL:$<TARGET_PROPERTY:COPPERFIN_MANAGED_CPP>>>>:/RTC1>"
    )
    add_link_options(
        /DYNAMICBASE
        /NXCOMPAT
        "$<$<NOT:$<BOOL:$<TARGET_PROPERTY:COPPERFIN_DISABLE_CFG>>>:/guard:cf>"
    )
endif()

option(COPPERFIN_BUILD_TESTS "Build Copperfin tests" ON)
option(COPPERFIN_BUILD_ROBUSTNESS_TESTS
    "Build optional bounded parser robustness tests (not a release gate)"
    OFF)
option(COPPERFIN_ENABLE_GNU_COVERAGE
    "Instrument native targets with GCC coverage data (local assurance evidence only)"
    OFF)
option(COPPERFIN_REQUIRE_SQLITE_CONNECTOR
    "Fail configuration when the read-only SQLite federation connector is unavailable"
    OFF)
option(COPPERFIN_MSVC_EXPLICIT_EXCEPTION_HANDLING
    "Apply MSVC exception flags per target for generators that cannot reconcile /EHsc with /clr"
    OFF)

option(COPPERFIN_ENABLE_PRODUCT_LICENSING
    "Enable the archived commercial product-license loader and status model"
    OFF)

option(COPPERFIN_ENFORCE_LAUNCHER_TRUST
    "Require signed app.cftrust metadata in the Windows generated-launcher guard"
    OFF)

# Coverage instrumentation is deliberately opt-in and GCC-only. It is useful
# for local assurance evidence but is not a shipping, package, or release
# configuration; mixing compiler-specific coverage flags into those builds
# would change their provenance and validation contract.
if(COPPERFIN_ENABLE_GNU_COVERAGE)
    if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        message(FATAL_ERROR
            "COPPERFIN_ENABLE_GNU_COVERAGE requires the GNU C++ compiler")
    endif()
    add_compile_options(-fprofile-arcs -ftest-coverage -O0)
    add_link_options(-fprofile-arcs -ftest-coverage)
endif()
set(COPPERFIN_LAUNCHER_TRUST_REGISTRY_HEADER
    "${CMAKE_CURRENT_SOURCE_DIR}/include/copperfin/package/launcher_trust_registry_empty.h"
    CACHE FILEPATH
    "Out-of-tree public-key registry header for release launcher trust")

if(NOT EXISTS "${COPPERFIN_LAUNCHER_TRUST_REGISTRY_HEADER}")
    message(FATAL_ERROR
        "COPPERFIN_LAUNCHER_TRUST_REGISTRY_HEADER does not exist: "
        "${COPPERFIN_LAUNCHER_TRUST_REGISTRY_HEADER}")
endif()

set(COPPERFIN_GENERATED_INCLUDE_ROOT "${CMAKE_CURRENT_BINARY_DIR}/generated/include")
set(COPPERFIN_GENERATED_PACKAGE_INCLUDE_DIR
    "${COPPERFIN_GENERATED_INCLUDE_ROOT}/copperfin/package")
file(MAKE_DIRECTORY "${COPPERFIN_GENERATED_PACKAGE_INCLUDE_DIR}")
configure_file(
    "${COPPERFIN_LAUNCHER_TRUST_REGISTRY_HEADER}"
    "${COPPERFIN_GENERATED_PACKAGE_INCLUDE_DIR}/launcher_trust_registry_generated.h"
    COPYONLY)

if(MSVC AND COPPERFIN_MSVC_EXPLICIT_EXCEPTION_HANDLING)
    if(CMAKE_CXX_FLAGS MATCHES "(^|[ \\t])/EH")
        message(FATAL_ERROR
            "COPPERFIN_MSVC_EXPLICIT_EXCEPTION_HANDLING requires CMAKE_CXX_FLAGS without a global /EH option")
    endif()
    add_compile_options(
        "$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<NOT:$<BOOL:$<TARGET_PROPERTY:COPPERFIN_MANAGED_CPP>>>>:/EHsc>"
    )
endif()

set(COPPERFIN_WINDOWS_STACK_RESERVE_BYTES 16777216)

function(copperfin_apply_windows_stack_reserve TARGET_NAME)
    if(WIN32)
        if(MSVC)
            target_link_options(${TARGET_NAME} PRIVATE "/STACK:${COPPERFIN_WINDOWS_STACK_RESERVE_BYTES}")
        else()
            target_link_options(${TARGET_NAME} PRIVATE "-Wl,--stack,${COPPERFIN_WINDOWS_STACK_RESERVE_BYTES}")
        endif()
    endif()
endfunction()

add_library(cf_platform_support
    src/platform/bounded_process.cpp
    src/platform/code_page.cpp
    src/platform/disk_space.cpp
    src/platform/environment.cpp
    src/platform/exclusive_file.cpp
    src/platform/executable_path.cpp
    src/platform/file_stream.cpp
    src/platform/file_version.cpp
    src/platform/font_directories.cpp
    src/platform/invariant_numeric.cpp
    src/platform/json.cpp
    src/platform/path.cpp
    src/platform/private_directory.cpp
    src/platform/private_executable_image.cpp
    src/platform/process_arguments.cpp
    src/platform/process_environment.cpp
    src/platform/safe_regex.cpp
    src/platform/windows_pe_image.cpp
)

target_include_directories(cf_platform_support
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(MSVC)
    target_compile_options(cf_platform_support PRIVATE /W4 /permissive-)
else()
    target_compile_options(cf_platform_support PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(WIN32)
    target_link_libraries(cf_platform_support PRIVATE advapi32)
    target_link_libraries(cf_platform_support PRIVATE user32)
    target_link_libraries(cf_platform_support PRIVATE version)
else()
    find_package(Iconv REQUIRED)
    target_link_libraries(cf_platform_support PRIVATE Iconv::Iconv)
endif()

add_library(cf_localization
    src/localization/localization.cpp
)

target_link_libraries(cf_localization PUBLIC cf_platform_support)

target_include_directories(cf_localization
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(MSVC)
    target_compile_options(cf_localization PRIVATE /W4 /permissive-)
else()
    target_compile_options(cf_localization PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_library(cf_security
    src/security/audit_stream.cpp
    src/security/authorization.cpp
    src/security/external_process_policy.cpp
    src/security/localized_text.cpp
    src/security/payload_crypto.cpp
    src/security/physical_path_containment.cpp
    src/security/process_hardening.cpp
    src/security/secret_provider.cpp
    src/security/sha256.cpp
    src/security/security_model.cpp
    src/security/workspace_agent_policy.cpp
    src/security/workspace_agent_audit_sink.cpp
    src/security/workspace_agent_environment.cpp
    src/security/workspace_agent_process_containment.cpp
    src/security/workspace_agent_process_parser.cpp
    src/security/workspace_agent_session.cpp
    src/security/workspace_agent_target_containment.cpp
    src/security/workspace_agent_tool_registry.cpp
)

target_include_directories(cf_security
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(MSVC)
    target_compile_options(cf_security PRIVATE /W4 /permissive-)
    target_link_libraries(cf_security PUBLIC bcrypt crypt32 wintrust version)
else()
    target_compile_options(cf_security PRIVATE -Wall -Wextra -Wpedantic)
endif()

target_link_libraries(cf_security PRIVATE cf_localization cf_platform_support)

add_library(cf_licensing
    src/licensing/base64.cpp
    src/licensing/canonical_payload_serializer.cpp
    src/licensing/ed25519_verify.cpp
    src/licensing/license_classifier.cpp
    src/licensing/license_payload_parser.cpp
    src/licensing/license_status.cpp
    src/licensing/third_party/ed25519_ref/fe.c
    src/licensing/third_party/ed25519_ref/ge.c
    src/licensing/third_party/ed25519_ref/sc.c
    src/licensing/third_party/ed25519_ref/sha512.c
    src/licensing/third_party/ed25519_ref/verify.c
)

target_include_directories(cf_licensing
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_definitions(cf_licensing
    PUBLIC
        COPPERFIN_ENABLE_PRODUCT_LICENSING=$<BOOL:${COPPERFIN_ENABLE_PRODUCT_LICENSING}>
)

if(MSVC)
    target_compile_options(cf_licensing PRIVATE /W4 /permissive-)
else()
    target_compile_options(cf_licensing PRIVATE -Wall -Wextra -Wpedantic)
endif()

target_link_libraries(cf_licensing PRIVATE cf_platform_support)

# Vendored third-party Ed25519 verify code: relax warnings so upstream
# source (see src/licensing/third_party/ed25519_ref/README.md) doesn't need
# to be edited to satisfy this project's own warning level.
set_source_files_properties(
    src/licensing/third_party/ed25519_ref/fe.c
    src/licensing/third_party/ed25519_ref/ge.c
    src/licensing/third_party/ed25519_ref/sc.c
    src/licensing/third_party/ed25519_ref/sha512.c
    src/licensing/third_party/ed25519_ref/verify.c
    PROPERTIES COMPILE_OPTIONS "$<IF:$<CXX_COMPILER_ID:MSVC>,/W1,-w>"
)

# cf_licensing retains the archived product-license parser/verifier for a
# possible later policy decision. Product loading is disabled by default.
# Package/launcher trust reuses only the low-level cryptographic primitives
# and remains independent of this product-license switch.

add_library(cf_package_trust
    src/package/launcher_inventory_trust.cpp
)

target_include_directories(cf_package_trust
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/licensing
)

if(MSVC)
    target_compile_options(cf_package_trust PRIVATE /W4 /permissive-)
else()
    target_compile_options(cf_package_trust PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Package trust may reuse the verify-only cryptographic primitive, but it has
# its own envelope, key registry, and verification API. It must not consume
# license parsing or license-status decisions.
target_link_libraries(cf_package_trust PRIVATE cf_licensing)

add_library(cf_platform_profile
    src/platform/printer.cpp
    src/platform/polyglot_benchmark.cpp
    src/platform/database_model.cpp
    src/platform/extensibility_model.cpp
    src/platform/federation_execution.cpp
    src/platform/localized_text.cpp
    src/platform/polyglot_artifact_admission.cpp
    src/platform/polyglot_artifact_adapter.cpp
    src/platform/polyglot_supporting_artifact_admission.cpp
    src/platform/polyglot_bridge_invocation.cpp
    src/platform/polyglot_interop_envelope.cpp
    src/platform/polyglot_migration_telemetry.cpp
    src/platform/polyglot_parity_comparator.cpp
    src/platform/polyglot_route_impact.cpp
    src/platform/polyglot_route_execution.cpp
    src/platform/polyglot_route_registry.cpp
    src/platform/query_translator.cpp
)

target_include_directories(cf_platform_profile
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(MSVC)
    target_compile_options(cf_platform_profile PRIVATE /W4 /permissive-)
else()
    target_compile_options(cf_platform_profile PRIVATE -Wall -Wextra -Wpedantic)
endif()

target_link_libraries(cf_platform_profile PRIVATE cf_localization cf_security)

set(COPPERFIN_SQLITE_CONNECTOR_AVAILABLE OFF)
set(COPPERFIN_SQLITE_CONNECTOR_SOURCE
    src/platform/sqlite_federation_connector_unavailable.cpp)
set(COPPERFIN_SQLITE_CONNECTOR_LIBRARY "")

if(WIN32)
    # Copperfin's Windows baseline is Windows 10 or newer, where the Windows
    # SDK and operating system provide winsqlite3.h and winsqlite3.lib.
    # Compile/link diagnostics are clearer than a configure-time try_compile,
    # whose SDK library discovery varies by Visual Studio generator.
    set(COPPERFIN_SQLITE_CONNECTOR_AVAILABLE ON)
    set(COPPERFIN_SQLITE_CONNECTOR_SOURCE
        src/platform/sqlite_federation_connector.cpp)
    set(COPPERFIN_SQLITE_CONNECTOR_LIBRARY winsqlite3)
else()
    find_package(SQLite3 QUIET)
    if(SQLite3_FOUND)
        set(COPPERFIN_SQLITE_CONNECTOR_AVAILABLE ON)
        set(COPPERFIN_SQLITE_CONNECTOR_SOURCE
            src/platform/sqlite_federation_connector.cpp)
        if(TARGET SQLite3::SQLite3)
            set(COPPERFIN_SQLITE_CONNECTOR_LIBRARY SQLite3::SQLite3)
        else()
            set(COPPERFIN_SQLITE_CONNECTOR_LIBRARY SQLite::SQLite3)
        endif()
    endif()
endif()

if(COPPERFIN_REQUIRE_SQLITE_CONNECTOR AND NOT COPPERFIN_SQLITE_CONNECTOR_AVAILABLE)
    message(FATAL_ERROR
        "COPPERFIN_REQUIRE_SQLITE_CONNECTOR requires SQLite3 on POSIX or winsqlite3 in the Windows SDK")
endif()

add_library(cf_sqlite_connector
    ${COPPERFIN_SQLITE_CONNECTOR_SOURCE}
)

target_include_directories(cf_sqlite_connector
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

if(MSVC)
    target_compile_options(cf_sqlite_connector PRIVATE /W4 /permissive-)
else()
    target_compile_options(cf_sqlite_connector PRIVATE -Wall -Wextra -Wpedantic)
endif()

target_link_libraries(cf_sqlite_connector
    PUBLIC cf_platform_profile ${COPPERFIN_SQLITE_CONNECTOR_LIBRARY}
    PRIVATE cf_security)

if(COPPERFIN_SQLITE_CONNECTOR_AVAILABLE)
    target_compile_definitions(cf_sqlite_connector PUBLIC
        COPPERFIN_SQLITE_CONNECTOR_AVAILABLE=1)
    if(WIN32)
        target_compile_definitions(cf_sqlite_connector PUBLIC
            COPPERFIN_SQLITE_USE_WINSQLITE3=1)
    endif()
else()
    target_compile_definitions(cf_sqlite_connector PUBLIC
        COPPERFIN_SQLITE_CONNECTOR_AVAILABLE=0)
endif()

add_library(cf_vfp_assets
    src/vfp/asset_inspector.cpp
    src/vfp/cdx_header.cpp
    src/vfp/dbf_header.cpp
    src/vfp/dbf_text_encoding.cpp
    src/vfp/dbf_table.cpp
    src/vfp/index_probe.cpp
    src/vfp/sidecar_path.cpp
    src/vfp/visual_asset_editor_shared_helpers.cpp
    src/vfp/visual_asset_editor_method_blobs.cpp
    src/vfp/visual_asset_editor_property_blobs.cpp
    src/vfp/visual_asset_editor_record_and_snapshot.cpp
    src/vfp/visual_asset_editor_undo.cpp
    src/vfp/visual_asset_editor_object_lifecycle.cpp
    src/vfp/visual_asset_editor_property_setters_behavior.cpp
    src/vfp/visual_asset_editor_property_setters_appearance.cpp
    src/vfp/visual_asset_editor_property_setters_data.cpp
)

target_include_directories(cf_vfp_assets
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(cf_vfp_assets PUBLIC cf_localization)

if(NOT WIN32)
    target_link_libraries(cf_vfp_assets PRIVATE Iconv::Iconv)
endif()

if(MSVC)
    target_compile_options(cf_vfp_assets PRIVATE /W4 /permissive-)
else()
    target_compile_options(cf_vfp_assets PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_library(cf_migration
    src/migration/project_inventory.cpp
)
target_include_directories(cf_migration PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
if(MSVC)
    target_compile_options(cf_migration PRIVATE /W4 /permissive-)
else()
    target_compile_options(cf_migration PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_library(cf_mcp_host
    src/platform/mcp_host.cpp
)
target_include_directories(cf_mcp_host PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(cf_mcp_host PRIVATE cf_platform_support cf_vfp_assets)
target_compile_definitions(cf_mcp_host PRIVATE
    COPPERFIN_MCP_HOST_VERSION="${PROJECT_VERSION}")
if(MSVC)
    target_compile_options(cf_mcp_host PRIVATE /W4 /permissive-)
else()
    target_compile_options(cf_mcp_host PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_library(cf_design_model
    src/studio/builder_dispatch.cpp
    src/studio/builder_invocation_admission.cpp
    src/studio/builder_registry.cpp
    src/studio/context_editor_actions.cpp
    src/studio/designer_context.cpp
    src/studio/designer_dispatch.cpp
    src/studio/designer_invocation_admission.cpp
    src/studio/designer_launch_surfaces.cpp
    src/studio/document_model.cpp
    src/studio/editor_action_dispatch.cpp
    src/studio/editor_action_invocation_admission.cpp
    src/studio/project_workspace.cpp
    src/studio/product_subsystems.cpp
    src/studio/report_layout.cpp
    src/studio/toolbox_creation.cpp
    src/studio/toolbox_dispatch.cpp
    src/studio/toolbox_invocation_admission.cpp
    src/studio/toolbox_palette.cpp
    src/studio/vs_launch_contract.cpp
    src/studio/vs_launch_contract_helpers_value_parsing.cpp
    src/studio/vs_launch_contract_helpers_error_messages.cpp
    src/studio/vs_launch_contract_helpers_argument_parsers.cpp
    src/studio/vs_launch_contract_helpers_request_validators.cpp
    src/studio/vs_launch_contract_diagnostics_and_session.cpp
    src/studio/vs_launch_contract_diagnostics_and_session_validate.cpp
    src/studio/vs_launch_contract_selection_context.cpp
    src/studio/vs_launch_contract_property_commands.cpp
    src/studio/vs_launch_contract_property_commands_validate.cpp
    src/studio/vs_launch_contract_object_lifecycle.cpp
    src/studio/vs_launch_contract_object_lifecycle_validate.cpp
    src/studio/vs_launch_contract_layout_actions.cpp
    src/studio/vs_launch_contract_layout_actions_validate.cpp
    src/studio/vs_launch_contract_setters_behavior.cpp
    src/studio/vs_launch_contract_setters_behavior_validate.cpp
    src/studio/vs_launch_contract_setters_appearance.cpp
    src/studio/vs_launch_contract_setters_appearance_validate.cpp
    src/studio/vs_launch_contract_setters_data.cpp
    src/studio/vs_launch_contract_setters_data_validate.cpp
    src/studio/vs_launch_contract_positional_and_fallback.cpp
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(copperfin_vs_launch_contract_compile_options
        -g1
        -fno-var-tracking
        -fno-var-tracking-assignments
    )
    check_cxx_compiler_flag("-fzero-init-padding-bits=standard" COPPERFIN_HAS_FZERO_INIT_PADDING_BITS)
    if(COPPERFIN_HAS_FZERO_INIT_PADDING_BITS)
        list(APPEND copperfin_vs_launch_contract_compile_options -fzero-init-padding-bits=standard)
    endif()
    set_source_files_properties(src/studio/vs_launch_contract.cpp PROPERTIES COMPILE_OPTIONS "${copperfin_vs_launch_contract_compile_options}")
endif()

set(COPPERFIN_RUNTIME_PIPELINE_SOURCES
    src/runtime/runtime_pipeline_text_and_identifier_utils.cpp
    src/runtime/runtime_pipeline_file_io_and_classification.cpp
    src/runtime/runtime_pipeline_library_export_metadata.cpp
    src/runtime/runtime_pipeline_library_export_artifacts.cpp
    src/runtime/runtime_pipeline_library_export_manifest.cpp
    src/runtime/runtime_pipeline_native_wrapper_process.cpp
    src/runtime/runtime_pipeline_fxp_and_archive_manifest.cpp
    src/runtime/runtime_pipeline_ast_ir_manifest.cpp
    src/runtime/runtime_pipeline_csharp_and_launcher.cpp
    src/runtime/runtime_pipeline_launcher_artifact_inventory.cpp
    src/runtime/runtime_pipeline_fd_file_io.cpp
    src/runtime/runtime_pipeline_package_content_io.cpp
    src/runtime/runtime_pipeline_manifest_pair.cpp
    src/runtime/runtime_pipeline_manifest_pair_io.cpp
    src/runtime/runtime_pipeline_public_api.cpp
)

add_library(cf_runtime_pipeline ${COPPERFIN_RUNTIME_PIPELINE_SOURCES})

if(COPPERFIN_BUILD_TESTS)
    add_library(cf_runtime_pipeline_test STATIC ${COPPERFIN_RUNTIME_PIPELINE_SOURCES})
    target_compile_definitions(cf_runtime_pipeline_test PRIVATE
        COPPERFIN_ENABLE_RUNTIME_PIPELINE_TEST_HOOKS=1
    )
endif()

add_library(cf_prg_analysis
    src/runtime/prg_engine_static_analysis.cpp
)

add_library(cf_runtime_text
    src/runtime/localized_text.cpp
)

add_library(cf_xbase_runtime
    src/runtime/prg_engine_command_helpers.cpp
    src/runtime/prg_engine_date_time_functions.cpp
    src/runtime/prg_engine_file_io_functions.cpp
    src/runtime/prg_engine.cpp
    src/runtime/polyglot_runtime_host.cpp
    src/runtime/index_seek_optimizer.cpp
    src/runtime/prg_engine_helpers.cpp
    src/runtime/prg_engine_locale_code_page.cpp
    src/runtime/prg_engine_numeric_functions.cpp
    src/runtime/prg_engine_path_functions.cpp
    src/runtime/prg_engine_parser.cpp
    src/runtime/prg_engine_runtime_surface_functions.cpp
    src/runtime/prg_engine_runtime_config.cpp
    src/runtime/prg_engine_string_functions.cpp
    src/runtime/prg_engine_table_structure_helpers.cpp
    src/runtime/prg_engine_type_functions.cpp
    src/runtime/xasset_methods.cpp
)

target_include_directories(cf_prg_analysis
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(cf_runtime_text
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(MSVC)
    target_compile_options(cf_prg_analysis PRIVATE /W4 /permissive-)
    target_compile_options(cf_runtime_text PRIVATE /W4 /permissive-)
else()
    target_compile_options(cf_prg_analysis PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(cf_runtime_text PRIVATE -Wall -Wextra -Wpedantic)
endif()

target_link_libraries(cf_runtime_text PRIVATE cf_localization)
target_link_libraries(cf_prg_analysis PUBLIC cf_runtime_text)

target_include_directories(cf_xbase_runtime
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(cf_xbase_runtime
    PUBLIC
        cf_prg_analysis
        cf_runtime_text
        cf_design_model
    PRIVATE
        cf_localization
        cf_platform_support
        cf_platform_profile
        cf_security
)

if(WIN32)
    target_link_libraries(cf_platform_profile PRIVATE winspool)
    target_sources(cf_xbase_runtime PRIVATE
        src/runtime/managed_declared_call.cpp
        src/runtime/managed_pe_image.cpp
        src/runtime/native_declared_call.cpp
        src/runtime/native_declared_library.cpp
        src/runtime/win64_native_call.cpp
    )
    target_link_libraries(cf_xbase_runtime PRIVATE oleaut32)
endif()

if(MSVC)
    # Embedded debug information can push the large interpreter translation
    # unit past the standard COFF section limit.
    target_compile_options(cf_xbase_runtime PRIVATE /W4 /permissive- /bigobj /we4530)
else()
    target_compile_options(cf_xbase_runtime PRIVATE -Wall -Wextra -Wpedantic)
endif()

set(copperfin_runtime_pipeline_targets cf_runtime_pipeline)
if(COPPERFIN_BUILD_TESTS)
    list(APPEND copperfin_runtime_pipeline_targets cf_runtime_pipeline_test)
endif()
foreach(runtime_pipeline_target IN LISTS copperfin_runtime_pipeline_targets)
    target_include_directories(${runtime_pipeline_target}
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    target_link_libraries(${runtime_pipeline_target}
        PUBLIC
            cf_design_model
            cf_security
            cf_platform_profile
            cf_xbase_runtime
            cf_licensing
    )
    if(MSVC)
        target_compile_options(${runtime_pipeline_target} PRIVATE /W4 /permissive-)
    else()
        target_compile_options(${runtime_pipeline_target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endforeach()

target_include_directories(cf_design_model
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(cf_design_model PUBLIC cf_vfp_assets cf_prg_analysis cf_localization)

if(MSVC)
    target_compile_options(cf_design_model PRIVATE /W4 /permissive-)
else()
    target_compile_options(cf_design_model PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(copperfin_inspect
    apps/copperfin_inspect/main.cpp
)

target_link_libraries(copperfin_inspect PRIVATE cf_vfp_assets cf_security cf_licensing)
target_link_libraries(copperfin_inspect PRIVATE cf_localization)

if(MSVC)
    target_compile_options(copperfin_inspect PRIVATE /W4 /permissive-)
else()
    target_compile_options(copperfin_inspect PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(copperfin_mcp_host
    apps/copperfin_mcp_host/main.cpp
)
target_link_libraries(copperfin_mcp_host PRIVATE cf_mcp_host cf_security)
copperfin_apply_windows_stack_reserve(copperfin_mcp_host)
if(MSVC)
    target_compile_options(copperfin_mcp_host PRIVATE /W4 /permissive-)
else()
    target_compile_options(copperfin_mcp_host PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(copperfin_studio_host
    apps/copperfin_studio_host/main.cpp
    apps/copperfin_studio_host/studio_host_main_shared_cli_infra.cpp
    apps/copperfin_studio_host/studio_host_main_workspace_agent_policy.cpp
    apps/copperfin_studio_host/studio_host_main_rushmore.cpp
    apps/copperfin_studio_host/studio_host_main_shell_command.cpp
    apps/copperfin_studio_host/studio_host_main_builder.cpp
    apps/copperfin_studio_host/studio_host_main_designer.cpp
    apps/copperfin_studio_host/studio_host_main_editor_action.cpp
    apps/copperfin_studio_host/studio_host_main_toolbox_shared_helpers.cpp
    apps/copperfin_studio_host/studio_host_main_toolbox_create.cpp
    apps/copperfin_studio_host/studio_host_main_toolbox_create_batch.cpp
    apps/copperfin_studio_host/studio_host_main_toolbox_dispatch_and_admission.cpp
    apps/copperfin_studio_host/studio_host_main_toolbox_palette.cpp
    apps/copperfin_studio_host/studio_host_main_visual_methods.cpp
    apps/copperfin_studio_host/studio_host_main_visual_objects.cpp
    apps/copperfin_studio_host/studio_host_main_visual_properties.cpp
    apps/copperfin_studio_host/studio_host_main_selection_and_report.cpp
)

target_link_libraries(copperfin_studio_host PRIVATE cf_design_model cf_security cf_platform_profile cf_licensing)

copperfin_apply_windows_stack_reserve(copperfin_studio_host)

if(MSVC)
    target_compile_options(copperfin_studio_host PRIVATE /W4 /permissive-)
else()
    target_compile_options(copperfin_studio_host PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(copperfin_runtime_host
    apps/copperfin_runtime_host/main.cpp
)

target_link_libraries(copperfin_runtime_host PRIVATE cf_xbase_runtime cf_security cf_platform_profile cf_sqlite_connector cf_licensing)
target_link_libraries(copperfin_runtime_host PRIVATE cf_localization)

if(COPPERFIN_BUILD_TESTS)
    target_compile_definitions(copperfin_runtime_host PRIVATE COPPERFIN_RUNTIME_HOST_TEST_HOOKS=1)
endif()

copperfin_apply_windows_stack_reserve(copperfin_runtime_host)

if(MSVC)
    target_compile_options(copperfin_runtime_host PRIVATE /W4 /permissive-)
else()
    target_compile_options(copperfin_runtime_host PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(copperfin_build_host
    apps/copperfin_build_host/main.cpp
)

target_link_libraries(copperfin_build_host PRIVATE cf_runtime_pipeline cf_localization)

if(WIN32)
    add_executable(copperfin_launcher_guard
        apps/copperfin_launcher_guard/main.cpp
    )
    target_link_libraries(copperfin_launcher_guard PRIVATE
        cf_security
        cf_localization
        cf_platform_support
        cf_package_trust)
    target_include_directories(copperfin_launcher_guard BEFORE PRIVATE
        ${COPPERFIN_GENERATED_INCLUDE_ROOT})
    if(COPPERFIN_ENFORCE_LAUNCHER_TRUST)
        target_compile_definitions(copperfin_launcher_guard PRIVATE
            COPPERFIN_ENFORCE_LAUNCHER_TRUST=1)
    endif()
    copperfin_apply_windows_stack_reserve(copperfin_launcher_guard)
    if(MSVC)
        target_compile_options(copperfin_launcher_guard PRIVATE /W4 /permissive-)
    else()
        target_compile_options(copperfin_launcher_guard PRIVATE -Wall -Wextra -Wpedantic)
    endif()
    add_dependencies(copperfin_build_host copperfin_launcher_guard)
    install(TARGETS copperfin_launcher_guard RUNTIME DESTINATION bin)
endif()

if(MSVC)
    target_compile_options(copperfin_build_host PRIVATE /W4 /permissive-)
else()
    target_compile_options(copperfin_build_host PRIVATE -Wall -Wextra -Wpedantic)
endif()

install(TARGETS copperfin_inspect copperfin_mcp_host copperfin_studio_host copperfin_runtime_host copperfin_build_host
    RUNTIME DESTINATION bin
)

if(WIN32)
    # The managed shell is built by the Windows installer workflow. Keep it in
    # a child directory so its host lookup reaches the native bin directory.
    add_executable(copperfin_studio_managed IMPORTED GLOBAL)
    set_target_properties(copperfin_studio_managed PROPERTIES
        IMPORTED_LOCATION_DEBUG
            "${CMAKE_SOURCE_DIR}/vsix/Copperfin.Studio/bin/Debug/net472/Copperfin.Studio.exe"
        IMPORTED_LOCATION_RELWITHDEBINFO
            "${CMAKE_SOURCE_DIR}/vsix/Copperfin.Studio/bin/RelWithDebInfo/net472/Copperfin.Studio.exe"
        IMPORTED_LOCATION_RELEASE
            "${CMAKE_SOURCE_DIR}/vsix/Copperfin.Studio/bin/Release/net472/Copperfin.Studio.exe"
        MAP_IMPORTED_CONFIG_MINSIZEREL
            Release
    )
    install(FILES
        "$<TARGET_FILE:copperfin_studio_managed>"
        "$<TARGET_FILE_DIR:copperfin_studio_managed>/Copperfin.Studio.exe.config"
        DESTINATION bin/studio
        COMPONENT Studio
    )
endif()

set(COPPERFIN_PACKAGE_DOCUMENTS
    README.md
    SECURITY.md
    SOURCE.md
    THIRD_PARTY_NOTICES.md
    remaining-work.md
)
set(COPPERFIN_PACKAGE_LICENSE_DOCUMENT LICENSE)
set(COPPERFIN_PACKAGE_EXCEPTION_DOCUMENT
    LICENSES/LicenseRef-Copperfin-Application-Runtime-Toolchain-Exception-1.0.txt)
set(COPPERFIN_PACKAGE_LICENSE_METADATA
    docs/contracts/release-license-metadata.json)

foreach(package_document IN ITEMS
        ${COPPERFIN_PACKAGE_DOCUMENTS}
        ${COPPERFIN_PACKAGE_LICENSE_DOCUMENT}
        ${COPPERFIN_PACKAGE_EXCEPTION_DOCUMENT}
        ${COPPERFIN_PACKAGE_LICENSE_METADATA})
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${package_document}")
        message(FATAL_ERROR "Required package document is missing: ${package_document}")
    endif()
endforeach()

install(FILES ${COPPERFIN_PACKAGE_DOCUMENTS}
    DESTINATION share/copperfin
    COMPONENT Documentation
)

install(FILES ${COPPERFIN_PACKAGE_LICENSE_DOCUMENT}
    DESTINATION share/copperfin
    RENAME LICENSE
    COMPONENT Documentation
)

install(FILES ${COPPERFIN_PACKAGE_EXCEPTION_DOCUMENT}
    DESTINATION share/copperfin/licenses
    COMPONENT Documentation
)

install(FILES ${COPPERFIN_PACKAGE_LICENSE_METADATA}
    DESTINATION share/copperfin/contracts
    COMPONENT Documentation
)

add_custom_target(copperfin_package_document_check
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
        -DBINARY_DIR=${CMAKE_BINARY_DIR}
        -DTEST_ROOT_BASE=${CMAKE_CURRENT_BINARY_DIR}/package-document-install
        -DTEST_CONFIGURATION=$<CONFIG>
        -P ${CMAKE_SOURCE_DIR}/tests/run_package_document_install_check.cmake
    VERBATIM
)

install(DIRECTORY resources/locales/
    DESTINATION share/copperfin/locales
)

# Make an out-of-tree developer build runnable from an arbitrary working
# directory, matching the executable-relative resource layout used by the
# installed hosts. Catalog inputs participate in CMake's reconfigure check so
# edits are reflected by the next normal build.
file(GLOB_RECURSE COPPERFIN_BUILD_TREE_LOCALE_CATALOGS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/resources/locales/*")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
    ${COPPERFIN_BUILD_TREE_LOCALE_CATALOGS})
# COPY is additive, so clear only this generated staging subtree before each
# configure-time copy. This prevents a deleted or renamed source catalog from
# remaining discoverable beside the out-of-tree executables.
file(REMOVE_RECURSE "${CMAKE_CURRENT_BINARY_DIR}/resources/locales")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/resources/locales"
    DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/resources")

# Traceability: RQ-CF-REL-002; DQ-windows-installer-lifecycle-scope;
# DV-windows-installer-lifecycle-contract; HZ-system-failure-01;
# HZ-data-corruption-01; HZ-doc-command-01.
set(CPACK_PACKAGE_NAME "copperfin")
set(CPACK_PACKAGE_VENDOR "Copperfin")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Copperfin standalone runtime and studio host tooling")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_CONTACT "maintainers@copperfin.local")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CMAKE_SYSTEM_NAME}")
set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "${CPACK_PACKAGE_NAME} ${CPACK_PACKAGE_VERSION}")
file(GENERATE
    OUTPUT "${CMAKE_BINARY_DIR}/CopperfinPackageVersion.txt"
    CONTENT "${PROJECT_VERSION}\n"
)
file(GENERATE
    OUTPUT "${CMAKE_BINARY_DIR}/CopperfinPackageInstallRegistryKey.txt"
    CONTENT "${CPACK_PACKAGE_INSTALL_REGISTRY_KEY}\n"
)

if(WIN32)
    set(CPACK_GENERATOR "NSIS;ZIP")
elseif(APPLE)
    set(CPACK_GENERATOR "productbuild;TGZ")
else()
    set(CPACK_GENERATOR "DEB;RPM;TGZ")
endif()

include(CPack)

if(COPPERFIN_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
