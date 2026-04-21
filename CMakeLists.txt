cmake_minimum_required(VERSION 3.20)

project(Copperfin LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/sdl /guard:cf /Qspectre)
    add_link_options(/DYNAMICBASE /NXCOMPAT /guard:cf)
endif()

option(COPPERFIN_BUILD_TESTS "Build Copperfin tests" ON)

add_library(cf_security
    src/security/audit_stream.cpp
    src/security/authorization.cpp
    src/security/external_process_policy.cpp
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

add_library(cf_platform_profile
    src/platform/database_model.cpp
    src/platform/extensibility_model.cpp
    src/platform/federation_execution.cpp
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

add_library(cf_vfp_assets
    src/vfp/asset_inspector.cpp
    src/vfp/cdx_header.cpp
    src/vfp/dbf_header.cpp
    src/vfp/dbf_table.cpp
    src/vfp/index_probe.cpp
    src/vfp/visual_asset_editor.cpp
)

target_include_directories(cf_vfp_assets
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(MSVC)
    target_compile_options(cf_vfp_assets PRIVATE /W4 /permissive-)
else()
    target_compile_options(cf_vfp_assets PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_library(cf_design_model
    src/studio/document_model.cpp
    src/studio/project_workspace.cpp
    src/studio/product_subsystems.cpp
    src/studio/report_layout.cpp
    src/studio/vs_launch_contract.cpp
)

add_library(cf_runtime_pipeline
    src/runtime/runtime_pipeline.cpp
)

add_library(cf_prg_analysis
    src/runtime/prg_engine_static_analysis.cpp
)

add_library(cf_xbase_runtime
    src/runtime/prg_engine_command_helpers.cpp
    src/runtime/prg_engine.cpp
    src/runtime/prg_engine_helpers.cpp
    src/runtime/prg_engine_parser.cpp
    src/runtime/prg_engine_runtime_config.cpp
    src/runtime/prg_engine_table_structure_helpers.cpp
    src/runtime/xasset_methods.cpp
)

target_include_directories(cf_prg_analysis
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(MSVC)
    target_compile_options(cf_prg_analysis PRIVATE /W4 /permissive-)
else()
    target_compile_options(cf_prg_analysis PRIVATE -Wall -Wextra -Wpedantic)
endif()

target_include_directories(cf_xbase_runtime
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(cf_xbase_runtime
    PUBLIC
        cf_prg_analysis
        cf_design_model
)

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

target_link_libraries(cf_design_model PUBLIC cf_vfp_assets cf_prg_analysis)

if(MSVC)
    target_compile_options(cf_design_model PRIVATE /W4 /permissive-)
else()
    target_compile_options(cf_design_model PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(copperfin_inspect
    apps/copperfin_inspect/main.cpp
)

target_link_libraries(copperfin_inspect PRIVATE cf_vfp_assets cf_security)

if(MSVC)
    target_compile_options(copperfin_inspect PRIVATE /W4 /permissive-)
else()
    target_compile_options(copperfin_inspect PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(copperfin_studio_host
    apps/copperfin_studio_host/main.cpp
)

target_link_libraries(copperfin_studio_host PRIVATE cf_design_model cf_security cf_platform_profile)

if(MSVC)
    target_compile_options(copperfin_studio_host PRIVATE /W4 /permissive-)
else()
    target_compile_options(copperfin_studio_host PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(copperfin_runtime_host
    apps/copperfin_runtime_host/main.cpp
)

target_link_libraries(copperfin_runtime_host PRIVATE cf_xbase_runtime cf_security cf_platform_profile)

if(MSVC)
    target_compile_options(copperfin_runtime_host PRIVATE /W4 /permissive-)
else()
    target_compile_options(copperfin_runtime_host PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(copperfin_build_host
    apps/copperfin_build_host/main.cpp
)

target_link_libraries(copperfin_build_host PRIVATE cf_runtime_pipeline)

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

    add_executable(test_vfp_assets
        tests/test_vfp_assets.cpp
    )

    target_link_libraries(test_vfp_assets PRIVATE cf_vfp_assets)

    if(MSVC)
        target_compile_options(test_vfp_assets PRIVATE /W4 /permissive-)
    else()
        target_compile_options(test_vfp_assets PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    add_test(NAME test_vfp_assets COMMAND $<TARGET_FILE:test_vfp_assets>)

    add_executable(test_studio_host
        tests/test_studio_host.cpp
    )

    target_link_libraries(test_studio_host PRIVATE cf_design_model)

    if(MSVC)
        target_compile_options(test_studio_host PRIVATE /W4 /permissive-)
    else()
        target_compile_options(test_studio_host PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    add_test(NAME test_studio_host COMMAND $<TARGET_FILE:test_studio_host>)

    add_executable(test_dbf_table
        tests/test_dbf_table.cpp
    )

    target_link_libraries(test_dbf_table PRIVATE cf_vfp_assets)

    if(MSVC)
        target_compile_options(test_dbf_table PRIVATE /W4 /permissive-)
    else()
        target_compile_options(test_dbf_table PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    add_test(NAME test_dbf_table COMMAND $<TARGET_FILE:test_dbf_table>)

    add_executable(test_visual_asset_editor
        tests/test_visual_asset_editor.cpp
    )

    target_link_libraries(test_visual_asset_editor PRIVATE cf_vfp_assets)

    if(MSVC)
        target_compile_options(test_visual_asset_editor PRIVATE /W4 /permissive-)
    else()
        target_compile_options(test_visual_asset_editor PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    add_test(NAME test_visual_asset_editor COMMAND $<TARGET_FILE:test_visual_asset_editor>)

    add_executable(test_product_subsystems
        tests/test_product_subsystems.cpp
    )

    target_link_libraries(test_product_subsystems PRIVATE cf_design_model)

    if(MSVC)
        target_compile_options(test_product_subsystems PRIVATE /W4 /permissive-)
    else()
        target_compile_options(test_product_subsystems PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    add_test(NAME test_product_subsystems COMMAND $<TARGET_FILE:test_product_subsystems>)

    add_executable(test_report_layout
        tests/test_report_layout.cpp
    )

    target_link_libraries(test_report_layout PRIVATE cf_design_model)

    if(MSVC)
        target_compile_options(test_report_layout PRIVATE /W4 /permissive-)
    else()
        target_compile_options(test_report_layout PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    add_test(NAME test_report_layout COMMAND $<TARGET_FILE:test_report_layout>)

    add_executable(test_project_workspace
        tests/test_project_workspace.cpp
    )

    target_link_libraries(test_project_workspace PRIVATE cf_design_model)

    if(MSVC)
        target_compile_options(test_project_workspace PRIVATE /W4 /permissive-)
    else()
        target_compile_options(test_project_workspace PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    add_test(NAME test_project_workspace COMMAND $<TARGET_FILE:test_project_workspace>)

    add_executable(test_platform_models
        tests/test_platform_models.cpp
    )

    target_link_libraries(test_platform_models PRIVATE cf_security cf_platform_profile)

    if(MSVC)
        target_compile_options(test_platform_models PRIVATE /W4 /permissive-)
    else()
        target_compile_options(test_platform_models PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    add_test(NAME test_platform_models COMMAND $<TARGET_FILE:test_platform_models>)

    add_executable(test_query_translator
        tests/test_query_translator.cpp
    )

    target_link_libraries(test_query_translator PRIVATE cf_platform_profile)

    if(MSVC)
        target_compile_options(test_query_translator PRIVATE /W4 /permissive-)
    else()
        target_compile_options(test_query_translator PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    add_test(NAME test_query_translator COMMAND $<TARGET_FILE:test_query_translator>)

    add_executable(test_federation_execution
        tests/test_federation_execution.cpp
    )

    target_link_libraries(test_federation_execution PRIVATE cf_platform_profile)

    if(MSVC)
        target_compile_options(test_federation_execution PRIVATE /W4 /permissive-)
    else()
        target_compile_options(test_federation_execution PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    add_test(NAME test_federation_execution COMMAND $<TARGET_FILE:test_federation_execution>)

    add_executable(test_runtime_pipeline
        tests/test_runtime_pipeline.cpp
    )

    target_link_libraries(test_runtime_pipeline PRIVATE cf_runtime_pipeline)

    if(MSVC)
        target_compile_options(test_runtime_pipeline PRIVATE /W4 /permissive-)
    else()
        target_compile_options(test_runtime_pipeline PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    add_test(NAME test_runtime_pipeline COMMAND $<TARGET_FILE:test_runtime_pipeline>)

    add_executable(test_security_controls
        tests/test_security_controls.cpp
    )

    target_link_libraries(test_security_controls PRIVATE cf_security)

    if(MSVC)
        target_compile_options(test_security_controls PRIVATE /W4 /permissive-)
    else()
        target_compile_options(test_security_controls PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    add_test(NAME test_security_controls COMMAND $<TARGET_FILE:test_security_controls>)

    add_executable(test_prg_engine
        tests/test_prg_engine.cpp
        tests/prg_engine_test_support.cpp
    )

    target_link_libraries(test_prg_engine PRIVATE cf_xbase_runtime)

    if(MSVC)
        target_compile_options(test_prg_engine PRIVATE /W4 /permissive-)
    else()
        target_compile_options(test_prg_engine PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    add_test(NAME test_prg_engine COMMAND $<TARGET_FILE:test_prg_engine>)

    add_executable(test_prg_engine_work_areas
        tests/test_prg_engine_work_areas.cpp
        tests/prg_engine_test_support.cpp
    )

    target_link_libraries(test_prg_engine_work_areas PRIVATE cf_xbase_runtime)

    if(MSVC)
        target_compile_options(test_prg_engine_work_areas PRIVATE /W4 /permissive-)
    else()
        target_compile_options(test_prg_engine_work_areas PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    add_test(NAME test_prg_engine_work_areas COMMAND $<TARGET_FILE:test_prg_engine_work_areas>)

    add_executable(test_prg_engine_seek_index
        tests/test_prg_engine_seek_index.cpp
        tests/prg_engine_test_support.cpp
    )

    target_link_libraries(test_prg_engine_seek_index PRIVATE cf_xbase_runtime)

    if(MSVC)
        target_compile_options(test_prg_engine_seek_index PRIVATE /W4 /permissive-)
    else()
        target_compile_options(test_prg_engine_seek_index PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    add_test(NAME test_prg_engine_seek_index COMMAND $<TARGET_FILE:test_prg_engine_seek_index>)

    add_executable(test_prg_engine_control_flow
        tests/test_prg_engine_control_flow.cpp
        tests/prg_engine_test_support.cpp
    )

    target_link_libraries(test_prg_engine_control_flow PRIVATE cf_xbase_runtime)

    if(MSVC)
        target_compile_options(test_prg_engine_control_flow PRIVATE /W4 /permissive-)
    else()
        target_compile_options(test_prg_engine_control_flow PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    add_test(NAME test_prg_engine_control_flow COMMAND $<TARGET_FILE:test_prg_engine_control_flow>)

    add_executable(test_prg_engine_data_io
        tests/test_prg_engine_data_io.cpp
        tests/prg_engine_test_support.cpp
    )

    target_link_libraries(test_prg_engine_data_io PRIVATE cf_xbase_runtime)

    if(MSVC)
        target_compile_options(test_prg_engine_data_io PRIVATE /W4 /permissive-)
    else()
        target_compile_options(test_prg_engine_data_io PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    add_test(NAME test_prg_engine_data_io COMMAND $<TARGET_FILE:test_prg_engine_data_io>)

    add_executable(test_prg_engine_arrays
        tests/test_prg_engine_arrays.cpp
        tests/prg_engine_test_support.cpp
    )

    target_link_libraries(test_prg_engine_arrays PRIVATE cf_xbase_runtime)

    if(MSVC)
        target_compile_options(test_prg_engine_arrays PRIVATE /W4 /permissive-)
    else()
        target_compile_options(test_prg_engine_arrays PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    add_test(NAME test_prg_engine_arrays COMMAND $<TARGET_FILE:test_prg_engine_arrays>)

    add_executable(test_prg_engine_functions
        tests/test_prg_engine_functions.cpp
        tests/prg_engine_test_support.cpp
    )

    target_link_libraries(test_prg_engine_functions PRIVATE cf_xbase_runtime)

    if(MSVC)
        target_compile_options(test_prg_engine_functions PRIVATE /W4 /permissive-)
    else()
        target_compile_options(test_prg_engine_functions PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    add_test(NAME test_prg_engine_functions COMMAND $<TARGET_FILE:test_prg_engine_functions>)

    add_executable(test_prg_engine_sql_cursors
        tests/test_prg_engine_sql_cursors.cpp
        tests/prg_engine_test_support.cpp
    )

    target_link_libraries(test_prg_engine_sql_cursors PRIVATE cf_xbase_runtime)

    if(MSVC)
        target_compile_options(test_prg_engine_sql_cursors PRIVATE /W4 /permissive-)
    else()
        target_compile_options(test_prg_engine_sql_cursors PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    add_test(NAME test_prg_engine_sql_cursors COMMAND $<TARGET_FILE:test_prg_engine_sql_cursors>)

    add_executable(test_prg_engine_table_mutation
        tests/test_prg_engine_table_mutation.cpp
        tests/prg_engine_test_support.cpp
    )

    target_link_libraries(test_prg_engine_table_mutation PRIVATE cf_xbase_runtime)

    if(MSVC)
        target_compile_options(test_prg_engine_table_mutation PRIVATE /W4 /permissive-)
    else()
        target_compile_options(test_prg_engine_table_mutation PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    add_test(NAME test_prg_engine_table_mutation COMMAND $<TARGET_FILE:test_prg_engine_table_mutation>)

    add_executable(test_prg_engine_table_structure
        tests/test_prg_engine_table_structure.cpp
        tests/prg_engine_test_support.cpp
    )

    target_link_libraries(test_prg_engine_table_structure PRIVATE cf_xbase_runtime)

    if(MSVC)
        target_compile_options(test_prg_engine_table_structure PRIVATE /W4 /permissive-)
    else()
        target_compile_options(test_prg_engine_table_structure PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    add_test(NAME test_prg_engine_table_structure COMMAND $<TARGET_FILE:test_prg_engine_table_structure>)

    add_executable(test_xasset_methods
        tests/test_xasset_methods.cpp
    )

    target_link_libraries(test_xasset_methods PRIVATE cf_xbase_runtime)

    if(MSVC)
        target_compile_options(test_xasset_methods PRIVATE /W4 /permissive-)
    else()
        target_compile_options(test_xasset_methods PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    add_test(NAME test_xasset_methods COMMAND $<TARGET_FILE:test_xasset_methods>)
endif()
