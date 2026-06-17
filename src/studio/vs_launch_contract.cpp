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
        (result.request.allow_output_object ? 1 : 0) +
        (result.request.auto_center_object ? 1 : 0) +
        (result.request.auto_size_object ? 1 : 0) +
        (result.request.auto_release_object ? 1 : 0) +
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
