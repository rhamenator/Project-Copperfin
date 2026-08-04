// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_studio_host_json_support.h"

#include <string_view>

using namespace cf_test_studio_host_json;

namespace {

using StudioHostJsonTestFunction = void (*)(const std::string&);

class StudioHostJsonTestRunner {
public:
    StudioHostJsonTestRunner(int argc, char** argv) {
        parse_arguments(argc, argv);
    }

    void run(const char* test_name, StudioHostJsonTestFunction test_function) {
        if (!ready_) {
            return;
        }
        if (!matches_filters(test_name)) {
            return;
        }

        matched_any_test_ = true;
        if (list_only_) {
            std::cout << test_name << '\n';
            return;
        }

        test_function(studio_host_path_);
    }

    int finish() const {
        if (!ready_) {
            return exit_code_;
        }
        if (!matched_any_test_) {
            std::cerr << "no tests matched the requested selection\n";
            return 3;
        }
        if (list_only_) {
            return 0;
        }
        return failures == 0 ? 0 : 1;
    }

private:
    static void print_usage() {
        std::cerr
            << "usage: test_studio_host_json [--list-tests] [--filter <substring>] [--exact <name>] <copperfin_studio_host>\n"
            << "       test_studio_host_json --list-tests [--filter <substring>] [--exact <name>]\n";
    }

    void fail_usage(const std::string& message, int code = 2) {
        std::cerr << message << '\n';
        print_usage();
        ready_ = false;
        exit_code_ = code;
    }

    void parse_arguments(int argc, char** argv) {
        for (int index = 1; index < argc; ++index) {
            const std::string_view argument = argv[index];
            if (argument == "--help") {
                print_usage();
                ready_ = false;
                exit_code_ = 0;
                return;
            }
            if (argument == "--list-tests") {
                list_only_ = true;
                continue;
            }
            if (argument == "--filter") {
                if (index + 1 >= argc) {
                    fail_usage("missing value for --filter");
                    return;
                }
                substring_filter_ = argv[++index];
                continue;
            }
            if (argument == "--exact") {
                if (index + 1 >= argc) {
                    fail_usage("missing value for --exact");
                    return;
                }
                exact_filter_ = argv[++index];
                continue;
            }
            if (!argument.empty() && argument.front() == '-') {
                fail_usage("unknown option: " + std::string(argument));
                return;
            }
            if (!studio_host_path_.empty()) {
                fail_usage("too many positional arguments");
                return;
            }
            studio_host_path_ = std::string(argument);
        }

        if (!list_only_ && studio_host_path_.empty()) {
            fail_usage("missing required <copperfin_studio_host> path");
        }
    }

    bool matches_filters(std::string_view test_name) const {
        if (!exact_filter_.empty() && test_name != exact_filter_) {
            return false;
        }
        if (!substring_filter_.empty() &&
            test_name.find(substring_filter_) == std::string_view::npos) {
            return false;
        }
        return true;
    }

    bool ready_ = true;
    int exit_code_ = 0;
    bool list_only_ = false;
    bool matched_any_test_ = false;
    std::string studio_host_path_;
    std::string substring_filter_;
    std::string exact_filter_;
};

}  // namespace

#define RUN_STUDIO_HOST_JSON_TEST(name) runner.run(#name, name)

int main(int argc, char** argv) {
    StudioHostJsonTestRunner runner(argc, argv);

    RUN_STUDIO_HOST_JSON_TEST(test_captured_process_output_line_endings_normalize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_exposes_rushmore_explain_plan);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_usage_exposes_selected_execution_catalogs);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_list_subsystems_localizes_descriptor_text);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_builder_parse_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_editor_action_parse_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_toolbox_palette_parse_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_toolbox_parse_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_toolbox_create_plan_parse_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_toolbox_dispatch_create_parse_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_toolbox_batch_dispatch_parse_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_toolbox_batch_dispatch_plan_parse_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_selection_toolbox_create_plan_parse_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_selection_toolbox_batch_create_parse_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_selection_toolbox_batch_dispatch_parse_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_toolbox_batch_dispatch_direct_parse_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_toolbox_batch_create_direct_parse_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_toolbox_batch_plan_catalog_parse_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_toolbox_plan_catalog_parse_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_toolbox_dispatch_catalog_parse_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_toolbox_batch_dispatch_catalog_parse_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_toolbox_direct_plan_parse_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_toolbox_direct_create_parse_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_designer_parse_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_launch_object_metadata_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_launch_layout_action_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_launch_tab_visibility_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_launch_basic_visual_property_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_launch_ole_icon_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_launch_drawing_data_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_launch_grid_layout_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_launch_list_grid_property_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_launch_text_binding_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_launch_row_list_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_launch_deleted_state_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_launch_bound_list_numeric_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_launch_list_item_color_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_launch_general_color_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_launch_window_flag_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_launch_form_bounds_style_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_launch_form_appearance_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_launch_dynamic_expression_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_launch_font_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_launch_max_auto_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_launch_selection_marker_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_launch_sizing_zorder_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_launch_command_mode_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_launch_core_value_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_launch_layout_state_value_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_launch_text_media_list_value_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_launch_list_scalar_value_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_launch_drag_ole_value_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_launch_drawing_buffer_value_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_launch_grid_value_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_launch_record_list_value_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_launch_display_dynamic_color_value_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_launch_window_flag_value_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_launch_bounds_border_value_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_launch_form_appearance_value_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_launch_dynamic_expression_value_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_launch_font_value_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_launch_max_auto_selection_value_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_launch_marker_sizing_zorder_value_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_launch_color_value_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_launch_layout_target_selector_value_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_launch_state_target_selector_value_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_launch_media_target_selector_value_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_launch_mouse_drag_target_selector_value_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_launch_ole_drop_drawing_target_selector_value_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_launch_drawing_buffer_target_selector_value_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_launch_grid_target_selector_value_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_launch_partition_list_target_selector_value_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_launch_record_source_text_target_selector_value_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_launch_row_source_list_target_selector_value_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_launch_row_source_type_list_index_target_selector_value_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_launch_deleted_state_value_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_launch_color_target_selector_value_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_launch_form_boolean_target_selector_value_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_launch_dimension_border_target_selector_value_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_launch_window_picture_target_selector_value_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_launch_dynamic_target_selector_value_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_launch_font_target_selector_value_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_launch_max_selection_target_selector_value_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_launch_marker_sizing_zorder_target_selector_value_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_visual_property_core_parse_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_visual_property_copy_move_parse_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_visual_property_rename_reorder_parse_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_visual_list_navigation_parse_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_visual_object_reparent_duplicate_parse_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_visual_object_rename_reorder_update_parse_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_visual_method_core_parse_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_visual_method_delete_rename_parse_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_visual_method_copy_move_parse_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_visual_method_reorder_parse_diagnostics_localize);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_exposes_designer_contexts);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_preserves_sidecar_path_spelling);
#if !defined(COPPERFIN_DETAIL_HEADER_FOOTER_SECTION_SKIP_HOST_SMOKE)
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_exposes_detail_header_footer_section_kinds);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_updates_detail_header_footer_section_heights_by_stable_selection);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_clears_detail_header_footer_section_heights_by_stable_selection);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_updates_deleted_detail_header_footer_section_heights_by_stable_selection);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_clears_deleted_detail_header_footer_section_heights_by_stable_selection);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_updates_detail_header_footer_section_tops_by_stable_selection);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_clears_detail_header_footer_section_tops_by_stable_selection);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_updates_deleted_detail_header_footer_section_tops_by_stable_selection);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_clears_deleted_detail_header_footer_section_tops_by_stable_selection);
#endif
#if !defined(COPPERFIN_DETAIL_HEADER_FOOTER_SECTION_LIFECYCLE_SKIP_HOST_SMOKE)
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_deletes_and_restores_detail_header_footer_sections_by_stable_selection);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_duplicates_detail_header_footer_sections_by_stable_selection);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_duplicates_deleted_detail_header_footer_sections_by_stable_selection);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_renames_detail_header_footer_sections_by_stable_selection);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_renames_deleted_detail_header_footer_sections_by_stable_selection);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_reorders_detail_header_footer_sections_by_stable_selection);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_reorders_deleted_detail_header_footer_sections_by_stable_selection);
#endif
#if !defined(COPPERFIN_DETAIL_HEADER_FOOTER_OBJECT_EXPRESSIONS_SKIP_HOST_SMOKE)
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_exposes_detail_header_footer_object_expressions_by_stable_selection);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_exposes_detail_header_footer_object_containment);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_updates_detail_header_footer_object_expressions_by_stable_selection);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_exposes_deleted_detail_header_footer_object_expressions_by_stable_selection);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_updates_deleted_detail_header_footer_object_expressions_by_stable_selection);
#endif
#if !defined(COPPERFIN_DETAIL_HEADER_FOOTER_OBJECT_LIFECYCLE_SKIP_HOST_SMOKE)
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_deletes_and_restores_detail_header_footer_objects_by_stable_selection);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_duplicates_detail_header_footer_objects_by_stable_selection);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_duplicates_deleted_detail_header_footer_objects_by_stable_selection);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_renames_detail_header_footer_objects_by_stable_selection);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_renames_deleted_detail_header_footer_objects_by_stable_selection);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_reorders_detail_header_footer_objects_by_stable_selection);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_reorders_deleted_detail_header_footer_objects_by_stable_selection);
#endif
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_defaults_malformed_report_layout_numerics);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_defaults_oversized_report_layout_numerics);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_exposes_builder_launch_plans);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_exposes_builder_launch_catalog);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_exposes_selection_builder_launch_catalog);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_exposes_builder_invocation_admission);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_exposes_builder_invocation_admission_catalog);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_exposes_selection_builder_invocation_admission_catalog);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_exposes_selection_builder_dispatch_catalog);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_exposes_selection_builder_dispatch_execution_catalog);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_exposes_builder_dispatch);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_exposes_builder_execution);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_exposes_builder_dispatch_catalog);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_exposes_builder_dispatch_execution_catalog);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_exposes_editor_action_launch_plans);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_exposes_editor_action_launch_catalog);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_exposes_editor_action_invocation_admission);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_exposes_editor_action_invocation_admission_catalog);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_exposes_editor_action_dispatch);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_exposes_editor_action_execution);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_exposes_editor_action_dispatch_catalog);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_exposes_editor_action_dispatch_execution_catalog);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_exposes_toolbox_palette_launch_plans);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_exposes_toolbox_palette_launch_catalog);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_exposes_toolbox_palette_query_filters);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_exposes_visual_property_filter);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_exposes_visual_property_query);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_clears_visual_properties);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_clears_visual_property_batches);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_copies_visual_properties);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_copies_visual_property_batches);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_moves_visual_properties);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_moves_visual_property_batches);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_renames_visual_properties);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_renames_visual_property_batches);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_reorders_visual_properties);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_reorders_visual_property_batches);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_exposes_visual_property_list);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_exposes_visual_method_list);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_exposes_visual_method_query);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_updates_visual_methods);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_deletes_visual_methods);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_deletes_visual_method_batches);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_renames_visual_method_batches);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_renames_visual_methods);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_copies_visual_methods);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_copies_visual_method_batches);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_moves_visual_methods);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_moves_visual_method_batches);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_reorders_visual_methods);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_reorders_visual_method_batches);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_exposes_visual_object_list);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_selects_form_and_class_objects_on_open);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_exposes_visual_object_children);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_exposes_visual_object_descendants);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_exposes_visual_object_ancestors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_exposes_toolbox_invocation_admission);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_exposes_toolbox_invocation_admission_catalog);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_exposes_selection_toolbox_invocation_admission_catalog);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_exposes_toolbox_dispatch);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_exposes_toolbox_execution);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_exposes_toolbox_dispatch_catalog);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_exposes_toolbox_dispatch_execution_catalog);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_exposes_selection_toolbox_dispatch_catalog);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_exposes_selection_toolbox_dispatch_execution_catalog);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_exposes_designer_launch_surfaces);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_exposes_designer_invocation_admission);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_exposes_designer_dispatch);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_exposes_designer_execution);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_exposes_designer_launch_surface_catalog);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_exposes_designer_invocation_admission_catalog);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_exposes_designer_dispatch_catalog);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_exposes_designer_dispatch_execution_catalog);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_plans_toolbox_object_creation);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_plans_selection_toolbox_object_creation);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_plans_toolbox_object_creation_dispatch);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_plans_selection_toolbox_object_creation_dispatch);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_plans_toolbox_object_creation_from_palette_dispatch);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_creates_toolbox_object_from_palette_dispatch);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_plans_toolbox_object_creation_dispatches_from_palette_dispatch);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_plans_toolbox_object_creation_batches_from_palette_dispatch);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_creates_toolbox_object_batches_from_palette_dispatch);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_plans_toolbox_object_creation_batch_dispatches_from_palette_dispatch);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_plans_toolbox_object_creation_catalog);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_plans_selection_toolbox_object_creation_catalog);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_plans_toolbox_object_creation_dispatch_catalog);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_plans_selection_toolbox_object_creation_dispatch_catalog);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_plans_toolbox_object_creation_batch_plan_catalog);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_plans_selection_toolbox_object_creation_batch_plan_catalog);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_plans_toolbox_object_creation_batch_dispatch_catalog);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_plans_selection_toolbox_object_creation_batch_dispatch_catalog);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_plans_toolbox_object_creation_batches);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_plans_selection_toolbox_object_creation_batches);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_plans_toolbox_object_creation_batch_dispatch);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_plans_selection_toolbox_object_creation_batch_dispatch);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_updates_visual_property_batches);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_updates_visual_object_batches);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_clears_properties_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_duplicates_visual_object_batches);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_renames_visual_object_batches);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_reparents_visual_object_batches);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_reorders_visual_object_batches);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_tab_order_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_tab_stop_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_visibility_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_enabled_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_read_only_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_locked_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_caption_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_picture_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_down_picture_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_disabled_picture_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_ole_drag_picture_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_mouse_icon_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_drag_icon_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_drag_mode_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_ole_drag_mode_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_ole_drop_mode_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_ole_drop_effects_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_ole_drop_text_insertion_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_button_count_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_curvature_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_draw_mode_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_draw_style_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_draw_width_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_fill_style_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_scale_mode_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_buffer_mode_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_buffer_mode_override_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_data_session_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_grid_line_color_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_header_height_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_row_height_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_lock_columns_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_lock_columns_left_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_grid_line_width_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_grid_lines_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_highlight_row_line_width_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_partition_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_record_source_type_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_column_order_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_highlight_style_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_child_order_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_fill_color_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_list_item_id_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_tab_orientation_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_display_orientation_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_help_context_id_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_whats_this_help_id_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_whats_this_help_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_whats_this_button_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_record_source_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_form_set_class_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_default_file_path_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_initial_selected_alias_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_tooltip_text_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_status_bar_text_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_link_master_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_control_source_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_current_control_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_input_mask_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_format_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_row_source_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_column_widths_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_column_lines_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_integral_height_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_incremental_search_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_multi_select_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_row_source_type_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_bound_column_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_column_count_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_style_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_list_index_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_left_column_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_display_value_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_selected_back_color_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_selected_fore_color_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_selected_item_back_color_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_selected_item_fore_color_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_disabled_item_back_color_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_disabled_item_fore_color_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_item_back_color_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_item_fore_color_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_highlight_back_color_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_highlight_fore_color_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_back_color_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_fore_color_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_disabled_back_color_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_disabled_fore_color_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_dynamic_back_color_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_dynamic_fore_color_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_closable_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_control_box_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_allow_output_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_bind_controls_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_auto_verb_menu_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_desktop_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_key_preview_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_mac_desktop_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_max_button_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_min_button_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_min_height_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_min_width_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_max_height_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_movable_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_half_height_caption_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_mdi_form_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_back_style_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_border_style_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_border_width_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_border_color_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_special_effect_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_scroll_bars_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_window_state_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_show_window_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_title_bar_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_mouse_pointer_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_picture_margin_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_picture_position_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_picture_spacing_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_picture_selection_display_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_dynamic_input_mask_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_dynamic_line_height_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_dynamic_current_control_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_dynamic_font_name_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_dynamic_font_size_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_dynamic_font_bold_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_dynamic_font_italic_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_dynamic_font_underline_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_dynamic_font_strikethru_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_dynamic_font_outline_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_dynamic_font_shadow_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_font_name_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_font_size_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_font_bold_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_font_italic_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_font_underline_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_font_strikethru_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_font_outline_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_font_shadow_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_max_width_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_max_left_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_max_top_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_auto_center_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_auto_size_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_auto_release_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_continuous_scroll_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_dockable_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_clip_controls_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_sparse_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_lock_screen_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_hide_selection_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_allow_cell_selection_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_record_mark_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_split_bar_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_highlight_row_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_panel_link_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_allow_header_sizing_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_allow_row_sizing_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_resizable_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_add_line_feeds_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_always_on_top_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_json_assigns_always_on_bottom_by_stable_selectors);
    RUN_STUDIO_HOST_JSON_TEST(test_studio_host_execution_fallback_errors_localize);
    return runner.finish();
}
