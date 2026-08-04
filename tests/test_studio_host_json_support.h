// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#ifndef COPPERFIN_TEST_STUDIO_HOST_JSON_SUPPORT_H
#define COPPERFIN_TEST_STUDIO_HOST_JSON_SUPPORT_H

#include "test_environment_support.h"
#include "test_locale_catalog_environment_support.h"
#include "copperfin/localization/localization.h"
#include "copperfin/vfp/dbf_table.h"
#include "copperfin/vfp/visual_asset_editor.h"

#include <cstdlib>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

namespace cf_test_studio_host_json {

#if !defined(COPPERFIN_TEST_SUCCESS_COMMAND)
#if defined(__APPLE__)
#define COPPERFIN_TEST_SUCCESS_COMMAND "/usr/bin/true"
#else
#define COPPERFIN_TEST_SUCCESS_COMMAND "/bin/true"
#endif
#endif

#if !defined(COPPERFIN_TEST_FAILURE_COMMAND)
#if defined(__APPLE__)
#define COPPERFIN_TEST_FAILURE_COMMAND "/usr/bin/false"
#else
#define COPPERFIN_TEST_FAILURE_COMMAND "/bin/false"
#endif
#endif

struct ProcessResult;

// ==== Shared test helpers (process invocation, locale env, assertion helpers, generic fixtures) ====

extern int failures;
using copperfin::test_support::getenv_value;
using copperfin::test_support::set_env_value;
void expect(bool condition, const std::string& message);
void expect_contains(const std::string& text, const std::string& needle, const std::string& message);
void expect_not_contains(const std::string& text, const std::string& needle, const std::string& message);
std::size_t count_missing_locale_keys(
    const copperfin::localization::LocalizedCatalog& catalog,
    std::string_view locale,
    const std::vector<std::string_view>& keys);
void expect_contains_in_order(
    const std::string& text,
    const std::vector<std::string>& needles,
    const std::string& message);
std::string expected_json_shell_quote(const std::string& value);
std::string expected_json_shell_command(
    const std::string& launch_command,
    std::initializer_list<std::string> arguments);
std::string read_text(const std::filesystem::path& path);
std::string normalize_captured_line_endings(std::string_view text);
ProcessResult run_process_capture(
    const std::string& executable_path,
    const std::vector<std::string>& arguments,
    const std::filesystem::path& working_directory);
void test_captured_process_output_line_endings_normalize(const std::string& studio_host_path);
void test_studio_host_json_exposes_rushmore_explain_plan(const std::string& studio_host_path);
std::vector<std::uint8_t> make_vfp_header();
void write_synthetic_form_asset(const std::filesystem::path& form_path);
void write_synthetic_form_table_with_objects(const std::filesystem::path& form_path);
void write_synthetic_form_table_with_invalid_raw_codes(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_toolbox_creation(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_visual_object_list(const std::filesystem::path& form_path);
std::size_t visual_object_count(const std::filesystem::path& form_path);
bool visual_object_deleted(const std::filesystem::path& form_path, const std::string& unique_id);
bool visual_object_exists(const std::filesystem::path& form_path, const std::string& unique_id);
bool dbf_record_deleted(const std::filesystem::path& table_path, std::size_t record_index);
bool visual_object_is_deleted(const std::filesystem::path& form_path, const std::string& unique_id);
std::string visual_object_parent(const std::filesystem::path& form_path, const std::string& unique_id);
std::string visual_object_property(
    const std::filesystem::path& form_path,
    const std::string& unique_id,
    const std::string& property_name);
std::string visual_object_property_order(
    const std::filesystem::path& form_path,
    const std::string& unique_id);
std::string visual_object_order(const std::filesystem::path& form_path);
void delete_existing_textbox(const std::filesystem::path& form_path, const std::string& evidence);
void write_synthetic_form_table_with_container_object(const std::filesystem::path& form_path);
void write_synthetic_table_with_data_environment(const std::filesystem::path& asset_path);
using copperfin::test_support::ScopedDefaultLocaleCatalogEnvironment;
using copperfin::test_support::ScopedEnvironmentValue;
struct ProcessResult {
    int exit_code = -1;
    std::string stdout_text;
    std::string stderr_text;
};

// ==== CLI launch-flag diagnostics localization tests: appearance/drawing values ====
void test_studio_host_launch_ole_icon_diagnostics_localize(const std::string& studio_host_path);
void test_studio_host_launch_drawing_data_diagnostics_localize(const std::string& studio_host_path);
void test_studio_host_launch_grid_layout_diagnostics_localize(const std::string& studio_host_path);
void test_studio_host_launch_list_item_color_diagnostics_localize(const std::string& studio_host_path);
void test_studio_host_launch_general_color_diagnostics_localize(const std::string& studio_host_path);
void test_studio_host_launch_form_appearance_diagnostics_localize(const std::string& studio_host_path);
void test_studio_host_launch_font_diagnostics_localize(const std::string& studio_host_path);
void test_studio_host_launch_drag_ole_value_diagnostics_localize(const std::string& studio_host_path);
void test_studio_host_launch_drawing_buffer_value_diagnostics_localize(const std::string& studio_host_path);
void test_studio_host_launch_display_dynamic_color_value_diagnostics_localize(
    const std::string& studio_host_path);
void test_studio_host_launch_form_appearance_value_diagnostics_localize(const std::string& studio_host_path);
void test_studio_host_launch_font_value_diagnostics_localize(const std::string& studio_host_path);
void test_studio_host_launch_marker_sizing_zorder_value_diagnostics_localize(
    const std::string& studio_host_path);
void test_studio_host_launch_color_value_diagnostics_localize(const std::string& studio_host_path);
void test_studio_host_launch_ole_drop_drawing_target_selector_value_diagnostics_localize(
    const std::string& studio_host_path);
void test_studio_host_launch_drawing_buffer_target_selector_value_diagnostics_localize(
    const std::string& studio_host_path);
void test_studio_host_launch_color_target_selector_value_diagnostics_localize(
    const std::string& studio_host_path);
void test_studio_host_launch_window_picture_target_selector_value_diagnostics_localize(
    const std::string& studio_host_path);
void test_studio_host_launch_font_target_selector_value_diagnostics_localize(
    const std::string& studio_host_path);
void test_studio_host_launch_marker_sizing_zorder_target_selector_value_diagnostics_localize(
    const std::string& studio_host_path);

// ==== CLI launch-flag diagnostics localization tests: target-selector values ====
void test_studio_host_launch_layout_target_selector_value_diagnostics_localize(
    const std::string& studio_host_path);
void test_studio_host_launch_state_target_selector_value_diagnostics_localize(
    const std::string& studio_host_path);
void test_studio_host_launch_media_target_selector_value_diagnostics_localize(
    const std::string& studio_host_path);
void test_studio_host_launch_mouse_drag_target_selector_value_diagnostics_localize(
    const std::string& studio_host_path);
void test_studio_host_launch_grid_target_selector_value_diagnostics_localize(
    const std::string& studio_host_path);
void test_studio_host_launch_partition_list_target_selector_value_diagnostics_localize(
    const std::string& studio_host_path);
void test_studio_host_launch_record_source_text_target_selector_value_diagnostics_localize(
    const std::string& studio_host_path);
void test_studio_host_launch_row_source_list_target_selector_value_diagnostics_localize(
    const std::string& studio_host_path);
void test_studio_host_launch_row_source_type_list_index_target_selector_value_diagnostics_localize(
    const std::string& studio_host_path);
void test_studio_host_launch_form_boolean_target_selector_value_diagnostics_localize(
    const std::string& studio_host_path);
void test_studio_host_launch_dimension_border_target_selector_value_diagnostics_localize(
    const std::string& studio_host_path);
void test_studio_host_launch_dynamic_target_selector_value_diagnostics_localize(
    const std::string& studio_host_path);
void test_studio_host_launch_max_selection_target_selector_value_diagnostics_localize(
    const std::string& studio_host_path);

// ==== CLI launch-flag diagnostics localization tests: non-selector values ====
void test_studio_host_launch_core_value_diagnostics_localize(const std::string& studio_host_path);
void test_studio_host_launch_layout_state_value_diagnostics_localize(const std::string& studio_host_path);
void test_studio_host_launch_text_media_list_value_diagnostics_localize(const std::string& studio_host_path);
void test_studio_host_launch_list_scalar_value_diagnostics_localize(const std::string& studio_host_path);
void test_studio_host_launch_grid_value_diagnostics_localize(const std::string& studio_host_path);
void test_studio_host_launch_record_list_value_diagnostics_localize(const std::string& studio_host_path);
void test_studio_host_launch_window_flag_value_diagnostics_localize(const std::string& studio_host_path);
void test_studio_host_launch_bounds_border_value_diagnostics_localize(const std::string& studio_host_path);
void test_studio_host_launch_dynamic_expression_value_diagnostics_localize(
    const std::string& studio_host_path);
void test_studio_host_launch_max_auto_selection_value_diagnostics_localize(
    const std::string& studio_host_path);
void test_studio_host_launch_deleted_state_value_diagnostics_localize(
    const std::string& studio_host_path);

// ==== CLI launch-flag diagnostics localization tests: general/structural ====
void test_studio_host_launch_object_metadata_diagnostics_localize(const std::string& studio_host_path);
void test_studio_host_launch_layout_action_diagnostics_localize(const std::string& studio_host_path);
void test_studio_host_launch_tab_visibility_diagnostics_localize(const std::string& studio_host_path);
void test_studio_host_launch_basic_visual_property_diagnostics_localize(const std::string& studio_host_path);
void test_studio_host_launch_list_grid_property_diagnostics_localize(const std::string& studio_host_path);
void test_studio_host_launch_text_binding_diagnostics_localize(const std::string& studio_host_path);
void test_studio_host_launch_row_list_diagnostics_localize(const std::string& studio_host_path);
void test_studio_host_launch_deleted_state_diagnostics_localize(const std::string& studio_host_path);
void test_studio_host_launch_bound_list_numeric_diagnostics_localize(const std::string& studio_host_path);
void test_studio_host_launch_window_flag_diagnostics_localize(const std::string& studio_host_path);
void test_studio_host_launch_form_bounds_style_diagnostics_localize(const std::string& studio_host_path);
void test_studio_host_launch_dynamic_expression_diagnostics_localize(const std::string& studio_host_path);
void test_studio_host_launch_max_auto_diagnostics_localize(const std::string& studio_host_path);
void test_studio_host_launch_selection_marker_diagnostics_localize(const std::string& studio_host_path);
void test_studio_host_launch_sizing_zorder_diagnostics_localize(const std::string& studio_host_path);
void test_studio_host_launch_command_mode_diagnostics_localize(const std::string& studio_host_path);

// ==== Toolbox subsystem parse-diagnostics localization tests ====
void test_studio_host_toolbox_palette_parse_diagnostics_localize(const std::string& studio_host_path);
void test_studio_host_toolbox_parse_diagnostics_localize(const std::string& studio_host_path);
void test_studio_host_toolbox_create_plan_parse_diagnostics_localize(const std::string& studio_host_path);
void test_studio_host_toolbox_dispatch_create_parse_diagnostics_localize(const std::string& studio_host_path);
void test_studio_host_toolbox_batch_dispatch_parse_diagnostics_localize(const std::string& studio_host_path);
void test_studio_host_toolbox_batch_dispatch_plan_parse_diagnostics_localize(const std::string& studio_host_path);
void test_studio_host_selection_toolbox_create_plan_parse_diagnostics_localize(const std::string& studio_host_path);
void test_studio_host_selection_toolbox_batch_create_parse_diagnostics_localize(const std::string& studio_host_path);
void test_studio_host_selection_toolbox_batch_dispatch_parse_diagnostics_localize(const std::string& studio_host_path);
void test_studio_host_toolbox_batch_dispatch_direct_parse_diagnostics_localize(const std::string& studio_host_path);
void test_studio_host_toolbox_batch_create_direct_parse_diagnostics_localize(const std::string& studio_host_path);
void test_studio_host_toolbox_batch_plan_catalog_parse_diagnostics_localize(const std::string& studio_host_path);
void test_studio_host_toolbox_plan_catalog_parse_diagnostics_localize(const std::string& studio_host_path);
void test_studio_host_toolbox_dispatch_catalog_parse_diagnostics_localize(const std::string& studio_host_path);
void test_studio_host_toolbox_batch_dispatch_catalog_parse_diagnostics_localize(const std::string& studio_host_path);
void test_studio_host_toolbox_direct_plan_parse_diagnostics_localize(const std::string& studio_host_path);
void test_studio_host_toolbox_direct_create_parse_diagnostics_localize(const std::string& studio_host_path);

// ==== Visual asset editor subsystem parse-diagnostics localization tests ====
void test_studio_host_visual_property_core_parse_diagnostics_localize(const std::string& studio_host_path);
void test_studio_host_visual_property_copy_move_parse_diagnostics_localize(const std::string& studio_host_path);
void test_studio_host_visual_property_rename_reorder_parse_diagnostics_localize(
    const std::string& studio_host_path);
void test_studio_host_visual_list_navigation_parse_diagnostics_localize(const std::string& studio_host_path);
void test_studio_host_visual_object_reparent_duplicate_parse_diagnostics_localize(
    const std::string& studio_host_path);
void test_studio_host_visual_object_rename_reorder_update_parse_diagnostics_localize(
    const std::string& studio_host_path);
void test_studio_host_visual_method_core_parse_diagnostics_localize(
    const std::string& studio_host_path);
void test_studio_host_visual_method_delete_rename_parse_diagnostics_localize(
    const std::string& studio_host_path);
void test_studio_host_visual_method_copy_move_parse_diagnostics_localize(
    const std::string& studio_host_path);
void test_studio_host_visual_method_reorder_parse_diagnostics_localize(
    const std::string& studio_host_path);

// ==== Builder/designer/editor-action parse-diagnostics and CLI-wide localization tests ====
void test_studio_host_usage_exposes_selected_execution_catalogs(const std::string& studio_host_path);
void test_studio_host_list_subsystems_localizes_descriptor_text(const std::string& studio_host_path);
void test_studio_host_builder_parse_diagnostics_localize(const std::string& studio_host_path);
void test_studio_host_editor_action_parse_diagnostics_localize(const std::string& studio_host_path);
void test_studio_host_designer_parse_diagnostics_localize(const std::string& studio_host_path);
void test_studio_host_execution_fallback_errors_localize(const std::string& studio_host_path);

// ==== Toolbox subsystem dispatch/catalog/execution/invocation-admission JSON exposure tests ====
void test_studio_host_json_exposes_toolbox_invocation_admission(const std::string& studio_host_path);
void test_studio_host_json_exposes_toolbox_invocation_admission_catalog(const std::string& studio_host_path);
void test_studio_host_json_exposes_selection_toolbox_invocation_admission_catalog(
    const std::string& studio_host_path);
void test_studio_host_json_exposes_toolbox_dispatch(const std::string& studio_host_path);
void test_studio_host_json_exposes_toolbox_execution(const std::string& studio_host_path);
void test_studio_host_json_exposes_toolbox_dispatch_catalog(const std::string& studio_host_path);
void test_studio_host_json_exposes_toolbox_dispatch_execution_catalog(const std::string& studio_host_path);
void test_studio_host_json_exposes_selection_toolbox_dispatch_catalog(const std::string& studio_host_path);
void test_studio_host_json_exposes_selection_toolbox_dispatch_execution_catalog(
    const std::string& studio_host_path);

// ==== Toolbox subsystem palette launch/query JSON exposure tests ====
void test_studio_host_json_exposes_toolbox_palette_launch_plans(const std::string& studio_host_path);
void test_studio_host_json_exposes_toolbox_palette_launch_catalog(const std::string& studio_host_path);
void test_studio_host_json_exposes_toolbox_palette_query_filters(const std::string& studio_host_path);
void test_studio_host_json_plans_toolbox_object_creation_from_palette_dispatch(
    const std::string& studio_host_path);
void test_studio_host_json_creates_toolbox_object_from_palette_dispatch(
    const std::string& studio_host_path);
void test_studio_host_json_plans_toolbox_object_creation_dispatches_from_palette_dispatch(
    const std::string& studio_host_path);
void test_studio_host_json_plans_toolbox_object_creation_batches_from_palette_dispatch(
    const std::string& studio_host_path);
void test_studio_host_json_creates_toolbox_object_batches_from_palette_dispatch(
    const std::string& studio_host_path);
void test_studio_host_json_plans_toolbox_object_creation_batch_dispatches_from_palette_dispatch(
    const std::string& studio_host_path);

// ==== Toolbox subsystem object-creation plan JSON exposure tests ====
void test_studio_host_json_plans_toolbox_object_creation(const std::string& studio_host_path);
void test_studio_host_json_plans_selection_toolbox_object_creation(const std::string& studio_host_path);
void test_studio_host_json_plans_toolbox_object_creation_dispatch(const std::string& studio_host_path);
void test_studio_host_json_plans_selection_toolbox_object_creation_dispatch(const std::string& studio_host_path);
void test_studio_host_json_plans_toolbox_object_creation_catalog(const std::string& studio_host_path);
void test_studio_host_json_plans_selection_toolbox_object_creation_catalog(
    const std::string& studio_host_path);
void test_studio_host_json_plans_toolbox_object_creation_dispatch_catalog(const std::string& studio_host_path);
void test_studio_host_json_plans_selection_toolbox_object_creation_dispatch_catalog(
    const std::string& studio_host_path);
void test_studio_host_json_plans_toolbox_object_creation_batch_plan_catalog(
    const std::string& studio_host_path);
void test_studio_host_json_plans_selection_toolbox_object_creation_batch_plan_catalog(
    const std::string& studio_host_path);
void test_studio_host_json_plans_toolbox_object_creation_batch_dispatch_catalog(
    const std::string& studio_host_path);
void test_studio_host_json_plans_selection_toolbox_object_creation_batch_dispatch_catalog(
    const std::string& studio_host_path);
void test_studio_host_json_plans_toolbox_object_creation_batches(const std::string& studio_host_path);
void test_studio_host_json_plans_selection_toolbox_object_creation_batches(const std::string& studio_host_path);
void test_studio_host_json_plans_toolbox_object_creation_batch_dispatch(const std::string& studio_host_path);
void test_studio_host_json_plans_selection_toolbox_object_creation_batch_dispatch(
    const std::string& studio_host_path);

// ==== Builder subsystem dispatch/catalog/execution JSON exposure tests ====
void test_studio_host_json_exposes_builder_launch_plans(const std::string& studio_host_path);
void test_studio_host_json_exposes_builder_launch_catalog(const std::string& studio_host_path);
void test_studio_host_json_exposes_selection_builder_launch_catalog(const std::string& studio_host_path);
void test_studio_host_json_exposes_builder_invocation_admission(const std::string& studio_host_path);
void test_studio_host_json_exposes_builder_invocation_admission_catalog(
    const std::string& studio_host_path);
void test_studio_host_json_exposes_selection_builder_invocation_admission_catalog(
    const std::string& studio_host_path);
void test_studio_host_json_exposes_selection_builder_dispatch_catalog(
    const std::string& studio_host_path);
void test_studio_host_json_exposes_selection_builder_dispatch_execution_catalog(
    const std::string& studio_host_path);
void test_studio_host_json_exposes_builder_dispatch(const std::string& studio_host_path);
void test_studio_host_json_exposes_builder_execution(const std::string& studio_host_path);
void test_studio_host_json_exposes_builder_dispatch_catalog(const std::string& studio_host_path);
void test_studio_host_json_exposes_builder_dispatch_execution_catalog(const std::string& studio_host_path);

// ==== Designer subsystem dispatch/catalog/execution JSON exposure tests ====
void test_studio_host_json_exposes_designer_contexts(const std::string& studio_host_path);
void test_studio_host_json_preserves_sidecar_path_spelling(const std::string& studio_host_path);
void test_studio_host_json_exposes_designer_launch_surfaces(const std::string& studio_host_path);
void test_studio_host_json_exposes_designer_invocation_admission(const std::string& studio_host_path);
void test_studio_host_json_exposes_designer_dispatch(const std::string& studio_host_path);
void test_studio_host_json_exposes_designer_execution(const std::string& studio_host_path);
void test_studio_host_json_exposes_designer_dispatch_catalog(const std::string& studio_host_path);
void test_studio_host_json_exposes_designer_dispatch_execution_catalog(const std::string& studio_host_path);
void test_studio_host_json_exposes_designer_launch_surface_catalog(const std::string& studio_host_path);
void test_studio_host_json_exposes_designer_invocation_admission_catalog(const std::string& studio_host_path);

// ==== Editor-action subsystem dispatch/catalog/execution JSON exposure tests ====
void test_studio_host_json_exposes_editor_action_launch_plans(const std::string& studio_host_path);
void test_studio_host_json_exposes_editor_action_launch_catalog(const std::string& studio_host_path);
void test_studio_host_json_exposes_editor_action_invocation_admission(const std::string& studio_host_path);
void test_studio_host_json_exposes_editor_action_invocation_admission_catalog(
    const std::string& studio_host_path);
void test_studio_host_json_exposes_editor_action_dispatch(const std::string& studio_host_path);
void test_studio_host_json_exposes_editor_action_execution(const std::string& studio_host_path);
void test_studio_host_json_exposes_editor_action_dispatch_catalog(const std::string& studio_host_path);
void test_studio_host_json_exposes_editor_action_dispatch_execution_catalog(
    const std::string& studio_host_path);

// ==== Object lifecycle tests (create/rename/reorder/duplicate/move/delete/restore/group) ====
void write_synthetic_form_table_for_deleted_states(const std::filesystem::path& form_path);
void mark_deleted_for_deleted_states_fixture(const std::filesystem::path& form_path,
                                             const std::string& unique_id,
                                             bool deleted,
                                             const char* message);
void write_synthetic_form_table_for_subtree_deleted_state(const std::filesystem::path& form_path);
std::filesystem::path write_synthetic_form_table_for_property_rename(
    const std::filesystem::path& temp_root,
    const std::string& file_name);
void write_synthetic_form_table_for_object_reparent(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_reorder(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_group(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_delete_mark(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_ungroup(const std::filesystem::path& form_path);
void test_studio_host_json_creates_toolbox_object_batches(const std::string& studio_host_path);
void test_studio_host_json_creates_selection_toolbox_object_batches(const std::string& studio_host_path);
void test_studio_host_json_creates_selection_toolbox_objects(const std::string& studio_host_path);
void test_studio_host_json_creates_toolbox_objects(const std::string& studio_host_path);
void test_studio_host_json_sets_properties_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_renames_properties_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_applies_deleted_states_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_applies_subtree_deleted_state_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_duplicates_visual_object_subtrees(const std::string& studio_host_path);
void test_studio_host_json_deletes_objects_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_restores_objects_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_duplicates_objects_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_renames_objects_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_reparents_objects_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_reorders_objects_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_groups_objects_by_stable_child_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_delete_mark_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_ungroups_objects_by_stable_selectors(const std::string& studio_host_path);

// ==== Layout action tests (align/distribute/snap/resize/nudge) ====
void write_synthetic_form_table_for_object_align(const std::filesystem::path& form_path);
// ==== Behavior-property setter tests: window chrome and positioning ====
void write_synthetic_form_table_for_object_caption(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_whats_this_help_id(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_whats_this_help(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_whats_this_button(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_status_bar_text(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_closable(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_control_box(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_auto_center(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_desktop(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_key_preview(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_mac_desktop(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_max_button(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_min_button(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_min_height(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_min_width(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_max_height(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_movable(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_half_height_caption(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_mdi_form(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_max_width(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_max_left(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_max_top(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_dockable(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_lock_screen(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_split_bar(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_panel_link(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_resizable(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_always_on_top(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_always_on_bottom(const std::filesystem::path& form_path);
void test_studio_host_json_assigns_caption_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_whats_this_help_id_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_whats_this_help_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_whats_this_button_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_status_bar_text_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_closable_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_control_box_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_desktop_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_key_preview_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_mac_desktop_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_max_button_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_min_button_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_min_height_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_min_width_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_max_height_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_movable_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_half_height_caption_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_mdi_form_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_max_width_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_max_left_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_max_top_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_auto_center_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_dockable_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_lock_screen_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_split_bar_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_panel_link_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_resizable_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_always_on_top_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_always_on_bottom_by_stable_selectors(const std::string& studio_host_path);

// ==== Behavior-property setter tests: grid and selection behavior ====
void write_synthetic_form_table_for_object_continuous_scroll(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_sparse(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_allow_cell_selection(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_hide_selection(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_record_mark(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_highlight_row(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_allow_header_sizing(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_allow_row_sizing(const std::filesystem::path& form_path);
void test_studio_host_json_assigns_continuous_scroll_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_sparse_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_allow_cell_selection_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_hide_selection_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_record_mark_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_highlight_row_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_allow_header_sizing_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_allow_row_sizing_by_stable_selectors(const std::string& studio_host_path);

// ==== Behavior-property setter tests: data binding and control behavior ====
void write_synthetic_form_table_for_object_list_item_id(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_control_source(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_current_control(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_auto_verb_menu(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_bind_controls(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_auto_size(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_auto_release(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_clip_controls(const std::filesystem::path& form_path);
void test_studio_host_json_assigns_list_item_id_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_control_source_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_current_control_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_bind_controls_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_auto_verb_menu_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_auto_size_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_auto_release_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_clip_controls_by_stable_selectors(const std::string& studio_host_path);

// ==== Behavior-property setter tests: general object state ====
void write_synthetic_form_table_for_object_tab_order(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_tab_stop(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_visibility(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_enabled(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_read_only(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_locked(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_tab_orientation(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_display_orientation(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_help_context_id(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_tooltip_text(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_style(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_allow_output(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_add_line_feeds(const std::filesystem::path& form_path);
void test_studio_host_json_assigns_tab_order_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_tab_stop_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_visibility_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_enabled_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_read_only_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_locked_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_tab_orientation_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_display_orientation_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_help_context_id_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_tooltip_text_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_style_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_allow_output_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_add_line_feeds_by_stable_selectors(const std::string& studio_host_path);

// ==== Appearance-property setter tests: colors ====
void write_synthetic_form_table_for_object_selected_back_color(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_selected_fore_color(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_selected_item_back_color(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_selected_item_fore_color(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_disabled_item_back_color(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_disabled_item_fore_color(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_item_back_color(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_item_fore_color(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_highlight_back_color(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_highlight_fore_color(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_back_color(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_fore_color(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_disabled_back_color(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_disabled_fore_color(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_dynamic_back_color(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_dynamic_fore_color(const std::filesystem::path& form_path);
void test_studio_host_json_assigns_selected_back_color_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_selected_fore_color_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_selected_item_back_color_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_selected_item_fore_color_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_disabled_item_back_color_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_disabled_item_fore_color_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_item_back_color_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_item_fore_color_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_highlight_back_color_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_highlight_fore_color_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_back_color_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_fore_color_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_disabled_back_color_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_disabled_fore_color_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_dynamic_back_color_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_dynamic_fore_color_by_stable_selectors(const std::string& studio_host_path);

// ==== Appearance-property setter tests: fonts ====
void write_synthetic_form_table_for_object_dynamic_line_height(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_dynamic_font_name(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_dynamic_font_size(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_dynamic_font_bold(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_dynamic_font_italic(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_dynamic_font_underline(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_dynamic_font_strikethru(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_dynamic_font_outline(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_dynamic_font_shadow(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_font_name(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_font_size(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_font_bold(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_font_italic(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_font_underline(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_font_strikethru(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_font_outline(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_font_shadow(const std::filesystem::path& form_path);
void test_studio_host_json_assigns_dynamic_line_height_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_dynamic_font_name_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_dynamic_font_size_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_dynamic_font_bold_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_dynamic_font_italic_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_dynamic_font_underline_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_dynamic_font_strikethru_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_dynamic_font_outline_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_dynamic_font_shadow_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_font_name_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_font_size_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_font_bold_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_font_italic_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_font_underline_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_font_strikethru_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_font_outline_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_font_shadow_by_stable_selectors(const std::string& studio_host_path);

// ==== Appearance-property setter tests: pictures and icons ====
void write_synthetic_form_table_for_object_picture(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_down_picture(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_disabled_picture(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_ole_drag_picture(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_mouse_icon(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_drag_icon(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_picture_margin(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_picture_position(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_picture_spacing(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_picture_selection_display(const std::filesystem::path& form_path);
void test_studio_host_json_assigns_picture_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_down_picture_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_disabled_picture_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_ole_drag_picture_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_mouse_icon_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_drag_icon_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_picture_margin_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_picture_position_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_picture_spacing_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_picture_selection_display_by_stable_selectors(const std::string& studio_host_path);

// ==== Appearance-property setter tests: borders, fill, and drawing ====
void write_synthetic_form_table_for_object_curvature(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_draw_mode(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_draw_style(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_draw_width(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_fill_style(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_grid_line_color(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_grid_line_width(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_grid_lines(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_fill_color(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_back_style(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_border_style(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_border_width(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_border_color(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_special_effect(const std::filesystem::path& form_path);
void test_studio_host_json_assigns_curvature_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_draw_mode_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_draw_style_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_draw_width_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_fill_style_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_grid_line_color_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_grid_line_width_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_grid_lines_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_fill_color_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_back_style_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_border_style_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_border_width_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_border_color_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_special_effect_by_stable_selectors(const std::string& studio_host_path);

// ==== Appearance-property setter tests: window and display chrome ====
void write_synthetic_form_table_for_object_scale_mode(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_header_height(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_row_height(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_highlight_row_line_width(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_highlight_style(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_input_mask(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_format(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_scroll_bars(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_window_state(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_show_window(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_title_bar(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_mouse_pointer(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_dynamic_input_mask(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_dynamic_current_control(const std::filesystem::path& form_path);
void test_studio_host_json_assigns_scale_mode_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_header_height_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_row_height_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_highlight_row_line_width_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_highlight_style_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_input_mask_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_format_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_scroll_bars_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_window_state_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_show_window_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_title_bar_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_mouse_pointer_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_dynamic_input_mask_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_dynamic_current_control_by_stable_selectors(const std::string& studio_host_path);

// ==== Appearance-property setter tests: OLE drag/drop and buffering ====
void write_synthetic_form_table_for_object_drag_mode(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_ole_drag_mode(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_ole_drop_mode(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_ole_drop_effects(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_ole_drop_text_insertion(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_buffer_mode(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_buffer_mode_override(const std::filesystem::path& form_path);
void test_studio_host_json_assigns_drag_mode_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_ole_drag_mode_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_ole_drop_mode_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_ole_drop_effects_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_ole_drop_text_insertion_by_stable_selectors(
    const std::string& studio_host_path);
void test_studio_host_json_assigns_buffer_mode_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_buffer_mode_override_by_stable_selectors(const std::string& studio_host_path);

// ==== Property setter tests (data-binding properties) by stable selectors ====
void write_synthetic_form_table_for_object_button_count(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_data_session(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_lock_columns(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_lock_columns_left(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_partition(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_record_source_type(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_column_order(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_child_order(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_record_source(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_form_set_class(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_default_file_path(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_initial_selected_alias(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_link_master(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_row_source(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_column_widths(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_column_lines(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_integral_height(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_incremental_search(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_multi_select(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_row_source_type(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_bound_column(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_column_count(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_list_index(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_left_column(const std::filesystem::path& form_path);
void write_synthetic_form_table_for_object_display_value(const std::filesystem::path& form_path);
void test_studio_host_json_assigns_button_count_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_data_session_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_lock_columns_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_lock_columns_left_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_partition_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_record_source_type_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_column_order_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_child_order_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_record_source_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_form_set_class_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_default_file_path_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_initial_selected_alias_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_link_master_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_row_source_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_column_widths_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_column_lines_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_integral_height_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_incremental_search_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_multi_select_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_row_source_type_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_bound_column_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_column_count_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_list_index_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_left_column_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_assigns_display_value_by_stable_selectors(const std::string& studio_host_path);

// ==== Report layout JSON tests: label layout/settings/objects/sections ====
void test_studio_host_json_exposes_label_layout_parity(const std::string& studio_host_path);
void test_studio_host_json_nudges_label_layout_objects_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_aligns_label_layout_objects_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_resizes_label_layout_objects_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_snaps_label_layout_objects_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_deletes_label_layout_objects_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_restores_label_layout_objects_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_distributes_label_layout_objects_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_reorders_label_layout_objects_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_duplicates_label_layout_objects_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_renames_label_layout_object_identity_by_stable_selectors(
    const std::string& studio_host_path);
void test_studio_host_json_updates_label_layout_object_expressions_by_record_selection(
    const std::string& studio_host_path);
void test_studio_host_json_updates_label_layout_object_expression_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_updates_deleted_label_layout_object_expressions_by_record_selection(
    const std::string& studio_host_path);
void test_studio_host_json_clears_label_layout_object_expressions_by_record_selection(
    const std::string& studio_host_path);
void test_studio_host_json_clears_label_layout_object_expression_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_clears_deleted_label_layout_object_expressions_by_record_selection(
    const std::string& studio_host_path);
void test_studio_host_json_restores_edited_deleted_label_layout_object_as_unplaced_by_record_selection(
    const std::string& studio_host_path);
void test_studio_host_json_restores_edited_deleted_label_layout_object_as_unplaced_by_stable_selection(
    const std::string& studio_host_path);

// ==== Report layout JSON tests: deleted visual-property copy/move/rename/reorder ====
void test_studio_host_json_clears_deleted_report_visual_properties_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_clears_deleted_report_visual_property_batches_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_copies_deleted_report_visual_properties_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_copies_deleted_report_visual_property_batches_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_moves_deleted_report_visual_properties_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_moves_deleted_report_visual_property_batches_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_rejects_deleted_report_visual_property_rename_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_rejects_deleted_report_visual_property_rename_batches_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_rejects_deleted_report_visual_property_reorder_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_rejects_deleted_report_visual_property_reorder_batches_by_stable_selection(
    const std::string& studio_host_path);

// ==== Report layout JSON tests: object/section classifications, objtype schema, band codes ====
void expect_normalized_classification_preview_bounds(const std::string& text, const std::string& prefix);
void expect_unknown_band_preview_bounds(const std::string& text, const std::string& prefix);
void write_synthetic_report_table_for_invalid_classification_layout_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_dot_leading_classification_layout_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_negative_classification_layout_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_unsupported_objtype_layout_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_fractional_classification_layout_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_trimmed_classification_layout_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_missing_classification_layout_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_missing_objtype_layout_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_unknown_band_layout_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_invalid_direct_page_setup_layout_json(
    const std::filesystem::path& report_path);
void test_studio_host_json_ignores_invalid_report_layout_classifications(
    const std::string& studio_host_path);
void test_studio_host_json_ignores_dot_leading_report_layout_classifications(
    const std::string& studio_host_path);
void test_studio_host_json_ignores_negative_report_layout_classifications(
    const std::string& studio_host_path);
void test_studio_host_json_ignores_unsupported_report_layout_objtype_codes(
    const std::string& studio_host_path);
void test_studio_host_json_uses_integer_portions_for_fractional_report_layout_classifications(
    const std::string& studio_host_path);
void test_studio_host_json_trims_report_layout_classifications(
    const std::string& studio_host_path);
void test_studio_host_json_ignores_missing_report_layout_classification_fields(
    const std::string& studio_host_path);
void test_studio_host_json_ignores_missing_report_layout_objtype_schema(
    const std::string& studio_host_path);
void test_studio_host_json_exposes_unknown_report_band_codes(const std::string& studio_host_path);
void test_studio_host_json_ignores_invalid_direct_report_page_setup_fields(
    const std::string& studio_host_path);

// ==== Report layout JSON tests: object preview-bounds geometry ====
void expect_full_report_layout_preview_bounds(const std::string& text, const std::string& prefix);
void expect_empty_report_layout_preview_bounds(const std::string& text, const std::string& prefix);
void expect_fractional_geometry_preview_bounds(const std::string& text, const std::string& prefix);
void expect_negative_dimension_preview_bounds(const std::string& text, const std::string& prefix);
void expect_zero_available_report_layout_preview_bounds(const std::string& text, const std::string& prefix);
void expect_unresolved_memo_preview_bounds(const std::string& text, const std::string& prefix);
void expect_unresolved_section_memo_preview_bounds(const std::string& text, const std::string& prefix);
void expect_unresolved_deleted_object_memo_preview_bounds(
    const std::string& text,
    const std::string& prefix);
void expect_unresolved_unplaced_object_memo_preview_bounds(
    const std::string& text,
    const std::string& prefix);
void expect_missing_section_objcode_preview_bounds(const std::string& text, const std::string& prefix);
void expect_missing_object_objcode_preview_bounds(const std::string& text, const std::string& prefix);
void expect_missing_object_expr_preview_bounds(const std::string& text, const std::string& prefix);
void expect_missing_object_title_preview_bounds(const std::string& text, const std::string& prefix);
void test_studio_host_json_refreshes_detail_header_footer_section_preview_bounds_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_updates_report_layout_object_width_preview_bounds_by_record_selection(
    const std::string& studio_host_path);
void test_studio_host_json_updates_report_layout_object_width_preview_bounds_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_clears_report_layout_object_width_preview_bounds_by_record_selection(
    const std::string& studio_host_path);
void test_studio_host_json_clears_report_layout_object_width_preview_bounds_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_updates_report_layout_object_left_preview_bounds_by_record_selection(
    const std::string& studio_host_path);
void test_studio_host_json_updates_report_layout_object_left_preview_bounds_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_clears_report_layout_object_left_preview_bounds_by_record_selection(
    const std::string& studio_host_path);
void test_studio_host_json_clears_report_layout_object_left_preview_bounds_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_updates_report_layout_object_height_preview_bounds_by_record_selection(
    const std::string& studio_host_path);
void test_studio_host_json_clears_report_layout_object_height_preview_bounds_by_record_selection(
    const std::string& studio_host_path);
void test_studio_host_json_updates_report_layout_object_height_preview_bounds_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_clears_report_layout_object_height_preview_bounds_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_updates_report_layout_object_top_preview_bounds_by_record_selection(
    const std::string& studio_host_path);
void test_studio_host_json_clears_report_layout_object_top_preview_bounds_by_record_selection(
    const std::string& studio_host_path);
void test_studio_host_json_updates_report_layout_object_top_preview_bounds_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_clears_report_layout_object_top_preview_bounds_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_refreshes_deleted_detail_header_footer_section_preview_bounds_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_refreshes_detail_header_footer_section_delete_restore_preview_bounds_by_stable_selection(
    const std::string& studio_host_path);

// ==== Report layout JSON tests: object height/left/top/width geometry ====
void write_synthetic_report_table_for_negative_dimension_layout_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_missing_geometry_layout_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_unresolved_geometry_memo_layout_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_missing_section_geometry_layout_json(
    const std::filesystem::path& report_path);
void test_studio_host_json_deletes_edited_report_layout_object_geometry_by_record_selection(
    const std::string& studio_host_path);
void test_studio_host_json_deletes_edited_unplaced_report_layout_object_geometry_by_record_selection(
    const std::string& studio_host_path);
void test_studio_host_json_updates_deleted_report_layout_object_width_by_record_selection(
    const std::string& studio_host_path);
void test_studio_host_json_clears_deleted_report_layout_object_width_by_record_selection(
    const std::string& studio_host_path);
void test_studio_host_json_updates_deleted_report_layout_object_width_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_clears_deleted_report_layout_object_width_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_updates_deleted_report_layout_object_left_by_record_selection(
    const std::string& studio_host_path);
void test_studio_host_json_clears_deleted_report_layout_object_left_by_record_selection(
    const std::string& studio_host_path);
void test_studio_host_json_updates_deleted_report_layout_object_left_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_clears_deleted_report_layout_object_left_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_updates_deleted_report_layout_object_height_by_record_selection(
    const std::string& studio_host_path);
void test_studio_host_json_clears_deleted_report_layout_object_height_by_record_selection(
    const std::string& studio_host_path);
void test_studio_host_json_updates_deleted_report_layout_object_height_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_clears_deleted_report_layout_object_height_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_updates_deleted_report_layout_object_top_by_record_selection(
    const std::string& studio_host_path);
void test_studio_host_json_clears_deleted_report_layout_object_top_by_record_selection(
    const std::string& studio_host_path);
void test_studio_host_json_updates_deleted_report_layout_object_top_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_clears_deleted_report_layout_object_top_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_restores_edited_deleted_report_layout_object_geometry_by_record_selection(
    const std::string& studio_host_path);
void test_studio_host_json_restores_edited_deleted_report_layout_object_geometry_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_updates_detail_header_footer_object_geometry_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_updates_deleted_detail_header_footer_object_geometry_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_clamps_negative_report_layout_dimensions(
    const std::string& studio_host_path);
void test_studio_host_json_uses_integer_portions_for_fractional_report_layout_geometry(
    const std::string& studio_host_path);
void test_studio_host_json_defaults_missing_report_layout_geometry_fields(
    const std::string& studio_host_path);
void test_studio_host_json_defaults_unresolved_report_geometry_memo_placeholders(
    const std::string& studio_host_path);
void test_studio_host_json_defaults_report_sections_without_geometry_schema(
    const std::string& studio_host_path);

// ==== Report layout JSON tests: object font metadata and options ====
void write_synthetic_report_table_for_layout_font_options_json(const std::filesystem::path& report_path);
void test_studio_host_json_updates_report_layout_object_font_metadata_by_record_selection(
    const std::string& studio_host_path);
void test_studio_host_json_updates_report_layout_object_font_metadata_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_updates_deleted_report_layout_object_font_metadata_by_record_selection(
    const std::string& studio_host_path);
void test_studio_host_json_clears_report_layout_object_font_metadata_by_record_selection(
    const std::string& studio_host_path);
void test_studio_host_json_clears_report_layout_object_font_metadata_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_clears_deleted_report_layout_object_font_metadata_by_record_selection(
    const std::string& studio_host_path);
void test_studio_host_json_updates_report_layout_object_font_options_by_record_selection(
    const std::string& studio_host_path);
void test_studio_host_json_updates_report_layout_object_font_options_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_updates_deleted_report_layout_object_font_options_by_record_selection(
    const std::string& studio_host_path);
void test_studio_host_json_exposes_detail_header_footer_object_font_metadata_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_exposes_deleted_detail_header_footer_object_font_metadata_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_updates_detail_header_footer_object_font_metadata_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_updates_deleted_detail_header_footer_object_font_metadata_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_updates_detail_header_footer_object_font_options_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_updates_deleted_detail_header_footer_object_font_options_by_stable_selection(
    const std::string& studio_host_path);

// ==== Report layout JSON tests: column-width geometry fields ====
void write_synthetic_report_table_for_column_width_field_json(const std::filesystem::path& report_path);
void write_synthetic_report_table_for_deleted_column_width_field_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_stable_column_width_field_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_stable_deleted_column_width_field_json(
    const std::filesystem::path& report_path);
void test_studio_host_json_updates_report_column_width_fields_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_clears_report_column_width_fields_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_updates_deleted_report_column_width_fields_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_clears_deleted_report_column_width_fields_by_stable_selection(
    const std::string& studio_host_path);

// ==== Report layout JSON tests: page setup fields (margins/grid/orientation/paper size) ====
void write_synthetic_report_table_for_invalid_direct_margin_grid_layout_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_bottom_margin_field_json(const std::filesystem::path& report_path);
void write_synthetic_report_table_for_deleted_bottom_margin_field_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_stable_bottom_margin_field_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_stable_deleted_bottom_margin_field_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_left_margin_field_json(const std::filesystem::path& report_path);
void write_synthetic_report_table_for_deleted_left_margin_field_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_stable_left_margin_field_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_stable_deleted_left_margin_field_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_right_margin_field_json(const std::filesystem::path& report_path);
void write_synthetic_report_table_for_deleted_right_margin_field_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_stable_right_margin_field_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_stable_deleted_right_margin_field_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_grid_vertical_field_json(const std::filesystem::path& report_path);
void write_synthetic_report_table_for_deleted_grid_vertical_field_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_stable_grid_vertical_field_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_stable_deleted_grid_vertical_field_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_grid_horizontal_field_json(const std::filesystem::path& report_path);
void write_synthetic_report_table_for_deleted_grid_horizontal_field_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_stable_grid_horizontal_field_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_stable_deleted_grid_horizontal_field_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_orientation_field_json(const std::filesystem::path& report_path);
void write_synthetic_report_table_for_deleted_orientation_field_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_stable_orientation_field_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_stable_deleted_orientation_field_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_paper_size_field_json(const std::filesystem::path& report_path);
void write_synthetic_report_table_for_deleted_paper_size_field_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_stable_paper_size_field_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_stable_deleted_paper_size_field_json(
    const std::filesystem::path& report_path);
void test_studio_host_json_updates_report_bottom_margin_fields_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_clears_report_bottom_margin_fields_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_updates_deleted_report_bottom_margin_fields_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_clears_deleted_report_bottom_margin_fields_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_updates_report_left_margin_fields_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_clears_report_left_margin_fields_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_updates_deleted_report_left_margin_fields_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_clears_deleted_report_left_margin_fields_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_updates_report_right_margin_fields_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_clears_report_right_margin_fields_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_updates_deleted_report_right_margin_fields_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_clears_deleted_report_right_margin_fields_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_updates_report_grid_vertical_fields_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_clears_report_grid_vertical_fields_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_updates_deleted_report_grid_vertical_fields_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_clears_deleted_report_grid_vertical_fields_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_updates_report_grid_horizontal_fields_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_clears_report_grid_horizontal_fields_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_updates_deleted_report_grid_horizontal_fields_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_clears_deleted_report_grid_horizontal_fields_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_updates_report_orientation_fields_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_clears_report_orientation_fields_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_updates_deleted_report_orientation_fields_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_clears_deleted_report_orientation_fields_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_updates_report_paper_size_fields_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_clears_report_paper_size_fields_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_updates_deleted_report_paper_size_fields_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_clears_deleted_report_paper_size_fields_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_ignores_invalid_direct_report_margin_grid_fields(
    const std::string& studio_host_path);

// ==== Report layout JSON tests: settings-memo parsing ====
void write_synthetic_report_table_for_unresolved_direct_setting_memo_layout_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_mixed_direct_setting_memo_layout_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_invalid_setting_memo_layout_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_fractional_setting_memo_layout_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_blank_setting_memo_layout_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_malformed_setting_memo_layout_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_cr_only_setting_memo_layout_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_mixed_case_setting_memo_layout_json(
    const std::filesystem::path& report_path);
void test_studio_host_json_suppresses_unresolved_report_direct_setting_memo_placeholders(
    const std::string& studio_host_path);
void test_studio_host_json_preserves_mixed_report_direct_setting_memo_placeholders(
    const std::string& studio_host_path);
void test_studio_host_json_ignores_invalid_report_setting_memo_values(
    const std::string& studio_host_path);
void test_studio_host_json_preserves_fractional_report_setting_memo_values(
    const std::string& studio_host_path);
void test_studio_host_json_ignores_blank_report_setting_memo_values(
    const std::string& studio_host_path);
void test_studio_host_json_ignores_malformed_report_setting_memo_lines(
    const std::string& studio_host_path);
void test_studio_host_json_parses_cr_only_report_setting_memo_lines(
    const std::string& studio_host_path);
void test_studio_host_json_parses_mixed_case_report_setting_memo_names(
    const std::string& studio_host_path);
void test_studio_host_json_writes_case_insensitive_expr_fields(
    const std::string& studio_host_path);

// ==== Report layout JSON tests: direct-setting field validation ====
void write_synthetic_report_table_for_blank_direct_setting_layout_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_mixed_invalid_direct_setting_layout_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_trimmed_direct_setting_layout_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_fractional_direct_setting_layout_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_oversized_direct_setting_layout_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_dot_leading_direct_setting_layout_json(
    const std::filesystem::path& report_path);
void test_studio_host_json_skips_blank_report_direct_setting_fields(
    const std::string& studio_host_path);
void test_studio_host_json_preserves_mixed_invalid_report_direct_setting_fields(
    const std::string& studio_host_path);
void test_studio_host_json_preserves_trimmed_report_direct_setting_fields(
    const std::string& studio_host_path);
void test_studio_host_json_preserves_fractional_report_direct_setting_fields(
    const std::string& studio_host_path);
void test_studio_host_json_ignores_oversized_report_direct_setting_fields(
    const std::string& studio_host_path);
void test_studio_host_json_ignores_dot_leading_report_direct_setting_fields(
    const std::string& studio_host_path);

// ==== Report layout JSON tests: column setup/count fields ====
void write_synthetic_report_table_for_invalid_direct_column_setup_layout_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_column_setup_json(const std::filesystem::path& report_path);
void write_synthetic_report_table_for_column_setup_field_json(const std::filesystem::path& report_path);
void write_synthetic_report_table_for_deleted_column_setup_field_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_stable_column_setup_field_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_stable_deleted_column_setup_field_json(
    const std::filesystem::path& report_path);
void test_studio_host_json_exposes_report_layout_column_setup(const std::string& studio_host_path);
void test_studio_host_json_updates_report_column_count_fields_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_clears_report_column_count_fields_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_updates_deleted_report_column_count_fields_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_clears_deleted_report_column_count_fields_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_ignores_invalid_direct_report_column_setup_fields(
    const std::string& studio_host_path);

// ==== Report layout JSON tests: settings exposure and precedence diagnostics ====
void write_synthetic_report_table_for_duplicate_setting_precedence_layout_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_ambiguous_settings_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_live_deleted_ambiguous_settings_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_padded_stable_settings_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_deep_stable_settings_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_deep_ambiguous_stable_settings_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_deep_live_deleted_ambiguous_stable_settings_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_deleted_settings_json(const std::filesystem::path& report_path);
void write_synthetic_report_table_for_stable_settings_and_section_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_stable_deleted_settings_and_section_json(
    const std::filesystem::path& report_path);
void test_studio_host_json_deletes_report_settings_by_record_selection(const std::string& studio_host_path);
void test_studio_host_json_preserves_report_settings_without_root_objcode_schema(
    const std::string& studio_host_path);
void test_studio_host_json_preserves_duplicate_report_setting_precedence(
    const std::string& studio_host_path);
void test_studio_host_json_preserves_invalid_first_duplicate_report_setting_precedence(
    const std::string& studio_host_path);
void test_studio_host_json_preserves_report_settings_without_root_expr_schema(
    const std::string& studio_host_path);
void test_studio_host_json_exposes_printer_identity_report_settings_summary(
    const std::string& studio_host_path);
void test_studio_host_json_exposes_color_and_copies_report_settings_summary(
    const std::string& studio_host_path);

// ==== Report layout JSON tests: section band exposure by selector ====
// ==== Report layout JSON tests: detail/header/footer section lifecycle ====
void test_studio_host_json_deletes_and_restores_detail_header_footer_sections_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_duplicates_detail_header_footer_sections_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_duplicates_deleted_detail_header_footer_sections_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_renames_detail_header_footer_sections_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_renames_deleted_detail_header_footer_sections_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_reorders_detail_header_footer_sections_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_reorders_deleted_detail_header_footer_sections_by_stable_selection(
    const std::string& studio_host_path);

// ==== Report layout JSON tests: detail/header/footer section expressions and exposure ====
void write_synthetic_report_table_for_detail_header_footer_section_kind_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_deleted_detail_header_footer_section_expression_json(
    const std::filesystem::path& report_path);
void test_studio_host_json_exposes_detail_header_footer_section_kinds(
    const std::string& studio_host_path);
void test_studio_host_json_updates_detail_header_footer_section_heights_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_clears_detail_header_footer_section_heights_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_updates_deleted_detail_header_footer_section_heights_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_clears_deleted_detail_header_footer_section_heights_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_updates_detail_header_footer_section_tops_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_clears_detail_header_footer_section_tops_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_updates_deleted_detail_header_footer_section_tops_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_clears_deleted_detail_header_footer_section_tops_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_updates_detail_header_footer_section_expressions(
    const std::string& studio_host_path);
void test_studio_host_json_exposes_detail_header_footer_section_expressions_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_updates_detail_header_footer_section_expressions_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_updates_deleted_detail_header_footer_section_expressions(
    const std::string& studio_host_path);
void test_studio_host_json_updates_deleted_detail_header_footer_section_expressions_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_exposes_deleted_detail_header_footer_sections_by_stable_selection(
    const std::string& studio_host_path);

// ==== Report layout JSON tests: section schema and selection diagnostics ====
void write_synthetic_report_table_for_missing_section_objcode_layout_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_unresolved_section_memo_layout_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_missing_section_expr_layout_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_stable_summary_section_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_live_deleted_ambiguous_summary_section_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_padded_stable_section_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_deep_stable_section_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_deep_ambiguous_stable_section_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_deep_live_deleted_ambiguous_stable_section_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_stable_title_section_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_stable_page_header_section_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_stable_column_section_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_stable_section_json(const std::filesystem::path& report_path);
void write_synthetic_report_table_for_deleted_section_json(const std::filesystem::path& report_path);
void write_synthetic_report_table_for_section_deleted_object_count_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_stable_deleted_section_json(const std::filesystem::path& report_path);
void test_studio_host_json_clears_report_section_and_settings_selection_for_ambiguous_stable_selectors(
    const std::string& studio_host_path);
void test_studio_host_json_defaults_missing_report_section_objcode_schema(
    const std::string& studio_host_path);
void test_studio_host_json_suppresses_unresolved_report_section_memo_placeholders(
    const std::string& studio_host_path);
void test_studio_host_json_preserves_report_sections_without_expr_schema(
    const std::string& studio_host_path);

// ==== Report layout JSON tests: section height/top geometry ====
void test_studio_host_json_updates_report_section_heights_by_record_selection(const std::string& studio_host_path);
void test_studio_host_json_updates_deleted_report_section_heights_by_record_selection(
    const std::string& studio_host_path);
void test_studio_host_json_clears_deleted_report_section_heights_by_record_selection(
    const std::string& studio_host_path);
void test_studio_host_json_clears_report_section_heights_by_record_selection(const std::string& studio_host_path);
void test_studio_host_json_updates_report_section_tops_by_record_selection(const std::string& studio_host_path);
void test_studio_host_json_updates_deleted_report_section_tops_by_record_selection(
    const std::string& studio_host_path);
void test_studio_host_json_clears_deleted_report_section_tops_by_record_selection(
    const std::string& studio_host_path);
void test_studio_host_json_clears_report_section_tops_by_record_selection(const std::string& studio_host_path);
void test_studio_host_json_preserves_realistic_zero_top_section_object_membership_on_top_update(
    const std::string& studio_host_path);
void test_studio_host_json_preserves_tall_object_membership_on_section_top_update(
    const std::string& studio_host_path);
void test_studio_host_json_updates_report_section_heights_and_tops_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_clears_report_section_heights_and_tops_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_updates_deleted_report_section_heights_and_tops_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_clears_deleted_report_section_heights_and_tops_by_stable_selection(
    const std::string& studio_host_path);

// ==== Report layout JSON tests: moving objects between sections and unplaced ====
void test_studio_host_json_moves_report_layout_objects_from_unplaced_to_sections_by_record_selection(
    const std::string& studio_host_path);
void test_studio_host_json_moves_report_layout_objects_from_unplaced_to_sections_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_moves_report_layout_objects_from_sections_to_unplaced_by_record_selection(
    const std::string& studio_host_path);
void test_studio_host_json_moves_report_layout_objects_from_sections_to_unplaced_by_stable_selection(
    const std::string& studio_host_path);

// ==== Report layout JSON tests: object schema and deleted-state diagnostics ====
void write_synthetic_report_table_for_extended_object_kind_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_missing_object_objcode_layout_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_unresolved_deleted_object_memo_layout_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_unresolved_unplaced_object_memo_layout_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_missing_object_expr_layout_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_missing_object_title_layout_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_stable_summary_object_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_deleted_summary_object_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_ambiguous_summary_object_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_live_deleted_ambiguous_summary_object_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_padded_stable_object_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_deep_stable_object_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_deep_ambiguous_stable_object_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_deep_live_deleted_ambiguous_stable_object_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_stable_group_header_object_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_deleted_group_header_object_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_stable_group_footer_object_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_deleted_group_footer_object_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_stable_title_object_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_deleted_title_object_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_stable_page_footer_object_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_deleted_page_footer_object_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_stable_column_header_object_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_deleted_column_header_object_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_stable_column_footer_object_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_deleted_column_footer_object_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_layout_subtree_deleted_state_json(
    const std::filesystem::path& report_path);
void test_studio_host_json_exposes_report_layout_provenance(const std::string& studio_host_path);
void test_studio_host_json_exposes_extended_report_object_kinds(
    const std::string& studio_host_path);
void test_studio_host_json_applies_report_deleted_states_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_applies_report_object_deleted_states_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_applies_report_object_subtree_deleted_state_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_applies_mixed_report_deleted_states_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_defaults_missing_report_object_objcode_schema(
    const std::string& studio_host_path);
void test_studio_host_json_suppresses_unresolved_deleted_report_object_memo_placeholders(
    const std::string& studio_host_path);
void test_studio_host_json_suppresses_unresolved_unplaced_report_object_memo_placeholders(
    const std::string& studio_host_path);
void test_studio_host_json_preserves_report_objects_without_expr_schema(
    const std::string& studio_host_path);
void test_studio_host_json_synthesizes_report_object_titles_without_title_schema(
    const std::string& studio_host_path);

// ==== Report layout JSON tests: detail/header/footer object layout actions ====
void test_studio_host_json_aligns_detail_header_footer_objects_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_aligns_deleted_detail_header_footer_objects_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_resizes_detail_header_footer_objects_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_resizes_deleted_detail_header_footer_objects_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_snaps_detail_header_footer_objects_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_snaps_deleted_detail_header_footer_objects_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_nudges_detail_header_footer_objects_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_nudges_deleted_detail_header_footer_objects_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_distributes_detail_header_footer_objects_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_distributes_deleted_detail_header_footer_objects_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_distributes_detail_header_footer_objects_vertically_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_distributes_deleted_detail_header_footer_objects_vertically_by_stable_selection(
    const std::string& studio_host_path);

// ==== Report layout JSON tests: detail/header/footer object lifecycle ====
void test_studio_host_json_deletes_and_restores_detail_header_footer_objects_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_duplicates_detail_header_footer_objects_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_duplicates_deleted_detail_header_footer_objects_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_renames_detail_header_footer_objects_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_renames_deleted_detail_header_footer_objects_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_reorders_detail_header_footer_objects_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_reorders_deleted_detail_header_footer_objects_by_stable_selection(
    const std::string& studio_host_path);

// ==== Report layout JSON tests: detail/header/footer object expressions and exposure ====
void write_synthetic_report_table_for_detail_header_footer_object_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_detail_header_footer_object_distribution_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_detail_header_footer_object_vertical_distribution_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_detail_header_footer_object_font_json(
    const std::filesystem::path& report_path);
void test_studio_host_json_exposes_detail_header_footer_object_containment(
    const std::string& studio_host_path);
void test_studio_host_json_exposes_detail_header_footer_object_expressions_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_updates_detail_header_footer_object_expressions_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_exposes_deleted_detail_header_footer_object_expressions_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_updates_deleted_detail_header_footer_object_expressions_by_stable_selection(
    const std::string& studio_host_path);

// ==== Report layout JSON tests: report-layout object actions and expressions ====
void test_studio_host_json_nudges_report_layout_objects_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_aligns_report_layout_objects_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_resizes_report_layout_objects_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_snaps_report_layout_objects_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_deletes_report_layout_objects_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_restores_report_layout_objects_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_distributes_report_layout_objects_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_reorders_report_layout_objects_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_duplicates_report_layout_objects_by_stable_selectors(const std::string& studio_host_path);
void test_studio_host_json_renames_report_layout_object_identity_by_stable_selectors(
    const std::string& studio_host_path);
void test_studio_host_json_updates_report_layout_object_expressions_by_record_selection(
    const std::string& studio_host_path);
void test_studio_host_json_updates_report_layout_object_expression_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_updates_deleted_report_layout_object_expressions_by_record_selection(
    const std::string& studio_host_path);
void test_studio_host_json_clears_report_layout_object_expressions_by_record_selection(
    const std::string& studio_host_path);
void test_studio_host_json_clears_report_layout_object_expression_by_stable_selection(
    const std::string& studio_host_path);
void test_studio_host_json_clears_deleted_report_layout_object_expressions_by_record_selection(
    const std::string& studio_host_path);
void test_studio_host_json_restores_edited_deleted_report_layout_object_as_unplaced_by_record_selection(
    const std::string& studio_host_path);
void test_studio_host_json_restores_edited_deleted_report_layout_object_as_unplaced_by_stable_selection(
    const std::string& studio_host_path);

// ==== Report layout JSON tests: selector resolution and ambiguity ====
// ==== Report layout JSON tests: memo placeholder resolution ====
void write_synthetic_report_table_for_unresolved_memo_placeholder_layout_json(
    const std::filesystem::path& report_path);
void test_studio_host_json_suppresses_unresolved_report_memo_placeholders(
    const std::string& studio_host_path);

// ==== Report layout JSON tests: shared fixtures and generic numeric/layout checks ====
void write_synthetic_report_table_for_layout_json(const std::filesystem::path& report_path);
void write_synthetic_report_table_for_zero_top_section_reflow_json(const std::filesystem::path& report_path);
void write_synthetic_report_table_for_stable_deleted_layout_json(const std::filesystem::path& report_path);
void write_synthetic_report_table_for_malformed_numeric_layout_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_oversized_numeric_layout_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_fractional_layout_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_missing_root_objcode_layout_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_printer_identity_layout_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_invalid_first_duplicate_setting_layout_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_missing_root_expr_layout_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_without_unique_id_json(
    const std::filesystem::path& report_path);
void write_synthetic_report_table_for_layout_distribution_json(const std::filesystem::path& report_path);
void write_synthetic_report_table_for_layout_reorder_json(const std::filesystem::path& report_path);
void test_studio_host_json_defaults_malformed_report_layout_numerics(
    const std::string& studio_host_path);
void test_studio_host_json_defaults_oversized_report_layout_numerics(
    const std::string& studio_host_path);

// ==== Visual asset editor JSON exposure tests: methods ====
void test_studio_host_json_exposes_visual_method_list(const std::string& studio_host_path);
void test_studio_host_json_exposes_visual_method_query(const std::string& studio_host_path);
void test_studio_host_json_updates_visual_methods(const std::string& studio_host_path);
void test_studio_host_json_deletes_visual_methods(const std::string& studio_host_path);
void test_studio_host_json_deletes_visual_method_batches(const std::string& studio_host_path);
void test_studio_host_json_renames_visual_method_batches(const std::string& studio_host_path);
void test_studio_host_json_renames_visual_methods(const std::string& studio_host_path);
void test_studio_host_json_copies_visual_methods(const std::string& studio_host_path);
void test_studio_host_json_copies_visual_method_batches(const std::string& studio_host_path);
void test_studio_host_json_moves_visual_method_batches(const std::string& studio_host_path);
void test_studio_host_json_moves_visual_methods(const std::string& studio_host_path);
void test_studio_host_json_reorders_visual_methods(const std::string& studio_host_path);
void test_studio_host_json_reorders_visual_method_batches(const std::string& studio_host_path);

// ==== Visual asset editor JSON exposure tests: objects ====
void test_studio_host_json_exposes_visual_object_list(const std::string& studio_host_path);
void test_studio_host_json_selects_form_and_class_objects_on_open(const std::string& studio_host_path);
void test_studio_host_json_exposes_visual_object_children(const std::string& studio_host_path);
void test_studio_host_json_exposes_visual_object_descendants(const std::string& studio_host_path);
void test_studio_host_json_exposes_visual_object_ancestors(const std::string& studio_host_path);
void test_studio_host_json_updates_visual_object_batches(const std::string& studio_host_path);
void test_studio_host_json_duplicates_visual_object_batches(const std::string& studio_host_path);
void test_studio_host_json_renames_visual_object_batches(const std::string& studio_host_path);
void test_studio_host_json_reparents_visual_object_batches(const std::string& studio_host_path);
void test_studio_host_json_reorders_visual_object_batches(const std::string& studio_host_path);

// ==== Visual asset editor JSON exposure tests: properties ====
void test_studio_host_json_exposes_visual_property_filter(const std::string& studio_host_path);
void test_studio_host_json_exposes_visual_property_query(const std::string& studio_host_path);
void test_studio_host_json_clears_visual_properties(const std::string& studio_host_path);
void test_studio_host_json_clears_visual_property_batches(const std::string& studio_host_path);
void test_studio_host_json_copies_visual_properties(const std::string& studio_host_path);
void test_studio_host_json_copies_visual_property_batches(const std::string& studio_host_path);
void test_studio_host_json_moves_visual_properties(const std::string& studio_host_path);
void test_studio_host_json_moves_visual_property_batches(const std::string& studio_host_path);
void test_studio_host_json_renames_visual_properties(const std::string& studio_host_path);
void test_studio_host_json_renames_visual_property_batches(const std::string& studio_host_path);
void test_studio_host_json_reorders_visual_properties(const std::string& studio_host_path);
void test_studio_host_json_reorders_visual_property_batches(const std::string& studio_host_path);
void test_studio_host_json_exposes_visual_property_list(const std::string& studio_host_path);
void test_studio_host_json_updates_visual_property_batches(const std::string& studio_host_path);
void test_studio_host_json_clears_properties_by_stable_selectors(const std::string& studio_host_path);

}  // namespace cf_test_studio_host_json

#endif
