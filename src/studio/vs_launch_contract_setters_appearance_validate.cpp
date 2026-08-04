// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "vs_launch_contract_internal.h"

namespace copperfin::studio {

std::optional<LaunchParseResult> validate_setters_appearance(
    const LaunchParseResult& result,
    const localization::LocalizedCatalog& catalog) {
if (result.request.picture_object && !result.request.picture_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.Picture"),
            "--picture")};
    }

if (result.request.picture_object && result.request.picture_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.Picture"))};
    }

if (!result.request.picture_object &&
        (result.request.picture_available ||
         !result.request.picture_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.PictureTitle"),
            "--picture-object")};
    }

if (result.request.down_picture_object && !result.request.down_picture_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DownPicture"),
            "--down-picture")};
    }

if (result.request.down_picture_object && result.request.down_picture_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DownPicture"))};
    }

if (!result.request.down_picture_object &&
        (result.request.down_picture_available ||
         !result.request.down_picture_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DownPictureTitle"),
            "--down-picture-object")};
    }

if (result.request.disabled_picture_object && !result.request.disabled_picture_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DisabledPicture"),
            "--disabled-picture")};
    }

if (result.request.disabled_picture_object && result.request.disabled_picture_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DisabledPicture"))};
    }

if (!result.request.disabled_picture_object &&
        (result.request.disabled_picture_available ||
         !result.request.disabled_picture_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DisabledPictureTitle"),
            "--disabled-picture-object")};
    }

if (result.request.ole_drag_picture_object && !result.request.ole_drag_picture_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.OleDragPicture"),
            "--ole-drag-picture")};
    }

if (result.request.ole_drag_picture_object && result.request.ole_drag_picture_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.OleDragPicture"))};
    }

if (!result.request.ole_drag_picture_object &&
        (result.request.ole_drag_picture_available ||
         !result.request.ole_drag_picture_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.OleDragPictureTitle"),
            "--ole-drag-picture-object")};
    }

if (result.request.mouse_icon_object && !result.request.mouse_icon_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.MouseIcon"),
            "--mouse-icon")};
    }

if (result.request.mouse_icon_object && result.request.mouse_icon_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.MouseIcon"))};
    }

if (!result.request.mouse_icon_object &&
        (result.request.mouse_icon_available ||
         !result.request.mouse_icon_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.MouseIconTitle"),
            "--mouse-icon-object")};
    }

if (result.request.drag_icon_object && !result.request.drag_icon_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DragIcon"),
            "--drag-icon")};
    }

if (result.request.drag_icon_object && result.request.drag_icon_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DragIcon"))};
    }

if (!result.request.drag_icon_object &&
        (result.request.drag_icon_available ||
         !result.request.drag_icon_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DragIconTitle"),
            "--drag-icon-object")};
    }

if (result.request.drag_mode_object && !result.request.drag_mode_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DragMode"),
            "--drag-mode")};
    }

if (result.request.drag_mode_object && result.request.drag_mode_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DragMode"))};
    }

if (!result.request.drag_mode_object &&
        (result.request.drag_mode_available ||
         !result.request.drag_mode_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DragModeTitle"),
            "--drag-mode-object")};
    }

if (result.request.ole_drag_mode_object && !result.request.ole_drag_mode_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.OleDragMode"),
            "--ole-drag-mode")};
    }

if (result.request.ole_drag_mode_object && result.request.ole_drag_mode_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.OleDragMode"))};
    }

if (!result.request.ole_drag_mode_object &&
        (result.request.ole_drag_mode_available ||
         !result.request.ole_drag_mode_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.OleDragModeTitle"),
            "--ole-drag-mode-object")};
    }

if (result.request.ole_drop_mode_object && !result.request.ole_drop_mode_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.OleDropMode"),
            "--ole-drop-mode")};
    }

if (result.request.ole_drop_mode_object && result.request.ole_drop_mode_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.OleDropMode"))};
    }

if (!result.request.ole_drop_mode_object &&
        (result.request.ole_drop_mode_available ||
         !result.request.ole_drop_mode_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.OleDropModeTitle"),
            "--ole-drop-mode-object")};
    }

if (result.request.ole_drop_effects_object && !result.request.ole_drop_effects_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.OleDropEffects"),
            "--ole-drop-effects")};
    }

if (result.request.ole_drop_effects_object && result.request.ole_drop_effects_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.OleDropEffects"))};
    }

if (!result.request.ole_drop_effects_object &&
        (result.request.ole_drop_effects_available ||
         !result.request.ole_drop_effects_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.OleDropEffectsTitle"),
            "--ole-drop-effects-object")};
    }

if (result.request.ole_drop_text_insertion_object && !result.request.ole_drop_text_insertion_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.OleDropTextInsertion"),
            "--ole-drop-text-insertion")};
    }

if (result.request.ole_drop_text_insertion_object && result.request.ole_drop_text_insertion_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.OleDropTextInsertion"))};
    }

if (!result.request.ole_drop_text_insertion_object &&
        (result.request.ole_drop_text_insertion_available ||
         !result.request.ole_drop_text_insertion_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.OleDropTextInsertionTitle"),
            "--ole-drop-text-insertion-object")};
    }

if (result.request.curvature_object && !result.request.curvature_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.Curvature"),
            "--curvature")};
    }

if (result.request.curvature_object && result.request.curvature_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.Curvature"))};
    }

if (!result.request.curvature_object &&
        (result.request.curvature_available ||
         !result.request.curvature_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.CurvatureTitle"),
            "--curvature-object")};
    }

if (result.request.draw_mode_object && !result.request.draw_mode_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DrawMode"),
            "--draw-mode")};
    }

if (result.request.draw_mode_object && result.request.draw_mode_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DrawMode"))};
    }

if (!result.request.draw_mode_object &&
        (result.request.draw_mode_available ||
         !result.request.draw_mode_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DrawModeTitle"),
            "--draw-mode-object")};
    }

if (result.request.draw_style_object && !result.request.draw_style_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DrawStyle"),
            "--draw-style")};
    }

if (result.request.draw_style_object && result.request.draw_style_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DrawStyle"))};
    }

if (!result.request.draw_style_object &&
        (result.request.draw_style_available ||
         !result.request.draw_style_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DrawStyleTitle"),
            "--draw-style-object")};
    }

if (result.request.draw_width_object && !result.request.draw_width_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DrawWidth"),
            "--draw-width")};
    }

if (result.request.draw_width_object && result.request.draw_width_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DrawWidth"))};
    }

if (!result.request.draw_width_object &&
        (result.request.draw_width_available ||
         !result.request.draw_width_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DrawWidthTitle"),
            "--draw-width-object")};
    }

if (result.request.fill_style_object && !result.request.fill_style_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.FillStyle"),
            "--fill-style")};
    }

if (result.request.fill_style_object && result.request.fill_style_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.FillStyle"))};
    }

if (!result.request.fill_style_object &&
        (result.request.fill_style_available ||
         !result.request.fill_style_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.FillStyleTitle"),
            "--fill-style-object")};
    }

if (result.request.scale_mode_object && !result.request.scale_mode_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ScaleMode"),
            "--scale-mode")};
    }

if (result.request.scale_mode_object && result.request.scale_mode_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ScaleMode"))};
    }

if (!result.request.scale_mode_object &&
        (result.request.scale_mode_available ||
         !result.request.scale_mode_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ScaleModeTitle"),
            "--scale-mode-object")};
    }

if (result.request.buffer_mode_object && !result.request.buffer_mode_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.BufferMode"),
            "--buffer-mode")};
    }

if (result.request.buffer_mode_object && result.request.buffer_mode_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.BufferMode"))};
    }

if (!result.request.buffer_mode_object &&
        (result.request.buffer_mode_available ||
         !result.request.buffer_mode_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.BufferModeTitle"),
            "--buffer-mode-object")};
    }

if (result.request.buffer_mode_override_object && !result.request.buffer_mode_override_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.BufferModeOverride"),
            "--buffer-mode-override")};
    }

if (result.request.buffer_mode_override_object && result.request.buffer_mode_override_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.BufferModeOverride"))};
    }

if (!result.request.buffer_mode_override_object &&
        (result.request.buffer_mode_override_available ||
         !result.request.buffer_mode_override_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.BufferModeOverrideTitle"),
            "--buffer-mode-override-object")};
    }

if (result.request.grid_line_color_object && !result.request.grid_line_color_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.GridLineColor"),
            "--grid-line-color")};
    }

if (result.request.grid_line_color_object && result.request.grid_line_color_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.GridLineColor"))};
    }

if (!result.request.grid_line_color_object &&
        (result.request.grid_line_color_available ||
         !result.request.grid_line_color_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.GridLineColorTitle"),
            "--grid-line-color-object")};
    }

if (result.request.header_height_object && !result.request.header_height_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.HeaderHeight"),
            "--header-height")};
    }

if (result.request.header_height_object && result.request.header_height_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.HeaderHeight"))};
    }

if (!result.request.header_height_object &&
        (result.request.header_height_available ||
         !result.request.header_height_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.HeaderHeightTitle"),
            "--header-height-object")};
    }

if (result.request.row_height_object && !result.request.row_height_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.RowHeight"),
            "--row-height")};
    }

if (result.request.row_height_object && result.request.row_height_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.RowHeight"))};
    }

if (!result.request.row_height_object &&
        (result.request.row_height_available ||
         !result.request.row_height_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.RowHeightTitle"),
            "--row-height-object")};
    }

if (result.request.grid_line_width_object && !result.request.grid_line_width_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.GridLineWidth"),
            "--grid-line-width")};
    }

if (result.request.grid_line_width_object && result.request.grid_line_width_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.GridLineWidth"))};
    }

if (!result.request.grid_line_width_object &&
        (result.request.grid_line_width_available ||
         !result.request.grid_line_width_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.GridLineWidthTitle"),
            "--grid-line-width-object")};
    }

if (result.request.grid_lines_object && !result.request.grid_lines_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.GridLines"),
            "--grid-lines")};
    }

if (result.request.grid_lines_object && result.request.grid_lines_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.GridLines"))};
    }

if (!result.request.grid_lines_object &&
        (result.request.grid_lines_available ||
         !result.request.grid_lines_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.GridLinesTitle"),
            "--grid-lines-object")};
    }

if (result.request.highlight_row_line_width_object && !result.request.highlight_row_line_width_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.HighlightRowLineWidth"),
            "--highlight-row-line-width")};
    }

if (result.request.highlight_row_line_width_object && result.request.highlight_row_line_width_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.HighlightRowLineWidth"))};
    }

if (!result.request.highlight_row_line_width_object &&
        (result.request.highlight_row_line_width_available ||
         !result.request.highlight_row_line_width_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.HighlightRowLineWidthTitle"),
            "--highlight-row-line-width-object")};
    }

if (result.request.highlight_style_object && !result.request.highlight_style_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.HighlightStyle"),
            "--highlight-style")};
    }

if (result.request.highlight_style_object && result.request.highlight_style_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.HighlightStyle"))};
    }

if (!result.request.highlight_style_object &&
        (result.request.highlight_style_available ||
         !result.request.highlight_style_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.HighlightStyleTitle"),
            "--highlight-style-object")};
    }

if (result.request.fill_color_object && !result.request.fill_color_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.FillColor"),
            "--fill-color")};
    }

if (result.request.fill_color_object && result.request.fill_color_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.FillColor"))};
    }

if (!result.request.fill_color_object &&
        (result.request.fill_color_available ||
         !result.request.fill_color_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.FillColorTitle"),
            "--fill-color-object")};
    }

if (result.request.input_mask_object && !result.request.input_mask_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.InputMask"),
            "--input-mask")};
    }

if (result.request.input_mask_object && result.request.input_mask_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.InputMask"))};
    }

if (!result.request.input_mask_object &&
        (result.request.input_mask_available ||
         !result.request.input_mask_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.InputMaskTitle"),
            "--input-mask-object")};
    }

if (result.request.format_object && !result.request.format_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            localized_object_assignment_format(catalog),
            "--format")};
    }

if (result.request.format_object && result.request.format_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            localized_object_assignment_format(catalog))};
    }

if (!result.request.format_object &&
        (result.request.format_available ||
         !result.request.format_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            localized_object_assignment_format_title(catalog),
            "--format-object")};
    }

if (result.request.selected_back_color_object && !result.request.selected_back_color_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.SelectedBackColor"),
            "--selected-back-color")};
    }

if (result.request.selected_back_color_object && result.request.selected_back_color < 0) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_non_negative_value(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.SelectedBackColor"),
            catalog.translate("StudioHost.LaunchParse.Value.Value"))};
    }

if (result.request.selected_back_color_object && result.request.selected_back_color_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.SelectedBackColor"))};
    }

if (!result.request.selected_back_color_object &&
        (result.request.selected_back_color_available ||
         !result.request.selected_back_color_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.SelectedBackColorTitle"),
            "--selected-back-color-object")};
    }

if (result.request.selected_fore_color_object && !result.request.selected_fore_color_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.SelectedForeColor"),
            "--selected-fore-color")};
    }

if (result.request.selected_fore_color_object && result.request.selected_fore_color < 0) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_non_negative_value(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.SelectedForeColor"),
            catalog.translate("StudioHost.LaunchParse.Value.Value"))};
    }

if (result.request.selected_fore_color_object && result.request.selected_fore_color_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.SelectedForeColor"))};
    }

if (!result.request.selected_fore_color_object &&
        (result.request.selected_fore_color_available ||
         !result.request.selected_fore_color_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.SelectedForeColorTitle"),
            "--selected-fore-color-object")};
    }

if (result.request.selected_item_back_color_object && !result.request.selected_item_back_color_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.SelectedItemBackColor"),
            "--selected-item-back-color")};
    }

if (result.request.selected_item_back_color_object && result.request.selected_item_back_color < 0) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_non_negative_value(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.SelectedItemBackColor"),
            catalog.translate("StudioHost.LaunchParse.Value.Value"))};
    }

if (result.request.selected_item_back_color_object && result.request.selected_item_back_color_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.SelectedItemBackColor"))};
    }

if (!result.request.selected_item_back_color_object &&
        (result.request.selected_item_back_color_available ||
         !result.request.selected_item_back_color_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.SelectedItemBackColorTitle"),
            "--selected-item-back-color-object")};
    }

if (result.request.selected_item_fore_color_object && !result.request.selected_item_fore_color_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.SelectedItemForeColor"),
            "--selected-item-fore-color")};
    }

if (result.request.selected_item_fore_color_object && result.request.selected_item_fore_color < 0) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_non_negative_value(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.SelectedItemForeColor"),
            catalog.translate("StudioHost.LaunchParse.Value.Value"))};
    }

if (result.request.selected_item_fore_color_object && result.request.selected_item_fore_color_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.SelectedItemForeColor"))};
    }

if (!result.request.selected_item_fore_color_object &&
        (result.request.selected_item_fore_color_available ||
         !result.request.selected_item_fore_color_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.SelectedItemForeColorTitle"),
            "--selected-item-fore-color-object")};
    }

if (result.request.disabled_item_back_color_object && !result.request.disabled_item_back_color_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DisabledItemBackColor"),
            "--disabled-item-back-color")};
    }

if (result.request.disabled_item_back_color_object && result.request.disabled_item_back_color < 0) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_non_negative_value(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DisabledItemBackColor"),
            catalog.translate("StudioHost.LaunchParse.Value.Value"))};
    }

if (result.request.disabled_item_back_color_object && result.request.disabled_item_back_color_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DisabledItemBackColor"))};
    }

if (!result.request.disabled_item_back_color_object &&
        (result.request.disabled_item_back_color_available ||
         !result.request.disabled_item_back_color_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DisabledItemBackColorTitle"),
            "--disabled-item-back-color-object")};
    }

if (result.request.disabled_item_fore_color_object && !result.request.disabled_item_fore_color_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DisabledItemForeColor"),
            "--disabled-item-fore-color")};
    }

if (result.request.disabled_item_fore_color_object && result.request.disabled_item_fore_color < 0) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_non_negative_value(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DisabledItemForeColor"),
            catalog.translate("StudioHost.LaunchParse.Value.Value"))};
    }

if (result.request.disabled_item_fore_color_object && result.request.disabled_item_fore_color_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DisabledItemForeColor"))};
    }

if (!result.request.disabled_item_fore_color_object &&
        (result.request.disabled_item_fore_color_available ||
         !result.request.disabled_item_fore_color_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DisabledItemForeColorTitle"),
            "--disabled-item-fore-color-object")};
    }

if (result.request.item_back_color_object && !result.request.item_back_color_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ItemBackColor"),
            "--item-back-color")};
    }

if (result.request.item_back_color_object && result.request.item_back_color < 0) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_non_negative_value(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ItemBackColor"),
            catalog.translate("StudioHost.LaunchParse.Value.Value"))};
    }

if (result.request.item_back_color_object && result.request.item_back_color_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ItemBackColor"))};
    }

if (!result.request.item_back_color_object &&
        (result.request.item_back_color_available ||
         !result.request.item_back_color_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ItemBackColorTitle"),
            "--item-back-color-object")};
    }

if (result.request.item_fore_color_object && !result.request.item_fore_color_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ItemForeColor"),
            "--item-fore-color")};
    }

if (result.request.item_fore_color_object && result.request.item_fore_color < 0) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_non_negative_value(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ItemForeColor"),
            catalog.translate("StudioHost.LaunchParse.Value.Value"))};
    }

if (result.request.item_fore_color_object && result.request.item_fore_color_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ItemForeColor"))};
    }

if (!result.request.item_fore_color_object &&
        (result.request.item_fore_color_available ||
         !result.request.item_fore_color_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ItemForeColorTitle"),
            "--item-fore-color-object")};
    }

if (result.request.highlight_back_color_object && !result.request.highlight_back_color_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.HighlightBackColor"),
            "--highlight-back-color")};
    }

if (result.request.highlight_back_color_object && result.request.highlight_back_color < 0) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_non_negative_value(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.HighlightBackColor"),
            catalog.translate("StudioHost.LaunchParse.Value.Value"))};
    }

if (result.request.highlight_back_color_object && result.request.highlight_back_color_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.HighlightBackColor"))};
    }

if (!result.request.highlight_back_color_object &&
        (result.request.highlight_back_color_available ||
         !result.request.highlight_back_color_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.HighlightBackColorTitle"),
            "--highlight-back-color-object")};
    }

if (result.request.highlight_fore_color_object && !result.request.highlight_fore_color_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.HighlightForeColor"),
            "--highlight-fore-color")};
    }

if (result.request.highlight_fore_color_object && result.request.highlight_fore_color < 0) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_non_negative_value(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.HighlightForeColor"),
            catalog.translate("StudioHost.LaunchParse.Value.Value"))};
    }

if (result.request.highlight_fore_color_object && result.request.highlight_fore_color_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.HighlightForeColor"))};
    }

if (!result.request.highlight_fore_color_object &&
        (result.request.highlight_fore_color_available ||
         !result.request.highlight_fore_color_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.HighlightForeColorTitle"),
            "--highlight-fore-color-object")};
    }

if (result.request.back_color_object && !result.request.back_color_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.BackColor"),
            "--back-color")};
    }

if (result.request.back_color_object && result.request.back_color < 0) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_non_negative_value(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.BackColor"),
            catalog.translate("StudioHost.LaunchParse.Value.Value"))};
    }

if (result.request.back_color_object && result.request.back_color_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.BackColor"))};
    }

if (!result.request.back_color_object &&
        (result.request.back_color_available ||
         !result.request.back_color_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.BackColorTitle"),
            "--back-color-object")};
    }

if (result.request.fore_color_object && !result.request.fore_color_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ForeColor"),
            "--fore-color")};
    }

if (result.request.fore_color_object && result.request.fore_color < 0) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_non_negative_value(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ForeColor"),
            catalog.translate("StudioHost.LaunchParse.Value.Value"))};
    }

if (result.request.fore_color_object && result.request.fore_color_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ForeColor"))};
    }

if (!result.request.fore_color_object &&
        (result.request.fore_color_available ||
         !result.request.fore_color_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ForeColorTitle"),
            "--fore-color-object")};
    }

if (result.request.disabled_back_color_object && !result.request.disabled_back_color_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DisabledBackColor"),
            "--disabled-back-color")};
    }

if (result.request.disabled_back_color_object && result.request.disabled_back_color < 0) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_non_negative_value(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DisabledBackColor"),
            catalog.translate("StudioHost.LaunchParse.Value.Value"))};
    }

if (result.request.disabled_back_color_object && result.request.disabled_back_color_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DisabledBackColor"))};
    }

if (!result.request.disabled_back_color_object &&
        (result.request.disabled_back_color_available ||
         !result.request.disabled_back_color_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DisabledBackColorTitle"),
            "--disabled-back-color-object")};
    }

if (result.request.disabled_fore_color_object && !result.request.disabled_fore_color_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DisabledForeColor"),
            "--disabled-fore-color")};
    }

if (result.request.disabled_fore_color_object && result.request.disabled_fore_color < 0) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_non_negative_value(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DisabledForeColor"),
            catalog.translate("StudioHost.LaunchParse.Value.Value"))};
    }

if (result.request.disabled_fore_color_object && result.request.disabled_fore_color_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DisabledForeColor"))};
    }

if (!result.request.disabled_fore_color_object &&
        (result.request.disabled_fore_color_available ||
         !result.request.disabled_fore_color_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DisabledForeColorTitle"),
            "--disabled-fore-color-object")};
    }

if (result.request.dynamic_back_color_object && !result.request.dynamic_back_color_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DynamicBackColor"),
            "--dynamic-back-color")};
    }

if (result.request.dynamic_back_color_object && result.request.dynamic_back_color_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DynamicBackColor"))};
    }

if (!result.request.dynamic_back_color_object &&
        (result.request.dynamic_back_color_available ||
         !result.request.dynamic_back_color_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DynamicBackColorTitle"),
            "--dynamic-back-color-object")};
    }

if (result.request.dynamic_fore_color_object && !result.request.dynamic_fore_color_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DynamicForeColor"),
            "--dynamic-fore-color")};
    }

if (result.request.dynamic_fore_color_object && result.request.dynamic_fore_color_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DynamicForeColor"))};
    }

if (!result.request.dynamic_fore_color_object &&
        (result.request.dynamic_fore_color_available ||
         !result.request.dynamic_fore_color_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DynamicForeColorTitle"),
            "--dynamic-fore-color-object")};
    }

if (result.request.back_style_object && !result.request.back_style_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.BackStyle"),
            "--back-style")};
    }

if (result.request.back_style_object && result.request.back_style_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.BackStyle"))};
    }

if (!result.request.back_style_object &&
        (result.request.back_style_available ||
         !result.request.back_style_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.BackStyleTitle"),
            "--back-style-object")};
    }

if (result.request.border_style_object && !result.request.border_style_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.BorderStyle"),
            "--border-style")};
    }

if (result.request.border_style_object && result.request.border_style_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.BorderStyle"))};
    }

if (!result.request.border_style_object &&
        (result.request.border_style_available ||
         !result.request.border_style_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.BorderStyleTitle"),
            "--border-style-object")};
    }

if (result.request.border_width_object && !result.request.border_width_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.BorderWidth"),
            "--border-width")};
    }

if (result.request.border_width_object && result.request.border_width_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.BorderWidth"))};
    }

if (!result.request.border_width_object &&
        (result.request.border_width_available ||
         !result.request.border_width_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.BorderWidthTitle"),
            "--border-width-object")};
    }

if (result.request.border_color_object && !result.request.border_color_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.BorderColor"),
            "--border-color")};
    }

if (result.request.border_color_object && result.request.border_color_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.BorderColor"))};
    }

if (!result.request.border_color_object &&
        (result.request.border_color_available ||
         !result.request.border_color_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.BorderColorTitle"),
            "--border-color-object")};
    }

if (result.request.special_effect_object && !result.request.special_effect_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.SpecialEffect"),
            "--special-effect")};
    }

if (result.request.special_effect_object && result.request.special_effect_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.SpecialEffect"))};
    }

if (!result.request.special_effect_object &&
        (result.request.special_effect_available ||
         !result.request.special_effect_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.SpecialEffectTitle"),
            "--special-effect-object")};
    }

if (result.request.scroll_bars_object && !result.request.scroll_bars_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ScrollBars"),
            "--scroll-bars")};
    }

if (result.request.scroll_bars_object && result.request.scroll_bars_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ScrollBars"))};
    }

if (!result.request.scroll_bars_object &&
        (result.request.scroll_bars_available ||
         !result.request.scroll_bars_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ScrollBarsTitle"),
            "--scroll-bars-object")};
    }

if (result.request.window_state_object && !result.request.window_state_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.WindowState"),
            "--window-state")};
    }

if (result.request.window_state_object && result.request.window_state_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.WindowState"))};
    }

if (!result.request.window_state_object &&
        (result.request.window_state_available ||
         !result.request.window_state_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.WindowStateTitle"),
            "--window-state-object")};
    }

if (result.request.show_window_object && !result.request.show_window_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ShowWindow"),
            "--show-window")};
    }

if (result.request.show_window_object && result.request.show_window_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ShowWindow"))};
    }

if (!result.request.show_window_object &&
        (result.request.show_window_available ||
         !result.request.show_window_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ShowWindowTitle"),
            "--show-window-object")};
    }

if (result.request.title_bar_object && !result.request.title_bar_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.TitleBar"),
            "--title-bar")};
    }

if (result.request.title_bar_object && result.request.title_bar_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.TitleBar"))};
    }

if (!result.request.title_bar_object &&
        (result.request.title_bar_available ||
         !result.request.title_bar_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.TitleBarTitle"),
            "--title-bar-object")};
    }

if (result.request.mouse_pointer_object && !result.request.mouse_pointer_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.MousePointer"),
            "--mouse-pointer")};
    }

if (result.request.mouse_pointer_object && result.request.mouse_pointer_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.MousePointer"))};
    }

if (!result.request.mouse_pointer_object &&
        (result.request.mouse_pointer_available ||
         !result.request.mouse_pointer_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.MousePointerTitle"),
            "--mouse-pointer-object")};
    }

if (result.request.picture_margin_object && !result.request.picture_margin_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.PictureMargin"),
            "--picture-margin")};
    }

if (result.request.picture_margin_object && result.request.picture_margin_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.PictureMargin"))};
    }

if (!result.request.picture_margin_object &&
        (result.request.picture_margin_available ||
         !result.request.picture_margin_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.PictureMarginTitle"),
            "--picture-margin-object")};
    }

if (result.request.picture_position_object && !result.request.picture_position_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.PicturePosition"),
            "--picture-position")};
    }

if (result.request.picture_position_object && result.request.picture_position_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.PicturePosition"))};
    }

if (!result.request.picture_position_object &&
        (result.request.picture_position_available ||
         !result.request.picture_position_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.PicturePositionTitle"),
            "--picture-position-object")};
    }

if (result.request.picture_spacing_object && !result.request.picture_spacing_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.PictureSpacing"),
            "--picture-spacing")};
    }

if (result.request.picture_spacing_object && result.request.picture_spacing_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.PictureSpacing"))};
    }

if (!result.request.picture_spacing_object &&
        (result.request.picture_spacing_available ||
         !result.request.picture_spacing_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.PictureSpacingTitle"),
            "--picture-spacing-object")};
    }

if (result.request.picture_selection_display_object && !result.request.picture_selection_display_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.PictureSelectionDisplay"),
            "--picture-selection-display")};
    }

if (result.request.picture_selection_display_object && result.request.picture_selection_display_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.PictureSelectionDisplay"))};
    }

if (!result.request.picture_selection_display_object &&
        (result.request.picture_selection_display_available ||
         !result.request.picture_selection_display_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.PictureSelectionDisplayTitle"),
            "--picture-selection-display-object")};
    }

if (result.request.dynamic_input_mask_object && !result.request.dynamic_input_mask_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DynamicInputMask"),
            "--dynamic-input-mask")};
    }

if (result.request.dynamic_input_mask_object && result.request.dynamic_input_mask_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DynamicInputMask"))};
    }

if (!result.request.dynamic_input_mask_object &&
        (result.request.dynamic_input_mask_available ||
         !result.request.dynamic_input_mask_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DynamicInputMaskTitle"),
            "--dynamic-input-mask-object")};
    }

if (result.request.dynamic_line_height_object && !result.request.dynamic_line_height_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DynamicLineHeight"),
            "--dynamic-line-height")};
    }

if (result.request.dynamic_line_height_object && result.request.dynamic_line_height_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DynamicLineHeight"))};
    }

if (!result.request.dynamic_line_height_object &&
        (result.request.dynamic_line_height_available ||
         !result.request.dynamic_line_height_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DynamicLineHeightTitle"),
            "--dynamic-line-height-object")};
    }

if (result.request.dynamic_alignment_object && !result.request.dynamic_alignment_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DynamicAlignment"),
            "--dynamic-alignment")};
    }

if (result.request.dynamic_alignment_object && result.request.dynamic_alignment_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DynamicAlignment"))};
    }

if (!result.request.dynamic_alignment_object &&
        (result.request.dynamic_alignment_available ||
         !result.request.dynamic_alignment_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DynamicAlignmentTitle"),
            "--dynamic-alignment-object")};
    }

if (result.request.dynamic_current_control_object && !result.request.dynamic_current_control_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DynamicCurrentControl"),
            "--dynamic-current-control")};
    }

if (result.request.dynamic_current_control_object && result.request.dynamic_current_control_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DynamicCurrentControl"))};
    }

if (!result.request.dynamic_current_control_object &&
        (result.request.dynamic_current_control_available ||
         !result.request.dynamic_current_control_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DynamicCurrentControlTitle"),
            "--dynamic-current-control-object")};
    }

if (result.request.dynamic_font_name_object && !result.request.dynamic_font_name_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DynamicFontName"),
            "--dynamic-font-name")};
    }

if (result.request.dynamic_font_name_object && result.request.dynamic_font_name_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DynamicFontName"))};
    }

if (!result.request.dynamic_font_name_object &&
        (result.request.dynamic_font_name_available ||
         !result.request.dynamic_font_name_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DynamicFontNameTitle"),
            "--dynamic-font-name-object")};
    }

if (result.request.dynamic_font_size_object && !result.request.dynamic_font_size_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DynamicFontSize"),
            "--dynamic-font-size")};
    }

if (result.request.dynamic_font_size_object && result.request.dynamic_font_size_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DynamicFontSize"))};
    }

if (!result.request.dynamic_font_size_object &&
        (result.request.dynamic_font_size_available ||
         !result.request.dynamic_font_size_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DynamicFontSizeTitle"),
            "--dynamic-font-size-object")};
    }

if (result.request.dynamic_font_bold_object && !result.request.dynamic_font_bold_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DynamicFontBold"),
            "--dynamic-font-bold")};
    }

if (result.request.dynamic_font_bold_object && result.request.dynamic_font_bold_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DynamicFontBold"))};
    }

if (!result.request.dynamic_font_bold_object &&
        (result.request.dynamic_font_bold_available ||
         !result.request.dynamic_font_bold_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DynamicFontBoldTitle"),
            "--dynamic-font-bold-object")};
    }

if (result.request.dynamic_font_italic_object && !result.request.dynamic_font_italic_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DynamicFontItalic"),
            "--dynamic-font-italic")};
    }

if (result.request.dynamic_font_italic_object && result.request.dynamic_font_italic_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DynamicFontItalic"))};
    }

if (!result.request.dynamic_font_italic_object &&
        (result.request.dynamic_font_italic_available ||
         !result.request.dynamic_font_italic_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DynamicFontItalicTitle"),
            "--dynamic-font-italic-object")};
    }

if (result.request.dynamic_font_underline_object && !result.request.dynamic_font_underline_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DynamicFontUnderline"),
            "--dynamic-font-underline")};
    }

if (result.request.dynamic_font_underline_object && result.request.dynamic_font_underline_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DynamicFontUnderline"))};
    }

if (!result.request.dynamic_font_underline_object &&
        (result.request.dynamic_font_underline_available ||
         !result.request.dynamic_font_underline_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DynamicFontUnderlineTitle"),
            "--dynamic-font-underline-object")};
    }

if (result.request.dynamic_font_strikethru_object && !result.request.dynamic_font_strikethru_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DynamicFontStrikethru"),
            "--dynamic-font-strikethru")};
    }

if (result.request.dynamic_font_strikethru_object && result.request.dynamic_font_strikethru_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DynamicFontStrikethru"))};
    }

if (!result.request.dynamic_font_strikethru_object &&
        (result.request.dynamic_font_strikethru_available ||
         !result.request.dynamic_font_strikethru_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DynamicFontStrikethruTitle"),
            "--dynamic-font-strikethru-object")};
    }

if (result.request.dynamic_font_outline_object && !result.request.dynamic_font_outline_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DynamicFontOutline"),
            "--dynamic-font-outline")};
    }

if (result.request.dynamic_font_outline_object && result.request.dynamic_font_outline_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DynamicFontOutline"))};
    }

if (!result.request.dynamic_font_outline_object &&
        (result.request.dynamic_font_outline_available ||
         !result.request.dynamic_font_outline_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DynamicFontOutlineTitle"),
            "--dynamic-font-outline-object")};
    }

if (result.request.dynamic_font_shadow_object && !result.request.dynamic_font_shadow_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DynamicFontShadow"),
            "--dynamic-font-shadow")};
    }

if (result.request.dynamic_font_shadow_object && result.request.dynamic_font_shadow_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DynamicFontShadow"))};
    }

if (!result.request.dynamic_font_shadow_object &&
        (result.request.dynamic_font_shadow_available ||
         !result.request.dynamic_font_shadow_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DynamicFontShadowTitle"),
            "--dynamic-font-shadow-object")};
    }

if (result.request.font_name_object && !result.request.font_name_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.FontName"),
            "--font-name")};
    }

if (result.request.font_name_object && result.request.font_name_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.FontName"))};
    }

if (!result.request.font_name_object &&
        (result.request.font_name_available ||
         !result.request.font_name_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.FontNameTitle"),
            "--font-name-object")};
    }

if (result.request.font_size_object && !result.request.font_size_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.FontSize"),
            "--font-size")};
    }

if (result.request.font_size_object && result.request.font_size_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.FontSize"))};
    }

if (!result.request.font_size_object &&
        (result.request.font_size_available ||
         !result.request.font_size_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.FontSizeTitle"),
            "--font-size-object")};
    }

if (result.request.font_bold_object && !result.request.font_bold_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.FontBold"),
            "--font-bold")};
    }

if (result.request.font_bold_object && result.request.font_bold_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.FontBold"))};
    }

if (!result.request.font_bold_object &&
        (result.request.font_bold_available ||
         !result.request.font_bold_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.FontBoldTitle"),
            "--font-bold-object")};
    }

if (result.request.font_italic_object && !result.request.font_italic_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.FontItalic"),
            "--font-italic")};
    }

if (result.request.font_italic_object && result.request.font_italic_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.FontItalic"))};
    }

if (!result.request.font_italic_object &&
        (result.request.font_italic_available ||
         !result.request.font_italic_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.FontItalicTitle"),
            "--font-italic-object")};
    }

if (result.request.font_underline_object && !result.request.font_underline_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.FontUnderline"),
            "--font-underline")};
    }

if (result.request.font_underline_object && result.request.font_underline_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.FontUnderline"))};
    }

if (!result.request.font_underline_object &&
        (result.request.font_underline_available ||
         !result.request.font_underline_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.FontUnderlineTitle"),
            "--font-underline-object")};
    }

if (result.request.font_strikethru_object && !result.request.font_strikethru_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.FontStrikethru"),
            "--font-strikethru")};
    }

if (result.request.font_strikethru_object && result.request.font_strikethru_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.FontStrikethru"))};
    }

if (!result.request.font_strikethru_object &&
        (result.request.font_strikethru_available ||
         !result.request.font_strikethru_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.FontStrikethruTitle"),
            "--font-strikethru-object")};
    }

if (result.request.font_outline_object && !result.request.font_outline_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.FontOutline"),
            "--font-outline")};
    }

if (result.request.font_outline_object && result.request.font_outline_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.FontOutline"))};
    }

if (!result.request.font_outline_object &&
        (result.request.font_outline_available ||
         !result.request.font_outline_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.FontOutlineTitle"),
            "--font-outline-object")};
    }

if (result.request.font_shadow_object && !result.request.font_shadow_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.FontShadow"),
            "--font-shadow")};
    }

if (result.request.font_shadow_object && result.request.font_shadow_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.FontShadow"))};
    }

if (!result.request.font_shadow_object &&
        (result.request.font_shadow_available ||
         !result.request.font_shadow_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.FontShadowTitle"),
            "--font-shadow-object")};
    }
    return std::nullopt;
}

}  // namespace copperfin::studio
