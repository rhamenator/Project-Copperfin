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
    test_studio_host_json_shared_test_helpers.cpp
    test_studio_host_json_diagnostics_appearance_and_drawing.cpp
    test_studio_host_json_diagnostics_target_selectors.cpp
    test_studio_host_json_diagnostics_values.cpp
    test_studio_host_json_diagnostics_general.cpp
    test_studio_host_json_parse_diagnostics_toolbox.cpp
    test_studio_host_json_parse_diagnostics_visual.cpp
    test_studio_host_json_parse_diagnostics_core.cpp
    test_studio_host_json_dispatch_toolbox_catalog_and_admission.cpp
    test_studio_host_json_dispatch_toolbox_palette.cpp
    test_studio_host_json_dispatch_toolbox_object_creation_plans.cpp
    test_studio_host_json_dispatch_builder.cpp
    test_studio_host_json_dispatch_designer.cpp
    test_studio_host_json_dispatch_editor_action.cpp
    test_studio_host_json_object_lifecycle.cpp
    test_studio_host_json_layout_actions.cpp
    test_studio_host_json_behavior_window_chrome.cpp
    test_studio_host_json_behavior_grid_and_selection.cpp
    test_studio_host_json_behavior_data_and_control.cpp
    test_studio_host_json_behavior_general_state.cpp
    test_studio_host_json_appearance_colors.cpp
    test_studio_host_json_appearance_fonts.cpp
    test_studio_host_json_appearance_pictures_and_icons.cpp
    test_studio_host_json_appearance_borders_fill_draw.cpp
    test_studio_host_json_appearance_window_and_display.cpp
    test_studio_host_json_appearance_ole_and_drag.cpp
    test_studio_host_json_setters_data.cpp
    test_studio_host_json_report_labels.cpp
    test_studio_host_json_report_grouping_expressions.cpp
    test_studio_host_json_report_visual_properties.cpp
    test_studio_host_json_report_layout_classifications.cpp
    test_studio_host_json_geometry_preview_bounds.cpp
    test_studio_host_json_geometry_dimensions.cpp
    test_studio_host_json_geometry_font_metadata.cpp
    test_studio_host_json_geometry_live_edit_actions.cpp
    test_studio_host_json_geometry_column_width.cpp
    test_studio_host_json_settings_page_layout_fields.cpp
    test_studio_host_json_settings_memo_parsing.cpp
    test_studio_host_json_settings_direct_field_validation.cpp
    test_studio_host_json_settings_column_setup.cpp
    test_studio_host_json_settings_diagnostics_and_misc.cpp
    test_studio_host_json_sections_band_exposure.cpp
    test_studio_host_json_sections_dhf_lifecycle.cpp
    test_studio_host_json_sections_dhf_expressions_and_exposure.cpp
    test_studio_host_json_sections_diagnostics_and_misc.cpp
    test_studio_host_json_sections_geometry.cpp
    test_studio_host_json_sections_move_and_placement.cpp
    test_studio_host_json_objects_diagnostics_and_misc.cpp
    test_studio_host_json_objects_dhf_layout_actions.cpp
    test_studio_host_json_objects_dhf_lifecycle.cpp
    test_studio_host_json_objects_dhf_expressions_and_exposure.cpp
    test_studio_host_json_objects_band_exposure.cpp
    test_studio_host_json_objects_layout_and_expressions.cpp
    test_studio_host_json_objects_visual_batches.cpp
    test_studio_host_json_report_selection_resolution.cpp
    test_studio_host_json_report_memo_and_placeholders.cpp
    test_studio_host_json_report_shared_fixtures.cpp
    test_studio_host_json_visual_editor_json_methods.cpp
    test_studio_host_json_visual_editor_json_objects.cpp
    test_studio_host_json_visual_editor_json_properties.cpp
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

add_executable(test_studio_host_deleted_update_batch
    test_studio_host_deleted_update_batch.cpp
)
target_link_libraries(test_studio_host_deleted_update_batch PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_deleted_update_batch PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_deleted_update_batch PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_deleted_update_batch
    COMMAND $<TARGET_FILE:test_studio_host_deleted_update_batch> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_deleted_update_batch_errors
    test_studio_host_deleted_update_batch_errors.cpp
)
target_link_libraries(test_studio_host_deleted_update_batch_errors PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_deleted_update_batch_errors PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_deleted_update_batch_errors PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_deleted_update_batch_errors
    COMMAND $<TARGET_FILE:test_studio_host_deleted_update_batch_errors> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_update_batch
    test_studio_host_update_batch.cpp
)
target_link_libraries(test_studio_host_update_batch PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_update_batch PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_update_batch PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_update_batch
    COMMAND $<TARGET_FILE:test_studio_host_update_batch> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_update_batch_errors
    test_studio_host_update_batch_errors.cpp
)
target_link_libraries(test_studio_host_update_batch_errors PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_update_batch_errors PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_update_batch_errors PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_update_batch_errors
    COMMAND $<TARGET_FILE:test_studio_host_update_batch_errors> $<TARGET_FILE:copperfin_studio_host>
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

add_executable(test_studio_host_deleted_object_rename_batch
    test_studio_host_deleted_object_rename_batch.cpp
)
target_link_libraries(test_studio_host_deleted_object_rename_batch PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_deleted_object_rename_batch PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_deleted_object_rename_batch PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_deleted_object_rename_batch
    COMMAND $<TARGET_FILE:test_studio_host_deleted_object_rename_batch> $<TARGET_FILE:copperfin_studio_host>
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

add_executable(test_studio_host_deleted_object_duplicate_batch
    test_studio_host_deleted_object_duplicate_batch.cpp
)
target_link_libraries(test_studio_host_deleted_object_duplicate_batch PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_deleted_object_duplicate_batch PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_deleted_object_duplicate_batch PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_deleted_object_duplicate_batch
    COMMAND $<TARGET_FILE:test_studio_host_deleted_object_duplicate_batch> $<TARGET_FILE:copperfin_studio_host>
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

add_executable(test_studio_host_deleted_object_reorder_batch
    test_studio_host_deleted_object_reorder_batch.cpp
)
target_link_libraries(test_studio_host_deleted_object_reorder_batch PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_deleted_object_reorder_batch PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_deleted_object_reorder_batch PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_deleted_object_reorder_batch
    COMMAND $<TARGET_FILE:test_studio_host_deleted_object_reorder_batch> $<TARGET_FILE:copperfin_studio_host>
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

add_executable(test_studio_host_object_reorder_batch
    test_studio_host_object_reorder_batch.cpp
)
target_link_libraries(test_studio_host_object_reorder_batch PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_object_reorder_batch PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_object_reorder_batch PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_object_reorder_batch
    COMMAND $<TARGET_FILE:test_studio_host_object_reorder_batch> $<TARGET_FILE:copperfin_studio_host>
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

add_executable(test_studio_host_object_duplicate_batch
    test_studio_host_object_duplicate_batch.cpp
)
target_link_libraries(test_studio_host_object_duplicate_batch PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_object_duplicate_batch PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_object_duplicate_batch PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_object_duplicate_batch
    COMMAND $<TARGET_FILE:test_studio_host_object_duplicate_batch> $<TARGET_FILE:copperfin_studio_host>
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

add_executable(test_studio_host_object_rename_batch
    test_studio_host_object_rename_batch.cpp
)
target_link_libraries(test_studio_host_object_rename_batch PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_object_rename_batch PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_object_rename_batch PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_object_rename_batch
    COMMAND $<TARGET_FILE:test_studio_host_object_rename_batch> $<TARGET_FILE:copperfin_studio_host>
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

add_executable(test_studio_host_object_deleted_states
    test_studio_host_object_deleted_states.cpp
)
target_link_libraries(test_studio_host_object_deleted_states PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_object_deleted_states PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_object_deleted_states PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_object_deleted_states
    COMMAND $<TARGET_FILE:test_studio_host_object_deleted_states> $<TARGET_FILE:copperfin_studio_host>
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

add_executable(test_studio_host_mixed_deleted_states
    test_studio_host_mixed_deleted_states.cpp
)
target_link_libraries(test_studio_host_mixed_deleted_states PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_mixed_deleted_states PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_mixed_deleted_states PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_mixed_deleted_states
    COMMAND $<TARGET_FILE:test_studio_host_mixed_deleted_states> $<TARGET_FILE:copperfin_studio_host>
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

add_executable(test_studio_host_settings_section_deleted_states
    test_studio_host_settings_section_deleted_states.cpp
)
target_link_libraries(test_studio_host_settings_section_deleted_states PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_settings_section_deleted_states PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_settings_section_deleted_states PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_settings_section_deleted_states
    COMMAND $<TARGET_FILE:test_studio_host_settings_section_deleted_states> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_page_margin_stable_settings
    test_studio_host_page_margin_stable_settings.cpp
)
target_link_libraries(test_studio_host_page_margin_stable_settings PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_page_margin_stable_settings PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_page_margin_stable_settings PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_page_margin_stable_settings
    COMMAND $<TARGET_FILE:test_studio_host_page_margin_stable_settings> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_page_margin_record_settings
    test_studio_host_page_margin_record_settings.cpp
)
target_link_libraries(test_studio_host_page_margin_record_settings PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_page_margin_record_settings PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_page_margin_record_settings PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_page_margin_record_settings
    COMMAND $<TARGET_FILE:test_studio_host_page_margin_record_settings> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_bottom_margin_stable_settings
    test_studio_host_bottom_margin_stable_settings.cpp
)
target_link_libraries(test_studio_host_bottom_margin_stable_settings PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_bottom_margin_stable_settings PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_bottom_margin_stable_settings PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_bottom_margin_stable_settings
    COMMAND $<TARGET_FILE:test_studio_host_bottom_margin_stable_settings> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_bottom_margin_record_settings
    test_studio_host_bottom_margin_record_settings.cpp
)
target_link_libraries(test_studio_host_bottom_margin_record_settings PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_bottom_margin_record_settings PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_bottom_margin_record_settings PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_bottom_margin_record_settings
    COMMAND $<TARGET_FILE:test_studio_host_bottom_margin_record_settings> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_vertical_grid_stable_settings
    test_studio_host_vertical_grid_stable_settings.cpp
)
target_link_libraries(test_studio_host_vertical_grid_stable_settings PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_vertical_grid_stable_settings PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_vertical_grid_stable_settings PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_vertical_grid_stable_settings
    COMMAND $<TARGET_FILE:test_studio_host_vertical_grid_stable_settings> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_vertical_grid_record_settings
    test_studio_host_vertical_grid_record_settings.cpp
)
target_link_libraries(test_studio_host_vertical_grid_record_settings PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_vertical_grid_record_settings PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_vertical_grid_record_settings PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_vertical_grid_record_settings
    COMMAND $<TARGET_FILE:test_studio_host_vertical_grid_record_settings> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_horizontal_grid_stable_settings
    test_studio_host_horizontal_grid_stable_settings.cpp
)
target_link_libraries(test_studio_host_horizontal_grid_stable_settings PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_horizontal_grid_stable_settings PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_horizontal_grid_stable_settings PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_horizontal_grid_stable_settings
    COMMAND $<TARGET_FILE:test_studio_host_horizontal_grid_stable_settings> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_horizontal_grid_record_settings
    test_studio_host_horizontal_grid_record_settings.cpp
)
target_link_libraries(test_studio_host_horizontal_grid_record_settings PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_horizontal_grid_record_settings PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_horizontal_grid_record_settings PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_horizontal_grid_record_settings
    COMMAND $<TARGET_FILE:test_studio_host_horizontal_grid_record_settings> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_orientation_stable_settings
    test_studio_host_orientation_stable_settings.cpp
)
target_link_libraries(test_studio_host_orientation_stable_settings PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_orientation_stable_settings PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_orientation_stable_settings PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_orientation_stable_settings
    COMMAND $<TARGET_FILE:test_studio_host_orientation_stable_settings> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_orientation_record_settings
    test_studio_host_orientation_record_settings.cpp
)
target_link_libraries(test_studio_host_orientation_record_settings PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_orientation_record_settings PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_orientation_record_settings PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_orientation_record_settings
    COMMAND $<TARGET_FILE:test_studio_host_orientation_record_settings> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_paper_size_stable_settings
    test_studio_host_paper_size_stable_settings.cpp
)
target_link_libraries(test_studio_host_paper_size_stable_settings PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_paper_size_stable_settings PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_paper_size_stable_settings PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_paper_size_stable_settings
    COMMAND $<TARGET_FILE:test_studio_host_paper_size_stable_settings> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_paper_size_record_settings
    test_studio_host_paper_size_record_settings.cpp
)
target_link_libraries(test_studio_host_paper_size_record_settings PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_paper_size_record_settings PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_paper_size_record_settings PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_paper_size_record_settings
    COMMAND $<TARGET_FILE:test_studio_host_paper_size_record_settings> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_settings_memo_stable
    test_studio_host_settings_memo_stable.cpp
)
target_link_libraries(test_studio_host_settings_memo_stable PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_settings_memo_stable PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_settings_memo_stable PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_settings_memo_stable
    COMMAND $<TARGET_FILE:test_studio_host_settings_memo_stable> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_settings_delete_stable
    test_studio_host_settings_delete_stable.cpp
)
target_link_libraries(test_studio_host_settings_delete_stable PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_settings_delete_stable PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_settings_delete_stable PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_settings_delete_stable
    COMMAND $<TARGET_FILE:test_studio_host_settings_delete_stable> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_settings_delete_record
    test_studio_host_settings_delete_record.cpp
)
target_link_libraries(test_studio_host_settings_delete_record PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_settings_delete_record PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_settings_delete_record PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_settings_delete_record
    COMMAND $<TARGET_FILE:test_studio_host_settings_delete_record> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_settings_restore_record
    test_studio_host_settings_restore_record.cpp
)
target_link_libraries(test_studio_host_settings_restore_record PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_settings_restore_record PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_settings_restore_record PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_settings_restore_record
    COMMAND $<TARGET_FILE:test_studio_host_settings_restore_record> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_settings_stable
    test_studio_host_selected_settings_stable.cpp
)
target_link_libraries(test_studio_host_selected_settings_stable PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_settings_stable PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_settings_stable PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_settings_stable
    COMMAND $<TARGET_FILE:test_studio_host_selected_settings_stable> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_sections_stable
    test_studio_host_selected_sections_stable.cpp
)
target_link_libraries(test_studio_host_selected_sections_stable PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_sections_stable PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_sections_stable PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_sections_stable
    COMMAND $<TARGET_FILE:test_studio_host_selected_sections_stable> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_page_header_sections_stable
    test_studio_host_selected_page_header_sections_stable.cpp
)
target_link_libraries(test_studio_host_selected_page_header_sections_stable PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_page_header_sections_stable PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_page_header_sections_stable PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_page_header_sections_stable
    COMMAND $<TARGET_FILE:test_studio_host_selected_page_header_sections_stable> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_objects_stable
    test_studio_host_selected_objects_stable.cpp
)
target_link_libraries(test_studio_host_selected_objects_stable PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_objects_stable PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_objects_stable PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_objects_stable
    COMMAND $<TARGET_FILE:test_studio_host_selected_objects_stable> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_summary_objects_stable
    test_studio_host_selected_summary_objects_stable.cpp
)
target_link_libraries(test_studio_host_selected_summary_objects_stable PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_summary_objects_stable PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_summary_objects_stable PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_summary_objects_stable
    COMMAND $<TARGET_FILE:test_studio_host_selected_summary_objects_stable> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_group_header_objects_stable
    test_studio_host_selected_group_header_objects_stable.cpp
)
target_link_libraries(test_studio_host_selected_group_header_objects_stable PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_group_header_objects_stable PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_group_header_objects_stable PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_group_header_objects_stable
    COMMAND $<TARGET_FILE:test_studio_host_selected_group_header_objects_stable> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_group_footer_objects_stable
    test_studio_host_selected_group_footer_objects_stable.cpp
)
target_link_libraries(test_studio_host_selected_group_footer_objects_stable PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_group_footer_objects_stable PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_group_footer_objects_stable PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_group_footer_objects_stable
    COMMAND $<TARGET_FILE:test_studio_host_selected_group_footer_objects_stable> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_title_objects_stable
    test_studio_host_selected_title_objects_stable.cpp
)
target_link_libraries(test_studio_host_selected_title_objects_stable PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_title_objects_stable PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_title_objects_stable PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_title_objects_stable
    COMMAND $<TARGET_FILE:test_studio_host_selected_title_objects_stable> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_page_footer_objects_stable
    test_studio_host_selected_page_footer_objects_stable.cpp
)
target_link_libraries(test_studio_host_selected_page_footer_objects_stable PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_page_footer_objects_stable PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_page_footer_objects_stable PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_page_footer_objects_stable
    COMMAND $<TARGET_FILE:test_studio_host_selected_page_footer_objects_stable> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_column_header_objects_stable
    test_studio_host_selected_column_header_objects_stable.cpp
)
target_link_libraries(test_studio_host_selected_column_header_objects_stable PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_column_header_objects_stable PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_column_header_objects_stable PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_column_header_objects_stable
    COMMAND $<TARGET_FILE:test_studio_host_selected_column_header_objects_stable> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_column_footer_objects_stable
    test_studio_host_selected_column_footer_objects_stable.cpp
)
target_link_libraries(test_studio_host_selected_column_footer_objects_stable PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_column_footer_objects_stable PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_column_footer_objects_stable PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_column_footer_objects_stable
    COMMAND $<TARGET_FILE:test_studio_host_selected_column_footer_objects_stable> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_unplaced_objects_stable
    test_studio_host_selected_unplaced_objects_stable.cpp
)
target_link_libraries(test_studio_host_selected_unplaced_objects_stable PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_unplaced_objects_stable PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_unplaced_objects_stable PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_unplaced_objects_stable
    COMMAND $<TARGET_FILE:test_studio_host_selected_unplaced_objects_stable> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_page_header_objects_stable
    test_studio_host_selected_page_header_objects_stable.cpp
)
target_link_libraries(test_studio_host_selected_page_header_objects_stable PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_page_header_objects_stable PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_page_header_objects_stable PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_page_header_objects_stable
    COMMAND $<TARGET_FILE:test_studio_host_selected_page_header_objects_stable> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_page_header_objects_orphaned
    test_studio_host_selected_page_header_objects_orphaned.cpp
)
target_link_libraries(test_studio_host_selected_page_header_objects_orphaned PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_page_header_objects_orphaned PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_page_header_objects_orphaned PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_page_header_objects_orphaned
    COMMAND $<TARGET_FILE:test_studio_host_selected_page_header_objects_orphaned> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_detail_objects_orphaned
    test_studio_host_selected_detail_objects_orphaned.cpp
)
target_link_libraries(test_studio_host_selected_detail_objects_orphaned PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_detail_objects_orphaned PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_detail_objects_orphaned PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_detail_objects_orphaned
    COMMAND $<TARGET_FILE:test_studio_host_selected_detail_objects_orphaned> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_summary_objects_record
    test_studio_host_selected_summary_objects_record.cpp
)
target_link_libraries(test_studio_host_selected_summary_objects_record PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_summary_objects_record PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_summary_objects_record PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_summary_objects_record
    COMMAND $<TARGET_FILE:test_studio_host_selected_summary_objects_record> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_summary_sections_record
    test_studio_host_selected_summary_sections_record.cpp
)
target_link_libraries(test_studio_host_selected_summary_sections_record PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_summary_sections_record PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_summary_sections_record PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_summary_sections_record
    COMMAND $<TARGET_FILE:test_studio_host_selected_summary_sections_record> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_title_sections_record
    test_studio_host_selected_title_sections_record.cpp
)
target_link_libraries(test_studio_host_selected_title_sections_record PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_title_sections_record PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_title_sections_record PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_title_sections_record
    COMMAND $<TARGET_FILE:test_studio_host_selected_title_sections_record> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_column_header_sections_record
    test_studio_host_selected_column_header_sections_record.cpp
)
target_link_libraries(test_studio_host_selected_column_header_sections_record PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_column_header_sections_record PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_column_header_sections_record PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_column_header_sections_record
    COMMAND $<TARGET_FILE:test_studio_host_selected_column_header_sections_record> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_column_footer_sections_record
    test_studio_host_selected_column_footer_sections_record.cpp
)
target_link_libraries(test_studio_host_selected_column_footer_sections_record PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_column_footer_sections_record PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_column_footer_sections_record PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_column_footer_sections_record
    COMMAND $<TARGET_FILE:test_studio_host_selected_column_footer_sections_record> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_page_footer_sections_record
    test_studio_host_selected_page_footer_sections_record.cpp
)
target_link_libraries(test_studio_host_selected_page_footer_sections_record PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_page_footer_sections_record PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_page_footer_sections_record PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_page_footer_sections_record
    COMMAND $<TARGET_FILE:test_studio_host_selected_page_footer_sections_record> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_page_header_sections_record
    test_studio_host_selected_page_header_sections_record.cpp
)
target_link_libraries(test_studio_host_selected_page_header_sections_record PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_page_header_sections_record PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_page_header_sections_record PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_page_header_sections_record
    COMMAND $<TARGET_FILE:test_studio_host_selected_page_header_sections_record> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_group_header_sections_record
    test_studio_host_selected_group_header_sections_record.cpp
)
target_link_libraries(test_studio_host_selected_group_header_sections_record PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_group_header_sections_record PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_group_header_sections_record PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_group_header_sections_record
    COMMAND $<TARGET_FILE:test_studio_host_selected_group_header_sections_record> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_group_footer_sections_record
    test_studio_host_selected_group_footer_sections_record.cpp
)
target_link_libraries(test_studio_host_selected_group_footer_sections_record PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_group_footer_sections_record PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_group_footer_sections_record PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_group_footer_sections_record
    COMMAND $<TARGET_FILE:test_studio_host_selected_group_footer_sections_record> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_nested_group_sections_record
    test_studio_host_selected_nested_group_sections_record.cpp
)
target_link_libraries(test_studio_host_selected_nested_group_sections_record PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_nested_group_sections_record PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_nested_group_sections_record PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_nested_group_sections_record
    COMMAND $<TARGET_FILE:test_studio_host_selected_nested_group_sections_record> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_section_deleted_object_counts
    test_studio_host_section_deleted_object_counts.cpp
)
target_link_libraries(test_studio_host_section_deleted_object_counts PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_section_deleted_object_counts PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_section_deleted_object_counts PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_section_deleted_object_counts
    COMMAND $<TARGET_FILE:test_studio_host_section_deleted_object_counts> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_report_sections
    test_studio_host_selected_report_sections.cpp
)
target_link_libraries(test_studio_host_selected_report_sections PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_report_sections PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_report_sections PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_report_sections
    COMMAND $<TARGET_FILE:test_studio_host_selected_report_sections> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_report_objects
    test_studio_host_selected_report_objects.cpp
)
target_link_libraries(test_studio_host_selected_report_objects PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_report_objects PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_report_objects PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_report_objects
    COMMAND $<TARGET_FILE:test_studio_host_selected_report_objects> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_report_settings
    test_studio_host_selected_report_settings.cpp
)
target_link_libraries(test_studio_host_selected_report_settings PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_report_settings PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_report_settings PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_report_settings
    COMMAND $<TARGET_FILE:test_studio_host_selected_report_settings> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_label_objects
    test_studio_host_selected_label_objects.cpp
)
target_link_libraries(test_studio_host_selected_label_objects PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_label_objects PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_label_objects PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_label_objects
    COMMAND $<TARGET_FILE:test_studio_host_selected_label_objects> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_label_sections
    test_studio_host_selected_label_sections.cpp
)
target_link_libraries(test_studio_host_selected_label_sections PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_label_sections PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_label_sections PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_label_sections
    COMMAND $<TARGET_FILE:test_studio_host_selected_label_sections> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_label_settings
    test_studio_host_selected_label_settings.cpp
)
target_link_libraries(test_studio_host_selected_label_settings PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_label_settings PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_label_settings PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_label_settings
    COMMAND $<TARGET_FILE:test_studio_host_selected_label_settings> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_summary_sections_stable
    test_studio_host_selected_summary_sections_stable.cpp
)
target_link_libraries(test_studio_host_selected_summary_sections_stable PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_summary_sections_stable PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_summary_sections_stable PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_summary_sections_stable
    COMMAND $<TARGET_FILE:test_studio_host_selected_summary_sections_stable> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_title_sections_stable
    test_studio_host_selected_title_sections_stable.cpp
)
target_link_libraries(test_studio_host_selected_title_sections_stable PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_title_sections_stable PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_title_sections_stable PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_title_sections_stable
    COMMAND $<TARGET_FILE:test_studio_host_selected_title_sections_stable> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_column_header_sections_stable
    test_studio_host_selected_column_header_sections_stable.cpp
)
target_link_libraries(test_studio_host_selected_column_header_sections_stable PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_column_header_sections_stable PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_column_header_sections_stable PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_column_header_sections_stable
    COMMAND $<TARGET_FILE:test_studio_host_selected_column_header_sections_stable> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_column_footer_sections_stable
    test_studio_host_selected_column_footer_sections_stable.cpp
)
target_link_libraries(test_studio_host_selected_column_footer_sections_stable PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_column_footer_sections_stable PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_column_footer_sections_stable PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_column_footer_sections_stable
    COMMAND $<TARGET_FILE:test_studio_host_selected_column_footer_sections_stable> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_page_footer_sections_stable
    test_studio_host_selected_page_footer_sections_stable.cpp
)
target_link_libraries(test_studio_host_selected_page_footer_sections_stable PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_page_footer_sections_stable PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_page_footer_sections_stable PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_page_footer_sections_stable
    COMMAND $<TARGET_FILE:test_studio_host_selected_page_footer_sections_stable> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_settings_restore_stable
    test_studio_host_settings_restore_stable.cpp
)
target_link_libraries(test_studio_host_settings_restore_stable PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_settings_restore_stable PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_settings_restore_stable PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_settings_restore_stable
    COMMAND $<TARGET_FILE:test_studio_host_settings_restore_stable> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_settings_memo_record
    test_studio_host_settings_memo_record.cpp
)
target_link_libraries(test_studio_host_settings_memo_record PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_settings_memo_record PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_settings_memo_record PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_settings_memo_record
    COMMAND $<TARGET_FILE:test_studio_host_settings_memo_record> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_column_count_record_settings
    test_studio_host_column_count_record_settings.cpp
)
target_link_libraries(test_studio_host_column_count_record_settings PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_column_count_record_settings PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_column_count_record_settings PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_column_count_record_settings
    COMMAND $<TARGET_FILE:test_studio_host_column_count_record_settings> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_column_width_record_settings
    test_studio_host_column_width_record_settings.cpp
)
target_link_libraries(test_studio_host_column_width_record_settings PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_column_width_record_settings PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_column_width_record_settings PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_column_width_record_settings
    COMMAND $<TARGET_FILE:test_studio_host_column_width_record_settings> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_column_spacing_record_settings
    test_studio_host_column_spacing_record_settings.cpp
)
target_link_libraries(test_studio_host_column_spacing_record_settings PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_column_spacing_record_settings PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_column_spacing_record_settings PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_column_spacing_record_settings
    COMMAND $<TARGET_FILE:test_studio_host_column_spacing_record_settings> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_column_spacing_stable_settings
    test_studio_host_column_spacing_stable_settings.cpp
)
target_link_libraries(test_studio_host_column_spacing_stable_settings PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_column_spacing_stable_settings PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_column_spacing_stable_settings PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_column_spacing_stable_settings
    COMMAND $<TARGET_FILE:test_studio_host_column_spacing_stable_settings> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_column_setup_record_settings
    test_studio_host_column_setup_record_settings.cpp
)
target_link_libraries(test_studio_host_column_setup_record_settings PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_column_setup_record_settings PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_column_setup_record_settings PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_column_setup_record_settings
    COMMAND $<TARGET_FILE:test_studio_host_column_setup_record_settings> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_column_setup_stable_settings
    test_studio_host_column_setup_stable_settings.cpp
)
target_link_libraries(test_studio_host_column_setup_stable_settings PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_column_setup_stable_settings PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_column_setup_stable_settings PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_column_setup_stable_settings
    COMMAND $<TARGET_FILE:test_studio_host_column_setup_stable_settings> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_section_delete_record_selection
    test_studio_host_section_delete_record_selection.cpp
)
target_link_libraries(test_studio_host_section_delete_record_selection PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_section_delete_record_selection PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_section_delete_record_selection PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_section_delete_record_selection
    COMMAND $<TARGET_FILE:test_studio_host_section_delete_record_selection> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_section_delete_stable_selection
    test_studio_host_section_delete_stable_selection.cpp
)
target_link_libraries(test_studio_host_section_delete_stable_selection PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_section_delete_stable_selection PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_section_delete_stable_selection PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_section_delete_stable_selection
    COMMAND $<TARGET_FILE:test_studio_host_section_delete_stable_selection> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_section_restore_record_selection
    test_studio_host_section_restore_record_selection.cpp
)
target_link_libraries(test_studio_host_section_restore_record_selection PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_section_restore_record_selection PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_section_restore_record_selection PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_section_restore_record_selection
    COMMAND $<TARGET_FILE:test_studio_host_section_restore_record_selection> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_section_restore_stable_selection
    test_studio_host_section_restore_stable_selection.cpp
)
target_link_libraries(test_studio_host_section_restore_stable_selection PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_section_restore_stable_selection PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_section_restore_stable_selection PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_section_restore_stable_selection
    COMMAND $<TARGET_FILE:test_studio_host_section_restore_stable_selection> $<TARGET_FILE:copperfin_studio_host>
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

add_executable(test_studio_host_subtree_deleted_state
    test_studio_host_subtree_deleted_state.cpp
)
target_link_libraries(test_studio_host_subtree_deleted_state PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_subtree_deleted_state PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_subtree_deleted_state PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_subtree_deleted_state
    COMMAND $<TARGET_FILE:test_studio_host_subtree_deleted_state> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_subtree_duplicate_errors
    test_studio_host_subtree_duplicate_errors.cpp
)
target_link_libraries(test_studio_host_subtree_duplicate_errors PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_subtree_duplicate_errors PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_subtree_duplicate_errors PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_subtree_duplicate_errors
    COMMAND $<TARGET_FILE:test_studio_host_subtree_duplicate_errors> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_subtree_duplicate_collision_errors
    test_studio_host_subtree_duplicate_collision_errors.cpp
)
target_link_libraries(test_studio_host_subtree_duplicate_collision_errors PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_subtree_duplicate_collision_errors PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_subtree_duplicate_collision_errors PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_subtree_duplicate_collision_errors
    COMMAND $<TARGET_FILE:test_studio_host_subtree_duplicate_collision_errors> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_subtree_duplicate
    test_studio_host_subtree_duplicate.cpp
)
target_link_libraries(test_studio_host_subtree_duplicate PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_subtree_duplicate PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_subtree_duplicate PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_subtree_duplicate
    COMMAND $<TARGET_FILE:test_studio_host_subtree_duplicate> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_deleted_subtree_duplicate_errors
    test_studio_host_deleted_subtree_duplicate_errors.cpp
)
target_link_libraries(test_studio_host_deleted_subtree_duplicate_errors PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_deleted_subtree_duplicate_errors PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_deleted_subtree_duplicate_errors PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_deleted_subtree_duplicate_errors
    COMMAND $<TARGET_FILE:test_studio_host_deleted_subtree_duplicate_errors> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_deleted_subtree_duplicate
    test_studio_host_deleted_subtree_duplicate.cpp
)
target_link_libraries(test_studio_host_deleted_subtree_duplicate PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_deleted_subtree_duplicate PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_deleted_subtree_duplicate PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_deleted_subtree_duplicate
    COMMAND $<TARGET_FILE:test_studio_host_deleted_subtree_duplicate> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_deleted_subtree_duplicate_collision_errors
    test_studio_host_deleted_subtree_duplicate_collision_errors.cpp
)
target_link_libraries(test_studio_host_deleted_subtree_duplicate_collision_errors PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_deleted_subtree_duplicate_collision_errors PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_deleted_subtree_duplicate_collision_errors PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_deleted_subtree_duplicate_collision_errors
    COMMAND $<TARGET_FILE:test_studio_host_deleted_subtree_duplicate_collision_errors> $<TARGET_FILE:copperfin_studio_host>
)

copperfin_add_test(test_vfp_assets cf_vfp_assets
    test_vfp_assets.cpp
)

copperfin_add_test(test_studio_host cf_design_model
    test_studio_host.cpp
    test_studio_host_shared_test_helpers.cpp
    test_studio_host_document_and_snapshot.cpp
    test_studio_host_diagnostics_and_session.cpp
    test_studio_host_selection_context.cpp
    test_studio_host_property_commands.cpp
    test_studio_host_object_lifecycle.cpp
    test_studio_host_layout_actions.cpp
    test_studio_host_behavior_window_chrome.cpp
    test_studio_host_behavior_grid_and_selection.cpp
    test_studio_host_behavior_data_and_control.cpp
    test_studio_host_behavior_general_state.cpp
    test_studio_host_appearance_colors.cpp
    test_studio_host_appearance_fonts.cpp
    test_studio_host_appearance_pictures_and_icons.cpp
    test_studio_host_appearance_borders_fill_draw.cpp
    test_studio_host_appearance_window_and_display.cpp
    test_studio_host_appearance_ole_and_drag.cpp
    test_studio_host_setters_data.cpp
)

copperfin_add_test(test_dbf_table cf_vfp_assets
    test_dbf_table.cpp
)

copperfin_add_test(test_visual_asset_editor cf_vfp_assets
    test_visual_asset_editor.cpp
    test_visual_asset_editor_shared_test_helpers.cpp
    test_visual_asset_editor_object_lifecycle.cpp
    test_visual_asset_editor_methods.cpp
    test_visual_asset_editor_properties.cpp
    test_visual_asset_editor_hierarchy_and_listing.cpp
    test_visual_asset_editor_update_batch.cpp
    test_visual_asset_editor_setters_behavior.cpp
    test_visual_asset_editor_appearance_colors.cpp
    test_visual_asset_editor_appearance_fonts.cpp
    test_visual_asset_editor_appearance_pictures_and_icons.cpp
    test_visual_asset_editor_appearance_borders_fill_draw.cpp
    test_visual_asset_editor_appearance_window_and_display.cpp
    test_visual_asset_editor_appearance_ole_and_drag.cpp
    test_visual_asset_editor_setters_data.cpp
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
    test_runtime_pipeline_shared_test_helpers.cpp
    test_runtime_pipeline_output_packaging.cpp
    test_runtime_pipeline_startup_and_staging.cpp
    test_runtime_pipeline_launcher_and_security.cpp
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
    test_prg_engine_control_flow_shared_test_helpers.cpp
    test_prg_engine_control_flow_control_flow_basics.cpp
    test_prg_engine_control_flow_variables_and_scope.cpp
    test_prg_engine_control_flow_aggregate_and_total_commands.cpp
    test_prg_engine_control_flow_concurrency_and_critical_sections.cpp
    test_prg_engine_control_flow_error_handling_and_faults.cpp
    test_prg_engine_control_flow_runtime_guardrails_and_lifecycle.cpp
    test_prg_engine_control_flow_routine_calls.cpp
    prg_engine_test_support.cpp
)

copperfin_add_test(test_prg_engine_data_io cf_xbase_runtime
    test_prg_engine_data_io.cpp
    test_prg_engine_data_io_shared_test_helpers.cpp
    test_prg_engine_data_io_scatter_gather.cpp
    test_prg_engine_data_io_import_export.cpp
    test_prg_engine_data_io_save_restore.cpp
    test_prg_engine_data_io_display_and_list.cpp
    test_prg_engine_data_io_interactive_io_commands.cpp
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
