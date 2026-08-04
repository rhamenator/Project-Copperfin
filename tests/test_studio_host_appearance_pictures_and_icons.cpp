// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_studio_host_support.h"

namespace cf_test_studio_host {
void test_parse_launch_arguments_for_picture_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--picture-object",
        "--picture", "forms\\customer.bmp",
        "--picture-target-object-name", "cmdSave",
        "--picture-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1098: launch contract should parse picture-object requests");
    expect(result.request.picture_object, "#1098: launch contract should detect --picture-object");
    expect(result.request.picture_available && result.request.picture == "forms\\customer.bmp",
        "#1098: picture-object requests should carry picture text");
    expect(result.request.picture_objects.size() == 2U,
        "#1098: picture-object requests should collect picture target selectors");
    if (result.request.picture_objects.size() == 2U) {
        expect(result.request.picture_objects[0].object_name == "cmdSave" &&
                result.request.picture_objects[0].unique_id.empty(),
            "#1098: picture-object requests should parse target object-name selectors");
        expect(result.request.picture_objects[1].object_name.empty() &&
                result.request.picture_objects[1].unique_id == "two-guid",
            "#1098: picture-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_picture_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--picture-object",
        "--picture-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1098: launch contract should reject picture-object requests without picture text");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--picture-object",
        "--picture", "forms\\customer.bmp"
    });
    expect(!missing_targets_result.ok,
        "#1098: launch contract should reject picture-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_picture_object_ambiguity() {
    const auto picture_locked_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--picture-object",
        "--locked-object",
        "--picture", "forms\\customer.bmp",
        "--picture-target-unique-id", "one-guid",
        "--locked", "true",
        "--locked-target-unique-id", "one-guid"
    });
    expect(!picture_locked_result.ok,
        "#1098: launch contract should reject simultaneous picture-object and locked-object requests");

    const auto picture_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--picture-object",
        "--clear-property",
        "--property-name", "Picture",
        "--picture", "forms\\customer.bmp",
        "--picture-target-unique-id", "one-guid"
    });
    expect(!picture_property_result.ok,
        "#1098: launch contract should reject picture-object combined with property commands");

    const auto stray_picture_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--picture", "forms\\customer.bmp"
    });
    expect(!stray_picture_result.ok,
        "#1098: launch contract should reject stray picture arguments");
}

void test_parse_launch_arguments_for_down_picture_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--down-picture-object",
        "--down-picture", "forms\\customer_down.bmp",
        "--down-picture-target-object-name", "cmdSave",
        "--down-picture-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1099: launch contract should parse down-picture-object requests");
    expect(result.request.down_picture_object, "#1099: launch contract should detect --down-picture-object");
    expect(result.request.down_picture_available && result.request.down_picture == "forms\\customer_down.bmp",
        "#1099: down-picture-object requests should carry down-picture text");
    expect(result.request.down_picture_objects.size() == 2U,
        "#1099: down-picture-object requests should collect down-picture target selectors");
    if (result.request.down_picture_objects.size() == 2U) {
        expect(result.request.down_picture_objects[0].object_name == "cmdSave" &&
                result.request.down_picture_objects[0].unique_id.empty(),
            "#1099: down-picture-object requests should parse target object-name selectors");
        expect(result.request.down_picture_objects[1].object_name.empty() &&
                result.request.down_picture_objects[1].unique_id == "two-guid",
            "#1099: down-picture-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_down_picture_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--down-picture-object",
        "--down-picture-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1099: launch contract should reject down-picture-object requests without down-picture text");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--down-picture-object",
        "--down-picture", "forms\\customer_down.bmp"
    });
    expect(!missing_targets_result.ok,
        "#1099: launch contract should reject down-picture-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_down_picture_object_ambiguity() {
    const auto down_picture_locked_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--down-picture-object",
        "--locked-object",
        "--down-picture", "forms\\customer_down.bmp",
        "--down-picture-target-unique-id", "one-guid",
        "--locked", "true",
        "--locked-target-unique-id", "one-guid"
    });
    expect(!down_picture_locked_result.ok,
        "#1099: launch contract should reject simultaneous down-picture-object and locked-object requests");

    const auto down_picture_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--down-picture-object",
        "--clear-property",
        "--property-name", "DownPicture",
        "--down-picture", "forms\\customer_down.bmp",
        "--down-picture-target-unique-id", "one-guid"
    });
    expect(!down_picture_property_result.ok,
        "#1099: launch contract should reject down-picture-object combined with property commands");

    const auto stray_down_picture_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--down-picture", "forms\\customer_down.bmp"
    });
    expect(!stray_down_picture_result.ok,
        "#1099: launch contract should reject stray down-picture arguments");
}

void test_parse_launch_arguments_for_disabled_picture_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--disabled-picture-object",
        "--disabled-picture", "forms\\customer_disabled.bmp",
        "--disabled-picture-target-object-name", "cmdSave",
        "--disabled-picture-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1100: launch contract should parse disabled-picture-object requests");
    expect(result.request.disabled_picture_object, "#1100: launch contract should detect --disabled-picture-object");
    expect(result.request.disabled_picture_available &&
            result.request.disabled_picture == "forms\\customer_disabled.bmp",
        "#1100: disabled-picture-object requests should carry disabled-picture text");
    expect(result.request.disabled_picture_objects.size() == 2U,
        "#1100: disabled-picture-object requests should collect disabled-picture target selectors");
    if (result.request.disabled_picture_objects.size() == 2U) {
        expect(result.request.disabled_picture_objects[0].object_name == "cmdSave" &&
                result.request.disabled_picture_objects[0].unique_id.empty(),
            "#1100: disabled-picture-object requests should parse target object-name selectors");
        expect(result.request.disabled_picture_objects[1].object_name.empty() &&
                result.request.disabled_picture_objects[1].unique_id == "two-guid",
            "#1100: disabled-picture-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_disabled_picture_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--disabled-picture-object",
        "--disabled-picture-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1100: launch contract should reject disabled-picture-object requests without disabled-picture text");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--disabled-picture-object",
        "--disabled-picture", "forms\\customer_disabled.bmp"
    });
    expect(!missing_targets_result.ok,
        "#1100: launch contract should reject disabled-picture-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_disabled_picture_object_ambiguity() {
    const auto disabled_picture_locked_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--disabled-picture-object",
        "--locked-object",
        "--disabled-picture", "forms\\customer_disabled.bmp",
        "--disabled-picture-target-unique-id", "one-guid",
        "--locked", "true",
        "--locked-target-unique-id", "one-guid"
    });
    expect(!disabled_picture_locked_result.ok,
        "#1100: launch contract should reject simultaneous disabled-picture-object and locked-object requests");

    const auto disabled_picture_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--disabled-picture-object",
        "--clear-property",
        "--property-name", "DisabledPicture",
        "--disabled-picture", "forms\\customer_disabled.bmp",
        "--disabled-picture-target-unique-id", "one-guid"
    });
    expect(!disabled_picture_property_result.ok,
        "#1100: launch contract should reject disabled-picture-object combined with property commands");

    const auto stray_disabled_picture_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--disabled-picture", "forms\\customer_disabled.bmp"
    });
    expect(!stray_disabled_picture_result.ok,
        "#1100: launch contract should reject stray disabled-picture arguments");
}

void test_parse_launch_arguments_for_ole_drag_picture_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--ole-drag-picture-object",
        "--ole-drag-picture", "forms\\customer_ole_drag.bmp",
        "--ole-drag-picture-target-object-name", "cmdSave",
        "--ole-drag-picture-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1101: launch contract should parse OLE drag-picture-object requests");
    expect(result.request.ole_drag_picture_object,
        "#1101: launch contract should detect --ole-drag-picture-object");
    expect(result.request.ole_drag_picture_available &&
            result.request.ole_drag_picture == "forms\\customer_ole_drag.bmp",
        "#1101: OLE drag-picture-object requests should carry OLE drag-picture text");
    expect(result.request.ole_drag_picture_objects.size() == 2U,
        "#1101: OLE drag-picture-object requests should collect OLE drag-picture target selectors");
    if (result.request.ole_drag_picture_objects.size() == 2U) {
        expect(result.request.ole_drag_picture_objects[0].object_name == "cmdSave" &&
                result.request.ole_drag_picture_objects[0].unique_id.empty(),
            "#1101: OLE drag-picture-object requests should parse target object-name selectors");
        expect(result.request.ole_drag_picture_objects[1].object_name.empty() &&
                result.request.ole_drag_picture_objects[1].unique_id == "two-guid",
            "#1101: OLE drag-picture-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_ole_drag_picture_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--ole-drag-picture-object",
        "--ole-drag-picture-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1101: launch contract should reject OLE drag-picture-object requests without OLE drag-picture text");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--ole-drag-picture-object",
        "--ole-drag-picture", "forms\\customer_ole_drag.bmp"
    });
    expect(!missing_targets_result.ok,
        "#1101: launch contract should reject OLE drag-picture-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_ole_drag_picture_object_ambiguity() {
    const auto ole_drag_picture_locked_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--ole-drag-picture-object",
        "--locked-object",
        "--ole-drag-picture", "forms\\customer_ole_drag.bmp",
        "--ole-drag-picture-target-unique-id", "one-guid",
        "--locked", "true",
        "--locked-target-unique-id", "one-guid"
    });
    expect(!ole_drag_picture_locked_result.ok,
        "#1101: launch contract should reject simultaneous OLE drag-picture-object and locked-object requests");

    const auto ole_drag_picture_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--ole-drag-picture-object",
        "--clear-property",
        "--property-name", "OLEDragPicture",
        "--ole-drag-picture", "forms\\customer_ole_drag.bmp",
        "--ole-drag-picture-target-unique-id", "one-guid"
    });
    expect(!ole_drag_picture_property_result.ok,
        "#1101: launch contract should reject OLE drag-picture-object combined with property commands");

    const auto stray_ole_drag_picture_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--ole-drag-picture", "forms\\customer_ole_drag.bmp"
    });
    expect(!stray_ole_drag_picture_result.ok,
        "#1101: launch contract should reject stray OLE drag-picture arguments");
}

void test_parse_launch_arguments_for_mouse_icon_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--mouse-icon-object",
        "--mouse-icon", "forms\\customer_mouse.cur",
        "--mouse-icon-target-object-name", "cmdSave",
        "--mouse-icon-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1102: launch contract should parse mouse-icon-object requests");
    expect(result.request.mouse_icon_object, "#1102: launch contract should detect --mouse-icon-object");
    expect(result.request.mouse_icon_available && result.request.mouse_icon == "forms\\customer_mouse.cur",
        "#1102: mouse-icon-object requests should carry mouse-icon text");
    expect(result.request.mouse_icon_objects.size() == 2U,
        "#1102: mouse-icon-object requests should collect mouse-icon target selectors");
    if (result.request.mouse_icon_objects.size() == 2U) {
        expect(result.request.mouse_icon_objects[0].object_name == "cmdSave" &&
                result.request.mouse_icon_objects[0].unique_id.empty(),
            "#1102: mouse-icon-object requests should parse target object-name selectors");
        expect(result.request.mouse_icon_objects[1].object_name.empty() &&
                result.request.mouse_icon_objects[1].unique_id == "two-guid",
            "#1102: mouse-icon-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_mouse_icon_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--mouse-icon-object",
        "--mouse-icon-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1102: launch contract should reject mouse-icon-object requests without mouse-icon text");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--mouse-icon-object",
        "--mouse-icon", "forms\\customer_mouse.cur"
    });
    expect(!missing_targets_result.ok,
        "#1102: launch contract should reject mouse-icon-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_mouse_icon_object_ambiguity() {
    const auto mouse_icon_locked_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--mouse-icon-object",
        "--locked-object",
        "--mouse-icon", "forms\\customer_mouse.cur",
        "--mouse-icon-target-unique-id", "one-guid",
        "--locked", "true",
        "--locked-target-unique-id", "one-guid"
    });
    expect(!mouse_icon_locked_result.ok,
        "#1102: launch contract should reject simultaneous mouse-icon-object and locked-object requests");

    const auto mouse_icon_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--mouse-icon-object",
        "--clear-property",
        "--property-name", "MouseIcon",
        "--mouse-icon", "forms\\customer_mouse.cur",
        "--mouse-icon-target-unique-id", "one-guid"
    });
    expect(!mouse_icon_property_result.ok,
        "#1102: launch contract should reject mouse-icon-object combined with property commands");

    const auto stray_mouse_icon_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--mouse-icon", "forms\\customer_mouse.cur"
    });
    expect(!stray_mouse_icon_result.ok,
        "#1102: launch contract should reject stray mouse-icon arguments");
}

void test_parse_launch_arguments_for_drag_icon_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--drag-icon-object",
        "--drag-icon", "forms\\customer_drag.cur",
        "--drag-icon-target-object-name", "cmdSave",
        "--drag-icon-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1103: launch contract should parse drag-icon-object requests");
    expect(result.request.drag_icon_object, "#1103: launch contract should detect --drag-icon-object");
    expect(result.request.drag_icon_available && result.request.drag_icon == "forms\\customer_drag.cur",
        "#1103: drag-icon-object requests should carry drag-icon text");
    expect(result.request.drag_icon_objects.size() == 2U,
        "#1103: drag-icon-object requests should collect drag-icon target selectors");
    if (result.request.drag_icon_objects.size() == 2U) {
        expect(result.request.drag_icon_objects[0].object_name == "cmdSave" &&
                result.request.drag_icon_objects[0].unique_id.empty(),
            "#1103: drag-icon-object requests should parse target object-name selectors");
        expect(result.request.drag_icon_objects[1].object_name.empty() &&
                result.request.drag_icon_objects[1].unique_id == "two-guid",
            "#1103: drag-icon-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_drag_icon_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--drag-icon-object",
        "--drag-icon-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1103: launch contract should reject drag-icon-object requests without drag-icon text");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--drag-icon-object",
        "--drag-icon", "forms\\customer_drag.cur"
    });
    expect(!missing_targets_result.ok,
        "#1103: launch contract should reject drag-icon-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_drag_icon_object_ambiguity() {
    const auto drag_icon_locked_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--drag-icon-object",
        "--locked-object",
        "--drag-icon", "forms\\customer_drag.cur",
        "--drag-icon-target-unique-id", "one-guid",
        "--locked", "true",
        "--locked-target-unique-id", "one-guid"
    });
    expect(!drag_icon_locked_result.ok,
        "#1103: launch contract should reject simultaneous drag-icon-object and locked-object requests");

    const auto drag_icon_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--drag-icon-object",
        "--clear-property",
        "--property-name", "DragIcon",
        "--drag-icon", "forms\\customer_drag.cur",
        "--drag-icon-target-unique-id", "one-guid"
    });
    expect(!drag_icon_property_result.ok,
        "#1103: launch contract should reject drag-icon-object combined with property commands");

    const auto stray_drag_icon_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--drag-icon", "forms\\customer_drag.cur"
    });
    expect(!stray_drag_icon_result.ok,
        "#1103: launch contract should reject stray drag-icon arguments");
}

void test_parse_launch_arguments_for_picture_margin_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--picture-margin-object",
        "--picture-margin", "2",
        "--picture-margin-target-object-name", "frmCustomer",
        "--picture-margin-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1172: launch contract should parse picture-margin-object requests");
    expect(result.request.picture_margin_object,
        "#1172: launch contract should detect --picture-margin-object");
    expect(result.request.picture_margin_available && result.request.picture_margin == 2,
        "#1172: picture-margin-object requests should carry picture-margin value");
    expect(result.request.picture_margin_objects.size() == 2U,
        "#1172: picture-margin-object requests should collect picture-margin target selectors");
    if (result.request.picture_margin_objects.size() == 2U) {
        expect(result.request.picture_margin_objects[0].object_name == "frmCustomer" &&
                result.request.picture_margin_objects[0].unique_id.empty(),
            "#1172: picture-margin-object requests should parse target object-name selectors");
        expect(result.request.picture_margin_objects[1].object_name.empty() &&
                result.request.picture_margin_objects[1].unique_id == "two-guid",
            "#1172: picture-margin-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_picture_margin_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--picture-margin-object",
        "--picture-margin-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1172: launch contract should reject picture-margin-object requests without picture-margin value");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--picture-margin-object",
        "--picture-margin", "2"
    });
    expect(!missing_targets_result.ok,
        "#1172: launch contract should reject picture-margin-object requests without target selectors");

    const auto non_integer_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--picture-margin-object",
        "--picture-margin", "wide",
        "--picture-margin-target-unique-id", "one-guid"
    });
    expect(!non_integer_result.ok,
        "#1172: launch contract should reject non-integer picture-margin values");

    const auto negative_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--picture-margin-object",
        "--picture-margin", "-1",
        "--picture-margin-target-unique-id", "one-guid"
    });
    expect(!negative_result.ok,
        "#1172: launch contract should reject negative picture-margin values");
}

void test_parse_launch_arguments_rejects_picture_margin_object_ambiguity() {
    const auto picture_margin_allow_output_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--picture-margin-object",
        "--allow-output-object",
        "--picture-margin", "2",
        "--picture-margin-target-unique-id", "one-guid",
        "--allow-output", "false",
        "--allow-output-target-unique-id", "one-guid"
    });
    expect(!picture_margin_allow_output_result.ok,
        "#1172: launch contract should reject simultaneous picture-margin-object and allow-output-object requests");

    const auto picture_margin_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--picture-margin-object",
        "--clear-property",
        "--property-name", "PictureMargin",
        "--picture-margin", "2",
        "--picture-margin-target-unique-id", "one-guid"
    });
    expect(!picture_margin_property_result.ok,
        "#1172: launch contract should reject picture-margin-object combined with property commands");

    const auto stray_picture_margin_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--picture-margin", "2"
    });
    expect(!stray_picture_margin_result.ok,
        "#1172: launch contract should reject stray picture-margin arguments");
}

void test_parse_launch_arguments_for_picture_position_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--picture-position-object",
        "--picture-position", "2",
        "--picture-position-target-object-name", "frmCustomer",
        "--picture-position-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1173: launch contract should parse picture-position-object requests");
    expect(result.request.picture_position_object,
        "#1173: launch contract should detect --picture-position-object");
    expect(result.request.picture_position_available && result.request.picture_position == 2,
        "#1173: picture-position-object requests should carry picture-position value");
    expect(result.request.picture_position_objects.size() == 2U,
        "#1173: picture-position-object requests should collect picture-position target selectors");
    if (result.request.picture_position_objects.size() == 2U) {
        expect(result.request.picture_position_objects[0].object_name == "frmCustomer" &&
                result.request.picture_position_objects[0].unique_id.empty(),
            "#1173: picture-position-object requests should parse target object-name selectors");
        expect(result.request.picture_position_objects[1].object_name.empty() &&
                result.request.picture_position_objects[1].unique_id == "two-guid",
            "#1173: picture-position-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_picture_position_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--picture-position-object",
        "--picture-position-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1173: launch contract should reject picture-position-object requests without picture-position value");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--picture-position-object",
        "--picture-position", "2"
    });
    expect(!missing_targets_result.ok,
        "#1173: launch contract should reject picture-position-object requests without target selectors");

    const auto non_integer_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--picture-position-object",
        "--picture-position", "center",
        "--picture-position-target-unique-id", "one-guid"
    });
    expect(!non_integer_result.ok,
        "#1173: launch contract should reject non-integer picture-position values");

    const auto negative_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--picture-position-object",
        "--picture-position", "-1",
        "--picture-position-target-unique-id", "one-guid"
    });
    expect(!negative_result.ok,
        "#1173: launch contract should reject negative picture-position values");
}

void test_parse_launch_arguments_rejects_picture_position_object_ambiguity() {
    const auto picture_position_allow_output_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--picture-position-object",
        "--allow-output-object",
        "--picture-position", "2",
        "--picture-position-target-unique-id", "one-guid",
        "--allow-output", "false",
        "--allow-output-target-unique-id", "one-guid"
    });
    expect(!picture_position_allow_output_result.ok,
        "#1173: launch contract should reject simultaneous picture-position-object and allow-output-object requests");

    const auto picture_position_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--picture-position-object",
        "--clear-property",
        "--property-name", "PicturePosition",
        "--picture-position", "2",
        "--picture-position-target-unique-id", "one-guid"
    });
    expect(!picture_position_property_result.ok,
        "#1173: launch contract should reject picture-position-object combined with property commands");

    const auto stray_picture_position_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--picture-position", "2"
    });
    expect(!stray_picture_position_result.ok,
        "#1173: launch contract should reject stray picture-position arguments");
}

void test_parse_launch_arguments_for_picture_spacing_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--picture-spacing-object",
        "--picture-spacing", "2",
        "--picture-spacing-target-object-name", "frmCustomer",
        "--picture-spacing-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1174: launch contract should parse picture-spacing-object requests");
    expect(result.request.picture_spacing_object,
        "#1174: launch contract should detect --picture-spacing-object");
    expect(result.request.picture_spacing_available && result.request.picture_spacing == 2,
        "#1174: picture-spacing-object requests should carry picture-spacing value");
    expect(result.request.picture_spacing_objects.size() == 2U,
        "#1174: picture-spacing-object requests should collect picture-spacing target selectors");
    if (result.request.picture_spacing_objects.size() == 2U) {
        expect(result.request.picture_spacing_objects[0].object_name == "frmCustomer" &&
                result.request.picture_spacing_objects[0].unique_id.empty(),
            "#1174: picture-spacing-object requests should parse target object-name selectors");
        expect(result.request.picture_spacing_objects[1].object_name.empty() &&
                result.request.picture_spacing_objects[1].unique_id == "two-guid",
            "#1174: picture-spacing-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_picture_spacing_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--picture-spacing-object",
        "--picture-spacing-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1174: launch contract should reject picture-spacing-object requests without picture-spacing value");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--picture-spacing-object",
        "--picture-spacing", "2"
    });
    expect(!missing_targets_result.ok,
        "#1174: launch contract should reject picture-spacing-object requests without target selectors");

    const auto non_integer_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--picture-spacing-object",
        "--picture-spacing", "gap",
        "--picture-spacing-target-unique-id", "one-guid"
    });
    expect(!non_integer_result.ok,
        "#1174: launch contract should reject non-integer picture-spacing values");

    const auto negative_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--picture-spacing-object",
        "--picture-spacing", "-1",
        "--picture-spacing-target-unique-id", "one-guid"
    });
    expect(!negative_result.ok,
        "#1174: launch contract should reject negative picture-spacing values");
}

void test_parse_launch_arguments_rejects_picture_spacing_object_ambiguity() {
    const auto picture_spacing_allow_output_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--picture-spacing-object",
        "--allow-output-object",
        "--picture-spacing", "2",
        "--picture-spacing-target-unique-id", "one-guid",
        "--allow-output", "false",
        "--allow-output-target-unique-id", "one-guid"
    });
    expect(!picture_spacing_allow_output_result.ok,
        "#1174: launch contract should reject simultaneous picture-spacing-object and allow-output-object requests");

    const auto picture_spacing_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--picture-spacing-object",
        "--clear-property",
        "--property-name", "PictureSpacing",
        "--picture-spacing", "2",
        "--picture-spacing-target-unique-id", "one-guid"
    });
    expect(!picture_spacing_property_result.ok,
        "#1174: launch contract should reject picture-spacing-object combined with property commands");

    const auto stray_picture_spacing_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--picture-spacing", "2"
    });
    expect(!stray_picture_spacing_result.ok,
        "#1174: launch contract should reject stray picture-spacing arguments");
}

void test_parse_launch_arguments_for_picture_selection_display_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--picture-selection-display-object",
        "--picture-selection-display", "2",
        "--picture-selection-display-target-object-name", "frmCustomer",
        "--picture-selection-display-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1175: launch contract should parse picture-selection-display-object requests");
    expect(result.request.picture_selection_display_object,
        "#1175: launch contract should detect --picture-selection-display-object");
    expect(result.request.picture_selection_display_available && result.request.picture_selection_display == 2,
        "#1175: picture-selection-display-object requests should carry picture-selection-display value");
    expect(result.request.picture_selection_display_objects.size() == 2U,
        "#1175: picture-selection-display-object requests should collect picture-selection-display target selectors");
    if (result.request.picture_selection_display_objects.size() == 2U) {
        expect(result.request.picture_selection_display_objects[0].object_name == "frmCustomer" &&
                result.request.picture_selection_display_objects[0].unique_id.empty(),
            "#1175: picture-selection-display-object requests should parse target object-name selectors");
        expect(result.request.picture_selection_display_objects[1].object_name.empty() &&
                result.request.picture_selection_display_objects[1].unique_id == "two-guid",
            "#1175: picture-selection-display-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_picture_selection_display_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--picture-selection-display-object",
        "--picture-selection-display-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1175: launch contract should reject picture-selection-display-object requests without picture-selection-display value");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--picture-selection-display-object",
        "--picture-selection-display", "2"
    });
    expect(!missing_targets_result.ok,
        "#1175: launch contract should reject picture-selection-display-object requests without target selectors");

    const auto non_integer_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--picture-selection-display-object",
        "--picture-selection-display", "selected",
        "--picture-selection-display-target-unique-id", "one-guid"
    });
    expect(!non_integer_result.ok,
        "#1175: launch contract should reject non-integer picture-selection-display values");

    const auto negative_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--picture-selection-display-object",
        "--picture-selection-display", "-1",
        "--picture-selection-display-target-unique-id", "one-guid"
    });
    expect(!negative_result.ok,
        "#1175: launch contract should reject negative picture-selection-display values");
}

void test_parse_launch_arguments_rejects_picture_selection_display_object_ambiguity() {
    const auto picture_selection_display_allow_output_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--picture-selection-display-object",
        "--allow-output-object",
        "--picture-selection-display", "2",
        "--picture-selection-display-target-unique-id", "one-guid",
        "--allow-output", "false",
        "--allow-output-target-unique-id", "one-guid"
    });
    expect(!picture_selection_display_allow_output_result.ok,
        "#1175: launch contract should reject simultaneous picture-selection-display-object and allow-output-object requests");

    const auto picture_selection_display_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--picture-selection-display-object",
        "--clear-property",
        "--property-name", "PictureSelectionDisplay",
        "--picture-selection-display", "2",
        "--picture-selection-display-target-unique-id", "one-guid"
    });
    expect(!picture_selection_display_property_result.ok,
        "#1175: launch contract should reject picture-selection-display-object combined with property commands");

    const auto stray_picture_selection_display_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--picture-selection-display", "2"
    });
    expect(!stray_picture_selection_display_result.ok,
        "#1175: launch contract should reject stray picture-selection-display arguments");
}

}  // namespace cf_test_studio_host
