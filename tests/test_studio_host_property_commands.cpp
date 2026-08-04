// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_studio_host_support.h"

namespace cf_test_studio_host {
void test_parse_launch_arguments_for_clear_property() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--clear-property",
        "--object-name", "txtName",
        "--unique-id", "textbox-guid",
        "--property-name", "Caption"
    });

    expect(result.ok, "#1021: launch contract should parse clear-property requests");
    expect(result.request.clear_property, "#1021: launch contract should detect --clear-property");
    expect(!result.request.apply_property_update,
        "#1021: launch contract should not treat clear-property as set-property");
    expect(result.request.object_name == "txtName",
        "#1021: clear-property requests should carry object-name selectors");
    expect(result.request.unique_id == "textbox-guid",
        "#1021: clear-property requests should carry unique-id selectors");
    expect(result.request.property_name == "Caption",
        "#1021: clear-property requests should carry property names");
}

void test_parse_launch_arguments_rejects_ambiguous_property_command() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--set-property",
        "--clear-property",
        "--property-name", "Caption",
        "--property-value", "New"
    });

    expect(!result.ok,
        "#1021: launch contract should reject simultaneous set-property and clear-property requests");
}

void test_parse_launch_arguments_for_rename_property() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--rename-property",
        "--object-name", "txtName",
        "--unique-id", "textbox-guid",
        "--property-name", "ControlSource",
        "--new-property-name", "InputSource"
    });

    expect(result.ok, "#1022: launch contract should parse rename-property requests");
    expect(result.request.rename_property, "#1022: launch contract should detect --rename-property");
    expect(!result.request.apply_property_update,
        "#1022: launch contract should not treat rename-property as set-property");
    expect(!result.request.clear_property,
        "#1022: launch contract should not treat rename-property as clear-property");
    expect(result.request.object_name == "txtName",
        "#1022: rename-property requests should carry object-name selectors");
    expect(result.request.unique_id == "textbox-guid",
        "#1022: rename-property requests should carry unique-id selectors");
    expect(result.request.property_name == "ControlSource",
        "#1022: rename-property requests should carry source property names");
    expect(result.request.new_property_name == "InputSource",
        "#1022: rename-property requests should carry target property names");
}

void test_parse_launch_arguments_rejects_rename_property_missing_target() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--rename-property",
        "--property-name", "ControlSource"
    });

    expect(!result.ok,
        "#1022: launch contract should reject rename-property requests without target property names");
}

void test_parse_launch_arguments_rejects_any_ambiguous_property_commands() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--set-property",
        "--rename-property",
        "--property-name", "Caption",
        "--property-value", "New",
        "--new-property-name", "Title"
    });

    expect(!result.ok,
        "#1022: launch contract should reject simultaneous set-property and rename-property requests");
}

}  // namespace cf_test_studio_host
