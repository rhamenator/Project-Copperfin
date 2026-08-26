// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_studio_host_support.h"

namespace cf_test_studio_host {
#include "test_studio_host_behavior_window_help_chrome.inl"
#include "test_studio_host_behavior_window_chrome_options.inl"

#include "test_studio_host_behavior_window_size_position.inl"

void test_parse_launch_arguments_for_auto_center_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--auto-center-object",
        "--auto-center", "false",
        "--auto-center-target-object-name", "frmCustomer",
        "--auto-center-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1078: launch contract should parse auto-center-object requests");
    expect(result.request.auto_center_object,
        "#1078: launch contract should detect --auto-center-object");
    expect(result.request.auto_center_available && !result.request.auto_center,
        "#1078: auto-center-object requests should carry auto center state");
    expect(result.request.auto_center_objects.size() == 2U,
        "#1078: auto-center-object requests should collect auto-center target selectors");
    if (result.request.auto_center_objects.size() == 2U) {
        expect(result.request.auto_center_objects[0].object_name == "frmCustomer" &&
                result.request.auto_center_objects[0].unique_id.empty(),
            "#1078: auto-center-object requests should parse target object-name selectors");
        expect(result.request.auto_center_objects[1].object_name.empty() &&
                result.request.auto_center_objects[1].unique_id == "two-guid",
            "#1078: auto-center-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_auto_center_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--auto-center-object",
        "--auto-center-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1078: launch contract should reject auto-center-object requests without auto center state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--auto-center-object",
        "--auto-center", "false"
    });
    expect(!missing_targets_result.ok,
        "#1078: launch contract should reject auto-center-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--auto-center-object",
        "--auto-center", "sometimes",
        "--auto-center-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1078: launch contract should reject invalid auto-center boolean values");
}

void test_parse_launch_arguments_rejects_auto_center_object_ambiguity() {
    const auto auto_center_allow_output_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--auto-center-object",
        "--allow-output-object",
        "--auto-center", "false",
        "--auto-center-target-unique-id", "one-guid",
        "--allow-output", "false",
        "--allow-output-target-unique-id", "one-guid"
    });
    expect(!auto_center_allow_output_result.ok,
        "#1078: launch contract should reject simultaneous auto-center-object and allow-output-object requests");

    const auto auto_center_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--auto-center-object",
        "--clear-property",
        "--property-name", "AutoCenter",
        "--auto-center", "false",
        "--auto-center-target-unique-id", "one-guid"
    });
    expect(!auto_center_property_result.ok,
        "#1078: launch contract should reject auto-center-object combined with property commands");

    const auto stray_auto_center_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--auto-center", "false"
    });
    expect(!stray_auto_center_result.ok,
        "#1078: launch contract should reject stray auto-center arguments");
}

void test_parse_launch_arguments_for_dockable_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--dockable-object",
        "--dockable", "false",
        "--dockable-target-object-name", "frmCustomer",
        "--dockable-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1082: launch contract should parse dockable-object requests");
    expect(result.request.dockable_object,
        "#1082: launch contract should detect --dockable-object");
    expect(result.request.dockable_available && !result.request.dockable,
        "#1082: dockable-object requests should carry dockable state");
    expect(result.request.dockable_objects.size() == 2U,
        "#1082: dockable-object requests should collect dockable target selectors");
    if (result.request.dockable_objects.size() == 2U) {
        expect(result.request.dockable_objects[0].object_name == "frmCustomer" &&
                result.request.dockable_objects[0].unique_id.empty(),
            "#1082: dockable-object requests should parse target object-name selectors");
        expect(result.request.dockable_objects[1].object_name.empty() &&
                result.request.dockable_objects[1].unique_id == "two-guid",
            "#1082: dockable-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_dockable_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dockable-object",
        "--dockable-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1082: launch contract should reject dockable-object requests without dockable state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dockable-object",
        "--dockable", "false"
    });
    expect(!missing_targets_result.ok,
        "#1082: launch contract should reject dockable-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dockable-object",
        "--dockable", "sometimes",
        "--dockable-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1082: launch contract should reject invalid dockable boolean values");
}

void test_parse_launch_arguments_rejects_dockable_object_ambiguity() {
    const auto dockable_auto_size_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dockable-object",
        "--auto-size-object",
        "--dockable", "false",
        "--dockable-target-unique-id", "one-guid",
        "--auto-size", "false",
        "--auto-size-target-unique-id", "one-guid"
    });
    expect(!dockable_auto_size_result.ok,
        "#1082: launch contract should reject simultaneous dockable-object and auto-size-object requests");

    const auto dockable_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dockable-object",
        "--clear-property",
        "--property-name", "Dockable",
        "--dockable", "false",
        "--dockable-target-unique-id", "one-guid"
    });
    expect(!dockable_property_result.ok,
        "#1082: launch contract should reject dockable-object combined with property commands");

    const auto stray_dockable_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--dockable", "false"
    });
    expect(!stray_dockable_result.ok,
        "#1082: launch contract should reject stray dockable arguments");
}

void test_parse_launch_arguments_for_lock_screen_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--lock-screen-object",
        "--lock-screen", "false",
        "--lock-screen-target-object-name", "frmCustomer",
        "--lock-screen-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1085: launch contract should parse lock-screen-object requests");
    expect(result.request.lock_screen_object,
        "#1085: launch contract should detect --lock-screen-object");
    expect(result.request.lock_screen_available && !result.request.lock_screen,
        "#1085: lock-screen-object requests should carry lock screen state");
    expect(result.request.lock_screen_objects.size() == 2U,
        "#1085: lock-screen-object requests should collect lock_screen target selectors");
    if (result.request.lock_screen_objects.size() == 2U) {
        expect(result.request.lock_screen_objects[0].object_name == "frmCustomer" &&
                result.request.lock_screen_objects[0].unique_id.empty(),
            "#1085: lock-screen-object requests should parse target object-name selectors");
        expect(result.request.lock_screen_objects[1].object_name.empty() &&
                result.request.lock_screen_objects[1].unique_id == "two-guid",
            "#1085: lock-screen-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_lock_screen_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--lock-screen-object",
        "--lock-screen-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1085: launch contract should reject lock-screen-object requests without lock screen state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--lock-screen-object",
        "--lock-screen", "false"
    });
    expect(!missing_targets_result.ok,
        "#1085: launch contract should reject lock-screen-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--lock-screen-object",
        "--lock-screen", "sometimes",
        "--lock-screen-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1085: launch contract should reject invalid lock-screen boolean values");
}

void test_parse_launch_arguments_rejects_lock_screen_object_ambiguity() {
    const auto lock_screen_auto_size_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--lock-screen-object",
        "--auto-size-object",
        "--lock-screen", "false",
        "--lock-screen-target-unique-id", "one-guid",
        "--auto-size", "false",
        "--auto-size-target-unique-id", "one-guid"
    });
    expect(!lock_screen_auto_size_result.ok,
        "#1085: launch contract should reject simultaneous lock-screen-object and auto-size-object requests");

    const auto lock_screen_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--lock-screen-object",
        "--clear-property",
        "--property-name", "Dockable",
        "--lock-screen", "false",
        "--lock-screen-target-unique-id", "one-guid"
    });
    expect(!lock_screen_property_result.ok,
        "#1085: launch contract should reject lock-screen-object combined with property commands");

    const auto stray_lock_screen_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--lock-screen", "false"
    });
    expect(!stray_lock_screen_result.ok,
        "#1085: launch contract should reject stray lock-screen arguments");
}

void test_parse_launch_arguments_for_split_bar_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--split-bar-object",
        "--split-bar", "false",
        "--split-bar-target-object-name", "frmCustomer",
        "--split-bar-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1089: launch contract should parse split-bar-object requests");
    expect(result.request.split_bar_object,
        "#1089: launch contract should detect --split-bar-object");
    expect(result.request.split_bar_available && !result.request.split_bar,
        "#1089: split-bar-object requests should carry split bar state");
    expect(result.request.split_bar_objects.size() == 2U,
        "#1089: split-bar-object requests should collect split_bar target selectors");
    if (result.request.split_bar_objects.size() == 2U) {
        expect(result.request.split_bar_objects[0].object_name == "frmCustomer" &&
                result.request.split_bar_objects[0].unique_id.empty(),
            "#1089: split-bar-object requests should parse target object-name selectors");
        expect(result.request.split_bar_objects[1].object_name.empty() &&
                result.request.split_bar_objects[1].unique_id == "two-guid",
            "#1089: split-bar-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_split_bar_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--split-bar-object",
        "--split-bar-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1089: launch contract should reject split-bar-object requests without split bar state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--split-bar-object",
        "--split-bar", "false"
    });
    expect(!missing_targets_result.ok,
        "#1089: launch contract should reject split-bar-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--split-bar-object",
        "--split-bar", "sometimes",
        "--split-bar-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1089: launch contract should reject invalid split-bar boolean values");
}

void test_parse_launch_arguments_rejects_split_bar_object_ambiguity() {
    const auto split_bar_auto_size_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--split-bar-object",
        "--auto-size-object",
        "--split-bar", "false",
        "--split-bar-target-unique-id", "one-guid",
        "--auto-size", "false",
        "--auto-size-target-unique-id", "one-guid"
    });
    expect(!split_bar_auto_size_result.ok,
        "#1089: launch contract should reject simultaneous split-bar-object and auto-size-object requests");

    const auto split_bar_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--split-bar-object",
        "--clear-property",
        "--property-name", "Dockable",
        "--split-bar", "false",
        "--split-bar-target-unique-id", "one-guid"
    });
    expect(!split_bar_property_result.ok,
        "#1089: launch contract should reject split-bar-object combined with property commands");

    const auto stray_split_bar_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--split-bar", "false"
    });
    expect(!stray_split_bar_result.ok,
        "#1089: launch contract should reject stray split-bar arguments");
}

void test_parse_launch_arguments_for_panel_link_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--panel-link-object",
        "--panel-link", "false",
        "--panel-link-target-object-name", "frmCustomer",
        "--panel-link-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1091: launch contract should parse panel-link-object requests");
    expect(result.request.panel_link_object,
        "#1091: launch contract should detect --panel-link-object");
    expect(result.request.panel_link_available && !result.request.panel_link,
        "#1091: panel-link-object requests should carry panel link state");
    expect(result.request.panel_link_objects.size() == 2U,
        "#1091: panel-link-object requests should collect panel_link target selectors");
    if (result.request.panel_link_objects.size() == 2U) {
        expect(result.request.panel_link_objects[0].object_name == "frmCustomer" &&
                result.request.panel_link_objects[0].unique_id.empty(),
            "#1091: panel-link-object requests should parse target object-name selectors");
        expect(result.request.panel_link_objects[1].object_name.empty() &&
                result.request.panel_link_objects[1].unique_id == "two-guid",
            "#1091: panel-link-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_panel_link_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--panel-link-object",
        "--panel-link-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1091: launch contract should reject panel-link-object requests without panel link state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--panel-link-object",
        "--panel-link", "false"
    });
    expect(!missing_targets_result.ok,
        "#1091: launch contract should reject panel-link-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--panel-link-object",
        "--panel-link", "sometimes",
        "--panel-link-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1091: launch contract should reject invalid panel-link boolean values");
}

void test_parse_launch_arguments_rejects_panel_link_object_ambiguity() {
    const auto panel_link_auto_size_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--panel-link-object",
        "--auto-size-object",
        "--panel-link", "false",
        "--panel-link-target-unique-id", "one-guid",
        "--auto-size", "false",
        "--auto-size-target-unique-id", "one-guid"
    });
    expect(!panel_link_auto_size_result.ok,
        "#1091: launch contract should reject simultaneous panel-link-object and auto-size-object requests");

    const auto panel_link_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--panel-link-object",
        "--clear-property",
        "--property-name", "Dockable",
        "--panel-link", "false",
        "--panel-link-target-unique-id", "one-guid"
    });
    expect(!panel_link_property_result.ok,
        "#1091: launch contract should reject panel-link-object combined with property commands");

    const auto stray_panel_link_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--panel-link", "false"
    });
    expect(!stray_panel_link_result.ok,
        "#1091: launch contract should reject stray panel-link arguments");
}

void test_parse_launch_arguments_for_resizable_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--resizable-object",
        "--resizable", "false",
        "--resizable-target-object-name", "frmCustomer",
        "--resizable-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1094: launch contract should parse resizable-object requests");
    expect(result.request.resizable_object,
        "#1094: launch contract should detect --resizable-object");
    expect(result.request.resizable_available && !result.request.resizable,
        "#1094: resizable-object requests should carry resizable state");
    expect(result.request.resizable_objects.size() == 2U,
        "#1094: resizable-object requests should collect resizable target selectors");
    if (result.request.resizable_objects.size() == 2U) {
        expect(result.request.resizable_objects[0].object_name == "frmCustomer" &&
                result.request.resizable_objects[0].unique_id.empty(),
            "#1094: resizable-object requests should parse target object-name selectors");
        expect(result.request.resizable_objects[1].object_name.empty() &&
                result.request.resizable_objects[1].unique_id == "two-guid",
            "#1094: resizable-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_resizable_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--resizable-object",
        "--resizable-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1094: launch contract should reject resizable-object requests without resizable state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--resizable-object",
        "--resizable", "false"
    });
    expect(!missing_targets_result.ok,
        "#1094: launch contract should reject resizable-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--resizable-object",
        "--resizable", "sometimes",
        "--resizable-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1094: launch contract should reject invalid resizable boolean values");
}

void test_parse_launch_arguments_rejects_resizable_object_ambiguity() {
    const auto resizable_auto_size_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--resizable-object",
        "--auto-size-object",
        "--resizable", "false",
        "--resizable-target-unique-id", "one-guid",
        "--auto-size", "false",
        "--auto-size-target-unique-id", "one-guid"
    });
    expect(!resizable_auto_size_result.ok,
        "#1094: launch contract should reject simultaneous resizable-object and auto-size-object requests");

    const auto resizable_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--resizable-object",
        "--clear-property",
        "--property-name", "Dockable",
        "--resizable", "false",
        "--resizable-target-unique-id", "one-guid"
    });
    expect(!resizable_property_result.ok,
        "#1094: launch contract should reject resizable-object combined with property commands");

    const auto stray_resizable_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--resizable", "false"
    });
    expect(!stray_resizable_result.ok,
        "#1094: launch contract should reject stray resizable arguments");
}

void test_parse_launch_arguments_for_always_on_top_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--always-on-top-object",
        "--always-on-top", "false",
        "--always-on-top-target-object-name", "frmCustomer",
        "--always-on-top-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1096: launch contract should parse always-on-top-object requests");
    expect(result.request.always_on_top_object,
        "#1096: launch contract should detect --always-on-top-object");
    expect(result.request.always_on_top_available && !result.request.always_on_top,
        "#1096: always-on-top-object requests should carry always on top state");
    expect(result.request.always_on_top_objects.size() == 2U,
        "#1096: always-on-top-object requests should collect always_on_top target selectors");
    if (result.request.always_on_top_objects.size() == 2U) {
        expect(result.request.always_on_top_objects[0].object_name == "frmCustomer" &&
                result.request.always_on_top_objects[0].unique_id.empty(),
            "#1096: always-on-top-object requests should parse target object-name selectors");
        expect(result.request.always_on_top_objects[1].object_name.empty() &&
                result.request.always_on_top_objects[1].unique_id == "two-guid",
            "#1096: always-on-top-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_always_on_top_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--always-on-top-object",
        "--always-on-top-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1096: launch contract should reject always-on-top-object requests without always on top state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--always-on-top-object",
        "--always-on-top", "false"
    });
    expect(!missing_targets_result.ok,
        "#1096: launch contract should reject always-on-top-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--always-on-top-object",
        "--always-on-top", "sometimes",
        "--always-on-top-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1096: launch contract should reject invalid always-on-top boolean values");
}

void test_parse_launch_arguments_rejects_always_on_top_object_ambiguity() {
    const auto always_on_top_auto_size_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--always-on-top-object",
        "--auto-size-object",
        "--always-on-top", "false",
        "--always-on-top-target-unique-id", "one-guid",
        "--auto-size", "false",
        "--auto-size-target-unique-id", "one-guid"
    });
    expect(!always_on_top_auto_size_result.ok,
        "#1096: launch contract should reject simultaneous always-on-top-object and auto-size-object requests");

    const auto always_on_top_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--always-on-top-object",
        "--clear-property",
        "--property-name", "Dockable",
        "--always-on-top", "false",
        "--always-on-top-target-unique-id", "one-guid"
    });
    expect(!always_on_top_property_result.ok,
        "#1096: launch contract should reject always-on-top-object combined with property commands");

    const auto stray_always_on_top_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--always-on-top", "false"
    });
    expect(!stray_always_on_top_result.ok,
        "#1096: launch contract should reject stray always-on-top arguments");
}

void test_parse_launch_arguments_for_always_on_bottom_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--always-on-bottom-object",
        "--always-on-bottom", "false",
        "--always-on-bottom-target-object-name", "frmCustomer",
        "--always-on-bottom-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1097: launch contract should parse always-on-bottom-object requests");
    expect(result.request.always_on_bottom_object,
        "#1097: launch contract should detect --always-on-bottom-object");
    expect(result.request.always_on_bottom_available && !result.request.always_on_bottom,
        "#1097: always-on-bottom-object requests should carry always on bottom state");
    expect(result.request.always_on_bottom_objects.size() == 2U,
        "#1097: always-on-bottom-object requests should collect always_on_bottom target selectors");
    if (result.request.always_on_bottom_objects.size() == 2U) {
        expect(result.request.always_on_bottom_objects[0].object_name == "frmCustomer" &&
                result.request.always_on_bottom_objects[0].unique_id.empty(),
            "#1097: always-on-bottom-object requests should parse target object-name selectors");
        expect(result.request.always_on_bottom_objects[1].object_name.empty() &&
                result.request.always_on_bottom_objects[1].unique_id == "two-guid",
            "#1097: always-on-bottom-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_always_on_bottom_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--always-on-bottom-object",
        "--always-on-bottom-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1097: launch contract should reject always-on-bottom-object requests without always on bottom state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--always-on-bottom-object",
        "--always-on-bottom", "false"
    });
    expect(!missing_targets_result.ok,
        "#1097: launch contract should reject always-on-bottom-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--always-on-bottom-object",
        "--always-on-bottom", "sometimes",
        "--always-on-bottom-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1097: launch contract should reject invalid always-on-bottom boolean values");
}

void test_parse_launch_arguments_rejects_always_on_bottom_object_ambiguity() {
    const auto always_on_bottom_auto_size_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--always-on-bottom-object",
        "--auto-size-object",
        "--always-on-bottom", "false",
        "--always-on-bottom-target-unique-id", "one-guid",
        "--auto-size", "false",
        "--auto-size-target-unique-id", "one-guid"
    });
    expect(!always_on_bottom_auto_size_result.ok,
        "#1097: launch contract should reject simultaneous always-on-bottom-object and auto-size-object requests");

    const auto always_on_bottom_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--always-on-bottom-object",
        "--clear-property",
        "--property-name", "Dockable",
        "--always-on-bottom", "false",
        "--always-on-bottom-target-unique-id", "one-guid"
    });
    expect(!always_on_bottom_property_result.ok,
        "#1097: launch contract should reject always-on-bottom-object combined with property commands");

    const auto stray_always_on_bottom_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--always-on-bottom", "false"
    });
    expect(!stray_always_on_bottom_result.ok,
        "#1097: launch contract should reject stray always-on-bottom arguments");
}

}  // namespace cf_test_studio_host
