// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "vs_launch_contract_internal.h"

namespace copperfin::studio {

std::optional<LaunchParseResult> validate_setters_data(
    const LaunchParseResult& result,
    const localization::LocalizedCatalog& catalog) {
if (result.request.button_count_object && !result.request.button_count_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ButtonCount"),
            "--button-count")};
    }

if (result.request.button_count_object && result.request.button_count_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ButtonCount"))};
    }

if (!result.request.button_count_object &&
        (result.request.button_count_available ||
         !result.request.button_count_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ButtonCountTitle"),
            "--button-count-object")};
    }

if (result.request.data_session_object && !result.request.data_session_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DataSession"),
            "--data-session")};
    }

if (result.request.data_session_object && result.request.data_session_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DataSession"))};
    }

if (!result.request.data_session_object &&
        (result.request.data_session_available ||
         !result.request.data_session_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DataSessionTitle"),
            "--data-session-object")};
    }

if (result.request.lock_columns_object && !result.request.lock_columns_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.LockColumns"),
            "--lock-columns")};
    }

if (result.request.lock_columns_object && result.request.lock_columns_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.LockColumns"))};
    }

if (!result.request.lock_columns_object &&
        (result.request.lock_columns_available ||
         !result.request.lock_columns_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.LockColumnsTitle"),
            "--lock-columns-object")};
    }

if (result.request.lock_columns_left_object && !result.request.lock_columns_left_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.LockColumnsLeft"),
            "--lock-columns-left")};
    }

if (result.request.lock_columns_left_object && result.request.lock_columns_left_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.LockColumnsLeft"))};
    }

if (!result.request.lock_columns_left_object &&
        (result.request.lock_columns_left_available ||
         !result.request.lock_columns_left_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.LockColumnsLeftTitle"),
            "--lock-columns-left-object")};
    }

if (result.request.partition_object && !result.request.partition_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.Partition"),
            "--partition")};
    }

if (result.request.partition_object && result.request.partition_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.Partition"))};
    }

if (!result.request.partition_object &&
        (result.request.partition_available ||
         !result.request.partition_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.PartitionTitle"),
            "--partition-object")};
    }

if (result.request.record_source_type_object && !result.request.record_source_type_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.RecordSourceType"),
            "--record-source-type")};
    }

if (result.request.record_source_type_object && result.request.record_source_type_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.RecordSourceType"))};
    }

if (!result.request.record_source_type_object &&
        (result.request.record_source_type_available ||
         !result.request.record_source_type_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.RecordSourceTypeTitle"),
            "--record-source-type-object")};
    }

if (result.request.column_order_object && !result.request.column_order_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ColumnOrder"),
            "--column-order")};
    }

if (result.request.column_order_object && result.request.column_order_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ColumnOrder"))};
    }

if (!result.request.column_order_object &&
        (result.request.column_order_available ||
         !result.request.column_order_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ColumnOrderTitle"),
            "--column-order-object")};
    }

if (result.request.child_order_object && !result.request.child_order_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ChildOrder"),
            "--child-order")};
    }

if (result.request.child_order_object && result.request.child_order_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ChildOrder"))};
    }

if (!result.request.child_order_object &&
        (result.request.child_order_available ||
         !result.request.child_order_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ChildOrderTitle"),
            "--child-order-object")};
    }

if (result.request.record_source_object && !result.request.record_source_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.RecordSource"),
            "--record-source")};
    }

if (result.request.record_source_object && result.request.record_source_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.RecordSource"))};
    }

if (!result.request.record_source_object &&
        (result.request.record_source_available ||
         !result.request.record_source_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.RecordSourceTitle"),
            "--record-source-object")};
    }

if (const auto form_set_class_error = validate_form_set_class_request(result.request, catalog)) {
        return LaunchParseResult{.ok = false, .error = *form_set_class_error};
    }

if (const auto default_file_path_error = validate_default_file_path_request(result.request, catalog)) {
        return LaunchParseResult{.ok = false, .error = *default_file_path_error};
    }

if (const auto initial_selected_alias_error = validate_initial_selected_alias_request(result.request, catalog)) {
        return LaunchParseResult{.ok = false, .error = *initial_selected_alias_error};
    }

if (result.request.link_master_object && !result.request.link_master_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.LinkMaster"),
            "--link-master")};
    }

if (result.request.link_master_object && result.request.link_master_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.LinkMaster"))};
    }

if (!result.request.link_master_object &&
        (result.request.link_master_available ||
         !result.request.link_master_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.LinkMasterTitle"),
            "--link-master-object")};
    }

if (result.request.row_source_object && !result.request.row_source_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.RowSource"),
            "--row-source")};
    }

if (result.request.row_source_object && result.request.row_source_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.RowSource"))};
    }

if (!result.request.row_source_object &&
        (result.request.row_source_available ||
         !result.request.row_source_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.RowSourceTitle"),
            "--row-source-object")};
    }

if (result.request.column_widths_object && !result.request.column_widths_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ColumnWidths"),
            "--column-widths")};
    }

if (result.request.column_widths_object && result.request.column_widths_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ColumnWidths"))};
    }

if (!result.request.column_widths_object &&
        (result.request.column_widths_available ||
         !result.request.column_widths_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ColumnWidthsTitle"),
            "--column-widths-object")};
    }

if (result.request.column_lines_object && !result.request.column_lines_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ColumnLines"),
            "--column-lines")};
    }

if (result.request.column_lines_object && result.request.column_lines_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ColumnLines"))};
    }

if (!result.request.column_lines_object &&
        (result.request.column_lines_available ||
         !result.request.column_lines_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ColumnLinesTitle"),
            "--column-lines-object")};
    }

if (result.request.integral_height_object && !result.request.integral_height_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.IntegralHeight"),
            "--integral-height")};
    }

if (result.request.integral_height_object && result.request.integral_height_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.IntegralHeight"))};
    }

if (!result.request.integral_height_object &&
        (result.request.integral_height_available ||
         !result.request.integral_height_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.IntegralHeightTitle"),
            "--integral-height-object")};
    }

if (result.request.incremental_search_object && !result.request.incremental_search_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.IncrementalSearch"),
            "--incremental-search")};
    }

if (result.request.incremental_search_object && result.request.incremental_search_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.IncrementalSearch"))};
    }

if (!result.request.incremental_search_object &&
        (result.request.incremental_search_available ||
         !result.request.incremental_search_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.IncrementalSearchTitle"),
            "--incremental-search-object")};
    }

if (result.request.multi_select_object && !result.request.multi_select_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.MultiSelect"),
            "--multi-select")};
    }

if (result.request.multi_select_object && result.request.multi_select_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.MultiSelect"))};
    }

if (!result.request.multi_select_object &&
        (result.request.multi_select_available ||
         !result.request.multi_select_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.MultiSelectTitle"),
            "--multi-select-object")};
    }

if (result.request.row_source_type_object && !result.request.row_source_type_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.RowSourceType"),
            "--row-source-type")};
    }

if (result.request.row_source_type_object && result.request.row_source_type < 0) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_non_negative_value(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.RowSourceType"),
            catalog.translate("StudioHost.LaunchParse.Value.Value"))};
    }

if (result.request.row_source_type_object && result.request.row_source_type_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.RowSourceType"))};
    }

if (!result.request.row_source_type_object &&
        (result.request.row_source_type_available ||
         !result.request.row_source_type_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.RowSourceTypeTitle"),
            "--row-source-type-object")};
    }

if (result.request.bound_column_object && !result.request.bound_column_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.BoundColumn"),
            "--bound-column")};
    }

if (result.request.bound_column_object && result.request.bound_column < 0) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_non_negative_value(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.BoundColumn"),
            catalog.translate("StudioHost.LaunchParse.Value.Value"))};
    }

if (result.request.bound_column_object && result.request.bound_column_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.BoundColumn"))};
    }

if (!result.request.bound_column_object &&
        (result.request.bound_column_available ||
         !result.request.bound_column_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.BoundColumnTitle"),
            "--bound-column-object")};
    }

if (result.request.column_count_object && !result.request.column_count_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ColumnCount"),
            "--column-count")};
    }

if (result.request.column_count_object && result.request.column_count < 0) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_non_negative_value(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ColumnCount"),
            catalog.translate("StudioHost.LaunchParse.Value.Value"))};
    }

if (result.request.column_count_object && result.request.column_count_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ColumnCount"))};
    }

if (!result.request.column_count_object &&
        (result.request.column_count_available ||
         !result.request.column_count_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ColumnCountTitle"),
            "--column-count-object")};
    }

if (result.request.style_object && !result.request.style_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.Style"),
            "--style")};
    }

if (result.request.style_object && result.request.style < 0) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_non_negative_value(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.Style"),
            catalog.translate("StudioHost.LaunchParse.Value.Value"))};
    }

if (result.request.style_object && result.request.style_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.Style"))};
    }

if (!result.request.style_object &&
        (result.request.style_available ||
         !result.request.style_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.StyleTitle"),
            "--style-object")};
    }

if (result.request.list_index_object && !result.request.list_index_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ListIndex"),
            "--list-index")};
    }

if (result.request.list_index_object && result.request.list_index < 0) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_non_negative_value(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ListIndex"),
            catalog.translate("StudioHost.LaunchParse.Value.Value"))};
    }

if (result.request.list_index_object && result.request.list_index_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ListIndex"))};
    }

if (!result.request.list_index_object &&
        (result.request.list_index_available ||
         !result.request.list_index_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.ListIndexTitle"),
            "--list-index-object")};
    }

if (result.request.left_column_object && !result.request.left_column_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.LeftColumn"),
            "--left-column")};
    }

if (result.request.left_column_object && result.request.left_column < 0) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_non_negative_value(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.LeftColumn"),
            catalog.translate("StudioHost.LaunchParse.Value.Value"))};
    }

if (result.request.left_column_object && result.request.left_column_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.LeftColumn"))};
    }

if (!result.request.left_column_object &&
        (result.request.left_column_available ||
         !result.request.left_column_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.LeftColumnTitle"),
            "--left-column-object")};
    }

if (result.request.display_value_object && !result.request.display_value_available) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DisplayValue"),
            "--display-value")};
    }

if (result.request.display_value_object && result.request.display_value_objects.empty()) {
        return LaunchParseResult{.ok = false, .error = localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DisplayValue"))};
    }

if (!result.request.display_value_object &&
        (result.request.display_value_available ||
         !result.request.display_value_objects.empty())) {
        return LaunchParseResult{.ok = false, .error = localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DisplayValueTitle"),
            "--display-value-object")};
    }
    return std::nullopt;
}

}  // namespace copperfin::studio
