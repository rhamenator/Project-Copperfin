// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_visual_asset_editor_support.h"

namespace cf_test_visual_asset_editor {
void test_set_visual_object_drag_mode_assigns_numeric_value() {
    test_visual_object_non_negative_numeric_property_assigns_value(
        "#872",
        "drag_mode",
        "DragMode",
        "DRAGMODE",
        "drag-mode",
        0,
        1,
        2,
        3,
        4,
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            int value) {
            return copperfin::vfp::set_visual_object_drag_mode({
                .path = path,
                .objects = objects,
                .drag_mode = value
            });
        });
}

void test_set_visual_object_ole_drag_mode_assigns_numeric_value() {
    test_visual_object_non_negative_numeric_property_assigns_value(
        "#873",
        "ole_drag_mode",
        "OLEDragMode",
        "OLEDRAGMODE",
        "OLE drag-mode",
        0,
        1,
        2,
        3,
        4,
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            int value) {
            return copperfin::vfp::set_visual_object_ole_drag_mode({
                .path = path,
                .objects = objects,
                .ole_drag_mode = value
            });
        });
}

void test_set_visual_object_ole_drop_mode_assigns_numeric_value() {
    test_visual_object_non_negative_numeric_property_assigns_value(
        "#874",
        "ole_drop_mode",
        "OLEDropMode",
        "OLEDROPMODE",
        "OLE drop-mode",
        0,
        1,
        2,
        3,
        4,
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            int value) {
            return copperfin::vfp::set_visual_object_ole_drop_mode({
                .path = path,
                .objects = objects,
                .ole_drop_mode = value
            });
        });
}

void test_set_visual_object_ole_drop_effects_assigns_numeric_value() {
    test_visual_object_non_negative_numeric_property_assigns_value(
        "#875",
        "ole_drop_effects",
        "OLEDropEffects",
        "OLEDROPEFFECTS",
        "OLE drop-effects",
        0,
        1,
        2,
        3,
        4,
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            int value) {
            return copperfin::vfp::set_visual_object_ole_drop_effects({
                .path = path,
                .objects = objects,
                .ole_drop_effects = value
            });
        });
}

void test_set_visual_object_ole_drop_text_insertion_assigns_numeric_value() {
    test_visual_object_non_negative_numeric_property_assigns_value(
        "#876",
        "ole_drop_text_insertion",
        "OLEDropTextInsertion",
        "OLEDROPTEXTINSERTION",
        "OLE drop text-insertion",
        0,
        1,
        2,
        3,
        4,
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            int value) {
            return copperfin::vfp::set_visual_object_ole_drop_text_insertion({
                .path = path,
                .objects = objects,
                .ole_drop_text_insertion = value
            });
        });
}

void test_set_visual_object_buffer_mode_assigns_numeric_value() {
    test_visual_object_non_negative_numeric_property_assigns_value(
        "#913",
        "buffer_mode",
        "BufferMode",
        "BUFFERMODE",
        "buffer-mode",
        0,
        1,
        2,
        3,
        4,
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            int value) {
            return copperfin::vfp::set_visual_object_buffer_mode({
                .path = path,
                .objects = objects,
                .buffer_mode = value
            });
        });
}

void test_set_visual_object_buffer_mode_override_assigns_numeric_value() {
    test_visual_object_non_negative_numeric_property_assigns_value(
        "#914",
        "buffer_mode_override",
        "BufferModeOverride",
        "BUFFERMODEOVERRIDE",
        "buffer-mode-override",
        0,
        1,
        2,
        3,
        4,
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            int value) {
            return copperfin::vfp::set_visual_object_buffer_mode_override({
                .path = path,
                .objects = objects,
                .buffer_mode_override = value
            });
        });
}

}  // namespace cf_test_visual_asset_editor
