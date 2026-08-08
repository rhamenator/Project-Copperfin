# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

function(copperfin_set_test_isolation TEST_NAME)
    set(options PARALLEL_SAFE)
    set(one_value_args
        FILESYSTEM ENVIRONMENT CHILD_PROCESSES NETWORK SAMPLES PLATFORM AUDIT RESOURCES LOCK)
    cmake_parse_arguments(CF_ISOLATION "${options}" "${one_value_args}" "" ${ARGN})

    if(NOT TEST ${TEST_NAME})
        message(FATAL_ERROR "Cannot classify unknown native test: ${TEST_NAME}")
    endif()

    if(NOT DEFINED CF_ISOLATION_RESOURCES OR "${CF_ISOLATION_RESOURCES}" STREQUAL "")
        if(CF_ISOLATION_AUDIT STREQUAL "complete")
            set(CF_ISOLATION_RESOURCES none)
        else()
            set(CF_ISOLATION_RESOURCES unverified)
        endif()
    endif()
    if(DEFINED CF_ISOLATION_LOCK AND NOT "${CF_ISOLATION_LOCK}" STREQUAL "")
        if(NOT CF_ISOLATION_RESOURCES STREQUAL "lock")
            message(FATAL_ERROR
                "Native test ${TEST_NAME} has RESOURCE_LOCK ${CF_ISOLATION_LOCK} without RESOURCES lock")
        endif()
    elseif(CF_ISOLATION_RESOURCES STREQUAL "lock")
        message(FATAL_ERROR "Native test ${TEST_NAME} declares RESOURCES lock without LOCK")
    endif()

    foreach(required_field IN ITEMS FILESYSTEM ENVIRONMENT CHILD_PROCESSES
            NETWORK SAMPLES PLATFORM AUDIT RESOURCES)
        if(NOT DEFINED CF_ISOLATION_${required_field} OR
           "${CF_ISOLATION_${required_field}}" STREQUAL "")
            message(FATAL_ERROR
                "Native test ${TEST_NAME} is missing isolation field ${required_field}")
        endif()
    endforeach()

    if(CF_ISOLATION_PARALLEL_SAFE)
        set(schedule parallel-safe)
        set(run_serial FALSE)
    else()
        set(schedule serial)
        set(run_serial TRUE)
    endif()

    set(labels
        "copperfin-isolation:filesystem=${CF_ISOLATION_FILESYSTEM}"
        "copperfin-isolation:environment=${CF_ISOLATION_ENVIRONMENT}"
        "copperfin-isolation:child-processes=${CF_ISOLATION_CHILD_PROCESSES}"
        "copperfin-isolation:network=${CF_ISOLATION_NETWORK}"
        "copperfin-isolation:samples=${CF_ISOLATION_SAMPLES}"
        "copperfin-isolation:platform=${CF_ISOLATION_PLATFORM}"
        "copperfin-isolation:resources=${CF_ISOLATION_RESOURCES}"
        "copperfin-isolation:audit=${CF_ISOLATION_AUDIT}"
        "copperfin-isolation:schedule=${schedule}"
    )
    get_property(existing_labels TEST ${TEST_NAME} PROPERTY LABELS)
    if(existing_labels)
        list(FILTER existing_labels EXCLUDE REGEX "^copperfin-isolation:")
        list(APPEND labels ${existing_labels})
    endif()
    list(REMOVE_DUPLICATES labels)
    set_tests_properties(${TEST_NAME} PROPERTIES
        LABELS "${labels}"
        RUN_SERIAL ${run_serial}
    )
    if(DEFINED CF_ISOLATION_LOCK AND NOT "${CF_ISOLATION_LOCK}" STREQUAL "")
        set_tests_properties(${TEST_NAME} PROPERTIES RESOURCE_LOCK "${CF_ISOLATION_LOCK}")
    else()
        set_tests_properties(${TEST_NAME} PROPERTIES RESOURCE_LOCK "")
    endif()
endfunction()

function(copperfin_configure_native_test_isolation)
    get_property(native_tests DIRECTORY PROPERTY TESTS)
    if(NOT native_tests)
        message(FATAL_ERROR "Native test isolation cannot be configured without registered tests")
    endif()

    # New and not-yet-audited tests are deliberately pessimistic. Promotion to
    # parallel-safe requires an explicit source audit of every risk axis below.
    foreach(test_name IN LISTS native_tests)
        copperfin_set_test_isolation(${test_name}
            FILESYSTEM unverified
            ENVIRONMENT unverified
            CHILD_PROCESSES unverified
            NETWORK unverified
            SAMPLES unverified
            PLATFORM configured
            AUDIT pending
            RESOURCES unverified
        )
    endforeach()

    foreach(test_name IN ITEMS
            test_report_layout
            test_report_layout_sort_settings
            test_report_layout_side_margin_settings
            test_project_workspace)
        copperfin_set_test_isolation(${test_name}
            PARALLEL_SAFE
            FILESYSTEM none
            ENVIRONMENT none
            CHILD_PROCESSES none
            NETWORK none
            SAMPLES none
            PLATFORM portable
            AUDIT complete
        )
    endforeach()

    foreach(test_name IN ITEMS
            test_polyglot_bridge_invocation
            test_polyglot_migration_telemetry
            test_polyglot_parity_comparator
            test_polyglot_route_registry)
        copperfin_set_test_isolation(${test_name}
            PARALLEL_SAFE
            FILESYSTEM none
            ENVIRONMENT none
            CHILD_PROCESSES none
            NETWORK none
            SAMPLES none
            PLATFORM portable
            AUDIT complete
        )
    endforeach()
    copperfin_set_test_isolation(test_bounded_process
        PARALLEL_SAFE
        FILESYSTEM process-owned
        ENVIRONMENT scoped-process
        CHILD_PROCESSES bounded
        NETWORK none
        SAMPLES none
        PLATFORM portable
        AUDIT complete
    )
    copperfin_set_test_isolation(test_polyglot_contract
        PARALLEL_SAFE
        FILESYSTEM read-only
        ENVIRONMENT none
        CHILD_PROCESSES none
        NETWORK none
        SAMPLES none
        PLATFORM portable
        AUDIT complete
    )
    copperfin_set_test_isolation(test_polyglot_route_contract
        PARALLEL_SAFE
        FILESYSTEM read-only
        ENVIRONMENT none
        CHILD_PROCESSES none
        NETWORK none
        SAMPLES none
        PLATFORM portable
        AUDIT complete
    )

    foreach(test_name IN ITEMS
            test_product_subsystems
            test_builder_registry
            test_toolbox_palette
            test_context_editor_actions
            test_designer_context)
        copperfin_set_test_isolation(${test_name}
            PARALLEL_SAFE
            FILESYSTEM read-only
            ENVIRONMENT none
            CHILD_PROCESSES none
            NETWORK none
            SAMPLES none
            PLATFORM portable
            AUDIT complete
        )
    endforeach()

    foreach(test_name IN ITEMS
            test_platform_models)
        copperfin_set_test_isolation(${test_name}
            PARALLEL_SAFE
            FILESYSTEM none
            ENVIRONMENT scoped-process
            CHILD_PROCESSES none
            NETWORK none
            SAMPLES none
            PLATFORM portable
            AUDIT complete
        )
    endforeach()

    foreach(test_name IN ITEMS
            test_query_translator
            test_federation_execution)
        copperfin_set_test_isolation(${test_name}
            PARALLEL_SAFE
            FILESYSTEM read-only
            ENVIRONMENT scoped-process
            CHILD_PROCESSES none
            NETWORK none
            SAMPLES none
            PLATFORM portable
            AUDIT complete
        )
    endforeach()

    copperfin_set_test_isolation(test_xasset_methods
        PARALLEL_SAFE
        FILESYSTEM read-only
        ENVIRONMENT scoped-process
        CHILD_PROCESSES none
        NETWORK none
        SAMPLES read-only
        PLATFORM portable
        AUDIT complete
    )

    if(TEST test_validate_posix_build_type)
        copperfin_set_test_isolation(test_validate_posix_build_type
            PARALLEL_SAFE
            FILESYSTEM fixed-build-tree
            ENVIRONMENT scoped-process
            CHILD_PROCESSES bounded
            NETWORK none
            SAMPLES none
            PLATFORM configured
            AUDIT complete
        )
    endif()

    foreach(test_name IN ITEMS
            test_vfp_sidecar_path
            test_toolbox_creation)
        copperfin_set_test_isolation(${test_name}
            PARALLEL_SAFE
            FILESYSTEM process-owned
            ENVIRONMENT scoped-process
            CHILD_PROCESSES none
            NETWORK none
            SAMPLES owned-copy
            PLATFORM portable
            AUDIT complete
        )
    endforeach()

    foreach(test_name IN ITEMS
            test_native_platform_workflow_contract
            test_windows_msvc_cache_workflow_contract
            test_native_test_isolation_contract
            test_launcher_trust_provisioning_contract
            test_package_launcher_inventory_trust
            test_security_supply_chain_workflow_contract
            test_focused_workflow_path_filters)
        copperfin_set_test_isolation(${test_name}
            PARALLEL_SAFE
            FILESYSTEM read-only
            ENVIRONMENT none
            CHILD_PROCESSES none
            NETWORK none
            SAMPLES none
            PLATFORM portable
            AUDIT complete
        )
    endforeach()

    copperfin_set_test_isolation(test_package_signer_contract
        PARALLEL_SAFE
        FILESYSTEM process-owned
        ENVIRONMENT none
        CHILD_PROCESSES bounded
        NETWORK none
        SAMPLES none
        PLATFORM configured
        AUDIT complete
    )

    if(TEST test_agent_issue_intake)
        copperfin_set_test_isolation(test_agent_issue_intake
            PARALLEL_SAFE
            FILESYSTEM read-only
            ENVIRONMENT none
            CHILD_PROCESSES none
            NETWORK none
            SAMPLES none
            PLATFORM portable
            AUDIT complete
        )
    endif()

    copperfin_set_test_isolation(test_product_licensing_policy_contract
        PARALLEL_SAFE
        FILESYSTEM read-only
        ENVIRONMENT none
        CHILD_PROCESSES none
        NETWORK none
        SAMPLES none
        PLATFORM portable
        AUDIT complete
    )

    copperfin_set_test_isolation(test_repository_community_contract
        PARALLEL_SAFE
        FILESYSTEM read-only
        ENVIRONMENT none
        CHILD_PROCESSES none
        NETWORK none
        SAMPLES none
        PLATFORM portable
        AUDIT complete
    )

    copperfin_set_test_isolation(test_release_licensing_contract
        PARALLEL_SAFE
        FILESYSTEM read-only
        ENVIRONMENT none
        CHILD_PROCESSES none
        NETWORK none
        SAMPLES none
        PLATFORM portable
        AUDIT complete
    )

    copperfin_set_test_isolation(test_rc_candidate_workflow_contract
        PARALLEL_SAFE
        FILESYSTEM read-only
        ENVIRONMENT none
        CHILD_PROCESSES none
        NETWORK none
        SAMPLES none
        PLATFORM portable
        AUDIT complete
    )

    if(TEST test_rc_candidate_assembly)
        copperfin_set_test_isolation(test_rc_candidate_assembly
            PARALLEL_SAFE
            FILESYSTEM process-owned
            ENVIRONMENT none
            CHILD_PROCESSES bounded
            NETWORK none
            SAMPLES none
            PLATFORM configured
            AUDIT complete
        )
    endif()

    if(TEST test_contributor_signoff_contract)
        copperfin_set_test_isolation(test_contributor_signoff_contract
            PARALLEL_SAFE
            FILESYSTEM process-owned
            ENVIRONMENT scoped-process
            CHILD_PROCESSES bounded
            NETWORK none
            SAMPLES none
            PLATFORM configured
            AUDIT complete
        )
    endif()

    copperfin_set_test_isolation(test_github_actions_contract
        FILESYSTEM read-only
        ENVIRONMENT none
        CHILD_PROCESSES none
        NETWORK none
        SAMPLES none
        PLATFORM portable
        AUDIT complete
    )

    foreach(test_name IN ITEMS
            test_prg_engine_dynamic_xasset_security
            test_prg_engine_report_security
            test_prg_engine_macro_expressions
            test_prg_engine_native_focus_move_events
            test_prg_engine_native_lifecycle_events
            test_prg_engine_ordered_iteration
            test_prg_engine_runtime_surface_functions_buffering
            test_prg_engine_scope_declarations
            test_prg_engine_transform_numeric_pictures
            test_prg_engine_verified_dbf_security)
        copperfin_set_test_isolation(${test_name}
            FILESYSTEM process-owned
            ENVIRONMENT none
            CHILD_PROCESSES none
            NETWORK none
            SAMPLES none
            PLATFORM portable
            AUDIT complete
        )
    endforeach()

    copperfin_set_test_isolation(test_package_document_install
        PARALLEL_SAFE
        FILESYSTEM process-owned
        ENVIRONMENT none
        CHILD_PROCESSES bounded
        NETWORK none
        SAMPLES none
        PLATFORM portable
        AUDIT complete
    )
    copperfin_set_test_isolation(test_locale_catalog_install_contract
        PARALLEL_SAFE
        FILESYSTEM test-owned-unique
        ENVIRONMENT none
        CHILD_PROCESSES bounded
        NETWORK none
        SAMPLES none
        PLATFORM portable
        AUDIT complete
    )
    copperfin_set_test_isolation(test_studio_install_contract
        PARALLEL_SAFE
        FILESYSTEM test-owned-unique
        ENVIRONMENT none
        CHILD_PROCESSES bounded
        NETWORK none
        SAMPLES none
        PLATFORM portable
        AUDIT complete
    )
    copperfin_set_test_isolation(test_cpack_artifact_contract
        PARALLEL_SAFE
        FILESYSTEM test-owned-unique
        ENVIRONMENT none
        CHILD_PROCESSES bounded
        NETWORK none
        SAMPLES none
        PLATFORM portable
        AUDIT complete
    )
    copperfin_set_test_isolation(test_safety_traceability_workflow_contract
        PARALLEL_SAFE
        FILESYSTEM read-only
        ENVIRONMENT child-scoped
        CHILD_PROCESSES bounded
        NETWORK disabled-probes
        SAMPLES none
        PLATFORM powershell-conditional
        AUDIT complete
    )

    copperfin_set_test_isolation(test_managed_compile
        FILESYSTEM shared-build-tree
        ENVIRONMENT none
        CHILD_PROCESSES bounded
        NETWORK possible-package-restore
        SAMPLES none
        PLATFORM dotnet-conditional
        AUDIT complete
    )
    copperfin_set_test_isolation(test_build_parallelism_contract
        FILESYSTEM fixed-build-tree
        ENVIRONMENT child-scoped
        CHILD_PROCESSES bounded
        NETWORK none
        SAMPLES none
        PLATFORM powershell-conditional
        AUDIT complete
    )

    foreach(test_name IN ITEMS
            test_studio_host_bottom_margin_record_settings
            test_studio_host_bottom_margin_stable_settings
            test_studio_host_code_page
            test_studio_host_column_count_record_settings
            test_studio_host_column_setup_record_settings
            test_studio_host_column_setup_stable_settings
            test_studio_host_column_spacing_record_settings
            test_studio_host_column_spacing_stable_settings
            test_studio_host_column_width_record_settings
            test_studio_host_deleted_band_object_sections
            test_studio_host_deleted_object_duplicate_batch
            test_studio_host_deleted_object_duplicate_batch_errors
            test_studio_host_deleted_object_duplicate_batch_sections
            test_studio_host_deleted_object_height_top_geometry
            test_studio_host_deleted_object_rename_batch
            test_studio_host_deleted_object_rename_batch_errors
            test_studio_host_deleted_object_rename_batch_sections
            test_studio_host_deleted_object_reorder_batch
            test_studio_host_deleted_object_reorder_batch_errors
            test_studio_host_deleted_object_reorder_batch_sections
            test_studio_host_deleted_object_restore_geometry
            test_studio_host_deleted_object_width_left_geometry
            test_studio_host_deleted_property_batch_rejection_sections
            test_studio_host_deleted_property_clear_batch_sections
            test_studio_host_deleted_property_clear_sections
            test_studio_host_deleted_property_copy_batch_sections
            test_studio_host_deleted_property_copy_sections
            test_studio_host_deleted_property_move_batch_sections
            test_studio_host_deleted_property_move_sections
            test_studio_host_deleted_property_rejection_sections
            test_studio_host_deleted_report_visual_property_clears
            test_studio_host_deleted_report_visual_property_copies
            test_studio_host_deleted_report_visual_property_moves
            test_studio_host_deleted_report_visual_property_rejections
            test_studio_host_deleted_section_geometry
            test_studio_host_deleted_subtree_duplicate
            test_studio_host_deleted_subtree_duplicate_collision_errors
            test_studio_host_deleted_subtree_duplicate_errors
            test_studio_host_deleted_update_batch
            test_studio_host_deleted_update_batch_errors
            test_studio_host_deleted_update_batch_sections
            test_studio_host_detail_header_footer_object_expressions
            test_studio_host_detail_header_footer_object_geometry
            test_studio_host_detail_header_footer_object_layout_actions
            test_studio_host_detail_header_footer_object_lifecycle
            test_studio_host_detail_header_footer_section_expressions
            test_studio_host_detail_header_footer_section_geometry
            test_studio_host_detail_header_footer_section_lifecycle
            test_studio_host_detail_header_footer_section_preview_bounds
            test_studio_host_edited_object_delete_geometry
            test_studio_host_geometry_live_edit_actions
            test_studio_host_horizontal_grid_record_settings
            test_studio_host_horizontal_grid_stable_settings
            test_studio_host_label_layout_actions
            test_studio_host_layout_actions
            test_studio_host_mixed_deleted_states
            test_studio_host_mixed_deleted_states_errors
            test_studio_host_object_action_lifecycle
            test_studio_host_object_deleted_states
            test_studio_host_object_deleted_states_errors
            test_studio_host_object_duplicate_batch
            test_studio_host_object_duplicate_batch_errors
            test_studio_host_object_rename_batch
            test_studio_host_object_rename_batch_errors
            test_studio_host_object_reorder_batch
            test_studio_host_object_reorder_batch_errors
            test_studio_host_orientation_record_settings
            test_studio_host_orientation_stable_settings
            test_studio_host_page_margin_record_settings
            test_studio_host_page_margin_stable_settings
            test_studio_host_paper_size_record_settings
            test_studio_host_paper_size_stable_settings
            test_studio_host_real_sample_ascii_round_trip
            test_studio_host_real_sample_collate_round_trip
            test_studio_host_real_sample_column_setup_round_trip
            test_studio_host_real_sample_copies_round_trip
            test_studio_host_real_sample_defaultsource_round_trip
            test_studio_host_real_sample_device_round_trip
            test_studio_host_real_sample_direct_margins_round_trip
            test_studio_host_real_sample_driver_round_trip
            test_studio_host_real_sample_discovery
            test_studio_host_real_sample_grouping_deleted_preview_round_trip
            test_studio_host_real_sample_grouping_round_trip
            test_studio_host_real_sample_left_margin_round_trip
            test_studio_host_real_sample_object_align_round_trip
            test_studio_host_real_sample_object_delete_restore_round_trip
            test_studio_host_real_sample_object_deleted_preview_round_trip
            test_studio_host_real_sample_object_distribute_round_trip
            test_studio_host_real_sample_object_duplicate_round_trip
            test_studio_host_real_sample_object_font_clear_round_trip
            test_studio_host_real_sample_object_font_round_trip
            test_studio_host_real_sample_object_memo_round_trip
            test_studio_host_real_sample_object_nudge_round_trip
            test_studio_host_real_sample_object_preview_bounds_round_trip
            test_studio_host_real_sample_object_rename_round_trip
            test_studio_host_real_sample_object_reorder_round_trip
            test_studio_host_real_sample_object_resize_round_trip
            test_studio_host_real_sample_object_round_trip
            test_studio_host_real_sample_object_snap_round_trip
            test_studio_host_real_sample_output_round_trip
            test_studio_host_real_sample_page_setup_settings_round_trip
            test_studio_host_real_sample_paper_dimensions_round_trip
            test_studio_host_real_sample_printquality_round_trip
            test_studio_host_real_sample_right_margin_round_trip
            test_studio_host_real_sample_round_trip
            test_studio_host_real_sample_section_delete_restore_round_trip
            test_studio_host_real_sample_section_deleted_preview_round_trip
            test_studio_host_real_sample_section_round_trip
            test_studio_host_real_sample_settings_delete_restore_round_trip
            test_studio_host_real_sample_sort_settings_round_trip
            test_studio_host_real_sample_ttoption_round_trip
            test_studio_host_real_sample_yresolution_round_trip
            test_studio_host_remaining_object_lifecycle
            test_studio_host_report_column_setup
            test_studio_host_report_column_width_fields
            test_studio_host_report_deleted_states
            test_studio_host_report_direct_setting_fields
            test_studio_host_report_font_metadata
            test_studio_host_report_geometry_defaults
            test_studio_host_report_grouping_deleted_exposure
            test_studio_host_report_grouping_deleted_mutation
            test_studio_host_report_grouping_exposure
            test_studio_host_report_grouping_nested_mutation
            test_studio_host_report_grouping_record_mutation
            test_studio_host_report_grouping_remaining_exposure
            test_studio_host_report_grouping_stable_mutation
            test_studio_host_report_height_top_preview_bounds
            test_studio_host_report_layout_actions
            test_studio_host_report_layout_classifications
            test_studio_host_report_layout_diagnostics
            test_studio_host_report_layout_placement
            test_studio_host_report_live_section_geometry
            test_studio_host_report_page_setup_fields
            test_studio_host_report_schema_fallbacks
            test_studio_host_report_section_ids
            test_studio_host_report_section_selection_diagnostics
            test_studio_host_report_selection_deep
            test_studio_host_report_selection_deep_section_settings
            test_studio_host_report_selection_none
            test_studio_host_report_selection_padded
            test_studio_host_report_settings_diagnostics
            test_studio_host_report_settings_memo_parsing
            test_studio_host_report_sort_settings_record
            test_studio_host_report_sort_settings_stable
            test_studio_host_report_unresolved_memo_placeholders
            test_studio_host_report_width_left_preview_bounds
            test_studio_host_section_delete_record_selection
            test_studio_host_section_delete_stable_selection
            test_studio_host_section_deleted_object_counts
            test_studio_host_section_restore_record_selection
            test_studio_host_section_restore_stable_selection
            test_studio_host_selected_column_footer_objects_stable
            test_studio_host_selected_column_footer_sections_record
            test_studio_host_selected_column_footer_sections_stable
            test_studio_host_selected_column_header_objects_stable
            test_studio_host_selected_column_header_sections_record
            test_studio_host_selected_column_header_sections_stable
            test_studio_host_selected_detail_objects_orphaned
            test_studio_host_selected_group_footer_objects_stable
            test_studio_host_selected_group_footer_sections_record
            test_studio_host_selected_group_footer_sections_stable
            test_studio_host_selected_group_header_objects_stable
            test_studio_host_selected_group_header_sections_record
            test_studio_host_selected_group_header_sections_stable
            test_studio_host_selected_label_objects
            test_studio_host_selected_label_sections
            test_studio_host_selected_label_settings
            test_studio_host_selected_nested_group_sections_record
            test_studio_host_selected_nested_group_sections_stable
            test_studio_host_selected_objects_stable
            test_studio_host_selected_page_footer_objects_stable
            test_studio_host_selected_page_footer_sections_record
            test_studio_host_selected_page_footer_sections_stable
            test_studio_host_selected_page_header_objects_orphaned
            test_studio_host_selected_page_header_objects_stable
            test_studio_host_selected_page_header_sections_record
            test_studio_host_selected_page_header_sections_stable
            test_studio_host_selected_report_objects
            test_studio_host_selected_report_sections
            test_studio_host_selected_report_settings
            test_studio_host_selected_sections_stable
            test_studio_host_selected_settings_stable
            test_studio_host_selected_summary_objects_record
            test_studio_host_selected_summary_objects_stable
            test_studio_host_selected_summary_sections_record
            test_studio_host_selected_summary_sections_stable
            test_studio_host_selected_title_objects_stable
            test_studio_host_selected_title_sections_record
            test_studio_host_selected_title_sections_stable
            test_studio_host_selected_unplaced_objects_stable
            test_studio_host_settings_delete_record
            test_studio_host_settings_delete_stable
            test_studio_host_settings_memo_record
            test_studio_host_settings_memo_stable
            test_studio_host_settings_restore_record
            test_studio_host_settings_restore_stable
            test_studio_host_settings_section_deleted_states
            test_studio_host_settings_section_deleted_states_errors
            test_studio_host_side_margin_record_settings
            test_studio_host_stable_selector_mutation
            test_studio_host_subtree_deleted_state
            test_studio_host_subtree_deleted_state_errors
            test_studio_host_subtree_duplicate
            test_studio_host_subtree_duplicate_collision_errors
            test_studio_host_subtree_duplicate_errors
            test_studio_host_toolbox_create_lifecycle
            test_studio_host_update_batch
            test_studio_host_update_batch_errors
            test_studio_host_vertical_grid_record_settings
            test_studio_host_vertical_grid_stable_settings)
        copperfin_set_test_isolation(${test_name}
            PARALLEL_SAFE
            FILESYSTEM test-owned-unique
            ENVIRONMENT scoped-process
            CHILD_PROCESSES bounded
            NETWORK none
            SAMPLES owned-copy
            PLATFORM portable
            AUDIT complete
        )
    endforeach()
    copperfin_set_test_isolation(test_studio_host_json
        FILESYSTEM fixed-shared-family
        ENVIRONMENT scoped-process
        CHILD_PROCESSES bounded
        NETWORK none
        SAMPLES owned-copy
        PLATFORM portable
        AUDIT complete
    )

    foreach(test_name IN ITEMS
            test_studio_host_deleted_object_height_top_geometry
            test_studio_host_deleted_object_restore_geometry
            test_studio_host_deleted_object_width_left_geometry
            test_studio_host_deleted_report_visual_property_clears
            test_studio_host_deleted_report_visual_property_copies
            test_studio_host_deleted_report_visual_property_moves
            test_studio_host_deleted_report_visual_property_rejections
            test_studio_host_deleted_section_geometry
            test_studio_host_detail_header_footer_object_expressions
            test_studio_host_detail_header_footer_object_geometry
            test_studio_host_detail_header_footer_object_layout_actions
            test_studio_host_detail_header_footer_object_lifecycle
            test_studio_host_detail_header_footer_section_expressions
            test_studio_host_detail_header_footer_section_geometry
            test_studio_host_detail_header_footer_section_lifecycle
            test_studio_host_detail_header_footer_section_preview_bounds
            test_studio_host_edited_object_delete_geometry
            test_studio_host_geometry_live_edit_actions
            test_studio_host_label_layout_actions
            test_studio_host_object_action_lifecycle
            test_studio_host_remaining_object_lifecycle
            test_studio_host_report_column_setup
            test_studio_host_report_column_width_fields
            test_studio_host_report_deleted_states
            test_studio_host_report_direct_setting_fields
            test_studio_host_report_font_metadata
            test_studio_host_report_geometry_defaults
            test_studio_host_report_height_top_preview_bounds
            test_studio_host_report_layout_actions
            test_studio_host_report_layout_classifications
            test_studio_host_report_layout_diagnostics
            test_studio_host_report_layout_placement
            test_studio_host_report_live_section_geometry
            test_studio_host_report_page_setup_fields
            test_studio_host_report_schema_fallbacks
            test_studio_host_report_section_selection_diagnostics
            test_studio_host_report_settings_diagnostics
            test_studio_host_report_settings_memo_parsing
            test_studio_host_report_unresolved_memo_placeholders
            test_studio_host_report_width_left_preview_bounds
            test_studio_host_stable_selector_mutation
            test_studio_host_toolbox_create_lifecycle)
        copperfin_set_test_isolation(${test_name}
            PARALLEL_SAFE
            FILESYSTEM fixed-resource-locked
            ENVIRONMENT scoped-process
            CHILD_PROCESSES bounded
            NETWORK none
            SAMPLES owned-copy
            PLATFORM portable
            AUDIT complete
            RESOURCES lock
            LOCK copperfin-studio-host-shared-fixtures
        )
    endforeach()

    foreach(test_name IN ITEMS
            test_visual_asset_editor_code_page
            test_studio_host_code_page)
        copperfin_set_test_isolation(${test_name}
            PARALLEL_SAFE
            FILESYSTEM fixed-resource-locked
            ENVIRONMENT none
            CHILD_PROCESSES bounded-optional
            NETWORK none
            SAMPLES owned-copy
            PLATFORM portable
            AUDIT complete
            RESOURCES lock
            LOCK copperfin-visual-asset-code-page-root
        )
    endforeach()

    foreach(test_name IN ITEMS
            test_build_host_output
            test_localization
            test_studio_host_deleted_record_localization
            test_studio_host_real_sample_discovery
            test_runtime_host_debug_output_formatting
            test_runtime_host_audit_stream
            test_runtime_host_audit_containment)
        copperfin_set_test_isolation(${test_name}
            PARALLEL_SAFE
            FILESYSTEM test-owned-unique
            ENVIRONMENT scoped-process
            CHILD_PROCESSES bounded
            NETWORK none
            SAMPLES none
            PLATFORM portable
            AUDIT complete
        )
    endforeach()

    foreach(test_name IN ITEMS
            test_runtime_host_binding
            test_runtime_host_implicit_path_launch
            test_tool_license_path_launch
            test_platform_environment)
        copperfin_set_test_isolation(${test_name}
            PARALLEL_SAFE
            FILESYSTEM process-owned
            ENVIRONMENT scoped-process
            CHILD_PROCESSES bounded
            NETWORK none
            SAMPLES none
            PLATFORM portable
            AUDIT complete
        )
    endforeach()

    foreach(test_name IN ITEMS
            test_dbf_table
            test_dbf_table_active_locale)
        copperfin_set_test_isolation(${test_name}
            PARALLEL_SAFE
            FILESYSTEM process-owned
            ENVIRONMENT scoped-process
            CHILD_PROCESSES none
            NETWORK none
            SAMPLES none
            PLATFORM portable
            AUDIT complete
        )
    endforeach()

    copperfin_set_test_isolation(test_dbf_text_encoding
        PARALLEL_SAFE
        FILESYSTEM none
        ENVIRONMENT none
        CHILD_PROCESSES none
        NETWORK none
        SAMPLES none
        PLATFORM portable
        AUDIT complete
    )
    foreach(copperfin_dbf_header_robustness_target IN ITEMS
            test_dbf_header_robustness
            test_dbf_header_robustness_sanitized)
        if(TARGET ${copperfin_dbf_header_robustness_target})
            copperfin_set_test_isolation(${copperfin_dbf_header_robustness_target}
                PARALLEL_SAFE
                FILESYSTEM test-owned-unique
                ENVIRONMENT none
                CHILD_PROCESSES none
                NETWORK none
                SAMPLES none
                PLATFORM portable
                AUDIT complete
            )
        endif()
    endforeach()
    copperfin_set_test_isolation(test_visual_asset_editor
        PARALLEL_SAFE
        FILESYSTEM process-owned
        ENVIRONMENT none
        CHILD_PROCESSES none
        NETWORK none
        SAMPLES none
        PLATFORM portable
        AUDIT complete
    )

    copperfin_set_test_isolation(test_licensing_status
        PARALLEL_SAFE
        FILESYSTEM test-owned-unique
        ENVIRONMENT scoped-process
        CHILD_PROCESSES none
        NETWORK none
        SAMPLES none
        PLATFORM portable
        AUDIT complete
    )
    foreach(test_name IN ITEMS
            test_process_capture
            test_studio_host_shell_command)
        copperfin_set_test_isolation(${test_name}
            PARALLEL_SAFE
            FILESYSTEM test-owned-unique
            ENVIRONMENT none
            CHILD_PROCESSES bounded
            NETWORK none
            SAMPLES none
            PLATFORM portable
            AUDIT complete
        )
    endforeach()
    copperfin_set_test_isolation(test_studio_host_utf8_arguments
        FILESYSTEM process-owned
        ENVIRONMENT scoped-process
        CHILD_PROCESSES bounded
        NETWORK none
        SAMPLES none
        PLATFORM portable
        AUDIT complete
    )
    copperfin_set_test_isolation(test_studio_host
        PARALLEL_SAFE
        FILESYSTEM test-owned-unique
        ENVIRONMENT none
        CHILD_PROCESSES none
        NETWORK none
        SAMPLES none
        PLATFORM portable
        AUDIT complete
    )

    copperfin_set_test_isolation(test_generated_launcher_process
        FILESYSTEM process-owned
        ENVIRONMENT scoped-process
        CHILD_PROCESSES bounded
        NETWORK possible-package-restore
        SAMPLES none
        PLATFORM dotnet-conditional
        AUDIT complete
    )
    if(TEST test_generated_launcher_posix_process)
        copperfin_set_test_isolation(test_generated_launcher_posix_process
            FILESYSTEM process-owned
            ENVIRONMENT scoped-process
            CHILD_PROCESSES bounded
            NETWORK possible-package-restore
            SAMPLES none
            PLATFORM dotnet-conditional
            AUDIT complete
        )
    endif()
    copperfin_set_test_isolation(test_build_host_utf8_launcher_paths
        FILESYSTEM process-owned
        ENVIRONMENT scoped-process
        CHILD_PROCESSES bounded
        NETWORK possible-package-restore
        SAMPLES none
        PLATFORM dotnet-conditional
        AUDIT complete
    )
    copperfin_set_test_isolation(test_runtime_pipeline
        FILESYSTEM process-owned
        ENVIRONMENT scoped-process
        CHILD_PROCESSES bounded
        NETWORK none
        SAMPLES owned-copy
        PLATFORM toolchain-conditional
        AUDIT complete
    )
    copperfin_set_test_isolation(test_security_controls
        FILESYSTEM test-owned-unique
        ENVIRONMENT scoped-process
        CHILD_PROCESSES bounded
        NETWORK none
        SAMPLES none
        PLATFORM portable
        AUDIT complete
    )
    copperfin_set_test_isolation(test_security_audit_concurrency
        PARALLEL_SAFE
        FILESYSTEM test-owned-unique
        ENVIRONMENT scoped-process
        CHILD_PROCESSES bounded
        NETWORK none
        SAMPLES none
        PLATFORM portable
        AUDIT complete
    )
    copperfin_set_test_isolation(test_vfp_assets
        FILESYSTEM test-owned-unique
        ENVIRONMENT none
        CHILD_PROCESSES none
        NETWORK none
        SAMPLES read-only
        PLATFORM portable
        AUDIT complete
    )

    foreach(test_name IN ITEMS
            test_prg_engine_aggregate_array_functions
            test_prg_engine_arrays
            test_prg_engine_control_flow
            test_prg_engine_data_io
            test_prg_engine_database_lifecycle
            test_prg_engine_date_time_functions
            test_prg_engine_debugger
            test_prg_engine_file_io_functions
            test_prg_engine_functions
            test_prg_engine_index_seek_optimization
            test_prg_engine_locale_code_page
            test_prg_engine_parser_classes
            test_prg_engine_path_functions
            test_prg_engine_runtime_surface_functions
            test_prg_engine_runtime_surface_itemid_selectors
            test_prg_engine_runtime_surface_procedure_classes
            test_prg_engine_runtime_surface_requery
            test_prg_engine_runtime_surface_value
            test_prg_engine_rushmore_optimization
            test_prg_engine_relations
            test_prg_engine_seek_index
            test_prg_engine_sql_cursors_metadata
            test_prg_engine_sql_cursors_mutation
            test_prg_engine_sql_cursors_seek_and_order
            test_prg_engine_string_math_functions
            test_prg_engine_string_minus
            test_prg_engine_table_mutation
            test_prg_engine_table_structure
            test_prg_engine_work_areas)
        copperfin_set_test_isolation(${test_name}
            PARALLEL_SAFE
            FILESYSTEM test-owned-unique
            ENVIRONMENT none
            CHILD_PROCESSES none
            NETWORK none
            SAMPLES none
            PLATFORM portable
            AUDIT complete
        )
    endforeach()

    foreach(test_name IN ITEMS
            test_prg_engine_arrays
            test_prg_engine_control_flow
            test_prg_engine_data_io
            test_prg_engine_database_lifecycle
            test_prg_engine_index_seek_optimization
            test_prg_engine_seek_index
            test_prg_engine_sql_cursors_metadata
            test_prg_engine_sql_cursors_mutation
            test_prg_engine_sql_cursors_seek_and_order
            test_prg_engine_string_minus
            test_prg_engine_table_structure)
        copperfin_set_test_isolation(${test_name}
            PARALLEL_SAFE
            FILESYSTEM test-owned-unique
            ENVIRONMENT scoped-process
            CHILD_PROCESSES none
            NETWORK none
            SAMPLES none
            PLATFORM portable
            AUDIT complete
        )
    endforeach()

    copperfin_set_test_isolation(test_prg_engine_locale_code_page
        PARALLEL_SAFE
        FILESYSTEM none
        ENVIRONMENT none
        CHILD_PROCESSES none
        NETWORK none
        SAMPLES none
        PLATFORM configured
        AUDIT complete
    )
    copperfin_set_test_isolation(test_prg_engine_runtime_surface_functions
        PARALLEL_SAFE
        FILESYSTEM test-owned-unique
        ENVIRONMENT none
        CHILD_PROCESSES none
        NETWORK none
        SAMPLES none
        PLATFORM configured
        AUDIT complete
    )

    copperfin_set_test_isolation(test_prg_engine
        FILESYSTEM fixed-build-tree
        ENVIRONMENT none
        CHILD_PROCESSES bounded
        NETWORK none
        SAMPLES owned-copy
        PLATFORM powershell-conditional
        AUDIT complete
    )
    if(TEST test_prg_engine_dotnet_dispatch)
        copperfin_set_test_isolation(test_prg_engine_dotnet_dispatch
            FILESYSTEM fixed-build-tree
            ENVIRONMENT scoped-process
            CHILD_PROCESSES bounded
            NETWORK none
            SAMPLES none
            PLATFORM dotnet-conditional
            AUDIT complete
        )
    endif()

    set(required_label_prefixes
        "copperfin-isolation:filesystem="
        "copperfin-isolation:environment="
        "copperfin-isolation:child-processes="
        "copperfin-isolation:network="
        "copperfin-isolation:samples="
        "copperfin-isolation:platform="
        "copperfin-isolation:resources="
        "copperfin-isolation:audit="
        "copperfin-isolation:schedule="
    )

    list(SORT native_tests)
    set(inventory_path "${CMAKE_BINARY_DIR}/native-test-isolation.tsv")
    file(SHA256 "${CMAKE_CURRENT_FUNCTION_LIST_FILE}" isolation_source_sha256)
    file(SHA256 "${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt"
        test_registration_source_sha256)
    file(WRITE "${inventory_path}"
        "isolation_source_sha256\t${isolation_source_sha256}\n"
        "test_registration_source_sha256\t${test_registration_source_sha256}\n"
        "schema_version\ttest\trun_serial\tresource_lock\tlabels\n")
    foreach(test_name IN LISTS native_tests)
        get_property(test_labels TEST ${test_name} PROPERTY LABELS)
        foreach(required_prefix IN LISTS required_label_prefixes)
            set(matching_labels)
            foreach(test_label IN LISTS test_labels)
                string(FIND "${test_label}" "${required_prefix}" prefix_index)
                if(prefix_index EQUAL 0)
                    list(APPEND matching_labels "${test_label}")
                endif()
            endforeach()
            list(LENGTH matching_labels matching_label_count)
            if(NOT matching_label_count EQUAL 1)
                message(FATAL_ERROR
                    "Native test ${test_name} must have exactly one ${required_prefix} label")
            endif()
        endforeach()

        get_property(test_run_serial TEST ${test_name} PROPERTY RUN_SERIAL)
        get_property(test_resource_lock TEST ${test_name} PROPERTY RESOURCE_LOCK)
        if(NOT test_resource_lock)
            set(test_resource_lock none)
        endif()
        foreach(test_label IN LISTS test_labels)
            string(FIND "${test_label}" "," comma_index)
            if(NOT comma_index EQUAL -1)
                message(FATAL_ERROR
                    "Native test ${test_name} label cannot be represented in the isolation inventory: ${test_label}")
            endif()
        endforeach()
        string(REPLACE ";" "," test_labels_csv "${test_labels}")
        file(APPEND "${inventory_path}"
            "1\t${test_name}\t${test_run_serial}\t${test_resource_lock}\t${test_labels_csv}\n")
    endforeach()

    set(COPPERFIN_NATIVE_TEST_ISOLATION_INVENTORY "${inventory_path}"
        CACHE INTERNAL "Generated native CTest isolation inventory")
endfunction()
