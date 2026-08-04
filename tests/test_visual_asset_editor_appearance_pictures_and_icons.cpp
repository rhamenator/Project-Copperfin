// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_visual_asset_editor_support.h"

namespace cf_test_visual_asset_editor {
void test_set_visual_object_picture_assigns_text() {
    test_visual_object_text_property_assigns_text(
        "#856",
        "picture",
        "Picture",
        "PICTURE",
        "picture",
        "forms\\customer.bmp",
        "forms\\orders.bmp",
        "forms\\other.bmp",
        "C:\\\\images\\\\customer \"hero\".bmp",
        "gallery.forms.customer",
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            const std::string& value) {
            return copperfin::vfp::set_visual_object_picture({
                .path = path,
                .objects = objects,
                .picture = value
                });
        });
}

void test_set_visual_object_down_picture_assigns_text() {
    test_visual_object_text_property_assigns_text(
        "#857",
        "down_picture",
        "DownPicture",
        "DOWNPICTURE",
        "down-picture",
        "buttons\\customer_down.bmp",
        "buttons\\orders_down.bmp",
        "buttons\\other_down.bmp",
        "C:\\\\images\\\\customer \"pressed\".bmp",
        "gallery.forms.customer_down",
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            const std::string& value) {
            return copperfin::vfp::set_visual_object_down_picture({
                .path = path,
                .objects = objects,
                .down_picture = value
                });
        });
}

void test_set_visual_object_disabled_picture_assigns_text() {
    test_visual_object_text_property_assigns_text(
        "#858",
        "disabled_picture",
        "DisabledPicture",
        "DISABLEDPICTURE",
        "disabled-picture",
        "buttons\\customer_disabled.bmp",
        "buttons\\orders_disabled.bmp",
        "buttons\\other_disabled.bmp",
        "C:\\\\images\\\\customer \"disabled\".bmp",
        "gallery.forms.customer_disabled",
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            const std::string& value) {
            return copperfin::vfp::set_visual_object_disabled_picture({
                .path = path,
                .objects = objects,
                .disabled_picture = value
            });
        });
}

void test_set_visual_object_ole_drag_picture_assigns_text() {
    test_visual_object_text_property_assigns_text(
        "#859",
        "ole_drag_picture",
        "OLEDragPicture",
        "OLEDRAGPICTURE",
        "OLE drag-picture",
        "drag\\customer.bmp",
        "drag\\orders.bmp",
        "drag\\other.bmp",
        "C:\\\\images\\\\customer \"drag\".bmp",
        "gallery.forms.customer_drag",
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            const std::string& value) {
            return copperfin::vfp::set_visual_object_ole_drag_picture({
                .path = path,
                .objects = objects,
                .ole_drag_picture = value
            });
        });
}

void test_set_visual_object_mouse_icon_assigns_text() {
    test_visual_object_text_property_assigns_text(
        "#870",
        "mouse_icon",
        "MouseIcon",
        "MOUSEICON",
        "mouse-icon",
        "icons\\customer.ico",
        "icons\\orders.ico",
        "icons\\other.ico",
        "C:\\\\icons\\\\customer \"hover\".ico",
        "gallery.forms.customer_mouse",
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            const std::string& value) {
            return copperfin::vfp::set_visual_object_mouse_icon({
                .path = path,
                .objects = objects,
                .mouse_icon = value
            });
        });
}

void test_set_visual_object_drag_icon_assigns_text() {
    test_visual_object_text_property_assigns_text(
        "#871",
        "drag_icon",
        "DragIcon",
        "DRAGICON",
        "drag-icon",
        "icons\\customer_drag.ico",
        "icons\\orders_drag.ico",
        "icons\\other_drag.ico",
        "C:\\\\icons\\\\customer \"drag\".ico",
        "gallery.forms.customer_drag_icon",
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            const std::string& value) {
            return copperfin::vfp::set_visual_object_drag_icon({
                .path = path,
                .objects = objects,
                .drag_icon = value
            });
        });
}

void test_set_visual_object_picture_margin_assigns_numeric_value() {
    test_visual_object_non_negative_numeric_property_assigns_value(
        "#866",
        "picture_margin",
        "PictureMargin",
        "PICTUREMARGIN",
        "picture-margin",
        0,
        0,
        8,
        4,
        2,
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            int value) {
            return copperfin::vfp::set_visual_object_picture_margin({
                .path = path,
                .objects = objects,
                .picture_margin = value
            });
        });
}

void test_set_visual_object_picture_position_assigns_numeric_value() {
    test_visual_object_non_negative_numeric_property_assigns_value(
        "#867",
        "picture_position",
        "PicturePosition",
        "PICTUREPOSITION",
        "picture-position",
        0,
        0,
        7,
        3,
        5,
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            int value) {
            return copperfin::vfp::set_visual_object_picture_position({
                .path = path,
                .objects = objects,
                .picture_position = value
            });
        });
}

void test_set_visual_object_picture_spacing_assigns_numeric_value() {
    test_visual_object_non_negative_numeric_property_assigns_value(
        "#868",
        "picture_spacing",
        "PictureSpacing",
        "PICTURESPACING",
        "picture-spacing",
        0,
        0,
        6,
        2,
        4,
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            int value) {
            return copperfin::vfp::set_visual_object_picture_spacing({
                .path = path,
                .objects = objects,
                .picture_spacing = value
            });
        });
}

void test_set_visual_object_picture_selection_display_assigns_numeric_value() {
    test_visual_object_non_negative_numeric_property_assigns_value(
        "#869",
        "picture_selection_display",
        "PictureSelectionDisplay",
        "PICTURESELECTIONDISPLAY",
        "picture-selection-display",
        0,
        0,
        2,
        1,
        3,
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            int value) {
            return copperfin::vfp::set_visual_object_picture_selection_display({
                .path = path,
                .objects = objects,
                .picture_selection_display = value
            });
        });
}

}  // namespace cf_test_visual_asset_editor
