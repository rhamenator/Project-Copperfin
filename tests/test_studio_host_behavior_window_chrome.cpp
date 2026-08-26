// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_studio_host_support.h"

namespace cf_test_studio_host {
void test_parse_launch_arguments_for_caption_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--caption-object",
        "--caption", "Save Customer",
        "--caption-target-object-name", "cmdSave",
        "--caption-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1042: launch contract should parse caption-object requests");
    expect(result.request.caption_object, "#1042: launch contract should detect --caption-object");
    expect(result.request.caption_available && result.request.caption == "Save Customer",
        "#1042: caption-object requests should carry caption text");
    expect(result.request.caption_objects.size() == 2U,
        "#1042: caption-object requests should collect caption target selectors");
    if (result.request.caption_objects.size() == 2U) {
        expect(result.request.caption_objects[0].object_name == "cmdSave" &&
                result.request.caption_objects[0].unique_id.empty(),
            "#1042: caption-object requests should parse target object-name selectors");
        expect(result.request.caption_objects[1].object_name.empty() &&
                result.request.caption_objects[1].unique_id == "two-guid",
            "#1042: caption-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_caption_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--caption-object",
        "--caption-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1042: launch contract should reject caption-object requests without caption text");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--caption-object",
        "--caption", "Save"
    });
    expect(!missing_targets_result.ok,
        "#1042: launch contract should reject caption-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_caption_object_ambiguity() {
    const auto caption_locked_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--caption-object",
        "--locked-object",
        "--caption", "Save",
        "--caption-target-unique-id", "one-guid",
        "--locked", "true",
        "--locked-target-unique-id", "one-guid"
    });
    expect(!caption_locked_result.ok,
        "#1042: launch contract should reject simultaneous caption-object and locked-object requests");

    const auto caption_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--caption-object",
        "--clear-property",
        "--property-name", "Caption",
        "--caption", "Save",
        "--caption-target-unique-id", "one-guid"
    });
    expect(!caption_property_result.ok,
        "#1042: launch contract should reject caption-object combined with property commands");

    const auto stray_caption_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--caption", "Save"
    });
    expect(!stray_caption_result.ok,
        "#1042: launch contract should reject stray caption arguments");
}

void test_parse_launch_arguments_for_whats_this_help_id_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--whats-this-help-id-object",
        "--whats-this-help-id", "900",
        "--whats-this-help-id-target-object-name", "cmdSave",
        "--whats-this-help-id-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1142: launch contract should parse whats-this-help-id-object requests");
    expect(result.request.whats_this_help_id_object,
        "#1142: launch contract should detect --whats-this-help-id-object");
    expect(result.request.whats_this_help_id_available && result.request.whats_this_help_id == 900,
        "#1142: whats-this-help-id-object requests should carry WhatsThis help ID");
    expect(result.request.whats_this_help_id_objects.size() == 2U,
        "#1142: whats-this-help-id-object requests should collect WhatsThis help ID target selectors");
    if (result.request.whats_this_help_id_objects.size() == 2U) {
        expect(result.request.whats_this_help_id_objects[0].object_name == "cmdSave" &&
                result.request.whats_this_help_id_objects[0].unique_id.empty(),
            "#1142: whats-this-help-id-object requests should parse target object-name selectors");
        expect(result.request.whats_this_help_id_objects[1].object_name.empty() &&
                result.request.whats_this_help_id_objects[1].unique_id == "two-guid",
            "#1142: whats-this-help-id-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_whats_this_help_id_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--whats-this-help-id-object",
        "--whats-this-help-id-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1142: launch contract should reject whats-this-help-id-object requests without WhatsThis help ID");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--whats-this-help-id-object",
        "--whats-this-help-id", "900"
    });
    expect(!missing_targets_result.ok,
        "#1142: launch contract should reject whats-this-help-id-object requests without target selectors");

    const auto non_integer_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--whats-this-help-id-object",
        "--whats-this-help-id", "topic",
        "--whats-this-help-id-target-unique-id", "one-guid"
    });
    expect(!non_integer_result.ok,
        "#1142: launch contract should reject non-integer WhatsThis help ID values");

    const auto negative_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--whats-this-help-id-object",
        "--whats-this-help-id", "-1",
        "--whats-this-help-id-target-unique-id", "one-guid"
    });
    expect(!negative_result.ok,
        "#1142: launch contract should reject negative WhatsThis help ID values");
}

void test_parse_launch_arguments_rejects_whats_this_help_id_object_ambiguity() {
    const auto whats_this_help_id_locked_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--whats-this-help-id-object",
        "--locked-object",
        "--whats-this-help-id", "900",
        "--whats-this-help-id-target-unique-id", "one-guid",
        "--locked", "true",
        "--locked-target-unique-id", "one-guid"
    });
    expect(!whats_this_help_id_locked_result.ok,
        "#1142: launch contract should reject simultaneous whats-this-help-id-object and locked-object requests");

    const auto whats_this_help_id_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--whats-this-help-id-object",
        "--clear-property",
        "--property-name", "ToolTipText",
        "--whats-this-help-id", "900",
        "--whats-this-help-id-target-unique-id", "one-guid"
    });
    expect(!whats_this_help_id_property_result.ok,
        "#1142: launch contract should reject whats-this-help-id-object combined with property commands");

    const auto stray_whats_this_help_id_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--whats-this-help-id", "900"
    });
    expect(!stray_whats_this_help_id_result.ok,
        "#1142: launch contract should reject stray whats-this-help-id arguments");
}

void test_parse_launch_arguments_for_whats_this_help_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--whats-this-help-object",
        "--whats-this-help", ".T.",
        "--whats-this-help-target-object-name", "cmdSave",
        "--whats-this-help-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1143: launch contract should parse whats-this-help-object requests");
    expect(result.request.whats_this_help_object,
        "#1143: launch contract should detect --whats-this-help-object");
    expect(result.request.whats_this_help_available && result.request.whats_this_help,
        "#1143: whats-this-help-object requests should carry WhatsThis help state");
    expect(result.request.whats_this_help_objects.size() == 2U,
        "#1143: whats-this-help-object requests should collect WhatsThis help target selectors");
    if (result.request.whats_this_help_objects.size() == 2U) {
        expect(result.request.whats_this_help_objects[0].object_name == "cmdSave" &&
                result.request.whats_this_help_objects[0].unique_id.empty(),
            "#1143: whats-this-help-object requests should parse target object-name selectors");
        expect(result.request.whats_this_help_objects[1].object_name.empty() &&
                result.request.whats_this_help_objects[1].unique_id == "two-guid",
            "#1143: whats-this-help-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_whats_this_help_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--whats-this-help-object",
        "--whats-this-help-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1143: launch contract should reject whats-this-help-object requests without WhatsThis help value");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--whats-this-help-object",
        "--whats-this-help", "true"
    });
    expect(!missing_targets_result.ok,
        "#1143: launch contract should reject whats-this-help-object requests without target selectors");

    const auto invalid_logical_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--whats-this-help-object",
        "--whats-this-help", "maybe",
        "--whats-this-help-target-unique-id", "one-guid"
    });
    expect(!invalid_logical_result.ok,
        "#1143: launch contract should reject invalid WhatsThis help logical values");
}

void test_parse_launch_arguments_rejects_whats_this_help_object_ambiguity() {
    const auto whats_this_help_locked_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--whats-this-help-object",
        "--locked-object",
        "--whats-this-help", "true",
        "--whats-this-help-target-unique-id", "one-guid",
        "--locked", "true",
        "--locked-target-unique-id", "one-guid"
    });
    expect(!whats_this_help_locked_result.ok,
        "#1143: launch contract should reject simultaneous whats-this-help-object and locked-object requests");

    const auto whats_this_help_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--whats-this-help-object",
        "--clear-property",
        "--property-name", "ToolTipText",
        "--whats-this-help", "true",
        "--whats-this-help-target-unique-id", "one-guid"
    });
    expect(!whats_this_help_property_result.ok,
        "#1143: launch contract should reject whats-this-help-object combined with property commands");

    const auto stray_whats_this_help_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--whats-this-help", "true"
    });
    expect(!stray_whats_this_help_result.ok,
        "#1143: launch contract should reject stray whats-this-help arguments");
}

void test_parse_launch_arguments_for_whats_this_button_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--whats-this-button-object",
        "--whats-this-button", ".T.",
        "--whats-this-button-target-object-name", "cmdSave",
        "--whats-this-button-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1144: launch contract should parse whats-this-button-object requests");
    expect(result.request.whats_this_button_object,
        "#1144: launch contract should detect --whats-this-button-object");
    expect(result.request.whats_this_button_available && result.request.whats_this_button,
        "#1144: whats-this-button-object requests should carry WhatsThis button state");
    expect(result.request.whats_this_button_objects.size() == 2U,
        "#1144: whats-this-button-object requests should collect WhatsThis button target selectors");
    if (result.request.whats_this_button_objects.size() == 2U) {
        expect(result.request.whats_this_button_objects[0].object_name == "cmdSave" &&
                result.request.whats_this_button_objects[0].unique_id.empty(),
            "#1144: whats-this-button-object requests should parse target object-name selectors");
        expect(result.request.whats_this_button_objects[1].object_name.empty() &&
                result.request.whats_this_button_objects[1].unique_id == "two-guid",
            "#1144: whats-this-button-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_whats_this_button_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--whats-this-button-object",
        "--whats-this-button-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1144: launch contract should reject whats-this-button-object requests without WhatsThis button value");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--whats-this-button-object",
        "--whats-this-button", "true"
    });
    expect(!missing_targets_result.ok,
        "#1144: launch contract should reject whats-this-button-object requests without target selectors");

    const auto invalid_logical_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--whats-this-button-object",
        "--whats-this-button", "maybe",
        "--whats-this-button-target-unique-id", "one-guid"
    });
    expect(!invalid_logical_result.ok,
        "#1144: launch contract should reject invalid WhatsThis button logical values");
}

void test_parse_launch_arguments_rejects_whats_this_button_object_ambiguity() {
    const auto whats_this_button_locked_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--whats-this-button-object",
        "--locked-object",
        "--whats-this-button", "true",
        "--whats-this-button-target-unique-id", "one-guid",
        "--locked", "true",
        "--locked-target-unique-id", "one-guid"
    });
    expect(!whats_this_button_locked_result.ok,
        "#1144: launch contract should reject simultaneous whats-this-button-object and locked-object requests");

    const auto whats_this_button_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--whats-this-button-object",
        "--clear-property",
        "--property-name", "ToolTipText",
        "--whats-this-button", "true",
        "--whats-this-button-target-unique-id", "one-guid"
    });
    expect(!whats_this_button_property_result.ok,
        "#1144: launch contract should reject whats-this-button-object combined with property commands");

    const auto stray_whats_this_button_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--whats-this-button", "true"
    });
    expect(!stray_whats_this_button_result.ok,
        "#1144: launch contract should reject stray whats-this-button arguments");
}

void test_parse_launch_arguments_for_status_bar_text_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--status-bar-text-object",
        "--status-bar-text", "Ready to save",
        "--status-bar-text-target-object-name", "cmdSave",
        "--status-bar-text-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1044: launch contract should parse status-bar-text-object requests");
    expect(result.request.status_bar_text_object, "#1044: launch contract should detect --status-bar-text-object");
    expect(result.request.status_bar_text_available && result.request.status_bar_text == "Ready to save",
        "#1044: status-bar-text-object requests should carry status-bar text");
    expect(result.request.status_bar_text_objects.size() == 2U,
        "#1044: status-bar-text-object requests should collect status-bar text target selectors");
    if (result.request.status_bar_text_objects.size() == 2U) {
        expect(result.request.status_bar_text_objects[0].object_name == "cmdSave" &&
                result.request.status_bar_text_objects[0].unique_id.empty(),
            "#1044: status-bar-text-object requests should parse target object-name selectors");
        expect(result.request.status_bar_text_objects[1].object_name.empty() &&
                result.request.status_bar_text_objects[1].unique_id == "two-guid",
            "#1044: status-bar-text-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_status_bar_text_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--status-bar-text-object",
        "--status-bar-text-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1044: launch contract should reject status-bar-text-object requests without status-bar text");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--status-bar-text-object",
        "--status-bar-text", "Ready"
    });
    expect(!missing_targets_result.ok,
        "#1044: launch contract should reject status-bar-text-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_status_bar_text_object_ambiguity() {
    const auto status_tooltip_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--status-bar-text-object",
        "--tooltip-text-object",
        "--status-bar-text", "Ready",
        "--status-bar-text-target-unique-id", "one-guid",
        "--tooltip-text", "Save",
        "--tooltip-text-target-unique-id", "one-guid"
    });
    expect(!status_tooltip_result.ok,
        "#1044: launch contract should reject simultaneous status-bar-text-object and tooltip-text-object requests");

    const auto status_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--status-bar-text-object",
        "--clear-property",
        "--property-name", "StatusBarText",
        "--status-bar-text", "Ready",
        "--status-bar-text-target-unique-id", "one-guid"
    });
    expect(!status_property_result.ok,
        "#1044: launch contract should reject status-bar-text-object combined with property commands");

    const auto stray_status_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--status-bar-text", "Ready"
    });
    expect(!stray_status_result.ok,
        "#1044: launch contract should reject stray status-bar text arguments");
}

void test_parse_launch_arguments_for_closable_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--closable-object",
        "--closable", "false",
        "--closable-target-object-name", "frmCustomer",
        "--closable-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1073: launch contract should parse closable-object requests");
    expect(result.request.closable_object,
        "#1073: launch contract should detect --closable-object");
    expect(result.request.closable_available && !result.request.closable,
        "#1073: closable-object requests should carry closable state");
    expect(result.request.closable_objects.size() == 2U,
        "#1073: closable-object requests should collect closable target selectors");
    if (result.request.closable_objects.size() == 2U) {
        expect(result.request.closable_objects[0].object_name == "frmCustomer" &&
                result.request.closable_objects[0].unique_id.empty(),
            "#1073: closable-object requests should parse target object-name selectors");
        expect(result.request.closable_objects[1].object_name.empty() &&
                result.request.closable_objects[1].unique_id == "two-guid",
            "#1073: closable-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_closable_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--closable-object",
        "--closable-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1073: launch contract should reject closable-object requests without closable state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--closable-object",
        "--closable", "false"
    });
    expect(!missing_targets_result.ok,
        "#1073: launch contract should reject closable-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--closable-object",
        "--closable", "sometimes",
        "--closable-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1073: launch contract should reject invalid closable boolean values");
}

void test_parse_launch_arguments_rejects_closable_object_ambiguity() {
    const auto closable_dynamic_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--closable-object",
        "--dynamic-fore-color-object",
        "--closable", "false",
        "--closable-target-unique-id", "one-guid",
        "--dynamic-fore-color", "RGB(7,8,9)",
        "--dynamic-fore-color-target-unique-id", "one-guid"
    });
    expect(!closable_dynamic_result.ok,
        "#1073: launch contract should reject simultaneous closable-object and dynamic-fore-color-object requests");

    const auto closable_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--closable-object",
        "--clear-property",
        "--property-name", "Closable",
        "--closable", "false",
        "--closable-target-unique-id", "one-guid"
    });
    expect(!closable_property_result.ok,
        "#1073: launch contract should reject closable-object combined with property commands");

    const auto stray_closable_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--closable", "false"
    });
    expect(!stray_closable_result.ok,
        "#1073: launch contract should reject stray closable arguments");
}

void test_parse_launch_arguments_for_control_box_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--control-box-object",
        "--control-box", "false",
        "--control-box-target-object-name", "frmCustomer",
        "--control-box-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1074: launch contract should parse control-box-object requests");
    expect(result.request.control_box_object,
        "#1074: launch contract should detect --control-box-object");
    expect(result.request.control_box_available && !result.request.control_box,
        "#1074: control-box-object requests should carry control box state");
    expect(result.request.control_box_objects.size() == 2U,
        "#1074: control-box-object requests should collect control-box target selectors");
    if (result.request.control_box_objects.size() == 2U) {
        expect(result.request.control_box_objects[0].object_name == "frmCustomer" &&
                result.request.control_box_objects[0].unique_id.empty(),
            "#1074: control-box-object requests should parse target object-name selectors");
        expect(result.request.control_box_objects[1].object_name.empty() &&
                result.request.control_box_objects[1].unique_id == "two-guid",
            "#1074: control-box-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_control_box_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--control-box-object",
        "--control-box-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1074: launch contract should reject control-box-object requests without control box state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--control-box-object",
        "--control-box", "false"
    });
    expect(!missing_targets_result.ok,
        "#1074: launch contract should reject control-box-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--control-box-object",
        "--control-box", "sometimes",
        "--control-box-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1074: launch contract should reject invalid control-box boolean values");
}

void test_parse_launch_arguments_rejects_control_box_object_ambiguity() {
    const auto control_box_closable_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--control-box-object",
        "--closable-object",
        "--control-box", "false",
        "--control-box-target-unique-id", "one-guid",
        "--closable", "false",
        "--closable-target-unique-id", "one-guid"
    });
    expect(!control_box_closable_result.ok,
        "#1074: launch contract should reject simultaneous control-box-object and closable-object requests");

    const auto control_box_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--control-box-object",
        "--clear-property",
        "--property-name", "ControlBox",
        "--control-box", "false",
        "--control-box-target-unique-id", "one-guid"
    });
    expect(!control_box_property_result.ok,
        "#1074: launch contract should reject control-box-object combined with property commands");

    const auto stray_control_box_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--control-box", "false"
    });
    expect(!stray_control_box_result.ok,
        "#1074: launch contract should reject stray control-box arguments");
}

void test_parse_launch_arguments_for_desktop_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--desktop-object",
        "--desktop", "false",
        "--desktop-target-object-name", "frmCustomer",
        "--desktop-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1147: launch contract should parse desktop-object requests");
    expect(result.request.desktop_object,
        "#1147: launch contract should detect --desktop-object");
    expect(result.request.desktop_available && !result.request.desktop,
        "#1147: desktop-object requests should carry desktop state");
    expect(result.request.desktop_objects.size() == 2U,
        "#1147: desktop-object requests should collect desktop target selectors");
    if (result.request.desktop_objects.size() == 2U) {
        expect(result.request.desktop_objects[0].object_name == "frmCustomer" &&
                result.request.desktop_objects[0].unique_id.empty(),
            "#1147: desktop-object requests should parse target object-name selectors");
        expect(result.request.desktop_objects[1].object_name.empty() &&
                result.request.desktop_objects[1].unique_id == "two-guid",
            "#1147: desktop-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_desktop_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--desktop-object",
        "--desktop-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1147: launch contract should reject desktop-object requests without desktop state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--desktop-object",
        "--desktop", "false"
    });
    expect(!missing_targets_result.ok,
        "#1147: launch contract should reject desktop-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--desktop-object",
        "--desktop", "sometimes",
        "--desktop-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1147: launch contract should reject invalid desktop boolean values");
}

void test_parse_launch_arguments_rejects_desktop_object_ambiguity() {
    const auto desktop_allow_output_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--desktop-object",
        "--allow-output-object",
        "--desktop", "false",
        "--desktop-target-unique-id", "one-guid",
        "--allow-output", "false",
        "--allow-output-target-unique-id", "one-guid"
    });
    expect(!desktop_allow_output_result.ok,
        "#1147: launch contract should reject simultaneous desktop-object and allow-output-object requests");

    const auto desktop_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--desktop-object",
        "--clear-property",
        "--property-name", "Desktop",
        "--desktop", "false",
        "--desktop-target-unique-id", "one-guid"
    });
    expect(!desktop_property_result.ok,
        "#1147: launch contract should reject desktop-object combined with property commands");

    const auto stray_desktop_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--desktop", "false"
    });
    expect(!stray_desktop_result.ok,
        "#1147: launch contract should reject stray desktop arguments");
}

void test_parse_launch_arguments_for_key_preview_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--key-preview-object",
        "--key-preview", "false",
        "--key-preview-target-object-name", "frmCustomer",
        "--key-preview-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1148: launch contract should parse key-preview-object requests");
    expect(result.request.key_preview_object,
        "#1148: launch contract should detect --key-preview-object");
    expect(result.request.key_preview_available && !result.request.key_preview,
        "#1148: key-preview-object requests should carry key preview state");
    expect(result.request.key_preview_objects.size() == 2U,
        "#1148: key-preview-object requests should collect key-preview target selectors");
    if (result.request.key_preview_objects.size() == 2U) {
        expect(result.request.key_preview_objects[0].object_name == "frmCustomer" &&
                result.request.key_preview_objects[0].unique_id.empty(),
            "#1148: key-preview-object requests should parse target object-name selectors");
        expect(result.request.key_preview_objects[1].object_name.empty() &&
                result.request.key_preview_objects[1].unique_id == "two-guid",
            "#1148: key-preview-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_key_preview_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--key-preview-object",
        "--key-preview-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1148: launch contract should reject key-preview-object requests without key preview state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--key-preview-object",
        "--key-preview", "false"
    });
    expect(!missing_targets_result.ok,
        "#1148: launch contract should reject key-preview-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--key-preview-object",
        "--key-preview", "sometimes",
        "--key-preview-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1148: launch contract should reject invalid key-preview boolean values");
}

void test_parse_launch_arguments_rejects_key_preview_object_ambiguity() {
    const auto key_preview_allow_output_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--key-preview-object",
        "--allow-output-object",
        "--key-preview", "false",
        "--key-preview-target-unique-id", "one-guid",
        "--allow-output", "false",
        "--allow-output-target-unique-id", "one-guid"
    });
    expect(!key_preview_allow_output_result.ok,
        "#1148: launch contract should reject simultaneous key-preview-object and allow-output-object requests");

    const auto key_preview_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--key-preview-object",
        "--clear-property",
        "--property-name", "KeyPreview",
        "--key-preview", "false",
        "--key-preview-target-unique-id", "one-guid"
    });
    expect(!key_preview_property_result.ok,
        "#1148: launch contract should reject key-preview-object combined with property commands");

    const auto stray_key_preview_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--key-preview", "false"
    });
    expect(!stray_key_preview_result.ok,
        "#1148: launch contract should reject stray key-preview arguments");
}

void test_parse_launch_arguments_for_mac_desktop_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--mac-desktop-object",
        "--mac-desktop", "false",
        "--mac-desktop-target-object-name", "frmCustomer",
        "--mac-desktop-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1149: launch contract should parse mac-desktop-object requests");
    expect(result.request.mac_desktop_object,
        "#1149: launch contract should detect --mac-desktop-object");
    expect(result.request.mac_desktop_available && !result.request.mac_desktop,
        "#1149: mac-desktop-object requests should carry mac desktop state");
    expect(result.request.mac_desktop_objects.size() == 2U,
        "#1149: mac-desktop-object requests should collect mac-desktop target selectors");
    if (result.request.mac_desktop_objects.size() == 2U) {
        expect(result.request.mac_desktop_objects[0].object_name == "frmCustomer" &&
                result.request.mac_desktop_objects[0].unique_id.empty(),
            "#1149: mac-desktop-object requests should parse target object-name selectors");
        expect(result.request.mac_desktop_objects[1].object_name.empty() &&
                result.request.mac_desktop_objects[1].unique_id == "two-guid",
            "#1149: mac-desktop-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_mac_desktop_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--mac-desktop-object",
        "--mac-desktop-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1149: launch contract should reject mac-desktop-object requests without mac desktop state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--mac-desktop-object",
        "--mac-desktop", "false"
    });
    expect(!missing_targets_result.ok,
        "#1149: launch contract should reject mac-desktop-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--mac-desktop-object",
        "--mac-desktop", "sometimes",
        "--mac-desktop-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1149: launch contract should reject invalid mac-desktop boolean values");
}

void test_parse_launch_arguments_rejects_mac_desktop_object_ambiguity() {
    const auto mac_desktop_allow_output_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--mac-desktop-object",
        "--allow-output-object",
        "--mac-desktop", "false",
        "--mac-desktop-target-unique-id", "one-guid",
        "--allow-output", "false",
        "--allow-output-target-unique-id", "one-guid"
    });
    expect(!mac_desktop_allow_output_result.ok,
        "#1149: launch contract should reject simultaneous mac-desktop-object and allow-output-object requests");

    const auto mac_desktop_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--mac-desktop-object",
        "--clear-property",
        "--property-name", "MacDesktop",
        "--mac-desktop", "false",
        "--mac-desktop-target-unique-id", "one-guid"
    });
    expect(!mac_desktop_property_result.ok,
        "#1149: launch contract should reject mac-desktop-object combined with property commands");

    const auto stray_mac_desktop_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--mac-desktop", "false"
    });
    expect(!stray_mac_desktop_result.ok,
        "#1149: launch contract should reject stray mac-desktop arguments");
}

void test_parse_launch_arguments_for_max_button_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--max-button-object",
        "--max-button", "false",
        "--max-button-target-object-name", "frmCustomer",
        "--max-button-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1150: launch contract should parse max-button-object requests");
    expect(result.request.max_button_object,
        "#1150: launch contract should detect --max-button-object");
    expect(result.request.max_button_available && !result.request.max_button,
        "#1150: max-button-object requests should carry max button state");
    expect(result.request.max_button_objects.size() == 2U,
        "#1150: max-button-object requests should collect max-button target selectors");
    if (result.request.max_button_objects.size() == 2U) {
        expect(result.request.max_button_objects[0].object_name == "frmCustomer" &&
                result.request.max_button_objects[0].unique_id.empty(),
            "#1150: max-button-object requests should parse target object-name selectors");
        expect(result.request.max_button_objects[1].object_name.empty() &&
                result.request.max_button_objects[1].unique_id == "two-guid",
            "#1150: max-button-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_max_button_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--max-button-object",
        "--max-button-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1150: launch contract should reject max-button-object requests without max button state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--max-button-object",
        "--max-button", "false"
    });
    expect(!missing_targets_result.ok,
        "#1150: launch contract should reject max-button-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--max-button-object",
        "--max-button", "sometimes",
        "--max-button-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1150: launch contract should reject invalid max-button boolean values");
}

void test_parse_launch_arguments_rejects_max_button_object_ambiguity() {
    const auto max_button_allow_output_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--max-button-object",
        "--allow-output-object",
        "--max-button", "false",
        "--max-button-target-unique-id", "one-guid",
        "--allow-output", "false",
        "--allow-output-target-unique-id", "one-guid"
    });
    expect(!max_button_allow_output_result.ok,
        "#1150: launch contract should reject simultaneous max-button-object and allow-output-object requests");

    const auto max_button_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--max-button-object",
        "--clear-property",
        "--property-name", "MaxButton",
        "--max-button", "false",
        "--max-button-target-unique-id", "one-guid"
    });
    expect(!max_button_property_result.ok,
        "#1150: launch contract should reject max-button-object combined with property commands");

    const auto stray_max_button_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--max-button", "false"
    });
    expect(!stray_max_button_result.ok,
        "#1150: launch contract should reject stray max-button arguments");
}

void test_parse_launch_arguments_for_min_button_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--min-button-object",
        "--min-button", "false",
        "--min-button-target-object-name", "frmCustomer",
        "--min-button-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1155: launch contract should parse min-button-object requests");
    expect(result.request.min_button_object,
        "#1155: launch contract should detect --min-button-object");
    expect(result.request.min_button_available && !result.request.min_button,
        "#1155: min-button-object requests should carry min button state");
    expect(result.request.min_button_objects.size() == 2U,
        "#1155: min-button-object requests should collect min-button target selectors");
    if (result.request.min_button_objects.size() == 2U) {
        expect(result.request.min_button_objects[0].object_name == "frmCustomer" &&
                result.request.min_button_objects[0].unique_id.empty(),
            "#1155: min-button-object requests should parse target object-name selectors");
        expect(result.request.min_button_objects[1].object_name.empty() &&
                result.request.min_button_objects[1].unique_id == "two-guid",
            "#1155: min-button-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_min_button_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--min-button-object",
        "--min-button-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1155: launch contract should reject min-button-object requests without min button state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--min-button-object",
        "--min-button", "false"
    });
    expect(!missing_targets_result.ok,
        "#1155: launch contract should reject min-button-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--min-button-object",
        "--min-button", "sometimes",
        "--min-button-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1155: launch contract should reject invalid min-button boolean values");
}

void test_parse_launch_arguments_rejects_min_button_object_ambiguity() {
    const auto min_button_allow_output_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--min-button-object",
        "--allow-output-object",
        "--min-button", "false",
        "--min-button-target-unique-id", "one-guid",
        "--allow-output", "false",
        "--allow-output-target-unique-id", "one-guid"
    });
    expect(!min_button_allow_output_result.ok,
        "#1155: launch contract should reject simultaneous min-button-object and allow-output-object requests");

    const auto min_button_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--min-button-object",
        "--clear-property",
        "--property-name", "MinButton",
        "--min-button", "false",
        "--min-button-target-unique-id", "one-guid"
    });
    expect(!min_button_property_result.ok,
        "#1155: launch contract should reject min-button-object combined with property commands");

    const auto stray_min_button_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--min-button", "false"
    });
    expect(!stray_min_button_result.ok,
        "#1155: launch contract should reject stray min-button arguments");
}


#include "test_studio_host_behavior_window_size_position.inl"

void test_parse_launch_arguments_for_auto_center_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--auto-center-object",
        "--auto-center", "false",
        "--auto-center-target-object-name", "frmCustomer",
        "--auto-center-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1078: launch contract should parse auto-center-object requests");
    expect(result.request.auto_center_object,
        "#1078: launch contract should detect --auto-center-object");
    expect(result.request.auto_center_available && !result.request.auto_center,
        "#1078: auto-center-object requests should carry auto center state");
    expect(result.request.auto_center_objects.size() == 2U,
        "#1078: auto-center-object requests should collect auto-center target selectors");
    if (result.request.auto_center_objects.size() == 2U) {
        expect(result.request.auto_center_objects[0].object_name == "frmCustomer" &&
                result.request.auto_center_objects[0].unique_id.empty(),
            "#1078: auto-center-object requests should parse target object-name selectors");
        expect(result.request.auto_center_objects[1].object_name.empty() &&
                result.request.auto_center_objects[1].unique_id == "two-guid",
            "#1078: auto-center-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_auto_center_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--auto-center-object",
        "--auto-center-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1078: launch contract should reject auto-center-object requests without auto center state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--auto-center-object",
        "--auto-center", "false"
    });
    expect(!missing_targets_result.ok,
        "#1078: launch contract should reject auto-center-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--auto-center-object",
        "--auto-center", "sometimes",
        "--auto-center-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1078: launch contract should reject invalid auto-center boolean values");
}

void test_parse_launch_arguments_rejects_auto_center_object_ambiguity() {
    const auto auto_center_allow_output_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--auto-center-object",
        "--allow-output-object",
        "--auto-center", "false",
        "--auto-center-target-unique-id", "one-guid",
        "--allow-output", "false",
        "--allow-output-target-unique-id", "one-guid"
    });
    expect(!auto_center_allow_output_result.ok,
        "#1078: launch contract should reject simultaneous auto-center-object and allow-output-object requests");

    const auto auto_center_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--auto-center-object",
        "--clear-property",
        "--property-name", "AutoCenter",
        "--auto-center", "false",
        "--auto-center-target-unique-id", "one-guid"
    });
    expect(!auto_center_property_result.ok,
        "#1078: launch contract should reject auto-center-object combined with property commands");

    const auto stray_auto_center_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--auto-center", "false"
    });
    expect(!stray_auto_center_result.ok,
        "#1078: launch contract should reject stray auto-center arguments");
}

void test_parse_launch_arguments_for_dockable_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--dockable-object",
        "--dockable", "false",
        "--dockable-target-object-name", "frmCustomer",
        "--dockable-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1082: launch contract should parse dockable-object requests");
    expect(result.request.dockable_object,
        "#1082: launch contract should detect --dockable-object");
    expect(result.request.dockable_available && !result.request.dockable,
        "#1082: dockable-object requests should carry dockable state");
    expect(result.request.dockable_objects.size() == 2U,
        "#1082: dockable-object requests should collect dockable target selectors");
    if (result.request.dockable_objects.size() == 2U) {
        expect(result.request.dockable_objects[0].object_name == "frmCustomer" &&
                result.request.dockable_objects[0].unique_id.empty(),
            "#1082: dockable-object requests should parse target object-name selectors");
        expect(result.request.dockable_objects[1].object_name.empty() &&
                result.request.dockable_objects[1].unique_id == "two-guid",
            "#1082: dockable-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_dockable_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dockable-object",
        "--dockable-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1082: launch contract should reject dockable-object requests without dockable state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dockable-object",
        "--dockable", "false"
    });
    expect(!missing_targets_result.ok,
        "#1082: launch contract should reject dockable-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dockable-object",
        "--dockable", "sometimes",
        "--dockable-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1082: launch contract should reject invalid dockable boolean values");
}

void test_parse_launch_arguments_rejects_dockable_object_ambiguity() {
    const auto dockable_auto_size_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dockable-object",
        "--auto-size-object",
        "--dockable", "false",
        "--dockable-target-unique-id", "one-guid",
        "--auto-size", "false",
        "--auto-size-target-unique-id", "one-guid"
    });
    expect(!dockable_auto_size_result.ok,
        "#1082: launch contract should reject simultaneous dockable-object and auto-size-object requests");

    const auto dockable_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dockable-object",
        "--clear-property",
        "--property-name", "Dockable",
        "--dockable", "false",
        "--dockable-target-unique-id", "one-guid"
    });
    expect(!dockable_property_result.ok,
        "#1082: launch contract should reject dockable-object combined with property commands");

    const auto stray_dockable_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dockable", "false"
    });
    expect(!stray_dockable_result.ok,
        "#1082: launch contract should reject stray dockable arguments");
}

void test_parse_launch_arguments_for_lock_screen_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--lock-screen-object",
        "--lock-screen", "false",
        "--lock-screen-target-object-name", "frmCustomer",
        "--lock-screen-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1085: launch contract should parse lock-screen-object requests");
    expect(result.request.lock_screen_object,
        "#1085: launch contract should detect --lock-screen-object");
    expect(result.request.lock_screen_available && !result.request.lock_screen,
        "#1085: lock-screen-object requests should carry lock screen state");
    expect(result.request.lock_screen_objects.size() == 2U,
        "#1085: lock-screen-object requests should collect lock_screen target selectors");
    if (result.request.lock_screen_objects.size() == 2U) {
        expect(result.request.lock_screen_objects[0].object_name == "frmCustomer" &&
                result.request.lock_screen_objects[0].unique_id.empty(),
            "#1085: lock-screen-object requests should parse target object-name selectors");
        expect(result.request.lock_screen_objects[1].object_name.empty() &&
                result.request.lock_screen_objects[1].unique_id == "two-guid",
            "#1085: lock-screen-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_lock_screen_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--lock-screen-object",
        "--lock-screen-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1085: launch contract should reject lock-screen-object requests without lock screen state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--lock-screen-object",
        "--lock-screen", "false"
    });
    expect(!missing_targets_result.ok,
        "#1085: launch contract should reject lock-screen-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--lock-screen-object",
        "--lock-screen", "sometimes",
        "--lock-screen-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1085: launch contract should reject invalid lock-screen boolean values");
}

void test_parse_launch_arguments_rejects_lock_screen_object_ambiguity() {
    const auto lock_screen_auto_size_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--lock-screen-object",
        "--auto-size-object",
        "--lock-screen", "false",
        "--lock-screen-target-unique-id", "one-guid",
        "--auto-size", "false",
        "--auto-size-target-unique-id", "one-guid"
    });
    expect(!lock_screen_auto_size_result.ok,
        "#1085: launch contract should reject simultaneous lock-screen-object and auto-size-object requests");

    const auto lock_screen_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--lock-screen-object",
        "--clear-property",
        "--property-name", "Dockable",
        "--lock-screen", "false",
        "--lock-screen-target-unique-id", "one-guid"
    });
    expect(!lock_screen_property_result.ok,
        "#1085: launch contract should reject lock-screen-object combined with property commands");

    const auto stray_lock_screen_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--lock-screen", "false"
    });
    expect(!stray_lock_screen_result.ok,
        "#1085: launch contract should reject stray lock-screen arguments");
}

void test_parse_launch_arguments_for_split_bar_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--split-bar-object",
        "--split-bar", "false",
        "--split-bar-target-object-name", "frmCustomer",
        "--split-bar-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1089: launch contract should parse split-bar-object requests");
    expect(result.request.split_bar_object,
        "#1089: launch contract should detect --split-bar-object");
    expect(result.request.split_bar_available && !result.request.split_bar,
        "#1089: split-bar-object requests should carry split bar state");
    expect(result.request.split_bar_objects.size() == 2U,
        "#1089: split-bar-object requests should collect split_bar target selectors");
    if (result.request.split_bar_objects.size() == 2U) {
        expect(result.request.split_bar_objects[0].object_name == "frmCustomer" &&
                result.request.split_bar_objects[0].unique_id.empty(),
            "#1089: split-bar-object requests should parse target object-name selectors");
        expect(result.request.split_bar_objects[1].object_name.empty() &&
                result.request.split_bar_objects[1].unique_id == "two-guid",
            "#1089: split-bar-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_split_bar_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--split-bar-object",
        "--split-bar-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1089: launch contract should reject split-bar-object requests without split bar state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--split-bar-object",
        "--split-bar", "false"
    });
    expect(!missing_targets_result.ok,
        "#1089: launch contract should reject split-bar-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--split-bar-object",
        "--split-bar", "sometimes",
        "--split-bar-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1089: launch contract should reject invalid split-bar boolean values");
}

void test_parse_launch_arguments_rejects_split_bar_object_ambiguity() {
    const auto split_bar_auto_size_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--split-bar-object",
        "--auto-size-object",
        "--split-bar", "false",
        "--split-bar-target-unique-id", "one-guid",
        "--auto-size", "false",
        "--auto-size-target-unique-id", "one-guid"
    });
    expect(!split_bar_auto_size_result.ok,
        "#1089: launch contract should reject simultaneous split-bar-object and auto-size-object requests");

    const auto split_bar_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--split-bar-object",
        "--clear-property",
        "--property-name", "Dockable",
        "--split-bar", "false",
        "--split-bar-target-unique-id", "one-guid"
    });
    expect(!split_bar_property_result.ok,
        "#1089: launch contract should reject split-bar-object combined with property commands");

    const auto stray_split_bar_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--split-bar", "false"
    });
    expect(!stray_split_bar_result.ok,
        "#1089: launch contract should reject stray split-bar arguments");
}

void test_parse_launch_arguments_for_panel_link_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--panel-link-object",
        "--panel-link", "false",
        "--panel-link-target-object-name", "frmCustomer",
        "--panel-link-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1091: launch contract should parse panel-link-object requests");
    expect(result.request.panel_link_object,
        "#1091: launch contract should detect --panel-link-object");
    expect(result.request.panel_link_available && !result.request.panel_link,
        "#1091: panel-link-object requests should carry panel link state");
    expect(result.request.panel_link_objects.size() == 2U,
        "#1091: panel-link-object requests should collect panel_link target selectors");
    if (result.request.panel_link_objects.size() == 2U) {
        expect(result.request.panel_link_objects[0].object_name == "frmCustomer" &&
                result.request.panel_link_objects[0].unique_id.empty(),
            "#1091: panel-link-object requests should parse target object-name selectors");
        expect(result.request.panel_link_objects[1].object_name.empty() &&
                result.request.panel_link_objects[1].unique_id == "two-guid",
            "#1091: panel-link-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_panel_link_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--panel-link-object",
        "--panel-link-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1091: launch contract should reject panel-link-object requests without panel link state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--panel-link-object",
        "--panel-link", "false"
    });
    expect(!missing_targets_result.ok,
        "#1091: launch contract should reject panel-link-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--panel-link-object",
        "--panel-link", "sometimes",
        "--panel-link-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1091: launch contract should reject invalid panel-link boolean values");
}

void test_parse_launch_arguments_rejects_panel_link_object_ambiguity() {
    const auto panel_link_auto_size_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--panel-link-object",
        "--auto-size-object",
        "--panel-link", "false",
        "--panel-link-target-unique-id", "one-guid",
        "--auto-size", "false",
        "--auto-size-target-unique-id", "one-guid"
    });
    expect(!panel_link_auto_size_result.ok,
        "#1091: launch contract should reject simultaneous panel-link-object and auto-size-object requests");

    const auto panel_link_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--panel-link-object",
        "--clear-property",
        "--property-name", "Dockable",
        "--panel-link", "false",
        "--panel-link-target-unique-id", "one-guid"
    });
    expect(!panel_link_property_result.ok,
        "#1091: launch contract should reject panel-link-object combined with property commands");

    const auto stray_panel_link_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--panel-link", "false"
    });
    expect(!stray_panel_link_result.ok,
        "#1091: launch contract should reject stray panel-link arguments");
}

void test_parse_launch_arguments_for_resizable_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--resizable-object",
        "--resizable", "false",
        "--resizable-target-object-name", "frmCustomer",
        "--resizable-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1094: launch contract should parse resizable-object requests");
    expect(result.request.resizable_object,
        "#1094: launch contract should detect --resizable-object");
    expect(result.request.resizable_available && !result.request.resizable,
        "#1094: resizable-object requests should carry resizable state");
    expect(result.request.resizable_objects.size() == 2U,
        "#1094: resizable-object requests should collect resizable target selectors");
    if (result.request.resizable_objects.size() == 2U) {
        expect(result.request.resizable_objects[0].object_name == "frmCustomer" &&
                result.request.resizable_objects[0].unique_id.empty(),
            "#1094: resizable-object requests should parse target object-name selectors");
        expect(result.request.resizable_objects[1].object_name.empty() &&
                result.request.resizable_objects[1].unique_id == "two-guid",
            "#1094: resizable-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_resizable_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--resizable-object",
        "--resizable-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1094: launch contract should reject resizable-object requests without resizable state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--resizable-object",
        "--resizable", "false"
    });
    expect(!missing_targets_result.ok,
        "#1094: launch contract should reject resizable-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--resizable-object",
        "--resizable", "sometimes",
        "--resizable-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1094: launch contract should reject invalid resizable boolean values");
}

void test_parse_launch_arguments_rejects_resizable_object_ambiguity() {
    const auto resizable_auto_size_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--resizable-object",
        "--auto-size-object",
        "--resizable", "false",
        "--resizable-target-unique-id", "one-guid",
        "--auto-size", "false",
        "--auto-size-target-unique-id", "one-guid"
    });
    expect(!resizable_auto_size_result.ok,
        "#1094: launch contract should reject simultaneous resizable-object and auto-size-object requests");

    const auto resizable_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--resizable-object",
        "--clear-property",
        "--property-name", "Dockable",
        "--resizable", "false",
        "--resizable-target-unique-id", "one-guid"
    });
    expect(!resizable_property_result.ok,
        "#1094: launch contract should reject resizable-object combined with property commands");

    const auto stray_resizable_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--resizable", "false"
    });
    expect(!stray_resizable_result.ok,
        "#1094: launch contract should reject stray resizable arguments");
}

void test_parse_launch_arguments_for_always_on_top_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--always-on-top-object",
        "--always-on-top", "false",
        "--always-on-top-target-object-name", "frmCustomer",
        "--always-on-top-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1096: launch contract should parse always-on-top-object requests");
    expect(result.request.always_on_top_object,
        "#1096: launch contract should detect --always-on-top-object");
    expect(result.request.always_on_top_available && !result.request.always_on_top,
        "#1096: always-on-top-object requests should carry always on top state");
    expect(result.request.always_on_top_objects.size() == 2U,
        "#1096: always-on-top-object requests should collect always_on_top target selectors");
    if (result.request.always_on_top_objects.size() == 2U) {
        expect(result.request.always_on_top_objects[0].object_name == "frmCustomer" &&
                result.request.always_on_top_objects[0].unique_id.empty(),
            "#1096: always-on-top-object requests should parse target object-name selectors");
        expect(result.request.always_on_top_objects[1].object_name.empty() &&
                result.request.always_on_top_objects[1].unique_id == "two-guid",
            "#1096: always-on-top-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_always_on_top_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--always-on-top-object",
        "--always-on-top-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1096: launch contract should reject always-on-top-object requests without always on top state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--always-on-top-object",
        "--always-on-top", "false"
    });
    expect(!missing_targets_result.ok,
        "#1096: launch contract should reject always-on-top-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--always-on-top-object",
        "--always-on-top", "sometimes",
        "--always-on-top-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1096: launch contract should reject invalid always-on-top boolean values");
}

void test_parse_launch_arguments_rejects_always_on_top_object_ambiguity() {
    const auto always_on_top_auto_size_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--always-on-top-object",
        "--auto-size-object",
        "--always-on-top", "false",
        "--always-on-top-target-unique-id", "one-guid",
        "--auto-size", "false",
        "--auto-size-target-unique-id", "one-guid"
    });
    expect(!always_on_top_auto_size_result.ok,
        "#1096: launch contract should reject simultaneous always-on-top-object and auto-size-object requests");

    const auto always_on_top_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--always-on-top-object",
        "--clear-property",
        "--property-name", "Dockable",
        "--always-on-top", "false",
        "--always-on-top-target-unique-id", "one-guid"
    });
    expect(!always_on_top_property_result.ok,
        "#1096: launch contract should reject always-on-top-object combined with property commands");

    const auto stray_always_on_top_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--always-on-top", "false"
    });
    expect(!stray_always_on_top_result.ok,
        "#1096: launch contract should reject stray always-on-top arguments");
}

void test_parse_launch_arguments_for_always_on_bottom_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--always-on-bottom-object",
        "--always-on-bottom", "false",
        "--always-on-bottom-target-object-name", "frmCustomer",
        "--always-on-bottom-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1097: launch contract should parse always-on-bottom-object requests");
    expect(result.request.always_on_bottom_object,
        "#1097: launch contract should detect --always-on-bottom-object");
    expect(result.request.always_on_bottom_available && !result.request.always_on_bottom,
        "#1097: always-on-bottom-object requests should carry always on bottom state");
    expect(result.request.always_on_bottom_objects.size() == 2U,
        "#1097: always-on-bottom-object requests should collect always_on_bottom target selectors");
    if (result.request.always_on_bottom_objects.size() == 2U) {
        expect(result.request.always_on_bottom_objects[0].object_name == "frmCustomer" &&
                result.request.always_on_bottom_objects[0].unique_id.empty(),
            "#1097: always-on-bottom-object requests should parse target object-name selectors");
        expect(result.request.always_on_bottom_objects[1].object_name.empty() &&
                result.request.always_on_bottom_objects[1].unique_id == "two-guid",
            "#1097: always-on-bottom-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_always_on_bottom_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--always-on-bottom-object",
        "--always-on-bottom-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1097: launch contract should reject always-on-bottom-object requests without always on bottom state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--always-on-bottom-object",
        "--always-on-bottom", "false"
    });
    expect(!missing_targets_result.ok,
        "#1097: launch contract should reject always-on-bottom-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--always-on-bottom-object",
        "--always-on-bottom", "sometimes",
        "--always-on-bottom-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1097: launch contract should reject invalid always-on-bottom boolean values");
}

void test_parse_launch_arguments_rejects_always_on_bottom_object_ambiguity() {
    const auto always_on_bottom_auto_size_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--always-on-bottom-object",
        "--auto-size-object",
        "--always-on-bottom", "false",
        "--always-on-bottom-target-unique-id", "one-guid",
        "--auto-size", "false",
        "--auto-size-target-unique-id", "one-guid"
    });
    expect(!always_on_bottom_auto_size_result.ok,
        "#1097: launch contract should reject simultaneous always-on-bottom-object and auto-size-object requests");

    const auto always_on_bottom_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--always-on-bottom-object",
        "--clear-property",
        "--property-name", "Dockable",
        "--always-on-bottom", "false",
        "--always-on-bottom-target-unique-id", "one-guid"
    });
    expect(!always_on_bottom_property_result.ok,
        "#1097: launch contract should reject always-on-bottom-object combined with property commands");

    const auto stray_always_on_bottom_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--always-on-bottom", "false"
    });
    expect(!stray_always_on_bottom_result.ok,
        "#1097: launch contract should reject stray always-on-bottom arguments");
}

}  // namespace cf_test_studio_host
