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
