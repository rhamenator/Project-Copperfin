// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_studio_host_support.h"

namespace cf_test_studio_host {
void test_parse_launch_arguments_for_drag_mode_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--drag-mode-object",
        "--drag-mode", "3",
        "--drag-mode-target-object-name", "cmdSave",
        "--drag-mode-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1104: launch contract should parse drag-mode-object requests");
    expect(result.request.drag_mode_object, "#1104: launch contract should detect --drag-mode-object");
    expect(result.request.drag_mode_available && result.request.drag_mode == 3,
        "#1104: drag-mode-object requests should carry drag-mode value");
    expect(result.request.drag_mode_objects.size() == 2U,
        "#1104: drag-mode-object requests should collect drag-mode target selectors");
    if (result.request.drag_mode_objects.size() == 2U) {
        expect(result.request.drag_mode_objects[0].object_name == "cmdSave" &&
                result.request.drag_mode_objects[0].unique_id.empty(),
            "#1104: drag-mode-object requests should parse target object-name selectors");
        expect(result.request.drag_mode_objects[1].object_name.empty() &&
                result.request.drag_mode_objects[1].unique_id == "two-guid",
            "#1104: drag-mode-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_drag_mode_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--drag-mode-object",
        "--drag-mode-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1104: launch contract should reject drag-mode-object requests without drag-mode value");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--drag-mode-object",
        "--drag-mode", "manual",
        "--drag-mode-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1104: launch contract should reject non-integer drag-mode values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--drag-mode-object",
        "--drag-mode", "-1",
        "--drag-mode-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1104: launch contract should reject negative drag-mode values before mutation");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--drag-mode-object",
        "--drag-mode", "2"
    });
    expect(!missing_targets_result.ok,
        "#1104: launch contract should reject drag-mode-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_drag_mode_object_ambiguity() {
    const auto drag_mode_locked_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--drag-mode-object",
        "--locked-object",
        "--drag-mode", "2",
        "--drag-mode-target-unique-id", "one-guid",
        "--locked", "true",
        "--locked-target-unique-id", "one-guid"
    });
    expect(!drag_mode_locked_result.ok,
        "#1104: launch contract should reject simultaneous drag-mode-object and locked-object requests");

    const auto drag_mode_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--drag-mode-object",
        "--clear-property",
        "--property-name", "DragMode",
        "--drag-mode", "2",
        "--drag-mode-target-unique-id", "one-guid"
    });
    expect(!drag_mode_property_result.ok,
        "#1104: launch contract should reject drag-mode-object combined with property commands");

    const auto stray_drag_mode_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--drag-mode", "2"
    });
    expect(!stray_drag_mode_result.ok,
        "#1104: launch contract should reject stray drag-mode arguments");
}

void test_parse_launch_arguments_for_ole_drag_mode_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--ole-drag-mode-object",
        "--ole-drag-mode", "3",
        "--ole-drag-mode-target-object-name", "cmdSave",
        "--ole-drag-mode-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1105: launch contract should parse OLE drag-mode-object requests");
    expect(result.request.ole_drag_mode_object,
        "#1105: launch contract should detect --ole-drag-mode-object");
    expect(result.request.ole_drag_mode_available && result.request.ole_drag_mode == 3,
        "#1105: OLE drag-mode-object requests should carry OLE drag-mode value");
    expect(result.request.ole_drag_mode_objects.size() == 2U,
        "#1105: OLE drag-mode-object requests should collect OLE drag-mode target selectors");
    if (result.request.ole_drag_mode_objects.size() == 2U) {
        expect(result.request.ole_drag_mode_objects[0].object_name == "cmdSave" &&
                result.request.ole_drag_mode_objects[0].unique_id.empty(),
            "#1105: OLE drag-mode-object requests should parse target object-name selectors");
        expect(result.request.ole_drag_mode_objects[1].object_name.empty() &&
                result.request.ole_drag_mode_objects[1].unique_id == "two-guid",
            "#1105: OLE drag-mode-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_ole_drag_mode_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--ole-drag-mode-object",
        "--ole-drag-mode-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1105: launch contract should reject OLE drag-mode-object requests without OLE drag-mode value");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--ole-drag-mode-object",
        "--ole-drag-mode", "manual",
        "--ole-drag-mode-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1105: launch contract should reject non-integer OLE drag-mode values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--ole-drag-mode-object",
        "--ole-drag-mode", "-1",
        "--ole-drag-mode-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1105: launch contract should reject negative OLE drag-mode values before mutation");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--ole-drag-mode-object",
        "--ole-drag-mode", "2"
    });
    expect(!missing_targets_result.ok,
        "#1105: launch contract should reject OLE drag-mode-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_ole_drag_mode_object_ambiguity() {
    const auto ole_drag_mode_locked_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--ole-drag-mode-object",
        "--locked-object",
        "--ole-drag-mode", "2",
        "--ole-drag-mode-target-unique-id", "one-guid",
        "--locked", "true",
        "--locked-target-unique-id", "one-guid"
    });
    expect(!ole_drag_mode_locked_result.ok,
        "#1105: launch contract should reject simultaneous OLE drag-mode-object and locked-object requests");

    const auto ole_drag_mode_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--ole-drag-mode-object",
        "--clear-property",
        "--property-name", "OLEDragMode",
        "--ole-drag-mode", "2",
        "--ole-drag-mode-target-unique-id", "one-guid"
    });
    expect(!ole_drag_mode_property_result.ok,
        "#1105: launch contract should reject OLE drag-mode-object combined with property commands");

    const auto stray_ole_drag_mode_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--ole-drag-mode", "2"
    });
    expect(!stray_ole_drag_mode_result.ok,
        "#1105: launch contract should reject stray OLE drag-mode arguments");
}

void test_parse_launch_arguments_for_ole_drop_mode_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--ole-drop-mode-object",
        "--ole-drop-mode", "3",
        "--ole-drop-mode-target-object-name", "cmdSave",
        "--ole-drop-mode-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1106: launch contract should parse OLE drop-mode-object requests");
    expect(result.request.ole_drop_mode_object,
        "#1106: launch contract should detect --ole-drop-mode-object");
    expect(result.request.ole_drop_mode_available && result.request.ole_drop_mode == 3,
        "#1106: OLE drop-mode-object requests should carry OLE drop-mode value");
    expect(result.request.ole_drop_mode_objects.size() == 2U,
        "#1106: OLE drop-mode-object requests should collect OLE drop-mode target selectors");
    if (result.request.ole_drop_mode_objects.size() == 2U) {
        expect(result.request.ole_drop_mode_objects[0].object_name == "cmdSave" &&
                result.request.ole_drop_mode_objects[0].unique_id.empty(),
            "#1106: OLE drop-mode-object requests should parse target object-name selectors");
        expect(result.request.ole_drop_mode_objects[1].object_name.empty() &&
                result.request.ole_drop_mode_objects[1].unique_id == "two-guid",
            "#1106: OLE drop-mode-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_ole_drop_mode_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--ole-drop-mode-object",
        "--ole-drop-mode-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1106: launch contract should reject OLE drop-mode-object requests without OLE drop-mode value");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--ole-drop-mode-object",
        "--ole-drop-mode", "manual",
        "--ole-drop-mode-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1106: launch contract should reject non-integer OLE drop-mode values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--ole-drop-mode-object",
        "--ole-drop-mode", "-1",
        "--ole-drop-mode-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1106: launch contract should reject negative OLE drop-mode values before mutation");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--ole-drop-mode-object",
        "--ole-drop-mode", "2"
    });
    expect(!missing_targets_result.ok,
        "#1106: launch contract should reject OLE drop-mode-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_ole_drop_mode_object_ambiguity() {
    const auto ole_drop_mode_locked_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--ole-drop-mode-object",
        "--locked-object",
        "--ole-drop-mode", "2",
        "--ole-drop-mode-target-unique-id", "one-guid",
        "--locked", "true",
        "--locked-target-unique-id", "one-guid"
    });
    expect(!ole_drop_mode_locked_result.ok,
        "#1106: launch contract should reject simultaneous OLE drop-mode-object and locked-object requests");

    const auto ole_drop_mode_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--ole-drop-mode-object",
        "--clear-property",
        "--property-name", "OLEDropMode",
        "--ole-drop-mode", "2",
        "--ole-drop-mode-target-unique-id", "one-guid"
    });
    expect(!ole_drop_mode_property_result.ok,
        "#1106: launch contract should reject OLE drop-mode-object combined with property commands");

    const auto stray_ole_drop_mode_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--ole-drop-mode", "2"
    });
    expect(!stray_ole_drop_mode_result.ok,
        "#1106: launch contract should reject stray OLE drop-mode arguments");
}

void test_parse_launch_arguments_for_ole_drop_effects_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--ole-drop-effects-object",
        "--ole-drop-effects", "3",
        "--ole-drop-effects-target-object-name", "cmdSave",
        "--ole-drop-effects-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1107: launch contract should parse OLE drop-effects-object requests");
    expect(result.request.ole_drop_effects_object,
        "#1107: launch contract should detect --ole-drop-effects-object");
    expect(result.request.ole_drop_effects_available && result.request.ole_drop_effects == 3,
        "#1107: OLE drop-effects-object requests should carry OLE drop-effects value");
    expect(result.request.ole_drop_effects_objects.size() == 2U,
        "#1107: OLE drop-effects-object requests should collect OLE drop-effects target selectors");
    if (result.request.ole_drop_effects_objects.size() == 2U) {
        expect(result.request.ole_drop_effects_objects[0].object_name == "cmdSave" &&
                result.request.ole_drop_effects_objects[0].unique_id.empty(),
            "#1107: OLE drop-effects-object requests should parse target object-name selectors");
        expect(result.request.ole_drop_effects_objects[1].object_name.empty() &&
                result.request.ole_drop_effects_objects[1].unique_id == "two-guid",
            "#1107: OLE drop-effects-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_ole_drop_effects_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--ole-drop-effects-object",
        "--ole-drop-effects-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1107: launch contract should reject OLE drop-effects-object requests without OLE drop-effects value");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--ole-drop-effects-object",
        "--ole-drop-effects", "manual",
        "--ole-drop-effects-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1107: launch contract should reject non-integer OLE drop-effects values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--ole-drop-effects-object",
        "--ole-drop-effects", "-1",
        "--ole-drop-effects-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1107: launch contract should reject negative OLE drop-effects values before mutation");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--ole-drop-effects-object",
        "--ole-drop-effects", "2"
    });
    expect(!missing_targets_result.ok,
        "#1107: launch contract should reject OLE drop-effects-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_ole_drop_effects_object_ambiguity() {
    const auto ole_drop_effects_locked_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--ole-drop-effects-object",
        "--locked-object",
        "--ole-drop-effects", "2",
        "--ole-drop-effects-target-unique-id", "one-guid",
        "--locked", "true",
        "--locked-target-unique-id", "one-guid"
    });
    expect(!ole_drop_effects_locked_result.ok,
        "#1107: launch contract should reject simultaneous OLE drop-effects-object and locked-object requests");

    const auto ole_drop_effects_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--ole-drop-effects-object",
        "--clear-property",
        "--property-name", "OLEDropEffects",
        "--ole-drop-effects", "2",
        "--ole-drop-effects-target-unique-id", "one-guid"
    });
    expect(!ole_drop_effects_property_result.ok,
        "#1107: launch contract should reject OLE drop-effects-object combined with property commands");

    const auto stray_ole_drop_effects_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--ole-drop-effects", "2"
    });
    expect(!stray_ole_drop_effects_result.ok,
        "#1107: launch contract should reject stray OLE drop-effects arguments");
}

void test_parse_launch_arguments_for_ole_drop_text_insertion_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--ole-drop-text-insertion-object",
        "--ole-drop-text-insertion", "3",
        "--ole-drop-text-insertion-target-object-name", "cmdSave",
        "--ole-drop-text-insertion-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1108: launch contract should parse OLE drop text-insertion-object requests");
    expect(result.request.ole_drop_text_insertion_object,
        "#1108: launch contract should detect --ole-drop-text-insertion-object");
    expect(result.request.ole_drop_text_insertion_available &&
            result.request.ole_drop_text_insertion == 3,
        "#1108: OLE drop text-insertion-object requests should carry OLE drop text-insertion value");
    expect(result.request.ole_drop_text_insertion_objects.size() == 2U,
        "#1108: OLE drop text-insertion-object requests should collect OLE drop text-insertion target selectors");
    if (result.request.ole_drop_text_insertion_objects.size() == 2U) {
        expect(result.request.ole_drop_text_insertion_objects[0].object_name == "cmdSave" &&
                result.request.ole_drop_text_insertion_objects[0].unique_id.empty(),
            "#1108: OLE drop text-insertion-object requests should parse target object-name selectors");
        expect(result.request.ole_drop_text_insertion_objects[1].object_name.empty() &&
                result.request.ole_drop_text_insertion_objects[1].unique_id == "two-guid",
            "#1108: OLE drop text-insertion-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_ole_drop_text_insertion_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--ole-drop-text-insertion-object",
        "--ole-drop-text-insertion-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1108: launch contract should reject OLE drop text-insertion-object requests without OLE drop text-insertion value");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--ole-drop-text-insertion-object",
        "--ole-drop-text-insertion", "manual",
        "--ole-drop-text-insertion-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1108: launch contract should reject non-integer OLE drop text-insertion values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--ole-drop-text-insertion-object",
        "--ole-drop-text-insertion", "-1",
        "--ole-drop-text-insertion-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1108: launch contract should reject negative OLE drop text-insertion values before mutation");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--ole-drop-text-insertion-object",
        "--ole-drop-text-insertion", "2"
    });
    expect(!missing_targets_result.ok,
        "#1108: launch contract should reject OLE drop text-insertion-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_ole_drop_text_insertion_object_ambiguity() {
    const auto ole_drop_text_insertion_locked_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--ole-drop-text-insertion-object",
        "--locked-object",
        "--ole-drop-text-insertion", "2",
        "--ole-drop-text-insertion-target-unique-id", "one-guid",
        "--locked", "true",
        "--locked-target-unique-id", "one-guid"
    });
    expect(!ole_drop_text_insertion_locked_result.ok,
        "#1108: launch contract should reject simultaneous OLE drop text-insertion-object and locked-object requests");

    const auto ole_drop_text_insertion_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--ole-drop-text-insertion-object",
        "--clear-property",
        "--property-name", "OLEDropTextInsertion",
        "--ole-drop-text-insertion", "2",
        "--ole-drop-text-insertion-target-unique-id", "one-guid"
    });
    expect(!ole_drop_text_insertion_property_result.ok,
        "#1108: launch contract should reject OLE drop text-insertion-object combined with property commands");

    const auto stray_ole_drop_text_insertion_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--ole-drop-text-insertion", "2"
    });
    expect(!stray_ole_drop_text_insertion_result.ok,
        "#1108: launch contract should reject stray OLE drop text-insertion arguments");
}

void test_parse_launch_arguments_for_buffer_mode_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--buffer-mode-object",
        "--buffer-mode", "9",
        "--buffer-mode-target-object-name", "cmdSave",
        "--buffer-mode-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1117: launch contract should parse buffer-mode-object requests");
    expect(result.request.buffer_mode_object,
        "#1117: launch contract should detect --buffer-mode-object");
    expect(result.request.buffer_mode_available && result.request.buffer_mode == 9,
        "#1117: buffer-mode-object requests should carry buffer-mode value");
    expect(result.request.buffer_mode_objects.size() == 2U,
        "#1117: buffer-mode-object requests should collect buffer-mode target selectors");
    if (result.request.buffer_mode_objects.size() == 2U) {
        expect(result.request.buffer_mode_objects[0].object_name == "cmdSave" &&
                result.request.buffer_mode_objects[0].unique_id.empty(),
            "#1117: buffer-mode-object requests should parse target object-name selectors");
        expect(result.request.buffer_mode_objects[1].object_name.empty() &&
                result.request.buffer_mode_objects[1].unique_id == "two-guid",
            "#1117: buffer-mode-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_buffer_mode_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--buffer-mode-object",
        "--buffer-mode-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1117: launch contract should reject buffer-mode-object requests without buffer-mode value");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--buffer-mode-object",
        "--buffer-mode", "manual",
        "--buffer-mode-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1117: launch contract should reject non-integer buffer-mode values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--buffer-mode-object",
        "--buffer-mode", "-1",
        "--buffer-mode-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1117: launch contract should reject negative buffer-mode values before mutation");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--buffer-mode-object",
        "--buffer-mode", "2"
    });
    expect(!missing_targets_result.ok,
        "#1117: launch contract should reject buffer-mode-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_buffer_mode_object_ambiguity() {
    const auto buffer_mode_locked_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--buffer-mode-object",
        "--locked-object",
        "--buffer-mode", "2",
        "--buffer-mode-target-unique-id", "one-guid",
        "--locked", "true",
        "--locked-target-unique-id", "one-guid"
    });
    expect(!buffer_mode_locked_result.ok,
        "#1117: launch contract should reject simultaneous buffer-mode-object and locked-object requests");

    const auto buffer_mode_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--buffer-mode-object",
        "--clear-property",
        "--property-name", "BufferMode",
        "--buffer-mode", "2",
        "--buffer-mode-target-unique-id", "one-guid"
    });
    expect(!buffer_mode_property_result.ok,
        "#1117: launch contract should reject buffer-mode-object combined with property commands");

    const auto stray_buffer_mode_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--buffer-mode", "2"
    });
    expect(!stray_buffer_mode_result.ok,
        "#1117: launch contract should reject stray buffer-mode arguments");
}

void test_parse_launch_arguments_for_buffer_mode_override_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--buffer-mode-override-object",
        "--buffer-mode-override", "9",
        "--buffer-mode-override-target-object-name", "cmdSave",
        "--buffer-mode-override-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1118: launch contract should parse buffer-mode-override-object requests");
    expect(result.request.buffer_mode_override_object,
        "#1118: launch contract should detect --buffer-mode-override-object");
    expect(result.request.buffer_mode_override_available && result.request.buffer_mode_override == 9,
        "#1118: buffer-mode-override-object requests should carry buffer-mode-override value");
    expect(result.request.buffer_mode_override_objects.size() == 2U,
        "#1118: buffer-mode-override-object requests should collect buffer-mode-override target selectors");
    if (result.request.buffer_mode_override_objects.size() == 2U) {
        expect(result.request.buffer_mode_override_objects[0].object_name == "cmdSave" &&
                result.request.buffer_mode_override_objects[0].unique_id.empty(),
            "#1118: buffer-mode-override-object requests should parse target object-name selectors");
        expect(result.request.buffer_mode_override_objects[1].object_name.empty() &&
                result.request.buffer_mode_override_objects[1].unique_id == "two-guid",
            "#1118: buffer-mode-override-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_buffer_mode_override_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--buffer-mode-override-object",
        "--buffer-mode-override-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1118: launch contract should reject buffer-mode-override-object requests without buffer-mode-override value");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--buffer-mode-override-object",
        "--buffer-mode-override", "manual",
        "--buffer-mode-override-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1118: launch contract should reject non-integer buffer-mode-override values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--buffer-mode-override-object",
        "--buffer-mode-override", "-1",
        "--buffer-mode-override-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1118: launch contract should reject negative buffer-mode-override values before mutation");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--buffer-mode-override-object",
        "--buffer-mode-override", "2"
    });
    expect(!missing_targets_result.ok,
        "#1118: launch contract should reject buffer-mode-override-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_buffer_mode_override_object_ambiguity() {
    const auto buffer_mode_override_locked_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--buffer-mode-override-object",
        "--locked-object",
        "--buffer-mode-override", "2",
        "--buffer-mode-override-target-unique-id", "one-guid",
        "--locked", "true",
        "--locked-target-unique-id", "one-guid"
    });
    expect(!buffer_mode_override_locked_result.ok,
        "#1118: launch contract should reject simultaneous buffer-mode-override-object and locked-object requests");

    const auto buffer_mode_override_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--buffer-mode-override-object",
        "--clear-property",
        "--property-name", "BufferModeOverride",
        "--buffer-mode-override", "2",
        "--buffer-mode-override-target-unique-id", "one-guid"
    });
    expect(!buffer_mode_override_property_result.ok,
        "#1118: launch contract should reject buffer-mode-override-object combined with property commands");

    const auto stray_buffer_mode_override_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--buffer-mode-override", "2"
    });
    expect(!stray_buffer_mode_override_result.ok,
        "#1118: launch contract should reject stray buffer-mode-override arguments");
}

}  // namespace cf_test_studio_host
