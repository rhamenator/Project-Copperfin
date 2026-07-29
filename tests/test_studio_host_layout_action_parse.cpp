// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "test_studio_host_support.h"
#include "../src/studio/vs_launch_contract_internal.h"

namespace cf_test_studio_host {
void test_launch_numeric_parsing_uses_invariant_decimal_text() {
    double value = 0.0;
    expect(copperfin::studio::parse_double_value("1.25", value) && value == 1.25,
           "VSIX/Studio launch numeric parsing should use the invariant period decimal separator");
    expect(copperfin::studio::parse_double_value("+1.25e2", value) && value == 125.0,
           "VSIX/Studio launch numeric parsing should preserve exponent and leading-plus forms");
    expect(!copperfin::studio::parse_double_value("1,25", value) &&
               !copperfin::studio::parse_double_value("1.25 trailing", value),
           "VSIX/Studio launch numeric parsing should reject comma-decimal and trailing-input forms");
}

void test_parse_launch_arguments_for_align_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--align-object",
        "--alignment-mode", "left",
        "--anchor-object-name", "cmdAnchor",
        "--align-target-object-name", "cmdOne",
        "--align-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1030: launch contract should parse align-object requests");
    expect(result.request.align_object, "#1030: launch contract should detect --align-object");
    expect(result.request.alignment_mode == "left",
        "#1030: align-object requests should preserve alignment modes");
    expect(result.request.anchor_object_name == "cmdAnchor" &&
            result.request.anchor_unique_id.empty(),
        "#1030: align-object requests should preserve anchor selectors");
    expect(result.request.align_objects.size() == 2U,
        "#1030: align-object requests should collect target selectors");
    if (result.request.align_objects.size() == 2U) {
        expect(result.request.align_objects[0].object_name == "cmdOne" &&
                result.request.align_objects[0].unique_id.empty(),
            "#1030: align-object requests should preserve object-name targets");
        expect(result.request.align_objects[1].object_name.empty() &&
                result.request.align_objects[1].unique_id == "two-guid",
            "#1030: align-object requests should preserve unique-id targets");
    }
}

void test_parse_launch_arguments_rejects_align_object_invalid_inputs() {
    const auto missing_mode_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--align-object",
        "--anchor-unique-id", "anchor-guid",
        "--align-target-unique-id", "one-guid"
    });
    expect(!missing_mode_result.ok,
        "#1030: launch contract should reject align-object requests without alignment mode");

    const auto missing_anchor_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--align-object",
        "--alignment-mode", "left",
        "--align-target-unique-id", "one-guid"
    });
    expect(!missing_anchor_result.ok,
        "#1030: launch contract should reject align-object requests without anchor selectors");

    const auto missing_target_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--align-object",
        "--alignment-mode", "left",
        "--anchor-unique-id", "anchor-guid"
    });
    expect(!missing_target_result.ok,
        "#1030: launch contract should reject align-object requests without target selectors");

    const auto stray_mode_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--alignment-mode", "left"
    });
    expect(!stray_mode_result.ok,
        "#1030: launch contract should reject stray align-object arguments");
}

void test_parse_launch_arguments_rejects_align_object_ambiguity() {
    const auto align_resize_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--align-object",
        "--alignment-mode", "left",
        "--anchor-unique-id", "anchor-guid",
        "--align-target-unique-id", "one-guid",
        "--resize-object",
        "--resize-mode", "width",
        "--resize-target-unique-id", "one-guid"
    });
    expect(!align_resize_result.ok,
        "#1030: launch contract should reject simultaneous align-object and resize-object requests");

    const auto align_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--align-object",
        "--alignment-mode", "left",
        "--anchor-unique-id", "anchor-guid",
        "--align-target-unique-id", "one-guid",
        "--clear-property",
        "--property-name", "Caption"
    });
    expect(!align_property_result.ok,
        "#1030: launch contract should reject align-object combined with property commands");

    const auto stray_anchor_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--anchor-unique-id", "anchor-guid"
    });
    expect(!stray_anchor_result.ok,
        "#1030: launch contract should reject anchor selectors without align-object or resize-object");
}

void test_parse_launch_arguments_for_resize_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--resize-object",
        "--resize-mode", "width",
        "--anchor-unique-id", "anchor-guid",
        "--resize-target-object-name", "cmdOne",
        "--resize-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1031: launch contract should parse resize-object requests");
    expect(result.request.resize_object, "#1031: launch contract should detect --resize-object");
    expect(result.request.resize_mode == "width",
        "#1031: resize-object requests should preserve resize modes");
    expect(result.request.anchor_object_name.empty() &&
            result.request.anchor_unique_id == "anchor-guid",
        "#1031: resize-object requests should preserve anchor selectors");
    expect(result.request.resize_objects.size() == 2U,
        "#1031: resize-object requests should collect target selectors");
    if (result.request.resize_objects.size() == 2U) {
        expect(result.request.resize_objects[0].object_name == "cmdOne" &&
                result.request.resize_objects[0].unique_id.empty(),
            "#1031: resize-object requests should preserve object-name targets");
        expect(result.request.resize_objects[1].object_name.empty() &&
                result.request.resize_objects[1].unique_id == "two-guid",
            "#1031: resize-object requests should preserve unique-id targets");
    }
}

void test_parse_launch_arguments_rejects_resize_object_invalid_inputs() {
    const auto missing_mode_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--resize-object",
        "--anchor-unique-id", "anchor-guid",
        "--resize-target-unique-id", "one-guid"
    });
    expect(!missing_mode_result.ok,
        "#1031: launch contract should reject resize-object requests without resize mode");

    const auto missing_anchor_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--resize-object",
        "--resize-mode", "width",
        "--resize-target-unique-id", "one-guid"
    });
    expect(!missing_anchor_result.ok,
        "#1031: launch contract should reject resize-object requests without anchor selectors");

    const auto missing_target_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--resize-object",
        "--resize-mode", "width",
        "--anchor-unique-id", "anchor-guid"
    });
    expect(!missing_target_result.ok,
        "#1031: launch contract should reject resize-object requests without target selectors");

    const auto stray_mode_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--resize-mode", "width"
    });
    expect(!stray_mode_result.ok,
        "#1031: launch contract should reject stray resize-object arguments");
}

void test_parse_launch_arguments_rejects_resize_object_ambiguity() {
    const auto resize_distribute_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--resize-object",
        "--resize-mode", "width",
        "--anchor-unique-id", "anchor-guid",
        "--resize-target-unique-id", "one-guid",
        "--distribute-object",
        "--distribution-mode", "horizontal-spacing",
        "--distribute-target-unique-id", "one-guid",
        "--distribute-target-unique-id", "two-guid"
    });
    expect(!resize_distribute_result.ok,
        "#1031: launch contract should reject simultaneous resize-object and distribute-object requests");

    const auto resize_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--resize-object",
        "--resize-mode", "width",
        "--anchor-unique-id", "anchor-guid",
        "--resize-target-unique-id", "one-guid",
        "--clear-property",
        "--property-name", "Caption"
    });
    expect(!resize_property_result.ok,
        "#1031: launch contract should reject resize-object combined with property commands");
}

void test_parse_launch_arguments_for_distribute_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--distribute-object",
        "--distribution-mode", "horizontal-spacing",
        "--distribute-target-object-name", "cmdOne",
        "--distribute-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1032: launch contract should parse distribute-object requests");
    expect(result.request.distribute_object,
        "#1032: launch contract should detect --distribute-object");
    expect(result.request.distribution_mode == "horizontal-spacing",
        "#1032: distribute-object requests should preserve distribution modes");
    expect(result.request.distribute_objects.size() == 2U,
        "#1032: distribute-object requests should collect target selectors");
    if (result.request.distribute_objects.size() == 2U) {
        expect(result.request.distribute_objects[0].object_name == "cmdOne" &&
                result.request.distribute_objects[0].unique_id.empty(),
            "#1032: distribute-object requests should preserve object-name targets");
        expect(result.request.distribute_objects[1].object_name.empty() &&
                result.request.distribute_objects[1].unique_id == "two-guid",
            "#1032: distribute-object requests should preserve unique-id targets");
    }
}

void test_parse_launch_arguments_rejects_distribute_object_invalid_inputs() {
    const auto missing_mode_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--distribute-object",
        "--distribute-target-unique-id", "one-guid",
        "--distribute-target-unique-id", "two-guid"
    });
    expect(!missing_mode_result.ok,
        "#1032: launch contract should reject distribute-object requests without distribution mode");

    const auto missing_target_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--distribute-object",
        "--distribution-mode", "horizontal-spacing"
    });
    expect(!missing_target_result.ok,
        "#1032: launch contract should reject distribute-object requests without target selectors");

    const auto stray_mode_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--distribution-mode", "horizontal-spacing"
    });
    expect(!stray_mode_result.ok,
        "#1032: launch contract should reject stray distribute-object arguments");
}

void test_parse_launch_arguments_rejects_distribute_object_ambiguity() {
    const auto distribute_snap_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--distribute-object",
        "--distribution-mode", "horizontal-spacing",
        "--distribute-target-unique-id", "one-guid",
        "--distribute-target-unique-id", "two-guid",
        "--snap-object",
        "--snap-mode", "nearest-grid",
        "--grid-width", "10",
        "--grid-height", "8",
        "--snap-target-unique-id", "one-guid"
    });
    expect(!distribute_snap_result.ok,
        "#1032: launch contract should reject simultaneous distribute-object and snap-object requests");

    const auto distribute_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--distribute-object",
        "--distribution-mode", "horizontal-spacing",
        "--distribute-target-unique-id", "one-guid",
        "--distribute-target-unique-id", "two-guid",
        "--clear-property",
        "--property-name", "Caption"
    });
    expect(!distribute_property_result.ok,
        "#1032: launch contract should reject distribute-object combined with property commands");
}

void test_parse_launch_arguments_for_snap_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--snap-object",
        "--snap-mode", "nearest-grid",
        "--grid-width", "10",
        "--grid-height", "8",
        "--snap-target-object-name", "cmdOne",
        "--snap-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1033: launch contract should parse snap-object requests");
    expect(result.request.snap_object, "#1033: launch contract should detect --snap-object");
    expect(result.request.snap_mode == "nearest-grid",
        "#1033: snap-object requests should preserve snap modes");
    expect(result.request.grid_width == 10.0 && result.request.grid_height == 8.0,
        "#1033: snap-object requests should preserve grid dimensions");
    expect(result.request.snap_objects.size() == 2U,
        "#1033: snap-object requests should collect target selectors");
    if (result.request.snap_objects.size() == 2U) {
        expect(result.request.snap_objects[0].object_name == "cmdOne" &&
                result.request.snap_objects[0].unique_id.empty(),
            "#1033: snap-object requests should preserve object-name targets");
        expect(result.request.snap_objects[1].object_name.empty() &&
                result.request.snap_objects[1].unique_id == "two-guid",
            "#1033: snap-object requests should preserve unique-id targets");
    }
}

void test_parse_launch_arguments_rejects_snap_object_invalid_inputs() {
    const auto missing_mode_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--snap-object",
        "--snap-target-unique-id", "one-guid"
    });
    expect(!missing_mode_result.ok,
        "#1033: launch contract should reject snap-object requests without snap mode");

    const auto missing_target_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--snap-object",
        "--snap-mode", "nearest-grid",
        "--grid-width", "10"
    });
    expect(!missing_target_result.ok,
        "#1033: launch contract should reject snap-object requests without target selectors");

    const auto invalid_width_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--snap-object",
        "--snap-mode", "nearest-grid",
        "--grid-width", "wide",
        "--snap-target-unique-id", "one-guid"
    });
    expect(!invalid_width_result.ok,
        "#1033: launch contract should reject non-numeric grid-width values");

    const auto stray_grid_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--grid-width", "10"
    });
    expect(!stray_grid_result.ok,
        "#1033: launch contract should reject stray snap-object arguments");
}

void test_parse_launch_arguments_rejects_snap_object_ambiguity() {
    const auto snap_nudge_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--snap-object",
        "--snap-mode", "nearest-grid",
        "--grid-width", "10",
        "--snap-target-unique-id", "one-guid",
        "--nudge-object",
        "--nudge-mode", "horizontal",
        "--delta-hpos", "1",
        "--nudge-target-unique-id", "one-guid"
    });
    expect(!snap_nudge_result.ok,
        "#1033: launch contract should reject simultaneous snap-object and nudge-object requests");

    const auto snap_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--snap-object",
        "--snap-mode", "nearest-grid",
        "--grid-width", "10",
        "--snap-target-unique-id", "one-guid",
        "--clear-property",
        "--property-name", "Caption"
    });
    expect(!snap_property_result.ok,
        "#1033: launch contract should reject snap-object combined with property commands");
}

void test_parse_launch_arguments_for_nudge_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--nudge-object",
        "--nudge-mode", "horizontal",
        "--delta-hpos", "2.5",
        "--delta-vpos", "-1.5",
        "--nudge-target-object-name", "cmdOne",
        "--nudge-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1034: launch contract should parse nudge-object requests");
    expect(result.request.nudge_object, "#1034: launch contract should detect --nudge-object");
    expect(result.request.nudge_mode == "horizontal",
        "#1034: nudge-object requests should preserve nudge modes");
    expect(result.request.delta_hpos == 2.5 && result.request.delta_vpos == -1.5,
        "#1034: nudge-object requests should preserve deltas");
    expect(result.request.nudge_objects.size() == 2U,
        "#1034: nudge-object requests should collect target selectors");
    if (result.request.nudge_objects.size() == 2U) {
        expect(result.request.nudge_objects[0].object_name == "cmdOne" &&
                result.request.nudge_objects[0].unique_id.empty(),
            "#1034: nudge-object requests should preserve object-name targets");
        expect(result.request.nudge_objects[1].object_name.empty() &&
                result.request.nudge_objects[1].unique_id == "two-guid",
            "#1034: nudge-object requests should preserve unique-id targets");
    }
}

void test_parse_launch_arguments_rejects_nudge_object_invalid_inputs() {
    const auto missing_mode_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--nudge-object",
        "--delta-hpos", "2",
        "--nudge-target-unique-id", "one-guid"
    });
    expect(!missing_mode_result.ok,
        "#1034: launch contract should reject nudge-object requests without nudge mode");

    const auto missing_target_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--nudge-object",
        "--nudge-mode", "horizontal",
        "--delta-hpos", "2"
    });
    expect(!missing_target_result.ok,
        "#1034: launch contract should reject nudge-object requests without target selectors");

    const auto invalid_delta_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--nudge-object",
        "--nudge-mode", "horizontal",
        "--delta-hpos", "far",
        "--nudge-target-unique-id", "one-guid"
    });
    expect(!invalid_delta_result.ok,
        "#1034: launch contract should reject non-numeric nudge deltas");

    const auto stray_delta_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--delta-vpos", "1"
    });
    expect(!stray_delta_result.ok,
        "#1034: launch contract should reject stray nudge-object arguments");
}

void test_parse_launch_arguments_rejects_nudge_object_ambiguity() {
    const auto nudge_tab_order_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--nudge-object",
        "--nudge-mode", "horizontal",
        "--delta-hpos", "1",
        "--nudge-target-unique-id", "one-guid",
        "--tab-order-object",
        "--starting-tab-index", "1",
        "--tab-order-target-unique-id", "one-guid"
    });
    expect(!nudge_tab_order_result.ok,
        "#1034: launch contract should reject simultaneous nudge-object and tab-order-object requests");

    const auto nudge_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--nudge-object",
        "--nudge-mode", "horizontal",
        "--delta-hpos", "1",
        "--nudge-target-unique-id", "one-guid",
        "--clear-property",
        "--property-name", "Caption"
    });
    expect(!nudge_property_result.ok,
        "#1034: launch contract should reject nudge-object combined with property commands");
}

void test_parse_launch_arguments_for_dynamic_alignment_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--dynamic-alignment-object",
        "--dynamic-alignment", "IIF(lLocked, 2, 0)",
        "--dynamic-alignment-target-object-name", "cmdOne",
        "--dynamic-alignment-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1186: launch contract should parse dynamic-alignment-object requests");
    expect(result.request.dynamic_alignment_object,
        "#1186: launch contract should detect --dynamic-alignment-object");
    expect(result.request.dynamic_alignment_available &&
            result.request.dynamic_alignment == "IIF(lLocked, 2, 0)",
        "#1186: dynamic-alignment-object requests should preserve dynamic alignment expressions");
    expect(result.request.dynamic_alignment_objects.size() == 2U,
        "#1186: dynamic-alignment-object requests should collect target selectors");
    if (result.request.dynamic_alignment_objects.size() == 2U) {
        expect(result.request.dynamic_alignment_objects[0].object_name == "cmdOne" &&
                result.request.dynamic_alignment_objects[0].unique_id.empty(),
            "#1186: dynamic-alignment-object requests should preserve object-name targets");
        expect(result.request.dynamic_alignment_objects[1].object_name.empty() &&
                result.request.dynamic_alignment_objects[1].unique_id == "two-guid",
            "#1186: dynamic-alignment-object requests should preserve unique-id targets");
    }
}

void test_parse_launch_arguments_rejects_dynamic_alignment_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-alignment-object",
        "--dynamic-alignment-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1186: launch contract should reject dynamic-alignment-object requests without expression values");

    const auto missing_target_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-alignment-object",
        "--dynamic-alignment", "IIF(lLocked, 2, 0)"
    });
    expect(!missing_target_result.ok,
        "#1186: launch contract should reject dynamic-alignment-object requests without target selectors");

    const auto stray_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-alignment", "IIF(lLocked, 2, 0)"
    });
    expect(!stray_value_result.ok,
        "#1186: launch contract should reject stray dynamic-alignment arguments");
}

void test_parse_launch_arguments_rejects_dynamic_alignment_object_ambiguity() {
    const auto dynamic_alignment_caption_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-alignment-object",
        "--dynamic-alignment", "IIF(lLocked, 2, 0)",
        "--dynamic-alignment-target-unique-id", "one-guid",
        "--caption-object",
        "--caption", "New caption",
        "--caption-target-unique-id", "one-guid"
    });
    expect(!dynamic_alignment_caption_result.ok,
        "#1186: launch contract should reject simultaneous dynamic-alignment-object and caption-object requests");

    const auto dynamic_alignment_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-alignment-object",
        "--dynamic-alignment", "IIF(lLocked, 2, 0)",
        "--dynamic-alignment-target-unique-id", "one-guid",
        "--clear-property",
        "--property-name", "Caption"
    });
    expect(!dynamic_alignment_property_result.ok,
        "#1186: launch contract should reject dynamic-alignment-object combined with property commands");
}

}  // namespace cf_test_studio_host
