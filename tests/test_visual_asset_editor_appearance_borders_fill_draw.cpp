// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_visual_asset_editor_support.h"

namespace cf_test_visual_asset_editor {
void test_set_visual_object_back_style_assigns_numeric_value() {
    test_visual_object_non_negative_numeric_property_assigns_value(
        "#860",
        "back_style",
        "BackStyle",
        "BACKSTYLE",
        "back-style",
        0,
        0,
        2,
        1,
        2,
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            int value) {
            return copperfin::vfp::set_visual_object_back_style({
                .path = path,
                .objects = objects,
                .back_style = value
            });
        });
}

void test_set_visual_object_border_style_assigns_numeric_value() {
    test_visual_object_non_negative_numeric_property_assigns_value(
        "#861",
        "border_style",
        "BorderStyle",
        "BORDERSTYLE",
        "border-style",
        0,
        0,
        2,
        1,
        2,
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            int value) {
            return copperfin::vfp::set_visual_object_border_style({
                .path = path,
                .objects = objects,
                .border_style = value
            });
        });
}

void test_set_visual_object_border_width_assigns_numeric_value() {
    test_visual_object_non_negative_numeric_property_assigns_value(
        "#862",
        "border_width",
        "BorderWidth",
        "BORDERWIDTH",
        "border-width",
        0,
        0,
        2,
        1,
        2,
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            int value) {
            return copperfin::vfp::set_visual_object_border_width({
                .path = path,
                .objects = objects,
                .border_width = value
            });
        });
}

void test_set_visual_object_grid_line_width_assigns_numeric_value() {
    test_visual_object_non_negative_numeric_property_assigns_value(
        "#922",
        "grid_line_width",
        "GridLineWidth",
        "GRIDLINEWIDTH",
        "grid-line-width",
        0,
        1,
        2,
        3,
        4,
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            int value) {
            return copperfin::vfp::set_visual_object_grid_line_width({
                .path = path,
                .objects = objects,
                .grid_line_width = value
            });
        });
}

void test_set_visual_object_grid_lines_assigns_numeric_value() {
    test_visual_object_non_negative_numeric_property_assigns_value(
        "#923",
        "grid_lines",
        "GridLines",
        "GRIDLINES",
        "grid-lines",
        0,
        1,
        2,
        3,
        4,
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            int value) {
            return copperfin::vfp::set_visual_object_grid_lines({
                .path = path,
                .objects = objects,
                .grid_lines = value
            });
        });
}

void test_set_visual_object_special_effect_assigns_numeric_value() {
    test_visual_object_non_negative_numeric_property_assigns_value(
        "#864",
        "special_effect",
        "SpecialEffect",
        "SPECIALEFFECT",
        "special-effect",
        0,
        0,
        2,
        1,
        2,
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            int value) {
            return copperfin::vfp::set_visual_object_special_effect({
                .path = path,
                .objects = objects,
                .special_effect = value
            });
        });
}

void test_set_visual_object_curvature_assigns_numeric_value() {
    test_visual_object_non_negative_numeric_property_assigns_value(
        "#884",
        "curvature",
        "Curvature",
        "CURVATURE",
        "curvature",
        0,
        10,
        99,
        25,
        50,
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            int value) {
            return copperfin::vfp::set_visual_object_curvature({
                .path = path,
                .objects = objects,
                .curvature = value
            });
        });
}

void test_set_visual_object_draw_mode_assigns_numeric_value() {
    test_visual_object_non_negative_numeric_property_assigns_value(
        "#885",
        "draw_mode",
        "DrawMode",
        "DRAWMODE",
        "draw-mode",
        13,
        7,
        1,
        6,
        10,
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            int value) {
            return copperfin::vfp::set_visual_object_draw_mode({
                .path = path,
                .objects = objects,
                .draw_mode = value
            });
        });
}

void test_set_visual_object_draw_style_assigns_numeric_value() {
    test_visual_object_non_negative_numeric_property_assigns_value(
        "#886",
        "draw_style",
        "DrawStyle",
        "DRAWSTYLE",
        "draw-style",
        0,
        1,
        2,
        3,
        5,
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            int value) {
            return copperfin::vfp::set_visual_object_draw_style({
                .path = path,
                .objects = objects,
                .draw_style = value
            });
        });
}

void test_set_visual_object_draw_width_assigns_numeric_value() {
    test_visual_object_non_negative_numeric_property_assigns_value(
        "#887",
        "draw_width",
        "DrawWidth",
        "DRAWWIDTH",
        "draw-width",
        1,
        2,
        3,
        4,
        6,
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            int value) {
            return copperfin::vfp::set_visual_object_draw_width({
                .path = path,
                .objects = objects,
                .draw_width = value
            });
        });
}

void test_set_visual_object_fill_style_assigns_numeric_value() {
    test_visual_object_non_negative_numeric_property_assigns_value(
        "#888",
        "fill_style",
        "FillStyle",
        "FILLSTYLE",
        "fill-style",
        0,
        1,
        7,
        3,
        4,
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            int value) {
            return copperfin::vfp::set_visual_object_fill_style({
                .path = path,
                .objects = objects,
                .fill_style = value
            });
        });
}

}  // namespace cf_test_visual_asset_editor
