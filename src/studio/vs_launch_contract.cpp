// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/studio/vs_launch_contract.h"
#include "vs_launch_contract_internal.h"

#include "copperfin/localization/localization.h"

#include <algorithm>
#include <cerrno>
#include <charconv>
#include <cctype>
#include <cstdlib>
#include <cmath>
#include <optional>
#include <string_view>

namespace copperfin::studio {

LaunchParseResult parse_launch_arguments(
    const std::vector<std::string>& args,
    const localization::LocalizedCatalog& catalog) {
    LaunchParseResult result;

    for (std::size_t index = 0; index < args.size(); ++index) {
        const std::string& argument = args[index];
        std::string parsed_argument_error;

        {
            const auto outcome = try_parse_diagnostics_and_session(argument, catalog, args, index, result, parsed_argument_error);
            if (outcome.handled) {
                if (outcome.should_return) {
                    return result;
                }
                continue;
            }
        }
        {
            const auto outcome = try_parse_selection_context(argument, catalog, args, index, result, parsed_argument_error);
            if (outcome.handled) {
                if (outcome.should_return) {
                    return result;
                }
                continue;
            }
        }
        {
            const auto outcome = try_parse_property_commands(argument, catalog, args, index, result, parsed_argument_error);
            if (outcome.handled) {
                if (outcome.should_return) {
                    return result;
                }
                continue;
            }
        }
        {
            const auto outcome = try_parse_object_lifecycle(argument, catalog, args, index, result, parsed_argument_error);
            if (outcome.handled) {
                if (outcome.should_return) {
                    return result;
                }
                continue;
            }
        }
        {
            const auto outcome = try_parse_layout_actions(argument, catalog, args, index, result, parsed_argument_error);
            if (outcome.handled) {
                if (outcome.should_return) {
                    return result;
                }
                continue;
            }
        }
        {
            const auto outcome = try_parse_setters_behavior(argument, catalog, args, index, result, parsed_argument_error);
            if (outcome.handled) {
                if (outcome.should_return) {
                    return result;
                }
                continue;
            }
        }
        {
            const auto outcome = try_parse_setters_appearance(argument, catalog, args, index, result, parsed_argument_error);
            if (outcome.handled) {
                if (outcome.should_return) {
                    return result;
                }
                continue;
            }
        }
        {
            const auto outcome = try_parse_setters_data(argument, catalog, args, index, result, parsed_argument_error);
            if (outcome.handled) {
                if (outcome.should_return) {
                    return result;
                }
                continue;
            }
        }
        {
            const auto outcome = try_parse_positional_and_fallback(argument, catalog, args, index, result, parsed_argument_error);
            if (outcome.handled) {
                if (outcome.should_return) {
                    return result;
                }
                continue;
            }
        }

        return {.ok = false, .error = localized_unexpected_extra_positional_argument(catalog, argument)};
    }

    if (auto launch_error_01 = validate_diagnostics_and_session(result, catalog)) {
        return *launch_error_01;
    }
    if (auto launch_error_02 = validate_property_commands(result, catalog)) {
        return *launch_error_02;
    }
    if (auto launch_error_03 = validate_object_lifecycle(result, catalog)) {
        return *launch_error_03;
    }
    if (auto launch_error_04 = validate_layout_actions(result, catalog)) {
        return *launch_error_04;
    }
    if (auto launch_error_05 = validate_setters_behavior(result, catalog)) {
        return *launch_error_05;
    }
    if (auto launch_error_06 = validate_setters_appearance(result, catalog)) {
        return *launch_error_06;
    }
    if (auto launch_error_07 = validate_setters_data(result, catalog)) {
        return *launch_error_07;
    }

    const int property_command_count =
        (result.request.apply_property_update ? 1 : 0) +
        (result.request.clear_property ? 1 : 0) +
        (result.request.rename_property ? 1 : 0);
    const int object_command_count =
        (result.request.delete_object ? 1 : 0) +
        (result.request.restore_object ? 1 : 0) +
        (result.request.deleted_states ? 1 : 0) +
        (result.request.subtree_deleted_state ? 1 : 0) +
        (result.request.duplicate_object ? 1 : 0) +
        (result.request.rename_object ? 1 : 0) +
        (result.request.reparent_object ? 1 : 0) +
        (result.request.reorder_object ? 1 : 0) +
        (result.request.group_object ? 1 : 0) +
        (result.request.align_object ? 1 : 0) +
        (result.request.resize_object ? 1 : 0) +
        (result.request.distribute_object ? 1 : 0) +
        (result.request.snap_object ? 1 : 0) +
        (result.request.nudge_object ? 1 : 0) +
        (result.request.tab_order_object ? 1 : 0) +
        (result.request.tab_stop_object ? 1 : 0) +
        (result.request.visibility_object ? 1 : 0) +
        (result.request.enabled_object ? 1 : 0) +
        (result.request.read_only_object ? 1 : 0) +
        (result.request.locked_object ? 1 : 0) +
        (result.request.caption_object ? 1 : 0) +
        (result.request.picture_object ? 1 : 0) +
        (result.request.down_picture_object ? 1 : 0) +
        (result.request.disabled_picture_object ? 1 : 0) +
        (result.request.ole_drag_picture_object ? 1 : 0) +
        (result.request.mouse_icon_object ? 1 : 0) +
        (result.request.drag_icon_object ? 1 : 0) +
        (result.request.drag_mode_object ? 1 : 0) +
        (result.request.ole_drag_mode_object ? 1 : 0) +
        (result.request.ole_drop_mode_object ? 1 : 0) +
        (result.request.ole_drop_effects_object ? 1 : 0) +
        (result.request.ole_drop_text_insertion_object ? 1 : 0) +
        (result.request.tooltip_text_object ? 1 : 0) +
        (result.request.status_bar_text_object ? 1 : 0) +
        (result.request.link_master_object ? 1 : 0) +
        (result.request.control_source_object ? 1 : 0) +
        (result.request.current_control_object ? 1 : 0) +
        (result.request.input_mask_object ? 1 : 0) +
        (result.request.format_object ? 1 : 0) +
        (result.request.row_source_object ? 1 : 0) +
        (result.request.column_widths_object ? 1 : 0) +
        (result.request.column_lines_object ? 1 : 0) +
        (result.request.integral_height_object ? 1 : 0) +
        (result.request.incremental_search_object ? 1 : 0) +
        (result.request.multi_select_object ? 1 : 0) +
        (result.request.row_source_type_object ? 1 : 0) +
        (result.request.bound_column_object ? 1 : 0) +
        (result.request.column_count_object ? 1 : 0) +
        (result.request.style_object ? 1 : 0) +
        (result.request.list_index_object ? 1 : 0) +
        (result.request.left_column_object ? 1 : 0) +
        (result.request.display_value_object ? 1 : 0) +
        (result.request.selected_back_color_object ? 1 : 0) +
        (result.request.selected_fore_color_object ? 1 : 0) +
        (result.request.selected_item_back_color_object ? 1 : 0) +
        (result.request.selected_item_fore_color_object ? 1 : 0) +
        (result.request.disabled_item_back_color_object ? 1 : 0) +
        (result.request.disabled_item_fore_color_object ? 1 : 0) +
        (result.request.item_back_color_object ? 1 : 0) +
        (result.request.item_fore_color_object ? 1 : 0) +
        (result.request.highlight_back_color_object ? 1 : 0) +
        (result.request.highlight_fore_color_object ? 1 : 0) +
        (result.request.back_color_object ? 1 : 0) +
        (result.request.fore_color_object ? 1 : 0) +
        (result.request.disabled_back_color_object ? 1 : 0) +
        (result.request.disabled_fore_color_object ? 1 : 0) +
        (result.request.dynamic_back_color_object ? 1 : 0) +
        (result.request.dynamic_fore_color_object ? 1 : 0) +
        (result.request.closable_object ? 1 : 0) +
        (result.request.control_box_object ? 1 : 0) +
        (result.request.button_count_object ? 1 : 0) +
        (result.request.curvature_object ? 1 : 0) +
        (result.request.draw_mode_object ? 1 : 0) +
        (result.request.draw_style_object ? 1 : 0) +
        (result.request.draw_width_object ? 1 : 0) +
        (result.request.fill_style_object ? 1 : 0) +
        (result.request.scale_mode_object ? 1 : 0) +
        (result.request.buffer_mode_object ? 1 : 0) +
        (result.request.buffer_mode_override_object ? 1 : 0) +
        (result.request.data_session_object ? 1 : 0) +
        (result.request.grid_line_color_object ? 1 : 0) +
        (result.request.header_height_object ? 1 : 0) +
        (result.request.row_height_object ? 1 : 0) +
        (result.request.lock_columns_object ? 1 : 0) +
        (result.request.lock_columns_left_object ? 1 : 0) +
        (result.request.grid_line_width_object ? 1 : 0) +
        (result.request.grid_lines_object ? 1 : 0) +
        (result.request.highlight_row_line_width_object ? 1 : 0) +
        (result.request.partition_object ? 1 : 0) +
        (result.request.record_source_type_object ? 1 : 0) +
        (result.request.column_order_object ? 1 : 0) +
        (result.request.highlight_style_object ? 1 : 0) +
        (result.request.child_order_object ? 1 : 0) +
        (result.request.fill_color_object ? 1 : 0) +
        (result.request.list_item_id_object ? 1 : 0) +
        (result.request.tab_orientation_object ? 1 : 0) +
        (result.request.display_orientation_object ? 1 : 0) +
        (result.request.help_context_id_object ? 1 : 0) +
        (result.request.whats_this_help_id_object ? 1 : 0) +
        (result.request.whats_this_help_object ? 1 : 0) +
        (result.request.whats_this_button_object ? 1 : 0) +
        (result.request.record_source_object ? 1 : 0) +
        (result.request.form_set_class_object ? 1 : 0) +
        (result.request.default_file_path_object ? 1 : 0) +
        (result.request.initial_selected_alias_object ? 1 : 0) +
        (result.request.allow_output_object ? 1 : 0) +
        (result.request.bind_controls_object ? 1 : 0) +
        (result.request.auto_verb_menu_object ? 1 : 0) +
        (result.request.desktop_object ? 1 : 0) +
        (result.request.key_preview_object ? 1 : 0) +
        (result.request.mac_desktop_object ? 1 : 0) +
        (result.request.max_button_object ? 1 : 0) +
        (result.request.min_button_object ? 1 : 0) +
        (result.request.min_height_object ? 1 : 0) +
        (result.request.min_width_object ? 1 : 0) +
        (result.request.max_height_object ? 1 : 0) +
        (result.request.movable_object ? 1 : 0) +
        (result.request.half_height_caption_object ? 1 : 0) +
        (result.request.mdi_form_object ? 1 : 0) +
        (result.request.back_style_object ? 1 : 0) +
        (result.request.border_style_object ? 1 : 0) +
        (result.request.border_width_object ? 1 : 0) +
        (result.request.border_color_object ? 1 : 0) +
        (result.request.special_effect_object ? 1 : 0) +
        (result.request.scroll_bars_object ? 1 : 0) +
        (result.request.window_state_object ? 1 : 0) +
        (result.request.show_window_object ? 1 : 0) +
        (result.request.title_bar_object ? 1 : 0) +
        (result.request.mouse_pointer_object ? 1 : 0) +
        (result.request.picture_margin_object ? 1 : 0) +
        (result.request.picture_position_object ? 1 : 0) +
        (result.request.picture_spacing_object ? 1 : 0) +
        (result.request.picture_selection_display_object ? 1 : 0) +
        (result.request.dynamic_input_mask_object ? 1 : 0) +
        (result.request.dynamic_line_height_object ? 1 : 0) +
        (result.request.dynamic_alignment_object ? 1 : 0) +
        (result.request.dynamic_current_control_object ? 1 : 0) +
        (result.request.dynamic_font_name_object ? 1 : 0) +
        (result.request.dynamic_font_size_object ? 1 : 0) +
        (result.request.dynamic_font_bold_object ? 1 : 0) +
        (result.request.dynamic_font_italic_object ? 1 : 0) +
        (result.request.dynamic_font_underline_object ? 1 : 0) +
        (result.request.dynamic_font_strikethru_object ? 1 : 0) +
        (result.request.dynamic_font_outline_object ? 1 : 0) +
        (result.request.dynamic_font_shadow_object ? 1 : 0) +
        (result.request.font_name_object ? 1 : 0) +
        (result.request.font_size_object ? 1 : 0) +
        (result.request.font_bold_object ? 1 : 0) +
        (result.request.font_italic_object ? 1 : 0) +
        (result.request.font_underline_object ? 1 : 0) +
        (result.request.font_strikethru_object ? 1 : 0) +
        (result.request.font_outline_object ? 1 : 0) +
        (result.request.font_shadow_object ? 1 : 0) +
        (result.request.max_width_object ? 1 : 0) +
        (result.request.max_left_object ? 1 : 0) +
        (result.request.max_top_object ? 1 : 0) +
        (result.request.auto_center_object ? 1 : 0) +
        (result.request.auto_size_object ? 1 : 0) +
        (result.request.auto_release_object ? 1 : 0) +
        (result.request.continuous_scroll_object ? 1 : 0) +
        (result.request.dockable_object ? 1 : 0) +
        (result.request.clip_controls_object ? 1 : 0) +
        (result.request.sparse_object ? 1 : 0) +
        (result.request.lock_screen_object ? 1 : 0) +
        (result.request.allow_cell_selection_object ? 1 : 0) +
        (result.request.hide_selection_object ? 1 : 0) +
        (result.request.delete_mark_object ? 1 : 0) +
        (result.request.record_mark_object ? 1 : 0) +
        (result.request.split_bar_object ? 1 : 0) +
        (result.request.highlight_row_object ? 1 : 0) +
        (result.request.panel_link_object ? 1 : 0) +
        (result.request.allow_header_sizing_object ? 1 : 0) +
        (result.request.allow_row_sizing_object ? 1 : 0) +
        (result.request.resizable_object ? 1 : 0) +
        (result.request.add_line_feeds_object ? 1 : 0) +
        (result.request.always_on_top_object ? 1 : 0) +
        (result.request.always_on_bottom_object ? 1 : 0) +
        (result.request.ungroup_object ? 1 : 0);
    if (property_command_count > 1) {
        return {.ok = false, .error = catalog.translate("StudioHost.LaunchParse.Error.SinglePropertyCommand")};
    }
    if (object_command_count > 1) {
        return {.ok = false, .error = catalog.translate("StudioHost.LaunchParse.Error.SingleObjectCommand")};
    }
    if (object_command_count > 0 && property_command_count > 0) {
        return {.ok = false, .error = catalog.translate("StudioHost.LaunchParse.Error.MixedObjectPropertyCommands")};
    }

    result.mutates_asset =
        property_command_count > 0 ||
        object_command_count > 0 ||
        result.request.undo_mode == StudioUndoMode::command;
    result.ok = true;
    return result;
}

LaunchParseResult parse_launch_arguments(const std::vector<std::string>& args) {
    return parse_launch_arguments(args, default_launch_catalog());
}

}  // namespace copperfin::studio
