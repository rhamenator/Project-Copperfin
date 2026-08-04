// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_studio_host_support.h"

namespace cf_test_studio_host {
void test_parse_launch_arguments_for_scale_mode_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--scale-mode-object",
        "--scale-mode", "9",
        "--scale-mode-target-object-name", "cmdSave",
        "--scale-mode-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1116: launch contract should parse scale-mode-object requests");
    expect(result.request.scale_mode_object,
        "#1116: launch contract should detect --scale-mode-object");
    expect(result.request.scale_mode_available && result.request.scale_mode == 9,
        "#1116: scale-mode-object requests should carry scale-mode value");
    expect(result.request.scale_mode_objects.size() == 2U,
        "#1116: scale-mode-object requests should collect scale-mode target selectors");
    if (result.request.scale_mode_objects.size() == 2U) {
        expect(result.request.scale_mode_objects[0].object_name == "cmdSave" &&
                result.request.scale_mode_objects[0].unique_id.empty(),
            "#1116: scale-mode-object requests should parse target object-name selectors");
        expect(result.request.scale_mode_objects[1].object_name.empty() &&
                result.request.scale_mode_objects[1].unique_id == "two-guid",
            "#1116: scale-mode-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_scale_mode_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--scale-mode-object",
        "--scale-mode-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1116: launch contract should reject scale-mode-object requests without scale-mode value");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--scale-mode-object",
        "--scale-mode", "manual",
        "--scale-mode-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1116: launch contract should reject non-integer scale-mode values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--scale-mode-object",
        "--scale-mode", "-1",
        "--scale-mode-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1116: launch contract should reject negative scale-mode values before mutation");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--scale-mode-object",
        "--scale-mode", "2"
    });
    expect(!missing_targets_result.ok,
        "#1116: launch contract should reject scale-mode-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_scale_mode_object_ambiguity() {
    const auto scale_mode_locked_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--scale-mode-object",
        "--locked-object",
        "--scale-mode", "2",
        "--scale-mode-target-unique-id", "one-guid",
        "--locked", "true",
        "--locked-target-unique-id", "one-guid"
    });
    expect(!scale_mode_locked_result.ok,
        "#1116: launch contract should reject simultaneous scale-mode-object and locked-object requests");

    const auto scale_mode_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--scale-mode-object",
        "--clear-property",
        "--property-name", "ScaleMode",
        "--scale-mode", "2",
        "--scale-mode-target-unique-id", "one-guid"
    });
    expect(!scale_mode_property_result.ok,
        "#1116: launch contract should reject scale-mode-object combined with property commands");

    const auto stray_scale_mode_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--scale-mode", "2"
    });
    expect(!stray_scale_mode_result.ok,
        "#1116: launch contract should reject stray scale-mode arguments");
}

void test_parse_launch_arguments_for_header_height_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--header-height-object",
        "--header-height", "9",
        "--header-height-target-object-name", "cmdSave",
        "--header-height-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1121: launch contract should parse header-height-object requests");
    expect(result.request.header_height_object,
        "#1121: launch contract should detect --header-height-object");
    expect(result.request.header_height_available && result.request.header_height == 9,
        "#1121: header-height-object requests should carry header-height value");
    expect(result.request.header_height_objects.size() == 2U,
        "#1121: header-height-object requests should collect header-height target selectors");
    if (result.request.header_height_objects.size() == 2U) {
        expect(result.request.header_height_objects[0].object_name == "cmdSave" &&
                result.request.header_height_objects[0].unique_id.empty(),
            "#1121: header-height-object requests should parse target object-name selectors");
        expect(result.request.header_height_objects[1].object_name.empty() &&
                result.request.header_height_objects[1].unique_id == "two-guid",
            "#1121: header-height-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_header_height_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--header-height-object",
        "--header-height-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1121: launch contract should reject header-height-object requests without header-height value");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--header-height-object",
        "--header-height", "manual",
        "--header-height-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1121: launch contract should reject non-integer header-height values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--header-height-object",
        "--header-height", "-1",
        "--header-height-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1121: launch contract should reject negative header-height values before mutation");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--header-height-object",
        "--header-height", "2"
    });
    expect(!missing_targets_result.ok,
        "#1121: launch contract should reject header-height-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_header_height_object_ambiguity() {
    const auto header_height_locked_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--header-height-object",
        "--locked-object",
        "--header-height", "2",
        "--header-height-target-unique-id", "one-guid",
        "--locked", "true",
        "--locked-target-unique-id", "one-guid"
    });
    expect(!header_height_locked_result.ok,
        "#1121: launch contract should reject simultaneous header-height-object and locked-object requests");

    const auto header_height_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--header-height-object",
        "--clear-property",
        "--property-name", "HeaderHeight",
        "--header-height", "2",
        "--header-height-target-unique-id", "one-guid"
    });
    expect(!header_height_property_result.ok,
        "#1121: launch contract should reject header-height-object combined with property commands");

    const auto stray_header_height_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--header-height", "2"
    });
    expect(!stray_header_height_result.ok,
        "#1121: launch contract should reject stray header-height arguments");
}

void test_parse_launch_arguments_for_row_height_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--row-height-object",
        "--row-height", "9",
        "--row-height-target-object-name", "cmdSave",
        "--row-height-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1122: launch contract should parse row-height-object requests");
    expect(result.request.row_height_object,
        "#1122: launch contract should detect --row-height-object");
    expect(result.request.row_height_available && result.request.row_height == 9,
        "#1122: row-height-object requests should carry row-height value");
    expect(result.request.row_height_objects.size() == 2U,
        "#1122: row-height-object requests should collect row-height target selectors");
    if (result.request.row_height_objects.size() == 2U) {
        expect(result.request.row_height_objects[0].object_name == "cmdSave" &&
                result.request.row_height_objects[0].unique_id.empty(),
            "#1122: row-height-object requests should parse target object-name selectors");
        expect(result.request.row_height_objects[1].object_name.empty() &&
                result.request.row_height_objects[1].unique_id == "two-guid",
            "#1122: row-height-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_row_height_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--row-height-object",
        "--row-height-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1122: launch contract should reject row-height-object requests without row-height value");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--row-height-object",
        "--row-height", "manual",
        "--row-height-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1122: launch contract should reject non-integer row-height values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--row-height-object",
        "--row-height", "-1",
        "--row-height-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1122: launch contract should reject negative row-height values before mutation");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--row-height-object",
        "--row-height", "2"
    });
    expect(!missing_targets_result.ok,
        "#1122: launch contract should reject row-height-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_row_height_object_ambiguity() {
    const auto row_height_locked_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--row-height-object",
        "--locked-object",
        "--row-height", "2",
        "--row-height-target-unique-id", "one-guid",
        "--locked", "true",
        "--locked-target-unique-id", "one-guid"
    });
    expect(!row_height_locked_result.ok,
        "#1122: launch contract should reject simultaneous row-height-object and locked-object requests");

    const auto row_height_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--row-height-object",
        "--clear-property",
        "--property-name", "RowHeight",
        "--row-height", "2",
        "--row-height-target-unique-id", "one-guid"
    });
    expect(!row_height_property_result.ok,
        "#1122: launch contract should reject row-height-object combined with property commands");

    const auto stray_row_height_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--row-height", "2"
    });
    expect(!stray_row_height_result.ok,
        "#1122: launch contract should reject stray row-height arguments");
}

void test_parse_launch_arguments_for_highlight_row_line_width_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--highlight-row-line-width-object",
        "--highlight-row-line-width", "9",
        "--highlight-row-line-width-target-object-name", "cmdSave",
        "--highlight-row-line-width-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1127: launch contract should parse highlight-row-line-width-object requests");
    expect(result.request.highlight_row_line_width_object,
        "#1127: launch contract should detect --highlight-row-line-width-object");
    expect(result.request.highlight_row_line_width_available && result.request.highlight_row_line_width == 9,
        "#1127: highlight-row-line-width-object requests should carry highlight-row-line-width value");
    expect(result.request.highlight_row_line_width_objects.size() == 2U,
        "#1127: highlight-row-line-width-object requests should collect highlight-row-line-width target selectors");
    if (result.request.highlight_row_line_width_objects.size() == 2U) {
        expect(result.request.highlight_row_line_width_objects[0].object_name == "cmdSave" &&
                result.request.highlight_row_line_width_objects[0].unique_id.empty(),
            "#1127: highlight-row-line-width-object requests should parse target object-name selectors");
        expect(result.request.highlight_row_line_width_objects[1].object_name.empty() &&
                result.request.highlight_row_line_width_objects[1].unique_id == "two-guid",
            "#1127: highlight-row-line-width-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_highlight_row_line_width_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--highlight-row-line-width-object",
        "--highlight-row-line-width-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1127: launch contract should reject highlight-row-line-width-object requests without highlight-row-line-width value");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--highlight-row-line-width-object",
        "--highlight-row-line-width", "manual",
        "--highlight-row-line-width-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1127: launch contract should reject non-integer highlight-row-line-width values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--highlight-row-line-width-object",
        "--highlight-row-line-width", "-1",
        "--highlight-row-line-width-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1127: launch contract should reject negative highlight-row-line-width values before mutation");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--highlight-row-line-width-object",
        "--highlight-row-line-width", "2"
    });
    expect(!missing_targets_result.ok,
        "#1127: launch contract should reject highlight-row-line-width-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_highlight_row_line_width_object_ambiguity() {
    const auto highlight_row_line_width_locked_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--highlight-row-line-width-object",
        "--locked-object",
        "--highlight-row-line-width", "2",
        "--highlight-row-line-width-target-unique-id", "one-guid",
        "--locked", "true",
        "--locked-target-unique-id", "one-guid"
    });
    expect(!highlight_row_line_width_locked_result.ok,
        "#1127: launch contract should reject simultaneous highlight-row-line-width-object and locked-object requests");

    const auto highlight_row_line_width_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--highlight-row-line-width-object",
        "--clear-property",
        "--property-name", "HighlightRowLineWidth",
        "--highlight-row-line-width", "2",
        "--highlight-row-line-width-target-unique-id", "one-guid"
    });
    expect(!highlight_row_line_width_property_result.ok,
        "#1127: launch contract should reject highlight-row-line-width-object combined with property commands");

    const auto stray_highlight_row_line_width_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--highlight-row-line-width", "2"
    });
    expect(!stray_highlight_row_line_width_result.ok,
        "#1127: launch contract should reject stray highlight-row-line-width arguments");
}

void test_parse_launch_arguments_for_highlight_style_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--highlight-style-object",
        "--highlight-style", "9",
        "--highlight-style-target-object-name", "cmdSave",
        "--highlight-style-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1132: launch contract should parse highlight-style-object requests");
    expect(result.request.highlight_style_object,
        "#1132: launch contract should detect --highlight-style-object");
    expect(result.request.highlight_style_available && result.request.highlight_style == 9,
        "#1132: highlight-style-object requests should carry highlight-style value");
    expect(result.request.highlight_style_objects.size() == 2U,
        "#1132: highlight-style-object requests should collect highlight_style target selectors");
    if (result.request.highlight_style_objects.size() == 2U) {
        expect(result.request.highlight_style_objects[0].object_name == "cmdSave" &&
                result.request.highlight_style_objects[0].unique_id.empty(),
            "#1132: highlight-style-object requests should parse target object-name selectors");
        expect(result.request.highlight_style_objects[1].object_name.empty() &&
                result.request.highlight_style_objects[1].unique_id == "two-guid",
            "#1132: highlight-style-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_highlight_style_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--highlight-style-object",
        "--highlight-style-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1132: launch contract should reject highlight-style-object requests without highlight-style value");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--highlight-style-object",
        "--highlight-style", "manual",
        "--highlight-style-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1132: launch contract should reject non-integer highlight-style values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--highlight-style-object",
        "--highlight-style", "-1",
        "--highlight-style-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1132: launch contract should reject negative highlight-style values before mutation");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--highlight-style-object",
        "--highlight-style", "2"
    });
    expect(!missing_targets_result.ok,
        "#1132: launch contract should reject highlight-style-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_highlight_style_object_ambiguity() {
    const auto highlight_style_locked_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--highlight-style-object",
        "--locked-object",
        "--highlight-style", "2",
        "--highlight-style-target-unique-id", "one-guid",
        "--locked", "true",
        "--locked-target-unique-id", "one-guid"
    });
    expect(!highlight_style_locked_result.ok,
        "#1132: launch contract should reject simultaneous highlight-style-object and locked-object requests");

    const auto highlight_style_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--highlight-style-object",
        "--clear-property",
        "--property-name", "HighlightStyle",
        "--highlight-style", "2",
        "--highlight-style-target-unique-id", "one-guid"
    });
    expect(!highlight_style_property_result.ok,
        "#1132: launch contract should reject highlight-style-object combined with property commands");

    const auto stray_highlight_style_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--highlight-style", "2"
    });
    expect(!stray_highlight_style_result.ok,
        "#1132: launch contract should reject stray highlight-style arguments");
}

void test_parse_launch_arguments_for_input_mask_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--input-mask-object",
        "--input-mask", "999-99-9999",
        "--input-mask-target-object-name", "txtPhone",
        "--input-mask-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1046: launch contract should parse input-mask-object requests");
    expect(result.request.input_mask_object, "#1046: launch contract should detect --input-mask-object");
    expect(result.request.input_mask_available && result.request.input_mask == "999-99-9999",
        "#1046: input-mask-object requests should carry input mask text");
    expect(result.request.input_mask_objects.size() == 2U,
        "#1046: input-mask-object requests should collect input mask target selectors");
    if (result.request.input_mask_objects.size() == 2U) {
        expect(result.request.input_mask_objects[0].object_name == "txtPhone" &&
                result.request.input_mask_objects[0].unique_id.empty(),
            "#1046: input-mask-object requests should parse target object-name selectors");
        expect(result.request.input_mask_objects[1].object_name.empty() &&
                result.request.input_mask_objects[1].unique_id == "two-guid",
            "#1046: input-mask-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_input_mask_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--input-mask-object",
        "--input-mask-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1046: launch contract should reject input-mask-object requests without input mask text");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--input-mask-object",
        "--input-mask", "99999"
    });
    expect(!missing_targets_result.ok,
        "#1046: launch contract should reject input-mask-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_input_mask_object_ambiguity() {
    const auto input_control_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--input-mask-object",
        "--control-source-object",
        "--input-mask", "99999",
        "--input-mask-target-unique-id", "one-guid",
        "--control-source", "customers.name",
        "--control-source-target-unique-id", "one-guid"
    });
    expect(!input_control_result.ok,
        "#1046: launch contract should reject simultaneous input-mask-object and control-source-object requests");

    const auto input_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--input-mask-object",
        "--clear-property",
        "--property-name", "InputMask",
        "--input-mask", "99999",
        "--input-mask-target-unique-id", "one-guid"
    });
    expect(!input_property_result.ok,
        "#1046: launch contract should reject input-mask-object combined with property commands");

    const auto stray_input_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--input-mask", "99999"
    });
    expect(!stray_input_result.ok,
        "#1046: launch contract should reject stray input-mask arguments");
}

void test_parse_launch_arguments_for_scroll_bars_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--scroll-bars-object",
        "--scroll-bars", "2",
        "--scroll-bars-target-object-name", "frmCustomer",
        "--scroll-bars-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1167: launch contract should parse scroll-bars-object requests");
    expect(result.request.scroll_bars_object,
        "#1167: launch contract should detect --scroll-bars-object");
    expect(result.request.scroll_bars_available && result.request.scroll_bars == 2,
        "#1167: scroll-bars-object requests should carry scroll-bars value");
    expect(result.request.scroll_bars_objects.size() == 2U,
        "#1167: scroll-bars-object requests should collect scroll-bars target selectors");
    if (result.request.scroll_bars_objects.size() == 2U) {
        expect(result.request.scroll_bars_objects[0].object_name == "frmCustomer" &&
                result.request.scroll_bars_objects[0].unique_id.empty(),
            "#1167: scroll-bars-object requests should parse target object-name selectors");
        expect(result.request.scroll_bars_objects[1].object_name.empty() &&
                result.request.scroll_bars_objects[1].unique_id == "two-guid",
            "#1167: scroll-bars-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_scroll_bars_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--scroll-bars-object",
        "--scroll-bars-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1167: launch contract should reject scroll-bars-object requests without scroll-bars value");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--scroll-bars-object",
        "--scroll-bars", "2"
    });
    expect(!missing_targets_result.ok,
        "#1167: launch contract should reject scroll-bars-object requests without target selectors");

    const auto non_integer_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--scroll-bars-object",
        "--scroll-bars", "vertical",
        "--scroll-bars-target-unique-id", "one-guid"
    });
    expect(!non_integer_result.ok,
        "#1167: launch contract should reject non-integer scroll-bars values");

    const auto negative_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--scroll-bars-object",
        "--scroll-bars", "-1",
        "--scroll-bars-target-unique-id", "one-guid"
    });
    expect(!negative_result.ok,
        "#1167: launch contract should reject negative scroll-bars values");
}

void test_parse_launch_arguments_rejects_scroll_bars_object_ambiguity() {
    const auto scroll_bars_allow_output_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--scroll-bars-object",
        "--allow-output-object",
        "--scroll-bars", "2",
        "--scroll-bars-target-unique-id", "one-guid",
        "--allow-output", "false",
        "--allow-output-target-unique-id", "one-guid"
    });
    expect(!scroll_bars_allow_output_result.ok,
        "#1167: launch contract should reject simultaneous scroll-bars-object and allow-output-object requests");

    const auto scroll_bars_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--scroll-bars-object",
        "--clear-property",
        "--property-name", "ScrollBars",
        "--scroll-bars", "2",
        "--scroll-bars-target-unique-id", "one-guid"
    });
    expect(!scroll_bars_property_result.ok,
        "#1167: launch contract should reject scroll-bars-object combined with property commands");

    const auto stray_scroll_bars_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--scroll-bars", "2"
    });
    expect(!stray_scroll_bars_result.ok,
        "#1167: launch contract should reject stray scroll-bars arguments");
}

void test_parse_launch_arguments_for_window_state_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--window-state-object",
        "--window-state", "2",
        "--window-state-target-object-name", "frmCustomer",
        "--window-state-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1168: launch contract should parse window-state-object requests");
    expect(result.request.window_state_object,
        "#1168: launch contract should detect --window-state-object");
    expect(result.request.window_state_available && result.request.window_state == 2,
        "#1168: window-state-object requests should carry window-state value");
    expect(result.request.window_state_objects.size() == 2U,
        "#1168: window-state-object requests should collect window-state target selectors");
    if (result.request.window_state_objects.size() == 2U) {
        expect(result.request.window_state_objects[0].object_name == "frmCustomer" &&
                result.request.window_state_objects[0].unique_id.empty(),
            "#1168: window-state-object requests should parse target object-name selectors");
        expect(result.request.window_state_objects[1].object_name.empty() &&
                result.request.window_state_objects[1].unique_id == "two-guid",
            "#1168: window-state-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_window_state_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--window-state-object",
        "--window-state-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1168: launch contract should reject window-state-object requests without window-state value");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--window-state-object",
        "--window-state", "2"
    });
    expect(!missing_targets_result.ok,
        "#1168: launch contract should reject window-state-object requests without target selectors");

    const auto non_integer_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--window-state-object",
        "--window-state", "maximized",
        "--window-state-target-unique-id", "one-guid"
    });
    expect(!non_integer_result.ok,
        "#1168: launch contract should reject non-integer window-state values");

    const auto negative_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--window-state-object",
        "--window-state", "-1",
        "--window-state-target-unique-id", "one-guid"
    });
    expect(!negative_result.ok,
        "#1168: launch contract should reject negative window-state values");
}

void test_parse_launch_arguments_rejects_window_state_object_ambiguity() {
    const auto window_state_allow_output_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--window-state-object",
        "--allow-output-object",
        "--window-state", "2",
        "--window-state-target-unique-id", "one-guid",
        "--allow-output", "false",
        "--allow-output-target-unique-id", "one-guid"
    });
    expect(!window_state_allow_output_result.ok,
        "#1168: launch contract should reject simultaneous window-state-object and allow-output-object requests");

    const auto window_state_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--window-state-object",
        "--clear-property",
        "--property-name", "WindowState",
        "--window-state", "2",
        "--window-state-target-unique-id", "one-guid"
    });
    expect(!window_state_property_result.ok,
        "#1168: launch contract should reject window-state-object combined with property commands");

    const auto stray_window_state_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--window-state", "2"
    });
    expect(!stray_window_state_result.ok,
        "#1168: launch contract should reject stray window-state arguments");
}

void test_parse_launch_arguments_for_show_window_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--show-window-object",
        "--show-window", "2",
        "--show-window-target-object-name", "frmCustomer",
        "--show-window-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1169: launch contract should parse show-window-object requests");
    expect(result.request.show_window_object,
        "#1169: launch contract should detect --show-window-object");
    expect(result.request.show_window_available && result.request.show_window == 2,
        "#1169: show-window-object requests should carry show-window value");
    expect(result.request.show_window_objects.size() == 2U,
        "#1169: show-window-object requests should collect show-window target selectors");
    if (result.request.show_window_objects.size() == 2U) {
        expect(result.request.show_window_objects[0].object_name == "frmCustomer" &&
                result.request.show_window_objects[0].unique_id.empty(),
            "#1169: show-window-object requests should parse target object-name selectors");
        expect(result.request.show_window_objects[1].object_name.empty() &&
                result.request.show_window_objects[1].unique_id == "two-guid",
            "#1169: show-window-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_show_window_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--show-window-object",
        "--show-window-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1169: launch contract should reject show-window-object requests without show-window value");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--show-window-object",
        "--show-window", "2"
    });
    expect(!missing_targets_result.ok,
        "#1169: launch contract should reject show-window-object requests without target selectors");

    const auto non_integer_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--show-window-object",
        "--show-window", "top-level",
        "--show-window-target-unique-id", "one-guid"
    });
    expect(!non_integer_result.ok,
        "#1169: launch contract should reject non-integer show-window values");

    const auto negative_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--show-window-object",
        "--show-window", "-1",
        "--show-window-target-unique-id", "one-guid"
    });
    expect(!negative_result.ok,
        "#1169: launch contract should reject negative show-window values");
}

void test_parse_launch_arguments_rejects_show_window_object_ambiguity() {
    const auto show_window_allow_output_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--show-window-object",
        "--allow-output-object",
        "--show-window", "2",
        "--show-window-target-unique-id", "one-guid",
        "--allow-output", "false",
        "--allow-output-target-unique-id", "one-guid"
    });
    expect(!show_window_allow_output_result.ok,
        "#1169: launch contract should reject simultaneous show-window-object and allow-output-object requests");

    const auto show_window_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--show-window-object",
        "--clear-property",
        "--property-name", "ShowWindow",
        "--show-window", "2",
        "--show-window-target-unique-id", "one-guid"
    });
    expect(!show_window_property_result.ok,
        "#1169: launch contract should reject show-window-object combined with property commands");

    const auto stray_show_window_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--show-window", "2"
    });
    expect(!stray_show_window_result.ok,
        "#1169: launch contract should reject stray show-window arguments");
}

void test_parse_launch_arguments_for_title_bar_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--title-bar-object",
        "--title-bar", "2",
        "--title-bar-target-object-name", "frmCustomer",
        "--title-bar-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1170: launch contract should parse title-bar-object requests");
    expect(result.request.title_bar_object,
        "#1170: launch contract should detect --title-bar-object");
    expect(result.request.title_bar_available && result.request.title_bar == 2,
        "#1170: title-bar-object requests should carry title-bar value");
    expect(result.request.title_bar_objects.size() == 2U,
        "#1170: title-bar-object requests should collect title-bar target selectors");
    if (result.request.title_bar_objects.size() == 2U) {
        expect(result.request.title_bar_objects[0].object_name == "frmCustomer" &&
                result.request.title_bar_objects[0].unique_id.empty(),
            "#1170: title-bar-object requests should parse target object-name selectors");
        expect(result.request.title_bar_objects[1].object_name.empty() &&
                result.request.title_bar_objects[1].unique_id == "two-guid",
            "#1170: title-bar-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_title_bar_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--title-bar-object",
        "--title-bar-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1170: launch contract should reject title-bar-object requests without title-bar value");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--title-bar-object",
        "--title-bar", "2"
    });
    expect(!missing_targets_result.ok,
        "#1170: launch contract should reject title-bar-object requests without target selectors");

    const auto non_integer_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--title-bar-object",
        "--title-bar", "captioned",
        "--title-bar-target-unique-id", "one-guid"
    });
    expect(!non_integer_result.ok,
        "#1170: launch contract should reject non-integer title-bar values");

    const auto negative_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--title-bar-object",
        "--title-bar", "-1",
        "--title-bar-target-unique-id", "one-guid"
    });
    expect(!negative_result.ok,
        "#1170: launch contract should reject negative title-bar values");
}

void test_parse_launch_arguments_rejects_title_bar_object_ambiguity() {
    const auto title_bar_allow_output_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--title-bar-object",
        "--allow-output-object",
        "--title-bar", "2",
        "--title-bar-target-unique-id", "one-guid",
        "--allow-output", "false",
        "--allow-output-target-unique-id", "one-guid"
    });
    expect(!title_bar_allow_output_result.ok,
        "#1170: launch contract should reject simultaneous title-bar-object and allow-output-object requests");

    const auto title_bar_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--title-bar-object",
        "--clear-property",
        "--property-name", "TitleBar",
        "--title-bar", "2",
        "--title-bar-target-unique-id", "one-guid"
    });
    expect(!title_bar_property_result.ok,
        "#1170: launch contract should reject title-bar-object combined with property commands");

    const auto stray_title_bar_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--title-bar", "2"
    });
    expect(!stray_title_bar_result.ok,
        "#1170: launch contract should reject stray title-bar arguments");
}

void test_parse_launch_arguments_for_mouse_pointer_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--mouse-pointer-object",
        "--mouse-pointer", "2",
        "--mouse-pointer-target-object-name", "frmCustomer",
        "--mouse-pointer-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1171: launch contract should parse mouse-pointer-object requests");
    expect(result.request.mouse_pointer_object,
        "#1171: launch contract should detect --mouse-pointer-object");
    expect(result.request.mouse_pointer_available && result.request.mouse_pointer == 2,
        "#1171: mouse-pointer-object requests should carry mouse-pointer value");
    expect(result.request.mouse_pointer_objects.size() == 2U,
        "#1171: mouse-pointer-object requests should collect mouse-pointer target selectors");
    if (result.request.mouse_pointer_objects.size() == 2U) {
        expect(result.request.mouse_pointer_objects[0].object_name == "frmCustomer" &&
                result.request.mouse_pointer_objects[0].unique_id.empty(),
            "#1171: mouse-pointer-object requests should parse target object-name selectors");
        expect(result.request.mouse_pointer_objects[1].object_name.empty() &&
                result.request.mouse_pointer_objects[1].unique_id == "two-guid",
            "#1171: mouse-pointer-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_mouse_pointer_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--mouse-pointer-object",
        "--mouse-pointer-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1171: launch contract should reject mouse-pointer-object requests without mouse-pointer value");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--mouse-pointer-object",
        "--mouse-pointer", "2"
    });
    expect(!missing_targets_result.ok,
        "#1171: launch contract should reject mouse-pointer-object requests without target selectors");

    const auto non_integer_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--mouse-pointer-object",
        "--mouse-pointer", "arrow",
        "--mouse-pointer-target-unique-id", "one-guid"
    });
    expect(!non_integer_result.ok,
        "#1171: launch contract should reject non-integer mouse-pointer values");

    const auto negative_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--mouse-pointer-object",
        "--mouse-pointer", "-1",
        "--mouse-pointer-target-unique-id", "one-guid"
    });
    expect(!negative_result.ok,
        "#1171: launch contract should reject negative mouse-pointer values");
}

void test_parse_launch_arguments_rejects_mouse_pointer_object_ambiguity() {
    const auto mouse_pointer_allow_output_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--mouse-pointer-object",
        "--allow-output-object",
        "--mouse-pointer", "2",
        "--mouse-pointer-target-unique-id", "one-guid",
        "--allow-output", "false",
        "--allow-output-target-unique-id", "one-guid"
    });
    expect(!mouse_pointer_allow_output_result.ok,
        "#1171: launch contract should reject simultaneous mouse-pointer-object and allow-output-object requests");

    const auto mouse_pointer_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--mouse-pointer-object",
        "--clear-property",
        "--property-name", "MousePointer",
        "--mouse-pointer", "2",
        "--mouse-pointer-target-unique-id", "one-guid"
    });
    expect(!mouse_pointer_property_result.ok,
        "#1171: launch contract should reject mouse-pointer-object combined with property commands");

    const auto stray_mouse_pointer_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--mouse-pointer", "2"
    });
    expect(!stray_mouse_pointer_result.ok,
        "#1171: launch contract should reject stray mouse-pointer arguments");
}

void test_parse_launch_arguments_for_dynamic_input_mask_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--dynamic-input-mask-object",
        "--dynamic-input-mask", "IIF(.T., '999-99-9999', '')",
        "--dynamic-input-mask-target-object-name", "txtSsn",
        "--dynamic-input-mask-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1176: launch contract should parse dynamic-input-mask-object requests");
    expect(result.request.dynamic_input_mask_object,
        "#1176: launch contract should detect --dynamic-input-mask-object");
    expect(result.request.dynamic_input_mask_available &&
            result.request.dynamic_input_mask == "IIF(.T., '999-99-9999', '')",
        "#1176: dynamic-input-mask-object requests should carry raw expression text");
    expect(result.request.dynamic_input_mask_objects.size() == 2U,
        "#1176: dynamic-input-mask-object requests should collect dynamic-input-mask target selectors");
    if (result.request.dynamic_input_mask_objects.size() == 2U) {
        expect(result.request.dynamic_input_mask_objects[0].object_name == "txtSsn" &&
                result.request.dynamic_input_mask_objects[0].unique_id.empty(),
            "#1176: dynamic-input-mask-object requests should parse target object-name selectors");
        expect(result.request.dynamic_input_mask_objects[1].object_name.empty() &&
                result.request.dynamic_input_mask_objects[1].unique_id == "two-guid",
            "#1176: dynamic-input-mask-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_dynamic_input_mask_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-input-mask-object",
        "--dynamic-input-mask-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1176: launch contract should reject dynamic-input-mask-object requests without dynamic input mask");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-input-mask-object",
        "--dynamic-input-mask", "IIF(.T., '999', '')"
    });
    expect(!missing_targets_result.ok,
        "#1176: launch contract should reject dynamic-input-mask-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_dynamic_input_mask_object_ambiguity() {
    const auto dynamic_input_mask_allow_output_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-input-mask-object",
        "--allow-output-object",
        "--dynamic-input-mask", "IIF(.T., '999', '')",
        "--dynamic-input-mask-target-unique-id", "one-guid",
        "--allow-output", "false",
        "--allow-output-target-unique-id", "one-guid"
    });
    expect(!dynamic_input_mask_allow_output_result.ok,
        "#1176: launch contract should reject simultaneous dynamic-input-mask-object and allow-output-object requests");

    const auto dynamic_input_mask_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-input-mask-object",
        "--clear-property",
        "--property-name", "DynamicInputMask",
        "--dynamic-input-mask", "IIF(.T., '999', '')",
        "--dynamic-input-mask-target-unique-id", "one-guid"
    });
    expect(!dynamic_input_mask_property_result.ok,
        "#1176: launch contract should reject dynamic-input-mask-object combined with property commands");

    const auto stray_dynamic_input_mask_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-input-mask", "IIF(.T., '999', '')"
    });
    expect(!stray_dynamic_input_mask_result.ok,
        "#1176: launch contract should reject stray dynamic-input-mask arguments");
}

void test_parse_launch_arguments_for_dynamic_current_control_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--dynamic-current-control-object",
        "--dynamic-current-control", "IIF(.T., 'txtMemo', 'txtNotes')",
        "--dynamic-current-control-target-object-name", "grdOrders",
        "--dynamic-current-control-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1187: launch contract should parse dynamic-current-control-object requests");
    expect(result.request.dynamic_current_control_object,
        "#1187: launch contract should detect --dynamic-current-control-object");
    expect(result.request.dynamic_current_control_available &&
            result.request.dynamic_current_control == "IIF(.T., 'txtMemo', 'txtNotes')",
        "#1187: dynamic-current-control-object requests should carry raw expression text");
    expect(result.request.dynamic_current_control_objects.size() == 2U,
        "#1187: dynamic-current-control-object requests should collect dynamic-current-control target selectors");
    if (result.request.dynamic_current_control_objects.size() == 2U) {
        expect(result.request.dynamic_current_control_objects[0].object_name == "grdOrders" &&
                result.request.dynamic_current_control_objects[0].unique_id.empty(),
            "#1187: dynamic-current-control-object requests should parse target object-name selectors");
        expect(result.request.dynamic_current_control_objects[1].object_name.empty() &&
                result.request.dynamic_current_control_objects[1].unique_id == "two-guid",
            "#1187: dynamic-current-control-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_dynamic_current_control_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-current-control-object",
        "--dynamic-current-control-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1187: launch contract should reject dynamic-current-control-object requests without dynamic current control");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-current-control-object",
        "--dynamic-current-control", "IIF(.T., 'txtMemo', 'txtNotes')"
    });
    expect(!missing_targets_result.ok,
        "#1187: launch contract should reject dynamic-current-control-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_dynamic_current_control_object_ambiguity() {
    const auto dynamic_current_control_allow_output_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-current-control-object",
        "--allow-output-object",
        "--dynamic-current-control", "IIF(.T., 'txtMemo', 'txtNotes')",
        "--dynamic-current-control-target-unique-id", "one-guid",
        "--allow-output", "false",
        "--allow-output-target-unique-id", "one-guid"
    });
    expect(!dynamic_current_control_allow_output_result.ok,
        "#1187: launch contract should reject simultaneous dynamic-current-control-object and allow-output-object requests");

    const auto dynamic_current_control_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-current-control-object",
        "--clear-property",
        "--property-name", "DynamicCurrentControl",
        "--dynamic-current-control", "IIF(.T., 'txtMemo', 'txtNotes')",
        "--dynamic-current-control-target-unique-id", "one-guid"
    });
    expect(!dynamic_current_control_property_result.ok,
        "#1187: launch contract should reject dynamic-current-control-object combined with property commands");

    const auto stray_dynamic_current_control_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-current-control", "IIF(.T., 'txtMemo', 'txtNotes')"
    });
    expect(!stray_dynamic_current_control_result.ok,
        "#1187: launch contract should reject stray dynamic-current-control arguments");
}

}  // namespace cf_test_studio_host
