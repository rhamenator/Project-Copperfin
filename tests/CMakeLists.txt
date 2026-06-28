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

add_executable(test_localization
    test_localization.cpp
    prg_engine_test_support.cpp
)
target_link_libraries(test_localization PRIVATE cf_localization cf_runtime_pipeline cf_xbase_runtime)

if(MSVC)
    target_compile_options(test_localization PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_localization PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_localization
    COMMAND $<TARGET_FILE:test_localization> $<TARGET_FILE:copperfin_inspect>
)

add_executable(test_runtime_host_debug_output
    test_runtime_host_debug_output.cpp
)
target_link_libraries(test_runtime_host_debug_output PRIVATE cf_design_model cf_xbase_runtime cf_vfp_assets)
target_link_libraries(test_runtime_host_debug_output PRIVATE cf_security)

if(MSVC)
    target_compile_options(test_runtime_host_debug_output PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_runtime_host_debug_output PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_runtime_host_debug_output
    COMMAND $<TARGET_FILE:test_runtime_host_debug_output> $<TARGET_FILE:copperfin_runtime_host>
)

add_executable(test_build_host_output
    test_build_host_output.cpp
)
target_link_libraries(test_build_host_output PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_build_host_output PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_build_host_output PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_build_host_output
    COMMAND $<TARGET_FILE:test_build_host_output> $<TARGET_FILE:copperfin_build_host>
)

add_executable(test_studio_host_json
    test_studio_host_json.cpp
)
target_link_libraries(test_studio_host_json PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_json PRIVATE /W4 /permissive-)
    target_compile_options(test_studio_host_json PRIVATE /bigobj)
else()
    target_compile_options(test_studio_host_json PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_json
    COMMAND $<TARGET_FILE:test_studio_host_json> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_report_section_ids
    test_studio_host_report_section_ids.cpp
)
target_link_libraries(test_studio_host_report_section_ids PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_report_section_ids PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_report_section_ids PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_report_section_ids
    COMMAND $<TARGET_FILE:test_studio_host_report_section_ids> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_deleted_band_object_sections
    test_studio_host_deleted_band_object_sections.cpp
)
target_link_libraries(test_studio_host_deleted_band_object_sections PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_deleted_band_object_sections PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_deleted_band_object_sections PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_deleted_band_object_sections
    COMMAND $<TARGET_FILE:test_studio_host_deleted_band_object_sections> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_deleted_update_batch_sections
    test_studio_host_deleted_update_batch_sections.cpp
)
target_link_libraries(test_studio_host_deleted_update_batch_sections PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_deleted_update_batch_sections PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_deleted_update_batch_sections PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_deleted_update_batch_sections
    COMMAND $<TARGET_FILE:test_studio_host_deleted_update_batch_sections> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_deleted_property_clear_sections
    test_studio_host_deleted_property_clear_sections.cpp
)
target_link_libraries(test_studio_host_deleted_property_clear_sections PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_deleted_property_clear_sections PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_deleted_property_clear_sections PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_deleted_property_clear_sections
    COMMAND $<TARGET_FILE:test_studio_host_deleted_property_clear_sections> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_deleted_property_clear_batch_sections
    test_studio_host_deleted_property_clear_batch_sections.cpp
)
target_link_libraries(test_studio_host_deleted_property_clear_batch_sections PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_deleted_property_clear_batch_sections PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_deleted_property_clear_batch_sections PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_deleted_property_clear_batch_sections
    COMMAND $<TARGET_FILE:test_studio_host_deleted_property_clear_batch_sections> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_deleted_property_copy_sections
    test_studio_host_deleted_property_copy_sections.cpp
)
target_link_libraries(test_studio_host_deleted_property_copy_sections PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_deleted_property_copy_sections PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_deleted_property_copy_sections PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_deleted_property_copy_sections
    COMMAND $<TARGET_FILE:test_studio_host_deleted_property_copy_sections> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_deleted_property_copy_batch_sections
    test_studio_host_deleted_property_copy_batch_sections.cpp
)
target_link_libraries(test_studio_host_deleted_property_copy_batch_sections PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_deleted_property_copy_batch_sections PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_deleted_property_copy_batch_sections PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_deleted_property_copy_batch_sections
    COMMAND $<TARGET_FILE:test_studio_host_deleted_property_copy_batch_sections> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_deleted_property_move_sections
    test_studio_host_deleted_property_move_sections.cpp
)
target_link_libraries(test_studio_host_deleted_property_move_sections PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_deleted_property_move_sections PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_deleted_property_move_sections PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_deleted_property_move_sections
    COMMAND $<TARGET_FILE:test_studio_host_deleted_property_move_sections> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_deleted_property_move_batch_sections
    test_studio_host_deleted_property_move_batch_sections.cpp
)
target_link_libraries(test_studio_host_deleted_property_move_batch_sections PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_deleted_property_move_batch_sections PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_deleted_property_move_batch_sections PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_deleted_property_move_batch_sections
    COMMAND $<TARGET_FILE:test_studio_host_deleted_property_move_batch_sections> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_deleted_property_rejection_sections
    test_studio_host_deleted_property_rejection_sections.cpp
)
target_link_libraries(test_studio_host_deleted_property_rejection_sections PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_deleted_property_rejection_sections PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_deleted_property_rejection_sections PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_deleted_property_rejection_sections
    COMMAND $<TARGET_FILE:test_studio_host_deleted_property_rejection_sections> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_deleted_property_batch_rejection_sections
    test_studio_host_deleted_property_batch_rejection_sections.cpp
)
target_link_libraries(test_studio_host_deleted_property_batch_rejection_sections PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_deleted_property_batch_rejection_sections PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_deleted_property_batch_rejection_sections PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_deleted_property_batch_rejection_sections
    COMMAND $<TARGET_FILE:test_studio_host_deleted_property_batch_rejection_sections> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_deleted_object_rename_batch_sections
    test_studio_host_deleted_object_rename_batch_sections.cpp
)
target_link_libraries(test_studio_host_deleted_object_rename_batch_sections PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_deleted_object_rename_batch_sections PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_deleted_object_rename_batch_sections PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_deleted_object_rename_batch_sections
    COMMAND $<TARGET_FILE:test_studio_host_deleted_object_rename_batch_sections> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_deleted_object_duplicate_batch_sections
    test_studio_host_deleted_object_duplicate_batch_sections.cpp
)
target_link_libraries(test_studio_host_deleted_object_duplicate_batch_sections PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_deleted_object_duplicate_batch_sections PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_deleted_object_duplicate_batch_sections PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_deleted_object_duplicate_batch_sections
    COMMAND $<TARGET_FILE:test_studio_host_deleted_object_duplicate_batch_sections> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_deleted_object_reorder_batch_sections
    test_studio_host_deleted_object_reorder_batch_sections.cpp
)
target_link_libraries(test_studio_host_deleted_object_reorder_batch_sections PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_deleted_object_reorder_batch_sections PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_deleted_object_reorder_batch_sections PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_deleted_object_reorder_batch_sections
    COMMAND $<TARGET_FILE:test_studio_host_deleted_object_reorder_batch_sections> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_deleted_object_reorder_batch_errors
    test_studio_host_deleted_object_reorder_batch_errors.cpp
)
target_link_libraries(test_studio_host_deleted_object_reorder_batch_errors PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_deleted_object_reorder_batch_errors PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_deleted_object_reorder_batch_errors PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_deleted_object_reorder_batch_errors
    COMMAND $<TARGET_FILE:test_studio_host_deleted_object_reorder_batch_errors> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_deleted_object_duplicate_batch_errors
    test_studio_host_deleted_object_duplicate_batch_errors.cpp
)
target_link_libraries(test_studio_host_deleted_object_duplicate_batch_errors PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_deleted_object_duplicate_batch_errors PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_deleted_object_duplicate_batch_errors PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_deleted_object_duplicate_batch_errors
    COMMAND $<TARGET_FILE:test_studio_host_deleted_object_duplicate_batch_errors> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_deleted_object_rename_batch_errors
    test_studio_host_deleted_object_rename_batch_errors.cpp
)
target_link_libraries(test_studio_host_deleted_object_rename_batch_errors PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_deleted_object_rename_batch_errors PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_deleted_object_rename_batch_errors PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_deleted_object_rename_batch_errors
    COMMAND $<TARGET_FILE:test_studio_host_deleted_object_rename_batch_errors> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_object_reorder_batch_errors
    test_studio_host_object_reorder_batch_errors.cpp
)
target_link_libraries(test_studio_host_object_reorder_batch_errors PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_object_reorder_batch_errors PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_object_reorder_batch_errors PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_object_reorder_batch_errors
    COMMAND $<TARGET_FILE:test_studio_host_object_reorder_batch_errors> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_object_duplicate_batch_errors
    test_studio_host_object_duplicate_batch_errors.cpp
)
target_link_libraries(test_studio_host_object_duplicate_batch_errors PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_object_duplicate_batch_errors PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_object_duplicate_batch_errors PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_object_duplicate_batch_errors
    COMMAND $<TARGET_FILE:test_studio_host_object_duplicate_batch_errors> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_object_rename_batch_errors
    test_studio_host_object_rename_batch_errors.cpp
)
target_link_libraries(test_studio_host_object_rename_batch_errors PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_object_rename_batch_errors PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_object_rename_batch_errors PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_object_rename_batch_errors
    COMMAND $<TARGET_FILE:test_studio_host_object_rename_batch_errors> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_object_deleted_states_errors
    test_studio_host_object_deleted_states_errors.cpp
)
target_link_libraries(test_studio_host_object_deleted_states_errors PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_object_deleted_states_errors PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_object_deleted_states_errors PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_object_deleted_states_errors
    COMMAND $<TARGET_FILE:test_studio_host_object_deleted_states_errors> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_mixed_deleted_states_errors
    test_studio_host_mixed_deleted_states_errors.cpp
)
target_link_libraries(test_studio_host_mixed_deleted_states_errors PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_mixed_deleted_states_errors PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_mixed_deleted_states_errors PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_mixed_deleted_states_errors
    COMMAND $<TARGET_FILE:test_studio_host_mixed_deleted_states_errors> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_settings_section_deleted_states_errors
    test_studio_host_settings_section_deleted_states_errors.cpp
)
target_link_libraries(test_studio_host_settings_section_deleted_states_errors PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_settings_section_deleted_states_errors PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_settings_section_deleted_states_errors PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_settings_section_deleted_states_errors
    COMMAND $<TARGET_FILE:test_studio_host_settings_section_deleted_states_errors> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_subtree_deleted_state_errors
    test_studio_host_subtree_deleted_state_errors.cpp
)
target_link_libraries(test_studio_host_subtree_deleted_state_errors PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_subtree_deleted_state_errors PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_subtree_deleted_state_errors PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_subtree_deleted_state_errors
    COMMAND $<TARGET_FILE:test_studio_host_subtree_deleted_state_errors> $<TARGET_FILE:copperfin_studio_host>
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
if(MSVC)
    target_compile_options(test_visual_asset_editor PRIVATE /bigobj)
endif()

copperfin_add_test(test_product_subsystems cf_design_model
    test_product_subsystems.cpp
)

copperfin_add_test(test_builder_registry cf_design_model
    test_builder_registry.cpp
)

copperfin_add_test(test_toolbox_palette cf_design_model
    test_toolbox_palette.cpp
)

copperfin_add_test(test_toolbox_creation cf_design_model
    test_toolbox_creation.cpp
)

copperfin_add_test(test_context_editor_actions cf_design_model
    test_context_editor_actions.cpp
)

copperfin_add_test(test_designer_context cf_design_model
    test_designer_context.cpp
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
