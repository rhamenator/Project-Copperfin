// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_studio_host_support.h"

namespace cf_test_studio_host {
void test_parse_launch_arguments_for_selected_back_color_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--selected-back-color-object",
        "--selected-back-color", "16777215",
        "--selected-back-color-target-object-name", "lstOrders",
        "--selected-back-color-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1056: launch contract should parse selected-back-color-object requests");
    expect(result.request.selected_back_color_object,
        "#1056: launch contract should detect --selected-back-color-object");
    expect(result.request.selected_back_color_available && result.request.selected_back_color == 16777215,
        "#1056: selected-back-color-object requests should carry selected back color values");
    expect(result.request.selected_back_color_objects.size() == 2U,
        "#1056: selected-back-color-object requests should collect selected-back-color target selectors");
    if (result.request.selected_back_color_objects.size() == 2U) {
        expect(result.request.selected_back_color_objects[0].object_name == "lstOrders" &&
                result.request.selected_back_color_objects[0].unique_id.empty(),
            "#1056: selected-back-color-object requests should parse target object-name selectors");
        expect(result.request.selected_back_color_objects[1].object_name.empty() &&
                result.request.selected_back_color_objects[1].unique_id == "two-guid",
            "#1056: selected-back-color-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_selected_back_color_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--selected-back-color-object",
        "--selected-back-color-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1056: launch contract should reject selected-back-color-object requests without selected back color");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--selected-back-color-object",
        "--selected-back-color", "16777215"
    });
    expect(!missing_targets_result.ok,
        "#1056: launch contract should reject selected-back-color-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--selected-back-color-object",
        "--selected-back-color", "white",
        "--selected-back-color-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1056: launch contract should reject non-integer selected-back-color values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--selected-back-color-object",
        "--selected-back-color", "-1",
        "--selected-back-color-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1056: launch contract should reject negative selected-back-color values before mutation");
}

void test_parse_launch_arguments_rejects_selected_back_color_object_ambiguity() {
    const auto selected_back_display_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--selected-back-color-object",
        "--display-value-object",
        "--selected-back-color", "16777215",
        "--selected-back-color-target-unique-id", "one-guid",
        "--display-value", "Bob",
        "--display-value-target-unique-id", "one-guid"
    });
    expect(!selected_back_display_value_result.ok,
        "#1056: launch contract should reject simultaneous selected-back-color-object and display-value-object requests");

    const auto selected_back_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--selected-back-color-object",
        "--clear-property",
        "--property-name", "SelectedBackColor",
        "--selected-back-color", "16777215",
        "--selected-back-color-target-unique-id", "one-guid"
    });
    expect(!selected_back_property_result.ok,
        "#1056: launch contract should reject selected-back-color-object combined with property commands");

    const auto stray_selected_back_color_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--selected-back-color", "16777215"
    });
    expect(!stray_selected_back_color_result.ok,
        "#1056: launch contract should reject stray selected-back-color arguments");
}

void test_parse_launch_arguments_for_selected_fore_color_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--selected-fore-color-object",
        "--selected-fore-color", "255",
        "--selected-fore-color-target-object-name", "lstOrders",
        "--selected-fore-color-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1057: launch contract should parse selected-fore-color-object requests");
    expect(result.request.selected_fore_color_object,
        "#1057: launch contract should detect --selected-fore-color-object");
    expect(result.request.selected_fore_color_available && result.request.selected_fore_color == 255,
        "#1057: selected-fore-color-object requests should carry selected fore color values");
    expect(result.request.selected_fore_color_objects.size() == 2U,
        "#1057: selected-fore-color-object requests should collect selected-fore-color target selectors");
    if (result.request.selected_fore_color_objects.size() == 2U) {
        expect(result.request.selected_fore_color_objects[0].object_name == "lstOrders" &&
                result.request.selected_fore_color_objects[0].unique_id.empty(),
            "#1057: selected-fore-color-object requests should parse target object-name selectors");
        expect(result.request.selected_fore_color_objects[1].object_name.empty() &&
                result.request.selected_fore_color_objects[1].unique_id == "two-guid",
            "#1057: selected-fore-color-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_selected_fore_color_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--selected-fore-color-object",
        "--selected-fore-color-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1057: launch contract should reject selected-fore-color-object requests without selected fore color");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--selected-fore-color-object",
        "--selected-fore-color", "255"
    });
    expect(!missing_targets_result.ok,
        "#1057: launch contract should reject selected-fore-color-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--selected-fore-color-object",
        "--selected-fore-color", "blue",
        "--selected-fore-color-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1057: launch contract should reject non-integer selected-fore-color values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--selected-fore-color-object",
        "--selected-fore-color", "-1",
        "--selected-fore-color-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1057: launch contract should reject negative selected-fore-color values before mutation");
}

void test_parse_launch_arguments_rejects_selected_fore_color_object_ambiguity() {
    const auto selected_fore_back_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--selected-fore-color-object",
        "--selected-back-color-object",
        "--selected-fore-color", "255",
        "--selected-fore-color-target-unique-id", "one-guid",
        "--selected-back-color", "16777215",
        "--selected-back-color-target-unique-id", "one-guid"
    });
    expect(!selected_fore_back_result.ok,
        "#1057: launch contract should reject simultaneous selected-fore-color-object and selected-back-color-object requests");

    const auto selected_fore_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--selected-fore-color-object",
        "--clear-property",
        "--property-name", "SelectedForeColor",
        "--selected-fore-color", "255",
        "--selected-fore-color-target-unique-id", "one-guid"
    });
    expect(!selected_fore_property_result.ok,
        "#1057: launch contract should reject selected-fore-color-object combined with property commands");

    const auto stray_selected_fore_color_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--selected-fore-color", "255"
    });
    expect(!stray_selected_fore_color_result.ok,
        "#1057: launch contract should reject stray selected-fore-color arguments");
}

void test_parse_launch_arguments_for_selected_item_back_color_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--selected-item-back-color-object",
        "--selected-item-back-color", "65280",
        "--selected-item-back-color-target-object-name", "lstOrders",
        "--selected-item-back-color-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1058: launch contract should parse selected-item-back-color-object requests");
    expect(result.request.selected_item_back_color_object,
        "#1058: launch contract should detect --selected-item-back-color-object");
    expect(result.request.selected_item_back_color_available && result.request.selected_item_back_color == 65280,
        "#1058: selected-item-back-color-object requests should carry selected item back color values");
    expect(result.request.selected_item_back_color_objects.size() == 2U,
        "#1058: selected-item-back-color-object requests should collect selected-item-back-color target selectors");
    if (result.request.selected_item_back_color_objects.size() == 2U) {
        expect(result.request.selected_item_back_color_objects[0].object_name == "lstOrders" &&
                result.request.selected_item_back_color_objects[0].unique_id.empty(),
            "#1058: selected-item-back-color-object requests should parse target object-name selectors");
        expect(result.request.selected_item_back_color_objects[1].object_name.empty() &&
                result.request.selected_item_back_color_objects[1].unique_id == "two-guid",
            "#1058: selected-item-back-color-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_selected_item_back_color_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--selected-item-back-color-object",
        "--selected-item-back-color-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1058: launch contract should reject selected-item-back-color-object requests without selected item back color");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--selected-item-back-color-object",
        "--selected-item-back-color", "65280"
    });
    expect(!missing_targets_result.ok,
        "#1058: launch contract should reject selected-item-back-color-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--selected-item-back-color-object",
        "--selected-item-back-color", "green",
        "--selected-item-back-color-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1058: launch contract should reject non-integer selected-item-back-color values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--selected-item-back-color-object",
        "--selected-item-back-color", "-1",
        "--selected-item-back-color-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1058: launch contract should reject negative selected-item-back-color values before mutation");
}

void test_parse_launch_arguments_rejects_selected_item_back_color_object_ambiguity() {
    const auto selected_item_selected_fore_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--selected-item-back-color-object",
        "--selected-fore-color-object",
        "--selected-item-back-color", "65280",
        "--selected-item-back-color-target-unique-id", "one-guid",
        "--selected-fore-color", "255",
        "--selected-fore-color-target-unique-id", "one-guid"
    });
    expect(!selected_item_selected_fore_result.ok,
        "#1058: launch contract should reject simultaneous selected-item-back-color-object and selected-fore-color-object requests");

    const auto selected_item_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--selected-item-back-color-object",
        "--clear-property",
        "--property-name", "SelectedItemBackColor",
        "--selected-item-back-color", "65280",
        "--selected-item-back-color-target-unique-id", "one-guid"
    });
    expect(!selected_item_property_result.ok,
        "#1058: launch contract should reject selected-item-back-color-object combined with property commands");

    const auto stray_selected_item_back_color_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--selected-item-back-color", "65280"
    });
    expect(!stray_selected_item_back_color_result.ok,
        "#1058: launch contract should reject stray selected-item-back-color arguments");
}

void test_parse_launch_arguments_for_selected_item_fore_color_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--selected-item-fore-color-object",
        "--selected-item-fore-color", "65280",
        "--selected-item-fore-color-target-object-name", "lstOrders",
        "--selected-item-fore-color-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1059: launch contract should parse selected-item-fore-color-object requests");
    expect(result.request.selected_item_fore_color_object,
        "#1059: launch contract should detect --selected-item-fore-color-object");
    expect(result.request.selected_item_fore_color_available && result.request.selected_item_fore_color == 65280,
        "#1059: selected-item-fore-color-object requests should carry selected item fore color values");
    expect(result.request.selected_item_fore_color_objects.size() == 2U,
        "#1059: selected-item-fore-color-object requests should collect selected-item-fore-color target selectors");
    if (result.request.selected_item_fore_color_objects.size() == 2U) {
        expect(result.request.selected_item_fore_color_objects[0].object_name == "lstOrders" &&
                result.request.selected_item_fore_color_objects[0].unique_id.empty(),
            "#1059: selected-item-fore-color-object requests should parse target object-name selectors");
        expect(result.request.selected_item_fore_color_objects[1].object_name.empty() &&
                result.request.selected_item_fore_color_objects[1].unique_id == "two-guid",
            "#1059: selected-item-fore-color-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_selected_item_fore_color_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--selected-item-fore-color-object",
        "--selected-item-fore-color-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1059: launch contract should reject selected-item-fore-color-object requests without selected item fore color");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--selected-item-fore-color-object",
        "--selected-item-fore-color", "65280"
    });
    expect(!missing_targets_result.ok,
        "#1059: launch contract should reject selected-item-fore-color-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--selected-item-fore-color-object",
        "--selected-item-fore-color", "green",
        "--selected-item-fore-color-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1059: launch contract should reject non-integer selected-item-fore-color values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--selected-item-fore-color-object",
        "--selected-item-fore-color", "-1",
        "--selected-item-fore-color-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1059: launch contract should reject negative selected-item-fore-color values before mutation");
}

void test_parse_launch_arguments_rejects_selected_item_fore_color_object_ambiguity() {
    const auto selected_item_fore_back_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--selected-item-fore-color-object",
        "--selected-item-back-color-object",
        "--selected-item-fore-color", "65280",
        "--selected-item-fore-color-target-unique-id", "one-guid",
        "--selected-item-back-color", "65280",
        "--selected-item-back-color-target-unique-id", "one-guid"
    });
    expect(!selected_item_fore_back_result.ok,
        "#1059: launch contract should reject simultaneous selected-item-fore-color-object and selected-item-back-color-object requests");

    const auto selected_item_fore_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--selected-item-fore-color-object",
        "--clear-property",
        "--property-name", "SelectedItemForeColor",
        "--selected-item-fore-color", "65280",
        "--selected-item-fore-color-target-unique-id", "one-guid"
    });
    expect(!selected_item_fore_property_result.ok,
        "#1059: launch contract should reject selected-item-fore-color-object combined with property commands");

    const auto stray_selected_item_fore_color_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--selected-item-fore-color", "65280"
    });
    expect(!stray_selected_item_fore_color_result.ok,
        "#1059: launch contract should reject stray selected-item-fore-color arguments");
}

void test_parse_launch_arguments_for_disabled_item_back_color_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--disabled-item-back-color-object",
        "--disabled-item-back-color", "65280",
        "--disabled-item-back-color-target-object-name", "lstOrders",
        "--disabled-item-back-color-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1060: launch contract should parse disabled-item-back-color-object requests");
    expect(result.request.disabled_item_back_color_object,
        "#1060: launch contract should detect --disabled-item-back-color-object");
    expect(result.request.disabled_item_back_color_available && result.request.disabled_item_back_color == 65280,
        "#1060: disabled-item-back-color-object requests should carry disabled item back color values");
    expect(result.request.disabled_item_back_color_objects.size() == 2U,
        "#1060: disabled-item-back-color-object requests should collect disabled-item-back-color target selectors");
    if (result.request.disabled_item_back_color_objects.size() == 2U) {
        expect(result.request.disabled_item_back_color_objects[0].object_name == "lstOrders" &&
                result.request.disabled_item_back_color_objects[0].unique_id.empty(),
            "#1060: disabled-item-back-color-object requests should parse target object-name selectors");
        expect(result.request.disabled_item_back_color_objects[1].object_name.empty() &&
                result.request.disabled_item_back_color_objects[1].unique_id == "two-guid",
            "#1060: disabled-item-back-color-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_disabled_item_back_color_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--disabled-item-back-color-object",
        "--disabled-item-back-color-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1060: launch contract should reject disabled-item-back-color-object requests without disabled item back color");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--disabled-item-back-color-object",
        "--disabled-item-back-color", "65280"
    });
    expect(!missing_targets_result.ok,
        "#1060: launch contract should reject disabled-item-back-color-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--disabled-item-back-color-object",
        "--disabled-item-back-color", "green",
        "--disabled-item-back-color-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1060: launch contract should reject non-integer disabled-item-back-color values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--disabled-item-back-color-object",
        "--disabled-item-back-color", "-1",
        "--disabled-item-back-color-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1060: launch contract should reject negative disabled-item-back-color values before mutation");
}

void test_parse_launch_arguments_rejects_disabled_item_back_color_object_ambiguity() {
    const auto disabled_selected_item_fore_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--disabled-item-back-color-object",
        "--selected-item-fore-color-object",
        "--disabled-item-back-color", "65280",
        "--disabled-item-back-color-target-unique-id", "one-guid",
        "--selected-item-fore-color", "65280",
        "--selected-item-fore-color-target-unique-id", "one-guid"
    });
    expect(!disabled_selected_item_fore_result.ok,
        "#1060: launch contract should reject simultaneous disabled-item-back-color-object and selected-item-fore-color-object requests");

    const auto disabled_item_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--disabled-item-back-color-object",
        "--clear-property",
        "--property-name", "DisabledItemBackColor",
        "--disabled-item-back-color", "65280",
        "--disabled-item-back-color-target-unique-id", "one-guid"
    });
    expect(!disabled_item_property_result.ok,
        "#1060: launch contract should reject disabled-item-back-color-object combined with property commands");

    const auto stray_disabled_item_back_color_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--disabled-item-back-color", "65280"
    });
    expect(!stray_disabled_item_back_color_result.ok,
        "#1060: launch contract should reject stray disabled-item-back-color arguments");
}

void test_parse_launch_arguments_for_disabled_item_fore_color_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--disabled-item-fore-color-object",
        "--disabled-item-fore-color", "65280",
        "--disabled-item-fore-color-target-object-name", "lstOrders",
        "--disabled-item-fore-color-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1061: launch contract should parse disabled-item-fore-color-object requests");
    expect(result.request.disabled_item_fore_color_object,
        "#1061: launch contract should detect --disabled-item-fore-color-object");
    expect(result.request.disabled_item_fore_color_available && result.request.disabled_item_fore_color == 65280,
        "#1061: disabled-item-fore-color-object requests should carry disabled item fore color values");
    expect(result.request.disabled_item_fore_color_objects.size() == 2U,
        "#1061: disabled-item-fore-color-object requests should collect disabled-item-fore-color target selectors");
    if (result.request.disabled_item_fore_color_objects.size() == 2U) {
        expect(result.request.disabled_item_fore_color_objects[0].object_name == "lstOrders" &&
                result.request.disabled_item_fore_color_objects[0].unique_id.empty(),
            "#1061: disabled-item-fore-color-object requests should parse target object-name selectors");
        expect(result.request.disabled_item_fore_color_objects[1].object_name.empty() &&
                result.request.disabled_item_fore_color_objects[1].unique_id == "two-guid",
            "#1061: disabled-item-fore-color-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_disabled_item_fore_color_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--disabled-item-fore-color-object",
        "--disabled-item-fore-color-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1061: launch contract should reject disabled-item-fore-color-object requests without disabled item fore color");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--disabled-item-fore-color-object",
        "--disabled-item-fore-color", "65280"
    });
    expect(!missing_targets_result.ok,
        "#1061: launch contract should reject disabled-item-fore-color-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--disabled-item-fore-color-object",
        "--disabled-item-fore-color", "green",
        "--disabled-item-fore-color-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1061: launch contract should reject non-integer disabled-item-fore-color values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--disabled-item-fore-color-object",
        "--disabled-item-fore-color", "-1",
        "--disabled-item-fore-color-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1061: launch contract should reject negative disabled-item-fore-color values before mutation");
}

void test_parse_launch_arguments_rejects_disabled_item_fore_color_object_ambiguity() {
    const auto disabled_fore_back_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--disabled-item-fore-color-object",
        "--disabled-item-back-color-object",
        "--disabled-item-fore-color", "65280",
        "--disabled-item-fore-color-target-unique-id", "one-guid",
        "--disabled-item-back-color", "65280",
        "--disabled-item-back-color-target-unique-id", "one-guid"
    });
    expect(!disabled_fore_back_result.ok,
        "#1061: launch contract should reject simultaneous disabled-item-fore-color-object and disabled-item-back-color-object requests");

    const auto disabled_item_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--disabled-item-fore-color-object",
        "--clear-property",
        "--property-name", "DisabledItemForeColor",
        "--disabled-item-fore-color", "65280",
        "--disabled-item-fore-color-target-unique-id", "one-guid"
    });
    expect(!disabled_item_property_result.ok,
        "#1061: launch contract should reject disabled-item-fore-color-object combined with property commands");

    const auto stray_disabled_item_fore_color_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--disabled-item-fore-color", "65280"
    });
    expect(!stray_disabled_item_fore_color_result.ok,
        "#1061: launch contract should reject stray disabled-item-fore-color arguments");
}

void test_parse_launch_arguments_for_item_back_color_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--item-back-color-object",
        "--item-back-color", "65280",
        "--item-back-color-target-object-name", "lstOrders",
        "--item-back-color-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1062: launch contract should parse item-back-color-object requests");
    expect(result.request.item_back_color_object,
        "#1062: launch contract should detect --item-back-color-object");
    expect(result.request.item_back_color_available && result.request.item_back_color == 65280,
        "#1062: item-back-color-object requests should carry item back color values");
    expect(result.request.item_back_color_objects.size() == 2U,
        "#1062: item-back-color-object requests should collect item-back-color target selectors");
    if (result.request.item_back_color_objects.size() == 2U) {
        expect(result.request.item_back_color_objects[0].object_name == "lstOrders" &&
                result.request.item_back_color_objects[0].unique_id.empty(),
            "#1062: item-back-color-object requests should parse target object-name selectors");
        expect(result.request.item_back_color_objects[1].object_name.empty() &&
                result.request.item_back_color_objects[1].unique_id == "two-guid",
            "#1062: item-back-color-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_item_back_color_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--item-back-color-object",
        "--item-back-color-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1062: launch contract should reject item-back-color-object requests without item back color");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--item-back-color-object",
        "--item-back-color", "65280"
    });
    expect(!missing_targets_result.ok,
        "#1062: launch contract should reject item-back-color-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--item-back-color-object",
        "--item-back-color", "green",
        "--item-back-color-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1062: launch contract should reject non-integer item-back-color values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--item-back-color-object",
        "--item-back-color", "-1",
        "--item-back-color-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1062: launch contract should reject negative item-back-color values before mutation");
}

void test_parse_launch_arguments_rejects_item_back_color_object_ambiguity() {
    const auto item_disabled_fore_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--item-back-color-object",
        "--disabled-item-fore-color-object",
        "--item-back-color", "65280",
        "--item-back-color-target-unique-id", "one-guid",
        "--disabled-item-fore-color", "65280",
        "--disabled-item-fore-color-target-unique-id", "one-guid"
    });
    expect(!item_disabled_fore_result.ok,
        "#1062: launch contract should reject simultaneous item-back-color-object and disabled-item-fore-color-object requests");

    const auto item_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--item-back-color-object",
        "--clear-property",
        "--property-name", "ItemBackColor",
        "--item-back-color", "65280",
        "--item-back-color-target-unique-id", "one-guid"
    });
    expect(!item_property_result.ok,
        "#1062: launch contract should reject item-back-color-object combined with property commands");

    const auto stray_item_back_color_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--item-back-color", "65280"
    });
    expect(!stray_item_back_color_result.ok,
        "#1062: launch contract should reject stray item-back-color arguments");
}

void test_parse_launch_arguments_for_item_fore_color_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--item-fore-color-object",
        "--item-fore-color", "65280",
        "--item-fore-color-target-object-name", "lstOrders",
        "--item-fore-color-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1063: launch contract should parse item-fore-color-object requests");
    expect(result.request.item_fore_color_object,
        "#1063: launch contract should detect --item-fore-color-object");
    expect(result.request.item_fore_color_available && result.request.item_fore_color == 65280,
        "#1063: item-fore-color-object requests should carry item fore color values");
    expect(result.request.item_fore_color_objects.size() == 2U,
        "#1063: item-fore-color-object requests should collect item-fore-color target selectors");
    if (result.request.item_fore_color_objects.size() == 2U) {
        expect(result.request.item_fore_color_objects[0].object_name == "lstOrders" &&
                result.request.item_fore_color_objects[0].unique_id.empty(),
            "#1063: item-fore-color-object requests should parse target object-name selectors");
        expect(result.request.item_fore_color_objects[1].object_name.empty() &&
                result.request.item_fore_color_objects[1].unique_id == "two-guid",
            "#1063: item-fore-color-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_item_fore_color_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--item-fore-color-object",
        "--item-fore-color-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1063: launch contract should reject item-fore-color-object requests without item fore color");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--item-fore-color-object",
        "--item-fore-color", "65280"
    });
    expect(!missing_targets_result.ok,
        "#1063: launch contract should reject item-fore-color-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--item-fore-color-object",
        "--item-fore-color", "green",
        "--item-fore-color-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1063: launch contract should reject non-integer item-fore-color values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--item-fore-color-object",
        "--item-fore-color", "-1",
        "--item-fore-color-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1063: launch contract should reject negative item-fore-color values before mutation");
}

void test_parse_launch_arguments_rejects_item_fore_color_object_ambiguity() {
    const auto item_fore_back_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--item-fore-color-object",
        "--item-back-color-object",
        "--item-fore-color", "65280",
        "--item-fore-color-target-unique-id", "one-guid",
        "--item-back-color", "65280",
        "--item-back-color-target-unique-id", "one-guid"
    });
    expect(!item_fore_back_result.ok,
        "#1063: launch contract should reject simultaneous item-fore-color-object and item-back-color-object requests");

    const auto item_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--item-fore-color-object",
        "--clear-property",
        "--property-name", "ItemForeColor",
        "--item-fore-color", "65280",
        "--item-fore-color-target-unique-id", "one-guid"
    });
    expect(!item_property_result.ok,
        "#1063: launch contract should reject item-fore-color-object combined with property commands");

    const auto stray_item_fore_color_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--item-fore-color", "65280"
    });
    expect(!stray_item_fore_color_result.ok,
        "#1063: launch contract should reject stray item-fore-color arguments");
}

void test_parse_launch_arguments_for_highlight_back_color_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--highlight-back-color-object",
        "--highlight-back-color", "65280",
        "--highlight-back-color-target-object-name", "lstOrders",
        "--highlight-back-color-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1064: launch contract should parse highlight-back-color-object requests");
    expect(result.request.highlight_back_color_object,
        "#1064: launch contract should detect --highlight-back-color-object");
    expect(result.request.highlight_back_color_available && result.request.highlight_back_color == 65280,
        "#1064: highlight-back-color-object requests should carry highlight back color values");
    expect(result.request.highlight_back_color_objects.size() == 2U,
        "#1064: highlight-back-color-object requests should collect highlight-back-color target selectors");
    if (result.request.highlight_back_color_objects.size() == 2U) {
        expect(result.request.highlight_back_color_objects[0].object_name == "lstOrders" &&
                result.request.highlight_back_color_objects[0].unique_id.empty(),
            "#1064: highlight-back-color-object requests should parse target object-name selectors");
        expect(result.request.highlight_back_color_objects[1].object_name.empty() &&
                result.request.highlight_back_color_objects[1].unique_id == "two-guid",
            "#1064: highlight-back-color-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_highlight_back_color_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--highlight-back-color-object",
        "--highlight-back-color-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1064: launch contract should reject highlight-back-color-object requests without highlight back color");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--highlight-back-color-object",
        "--highlight-back-color", "65280"
    });
    expect(!missing_targets_result.ok,
        "#1064: launch contract should reject highlight-back-color-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--highlight-back-color-object",
        "--highlight-back-color", "green",
        "--highlight-back-color-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1064: launch contract should reject non-integer highlight-back-color values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--highlight-back-color-object",
        "--highlight-back-color", "-1",
        "--highlight-back-color-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1064: launch contract should reject negative highlight-back-color values before mutation");
}

void test_parse_launch_arguments_rejects_highlight_back_color_object_ambiguity() {
    const auto highlight_item_fore_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--highlight-back-color-object",
        "--item-fore-color-object",
        "--highlight-back-color", "65280",
        "--highlight-back-color-target-unique-id", "one-guid",
        "--item-fore-color", "65280",
        "--item-fore-color-target-unique-id", "one-guid"
    });
    expect(!highlight_item_fore_result.ok,
        "#1064: launch contract should reject simultaneous highlight-back-color-object and item-fore-color-object requests");

    const auto highlight_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--highlight-back-color-object",
        "--clear-property",
        "--property-name", "HighlightBackColor",
        "--highlight-back-color", "65280",
        "--highlight-back-color-target-unique-id", "one-guid"
    });
    expect(!highlight_property_result.ok,
        "#1064: launch contract should reject highlight-back-color-object combined with property commands");

    const auto stray_highlight_back_color_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--highlight-back-color", "65280"
    });
    expect(!stray_highlight_back_color_result.ok,
        "#1064: launch contract should reject stray highlight-back-color arguments");
}

void test_parse_launch_arguments_for_highlight_fore_color_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--highlight-fore-color-object",
        "--highlight-fore-color", "65280",
        "--highlight-fore-color-target-object-name", "lstOrders",
        "--highlight-fore-color-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1065: launch contract should parse highlight-fore-color-object requests");
    expect(result.request.highlight_fore_color_object,
        "#1065: launch contract should detect --highlight-fore-color-object");
    expect(result.request.highlight_fore_color_available && result.request.highlight_fore_color == 65280,
        "#1065: highlight-fore-color-object requests should carry highlight fore color values");
    expect(result.request.highlight_fore_color_objects.size() == 2U,
        "#1065: highlight-fore-color-object requests should collect highlight-fore-color target selectors");
    if (result.request.highlight_fore_color_objects.size() == 2U) {
        expect(result.request.highlight_fore_color_objects[0].object_name == "lstOrders" &&
                result.request.highlight_fore_color_objects[0].unique_id.empty(),
            "#1065: highlight-fore-color-object requests should parse target object-name selectors");
        expect(result.request.highlight_fore_color_objects[1].object_name.empty() &&
                result.request.highlight_fore_color_objects[1].unique_id == "two-guid",
            "#1065: highlight-fore-color-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_highlight_fore_color_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--highlight-fore-color-object",
        "--highlight-fore-color-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1065: launch contract should reject highlight-fore-color-object requests without highlight fore color");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--highlight-fore-color-object",
        "--highlight-fore-color", "65280"
    });
    expect(!missing_targets_result.ok,
        "#1065: launch contract should reject highlight-fore-color-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--highlight-fore-color-object",
        "--highlight-fore-color", "green",
        "--highlight-fore-color-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1065: launch contract should reject non-integer highlight-fore-color values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--highlight-fore-color-object",
        "--highlight-fore-color", "-1",
        "--highlight-fore-color-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1065: launch contract should reject negative highlight-fore-color values before mutation");
}

void test_parse_launch_arguments_rejects_highlight_fore_color_object_ambiguity() {
    const auto highlight_fore_back_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--highlight-fore-color-object",
        "--highlight-back-color-object",
        "--highlight-fore-color", "65280",
        "--highlight-fore-color-target-unique-id", "one-guid",
        "--highlight-back-color", "65280",
        "--highlight-back-color-target-unique-id", "one-guid"
    });
    expect(!highlight_fore_back_result.ok,
        "#1065: launch contract should reject simultaneous highlight-fore-color-object and highlight-back-color-object requests");

    const auto highlight_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--highlight-fore-color-object",
        "--clear-property",
        "--property-name", "HighlightForeColor",
        "--highlight-fore-color", "65280",
        "--highlight-fore-color-target-unique-id", "one-guid"
    });
    expect(!highlight_property_result.ok,
        "#1065: launch contract should reject highlight-fore-color-object combined with property commands");

    const auto stray_highlight_fore_color_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--highlight-fore-color", "65280"
    });
    expect(!stray_highlight_fore_color_result.ok,
        "#1065: launch contract should reject stray highlight-fore-color arguments");
}

void test_parse_launch_arguments_for_back_color_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--back-color-object",
        "--back-color", "65280",
        "--back-color-target-object-name", "lstOrders",
        "--back-color-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1066: launch contract should parse back-color-object requests");
    expect(result.request.back_color_object,
        "#1066: launch contract should detect --back-color-object");
    expect(result.request.back_color_available && result.request.back_color == 65280,
        "#1066: back-color-object requests should carry back color values");
    expect(result.request.back_color_objects.size() == 2U,
        "#1066: back-color-object requests should collect back-color target selectors");
    if (result.request.back_color_objects.size() == 2U) {
        expect(result.request.back_color_objects[0].object_name == "lstOrders" &&
                result.request.back_color_objects[0].unique_id.empty(),
            "#1066: back-color-object requests should parse target object-name selectors");
        expect(result.request.back_color_objects[1].object_name.empty() &&
                result.request.back_color_objects[1].unique_id == "two-guid",
            "#1066: back-color-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_back_color_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--back-color-object",
        "--back-color-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1066: launch contract should reject back-color-object requests without back color");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--back-color-object",
        "--back-color", "65280"
    });
    expect(!missing_targets_result.ok,
        "#1066: launch contract should reject back-color-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--back-color-object",
        "--back-color", "green",
        "--back-color-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1066: launch contract should reject non-integer back-color values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--back-color-object",
        "--back-color", "-1",
        "--back-color-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1066: launch contract should reject negative back-color values before mutation");
}

void test_parse_launch_arguments_rejects_back_color_object_ambiguity() {
    const auto back_highlight_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--back-color-object",
        "--highlight-fore-color-object",
        "--back-color", "65280",
        "--back-color-target-unique-id", "one-guid",
        "--highlight-fore-color", "65280",
        "--highlight-fore-color-target-unique-id", "one-guid"
    });
    expect(!back_highlight_result.ok,
        "#1066: launch contract should reject simultaneous back-color-object and highlight-fore-color-object requests");

    const auto back_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--back-color-object",
        "--clear-property",
        "--property-name", "BackColor",
        "--back-color", "65280",
        "--back-color-target-unique-id", "one-guid"
    });
    expect(!back_property_result.ok,
        "#1066: launch contract should reject back-color-object combined with property commands");

    const auto stray_back_color_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--back-color", "65280"
    });
    expect(!stray_back_color_result.ok,
        "#1066: launch contract should reject stray back-color arguments");
}

void test_parse_launch_arguments_for_fore_color_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--fore-color-object",
        "--fore-color", "65280",
        "--fore-color-target-object-name", "lstOrders",
        "--fore-color-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1067: launch contract should parse fore-color-object requests");
    expect(result.request.fore_color_object,
        "#1067: launch contract should detect --fore-color-object");
    expect(result.request.fore_color_available && result.request.fore_color == 65280,
        "#1067: fore-color-object requests should carry fore color values");
    expect(result.request.fore_color_objects.size() == 2U,
        "#1067: fore-color-object requests should collect fore-color target selectors");
    if (result.request.fore_color_objects.size() == 2U) {
        expect(result.request.fore_color_objects[0].object_name == "lstOrders" &&
                result.request.fore_color_objects[0].unique_id.empty(),
            "#1067: fore-color-object requests should parse target object-name selectors");
        expect(result.request.fore_color_objects[1].object_name.empty() &&
                result.request.fore_color_objects[1].unique_id == "two-guid",
            "#1067: fore-color-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_fore_color_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--fore-color-object",
        "--fore-color-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1067: launch contract should reject fore-color-object requests without fore color");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--fore-color-object",
        "--fore-color", "65280"
    });
    expect(!missing_targets_result.ok,
        "#1067: launch contract should reject fore-color-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--fore-color-object",
        "--fore-color", "green",
        "--fore-color-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1067: launch contract should reject non-integer fore-color values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--fore-color-object",
        "--fore-color", "-1",
        "--fore-color-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1067: launch contract should reject negative fore-color values before mutation");
}

void test_parse_launch_arguments_rejects_fore_color_object_ambiguity() {
    const auto fore_back_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--fore-color-object",
        "--back-color-object",
        "--fore-color", "65280",
        "--fore-color-target-unique-id", "one-guid",
        "--back-color", "65280",
        "--back-color-target-unique-id", "one-guid"
    });
    expect(!fore_back_result.ok,
        "#1067: launch contract should reject simultaneous fore-color-object and back-color-object requests");

    const auto fore_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--fore-color-object",
        "--clear-property",
        "--property-name", "ForeColor",
        "--fore-color", "65280",
        "--fore-color-target-unique-id", "one-guid"
    });
    expect(!fore_property_result.ok,
        "#1067: launch contract should reject fore-color-object combined with property commands");

    const auto stray_fore_color_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--fore-color", "65280"
    });
    expect(!stray_fore_color_result.ok,
        "#1067: launch contract should reject stray fore-color arguments");
}

void test_parse_launch_arguments_for_disabled_back_color_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--disabled-back-color-object",
        "--disabled-back-color", "65280",
        "--disabled-back-color-target-object-name", "lstOrders",
        "--disabled-back-color-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1068: launch contract should parse disabled-back-color-object requests");
    expect(result.request.disabled_back_color_object,
        "#1068: launch contract should detect --disabled-back-color-object");
    expect(result.request.disabled_back_color_available && result.request.disabled_back_color == 65280,
        "#1068: disabled-back-color-object requests should carry disabled back color values");
    expect(result.request.disabled_back_color_objects.size() == 2U,
        "#1068: disabled-back-color-object requests should collect disabled-back-color target selectors");
    if (result.request.disabled_back_color_objects.size() == 2U) {
        expect(result.request.disabled_back_color_objects[0].object_name == "lstOrders" &&
                result.request.disabled_back_color_objects[0].unique_id.empty(),
            "#1068: disabled-back-color-object requests should parse target object-name selectors");
        expect(result.request.disabled_back_color_objects[1].object_name.empty() &&
                result.request.disabled_back_color_objects[1].unique_id == "two-guid",
            "#1068: disabled-back-color-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_disabled_back_color_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--disabled-back-color-object",
        "--disabled-back-color-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1068: launch contract should reject disabled-back-color-object requests without disabled back color");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--disabled-back-color-object",
        "--disabled-back-color", "65280"
    });
    expect(!missing_targets_result.ok,
        "#1068: launch contract should reject disabled-back-color-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--disabled-back-color-object",
        "--disabled-back-color", "green",
        "--disabled-back-color-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1068: launch contract should reject non-integer disabled-back-color values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--disabled-back-color-object",
        "--disabled-back-color", "-1",
        "--disabled-back-color-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1068: launch contract should reject negative disabled-back-color values before mutation");
}

void test_parse_launch_arguments_rejects_disabled_back_color_object_ambiguity() {
    const auto disabled_back_fore_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--disabled-back-color-object",
        "--fore-color-object",
        "--disabled-back-color", "65280",
        "--disabled-back-color-target-unique-id", "one-guid",
        "--fore-color", "65280",
        "--fore-color-target-unique-id", "one-guid"
    });
    expect(!disabled_back_fore_result.ok,
        "#1068: launch contract should reject simultaneous disabled-back-color-object and fore-color-object requests");

    const auto disabled_back_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--disabled-back-color-object",
        "--clear-property",
        "--property-name", "DisabledBackColor",
        "--disabled-back-color", "65280",
        "--disabled-back-color-target-unique-id", "one-guid"
    });
    expect(!disabled_back_property_result.ok,
        "#1068: launch contract should reject disabled-back-color-object combined with property commands");

    const auto stray_disabled_back_color_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--disabled-back-color", "65280"
    });
    expect(!stray_disabled_back_color_result.ok,
        "#1068: launch contract should reject stray disabled-back-color arguments");
}

void test_parse_launch_arguments_for_disabled_fore_color_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--disabled-fore-color-object",
        "--disabled-fore-color", "65280",
        "--disabled-fore-color-target-object-name", "lstOrders",
        "--disabled-fore-color-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1069: launch contract should parse disabled-fore-color-object requests");
    expect(result.request.disabled_fore_color_object,
        "#1069: launch contract should detect --disabled-fore-color-object");
    expect(result.request.disabled_fore_color_available && result.request.disabled_fore_color == 65280,
        "#1069: disabled-fore-color-object requests should carry disabled fore color values");
    expect(result.request.disabled_fore_color_objects.size() == 2U,
        "#1069: disabled-fore-color-object requests should collect disabled-fore-color target selectors");
    if (result.request.disabled_fore_color_objects.size() == 2U) {
        expect(result.request.disabled_fore_color_objects[0].object_name == "lstOrders" &&
                result.request.disabled_fore_color_objects[0].unique_id.empty(),
            "#1069: disabled-fore-color-object requests should parse target object-name selectors");
        expect(result.request.disabled_fore_color_objects[1].object_name.empty() &&
                result.request.disabled_fore_color_objects[1].unique_id == "two-guid",
            "#1069: disabled-fore-color-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_disabled_fore_color_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--disabled-fore-color-object",
        "--disabled-fore-color-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1069: launch contract should reject disabled-fore-color-object requests without disabled fore color");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--disabled-fore-color-object",
        "--disabled-fore-color", "65280"
    });
    expect(!missing_targets_result.ok,
        "#1069: launch contract should reject disabled-fore-color-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--disabled-fore-color-object",
        "--disabled-fore-color", "green",
        "--disabled-fore-color-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1069: launch contract should reject non-integer disabled-fore-color values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--disabled-fore-color-object",
        "--disabled-fore-color", "-1",
        "--disabled-fore-color-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1069: launch contract should reject negative disabled-fore-color values before mutation");
}

void test_parse_launch_arguments_rejects_disabled_fore_color_object_ambiguity() {
    const auto disabled_fore_back_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--disabled-fore-color-object",
        "--disabled-back-color-object",
        "--disabled-fore-color", "65280",
        "--disabled-fore-color-target-unique-id", "one-guid",
        "--disabled-back-color", "65280",
        "--disabled-back-color-target-unique-id", "one-guid"
    });
    expect(!disabled_fore_back_result.ok,
        "#1069: launch contract should reject simultaneous disabled-fore-color-object and disabled-back-color-object requests");

    const auto disabled_fore_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--disabled-fore-color-object",
        "--clear-property",
        "--property-name", "DisabledForeColor",
        "--disabled-fore-color", "65280",
        "--disabled-fore-color-target-unique-id", "one-guid"
    });
    expect(!disabled_fore_property_result.ok,
        "#1069: launch contract should reject disabled-fore-color-object combined with property commands");

    const auto stray_disabled_fore_color_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--disabled-fore-color", "65280"
    });
    expect(!stray_disabled_fore_color_result.ok,
        "#1069: launch contract should reject stray disabled-fore-color arguments");
}

void test_parse_launch_arguments_for_dynamic_back_color_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--dynamic-back-color-object",
        "--dynamic-back-color", "IIF(.T., RGB(1,2,3), 0)",
        "--dynamic-back-color-target-object-name", "lstOrders",
        "--dynamic-back-color-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1070: launch contract should parse dynamic-back-color-object requests");
    expect(result.request.dynamic_back_color_object,
        "#1070: launch contract should detect --dynamic-back-color-object");
    expect(result.request.dynamic_back_color_available &&
            result.request.dynamic_back_color == "IIF(.T., RGB(1,2,3), 0)",
        "#1070: dynamic-back-color-object requests should carry raw expression text");
    expect(result.request.dynamic_back_color_objects.size() == 2U,
        "#1070: dynamic-back-color-object requests should collect dynamic-back-color target selectors");
    if (result.request.dynamic_back_color_objects.size() == 2U) {
        expect(result.request.dynamic_back_color_objects[0].object_name == "lstOrders" &&
                result.request.dynamic_back_color_objects[0].unique_id.empty(),
            "#1070: dynamic-back-color-object requests should parse target object-name selectors");
        expect(result.request.dynamic_back_color_objects[1].object_name.empty() &&
                result.request.dynamic_back_color_objects[1].unique_id == "two-guid",
            "#1070: dynamic-back-color-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_dynamic_back_color_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-back-color-object",
        "--dynamic-back-color-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1070: launch contract should reject dynamic-back-color-object requests without dynamic back color");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-back-color-object",
        "--dynamic-back-color", "RGB(1,2,3)"
    });
    expect(!missing_targets_result.ok,
        "#1070: launch contract should reject dynamic-back-color-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_dynamic_back_color_object_ambiguity() {
    const auto dynamic_disabled_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-back-color-object",
        "--disabled-fore-color-object",
        "--dynamic-back-color", "RGB(1,2,3)",
        "--dynamic-back-color-target-unique-id", "one-guid",
        "--disabled-fore-color", "65280",
        "--disabled-fore-color-target-unique-id", "one-guid"
    });
    expect(!dynamic_disabled_result.ok,
        "#1070: launch contract should reject simultaneous dynamic-back-color-object and disabled-fore-color-object requests");

    const auto dynamic_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-back-color-object",
        "--clear-property",
        "--property-name", "DynamicBackColor",
        "--dynamic-back-color", "RGB(1,2,3)",
        "--dynamic-back-color-target-unique-id", "one-guid"
    });
    expect(!dynamic_property_result.ok,
        "#1070: launch contract should reject dynamic-back-color-object combined with property commands");

    const auto stray_dynamic_back_color_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-back-color", "RGB(1,2,3)"
    });
    expect(!stray_dynamic_back_color_result.ok,
        "#1070: launch contract should reject stray dynamic-back-color arguments");
}

void test_parse_launch_arguments_for_dynamic_fore_color_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--dynamic-fore-color-object",
        "--dynamic-fore-color", "IIF(.T., RGB(7,8,9), 0)",
        "--dynamic-fore-color-target-object-name", "lstOrders",
        "--dynamic-fore-color-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1071: launch contract should parse dynamic-fore-color-object requests");
    expect(result.request.dynamic_fore_color_object,
        "#1071: launch contract should detect --dynamic-fore-color-object");
    expect(result.request.dynamic_fore_color_available &&
            result.request.dynamic_fore_color == "IIF(.T., RGB(7,8,9), 0)",
        "#1071: dynamic-fore-color-object requests should carry raw expression text");
    expect(result.request.dynamic_fore_color_objects.size() == 2U,
        "#1071: dynamic-fore-color-object requests should collect dynamic-fore-color target selectors");
    if (result.request.dynamic_fore_color_objects.size() == 2U) {
        expect(result.request.dynamic_fore_color_objects[0].object_name == "lstOrders" &&
                result.request.dynamic_fore_color_objects[0].unique_id.empty(),
            "#1071: dynamic-fore-color-object requests should parse target object-name selectors");
        expect(result.request.dynamic_fore_color_objects[1].object_name.empty() &&
                result.request.dynamic_fore_color_objects[1].unique_id == "two-guid",
            "#1071: dynamic-fore-color-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_dynamic_fore_color_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-fore-color-object",
        "--dynamic-fore-color-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1071: launch contract should reject dynamic-fore-color-object requests without dynamic fore color");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-fore-color-object",
        "--dynamic-fore-color", "RGB(7,8,9)"
    });
    expect(!missing_targets_result.ok,
        "#1071: launch contract should reject dynamic-fore-color-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_dynamic_fore_color_object_ambiguity() {
    const auto dynamic_fore_back_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-fore-color-object",
        "--dynamic-back-color-object",
        "--dynamic-fore-color", "RGB(7,8,9)",
        "--dynamic-fore-color-target-unique-id", "one-guid",
        "--dynamic-back-color", "RGB(1,2,3)",
        "--dynamic-back-color-target-unique-id", "one-guid"
    });
    expect(!dynamic_fore_back_result.ok,
        "#1071: launch contract should reject simultaneous dynamic-fore-color-object and dynamic-back-color-object requests");

    const auto dynamic_fore_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-fore-color-object",
        "--clear-property",
        "--property-name", "DynamicForeColor",
        "--dynamic-fore-color", "RGB(7,8,9)",
        "--dynamic-fore-color-target-unique-id", "one-guid"
    });
    expect(!dynamic_fore_property_result.ok,
        "#1071: launch contract should reject dynamic-fore-color-object combined with property commands");

    const auto stray_dynamic_fore_color_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-fore-color", "RGB(7,8,9)"
    });
    expect(!stray_dynamic_fore_color_result.ok,
        "#1071: launch contract should reject stray dynamic-fore-color arguments");
}

}  // namespace cf_test_studio_host
