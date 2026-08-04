// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "vs_launch_contract_internal.h"

namespace copperfin::studio {

LaunchArgumentDispatchOutcome try_parse_setters_behavior(
    const std::string& argument,
    const localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args,
    std::size_t& index,
    LaunchParseResult& result,
    std::string& parsed_argument_error) {
if (parse_tab_orientation_argument(argument, catalog, args, index, result, parsed_argument_error)) {
            if (!parsed_argument_error.empty()) {
                result = {.ok = false, .error = parsed_argument_error}; return {true, true};
            }
            return {true, false};
        }

if (parse_display_orientation_argument(argument, catalog, args, index, result, parsed_argument_error)) {
            if (!parsed_argument_error.empty()) {
                result = {.ok = false, .error = parsed_argument_error}; return {true, true};
            }
            return {true, false};
        }

if (parse_help_context_id_argument(argument, catalog, args, index, result, parsed_argument_error)) {
            if (!parsed_argument_error.empty()) {
                result = {.ok = false, .error = parsed_argument_error}; return {true, true};
            }
            return {true, false};
        }

if (parse_whats_this_help_id_argument(argument, catalog, args, index, result, parsed_argument_error)) {
            if (!parsed_argument_error.empty()) {
                result = {.ok = false, .error = parsed_argument_error}; return {true, true};
            }
            return {true, false};
        }

if (parse_whats_this_help_argument(argument, catalog, args, index, result, parsed_argument_error)) {
            if (!parsed_argument_error.empty()) {
                result = {.ok = false, .error = parsed_argument_error}; return {true, true};
            }
            return {true, false};
        }

if (parse_whats_this_button_argument(argument, catalog, args, index, result, parsed_argument_error)) {
            if (!parsed_argument_error.empty()) {
                result = {.ok = false, .error = parsed_argument_error}; return {true, true};
            }
            return {true, false};
        }

if (argument == "--tab-order-object") {
            result.request.tab_order_object = true;
            return {true, false};
        }

if (argument == "--tab-stop-object") {
            result.request.tab_stop_object = true;
            return {true, false};
        }

if (argument == "--visibility-object") {
            result.request.visibility_object = true;
            return {true, false};
        }

if (argument == "--enabled-object") {
            result.request.enabled_object = true;
            return {true, false};
        }

if (argument == "--read-only-object") {
            result.request.read_only_object = true;
            return {true, false};
        }

if (argument == "--locked-object") {
            result.request.locked_object = true;
            return {true, false};
        }

if (argument == "--caption-object") {
            result.request.caption_object = true;
            return {true, false};
        }

if (argument == "--tooltip-text-object") {
            result.request.tooltip_text_object = true;
            return {true, false};
        }

if (argument == "--status-bar-text-object") {
            result.request.status_bar_text_object = true;
            return {true, false};
        }

if (argument == "--control-source-object") {
            result.request.control_source_object = true;
            return {true, false};
        }

if (argument == "--current-control-object") {
            result.request.current_control_object = true;
            return {true, false};
        }

if (argument == "--closable-object") {
            result.request.closable_object = true;
            return {true, false};
        }

if (argument == "--control-box-object") {
            result.request.control_box_object = true;
            return {true, false};
        }

if (argument == "--allow-output-object") {
            result.request.allow_output_object = true;
            return {true, false};
        }

if (argument == "--bind-controls-object") {
            result.request.bind_controls_object = true;
            return {true, false};
        }

if (argument == "--auto-verb-menu-object") {
            result.request.auto_verb_menu_object = true;
            return {true, false};
        }

if (argument == "--desktop-object") {
            result.request.desktop_object = true;
            return {true, false};
        }

if (argument == "--key-preview-object") {
            result.request.key_preview_object = true;
            return {true, false};
        }

if (argument == "--mac-desktop-object") {
            result.request.mac_desktop_object = true;
            return {true, false};
        }

if (argument == "--max-button-object") {
            result.request.max_button_object = true;
            return {true, false};
        }

if (argument == "--min-button-object") {
            result.request.min_button_object = true;
            return {true, false};
        }

if (argument == "--min-height-object") {
            result.request.min_height_object = true;
            return {true, false};
        }

if (argument == "--min-width-object") {
            result.request.min_width_object = true;
            return {true, false};
        }

if (argument == "--max-height-object") {
            result.request.max_height_object = true;
            return {true, false};
        }

if (argument == "--movable-object") {
            result.request.movable_object = true;
            return {true, false};
        }

if (argument == "--half-height-caption-object") {
            result.request.half_height_caption_object = true;
            return {true, false};
        }

if (argument == "--mdi-form-object") {
            result.request.mdi_form_object = true;
            return {true, false};
        }

if (argument == "--max-width-object") {
            result.request.max_width_object = true;
            return {true, false};
        }

if (argument == "--max-left-object") {
            result.request.max_left_object = true;
            return {true, false};
        }

if (argument == "--max-top-object") {
            result.request.max_top_object = true;
            return {true, false};
        }

if (argument == "--auto-center-object") {
            result.request.auto_center_object = true;
            return {true, false};
        }

if (argument == "--auto-size-object") {
            result.request.auto_size_object = true;
            return {true, false};
        }

if (argument == "--auto-release-object") {
            result.request.auto_release_object = true;
            return {true, false};
        }

if (argument == "--continuous-scroll-object") {
            result.request.continuous_scroll_object = true;
            return {true, false};
        }

if (argument == "--dockable-object") {
            result.request.dockable_object = true;
            return {true, false};
        }

if (argument == "--clip-controls-object") {
            result.request.clip_controls_object = true;
            return {true, false};
        }

if (argument == "--sparse-object") {
            result.request.sparse_object = true;
            return {true, false};
        }

if (argument == "--lock-screen-object") {
            result.request.lock_screen_object = true;
            return {true, false};
        }

if (argument == "--hide-selection-object") {
            result.request.hide_selection_object = true;
            return {true, false};
        }

if (argument == "--allow-cell-selection-object") {
            result.request.allow_cell_selection_object = true;
            return {true, false};
        }

if (argument == "--delete-mark-object") {
            result.request.delete_mark_object = true;
            return {true, false};
        }

if (argument == "--record-mark-object") {
            result.request.record_mark_object = true;
            return {true, false};
        }

if (argument == "--split-bar-object") {
            result.request.split_bar_object = true;
            return {true, false};
        }

if (argument == "--highlight-row-object") {
            result.request.highlight_row_object = true;
            return {true, false};
        }

if (argument == "--panel-link-object") {
            result.request.panel_link_object = true;
            return {true, false};
        }

if (argument == "--allow-header-sizing-object") {
            result.request.allow_header_sizing_object = true;
            return {true, false};
        }

if (argument == "--allow-row-sizing-object") {
            result.request.allow_row_sizing_object = true;
            return {true, false};
        }

if (argument == "--resizable-object") {
            result.request.resizable_object = true;
            return {true, false};
        }

if (argument == "--add-line-feeds-object") {
            result.request.add_line_feeds_object = true;
            return {true, false};
        }

if (argument == "--always-on-top-object") {
            result.request.always_on_top_object = true;
            return {true, false};
        }

if (argument == "--always-on-bottom-object") {
            result.request.always_on_bottom_object = true;
            return {true, false};
        }

if (argument == "--list-item-id-object") {
            result.request.list_item_id_object = true;
            return {true, false};
        }

if (argument == "--tab-stop") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--tab-stop")}; return {true, true};
            }
            const auto tab_stop = parse_bool_value(args[++index]);
            if (!tab_stop.has_value()) {
                result = {.ok = false, .error = localized_true_false_value_required(catalog, "--tab-stop")}; return {true, true};
            }
            result.request.tab_stop = *tab_stop;
            result.request.tab_stop_available = true;
            return {true, false};
        }

if (argument == "--caption") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--caption")}; return {true, true};
            }
            result.request.caption = args[++index];
            result.request.caption_available = true;
            return {true, false};
        }

if (argument == "--tooltip-text") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--tooltip-text")}; return {true, true};
            }
            result.request.tooltip_text = args[++index];
            result.request.tooltip_text_available = true;
            return {true, false};
        }

if (argument == "--status-bar-text") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--status-bar-text")}; return {true, true};
            }
            result.request.status_bar_text = args[++index];
            result.request.status_bar_text_available = true;
            return {true, false};
        }

if (argument == "--control-source") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--control-source")}; return {true, true};
            }
            result.request.control_source = args[++index];
            result.request.control_source_available = true;
            return {true, false};
        }

if (argument == "--current-control") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--current-control")}; return {true, true};
            }
            result.request.current_control = args[++index];
            result.request.current_control_available = true;
            return {true, false};
        }

if (argument == "--list-item-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--list-item-id")}; return {true, true};
            }
            int list_item_id = 0;
            if (!parse_int_value(args[++index], list_item_id)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--list-item-id")}; return {true, true};
            }
            if (list_item_id < 0) {
                result = {.ok = false, .error = localized_non_negative_value_required(catalog, "--list-item-id")}; return {true, true};
            }
            result.request.list_item_id = list_item_id;
            result.request.list_item_id_available = true;
            return {true, false};
        }

if (argument == "--closable") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--closable")}; return {true, true};
            }
            const auto closable = parse_bool_value(args[++index]);
            if (!closable.has_value()) {
                result = {.ok = false, .error = localized_true_false_value_required(catalog, "--closable")}; return {true, true};
            }
            result.request.closable = *closable;
            result.request.closable_available = true;
            return {true, false};
        }

if (argument == "--control-box") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--control-box")}; return {true, true};
            }
            const auto control_box = parse_bool_value(args[++index]);
            if (!control_box.has_value()) {
                result = {.ok = false, .error = localized_true_false_value_required(catalog, "--control-box")}; return {true, true};
            }
            result.request.control_box = *control_box;
            result.request.control_box_available = true;
            return {true, false};
        }

if (argument == "--allow-output") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--allow-output")}; return {true, true};
            }
            const auto allow_output = parse_bool_value(args[++index]);
            if (!allow_output.has_value()) {
                result = {.ok = false, .error = localized_true_false_value_required(catalog, "--allow-output")}; return {true, true};
            }
            result.request.allow_output = *allow_output;
            result.request.allow_output_available = true;
            return {true, false};
        }

if (argument == "--auto-center") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--auto-center")}; return {true, true};
            }
            const auto auto_center = parse_bool_value(args[++index]);
            if (!auto_center.has_value()) {
                result = {.ok = false, .error = localized_true_false_value_required(catalog, "--auto-center")}; return {true, true};
            }
            result.request.auto_center = *auto_center;
            result.request.auto_center_available = true;
            return {true, false};
        }

if (argument == "--auto-verb-menu") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--auto-verb-menu")}; return {true, true};
            }
            const auto auto_verb_menu = parse_bool_value(args[++index]);
            if (!auto_verb_menu.has_value()) {
                result = {.ok = false, .error = localized_true_false_value_required(catalog, "--auto-verb-menu")}; return {true, true};
            }
            result.request.auto_verb_menu = *auto_verb_menu;
            result.request.auto_verb_menu_available = true;
            return {true, false};
        }

if (argument == "--bind-controls") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--bind-controls")}; return {true, true};
            }
            const auto bind_controls = parse_bool_value(args[++index]);
            if (!bind_controls.has_value()) {
                result = {.ok = false, .error = localized_true_false_value_required(catalog, "--bind-controls")}; return {true, true};
            }
            result.request.bind_controls = *bind_controls;
            result.request.bind_controls_available = true;
            return {true, false};
        }

if (argument == "--desktop") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--desktop")}; return {true, true};
            }
            const auto desktop = parse_bool_value(args[++index]);
            if (!desktop.has_value()) {
                result = {.ok = false, .error = localized_true_false_value_required(catalog, "--desktop")}; return {true, true};
            }
            result.request.desktop = *desktop;
            result.request.desktop_available = true;
            return {true, false};
        }

if (argument == "--key-preview") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--key-preview")}; return {true, true};
            }
            const auto key_preview = parse_bool_value(args[++index]);
            if (!key_preview.has_value()) {
                result = {.ok = false, .error = localized_true_false_value_required(catalog, "--key-preview")}; return {true, true};
            }
            result.request.key_preview = *key_preview;
            result.request.key_preview_available = true;
            return {true, false};
        }

if (argument == "--mac-desktop") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--mac-desktop")}; return {true, true};
            }
            const auto mac_desktop = parse_bool_value(args[++index]);
            if (!mac_desktop.has_value()) {
                result = {.ok = false, .error = localized_true_false_value_required(catalog, "--mac-desktop")}; return {true, true};
            }
            result.request.mac_desktop = *mac_desktop;
            result.request.mac_desktop_available = true;
            return {true, false};
        }

if (argument == "--max-button") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--max-button")}; return {true, true};
            }
            const auto max_button = parse_bool_value(args[++index]);
            if (!max_button.has_value()) {
                result = {.ok = false, .error = localized_true_false_value_required(catalog, "--max-button")}; return {true, true};
            }
            result.request.max_button = *max_button;
            result.request.max_button_available = true;
            return {true, false};
        }

if (argument == "--min-button") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--min-button")}; return {true, true};
            }
            const auto min_button = parse_bool_value(args[++index]);
            if (!min_button.has_value()) {
                result = {.ok = false, .error = localized_true_false_value_required(catalog, "--min-button")}; return {true, true};
            }
            result.request.min_button = *min_button;
            result.request.min_button_available = true;
            return {true, false};
        }

if (argument == "--min-height") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--min-height")}; return {true, true};
            }
            int min_height = 0;
            if (!parse_int_value(args[++index], min_height)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--min-height")}; return {true, true};
            }
            if (min_height < 0) {
                result = {.ok = false, .error = localized_not_negative_value_required(catalog, "--min-height")}; return {true, true};
            }
            result.request.min_height = min_height;
            result.request.min_height_available = true;
            return {true, false};
        }

if (argument == "--min-width") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--min-width")}; return {true, true};
            }
            int min_width = 0;
            if (!parse_int_value(args[++index], min_width)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--min-width")}; return {true, true};
            }
            if (min_width < 0) {
                result = {.ok = false, .error = localized_not_negative_value_required(catalog, "--min-width")}; return {true, true};
            }
            result.request.min_width = min_width;
            result.request.min_width_available = true;
            return {true, false};
        }

if (argument == "--max-height") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--max-height")}; return {true, true};
            }
            int max_height = 0;
            if (!parse_int_value(args[++index], max_height)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--max-height")}; return {true, true};
            }
            if (max_height < 0) {
                result = {.ok = false, .error = localized_not_negative_value_required(catalog, "--max-height")}; return {true, true};
            }
            result.request.max_height = max_height;
            result.request.max_height_available = true;
            return {true, false};
        }

if (argument == "--movable") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--movable")}; return {true, true};
            }
            const auto movable = parse_bool_value(args[++index]);
            if (!movable.has_value()) {
                result = {.ok = false, .error = localized_true_false_value_required(catalog, "--movable")}; return {true, true};
            }
            result.request.movable = *movable;
            result.request.movable_available = true;
            return {true, false};
        }

if (argument == "--half-height-caption") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--half-height-caption")}; return {true, true};
            }
            const auto half_height_caption = parse_bool_value(args[++index]);
            if (!half_height_caption.has_value()) {
                result = {.ok = false, .error = localized_true_false_value_required(catalog, "--half-height-caption")}; return {true, true};
            }
            result.request.half_height_caption = *half_height_caption;
            result.request.half_height_caption_available = true;
            return {true, false};
        }

if (argument == "--mdi-form") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--mdi-form")}; return {true, true};
            }
            const auto mdi_form = parse_bool_value(args[++index]);
            if (!mdi_form.has_value()) {
                result = {.ok = false, .error = localized_true_false_value_required(catalog, "--mdi-form")}; return {true, true};
            }
            result.request.mdi_form = *mdi_form;
            result.request.mdi_form_available = true;
            return {true, false};
        }

if (argument == "--max-width") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--max-width")}; return {true, true};
            }
            int max_width = 0;
            if (!parse_int_value(args[++index], max_width)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--max-width")}; return {true, true};
            }
            if (max_width < 0) {
                result = {.ok = false, .error = localized_not_negative_value_required(catalog, "--max-width")}; return {true, true};
            }
            result.request.max_width = max_width;
            result.request.max_width_available = true;
            return {true, false};
        }

if (argument == "--max-left") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--max-left")}; return {true, true};
            }
            int max_left = 0;
            if (!parse_int_value(args[++index], max_left)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--max-left")}; return {true, true};
            }
            if (max_left < 0) {
                result = {.ok = false, .error = localized_not_negative_value_required(catalog, "--max-left")}; return {true, true};
            }
            result.request.max_left = max_left;
            result.request.max_left_available = true;
            return {true, false};
        }

if (argument == "--max-top") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--max-top")}; return {true, true};
            }
            int max_top = 0;
            if (!parse_int_value(args[++index], max_top)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--max-top")}; return {true, true};
            }
            if (max_top < 0) {
                result = {.ok = false, .error = localized_not_negative_value_required(catalog, "--max-top")}; return {true, true};
            }
            result.request.max_top = max_top;
            result.request.max_top_available = true;
            return {true, false};
        }

if (argument == "--auto-size") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--auto-size")}; return {true, true};
            }
            const auto auto_size = parse_bool_value(args[++index]);
            if (!auto_size.has_value()) {
                result = {.ok = false, .error = localized_true_false_value_required(catalog, "--auto-size")}; return {true, true};
            }
            result.request.auto_size = *auto_size;
            result.request.auto_size_available = true;
            return {true, false};
        }

if (argument == "--auto-release") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--auto-release")}; return {true, true};
            }
            const auto auto_release = parse_bool_value(args[++index]);
            if (!auto_release.has_value()) {
                result = {.ok = false, .error = localized_true_false_value_required(catalog, "--auto-release")}; return {true, true};
            }
            result.request.auto_release = *auto_release;
            result.request.auto_release_available = true;
            return {true, false};
        }

if (argument == "--continuous-scroll") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--continuous-scroll")}; return {true, true};
            }
            const auto continuous_scroll = parse_bool_value(args[++index]);
            if (!continuous_scroll.has_value()) {
                result = {.ok = false, .error = localized_true_false_value_required(catalog, "--continuous-scroll")}; return {true, true};
            }
            result.request.continuous_scroll = *continuous_scroll;
            result.request.continuous_scroll_available = true;
            return {true, false};
        }

if (argument == "--dockable") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--dockable")}; return {true, true};
            }
            const auto dockable = parse_bool_value(args[++index]);
            if (!dockable.has_value()) {
                result = {.ok = false, .error = localized_true_false_value_required(catalog, "--dockable")}; return {true, true};
            }
            result.request.dockable = *dockable;
            result.request.dockable_available = true;
            return {true, false};
        }

if (argument == "--clip-controls") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--clip-controls")}; return {true, true};
            }
            const auto clip_controls = parse_bool_value(args[++index]);
            if (!clip_controls.has_value()) {
                result = {.ok = false, .error = localized_true_false_value_required(catalog, "--clip-controls")}; return {true, true};
            }
            result.request.clip_controls = *clip_controls;
            result.request.clip_controls_available = true;
            return {true, false};
        }

if (argument == "--sparse") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--sparse")}; return {true, true};
            }
            const auto sparse = parse_bool_value(args[++index]);
            if (!sparse.has_value()) {
                result = {.ok = false, .error = localized_true_false_value_required(catalog, "--sparse")}; return {true, true};
            }
            result.request.sparse = *sparse;
            result.request.sparse_available = true;
            return {true, false};
        }

if (argument == "--lock-screen") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--lock-screen")}; return {true, true};
            }
            const auto lock_screen = parse_bool_value(args[++index]);
            if (!lock_screen.has_value()) {
                result = {.ok = false, .error = localized_true_false_value_required(catalog, "--lock-screen")}; return {true, true};
            }
            result.request.lock_screen = *lock_screen;
            result.request.lock_screen_available = true;
            return {true, false};
        }

if (argument == "--allow-cell-selection") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--allow-cell-selection")}; return {true, true};
            }
            const auto allow_cell_selection = parse_bool_value(args[++index]);
            if (!allow_cell_selection.has_value()) {
                result = {.ok = false, .error = localized_true_false_value_required(catalog, "--allow-cell-selection")}; return {true, true};
            }
            result.request.allow_cell_selection = *allow_cell_selection;
            result.request.allow_cell_selection_available = true;
            return {true, false};
        }

if (argument == "--hide-selection") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--hide-selection")}; return {true, true};
            }
            const auto hide_selection = parse_bool_value(args[++index]);
            if (!hide_selection.has_value()) {
                result = {.ok = false, .error = localized_true_false_value_required(catalog, "--hide-selection")}; return {true, true};
            }
            result.request.hide_selection = *hide_selection;
            result.request.hide_selection_available = true;
            return {true, false};
        }

if (argument == "--delete-mark") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--delete-mark")}; return {true, true};
            }
            const auto delete_mark = parse_bool_value(args[++index]);
            if (!delete_mark.has_value()) {
                result = {.ok = false, .error = localized_true_false_value_required(catalog, "--delete-mark")}; return {true, true};
            }
            result.request.delete_mark = *delete_mark;
            result.request.delete_mark_available = true;
            return {true, false};
        }

if (argument == "--record-mark") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--record-mark")}; return {true, true};
            }
            const auto record_mark = parse_bool_value(args[++index]);
            if (!record_mark.has_value()) {
                result = {.ok = false, .error = localized_true_false_value_required(catalog, "--record-mark")}; return {true, true};
            }
            result.request.record_mark = *record_mark;
            result.request.record_mark_available = true;
            return {true, false};
        }

if (argument == "--split-bar") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--split-bar")}; return {true, true};
            }
            const auto split_bar = parse_bool_value(args[++index]);
            if (!split_bar.has_value()) {
                result = {.ok = false, .error = localized_true_false_value_required(catalog, "--split-bar")}; return {true, true};
            }
            result.request.split_bar = *split_bar;
            result.request.split_bar_available = true;
            return {true, false};
        }

if (argument == "--highlight-row") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--highlight-row")}; return {true, true};
            }
            const auto highlight_row = parse_bool_value(args[++index]);
            if (!highlight_row.has_value()) {
                result = {.ok = false, .error = localized_true_false_value_required(catalog, "--highlight-row")}; return {true, true};
            }
            result.request.highlight_row = *highlight_row;
            result.request.highlight_row_available = true;
            return {true, false};
        }

if (argument == "--panel-link") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--panel-link")}; return {true, true};
            }
            const auto panel_link = parse_bool_value(args[++index]);
            if (!panel_link.has_value()) {
                result = {.ok = false, .error = localized_true_false_value_required(catalog, "--panel-link")}; return {true, true};
            }
            result.request.panel_link = *panel_link;
            result.request.panel_link_available = true;
            return {true, false};
        }

if (argument == "--allow-header-sizing") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--allow-header-sizing")}; return {true, true};
            }
            const auto allow_header_sizing = parse_bool_value(args[++index]);
            if (!allow_header_sizing.has_value()) {
                result = {.ok = false, .error = localized_true_false_value_required(catalog, "--allow-header-sizing")}; return {true, true};
            }
            result.request.allow_header_sizing = *allow_header_sizing;
            result.request.allow_header_sizing_available = true;
            return {true, false};
        }

if (argument == "--allow-row-sizing") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--allow-row-sizing")}; return {true, true};
            }
            const auto allow_row_sizing = parse_bool_value(args[++index]);
            if (!allow_row_sizing.has_value()) {
                result = {.ok = false, .error = localized_true_false_value_required(catalog, "--allow-row-sizing")}; return {true, true};
            }
            result.request.allow_row_sizing = *allow_row_sizing;
            result.request.allow_row_sizing_available = true;
            return {true, false};
        }

if (argument == "--resizable") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--resizable")}; return {true, true};
            }
            const auto resizable = parse_bool_value(args[++index]);
            if (!resizable.has_value()) {
                result = {.ok = false, .error = localized_true_false_value_required(catalog, "--resizable")}; return {true, true};
            }
            result.request.resizable = *resizable;
            result.request.resizable_available = true;
            return {true, false};
        }

if (argument == "--add-line-feeds") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--add-line-feeds")}; return {true, true};
            }
            const auto add_line_feeds = parse_bool_value(args[++index]);
            if (!add_line_feeds.has_value()) {
                result = {.ok = false, .error = localized_true_false_value_required(catalog, "--add-line-feeds")}; return {true, true};
            }
            result.request.add_line_feeds = *add_line_feeds;
            result.request.add_line_feeds_available = true;
            return {true, false};
        }

if (argument == "--always-on-top") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--always-on-top")}; return {true, true};
            }
            const auto always_on_top = parse_bool_value(args[++index]);
            if (!always_on_top.has_value()) {
                result = {.ok = false, .error = localized_true_false_value_required(catalog, "--always-on-top")}; return {true, true};
            }
            result.request.always_on_top = *always_on_top;
            result.request.always_on_top_available = true;
            return {true, false};
        }

if (argument == "--always-on-bottom") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--always-on-bottom")}; return {true, true};
            }
            const auto always_on_bottom = parse_bool_value(args[++index]);
            if (!always_on_bottom.has_value()) {
                result = {.ok = false, .error = localized_true_false_value_required(catalog, "--always-on-bottom")}; return {true, true};
            }
            result.request.always_on_bottom = *always_on_bottom;
            result.request.always_on_bottom_available = true;
            return {true, false};
        }

if (argument == "--anchor-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--anchor-object-name")}; return {true, true};
            }
            result.request.anchor_object_name = args[++index];
            return {true, false};
        }

if (argument == "--anchor-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--anchor-unique-id")}; return {true, true};
            }
            result.request.anchor_unique_id = args[++index];
            return {true, false};
        }

if (argument == "--tab-order-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--tab-order-target-object-name")}; return {true, true};
            }
            result.request.tab_order_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--tab-order-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--tab-order-target-unique-id")}; return {true, true};
            }
            result.request.tab_order_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--tab-stop-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--tab-stop-target-object-name")}; return {true, true};
            }
            result.request.tab_stop_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--tab-stop-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--tab-stop-target-unique-id")}; return {true, true};
            }
            result.request.tab_stop_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--visibility-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--visibility-target-object-name")}; return {true, true};
            }
            result.request.visibility_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--visibility-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--visibility-target-unique-id")}; return {true, true};
            }
            result.request.visibility_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--enabled-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--enabled-target-object-name")}; return {true, true};
            }
            result.request.enabled_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--enabled-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--enabled-target-unique-id")}; return {true, true};
            }
            result.request.enabled_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--read-only-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--read-only-target-object-name")}; return {true, true};
            }
            result.request.read_only_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--read-only-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--read-only-target-unique-id")}; return {true, true};
            }
            result.request.read_only_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--locked-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--locked-target-object-name")}; return {true, true};
            }
            result.request.locked_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--locked-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--locked-target-unique-id")}; return {true, true};
            }
            result.request.locked_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--caption-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--caption-target-object-name")}; return {true, true};
            }
            result.request.caption_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--caption-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--caption-target-unique-id")}; return {true, true};
            }
            result.request.caption_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--list-item-id-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--list-item-id-target-object-name")}; return {true, true};
            }
            result.request.list_item_id_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--list-item-id-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--list-item-id-target-unique-id")}; return {true, true};
            }
            result.request.list_item_id_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--tooltip-text-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--tooltip-text-target-object-name")}; return {true, true};
            }
            result.request.tooltip_text_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--tooltip-text-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--tooltip-text-target-unique-id")}; return {true, true};
            }
            result.request.tooltip_text_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--status-bar-text-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--status-bar-text-target-object-name")}; return {true, true};
            }
            result.request.status_bar_text_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--status-bar-text-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--status-bar-text-target-unique-id")}; return {true, true};
            }
            result.request.status_bar_text_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--control-source-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--control-source-target-object-name")}; return {true, true};
            }
            result.request.control_source_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--control-source-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--control-source-target-unique-id")}; return {true, true};
            }
            result.request.control_source_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--current-control-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--current-control-target-object-name")}; return {true, true};
            }
            result.request.current_control_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--current-control-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--current-control-target-unique-id")}; return {true, true};
            }
            result.request.current_control_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--closable-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--closable-target-object-name")}; return {true, true};
            }
            result.request.closable_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--closable-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--closable-target-unique-id")}; return {true, true};
            }
            result.request.closable_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--control-box-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--control-box-target-object-name")}; return {true, true};
            }
            result.request.control_box_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--control-box-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--control-box-target-unique-id")}; return {true, true};
            }
            result.request.control_box_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--allow-output-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--allow-output-target-object-name")}; return {true, true};
            }
            result.request.allow_output_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--allow-output-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--allow-output-target-unique-id")}; return {true, true};
            }
            result.request.allow_output_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--auto-center-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--auto-center-target-object-name")}; return {true, true};
            }
            result.request.auto_center_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--auto-center-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--auto-center-target-unique-id")}; return {true, true};
            }
            result.request.auto_center_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--auto-verb-menu-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--auto-verb-menu-target-object-name")}; return {true, true};
            }
            result.request.auto_verb_menu_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--auto-verb-menu-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--auto-verb-menu-target-unique-id")}; return {true, true};
            }
            result.request.auto_verb_menu_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--bind-controls-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--bind-controls-target-object-name")}; return {true, true};
            }
            result.request.bind_controls_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--bind-controls-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--bind-controls-target-unique-id")}; return {true, true};
            }
            result.request.bind_controls_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--desktop-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--desktop-target-object-name")}; return {true, true};
            }
            result.request.desktop_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--desktop-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--desktop-target-unique-id")}; return {true, true};
            }
            result.request.desktop_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--key-preview-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--key-preview-target-object-name")}; return {true, true};
            }
            result.request.key_preview_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--key-preview-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--key-preview-target-unique-id")}; return {true, true};
            }
            result.request.key_preview_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--mac-desktop-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--mac-desktop-target-object-name")}; return {true, true};
            }
            result.request.mac_desktop_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--mac-desktop-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--mac-desktop-target-unique-id")}; return {true, true};
            }
            result.request.mac_desktop_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--max-button-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--max-button-target-object-name")}; return {true, true};
            }
            result.request.max_button_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--max-button-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--max-button-target-unique-id")}; return {true, true};
            }
            result.request.max_button_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--min-button-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--min-button-target-object-name")}; return {true, true};
            }
            result.request.min_button_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--min-button-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--min-button-target-unique-id")}; return {true, true};
            }
            result.request.min_button_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--min-height-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--min-height-target-object-name")}; return {true, true};
            }
            result.request.min_height_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--min-height-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--min-height-target-unique-id")}; return {true, true};
            }
            result.request.min_height_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--min-width-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--min-width-target-object-name")}; return {true, true};
            }
            result.request.min_width_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--min-width-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--min-width-target-unique-id")}; return {true, true};
            }
            result.request.min_width_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--max-height-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--max-height-target-object-name")}; return {true, true};
            }
            result.request.max_height_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--max-height-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--max-height-target-unique-id")}; return {true, true};
            }
            result.request.max_height_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--movable-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--movable-target-object-name")}; return {true, true};
            }
            result.request.movable_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--movable-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--movable-target-unique-id")}; return {true, true};
            }
            result.request.movable_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--half-height-caption-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--half-height-caption-target-object-name")}; return {true, true};
            }
            result.request.half_height_caption_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--half-height-caption-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--half-height-caption-target-unique-id")}; return {true, true};
            }
            result.request.half_height_caption_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--mdi-form-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--mdi-form-target-object-name")}; return {true, true};
            }
            result.request.mdi_form_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--mdi-form-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--mdi-form-target-unique-id")}; return {true, true};
            }
            result.request.mdi_form_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--max-width-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--max-width-target-object-name")}; return {true, true};
            }
            result.request.max_width_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--max-width-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--max-width-target-unique-id")}; return {true, true};
            }
            result.request.max_width_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--max-left-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--max-left-target-object-name")}; return {true, true};
            }
            result.request.max_left_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--max-left-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--max-left-target-unique-id")}; return {true, true};
            }
            result.request.max_left_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--max-top-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--max-top-target-object-name")}; return {true, true};
            }
            result.request.max_top_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--max-top-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--max-top-target-unique-id")}; return {true, true};
            }
            result.request.max_top_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--auto-size-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--auto-size-target-object-name")}; return {true, true};
            }
            result.request.auto_size_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--auto-size-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--auto-size-target-unique-id")}; return {true, true};
            }
            result.request.auto_size_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--auto-release-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--auto-release-target-object-name")}; return {true, true};
            }
            result.request.auto_release_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--auto-release-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--auto-release-target-unique-id")}; return {true, true};
            }
            result.request.auto_release_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--continuous-scroll-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--continuous-scroll-target-object-name")}; return {true, true};
            }
            result.request.continuous_scroll_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--continuous-scroll-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--continuous-scroll-target-unique-id")}; return {true, true};
            }
            result.request.continuous_scroll_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--dockable-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--dockable-target-object-name")}; return {true, true};
            }
            result.request.dockable_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--dockable-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--dockable-target-unique-id")}; return {true, true};
            }
            result.request.dockable_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--clip-controls-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--clip-controls-target-object-name")}; return {true, true};
            }
            result.request.clip_controls_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--clip-controls-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--clip-controls-target-unique-id")}; return {true, true};
            }
            result.request.clip_controls_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--sparse-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--sparse-target-object-name")}; return {true, true};
            }
            result.request.sparse_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--sparse-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--sparse-target-unique-id")}; return {true, true};
            }
            result.request.sparse_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--lock-screen-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--lock-screen-target-object-name")}; return {true, true};
            }
            result.request.lock_screen_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--lock-screen-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--lock-screen-target-unique-id")}; return {true, true};
            }
            result.request.lock_screen_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--allow-cell-selection-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--allow-cell-selection-target-object-name")}; return {true, true};
            }
            result.request.allow_cell_selection_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--allow-cell-selection-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--allow-cell-selection-target-unique-id")}; return {true, true};
            }
            result.request.allow_cell_selection_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--hide-selection-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--hide-selection-target-object-name")}; return {true, true};
            }
            result.request.hide_selection_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--hide-selection-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--hide-selection-target-unique-id")}; return {true, true};
            }
            result.request.hide_selection_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--delete-mark-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--delete-mark-target-object-name")}; return {true, true};
            }
            result.request.delete_mark_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--delete-mark-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--delete-mark-target-unique-id")}; return {true, true};
            }
            result.request.delete_mark_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--record-mark-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--record-mark-target-object-name")}; return {true, true};
            }
            result.request.record_mark_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--record-mark-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--record-mark-target-unique-id")}; return {true, true};
            }
            result.request.record_mark_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--split-bar-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--split-bar-target-object-name")}; return {true, true};
            }
            result.request.split_bar_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--split-bar-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--split-bar-target-unique-id")}; return {true, true};
            }
            result.request.split_bar_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--highlight-row-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--highlight-row-target-object-name")}; return {true, true};
            }
            result.request.highlight_row_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--highlight-row-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--highlight-row-target-unique-id")}; return {true, true};
            }
            result.request.highlight_row_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--panel-link-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--panel-link-target-object-name")}; return {true, true};
            }
            result.request.panel_link_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--panel-link-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--panel-link-target-unique-id")}; return {true, true};
            }
            result.request.panel_link_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--allow-header-sizing-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--allow-header-sizing-target-object-name")}; return {true, true};
            }
            result.request.allow_header_sizing_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--allow-header-sizing-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--allow-header-sizing-target-unique-id")}; return {true, true};
            }
            result.request.allow_header_sizing_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--allow-row-sizing-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--allow-row-sizing-target-object-name")}; return {true, true};
            }
            result.request.allow_row_sizing_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--allow-row-sizing-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--allow-row-sizing-target-unique-id")}; return {true, true};
            }
            result.request.allow_row_sizing_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--resizable-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--resizable-target-object-name")}; return {true, true};
            }
            result.request.resizable_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--resizable-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--resizable-target-unique-id")}; return {true, true};
            }
            result.request.resizable_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--add-line-feeds-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--add-line-feeds-target-object-name")}; return {true, true};
            }
            result.request.add_line_feeds_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--add-line-feeds-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--add-line-feeds-target-unique-id")}; return {true, true};
            }
            result.request.add_line_feeds_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--always-on-top-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--always-on-top-target-object-name")}; return {true, true};
            }
            result.request.always_on_top_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--always-on-top-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--always-on-top-target-unique-id")}; return {true, true};
            }
            result.request.always_on_top_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--always-on-bottom-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--always-on-bottom-target-object-name")}; return {true, true};
            }
            result.request.always_on_bottom_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--always-on-bottom-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--always-on-bottom-target-unique-id")}; return {true, true};
            }
            result.request.always_on_bottom_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }
    return {false, false};
}

}  // namespace copperfin::studio
