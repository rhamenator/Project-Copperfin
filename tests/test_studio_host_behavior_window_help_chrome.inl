void test_parse_launch_arguments_for_caption_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--caption-object",
        "--caption", "Save Customer",
        "--caption-target-object-name", "cmdSave",
        "--caption-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1042: launch contract should parse caption-object requests");
    expect(result.request.caption_object, "#1042: launch contract should detect --caption-object");
    expect(result.request.caption_available && result.request.caption == "Save Customer",
        "#1042: caption-object requests should carry caption text");
    expect(result.request.caption_objects.size() == 2U,
        "#1042: caption-object requests should collect caption target selectors");
    if (result.request.caption_objects.size() == 2U) {
        expect(result.request.caption_objects[0].object_name == "cmdSave" &&
                result.request.caption_objects[0].unique_id.empty(),
            "#1042: caption-object requests should parse target object-name selectors");
        expect(result.request.caption_objects[1].object_name.empty() &&
                result.request.caption_objects[1].unique_id == "two-guid",
            "#1042: caption-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_caption_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--caption-object",
        "--caption-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1042: launch contract should reject caption-object requests without caption text");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--caption-object",
        "--caption", "Save"
    });
    expect(!missing_targets_result.ok,
        "#1042: launch contract should reject caption-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_caption_object_ambiguity() {
    const auto caption_locked_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--caption-object",
        "--locked-object",
        "--caption", "Save",
        "--caption-target-unique-id", "one-guid",
        "--locked", "true",
        "--locked-target-unique-id", "one-guid"
    });
    expect(!caption_locked_result.ok,
        "#1042: launch contract should reject simultaneous caption-object and locked-object requests");

    const auto caption_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--caption-object",
        "--clear-property",
        "--property-name", "Caption",
        "--caption", "Save",
        "--caption-target-unique-id", "one-guid"
    });
    expect(!caption_property_result.ok,
        "#1042: launch contract should reject caption-object combined with property commands");

    const auto stray_caption_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--caption", "Save"
    });
    expect(!stray_caption_result.ok,
        "#1042: launch contract should reject stray caption arguments");
}

void test_parse_launch_arguments_for_whats_this_help_id_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--whats-this-help-id-object",
        "--whats-this-help-id", "900",
        "--whats-this-help-id-target-object-name", "cmdSave",
        "--whats-this-help-id-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1142: launch contract should parse whats-this-help-id-object requests");
    expect(result.request.whats_this_help_id_object,
        "#1142: launch contract should detect --whats-this-help-id-object");
    expect(result.request.whats_this_help_id_available && result.request.whats_this_help_id == 900,
        "#1142: whats-this-help-id-object requests should carry WhatsThis help ID");
    expect(result.request.whats_this_help_id_objects.size() == 2U,
        "#1142: whats-this-help-id-object requests should collect WhatsThis help ID target selectors");
    if (result.request.whats_this_help_id_objects.size() == 2U) {
        expect(result.request.whats_this_help_id_objects[0].object_name == "cmdSave" &&
                result.request.whats_this_help_id_objects[0].unique_id.empty(),
            "#1142: whats-this-help-id-object requests should parse target object-name selectors");
        expect(result.request.whats_this_help_id_objects[1].object_name.empty() &&
                result.request.whats_this_help_id_objects[1].unique_id == "two-guid",
            "#1142: whats-this-help-id-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_whats_this_help_id_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--whats-this-help-id-object",
        "--whats-this-help-id-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1142: launch contract should reject whats-this-help-id-object requests without WhatsThis help ID");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--whats-this-help-id-object",
        "--whats-this-help-id", "900"
    });
    expect(!missing_targets_result.ok,
        "#1142: launch contract should reject whats-this-help-id-object requests without target selectors");

    const auto non_integer_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--whats-this-help-id-object",
        "--whats-this-help-id", "topic",
        "--whats-this-help-id-target-unique-id", "one-guid"
    });
    expect(!non_integer_result.ok,
        "#1142: launch contract should reject non-integer WhatsThis help ID values");

    const auto negative_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--whats-this-help-id-object",
        "--whats-this-help-id", "-1",
        "--whats-this-help-id-target-unique-id", "one-guid"
    });
    expect(!negative_result.ok,
        "#1142: launch contract should reject negative WhatsThis help ID values");
}

void test_parse_launch_arguments_rejects_whats_this_help_id_object_ambiguity() {
    const auto whats_this_help_id_locked_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--whats-this-help-id-object",
        "--locked-object",
        "--whats-this-help-id", "900",
        "--whats-this-help-id-target-unique-id", "one-guid",
        "--locked", "true",
        "--locked-target-unique-id", "one-guid"
    });
    expect(!whats_this_help_id_locked_result.ok,
        "#1142: launch contract should reject simultaneous whats-this-help-id-object and locked-object requests");

    const auto whats_this_help_id_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--whats-this-help-id-object",
        "--clear-property",
        "--property-name", "ToolTipText",
        "--whats-this-help-id", "900",
        "--whats-this-help-id-target-unique-id", "one-guid"
    });
    expect(!whats_this_help_id_property_result.ok,
        "#1142: launch contract should reject whats-this-help-id-object combined with property commands");

    const auto stray_whats_this_help_id_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--whats-this-help-id", "900"
    });
    expect(!stray_whats_this_help_id_result.ok,
        "#1142: launch contract should reject stray whats-this-help-id arguments");
}

void test_parse_launch_arguments_for_whats_this_help_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--whats-this-help-object",
        "--whats-this-help", ".T.",
        "--whats-this-help-target-object-name", "cmdSave",
        "--whats-this-help-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1143: launch contract should parse whats-this-help-object requests");
    expect(result.request.whats_this_help_object,
        "#1143: launch contract should detect --whats-this-help-object");
    expect(result.request.whats_this_help_available && result.request.whats_this_help,
        "#1143: whats-this-help-object requests should carry WhatsThis help state");
    expect(result.request.whats_this_help_objects.size() == 2U,
        "#1143: whats-this-help-object requests should collect WhatsThis help target selectors");
    if (result.request.whats_this_help_objects.size() == 2U) {
        expect(result.request.whats_this_help_objects[0].object_name == "cmdSave" &&
                result.request.whats_this_help_objects[0].unique_id.empty(),
            "#1143: whats-this-help-object requests should parse target object-name selectors");
        expect(result.request.whats_this_help_objects[1].object_name.empty() &&
                result.request.whats_this_help_objects[1].unique_id == "two-guid",
            "#1143: whats-this-help-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_whats_this_help_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--whats-this-help-object",
        "--whats-this-help-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1143: launch contract should reject whats-this-help-object requests without WhatsThis help value");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--whats-this-help-object",
        "--whats-this-help", "true"
    });
    expect(!missing_targets_result.ok,
        "#1143: launch contract should reject whats-this-help-object requests without target selectors");

    const auto invalid_logical_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--whats-this-help-object",
        "--whats-this-help", "maybe",
        "--whats-this-help-target-unique-id", "one-guid"
    });
    expect(!invalid_logical_result.ok,
        "#1143: launch contract should reject invalid WhatsThis help logical values");
}

void test_parse_launch_arguments_rejects_whats_this_help_object_ambiguity() {
    const auto whats_this_help_locked_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--whats-this-help-object",
        "--locked-object",
        "--whats-this-help", "true",
        "--whats-this-help-target-unique-id", "one-guid",
        "--locked", "true",
        "--locked-target-unique-id", "one-guid"
    });
    expect(!whats_this_help_locked_result.ok,
        "#1143: launch contract should reject simultaneous whats-this-help-object and locked-object requests");

    const auto whats_this_help_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--whats-this-help-object",
        "--clear-property",
        "--property-name", "ToolTipText",
        "--whats-this-help", "true",
        "--whats-this-help-target-unique-id", "one-guid"
    });
    expect(!whats_this_help_property_result.ok,
        "#1143: launch contract should reject whats-this-help-object combined with property commands");

    const auto stray_whats_this_help_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--whats-this-help", "true"
    });
    expect(!stray_whats_this_help_result.ok,
        "#1143: launch contract should reject stray whats-this-help arguments");
}

void test_parse_launch_arguments_for_whats_this_button_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--whats-this-button-object",
        "--whats-this-button", ".T.",
        "--whats-this-button-target-object-name", "cmdSave",
        "--whats-this-button-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1144: launch contract should parse whats-this-button-object requests");
    expect(result.request.whats_this_button_object,
        "#1144: launch contract should detect --whats-this-button-object");
    expect(result.request.whats_this_button_available && result.request.whats_this_button,
        "#1144: whats-this-button-object requests should carry WhatsThis button state");
    expect(result.request.whats_this_button_objects.size() == 2U,
        "#1144: whats-this-button-object requests should collect WhatsThis button target selectors");
    if (result.request.whats_this_button_objects.size() == 2U) {
        expect(result.request.whats_this_button_objects[0].object_name == "cmdSave" &&
                result.request.whats_this_button_objects[0].unique_id.empty(),
            "#1144: whats-this-button-object requests should parse target object-name selectors");
        expect(result.request.whats_this_button_objects[1].object_name.empty() &&
                result.request.whats_this_button_objects[1].unique_id == "two-guid",
            "#1144: whats-this-button-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_whats_this_button_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--whats-this-button-object",
        "--whats-this-button-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1144: launch contract should reject whats-this-button-object requests without WhatsThis button value");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--whats-this-button-object",
        "--whats-this-button", "true"
    });
    expect(!missing_targets_result.ok,
        "#1144: launch contract should reject whats-this-button-object requests without target selectors");

    const auto invalid_logical_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--whats-this-button-object",
        "--whats-this-button", "maybe",
        "--whats-this-button-target-unique-id", "one-guid"
    });
    expect(!invalid_logical_result.ok,
        "#1144: launch contract should reject invalid WhatsThis button logical values");
}

void test_parse_launch_arguments_rejects_whats_this_button_object_ambiguity() {
    const auto whats_this_button_locked_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--whats-this-button-object",
        "--locked-object",
        "--whats-this-button", "true",
        "--whats-this-button-target-unique-id", "one-guid",
        "--locked", "true",
        "--locked-target-unique-id", "one-guid"
    });
    expect(!whats_this_button_locked_result.ok,
        "#1144: launch contract should reject simultaneous whats-this-button-object and locked-object requests");

    const auto whats_this_button_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--whats-this-button-object",
        "--clear-property",
        "--property-name", "ToolTipText",
        "--whats-this-button", "true",
        "--whats-this-button-target-unique-id", "one-guid"
    });
    expect(!whats_this_button_property_result.ok,
        "#1144: launch contract should reject whats-this-button-object combined with property commands");

    const auto stray_whats_this_button_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--whats-this-button", "true"
    });
    expect(!stray_whats_this_button_result.ok,
        "#1144: launch contract should reject stray whats-this-button arguments");
}

void test_parse_launch_arguments_for_status_bar_text_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--status-bar-text-object",
        "--status-bar-text", "Ready to save",
        "--status-bar-text-target-object-name", "cmdSave",
        "--status-bar-text-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1044: launch contract should parse status-bar-text-object requests");
    expect(result.request.status_bar_text_object, "#1044: launch contract should detect --status-bar-text-object");
    expect(result.request.status_bar_text_available && result.request.status_bar_text == "Ready to save",
        "#1044: status-bar-text-object requests should carry status-bar text");
    expect(result.request.status_bar_text_objects.size() == 2U,
        "#1044: status-bar-text-object requests should collect status-bar text target selectors");
    if (result.request.status_bar_text_objects.size() == 2U) {
        expect(result.request.status_bar_text_objects[0].object_name == "cmdSave" &&
                result.request.status_bar_text_objects[0].unique_id.empty(),
            "#1044: status-bar-text-object requests should parse target object-name selectors");
        expect(result.request.status_bar_text_objects[1].object_name.empty() &&
                result.request.status_bar_text_objects[1].unique_id == "two-guid",
            "#1044: status-bar-text-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_status_bar_text_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--status-bar-text-object",
        "--status-bar-text-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1044: launch contract should reject status-bar-text-object requests without status-bar text");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--status-bar-text-object",
        "--status-bar-text", "Ready"
    });
    expect(!missing_targets_result.ok,
        "#1044: launch contract should reject status-bar-text-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_status_bar_text_object_ambiguity() {
    const auto status_tooltip_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--status-bar-text-object",
        "--tooltip-text-object",
        "--status-bar-text", "Ready",
        "--status-bar-text-target-unique-id", "one-guid",
        "--tooltip-text", "Save",
        "--tooltip-text-target-unique-id", "one-guid"
    });
    expect(!status_tooltip_result.ok,
        "#1044: launch contract should reject simultaneous status-bar-text-object and tooltip-text-object requests");

    const auto status_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--status-bar-text-object",
        "--clear-property",
        "--property-name", "StatusBarText",
        "--status-bar-text", "Ready",
        "--status-bar-text-target-unique-id", "one-guid"
    });
    expect(!status_property_result.ok,
        "#1044: launch contract should reject status-bar-text-object combined with property commands");

    const auto stray_status_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--status-bar-text", "Ready"
    });
    expect(!stray_status_result.ok,
        "#1044: launch contract should reject stray status-bar text arguments");
}

void test_parse_launch_arguments_for_closable_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--closable-object",
        "--closable", "false",
        "--closable-target-object-name", "frmCustomer",
        "--closable-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1073: launch contract should parse closable-object requests");
    expect(result.request.closable_object,
        "#1073: launch contract should detect --closable-object");
    expect(result.request.closable_available && !result.request.closable,
        "#1073: closable-object requests should carry closable state");
    expect(result.request.closable_objects.size() == 2U,
        "#1073: closable-object requests should collect closable target selectors");
    if (result.request.closable_objects.size() == 2U) {
        expect(result.request.closable_objects[0].object_name == "frmCustomer" &&
                result.request.closable_objects[0].unique_id.empty(),
            "#1073: closable-object requests should parse target object-name selectors");
        expect(result.request.closable_objects[1].object_name.empty() &&
                result.request.closable_objects[1].unique_id == "two-guid",
            "#1073: closable-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_closable_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--closable-object",
        "--closable-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1073: launch contract should reject closable-object requests without closable state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--closable-object",
        "--closable", "false"
    });
    expect(!missing_targets_result.ok,
        "#1073: launch contract should reject closable-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--closable-object",
        "--closable", "sometimes",
        "--closable-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1073: launch contract should reject invalid closable boolean values");
}

void test_parse_launch_arguments_rejects_closable_object_ambiguity() {
    const auto closable_dynamic_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--closable-object",
        "--dynamic-fore-color-object",
        "--closable", "false",
        "--closable-target-unique-id", "one-guid",
        "--dynamic-fore-color", "RGB(7,8,9)",
        "--dynamic-fore-color-target-unique-id", "one-guid"
    });
    expect(!closable_dynamic_result.ok,
        "#1073: launch contract should reject simultaneous closable-object and dynamic-fore-color-object requests");

    const auto closable_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--closable-object",
        "--clear-property",
        "--property-name", "Closable",
        "--closable", "false",
        "--closable-target-unique-id", "one-guid"
    });
    expect(!closable_property_result.ok,
        "#1073: launch contract should reject closable-object combined with property commands");

    const auto stray_closable_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--closable", "false"
    });
    expect(!stray_closable_result.ok,
        "#1073: launch contract should reject stray closable arguments");
}
