void test_parse_launch_arguments_for_control_box_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--control-box-object",
        "--control-box", "false",
        "--control-box-target-object-name", "frmCustomer",
        "--control-box-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1074: launch contract should parse control-box-object requests");
    expect(result.request.control_box_object,
        "#1074: launch contract should detect --control-box-object");
    expect(result.request.control_box_available && !result.request.control_box,
        "#1074: control-box-object requests should carry control box state");
    expect(result.request.control_box_objects.size() == 2U,
        "#1074: control-box-object requests should collect control-box target selectors");
    if (result.request.control_box_objects.size() == 2U) {
        expect(result.request.control_box_objects[0].object_name == "frmCustomer" &&
                result.request.control_box_objects[0].unique_id.empty(),
            "#1074: control-box-object requests should parse target object-name selectors");
        expect(result.request.control_box_objects[1].object_name.empty() &&
                result.request.control_box_objects[1].unique_id == "two-guid",
            "#1074: control-box-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_control_box_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--control-box-object",
        "--control-box-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1074: launch contract should reject control-box-object requests without control box state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--control-box-object",
        "--control-box", "false"
    });
    expect(!missing_targets_result.ok,
        "#1074: launch contract should reject control-box-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--control-box-object",
        "--control-box", "sometimes",
        "--control-box-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1074: launch contract should reject invalid control-box boolean values");
}

void test_parse_launch_arguments_rejects_control_box_object_ambiguity() {
    const auto control_box_closable_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--control-box-object",
        "--closable-object",
        "--control-box", "false",
        "--control-box-target-unique-id", "one-guid",
        "--closable", "false",
        "--closable-target-unique-id", "one-guid"
    });
    expect(!control_box_closable_result.ok,
        "#1074: launch contract should reject simultaneous control-box-object and closable-object requests");

    const auto control_box_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--control-box-object",
        "--clear-property",
        "--property-name", "ControlBox",
        "--control-box", "false",
        "--control-box-target-unique-id", "one-guid"
    });
    expect(!control_box_property_result.ok,
        "#1074: launch contract should reject control-box-object combined with property commands");

    const auto stray_control_box_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--control-box", "false"
    });
    expect(!stray_control_box_result.ok,
        "#1074: launch contract should reject stray control-box arguments");
}

void test_parse_launch_arguments_for_desktop_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--desktop-object",
        "--desktop", "false",
        "--desktop-target-object-name", "frmCustomer",
        "--desktop-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1147: launch contract should parse desktop-object requests");
    expect(result.request.desktop_object,
        "#1147: launch contract should detect --desktop-object");
    expect(result.request.desktop_available && !result.request.desktop,
        "#1147: desktop-object requests should carry desktop state");
    expect(result.request.desktop_objects.size() == 2U,
        "#1147: desktop-object requests should collect desktop target selectors");
    if (result.request.desktop_objects.size() == 2U) {
        expect(result.request.desktop_objects[0].object_name == "frmCustomer" &&
                result.request.desktop_objects[0].unique_id.empty(),
            "#1147: desktop-object requests should parse target object-name selectors");
        expect(result.request.desktop_objects[1].object_name.empty() &&
                result.request.desktop_objects[1].unique_id == "two-guid",
            "#1147: desktop-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_desktop_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--desktop-object",
        "--desktop-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1147: launch contract should reject desktop-object requests without desktop state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--desktop-object",
        "--desktop", "false"
    });
    expect(!missing_targets_result.ok,
        "#1147: launch contract should reject desktop-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--desktop-object",
        "--desktop", "sometimes",
        "--desktop-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1147: launch contract should reject invalid desktop boolean values");
}

void test_parse_launch_arguments_rejects_desktop_object_ambiguity() {
    const auto desktop_allow_output_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--desktop-object",
        "--allow-output-object",
        "--desktop", "false",
        "--desktop-target-unique-id", "one-guid",
        "--allow-output", "false",
        "--allow-output-target-unique-id", "one-guid"
    });
    expect(!desktop_allow_output_result.ok,
        "#1147: launch contract should reject simultaneous desktop-object and allow-output-object requests");

    const auto desktop_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--desktop-object",
        "--clear-property",
        "--property-name", "Desktop",
        "--desktop", "false",
        "--desktop-target-unique-id", "one-guid"
    });
    expect(!desktop_property_result.ok,
        "#1147: launch contract should reject desktop-object combined with property commands");

    const auto stray_desktop_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--desktop", "false"
    });
    expect(!stray_desktop_result.ok,
        "#1147: launch contract should reject stray desktop arguments");
}

void test_parse_launch_arguments_for_key_preview_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--key-preview-object",
        "--key-preview", "false",
        "--key-preview-target-object-name", "frmCustomer",
        "--key-preview-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1148: launch contract should parse key-preview-object requests");
    expect(result.request.key_preview_object,
        "#1148: launch contract should detect --key-preview-object");
    expect(result.request.key_preview_available && !result.request.key_preview,
        "#1148: key-preview-object requests should carry key preview state");
    expect(result.request.key_preview_objects.size() == 2U,
        "#1148: key-preview-object requests should collect key-preview target selectors");
    if (result.request.key_preview_objects.size() == 2U) {
        expect(result.request.key_preview_objects[0].object_name == "frmCustomer" &&
                result.request.key_preview_objects[0].unique_id.empty(),
            "#1148: key-preview-object requests should parse target object-name selectors");
        expect(result.request.key_preview_objects[1].object_name.empty() &&
                result.request.key_preview_objects[1].unique_id == "two-guid",
            "#1148: key-preview-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_key_preview_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--key-preview-object",
        "--key-preview-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1148: launch contract should reject key-preview-object requests without key preview state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--key-preview-object",
        "--key-preview", "false"
    });
    expect(!missing_targets_result.ok,
        "#1148: launch contract should reject key-preview-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--key-preview-object",
        "--key-preview", "sometimes",
        "--key-preview-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1148: launch contract should reject invalid key-preview boolean values");
}

void test_parse_launch_arguments_rejects_key_preview_object_ambiguity() {
    const auto key_preview_allow_output_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--key-preview-object",
        "--allow-output-object",
        "--key-preview", "false",
        "--key-preview-target-unique-id", "one-guid",
        "--allow-output", "false",
        "--allow-output-target-unique-id", "one-guid"
    });
    expect(!key_preview_allow_output_result.ok,
        "#1148: launch contract should reject simultaneous key-preview-object and allow-output-object requests");

    const auto key_preview_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--key-preview-object",
        "--clear-property",
        "--property-name", "KeyPreview",
        "--key-preview", "false",
        "--key-preview-target-unique-id", "one-guid"
    });
    expect(!key_preview_property_result.ok,
        "#1148: launch contract should reject key-preview-object combined with property commands");

    const auto stray_key_preview_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--key-preview", "false"
    });
    expect(!stray_key_preview_result.ok,
        "#1148: launch contract should reject stray key-preview arguments");
}

void test_parse_launch_arguments_for_mac_desktop_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--mac-desktop-object",
        "--mac-desktop", "false",
        "--mac-desktop-target-object-name", "frmCustomer",
        "--mac-desktop-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1149: launch contract should parse mac-desktop-object requests");
    expect(result.request.mac_desktop_object,
        "#1149: launch contract should detect --mac-desktop-object");
    expect(result.request.mac_desktop_available && !result.request.mac_desktop,
        "#1149: mac-desktop-object requests should carry mac desktop state");
    expect(result.request.mac_desktop_objects.size() == 2U,
        "#1149: mac-desktop-object requests should collect mac-desktop target selectors");
    if (result.request.mac_desktop_objects.size() == 2U) {
        expect(result.request.mac_desktop_objects[0].object_name == "frmCustomer" &&
                result.request.mac_desktop_objects[0].unique_id.empty(),
            "#1149: mac-desktop-object requests should parse target object-name selectors");
        expect(result.request.mac_desktop_objects[1].object_name.empty() &&
                result.request.mac_desktop_objects[1].unique_id == "two-guid",
            "#1149: mac-desktop-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_mac_desktop_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--mac-desktop-object",
        "--mac-desktop-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1149: launch contract should reject mac-desktop-object requests without mac desktop state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--mac-desktop-object",
        "--mac-desktop", "false"
    });
    expect(!missing_targets_result.ok,
        "#1149: launch contract should reject mac-desktop-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--mac-desktop-object",
        "--mac-desktop", "sometimes",
        "--mac-desktop-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1149: launch contract should reject invalid mac-desktop boolean values");
}

void test_parse_launch_arguments_rejects_mac_desktop_object_ambiguity() {
    const auto mac_desktop_allow_output_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--mac-desktop-object",
        "--allow-output-object",
        "--mac-desktop", "false",
        "--mac-desktop-target-unique-id", "one-guid",
        "--allow-output", "false",
        "--allow-output-target-unique-id", "one-guid"
    });
    expect(!mac_desktop_allow_output_result.ok,
        "#1149: launch contract should reject simultaneous mac-desktop-object and allow-output-object requests");

    const auto mac_desktop_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--mac-desktop-object",
        "--clear-property",
        "--property-name", "MacDesktop",
        "--mac-desktop", "false",
        "--mac-desktop-target-unique-id", "one-guid"
    });
    expect(!mac_desktop_property_result.ok,
        "#1149: launch contract should reject mac-desktop-object combined with property commands");

    const auto stray_mac_desktop_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--mac-desktop", "false"
    });
    expect(!stray_mac_desktop_result.ok,
        "#1149: launch contract should reject stray mac-desktop arguments");
}

void test_parse_launch_arguments_for_max_button_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--max-button-object",
        "--max-button", "false",
        "--max-button-target-object-name", "frmCustomer",
        "--max-button-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1150: launch contract should parse max-button-object requests");
    expect(result.request.max_button_object,
        "#1150: launch contract should detect --max-button-object");
    expect(result.request.max_button_available && !result.request.max_button,
        "#1150: max-button-object requests should carry max button state");
    expect(result.request.max_button_objects.size() == 2U,
        "#1150: max-button-object requests should collect max-button target selectors");
    if (result.request.max_button_objects.size() == 2U) {
        expect(result.request.max_button_objects[0].object_name == "frmCustomer" &&
                result.request.max_button_objects[0].unique_id.empty(),
            "#1150: max-button-object requests should parse target object-name selectors");
        expect(result.request.max_button_objects[1].object_name.empty() &&
                result.request.max_button_objects[1].unique_id == "two-guid",
            "#1150: max-button-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_max_button_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--max-button-object",
        "--max-button-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1150: launch contract should reject max-button-object requests without max button state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--max-button-object",
        "--max-button", "false"
    });
    expect(!missing_targets_result.ok,
        "#1150: launch contract should reject max-button-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--max-button-object",
        "--max-button", "sometimes",
        "--max-button-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1150: launch contract should reject invalid max-button boolean values");
}

void test_parse_launch_arguments_rejects_max_button_object_ambiguity() {
    const auto max_button_allow_output_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--max-button-object",
        "--allow-output-object",
        "--max-button", "false",
        "--max-button-target-unique-id", "one-guid",
        "--allow-output", "false",
        "--allow-output-target-unique-id", "one-guid"
    });
    expect(!max_button_allow_output_result.ok,
        "#1150: launch contract should reject simultaneous max-button-object and allow-output-object requests");

    const auto max_button_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--max-button-object",
        "--clear-property",
        "--property-name", "MaxButton",
        "--max-button", "false",
        "--max-button-target-unique-id", "one-guid"
    });
    expect(!max_button_property_result.ok,
        "#1150: launch contract should reject max-button-object combined with property commands");

    const auto stray_max_button_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--max-button", "false"
    });
    expect(!stray_max_button_result.ok,
        "#1150: launch contract should reject stray max-button arguments");
}

void test_parse_launch_arguments_for_min_button_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--min-button-object",
        "--min-button", "false",
        "--min-button-target-object-name", "frmCustomer",
        "--min-button-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1155: launch contract should parse min-button-object requests");
    expect(result.request.min_button_object,
        "#1155: launch contract should detect --min-button-object");
    expect(result.request.min_button_available && !result.request.min_button,
        "#1155: min-button-object requests should carry min button state");
    expect(result.request.min_button_objects.size() == 2U,
        "#1155: min-button-object requests should collect min-button target selectors");
    if (result.request.min_button_objects.size() == 2U) {
        expect(result.request.min_button_objects[0].object_name == "frmCustomer" &&
                result.request.min_button_objects[0].unique_id.empty(),
            "#1155: min-button-object requests should parse target object-name selectors");
        expect(result.request.min_button_objects[1].object_name.empty() &&
                result.request.min_button_objects[1].unique_id == "two-guid",
            "#1155: min-button-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_min_button_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--min-button-object",
        "--min-button-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1155: launch contract should reject min-button-object requests without min button state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--min-button-object",
        "--min-button", "false"
    });
    expect(!missing_targets_result.ok,
        "#1155: launch contract should reject min-button-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--min-button-object",
        "--min-button", "sometimes",
        "--min-button-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1155: launch contract should reject invalid min-button boolean values");
}

void test_parse_launch_arguments_rejects_min_button_object_ambiguity() {
    const auto min_button_allow_output_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--min-button-object",
        "--allow-output-object",
        "--min-button", "false",
        "--min-button-target-unique-id", "one-guid",
        "--allow-output", "false",
        "--allow-output-target-unique-id", "one-guid"
    });
    expect(!min_button_allow_output_result.ok,
        "#1155: launch contract should reject simultaneous min-button-object and allow-output-object requests");

    const auto min_button_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--min-button-object",
        "--clear-property",
        "--property-name", "MinButton",
        "--min-button", "false",
        "--min-button-target-unique-id", "one-guid"
    });
    expect(!min_button_property_result.ok,
        "#1155: launch contract should reject min-button-object combined with property commands");

    const auto stray_min_button_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--min-button", "false"
    });
    expect(!stray_min_button_result.ok,
        "#1155: launch contract should reject stray min-button arguments");
}
