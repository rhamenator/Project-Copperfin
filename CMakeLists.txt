cmake_minimum_required(VERSION 3.20)

project(Copperfin LANGUAGES CXX C)

include(CheckCXXCompilerFlag)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/sdl /guard:cf /Qspectre)
    add_link_options(/DYNAMICBASE /NXCOMPAT /guard:cf)
endif()

option(COPPERFIN_BUILD_TESTS "Build Copperfin tests" ON)
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

add_library(cf_localization
    src/localization/localization.cpp
)

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
    src/security/physical_path_containment.cpp
    src/security/process_hardening.cpp
    src/security/secret_provider.cpp
    src/security/sha256.cpp
    src/security/security_model.cpp
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

target_link_libraries(cf_security PRIVATE cf_localization)

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

if(MSVC)
    target_compile_options(cf_licensing PRIVATE /W4 /permissive-)
else()
    target_compile_options(cf_licensing PRIVATE -Wall -Wextra -Wpedantic)
endif()

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

# cf_licensing is intentionally dependency-free (no cf_localization link) so
# it stays trivially unit-testable offline; see license_status.h.

add_library(cf_platform_profile
    src/platform/database_model.cpp
    src/platform/extensibility_model.cpp
    src/platform/federation_execution.cpp
    src/platform/localized_text.cpp
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

target_link_libraries(cf_platform_profile PRIVATE cf_localization)

add_library(cf_vfp_assets
    src/vfp/asset_inspector.cpp
    src/vfp/cdx_header.cpp
    src/vfp/dbf_header.cpp
    src/vfp/dbf_table.cpp
    src/vfp/index_probe.cpp
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

if(MSVC)
    target_compile_options(cf_vfp_assets PRIVATE /W4 /permissive-)
else()
    target_compile_options(cf_vfp_assets PRIVATE -Wall -Wextra -Wpedantic)
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

add_library(cf_runtime_pipeline
    src/runtime/runtime_pipeline_text_and_identifier_utils.cpp
    src/runtime/runtime_pipeline_file_io_and_classification.cpp
    src/runtime/runtime_pipeline_library_export_manifest.cpp
    src/runtime/runtime_pipeline_fxp_and_archive_manifest.cpp
    src/runtime/runtime_pipeline_ast_ir_manifest.cpp
    src/runtime/runtime_pipeline_csharp_and_launcher.cpp
    src/runtime/runtime_pipeline_public_api.cpp
)

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
    src/runtime/index_seek_optimizer.cpp
    src/runtime/prg_engine_helpers.cpp
    src/runtime/prg_engine_numeric_functions.cpp
    src/runtime/prg_engine_path_functions.cpp
    src/runtime/prg_engine_parser.cpp
    src/runtime/prg_engine_runtime_surface_functions.cpp
    src/runtime/prg_engine_runtime_config.cpp
    src/runtime/prg_engine_string_functions.cpp
    src/runtime/prg_engine_table_structure_helpers.cpp
    src/runtime/prg_engine_type_functions.cpp
    src/runtime/win64_native_call.cpp
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
)

if(WIN32)
    target_sources(cf_xbase_runtime PRIVATE
        src/runtime/dispatch_exception_info.cpp
        src/runtime/managed_pe_image.cpp
    )
    target_link_libraries(cf_xbase_runtime PRIVATE oleaut32)
    if(COPPERFIN_BUILD_TESTS)
        target_compile_definitions(cf_xbase_runtime PRIVATE
            COPPERFIN_ENABLE_DISPATCH_TEST_HOOKS=1
        )
    endif()
endif()

if(MSVC)
    target_compile_options(cf_xbase_runtime PRIVATE /W4 /permissive-)
else()
    target_compile_options(cf_xbase_runtime PRIVATE -Wall -Wextra -Wpedantic)
endif()

target_include_directories(cf_runtime_pipeline
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(cf_runtime_pipeline
    PUBLIC
        cf_design_model
        cf_security
        cf_platform_profile
        cf_xbase_runtime
        cf_licensing
)

if(MSVC)
    target_compile_options(cf_runtime_pipeline PRIVATE /W4 /permissive-)
else()
    target_compile_options(cf_runtime_pipeline PRIVATE -Wall -Wextra -Wpedantic)
endif()

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

add_executable(copperfin_studio_host
    apps/copperfin_studio_host/main.cpp
    apps/copperfin_studio_host/studio_host_main_shared_cli_infra.cpp
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

target_link_libraries(copperfin_runtime_host PRIVATE cf_xbase_runtime cf_security cf_platform_profile cf_licensing)
target_link_libraries(copperfin_runtime_host PRIVATE cf_localization)

if(MSVC)
    target_compile_options(copperfin_runtime_host PRIVATE /W4 /permissive-)
else()
    target_compile_options(copperfin_runtime_host PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(copperfin_build_host
    apps/copperfin_build_host/main.cpp
)

target_link_libraries(copperfin_build_host PRIVATE cf_runtime_pipeline cf_localization)

if(MSVC)
    target_compile_options(copperfin_build_host PRIVATE /W4 /permissive-)
else()
    target_compile_options(copperfin_build_host PRIVATE -Wall -Wextra -Wpedantic)
endif()

install(TARGETS copperfin_inspect copperfin_studio_host copperfin_runtime_host copperfin_build_host
    RUNTIME DESTINATION bin
)

install(FILES README.md LICENSE SECURITY.md remaining-work.md
    DESTINATION share/copperfin
)

install(DIRECTORY resources/locales/
    DESTINATION share/copperfin/locales
)

set(CPACK_PACKAGE_NAME "copperfin")
set(CPACK_PACKAGE_VENDOR "Copperfin")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Copperfin standalone runtime and studio host tooling")
set(CPACK_PACKAGE_VERSION "0.1.0")
set(CPACK_PACKAGE_CONTACT "maintainers@copperfin.local")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CMAKE_SYSTEM_NAME}")

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
