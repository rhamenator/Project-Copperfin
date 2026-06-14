function(copperfin_add_test TARGET_NAME LINK_LIBS)
    add_executable(${TARGET_NAME} ${ARGN})
    target_link_libraries(${TARGET_NAME} PRIVATE ${LINK_LIBS})

    if(MSVC)
        target_compile_options(${TARGET_NAME} PRIVATE /W4 /permissive-)
    else()
        target_compile_options(${TARGET_NAME} PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    add_test(NAME ${TARGET_NAME} COMMAND $<TARGET_FILE:${TARGET_NAME}>)
endfunction()

find_program(DOTNET_EXECUTABLE NAMES dotnet)
if(DOTNET_EXECUTABLE)
    add_test(
        NAME test_managed_compile
        COMMAND ${CMAKE_COMMAND}
            -DDOTNET_EXECUTABLE=${DOTNET_EXECUTABLE}
            -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
            -DTEST_CONFIGURATION=$<CONFIG>
            -P ${CMAKE_CURRENT_SOURCE_DIR}/run_managed_compile_check.cmake
    )
endif()

add_test(
    NAME test_runtime_host_binding
    COMMAND ${CMAKE_COMMAND}
        -DRUNTIME_HOST_EXECUTABLE=$<TARGET_FILE:copperfin_runtime_host>
        -P ${CMAKE_CURRENT_SOURCE_DIR}/run_runtime_host_binding_check.cmake
)

add_executable(test_runtime_host_debug_output
    test_runtime_host_debug_output.cpp
)
target_link_libraries(test_runtime_host_debug_output PRIVATE cf_design_model cf_xbase_runtime cf_vfp_assets)

if(MSVC)
    target_compile_options(test_runtime_host_debug_output PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_runtime_host_debug_output PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_runtime_host_debug_output
    COMMAND $<TARGET_FILE:test_runtime_host_debug_output> $<TARGET_FILE:copperfin_runtime_host>
)

copperfin_add_test(test_vfp_assets cf_vfp_assets
    test_vfp_assets.cpp
)

copperfin_add_test(test_studio_host cf_design_model
    test_studio_host.cpp
)

copperfin_add_test(test_dbf_table cf_vfp_assets
    test_dbf_table.cpp
)

copperfin_add_test(test_visual_asset_editor cf_vfp_assets
    test_visual_asset_editor.cpp
)

copperfin_add_test(test_product_subsystems cf_design_model
    test_product_subsystems.cpp
)

copperfin_add_test(test_report_layout cf_design_model
    test_report_layout.cpp
)

copperfin_add_test(test_project_workspace cf_design_model
    test_project_workspace.cpp
)

copperfin_add_test(test_platform_models "cf_security;cf_platform_profile"
    test_platform_models.cpp
)

copperfin_add_test(test_query_translator cf_platform_profile
    test_query_translator.cpp
)

copperfin_add_test(test_federation_execution cf_platform_profile
    test_federation_execution.cpp
)

copperfin_add_test(test_runtime_pipeline cf_runtime_pipeline
    test_runtime_pipeline.cpp
)

copperfin_add_test(test_security_controls cf_security
    test_security_controls.cpp
)

copperfin_add_test(test_prg_engine cf_xbase_runtime
    test_prg_engine.cpp
    prg_engine_test_support.cpp
)

copperfin_add_test(test_prg_engine_work_areas cf_xbase_runtime
    test_prg_engine_work_areas.cpp
    prg_engine_test_support.cpp
)

copperfin_add_test(test_prg_engine_seek_index cf_xbase_runtime
    test_prg_engine_seek_index.cpp
    prg_engine_test_support.cpp
)

copperfin_add_test(test_prg_engine_index_seek_optimization cf_xbase_runtime
    test_prg_engine_index_seek_optimization.cpp
    prg_engine_test_support.cpp
)

copperfin_add_test(test_prg_engine_rushmore_optimization cf_xbase_runtime
    test_prg_engine_rushmore_optimization.cpp
    prg_engine_test_support.cpp
)

copperfin_add_test(test_prg_engine_control_flow cf_xbase_runtime
    test_prg_engine_control_flow.cpp
    prg_engine_test_support.cpp
)

copperfin_add_test(test_prg_engine_data_io cf_xbase_runtime
    test_prg_engine_data_io.cpp
    prg_engine_test_support.cpp
)

copperfin_add_test(test_prg_engine_arrays cf_xbase_runtime
    test_prg_engine_arrays.cpp
    prg_engine_test_support.cpp
)

copperfin_add_test(test_prg_engine_functions cf_xbase_runtime
    test_prg_engine_functions.cpp
    prg_engine_test_support.cpp
)

copperfin_add_test(test_prg_engine_runtime_surface_functions cf_xbase_runtime
    test_prg_engine_runtime_surface_functions.cpp
    prg_engine_test_support.cpp
)

copperfin_add_test(test_prg_engine_file_io_functions cf_xbase_runtime
    test_prg_engine_file_io_functions.cpp
    prg_engine_test_support.cpp
)

copperfin_add_test(test_prg_engine_aggregate_array_functions cf_xbase_runtime
    test_prg_engine_aggregate_array_functions.cpp
    prg_engine_test_support.cpp
)

copperfin_add_test(test_prg_engine_string_math_functions cf_xbase_runtime
    test_prg_engine_string_math_functions.cpp
    prg_engine_test_support.cpp
)

copperfin_add_test(test_prg_engine_path_functions cf_xbase_runtime
    test_prg_engine_path_functions.cpp
    prg_engine_test_support.cpp
)

copperfin_add_test(test_prg_engine_date_time_functions cf_xbase_runtime
    test_prg_engine_date_time_functions.cpp
    prg_engine_test_support.cpp
)

copperfin_add_test(test_prg_engine_sql_cursors cf_xbase_runtime
    test_prg_engine_sql_cursors.cpp
    prg_engine_test_support.cpp
)

copperfin_add_test(test_prg_engine_table_mutation cf_xbase_runtime
    test_prg_engine_table_mutation.cpp
    prg_engine_test_support.cpp
)

copperfin_add_test(test_prg_engine_table_structure cf_xbase_runtime
    test_prg_engine_table_structure.cpp
    prg_engine_test_support.cpp
)

copperfin_add_test(test_xasset_methods cf_xbase_runtime
    test_xasset_methods.cpp
)
