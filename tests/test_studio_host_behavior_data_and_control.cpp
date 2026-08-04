// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_studio_host_support.h"

namespace cf_test_studio_host {
void test_parse_launch_arguments_for_list_item_id_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--list-item-id-object",
        "--list-item-id", "9",
        "--list-item-id-target-object-name", "cmdSave",
        "--list-item-id-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1135: launch contract should parse list-item-id-object requests");
    expect(result.request.list_item_id_object,
        "#1135: launch contract should detect --list-item-id-object");
    expect(result.request.list_item_id_available && result.request.list_item_id == 9,
        "#1135: list-item-id-object requests should carry list-item-id value");
    expect(result.request.list_item_id_objects.size() == 2U,
        "#1135: list-item-id-object requests should collect list_item_id target selectors");
    if (result.request.list_item_id_objects.size() == 2U) {
        expect(result.request.list_item_id_objects[0].object_name == "cmdSave" &&
                result.request.list_item_id_objects[0].unique_id.empty(),
            "#1135: list-item-id-object requests should parse target object-name selectors");
        expect(result.request.list_item_id_objects[1].object_name.empty() &&
                result.request.list_item_id_objects[1].unique_id == "two-guid",
            "#1135: list-item-id-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_list_item_id_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--list-item-id-object",
        "--list-item-id-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1135: launch contract should reject list-item-id-object requests without list-item-id value");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--list-item-id-object",
        "--list-item-id", "manual",
        "--list-item-id-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1135: launch contract should reject non-integer list-item-id values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--list-item-id-object",
        "--list-item-id", "-1",
        "--list-item-id-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1135: launch contract should reject negative list-item-id values before mutation");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--list-item-id-object",
        "--list-item-id", "2"
    });
    expect(!missing_targets_result.ok,
        "#1135: launch contract should reject list-item-id-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_list_item_id_object_ambiguity() {
    const auto list_item_id_locked_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--list-item-id-object",
        "--locked-object",
        "--list-item-id", "2",
        "--list-item-id-target-unique-id", "one-guid",
        "--locked", "true",
        "--locked-target-unique-id", "one-guid"
    });
    expect(!list_item_id_locked_result.ok,
        "#1135: launch contract should reject simultaneous list-item-id-object and locked-object requests");

    const auto list_item_id_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--list-item-id-object",
        "--clear-property",
        "--property-name", "ListItemId",
        "--list-item-id", "2",
        "--list-item-id-target-unique-id", "one-guid"
    });
    expect(!list_item_id_property_result.ok,
        "#1135: launch contract should reject list-item-id-object combined with property commands");

    const auto stray_list_item_id_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--list-item-id", "2"
    });
    expect(!stray_list_item_id_result.ok,
        "#1135: launch contract should reject stray list-item-id arguments");
}

void test_parse_launch_arguments_for_control_source_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--control-source-object",
        "--control-source", "customers.name",
        "--control-source-target-object-name", "txtName",
        "--control-source-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1045: launch contract should parse control-source-object requests");
    expect(result.request.control_source_object, "#1045: launch contract should detect --control-source-object");
    expect(result.request.control_source_available && result.request.control_source == "customers.name",
        "#1045: control-source-object requests should carry control source text");
    expect(result.request.control_source_objects.size() == 2U,
        "#1045: control-source-object requests should collect control source target selectors");
    if (result.request.control_source_objects.size() == 2U) {
        expect(result.request.control_source_objects[0].object_name == "txtName" &&
                result.request.control_source_objects[0].unique_id.empty(),
            "#1045: control-source-object requests should parse target object-name selectors");
        expect(result.request.control_source_objects[1].object_name.empty() &&
                result.request.control_source_objects[1].unique_id == "two-guid",
            "#1045: control-source-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_control_source_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--control-source-object",
        "--control-source-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1045: launch contract should reject control-source-object requests without control source text");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--control-source-object",
        "--control-source", "customers.name"
    });
    expect(!missing_targets_result.ok,
        "#1045: launch contract should reject control-source-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_control_source_object_ambiguity() {
    const auto control_status_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--control-source-object",
        "--status-bar-text-object",
        "--control-source", "customers.name",
        "--control-source-target-unique-id", "one-guid",
        "--status-bar-text", "Ready",
        "--status-bar-text-target-unique-id", "one-guid"
    });
    expect(!control_status_result.ok,
        "#1045: launch contract should reject simultaneous control-source-object and status-bar-text-object requests");

    const auto control_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--control-source-object",
        "--clear-property",
        "--property-name", "ControlSource",
        "--control-source", "customers.name",
        "--control-source-target-unique-id", "one-guid"
    });
    expect(!control_property_result.ok,
        "#1045: launch contract should reject control-source-object combined with property commands");

    const auto stray_control_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--control-source", "customers.name"
    });
    expect(!stray_control_result.ok,
        "#1045: launch contract should reject stray control-source arguments");
}

void test_parse_launch_arguments_for_current_control_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--current-control-object",
        "--current-control", "txtCity",
        "--current-control-target-object-name", "frmCustomer",
        "--current-control-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1072: launch contract should parse current-control-object requests");
    expect(result.request.current_control_object,
        "#1072: launch contract should detect --current-control-object");
    expect(result.request.current_control_available && result.request.current_control == "txtCity",
        "#1072: current-control-object requests should carry current control text");
    expect(result.request.current_control_objects.size() == 2U,
        "#1072: current-control-object requests should collect current-control target selectors");
    if (result.request.current_control_objects.size() == 2U) {
        expect(result.request.current_control_objects[0].object_name == "frmCustomer" &&
                result.request.current_control_objects[0].unique_id.empty(),
            "#1072: current-control-object requests should parse target object-name selectors");
        expect(result.request.current_control_objects[1].object_name.empty() &&
                result.request.current_control_objects[1].unique_id == "two-guid",
            "#1072: current-control-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_current_control_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--current-control-object",
        "--current-control-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1072: launch contract should reject current-control-object requests without current control text");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--current-control-object",
        "--current-control", "txtCity"
    });
    expect(!missing_targets_result.ok,
        "#1072: launch contract should reject current-control-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_current_control_object_ambiguity() {
    const auto current_control_source_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--current-control-object",
        "--control-source-object",
        "--current-control", "txtCity",
        "--current-control-target-unique-id", "one-guid",
        "--control-source", "customers.name",
        "--control-source-target-unique-id", "one-guid"
    });
    expect(!current_control_source_result.ok,
        "#1072: launch contract should reject simultaneous current-control-object and control-source-object requests");

    const auto current_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--current-control-object",
        "--clear-property",
        "--property-name", "CurrentControl",
        "--current-control", "txtCity",
        "--current-control-target-unique-id", "one-guid"
    });
    expect(!current_property_result.ok,
        "#1072: launch contract should reject current-control-object combined with property commands");

    const auto stray_current_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--current-control", "txtCity"
    });
    expect(!stray_current_result.ok,
        "#1072: launch contract should reject stray current-control arguments");
}

void test_parse_launch_arguments_for_bind_controls_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--bind-controls-object",
        "--bind-controls", "false",
        "--bind-controls-target-object-name", "frmCustomer",
        "--bind-controls-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1146: launch contract should parse bind-controls-object requests");
    expect(result.request.bind_controls_object,
        "#1146: launch contract should detect --bind-controls-object");
    expect(result.request.bind_controls_available && !result.request.bind_controls,
        "#1146: bind-controls-object requests should carry bind controls state");
    expect(result.request.bind_controls_objects.size() == 2U,
        "#1146: bind-controls-object requests should collect bind-controls target selectors");
    if (result.request.bind_controls_objects.size() == 2U) {
        expect(result.request.bind_controls_objects[0].object_name == "frmCustomer" &&
                result.request.bind_controls_objects[0].unique_id.empty(),
            "#1146: bind-controls-object requests should parse target object-name selectors");
        expect(result.request.bind_controls_objects[1].object_name.empty() &&
                result.request.bind_controls_objects[1].unique_id == "two-guid",
            "#1146: bind-controls-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_bind_controls_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--bind-controls-object",
        "--bind-controls-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1146: launch contract should reject bind-controls-object requests without bind controls state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--bind-controls-object",
        "--bind-controls", "false"
    });
    expect(!missing_targets_result.ok,
        "#1146: launch contract should reject bind-controls-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--bind-controls-object",
        "--bind-controls", "sometimes",
        "--bind-controls-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1146: launch contract should reject invalid bind-controls boolean values");
}

void test_parse_launch_arguments_rejects_bind_controls_object_ambiguity() {
    const auto bind_controls_allow_output_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--bind-controls-object",
        "--allow-output-object",
        "--bind-controls", "false",
        "--bind-controls-target-unique-id", "one-guid",
        "--allow-output", "false",
        "--allow-output-target-unique-id", "one-guid"
    });
    expect(!bind_controls_allow_output_result.ok,
        "#1146: launch contract should reject simultaneous bind-controls-object and allow-output-object requests");

    const auto bind_controls_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--bind-controls-object",
        "--clear-property",
        "--property-name", "BindControls",
        "--bind-controls", "false",
        "--bind-controls-target-unique-id", "one-guid"
    });
    expect(!bind_controls_property_result.ok,
        "#1146: launch contract should reject bind-controls-object combined with property commands");

    const auto stray_bind_controls_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--bind-controls", "false"
    });
    expect(!stray_bind_controls_result.ok,
        "#1146: launch contract should reject stray bind-controls arguments");
}

void test_parse_launch_arguments_for_auto_verb_menu_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--auto-verb-menu-object",
        "--auto-verb-menu", "false",
        "--auto-verb-menu-target-object-name", "frmCustomer",
        "--auto-verb-menu-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1145: launch contract should parse auto-verb-menu-object requests");
    expect(result.request.auto_verb_menu_object,
        "#1145: launch contract should detect --auto-verb-menu-object");
    expect(result.request.auto_verb_menu_available && !result.request.auto_verb_menu,
        "#1145: auto-verb-menu-object requests should carry auto verb menu state");
    expect(result.request.auto_verb_menu_objects.size() == 2U,
        "#1145: auto-verb-menu-object requests should collect auto-verb-menu target selectors");
    if (result.request.auto_verb_menu_objects.size() == 2U) {
        expect(result.request.auto_verb_menu_objects[0].object_name == "frmCustomer" &&
                result.request.auto_verb_menu_objects[0].unique_id.empty(),
            "#1145: auto-verb-menu-object requests should parse target object-name selectors");
        expect(result.request.auto_verb_menu_objects[1].object_name.empty() &&
                result.request.auto_verb_menu_objects[1].unique_id == "two-guid",
            "#1145: auto-verb-menu-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_auto_verb_menu_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--auto-verb-menu-object",
        "--auto-verb-menu-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1145: launch contract should reject auto-verb-menu-object requests without auto verb menu state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--auto-verb-menu-object",
        "--auto-verb-menu", "false"
    });
    expect(!missing_targets_result.ok,
        "#1145: launch contract should reject auto-verb-menu-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--auto-verb-menu-object",
        "--auto-verb-menu", "sometimes",
        "--auto-verb-menu-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1145: launch contract should reject invalid auto-verb-menu boolean values");
}

void test_parse_launch_arguments_rejects_auto_verb_menu_object_ambiguity() {
    const auto auto_verb_menu_allow_output_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--auto-verb-menu-object",
        "--allow-output-object",
        "--auto-verb-menu", "false",
        "--auto-verb-menu-target-unique-id", "one-guid",
        "--allow-output", "false",
        "--allow-output-target-unique-id", "one-guid"
    });
    expect(!auto_verb_menu_allow_output_result.ok,
        "#1145: launch contract should reject simultaneous auto-verb-menu-object and allow-output-object requests");

    const auto auto_verb_menu_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--auto-verb-menu-object",
        "--clear-property",
        "--property-name", "AutoVerbMenu",
        "--auto-verb-menu", "false",
        "--auto-verb-menu-target-unique-id", "one-guid"
    });
    expect(!auto_verb_menu_property_result.ok,
        "#1145: launch contract should reject auto-verb-menu-object combined with property commands");

    const auto stray_auto_verb_menu_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--auto-verb-menu", "false"
    });
    expect(!stray_auto_verb_menu_result.ok,
        "#1145: launch contract should reject stray auto-verb-menu arguments");
}

void test_parse_launch_arguments_for_auto_size_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--auto-size-object",
        "--auto-size", "false",
        "--auto-size-target-object-name", "frmCustomer",
        "--auto-size-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1079: launch contract should parse auto-size-object requests");
    expect(result.request.auto_size_object,
        "#1079: launch contract should detect --auto-size-object");
    expect(result.request.auto_size_available && !result.request.auto_size,
        "#1079: auto-size-object requests should carry auto size state");
    expect(result.request.auto_size_objects.size() == 2U,
        "#1079: auto-size-object requests should collect auto-size target selectors");
    if (result.request.auto_size_objects.size() == 2U) {
        expect(result.request.auto_size_objects[0].object_name == "frmCustomer" &&
                result.request.auto_size_objects[0].unique_id.empty(),
            "#1079: auto-size-object requests should parse target object-name selectors");
        expect(result.request.auto_size_objects[1].object_name.empty() &&
                result.request.auto_size_objects[1].unique_id == "two-guid",
            "#1079: auto-size-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_auto_size_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--auto-size-object",
        "--auto-size-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1079: launch contract should reject auto-size-object requests without auto size state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--auto-size-object",
        "--auto-size", "false"
    });
    expect(!missing_targets_result.ok,
        "#1079: launch contract should reject auto-size-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--auto-size-object",
        "--auto-size", "sometimes",
        "--auto-size-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1079: launch contract should reject invalid auto-size boolean values");
}

void test_parse_launch_arguments_rejects_auto_size_object_ambiguity() {
    const auto auto_size_auto_center_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--auto-size-object",
        "--auto-center-object",
        "--auto-size", "false",
        "--auto-size-target-unique-id", "one-guid",
        "--auto-center", "false",
        "--auto-center-target-unique-id", "one-guid"
    });
    expect(!auto_size_auto_center_result.ok,
        "#1079: launch contract should reject simultaneous auto-size-object and auto-center-object requests");

    const auto auto_size_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--auto-size-object",
        "--clear-property",
        "--property-name", "AutoSize",
        "--auto-size", "false",
        "--auto-size-target-unique-id", "one-guid"
    });
    expect(!auto_size_property_result.ok,
        "#1079: launch contract should reject auto-size-object combined with property commands");

    const auto stray_auto_size_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--auto-size", "false"
    });
    expect(!stray_auto_size_result.ok,
        "#1079: launch contract should reject stray auto-size arguments");
}

void test_parse_launch_arguments_for_auto_release_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--auto-release-object",
        "--auto-release", "false",
        "--auto-release-target-object-name", "frmCustomer",
        "--auto-release-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1080: launch contract should parse auto-release-object requests");
    expect(result.request.auto_release_object,
        "#1080: launch contract should detect --auto-release-object");
    expect(result.request.auto_release_available && !result.request.auto_release,
        "#1080: auto-release-object requests should carry auto release state");
    expect(result.request.auto_release_objects.size() == 2U,
        "#1080: auto-release-object requests should collect auto-release target selectors");
    if (result.request.auto_release_objects.size() == 2U) {
        expect(result.request.auto_release_objects[0].object_name == "frmCustomer" &&
                result.request.auto_release_objects[0].unique_id.empty(),
            "#1080: auto-release-object requests should parse target object-name selectors");
        expect(result.request.auto_release_objects[1].object_name.empty() &&
                result.request.auto_release_objects[1].unique_id == "two-guid",
            "#1080: auto-release-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_auto_release_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--auto-release-object",
        "--auto-release-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1080: launch contract should reject auto-release-object requests without auto release state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--auto-release-object",
        "--auto-release", "false"
    });
    expect(!missing_targets_result.ok,
        "#1080: launch contract should reject auto-release-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--auto-release-object",
        "--auto-release", "sometimes",
        "--auto-release-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1080: launch contract should reject invalid auto-release boolean values");
}

void test_parse_launch_arguments_rejects_auto_release_object_ambiguity() {
    const auto auto_release_auto_size_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--auto-release-object",
        "--auto-size-object",
        "--auto-release", "false",
        "--auto-release-target-unique-id", "one-guid",
        "--auto-size", "false",
        "--auto-size-target-unique-id", "one-guid"
    });
    expect(!auto_release_auto_size_result.ok,
        "#1080: launch contract should reject simultaneous auto-release-object and auto-size-object requests");

    const auto auto_release_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--auto-release-object",
        "--clear-property",
        "--property-name", "AutoRelease",
        "--auto-release", "false",
        "--auto-release-target-unique-id", "one-guid"
    });
    expect(!auto_release_property_result.ok,
        "#1080: launch contract should reject auto-release-object combined with property commands");

    const auto stray_auto_release_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--auto-release", "false"
    });
    expect(!stray_auto_release_result.ok,
        "#1080: launch contract should reject stray auto-release arguments");
}

void test_parse_launch_arguments_for_clip_controls_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--clip-controls-object",
        "--clip-controls", "false",
        "--clip-controls-target-object-name", "frmCustomer",
        "--clip-controls-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1083: launch contract should parse clip-controls-object requests");
    expect(result.request.clip_controls_object,
        "#1083: launch contract should detect --clip-controls-object");
    expect(result.request.clip_controls_available && !result.request.clip_controls,
        "#1083: clip-controls-object requests should carry clip controls state");
    expect(result.request.clip_controls_objects.size() == 2U,
        "#1083: clip-controls-object requests should collect clip_controls target selectors");
    if (result.request.clip_controls_objects.size() == 2U) {
        expect(result.request.clip_controls_objects[0].object_name == "frmCustomer" &&
                result.request.clip_controls_objects[0].unique_id.empty(),
            "#1083: clip-controls-object requests should parse target object-name selectors");
        expect(result.request.clip_controls_objects[1].object_name.empty() &&
                result.request.clip_controls_objects[1].unique_id == "two-guid",
            "#1083: clip-controls-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_clip_controls_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--clip-controls-object",
        "--clip-controls-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1083: launch contract should reject clip-controls-object requests without clip controls state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--clip-controls-object",
        "--clip-controls", "false"
    });
    expect(!missing_targets_result.ok,
        "#1083: launch contract should reject clip-controls-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--clip-controls-object",
        "--clip-controls", "sometimes",
        "--clip-controls-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1083: launch contract should reject invalid clip-controls boolean values");
}

void test_parse_launch_arguments_rejects_clip_controls_object_ambiguity() {
    const auto clip_controls_auto_size_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--clip-controls-object",
        "--auto-size-object",
        "--clip-controls", "false",
        "--clip-controls-target-unique-id", "one-guid",
        "--auto-size", "false",
        "--auto-size-target-unique-id", "one-guid"
    });
    expect(!clip_controls_auto_size_result.ok,
        "#1083: launch contract should reject simultaneous clip-controls-object and auto-size-object requests");

    const auto clip_controls_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--clip-controls-object",
        "--clear-property",
        "--property-name", "Dockable",
        "--clip-controls", "false",
        "--clip-controls-target-unique-id", "one-guid"
    });
    expect(!clip_controls_property_result.ok,
        "#1083: launch contract should reject clip-controls-object combined with property commands");

    const auto stray_clip_controls_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--clip-controls", "false"
    });
    expect(!stray_clip_controls_result.ok,
        "#1083: launch contract should reject stray clip-controls arguments");
}

}  // namespace cf_test_studio_host
