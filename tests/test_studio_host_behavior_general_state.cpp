// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_studio_host_support.h"

namespace cf_test_studio_host {
void test_parse_launch_arguments_for_tab_order_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--tab-order-object",
        "--starting-tab-index", "5",
        "--tab-order-target-object-name", "cmdOne",
        "--tab-order-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1036: launch contract should parse tab-order-object requests");
    expect(result.request.tab_order_object, "#1036: launch contract should detect --tab-order-object");
    expect(result.request.starting_tab_index == 5 && result.request.starting_tab_index_available,
        "#1036: tab-order-object requests should carry starting tab index");
    expect(result.request.tab_order_objects.size() == 2U,
        "#1036: tab-order-object requests should collect tab-order target selectors");
    if (result.request.tab_order_objects.size() == 2U) {
        expect(result.request.tab_order_objects[0].object_name == "cmdOne" &&
                result.request.tab_order_objects[0].unique_id.empty(),
            "#1036: tab-order-object requests should parse target object-name selectors");
        expect(result.request.tab_order_objects[1].object_name.empty() &&
                result.request.tab_order_objects[1].unique_id == "two-guid",
            "#1036: tab-order-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_tab_order_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--tab-order-object",
        "--tab-order-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "launch contract should reject tab-order-object requests without --starting-tab-index");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--tab-order-object",
        "--starting-tab-index", "0"
    });
    expect(!missing_targets_result.ok,
        "#1036: launch contract should reject tab-order-object requests without target selectors");

    const auto negative_start_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--tab-order-object",
        "--starting-tab-index", "-1",
        "--tab-order-target-unique-id", "one-guid"
    });
    expect(!negative_start_result.ok,
        "#1036: launch contract should reject negative tab-order starting indexes");

    const auto invalid_start_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--tab-order-object",
        "--starting-tab-index", "first",
        "--tab-order-target-unique-id", "one-guid"
    });
    expect(!invalid_start_result.ok,
        "#1036: launch contract should reject non-integer tab-order starting indexes");
}

void test_parse_launch_arguments_rejects_tab_order_object_ambiguity() {
    const auto tab_order_nudge_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--tab-order-object",
        "--nudge-object",
        "--starting-tab-index", "1",
        "--tab-order-target-unique-id", "one-guid",
        "--nudge-mode", "horizontal",
        "--delta-hpos", "1",
        "--nudge-target-unique-id", "one-guid"
    });
    expect(!tab_order_nudge_result.ok,
        "#1036: launch contract should reject simultaneous tab-order-object and nudge-object requests");

    const auto tab_order_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--tab-order-object",
        "--clear-property",
        "--property-name", "Caption",
        "--tab-order-target-unique-id", "one-guid"
    });
    expect(!tab_order_property_result.ok,
        "#1036: launch contract should reject tab-order-object combined with property commands");

    const auto stray_tab_order_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--starting-tab-index", "0"
    });
    expect(!stray_tab_order_result.ok,
        "#1036: launch contract should reject stray tab-order arguments");
}

void test_parse_launch_arguments_for_tab_stop_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--tab-stop-object",
        "--tab-stop", "false",
        "--tab-stop-target-object-name", "cmdOne",
        "--tab-stop-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1037: launch contract should parse tab-stop-object requests");
    expect(result.request.tab_stop_object, "#1037: launch contract should detect --tab-stop-object");
    expect(result.request.tab_stop_available && !result.request.tab_stop,
        "#1037: tab-stop-object requests should carry false tab-stop state");
    expect(result.request.tab_stop_objects.size() == 2U,
        "#1037: tab-stop-object requests should collect tab-stop target selectors");
    if (result.request.tab_stop_objects.size() == 2U) {
        expect(result.request.tab_stop_objects[0].object_name == "cmdOne" &&
                result.request.tab_stop_objects[0].unique_id.empty(),
            "#1037: tab-stop-object requests should parse target object-name selectors");
        expect(result.request.tab_stop_objects[1].object_name.empty() &&
                result.request.tab_stop_objects[1].unique_id == "two-guid",
            "#1037: tab-stop-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_tab_stop_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--tab-stop-object",
        "--tab-stop-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1037: launch contract should reject tab-stop-object requests without tab-stop state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--tab-stop-object",
        "--tab-stop", "true"
    });
    expect(!missing_targets_result.ok,
        "#1037: launch contract should reject tab-stop-object requests without target selectors");

    const auto invalid_bool_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--tab-stop-object",
        "--tab-stop", "maybe",
        "--tab-stop-target-unique-id", "one-guid"
    });
    expect(!invalid_bool_result.ok,
        "#1037: launch contract should reject unsupported tab-stop values");
}

void test_parse_launch_arguments_rejects_tab_stop_object_ambiguity() {
    const auto tab_stop_tab_order_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--tab-stop-object",
        "--tab-order-object",
        "--tab-stop", "true",
        "--tab-stop-target-unique-id", "one-guid",
        "--starting-tab-index", "1",
        "--tab-order-target-unique-id", "one-guid"
    });
    expect(!tab_stop_tab_order_result.ok,
        "#1037: launch contract should reject simultaneous tab-stop-object and tab-order-object requests");

    const auto tab_stop_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--tab-stop-object",
        "--clear-property",
        "--property-name", "Caption",
        "--tab-stop", "true",
        "--tab-stop-target-unique-id", "one-guid"
    });
    expect(!tab_stop_property_result.ok,
        "#1037: launch contract should reject tab-stop-object combined with property commands");

    const auto stray_tab_stop_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--tab-stop", "true"
    });
    expect(!stray_tab_stop_result.ok,
        "#1037: launch contract should reject stray tab-stop arguments");
}

void test_parse_launch_arguments_for_visibility_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--visibility-object",
        "--visible", "false",
        "--visibility-target-object-name", "cmdOne",
        "--visibility-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1038: launch contract should parse visibility-object requests");
    expect(result.request.visibility_object, "#1038: launch contract should detect --visibility-object");
    expect(result.request.visible_available && !result.request.visible,
        "#1038: visibility-object requests should carry false visible state");
    expect(result.request.visibility_objects.size() == 2U,
        "#1038: visibility-object requests should collect visibility target selectors");
    if (result.request.visibility_objects.size() == 2U) {
        expect(result.request.visibility_objects[0].object_name == "cmdOne" &&
                result.request.visibility_objects[0].unique_id.empty(),
            "#1038: visibility-object requests should parse target object-name selectors");
        expect(result.request.visibility_objects[1].object_name.empty() &&
                result.request.visibility_objects[1].unique_id == "two-guid",
            "#1038: visibility-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_visibility_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--visibility-object",
        "--visibility-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1038: launch contract should reject visibility-object requests without visible state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--visibility-object",
        "--visible", "true"
    });
    expect(!missing_targets_result.ok,
        "#1038: launch contract should reject visibility-object requests without target selectors");

    const auto invalid_bool_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--visibility-object",
        "--visible", "maybe",
        "--visibility-target-unique-id", "one-guid"
    });
    expect(!invalid_bool_result.ok,
        "#1038: launch contract should reject unsupported visible values");
}

void test_parse_launch_arguments_rejects_visibility_object_ambiguity() {
    const auto visibility_tab_stop_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--visibility-object",
        "--tab-stop-object",
        "--visible", "true",
        "--visibility-target-unique-id", "one-guid",
        "--tab-stop", "true",
        "--tab-stop-target-unique-id", "one-guid"
    });
    expect(!visibility_tab_stop_result.ok,
        "#1038: launch contract should reject simultaneous visibility-object and tab-stop-object requests");

    const auto visibility_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--visibility-object",
        "--clear-property",
        "--property-name", "Caption",
        "--visible", "true",
        "--visibility-target-unique-id", "one-guid"
    });
    expect(!visibility_property_result.ok,
        "#1038: launch contract should reject visibility-object combined with property commands");

    const auto stray_visible_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--visible", "true"
    });
    expect(!stray_visible_result.ok,
        "#1038: launch contract should reject stray visible arguments");
}

void test_parse_launch_arguments_for_enabled_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--enabled-object",
        "--enabled", "false",
        "--enabled-target-object-name", "cmdOne",
        "--enabled-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1039: launch contract should parse enabled-object requests");
    expect(result.request.enabled_object, "#1039: launch contract should detect --enabled-object");
    expect(result.request.enabled_available && !result.request.enabled,
        "#1039: enabled-object requests should carry false enabled state");
    expect(result.request.enabled_objects.size() == 2U,
        "#1039: enabled-object requests should collect enabled target selectors");
    if (result.request.enabled_objects.size() == 2U) {
        expect(result.request.enabled_objects[0].object_name == "cmdOne" &&
                result.request.enabled_objects[0].unique_id.empty(),
            "#1039: enabled-object requests should parse target object-name selectors");
        expect(result.request.enabled_objects[1].object_name.empty() &&
                result.request.enabled_objects[1].unique_id == "two-guid",
            "#1039: enabled-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_enabled_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--enabled-object",
        "--enabled-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1039: launch contract should reject enabled-object requests without enabled state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--enabled-object",
        "--enabled", "true"
    });
    expect(!missing_targets_result.ok,
        "#1039: launch contract should reject enabled-object requests without target selectors");

    const auto invalid_bool_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--enabled-object",
        "--enabled", "maybe",
        "--enabled-target-unique-id", "one-guid"
    });
    expect(!invalid_bool_result.ok,
        "#1039: launch contract should reject unsupported enabled values");
}

void test_parse_launch_arguments_rejects_enabled_object_ambiguity() {
    const auto enabled_visibility_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--enabled-object",
        "--visibility-object",
        "--enabled", "true",
        "--enabled-target-unique-id", "one-guid",
        "--visible", "true",
        "--visibility-target-unique-id", "one-guid"
    });
    expect(!enabled_visibility_result.ok,
        "#1039: launch contract should reject simultaneous enabled-object and visibility-object requests");

    const auto enabled_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--enabled-object",
        "--clear-property",
        "--property-name", "Caption",
        "--enabled", "true",
        "--enabled-target-unique-id", "one-guid"
    });
    expect(!enabled_property_result.ok,
        "#1039: launch contract should reject enabled-object combined with property commands");

    const auto stray_enabled_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--enabled", "true"
    });
    expect(!stray_enabled_result.ok,
        "#1039: launch contract should reject stray enabled arguments");
}

void test_parse_launch_arguments_for_read_only_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--read-only-object",
        "--object-read-only", "true",
        "--read-only-target-object-name", "txtOne",
        "--read-only-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1040: launch contract should parse read-only-object requests");
    expect(result.request.read_only_object, "#1040: launch contract should detect --read-only-object");
    expect(result.request.object_read_only_available && result.request.object_read_only,
        "#1040: read-only-object requests should carry true read-only state");
    expect(result.request.read_only_objects.size() == 2U,
        "#1040: read-only-object requests should collect read-only target selectors");
    if (result.request.read_only_objects.size() == 2U) {
        expect(result.request.read_only_objects[0].object_name == "txtOne" &&
                result.request.read_only_objects[0].unique_id.empty(),
            "#1040: read-only-object requests should parse target object-name selectors");
        expect(result.request.read_only_objects[1].object_name.empty() &&
                result.request.read_only_objects[1].unique_id == "two-guid",
            "#1040: read-only-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_read_only_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--read-only-object",
        "--read-only-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1040: launch contract should reject read-only-object requests without object read-only state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--read-only-object",
        "--object-read-only", "true"
    });
    expect(!missing_targets_result.ok,
        "#1040: launch contract should reject read-only-object requests without target selectors");

    const auto invalid_bool_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--read-only-object",
        "--object-read-only", "maybe",
        "--read-only-target-unique-id", "one-guid"
    });
    expect(!invalid_bool_result.ok,
        "#1040: launch contract should reject unsupported object read-only values");
}

void test_parse_launch_arguments_rejects_read_only_object_ambiguity() {
    const auto read_only_enabled_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--read-only-object",
        "--enabled-object",
        "--object-read-only", "true",
        "--read-only-target-unique-id", "one-guid",
        "--enabled", "true",
        "--enabled-target-unique-id", "one-guid"
    });
    expect(!read_only_enabled_result.ok,
        "#1040: launch contract should reject simultaneous read-only-object and enabled-object requests");

    const auto read_only_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--read-only-object",
        "--clear-property",
        "--property-name", "Caption",
        "--object-read-only", "true",
        "--read-only-target-unique-id", "one-guid"
    });
    expect(!read_only_property_result.ok,
        "#1040: launch contract should reject read-only-object combined with property commands");

    const auto stray_read_only_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--object-read-only", "true"
    });
    expect(!stray_read_only_result.ok,
        "#1040: launch contract should reject stray object read-only arguments");
}

void test_parse_launch_arguments_for_locked_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--locked-object",
        "--locked", "true",
        "--locked-target-object-name", "txtOne",
        "--locked-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1041: launch contract should parse locked-object requests");
    expect(result.request.locked_object, "#1041: launch contract should detect --locked-object");
    expect(result.request.locked_available && result.request.locked,
        "#1041: locked-object requests should carry true locked state");
    expect(result.request.locked_objects.size() == 2U,
        "#1041: locked-object requests should collect locked target selectors");
    if (result.request.locked_objects.size() == 2U) {
        expect(result.request.locked_objects[0].object_name == "txtOne" &&
                result.request.locked_objects[0].unique_id.empty(),
            "#1041: locked-object requests should parse target object-name selectors");
        expect(result.request.locked_objects[1].object_name.empty() &&
                result.request.locked_objects[1].unique_id == "two-guid",
            "#1041: locked-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_locked_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--locked-object",
        "--locked-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1041: launch contract should reject locked-object requests without locked state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--locked-object",
        "--locked", "true"
    });
    expect(!missing_targets_result.ok,
        "#1041: launch contract should reject locked-object requests without target selectors");

    const auto invalid_bool_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--locked-object",
        "--locked", "maybe",
        "--locked-target-unique-id", "one-guid"
    });
    expect(!invalid_bool_result.ok,
        "#1041: launch contract should reject unsupported locked values");
}

void test_parse_launch_arguments_rejects_locked_object_ambiguity() {
    const auto locked_read_only_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--locked-object",
        "--read-only-object",
        "--locked", "true",
        "--locked-target-unique-id", "one-guid",
        "--object-read-only", "true",
        "--read-only-target-unique-id", "one-guid"
    });
    expect(!locked_read_only_result.ok,
        "#1041: launch contract should reject simultaneous locked-object and read-only-object requests");

    const auto locked_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--locked-object",
        "--clear-property",
        "--property-name", "Caption",
        "--locked", "true",
        "--locked-target-unique-id", "one-guid"
    });
    expect(!locked_property_result.ok,
        "#1041: launch contract should reject locked-object combined with property commands");

    const auto stray_locked_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--locked", "true"
    });
    expect(!stray_locked_result.ok,
        "#1041: launch contract should reject stray locked arguments");
}

void test_parse_launch_arguments_for_tab_orientation_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--tab-orientation-object",
        "--tab-orientation", "9",
        "--tab-orientation-target-object-name", "cmdSave",
        "--tab-orientation-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1139: launch contract should parse tab-orientation-object requests");
    expect(result.request.tab_orientation_object,
        "#1139: launch contract should detect --tab-orientation-object");
    expect(result.request.tab_orientation_available && result.request.tab_orientation == 9,
        "#1139: tab-orientation-object requests should carry tab orientation");
    expect(result.request.tab_orientation_objects.size() == 2U,
        "#1139: tab-orientation-object requests should collect tab orientation target selectors");
    if (result.request.tab_orientation_objects.size() == 2U) {
        expect(result.request.tab_orientation_objects[0].object_name == "cmdSave" &&
                result.request.tab_orientation_objects[0].unique_id.empty(),
            "#1139: tab-orientation-object requests should parse target object-name selectors");
        expect(result.request.tab_orientation_objects[1].object_name.empty() &&
                result.request.tab_orientation_objects[1].unique_id == "two-guid",
            "#1139: tab-orientation-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_tab_orientation_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--tab-orientation-object",
        "--tab-orientation-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1139: launch contract should reject tab-orientation-object requests without tab orientation");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--tab-orientation-object",
        "--tab-orientation", "9"
    });
    expect(!missing_targets_result.ok,
        "#1139: launch contract should reject tab-orientation-object requests without target selectors");

    const auto non_integer_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--tab-orientation-object",
        "--tab-orientation", "east",
        "--tab-orientation-target-unique-id", "one-guid"
    });
    expect(!non_integer_result.ok,
        "#1139: launch contract should reject non-integer tab orientation values");

    const auto negative_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--tab-orientation-object",
        "--tab-orientation", "-1",
        "--tab-orientation-target-unique-id", "one-guid"
    });
    expect(!negative_result.ok,
        "#1139: launch contract should reject negative tab orientation values");
}

void test_parse_launch_arguments_rejects_tab_orientation_object_ambiguity() {
    const auto tab_orientation_locked_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--tab-orientation-object",
        "--locked-object",
        "--tab-orientation", "9",
        "--tab-orientation-target-unique-id", "one-guid",
        "--locked", "true",
        "--locked-target-unique-id", "one-guid"
    });
    expect(!tab_orientation_locked_result.ok,
        "#1139: launch contract should reject simultaneous tab-orientation-object and locked-object requests");

    const auto tab_orientation_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--tab-orientation-object",
        "--clear-property",
        "--property-name", "ToolTipText",
        "--tab-orientation", "9",
        "--tab-orientation-target-unique-id", "one-guid"
    });
    expect(!tab_orientation_property_result.ok,
        "#1139: launch contract should reject tab-orientation-object combined with property commands");

    const auto stray_tab_orientation_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--tab-orientation", "9"
    });
    expect(!stray_tab_orientation_result.ok,
        "#1139: launch contract should reject stray tab-orientation arguments");
}

void test_parse_launch_arguments_for_display_orientation_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--display-orientation-object",
        "--display-orientation", "9",
        "--display-orientation-target-object-name", "cmdSave",
        "--display-orientation-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1140: launch contract should parse display-orientation-object requests");
    expect(result.request.display_orientation_object,
        "#1140: launch contract should detect --display-orientation-object");
    expect(result.request.display_orientation_available && result.request.display_orientation == 9,
        "#1140: display-orientation-object requests should carry display orientation");
    expect(result.request.display_orientation_objects.size() == 2U,
        "#1140: display-orientation-object requests should collect display orientation target selectors");
    if (result.request.display_orientation_objects.size() == 2U) {
        expect(result.request.display_orientation_objects[0].object_name == "cmdSave" &&
                result.request.display_orientation_objects[0].unique_id.empty(),
            "#1140: display-orientation-object requests should parse target object-name selectors");
        expect(result.request.display_orientation_objects[1].object_name.empty() &&
                result.request.display_orientation_objects[1].unique_id == "two-guid",
            "#1140: display-orientation-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_display_orientation_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--display-orientation-object",
        "--display-orientation-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1140: launch contract should reject display-orientation-object requests without display orientation");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--display-orientation-object",
        "--display-orientation", "9"
    });
    expect(!missing_targets_result.ok,
        "#1140: launch contract should reject display-orientation-object requests without target selectors");

    const auto non_integer_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--display-orientation-object",
        "--display-orientation", "east",
        "--display-orientation-target-unique-id", "one-guid"
    });
    expect(!non_integer_result.ok,
        "#1140: launch contract should reject non-integer display orientation values");

    const auto negative_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--display-orientation-object",
        "--display-orientation", "-1",
        "--display-orientation-target-unique-id", "one-guid"
    });
    expect(!negative_result.ok,
        "#1140: launch contract should reject negative display orientation values");
}

void test_parse_launch_arguments_rejects_display_orientation_object_ambiguity() {
    const auto display_orientation_locked_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--display-orientation-object",
        "--locked-object",
        "--display-orientation", "9",
        "--display-orientation-target-unique-id", "one-guid",
        "--locked", "true",
        "--locked-target-unique-id", "one-guid"
    });
    expect(!display_orientation_locked_result.ok,
        "#1140: launch contract should reject simultaneous display-orientation-object and locked-object requests");

    const auto display_orientation_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--display-orientation-object",
        "--clear-property",
        "--property-name", "ToolTipText",
        "--display-orientation", "9",
        "--display-orientation-target-unique-id", "one-guid"
    });
    expect(!display_orientation_property_result.ok,
        "#1140: launch contract should reject display-orientation-object combined with property commands");

    const auto stray_display_orientation_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--display-orientation", "9"
    });
    expect(!stray_display_orientation_result.ok,
        "#1140: launch contract should reject stray display-orientation arguments");
}

void test_parse_launch_arguments_for_help_context_id_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--help-context-id-object",
        "--help-context-id", "900",
        "--help-context-id-target-object-name", "cmdSave",
        "--help-context-id-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1141: launch contract should parse help-context-id-object requests");
    expect(result.request.help_context_id_object,
        "#1141: launch contract should detect --help-context-id-object");
    expect(result.request.help_context_id_available && result.request.help_context_id == 900,
        "#1141: help-context-id-object requests should carry help context ID");
    expect(result.request.help_context_id_objects.size() == 2U,
        "#1141: help-context-id-object requests should collect help context ID target selectors");
    if (result.request.help_context_id_objects.size() == 2U) {
        expect(result.request.help_context_id_objects[0].object_name == "cmdSave" &&
                result.request.help_context_id_objects[0].unique_id.empty(),
            "#1141: help-context-id-object requests should parse target object-name selectors");
        expect(result.request.help_context_id_objects[1].object_name.empty() &&
                result.request.help_context_id_objects[1].unique_id == "two-guid",
            "#1141: help-context-id-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_help_context_id_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--help-context-id-object",
        "--help-context-id-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1141: launch contract should reject help-context-id-object requests without help context ID");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--help-context-id-object",
        "--help-context-id", "900"
    });
    expect(!missing_targets_result.ok,
        "#1141: launch contract should reject help-context-id-object requests without target selectors");

    const auto non_integer_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--help-context-id-object",
        "--help-context-id", "topic",
        "--help-context-id-target-unique-id", "one-guid"
    });
    expect(!non_integer_result.ok,
        "#1141: launch contract should reject non-integer help context ID values");

    const auto negative_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--help-context-id-object",
        "--help-context-id", "-1",
        "--help-context-id-target-unique-id", "one-guid"
    });
    expect(!negative_result.ok,
        "#1141: launch contract should reject negative help context ID values");
}

void test_parse_launch_arguments_rejects_help_context_id_object_ambiguity() {
    const auto help_context_id_locked_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--help-context-id-object",
        "--locked-object",
        "--help-context-id", "900",
        "--help-context-id-target-unique-id", "one-guid",
        "--locked", "true",
        "--locked-target-unique-id", "one-guid"
    });
    expect(!help_context_id_locked_result.ok,
        "#1141: launch contract should reject simultaneous help-context-id-object and locked-object requests");

    const auto help_context_id_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--help-context-id-object",
        "--clear-property",
        "--property-name", "ToolTipText",
        "--help-context-id", "900",
        "--help-context-id-target-unique-id", "one-guid"
    });
    expect(!help_context_id_property_result.ok,
        "#1141: launch contract should reject help-context-id-object combined with property commands");

    const auto stray_help_context_id_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--help-context-id", "900"
    });
    expect(!stray_help_context_id_result.ok,
        "#1141: launch contract should reject stray help-context-id arguments");
}

void test_parse_launch_arguments_for_tooltip_text_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--tooltip-text-object",
        "--tooltip-text", "Save this customer",
        "--tooltip-text-target-object-name", "cmdSave",
        "--tooltip-text-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1043: launch contract should parse tooltip-text-object requests");
    expect(result.request.tooltip_text_object, "#1043: launch contract should detect --tooltip-text-object");
    expect(result.request.tooltip_text_available && result.request.tooltip_text == "Save this customer",
        "#1043: tooltip-text-object requests should carry tooltip text");
    expect(result.request.tooltip_text_objects.size() == 2U,
        "#1043: tooltip-text-object requests should collect tooltip text target selectors");
    if (result.request.tooltip_text_objects.size() == 2U) {
        expect(result.request.tooltip_text_objects[0].object_name == "cmdSave" &&
                result.request.tooltip_text_objects[0].unique_id.empty(),
            "#1043: tooltip-text-object requests should parse target object-name selectors");
        expect(result.request.tooltip_text_objects[1].object_name.empty() &&
                result.request.tooltip_text_objects[1].unique_id == "two-guid",
            "#1043: tooltip-text-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_tooltip_text_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--tooltip-text-object",
        "--tooltip-text-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1043: launch contract should reject tooltip-text-object requests without tooltip text");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--tooltip-text-object",
        "--tooltip-text", "Save"
    });
    expect(!missing_targets_result.ok,
        "#1043: launch contract should reject tooltip-text-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_tooltip_text_object_ambiguity() {
    const auto tooltip_caption_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--tooltip-text-object",
        "--caption-object",
        "--tooltip-text", "Save",
        "--tooltip-text-target-unique-id", "one-guid",
        "--caption", "Save",
        "--caption-target-unique-id", "one-guid"
    });
    expect(!tooltip_caption_result.ok,
        "#1043: launch contract should reject simultaneous tooltip-text-object and caption-object requests");

    const auto tooltip_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--tooltip-text-object",
        "--clear-property",
        "--property-name", "ToolTipText",
        "--tooltip-text", "Save",
        "--tooltip-text-target-unique-id", "one-guid"
    });
    expect(!tooltip_property_result.ok,
        "#1043: launch contract should reject tooltip-text-object combined with property commands");

    const auto stray_tooltip_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--tooltip-text", "Save"
    });
    expect(!stray_tooltip_result.ok,
        "#1043: launch contract should reject stray tooltip text arguments");
}

void test_parse_launch_arguments_for_format_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--format-object",
        "--format", "999,999.99",
        "--format-target-object-name", "txtAmount",
        "--format-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1047: launch contract should parse format-object requests");
    expect(result.request.format_object, "#1047: launch contract should detect --format-object");
    expect(result.request.format_available && result.request.format == "999,999.99",
        "#1047: format-object requests should carry format text");
    expect(result.request.format_objects.size() == 2U,
        "#1047: format-object requests should collect format target selectors");
    if (result.request.format_objects.size() == 2U) {
        expect(result.request.format_objects[0].object_name == "txtAmount" &&
                result.request.format_objects[0].unique_id.empty(),
            "#1047: format-object requests should parse target object-name selectors");
        expect(result.request.format_objects[1].object_name.empty() &&
                result.request.format_objects[1].unique_id == "two-guid",
            "#1047: format-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_format_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--format-object",
        "--format-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1047: launch contract should reject format-object requests without format text");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--format-object",
        "--format", "99999"
    });
    expect(!missing_targets_result.ok,
        "#1047: launch contract should reject format-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_format_object_ambiguity() {
    const auto format_input_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--format-object",
        "--input-mask-object",
        "--format", "99999",
        "--format-target-unique-id", "one-guid",
        "--input-mask", "99999",
        "--input-mask-target-unique-id", "one-guid"
    });
    expect(!format_input_result.ok,
        "#1047: launch contract should reject simultaneous format-object and input-mask-object requests");

    const auto format_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--format-object",
        "--clear-property",
        "--property-name", "Format",
        "--format", "99999",
        "--format-target-unique-id", "one-guid"
    });
    expect(!format_property_result.ok,
        "#1047: launch contract should reject format-object combined with property commands");

    const auto stray_format_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--format", "99999"
    });
    expect(!stray_format_result.ok,
        "#1047: launch contract should reject stray format arguments");
}

void test_parse_launch_arguments_for_allow_output_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--allow-output-object",
        "--allow-output", "false",
        "--allow-output-target-object-name", "frmCustomer",
        "--allow-output-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1075: launch contract should parse allow-output-object requests");
    expect(result.request.allow_output_object,
        "#1075: launch contract should detect --allow-output-object");
    expect(result.request.allow_output_available && !result.request.allow_output,
        "#1075: allow-output-object requests should carry allow output state");
    expect(result.request.allow_output_objects.size() == 2U,
        "#1075: allow-output-object requests should collect allow-output target selectors");
    if (result.request.allow_output_objects.size() == 2U) {
        expect(result.request.allow_output_objects[0].object_name == "frmCustomer" &&
                result.request.allow_output_objects[0].unique_id.empty(),
            "#1075: allow-output-object requests should parse target object-name selectors");
        expect(result.request.allow_output_objects[1].object_name.empty() &&
                result.request.allow_output_objects[1].unique_id == "two-guid",
            "#1075: allow-output-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_allow_output_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--allow-output-object",
        "--allow-output-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1075: launch contract should reject allow-output-object requests without allow output state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--allow-output-object",
        "--allow-output", "false"
    });
    expect(!missing_targets_result.ok,
        "#1075: launch contract should reject allow-output-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--allow-output-object",
        "--allow-output", "sometimes",
        "--allow-output-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1075: launch contract should reject invalid allow-output boolean values");
}

void test_parse_launch_arguments_rejects_allow_output_object_ambiguity() {
    const auto allow_output_control_box_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--allow-output-object",
        "--control-box-object",
        "--allow-output", "false",
        "--allow-output-target-unique-id", "one-guid",
        "--control-box", "false",
        "--control-box-target-unique-id", "one-guid"
    });
    expect(!allow_output_control_box_result.ok,
        "#1075: launch contract should reject simultaneous allow-output-object and control-box-object requests");

    const auto allow_output_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--allow-output-object",
        "--clear-property",
        "--property-name", "AllowOutput",
        "--allow-output", "false",
        "--allow-output-target-unique-id", "one-guid"
    });
    expect(!allow_output_property_result.ok,
        "#1075: launch contract should reject allow-output-object combined with property commands");

    const auto stray_allow_output_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--allow-output", "false"
    });
    expect(!stray_allow_output_result.ok,
        "#1075: launch contract should reject stray allow-output arguments");
}

void test_parse_launch_arguments_for_delete_mark_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--delete-mark-object",
        "--delete-mark", "false",
        "--delete-mark-target-object-name", "frmCustomer",
        "--delete-mark-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1087: launch contract should parse delete-mark-object requests");
    expect(result.request.delete_mark_object,
        "#1087: launch contract should detect --delete-mark-object");
    expect(result.request.delete_mark_available && !result.request.delete_mark,
        "#1087: delete-mark-object requests should carry delete mark state");
    expect(result.request.delete_mark_objects.size() == 2U,
        "#1087: delete-mark-object requests should collect delete_mark target selectors");
    if (result.request.delete_mark_objects.size() == 2U) {
        expect(result.request.delete_mark_objects[0].object_name == "frmCustomer" &&
                result.request.delete_mark_objects[0].unique_id.empty(),
            "#1087: delete-mark-object requests should parse target object-name selectors");
        expect(result.request.delete_mark_objects[1].object_name.empty() &&
                result.request.delete_mark_objects[1].unique_id == "two-guid",
            "#1087: delete-mark-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_delete_mark_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--delete-mark-object",
        "--delete-mark-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1087: launch contract should reject delete-mark-object requests without delete mark state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--delete-mark-object",
        "--delete-mark", "false"
    });
    expect(!missing_targets_result.ok,
        "#1087: launch contract should reject delete-mark-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--delete-mark-object",
        "--delete-mark", "sometimes",
        "--delete-mark-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1087: launch contract should reject invalid delete-mark boolean values");
}

void test_parse_launch_arguments_rejects_delete_mark_object_ambiguity() {
    const auto delete_mark_auto_size_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--delete-mark-object",
        "--auto-size-object",
        "--delete-mark", "false",
        "--delete-mark-target-unique-id", "one-guid",
        "--auto-size", "false",
        "--auto-size-target-unique-id", "one-guid"
    });
    expect(!delete_mark_auto_size_result.ok,
        "#1087: launch contract should reject simultaneous delete-mark-object and auto-size-object requests");

    const auto delete_mark_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--delete-mark-object",
        "--clear-property",
        "--property-name", "Dockable",
        "--delete-mark", "false",
        "--delete-mark-target-unique-id", "one-guid"
    });
    expect(!delete_mark_property_result.ok,
        "#1087: launch contract should reject delete-mark-object combined with property commands");

    const auto stray_delete_mark_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--delete-mark", "false"
    });
    expect(!stray_delete_mark_result.ok,
        "#1087: launch contract should reject stray delete-mark arguments");
}

void test_parse_launch_arguments_for_add_line_feeds_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--add-line-feeds-object",
        "--add-line-feeds", "false",
        "--add-line-feeds-target-object-name", "frmCustomer",
        "--add-line-feeds-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1095: launch contract should parse add-line-feeds-object requests");
    expect(result.request.add_line_feeds_object,
        "#1095: launch contract should detect --add-line-feeds-object");
    expect(result.request.add_line_feeds_available && !result.request.add_line_feeds,
        "#1095: add-line-feeds-object requests should carry add line feeds state");
    expect(result.request.add_line_feeds_objects.size() == 2U,
        "#1095: add-line-feeds-object requests should collect add_line_feeds target selectors");
    if (result.request.add_line_feeds_objects.size() == 2U) {
        expect(result.request.add_line_feeds_objects[0].object_name == "frmCustomer" &&
                result.request.add_line_feeds_objects[0].unique_id.empty(),
            "#1095: add-line-feeds-object requests should parse target object-name selectors");
        expect(result.request.add_line_feeds_objects[1].object_name.empty() &&
                result.request.add_line_feeds_objects[1].unique_id == "two-guid",
            "#1095: add-line-feeds-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_add_line_feeds_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--add-line-feeds-object",
        "--add-line-feeds-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1095: launch contract should reject add-line-feeds-object requests without add line feeds state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--add-line-feeds-object",
        "--add-line-feeds", "false"
    });
    expect(!missing_targets_result.ok,
        "#1095: launch contract should reject add-line-feeds-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--add-line-feeds-object",
        "--add-line-feeds", "sometimes",
        "--add-line-feeds-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1095: launch contract should reject invalid add-line-feeds boolean values");
}

void test_parse_launch_arguments_rejects_add_line_feeds_object_ambiguity() {
    const auto add_line_feeds_auto_size_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--add-line-feeds-object",
        "--auto-size-object",
        "--add-line-feeds", "false",
        "--add-line-feeds-target-unique-id", "one-guid",
        "--auto-size", "false",
        "--auto-size-target-unique-id", "one-guid"
    });
    expect(!add_line_feeds_auto_size_result.ok,
        "#1095: launch contract should reject simultaneous add-line-feeds-object and auto-size-object requests");

    const auto add_line_feeds_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--add-line-feeds-object",
        "--clear-property",
        "--property-name", "Dockable",
        "--add-line-feeds", "false",
        "--add-line-feeds-target-unique-id", "one-guid"
    });
    expect(!add_line_feeds_property_result.ok,
        "#1095: launch contract should reject add-line-feeds-object combined with property commands");

    const auto stray_add_line_feeds_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--add-line-feeds", "false"
    });
    expect(!stray_add_line_feeds_result.ok,
        "#1095: launch contract should reject stray add-line-feeds arguments");
}

}  // namespace cf_test_studio_host
