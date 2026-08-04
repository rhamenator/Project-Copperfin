// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "vs_launch_contract_internal.h"

namespace copperfin::studio {

std::optional<LaunchParseResult> validate_setters_behavior(
    const LaunchParseResult& result,
    const localization::LocalizedCatalog& catalog) {
if (result.request.tab_order_object && !result.request.starting_tab_index_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.TabOrder"),
            "--starting-tab-index")};
    }

if (result.request.tab_order_object && result.request.tab_order_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.TabOrder"))};
    }

if (result.request.tab_order_object && result.request.starting_tab_index < 0) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_non_negative_value(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.TabOrder"),
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.StartingTabIndex"))};
    }

if (!result.request.tab_order_object &&
        (result.request.starting_tab_index_available ||
         !result.request.tab_order_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.TabOrderTitle"),
            "--tab-order-object")};
    }

if (result.request.tab_stop_object && !result.request.tab_stop_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.TabStop"),
            "--tab-stop")};
    }

if (result.request.tab_stop_object && result.request.tab_stop_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.TabStop"))};
    }

if (!result.request.tab_stop_object &&
        (result.request.tab_stop_available ||
         !result.request.tab_stop_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.TabStopTitle"),
            "--tab-stop-object")};
    }

if (result.request.visibility_object && !result.request.visible_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.Visibility"),
            "--visible")};
    }

if (result.request.visibility_object && result.request.visibility_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.Visibility"))};
    }

if (!result.request.visibility_object &&
        (result.request.visible_available ||
         !result.request.visibility_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.VisibilityTitle"),
            "--visibility-object")};
    }

if (result.request.caption_object && !result.request.caption_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.Caption"),
            "--caption")};
    }

if (result.request.caption_object && result.request.caption_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.Caption"))};
    }

if (!result.request.caption_object &&
        (result.request.caption_available ||
         !result.request.caption_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.CaptionTitle"),
            "--caption-object")};
    }

if (result.request.list_item_id_object && !result.request.list_item_id_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ListItemId"),
            "--list-item-id")};
    }

if (result.request.list_item_id_object && result.request.list_item_id_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ListItemId"))};
    }

if (!result.request.list_item_id_object &&
        (result.request.list_item_id_available ||
         !result.request.list_item_id_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ListItemIdTitle"),
            "--list-item-id-object")};
    }

if (const auto tab_orientation_error = validate_tab_orientation_request(result.request, catalog)) {
        return LaunchParseResult{.ok = false, .error = *tab_orientation_error};
    }

if (const auto display_orientation_error = validate_display_orientation_request(result.request, catalog)) {
        return LaunchParseResult{.ok = false, .error = *display_orientation_error};
    }

if (const auto help_context_id_error = validate_help_context_id_request(result.request, catalog)) {
        return LaunchParseResult{.ok = false, .error = *help_context_id_error};
    }

if (const auto whats_this_help_id_error = validate_whats_this_help_id_request(result.request, catalog)) {
        return LaunchParseResult{.ok = false, .error = *whats_this_help_id_error};
    }

if (const auto whats_this_help_error = validate_whats_this_help_request(result.request, catalog)) {
        return LaunchParseResult{.ok = false, .error = *whats_this_help_error};
    }

if (const auto whats_this_button_error = validate_whats_this_button_request(result.request, catalog)) {
        return LaunchParseResult{.ok = false, .error = *whats_this_button_error};
    }

if (result.request.tooltip_text_object && !result.request.tooltip_text_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.TooltipText"),
            "--tooltip-text")};
    }

if (result.request.tooltip_text_object && result.request.tooltip_text_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.TooltipText"))};
    }

if (!result.request.tooltip_text_object &&
        (result.request.tooltip_text_available ||
         !result.request.tooltip_text_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.TooltipTextTitle"),
            "--tooltip-text-object")};
    }

if (result.request.status_bar_text_object && !result.request.status_bar_text_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.StatusBarText"),
            "--status-bar-text")};
    }

if (result.request.status_bar_text_object && result.request.status_bar_text_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.StatusBarText"))};
    }

if (!result.request.status_bar_text_object &&
        (result.request.status_bar_text_available ||
         !result.request.status_bar_text_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.StatusBarTextTitle"),
            "--status-bar-text-object")};
    }

if (result.request.control_source_object && !result.request.control_source_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ControlSource"),
            "--control-source")};
    }

if (result.request.control_source_object && result.request.control_source_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ControlSource"))};
    }

if (!result.request.control_source_object &&
        (result.request.control_source_available ||
         !result.request.control_source_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ControlSourceTitle"),
            "--control-source-object")};
    }

if (result.request.current_control_object && !result.request.current_control_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.CurrentControl"),
            "--current-control")};
    }

if (result.request.current_control_object && result.request.current_control_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.CurrentControl"))};
    }

if (!result.request.current_control_object &&
        (result.request.current_control_available ||
         !result.request.current_control_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.CurrentControlTitle"),
            "--current-control-object")};
    }

if (result.request.closable_object && !result.request.closable_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.Closable"),
            "--closable")};
    }

if (result.request.closable_object && result.request.closable_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.Closable"))};
    }

if (!result.request.closable_object &&
        (result.request.closable_available ||
         !result.request.closable_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ClosableTitle"),
            "--closable-object")};
    }

if (result.request.control_box_object && !result.request.control_box_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ControlBox"),
            "--control-box")};
    }

if (result.request.control_box_object && result.request.control_box_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ControlBox"))};
    }

if (!result.request.control_box_object &&
        (result.request.control_box_available ||
         !result.request.control_box_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ControlBoxTitle"),
            "--control-box-object")};
    }

if (result.request.allow_output_object && !result.request.allow_output_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.AllowOutput"),
            "--allow-output")};
    }

if (result.request.allow_output_object && result.request.allow_output_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.AllowOutput"))};
    }

if (!result.request.allow_output_object &&
        (result.request.allow_output_available ||
         !result.request.allow_output_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.AllowOutputTitle"),
            "--allow-output-object")};
    }

if (result.request.bind_controls_object && !result.request.bind_controls_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.BindControls"),
            "--bind-controls")};
    }

if (result.request.bind_controls_object && result.request.bind_controls_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.BindControls"))};
    }

if (!result.request.bind_controls_object &&
        (result.request.bind_controls_available ||
         !result.request.bind_controls_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.BindControlsTitle"),
            "--bind-controls-object")};
    }

if (result.request.auto_verb_menu_object && !result.request.auto_verb_menu_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.AutoVerbMenu"),
            "--auto-verb-menu")};
    }

if (result.request.auto_verb_menu_object && result.request.auto_verb_menu_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.AutoVerbMenu"))};
    }

if (!result.request.auto_verb_menu_object &&
        (result.request.auto_verb_menu_available ||
         !result.request.auto_verb_menu_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.AutoVerbMenuTitle"),
            "--auto-verb-menu-object")};
    }

if (result.request.desktop_object && !result.request.desktop_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.Desktop"),
            "--desktop")};
    }

if (result.request.desktop_object && result.request.desktop_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.Desktop"))};
    }

if (!result.request.desktop_object &&
        (result.request.desktop_available ||
         !result.request.desktop_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DesktopTitle"),
            "--desktop-object")};
    }

if (result.request.key_preview_object && !result.request.key_preview_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.KeyPreview"),
            "--key-preview")};
    }

if (result.request.key_preview_object && result.request.key_preview_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.KeyPreview"))};
    }

if (!result.request.key_preview_object &&
        (result.request.key_preview_available ||
         !result.request.key_preview_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.KeyPreviewTitle"),
            "--key-preview-object")};
    }

if (result.request.mac_desktop_object && !result.request.mac_desktop_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.MacDesktop"),
            "--mac-desktop")};
    }

if (result.request.mac_desktop_object && result.request.mac_desktop_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.MacDesktop"))};
    }

if (!result.request.mac_desktop_object &&
        (result.request.mac_desktop_available ||
         !result.request.mac_desktop_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.MacDesktopTitle"),
            "--mac-desktop-object")};
    }

if (result.request.max_button_object && !result.request.max_button_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.MaxButton"),
            "--max-button")};
    }

if (result.request.max_button_object && result.request.max_button_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.MaxButton"))};
    }

if (!result.request.max_button_object &&
        (result.request.max_button_available ||
         !result.request.max_button_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.MaxButtonTitle"),
            "--max-button-object")};
    }

if (result.request.min_button_object && !result.request.min_button_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.MinButton"),
            "--min-button")};
    }

if (result.request.min_button_object && result.request.min_button_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.MinButton"))};
    }

if (!result.request.min_button_object &&
        (result.request.min_button_available ||
         !result.request.min_button_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.MinButtonTitle"),
            "--min-button-object")};
    }

if (result.request.min_height_object && !result.request.min_height_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.MinHeight"),
            "--min-height")};
    }

if (result.request.min_height_object && result.request.min_height_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.MinHeight"))};
    }

if (!result.request.min_height_object &&
        (result.request.min_height_available ||
         !result.request.min_height_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.MinHeightTitle"),
            "--min-height-object")};
    }

if (result.request.min_width_object && !result.request.min_width_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.MinWidth"),
            "--min-width")};
    }

if (result.request.min_width_object && result.request.min_width_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.MinWidth"))};
    }

if (!result.request.min_width_object &&
        (result.request.min_width_available ||
         !result.request.min_width_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.MinWidthTitle"),
            "--min-width-object")};
    }

if (result.request.max_height_object && !result.request.max_height_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.MaxHeight"),
            "--max-height")};
    }

if (result.request.max_height_object && result.request.max_height_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.MaxHeight"))};
    }

if (!result.request.max_height_object &&
        (result.request.max_height_available ||
         !result.request.max_height_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.MaxHeightTitle"),
            "--max-height-object")};
    }

if (result.request.movable_object && !result.request.movable_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.Movable"),
            "--movable")};
    }

if (result.request.movable_object && result.request.movable_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.Movable"))};
    }

if (!result.request.movable_object &&
        (result.request.movable_available ||
         !result.request.movable_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.MovableTitle"),
            "--movable-object")};
    }

if (result.request.half_height_caption_object && !result.request.half_height_caption_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.HalfHeightCaption"),
            "--half-height-caption")};
    }

if (result.request.half_height_caption_object && result.request.half_height_caption_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.HalfHeightCaption"))};
    }

if (!result.request.half_height_caption_object &&
        (result.request.half_height_caption_available ||
         !result.request.half_height_caption_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.HalfHeightCaptionTitle"),
            "--half-height-caption-object")};
    }

if (result.request.mdi_form_object && !result.request.mdi_form_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.MdiForm"),
            "--mdi-form")};
    }

if (result.request.mdi_form_object && result.request.mdi_form_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.MdiForm"))};
    }

if (!result.request.mdi_form_object &&
        (result.request.mdi_form_available ||
         !result.request.mdi_form_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.MdiFormTitle"),
            "--mdi-form-object")};
    }

if (result.request.max_width_object && !result.request.max_width_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.MaxWidth"),
            "--max-width")};
    }

if (result.request.max_width_object && result.request.max_width_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.MaxWidth"))};
    }

if (!result.request.max_width_object &&
        (result.request.max_width_available ||
         !result.request.max_width_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.MaxWidthTitle"),
            "--max-width-object")};
    }

if (result.request.max_left_object && !result.request.max_left_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.MaxLeft"),
            "--max-left")};
    }

if (result.request.max_left_object && result.request.max_left_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.MaxLeft"))};
    }

if (!result.request.max_left_object &&
        (result.request.max_left_available ||
         !result.request.max_left_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.MaxLeftTitle"),
            "--max-left-object")};
    }

if (result.request.max_top_object && !result.request.max_top_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.MaxTop"),
            "--max-top")};
    }

if (result.request.max_top_object && result.request.max_top_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.MaxTop"))};
    }

if (!result.request.max_top_object &&
        (result.request.max_top_available ||
         !result.request.max_top_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.MaxTopTitle"),
            "--max-top-object")};
    }

if (result.request.auto_center_object && !result.request.auto_center_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.AutoCenter"),
            "--auto-center")};
    }

if (result.request.auto_center_object && result.request.auto_center_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.AutoCenter"))};
    }

if (!result.request.auto_center_object &&
        (result.request.auto_center_available ||
         !result.request.auto_center_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.AutoCenterTitle"),
            "--auto-center-object")};
    }

if (result.request.auto_size_object && !result.request.auto_size_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.AutoSize"),
            "--auto-size")};
    }

if (result.request.auto_size_object && result.request.auto_size_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.AutoSize"))};
    }

if (!result.request.auto_size_object &&
        (result.request.auto_size_available ||
         !result.request.auto_size_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.AutoSizeTitle"),
            "--auto-size-object")};
    }

if (result.request.auto_release_object && !result.request.auto_release_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.AutoRelease"),
            "--auto-release")};
    }

if (result.request.auto_release_object && result.request.auto_release_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.AutoRelease"))};
    }

if (!result.request.auto_release_object &&
        (result.request.auto_release_available ||
         !result.request.auto_release_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.AutoReleaseTitle"),
            "--auto-release-object")};
    }

if (result.request.continuous_scroll_object && !result.request.continuous_scroll_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ContinuousScroll"),
            "--continuous-scroll")};
    }

if (result.request.continuous_scroll_object && result.request.continuous_scroll_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ContinuousScroll"))};
    }

if (!result.request.continuous_scroll_object &&
        (result.request.continuous_scroll_available ||
         !result.request.continuous_scroll_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ContinuousScrollTitle"),
            "--continuous-scroll-object")};
    }

if (result.request.dockable_object && !result.request.dockable_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.Dockable"),
            "--dockable")};
    }

if (result.request.dockable_object && result.request.dockable_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.Dockable"))};
    }

if (!result.request.dockable_object &&
        (result.request.dockable_available ||
         !result.request.dockable_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DockableTitle"),
            "--dockable-object")};
    }

if (result.request.clip_controls_object && !result.request.clip_controls_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ClipControls"),
            "--clip-controls")};
    }

if (result.request.clip_controls_object && result.request.clip_controls_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ClipControls"))};
    }

if (!result.request.clip_controls_object &&
        (result.request.clip_controls_available ||
         !result.request.clip_controls_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ClipControlsTitle"),
            "--clip-controls-object")};
    }

if (result.request.sparse_object && !result.request.sparse_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.Sparse"),
            "--sparse")};
    }

if (result.request.sparse_object && result.request.sparse_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.Sparse"))};
    }

if (!result.request.sparse_object &&
        (result.request.sparse_available ||
         !result.request.sparse_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.SparseTitle"),
            "--sparse-object")};
    }

if (result.request.lock_screen_object && !result.request.lock_screen_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.LockScreen"),
            "--lock-screen")};
    }

if (result.request.lock_screen_object && result.request.lock_screen_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.LockScreen"))};
    }

if (!result.request.lock_screen_object &&
        (result.request.lock_screen_available ||
         !result.request.lock_screen_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.LockScreenTitle"),
            "--lock-screen-object")};
    }

if (result.request.allow_cell_selection_object && !result.request.allow_cell_selection_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.AllowCellSelection"),
            "--allow-cell-selection")};
    }

if (result.request.allow_cell_selection_object && result.request.allow_cell_selection_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.AllowCellSelection"))};
    }

if (!result.request.allow_cell_selection_object &&
        (result.request.allow_cell_selection_available ||
         !result.request.allow_cell_selection_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.AllowCellSelectionTitle"),
            "--allow-cell-selection-object")};
    }

if (result.request.hide_selection_object && !result.request.hide_selection_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.HideSelection"),
            "--hide-selection")};
    }

if (result.request.hide_selection_object && result.request.hide_selection_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.HideSelection"))};
    }

if (!result.request.hide_selection_object &&
        (result.request.hide_selection_available ||
         !result.request.hide_selection_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.HideSelectionTitle"),
            "--hide-selection-object")};
    }

if (result.request.delete_mark_object && !result.request.delete_mark_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DeleteMark"),
            "--delete-mark")};
    }

if (result.request.delete_mark_object && result.request.delete_mark_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DeleteMark"))};
    }

if (!result.request.delete_mark_object &&
        (result.request.delete_mark_available ||
         !result.request.delete_mark_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DeleteMarkTitle"),
            "--delete-mark-object")};
    }

if (result.request.record_mark_object && !result.request.record_mark_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.RecordMark"),
            "--record-mark")};
    }

if (result.request.record_mark_object && result.request.record_mark_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.RecordMark"))};
    }

if (!result.request.record_mark_object &&
        (result.request.record_mark_available ||
         !result.request.record_mark_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.RecordMarkTitle"),
            "--record-mark-object")};
    }

if (result.request.split_bar_object && !result.request.split_bar_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.SplitBar"),
            "--split-bar")};
    }

if (result.request.split_bar_object && result.request.split_bar_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.SplitBar"))};
    }

if (!result.request.split_bar_object &&
        (result.request.split_bar_available ||
         !result.request.split_bar_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.SplitBarTitle"),
            "--split-bar-object")};
    }

if (result.request.highlight_row_object && !result.request.highlight_row_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.HighlightRow"),
            "--highlight-row")};
    }

if (result.request.highlight_row_object && result.request.highlight_row_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.HighlightRow"))};
    }

if (!result.request.highlight_row_object &&
        (result.request.highlight_row_available ||
         !result.request.highlight_row_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.HighlightRowTitle"),
            "--highlight-row-object")};
    }

if (result.request.panel_link_object && !result.request.panel_link_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.PanelLink"),
            "--panel-link")};
    }

if (result.request.panel_link_object && result.request.panel_link_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.PanelLink"))};
    }

if (!result.request.panel_link_object &&
        (result.request.panel_link_available ||
         !result.request.panel_link_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.PanelLinkTitle"),
            "--panel-link-object")};
    }

if (result.request.allow_header_sizing_object && !result.request.allow_header_sizing_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.AllowHeaderSizing"),
            "--allow-header-sizing")};
    }

if (result.request.allow_header_sizing_object && result.request.allow_header_sizing_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.AllowHeaderSizing"))};
    }

if (!result.request.allow_header_sizing_object &&
        (result.request.allow_header_sizing_available ||
         !result.request.allow_header_sizing_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.AllowHeaderSizingTitle"),
            "--allow-header-sizing-object")};
    }

if (result.request.allow_row_sizing_object && !result.request.allow_row_sizing_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.AllowRowSizing"),
            "--allow-row-sizing")};
    }

if (result.request.allow_row_sizing_object && result.request.allow_row_sizing_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.AllowRowSizing"))};
    }

if (!result.request.allow_row_sizing_object &&
        (result.request.allow_row_sizing_available ||
         !result.request.allow_row_sizing_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.AllowRowSizingTitle"),
            "--allow-row-sizing-object")};
    }

if (result.request.resizable_object && !result.request.resizable_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.Resizable"),
            "--resizable")};
    }

if (result.request.resizable_object && result.request.resizable_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.Resizable"))};
    }

if (!result.request.resizable_object &&
        (result.request.resizable_available ||
         !result.request.resizable_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ResizableTitle"),
            "--resizable-object")};
    }

if (result.request.add_line_feeds_object && !result.request.add_line_feeds_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.AddLineFeeds"),
            "--add-line-feeds")};
    }

if (result.request.add_line_feeds_object && result.request.add_line_feeds_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.AddLineFeeds"))};
    }

if (!result.request.add_line_feeds_object &&
        (result.request.add_line_feeds_available ||
         !result.request.add_line_feeds_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.AddLineFeedsTitle"),
            "--add-line-feeds-object")};
    }

if (result.request.always_on_top_object && !result.request.always_on_top_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.AlwaysOnTop"),
            "--always-on-top")};
    }

if (result.request.always_on_top_object && result.request.always_on_top_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.AlwaysOnTop"))};
    }

if (!result.request.always_on_top_object &&
        (result.request.always_on_top_available ||
         !result.request.always_on_top_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.AlwaysOnTopTitle"),
            "--always-on-top-object")};
    }

if (result.request.always_on_bottom_object && !result.request.always_on_bottom_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.AlwaysOnBottom"),
            "--always-on-bottom")};
    }

if (result.request.always_on_bottom_object && result.request.always_on_bottom_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.AlwaysOnBottom"))};
    }

if (!result.request.always_on_bottom_object &&
        (result.request.always_on_bottom_available ||
         !result.request.always_on_bottom_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.AlwaysOnBottomTitle"),
            "--always-on-bottom-object")};
    }
    return std::nullopt;
}

}  // namespace copperfin::studio
