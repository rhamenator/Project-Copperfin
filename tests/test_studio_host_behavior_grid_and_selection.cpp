// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_studio_host_support.h"

namespace cf_test_studio_host {
void test_parse_launch_arguments_for_continuous_scroll_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--continuous-scroll-object",
        "--continuous-scroll", "false",
        "--continuous-scroll-target-object-name", "frmCustomer",
        "--continuous-scroll-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1081: launch contract should parse continuous-scroll-object requests");
    expect(result.request.continuous_scroll_object,
        "#1081: launch contract should detect --continuous-scroll-object");
    expect(result.request.continuous_scroll_available && !result.request.continuous_scroll,
        "#1081: continuous-scroll-object requests should carry continuous scroll state");
    expect(result.request.continuous_scroll_objects.size() == 2U,
        "#1081: continuous-scroll-object requests should collect continuous-scroll target selectors");
    if (result.request.continuous_scroll_objects.size() == 2U) {
        expect(result.request.continuous_scroll_objects[0].object_name == "frmCustomer" &&
                result.request.continuous_scroll_objects[0].unique_id.empty(),
            "#1081: continuous-scroll-object requests should parse target object-name selectors");
        expect(result.request.continuous_scroll_objects[1].object_name.empty() &&
                result.request.continuous_scroll_objects[1].unique_id == "two-guid",
            "#1081: continuous-scroll-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_continuous_scroll_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--continuous-scroll-object",
        "--continuous-scroll-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1081: launch contract should reject continuous-scroll-object requests without continuous scroll state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--continuous-scroll-object",
        "--continuous-scroll", "false"
    });
    expect(!missing_targets_result.ok,
        "#1081: launch contract should reject continuous-scroll-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--continuous-scroll-object",
        "--continuous-scroll", "sometimes",
        "--continuous-scroll-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1081: launch contract should reject invalid continuous-scroll boolean values");
}

void test_parse_launch_arguments_rejects_continuous_scroll_object_ambiguity() {
    const auto continuous_scroll_auto_size_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--continuous-scroll-object",
        "--auto-size-object",
        "--continuous-scroll", "false",
        "--continuous-scroll-target-unique-id", "one-guid",
        "--auto-size", "false",
        "--auto-size-target-unique-id", "one-guid"
    });
    expect(!continuous_scroll_auto_size_result.ok,
        "#1081: launch contract should reject simultaneous continuous-scroll-object and auto-size-object requests");

    const auto continuous_scroll_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--continuous-scroll-object",
        "--clear-property",
        "--property-name", "ContinuousScroll",
        "--continuous-scroll", "false",
        "--continuous-scroll-target-unique-id", "one-guid"
    });
    expect(!continuous_scroll_property_result.ok,
        "#1081: launch contract should reject continuous-scroll-object combined with property commands");

    const auto stray_continuous_scroll_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--continuous-scroll", "false"
    });
    expect(!stray_continuous_scroll_result.ok,
        "#1081: launch contract should reject stray continuous-scroll arguments");
}

void test_parse_launch_arguments_for_sparse_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--sparse-object",
        "--sparse", "false",
        "--sparse-target-object-name", "frmCustomer",
        "--sparse-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1084: launch contract should parse sparse-object requests");
    expect(result.request.sparse_object,
        "#1084: launch contract should detect --sparse-object");
    expect(result.request.sparse_available && !result.request.sparse,
        "#1084: sparse-object requests should carry sparse state");
    expect(result.request.sparse_objects.size() == 2U,
        "#1084: sparse-object requests should collect sparse target selectors");
    if (result.request.sparse_objects.size() == 2U) {
        expect(result.request.sparse_objects[0].object_name == "frmCustomer" &&
                result.request.sparse_objects[0].unique_id.empty(),
            "#1084: sparse-object requests should parse target object-name selectors");
        expect(result.request.sparse_objects[1].object_name.empty() &&
                result.request.sparse_objects[1].unique_id == "two-guid",
            "#1084: sparse-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_sparse_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--sparse-object",
        "--sparse-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1084: launch contract should reject sparse-object requests without sparse state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--sparse-object",
        "--sparse", "false"
    });
    expect(!missing_targets_result.ok,
        "#1084: launch contract should reject sparse-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--sparse-object",
        "--sparse", "sometimes",
        "--sparse-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1084: launch contract should reject invalid sparse boolean values");
}

void test_parse_launch_arguments_rejects_sparse_object_ambiguity() {
    const auto sparse_auto_size_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--sparse-object",
        "--auto-size-object",
        "--sparse", "false",
        "--sparse-target-unique-id", "one-guid",
        "--auto-size", "false",
        "--auto-size-target-unique-id", "one-guid"
    });
    expect(!sparse_auto_size_result.ok,
        "#1084: launch contract should reject simultaneous sparse-object and auto-size-object requests");

    const auto sparse_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--sparse-object",
        "--clear-property",
        "--property-name", "Dockable",
        "--sparse", "false",
        "--sparse-target-unique-id", "one-guid"
    });
    expect(!sparse_property_result.ok,
        "#1084: launch contract should reject sparse-object combined with property commands");

    const auto stray_sparse_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--sparse", "false"
    });
    expect(!stray_sparse_result.ok,
        "#1084: launch contract should reject stray sparse arguments");
}

void test_parse_launch_arguments_for_allow_cell_selection_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--allow-cell-selection-object",
        "--allow-cell-selection", "false",
        "--allow-cell-selection-target-object-name", "frmCustomer",
        "--allow-cell-selection-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1086: launch contract should parse allow-cell-selection-object requests");
    expect(result.request.allow_cell_selection_object,
        "#1086: launch contract should detect --allow-cell-selection-object");
    expect(result.request.allow_cell_selection_available && !result.request.allow_cell_selection,
        "#1086: allow-cell-selection-object requests should carry allow cell selection state");
    expect(result.request.allow_cell_selection_objects.size() == 2U,
        "#1086: allow-cell-selection-object requests should collect allow_cell_selection target selectors");
    if (result.request.allow_cell_selection_objects.size() == 2U) {
        expect(result.request.allow_cell_selection_objects[0].object_name == "frmCustomer" &&
                result.request.allow_cell_selection_objects[0].unique_id.empty(),
            "#1086: allow-cell-selection-object requests should parse target object-name selectors");
        expect(result.request.allow_cell_selection_objects[1].object_name.empty() &&
                result.request.allow_cell_selection_objects[1].unique_id == "two-guid",
            "#1086: allow-cell-selection-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_allow_cell_selection_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--allow-cell-selection-object",
        "--allow-cell-selection-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1086: launch contract should reject allow-cell-selection-object requests without allow cell selection state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--allow-cell-selection-object",
        "--allow-cell-selection", "false"
    });
    expect(!missing_targets_result.ok,
        "#1086: launch contract should reject allow-cell-selection-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--allow-cell-selection-object",
        "--allow-cell-selection", "sometimes",
        "--allow-cell-selection-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1086: launch contract should reject invalid allow-cell-selection boolean values");
}

void test_parse_launch_arguments_rejects_allow_cell_selection_object_ambiguity() {
    const auto allow_cell_selection_auto_size_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--allow-cell-selection-object",
        "--auto-size-object",
        "--allow-cell-selection", "false",
        "--allow-cell-selection-target-unique-id", "one-guid",
        "--auto-size", "false",
        "--auto-size-target-unique-id", "one-guid"
    });
    expect(!allow_cell_selection_auto_size_result.ok,
        "#1086: launch contract should reject simultaneous allow-cell-selection-object and auto-size-object requests");

    const auto allow_cell_selection_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--allow-cell-selection-object",
        "--clear-property",
        "--property-name", "Dockable",
        "--allow-cell-selection", "false",
        "--allow-cell-selection-target-unique-id", "one-guid"
    });
    expect(!allow_cell_selection_property_result.ok,
        "#1086: launch contract should reject allow-cell-selection-object combined with property commands");

    const auto stray_allow_cell_selection_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--allow-cell-selection", "false"
    });
    expect(!stray_allow_cell_selection_result.ok,
        "#1086: launch contract should reject stray allow-cell-selection arguments");
}

void test_parse_launch_arguments_for_hide_selection_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--hide-selection-object",
        "--hide-selection", "false",
        "--hide-selection-target-object-name", "frmCustomer",
        "--hide-selection-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1109: launch contract should parse hide-selection-object requests");
    expect(result.request.hide_selection_object,
        "#1109: launch contract should detect --hide-selection-object");
    expect(result.request.hide_selection_available && !result.request.hide_selection,
        "#1109: hide-selection-object requests should carry hide selection state");
    expect(result.request.hide_selection_objects.size() == 2U,
        "#1109: hide-selection-object requests should collect hide-selection target selectors");
    if (result.request.hide_selection_objects.size() == 2U) {
        expect(result.request.hide_selection_objects[0].object_name == "frmCustomer" &&
                result.request.hide_selection_objects[0].unique_id.empty(),
            "#1109: hide-selection-object requests should parse target object-name selectors");
        expect(result.request.hide_selection_objects[1].object_name.empty() &&
                result.request.hide_selection_objects[1].unique_id == "two-guid",
            "#1109: hide-selection-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_hide_selection_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--hide-selection-object",
        "--hide-selection-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1109: launch contract should reject hide-selection-object requests without hide selection state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--hide-selection-object",
        "--hide-selection", "false"
    });
    expect(!missing_targets_result.ok,
        "#1109: launch contract should reject hide-selection-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--hide-selection-object",
        "--hide-selection", "sometimes",
        "--hide-selection-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1109: launch contract should reject invalid hide-selection boolean values");
}

void test_parse_launch_arguments_rejects_hide_selection_object_ambiguity() {
    const auto hide_selection_auto_size_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--hide-selection-object",
        "--auto-size-object",
        "--hide-selection", "false",
        "--hide-selection-target-unique-id", "one-guid",
        "--auto-size", "false",
        "--auto-size-target-unique-id", "one-guid"
    });
    expect(!hide_selection_auto_size_result.ok,
        "#1109: launch contract should reject simultaneous hide-selection-object and auto-size-object requests");

    const auto hide_selection_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--hide-selection-object",
        "--clear-property",
        "--property-name", "HideSelection",
        "--hide-selection", "false",
        "--hide-selection-target-unique-id", "one-guid"
    });
    expect(!hide_selection_property_result.ok,
        "#1109: launch contract should reject hide-selection-object combined with property commands");

    const auto stray_hide_selection_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--hide-selection", "false"
    });
    expect(!stray_hide_selection_result.ok,
        "#1109: launch contract should reject stray hide-selection arguments");
}

void test_parse_launch_arguments_for_record_mark_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--record-mark-object",
        "--record-mark", "false",
        "--record-mark-target-object-name", "frmCustomer",
        "--record-mark-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1088: launch contract should parse record-mark-object requests");
    expect(result.request.record_mark_object,
        "#1088: launch contract should detect --record-mark-object");
    expect(result.request.record_mark_available && !result.request.record_mark,
        "#1088: record-mark-object requests should carry record mark state");
    expect(result.request.record_mark_objects.size() == 2U,
        "#1088: record-mark-object requests should collect record_mark target selectors");
    if (result.request.record_mark_objects.size() == 2U) {
        expect(result.request.record_mark_objects[0].object_name == "frmCustomer" &&
                result.request.record_mark_objects[0].unique_id.empty(),
            "#1088: record-mark-object requests should parse target object-name selectors");
        expect(result.request.record_mark_objects[1].object_name.empty() &&
                result.request.record_mark_objects[1].unique_id == "two-guid",
            "#1088: record-mark-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_record_mark_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--record-mark-object",
        "--record-mark-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1088: launch contract should reject record-mark-object requests without record mark state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--record-mark-object",
        "--record-mark", "false"
    });
    expect(!missing_targets_result.ok,
        "#1088: launch contract should reject record-mark-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--record-mark-object",
        "--record-mark", "sometimes",
        "--record-mark-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1088: launch contract should reject invalid record-mark boolean values");
}

void test_parse_launch_arguments_rejects_record_mark_object_ambiguity() {
    const auto record_mark_auto_size_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--record-mark-object",
        "--auto-size-object",
        "--record-mark", "false",
        "--record-mark-target-unique-id", "one-guid",
        "--auto-size", "false",
        "--auto-size-target-unique-id", "one-guid"
    });
    expect(!record_mark_auto_size_result.ok,
        "#1088: launch contract should reject simultaneous record-mark-object and auto-size-object requests");

    const auto record_mark_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--record-mark-object",
        "--clear-property",
        "--property-name", "Dockable",
        "--record-mark", "false",
        "--record-mark-target-unique-id", "one-guid"
    });
    expect(!record_mark_property_result.ok,
        "#1088: launch contract should reject record-mark-object combined with property commands");

    const auto stray_record_mark_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--record-mark", "false"
    });
    expect(!stray_record_mark_result.ok,
        "#1088: launch contract should reject stray record-mark arguments");
}

void test_parse_launch_arguments_for_highlight_row_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--highlight-row-object",
        "--highlight-row", "false",
        "--highlight-row-target-object-name", "frmCustomer",
        "--highlight-row-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1090: launch contract should parse highlight-row-object requests");
    expect(result.request.highlight_row_object,
        "#1090: launch contract should detect --highlight-row-object");
    expect(result.request.highlight_row_available && !result.request.highlight_row,
        "#1090: highlight-row-object requests should carry highlight row state");
    expect(result.request.highlight_row_objects.size() == 2U,
        "#1090: highlight-row-object requests should collect highlight_row target selectors");
    if (result.request.highlight_row_objects.size() == 2U) {
        expect(result.request.highlight_row_objects[0].object_name == "frmCustomer" &&
                result.request.highlight_row_objects[0].unique_id.empty(),
            "#1090: highlight-row-object requests should parse target object-name selectors");
        expect(result.request.highlight_row_objects[1].object_name.empty() &&
                result.request.highlight_row_objects[1].unique_id == "two-guid",
            "#1090: highlight-row-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_highlight_row_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--highlight-row-object",
        "--highlight-row-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1090: launch contract should reject highlight-row-object requests without highlight row state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--highlight-row-object",
        "--highlight-row", "false"
    });
    expect(!missing_targets_result.ok,
        "#1090: launch contract should reject highlight-row-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--highlight-row-object",
        "--highlight-row", "sometimes",
        "--highlight-row-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1090: launch contract should reject invalid highlight-row boolean values");
}

void test_parse_launch_arguments_rejects_highlight_row_object_ambiguity() {
    const auto highlight_row_auto_size_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--highlight-row-object",
        "--auto-size-object",
        "--highlight-row", "false",
        "--highlight-row-target-unique-id", "one-guid",
        "--auto-size", "false",
        "--auto-size-target-unique-id", "one-guid"
    });
    expect(!highlight_row_auto_size_result.ok,
        "#1090: launch contract should reject simultaneous highlight-row-object and auto-size-object requests");

    const auto highlight_row_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--highlight-row-object",
        "--clear-property",
        "--property-name", "Dockable",
        "--highlight-row", "false",
        "--highlight-row-target-unique-id", "one-guid"
    });
    expect(!highlight_row_property_result.ok,
        "#1090: launch contract should reject highlight-row-object combined with property commands");

    const auto stray_highlight_row_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--highlight-row", "false"
    });
    expect(!stray_highlight_row_result.ok,
        "#1090: launch contract should reject stray highlight-row arguments");
}

void test_parse_launch_arguments_for_allow_header_sizing_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--allow-header-sizing-object",
        "--allow-header-sizing", "false",
        "--allow-header-sizing-target-object-name", "frmCustomer",
        "--allow-header-sizing-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1092: launch contract should parse allow-header-sizing-object requests");
    expect(result.request.allow_header_sizing_object,
        "#1092: launch contract should detect --allow-header-sizing-object");
    expect(result.request.allow_header_sizing_available && !result.request.allow_header_sizing,
        "#1092: allow-header-sizing-object requests should carry allow header sizing state");
    expect(result.request.allow_header_sizing_objects.size() == 2U,
        "#1092: allow-header-sizing-object requests should collect allow_header_sizing target selectors");
    if (result.request.allow_header_sizing_objects.size() == 2U) {
        expect(result.request.allow_header_sizing_objects[0].object_name == "frmCustomer" &&
                result.request.allow_header_sizing_objects[0].unique_id.empty(),
            "#1092: allow-header-sizing-object requests should parse target object-name selectors");
        expect(result.request.allow_header_sizing_objects[1].object_name.empty() &&
                result.request.allow_header_sizing_objects[1].unique_id == "two-guid",
            "#1092: allow-header-sizing-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_allow_header_sizing_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--allow-header-sizing-object",
        "--allow-header-sizing-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1092: launch contract should reject allow-header-sizing-object requests without allow header sizing state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--allow-header-sizing-object",
        "--allow-header-sizing", "false"
    });
    expect(!missing_targets_result.ok,
        "#1092: launch contract should reject allow-header-sizing-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--allow-header-sizing-object",
        "--allow-header-sizing", "sometimes",
        "--allow-header-sizing-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1092: launch contract should reject invalid allow-header-sizing boolean values");
}

void test_parse_launch_arguments_rejects_allow_header_sizing_object_ambiguity() {
    const auto allow_header_sizing_auto_size_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--allow-header-sizing-object",
        "--auto-size-object",
        "--allow-header-sizing", "false",
        "--allow-header-sizing-target-unique-id", "one-guid",
        "--auto-size", "false",
        "--auto-size-target-unique-id", "one-guid"
    });
    expect(!allow_header_sizing_auto_size_result.ok,
        "#1092: launch contract should reject simultaneous allow-header-sizing-object and auto-size-object requests");

    const auto allow_header_sizing_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--allow-header-sizing-object",
        "--clear-property",
        "--property-name", "Dockable",
        "--allow-header-sizing", "false",
        "--allow-header-sizing-target-unique-id", "one-guid"
    });
    expect(!allow_header_sizing_property_result.ok,
        "#1092: launch contract should reject allow-header-sizing-object combined with property commands");

    const auto stray_allow_header_sizing_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--allow-header-sizing", "false"
    });
    expect(!stray_allow_header_sizing_result.ok,
        "#1092: launch contract should reject stray allow-header-sizing arguments");
}

void test_parse_launch_arguments_for_allow_row_sizing_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--allow-row-sizing-object",
        "--allow-row-sizing", "false",
        "--allow-row-sizing-target-object-name", "frmCustomer",
        "--allow-row-sizing-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1093: launch contract should parse allow-row-sizing-object requests");
    expect(result.request.allow_row_sizing_object,
        "#1093: launch contract should detect --allow-row-sizing-object");
    expect(result.request.allow_row_sizing_available && !result.request.allow_row_sizing,
        "#1093: allow-row-sizing-object requests should carry allow row sizing state");
    expect(result.request.allow_row_sizing_objects.size() == 2U,
        "#1093: allow-row-sizing-object requests should collect allow_row_sizing target selectors");
    if (result.request.allow_row_sizing_objects.size() == 2U) {
        expect(result.request.allow_row_sizing_objects[0].object_name == "frmCustomer" &&
                result.request.allow_row_sizing_objects[0].unique_id.empty(),
            "#1093: allow-row-sizing-object requests should parse target object-name selectors");
        expect(result.request.allow_row_sizing_objects[1].object_name.empty() &&
                result.request.allow_row_sizing_objects[1].unique_id == "two-guid",
            "#1093: allow-row-sizing-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_allow_row_sizing_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--allow-row-sizing-object",
        "--allow-row-sizing-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1093: launch contract should reject allow-row-sizing-object requests without allow row sizing state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--allow-row-sizing-object",
        "--allow-row-sizing", "false"
    });
    expect(!missing_targets_result.ok,
        "#1093: launch contract should reject allow-row-sizing-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--allow-row-sizing-object",
        "--allow-row-sizing", "sometimes",
        "--allow-row-sizing-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1093: launch contract should reject invalid allow-row-sizing boolean values");
}

void test_parse_launch_arguments_rejects_allow_row_sizing_object_ambiguity() {
    const auto allow_row_sizing_auto_size_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--allow-row-sizing-object",
        "--auto-size-object",
        "--allow-row-sizing", "false",
        "--allow-row-sizing-target-unique-id", "one-guid",
        "--auto-size", "false",
        "--auto-size-target-unique-id", "one-guid"
    });
    expect(!allow_row_sizing_auto_size_result.ok,
        "#1093: launch contract should reject simultaneous allow-row-sizing-object and auto-size-object requests");

    const auto allow_row_sizing_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--allow-row-sizing-object",
        "--clear-property",
        "--property-name", "Dockable",
        "--allow-row-sizing", "false",
        "--allow-row-sizing-target-unique-id", "one-guid"
    });
    expect(!allow_row_sizing_property_result.ok,
        "#1093: launch contract should reject allow-row-sizing-object combined with property commands");

    const auto stray_allow_row_sizing_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--allow-row-sizing", "false"
    });
    expect(!stray_allow_row_sizing_result.ok,
        "#1093: launch contract should reject stray allow-row-sizing arguments");
}

}  // namespace cf_test_studio_host
