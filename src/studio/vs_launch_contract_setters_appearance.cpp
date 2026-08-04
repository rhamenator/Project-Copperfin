// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "vs_launch_contract_internal.h"

namespace copperfin::studio {

LaunchArgumentDispatchOutcome try_parse_setters_appearance(
    const std::string& argument,
    const localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args,
    std::size_t& index,
    LaunchParseResult& result,
    [[maybe_unused]] std::string& parsed_argument_error) {
if (argument == "--input-mask-object") {
            result.request.input_mask_object = true;
            return {true, false};
        }

if (argument == "--format-object") {
            result.request.format_object = true;
            return {true, false};
        }

if (argument == "--selected-back-color-object") {
            result.request.selected_back_color_object = true;
            return {true, false};
        }

if (argument == "--selected-fore-color-object") {
            result.request.selected_fore_color_object = true;
            return {true, false};
        }

if (argument == "--selected-item-back-color-object") {
            result.request.selected_item_back_color_object = true;
            return {true, false};
        }

if (argument == "--selected-item-fore-color-object") {
            result.request.selected_item_fore_color_object = true;
            return {true, false};
        }

if (argument == "--disabled-item-back-color-object") {
            result.request.disabled_item_back_color_object = true;
            return {true, false};
        }

if (argument == "--disabled-item-fore-color-object") {
            result.request.disabled_item_fore_color_object = true;
            return {true, false};
        }

if (argument == "--item-back-color-object") {
            result.request.item_back_color_object = true;
            return {true, false};
        }

if (argument == "--item-fore-color-object") {
            result.request.item_fore_color_object = true;
            return {true, false};
        }

if (argument == "--highlight-back-color-object") {
            result.request.highlight_back_color_object = true;
            return {true, false};
        }

if (argument == "--highlight-fore-color-object") {
            result.request.highlight_fore_color_object = true;
            return {true, false};
        }

if (argument == "--back-color-object") {
            result.request.back_color_object = true;
            return {true, false};
        }

if (argument == "--fore-color-object") {
            result.request.fore_color_object = true;
            return {true, false};
        }

if (argument == "--disabled-back-color-object") {
            result.request.disabled_back_color_object = true;
            return {true, false};
        }

if (argument == "--disabled-fore-color-object") {
            result.request.disabled_fore_color_object = true;
            return {true, false};
        }

if (argument == "--dynamic-back-color-object") {
            result.request.dynamic_back_color_object = true;
            return {true, false};
        }

if (argument == "--dynamic-fore-color-object") {
            result.request.dynamic_fore_color_object = true;
            return {true, false};
        }

if (argument == "--back-style-object") {
            result.request.back_style_object = true;
            return {true, false};
        }

if (argument == "--border-style-object") {
            result.request.border_style_object = true;
            return {true, false};
        }

if (argument == "--border-width-object") {
            result.request.border_width_object = true;
            return {true, false};
        }

if (argument == "--border-color-object") {
            result.request.border_color_object = true;
            return {true, false};
        }

if (argument == "--special-effect-object") {
            result.request.special_effect_object = true;
            return {true, false};
        }

if (argument == "--scroll-bars-object") {
            result.request.scroll_bars_object = true;
            return {true, false};
        }

if (argument == "--window-state-object") {
            result.request.window_state_object = true;
            return {true, false};
        }

if (argument == "--show-window-object") {
            result.request.show_window_object = true;
            return {true, false};
        }

if (argument == "--title-bar-object") {
            result.request.title_bar_object = true;
            return {true, false};
        }

if (argument == "--mouse-pointer-object") {
            result.request.mouse_pointer_object = true;
            return {true, false};
        }

if (argument == "--picture-margin-object") {
            result.request.picture_margin_object = true;
            return {true, false};
        }

if (argument == "--picture-position-object") {
            result.request.picture_position_object = true;
            return {true, false};
        }

if (argument == "--picture-spacing-object") {
            result.request.picture_spacing_object = true;
            return {true, false};
        }

if (argument == "--picture-selection-display-object") {
            result.request.picture_selection_display_object = true;
            return {true, false};
        }

if (argument == "--dynamic-input-mask-object") {
            result.request.dynamic_input_mask_object = true;
            return {true, false};
        }

if (argument == "--dynamic-line-height-object") {
            result.request.dynamic_line_height_object = true;
            return {true, false};
        }

if (argument == "--dynamic-alignment-object") {
            result.request.dynamic_alignment_object = true;
            return {true, false};
        }

if (argument == "--dynamic-current-control-object") {
            result.request.dynamic_current_control_object = true;
            return {true, false};
        }

if (argument == "--dynamic-font-name-object") {
            result.request.dynamic_font_name_object = true;
            return {true, false};
        }

if (argument == "--dynamic-font-size-object") {
            result.request.dynamic_font_size_object = true;
            return {true, false};
        }

if (argument == "--dynamic-font-bold-object") {
            result.request.dynamic_font_bold_object = true;
            return {true, false};
        }

if (argument == "--dynamic-font-italic-object") {
            result.request.dynamic_font_italic_object = true;
            return {true, false};
        }

if (argument == "--dynamic-font-underline-object") {
            result.request.dynamic_font_underline_object = true;
            return {true, false};
        }

if (argument == "--dynamic-font-strikethru-object") {
            result.request.dynamic_font_strikethru_object = true;
            return {true, false};
        }

if (argument == "--dynamic-font-outline-object") {
            result.request.dynamic_font_outline_object = true;
            return {true, false};
        }

if (argument == "--dynamic-font-shadow-object") {
            result.request.dynamic_font_shadow_object = true;
            return {true, false};
        }

if (argument == "--font-name-object") {
            result.request.font_name_object = true;
            return {true, false};
        }

if (argument == "--font-size-object") {
            result.request.font_size_object = true;
            return {true, false};
        }

if (argument == "--font-bold-object") {
            result.request.font_bold_object = true;
            return {true, false};
        }

if (argument == "--font-italic-object") {
            result.request.font_italic_object = true;
            return {true, false};
        }

if (argument == "--font-underline-object") {
            result.request.font_underline_object = true;
            return {true, false};
        }

if (argument == "--font-strikethru-object") {
            result.request.font_strikethru_object = true;
            return {true, false};
        }

if (argument == "--font-outline-object") {
            result.request.font_outline_object = true;
            return {true, false};
        }

if (argument == "--font-shadow-object") {
            result.request.font_shadow_object = true;
            return {true, false};
        }

if (argument == "--picture-object") {
            result.request.picture_object = true;
            return {true, false};
        }

if (argument == "--down-picture-object") {
            result.request.down_picture_object = true;
            return {true, false};
        }

if (argument == "--disabled-picture-object") {
            result.request.disabled_picture_object = true;
            return {true, false};
        }

if (argument == "--ole-drag-picture-object") {
            result.request.ole_drag_picture_object = true;
            return {true, false};
        }

if (argument == "--mouse-icon-object") {
            result.request.mouse_icon_object = true;
            return {true, false};
        }

if (argument == "--drag-icon-object") {
            result.request.drag_icon_object = true;
            return {true, false};
        }

if (argument == "--drag-mode-object") {
            result.request.drag_mode_object = true;
            return {true, false};
        }

if (argument == "--ole-drag-mode-object") {
            result.request.ole_drag_mode_object = true;
            return {true, false};
        }

if (argument == "--ole-drop-mode-object") {
            result.request.ole_drop_mode_object = true;
            return {true, false};
        }

if (argument == "--ole-drop-effects-object") {
            result.request.ole_drop_effects_object = true;
            return {true, false};
        }

if (argument == "--ole-drop-text-insertion-object") {
            result.request.ole_drop_text_insertion_object = true;
            return {true, false};
        }

if (argument == "--curvature-object") {
            result.request.curvature_object = true;
            return {true, false};
        }

if (argument == "--draw-mode-object") {
            result.request.draw_mode_object = true;
            return {true, false};
        }

if (argument == "--draw-style-object") {
            result.request.draw_style_object = true;
            return {true, false};
        }

if (argument == "--draw-width-object") {
            result.request.draw_width_object = true;
            return {true, false};
        }

if (argument == "--fill-style-object") {
            result.request.fill_style_object = true;
            return {true, false};
        }

if (argument == "--scale-mode-object") {
            result.request.scale_mode_object = true;
            return {true, false};
        }

if (argument == "--buffer-mode-object") {
            result.request.buffer_mode_object = true;
            return {true, false};
        }

if (argument == "--buffer-mode-override-object") {
            result.request.buffer_mode_override_object = true;
            return {true, false};
        }

if (argument == "--grid-line-color-object") {
            result.request.grid_line_color_object = true;
            return {true, false};
        }

if (argument == "--header-height-object") {
            result.request.header_height_object = true;
            return {true, false};
        }

if (argument == "--row-height-object") {
            result.request.row_height_object = true;
            return {true, false};
        }

if (argument == "--grid-line-width-object") {
            result.request.grid_line_width_object = true;
            return {true, false};
        }

if (argument == "--grid-lines-object") {
            result.request.grid_lines_object = true;
            return {true, false};
        }

if (argument == "--highlight-row-line-width-object") {
            result.request.highlight_row_line_width_object = true;
            return {true, false};
        }

if (argument == "--highlight-style-object") {
            result.request.highlight_style_object = true;
            return {true, false};
        }

if (argument == "--fill-color-object") {
            result.request.fill_color_object = true;
            return {true, false};
        }

if (argument == "--picture") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--picture")}; return {true, true};
            }
            result.request.picture = args[++index];
            result.request.picture_available = true;
            return {true, false};
        }

if (argument == "--down-picture") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--down-picture")}; return {true, true};
            }
            result.request.down_picture = args[++index];
            result.request.down_picture_available = true;
            return {true, false};
        }

if (argument == "--disabled-picture") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--disabled-picture")}; return {true, true};
            }
            result.request.disabled_picture = args[++index];
            result.request.disabled_picture_available = true;
            return {true, false};
        }

if (argument == "--ole-drag-picture") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--ole-drag-picture")}; return {true, true};
            }
            result.request.ole_drag_picture = args[++index];
            result.request.ole_drag_picture_available = true;
            return {true, false};
        }

if (argument == "--mouse-icon") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--mouse-icon")}; return {true, true};
            }
            result.request.mouse_icon = args[++index];
            result.request.mouse_icon_available = true;
            return {true, false};
        }

if (argument == "--drag-icon") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--drag-icon")}; return {true, true};
            }
            result.request.drag_icon = args[++index];
            result.request.drag_icon_available = true;
            return {true, false};
        }

if (argument == "--input-mask") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--input-mask")}; return {true, true};
            }
            result.request.input_mask = args[++index];
            result.request.input_mask_available = true;
            return {true, false};
        }

if (argument == "--format") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--format")}; return {true, true};
            }
            result.request.format = args[++index];
            result.request.format_available = true;
            return {true, false};
        }

if (argument == "--drag-mode") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--drag-mode")}; return {true, true};
            }
            int drag_mode = 0;
            if (!parse_int_value(args[++index], drag_mode)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--drag-mode")}; return {true, true};
            }
            if (drag_mode < 0) {
                result = {.ok = false, .error = localized_non_negative_value_required(catalog, "--drag-mode")}; return {true, true};
            }
            result.request.drag_mode = drag_mode;
            result.request.drag_mode_available = true;
            return {true, false};
        }

if (argument == "--ole-drag-mode") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--ole-drag-mode")}; return {true, true};
            }
            int ole_drag_mode = 0;
            if (!parse_int_value(args[++index], ole_drag_mode)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--ole-drag-mode")}; return {true, true};
            }
            if (ole_drag_mode < 0) {
                result = {.ok = false, .error = localized_non_negative_value_required(catalog, "--ole-drag-mode")}; return {true, true};
            }
            result.request.ole_drag_mode = ole_drag_mode;
            result.request.ole_drag_mode_available = true;
            return {true, false};
        }

if (argument == "--ole-drop-mode") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--ole-drop-mode")}; return {true, true};
            }
            int ole_drop_mode = 0;
            if (!parse_int_value(args[++index], ole_drop_mode)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--ole-drop-mode")}; return {true, true};
            }
            if (ole_drop_mode < 0) {
                result = {.ok = false, .error = localized_non_negative_value_required(catalog, "--ole-drop-mode")}; return {true, true};
            }
            result.request.ole_drop_mode = ole_drop_mode;
            result.request.ole_drop_mode_available = true;
            return {true, false};
        }

if (argument == "--ole-drop-effects") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--ole-drop-effects")}; return {true, true};
            }
            int ole_drop_effects = 0;
            if (!parse_int_value(args[++index], ole_drop_effects)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--ole-drop-effects")}; return {true, true};
            }
            if (ole_drop_effects < 0) {
                result = {.ok = false, .error = localized_non_negative_value_required(catalog, "--ole-drop-effects")}; return {true, true};
            }
            result.request.ole_drop_effects = ole_drop_effects;
            result.request.ole_drop_effects_available = true;
            return {true, false};
        }

if (argument == "--ole-drop-text-insertion") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--ole-drop-text-insertion")}; return {true, true};
            }
            int ole_drop_text_insertion = 0;
            if (!parse_int_value(args[++index], ole_drop_text_insertion)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--ole-drop-text-insertion")}; return {true, true};
            }
            if (ole_drop_text_insertion < 0) {
                result = {.ok = false, .error = localized_non_negative_value_required(catalog, "--ole-drop-text-insertion")}; return {true, true};
            }
            result.request.ole_drop_text_insertion = ole_drop_text_insertion;
            result.request.ole_drop_text_insertion_available = true;
            return {true, false};
        }

if (argument == "--curvature") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--curvature")}; return {true, true};
            }
            int curvature = 0;
            if (!parse_int_value(args[++index], curvature)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--curvature")}; return {true, true};
            }
            if (curvature < 0) {
                result = {.ok = false, .error = localized_non_negative_value_required(catalog, "--curvature")}; return {true, true};
            }
            result.request.curvature = curvature;
            result.request.curvature_available = true;
            return {true, false};
        }

if (argument == "--draw-mode") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--draw-mode")}; return {true, true};
            }
            int draw_mode = 0;
            if (!parse_int_value(args[++index], draw_mode)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--draw-mode")}; return {true, true};
            }
            if (draw_mode < 0) {
                result = {.ok = false, .error = localized_non_negative_value_required(catalog, "--draw-mode")}; return {true, true};
            }
            result.request.draw_mode = draw_mode;
            result.request.draw_mode_available = true;
            return {true, false};
        }

if (argument == "--draw-style") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--draw-style")}; return {true, true};
            }
            int draw_style = 0;
            if (!parse_int_value(args[++index], draw_style)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--draw-style")}; return {true, true};
            }
            if (draw_style < 0) {
                result = {.ok = false, .error = localized_non_negative_value_required(catalog, "--draw-style")}; return {true, true};
            }
            result.request.draw_style = draw_style;
            result.request.draw_style_available = true;
            return {true, false};
        }

if (argument == "--draw-width") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--draw-width")}; return {true, true};
            }
            int draw_width = 0;
            if (!parse_int_value(args[++index], draw_width)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--draw-width")}; return {true, true};
            }
            if (draw_width < 0) {
                result = {.ok = false, .error = localized_non_negative_value_required(catalog, "--draw-width")}; return {true, true};
            }
            result.request.draw_width = draw_width;
            result.request.draw_width_available = true;
            return {true, false};
        }

if (argument == "--fill-style") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--fill-style")}; return {true, true};
            }
            int fill_style = 0;
            if (!parse_int_value(args[++index], fill_style)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--fill-style")}; return {true, true};
            }
            if (fill_style < 0) {
                result = {.ok = false, .error = localized_non_negative_value_required(catalog, "--fill-style")}; return {true, true};
            }
            result.request.fill_style = fill_style;
            result.request.fill_style_available = true;
            return {true, false};
        }

if (argument == "--scale-mode") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--scale-mode")}; return {true, true};
            }
            int scale_mode = 0;
            if (!parse_int_value(args[++index], scale_mode)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--scale-mode")}; return {true, true};
            }
            if (scale_mode < 0) {
                result = {.ok = false, .error = localized_non_negative_value_required(catalog, "--scale-mode")}; return {true, true};
            }
            result.request.scale_mode = scale_mode;
            result.request.scale_mode_available = true;
            return {true, false};
        }

if (argument == "--buffer-mode") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--buffer-mode")}; return {true, true};
            }
            int buffer_mode = 0;
            if (!parse_int_value(args[++index], buffer_mode)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--buffer-mode")}; return {true, true};
            }
            if (buffer_mode < 0) {
                result = {.ok = false, .error = localized_non_negative_value_required(catalog, "--buffer-mode")}; return {true, true};
            }
            result.request.buffer_mode = buffer_mode;
            result.request.buffer_mode_available = true;
            return {true, false};
        }

if (argument == "--buffer-mode-override") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--buffer-mode-override")}; return {true, true};
            }
            int buffer_mode_override = 0;
            if (!parse_int_value(args[++index], buffer_mode_override)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--buffer-mode-override")}; return {true, true};
            }
            if (buffer_mode_override < 0) {
                result = {.ok = false, .error = localized_non_negative_value_required(catalog, "--buffer-mode-override")}; return {true, true};
            }
            result.request.buffer_mode_override = buffer_mode_override;
            result.request.buffer_mode_override_available = true;
            return {true, false};
        }

if (argument == "--grid-line-color") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--grid-line-color")}; return {true, true};
            }
            int grid_line_color = 0;
            if (!parse_int_value(args[++index], grid_line_color)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--grid-line-color")}; return {true, true};
            }
            if (grid_line_color < 0) {
                result = {.ok = false, .error = localized_non_negative_value_required(catalog, "--grid-line-color")}; return {true, true};
            }
            result.request.grid_line_color = grid_line_color;
            result.request.grid_line_color_available = true;
            return {true, false};
        }

if (argument == "--header-height") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--header-height")}; return {true, true};
            }
            int header_height = 0;
            if (!parse_int_value(args[++index], header_height)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--header-height")}; return {true, true};
            }
            if (header_height < 0) {
                result = {.ok = false, .error = localized_non_negative_value_required(catalog, "--header-height")}; return {true, true};
            }
            result.request.header_height = header_height;
            result.request.header_height_available = true;
            return {true, false};
        }

if (argument == "--row-height") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--row-height")}; return {true, true};
            }
            int row_height = 0;
            if (!parse_int_value(args[++index], row_height)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--row-height")}; return {true, true};
            }
            if (row_height < 0) {
                result = {.ok = false, .error = localized_non_negative_value_required(catalog, "--row-height")}; return {true, true};
            }
            result.request.row_height = row_height;
            result.request.row_height_available = true;
            return {true, false};
        }

if (argument == "--grid-line-width") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--grid-line-width")}; return {true, true};
            }
            int grid_line_width = 0;
            if (!parse_int_value(args[++index], grid_line_width)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--grid-line-width")}; return {true, true};
            }
            if (grid_line_width < 0) {
                result = {.ok = false, .error = localized_non_negative_value_required(catalog, "--grid-line-width")}; return {true, true};
            }
            result.request.grid_line_width = grid_line_width;
            result.request.grid_line_width_available = true;
            return {true, false};
        }

if (argument == "--grid-lines") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--grid-lines")}; return {true, true};
            }
            int grid_lines = 0;
            if (!parse_int_value(args[++index], grid_lines)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--grid-lines")}; return {true, true};
            }
            if (grid_lines < 0) {
                result = {.ok = false, .error = localized_non_negative_value_required(catalog, "--grid-lines")}; return {true, true};
            }
            result.request.grid_lines = grid_lines;
            result.request.grid_lines_available = true;
            return {true, false};
        }

if (argument == "--highlight-row-line-width") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--highlight-row-line-width")}; return {true, true};
            }
            int highlight_row_line_width = 0;
            if (!parse_int_value(args[++index], highlight_row_line_width)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--highlight-row-line-width")}; return {true, true};
            }
            if (highlight_row_line_width < 0) {
                result = {.ok = false, .error = localized_non_negative_value_required(catalog, "--highlight-row-line-width")}; return {true, true};
            }
            result.request.highlight_row_line_width = highlight_row_line_width;
            result.request.highlight_row_line_width_available = true;
            return {true, false};
        }

if (argument == "--highlight-style") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--highlight-style")}; return {true, true};
            }
            int highlight_style = 0;
            if (!parse_int_value(args[++index], highlight_style)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--highlight-style")}; return {true, true};
            }
            if (highlight_style < 0) {
                result = {.ok = false, .error = localized_non_negative_value_required(catalog, "--highlight-style")}; return {true, true};
            }
            result.request.highlight_style = highlight_style;
            result.request.highlight_style_available = true;
            return {true, false};
        }

if (argument == "--fill-color") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--fill-color")}; return {true, true};
            }
            int fill_color = 0;
            if (!parse_int_value(args[++index], fill_color)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--fill-color")}; return {true, true};
            }
            if (fill_color < 0) {
                result = {.ok = false, .error = localized_non_negative_value_required(catalog, "--fill-color")}; return {true, true};
            }
            result.request.fill_color = fill_color;
            result.request.fill_color_available = true;
            return {true, false};
        }

if (argument == "--dynamic-back-color") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--dynamic-back-color")}; return {true, true};
            }
            result.request.dynamic_back_color = args[++index];
            result.request.dynamic_back_color_available = true;
            return {true, false};
        }

if (argument == "--dynamic-fore-color") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--dynamic-fore-color")}; return {true, true};
            }
            result.request.dynamic_fore_color = args[++index];
            result.request.dynamic_fore_color_available = true;
            return {true, false};
        }

if (argument == "--back-style") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--back-style")}; return {true, true};
            }
            int back_style = 0;
            if (!parse_int_value(args[++index], back_style)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--back-style")}; return {true, true};
            }
            if (back_style < 0) {
                result = {.ok = false, .error = localized_not_negative_value_required(catalog, "--back-style")}; return {true, true};
            }
            result.request.back_style = back_style;
            result.request.back_style_available = true;
            return {true, false};
        }

if (argument == "--border-style") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--border-style")}; return {true, true};
            }
            int border_style = 0;
            if (!parse_int_value(args[++index], border_style)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--border-style")}; return {true, true};
            }
            if (border_style < 0) {
                result = {.ok = false, .error = localized_not_negative_value_required(catalog, "--border-style")}; return {true, true};
            }
            result.request.border_style = border_style;
            result.request.border_style_available = true;
            return {true, false};
        }

if (argument == "--border-width") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--border-width")}; return {true, true};
            }
            int border_width = 0;
            if (!parse_int_value(args[++index], border_width)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--border-width")}; return {true, true};
            }
            if (border_width < 0) {
                result = {.ok = false, .error = localized_not_negative_value_required(catalog, "--border-width")}; return {true, true};
            }
            result.request.border_width = border_width;
            result.request.border_width_available = true;
            return {true, false};
        }

if (argument == "--border-color") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--border-color")}; return {true, true};
            }
            int border_color = 0;
            if (!parse_int_value(args[++index], border_color)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--border-color")}; return {true, true};
            }
            if (border_color < 0) {
                result = {.ok = false, .error = localized_not_negative_value_required(catalog, "--border-color")}; return {true, true};
            }
            result.request.border_color = border_color;
            result.request.border_color_available = true;
            return {true, false};
        }

if (argument == "--special-effect") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--special-effect")}; return {true, true};
            }
            int special_effect = 0;
            if (!parse_int_value(args[++index], special_effect)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--special-effect")}; return {true, true};
            }
            if (special_effect < 0) {
                result = {.ok = false, .error = localized_not_negative_value_required(catalog, "--special-effect")}; return {true, true};
            }
            result.request.special_effect = special_effect;
            result.request.special_effect_available = true;
            return {true, false};
        }

if (argument == "--scroll-bars") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--scroll-bars")}; return {true, true};
            }
            int scroll_bars = 0;
            if (!parse_int_value(args[++index], scroll_bars)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--scroll-bars")}; return {true, true};
            }
            if (scroll_bars < 0) {
                result = {.ok = false, .error = localized_not_negative_value_required(catalog, "--scroll-bars")}; return {true, true};
            }
            result.request.scroll_bars = scroll_bars;
            result.request.scroll_bars_available = true;
            return {true, false};
        }

if (argument == "--window-state") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--window-state")}; return {true, true};
            }
            int window_state = 0;
            if (!parse_int_value(args[++index], window_state)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--window-state")}; return {true, true};
            }
            if (window_state < 0) {
                result = {.ok = false, .error = localized_not_negative_value_required(catalog, "--window-state")}; return {true, true};
            }
            result.request.window_state = window_state;
            result.request.window_state_available = true;
            return {true, false};
        }

if (argument == "--show-window") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--show-window")}; return {true, true};
            }
            int show_window = 0;
            if (!parse_int_value(args[++index], show_window)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--show-window")}; return {true, true};
            }
            if (show_window < 0) {
                result = {.ok = false, .error = localized_not_negative_value_required(catalog, "--show-window")}; return {true, true};
            }
            result.request.show_window = show_window;
            result.request.show_window_available = true;
            return {true, false};
        }

if (argument == "--title-bar") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--title-bar")}; return {true, true};
            }
            int title_bar = 0;
            if (!parse_int_value(args[++index], title_bar)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--title-bar")}; return {true, true};
            }
            if (title_bar < 0) {
                result = {.ok = false, .error = localized_not_negative_value_required(catalog, "--title-bar")}; return {true, true};
            }
            result.request.title_bar = title_bar;
            result.request.title_bar_available = true;
            return {true, false};
        }

if (argument == "--mouse-pointer") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--mouse-pointer")}; return {true, true};
            }
            int mouse_pointer = 0;
            if (!parse_int_value(args[++index], mouse_pointer)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--mouse-pointer")}; return {true, true};
            }
            if (mouse_pointer < 0) {
                result = {.ok = false, .error = localized_not_negative_value_required(catalog, "--mouse-pointer")}; return {true, true};
            }
            result.request.mouse_pointer = mouse_pointer;
            result.request.mouse_pointer_available = true;
            return {true, false};
        }

if (argument == "--picture-margin") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--picture-margin")}; return {true, true};
            }
            int picture_margin = 0;
            if (!parse_int_value(args[++index], picture_margin)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--picture-margin")}; return {true, true};
            }
            if (picture_margin < 0) {
                result = {.ok = false, .error = localized_not_negative_value_required(catalog, "--picture-margin")}; return {true, true};
            }
            result.request.picture_margin = picture_margin;
            result.request.picture_margin_available = true;
            return {true, false};
        }

if (argument == "--picture-position") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--picture-position")}; return {true, true};
            }
            int picture_position = 0;
            if (!parse_int_value(args[++index], picture_position)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--picture-position")}; return {true, true};
            }
            if (picture_position < 0) {
                result = {.ok = false, .error = localized_not_negative_value_required(catalog, "--picture-position")}; return {true, true};
            }
            result.request.picture_position = picture_position;
            result.request.picture_position_available = true;
            return {true, false};
        }

if (argument == "--picture-spacing") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--picture-spacing")}; return {true, true};
            }
            int picture_spacing = 0;
            if (!parse_int_value(args[++index], picture_spacing)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--picture-spacing")}; return {true, true};
            }
            if (picture_spacing < 0) {
                result = {.ok = false, .error = localized_not_negative_value_required(catalog, "--picture-spacing")}; return {true, true};
            }
            result.request.picture_spacing = picture_spacing;
            result.request.picture_spacing_available = true;
            return {true, false};
        }

if (argument == "--picture-selection-display") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--picture-selection-display")}; return {true, true};
            }
            int picture_selection_display = 0;
            if (!parse_int_value(args[++index], picture_selection_display)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--picture-selection-display")}; return {true, true};
            }
            if (picture_selection_display < 0) {
                result = {.ok = false, .error = localized_not_negative_value_required(catalog, "--picture-selection-display")}; return {true, true};
            }
            result.request.picture_selection_display = picture_selection_display;
            result.request.picture_selection_display_available = true;
            return {true, false};
        }

if (argument == "--dynamic-input-mask") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--dynamic-input-mask")}; return {true, true};
            }
            result.request.dynamic_input_mask = args[++index];
            result.request.dynamic_input_mask_available = true;
            return {true, false};
        }

if (argument == "--dynamic-line-height") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--dynamic-line-height")}; return {true, true};
            }
            result.request.dynamic_line_height = args[++index];
            result.request.dynamic_line_height_available = true;
            return {true, false};
        }

if (argument == "--dynamic-alignment") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--dynamic-alignment")}; return {true, true};
            }
            result.request.dynamic_alignment = args[++index];
            result.request.dynamic_alignment_available = true;
            return {true, false};
        }

if (argument == "--dynamic-current-control") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--dynamic-current-control")}; return {true, true};
            }
            result.request.dynamic_current_control = args[++index];
            result.request.dynamic_current_control_available = true;
            return {true, false};
        }

if (argument == "--dynamic-font-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--dynamic-font-name")}; return {true, true};
            }
            result.request.dynamic_font_name = args[++index];
            result.request.dynamic_font_name_available = true;
            return {true, false};
        }

if (argument == "--dynamic-font-size") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--dynamic-font-size")}; return {true, true};
            }
            result.request.dynamic_font_size = args[++index];
            result.request.dynamic_font_size_available = true;
            return {true, false};
        }

if (argument == "--dynamic-font-bold") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--dynamic-font-bold")}; return {true, true};
            }
            result.request.dynamic_font_bold = args[++index];
            result.request.dynamic_font_bold_available = true;
            return {true, false};
        }

if (argument == "--dynamic-font-italic") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--dynamic-font-italic")}; return {true, true};
            }
            result.request.dynamic_font_italic = args[++index];
            result.request.dynamic_font_italic_available = true;
            return {true, false};
        }

if (argument == "--dynamic-font-underline") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--dynamic-font-underline")}; return {true, true};
            }
            result.request.dynamic_font_underline = args[++index];
            result.request.dynamic_font_underline_available = true;
            return {true, false};
        }

if (argument == "--dynamic-font-strikethru") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--dynamic-font-strikethru")}; return {true, true};
            }
            result.request.dynamic_font_strikethru = args[++index];
            result.request.dynamic_font_strikethru_available = true;
            return {true, false};
        }

if (argument == "--dynamic-font-outline") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--dynamic-font-outline")}; return {true, true};
            }
            result.request.dynamic_font_outline = args[++index];
            result.request.dynamic_font_outline_available = true;
            return {true, false};
        }

if (argument == "--dynamic-font-shadow") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--dynamic-font-shadow")}; return {true, true};
            }
            result.request.dynamic_font_shadow = args[++index];
            result.request.dynamic_font_shadow_available = true;
            return {true, false};
        }

if (argument == "--font-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--font-name")}; return {true, true};
            }
            result.request.font_name = args[++index];
            result.request.font_name_available = true;
            return {true, false};
        }

if (argument == "--font-size") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--font-size")}; return {true, true};
            }
            double font_size = 0.0;
            if (!parse_double_value(args[++index], font_size) || !std::isfinite(font_size)) {
                result = {.ok = false, .error = localized_numeric_value_required(catalog, "--font-size")}; return {true, true};
            }
            if (font_size < 0.0) {
                result = {.ok = false, .error = localized_not_negative_value_required(catalog, "--font-size")}; return {true, true};
            }
            result.request.font_size = font_size;
            result.request.font_size_available = true;
            return {true, false};
        }

if (argument == "--font-bold") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--font-bold")}; return {true, true};
            }
            const auto font_bold = parse_bool_value(args[++index]);
            if (!font_bold.has_value()) {
                result = {.ok = false, .error = localized_true_false_value_required(catalog, "--font-bold")}; return {true, true};
            }
            result.request.font_bold = *font_bold;
            result.request.font_bold_available = true;
            return {true, false};
        }

if (argument == "--font-italic") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--font-italic")}; return {true, true};
            }
            const auto font_italic = parse_bool_value(args[++index]);
            if (!font_italic.has_value()) {
                result = {.ok = false, .error = localized_true_false_value_required(catalog, "--font-italic")}; return {true, true};
            }
            result.request.font_italic = *font_italic;
            result.request.font_italic_available = true;
            return {true, false};
        }

if (argument == "--font-underline") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--font-underline")}; return {true, true};
            }
            const auto font_underline = parse_bool_value(args[++index]);
            if (!font_underline.has_value()) {
                result = {.ok = false, .error = localized_true_false_value_required(catalog, "--font-underline")}; return {true, true};
            }
            result.request.font_underline = *font_underline;
            result.request.font_underline_available = true;
            return {true, false};
        }

if (argument == "--font-strikethru") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--font-strikethru")}; return {true, true};
            }
            const auto font_strikethru = parse_bool_value(args[++index]);
            if (!font_strikethru.has_value()) {
                result = {.ok = false, .error = localized_true_false_value_required(catalog, "--font-strikethru")}; return {true, true};
            }
            result.request.font_strikethru = *font_strikethru;
            result.request.font_strikethru_available = true;
            return {true, false};
        }

if (argument == "--font-outline") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--font-outline")}; return {true, true};
            }
            const auto font_outline = parse_bool_value(args[++index]);
            if (!font_outline.has_value()) {
                result = {.ok = false, .error = localized_true_false_value_required(catalog, "--font-outline")}; return {true, true};
            }
            result.request.font_outline = *font_outline;
            result.request.font_outline_available = true;
            return {true, false};
        }

if (argument == "--font-shadow") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--font-shadow")}; return {true, true};
            }
            const auto font_shadow = parse_bool_value(args[++index]);
            if (!font_shadow.has_value()) {
                result = {.ok = false, .error = localized_true_false_value_required(catalog, "--font-shadow")}; return {true, true};
            }
            result.request.font_shadow = *font_shadow;
            result.request.font_shadow_available = true;
            return {true, false};
        }

if (argument == "--selected-back-color") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--selected-back-color")}; return {true, true};
            }
            int selected_back_color = 0;
            if (!parse_int_value(args[++index], selected_back_color)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--selected-back-color")}; return {true, true};
            }
            result.request.selected_back_color = selected_back_color;
            result.request.selected_back_color_available = true;
            return {true, false};
        }

if (argument == "--selected-fore-color") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--selected-fore-color")}; return {true, true};
            }
            int selected_fore_color = 0;
            if (!parse_int_value(args[++index], selected_fore_color)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--selected-fore-color")}; return {true, true};
            }
            result.request.selected_fore_color = selected_fore_color;
            result.request.selected_fore_color_available = true;
            return {true, false};
        }

if (argument == "--selected-item-back-color") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--selected-item-back-color")}; return {true, true};
            }
            int selected_item_back_color = 0;
            if (!parse_int_value(args[++index], selected_item_back_color)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--selected-item-back-color")}; return {true, true};
            }
            result.request.selected_item_back_color = selected_item_back_color;
            result.request.selected_item_back_color_available = true;
            return {true, false};
        }

if (argument == "--selected-item-fore-color") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--selected-item-fore-color")}; return {true, true};
            }
            int selected_item_fore_color = 0;
            if (!parse_int_value(args[++index], selected_item_fore_color)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--selected-item-fore-color")}; return {true, true};
            }
            result.request.selected_item_fore_color = selected_item_fore_color;
            result.request.selected_item_fore_color_available = true;
            return {true, false};
        }

if (argument == "--disabled-item-back-color") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--disabled-item-back-color")}; return {true, true};
            }
            int disabled_item_back_color = 0;
            if (!parse_int_value(args[++index], disabled_item_back_color)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--disabled-item-back-color")}; return {true, true};
            }
            result.request.disabled_item_back_color = disabled_item_back_color;
            result.request.disabled_item_back_color_available = true;
            return {true, false};
        }

if (argument == "--disabled-item-fore-color") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--disabled-item-fore-color")}; return {true, true};
            }
            int disabled_item_fore_color = 0;
            if (!parse_int_value(args[++index], disabled_item_fore_color)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--disabled-item-fore-color")}; return {true, true};
            }
            result.request.disabled_item_fore_color = disabled_item_fore_color;
            result.request.disabled_item_fore_color_available = true;
            return {true, false};
        }

if (argument == "--item-back-color") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--item-back-color")}; return {true, true};
            }
            int item_back_color = 0;
            if (!parse_int_value(args[++index], item_back_color)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--item-back-color")}; return {true, true};
            }
            result.request.item_back_color = item_back_color;
            result.request.item_back_color_available = true;
            return {true, false};
        }

if (argument == "--item-fore-color") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--item-fore-color")}; return {true, true};
            }
            int item_fore_color = 0;
            if (!parse_int_value(args[++index], item_fore_color)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--item-fore-color")}; return {true, true};
            }
            result.request.item_fore_color = item_fore_color;
            result.request.item_fore_color_available = true;
            return {true, false};
        }

if (argument == "--highlight-back-color") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--highlight-back-color")}; return {true, true};
            }
            int highlight_back_color = 0;
            if (!parse_int_value(args[++index], highlight_back_color)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--highlight-back-color")}; return {true, true};
            }
            result.request.highlight_back_color = highlight_back_color;
            result.request.highlight_back_color_available = true;
            return {true, false};
        }

if (argument == "--highlight-fore-color") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--highlight-fore-color")}; return {true, true};
            }
            int highlight_fore_color = 0;
            if (!parse_int_value(args[++index], highlight_fore_color)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--highlight-fore-color")}; return {true, true};
            }
            result.request.highlight_fore_color = highlight_fore_color;
            result.request.highlight_fore_color_available = true;
            return {true, false};
        }

if (argument == "--back-color") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--back-color")}; return {true, true};
            }
            int back_color = 0;
            if (!parse_int_value(args[++index], back_color)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--back-color")}; return {true, true};
            }
            result.request.back_color = back_color;
            result.request.back_color_available = true;
            return {true, false};
        }

if (argument == "--fore-color") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--fore-color")}; return {true, true};
            }
            int fore_color = 0;
            if (!parse_int_value(args[++index], fore_color)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--fore-color")}; return {true, true};
            }
            result.request.fore_color = fore_color;
            result.request.fore_color_available = true;
            return {true, false};
        }

if (argument == "--disabled-back-color") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--disabled-back-color")}; return {true, true};
            }
            int disabled_back_color = 0;
            if (!parse_int_value(args[++index], disabled_back_color)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--disabled-back-color")}; return {true, true};
            }
            result.request.disabled_back_color = disabled_back_color;
            result.request.disabled_back_color_available = true;
            return {true, false};
        }

if (argument == "--disabled-fore-color") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--disabled-fore-color")}; return {true, true};
            }
            int disabled_fore_color = 0;
            if (!parse_int_value(args[++index], disabled_fore_color)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--disabled-fore-color")}; return {true, true};
            }
            result.request.disabled_fore_color = disabled_fore_color;
            result.request.disabled_fore_color_available = true;
            return {true, false};
        }

if (argument == "--picture-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--picture-target-object-name")}; return {true, true};
            }
            result.request.picture_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--picture-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--picture-target-unique-id")}; return {true, true};
            }
            result.request.picture_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--down-picture-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--down-picture-target-object-name")}; return {true, true};
            }
            result.request.down_picture_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--down-picture-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--down-picture-target-unique-id")}; return {true, true};
            }
            result.request.down_picture_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--disabled-picture-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--disabled-picture-target-object-name")}; return {true, true};
            }
            result.request.disabled_picture_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--disabled-picture-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--disabled-picture-target-unique-id")}; return {true, true};
            }
            result.request.disabled_picture_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--ole-drag-picture-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--ole-drag-picture-target-object-name")}; return {true, true};
            }
            result.request.ole_drag_picture_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--ole-drag-picture-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--ole-drag-picture-target-unique-id")}; return {true, true};
            }
            result.request.ole_drag_picture_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--mouse-icon-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--mouse-icon-target-object-name")}; return {true, true};
            }
            result.request.mouse_icon_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--mouse-icon-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--mouse-icon-target-unique-id")}; return {true, true};
            }
            result.request.mouse_icon_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--drag-icon-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--drag-icon-target-object-name")}; return {true, true};
            }
            result.request.drag_icon_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--drag-icon-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--drag-icon-target-unique-id")}; return {true, true};
            }
            result.request.drag_icon_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--drag-mode-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--drag-mode-target-object-name")}; return {true, true};
            }
            result.request.drag_mode_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--drag-mode-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--drag-mode-target-unique-id")}; return {true, true};
            }
            result.request.drag_mode_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--ole-drag-mode-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--ole-drag-mode-target-object-name")}; return {true, true};
            }
            result.request.ole_drag_mode_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--ole-drag-mode-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--ole-drag-mode-target-unique-id")}; return {true, true};
            }
            result.request.ole_drag_mode_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--ole-drop-mode-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--ole-drop-mode-target-object-name")}; return {true, true};
            }
            result.request.ole_drop_mode_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--ole-drop-mode-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--ole-drop-mode-target-unique-id")}; return {true, true};
            }
            result.request.ole_drop_mode_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--ole-drop-effects-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--ole-drop-effects-target-object-name")}; return {true, true};
            }
            result.request.ole_drop_effects_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--ole-drop-effects-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--ole-drop-effects-target-unique-id")}; return {true, true};
            }
            result.request.ole_drop_effects_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--ole-drop-text-insertion-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--ole-drop-text-insertion-target-object-name")}; return {true, true};
            }
            result.request.ole_drop_text_insertion_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--ole-drop-text-insertion-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--ole-drop-text-insertion-target-unique-id")}; return {true, true};
            }
            result.request.ole_drop_text_insertion_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--curvature-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--curvature-target-object-name")}; return {true, true};
            }
            result.request.curvature_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--curvature-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--curvature-target-unique-id")}; return {true, true};
            }
            result.request.curvature_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--draw-mode-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--draw-mode-target-object-name")}; return {true, true};
            }
            result.request.draw_mode_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--draw-mode-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--draw-mode-target-unique-id")}; return {true, true};
            }
            result.request.draw_mode_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--draw-style-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--draw-style-target-object-name")}; return {true, true};
            }
            result.request.draw_style_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--draw-style-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--draw-style-target-unique-id")}; return {true, true};
            }
            result.request.draw_style_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--draw-width-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--draw-width-target-object-name")}; return {true, true};
            }
            result.request.draw_width_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--draw-width-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--draw-width-target-unique-id")}; return {true, true};
            }
            result.request.draw_width_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--fill-style-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--fill-style-target-object-name")}; return {true, true};
            }
            result.request.fill_style_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--fill-style-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--fill-style-target-unique-id")}; return {true, true};
            }
            result.request.fill_style_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--scale-mode-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--scale-mode-target-object-name")}; return {true, true};
            }
            result.request.scale_mode_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--scale-mode-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--scale-mode-target-unique-id")}; return {true, true};
            }
            result.request.scale_mode_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--buffer-mode-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--buffer-mode-target-object-name")}; return {true, true};
            }
            result.request.buffer_mode_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--buffer-mode-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--buffer-mode-target-unique-id")}; return {true, true};
            }
            result.request.buffer_mode_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--buffer-mode-override-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--buffer-mode-override-target-object-name")}; return {true, true};
            }
            result.request.buffer_mode_override_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--buffer-mode-override-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--buffer-mode-override-target-unique-id")}; return {true, true};
            }
            result.request.buffer_mode_override_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--grid-line-color-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--grid-line-color-target-object-name")}; return {true, true};
            }
            result.request.grid_line_color_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--grid-line-color-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--grid-line-color-target-unique-id")}; return {true, true};
            }
            result.request.grid_line_color_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--header-height-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--header-height-target-object-name")}; return {true, true};
            }
            result.request.header_height_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--header-height-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--header-height-target-unique-id")}; return {true, true};
            }
            result.request.header_height_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--row-height-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--row-height-target-object-name")}; return {true, true};
            }
            result.request.row_height_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--row-height-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--row-height-target-unique-id")}; return {true, true};
            }
            result.request.row_height_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--grid-line-width-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--grid-line-width-target-object-name")}; return {true, true};
            }
            result.request.grid_line_width_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--grid-line-width-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--grid-line-width-target-unique-id")}; return {true, true};
            }
            result.request.grid_line_width_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--grid-lines-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--grid-lines-target-object-name")}; return {true, true};
            }
            result.request.grid_lines_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--grid-lines-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--grid-lines-target-unique-id")}; return {true, true};
            }
            result.request.grid_lines_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--highlight-row-line-width-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--highlight-row-line-width-target-object-name")}; return {true, true};
            }
            result.request.highlight_row_line_width_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--highlight-row-line-width-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--highlight-row-line-width-target-unique-id")}; return {true, true};
            }
            result.request.highlight_row_line_width_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--highlight-style-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--highlight-style-target-object-name")}; return {true, true};
            }
            result.request.highlight_style_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--highlight-style-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--highlight-style-target-unique-id")}; return {true, true};
            }
            result.request.highlight_style_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--fill-color-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--fill-color-target-object-name")}; return {true, true};
            }
            result.request.fill_color_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--fill-color-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--fill-color-target-unique-id")}; return {true, true};
            }
            result.request.fill_color_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--input-mask-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--input-mask-target-object-name")}; return {true, true};
            }
            result.request.input_mask_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--input-mask-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--input-mask-target-unique-id")}; return {true, true};
            }
            result.request.input_mask_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--format-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--format-target-object-name")}; return {true, true};
            }
            result.request.format_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--format-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--format-target-unique-id")}; return {true, true};
            }
            result.request.format_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--selected-back-color-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--selected-back-color-target-object-name")}; return {true, true};
            }
            result.request.selected_back_color_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--selected-back-color-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--selected-back-color-target-unique-id")}; return {true, true};
            }
            result.request.selected_back_color_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--selected-fore-color-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--selected-fore-color-target-object-name")}; return {true, true};
            }
            result.request.selected_fore_color_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--selected-fore-color-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--selected-fore-color-target-unique-id")}; return {true, true};
            }
            result.request.selected_fore_color_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--selected-item-back-color-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--selected-item-back-color-target-object-name")}; return {true, true};
            }
            result.request.selected_item_back_color_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--selected-item-back-color-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--selected-item-back-color-target-unique-id")}; return {true, true};
            }
            result.request.selected_item_back_color_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--selected-item-fore-color-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--selected-item-fore-color-target-object-name")}; return {true, true};
            }
            result.request.selected_item_fore_color_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--selected-item-fore-color-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--selected-item-fore-color-target-unique-id")}; return {true, true};
            }
            result.request.selected_item_fore_color_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--disabled-item-back-color-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--disabled-item-back-color-target-object-name")}; return {true, true};
            }
            result.request.disabled_item_back_color_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--disabled-item-back-color-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--disabled-item-back-color-target-unique-id")}; return {true, true};
            }
            result.request.disabled_item_back_color_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--disabled-item-fore-color-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--disabled-item-fore-color-target-object-name")}; return {true, true};
            }
            result.request.disabled_item_fore_color_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--disabled-item-fore-color-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--disabled-item-fore-color-target-unique-id")}; return {true, true};
            }
            result.request.disabled_item_fore_color_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--item-back-color-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--item-back-color-target-object-name")}; return {true, true};
            }
            result.request.item_back_color_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--item-back-color-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--item-back-color-target-unique-id")}; return {true, true};
            }
            result.request.item_back_color_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--item-fore-color-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--item-fore-color-target-object-name")}; return {true, true};
            }
            result.request.item_fore_color_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--item-fore-color-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--item-fore-color-target-unique-id")}; return {true, true};
            }
            result.request.item_fore_color_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--highlight-back-color-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--highlight-back-color-target-object-name")}; return {true, true};
            }
            result.request.highlight_back_color_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--highlight-back-color-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--highlight-back-color-target-unique-id")}; return {true, true};
            }
            result.request.highlight_back_color_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--highlight-fore-color-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--highlight-fore-color-target-object-name")}; return {true, true};
            }
            result.request.highlight_fore_color_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--highlight-fore-color-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--highlight-fore-color-target-unique-id")}; return {true, true};
            }
            result.request.highlight_fore_color_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--back-color-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--back-color-target-object-name")}; return {true, true};
            }
            result.request.back_color_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--back-color-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--back-color-target-unique-id")}; return {true, true};
            }
            result.request.back_color_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--fore-color-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--fore-color-target-object-name")}; return {true, true};
            }
            result.request.fore_color_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--fore-color-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--fore-color-target-unique-id")}; return {true, true};
            }
            result.request.fore_color_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--disabled-back-color-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--disabled-back-color-target-object-name")}; return {true, true};
            }
            result.request.disabled_back_color_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--disabled-back-color-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--disabled-back-color-target-unique-id")}; return {true, true};
            }
            result.request.disabled_back_color_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--disabled-fore-color-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--disabled-fore-color-target-object-name")}; return {true, true};
            }
            result.request.disabled_fore_color_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--disabled-fore-color-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--disabled-fore-color-target-unique-id")}; return {true, true};
            }
            result.request.disabled_fore_color_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--dynamic-back-color-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--dynamic-back-color-target-object-name")}; return {true, true};
            }
            result.request.dynamic_back_color_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--dynamic-back-color-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--dynamic-back-color-target-unique-id")}; return {true, true};
            }
            result.request.dynamic_back_color_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--dynamic-fore-color-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--dynamic-fore-color-target-object-name")}; return {true, true};
            }
            result.request.dynamic_fore_color_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--dynamic-fore-color-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--dynamic-fore-color-target-unique-id")}; return {true, true};
            }
            result.request.dynamic_fore_color_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--back-style-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--back-style-target-object-name")}; return {true, true};
            }
            result.request.back_style_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--back-style-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--back-style-target-unique-id")}; return {true, true};
            }
            result.request.back_style_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--border-style-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--border-style-target-object-name")}; return {true, true};
            }
            result.request.border_style_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--border-style-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--border-style-target-unique-id")}; return {true, true};
            }
            result.request.border_style_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--border-width-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--border-width-target-object-name")}; return {true, true};
            }
            result.request.border_width_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--border-width-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--border-width-target-unique-id")}; return {true, true};
            }
            result.request.border_width_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--border-color-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--border-color-target-object-name")}; return {true, true};
            }
            result.request.border_color_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--border-color-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--border-color-target-unique-id")}; return {true, true};
            }
            result.request.border_color_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--special-effect-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--special-effect-target-object-name")}; return {true, true};
            }
            result.request.special_effect_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--special-effect-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--special-effect-target-unique-id")}; return {true, true};
            }
            result.request.special_effect_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--scroll-bars-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--scroll-bars-target-object-name")}; return {true, true};
            }
            result.request.scroll_bars_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--scroll-bars-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--scroll-bars-target-unique-id")}; return {true, true};
            }
            result.request.scroll_bars_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--window-state-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--window-state-target-object-name")}; return {true, true};
            }
            result.request.window_state_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--window-state-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--window-state-target-unique-id")}; return {true, true};
            }
            result.request.window_state_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--show-window-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--show-window-target-object-name")}; return {true, true};
            }
            result.request.show_window_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--show-window-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--show-window-target-unique-id")}; return {true, true};
            }
            result.request.show_window_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--title-bar-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--title-bar-target-object-name")}; return {true, true};
            }
            result.request.title_bar_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--title-bar-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--title-bar-target-unique-id")}; return {true, true};
            }
            result.request.title_bar_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--mouse-pointer-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--mouse-pointer-target-object-name")}; return {true, true};
            }
            result.request.mouse_pointer_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--mouse-pointer-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--mouse-pointer-target-unique-id")}; return {true, true};
            }
            result.request.mouse_pointer_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--picture-margin-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--picture-margin-target-object-name")}; return {true, true};
            }
            result.request.picture_margin_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--picture-margin-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--picture-margin-target-unique-id")}; return {true, true};
            }
            result.request.picture_margin_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--picture-position-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--picture-position-target-object-name")}; return {true, true};
            }
            result.request.picture_position_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--picture-position-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--picture-position-target-unique-id")}; return {true, true};
            }
            result.request.picture_position_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--picture-spacing-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--picture-spacing-target-object-name")}; return {true, true};
            }
            result.request.picture_spacing_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--picture-spacing-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--picture-spacing-target-unique-id")}; return {true, true};
            }
            result.request.picture_spacing_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--picture-selection-display-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--picture-selection-display-target-object-name")}; return {true, true};
            }
            result.request.picture_selection_display_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--picture-selection-display-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--picture-selection-display-target-unique-id")}; return {true, true};
            }
            result.request.picture_selection_display_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--dynamic-input-mask-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--dynamic-input-mask-target-object-name")}; return {true, true};
            }
            result.request.dynamic_input_mask_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--dynamic-input-mask-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--dynamic-input-mask-target-unique-id")}; return {true, true};
            }
            result.request.dynamic_input_mask_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--dynamic-line-height-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--dynamic-line-height-target-object-name")}; return {true, true};
            }
            result.request.dynamic_line_height_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--dynamic-line-height-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--dynamic-line-height-target-unique-id")}; return {true, true};
            }
            result.request.dynamic_line_height_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--dynamic-alignment-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--dynamic-alignment-target-object-name")}; return {true, true};
            }
            result.request.dynamic_alignment_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--dynamic-alignment-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--dynamic-alignment-target-unique-id")}; return {true, true};
            }
            result.request.dynamic_alignment_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--dynamic-current-control-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--dynamic-current-control-target-object-name")}; return {true, true};
            }
            result.request.dynamic_current_control_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--dynamic-current-control-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--dynamic-current-control-target-unique-id")}; return {true, true};
            }
            result.request.dynamic_current_control_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--dynamic-font-name-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--dynamic-font-name-target-object-name")}; return {true, true};
            }
            result.request.dynamic_font_name_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--dynamic-font-name-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--dynamic-font-name-target-unique-id")}; return {true, true};
            }
            result.request.dynamic_font_name_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--dynamic-font-size-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--dynamic-font-size-target-object-name")}; return {true, true};
            }
            result.request.dynamic_font_size_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--dynamic-font-size-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--dynamic-font-size-target-unique-id")}; return {true, true};
            }
            result.request.dynamic_font_size_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--dynamic-font-bold-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--dynamic-font-bold-target-object-name")}; return {true, true};
            }
            result.request.dynamic_font_bold_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--dynamic-font-bold-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--dynamic-font-bold-target-unique-id")}; return {true, true};
            }
            result.request.dynamic_font_bold_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--dynamic-font-italic-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--dynamic-font-italic-target-object-name")}; return {true, true};
            }
            result.request.dynamic_font_italic_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--dynamic-font-italic-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--dynamic-font-italic-target-unique-id")}; return {true, true};
            }
            result.request.dynamic_font_italic_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--dynamic-font-underline-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--dynamic-font-underline-target-object-name")}; return {true, true};
            }
            result.request.dynamic_font_underline_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--dynamic-font-underline-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--dynamic-font-underline-target-unique-id")}; return {true, true};
            }
            result.request.dynamic_font_underline_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--dynamic-font-strikethru-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--dynamic-font-strikethru-target-object-name")}; return {true, true};
            }
            result.request.dynamic_font_strikethru_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--dynamic-font-strikethru-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--dynamic-font-strikethru-target-unique-id")}; return {true, true};
            }
            result.request.dynamic_font_strikethru_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--dynamic-font-outline-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--dynamic-font-outline-target-object-name")}; return {true, true};
            }
            result.request.dynamic_font_outline_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--dynamic-font-outline-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--dynamic-font-outline-target-unique-id")}; return {true, true};
            }
            result.request.dynamic_font_outline_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--dynamic-font-shadow-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--dynamic-font-shadow-target-object-name")}; return {true, true};
            }
            result.request.dynamic_font_shadow_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--dynamic-font-shadow-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--dynamic-font-shadow-target-unique-id")}; return {true, true};
            }
            result.request.dynamic_font_shadow_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--font-name-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--font-name-target-object-name")}; return {true, true};
            }
            result.request.font_name_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--font-name-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--font-name-target-unique-id")}; return {true, true};
            }
            result.request.font_name_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--font-size-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--font-size-target-object-name")}; return {true, true};
            }
            result.request.font_size_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--font-size-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--font-size-target-unique-id")}; return {true, true};
            }
            result.request.font_size_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--font-bold-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--font-bold-target-object-name")}; return {true, true};
            }
            result.request.font_bold_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--font-bold-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--font-bold-target-unique-id")}; return {true, true};
            }
            result.request.font_bold_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--font-italic-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--font-italic-target-object-name")}; return {true, true};
            }
            result.request.font_italic_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--font-italic-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--font-italic-target-unique-id")}; return {true, true};
            }
            result.request.font_italic_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--font-underline-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--font-underline-target-object-name")}; return {true, true};
            }
            result.request.font_underline_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--font-underline-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--font-underline-target-unique-id")}; return {true, true};
            }
            result.request.font_underline_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--font-strikethru-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--font-strikethru-target-object-name")}; return {true, true};
            }
            result.request.font_strikethru_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--font-strikethru-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--font-strikethru-target-unique-id")}; return {true, true};
            }
            result.request.font_strikethru_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--font-outline-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--font-outline-target-object-name")}; return {true, true};
            }
            result.request.font_outline_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--font-outline-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--font-outline-target-unique-id")}; return {true, true};
            }
            result.request.font_outline_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--font-shadow-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--font-shadow-target-object-name")}; return {true, true};
            }
            result.request.font_shadow_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--font-shadow-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--font-shadow-target-unique-id")}; return {true, true};
            }
            result.request.font_shadow_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }
    return {false, false};
}

}  // namespace copperfin::studio
