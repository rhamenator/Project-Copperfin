// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_studio_host_support.h"

namespace cf_test_studio_host {
void test_parse_launch_arguments_for_curvature_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--curvature-object",
        "--curvature", "4",
        "--curvature-target-object-name", "cmdSave",
        "--curvature-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1111: launch contract should parse curvature-object requests");
    expect(result.request.curvature_object,
        "#1111: launch contract should detect --curvature-object");
    expect(result.request.curvature_available && result.request.curvature == 4,
        "#1111: curvature-object requests should carry curvature value");
    expect(result.request.curvature_objects.size() == 2U,
        "#1111: curvature-object requests should collect curvature target selectors");
    if (result.request.curvature_objects.size() == 2U) {
        expect(result.request.curvature_objects[0].object_name == "cmdSave" &&
                result.request.curvature_objects[0].unique_id.empty(),
            "#1111: curvature-object requests should parse target object-name selectors");
        expect(result.request.curvature_objects[1].object_name.empty() &&
                result.request.curvature_objects[1].unique_id == "two-guid",
            "#1111: curvature-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_curvature_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--curvature-object",
        "--curvature-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1111: launch contract should reject curvature-object requests without curvature value");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--curvature-object",
        "--curvature", "manual",
        "--curvature-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1111: launch contract should reject non-integer curvature values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--curvature-object",
        "--curvature", "-1",
        "--curvature-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1111: launch contract should reject negative curvature values before mutation");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--curvature-object",
        "--curvature", "2"
    });
    expect(!missing_targets_result.ok,
        "#1111: launch contract should reject curvature-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_curvature_object_ambiguity() {
    const auto curvature_locked_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--curvature-object",
        "--locked-object",
        "--curvature", "2",
        "--curvature-target-unique-id", "one-guid",
        "--locked", "true",
        "--locked-target-unique-id", "one-guid"
    });
    expect(!curvature_locked_result.ok,
        "#1111: launch contract should reject simultaneous curvature-object and locked-object requests");

    const auto curvature_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--curvature-object",
        "--clear-property",
        "--property-name", "Curvature",
        "--curvature", "2",
        "--curvature-target-unique-id", "one-guid"
    });
    expect(!curvature_property_result.ok,
        "#1111: launch contract should reject curvature-object combined with property commands");

    const auto stray_curvature_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--curvature", "2"
    });
    expect(!stray_curvature_result.ok,
        "#1111: launch contract should reject stray curvature arguments");
}

void test_parse_launch_arguments_for_draw_mode_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--draw-mode-object",
        "--draw-mode", "5",
        "--draw-mode-target-object-name", "cmdSave",
        "--draw-mode-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1112: launch contract should parse draw-mode-object requests");
    expect(result.request.draw_mode_object,
        "#1112: launch contract should detect --draw-mode-object");
    expect(result.request.draw_mode_available && result.request.draw_mode == 5,
        "#1112: draw-mode-object requests should carry draw-mode value");
    expect(result.request.draw_mode_objects.size() == 2U,
        "#1112: draw-mode-object requests should collect draw-mode target selectors");
    if (result.request.draw_mode_objects.size() == 2U) {
        expect(result.request.draw_mode_objects[0].object_name == "cmdSave" &&
                result.request.draw_mode_objects[0].unique_id.empty(),
            "#1112: draw-mode-object requests should parse target object-name selectors");
        expect(result.request.draw_mode_objects[1].object_name.empty() &&
                result.request.draw_mode_objects[1].unique_id == "two-guid",
            "#1112: draw-mode-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_draw_mode_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--draw-mode-object",
        "--draw-mode-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1112: launch contract should reject draw-mode-object requests without draw-mode value");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--draw-mode-object",
        "--draw-mode", "manual",
        "--draw-mode-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1112: launch contract should reject non-integer draw-mode values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--draw-mode-object",
        "--draw-mode", "-1",
        "--draw-mode-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1112: launch contract should reject negative draw-mode values before mutation");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--draw-mode-object",
        "--draw-mode", "2"
    });
    expect(!missing_targets_result.ok,
        "#1112: launch contract should reject draw-mode-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_draw_mode_object_ambiguity() {
    const auto draw_mode_locked_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--draw-mode-object",
        "--locked-object",
        "--draw-mode", "2",
        "--draw-mode-target-unique-id", "one-guid",
        "--locked", "true",
        "--locked-target-unique-id", "one-guid"
    });
    expect(!draw_mode_locked_result.ok,
        "#1112: launch contract should reject simultaneous draw-mode-object and locked-object requests");

    const auto draw_mode_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--draw-mode-object",
        "--clear-property",
        "--property-name", "DrawMode",
        "--draw-mode", "2",
        "--draw-mode-target-unique-id", "one-guid"
    });
    expect(!draw_mode_property_result.ok,
        "#1112: launch contract should reject draw-mode-object combined with property commands");

    const auto stray_draw_mode_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--draw-mode", "2"
    });
    expect(!stray_draw_mode_result.ok,
        "#1112: launch contract should reject stray draw-mode arguments");
}

void test_parse_launch_arguments_for_draw_style_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--draw-style-object",
        "--draw-style", "6",
        "--draw-style-target-object-name", "cmdSave",
        "--draw-style-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1113: launch contract should parse draw-style-object requests");
    expect(result.request.draw_style_object,
        "#1113: launch contract should detect --draw-style-object");
    expect(result.request.draw_style_available && result.request.draw_style == 6,
        "#1113: draw-style-object requests should carry draw-style value");
    expect(result.request.draw_style_objects.size() == 2U,
        "#1113: draw-style-object requests should collect draw-style target selectors");
    if (result.request.draw_style_objects.size() == 2U) {
        expect(result.request.draw_style_objects[0].object_name == "cmdSave" &&
                result.request.draw_style_objects[0].unique_id.empty(),
            "#1113: draw-style-object requests should parse target object-name selectors");
        expect(result.request.draw_style_objects[1].object_name.empty() &&
                result.request.draw_style_objects[1].unique_id == "two-guid",
            "#1113: draw-style-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_draw_style_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--draw-style-object",
        "--draw-style-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1113: launch contract should reject draw-style-object requests without draw-style value");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--draw-style-object",
        "--draw-style", "manual",
        "--draw-style-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1113: launch contract should reject non-integer draw-style values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--draw-style-object",
        "--draw-style", "-1",
        "--draw-style-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1113: launch contract should reject negative draw-style values before mutation");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--draw-style-object",
        "--draw-style", "2"
    });
    expect(!missing_targets_result.ok,
        "#1113: launch contract should reject draw-style-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_draw_style_object_ambiguity() {
    const auto draw_style_locked_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--draw-style-object",
        "--locked-object",
        "--draw-style", "2",
        "--draw-style-target-unique-id", "one-guid",
        "--locked", "true",
        "--locked-target-unique-id", "one-guid"
    });
    expect(!draw_style_locked_result.ok,
        "#1113: launch contract should reject simultaneous draw-style-object and locked-object requests");

    const auto draw_style_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--draw-style-object",
        "--clear-property",
        "--property-name", "DrawStyle",
        "--draw-style", "2",
        "--draw-style-target-unique-id", "one-guid"
    });
    expect(!draw_style_property_result.ok,
        "#1113: launch contract should reject draw-style-object combined with property commands");

    const auto stray_draw_style_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--draw-style", "2"
    });
    expect(!stray_draw_style_result.ok,
        "#1113: launch contract should reject stray draw-style arguments");
}

void test_parse_launch_arguments_for_draw_width_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--draw-width-object",
        "--draw-width", "7",
        "--draw-width-target-object-name", "cmdSave",
        "--draw-width-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1114: launch contract should parse draw-width-object requests");
    expect(result.request.draw_width_object,
        "#1114: launch contract should detect --draw-width-object");
    expect(result.request.draw_width_available && result.request.draw_width == 7,
        "#1114: draw-width-object requests should carry draw-width value");
    expect(result.request.draw_width_objects.size() == 2U,
        "#1114: draw-width-object requests should collect draw-width target selectors");
    if (result.request.draw_width_objects.size() == 2U) {
        expect(result.request.draw_width_objects[0].object_name == "cmdSave" &&
                result.request.draw_width_objects[0].unique_id.empty(),
            "#1114: draw-width-object requests should parse target object-name selectors");
        expect(result.request.draw_width_objects[1].object_name.empty() &&
                result.request.draw_width_objects[1].unique_id == "two-guid",
            "#1114: draw-width-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_draw_width_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--draw-width-object",
        "--draw-width-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1114: launch contract should reject draw-width-object requests without draw-width value");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--draw-width-object",
        "--draw-width", "manual",
        "--draw-width-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1114: launch contract should reject non-integer draw-width values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--draw-width-object",
        "--draw-width", "-1",
        "--draw-width-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1114: launch contract should reject negative draw-width values before mutation");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--draw-width-object",
        "--draw-width", "2"
    });
    expect(!missing_targets_result.ok,
        "#1114: launch contract should reject draw-width-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_draw_width_object_ambiguity() {
    const auto draw_width_locked_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--draw-width-object",
        "--locked-object",
        "--draw-width", "2",
        "--draw-width-target-unique-id", "one-guid",
        "--locked", "true",
        "--locked-target-unique-id", "one-guid"
    });
    expect(!draw_width_locked_result.ok,
        "#1114: launch contract should reject simultaneous draw-width-object and locked-object requests");

    const auto draw_width_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--draw-width-object",
        "--clear-property",
        "--property-name", "DrawWidth",
        "--draw-width", "2",
        "--draw-width-target-unique-id", "one-guid"
    });
    expect(!draw_width_property_result.ok,
        "#1114: launch contract should reject draw-width-object combined with property commands");

    const auto stray_draw_width_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--draw-width", "2"
    });
    expect(!stray_draw_width_result.ok,
        "#1114: launch contract should reject stray draw-width arguments");
}

void test_parse_launch_arguments_for_fill_style_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--fill-style-object",
        "--fill-style", "8",
        "--fill-style-target-object-name", "cmdSave",
        "--fill-style-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1115: launch contract should parse fill-style-object requests");
    expect(result.request.fill_style_object,
        "#1115: launch contract should detect --fill-style-object");
    expect(result.request.fill_style_available && result.request.fill_style == 8,
        "#1115: fill-style-object requests should carry fill-style value");
    expect(result.request.fill_style_objects.size() == 2U,
        "#1115: fill-style-object requests should collect fill-style target selectors");
    if (result.request.fill_style_objects.size() == 2U) {
        expect(result.request.fill_style_objects[0].object_name == "cmdSave" &&
                result.request.fill_style_objects[0].unique_id.empty(),
            "#1115: fill-style-object requests should parse target object-name selectors");
        expect(result.request.fill_style_objects[1].object_name.empty() &&
                result.request.fill_style_objects[1].unique_id == "two-guid",
            "#1115: fill-style-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_fill_style_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--fill-style-object",
        "--fill-style-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1115: launch contract should reject fill-style-object requests without fill-style value");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--fill-style-object",
        "--fill-style", "manual",
        "--fill-style-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1115: launch contract should reject non-integer fill-style values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--fill-style-object",
        "--fill-style", "-1",
        "--fill-style-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1115: launch contract should reject negative fill-style values before mutation");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--fill-style-object",
        "--fill-style", "2"
    });
    expect(!missing_targets_result.ok,
        "#1115: launch contract should reject fill-style-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_fill_style_object_ambiguity() {
    const auto fill_style_locked_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--fill-style-object",
        "--locked-object",
        "--fill-style", "2",
        "--fill-style-target-unique-id", "one-guid",
        "--locked", "true",
        "--locked-target-unique-id", "one-guid"
    });
    expect(!fill_style_locked_result.ok,
        "#1115: launch contract should reject simultaneous fill-style-object and locked-object requests");

    const auto fill_style_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--fill-style-object",
        "--clear-property",
        "--property-name", "FillStyle",
        "--fill-style", "2",
        "--fill-style-target-unique-id", "one-guid"
    });
    expect(!fill_style_property_result.ok,
        "#1115: launch contract should reject fill-style-object combined with property commands");

    const auto stray_fill_style_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--fill-style", "2"
    });
    expect(!stray_fill_style_result.ok,
        "#1115: launch contract should reject stray fill-style arguments");
}

void test_parse_launch_arguments_for_grid_line_color_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--grid-line-color-object",
        "--grid-line-color", "9",
        "--grid-line-color-target-object-name", "cmdSave",
        "--grid-line-color-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1120: launch contract should parse grid-line-color-object requests");
    expect(result.request.grid_line_color_object,
        "#1120: launch contract should detect --grid-line-color-object");
    expect(result.request.grid_line_color_available && result.request.grid_line_color == 9,
        "#1120: grid-line-color-object requests should carry grid-line-color value");
    expect(result.request.grid_line_color_objects.size() == 2U,
        "#1120: grid-line-color-object requests should collect grid-line-color target selectors");
    if (result.request.grid_line_color_objects.size() == 2U) {
        expect(result.request.grid_line_color_objects[0].object_name == "cmdSave" &&
                result.request.grid_line_color_objects[0].unique_id.empty(),
            "#1120: grid-line-color-object requests should parse target object-name selectors");
        expect(result.request.grid_line_color_objects[1].object_name.empty() &&
                result.request.grid_line_color_objects[1].unique_id == "two-guid",
            "#1120: grid-line-color-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_grid_line_color_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--grid-line-color-object",
        "--grid-line-color-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1120: launch contract should reject grid-line-color-object requests without grid-line-color value");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--grid-line-color-object",
        "--grid-line-color", "manual",
        "--grid-line-color-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1120: launch contract should reject non-integer grid-line-color values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--grid-line-color-object",
        "--grid-line-color", "-1",
        "--grid-line-color-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1120: launch contract should reject negative grid-line-color values before mutation");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--grid-line-color-object",
        "--grid-line-color", "2"
    });
    expect(!missing_targets_result.ok,
        "#1120: launch contract should reject grid-line-color-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_grid_line_color_object_ambiguity() {
    const auto grid_line_color_locked_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--grid-line-color-object",
        "--locked-object",
        "--grid-line-color", "2",
        "--grid-line-color-target-unique-id", "one-guid",
        "--locked", "true",
        "--locked-target-unique-id", "one-guid"
    });
    expect(!grid_line_color_locked_result.ok,
        "#1120: launch contract should reject simultaneous grid-line-color-object and locked-object requests");

    const auto grid_line_color_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--grid-line-color-object",
        "--clear-property",
        "--property-name", "GridLineColor",
        "--grid-line-color", "2",
        "--grid-line-color-target-unique-id", "one-guid"
    });
    expect(!grid_line_color_property_result.ok,
        "#1120: launch contract should reject grid-line-color-object combined with property commands");

    const auto stray_grid_line_color_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--grid-line-color", "2"
    });
    expect(!stray_grid_line_color_result.ok,
        "#1120: launch contract should reject stray grid-line-color arguments");
}

void test_parse_launch_arguments_for_grid_line_width_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--grid-line-width-object",
        "--grid-line-width", "9",
        "--grid-line-width-target-object-name", "cmdSave",
        "--grid-line-width-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1125: launch contract should parse grid-line-width-object requests");
    expect(result.request.grid_line_width_object,
        "#1125: launch contract should detect --grid-line-width-object");
    expect(result.request.grid_line_width_available && result.request.grid_line_width == 9,
        "#1125: grid-line-width-object requests should carry grid-line-width value");
    expect(result.request.grid_line_width_objects.size() == 2U,
        "#1125: grid-line-width-object requests should collect grid-line-width target selectors");
    if (result.request.grid_line_width_objects.size() == 2U) {
        expect(result.request.grid_line_width_objects[0].object_name == "cmdSave" &&
                result.request.grid_line_width_objects[0].unique_id.empty(),
            "#1125: grid-line-width-object requests should parse target object-name selectors");
        expect(result.request.grid_line_width_objects[1].object_name.empty() &&
                result.request.grid_line_width_objects[1].unique_id == "two-guid",
            "#1125: grid-line-width-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_grid_line_width_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--grid-line-width-object",
        "--grid-line-width-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1125: launch contract should reject grid-line-width-object requests without grid-line-width value");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--grid-line-width-object",
        "--grid-line-width", "manual",
        "--grid-line-width-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1125: launch contract should reject non-integer grid-line-width values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--grid-line-width-object",
        "--grid-line-width", "-1",
        "--grid-line-width-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1125: launch contract should reject negative grid-line-width values before mutation");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--grid-line-width-object",
        "--grid-line-width", "2"
    });
    expect(!missing_targets_result.ok,
        "#1125: launch contract should reject grid-line-width-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_grid_line_width_object_ambiguity() {
    const auto grid_line_width_locked_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--grid-line-width-object",
        "--locked-object",
        "--grid-line-width", "2",
        "--grid-line-width-target-unique-id", "one-guid",
        "--locked", "true",
        "--locked-target-unique-id", "one-guid"
    });
    expect(!grid_line_width_locked_result.ok,
        "#1125: launch contract should reject simultaneous grid-line-width-object and locked-object requests");

    const auto grid_line_width_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--grid-line-width-object",
        "--clear-property",
        "--property-name", "GridLineWidth",
        "--grid-line-width", "2",
        "--grid-line-width-target-unique-id", "one-guid"
    });
    expect(!grid_line_width_property_result.ok,
        "#1125: launch contract should reject grid-line-width-object combined with property commands");

    const auto stray_grid_line_width_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--grid-line-width", "2"
    });
    expect(!stray_grid_line_width_result.ok,
        "#1125: launch contract should reject stray grid-line-width arguments");
}

void test_parse_launch_arguments_for_grid_lines_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--grid-lines-object",
        "--grid-lines", "9",
        "--grid-lines-target-object-name", "cmdSave",
        "--grid-lines-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1126: launch contract should parse grid-lines-object requests");
    expect(result.request.grid_lines_object,
        "#1126: launch contract should detect --grid-lines-object");
    expect(result.request.grid_lines_available && result.request.grid_lines == 9,
        "#1126: grid-lines-object requests should carry grid-lines value");
    expect(result.request.grid_lines_objects.size() == 2U,
        "#1126: grid-lines-object requests should collect grid-lines target selectors");
    if (result.request.grid_lines_objects.size() == 2U) {
        expect(result.request.grid_lines_objects[0].object_name == "cmdSave" &&
                result.request.grid_lines_objects[0].unique_id.empty(),
            "#1126: grid-lines-object requests should parse target object-name selectors");
        expect(result.request.grid_lines_objects[1].object_name.empty() &&
                result.request.grid_lines_objects[1].unique_id == "two-guid",
            "#1126: grid-lines-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_grid_lines_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--grid-lines-object",
        "--grid-lines-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1126: launch contract should reject grid-lines-object requests without grid-lines value");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--grid-lines-object",
        "--grid-lines", "manual",
        "--grid-lines-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1126: launch contract should reject non-integer grid-lines values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--grid-lines-object",
        "--grid-lines", "-1",
        "--grid-lines-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1126: launch contract should reject negative grid-lines values before mutation");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--grid-lines-object",
        "--grid-lines", "2"
    });
    expect(!missing_targets_result.ok,
        "#1126: launch contract should reject grid-lines-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_grid_lines_object_ambiguity() {
    const auto grid_lines_locked_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--grid-lines-object",
        "--locked-object",
        "--grid-lines", "2",
        "--grid-lines-target-unique-id", "one-guid",
        "--locked", "true",
        "--locked-target-unique-id", "one-guid"
    });
    expect(!grid_lines_locked_result.ok,
        "#1126: launch contract should reject simultaneous grid-lines-object and locked-object requests");

    const auto grid_lines_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--grid-lines-object",
        "--clear-property",
        "--property-name", "GridLines",
        "--grid-lines", "2",
        "--grid-lines-target-unique-id", "one-guid"
    });
    expect(!grid_lines_property_result.ok,
        "#1126: launch contract should reject grid-lines-object combined with property commands");

    const auto stray_grid_lines_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--grid-lines", "2"
    });
    expect(!stray_grid_lines_result.ok,
        "#1126: launch contract should reject stray grid-lines arguments");
}

void test_parse_launch_arguments_for_fill_color_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--fill-color-object",
        "--fill-color", "9",
        "--fill-color-target-object-name", "cmdSave",
        "--fill-color-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1134: launch contract should parse fill-color-object requests");
    expect(result.request.fill_color_object,
        "#1134: launch contract should detect --fill-color-object");
    expect(result.request.fill_color_available && result.request.fill_color == 9,
        "#1134: fill-color-object requests should carry fill-color value");
    expect(result.request.fill_color_objects.size() == 2U,
        "#1134: fill-color-object requests should collect fill_color target selectors");
    if (result.request.fill_color_objects.size() == 2U) {
        expect(result.request.fill_color_objects[0].object_name == "cmdSave" &&
                result.request.fill_color_objects[0].unique_id.empty(),
            "#1134: fill-color-object requests should parse target object-name selectors");
        expect(result.request.fill_color_objects[1].object_name.empty() &&
                result.request.fill_color_objects[1].unique_id == "two-guid",
            "#1134: fill-color-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_fill_color_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--fill-color-object",
        "--fill-color-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1134: launch contract should reject fill-color-object requests without fill-color value");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--fill-color-object",
        "--fill-color", "manual",
        "--fill-color-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1134: launch contract should reject non-integer fill-color values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--fill-color-object",
        "--fill-color", "-1",
        "--fill-color-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1134: launch contract should reject negative fill-color values before mutation");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--fill-color-object",
        "--fill-color", "2"
    });
    expect(!missing_targets_result.ok,
        "#1134: launch contract should reject fill-color-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_fill_color_object_ambiguity() {
    const auto fill_color_locked_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--fill-color-object",
        "--locked-object",
        "--fill-color", "2",
        "--fill-color-target-unique-id", "one-guid",
        "--locked", "true",
        "--locked-target-unique-id", "one-guid"
    });
    expect(!fill_color_locked_result.ok,
        "#1134: launch contract should reject simultaneous fill-color-object and locked-object requests");

    const auto fill_color_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--fill-color-object",
        "--clear-property",
        "--property-name", "FillColor",
        "--fill-color", "2",
        "--fill-color-target-unique-id", "one-guid"
    });
    expect(!fill_color_property_result.ok,
        "#1134: launch contract should reject fill-color-object combined with property commands");

    const auto stray_fill_color_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--fill-color", "2"
    });
    expect(!stray_fill_color_result.ok,
        "#1134: launch contract should reject stray fill-color arguments");
}

void test_parse_launch_arguments_for_back_style_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--back-style-object",
        "--back-style", "2",
        "--back-style-target-object-name", "frmCustomer",
        "--back-style-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1161: launch contract should parse back-style-object requests");
    expect(result.request.back_style_object,
        "#1161: launch contract should detect --back-style-object");
    expect(result.request.back_style_available && result.request.back_style == 2,
        "#1161: back-style-object requests should carry back-style value");
    expect(result.request.back_style_objects.size() == 2U,
        "#1161: back-style-object requests should collect back-style target selectors");
    if (result.request.back_style_objects.size() == 2U) {
        expect(result.request.back_style_objects[0].object_name == "frmCustomer" &&
                result.request.back_style_objects[0].unique_id.empty(),
            "#1161: back-style-object requests should parse target object-name selectors");
        expect(result.request.back_style_objects[1].object_name.empty() &&
                result.request.back_style_objects[1].unique_id == "two-guid",
            "#1161: back-style-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_back_style_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--back-style-object",
        "--back-style-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1161: launch contract should reject back-style-object requests without back-style value");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--back-style-object",
        "--back-style", "2"
    });
    expect(!missing_targets_result.ok,
        "#1161: launch contract should reject back-style-object requests without target selectors");

    const auto non_integer_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--back-style-object",
        "--back-style", "transparent",
        "--back-style-target-unique-id", "one-guid"
    });
    expect(!non_integer_result.ok,
        "#1161: launch contract should reject non-integer back-style values");

    const auto negative_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--back-style-object",
        "--back-style", "-1",
        "--back-style-target-unique-id", "one-guid"
    });
    expect(!negative_result.ok,
        "#1161: launch contract should reject negative back-style values");
}

void test_parse_launch_arguments_rejects_back_style_object_ambiguity() {
    const auto back_style_allow_output_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--back-style-object",
        "--allow-output-object",
        "--back-style", "2",
        "--back-style-target-unique-id", "one-guid",
        "--allow-output", "false",
        "--allow-output-target-unique-id", "one-guid"
    });
    expect(!back_style_allow_output_result.ok,
        "#1161: launch contract should reject simultaneous back-style-object and allow-output-object requests");

    const auto back_style_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--back-style-object",
        "--clear-property",
        "--property-name", "BackStyle",
        "--back-style", "2",
        "--back-style-target-unique-id", "one-guid"
    });
    expect(!back_style_property_result.ok,
        "#1161: launch contract should reject back-style-object combined with property commands");

    const auto stray_back_style_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--back-style", "2"
    });
    expect(!stray_back_style_result.ok,
        "#1161: launch contract should reject stray back-style arguments");
}

void test_parse_launch_arguments_for_border_style_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--border-style-object",
        "--border-style", "2",
        "--border-style-target-object-name", "frmCustomer",
        "--border-style-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1162: launch contract should parse border-style-object requests");
    expect(result.request.border_style_object,
        "#1162: launch contract should detect --border-style-object");
    expect(result.request.border_style_available && result.request.border_style == 2,
        "#1162: border-style-object requests should carry border-style value");
    expect(result.request.border_style_objects.size() == 2U,
        "#1162: border-style-object requests should collect border-style target selectors");
    if (result.request.border_style_objects.size() == 2U) {
        expect(result.request.border_style_objects[0].object_name == "frmCustomer" &&
                result.request.border_style_objects[0].unique_id.empty(),
            "#1162: border-style-object requests should parse target object-name selectors");
        expect(result.request.border_style_objects[1].object_name.empty() &&
                result.request.border_style_objects[1].unique_id == "two-guid",
            "#1162: border-style-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_border_style_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--border-style-object",
        "--border-style-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1162: launch contract should reject border-style-object requests without border-style value");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--border-style-object",
        "--border-style", "2"
    });
    expect(!missing_targets_result.ok,
        "#1162: launch contract should reject border-style-object requests without target selectors");

    const auto non_integer_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--border-style-object",
        "--border-style", "fixed",
        "--border-style-target-unique-id", "one-guid"
    });
    expect(!non_integer_result.ok,
        "#1162: launch contract should reject non-integer border-style values");

    const auto negative_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--border-style-object",
        "--border-style", "-1",
        "--border-style-target-unique-id", "one-guid"
    });
    expect(!negative_result.ok,
        "#1162: launch contract should reject negative border-style values");
}

void test_parse_launch_arguments_rejects_border_style_object_ambiguity() {
    const auto border_style_allow_output_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--border-style-object",
        "--allow-output-object",
        "--border-style", "2",
        "--border-style-target-unique-id", "one-guid",
        "--allow-output", "false",
        "--allow-output-target-unique-id", "one-guid"
    });
    expect(!border_style_allow_output_result.ok,
        "#1162: launch contract should reject simultaneous border-style-object and allow-output-object requests");

    const auto border_style_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--border-style-object",
        "--clear-property",
        "--property-name", "BorderStyle",
        "--border-style", "2",
        "--border-style-target-unique-id", "one-guid"
    });
    expect(!border_style_property_result.ok,
        "#1162: launch contract should reject border-style-object combined with property commands");

    const auto stray_border_style_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--border-style", "2"
    });
    expect(!stray_border_style_result.ok,
        "#1162: launch contract should reject stray border-style arguments");
}

void test_parse_launch_arguments_for_border_width_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--border-width-object",
        "--border-width", "2",
        "--border-width-target-object-name", "frmCustomer",
        "--border-width-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1163: launch contract should parse border-width-object requests");
    expect(result.request.border_width_object,
        "#1163: launch contract should detect --border-width-object");
    expect(result.request.border_width_available && result.request.border_width == 2,
        "#1163: border-width-object requests should carry border-width value");
    expect(result.request.border_width_objects.size() == 2U,
        "#1163: border-width-object requests should collect border-width target selectors");
    if (result.request.border_width_objects.size() == 2U) {
        expect(result.request.border_width_objects[0].object_name == "frmCustomer" &&
                result.request.border_width_objects[0].unique_id.empty(),
            "#1163: border-width-object requests should parse target object-name selectors");
        expect(result.request.border_width_objects[1].object_name.empty() &&
                result.request.border_width_objects[1].unique_id == "two-guid",
            "#1163: border-width-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_border_width_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--border-width-object",
        "--border-width-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1163: launch contract should reject border-width-object requests without border-width value");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--border-width-object",
        "--border-width", "2"
    });
    expect(!missing_targets_result.ok,
        "#1163: launch contract should reject border-width-object requests without target selectors");

    const auto non_integer_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--border-width-object",
        "--border-width", "wide",
        "--border-width-target-unique-id", "one-guid"
    });
    expect(!non_integer_result.ok,
        "#1163: launch contract should reject non-integer border-width values");

    const auto negative_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--border-width-object",
        "--border-width", "-1",
        "--border-width-target-unique-id", "one-guid"
    });
    expect(!negative_result.ok,
        "#1163: launch contract should reject negative border-width values");
}

void test_parse_launch_arguments_rejects_border_width_object_ambiguity() {
    const auto border_width_allow_output_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--border-width-object",
        "--allow-output-object",
        "--border-width", "2",
        "--border-width-target-unique-id", "one-guid",
        "--allow-output", "false",
        "--allow-output-target-unique-id", "one-guid"
    });
    expect(!border_width_allow_output_result.ok,
        "#1163: launch contract should reject simultaneous border-width-object and allow-output-object requests");

    const auto border_width_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--border-width-object",
        "--clear-property",
        "--property-name", "BorderWidth",
        "--border-width", "2",
        "--border-width-target-unique-id", "one-guid"
    });
    expect(!border_width_property_result.ok,
        "#1163: launch contract should reject border-width-object combined with property commands");

    const auto stray_border_width_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--border-width", "2"
    });
    expect(!stray_border_width_result.ok,
        "#1163: launch contract should reject stray border-width arguments");
}

void test_parse_launch_arguments_for_border_color_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--border-color-object",
        "--border-color", "8421504",
        "--border-color-target-object-name", "frmCustomer",
        "--border-color-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1164: launch contract should parse border-color-object requests");
    expect(result.request.border_color_object,
        "#1164: launch contract should detect --border-color-object");
    expect(result.request.border_color_available && result.request.border_color == 8421504,
        "#1164: border-color-object requests should carry border-color value");
    expect(result.request.border_color_objects.size() == 2U,
        "#1164: border-color-object requests should collect border-color target selectors");
    if (result.request.border_color_objects.size() == 2U) {
        expect(result.request.border_color_objects[0].object_name == "frmCustomer" &&
                result.request.border_color_objects[0].unique_id.empty(),
            "#1164: border-color-object requests should parse target object-name selectors");
        expect(result.request.border_color_objects[1].object_name.empty() &&
                result.request.border_color_objects[1].unique_id == "two-guid",
            "#1164: border-color-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_border_color_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--border-color-object",
        "--border-color-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1164: launch contract should reject border-color-object requests without border-color value");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--border-color-object",
        "--border-color", "8421504"
    });
    expect(!missing_targets_result.ok,
        "#1164: launch contract should reject border-color-object requests without target selectors");

    const auto non_integer_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--border-color-object",
        "--border-color", "gray",
        "--border-color-target-unique-id", "one-guid"
    });
    expect(!non_integer_result.ok,
        "#1164: launch contract should reject non-integer border-color values");

    const auto negative_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--border-color-object",
        "--border-color", "-1",
        "--border-color-target-unique-id", "one-guid"
    });
    expect(!negative_result.ok,
        "#1164: launch contract should reject negative border-color values");
}

void test_parse_launch_arguments_rejects_border_color_object_ambiguity() {
    const auto border_color_allow_output_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--border-color-object",
        "--allow-output-object",
        "--border-color", "8421504",
        "--border-color-target-unique-id", "one-guid",
        "--allow-output", "false",
        "--allow-output-target-unique-id", "one-guid"
    });
    expect(!border_color_allow_output_result.ok,
        "#1164: launch contract should reject simultaneous border-color-object and allow-output-object requests");

    const auto border_color_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--border-color-object",
        "--clear-property",
        "--property-name", "BorderColor",
        "--border-color", "8421504",
        "--border-color-target-unique-id", "one-guid"
    });
    expect(!border_color_property_result.ok,
        "#1164: launch contract should reject border-color-object combined with property commands");

    const auto stray_border_color_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--border-color", "8421504"
    });
    expect(!stray_border_color_result.ok,
        "#1164: launch contract should reject stray border-color arguments");
}

void test_parse_launch_arguments_for_special_effect_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--special-effect-object",
        "--special-effect", "2",
        "--special-effect-target-object-name", "frmCustomer",
        "--special-effect-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1166: launch contract should parse special-effect-object requests");
    expect(result.request.special_effect_object,
        "#1166: launch contract should detect --special-effect-object");
    expect(result.request.special_effect_available && result.request.special_effect == 2,
        "#1166: special-effect-object requests should carry special-effect value");
    expect(result.request.special_effect_objects.size() == 2U,
        "#1166: special-effect-object requests should collect special-effect target selectors");
    if (result.request.special_effect_objects.size() == 2U) {
        expect(result.request.special_effect_objects[0].object_name == "frmCustomer" &&
                result.request.special_effect_objects[0].unique_id.empty(),
            "#1166: special-effect-object requests should parse target object-name selectors");
        expect(result.request.special_effect_objects[1].object_name.empty() &&
                result.request.special_effect_objects[1].unique_id == "two-guid",
            "#1166: special-effect-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_special_effect_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--special-effect-object",
        "--special-effect-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1166: launch contract should reject special-effect-object requests without special-effect value");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--special-effect-object",
        "--special-effect", "2"
    });
    expect(!missing_targets_result.ok,
        "#1166: launch contract should reject special-effect-object requests without target selectors");

    const auto non_integer_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--special-effect-object",
        "--special-effect", "raised",
        "--special-effect-target-unique-id", "one-guid"
    });
    expect(!non_integer_result.ok,
        "#1166: launch contract should reject non-integer special-effect values");

    const auto negative_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--special-effect-object",
        "--special-effect", "-1",
        "--special-effect-target-unique-id", "one-guid"
    });
    expect(!negative_result.ok,
        "#1166: launch contract should reject negative special-effect values");
}

void test_parse_launch_arguments_rejects_special_effect_object_ambiguity() {
    const auto special_effect_allow_output_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--special-effect-object",
        "--allow-output-object",
        "--special-effect", "2",
        "--special-effect-target-unique-id", "one-guid",
        "--allow-output", "false",
        "--allow-output-target-unique-id", "one-guid"
    });
    expect(!special_effect_allow_output_result.ok,
        "#1166: launch contract should reject simultaneous special-effect-object and allow-output-object requests");

    const auto special_effect_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--special-effect-object",
        "--clear-property",
        "--property-name", "SpecialEffect",
        "--special-effect", "2",
        "--special-effect-target-unique-id", "one-guid"
    });
    expect(!special_effect_property_result.ok,
        "#1166: launch contract should reject special-effect-object combined with property commands");

    const auto stray_special_effect_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--special-effect", "2"
    });
    expect(!stray_special_effect_result.ok,
        "#1166: launch contract should reject stray special-effect arguments");
}

}  // namespace cf_test_studio_host
