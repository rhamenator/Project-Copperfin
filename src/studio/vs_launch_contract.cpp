#include "copperfin/studio/vs_launch_contract.h"

#include <algorithm>
#include <charconv>
#include <cctype>
#include <optional>

namespace copperfin::studio {

namespace {

bool parse_size_value(const std::string& text, std::size_t& value) {
    const char* begin = text.data();
    const char* end = text.data() + text.size();
    const auto result = std::from_chars(begin, end, value);
    return result.ec == std::errc{} && result.ptr == end;
}

bool parse_int_value(const std::string& text, int& value) {
    const char* begin = text.data();
    const char* end = text.data() + text.size();
    const auto result = std::from_chars(begin, end, value);
    return result.ec == std::errc{} && result.ptr == end;
}

bool parse_double_value(const std::string& text, double& value) {
    const char* begin = text.data();
    const char* end = text.data() + text.size();
    const auto result = std::from_chars(begin, end, value);
    return result.ec == std::errc{} && result.ptr == end;
}

std::string lowercase_copy(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return text;
}

std::optional<bool> parse_bool_value(std::string text) {
    text = lowercase_copy(std::move(text));
    if (text == "true" || text == ".t." || text == "t" || text == "1" || text == "yes" || text == "on") {
        return true;
    }
    if (text == "false" || text == ".f." || text == "f" || text == "0" || text == "no" || text == "off") {
        return false;
    }
    return std::nullopt;
}

std::optional<StudioEditorSelectionContext> parse_selection_context_token(std::string token) {
    token = lowercase_copy(std::move(token));
    if (token == studio_editor_selection_context_name(StudioEditorSelectionContext::visual_object)) {
        return StudioEditorSelectionContext::visual_object;
    }
    if (token == studio_editor_selection_context_name(StudioEditorSelectionContext::visual_method)) {
        return StudioEditorSelectionContext::visual_method;
    }
    if (token == studio_editor_selection_context_name(StudioEditorSelectionContext::container_object)) {
        return StudioEditorSelectionContext::container_object;
    }
    if (token == studio_editor_selection_context_name(StudioEditorSelectionContext::class_designer)) {
        return StudioEditorSelectionContext::class_designer;
    }
    if (token == studio_editor_selection_context_name(StudioEditorSelectionContext::report_expression)) {
        return StudioEditorSelectionContext::report_expression;
    }
    if (token == studio_editor_selection_context_name(StudioEditorSelectionContext::label_expression)) {
        return StudioEditorSelectionContext::label_expression;
    }
    if (token == studio_editor_selection_context_name(StudioEditorSelectionContext::menu_item)) {
        return StudioEditorSelectionContext::menu_item;
    }
    if (token == studio_editor_selection_context_name(StudioEditorSelectionContext::project_item)) {
        return StudioEditorSelectionContext::project_item;
    }
    if (token == studio_editor_selection_context_name(StudioEditorSelectionContext::data_environment)) {
        return StudioEditorSelectionContext::data_environment;
    }
    return std::nullopt;
}

std::string selection_context_error() {
    return "The --selection-context value must be visual_object, visual_method, container_object, class_designer, report_expression, label_expression, menu_item, project_item, or data_environment.";
}

}  // namespace

LaunchParseResult parse_launch_arguments(const std::vector<std::string>& args) {
    LaunchParseResult result;

    for (std::size_t index = 0; index < args.size(); ++index) {
        const std::string& argument = args[index];

        if (argument == "--help" || argument == "-h" || argument == "/?") {
            result.ok = true;
            result.show_help = true;
            return result;
        }

        if (argument == "--from-vs") {
            result.request.launched_from_visual_studio = true;
            continue;
        }

        if (argument == "--read-only") {
            result.request.read_only = true;
            continue;
        }

        if (argument == "--set-property") {
            result.request.apply_property_update = true;
            continue;
        }

        if (argument == "--clear-property") {
            result.request.clear_property = true;
            continue;
        }

        if (argument == "--rename-property") {
            result.request.rename_property = true;
            continue;
        }

        if (argument == "--delete-object") {
            result.request.delete_object = true;
            continue;
        }

        if (argument == "--restore-object") {
            result.request.restore_object = true;
            continue;
        }

        if (argument == "--duplicate-object") {
            result.request.duplicate_object = true;
            continue;
        }

        if (argument == "--rename-object") {
            result.request.rename_object = true;
            continue;
        }

        if (argument == "--reparent-object") {
            result.request.reparent_object = true;
            continue;
        }

        if (argument == "--reorder-object") {
            result.request.reorder_object = true;
            continue;
        }

        if (argument == "--group-object") {
            result.request.group_object = true;
            continue;
        }

        if (argument == "--align-object") {
            result.request.align_object = true;
            continue;
        }

        if (argument == "--resize-object") {
            result.request.resize_object = true;
            continue;
        }

        if (argument == "--distribute-object") {
            result.request.distribute_object = true;
            continue;
        }

        if (argument == "--snap-object") {
            result.request.snap_object = true;
            continue;
        }

        if (argument == "--nudge-object") {
            result.request.nudge_object = true;
            continue;
        }

        if (argument == "--tab-order-object") {
            result.request.tab_order_object = true;
            continue;
        }

        if (argument == "--tab-stop-object") {
            result.request.tab_stop_object = true;
            continue;
        }

        if (argument == "--visibility-object") {
            result.request.visibility_object = true;
            continue;
        }

        if (argument == "--enabled-object") {
            result.request.enabled_object = true;
            continue;
        }

        if (argument == "--read-only-object") {
            result.request.read_only_object = true;
            continue;
        }

        if (argument == "--locked-object") {
            result.request.locked_object = true;
            continue;
        }

        if (argument == "--caption-object") {
            result.request.caption_object = true;
            continue;
        }

        if (argument == "--tooltip-text-object") {
            result.request.tooltip_text_object = true;
            continue;
        }

        if (argument == "--status-bar-text-object") {
            result.request.status_bar_text_object = true;
            continue;
        }

        if (argument == "--control-source-object") {
            result.request.control_source_object = true;
            continue;
        }

        if (argument == "--current-control-object") {
            result.request.current_control_object = true;
            continue;
        }

        if (argument == "--input-mask-object") {
            result.request.input_mask_object = true;
            continue;
        }

        if (argument == "--format-object") {
            result.request.format_object = true;
            continue;
        }

        if (argument == "--row-source-object") {
            result.request.row_source_object = true;
            continue;
        }

        if (argument == "--row-source-type-object") {
            result.request.row_source_type_object = true;
            continue;
        }

        if (argument == "--bound-column-object") {
            result.request.bound_column_object = true;
            continue;
        }

        if (argument == "--column-count-object") {
            result.request.column_count_object = true;
            continue;
        }

        if (argument == "--style-object") {
            result.request.style_object = true;
            continue;
        }

        if (argument == "--list-index-object") {
            result.request.list_index_object = true;
            continue;
        }

        if (argument == "--left-column-object") {
            result.request.left_column_object = true;
            continue;
        }

        if (argument == "--display-value-object") {
            result.request.display_value_object = true;
            continue;
        }

        if (argument == "--selected-back-color-object") {
            result.request.selected_back_color_object = true;
            continue;
        }

        if (argument == "--selected-fore-color-object") {
            result.request.selected_fore_color_object = true;
            continue;
        }

        if (argument == "--selected-item-back-color-object") {
            result.request.selected_item_back_color_object = true;
            continue;
        }

        if (argument == "--selected-item-fore-color-object") {
            result.request.selected_item_fore_color_object = true;
            continue;
        }

        if (argument == "--disabled-item-back-color-object") {
            result.request.disabled_item_back_color_object = true;
            continue;
        }

        if (argument == "--disabled-item-fore-color-object") {
            result.request.disabled_item_fore_color_object = true;
            continue;
        }

        if (argument == "--item-back-color-object") {
            result.request.item_back_color_object = true;
            continue;
        }

        if (argument == "--item-fore-color-object") {
            result.request.item_fore_color_object = true;
            continue;
        }

        if (argument == "--highlight-back-color-object") {
            result.request.highlight_back_color_object = true;
            continue;
        }

        if (argument == "--highlight-fore-color-object") {
            result.request.highlight_fore_color_object = true;
            continue;
        }

        if (argument == "--back-color-object") {
            result.request.back_color_object = true;
            continue;
        }

        if (argument == "--fore-color-object") {
            result.request.fore_color_object = true;
            continue;
        }

        if (argument == "--disabled-back-color-object") {
            result.request.disabled_back_color_object = true;
            continue;
        }

        if (argument == "--disabled-fore-color-object") {
            result.request.disabled_fore_color_object = true;
            continue;
        }

        if (argument == "--dynamic-back-color-object") {
            result.request.dynamic_back_color_object = true;
            continue;
        }

        if (argument == "--dynamic-fore-color-object") {
            result.request.dynamic_fore_color_object = true;
            continue;
        }

        if (argument == "--closable-object") {
            result.request.closable_object = true;
            continue;
        }

        if (argument == "--control-box-object") {
            result.request.control_box_object = true;
            continue;
        }

        if (argument == "--allow-output-object") {
            result.request.allow_output_object = true;
            continue;
        }

        if (argument == "--auto-center-object") {
            result.request.auto_center_object = true;
            continue;
        }

        if (argument == "--auto-size-object") {
            result.request.auto_size_object = true;
            continue;
        }

        if (argument == "--auto-release-object") {
            result.request.auto_release_object = true;
            continue;
        }

        if (argument == "--continuous-scroll-object") {
            result.request.continuous_scroll_object = true;
            continue;
        }

        if (argument == "--dockable-object") {
            result.request.dockable_object = true;
            continue;
        }

        if (argument == "--clip-controls-object") {
            result.request.clip_controls_object = true;
            continue;
        }

        if (argument == "--sparse-object") {
            result.request.sparse_object = true;
            continue;
        }

        if (argument == "--lock-screen-object") {
            result.request.lock_screen_object = true;
            continue;
        }

        if (argument == "--hide-selection-object") {
            result.request.hide_selection_object = true;
            continue;
        }

        if (argument == "--allow-cell-selection-object") {
            result.request.allow_cell_selection_object = true;
            continue;
        }

        if (argument == "--delete-mark-object") {
            result.request.delete_mark_object = true;
            continue;
        }

        if (argument == "--record-mark-object") {
            result.request.record_mark_object = true;
            continue;
        }

        if (argument == "--split-bar-object") {
            result.request.split_bar_object = true;
            continue;
        }

        if (argument == "--highlight-row-object") {
            result.request.highlight_row_object = true;
            continue;
        }

        if (argument == "--panel-link-object") {
            result.request.panel_link_object = true;
            continue;
        }

        if (argument == "--allow-header-sizing-object") {
            result.request.allow_header_sizing_object = true;
            continue;
        }

        if (argument == "--allow-row-sizing-object") {
            result.request.allow_row_sizing_object = true;
            continue;
        }

        if (argument == "--resizable-object") {
            result.request.resizable_object = true;
            continue;
        }

        if (argument == "--add-line-feeds-object") {
            result.request.add_line_feeds_object = true;
            continue;
        }

        if (argument == "--always-on-top-object") {
            result.request.always_on_top_object = true;
            continue;
        }

        if (argument == "--always-on-bottom-object") {
            result.request.always_on_bottom_object = true;
            continue;
        }

        if (argument == "--picture-object") {
            result.request.picture_object = true;
            continue;
        }

        if (argument == "--down-picture-object") {
            result.request.down_picture_object = true;
            continue;
        }

        if (argument == "--disabled-picture-object") {
            result.request.disabled_picture_object = true;
            continue;
        }

        if (argument == "--ole-drag-picture-object") {
            result.request.ole_drag_picture_object = true;
            continue;
        }

        if (argument == "--mouse-icon-object") {
            result.request.mouse_icon_object = true;
            continue;
        }

        if (argument == "--drag-icon-object") {
            result.request.drag_icon_object = true;
            continue;
        }

        if (argument == "--drag-mode-object") {
            result.request.drag_mode_object = true;
            continue;
        }

        if (argument == "--ole-drag-mode-object") {
            result.request.ole_drag_mode_object = true;
            continue;
        }

        if (argument == "--ole-drop-mode-object") {
            result.request.ole_drop_mode_object = true;
            continue;
        }

        if (argument == "--ole-drop-effects-object") {
            result.request.ole_drop_effects_object = true;
            continue;
        }

        if (argument == "--ole-drop-text-insertion-object") {
            result.request.ole_drop_text_insertion_object = true;
            continue;
        }

        if (argument == "--button-count-object") {
            result.request.button_count_object = true;
            continue;
        }

        if (argument == "--curvature-object") {
            result.request.curvature_object = true;
            continue;
        }

        if (argument == "--draw-mode-object") {
            result.request.draw_mode_object = true;
            continue;
        }

        if (argument == "--draw-style-object") {
            result.request.draw_style_object = true;
            continue;
        }

        if (argument == "--draw-width-object") {
            result.request.draw_width_object = true;
            continue;
        }

        if (argument == "--fill-style-object") {
            result.request.fill_style_object = true;
            continue;
        }

        if (argument == "--scale-mode-object") {
            result.request.scale_mode_object = true;
            continue;
        }

        if (argument == "--buffer-mode-object") {
            result.request.buffer_mode_object = true;
            continue;
        }

        if (argument == "--buffer-mode-override-object") {
            result.request.buffer_mode_override_object = true;
            continue;
        }

        if (argument == "--data-session-object") {
            result.request.data_session_object = true;
            continue;
        }

        if (argument == "--grid-line-color-object") {
            result.request.grid_line_color_object = true;
            continue;
        }

        if (argument == "--header-height-object") {
            result.request.header_height_object = true;
            continue;
        }

        if (argument == "--row-height-object") {
            result.request.row_height_object = true;
            continue;
        }

        if (argument == "--lock-columns-object") {
            result.request.lock_columns_object = true;
            continue;
        }

        if (argument == "--lock-columns-left-object") {
            result.request.lock_columns_left_object = true;
            continue;
        }

        if (argument == "--grid-line-width-object") {
            result.request.grid_line_width_object = true;
            continue;
        }

        if (argument == "--grid-lines-object") {
            result.request.grid_lines_object = true;
            continue;
        }

        if (argument == "--highlight-row-line-width-object") {
            result.request.highlight_row_line_width_object = true;
            continue;
        }

        if (argument == "--ungroup-object") {
            result.request.ungroup_object = true;
            continue;
        }

        if (argument == "--clear-parent") {
            result.request.clear_parent = true;
            continue;
        }

        if (argument == "--json") {
            result.output_json = true;
            continue;
        }

        if (argument == "--path") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --path."};
            }
            result.request.path = args[++index];
            continue;
        }

        if (argument == "--symbol") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --symbol."};
            }
            result.request.symbol = args[++index];
            continue;
        }

        if (argument == "--selection-context") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --selection-context."};
            }
            const auto selection_context = parse_selection_context_token(args[++index]);
            if (!selection_context.has_value()) {
                return {.ok = false, .error = selection_context_error()};
            }
            result.request.designer_selection_contexts.push_back(*selection_context);
            continue;
        }

        if (argument == "--record") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --record."};
            }
            std::size_t record_index = 0;
            if (!parse_size_value(args[++index], record_index)) {
                return {.ok = false, .error = "The --record value must be an unsigned integer."};
            }
            result.request.record_index = record_index;
            result.request.selection_record_available = true;
            continue;
        }

        if (argument == "--property-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --property-name."};
            }
            result.request.property_name = args[++index];
            continue;
        }

        if (argument == "--property-value") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --property-value."};
            }
            result.request.property_value = args[++index];
            continue;
        }

        if (argument == "--new-property-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --new-property-name."};
            }
            result.request.new_property_name = args[++index];
            continue;
        }

        if (argument == "--new-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --new-object-name."};
            }
            result.request.new_object_name = args[++index];
            continue;
        }

        if (argument == "--new-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --new-name."};
            }
            result.request.new_name = args[++index];
            continue;
        }

        if (argument == "--new-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --new-unique-id."};
            }
            result.request.new_unique_id = args[++index];
            continue;
        }

        if (argument == "--parent-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --parent-name."};
            }
            result.request.parent_name = args[++index];
            continue;
        }

        if (argument == "--parent-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --parent-unique-id."};
            }
            result.request.parent_unique_id = args[++index];
            continue;
        }

        if (argument == "--placement") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --placement."};
            }
            result.request.placement = args[++index];
            continue;
        }

        if (argument == "--target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --target-object-name."};
            }
            result.request.target_object_name = args[++index];
            continue;
        }

        if (argument == "--target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --target-unique-id."};
            }
            result.request.target_unique_id = args[++index];
            continue;
        }

        if (argument == "--group-child-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --group-child-object-name."};
            }
            result.request.group_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--group-child-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --group-child-unique-id."};
            }
            result.request.group_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--field-value") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --field-value."};
            }
            const std::string assignment = args[++index];
            const auto separator = assignment.find('=');
            if (separator == std::string::npos || separator == 0U) {
                return {.ok = false, .error = "Field values must use name=value syntax."};
            }
            result.request.field_values.push_back({
                .property_name = assignment.substr(0U, separator),
                .property_value = assignment.substr(separator + 1U)
            });
            continue;
        }

        if (argument == "--alignment-mode") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --alignment-mode."};
            }
            result.request.alignment_mode = args[++index];
            continue;
        }

        if (argument == "--resize-mode") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --resize-mode."};
            }
            result.request.resize_mode = args[++index];
            continue;
        }

        if (argument == "--distribution-mode") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --distribution-mode."};
            }
            result.request.distribution_mode = args[++index];
            continue;
        }

        if (argument == "--snap-mode") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --snap-mode."};
            }
            result.request.snap_mode = args[++index];
            continue;
        }

        if (argument == "--nudge-mode") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --nudge-mode."};
            }
            result.request.nudge_mode = args[++index];
            continue;
        }

        if (argument == "--grid-width") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --grid-width."};
            }
            double grid_width = 0.0;
            if (!parse_double_value(args[++index], grid_width)) {
                return {.ok = false, .error = "The --grid-width value must be numeric."};
            }
            result.request.grid_width = grid_width;
            continue;
        }

        if (argument == "--grid-height") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --grid-height."};
            }
            double grid_height = 0.0;
            if (!parse_double_value(args[++index], grid_height)) {
                return {.ok = false, .error = "The --grid-height value must be numeric."};
            }
            result.request.grid_height = grid_height;
            continue;
        }

        if (argument == "--delta-hpos") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --delta-hpos."};
            }
            double delta_hpos = 0.0;
            if (!parse_double_value(args[++index], delta_hpos)) {
                return {.ok = false, .error = "The --delta-hpos value must be numeric."};
            }
            result.request.delta_hpos = delta_hpos;
            continue;
        }

        if (argument == "--delta-vpos") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --delta-vpos."};
            }
            double delta_vpos = 0.0;
            if (!parse_double_value(args[++index], delta_vpos)) {
                return {.ok = false, .error = "The --delta-vpos value must be numeric."};
            }
            result.request.delta_vpos = delta_vpos;
            continue;
        }

        if (argument == "--starting-tab-index") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --starting-tab-index."};
            }
            int starting_tab_index = 0;
            if (!parse_int_value(args[++index], starting_tab_index)) {
                return {.ok = false, .error = "The --starting-tab-index value must be an integer."};
            }
            result.request.starting_tab_index = starting_tab_index;
            result.request.starting_tab_index_available = true;
            continue;
        }

        if (argument == "--tab-stop") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --tab-stop."};
            }
            const auto tab_stop = parse_bool_value(args[++index]);
            if (!tab_stop.has_value()) {
                return {.ok = false, .error = "The --tab-stop value must be true or false."};
            }
            result.request.tab_stop = *tab_stop;
            result.request.tab_stop_available = true;
            continue;
        }

        if (argument == "--visible") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --visible."};
            }
            const auto visible = parse_bool_value(args[++index]);
            if (!visible.has_value()) {
                return {.ok = false, .error = "The --visible value must be true or false."};
            }
            result.request.visible = *visible;
            result.request.visible_available = true;
            continue;
        }

        if (argument == "--enabled") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --enabled."};
            }
            const auto enabled = parse_bool_value(args[++index]);
            if (!enabled.has_value()) {
                return {.ok = false, .error = "The --enabled value must be true or false."};
            }
            result.request.enabled = *enabled;
            result.request.enabled_available = true;
            continue;
        }

        if (argument == "--object-read-only") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --object-read-only."};
            }
            const auto object_read_only = parse_bool_value(args[++index]);
            if (!object_read_only.has_value()) {
                return {.ok = false, .error = "The --object-read-only value must be true or false."};
            }
            result.request.object_read_only = *object_read_only;
            result.request.object_read_only_available = true;
            continue;
        }

        if (argument == "--locked") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --locked."};
            }
            const auto locked = parse_bool_value(args[++index]);
            if (!locked.has_value()) {
                return {.ok = false, .error = "The --locked value must be true or false."};
            }
            result.request.locked = *locked;
            result.request.locked_available = true;
            continue;
        }

        if (argument == "--caption") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --caption."};
            }
            result.request.caption = args[++index];
            result.request.caption_available = true;
            continue;
        }

        if (argument == "--picture") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --picture."};
            }
            result.request.picture = args[++index];
            result.request.picture_available = true;
            continue;
        }

        if (argument == "--down-picture") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --down-picture."};
            }
            result.request.down_picture = args[++index];
            result.request.down_picture_available = true;
            continue;
        }

        if (argument == "--disabled-picture") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --disabled-picture."};
            }
            result.request.disabled_picture = args[++index];
            result.request.disabled_picture_available = true;
            continue;
        }

        if (argument == "--ole-drag-picture") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --ole-drag-picture."};
            }
            result.request.ole_drag_picture = args[++index];
            result.request.ole_drag_picture_available = true;
            continue;
        }

        if (argument == "--mouse-icon") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --mouse-icon."};
            }
            result.request.mouse_icon = args[++index];
            result.request.mouse_icon_available = true;
            continue;
        }

        if (argument == "--drag-icon") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --drag-icon."};
            }
            result.request.drag_icon = args[++index];
            result.request.drag_icon_available = true;
            continue;
        }

        if (argument == "--tooltip-text") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --tooltip-text."};
            }
            result.request.tooltip_text = args[++index];
            result.request.tooltip_text_available = true;
            continue;
        }

        if (argument == "--status-bar-text") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --status-bar-text."};
            }
            result.request.status_bar_text = args[++index];
            result.request.status_bar_text_available = true;
            continue;
        }

        if (argument == "--control-source") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --control-source."};
            }
            result.request.control_source = args[++index];
            result.request.control_source_available = true;
            continue;
        }

        if (argument == "--current-control") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --current-control."};
            }
            result.request.current_control = args[++index];
            result.request.current_control_available = true;
            continue;
        }

        if (argument == "--input-mask") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --input-mask."};
            }
            result.request.input_mask = args[++index];
            result.request.input_mask_available = true;
            continue;
        }

        if (argument == "--format") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --format."};
            }
            result.request.format = args[++index];
            result.request.format_available = true;
            continue;
        }

        if (argument == "--row-source") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --row-source."};
            }
            result.request.row_source = args[++index];
            result.request.row_source_available = true;
            continue;
        }

        if (argument == "--row-source-type") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --row-source-type."};
            }
            int row_source_type = 0;
            if (!parse_int_value(args[++index], row_source_type)) {
                return {.ok = false, .error = "The --row-source-type value must be an integer."};
            }
            result.request.row_source_type = row_source_type;
            result.request.row_source_type_available = true;
            continue;
        }

        if (argument == "--bound-column") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --bound-column."};
            }
            int bound_column = 0;
            if (!parse_int_value(args[++index], bound_column)) {
                return {.ok = false, .error = "The --bound-column value must be an integer."};
            }
            result.request.bound_column = bound_column;
            result.request.bound_column_available = true;
            continue;
        }

        if (argument == "--column-count") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --column-count."};
            }
            int column_count = 0;
            if (!parse_int_value(args[++index], column_count)) {
                return {.ok = false, .error = "The --column-count value must be an integer."};
            }
            result.request.column_count = column_count;
            result.request.column_count_available = true;
            continue;
        }

        if (argument == "--style") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --style."};
            }
            int style = 0;
            if (!parse_int_value(args[++index], style)) {
                return {.ok = false, .error = "The --style value must be an integer."};
            }
            result.request.style = style;
            result.request.style_available = true;
            continue;
        }

        if (argument == "--list-index") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --list-index."};
            }
            int list_index = 0;
            if (!parse_int_value(args[++index], list_index)) {
                return {.ok = false, .error = "The --list-index value must be an integer."};
            }
            result.request.list_index = list_index;
            result.request.list_index_available = true;
            continue;
        }

        if (argument == "--left-column") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --left-column."};
            }
            int left_column = 0;
            if (!parse_int_value(args[++index], left_column)) {
                return {.ok = false, .error = "The --left-column value must be an integer."};
            }
            result.request.left_column = left_column;
            result.request.left_column_available = true;
            continue;
        }

        if (argument == "--drag-mode") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --drag-mode."};
            }
            int drag_mode = 0;
            if (!parse_int_value(args[++index], drag_mode)) {
                return {.ok = false, .error = "The --drag-mode value must be an integer."};
            }
            if (drag_mode < 0) {
                return {.ok = false, .error = "The --drag-mode value must be non-negative."};
            }
            result.request.drag_mode = drag_mode;
            result.request.drag_mode_available = true;
            continue;
        }

        if (argument == "--ole-drag-mode") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --ole-drag-mode."};
            }
            int ole_drag_mode = 0;
            if (!parse_int_value(args[++index], ole_drag_mode)) {
                return {.ok = false, .error = "The --ole-drag-mode value must be an integer."};
            }
            if (ole_drag_mode < 0) {
                return {.ok = false, .error = "The --ole-drag-mode value must be non-negative."};
            }
            result.request.ole_drag_mode = ole_drag_mode;
            result.request.ole_drag_mode_available = true;
            continue;
        }

        if (argument == "--ole-drop-mode") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --ole-drop-mode."};
            }
            int ole_drop_mode = 0;
            if (!parse_int_value(args[++index], ole_drop_mode)) {
                return {.ok = false, .error = "The --ole-drop-mode value must be an integer."};
            }
            if (ole_drop_mode < 0) {
                return {.ok = false, .error = "The --ole-drop-mode value must be non-negative."};
            }
            result.request.ole_drop_mode = ole_drop_mode;
            result.request.ole_drop_mode_available = true;
            continue;
        }

        if (argument == "--ole-drop-effects") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --ole-drop-effects."};
            }
            int ole_drop_effects = 0;
            if (!parse_int_value(args[++index], ole_drop_effects)) {
                return {.ok = false, .error = "The --ole-drop-effects value must be an integer."};
            }
            if (ole_drop_effects < 0) {
                return {.ok = false, .error = "The --ole-drop-effects value must be non-negative."};
            }
            result.request.ole_drop_effects = ole_drop_effects;
            result.request.ole_drop_effects_available = true;
            continue;
        }

        if (argument == "--ole-drop-text-insertion") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --ole-drop-text-insertion."};
            }
            int ole_drop_text_insertion = 0;
            if (!parse_int_value(args[++index], ole_drop_text_insertion)) {
                return {.ok = false, .error = "The --ole-drop-text-insertion value must be an integer."};
            }
            if (ole_drop_text_insertion < 0) {
                return {.ok = false, .error = "The --ole-drop-text-insertion value must be non-negative."};
            }
            result.request.ole_drop_text_insertion = ole_drop_text_insertion;
            result.request.ole_drop_text_insertion_available = true;
            continue;
        }

        if (argument == "--button-count") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --button-count."};
            }
            int button_count = 0;
            if (!parse_int_value(args[++index], button_count)) {
                return {.ok = false, .error = "The --button-count value must be an integer."};
            }
            if (button_count < 0) {
                return {.ok = false, .error = "The --button-count value must be non-negative."};
            }
            result.request.button_count = button_count;
            result.request.button_count_available = true;
            continue;
        }

        if (argument == "--curvature") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --curvature."};
            }
            int curvature = 0;
            if (!parse_int_value(args[++index], curvature)) {
                return {.ok = false, .error = "The --curvature value must be an integer."};
            }
            if (curvature < 0) {
                return {.ok = false, .error = "The --curvature value must be non-negative."};
            }
            result.request.curvature = curvature;
            result.request.curvature_available = true;
            continue;
        }

        if (argument == "--draw-mode") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --draw-mode."};
            }
            int draw_mode = 0;
            if (!parse_int_value(args[++index], draw_mode)) {
                return {.ok = false, .error = "The --draw-mode value must be an integer."};
            }
            if (draw_mode < 0) {
                return {.ok = false, .error = "The --draw-mode value must be non-negative."};
            }
            result.request.draw_mode = draw_mode;
            result.request.draw_mode_available = true;
            continue;
        }

        if (argument == "--draw-style") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --draw-style."};
            }
            int draw_style = 0;
            if (!parse_int_value(args[++index], draw_style)) {
                return {.ok = false, .error = "The --draw-style value must be an integer."};
            }
            if (draw_style < 0) {
                return {.ok = false, .error = "The --draw-style value must be non-negative."};
            }
            result.request.draw_style = draw_style;
            result.request.draw_style_available = true;
            continue;
        }

        if (argument == "--draw-width") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --draw-width."};
            }
            int draw_width = 0;
            if (!parse_int_value(args[++index], draw_width)) {
                return {.ok = false, .error = "The --draw-width value must be an integer."};
            }
            if (draw_width < 0) {
                return {.ok = false, .error = "The --draw-width value must be non-negative."};
            }
            result.request.draw_width = draw_width;
            result.request.draw_width_available = true;
            continue;
        }

        if (argument == "--fill-style") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --fill-style."};
            }
            int fill_style = 0;
            if (!parse_int_value(args[++index], fill_style)) {
                return {.ok = false, .error = "The --fill-style value must be an integer."};
            }
            if (fill_style < 0) {
                return {.ok = false, .error = "The --fill-style value must be non-negative."};
            }
            result.request.fill_style = fill_style;
            result.request.fill_style_available = true;
            continue;
        }

        if (argument == "--scale-mode") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --scale-mode."};
            }
            int scale_mode = 0;
            if (!parse_int_value(args[++index], scale_mode)) {
                return {.ok = false, .error = "The --scale-mode value must be an integer."};
            }
            if (scale_mode < 0) {
                return {.ok = false, .error = "The --scale-mode value must be non-negative."};
            }
            result.request.scale_mode = scale_mode;
            result.request.scale_mode_available = true;
            continue;
        }

        if (argument == "--buffer-mode") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --buffer-mode."};
            }
            int buffer_mode = 0;
            if (!parse_int_value(args[++index], buffer_mode)) {
                return {.ok = false, .error = "The --buffer-mode value must be an integer."};
            }
            if (buffer_mode < 0) {
                return {.ok = false, .error = "The --buffer-mode value must be non-negative."};
            }
            result.request.buffer_mode = buffer_mode;
            result.request.buffer_mode_available = true;
            continue;
        }

        if (argument == "--buffer-mode-override") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --buffer-mode-override."};
            }
            int buffer_mode_override = 0;
            if (!parse_int_value(args[++index], buffer_mode_override)) {
                return {.ok = false, .error = "The --buffer-mode-override value must be an integer."};
            }
            if (buffer_mode_override < 0) {
                return {.ok = false, .error = "The --buffer-mode-override value must be non-negative."};
            }
            result.request.buffer_mode_override = buffer_mode_override;
            result.request.buffer_mode_override_available = true;
            continue;
        }

        if (argument == "--data-session") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --data-session."};
            }
            int data_session = 0;
            if (!parse_int_value(args[++index], data_session)) {
                return {.ok = false, .error = "The --data-session value must be an integer."};
            }
            if (data_session < 0) {
                return {.ok = false, .error = "The --data-session value must be non-negative."};
            }
            result.request.data_session = data_session;
            result.request.data_session_available = true;
            continue;
        }

        if (argument == "--grid-line-color") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --grid-line-color."};
            }
            int grid_line_color = 0;
            if (!parse_int_value(args[++index], grid_line_color)) {
                return {.ok = false, .error = "The --grid-line-color value must be an integer."};
            }
            if (grid_line_color < 0) {
                return {.ok = false, .error = "The --grid-line-color value must be non-negative."};
            }
            result.request.grid_line_color = grid_line_color;
            result.request.grid_line_color_available = true;
            continue;
        }

        if (argument == "--header-height") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --header-height."};
            }
            int header_height = 0;
            if (!parse_int_value(args[++index], header_height)) {
                return {.ok = false, .error = "The --header-height value must be an integer."};
            }
            if (header_height < 0) {
                return {.ok = false, .error = "The --header-height value must be non-negative."};
            }
            result.request.header_height = header_height;
            result.request.header_height_available = true;
            continue;
        }

        if (argument == "--row-height") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --row-height."};
            }
            int row_height = 0;
            if (!parse_int_value(args[++index], row_height)) {
                return {.ok = false, .error = "The --row-height value must be an integer."};
            }
            if (row_height < 0) {
                return {.ok = false, .error = "The --row-height value must be non-negative."};
            }
            result.request.row_height = row_height;
            result.request.row_height_available = true;
            continue;
        }

        if (argument == "--lock-columns") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --lock-columns."};
            }
            int lock_columns = 0;
            if (!parse_int_value(args[++index], lock_columns)) {
                return {.ok = false, .error = "The --lock-columns value must be an integer."};
            }
            if (lock_columns < 0) {
                return {.ok = false, .error = "The --lock-columns value must be non-negative."};
            }
            result.request.lock_columns = lock_columns;
            result.request.lock_columns_available = true;
            continue;
        }

        if (argument == "--lock-columns-left") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --lock-columns-left."};
            }
            int lock_columns_left = 0;
            if (!parse_int_value(args[++index], lock_columns_left)) {
                return {.ok = false, .error = "The --lock-columns-left value must be an integer."};
            }
            if (lock_columns_left < 0) {
                return {.ok = false, .error = "The --lock-columns-left value must be non-negative."};
            }
            result.request.lock_columns_left = lock_columns_left;
            result.request.lock_columns_left_available = true;
            continue;
        }

        if (argument == "--grid-line-width") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --grid-line-width."};
            }
            int grid_line_width = 0;
            if (!parse_int_value(args[++index], grid_line_width)) {
                return {.ok = false, .error = "The --grid-line-width value must be an integer."};
            }
            if (grid_line_width < 0) {
                return {.ok = false, .error = "The --grid-line-width value must be non-negative."};
            }
            result.request.grid_line_width = grid_line_width;
            result.request.grid_line_width_available = true;
            continue;
        }

        if (argument == "--grid-lines") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --grid-lines."};
            }
            int grid_lines = 0;
            if (!parse_int_value(args[++index], grid_lines)) {
                return {.ok = false, .error = "The --grid-lines value must be an integer."};
            }
            if (grid_lines < 0) {
                return {.ok = false, .error = "The --grid-lines value must be non-negative."};
            }
            result.request.grid_lines = grid_lines;
            result.request.grid_lines_available = true;
            continue;
        }

        if (argument == "--highlight-row-line-width") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --highlight-row-line-width."};
            }
            int highlight_row_line_width = 0;
            if (!parse_int_value(args[++index], highlight_row_line_width)) {
                return {.ok = false, .error = "The --highlight-row-line-width value must be an integer."};
            }
            if (highlight_row_line_width < 0) {
                return {.ok = false, .error = "The --highlight-row-line-width value must be non-negative."};
            }
            result.request.highlight_row_line_width = highlight_row_line_width;
            result.request.highlight_row_line_width_available = true;
            continue;
        }

        if (argument == "--display-value") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --display-value."};
            }
            result.request.display_value = args[++index];
            result.request.display_value_available = true;
            continue;
        }

        if (argument == "--dynamic-back-color") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --dynamic-back-color."};
            }
            result.request.dynamic_back_color = args[++index];
            result.request.dynamic_back_color_available = true;
            continue;
        }

        if (argument == "--dynamic-fore-color") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --dynamic-fore-color."};
            }
            result.request.dynamic_fore_color = args[++index];
            result.request.dynamic_fore_color_available = true;
            continue;
        }

        if (argument == "--closable") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --closable."};
            }
            const auto closable = parse_bool_value(args[++index]);
            if (!closable.has_value()) {
                return {.ok = false, .error = "The --closable value must be true or false."};
            }
            result.request.closable = *closable;
            result.request.closable_available = true;
            continue;
        }

        if (argument == "--control-box") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --control-box."};
            }
            const auto control_box = parse_bool_value(args[++index]);
            if (!control_box.has_value()) {
                return {.ok = false, .error = "The --control-box value must be true or false."};
            }
            result.request.control_box = *control_box;
            result.request.control_box_available = true;
            continue;
        }

        if (argument == "--allow-output") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --allow-output."};
            }
            const auto allow_output = parse_bool_value(args[++index]);
            if (!allow_output.has_value()) {
                return {.ok = false, .error = "The --allow-output value must be true or false."};
            }
            result.request.allow_output = *allow_output;
            result.request.allow_output_available = true;
            continue;
        }

        if (argument == "--auto-center") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --auto-center."};
            }
            const auto auto_center = parse_bool_value(args[++index]);
            if (!auto_center.has_value()) {
                return {.ok = false, .error = "The --auto-center value must be true or false."};
            }
            result.request.auto_center = *auto_center;
            result.request.auto_center_available = true;
            continue;
        }

        if (argument == "--auto-size") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --auto-size."};
            }
            const auto auto_size = parse_bool_value(args[++index]);
            if (!auto_size.has_value()) {
                return {.ok = false, .error = "The --auto-size value must be true or false."};
            }
            result.request.auto_size = *auto_size;
            result.request.auto_size_available = true;
            continue;
        }

        if (argument == "--auto-release") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --auto-release."};
            }
            const auto auto_release = parse_bool_value(args[++index]);
            if (!auto_release.has_value()) {
                return {.ok = false, .error = "The --auto-release value must be true or false."};
            }
            result.request.auto_release = *auto_release;
            result.request.auto_release_available = true;
            continue;
        }

        if (argument == "--continuous-scroll") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --continuous-scroll."};
            }
            const auto continuous_scroll = parse_bool_value(args[++index]);
            if (!continuous_scroll.has_value()) {
                return {.ok = false, .error = "The --continuous-scroll value must be true or false."};
            }
            result.request.continuous_scroll = *continuous_scroll;
            result.request.continuous_scroll_available = true;
            continue;
        }

        if (argument == "--dockable") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --dockable."};
            }
            const auto dockable = parse_bool_value(args[++index]);
            if (!dockable.has_value()) {
                return {.ok = false, .error = "The --dockable value must be true or false."};
            }
            result.request.dockable = *dockable;
            result.request.dockable_available = true;
            continue;
        }

        if (argument == "--clip-controls") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --clip-controls."};
            }
            const auto clip_controls = parse_bool_value(args[++index]);
            if (!clip_controls.has_value()) {
                return {.ok = false, .error = "The --clip-controls value must be true or false."};
            }
            result.request.clip_controls = *clip_controls;
            result.request.clip_controls_available = true;
            continue;
        }

        if (argument == "--sparse") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --sparse."};
            }
            const auto sparse = parse_bool_value(args[++index]);
            if (!sparse.has_value()) {
                return {.ok = false, .error = "The --sparse value must be true or false."};
            }
            result.request.sparse = *sparse;
            result.request.sparse_available = true;
            continue;
        }

        if (argument == "--lock-screen") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --lock-screen."};
            }
            const auto lock_screen = parse_bool_value(args[++index]);
            if (!lock_screen.has_value()) {
                return {.ok = false, .error = "The --lock-screen value must be true or false."};
            }
            result.request.lock_screen = *lock_screen;
            result.request.lock_screen_available = true;
            continue;
        }

        if (argument == "--allow-cell-selection") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --allow-cell-selection."};
            }
            const auto allow_cell_selection = parse_bool_value(args[++index]);
            if (!allow_cell_selection.has_value()) {
                return {.ok = false, .error = "The --allow-cell-selection value must be true or false."};
            }
            result.request.allow_cell_selection = *allow_cell_selection;
            result.request.allow_cell_selection_available = true;
            continue;
        }

        if (argument == "--hide-selection") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --hide-selection."};
            }
            const auto hide_selection = parse_bool_value(args[++index]);
            if (!hide_selection.has_value()) {
                return {.ok = false, .error = "The --hide-selection value must be true or false."};
            }
            result.request.hide_selection = *hide_selection;
            result.request.hide_selection_available = true;
            continue;
        }

        if (argument == "--delete-mark") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --delete-mark."};
            }
            const auto delete_mark = parse_bool_value(args[++index]);
            if (!delete_mark.has_value()) {
                return {.ok = false, .error = "The --delete-mark value must be true or false."};
            }
            result.request.delete_mark = *delete_mark;
            result.request.delete_mark_available = true;
            continue;
        }

        if (argument == "--record-mark") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --record-mark."};
            }
            const auto record_mark = parse_bool_value(args[++index]);
            if (!record_mark.has_value()) {
                return {.ok = false, .error = "The --record-mark value must be true or false."};
            }
            result.request.record_mark = *record_mark;
            result.request.record_mark_available = true;
            continue;
        }

        if (argument == "--split-bar") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --split-bar."};
            }
            const auto split_bar = parse_bool_value(args[++index]);
            if (!split_bar.has_value()) {
                return {.ok = false, .error = "The --split-bar value must be true or false."};
            }
            result.request.split_bar = *split_bar;
            result.request.split_bar_available = true;
            continue;
        }

        if (argument == "--highlight-row") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --highlight-row."};
            }
            const auto highlight_row = parse_bool_value(args[++index]);
            if (!highlight_row.has_value()) {
                return {.ok = false, .error = "The --highlight-row value must be true or false."};
            }
            result.request.highlight_row = *highlight_row;
            result.request.highlight_row_available = true;
            continue;
        }

        if (argument == "--panel-link") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --panel-link."};
            }
            const auto panel_link = parse_bool_value(args[++index]);
            if (!panel_link.has_value()) {
                return {.ok = false, .error = "The --panel-link value must be true or false."};
            }
            result.request.panel_link = *panel_link;
            result.request.panel_link_available = true;
            continue;
        }

        if (argument == "--allow-header-sizing") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --allow-header-sizing."};
            }
            const auto allow_header_sizing = parse_bool_value(args[++index]);
            if (!allow_header_sizing.has_value()) {
                return {.ok = false, .error = "The --allow-header-sizing value must be true or false."};
            }
            result.request.allow_header_sizing = *allow_header_sizing;
            result.request.allow_header_sizing_available = true;
            continue;
        }

        if (argument == "--allow-row-sizing") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --allow-row-sizing."};
            }
            const auto allow_row_sizing = parse_bool_value(args[++index]);
            if (!allow_row_sizing.has_value()) {
                return {.ok = false, .error = "The --allow-row-sizing value must be true or false."};
            }
            result.request.allow_row_sizing = *allow_row_sizing;
            result.request.allow_row_sizing_available = true;
            continue;
        }

        if (argument == "--resizable") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --resizable."};
            }
            const auto resizable = parse_bool_value(args[++index]);
            if (!resizable.has_value()) {
                return {.ok = false, .error = "The --resizable value must be true or false."};
            }
            result.request.resizable = *resizable;
            result.request.resizable_available = true;
            continue;
        }

        if (argument == "--add-line-feeds") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --add-line-feeds."};
            }
            const auto add_line_feeds = parse_bool_value(args[++index]);
            if (!add_line_feeds.has_value()) {
                return {.ok = false, .error = "The --add-line-feeds value must be true or false."};
            }
            result.request.add_line_feeds = *add_line_feeds;
            result.request.add_line_feeds_available = true;
            continue;
        }

        if (argument == "--always-on-top") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --always-on-top."};
            }
            const auto always_on_top = parse_bool_value(args[++index]);
            if (!always_on_top.has_value()) {
                return {.ok = false, .error = "The --always-on-top value must be true or false."};
            }
            result.request.always_on_top = *always_on_top;
            result.request.always_on_top_available = true;
            continue;
        }

        if (argument == "--always-on-bottom") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --always-on-bottom."};
            }
            const auto always_on_bottom = parse_bool_value(args[++index]);
            if (!always_on_bottom.has_value()) {
                return {.ok = false, .error = "The --always-on-bottom value must be true or false."};
            }
            result.request.always_on_bottom = *always_on_bottom;
            result.request.always_on_bottom_available = true;
            continue;
        }

        if (argument == "--selected-back-color") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --selected-back-color."};
            }
            int selected_back_color = 0;
            if (!parse_int_value(args[++index], selected_back_color)) {
                return {.ok = false, .error = "The --selected-back-color value must be an integer."};
            }
            result.request.selected_back_color = selected_back_color;
            result.request.selected_back_color_available = true;
            continue;
        }

        if (argument == "--selected-fore-color") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --selected-fore-color."};
            }
            int selected_fore_color = 0;
            if (!parse_int_value(args[++index], selected_fore_color)) {
                return {.ok = false, .error = "The --selected-fore-color value must be an integer."};
            }
            result.request.selected_fore_color = selected_fore_color;
            result.request.selected_fore_color_available = true;
            continue;
        }

        if (argument == "--selected-item-back-color") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --selected-item-back-color."};
            }
            int selected_item_back_color = 0;
            if (!parse_int_value(args[++index], selected_item_back_color)) {
                return {.ok = false, .error = "The --selected-item-back-color value must be an integer."};
            }
            result.request.selected_item_back_color = selected_item_back_color;
            result.request.selected_item_back_color_available = true;
            continue;
        }

        if (argument == "--selected-item-fore-color") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --selected-item-fore-color."};
            }
            int selected_item_fore_color = 0;
            if (!parse_int_value(args[++index], selected_item_fore_color)) {
                return {.ok = false, .error = "The --selected-item-fore-color value must be an integer."};
            }
            result.request.selected_item_fore_color = selected_item_fore_color;
            result.request.selected_item_fore_color_available = true;
            continue;
        }

        if (argument == "--disabled-item-back-color") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --disabled-item-back-color."};
            }
            int disabled_item_back_color = 0;
            if (!parse_int_value(args[++index], disabled_item_back_color)) {
                return {.ok = false, .error = "The --disabled-item-back-color value must be an integer."};
            }
            result.request.disabled_item_back_color = disabled_item_back_color;
            result.request.disabled_item_back_color_available = true;
            continue;
        }

        if (argument == "--disabled-item-fore-color") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --disabled-item-fore-color."};
            }
            int disabled_item_fore_color = 0;
            if (!parse_int_value(args[++index], disabled_item_fore_color)) {
                return {.ok = false, .error = "The --disabled-item-fore-color value must be an integer."};
            }
            result.request.disabled_item_fore_color = disabled_item_fore_color;
            result.request.disabled_item_fore_color_available = true;
            continue;
        }

        if (argument == "--item-back-color") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --item-back-color."};
            }
            int item_back_color = 0;
            if (!parse_int_value(args[++index], item_back_color)) {
                return {.ok = false, .error = "The --item-back-color value must be an integer."};
            }
            result.request.item_back_color = item_back_color;
            result.request.item_back_color_available = true;
            continue;
        }

        if (argument == "--item-fore-color") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --item-fore-color."};
            }
            int item_fore_color = 0;
            if (!parse_int_value(args[++index], item_fore_color)) {
                return {.ok = false, .error = "The --item-fore-color value must be an integer."};
            }
            result.request.item_fore_color = item_fore_color;
            result.request.item_fore_color_available = true;
            continue;
        }

        if (argument == "--highlight-back-color") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --highlight-back-color."};
            }
            int highlight_back_color = 0;
            if (!parse_int_value(args[++index], highlight_back_color)) {
                return {.ok = false, .error = "The --highlight-back-color value must be an integer."};
            }
            result.request.highlight_back_color = highlight_back_color;
            result.request.highlight_back_color_available = true;
            continue;
        }

        if (argument == "--highlight-fore-color") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --highlight-fore-color."};
            }
            int highlight_fore_color = 0;
            if (!parse_int_value(args[++index], highlight_fore_color)) {
                return {.ok = false, .error = "The --highlight-fore-color value must be an integer."};
            }
            result.request.highlight_fore_color = highlight_fore_color;
            result.request.highlight_fore_color_available = true;
            continue;
        }

        if (argument == "--back-color") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --back-color."};
            }
            int back_color = 0;
            if (!parse_int_value(args[++index], back_color)) {
                return {.ok = false, .error = "The --back-color value must be an integer."};
            }
            result.request.back_color = back_color;
            result.request.back_color_available = true;
            continue;
        }

        if (argument == "--fore-color") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --fore-color."};
            }
            int fore_color = 0;
            if (!parse_int_value(args[++index], fore_color)) {
                return {.ok = false, .error = "The --fore-color value must be an integer."};
            }
            result.request.fore_color = fore_color;
            result.request.fore_color_available = true;
            continue;
        }

        if (argument == "--disabled-back-color") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --disabled-back-color."};
            }
            int disabled_back_color = 0;
            if (!parse_int_value(args[++index], disabled_back_color)) {
                return {.ok = false, .error = "The --disabled-back-color value must be an integer."};
            }
            result.request.disabled_back_color = disabled_back_color;
            result.request.disabled_back_color_available = true;
            continue;
        }

        if (argument == "--disabled-fore-color") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --disabled-fore-color."};
            }
            int disabled_fore_color = 0;
            if (!parse_int_value(args[++index], disabled_fore_color)) {
                return {.ok = false, .error = "The --disabled-fore-color value must be an integer."};
            }
            result.request.disabled_fore_color = disabled_fore_color;
            result.request.disabled_fore_color_available = true;
            continue;
        }

        if (argument == "--anchor-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --anchor-object-name."};
            }
            result.request.anchor_object_name = args[++index];
            continue;
        }

        if (argument == "--anchor-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --anchor-unique-id."};
            }
            result.request.anchor_unique_id = args[++index];
            continue;
        }

        if (argument == "--align-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --align-target-object-name."};
            }
            result.request.align_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--align-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --align-target-unique-id."};
            }
            result.request.align_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--resize-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --resize-target-object-name."};
            }
            result.request.resize_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--resize-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --resize-target-unique-id."};
            }
            result.request.resize_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--distribute-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --distribute-target-object-name."};
            }
            result.request.distribute_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--distribute-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --distribute-target-unique-id."};
            }
            result.request.distribute_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--snap-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --snap-target-object-name."};
            }
            result.request.snap_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--snap-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --snap-target-unique-id."};
            }
            result.request.snap_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--nudge-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --nudge-target-object-name."};
            }
            result.request.nudge_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--nudge-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --nudge-target-unique-id."};
            }
            result.request.nudge_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--tab-order-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --tab-order-target-object-name."};
            }
            result.request.tab_order_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--tab-order-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --tab-order-target-unique-id."};
            }
            result.request.tab_order_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--tab-stop-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --tab-stop-target-object-name."};
            }
            result.request.tab_stop_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--tab-stop-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --tab-stop-target-unique-id."};
            }
            result.request.tab_stop_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--visibility-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --visibility-target-object-name."};
            }
            result.request.visibility_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--visibility-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --visibility-target-unique-id."};
            }
            result.request.visibility_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--enabled-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --enabled-target-object-name."};
            }
            result.request.enabled_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--enabled-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --enabled-target-unique-id."};
            }
            result.request.enabled_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--read-only-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --read-only-target-object-name."};
            }
            result.request.read_only_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--read-only-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --read-only-target-unique-id."};
            }
            result.request.read_only_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--locked-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --locked-target-object-name."};
            }
            result.request.locked_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--locked-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --locked-target-unique-id."};
            }
            result.request.locked_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--caption-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --caption-target-object-name."};
            }
            result.request.caption_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--caption-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --caption-target-unique-id."};
            }
            result.request.caption_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--picture-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --picture-target-object-name."};
            }
            result.request.picture_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--picture-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --picture-target-unique-id."};
            }
            result.request.picture_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--down-picture-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --down-picture-target-object-name."};
            }
            result.request.down_picture_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--down-picture-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --down-picture-target-unique-id."};
            }
            result.request.down_picture_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--disabled-picture-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --disabled-picture-target-object-name."};
            }
            result.request.disabled_picture_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--disabled-picture-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --disabled-picture-target-unique-id."};
            }
            result.request.disabled_picture_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--ole-drag-picture-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --ole-drag-picture-target-object-name."};
            }
            result.request.ole_drag_picture_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--ole-drag-picture-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --ole-drag-picture-target-unique-id."};
            }
            result.request.ole_drag_picture_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--mouse-icon-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --mouse-icon-target-object-name."};
            }
            result.request.mouse_icon_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--mouse-icon-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --mouse-icon-target-unique-id."};
            }
            result.request.mouse_icon_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--drag-icon-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --drag-icon-target-object-name."};
            }
            result.request.drag_icon_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--drag-icon-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --drag-icon-target-unique-id."};
            }
            result.request.drag_icon_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--drag-mode-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --drag-mode-target-object-name."};
            }
            result.request.drag_mode_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--drag-mode-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --drag-mode-target-unique-id."};
            }
            result.request.drag_mode_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--ole-drag-mode-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --ole-drag-mode-target-object-name."};
            }
            result.request.ole_drag_mode_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--ole-drag-mode-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --ole-drag-mode-target-unique-id."};
            }
            result.request.ole_drag_mode_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--ole-drop-mode-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --ole-drop-mode-target-object-name."};
            }
            result.request.ole_drop_mode_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--ole-drop-mode-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --ole-drop-mode-target-unique-id."};
            }
            result.request.ole_drop_mode_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--ole-drop-effects-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --ole-drop-effects-target-object-name."};
            }
            result.request.ole_drop_effects_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--ole-drop-effects-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --ole-drop-effects-target-unique-id."};
            }
            result.request.ole_drop_effects_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--ole-drop-text-insertion-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --ole-drop-text-insertion-target-object-name."};
            }
            result.request.ole_drop_text_insertion_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--ole-drop-text-insertion-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --ole-drop-text-insertion-target-unique-id."};
            }
            result.request.ole_drop_text_insertion_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--button-count-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --button-count-target-object-name."};
            }
            result.request.button_count_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--button-count-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --button-count-target-unique-id."};
            }
            result.request.button_count_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--curvature-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --curvature-target-object-name."};
            }
            result.request.curvature_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--curvature-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --curvature-target-unique-id."};
            }
            result.request.curvature_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--draw-mode-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --draw-mode-target-object-name."};
            }
            result.request.draw_mode_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--draw-mode-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --draw-mode-target-unique-id."};
            }
            result.request.draw_mode_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--draw-style-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --draw-style-target-object-name."};
            }
            result.request.draw_style_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--draw-style-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --draw-style-target-unique-id."};
            }
            result.request.draw_style_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--draw-width-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --draw-width-target-object-name."};
            }
            result.request.draw_width_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--draw-width-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --draw-width-target-unique-id."};
            }
            result.request.draw_width_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--fill-style-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --fill-style-target-object-name."};
            }
            result.request.fill_style_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--fill-style-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --fill-style-target-unique-id."};
            }
            result.request.fill_style_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--scale-mode-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --scale-mode-target-object-name."};
            }
            result.request.scale_mode_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--scale-mode-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --scale-mode-target-unique-id."};
            }
            result.request.scale_mode_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--buffer-mode-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --buffer-mode-target-object-name."};
            }
            result.request.buffer_mode_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--buffer-mode-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --buffer-mode-target-unique-id."};
            }
            result.request.buffer_mode_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--buffer-mode-override-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --buffer-mode-override-target-object-name."};
            }
            result.request.buffer_mode_override_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--buffer-mode-override-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --buffer-mode-override-target-unique-id."};
            }
            result.request.buffer_mode_override_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--data-session-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --data-session-target-object-name."};
            }
            result.request.data_session_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--data-session-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --data-session-target-unique-id."};
            }
            result.request.data_session_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--grid-line-color-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --grid-line-color-target-object-name."};
            }
            result.request.grid_line_color_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--grid-line-color-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --grid-line-color-target-unique-id."};
            }
            result.request.grid_line_color_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--header-height-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --header-height-target-object-name."};
            }
            result.request.header_height_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--header-height-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --header-height-target-unique-id."};
            }
            result.request.header_height_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--row-height-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --row-height-target-object-name."};
            }
            result.request.row_height_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--row-height-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --row-height-target-unique-id."};
            }
            result.request.row_height_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--lock-columns-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --lock-columns-target-object-name."};
            }
            result.request.lock_columns_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--lock-columns-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --lock-columns-target-unique-id."};
            }
            result.request.lock_columns_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--lock-columns-left-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --lock-columns-left-target-object-name."};
            }
            result.request.lock_columns_left_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--lock-columns-left-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --lock-columns-left-target-unique-id."};
            }
            result.request.lock_columns_left_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--grid-line-width-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --grid-line-width-target-object-name."};
            }
            result.request.grid_line_width_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--grid-line-width-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --grid-line-width-target-unique-id."};
            }
            result.request.grid_line_width_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--grid-lines-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --grid-lines-target-object-name."};
            }
            result.request.grid_lines_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--grid-lines-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --grid-lines-target-unique-id."};
            }
            result.request.grid_lines_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--highlight-row-line-width-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --highlight-row-line-width-target-object-name."};
            }
            result.request.highlight_row_line_width_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--highlight-row-line-width-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --highlight-row-line-width-target-unique-id."};
            }
            result.request.highlight_row_line_width_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--tooltip-text-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --tooltip-text-target-object-name."};
            }
            result.request.tooltip_text_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--tooltip-text-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --tooltip-text-target-unique-id."};
            }
            result.request.tooltip_text_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--status-bar-text-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --status-bar-text-target-object-name."};
            }
            result.request.status_bar_text_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--status-bar-text-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --status-bar-text-target-unique-id."};
            }
            result.request.status_bar_text_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--control-source-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --control-source-target-object-name."};
            }
            result.request.control_source_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--control-source-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --control-source-target-unique-id."};
            }
            result.request.control_source_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--current-control-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --current-control-target-object-name."};
            }
            result.request.current_control_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--current-control-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --current-control-target-unique-id."};
            }
            result.request.current_control_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--input-mask-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --input-mask-target-object-name."};
            }
            result.request.input_mask_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--input-mask-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --input-mask-target-unique-id."};
            }
            result.request.input_mask_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--format-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --format-target-object-name."};
            }
            result.request.format_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--format-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --format-target-unique-id."};
            }
            result.request.format_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--row-source-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --row-source-target-object-name."};
            }
            result.request.row_source_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--row-source-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --row-source-target-unique-id."};
            }
            result.request.row_source_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--row-source-type-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --row-source-type-target-object-name."};
            }
            result.request.row_source_type_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--row-source-type-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --row-source-type-target-unique-id."};
            }
            result.request.row_source_type_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--bound-column-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --bound-column-target-object-name."};
            }
            result.request.bound_column_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--bound-column-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --bound-column-target-unique-id."};
            }
            result.request.bound_column_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--column-count-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --column-count-target-object-name."};
            }
            result.request.column_count_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--column-count-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --column-count-target-unique-id."};
            }
            result.request.column_count_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--style-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --style-target-object-name."};
            }
            result.request.style_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--style-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --style-target-unique-id."};
            }
            result.request.style_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--list-index-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --list-index-target-object-name."};
            }
            result.request.list_index_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--list-index-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --list-index-target-unique-id."};
            }
            result.request.list_index_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--left-column-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --left-column-target-object-name."};
            }
            result.request.left_column_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--left-column-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --left-column-target-unique-id."};
            }
            result.request.left_column_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--display-value-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --display-value-target-object-name."};
            }
            result.request.display_value_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--display-value-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --display-value-target-unique-id."};
            }
            result.request.display_value_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--selected-back-color-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --selected-back-color-target-object-name."};
            }
            result.request.selected_back_color_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--selected-back-color-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --selected-back-color-target-unique-id."};
            }
            result.request.selected_back_color_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--selected-fore-color-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --selected-fore-color-target-object-name."};
            }
            result.request.selected_fore_color_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--selected-fore-color-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --selected-fore-color-target-unique-id."};
            }
            result.request.selected_fore_color_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--selected-item-back-color-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --selected-item-back-color-target-object-name."};
            }
            result.request.selected_item_back_color_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--selected-item-back-color-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --selected-item-back-color-target-unique-id."};
            }
            result.request.selected_item_back_color_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--selected-item-fore-color-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --selected-item-fore-color-target-object-name."};
            }
            result.request.selected_item_fore_color_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--selected-item-fore-color-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --selected-item-fore-color-target-unique-id."};
            }
            result.request.selected_item_fore_color_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--disabled-item-back-color-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --disabled-item-back-color-target-object-name."};
            }
            result.request.disabled_item_back_color_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--disabled-item-back-color-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --disabled-item-back-color-target-unique-id."};
            }
            result.request.disabled_item_back_color_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--disabled-item-fore-color-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --disabled-item-fore-color-target-object-name."};
            }
            result.request.disabled_item_fore_color_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--disabled-item-fore-color-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --disabled-item-fore-color-target-unique-id."};
            }
            result.request.disabled_item_fore_color_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--item-back-color-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --item-back-color-target-object-name."};
            }
            result.request.item_back_color_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--item-back-color-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --item-back-color-target-unique-id."};
            }
            result.request.item_back_color_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--item-fore-color-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --item-fore-color-target-object-name."};
            }
            result.request.item_fore_color_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--item-fore-color-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --item-fore-color-target-unique-id."};
            }
            result.request.item_fore_color_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--highlight-back-color-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --highlight-back-color-target-object-name."};
            }
            result.request.highlight_back_color_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--highlight-back-color-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --highlight-back-color-target-unique-id."};
            }
            result.request.highlight_back_color_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--highlight-fore-color-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --highlight-fore-color-target-object-name."};
            }
            result.request.highlight_fore_color_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--highlight-fore-color-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --highlight-fore-color-target-unique-id."};
            }
            result.request.highlight_fore_color_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--back-color-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --back-color-target-object-name."};
            }
            result.request.back_color_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--back-color-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --back-color-target-unique-id."};
            }
            result.request.back_color_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--fore-color-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --fore-color-target-object-name."};
            }
            result.request.fore_color_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--fore-color-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --fore-color-target-unique-id."};
            }
            result.request.fore_color_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--disabled-back-color-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --disabled-back-color-target-object-name."};
            }
            result.request.disabled_back_color_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--disabled-back-color-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --disabled-back-color-target-unique-id."};
            }
            result.request.disabled_back_color_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--disabled-fore-color-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --disabled-fore-color-target-object-name."};
            }
            result.request.disabled_fore_color_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--disabled-fore-color-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --disabled-fore-color-target-unique-id."};
            }
            result.request.disabled_fore_color_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--dynamic-back-color-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --dynamic-back-color-target-object-name."};
            }
            result.request.dynamic_back_color_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--dynamic-back-color-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --dynamic-back-color-target-unique-id."};
            }
            result.request.dynamic_back_color_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--dynamic-fore-color-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --dynamic-fore-color-target-object-name."};
            }
            result.request.dynamic_fore_color_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--dynamic-fore-color-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --dynamic-fore-color-target-unique-id."};
            }
            result.request.dynamic_fore_color_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--closable-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --closable-target-object-name."};
            }
            result.request.closable_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--closable-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --closable-target-unique-id."};
            }
            result.request.closable_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--control-box-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --control-box-target-object-name."};
            }
            result.request.control_box_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--control-box-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --control-box-target-unique-id."};
            }
            result.request.control_box_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--allow-output-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --allow-output-target-object-name."};
            }
            result.request.allow_output_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--allow-output-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --allow-output-target-unique-id."};
            }
            result.request.allow_output_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--auto-center-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --auto-center-target-object-name."};
            }
            result.request.auto_center_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--auto-center-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --auto-center-target-unique-id."};
            }
            result.request.auto_center_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--auto-size-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --auto-size-target-object-name."};
            }
            result.request.auto_size_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--auto-size-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --auto-size-target-unique-id."};
            }
            result.request.auto_size_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--auto-release-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --auto-release-target-object-name."};
            }
            result.request.auto_release_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--auto-release-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --auto-release-target-unique-id."};
            }
            result.request.auto_release_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--continuous-scroll-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --continuous-scroll-target-object-name."};
            }
            result.request.continuous_scroll_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--continuous-scroll-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --continuous-scroll-target-unique-id."};
            }
            result.request.continuous_scroll_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--dockable-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --dockable-target-object-name."};
            }
            result.request.dockable_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--dockable-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --dockable-target-unique-id."};
            }
            result.request.dockable_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--clip-controls-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --clip-controls-target-object-name."};
            }
            result.request.clip_controls_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--clip-controls-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --clip-controls-target-unique-id."};
            }
            result.request.clip_controls_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--sparse-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --sparse-target-object-name."};
            }
            result.request.sparse_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--sparse-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --sparse-target-unique-id."};
            }
            result.request.sparse_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--lock-screen-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --lock-screen-target-object-name."};
            }
            result.request.lock_screen_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--lock-screen-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --lock-screen-target-unique-id."};
            }
            result.request.lock_screen_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--allow-cell-selection-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --allow-cell-selection-target-object-name."};
            }
            result.request.allow_cell_selection_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--allow-cell-selection-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --allow-cell-selection-target-unique-id."};
            }
            result.request.allow_cell_selection_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--hide-selection-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --hide-selection-target-object-name."};
            }
            result.request.hide_selection_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--hide-selection-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --hide-selection-target-unique-id."};
            }
            result.request.hide_selection_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--delete-mark-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --delete-mark-target-object-name."};
            }
            result.request.delete_mark_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--delete-mark-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --delete-mark-target-unique-id."};
            }
            result.request.delete_mark_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--record-mark-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --record-mark-target-object-name."};
            }
            result.request.record_mark_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--record-mark-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --record-mark-target-unique-id."};
            }
            result.request.record_mark_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--split-bar-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --split-bar-target-object-name."};
            }
            result.request.split_bar_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--split-bar-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --split-bar-target-unique-id."};
            }
            result.request.split_bar_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--highlight-row-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --highlight-row-target-object-name."};
            }
            result.request.highlight_row_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--highlight-row-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --highlight-row-target-unique-id."};
            }
            result.request.highlight_row_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--panel-link-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --panel-link-target-object-name."};
            }
            result.request.panel_link_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--panel-link-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --panel-link-target-unique-id."};
            }
            result.request.panel_link_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--allow-header-sizing-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --allow-header-sizing-target-object-name."};
            }
            result.request.allow_header_sizing_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--allow-header-sizing-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --allow-header-sizing-target-unique-id."};
            }
            result.request.allow_header_sizing_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--allow-row-sizing-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --allow-row-sizing-target-object-name."};
            }
            result.request.allow_row_sizing_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--allow-row-sizing-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --allow-row-sizing-target-unique-id."};
            }
            result.request.allow_row_sizing_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--resizable-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --resizable-target-object-name."};
            }
            result.request.resizable_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--resizable-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --resizable-target-unique-id."};
            }
            result.request.resizable_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--add-line-feeds-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --add-line-feeds-target-object-name."};
            }
            result.request.add_line_feeds_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--add-line-feeds-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --add-line-feeds-target-unique-id."};
            }
            result.request.add_line_feeds_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--always-on-top-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --always-on-top-target-object-name."};
            }
            result.request.always_on_top_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--always-on-top-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --always-on-top-target-unique-id."};
            }
            result.request.always_on_top_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--always-on-bottom-target-object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --always-on-bottom-target-object-name."};
            }
            result.request.always_on_bottom_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            continue;
        }

        if (argument == "--always-on-bottom-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --always-on-bottom-target-unique-id."};
            }
            result.request.always_on_bottom_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            continue;
        }

        if (argument == "--object-name") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --object-name."};
            }
            result.request.object_name = args[++index];
            continue;
        }

        if (argument == "--unique-id") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --unique-id."};
            }
            result.request.unique_id = args[++index];
            continue;
        }

        if (argument == "--line") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --line."};
            }
            std::size_t line = 0;
            if (!parse_size_value(args[++index], line)) {
                return {.ok = false, .error = "The --line value must be an unsigned integer."};
            }
            result.request.line = line;
            continue;
        }

        if (argument == "--column") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --column."};
            }
            std::size_t column = 0;
            if (!parse_size_value(args[++index], column)) {
                return {.ok = false, .error = "The --column value must be an unsigned integer."};
            }
            result.request.column = column;
            continue;
        }

        if (argument == "--undo-mode") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --undo-mode."};
            }
            std::string mode = lowercase_copy(args[++index]);
            if (mode == "edit") {
                result.request.undo_mode = StudioUndoMode::edit;
                continue;
            }
            if (mode == "command") {
                result.request.undo_mode = StudioUndoMode::command;
                continue;
            }
            return {.ok = false, .error = "The --undo-mode value must be edit or command."};
        }

        if (argument == "--undo-label") {
            if ((index + 1U) >= args.size()) {
                return {.ok = false, .error = "Missing value after --undo-label."};
            }
            result.request.undo_label = args[++index];
            continue;
        }

        if (!argument.empty() && argument[0] == '-') {
            return {.ok = false, .error = "Unknown argument: " + argument};
        }

        if (result.request.path.empty()) {
            result.request.path = argument;
            continue;
        }

        return {.ok = false, .error = "Unexpected extra positional argument: " + argument};
    }

    if (result.request.path.empty()) {
        return {.ok = false, .error = "No asset path was provided."};
    }

    if (result.request.apply_property_update && result.request.property_name.empty()) {
        return {.ok = false, .error = "A property update requires --property-name."};
    }
    if (result.request.clear_property && result.request.property_name.empty()) {
        return {.ok = false, .error = "A property clear requires --property-name."};
    }
    if (result.request.rename_property && result.request.property_name.empty()) {
        return {.ok = false, .error = "A property rename requires --property-name."};
    }
    if (result.request.rename_property && result.request.new_property_name.empty()) {
        return {.ok = false, .error = "A property rename requires --new-property-name."};
    }
    if (result.request.rename_object &&
        result.request.new_object_name.empty() &&
        result.request.new_name.empty() &&
        result.request.new_unique_id.empty()) {
        return {.ok = false, .error = "An object rename requires --new-object-name, --new-name, or --new-unique-id."};
    }
    if (result.request.reparent_object &&
        !result.request.clear_parent &&
        result.request.parent_name.empty() &&
        result.request.parent_unique_id.empty()) {
        return {.ok = false, .error = "An object reparent requires --parent-name, --parent-unique-id, or --clear-parent."};
    }
    if (result.request.reorder_object && result.request.placement.empty()) {
        return {.ok = false, .error = "An object reorder requires --placement."};
    }
    if (result.request.group_object && result.request.field_values.empty()) {
        return {.ok = false, .error = "An object group requires at least one --field-value."};
    }
    if (result.request.group_object && result.request.group_objects.empty()) {
        return {.ok = false, .error = "An object group requires at least one grouped child selector."};
    }
    if (!result.request.group_object && !result.request.field_values.empty()) {
        return {.ok = false, .error = "--field-value can only be used with --group-object."};
    }
    if (!result.request.group_object && !result.request.group_objects.empty()) {
        return {.ok = false, .error = "Grouped child selectors can only be used with --group-object."};
    }
    if (result.request.align_object && result.request.alignment_mode.empty()) {
        return {.ok = false, .error = "An object alignment requires --alignment-mode."};
    }
    if (result.request.align_object &&
        result.request.anchor_object_name.empty() &&
        result.request.anchor_unique_id.empty()) {
        return {.ok = false, .error = "An object alignment requires --anchor-object-name or --anchor-unique-id."};
    }
    if (result.request.align_object && result.request.align_objects.empty()) {
        return {.ok = false, .error = "An object alignment requires at least one target selector."};
    }
    if (!result.request.align_object &&
        (!result.request.alignment_mode.empty() ||
         !result.request.align_objects.empty())) {
        return {.ok = false, .error = "Alignment arguments can only be used with --align-object."};
    }
    if (result.request.resize_object && result.request.resize_mode.empty()) {
        return {.ok = false, .error = "An object resize requires --resize-mode."};
    }
    if (result.request.resize_object &&
        result.request.anchor_object_name.empty() &&
        result.request.anchor_unique_id.empty()) {
        return {.ok = false, .error = "An object resize requires --anchor-object-name or --anchor-unique-id."};
    }
    if (result.request.resize_object && result.request.resize_objects.empty()) {
        return {.ok = false, .error = "An object resize requires at least one target selector."};
    }
    if (!result.request.resize_object &&
        (!result.request.resize_mode.empty() ||
         !result.request.resize_objects.empty())) {
        return {.ok = false, .error = "Resize arguments can only be used with --resize-object."};
    }
    if (result.request.distribute_object && result.request.distribution_mode.empty()) {
        return {.ok = false, .error = "An object distribution requires --distribution-mode."};
    }
    if (result.request.distribute_object && result.request.distribute_objects.empty()) {
        return {.ok = false, .error = "An object distribution requires at least one target selector."};
    }
    if (!result.request.distribute_object &&
        (!result.request.distribution_mode.empty() ||
         !result.request.distribute_objects.empty())) {
        return {.ok = false, .error = "Distribution arguments can only be used with --distribute-object."};
    }
    if (result.request.snap_object && result.request.snap_mode.empty()) {
        return {.ok = false, .error = "An object snap requires --snap-mode."};
    }
    if (result.request.snap_object && result.request.snap_objects.empty()) {
        return {.ok = false, .error = "An object snap requires at least one target selector."};
    }
    if (!result.request.snap_object &&
        (!result.request.snap_mode.empty() ||
         result.request.grid_width != 0.0 ||
         result.request.grid_height != 0.0 ||
         !result.request.snap_objects.empty())) {
        return {.ok = false, .error = "Snap arguments can only be used with --snap-object."};
    }
    if (result.request.nudge_object && result.request.nudge_mode.empty()) {
        return {.ok = false, .error = "An object nudge requires --nudge-mode."};
    }
    if (result.request.nudge_object && result.request.nudge_objects.empty()) {
        return {.ok = false, .error = "An object nudge requires at least one target selector."};
    }
    if (!result.request.nudge_object &&
        (!result.request.nudge_mode.empty() ||
         result.request.delta_hpos != 0.0 ||
         result.request.delta_vpos != 0.0 ||
         !result.request.nudge_objects.empty())) {
        return {.ok = false, .error = "Nudge arguments can only be used with --nudge-object."};
    }
    if (result.request.tab_order_object && result.request.tab_order_objects.empty()) {
        return {.ok = false, .error = "An object tab-order assignment requires at least one target selector."};
    }
    if (result.request.tab_order_object && result.request.starting_tab_index < 0) {
        return {.ok = false, .error = "An object tab-order assignment requires a non-negative starting tab index."};
    }
    if (!result.request.tab_order_object &&
        (result.request.starting_tab_index_available ||
         !result.request.tab_order_objects.empty())) {
        return {.ok = false, .error = "Tab-order arguments can only be used with --tab-order-object."};
    }
    if (result.request.tab_stop_object && !result.request.tab_stop_available) {
        return {.ok = false, .error = "An object tab-stop assignment requires --tab-stop."};
    }
    if (result.request.tab_stop_object && result.request.tab_stop_objects.empty()) {
        return {.ok = false, .error = "An object tab-stop assignment requires at least one target selector."};
    }
    if (!result.request.tab_stop_object &&
        (result.request.tab_stop_available ||
         !result.request.tab_stop_objects.empty())) {
        return {.ok = false, .error = "Tab-stop arguments can only be used with --tab-stop-object."};
    }
    if (result.request.visibility_object && !result.request.visible_available) {
        return {.ok = false, .error = "An object visibility assignment requires --visible."};
    }
    if (result.request.visibility_object && result.request.visibility_objects.empty()) {
        return {.ok = false, .error = "An object visibility assignment requires at least one target selector."};
    }
    if (!result.request.visibility_object &&
        (result.request.visible_available ||
         !result.request.visibility_objects.empty())) {
        return {.ok = false, .error = "Visibility arguments can only be used with --visibility-object."};
    }
    if (result.request.enabled_object && !result.request.enabled_available) {
        return {.ok = false, .error = "An object enabled assignment requires --enabled."};
    }
    if (result.request.enabled_object && result.request.enabled_objects.empty()) {
        return {.ok = false, .error = "An object enabled assignment requires at least one target selector."};
    }
    if (!result.request.enabled_object &&
        (result.request.enabled_available ||
         !result.request.enabled_objects.empty())) {
        return {.ok = false, .error = "Enabled arguments can only be used with --enabled-object."};
    }
    if (result.request.read_only_object && !result.request.object_read_only_available) {
        return {.ok = false, .error = "An object read-only assignment requires --object-read-only."};
    }
    if (result.request.read_only_object && result.request.read_only_objects.empty()) {
        return {.ok = false, .error = "An object read-only assignment requires at least one target selector."};
    }
    if (!result.request.read_only_object &&
        (result.request.object_read_only_available ||
         !result.request.read_only_objects.empty())) {
        return {.ok = false, .error = "Read-only arguments can only be used with --read-only-object."};
    }
    if (result.request.locked_object && !result.request.locked_available) {
        return {.ok = false, .error = "An object locked assignment requires --locked."};
    }
    if (result.request.locked_object && result.request.locked_objects.empty()) {
        return {.ok = false, .error = "An object locked assignment requires at least one target selector."};
    }
    if (!result.request.locked_object &&
        (result.request.locked_available ||
         !result.request.locked_objects.empty())) {
        return {.ok = false, .error = "Locked arguments can only be used with --locked-object."};
    }
    if (result.request.caption_object && !result.request.caption_available) {
        return {.ok = false, .error = "An object caption assignment requires --caption."};
    }
    if (result.request.caption_object && result.request.caption_objects.empty()) {
        return {.ok = false, .error = "An object caption assignment requires at least one target selector."};
    }
    if (!result.request.caption_object &&
        (result.request.caption_available ||
         !result.request.caption_objects.empty())) {
        return {.ok = false, .error = "Caption arguments can only be used with --caption-object."};
    }
    if (result.request.picture_object && !result.request.picture_available) {
        return {.ok = false, .error = "An object picture assignment requires --picture."};
    }
    if (result.request.picture_object && result.request.picture_objects.empty()) {
        return {.ok = false, .error = "An object picture assignment requires at least one target selector."};
    }
    if (!result.request.picture_object &&
        (result.request.picture_available ||
         !result.request.picture_objects.empty())) {
        return {.ok = false, .error = "Picture arguments can only be used with --picture-object."};
    }
    if (result.request.down_picture_object && !result.request.down_picture_available) {
        return {.ok = false, .error = "An object down-picture assignment requires --down-picture."};
    }
    if (result.request.down_picture_object && result.request.down_picture_objects.empty()) {
        return {.ok = false, .error = "An object down-picture assignment requires at least one target selector."};
    }
    if (!result.request.down_picture_object &&
        (result.request.down_picture_available ||
         !result.request.down_picture_objects.empty())) {
        return {.ok = false, .error = "Down-picture arguments can only be used with --down-picture-object."};
    }
    if (result.request.disabled_picture_object && !result.request.disabled_picture_available) {
        return {.ok = false, .error = "An object disabled-picture assignment requires --disabled-picture."};
    }
    if (result.request.disabled_picture_object && result.request.disabled_picture_objects.empty()) {
        return {.ok = false, .error = "An object disabled-picture assignment requires at least one target selector."};
    }
    if (!result.request.disabled_picture_object &&
        (result.request.disabled_picture_available ||
         !result.request.disabled_picture_objects.empty())) {
        return {.ok = false, .error = "Disabled-picture arguments can only be used with --disabled-picture-object."};
    }
    if (result.request.ole_drag_picture_object && !result.request.ole_drag_picture_available) {
        return {.ok = false, .error = "An object OLE drag-picture assignment requires --ole-drag-picture."};
    }
    if (result.request.ole_drag_picture_object && result.request.ole_drag_picture_objects.empty()) {
        return {.ok = false, .error = "An object OLE drag-picture assignment requires at least one target selector."};
    }
    if (!result.request.ole_drag_picture_object &&
        (result.request.ole_drag_picture_available ||
         !result.request.ole_drag_picture_objects.empty())) {
        return {.ok = false, .error = "OLE drag-picture arguments can only be used with --ole-drag-picture-object."};
    }
    if (result.request.mouse_icon_object && !result.request.mouse_icon_available) {
        return {.ok = false, .error = "An object mouse-icon assignment requires --mouse-icon."};
    }
    if (result.request.mouse_icon_object && result.request.mouse_icon_objects.empty()) {
        return {.ok = false, .error = "An object mouse-icon assignment requires at least one target selector."};
    }
    if (!result.request.mouse_icon_object &&
        (result.request.mouse_icon_available ||
         !result.request.mouse_icon_objects.empty())) {
        return {.ok = false, .error = "Mouse-icon arguments can only be used with --mouse-icon-object."};
    }
    if (result.request.drag_icon_object && !result.request.drag_icon_available) {
        return {.ok = false, .error = "An object drag-icon assignment requires --drag-icon."};
    }
    if (result.request.drag_icon_object && result.request.drag_icon_objects.empty()) {
        return {.ok = false, .error = "An object drag-icon assignment requires at least one target selector."};
    }
    if (!result.request.drag_icon_object &&
        (result.request.drag_icon_available ||
         !result.request.drag_icon_objects.empty())) {
        return {.ok = false, .error = "Drag-icon arguments can only be used with --drag-icon-object."};
    }
    if (result.request.drag_mode_object && !result.request.drag_mode_available) {
        return {.ok = false, .error = "An object drag-mode assignment requires --drag-mode."};
    }
    if (result.request.drag_mode_object && result.request.drag_mode_objects.empty()) {
        return {.ok = false, .error = "An object drag-mode assignment requires at least one target selector."};
    }
    if (!result.request.drag_mode_object &&
        (result.request.drag_mode_available ||
         !result.request.drag_mode_objects.empty())) {
        return {.ok = false, .error = "Drag-mode arguments can only be used with --drag-mode-object."};
    }
    if (result.request.ole_drag_mode_object && !result.request.ole_drag_mode_available) {
        return {.ok = false, .error = "An object OLE drag-mode assignment requires --ole-drag-mode."};
    }
    if (result.request.ole_drag_mode_object && result.request.ole_drag_mode_objects.empty()) {
        return {.ok = false, .error = "An object OLE drag-mode assignment requires at least one target selector."};
    }
    if (!result.request.ole_drag_mode_object &&
        (result.request.ole_drag_mode_available ||
         !result.request.ole_drag_mode_objects.empty())) {
        return {.ok = false, .error = "OLE drag-mode arguments can only be used with --ole-drag-mode-object."};
    }
    if (result.request.ole_drop_mode_object && !result.request.ole_drop_mode_available) {
        return {.ok = false, .error = "An object OLE drop-mode assignment requires --ole-drop-mode."};
    }
    if (result.request.ole_drop_mode_object && result.request.ole_drop_mode_objects.empty()) {
        return {.ok = false, .error = "An object OLE drop-mode assignment requires at least one target selector."};
    }
    if (!result.request.ole_drop_mode_object &&
        (result.request.ole_drop_mode_available ||
         !result.request.ole_drop_mode_objects.empty())) {
        return {.ok = false, .error = "OLE drop-mode arguments can only be used with --ole-drop-mode-object."};
    }
    if (result.request.ole_drop_effects_object && !result.request.ole_drop_effects_available) {
        return {.ok = false, .error = "An object OLE drop-effects assignment requires --ole-drop-effects."};
    }
    if (result.request.ole_drop_effects_object && result.request.ole_drop_effects_objects.empty()) {
        return {.ok = false, .error = "An object OLE drop-effects assignment requires at least one target selector."};
    }
    if (!result.request.ole_drop_effects_object &&
        (result.request.ole_drop_effects_available ||
         !result.request.ole_drop_effects_objects.empty())) {
        return {.ok = false, .error = "OLE drop-effects arguments can only be used with --ole-drop-effects-object."};
    }
    if (result.request.ole_drop_text_insertion_object && !result.request.ole_drop_text_insertion_available) {
        return {.ok = false, .error = "An object OLE drop text-insertion assignment requires --ole-drop-text-insertion."};
    }
    if (result.request.ole_drop_text_insertion_object && result.request.ole_drop_text_insertion_objects.empty()) {
        return {.ok = false, .error = "An object OLE drop text-insertion assignment requires at least one target selector."};
    }
    if (!result.request.ole_drop_text_insertion_object &&
        (result.request.ole_drop_text_insertion_available ||
         !result.request.ole_drop_text_insertion_objects.empty())) {
        return {.ok = false, .error = "OLE drop text-insertion arguments can only be used with --ole-drop-text-insertion-object."};
    }
    if (result.request.button_count_object && !result.request.button_count_available) {
        return {.ok = false, .error = "An object button-count assignment requires --button-count."};
    }
    if (result.request.button_count_object && result.request.button_count_objects.empty()) {
        return {.ok = false, .error = "An object button-count assignment requires at least one target selector."};
    }
    if (!result.request.button_count_object &&
        (result.request.button_count_available ||
         !result.request.button_count_objects.empty())) {
        return {.ok = false, .error = "Button-count arguments can only be used with --button-count-object."};
    }
    if (result.request.curvature_object && !result.request.curvature_available) {
        return {.ok = false, .error = "An object curvature assignment requires --curvature."};
    }
    if (result.request.curvature_object && result.request.curvature_objects.empty()) {
        return {.ok = false, .error = "An object curvature assignment requires at least one target selector."};
    }
    if (!result.request.curvature_object &&
        (result.request.curvature_available ||
         !result.request.curvature_objects.empty())) {
        return {.ok = false, .error = "Curvature arguments can only be used with --curvature-object."};
    }
    if (result.request.draw_mode_object && !result.request.draw_mode_available) {
        return {.ok = false, .error = "An object draw-mode assignment requires --draw-mode."};
    }
    if (result.request.draw_mode_object && result.request.draw_mode_objects.empty()) {
        return {.ok = false, .error = "An object draw-mode assignment requires at least one target selector."};
    }
    if (!result.request.draw_mode_object &&
        (result.request.draw_mode_available ||
         !result.request.draw_mode_objects.empty())) {
        return {.ok = false, .error = "Draw-mode arguments can only be used with --draw-mode-object."};
    }
    if (result.request.draw_style_object && !result.request.draw_style_available) {
        return {.ok = false, .error = "An object draw-style assignment requires --draw-style."};
    }
    if (result.request.draw_style_object && result.request.draw_style_objects.empty()) {
        return {.ok = false, .error = "An object draw-style assignment requires at least one target selector."};
    }
    if (!result.request.draw_style_object &&
        (result.request.draw_style_available ||
         !result.request.draw_style_objects.empty())) {
        return {.ok = false, .error = "Draw-style arguments can only be used with --draw-style-object."};
    }
    if (result.request.draw_width_object && !result.request.draw_width_available) {
        return {.ok = false, .error = "An object draw-width assignment requires --draw-width."};
    }
    if (result.request.draw_width_object && result.request.draw_width_objects.empty()) {
        return {.ok = false, .error = "An object draw-width assignment requires at least one target selector."};
    }
    if (!result.request.draw_width_object &&
        (result.request.draw_width_available ||
         !result.request.draw_width_objects.empty())) {
        return {.ok = false, .error = "Draw-width arguments can only be used with --draw-width-object."};
    }
    if (result.request.fill_style_object && !result.request.fill_style_available) {
        return {.ok = false, .error = "An object fill-style assignment requires --fill-style."};
    }
    if (result.request.fill_style_object && result.request.fill_style_objects.empty()) {
        return {.ok = false, .error = "An object fill-style assignment requires at least one target selector."};
    }
    if (!result.request.fill_style_object &&
        (result.request.fill_style_available ||
         !result.request.fill_style_objects.empty())) {
        return {.ok = false, .error = "Fill-style arguments can only be used with --fill-style-object."};
    }
    if (result.request.scale_mode_object && !result.request.scale_mode_available) {
        return {.ok = false, .error = "An object scale-mode assignment requires --scale-mode."};
    }
    if (result.request.scale_mode_object && result.request.scale_mode_objects.empty()) {
        return {.ok = false, .error = "An object scale-mode assignment requires at least one target selector."};
    }
    if (!result.request.scale_mode_object &&
        (result.request.scale_mode_available ||
         !result.request.scale_mode_objects.empty())) {
        return {.ok = false, .error = "Scale-mode arguments can only be used with --scale-mode-object."};
    }
    if (result.request.buffer_mode_object && !result.request.buffer_mode_available) {
        return {.ok = false, .error = "An object buffer-mode assignment requires --buffer-mode."};
    }
    if (result.request.buffer_mode_object && result.request.buffer_mode_objects.empty()) {
        return {.ok = false, .error = "An object buffer-mode assignment requires at least one target selector."};
    }
    if (!result.request.buffer_mode_object &&
        (result.request.buffer_mode_available ||
         !result.request.buffer_mode_objects.empty())) {
        return {.ok = false, .error = "Buffer-mode arguments can only be used with --buffer-mode-object."};
    }
    if (result.request.buffer_mode_override_object && !result.request.buffer_mode_override_available) {
        return {.ok = false, .error = "An object buffer-mode-override assignment requires --buffer-mode-override."};
    }
    if (result.request.buffer_mode_override_object && result.request.buffer_mode_override_objects.empty()) {
        return {.ok = false, .error = "An object buffer-mode-override assignment requires at least one target selector."};
    }
    if (!result.request.buffer_mode_override_object &&
        (result.request.buffer_mode_override_available ||
         !result.request.buffer_mode_override_objects.empty())) {
        return {.ok = false, .error = "Buffer-mode-override arguments can only be used with --buffer-mode-override-object."};
    }
    if (result.request.data_session_object && !result.request.data_session_available) {
        return {.ok = false, .error = "An object data-session assignment requires --data-session."};
    }
    if (result.request.data_session_object && result.request.data_session_objects.empty()) {
        return {.ok = false, .error = "An object data-session assignment requires at least one target selector."};
    }
    if (!result.request.data_session_object &&
        (result.request.data_session_available ||
         !result.request.data_session_objects.empty())) {
        return {.ok = false, .error = "Data-session arguments can only be used with --data-session-object."};
    }
    if (result.request.grid_line_color_object && !result.request.grid_line_color_available) {
        return {.ok = false, .error = "An object grid-line-color assignment requires --grid-line-color."};
    }
    if (result.request.grid_line_color_object && result.request.grid_line_color_objects.empty()) {
        return {.ok = false, .error = "An object grid-line-color assignment requires at least one target selector."};
    }
    if (!result.request.grid_line_color_object &&
        (result.request.grid_line_color_available ||
         !result.request.grid_line_color_objects.empty())) {
        return {.ok = false, .error = "Grid-line-color arguments can only be used with --grid-line-color-object."};
    }
    if (result.request.header_height_object && !result.request.header_height_available) {
        return {.ok = false, .error = "An object header-height assignment requires --header-height."};
    }
    if (result.request.header_height_object && result.request.header_height_objects.empty()) {
        return {.ok = false, .error = "An object header-height assignment requires at least one target selector."};
    }
    if (!result.request.header_height_object &&
        (result.request.header_height_available ||
         !result.request.header_height_objects.empty())) {
        return {.ok = false, .error = "Header-height arguments can only be used with --header-height-object."};
    }
    if (result.request.row_height_object && !result.request.row_height_available) {
        return {.ok = false, .error = "An object row-height assignment requires --row-height."};
    }
    if (result.request.row_height_object && result.request.row_height_objects.empty()) {
        return {.ok = false, .error = "An object row-height assignment requires at least one target selector."};
    }
    if (!result.request.row_height_object &&
        (result.request.row_height_available ||
         !result.request.row_height_objects.empty())) {
        return {.ok = false, .error = "Row-height arguments can only be used with --row-height-object."};
    }
    if (result.request.lock_columns_object && !result.request.lock_columns_available) {
        return {.ok = false, .error = "An object lock-columns assignment requires --lock-columns."};
    }
    if (result.request.lock_columns_object && result.request.lock_columns_objects.empty()) {
        return {.ok = false, .error = "An object lock-columns assignment requires at least one target selector."};
    }
    if (!result.request.lock_columns_object &&
        (result.request.lock_columns_available ||
         !result.request.lock_columns_objects.empty())) {
        return {.ok = false, .error = "Lock-columns arguments can only be used with --lock-columns-object."};
    }
    if (result.request.lock_columns_left_object && !result.request.lock_columns_left_available) {
        return {.ok = false, .error = "An object lock-columns-left assignment requires --lock-columns-left."};
    }
    if (result.request.lock_columns_left_object && result.request.lock_columns_left_objects.empty()) {
        return {.ok = false, .error = "An object lock-columns-left assignment requires at least one target selector."};
    }
    if (!result.request.lock_columns_left_object &&
        (result.request.lock_columns_left_available ||
         !result.request.lock_columns_left_objects.empty())) {
        return {.ok = false, .error = "Lock-columns-left arguments can only be used with --lock-columns-left-object."};
    }
    if (result.request.grid_line_width_object && !result.request.grid_line_width_available) {
        return {.ok = false, .error = "An object grid-line-width assignment requires --grid-line-width."};
    }
    if (result.request.grid_line_width_object && result.request.grid_line_width_objects.empty()) {
        return {.ok = false, .error = "An object grid-line-width assignment requires at least one target selector."};
    }
    if (!result.request.grid_line_width_object &&
        (result.request.grid_line_width_available ||
         !result.request.grid_line_width_objects.empty())) {
        return {.ok = false, .error = "Grid-line-width arguments can only be used with --grid-line-width-object."};
    }
    if (result.request.grid_lines_object && !result.request.grid_lines_available) {
        return {.ok = false, .error = "An object grid-lines assignment requires --grid-lines."};
    }
    if (result.request.grid_lines_object && result.request.grid_lines_objects.empty()) {
        return {.ok = false, .error = "An object grid-lines assignment requires at least one target selector."};
    }
    if (!result.request.grid_lines_object &&
        (result.request.grid_lines_available ||
         !result.request.grid_lines_objects.empty())) {
        return {.ok = false, .error = "Grid-lines arguments can only be used with --grid-lines-object."};
    }
    if (result.request.highlight_row_line_width_object && !result.request.highlight_row_line_width_available) {
        return {.ok = false, .error = "An object highlight-row-line-width assignment requires --highlight-row-line-width."};
    }
    if (result.request.highlight_row_line_width_object && result.request.highlight_row_line_width_objects.empty()) {
        return {.ok = false, .error = "An object highlight-row-line-width assignment requires at least one target selector."};
    }
    if (!result.request.highlight_row_line_width_object &&
        (result.request.highlight_row_line_width_available ||
         !result.request.highlight_row_line_width_objects.empty())) {
        return {.ok = false, .error = "Highlight-row-line-width arguments can only be used with --highlight-row-line-width-object."};
    }
    if (result.request.tooltip_text_object && !result.request.tooltip_text_available) {
        return {.ok = false, .error = "An object tooltip text assignment requires --tooltip-text."};
    }
    if (result.request.tooltip_text_object && result.request.tooltip_text_objects.empty()) {
        return {.ok = false, .error = "An object tooltip text assignment requires at least one target selector."};
    }
    if (!result.request.tooltip_text_object &&
        (result.request.tooltip_text_available ||
         !result.request.tooltip_text_objects.empty())) {
        return {.ok = false, .error = "Tooltip text arguments can only be used with --tooltip-text-object."};
    }
    if (result.request.status_bar_text_object && !result.request.status_bar_text_available) {
        return {.ok = false, .error = "An object status-bar text assignment requires --status-bar-text."};
    }
    if (result.request.status_bar_text_object && result.request.status_bar_text_objects.empty()) {
        return {.ok = false, .error = "An object status-bar text assignment requires at least one target selector."};
    }
    if (!result.request.status_bar_text_object &&
        (result.request.status_bar_text_available ||
         !result.request.status_bar_text_objects.empty())) {
        return {.ok = false, .error = "Status-bar text arguments can only be used with --status-bar-text-object."};
    }
    if (result.request.control_source_object && !result.request.control_source_available) {
        return {.ok = false, .error = "An object control-source assignment requires --control-source."};
    }
    if (result.request.control_source_object && result.request.control_source_objects.empty()) {
        return {.ok = false, .error = "An object control-source assignment requires at least one target selector."};
    }
    if (!result.request.control_source_object &&
        (result.request.control_source_available ||
         !result.request.control_source_objects.empty())) {
        return {.ok = false, .error = "Control-source arguments can only be used with --control-source-object."};
    }
    if (result.request.current_control_object && !result.request.current_control_available) {
        return {.ok = false, .error = "An object current-control assignment requires --current-control."};
    }
    if (result.request.current_control_object && result.request.current_control_objects.empty()) {
        return {.ok = false, .error = "An object current-control assignment requires at least one target selector."};
    }
    if (!result.request.current_control_object &&
        (result.request.current_control_available ||
         !result.request.current_control_objects.empty())) {
        return {.ok = false, .error = "Current-control arguments can only be used with --current-control-object."};
    }
    if (result.request.input_mask_object && !result.request.input_mask_available) {
        return {.ok = false, .error = "An object input-mask assignment requires --input-mask."};
    }
    if (result.request.input_mask_object && result.request.input_mask_objects.empty()) {
        return {.ok = false, .error = "An object input-mask assignment requires at least one target selector."};
    }
    if (!result.request.input_mask_object &&
        (result.request.input_mask_available ||
         !result.request.input_mask_objects.empty())) {
        return {.ok = false, .error = "Input-mask arguments can only be used with --input-mask-object."};
    }
    if (result.request.format_object && !result.request.format_available) {
        return {.ok = false, .error = "An object format assignment requires --format."};
    }
    if (result.request.format_object && result.request.format_objects.empty()) {
        return {.ok = false, .error = "An object format assignment requires at least one target selector."};
    }
    if (!result.request.format_object &&
        (result.request.format_available ||
         !result.request.format_objects.empty())) {
        return {.ok = false, .error = "Format arguments can only be used with --format-object."};
    }
    if (result.request.row_source_object && !result.request.row_source_available) {
        return {.ok = false, .error = "An object row-source assignment requires --row-source."};
    }
    if (result.request.row_source_object && result.request.row_source_objects.empty()) {
        return {.ok = false, .error = "An object row-source assignment requires at least one target selector."};
    }
    if (!result.request.row_source_object &&
        (result.request.row_source_available ||
         !result.request.row_source_objects.empty())) {
        return {.ok = false, .error = "Row-source arguments can only be used with --row-source-object."};
    }
    if (result.request.row_source_type_object && !result.request.row_source_type_available) {
        return {.ok = false, .error = "An object row-source-type assignment requires --row-source-type."};
    }
    if (result.request.row_source_type_object && result.request.row_source_type < 0) {
        return {.ok = false, .error = "An object row-source-type assignment requires a non-negative value."};
    }
    if (result.request.row_source_type_object && result.request.row_source_type_objects.empty()) {
        return {.ok = false, .error = "An object row-source-type assignment requires at least one target selector."};
    }
    if (!result.request.row_source_type_object &&
        (result.request.row_source_type_available ||
         !result.request.row_source_type_objects.empty())) {
        return {.ok = false, .error = "Row-source-type arguments can only be used with --row-source-type-object."};
    }
    if (result.request.bound_column_object && !result.request.bound_column_available) {
        return {.ok = false, .error = "An object bound-column assignment requires --bound-column."};
    }
    if (result.request.bound_column_object && result.request.bound_column < 0) {
        return {.ok = false, .error = "An object bound-column assignment requires a non-negative value."};
    }
    if (result.request.bound_column_object && result.request.bound_column_objects.empty()) {
        return {.ok = false, .error = "An object bound-column assignment requires at least one target selector."};
    }
    if (!result.request.bound_column_object &&
        (result.request.bound_column_available ||
         !result.request.bound_column_objects.empty())) {
        return {.ok = false, .error = "Bound-column arguments can only be used with --bound-column-object."};
    }
    if (result.request.column_count_object && !result.request.column_count_available) {
        return {.ok = false, .error = "An object column-count assignment requires --column-count."};
    }
    if (result.request.column_count_object && result.request.column_count < 0) {
        return {.ok = false, .error = "An object column-count assignment requires a non-negative value."};
    }
    if (result.request.column_count_object && result.request.column_count_objects.empty()) {
        return {.ok = false, .error = "An object column-count assignment requires at least one target selector."};
    }
    if (!result.request.column_count_object &&
        (result.request.column_count_available ||
         !result.request.column_count_objects.empty())) {
        return {.ok = false, .error = "Column-count arguments can only be used with --column-count-object."};
    }
    if (result.request.style_object && !result.request.style_available) {
        return {.ok = false, .error = "An object style assignment requires --style."};
    }
    if (result.request.style_object && result.request.style < 0) {
        return {.ok = false, .error = "An object style assignment requires a non-negative value."};
    }
    if (result.request.style_object && result.request.style_objects.empty()) {
        return {.ok = false, .error = "An object style assignment requires at least one target selector."};
    }
    if (!result.request.style_object &&
        (result.request.style_available ||
         !result.request.style_objects.empty())) {
        return {.ok = false, .error = "Style arguments can only be used with --style-object."};
    }
    if (result.request.list_index_object && !result.request.list_index_available) {
        return {.ok = false, .error = "An object list-index assignment requires --list-index."};
    }
    if (result.request.list_index_object && result.request.list_index < 0) {
        return {.ok = false, .error = "An object list-index assignment requires a non-negative value."};
    }
    if (result.request.list_index_object && result.request.list_index_objects.empty()) {
        return {.ok = false, .error = "An object list-index assignment requires at least one target selector."};
    }
    if (!result.request.list_index_object &&
        (result.request.list_index_available ||
         !result.request.list_index_objects.empty())) {
        return {.ok = false, .error = "List-index arguments can only be used with --list-index-object."};
    }
    if (result.request.left_column_object && !result.request.left_column_available) {
        return {.ok = false, .error = "An object left-column assignment requires --left-column."};
    }
    if (result.request.left_column_object && result.request.left_column < 0) {
        return {.ok = false, .error = "An object left-column assignment requires a non-negative value."};
    }
    if (result.request.left_column_object && result.request.left_column_objects.empty()) {
        return {.ok = false, .error = "An object left-column assignment requires at least one target selector."};
    }
    if (!result.request.left_column_object &&
        (result.request.left_column_available ||
         !result.request.left_column_objects.empty())) {
        return {.ok = false, .error = "Left-column arguments can only be used with --left-column-object."};
    }
    if (result.request.display_value_object && !result.request.display_value_available) {
        return {.ok = false, .error = "An object display-value assignment requires --display-value."};
    }
    if (result.request.display_value_object && result.request.display_value_objects.empty()) {
        return {.ok = false, .error = "An object display-value assignment requires at least one target selector."};
    }
    if (!result.request.display_value_object &&
        (result.request.display_value_available ||
         !result.request.display_value_objects.empty())) {
        return {.ok = false, .error = "Display-value arguments can only be used with --display-value-object."};
    }
    if (result.request.selected_back_color_object && !result.request.selected_back_color_available) {
        return {.ok = false, .error = "An object selected-back-color assignment requires --selected-back-color."};
    }
    if (result.request.selected_back_color_object && result.request.selected_back_color < 0) {
        return {.ok = false, .error = "An object selected-back-color assignment requires a non-negative value."};
    }
    if (result.request.selected_back_color_object && result.request.selected_back_color_objects.empty()) {
        return {.ok = false, .error = "An object selected-back-color assignment requires at least one target selector."};
    }
    if (!result.request.selected_back_color_object &&
        (result.request.selected_back_color_available ||
         !result.request.selected_back_color_objects.empty())) {
        return {.ok = false, .error = "Selected-back-color arguments can only be used with --selected-back-color-object."};
    }
    if (result.request.selected_fore_color_object && !result.request.selected_fore_color_available) {
        return {.ok = false, .error = "An object selected-fore-color assignment requires --selected-fore-color."};
    }
    if (result.request.selected_fore_color_object && result.request.selected_fore_color < 0) {
        return {.ok = false, .error = "An object selected-fore-color assignment requires a non-negative value."};
    }
    if (result.request.selected_fore_color_object && result.request.selected_fore_color_objects.empty()) {
        return {.ok = false, .error = "An object selected-fore-color assignment requires at least one target selector."};
    }
    if (!result.request.selected_fore_color_object &&
        (result.request.selected_fore_color_available ||
         !result.request.selected_fore_color_objects.empty())) {
        return {.ok = false, .error = "Selected-fore-color arguments can only be used with --selected-fore-color-object."};
    }
    if (result.request.selected_item_back_color_object && !result.request.selected_item_back_color_available) {
        return {.ok = false, .error = "An object selected-item-back-color assignment requires --selected-item-back-color."};
    }
    if (result.request.selected_item_back_color_object && result.request.selected_item_back_color < 0) {
        return {.ok = false, .error = "An object selected-item-back-color assignment requires a non-negative value."};
    }
    if (result.request.selected_item_back_color_object && result.request.selected_item_back_color_objects.empty()) {
        return {.ok = false, .error = "An object selected-item-back-color assignment requires at least one target selector."};
    }
    if (!result.request.selected_item_back_color_object &&
        (result.request.selected_item_back_color_available ||
         !result.request.selected_item_back_color_objects.empty())) {
        return {.ok = false, .error = "Selected-item-back-color arguments can only be used with --selected-item-back-color-object."};
    }
    if (result.request.selected_item_fore_color_object && !result.request.selected_item_fore_color_available) {
        return {.ok = false, .error = "An object selected-item-fore-color assignment requires --selected-item-fore-color."};
    }
    if (result.request.selected_item_fore_color_object && result.request.selected_item_fore_color < 0) {
        return {.ok = false, .error = "An object selected-item-fore-color assignment requires a non-negative value."};
    }
    if (result.request.selected_item_fore_color_object && result.request.selected_item_fore_color_objects.empty()) {
        return {.ok = false, .error = "An object selected-item-fore-color assignment requires at least one target selector."};
    }
    if (!result.request.selected_item_fore_color_object &&
        (result.request.selected_item_fore_color_available ||
         !result.request.selected_item_fore_color_objects.empty())) {
        return {.ok = false, .error = "Selected-item-fore-color arguments can only be used with --selected-item-fore-color-object."};
    }
    if (result.request.disabled_item_back_color_object && !result.request.disabled_item_back_color_available) {
        return {.ok = false, .error = "An object disabled-item-back-color assignment requires --disabled-item-back-color."};
    }
    if (result.request.disabled_item_back_color_object && result.request.disabled_item_back_color < 0) {
        return {.ok = false, .error = "An object disabled-item-back-color assignment requires a non-negative value."};
    }
    if (result.request.disabled_item_back_color_object && result.request.disabled_item_back_color_objects.empty()) {
        return {.ok = false, .error = "An object disabled-item-back-color assignment requires at least one target selector."};
    }
    if (!result.request.disabled_item_back_color_object &&
        (result.request.disabled_item_back_color_available ||
         !result.request.disabled_item_back_color_objects.empty())) {
        return {.ok = false, .error = "Disabled-item-back-color arguments can only be used with --disabled-item-back-color-object."};
    }
    if (result.request.disabled_item_fore_color_object && !result.request.disabled_item_fore_color_available) {
        return {.ok = false, .error = "An object disabled-item-fore-color assignment requires --disabled-item-fore-color."};
    }
    if (result.request.disabled_item_fore_color_object && result.request.disabled_item_fore_color < 0) {
        return {.ok = false, .error = "An object disabled-item-fore-color assignment requires a non-negative value."};
    }
    if (result.request.disabled_item_fore_color_object && result.request.disabled_item_fore_color_objects.empty()) {
        return {.ok = false, .error = "An object disabled-item-fore-color assignment requires at least one target selector."};
    }
    if (!result.request.disabled_item_fore_color_object &&
        (result.request.disabled_item_fore_color_available ||
         !result.request.disabled_item_fore_color_objects.empty())) {
        return {.ok = false, .error = "Disabled-item-fore-color arguments can only be used with --disabled-item-fore-color-object."};
    }
    if (result.request.item_back_color_object && !result.request.item_back_color_available) {
        return {.ok = false, .error = "An object item-back-color assignment requires --item-back-color."};
    }
    if (result.request.item_back_color_object && result.request.item_back_color < 0) {
        return {.ok = false, .error = "An object item-back-color assignment requires a non-negative value."};
    }
    if (result.request.item_back_color_object && result.request.item_back_color_objects.empty()) {
        return {.ok = false, .error = "An object item-back-color assignment requires at least one target selector."};
    }
    if (!result.request.item_back_color_object &&
        (result.request.item_back_color_available ||
         !result.request.item_back_color_objects.empty())) {
        return {.ok = false, .error = "Item-back-color arguments can only be used with --item-back-color-object."};
    }
    if (result.request.item_fore_color_object && !result.request.item_fore_color_available) {
        return {.ok = false, .error = "An object item-fore-color assignment requires --item-fore-color."};
    }
    if (result.request.item_fore_color_object && result.request.item_fore_color < 0) {
        return {.ok = false, .error = "An object item-fore-color assignment requires a non-negative value."};
    }
    if (result.request.item_fore_color_object && result.request.item_fore_color_objects.empty()) {
        return {.ok = false, .error = "An object item-fore-color assignment requires at least one target selector."};
    }
    if (!result.request.item_fore_color_object &&
        (result.request.item_fore_color_available ||
         !result.request.item_fore_color_objects.empty())) {
        return {.ok = false, .error = "Item-fore-color arguments can only be used with --item-fore-color-object."};
    }
    if (result.request.highlight_back_color_object && !result.request.highlight_back_color_available) {
        return {.ok = false, .error = "An object highlight-back-color assignment requires --highlight-back-color."};
    }
    if (result.request.highlight_back_color_object && result.request.highlight_back_color < 0) {
        return {.ok = false, .error = "An object highlight-back-color assignment requires a non-negative value."};
    }
    if (result.request.highlight_back_color_object && result.request.highlight_back_color_objects.empty()) {
        return {.ok = false, .error = "An object highlight-back-color assignment requires at least one target selector."};
    }
    if (!result.request.highlight_back_color_object &&
        (result.request.highlight_back_color_available ||
         !result.request.highlight_back_color_objects.empty())) {
        return {.ok = false, .error = "Highlight-back-color arguments can only be used with --highlight-back-color-object."};
    }
    if (result.request.highlight_fore_color_object && !result.request.highlight_fore_color_available) {
        return {.ok = false, .error = "An object highlight-fore-color assignment requires --highlight-fore-color."};
    }
    if (result.request.highlight_fore_color_object && result.request.highlight_fore_color < 0) {
        return {.ok = false, .error = "An object highlight-fore-color assignment requires a non-negative value."};
    }
    if (result.request.highlight_fore_color_object && result.request.highlight_fore_color_objects.empty()) {
        return {.ok = false, .error = "An object highlight-fore-color assignment requires at least one target selector."};
    }
    if (!result.request.highlight_fore_color_object &&
        (result.request.highlight_fore_color_available ||
         !result.request.highlight_fore_color_objects.empty())) {
        return {.ok = false, .error = "Highlight-fore-color arguments can only be used with --highlight-fore-color-object."};
    }
    if (result.request.back_color_object && !result.request.back_color_available) {
        return {.ok = false, .error = "An object back-color assignment requires --back-color."};
    }
    if (result.request.back_color_object && result.request.back_color < 0) {
        return {.ok = false, .error = "An object back-color assignment requires a non-negative value."};
    }
    if (result.request.back_color_object && result.request.back_color_objects.empty()) {
        return {.ok = false, .error = "An object back-color assignment requires at least one target selector."};
    }
    if (!result.request.back_color_object &&
        (result.request.back_color_available ||
         !result.request.back_color_objects.empty())) {
        return {.ok = false, .error = "Back-color arguments can only be used with --back-color-object."};
    }
    if (result.request.fore_color_object && !result.request.fore_color_available) {
        return {.ok = false, .error = "An object fore-color assignment requires --fore-color."};
    }
    if (result.request.fore_color_object && result.request.fore_color < 0) {
        return {.ok = false, .error = "An object fore-color assignment requires a non-negative value."};
    }
    if (result.request.fore_color_object && result.request.fore_color_objects.empty()) {
        return {.ok = false, .error = "An object fore-color assignment requires at least one target selector."};
    }
    if (!result.request.fore_color_object &&
        (result.request.fore_color_available ||
         !result.request.fore_color_objects.empty())) {
        return {.ok = false, .error = "Fore-color arguments can only be used with --fore-color-object."};
    }
    if (result.request.disabled_back_color_object && !result.request.disabled_back_color_available) {
        return {.ok = false, .error = "An object disabled-back-color assignment requires --disabled-back-color."};
    }
    if (result.request.disabled_back_color_object && result.request.disabled_back_color < 0) {
        return {.ok = false, .error = "An object disabled-back-color assignment requires a non-negative value."};
    }
    if (result.request.disabled_back_color_object && result.request.disabled_back_color_objects.empty()) {
        return {.ok = false, .error = "An object disabled-back-color assignment requires at least one target selector."};
    }
    if (!result.request.disabled_back_color_object &&
        (result.request.disabled_back_color_available ||
         !result.request.disabled_back_color_objects.empty())) {
        return {.ok = false, .error = "Disabled-back-color arguments can only be used with --disabled-back-color-object."};
    }
    if (result.request.disabled_fore_color_object && !result.request.disabled_fore_color_available) {
        return {.ok = false, .error = "An object disabled-fore-color assignment requires --disabled-fore-color."};
    }
    if (result.request.disabled_fore_color_object && result.request.disabled_fore_color < 0) {
        return {.ok = false, .error = "An object disabled-fore-color assignment requires a non-negative value."};
    }
    if (result.request.disabled_fore_color_object && result.request.disabled_fore_color_objects.empty()) {
        return {.ok = false, .error = "An object disabled-fore-color assignment requires at least one target selector."};
    }
    if (!result.request.disabled_fore_color_object &&
        (result.request.disabled_fore_color_available ||
         !result.request.disabled_fore_color_objects.empty())) {
        return {.ok = false, .error = "Disabled-fore-color arguments can only be used with --disabled-fore-color-object."};
    }
    if (result.request.dynamic_back_color_object && !result.request.dynamic_back_color_available) {
        return {.ok = false, .error = "An object dynamic-back-color assignment requires --dynamic-back-color."};
    }
    if (result.request.dynamic_back_color_object && result.request.dynamic_back_color_objects.empty()) {
        return {.ok = false, .error = "An object dynamic-back-color assignment requires at least one target selector."};
    }
    if (!result.request.dynamic_back_color_object &&
        (result.request.dynamic_back_color_available ||
         !result.request.dynamic_back_color_objects.empty())) {
        return {.ok = false, .error = "Dynamic-back-color arguments can only be used with --dynamic-back-color-object."};
    }
    if (result.request.dynamic_fore_color_object && !result.request.dynamic_fore_color_available) {
        return {.ok = false, .error = "An object dynamic-fore-color assignment requires --dynamic-fore-color."};
    }
    if (result.request.dynamic_fore_color_object && result.request.dynamic_fore_color_objects.empty()) {
        return {.ok = false, .error = "An object dynamic-fore-color assignment requires at least one target selector."};
    }
    if (!result.request.dynamic_fore_color_object &&
        (result.request.dynamic_fore_color_available ||
         !result.request.dynamic_fore_color_objects.empty())) {
        return {.ok = false, .error = "Dynamic-fore-color arguments can only be used with --dynamic-fore-color-object."};
    }
    if (result.request.closable_object && !result.request.closable_available) {
        return {.ok = false, .error = "An object closable assignment requires --closable."};
    }
    if (result.request.closable_object && result.request.closable_objects.empty()) {
        return {.ok = false, .error = "An object closable assignment requires at least one target selector."};
    }
    if (!result.request.closable_object &&
        (result.request.closable_available ||
         !result.request.closable_objects.empty())) {
        return {.ok = false, .error = "Closable arguments can only be used with --closable-object."};
    }
    if (result.request.control_box_object && !result.request.control_box_available) {
        return {.ok = false, .error = "An object control-box assignment requires --control-box."};
    }
    if (result.request.control_box_object && result.request.control_box_objects.empty()) {
        return {.ok = false, .error = "An object control-box assignment requires at least one target selector."};
    }
    if (!result.request.control_box_object &&
        (result.request.control_box_available ||
         !result.request.control_box_objects.empty())) {
        return {.ok = false, .error = "Control-box arguments can only be used with --control-box-object."};
    }
    if (result.request.allow_output_object && !result.request.allow_output_available) {
        return {.ok = false, .error = "An object allow-output assignment requires --allow-output."};
    }
    if (result.request.allow_output_object && result.request.allow_output_objects.empty()) {
        return {.ok = false, .error = "An object allow-output assignment requires at least one target selector."};
    }
    if (!result.request.allow_output_object &&
        (result.request.allow_output_available ||
         !result.request.allow_output_objects.empty())) {
        return {.ok = false, .error = "Allow-output arguments can only be used with --allow-output-object."};
    }
    if (result.request.auto_center_object && !result.request.auto_center_available) {
        return {.ok = false, .error = "An object auto-center assignment requires --auto-center."};
    }
    if (result.request.auto_center_object && result.request.auto_center_objects.empty()) {
        return {.ok = false, .error = "An object auto-center assignment requires at least one target selector."};
    }
    if (!result.request.auto_center_object &&
        (result.request.auto_center_available ||
         !result.request.auto_center_objects.empty())) {
        return {.ok = false, .error = "Auto-center arguments can only be used with --auto-center-object."};
    }
    if (result.request.auto_size_object && !result.request.auto_size_available) {
        return {.ok = false, .error = "An object auto-size assignment requires --auto-size."};
    }
    if (result.request.auto_size_object && result.request.auto_size_objects.empty()) {
        return {.ok = false, .error = "An object auto-size assignment requires at least one target selector."};
    }
    if (!result.request.auto_size_object &&
        (result.request.auto_size_available ||
         !result.request.auto_size_objects.empty())) {
        return {.ok = false, .error = "Auto-size arguments can only be used with --auto-size-object."};
    }
    if (result.request.auto_release_object && !result.request.auto_release_available) {
        return {.ok = false, .error = "An object auto-release assignment requires --auto-release."};
    }
    if (result.request.auto_release_object && result.request.auto_release_objects.empty()) {
        return {.ok = false, .error = "An object auto-release assignment requires at least one target selector."};
    }
    if (!result.request.auto_release_object &&
        (result.request.auto_release_available ||
         !result.request.auto_release_objects.empty())) {
        return {.ok = false, .error = "Auto-release arguments can only be used with --auto-release-object."};
    }
    if (result.request.continuous_scroll_object && !result.request.continuous_scroll_available) {
        return {.ok = false, .error = "An object continuous-scroll assignment requires --continuous-scroll."};
    }
    if (result.request.continuous_scroll_object && result.request.continuous_scroll_objects.empty()) {
        return {.ok = false, .error = "An object continuous-scroll assignment requires at least one target selector."};
    }
    if (!result.request.continuous_scroll_object &&
        (result.request.continuous_scroll_available ||
         !result.request.continuous_scroll_objects.empty())) {
        return {.ok = false, .error = "Continuous-scroll arguments can only be used with --continuous-scroll-object."};
    }
    if (result.request.dockable_object && !result.request.dockable_available) {
        return {.ok = false, .error = "An object dockable assignment requires --dockable."};
    }
    if (result.request.dockable_object && result.request.dockable_objects.empty()) {
        return {.ok = false, .error = "An object dockable assignment requires at least one target selector."};
    }
    if (!result.request.dockable_object &&
        (result.request.dockable_available ||
         !result.request.dockable_objects.empty())) {
        return {.ok = false, .error = "Dockable arguments can only be used with --dockable-object."};
    }
    if (result.request.clip_controls_object && !result.request.clip_controls_available) {
        return {.ok = false, .error = "An object clip-controls assignment requires --clip-controls."};
    }
    if (result.request.clip_controls_object && result.request.clip_controls_objects.empty()) {
        return {.ok = false, .error = "An object clip-controls assignment requires at least one target selector."};
    }
    if (!result.request.clip_controls_object &&
        (result.request.clip_controls_available ||
         !result.request.clip_controls_objects.empty())) {
        return {.ok = false, .error = "Clip-controls arguments can only be used with --clip-controls-object."};
    }
    if (result.request.sparse_object && !result.request.sparse_available) {
        return {.ok = false, .error = "An object sparse assignment requires --sparse."};
    }
    if (result.request.sparse_object && result.request.sparse_objects.empty()) {
        return {.ok = false, .error = "An object sparse assignment requires at least one target selector."};
    }
    if (!result.request.sparse_object &&
        (result.request.sparse_available ||
         !result.request.sparse_objects.empty())) {
        return {.ok = false, .error = "Sparse arguments can only be used with --sparse-object."};
    }
    if (result.request.lock_screen_object && !result.request.lock_screen_available) {
        return {.ok = false, .error = "An object lock-screen assignment requires --lock-screen."};
    }
    if (result.request.lock_screen_object && result.request.lock_screen_objects.empty()) {
        return {.ok = false, .error = "An object lock-screen assignment requires at least one target selector."};
    }
    if (!result.request.lock_screen_object &&
        (result.request.lock_screen_available ||
         !result.request.lock_screen_objects.empty())) {
        return {.ok = false, .error = "Lock-screen arguments can only be used with --lock-screen-object."};
    }
    if (result.request.allow_cell_selection_object && !result.request.allow_cell_selection_available) {
        return {.ok = false, .error = "An object allow-cell-selection assignment requires --allow-cell-selection."};
    }
    if (result.request.allow_cell_selection_object && result.request.allow_cell_selection_objects.empty()) {
        return {.ok = false, .error = "An object allow-cell-selection assignment requires at least one target selector."};
    }
    if (!result.request.allow_cell_selection_object &&
        (result.request.allow_cell_selection_available ||
         !result.request.allow_cell_selection_objects.empty())) {
        return {.ok = false, .error = "Allow-cell-selection arguments can only be used with --allow-cell-selection-object."};
    }
    if (result.request.hide_selection_object && !result.request.hide_selection_available) {
        return {.ok = false, .error = "An object hide-selection assignment requires --hide-selection."};
    }
    if (result.request.hide_selection_object && result.request.hide_selection_objects.empty()) {
        return {.ok = false, .error = "An object hide-selection assignment requires at least one target selector."};
    }
    if (!result.request.hide_selection_object &&
        (result.request.hide_selection_available ||
         !result.request.hide_selection_objects.empty())) {
        return {.ok = false, .error = "Hide-selection arguments can only be used with --hide-selection-object."};
    }
    if (result.request.delete_mark_object && !result.request.delete_mark_available) {
        return {.ok = false, .error = "An object delete-mark assignment requires --delete-mark."};
    }
    if (result.request.delete_mark_object && result.request.delete_mark_objects.empty()) {
        return {.ok = false, .error = "An object delete-mark assignment requires at least one target selector."};
    }
    if (!result.request.delete_mark_object &&
        (result.request.delete_mark_available ||
         !result.request.delete_mark_objects.empty())) {
        return {.ok = false, .error = "Delete-mark arguments can only be used with --delete-mark-object."};
    }
    if (result.request.record_mark_object && !result.request.record_mark_available) {
        return {.ok = false, .error = "An object record-mark assignment requires --record-mark."};
    }
    if (result.request.record_mark_object && result.request.record_mark_objects.empty()) {
        return {.ok = false, .error = "An object record-mark assignment requires at least one target selector."};
    }
    if (!result.request.record_mark_object &&
        (result.request.record_mark_available ||
         !result.request.record_mark_objects.empty())) {
        return {.ok = false, .error = "Record-mark arguments can only be used with --record-mark-object."};
    }
    if (result.request.split_bar_object && !result.request.split_bar_available) {
        return {.ok = false, .error = "An object split-bar assignment requires --split-bar."};
    }
    if (result.request.split_bar_object && result.request.split_bar_objects.empty()) {
        return {.ok = false, .error = "An object split-bar assignment requires at least one target selector."};
    }
    if (!result.request.split_bar_object &&
        (result.request.split_bar_available ||
         !result.request.split_bar_objects.empty())) {
        return {.ok = false, .error = "Split-bar arguments can only be used with --split-bar-object."};
    }
    if (result.request.highlight_row_object && !result.request.highlight_row_available) {
        return {.ok = false, .error = "An object highlight-row assignment requires --highlight-row."};
    }
    if (result.request.highlight_row_object && result.request.highlight_row_objects.empty()) {
        return {.ok = false, .error = "An object highlight-row assignment requires at least one target selector."};
    }
    if (!result.request.highlight_row_object &&
        (result.request.highlight_row_available ||
         !result.request.highlight_row_objects.empty())) {
        return {.ok = false, .error = "Highlight-row arguments can only be used with --highlight-row-object."};
    }
    if (result.request.panel_link_object && !result.request.panel_link_available) {
        return {.ok = false, .error = "An object panel-link assignment requires --panel-link."};
    }
    if (result.request.panel_link_object && result.request.panel_link_objects.empty()) {
        return {.ok = false, .error = "An object panel-link assignment requires at least one target selector."};
    }
    if (!result.request.panel_link_object &&
        (result.request.panel_link_available ||
         !result.request.panel_link_objects.empty())) {
        return {.ok = false, .error = "Panel-link arguments can only be used with --panel-link-object."};
    }
    if (result.request.allow_header_sizing_object && !result.request.allow_header_sizing_available) {
        return {.ok = false, .error = "An object allow-header-sizing assignment requires --allow-header-sizing."};
    }
    if (result.request.allow_header_sizing_object && result.request.allow_header_sizing_objects.empty()) {
        return {.ok = false, .error = "An object allow-header-sizing assignment requires at least one target selector."};
    }
    if (!result.request.allow_header_sizing_object &&
        (result.request.allow_header_sizing_available ||
         !result.request.allow_header_sizing_objects.empty())) {
        return {.ok = false, .error = "Allow-header-sizing arguments can only be used with --allow-header-sizing-object."};
    }
    if (result.request.allow_row_sizing_object && !result.request.allow_row_sizing_available) {
        return {.ok = false, .error = "An object allow-row-sizing assignment requires --allow-row-sizing."};
    }
    if (result.request.allow_row_sizing_object && result.request.allow_row_sizing_objects.empty()) {
        return {.ok = false, .error = "An object allow-row-sizing assignment requires at least one target selector."};
    }
    if (!result.request.allow_row_sizing_object &&
        (result.request.allow_row_sizing_available ||
         !result.request.allow_row_sizing_objects.empty())) {
        return {.ok = false, .error = "Allow-row-sizing arguments can only be used with --allow-row-sizing-object."};
    }
    if (result.request.resizable_object && !result.request.resizable_available) {
        return {.ok = false, .error = "An object resizable assignment requires --resizable."};
    }
    if (result.request.resizable_object && result.request.resizable_objects.empty()) {
        return {.ok = false, .error = "An object resizable assignment requires at least one target selector."};
    }
    if (!result.request.resizable_object &&
        (result.request.resizable_available ||
         !result.request.resizable_objects.empty())) {
        return {.ok = false, .error = "Resizable arguments can only be used with --resizable-object."};
    }
    if (result.request.add_line_feeds_object && !result.request.add_line_feeds_available) {
        return {.ok = false, .error = "An object add-line-feeds assignment requires --add-line-feeds."};
    }
    if (result.request.add_line_feeds_object && result.request.add_line_feeds_objects.empty()) {
        return {.ok = false, .error = "An object add-line-feeds assignment requires at least one target selector."};
    }
    if (!result.request.add_line_feeds_object &&
        (result.request.add_line_feeds_available ||
         !result.request.add_line_feeds_objects.empty())) {
        return {.ok = false, .error = "Add-line-feeds arguments can only be used with --add-line-feeds-object."};
    }
    if (result.request.always_on_top_object && !result.request.always_on_top_available) {
        return {.ok = false, .error = "An object always-on-top assignment requires --always-on-top."};
    }
    if (result.request.always_on_top_object && result.request.always_on_top_objects.empty()) {
        return {.ok = false, .error = "An object always-on-top assignment requires at least one target selector."};
    }
    if (!result.request.always_on_top_object &&
        (result.request.always_on_top_available ||
         !result.request.always_on_top_objects.empty())) {
        return {.ok = false, .error = "Always-on-top arguments can only be used with --always-on-top-object."};
    }
    if (result.request.always_on_bottom_object && !result.request.always_on_bottom_available) {
        return {.ok = false, .error = "An object always-on-bottom assignment requires --always-on-bottom."};
    }
    if (result.request.always_on_bottom_object && result.request.always_on_bottom_objects.empty()) {
        return {.ok = false, .error = "An object always-on-bottom assignment requires at least one target selector."};
    }
    if (!result.request.always_on_bottom_object &&
        (result.request.always_on_bottom_available ||
         !result.request.always_on_bottom_objects.empty())) {
        return {.ok = false, .error = "Always-on-bottom arguments can only be used with --always-on-bottom-object."};
    }
    if (!result.request.align_object && !result.request.resize_object &&
        (!result.request.anchor_object_name.empty() || !result.request.anchor_unique_id.empty())) {
        return {.ok = false, .error = "Anchor selectors can only be used with --align-object or --resize-object."};
    }
    const int property_command_count =
        (result.request.apply_property_update ? 1 : 0) +
        (result.request.clear_property ? 1 : 0) +
        (result.request.rename_property ? 1 : 0);
    const int object_command_count =
        (result.request.delete_object ? 1 : 0) +
        (result.request.restore_object ? 1 : 0) +
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
        (result.request.control_source_object ? 1 : 0) +
        (result.request.current_control_object ? 1 : 0) +
        (result.request.input_mask_object ? 1 : 0) +
        (result.request.format_object ? 1 : 0) +
        (result.request.row_source_object ? 1 : 0) +
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
        (result.request.allow_output_object ? 1 : 0) +
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
        return {.ok = false, .error = "Only one property command can be used at a time."};
    }
    if (object_command_count > 1) {
        return {.ok = false, .error = "Only one object command can be used at a time."};
    }
    if (object_command_count > 0 && property_command_count > 0) {
        return {.ok = false, .error = "Object commands cannot be combined with property commands."};
    }

    result.ok = true;
    return result;
}

}  // namespace copperfin::studio
