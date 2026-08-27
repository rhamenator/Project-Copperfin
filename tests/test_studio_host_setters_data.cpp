// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_studio_host_support.h"

namespace cf_test_studio_host {
#include "test_studio_host_setters_data_button_count.inl"

void test_parse_launch_arguments_for_data_session_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--data-session-object",
        "--data-session", "9",
        "--data-session-target-object-name", "cmdSave",
        "--data-session-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1119: launch contract should parse data-session-object requests");
    expect(result.request.data_session_object,
        "#1119: launch contract should detect --data-session-object");
    expect(result.request.data_session_available && result.request.data_session == 9,
        "#1119: data-session-object requests should carry data-session value");
    expect(result.request.data_session_objects.size() == 2U,
        "#1119: data-session-object requests should collect data-session target selectors");
    if (result.request.data_session_objects.size() == 2U) {
        expect(result.request.data_session_objects[0].object_name == "cmdSave" &&
                result.request.data_session_objects[0].unique_id.empty(),
            "#1119: data-session-object requests should parse target object-name selectors");
        expect(result.request.data_session_objects[1].object_name.empty() &&
                result.request.data_session_objects[1].unique_id == "two-guid",
            "#1119: data-session-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_data_session_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--data-session-object",
        "--data-session-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1119: launch contract should reject data-session-object requests without data-session value");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--data-session-object",
        "--data-session", "manual",
        "--data-session-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1119: launch contract should reject non-integer data-session values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--data-session-object",
        "--data-session", "-1",
        "--data-session-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1119: launch contract should reject negative data-session values before mutation");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--data-session-object",
        "--data-session", "2"
    });
    expect(!missing_targets_result.ok,
        "#1119: launch contract should reject data-session-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_data_session_object_ambiguity() {
    const auto data_session_locked_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--data-session-object",
        "--locked-object",
        "--data-session", "2",
        "--data-session-target-unique-id", "one-guid",
        "--locked", "true",
        "--locked-target-unique-id", "one-guid"
    });
    expect(!data_session_locked_result.ok,
        "#1119: launch contract should reject simultaneous data-session-object and locked-object requests");

    const auto data_session_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--data-session-object",
        "--clear-property",
        "--property-name", "DataSession",
        "--data-session", "2",
        "--data-session-target-unique-id", "one-guid"
    });
    expect(!data_session_property_result.ok,
        "#1119: launch contract should reject data-session-object combined with property commands");

    const auto stray_data_session_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--data-session", "2"
    });
    expect(!stray_data_session_result.ok,
        "#1119: launch contract should reject stray data-session arguments");
}

void test_parse_launch_arguments_for_lock_columns_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--lock-columns-object",
        "--lock-columns", "9",
        "--lock-columns-target-object-name", "cmdSave",
        "--lock-columns-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1123: launch contract should parse lock-columns-object requests");
    expect(result.request.lock_columns_object,
        "#1123: launch contract should detect --lock-columns-object");
    expect(result.request.lock_columns_available && result.request.lock_columns == 9,
        "#1123: lock-columns-object requests should carry lock-columns value");
    expect(result.request.lock_columns_objects.size() == 2U,
        "#1123: lock-columns-object requests should collect lock-columns target selectors");
    if (result.request.lock_columns_objects.size() == 2U) {
        expect(result.request.lock_columns_objects[0].object_name == "cmdSave" &&
                result.request.lock_columns_objects[0].unique_id.empty(),
            "#1123: lock-columns-object requests should parse target object-name selectors");
        expect(result.request.lock_columns_objects[1].object_name.empty() &&
                result.request.lock_columns_objects[1].unique_id == "two-guid",
            "#1123: lock-columns-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_lock_columns_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--lock-columns-object",
        "--lock-columns-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1123: launch contract should reject lock-columns-object requests without lock-columns value");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--lock-columns-object",
        "--lock-columns", "manual",
        "--lock-columns-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1123: launch contract should reject non-integer lock-columns values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--lock-columns-object",
        "--lock-columns", "-1",
        "--lock-columns-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1123: launch contract should reject negative lock-columns values before mutation");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--lock-columns-object",
        "--lock-columns", "2"
    });
    expect(!missing_targets_result.ok,
        "#1123: launch contract should reject lock-columns-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_lock_columns_object_ambiguity() {
    const auto lock_columns_locked_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--lock-columns-object",
        "--locked-object",
        "--lock-columns", "2",
        "--lock-columns-target-unique-id", "one-guid",
        "--locked", "true",
        "--locked-target-unique-id", "one-guid"
    });
    expect(!lock_columns_locked_result.ok,
        "#1123: launch contract should reject simultaneous lock-columns-object and locked-object requests");

    const auto lock_columns_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--lock-columns-object",
        "--clear-property",
        "--property-name", "LockColumns",
        "--lock-columns", "2",
        "--lock-columns-target-unique-id", "one-guid"
    });
    expect(!lock_columns_property_result.ok,
        "#1123: launch contract should reject lock-columns-object combined with property commands");

    const auto stray_lock_columns_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--lock-columns", "2"
    });
    expect(!stray_lock_columns_result.ok,
        "#1123: launch contract should reject stray lock-columns arguments");
}

void test_parse_launch_arguments_for_lock_columns_left_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--lock-columns-left-object",
        "--lock-columns-left", "9",
        "--lock-columns-left-target-object-name", "cmdSave",
        "--lock-columns-left-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1124: launch contract should parse lock-columns-left-object requests");
    expect(result.request.lock_columns_left_object,
        "#1124: launch contract should detect --lock-columns-left-object");
    expect(result.request.lock_columns_left_available && result.request.lock_columns_left == 9,
        "#1124: lock-columns-left-object requests should carry lock-columns-left value");
    expect(result.request.lock_columns_left_objects.size() == 2U,
        "#1124: lock-columns-left-object requests should collect lock-columns-left target selectors");
    if (result.request.lock_columns_left_objects.size() == 2U) {
        expect(result.request.lock_columns_left_objects[0].object_name == "cmdSave" &&
                result.request.lock_columns_left_objects[0].unique_id.empty(),
            "#1124: lock-columns-left-object requests should parse target object-name selectors");
        expect(result.request.lock_columns_left_objects[1].object_name.empty() &&
                result.request.lock_columns_left_objects[1].unique_id == "two-guid",
            "#1124: lock-columns-left-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_lock_columns_left_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--lock-columns-left-object",
        "--lock-columns-left-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1124: launch contract should reject lock-columns-left-object requests without lock-columns-left value");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--lock-columns-left-object",
        "--lock-columns-left", "manual",
        "--lock-columns-left-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1124: launch contract should reject non-integer lock-columns-left values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--lock-columns-left-object",
        "--lock-columns-left", "-1",
        "--lock-columns-left-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1124: launch contract should reject negative lock-columns-left values before mutation");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--lock-columns-left-object",
        "--lock-columns-left", "2"
    });
    expect(!missing_targets_result.ok,
        "#1124: launch contract should reject lock-columns-left-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_lock_columns_left_object_ambiguity() {
    const auto lock_columns_left_locked_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--lock-columns-left-object",
        "--locked-object",
        "--lock-columns-left", "2",
        "--lock-columns-left-target-unique-id", "one-guid",
        "--locked", "true",
        "--locked-target-unique-id", "one-guid"
    });
    expect(!lock_columns_left_locked_result.ok,
        "#1124: launch contract should reject simultaneous lock-columns-left-object and locked-object requests");

    const auto lock_columns_left_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--lock-columns-left-object",
        "--clear-property",
        "--property-name", "LockColumnsLeft",
        "--lock-columns-left", "2",
        "--lock-columns-left-target-unique-id", "one-guid"
    });
    expect(!lock_columns_left_property_result.ok,
        "#1124: launch contract should reject lock-columns-left-object combined with property commands");

    const auto stray_lock_columns_left_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--lock-columns-left", "2"
    });
    expect(!stray_lock_columns_left_result.ok,
        "#1124: launch contract should reject stray lock-columns-left arguments");
}

void test_parse_launch_arguments_for_partition_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--partition-object",
        "--partition", "9",
        "--partition-target-object-name", "cmdSave",
        "--partition-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1128: launch contract should parse partition-object requests");
    expect(result.request.partition_object,
        "#1128: launch contract should detect --partition-object");
    expect(result.request.partition_available && result.request.partition == 9,
        "#1128: partition-object requests should carry partition value");
    expect(result.request.partition_objects.size() == 2U,
        "#1128: partition-object requests should collect partition target selectors");
    if (result.request.partition_objects.size() == 2U) {
        expect(result.request.partition_objects[0].object_name == "cmdSave" &&
                result.request.partition_objects[0].unique_id.empty(),
            "#1128: partition-object requests should parse target object-name selectors");
        expect(result.request.partition_objects[1].object_name.empty() &&
                result.request.partition_objects[1].unique_id == "two-guid",
            "#1128: partition-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_partition_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--partition-object",
        "--partition-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1128: launch contract should reject partition-object requests without partition value");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--partition-object",
        "--partition", "manual",
        "--partition-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1128: launch contract should reject non-integer partition values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--partition-object",
        "--partition", "-1",
        "--partition-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1128: launch contract should reject negative partition values before mutation");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--partition-object",
        "--partition", "2"
    });
    expect(!missing_targets_result.ok,
        "#1128: launch contract should reject partition-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_partition_object_ambiguity() {
    const auto partition_locked_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--partition-object",
        "--locked-object",
        "--partition", "2",
        "--partition-target-unique-id", "one-guid",
        "--locked", "true",
        "--locked-target-unique-id", "one-guid"
    });
    expect(!partition_locked_result.ok,
        "#1128: launch contract should reject simultaneous partition-object and locked-object requests");

    const auto partition_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--partition-object",
        "--clear-property",
        "--property-name", "Partition",
        "--partition", "2",
        "--partition-target-unique-id", "one-guid"
    });
    expect(!partition_property_result.ok,
        "#1128: launch contract should reject partition-object combined with property commands");

    const auto stray_partition_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--partition", "2"
    });
    expect(!stray_partition_result.ok,
        "#1128: launch contract should reject stray partition arguments");
}

void test_parse_launch_arguments_for_record_source_type_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--record-source-type-object",
        "--record-source-type", "9",
        "--record-source-type-target-object-name", "cmdSave",
        "--record-source-type-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1129: launch contract should parse record-source-type-object requests");
    expect(result.request.record_source_type_object,
        "#1129: launch contract should detect --record-source-type-object");
    expect(result.request.record_source_type_available && result.request.record_source_type == 9,
        "#1129: record-source-type-object requests should carry record-source-type value");
    expect(result.request.record_source_type_objects.size() == 2U,
        "#1129: record-source-type-object requests should collect record_source_type target selectors");
    if (result.request.record_source_type_objects.size() == 2U) {
        expect(result.request.record_source_type_objects[0].object_name == "cmdSave" &&
                result.request.record_source_type_objects[0].unique_id.empty(),
            "#1129: record-source-type-object requests should parse target object-name selectors");
        expect(result.request.record_source_type_objects[1].object_name.empty() &&
                result.request.record_source_type_objects[1].unique_id == "two-guid",
            "#1129: record-source-type-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_record_source_type_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--record-source-type-object",
        "--record-source-type-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1129: launch contract should reject record-source-type-object requests without record-source-type value");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--record-source-type-object",
        "--record-source-type", "manual",
        "--record-source-type-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1129: launch contract should reject non-integer record-source-type values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--record-source-type-object",
        "--record-source-type", "-1",
        "--record-source-type-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1129: launch contract should reject negative record-source-type values before mutation");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--record-source-type-object",
        "--record-source-type", "2"
    });
    expect(!missing_targets_result.ok,
        "#1129: launch contract should reject record-source-type-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_record_source_type_object_ambiguity() {
    const auto record_source_type_locked_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--record-source-type-object",
        "--locked-object",
        "--record-source-type", "2",
        "--record-source-type-target-unique-id", "one-guid",
        "--locked", "true",
        "--locked-target-unique-id", "one-guid"
    });
    expect(!record_source_type_locked_result.ok,
        "#1129: launch contract should reject simultaneous record-source-type-object and locked-object requests");

    const auto record_source_type_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--record-source-type-object",
        "--clear-property",
        "--property-name", "RecordSourceType",
        "--record-source-type", "2",
        "--record-source-type-target-unique-id", "one-guid"
    });
    expect(!record_source_type_property_result.ok,
        "#1129: launch contract should reject record-source-type-object combined with property commands");

    const auto stray_record_source_type_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--record-source-type", "2"
    });
    expect(!stray_record_source_type_result.ok,
        "#1129: launch contract should reject stray record-source-type arguments");
}

void test_parse_launch_arguments_for_column_order_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--column-order-object",
        "--column-order", "9",
        "--column-order-target-object-name", "cmdSave",
        "--column-order-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1131: launch contract should parse column-order-object requests");
    expect(result.request.column_order_object,
        "#1131: launch contract should detect --column-order-object");
    expect(result.request.column_order_available && result.request.column_order == 9,
        "#1131: column-order-object requests should carry column-order value");
    expect(result.request.column_order_objects.size() == 2U,
        "#1131: column-order-object requests should collect column_order target selectors");
    if (result.request.column_order_objects.size() == 2U) {
        expect(result.request.column_order_objects[0].object_name == "cmdSave" &&
                result.request.column_order_objects[0].unique_id.empty(),
            "#1131: column-order-object requests should parse target object-name selectors");
        expect(result.request.column_order_objects[1].object_name.empty() &&
                result.request.column_order_objects[1].unique_id == "two-guid",
            "#1131: column-order-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_column_order_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--column-order-object",
        "--column-order-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1131: launch contract should reject column-order-object requests without column-order value");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--column-order-object",
        "--column-order", "manual",
        "--column-order-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1131: launch contract should reject non-integer column-order values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--column-order-object",
        "--column-order", "-1",
        "--column-order-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1131: launch contract should reject negative column-order values before mutation");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--column-order-object",
        "--column-order", "2"
    });
    expect(!missing_targets_result.ok,
        "#1131: launch contract should reject column-order-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_column_order_object_ambiguity() {
    const auto column_order_locked_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--column-order-object",
        "--locked-object",
        "--column-order", "2",
        "--column-order-target-unique-id", "one-guid",
        "--locked", "true",
        "--locked-target-unique-id", "one-guid"
    });
    expect(!column_order_locked_result.ok,
        "#1131: launch contract should reject simultaneous column-order-object and locked-object requests");

    const auto column_order_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--column-order-object",
        "--clear-property",
        "--property-name", "ColumnOrder",
        "--column-order", "2",
        "--column-order-target-unique-id", "one-guid"
    });
    expect(!column_order_property_result.ok,
        "#1131: launch contract should reject column-order-object combined with property commands");

    const auto stray_column_order_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--column-order", "2"
    });
    expect(!stray_column_order_result.ok,
        "#1131: launch contract should reject stray column-order arguments");
}

void test_parse_launch_arguments_for_child_order_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--child-order-object",
        "--child-order", "9",
        "--child-order-target-object-name", "cmdSave",
        "--child-order-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1133: launch contract should parse child-order-object requests");
    expect(result.request.child_order_object,
        "#1133: launch contract should detect --child-order-object");
    expect(result.request.child_order_available && result.request.child_order == 9,
        "#1133: child-order-object requests should carry child-order value");
    expect(result.request.child_order_objects.size() == 2U,
        "#1133: child-order-object requests should collect child_order target selectors");
    if (result.request.child_order_objects.size() == 2U) {
        expect(result.request.child_order_objects[0].object_name == "cmdSave" &&
                result.request.child_order_objects[0].unique_id.empty(),
            "#1133: child-order-object requests should parse target object-name selectors");
        expect(result.request.child_order_objects[1].object_name.empty() &&
                result.request.child_order_objects[1].unique_id == "two-guid",
            "#1133: child-order-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_child_order_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--child-order-object",
        "--child-order-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1133: launch contract should reject child-order-object requests without child-order value");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--child-order-object",
        "--child-order", "manual",
        "--child-order-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1133: launch contract should reject non-integer child-order values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--child-order-object",
        "--child-order", "-1",
        "--child-order-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1133: launch contract should reject negative child-order values before mutation");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--child-order-object",
        "--child-order", "2"
    });
    expect(!missing_targets_result.ok,
        "#1133: launch contract should reject child-order-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_child_order_object_ambiguity() {
    const auto child_order_locked_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--child-order-object",
        "--locked-object",
        "--child-order", "2",
        "--child-order-target-unique-id", "one-guid",
        "--locked", "true",
        "--locked-target-unique-id", "one-guid"
    });
    expect(!child_order_locked_result.ok,
        "#1133: launch contract should reject simultaneous child-order-object and locked-object requests");

    const auto child_order_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--child-order-object",
        "--clear-property",
        "--property-name", "ChildOrder",
        "--child-order", "2",
        "--child-order-target-unique-id", "one-guid"
    });
    expect(!child_order_property_result.ok,
        "#1133: launch contract should reject child-order-object combined with property commands");

    const auto stray_child_order_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--child-order", "2"
    });
    expect(!stray_child_order_result.ok,
        "#1133: launch contract should reject stray child-order arguments");
}

void test_parse_launch_arguments_for_record_source_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--record-source-object",
        "--record-source", "customers",
        "--record-source-target-object-name", "cmdSave",
        "--record-source-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1130: launch contract should parse record-source-object requests");
    expect(result.request.record_source_object, "#1130: launch contract should detect --record-source-object");
    expect(result.request.record_source_available && result.request.record_source == "customers",
        "#1130: record-source-object requests should carry record source");
    expect(result.request.record_source_objects.size() == 2U,
        "#1130: record-source-object requests should collect record source target selectors");
    if (result.request.record_source_objects.size() == 2U) {
        expect(result.request.record_source_objects[0].object_name == "cmdSave" &&
                result.request.record_source_objects[0].unique_id.empty(),
            "#1130: record-source-object requests should parse target object-name selectors");
        expect(result.request.record_source_objects[1].object_name.empty() &&
                result.request.record_source_objects[1].unique_id == "two-guid",
            "#1130: record-source-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_record_source_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--record-source-object",
        "--record-source-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1130: launch contract should reject record-source-object requests without record source");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--record-source-object",
        "--record-source", "Save"
    });
    expect(!missing_targets_result.ok,
        "#1130: launch contract should reject record-source-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_record_source_object_ambiguity() {
    const auto record_source_caption_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--record-source-object",
        "--caption-object",
        "--record-source", "Save",
        "--record-source-target-unique-id", "one-guid",
        "--caption", "Save",
        "--caption-target-unique-id", "one-guid"
    });
    expect(!record_source_caption_result.ok,
        "#1130: launch contract should reject simultaneous record-source-object and caption-object requests");

    const auto record_source_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--record-source-object",
        "--clear-property",
        "--property-name", "ToolTipText",
        "--record-source", "Save",
        "--record-source-target-unique-id", "one-guid"
    });
    expect(!record_source_property_result.ok,
        "#1130: launch contract should reject record-source-object combined with property commands");

    const auto stray_record_source_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--record-source", "Save"
    });
    expect(!stray_record_source_result.ok,
        "#1130: launch contract should reject stray record source arguments");
}

void test_parse_launch_arguments_for_form_set_class_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--form-set-class-object",
        "--form-set-class", "BaseFormSet",
        "--form-set-class-target-object-name", "cmdSaveFormSet",
        "--form-set-class-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1136: launch contract should parse form-set-class-object requests");
    expect(result.request.form_set_class_object, "#1136: launch contract should detect --form-set-class-object");
    expect(result.request.form_set_class_available && result.request.form_set_class == "BaseFormSet",
        "#1136: form-set-class-object requests should carry form set class");
    expect(result.request.form_set_class_objects.size() == 2U,
        "#1136: form-set-class-object requests should collect form set class target selectors");
    if (result.request.form_set_class_objects.size() == 2U) {
        expect(result.request.form_set_class_objects[0].object_name == "cmdSaveFormSet" &&
                result.request.form_set_class_objects[0].unique_id.empty(),
            "#1136: form-set-class-object requests should parse target object-name selectors");
        expect(result.request.form_set_class_objects[1].object_name.empty() &&
                result.request.form_set_class_objects[1].unique_id == "two-guid",
            "#1136: form-set-class-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_form_set_class_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--form-set-class-object",
        "--form-set-class-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1136: launch contract should reject form-set-class-object requests without form set class");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--form-set-class-object",
        "--form-set-class", "SaveFormSet"
    });
    expect(!missing_targets_result.ok,
        "#1136: launch contract should reject form-set-class-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_form_set_class_object_ambiguity() {
    const auto form_set_class_caption_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--form-set-class-object",
        "--caption-object",
        "--form-set-class", "SaveFormSet",
        "--form-set-class-target-unique-id", "one-guid",
        "--caption", "SaveFormSet",
        "--caption-target-unique-id", "one-guid"
    });
    expect(!form_set_class_caption_result.ok,
        "#1136: launch contract should reject simultaneous form-set-class-object and caption-object requests");

    const auto form_set_class_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--form-set-class-object",
        "--clear-property",
        "--property-name", "ToolTipText",
        "--form-set-class", "SaveFormSet",
        "--form-set-class-target-unique-id", "one-guid"
    });
    expect(!form_set_class_property_result.ok,
        "#1136: launch contract should reject form-set-class-object combined with property commands");

    const auto stray_form_set_class_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--form-set-class", "SaveFormSet"
    });
    expect(!stray_form_set_class_result.ok,
        "#1136: launch contract should reject stray form set class arguments");
}

void test_parse_launch_arguments_for_default_file_path_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--default-file-path-object",
        "--default-file-path", "Data\\Customers",
        "--default-file-path-target-object-name", "cmdDefaultPath",
        "--default-file-path-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1137: launch contract should parse default-file-path-object requests");
    expect(result.request.default_file_path_object, "#1137: launch contract should detect --default-file-path-object");
    expect(result.request.default_file_path_available && result.request.default_file_path == "Data\\Customers",
        "#1137: default-file-path-object requests should carry default file path");
    expect(result.request.default_file_path_objects.size() == 2U,
        "#1137: default-file-path-object requests should collect default file path target selectors");
    if (result.request.default_file_path_objects.size() == 2U) {
        expect(result.request.default_file_path_objects[0].object_name == "cmdDefaultPath" &&
                result.request.default_file_path_objects[0].unique_id.empty(),
            "#1137: default-file-path-object requests should parse target object-name selectors");
        expect(result.request.default_file_path_objects[1].object_name.empty() &&
                result.request.default_file_path_objects[1].unique_id == "two-guid",
            "#1137: default-file-path-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_default_file_path_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--default-file-path-object",
        "--default-file-path-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1137: launch contract should reject default-file-path-object requests without default file path");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--default-file-path-object",
        "--default-file-path", "Data\\Customers"
    });
    expect(!missing_targets_result.ok,
        "#1137: launch contract should reject default-file-path-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_default_file_path_object_ambiguity() {
    const auto default_file_path_caption_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--default-file-path-object",
        "--caption-object",
        "--default-file-path", "Data\\Customers",
        "--default-file-path-target-unique-id", "one-guid",
        "--caption", "Default path",
        "--caption-target-unique-id", "one-guid"
    });
    expect(!default_file_path_caption_result.ok,
        "#1137: launch contract should reject simultaneous default-file-path-object and caption-object requests");

    const auto default_file_path_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--default-file-path-object",
        "--clear-property",
        "--property-name", "ToolTipText",
        "--default-file-path", "Data\\Customers",
        "--default-file-path-target-unique-id", "one-guid"
    });
    expect(!default_file_path_property_result.ok,
        "#1137: launch contract should reject default-file-path-object combined with property commands");

    const auto stray_default_file_path_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--default-file-path", "Data\\Customers"
    });
    expect(!stray_default_file_path_result.ok,
        "#1137: launch contract should reject stray default file path arguments");
}

void test_parse_launch_arguments_for_initial_selected_alias_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--initial-selected-alias-object",
        "--initial-selected-alias", "customers",
        "--initial-selected-alias-target-object-name", "cmdInitialAlias",
        "--initial-selected-alias-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1138: launch contract should parse initial-selected-alias-object requests");
    expect(result.request.initial_selected_alias_object,
        "#1138: launch contract should detect --initial-selected-alias-object");
    expect(result.request.initial_selected_alias_available && result.request.initial_selected_alias == "customers",
        "#1138: initial-selected-alias-object requests should carry initial selected alias");
    expect(result.request.initial_selected_alias_objects.size() == 2U,
        "#1138: initial-selected-alias-object requests should collect initial selected alias target selectors");
    if (result.request.initial_selected_alias_objects.size() == 2U) {
        expect(result.request.initial_selected_alias_objects[0].object_name == "cmdInitialAlias" &&
                result.request.initial_selected_alias_objects[0].unique_id.empty(),
            "#1138: initial-selected-alias-object requests should parse target object-name selectors");
        expect(result.request.initial_selected_alias_objects[1].object_name.empty() &&
                result.request.initial_selected_alias_objects[1].unique_id == "two-guid",
            "#1138: initial-selected-alias-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_initial_selected_alias_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--initial-selected-alias-object",
        "--initial-selected-alias-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1138: launch contract should reject initial-selected-alias-object requests without initial selected alias");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--initial-selected-alias-object",
        "--initial-selected-alias", "customers"
    });
    expect(!missing_targets_result.ok,
        "#1138: launch contract should reject initial-selected-alias-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_initial_selected_alias_object_ambiguity() {
    const auto initial_selected_alias_caption_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--initial-selected-alias-object",
        "--caption-object",
        "--initial-selected-alias", "customers",
        "--initial-selected-alias-target-unique-id", "one-guid",
        "--caption", "Initial alias",
        "--caption-target-unique-id", "one-guid"
    });
    expect(!initial_selected_alias_caption_result.ok,
        "#1138: launch contract should reject simultaneous initial-selected-alias-object and caption-object requests");

    const auto initial_selected_alias_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--initial-selected-alias-object",
        "--clear-property",
        "--property-name", "ToolTipText",
        "--initial-selected-alias", "customers",
        "--initial-selected-alias-target-unique-id", "one-guid"
    });
    expect(!initial_selected_alias_property_result.ok,
        "#1138: launch contract should reject initial-selected-alias-object combined with property commands");

    const auto stray_initial_selected_alias_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--initial-selected-alias", "customers"
    });
    expect(!stray_initial_selected_alias_result.ok,
        "#1138: launch contract should reject stray initial selected alias arguments");
}

void test_parse_launch_arguments_for_link_master_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--link-master-object",
        "--link-master", "customer_id",
        "--link-master-target-object-name", "cmdSave",
        "--link-master-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1165: launch contract should parse link-master-object requests");
    expect(result.request.link_master_object, "#1165: launch contract should detect --link-master-object");
    expect(result.request.link_master_available && result.request.link_master == "customer_id",
        "#1165: link-master-object requests should carry link-master text");
    expect(result.request.link_master_objects.size() == 2U,
        "#1165: link-master-object requests should collect link-master target selectors");
    if (result.request.link_master_objects.size() == 2U) {
        expect(result.request.link_master_objects[0].object_name == "cmdSave" &&
                result.request.link_master_objects[0].unique_id.empty(),
            "#1165: link-master-object requests should parse target object-name selectors");
        expect(result.request.link_master_objects[1].object_name.empty() &&
                result.request.link_master_objects[1].unique_id == "two-guid",
            "#1165: link-master-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_link_master_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--link-master-object",
        "--link-master-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1165: launch contract should reject link-master-object requests without link-master text");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--link-master-object",
        "--link-master", "customer_id"
    });
    expect(!missing_targets_result.ok,
        "#1165: launch contract should reject link-master-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_link_master_object_ambiguity() {
    const auto link_master_status_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--link-master-object",
        "--status-bar-text-object",
        "--link-master", "customer_id",
        "--link-master-target-unique-id", "one-guid",
        "--status-bar-text", "Ready",
        "--status-bar-text-target-unique-id", "one-guid"
    });
    expect(!link_master_status_result.ok,
        "#1165: launch contract should reject simultaneous link-master-object and status-bar-text-object requests");

    const auto link_master_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--link-master-object",
        "--clear-property",
        "--property-name", "LinkMaster",
        "--link-master", "customer_id",
        "--link-master-target-unique-id", "one-guid"
    });
    expect(!link_master_property_result.ok,
        "#1165: launch contract should reject link-master-object combined with property commands");

    const auto stray_link_master_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--link-master", "customer_id"
    });
    expect(!stray_link_master_result.ok,
        "#1165: launch contract should reject stray link-master arguments");
}

void test_parse_launch_arguments_for_row_source_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--row-source-object",
        "--row-source", "customers.name,customer_id",
        "--row-source-target-object-name", "cboCustomer",
        "--row-source-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1048: launch contract should parse row-source-object requests");
    expect(result.request.row_source_object, "#1048: launch contract should detect --row-source-object");
    expect(result.request.row_source_available && result.request.row_source == "customers.name,customer_id",
        "#1048: row-source-object requests should carry row source text");
    expect(result.request.row_source_objects.size() == 2U,
        "#1048: row-source-object requests should collect row source target selectors");
    if (result.request.row_source_objects.size() == 2U) {
        expect(result.request.row_source_objects[0].object_name == "cboCustomer" &&
                result.request.row_source_objects[0].unique_id.empty(),
            "#1048: row-source-object requests should parse target object-name selectors");
        expect(result.request.row_source_objects[1].object_name.empty() &&
                result.request.row_source_objects[1].unique_id == "two-guid",
            "#1048: row-source-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_row_source_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--row-source-object",
        "--row-source-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1048: launch contract should reject row-source-object requests without row source text");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--row-source-object",
        "--row-source", "customers.name"
    });
    expect(!missing_targets_result.ok,
        "#1048: launch contract should reject row-source-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_row_source_object_ambiguity() {
    const auto row_format_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--row-source-object",
        "--format-object",
        "--row-source", "customers.name",
        "--row-source-target-unique-id", "one-guid",
        "--format", "!",
        "--format-target-unique-id", "one-guid"
    });
    expect(!row_format_result.ok,
        "#1048: launch contract should reject simultaneous row-source-object and format-object requests");

    const auto row_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--row-source-object",
        "--clear-property",
        "--property-name", "RowSource",
        "--row-source", "customers.name",
        "--row-source-target-unique-id", "one-guid"
    });
    expect(!row_property_result.ok,
        "#1048: launch contract should reject row-source-object combined with property commands");

    const auto stray_row_source_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--row-source", "customers.name"
    });
    expect(!stray_row_source_result.ok,
        "#1048: launch contract should reject stray row-source arguments");
}

void test_parse_launch_arguments_for_column_widths_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--column-widths-object",
        "--column-widths", "40,90,120",
        "--column-widths-target-object-name", "cboCustomer",
        "--column-widths-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1196: launch contract should parse column-widths-object requests");
    expect(result.request.column_widths_object, "#1196: launch contract should detect --column-widths-object");
    expect(result.request.column_widths_available && result.request.column_widths == "40,90,120",
        "#1196: column-widths-object requests should carry column widths text");
    expect(result.request.column_widths_objects.size() == 2U,
        "#1196: column-widths-object requests should collect column-widths target selectors");
    if (result.request.column_widths_objects.size() == 2U) {
        expect(result.request.column_widths_objects[0].object_name == "cboCustomer" &&
                result.request.column_widths_objects[0].unique_id.empty(),
            "#1196: column-widths-object requests should parse target object-name selectors");
        expect(result.request.column_widths_objects[1].object_name.empty() &&
                result.request.column_widths_objects[1].unique_id == "two-guid",
            "#1196: column-widths-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_column_widths_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--column-widths-object",
        "--column-widths-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1196: launch contract should reject column-widths-object requests without column widths text");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--column-widths-object",
        "--column-widths", "40,90,120"
    });
    expect(!missing_targets_result.ok,
        "#1196: launch contract should reject column-widths-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_column_widths_object_ambiguity() {
    const auto column_widths_row_source_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--column-widths-object",
        "--row-source-object",
        "--column-widths", "40,90,120",
        "--column-widths-target-unique-id", "one-guid",
        "--row-source", "customers.name",
        "--row-source-target-unique-id", "one-guid"
    });
    expect(!column_widths_row_source_result.ok,
        "#1196: launch contract should reject simultaneous column-widths-object and row-source-object requests");

    const auto column_widths_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--column-widths-object",
        "--clear-property",
        "--property-name", "ColumnWidths",
        "--column-widths", "40,90,120",
        "--column-widths-target-unique-id", "one-guid"
    });
    expect(!column_widths_property_result.ok,
        "#1196: launch contract should reject column-widths-object combined with property commands");

    const auto stray_column_widths_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--column-widths", "40,90,120"
    });
    expect(!stray_column_widths_result.ok,
        "#1196: launch contract should reject stray column-widths arguments");
}

void test_parse_launch_arguments_for_column_lines_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--column-lines-object",
        "--column-lines", "true",
        "--column-lines-target-object-name", "cboCustomer",
        "--column-lines-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1197: launch contract should parse column-lines-object requests");
    expect(result.request.column_lines_object, "#1197: launch contract should detect --column-lines-object");
    expect(result.request.column_lines_available && result.request.column_lines,
        "#1197: column-lines-object requests should carry column lines state");
    expect(result.request.column_lines_objects.size() == 2U,
        "#1197: column-lines-object requests should collect column-lines target selectors");
    if (result.request.column_lines_objects.size() == 2U) {
        expect(result.request.column_lines_objects[0].object_name == "cboCustomer" &&
                result.request.column_lines_objects[0].unique_id.empty(),
            "#1197: column-lines-object requests should parse target object-name selectors");
        expect(result.request.column_lines_objects[1].object_name.empty() &&
                result.request.column_lines_objects[1].unique_id == "two-guid",
            "#1197: column-lines-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_column_lines_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--column-lines-object",
        "--column-lines-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1197: launch contract should reject column-lines-object requests without column lines state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--column-lines-object",
        "--column-lines", "true"
    });
    expect(!missing_targets_result.ok,
        "#1197: launch contract should reject column-lines-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--column-lines-object",
        "--column-lines", "sometimes",
        "--column-lines-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1197: launch contract should reject invalid column-lines logical values");
}

void test_parse_launch_arguments_rejects_column_lines_object_ambiguity() {
    const auto column_lines_row_source_type_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--column-lines-object",
        "--row-source-type-object",
        "--column-lines", "true",
        "--column-lines-target-unique-id", "one-guid",
        "--row-source-type", "2",
        "--row-source-type-target-unique-id", "one-guid"
    });
    expect(!column_lines_row_source_type_result.ok,
        "#1197: launch contract should reject simultaneous column-lines-object and row-source-type-object requests");

    const auto column_lines_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--column-lines-object",
        "--clear-property",
        "--property-name", "ColumnLines",
        "--column-lines", "true",
        "--column-lines-target-unique-id", "one-guid"
    });
    expect(!column_lines_property_result.ok,
        "#1197: launch contract should reject column-lines-object combined with property commands");

    const auto stray_column_lines_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--column-lines", "true"
    });
    expect(!stray_column_lines_result.ok,
        "#1197: launch contract should reject stray column-lines arguments");
}

void test_parse_launch_arguments_for_integral_height_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--integral-height-object",
        "--integral-height", "true",
        "--integral-height-target-object-name", "cboCustomer",
        "--integral-height-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1198: launch contract should parse integral-height-object requests");
    expect(result.request.integral_height_object, "#1198: launch contract should detect --integral-height-object");
    expect(result.request.integral_height_available && result.request.integral_height,
        "#1198: integral-height-object requests should carry integral height state");
    expect(result.request.integral_height_objects.size() == 2U,
        "#1198: integral-height-object requests should collect integral-height target selectors");
    if (result.request.integral_height_objects.size() == 2U) {
        expect(result.request.integral_height_objects[0].object_name == "cboCustomer" &&
                result.request.integral_height_objects[0].unique_id.empty(),
            "#1198: integral-height-object requests should parse target object-name selectors");
        expect(result.request.integral_height_objects[1].object_name.empty() &&
                result.request.integral_height_objects[1].unique_id == "two-guid",
            "#1198: integral-height-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_integral_height_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--integral-height-object",
        "--integral-height-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1198: launch contract should reject integral-height-object requests without integral height state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--integral-height-object",
        "--integral-height", "true"
    });
    expect(!missing_targets_result.ok,
        "#1198: launch contract should reject integral-height-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--integral-height-object",
        "--integral-height", "sometimes",
        "--integral-height-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1198: launch contract should reject invalid integral-height logical values");
}

void test_parse_launch_arguments_rejects_integral_height_object_ambiguity() {
    const auto integral_height_row_source_type_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--integral-height-object",
        "--row-source-type-object",
        "--integral-height", "true",
        "--integral-height-target-unique-id", "one-guid",
        "--row-source-type", "2",
        "--row-source-type-target-unique-id", "one-guid"
    });
    expect(!integral_height_row_source_type_result.ok,
        "#1198: launch contract should reject simultaneous integral-height-object and row-source-type-object requests");

    const auto integral_height_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--integral-height-object",
        "--clear-property",
        "--property-name", "IntegralHeight",
        "--integral-height", "true",
        "--integral-height-target-unique-id", "one-guid"
    });
    expect(!integral_height_property_result.ok,
        "#1198: launch contract should reject integral-height-object combined with property commands");

    const auto stray_integral_height_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--integral-height", "true"
    });
    expect(!stray_integral_height_result.ok,
        "#1198: launch contract should reject stray integral-height arguments");
}

void test_parse_launch_arguments_for_incremental_search_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--incremental-search-object",
        "--incremental-search", "true",
        "--incremental-search-target-object-name", "cboCustomer",
        "--incremental-search-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1199: launch contract should parse incremental-search-object requests");
    expect(result.request.incremental_search_object, "#1199: launch contract should detect --incremental-search-object");
    expect(result.request.incremental_search_available && result.request.incremental_search,
        "#1199: incremental-search-object requests should carry incremental search state");
    expect(result.request.incremental_search_objects.size() == 2U,
        "#1199: incremental-search-object requests should collect incremental-search target selectors");
    if (result.request.incremental_search_objects.size() == 2U) {
        expect(result.request.incremental_search_objects[0].object_name == "cboCustomer" &&
                result.request.incremental_search_objects[0].unique_id.empty(),
            "#1199: incremental-search-object requests should parse target object-name selectors");
        expect(result.request.incremental_search_objects[1].object_name.empty() &&
                result.request.incremental_search_objects[1].unique_id == "two-guid",
            "#1199: incremental-search-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_incremental_search_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--incremental-search-object",
        "--incremental-search-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1199: launch contract should reject incremental-search-object requests without incremental search state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--incremental-search-object",
        "--incremental-search", "true"
    });
    expect(!missing_targets_result.ok,
        "#1199: launch contract should reject incremental-search-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--incremental-search-object",
        "--incremental-search", "sometimes",
        "--incremental-search-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1199: launch contract should reject invalid incremental-search logical values");
}

void test_parse_launch_arguments_rejects_incremental_search_object_ambiguity() {
    const auto incremental_search_row_source_type_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--incremental-search-object",
        "--row-source-type-object",
        "--incremental-search", "true",
        "--incremental-search-target-unique-id", "one-guid",
        "--row-source-type", "2",
        "--row-source-type-target-unique-id", "one-guid"
    });
    expect(!incremental_search_row_source_type_result.ok,
        "#1199: launch contract should reject simultaneous incremental-search-object and row-source-type-object requests");

    const auto incremental_search_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--incremental-search-object",
        "--clear-property",
        "--property-name", "IncrementalSearch",
        "--incremental-search", "true",
        "--incremental-search-target-unique-id", "one-guid"
    });
    expect(!incremental_search_property_result.ok,
        "#1199: launch contract should reject incremental-search-object combined with property commands");

    const auto stray_incremental_search_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--incremental-search", "true"
    });
    expect(!stray_incremental_search_result.ok,
        "#1199: launch contract should reject stray incremental-search arguments");
}

void test_parse_launch_arguments_for_multi_select_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--multi-select-object",
        "--multi-select", "true",
        "--multi-select-target-object-name", "cboCustomer",
        "--multi-select-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1200: launch contract should parse multi-select-object requests");
    expect(result.request.multi_select_object, "#1200: launch contract should detect --multi-select-object");
    expect(result.request.multi_select_available && result.request.multi_select,
        "#1200: multi-select-object requests should carry multi select state");
    expect(result.request.multi_select_objects.size() == 2U,
        "#1200: multi-select-object requests should collect multi-select target selectors");
    if (result.request.multi_select_objects.size() == 2U) {
        expect(result.request.multi_select_objects[0].object_name == "cboCustomer" &&
                result.request.multi_select_objects[0].unique_id.empty(),
            "#1200: multi-select-object requests should parse target object-name selectors");
        expect(result.request.multi_select_objects[1].object_name.empty() &&
                result.request.multi_select_objects[1].unique_id == "two-guid",
            "#1200: multi-select-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_multi_select_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--multi-select-object",
        "--multi-select-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1200: launch contract should reject multi-select-object requests without multi select state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--multi-select-object",
        "--multi-select", "true"
    });
    expect(!missing_targets_result.ok,
        "#1200: launch contract should reject multi-select-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--multi-select-object",
        "--multi-select", "sometimes",
        "--multi-select-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1200: launch contract should reject invalid multi-select logical values");
}

void test_parse_launch_arguments_rejects_multi_select_object_ambiguity() {
    const auto multi_select_row_source_type_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--multi-select-object",
        "--row-source-type-object",
        "--multi-select", "true",
        "--multi-select-target-unique-id", "one-guid",
        "--row-source-type", "2",
        "--row-source-type-target-unique-id", "one-guid"
    });
    expect(!multi_select_row_source_type_result.ok,
        "#1200: launch contract should reject simultaneous multi-select-object and row-source-type-object requests");

    const auto multi_select_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--multi-select-object",
        "--clear-property",
        "--property-name", "MultiSelect",
        "--multi-select", "true",
        "--multi-select-target-unique-id", "one-guid"
    });
    expect(!multi_select_property_result.ok,
        "#1200: launch contract should reject multi-select-object combined with property commands");

    const auto stray_multi_select_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--multi-select", "true"
    });
    expect(!stray_multi_select_result.ok,
        "#1200: launch contract should reject stray multi-select arguments");
}

void test_parse_launch_arguments_for_row_source_type_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--row-source-type-object",
        "--row-source-type", "6",
        "--row-source-type-target-object-name", "cboCustomer",
        "--row-source-type-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1049: launch contract should parse row-source-type-object requests");
    expect(result.request.row_source_type_object, "#1049: launch contract should detect --row-source-type-object");
    expect(result.request.row_source_type_available && result.request.row_source_type == 6,
        "#1049: row-source-type-object requests should carry row source type values");
    expect(result.request.row_source_type_objects.size() == 2U,
        "#1049: row-source-type-object requests should collect row source type target selectors");
    if (result.request.row_source_type_objects.size() == 2U) {
        expect(result.request.row_source_type_objects[0].object_name == "cboCustomer" &&
                result.request.row_source_type_objects[0].unique_id.empty(),
            "#1049: row-source-type-object requests should parse target object-name selectors");
        expect(result.request.row_source_type_objects[1].object_name.empty() &&
                result.request.row_source_type_objects[1].unique_id == "two-guid",
            "#1049: row-source-type-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_row_source_type_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--row-source-type-object",
        "--row-source-type-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1049: launch contract should reject row-source-type-object requests without row source type");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--row-source-type-object",
        "--row-source-type", "6"
    });
    expect(!missing_targets_result.ok,
        "#1049: launch contract should reject row-source-type-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--row-source-type-object",
        "--row-source-type", "fields",
        "--row-source-type-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1049: launch contract should reject non-integer row-source-type values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--row-source-type-object",
        "--row-source-type", "-1",
        "--row-source-type-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1049: launch contract should reject negative row-source-type values before mutation");
}

void test_parse_launch_arguments_rejects_row_source_type_object_ambiguity() {
    const auto type_row_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--row-source-type-object",
        "--row-source-object",
        "--row-source-type", "6",
        "--row-source-type-target-unique-id", "one-guid",
        "--row-source", "customers.name",
        "--row-source-target-unique-id", "one-guid"
    });
    expect(!type_row_result.ok,
        "#1049: launch contract should reject simultaneous row-source-type-object and row-source-object requests");

    const auto type_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--row-source-type-object",
        "--clear-property",
        "--property-name", "RowSourceType",
        "--row-source-type", "6",
        "--row-source-type-target-unique-id", "one-guid"
    });
    expect(!type_property_result.ok,
        "#1049: launch contract should reject row-source-type-object combined with property commands");

    const auto stray_row_source_type_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--row-source-type", "6"
    });
    expect(!stray_row_source_type_result.ok,
        "#1049: launch contract should reject stray row-source-type arguments");
}

void test_parse_launch_arguments_for_bound_column_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--bound-column-object",
        "--bound-column", "4",
        "--bound-column-target-object-name", "cboCustomer",
        "--bound-column-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1050: launch contract should parse bound-column-object requests");
    expect(result.request.bound_column_object, "#1050: launch contract should detect --bound-column-object");
    expect(result.request.bound_column_available && result.request.bound_column == 4,
        "#1050: bound-column-object requests should carry bound column values");
    expect(result.request.bound_column_objects.size() == 2U,
        "#1050: bound-column-object requests should collect bound column target selectors");
    if (result.request.bound_column_objects.size() == 2U) {
        expect(result.request.bound_column_objects[0].object_name == "cboCustomer" &&
                result.request.bound_column_objects[0].unique_id.empty(),
            "#1050: bound-column-object requests should parse target object-name selectors");
        expect(result.request.bound_column_objects[1].object_name.empty() &&
                result.request.bound_column_objects[1].unique_id == "two-guid",
            "#1050: bound-column-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_bound_column_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--bound-column-object",
        "--bound-column-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1050: launch contract should reject bound-column-object requests without bound column");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--bound-column-object",
        "--bound-column", "4"
    });
    expect(!missing_targets_result.ok,
        "#1050: launch contract should reject bound-column-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--bound-column-object",
        "--bound-column", "first",
        "--bound-column-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1050: launch contract should reject non-integer bound-column values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--bound-column-object",
        "--bound-column", "-1",
        "--bound-column-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1050: launch contract should reject negative bound-column values before mutation");
}

void test_parse_launch_arguments_rejects_bound_column_object_ambiguity() {
    const auto column_type_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--bound-column-object",
        "--row-source-type-object",
        "--bound-column", "4",
        "--bound-column-target-unique-id", "one-guid",
        "--row-source-type", "6",
        "--row-source-type-target-unique-id", "one-guid"
    });
    expect(!column_type_result.ok,
        "#1050: launch contract should reject simultaneous bound-column-object and row-source-type-object requests");

    const auto column_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--bound-column-object",
        "--clear-property",
        "--property-name", "BoundColumn",
        "--bound-column", "4",
        "--bound-column-target-unique-id", "one-guid"
    });
    expect(!column_property_result.ok,
        "#1050: launch contract should reject bound-column-object combined with property commands");

    const auto stray_bound_column_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--bound-column", "4"
    });
    expect(!stray_bound_column_result.ok,
        "#1050: launch contract should reject stray bound-column arguments");
}

void test_parse_launch_arguments_for_column_count_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--column-count-object",
        "--column-count", "5",
        "--column-count-target-object-name", "cboCustomer",
        "--column-count-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1051: launch contract should parse column-count-object requests");
    expect(result.request.column_count_object, "#1051: launch contract should detect --column-count-object");
    expect(result.request.column_count_available && result.request.column_count == 5,
        "#1051: column-count-object requests should carry column count values");
    expect(result.request.column_count_objects.size() == 2U,
        "#1051: column-count-object requests should collect column count target selectors");
    if (result.request.column_count_objects.size() == 2U) {
        expect(result.request.column_count_objects[0].object_name == "cboCustomer" &&
                result.request.column_count_objects[0].unique_id.empty(),
            "#1051: column-count-object requests should parse target object-name selectors");
        expect(result.request.column_count_objects[1].object_name.empty() &&
                result.request.column_count_objects[1].unique_id == "two-guid",
            "#1051: column-count-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_column_count_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--column-count-object",
        "--column-count-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1051: launch contract should reject column-count-object requests without column count");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--column-count-object",
        "--column-count", "5"
    });
    expect(!missing_targets_result.ok,
        "#1051: launch contract should reject column-count-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--column-count-object",
        "--column-count", "many",
        "--column-count-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1051: launch contract should reject non-integer column-count values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--column-count-object",
        "--column-count", "-1",
        "--column-count-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1051: launch contract should reject negative column-count values before mutation");
}

void test_parse_launch_arguments_rejects_column_count_object_ambiguity() {
    const auto count_column_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--column-count-object",
        "--bound-column-object",
        "--column-count", "5",
        "--column-count-target-unique-id", "one-guid",
        "--bound-column", "4",
        "--bound-column-target-unique-id", "one-guid"
    });
    expect(!count_column_result.ok,
        "#1051: launch contract should reject simultaneous column-count-object and bound-column-object requests");

    const auto count_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--column-count-object",
        "--clear-property",
        "--property-name", "ColumnCount",
        "--column-count", "5",
        "--column-count-target-unique-id", "one-guid"
    });
    expect(!count_property_result.ok,
        "#1051: launch contract should reject column-count-object combined with property commands");

    const auto stray_column_count_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--column-count", "5"
    });
    expect(!stray_column_count_result.ok,
        "#1051: launch contract should reject stray column-count arguments");
}

void test_parse_launch_arguments_for_style_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--style-object",
        "--style", "2",
        "--style-target-object-name", "cboCustomer",
        "--style-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1052: launch contract should parse style-object requests");
    expect(result.request.style_object, "#1052: launch contract should detect --style-object");
    expect(result.request.style_available && result.request.style == 2,
        "#1052: style-object requests should carry style values");
    expect(result.request.style_objects.size() == 2U,
        "#1052: style-object requests should collect style target selectors");
    if (result.request.style_objects.size() == 2U) {
        expect(result.request.style_objects[0].object_name == "cboCustomer" &&
                result.request.style_objects[0].unique_id.empty(),
            "#1052: style-object requests should parse target object-name selectors");
        expect(result.request.style_objects[1].object_name.empty() &&
                result.request.style_objects[1].unique_id == "two-guid",
            "#1052: style-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_style_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--style-object",
        "--style-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1052: launch contract should reject style-object requests without style");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--style-object",
        "--style", "2"
    });
    expect(!missing_targets_result.ok,
        "#1052: launch contract should reject style-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--style-object",
        "--style", "combo",
        "--style-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1052: launch contract should reject non-integer style values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--style-object",
        "--style", "-1",
        "--style-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1052: launch contract should reject negative style values before mutation");
}

void test_parse_launch_arguments_rejects_style_object_ambiguity() {
    const auto style_count_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--style-object",
        "--column-count-object",
        "--style", "2",
        "--style-target-unique-id", "one-guid",
        "--column-count", "5",
        "--column-count-target-unique-id", "one-guid"
    });
    expect(!style_count_result.ok,
        "#1052: launch contract should reject simultaneous style-object and column-count-object requests");

    const auto style_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--style-object",
        "--clear-property",
        "--property-name", "Style",
        "--style", "2",
        "--style-target-unique-id", "one-guid"
    });
    expect(!style_property_result.ok,
        "#1052: launch contract should reject style-object combined with property commands");

    const auto stray_style_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--style", "2"
    });
    expect(!stray_style_result.ok,
        "#1052: launch contract should reject stray style arguments");
}

void test_parse_launch_arguments_for_list_index_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--list-index-object",
        "--list-index", "3",
        "--list-index-target-object-name", "cboCustomer",
        "--list-index-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1053: launch contract should parse list-index-object requests");
    expect(result.request.list_index_object, "#1053: launch contract should detect --list-index-object");
    expect(result.request.list_index_available && result.request.list_index == 3,
        "#1053: list-index-object requests should carry list index values");
    expect(result.request.list_index_objects.size() == 2U,
        "#1053: list-index-object requests should collect list index target selectors");
    if (result.request.list_index_objects.size() == 2U) {
        expect(result.request.list_index_objects[0].object_name == "cboCustomer" &&
                result.request.list_index_objects[0].unique_id.empty(),
            "#1053: list-index-object requests should parse target object-name selectors");
        expect(result.request.list_index_objects[1].object_name.empty() &&
                result.request.list_index_objects[1].unique_id == "two-guid",
            "#1053: list-index-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_list_index_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--list-index-object",
        "--list-index-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1053: launch contract should reject list-index-object requests without list index");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--list-index-object",
        "--list-index", "3"
    });
    expect(!missing_targets_result.ok,
        "#1053: launch contract should reject list-index-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--list-index-object",
        "--list-index", "selected",
        "--list-index-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1053: launch contract should reject non-integer list-index values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--list-index-object",
        "--list-index", "-1",
        "--list-index-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1053: launch contract should reject negative list-index values before mutation");
}

void test_parse_launch_arguments_rejects_list_index_object_ambiguity() {
    const auto index_style_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--list-index-object",
        "--style-object",
        "--list-index", "3",
        "--list-index-target-unique-id", "one-guid",
        "--style", "2",
        "--style-target-unique-id", "one-guid"
    });
    expect(!index_style_result.ok,
        "#1053: launch contract should reject simultaneous list-index-object and style-object requests");

    const auto index_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--list-index-object",
        "--clear-property",
        "--property-name", "ListIndex",
        "--list-index", "3",
        "--list-index-target-unique-id", "one-guid"
    });
    expect(!index_property_result.ok,
        "#1053: launch contract should reject list-index-object combined with property commands");

    const auto stray_list_index_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--list-index", "3"
    });
    expect(!stray_list_index_result.ok,
        "#1053: launch contract should reject stray list-index arguments");
}

void test_parse_launch_arguments_for_left_column_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--left-column-object",
        "--left-column", "7",
        "--left-column-target-object-name", "cboCustomer",
        "--left-column-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1054: launch contract should parse left-column-object requests");
    expect(result.request.left_column_object, "#1054: launch contract should detect --left-column-object");
    expect(result.request.left_column_available && result.request.left_column == 7,
        "#1054: left-column-object requests should carry left column values");
    expect(result.request.left_column_objects.size() == 2U,
        "#1054: left-column-object requests should collect left column target selectors");
    if (result.request.left_column_objects.size() == 2U) {
        expect(result.request.left_column_objects[0].object_name == "cboCustomer" &&
                result.request.left_column_objects[0].unique_id.empty(),
            "#1054: left-column-object requests should parse target object-name selectors");
        expect(result.request.left_column_objects[1].object_name.empty() &&
                result.request.left_column_objects[1].unique_id == "two-guid",
            "#1054: left-column-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_left_column_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--left-column-object",
        "--left-column-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1054: launch contract should reject left-column-object requests without left column");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--left-column-object",
        "--left-column", "7"
    });
    expect(!missing_targets_result.ok,
        "#1054: launch contract should reject left-column-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--left-column-object",
        "--left-column", "first",
        "--left-column-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1054: launch contract should reject non-integer left-column values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--left-column-object",
        "--left-column", "-1",
        "--left-column-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1054: launch contract should reject negative left-column values before mutation");
}

void test_parse_launch_arguments_rejects_left_column_object_ambiguity() {
    const auto left_index_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--left-column-object",
        "--list-index-object",
        "--left-column", "7",
        "--left-column-target-unique-id", "one-guid",
        "--list-index", "3",
        "--list-index-target-unique-id", "one-guid"
    });
    expect(!left_index_result.ok,
        "#1054: launch contract should reject simultaneous left-column-object and list-index-object requests");

    const auto left_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--left-column-object",
        "--clear-property",
        "--property-name", "LeftColumn",
        "--left-column", "7",
        "--left-column-target-unique-id", "one-guid"
    });
    expect(!left_property_result.ok,
        "#1054: launch contract should reject left-column-object combined with property commands");

    const auto stray_left_column_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--left-column", "7"
    });
    expect(!stray_left_column_result.ok,
        "#1054: launch contract should reject stray left-column arguments");
}

void test_parse_launch_arguments_for_display_value_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--display-value-object",
        "--display-value", "Bob \"B\"",
        "--display-value-target-object-name", "cboCustomer",
        "--display-value-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1055: launch contract should parse display-value-object requests");
    expect(result.request.display_value_object, "#1055: launch contract should detect --display-value-object");
    expect(result.request.display_value_available && result.request.display_value == "Bob \"B\"",
        "#1055: display-value-object requests should carry display values");
    expect(result.request.display_value_objects.size() == 2U,
        "#1055: display-value-object requests should collect display-value target selectors");
    if (result.request.display_value_objects.size() == 2U) {
        expect(result.request.display_value_objects[0].object_name == "cboCustomer" &&
                result.request.display_value_objects[0].unique_id.empty(),
            "#1055: display-value-object requests should parse target object-name selectors");
        expect(result.request.display_value_objects[1].object_name.empty() &&
                result.request.display_value_objects[1].unique_id == "two-guid",
            "#1055: display-value-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_display_value_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--display-value-object",
        "--display-value-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1055: launch contract should reject display-value-object requests without display value");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--display-value-object",
        "--display-value", "Bob"
    });
    expect(!missing_targets_result.ok,
        "#1055: launch contract should reject display-value-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_display_value_object_ambiguity() {
    const auto display_left_column_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--display-value-object",
        "--left-column-object",
        "--display-value", "Bob",
        "--display-value-target-unique-id", "one-guid",
        "--left-column", "7",
        "--left-column-target-unique-id", "one-guid"
    });
    expect(!display_left_column_result.ok,
        "#1055: launch contract should reject simultaneous display-value-object and left-column-object requests");

    const auto display_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--display-value-object",
        "--clear-property",
        "--property-name", "DisplayValue",
        "--display-value", "Bob",
        "--display-value-target-unique-id", "one-guid"
    });
    expect(!display_property_result.ok,
        "#1055: launch contract should reject display-value-object combined with property commands");

    const auto stray_display_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--display-value", "Bob"
    });
    expect(!stray_display_value_result.ok,
        "#1055: launch contract should reject stray display-value arguments");
}

}  // namespace cf_test_studio_host
