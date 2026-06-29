#include "test_studio_host_support.h"

namespace cf_test_studio_host {
void test_parse_launch_arguments_for_align_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--align-object",
        "--alignment-mode", "left",
        "--anchor-unique-id", "anchor-guid",
        "--align-target-object-name", "txtName",
        "--align-target-unique-id", "status-guid"
    });

    expect(result.ok, "#1031: launch contract should parse align-object requests");
    expect(result.request.align_object, "#1031: launch contract should detect --align-object");
    expect(result.request.alignment_mode == "left",
        "#1031: align-object requests should carry alignment mode");
    expect(result.request.anchor_unique_id == "anchor-guid",
        "#1031: align-object requests should carry anchor unique-id selectors");
    expect(result.request.align_objects.size() == 2U,
        "#1031: align-object requests should collect alignment target selectors");
    if (result.request.align_objects.size() == 2U) {
        expect(result.request.align_objects[0].object_name == "txtName" &&
                result.request.align_objects[0].unique_id.empty(),
            "#1031: align-object requests should parse target object-name selectors");
        expect(result.request.align_objects[1].object_name.empty() &&
                result.request.align_objects[1].unique_id == "status-guid",
            "#1031: align-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_align_object_invalid_inputs() {
    const auto missing_mode_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--align-object",
        "--anchor-unique-id", "anchor-guid",
        "--align-target-unique-id", "name-guid"
    });
    expect(!missing_mode_result.ok,
        "#1031: launch contract should reject align-object requests without alignment mode");

    const auto missing_anchor_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--align-object",
        "--alignment-mode", "left",
        "--align-target-unique-id", "name-guid"
    });
    expect(!missing_anchor_result.ok,
        "#1031: launch contract should reject align-object requests without anchor selectors");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--align-object",
        "--alignment-mode", "left",
        "--anchor-object-name", "cmdAnchor"
    });
    expect(!missing_targets_result.ok,
        "#1031: launch contract should reject align-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_align_object_ambiguity() {
    const auto align_group_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--align-object",
        "--group-object",
        "--alignment-mode", "left",
        "--anchor-unique-id", "anchor-guid",
        "--align-target-unique-id", "name-guid",
        "--field-value", "OBJNAME=cntGroup",
        "--group-child-unique-id", "name-guid"
    });
    expect(!align_group_result.ok,
        "#1031: launch contract should reject simultaneous align-object and group-object requests");

    const auto align_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--align-object",
        "--clear-property",
        "--property-name", "Caption",
        "--alignment-mode", "left",
        "--anchor-unique-id", "anchor-guid",
        "--align-target-unique-id", "name-guid"
    });
    expect(!align_property_result.ok,
        "#1031: launch contract should reject align-object combined with property commands");

    const auto stray_alignment_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--alignment-mode", "left"
    });
    expect(!stray_alignment_result.ok,
        "#1031: launch contract should reject stray alignment arguments");
}

void test_parse_launch_arguments_for_resize_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--resize-object",
        "--resize-mode", "width",
        "--anchor-unique-id", "anchor-guid",
        "--resize-target-object-name", "txtName",
        "--resize-target-unique-id", "status-guid"
    });

    expect(result.ok, "#1032: launch contract should parse resize-object requests");
    expect(result.request.resize_object, "#1032: launch contract should detect --resize-object");
    expect(result.request.resize_mode == "width",
        "#1032: resize-object requests should carry resize mode");
    expect(result.request.anchor_unique_id == "anchor-guid",
        "#1032: resize-object requests should carry anchor unique-id selectors");
    expect(result.request.resize_objects.size() == 2U,
        "#1032: resize-object requests should collect resize target selectors");
    if (result.request.resize_objects.size() == 2U) {
        expect(result.request.resize_objects[0].object_name == "txtName" &&
                result.request.resize_objects[0].unique_id.empty(),
            "#1032: resize-object requests should parse target object-name selectors");
        expect(result.request.resize_objects[1].object_name.empty() &&
                result.request.resize_objects[1].unique_id == "status-guid",
            "#1032: resize-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_resize_object_invalid_inputs() {
    const auto missing_mode_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--resize-object",
        "--anchor-unique-id", "anchor-guid",
        "--resize-target-unique-id", "name-guid"
    });
    expect(!missing_mode_result.ok,
        "#1032: launch contract should reject resize-object requests without resize mode");

    const auto missing_anchor_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--resize-object",
        "--resize-mode", "width",
        "--resize-target-unique-id", "name-guid"
    });
    expect(!missing_anchor_result.ok,
        "#1032: launch contract should reject resize-object requests without anchor selectors");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--resize-object",
        "--resize-mode", "width",
        "--anchor-object-name", "cmdAnchor"
    });
    expect(!missing_targets_result.ok,
        "#1032: launch contract should reject resize-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_resize_object_ambiguity() {
    const auto resize_align_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--resize-object",
        "--align-object",
        "--resize-mode", "width",
        "--alignment-mode", "left",
        "--anchor-unique-id", "anchor-guid",
        "--resize-target-unique-id", "name-guid",
        "--align-target-unique-id", "name-guid"
    });
    expect(!resize_align_result.ok,
        "#1032: launch contract should reject simultaneous resize-object and align-object requests");

    const auto resize_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--resize-object",
        "--clear-property",
        "--property-name", "Caption",
        "--resize-mode", "width",
        "--anchor-unique-id", "anchor-guid",
        "--resize-target-unique-id", "name-guid"
    });
    expect(!resize_property_result.ok,
        "#1032: launch contract should reject resize-object combined with property commands");

    const auto stray_resize_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--resize-mode", "width"
    });
    expect(!stray_resize_result.ok,
        "#1032: launch contract should reject stray resize arguments");
}

void test_parse_launch_arguments_for_distribute_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--distribute-object",
        "--distribution-mode", "horizontal",
        "--distribute-target-object-name", "cmdLeft",
        "--distribute-target-unique-id", "middle-guid",
        "--distribute-target-object-name", "cmdRight"
    });

    expect(result.ok, "#1033: launch contract should parse distribute-object requests");
    expect(result.request.distribute_object, "#1033: launch contract should detect --distribute-object");
    expect(result.request.distribution_mode == "horizontal",
        "#1033: distribute-object requests should carry distribution mode");
    expect(result.request.distribute_objects.size() == 3U,
        "#1033: distribute-object requests should collect distribution target selectors");
    if (result.request.distribute_objects.size() == 3U) {
        expect(result.request.distribute_objects[0].object_name == "cmdLeft" &&
                result.request.distribute_objects[0].unique_id.empty(),
            "#1033: distribute-object requests should parse target object-name selectors");
        expect(result.request.distribute_objects[1].object_name.empty() &&
                result.request.distribute_objects[1].unique_id == "middle-guid",
            "#1033: distribute-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_distribute_object_invalid_inputs() {
    const auto missing_mode_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--distribute-object",
        "--distribute-target-unique-id", "left-guid",
        "--distribute-target-unique-id", "middle-guid",
        "--distribute-target-unique-id", "right-guid"
    });
    expect(!missing_mode_result.ok,
        "#1033: launch contract should reject distribute-object requests without distribution mode");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--distribute-object",
        "--distribution-mode", "horizontal"
    });
    expect(!missing_targets_result.ok,
        "#1033: launch contract should reject distribute-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_distribute_object_ambiguity() {
    const auto distribute_resize_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--distribute-object",
        "--resize-object",
        "--distribution-mode", "horizontal",
        "--resize-mode", "width",
        "--anchor-unique-id", "anchor-guid",
        "--distribute-target-unique-id", "left-guid",
        "--distribute-target-unique-id", "middle-guid",
        "--distribute-target-unique-id", "right-guid",
        "--resize-target-unique-id", "middle-guid"
    });
    expect(!distribute_resize_result.ok,
        "#1033: launch contract should reject simultaneous distribute-object and resize-object requests");

    const auto distribute_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--distribute-object",
        "--clear-property",
        "--property-name", "Caption",
        "--distribution-mode", "horizontal",
        "--distribute-target-unique-id", "left-guid",
        "--distribute-target-unique-id", "middle-guid",
        "--distribute-target-unique-id", "right-guid"
    });
    expect(!distribute_property_result.ok,
        "#1033: launch contract should reject distribute-object combined with property commands");

    const auto stray_distribution_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--distribution-mode", "horizontal"
    });
    expect(!stray_distribution_result.ok,
        "#1033: launch contract should reject stray distribution arguments");
}

void test_parse_launch_arguments_for_snap_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--snap-object",
        "--snap-mode", "both",
        "--grid-width", "10.5",
        "--grid-height", "25",
        "--snap-target-object-name", "cmdOne",
        "--snap-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1034: launch contract should parse snap-object requests");
    expect(result.request.snap_object, "#1034: launch contract should detect --snap-object");
    expect(result.request.snap_mode == "both",
        "#1034: snap-object requests should carry snap mode");
    expect(result.request.grid_width == 10.5 && result.request.grid_height == 25.0,
        "#1034: snap-object requests should carry numeric grid dimensions");
    expect(result.request.snap_objects.size() == 2U,
        "#1034: snap-object requests should collect snap target selectors");
    if (result.request.snap_objects.size() == 2U) {
        expect(result.request.snap_objects[0].object_name == "cmdOne" &&
                result.request.snap_objects[0].unique_id.empty(),
            "#1034: snap-object requests should parse target object-name selectors");
        expect(result.request.snap_objects[1].object_name.empty() &&
                result.request.snap_objects[1].unique_id == "two-guid",
            "#1034: snap-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_snap_object_invalid_inputs() {
    const auto missing_mode_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--snap-object",
        "--grid-width", "10",
        "--snap-target-unique-id", "one-guid"
    });
    expect(!missing_mode_result.ok,
        "#1034: launch contract should reject snap-object requests without snap mode");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--snap-object",
        "--snap-mode", "horizontal",
        "--grid-width", "10"
    });
    expect(!missing_targets_result.ok,
        "#1034: launch contract should reject snap-object requests without target selectors");

    const auto invalid_grid_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--snap-object",
        "--snap-mode", "horizontal",
        "--grid-width", "wide",
        "--snap-target-unique-id", "one-guid"
    });
    expect(!invalid_grid_result.ok,
        "#1034: launch contract should reject non-numeric grid widths");
}

void test_parse_launch_arguments_rejects_snap_object_ambiguity() {
    const auto snap_distribute_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--snap-object",
        "--distribute-object",
        "--snap-mode", "horizontal",
        "--grid-width", "10",
        "--distribution-mode", "horizontal",
        "--snap-target-unique-id", "one-guid",
        "--distribute-target-unique-id", "one-guid",
        "--distribute-target-unique-id", "two-guid",
        "--distribute-target-unique-id", "three-guid"
    });
    expect(!snap_distribute_result.ok,
        "#1034: launch contract should reject simultaneous snap-object and distribute-object requests");

    const auto snap_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--snap-object",
        "--clear-property",
        "--property-name", "Caption",
        "--snap-mode", "horizontal",
        "--grid-width", "10",
        "--snap-target-unique-id", "one-guid"
    });
    expect(!snap_property_result.ok,
        "#1034: launch contract should reject snap-object combined with property commands");

    const auto stray_snap_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--snap-mode", "horizontal"
    });
    expect(!stray_snap_result.ok,
        "#1034: launch contract should reject stray snap arguments");
}

void test_parse_launch_arguments_for_nudge_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--nudge-object",
        "--nudge-mode", "both",
        "--delta-hpos", "5.5",
        "--delta-vpos", "-2",
        "--nudge-target-object-name", "cmdOne",
        "--nudge-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1035: launch contract should parse nudge-object requests");
    expect(result.request.nudge_object, "#1035: launch contract should detect --nudge-object");
    expect(result.request.nudge_mode == "both",
        "#1035: nudge-object requests should carry nudge mode");
    expect(result.request.delta_hpos == 5.5 && result.request.delta_vpos == -2.0,
        "#1035: nudge-object requests should carry numeric deltas");
    expect(result.request.nudge_objects.size() == 2U,
        "#1035: nudge-object requests should collect nudge target selectors");
    if (result.request.nudge_objects.size() == 2U) {
        expect(result.request.nudge_objects[0].object_name == "cmdOne" &&
                result.request.nudge_objects[0].unique_id.empty(),
            "#1035: nudge-object requests should parse target object-name selectors");
        expect(result.request.nudge_objects[1].object_name.empty() &&
                result.request.nudge_objects[1].unique_id == "two-guid",
            "#1035: nudge-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_nudge_object_invalid_inputs() {
    const auto missing_mode_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--nudge-object",
        "--delta-hpos", "1",
        "--nudge-target-unique-id", "one-guid"
    });
    expect(!missing_mode_result.ok,
        "#1035: launch contract should reject nudge-object requests without nudge mode");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--nudge-object",
        "--nudge-mode", "horizontal",
        "--delta-hpos", "1"
    });
    expect(!missing_targets_result.ok,
        "#1035: launch contract should reject nudge-object requests without target selectors");

    const auto invalid_delta_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--nudge-object",
        "--nudge-mode", "horizontal",
        "--delta-hpos", "right",
        "--nudge-target-unique-id", "one-guid"
    });
    expect(!invalid_delta_result.ok,
        "#1035: launch contract should reject non-numeric horizontal deltas");
}

void test_parse_launch_arguments_rejects_nudge_object_ambiguity() {
    const auto nudge_snap_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--nudge-object",
        "--snap-object",
        "--nudge-mode", "horizontal",
        "--delta-hpos", "1",
        "--snap-mode", "horizontal",
        "--grid-width", "10",
        "--nudge-target-unique-id", "one-guid",
        "--snap-target-unique-id", "one-guid"
    });
    expect(!nudge_snap_result.ok,
        "#1035: launch contract should reject simultaneous nudge-object and snap-object requests");

    const auto nudge_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--nudge-object",
        "--clear-property",
        "--property-name", "Caption",
        "--nudge-mode", "horizontal",
        "--delta-hpos", "1",
        "--nudge-target-unique-id", "one-guid"
    });
    expect(!nudge_property_result.ok,
        "#1035: launch contract should reject nudge-object combined with property commands");

    const auto stray_nudge_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--nudge-mode", "horizontal"
    });
    expect(!stray_nudge_result.ok,
        "#1035: launch contract should reject stray nudge arguments");
}

void test_parse_launch_arguments_for_dynamic_alignment_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--dynamic-alignment-object",
        "--dynamic-alignment", "IIF(.T., 2, 0)",
        "--dynamic-alignment-target-object-name", "txtNotes",
        "--dynamic-alignment-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1186: launch contract should parse dynamic-alignment-object requests");
    expect(result.request.dynamic_alignment_object,
        "#1186: launch contract should detect --dynamic-alignment-object");
    expect(result.request.dynamic_alignment_available &&
            result.request.dynamic_alignment == "IIF(.T., 2, 0)",
        "#1186: dynamic-alignment-object requests should carry raw expression text");
    expect(result.request.dynamic_alignment_objects.size() == 2U,
        "#1186: dynamic-alignment-object requests should collect dynamic-alignment target selectors");
    if (result.request.dynamic_alignment_objects.size() == 2U) {
        expect(result.request.dynamic_alignment_objects[0].object_name == "txtNotes" &&
                result.request.dynamic_alignment_objects[0].unique_id.empty(),
            "#1186: dynamic-alignment-object requests should parse target object-name selectors");
        expect(result.request.dynamic_alignment_objects[1].object_name.empty() &&
                result.request.dynamic_alignment_objects[1].unique_id == "two-guid",
            "#1186: dynamic-alignment-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_dynamic_alignment_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-alignment-object",
        "--dynamic-alignment-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1186: launch contract should reject dynamic-alignment-object requests without dynamic alignment");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-alignment-object",
        "--dynamic-alignment", "IIF(.T., 2, 0)"
    });
    expect(!missing_targets_result.ok,
        "#1186: launch contract should reject dynamic-alignment-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_dynamic_alignment_object_ambiguity() {
    const auto dynamic_alignment_allow_output_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-alignment-object",
        "--allow-output-object",
        "--dynamic-alignment", "IIF(.T., 2, 0)",
        "--dynamic-alignment-target-unique-id", "one-guid",
        "--allow-output", "false",
        "--allow-output-target-unique-id", "one-guid"
    });
    expect(!dynamic_alignment_allow_output_result.ok,
        "#1186: launch contract should reject simultaneous dynamic-alignment-object and allow-output-object requests");

    const auto dynamic_alignment_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-alignment-object",
        "--clear-property",
        "--property-name", "DynamicAlignment",
        "--dynamic-alignment", "IIF(.T., 2, 0)",
        "--dynamic-alignment-target-unique-id", "one-guid"
    });
    expect(!dynamic_alignment_property_result.ok,
        "#1186: launch contract should reject dynamic-alignment-object combined with property commands");

    const auto stray_dynamic_alignment_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dynamic-alignment", "IIF(.T., 2, 0)"
    });
    expect(!stray_dynamic_alignment_result.ok,
        "#1186: launch contract should reject stray dynamic-alignment arguments");
}

}  // namespace cf_test_studio_host
