cmake_minimum_required(VERSION 3.20)

project(Copperfin LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(COPPERFIN_BUILD_TESTS "Build Copperfin tests" ON)

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

target_include_directories(cf_design_model
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(cf_design_model PUBLIC cf_vfp_assets)

if(MSVC)
    target_compile_options(cf_design_model PRIVATE /W4 /permissive-)
else()
    target_compile_options(cf_design_model PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(copperfin_inspect
    apps/copperfin_inspect/main.cpp
)

target_link_libraries(copperfin_inspect PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(copperfin_inspect PRIVATE /W4 /permissive-)
else()
    target_compile_options(copperfin_inspect PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(copperfin_studio_host
    apps/copperfin_studio_host/main.cpp
)

target_link_libraries(copperfin_studio_host PRIVATE cf_design_model)

if(MSVC)
    target_compile_options(copperfin_studio_host PRIVATE /W4 /permissive-)
else()
    target_compile_options(copperfin_studio_host PRIVATE -Wall -Wextra -Wpedantic)
endif()

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
endif()
