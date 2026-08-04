// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_studio_host_support.h"

namespace cf_test_studio_host {
void test_parse_launch_arguments_for_dynamic_line_height_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--dynamic-line-height-object",
        "--dynamic-line-height", "IIF(.T., 18, 12)",
        "--dynamic-line-height-target-object-name", "txtNotes",
        "--dynamic-line-height-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1177: launch contract should parse dynamic-line-height-object requests");
    expect(result.request.dynamic_line_height_object,
        "#1177: launch contract should detect --dynamic-line-height-object");
    expect(result.request.dynamic_line_height_available &&
            result.request.dynamic_line_height == "IIF(.T., 18, 12)",
        "#1177: dynamic-line-height-object requests should carry raw expression text");
    expect(result.request.dynamic_line_height_objects.size() == 2U,
        "#1177: dynamic-line-height-object requests should collect dynamic-line-height target selectors");
    if (result.request.dynamic_line_height_objects.size() == 2U) {
        expect(result.request.dynamic_line_height_objects[0].object_name == "txtNotes" &&
                result.request.dynamic_line_height_objects[0].unique_id.empty(),
            "#1177: dynamic-line-height-object requests should parse target object-name selectors");
        expect(result.request.dynamic_line_height_objects[1].object_name.empty() &&
                result.request.dynamic_line_height_objects[1].unique_id == "two-guid",
            "#1177: dynamic-line-height-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_dynamic_line_height_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-line-height-object",
        "--dynamic-line-height-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1177: launch contract should reject dynamic-line-height-object requests without dynamic line height");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-line-height-object",
        "--dynamic-line-height", "IIF(.T., 18, 12)"
    });
    expect(!missing_targets_result.ok,
        "#1177: launch contract should reject dynamic-line-height-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_dynamic_line_height_object_ambiguity() {
    const auto dynamic_line_height_allow_output_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-line-height-object",
        "--allow-output-object",
        "--dynamic-line-height", "IIF(.T., 18, 12)",
        "--dynamic-line-height-target-unique-id", "one-guid",
        "--allow-output", "false",
        "--allow-output-target-unique-id", "one-guid"
    });
    expect(!dynamic_line_height_allow_output_result.ok,
        "#1177: launch contract should reject simultaneous dynamic-line-height-object and allow-output-object requests");

    const auto dynamic_line_height_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-line-height-object",
        "--clear-property",
        "--property-name", "DynamicLineHeight",
        "--dynamic-line-height", "IIF(.T., 18, 12)",
        "--dynamic-line-height-target-unique-id", "one-guid"
    });
    expect(!dynamic_line_height_property_result.ok,
        "#1177: launch contract should reject dynamic-line-height-object combined with property commands");

    const auto stray_dynamic_line_height_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-line-height", "IIF(.T., 18, 12)"
    });
    expect(!stray_dynamic_line_height_result.ok,
        "#1177: launch contract should reject stray dynamic-line-height arguments");
}

void test_parse_launch_arguments_for_dynamic_font_name_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--dynamic-font-name-object",
        "--dynamic-font-name", "IIF(.T., 'Arial', 'Tahoma')",
        "--dynamic-font-name-target-object-name", "txtName",
        "--dynamic-font-name-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1188: launch contract should parse dynamic-font-name-object requests");
    expect(result.request.dynamic_font_name_object,
        "#1188: launch contract should detect --dynamic-font-name-object");
    expect(result.request.dynamic_font_name_available &&
            result.request.dynamic_font_name == "IIF(.T., 'Arial', 'Tahoma')",
        "#1188: dynamic-font-name-object requests should carry raw expression text");
    expect(result.request.dynamic_font_name_objects.size() == 2U,
        "#1188: dynamic-font-name-object requests should collect dynamic-font-name target selectors");
    if (result.request.dynamic_font_name_objects.size() == 2U) {
        expect(result.request.dynamic_font_name_objects[0].object_name == "txtName" &&
                result.request.dynamic_font_name_objects[0].unique_id.empty(),
            "#1188: dynamic-font-name-object requests should parse target object-name selectors");
        expect(result.request.dynamic_font_name_objects[1].object_name.empty() &&
                result.request.dynamic_font_name_objects[1].unique_id == "two-guid",
            "#1188: dynamic-font-name-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_dynamic_font_name_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-font-name-object",
        "--dynamic-font-name-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1188: launch contract should reject dynamic-font-name-object requests without dynamic font name");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-font-name-object",
        "--dynamic-font-name", "IIF(.T., 'Arial', 'Tahoma')"
    });
    expect(!missing_targets_result.ok,
        "#1188: launch contract should reject dynamic-font-name-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_dynamic_font_name_object_ambiguity() {
    const auto dynamic_font_name_allow_output_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-font-name-object",
        "--allow-output-object",
        "--dynamic-font-name", "IIF(.T., 'Arial', 'Tahoma')",
        "--dynamic-font-name-target-unique-id", "one-guid",
        "--allow-output", "false",
        "--allow-output-target-unique-id", "one-guid"
    });
    expect(!dynamic_font_name_allow_output_result.ok,
        "#1188: launch contract should reject simultaneous dynamic-font-name-object and allow-output-object requests");

    const auto dynamic_font_name_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-font-name-object",
        "--clear-property",
        "--property-name", "DynamicFontName",
        "--dynamic-font-name", "IIF(.T., 'Arial', 'Tahoma')",
        "--dynamic-font-name-target-unique-id", "one-guid"
    });
    expect(!dynamic_font_name_property_result.ok,
        "#1188: launch contract should reject dynamic-font-name-object combined with property commands");

    const auto stray_dynamic_font_name_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-font-name", "IIF(.T., 'Arial', 'Tahoma')"
    });
    expect(!stray_dynamic_font_name_result.ok,
        "#1188: launch contract should reject stray dynamic-font-name arguments");
}

void test_parse_launch_arguments_for_dynamic_font_size_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--dynamic-font-size-object",
        "--dynamic-font-size", "IIF(.T., 14, 10)",
        "--dynamic-font-size-target-object-name", "txtName",
        "--dynamic-font-size-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1189: launch contract should parse dynamic-font-size-object requests");
    expect(result.request.dynamic_font_size_object,
        "#1189: launch contract should detect --dynamic-font-size-object");
    expect(result.request.dynamic_font_size_available &&
            result.request.dynamic_font_size == "IIF(.T., 14, 10)",
        "#1189: dynamic-font-size-object requests should carry raw expression text");
    expect(result.request.dynamic_font_size_objects.size() == 2U,
        "#1189: dynamic-font-size-object requests should collect dynamic-font-size target selectors");
    if (result.request.dynamic_font_size_objects.size() == 2U) {
        expect(result.request.dynamic_font_size_objects[0].object_name == "txtName" &&
                result.request.dynamic_font_size_objects[0].unique_id.empty(),
            "#1189: dynamic-font-size-object requests should parse target object-name selectors");
        expect(result.request.dynamic_font_size_objects[1].object_name.empty() &&
                result.request.dynamic_font_size_objects[1].unique_id == "two-guid",
            "#1189: dynamic-font-size-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_dynamic_font_size_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-font-size-object",
        "--dynamic-font-size-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1189: launch contract should reject dynamic-font-size-object requests without dynamic font size");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-font-size-object",
        "--dynamic-font-size", "IIF(.T., 14, 10)"
    });
    expect(!missing_targets_result.ok,
        "#1189: launch contract should reject dynamic-font-size-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_dynamic_font_size_object_ambiguity() {
    const auto dynamic_font_size_allow_output_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-font-size-object",
        "--allow-output-object",
        "--dynamic-font-size", "IIF(.T., 14, 10)",
        "--dynamic-font-size-target-unique-id", "one-guid",
        "--allow-output", "false",
        "--allow-output-target-unique-id", "one-guid"
    });
    expect(!dynamic_font_size_allow_output_result.ok,
        "#1189: launch contract should reject simultaneous dynamic-font-size-object and allow-output-object requests");

    const auto dynamic_font_size_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-font-size-object",
        "--clear-property",
        "--property-name", "DynamicFontSize",
        "--dynamic-font-size", "IIF(.T., 14, 10)",
        "--dynamic-font-size-target-unique-id", "one-guid"
    });
    expect(!dynamic_font_size_property_result.ok,
        "#1189: launch contract should reject dynamic-font-size-object combined with property commands");

    const auto stray_dynamic_font_size_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-font-size", "IIF(.T., 14, 10)"
    });
    expect(!stray_dynamic_font_size_result.ok,
        "#1189: launch contract should reject stray dynamic-font-size arguments");
}

void test_parse_launch_arguments_for_dynamic_font_bold_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--dynamic-font-bold-object",
        "--dynamic-font-bold", "IIF(.T., .T., .F.)",
        "--dynamic-font-bold-target-object-name", "txtName",
        "--dynamic-font-bold-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1190: launch contract should parse dynamic-font-bold-object requests");
    expect(result.request.dynamic_font_bold_object,
        "#1190: launch contract should detect --dynamic-font-bold-object");
    expect(result.request.dynamic_font_bold_available &&
            result.request.dynamic_font_bold == "IIF(.T., .T., .F.)",
        "#1190: dynamic-font-bold-object requests should carry raw expression text");
    expect(result.request.dynamic_font_bold_objects.size() == 2U,
        "#1190: dynamic-font-bold-object requests should collect dynamic-font-bold target selectors");
    if (result.request.dynamic_font_bold_objects.size() == 2U) {
        expect(result.request.dynamic_font_bold_objects[0].object_name == "txtName" &&
                result.request.dynamic_font_bold_objects[0].unique_id.empty(),
            "#1190: dynamic-font-bold-object requests should parse target object-name selectors");
        expect(result.request.dynamic_font_bold_objects[1].object_name.empty() &&
                result.request.dynamic_font_bold_objects[1].unique_id == "two-guid",
            "#1190: dynamic-font-bold-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_dynamic_font_bold_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-font-bold-object",
        "--dynamic-font-bold-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1190: launch contract should reject dynamic-font-bold-object requests without dynamic font bold");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-font-bold-object",
        "--dynamic-font-bold", "IIF(.T., .T., .F.)"
    });
    expect(!missing_targets_result.ok,
        "#1190: launch contract should reject dynamic-font-bold-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_dynamic_font_bold_object_ambiguity() {
    const auto dynamic_font_bold_allow_output_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-font-bold-object",
        "--allow-output-object",
        "--dynamic-font-bold", "IIF(.T., .T., .F.)",
        "--dynamic-font-bold-target-unique-id", "one-guid",
        "--allow-output", "false",
        "--allow-output-target-unique-id", "one-guid"
    });
    expect(!dynamic_font_bold_allow_output_result.ok,
        "#1190: launch contract should reject simultaneous dynamic-font-bold-object and allow-output-object requests");

    const auto dynamic_font_bold_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-font-bold-object",
        "--clear-property",
        "--property-name", "DynamicFontBold",
        "--dynamic-font-bold", "IIF(.T., .T., .F.)",
        "--dynamic-font-bold-target-unique-id", "one-guid"
    });
    expect(!dynamic_font_bold_property_result.ok,
        "#1190: launch contract should reject dynamic-font-bold-object combined with property commands");

    const auto stray_dynamic_font_bold_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-font-bold", "IIF(.T., .T., .F.)"
    });
    expect(!stray_dynamic_font_bold_result.ok,
        "#1190: launch contract should reject stray dynamic-font-bold arguments");
}

void test_parse_launch_arguments_for_dynamic_font_italic_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--dynamic-font-italic-object",
        "--dynamic-font-italic", "IIF(.T., .T., .F.)",
        "--dynamic-font-italic-target-object-name", "txtName",
        "--dynamic-font-italic-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1191: launch contract should parse dynamic-font-italic-object requests");
    expect(result.request.dynamic_font_italic_object,
        "#1191: launch contract should detect --dynamic-font-italic-object");
    expect(result.request.dynamic_font_italic_available &&
            result.request.dynamic_font_italic == "IIF(.T., .T., .F.)",
        "#1191: dynamic-font-italic-object requests should carry raw expression text");
    expect(result.request.dynamic_font_italic_objects.size() == 2U,
        "#1191: dynamic-font-italic-object requests should collect dynamic-font-italic target selectors");
    if (result.request.dynamic_font_italic_objects.size() == 2U) {
        expect(result.request.dynamic_font_italic_objects[0].object_name == "txtName" &&
                result.request.dynamic_font_italic_objects[0].unique_id.empty(),
            "#1191: dynamic-font-italic-object requests should parse target object-name selectors");
        expect(result.request.dynamic_font_italic_objects[1].object_name.empty() &&
                result.request.dynamic_font_italic_objects[1].unique_id == "two-guid",
            "#1191: dynamic-font-italic-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_dynamic_font_italic_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-font-italic-object",
        "--dynamic-font-italic-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1191: launch contract should reject dynamic-font-italic-object requests without dynamic font italic");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-font-italic-object",
        "--dynamic-font-italic", "IIF(.T., .T., .F.)"
    });
    expect(!missing_targets_result.ok,
        "#1191: launch contract should reject dynamic-font-italic-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_dynamic_font_italic_object_ambiguity() {
    const auto dynamic_font_italic_allow_output_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-font-italic-object",
        "--allow-output-object",
        "--dynamic-font-italic", "IIF(.T., .T., .F.)",
        "--dynamic-font-italic-target-unique-id", "one-guid",
        "--allow-output", "false",
        "--allow-output-target-unique-id", "one-guid"
    });
    expect(!dynamic_font_italic_allow_output_result.ok,
        "#1191: launch contract should reject simultaneous dynamic-font-italic-object and allow-output-object requests");

    const auto dynamic_font_italic_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-font-italic-object",
        "--clear-property",
        "--property-name", "DynamicFontItalic",
        "--dynamic-font-italic", "IIF(.T., .T., .F.)",
        "--dynamic-font-italic-target-unique-id", "one-guid"
    });
    expect(!dynamic_font_italic_property_result.ok,
        "#1191: launch contract should reject dynamic-font-italic-object combined with property commands");

    const auto stray_dynamic_font_italic_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-font-italic", "IIF(.T., .T., .F.)"
    });
    expect(!stray_dynamic_font_italic_result.ok,
        "#1191: launch contract should reject stray dynamic-font-italic arguments");
}

void test_parse_launch_arguments_for_dynamic_font_underline_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--dynamic-font-underline-object",
        "--dynamic-font-underline", "IIF(.T., .T., .F.)",
        "--dynamic-font-underline-target-object-name", "txtName",
        "--dynamic-font-underline-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1192: launch contract should parse dynamic-font-underline-object requests");
    expect(result.request.dynamic_font_underline_object,
        "#1192: launch contract should detect --dynamic-font-underline-object");
    expect(result.request.dynamic_font_underline_available &&
            result.request.dynamic_font_underline == "IIF(.T., .T., .F.)",
        "#1192: dynamic-font-underline-object requests should carry raw expression text");
    expect(result.request.dynamic_font_underline_objects.size() == 2U,
        "#1192: dynamic-font-underline-object requests should collect dynamic-font-underline target selectors");
    if (result.request.dynamic_font_underline_objects.size() == 2U) {
        expect(result.request.dynamic_font_underline_objects[0].object_name == "txtName" &&
                result.request.dynamic_font_underline_objects[0].unique_id.empty(),
            "#1192: dynamic-font-underline-object requests should parse target object-name selectors");
        expect(result.request.dynamic_font_underline_objects[1].object_name.empty() &&
                result.request.dynamic_font_underline_objects[1].unique_id == "two-guid",
            "#1192: dynamic-font-underline-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_dynamic_font_underline_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-font-underline-object",
        "--dynamic-font-underline-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1192: launch contract should reject dynamic-font-underline-object requests without dynamic font underline");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-font-underline-object",
        "--dynamic-font-underline", "IIF(.T., .T., .F.)"
    });
    expect(!missing_targets_result.ok,
        "#1192: launch contract should reject dynamic-font-underline-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_dynamic_font_underline_object_ambiguity() {
    const auto dynamic_font_underline_allow_output_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-font-underline-object",
        "--allow-output-object",
        "--dynamic-font-underline", "IIF(.T., .T., .F.)",
        "--dynamic-font-underline-target-unique-id", "one-guid",
        "--allow-output", "false",
        "--allow-output-target-unique-id", "one-guid"
    });
    expect(!dynamic_font_underline_allow_output_result.ok,
        "#1192: launch contract should reject simultaneous dynamic-font-underline-object and allow-output-object requests");

    const auto dynamic_font_underline_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-font-underline-object",
        "--clear-property",
        "--property-name", "DynamicFontUnderline",
        "--dynamic-font-underline", "IIF(.T., .T., .F.)",
        "--dynamic-font-underline-target-unique-id", "one-guid"
    });
    expect(!dynamic_font_underline_property_result.ok,
        "#1192: launch contract should reject dynamic-font-underline-object combined with property commands");

    const auto stray_dynamic_font_underline_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-font-underline", "IIF(.T., .T., .F.)"
    });
    expect(!stray_dynamic_font_underline_result.ok,
        "#1192: launch contract should reject stray dynamic-font-underline arguments");
}

void test_parse_launch_arguments_for_dynamic_font_strikethru_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--dynamic-font-strikethru-object",
        "--dynamic-font-strikethru", "IIF(.T., .T., .F.)",
        "--dynamic-font-strikethru-target-object-name", "txtName",
        "--dynamic-font-strikethru-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1193: launch contract should parse dynamic-font-strikethru-object requests");
    expect(result.request.dynamic_font_strikethru_object,
        "#1193: launch contract should detect --dynamic-font-strikethru-object");
    expect(result.request.dynamic_font_strikethru_available &&
            result.request.dynamic_font_strikethru == "IIF(.T., .T., .F.)",
        "#1193: dynamic-font-strikethru-object requests should carry raw expression text");
    expect(result.request.dynamic_font_strikethru_objects.size() == 2U,
        "#1193: dynamic-font-strikethru-object requests should collect dynamic-font-strikethru target selectors");
    if (result.request.dynamic_font_strikethru_objects.size() == 2U) {
        expect(result.request.dynamic_font_strikethru_objects[0].object_name == "txtName" &&
                result.request.dynamic_font_strikethru_objects[0].unique_id.empty(),
            "#1193: dynamic-font-strikethru-object requests should parse target object-name selectors");
        expect(result.request.dynamic_font_strikethru_objects[1].object_name.empty() &&
                result.request.dynamic_font_strikethru_objects[1].unique_id == "two-guid",
            "#1193: dynamic-font-strikethru-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_dynamic_font_strikethru_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-font-strikethru-object",
        "--dynamic-font-strikethru-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1193: launch contract should reject dynamic-font-strikethru-object requests without dynamic font strikethru");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-font-strikethru-object",
        "--dynamic-font-strikethru", "IIF(.T., .T., .F.)"
    });
    expect(!missing_targets_result.ok,
        "#1193: launch contract should reject dynamic-font-strikethru-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_dynamic_font_strikethru_object_ambiguity() {
    const auto dynamic_font_strikethru_allow_output_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-font-strikethru-object",
        "--allow-output-object",
        "--dynamic-font-strikethru", "IIF(.T., .T., .F.)",
        "--dynamic-font-strikethru-target-unique-id", "one-guid",
        "--allow-output", "false",
        "--allow-output-target-unique-id", "one-guid"
    });
    expect(!dynamic_font_strikethru_allow_output_result.ok,
        "#1193: launch contract should reject simultaneous dynamic-font-strikethru-object and allow-output-object requests");

    const auto dynamic_font_strikethru_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-font-strikethru-object",
        "--clear-property",
        "--property-name", "DynamicFontStrikethru",
        "--dynamic-font-strikethru", "IIF(.T., .T., .F.)",
        "--dynamic-font-strikethru-target-unique-id", "one-guid"
    });
    expect(!dynamic_font_strikethru_property_result.ok,
        "#1193: launch contract should reject dynamic-font-strikethru-object combined with property commands");

    const auto stray_dynamic_font_strikethru_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-font-strikethru", "IIF(.T., .T., .F.)"
    });
    expect(!stray_dynamic_font_strikethru_result.ok,
        "#1193: launch contract should reject stray dynamic-font-strikethru arguments");
}

void test_parse_launch_arguments_for_dynamic_font_outline_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--dynamic-font-outline-object",
        "--dynamic-font-outline", "IIF(.T., .T., .F.)",
        "--dynamic-font-outline-target-object-name", "txtName",
        "--dynamic-font-outline-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1194: launch contract should parse dynamic-font-outline-object requests");
    expect(result.request.dynamic_font_outline_object,
        "#1194: launch contract should detect --dynamic-font-outline-object");
    expect(result.request.dynamic_font_outline_available &&
            result.request.dynamic_font_outline == "IIF(.T., .T., .F.)",
        "#1194: dynamic-font-outline-object requests should carry raw expression text");
    expect(result.request.dynamic_font_outline_objects.size() == 2U,
        "#1194: dynamic-font-outline-object requests should collect dynamic-font-outline target selectors");
    if (result.request.dynamic_font_outline_objects.size() == 2U) {
        expect(result.request.dynamic_font_outline_objects[0].object_name == "txtName" &&
                result.request.dynamic_font_outline_objects[0].unique_id.empty(),
            "#1194: dynamic-font-outline-object requests should parse target object-name selectors");
        expect(result.request.dynamic_font_outline_objects[1].object_name.empty() &&
                result.request.dynamic_font_outline_objects[1].unique_id == "two-guid",
            "#1194: dynamic-font-outline-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_dynamic_font_outline_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-font-outline-object",
        "--dynamic-font-outline-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1194: launch contract should reject dynamic-font-outline-object requests without dynamic font outline");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-font-outline-object",
        "--dynamic-font-outline", "IIF(.T., .T., .F.)"
    });
    expect(!missing_targets_result.ok,
        "#1194: launch contract should reject dynamic-font-outline-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_dynamic_font_outline_object_ambiguity() {
    const auto dynamic_font_outline_allow_output_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-font-outline-object",
        "--allow-output-object",
        "--dynamic-font-outline", "IIF(.T., .T., .F.)",
        "--dynamic-font-outline-target-unique-id", "one-guid",
        "--allow-output", "false",
        "--allow-output-target-unique-id", "one-guid"
    });
    expect(!dynamic_font_outline_allow_output_result.ok,
        "#1194: launch contract should reject simultaneous dynamic-font-outline-object and allow-output-object requests");

    const auto dynamic_font_outline_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-font-outline-object",
        "--clear-property",
        "--property-name", "DynamicFontOutline",
        "--dynamic-font-outline", "IIF(.T., .T., .F.)",
        "--dynamic-font-outline-target-unique-id", "one-guid"
    });
    expect(!dynamic_font_outline_property_result.ok,
        "#1194: launch contract should reject dynamic-font-outline-object combined with property commands");

    const auto stray_dynamic_font_outline_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-font-outline", "IIF(.T., .T., .F.)"
    });
    expect(!stray_dynamic_font_outline_result.ok,
        "#1194: launch contract should reject stray dynamic-font-outline arguments");
}

void test_parse_launch_arguments_for_dynamic_font_shadow_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--dynamic-font-shadow-object",
        "--dynamic-font-shadow", "IIF(.T., .T., .F.)",
        "--dynamic-font-shadow-target-object-name", "txtName",
        "--dynamic-font-shadow-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1195: launch contract should parse dynamic-font-shadow-object requests");
    expect(result.request.dynamic_font_shadow_object,
        "#1195: launch contract should detect --dynamic-font-shadow-object");
    expect(result.request.dynamic_font_shadow_available &&
            result.request.dynamic_font_shadow == "IIF(.T., .T., .F.)",
        "#1195: dynamic-font-shadow-object requests should carry raw expression text");
    expect(result.request.dynamic_font_shadow_objects.size() == 2U,
        "#1195: dynamic-font-shadow-object requests should collect dynamic-font-shadow target selectors");
    if (result.request.dynamic_font_shadow_objects.size() == 2U) {
        expect(result.request.dynamic_font_shadow_objects[0].object_name == "txtName" &&
                result.request.dynamic_font_shadow_objects[0].unique_id.empty(),
            "#1195: dynamic-font-shadow-object requests should parse target object-name selectors");
        expect(result.request.dynamic_font_shadow_objects[1].object_name.empty() &&
                result.request.dynamic_font_shadow_objects[1].unique_id == "two-guid",
            "#1195: dynamic-font-shadow-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_dynamic_font_shadow_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-font-shadow-object",
        "--dynamic-font-shadow-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1195: launch contract should reject dynamic-font-shadow-object requests without dynamic font shadow");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-font-shadow-object",
        "--dynamic-font-shadow", "IIF(.T., .T., .F.)"
    });
    expect(!missing_targets_result.ok,
        "#1195: launch contract should reject dynamic-font-shadow-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_dynamic_font_shadow_object_ambiguity() {
    const auto dynamic_font_shadow_allow_output_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-font-shadow-object",
        "--allow-output-object",
        "--dynamic-font-shadow", "IIF(.T., .T., .F.)",
        "--dynamic-font-shadow-target-unique-id", "one-guid",
        "--allow-output", "false",
        "--allow-output-target-unique-id", "one-guid"
    });
    expect(!dynamic_font_shadow_allow_output_result.ok,
        "#1195: launch contract should reject simultaneous dynamic-font-shadow-object and allow-output-object requests");

    const auto dynamic_font_shadow_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-font-shadow-object",
        "--clear-property",
        "--property-name", "DynamicFontShadow",
        "--dynamic-font-shadow", "IIF(.T., .T., .F.)",
        "--dynamic-font-shadow-target-unique-id", "one-guid"
    });
    expect(!dynamic_font_shadow_property_result.ok,
        "#1195: launch contract should reject dynamic-font-shadow-object combined with property commands");

    const auto stray_dynamic_font_shadow_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-font-shadow", "IIF(.T., .T., .F.)"
    });
    expect(!stray_dynamic_font_shadow_result.ok,
        "#1195: launch contract should reject stray dynamic-font-shadow arguments");
}

void test_parse_launch_arguments_for_font_name_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--font-name-object",
        "--font-name", "Courier New",
        "--font-name-target-object-name", "txtName",
        "--font-name-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1178: launch contract should parse font-name-object requests");
    expect(result.request.font_name_object,
        "#1178: launch contract should detect --font-name-object");
    expect(result.request.font_name_available && result.request.font_name == "Courier New",
        "#1178: font-name-object requests should carry font name text");
    expect(result.request.font_name_objects.size() == 2U,
        "#1178: font-name-object requests should collect font-name target selectors");
    if (result.request.font_name_objects.size() == 2U) {
        expect(result.request.font_name_objects[0].object_name == "txtName" &&
                result.request.font_name_objects[0].unique_id.empty(),
            "#1178: font-name-object requests should parse target object-name selectors");
        expect(result.request.font_name_objects[1].object_name.empty() &&
                result.request.font_name_objects[1].unique_id == "two-guid",
            "#1178: font-name-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_font_name_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--font-name-object",
        "--font-name-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1178: launch contract should reject font-name-object requests without font name");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--font-name-object",
        "--font-name", "Courier New"
    });
    expect(!missing_targets_result.ok,
        "#1178: launch contract should reject font-name-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_font_name_object_ambiguity() {
    const auto font_name_allow_output_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--font-name-object",
        "--allow-output-object",
        "--font-name", "Courier New",
        "--font-name-target-unique-id", "one-guid",
        "--allow-output", "false",
        "--allow-output-target-unique-id", "one-guid"
    });
    expect(!font_name_allow_output_result.ok,
        "#1178: launch contract should reject simultaneous font-name-object and allow-output-object requests");

    const auto font_name_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--font-name-object",
        "--clear-property",
        "--property-name", "FontName",
        "--font-name", "Courier New",
        "--font-name-target-unique-id", "one-guid"
    });
    expect(!font_name_property_result.ok,
        "#1178: launch contract should reject font-name-object combined with property commands");

    const auto stray_font_name_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--font-name", "Courier New"
    });
    expect(!stray_font_name_result.ok,
        "#1178: launch contract should reject stray font-name arguments");
}

void test_parse_launch_arguments_for_font_size_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--font-size-object",
        "--font-size", "13.5",
        "--font-size-target-object-name", "txtName",
        "--font-size-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1179: launch contract should parse font-size-object requests");
    expect(result.request.font_size_object,
        "#1179: launch contract should detect --font-size-object");
    expect(result.request.font_size_available && result.request.font_size == 13.5,
        "#1179: font-size-object requests should carry numeric font size");
    expect(result.request.font_size_objects.size() == 2U,
        "#1179: font-size-object requests should collect font-size target selectors");
    if (result.request.font_size_objects.size() == 2U) {
        expect(result.request.font_size_objects[0].object_name == "txtName" &&
                result.request.font_size_objects[0].unique_id.empty(),
            "#1179: font-size-object requests should parse target object-name selectors");
        expect(result.request.font_size_objects[1].object_name.empty() &&
                result.request.font_size_objects[1].unique_id == "two-guid",
            "#1179: font-size-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_font_size_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--font-size-object",
        "--font-size-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1179: launch contract should reject font-size-object requests without font size");

    const auto non_numeric_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--font-size-object",
        "--font-size", "large",
        "--font-size-target-unique-id", "one-guid"
    });
    expect(!non_numeric_result.ok,
        "#1179: launch contract should reject non-numeric font-size values");

    const auto negative_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--font-size-object",
        "--font-size", "-1",
        "--font-size-target-unique-id", "one-guid"
    });
    expect(!negative_result.ok,
        "#1179: launch contract should reject negative font-size values before mutation");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--font-size-object",
        "--font-size", "13.5"
    });
    expect(!missing_targets_result.ok,
        "#1179: launch contract should reject font-size-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_font_size_object_ambiguity() {
    const auto font_size_allow_output_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--font-size-object",
        "--allow-output-object",
        "--font-size", "13.5",
        "--font-size-target-unique-id", "one-guid",
        "--allow-output", "false",
        "--allow-output-target-unique-id", "one-guid"
    });
    expect(!font_size_allow_output_result.ok,
        "#1179: launch contract should reject simultaneous font-size-object and allow-output-object requests");

    const auto font_size_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--font-size-object",
        "--clear-property",
        "--property-name", "FontSize",
        "--font-size", "13.5",
        "--font-size-target-unique-id", "one-guid"
    });
    expect(!font_size_property_result.ok,
        "#1179: launch contract should reject font-size-object combined with property commands");

    const auto stray_font_size_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--font-size", "13.5"
    });
    expect(!stray_font_size_result.ok,
        "#1179: launch contract should reject stray font-size arguments");
}

void test_parse_launch_arguments_for_font_bold_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--font-bold-object",
        "--font-bold", "true",
        "--font-bold-target-object-name", "txtName",
        "--font-bold-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1180: launch contract should parse font-bold-object requests");
    expect(result.request.font_bold_object,
        "#1180: launch contract should detect --font-bold-object");
    expect(result.request.font_bold_available && result.request.font_bold,
        "#1180: font-bold-object requests should carry font bold state");
    expect(result.request.font_bold_objects.size() == 2U,
        "#1180: font-bold-object requests should collect font-bold target selectors");
    if (result.request.font_bold_objects.size() == 2U) {
        expect(result.request.font_bold_objects[0].object_name == "txtName" &&
                result.request.font_bold_objects[0].unique_id.empty(),
            "#1180: font-bold-object requests should parse target object-name selectors");
        expect(result.request.font_bold_objects[1].object_name.empty() &&
                result.request.font_bold_objects[1].unique_id == "two-guid",
            "#1180: font-bold-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_font_bold_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--font-bold-object",
        "--font-bold-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1180: launch contract should reject font-bold-object requests without font bold state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--font-bold-object",
        "--font-bold", "true"
    });
    expect(!missing_targets_result.ok,
        "#1180: launch contract should reject font-bold-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--font-bold-object",
        "--font-bold", "sometimes",
        "--font-bold-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1180: launch contract should reject invalid font-bold boolean values");
}

void test_parse_launch_arguments_rejects_font_bold_object_ambiguity() {
    const auto font_bold_allow_output_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--font-bold-object",
        "--allow-output-object",
        "--font-bold", "true",
        "--font-bold-target-unique-id", "one-guid",
        "--allow-output", "false",
        "--allow-output-target-unique-id", "one-guid"
    });
    expect(!font_bold_allow_output_result.ok,
        "#1180: launch contract should reject simultaneous font-bold-object and allow-output-object requests");

    const auto font_bold_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--font-bold-object",
        "--clear-property",
        "--property-name", "FontBold",
        "--font-bold", "true",
        "--font-bold-target-unique-id", "one-guid"
    });
    expect(!font_bold_property_result.ok,
        "#1180: launch contract should reject font-bold-object combined with property commands");

    const auto stray_font_bold_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--font-bold", "true"
    });
    expect(!stray_font_bold_result.ok,
        "#1180: launch contract should reject stray font-bold arguments");
}

void test_parse_launch_arguments_for_font_italic_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--font-italic-object",
        "--font-italic", "true",
        "--font-italic-target-object-name", "txtName",
        "--font-italic-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1181: launch contract should parse font-italic-object requests");
    expect(result.request.font_italic_object,
        "#1181: launch contract should detect --font-italic-object");
    expect(result.request.font_italic_available && result.request.font_italic,
        "#1181: font-italic-object requests should carry font italic state");
    expect(result.request.font_italic_objects.size() == 2U,
        "#1181: font-italic-object requests should collect font-italic target selectors");
    if (result.request.font_italic_objects.size() == 2U) {
        expect(result.request.font_italic_objects[0].object_name == "txtName" &&
                result.request.font_italic_objects[0].unique_id.empty(),
            "#1181: font-italic-object requests should parse target object-name selectors");
        expect(result.request.font_italic_objects[1].object_name.empty() &&
                result.request.font_italic_objects[1].unique_id == "two-guid",
            "#1181: font-italic-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_font_italic_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--font-italic-object",
        "--font-italic-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1181: launch contract should reject font-italic-object requests without font italic state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--font-italic-object",
        "--font-italic", "true"
    });
    expect(!missing_targets_result.ok,
        "#1181: launch contract should reject font-italic-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--font-italic-object",
        "--font-italic", "sometimes",
        "--font-italic-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1181: launch contract should reject invalid font-italic boolean values");
}

void test_parse_launch_arguments_rejects_font_italic_object_ambiguity() {
    const auto font_italic_allow_output_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--font-italic-object",
        "--allow-output-object",
        "--font-italic", "true",
        "--font-italic-target-unique-id", "one-guid",
        "--allow-output", "false",
        "--allow-output-target-unique-id", "one-guid"
    });
    expect(!font_italic_allow_output_result.ok,
        "#1181: launch contract should reject simultaneous font-italic-object and allow-output-object requests");

    const auto font_italic_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--font-italic-object",
        "--clear-property",
        "--property-name", "FontItalic",
        "--font-italic", "true",
        "--font-italic-target-unique-id", "one-guid"
    });
    expect(!font_italic_property_result.ok,
        "#1181: launch contract should reject font-italic-object combined with property commands");

    const auto stray_font_italic_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--font-italic", "true"
    });
    expect(!stray_font_italic_result.ok,
        "#1181: launch contract should reject stray font-italic arguments");
}

void test_parse_launch_arguments_for_font_underline_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--font-underline-object",
        "--font-underline", "true",
        "--font-underline-target-object-name", "txtName",
        "--font-underline-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1182: launch contract should parse font-underline-object requests");
    expect(result.request.font_underline_object,
        "#1182: launch contract should detect --font-underline-object");
    expect(result.request.font_underline_available && result.request.font_underline,
        "#1182: font-underline-object requests should carry font underline state");
    expect(result.request.font_underline_objects.size() == 2U,
        "#1182: font-underline-object requests should collect font-underline target selectors");
    if (result.request.font_underline_objects.size() == 2U) {
        expect(result.request.font_underline_objects[0].object_name == "txtName" &&
                result.request.font_underline_objects[0].unique_id.empty(),
            "#1182: font-underline-object requests should parse target object-name selectors");
        expect(result.request.font_underline_objects[1].object_name.empty() &&
                result.request.font_underline_objects[1].unique_id == "two-guid",
            "#1182: font-underline-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_font_underline_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--font-underline-object",
        "--font-underline-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1182: launch contract should reject font-underline-object requests without font underline state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--font-underline-object",
        "--font-underline", "true"
    });
    expect(!missing_targets_result.ok,
        "#1182: launch contract should reject font-underline-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--font-underline-object",
        "--font-underline", "sometimes",
        "--font-underline-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1182: launch contract should reject invalid font-underline boolean values");
}

void test_parse_launch_arguments_rejects_font_underline_object_ambiguity() {
    const auto font_underline_allow_output_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--font-underline-object",
        "--allow-output-object",
        "--font-underline", "true",
        "--font-underline-target-unique-id", "one-guid",
        "--allow-output", "false",
        "--allow-output-target-unique-id", "one-guid"
    });
    expect(!font_underline_allow_output_result.ok,
        "#1182: launch contract should reject simultaneous font-underline-object and allow-output-object requests");

    const auto font_underline_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--font-underline-object",
        "--clear-property",
        "--property-name", "FontUnderline",
        "--font-underline", "true",
        "--font-underline-target-unique-id", "one-guid"
    });
    expect(!font_underline_property_result.ok,
        "#1182: launch contract should reject font-underline-object combined with property commands");

    const auto stray_font_underline_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--font-underline", "true"
    });
    expect(!stray_font_underline_result.ok,
        "#1182: launch contract should reject stray font-underline arguments");
}

void test_parse_launch_arguments_for_font_strikethru_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--font-strikethru-object",
        "--font-strikethru", "true",
        "--font-strikethru-target-object-name", "txtName",
        "--font-strikethru-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1183: launch contract should parse font-strikethru-object requests");
    expect(result.request.font_strikethru_object,
        "#1183: launch contract should detect --font-strikethru-object");
    expect(result.request.font_strikethru_available && result.request.font_strikethru,
        "#1183: font-strikethru-object requests should carry font strikethru state");
    expect(result.request.font_strikethru_objects.size() == 2U,
        "#1183: font-strikethru-object requests should collect font-strikethru target selectors");
    if (result.request.font_strikethru_objects.size() == 2U) {
        expect(result.request.font_strikethru_objects[0].object_name == "txtName" &&
                result.request.font_strikethru_objects[0].unique_id.empty(),
            "#1183: font-strikethru-object requests should parse target object-name selectors");
        expect(result.request.font_strikethru_objects[1].object_name.empty() &&
                result.request.font_strikethru_objects[1].unique_id == "two-guid",
            "#1183: font-strikethru-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_font_strikethru_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--font-strikethru-object",
        "--font-strikethru-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1183: launch contract should reject font-strikethru-object requests without font strikethru state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--font-strikethru-object",
        "--font-strikethru", "true"
    });
    expect(!missing_targets_result.ok,
        "#1183: launch contract should reject font-strikethru-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--font-strikethru-object",
        "--font-strikethru", "sometimes",
        "--font-strikethru-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1183: launch contract should reject invalid font-strikethru boolean values");
}

void test_parse_launch_arguments_rejects_font_strikethru_object_ambiguity() {
    const auto font_strikethru_allow_output_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--font-strikethru-object",
        "--allow-output-object",
        "--font-strikethru", "true",
        "--font-strikethru-target-unique-id", "one-guid",
        "--allow-output", "false",
        "--allow-output-target-unique-id", "one-guid"
    });
    expect(!font_strikethru_allow_output_result.ok,
        "#1183: launch contract should reject simultaneous font-strikethru-object and allow-output-object requests");

    const auto font_strikethru_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--font-strikethru-object",
        "--clear-property",
        "--property-name", "FontStrikethru",
        "--font-strikethru", "true",
        "--font-strikethru-target-unique-id", "one-guid"
    });
    expect(!font_strikethru_property_result.ok,
        "#1183: launch contract should reject font-strikethru-object combined with property commands");

    const auto stray_font_strikethru_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--font-strikethru", "true"
    });
    expect(!stray_font_strikethru_result.ok,
        "#1183: launch contract should reject stray font-strikethru arguments");
}

void test_parse_launch_arguments_for_font_outline_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--font-outline-object",
        "--font-outline", "true",
        "--font-outline-target-object-name", "txtName",
        "--font-outline-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1184: launch contract should parse font-outline-object requests");
    expect(result.request.font_outline_object,
        "#1184: launch contract should detect --font-outline-object");
    expect(result.request.font_outline_available && result.request.font_outline,
        "#1184: font-outline-object requests should carry font outline state");
    expect(result.request.font_outline_objects.size() == 2U,
        "#1184: font-outline-object requests should collect font-outline target selectors");
    if (result.request.font_outline_objects.size() == 2U) {
        expect(result.request.font_outline_objects[0].object_name == "txtName" &&
                result.request.font_outline_objects[0].unique_id.empty(),
            "#1184: font-outline-object requests should parse target object-name selectors");
        expect(result.request.font_outline_objects[1].object_name.empty() &&
                result.request.font_outline_objects[1].unique_id == "two-guid",
            "#1184: font-outline-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_font_outline_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--font-outline-object",
        "--font-outline-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1184: launch contract should reject font-outline-object requests without font outline state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--font-outline-object",
        "--font-outline", "true"
    });
    expect(!missing_targets_result.ok,
        "#1184: launch contract should reject font-outline-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--font-outline-object",
        "--font-outline", "sometimes",
        "--font-outline-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1184: launch contract should reject invalid font-outline boolean values");
}

void test_parse_launch_arguments_rejects_font_outline_object_ambiguity() {
    const auto font_outline_allow_output_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--font-outline-object",
        "--allow-output-object",
        "--font-outline", "true",
        "--font-outline-target-unique-id", "one-guid",
        "--allow-output", "false",
        "--allow-output-target-unique-id", "one-guid"
    });
    expect(!font_outline_allow_output_result.ok,
        "#1184: launch contract should reject simultaneous font-outline-object and allow-output-object requests");

    const auto font_outline_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--font-outline-object",
        "--clear-property",
        "--property-name", "FontOutline",
        "--font-outline", "true",
        "--font-outline-target-unique-id", "one-guid"
    });
    expect(!font_outline_property_result.ok,
        "#1184: launch contract should reject font-outline-object combined with property commands");

    const auto stray_font_outline_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--font-outline", "true"
    });
    expect(!stray_font_outline_result.ok,
        "#1184: launch contract should reject stray font-outline arguments");
}

void test_parse_launch_arguments_for_font_shadow_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--font-shadow-object",
        "--font-shadow", "true",
        "--font-shadow-target-object-name", "txtName",
        "--font-shadow-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1185: launch contract should parse font-shadow-object requests");
    expect(result.request.font_shadow_object,
        "#1185: launch contract should detect --font-shadow-object");
    expect(result.request.font_shadow_available && result.request.font_shadow,
        "#1185: font-shadow-object requests should carry font shadow state");
    expect(result.request.font_shadow_objects.size() == 2U,
        "#1185: font-shadow-object requests should collect font-shadow target selectors");
    if (result.request.font_shadow_objects.size() == 2U) {
        expect(result.request.font_shadow_objects[0].object_name == "txtName" &&
                result.request.font_shadow_objects[0].unique_id.empty(),
            "#1185: font-shadow-object requests should parse target object-name selectors");
        expect(result.request.font_shadow_objects[1].object_name.empty() &&
                result.request.font_shadow_objects[1].unique_id == "two-guid",
            "#1185: font-shadow-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_font_shadow_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--font-shadow-object",
        "--font-shadow-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1185: launch contract should reject font-shadow-object requests without font shadow state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--font-shadow-object",
        "--font-shadow", "true"
    });
    expect(!missing_targets_result.ok,
        "#1185: launch contract should reject font-shadow-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--font-shadow-object",
        "--font-shadow", "sometimes",
        "--font-shadow-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1185: launch contract should reject invalid font-shadow boolean values");
}

void test_parse_launch_arguments_rejects_font_shadow_object_ambiguity() {
    const auto font_shadow_allow_output_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--font-shadow-object",
        "--allow-output-object",
        "--font-shadow", "true",
        "--font-shadow-target-unique-id", "one-guid",
        "--allow-output", "false",
        "--allow-output-target-unique-id", "one-guid"
    });
    expect(!font_shadow_allow_output_result.ok,
        "#1185: launch contract should reject simultaneous font-shadow-object and allow-output-object requests");

    const auto font_shadow_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--font-shadow-object",
        "--clear-property",
        "--property-name", "FontShadow",
        "--font-shadow", "true",
        "--font-shadow-target-unique-id", "one-guid"
    });
    expect(!font_shadow_property_result.ok,
        "#1185: launch contract should reject font-shadow-object combined with property commands");

    const auto stray_font_shadow_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--font-shadow", "true"
    });
    expect(!stray_font_shadow_result.ok,
        "#1185: launch contract should reject stray font-shadow arguments");
}

}  // namespace cf_test_studio_host
