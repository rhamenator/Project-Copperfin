// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "vs_launch_contract_internal.h"

namespace copperfin::studio {

LaunchArgumentDispatchOutcome try_parse_setters_data(
    const std::string& argument,
    const localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args,
    std::size_t& index,
    LaunchParseResult& result,
    std::string& parsed_argument_error) {
if (parse_form_set_class_argument(argument, catalog, args, index, result, parsed_argument_error)) {
            if (!parsed_argument_error.empty()) {
                result = {.ok = false, .error = parsed_argument_error}; return {true, true};
            }
            return {true, false};
        }

if (parse_default_file_path_argument(argument, catalog, args, index, result, parsed_argument_error)) {
            if (!parsed_argument_error.empty()) {
                result = {.ok = false, .error = parsed_argument_error}; return {true, true};
            }
            return {true, false};
        }

if (parse_initial_selected_alias_argument(argument, catalog, args, index, result, parsed_argument_error)) {
            if (!parsed_argument_error.empty()) {
                result = {.ok = false, .error = parsed_argument_error}; return {true, true};
            }
            return {true, false};
        }

if (argument == "--record-source-object") {
            result.request.record_source_object = true;
            return {true, false};
        }

if (argument == "--link-master-object") {
            result.request.link_master_object = true;
            return {true, false};
        }

if (argument == "--row-source-object") {
            result.request.row_source_object = true;
            return {true, false};
        }

if (argument == "--column-widths-object") {
            result.request.column_widths_object = true;
            return {true, false};
        }

if (argument == "--column-lines-object") {
            result.request.column_lines_object = true;
            return {true, false};
        }

if (argument == "--integral-height-object") {
            result.request.integral_height_object = true;
            return {true, false};
        }

if (argument == "--incremental-search-object") {
            result.request.incremental_search_object = true;
            return {true, false};
        }

if (argument == "--multi-select-object") {
            result.request.multi_select_object = true;
            return {true, false};
        }

if (argument == "--row-source-type-object") {
            result.request.row_source_type_object = true;
            return {true, false};
        }

if (argument == "--bound-column-object") {
            result.request.bound_column_object = true;
            return {true, false};
        }

if (argument == "--column-count-object") {
            result.request.column_count_object = true;
            return {true, false};
        }

if (argument == "--style-object") {
            result.request.style_object = true;
            return {true, false};
        }

if (argument == "--list-index-object") {
            result.request.list_index_object = true;
            return {true, false};
        }

if (argument == "--left-column-object") {
            result.request.left_column_object = true;
            return {true, false};
        }

if (argument == "--display-value-object") {
            result.request.display_value_object = true;
            return {true, false};
        }

if (argument == "--button-count-object") {
            result.request.button_count_object = true;
            return {true, false};
        }

if (argument == "--data-session-object") {
            result.request.data_session_object = true;
            return {true, false};
        }

if (argument == "--lock-columns-object") {
            result.request.lock_columns_object = true;
            return {true, false};
        }

if (argument == "--lock-columns-left-object") {
            result.request.lock_columns_left_object = true;
            return {true, false};
        }

if (argument == "--partition-object") {
            result.request.partition_object = true;
            return {true, false};
        }

if (argument == "--record-source-type-object") {
            result.request.record_source_type_object = true;
            return {true, false};
        }

if (argument == "--column-order-object") {
            result.request.column_order_object = true;
            return {true, false};
        }

if (argument == "--child-order-object") {
            result.request.child_order_object = true;
            return {true, false};
        }

if (argument == "--record-source") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--record-source")}; return {true, true};
            }
            result.request.record_source = args[++index];
            result.request.record_source_available = true;
            return {true, false};
        }

if (argument == "--link-master") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--link-master")}; return {true, true};
            }
            result.request.link_master = args[++index];
            result.request.link_master_available = true;
            return {true, false};
        }

if (argument == "--row-source") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--row-source")}; return {true, true};
            }
            result.request.row_source = args[++index];
            result.request.row_source_available = true;
            return {true, false};
        }

if (argument == "--column-widths") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--column-widths")}; return {true, true};
            }
            result.request.column_widths = args[++index];
            result.request.column_widths_available = true;
            return {true, false};
        }

if (argument == "--column-lines") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--column-lines")}; return {true, true};
            }
            const auto column_lines = parse_bool_value(args[++index]);
            if (!column_lines.has_value()) {
                result = {.ok = false, .error = localized_true_false_value_required(catalog, "--column-lines")}; return {true, true};
            }
            result.request.column_lines = *column_lines;
            result.request.column_lines_available = true;
            return {true, false};
        }

if (argument == "--integral-height") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--integral-height")}; return {true, true};
            }
            const auto integral_height = parse_bool_value(args[++index]);
            if (!integral_height.has_value()) {
                result = {.ok = false, .error = localized_true_false_value_required(catalog, "--integral-height")}; return {true, true};
            }
            result.request.integral_height = *integral_height;
            result.request.integral_height_available = true;
            return {true, false};
        }

if (argument == "--incremental-search") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--incremental-search")}; return {true, true};
            }
            const auto incremental_search = parse_bool_value(args[++index]);
            if (!incremental_search.has_value()) {
                result = {.ok = false, .error = localized_true_false_value_required(catalog, "--incremental-search")}; return {true, true};
            }
            result.request.incremental_search = *incremental_search;
            result.request.incremental_search_available = true;
            return {true, false};
        }

if (argument == "--multi-select") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--multi-select")}; return {true, true};
            }
            const auto multi_select = parse_bool_value(args[++index]);
            if (!multi_select.has_value()) {
                result = {.ok = false, .error = localized_true_false_value_required(catalog, "--multi-select")}; return {true, true};
            }
            result.request.multi_select = *multi_select;
            result.request.multi_select_available = true;
            return {true, false};
        }

if (argument == "--row-source-type") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--row-source-type")}; return {true, true};
            }
            int row_source_type = 0;
            if (!parse_int_value(args[++index], row_source_type)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--row-source-type")}; return {true, true};
            }
            result.request.row_source_type = row_source_type;
            result.request.row_source_type_available = true;
            return {true, false};
        }

if (argument == "--bound-column") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--bound-column")}; return {true, true};
            }
            int bound_column = 0;
            if (!parse_int_value(args[++index], bound_column)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--bound-column")}; return {true, true};
            }
            result.request.bound_column = bound_column;
            result.request.bound_column_available = true;
            return {true, false};
        }

if (argument == "--column-count") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--column-count")}; return {true, true};
            }
            int column_count = 0;
            if (!parse_int_value(args[++index], column_count)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--column-count")}; return {true, true};
            }
            result.request.column_count = column_count;
            result.request.column_count_available = true;
            return {true, false};
        }

if (argument == "--style") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--style")}; return {true, true};
            }
            int style = 0;
            if (!parse_int_value(args[++index], style)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--style")}; return {true, true};
            }
            result.request.style = style;
            result.request.style_available = true;
            return {true, false};
        }

if (argument == "--list-index") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--list-index")}; return {true, true};
            }
            int list_index = 0;
            if (!parse_int_value(args[++index], list_index)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--list-index")}; return {true, true};
            }
            result.request.list_index = list_index;
            result.request.list_index_available = true;
            return {true, false};
        }

if (argument == "--left-column") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--left-column")}; return {true, true};
            }
            int left_column = 0;
            if (!parse_int_value(args[++index], left_column)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--left-column")}; return {true, true};
            }
            result.request.left_column = left_column;
            result.request.left_column_available = true;
            return {true, false};
        }

if (argument == "--button-count") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--button-count")}; return {true, true};
            }
            int button_count = 0;
            if (!parse_int_value(args[++index], button_count)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--button-count")}; return {true, true};
            }
            if (button_count < 0) {
                result = {.ok = false, .error = localized_non_negative_value_required(catalog, "--button-count")}; return {true, true};
            }
            result.request.button_count = button_count;
            result.request.button_count_available = true;
            return {true, false};
        }

if (argument == "--data-session") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--data-session")}; return {true, true};
            }
            int data_session = 0;
            if (!parse_int_value(args[++index], data_session)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--data-session")}; return {true, true};
            }
            if (data_session < 0) {
                result = {.ok = false, .error = localized_non_negative_value_required(catalog, "--data-session")}; return {true, true};
            }
            result.request.data_session = data_session;
            result.request.data_session_available = true;
            return {true, false};
        }

if (argument == "--lock-columns") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--lock-columns")}; return {true, true};
            }
            int lock_columns = 0;
            if (!parse_int_value(args[++index], lock_columns)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--lock-columns")}; return {true, true};
            }
            if (lock_columns < 0) {
                result = {.ok = false, .error = localized_non_negative_value_required(catalog, "--lock-columns")}; return {true, true};
            }
            result.request.lock_columns = lock_columns;
            result.request.lock_columns_available = true;
            return {true, false};
        }

if (argument == "--lock-columns-left") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--lock-columns-left")}; return {true, true};
            }
            int lock_columns_left = 0;
            if (!parse_int_value(args[++index], lock_columns_left)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--lock-columns-left")}; return {true, true};
            }
            if (lock_columns_left < 0) {
                result = {.ok = false, .error = localized_non_negative_value_required(catalog, "--lock-columns-left")}; return {true, true};
            }
            result.request.lock_columns_left = lock_columns_left;
            result.request.lock_columns_left_available = true;
            return {true, false};
        }

if (argument == "--partition") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--partition")}; return {true, true};
            }
            int partition = 0;
            if (!parse_int_value(args[++index], partition)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--partition")}; return {true, true};
            }
            if (partition < 0) {
                result = {.ok = false, .error = localized_non_negative_value_required(catalog, "--partition")}; return {true, true};
            }
            result.request.partition = partition;
            result.request.partition_available = true;
            return {true, false};
        }

if (argument == "--record-source-type") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--record-source-type")}; return {true, true};
            }
            int record_source_type = 0;
            if (!parse_int_value(args[++index], record_source_type)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--record-source-type")}; return {true, true};
            }
            if (record_source_type < 0) {
                result = {.ok = false, .error = localized_non_negative_value_required(catalog, "--record-source-type")}; return {true, true};
            }
            result.request.record_source_type = record_source_type;
            result.request.record_source_type_available = true;
            return {true, false};
        }

if (argument == "--column-order") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--column-order")}; return {true, true};
            }
            int column_order = 0;
            if (!parse_int_value(args[++index], column_order)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--column-order")}; return {true, true};
            }
            if (column_order < 0) {
                result = {.ok = false, .error = localized_non_negative_value_required(catalog, "--column-order")}; return {true, true};
            }
            result.request.column_order = column_order;
            result.request.column_order_available = true;
            return {true, false};
        }

if (argument == "--child-order") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--child-order")}; return {true, true};
            }
            int child_order = 0;
            if (!parse_int_value(args[++index], child_order)) {
                result = {.ok = false, .error = localized_integer_value_required(catalog, "--child-order")}; return {true, true};
            }
            if (child_order < 0) {
                result = {.ok = false, .error = localized_non_negative_value_required(catalog, "--child-order")}; return {true, true};
            }
            result.request.child_order = child_order;
            result.request.child_order_available = true;
            return {true, false};
        }

if (argument == "--display-value") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--display-value")}; return {true, true};
            }
            result.request.display_value = args[++index];
            result.request.display_value_available = true;
            return {true, false};
        }

if (argument == "--button-count-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--button-count-target-object-name")}; return {true, true};
            }
            result.request.button_count_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--button-count-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--button-count-target-unique-id")}; return {true, true};
            }
            result.request.button_count_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--data-session-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--data-session-target-object-name")}; return {true, true};
            }
            result.request.data_session_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--data-session-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--data-session-target-unique-id")}; return {true, true};
            }
            result.request.data_session_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--lock-columns-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--lock-columns-target-object-name")}; return {true, true};
            }
            result.request.lock_columns_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--lock-columns-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--lock-columns-target-unique-id")}; return {true, true};
            }
            result.request.lock_columns_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--lock-columns-left-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--lock-columns-left-target-object-name")}; return {true, true};
            }
            result.request.lock_columns_left_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--lock-columns-left-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--lock-columns-left-target-unique-id")}; return {true, true};
            }
            result.request.lock_columns_left_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--partition-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--partition-target-object-name")}; return {true, true};
            }
            result.request.partition_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--partition-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--partition-target-unique-id")}; return {true, true};
            }
            result.request.partition_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--record-source-type-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--record-source-type-target-object-name")}; return {true, true};
            }
            result.request.record_source_type_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--record-source-type-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--record-source-type-target-unique-id")}; return {true, true};
            }
            result.request.record_source_type_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--column-order-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--column-order-target-object-name")}; return {true, true};
            }
            result.request.column_order_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--column-order-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--column-order-target-unique-id")}; return {true, true};
            }
            result.request.column_order_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--child-order-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--child-order-target-object-name")}; return {true, true};
            }
            result.request.child_order_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--child-order-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--child-order-target-unique-id")}; return {true, true};
            }
            result.request.child_order_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--record-source-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--record-source-target-object-name")}; return {true, true};
            }
            result.request.record_source_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--record-source-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--record-source-target-unique-id")}; return {true, true};
            }
            result.request.record_source_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--link-master-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--link-master-target-object-name")}; return {true, true};
            }
            result.request.link_master_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--link-master-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--link-master-target-unique-id")}; return {true, true};
            }
            result.request.link_master_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--row-source-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--row-source-target-object-name")}; return {true, true};
            }
            result.request.row_source_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--row-source-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--row-source-target-unique-id")}; return {true, true};
            }
            result.request.row_source_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--column-widths-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--column-widths-target-object-name")}; return {true, true};
            }
            result.request.column_widths_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--column-widths-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--column-widths-target-unique-id")}; return {true, true};
            }
            result.request.column_widths_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--column-lines-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--column-lines-target-object-name")}; return {true, true};
            }
            result.request.column_lines_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--column-lines-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--column-lines-target-unique-id")}; return {true, true};
            }
            result.request.column_lines_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--integral-height-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--integral-height-target-object-name")}; return {true, true};
            }
            result.request.integral_height_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--integral-height-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--integral-height-target-unique-id")}; return {true, true};
            }
            result.request.integral_height_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--incremental-search-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--incremental-search-target-object-name")}; return {true, true};
            }
            result.request.incremental_search_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--incremental-search-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--incremental-search-target-unique-id")}; return {true, true};
            }
            result.request.incremental_search_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--multi-select-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--multi-select-target-object-name")}; return {true, true};
            }
            result.request.multi_select_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--multi-select-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--multi-select-target-unique-id")}; return {true, true};
            }
            result.request.multi_select_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--row-source-type-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--row-source-type-target-object-name")}; return {true, true};
            }
            result.request.row_source_type_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--row-source-type-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--row-source-type-target-unique-id")}; return {true, true};
            }
            result.request.row_source_type_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--bound-column-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--bound-column-target-object-name")}; return {true, true};
            }
            result.request.bound_column_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--bound-column-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--bound-column-target-unique-id")}; return {true, true};
            }
            result.request.bound_column_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--column-count-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--column-count-target-object-name")}; return {true, true};
            }
            result.request.column_count_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--column-count-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--column-count-target-unique-id")}; return {true, true};
            }
            result.request.column_count_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--style-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--style-target-object-name")}; return {true, true};
            }
            result.request.style_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--style-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--style-target-unique-id")}; return {true, true};
            }
            result.request.style_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--list-index-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--list-index-target-object-name")}; return {true, true};
            }
            result.request.list_index_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--list-index-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--list-index-target-unique-id")}; return {true, true};
            }
            result.request.list_index_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--left-column-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--left-column-target-object-name")}; return {true, true};
            }
            result.request.left_column_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--left-column-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--left-column-target-unique-id")}; return {true, true};
            }
            result.request.left_column_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }

if (argument == "--display-value-target-object-name") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--display-value-target-object-name")}; return {true, true};
            }
            result.request.display_value_objects.push_back({
                .record_index = 0U,
                .object_name = args[++index],
                .unique_id = {}
            });
            return {true, false};
        }

if (argument == "--display-value-target-unique-id") {
            if ((index + 1U) >= args.size()) {
                result = {.ok = false, .error = localized_missing_value_after_option(catalog, "--display-value-target-unique-id")}; return {true, true};
            }
            result.request.display_value_objects.push_back({
                .record_index = 0U,
                .object_name = {},
                .unique_id = args[++index]
            });
            return {true, false};
        }
    return {false, false};
}

}  // namespace copperfin::studio
