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
    NAME test_package_document_install
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
        -DBINARY_DIR=${CMAKE_BINARY_DIR}
        -DTEST_ROOT_BASE=${CMAKE_CURRENT_BINARY_DIR}/package-document-install
        -DTEST_CONFIGURATION=$<CONFIG>
        -P ${CMAKE_CURRENT_SOURCE_DIR}/run_package_document_install_check.cmake
)

add_test(
    NAME test_runtime_host_binding
    COMMAND ${CMAKE_COMMAND}
        -DRUNTIME_HOST_EXECUTABLE=$<TARGET_FILE:copperfin_runtime_host>
        -DLOCALE_ROOT=${CMAKE_SOURCE_DIR}/resources/locales
        -P ${CMAKE_CURRENT_SOURCE_DIR}/run_runtime_host_binding_check.cmake
)

add_executable(test_localization
    test_localization.cpp
    prg_engine_test_support.cpp
)
target_link_libraries(test_localization PRIVATE cf_localization cf_runtime_pipeline cf_xbase_runtime)
add_dependencies(test_localization copperfin_inspect)

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
add_dependencies(test_runtime_host_debug_output copperfin_runtime_host)

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
add_dependencies(test_build_host_output copperfin_build_host copperfin_runtime_host)

if(MSVC)
    target_compile_options(test_build_host_output PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_build_host_output PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_build_host_output
    COMMAND $<TARGET_FILE:test_build_host_output> $<TARGET_FILE:copperfin_build_host>
)

add_executable(test_studio_host_shell_command
    test_studio_host_shell_command.cpp
    ../apps/copperfin_studio_host/studio_host_main_shell_command.cpp
)
target_include_directories(test_studio_host_shell_command PRIVATE
    ${CMAKE_SOURCE_DIR}/apps/copperfin_studio_host
    ${CMAKE_SOURCE_DIR}/include
)

if(MSVC)
    target_compile_options(test_studio_host_shell_command PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_shell_command PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_shell_command
    COMMAND $<TARGET_FILE:test_studio_host_shell_command>
)

add_executable(test_licensing_status
    test_licensing_status.cpp
)
target_link_libraries(test_licensing_status PRIVATE cf_licensing)
target_include_directories(test_licensing_status PRIVATE ${CMAKE_SOURCE_DIR}/src/licensing)

if(MSVC)
    target_compile_options(test_licensing_status PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_licensing_status PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_licensing_status
    COMMAND $<TARGET_FILE:test_licensing_status>
)

add_executable(test_process_success test_process_exit.cpp)
target_compile_definitions(test_process_success PRIVATE COPPERFIN_TEST_PROCESS_EXIT_CODE=0)

add_executable(test_process_failure test_process_exit.cpp)
target_compile_definitions(test_process_failure PRIVATE COPPERFIN_TEST_PROCESS_EXIT_CODE=1)

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
    test_studio_host_json_geometry_preview_helpers.cpp
    test_studio_host_json_objects_dhf_shared_fixtures.cpp
    test_studio_host_json_report_settings_shared_fixtures.cpp
    test_studio_host_json_report_section_shared_fixtures.cpp
    test_studio_host_json_sections_dhf_shared_fixtures.cpp
    test_studio_host_json_sections_band_exposure.cpp
    test_studio_host_json_objects_band_exposure.cpp
    test_studio_host_json_report_shared_fixtures.cpp
    test_studio_host_json_visual_editor_json_methods.cpp
    test_studio_host_json_visual_editor_json_objects.cpp
    test_studio_host_json_visual_editor_json_properties.cpp
)
target_link_libraries(test_studio_host_json PRIVATE cf_vfp_assets)
target_compile_definitions(test_studio_host_json PRIVATE
    COPPERFIN_TEST_SUCCESS_COMMAND="$<TARGET_FILE:test_process_success>"
    COPPERFIN_TEST_FAILURE_COMMAND="$<TARGET_FILE:test_process_failure>"
    COPPERFIN_OBJECT_LIFECYCLE_SKIP_TOOLBOX_CREATE
    COPPERFIN_OBJECT_LIFECYCLE_SKIP_STABLE_SELECTOR_MUTATION
    COPPERFIN_OBJECT_LIFECYCLE_SKIP_OBJECT_ACTION_LIFECYCLE
    COPPERFIN_OBJECT_LIFECYCLE_SKIP_REMAINING_OBJECT_ACTIONS
    COPPERFIN_DETAIL_HEADER_FOOTER_SECTION_SKIP_EXPRESSIONS
    COPPERFIN_DETAIL_HEADER_FOOTER_SECTION_SKIP_HOST_SMOKE
    COPPERFIN_DETAIL_HEADER_FOOTER_SECTION_LIFECYCLE_SKIP_HOST_SMOKE
    COPPERFIN_DETAIL_HEADER_FOOTER_OBJECT_EXPRESSIONS_SKIP_HOST_SMOKE
    COPPERFIN_DETAIL_HEADER_FOOTER_OBJECT_LIFECYCLE_SKIP_HOST_SMOKE
    COPPERFIN_DETAIL_HEADER_FOOTER_OBJECT_GEOMETRY_SKIP_STABLE
    COPPERFIN_REPORT_DIRECT_SETTING_FIELDS_SKIP_HOST_SMOKE
    COPPERFIN_REPORT_SETTINGS_DIAGNOSTICS_SKIP_HOST_SMOKE
    COPPERFIN_REPORT_UNRESOLVED_MEMO_SKIP_HOST_SMOKE
    COPPERFIN_REPORT_SCHEMA_FALLBACK_SKIP_HOST_SMOKE
    COPPERFIN_REPORT_DELETED_STATES_SKIP_HOST_SMOKE
    COPPERFIN_REPORT_LAYOUT_DIAGNOSTICS_SKIP_HOST_SMOKE
    COPPERFIN_REPORT_LAYOUT_DIAGNOSTICS_SKIP_HELPERS
    COPPERFIN_REPORT_OBJECT_FALLBACKS_SKIP_HELPERS
    COPPERFIN_REPORT_LAYOUT_WIDTH_LEFT_PREVIEW_BOUNDS_SKIP_HOST_SMOKE
    COPPERFIN_REPORT_LAYOUT_HEIGHT_TOP_PREVIEW_BOUNDS_SKIP_HOST_SMOKE
    COPPERFIN_DETAIL_HEADER_FOOTER_SECTION_PREVIEW_BOUNDS_SKIP_HOST_SMOKE
    COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_SKIP_UNRESOLVED_AND_MISSING_HELPERS
)
add_dependencies(test_studio_host_json test_process_success test_process_failure)

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

add_executable(test_studio_host_toolbox_create_lifecycle
    test_studio_host_toolbox_create_lifecycle.cpp
    test_studio_host_json_object_lifecycle.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_toolbox_create_lifecycle PRIVATE cf_vfp_assets)
target_compile_definitions(test_studio_host_toolbox_create_lifecycle PRIVATE
    COPPERFIN_OBJECT_LIFECYCLE_ONLY_TOOLBOX_CREATE
)

if(MSVC)
    target_compile_options(test_studio_host_toolbox_create_lifecycle PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_toolbox_create_lifecycle PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_toolbox_create_lifecycle
    COMMAND $<TARGET_FILE:test_studio_host_toolbox_create_lifecycle> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_stable_selector_mutation
    test_studio_host_stable_selector_mutation.cpp
    test_studio_host_json_object_lifecycle.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_stable_selector_mutation PRIVATE cf_vfp_assets)
target_compile_definitions(test_studio_host_stable_selector_mutation PRIVATE
    COPPERFIN_OBJECT_LIFECYCLE_SKIP_TOOLBOX_CREATE
    COPPERFIN_OBJECT_LIFECYCLE_ONLY_STABLE_SELECTOR_MUTATION
)

if(MSVC)
    target_compile_options(test_studio_host_stable_selector_mutation PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_stable_selector_mutation PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_stable_selector_mutation
    COMMAND $<TARGET_FILE:test_studio_host_stable_selector_mutation> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_object_action_lifecycle
    test_studio_host_object_action_lifecycle.cpp
    test_studio_host_json_object_lifecycle.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_object_action_lifecycle PRIVATE cf_vfp_assets)
target_compile_definitions(test_studio_host_object_action_lifecycle PRIVATE
    COPPERFIN_OBJECT_LIFECYCLE_SKIP_TOOLBOX_CREATE
    COPPERFIN_OBJECT_LIFECYCLE_SKIP_STABLE_SELECTOR_MUTATION
    COPPERFIN_OBJECT_LIFECYCLE_ONLY_OBJECT_ACTION_LIFECYCLE
)

if(MSVC)
    target_compile_options(test_studio_host_object_action_lifecycle PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_object_action_lifecycle PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_object_action_lifecycle
    COMMAND $<TARGET_FILE:test_studio_host_object_action_lifecycle> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_remaining_object_lifecycle
    test_studio_host_remaining_object_lifecycle.cpp
    test_studio_host_json_object_lifecycle.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_remaining_object_lifecycle PRIVATE cf_vfp_assets)
target_compile_definitions(test_studio_host_remaining_object_lifecycle PRIVATE
    COPPERFIN_OBJECT_LIFECYCLE_SKIP_TOOLBOX_CREATE
    COPPERFIN_OBJECT_LIFECYCLE_SKIP_STABLE_SELECTOR_MUTATION
    COPPERFIN_OBJECT_LIFECYCLE_SKIP_OBJECT_ACTION_LIFECYCLE
)

if(MSVC)
    target_compile_options(test_studio_host_remaining_object_lifecycle PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_remaining_object_lifecycle PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_remaining_object_lifecycle
    COMMAND $<TARGET_FILE:test_studio_host_remaining_object_lifecycle> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_detail_header_footer_section_expressions
    test_studio_host_detail_header_footer_section_expressions.cpp
    test_studio_host_json_sections_dhf_shared_fixtures.cpp
    test_studio_host_json_sections_dhf_expressions_and_exposure.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_detail_header_footer_section_expressions PRIVATE cf_vfp_assets)
target_compile_definitions(test_studio_host_detail_header_footer_section_expressions PRIVATE
    COPPERFIN_DETAIL_HEADER_FOOTER_SECTION_ONLY_EXPRESSIONS
)

if(MSVC)
    target_compile_options(test_studio_host_detail_header_footer_section_expressions PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_detail_header_footer_section_expressions PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_detail_header_footer_section_expressions
    COMMAND $<TARGET_FILE:test_studio_host_detail_header_footer_section_expressions> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_detail_header_footer_section_geometry
    test_studio_host_detail_header_footer_section_geometry.cpp
    test_studio_host_json_sections_dhf_shared_fixtures.cpp
    test_studio_host_json_sections_dhf_expressions_and_exposure.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_detail_header_footer_section_geometry PRIVATE cf_vfp_assets)
target_compile_definitions(test_studio_host_detail_header_footer_section_geometry PRIVATE
    COPPERFIN_DETAIL_HEADER_FOOTER_SECTION_SKIP_EXPRESSIONS
)

if(MSVC)
    target_compile_options(test_studio_host_detail_header_footer_section_geometry PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_detail_header_footer_section_geometry PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_detail_header_footer_section_geometry
    COMMAND $<TARGET_FILE:test_studio_host_detail_header_footer_section_geometry> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_detail_header_footer_section_lifecycle
    test_studio_host_detail_header_footer_section_lifecycle.cpp
    test_studio_host_json_objects_dhf_shared_fixtures.cpp
    test_studio_host_json_sections_dhf_shared_fixtures.cpp
    test_studio_host_json_sections_dhf_lifecycle.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_detail_header_footer_section_lifecycle PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_detail_header_footer_section_lifecycle PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_detail_header_footer_section_lifecycle PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_detail_header_footer_section_lifecycle
    COMMAND $<TARGET_FILE:test_studio_host_detail_header_footer_section_lifecycle> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_detail_header_footer_object_geometry
    test_studio_host_detail_header_footer_object_geometry.cpp
    test_studio_host_json_geometry_dimensions.cpp
    test_studio_host_json_geometry_preview_bounds.cpp
    test_studio_host_json_objects_dhf_shared_fixtures.cpp
    test_studio_host_json_objects_dhf_expressions_and_exposure.cpp
    test_studio_host_json_report_shared_fixtures.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_detail_header_footer_object_geometry PRIVATE cf_vfp_assets)
target_compile_definitions(test_studio_host_detail_header_footer_object_geometry PRIVATE
    COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_ONLY_HELPERS
)

if(MSVC)
    target_compile_options(test_studio_host_detail_header_footer_object_geometry PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_detail_header_footer_object_geometry PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_detail_header_footer_object_geometry
    COMMAND $<TARGET_FILE:test_studio_host_detail_header_footer_object_geometry> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_detail_header_footer_object_expressions
    test_studio_host_detail_header_footer_object_expressions.cpp
    test_studio_host_json_objects_dhf_shared_fixtures.cpp
    test_studio_host_json_objects_dhf_expressions_and_exposure.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_detail_header_footer_object_expressions PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_detail_header_footer_object_expressions PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_detail_header_footer_object_expressions PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_detail_header_footer_object_expressions
    COMMAND $<TARGET_FILE:test_studio_host_detail_header_footer_object_expressions> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_detail_header_footer_object_lifecycle
    test_studio_host_detail_header_footer_object_lifecycle.cpp
    test_studio_host_json_objects_dhf_shared_fixtures.cpp
    test_studio_host_json_objects_dhf_lifecycle.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_detail_header_footer_object_lifecycle PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_detail_header_footer_object_lifecycle PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_detail_header_footer_object_lifecycle PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_detail_header_footer_object_lifecycle
    COMMAND $<TARGET_FILE:test_studio_host_detail_header_footer_object_lifecycle> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_detail_header_footer_object_layout_actions
    test_studio_host_detail_header_footer_object_layout_actions.cpp
    test_studio_host_json_objects_dhf_shared_fixtures.cpp
    test_studio_host_json_objects_dhf_layout_actions.cpp
    test_studio_host_json_objects_dhf_expressions_and_exposure.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_detail_header_footer_object_layout_actions PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_detail_header_footer_object_layout_actions PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_detail_header_footer_object_layout_actions PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_detail_header_footer_object_layout_actions
    COMMAND $<TARGET_FILE:test_studio_host_detail_header_footer_object_layout_actions> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_report_layout_classifications
    test_studio_host_report_layout_classifications.cpp
    test_studio_host_json_report_layout_classifications.cpp
    test_studio_host_json_geometry_preview_bounds.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_report_layout_classifications PRIVATE cf_vfp_assets)
target_compile_definitions(test_studio_host_report_layout_classifications PRIVATE
    COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_ONLY_HELPERS
)

if(MSVC)
    target_compile_options(test_studio_host_report_layout_classifications PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_report_layout_classifications PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_report_layout_classifications
    COMMAND $<TARGET_FILE:test_studio_host_report_layout_classifications> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_report_direct_setting_fields
    test_studio_host_report_direct_setting_fields.cpp
    test_studio_host_json_geometry_preview_bounds.cpp
    test_studio_host_json_settings_column_setup.cpp
    test_studio_host_json_settings_direct_field_validation.cpp
    test_studio_host_json_settings_page_layout_fields.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_report_direct_setting_fields PRIVATE cf_vfp_assets)
target_compile_definitions(test_studio_host_report_direct_setting_fields PRIVATE
    COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_ONLY_HELPERS
)

if(MSVC)
    target_compile_options(test_studio_host_report_direct_setting_fields PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_report_direct_setting_fields PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_report_direct_setting_fields
    COMMAND $<TARGET_FILE:test_studio_host_report_direct_setting_fields> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_report_column_setup
    test_studio_host_report_column_setup.cpp
    test_studio_host_json_geometry_preview_bounds.cpp
    test_studio_host_json_settings_column_setup.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_report_column_setup PRIVATE cf_vfp_assets)
target_compile_definitions(test_studio_host_report_column_setup PRIVATE
    COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_ONLY_HELPERS
)

if(MSVC)
    target_compile_options(test_studio_host_report_column_setup PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_report_column_setup PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_report_column_setup
    COMMAND $<TARGET_FILE:test_studio_host_report_column_setup> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_report_page_setup_fields
    test_studio_host_report_page_setup_fields.cpp
    test_studio_host_json_geometry_preview_bounds.cpp
    test_studio_host_json_settings_page_layout_fields.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_report_page_setup_fields PRIVATE cf_vfp_assets)
target_compile_definitions(test_studio_host_report_page_setup_fields PRIVATE
    COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_ONLY_HELPERS
)

if(MSVC)
    target_compile_options(test_studio_host_report_page_setup_fields PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_report_page_setup_fields PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_report_page_setup_fields
    COMMAND $<TARGET_FILE:test_studio_host_report_page_setup_fields> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_report_column_width_fields
    test_studio_host_report_column_width_fields.cpp
    test_studio_host_json_geometry_preview_bounds.cpp
    test_studio_host_json_geometry_column_width.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_report_column_width_fields PRIVATE cf_vfp_assets)
target_compile_definitions(test_studio_host_report_column_width_fields PRIVATE
    COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_ONLY_HELPERS
)

if(MSVC)
    target_compile_options(test_studio_host_report_column_width_fields PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_report_column_width_fields PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_report_column_width_fields
    COMMAND $<TARGET_FILE:test_studio_host_report_column_width_fields> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_label_layout_actions
    test_studio_host_label_layout_actions.cpp
    test_studio_host_json_objects_layout_and_expressions.cpp
    test_studio_host_json_report_labels.cpp
    test_studio_host_json_geometry_preview_helpers.cpp
    test_studio_host_json_report_shared_fixtures.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_label_layout_actions PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_label_layout_actions PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_label_layout_actions PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_label_layout_actions
    COMMAND $<TARGET_FILE:test_studio_host_label_layout_actions> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_report_layout_actions
    test_studio_host_report_layout_actions.cpp
    test_studio_host_json_geometry_preview_helpers.cpp
    test_studio_host_json_objects_layout_and_expressions.cpp
    test_studio_host_json_report_shared_fixtures.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_report_layout_actions PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_report_layout_actions PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_report_layout_actions PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_report_layout_actions
    COMMAND $<TARGET_FILE:test_studio_host_report_layout_actions> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_report_font_metadata
    test_studio_host_report_font_metadata.cpp
    test_studio_host_json_geometry_preview_helpers.cpp
    test_studio_host_json_geometry_font_metadata.cpp
    test_studio_host_json_objects_dhf_shared_fixtures.cpp
    test_studio_host_json_objects_dhf_expressions_and_exposure.cpp
    test_studio_host_json_report_shared_fixtures.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_report_font_metadata PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_report_font_metadata PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_report_font_metadata PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_report_font_metadata
    COMMAND $<TARGET_FILE:test_studio_host_report_font_metadata> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_report_layout_placement
    test_studio_host_report_layout_placement.cpp
    test_studio_host_json_geometry_preview_helpers.cpp
    test_studio_host_json_sections_move_and_placement.cpp
    test_studio_host_json_report_shared_fixtures.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_report_layout_placement PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_report_layout_placement PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_report_layout_placement PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_report_layout_placement
    COMMAND $<TARGET_FILE:test_studio_host_report_layout_placement> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_report_settings_memo_parsing
    test_studio_host_report_settings_memo_parsing.cpp
    test_studio_host_json_geometry_preview_bounds.cpp
    test_studio_host_json_settings_memo_parsing.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_report_settings_memo_parsing PRIVATE cf_vfp_assets)
target_compile_definitions(test_studio_host_report_settings_memo_parsing PRIVATE
    COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_ONLY_HELPERS
)

if(MSVC)
    target_compile_options(test_studio_host_report_settings_memo_parsing PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_report_settings_memo_parsing PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_report_settings_memo_parsing
    COMMAND $<TARGET_FILE:test_studio_host_report_settings_memo_parsing> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_report_settings_diagnostics
    test_studio_host_report_settings_diagnostics.cpp
    test_studio_host_json_geometry_preview_bounds.cpp
    test_studio_host_json_report_shared_fixtures.cpp
    test_studio_host_json_settings_diagnostics_and_misc.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_report_settings_diagnostics PRIVATE cf_vfp_assets)
target_compile_definitions(test_studio_host_report_settings_diagnostics PRIVATE
    COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_ONLY_HELPERS
)

if(MSVC)
    target_compile_options(test_studio_host_report_settings_diagnostics PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_report_settings_diagnostics PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_report_settings_diagnostics
    COMMAND $<TARGET_FILE:test_studio_host_report_settings_diagnostics> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_report_section_selection_diagnostics
    test_studio_host_report_section_selection_diagnostics.cpp
    test_studio_host_json_report_section_selection_diagnostics.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_report_section_selection_diagnostics PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_report_section_selection_diagnostics PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_report_section_selection_diagnostics PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_report_section_selection_diagnostics
    COMMAND $<TARGET_FILE:test_studio_host_report_section_selection_diagnostics> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_report_live_section_geometry
    test_studio_host_report_live_section_geometry.cpp
    test_studio_host_json_geometry_preview_helpers.cpp
    test_studio_host_json_sections_geometry.cpp
    test_studio_host_json_report_shared_fixtures.cpp
    test_studio_host_json_report_section_shared_fixtures.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_report_live_section_geometry PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_report_live_section_geometry PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_report_live_section_geometry PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_report_live_section_geometry
    COMMAND $<TARGET_FILE:test_studio_host_report_live_section_geometry> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_report_unresolved_memo_placeholders
    test_studio_host_report_unresolved_memo_placeholders.cpp
    test_studio_host_json_geometry_dimensions.cpp
    test_studio_host_json_geometry_preview_bounds.cpp
    test_studio_host_json_report_object_fallbacks.cpp
    test_studio_host_json_objects_dhf_shared_fixtures.cpp
    test_studio_host_json_objects_dhf_expressions_and_exposure.cpp
    test_studio_host_json_report_memo_and_placeholders.cpp
    test_studio_host_json_report_shared_fixtures.cpp
    test_studio_host_json_report_settings_shared_fixtures.cpp
    test_studio_host_json_sections_diagnostics_and_misc.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_report_unresolved_memo_placeholders PRIVATE cf_vfp_assets)
target_compile_definitions(test_studio_host_report_unresolved_memo_placeholders PRIVATE
    COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_ONLY_HELPERS
    COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_INCLUDE_UNRESOLVED_AND_MISSING_HELPERS
)

if(MSVC)
    target_compile_options(test_studio_host_report_unresolved_memo_placeholders PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_report_unresolved_memo_placeholders PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_report_unresolved_memo_placeholders
    COMMAND $<TARGET_FILE:test_studio_host_report_unresolved_memo_placeholders> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_report_schema_fallbacks
    test_studio_host_report_schema_fallbacks.cpp
    test_studio_host_json_geometry_dimensions.cpp
    test_studio_host_json_geometry_preview_bounds.cpp
    test_studio_host_json_report_object_fallbacks.cpp
    test_studio_host_json_objects_dhf_shared_fixtures.cpp
    test_studio_host_json_objects_dhf_expressions_and_exposure.cpp
    test_studio_host_json_report_shared_fixtures.cpp
    test_studio_host_json_report_settings_shared_fixtures.cpp
    test_studio_host_json_sections_diagnostics_and_misc.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_report_schema_fallbacks PRIVATE cf_vfp_assets)
target_compile_definitions(test_studio_host_report_schema_fallbacks PRIVATE
    COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_ONLY_HELPERS
    COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_INCLUDE_UNRESOLVED_AND_MISSING_HELPERS
)

if(MSVC)
    target_compile_options(test_studio_host_report_schema_fallbacks PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_report_schema_fallbacks PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_report_schema_fallbacks
    COMMAND $<TARGET_FILE:test_studio_host_report_schema_fallbacks> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_round_trip
    test_studio_host_real_sample_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_sort_settings_round_trip
    test_studio_host_real_sample_sort_settings_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_sort_settings_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_sort_settings_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_sort_settings_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_sort_settings_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_sort_settings_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_settings_delete_restore_round_trip
    test_studio_host_real_sample_settings_delete_restore_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_settings_delete_restore_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_settings_delete_restore_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_settings_delete_restore_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_settings_delete_restore_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_settings_delete_restore_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_column_setup_round_trip
    test_studio_host_real_sample_column_setup_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_column_setup_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_column_setup_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_column_setup_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_column_setup_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_column_setup_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_paper_dimensions_round_trip
    test_studio_host_real_sample_paper_dimensions_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_paper_dimensions_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_paper_dimensions_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_paper_dimensions_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_paper_dimensions_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_paper_dimensions_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_left_margin_round_trip
    test_studio_host_real_sample_left_margin_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_left_margin_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_left_margin_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_left_margin_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_left_margin_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_left_margin_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_right_margin_round_trip
    test_studio_host_real_sample_right_margin_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_right_margin_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_right_margin_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_right_margin_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_right_margin_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_right_margin_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_driver_round_trip
    test_studio_host_real_sample_driver_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_driver_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_driver_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_driver_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_driver_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_driver_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_device_round_trip
    test_studio_host_real_sample_device_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_device_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_device_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_device_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_device_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_device_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_output_round_trip
    test_studio_host_real_sample_output_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_output_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_output_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_output_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_output_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_output_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_defaultsource_round_trip
    test_studio_host_real_sample_defaultsource_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_defaultsource_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_defaultsource_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_defaultsource_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_defaultsource_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_defaultsource_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_printquality_round_trip
    test_studio_host_real_sample_printquality_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_printquality_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_printquality_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_printquality_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_printquality_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_printquality_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_yresolution_round_trip
    test_studio_host_real_sample_yresolution_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_yresolution_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_yresolution_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_yresolution_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_yresolution_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_yresolution_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_ttoption_round_trip
    test_studio_host_real_sample_ttoption_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_ttoption_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_ttoption_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_ttoption_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_ttoption_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_ttoption_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_ascii_round_trip
    test_studio_host_real_sample_ascii_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_ascii_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_ascii_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_ascii_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_ascii_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_ascii_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_collate_round_trip
    test_studio_host_real_sample_collate_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_collate_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_collate_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_collate_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_collate_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_collate_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_copies_round_trip
    test_studio_host_real_sample_copies_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_copies_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_copies_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_copies_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_copies_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_copies_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_page_setup_settings_round_trip
    test_studio_host_real_sample_page_setup_settings_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_page_setup_settings_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_page_setup_settings_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_page_setup_settings_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_page_setup_settings_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_page_setup_settings_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_direct_margins_round_trip
    test_studio_host_real_sample_direct_margins_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_direct_margins_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_direct_margins_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_direct_margins_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_direct_margins_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_direct_margins_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_object_round_trip
    test_studio_host_real_sample_object_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_object_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_object_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_object_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_object_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_object_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_object_preview_bounds_round_trip
    test_studio_host_real_sample_object_preview_bounds_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_object_preview_bounds_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_object_preview_bounds_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_object_preview_bounds_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_object_preview_bounds_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_object_preview_bounds_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_object_nudge_round_trip
    test_studio_host_real_sample_object_nudge_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_object_nudge_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_object_nudge_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_object_nudge_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_object_nudge_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_object_nudge_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_object_align_round_trip
    test_studio_host_real_sample_object_align_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_object_align_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_object_align_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_object_align_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_object_align_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_object_align_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_object_distribute_round_trip
    test_studio_host_real_sample_object_distribute_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_object_distribute_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_object_distribute_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_object_distribute_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_object_distribute_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_object_distribute_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_object_resize_round_trip
    test_studio_host_real_sample_object_resize_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_object_resize_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_object_resize_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_object_resize_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_object_resize_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_object_resize_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_object_snap_round_trip
    test_studio_host_real_sample_object_snap_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_object_snap_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_object_snap_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_object_snap_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_object_snap_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_object_snap_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_object_duplicate_round_trip
    test_studio_host_real_sample_object_duplicate_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_object_duplicate_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_object_duplicate_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_object_duplicate_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_object_duplicate_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_object_duplicate_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_object_rename_round_trip
    test_studio_host_real_sample_object_rename_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_object_rename_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_object_rename_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_object_rename_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_object_rename_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_object_rename_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_object_reorder_round_trip
    test_studio_host_real_sample_object_reorder_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_object_reorder_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_object_reorder_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_object_reorder_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_object_reorder_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_object_reorder_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_object_memo_round_trip
    test_studio_host_real_sample_object_memo_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_object_memo_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_object_memo_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_object_memo_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_object_memo_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_object_memo_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_object_font_round_trip
    test_studio_host_real_sample_object_font_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_object_font_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_object_font_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_object_font_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_object_font_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_object_font_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_object_font_clear_round_trip
    test_studio_host_real_sample_object_font_clear_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_object_font_clear_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_object_font_clear_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_object_font_clear_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_object_font_clear_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_object_font_clear_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_object_delete_restore_round_trip
    test_studio_host_real_sample_object_delete_restore_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_object_delete_restore_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_object_delete_restore_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_object_delete_restore_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_object_delete_restore_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_object_delete_restore_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_object_deleted_preview_round_trip
    test_studio_host_real_sample_object_deleted_preview_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_object_deleted_preview_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_object_deleted_preview_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_object_deleted_preview_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_object_deleted_preview_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_object_deleted_preview_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_section_round_trip
    test_studio_host_real_sample_section_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_section_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_section_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_section_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_section_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_section_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_section_deleted_preview_round_trip
    test_studio_host_real_sample_section_deleted_preview_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_section_deleted_preview_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_section_deleted_preview_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_section_deleted_preview_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_section_deleted_preview_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_section_deleted_preview_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_section_delete_restore_round_trip
    test_studio_host_real_sample_section_delete_restore_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_section_delete_restore_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_section_delete_restore_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_section_delete_restore_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_section_delete_restore_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_section_delete_restore_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_grouping_round_trip
    test_studio_host_real_sample_grouping_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_grouping_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_grouping_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_grouping_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_grouping_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_grouping_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_real_sample_grouping_deleted_preview_round_trip
    test_studio_host_real_sample_grouping_deleted_preview_round_trip.cpp
)
target_link_libraries(test_studio_host_real_sample_grouping_deleted_preview_round_trip PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_real_sample_grouping_deleted_preview_round_trip PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_real_sample_grouping_deleted_preview_round_trip PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_real_sample_grouping_deleted_preview_round_trip
    COMMAND $<TARGET_FILE:test_studio_host_real_sample_grouping_deleted_preview_round_trip> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_deleted_section_geometry
    test_studio_host_deleted_section_geometry.cpp
    test_studio_host_json_geometry_preview_bounds.cpp
    test_studio_host_json_shared_test_helpers.cpp
    test_studio_host_json_report_shared_fixtures.cpp
)
target_link_libraries(test_studio_host_deleted_section_geometry PRIVATE cf_vfp_assets)
target_compile_definitions(test_studio_host_deleted_section_geometry PRIVATE
    COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_ONLY_HELPERS
)

if(MSVC)
    target_compile_options(test_studio_host_deleted_section_geometry PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_deleted_section_geometry PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_deleted_section_geometry
    COMMAND $<TARGET_FILE:test_studio_host_deleted_section_geometry> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_deleted_object_width_left_geometry
    test_studio_host_deleted_object_width_left_geometry.cpp
    test_studio_host_json_geometry_preview_bounds.cpp
    test_studio_host_json_shared_test_helpers.cpp
    test_studio_host_json_report_shared_fixtures.cpp
)
target_link_libraries(test_studio_host_deleted_object_width_left_geometry PRIVATE cf_vfp_assets)
target_compile_definitions(test_studio_host_deleted_object_width_left_geometry PRIVATE
    COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_ONLY_HELPERS
)

if(MSVC)
    target_compile_options(test_studio_host_deleted_object_width_left_geometry PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_deleted_object_width_left_geometry PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_deleted_object_width_left_geometry
    COMMAND $<TARGET_FILE:test_studio_host_deleted_object_width_left_geometry> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_deleted_object_height_top_geometry
    test_studio_host_deleted_object_height_top_geometry.cpp
    test_studio_host_json_geometry_preview_bounds.cpp
    test_studio_host_json_shared_test_helpers.cpp
    test_studio_host_json_report_shared_fixtures.cpp
)
target_link_libraries(test_studio_host_deleted_object_height_top_geometry PRIVATE cf_vfp_assets)
target_compile_definitions(test_studio_host_deleted_object_height_top_geometry PRIVATE
    COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_ONLY_HELPERS
)

if(MSVC)
    target_compile_options(test_studio_host_deleted_object_height_top_geometry PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_deleted_object_height_top_geometry PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_deleted_object_height_top_geometry
    COMMAND $<TARGET_FILE:test_studio_host_deleted_object_height_top_geometry> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_deleted_object_restore_geometry
    test_studio_host_deleted_object_restore_geometry.cpp
    test_studio_host_json_geometry_preview_bounds.cpp
    test_studio_host_json_shared_test_helpers.cpp
    test_studio_host_json_report_shared_fixtures.cpp
)
target_link_libraries(test_studio_host_deleted_object_restore_geometry PRIVATE cf_vfp_assets)
target_compile_definitions(test_studio_host_deleted_object_restore_geometry PRIVATE
    COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_ONLY_HELPERS
)

if(MSVC)
    target_compile_options(test_studio_host_deleted_object_restore_geometry PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_deleted_object_restore_geometry PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_deleted_object_restore_geometry
    COMMAND $<TARGET_FILE:test_studio_host_deleted_object_restore_geometry> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_edited_object_delete_geometry
    test_studio_host_edited_object_delete_geometry.cpp
    test_studio_host_json_geometry_preview_bounds.cpp
    test_studio_host_json_shared_test_helpers.cpp
    test_studio_host_json_report_shared_fixtures.cpp
)
target_link_libraries(test_studio_host_edited_object_delete_geometry PRIVATE cf_vfp_assets)
target_compile_definitions(test_studio_host_edited_object_delete_geometry PRIVATE
    COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_ONLY_HELPERS
)

if(MSVC)
    target_compile_options(test_studio_host_edited_object_delete_geometry PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_edited_object_delete_geometry PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_edited_object_delete_geometry
    COMMAND $<TARGET_FILE:test_studio_host_edited_object_delete_geometry> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_report_width_left_preview_bounds
    test_studio_host_report_width_left_preview_bounds.cpp
    test_studio_host_json_geometry_preview_bounds.cpp
    test_studio_host_json_shared_test_helpers.cpp
    test_studio_host_json_report_shared_fixtures.cpp
)
target_link_libraries(test_studio_host_report_width_left_preview_bounds PRIVATE cf_vfp_assets)
target_compile_definitions(test_studio_host_report_width_left_preview_bounds PRIVATE
    COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_SKIP_UNRESOLVED_AND_MISSING_HELPERS
    COPPERFIN_REPORT_LAYOUT_WIDTH_LEFT_PREVIEW_BOUNDS_ONLY
)

if(MSVC)
    target_compile_options(test_studio_host_report_width_left_preview_bounds PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_report_width_left_preview_bounds PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_report_width_left_preview_bounds
    COMMAND $<TARGET_FILE:test_studio_host_report_width_left_preview_bounds> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_report_height_top_preview_bounds
    test_studio_host_report_height_top_preview_bounds.cpp
    test_studio_host_json_geometry_preview_bounds.cpp
    test_studio_host_json_shared_test_helpers.cpp
    test_studio_host_json_report_shared_fixtures.cpp
)
target_link_libraries(test_studio_host_report_height_top_preview_bounds PRIVATE cf_vfp_assets)
target_compile_definitions(test_studio_host_report_height_top_preview_bounds PRIVATE
    COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_SKIP_UNRESOLVED_AND_MISSING_HELPERS
    COPPERFIN_REPORT_LAYOUT_HEIGHT_TOP_PREVIEW_BOUNDS_ONLY
)

if(MSVC)
    target_compile_options(test_studio_host_report_height_top_preview_bounds PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_report_height_top_preview_bounds PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_report_height_top_preview_bounds
    COMMAND $<TARGET_FILE:test_studio_host_report_height_top_preview_bounds> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_detail_header_footer_section_preview_bounds
    test_studio_host_detail_header_footer_section_preview_bounds.cpp
    test_studio_host_json_geometry_preview_bounds.cpp
    test_studio_host_json_objects_dhf_shared_fixtures.cpp
    test_studio_host_json_objects_dhf_expressions_and_exposure.cpp
    test_studio_host_json_sections_dhf_shared_fixtures.cpp
    test_studio_host_json_sections_dhf_expressions_and_exposure.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_detail_header_footer_section_preview_bounds PRIVATE cf_vfp_assets)
target_compile_definitions(test_studio_host_detail_header_footer_section_preview_bounds PRIVATE
    COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_SKIP_UNRESOLVED_AND_MISSING_HELPERS
    COPPERFIN_DETAIL_HEADER_FOOTER_SECTION_PREVIEW_BOUNDS_ONLY
)

if(MSVC)
    target_compile_options(test_studio_host_detail_header_footer_section_preview_bounds PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_detail_header_footer_section_preview_bounds PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_detail_header_footer_section_preview_bounds
    COMMAND $<TARGET_FILE:test_studio_host_detail_header_footer_section_preview_bounds> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_deleted_report_visual_property_rejections
    test_studio_host_deleted_report_visual_property_rejections.cpp
    test_studio_host_json_geometry_preview_bounds.cpp
    test_studio_host_json_report_visual_properties.cpp
    test_studio_host_json_report_shared_fixtures.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_deleted_report_visual_property_rejections PRIVATE cf_vfp_assets)
target_compile_definitions(test_studio_host_deleted_report_visual_property_rejections PRIVATE
    COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_ONLY_HELPERS
)

if(MSVC)
    target_compile_options(test_studio_host_deleted_report_visual_property_rejections PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_deleted_report_visual_property_rejections PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_deleted_report_visual_property_rejections
    COMMAND $<TARGET_FILE:test_studio_host_deleted_report_visual_property_rejections> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_deleted_report_visual_property_clears
    test_studio_host_deleted_report_visual_property_clears.cpp
    test_studio_host_json_geometry_preview_bounds.cpp
    test_studio_host_json_report_visual_properties.cpp
    test_studio_host_json_report_shared_fixtures.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_deleted_report_visual_property_clears PRIVATE cf_vfp_assets)
target_compile_definitions(test_studio_host_deleted_report_visual_property_clears PRIVATE
    COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_ONLY_HELPERS
    COPPERFIN_DELETED_REPORT_VISUAL_PROPERTY_CLEARS_ONLY
)

if(MSVC)
    target_compile_options(test_studio_host_deleted_report_visual_property_clears PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_deleted_report_visual_property_clears PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_deleted_report_visual_property_clears
    COMMAND $<TARGET_FILE:test_studio_host_deleted_report_visual_property_clears> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_deleted_report_visual_property_copies
    test_studio_host_deleted_report_visual_property_copies.cpp
    test_studio_host_json_geometry_preview_bounds.cpp
    test_studio_host_json_deleted_report_visual_property_copies.cpp
    test_studio_host_json_report_shared_fixtures.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_deleted_report_visual_property_copies PRIVATE cf_vfp_assets)
target_compile_definitions(test_studio_host_deleted_report_visual_property_copies PRIVATE
    COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_ONLY_HELPERS
)

if(MSVC)
    target_compile_options(test_studio_host_deleted_report_visual_property_copies PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_deleted_report_visual_property_copies PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_deleted_report_visual_property_copies
    COMMAND $<TARGET_FILE:test_studio_host_deleted_report_visual_property_copies> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_deleted_report_visual_property_moves
    test_studio_host_deleted_report_visual_property_moves.cpp
    test_studio_host_json_geometry_preview_bounds.cpp
    test_studio_host_json_report_visual_properties.cpp
    test_studio_host_json_report_shared_fixtures.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_deleted_report_visual_property_moves PRIVATE cf_vfp_assets)
target_compile_definitions(test_studio_host_deleted_report_visual_property_moves PRIVATE
    COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_ONLY_HELPERS
    COPPERFIN_DELETED_REPORT_VISUAL_PROPERTY_CLEARS_SKIP_HOST_SMOKE
    COPPERFIN_DELETED_REPORT_VISUAL_PROPERTY_MOVES_ONLY
)

if(MSVC)
    target_compile_options(test_studio_host_deleted_report_visual_property_moves PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_deleted_report_visual_property_moves PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_deleted_report_visual_property_moves
    COMMAND $<TARGET_FILE:test_studio_host_deleted_report_visual_property_moves> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_report_geometry_defaults
    test_studio_host_report_geometry_defaults.cpp
    test_studio_host_json_geometry_preview_bounds.cpp
    test_studio_host_json_shared_test_helpers.cpp
    test_studio_host_json_report_shared_fixtures.cpp
)
target_link_libraries(test_studio_host_report_geometry_defaults PRIVATE cf_vfp_assets)
target_compile_definitions(test_studio_host_report_geometry_defaults PRIVATE
    COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_ONLY_HELPERS
)

if(MSVC)
    target_compile_options(test_studio_host_report_geometry_defaults PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_report_geometry_defaults PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_report_geometry_defaults
    COMMAND $<TARGET_FILE:test_studio_host_report_geometry_defaults> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_report_deleted_states
    test_studio_host_report_deleted_states.cpp
    test_studio_host_json_geometry_preview_bounds.cpp
    test_studio_host_json_objects_diagnostics_and_misc.cpp
    test_studio_host_json_report_shared_fixtures.cpp
    test_studio_host_json_report_settings_shared_fixtures.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_report_deleted_states PRIVATE cf_vfp_assets)
target_compile_definitions(test_studio_host_report_deleted_states PRIVATE
    COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_ONLY_HELPERS
    COPPERFIN_REPORT_LAYOUT_DIAGNOSTICS_SKIP_HOST_SMOKE
    COPPERFIN_REPORT_LAYOUT_DIAGNOSTICS_SKIP_HELPERS
    COPPERFIN_REPORT_OBJECT_FALLBACKS_SKIP_HELPERS
    COPPERFIN_REPORT_SETTINGS_DIAGNOSTICS_SKIP_HOST_SMOKE
    COPPERFIN_REPORT_DELETED_STATES_ONLY
)

if(MSVC)
    target_compile_options(test_studio_host_report_deleted_states PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_report_deleted_states PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_report_deleted_states
    COMMAND $<TARGET_FILE:test_studio_host_report_deleted_states> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_report_layout_diagnostics
    test_studio_host_report_layout_diagnostics.cpp
    test_studio_host_json_geometry_preview_bounds.cpp
    test_studio_host_json_objects_diagnostics_and_misc.cpp
    test_studio_host_json_report_shared_fixtures.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_report_layout_diagnostics PRIVATE cf_vfp_assets)
target_compile_definitions(test_studio_host_report_layout_diagnostics PRIVATE
    COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_ONLY_HELPERS
    COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_INCLUDE_UNRESOLVED_AND_MISSING_HELPERS
    COPPERFIN_REPORT_DELETED_STATES_SKIP_HELPERS
    COPPERFIN_REPORT_OBJECT_FALLBACKS_SKIP_HELPERS
    COPPERFIN_REPORT_LAYOUT_DIAGNOSTICS_ONLY
)

if(MSVC)
    target_compile_options(test_studio_host_report_layout_diagnostics PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_report_layout_diagnostics PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_report_layout_diagnostics
    COMMAND $<TARGET_FILE:test_studio_host_report_layout_diagnostics> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_report_selection_none
    test_studio_host_report_selection_none.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_report_selection_none PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_report_selection_none PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_report_selection_none PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_report_selection_none
    COMMAND $<TARGET_FILE:test_studio_host_report_selection_none> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_report_selection_padded
    test_studio_host_report_selection_padded.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_report_selection_padded PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_report_selection_padded PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_report_selection_padded PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_report_selection_padded
    COMMAND $<TARGET_FILE:test_studio_host_report_selection_padded> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_report_selection_deep
    test_studio_host_report_selection_deep.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_report_selection_deep PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_report_selection_deep PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_report_selection_deep PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_report_selection_deep
    COMMAND $<TARGET_FILE:test_studio_host_report_selection_deep> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_report_selection_deep_section_settings
    test_studio_host_report_selection_deep_section_settings.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_report_selection_deep_section_settings PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_report_selection_deep_section_settings PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_report_selection_deep_section_settings PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_report_selection_deep_section_settings
    COMMAND $<TARGET_FILE:test_studio_host_report_selection_deep_section_settings> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_report_grouping_exposure
    test_studio_host_report_grouping_exposure.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_report_grouping_exposure PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_report_grouping_exposure PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_report_grouping_exposure PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_report_grouping_exposure
    COMMAND $<TARGET_FILE:test_studio_host_report_grouping_exposure> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_report_grouping_nested_mutation
    test_studio_host_report_grouping_nested_mutation.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_report_grouping_nested_mutation PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_report_grouping_nested_mutation PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_report_grouping_nested_mutation PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_report_grouping_nested_mutation
    COMMAND $<TARGET_FILE:test_studio_host_report_grouping_nested_mutation> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_report_grouping_stable_mutation
    test_studio_host_report_grouping_stable_mutation.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_report_grouping_stable_mutation PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_report_grouping_stable_mutation PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_report_grouping_stable_mutation PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_report_grouping_stable_mutation
    COMMAND $<TARGET_FILE:test_studio_host_report_grouping_stable_mutation> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_report_grouping_record_mutation
    test_studio_host_report_grouping_record_mutation.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_report_grouping_record_mutation PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_report_grouping_record_mutation PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_report_grouping_record_mutation PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_report_grouping_record_mutation
    COMMAND $<TARGET_FILE:test_studio_host_report_grouping_record_mutation> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_report_grouping_deleted_exposure
    test_studio_host_report_grouping_deleted_exposure.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_report_grouping_deleted_exposure PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_report_grouping_deleted_exposure PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_report_grouping_deleted_exposure PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_report_grouping_deleted_exposure
    COMMAND $<TARGET_FILE:test_studio_host_report_grouping_deleted_exposure> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_report_grouping_deleted_mutation
    test_studio_host_report_grouping_deleted_mutation.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_report_grouping_deleted_mutation PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_report_grouping_deleted_mutation PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_report_grouping_deleted_mutation PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_report_grouping_deleted_mutation
    COMMAND $<TARGET_FILE:test_studio_host_report_grouping_deleted_mutation> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_report_grouping_remaining_exposure
    test_studio_host_report_grouping_remaining_exposure.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_report_grouping_remaining_exposure PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_report_grouping_remaining_exposure PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_report_grouping_remaining_exposure PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_report_grouping_remaining_exposure
    COMMAND $<TARGET_FILE:test_studio_host_report_grouping_remaining_exposure> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_layout_actions
    test_studio_host_layout_actions.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_layout_actions PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_layout_actions PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_layout_actions PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_layout_actions
    COMMAND $<TARGET_FILE:test_studio_host_layout_actions> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_geometry_live_edit_actions
    test_studio_host_geometry_live_edit_actions.cpp
    test_studio_host_json_shared_test_helpers.cpp
)
target_link_libraries(test_studio_host_geometry_live_edit_actions PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_geometry_live_edit_actions PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_geometry_live_edit_actions PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_geometry_live_edit_actions
    COMMAND $<TARGET_FILE:test_studio_host_geometry_live_edit_actions> $<TARGET_FILE:copperfin_studio_host>
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

add_executable(test_studio_host_side_margin_record_settings
    test_studio_host_side_margin_record_settings.cpp
)
target_link_libraries(test_studio_host_side_margin_record_settings PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_side_margin_record_settings PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_side_margin_record_settings PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_side_margin_record_settings
    COMMAND $<TARGET_FILE:test_studio_host_side_margin_record_settings> $<TARGET_FILE:copperfin_studio_host>
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

add_executable(test_studio_host_report_sort_settings_stable
    test_studio_host_report_sort_settings_stable.cpp
)
target_link_libraries(test_studio_host_report_sort_settings_stable PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_report_sort_settings_stable PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_report_sort_settings_stable PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_report_sort_settings_stable
    COMMAND $<TARGET_FILE:test_studio_host_report_sort_settings_stable> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_report_sort_settings_record
    test_studio_host_report_sort_settings_record.cpp
)
target_link_libraries(test_studio_host_report_sort_settings_record PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_report_sort_settings_record PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_report_sort_settings_record PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_report_sort_settings_record
    COMMAND $<TARGET_FILE:test_studio_host_report_sort_settings_record> $<TARGET_FILE:copperfin_studio_host>
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

add_executable(test_studio_host_selected_group_header_sections_stable
    test_studio_host_selected_group_header_sections_stable.cpp
)
target_link_libraries(test_studio_host_selected_group_header_sections_stable PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_group_header_sections_stable PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_group_header_sections_stable PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_group_header_sections_stable
    COMMAND $<TARGET_FILE:test_studio_host_selected_group_header_sections_stable> $<TARGET_FILE:copperfin_studio_host>
)

add_executable(test_studio_host_selected_group_footer_sections_stable
    test_studio_host_selected_group_footer_sections_stable.cpp
)
target_link_libraries(test_studio_host_selected_group_footer_sections_stable PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_group_footer_sections_stable PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_group_footer_sections_stable PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_group_footer_sections_stable
    COMMAND $<TARGET_FILE:test_studio_host_selected_group_footer_sections_stable> $<TARGET_FILE:copperfin_studio_host>
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

add_executable(test_studio_host_selected_nested_group_sections_stable
    test_studio_host_selected_nested_group_sections_stable.cpp
)
target_link_libraries(test_studio_host_selected_nested_group_sections_stable PRIVATE cf_vfp_assets)

if(MSVC)
    target_compile_options(test_studio_host_selected_nested_group_sections_stable PRIVATE /W4 /permissive-)
else()
    target_compile_options(test_studio_host_selected_nested_group_sections_stable PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME test_studio_host_selected_nested_group_sections_stable
    COMMAND $<TARGET_FILE:test_studio_host_selected_nested_group_sections_stable> $<TARGET_FILE:copperfin_studio_host>
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

copperfin_add_test(test_vfp_sidecar_path cf_design_model
    test_vfp_sidecar_path.cpp
)

copperfin_add_test(test_studio_host cf_design_model
    test_studio_host.cpp
    test_studio_host_shared_test_helpers.cpp
    test_studio_host_document_and_snapshot.cpp
    test_studio_host_diagnostics_and_session.cpp
    test_studio_host_selection_context.cpp
    test_studio_host_property_commands.cpp
    test_studio_host_object_lifecycle.cpp
    test_studio_host_layout_action_parse.cpp
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
copperfin_apply_windows_stack_reserve(test_studio_host)

copperfin_add_test(test_dbf_table cf_vfp_assets
    test_dbf_table.cpp
)
add_test(
    NAME test_dbf_table_active_locale
    COMMAND $<TARGET_FILE:test_dbf_table> --active-locale-missing-field
)
set_tests_properties(
    test_dbf_table_active_locale
    PROPERTIES ENVIRONMENT
        "COPPERFIN_LOCALE=qps-ploc;COPPERFIN_LOCALE_DIR=${PROJECT_SOURCE_DIR}/resources/locales"
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

copperfin_add_test(test_report_layout_sort_settings cf_design_model
    test_report_layout_sort_settings.cpp
)

copperfin_add_test(test_report_layout_side_margin_settings cf_design_model
    test_report_layout_side_margin_settings.cpp
)

copperfin_add_test(test_project_workspace cf_design_model
    test_project_workspace.cpp
)

copperfin_add_test(test_platform_models "cf_security;cf_platform_profile"
    test_platform_models.cpp
)

copperfin_add_test(test_platform_environment cf_localization
    test_platform_environment.cpp
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
    test_runtime_pipeline_rematerialization.cpp
    test_runtime_pipeline_startup_casefold.cpp
    test_runtime_pipeline_startup_and_staging.cpp
    test_runtime_pipeline_launcher_and_security.cpp
)
target_compile_definitions(test_runtime_pipeline PRIVATE COPPERFIN_TEST_SOURCE_DIR="${CMAKE_SOURCE_DIR}")

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

copperfin_add_test(test_prg_engine_database_lifecycle cf_xbase_runtime
    test_prg_engine_database_lifecycle.cpp
    prg_engine_test_support.cpp
)

copperfin_add_test(test_prg_engine_seek_index cf_xbase_runtime
    test_prg_engine_seek_index.cpp
    prg_engine_test_support.cpp
)
if(WIN32)
    add_library(copperfin_declared_dll_fixture SHARED
        fixtures/declared_dll_fixture.cpp
    )
    target_compile_definitions(test_prg_engine_seek_index PRIVATE
        COPPERFIN_DECLARED_DLL_FIXTURE_NAME="$<TARGET_FILE_NAME:copperfin_declared_dll_fixture>"
    )
    add_dependencies(test_prg_engine_seek_index copperfin_declared_dll_fixture)

    if(MSVC)
        add_library(copperfin_mixed_mode_declared_dll_fixture SHARED
            fixtures/mixed_mode_declared_dll_fixture.cpp
        )
        set_target_properties(copperfin_mixed_mode_declared_dll_fixture PROPERTIES
            CXX_STANDARD 17
            CXX_STANDARD_REQUIRED ON
            COPPERFIN_DISABLE_CFG TRUE
        )
        target_compile_options(copperfin_mixed_mode_declared_dll_fixture PRIVATE
            /guard:cf-
            /clr
            /EHa
        )
        target_link_options(copperfin_mixed_mode_declared_dll_fixture PRIVATE /GUARD:NO)
    endif()

    find_program(COPPERFIN_FRAMEWORK_CSC
        NAMES csc.exe
        HINTS
            "$ENV{WINDIR}/Microsoft.NET/Framework/v4.0.30319"
            "$ENV{WINDIR}/Microsoft.NET/Framework64/v4.0.30319"
    )
    if(COPPERFIN_FRAMEWORK_CSC)
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(COPPERFIN_MANAGED_DECLARE_FIXTURE_PLATFORM x64)
        else()
            set(COPPERFIN_MANAGED_DECLARE_FIXTURE_PLATFORM x86)
        endif()
        set(COPPERFIN_MANAGED_DECLARE_FIXTURE_PATH
            "${CMAKE_CURRENT_BINARY_DIR}/fixtures/Copperfin.ManagedDeclareFixture.dll")
        set(COPPERFIN_MANAGED_DECLARE_FIXTURE_X86_PATH
            "${CMAKE_CURRENT_BINARY_DIR}/fixtures/Copperfin.ManagedDeclareFixture.x86.dll")
        set(COPPERFIN_MANAGED_DECLARE_FIXTURE_X64_PATH
            "${CMAKE_CURRENT_BINARY_DIR}/fixtures/Copperfin.ManagedDeclareFixture.x64.dll")
        set(COPPERFIN_MANAGED_DECLARE_DEPENDENCY_PATH
            "${CMAKE_CURRENT_BINARY_DIR}/fixtures/Copperfin.ManagedDeclareDependency.dll")
        set(COPPERFIN_MANAGED_DECLARE_MISSING_PATH
            "${CMAKE_CURRENT_BINARY_DIR}/fixtures/Copperfin.ManagedDeclareMissing.dll")
        set(COPPERFIN_MANAGED_DECLARE_RELATIVE_PATH
            "${CMAKE_CURRENT_BINARY_DIR}/fixtures/Copperfin.ManagedDeclareRelative.dll")
        set(COPPERFIN_MANAGED_DECLARE_PARENTLESS_PATH
            "${CMAKE_CURRENT_BINARY_DIR}/fixtures/Copperfin.ManagedDeclareParentless.dll")
        add_custom_command(
            OUTPUT
                "${COPPERFIN_MANAGED_DECLARE_FIXTURE_PATH}"
                "${COPPERFIN_MANAGED_DECLARE_FIXTURE_X86_PATH}"
                "${COPPERFIN_MANAGED_DECLARE_FIXTURE_X64_PATH}"
                "${COPPERFIN_MANAGED_DECLARE_DEPENDENCY_PATH}"
                "${COPPERFIN_MANAGED_DECLARE_MISSING_PATH}"
                "${COPPERFIN_MANAGED_DECLARE_RELATIVE_PATH}"
                "${COPPERFIN_MANAGED_DECLARE_PARENTLESS_PATH}"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/fixtures"
            COMMAND "${COPPERFIN_FRAMEWORK_CSC}"
                /nologo
                /target:library
                /platform:anycpu
                /out:${COPPERFIN_MANAGED_DECLARE_DEPENDENCY_PATH}
                managed_declare_dependency.cs
            COMMAND "${COPPERFIN_FRAMEWORK_CSC}"
                /nologo
                /target:library
                /platform:${COPPERFIN_MANAGED_DECLARE_FIXTURE_PLATFORM}
                /reference:${COPPERFIN_MANAGED_DECLARE_DEPENDENCY_PATH}
                /out:${COPPERFIN_MANAGED_DECLARE_FIXTURE_PATH}
                managed_declare_fixture.cs
            COMMAND "${COPPERFIN_FRAMEWORK_CSC}"
                /nologo
                /target:library
                /platform:x86
                /reference:${COPPERFIN_MANAGED_DECLARE_DEPENDENCY_PATH}
                /out:${COPPERFIN_MANAGED_DECLARE_FIXTURE_X86_PATH}
                managed_declare_fixture.cs
            COMMAND "${COPPERFIN_FRAMEWORK_CSC}"
                /nologo
                /target:library
                /platform:x64
                /reference:${COPPERFIN_MANAGED_DECLARE_DEPENDENCY_PATH}
                /out:${COPPERFIN_MANAGED_DECLARE_FIXTURE_X64_PATH}
                managed_declare_fixture.cs
            COMMAND "${COPPERFIN_FRAMEWORK_CSC}"
                /nologo
                /target:library
                /platform:${COPPERFIN_MANAGED_DECLARE_FIXTURE_PLATFORM}
                /reference:${COPPERFIN_MANAGED_DECLARE_DEPENDENCY_PATH}
                /out:${COPPERFIN_MANAGED_DECLARE_MISSING_PATH}
                managed_declare_fixture.cs
            COMMAND "${COPPERFIN_FRAMEWORK_CSC}"
                /nologo
                /target:library
                /platform:${COPPERFIN_MANAGED_DECLARE_FIXTURE_PLATFORM}
                /reference:${COPPERFIN_MANAGED_DECLARE_DEPENDENCY_PATH}
                /out:${COPPERFIN_MANAGED_DECLARE_RELATIVE_PATH}
                managed_declare_fixture.cs
            COMMAND "${COPPERFIN_FRAMEWORK_CSC}"
                /nologo
                /target:library
                /platform:${COPPERFIN_MANAGED_DECLARE_FIXTURE_PLATFORM}
                /reference:${COPPERFIN_MANAGED_DECLARE_DEPENDENCY_PATH}
                /out:${COPPERFIN_MANAGED_DECLARE_PARENTLESS_PATH}
                managed_declare_fixture.cs
            DEPENDS
                "${CMAKE_CURRENT_SOURCE_DIR}/fixtures/managed_declare_dependency.cs"
                "${CMAKE_CURRENT_SOURCE_DIR}/fixtures/managed_declare_fixture.cs"
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/fixtures"
            VERBATIM
        )
        add_custom_target(copperfin_managed_declare_fixture
            DEPENDS
                "${COPPERFIN_MANAGED_DECLARE_FIXTURE_PATH}"
                "${COPPERFIN_MANAGED_DECLARE_FIXTURE_X86_PATH}"
                "${COPPERFIN_MANAGED_DECLARE_FIXTURE_X64_PATH}"
                "${COPPERFIN_MANAGED_DECLARE_DEPENDENCY_PATH}"
                "${COPPERFIN_MANAGED_DECLARE_MISSING_PATH}"
                "${COPPERFIN_MANAGED_DECLARE_RELATIVE_PATH}"
                "${COPPERFIN_MANAGED_DECLARE_PARENTLESS_PATH}"
        )
        copperfin_add_test(test_prg_engine_dotnet_dispatch cf_xbase_runtime
            test_prg_engine_dotnet_dispatch.cpp
            prg_engine_test_support.cpp
        )
        target_include_directories(test_prg_engine_dotnet_dispatch PRIVATE
            "${CMAKE_SOURCE_DIR}/src/runtime"
        )
        target_compile_definitions(test_prg_engine_dotnet_dispatch PRIVATE
            COPPERFIN_ENABLE_DISPATCH_TEST_HOOKS=1
            COPPERFIN_DECLARED_DLL_FIXTURE_PATH="$<TARGET_FILE:copperfin_declared_dll_fixture>"
            COPPERFIN_MANAGED_DECLARE_FIXTURE_PATH="${COPPERFIN_MANAGED_DECLARE_FIXTURE_PATH}"
            COPPERFIN_MANAGED_DECLARE_FIXTURE_X86_PATH="${COPPERFIN_MANAGED_DECLARE_FIXTURE_X86_PATH}"
            COPPERFIN_MANAGED_DECLARE_FIXTURE_X64_PATH="${COPPERFIN_MANAGED_DECLARE_FIXTURE_X64_PATH}"
            COPPERFIN_MANAGED_DECLARE_DEPENDENCY_PATH="${COPPERFIN_MANAGED_DECLARE_DEPENDENCY_PATH}"
            COPPERFIN_MANAGED_DECLARE_MISSING_PATH="${COPPERFIN_MANAGED_DECLARE_MISSING_PATH}"
            COPPERFIN_MANAGED_DECLARE_RELATIVE_PATH="${COPPERFIN_MANAGED_DECLARE_RELATIVE_PATH}"
            COPPERFIN_MANAGED_DECLARE_PARENTLESS_PATH="${COPPERFIN_MANAGED_DECLARE_PARENTLESS_PATH}"
        )
        add_dependencies(
            test_prg_engine_dotnet_dispatch
            copperfin_declared_dll_fixture
            copperfin_managed_declare_fixture
        )
        if(TARGET copperfin_mixed_mode_declared_dll_fixture)
            target_compile_definitions(test_prg_engine_dotnet_dispatch PRIVATE
                COPPERFIN_MIXED_MODE_DECLARED_DLL_FIXTURE_PATH="$<TARGET_FILE:copperfin_mixed_mode_declared_dll_fixture>"
            )
            add_dependencies(
                test_prg_engine_dotnet_dispatch
                copperfin_mixed_mode_declared_dll_fixture
            )
        endif()
    else()
        message(STATUS "Legacy .NET Framework csc.exe not found; managed DECLARE runtime test is unavailable")
    endif()
endif()

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

copperfin_add_test(test_prg_engine_parser_classes cf_xbase_runtime
    test_prg_engine_parser_classes.cpp
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
if(NOT WIN32)
    target_link_libraries(test_prg_engine_runtime_surface_functions PRIVATE Iconv::Iconv)
endif()
target_include_directories(test_prg_engine_runtime_surface_functions PRIVATE
    ${CMAKE_SOURCE_DIR}/src/runtime
)

copperfin_add_test(test_prg_engine_locale_code_page cf_xbase_runtime
    test_prg_engine_locale_code_page.cpp
    prg_engine_test_support.cpp
)
target_include_directories(test_prg_engine_locale_code_page PRIVATE
    ${CMAKE_SOURCE_DIR}/src/runtime
)

copperfin_add_test(test_prg_engine_runtime_surface_itemid_selectors cf_xbase_runtime
    test_prg_engine_runtime_surface_itemid_selectors.cpp
    prg_engine_test_support.cpp
)

copperfin_add_test(test_prg_engine_runtime_surface_requery cf_xbase_runtime
    test_prg_engine_runtime_surface_requery.cpp
    prg_engine_test_support.cpp
)

copperfin_add_test(test_prg_engine_runtime_surface_value cf_xbase_runtime
    test_prg_engine_runtime_surface_value.cpp
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

copperfin_add_test(test_prg_engine_string_minus cf_xbase_runtime
    test_prg_engine_string_minus.cpp
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
target_include_directories(test_prg_engine_table_structure PRIVATE
    ${CMAKE_SOURCE_DIR}/src/runtime
)

copperfin_add_test(test_xasset_methods cf_xbase_runtime
    test_xasset_methods.cpp
)

# Studio-host tests launch this executable out of process. A TARGET_FILE generator
# expression in add_test() does not make narrow test-target builds rebuild the host.
get_property(copperfin_test_directory_targets DIRECTORY PROPERTY BUILDSYSTEM_TARGETS)
set(copperfin_self_contained_studio_host_tests
    test_studio_host
    test_studio_host_shell_command
)
foreach(copperfin_test_target IN LISTS copperfin_test_directory_targets)
    if(copperfin_test_target MATCHES "^test_studio_host($|_)" AND
       NOT copperfin_test_target IN_LIST copperfin_self_contained_studio_host_tests)
        get_target_property(copperfin_test_target_type ${copperfin_test_target} TYPE)
        if(copperfin_test_target_type STREQUAL "EXECUTABLE")
            add_dependencies(${copperfin_test_target} copperfin_studio_host)
        endif()
    endif()
endforeach()
