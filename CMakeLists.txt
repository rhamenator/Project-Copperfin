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
    src/runtime/prg_engine_date_time_functions.cpp
    src/runtime/prg_engine_file_io_functions.cpp
    src/runtime/prg_engine.cpp
    src/runtime/prg_engine_helpers.cpp
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
    add_subdirectory(tests)
endif()
